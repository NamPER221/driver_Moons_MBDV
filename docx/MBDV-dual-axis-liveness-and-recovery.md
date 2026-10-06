# Moons' MBDV-2X-520AC — Liveness, Auto-Reconnect, 200 Hz Odometry

*Covers the CiA 301 heartbeat supervision, the reconnect state machine, the fault and STO handling, the 200 Hz control and feedback rate, the odometry output interface, and the config/params.yaml reference for the dual-axis CANopen master.*

## 1. Heartbeat supervision (CiA 301)

Liveness is decided from real CAN frames, not from a software timer. The drive broadcasts its own NMT state on 0x700+node from object 0x1017 (producer heartbeat time); the master subscribes to it with a consumer entry in its own 0x1016 and lets lely raise a heartbeat event when a frame does not arrive in time. That event reaches the driver through DriverBase::OnHeartbeat(), so a node that has genuinely stopped answering is distinguishable from a busy scheduler.

The drive ships 0x1017 = 1000 ms, which is its own documented default, and the master watches with a 1500 ms consumer period, so a node that stops answering is declared lost about 1.5 seconds later. During that window the master keeps sending setpoints to it (they go nowhere, the node is gone) and the odometry stops updating. The 1.5x ratio leaves 500 ms of margin between the expected frame and the deadline: fine on an idle bus, where a drive's own heartbeat timer is stable and the link runs near 40% load, but it would leave little room for jitter. If the program ever reports a spurious 'heartbeat lost', halving heartbeat.producer_ms to 500 ms triples the margin at the cost of one extra frame per second per axis.

| Setting | Object | Default | Meaning |
|---|---|---|---|
| heartbeat.producer_ms | drive 0x1017 | 1000 | how often the drive broadcasts 0x700+node. 1000 is the drive's own default, so this pins the period rather than changing it. 0 skips the write |
| heartbeat.consumer_ms | master 0x1016 | 1500 | the period lely watches. THIS IS the loss-detection time: the supervisor acts on exactly this one signal, which is why there is no separate timeout setting. Must be > producer_ms; keep the ratio at 2x or more unless you have measured the jitter. 0 disables monitoring |

*A node is also reported as not alive in the published odometry if its encoder feedback goes stale for 500 ms, independently of the heartbeat. That is a second, faster signal and it only flags the sample - it does not trigger a reconnect, which stays the heartbeat's job. Without it a dead node would look healthy for up to a full consumer window.*

*0x1016 must be present in the master's object dictionary, because lely builds its NMT service from it at construction time. tools/fix_master_dcf.py verifies this and refuses to write a master.dcf that lacks a consumer slot per node - a missing 0x1016 means OnHeartbeat() can never fire and a lost node goes unnoticed.*

## 2. Automatic reconnect

The program never exits because a node or the CAN link went away. A supervisor runs once per control period and drives each axis through the states below.

| State | Meaning | What happens next |
|---|---|---|
| alive | heartbeat arriving, servo enabled | setpoints are sent normally |
| lost | heartbeat timeout, or the CAN link was re-opened | the axis stops being commanded (setpoint 0) and a reconnect is scheduled |
| recovering | a reconnect attempt is in progress | stages S05..S11 then S10+S12 are re-run for that axis alone |
| failed | attempts exhausted | stops retrying; the process keeps running and waits for an operator |

### 2.1 What a node reconnect does

- Zero that axis' setpoint so a dead node is never driven.
- Run the normal bring-up for that node: NMT reset, wait for boot-up, SDO reachable, NMT start to OPERATIONAL, identity, PDO verification, fault reset.
- Re-apply the mode (S10) and re-enable the servo (S12).
- On success, re-baseline the odometry (see section 4.4) and count the recovery.
- On failure, wait the backoff and try again.

The other axis is not touched by any of this: a failure on one side leaves the other side controllable.

### 2.2 CAN link recovery

A CAN controller error (a yanked cable, a drive power-cycling, the interface being reset) is handled by closing and re-opening the CAN channel, which re-creates the raw socket and re-registers the CAN filters without rebuilding the master and throwing away the odometry. Both nodes are then treated as lost so they are re-established through the normal node reconnect path.

*If the interface was taken down administratively (sudo ip link set can0 down), re-opening cannot succeed on its own. The program says so explicitly and prints the ip command to run; it does not attempt to reconfigure the interface unilaterally, because that needs privileges the process may not have and would silently change the host's network configuration.*

| Setting | Default | Meaning |
|---|---|---|
| reconnect.enabled | true | master the reconnect behaviour at all |
| reconnect.recover_can_link | true | re-open the CAN channel on a controller error |
| reconnect.backoff_ms | 500 | first retry delay |
| reconnect.backoff_max_ms | 5000 | delay ceiling while a node stays down |
| reconnect.max_attempts | 0 | 0 = retry forever, which an unattended rig needs |

## 3. Faults, emergency stop and STO

### 3.1 STO cannot be released in software

