#include "mbdv/dual_axis_controller.hpp"

#include <csignal>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {
std::atomic<bool> g_shutdown_requested{false};

void signal_handler(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    g_shutdown_requested.store(true);
  }
}

void print_help(const char* prog_name) {
  std::cout << "Usage: " << prog_name << " [options]\n"
            << "Options:\n"
            << "  -i, --interface <dev>      CAN interface name (default: can0)\n"
            << "  -d, --dcf <path>           Master DCF file (default: config/master.dcf)\n"
            << "  -1, --axis1 <id>           Axis 1 CANopen Node ID (default: 1)\n"
            << "  -2, --axis2 <id>           Axis 2 CANopen Node ID (default: 2)\n"
            << "  -t, --test-motion          Run CiA 402 Servo ON & position motion test\n"
            << "  -k, --test-kinematics      Run differential drive kinematic velocity test (cmd_vel)\n"
            << "  -h, --help                 Display this help message\n"
            << std::endl;
}
}  // namespace

int main(int argc, char* argv[]) {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  std::string can_interface = "can0";
  std::string dcf_path = "config/master.dcf";
  uint8_t axis1_id = 1;
  uint8_t axis2_id = 2;
  bool test_motion = false;
  bool test_kinematics = false;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-i" || arg == "--interface") && i + 1 < argc) {
      can_interface = argv[++i];
    } else if ((arg == "-d" || arg == "--dcf") && i + 1 < argc) {
      dcf_path = argv[++i];
    } else if ((arg == "-1" || arg == "--axis1") && i + 1 < argc) {
      axis1_id = static_cast<uint8_t>(std::stoi(argv[++i]));
    } else if ((arg == "-2" || arg == "--axis2") && i + 1 < argc) {
      axis2_id = static_cast<uint8_t>(std::stoi(argv[++i]));
    } else if (arg == "-t" || arg == "--test-motion") {
      test_motion = true;
    } else if (arg == "-k" || arg == "--test-kinematics") {
      test_kinematics = true;
    } else if (arg == "-h" || arg == "--help") {
      print_help(argv[0]);
      return 0;
    }
  }

  mbdv::DualAxisController controller;
  if (!controller.Initialize(can_interface, dcf_path, "", axis1_id, axis2_id)) {
    std::cerr << "Initialization failed! Please check CAN interface and DCF path."
              << std::endl;
    return 1;
  }

  controller.Start();

  std::cout << "\nWaiting for remote nodes to boot and configure..." << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));

  if (test_motion) {
    std::cout << "\n--- Starting Automated Position Motion Test ---" << std::endl;

    controller.SetModeBothAxes(mbdv::CiA402Mode::PROFILE_POSITION);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    if (!controller.EnableBothAxes(std::chrono::milliseconds(3000))) {
      std::cerr << "Failed to enable both axes. Aborting test." << std::endl;
      controller.Stop();
      return 1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "\nCommanding Simultaneous Motion: Axis1 = +10000, Axis2 = -10000..."
              << std::endl;
    controller.MoveBothAxes(10000, -10000, false);

    auto start_time = std::chrono::steady_clock::now();
    auto last_time = start_time;
    while (!g_shutdown_requested.load() &&
           std::chrono::steady_clock::now() - start_time < std::chrono::seconds(5)) {
      auto now = std::chrono::steady_clock::now();
      double dt = std::chrono::duration<double>(now - last_time).count();
      last_time = now;
      controller.UpdateOdometry(dt);
      controller.PrintTelemetry();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\nReturning both axes to 0..." << std::endl;
    controller.MoveBothAxes(0, 0, false);
    start_time = std::chrono::steady_clock::now();
    last_time = start_time;
    while (!g_shutdown_requested.load() &&
           std::chrono::steady_clock::now() - start_time < std::chrono::seconds(4)) {
      auto now = std::chrono::steady_clock::now();
      double dt = std::chrono::duration<double>(now - last_time).count();
      last_time = now;
      controller.UpdateOdometry(dt);
      controller.PrintTelemetry();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    controller.DisableBothAxes();
    std::cout << "\n--- Motion Test Completed ---" << std::endl;

  } else if (test_kinematics) {
    std::cout << "\n--- Starting Kinematic Differential Drive Control Test (cmd_vel) ---"
              << std::endl;

    // 1. Set mode of operation to Profile Velocity (PV = 3)
    controller.SetModeBothAxes(mbdv::CiA402Mode::PROFILE_VELOCITY);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 2. Enable both drives
    if (!controller.EnableBothAxes(std::chrono::milliseconds(3000))) {
      std::cerr << "Failed to enable both axes for kinematics test. Aborting."
                << std::endl;
      controller.Stop();
      return 1;
    }

    // 3. Forward motion: v = 0.2 m/s, w = 0.0 rad/s for 3 seconds
    std::cout << "\n>>> Phase 1: Forward Motion (v = 0.2 m/s, w = 0.0 rad/s)..."
              << std::endl;
    controller.SetCmdVel(0.2, 0.0);

    auto start_time = std::chrono::steady_clock::now();
    auto last_time = start_time;
    while (!g_shutdown_requested.load() &&
           std::chrono::steady_clock::now() - start_time < std::chrono::seconds(3)) {
      auto now = std::chrono::steady_clock::now();
      double dt = std::chrono::duration<double>(now - last_time).count();
      last_time = now;
      controller.UpdateOdometry(dt);
      controller.PrintTelemetry();
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // 4. In-place rotation: v = 0.0 m/s, w = 0.5 rad/s for 3 seconds
    std::cout << "\n>>> Phase 2: In-place Rotation (v = 0.0 m/s, w = 0.5 rad/s)..."
              << std::endl;
    controller.SetCmdVel(0.0, 0.5);

    start_time = std::chrono::steady_clock::now();
    last_time = start_time;
    while (!g_shutdown_requested.load() &&
           std::chrono::steady_clock::now() - start_time < std::chrono::seconds(3)) {
      auto now = std::chrono::steady_clock::now();
      double dt = std::chrono::duration<double>(now - last_time).count();
      last_time = now;
      controller.UpdateOdometry(dt);
      controller.PrintTelemetry();
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // 5. Stop robot: v = 0, w = 0
    std::cout << "\n>>> Phase 3: Stopping Robot (v = 0.0, w = 0.0)..." << std::endl;
    controller.SetCmdVel(0.0, 0.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    controller.DisableBothAxes();
    std::cout << "\n--- Kinematic Control Test Completed ---" << std::endl;

  } else {
    std::cout << "\n--- Running in Continuous Telemetry Monitor Mode (Ctrl+C to stop) ---"
              << std::endl;
    auto last_time = std::chrono::steady_clock::now();
    while (!g_shutdown_requested.load()) {
      auto now = std::chrono::steady_clock::now();
      double dt = std::chrono::duration<double>(now - last_time).count();
      last_time = now;
      controller.UpdateOdometry(dt);
      controller.PrintTelemetry();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }

  controller.Stop();
  std::cout << "Exited cleanly." << std::endl;
  return 0;
}
