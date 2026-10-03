#pragma once

#include "mbdv/diff_drive_kinematics.hpp"
#include "mbdv/mbdv_axis_driver.hpp"

#include <lely/coapp/master.hpp>
#include <lely/ev/loop.hpp>
#include <lely/io2/linux/can.hpp>
#include <lely/io2/posix/poll.hpp>
#include <lely/io2/sys/io.hpp>
#include <lely/io2/sys/timer.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

namespace mbdv {

/**
 * @brief Everything the dual-axis master needs, in one place.
 */
struct ControllerOptions {
  // --- bus ---
  std::string can_interface{"can0"};
  std::string dcf_path{"config/master.dcf"};
  std::string bin_path;  ///< empty -> only the DCF is used

  // --- nodes ---
  uint8_t axis1_node_id{1};
  uint8_t axis2_node_id{2};

  // --- expectations checked in stage S08 (0 -> skip that check) ---
  uint32_t expect_bitrate_bps{500000};
  uint32_t expect_control_mode{static_cast<uint32_t>(DriveControlMode::kPositionControl)};
  /// When non-zero, S08 writes this value to 0x2A30 (P1-00) before checking it.
  uint32_t write_control_mode{0};
  /// Warn in S08 when Statusword bit 4 reports the main voltage absent.
  bool check_dc_bus{true};
  /// Retry a failed Controlword over SDO. Disable to test the RPDO path in isolation.
  bool sdo_controlword_fallback{true};
  /// Send setpoints over SDO instead of RPDO (drives that ignore received RPDOs).
  bool sdo_setpoints{false};
  /// 0x2060 communication watchdog: -1 untouched, 0 disabled, >0 timeout in ms.
  int32_t watchdog_timeout_ms{-1};
  /// Persist a --p1-00 change with 0x1010:01 = 1 (opt-in, see README).
  bool store_parameters{false};

  // --- behaviour ---
  CiA402Mode mode{CiA402Mode::PROFILE_POSITION};
  PdoPlan pdo{};
  std::chrono::milliseconds boot_timeout{3000};
  std::chrono::milliseconds servo_timeout{2000};

  // --- logging ---
  LogLevel log_level{LogLevel::INFO};
  std::string log_file;  ///< empty -> console only
  bool colour{true};

  // --- test script ---
  int32_t step_counts{10000};   ///< position-test stroke in encoder counts
  int32_t test_velocity{5000};  ///< velocity-test setpoint in counts/s
};

/**
 * @brief Coordinates dual-axis control of the Moons' MBDV-2X-520AC servo drive.
 *
 * Owns the lely CANopen master, the event-loop thread and the two axis drivers.
 * Every lifecycle step is a numbered Stage and is recorded in a DiagnosticReport
 * so a failure can be traced to exactly one step.
 */
class DualAxisController {
 public:
  DualAxisController();
  ~DualAxisController();

  DualAxisController(const DualAxisController&) = delete;
  DualAxisController& operator=(const DualAxisController&) = delete;

  /**
   * @brief Stages S01..S03: configure logging, open the CAN link, load the DCF.
   * @return true when all three stages passed.
   */
  bool Initialize(const ControllerOptions& options, DiagnosticReport& report);

  /** Stage S04: start the event-loop thread and post master Reset. */
  bool Start(DiagnosticReport& report);

  /** Stages S05..S11 for both axes, then a summary. */
  bool BringUpBothAxes(DiagnosticReport& report,
                       const ControllerOptions& bring_up_options = ControllerOptions{});

  /** Stage S10 for both axes. */
  bool SetModeBothAxes(DiagnosticReport& report, CiA402Mode mode);

  /** Stage S12 for both axes. */
  bool EnableBothAxes(DiagnosticReport& report, std::chrono::milliseconds timeout);

  /** Stage S15 for both axes. */
  void DisableBothAxes(DiagnosticReport& report);

  /** Stages S16 + S02 teardown. */
  /// Starts the continuous 0x200F / 0x1001 alarm watch on both axes.
  void StartAlarmWatch(std::chrono::milliseconds period);
  void StopAlarmWatch();

  void Stop();

  // --- motion helpers (unstaged, used by the interactive monitor) ---
  void MoveBothAxes(int32_t pos1, int32_t pos2, bool relative = false);
  void SetBothVelocities(int32_t vel1, int32_t vel2);
  void SetCmdVel(double linear_v, double angular_w);
  bool MoveBothAxesStaged(DiagnosticReport& report, int32_t pos1, int32_t pos2,
                          std::chrono::milliseconds timeout);
  bool SetVelocitiesStaged(DiagnosticReport& report, int32_t vel1, int32_t vel2,
                           std::chrono::milliseconds settle);

  void UpdateOdometry(double dt_sec);

  RobotPose GetRobotPose() const { return kinematics_.GetPose(); }
  RobotTwist GetRobotTwist() const { return kinematics_.GetTwist(); }
  void ResetOdometry(double x = 0.0, double y = 0.0, double theta = 0.0) {
    kinematics_.ResetPose(x, y, theta);
  }

  DiffDriveKinematics& GetKinematics() noexcept { return kinematics_; }
  const DiffDriveKinematics& GetKinematics() const noexcept { return kinematics_; }

  /** Prints a two-axis telemetry table plus the integrated odometry. */
  void PrintTelemetry() const;

  /** Prints every diagnostic object of both axes (blocking SDO reads). */
  void PrintDriveDiagnostics() const;

  MbdvAxisDriver& GetAxis1() { return *axis1_; }
  const MbdvAxisDriver& GetAxis1() const { return *axis1_; }
  MbdvAxisDriver& GetAxis2() { return *axis2_; }
  const MbdvAxisDriver& GetAxis2() const { return *axis2_; }

  bool IsRunning() const noexcept { return is_running_.load(); }
  const ControllerOptions& Options() const noexcept { return options_; }

 private:
  ControllerOptions options_;

  std::unique_ptr<lely::io::IoGuard> io_guard_;
  std::unique_ptr<lely::io::Context> ctx_;
  std::unique_ptr<lely::io::Poll> poll_;
  std::unique_ptr<lely::ev::Loop> loop_;
  std::unique_ptr<lely::io::Timer> timer_;
  std::unique_ptr<lely::io::CanController> ctrl_;
  std::unique_ptr<lely::io::CanChannel> chan_;
  std::unique_ptr<lely::canopen::AsyncMaster> master_;

  std::unique_ptr<MbdvAxisDriver> axis1_;
  std::unique_ptr<MbdvAxisDriver> axis2_;

  DiffDriveKinematics kinematics_;

  std::string original_cwd_;  ///< restored by Stop(); see config_path.hpp
  std::thread loop_thread_;
  std::atomic<bool> is_running_{false};
};

}  // namespace mbdv