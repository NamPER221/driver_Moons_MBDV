#!/usr/bin/env bash
#
# verify_run.sh - prove whether PDO motion actually works on the bus.
#
# tools/fix_master_dcf.py can only prove the master's object dictionary is coherent.
# Whether the drive really follows a *PDO* controlword is a hardware question, so this
# script watches the wire while the tool drives the axes, then looks for evidence:
#
#   1. the master transmits RPDO frames          0x201/0x301/0x401 (+ 0x202/0x302/0x402)
#   2. the drive transmits TPDO frames           0x181/0x281/0x381 (+ 0x182/0x282/0x382)
#   3. the drive reports Statusword 0x0627 Operation Enabled, received on TPDO1
#   4. the position the drive reports actually changes while the move runs
#
# Evidence 3 is the decisive one. 0x0627 can only be reached by acting on a
# Controlword, and if that Controlword arrived as an RPDO then PDO genuinely works.
# The same check is what fails on a bus that only ever does SDO.
#
# Needs root (raw CAN socket) with the interface already up at the configured bit rate.
#
# Usage:
#   sudo ./tools/verify_run.sh                                 # PP move, both axes
#   sudo ./tools/verify_run.sh --test-velocity
#   sudo ./tools/verify_run.sh --test-motion --step 2000 --p1-00 21
#   sudo ./tools/verify_run.sh -1 1 -2 2 --test-motion

set -uo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="${MBDV_BIN:-$PROJECT_DIR/build/mbdv_dual_axis_node}"
IFACE="${MBDV_IFACE:-can0}"
CAPTURE="${MBDV_CAPTURE:-/tmp/mbdv_candump.log}"
NODE1=1
NODE2=2
ARGS=()

bold() { printf '\n\033[1m%s\033[0m\n' "$*"; }
ok()   { printf '  \033[32mPASS\033[0m  %s\n' "$*"; }
bad()  { printf '  \033[31mFAIL\033[0m  %s\n' "$*"; }
warn() { printf '  \033[33mWARN\033[0m  %s\n' "$*"; }

while [[ $# -gt 0 ]]; do
  case "$1" in
    -1|--axis1)   NODE1="$2"; shift 2 ;;
    -2|--axis2)   NODE2="$2"; shift 2 ;;
    -i|--interface) IFACE="$2"; shift 2 ;;
    -h|--help)    sed -n '2,25p' "${BASH_SOURCE[0]}"; exit 0 ;;
    *)            ARGS+=("$1"); shift ;;
  esac
