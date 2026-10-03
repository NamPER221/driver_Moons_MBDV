#include "mbdv/single_axis_controller.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

namespace {

std::atomic<bool> g_shutdown{false};

void OnSignal(int /*sig*/) { g_shutdown.store(true); }

void PrintHelp(const char* prog) {
  std::cout << "Moons' MBDV single-axis (Axis 1) CiA 402 CANopen master\n\n"
            << "Usage: " << prog << " [mode] [options]\n\n"
            << "Modes:\n"
            << "  --selftest          Diagnose the bus only (S01..S11), then exit\n"
            << "  --test-motion       PP mode: servo on, +step, back to 0, servo off\n"
            << "  --test-velocity     PV mode: servo on, run at --velocity, servo off\n"
            << "  --monitor           Telemetry monitor (default)\n\n"
            << "Options:\n"
            << "  -i, --interface <dev>   CAN interface (default can0)\n"
            << "  -d, --dcf <path>        DCF (default config/single_axis_500k/master.dcf)\n"
            << "  -b, --bin <path>        Concise DCF to download (optional)\n"
            << "  -1, --axis1 <id>        CANopen node-ID (default 1)\n"
            << "      --baud <kbps>       Expected bus speed for S08 (default 500)\n"
            << "      --control-mode <n>  Expected P1-00 in 0x2A30: 1|15|21 (default 21)\n"
            << "      --p1-00 <n>         WRITE this value to 0x2A30 before S10\n"
            << "      --no-pdo-program    Verify PDOs only, never write them\n"
            << "      --no-sdo-fallback   Do NOT retry a failed Controlword over SDO\n"
            << "      --step <counts>     Position-test stroke (default 10000)\n"
            << "      --velocity <c/s>    Velocity-test setpoint (default 5000)\n"
            << "      --boot-timeout <ms> Boot-up / SDO timeout (default 3000)\n"
            << "      --servo-timeout <ms> Timeout per CiA 402 transition (default 2000)\n"
            << "      --log-level <lvl>   trace|debug|info|warn|error|fatal (default info)\n"
            << "      --log-file <path>   Also append the log to this file\n"
            << "      --no-color          Disable ANSI colours\n"
            << "      --diag             Print every diagnostic object before exiting\n"
            << "  -h, --help              This text\n"
            << "\nTypical first run:\n"
            << "  sudo ./build/mbdv_single_axis_node --selftest --diag\n"
            << std::endl;
}

bool ParseBaudKbps(const std::string& text, uint32_t* bps) {
  static const struct {
    int kbps;
    uint32_t bps;
  } kTable[] = {{1000, 1000000u}, {800, 800000u}, {500, 500000u}, {250, 250000u},
                {125, 125000u},  {50, 50000u},    {20, 20000u},    {12, 12500u}};
  try {
    const int kbps = std::stoi(text);
    for (const auto& entry : kTable) {
      if (entry.kbps == kbps) {
        *bps = entry.bps;
        return true;
      }
    }
  } catch (const std::exception&) {
  }
  return false;
}

enum class Mode { kMonitor, kSelfTest, kMotion, kVelocity };

}  // namespace

