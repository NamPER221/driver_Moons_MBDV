#pragma once

#include <cstdint>
#include <string>

namespace mbdv {

/**
 * @brief CiA 402 Operation Modes (Object 0x6060)
 */
enum class CiA402Mode : int8_t {
  NO_MODE = 0,
  PROFILE_POSITION = 1,       // PP mode
  PROFILE_VELOCITY = 3,       // PV mode
  PROFILE_TORQUE = 4,         // TQ mode
  HOMING = 6,                 // HM mode
  INTERPOLATED_POSITION = 7,  // IP mode
  CYCLIC_SYNC_POSITION = 8,   // CSP mode
  CYCLIC_SYNC_VELOCITY = 9,   // CSV mode
  CYCLIC_SYNC_TORQUE = 10     // CST mode
};

inline const char* cia402_mode_to_string(CiA402Mode mode) noexcept {
  switch (mode) {
    case CiA402Mode::NO_MODE:
      return "No Mode";
    case CiA402Mode::PROFILE_POSITION:
      return "Profile Position (PP)";
    case CiA402Mode::PROFILE_VELOCITY:
      return "Profile Velocity (PV)";
    case CiA402Mode::PROFILE_TORQUE:
      return "Profile Torque (TQ)";
    case CiA402Mode::HOMING:
      return "Homing (HM)";
    case CiA402Mode::INTERPOLATED_POSITION:
      return "Interpolated Position (IP)";
    case CiA402Mode::CYCLIC_SYNC_POSITION:
      return "Cyclic Synchronous Position (CSP)";
    case CiA402Mode::CYCLIC_SYNC_VELOCITY:
      return "Cyclic Synchronous Velocity (CSV)";
    case CiA402Mode::CYCLIC_SYNC_TORQUE:
      return "Cyclic Synchronous Torque (CST)";
    default:
      return "Unknown Mode";
  }
}

/**
 * @brief CiA 402 Controlword bit definitions (Object 0x6040)
 */
namespace controlword_bits {
constexpr uint16_t SWITCH_ON = 1 << 0;
constexpr uint16_t ENABLE_VOLTAGE = 1 << 1;
constexpr uint16_t QUICK_STOP = 1 << 2;
constexpr uint16_t ENABLE_OPERATION = 1 << 3;
constexpr uint16_t NEW_SET_POINT = 1 << 4;
constexpr uint16_t CHANGE_SET_IMMEDIATELY = 1 << 5;
constexpr uint16_t ABS_REL = 1 << 6;
constexpr uint16_t FAULT_RESET = 1 << 7;
constexpr uint16_t HALT = 1 << 8;
}  // namespace controlword_bits

/**
 * @brief CiA 402 Standard Controlword Command Sequences
 */
namespace controlword_commands {
constexpr uint16_t SHUTDOWN = 0x0006;          // Transitions to Ready to Switch ON
constexpr uint16_t SWITCH_ON = 0x0007;         // Transitions to Switched ON
constexpr uint16_t ENABLE_OPERATION = 0x000F;  // Transitions to Operation Enabled (Servo ON)
constexpr uint16_t DISABLE_OPERATION = 0x0007;
constexpr uint16_t DISABLE_VOLTAGE = 0x0000;
constexpr uint16_t QUICK_STOP = 0x0002;
constexpr uint16_t FAULT_RESET = 0x0080;       // Rising edge resets drive fault
}  // namespace controlword_commands

/**
 * @brief CiA 402 Statusword bit definitions (Object 0x6041)
 */
namespace statusword_bits {
constexpr uint16_t READY_TO_SWITCH_ON = 1 << 0;
constexpr uint16_t SWITCHED_ON = 1 << 1;
constexpr uint16_t OPERATION_ENABLED = 1 << 2;
constexpr uint16_t FAULT = 1 << 3;
constexpr uint16_t VOLTAGE_ENABLED = 1 << 4;
constexpr uint16_t QUICK_STOP = 1 << 5;
constexpr uint16_t SWITCH_ON_DISABLED = 1 << 6;
constexpr uint16_t WARNING = 1 << 7;
constexpr uint16_t REMOTE = 1 << 9;
constexpr uint16_t TARGET_REACHED = 1 << 10;
constexpr uint16_t INTERNAL_LIMIT_ACTIVE = 1 << 11;
constexpr uint16_t SET_POINT_ACKNOWLEDGE = 1 << 12;
constexpr uint16_t FOLLOWING_ERROR = 1 << 13;
}  // namespace statusword_bits

/**
 * @brief CiA 402 Drive State Machine States
 */
enum class CiA402State {
  NOT_READY_TO_SWITCH_ON,
  SWITCH_ON_DISABLED,
  READY_TO_SWITCH_ON,
  SWITCHED_ON,
  OPERATION_ENABLED,
  QUICK_STOP_ACTIVE,
  FAULT_REACTION_ACTIVE,
  FAULT,
  UNKNOWN
};

inline CiA402State decode_cia402_state(uint16_t sw) noexcept {
  // Check Fault Reaction Active (xxxx xxxx x0xx 1111)
  if ((sw & 0x004F) == 0x000F) {
    return CiA402State::FAULT_REACTION_ACTIVE;
  }
  // Check Fault (xxxx xxxx x0xx 1000)
  if ((sw & 0x004F) == 0x0008) {
    return CiA402State::FAULT;
  }
  // Check Quick Stop Active (xxxx xxxx x00x 0111)
  if ((sw & 0x006F) == 0x0007) {
    return CiA402State::QUICK_STOP_ACTIVE;
  }
  // Check Operation Enabled (xxxx xxxx x01x 0111)
  if ((sw & 0x006F) == 0x0027) {
    return CiA402State::OPERATION_ENABLED;
  }
  // Check Switched ON (xxxx xxxx x01x 0011)
  if ((sw & 0x006F) == 0x0023) {
    return CiA402State::SWITCHED_ON;
  }
  // Check Ready to Switch ON (xxxx xxxx x01x 0001)
  if ((sw & 0x006F) == 0x0021) {
    return CiA402State::READY_TO_SWITCH_ON;
  }
  // Check Switch ON Disabled (xxxx xxxx x1xx 0000)
  if ((sw & 0x004F) == 0x0040) {
    return CiA402State::SWITCH_ON_DISABLED;
  }
  // Check Not Ready to Switch ON (xxxx xxxx x0xx 0000)
  if ((sw & 0x004F) == 0x0000) {
    return CiA402State::NOT_READY_TO_SWITCH_ON;
  }
  return CiA402State::UNKNOWN;
}

inline const char* cia402_state_to_string(CiA402State state) noexcept {
  switch (state) {
    case CiA402State::NOT_READY_TO_SWITCH_ON:
      return "Not Ready to Switch ON";
    case CiA402State::SWITCH_ON_DISABLED:
      return "Switch ON Disabled";
    case CiA402State::READY_TO_SWITCH_ON:
      return "Ready to Switch ON";
    case CiA402State::SWITCHED_ON:
      return "Switched ON";
    case CiA402State::OPERATION_ENABLED:
      return "Operation Enabled (Servo ON)";
    case CiA402State::QUICK_STOP_ACTIVE:
      return "Quick Stop Active";
    case CiA402State::FAULT_REACTION_ACTIVE:
      return "Fault Reaction Active";
    case CiA402State::FAULT:
      return "Fault";
    case CiA402State::UNKNOWN:
    default:
      return "Unknown State";
  }
}

}  // namespace mbdv
