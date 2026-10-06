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
#include <functional>
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
  /**
   * @brief Profile acceleration/deceleration written to 0x6083/0x6084 in S08.
   *
   * A CiA 402 drive that reports Operation Enabled but never moves is, on this drive,
   * most often sitting on a zero acceleration ramp. Set both to 0 to leave whatever the
   * drive already has alone. The defaults mirror the bring-up that is known to work on
   * this hardware (counts/s^2).
   */
  uint32_t profile_accel{25000};
  uint32_t profile_decel{50000};
  /**
   * @brief Communication watchdog (0x2060, manual P1-39), applied in S08.
   *
   * -1 leave untouched, 0 disable (0x2060:01 = 0), >0 set the timeout in ms. A drive
   * configured for communication control expects periodic RPDO traffic and raises
   * EMERGENCY xxxx with error-register bit 4 (COMMUNICATION) once 0x2060:03 elapses -
   * measured at 525 ms with the 500 ms default, immediately after a setpoint was sent.
   */
  int32_t watchdog_timeout_ms{-1};
  /// 0x2060:05 timeout option code (Luna P1-40), written in S08 when >= 0. See
  /// Params::watchdog_action for why there is no default.
  int32_t watchdog_action{-1};
  /**
   * @brief CiA 301 consumer heartbeat period for this node, in ms (0 = off).
   *
   * Installed into the master's 0x1016 through lely's co_dev_cfg_hb(), so a node that
   * stops producing 0x700+node frames raises a heartbeat event and reaches
   * OnHeartbeat(false). Must be greater than producer_ms.
   */
  uint16_t heartbeat_consumer_ms{0};
  /**
   * @brief Producer heartbeat period written to the DRIVE's 0x1017, in ms (0 = leave it).
   *
   * The drive ships 0x1017 = 1000 ms, which takes over a second to notice a failure.
   * Shortening it is what makes the reconnect state machine react in ~300 ms.
   */
  uint16_t heartbeat_producer_ms{0};
  PdoPlan pdo{};
  std::chrono::milliseconds boot_timeout{3000};
  /**
   * @brief Run stage S11 (fault reset) at the end of BringUp().
   *
   * A reconnect turns it off: the drives may be held by the stop output's E-STOP circuit at
   * that point, a reset cannot succeed then, and RecoverServo() clears any fault right
   * before it re-enables the servo anyway.
   */
  bool fault_reset{true};
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

  /**
   * @brief Sends one RPDO to a single node, addressed by its COB-ID.
   *
   * @param node_id    target node; the COB-ID is derived from it.
   * @param pdo_no     1, 2 or 3 (0x200/0x300/0x400 + node).
   * @param controlword value for 0x6040, always present.
   * @param has_setpoint false for RPDO1, which carries no setpoint.
   * @param setpoint   target position (RPDO2) or velocity (RPDO3).
   *
   * Why this exists: a master's object dictionary holds ONE copy of 0x6040/0x607A/
   * 0x60FF, and co_dev_tpdo_event() transmits every PDO whose mapping contains the object
   * that just changed. With two nodes sharing one master that fires the setpoint PDO of
   * *both* nodes, so both drives receive the union of both commands - which for a
   * differential-drive robot means the axes cancel out and it never turns. Addressing the
   * frame by node is the only way to give each axis its own value.
   */
  using RpdoSender = std::function<bool(uint8_t node_id, uint8_t pdo_no, uint16_t controlword,
                                        bool has_setpoint, int32_t setpoint)>;

  /**
   * @brief Decodes one raw TPDO frame addressed to this axis' node.
   *
   * @param pdo_no 1, 2 or 3, from the COB-ID (0x180/0x280/0x380 + node).
   * @param data   frame payload.
   * @param len    payload length.
   *
   * Called from the can sniffer's thread once per frame. Layouts follow the master's
   * 0x1A00..0x1A02 mapping: TPDO1 = 0x6041 (u16); TPDO2 = 0x6064 (i32) + 0x606C (i32);
   * TPDO3 = 0x603F (u32) + 0x200F (u32).
   */
  void HandleRawTpdo(uint8_t pdo_no, const uint8_t* data, uint8_t len);

  /// True once a raw TPDO source is feeding this axis, so lely's OD callback is ignored.
  void SetRawFeedback(bool on) { raw_feedback_.store(on); }

  /**
   * @brief Forces target velocity 0 to this node, bypassing the change-detection cache.
   *
   * Must be called right after the servo is enabled. The drive keeps 0x60FF from whatever
   * ran before, so without this the axes move on their own: observed on this rig as both
   * drives travelling tens of thousands of counts while the program had not sent a single
   * RPDO3. A zero setpoint is also a brake (CiA 402 Quick Stop ramp), so it is the right
   * first action after enable, not a formality.
   */
  void ZeroVelocityNow();

  /**
   * @brief Commands this axis to stand still, from any thread, without ever enabling it.
   *
   * Profile velocity: target velocity 0 on RPDO3, so the drive ramps down on 0x6084.
   * Profile position: the Halt bit (controlword bit 8) on RPDO1, kept set by
   * RefreshControlword() until the next position command.
   * The setpoint cache becomes 0 as well, so a velocity task already queued on the strand
   * cannot restart the axis afterwards (see SetTargetVelocity()). An axis in Switched On,
   * Ready to Switch On or Quick Stop Active gets no frame: controlword 0x000F would enable
   * or resume it.
   */
  void StopNow();

  /**
   * @brief Blocks until a reconnect may start, or @p timeout elapses. Not for the loop thread.
   *
   * On a heartbeat timeout lely itself sends NMT Reset Node to the slave (0x1F80 = 1, see
   * co_nmt_node_err_ind()). On the boot-up that follows it cancels every pending SDO request
   * for the node and runs its own 'boot slave' SDO sequence (BasicMaster::OnState()), so
   * bring-up traffic started before that finishes would be cancelled or would race it. This
   * waits for: heartbeat present, and no boot-slave process in progress.
   */
  bool WaitForReconnectReady(std::chrono::milliseconds timeout);

  /// Makes the waiting loops of the staged bring-up return early. Called before shutdown so
  /// a reconnect running on the supervisor's worker thread does not hold up Stop().
  void AbortPendingWork() noexcept { abort_.store(true); }

  /// Installs the per-node RPDO transport. Without one, transmit falls back to the
  /// shared lely object dictionary, which cannot address a single node.
  void SetRpdoSender(RpdoSender sender);

  /// Sends one RPDO to this axis' own node.
  bool SendRpdo(uint8_t pdo_no, uint16_t controlword, bool has_setpoint, int32_t setpoint);

  /// 0x6060 as RPDO1 must carry it; see SendRpdo(1, ...).
  int8_t CurrentModeForRpdo1();

  /**
   * @brief Declares which receive-PDO slot this axis occupies, i.e. its master's-local
   *        shadow indices.
   *
   * The master's object dictionary is shared by the whole bus, so the TPDO mapping points
   * at a per-slot copy of each object (see shadow_index() in tools/fix_master_dcf.py).
   * Slot order is node-major: slot = node_index * 3 + pdo_number. Without this, two nodes
   * share one copy of 0x6064/0x606C and each axis' odometry is a mixture of both.
   */
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

  /**
   * @brief Queues a profile-velocity setpoint at the control rate.
   *
   * Only 0x60FF is written, which makes lely transmit exactly one frame - RPDO3, which
   * also carries the Controlword value latched at servo enable. The Controlword appears
   * in all three RPDOs, so writing it every cycle would triple the frame count for no
   * benefit at 200 Hz; RefreshControlword() does that separately at a lower rate.
   */
  void SetTargetVelocity(int32_t target_velocity);

  /**
   * @brief Re-sends the Controlword, which also refreshes the drive's comm watchdog.
   *
   * Call this at the configured controlword rate, not the control rate.
   */
  void RefreshControlword();

  // ------------------------------------------------------------------
  // Liveness and recovery
  // ------------------------------------------------------------------

  /**
   * @brief True while the node's heartbeat is arriving.
   *
   * Driven by lely's heartbeat consumer, so it reflects real 0x700+node frames rather
   * than a software timer. Always true when heartbeat monitoring is disabled.
   */
  bool IsAlive() const noexcept {
    return !heartbeat_lost_.load();
  }

  /// True once lely has reported a heartbeat timeout, until frames resume.
  bool HeartbeatLost() const noexcept { return heartbeat_lost_.load(); }

  /// True when the kernel reported a CAN controller error since the last clear.
  bool CanLinkErrored() const noexcept { return can_error_.load(); }
  void ClearCanLinkError() noexcept { can_error_.store(false); }
  const std::string& LastCanErrorText() const noexcept { return last_can_error_; }

  /**
   * @brief Stage S11 + S12 in one call: clear a latched fault and enable the servo again.
   *
   * Used by the supervisor after a fault clears on its own (an operator releasing a
   * limit switch, or refitting the STO connector). Returns false when the cause is still
   * present, in which case nothing is left half-done. @p cause, when given, receives the
   * classification, so the caller can tell "only hardware can clear this" from a failure.
   */
  bool RecoverServo(const char* reason, std::chrono::milliseconds fault_timeout,
                    std::chrono::milliseconds servo_timeout, FaultKind* cause = nullptr);

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
  uint16_t GetErrorCode() const noexcept { return error_code_.load(); }
  uint32_t GetDspAlarmCode() const noexcept { return dsp_alarm_.load(); }
  uint8_t GetErrorRegister() const noexcept { return error_register_.load(); }

  bool IsTargetReached() const noexcept { return target_reached_.load(); }
  bool IsOperational() const noexcept { return is_operational_.load(); }
  bool HasFault() const noexcept { return state_.load() == CiA402State::FAULT; }

  bool SawBootUp() const noexcept { return boot_seen_.load(); }
  bool SawEmergency() const noexcept { return emergency_seen_.load(); }
  /// True when no Statusword has arrived for @p window (TPDO1 not flowing).
  bool IsTelemetryStale(std::chrono::milliseconds window) const;

  /// Number of received TPDO cycles, used to prove the feedback path works.
  /// Number of Statusword frames received via TPDO (excludes SDO refreshes).
  uint64_t GetStatuswordCount() const noexcept { return statusword_count_.load(); }
  /// Number of Statusword reads performed over SDO as a TPDO fallback.
  uint64_t GetStatuswordSdoReads() const noexcept { return statusword_sdo_reads_.load(); }
 protected:
  void OnBoot(lely::canopen::NmtState st, char es, const std::string& what) noexcept override;
  void OnState(lely::canopen::NmtState st) noexcept override;
  /**
   * Overridden only to observe the outcome. The base implementation performs the
   * concise-DCF SDO download, so it MUST still be invoked.
   */
  void OnConfig(::std::function<void(::std::error_code ec)> res) noexcept override;
  void OnRpdoWrite(uint16_t idx, uint8_t subidx) noexcept override;
  /**
   * @brief lely heartbeat consumer event for this node.
   *
   * @p occurred is true when a heartbeat timeout was raised and false when the frames
   * resumed. Both edges are recorded so the supervisor can tell "never came up" from
   * "went away and came back".
   */
  void OnHeartbeat(bool occurred) noexcept override;
  void OnEmcy(uint16_t eec, uint8_t er, uint8_t msef[5]) noexcept override;
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

  /**
   * @brief Writes @p controlword into RPDO1 and waits for @p expect.
   *
   * On failure 0x6040 is read back over SDO: a value that came back unchanged proves
   * the frame never reached the drive's object dictionary, which separates a transport
   * problem from a drive that received the command and refused the transition.
   */
  bool ApplyControlword(uint16_t controlword, CiA402State expect,
                        std::chrono::milliseconds timeout);

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
  /// Writes the drive's 0x1017 producer heartbeat period. Fibre-side, blocking.
  bool SetHeartbeatProducerImpl(uint16_t period_ms);
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
  std::atomic<bool> target_reached_{false};
  std::atomic<bool> is_operational_{false};
  std::atomic<bool> boot_seen_{false};
  std::atomic<bool> emergency_seen_{false};
  std::atomic<bool> heartbeat_lost_{false};
  std::atomic<uint32_t> heartbeat_events_{0};
  std::atomic<bool> can_error_{false};
  std::atomic<uint16_t> heartbeat_consumer_ms_{0};
  std::atomic<uint16_t> last_emcy_code_{0};
  std::atomic<uint64_t> statusword_count_{0};
  /// Per-node RPDO transport; see SetRpdoSender().
  RpdoSender rpdo_sender_{};
  /// 0x6060 as last written, echoed in every RPDO1. -1 until SetModeImpl() has run.
  std::atomic<int8_t> mode_of_operation_{-1};
  /// Set when feedback comes from the sniffer rather than lely's object dictionary.
  std::atomic<bool> raw_feedback_{false};
  std::atomic<uint64_t> statusword_sdo_reads_{0};
  std::atomic<bool> alarm_watch_running_{false};
  std::atomic<uint32_t> watched_alarm_{0xFFFFFFFFu};
  std::atomic<uint32_t> watched_error_reg_{0xFFFFFFFFu};
  std::atomic<int32_t> last_target_velocity_{0x7FFFFFFF};
  /// Set by StopNow() in profile position; RefreshControlword() keeps the Halt bit until
  /// the next position command clears it.
  std::atomic<bool> halted_{false};
  /// Between a boot-up frame (OnState(BOOTUP)) and the end of lely's boot-slave process
  /// (OnBoot()); see WaitForReconnectReady().
  std::atomic<bool> boot_in_progress_{false};
  /// See AbortPendingWork().
  std::atomic<bool> abort_{false};

  std::string last_can_error_;
  std::chrono::steady_clock::time_point last_nmt_time_{};

  std::mutex time_mutex_;
  std::chrono::steady_clock::time_point statusword_time_{};
  std::chrono::steady_clock::time_point emcy_time_{};
  std::chrono::steady_clock::time_point boot_start_time_{};
};

}  // namespace mbdv