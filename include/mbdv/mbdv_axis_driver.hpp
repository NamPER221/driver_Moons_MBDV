#pragma once

#include "mbdv/cia402_defs.hpp"
#include "mbdv/diagnostics.hpp"
#include "mbdv/drive_errors.hpp"

#include <lely/coapp/fiber_driver.hpp>
#include <lely/coapp/master.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace mbdv {

/**
 * @brief Runtime PDO layout this driver programs and verifies in stage S09.
 *
 * COB-IDs follow the drive factory defaults (EDS 0x1400/0x1600 and 0x1800/0x1A00):
 *   RPDO1 0x200+node : Controlword + Modes of operation
 *   RPDO2 0x300+node : Controlword + Target position
 *   RPDO3 0x400+node : Controlword + Target velocity
 *   TPDO1 0x180+node : Statusword                (10 ms event timer)
 *   TPDO2 0x280+node : Position actual + Velocity actual (10 ms event timer)
 *   TPDO3 0x380+node : Error code + DSP alarm code (diagnostics, 100 ms)
 *
 * Note on transmission type: the drive ships 0x1401:02 and 0x1402:02 = 0xFE, whose
 * low two bits (10b) select "RTR only". An RPDO left in that mode silently discards
 * normally-transmitted frames, so target position/velocity commands would never be
 * acted upon. Stage S09 therefore writes 0xFF (event driven) and verifies it.
 */
struct PdoPlan {
  uint8_t transmission_type{0xFF};  ///< low 2 bits = 11b -> event driven
  uint16_t tpdo1_event_timer_ms{10};
  uint16_t tpdo2_event_timer_ms{10};
  uint16_t tpdo3_event_timer_ms{100};
  bool program_pdos{true};  ///< false = verify only, never write
};

/// Which transport actually got a Controlword through to the drive.
enum class ControlwordPath : int {
  kNone = 0,  ///< neither RPDO nor SDO produced the expected transition
  kRpdo = 1,  ///< real-time PDO worked
  kSdo = 2,   ///< only the SDO fallback worked -> the RPDO path is not delivering
};

const char* controlword_path_to_string(ControlwordPath path) noexcept;

/// Options for BringUp().
struct BringUpOptions {
  /// Expected bus speed in bit/s, compared against object 0x2021. 0 = skip the check.
  uint32_t expect_bitrate_bps{500000};
  /// Expected drive control mode from 0x2A30 (see DriveControlMode). 0 = skip.
  uint32_t expect_control_mode{0};
  /**
   * @brief When non-zero, S08 writes this value to 0x2A30 (P1-00) *before* checking it.
   *
   * This must happen inside S08: the mode check lives there, so writing the value later
   * could never repair a mismatch.
   */
  uint32_t write_control_mode{0};
  /// Warn in S08 when Statusword bit 4 says the main voltage is absent.
  bool check_dc_bus{true};
  /// Retry a failed Controlword over SDO. Disable to test the RPDO path in isolation.
  bool sdo_controlword_fallback{true};
  /**
   * @brief Send setpoints over SDO instead of RPDO.
   *
   * Set on the command line for drives whose firmware receives TPDOs but ignores RPDOs,
   * which was measured on an MBDV-2X-520AC: a correctly formed 0x201 frame carrying
   * CW=0x0006 never changed the Statusword, while the identical value written over SDO did.
   * SDO is far slower than PDO, so this is a fallback, not the intended path.
   */
  bool sdo_setpoints{false};
  /**
   * @brief Communication watchdog (0x2060, manual P1-39), applied in S08.
   *
   * -1 leave untouched, 0 disable (0x2060:01 = 0), >0 set the timeout in ms. A drive
   * configured for communication control expects periodic RPDO traffic and raises
   * EMERGENCY xxxx with error-register bit 4 (COMMUNICATION) once 0x2060:03 elapses -
   * measured at 525 ms with the 500 ms default, immediately after a setpoint was sent.
   */
  int32_t watchdog_timeout_ms{-1};
  /// Persist a --p1-00 change with 0x1010:01 = 1. Off by default: see the comment in
  /// SetDriveControlModeImpl().
  bool store_parameters{false};
  PdoPlan pdo{};
  std::chrono::milliseconds boot_timeout{3000};
  std::chrono::milliseconds sdo_timeout{1000};
};

