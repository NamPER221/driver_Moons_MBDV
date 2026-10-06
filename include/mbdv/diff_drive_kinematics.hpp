#pragma once

#include <cmath>
#include <cstdint>
#include <mutex>

namespace mbdv {

/**
 * @brief Kinematic configuration parameters for differential drive mobile base.
 */
struct KinematicsConfig {
  double wheel_radius{0.07333};       // r [m]
  double wheel_base{0.4544};          // L [m]
  double gear_ratio{1.0};             // N (Gear reduction ratio)
  int32_t encoder_cpr{10000};         // Counts Per Revolution (CPR) of motor encoder
  double max_linear_velocity{1.5};    // Max linear velocity clamp [m/s]
  double max_angular_velocity{3.0};   // Max angular velocity clamp [rad/s]
};

/**
 * @brief Robot velocity in body frame (linear x, angular z).
 */
struct RobotTwist {
  double linear_v{0.0};   // v [m/s]
  double angular_w{0.0};  // omega [rad/s]
};

/**
 * @brief Robot pose in 2D odometry frame (x, y, theta).
 */
struct RobotPose {
  double x{0.0};      // [m]
  double y{0.0};      // [m]
  double theta{0.0};  // [rad], normalized to [-pi, pi]
};

/**
 * @brief Wheel speed targets in SI units (rad/s) and driver pulse frequency (counts/s).
 */
struct WheelSpeeds {
  double left_rad_per_s{0.0};   // [rad/s]
  double right_rad_per_s{0.0};  // [rad/s]
  int32_t left_driver_vel{0};   // Object 0x60FF [counts/s]
  int32_t right_driver_vel{0};  // Object 0x60FF [counts/s]
};

/**
 * @brief Differential drive kinematics calculator and odometry integrator.
 */
class DiffDriveKinematics {
 public:
  explicit DiffDriveKinematics(const KinematicsConfig& config = KinematicsConfig());

  /**
   * @brief Replaces the geometry and limits, keeping the integrated pose.
   *
   * A setter rather than assignment because the class holds a mutex, so it is neither
   * copyable nor movable. The pose is deliberately preserved: a geometry correction
   * should not teleport the robot, even though it does invalidate the wheel travel
   * already integrated with the old numbers.
   */
  void SetConfig(const KinematicsConfig& config);

  /**
   * @brief Inverse Kinematics: Converts robot body twist (v, w) into left/right wheel speeds.
   * Clamps velocities to configured maximums to protect actuators.
   */
  WheelSpeeds ComputeWheelSpeeds(double linear_v, double angular_w) const;

  /**
   * @brief Forward Kinematics: Integrates odometry pose from accumulated encoder counts.
   * @param left_ticks Actual position from Axis 1 (Object 0x6064).
   * @param right_ticks Actual position from Axis 2 (Object 0x6064).
   * @param dt_sec Elapsed time interval in seconds (unused: the twist comes from
   *               UpdateTwistFromSpeeds()).
   */
  void UpdateOdometryFromTicks(int32_t left_ticks, int32_t right_ticks, double dt_sec);

  /**
   * @brief Computes robot linear and angular velocity from motor velocity feedback.
   * @param left_driver_vel Actual velocity of Axis 1 (Object 0x606C, counts/s).
   * @param right_driver_vel Actual velocity of Axis 2 (Object 0x606C, counts/s).
   */
  RobotTwist ComputeRobotTwistFromSpeeds(int32_t left_driver_vel, int32_t right_driver_vel) const;

  /// ComputeRobotTwistFromSpeeds() stored as the twist GetTwist() returns.
  void UpdateTwistFromSpeeds(int32_t left_driver_vel, int32_t right_driver_vel);

  // Thread-safe accessors
  RobotPose GetPose() const;
  RobotTwist GetTwist() const;
  void ResetPose(double x = 0.0, double y = 0.0, double theta = 0.0);

  /**
   * @brief Re-zeroes the pose AND re-baselines the encoder deltas.
   *
   * ResetPose() on its own only moves the pose, so the next UpdateOdometryFromTicks()
   * still sees the previous tick values and integrates one huge phantom step. That is
   * exactly what happens after a drive reconnects: the drive may have restarted and
   * reported position 0 while the integrated pose is somewhere else entirely. This
   * makes both the pose and the delta reference change at the same instant.
   */
  void Realign(int32_t left_ticks, int32_t right_ticks);

 private:
  static double NormalizeAngle(double angle);

  KinematicsConfig config_;
  mutable std::mutex mutex_;

  RobotPose pose_{0.0, 0.0, 0.0};
  RobotTwist twist_{0.0, 0.0};

  bool first_run_{true};
  int32_t prev_left_ticks_{0};
  int32_t prev_right_ticks_{0};
};

}  // namespace mbdv
