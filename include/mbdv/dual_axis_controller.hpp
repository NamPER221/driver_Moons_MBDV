#pragma once

#include "mbdv/can_sniffer.hpp"
#include "mbdv/diff_drive_kinematics.hpp"
#include "mbdv/mbdv_axis_driver.hpp"
#include "mbdv/odometry_publisher.hpp"
#include "mbdv/params.hpp"

#include <lely/coapp/master.hpp>
#include <lely/ev/loop.hpp>
#include <lely/io2/linux/can.hpp>
#include <lely/io2/posix/poll.hpp>
#include <lely/io2/sys/io.hpp>
#include <lely/io2/sys/timer.hpp>

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
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
  /// Profile ramp written to 0x6083/0x6084 in S08 (counts/s^2). 0 = leave the drive's own.
  uint32_t profile_accel{25000};
  uint32_t profile_decel{50000};
  /// 0x2060 communication watchdog: -1 untouched, 0 disabled, >0 timeout in ms.
  int32_t watchdog_timeout_ms{-1};

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

/// Where one axis is in the liveness / reconnect state machine.
enum class AxisHealth : int {
  kUnknown = 0,  ///< bring-up has not finished yet
  kAlive,        ///< heartbeat arriving, Statusword fresh, servo enabled
  kLost,         ///< node silent or not OPERATIONAL; needs a full reconnect (S05..S12)
  kFaulted,      ///< node answers but the servo left Operation Enabled; needs reset + enable
  kRecovering,   ///< a recovery attempt is running on a worker thread
  kFailed,       ///< recovery gave up (max_attempts reached); needs operator action
};

const char* axis_health_to_string(AxisHealth health) noexcept;

/// Result of one recovery attempt, produced on a worker thread.
struct RecoveryOutcome {
  bool ok{false};
  /// The servo refused for a reason only hardware can clear (STO, a limit/E-STOP input, no
  /// main power). Such attempts are retried but not counted against max_attempts.
  bool hardware_cause{false};
  std::string detail;
};

/// Per-axis supervisor bookkeeping. Public so the telemetry line can show it.
struct AxisSupervisor {
  /// Atomic: Supervise() runs on the control thread while the telemetry line reads it.
  std::atomic<AxisHealth> health{AxisHealth::kUnknown};
  std::atomic<uint32_t> attempts{0};
  std::atomic<uint32_t> recoveries{0};
  std::atomic<int32_t> last_known_counts{0};
  /// Only touched by Supervise() on the control thread.
  std::chrono::steady_clock::time_point next_attempt{};
  std::chrono::milliseconds backoff{};
  std::string last_reason{"-"};
  /// What the running (or last) recovery task is for: kLost or kFaulted.
  AxisHealth recovering_from{AxisHealth::kUnknown};
  /// Counted attempts in the current loss/fault episode; reset when the axis is back.
  uint32_t episode_attempts{0};
  /// Set when a reconnect brought the node back with its servo off: it is then re-enabled
  /// even when fault_recovery.auto_reset is false.
  bool enable_after_reconnect{false};
  /// The recovery attempt in flight, if any. Never waited on in Supervise() - only polled.
  std::future<RecoveryOutcome> task;
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

  /**
   * @brief Applies @p params: rates, heartbeat periods, reconnect policy, kinematics and
   * the odometry output. Must be called before Initialize().
   *
   * @return false with @p error set when the odometry UDP socket could not be opened;
   *         that is the only configuration failure, and it is a warning-level one because
   *         the control loop itself is unaffected.
   */
  bool Configure(const Params& params, std::string* error);

  /// Period the control loop should use, from rates.control_hz.
  std::chrono::milliseconds ControlPeriod() const;
  const Params& GetParams() const noexcept { return params_; }

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

  // ------------------------------------------------------------------
  // Supervision: keep running when something goes away
  // ------------------------------------------------------------------

