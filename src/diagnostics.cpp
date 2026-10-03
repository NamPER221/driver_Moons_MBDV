#include "mbdv/diagnostics.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>

namespace mbdv {
namespace {

// ---------------------------------------------------------------------------
// Stage metadata table
// ---------------------------------------------------------------------------

struct StageInfo {
  Stage stage;
  const char* code;
  const char* name;
  const char* purpose;
};

constexpr StageInfo kStages[] = {
    {Stage::NONE, "S00", "-", "no stage"},
    {Stage::S01_CONFIG, "S01", "CONFIG", "parse CLI arguments, resolve config files"},
    {Stage::S02_CAN_LINK, "S02", "CAN_LINK", "open SocketCAN controller and channel"},
    {Stage::S03_MASTER_LOAD, "S03", "MASTER_LOAD", "load master.dcf, create AsyncMaster"},
    {Stage::S04_EVENT_LOOP, "S04", "EVENT_LOOP", "start lely event-loop thread (master Reset)"},
    {Stage::S05_BOOTUP, "S05", "BOOTUP", "receive slave Boot-up frame (0x700+node = 0x00)"},
    {Stage::S06_PREOP, "S06", "PREOP", "confirm PRE-OPERATIONAL, SDO channel usable"},
    {Stage::S07_NMT_START, "S07", "NMT_START", "NMT Start Remote Node -> OPERATIONAL"},
    {Stage::S08_IDENTITY, "S08", "IDENTITY", "verify identity + bus params vs expectation"},
    {Stage::S09_PDO_VERIFY, "S09", "PDO_VERIFY", "verify PDO COB-ID, mapping, transmission"},
    {Stage::S10_MODE_OF_OPERATION, "S10", "MODE_OF_OP", "write 0x6060, confirm 0x6061 agrees"},
    {Stage::S11_FAULT_RESET, "S11", "FAULT_RESET", "clear latched fault (0x0080 / 0x2006)"},
    {Stage::S12_SERVO_ON, "S12", "SERVO_ON", "CiA 402: Shutdown -> SwitchOn -> EnableOp"},
    {Stage::S13_MOTION_COMMAND, "S13", "MOTION_CMD", "write target position 0x607A / velocity 0x60FF"},
    {Stage::S14_MOTION_TRACKING, "S14", "MOTION_TRACK", "wait Target Reached, verify feedback"},
    {Stage::S15_SERVO_OFF, "S15", "SERVO_OFF", "controlled ramp-down + Disable Operation"},
    {Stage::S16_SHUTDOWN, "S16", "SHUTDOWN", "stop event loop, release CAN resources"},
};

const StageInfo& Info(Stage stage) noexcept {
  for (const StageInfo& info : kStages) {
    if (info.stage == stage) return info;
  }
  return kStages[0];
}

const char* ColourFor(LogLevel level, bool colour) noexcept {
  if (!colour) return "";
  switch (level) {
    case LogLevel::TRACE: return "\033[90m";  // bright black
    case LogLevel::DEBUG: return "\033[36m";  // cyan
    case LogLevel::INFO: return "\033[32m";   // green
    case LogLevel::WARN: return "\033[33m";   // yellow
    case LogLevel::ERROR: return "\033[31m";  // red
    case LogLevel::FATAL: return "\033[1;35m";  // bold magenta
  }
  return "";
}

constexpr const char* kResetColour = "\033[0m";

/// Wraps @p text in an ANSI colour only when colour output is enabled.
std::string Paint(const char* colour, const std::string& text, bool enabled) {
  if (!enabled || colour == nullptr || colour[0] == '\0') return text;
  return std::string(colour) + text + kResetColour;
}

/// Truncates @p text so the diagnosis table keeps its column alignment.
std::string Ellipsize(const std::string& text, std::size_t max_len) {
  if (text.size() <= max_len) return text;
  if (max_len <= 3) return text.substr(0, max_len);
  return text.substr(0, max_len - 3) + "...";
}

}  // namespace

// ---------------------------------------------------------------------------
// Log level
// ---------------------------------------------------------------------------

const char* log_level_tag(LogLevel level) noexcept {
  switch (level) {
    case LogLevel::TRACE: return "TRACE";
    case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::INFO: return "INFO ";
    case LogLevel::WARN: return "WARN ";
    case LogLevel::ERROR: return "ERROR";
    case LogLevel::FATAL: return "FATAL";
  }
  return "?????";
}

const char* log_level_name(LogLevel level) noexcept {
  switch (level) {
    case LogLevel::TRACE: return "TRACE";
    case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::INFO: return "INFO";
    case LogLevel::WARN: return "WARN";
    case LogLevel::ERROR: return "ERROR";
    case LogLevel::FATAL: return "FATAL";
  }
  return "UNKNOWN";
}

bool parse_log_level(const std::string& text, LogLevel& out) noexcept {
  std::string upper;
  upper.reserve(text.size());
  for (char c : text) {
    upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
  }
  if (upper == "TRACE") { out = LogLevel::TRACE; return true; }
  if (upper == "DEBUG") { out = LogLevel::DEBUG; return true; }
  if (upper == "INFO") { out = LogLevel::INFO; return true; }
  if (upper == "WARN" || upper == "WARNING") { out = LogLevel::WARN; return true; }
  if (upper == "ERROR") { out = LogLevel::ERROR; return true; }
  if (upper == "FATAL" || upper == "CRITICAL") { out = LogLevel::FATAL; return true; }
  return false;
}

// ---------------------------------------------------------------------------
// Stages
// ---------------------------------------------------------------------------

const char* stage_code(Stage stage) noexcept { return Info(stage).code; }
const char* stage_name(Stage stage) noexcept { return Info(stage).name; }
const char* stage_purpose(Stage stage) noexcept { return Info(stage).purpose; }

bool stage_reached(Stage stage) noexcept { return stage_index(stage) > 0; }

const char* stage_status_tag(StageStatus status) noexcept {
  switch (status) {
    case StageStatus::PASS: return "PASS";
    case StageStatus::FAIL: return "FAIL";
    case StageStatus::SKIP: return "SKIP";
    case StageStatus::PENDING: return "----";
  }
  return "????";
}

// ---------------------------------------------------------------------------
// Formatting helpers
// ---------------------------------------------------------------------------

std::string Hex(uint64_t value, int width) {
  std::ostringstream os;
  os << std::uppercase << std::hex << std::setfill('0');
  if (width > 0) os << std::setw(width);
  os << value;
  return os.str();
}

/// "0x1401:01" - the canonical way object references are written in this project.
/// The "0x" prefix is included, so this must not be combined with a literal "0x".
std::string ObjRef(uint16_t index, uint8_t sub_index) {
  return Str("0x", Hex(index, 4), ":", Hex(static_cast<uint64_t>(sub_index), 2));
}

std::string Sgn(int64_t value) {
  std::ostringstream os;
  os << (value < 0 ? "-" : "+") << std::llabs(value);
  return os.str();
}

// ---------------------------------------------------------------------------
// Logger
// ---------------------------------------------------------------------------

Logger& Logger::Instance() noexcept {
  static Logger instance;
  return instance;
}

void Logger::Configure(const std::string& log_file, LogLevel min_level, bool use_colour) {
  std::lock_guard<std::mutex> lock(mutex_);
  min_level_ = min_level;
  colour_ = use_colour;
  t0_ = std::chrono::steady_clock::now();
  log_path_.clear();
  last_duplicate_.clear();
  duplicate_count_ = 0;
  if (!log_file.empty()) {
    std::ofstream probe(log_file, std::ios::out | std::ios::trunc);
    if (probe.is_open()) {
      log_path_ = log_file;
    } else {
      log_path_.clear();
    }
  }
}

void Logger::SetMinLevel(LogLevel level) noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  min_level_ = level;
}

