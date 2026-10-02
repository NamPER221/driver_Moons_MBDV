#include "mbdv/single_axis_controller.hpp"

#include <iomanip>
#include <iostream>

namespace mbdv {

SingleAxisController::SingleAxisController() = default;

SingleAxisController::~SingleAxisController() {
  Stop();
}

bool SingleAxisController::Initialize(const std::string& can_interface,
                                      const std::string& dcf_path,
                                      const std::string& bin_path,
                                      uint8_t node_id) {
  std::cout << "==================================================" << std::endl;
  std::cout << " Initializing Moons' MBDV Single-Axis Controller..." << std::endl;
  std::cout << " CAN Interface:   " << can_interface << std::endl;
  std::cout << " Master DCF:      " << dcf_path << std::endl;
  std::cout << " Axis 1 Node ID:  " << static_cast<int>(node_id) << std::endl;
  std::cout << " Baudrate:        500 kbps (500k)" << std::endl;
  std::cout << "==================================================" << std::endl;

  io_guard_ = std::make_unique<lely::io::IoGuard>();
  ctx_ = std::make_unique<lely::io::Context>();
  poll_ = std::make_unique<lely::io::Poll>(*ctx_);
  loop_ = std::make_unique<lely::ev::Loop>(poll_->get_poll());
  auto exec = loop_->get_executor();
  timer_ = std::make_unique<lely::io::Timer>(*poll_, exec, CLOCK_MONOTONIC);

  try {
    // Pass txlen = 1 to prevent lely from attempting to increase txqueuelen via Netlink
    // which requires root privileges (CAP_NET_ADMIN / EPERM).
    ctrl_ = std::make_unique<lely::io::CanController>(can_interface.c_str(), 1);
  } catch (const std::exception& ex) {
    try {
      ctrl_ = std::make_unique<lely::io::CanController>(can_interface.c_str());
    } catch (const std::exception& e2) {
      std::cerr << "ERROR: Failed to initialize CAN controller for '" << can_interface
                << "': " << e2.what()
                << "\n[HINT]: Run with 'sudo' or configure txqueuelen: 'sudo ip link set "
                << can_interface << " txqueuelen 1000'" << std::endl;
      return false;
    }
  }

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

  axis_ = std::make_unique<MbdvAxisDriver>(*master_, node_id, "Axis_1");

  std::cout << "Single Axis 1 Driver (Node " << static_cast<int>(node_id)
            << ") initialized successfully." << std::endl;
  return true;
}

void SingleAxisController::Start() {
  if (is_running_.load()) {
    return;
  }
  is_running_.store(true);
  std::cout << "Starting CANopen single-axis loop thread..." << std::endl;
  loop_thread_ = std::thread([this]() {
    try {
      loop_->run();
    } catch (const std::exception& ex) {
      std::cerr << "Exception in CANopen loop: " << ex.what() << std::endl;
    }
  });
}

void SingleAxisController::Stop() {
  if (!is_running_.load()) {
    return;
  }
  std::cout << "Stopping Single Axis Controller..." << std::endl;
  is_running_.store(false);

  DisableServo();

  if (loop_) {
    loop_->stop();
  }
  if (loop_thread_.joinable()) {
    loop_thread_.join();
  }

  std::cout << "Single Axis Controller stopped." << std::endl;
}

bool SingleAxisController::EnableServo(std::chrono::milliseconds timeout) {
  if (!axis_) return false;
  return axis_->EnableServo(timeout);
}

bool SingleAxisController::DisableServo() {
  if (!axis_) return false;
  return axis_->DisableServo();
}

void SingleAxisController::ResetFault() {
  if (axis_) axis_->ResetFault();
}

void SingleAxisController::QuickStop() {
  if (axis_) axis_->QuickStop();
}

void SingleAxisController::SetMode(CiA402Mode mode) {
  if (axis_) axis_->SetModeOfOperation(mode);
}

void SingleAxisController::MoveToPosition(int32_t target_position, bool relative) {
  if (axis_) axis_->SetTargetPosition(target_position, true, true, relative);
}

void SingleAxisController::SetTargetVelocity(int32_t target_velocity) {
  if (axis_) axis_->SetTargetVelocity(target_velocity);
}

void SingleAxisController::PrintTelemetry() const {
  if (!axis_) return;

  std::cout << "\n+---------+--------+------------------+-----------------------------+--------------+--------------+---------+"
            << "\n| Axis    | NodeID | NMT State        | CiA 402 State               | Act Position | Act Velocity | TrgtRch |"
            << "\n+---------+--------+------------------+-----------------------------+--------------+--------------+---------+"
            << std::endl;

  std::cout << "| " << std::left << std::setw(7) << axis_->GetAxisName()
            << " | " << std::setw(6) << static_cast<int>(axis_->GetNodeId())
            << " | " << std::setw(16)
            << (axis_->IsOperational() ? "OPERATIONAL (01)" : "PRE-OP (7F)")
            << " | " << std::setw(27) << cia402_state_to_string(axis_->GetCiA402State())
            << " | " << std::right << std::setw(12) << axis_->GetActualPosition()
            << " | " << std::setw(12) << axis_->GetActualVelocity()
            << " | " << std::setw(7) << (axis_->IsTargetReached() ? "YES" : "NO")
            << " |" << std::endl;

  std::cout << "+---------+--------+------------------+-----------------------------+--------------+--------------+---------+\n"
            << std::endl;
}

}  // namespace mbdv
