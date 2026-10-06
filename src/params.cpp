#include "mbdv/params.hpp"

#include "mbdv/config_path.hpp"

#include <yaml-cpp/yaml.h>

#include <cstring>
#include <fstream>
#include <sstream>

namespace mbdv {
namespace {

// yaml-cpp throws on a type mismatch. Reading through these helpers turns that into a
// message naming the offending key, which is the difference between "bad config" and
// "field does not exist" on a machine with no operator watching.
[[noreturn]] void Fail(const std::string& key, const std::string& detail) {
  throw std::runtime_error("params.yaml: '" + key + "' " + detail);
}

double GetNumber(const YAML::Node& parent, const char* key, double fallback) {
  const YAML::Node n = parent[key];
  if (!n) return fallback;
  if (n.IsScalar()) {
    try {
      return n.as<double>();
    } catch (const std::exception&) {
      Fail(key, "must be a number");
    }
  }
  Fail(key, "must be a scalar, not a list or map");
}

bool GetBool(const YAML::Node& parent, const char* key, bool fallback) {
  const YAML::Node n = parent[key];
  if (!n) return fallback;
  if (n.IsScalar()) {
    try {
      return n.as<bool>();
    } catch (const std::exception&) {
      Fail(key, "must be true or false");
    }
  }
  Fail(key, "must be true or false");
}

std::string GetString(const YAML::Node& parent, const char* key, const std::string& fallback) {
  const YAML::Node n = parent[key];
  if (!n) return fallback;
  if (n.IsScalar()) return n.as<std::string>();
  Fail(key, "must be text");
}

/// Reads an optional nested block, returning an undefined node when absent.
YAML::Node OptionalBlock(const YAML::Node& parent, const char* key) {
  const YAML::Node n = parent[key];
  if (!n) return YAML::Node{};
  if (!n.IsMap()) Fail(key, "must be a block of 'key: value' lines");
  return n;
}

/// Range-checks a rate and reports it in Hz, because every failure mode of this file is
/// ultimately "the loop is slower than the operator thinks it is".
double CheckRate(const char* key, double hz) {
  if (hz <= 0.0) Fail(key, "must be > 0");
  if (hz > 1000.0) Fail(key, "must be <= 1000 Hz; 6 TPDOs would already overrun the bus");
  return hz;
}

void PutIfSet(YAML::Emitter& out, const char* key, const std::string& v) {
  if (!v.empty()) out << YAML::Key << key << YAML::Value << v;
}
void PutIfSet(YAML::Emitter& out, const char* key, double v) {
  out << YAML::Key << key << YAML::Value << v;
}
void PutIfSet(YAML::Emitter& out, const char* key, bool v) {
  out << YAML::Key << key << YAML::Value << (v ? "true" : "false");
}

void WriteBlock(YAML::Emitter& out, const char* name) {
  out << YAML::Key << name << YAML::Value << YAML::BeginMap;
}

void WriteBlockEnd(YAML::Emitter& out) { out << YAML::EndMap; }

}  // namespace

bool LoadParams(const std::string& path, Params* p, std::string* error, std::string* used_path) {
  if (!p) return false;
  if (error) error->clear();
  if (used_path) used_path->clear();

  const std::string resolved = ResolveConfigPathOr(path);
  if (resolved.empty()) {
    // No file: the built-in defaults are a known-good configuration, so this is not fatal.
    return true;
  }
  if (used_path) *used_path = resolved;

  YAML::Node root;
  try {
    root = YAML::LoadFile(resolved);
  } catch (const std::exception& ex) {
    if (error) *error = "cannot parse: " + std::string(ex.what());
    return false;
  }
  if (!root.IsMap()) {
    if (error) *error = "top level must be a mapping of sections";
    return false;
  }

  try {
    if (YAML::Node bus = OptionalBlock(root, "bus")) {
      p->can_interface = GetString(bus, "interface", p->can_interface);
      p->expect_bitrate_bps =
          static_cast<uint32_t>(GetNumber(bus, "expect_bitrate_bps", p->expect_bitrate_bps));
      if (p->expect_bitrate_bps == 0) Fail("bus.expect_bitrate_bps", "must be > 0");
    }

    if (YAML::Node nodes = OptionalBlock(root, "nodes")) {
      const double n1 = GetNumber(nodes, "axis1_id", p->axis1_node_id);
      const double n2 = GetNumber(nodes, "axis2_id", p->axis2_node_id);
      if (n1 < 1 || n1 > 127) Fail("nodes.axis1_id", "must be 1..127");
      if (n2 < 1 || n2 > 127) Fail("nodes.axis2_id", "must be 1..127");
      p->axis1_node_id = static_cast<uint8_t>(n1);
      p->axis2_node_id = static_cast<uint8_t>(n2);
    }

    if (YAML::Node rates = OptionalBlock(root, "rates")) {
      p->control_rate_hz = CheckRate("rates.control_hz", GetNumber(rates, "control_hz", p->control_rate_hz));
      p->controlword_rate_hz = CheckRate("rates.controlword_hz",
                                         GetNumber(rates, "controlword_hz", p->controlword_rate_hz));
      if (p->controlword_rate_hz > p->control_rate_hz) {
        // Harmless, but it silently means "every cycle", which is usually a mistake.
        p->controlword_rate_hz = p->control_rate_hz;
      }
      const double hz = p->control_rate_hz;
      p->tpdo1_event_timer_ms = static_cast<uint16_t>(GetNumber(rates, "tpdo1_event_ms", p->tpdo1_event_timer_ms));
      p->tpdo2_event_timer_ms = static_cast<uint16_t>(GetNumber(rates, "tpdo2_event_ms", p->tpdo2_event_timer_ms));
      p->tpdo3_event_timer_ms = static_cast<uint16_t>(GetNumber(rates, "tpdo3_event_ms", p->tpdo3_event_timer_ms));
      if (p->tpdo1_event_timer_ms == 0 || p->tpdo2_event_timer_ms == 0) {
        Fail("rates.tpdo1_event_ms", "must be > 0; 0 would disable the feedback TPDO");
      }
      // Warn-worthy rather than fatal: a timer slower than the loop just means the
      // odometry integrates in coarser steps than it publishes.
      if (p->tpdo1_event_timer_ms * 2 > 1000.0 / hz) {
        // nothing to do here, kept as an explicit record of the relationship
      }
    }

    if (YAML::Node hb = OptionalBlock(root, "heartbeat")) {
      p->heartbeat_producer_ms = static_cast<uint16_t>(GetNumber(hb, "producer_ms", p->heartbeat_producer_ms));
      p->heartbeat_consumer_ms = static_cast<uint16_t>(GetNumber(hb, "consumer_ms", p->heartbeat_consumer_ms));
      if (p->heartbeat_consumer_ms && p->heartbeat_producer_ms &&
          p->heartbeat_consumer_ms <= p->heartbeat_producer_ms) {
        Fail("heartbeat.consumer_ms", "must be greater than heartbeat.producer_ms; it is "
                                      "the window in which a node is declared lost");
      }
    }

    if (YAML::Node lv = OptionalBlock(root, "liveness")) {
      const double hold = GetNumber(lv, "stop_hold_ms", p->stop_hold_ms);
      if (hold < 0) Fail("liveness.stop_hold_ms", "must be >= 0");
      p->stop_hold_ms = static_cast<uint32_t>(hold);
    }

    if (YAML::Node rc = OptionalBlock(root, "reconnect")) {
      p->reconnect_enabled = GetBool(rc, "enabled", p->reconnect_enabled);
      p->recover_can_link = GetBool(rc, "recover_can_link", p->recover_can_link);
      p->reconnect_backoff_ms = static_cast<uint32_t>(GetNumber(rc, "backoff_ms", p->reconnect_backoff_ms));
      p->reconnect_backoff_max_ms = static_cast<uint32_t>(GetNumber(rc, "backoff_max_ms", p->reconnect_backoff_max_ms));
      p->reconnect_max_attempts = static_cast<uint32_t>(GetNumber(rc, "max_attempts", p->reconnect_max_attempts));
      if (p->reconnect_backoff_max_ms < p->reconnect_backoff_ms) {
        Fail("reconnect.backoff_max_ms", "must be >= reconnect.backoff_ms");
      }
    }

    if (YAML::Node fr = OptionalBlock(root, "fault_recovery")) {
      p->auto_recover_faults = GetBool(fr, "auto_reset", p->auto_recover_faults);
      p->fault_retry_backoff_ms = static_cast<uint32_t>(GetNumber(fr, "retry_backoff_ms", p->fault_retry_backoff_ms));
      p->fault_max_attempts = static_cast<uint32_t>(GetNumber(fr, "max_attempts", p->fault_max_attempts));
    }

    if (YAML::Node drv = OptionalBlock(root, "drive")) {
      p->expect_control_mode = static_cast<uint32_t>(GetNumber(drv, "expect_p1_00", p->expect_control_mode));
      p->write_control_mode = static_cast<uint32_t>(GetNumber(drv, "write_p1_00", p->write_control_mode));
      p->profile_accel = static_cast<uint32_t>(GetNumber(drv, "profile_accel", p->profile_accel));
      p->profile_decel = static_cast<uint32_t>(GetNumber(drv, "profile_decel", p->profile_decel));
      p->watchdog_timeout_ms = static_cast<int32_t>(GetNumber(drv, "watchdog_ms", p->watchdog_timeout_ms));
      p->watchdog_action = static_cast<int32_t>(GetNumber(drv, "watchdog_action", p->watchdog_action));
      p->program_pdos = GetBool(drv, "program_pdos", p->program_pdos);
      if (p->watchdog_timeout_ms < -1) Fail("drive.watchdog_ms", "must be -1, 0 or > 0");
      if (p->watchdog_action < -1 || p->watchdog_action > 0xFFFF) {
        Fail("drive.watchdog_action", "must be -1 (leave untouched) or 0..65535");
      }
    }

    if (YAML::Node kin = OptionalBlock(root, "kinematics")) {
      KinematicsConfig& k = p->kinematics;
      k.wheel_radius = GetNumber(kin, "wheel_radius_m", k.wheel_radius);
      k.wheel_base = GetNumber(kin, "wheel_base_m", k.wheel_base);
      k.gear_ratio = GetNumber(kin, "gear_ratio", k.gear_ratio);
      k.encoder_cpr = static_cast<int32_t>(GetNumber(kin, "encoder_cpr", k.encoder_cpr));
      k.max_linear_velocity = GetNumber(kin, "max_linear_velocity", k.max_linear_velocity);
      k.max_angular_velocity = GetNumber(kin, "max_angular_velocity", k.max_angular_velocity);
      if (k.wheel_radius <= 0.0) Fail("kinematics.wheel_radius_m", "must be > 0");
      if (k.wheel_base <= 0.0) Fail("kinematics.wheel_base_m", "must be > 0");
      if (k.gear_ratio <= 0.0) Fail("kinematics.gear_ratio", "must be > 0");
      if (k.encoder_cpr <= 0) Fail("kinematics.encoder_cpr", "must be > 0");
      if (k.max_linear_velocity <= 0.0) Fail("kinematics.max_linear_velocity", "must be > 0");
      if (k.max_angular_velocity <= 0.0) Fail("kinematics.max_angular_velocity", "must be > 0");
    }

    if (YAML::Node od = OptionalBlock(root, "odometry")) {
      p->publish_odometry = GetBool(od, "enabled", p->publish_odometry);
      p->publish_callback = GetBool(od, "callback", p->publish_callback);
      p->publish_udp = GetBool(od, "udp", p->publish_udp);
      p->udp_host = GetString(od, "udp_host", p->udp_host);
      p->udp_port = static_cast<uint16_t>(GetNumber(od, "udp_port", p->udp_port));
      p->udp_repeat = static_cast<int>(GetNumber(od, "udp_repeat", p->udp_repeat));
      if (p->udp_repeat < 1) Fail("odometry.udp_repeat", "must be >= 1");
    }

    if (YAML::Node misc = OptionalBlock(root, "misc")) {
      p->dcf_path = GetString(misc, "dcf", p->dcf_path);
      p->bin_path = GetString(misc, "bin", p->bin_path);
      p->boot_timeout = std::chrono::milliseconds(
          static_cast<int64_t>(GetNumber(misc, "boot_timeout_ms", p->boot_timeout.count())));
      p->servo_timeout = std::chrono::milliseconds(
          static_cast<int64_t>(GetNumber(misc, "servo_timeout_ms", p->servo_timeout.count())));
      if (p->boot_timeout.count() <= 0) Fail("misc.boot_timeout_ms", "must be > 0");
      if (p->servo_timeout.count() <= 0) Fail("misc.servo_timeout_ms", "must be > 0");
    }
  } catch (const std::exception& ex) {
    if (error) *error = ex.what();
    return false;
  }

  return true;
}

std::string DumpParams(const Params& p) {
  YAML::Emitter out;
  out << YAML::BeginMap;

  WriteBlock(out, "bus");
  PutIfSet(out, "interface", p.can_interface);
  PutIfSet(out, "expect_bitrate_bps", static_cast<double>(p.expect_bitrate_bps));
  WriteBlockEnd(out);

  WriteBlock(out, "nodes");
  PutIfSet(out, "axis1_id", static_cast<double>(p.axis1_node_id));
  PutIfSet(out, "axis2_id", static_cast<double>(p.axis2_node_id));
  WriteBlockEnd(out);

  WriteBlock(out, "rates");
  PutIfSet(out, "control_hz", p.control_rate_hz);
  PutIfSet(out, "controlword_hz", p.controlword_rate_hz);
  PutIfSet(out, "tpdo1_event_ms", static_cast<double>(p.tpdo1_event_timer_ms));
  PutIfSet(out, "tpdo2_event_ms", static_cast<double>(p.tpdo2_event_timer_ms));
  PutIfSet(out, "tpdo3_event_ms", static_cast<double>(p.tpdo3_event_timer_ms));
  WriteBlockEnd(out);

  WriteBlock(out, "heartbeat");
  PutIfSet(out, "producer_ms", static_cast<double>(p.heartbeat_producer_ms));
  PutIfSet(out, "consumer_ms", static_cast<double>(p.heartbeat_consumer_ms));
  WriteBlockEnd(out);

  WriteBlock(out, "liveness");
  PutIfSet(out, "stop_hold_ms", static_cast<double>(p.stop_hold_ms));
  WriteBlockEnd(out);

  WriteBlock(out, "reconnect");
  PutIfSet(out, "enabled", p.reconnect_enabled);
  PutIfSet(out, "recover_can_link", p.recover_can_link);
  PutIfSet(out, "backoff_ms", static_cast<double>(p.reconnect_backoff_ms));
  PutIfSet(out, "backoff_max_ms", static_cast<double>(p.reconnect_backoff_max_ms));
  PutIfSet(out, "max_attempts", static_cast<double>(p.reconnect_max_attempts));
  WriteBlockEnd(out);

  WriteBlock(out, "fault_recovery");
  PutIfSet(out, "auto_reset", p.auto_recover_faults);
  PutIfSet(out, "retry_backoff_ms", static_cast<double>(p.fault_retry_backoff_ms));
  PutIfSet(out, "max_attempts", static_cast<double>(p.fault_max_attempts));
  WriteBlockEnd(out);

  WriteBlock(out, "drive");
  PutIfSet(out, "expect_p1_00", static_cast<double>(p.expect_control_mode));
  PutIfSet(out, "write_p1_00", static_cast<double>(p.write_control_mode));
  PutIfSet(out, "profile_accel", static_cast<double>(p.profile_accel));
  PutIfSet(out, "profile_decel", static_cast<double>(p.profile_decel));
  PutIfSet(out, "watchdog_ms", static_cast<double>(p.watchdog_timeout_ms));
  PutIfSet(out, "watchdog_action", static_cast<double>(p.watchdog_action));
  PutIfSet(out, "program_pdos", p.program_pdos);
  WriteBlockEnd(out);

  WriteBlock(out, "kinematics");
  PutIfSet(out, "wheel_radius_m", p.kinematics.wheel_radius);
  PutIfSet(out, "wheel_base_m", p.kinematics.wheel_base);
  PutIfSet(out, "gear_ratio", p.kinematics.gear_ratio);
  PutIfSet(out, "encoder_cpr", static_cast<double>(p.kinematics.encoder_cpr));
  PutIfSet(out, "max_linear_velocity", p.kinematics.max_linear_velocity);
  PutIfSet(out, "max_angular_velocity", p.kinematics.max_angular_velocity);
  WriteBlockEnd(out);

  WriteBlock(out, "odometry");
  PutIfSet(out, "enabled", p.publish_odometry);
  PutIfSet(out, "callback", p.publish_callback);
  PutIfSet(out, "udp", p.publish_udp);
  PutIfSet(out, "udp_host", p.udp_host);
  PutIfSet(out, "udp_port", static_cast<double>(p.udp_port));
  PutIfSet(out, "udp_repeat", static_cast<double>(p.udp_repeat));
  WriteBlockEnd(out);

  WriteBlock(out, "misc");
  PutIfSet(out, "dcf", p.dcf_path);
  PutIfSet(out, "bin", p.bin_path);
  PutIfSet(out, "boot_timeout_ms", static_cast<double>(p.boot_timeout.count()));
  PutIfSet(out, "servo_timeout_ms", static_cast<double>(p.servo_timeout.count()));
  WriteBlockEnd(out);

  out << YAML::EndMap;
  return out.c_str() ? out.c_str() : std::string{};
}

// ---------------------------------------------------------------------------
// Odometry sample wire format
//
// Two uint32 headers then six doubles, all little endian, fixed 64 bytes. Fixed width and
// fixed layout is the point: a subscriber can decode it without a schema, and a
// mismatched version shows up as an implausible pose rather than silent corruption.
// ---------------------------------------------------------------------------

void PutU32(unsigned char* p, uint32_t v) {
  p[0] = static_cast<unsigned char>(v & 0xFF);
  p[1] = static_cast<unsigned char>((v >> 8) & 0xFF);
  p[2] = static_cast<unsigned char>((v >> 16) & 0xFF);
  p[3] = static_cast<unsigned char>((v >> 24) & 0xFF);
}

uint32_t GetU32(const unsigned char* p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

void PutF64(unsigned char* p, double v) {
  uint64_t bits;
  std::memcpy(&bits, &v, sizeof(bits));
  for (int i = 0; i < 8; ++i) p[i] = static_cast<unsigned char>((bits >> (8 * i)) & 0xFF);
}

double GetF64(const unsigned char* p) {
  uint64_t bits = 0;
  for (int i = 0; i < 8; ++i) bits |= static_cast<uint64_t>(p[i]) << (8 * i);
  double v;
  std::memcpy(&v, &bits, sizeof(v));
  return v;
}

bool SerializeOdometrySample(const OdometrySample& s, unsigned char* out, std::size_t out_size) {
  if (!out || out_size < kOdometrySampleWireSize) return false;
  std::memset(out, 0, kOdometrySampleWireSize);
  PutU32(out + 0, 1u);                                    // wire format version
  PutU32(out + 4, s.alive ? 1u : 0u);
  PutF64(out + 8, s.stamp_sec);
  PutF64(out + 16, s.x);
  PutF64(out + 24, s.y);
  PutF64(out + 32, s.theta);
  PutF64(out + 40, s.linear_v);
  PutF64(out + 48, s.angular_w);
  PutU32(out + 56, static_cast<uint32_t>(s.left_counts));
  PutU32(out + 60, static_cast<uint32_t>(s.right_counts));
  return true;
}

bool DeserializeOdometrySample(const unsigned char* in, std::size_t in_size, OdometrySample* s) {
  if (!in || !s || in_size < kOdometrySampleWireSize) return false;
  s->alive = GetU32(in + 4) != 0;
  s->stamp_sec = GetF64(in + 8);
  s->x = GetF64(in + 16);
  s->y = GetF64(in + 24);
  s->theta = GetF64(in + 32);
  s->linear_v = GetF64(in + 40);
  s->angular_w = GetF64(in + 48);
  s->left_counts = static_cast<int32_t>(GetU32(in + 56));
  s->right_counts = static_cast<int32_t>(GetU32(in + 60));
  return true;
}

}  // namespace mbdv