LogLevel Logger::GetMinLevel() const noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  return min_level_;
}

bool Logger::SetLogFile(const std::string& path) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (path.empty()) {
    log_path_.clear();
    return true;
  }
  std::ofstream probe(path, std::ios::out | std::ios::trunc);
  if (!probe.is_open()) return false;
  log_path_ = path;
  return true;
}

void Logger::CloseLogFile() {
  std::lock_guard<std::mutex> lock(mutex_);
  log_path_.clear();
}

bool Logger::IsEnabled(LogLevel level) const {
  std::lock_guard<std::mutex> lock(mutex_);
  return static_cast<int>(level) >= static_cast<int>(min_level_);
}

std::chrono::milliseconds Logger::Elapsed() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - t0_);
}

void Logger::ResetDuplicateFilter() noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  last_duplicate_.clear();
  duplicate_count_ = 0;
}

void Logger::Emit(LogLevel level, Stage stage, const char* axis, const std::string& message) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (static_cast<int>(level) < static_cast<int>(min_level_)) return;

  // Collapse bursts of identical lines (an absent TPDO would otherwise flood the log).
  std::string line_key =
      Str(stage_code(stage), "|", axis ? axis : "", "|", log_level_name(level), "|", message);
  if (line_key == last_duplicate_) {
    ++duplicate_count_;
    return;
  }
  const int suppress = duplicate_count_;
  last_duplicate_ = line_key;
  duplicate_count_ = 0;

  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - t0_)
                      .count();

  std::ostringstream os;
  os << '[' << std::setw(8) << std::fixed << std::setprecision(3)
     << (static_cast<double>(ms) / 1000.0) << "s]"
     << '[' << stage_code(stage) << "]"
     << '[' << std::left << std::setw(6) << (axis ? axis : "-") << std::right << ']'
     << '[' << Paint(ColourFor(level, colour_), log_level_tag(level), colour_) << "] "
     << message;
  if (suppress > 0) {
    os << Str("  (", suppress, " identical earlier line(s) suppressed)");
  }

  const std::string text = os.str();
  std::cout << text << std::endl;

  if (!log_path_.empty()) {
    std::ofstream file(log_path_, std::ios::out | std::ios::app);
    if (file.is_open()) {
      file << text << '\n';
    }
  }
}