**This is the single most important operational fact in this document.** The STO function of the MBDV is a hardwired safety circuit, not a software state. The hardware manual states in section 4.11 that "Safe Torque Off is a hardware level safety function", that it is armed on connector CN5 by the SF1 and SF2 inputs going OFF, and section 4.11.1(7) that "when the STO function is disabled (no longer in use), the STO alarm status of the drive is automatically cleared". There is no CANopen object that releases it, and this program does not pretend otherwise. Re-arming STO requires a person to close the safety circuit - refitting the connector, or closing the safety relay.

### 3.2 What the program does about it

- Detects the state: Statusword, 0x603F, 0x1001, the manufacturer alarm 0x200F, and the digital inputs 0x60FD with their assigned functions from 0x2A20:01..04.
- Classifies the cause (see the table below) and logs the manual's own remedy.
- Refuses to retry the hardware-caused kinds (STO, limit/E-STOP, no main power) rather than spinning, because retrying cannot clear them.
- Once the cause is gone, recovers on its own: CiA 402 fault reset (controlword bit 7), falling back to the manufacturer clear 0x2006 = 1, then the full Shutdown -> Switch On -> Enable Operation sequence.
- Keeps running throughout, and keeps the other axis alive.

### 3.3 Fault classification

| Class | Detected from | Who clears it |
|---|---|---|
| STO engaged | alarm present with no other documented cause; SF1/SF2 on CN5 | an operator, by closing the safety circuit |
| limit / E-STOP input asserted | 0x60FD bit set where 0x2A20 assigns a function that blocks servo enable (factory: X1 CCW-LMT, X2 CW-LMT, X3 HOM-SW, X4 E-STOP) | release the switch, or reassign the input to GPIN via P5-00..P5-03 |
| no main power on V+/V- | Statusword bit 4 (Voltage_enabled) clear | restore 24..60 VDC on V+/V- (manual 4.3); the 24 VDC AUX supply is not enough |
| encoder feedback | manufacturer alarm 0x200F low byte 09/0A/0B/0C | check the encoder cable and 0x6064 against the commanded 0x607A |
| overload / over-current | 0x603F in the 0x23xx / 0x32xx family | reduce the load, the duty cycle or the torque limit P1-06 |
| excessive following error | 0x603F 0x8611, or a large 0x60F4 | clear the mechanical obstruction; check the position error limit P3-04 |
| CAN communication | 0x1001 error register bit 4 | close the gap in the setpoint stream; check watchdog 0x2060 |

*0x200F is the manufacturer's alarm and only its low byte is confirmed to be the code the 2-digit front LED shows; the EDS publishes no code-to-name table. It is therefore reported as the raw code to compare with the LED, not mapped to a name.*

| Setting | Default | Meaning |
|---|---|---|
| fault_recovery.auto_reset | true | attempt a fault reset and re-enable automatically |
| fault_recovery.retry_backoff_ms | 1000 | delay between fault retries |
| fault_recovery.max_attempts | 5 | per fault episode, then wait for a new fault |

## 4. The 200 Hz rate

### 4.1 What runs at 200 Hz

- The control loop period (5 ms): supervision, odometry integration, publication and the mode-specific work such as reading keys and writing setpoints.
- The encoder feedback, because the TPDO event timers are 5 ms.
- The odometry output, one sample per control period.

The loop is scheduled on an absolute timeline rather than "sleep the period", so a slow pass does not make the rate drift, and if it does fall behind it resynchronises instead of accumulating lag.

### 4.2 Why the Controlword has its own, slower rate

The Controlword (0x6040) appears in all three RPDOs, and lely transmits one frame per PDO whenever the object is written. Writing it every 5 ms would therefore triple the frame count for no benefit. The setpoint path therefore writes only 0x60FF, which fires exactly one frame - RPDO3, which also carries the Controlword value latched at servo enable - and the Controlword is refreshed separately at rates.controlword_hz (20 Hz by default), which is still far faster than the drive's communication watchdog 0x2060 (500 ms) needs.

### 4.3 Bus budget

Classic CAN at 500 kbps carries roughly 4100 frames per second with 11-bit IDs and 8 data bytes. At 200 Hz the projection is 6 TPDO frames per period plus one setpoint frame per axis per period and the controlword refresh:

| Traffic | Frames/s at 200 Hz |
|---|---|
| 6 TPDO (statusword, position+velocity, alarms) | 1200 |
| 2 setpoint RPDO (0x60FF per axis) | 400 |
| controlword refresh (2 RPDO per axis at 20 Hz) | 80 |
| total | about 1680, i.e. roughly 41% of a 500 kbps link |

The program prints this projection at start-up and warns above 60%, because a rate that is fine on paper can still saturate the wiring.

### 4.4 Odometry re-baselining after a reconnect

A drive that restarts can report encoder position 0 while the integrated pose is somewhere else entirely. Integrating across that step would teleport the robot in the odometry, so a successful reconnect re-baselines both together: the pose returns to (0,0,0) and the delta reference is re-seeded from the current encoder readings. The log line says so at the time.

*A lost axis contributes its last known encoder reading rather than whatever it happens to report, so a dead axis freezes the odometry instead of corrupting it, and every published sample carries an 'alive' flag so a subscriber can tell a frozen pose from a stationary one.*

## 5. Odometry output for the layers above

