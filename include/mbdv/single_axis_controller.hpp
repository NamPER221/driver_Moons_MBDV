#pragma once

#include "mbdv/mbdv_axis_driver.hpp"

#include <lely/ev/loop.hpp>
#include <lely/io2/linux/can.hpp>
#include <lely/io2/posix/poll.hpp>
#include <lely/io2/sys/io.hpp>
#include <lely/io2/sys/timer.hpp>
#include <lely/coapp/master.hpp>

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace mbdv {

/**
 * @brief Controller for single axis (Axis 1, Node ID 1) of Moons' MBDV Servo Drive.
 * Designed for 500 kbps baudrate CANopen network.
 */
class SingleAxisController {
 public:
  SingleAxisController();
  ~SingleAxisController();

  // Non-copyable, non-movable
  SingleAxisController(const SingleAxisController&) = delete;
  SingleAxisController& operator=(const SingleAxisController&) = delete;

  /**
   * @brief Initializes CAN channel, AsyncMaster, and Axis 1 Driver.
   * @param can_interface SocketCAN interface name (e.g. "can0").
   * @param dcf_path Path to master.dcf file (e.g. "config/single_axis_500k/master.dcf").
   * @param bin_path Optional path to master.bin file.
   * @param node_id CANopen Node ID for Axis 1 (default 1).
   */
  bool Initialize(const std::string& can_interface, const std::string& dcf_path,
                  const std::string& bin_path = "", uint8_t node_id = 1);

  /**
   * @brief Starts the CANopen master event loop in a dedicated thread.
   */
  void Start();

  /**
   * @brief Stops the master, disables servo, and cleans up resources.
   */
  void Stop();

  /**
   * @brief Commands Servo ON sequence (Shutdown -> Switch ON -> Enable Operation).
   * @return true if drive reached OPERATION_ENABLED.
   */
  bool EnableServo(std::chrono::milliseconds timeout = std::chrono::milliseconds(3000));

  /**
   * @brief Disables servo (Servo OFF).
   */
  bool DisableServo();

  /**
   * @brief Resets drive fault.
   */
  void ResetFault();

  /**
   * @brief Commands quick stop.
   */
  void QuickStop();

  /**
   * @brief Sets CiA 402 operation mode (e.g. Profile Position or Profile Velocity).
   */
  void SetMode(CiA402Mode mode);

  /**
   * @brief Commands position motion in Profile Position (PP) mode.
   * @param target_position Target position in encoder counts (Object 0x607A).
   * @param relative If true, position is relative to current position.
   */
  void MoveToPosition(int32_t target_position, bool relative = false);

  /**
   * @brief Commands target velocity in Profile Velocity (PV) mode.
   * @param target_velocity Target velocity in counts/s (Object 0x60FF).
   */
  void SetTargetVelocity(int32_t target_velocity);

  /**
   * @brief Prints real-time status telemetry of Axis 1.
   */
  void PrintTelemetry() const;

  // Accessors
  MbdvAxisDriver& GetAxis() { return *axis_; }
  const MbdvAxisDriver& GetAxis() const { return *axis_; }
  bool IsRunning() const noexcept { return is_running_.load(); }

 private:
  std::unique_ptr<lely::io::IoGuard> io_guard_;
  std::unique_ptr<lely::io::Context> ctx_;
  std::unique_ptr<lely::io::Poll> poll_;
  std::unique_ptr<lely::ev::Loop> loop_;
  std::unique_ptr<lely::io::Timer> timer_;
  std::unique_ptr<lely::io::CanController> ctrl_;
  std::unique_ptr<lely::io::CanChannel> chan_;
  std::unique_ptr<lely::canopen::AsyncMaster> master_;

  std::unique_ptr<MbdvAxisDriver> axis_;

  std::thread loop_thread_;
  std::atomic<bool> is_running_{false};
};

}  // namespace mbdv
