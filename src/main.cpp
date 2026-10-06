#include "mbdv/dual_axis_controller.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstring>
#include <functional>
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
      << "Moons' MBDV-2X-520AC dual-axis CiA 402 CANopen master\n"
      << "\nUsage: " << prog << " [mode] [options]\n"
      << "\nModes:\n"
      << "  -t, --teleop               Keyboard teleop through the differential-drive\n"
      << "                            kinematics (DEFAULT when no mode is given).\n"
      << "                            W/A/S/D or the arrow keys drive, Space brakes, Q quits.\n"
      << "  --selftest                 Diagnose the bus only (S01..S11), then exit. Safest first step.\n"
      << "  --test-motion              PP mode: servo on, +step/-step, back to 0, servo off.\n"
      << "  --test-velocity            PV mode: servo on, run at --velocity, servo off.\n"
      << "  --test-kinematics          PV mode: scripted differential-drive (v, w) sequence.\n"
      << "  --monitor                  Bring up, then print telemetry until Ctrl+C.\n"
      << "  --watch                    Bring up, then poll 0x200F/0x1001 every 250 ms and log\n"
      << "                            every alarm change with a timestamp. Use this to catch\n"
      << "                            LED codes that appear outside a motion test.\n"
      << "\nOptions:\n"
      << "  -i, --interface <dev>      CAN interface (default can0)\n"
      << "      --params <path>        Runtime parameters (default config/params.yaml)\n"
      << "      --dump-params          Print the effective parameters and exit\n"
      << "  -d, --dcf <path>           Master DCF (default from params.yaml)\n"
      << "  -b, --bin <path>           Concise DCF to download to the slaves (optional)\n"
      << "  -1, --axis1 <id>           Axis 1 node-ID (default 1)\n"
      << "  -2, --axis2 <id>           Axis 2 node-ID (default 2)\n"
      << "      --baud <kbps>          Expected bus speed for the S08 check: 1000|800|500|250|125\n"
      << "                            (default 500)\n"
      << "      --control-mode <n>     Expected P1-00 in 0x2A30: 1=TQ 15=PV 21=PP.\n"
      << "                            Defaults to the mode's own value: 15 for the velocity\n"
      << "                            modes (teleop, --test-velocity, --test-kinematics),\n"
      << "                            21 for the position ones. 0 skips the check.\n"
      << "      --p1-00 <n>            Actually WRITE this value to 0x2A30 before S10 (1|15|21)\n"
      << "      --no-pdo-program       Verify PDOs only, never write them (diagnose only)\n"
      << "      --watchdog <ms|off>    Communication watchdog 0x2060 (manual P1-39): enable it\n"
      << "                            with this timeout, or 'off'. Its action (P1-40) is\n"
      << "                            drive.watchdog_action in params.yaml.\n"
      << "      --step <counts>        Position-test stroke (default 10000)\n"
      << "      --velocity <counts/s>  Velocity-test setpoint (default 5000)\n"
      << "      --boot-timeout <ms>    Boot-up / SDO timeout per node (default 3000)\n"
      << "      --servo-timeout <ms>   Timeout per CiA 402 transition (default 2000)\n"
      << "      --log-level <lvl>      trace|debug|info|warn|error|fatal (default info)\n"
      << "      --log-file <path>      Also append the log to this file\n"
      << "      --no-color             Disable ANSI colours\n"
      << "      --diag                Print every diagnostic object of both axes before exiting\n"
      << "  -h, --help                 This text\n"
      << "\nTypical first run (both axes, keyboard driven):\n"
      << "  sudo ./build/mbdv_dual_axis_node --selftest --diag   # prove the bus first\n"
      << "  sudo ./build/mbdv_dual_axis_node                      # then teleop\n"
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
    kTeleop,      ///< default: keyboard driving of the two axes as a pair
    kMonitor,
    kSelfTest,
    kMotion,
    kVelocity,
    kKinematics,
    kWatch
  };
  Mode mode{Mode::kTeleop};
  /// Set once a mode flag is seen, so the implicit default can still pick its
  /// own motion mode and P1-00 expectation.
  bool mode_explicit{false};
  /// True when the operator pinned P1-00 with --p1-00/--control-mode, which then wins
  /// over the value derived from the selected CiA 402 mode.
  bool control_mode_explicit{false};
  mbdv::ControllerOptions options;
  bool print_diag{false};
  std::string params_path{"config/params.yaml"};
  bool dump_params{false};
  /// Set when the flag appears on the command line. Overrides are keyed on these rather
  /// than on "differs from the compiled-in default": that comparison let an absent flag's
  /// default (can0, config/master.dcf, 3000/2000 ms) silently replace params.yaml's value.
  bool interface_set{false};
  bool dcf_set{false};
  bool bin_set{false};
  bool baud_set{false};
  bool axis1_set{false};
  bool axis2_set{false};
  bool boot_timeout_set{false};
  bool servo_timeout_set{false};

  void SelectMode(Mode m, mbdv::CiA402Mode drive_mode) {
    mode = m;
    mode_explicit = true;
    options.mode = drive_mode;
    // Keep P1-00 (0x2A30) in step with the CiA 402 mode we are about to select.
    //
    // They have to agree: S08 verifies 0x2A30 against expect_control_mode and rewrites
    // it when it differs, while S10 writes 0x6060/0x6061 for the drive mode. Leaving
    // expect_control_mode at its YAML value for a position mode makes S08 write
    // 15 (velocity) into 0x2A30 and then S10 select Profile Position - the drive would
    // run in velocity while the master sends position setpoints. 21 is the factory
    // default, so --test-motion/PP now needs no 0x2A30 write at all.
    switch (drive_mode) {
      case mbdv::CiA402Mode::PROFILE_VELOCITY:
        options.expect_control_mode =
            static_cast<uint32_t>(mbdv::DriveControlMode::kVelocityControl);
        break;
      case mbdv::CiA402Mode::PROFILE_POSITION:
        options.expect_control_mode =
            static_cast<uint32_t>(mbdv::DriveControlMode::kPositionControl);
        break;
      default:
        break;
    }
  }
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
      cli->interface_set = true;
    } else if (arg == "--params") {
      const char* v = next("--params"); if (!v) return false;
      cli->params_path = v;
    } else if (arg == "--dump-params") {
      cli->dump_params = true;
    } else if (arg == "-d" || arg == "--dcf") {
      const char* v = next("--dcf"); if (!v) return false;
      cli->options.dcf_path = v;
      cli->dcf_set = true;
    } else if (arg == "-b" || arg == "--bin") {
      const char* v = next("--bin"); if (!v) return false;
      cli->options.bin_path = v;
      cli->bin_set = true;
    } else if (arg == "-1" || arg == "--axis1" || arg == "-2" || arg == "--axis2") {
      const bool first = arg == "-1" || arg == "--axis1";
      const char* v = next(first ? "--axis1" : "--axis2"); if (!v) return false;
      const int id = std::stoi(v);
      if (id < 1 || id > 127) {
        std::cerr << "ERROR: " << arg << " takes a CANopen node-ID in 1..127\n";
        return false;
      }
      (first ? cli->options.axis1_node_id : cli->options.axis2_node_id) = static_cast<uint8_t>(id);
      (first ? cli->axis1_set : cli->axis2_set) = true;
    } else if (arg == "--baud") {
      const char* v = next("--baud"); if (!v) return false;
      if (!ParseBaudKbps(v, &cli->options.expect_bitrate_bps)) {
        std::cerr << "ERROR: --baud must be one of 1000 800 500 250 125 50 20 12 (kbps)\n";
        return false;
      }
      cli->baud_set = true;
    } else if (arg == "--control-mode") {
      const char* v = next("--control-mode"); if (!v) return false;
      cli->options.expect_control_mode = static_cast<uint32_t>(std::stoul(v));
      cli->control_mode_explicit = true;
    } else if (arg == "--p1-00") {
      // Consumed inside stage S08, which owns the P1-00 check.
      const char* v = next("--p1-00"); if (!v) return false;
      cli->options.write_control_mode = static_cast<uint32_t>(std::stoul(v));
      // Unconditional: a mode flag parsed earlier has already filled
      // expect_control_mode with its own derived value, so only overwriting when the
      // field is still 0 would silently let the mode win over the operator.
      cli->options.expect_control_mode = cli->options.write_control_mode;
      cli->control_mode_explicit = true;
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
    } else if (arg == "--no-pdo-program") {
      cli->options.pdo.program_pdos = false;
    } else if (arg == "--step") {
      const char* v = next("--step"); if (!v) return false;
      cli->options.step_counts = std::stoi(v);
    } else if (arg == "--velocity") {
      const char* v = next("--velocity"); if (!v) return false;
      cli->options.test_velocity = std::stoi(v);
    } else if (arg == "--boot-timeout" || arg == "--servo-timeout") {
      const bool boot = arg == "--boot-timeout";
      const char* v = next(boot ? "--boot-timeout" : "--servo-timeout"); if (!v) return false;
      const int ms = std::stoi(v);
      if (ms <= 0) {
        std::cerr << "ERROR: " << arg << " must be > 0 ms\n";
        return false;
      }
      (boot ? cli->options.boot_timeout : cli->options.servo_timeout) = std::chrono::milliseconds(ms);
      (boot ? cli->boot_timeout_set : cli->servo_timeout_set) = true;
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
      cli->SelectMode(Cli::Mode::kSelfTest, mbdv::CiA402Mode::PROFILE_POSITION);
      // The self-test exercises the bring-up state machine, not P1-00. It deliberately
      // runs with the P1-00 check disabled (expect_control_mode == 0) so an unrelated
      // mode setting on the bench cannot mask a real bus fault; SelectMode() above just
      // derived 21 from Profile Position, so clear it again and mark the field as
      // explicitly chosen so the mode/P1-00 coupling does not put it back.
      cli->options.expect_control_mode = 0;
      cli->control_mode_explicit = true;
    } else if (arg == "--test-motion") {
      cli->SelectMode(Cli::Mode::kMotion, mbdv::CiA402Mode::PROFILE_POSITION);
    } else if (arg == "--test-velocity") {
      cli->SelectMode(Cli::Mode::kVelocity, mbdv::CiA402Mode::PROFILE_VELOCITY);
    } else if (arg == "--test-kinematics") {
      cli->SelectMode(Cli::Mode::kKinematics, mbdv::CiA402Mode::PROFILE_VELOCITY);
    } else if (arg == "-t" || arg == "--teleop") {
      cli->SelectMode(Cli::Mode::kTeleop, mbdv::CiA402Mode::PROFILE_VELOCITY);
    } else if (arg == "--watch") {
      cli->SelectMode(Cli::Mode::kWatch, mbdv::CiA402Mode::PROFILE_POSITION);
    } else if (arg == "--monitor") {
      cli->SelectMode(Cli::Mode::kMonitor, mbdv::CiA402Mode::PROFILE_POSITION);
    } else {
      std::cerr << "ERROR: unknown option '" << arg << "'\n";
      PrintHelp(argv[0]);
      return false;
    }
  }
  return true;
}

