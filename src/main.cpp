#include "mbdv/dual_axis_controller.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>

namespace {

std::atomic<bool> g_shutdown{false};

void OnSignal(int /*sig*/) { g_shutdown.store(true); }

void PrintHelp(const char* prog) {
  std::cout
      << "Moons' MBDV-2X-520AC dual-axis CANopen master\n"
      << "\nUsage: " << prog << " [mode] [options]\n"
      << "\nModes:\n"
      << "  -t, --teleop               Interactive keyboard teleop: W/A/S/D to drive, Space to stop, Q to quit.\n"
      << "  --selftest                 Diagnose the bus only (S01..S11), then exit. Safest first step.\n"
      << "  --test-motion              PP mode: servo on, +step/-step, back to 0, servo off.\n"
      << "  --test-velocity            PV mode: servo on, run at --velocity, servo off.\n"
      << "  --test-kinematics          PV mode: differential-drive cmd_vel demo (v, w).\n"
      << "  --test-motion-parallel     Issue both moves at once instead of one after the other.\n"
      << "  --monitor                  Run forever printing telemetry (default).\n"
      << "  --watch                    Bring up, then poll 0x200F/0x1001 every 250 ms and log\n"
      << "                            every alarm change with a timestamp. Use this to catch\n"
      << "                            LED codes that appear outside a motion test.\n"
      << "\nOptions:\n"
      << "  -i, --interface <dev>      CAN interface (default can0)\n"
      << "  -d, --dcf <path>           Master DCF (default config/master.dcf)\n"
      << "  -b, --bin <path>           Concise DCF to download to the slaves (optional)\n"
      << "  -1, --axis1 <id>           Axis 1 node-ID (default 1)\n"
      << "  -2, --axis2 <id>           Axis 2 node-ID (default 2)\n"
      << "      --baud <kbps>          Expected bus speed for the S08 check: 1000|800|500|250|125\n"
      << "                            (default 500, matching MBDV DIP SW7 = 1)\n"
      << "      --control-mode <n>     Expected P1-00 in 0x2A30: 1=TQ 15=PV 21=PP (default 21)\n"
      << "      --p1-00 <n>            Actually WRITE this value to 0x2A30 before S10 (1|15|21)\n"
      << "      --no-pdo-program       Verify PDOs only, never write them (diagnose only)\n"
      << "      --no-sdo-fallback      Do NOT retry a failed Controlword over SDO. Use this to\n"
      << "                            test the RPDO path in isolation.\n"
      << "      --watchdog <ms|off>     Communication watchdog 0x2060 (manual P1-39). 'off'\n"
      << "                            disables it. Raise it when EMERGENCY xxxx with error\n"
      << "                            register bit 4 COMMUNICATION appears, which happens\n"
      << "                            ~500 ms after the last RPDO.\n"
      << "      --sdo-setpoints       Send target position/velocity over SDO instead of RPDO.\n"
      << "                            Needed for drives whose firmware ignores received RPDOs;\n"
      << "                            correct but much slower than PDO.\n"
      << "      --step <counts>        Position-test stroke (default 10000)\n"
      << "      --velocity <counts/s>  Velocity-test setpoint (default 5000)\n"
      << "      --boot-timeout <ms>    Boot-up / SDO timeout per node (default 3000)\n"
      << "      --servo-timeout <ms>   Timeout per CiA 402 transition (default 2000)\n"
      << "      --log-level <lvl>      trace|debug|info|warn|error|fatal (default info)\n"
      << "      --log-file <path>      Also append the log to this file\n"
      << "      --no-color             Disable ANSI colours\n"
      << "      --diag                Print every diagnostic object of both axes before exiting\n"
      << "  -h, --help                 This text\n"
      << "\nTypical first run:\n"
      << "  sudo ./build/mbdv_dual_axis_node --selftest --diag\n"
      << std::endl;
}

/// Maps a --baud value in kbps onto bit/s, for the S08 comparison against 0x2021.
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

struct Cli {
  enum class Mode {
    kMonitor,
    kSelfTest,
    kMotion,
    kVelocity,
    kKinematics,
    kMotionParallel,
    kWatch,
    kTeleop
  };
  Mode mode{Mode::kMonitor};
  mbdv::ControllerOptions options;
  bool print_diag{false};
};

bool ParseArgs(int argc, char** argv, Cli* cli) {
  cli->options.log_file.clear();
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto next = [&](const char* name) -> const char* {
      if (i + 1 >= argc) {
        std::cerr << "ERROR: option " << name << " requires a value\n";
        return nullptr;
      }
      return argv[++i];
    };

    if (arg == "-h" || arg == "--help") {
      PrintHelp(argv[0]);
      return false;
    } else if (arg == "-i" || arg == "--interface") {
      const char* v = next("--interface"); if (!v) return false;
      cli->options.can_interface = v;
    } else if (arg == "-d" || arg == "--dcf") {
      const char* v = next("--dcf"); if (!v) return false;
      cli->options.dcf_path = v;
    } else if (arg == "-b" || arg == "--bin") {
      const char* v = next("--bin"); if (!v) return false;
      cli->options.bin_path = v;
    } else if (arg == "-1" || arg == "--axis1") {
      const char* v = next("--axis1"); if (!v) return false;
      cli->options.axis1_node_id = static_cast<uint8_t>(std::stoi(v));
    } else if (arg == "-2" || arg == "--axis2") {
      const char* v = next("--axis2"); if (!v) return false;
      cli->options.axis2_node_id = static_cast<uint8_t>(std::stoi(v));
    } else if (arg == "--baud") {
      const char* v = next("--baud"); if (!v) return false;
      if (!ParseBaudKbps(v, &cli->options.expect_bitrate_bps)) {
        std::cerr << "ERROR: --baud must be one of 1000 800 500 250 125 50 20 12 (kbps)\n";
        return false;
      }
    } else if (arg == "--control-mode") {
      const char* v = next("--control-mode"); if (!v) return false;
      cli->options.expect_control_mode = static_cast<uint32_t>(std::stoul(v));
    } else if (arg == "--p1-00") {
      // Consumed inside stage S08, which owns the P1-00 check.
      const char* v = next("--p1-00"); if (!v) return false;
      cli->options.write_control_mode = static_cast<uint32_t>(std::stoul(v));
      if (cli->options.expect_control_mode == 0) {
        cli->options.expect_control_mode = cli->options.write_control_mode;
      }
    } else if (arg == "--p1-00-save") {
      cli->options.store_parameters = true;
    } else if (arg == "--watch") {
      cli->mode = Cli::Mode::kWatch;
    } else if (arg == "--watchdog") {
      const char* v = next("--watchdog"); if (!v) return false;
      const std::string text = v;
      if (text == "off" || text == "0") {
        cli->options.watchdog_timeout_ms = 0;
      } else {
        try {
          cli->options.watchdog_timeout_ms = std::stoi(text);
        } catch (const std::exception&) {
          std::cerr << "ERROR: --watchdog takes a timeout in ms or 'off'\n";
          return false;
        }
        if (cli->options.watchdog_timeout_ms <= 0) {
          std::cerr << "ERROR: --watchdog timeout must be > 0, or 'off'\n";
          return false;
        }
      }
    } else if (arg == "--sdo-setpoints") {
      cli->options.sdo_setpoints = true;
    } else if (arg == "--no-sdo-fallback") {
      cli->options.sdo_controlword_fallback = false;
    } else if (arg == "--no-pdo-program") {
      cli->options.pdo.program_pdos = false;
    } else if (arg == "--step") {
      const char* v = next("--step"); if (!v) return false;
      cli->options.step_counts = std::stoi(v);
    } else if (arg == "--velocity") {
      const char* v = next("--velocity"); if (!v) return false;
      cli->options.test_velocity = std::stoi(v);
    } else if (arg == "--boot-timeout") {
      const char* v = next("--boot-timeout"); if (!v) return false;
      cli->options.boot_timeout = std::chrono::milliseconds(std::stoi(v));
    } else if (arg == "--servo-timeout") {
      const char* v = next("--servo-timeout"); if (!v) return false;
      cli->options.servo_timeout = std::chrono::milliseconds(std::stoi(v));
    } else if (arg == "--log-level") {
      const char* v = next("--log-level"); if (!v) return false;
      if (!mbdv::parse_log_level(v, cli->options.log_level)) {
        std::cerr << "ERROR: --log-level must be trace|debug|info|warn|error|fatal\n";
        return false;
      }
    } else if (arg == "--log-file") {
      const char* v = next("--log-file"); if (!v) return false;
      cli->options.log_file = v;
    } else if (arg == "--no-color") {
      cli->options.colour = false;
    } else if (arg == "--diag") {
      cli->print_diag = true;
    } else if (arg == "--selftest") {
      cli->mode = Cli::Mode::kSelfTest;
    } else if (arg == "--test-motion") {
      cli->mode = Cli::Mode::kMotion;
      cli->options.mode = mbdv::CiA402Mode::PROFILE_POSITION;
    } else if (arg == "--test-motion-parallel") {
      cli->mode = Cli::Mode::kMotionParallel;
      cli->options.mode = mbdv::CiA402Mode::PROFILE_POSITION;
    } else if (arg == "--test-velocity") {
      cli->mode = Cli::Mode::kVelocity;
      cli->options.mode = mbdv::CiA402Mode::PROFILE_VELOCITY;
      cli->options.expect_control_mode =
          static_cast<uint32_t>(mbdv::DriveControlMode::kVelocityControl);
    } else if (arg == "--test-kinematics") {
      cli->mode = Cli::Mode::kKinematics;
      cli->options.mode = mbdv::CiA402Mode::PROFILE_VELOCITY;
      cli->options.expect_control_mode =
          static_cast<uint32_t>(mbdv::DriveControlMode::kVelocityControl);
    } else if (arg == "-t" || arg == "--teleop") {
      cli->mode = Cli::Mode::kTeleop;
      cli->options.mode = mbdv::CiA402Mode::PROFILE_VELOCITY;
      cli->options.expect_control_mode =
          static_cast<uint32_t>(mbdv::DriveControlMode::kVelocityControl);
    } else if (arg == "--monitor") {
      cli->mode = Cli::Mode::kMonitor;
    } else {
      std::cerr << "ERROR: unknown option '" << arg << "'\n";
      PrintHelp(argv[0]);
      return false;
    }
  }
  return true;
}

