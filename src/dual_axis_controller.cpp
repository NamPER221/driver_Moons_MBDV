#include "mbdv/dual_axis_controller.hpp"

#include "mbdv/config_path.hpp"

#include <lely/ev/fiber_exec.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace mbdv {

DualAxisController::DualAxisController() = default;

DualAxisController::~DualAxisController() { Stop(); }

// ---------------------------------------------------------------------------
// Stages S01..S03 - initialisation
// ---------------------------------------------------------------------------

bool DualAxisController::Initialize(const ControllerOptions& options, DiagnosticReport& report) {
  options_ = options;

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
                static_cast<int>(options_.expect_control_mode)));
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
      // Waking a stopped loop can surface here; it is not by itself a bring-up fault,
      // so it must not be reported at FATAL while Stop() is tearing things down.
      LogWarn(Stage::S04_EVENT_LOOP, nullptr,
              Str("CANopen event loop exited: ", ex.what()));
      is_running_.store(false);
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
  BringUpOptions bring_up;
  bring_up.expect_bitrate_bps = bring_up_options.expect_bitrate_bps;
  bring_up.expect_control_mode = bring_up_options.expect_control_mode;
  bring_up.write_control_mode = bring_up_options.write_control_mode;
  bring_up.check_dc_bus = bring_up_options.check_dc_bus;
  bring_up.sdo_controlword_fallback = bring_up_options.sdo_controlword_fallback;
  bring_up.sdo_setpoints = bring_up_options.sdo_setpoints;
  bring_up.watchdog_timeout_ms = bring_up_options.watchdog_timeout_ms;
  bring_up.store_parameters = bring_up_options.store_parameters;
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
// Stage S16 - shutdown
// ---------------------------------------------------------------------------

void DualAxisController::Stop() {
  if (!is_running_.load() && !loop_) return;

  LogBanner(Stage::S16_SHUTDOWN, nullptr);
  is_running_.store(false);

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
  StopAlarmWatch();

  if (loop_) loop_->stop();
  if (loop_thread_.joinable()) loop_thread_.join();

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

void DualAxisController::MoveBothAxes(int32_t pos1, int32_t pos2, bool relative) {
  LogInfo(Stage::S13_MOTION_COMMAND, nullptr,
          Str("commanding AX1 -> ", Sgn(pos1), " AX2 -> ", Sgn(pos2), " counts (relative=",
              relative ? "yes" : "no", ")"));
  if (axis1_) axis1_->SetTargetPosition(pos1, true, true, relative);
  if (axis2_) axis2_->SetTargetPosition(pos2, true, true, relative);
}

bool DualAxisController::MoveBothAxesStaged(DiagnosticReport& report, int32_t pos1, int32_t pos2,
                                             std::chrono::milliseconds timeout) {
  LogBanner(Stage::S13_MOTION_COMMAND, nullptr);
  bool ok1 = axis1_ ? axis1_->MoveToPositionStaged(report, pos1, timeout) : false;
  bool ok2 = axis2_ ? axis2_->MoveToPositionStaged(report, pos2, timeout) : false;
  return ok1 && ok2;
}

void DualAxisController::SetBothVelocities(int32_t vel1, int32_t vel2) {
  LogInfo(Stage::S13_MOTION_COMMAND, nullptr,
          Str("commanding AX1 -> ", vel1, " AX2 -> ", vel2, " counts/s"));
  if (axis1_) axis1_->SetTargetVelocity(vel1);
  if (axis2_) axis2_->SetTargetVelocity(vel2);
}

bool DualAxisController::SetVelocitiesStaged(DiagnosticReport& report, int32_t vel1, int32_t vel2,
                                              std::chrono::milliseconds settle) {
  LogBanner(Stage::S13_MOTION_COMMAND, nullptr);
  bool ok1 = axis1_ ? axis1_->SetVelocityStaged(report, vel1, settle) : false;
  bool ok2 = axis2_ ? axis2_->SetVelocityStaged(report, vel2, settle) : false;
  return ok1 && ok2;
}

void DualAxisController::SetCmdVel(double linear_v, double angular_w) {
  const WheelSpeeds speeds = kinematics_.ComputeWheelSpeeds(linear_v, angular_w);
  LogInfo(Stage::S13_MOTION_COMMAND, nullptr,
          Str("cmd_vel v=", linear_v, " m/s w=", angular_w, " rad/s -> AX1 ",
              speeds.left_driver_vel, " AX2 ", speeds.right_driver_vel, " counts/s"));
  if (axis1_) axis1_->SetTargetVelocity(speeds.left_driver_vel);
  if (axis2_) axis2_->SetTargetVelocity(speeds.right_driver_vel);
}

void DualAxisController::UpdateOdometry(double dt_sec) {
  if (!axis1_ || !axis2_) return;
  kinematics_.UpdateOdometryFromTicks(axis1_->GetActualPosition(), axis2_->GetActualPosition(),
                                      dt_sec);
}

// ---------------------------------------------------------------------------
// Telemetry
// ---------------------------------------------------------------------------

void DualAxisController::PrintTelemetry() const {
  std::ostringstream os;
  os << "\n+--------+------+-------------+------------------------------+--------------+"
        "--------------+------------+--------+---------+\n"
     << "| Axis   | Node | NMT         | CiA 402 State                 | Act Position |"
        " Act Velocity | Target Hit | Frames   |\n"
     << "+--------+------+-------------+------------------------------+--------------+"
        "--------------+------------+--------+---------+\n";

  auto row = [&os](const MbdvAxisDriver& ax) {
    os << "| " << std::left << std::setw(6) << ax.GetAxisName() << " | " << std::setw(4)
       << static_cast<int>(ax.GetNodeId()) << " | " << std::setw(11)
       << (ax.IsOperational() ? "OPERATIONAL" : (ax.SawBootUp() ? "PRE-OP" : "NOT SEEN")) << " | "
       << std::setw(28) << cia402_state_to_string(ax.GetCiA402State()) << " | " << std::right
       << std::setw(12) << ax.GetActualPosition() << " | " << std::setw(12)
       << ax.GetActualVelocity() << " | " << std::setw(10)
       << (ax.IsTargetReached() ? "YES" : "no") << " | " << std::setw(7)
       << ax.GetStatuswordCount() << "/" << ax.GetStatuswordSdoReads() << " |\n";
  };
  if (axis1_) row(*axis1_);
  if (axis2_) row(*axis2_);
  os << "+--------+------+-------------+------------------------------+--------------+"
        "--------------+------------+--------+---------+\n";

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