The master is the motor layer of a larger navigation stack, so the pose and twist are offered at the control rate in two ways, both selected in config/params.yaml.

| Setting | Mechanism | For |
|---|---|---|
| odometry.callback | std::function invoked per sample | code linked into this binary |
| odometry.udp | 64-byte binary datagram per sample | a subscriber in another process (Python, ROS2) without rebuilding this program |

### 5.1 UDP wire format

Fixed 64 bytes, little endian, versioned. A subscriber can decode it with six struct reads and no schema, and a mismatched build shows up as an implausible pose rather than silent corruption.

| Offset | Type | Field |
|---|---|---|
| 0 | uint32 | wire format version (1) |
| 4 | uint32 | alive: 1 when both axes are healthy |
| 8 | float64 | stamp_sec, seconds since process start (steady clock) |
| 16 | float64 | x [m] |
| 24 | float64 | y [m] |
| 32 | float64 | theta [rad] |
| 40 | float64 | linear_v [m/s], from 0x606C feedback |
| 48 | float64 | angular_w [rad/s], from 0x606C feedback |
| 56 | int32 | left encoder counts, 0x6064 of axis 1 |
| 60 | int32 | right encoder counts, 0x6064 of axis 2 |

```
# receive side, e.g. a Python or ROS2 node
import socket, struct
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('239.255.0.10', 5565))          # join the group and port
s.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP,
            socket.inet_aton('239.255.0.10') + socket.inet_aton('239.255.0.10'))
while True:
    data, _ = s.recvfrom(64)
    (ver, alive, stamp, x, y, th, v, w, left, right) = struct.unpack('<II6d2i', data)
    if ver != 1:
        continue
```

*Publishing is best effort by design: a subscriber that stalls or a socket that fills up must never slow the 200 Hz loop, so failures are counted and logged rather than retried. The telemetry line on the terminal is throttled to about 2.5 Hz independently of the control rate, because a human cannot read 200 lines per second.*

## 6. config/params.yaml reference

Every tunable lives in config/params.yaml. A missing file is not an error - the built-in defaults are used and a warning is printed - but a malformed value is refused, because silently running at the wrong rate or with the wrong wheel geometry would be worse than not starting. The source file is the authority; the table below is a summary.

```
(see config/params.yaml for the annotated file)
```

| Section | Keys |
|---|---|
| bus | interface, expect_bitrate_bps |
| nodes | axis1_id, axis2_id |
| rates | control_hz, controlword_hz, tpdo1_event_ms, tpdo2_event_ms, tpdo3_event_ms |
| heartbeat | producer_ms, consumer_ms, timeout_ms |
| reconnect | enabled, recover_can_link, backoff_ms, backoff_max_ms, max_attempts |
| fault_recovery | auto_reset, retry_backoff_ms, max_attempts |
| drive | expect_p1_00, write_p1_00, watchdog_ms, program_pdos |
| kinematics | wheel_radius_m, wheel_base_m, gear_ratio, encoder_cpr, max_linear_velocity, max_angular_velocity |
| odometry | enabled, callback, udp, udp_host, udp_port, udp_repeat |
| misc | dcf, bin, boot_timeout_ms, servo_timeout_ms |

Inspect what is actually in force, including command-line overrides:

```
sudo ./build/mbdv_dual_axis_node --dump-params
```

## 7. Verifying it on the bench

Configuration is self-consistent and round-trips:

```
./build/mbdv_dual_axis_node --dump-params | tail -n +2 > /tmp/p.yaml
./build/mbdv_dual_axis_node --dump-params --params /tmp/p.yaml | tail -n +2 > /tmp/p2.yaml
diff /tmp/p.yaml /tmp/p2.yaml && echo 'round-trip OK'
```

The bus itself has to be proven separately, because a coherent object dictionary says nothing about whether a drive follows a PDO controlword:

```
sudo ./tools/verify_run.sh
```

That script captures the wire while the tool drives, and its decisive check is Statusword 0x0627 (Operation Enabled) appearing on TPDO1: that state is only reachable by acting on a Controlword, and if that Controlword arrived as an RPDO then PDO genuinely works. It reports PASS or FAIL per layer and names the layer to inspect.

## 8. Known limits

- A lost node takes the full heartbeat consumer window (1.5 s by default) to be detected. The producer/consumer ratio is 1.5x, which is workable but tight; 2x or more is safer if jitter is ever seen on the bus.
- STO is not releasable in software. The program detects, diagnoses and recovers from it, but a person must close the safety circuit.
- The CAN link recovery re-opens the channel; it does not reconfigure the network interface. If can0 was taken down administratively, the program reports that and prints the ip command rather than changing the host configuration itself.
- The bus-load figure is a frame-count projection, not a measurement. Actual load depends on the wiring, the transceiver and the drive's own DSP cycle.
- A 0x1016 heartbeat entry is what makes lost-node detection possible; if a future regeneration of master.dcf drops it, tools/fix_master_dcf.py fails the build rather than letting the master come up blind.
- The odometry is dead-reckoned from the encoders. It drifts; the layers above are expected to fuse it with an inertial or absolute reference, as the EKF configuration in odometry_kinematics_summary.md describes.