  /**
   * @brief Advances the liveness, reconnect and link-recovery state machines.
   *
   * Non-blocking: recovery attempts run on a worker thread (std::async) and are only
   * polled here, so the control loop keeps its rate while an axis is being reconnected.
   *
   * An enabled axis that goes away - CAN connection lost (heartbeat timeout, or the
   * heartbeat reporting the node out of OPERATIONAL), or its servo leaving Operation
   * Enabled - triggers EmergencyStopAll() at once, then:
   *   - kLost    -> wait for lely's own reset/boot of the node, re-run S05..S10 for it with
   *                 the servo left off, then treat it as kFaulted;
   *   - kFaulted -> fault reset (controlword bit 7, then 0x2006) and re-enable, but only
   *                 once the stop output is off (a held E-STOP input blocks enabling).
   * It also releases the stop output when it is safe to (see StopRequested()).
   * A CAN controller error re-opens the channel on the event-loop thread; if the link is
   * really gone the heartbeat timeout is what stops the robot.
   */
  void Supervise();

  /// True when both axes are alive and enabled. The odometry sample carries this.
  bool AllAxesAlive() const;

  // ------------------------------------------------------------------
  // Stop output - for a hardware IO
  // ------------------------------------------------------------------

  /**
   * @brief The stop output: true while the robot must be held stopped.
   *
   * Meant to drive a hardware IO, because once the CAN link to a drive is gone nothing sent
   * over CAN can stop it - typically the drives' E-STOP inputs (MBDV-2X 1_X4 / 2_X4, P5-03,
   * hardware manual 7.1.7). Thread-safe; poll it from any thread, or use SetStopCallback().
   *
   * Turns ON (EmergencyStopAll()) the moment any enabled axis is lost or faults, and when
   * the controller shuts down. Turns OFF in Supervise() only when all of these hold:
   *   - every node's heartbeat is back and reports NMT OPERATIONAL, no reconnect is still
   *     running and no axis has given up (kFailed);
   *   - the last SetCmdVel() request is zero;
   *   - it has been on for at least liveness.stop_hold_ms.
   * Servos switched off meanwhile are re-enabled only after it turns OFF, at zero speed, and
   * motion still needs a new command - the robot never restarts by itself.
   */
  bool StopRequested() const noexcept { return stop_latched_.load(); }

  /// Invoked on the control thread on every change of StopRequested(): (true, reason) or
  /// (false, ""). Called before anything else is done about the stop, so keep it short and
  /// non-blocking. It must stay valid until Stop() has returned.
  using StopCallback = std::function<void(bool stop, const std::string& reason)>;
  void SetStopCallback(StopCallback cb) { stop_callback_ = std::move(cb); }

  /// Why the stop output is on. Control thread only, like Supervise().
  const std::string& StopReason() const noexcept { return stop_reason_; }

  /**
   * @brief Turns the stop output on and commands every axis CAN still reaches to zero.
   *
   * Called by Supervise() the moment any axis is lost or faults: with one wheel gone a
   * differential-drive robot can only spin, so the other wheel must not keep driving.
   */
  void EmergencyStopAll(const std::string& reason);

  /// True when motion commands are accepted: both axes alive and the stop output off.
  bool MotionPermitted() const { return AllAxesAlive() && !StopRequested(); }
  const AxisSupervisor& Supervisor1() const noexcept { return sup1_; }
  const AxisSupervisor& Supervisor2() const noexcept { return sup2_; }
  AxisHealth Health1() const noexcept { return sup1_.health.load(); }
  AxisHealth Health2() const noexcept { return sup2_.health.load(); }

  /// Registers the in-process odometry subscriber, invoked at the control rate.
  void SetOdometryCallback(OdometryPublisher::Callback cb) {
    publisher_.SetCallback(std::move(cb));
  }
  const OdometryPublisher& Publisher() const noexcept { return publisher_; }

  /**
   * @brief Integrates odometry for @p dt_sec and publishes one sample.
   *
   * A lost axis contributes its last known encoder reading, so a dead axis freezes the
   * odometry instead of corrupting it, and the sample is marked not alive.
   */
  void UpdateAndPublishOdometry(double dt_sec);

  void Stop();