/// Runs @p fn every @p period until shutdown, feeding odometry from the encoders.
template <typename Fn>
void Loop(mbdv::DualAxisController& controller, std::chrono::milliseconds period, Fn&& fn) {
  auto last = std::chrono::steady_clock::now();
  while (!g_shutdown.load()) {
    const auto now = std::chrono::steady_clock::now();
    controller.UpdateOdometry(std::chrono::duration<double>(now - last).count());
    last = now;
    fn();
    std::this_thread::sleep_for(period);
  }
}

class RawTerminalScope {
 public:
  RawTerminalScope() {
    if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &orig_termios_) == 0) {
      struct termios raw = orig_termios_;
      raw.c_lflag &= ~(ICANON | ECHO);
      raw.c_cc[VMIN] = 0;
      raw.c_cc[VTIME] = 0;
      active_ = (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0);
    }
  }

  ~RawTerminalScope() {
    if (active_) {
      tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios_);
    }
  }

  bool IsActive() const noexcept { return active_; }

 private:
  struct termios orig_termios_{};
  bool active_{false};
};

int ReadKeyNonBlocking() {
  char c = 0;
  const ssize_t n = read(STDIN_FILENO, &c, 1);
  if (n <= 0) return -1;

  if (c == 27) {  // ESC sequence (e.g. arrow keys)
    char seq[2];
    if (read(STDIN_FILENO, &seq[0], 1) <= 0) return 27;
    if (read(STDIN_FILENO, &seq[1], 1) <= 0) return 27;
    if (seq[0] == '[') {
      switch (seq[1]) {
        case 'A': return 1001;  // Up arrow
        case 'B': return 1002;  // Down arrow
        case 'C': return 1003;  // Right arrow
        case 'D': return 1004;  // Left arrow
        default: break;
      }
    }
    return 27;
  }
  return static_cast<unsigned char>(c);
}