void Logger::Banner(Stage stage, const char* axis) {
  std::lock_guard<std::mutex> lock(mutex_);
  const std::string head =
      Str("---- ", stage_code(stage), " ", stage_name(stage),
          axis ? Str("  [", axis, "]") : std::string(),
          " : ", stage_purpose(stage), " ----");
  const std::string painted = Paint(colour_ ? "\033[1;36m" : nullptr, head, colour_);
  std::cout << '\n' << painted << std::endl;
  if (!log_path_.empty()) {
    std::ofstream file(log_path_, std::ios::out | std::ios::app);
    if (file.is_open()) file << '\n' << head << '\n';
  }
}

// ---------------------------------------------------------------------------
// Free-function helpers
// ---------------------------------------------------------------------------

void LogTrace(Stage s, const char* axis, const std::string& msg) {
  Logger::Instance().Emit(LogLevel::TRACE, s, axis, msg);
}
void LogDebug(Stage s, const char* axis, const std::string& msg) {
  Logger::Instance().Emit(LogLevel::DEBUG, s, axis, msg);
}
void LogInfo(Stage s, const char* axis, const std::string& msg) {
  Logger::Instance().Emit(LogLevel::INFO, s, axis, msg);
}
void LogWarn(Stage s, const char* axis, const std::string& msg) {
  Logger::Instance().Emit(LogLevel::WARN, s, axis, msg);
}
void LogError(Stage s, const char* axis, const std::string& msg) {
  Logger::Instance().Emit(LogLevel::ERROR, s, axis, msg);
}
void LogFatal(Stage s, const char* axis, const std::string& msg) {
  Logger::Instance().Emit(LogLevel::FATAL, s, axis, msg);
}

void LogBanner(Stage s, const char* axis) { Logger::Instance().Banner(s, axis); }

// ---------------------------------------------------------------------------
// DiagnosticReport
// ---------------------------------------------------------------------------

