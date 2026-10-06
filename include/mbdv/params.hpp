#pragma once

/**
 * @file params.hpp
 * @brief Runtime parameters, loaded from config/params.yaml.
 *
 * Every tunable that used to be hard-coded or only reachable through a command-line flag
 * lives here instead, so an integrator can retune the machine without rebuilding:
 *
 *   - the CAN interface and the node-IDs of the two axes;
 *   - the control and feedback rate (default 200 Hz) that the odometry loop runs at and
 *     the TPDO event timers that feed it;
 *   - the CiA 301 heartbeat periods used for liveness detection;
 *   - the reconnect policy for a lost node and for a dead CAN link;
 *   - the differential-drive geometry and velocity limits;
 *   - where and how odometry is published (in-process callback and/or UDP).
 *
 * A missing file is not an error: Params falls back to the defaults below, which are the
 * values this rig was characterised with. That keeps `--selftest` working on a machine
 * where only the DCF was deployed. Malformed values ARE an error, because silently
 * running at 20 Hz because a key was misspelled would be far worse than refusing to start.
 */

#include "mbdv/cia402_defs.hpp"
#include "mbdv/diff_drive_kinematics.hpp"
#include "mbdv/drive_errors.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace mbdv {

/// Everything the master reads from config/params.yaml.
struct Params {
  // ---------------------------------------------------------------- bus
  std::string can_interface{"can0"};
  /// Bit rate in bit/s the drives are expected to run at (compared against 0x2021 in S08).
  uint32_t expect_bitrate_bps{500000};

  // ---------------------------------------------------------------- nodes
  uint8_t axis1_node_id{1};
  uint8_t axis2_node_id{2};

  // ---------------------------------------------------------------- rates
  /**
   * @brief Control and feedback rate in Hz.
   *
   * Drives the teleop loop period, the odometry integration period and the odometry
   * publication rate. 200 Hz means a 5 ms period. Anything above roughly 400 Hz is not
   * useful: the 6 TPDOs alone would then exceed what the bus carries comfortably.
   */
  double control_rate_hz{200.0};

  /**
   * @brief Rate at which the Controlword is re-sent, in Hz.
   *
   * The target setpoint goes out every control period, but the Controlword only has to
   * be refreshed often enough to satisfy the drive's communication watchdog (0x2060) and
   * to keep CiA 402's "Enable Operation" edge latched. Keeping this low is what makes
   * 200 Hz affordable: the controlword appears in all three RPDOs, so lely transmits one
   * frame per RPDO whenever it is written.
   */
  double controlword_rate_hz{20.0};

  /// TPDO event timers in ms. 5 ms == 200 Hz of encoder feedback.
  uint16_t tpdo1_event_timer_ms{5};
  uint16_t tpdo2_event_timer_ms{5};
  uint16_t tpdo3_event_timer_ms{50};

  // ---------------------------------------------------------------- heartbeat
  /**
   * @brief Producer heartbeat period written to the drive's 0x1017, in ms.
   *
   * The heartbeat is the signal for a lost CAN connection, so its period sets how fast a
   * cut cable stops the robot. The drive ships 1000 ms (EDS 0x1017 = 0x3e8), which would
   * take 1.5 s or more to notice; 100 ms costs 10 frames/s per axis, well under 1% of a
   * 500 kbps bus. 0 skips the write and leaves whatever the drive already has.
   */
  uint16_t heartbeat_producer_ms{100};

  /**
   * @brief Consumer period lely monitors for, in ms (0x1016 in the master's OD).
   *
   * Must be larger than the producer period; 3x leaves room for two late frames. This single
   * number IS the loss-detection time: lely raises a heartbeat event when no frame
   * arrives within the window, and that event is what turns the stop output on.
   * There is deliberately no separate "timeout" setting, because a second deadline that
   * nothing enforces would be worse than none.
   */
  uint16_t heartbeat_consumer_ms{300};

  // ---------------------------------------------------------------- liveness
  /**
   * @brief Minimum time the stop output stays asserted, in ms.
   *
   * The stop output (DualAxisController::StopRequested()) is meant to drive a hardware IO,
   * e.g. the drives' E-STOP inputs. A fault that leaves the CAN link intact could otherwise
   * clear it on the very next control cycle, a pulse too short for a relay to act on.
   */
  uint32_t stop_hold_ms{500};

  // ---------------------------------------------------------------- reconnect
  bool reconnect_enabled{true};
  /// Backoff between reconnect attempts, in ms. Grows up to reconnect_backoff_max_ms.
  uint32_t reconnect_backoff_ms{500};
  uint32_t reconnect_backoff_max_ms{5000};
  /// 0 = keep trying forever, which is what an unattended machine needs.
  uint32_t reconnect_max_attempts{0};
  /// Re-open the SocketCAN interface when the kernel reports a controller error.
  bool recover_can_link{true};