done
[[ ${#ARGS[@]} -eq 0 ]] && ARGS=(--test-motion --step 5000)

# COB-ID of pre-defined connection set PDO <pdo> of <node>.
cob_id() { printf '%03X' $(( $1 + $2 )); }

# Mirrors decode_cia402_state() in include/mbdv/cia402_defs.hpp.
cia402_state_name() {
  local sw="$1"
  if   (( (sw & 0x004F) == 0x000F )); then echo "Fault Reaction Active"
  elif (( (sw & 0x004F) == 0x0008 )); then echo "Fault"
  elif (( (sw & 0x006F) == 0x0007 )); then echo "Quick Stop Active"
  elif (( (sw & 0x006F) == 0x0027 )); then echo "Operation Enabled (Servo ON)"
  elif (( (sw & 0x006F) == 0x0023 )); then echo "Switched ON"
  elif (( (sw & 0x006F) == 0x0021 )); then echo "Ready to Switch ON"
  elif (( (sw & 0x004F) == 0x0040 )); then echo "Switch ON Disabled"
  elif (( (sw & 0x004F) == 0x0000 )); then echo "Not Ready to Switch ON"
  else echo "Unknown"
  fi
}

if [[ $EUID -ne 0 ]]; then
  echo "ERROR: run as root - a raw CAN socket is needed." >&2
  exit 2
fi
if [[ ! -x $BIN ]]; then
  echo "ERROR: $BIN missing. Build it first:" >&2
  echo "  cmake -S '$PROJECT_DIR' -B '$PROJECT_DIR/build' && cmake --build '$PROJECT_DIR/build' -j" >&2
  exit 2
fi
command -v candump >/dev/null || { echo "ERROR: can-utils (candump) not installed." >&2; exit 2; }

# ------------------------------------------------------------------ 1. the bus
bold "=== 1. Bus $IFACE ==="
if ! ip link show "$IFACE" >/dev/null 2>&1; then
  echo "ERROR: $IFACE does not exist." >&2
  echo "       sudo ip link add dev $IFACE type can" >&2
  exit 1
fi
ip -details link show "$IFACE" | head -3 | sed 's/^/  /'
RATE=$(ip -details link show "$IFACE" | grep -o 'bitrate [0-9]*' | head -1 | cut -d' ' -f2)
if [[ -z ${RATE:-} ]]; then
  warn "no CAN bit rate configured - nothing can be heard."
elif [[ $RATE != 500000 ]]; then
  bad "bit rate is $RATE bps; the MBDV is set to 500 kbps (DIP SW7 = 1)."
  echo "       sudo ip link set $IFACE down && sudo ip link set $IFACE type can bitrate 500000 && sudo ip link set $IFACE up"
else
  ok "bit rate 500000 bps matches MBDV DIP SW7 = 1"
fi

# ------------------------------------------------------------- 2. capture + run
bold "=== 2. Running: $(basename "$BIN") ${ARGS[*]} ==="
echo "  nodes: AX1=$NODE1 AX2=$NODE2    capture: $CAPTURE"

candump -L "$IFACE" > "$CAPTURE" 2>/dev/null &
CANDUMP_PID=$!
sleep 1

"$BIN" -i "$IFACE" -1 "$NODE1" -2 "$NODE2" --no-color "${ARGS[@]}"
TOOL_RC=$?
echo "  tool exit code: $TOOL_RC"

sleep 1
kill "$CANDUMP_PID" 2>/dev/null
wait "$CANDUMP_PID" 2>/dev/null

# -------------------------------------------------------- 3. what hit the wire
bold "=== 3. What was actually on the wire ==="

if [[ ! -s $CAPTURE ]]; then
  bad "$CAPTURE is empty - nothing was transmitted or received at all."
  echo "       Check the transceiver, CAN_H/CAN_L, the GND bond and the 120 ohm"
  echo "       terminator (DIP SW8 = 1 on the last device). A silently wired bus"
  echo "       looks exactly like this."
  exit 1
fi

# Normalise to "<COB-ID> <data>" per line. candump -L prefixes a timestamp and the
# interface name, and that layout has changed between can-utils releases, so pick
# the ID#DATA token out of each line rather than anchoring on a prefix.
FRAMES="${CAPTURE}.frames"
grep -oE '[0-9A-Fa-f]{3,8}#[0-9A-Fa-f]+' "$CAPTURE" \
  | tr 'A-F' 'a-f' | tr '#' ' ' > "$FRAMES"

count_frames() {  # count_frames <cob-id> [data-prefix]
  awk -v id="$1" -v pfx="${2:-}" \
    '$1 == id { if (pfx == "" || index(tolower($2), pfx) == 1) c++ } END { print c + 0 }' \
    "$FRAMES"
}

TX_IDS=(); RX_IDS=(); TPDO1_IDS=(); TPDO2_IDS=()
for n in "$NODE1" "$NODE2"; do
  for base in 0x200 0x300 0x400; do TX_IDS+=("$(cob_id "$base" "$n")"); done
  for base in 0x180 0x280 0x380; do RX_IDS+=("$(cob_id "$base" "$n")"); done
  TPDO1_IDS+=("$(cob_id 0x180 "$n")")
  TPDO2_IDS+=("$(cob_id 0x280 "$n")")
done

tx_master=0; rx_drive=0
for id in "${TX_IDS[@]}"; do tx_master=$(( tx_master + $(count_frames "$id") )); done
for id in "${RX_IDS[@]}"; do rx_drive=$(( rx_drive + $(count_frames "$id") )); done

echo "  master -> drive (RPDO) frames : $tx_master"
echo "  drive  -> master (TPDO) frames : $rx_drive"
echo "  distinct frames captured       : $(sort -u "$FRAMES" | wc -l)"
echo
echo "  frame count per COB-ID:"
awk '{print $1}' "$FRAMES" | sort | uniq -c | sort -k2 \
  | awk '{printf "        0x%s  %6d frames\n", toupper($2), $1}'

if (( tx_master > 0 )); then
  ok "the master put RPDO frames on the bus"
else
  bad "the master never transmitted an RPDO frame - PDO cannot leave the tool."
  echo "       With a correct master.dcf this should not happen. Re-check with:"
  echo "         python3 tools/fix_master_dcf.py config/master.dcf --check"
fi

if (( rx_drive > 0 )); then
  ok "the drive transmits TPDO frames"
else
  bad "no TPDO frames from the drive - check bit rate, wiring and the node-ID switches."
fi

# Statusword 0x0627 = Operation Enabled, arriving on TPDO1. It is a 16 bit little
# endian value, so on the wire it is the byte pair 27 06.
OP_ENABLED=$(printf '%02x%02x' $(( 0x0627 & 0xFF )) $(( 0x0627 >> 8 )))
op_enabled=0
for id in "${TPDO1_IDS[@]}"; do
  op_enabled=$(( op_enabled + $(count_frames "$id" "$OP_ENABLED") ))
done

if (( op_enabled > 0 )); then
  ok "Statusword 0x0627 Operation Enabled seen on TPDO1 ($op_enabled frames)"
  ok "the drive acted on a Controlword delivered over PDO - PDO WORKS"
else
  bad "Statusword never reached 0x0627 on TPDO1: the drive never enabled over PDO."
  echo "        Statusword actually seen on TPDO1 (decoded, most frequent first):"
  for id in "${TPDO1_IDS[@]}"; do
    awk -v id="$id" '$1 == id { print substr(tolower($2), 1, 4) }' "$FRAMES" \
      | sort | uniq -c | sort -rn | head -6 \
      | while read -r c bytes; do
          # Statusword is 16 bit little endian: value = high byte << 8 | low byte.
          sw=$(( 16#${bytes:2:2}${bytes:0:2} ))
          printf '          0x%s  %6d frames   %s\n' "$id" "$c" "$(cia402_state_name "$sw")"
        done
  done
  cat <<'EOF'

        CiA 402 reference (16 bit little endian, so swap each byte pair):
          0x0240 Switch On Disabled    0x0221 Ready to Switch On
          0x0223 Switched On           0x0627 Operation Enabled
        * stuck at 0x0240 with bit 4 = 1 -> the Enable Voltage command was refused.
          Check the STO circuit (manual 4.11) and leave the factory inputs open:
          X1 = CCW-LMT, X2 = CW-LMT, X3 = HOM-SW, X4 = E-STOP (manual 7.1.1.2).
        * stuck at 0x0240 with bit 4 = 0 -> no main power on V+/V-. Manual 4.3 wants
          24..60 VDC there; the 24 VDC AUX supply alone does not enable a servo.
        * nothing on TPDO1 at all -> the receive path, not the state machine.
        * a fault code in 0x603F / 0x200F (TPDO3, 0x381) -> decode it and fix that first.
EOF
fi

# Did the position the drive reports actually change?
distinct_pos=0
for id in "${TPDO2_IDS[@]}"; do
  c=$(awk -v id="$id" '$1 == id { print substr(tolower($2), 5, 8) }' "$FRAMES" | sort -u | wc -l)
  distinct_pos=$(( distinct_pos + c ))
done
if (( distinct_pos > 2 )); then
  ok "position feedback changed across $distinct_pos values - the drive actually moved"
else
  warn "position feedback barely changed - PDO works but the drive is not turning."
  warn "      Check mechanical coupling, CW/CCW limit inputs, and the torque limit P1-06."
fi

# ------------------------------------------------------------------ 4. verdict
bold "=== 4. Verdict ==="
if (( tx_master > 0 && op_enabled > 0 )); then
  printf '\033[32m  PASS - PDO works, this master can drive the axes.\033[0m\n\n'
  echo "  Then: raise --step gradually, try --test-velocity, then --test-kinematics,"
  echo "        then -t for keyboard teleop."
  rm -f "$FRAMES"
  exit 0
fi
printf '\033[31m  NOT YET - each FAIL above names the layer to inspect.\033[0m\n\n'
echo "  Full capture kept: $CAPTURE"
echo "  Inspect it with:  grep -E '^ *($(IFS=,; echo "${TPDO1_IDS[*]}")'\"'#' $CAPTURE | head -20"
exit 1