void DiagnosticReport::Record(Stage stage, const char* axis, StageStatus status,
                             std::string reason, std::string hint) {
  std::lock_guard<std::mutex> lock(mutex_);
  const std::string key = axis ? axis : "";
  for (StageOutcome& outcome : outcomes_) {
    if (outcome.stage == stage && outcome.axis == key) {
      outcome.status = status;
      outcome.reason = std::move(reason);
      outcome.hint = std::move(hint);
      return;
    }
  }
  StageOutcome outcome;
  outcome.stage = stage;
  outcome.axis = key;
  outcome.status = status;
  outcome.reason = std::move(reason);
  outcome.hint = std::move(hint);
  outcomes_.push_back(std::move(outcome));
  std::sort(outcomes_.begin(), outcomes_.end(),
            [](const StageOutcome& a, const StageOutcome& b) {
              return stage_index(a.stage) < stage_index(b.stage);
            });
}

void DiagnosticReport::Pass(Stage stage, const char* axis, std::string reason) {
  Record(stage, axis, StageStatus::PASS, std::move(reason));
}

void DiagnosticReport::Fail(Stage stage, const char* axis, std::string reason,
                            std::string hint) {
  Record(stage, axis, StageStatus::FAIL, std::move(reason), std::move(hint));
}

void DiagnosticReport::Skip(Stage stage, const char* axis, std::string reason) {
  Record(stage, axis, StageStatus::SKIP, std::move(reason));
}

void DiagnosticReport::Pending(Stage stage, const char* axis) {
  Record(stage, axis, StageStatus::PENDING, "not reached");
}

bool DiagnosticReport::RecordFailure(Stage stage, const char* axis,
                                     const std::string& reason, const std::string& hint) {
  Fail(stage, axis, reason, hint);
  LogError(stage, axis, Str("STAGE FAILED | ", stage_code(stage), " ", stage_name(stage),
                            " | reason: ", reason, hint.empty() ? "" : Str(" | hint: ", hint)));
  return false;
}

const StageOutcome* DiagnosticReport::Find(Stage stage, const char* axis) const {
  std::lock_guard<std::mutex> lock(mutex_);
  const std::string key = axis ? axis : "";
  // Exact axis match first, then the bus-global record for the same stage.
  for (const StageOutcome& outcome : outcomes_) {
    if (outcome.stage == stage && outcome.axis == key) return &outcome;
  }
  if (!key.empty()) {
    for (const StageOutcome& outcome : outcomes_) {
      if (outcome.stage == stage && outcome.axis.empty()) return &outcome;
    }
  }
  return nullptr;
}

const StageOutcome* DiagnosticReport::FirstFailure() const {
  std::lock_guard<std::mutex> lock(mutex_);
  for (const StageOutcome& outcome : outcomes_) {
    if (outcome.status == StageStatus::FAIL) return &outcome;
  }
  return nullptr;
}

Stage DiagnosticReport::LastSuccessfulStage() const {
  std::lock_guard<std::mutex> lock(mutex_);
  Stage best = Stage::NONE;
  for (const StageOutcome& outcome : outcomes_) {
    if (outcome.status == StageStatus::PASS && stage_index(outcome.stage) > stage_index(best)) {
      best = outcome.stage;
    }
  }
  return best;
}

Stage DiagnosticReport::LastSuccessfulStageForAxis(const char* axis) const {
  std::lock_guard<std::mutex> lock(mutex_);
  const std::string key = axis ? axis : "";
  Stage best = Stage::NONE;
  for (const StageOutcome& outcome : outcomes_) {
    if (outcome.status == StageStatus::PASS && outcome.axis == key &&
        stage_index(outcome.stage) > stage_index(best)) {
      best = outcome.stage;
    }
  }
  if (best != Stage::NONE) return best;
  return LastSuccessfulStage();
}

bool DiagnosticReport::AllPassed() const noexcept { return FirstFailure() == nullptr; }

void DiagnosticReport::Clear() noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  outcomes_.clear();
}

