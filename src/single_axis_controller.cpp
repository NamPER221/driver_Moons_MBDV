#include "mbdv/single_axis_controller.hpp"

#include "mbdv/config_path.hpp"

#include <lely/ev/fiber_exec.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace mbdv {

SingleAxisController::SingleAxisController() = default;

SingleAxisController::~SingleAxisController() { Stop(); }

bool SingleAxisController::Initialize(const SingleAxisOptions& options,
                                      DiagnosticReport& report) {
  options_ = options;

  // Resolved relative to the working directory, then to the build-time config
  // directory and to the executable, so the tools run from anywhere.
  const std::string dcf_path = ResolveConfigPath(options_.dcf_path);

  LogBanner(Stage::S01_CONFIG, nullptr);

  // ---- S01 CONFIG ----
  {
    LogInfo(Stage::S01_CONFIG, nullptr, Str("working directory = '", WorkingDirectory(), "'"));
    LogInfo(Stage::S01_CONFIG, nullptr,
            Str("requested DCF '", options_.dcf_path, "' resolved to '",
                dcf_path.empty() ? "(NOT FOUND)" : dcf_path, "'"));
    LogInfo(Stage::S01_CONFIG, nullptr,
            Str("bus=", options_.can_interface, " expected=",
                can_bit_rate_bps_to_string(options_.expect_bitrate_bps), " node=",
                static_cast<int>(options_.node_id), " mode=",
                cia402_mode_to_string(options_.mode), " p1-00 expected=",
                static_cast<int>(options_.expect_control_mode)));

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
           << "\n          cmake --build <build-dir> --target generate_single_axis_500k_dcf";
      report.RecordFailure(Stage::S01_CONFIG, nullptr,
                           Str("cannot find DCF file '", options_.dcf_path, "'"), hint.str());
      return false;
    }
    options_.dcf_path = dcf_path;
    report.Pass(Stage::S01_CONFIG, nullptr, Str("DCF '", dcf_path, "' is readable"));

    // lely resolves the DCF's DownloadFile entries (slave_1.bin) against the process
    // working directory, so move there for the lifetime of the controller.
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
          "Checklist: (1) 'ip link show " + options_.can_interface + "'; (2) 'sudo ip link set " +
              options_.can_interface + " up'; (3) 'sudo ip link set " + options_.can_interface +
              " type can bitrate " +
              std::to_string(options_.expect_bitrate_bps) +
              "'; (4) run as root or with CAP_NET_ADMIN.");
      return false;
    }

    chan_ = std::make_unique<lely::io::CanChannel>(*poll_, exec);
    try {
      chan_->open(*ctrl_);
    } catch (const std::exception& ex) {
      report.RecordFailure(Stage::S02_CAN_LINK, nullptr, Str("CanChannel::open failed: ",
                                                             ex.what()),
                           "The controller exists but cannot be attached. Verify the interface is "
                           "UP and the bit rate is set.");
      return false;
    }
    LogInfo(Stage::S02_CAN_LINK, nullptr,
            Str("SocketCAN '", options_.can_interface, "' opened"));
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
                           "The DCF must parse; re-run dcfgen and check the EDS path inside "
                           "config/single_axis_500k/master.yaml.");
      return false;
    }
    axis_ = std::make_unique<MbdvAxisDriver>(*master_, options_.node_id, "Axis 1", "AX1");
    LogInfo(Stage::S03_MASTER_LOAD, nullptr,
            Str("AsyncMaster ready; AX1 -> node ", static_cast<int>(options_.node_id)));
    report.Pass(Stage::S03_MASTER_LOAD, nullptr, "AsyncMaster + axis driver created");
  }
  return true;
}

