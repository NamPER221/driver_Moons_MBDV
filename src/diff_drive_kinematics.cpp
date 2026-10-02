#include "mbdv/diff_drive_kinematics.hpp"

#include <algorithm>
#include <cmath>

namespace mbdv {

DiffDriveKinematics::DiffDriveKinematics(const KinematicsConfig& config)
    : config_(config) {}

WheelSpeeds DiffDriveKinematics::ComputeWheelSpeeds(double linear_v,
                                                   double angular_w) const {
  // Velocity Clamping to prevent actuator over-speed
  double v_clamped = std::max(-config_.max_linear_velocity,
                              std::min(config_.max_linear_velocity, linear_v));
  double w_clamped = std::max(-config_.max_angular_velocity,
                              std::min(config_.max_angular_velocity, angular_w));

  // Inverse Kinematics for Differential Drive:
  // omega_left  = (v - w * L / 2) / r
  // omega_right = (v + w * L / 2) / r
  double half_base = config_.wheel_base / 2.0;
  double w_left_rad = (v_clamped - (w_clamped * half_base)) / config_.wheel_radius;
  double w_right_rad = (v_clamped + (w_clamped * half_base)) / config_.wheel_radius;

  // Convert angular velocity (rad/s) to driver pulse frequency (counts/s):
  // 1 rev = 2*pi rad = (CPR * N) counts
  // driver_vel = (omega * CPR * N) / (2 * pi)
  double counts_per_rad =
      (static_cast<double>(config_.encoder_cpr) * config_.gear_ratio) / (2.0 * M_PI);

  WheelSpeeds speeds;
  speeds.left_rad_per_s = w_left_rad;
  speeds.right_rad_per_s = w_right_rad;
  speeds.left_driver_vel = static_cast<int32_t>(std::round(w_left_rad * counts_per_rad));
  speeds.right_driver_vel = static_cast<int32_t>(std::round(w_right_rad * counts_per_rad));

  return speeds;
}

void DiffDriveKinematics::UpdateOdometryFromTicks(int32_t left_ticks,
                                                int32_t right_ticks,
                                                double dt_sec) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (first_run_) {
    prev_left_ticks_ = left_ticks;
    prev_right_ticks_ = right_ticks;
    first_run_ = false;
    return;
  }

  int32_t delta_left_ticks = left_ticks - prev_left_ticks_;
  int32_t delta_right_ticks = right_ticks - prev_right_ticks_;

  prev_left_ticks_ = left_ticks;
  prev_right_ticks_ = right_ticks;

  // Linear displacement per wheel
  double meters_per_count = (2.0 * M_PI * config_.wheel_radius) /
                            (static_cast<double>(config_.encoder_cpr) * config_.gear_ratio);
  double d_left = delta_left_ticks * meters_per_count;
  double d_right = delta_right_ticks * meters_per_count;

  // Forward Kinematics:
  // d_s     = (d_right + d_left) / 2
  // d_theta = (d_right - d_left) / L
  double d_s = (d_right + d_left) / 2.0;
  double d_theta = (d_right - d_left) / config_.wheel_base;

  // Runge-Kutta 2nd order (midpoint) pose integration
  double mid_theta = pose_.theta + (d_theta / 2.0);
  pose_.x += d_s * std::cos(mid_theta);
  pose_.y += d_s * std::sin(mid_theta);
  pose_.theta = NormalizeAngle(pose_.theta + d_theta);

  // Velocity derivation
  if (dt_sec > 1e-4) {
    twist_.linear_v = d_s / dt_sec;
    twist_.angular_w = d_theta / dt_sec;
  }
}

RobotTwist DiffDriveKinematics::ComputeRobotTwistFromSpeeds(
    int32_t left_driver_vel, int32_t right_driver_vel) const {
  double rad_per_count =
      (2.0 * M_PI) / (static_cast<double>(config_.encoder_cpr) * config_.gear_ratio);
  double w_left = left_driver_vel * rad_per_count;
  double w_right = right_driver_vel * rad_per_count;

  RobotTwist tw;
  // linear  = r * (w_right + w_left) / 2
  // angular = r * (w_right - w_left) / L
  tw.linear_v = config_.wheel_radius * (w_right + w_left) / 2.0;
  tw.angular_w = config_.wheel_radius * (w_right - w_left) / config_.wheel_base;
  return tw;
}

RobotPose DiffDriveKinematics::GetPose() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return pose_;
}

RobotTwist DiffDriveKinematics::GetTwist() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return twist_;
}

void DiffDriveKinematics::ResetPose(double x, double y, double theta) {
  std::lock_guard<std::mutex> lock(mutex_);
  pose_.x = x;
  pose_.y = y;
  pose_.theta = NormalizeAngle(theta);
  first_run_ = true;
}

double DiffDriveKinematics::NormalizeAngle(double angle) {
  while (angle > M_PI) {
    angle -= 2.0 * M_PI;
  }
  while (angle < -M_PI) {
    angle += 2.0 * M_PI;
  }
  return angle;
}

}  // namespace mbdv
