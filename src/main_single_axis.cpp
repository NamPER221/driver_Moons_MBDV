#include "mbdv/single_axis_controller.hpp"

#include <csignal>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace {
std::atomic<bool> g_shutdown_requested{false};

void signal_handler(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    g_shutdown_requested.store(true);
  }
}

void print_help(const char* prog_name) {
  std::cout << "Usage: " << prog_name << " [options]\n"
            << "Moons' MBDV Single Axis 1 Controller (Node ID: 1, Baudrate: 500k)\n\n"
            << "Options:\n"
            << "  -i, --interface <dev>      CAN interface name (default: can0)\n"
            << "  -d, --dcf <path>           Master DCF file (default: config/single_axis_500k/master.dcf)\n"
            << "  -1, --node-id <id>         Axis 1 Node ID (default: 1)\n"
            << "  -t, --test-motion          Run CiA 402 Servo ON & position motion test\n"
            << "  -v, --test-velocity <vel>  Run velocity motion test with specified counts/s (e.g. 5000)\n"
            << "  -h, --help                 Display this help message\n"
            << std::endl;
}
}  // namespace

int main(int argc, char* argv[]) {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  std::string can_interface = "can0";
  std::string dcf_path = "config/single_axis_500k/master.dcf";
  uint8_t node_id = 1;
  bool test_motion = false;
  bool test_velocity = false;
  int32_t target_vel = 5000;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-i" || arg == "--interface") && i + 1 < argc) {
      can_interface = argv[++i];
    } else if ((arg == "-d" || arg == "--dcf") && i + 1 < argc) {
      dcf_path = argv[++i];
    } else if ((arg == "-1" || arg == "--node-id") && i + 1 < argc) {
      node_id = static_cast<uint8_t>(std::stoi(argv[++i]));
    } else if (arg == "-t" || arg == "--test-motion") {
      test_motion = true;
    } else if ((arg == "-v" || arg == "--test-velocity") && i + 1 < argc) {
      test_velocity = true;
      target_vel = std::stoi(argv[++i]);
    } else if (arg == "-h" || arg == "--help") {
      print_help(argv[0]);
      return 0;
    }
  }

  mbdv::SingleAxisController controller;
  if (!controller.Initialize(can_interface, dcf_path, "", node_id)) {
    std::cerr << "Initialization failed! Check SocketCAN interface, baudrate (500k), and DCF."
              << std::endl;
    return 1;
  }

  controller.Start();

  std::cout << "\nWaiting for Axis 1 (Node " << static_cast<int>(node_id)
            << ") to boot and configure..." << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));

  if (test_motion) {
    std::cout << "\n--- Starting Axis 1 Position Motion Test (PP Mode) ---" << std::endl;

    // 1. Set mode to Profile Position (PP = 1)
    controller.SetMode(mbdv::CiA402Mode::PROFILE_POSITION);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 2. Enable Servo ON (0x0006 -> 0x0007 -> 0x000F)
    if (!controller.EnableServo(std::chrono::milliseconds(3000))) {
      std::cerr << "Failed to enable Servo on Axis 1. Aborting." << std::endl;
      controller.Stop();
      return 1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 3. Move to +10000 counts
    std::cout << "Commanding Axis 1 position target: +10000 counts..." << std::endl;
    controller.MoveToPosition(10000, false);

    auto start_time = std::chrono::steady_clock::now();
    while (!g_shutdown_requested.load() &&
           std::chrono::steady_clock::now() - start_time < std::chrono::seconds(4)) {
      controller.PrintTelemetry();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // 4. Return to 0 counts
    std::cout << "Returning Axis 1 to 0 counts..." << std::endl;
    controller.MoveToPosition(0, false);
    start_time = std::chrono::steady_clock::now();
    while (!g_shutdown_requested.load() &&
           std::chrono::steady_clock::now() - start_time < std::chrono::seconds(4)) {
      controller.PrintTelemetry();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // 5. Disable Servo
    controller.DisableServo();
    std::cout << "--- Position Motion Test Completed ---" << std::endl;

  } else if (test_velocity) {
    std::cout << "\n--- Starting Axis 1 Velocity Motion Test (PV Mode, Target: "
              << target_vel << " counts/s) ---" << std::endl;

    // 1. Set mode to Profile Velocity (PV = 3)
    controller.SetMode(mbdv::CiA402Mode::PROFILE_VELOCITY);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 2. Enable Servo ON
    if (!controller.EnableServo(std::chrono::milliseconds(3000))) {
      std::cerr << "Failed to enable Servo on Axis 1. Aborting." << std::endl;
      controller.Stop();
      return 1;
    }

    // 3. Set target velocity
    std::cout << "Setting target velocity = " << target_vel << " counts/s for 4 seconds..."
              << std::endl;
    controller.SetTargetVelocity(target_vel);

    auto start_time = std::chrono::steady_clock::now();
    while (!g_shutdown_requested.load() &&
           std::chrono::steady_clock::now() - start_time < std::chrono::seconds(4)) {
      controller.PrintTelemetry();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // 4. Stop velocity
    std::cout << "Stopping motor (velocity = 0)..." << std::endl;
    controller.SetTargetVelocity(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    controller.DisableServo();
    std::cout << "--- Velocity Motion Test Completed ---" << std::endl;

  } else {
    std::cout << "\n--- Running in Continuous Telemetry Monitor Mode (Ctrl+C to stop) ---"
              << std::endl;
    while (!g_shutdown_requested.load()) {
      controller.PrintTelemetry();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }

  controller.Stop();
  std::cout << "Exited cleanly." << std::endl;
  return 0;
}
