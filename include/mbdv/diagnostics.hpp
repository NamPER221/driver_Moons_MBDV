#pragma once

/**
 * @file diagnostics.hpp
 * @brief Staged logging + bring-up diagnosis for the Moons' MBDV dual-axis CANopen master.
 *
 * Design goal: every step of the bring-up sequence is a numbered STAGE. Each stage
 * emits exactly one verdict (PASS / FAIL / SKIP) into a DiagnosticReport, and the
 * report prints the first failing stage together with its reason and an actionable
 * hint. That makes it possible to answer "which stage did it fail at?" from the
 * console output alone.
 *
 * Log line format (one line per event):
 *
 *   [  1.234s][S12    ][AX1    ][INFO ] CiA402: Enable Operation sent (CW=0x000F)
 *   ^ timestamp  ^ stage  ^ axis    ^ level   ^ message
 *
 * References used while writing this file:
 *  - docx/CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds  (object dictionary, manufacturer objects)
 *  - docx/MBDV-Hardware-Manual-EN20230926-MOONS.pdf  section 4.2.2 (DIP switches)
 *                                                    section 6   (commissioning steps)
 *                                                    section 9.1 (drive alarm inventory)
 */

#include <chrono>
#include <cstdint>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace mbdv {

// ---------------------------------------------------------------------------
// Log levels
// ---------------------------------------------------------------------------

enum class LogLevel : int {
  TRACE = 0,
  DEBUG = 1,
  INFO = 2,
  WARN = 3,
  ERROR = 4,
  FATAL = 5,
};

const char* log_level_tag(LogLevel level) noexcept;
const char* log_level_name(LogLevel level) noexcept;
bool parse_log_level(const std::string& text, LogLevel& out) noexcept;

// ---------------------------------------------------------------------------
// Bring-up stages
// ---------------------------------------------------------------------------

/**
 * @brief Ordered bring-up stages. The numeric order *is* the execution order,
 * so the first failing stage also identifies the last stage that succeeded.
 */
enum class Stage : int {
  NONE = 0,

  S01_CONFIG,          ///< Parse CLI arguments, resolve configuration paths
  S02_CAN_LINK,        ///< SocketCAN controller + channel open
  S03_MASTER_LOAD,     ///< Load master.dcf and create AsyncMaster
  S04_EVENT_LOOP,      ///< Start the lely event-loop thread (master Reset posted)
  S05_BOOTUP,          ///< Wait for slave Boot-up frame (0x700+node, data 0x00)
  S06_PREOP,           ///< Confirm PRE-OPERATIONAL (0x7F) / SDO channel alive
  S07_NMT_START,       ///< NMT Start Remote Node -> OPERATIONAL (0x01)
  S08_IDENTITY,        ///< Verify identity + bus parameters against expectation
  S09_PDO_VERIFY,      ///< Verify PDO COB-ID / mapping / transmission type
  S10_MODE_OF_OPERATION,  ///< Write 0x6060 and confirm 0x6061 display agrees
  S11_FAULT_RESET,     ///< Clear a latched fault (0x0080 / 0x2006) if needed
  S12_SERVO_ON,        ///< CiA 402 state machine Shutdown -> SwitchOn -> EnableOp
  S13_MOTION_COMMAND,  ///< Write target position (0x607A) or target velocity (0x60FF)
  S14_MOTION_TRACKING, ///< Wait for Target Reached / verify feedback follows command
  S15_SERVO_OFF,       ///< Controlled ramp-down and Disable Operation
  S16_SHUTDOWN,        ///< Stop event loop and release CAN resources

  COUNT
};

/// Short machine-readable stage code, e.g. "S08".
const char* stage_code(Stage stage) noexcept;
/// Human-readable stage name, e.g. "IDENTITY".
const char* stage_name(Stage stage) noexcept;
/// One-line description of what the stage is supposed to prove.
const char* stage_purpose(Stage stage) noexcept;
/// True once stage execution order has reached @p stage.
bool stage_reached(Stage stage) noexcept;

inline constexpr int stage_index(Stage s) noexcept {
  return static_cast<int>(s);
}

// ---------------------------------------------------------------------------
// Stage verdict
// ---------------------------------------------------------------------------

enum class StageStatus : int { PASS = 0, FAIL = 1, SKIP = 2, PENDING = 3 };

const char* stage_status_tag(StageStatus status) noexcept;

/// One recorded stage verdict.
struct StageOutcome {
  Stage stage{Stage::NONE};
  std::string axis;      ///< "AX1", "AX2" or "" for bus-global stages
  StageStatus status{StageStatus::PENDING};
  std::string reason;    ///< Short factual reason (always set, also on PASS)
  std::string hint;      ///< Actionable remediation hint (usually only on FAIL)
  std::chrono::milliseconds elapsed{0};
};

// ---------------------------------------------------------------------------
// Small string builder (avoids pulling in <format>, which needs GCC 13)
// ---------------------------------------------------------------------------

namespace detail {
inline void AppendAll(std::ostringstream&) noexcept {}

template <typename T, typename... Rest>
void AppendAll(std::ostringstream& os, const T& first, const Rest&... rest) {
  os << first;
  AppendAll(os, rest...);
}
}  // namespace detail

/// Concatenates its arguments into a std::string, e.g. Str("pos=", pos).
template <typename... Args>
std::string Str(const Args&... args) {
  std::ostringstream os;
  detail::AppendAll(os, args...);
  return os.str();
}