int main(int argc, char* argv[]) {
  std::signal(SIGINT, OnSignal);
  std::signal(SIGTERM, OnSignal);

  Mode mode = Mode::kMonitor;
  mbdv::SingleAxisOptions options;
  bool print_diag = false;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto next = [&](const char* name) -> const char* {
      if (i + 1 >= argc) {
        std::cerr << "ERROR: option " << name << " requires a value\n";
        return nullptr;
      }
      return argv[++i];
    };

    if (arg == "-h" || arg == "--help") { PrintHelp(argv[0]); return 0; }
    else if (arg == "-i" || arg == "--interface") { const char* v = next("--interface"); if (!v) return 1; options.can_interface = v; }
    else if (arg == "-d" || arg == "--dcf") { const char* v = next("--dcf"); if (!v) return 1; options.dcf_path = v; }
    else if (arg == "-b" || arg == "--bin") { const char* v = next("--bin"); if (!v) return 1; options.bin_path = v; }
    else if (arg == "-1" || arg == "--axis1") { const char* v = next("--axis1"); if (!v) return 1; options.node_id = static_cast<uint8_t>(std::stoi(v)); }
    else if (arg == "--baud") { const char* v = next("--baud"); if (!v) return 1; if (!ParseBaudKbps(v, &options.expect_bitrate_bps)) { std::cerr << "ERROR: --baud must be one of 1000 800 500 250 125 50 20 12 (kbps)\n"; return 1; } }
    else if (arg == "--control-mode") { const char* v = next("--control-mode"); if (!v) return 1; options.expect_control_mode = static_cast<uint32_t>(std::stoul(v)); }
    else if (arg == "--p1-00") { const char* v = next("--p1-00"); if (!v) return 1; options.write_control_mode = static_cast<uint32_t>(std::stoul(v)); if (options.expect_control_mode == 0) options.expect_control_mode = options.write_control_mode; }
    else if (arg == "--no-pdo-program") { options.pdo.program_pdos = false; }
    else if (arg == "--no-sdo-fallback") { options.sdo_controlword_fallback = false; }
    else if (arg == "--sdo-setpoints") { options.sdo_setpoints = true; }
    else if (arg == "--watchdog") { const char* v = next("--watchdog"); if (!v) return 1; std::string t = v; if (t == "off" || t == "0") options.watchdog_timeout_ms = 0; else { options.watchdog_timeout_ms = std::stoi(t); if (options.watchdog_timeout_ms <= 0) { std::cerr << "ERROR: --watchdog takes ms or 'off'\n"; return 1; } } }
    else if (arg == "--step") { const char* v = next("--step"); if (!v) return 1; options.step_counts = std::stoi(v); }
    else if (arg == "--velocity") { const char* v = next("--velocity"); if (!v) return 1; options.test_velocity = std::stoi(v); }
    else if (arg == "--boot-timeout") { const char* v = next("--boot-timeout"); if (!v) return 1; options.boot_timeout = std::chrono::milliseconds(std::stoi(v)); }
    else if (arg == "--servo-timeout") { const char* v = next("--servo-timeout"); if (!v) return 1; options.servo_timeout = std::chrono::milliseconds(std::stoi(v)); }
    else if (arg == "--log-level") { const char* v = next("--log-level"); if (!v) return 1; if (!mbdv::parse_log_level(v, options.log_level)) { std::cerr << "ERROR: invalid --log-level\n"; return 1; } }
    else if (arg == "--log-file") { const char* v = next("--log-file"); if (!v) return 1; options.log_file = v; }
    else if (arg == "--no-color") { options.colour = false; }
    else if (arg == "--diag") { print_diag = true; }
    else if (arg == "--selftest") { mode = Mode::kSelfTest; }
    else if (arg == "--test-motion") { mode = Mode::kMotion; options.mode = mbdv::CiA402Mode::PROFILE_POSITION; }
    else if (arg == "--test-velocity") { mode = Mode::kVelocity; options.mode = mbdv::CiA402Mode::PROFILE_VELOCITY; options.expect_control_mode = static_cast<uint32_t>(mbdv::DriveControlMode::kVelocityControl); }
    else if (arg == "--monitor") { mode = Mode::kMonitor; }
    else { std::cerr << "ERROR: unknown option '" << arg << "'\n"; PrintHelp(argv[0]); return 1; }
  }

  mbdv::Logger::Instance().Configure(options.log_file, options.log_level,
                                     options.colour && isatty(fileno(stdout)) != 0);

  mbdv::DiagnosticReport report;

  std::cout << "\n"
            << "===============================================================================\n"
            << "  Moons' MBDV  |  Single-axis CiA 402 CANopen master bring-up\n"
            << "  bus " << options.can_interface << " @ "
            << mbdv::can_bit_rate_bps_to_string(options.expect_bitrate_bps) << "  |  axis1 node "
            << static_cast<int>(options.node_id) << "\n"
            << "===============================================================================\n"
            << std::endl;

  mbdv::SingleAxisController controller;

  if (!controller.Initialize(options, report) || !controller.Start(report)) {
    report.PrintFirstFailure(std::cout);
    report.Print(std::cout, "MBDV SINGLE-AXIS BRING-UP");
    return 1;
  }

  // --selftest checks bus health, not motion mode, so drop the P1-00 expectation.
  mbdv::SingleAxisOptions bring_up_options = options;
  if (mode == Mode::kSelfTest) bring_up_options.expect_control_mode = 0;

  const bool bus_ok = controller.BringUp(report, bring_up_options);
  if (print_diag) controller.PrintDriveDiagnostics();

  if (!bus_ok) {
    mbdv::LogError(mbdv::Stage::S11_FAULT_RESET, nullptr,
                   "bring-up aborted before mode selection and servo enable");
    controller.Stop();
    report.PrintFirstFailure(std::cout);
    report.Print(std::cout, "MBDV SINGLE-AXIS BRING-UP");
    std::cout << "\nNext step: fix the failing stage above, then re-run --selftest.\n" << std::endl;
    return 1;
  }

  if (mode == Mode::kSelfTest) {
    mbdv::LogInfo(mbdv::Stage::S11_FAULT_RESET, nullptr,
                  "self-test finished: bus, identity and PDOs are healthy");
    controller.Stop();
    report.Pending(mbdv::Stage::S12_SERVO_ON, "AX1");
    report.Print(std::cout, "MBDV SINGLE-AXIS BRING-UP (SELFTEST)");
    std::cout << "\nThe bus is healthy. Next: --test-motion with the motor uncoupled.\n"
              << std::endl;
    return 0;
  }

  if (!controller.SetMode(report, options.mode) ||
      !controller.EnableServo(report, options.servo_timeout)) {
    controller.Stop();
    report.PrintFirstFailure(std::cout);
    report.Print(std::cout, "MBDV SINGLE-AXIS BRING-UP");
    return 1;
  }

  int exit_code = 0;
  switch (mode) {
    case Mode::kMotion: {
      const int32_t step = options.step_counts;
      mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr,
                    mbdv::Str("PP test: target ", mbdv::Sgn(step), " counts"));
      if (!controller.MoveToPositionStaged(report, step, std::chrono::milliseconds(8000))) {
        exit_code = 1;
        break;
      }
      while (!g_shutdown.load()) { controller.PrintTelemetry(); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
      if (g_shutdown.load()) break;
      mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr, "PP test: returning to 0");
      if (!controller.MoveToPositionStaged(report, 0, std::chrono::milliseconds(8000))) exit_code = 1;
      break;
    }
    case Mode::kVelocity: {
      const int32_t vel = options.test_velocity;
      mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr,
                    mbdv::Str("PV test: ", vel, " counts/s"));
      if (controller.SetVelocityStaged(report, vel, std::chrono::milliseconds(1500))) {
        while (!g_shutdown.load()) { controller.PrintTelemetry(); std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
      } else {
        exit_code = 1;
      }
      if (!g_shutdown.load()) controller.SetVelocityStaged(report, 0, std::chrono::milliseconds(500));
      break;
    }
    case Mode::kMonitor:
    default:
      mbdv::LogInfo(mbdv::Stage::S14_MOTION_TRACKING, nullptr,
                    "telemetry monitor running - press Ctrl+C to stop");
      while (!g_shutdown.load()) {
        controller.PrintTelemetry();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }
      break;
  }

  controller.DisableServo(report);
  if (print_diag) controller.PrintDriveDiagnostics();
  controller.Stop();

  const bool ok = report.Print(std::cout, "MBDV SINGLE-AXIS BRING-UP");
  if (!ok) {
    report.PrintFirstFailure(std::cout);
    std::cout << "\nThe run failed at the stage named above. Re-run with --log-level debug and "
                 "--log-file <path> for a full trace.\n"
              << std::endl;
    return 1;
  }
  std::cout << "\nAll recorded stages passed.\n" << std::endl;
  return exit_code;
}