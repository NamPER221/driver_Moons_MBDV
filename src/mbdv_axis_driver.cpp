#include "mbdv/mbdv_axis_driver.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <future>
#include <sstream>
#include <thread>
#include <utility>

namespace mbdv {

namespace {

/// CiA 301 Sub-index 01h (COB-ID used by PDO):
/// Bit 31: 0 = PDO exists / is valid (ENABLED), 1 = PDO does not exist / is not valid (DISABLED).
constexpr uint32_t kPdoDisabledBit = 0x80000000u;

/// last_target_velocity_ before any setpoint was sent (see the header's initialiser).
constexpr int32_t kNoSetpoint = 0x7FFFFFFF;

}  // namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

MbdvAxisDriver::MbdvAxisDriver(lely::canopen::AsyncMaster& master, uint8_t node_id,
                               std::string axis_name, const char* axis_tag)
    : lely::canopen::FiberDriver(master, node_id),
      node_id_(node_id),
      axis_name_(std::move(axis_name)),
      axis_tag_(axis_tag ? axis_tag : "AX?") {}


// ---------------------------------------------------------------------------
// Per-node RPDO transmit
// ---------------------------------------------------------------------------

void MbdvAxisDriver::SetRpdoSender(RpdoSender sender) {
  rpdo_sender_ = std::move(sender);
}

int8_t MbdvAxisDriver::CurrentModeForRpdo1() {
  int8_t mode = mode_of_operation_.load();
  if (mode >= 0) return mode;
  // No mode has been selected through this driver yet: fall back to whatever the master
  // object dictionary holds, so an RPDO1 sent before S10 cannot write 0xFF into 0x6060.
  try {
    mode = ReadOr<int8_t>(od::kModeOfOperation, 0, static_cast<int8_t>(0));
  } catch (const std::exception&) {
    mode = 0;
  }
  return mode;
}

void MbdvAxisDriver::ZeroVelocityNow() {
  last_target_velocity_.store(0);
  try {
    if (!SendRpdo(3, controlword_commands::ENABLE_OPERATION, true, 0)) {
      LogError(Stage::S12_SERVO_ON, axis_tag_,
               "could not send the zero-velocity stop command; this axis may move on its own");
    }
  } catch (const std::exception& ex) {
    LogError(Stage::S12_SERVO_ON, axis_tag_, Str("zero-velocity stop failed: ", ex.what()));
  }
}

void MbdvAxisDriver::StopNow() {
  last_target_velocity_.store(0);
  const CiA402State st = state_.load();
  // Controlword 0x000F enables an axis in Switched On or Ready to Switch On (CiA 402
  // transitions 3 and 4) and resumes one in Quick Stop Active (transition 16): a stop must
  // never be what switches an axis back on.
  if (st == CiA402State::SWITCHED_ON || st == CiA402State::READY_TO_SWITCH_ON ||
      st == CiA402State::QUICK_STOP_ACTIVE) {
    return;
  }
  try {
    bool sent = false;
    const int8_t mode = mode_of_operation_.load();
    if (mode == static_cast<int8_t>(CiA402Mode::PROFILE_POSITION)) {
      // 0x60FF means nothing in profile position; the Halt bit stops the running profile.
      halted_.store(true);
      sent = SendRpdo(1,
                      static_cast<uint16_t>(controlword_commands::ENABLE_OPERATION |
                                            controlword_bits::HALT),
                      false, mode);
    } else {
      sent = SendRpdo(3, controlword_commands::ENABLE_OPERATION, true, 0);
    }
    if (!sent) {
      LogError(Stage::S13_MOTION_COMMAND, axis_tag_, "could not send the stop command");
    }
  } catch (const std::exception& ex) {
    LogError(Stage::S13_MOTION_COMMAND, axis_tag_, Str("stop command failed: ", ex.what()));
  }
}

bool MbdvAxisDriver::WaitForReconnectReady(std::chrono::milliseconds timeout) {
  // lely ends every boot-slave process with OnBoot(), successful or not; the cap only
  // guards against a boot-up frame that never started one.
  constexpr std::chrono::seconds kBootProcessCap{10};
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  while (!abort_.load()) {
    bool booting = boot_in_progress_.load();
    if (booting) {
      std::lock_guard<std::mutex> lock(time_mutex_);
      booting = std::chrono::steady_clock::now() - boot_start_time_ < kBootProcessCap;
    }
    if (!heartbeat_lost_.load() && !booting) return true;
    if (std::chrono::steady_clock::now() >= deadline) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  return false;
}

bool MbdvAxisDriver::SendRpdo(uint8_t pdo_no, uint16_t controlword, bool has_setpoint,
                              int32_t setpoint) {
  if (!rpdo_sender_) {
    // No per-node transport installed: fall back to the shared object dictionary. That is
    // only correct for a single-node bus, so say so once and loudly.
    static std::atomic_flag warned = ATOMIC_FLAG_INIT;
    if (!warned.test_and_set()) {
      LogError(Stage::S13_MOTION_COMMAND, axis_tag_,
               "no per-node RPDO transport installed: falling back to the master's shared "
               "object dictionary. With more than one node on this master every axis will "
               "receive every other axis' setpoint and the axes will cancel each other out.");
    }
    try {
      tpdo_mapped[od::kControlword][0] = controlword;
      if (has_setpoint) {
        if (pdo_no == 2) {
          tpdo_mapped[od::kTargetPosition][0] = setpoint;
          tpdo_mapped[od::kTargetPosition][0].WriteEvent();
        } else if (pdo_no == 3) {
          tpdo_mapped[od::kTargetVelocity][0] = setpoint;
          tpdo_mapped[od::kTargetVelocity][0].WriteEvent();
        }
      }
      tpdo_mapped[od::kControlword][0].WriteEvent();
      master.TpdoEvent();
      return true;
    } catch (const std::exception&) {
      return false;
    }
  }
  return rpdo_sender_(node_id_, pdo_no, controlword, has_setpoint, setpoint);
}

// ---------------------------------------------------------------------------
// Small SDO helpers (fiber side only)
// ---------------------------------------------------------------------------

template <typename T>
T MbdvAxisDriver::ReadOr(uint16_t idx, uint8_t subidx, T fallback) {
  try {
    return Wait(AsyncRead<T>(idx, subidx));
  } catch (const std::exception&) {
    return fallback;
  }
}

template <typename T>
bool MbdvAxisDriver::TryWrite(uint16_t idx, uint8_t subidx, T value, std::string* why) {
  try {
    Wait(AsyncWrite(idx, subidx, value));
    return true;
  } catch (const std::exception& ex) {
    if (why) *why = ex.what();
    return false;
  }
}

void MbdvAxisDriver::RefreshStateFromStatusword(uint16_t sw) noexcept {
  statusword_.store(sw);
  state_.store(decode_cia402_state(sw));
  target_reached_.store((sw & statusword_bits::TARGET_REACHED) != 0);
  std::lock_guard<std::mutex> lock(time_mutex_);
  statusword_time_ = std::chrono::steady_clock::now();
}

bool MbdvAxisDriver::RefreshStatusword() {
  try {
    const uint16_t sw = Wait(AsyncRead<uint16_t>(od::kStatusword, 0));
    RefreshStateFromStatusword(sw);
    // Deliberately NOT counted in statusword_count_: that counter is the TPDO frame
    // count, and mixing SDO reads into it made the telemetry-rate figure meaningless.
    ++statusword_sdo_reads_;
    return true;
  } catch (const std::exception& ex) {
    LogDebug(Stage::S12_SERVO_ON, axis_tag_, Str("SDO statusword read failed: ", ex.what()));
    return false;
  }
}

bool MbdvAxisDriver::IsTelemetryStale(std::chrono::milliseconds window) const {
  std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(time_mutex_));
  if (statusword_time_ == std::chrono::steady_clock::time_point{}) return true;
  return std::chrono::steady_clock::now() - statusword_time_ > window;
}

std::string MbdvAxisDriver::TelemetryStaleNote() const {
  return IsTelemetryStale(std::chrono::milliseconds(500)) ? ", telemetry STALE"
                                                          : ", telemetry live";
}

// ---------------------------------------------------------------------------
// Stage S05 - Boot-up
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::StageBootUp(const BringUpOptions& opt) {
  const auto deadline = std::chrono::steady_clock::now() + opt.boot_timeout;
  while (std::chrono::steady_clock::now() < deadline && !abort_.load()) {
    if (boot_seen_.load()) return true;
    USleep(10000);
  }
  return false;
}

// ---------------------------------------------------------------------------
// Stage S06 - PRE-OPERATIONAL / SDO reachable
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::StagePreOp(const BringUpOptions& opt) {
  // A successful SDO upload is the authoritative proof that the node answers on
  // 0x600+node, i.e. it left Boot-up and reached PRE-OPERATIONAL.
  const auto deadline = std::chrono::steady_clock::now() + opt.boot_timeout;
  std::string last_error = "no attempt made";
  while (std::chrono::steady_clock::now() < deadline && !abort_.load()) {
    try {
      const uint16_t sw = Wait(AsyncRead<uint16_t>(od::kStatusword, 0));
      RefreshStateFromStatusword(sw);
      ++statusword_sdo_reads_;
      LogInfo(Stage::S06_PREOP, axis_tag_,
              Str("SDO channel alive; Statusword=", Hex(sw, 4), " (",
                  cia402_state_to_string(decode_cia402_state(sw)), ")"));
      return true;
    } catch (const std::exception& ex) {
      last_error = ex.what();
    }
    USleep(100000);
  }
  LogError(Stage::S06_PREOP, axis_tag_,
           Str("SDO upload 0x6041 timed out after ", opt.boot_timeout.count(),
               " ms; last error: ", last_error));
  return false;
}

// ---------------------------------------------------------------------------
// Stage S07 - NMT START -> OPERATIONAL
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::StageNmtStart(const BringUpOptions& opt) {
  LogInfo(Stage::S07_NMT_START, axis_tag_, "sending NMT Start Remote Node (CS=0x01)");
  master.Command(lely::canopen::NmtCommand::START, id());

  const auto deadline = std::chrono::steady_clock::now() + opt.boot_timeout;
  while (std::chrono::steady_clock::now() < deadline && !abort_.load()) {
    if (is_operational_.load()) return true;
    USleep(10000);
  }
  return false;
}

// ---------------------------------------------------------------------------
// Stage S08 - Identity + bus parameter verification
// ---------------------------------------------------------------------------

MbdvAxisDriver::StageVerdict MbdvAxisDriver::StageIdentity(const BringUpOptions& opt) {
  StageVerdict verdict;
  const Stage stage = Stage::S08_IDENTITY;

  auto reject = [this, &verdict](const std::string& reason, const std::string& hint) {
    verdict.ok = false;
    verdict.reason = reason;
    verdict.hint = hint;
    LogError(Stage::S08_IDENTITY, axis_tag_, Str("identity check failed: ", reason));
    return verdict;
  };

  // --- 0x1000 device type ---
  const uint32_t device_type = ReadOr<uint32_t>(od::kDeviceType, 0, 0u);
  const uint8_t profile_no = static_cast<uint8_t>((device_type >> 16) & 0xFFu);
  const uint16_t type_no = static_cast<uint16_t>(device_type & 0xFFFFu);
  LogInfo(stage, axis_tag_,
          Str("0x1000 Device type = ", Hex(device_type, 8), " -> CiA profile ",
              static_cast<int>(profile_no), ", type ", type_no));
  if (profile_no != 0x06) {
    return reject(Str("device type ", Hex(device_type, 8), " does not declare the CiA 402 "
                      "profile (expected profile number 0x06 in bits 23..16)"),
                  "A non-402 device is answering at this node-ID. Check the CAN wiring and the "
                  "DIP switch node address.");
  }

  // --- 0x1018:01 vendor id ---
  const uint32_t vendor_id = ReadOr<uint32_t>(od::kIdentity, 1, 0u);
  if (vendor_id != 0x000002D9u) {
    return reject(Str("unexpected vendor ID ", Hex(vendor_id, 8),
                      " (the EDS declares 0x000002D9 = Shanghai AMP & Moons')"),
                  "A foreign device is answering on this node-ID. Verify CAN_H/CAN_L, the GND "
                  "connection and the DIP switch node address.");
  }
  LogInfo(stage, axis_tag_, "0x1018:01 Vendor ID matches 0x000002D9 (Shanghai AMP & Moons')");

  // --- 0x1018:02 product code / :03 revision ---
  const uint32_t product_code = ReadOr<uint32_t>(od::kIdentity, 2, 0u);
  const uint32_t revision = ReadOr<uint32_t>(od::kIdentity, 3, 0u);
  LogInfo(stage, axis_tag_,
          Str("0x1018:02 Product code = ", Hex(product_code, 8), ", 0x1018:03 Revision = ",
              Hex(revision, 8)));

  // --- 0x2020 node id actually in use ---
  const uint16_t reported_node = ReadOr<uint16_t>(od::kNodeIdObject, 0, 0xFFFFu);
  if (reported_node == 0xFFFFu) {
    return reject("object 0x2020 (Node ID) is not readable",
                  "The EDS marks 0x2020 read-only; an unreadable value means the node is still "
                  "in Boot-up or the SDO channel is not established (see stage S06).");
  }
  if (reported_node != node_id_) {
    return reject(Str("node-ID mismatch: the master addresses node ", static_cast<int>(node_id_),
                      " but the drive reports 0x2020 = ", static_cast<int>(reported_node),
                      " (", node_id_to_string(reported_node), ")"),
                  "MBDV-2X-520AC DIP switches: axis 1 node-ID = SW1..SW3, axis 2 node-ID = "
                  "SW4..SW6. A reported value of 1 while you expected 2 means both axes are "
                  "configured with all switches OFF, which selects the Luna software address "
                  "(default 1).");
  }
  LogInfo(stage, axis_tag_,
          Str("0x2020 Node ID = ", static_cast<int>(reported_node), " matches the master"));

  // --- 0x2021 bit rate actually in use ---
  // Measured on an MBDV-2X-520AC: this object returns the speed in kbps (a 500 kbps
  // drive reads 500), not the P1-18 CB code (which would read 2).
  const uint16_t bitrate_raw = ReadOr<uint16_t>(od::kBitRateObject, 0, 0xFFFFu);
  if (bitrate_raw == 0xFFFFu) {
    return reject("object 0x2021 (Bit rate) is not readable",
                  "The EDS marks 0x2021 read-only; an unreadable value means the node is still "
                  "in Boot-up or the SDO channel is not established (see stage S06).");
  }
  LogInfo(stage, axis_tag_, DecodeBitRateObject(bitrate_raw));

  uint32_t measured_bps = 0;
  if (bitrate_raw >= 12u && bitrate_raw <= 1000u) {
    measured_bps = static_cast<uint32_t>(bitrate_raw) * 1000u;
  } else if (bitrate_raw <= 7u) {
    measured_bps = can_bit_rate_code_to_bps(bitrate_raw);
  }

  if (opt.expect_bitrate_bps != 0 && measured_bps != opt.expect_bitrate_bps) {
    return reject(Str("bit-rate mismatch: the drive runs at ",
                      can_bit_rate_bps_to_string(measured_bps), " (0x2021 = ",
                      static_cast<int>(bitrate_raw), ") but the host channel is configured for ",
                      can_bit_rate_bps_to_string(opt.expect_bitrate_bps)),
                  "MBDV-2X-520AC DIP SW7: 0 = Luna setting (default 1 Mbps), 1 = 500 kbps. "
                  "Match the host with 'ip link set <if> type can bitrate <n>', or tell the "
                  "tool what to expect with --baud.");
  }

  // --- 0x2030 DC bus + Statusword bit 4: is the drive actually powered? ---
  // CiA 402 Statusword bit 4 (Voltage_enabled) is the authoritative "main voltage
  // present" signal and is preferred over interpreting 0x2030, whose unit the EDS does
  // not document. An earlier version of this code called any reading below 25 V
  // "unpowered"; that was wrong - manual section 4.3 gives the main input as
  // 24 ~ 60 VDC, so ~24 V is a legal, in-spec reading.
  const uint16_t dc_bus_raw = ReadOr<uint16_t>(od::kDcBusVoltage, 0, 0u);
  const uint16_t sw_now = statusword_.load();
  const bool main_voltage = MainVoltagePresent(sw_now);
  LogInfo(stage, axis_tag_,
          Str("0x2030 DC bus: ", DecodeDcBusVoltage(dc_bus_raw)));
  LogInfo(stage, axis_tag_,
          Str("main voltage: ", main_voltage ? "PRESENT" : "ABSENT",
              " (Statusword bit 4 Voltage_enabled = ", main_voltage ? "1" : "0", ")"));

  if (opt.check_dc_bus && !main_voltage) {
    LogWarn(stage, axis_tag_,
            "The drive reports no main voltage. Manual section 4.3: V+/V- takes "
            "24 ~ 60 VDC; the 24V/GND auxiliary supply alone cannot enable the servo.");
  } else if (opt.check_dc_bus && DcBusOutOfSpec(dc_bus_raw)) {
    LogWarn(stage, axis_tag_,
            Str("Statusword reports main voltage present, but 0x2030 is outside the "
                "24 ~ 60 VDC range of manual section 4.3. Check the supply under load."));
  }

  // --- 0x2070 DIP switch bitmap (manual section 4.2.2) ---
  const uint32_t switches = ReadOr<uint32_t>(od::kSwitchValue, 0, 0u);
  const DipSwitchDecode dip = DecodeDipSwitch(switches);
  LogInfo(stage, axis_tag_, Str("0x2070 DIP switches = ", Hex(switches, 8), " -> ",
                                DescribeDipSwitch(dip)));
  if (dip.term_resistor) {
    LogInfo(stage, axis_tag_,
            "SW8 reports a 120 ohm terminator fitted; it must only be ON at the last device "
            "on the bus");
  }
  if (!dip.baud_from_dip && opt.expect_bitrate_bps != 0 &&
      opt.expect_bitrate_bps != 500000u) {
    LogInfo(stage, axis_tag_,
            "SW7 is OFF, so the bit rate comes from the Luna software (P1-18) rather than "
            "from the DIP");
  }

  // --- 0x2A30 drive control mode P1-00 ---
  uint32_t control_mode = ReadOr<uint32_t>(od::kControlMode, 0, 0u);
  LogInfo(stage, axis_tag_,
          Str("0x2A30 Control mode (P1-00) = ", static_cast<int>(control_mode), " -> ",
              drive_control_mode_to_string(control_mode)));
  const uint32_t operation_mode = ReadOr<uint32_t>(od::kOperationMode, 0, 0u);
  LogInfo(stage, axis_tag_,
          Str("0x2A32 Operation mode (read-only) = ", static_cast<int>(operation_mode)));
  if (control_mode != 1u && control_mode != 15u && control_mode != 21u) {
    LogError(stage, axis_tag_,
             Str("P1-00 reads ", static_cast<int>(control_mode),
                 ", which is NOT a documented value (1 = Torque, 15 = Velocity, "
                 "21 = Position - manual section 8.3.2). The drive control mode is therefore "
                 "undefined and this can explain unexpected RPDO behaviour. Do NOT keep "
                 "writing 0x1010:01 = 1 from this tool; set P1-00 once in Luna and power-cycle "
                 "to confirm it sticks."));
  }

  // Apply a requested or expected P1-00 *here*, before the check: this stage owns the comparison, so
  // writing the value later could never repair a mismatch.
  const uint32_t target_mode = (opt.write_control_mode != 0) ? opt.write_control_mode : opt.expect_control_mode;
  if (target_mode != 0 && control_mode != target_mode) {
    LogWarn(stage, axis_tag_,
            Str("auto-aligning P1-00 to required mode: writing ", static_cast<int>(control_mode), " -> ",
                static_cast<int>(target_mode), " to ", ObjRef(od::kControlMode, 0),
                " (", drive_control_mode_to_string(target_mode), ")"));
    if (!SetDriveControlModeImpl(target_mode)) {
      return reject(Str("could not write ", static_cast<int>(target_mode),
                        " to ", ObjRef(od::kControlMode, 0)),
                    "The EDS marks 0x2A30 as rw. If the read-back does not match, the write was "
                    "refused by the drive - check that the parameter lock (0x2A35) is not "
                    "enabled and that the drive is not in a protected control mode.");
    }
    control_mode = ReadOr<uint32_t>(od::kControlMode, 0, 0u);
  }

  // --- 0x6083/0x6084 profile acceleration / deceleration ---
  //
  // These are the ramp the drive uses for *both* profile position and profile velocity.
  // If they are zero the drive will happily reach Operation Enabled and then ignore every
  // setpoint forever, because there is no ramp to follow. Write them here, while the node
  // is still in pre-operational, and report what came back so a refused write is visible
  // instead of showing up later as "the motor never moves".
  if (opt.profile_accel != 0 || opt.profile_decel != 0) {
    const auto apply = [this, opt](uint16_t idx, uint32_t value, const char* label) {
      const uint32_t before = ReadOr<uint32_t>(idx, 0, 0u);
      if (before == value) {
        LogInfo(stage, axis_tag_,
                Str(label, " ", ObjRef(idx, 0), " = ", before, " already as required"));
        return true;
      }
      std::string why;
      if (!TryWrite<uint32_t>(idx, 0, value, &why)) {
        LogWarn(stage, axis_tag_,
                Str("SDO write ", value, " to ", ObjRef(idx, 0), " (", label, ") failed: ",
                    why, "; the drive keeps ", before,
                    ". A zero ramp makes the drive ignore every target position even while "
                    "it reports Operation Enabled."));
        return false;
      }
      const uint32_t after = ReadOr<uint32_t>(idx, 0, 0u);
      LogInfo(stage, axis_tag_,
              Str(label, ": ", ObjRef(idx, 0), " ", before, " -> ", after,
                  (after == value) ? " (accepted)" : " (READ-BACK MISMATCH)"));
      return after == value;
    };
    apply(od::kProfileAcceleration, opt.profile_accel, "profile acceleration");
    apply(od::kProfileDeceleration, opt.profile_decel, "profile deceleration");
  }

  if (opt.expect_control_mode != 0 && control_mode != opt.expect_control_mode) {
    return reject(Str("drive control mode mismatch: 0x2A30 = ", static_cast<int>(control_mode),
                      " but ", static_cast<int>(opt.expect_control_mode),
                      " is required for the requested CiA 402 mode"),
                  "0x6060 is ignored unless P1-00 selects the matching drive control mode "
                  "(PP=21, PV=15, TQ=1). Re-run with --p1-00 21 for PP, --p1-00 15 for PV, "
                  "or --p1-00 1 for TQ; the tool writes it for this power cycle only (it is "
                  "deliberately not stored with 0x1010:01) - set P1-00 in Luna to keep it.");
  }

  // --- 0x2060 communication watchdog (manual P1-39) ---
  const uint32_t wd_enable = ReadOr<uint32_t>(od::kCommWatchdog, od::kCommWatchdogEnable, 0u);
  const uint32_t wd_status = ReadOr<uint32_t>(od::kCommWatchdog, od::kCommWatchdogStatus, 0u);
  const uint32_t wd_timeout = ReadOr<uint32_t>(od::kCommWatchdog, od::kCommWatchdogTimeout, 0u);
  const uint32_t wd_trigger = ReadOr<uint32_t>(od::kCommWatchdog, od::kCommWatchdogTrigger, 0u);
  LogInfo(stage, axis_tag_,
          DescribeCommWatchdog(wd_enable, wd_status, wd_timeout, wd_trigger));

  if (opt.watchdog_timeout_ms >= 0) {
    // The EDS declares 0x2060:01 and :03 as UNSIGNED32, but this firmware rejects a 4-byte
    // download with abort 0x06070010, so the widths are tried in turn. Failure is a warning:
    // tuning the watchdog is a convenience, never a bring-up requirement.
    std::string why;
    bool applied = false;
    const uint32_t value = static_cast<uint32_t>(opt.watchdog_timeout_ms);
    const uint16_t value16 = static_cast<uint16_t>(value);
    for (int width = 0; width < 3 && !applied; ++width) {
      why.clear();
      bool ok = false;
      if (opt.watchdog_timeout_ms == 0) {
        ok = width == 0 ? TryWrite<uint8_t>(od::kCommWatchdog, od::kCommWatchdogEnable, uint8_t{0}, &why)
            : width == 1 ? TryWrite<uint16_t>(od::kCommWatchdog, od::kCommWatchdogEnable, uint16_t{0}, &why)
                         : TryWrite<uint32_t>(od::kCommWatchdog, od::kCommWatchdogEnable, uint32_t{0}, &why);
      } else {
        const bool enable_ok =
            width == 0 ? TryWrite<uint8_t>(od::kCommWatchdog, od::kCommWatchdogEnable, uint8_t{1}, &why)
            : width == 1 ? TryWrite<uint16_t>(od::kCommWatchdog, od::kCommWatchdogEnable, uint16_t{1}, &why)
                         : TryWrite<uint32_t>(od::kCommWatchdog, od::kCommWatchdogEnable, uint32_t{1}, &why);
        const bool timeout_ok =
            enable_ok &&
            (width == 0 ? TryWrite<uint16_t>(od::kCommWatchdog, od::kCommWatchdogTimeout, value16, &why)
             : width == 1 ? TryWrite<uint16_t>(od::kCommWatchdog, od::kCommWatchdogTimeout, value16, &why)
                          : TryWrite<uint32_t>(od::kCommWatchdog, od::kCommWatchdogTimeout, value, &why));
        ok = enable_ok && timeout_ok;
      }
      applied = ok;
    }
    if (applied) {
      LogWarn(stage, axis_tag_,
              Str(opt.watchdog_timeout_ms == 0 ? "communication watchdog DISABLED"
                                               : "communication watchdog timeout set",
                  " via --watchdog"));
    } else {
      LogWarn(stage, axis_tag_,
              Str("could not write the communication watchdog (0x2060), last error: ", why,
                  ". This drive reports the watchdog as disabled anyway (enable=0, "
                  "timeout=0 ms), so it is not the source of the COMMUNICATION(b4) EMERGENCY. "
                  "Set P1-39 in Luna if it needs changing."));
    }
  }

  // --- 0x2060:05 action on a watchdog trip (Luna P1-40) ---
  // Without a stopping action a tripped watchdog is only a Warning that "does not change the
  // current state" (hardware manual 9.1): with the CAN cable cut the axes keep their last
  // setpoint. The meaning of each value is not in the hardware manual, so nothing is written
  // unless params.yaml names one.
  const uint16_t wd_option = ReadOr<uint16_t>(od::kCommWatchdog, od::kCommWatchdogOption, 0xFFFFu);
  if (opt.watchdog_action >= 0 && wd_option != static_cast<uint16_t>(opt.watchdog_action)) {
    std::string why;
    if (TryWrite<uint16_t>(od::kCommWatchdog, od::kCommWatchdogOption,
                           static_cast<uint16_t>(opt.watchdog_action), &why)) {
      LogWarn(stage, axis_tag_,
              Str(ObjRef(od::kCommWatchdog, od::kCommWatchdogOption), " (watchdog action) ",
                  static_cast<int>(wd_option), " -> ",
                  static_cast<int>(ReadOr<uint16_t>(od::kCommWatchdog, od::kCommWatchdogOption,
                                                    0xFFFFu))));
    } else {
      LogWarn(stage, axis_tag_,
              Str("could not write ", opt.watchdog_action, " to ",
                  ObjRef(od::kCommWatchdog, od::kCommWatchdogOption), ": ", why));
    }
  } else {
    LogInfo(stage, axis_tag_,
            Str(ObjRef(od::kCommWatchdog, od::kCommWatchdogOption), " (watchdog action) = ",
                wd_option == 0xFFFFu ? std::string("unreadable") : Str(static_cast<int>(wd_option)),
                opt.watchdog_action >= 0 ? " as required" : " (left untouched)"));
  }

  // --- 0x1001 error register / 0x603F error code ---
  const uint8_t error_register = ReadOr<uint8_t>(od::kErrorRegister, 0, 0u);
  const uint16_t error_code = ReadOr<uint16_t>(od::kErrorCode, 0, 0u);
  error_register_.store(error_register);
  error_code_.store(error_code);
  if (error_register != 0 || error_code != 0) {
    DriveSnapshot snap;
    snap.statusword = statusword_.load();
    snap.error_code = error_code;
    snap.error_register = error_register;
    snap.dsp_alarm = ReadOr<uint32_t>(od::kDspAlarmCode, 0, 0u);
    snap.dsp_status = ReadOr<uint32_t>(od::kDspStatusCode, 0, 0u);
    snap.sub_alarm = ReadOr<uint32_t>(od::kSubAlarmCode, 0, 0u);
    snap.dc_bus_raw = ReadOr<uint16_t>(od::kDcBusVoltage, 0, 0u);
    snap.current_actual = ReadOr<int16_t>(od::kCurrentActual, 0, 0);
    snap.following_error = ReadOr<int32_t>(od::kFollowingError, 0, 0);
    LogWarn(stage, axis_tag_,
            Str("drive already reports an error: ", DecodeErrorRegister(error_register), " / ",
                DecodeErrorCode402(error_code), FormatDriveSnapshot(snap),
                "This is not fatal for stage S08; stage S11 will try to clear it."));
  } else {
    LogInfo(stage, axis_tag_, "0x1001 error register and 0x603F error code are both clear");
  }

  verdict.ok = true;
  verdict.reason = Str("vendor 0x000002D9, node-ID ", static_cast<int>(node_id_), ", ",
                       can_bit_rate_bps_to_string(measured_bps), ", P1-00=",
                       static_cast<int>(control_mode));
  return verdict;
}

// ---------------------------------------------------------------------------
// Stage S09 - PDO configuration + verification
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::ConfigureAndVerifyPdos(const PdoPlan& plan, std::string* detail) {
  const uint8_t node = node_id_;
  std::string why;
  std::ostringstream notes;

  struct PdoSpec {
    uint16_t comm_idx;  ///< 0x1400.. / 0x1800..
    uint32_t cob_id;
    uint32_t map[2];
    uint8_t map_count;
    uint16_t event_timer_ms;
  };

  // COB-IDs follow the drive factory defaults (EDS 0x1400/0x1600, 0x1800/0x1A00).
  const PdoSpec specs[] = {
      // RPDO1 0x200+node : Controlword + Modes of operation
      {0x1400, 0x200u + node, {0x60400010u, 0x60600008u}, 2, 0},
      // RPDO2 0x300+node : Controlword + Target position
      {0x1401, 0x300u + node, {0x60400010u, 0x607A0020u}, 2, 0},
      // RPDO3 0x400+node : Controlword + Target velocity
      {0x1402, 0x400u + node, {0x60400010u, 0x60FF0020u}, 2, 0},
      // TPDO1 0x180+node : Statusword
      {0x1800, 0x180u + node, {0x60410010u, 0x00000000u}, 1, plan.tpdo1_event_timer_ms},
      // TPDO2 0x280+node : Position actual + Velocity actual
      {0x1801, 0x280u + node, {0x60640020u, 0x606C0020u}, 2, plan.tpdo2_event_timer_ms},
      // TPDO3 0x380+node : Error code + DSP alarm code (live diagnostics)
      {0x1802, 0x380u + node, {0x603F0010u, 0x200F0020u}, 2, plan.tpdo3_event_timer_ms},
  };
  constexpr std::size_t kSpecCount = sizeof(specs) / sizeof(specs[0]);

  auto fail = [&](const std::string& msg) {
    if (detail) *detail = msg;
    return false;
  };

  // Reads back every configured PDO and compares against the plan.
  auto verify = [&specs, kSpecCount, this](std::vector<std::string>* out) {
    out->clear();
    for (std::size_t s = 0; s < kSpecCount; ++s) {
      const PdoSpec& spec = specs[s];
      const uint32_t cob = ReadOr<uint32_t>(spec.comm_idx, 1, 0u);
      if ((cob & ~kPdoDisabledBit) != spec.cob_id) {
        out->push_back(Str(ObjRef(spec.comm_idx, 1), " expected COB-ID ",
                           Hex(spec.cob_id, 3), " got ", Hex(cob & ~kPdoDisabledBit, 3)));
      }
      if ((cob & kPdoDisabledBit) != 0) {
        out->push_back(Str(ObjRef(spec.comm_idx, 1), " has bit 31 set (PDO is disabled)"));
      }
      const uint8_t transmission = ReadOr<uint8_t>(spec.comm_idx, 2, 0xFFu);
      if ((transmission & 0x03u) == 0x02u) {
        out->push_back(Str(ObjRef(spec.comm_idx, 2), " = ", Hex(transmission, 2),
                           " is RTR-only; the drive would ignore spontaneous frames"));
      }
      const uint16_t map_idx = static_cast<uint16_t>(spec.comm_idx + 0x200);
      const uint8_t map_count = ReadOr<uint8_t>(map_idx, 0, 0u);
      if (map_count != spec.map_count) {
        out->push_back(Str(ObjRef(map_idx, 0), " expected ", static_cast<int>(spec.map_count),
                           " got ", static_cast<int>(map_count)));
        continue;
      }
      for (uint8_t i = 0; i < spec.map_count; ++i) {
        const uint32_t entry = ReadOr<uint32_t>(map_idx, static_cast<uint8_t>(i + 1), 0u);
        if (entry != spec.map[i]) {
          out->push_back(Str(ObjRef(map_idx, static_cast<uint8_t>(i + 1)), " expected ",
                             Hex(spec.map[i], 8), " got ", Hex(entry, 8)));
        }
      }
      if (spec.comm_idx >= 0x1800 && spec.event_timer_ms > 0) {
        const uint16_t timer = ReadOr<uint16_t>(spec.comm_idx, 5, 0u);
        if (timer != spec.event_timer_ms) {
          out->push_back(Str(ObjRef(spec.comm_idx, 5), " expected ",
                             static_cast<int>(spec.event_timer_ms), " got ",
                             static_cast<int>(timer)));
        }
      }
    }
  };

  // Enables or disables a PDO by writing its COB-ID with bit 31 clear or set.
  // In CiA 301: bit 31 = 0 means PDO valid (enabled), bit 31 = 1 means PDO not valid (disabled).
  // The COB-ID value itself is always preserved: this drive rejects a literal 0 with
  // SDO abort 0x06090030 ("Invalid value for parameter"), so 0 must never be written
  // to 0x140x:01 / 0x180x:01.
  auto set_pdo_valid = [this, &why](uint16_t comm_idx, uint32_t cob, bool valid) {
    const uint32_t desired = valid ? (cob & ~kPdoDisabledBit) : (cob | kPdoDisabledBit);
    const uint32_t current = ReadOr<uint32_t>(comm_idx, 1, 0u);
    if (current == desired) {
      LogDebug(Stage::S09_PDO_VERIFY, axis_tag_,
               Str(ObjRef(comm_idx, 1), " already ", valid ? "enabled" : "disabled",
                   " (COB-ID ", Hex(current & ~kPdoDisabledBit, 3), ")"));
      return true;
    }
    if (!TryWrite<uint32_t>(comm_idx, 1, desired, &why)) {
      LogError(Stage::S09_PDO_VERIFY, axis_tag_,
               Str("SDO write ", ObjRef(comm_idx, 1), " = ", Hex(desired, 8), " failed: ",
                   why));
      return false;
    }
    LogDebug(Stage::S09_PDO_VERIFY, axis_tag_,
             Str(ObjRef(comm_idx, 1), " set to ", Hex(desired, 8)));
    return true;
  };

  // ---- Phase A: verify only. The concise-DCF download has normally already applied the
  // mapping, so reprogramming would be unnecessary work. ----
  std::vector<std::string> mismatches;
  verify(&mismatches);
  bool repaired = false;

  if (mismatches.empty()) {
    notes << "mapping already matched the plan (applied by the concise-DCF download)";
  } else if (!plan.program_pdos) {
    std::ostringstream os;
    for (const std::string& item : mismatches) os << "\n        - " << item;
    return fail(Str(mismatches.size(),
                   " PDO mismatch(es), and --no-pdo-program was given:", os.str()));
  } else {
    // ---- Phase B: repair, per PDO, in CiA 301 order ----
    LogWarn(Stage::S09_PDO_VERIFY, axis_tag_,
            Str("PDO verification found ", static_cast<int>(mismatches.size()),
                " mismatch(es); re-programming the affected PDOs in CiA 301 order"));

    for (std::size_t s = 0; s < kSpecCount; ++s) {
      const PdoSpec& spec = specs[s];
      const uint16_t map_idx = static_cast<uint16_t>(spec.comm_idx + 0x200);

      // 1. disable (bit 31 set)
      if (!set_pdo_valid(spec.comm_idx, spec.cob_id, false)) {
        return fail(Str("could not disable ", ObjRef(spec.comm_idx, 1),
                        " before remapping; abort: ", why));
      }
      // 2. clear the mapping
      if (!TryWrite<uint8_t>(map_idx, 0, uint8_t{0}, &why)) {
        return fail(Str("SDO write ", ObjRef(map_idx, 0), " = 0 failed: ", why));
      }
      // 3. write the mapping entries
      for (uint8_t i = 0; i < spec.map_count; ++i) {
        if (!TryWrite<uint32_t>(map_idx, static_cast<uint8_t>(i + 1), spec.map[i], &why)) {
          return fail(Str("SDO write ", ObjRef(map_idx, static_cast<uint8_t>(i + 1)), " = ",
                          Hex(spec.map[i], 8), " failed: ", why));
        }
      }
      // 4. activate the new mapping
      if (!TryWrite<uint8_t>(map_idx, 0, spec.map_count, &why)) {
        return fail(Str("SDO write ", ObjRef(map_idx, 0), " = ",
                        static_cast<int>(spec.map_count), " failed: ", why));
      }
      // 5. transmission type (the drive ships 0xFE for RPDO2/RPDO3 = "RTR only")
      if (!TryWrite<uint8_t>(spec.comm_idx, 2, static_cast<uint8_t>(plan.transmission_type),
                             &why)) {
        return fail(Str("SDO write ", ObjRef(spec.comm_idx, 2), " = ",
                        static_cast<int>(plan.transmission_type), " failed: ", why));
      }
      // 6. event timer (TPDO only)
      if (spec.comm_idx >= 0x1800 && spec.event_timer_ms > 0) {
        if (!TryWrite<uint16_t>(spec.comm_idx, 5, spec.event_timer_ms, &why)) {
          return fail(Str("SDO write ", ObjRef(spec.comm_idx, 5), " = ",
                          static_cast<int>(spec.event_timer_ms), " ms failed: ", why));
        }
      }
      // 7. re-enable (bit 31 clear)
      if (!set_pdo_valid(spec.comm_idx, spec.cob_id, true)) {
        return fail(Str("could not enable ", ObjRef(spec.comm_idx, 1), "; abort: ", why));
      }
    }
    repaired = true;
    notes << "mismatches repaired in CiA 301 order";
    verify(&mismatches);
  }

  // ---- Phase C: the used PDOs must be ENABLED (bit 31 CLEAR) to actually exchange data ----
  // In CiA 301: bit 31 = 0 means valid/enabled, bit 31 = 1 means invalid/disabled.
  int enabled_here = 0;
  for (std::size_t s = 0; s < kSpecCount; ++s) {
    const PdoSpec& spec = specs[s];
    const uint32_t cob = ReadOr<uint32_t>(spec.comm_idx, 1, 0u);
    if ((cob & ~kPdoDisabledBit) != spec.cob_id || (cob & kPdoDisabledBit) != 0) {
      if (!TryWrite<uint32_t>(spec.comm_idx, 1, spec.cob_id & ~kPdoDisabledBit, &why)) {
        return fail(Str("SDO write ", ObjRef(spec.comm_idx, 1), " = ",
                        Hex(spec.cob_id & ~kPdoDisabledBit, 8), " (enable) failed: ", why));
      }
      ++enabled_here;
    }
  }

  // The unused factory PDOs must stay disabled (bit 31 set).
  for (uint16_t unused : {0x1403u, 0x1803u}) {
    if (!set_pdo_valid(unused, unused == 0x1403 ? 0x500u + node : 0x480u + node, false)) {
      return fail(Str("could not disable unused ", ObjRef(unused, 1), "; abort: ", why));
    }
  }

  if (enabled_here > 0) {
    notes << "; enabled " << enabled_here << " PDO(s)";
  }

  if (!mismatches.empty()) {
    std::ostringstream os;
    for (const std::string& item : mismatches) {
      os << "\n        - " << item;
    }
    return fail(Str(mismatches.size(), " PDO mismatch(es) remain after",
                    repaired ? " repair" : " inspection", ":", os.str()));
  }

  if (detail) {
    *detail = Str(notes.str(), "; COB-IDs RPDO1 ", Hex(0x200u + node, 3), " RPDO2 ",
                  Hex(0x300u + node, 3), " RPDO3 ", Hex(0x400u + node, 3), " TPDO1 ",
                  Hex(0x180u + node, 3), " TPDO2 ", Hex(0x280u + node, 3), " TPDO3 ",
                  Hex(0x380u + node, 3), "; transmission 0x", Hex(plan.transmission_type, 2),
                  " (event driven); TPDO timers ", static_cast<int>(plan.tpdo1_event_timer_ms),
                  "/", static_cast<int>(plan.tpdo2_event_timer_ms), "/",
                  static_cast<int>(plan.tpdo3_event_timer_ms), " ms");
  }

  // ---- Phase D: always report what the DRIVE believes, not only what mismatched ----
  // Captured on the bus: the master transmits 0x201#060000 (CW=0x0006) and
  // 0x301#0F0088130000 (CW=0x000F, target 5000) correctly, and the drive transmits
  // 0x181/0x281/0x381 correctly, yet it acts on neither. This dump is what identifies
  // whether the drive's own RPDO COB-ID/mapping/transmission type agrees with ours.
  if (detail) {
    std::ostringstream os;
    os << "\n      ---- what the drive reports about its own PDOs ----\n";
    for (uint16_t comm : {0x1400u, 0x1401u, 0x1402u, 0x1403u, 0x1800u, 0x1801u, 0x1802u,
                          0x1803u}) {
      const bool rpdo = comm < 0x1800;
      const uint32_t cob = ReadOr<uint32_t>(comm, 1, 0u);
      const uint8_t type = ReadOr<uint8_t>(comm, 2, 0u);
      const uint16_t map_idx = static_cast<uint16_t>(comm + 0x200);
      const uint8_t count = ReadOr<uint8_t>(map_idx, 0, 0u);
      os << "      " << ObjRef(comm, 1) << " COB-ID=" << Hex(cob & ~kPdoDisabledBit, 4)
         << " valid=" << (((cob & kPdoDisabledBit) == 0) ? "1" : "0") << "  " << ObjRef(comm, 2)
         << "=" << Hex(type, 2) << " (trigger "
         << ((type & 0x03u) == 0x02u ? "RTR-only" : ((type & 0x03u) == 0x03u ? "event" : "sync"))
         << ")  " << ObjRef(map_idx, 0) << "=" << static_cast<int>(count);
      for (uint8_t e = 1; e <= count && e <= 8; ++e) {
        os << " " << Hex(ReadOr<uint32_t>(map_idx, e, 0u), 8);
      }
      if (!rpdo && count > 0) {
        os << "  " << ObjRef(comm, 5) << "=" << ReadOr<uint16_t>(comm, 5, 0u) << "ms";
      }
      os << '\n';
    }
    *detail += os.str();
  }
  return true;
}

// ---------------------------------------------------------------------------
// Stage S11 - Fault reset
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::StageFaultReset(const BringUpOptions& opt) {
  (void)opt;
  if (state_.load() != CiA402State::FAULT &&
      (statusword_.load() & statusword_bits::FAULT) == 0) {
    LogInfo(Stage::S11_FAULT_RESET, axis_tag_,
            Str("no latched fault (Statusword ", DecodeStatusword(statusword_.load()), ")"));
    return true;
  }

  LogWarn(Stage::S11_FAULT_RESET, axis_tag_,
          Str("drive reports FAULT: ", DecodeStatusword(statusword_.load())));

  // CiA 402: the rising edge of controlword bit 7 clears the fault.
  bool cleared = ApplyControlword(controlword_commands::FAULT_RESET,
                                  CiA402State::SWITCH_ON_DISABLED, std::chrono::milliseconds(500));
  if (!cleared) {
    LogWarn(Stage::S11_FAULT_RESET, axis_tag_,
            "controlword fault reset did not clear the fault; writing manufacturer 0x2006 = 1");
    std::string why;
    if (!TryWrite<uint8_t>(od::kDspClearAlarm, 0, uint8_t{1}, &why)) {
      LogError(Stage::S11_FAULT_RESET, axis_tag_, Str("0x2006 write failed: ", why));
    }
    USleep(200000);
    RefreshStatusword();
    cleared = (statusword_.load() & statusword_bits::FAULT) == 0;
  }

  // Park the controlword so the next transition starts from a clean edge.
  SendControlwordQuiet(controlword_commands::DISABLE_VOLTAGE);
  USleep(50000);
  RefreshStatusword();

  if (cleared) {
    LogInfo(Stage::S11_FAULT_RESET, axis_tag_,
            Str("fault cleared, Statusword now ", DecodeStatusword(statusword_.load())));
    return true;
  }

  const DriveSnapshot snap = ReadDriveSnapshotImpl();
  LogError(Stage::S11_FAULT_RESET, axis_tag_,
           Str("fault did not clear: ", DecodeStatusword(snap.statusword),
               FormatDriveSnapshot(snap)));
  return false;
}

// ---------------------------------------------------------------------------
// Liveness: CiA 301 heartbeat
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::SetHeartbeatProducerImpl(uint16_t period_ms) {
  if (period_ms == 0) return true;  // caller asked to leave the drive's own default

  std::string why;
  if (!TryWrite<uint16_t>(od::kProducerHeartbeatTime, 0, period_ms, &why)) {
    LogWarn(Stage::S08_IDENTITY, axis_tag_,
            Str("cannot set ", ObjRef(od::kProducerHeartbeatTime, 0), " = ", period_ms,
                " ms (", why, "). Liveness detection will be as slow as the drive's own "
                "default."));
    return false;
  }
  const uint16_t readback = ReadOr<uint16_t>(od::kProducerHeartbeatTime, 0, 0u);
  LogInfo(Stage::S08_IDENTITY, axis_tag_,
          Str(ObjRef(od::kProducerHeartbeatTime, 0), " = ", readback, " ms (the drive now "
              "broadcasts 0x700+node every ", readback, " ms)"));
  return readback == period_ms;
}

// ---------------------------------------------------------------------------
// Fault diagnosis and recovery
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::RecoverServo(const char* reason,
                                  std::chrono::milliseconds fault_timeout,
                                  std::chrono::milliseconds servo_timeout, FaultKind* cause) {
  if (cause) *cause = FaultKind::kNone;
  if (abort_.load()) return false;
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();
  // Written on the driver strand, read here once the future is ready.
  auto kind_seen = std::make_shared<std::atomic<int>>(static_cast<int>(FaultKind::kNone));

  Defer([this, reason, fault_timeout, servo_timeout, promise, kind_seen]() {
    try {
      LogInfo(Stage::S11_FAULT_RESET, axis_tag_,
              Str("automatic recovery requested (", reason ? reason : "unspecified", ")"));
      const DriveSnapshot before = ReadDriveSnapshotImpl();
      const FaultKind kind = ClassifyFault(before);
      kind_seen->store(static_cast<int>(kind));
      LogInfo(Stage::S11_FAULT_RESET, axis_tag_,
              Str("cause: ", fault_kind_to_string(kind), " - ", ExplainFaultKind(kind)));

      if (kind == FaultKind::kSto || kind == FaultKind::kLimit ||
          kind == FaultKind::kNoMainPower) {
        // These are hardware states. Retrying now would just spin, and for STO the
        // manual is explicit that only the safety circuit can clear it.
        LogWarn(Stage::S11_FAULT_RESET, axis_tag_,
                "not retrying: the cause is a hardware input that a person has to clear. "
                "Waiting for it to change.");
        promise->set_value(false);
        return;
      }

      // CiA 402: the rising edge of controlword bit 7 clears a latched fault, and
      // 0x2006 = 1 is the manufacturer's DSP-level clear.
      const bool faulted = (statusword_.load() & statusword_bits::FAULT) != 0;
      if (faulted) {
        bool cleared = ApplyControlword(controlword_commands::FAULT_RESET,
                                        CiA402State::SWITCH_ON_DISABLED, fault_timeout);
        if (!cleared) {
          std::string why;
          if (TryWrite<uint8_t>(od::kDspClearAlarm, 0, uint8_t{1}, &why)) {
            USleep(200000);
            RefreshStatusword();
            cleared = (statusword_.load() & statusword_bits::FAULT) == 0;
          } else {
            LogWarn(Stage::S11_FAULT_RESET, axis_tag_, Str("0x2006 write failed: ", why));
          }
        }
        if (!cleared) {
          LogWarn(Stage::S11_FAULT_RESET, axis_tag_,
                  "the fault did not clear; leaving the axis alone and waiting.");
          promise->set_value(false);
          return;
        }
        LogInfo(Stage::S11_FAULT_RESET, axis_tag_,
                Str("fault cleared, Statusword now ", DecodeStatusword(statusword_.load())));
      }

      SendControlwordQuiet(controlword_commands::DISABLE_VOLTAGE);
      USleep(50000);

      if (!EnableServoImpl(servo_timeout)) {
        LogWarn(Stage::S12_SERVO_ON, axis_tag_,
                Str("re-enable after recovery failed; still ",
                    cia402_state_to_string(state_.load())));
        promise->set_value(false);
        return;
      }
      LogInfo(Stage::S12_SERVO_ON, axis_tag_,
              Str("servo re-enabled by automatic recovery: ", DecodeStatusword(statusword_.load())));
      promise->set_value(true);
    } catch (const std::exception& ex) {
      LogWarn(Stage::S11_FAULT_RESET, axis_tag_, Str("automatic recovery failed: ", ex.what()));
      promise->set_value(false);
    }
  });

  const auto budget = fault_timeout + servo_timeout * 3 + std::chrono::milliseconds(3000);
  const bool ready = future.wait_for(budget) == std::future_status::ready;
  if (cause) *cause = static_cast<FaultKind>(kind_seen->load());
  if (ready) return future.get();
  LogWarn(Stage::S11_FAULT_RESET, axis_tag_, "automatic recovery timed out");
  return false;
}

// ---------------------------------------------------------------------------
// BringUp driver (stages S05..S11)
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::BringUp(DiagnosticReport& report, const BringUpOptions& options) {
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();
  const int64_t budget_ms = options.boot_timeout.count() * 5 + 8000;

  Defer([this, &report, options, promise]() {
    auto record = [this, &report](Stage stage, bool ok, const std::string& reason,
                                  const std::string& hint = std::string()) {
      if (ok) {
        report.Pass(stage, axis_tag_, reason);
      } else {
        report.Fail(stage, axis_tag_, reason, hint);
        LogError(stage, axis_tag_, Str("STAGE ", stage_code(stage), " (", stage_name(stage),
                                       ") FAILED | reason: ", reason,
                                       hint.empty() ? "" : Str(" | hint: ", hint)));
      }
    };

    try {
      LogBanner(Stage::S05_BOOTUP, axis_tag_);

      // ---------------- S05 BOOTUP ----------------
      {
        const bool ok = StageBootUp(options);
        record(Stage::S05_BOOTUP, ok,
               ok ? Str("Boot-up frame received on COB-ID ", Hex(0x700u + node_id_, 3))
                  : Str("no Boot-up frame within ", options.boot_timeout.count(), " ms"),
               ok ? std::string()
                  : "Power the drive and confirm 24 VDC on AUX. Check CAN_H/CAN_L, the GND "
                    "connection and that the front LED shows the node number rather than an "
                    "alarm code (manual section 6.2).");
        if (!ok) {
          promise->set_value(false);
          return;
        }
      }

      // Liveness right after boot-up: from here on a node that stops answering is
      // detectable, which is what lets the supervisor reconnect instead of hanging on a
      // dead axis. Not earlier: until OnBoot() lely's concise-DCF download owns this node's
      // SDO channel, so the write failed on whichever axis booted first ("Resource not
      // available: SDO connection").
      //
      // The drive's producer is written BEFORE the master's consumer is tightened: lely
      // re-arms the consumer on every heartbeat it receives (co_nmt_hb_recv), so a 300 ms
      // window armed while the drive still sends every 1000 ms would time out, and lely would
      // reset the node.
      const bool producer_ok = options.heartbeat_producer_ms == 0 ||
                               SetHeartbeatProducerImpl(options.heartbeat_producer_ms);
      if (!producer_ok) {
        LogWarn(Stage::S08_IDENTITY, axis_tag_,
                Str(ObjRef(od::kProducerHeartbeatTime, 0), " was not set to ",
                    options.heartbeat_producer_ms,
                    " ms; keeping the consumer time from the DCF (config/master.yaml) rather "
                    "than a window the drive's own period might not meet."));
      } else if (options.heartbeat_consumer_ms > 0) {
        heartbeat_consumer_ms_.store(options.heartbeat_consumer_ms);
        std::error_code ec;
        ConfigHeartbeat(std::chrono::milliseconds(options.heartbeat_consumer_ms), ec);
        if (ec) {
          LogWarn(Stage::S08_IDENTITY, axis_tag_,
                  Str("cannot install a heartbeat consumer: ", ec.message(),
                      ". Liveness detection is disabled for this node."));
          heartbeat_consumer_ms_.store(0);
        } else {
          LogInfo(Stage::S08_IDENTITY, axis_tag_,
                  Str("watching ", ObjRef(0x700, 0), "+node for this node every ",
                      options.heartbeat_consumer_ms, " ms"));
        }
      }

      // ---------------- S06 PREOP ----------------
      {
        const bool ok = StagePreOp(options);
        record(Stage::S06_PREOP, ok,
               ok ? Str("PRE-OPERATIONAL; SDO upload of ", ObjRef(od::kStatusword, 0),
                        " answered")
                  : Str("SDO upload timed out; the node did not answer on COB-ID ",
                        Hex(0x600u + node_id_, 3)),
               ok ? std::string()
                  : "The node sent Boot-up but ignored SDO. Check the bit rate, the 120 ohm "
                    "terminator (SW8=1 on the last device only) and that no second master owns "
                    "the bus.");
        if (!ok) {
          promise->set_value(false);
          return;
        }
      }

      // ---------------- S07 NMT START ----------------
      {
        const bool ok = StageNmtStart(options);
        record(Stage::S07_NMT_START, ok,
               ok ? "OPERATIONAL reached"
                  : Str("still not OPERATIONAL after ", options.boot_timeout.count(), " ms"),
               ok ? std::string()
                  : "NMT Start was not acknowledged. Verify the node reached PRE-OPERATIONAL and "
                    "that no other master is sending NMT commands on this bus.");
        if (!ok) {
          promise->set_value(false);
          return;
        }
      }

      // ---------------- S08 IDENTITY ----------------
      {
        const StageVerdict verdict = StageIdentity(options);
        record(Stage::S08_IDENTITY, verdict.ok, verdict.reason, verdict.hint);
        if (!verdict.ok) {
          promise->set_value(false);
          return;
        }
      }

      // ---------------- S09 PDO VERIFY ----------------
      {
        std::string detail;
        const bool ok = ConfigureAndVerifyPdos(options.pdo, &detail);
        // The verdict table truncates its Reason column, so emit the full PDO report here
        // as well; otherwise the drive's own view of its PDOs is never visible.
        if (ok) {
          LogInfo(Stage::S09_PDO_VERIFY, axis_tag_, detail);
        } else {
          LogError(Stage::S09_PDO_VERIFY, axis_tag_, detail);
        }
        record(Stage::S09_PDO_VERIFY, ok, ok ? detail : detail,
               ok ? std::string()
                  : "Program the PDOs in CiA 301 order: clear bit 31 of 0x140x:01/0x180x:01, set "
                    "0x160x:00 / 0x1A00x:00 = 0, write the mapping entries, set the transmission "
                    "type, then re-enable. The transmission type must NOT be 0xFE (RTR only).");
        if (!ok) {
          promise->set_value(false);
          return;
        }
      }

      // S10 is executed later by SetModeStaged(); declare it as not yet reached.
      report.Pending(Stage::S10_MODE_OF_OPERATION, axis_tag_);

      // ---------------- S11 FAULT RESET ----------------
      if (!options.fault_reset) {
        // A reconnect: the stop output may be holding the drives' E-STOP inputs, so a reset
        // could not succeed yet; RecoverServo() clears any fault before it re-enables.
        report.Skip(Stage::S11_FAULT_RESET, axis_tag_, "deferred to the servo-enable step");
      } else {
        const bool ok = StageFaultReset(options);
        record(Stage::S11_FAULT_RESET, ok,
               ok ? "fault state clear"
                  : "the drive refused to leave the FAULT state",
               ok ? std::string()
                  : "Read the drive LED code together with 0x603F and 0x200F. Non-resettable "
                    "faults (internal voltage, encoder signal error) need the encoder/motor "
                    "wiring checked and the drive power-cycled.");
        if (!ok) {
          promise->set_value(false);
          return;
        }
      }

      LogInfo(Stage::S11_FAULT_RESET, axis_tag_,
              "stages S05..S11 complete; ready for S10 (mode) and S12 (servo on)");
      promise->set_value(true);
    } catch (const std::exception& ex) {
      LogError(Stage::S06_PREOP, axis_tag_,
               Str("unhandled exception during bring-up: ", ex.what()));
      promise->set_value(false);
    }
  });

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(budget_ms);
  if (future.wait_until(deadline) == std::future_status::ready) {
    return future.get();
  }
  LogError(Stage::S05_BOOTUP, axis_tag_,
           Str("bring-up did not finish within ", budget_ms, " ms (event loop starved)"));
  return false;
}

// ---------------------------------------------------------------------------
// Stage S10 - Mode of operation
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::SetModeImpl(CiA402Mode mode, std::chrono::milliseconds timeout) {
  const int8_t want = static_cast<int8_t>(mode);
  LogInfo(Stage::S10_MODE_OF_OPERATION, axis_tag_,
          Str("writing 0x6060 Modes of operation = ", static_cast<int>(want), " (",
              cia402_mode_to_string(mode), ")"));

  std::string why;
  mode_of_operation_.store(want);
  if (!TryWrite<int8_t>(od::kModeOfOperation, 0, want, &why)) {
    LogError(Stage::S10_MODE_OF_OPERATION, axis_tag_, Str("SDO write 0x6060 failed: ", why));
    return false;
  }

  // Confirm with 0x6061 Modes of operation display, as CiA 402 requires.
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  int8_t display = -1;
  while (std::chrono::steady_clock::now() < deadline && !abort_.load()) {
    display = ReadOr<int8_t>(od::kModeOfOperationDisplay, 0, static_cast<int8_t>(-1));
    if (display == want) {
      LogInfo(Stage::S10_MODE_OF_OPERATION, axis_tag_,
              Str("0x6061 confirms mode ", static_cast<int>(display), " (",
                  cia402_mode_to_string(static_cast<CiA402Mode>(display)), ")"));
      return true;
    }
    USleep(20000);
  }

  LogError(Stage::S10_MODE_OF_OPERATION, axis_tag_,
           Str("0x6061 Modes of operation display = ", static_cast<int>(display),
               " after writing 0x6060 = ", static_cast<int>(want),
               " - the drive did not accept the requested mode"));
  return false;
}

bool MbdvAxisDriver::SetModeStaged(DiagnosticReport& report, CiA402Mode mode,
                                   std::chrono::milliseconds timeout) {
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();

  Defer([this, &report, mode, timeout, promise]() {
    try {
      const bool ok = SetModeImpl(mode, timeout);
      if (ok) {
        report.Pass(Stage::S10_MODE_OF_OPERATION, axis_tag_,
                    Str("0x6060 = ", static_cast<int>(mode), " confirmed by 0x6061"));
      } else {
        const int8_t display =
            ReadOr<int8_t>(od::kModeOfOperationDisplay, 0, static_cast<int8_t>(-1));
        const uint32_t p1_00 = ReadOr<uint32_t>(od::kControlMode, 0, 0u);
        std::ostringstream why;
        why << "0x6061 reports " << static_cast<int>(display) << " after writing 0x6060 = "
            << static_cast<int>(mode);
        std::ostringstream hint;
        hint << "The MBDV ignores 0x6060 unless P1-00 (object 0x2A30) selects a matching drive "
                "control mode: PP = 21, PV = 15, TQ = 1. 0x2A30 currently reads "
             << static_cast<int>(p1_00) << " (" << drive_control_mode_to_string(p1_00)
             << "). Write the matching value to 0x2A30 and repeat.";
        report.Fail(Stage::S10_MODE_OF_OPERATION, axis_tag_, why.str(), hint.str());
        LogError(Stage::S10_MODE_OF_OPERATION, axis_tag_,
                 Str("reason: ", why.str(), " | hint: ", hint.str()));
      }
      promise->set_value(ok);
    } catch (const std::exception& ex) {
      report.Fail(Stage::S10_MODE_OF_OPERATION, axis_tag_, ex.what());
      promise->set_value(false);
    }
  });

  if (future.wait_for(timeout + std::chrono::milliseconds(2000)) == std::future_status::ready) {
    return future.get();
  }
  report.Fail(Stage::S10_MODE_OF_OPERATION, axis_tag_, "mode change timed out");
  return false;
}

// ---------------------------------------------------------------------------
// Stage S12 - Servo ON
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::WaitForState(CiA402State target, std::chrono::milliseconds timeout) {
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  int refresh_countdown = 0;
  while (std::chrono::steady_clock::now() < deadline && !abort_.load()) {
    if (state_.load() == target) return true;
    if (state_.load() == CiA402State::FAULT ||
        state_.load() == CiA402State::FAULT_REACTION_ACTIVE) {
      return false;
    }
    USleep(10000);
    // Every ~200 ms fall back to an SDO read so a silent TPDO cannot stall the
    // state machine forever.
    if (++refresh_countdown >= 20) {
      refresh_countdown = 0;
      RefreshStatusword();
    }
  }
  return state_.load() == target;
}

bool MbdvAxisDriver::ApplyControlword(uint16_t controlword, CiA402State expect,
                                      std::chrono::milliseconds timeout) {
  LogDebug(Stage::S12_SERVO_ON, axis_tag_,
           Str("Controlword = ", DecodeControlword(controlword)));

  try {
    if (!SendRpdo(1, controlword, false, mode_of_operation_.load())) {
      throw std::runtime_error("RPDO1 transmit failed for node " + std::to_string(node_id_));
    }
  } catch (const std::exception& ex) {
    LogError(Stage::S12_SERVO_ON, axis_tag_, Str("RPDO1 transmission failed: ", ex.what()));
    return false;
  }

  if (WaitForState(expect, timeout)) return true;

  // Reading 0x6040 back is the only way to tell "the frame never arrived" from "the
  // drive applied it and refused to act": an unchanged value proves the RPDO did not
  // reach the object dictionary at all.
  const uint16_t readback = ReadOr<uint16_t>(od::kControlword, 0, 0xFFFFu);
  const bool applied = readback != 0xFFFFu && readback == controlword;
  LogError(Stage::S12_SERVO_ON, axis_tag_,
           Str("RPDO1 controlword produced no transition within ", timeout.count(),
               " ms (still ", cia402_state_to_string(state_.load()), "). Reading ",
               ObjRef(od::kControlword, 0), " back gives ", Hex(readback, 4), " versus the sent ",
               Hex(controlword, 4), " -> ",
               applied
                   ? "the drive DID store the command, so it refuses the transition on its own "
                     "side (check the digital inputs, STO and 0x603F/0x200F)"
                   : "the drive did NOT store the command, so the RPDO frame is not being "
                     "applied at all - check the 0x1400/0x1600 mapping and the CAN wiring"));
  return false;
}

bool MbdvAxisDriver::EnableServoImpl(std::chrono::milliseconds timeout) {
  const Stage stage = Stage::S12_SERVO_ON;

  if (statusword_count_.load() == 0) {
    RefreshStatusword();
  }

  LogInfo(stage, axis_tag_,
          Str("current status ", DecodeStatusword(statusword_.load()), " | telemetry frames=",
              statusword_count_.load(), TelemetryStaleNote()));

  // The drive keeps 0x60FF from before the servo was last switched off - a reconnect or a
  // fault recovery re-enables it on whatever was commanded then - so zero it over SDO
  // first: Enable Operation must never start the motor on an old setpoint, and an axis
  // found still enabled after a reconnect must not keep running on one either.
  {
    std::string why;
    if (!TryWrite<int32_t>(od::kTargetVelocity, 0, int32_t{0}, &why)) {
      LogWarn(stage, axis_tag_,
              Str("could not zero ", ObjRef(od::kTargetVelocity, 0), " before enabling: ", why));
    }
    last_target_velocity_.store(0);
  }

  if (state_.load() == CiA402State::OPERATION_ENABLED) {
    LogInfo(stage, axis_tag_, "already in OPERATION_ENABLED, nothing to do");
    return true;
  }
  halted_.store(false);  // a fresh enable starts without StopNow()'s Halt

  struct Transition {
    uint16_t controlword;
    const char* label;
    CiA402State expect;
  };
  const Transition kChain[] = {
      {controlword_commands::SHUTDOWN, "Shutdown", CiA402State::READY_TO_SWITCH_ON},
      {controlword_commands::SWITCH_ON, "Switch ON", CiA402State::SWITCHED_ON},
      {controlword_commands::ENABLE_OPERATION, "Enable Operation", CiA402State::OPERATION_ENABLED},
  };

  int step_no = 0;
  for (const Transition& step : kChain) {
    ++step_no;
    LogInfo(stage, axis_tag_,
            Str("S12.", static_cast<int>(step_no), " ", step.label, ": CW=",
                Hex(step.controlword, 4), " -> wait for ",
                cia402_state_to_string(step.expect)));

    if (!ApplyControlword(step.controlword, step.expect, timeout)) {
      const DriveSnapshot snap = ReadDriveSnapshotImpl();
      LogError(stage, axis_tag_,
               Str("S12.", static_cast<int>(step_no), " ", step.label,
                   " did not complete; drive is in ", cia402_state_to_string(state_.load()),
                   " | Statusword ", DecodeStatusword(snap.statusword),
                   " | TPDO frames=", statusword_count_.load(),
                   FormatDriveSnapshot(snap)));
      return false;
    }
    LogInfo(stage, axis_tag_,
            Str("S12.", static_cast<int>(step_no), " ", step.label, " reached -> ",
                DecodeStatusword(statusword_.load())));
  }

  LogInfo(stage, axis_tag_,
          Str("Servo ON complete: ", DecodeStatusword(statusword_.load()),
              " | TPDO frames=", statusword_count_.load(),
              " | SDO statusword reads=", statusword_sdo_reads_.load()));
  return true;
}

bool MbdvAxisDriver::EnableServoStaged(DiagnosticReport& report, std::chrono::milliseconds timeout) {
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();

  Defer([this, &report, timeout, promise]() {
    try {
      const bool ok = EnableServoImpl(timeout);
      if (ok) {
        report.Pass(Stage::S12_SERVO_ON, axis_tag_,
                    Str("OPERATION_ENABLED, Statusword ", Hex(statusword_.load(), 4)));
      } else {
        const DriveSnapshot snap = ReadDriveSnapshotImpl();
        const std::string reason = Str("stuck in ", cia402_state_to_string(state_.load()),
                                       "; Statusword ", Hex(snap.statusword, 4),
                                       "; telemetry frames=", statusword_count_.load());
        std::string hint;
        if (state_.load() == CiA402State::FAULT || snap.error_code != 0 || snap.dsp_alarm != 0) {
          hint = "The drive raised a fault. Decode 0x603F / 0x200F above and read the front "
                 "LED. Frequent causes on this drive: main supply absent on V+/V- "
                 "(manual 4.3 requires 24 ~ 60 VDC there; the 24V/GND auxiliary supply "
                 "alone is not enough), the STO circuit open, the encoder cable unplugged "
                 "(manual 6.2 flags r09 for encoder wiring), or a CW/CCW limit input "
                 "asserted (factory defaults: X1 = CCW-LMT, X2 = CW-LMT, X4 = E-STOP).";
        } else if (state_.load() == CiA402State::SWITCH_ON_DISABLED) {
          hint = "The drive never left Switch On Disabled, so the Enable Voltage command "
                 "(controlword bit 1) was not accepted. The main voltage IS present "
                 "(Statusword bit 4 = 1), so check the digital inputs listed above: a CW "
                 "limit, CCW limit or E-STOP input that reads Closed will block enable. "
                 "Factory defaults are X1 = CCW-LMT, X2 = CW-LMT, X3 = HOM-SW, X4 = E-STOP "
                 "(manual 7.1.1.2) - leave them open, or reassign them to GPIN via "
                 "P5-00..P5-03. Also close the STO circuit (manual 4.11).";
        } else if (!is_operational_.load()) {
          hint = "The node is not in NMT OPERATIONAL. Stage S07 must pass before the CiA 402 "
                 "state machine can run.";
        } else if (statusword_count_.load() == 0) {
          hint = "No TPDO1 Statusword ever arrived, so stage S09 PDO verification did not hold. "
                 "Check TPDO1 (0x180+node) mapping, event timer and transmission type.";
        } else {
          hint = "The node is OPERATIONAL and the Statusword flows, but the controlword "
                 "transitions did not take effect. Confirm RPDO1 (0x200+node) carries 0x6040 "
                 "with an event-driven transmission type (stage S09).";
        }
        report.Fail(Stage::S12_SERVO_ON, axis_tag_, reason, hint);
        LogError(Stage::S12_SERVO_ON, axis_tag_,
                 Str("reason: ", reason, FormatDriveSnapshot(snap)));
      }
      promise->set_value(ok);
    } catch (const std::exception& ex) {
      report.Fail(Stage::S12_SERVO_ON, axis_tag_, ex.what());
      promise->set_value(false);
    }
  });

  if (future.wait_for(timeout * 4 + std::chrono::milliseconds(3000)) ==
      std::future_status::ready) {
    return future.get();
  }
  report.Fail(Stage::S12_SERVO_ON, axis_tag_, "servo-on sequence timed out");
  return false;
}

// ---------------------------------------------------------------------------
// Stage S15 - Servo OFF
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::DisableServoImpl(std::chrono::milliseconds timeout) {
  const Stage stage = Stage::S15_SERVO_OFF;
  if (state_.load() != CiA402State::OPERATION_ENABLED &&
      state_.load() != CiA402State::SWITCHED_ON &&
      state_.load() != CiA402State::READY_TO_SWITCH_ON) {
    LogInfo(stage, axis_tag_,
            Str("already out of Operation Enabled (", cia402_state_to_string(state_.load()),
                "), nothing to do"));
    return true;
  }

  // Zero the velocity first so the drive is not commanded while decelerating.
  try {
    SendRpdo(3, controlword_commands::ENABLE_OPERATION, true, 0);
  } catch (...) {
  }
  // RPDO3 carries the controlword as well (0x000F). Sent back to back with RPDO1's 0x0007,
  // the drive could apply them in either order; with 0x000F last the axis stays enabled -
  // AX2 failed S15 that way on the bench, 0x6040 reading back 000F. Waiting for the ramp
  // (at most 1 s) keeps the two frames apart and the stop controlled.
  for (int i = 0; i < 100 && !abort_.load() && std::abs(actual_velocity_.load()) > 50; ++i) {
    USleep(10000);
  }
  USleep(20000);

  bool ok = ApplyControlword(controlword_commands::DISABLE_OPERATION, CiA402State::SWITCHED_ON,
                             timeout);
  if (!ok) {
    ok = WaitForState(CiA402State::SWITCH_ON_DISABLED, timeout);
  }
  if (!ok) {
    LogWarn(stage, axis_tag_,
            "Disable Operation did not settle; forcing Disable Voltage (CW=0x0000)");
    SendControlwordQuiet(controlword_commands::DISABLE_VOLTAGE);
    USleep(100000);
    RefreshStatusword();
  }
  return state_.load() != CiA402State::OPERATION_ENABLED;
}

bool MbdvAxisDriver::DisableServoStaged(DiagnosticReport& report, std::chrono::milliseconds timeout) {
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();

  Defer([this, &report, timeout, promise]() {
    try {
      const bool ok = DisableServoImpl(timeout);
      if (ok) {
        report.Pass(Stage::S15_SERVO_OFF, axis_tag_,
                    Str("servo off, Statusword ", Hex(statusword_.load(), 4)));
      } else {
        report.Fail(Stage::S15_SERVO_OFF, axis_tag_,
                    Str("still ", cia402_state_to_string(state_.load()),
                        " after Disable Voltage"),
                    "If the axis refuses to disable, a fault is latched (see 0x603F / 0x200F) or "
                    "the node left NMT OPERATIONAL. Power-cycling is the last resort.");
      }
      promise->set_value(ok);
    } catch (const std::exception& ex) {
      report.Fail(Stage::S15_SERVO_OFF, axis_tag_, ex.what());
      promise->set_value(false);
    }
  });

  if (future.wait_for(timeout * 3 + std::chrono::milliseconds(2000)) ==
      std::future_status::ready) {
    return future.get();
  }
  report.Fail(Stage::S15_SERVO_OFF, axis_tag_, "servo-off sequence timed out");
  return false;
}

// ---------------------------------------------------------------------------
// Stages S13 / S14 - motion
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::MoveToPositionImpl(int32_t target, std::chrono::milliseconds timeout) {
  const Stage cmd = Stage::S13_MOTION_COMMAND;
  const Stage track = Stage::S14_MOTION_TRACKING;

  if (state_.load() != CiA402State::OPERATION_ENABLED) {
    LogError(cmd, axis_tag_,
             Str("cannot command motion: state is ", cia402_state_to_string(state_.load()),
                 " but Operation Enabled is required"));
    return false;
  }
  // An explicit position command is what releases a Halt set by StopNow().
  halted_.store(false);

  // Statusword bit 10 (Target reached) is often ALREADY set on an idle drive, so clearing
  // the cached flag is not enough - it is re-armed from a fresh Statusword read below.
  target_reached_.store(false);
  RefreshStatusword();
  const bool target_reached_before = target_reached_.load();
  const int32_t start_position = actual_position_.load();
  LogInfo(cmd, axis_tag_,
          Str("RPDO2 target position = ", target, " (", Sgn(target), " counts); start position = ",
              start_position, "; Statusword bit 10 before the move = ",
              target_reached_before ? "ALREADY SET (will be ignored)" : "clear"));

  try {
    // bit 4 new setpoint + bit 5 change immediately, keeping bit 3 enable operation.
    const uint16_t cw = static_cast<uint16_t>(controlword_commands::ENABLE_OPERATION |
                                              controlword_bits::NEW_SET_POINT |
                                              controlword_bits::CHANGE_SET_IMMEDIATELY);
    if (!SendRpdo(2, cw, true, target)) {
      throw std::runtime_error("RPDO2 transmit failed for node " + std::to_string(node_id_));
    }
  } catch (const std::exception& ex) {
    LogError(cmd, axis_tag_, Str("RPDO2 transmission failed: ", ex.what()));
    return false;
  }

  // Release the new-setpoint edge so later cycles are not seen as new setpoints.
  USleep(20000);
  try {
    SendRpdo(2, controlword_commands::ENABLE_OPERATION, true, target);
  } catch (...) {
  }

  // ---- S14: wait for Target Reached, abort early on FAULT ----
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  int last_position = actual_position_.load();
  int refresh_countdown = 0;
  // Bit 10 was already set before the command, so it proves nothing on its own: require the
  // position to have moved as well, otherwise report "did not move" instead of a false pass.
  const bool require_motion = target_reached_before || (target == start_position);
  if (require_motion) {
    LogWarn(track, axis_tag_,
            Str("Statusword bit 10 was already set and/or the target equals the current "
                "position, so Target Reached alone is not accepted as success; the position "
                "must also change"));
  }
  while (std::chrono::steady_clock::now() < deadline) {
    if (target_reached_.load() && (!require_motion || actual_position_.load() != start_position)) {
      LogInfo(track, axis_tag_,
              Str("Target Reached (Statusword bit 10); position = ", actual_position_.load(),
                  ", following error = ", following_error_.load()));
      return true;
    }
    if (state_.load() == CiA402State::FAULT ||
        state_.load() == CiA402State::FAULT_REACTION_ACTIVE) {
      const DriveSnapshot snap = ReadDriveSnapshotImpl();
      LogError(track, axis_tag_,
               Str("drive faulted during the move: ", DecodeStatusword(snap.statusword),
                   FormatDriveSnapshot(snap)));
      return false;
    }
    USleep(10000);
    last_position = actual_position_.load();
    if (++refresh_countdown >= 20) {
      refresh_countdown = 0;
      RefreshStatusword();
      last_position = actual_position_.load();
    }
  }

  LogError(track, axis_tag_,
           Str("Target Reached never set within ", timeout.count(),
               " ms; position stalled at ", last_position, " (target ", target, ")"));
  return false;
}

bool MbdvAxisDriver::MoveToPositionStaged(DiagnosticReport& report, int32_t target,
                                          std::chrono::milliseconds timeout) {
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();

  Defer([this, &report, target, timeout, promise]() {
    try {
      const bool ok = MoveToPositionImpl(target, timeout);
      if (ok) {
        report.Pass(Stage::S13_MOTION_COMMAND, axis_tag_, Str("target ", target, " written"));
        report.Pass(Stage::S14_MOTION_TRACKING, axis_tag_,
                    Str("Target Reached at position ", actual_position_.load()));
      } else {
        // Find() with an explicit tag, otherwise this always reports "not found"
        // and would wrongly mark a command that was accepted as failed.
        if (!report.Find(Stage::S13_MOTION_COMMAND, axis_tag_)) {
          report.Fail(Stage::S13_MOTION_COMMAND, axis_tag_,
                      "the target position was not accepted",
                      "Verify the axis is in PP mode (stage S10) and that RPDO2 (0x300+node) "
                      "carries 0x6040 + 0x607A with an event-driven transmission type.");
        }
        report.Fail(Stage::S14_MOTION_TRACKING, axis_tag_,
                    Str("no Target Reached within ", timeout.count(),
                        " ms; actual position ", actual_position_.load()),
                    "Common causes: the no-load trial run has not been done yet, a mechanical "
                    "end stop or CW/CCW limit is active, the position error limit P3-04 is too "
                    "small, the torque limit P1-06 is too low, or encoder feedback is absent "
                    "(compare 0x6064 against the commanded 0x607A).");
      }
      promise->set_value(ok);
    } catch (const std::exception& ex) {
      report.Fail(Stage::S13_MOTION_COMMAND, axis_tag_, ex.what());
      promise->set_value(false);
    }
  });

  if (future.wait_for(timeout + std::chrono::milliseconds(3000)) == std::future_status::ready) {
    return future.get();
  }
  report.Fail(Stage::S14_MOTION_TRACKING, axis_tag_, "motion command timed out");
  return false;
}

bool MbdvAxisDriver::SetVelocityImpl(int32_t target, std::chrono::milliseconds settle) {
  const Stage cmd = Stage::S13_MOTION_COMMAND;
  const Stage track = Stage::S14_MOTION_TRACKING;

  if (state_.load() != CiA402State::OPERATION_ENABLED) {
    LogError(cmd, axis_tag_,
             Str("cannot command velocity: state is ", cia402_state_to_string(state_.load()),
                 " but Operation Enabled is required"));
    return false;
  }

  // RefreshControlword() re-sends last_target_velocity_ at 20 Hz; without this it would
  // overwrite the staged setpoint with the previous one (0 after enable in --test-velocity).
  last_target_velocity_.store(target);
  LogInfo(cmd, axis_tag_,
          Str("RPDO3 target velocity = ", target, " counts/s; actual = ", actual_velocity_.load()));
  try {
    if (!SendRpdo(3, controlword_commands::ENABLE_OPERATION, true, target)) {
      throw std::runtime_error("RPDO3 transmit failed for node " + std::to_string(node_id_));
    }
  } catch (const std::exception& ex) {
    LogError(cmd, axis_tag_, Str("RPDO3 transmission failed: ", ex.what()));
    return false;
  }

  // Let the loop ramp, then verify the feedback is heading the right way.
  USleep(static_cast<uint64_t>(settle.count()) * 1000);
  RefreshStatusword();

  const int32_t actual = actual_velocity_.load();
  const int64_t error = static_cast<int64_t>(actual) - static_cast<int64_t>(target);
  const int64_t tolerance =
      std::max<int64_t>(std::llabs(static_cast<long long>(target)) / 4 + 100, 200);

  if (std::llabs(static_cast<long long>(error)) <= tolerance) {
    LogInfo(track, axis_tag_,
            Str("velocity tracking OK: target ", target, ", actual ", actual, " counts/s"));
    return true;
  }

  LogError(track, axis_tag_,
           Str("velocity did not track: target ", target, ", actual ", actual,
               " counts/s (error ", error, ")"));
  return false;
}

bool MbdvAxisDriver::SetVelocityStaged(DiagnosticReport& report, int32_t target,
                                       std::chrono::milliseconds settle) {
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();

  Defer([this, &report, target, settle, promise]() {
    try {
      const bool ok = SetVelocityImpl(target, settle);
      if (ok) {
        report.Pass(Stage::S13_MOTION_COMMAND, axis_tag_, Str("target ", target, " counts/s"));
        report.Pass(Stage::S14_MOTION_TRACKING, axis_tag_,
                    Str("actual ", actual_velocity_.load(), " counts/s"));
      } else {
        report.Fail(Stage::S13_MOTION_COMMAND, axis_tag_, "the target velocity was not accepted",
                    "Verify PV mode (stage S10) and that RPDO3 (0x400+node) carries 0x6040 + "
                    "0x60FF with an event-driven transmission type.");
        report.Fail(Stage::S14_MOTION_TRACKING, axis_tag_,
                    Str("actual ", actual_velocity_.load(), " counts/s vs target ", target),
                    "The motor may be mechanically blocked, the torque limit P1-06 too low, the "
                    "load too heavy, or a limit switch input asserted.");
      }
      promise->set_value(ok);
    } catch (const std::exception& ex) {
      report.Fail(Stage::S13_MOTION_COMMAND, axis_tag_, ex.what());
      promise->set_value(false);
    }
  });

  if (future.wait_for(settle + std::chrono::milliseconds(3000)) == std::future_status::ready) {
    return future.get();
  }
  report.Fail(Stage::S14_MOTION_TRACKING, axis_tag_, "velocity command timed out");
  return false;
}

// ---------------------------------------------------------------------------
// Continuous alarm watch
// ---------------------------------------------------------------------------

void MbdvAxisDriver::StartAlarmWatch(std::chrono::milliseconds period) {
  if (period.count() <= 0) return;
  if (alarm_watch_running_.exchange(true)) return;
  watched_alarm_.store(0xFFFFFFFFu);
  watched_error_reg_.store(0xFFFFFFFFu);

  Defer([this, period]() {
    LogInfo(Stage::S14_MOTION_TRACKING, axis_tag_,
            Str("alarm watch active: polling ", ObjRef(od::kDspAlarmCode, 0), " and ",
                ObjRef(od::kErrorRegister, 0), " every ", period.count(),
                " ms; every change is logged with a timestamp"));
    while (alarm_watch_running_.load()) {
      try {
        const uint32_t alarm = ReadOr<uint32_t>(od::kDspAlarmCode, 0, 0xFFFFFFFFu);
        const uint32_t error_reg = ReadOr<uint8_t>(od::kErrorRegister, 0, 0xFFu);
        if (alarm != watched_alarm_.exchange(alarm)) {
          dsp_alarm_.store(alarm);
          const uint16_t error_code = ReadOr<uint16_t>(od::kErrorCode, 0, 0u);
          error_code_.store(error_code);
          const std::string text =
              Str("ALARM CHANGE -> ", DecodeDspAlarmCode(alarm), " | ",
                  DecodeErrorRegister(static_cast<uint8_t>(error_reg)), " | ",
                  DecodeErrorCode402(error_code));
          if (alarm == 0) {
            LogInfo(Stage::S14_MOTION_TRACKING, axis_tag_, text);
          } else {
            LogError(Stage::S14_MOTION_TRACKING, axis_tag_, text);
          }
        }
        if (error_reg != watched_error_reg_.exchange(error_reg)) {
          error_register_.store(error_reg);
          if (error_reg != 0) {
            LogWarn(Stage::S14_MOTION_TRACKING, axis_tag_,
                    Str("error register changed -> ", DecodeErrorRegister(
                                                     static_cast<uint8_t>(error_reg))));
          }
        }
      } catch (const std::exception&) {
        // A transient SDO failure is not itself a finding; the next tick retries.
      }
      USleep(static_cast<uint64_t>(period.count()) * 1000);
    }
    LogInfo(Stage::S14_MOTION_TRACKING, axis_tag_, "alarm watch stopped");
  });
}

void MbdvAxisDriver::StopAlarmWatch() { alarm_watch_running_.store(false); }

// ---------------------------------------------------------------------------
// Diagnostics snapshot
// ---------------------------------------------------------------------------

DriveSnapshot MbdvAxisDriver::ReadDriveSnapshotImpl() {
  DriveSnapshot snap;
  snap.statusword = ReadOr<uint16_t>(od::kStatusword, 0, statusword_.load());
  snap.error_code = ReadOr<uint16_t>(od::kErrorCode, 0, 0u);
  snap.error_register = ReadOr<uint8_t>(od::kErrorRegister, 0, 0u);
  snap.dsp_alarm = ReadOr<uint32_t>(od::kDspAlarmCode, 0, 0u);
  snap.dsp_status = ReadOr<uint32_t>(od::kDspStatusCode, 0, 0u);
  snap.sub_alarm = ReadOr<uint32_t>(od::kSubAlarmCode, 0, 0u);
  snap.dc_bus_raw = ReadOr<uint16_t>(od::kDcBusVoltage, 0, 0u);
  snap.current_actual = ReadOr<int16_t>(od::kCurrentActual, 0, 0);
  snap.following_error = ReadOr<int32_t>(od::kFollowingError, 0, 0);
  // Digital inputs, plus the function each input is assigned to (manual 7.1).
  snap.watchdog_enable = ReadOr<uint32_t>(od::kCommWatchdog, od::kCommWatchdogEnable, 0u);
  snap.watchdog_status = ReadOr<uint32_t>(od::kCommWatchdog, od::kCommWatchdogStatus, 0u);
  snap.watchdog_timeout_ms = ReadOr<uint32_t>(od::kCommWatchdog, od::kCommWatchdogTimeout, 0u);
  snap.watchdog_trigger = ReadOr<uint32_t>(od::kCommWatchdog, od::kCommWatchdogTrigger, 0u);
  // Motion limits: the reason a drive can sit in Operation Enabled forever while
  // ignoring every target position.
  {
    const uint32_t vmax = ReadOr<uint32_t>(od::kMaxProfileSpeed, 0, 0u);
    const uint32_t acc = ReadOr<uint32_t>(od::kProfileAcceleration, 0, 0u);
    const uint32_t dec = ReadOr<uint32_t>(od::kProfileDeceleration, 0, 0u);
    const int32_t lo = ReadOr<int32_t>(od::kSoftwarePositionLimit, 1, 0);
    const int32_t hi = ReadOr<int32_t>(od::kSoftwarePositionLimit, 2, 0);
    snap.max_profile_speed = vmax;
    snap.profile_accel = acc;
    snap.profile_decel = dec;
    snap.position_limit_min = lo;
    snap.position_limit_max = hi;
    snap.motion_limits_valid = true;
  }
  snap.inputs.raw = ReadOr<uint32_t>(od::kDigitalInputs, 0, 0u);
  for (uint8_t i = 0; i < 4; ++i) {
    snap.inputs.function[i] = ReadOr<uint32_t>(od::kInputConfig, static_cast<uint8_t>(i + 1), 0u);
  }
  return snap;
}

DriveSnapshot MbdvAxisDriver::ReadDriveSnapshot() {
  auto promise = std::make_shared<std::promise<DriveSnapshot>>();
  auto future = promise->get_future();
  Defer([this, promise]() {
    try {
      promise->set_value(ReadDriveSnapshotImpl());
    } catch (...) {
      promise->set_value(DriveSnapshot{});
    }
  });
  if (future.wait_for(std::chrono::milliseconds(3000)) == std::future_status::ready) {
    return future.get();
  }
  return DriveSnapshot{};
}

// ---------------------------------------------------------------------------
// Unstaged helpers
// ---------------------------------------------------------------------------

void MbdvAxisDriver::SendControlwordQuiet(uint16_t controlword) {
  // Sent at once: every caller already runs on this driver's strand, where a Defer() queued
  // the frame behind the very task waiting for its effect. S15's forced Disable Voltage
  // therefore went out only after the stage had failed, and after a recovery the parked
  // 0x0000 would land after the re-enable and switch the axis back off.
  try {
    SendRpdo(1, controlword, false, CurrentModeForRpdo1());
  } catch (...) {
  }
}

void MbdvAxisDriver::SetTargetVelocity(int32_t target_velocity) {
  // Checked before posting: at 200 Hz most cycles do not change the setpoint, and a
  // queued task per cycle would fill the fiber strand for no benefit.
  if (target_velocity == last_target_velocity_.load()) return;
  last_target_velocity_.store(target_velocity);

  Defer([this, target_velocity]() {
    // A newer setpoint - or a stop from StopNow()/ZeroVelocityNow(), which bypass the strand
    // - may have been issued after this task was queued; sending this one would undo it.
    if (last_target_velocity_.load() != target_velocity) return;
    LogDebug(Stage::S13_MOTION_COMMAND, axis_tag_,
             Str("target velocity = ", target_velocity, " counts/s"));
    try {
      // RPDO3 only: it carries 0x60FF alongside the controlword, one frame at 200 Hz.
      if (!SendRpdo(3, controlword_commands::ENABLE_OPERATION, true, target_velocity)) {
        throw std::runtime_error("RPDO3 transmit failed for node " + std::to_string(node_id_));
      }
    } catch (const std::exception& ex) {
      LogError(Stage::S13_MOTION_COMMAND, axis_tag_,
               Str("RPDO3 transmission failed: ", ex.what()));
    }
  });
}

void MbdvAxisDriver::RefreshControlword() {
  Defer([this]() {
    try {
      if (mode_of_operation_.load() == static_cast<int8_t>(CiA402Mode::PROFILE_VELOCITY)) {
        // RPDO3 carries the Controlword AND 0x60FF, so the same frame also re-sends the
        // current setpoint. SetTargetVelocity() only transmits on a change, so without this a
        // single lost frame - a lost stop included - would stand until the next change.
        int32_t velocity = last_target_velocity_.load();
        if (velocity == kNoSetpoint) velocity = 0;
        SendRpdo(3, controlword_commands::ENABLE_OPERATION, true, velocity);
      } else {
        uint16_t cw = controlword_commands::ENABLE_OPERATION;
        if (halted_.load()) cw |= controlword_bits::HALT;  // keep StopNow()'s Halt latched
        SendRpdo(1, cw, false, CurrentModeForRpdo1());
      }
    } catch (...) {
    }
  });
}

bool MbdvAxisDriver::SetDriveControlModeImpl(uint32_t p1_00_value) {
  std::string why;
  if (!TryWrite<uint32_t>(od::kControlMode, 0, p1_00_value, &why)) {
    LogError(Stage::S10_MODE_OF_OPERATION, axis_tag_,
             Str("SDO write 0x2A30 = ", static_cast<int>(p1_00_value), " failed: ", why));
    return false;
  }
  const uint32_t readback = ReadOr<uint32_t>(od::kControlMode, 0, 0u);
  LogInfo(Stage::S10_MODE_OF_OPERATION, axis_tag_,
          Str("0x2A30 read back ", static_cast<int>(readback), " -> ",
              drive_control_mode_to_string(readback)));
  if (readback != p1_00_value) {
    LogWarn(Stage::S10_MODE_OF_OPERATION, axis_tag_,
            "NOT saving parameters (0x1010:01): a stored-parameter write that aborts part way "
            "(observed as SDO abort 0x08000020) can leave the block inconsistent. The value "
            "stays valid until the next power cycle; set P1-00 once in Luna to persist it.");
    return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// lely callbacks
// ---------------------------------------------------------------------------

void MbdvAxisDriver::OnBoot(lely::canopen::NmtState st, char es, const std::string& what) noexcept {
  boot_seen_.store(true);
  boot_in_progress_.store(false);  // lely's boot-slave process is over, whatever the outcome
  {
    std::lock_guard<std::mutex> lock(time_mutex_);
    last_nmt_time_ = std::chrono::steady_clock::now();
  }
  if (es) {
    LogError(Stage::S05_BOOTUP, axis_tag_,
             Str("boot-up / SDO configuration FAILED with error ", static_cast<int>(es), ": ",
                 what, " - this is where a DCF/EDS mismatch surfaces"));
  } else {
    LogInfo(Stage::S05_BOOTUP, axis_tag_,
            Str("boot-up + concise-DCF download completed (NMT state ",
                static_cast<int>(st), ")"));
  }
}

void MbdvAxisDriver::OnState(lely::canopen::NmtState st) noexcept {
  {
    std::lock_guard<std::mutex> lock(time_mutex_);
    last_nmt_time_ = std::chrono::steady_clock::now();
    if (st == lely::canopen::NmtState::BOOTUP) boot_start_time_ = last_nmt_time_;
  }
  // A boot-up frame starts lely's boot-slave process (BasicMaster::OnState() has already
  // cancelled our pending SDO requests for it); OnBoot() marks its end.
  if (st == lely::canopen::NmtState::BOOTUP) boot_in_progress_.store(true);
  is_operational_.store(st == lely::canopen::NmtState::START);

  const char* name = "UNKNOWN";
  switch (st) {
    case lely::canopen::NmtState::BOOTUP: name = "BOOT-UP (0x00)"; break;
    case lely::canopen::NmtState::STOP: name = "STOPPED (0x04)"; break;
    case lely::canopen::NmtState::START: name = "OPERATIONAL (0x05)"; break;
    case lely::canopen::NmtState::PREOP: name = "PRE-OPERATIONAL (0x7F)"; break;
    case lely::canopen::NmtState::RESET_NODE: name = "RESET NODE (0x06)"; break;
    case lely::canopen::NmtState::RESET_COMM: name = "RESET COMMUNICATION (0x07)"; break;
    default: break;
  }
  LogInfo(Stage::S06_PREOP, axis_tag_, Str("NMT state -> ", name));
}

void MbdvAxisDriver::OnConfig(::std::function<void(::std::error_code ec)> res) noexcept {
  // The base implementation starts the concise-DCF SDO download; it must run.
  // Wrapping the completion callback only observes the outcome.
  lely::canopen::BasicDriver::OnConfig([this, res](::std::error_code ec) {
    if (ec) {
      LogError(Stage::S09_PDO_VERIFY, axis_tag_,
               Str("concise-DCF download reported error ", ec.value(), " (", ec.message(),
                   ") - stage S09 will re-program and verify the PDOs explicitly"));
    } else {
      LogInfo(Stage::S09_PDO_VERIFY, axis_tag_,
              "concise-DCF download completed (see S09 for the explicit verification)");
    }
    if (res) res(ec);
  });
}

void MbdvAxisDriver::HandleRawTpdo(uint8_t pdo_no, const uint8_t* data, uint8_t len) {
  try {
    if (data == nullptr) return;
    const auto u16 = [data](uint8_t off) -> uint16_t {
      return static_cast<uint16_t>(data[off] | (static_cast<uint16_t>(data[off + 1]) << 8));
    };
    const auto u32 = [data](uint8_t off) -> uint32_t {
      return static_cast<uint32_t>(data[off]) |
             (static_cast<uint32_t>(data[off + 1]) << 8) |
             (static_cast<uint32_t>(data[off + 2]) << 16) |
             (static_cast<uint32_t>(data[off + 3]) << 24);
    };
    if (pdo_no == 1 && len >= 2) {
      RefreshStateFromStatusword(u16(0));
      ++statusword_count_;
    } else if (pdo_no == 2 && len >= 8) {
      actual_position_.store(static_cast<int32_t>(u32(0)));
      actual_velocity_.store(static_cast<int32_t>(u32(4)));
    } else if (pdo_no == 2 && len >= 4) {
      actual_position_.store(static_cast<int32_t>(u32(0)));
    } else if (pdo_no == 3 && len >= 8) {
      const uint16_t code = static_cast<uint16_t>(u32(0) & 0xFFFFu);
      const uint16_t previous = error_code_.exchange(code);
      if (code != 0 && code != previous) {
        LogError(Stage::S14_MOTION_TRACKING, axis_tag_,
                 Str("live CiA 402 error code from TPDO3: ", DecodeErrorCode402(code)));
      }
      const uint32_t alarm = u32(4);
      const uint32_t prev_alarm = dsp_alarm_.exchange(alarm);
      if (alarm != 0 && alarm != prev_alarm) {
        LogError(Stage::S14_MOTION_TRACKING, axis_tag_,
                 Str("live drive alarm from TPDO3: ", DecodeDspAlarmCode(alarm)));
      }
    }
  } catch (const std::exception&) {
    // A frame too short to hold the mapped payload: nothing to decode.
  }
}

void MbdvAxisDriver::OnRpdoWrite(uint16_t idx, uint8_t subidx) noexcept {
  (void)subidx;
  // Once the sniffer feeds this axis, the object dictionary is shared between both nodes
  // and would mix their values in; the per-frame decode above is authoritative.
  if (raw_feedback_.load()) return;
  try {
    if (idx == od::kStatusword) {
      const uint16_t sw = rpdo_mapped[idx][0];
      RefreshStateFromStatusword(sw);
      ++statusword_count_;
    } else if (idx == od::kPositionActual) {
      actual_position_.store(rpdo_mapped[idx][0]);
    } else if (idx == od::kVelocityActual) {
      actual_velocity_.store(rpdo_mapped[idx][0]);
    } else if (idx == od::kFollowingError) {
      following_error_.store(rpdo_mapped[idx][0]);
    } else if (idx == od::kErrorCode) {
      const uint16_t code = rpdo_mapped[idx][0];
      const uint16_t previous = error_code_.exchange(code);
      if (code != 0 && code != previous) {
        LogError(Stage::S14_MOTION_TRACKING, axis_tag_,
                 Str("live CiA 402 error code from TPDO3: ", DecodeErrorCode402(code)));
      }
    } else if (idx == od::kDspAlarmCode) {
      const uint32_t alarm = rpdo_mapped[idx][0];
      const uint32_t previous = dsp_alarm_.exchange(alarm);
      if (alarm != 0 && alarm != previous) {
        LogError(Stage::S14_MOTION_TRACKING, axis_tag_,
                 Str("live drive alarm from TPDO3: ", DecodeDspAlarmCode(alarm)));
      }
    }
  } catch (const std::exception&) {
    // A mapped object that has not arrived yet: nothing to decode.
  }
}

void MbdvAxisDriver::OnHeartbeat(bool occurred) noexcept {
  // Driven by lely's heartbeat consumer, so this reflects real 0x700+node frames: a
  // timeout means the node genuinely stopped answering, not that a timer expired.
  heartbeat_lost_.store(occurred);
  ++heartbeat_events_;
  if (occurred) {
    LogError(Stage::S12_SERVO_ON, axis_tag_,
             Str("heartbeat LOST: no 0x700+node frame within ",
                 heartbeat_consumer_ms_.load(), " ms. The node is treated as gone."));
  } else {
    LogInfo(Stage::S12_SERVO_ON, axis_tag_, "heartbeat resumed: the node is answering again");
  }
}

void MbdvAxisDriver::OnEmcy(uint16_t eec, uint8_t er, uint8_t msef[5]) noexcept {
  emergency_seen_.store(true);
  last_emcy_code_.store(eec);
  {
    std::lock_guard<std::mutex> lock(time_mutex_);
    emcy_time_ = std::chrono::steady_clock::now();
  }
  LogError(Stage::S14_MOTION_TRACKING, axis_tag_,
           Str("EMERGENCY frame: ", Hex(eec, 4), "; error register ",
               DecodeErrorRegister(er),
               "; manufacturer data [",
               static_cast<int>(msef[0]), " ", static_cast<int>(msef[1]), " ",
               static_cast<int>(msef[2]), " ", static_cast<int>(msef[3]), " ",
               static_cast<int>(msef[4]),
               "]. Dumping the full drive diagnostic snapshot on the driver strand next; "
               "0x200F carries the authoritative alarm code."));
  // OnEmcy is noexcept and runs in the C callback, so the SDO reads must be deferred to the
  // driver strand. Without this the alarm code (0x200F) is never sampled at the moment it
  // matters, and the fault stays unidentified.
  Defer([this, eec]() {
    try {
      const DriveSnapshot snap = ReadDriveSnapshotImpl();
      LogError(Stage::S14_MOTION_TRACKING, axis_tag_,
               Str("drive snapshot taken at EMERGENCY ", Hex(eec, 4), FormatDriveSnapshot(snap)));
    } catch (const std::exception& ex) {
      LogError(Stage::S14_MOTION_TRACKING, axis_tag_,
               Str("could not read the drive snapshot after EMERGENCY: ", ex.what()));
    }
  });
}


void MbdvAxisDriver::OnCanError(lely::io::CanError error) noexcept {
  can_error_.store(true);
  {
    std::lock_guard<std::mutex> lock(time_mutex_);
    last_can_error_ = Str("CAN controller error ", static_cast<int>(error), " at ",
                          std::chrono::steady_clock::now().time_since_epoch().count());
  }
  LogError(Stage::S02_CAN_LINK, axis_tag_,
           Str("CAN controller error code ", static_cast<int>(error),
               " - check the physical CAN layer (wiring, terminator, bit rate). The "
               "supervisor will re-open the interface if reconnect.recover_can_link is on."));
}

}  // namespace mbdv