void DiagnosticReport::PrintFirstFailure(std::ostream& os) const {
  const StageOutcome* failure = FirstFailure();
  if (!failure) return;
  const std::size_t rule = 78;
  os << '\n' << std::string(rule, '=') << '\n'
     << "  DIAGNOSIS - FIRST FAILING STAGE" << '\n'
     << std::string(rule, '=') << '\n'
     << "  Stage      : " << stage_code(failure->stage) << "  " << stage_name(failure->stage)
     << '\n'
     << "  Purpose    : " << stage_purpose(failure->stage) << '\n'
     << "  Axis       : " << (failure->axis.empty() ? "(bus-global)" : failure->axis) << '\n'
     << "  Elapsed    : " << failure->elapsed.count() << " ms\n"
     << "  Reason     : " << failure->reason << '\n';
  if (!failure->hint.empty()) {
    os << "  Hint       : " << failure->hint << '\n';
  }
  const Stage last_ok = LastSuccessfulStageForAxis(failure->axis.c_str());
  os << "  Last OK    : " << stage_code(last_ok) << " " << stage_name(last_ok) << '\n'
     << "  Conclusion : bring-up for "
     << (failure->axis.empty() ? "the bus" : failure->axis)
     << " worked up to and including " << stage_code(last_ok)
     << "; the fault is isolated to " << stage_code(failure->stage) << " "
     << stage_name(failure->stage) << ".\n"
     << std::string(rule, '=') << '\n' << std::flush;
}

bool DiagnosticReport::Print(std::ostream& os, const std::string& title) const {
  std::lock_guard<std::mutex> lock(mutex_);
  const StageOutcome* failure = nullptr;
  for (const StageOutcome& outcome : outcomes_) {
    if (outcome.status == StageStatus::FAIL) {
      failure = &outcome;
      break;
    }
  }

  // No ANSI colour here on purpose: the report is also written to log files and
  // piped to less, where escape sequences are noise rather than signal.
  const char* verdict = failure ? "FAILURE" : "SUCCESS";

  const std::size_t rule = 78;
  os << '\n' << std::string(rule, '=') << '\n'
     << "  " << title << " : " << verdict << '\n'
     << std::string(rule, '=') << '\n'
     << "  Stage  Name            Axis   Result  Elapsed   Reason"
     << '\n'
     << "  " << std::string(rule - 2, '-') << '\n';

  for (const StageOutcome& outcome : outcomes_) {
    const std::string elapsed =
        outcome.status == StageStatus::PENDING ? std::string("-")
                                               : Str(outcome.elapsed.count(), "ms");
    os << "  " << std::left << std::setw(6) << stage_code(outcome.stage) << std::setw(14)
       << stage_name(outcome.stage) << std::setw(7)
       << (outcome.axis.empty() ? "-" : outcome.axis) << std::setw(8)
       << stage_status_tag(outcome.status) << std::setw(10) << elapsed << ' '
       << Ellipsize(outcome.reason, 44) << '\n';
  }

  if (failure) {
    os << std::string(rule, '=') << '\n'
       << "  >>> FIRST FAILING STAGE : " << stage_code(failure->stage) << "  "
       << stage_name(failure->stage) << "   (axis "
       << (failure->axis.empty() ? "bus-global" : failure->axis) << ")\n"
       << "  >>> REASON              : " << failure->reason << '\n';
    if (!failure->hint.empty()) {
      os << "  >>> HINT                : " << failure->hint << '\n';
    }
    // Scoped to the failing axis: a global maximum would report the other axis's
    // progress and badly misrepresent where this axis actually stopped.
    const Stage same_axis_ok = LastSuccessfulStageForAxis(failure->axis.c_str());
    os << "  >>> LAST GOOD STAGE     : " << stage_code(same_axis_ok) << " "
       << stage_name(same_axis_ok) << "  (for " << failure->axis << ")\n"
       << "  >>> VERDICT             : bring-up is blocked at " << stage_code(failure->stage)
       << " " << stage_name(failure->stage) << '\n'
       << std::string(rule, '=') << '\n';
  } else {
    os << std::string(rule, '=') << '\n'
       << "  >>> Every recorded stage returned PASS.\n"
       << std::string(rule, '=') << '\n';
  }
  os << std::flush;
  return failure == nullptr;
}

}  // namespace mbdv