  // --- motion ---
  /**
   * @brief Applies a body-frame twist through the differential-drive kinematics.
   *
   * This is the single entry point the keyboard teleop uses: (v, w) is converted to
   * left/right wheel speeds and written to 0x60FF on both axes, so the two servos are
   * always commanded as a coordinated pair rather than independently.
   *
   * Not applied while an axis is not alive or the stop output is on, but always recorded:
   * the stop output is only released while the request is zero (see StopRequested()).
   */
  void SetCmdVel(double linear_v, double angular_w);
  bool MoveBothAxesStaged(DiagnosticReport& report, int32_t pos1, int32_t pos2,
                          std::chrono::milliseconds timeout);
  bool SetVelocitiesStaged(DiagnosticReport& report, int32_t vel1, int32_t vel2,
                           std::chrono::milliseconds settle);

  RobotPose GetRobotPose() const { return kinematics_.GetPose(); }
  RobotTwist GetRobotTwist() const { return kinematics_.GetTwist(); }
  void ResetOdometry(double x = 0.0, double y = 0.0, double theta = 0.0) {
    kinematics_.ResetPose(x, y, theta);
  }

  /** Prints a two-axis telemetry table plus the integrated odometry. */
  void PrintTelemetry() const;

  /** Prints every diagnostic object of both axes (blocking SDO reads). */
  void PrintDriveDiagnostics() const;

  MbdvAxisDriver& GetAxis1() { return *axis1_; }
  const MbdvAxisDriver& GetAxis1() const { return *axis1_; }
  MbdvAxisDriver& GetAxis2() { return *axis2_; }
  const MbdvAxisDriver& GetAxis2() const { return *axis2_; }


 private:
  ControllerOptions options_;

  std::unique_ptr<lely::io::IoGuard> io_guard_;
  std::unique_ptr<lely::io::Context> ctx_;
  std::unique_ptr<lely::io::Poll> poll_;
  std::unique_ptr<lely::ev::Loop> loop_;
  std::unique_ptr<lely::io::Timer> timer_;
  std::unique_ptr<lely::io::CanController> ctrl_;
  std::unique_ptr<lely::io::CanChannel> chan_;
  /// Serialises raw RPDO sends against each other and lely's own writes.
  std::unique_ptr<std::mutex> rpdo_mutex_;
  /// Receive-only mirror of the bus, used to decode each node's TPDO from raw bytes.
  std::unique_ptr<CanSniffer> sniffer_;
  std::unique_ptr<lely::canopen::AsyncMaster> master_;

  std::unique_ptr<MbdvAxisDriver> axis1_;
  std::unique_ptr<MbdvAxisDriver> axis2_;

  DiffDriveKinematics kinematics_;

  Params params_{};
  BringUpOptions bring_up_{};
  OdometryPublisher publisher_{};
  AxisSupervisor sup1_{};
  AxisSupervisor sup2_{};
  std::chrono::milliseconds control_period_{5};
  std::chrono::milliseconds controlword_period_{50};
  std::chrono::steady_clock::time_point last_controlword_refresh_{};
  std::chrono::steady_clock::time_point start_time_{};
  std::atomic<bool> link_recovery_in_progress_{false};
  uint32_t link_recoveries_{0};
  std::chrono::steady_clock::time_point next_link_attempt_{};

  /// Runs one recovery attempt for @p ax on a worker thread; see Supervise().
  RecoveryOutcome RunRecovery(MbdvAxisDriver* ax, AxisHealth from);
  /// Turns the stop output off and tells the callback; see StopRequested().
  void ReleaseStop();

  /// The stop output; see StopRequested().
  std::atomic<bool> stop_latched_{false};
  /// The rest is control thread only.
  std::string stop_reason_;
  std::chrono::steady_clock::time_point stop_since_{};
  StopCallback stop_callback_;
  /// Last (v, w) asked of SetCmdVel(), applied or not: the stop output is only released
  /// while this is zero, so a client still asking for motion never restarts the robot.
  double requested_v_{0.0};
  double requested_w_{0.0};

  std::string original_cwd_;  ///< restored by Stop(); see config_path.hpp
  std::thread loop_thread_;
  std::atomic<bool> is_running_{false};

  double last_cmd_v_{-999.0};
  double last_cmd_w_{-999.0};
};

}  // namespace mbdv