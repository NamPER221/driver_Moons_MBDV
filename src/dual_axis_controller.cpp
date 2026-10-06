#include "mbdv/dual_axis_controller.hpp"

#include <cstring>
#include <mutex>

#include "mbdv/config_path.hpp"

#include <lely/ev/fiber_exec.hpp>

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

namespace mbdv {

DualAxisController::DualAxisController() = default;

DualAxisController::~DualAxisController() { Stop(); }

const char* axis_health_to_string(AxisHealth health) noexcept {
  switch (health) {
    case AxisHealth::kUnknown:    return "unknown";
    case AxisHealth::kAlive:      return "alive";
    case AxisHealth::kLost:       return "LOST";
    case AxisHealth::kFaulted:    return "FAULTED";
    case AxisHealth::kRecovering: return "recovering";
    case AxisHealth::kFailed:     return "FAILED";
  }
  return "unknown";
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool DualAxisController::Configure(const Params& params, std::string* error) {
  params_ = params;
  start_time_ = std::chrono::steady_clock::now();

  // A rate of 200 Hz means a 5 ms period; round to whole milliseconds because the loop
  // sleeps on a millisecond clock, and never let the rounding land on 0.
  const double hz = params_.control_rate_hz > 0.0 ? params_.control_rate_hz : 1.0;
  const auto period = static_cast<int64_t>(1000.0 / hz);
  control_period_ = std::chrono::milliseconds(period > 0 ? period : 1);

  const double cw_hz = params_.controlword_rate_hz > 0.0 ? params_.controlword_rate_hz : 1.0;
  const auto cw_period = static_cast<int64_t>(1000.0 / cw_hz);
  controlword_period_ = std::chrono::milliseconds(cw_period > 0 ? cw_period : 1);

  // Everything the reconnect path needs, so Supervise() never has to guess.
  bring_up_.expect_bitrate_bps = params_.expect_bitrate_bps;
  bring_up_.expect_control_mode = params_.expect_control_mode;
  bring_up_.write_control_mode = params_.write_control_mode;
  bring_up_.check_dc_bus = true;
  bring_up_.profile_accel = params_.profile_accel;
  bring_up_.profile_decel = params_.profile_decel;
  bring_up_.watchdog_timeout_ms = params_.watchdog_timeout_ms;
  bring_up_.watchdog_action = params_.watchdog_action;
  bring_up_.heartbeat_consumer_ms = params_.heartbeat_consumer_ms;
  bring_up_.heartbeat_producer_ms = params_.heartbeat_producer_ms;
  bring_up_.pdo.transmission_type = 0xFF;
  bring_up_.pdo.tpdo1_event_timer_ms = params_.tpdo1_event_timer_ms;
  bring_up_.pdo.tpdo2_event_timer_ms = params_.tpdo2_event_timer_ms;
  bring_up_.pdo.tpdo3_event_timer_ms = params_.tpdo3_event_timer_ms;
  bring_up_.pdo.program_pdos = params_.program_pdos;
  bring_up_.boot_timeout = params_.boot_timeout;

  // Bus budget, reported once at start-up so a rate that looks fine on paper but
  // saturates the wiring is visible before anything is commanded.
  //
  // Classic CAN at 500 kbps carries roughly 4100 frames/s with 11-bit IDs and 8 data
  // bytes. Six TPDOs at the control rate dominate; the setpoint is one frame per axis per
  // period, and the controlword refresh adds two more per axis at its own lower rate.
  const double tpdo_fps = 6.0 * params_.control_rate_hz;
  const double rpdo_fps = 2.0 * params_.control_rate_hz + 4.0 * params_.controlword_rate_hz;
  const double frames_per_s = tpdo_fps + rpdo_fps;
  const double load_pct = 100.0 * frames_per_s * 122.0 / static_cast<double>(params_.expect_bitrate_bps);
  LogInfo(Stage::S01_CONFIG, nullptr,
          Str("bus budget at ", params_.control_rate_hz, " Hz: ~", static_cast<int>(tpdo_fps + 0.5),
              " TPDO + ~", static_cast<int>(rpdo_fps + 0.5), " RPDO frames/s = ~",
              static_cast<int>(frames_per_s + 0.5), " frames/s, about ", static_cast<int>(load_pct + 0.5),
              "% of the ", params_.expect_bitrate_bps / 1000, " kbps link"));
  if (load_pct > 60.0) {
    LogWarn(Stage::S01_CONFIG, nullptr,
            Str("the projected bus load is about ", static_cast<int>(load_pct + 0.5),
                "%, which is high enough to cause lost frames and jitter. Lower"
                " rates.control_hz, or raise the TPDO event timers in config/params.yaml."));
  }

  return publisher_.Configure(params_, error);
}

std::chrono::milliseconds DualAxisController::ControlPeriod() const { return control_period_; }

// ---------------------------------------------------------------------------
// Stages S01..S03 - initialisation
// ---------------------------------------------------------------------------

bool DualAxisController::Initialize(const ControllerOptions& options, DiagnosticReport& report) {
  options_ = options;

  // The geometry decides how encoder counts become metres, so it comes from params.yaml
  // rather than from a compiled-in constant.
  kinematics_.SetConfig(params_.kinematics);

  // The DCF is looked up relative to the working directory first, then relative to the
  // build-time config directory and to the executable, so the tools run from anywhere.
  const std::string dcf_path = ResolveConfigPath(options_.dcf_path);

  LogBanner(Stage::S01_CONFIG, nullptr);

  // ---- S01 CONFIG ----
  {
    LogInfo(Stage::S01_CONFIG, nullptr,
            Str("working directory = '", WorkingDirectory(), "'"));
    LogInfo(Stage::S01_CONFIG, nullptr,
            Str("requested DCF '", options_.dcf_path, "' resolved to '",
                dcf_path.empty() ? "(NOT FOUND)" : dcf_path, "'"));
    LogInfo(Stage::S01_CONFIG, nullptr,
            Str("bus=", options_.can_interface, " expected=",
                can_bit_rate_bps_to_string(options_.expect_bitrate_bps), " axis1 node=",
                static_cast<int>(options_.axis1_node_id), " axis2 node=",
                static_cast<int>(options_.axis2_node_id), " mode=",
                cia402_mode_to_string(options_.mode), " p1-00 expected=",
                options_.expect_control_mode == 0
                    ? std::string("(skipped: the bus check does not depend on the drive mode)")
                    : Str(static_cast<int>(options_.expect_control_mode))));
    LogInfo(Stage::S01_CONFIG, nullptr,
            Str("control rate=", params_.control_rate_hz, " Hz (period ",
                control_period_.count(), " ms), controlword refresh=",
                params_.controlword_rate_hz, " Hz, TPDO timers ",
                static_cast<int>(params_.tpdo1_event_timer_ms), "/",
                static_cast<int>(params_.tpdo2_event_timer_ms), "/",
                static_cast<int>(params_.tpdo3_event_timer_ms), " ms"));
    LogInfo(Stage::S01_CONFIG, nullptr,
            Str("heartbeat: drive 0x1017=",
                params_.heartbeat_producer_ms ? Str(params_.heartbeat_producer_ms, " ms")
                                              : "its own default",
                ", master watches ", params_.heartbeat_consumer_ms,
                " ms - a node is declared lost after that silence; reconnect ",
                params_.reconnect_enabled ? "enabled" : "disabled", ", CAN link recovery ",
                params_.recover_can_link ? "enabled" : "disabled"));
  if (params_.heartbeat_producer_ms && params_.heartbeat_consumer_ms) {
    const double ratio = static_cast<double>(params_.heartbeat_consumer_ms) /
                         params_.heartbeat_producer_ms;
    const int margin = params_.heartbeat_consumer_ms - params_.heartbeat_producer_ms;
    if (ratio < 2.0) {
      LogWarn(Stage::S01_CONFIG, nullptr,
              Str("the heartbeat margin is only ", margin, " ms (ratio ",
                  static_cast<int>(ratio * 10) / 10.0,
                  "x). That is workable on an idle bus but leaves little room for jitter; if"
                  " the program ever reports a spurious 'heartbeat lost', halve"
                  " heartbeat.producer_ms in config/params.yaml."));
    }
  }
    LogInfo(Stage::S01_CONFIG, nullptr,
            Str("odometry: ", params_.publish_odometry ? "published" : "not published", " at ",
                params_.control_rate_hz, " Hz via ",
                Str((params_.publish_callback ? "callback" : "")
                    + std::string(params_.publish_udp ? " + udp" : ""))));
    LogInfo(Stage::S01_CONFIG, nullptr,
            Str("slave download file=", options_.bin_path.empty()
                                          ? "(named in the DCF; lely resolves it against the "
                                            "working directory)"
                                          : options_.bin_path));

    if (dcf_path.empty()) {
      std::ostringstream tried;
      const std::vector<std::string> candidates = ConfigPathCandidates(options_.dcf_path);
      for (std::size_t i = 0; i < candidates.size(); ++i) {
        tried << "\n        " << (i + 1) << ". " << candidates[i];
      }
      std::ostringstream hint;
      hint << "Searched:" << tried.str()
           << "\n        Fix by either running from the project root, or passing an absolute "
              "path with -d /path/to/master.dcf."
           << "\n        If the file is genuinely missing, generate it with:"
           << "\n          cmake --build <build-dir> --target generate_canopen_dcf"
           << "\n        which runs 'dcfgen -r -d config config/master.yaml'.";
      report.RecordFailure(Stage::S01_CONFIG, nullptr,
                           Str("cannot find DCF file '", options_.dcf_path, "'"), hint.str());
      return false;
    }
    options_.dcf_path = dcf_path;
    report.Pass(Stage::S01_CONFIG, nullptr, Str("DCF '", dcf_path, "' is readable"));

    // lely resolves the DCF's DownloadFile entries (slave_1.bin, slave_2.bin) against
    // the *process* working directory - co_sub_get_download_file() returns the raw
    // filename stored in the DCF, with no directory prepended. Without this the SDO
    // download aborts with "slave_1.bin: No such file or directory" and SDO abort
    // code 08000020, which surfaces as a bogus S05 configuration failure.
    const std::string dcf_dir = DirectoryOf(dcf_path);
    std::string previous;
    if (ChangeWorkingDirectory(dcf_dir, &previous)) {
      original_cwd_ = previous;
      LogInfo(Stage::S01_CONFIG, nullptr,
              Str("working directory changed to '", WorkingDirectory(),
                  "' so the DCF's slave_*.bin references resolve; restored on shutdown"));
    } else {
      LogWarn(Stage::S01_CONFIG, nullptr,
              Str("could not change directory to '", dcf_dir,
                  "' - the DCF's slave_*.bin references will be looked up in '",
                  WorkingDirectory(), "' instead"));
    }
  }

  LogBanner(Stage::S02_CAN_LINK, nullptr);

  // ---- S02 CAN_LINK ----
  {
    io_guard_ = std::make_unique<lely::io::IoGuard>();
    ctx_ = std::make_unique<lely::io::Context>();
    poll_ = std::make_unique<lely::io::Poll>(*ctx_);
    loop_ = std::make_unique<lely::ev::Loop>(poll_->get_poll());
    const auto exec = loop_->get_executor();
    timer_ = std::make_unique<lely::io::Timer>(*poll_, exec, CLOCK_MONOTONIC);

    // Reuse the kernel's existing tx_queue_len; changing it over netlink needs
    // CAP_NET_ADMIN and fails with EPERM for unprivileged runs.
    size_t iface_qlen = 10;
    {
      std::ifstream qlen_file("/sys/class/net/" + options_.can_interface + "/tx_queue_len");
      if (!(qlen_file >> iface_qlen) || iface_qlen == 0) iface_qlen = 10;
    }

    std::string failure;
    try {
      ctrl_ = std::make_unique<lely::io::CanController>(options_.can_interface.c_str(),
                                                        iface_qlen);
    } catch (const std::exception& ex) {
      LogWarn(Stage::S02_CAN_LINK, nullptr,
              Str("CanController with tx_queue_len=", static_cast<int>(iface_qlen),
                  " failed (", ex.what(), "), retrying with 1"));
      try {
        ctrl_ = std::make_unique<lely::io::CanController>(options_.can_interface.c_str(), 1);
      } catch (const std::exception& ex2) {
        failure = ex2.what();
      }
    }

    if (!ctrl_) {
      report.RecordFailure(
          Stage::S02_CAN_LINK, nullptr,
          Str("cannot open SocketCAN interface '", options_.can_interface, "': ", failure),
          "Checklist: (1) the interface exists - 'ip link show " + options_.can_interface +
              "'; (2) bring it up - 'sudo ip link set " + options_.can_interface +
              " up'; (3) set the bit rate - 'sudo ip link set " + options_.can_interface +
              " type can bitrate " + std::to_string(options_.expect_bitrate_bps) +
              "'; (4) run with sufficient privileges (root or CAP_NET_ADMIN).");
      return false;
    }

    chan_ = std::make_unique<lely::io::CanChannel>(*poll_, exec);
    try {
      chan_->open(*ctrl_);
    } catch (const std::exception& ex) {
      report.RecordFailure(Stage::S02_CAN_LINK, nullptr,
                           Str("CanChannel::open failed: ", ex.what()),
                           "The controller exists but cannot be attached. Verify the interface "
                           "is UP and the bit rate is set.");
      return false;
    }

    LogInfo(Stage::S02_CAN_LINK, nullptr,
            Str("SocketCAN '", options_.can_interface, "' opened (tx_queue_len=",
                static_cast<int>(iface_qlen), ")"));
    report.Pass(Stage::S02_CAN_LINK, nullptr,
                Str("SocketCAN '", options_.can_interface, "' open"));
  }

  LogBanner(Stage::S03_MASTER_LOAD, nullptr);

  // ---- S03 MASTER_LOAD ----
  {
    try {
      master_ = std::make_unique<lely::canopen::AsyncMaster>(
          *timer_, *chan_, options_.dcf_path, options_.bin_path);
    } catch (const std::exception& ex) {
      report.RecordFailure(Stage::S03_MASTER_LOAD, nullptr,
                           Str("AsyncMaster construction failed: ", ex.what()),
                           "The DCF must parse. Re-run dcfgen and confirm the EDS path inside "
                           "config/master.yaml resolves.");
      return false;
    }

    axis1_ = std::make_unique<MbdvAxisDriver>(*master_, options_.axis1_node_id, "Axis 1", "AX1");
    axis2_ = std::make_unique<MbdvAxisDriver>(*master_, options_.axis2_node_id, "Axis 2", "AX2");

    // Give each axis a transport that addresses its own node.
    //
    // The master's object dictionary holds a single copy of 0x6040/0x607A/0x60FF, and
    // co_dev_tpdo_event() transmits every PDO whose mapping contains whatever just changed.
    // Writing 0x60FF on a two-node master therefore fires RPDO3 for BOTH nodes, so both
    // drives receive the union of both commands - captured on the bus as node 1 and node 2
    // both receiving +2000 and -2000, which cancels to zero. A differential-drive robot
    // built on that can only go straight; it can never turn. Building the frame here and
    // sending it on the COB-ID of the node that owns the value is what fixes it, and it is
    // what the known-good bring-up on this hardware does.
    lely::io::CanChannel* chan = chan_.get();
    rpdo_mutex_ = std::make_unique<std::mutex>();
    const MbdvAxisDriver::RpdoSender sender =
        [this, chan](uint8_t node_id, uint8_t pdo_no, uint16_t controlword, bool has_setpoint,
                    int32_t setpoint) {
      // CiA 301 pre-defined connection set: PDO n sits at base + 0x100 * (n - 1) + node.
      const uint32_t base = pdo_no == 1 ? 0x200u : pdo_no == 2 ? 0x300u : 0x400u;
      if (pdo_no < 1 || pdo_no > 3 || node_id < 1 || node_id > 127) {
        LogError(Stage::S03_MASTER_LOAD, nullptr,
                 Str("refusing to build an RPDO frame for pdo ", static_cast<int>(pdo_no),
                     " / node ", static_cast<int>(node_id)));
        return false;
      }
      can_msg msg = CAN_MSG_INIT;
      msg.id = base + node_id;
      // RPDO1 is 0x6040 + 0x6060 (3 bytes), so its third byte carries the mode, not a
      // setpoint. RPDO2/RPDO3 are 0x6040 + a 32-bit setpoint (6 bytes).
      msg.len = pdo_no == 1 ? 3 : (has_setpoint ? 6 : 3);
      msg.data[0] = static_cast<uint_least8_t>(controlword & 0xFFu);
      msg.data[1] = static_cast<uint_least8_t>((controlword >> 8) & 0xFFu);
      if (pdo_no == 1) {
        msg.data[2] = static_cast<uint_least8_t>(setpoint & 0xFF);
      } else if (has_setpoint) {
        const uint32_t raw = static_cast<uint32_t>(setpoint);
        msg.data[2] = static_cast<uint_least8_t>(raw & 0xFFu);
        msg.data[3] = static_cast<uint_least8_t>((raw >> 8) & 0xFFu);
        msg.data[4] = static_cast<uint_least8_t>((raw >> 16) & 0xFFu);
        msg.data[5] = static_cast<uint_least8_t>((raw >> 24) & 0xFFu);
      }
      std::error_code ec;
      {
        // Serialise against the other axis' strand and against lely's own writes.
        std::lock_guard<std::mutex> lock(*rpdo_mutex_);
        chan->write(msg, 100, ec);
      }
      if (ec) {
        LogError(Stage::S13_MOTION_COMMAND, nullptr,
                 Str("RPDO", static_cast<int>(pdo_no), " send to node ",
                     static_cast<int>(node_id), " failed: ", ec.message()));
      }
      return !ec;
    };
    axis1_->SetRpdoSender(sender);
    axis2_->SetRpdoSender(sender);

    // Decode each node's TPDO from raw bytes on a second socket. lely's object dictionary
    // is shared between both nodes, so the mapping it builds writes both nodes' statusword,
    // position and velocity into one place - which is why node 1 reported no feedback and
    // node 2's numbers were a mixture of the two axes. Routing by COB-ID keeps them apart.
    sniffer_ = std::make_unique<CanSniffer>();
    std::string sniffer_error;
    const uint8_t node1 = options_.axis1_node_id;
    const uint8_t node2 = options_.axis2_node_id;
    const bool sniffer_started =
        sniffer_->Start(options_.can_interface,
                        [this, node1, node2](uint32_t id, const uint8_t* data, uint8_t len) {
                          const uint32_t base = id & 0x780u;  // 0x181->0x180, 0x281->0x280, 0x381->0x380
                          const uint32_t node = id & 0x7Fu;
                          MbdvAxisDriver* axis = nullptr;
                          if (node == node1) axis = axis1_.get();
                          else if (node == node2) axis = axis2_.get();
                          if (axis == nullptr) return;
                          switch (base) {
                            case 0x180: axis->HandleRawTpdo(1, data, len); break;
                            case 0x280: axis->HandleRawTpdo(2, data, len); break;
                            case 0x380: axis->HandleRawTpdo(3, data, len); break;
                            default: break;
                          }
                        },
                        &sniffer_error);
    if (sniffer_started) {
      axis1_->SetRawFeedback(true);
      axis2_->SetRawFeedback(true);
      LogInfo(Stage::S02_CAN_LINK, nullptr,
              Str("TPDO feedback: decoding raw frames on a second receive-only socket so each"
                  " node reports its own statusword, position and velocity"));
    } else {
      LogWarn(Stage::S02_CAN_LINK, nullptr,
              Str("could not open the feedback sniffer (", sniffer_error,
                  "). Falling back to lely's shared object dictionary: with two nodes on one"
                  " master their statusword, position and velocity will be mixed together"
                  " and the odometry of the two axes cannot be separated."));
    }


    LogInfo(Stage::S03_MASTER_LOAD, nullptr,
            Str("AsyncMaster ready; AX1 -> node ", static_cast<int>(options_.axis1_node_id),
                ", AX2 -> node ", static_cast<int>(options_.axis2_node_id)));
    report.Pass(Stage::S03_MASTER_LOAD, nullptr, "AsyncMaster + both axis drivers created");
  }
  return true;
}

// ---------------------------------------------------------------------------
// Stage S04 - event loop
// ---------------------------------------------------------------------------

bool DualAxisController::Start(DiagnosticReport& report) {
  if (is_running_.load()) return true;
  LogBanner(Stage::S04_EVENT_LOOP, nullptr);

  is_running_.store(true);
  loop_thread_ = std::thread([this]() {
    try {
      lely::ev::FiberThread fiber_thrd;
      loop_->run();
    } catch (const std::exception& ex) {
      // lely's run() throws with whatever errno it last saw once Stop() ends the loop
      // ("Resource temporarily unavailable", "Connection timed out"): expected during a
      // shutdown, a real problem only while the controller still believes it is running.
      if (is_running_.exchange(false)) {
        LogWarn(Stage::S04_EVENT_LOOP, nullptr, Str("CANopen event loop exited: ", ex.what()));
      } else {
        LogDebug(Stage::S04_EVENT_LOOP, nullptr, Str("CANopen event loop stopped: ", ex.what()));
      }
    }
  });

  loop_->get_executor().post([this]() {
    try {
      master_->Reset();
    } catch (const std::exception& ex) {
      LogError(Stage::S04_EVENT_LOOP, nullptr, Str("master Reset failed: ", ex.what()));
    }
  });

  LogInfo(Stage::S04_EVENT_LOOP, nullptr,
          "event-loop thread started; master Reset() posted (boot-up + SDO download)");
  report.Pass(Stage::S04_EVENT_LOOP, nullptr, "event loop running, master Reset posted");
  return true;
}

// ---------------------------------------------------------------------------
// Stages S05..S11 - per-axis bring-up
// ---------------------------------------------------------------------------

bool DualAxisController::BringUpBothAxes(DiagnosticReport& report,
                                          const ControllerOptions& bring_up_options) {
  // The controller keeps the options it was initialised with, but bring-up may run under
  // relaxed expectations (--selftest drops the P1-00 check so a real bus fault is not
  // masked by an unrelated mode setting).
  //
  // Start from the params-derived set: ControllerOptions has no heartbeat fields, and a
  // default-constructed BringUpOptions left heartbeat_consumer_ms/producer_ms at 0, so the
  // first bring-up never installed params.yaml's heartbeat - the master kept watching with
  // the DCF's 3000 ms (0x1016 = 0x0BB8) instead of 1500 ms, and 0x1017 was never written.
  BringUpOptions bring_up = bring_up_;
  bring_up.expect_bitrate_bps = bring_up_options.expect_bitrate_bps;
  bring_up.expect_control_mode = bring_up_options.expect_control_mode;
  bring_up.write_control_mode = bring_up_options.write_control_mode;
  bring_up.check_dc_bus = bring_up_options.check_dc_bus;
  bring_up.profile_accel = bring_up_options.profile_accel;
  bring_up.profile_decel = bring_up_options.profile_decel;
  bring_up.watchdog_timeout_ms = bring_up_options.watchdog_timeout_ms;
  bring_up.pdo = bring_up_options.pdo;
  bring_up.boot_timeout = bring_up_options.boot_timeout;

  bool ok1 = false;
  bool ok2 = false;

  // Sequential on purpose: each BringUp() blocks until that node has finished its
  // stages, so the log reads strictly in stage order.
  if (axis1_) ok1 = axis1_->BringUp(report, bring_up);
  if (axis2_) ok2 = axis2_->BringUp(report, bring_up);

  // Nodes that never reported at all must still appear in the report.
  if (!axis1_ || !axis1_->SawBootUp()) {
    if (!report.Find(Stage::S05_BOOTUP, "AX1")) {
      report.Fail(Stage::S05_BOOTUP, "AX1", "axis driver was never initialised",
                  "Check ControllerOptions.axis1_node_id.");
    }
  }
  if (!axis2_ || !axis2_->SawBootUp()) {
    if (!report.Find(Stage::S05_BOOTUP, "AX2")) {
      report.Fail(Stage::S05_BOOTUP, "AX2", "axis driver was never initialised",
                  "Check ControllerOptions.axis2_node_id.");
    }
  }

  const bool both = ok1 && ok2;
  if (both) {
    LogInfo(Stage::S11_FAULT_RESET, nullptr,
            "AX1 and AX2 both passed stages S05..S11");
  } else {
    LogError(Stage::S11_FAULT_RESET, nullptr,
             Str("bring-up failed: AX1=", ok1 ? "PASS" : "FAIL", " AX2=", ok2 ? "PASS" : "FAIL"));
  }
  return both;
}

// ---------------------------------------------------------------------------
// Stage S10 - mode of operation
// ---------------------------------------------------------------------------

bool DualAxisController::SetModeBothAxes(DiagnosticReport& report, CiA402Mode mode) {
  LogBanner(Stage::S10_MODE_OF_OPERATION, nullptr);
  bool ok1 = axis1_ ? axis1_->SetModeStaged(report, mode) : false;
  bool ok2 = axis2_ ? axis2_->SetModeStaged(report, mode) : false;
  return ok1 && ok2;
}

// ---------------------------------------------------------------------------
// Stage S12 - servo on
// ---------------------------------------------------------------------------

bool DualAxisController::EnableBothAxes(DiagnosticReport& report, std::chrono::milliseconds timeout) {
  LogBanner(Stage::S12_SERVO_ON, nullptr);
  bool ok1 = axis1_ ? axis1_->EnableServoStaged(report, timeout) : false;
  bool ok2 = axis2_ ? axis2_->EnableServoStaged(report, timeout) : false;

  // An axis that reached Operation Enabled is live. Nothing else sets this on the startup
  // path - only the recovery branch does - so without it health stays kUnknown, AllAxesAlive()
  // returns false, and the differential-drive command path never runs a single cycle: the
  // servo is on and the teleop silently commands nothing.
  if (ok1) {
    sup1_.health.store(AxisHealth::kAlive);
    sup1_.last_known_counts.store(axis1_->GetActualPosition());
    sup1_.last_reason = "startup";
  }
  if (ok2) {
    sup2_.health.store(AxisHealth::kAlive);
    sup2_.last_known_counts.store(axis2_->GetActualPosition());
    sup2_.last_reason = "startup";
  }
  if (ok1 && ok2) {
    // Stop both axes before anything else can command them. The drive holds whatever
    // 0x60FF was left at, so enabling without an explicit zero lets the robot drive off on
    // its own - measured here as tens of thousands of counts of travel with no RPDO3 sent.
    if (axis1_) axis1_->ZeroVelocityNow();
    if (axis2_) axis2_->ZeroVelocityNow();
    LogInfo(Stage::S12_SERVO_ON, nullptr,
            "both axes commanded to a full stop before any motion command");
    kinematics_.Realign(sup1_.last_known_counts.load(), sup2_.last_known_counts.load());
  }
  return ok1 && ok2;
}

// ---------------------------------------------------------------------------
// Stage S15 - servo off
// ---------------------------------------------------------------------------

void DualAxisController::DisableBothAxes(DiagnosticReport& report) {
  LogBanner(Stage::S15_SERVO_OFF, nullptr);
  if (axis1_) axis1_->DisableServoStaged(report);
  if (axis2_) axis2_->DisableServoStaged(report);
}

// ---------------------------------------------------------------------------
// Continuous alarm watch
// ---------------------------------------------------------------------------

void DualAxisController::StartAlarmWatch(std::chrono::milliseconds period) {
  if (axis1_) axis1_->StartAlarmWatch(period);
  if (axis2_) axis2_->StartAlarmWatch(period);
}

void DualAxisController::StopAlarmWatch() {
  if (axis1_) axis1_->StopAlarmWatch();
  if (axis2_) axis2_->StopAlarmWatch();
}

// ---------------------------------------------------------------------------
// Supervision
// ---------------------------------------------------------------------------

namespace {

/// Encoder feedback older than this marks the published odometry sample as not alive, so a
/// subscriber can tell a genuinely stationary robot from a frozen pose. It does not stop
/// the robot: a lost CAN connection is decided by the heartbeat alone.
constexpr std::chrono::milliseconds kTelemetryStaleMs{500};

/// Brings @p current up to a sensible backoff, capped by params.reconnect_backoff_max_ms.
std::chrono::milliseconds NextBackoff(std::chrono::milliseconds current,
                                       uint32_t attempts,
                                       const Params& params) {
  const auto cap = std::chrono::milliseconds(params.reconnect_backoff_max_ms);
  std::chrono::milliseconds next = current;
  if (next.count() > 0) next *= 2;
  if (next < std::chrono::milliseconds(params.reconnect_backoff_ms)) {
    next = std::chrono::milliseconds(params.reconnect_backoff_ms);
  }
  if (next > cap) next = cap;
  (void)attempts;
  return next;
}

/// True while the axis' node is answering, so its encoder and velocity feedback are real:
/// a faulted axis still reports where its (possibly coasting) wheel is, a lost one does not.
bool FeedbackTrusted(const AxisSupervisor& sup) {
  const AxisHealth health = sup.health.load();
  return health == AxisHealth::kAlive || health == AxisHealth::kFaulted ||
         (health == AxisHealth::kRecovering && sup.recovering_from == AxisHealth::kFaulted);
}

/// Encoder reading to integrate for an axis: live while trusted, frozen otherwise.
int32_t FeedbackCounts(const MbdvAxisDriver& ax, const AxisSupervisor& sup) {
  return FeedbackTrusted(sup) ? ax.GetActualPosition() : sup.last_known_counts.load();
}

/// The CAN connection to the node is back: its heartbeat arrives and reports OPERATIONAL.
/// Read from the live heartbeat rather than from the supervisor state, so the stop output
/// can be released as soon as the link is back, whatever the bring-up stages reported.
bool NodeReachable(const MbdvAxisDriver& ax) {
  return !ax.HeartbeatLost() && ax.IsOperational();
}

}  // namespace

void DualAxisController::EmergencyStopAll(const std::string& reason) {
  if (!stop_latched_.exchange(true)) {
    stop_since_ = std::chrono::steady_clock::now();
    stop_reason_ = reason;
    // The hardware output first: with the CAN link down, every RPDO write below can block
    // for its full timeout, and the IO is the only thing that still reaches a lost drive.
    if (stop_callback_) stop_callback_(true, reason);
    LogError(Stage::S13_MOTION_COMMAND, nullptr,
             Str("STOP OUTPUT ON: ", reason,
                 ". Motion stays blocked until every node answers again and a zero command "
                 "is in force."));
  }
  // Then every axis CAN still reaches, the lost one included: its node may still act on an
  // RPDO even when its heartbeat or Statusword did not get through. StopNow() never enables
  // an axis.
  if (axis1_) axis1_->StopNow();
  if (axis2_) axis2_->StopNow();
  last_cmd_v_ = 0.0;
  last_cmd_w_ = 0.0;
}

void DualAxisController::ReleaseStop() {
  if (!stop_latched_.exchange(false)) return;
  LogInfo(Stage::S13_MOTION_COMMAND, nullptr,
          Str("STOP OUTPUT OFF: every node answers again and a zero command is in force (the "
              "stop was: ", stop_reason_, "). Servos switched off meanwhile are re-enabled at "
              "zero speed; motion needs a new command."));
  stop_reason_.clear();
  if (stop_callback_) stop_callback_(false, std::string());
}

bool DualAxisController::AllAxesAlive() const {
  return Health1() == AxisHealth::kAlive && Health2() == AxisHealth::kAlive;
}

RecoveryOutcome DualAxisController::RunRecovery(MbdvAxisDriver* ax, AxisHealth from) {
  RecoveryOutcome out;
  try {
    if (from == AxisHealth::kLost) {
      if (!ax->WaitForReconnectReady(bring_up_.boot_timeout)) {
        out.detail = ax->HeartbeatLost()
                         ? "the node's heartbeat has not come back"
                         : "lely's boot-slave process for the node has not finished";
        return out;
      }
      // Communication only - the servo is left off. The stop output may be holding the
      // drives' E-STOP inputs right now, so enabling (and the fault reset before it) waits
      // for the kFaulted path, which runs once the stop output is released.
      BringUpOptions options = bring_up_;
      options.fault_reset = false;
      DiagnosticReport local;
      if (!ax->BringUp(local, options)) {
        out.detail = "bring-up stages S05..S09 failed (see the log above)";
      } else if (!ax->SetModeStaged(local, options_.mode, params_.servo_timeout)) {
        out.detail = "mode of operation (S10) was not accepted";
      } else {
        out.ok = true;
      }
      return out;
    }

    FaultKind cause = FaultKind::kNone;
    out.ok = ax->RecoverServo("supervisor", params_.servo_timeout, params_.servo_timeout,
                              &cause);
    if (!out.ok) {
      out.hardware_cause = cause == FaultKind::kSto || cause == FaultKind::kLimit ||
                           cause == FaultKind::kNoMainPower;
      out.detail = Str("fault reset / re-enable did not succeed (cause: ",
                       fault_kind_to_string(cause), ")");
    }
  } catch (const std::exception& ex) {
    out.ok = false;
    out.detail = Str("recovery attempt threw: ", ex.what());
  }
  return out;
}

void DualAxisController::Supervise() {
  const auto now = std::chrono::steady_clock::now();

  struct Slot {
    MbdvAxisDriver* driver;
    AxisSupervisor* sup;
    const char* tag;
  };
  const Slot slots[] = {
      {axis1_.get(), &sup1_, "AX1"},
      {axis2_.get(), &sup2_, "AX2"},
  };

  // ---- 1. collect recovery attempts that finished on the worker thread ----
  for (const Slot& slot : slots) {
    MbdvAxisDriver* ax = slot.driver;
    AxisSupervisor* sup = slot.sup;
    if (!ax || !sup->task.valid() ||
        sup->task.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
      continue;
    }
    const RecoveryOutcome result = sup->task.get();
    const bool was_lost = sup->recovering_from == AxisHealth::kLost;
    if (result.ok && was_lost) {
      // The node talks again but its servo is off: it continues on the kFaulted path, which
      // re-enables it once the stop output has been released.
      sup->health.store(AxisHealth::kFaulted);
      sup->enable_after_reconnect = true;
      sup->episode_attempts = 0;
      sup->backoff = std::chrono::milliseconds(0);
      sup->next_attempt = now;
      sup->last_reason = "reconnected, servo off";
      // An axis found still enabled must not keep running on a setpoint from before.
      ax->StopNow();
      if (axis1_ && axis2_) {
        // The drive may have restarted and now count from another encoder origin, so the
        // pose and the delta reference are re-baselined together (see Realign()).
        kinematics_.Realign(FeedbackCounts(*axis1_, sup1_), FeedbackCounts(*axis2_, sup2_));
      }
      sup->last_known_counts.store(ax->GetActualPosition());
      LogInfo(Stage::S12_SERVO_ON, slot.tag,
              "reconnected; odometry re-baselined to (0,0,0) because the encoder reference "
              "may have changed. The servo is re-enabled once the stop output is released.");
    } else if (result.ok) {
      sup->health.store(AxisHealth::kAlive);
      sup->recoveries.fetch_add(1);
      sup->enable_after_reconnect = false;
      sup->episode_attempts = 0;
      sup->backoff = std::chrono::milliseconds(0);
      sup->last_reason = "recovered";
      sup->last_known_counts.store(ax->GetActualPosition());
      LogInfo(Stage::S12_SERVO_ON, slot.tag,
              Str("servo re-enabled at zero speed (", sup->recoveries.load(),
                  " recoveries so far); motion needs a new command."));
    } else {
      sup->health.store(sup->recovering_from);
      sup->last_reason = result.detail;
      if (result.hardware_cause) {
        // Only a person can clear this; keep checking without spending an attempt on it.
        if (sup->episode_attempts > 0) --sup->episode_attempts;
        sup->next_attempt = now + std::chrono::milliseconds(params_.fault_retry_backoff_ms);
        LogWarn(Stage::S11_FAULT_RESET, slot.tag,
                Str(result.detail, "; waiting for the hardware cause to clear, next check in ",
                    params_.fault_retry_backoff_ms, " ms"));
      } else {
        sup->backoff = was_lost ? NextBackoff(sup->backoff, sup->episode_attempts, params_)
                                : std::chrono::milliseconds(params_.fault_retry_backoff_ms);
        sup->next_attempt = now + sup->backoff;
        LogWarn(Stage::S12_SERVO_ON, slot.tag,
                Str(was_lost ? "reconnect" : "fault recovery", " attempt ",
                    sup->episode_attempts, " failed: ", result.detail, "; retrying in ",
                    sup->backoff.count(), " ms"));
      }
    }
  }

  // ---- 2. an axis that went away - on any signal - stops BOTH axes at once ----
  //
  // A lost CAN connection is decided by the heartbeat: the consumer timeout, or the NMT
  // state carried in the heartbeat frame no longer being OPERATIONAL. A CAN controller
  // error is deliberately not a signal: lely reports every error frame, and a noisy but
  // working bus would stop the robot. A link that is really gone (bus-off, cut cable) also
  // silences the heartbeat. The error only triggers the channel re-open in step 5.
  for (const Slot& slot : slots) {
    MbdvAxisDriver* ax = slot.driver;
    AxisSupervisor* sup = slot.sup;
    if (!ax) continue;
    const bool node_gone = ax->HeartbeatLost() || !ax->IsOperational();

    if (sup->health.load() == AxisHealth::kFaulted && node_gone) {
      // A faulted axis whose node then went silent needs the full reconnect, not a reset.
      sup->health.store(AxisHealth::kLost);
      sup->last_reason = "node went silent while faulted";
      sup->episode_attempts = 0;
      sup->backoff = std::chrono::milliseconds(0);
      sup->next_attempt = now;
      continue;
    }
    if (sup->health.load() != AxisHealth::kAlive) continue;

    AxisHealth next = AxisHealth::kAlive;
    std::string why;
    if (ax->HeartbeatLost()) {
      next = AxisHealth::kLost;
      why = Str("CAN connection lost: no heartbeat for ", params_.heartbeat_consumer_ms, " ms");
    } else if (!ax->IsOperational()) {
      next = AxisHealth::kLost;
      why = "CAN connection lost: heartbeat reports the node out of NMT OPERATIONAL";
    } else if (ax->GetCiA402State() != CiA402State::OPERATION_ENABLED) {
      next = AxisHealth::kFaulted;
      why = Str("servo left Operation Enabled (now ",
                cia402_state_to_string(ax->GetCiA402State()), ")");
    }

    if (next == AxisHealth::kAlive) {
      // Remember the last good encoder reading so a dead axis freezes the odometry instead
      // of injecting a step.
      sup->last_known_counts.store(ax->GetActualPosition());
      continue;
    }
    sup->health.store(next);
    sup->last_reason = why;
    sup->episode_attempts = 0;
    sup->backoff = std::chrono::milliseconds(0);
    sup->next_attempt = next == AxisHealth::kFaulted
                            ? now + std::chrono::milliseconds(params_.fault_retry_backoff_ms)
                            : now;
    LogError(Stage::S12_SERVO_ON, slot.tag, Str(slot.tag, " went away: ", why));
    EmergencyStopAll(Str(slot.tag, " ", why));
  }

  // ---- 3. release the stop output once it is safe to ----
  //
  // Not tied to the servos being enabled: with the output wired to the drives' E-STOP inputs
  // they cannot enable while it is on, so waiting for that would hold it on forever. Enabling
  // comes after the release (step 6), at zero speed, and motion still needs a new command.
  if (stop_latched_.load() &&
      now - stop_since_ >= std::chrono::milliseconds(params_.stop_hold_ms) &&
      requested_v_ == 0.0 && requested_w_ == 0.0) {
    bool all_back = true;
    for (const Slot& slot : slots) {
      const AxisHealth health = slot.sup->health.load();
      const bool reconnecting =
          health == AxisHealth::kLost || health == AxisHealth::kFailed ||
          (health == AxisHealth::kRecovering && slot.sup->recovering_from == AxisHealth::kLost);
      if (!slot.driver || reconnecting || !NodeReachable(*slot.driver)) {
        all_back = false;
      }
    }
    if (all_back) ReleaseStop();
  }

  // ---- 4. keep the enabled axes fed: controlword + current setpoint, slower schedule ----
  if (now - last_controlword_refresh_ >= controlword_period_) {
    last_controlword_refresh_ = now;
    for (const Slot& slot : slots) {
      if (slot.driver && slot.sup->health.load() == AxisHealth::kAlive) {
        slot.driver->RefreshControlword();
      }
    }
  }

  // ---- 5. the CAN link itself ----
  const bool link_errored = (axis1_ && axis1_->CanLinkErrored()) ||
                            (axis2_ && axis2_->CanLinkErrored());
  if (link_errored && params_.recover_can_link && params_.reconnect_enabled &&
      now >= next_link_attempt_ && !link_recovery_in_progress_.exchange(true)) {
    ++link_recoveries_;
    next_link_attempt_ = now + std::chrono::milliseconds(params_.reconnect_backoff_ms);
    LogWarn(Stage::S02_CAN_LINK, nullptr,
            Str("CAN controller error detected; re-opening ", params_.can_interface,
                " (recovery #", link_recoveries_, ")"));
    // Closing and re-opening the channel re-creates the raw socket and re-registers the CAN
    // filters without rebuilding the master and throwing away the odometry. It runs on the
    // event-loop thread: lely's CanNet owns a read on this channel and resubmits it from
    // that thread (io_can_net_read_func), so doing it from here raced lely mid-operation.
    loop_->get_executor().post([this]() {
      try {
        if (chan_->is_open()) chan_->close();
        chan_->open(*ctrl_);
        LogInfo(Stage::S02_CAN_LINK, nullptr,
                Str(params_.can_interface,
                    " re-opened; the nodes are re-established by the reconnect path"));
      } catch (const std::exception& ex) {
        LogError(Stage::S02_CAN_LINK, nullptr,
                 Str("re-opening ", params_.can_interface, " failed: ", ex.what(),
                     ". If the interface was taken down administratively, bring it back with:"
                     "  sudo ip link set ", params_.can_interface, " type can bitrate ",
                     params_.expect_bitrate_bps, " && sudo ip link set ",
                     params_.can_interface, " up"));
      }
      // An edge, not a level: a link that is still broken reports a new error.
      if (axis1_) axis1_->ClearCanLinkError();
      if (axis2_) axis2_->ClearCanLinkError();
      link_recovery_in_progress_.store(false);
    });
  }

  // ---- 6. start a recovery attempt when one is due ----
  for (const Slot& slot : slots) {
    MbdvAxisDriver* ax = slot.driver;
    AxisSupervisor* sup = slot.sup;
    const AxisHealth health = sup->health.load();
    if (!ax || sup->task.valid() || now < sup->next_attempt) continue;

    uint32_t limit = 0;  // shown in the log; 0 = unlimited (reconnect only)
    bool exhausted = false;
    if (health == AxisHealth::kLost) {
      if (!params_.reconnect_enabled) continue;
      limit = params_.reconnect_max_attempts;
      exhausted = limit > 0 && sup->episode_attempts >= limit;
    } else if (health == AxisHealth::kFaulted) {
      // Never while the stop output is on: wired to the E-STOP inputs it keeps the drives
      // from enabling, and every attempt would just fail and count.
      if (stop_latched_.load()) continue;
      if (!params_.auto_recover_faults && !sup->enable_after_reconnect) continue;
      limit = params_.fault_max_attempts;
      exhausted = sup->episode_attempts >= limit;
    } else {
      continue;
    }
    if (exhausted) {
      sup->health.store(AxisHealth::kFailed);
      sup->last_reason = Str(health == AxisHealth::kLost ? "reconnect" : "fault recovery",
                             " gave up after ", sup->episode_attempts, " attempts");
      LogError(Stage::S12_SERVO_ON, slot.tag,
               Str("giving up on ", slot.tag, ": ", sup->last_reason,
                   ". Both axes stay stopped; clear the cause and restart the program."));
      continue;
    }

    sup->recovering_from = health;
    sup->health.store(AxisHealth::kRecovering);
    ++sup->episode_attempts;
    sup->attempts.fetch_add(1);
    LogInfo(Stage::S12_SERVO_ON, slot.tag,
            Str(health == AxisHealth::kLost ? "reconnect" : "fault recovery", " attempt ",
                sup->episode_attempts, limit > 0 ? Str("/", limit) : std::string(),
                " started (", sup->last_reason, ")"));
    sup->task = std::async(std::launch::async,
                           [this, ax, health]() { return RunRecovery(ax, health); });
  }
}

void DualAxisController::UpdateAndPublishOdometry(double dt_sec) {
  if (!axis1_ || !axis2_) return;

  // An axis whose node is gone keeps contributing its last known reading, so the pose
  // freezes rather than jumping to whatever the encoder reports once it is back.
  const int32_t left = FeedbackCounts(*axis1_, sup1_);
  const int32_t right = FeedbackCounts(*axis2_, sup2_);

  kinematics_.UpdateOdometryFromTicks(left, right, dt_sec);
  // The twist comes from the drives' own velocity feedback (0x606C), as the odometry
  // interface documents; an axis that is gone contributes 0, not its last stale value.
  kinematics_.UpdateTwistFromSpeeds(FeedbackTrusted(sup1_) ? axis1_->GetActualVelocity() : 0,
                                    FeedbackTrusted(sup2_) ? axis2_->GetActualVelocity() : 0);
  if (!params_.publish_odometry) return;

  const RobotPose pose = kinematics_.GetPose();
  const RobotTwist twist = kinematics_.GetTwist();
  OdometrySample sample;
  sample.stamp_sec = std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                                  start_time_)
                         .count();
  sample.x = pose.x;
  sample.y = pose.y;
  sample.theta = pose.theta;
  sample.linear_v = twist.linear_v;
  sample.angular_w = twist.angular_w;
  sample.left_counts = left;
  sample.right_counts = right;
  // Both feeds must be fresh: a single frozen axis already makes the pose wrong.
  const bool feedback_fresh =
      !axis1_->IsTelemetryStale(kTelemetryStaleMs) &&
      !axis2_->IsTelemetryStale(kTelemetryStaleMs);
  sample.alive = AllAxesAlive() && feedback_fresh;
  publisher_.Publish(sample);
}

// ---------------------------------------------------------------------------
// Stage S16 - shutdown
// ---------------------------------------------------------------------------

void DualAxisController::Stop() {
  if (!is_running_.load() && !loop_) return;

  LogBanner(Stage::S16_SHUTDOWN, nullptr);
  is_running_.store(false);

  // From here on nothing supervises the drives, so whatever is wired to the stop output must
  // see it on - the same as for a lost axis.
  if (!stop_latched_.exchange(true)) {
    stop_since_ = std::chrono::steady_clock::now();
    stop_reason_ = "controller shutting down";
    if (stop_callback_) stop_callback_(true, stop_reason_);
  }

  // Teardown order is forced by object lifetimes and is not arbitrary:
  //   1. stop the loop and join the thread, so nothing runs concurrently;
  //   2. destroy the axis drivers - a FiberDriver destructor deregisters itself from
  //      the master and touches the fiber executor, so both must still be alive;
  //   3. destroy the master - AsyncMaster holds references to the CAN channel and the
  //      timer, and uses the loop's executor;
  //   4. only then release the CAN I/O, the timer, and finally the loop, poll,
  //      context and IoGuard.
  // Destroying the loop before the drivers (as an earlier version did) leaves the
  // driver destructors writing into freed memory and crashed at exit.
  //
  // A recovery attempt on the worker thread waits on its driver's strand, so it is told to
  // abort and allowed to finish while the event loop still runs; after loop_->stop() it
  // could only sit out its full timeout, and it must never outlive the drivers.
  if (axis1_) axis1_->AbortPendingWork();
  if (axis2_) axis2_->AbortPendingWork();
  for (AxisSupervisor* sup : {&sup1_, &sup2_}) {
    if (sup->task.valid()) sup->task.wait();
  }
  StopAlarmWatch();

  if (loop_) loop_->stop();
  if (loop_thread_.joinable()) loop_thread_.join();

  // Stop the sniffer before the drivers it calls into go away.
  if (sniffer_) sniffer_->Stop();
  axis1_.reset();
  axis2_.reset();
  master_.reset();

  chan_.reset();
  ctrl_.reset();
  timer_.reset();
  loop_.reset();
  poll_.reset();
  ctx_.reset();
  io_guard_.reset();

  if (!original_cwd_.empty() && WorkingDirectory() != original_cwd_) {
    if (!ChangeWorkingDirectory(original_cwd_, nullptr)) {
      LogWarn(Stage::S16_SHUTDOWN, nullptr,
              Str("could not restore the working directory to '", original_cwd_, "'"));
    }
  }

  LogInfo(Stage::S16_SHUTDOWN, nullptr, "CAN resources released");
}

// ---------------------------------------------------------------------------
// Motion helpers
// ---------------------------------------------------------------------------

bool DualAxisController::MoveBothAxesStaged(DiagnosticReport& report, int32_t pos1, int32_t pos2,
                                             std::chrono::milliseconds timeout) {
  LogBanner(Stage::S13_MOTION_COMMAND, nullptr);
  if (!MotionPermitted()) {
    const std::string why = StopRequested() ? Str("stop output on: ", stop_reason_)
                                            : std::string("an axis is not alive");
    report.Fail(Stage::S13_MOTION_COMMAND, nullptr, Str("position move refused (", why, ")"));
    LogError(Stage::S13_MOTION_COMMAND, nullptr, Str("position move refused (", why, ")"));
    return false;
  }
  bool ok1 = axis1_ ? axis1_->MoveToPositionStaged(report, pos1, timeout) : false;
  bool ok2 = axis2_ ? axis2_->MoveToPositionStaged(report, pos2, timeout) : false;
  return ok1 && ok2;
}

bool DualAxisController::SetVelocitiesStaged(DiagnosticReport& report, int32_t vel1, int32_t vel2,
                                              std::chrono::milliseconds settle) {
  LogBanner(Stage::S13_MOTION_COMMAND, nullptr);
  // Stopping is always allowed; anything else needs both axes and the stop output off.
  if ((vel1 != 0 || vel2 != 0) && !MotionPermitted()) {
    const std::string why = StopRequested() ? Str("stop output on: ", stop_reason_)
                                            : std::string("an axis is not alive");
    report.Fail(Stage::S13_MOTION_COMMAND, nullptr, Str("velocity command refused (", why, ")"));
    LogError(Stage::S13_MOTION_COMMAND, nullptr, Str("velocity command refused (", why, ")"));
    return false;
  }
  // Both setpoints go out first, on the strands the 200 Hz loop already uses, so the drives
  // ramp together; the staged calls below then re-send and verify one axis at a time.
  // Only to an enabled axis: controlword 0x000F must never be what switches one on.
  if (axis1_ && axis1_->GetCiA402State() == CiA402State::OPERATION_ENABLED)
    axis1_->SetTargetVelocity(vel1);
  if (axis2_ && axis2_->GetCiA402State() == CiA402State::OPERATION_ENABLED)
    axis2_->SetTargetVelocity(vel2);
  bool ok1 = axis1_ ? axis1_->SetVelocityStaged(report, vel1, settle) : false;
  bool ok2 = axis2_ ? axis2_->SetVelocityStaged(report, vel2, settle) : false;
  return ok1 && ok2;
}

void DualAxisController::SetCmdVel(double linear_v, double angular_w) {
  // Recorded even when it cannot be applied: Supervise() only releases the stop output
  // while the request is zero.
  requested_v_ = linear_v;
  requested_w_ = angular_w;
  if (!MotionPermitted()) return;

  const WheelSpeeds speeds = kinematics_.ComputeWheelSpeeds(linear_v, angular_w);
  if (linear_v != last_cmd_v_ || angular_w != last_cmd_w_) {
    last_cmd_v_ = linear_v;
    last_cmd_w_ = angular_w;
    LogInfo(Stage::S13_MOTION_COMMAND, nullptr,
            Str("cmd_vel v=", linear_v, " m/s w=", angular_w, " rad/s -> AX1 ",
                speeds.left_driver_vel, " AX2 ", speeds.right_driver_vel, " counts/s"));
  }
  if (axis1_) axis1_->SetTargetVelocity(speeds.left_driver_vel);
  if (axis2_) axis2_->SetTargetVelocity(speeds.right_driver_vel);
}

// ---------------------------------------------------------------------------
// Telemetry
// ---------------------------------------------------------------------------

void DualAxisController::PrintTelemetry() const {
  // One rule for all three borders, sized to the rows below.
  static const char* const kRule =
      "+--------+------+-------------+------------------------------+--------------+"
      "--------------+------------+-------------+\n";
  std::ostringstream os;
  os << '\n' << kRule
     << "| Axis   | Node | NMT         | CiA 402 State                | Act Position |"
        " Act Velocity | Target Hit | Frames/SDO  |\n"
     << kRule;

  auto row = [&os](const MbdvAxisDriver& ax) {
    os << "| " << std::left << std::setw(6) << ax.GetAxisName() << " | " << std::setw(4)
       << static_cast<int>(ax.GetNodeId()) << " | " << std::setw(11)
       << (ax.IsOperational() ? "OPERATIONAL" : (ax.SawBootUp() ? "PRE-OP" : "NOT SEEN")) << " | "
       << std::setw(28) << cia402_state_to_string(ax.GetCiA402State()) << " | " << std::right
       << std::setw(12) << ax.GetActualPosition() << " | " << std::setw(12)
       << ax.GetActualVelocity() << " | " << std::setw(10)
       << (ax.IsTargetReached() ? "YES" : "no") << " | " << std::setw(11)
       << Str(ax.GetStatuswordCount(), "/", ax.GetStatuswordSdoReads()) << " |\n";
  };
  if (axis1_) row(*axis1_);
  if (axis2_) row(*axis2_);
  os << kRule;

  const RobotPose pose = kinematics_.GetPose();
  const RobotTwist twist = kinematics_.GetTwist();
  os << std::fixed << std::setprecision(4) << ">>> ODOMETRY  x = " << pose.x << " m, y = " << pose.y
     << " m, theta = " << pose.theta << " rad (" << (pose.theta * 180.0 / M_PI) << " deg)\n"
     << ">>> VELOCITY  v = " << twist.linear_v << " m/s, w = " << twist.angular_w << " rad/s\n";

  const bool stale1 = axis1_ && axis1_->IsTelemetryStale(std::chrono::milliseconds(500));
  const bool stale2 = axis2_ && axis2_->IsTelemetryStale(std::chrono::milliseconds(500));
  if (stale1 || stale2) {
    os << ">>> WARNING: no Statusword for >500 ms on " << (stale1 ? "AX1 " : "")
       << (stale2 ? "AX2" : "") << " -> check stage S09 (TPDO1 mapping / event timer).\n";
  }
  os << ">>> HEALTH  AX1 " << axis_health_to_string(Health1()) << " (" << sup1_.last_reason
     << "), AX2 " << axis_health_to_string(Health2()) << " (" << sup2_.last_reason << ")\n";
  if (StopRequested()) {
    os << ">>> STOP OUTPUT ON: " << stop_reason_
       << " - held until every node answers again and a zero command is in force.\n";
  }
  std::cout << os.str() << std::endl;
}

void DualAxisController::PrintDriveDiagnostics() const {
  // ReadDriveSnapshot() posts to the driver strand, so it needs a mutable driver.
  auto dump = [](MbdvAxisDriver& ax) {
    const DriveSnapshot snap = ax.ReadDriveSnapshot();
    std::ostringstream os;
    os << "\n===== " << ax.GetAxisName() << " (node " << static_cast<int>(ax.GetNodeId())
       << ", tag " << ax.GetAxisTag() << ") =====\n"
       << FormatDriveSnapshot(snap)
       << "      0x6041 decode            : " << DecodeStatusword(snap.statusword) << '\n'
       << "      telemetry                 : " << ax.GetStatuswordCount()
       << " Statusword frames"
       << (ax.IsTelemetryStale(std::chrono::milliseconds(500)) ? " (STALE)" : " (live)") << '\n'
       << "      emergency frames          : " << (ax.SawEmergency() ? "yes" : "none") << '\n';
    std::cout << os.str() << std::endl;
  };
  if (axis1_) dump(*axis1_);
  if (axis2_) dump(*axis2_);
}

}  // namespace mbdv