#pragma once

/**
 * @file drive_errors.hpp
 * @brief Decoders for the diagnostic objects exposed by the Moons' MBDV EDS.
 *
 * All object indices and data types below were taken from
 * docx/CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds. Alarm names and the DIP-switch /
 * bus-parameter semantics were taken from
 * docx/MBDV-Hardware-Manual-EN20230926-MOONS.pdf sections 4.2.2, 5.2, 6 and 9.1.
 */

#include <cstdint>
#include <string>

namespace mbdv {

// ---------------------------------------------------------------------------
// Object dictionary indices used for diagnostics (EDS source of truth)
// ---------------------------------------------------------------------------

namespace od {
// --- CiA 301 communication profile ---
constexpr uint16_t kDeviceType = 0x1000;         ///< UNSIGNED32
constexpr uint16_t kErrorRegister = 0x1001;      ///< UNSIGNED8
constexpr uint16_t kManufacturerName = 0x1008;   ///< VISIBLE_STRING
constexpr uint16_t kHardwareVersion = 0x1009;    ///< VISIBLE_STRING
constexpr uint16_t kSoftwareVersion = 0x100A;    ///< VISIBLE_STRING
constexpr uint16_t kIdentity = 0x1018;           ///< Identity (ro)

// --- CiA 402 drive profile ---
constexpr uint16_t kErrorCode = 0x603F;          ///< UNSIGNED16, PDO-mappable
constexpr uint16_t kControlword = 0x6040;        ///< UNSIGNED16
constexpr uint16_t kStatusword = 0x6041;         ///< UNSIGNED16, PDO-mappable
constexpr uint16_t kModeOfOperation = 0x6060;    ///< INTEGER8
constexpr uint16_t kModeOfOperationDisplay = 0x6061;  ///< INTEGER8
constexpr uint16_t kPositionActual = 0x6064;     ///< INTEGER32
constexpr uint16_t kVelocityActual = 0x606C;     ///< INTEGER32
constexpr uint16_t kTargetPosition = 0x607A;     ///< INTEGER32
constexpr uint16_t kFollowingError = 0x60F4;     ///< INTEGER32
constexpr uint16_t kCurrentActual = 0x6078;      ///< INTEGER16
constexpr uint16_t kDigitalInputs = 0x60FD;       ///< UNSIGNED32 (ro), PDO-mappable
constexpr uint16_t kTargetVelocity = 0x60FF;     ///< INTEGER32
constexpr uint16_t kSupportedDriveModes = 0x6502;  ///< UNSIGNED32
constexpr uint16_t kInputConfig = 0x2A20;        ///< ARRAY, sub 1..10 = P5-00.. (rw)


// --- Moons' / AMA manufacturer objects ---
constexpr uint16_t kHomeSwitch = 0x2001;         ///< UNSIGNED8  (ro)
constexpr uint16_t kOutputStatus = 0x2002;       ///< UNSIGNED32 (ro)
constexpr uint16_t kDspClearAlarm = 0x2006;      ///< UNSIGNED8  (wo)
constexpr uint16_t kDspStatusCode = 0x200B;      ///< UNSIGNED32 (ro)
constexpr uint16_t kSetZeroPosition = 0x200C;    ///< UNSIGNED8  (wo)
constexpr uint16_t kDspAlarmCode = 0x200F;       ///< UNSIGNED32 (ro)  <- drive alarm
constexpr uint16_t kDeviceTemperature = 0x2019;  ///< ARRAY
constexpr uint16_t kNodeIdObject = 0x2020;       ///< UNSIGNED16 (ro)
constexpr uint16_t kBitRateObject = 0x2021;      ///< UNSIGNED16 (ro)
constexpr uint16_t kDcBusVoltage = 0x2030;       ///< UNSIGNED16 (ro)  [mV]
constexpr uint16_t kDspVersion = 0x2031;         ///< VISIBLE_STRING
constexpr uint16_t kCommWatchdog = 0x2060;       ///< ARRAY (manual P1-39 watchdog)
constexpr uint8_t kCommWatchdogEnable = 1;       ///< rww, EDS default 0 (disabled)
constexpr uint8_t kCommWatchdogStatus = 2;       ///< ro, non-zero once it has triggered
constexpr uint8_t kCommWatchdogTimeout = 3;      ///< rw, EDS default 0x1F4 = 500 ms
constexpr uint8_t kCommWatchdogTrigger = 4;      ///< rw, bitmask of triggering events
constexpr uint16_t kSwitchValue = 0x2070;        ///< UNSIGNED32 (ro)  <- DIP switches
constexpr uint16_t kControlMode = 0x2A30;        ///< UNSIGNED32 (rw)  <- P1-00 CM
constexpr uint16_t kControlModePowerUp = 0x2A31; ///< UNSIGNED32 (rw)  <- P1-02 PM
constexpr uint16_t kOperationMode = 0x2A32;      ///< UNSIGNED32 (ro)
constexpr uint16_t kAlarmMask = 0x2A37;          ///< UNSIGNED32 (rw)  <- P1-24
constexpr uint16_t kStepsPerRev = 0x2A90;        ///< UNSIGNED32 (rw)  <- P3-05
constexpr uint16_t kSubAlarmCode = 0x2AC0;       ///< UNSIGNED32 (rw)
}  // namespace od

// ---------------------------------------------------------------------------
// CAN bus bit rate
// ---------------------------------------------------------------------------

/**
 * @brief P1-18 CB enumeration (CANopen "CB" parameter) - manual section 8.3.2.
 */
enum class CanBitRateCode : uint16_t {
  k1Mbps = 0,
  k800kbps = 1,
  k500kbps = 2,
  k250kbps = 3,
  k125kbps = 4,
  k50kbps = 5,
  k20kbps = 6,
  k12_5kbps = 7,
};

const char* can_bit_rate_code_to_string(uint16_t code) noexcept;
uint32_t can_bit_rate_code_to_bps(uint16_t code) noexcept;

/// Nearest standard CAN bit rate, for turning a measured figure into a name.
const char* can_bit_rate_bps_to_string(uint32_t bps) noexcept;

/**
 * @brief Decodes object 0x2021 "Bit rate".
 *
 * Measured on an MBDV-2X-520AC: this object returns the bus speed in **kbps**
 * (a 500 kbps drive reads 500), NOT the P1-18 CB code, which would read 2 for the
 * same speed. Both interpretations are therefore reported so either firmware
 * convention is readable.
 */
std::string DecodeBitRateObject(uint16_t raw);

// ---------------------------------------------------------------------------
// Drive control-mode enumeration (P1-00 CM / manual section 8.3.2)
// ---------------------------------------------------------------------------

enum class DriveControlMode : uint32_t {
  kTorqueControl = 1,   ///< communication command torque
  kVelocityControl = 15,  ///< internal 8-speed via P2-10..P2-17
  kPositionControl = 21,  ///< communication point-to-point position (factory default)
};

const char* drive_control_mode_to_string(uint32_t value) noexcept;

// ---------------------------------------------------------------------------
// Decoders
// ---------------------------------------------------------------------------

/// Full bit-by-bit decode of a Statusword (0x6041), e.g. "READY(0) SWITCHED(1) ..."
std::string DecodeStatusword(uint16_t statusword);

/// Decode of the significant bits of a Controlword (0x6040).
std::string DecodeControlword(uint16_t controlword);

/// CiA 402 standard error code (0x603F). Returns a descriptive string.
std::string DecodeErrorCode402(uint16_t error_code);

/// CiA 301 error register (0x1001) bit flags.
std::string DecodeErrorRegister(uint8_t error_register);

/// DSP status code (0x200B) from the AMA stack.
std::string DecodeDspStatusCode(uint32_t dsp_status);

/// DSP alarm code (0x200F). Returns the LED-style code (e.g. "r09") plus a
/// best-effort description. The MBDV drive prints these same codes on its
/// 2-digit LED as "rNN" - cross-check against the drive display.
std::string DecodeDspAlarmCode(uint32_t dsp_alarm);

/// Sub alarm code (0x2AC0).
std::string DecodeSubAlarmCode(uint32_t sub_alarm);

/// Raw DIP switch bitmap read back from 0x2070, decoded per manual section 4.2.2.
///
/// The EDS only names the object "Switch value" (UNSIGNED32, ro) and does not define the
/// bit layout, so the field assignment below is this project's documented assumption
/// (bit0..2 = SW1..SW3, bit3..5 = SW4..SW6, bit6 = SW7, bit7 = SW8, 1 = switch ON).
/// Verified indirectly: an MBDV pair whose node-IDs come from the Luna software reads
/// 0x00000000 here, which is exactly what "all DIP switches OFF" means.
struct DipSwitchDecode {
  uint8_t node1_raw{0};      ///< SW1..SW3 as a 3-bit value; 0 = address set by Luna software
  uint8_t node2_raw{0};      ///< SW4..SW6 as a 3-bit value; 0 = address set by Luna software
  bool baud_from_dip{false}; ///< SW7: true = DIP forces 500 kbps, false = Luna software P1-18
  bool term_resistor{false}; ///< SW8: true = 120 ohm fitted
};
DipSwitchDecode DecodeDipSwitch(uint32_t switch_value);

/// One-line human summary of the DIP decode, phrased so it never overstates what the
/// object is known to mean (SW7 = 0 does NOT mean 1 Mbps, it means "from Luna software").
std::string DescribeDipSwitch(const DipSwitchDecode& dip);

/// Human-readable one-liner for a decoded node id (0x2020).
const char* node_id_to_string(uint16_t node_id) noexcept;

// ---------------------------------------------------------------------------
// Digital input diagnosis (manual section 7.1)
// ---------------------------------------------------------------------------

/**
 * @brief Decodes an input-function setup value (P5-00.., objects 0x2A20:01..0A).
 *
 * Manual section 7.1.1.1 lists the setup values; the odd/even pair encodes the active
 * logic (odd = valid when the input is Closed, even = valid when Open):
 *   0 GPIN | 1/2 S-ON | 3/4 A-CLR | 5/6 CW-LMT | 7/8 CCW-LMT | 11/12 GAIN-SEL
 *   13/14 E-STOP | 15/16 S-HOM | 19/20 TQ-LMT | 21/22 ZCLAMP | 37/38 V-LMT
 *   39/40 HOM-SW | 45/46 START-Q
 */
std::string DecodeInputFunction(uint32_t setup_value);

/// True when an input assigned this function can keep the servo from enabling.
bool InputFunctionBlocksServo(uint32_t setup_value);

/**
 * @brief Live state of the first four digital inputs.
 *
 * 0x60FD is documented only as "Ditigal inputs" (UNSIGNED32, ro) - the bit-to-terminal
 * mapping is not published, so the assumption bit N == input X(N+1) is stated in the
 * output rather than presented as fact.
 */
struct DigitalInputState {
  uint32_t raw{0};          ///< 0x60FD
  uint32_t function[4]{0};  ///< 0x2A20:01..04 (P5-00..P5-03)
};

/// Multi-line report naming each input, its assigned function and whether it is active.
std::string DescribeDigitalInputs(const DigitalInputState& inputs);

/// Interprets the communication watchdog (0x2060), manual parameter P1-39.
std::string DescribeCommWatchdog(uint32_t enable, uint32_t status, uint32_t timeout_ms,
                                 uint32_t trigger);

/// Renders a full drive diagnostic snapshot as a multi-line block.
struct DriveSnapshot {
  uint16_t statusword{0};
  uint16_t error_code{0};       ///< 0x603F
  uint8_t error_register{0};    ///< 0x1001
  uint32_t dsp_alarm{0};        ///< 0x200F
  uint32_t dsp_status{0};       ///< 0x200B
  uint32_t sub_alarm{0};        ///< 0x2AC0
  uint16_t dc_bus_raw{0};       ///< 0x2030 raw UNSIGNED16 (scale not in the EDS)
  int16_t current_actual{0};    ///< 0x6078
  int32_t following_error{0};   ///< 0x60F4
  DigitalInputState inputs{};  ///< 0x60FD + 0x2A20:01..04
  uint32_t watchdog_enable{0};    ///< 0x2060:01
  uint32_t watchdog_status{0};     ///< 0x2060:02, non-zero = has triggered
  uint32_t watchdog_timeout_ms{0}; ///< 0x2060:03
  uint32_t watchdog_trigger{0};    ///< 0x2060:04
};

/**
 * @brief DC-bus limits from manual section 4.3.
 *
 * Manual section 4.3 specifies the MBDV main power input as **24 ~ 60 VDC** on V+/V-
 * (DC, not AC), with a separate 24 VDC +/-10% auxiliary/control supply on 24V/GND.
 * An AUX-only drive therefore still reads roughly the 24 V auxiliary rail, which is
 * *inside* the specification range and must not be reported as a fault.
 *
 * Values are in the 0.1 V units that object 0x2030 was measured in.
 */
constexpr uint16_t kDcBusSpecMinTenths = 240;  ///< 24.0 VDC, manual 4.3 lower limit
constexpr uint16_t kDcBusSpecMaxTenths = 600;  ///< 60.0 VDC, manual 4.3 upper limit

/// Interprets 0x2030. The EDS does not document the unit; 0.1 V is what was measured.
std::string DecodeDcBusVoltage(uint16_t raw);

/// True when the interpreted DC bus lies outside the 24..60 VDC specification range.
bool DcBusOutOfSpec(uint16_t raw);

/// True when Statusword bit 4 (Voltage_enabled, CiA 402) reports main voltage present.
/// This, not the 0x2030 scale guess, is the authoritative "is the drive powered" signal.
bool MainVoltagePresent(uint16_t statusword);

/// Builds the diagnostic block printed when a stage fails.
std::string FormatDriveSnapshot(const DriveSnapshot& snapshot);

}  // namespace mbdv