  // ---------------------------------------------------------------- fault recovery
  /// Attempt a fault reset + re-enable automatically when a drive faults.
  bool auto_recover_faults{true};
  uint32_t fault_retry_backoff_ms{1000};
  uint32_t fault_max_attempts{5};

  // ---------------------------------------------------------------- drive mode
  /// Expected P1-00 in 0x2A30. 0 = skip the check. See DriveControlMode.
  uint32_t expect_control_mode{static_cast<uint32_t>(DriveControlMode::kVelocityControl)};
  /// Written to 0x2A30 during S08 when non-zero. Not persisted to 0x1010.
  uint32_t write_control_mode{0};
  /// Profile acceleration/deceleration written to 0x6083/0x6084 during bring-up
  /// (counts/s^2). 0 = leave whatever the drive already has.
  uint32_t profile_accel{25000};
  uint32_t profile_decel{50000};
  /// Communication watchdog 0x2060. -1 untouched, 0 disabled, >0 timeout in ms.
  int32_t watchdog_timeout_ms{-1};
  /**
   * @brief 0x2060:05 "timeout option code" (Luna P1-40, "action after the watchdog is
   *        triggered"). -1 = leave untouched.
   *
   * Once the CAN link is gone the master cannot stop a drive any more, and a bus-watchdog
   * trip on its own is a Warning that "does not change the current state" (hardware manual
   * 9.1), so the axes keep their last setpoint unless this selects a stopping action. The
   * hardware manual gives the range (1..16, default 1) but not the meaning of each value;
   * that table is in the MBDV CANopen manual, so there is no safe value to write blindly.
   */
  int32_t watchdog_action{-1};
  /// Program the PDO records at S09 (false = verify only, for diagnosis).
  bool program_pdos{true};

  // ---------------------------------------------------------------- kinematics
  KinematicsConfig kinematics{};

  // ---------------------------------------------------------------- odometry output
  /// Publish odometry at control_rate_hz. Off keeps the loop purely local.
  bool publish_odometry{true};
  /// Invoke a callback registered with SetOdometryCallback() on every sample.
  bool publish_callback{true};
  /// Also send each sample as a compact binary datagram over UDP.
  bool publish_udp{false};
  std::string udp_host{"239.255.0.10"};
  uint16_t udp_port{5565};
  /// Resend the latest sample this many times when no new data arrives, so a subscriber
  /// that joins late still gets a steady stream. 1 = only publish fresh samples.
  int udp_repeat{1};

  // ---------------------------------------------------------------- misc
  std::string dcf_path{"config/master.dcf"};
  std::string bin_path;  ///< empty -> the DCF names its own download file
  std::chrono::milliseconds boot_timeout{3000};
  std::chrono::milliseconds servo_timeout{2000};
};

/**
 * @brief Loads @p path into @p params.
 *
 * @param path    YAML file; resolved like the DCF path (see config_path.hpp) when the
 *                value as given does not exist.
 * @param params  filled with the file's values, or the defaults when the file is absent.
 * @param error   receives a human-readable reason when the file exists but is unusable.
 * @param used_path if non-null, receives the path that was actually read ("" when none).
 * @return true on success, or when the file does not exist. False only on a malformed file.
 */
bool LoadParams(const std::string& path, Params* params, std::string* error,
                std::string* used_path = nullptr);

/// Serialises @p params back to YAML, so a running configuration can be captured.
std::string DumpParams(const Params& params);

/// One published odometry sample. Wire format for UDP is DumpOdometrySample()'s.
struct OdometrySample {
  double stamp_sec{0.0};  ///< seconds since the process started, from a steady clock
  double x{0.0};          ///< [m]
  double y{0.0};          ///< [m]
  double theta{0.0};      ///< [rad]
  double linear_v{0.0};   ///< [m/s]
  double angular_w{0.0};  ///< [rad/s]
  int32_t left_counts{0};   ///< 0x6064 of axis 1
  int32_t right_counts{0};  ///< 0x6064 of axis 2
  bool alive{false};        ///< both axes alive AND both feeding fresh Statuswords
};

/// Number of bytes an OdometrySample occupies in a UDP datagram.
constexpr std::size_t kOdometrySampleWireSize = 64;

/// Packs @p s into @p out (kOdometrySampleWireSize bytes, little endian, network order
/// doubles). Returns false when @p out is too small.
bool SerializeOdometrySample(const OdometrySample& s, unsigned char* out, std::size_t out_size);

/// Inverse of SerializeOdometrySample(). Returns false on a short buffer.
bool DeserializeOdometrySample(const unsigned char* in, std::size_t in_size, OdometrySample* s);

}  // namespace mbdv
