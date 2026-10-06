#include "mbdv/drive_errors.hpp"

#include "mbdv/cia402_defs.hpp"
#include "mbdv/diagnostics.hpp"

#include <sstream>

namespace mbdv {
namespace {

/// Joins non-empty fragments with ", ".
std::string JoinFlags(const std::vector<std::string>& flags) {
  std::string out;
  for (const std::string& flag : flags) {
    if (!out.empty()) out += ", ";
    out += flag;
  }
  return out.empty() ? std::string("none (all clear)") : out;
}

}  // namespace

// ---------------------------------------------------------------------------
// CAN bit rate
// ---------------------------------------------------------------------------

const char* can_bit_rate_code_to_string(uint16_t code) noexcept {
  switch (code) {
    case 0: return "1 Mbps (P1-18 CB code 0)";
    case 1: return "800 kbps (P1-18 CB code 1)";
    case 2: return "500 kbps (P1-18 CB code 2)";
    case 3: return "250 kbps (P1-18 CB code 3)";
    case 4: return "125 kbps (P1-18 CB code 4)";
    case 5: return "50 kbps (P1-18 CB code 5)";
    case 6: return "20 kbps (P1-18 CB code 6)";
    case 7: return "12.5 kbps (P1-18 CB code 7)";
    default: return "unknown P1-18 CB code";
  }
}

uint32_t can_bit_rate_code_to_bps(uint16_t code) noexcept {
  switch (code) {
    case 0: return 1000000u;
    case 1: return 800000u;
    case 2: return 500000u;
    case 3: return 250000u;
    case 4: return 125000u;
    case 5: return 50000u;
    case 6: return 20000u;
    case 7: return 12500u;
    default: return 0u;
  }
}

const char* can_bit_rate_bps_to_string(uint32_t bps) noexcept {
  switch (bps) {
    case 1000000u: return "1 Mbps";
    case 800000u: return "800 kbps";
    case 500000u: return "500 kbps";
    case 250000u: return "250 kbps";
    case 125000u: return "125 kbps";
    case 50000u: return "50 kbps";
    case 20000u: return "20 kbps";
    case 12500u: return "12.5 kbps";
    default: return "non-standard bit rate";
  }
}

std::string DecodeBitRateObject(uint16_t raw) {
  if (raw == 0) return "0x2021 = 0 (drive reports no bit rate)";

  // Interpretation A (observed on hardware): the value is the speed in kbps.
  if (raw >= 12u && raw <= 1000u) {
    const uint32_t bps = static_cast<uint32_t>(raw) * 1000u;
    return Str("0x2021 = ", static_cast<int>(raw), " kbps (", bps, " bit/s, ",
               can_bit_rate_bps_to_string(bps), ")");
  }
  // Interpretation B: the value is the P1-18 CB code.
  if (raw <= 7u) {
    return Str("0x2021 = ", static_cast<int>(raw), " -> ", can_bit_rate_code_to_string(raw),
               " = ", can_bit_rate_code_to_bps(raw), " bit/s");
  }
  return Str("0x2021 = ", static_cast<int>(raw), " (neither a kbps figure nor a P1-18 CB code)");
}

const char* drive_control_mode_to_string(uint32_t value) noexcept {
  switch (value) {
    case 1: return "1 = Torque Control";
    case 15: return "15 = Velocity Control (internal 8-speed)";
    case 21: return "21 = Position Control (communication point-to-point)";
    default: return "not a documented P1-00 value";
  }
}

const char* node_id_to_string(uint16_t node_id) noexcept {
  switch (node_id) {
    case 1: return "1";
    case 2: return "2";
    case 3: return "3";
    case 4: return "4";
    case 5: return "5";
    case 6: return "6";
    case 7: return "7";
    default: return "outside the DIP-switch range 1..7";
  }
}

// ---------------------------------------------------------------------------
// Statusword / Controlword
// ---------------------------------------------------------------------------

std::string DecodeStatusword(uint16_t statusword) {
  std::vector<std::string> bits;
  auto add = [&bits](bool set, const char* name) {
    if (set) bits.emplace_back(name);
  };

  add(statusword & statusword_bits::READY_TO_SWITCH_ON, "READY_TO_SWITCH_ON(b0)");
  add(statusword & statusword_bits::SWITCHED_ON, "SWITCHED_ON(b1)");
  add(statusword & statusword_bits::OPERATION_ENABLED, "OPERATION_ENABLED(b2)");
  add(statusword & statusword_bits::FAULT, "FAULT(b3)");
  add(statusword & statusword_bits::VOLTAGE_ENABLED, "VOLTAGE_ENABLED(b4)");
  add(statusword & statusword_bits::QUICK_STOP, "QUICK_STOP(b5)");
  add(statusword & statusword_bits::SWITCH_ON_DISABLED, "SWITCH_ON_DISABLED(b6)");
  add(statusword & statusword_bits::WARNING, "WARNING(b7)");
  add(statusword & statusword_bits::REMOTE, "REMOTE(b9)");
  add(statusword & statusword_bits::TARGET_REACHED, "TARGET_REACHED(b10)");
  add(statusword & statusword_bits::INTERNAL_LIMIT_ACTIVE, "INTERNAL_LIMIT_ACTIVE(b11)");
  add(statusword & statusword_bits::SET_POINT_ACKNOWLEDGE, "SETPOINT_ACK(b12)");
  add(statusword & statusword_bits::FOLLOWING_ERROR, "FOLLOWING_ERROR(b13)");

  return Str(Hex(statusword, 4), " [", cia402_state_to_string(decode_cia402_state(statusword)),
             "] ", JoinFlags(bits));
}

std::string DecodeControlword(uint16_t controlword) {
  std::vector<std::string> bits;
  auto add = [&bits](bool set, const char* name) {
    if (set) bits.emplace_back(name);
  };

  add(controlword & controlword_bits::SWITCH_ON, "SWITCH_ON(b0)");
  add(controlword & controlword_bits::ENABLE_VOLTAGE, "ENABLE_VOLTAGE(b1)");
  add(controlword & controlword_bits::QUICK_STOP, "QUICK_STOP(b2)");
  add(controlword & controlword_bits::ENABLE_OPERATION, "ENABLE_OPERATION(b3)");
  add(controlword & controlword_bits::NEW_SET_POINT, "NEW_SETPOINT(b4)");
  add(controlword & controlword_bits::CHANGE_SET_IMMEDIATELY, "CHANGE_SET_IMMEDIATELY(b5)");
  add(controlword & controlword_bits::ABS_REL, "RELATIVE(b6)");
  add(controlword & controlword_bits::FAULT_RESET, "FAULT_RESET(b7)");
  add(controlword & controlword_bits::HALT, "HALT(b8)");

  return Str(Hex(controlword, 4), " ", JoinFlags(bits));
}

// ---------------------------------------------------------------------------
// CiA 402 error code 0x603F
// ---------------------------------------------------------------------------

std::string DecodeErrorCode402(uint16_t error_code) {
  switch (error_code) {
    case 0x0000: return "0x0000 no error";
    case 0x0001: return "0x0001 generic error";
    case 0x0002: return "0x0002 current limit";
    case 0x0003: return "0x0003 drive overvoltage";
    case 0x0004: return "0x0004 drive undervoltage";
    case 0x0005: return "0x0005 drive over temperature";
    case 0x0006: return "0x0006 motor over temperature";
    case 0x0007: return "0x0007 over current (drive/motor)";
    case 0x0008: return "0x0008 over speed";
    case 0x0009: return "0x0009 following error (position overshoot, P3-04 exceeded)";
    case 0x000A: return "0x000A ALM - external alarm input active";
    case 0x000B: return "0x000B positive side internal limit switch";
    case 0x000C: return "0x000C negative side internal limit switch";
    case 0x000D: return "0x000D positive side external limit switch";
    case 0x000E: return "0x000E negative side external limit switch";
    case 0x000F: return "0x000F encoder fault";
    case 0x0010: return "0x0010 encoder data error (software side)";
    case 0x0011: return "0x0011 encoder hardware error";
    case 0x0012: return "0x0012 encoder data error";
    case 0x0013: return "0x0013 encoder hardware error";
    case 0x0014: return "0x0014 position controller error";
    case 0x0015: return "0x0015 brake error / brake released unexpectedly";
    case 0x0016: return "0x0016 slave drive link loss";
    case 0x0017: return "0x0017 internal voltage error";
    case 0x0018: return "0x0018 internal fault";
    case 0x0019: return "0x0019 runaway / over speed on load";
    case 0x001A: return "0x001A reserved";
    case 0x001B: return "0x001B reserved";
    case 0x001C: return "0x001C reserved";
    case 0x001D: return "0x001D reserved";
    case 0x001E: return "0x001E reserved";
    case 0x001F: return "0x001F manufacturer specific error";
    case 0x0020: return "0x0020 CANopen communication error (bad sequence / bad frame)";
    case 0x0021: return "0x0021 reserved";
    case 0x0022: return "0x0022 sum error over multiple modules";
    case 0x0023: return "0x0023 external power supply error";
    default: break;
  }
  if (error_code >= 0xFF00 && error_code <= 0xFF32) {
    return Str(Hex(error_code, 4), " device-profile specific error");
  }
  return Str(Hex(error_code, 4), " undocumented CiA 402 error code - consult the drive LED");
}

// ---------------------------------------------------------------------------
// CiA 301 error register 0x1001
// ---------------------------------------------------------------------------

std::string DecodeErrorRegister(uint8_t error_register) {
  std::vector<std::string> flags;
  auto add = [&flags](bool set, const char* name) {
    if (set) flags.emplace_back(name);
  };

  add(error_register & 0x01, "GENERIC(b0)");
  add(error_register & 0x02, "CURRENT(b1)");
  add(error_register & 0x04, "VOLTAGE(b2)");
  add(error_register & 0x08, "TEMPERATURE(b3)");
  add(error_register & 0x10, "COMMUNICATION(b4)");
  add(error_register & 0x20, "DEVICE_PROFILE_SPECIFIC(b5)");
  add(error_register & 0x40, "RESERVED(b6)");
  add(error_register & 0x80, "MANUFACTURER(b7)");

  return Str(Hex(error_register, 2), " ", JoinFlags(flags));
}

// ---------------------------------------------------------------------------
// Moons' DSP status / alarm codes
// ---------------------------------------------------------------------------

std::string DecodeDspStatusCode(uint32_t dsp_status) {
  // The AMA stack reports the DSP state in the low byte; the upper bytes carry
  // the DSP register image. Only the low byte is stable across firmware, so
  // decode the documented states and stay explicit about the rest.
  const uint32_t low = dsp_status & 0xFFu;
  switch (low) {
    case 0x00: return Str(Hex(dsp_status, 8), " DSP idle / no motion");
    case 0x01: return Str(Hex(dsp_status, 8), " DSP positioning");
    case 0x02: return Str(Hex(dsp_status, 8), " DSP velocity control");
    case 0x03: return Str(Hex(dsp_status, 8), " DSP torque control");
    default: break;
  }
  return Str(Hex(dsp_status, 8), " DSP status raw value (not decoded; no EDS mapping)");
}

std::string DecodeDspAlarmCode(uint32_t dsp_alarm) {
  if (dsp_alarm == 0) return "0x200F = 0 -> no drive alarm (the LED shows the node number)";

  // CONFIRMED on hardware: the low byte of 0x200F is the code the MBDV flashes on its
  // 2-digit LED, shown in HEX (manual appendix 1 lists the LED character table as 0-9, A-F).
  // A drive reporting 0x200F = 0x...20 flashed "20"; the codes 40 and 11 read on the LED
  // correspond to 0x40 and 0x11 of this same object.
  const uint32_t code = dsp_alarm & 0xFFu;
  return Str("0x200F alarm code = ", Hex(code, 2),
             " -> the drive LED flashes \"", Hex(code, 2),
             "\" (hex digits, as on the LED)   [raw ", Hex(dsp_alarm, 8), "]");
}

std::string DecodeSubAlarmCode(uint32_t sub_alarm) {
  if (sub_alarm == 0) return "0x2AC0 = 0 (no sub-alarm)";
  // CORRECTION: an earlier revision of this decoder called 0x2AC0 a plain mask that is not an
  // alarm indicator. That was wrong - it tracks the active alarm: 0x04000000 while healthy,
  // 0x02010000 / 0x00010000 at the moment an EMERGENCY was raised. It is the drive's
  // sub-alarm code and is only meaningful together with 0x200F and 0x1001.
  return Str("0x2AC0 sub-alarm code = ", Hex(sub_alarm, 8),
             " - read it together with 0x200F and the 0x1001 error register; the value "
             "changes when an alarm is active");
}

bool DcBusOutOfSpec(uint16_t raw) {
  if (raw == 0) return true;
  return raw < kDcBusSpecMinTenths || raw > kDcBusSpecMaxTenths;
}

bool MainVoltagePresent(uint16_t statusword) {
  return (statusword & 0x0010u) != 0u;  // CiA 402 Statusword bit 4 Voltage_enabled
}

// ---------------------------------------------------------------------------
// DC bus voltage (object 0x2030)
// ---------------------------------------------------------------------------

std::string DecodeDcBusVoltage(uint16_t raw) {
  if (raw == 0) return "0x2030 = 0 (bus reads zero)";

  // Measured on hardware: the object is in 0.1 V units (a drive on the 24 V auxiliary
  // rail reads 241 -> 24.1 V). The EDS does not document the unit, so the alternative
  // mV reading is shown alongside. Manual section 4.3 gives the main input as
  // 24 ~ 60 VDC.
  std::ostringstream os;
  os << "0x2030 = " << raw << " raw -> " << (static_cast<double>(raw) / 10.0)
     << " V at 0.1 V/unit (measured convention)";
  if (raw < 1000) {
    os << " [" << static_cast<double>(kDcBusSpecMinTenths) / 10.0 << ".."
       << static_cast<double>(kDcBusSpecMaxTenths) / 10.0 << " VDC spec range (manual 4.3)]";
  }
  os << "; " << raw << " mV at 1 mV/unit (alternative reading)";
  return os.str();
}

// ---------------------------------------------------------------------------
// DIP switches (object 0x2070, manual section 4.2.2)
// ---------------------------------------------------------------------------

DipSwitchDecode DecodeDipSwitch(uint32_t switch_value) {
  DipSwitchDecode out;
  out.node1_raw = static_cast<uint8_t>((switch_value >> 0) & 0x07u);
  out.node2_raw = static_cast<uint8_t>((switch_value >> 3) & 0x07u);
  out.baud_from_dip = ((switch_value >> 6) & 0x01u) != 0u;
  out.term_resistor = ((switch_value >> 7) & 0x01u) != 0u;
  return out;
}

std::string DescribeDipSwitch(const DipSwitchDecode& dip) {
  std::ostringstream os;
  os << "SW1..SW3=" << static_cast<int>(dip.node1_raw) << " SW4..SW6="
     << static_cast<int>(dip.node2_raw);
  // 0x2070 is one bitmap for the whole drive, so both nodes report the same value. A zero
  // address field is valid: that axis takes its node-ID from Luna, and 0x2020 stays the
  // authoritative node-ID.
  if (dip.node1_raw == 0 && dip.node2_raw == 0) {
    os << " (both node-IDs come from the Luna software setting, not the DIP)";
  } else if (dip.node1_raw == 0 || dip.node2_raw == 0) {
    os << " (axis " << (dip.node1_raw == 0 ? 1 : 2)
       << " node-ID comes from the Luna software setting, not the DIP)";
  }
  // SW7 = 0 means "bit rate is set by the Luna software (P1-18)", whose default happens to
  // be 1 Mbps - it does NOT mean the drive is running at 1 Mbps. Object 0x2021 is the
  // authoritative measured speed and is checked separately.
  os << "; SW7=" << (dip.baud_from_dip ? "ON -> 500 kbps forced by the DIP"
                                      : "OFF -> bit rate from Luna software (P1-18)");
  os << "; SW8=" << (dip.term_resistor ? "ON -> 120 ohm fitted" : "OFF -> no terminator");
  return os.str();
}

// ---------------------------------------------------------------------------
// Digital input diagnosis (manual section 7.1)
// ---------------------------------------------------------------------------

namespace {

struct InputFunctionEntry {
  uint32_t value;
  const char* symbol;
  const char* name;
  bool blocks_servo;
};

// Manual section 7.1.1.1. Odd value = "valid when Closed", even = "valid when Open".
constexpr InputFunctionEntry kInputFunctions[] = {
    {0, "GPIN", "General Purpose Input", false},
    {1, "S-ON", "Servo On", false},          // enabling is the goal, not a blocker
    {3, "A-CLR", "Alarm Reset", false},
    {5, "CW-LMT", "CW Limit", true},
    {7, "CCW-LMT", "CCW Limit", true},
    {11, "GAIN-SEL", "Gain Select", false},
    {13, "E-STOP", "Emergency Stop", true},
    {15, "S-HOM", "Start Homing", false},
    {19, "TQ-LMT", "Torque Limit", false},
    {21, "ZCLAMP", "Zero Speed Clamp", false},
    {37, "V-LMT", "Speed Limit Select", false},
    {39, "HOM-SW", "Home Switch", false},
    {45, "START-Q", "Start Q Program", false},
};

const InputFunctionEntry* LookupInputFunction(uint32_t value) {
  // Strip the logic bit: 5/6 are both CW-LMT, 7/8 both CCW-LMT, and so on.
  const uint32_t base = value & ~1u;
  for (const InputFunctionEntry& entry : kInputFunctions) {
    if (entry.value == base || entry.value == value) return &entry;
  }
  return nullptr;
}

}  // namespace

std::string DecodeInputFunction(uint32_t setup_value) {
  const InputFunctionEntry* entry = LookupInputFunction(setup_value);
  std::ostringstream os;
  if (!entry) {
    os << setup_value << " (not a documented setup value)";
    return os.str();
  }
  os << entry->value << " = " << entry->name << " (" << entry->symbol << ")";
  if (setup_value != 0) {
    os << ", valid when " << ((setup_value & 1u) ? "CLOSED" : "OPEN");
  }
  return os.str();
}

bool InputFunctionBlocksServo(uint32_t setup_value) {
  const InputFunctionEntry* entry = LookupInputFunction(setup_value);
  return entry != nullptr && entry->blocks_servo;
}

std::string DescribeDigitalInputs(const DigitalInputState& inputs) {
  std::ostringstream os;
  os << "0x60FD digital inputs = " << Hex(inputs.raw, 8)
     << "   (bit N assumed to be input X(N+1); the EDS does not publish the bit mapping)\n";

  bool any_blocker = false;
  for (int i = 0; i < 4; ++i) {
    const bool active = (inputs.raw >> i) & 0x01u;
    const uint32_t setup_value = inputs.function[i];
    const InputFunctionEntry* entry = LookupInputFunction(setup_value);

    os << "        X" << (i + 1) << " (P5-0" << i << ", " << ObjRef(0x2A20, static_cast<uint8_t>(i + 1))
       << ") = " << DecodeInputFunction(setup_value) << "  ->  "
       << (active ? "ACTIVE (input circuit closed)" : "inactive (input circuit open)");

    if (active && entry && entry->blocks_servo) {
      any_blocker = true;
      os << "   <== BLOCKING: " << entry->name
         << " is asserted, so the drive will refuse to leave Switch On Disabled";
    }
    os << '\n';
  }

  if (any_blocker) {
    os << "        Reminder (manual 7.1): 'Closed' means the input's optocoupler circuit is "
          "completed.\n"
          "        Factory defaults are X1 = CCW-LMT (7), X2 = CW-LMT (5), X3 = HOM-SW (39), "
          "X4 = E-STOP (13).\n"
          "        Leave the CW/CCW limit and E-STOP inputs open, or reassign them to GPIN "
          "(0) via P5-00..P5-03,\n"
          "        or wire the physical switches so they read open while the drive is meant to "
          "run.\n";
  } else {
    os << "        No input currently asserts a function that blocks servo enable. If the "
          "drive still\n"
          "        will not leave Switch On Disabled, suspect the STO circuit (manual 4.11) or "
          "a setting in P1-02.\n";
  }
  return os.str();
}

std::string DescribeCommWatchdog(uint32_t enable, uint32_t status, uint32_t timeout_ms,
                                uint32_t trigger) {
  std::ostringstream os;
  os << "0x2060 communication watchdog (manual P1-39): "
     << ObjRef(od::kCommWatchdog, od::kCommWatchdogEnable) << " enable=" << enable << " ("
     << (enable ? "ACTIVE" : "disabled") << "), "
     << ObjRef(od::kCommWatchdog, od::kCommWatchdogStatus) << " status=" << Hex(status, 2)
     << " (" << (status ? "HAS TRIGGERED" : "not triggered") << "), "
     << ObjRef(od::kCommWatchdog, od::kCommWatchdogTimeout) << " timeout=" << timeout_ms << " ms, "
     << ObjRef(od::kCommWatchdog, od::kCommWatchdogTrigger) << " trigger=0x" << std::hex
     << trigger;
  if (enable && status) {
    os << "\n        -> the drive raised EMERGENCY xxxx (error register bit 4 COMMUNICATION) "
          "because no RPDO arrived within " << timeout_ms
       << " ms. This is why motion stops after a while when setpoints go over SDO.";
  }
  return os.str();
}

const char* fault_kind_to_string(FaultKind kind) noexcept {
  switch (kind) {
    case FaultKind::kNone:          return "none";
    case FaultKind::kSto:           return "STO engaged";
    case FaultKind::kLimit:         return "limit/E-STOP input asserted";
    case FaultKind::kNoMainPower:   return "no main power on V+/V-";
    case FaultKind::kEncoder:       return "encoder feedback";
    case FaultKind::kOverload:      return "overload / over-current";
    case FaultKind::kPositionError: return "excessive following error";
    case FaultKind::kCommunication: return "CAN communication error";
    case FaultKind::kUnknown:       return "unclassified";
  }
  return "unclassified";
}

std::string ExplainFaultKind(FaultKind kind) {
  switch (kind) {
    case FaultKind::kNone:
      return "no fault";
    case FaultKind::kSto:
      return
          "STO is engaged. Manual 4.11 calls it \"a hardware level safety function\": it is "
          "armed by the SF1/SF2 inputs on connector CN5 going OPEN, and no CANopen object "
          "can release it. Manual 4.11.1(7) states the alarm clears by itself once STO is "
          "deactivated, so this program will recover on its own as soon as the safety "
          "circuit is closed again - an operator has to refit the connector or close the "
          "safety relay. Do NOT try to work around this in software.";
    case FaultKind::kLimit:
      return
          "a digital input assigned to a blocking function (CW-LMT, CCW-LMT or E-STOP) is "
          "asserted. Check 0x60FD and 0x2A20:01..04 for which one. Release the switch, or "
          "reassign the input to GPIN (0) via P5-00..P5-03 (manual 7.1.1.2).";
    case FaultKind::kNoMainPower:
      return
          "Statusword bit 4 (Voltage_enabled, CiA 402) is clear, so the drive sees nothing "
          "on V+/V-. Manual 4.3 requires 24..60 VDC there; the 24 VDC auxiliary supply on "
          "24V/GND alone powers the logic but will not enable a servo.";
    case FaultKind::kEncoder:
      return
          "encoder feedback is missing or implausible. Compare 0x6064 against the "
          "commanded 0x607A, and check the encoder cable - manual 6.2 flags code r09 for "
          "encoder wiring.";
    case FaultKind::kOverload:
      return
          "an overload or over-current class alarm is latched. Decode 0x603F and 0x200F "
          "above; check the load, the duty cycle and the torque limit P1-06.";
    case FaultKind::kPositionError:
      return
          "excessive following error. 0x60F4 is the position error at the instant of the "
          "fault; check for a mechanical obstruction, a stall, and the position error limit "
          "P3-04.";
    case FaultKind::kCommunication:
      return
          "the drive raised an EMERGENCY with error-register bit 4 (COMMUNICATION) set, "
          "which on this drive is the communication watchdog 0x2060 firing roughly 500 ms "
          "after the last RPDO. It was measured disabled, so look for a gap in the setpoint "
          "stream.";
    case FaultKind::kUnknown:
      return
          "the fault does not match any documented pattern. Decode 0x603F, 0x1001 and the "
          "manufacturer 0x200F, and read the 2-digit code on the front LED against manual "
          "appendix 1.";
  }
  return "unclassified";
}

FaultKind ClassifyFault(const DriveSnapshot& s) {
  // The CiA 301 error register is the only normative source for the communication bit.
  if (s.error_register & 0x10u) return FaultKind::kCommunication;

  // Statusword bit 4 is the authoritative "is the drive powered" signal; a drive with no
  // main voltage can never leave Switch On Disabled regardless of anything else.
  if (!MainVoltagePresent(s.statusword)) return FaultKind::kNoMainPower;

  // An asserted input whose assigned function blocks servo enable explains a refusal
  // better than any alarm code, and it is the one cause an operator can clear.
  for (int i = 0; i < 4; ++i) {
    const bool active = (s.inputs.raw >> i) & 0x01u;
    if (active && InputFunctionBlocksServo(s.inputs.function[i])) {
      return FaultKind::kLimit;
    }
  }

  // 0x200F is the manufacturer's alarm; only its low byte is confirmed to be the code the
  // front LED flashes, so the comparisons below are on that byte and are deliberately
  // conservative. Codes outside this list fall through to kUnknown rather than being
  // guessed at, because the EDS publishes no code-to-name table.
  const uint32_t led = s.dsp_alarm & 0xFFu;
  switch (led) {
    case 0x09:  // manual 6.2: encoder cable / wiring
      return FaultKind::kEncoder;
    case 0x0A:
    case 0x0B:
    case 0x0C:
      return FaultKind::kEncoder;
    default:
      break;
  }

  // 0x603F overload / over-current family (CiA 402 standard error codes).
  switch (s.error_code) {
    case 0x2311:  // continuous over current
    case 0x2312:  // short circuit / earth leakage
    case 0x2313:  // over current at output
    case 0x3210:  // DC link over-voltage
    case 0x3220:  // DC link under-voltage
      return FaultKind::kOverload;
    case 0x8611:  // following error
      return FaultKind::kPositionError;
    default:
      break;
  }

  // A fault with no main voltage indication, no blocking input and no recognised code is
  // most often the STO circuit on this drive family, because STO raises an alarm the
  // vendor does not describe in the EDS. Reported as kUnknown unless the operator
  // confirms it, so the log never claims a hardware safety state it cannot see.
  if (s.dsp_alarm != 0 || s.error_code != 0) return FaultKind::kUnknown;
  return FaultKind::kNone;
}

std::string FormatDriveSnapshot(const DriveSnapshot& snapshot) {
  std::ostringstream os;
  os << "\n      ---- drive object dictionary snapshot ----\n"
     << "      0x6041 Statusword          : " << DecodeStatusword(snapshot.statusword) << '\n'
     << "      0x603F Error code         : " << DecodeErrorCode402(snapshot.error_code) << '\n'
     << "      0x1001 Error register     : " << DecodeErrorRegister(snapshot.error_register)
     << '\n'
     << "      0x200F DSP alarm code     : " << DecodeDspAlarmCode(snapshot.dsp_alarm) << '\n'
     << "      0x200B DSP status code    : " << DecodeDspStatusCode(snapshot.dsp_status) << '\n'
     << "      0x2AC0 Sub-alarm code    : " << DecodeSubAlarmCode(snapshot.sub_alarm) << '\n'
     << "      0x2030 DC bus             : " << DecodeDcBusVoltage(snapshot.dc_bus_raw) << '\n'
     << "      0x6078 Actual current     : " << snapshot.current_actual << '\n'
     << "      0x60F4 Following error    : " << snapshot.following_error << " counts\n";
  if (snapshot.motion_limits_valid) {
    os << "      ---- motion limits (a zero here blocks every target position) ----\n"
       << "      0x607F Max profile speed : " << snapshot.max_profile_speed << '\n'
       << "      0x6083 Profile accel     : " << snapshot.profile_accel << '\n'
       << "      0x6084 Profile decel     : " << snapshot.profile_decel << '\n'
       << "      0x607D:01 Position limit : " << snapshot.position_limit_min << '\n'
       << "      0x607D:02 Position limit : " << snapshot.position_limit_max << '\n';
    if (snapshot.max_profile_speed == 0) {
      os << "      0x607F is ZERO: this drive cannot execute a profile move at any"
            " setpoint. Raise it in Luna (P2-xx) before trusting any motion test.\n";
    }
    if (snapshot.position_limit_min != 0 && snapshot.position_limit_max != 0 &&
        (snapshot.position_limit_min > snapshot.position_limit_max ||
         0 < snapshot.position_limit_min ||
         0 > snapshot.position_limit_max)) {
      os << "      0x607D window [" << snapshot.position_limit_min << " .. "
         << snapshot.position_limit_max << "] excludes 0: the encoder's current origin"
            " sits outside the software position limits.\n";
    }
  }
  os
     << "      main voltage (SW b4)      : "
     << (MainVoltagePresent(snapshot.statusword)
             ? "PRESENT (Statusword bit 4 Voltage_enabled = 1) -> the drive is powered"
             : "ABSENT (Statusword bit 4 = 0) -> check the main supply on V+/V-")
     << "\n      ---- digital inputs (manual 7.1) ----\n"
     << DescribeDigitalInputs(snapshot.inputs) << std::endl;
  if (!MainVoltagePresent(snapshot.statusword)) {
    os << "      *** The drive reports no main voltage. Manual section 4.3: V+/V- takes "
          "24 ~ 60 VDC; the 24V/GND auxiliary supply alone does not enable the servo. ***\n";
  } else if (DcBusOutOfSpec(snapshot.dc_bus_raw)) {
    os << "      *** Statusword says the voltage is present, but 0x2030 is outside the "
          "24 ~ 60 VDC range of manual section 4.3. Check the supply under load. ***\n";
  }
  return os.str();
}

}  // namespace mbdv