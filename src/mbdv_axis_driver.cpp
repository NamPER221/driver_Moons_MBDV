#include "mbdv/mbdv_axis_driver.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <future>
#include <sstream>
#include <utility>

namespace mbdv {

const char* controlword_path_to_string(ControlwordPath path) noexcept {
  switch (path) {
    case ControlwordPath::kRpdo: return "RPDO (real-time PDO)";
    case ControlwordPath::kSdo: return "SDO fallback (RPDO did not work)";
    case ControlwordPath::kNone: return "none";
  }
  return "none";
}

namespace {

/// CiA 301 Sub-index 01h (COB-ID used by PDO):
/// Bit 31: 0 = PDO exists / is valid (ENABLED), 1 = PDO does not exist / is not valid (DISABLED).
constexpr uint32_t kPdoDisabledBit = 0x80000000u;

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
  while (std::chrono::steady_clock::now() < deadline) {
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
  while (std::chrono::steady_clock::now() < deadline) {
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
  while (std::chrono::steady_clock::now() < deadline) {
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
  if (dip.node1_raw == 0) {
    LogInfo(stage, axis_tag_,
            "axis1 DIP address field is 0, so this axis takes its node-ID from the Luna "
            "software setting. That is valid - objects 0x2020/0x2021 remain authoritative.");
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

  if (opt.expect_control_mode != 0 && control_mode != opt.expect_control_mode) {
    return reject(Str("drive control mode mismatch: 0x2A30 = ", static_cast<int>(control_mode),
                      " but ", static_cast<int>(opt.expect_control_mode),
                      " is required for the requested CiA 402 mode"),
                  "0x6060 is ignored unless P1-00 selects the matching drive control mode "
                  "(PP=21, PV=15, TQ=1). Re-run with --p1-00 21 for PP, --p1-00 15 for PV, "
                  "or --p1-00 1 for TQ; the tool writes it and stores it with 0x1010:01 = 1.");
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
// BringUp driver (stages S05..S11)
// ---------------------------------------------------------------------------

bool MbdvAxisDriver::BringUp(DiagnosticReport& report, const BringUpOptions& options) {
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();
  const int64_t budget_ms = options.boot_timeout.count() * 5 + 8000;

  Defer([this, &report, options, promise]() {
    SetSdoControlwordFallback(options.sdo_controlword_fallback);
    SetSdoSetpoints(options.sdo_setpoints);
    SetStoreParameters(options.store_parameters);
    auto t0 = std::chrono::steady_clock::now();
    auto record = [this, &report, &t0](Stage stage, bool ok, const std::string& reason,
                                        const std::string& hint = std::string()) {
      const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - t0);
      t0 = std::chrono::steady_clock::now();
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
      {
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
  if (!TryWrite<int8_t>(od::kModeOfOperation, 0, want, &why)) {
    LogError(Stage::S10_MODE_OF_OPERATION, axis_tag_, Str("SDO write 0x6060 failed: ", why));
    return false;
  }

  // Confirm with 0x6061 Modes of operation display, as CiA 402 requires.
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  int8_t display = -1;
  while (std::chrono::steady_clock::now() < deadline) {
    display = ReadOr<int8_t>(od::kModeOfOperationDisplay, 0, static_cast<int8_t>(-1));
    if (display == want) {
      mode_display_.store(display);
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
  while (std::chrono::steady_clock::now() < deadline) {
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

  // Split the budget so a broken RPDO path is detected without doubling the wait.
  const std::chrono::milliseconds rpdo_budget(timeout.count() / 2);
  const std::chrono::milliseconds sdo_budget(timeout.count() - rpdo_budget.count());

  // ---- path 1: real-time PDO (RPDO1) ----
  try {
    tpdo_mapped[od::kControlword][0] = controlword;
    master.TpdoEvent();
  } catch (const std::exception& ex) {
    LogError(Stage::S12_SERVO_ON, axis_tag_, Str("RPDO1 transmission failed: ", ex.what()));
    return false;
  }
  if (WaitForState(expect, rpdo_budget)) {
    controlword_path_.store(ControlwordPath::kRpdo);
    controlword_time_ = std::chrono::steady_clock::now();
    return true;
  }

  // Did the drive actually receive it? Reading 0x6040 back over SDO is the only way to
  // tell "the frame never arrived" from "the drive applied it and refused to act".
  // Captured on the bus: frames 0x201#060000 (CW=0x0006) and 0x201#0F0000 (CW=0x000F) were
  // transmitted correctly, so this distinguishes a layout problem from a drive-side block.
  const uint16_t cw_readback = ReadOr<uint16_t>(od::kControlword, 0, 0xFFFFu);
  const bool drive_saw_it = (cw_readback != 0xFFFFu) && (cw_readback == controlword);
  LogWarn(Stage::S12_SERVO_ON, axis_tag_,
          Str("RPDO1 controlword produced no transition within ", rpdo_budget.count(),
              " ms (still ", cia402_state_to_string(state_.load()),
              "). Reading ", ObjRef(od::kControlword, 0), " back gives ", Hex(cw_readback, 4),
              " versus the sent ", Hex(controlword, 4), " -> ",
              drive_saw_it
                  ? "the drive DID store the command, so it is refusing the transition on "
                    "its own side, not a transport problem"
                  : "the drive did NOT store the command, so the RPDO frame is not being "
                    "applied (check the mapping byte order or a leading dummy byte)"));

  // ---- path 2: SDO fallback ----
  // The drive ignored CW over PDO. Repeating the identical command over SDO separates
  // "the frame never arrived" from "the drive refuses this transition".
  if (!sdo_controlword_fallback_) {
    LogError(Stage::S12_SERVO_ON, axis_tag_,
             Str("RPDO1 controlword had no effect and the SDO fallback is disabled "
                 "(--no-sdo-fallback), so the RPDO transmit path cannot be confirmed working"));
    controlword_path_.store(ControlwordPath::kNone);
    return false;
  }
  LogWarn(Stage::S12_SERVO_ON, axis_tag_,
          Str("RPDO1 controlword produced no transition within ", rpdo_budget.count(),
              " ms (still ", cia402_state_to_string(state_.load()),
              "); repeating the same command over SDO to ", ObjRef(od::kControlword, 0),
              " to tell the two cases apart"));

  std::string why;
  if (!TryWrite<uint16_t>(od::kControlword, 0, controlword, &why)) {
    LogError(Stage::S12_SERVO_ON, axis_tag_,
             Str("SDO write of the controlword also failed: ", why));
    controlword_path_.store(ControlwordPath::kNone);
    return false;
  }
  if (WaitForState(expect, sdo_budget)) {
    controlword_path_.store(ControlwordPath::kSdo);
    controlword_time_ = std::chrono::steady_clock::now();
    LogWarn(Stage::S12_SERVO_ON, axis_tag_,
            Str("SDO worked but RPDO did not -> the RPDO transmit path is not delivering "
                "frames to the drive. The drive is CiA 402 capable, so the cause is in the "
                "PDO configuration or the CAN wiring, not in the drive."));
    return true;
  }

  controlword_path_.store(ControlwordPath::kNone);
  LogError(Stage::S12_SERVO_ON, axis_tag_,
           Str("neither RPDO nor SDO moved the drive out of ",
               cia402_state_to_string(state_.load()),
               ". The command is reaching the object dictionary but the drive refuses the "
               "transition, so the block is inside the drive."));
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

  if (state_.load() == CiA402State::OPERATION_ENABLED) {
    LogInfo(stage, axis_tag_, "already in OPERATION_ENABLED, nothing to do");
    return true;
  }

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
                   " | controlword path = ", controlword_path_to_string(GetControlwordPath()),
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
              " | controlword path = ", controlword_path_to_string(GetControlwordPath()),
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
                    Str("OPERATION_ENABLED, Statusword ", Hex(statusword_.load(), 4),
                        ", controlword path = ", controlword_path_to_string(
                                                      GetControlwordPath())));
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
    tpdo_mapped[od::kTargetVelocity][0] = int32_t{0};
    master.TpdoEvent();
  } catch (...) {
  }

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
    tpdo_mapped[od::kTargetPosition][0] = target;
    // bit 4 new setpoint + bit 5 change immediately, keeping bit 3 enable operation.
    tpdo_mapped[od::kControlword][0] =
        static_cast<uint16_t>(controlword_commands::ENABLE_OPERATION |
                              controlword_bits::NEW_SET_POINT |
                              controlword_bits::CHANGE_SET_IMMEDIATELY);
    master.TpdoEvent();
  } catch (const std::exception& ex) {
    LogError(cmd, axis_tag_, Str("RPDO2 transmission failed: ", ex.what()));
    return false;
  }

  // Release the new-setpoint edge so later cycles are not seen as new setpoints.
  USleep(20000);
  try {
    tpdo_mapped[od::kControlword][0] = controlword_commands::ENABLE_OPERATION;
    master.TpdoEvent();
  } catch (...) {
  }

  // Some drive firmwares receive TPDOs but ignore RPDOs (measured on this MBDV-2X-520AC).
  // When asked to, deliver the setpoint over SDO instead, which the drive does honour.
  if (sdo_setpoints_.load()) {
    ++sdo_setpoint_count_;
    std::string why;
    const uint16_t cw = static_cast<uint16_t>(controlword_commands::ENABLE_OPERATION |
                                              controlword_bits::NEW_SET_POINT |
                                              controlword_bits::CHANGE_SET_IMMEDIATELY);
    if (!TryWrite<int32_t>(od::kTargetPosition, 0, target, &why) ||
        !TryWrite<uint16_t>(od::kControlword, 0, cw, &why)) {
      LogError(cmd, axis_tag_, Str("SDO setpoint to ", ObjRef(od::kTargetPosition, 0),
                                   " failed: ", why));
      return false;
    }
    USleep(20000);
    TryWrite<uint16_t>(od::kControlword, 0, controlword_commands::ENABLE_OPERATION, nullptr);
    LogInfo(cmd, axis_tag_,
            Str("setpoint delivered over SDO (--sdo-setpoints), count=",
                sdo_setpoint_count_.load()));
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

  LogInfo(cmd, axis_tag_,
          Str("RPDO3 target velocity = ", target, " counts/s; actual = ", actual_velocity_.load()));
  try {
    tpdo_mapped[od::kTargetVelocity][0] = target;
    tpdo_mapped[od::kControlword][0] = controlword_commands::ENABLE_OPERATION;
    master.TpdoEvent();
  } catch (const std::exception& ex) {
    LogError(cmd, axis_tag_, Str("RPDO3 transmission failed: ", ex.what()));
    return false;
  }

  if (sdo_setpoints_.load()) {
    ++sdo_setpoint_count_;
    std::string why;
    if (!TryWrite<int32_t>(od::kTargetVelocity, 0, target, &why)) {
      LogError(cmd, axis_tag_, Str("SDO setpoint to ", ObjRef(od::kTargetVelocity, 0),
                                   " failed: ", why));
      return false;
    }
    LogInfo(cmd, axis_tag_, Str("velocity delivered over SDO (--sdo-setpoints), count=",
                                sdo_setpoint_count_.load()));
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

void MbdvAxisDriver::RequestNmtStart() {
  LogInfo(Stage::S07_NMT_START, axis_tag_, "NMT Start Remote Node (CS=0x01)");
  master.Command(lely::canopen::NmtCommand::START, id());
}

void MbdvAxisDriver::RequestNmtPreOp() {
  master.Command(lely::canopen::NmtCommand::ENTER_PREOP, id());
}

void MbdvAxisDriver::RequestNmtReset() {
  master.Command(lely::canopen::NmtCommand::RESET_NODE, id());
}

void MbdvAxisDriver::SetModeOfOperation(CiA402Mode mode) {
  Defer([this, mode]() {
    LogInfo(Stage::S10_MODE_OF_OPERATION, axis_tag_,
            Str("writing 0x6060 = ", static_cast<int>(mode)));
    std::string why;
    if (!TryWrite<int8_t>(od::kModeOfOperation, 0, static_cast<int8_t>(mode), &why)) {
      LogWarn(Stage::S10_MODE_OF_OPERATION, axis_tag_, Str("SDO write 0x6060 failed: ", why));
      return;
    }
    try {
      tpdo_mapped[od::kModeOfOperation][0] = static_cast<int8_t>(mode);
      master.TpdoEvent();
    } catch (...) {
    }
  });
}

void MbdvAxisDriver::SendControlword(uint16_t controlword) {
  Defer([this, controlword]() {
    LogDebug(Stage::S12_SERVO_ON, axis_tag_,
             Str("Controlword = ", DecodeControlword(controlword)));
    try {
      tpdo_mapped[od::kControlword][0] = controlword;
      master.TpdoEvent();
    } catch (const std::exception& ex) {
      LogError(Stage::S12_SERVO_ON, axis_tag_,
               Str("RPDO1 transmission failed: ", ex.what()));
    }
  });
}

void MbdvAxisDriver::SendControlwordQuiet(uint16_t controlword) {
  Defer([this, controlword]() {
    try {
      tpdo_mapped[od::kControlword][0] = controlword;
      master.TpdoEvent();
    } catch (...) {
    }
  });
}

void MbdvAxisDriver::ResetFault() {
  Defer([this]() {
    LogInfo(Stage::S11_FAULT_RESET, axis_tag_,
            Str("fault reset; Statusword ", DecodeStatusword(statusword_.load())));
    try {
      tpdo_mapped[od::kControlword][0] = controlword_commands::FAULT_RESET;
      master.TpdoEvent();
    } catch (...) {
    }
    USleep(50000);
    try {
      tpdo_mapped[od::kControlword][0] = controlword_commands::DISABLE_VOLTAGE;
      master.TpdoEvent();
    } catch (...) {
    }
  });
}

void MbdvAxisDriver::QuickStop() {
  Defer([this]() {
    LogWarn(Stage::S15_SERVO_OFF, axis_tag_, "Quick Stop (CW=0x0002)");
    try {
      tpdo_mapped[od::kControlword][0] = controlword_commands::QUICK_STOP;
      master.TpdoEvent();
    } catch (...) {
    }
  });
}

bool MbdvAxisDriver::EnableServo(std::chrono::milliseconds timeout) {
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();
  Defer([this, timeout, promise]() {
    try {
      promise->set_value(EnableServoImpl(timeout));
    } catch (...) {
      promise->set_value(false);
    }
  });
  if (future.wait_for(timeout * 4 + std::chrono::milliseconds(2000)) ==
      std::future_status::ready) {
    return future.get();
  }
  return false;
}

bool MbdvAxisDriver::DisableServo() {
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();
  Defer([this, promise]() {
    try {
      promise->set_value(DisableServoImpl(std::chrono::milliseconds(1000)));
    } catch (...) {
      promise->set_value(false);
    }
  });
  if (future.wait_for(std::chrono::milliseconds(4000)) == std::future_status::ready) {
    return future.get();
  }
  return false;
}

void MbdvAxisDriver::SetTargetPosition(int32_t target_position, bool new_setpoint, bool immediate,
                                       bool relative) {
  Defer([this, target_position, new_setpoint, immediate, relative]() {
    LogInfo(Stage::S13_MOTION_COMMAND, axis_tag_,
            Str("target position = ", target_position, " (relative=",
                relative ? "yes" : "no", ", immediate=", immediate ? "yes" : "no", ")"));
    try {
      tpdo_mapped[od::kTargetPosition][0] = target_position;
      uint16_t cw = controlword_commands::ENABLE_OPERATION;
      if (new_setpoint) cw |= controlword_bits::NEW_SET_POINT;
      if (immediate) cw |= controlword_bits::CHANGE_SET_IMMEDIATELY;
      if (relative) cw |= controlword_bits::ABS_REL;
      tpdo_mapped[od::kControlword][0] = cw;
      master.TpdoEvent();
      if (new_setpoint) {
        USleep(20000);
        tpdo_mapped[od::kControlword][0] = controlword_commands::ENABLE_OPERATION;
        master.TpdoEvent();
      }
    } catch (const std::exception& ex) {
      LogError(Stage::S13_MOTION_COMMAND, axis_tag_,
               Str("RPDO2 transmission failed: ", ex.what()));
    }
  });
}

void MbdvAxisDriver::SetTargetVelocity(int32_t target_velocity) {
  Defer([this, target_velocity]() {
    if (target_velocity != last_target_velocity_.exchange(target_velocity)) {
      LogInfo(Stage::S13_MOTION_COMMAND, axis_tag_,
              Str("target velocity = ", target_velocity, " counts/s"));
    }
    try {
      tpdo_mapped[od::kTargetVelocity][0] = target_velocity;
      tpdo_mapped[od::kControlword][0] = controlword_commands::ENABLE_OPERATION;
      master.TpdoEvent();
    } catch (const std::exception& ex) {
      LogError(Stage::S13_MOTION_COMMAND, axis_tag_,
               Str("RPDO3 transmission failed: ", ex.what()));
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

  // Storing the whole parameter block over CAN right after modifying it is NOT done by
  // default: a stored-parameter write that aborts part way (observed as SDO abort 0x08000020
  // on 0x1010:01) can leave the block inconsistent, and after repeated --p1-00 runs 0x2A30
  // was found reading the undocumented value 30 on BOTH axes. Persistence is therefore
  // opt-in, and P1-00 is better set once in Luna than from this tool.
  if (!store_parameters_) {
    LogWarn(Stage::S10_MODE_OF_OPERATION, axis_tag_,
            "NOT saving parameters (0x1010:01). The value stays valid until the next power "
            "cycle. Add --p1-00-save to persist it, or set P1-00 once in Luna.");
    return readback == p1_00_value;
  }

  std::string save_why;
  if (!TryWrite<uint32_t>(0x1010, 1, uint32_t{1}, &save_why)) {
    LogError(Stage::S10_MODE_OF_OPERATION, axis_tag_,
             Str("--p1-00-save: 0x1010:01 = 1 failed: ", save_why,
                 ". The parameter block may be partly written - power-cycle the drive and "
                 "verify 0x2A30 before trusting it."));
    return false;
  }
  const uint32_t after_save = ReadOr<uint32_t>(od::kControlMode, 0, 0xFFFFFFFFu);
  if (after_save != p1_00_value) {
    LogError(Stage::S10_MODE_OF_OPERATION, axis_tag_,
             Str("after saving, 0x2A30 reads ", static_cast<int>(after_save),
                 " instead of ", static_cast<int>(p1_00_value),
                 ". The stored parameter block looks inconsistent - power-cycle and check "
                 "P1-00 in Luna."));
    return false;
  }
  LogInfo(Stage::S10_MODE_OF_OPERATION, axis_tag_,
          "parameters stored (0x1010:01 = 1) and 0x2A30 re-verified after the save");
  return true;
}

bool MbdvAxisDriver::SetDriveControlMode(uint32_t p1_00_value) {
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();
  Defer([this, p1_00_value, promise]() {
    try {
      promise->set_value(SetDriveControlModeImpl(p1_00_value));
    } catch (...) {
      promise->set_value(false);
    }
  });
  if (future.wait_for(std::chrono::milliseconds(3000)) == std::future_status::ready) {
    return future.get();
  }
  return false;
}

// ---------------------------------------------------------------------------
// lely callbacks
// ---------------------------------------------------------------------------

void MbdvAxisDriver::OnBoot(lely::canopen::NmtState st, char es, const std::string& what) noexcept {
  boot_seen_.store(true);
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
    config_done_.store(true);
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

void MbdvAxisDriver::OnRpdoWrite(uint16_t idx, uint8_t subidx) noexcept {
  (void)subidx;
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
    } else if (idx == od::kModeOfOperationDisplay) {
      mode_display_.store(rpdo_mapped[idx][0]);
    }
  } catch (const std::exception&) {
    // A mapped object that has not arrived yet: nothing to decode.
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

void MbdvAxisDriver::OnHeartbeat(bool occurred) noexcept {
  if (occurred) heartbeat_seen_.store(true);
}

void MbdvAxisDriver::OnCanError(lely::io::CanError error) noexcept {
  LogError(Stage::S02_CAN_LINK, axis_tag_,
           Str("CAN controller error code ", static_cast<int>(error),
               " - check the physical CAN layer (wiring, terminator, bit rate)"));
}

}  // namespace mbdv