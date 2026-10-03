#pragma once

#include "mbdv/mbdv_axis_driver.hpp"

#include <lely/coapp/master.hpp>
#include <lely/ev/loop.hpp>
#include <lely/io2/linux/can.hpp>
#include <lely/io2/posix/poll.hpp>
#include <lely/io2/sys/io.hpp>
#include <lely/io2/sys/timer.hpp>

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace mbdv {

struct SingleAxisOptions {
  std::string can_interface{"can0"};
  std::string dcf_path{"config/single_axis_500k/master.dcf"};
  std::string bin_path;
  uint8_t node_id{1};
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
  CiA402Mode mode{CiA402Mode::PROFILE_POSITION};
  PdoPlan pdo{};
  std::chrono::milliseconds boot_timeout{3000};
  std::chrono::milliseconds servo_timeout{2000};
  LogLevel log_level{LogLevel::INFO};
  std::string log_file;
  bool colour{true};
  int32_t step_counts{10000};
  int32_t test_velocity{5000};
};

/**
 * @brief Single-axis (Axis 1) controller for the Moons' MBDV servo drive.
 *
 * Same staged bring-up as DualAxisController, restricted to one node. Kept so a
 * single channel can be commissioned before both axes share the bus.
 */
class SingleAxisController {
 public:
  SingleAxisController();
  ~SingleAxisController();

  SingleAxisController(const SingleAxisController&) = delete;
  SingleAxisController& operator=(const SingleAxisController&) = delete;

  bool Initialize(const SingleAxisOptions& options, DiagnosticReport& report);
  bool Start(DiagnosticReport& report);

  /// Stages S05..S11 for the single node.
  bool BringUp(DiagnosticReport& report,
             const SingleAxisOptions& bring_up_options = SingleAxisOptions{});

  bool SetMode(DiagnosticReport& report, CiA402Mode mode);
  bool EnableServo(DiagnosticReport& report, std::chrono::milliseconds timeout);
  bool DisableServo(DiagnosticReport& report);
  /// Starts the continuous 0x200F / 0x1001 alarm watch.
  void StartAlarmWatch(std::chrono::milliseconds period);
  void StopAlarmWatch();

  void Stop();

  bool MoveToPositionStaged(DiagnosticReport& report, int32_t target,
                            std::chrono::milliseconds timeout);
  bool SetVelocityStaged(DiagnosticReport& report, int32_t target,
                         std::chrono::milliseconds settle);

  void PrintTelemetry() const;
  void PrintDriveDiagnostics() const;

  MbdvAxisDriver& GetAxis() { return *axis_; }
  const MbdvAxisDriver& GetAxis() const { return *axis_; }

  bool IsRunning() const noexcept { return is_running_.load(); }
  const SingleAxisOptions& Options() const noexcept { return options_; }

 private:
  SingleAxisOptions options_;

  std::unique_ptr<lely::io::IoGuard> io_guard_;
  std::unique_ptr<lely::io::Context> ctx_;
  std::unique_ptr<lely::io::Poll> poll_;
  std::unique_ptr<lely::ev::Loop> loop_;
  std::unique_ptr<lely::io::Timer> timer_;
  std::unique_ptr<lely::io::CanController> ctrl_;
  std::unique_ptr<lely::io::CanChannel> chan_;
  std::unique_ptr<lely::canopen::AsyncMaster> master_;

  std::unique_ptr<MbdvAxisDriver> axis_;

  std::string original_cwd_;  ///< restored by Stop(); see config_path.hpp
  std::thread loop_thread_;
  std::atomic<bool> is_running_{false};
};

}  // namespace mbdv