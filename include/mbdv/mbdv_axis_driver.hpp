#pragma once

#include "mbdv/cia402_defs.hpp"

#include <lely/coapp/fiber_driver.hpp>
#include <lely/coapp/master.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

namespace mbdv {

/**
 * @brief CiA 402 Servo Axis Driver using lely::canopen::FiberDriver.
 *
 * Implements real-time PDO reception (Statusword 0x6041, Actual Position 0x6064,
 * Actual Velocity 0x606C) and transmission (Controlword 0x6040, Mode 0x6060,
 * Target Position 0x607A, Target Velocity 0x60FF).
 */
class MbdvAxisDriver : public lely::canopen::FiberDriver {
 public:
  /**
   * @brief Constructs an axis driver for the specified CANopen node ID.
   * @param master CANopen AsyncMaster reference.
   * @param node_id Remote CANopen node ID (e.g. 1 for Axis 1, 2 for Axis 2).
   * @param axis_name Descriptive name for logging (e.g. "Axis_1").
   */
  MbdvAxisDriver(lely::canopen::AsyncMaster& master, uint8_t node_id,
                 std::string axis_name);

  ~MbdvAxisDriver() override = default;

  // Non-copyable, non-movable (managed by master/unique_ptr)
  MbdvAxisDriver(const MbdvAxisDriver&) = delete;
  MbdvAxisDriver& operator=(const MbdvAxisDriver&) = delete;

  // --- NMT & Initialization ---
  void RequestNmtStart();
  void RequestNmtPreOp();
  void RequestNmtReset();

  // --- CiA 402 Mode Configuration ---
  void SetModeOfOperation(CiA402Mode mode);

  // --- CiA 402 State Machine Control ---
  bool EnableServo(std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));
  bool DisableServo();
  void ResetFault();
  void QuickStop();

  // --- Real-time Motion Control (via RPDO) ---
  void SetTargetPosition(int32_t target_position, bool new_setpoint = true,
                         bool immediate = true, bool relative = false);
  void SetTargetVelocity(int32_t target_velocity);
  void SendControlword(uint16_t controlword);

  // --- Status & Telemetry Accessors (Thread-safe) ---
  uint8_t GetNodeId() const noexcept { return node_id_; }
  const std::string& GetAxisName() const noexcept { return axis_name_; }
  uint16_t GetStatusword() const noexcept { return statusword_.load(); }
  CiA402State GetCiA402State() const noexcept { return state_.load(); }
  int32_t GetActualPosition() const noexcept { return actual_position_.load(); }
  int32_t GetActualVelocity() const noexcept { return actual_velocity_.load(); }
  bool IsTargetReached() const noexcept { return target_reached_.load(); }
  bool IsOperational() const noexcept { return is_operational_.load(); }
  bool HasFault() const noexcept { return state_.load() == CiA402State::FAULT; }

 protected:
  // --- Lely Virtual Callbacks ---
  void OnBoot(lely::canopen::NmtState st, char es,
              const std::string& what) noexcept override;

  void OnState(lely::canopen::NmtState st) noexcept override;

  void OnRpdoWrite(uint16_t idx, uint8_t subidx) noexcept override;

  void OnEmcy(uint16_t eec, uint8_t er, uint8_t msef[5]) noexcept override;

 private:
  bool WaitForState(CiA402State target_state, std::chrono::milliseconds timeout);

  uint8_t node_id_;
  std::string axis_name_;

  std::atomic<uint16_t> statusword_{0};
  std::atomic<CiA402State> state_{CiA402State::UNKNOWN};
  std::atomic<int32_t> actual_position_{0};
  std::atomic<int32_t> actual_velocity_{0};
  std::atomic<bool> target_reached_{false};
  std::atomic<bool> is_operational_{false};
};

}  // namespace mbdv