/**
 * @brief CiA 402 axis driver for one Moons' MBDV channel.
 *
 * Bring-up is split into numbered stages so that a failure can be attributed to an
 * exact step; see mbdv/diagnostics.hpp for the stage list.
 */
class MbdvAxisDriver : public lely::canopen::FiberDriver {
 public:
  MbdvAxisDriver(lely::canopen::AsyncMaster& master, uint8_t node_id,
                 std::string axis_name, const char* axis_tag);

  ~MbdvAxisDriver() override = default;

  MbdvAxisDriver(const MbdvAxisDriver&) = delete;
  MbdvAxisDriver& operator=(const MbdvAxisDriver&) = delete;

  // ------------------------------------------------------------------
  // Staged bring-up
  // ------------------------------------------------------------------

  /**
   * @brief Runs stages S05..S11 for this node and records one verdict per stage
   * in @p report. Blocking.
   * @return true when every executed stage passed.
   */
  bool BringUp(DiagnosticReport& report, const BringUpOptions& options = {});

  /// Stage S12 - CiA 402 Shutdown -> SwitchOn -> Enable Operation.
  bool EnableServoStaged(DiagnosticReport& report,
                         std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));

  /// Stage S15 - Disable Operation -> Disable Voltage -> Switch On Disabled.
  bool DisableServoStaged(DiagnosticReport& report,
                          std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));

  /// Stage S10 - write 0x6060 and confirm 0x6061 reports the same mode.
  bool SetModeStaged(DiagnosticReport& report, CiA402Mode mode,
                     std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

  /// Stage S13 + S14 - command a target and wait for Target Reached.
  bool MoveToPositionStaged(DiagnosticReport& report, int32_t target,
                            std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

  /// Stage S13 + S14 - command a velocity and confirm the feedback follows.
  bool SetVelocityStaged(DiagnosticReport& report, int32_t target,
                         std::chrono::milliseconds settle = std::chrono::milliseconds(500));

  /// Reads the diagnostic objects listed in DriveSnapshot. Blocking.
  DriveSnapshot ReadDriveSnapshot();

  /**
   * @brief Starts a background poll of 0x200F / 0x1001 / 0x603F and logs every change.
   *
   * The alarm codes "40" and "11" were seen blinking on the drive but never appeared in
   * 0x200F at the instants sampled by the snapshot, so they occur while the tool is not
   * looking. A continuous poll with a timestamp is the only way to catch them.
   */
  void StartAlarmWatch(std::chrono::milliseconds period);
  void StopAlarmWatch();

  /// Sets whether setpoints go over SDO instead of RPDO (see BringUpOptions::sdo_setpoints).
  void SetSdoSetpoints(bool enabled) noexcept { sdo_setpoints_.store(enabled); }
  /// Enables the opt-in 0x1010:01 parameter save used by --p1-00.
  void SetStoreParameters(bool enabled) noexcept { store_parameters_ = enabled; }
  /// True when setpoints are being delivered over SDO.
  bool GetSdoSetpoints() const noexcept { return sdo_setpoints_.load(); }

  // ------------------------------------------------------------------
  // Unstaged helpers (kept for the simple single-axis flow)
  // ------------------------------------------------------------------

  void RequestNmtStart();
  void RequestNmtPreOp();
  void RequestNmtReset();

  void SetModeOfOperation(CiA402Mode mode);
  bool EnableServo(std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));
  bool DisableServo();
  void ResetFault();
  void QuickStop();

  void SetTargetPosition(int32_t target_position, bool new_setpoint = true,
                         bool immediate = true, bool relative = false);
  void SetTargetVelocity(int32_t target_velocity);
  void SendControlword(uint16_t controlword);

  /// Sets the drive control mode parameter P1-00 (object 0x2A30). Blocking.
  bool SetDriveControlMode(uint32_t p1_00_value);

  // ------------------------------------------------------------------
  // Status & telemetry (thread-safe)
  // ------------------------------------------------------------------

  uint8_t GetNodeId() const noexcept { return node_id_; }
  const std::string& GetAxisName() const noexcept { return axis_name_; }
  const char* GetAxisTag() const noexcept { return axis_tag_; }

  uint16_t GetStatusword() const noexcept { return statusword_.load(); }
  CiA402State GetCiA402State() const noexcept { return state_.load(); }
  int32_t GetActualPosition() const noexcept { return actual_position_.load(); }
  int32_t GetActualVelocity() const noexcept { return actual_velocity_.load(); }
  int32_t GetFollowingError() const noexcept { return following_error_.load(); }
  uint16_t GetErrorCode() const noexcept { return error_code_.load(); }
  uint32_t GetDspAlarmCode() const noexcept { return dsp_alarm_.load(); }
  uint8_t GetErrorRegister() const noexcept { return error_register_.load(); }
  int8_t GetModeDisplay() const noexcept { return mode_display_.load(); }

  bool IsTargetReached() const noexcept { return target_reached_.load(); }
  bool IsOperational() const noexcept { return is_operational_.load(); }
  bool HasFault() const noexcept { return state_.load() == CiA402State::FAULT; }

  bool SawBootUp() const noexcept { return boot_seen_.load(); }
  bool SawConfigDone() const noexcept { return config_done_.load(); }
  bool SawEmergency() const noexcept { return emergency_seen_.load(); }
  uint16_t GetLastEmergencyCode() const noexcept { return last_emcy_code_.load(); }
  std::chrono::steady_clock::time_point LastStatuswordUpdate() const noexcept {
    return statusword_time_;
  }
  std::chrono::steady_clock::time_point LastEmergencyTime() const noexcept {
    return emcy_time_;
  }

  /// True when no Statusword has arrived for @p window (TPDO1 not flowing).
  bool IsTelemetryStale(std::chrono::milliseconds window) const;

  /// Number of received TPDO cycles, used to prove the feedback path works.
  /// Number of Statusword frames received via TPDO (excludes SDO refreshes).
  uint64_t GetStatuswordCount() const noexcept { return statusword_count_.load(); }
  /// Number of Statusword reads performed over SDO as a TPDO fallback.
  uint64_t GetStatuswordSdoReads() const noexcept { return statusword_sdo_reads_.load(); }
  /// Setpoints delivered over SDO instead of RPDO (0 = the RPDO path is being used).
  uint64_t GetSdoSetpointCount() const noexcept { return sdo_setpoint_count_.load(); }

  /// Enables or disables the SDO retry used when an RPDO Controlword has no effect.
  void SetSdoControlwordFallback(bool enabled) noexcept {
    sdo_controlword_fallback_.store(enabled);
  }

  /// Transport that last succeeded in changing the CiA 402 state.
  ControlwordPath GetControlwordPath() const noexcept {
    return controlword_path_.load();
  }
  /// Time of the last successful controlword transition.
  std::chrono::steady_clock::time_point LastControlwordTime() const noexcept {
    return controlword_time_;
  }

  /**
   * @brief Try the Controlword over RPDO, then over SDO.
   *
   * A drive that ignores CW=0x0006 (the mandatory Switch-on-disabled -> Ready to switch
   * on transition) is either mis-wired or never received the frame, so the two transports
   * are tried in turn and the one that worked is recorded. Exposed for experiments.
   */
  bool ApplyControlword(uint16_t controlword, CiA402State expect,
                        std::chrono::milliseconds timeout);

 protected:
  void OnBoot(lely::canopen::NmtState st, char es, const std::string& what) noexcept override;
  void OnState(lely::canopen::NmtState st) noexcept override;
  /**
   * Overridden only to observe the outcome. The base implementation performs the
   * concise-DCF SDO download, so it MUST still be invoked.
   */
  void OnConfig(::std::function<void(::std::error_code ec)> res) noexcept override;
  void OnRpdoWrite(uint16_t idx, uint8_t subidx) noexcept override;
  void OnEmcy(uint16_t eec, uint8_t er, uint8_t msef[5]) noexcept override;
  void OnHeartbeat(bool occurred) noexcept override;
  void OnCanError(lely::io::CanError error) noexcept override;

 private:
  /// A stage verdict produced inside the fiber and recorded by BringUp().
  struct StageVerdict {
    bool ok{false};
    std::string reason;
    std::string hint;
  };

  // --- Fiber-side stage implementations (must run on the driver strand) ---
  bool StageBootUp(const BringUpOptions& opt);
  bool StagePreOp(const BringUpOptions& opt);
  bool StageNmtStart(const BringUpOptions& opt);
  StageVerdict StageIdentity(const BringUpOptions& opt);
  bool StageFaultReset(const BringUpOptions& opt);
  bool EnableServoImpl(std::chrono::milliseconds timeout);
  bool DisableServoImpl(std::chrono::milliseconds timeout);
  bool SetModeImpl(CiA402Mode mode, std::chrono::milliseconds timeout);
  bool MoveToPositionImpl(int32_t target, std::chrono::milliseconds timeout);
  bool SetVelocityImpl(int32_t target, std::chrono::milliseconds settle);
  DriveSnapshot ReadDriveSnapshotImpl();
  bool SetDriveControlModeImpl(uint32_t p1_00_value);

  /// Polls the Statusword (TPDO first, SDO fallback).
  bool WaitForState(CiA402State target, std::chrono::milliseconds timeout);
  /// Reads 0x6041 over SDO and refreshes the cached Statusword.
  bool RefreshStatusword();
  /// SDO read returning @p fallback instead of throwing.
  template <typename T>
  T ReadOr(uint16_t idx, uint8_t subidx, T fallback);
  /// SDO write returning false on abort; @p why receives the error text.
  template <typename T>
  bool TryWrite(uint16_t idx, uint8_t subidx, T value, std::string* why = nullptr);
  /// Programs + verifies the PDO layout. Returns false and fills @p detail on error.
  bool ConfigureAndVerifyPdos(const PdoPlan& plan, std::string* detail);
  /// Writes the Controlword into the RPDO buffer without logging (internal use).
  void SendControlwordQuiet(uint16_t controlword);

  void RefreshStateFromStatusword(uint16_t sw) noexcept;
  /// "(telemetry fresh)" / "(telemetry stale)" suffix for log messages.
  std::string TelemetryStaleNote() const;

  uint8_t node_id_;
  std::string axis_name_;
  const char* axis_tag_;

  std::atomic<uint16_t> statusword_{0};
  std::atomic<CiA402State> state_{CiA402State::UNKNOWN};
  std::atomic<int32_t> actual_position_{0};
  std::atomic<int32_t> actual_velocity_{0};
  std::atomic<int32_t> following_error_{0};
  std::atomic<uint16_t> error_code_{0};
  std::atomic<uint32_t> dsp_alarm_{0};
  std::atomic<uint8_t> error_register_{0};
  std::atomic<int8_t> mode_display_{0};
  std::atomic<bool> target_reached_{false};
  std::atomic<bool> is_operational_{false};
  std::atomic<bool> boot_seen_{false};
  std::atomic<bool> config_done_{false};
  std::atomic<bool> emergency_seen_{false};
  std::atomic<uint16_t> last_emcy_code_{0};
  std::atomic<uint64_t> statusword_count_{0};
  std::atomic<uint64_t> statusword_sdo_reads_{0};
  std::atomic<bool> heartbeat_seen_{false};
  std::atomic<ControlwordPath> controlword_path_{ControlwordPath::kNone};
  std::atomic<bool> sdo_controlword_fallback_{true};
  std::atomic<uint64_t> sdo_setpoint_count_{0};
  std::atomic<bool> alarm_watch_running_{false};
  std::atomic<uint32_t> watched_alarm_{0xFFFFFFFFu};
  std::atomic<uint32_t> watched_error_reg_{0xFFFFFFFFu};
  std::atomic<bool> sdo_setpoints_{false};
  bool store_parameters_{false};

  std::mutex time_mutex_;
  std::chrono::steady_clock::time_point statusword_time_{};
  std::chrono::steady_clock::time_point emcy_time_{};
  std::chrono::steady_clock::time_point controlword_time_{};
};

}  // namespace mbdv