bool SingleAxisController::Start(DiagnosticReport& report) {
  if (is_running_.load()) return true;
  LogBanner(Stage::S04_EVENT_LOOP, nullptr);

  is_running_.store(true);
  loop_thread_ = std::thread([this]() {
    try {
      lely::ev::FiberThread fiber_thrd;
      loop_->run();
    } catch (const std::exception& ex) {
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
          "event-loop thread started; master Reset() posted");
  report.Pass(Stage::S04_EVENT_LOOP, nullptr, "event loop running, master Reset posted");
  return true;
}

bool SingleAxisController::BringUp(DiagnosticReport& report,
                                  const SingleAxisOptions& bring_up_options) {
  // Bring-up may run under relaxed expectations (--selftest drops the P1-00 check).
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
  return axis_ ? axis_->BringUp(report, bring_up) : false;
}

bool SingleAxisController::SetMode(DiagnosticReport& report, CiA402Mode mode) {
  LogBanner(Stage::S10_MODE_OF_OPERATION, nullptr);
  return axis_ ? axis_->SetModeStaged(report, mode) : false;
}

bool SingleAxisController::EnableServo(DiagnosticReport& report, std::chrono::milliseconds timeout) {
  LogBanner(Stage::S12_SERVO_ON, nullptr);
  return axis_ ? axis_->EnableServoStaged(report, timeout) : false;
}

bool SingleAxisController::DisableServo(DiagnosticReport& report) {
  LogBanner(Stage::S15_SERVO_OFF, nullptr);
  return axis_ ? axis_->DisableServoStaged(report) : false;
}

void SingleAxisController::StartAlarmWatch(std::chrono::milliseconds period) {
  if (axis_) axis_->StartAlarmWatch(period);
}

void SingleAxisController::StopAlarmWatch() {
  if (axis_) axis_->StopAlarmWatch();
}

void SingleAxisController::Stop() {
  if (!is_running_.load() && !loop_) return;
  LogBanner(Stage::S16_SHUTDOWN, nullptr);
  is_running_.store(false);

  // Same ordering requirement as DualAxisController::Stop(): stop the loop, then the
  // axis driver (its destructor needs both the master and the fiber executor), then
  // the master (which holds references to the channel and timer), then the I/O.
  StopAlarmWatch();

  if (loop_) loop_->stop();
  if (loop_thread_.joinable()) loop_thread_.join();

  axis_.reset();
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

bool SingleAxisController::MoveToPositionStaged(DiagnosticReport& report, int32_t target,
                                                std::chrono::milliseconds timeout) {
  LogBanner(Stage::S13_MOTION_COMMAND, nullptr);
  return axis_ ? axis_->MoveToPositionStaged(report, target, timeout) : false;
}

bool SingleAxisController::SetVelocityStaged(DiagnosticReport& report, int32_t target,
                                             std::chrono::milliseconds settle) {
  LogBanner(Stage::S13_MOTION_COMMAND, nullptr);
  return axis_ ? axis_->SetVelocityStaged(report, target, settle) : false;
}

void SingleAxisController::PrintTelemetry() const {
  if (!axis_) return;
  std::ostringstream os;
  os << "\n+--------+------+-------------+------------------------------+--------------+"
        "--------------+------------+--------+\n"
     << "| Axis   | Node | NMT         | CiA 402 State                 | Act Position |"
        " Act Velocity | Target Hit | Frames   |\n"
     << "+--------+------+-------------+------------------------------+--------------+"
        "--------------+------------+--------+\n"
     << "| " << std::left << std::setw(6) << axis_->GetAxisName() << " | " << std::setw(4)
     << static_cast<int>(axis_->GetNodeId()) << " | " << std::setw(11)
     << (axis_->IsOperational() ? "OPERATIONAL" : (axis_->SawBootUp() ? "PRE-OP" : "NOT SEEN"))
     << " | " << std::setw(28) << cia402_state_to_string(axis_->GetCiA402State()) << " | "
     << std::right << std::setw(12) << axis_->GetActualPosition() << " | " << std::setw(12)
     << axis_->GetActualVelocity() << " | " << std::setw(10)
     << (axis_->IsTargetReached() ? "YES" : "no") << " | " << std::setw(7)
     << axis_->GetStatuswordCount() << "/" << axis_->GetStatuswordSdoReads() << " |\n"
     << "+--------+------+-------------+------------------------------+--------------+"
        "--------------+------------+--------+\n";
  if (axis_->IsTelemetryStale(std::chrono::milliseconds(500))) {
    os << ">>> WARNING: no Statusword for >500 ms -> check stage S09 (TPDO1 event timer).\n";
  }
  std::cout << os.str() << std::endl;
}

void SingleAxisController::PrintDriveDiagnostics() const {
  if (!axis_) return;
  const DriveSnapshot snap = const_cast<MbdvAxisDriver&>(*axis_).ReadDriveSnapshot();
  std::ostringstream os;
  os << "\n===== " << axis_->GetAxisName() << " (node " << static_cast<int>(axis_->GetNodeId())
     << ") =====\n"
     << FormatDriveSnapshot(snap)
     << "      0x6041 decode            : " << DecodeStatusword(snap.statusword) << '\n'
     << "      telemetry                 : " << axis_->GetStatuswordCount() << " Statusword frames"
     << (axis_->IsTelemetryStale(std::chrono::milliseconds(500)) ? " (STALE)" : " (live)") << '\n';
  std::cout << os.str() << std::endl;
}

}  // namespace mbdv