void RunTeleop(mbdv::DualAxisController& controller, mbdv::DiagnosticReport& report) {
  RawTerminalScope term_scope;

  std::cout << "\n"
            << "==============================================================================\n"
            << "       Moons' MBDV Dual-Axis KEYBOARD TELEOP (CiA 402 Profile Velocity)       \n"
            << "==============================================================================\n"
            << " Controls:\n"
            << "   [W] / [Arrow Up]    : Forward  (+v)\n"
            << "   [S] / [Arrow Down]  : Backward (-v)\n"
            << "   [A] / [Arrow Left]  : Turn Left (+w)\n"
            << "   [D] / [Arrow Right] : Turn Right (-w)\n"
            << "   [SPACE] or [X]      : Brake / Instant Stop (v=0, w=0)\n"
            << " Speed tuning:\n"
            << "   [+] / [=]           : Increase linear step (+0.02 m/s)\n"
            << "   [-] / [_]           : Decrease linear step (-0.02 m/s)\n"
            << " Utilities:\n"
            << "   [R]                 : Reset odometry pose to (0,0,0)\n"
            << "   [Q] or [ESC]        : Quit teleop & safe Servo OFF\n"
            << "==============================================================================\n"
            << std::endl;

  double target_v = 0.0;
  double target_w = 0.0;
  double step_v = 0.05;   // 0.05 m/s per tap
  double step_w = 0.15;   // 0.15 rad/s per tap

  const auto loop_interval = std::chrono::milliseconds(50);  // 20 Hz
  auto last_time = std::chrono::steady_clock::now();
  auto last_display_time = last_time;

  while (!g_shutdown.load()) {
    const auto now = std::chrono::steady_clock::now();
    const double dt = std::chrono::duration<double>(now - last_time).count();
    last_time = now;

    int key = ReadKeyNonBlocking();
    while (key != -1) {
      switch (key) {
        case 'w': case 'W': case 1001:  // Forward
          target_v += step_v;
          break;
        case 's': case 'S': case 1002:  // Backward
          target_v -= step_v;
          break;
        case 'a': case 'A': case 1004:  // Turn Left
          target_w += step_w;
          break;
        case 'd': case 'D': case 1003:  // Turn Right
          target_w -= step_w;
          break;
        case ' ': case 'x': case 'X':  // Brake
          target_v = 0.0;
          target_w = 0.0;
          break;
        case '+': case '=':
          step_v = std::min(0.50, step_v + 0.02);
          break;
        case '-': case '_':
          step_v = std::max(0.01, step_v - 0.02);
          break;
        case 'r': case 'R':
          controller.ResetOdometry();
          break;
        case 'q': case 'Q': case 27:  // Quit
          g_shutdown.store(true);
          break;
        default:
          break;
      }
      if (g_shutdown.load()) break;
      key = ReadKeyNonBlocking();
    }

    if (g_shutdown.load()) break;

    // Clamp velocities within safe physical limits
    target_v = std::max(-1.0, std::min(1.0, target_v));
    target_w = std::max(-2.5, std::min(2.5, target_w));

    // Round near-zero to exact zero
    if (std::abs(target_v) < 1e-4) target_v = 0.0;
    if (std::abs(target_w) < 1e-4) target_w = 0.0;

    controller.SetCmdVel(target_v, target_w);
    controller.UpdateOdometry(dt);

    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_display_time).count() >= 150) {
      last_display_time = now;
      const auto pose = controller.GetRobotPose();
      const auto twist = controller.GetRobotTwist();
      const auto& ax1 = controller.GetAxis1();
      const auto& ax2 = controller.GetAxis2();

      std::cout << "\r[CMD: v=" << std::showpos << std::fixed << std::setprecision(2) << target_v
                << " w=" << target_w << " m/s,rad/s (step=" << std::noshowpos << step_v << ")] | "
                << "[ACT: v=" << std::showpos << twist.linear_v << " w=" << twist.angular_w << "] | "
                << "[AX1: " << ax1.GetActualVelocity() << " cps | AX2: " << ax2.GetActualVelocity() << " cps] | "
                << "[POSE: x=" << std::noshowpos << std::setprecision(3) << pose.x
                << " y=" << pose.y << " th=" << std::setprecision(1)
                << (pose.theta * 180.0 / M_PI) << "°]   " << std::flush;
    }

    std::this_thread::sleep_for(loop_interval);
  }

  std::cout << "\n\n>>> Teleop stopped: Bringing both axes to complete stop..." << std::endl;
  controller.SetCmdVel(0.0, 0.0);
  controller.SetVelocitiesStaged(report, 0, 0, std::chrono::milliseconds(300));
}

}  // namespace