/**
 * @brief Rate-limits @p fn so a fast control loop does not flood a terminal.
 *
 * The control loop runs at 200 Hz while a human reads about 2 lines per second, so the
 * telemetry call has to be throttled independently of the loop rate.
 */
inline std::function<void()> Throttled(std::function<void()> fn,
                                       std::chrono::milliseconds period) {
  auto next = std::chrono::steady_clock::now();
  return [fn = std::move(fn), period, next]() mutable {
    const auto now = std::chrono::steady_clock::now();
    if (now < next) return;
    next = now + period;
    fn();
  };
}

/**
 * @brief The control loop: one iteration per control period until shutdown.
 *
 * Every iteration does the same three things in this order, which is what makes the
 * 200 Hz figure meaningful:
 *   1. Supervise() - liveness, reconnect and the slower Controlword refresh;
 *   2. UpdateAndPublishOdometry(dt) - integrate the encoders and emit one sample;
 *   3. fn() - the mode-specific work, e.g. reading keys and writing setpoints.
 *
 * The period is taken from the controller (rates.control_hz in params.yaml) and the
 * schedule is absolute rather than "sleep the period", so a slow iteration does not make
 * the loop drift.
 *
 * Runs until Ctrl+C, or for @p duration when it is non-zero.
 */
template <typename Fn>
void Loop(mbdv::DualAxisController& controller, Fn&& fn,
          std::chrono::milliseconds duration = std::chrono::milliseconds(0)) {
  const std::chrono::milliseconds period = controller.ControlPeriod();
  const auto start = std::chrono::steady_clock::now();
  auto last = start;
  auto next = last;

  while (!g_shutdown.load() &&
         (duration.count() == 0 || std::chrono::steady_clock::now() - start < duration)) {
    const auto now = std::chrono::steady_clock::now();
    const double dt = std::chrono::duration<double>(now - last).count();
    last = now;

    controller.Supervise();
    controller.UpdateAndPublishOdometry(dt);
    fn();

    next += period;
    const auto sleep_for = next - std::chrono::steady_clock::now();
    if (sleep_for > std::chrono::milliseconds(0)) std::this_thread::sleep_for(sleep_for);
    else next = std::chrono::steady_clock::now();  // fell behind; resynchronise
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

// Values above 0xFF that ReadKeyNonBlocking() can return for a multi-byte sequence.
enum : int {
  kKeyUp = 1001,
  kKeyDown = 1002,
  kKeyRight = 1003,
  kKeyLeft = 1004,
  kKeyEsc = 27,
};

/// Reads one byte, or one decoded escape sequence. Returns -1 when nothing is pending.
int ReadKeyNonBlocking() {
  char c = 0;
  if (read(STDIN_FILENO, &c, 1) <= 0) return -1;

  if (c == 27) {  // start of an escape sequence, or a bare ESC
    char seq[2];
    if (read(STDIN_FILENO, &seq[0], 1) <= 0) return kKeyEsc;
    if (read(STDIN_FILENO, &seq[1], 1) <= 0) return kKeyEsc;
    if (seq[0] != '[') return kKeyEsc;
    switch (seq[1]) {
      case 'A': return kKeyUp;
      case 'B': return kKeyDown;
      case 'C': return kKeyRight;
      case 'D': return kKeyLeft;
      default: return kKeyEsc;
    }
  }
  return static_cast<unsigned char>(c);
}

void RunTeleop(mbdv::DualAxisController& controller, mbdv::DiagnosticReport& report) {
  RawTerminalScope term_scope;
  if (!term_scope.IsActive()) {
    std::cout << "\n"
              << "WARNING: stdin is not a terminal, so no keystrokes can be read.\n"
              << "         Run from a real terminal (not through a pipe) to drive with the "
                 "keyboard.\n"
              << "         Falling through to the telemetry monitor; press Ctrl+C to exit.\n"
              << std::endl;
    Loop(controller, [&] { controller.PrintTelemetry(); });
    return;
  }

  std::cout << "\n"
            << "==============================================================================\n"
            << "      Moons' MBDV DUAL-AXIS KEYBOARD TELEOP  (differential drive)\n"
            << "      keys -> (v, w) in the robot body frame -> left/right wheel speeds\n"
            << "==============================================================================\n"
            << " Driving:\n"
            << "   [W] / [Arrow Up]    : forward   (+v)\n"
            << "   [S] / [Arrow Down]  : backward  (-v)\n"
            << "   [A] / [Arrow Left]  : turn left  (+w)\n"
            << "   [D] / [Arrow Right] : turn right (-w)\n"
            << "   [SPACE] or [X]      : brake      (v = 0, w = 0)\n"
            << " Speed step (how much one key press changes the setpoint):\n"
            << "   [+] / [=]           : linear step +0.02 m/s   (max 0.50)\n"
            << "   [-] / [_]           : linear step -0.02 m/s   (min 0.01)\n"
            << "   []] / [>]           : angular step +0.05 rad/s (max 0.60)\n"
            << "   [[] / [<]           : angular step -0.05 rad/s (min 0.05)\n"
            << " Utilities:\n"
            << "   [R]                 : reset the odometry pose to (0,0,0)\n"
            << "   [Q] or [ESC]        : quit and switch both servos off safely\n"
            << "==============================================================================\n"
            << std::endl;

  // v in m/s, w in rad/s. The controller converts them to per-wheel counts/s, so a
  // straight run gives equal wheel speeds and a turn gives opposite ones.
  double target_v = 0.0;
  double target_w = 0.0;
  double step_v = 0.05;  // m/s per press
  double step_w = 0.15;  // rad/s per press

  constexpr double kMaxLinearV = 1.0;    // m/s
  constexpr double kMaxAngularW = 2.5;   // rad/s
  constexpr double kMaxStepV = 0.50;
  constexpr double kMaxStepW = 0.60;
  constexpr double kMinStepV = 0.01;
  constexpr double kMinStepW = 0.05;

  const std::chrono::milliseconds loop_interval = controller.ControlPeriod();
  auto last_time = std::chrono::steady_clock::now();
  auto last_display_time = last_time;
  auto next_tick = last_time;

  while (!g_shutdown.load()) {
    const auto now = std::chrono::steady_clock::now();
    const double dt = std::chrono::duration<double>(now - last_time).count();
    last_time = now;

    // Drain every key pressed since the last pass so a burst is not lost.
    int key = ReadKeyNonBlocking();
    while (key != -1) {
      switch (key) {
        case 'w': case 'W': case kKeyUp:    target_v += step_v; break;
        case 's': case 'S': case kKeyDown:  target_v -= step_v; break;
        case 'a': case 'A': case kKeyLeft:  target_w += step_w; break;
        case 'd': case 'D': case kKeyRight: target_w -= step_w; break;
        case ' ': case 'x': case 'X':
          target_v = 0.0;
          target_w = 0.0;
          break;
        case '+': case '=':
          step_v = std::min(kMaxStepV, step_v + 0.02);
          break;
        case '-': case '_':
          step_v = std::max(kMinStepV, step_v - 0.02);
          break;
        case ']': case '>':
          step_w = std::min(kMaxStepW, step_w + 0.05);
          break;
        case '[': case '<':
          step_w = std::max(kMinStepW, step_w - 0.05);
          break;
        case 'r': case 'R':
          controller.ResetOdometry();
          break;
        case 'q': case 'Q': case kKeyEsc:
          g_shutdown.store(true);
          break;
        default:
          break;
      }
      if (g_shutdown.load()) break;
      key = ReadKeyNonBlocking();
    }

    if (g_shutdown.load()) break;

    target_v = std::max(-kMaxLinearV, std::min(kMaxLinearV, target_v));
    target_w = std::max(-kMaxAngularW, std::min(kMaxAngularW, target_w));
    if (std::abs(target_v) < 1e-4) target_v = 0.0;
    if (std::abs(target_w) < 1e-4) target_w = 0.0;

    controller.Supervise();
    controller.UpdateAndPublishOdometry(dt);
    // Supervise() has already turned the stop output on if either axis went away. The keys
    // are dropped too, so the operator has to command motion again once the axis is back: a
    // speed typed before the stop must never restart the robot on its own.
    if (controller.StopRequested()) {
      target_v = 0.0;
      target_w = 0.0;
    }
    // Not applied while an axis is down or the stop output is on; the zero request sent
    // meanwhile is one of the conditions for releasing the stop output.
    controller.SetCmdVel(target_v, target_w);

    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_display_time)
            .count() >= 150) {
      last_display_time = now;
      const auto pose = controller.GetRobotPose();
      const auto twist = controller.GetRobotTwist();
      const auto& ax1 = controller.GetAxis1();
      const auto& ax2 = controller.GetAxis2();

      std::cout << "\r[CMD v=" << std::showpos << std::fixed << std::setprecision(2) << target_v
                << " w=" << target_w << "] [ACT v=" << twist.linear_v << " w=" << twist.angular_w
                << "] [AX1 " << std::noshowpos << std::setw(7) << ax1.GetActualVelocity()
                << " | AX2 " << std::setw(7) << ax2.GetActualVelocity() << " cps]"
                << " [x=" << pose.x << " y=" << pose.y << " th="
                << std::setprecision(1) << (pose.theta * 180.0 / M_PI) << "deg]"
                << "  step " << std::setprecision(2) << step_v << "/" << step_w
                << (controller.StopRequested() ? "  [STOP OUTPUT ON]" : "") << "   "
                << std::flush;
    }

    // Absolute schedule, so a slow pass does not accumulate drift at 200 Hz.
    next_tick += loop_interval;
    const auto sleep_for = next_tick - std::chrono::steady_clock::now();
    if (sleep_for > std::chrono::milliseconds(0)) std::this_thread::sleep_for(sleep_for);
    else next_tick = std::chrono::steady_clock::now();
  }

  std::cout << "\n\n>>> Teleop stopped: braking both axes to zero and switching servo off."
            << std::endl;
  controller.SetCmdVel(0.0, 0.0);
  controller.SetVelocitiesStaged(report, 0, 0, std::chrono::milliseconds(300));
}

}  // namespace

