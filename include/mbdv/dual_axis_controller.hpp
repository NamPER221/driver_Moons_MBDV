#pragma once

#include "mbdv/mbdv_axis_driver.hpp"
#include "mbdv/diff_drive_kinematics.hpp"

#include <lely/ev/loop.hpp>
#include <lely/io2/linux/can.hpp>
#include <lely/io2/posix/poll.hpp>
#include <lely/io2/sys/io.hpp>
#include <lely/io2/sys/timer.hpp>
#include <lely/coapp/master.hpp>

#include <memory>
#include <string>
#include <thread>
#include <atomic>

namespace mbdv {

/**
 * @brief Coordinates dual-axis control for Moons' MBDV Servo Drive.
 *
 * Manages the Lely CANopen Master, event loop execution thread, and both
 * Axis 1 (Node 1) and Axis 2 (Node 2) instances.
 */
class DualAxisController {
 public:
  DualAxisController();
  ~DualAxisController();

  // Non-copyable, non-movable
  DualAxisController(const DualAxisController&) = delete;
  DualAxisController& operator=(const DualAxisController&) = delete;

  /**
   * @brief Initializes CAN channel, AsyncMaster, and Axis Drivers.
   * @param can_interface SocketCAN interface (e.g. "can0", "vcan0").
   * @param dcf_path Path to master.dcf file.
   * @param bin_path Optional path to master.bin file.
   * @param axis1_node_id Node ID for Axis 1 (default 1).
   * @param axis2_node_id Node ID for Axis 2 (default 2).
   */
  bool Initialize(const std::string& can_interface, const std::string& dcf_path,
                  const std::string& bin_path = "", uint8_t axis1_node_id = 1,
                  uint8_t axis2_node_id = 2);

  /**
   * @brief Starts the CANopen master event loop in a dedicated thread.
   */
  void Start();

  /**
   * @brief Stops the master, disables drives, and cleans up resources.
   */
  void Stop();

  /**
   * @brief Simultaneously commands Servo ON for both axes.
   * @return true if both axes reached OPERATION_ENABLED.
   */
  bool EnableBothAxes(std::chrono::milliseconds timeout = std::chrono::milliseconds(3000));

  /**
   * @brief Disables both axes (Servo OFF).
   */
  void DisableBothAxes();

  /**
   * @brief Sends simultaneous position setpoints to both axes.
   * @param pos1 Target position for Axis 1.
   * @param pos2 Target position for Axis 2.
   * @param relative If true, target position is relative to current setpoint.
   */
  void MoveBothAxes(int32_t pos1, int32_t pos2, bool relative = false);

  /**
   * @brief Sends simultaneous velocity setpoints to both axes.
   * @param vel1 Target velocity for Axis 1.
   * @param vel2 Target velocity for Axis 2.
   */
  void SetBothVelocities(int32_t vel1, int32_t vel2);

  /**
   * @brief Sets operation mode for both axes (e.g. Profile Position or Profile Velocity).
   */
  void SetModeBothAxes(CiA402Mode mode);

  /**
   * @brief Kinematic Control: Sets robot cmd_vel (v, w) using differential drive inverse kinematics.
   * Converts linear and angular velocity to left/right wheel driver speeds (0x60FF) and transmits via RPDO3.
   * @param linear_v Linear velocity in m/s.
   * @param angular_w Angular velocity in rad/s.
   */
  void SetCmdVel(double linear_v, double angular_w);

  /**
   * @brief Kinematic Odometry: Updates robot odometry pose (x, y, theta) from encoder feedback (0x6064).
   * @param dt_sec Elapsed time interval in seconds.
   */
  void UpdateOdometry(double dt_sec);

  RobotPose GetRobotPose() const { return kinematics_.GetPose(); }
  RobotTwist GetRobotTwist() const { return kinematics_.GetTwist(); }
  void ResetOdometry(double x = 0.0, double y = 0.0, double theta = 0.0) {
    kinematics_.ResetPose(x, y, theta);
  }

  DiffDriveKinematics& GetKinematics() noexcept { return kinematics_; }
  const DiffDriveKinematics& GetKinematics() const noexcept { return kinematics_; }

  /**
   * @brief Prints formatted real-time status of both axes and integrated odometry.
   */
  void PrintTelemetry() const;

  // Accessors
  MbdvAxisDriver& GetAxis1() { return *axis1_; }
  const MbdvAxisDriver& GetAxis1() const { return *axis1_; }

  MbdvAxisDriver& GetAxis2() { return *axis2_; }
  const MbdvAxisDriver& GetAxis2() const { return *axis2_; }

  bool IsRunning() const noexcept { return is_running_.load(); }

 private:
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

  std::thread loop_thread_;
  std::atomic<bool> is_running_{false};
};

}  // namespace mbdv