/// Formats an integer as fixed-width hex WITHOUT a "0x" prefix, e.g. Hex(0x6041, 4) == "6041".
/// Use ObjRef() for object references so a "0x" prefix appears exactly once.
std::string Hex(uint64_t value, int width = 0);

/// Formats a CANopen object reference, e.g. ObjRef(0x1401, 1) == "0x1401:01".
std::string ObjRef(uint16_t index, uint8_t sub_index);

/// Formats a signed value with an explicit sign, e.g. Sgn(-3) == "-3".
std::string Sgn(int64_t value);

// ---------------------------------------------------------------------------
// Logger
// ---------------------------------------------------------------------------

/**
 * @brief Process-wide structured logger. Thread-safe: CANopen callbacks run on
 * the event-loop thread while the main thread logs progress, so every write is
 * serialised through a mutex.
 */
class Logger {
 public:
  static Logger& Instance() noexcept;

  /**
   * @brief Configures the logger.
   * @param log_file Optional file to mirror the console output into ("" = no file).
   * @param min_level Messages below this level are discarded.
   * @param use_colour Emit ANSI colour codes on a TTY.
   */
  void Configure(const std::string& log_file, LogLevel min_level, bool use_colour);
  void SetMinLevel(LogLevel level) noexcept;
  LogLevel GetMinLevel() const noexcept;

  /// Mirrors every line into @p path in addition to the console.
  bool SetLogFile(const std::string& path);
  void CloseLogFile();

  bool IsEnabled(LogLevel level) const;

  /// Core entry point. @p axis may be empty for bus-global messages.
  void Emit(LogLevel level, Stage stage, const char* axis, const std::string& message);

  /// Emits the banner that delimits a stage, e.g. "--- S12 SERVO_ON ---".
  void Banner(Stage stage, const char* axis);

  /// Milliseconds since Logger::Configure() (or first use).
  std::chrono::milliseconds Elapsed() const;

  /// Suppresses repeated identical messages (e.g. a TPDO that never arrives).
  void ResetDuplicateFilter() noexcept;

 private:
  Logger() = default;
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  mutable std::mutex mutex_;
  std::chrono::steady_clock::time_point t0_{std::chrono::steady_clock::now()};
  std::string log_path_;
  LogLevel min_level_{LogLevel::INFO};
  bool colour_{false};
  std::string last_duplicate_;
  int duplicate_count_{0};
};

// ---------------------------------------------------------------------------
// Convenient free-function logging helpers
// ---------------------------------------------------------------------------

void LogTrace(Stage stage, const char* axis, const std::string& msg);
void LogDebug(Stage stage, const char* axis, const std::string& msg);
void LogInfo(Stage stage, const char* axis, const std::string& msg);
void LogWarn(Stage stage, const char* axis, const std::string& msg);
void LogError(Stage stage, const char* axis, const std::string& msg);
void LogFatal(Stage stage, const char* axis, const std::string& msg);

void LogBanner(Stage stage, const char* axis = nullptr);

// ---------------------------------------------------------------------------
// DiagnosticReport
// ---------------------------------------------------------------------------

/**
 * @brief Collects stage verdicts and renders the final "where did it fail?"
 * diagnosis.
 */
class DiagnosticReport {
 public:
  /// Records (or overwrites) the verdict for a stage/axis pair.
  void Record(Stage stage, const char* axis, StageStatus status, std::string reason,
              std::string hint = "");

  /// Convenience wrappers.
  void Pass(Stage stage, const char* axis, std::string reason = "OK");
  void Fail(Stage stage, const char* axis, std::string reason, std::string hint = "");
  void Skip(Stage stage, const char* axis, std::string reason);
  void Pending(Stage stage, const char* axis);

  /// Marks a stage FAIL with a reason/hint, then logs it as an error. Returns false.
  bool RecordFailure(Stage stage, const char* axis, const std::string& reason,
                     const std::string& hint);

/// Verdict for a stage, or nullptr when the stage never ran.
  const StageOutcome* Find(Stage stage, const char* axis = nullptr) const;

  /// First FAIL in execution order, or nullptr when nothing failed.
  const StageOutcome* FirstFailure() const;

  /** Highest stage index that reached a PASS (== last successfully completed stage). */
  Stage LastSuccessfulStage() const;

  /**
   * @brief Highest stage that reached a PASS *for the same axis as @p axis*.
   *
   * A dual-axis report mixes both axes, so a global maximum would be misleading: AX1
   * passing S11 while AX2 fails at S08 would otherwise report "Last OK: S11", implying
   * AX2 got much further than it did. Falls back to the global maximum when the axis has
   * no PASS of its own, then to Stage::NONE.
   */
  Stage LastSuccessfulStageForAxis(const char* axis) const;

  bool AllPassed() const noexcept;
  bool HasFailure() const noexcept { return FirstFailure() != nullptr; }
  std::size_t Count() const noexcept { return outcomes_.size(); }

  /**
   * @brief Prints the verdict table, the first failing stage with its reason and
   * hint, and a checklist of what to verify on the hardware.
   * @return true when no stage failed.
   */
  bool Print(std::ostream& os, const std::string& title) const;

  /// Prints only the "FIRST FAILURE" block (used when the run aborts early).
  void PrintFirstFailure(std::ostream& os) const;

  void Clear() noexcept;

 private:
  mutable std::mutex mutex_;
  std::vector<StageOutcome> outcomes_;
};

}  // namespace mbdv