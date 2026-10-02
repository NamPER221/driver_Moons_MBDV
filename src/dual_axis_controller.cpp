#include "mbdv/dual_axis_controller.hpp"

#include <future>
#include <iomanip>
#include <iostream>

namespace mbdv {

DualAxisController::DualAxisController() = default;

DualAxisController::~DualAxisController() {
  Stop();
}

bool DualAxisController::Initialize(const std::string& can_interface,
                                    const std::string& dcf_path,
                                    const std::string& bin_path,
                                    uint8_t axis1_node_id,
                                    uint8_t axis2_node_id) {
  std::cout << "==================================================" << std::endl;
  std::cout << " Initializing MBDV Dual-Axis CANopen Master..." << std::endl;
  std::cout << " CAN Interface: " << can_interface << std::endl;
  std::cout << " Master DCF:    " << dcf_path << std::endl;
  std::cout << " Axis 1 Node ID: " << static_cast<int>(axis1_node_id) << std::endl;
  std::cout << " Axis 2 Node ID: " << static_cast<int>(axis2_node_id) << std::endl;
  std::cout << "==================================================" << std::endl;

  io_guard_ = std::make_unique<lely::io::IoGuard>();
  ctx_ = std::make_unique<lely::io::Context>();
  poll_ = std::make_unique<lely::io::Poll>(*ctx_);
  loop_ = std::make_unique<lely::ev::Loop>(poll_->get_poll());
  auto exec = loop_->get_executor();
  timer_ = std::make_unique<lely::io::Timer>(*poll_, exec, CLOCK_MONOTONIC);
  ctrl_ = std::make_unique<lely::io::CanController>(can_interface.c_str());
  chan_ = std::make_unique<lely::io::CanChannel>(*poll_, exec);

  try {
    chan_->open(*ctrl_);
  } catch (const std::exception& ex) {
    std::cerr << "ERROR: Failed to open CAN channel on '" << can_interface
              << "': " << ex.what() << std::endl;
    return false;
  }

  try {
    master_ = std::make_unique<lely::canopen::AsyncMaster>(
        *timer_, *chan_, dcf_path, bin_path);
  } catch (const std::exception& ex) {
    std::cerr << "ERROR: Failed to instantiate CANopen AsyncMaster: "
              << ex.what() << std::endl;
    return false;
  }

  axis1_ = std::make_unique<MbdvAxisDriver>(*master_, axis1_node_id, "Axis_1");
  axis2_ = std::make_unique<MbdvAxisDriver>(*master_, axis2_node_id, "Axis_2");

  std::cout << "CANopen Master and Dual-Axis Drivers initialized successfully."
            << std::endl;
  return true;
}

void DualAxisController::Start() {
  if (is_running_.load()) {
    return;
  }
  is_running_.store(true);
  std::cout << "Starting CANopen event loop thread..." << std::endl;
  loop_thread_ = std::thread([this]() {
    try {
      loop_->run();
    } catch (const std::exception& ex) {
      std::cerr << "Exception in CANopen loop thread: " << ex.what()
                << std::endl;
    }
  });
}

void DualAxisController::Stop() {
  if (!is_running_.load()) {
    return;
  }
  std::cout << "Stopping CANopen Dual Axis Controller..." << std::endl;
  is_running_.store(false);

  DisableBothAxes();

  if (loop_) {
    loop_->stop();
  }
  if (loop_thread_.joinable()) {
    loop_thread_.join();
  }

  std::cout << "CANopen Dual Axis Controller stopped." << std::endl;
}

bool DualAxisController::EnableBothAxes(std::chrono::milliseconds timeout) {
  std::cout << ">>> Enabling Both Axis 1 & Axis 2 Simultaneously..." << std::endl;

  // Run both enable sequences concurrently
  auto fut1 = std::async(std::launch::async, [this, timeout]() {
    return axis1_->EnableServo(timeout);
  });
  auto fut2 = std::async(std::launch::async, [this, timeout]() {
    return axis2_->EnableServo(timeout);
  });

  bool ok1 = fut1.get();
  bool ok2 = fut2.get();

  if (ok1 && ok2) {
    std::cout << ">>> Both axes successfully reached OPERATION_ENABLED (Servo ON)!"
              << std::endl;
    return true;
  } else {
    std::cerr << ">>> Failed to enable both axes. Axis1=" << (ok1 ? "OK" : "FAIL")
              << ", Axis2=" << (ok2 ? "OK" : "FAIL") << std::endl;
    return false;
  }
}

void DualAxisController::DisableBothAxes() {
  std::cout << ">>> Disabling Both Axes (Servo OFF)..." << std::endl;
  if (axis1_) axis1_->DisableServo();
  if (axis2_) axis2_->DisableServo();
}

void DualAxisController::SetModeBothAxes(CiA402Mode mode) {
  std::cout << ">>> Setting Mode for Both Axes: " << cia402_mode_to_string(mode)
            << std::endl;
  if (axis1_) axis1_->SetModeOfOperation(mode);
  if (axis2_) axis2_->SetModeOfOperation(mode);
}

void DualAxisController::MoveBothAxes(int32_t pos1, int32_t pos2, bool relative) {
  std::cout << ">>> Commanding Synchronized Motion: Axis 1 -> " << pos1
            << ", Axis 2 -> " << pos2 << " (relative: " << (relative ? "true" : "false")
            << ")" << std::endl;
  if (axis1_) axis1_->SetTargetPosition(pos1, true, true, relative);
  if (axis2_) axis2_->SetTargetPosition(pos2, true, true, relative);
}

void DualAxisController::SetBothVelocities(int32_t vel1, int32_t vel2) {
  std::cout << ">>> Commanding Synchronized Velocity: Axis 1 -> " << vel1
            << ", Axis 2 -> " << vel2 << std::endl;
  if (axis1_) axis1_->SetTargetVelocity(vel1);
  if (axis2_) axis2_->SetTargetVelocity(vel2);
}

void DualAxisController::SetCmdVel(double linear_v, double angular_w) {
  WheelSpeeds speeds = kinematics_.ComputeWheelSpeeds(linear_v, angular_w);
  if (axis1_) axis1_->SetTargetVelocity(speeds.left_driver_vel);
  if (axis2_) axis2_->SetTargetVelocity(speeds.right_driver_vel);
}

void DualAxisController::UpdateOdometry(double dt_sec) {
  if (!axis1_ || !axis2_) return;
  int32_t left_ticks = axis1_->GetActualPosition();
  int32_t right_ticks = axis2_->GetActualPosition();
  kinematics_.UpdateOdometryFromTicks(left_ticks, right_ticks, dt_sec);
}

void DualAxisController::PrintTelemetry() const {
  std::cout << "\n+---------+--------+------------------+-----------------------------+--------------+--------------+---------+"
            << "\n| Axis    | NodeID | NMT State        | CiA 402 State               | Act Position | Act Velocity | TrgtRch |"
            << "\n+---------+--------+------------------+-----------------------------+--------------+--------------+---------+"
            << std::endl;

  auto print_row = [](const MbdvAxisDriver& ax) {
    std::cout << "| " << std::left << std::setw(7) << ax.GetAxisName()
              << " | " << std::setw(6) << static_cast<int>(ax.GetNodeId())
              << " | " << std::setw(16)
              << (ax.IsOperational() ? "OPERATIONAL (01)" : "PRE-OP (7F)")
              << " | " << std::setw(27) << cia402_state_to_string(ax.GetCiA402State())
              << " | " << std::right << std::setw(12) << ax.GetActualPosition()
              << " | " << std::setw(12) << ax.GetActualVelocity()
              << " | " << std::setw(7) << (ax.IsTargetReached() ? "YES" : "NO")
              << " |" << std::endl;
  };

  if (axis1_) print_row(*axis1_);
  if (axis2_) print_row(*axis2_);
  std::cout << "+---------+--------+------------------+-----------------------------+--------------+--------------+---------+"
            << std::endl;

  RobotPose pose = kinematics_.GetPose();
  RobotTwist twist = kinematics_.GetTwist();
  std::cout << ">>> ROBOT ODOMETRY POSE:  x = " << std::fixed << std::setprecision(4)
            << pose.x << " m, y = " << pose.y << " m, theta = " << pose.theta
            << " rad (" << (pose.theta * 180.0 / M_PI) << " deg)"
            << "\n>>> ROBOT VELOCITY:       v = " << twist.linear_v << " m/s, w = "
            << twist.angular_w << " rad/s\n" << std::endl;
}

}  // namespace mbdv