int main(int argc, char* argv[]) {
  std::signal(SIGINT, OnSignal);
  std::signal(SIGTERM, OnSignal);

  Cli cli;
  try {
    if (!ParseArgs(argc, argv, &cli)) return 1;
  } catch (const std::exception& ex) {
    // std::stoi/std::stoul on a non-numeric or out-of-range value.
    std::cerr << "ERROR: invalid option value (" << ex.what() << "); see --help\n";
    return 1;
  }

  // No mode flag given: drive the two axes from the keyboard, which needs profile
  // velocity and therefore P1-00 = 15 on both drives.
  if (!cli.mode_explicit) {
    cli.SelectMode(Cli::Mode::kTeleop, mbdv::CiA402Mode::PROFILE_VELOCITY);
  }

  // ---- runtime parameters -------------------------------------------------
  // params.yaml is the source of truth for rates, heartbeat, reconnect, kinematics and
  // odometry output. Command-line flags are applied on top of it below, so a flag always
  // wins over the file and the file always wins over the compiled-in default.
  mbdv::Params params;
  std::string params_error;
  std::string params_used;
  if (!mbdv::LoadParams(cli.params_path, &params, &params_error, &params_used)) {
    std::cerr << "ERROR: " << cli.params_path << ": " << params_error << "\n"
              << "       Fix the file, or point elsewhere with --params <path>.\n"
              << "       Running on built-in defaults instead would be worse than refusing:"
              << " the control rate, the heartbeat periods and the wheel geometry all come"
              << " from this file.\n";
    return 1;
  }
  if (params_used.empty()) {
    std::cerr << "WARNING: no " << cli.params_path
              << " found; using built-in defaults. Copy config/params.yaml next to the"
              << " binary to pin the rates, heartbeat and wheel geometry.\n";
  }

  // Command-line overrides: only flags that were actually given.
  if (cli.interface_set) params.can_interface = cli.options.can_interface;
  if (cli.baud_set) params.expect_bitrate_bps = cli.options.expect_bitrate_bps;
  if (cli.axis1_set) params.axis1_node_id = cli.options.axis1_node_id;
  if (cli.axis2_set) params.axis2_node_id = cli.options.axis2_node_id;
  if (cli.options.write_control_mode != 0) {
    params.write_control_mode = cli.options.write_control_mode;
    if (!cli.mode_explicit || cli.options.expect_control_mode != 0) {
      params.expect_control_mode = cli.options.expect_control_mode;
    }
  }
  // The self-test runs with the P1-00 check switched off, so clear the value the YAML
  // supplies rather than merely declining to derive one.
  if (cli.mode == Cli::Mode::kSelfTest) params.expect_control_mode = 0;

  // P1-00 (0x2A30) and the CiA 402 mode must agree: S08 verifies 0x2A30 against
  // expect_control_mode and rewrites it when it differs, while S10 separately writes
  // 0x6060/0x6061 for the selected mode. Derive the expectation from the mode we are
  // actually going to run in, so no command line can leave the two contradicting each
  // other - previously a position mode kept the YAML value (15, velocity) while S10
  // selected Profile Position, so the drive ran in velocity while the master sent
  // position setpoints. An explicit --p1-00/--control-mode still wins.
  if (!cli.control_mode_explicit) {
    switch (cli.options.mode) {
      case mbdv::CiA402Mode::PROFILE_VELOCITY:
        params.expect_control_mode =
            static_cast<uint32_t>(mbdv::DriveControlMode::kVelocityControl);
        break;
      case mbdv::CiA402Mode::PROFILE_POSITION:
        params.expect_control_mode =
            static_cast<uint32_t>(mbdv::DriveControlMode::kPositionControl);
        break;
      default:
        break;
    }
  }
  if (cli.options.watchdog_timeout_ms != -1) params.watchdog_timeout_ms = cli.options.watchdog_timeout_ms;
  // params.yaml owns the ramp; cli.options defaults to the same numbers so an explicit
  // 0 in the file is the only way to skip the write.
  cli.options.profile_accel = params.profile_accel;
  cli.options.profile_decel = params.profile_decel;
  if (!cli.options.pdo.program_pdos) params.program_pdos = false;
  if (cli.dcf_set) params.dcf_path = cli.options.dcf_path;
  if (cli.bin_set) params.bin_path = cli.options.bin_path;
  if (cli.boot_timeout_set) params.boot_timeout = cli.options.boot_timeout;
  if (cli.servo_timeout_set) params.servo_timeout = cli.options.servo_timeout;

  if (cli.dump_params) {
    std::cout << "# effective parameters (from " << (params_used.empty() ? "built-in defaults"
                                                                          : params_used)
              << (cli.mode_explicit ? ", with command-line overrides" : "") << ")\n"
              << mbdv::DumpParams(params);
    return 0;
  }

  mbdv::Logger::Instance().Configure(cli.options.log_file, cli.options.log_level,
                                     cli.options.colour && isatty(fileno(stdout)) != 0);

  std::cout << "\n"
            << "===============================================================================\n"
            << "  Moons' MBDV-2X-520AC  |  Dual-axis CiA 402 CANopen master bring-up\n"
            << "  bus " << params.can_interface << " @ "
            << mbdv::can_bit_rate_bps_to_string(params.expect_bitrate_bps) << "  |  axis1 node "
            << static_cast<int>(params.axis1_node_id) << "  |  axis2 node "
            << static_cast<int>(params.axis2_node_id) << "  |  control "
            << params.control_rate_hz << " Hz  |  params "
            << (params_used.empty() ? "(defaults)" : params_used) << "\n"
            << "===============================================================================\n"
            << std::endl;

  mbdv::DualAxisController controller;
  mbdv::DiagnosticReport report;

  // The stop output. Drive your hardware IO from here - e.g. the drives' E-STOP inputs
  // (1_X4 / 2_X4) - because once the CAN connection is lost nothing sent over CAN reaches the
  // drive any more. Called on the control thread on every change, before the CAN stop
  // commands, so keep it short and non-blocking. controller.StopRequested() gives the same
  // value to any other thread.
  controller.SetStopCallback([](bool stop, const std::string& reason) {
    if (stop) {
      mbdv::LogWarn(mbdv::Stage::S13_MOTION_COMMAND, nullptr,
                    mbdv::Str("stop output ON (", reason, ") - set the stop IO here"));
    } else {
      mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr,
                    "stop output OFF - release the stop IO here");
    }
  });

  std::string config_error;
  if (!controller.Configure(params, &config_error)) {
    std::cerr << "WARNING: " << config_error
              << "\n         Continuing without the UDP odometry output; the control loop"
                 " itself is unaffected.\n";
  }

  // The options that Initialize() consumes are now derived from the parameters.
  mbdv::ControllerOptions options = cli.options;
  options.can_interface = params.can_interface;
  options.dcf_path = params.dcf_path;
  options.bin_path = params.bin_path;
  options.axis1_node_id = params.axis1_node_id;
  options.axis2_node_id = params.axis2_node_id;
  options.expect_bitrate_bps = params.expect_bitrate_bps;
  options.expect_control_mode = params.expect_control_mode;
  options.write_control_mode = params.write_control_mode;
  options.watchdog_timeout_ms = params.watchdog_timeout_ms;
  options.pdo.program_pdos = params.program_pdos;
  options.pdo.tpdo1_event_timer_ms = params.tpdo1_event_timer_ms;
  options.pdo.tpdo2_event_timer_ms = params.tpdo2_event_timer_ms;
  options.pdo.tpdo3_event_timer_ms = params.tpdo3_event_timer_ms;
  options.boot_timeout = params.boot_timeout;
  options.servo_timeout = params.servo_timeout;

  // ---- S01..S04 ----
  if (!controller.Initialize(options, report)) {
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
  // Built from `options` (the params-derived set), not from the raw CLI struct, so that
  // everything in config/params.yaml actually reaches the bring-up stages.
  mbdv::ControllerOptions bring_up_options = options;
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
    Loop(controller, Throttled([&] { controller.PrintTelemetry(); },
                              std::chrono::milliseconds(400)));
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
  if (!controller.EnableBothAxes(report, options.servo_timeout)) {
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
      // A short dwell with telemetry, then back to 0. This used to loop until Ctrl+C and then
      // break, so the return move below could never run.
      Loop(controller, Throttled([&] { controller.PrintTelemetry(); },
                                std::chrono::milliseconds(400)),
           std::chrono::milliseconds(1500));
      if (g_shutdown.load()) break;

      mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr, "PP test: returning both axes to 0");
      if (!controller.MoveBothAxesStaged(report, 0, 0, std::chrono::milliseconds(8000))) {
        exit_code = 1;
      }
      break;
    }

    case Cli::Mode::kVelocity: {
      const int32_t vel = cli.options.test_velocity;
      mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr,
                    mbdv::Str("PV test: AX1 = +", vel, " AX2 = ", mbdv::Sgn(-vel), " counts/s"));
      if (controller.SetVelocitiesStaged(report, vel, -vel, std::chrono::milliseconds(1500))) {
        Loop(controller, Throttled([&] { controller.PrintTelemetry(); },
                                  std::chrono::milliseconds(400)));
      } else {
        exit_code = 1;
      }
      // Always: the loop above only ends on Ctrl+C, so the old "if not shut down" guard meant
      // the zero was never sent and S15 disabled the servo with the motor still running.
      controller.SetVelocitiesStaged(report, 0, 0, std::chrono::milliseconds(500));
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
      auto last_display = std::chrono::steady_clock::now();
      for (const Phase& phase : phases) {
        if (g_shutdown.load()) break;
        mbdv::LogInfo(mbdv::Stage::S13_MOTION_COMMAND, nullptr,
                      mbdv::Str("kinematics phase: ", phase.label, " v=", phase.v,
                                " m/s w=", phase.w, " rad/s for ", phase.seconds, " s"));
        const auto phase_start = std::chrono::steady_clock::now();
        auto last = phase_start;
        auto next = phase_start;
        while (!g_shutdown.load() &&
               std::chrono::steady_clock::now() - phase_start <
                   std::chrono::seconds(phase.seconds)) {
          const auto now = std::chrono::steady_clock::now();
          controller.Supervise();
          controller.UpdateAndPublishOdometry(std::chrono::duration<double>(now - last).count());
          last = now;
          if (controller.StopRequested()) break;  // an axis went away: both are already stopped
          controller.SetCmdVel(phase.v, phase.w);
          if (now - last_display >= std::chrono::milliseconds(250)) {
            last_display = now;
            controller.PrintTelemetry();
          }
          next += controller.ControlPeriod();
          const auto sleep_for = next - std::chrono::steady_clock::now();
          if (sleep_for > std::chrono::milliseconds(0)) std::this_thread::sleep_for(sleep_for);
          else next = std::chrono::steady_clock::now();
        }
        if (controller.StopRequested()) {
          mbdv::LogError(mbdv::Stage::S13_MOTION_COMMAND, nullptr,
                         mbdv::Str("kinematics test aborted: ", controller.StopReason()));
          exit_code = 1;
          break;
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
      Loop(controller, Throttled([&] { controller.PrintTelemetry(); },
                                std::chrono::milliseconds(400)));
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