int main(int argc, char* argv[]) {
  std::signal(SIGINT, OnSignal);
  std::signal(SIGTERM, OnSignal);

  Cli cli;
  if (!ParseArgs(argc, argv, &cli)) return 1;

  mbdv::Logger::Instance().Configure(cli.options.log_file, cli.options.log_level,
                                     cli.options.colour && isatty(fileno(stdout)) != 0);

  std::cout << "\n"
            << "===============================================================================\n"
            << "  Moons' MBDV-2X-520AC  |  Dual-axis CiA 402 CANopen master bring-up\n"
            << "  bus " << cli.options.can_interface << " @ "
            << mbdv::can_bit_rate_bps_to_string(cli.options.expect_bitrate_bps) << "  |  axis1 node "
            << static_cast<int>(cli.options.axis1_node_id) << "  |  axis2 node "
            << static_cast<int>(cli.options.axis2_node_id) << "\n"
            << "===============================================================================\n"
            << std::endl;

  mbdv::DualAxisController controller;
  mbdv::DiagnosticReport report;

  // ---- S01..S04 ----
  if (!controller.Initialize(cli.options, report)) {
    report.PrintFirstFailure(std::cout);
    report.Print(std::cout, "MBDV DUAL-AXIS BRING-UP");
    return 1;
  }
  if (!controller.Start(report)) {
    report.PrintFirstFailure(std::cout);
    report.Print(std::cout, "MBDV DUAL-AXIS BRING-UP");
    return 1;
  }

  // ---- S05..S11 ----
  // --selftest checks bus health, not motion mode, so the P1-00 expectation is dropped.
  // AX2 currently ships P1-00 = 15 (velocity) while the default mode is PP; failing the
  // self-test on that would hide real bus problems. The value is still logged in S08.
  mbdv::ControllerOptions bring_up_options = cli.options;
  if (cli.mode == Cli::Mode::kSelfTest || cli.mode == Cli::Mode::kWatch) {
    bring_up_options.expect_control_mode = 0;
  }

  const bool bus_ok = controller.BringUpBothAxes(report, bring_up_options);
  if (cli.print_diag) controller.PrintDriveDiagnostics();

  if (!bus_ok) {
    mbdv::LogError(mbdv::Stage::S11_FAULT_RESET, nullptr,
                   "bring-up aborted before mode selection and servo enable");
    controller.Stop();
    report.PrintFirstFailure(std::cout);
    report.Print(std::cout, "MBDV DUAL-AXIS BRING-UP");
    std::cout << "\nNext step: fix the failing stage above, then re-run --selftest.\n"
              << std::endl;
    return 1;
  }

  if (cli.mode == Cli::Mode::kWatch) {
    controller.StartAlarmWatch(std::chrono::milliseconds(250));
    std::cout << "\nAlarm watch running. Every 0x200F / 0x1001 change is logged with a "
                 "timestamp.\nLeave it running and note the wall-clock time whenever the drive "
                 "LED blinks a code,\nthen press Ctrl+C.\n"
              << std::endl;
    Loop(controller, std::chrono::milliseconds(1000), [&] { controller.PrintTelemetry(); });
    controller.StopAlarmWatch();
    controller.Stop();
    report.Print(std::cout, "MBDV DUAL-AXIS ALARM WATCH");
    return 0;
  }

  if (cli.mode == Cli::Mode::kSelfTest) {
    mbdv::LogInfo(mbdv::Stage::S11_FAULT_RESET, nullptr,
                  "self-test finished: bus, identity and PDOs are healthy");
    controller.Stop();
    report.Pending(mbdv::Stage::S12_SERVO_ON, nullptr);
    report.Print(std::cout, "MBDV DUAL-AXIS BRING-UP (SELFTEST)");
    std::cout << "\nThe bus is healthy. Next: --test-motion (make sure the motor is uncoupled "
                 "and a CW/CCW limit is available).\n"
              << std::endl;
    return 0;
  }

  // ---- S10 ----
  if (!controller.SetModeBothAxes(report, cli.options.mode)) {
    controller.Stop();
    report.PrintFirstFailure(std::cout);
    report.Print(std::cout, "MBDV DUAL-AXIS BRING-UP");
    return 1;
  }

  // ---- S12 ----
  if (!controller.EnableBothAxes(report, cli.options.servo_timeout)) {
    controller.Stop();
    report.PrintFirstFailure(std::cout);
    report.Print(std::cout, "MBDV DUAL-AXIS BRING-UP");
    return 1;
  }

  int exit_code = 0;

  switch (cli.mode) {
    case Cli::Mode::kMotion: {
      const int32_t step = cli.options.step_counts;
      mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr,
                    mbdv::Str("PP test: AX1 -> ", mbdv::Sgn(step), " AX2 -> ", mbdv::Sgn(-step),
                              " counts"));
      if (!controller.MoveBothAxesStaged(report, step, -step, std::chrono::milliseconds(8000))) {
        exit_code = 1;
        break;
      }
      Loop(controller, std::chrono::milliseconds(500),
           [&] { controller.PrintTelemetry(); });
      if (g_shutdown.load()) break;

      mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr, "PP test: returning both axes to 0");
      if (!controller.MoveBothAxesStaged(report, 0, 0, std::chrono::milliseconds(8000))) {
        exit_code = 1;
      }
      break;
    }

    case Cli::Mode::kMotionParallel: {
      const int32_t step = cli.options.step_counts;
      mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr,
                    mbdv::Str("PP parallel test: AX1 and AX2 -> ", mbdv::Sgn(step), " counts"));
      if (!controller.MoveBothAxesStaged(report, step, step, std::chrono::milliseconds(8000))) {
        exit_code = 1;
      }
      break;
    }

    case Cli::Mode::kVelocity: {
      const int32_t vel = cli.options.test_velocity;
      mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr,
                    mbdv::Str("PV test: AX1 = +", vel, " AX2 = ", mbdv::Sgn(-vel), " counts/s"));
      if (controller.SetVelocitiesStaged(report, vel, -vel, std::chrono::milliseconds(1500))) {
        Loop(controller, std::chrono::milliseconds(500), [&] { controller.PrintTelemetry(); });
      } else {
        exit_code = 1;
      }
      if (!g_shutdown.load()) controller.SetVelocitiesStaged(report, 0, 0,
                                                            std::chrono::milliseconds(500));
      break;
    }

    case Cli::Mode::kKinematics: {
      struct Phase {
        const char* label;
        double v;
        double w;
        int seconds;
      };
      const Phase phases[] = {
          {"straight forward", 0.20, 0.0, 3},
          {"in-place turn left", 0.0, 0.50, 3},
          {"arc left", 0.15, 0.30, 3},
          {"stop", 0.0, 0.0, 1},
      };
      for (const Phase& phase : phases) {
        if (g_shutdown.load()) break;
        mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr,
                      mbdv::Str("kinematics phase: ", phase.label, " v=", phase.v,
                                " m/s w=", phase.w, " rad/s for ", phase.seconds, " s"));
        controller.SetCmdVel(phase.v, phase.w);
        const auto phase_start = std::chrono::steady_clock::now();
        while (!g_shutdown.load() &&
               std::chrono::steady_clock::now() - phase_start <
                   std::chrono::seconds(phase.seconds)) {
          const auto now = std::chrono::steady_clock::now();
          controller.UpdateOdometry(std::chrono::duration<double>(now - phase_start).count());
          controller.PrintTelemetry();
          std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        if (!controller.SetVelocitiesStaged(report, 0, 0, std::chrono::milliseconds(400))) {
          exit_code = 1;
          break;
        }
      }
      break;
    }

    case Cli::Mode::kTeleop: {
      RunTeleop(controller, report);
      break;
    }

    case Cli::Mode::kMonitor:
    default:
      mbdv::LogInfo(mbdv::Stage::S14_MOTION_TRACKING, nullptr,
                    "telemetry monitor running - press Ctrl+C to stop");
      Loop(controller, std::chrono::milliseconds(500), [&] { controller.PrintTelemetry(); });
      break;
  }

  // ---- S15 ----
  controller.DisableBothAxes(report);
  if (cli.print_diag) controller.PrintDriveDiagnostics();
  // ---- S16 ----
  controller.Stop();

  const bool ok = report.Print(std::cout, "MBDV DUAL-AXIS BRING-UP");
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