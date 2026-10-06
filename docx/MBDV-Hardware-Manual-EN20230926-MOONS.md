# MBDV-Hardware-Manual-EN20230926-MOONS.pdf

Converted from `MBDV-Hardware-Manual-EN20230926-MOONS.pdf` (181 pages). `<!-- page N -->` marks the start of
page N of the original PDF; figures are not reproduced.

<!-- page 1 -->

MBDV Series
Low Voltage Servo System
Hardware Manual

### SHANGHAI AMP&MOONS’ AUTOMATION CO.,LTD.

<!-- page 2 -->

### Contents

1 Introduction ....................................................................................................................7
1.1 About this manual .................................................................................................7
1.2 MBDV Series Low Voltage Servo Documentation ................................................7
1.3 Safety ...................................................................................................................7
1.4 Safety sign ............................................................................................................7
1.5 Safety Precautions ...............................................................................................8
1.5.1 Storage ........................................................................................................8
1.5.2 Installation Precautions ...............................................................................8
1.5.3 Wiring Precautions ......................................................................................8
1.5.4 Precautions during trial operation ................................................................9
1.6 Certified Specifications .........................................................................................9
1.7 Maintenance and Inspection...............................................................................10
1.7.1 Check items and cycles .............................................................................10
1.7.2 Replacement of parts ................................................................................10
2 Basic Information .........................................................................................................11
2.1 Product confirmation...........................................................................................11
2.2 Drive model introduction .....................................................................................11
2.2.1 Drive model description .............................................................................11
2.2.2 Drive Specifications ...................................................................................12
2.2.3 Dimensions of the driver (Unit: mm) ..........................................................13
2.3 Motor model introduction ....................................................................................14
2.3.1 Motor nameplate description .....................................................................14
2.3.2 Motor model description ............................................................................14
2.3.3 Motor Specifications and Dimensions .......................................................15
2.4 General Motor Specifications .............................................................................20
2.5 Brake Specifications ...........................................................................................20
3 Installation ....................................................................................................................21
3.1 Storage conditions ..............................................................................................21
3.2 Installation conditions .........................................................................................21
3.3 Drive installation space.......................................................................................21
3.4 Motor Installation Precautions ............................................................................21
3.4.1 Protection of encoders and bearings .........................................................21
3.4.2 Precautions for motor in oil and water environment ..................................22
3.4.3 Wiring ........................................................................................................23
3.4.4 Motor temperature rise ..............................................................................23
4 System Configuration and Wiring ..............................................................................24
4.1 Electromagnetic Compatibility (EMC) .................................................................24
4.1.1 Ground ......................................................................................................24
4.1.2 Shielding of motor cables ..........................................................................25
4.1.3 Ferrite Ring ................................................................................................25
4.2 External circuit wiring..........................................................................................26
4.2.1 Interface introduction .................................................................................26
4.2.2 DIP switch function description ..................................................................27

<!-- page 3 -->

4.2.3 Driver terminal description ........................................................................28
4.2.4 Considerations when Wiring .....................................................................28
4.2.5 Precautions for the use of drag chain cables ...........................................29
4.2.6 Recommended Wires ...............................................................................29
4.2.7 Crimp terminal ..........................................................................................30
4.2.8 Ground terminal ........................................................................................30
4.3 Main & AUX - Driver power wiring method ........................................................31
4.4 Motor connection method ..................................................................................31
4.4.1 Block diagram of the connection between the driver and the motor power line ...31
4.4.2 Definition of motor power line wiring .........................................................31
4.5 Encoder -the connection method between the driver and the motor encoder line .........32
4.5.1 The block diagram of the connection between the driver and the motor encoder line .32
4.5.2 Encoder Pinout .........................................................................................32
4.5.3 Encoder line interface definition ...............................................................33
4.6 Using an electromagnetic brake ........................................................................34
4.6.1 Connection diagram .................................................................................34
4.6.2 Precautions for the use of brake motors ...................................................34
4.6.3 Brake action sequence .............................................................................35
4.7 REG- Regenerative energy absorption resistor wiring method .........................35
4.7.1 Calculation method of regenerative energy ..............................................36
4.7.2 Wiring method .........................................................................................38
4.7.3 Device setting parameters ........................................................................38
4.8 Communication Interface...................................................................................39
4.8.1 USB- Host computer debugging interface ................................................39
4.8.2 Wireless - Host computer wireless debugging interface ...........................39
4.8.3 COM1/2 – CANOpen / RS485 Communication interface .........................39
4.9 I/O Input and output signal wiring (MBDV-520AC) ............................................40
4.9.1 I/O Input and output signal specifications .................................................40
4.9.2 I/O Signal Pin Block Diagram ...................................................................40
4.9.3 Input and Output Pinout ............................................................................41
4.10 Input and Output Signals (MBDV-2X-520AC) .................................................45
4.10.1 Input and Output Specifications ..............................................................45
4.10.2 I/O Signal Pin Block Diagram .................................................................46
4.10.3 Input and Output Pinout ..........................................................................47
4.10.4 Encoder Feedback Output ......................................................................51
4.11 STO - Safe torque off function .........................................................................52
4.11.1 Safe Torque Off Precautions ...................................................................52
4.11.2 STO Function Connector ........................................................................53
5 LED Display .................................................................................................................55
5.1 Display content ..................................................................................................55
5.1.1 Decimal point meaning .............................................................................55
5.2 Alarm Codes and Definitions .............................................................................55
6 Commissioning ...........................................................................................................57
6.1 Inspection before commissioning ......................................................................57
6.2 Commissioning Steps ........................................................................................57
6.3 Drive Configuration ............................................................................................58

<!-- page 4 -->

7 Control Modes and Functions ...................................................................................59
7.1 I/O signal setting ................................................................................................59
7.1.1 Input Signal Configuration ........................................................................59
7.1.2 Output signal configuration .......................................................................60
7.1.3 Servo Enable ............................................................................................61
7.1.4 Alarm Reset ..............................................................................................61
7.1.5 CW, CCW Limit .........................................................................................62
7.1.6 Gain Select ...............................................................................................62
7.1.7 Emergency stop ........................................................................................64
7.1.8 Fault output ..............................................................................................65
7.1.9 Warning output ........................................................................................66
7.1.10 Motor brake control .................................................................................67
7.1.11 Servo Ready Output ...............................................................................68
7.1.12 Servo-on Status Output .........................................................................68
7.1.13 Dynamic position error output .................................................................69
7.1.14 Software limit output ...............................................................................70
7.1.15 Timing diagram .......................................................................................71
7.2 Position control mode ........................................................................................72
7.2.1 Position control mode configuration .........................................................72
7.2.2 Command smoothing filter setting ............................................................74
7.2.3 Positioning complete signal ......................................................................75
7.2.4 position reached output ............................................................................75
7.2.5 Gain parameter in position mode .............................................................76
7.3 Velocity control mode ........................................................................................77
7.3.1 Velocity control mode configuration ..........................................................77
7.3.2 Zero speed clamp function .......................................................................78
7.3.3 Rotation direction switch ...........................................................................79
7.3.4 Zero speed output ....................................................................................80
7.3.5 Velocity reached output ............................................................................81
7.3.6 Velocity consistent output .........................................................................82
7.3.7 Velocity control Mode methods .................................................................83
7.4 Torque control mode ..........................................................................................84
7.4.1 Commanded torque control .....................................................................84
7.4.2 Torque command filter ..............................................................................84
7.4.3 Speed limit in torque mode .......................................................................85
7.4.4 Output during speed limiting .....................................................................86
7.4.5 Torque reaches output ..............................................................................86
7.4.6 Torque consistent output ..........................................................................87
7.5 Torque limit ........................................................................................................88
7.5.1 Torque Limiting Methods ..........................................................................88
7.5.2 Torque limit reached output ....................................................................91
7.6 Encoder feedback output function .....................................................................91
7.6.1 Pulse frequency division output signal pin ................................................91
7.6.2 Pulse frequency division output mode setting ..........................................92
7.6.3 Pulse output gear ratio .............................................................................93
7.7 Dynamic braking ................................................................................................94

<!-- page 5 -->

7.7.1 Dynamic braking when Servo Off signal triggered ...................................95
7.7.2 Dynamic braking when driver fault present ..............................................95
7.8 Home function ...................................................................................................95
7.8.1 Back to the origin basic concept ...............................................................97
7.8.2 Introduction to the way of returning to the origin ......................................97
8 Parameter setting ......................................................................................................121
8.1 Parameter classification ..................................................................................121
8.2 List of parameters ............................................................................................121
8.3 Parameter Description .....................................................................................125
8.3.1 Group P0-XX: PID gain setting ...............................................................125
8.3.2 Group P1-XX: Configuration --- Configuration class parameters ...........131
8.3.3 P2-XX: Trajectory--- trajectory planning .................................................138
8.3.4 P3-XX group: Encoder & Step/Dir--- Encoder and input pulse settings .140
8.3.5 Group P5-XX: IO settings .......................................................................143
9 Troubleshooting ........................................................................................................150
9.1 List of drive alarms ..........................................................................................150
9.2 Driver Alarm Causes and Solutions .................................................................152
10 Servo gain setting ...................................................................................................156
10.1 Servo tuning process and mode introduction ................................................156
10.1.1 Servo tuning flow chart .........................................................................156
10.1.2 Introduction to parameter tuning mode .................................................157
10.2 No-Tuning Mode ............................................................................................158
10.3 Auto tuning mode...........................................................................................159
10.3.1 Auto-tuned motion profile conditions ....................................................159
10.3.2 Auto-Tuning Flowchart .........................................................................160
10.3.3 Start auto-tuning -- software operation on ............................................161
10.4 Advanced Tuning Mode .................................................................................164
10.4.1 Introduction to Advanced Tuning Mode ................................................164
10.4.2 Parameters in Advanced Tuning Mode .................................................165
10.4.3 Servo system parameter description ....................................................166
10.4.4 Gain parameter of speed loop ..............................................................167
10.5 Resonance suppression ................................................................................168
10.5.1 Torque Command Filter ........................................................................168
10.5.2 Notch Filters .........................................................................................169
10.5.3 Setting the notch filter manually ...........................................................171
10.6 End Vibration Suppression ............................................................................178
10.7 External Vibration Suppression .....................................................................179
11 Connect us ...............................................................................................................181

<!-- page 6 -->

**Disclaimer**
The information in this manual was accurate and reliable at the time of its publication. Shanghai
Anpu Mingzhi Automation Equipment Co., Ltd. reserves the right to change the product
specifications described in this manual at any time without prior notice.
**Trademark Rights**
All proper names mentioned in this manual are the trademarks of their respective owners.
**Customer service**
Shanghai Anpu Mingzhi Automation Equipment Co., Ltd. is committed to providing quality customer
service and support for all of our products. Our goal is to provide our clients with the information
and resources they need in a timely and reliable manner. For quick service, we recommend that
you contact your local sales representative for order status and logistics information, product
information and documentation, as well as on-site technical support and applications. If you have
special reasons and cannot contact your sales representative, please use the following relevant
contact methods:
If you need technical support, please contact: ama-support@moons.com.cn

<!-- page 7 -->

## 1 Introduction

### 1.1 About this manual

This manual is the hardware manual of MBDV series low voltage servo driver. It provides
installation, configuration and basic operation of MBDV servo unit. This document is intended for
qualified personnel who transport, assemble, and maintain this equipment.

### 1.2 MBDV Series Low Voltage Servo Documentation

This manual is part of a series of documents, the entire series consists of the following:
- MBDV series low voltage servo hardware manual. Details hardware installation, configuration
and operation.
- CANopen communication manual. Describes the CANopen communication function of the drive-
in detail.
- Modbus communication manual. Detailed introduction to drive Modbus RTU communication
function.
- Luna software user manual. Introduce how to use the Luna software.

### 1.3 Safety

To prevent hazards to people and damage to property, installation should only be performed by
qualified personnel.
MBDV series low voltage servo products use dangerous voltage. The drive must be
properly grounded.
Before you install MBDV series low voltage servo products, please read the product manual
carefully. Failure to follow safe operating instructions may result in personal injury or equipment
damage.

### 1.4 Safety sign

Safety signs indicate potential personal hazards or equipment damage, such as failure to follow
recommended precautions and actual safe practices. The following are cautionary safety symbols
used in this manual and on the drive:

Caution Dangerous Voltage Earth Caution,Hot Surface

<!-- page 8 -->

### 1.5 Safety Precautions

#### 1.5.1 Storage

Please note the following when storing:
◆ Please put this drive in the packing box and store it in a dry, dust-free place away from direct
sunlight
◆ Storage ambient temperature between -20 ℃ to +65 ℃
◆ Storage environment humidity is in the range of 10% to 85% without condensation
◆ Avoid storage in corrosive atmospheres

#### 1.5.2 Installation Precautions

| ◆ It is forbidden to use this product in the environment with moisture, corrosive gas, flammable and<br>explosive |
|---|
| ◆ Do not use this product in places with strong vibration and shock |
| ◆ Do not use cables immersed in water or oil |
| ◆ Do not squeeze or stress the cable to avoid dangerous situations such as electric leakage caused<br>by damage to the cable. |
| ◆ Do not block the heat dissipation holes of the driver, and avoid metal chips and other conductive<br>objects entering the driver during installation |
| ◆ Do not directly touch the rotating motor shaft with your hands |
| ◆ Do not knock the motor during installation, so as not to damage the motor shaft or the internal<br>optical encoder |
| ◆ In the first test run, first separate the coupling or belt of the mechanical equipment, so that the motor<br>is in a no-load state |
| ◆ Incorrect parameters will cause abnormal operation under load |
| ◆ The temperature of the drive radiator, motor, and external regeneration resistor will rise during<br>operation, please avoid touching |
| ◆ Do not lift the motor lead wire during transportation and installation |

#### 1.5.3 Wiring Precautions

| ◆ Do not connect the power grid to the UVW motor terminal on the drive side, it will damage the drive<br>or cause a fire |
|---|
| ◆ Please connect the output UVW of the driver and the UVW of the servo motor directly, and do not<br>pass the electromagnetic contactor in the middle. |
| ◆ Please tighten the fixing screws of the power supply and motor output terminals, otherwise it may<br>cause fire |
| ◆ Do not switch the main power supply of the driver frequently. If you really need to switch the power<br>supply repeatedly, please control it to less than 1 time per minute. |
| ◆ Avoid bundling the main circuit cable with the input and output signal cables. |
| ◆ Please use twisted pair shielded wire for input signal wire and encoder signal wire |
| ◆ Please use the specified power supply voltage |
| ◆ One wire insertion port of the terminal block, please insert only one wire |
| ◆ When inserting the wire, do not short the core wire with the adjacent wire |
| ◆ Be sure to ensure that the driver power supply and motor are well grounded |
| ◆ Before powering on, make sure all wiring is correct |

<!-- page 9 -->

#### 1.5.4 Precautions during trial operation

| ◆ Do not directly touch the rotating motor shaft with your hands |
|---|
| ◆ In the first test run, first separate the coupling or belt of the mechanical equipment, so that the motor<br>is in a no-load state |
| ◆ Incorrect parameters will cause abnormal operation under load |
| ◆ The temperature of the drive radiator, motor, and external regeneration resistor will rise during<br>operation, please avoid touching |
| ◆ Before the machine starts running, please confirm whether the emergency stop device can be<br>activated at any time |
| ◆ Use servo motors with brakes on vertical loads to avoid equipment falling during alarm, failure,<br>power failure |

### 1.6 Certified Specifications

MBDV series low voltage servo products are designed to meet the following standards.

|  | Drive | Motor |
|---|---|---|
| EMC<br>Directive | EN 61800-3 |  |
| LVD | EN 61800-5-1 |  |
| Functional<br>Safety(STO) | UL61800-5-2(SIL2) |  |
|  | IEC61508 |  |
|  | ISO13849-1(PL d) |  |
|  | UL 61800-5-1 |  |
|  | C22.2 No.274-13 |  |

<!-- page 10 -->

### 1.7 Maintenance and Inspection

#### 1.7.1 Check items and cycles

The normal use conditions of the servo are: Annual average ambient temperature: 30 ℃ Average
load rate: below 80% Daily operating time: 20 hours or less. The items of daily inspection are as
follows:

| Type | Inspection cycle | Check item |
|---|---|---|
| Daily inspection | Daily | ◆ Check the ambient temperature, humidity, dust, foreign matter, and condensation |
|  |  | ◆ Is there any abnormal vibration or noise |
|  |  | ◆ Voltage |
|  |  | ◆ Peculiar smell |
|  |  | ◆ Are there any foreign objects in the vents? |
|  |  | ◆ Whether the connector is loose |
|  |  | ◆ Whether there is foreign matter between the cable and the connector, and whether the<br>cable conductor is exposed |
|  |  | ◆ Is the fastening part loose? |

#### 1.7.2 Replacement of parts

The components inside the servo products will wear and age over time, and the replacement time
of the components varies according to the environmental conditions and usage methods. When
replacement is required, please contact our company or our agent.
Please do not disassemble and repair by yourself.

| Part | Part | Standard replacement cycle | Remark |
|---|---|---|---|
| Driver | Filter capacitor | About 6 years | The standard replacement cycle is for<br>reference only. Even if the standard<br>replacement cycle is not completed, it<br>needs to be replaced in the event of an<br>abnormality. |
|  | Aluminum electrolytic<br>capacitors on circuit<br>boards | About 6 years |  |
|  | Power-up buffer relay | About 100,000 times (depending on usage<br>conditions) |  |
|  | Power-on snubber resistor | About 20,000 times (depending on usage<br>conditions) |  |
| Motor | Oil seal | 5000 hours |  |

<!-- page 11 -->

## 2 Basic Information

### 2.1 Product confirmation

Please refer to the following chapters to confirm the model of the driver and the model of the servo
motor. A complete operational servo should include the following components:
- Power-matched servo drives and servo motors
- Mini for USB to PC _ USB communication cable (optional)
- Wireless communication module for Wireless port (optional)
- Connector for I/O port (standard item)
- Encoder signal frequency division output connector for Encoder output port (for Dual axis
driver, standard product)
- STO connector for STO port (standard item)
- Connectors for COM1 and COM2 ports, for CAN Open or RS485 communication
(standard)

### 2.2 Drive model introduction

#### 2.2.1 Drive model description

### MBDV - 2X - 5 20A C -

### ***

① ② ③ ④ ⑤

① MBDV Series ④ Current
② # of Axes 20A: Continuous Curent 20A (RMS)
Empty: Single axis Peak Cureent 60A (RMS)
2X: Dual axis ⑤ Communication Protocol
③ Input Voltage C: CANopen、RS485
5: 24 ~ 60VDC ⑥ Custom Code

<!-- page 12 -->

#### 2.2.2 Drive Specifications

| Input power | Main circuit power supply |  |  | 24V ~ 60VDC ± 10% |
|---|---|---|---|---|
|  | Control loop power |  |  | 24VDC ± 10% |
| Insulation withstand voltage |  |  |  | One time to ground: withstand voltage 500 VAC, 1 min, (leak: 10 mA) |
| Use<br>environment | Temperature |  |  | ◆ Use temperature: 0 ~ 50 ℃ (if the ambient temperature exceeds 45 ℃, please place<br>it in a well-ventilated place)<br>◆ Storage temperature: -20 ℃ ~ 65 ℃ |
|  | Humidity |  |  | Storage and use: 10 ~ 85%RH, no condensation |
|  | Altitude |  |  | Altitude below 1000m |
|  | Vibration |  |  | 9.8m/s2 or less, 10 ~ 60Hz (Not sustainable at resonance point) |
| Encoder feedback |  |  |  | ◆ 16-bit Magnetic Incremental Encoder<br>◆ 2500-line optical incremental encoder |
| I/O *1 | Digital signal |  | Input | 4 -way optocoupler isolation general-purpose input, can be configured by parameters,<br>5 ~ 24VDC, 20mA |
|  |  |  | Output | ◆ 2 -way optocoupler isolated general-purpose output, function can be configured<br>through parameters, maximum 30VDC, 100mA<br>◆ 1 channel optocoupler isolation motor brake control output |
|  | Pulse signal * 2 |  | Output | 3 -way Line Driver output: encoder feedback frequency division output A ±, B ±, Z ± |
| Comm Port | USB Mini |  |  | Used to connect PC for software debugging |
|  | Wireless |  |  | It is used to connect the wireless module, and connect the PC through the wireless<br>module for software debugging |
|  | CANopen |  |  | CANopen protocol communication |
|  | RS-485 *3 |  |  | Modbus/RTU protocol communication |
| LED display |  |  |  | 2 -digit 7 -segment LED display |
| Regenerative resistor |  |  |  | External regenerative resistor |
| Control mode |  |  |  | ◆ CANopen communication control mode:<br>Complies with CiA402 standard, supports PP, PV, PVT, TQ and HM modes<br>◆ Modbus/RTU communication control mode:<br>Command position mode, command speed mode, command torque mode |
| Available input signals |  |  |  | Alarm clear, forward/reverse rotation inhibit limit, gain switching, zero speed clamp,<br>emergency stop, forward/reverse running torque limit, speed limit, general-purpose<br>input |
| Available output signals |  |  |  | Fault (Error), Warning (Alarm), Servo-Ready Status, Motor Brake Control, Speed<br>Reached, Torque Reached, Position Reached, Servo-ON Status, Dynamic Error<br>Follow, Positioning Complete, Zero Speed Signal, Speed Consistency, Torque<br>Consistent, in speed limit, in torque limit, homing completed, software limit (forward,<br>reverse), general-purpose output |
| Protective function |  |  |  | Overcurrent, overvoltage, under voltage, overheating, abnormal encoder feedback,<br>overload, excessive speed, excessive position error, emergency stop, forward/reverse<br>rotation limit, communication error |
| Protective function |  |  |  | Built-in |
| STO |  |  |  | Built-in |
| Certification |  |  |  | RoHS、CE |
| Drive weight |  | MBDV-520AC |  | 0.4kg |
|  |  | MBDV-2X-520AC |  | 0.9kg |

* 1 The multi-axis drive is described for a single axis
* 2 Single axis drive does not support this function, can be customized if necessary
* 3 RS485 and CANopen share communication interface

<!-- page 13 -->

#### 2.2.3 Dimensions of the driver (Unit: mm)

##### 2.2.3.1 MBDV-520AC

##### 2.2.3.2 MBDV-2X-520AC

<!-- page 14 -->

### 2.3 Motor model introduction

#### 2.3.1 Motor nameplate description

Model Number SM0602GSL-KCY-NNV
Serial No 2109179002
Input Voltage

11.8A Rated 12.9A Stall
Rated Output Power 400W
Output torque, speed 1.27Nm 3000r/min
Maximum speed 4000r/min
Protection class

#### 2.3.2 Motor model description

### S M 0 6 0 2G S L- K CY - N NV * *

| Brake |
|---|
| N No brake |
| B With brake(24VDC) |

SM Series
Custom code

| Frame Size |
|---|
| 04-□40mm |
| 06-□60mm |
| 08-□80mm |

| Motor Length |  |  |  |
|---|---|---|---|
|  | □40 | □60 | □80 |
| 01 |  | 200W |  |
| 02 | 100W | 400W |  |
| 03 |  |  | 750W |

| Winding(DC) |  |
|---|---|
| G | 48V |
| J | 24V |

| Shaft and lead type |
|---|
| KCY: Standard keway, molor cable with Y<br>type terminal |

| Encoder Type |
|---|
| SL: 16-bit incrementel Magnetic Encoder,<br>Communication type |
| EL: 16-bit incremental Magnetic Master<br>EncoderCommunication type<br>1000line incremental Optical Auliary Encoder |

<!-- page 15 -->

#### 2.3.3 Motor Specifications and Dimensions

##### 2.3.3.1 □ 40mm specifications and dimensions

□ Specification

| Type |  | SM0402JSL-KCY-□ NV | SM0402GSL-KCY-□ NV |
|---|---|---|---|
| Recommended drive input voltage at rated speed(DC-Bu |  | s) 24 | 48 |
|  | watts<br>rpm<br>rpm | 100<br>3000<br>4500 |  |
|  | Nm<br>Nm | 0.32<br>0.96 |  |
|  | A (rms)<br>A (rms) | 8.1<br>24.5 |  |
|  | V (rms) / K rpm<br>Nm / A (rms)<br>Ohm @25°C<br>mH (typ.) | 2.53<br>0.042<br>0.23<br>0.25 |  |
|  | Kg·m2<br>Kg·m2<br>N (max.)<br>N (max.)<br>kg<br>kg | 0.0428×10-4<br>0.0494×10-4<br>50<br>60<br>0.55<br>0.8 |  |

Note: The torque and maximum speed depend on the DC bus voltage, please choose the proper supply voltage.
□ Brake Options
□ Dimensions (unit: mm)
1) Without Brake 1) With Brake

050001 050001
05±0001 05±0001
05±0001

900.0
900.0-6h8
M3 8
0 -6h8
M3 8
0
4- 4.2
4.2
4-
120.0
5
���
0 -7h03
)85(
85
120.
64
0 0-7h03
04□
64
520.0-9h3
520.0
A
0
0 -9h3
+0.20 9.2-0.13
2.5±0.2 25±1 L1±1
+0.20 9.2-0.13
A 2.50.2 251 L11
40

| Without Brake | L1 |
|---|---|

| With Brake | L1 |
|---|---|

SM0402JSL-KCY-NNV 96 SM0402JSL-KCY-BNV 133
SM0402GSL-KCY-NNV 96 SM0402GSL-KCY-BNV 133

□ Torque Curves

| SM0402J 24VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d T T o o rq rq u u e e 48VDC P R e a a te k d T T o o rq rq u u e e<br>1.2<br>1.0<br>0.8 )mN(<br>euqroT<br>0.6<br>0.4<br>0.2<br>0.0<br>0 1000 2000 3000 4000 5000 600<br>Speed (rpm) | 24VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d T T o o rq rq u u e e<br>48VDC P R e a a te k d T T o o rq rq u u e e |
|---|---|

| SM0402G 2 4 4 8 V V D D C C P R P R e a e a a t a t e e k k d d T T T T o o o o r r q r q r q q u u u u e e e e 3 6 6 0 V V D D C C P R P R e e a a a a t t e e k k d d<br>1.2<br>1.0<br>0.8 )mN(<br>euqroT<br>0.6<br>0.4<br>0.2<br>0.0<br>0 1000 2000 3000 4000 5000 | 24VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d<br>48VDC P R e a a te k d T T o o rq rq u u e e 60VDC P R e a a te k d |
|---|---|
| Speed (rpm) |  |

Torque
Torque
Torque
Torque

0 6000

<!-- page 16 -->

##### 2.3.3.2 □ 60mm specification and size

□ Specification

| Type |  | SM0601JSL-KCY-□NV | SM0601GSL-KCY-□NV | SM0602GSL-KCY-□NV |
|---|---|---|---|---|
| Recommended drive input voltage at rated speed(DC- |  | Bus) 24 | 48 | 48 |
|  | watts<br>rpm<br>rpm | 200<br>3000<br>4200 | 200<br>3000<br>3900 |  |
|  | Nm<br>Nm | 0.64<br>1.92 | 0.64<br>1.92 |  |
|  | A (rms)<br>A (rms) | 16.3<br>49 | 6.5<br>19.3 |  |
|  | V (rms) / K rpm<br>Nm / A (rms)<br>Ohm @25°C<br>mH (typ.) | 2.61<br>0.043<br>0.1<br>0.216 | 6.52<br>0.108<br>0.52<br>1.348 |  |
|  | Kg·m2<br>Kg·m2<br>N (max.)<br>N (max.)<br>kg<br>kg | 0.165×10-4<br>0.22×10-4<br>70<br>200<br>1.1<br>1.6 | 0.165×10-4<br>0.22×10-4<br>70<br>200<br>1.1<br>1.6 |  |

Note: The torque and maximum speed depend on the DC bus voltage, please choose the proper supply voltage.
□ Brake Options
□ Dimensions (unit: mm)
1) Without Brake 1) With Brake

05±0001 05±0001
05±0001 05±0001 05±0001
0.04A
110.0
0 - 6h41
110.0 0 - 6h41 YEK
M5 10
YEK
M5 10
0.04A
0.04A
0.04A 22.5
3 0 .0 0 - 9h5
3 0 .0 0 - 9h5
47
520.0
22.5
520.0
4
07
0 - 7h05

| 3<br>8<br>30±1 | L1±1 |
|---|---|

| 3<br>8<br>30±1 | 7<br>L1±1 |
|---|---|

0 - 7h05
07

A 16+ -0 0 . . 1 3 1 0 8 0
A 16+ -0 0 . . 1 3 1 0 8 0
4- 5.5
□60
□60
4- 5.5

| Without Brake | L1 |
|---|---|

| With Brake | L1 |
|---|---|

SM0601JSL-KCY-NNV 78 SM0601JSL-KCY-BNV 117.5
SM0601GSL-KCY-NNV 78 SM0601GSL-KCY-BNV 117.5
SM0602GSL-KCY-NNV 107 SM0602GSL-KCY-BNV 146.5

□ Torque curve

| )mN(<br>euqroT | SM0601J 2 4 4 8 V V D D C C P R P R e a e a a t a t e e k k d d T T T T o o o o r r q r q r q q u u u u e e e e 36VDC P R e a a te k d T T o o rq rq u u e e<br>2.0<br>1.6<br>1.2<br>0.8<br>0.4 | 24VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d T T o o rq rq u u e e<br>48VDC P R e a a te k d T T o o rq rq u u e e |
|---|---|---|
| 0<br>0 1000 2000 3000 4000 5000 600<br>Speed (rpm) |  |  |

| 24VDC<br>SM0601G<br>48VDC | P R e a a te k d T T o o rq rq u u e e 36VDC<br>P R e a a te k d T T o o rq rq u u e e 60VDC |
|---|---|

| SM0602G 24VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d T T o o rq rq u u e e 48VDC P R e a a te k d T T o o rq rq u u e e 60VDC P R e a a te k d T T o o rq rq u u e e<br>5.0<br>4.0 )mN(<br>3.0 euqroT<br>2.0<br>1.0<br>0<br>0 1000 2000 3000 4000 5000 6000<br>Speed (rpm) |
|---|

| 24VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d T T o o rq rq u u e e<br>48VDC P R e a a te k d T T o o rq rq u u e e 60VDC P R e a a te k d T T o o rq rq u u e e |
|---|

Peak Torque
Rated Torque
Peak Torque
Rated Torque
3.0

2.4
**)mN(**
1.8
**euqroT**
1.2

0.6

0
0 1000 2000 3000 4000 5000 6000
0
**Speed (rpm)**

<!-- page 17 -->

##### 2.3.3.3 □ 80mm specification and size

□ Specification

| Type |  | SM0803GSL-KCY-NNV | SM0803GSL-KCY-BNV |
|---|---|---|---|
| ecommended drive input voltage at rated speed(DC- |  | Bus) 48 | 48 |
|  | watts<br>rpm<br>rpm | 750<br>3000<br>3600 |  |
|  | Nm<br>Nm | 2.4<br>7.2 |  |
|  | A (rms)<br>A (rms) | 18.8<br>56.7 |  |
|  | V (rms) / K rpm<br>Nm / A (rms)<br>Ohm @25°C<br>mH (typ.) | 8.36<br>0.138<br>0.094<br>0.366 |  |
|  | Kg·m2<br>N (max.)<br>N (max.)<br>kg | 0.89×10-4<br>90<br>270<br>2.6 |  |

R
Ra
Ra
M
Ra
Pe
Ra
Pe
Vo
To
W
W
Ro
Sh
Sh
W
Note: The torque and maximum speed depend on the DC bus voltage, please choose the proper supply voltage.

□ Dimensions (unit: mm)
1) Without Brake 1) With Brake

05±0001
05±0001 0.04A
05±0001 05±0001
05±0001

| 0.0<br>310.0<br>0 - 6h91 |  |
|---|---|
|  | 3<br>40±1 |

310.0
0 - 6h91
M5 10 4- 6.5
4- 6.5 M5 10
0.04A
45°
45°
30.0 0 - 7h07
09□
09□

| 22<br>3<br>40±1 | 49<br>9.5<br>L1±1 |
|---|---|

30.0
0 - 7h07
A

YEK
21.5+
0
.
3
0
0
-0
.
1
1
8
9.5 L1±1
YEK 3 0 .0 0 -
3 0 .0 0 - 9h6
21.5+ -0 0 . . 1 3 1 0 8 0

9h6

| Without Brake | L1 |
|---|---|

| With Brake | L1 |
|---|---|

SM0803GSL-KCY-NNV 110 SM0803GSL-KCY-BNV 156.8

□ Torque curve

24VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d T T o o rq rq u u e e
**SM0803G**
48VDC P R e a a te k d T T o o rq rq u u e e 60VDC P R e a a te k d T T o o rq rq u u e e

8.0

7.0

**)mN(**
6.0

5.0
**euqroT**
4.0

3.0

2.0

1.0

0
0 1000 2000 3000 4000 5000 6000
**Speed (rpm)**

<!-- page 18 -->

##### 2.3.3.4 □ 60mm specification and size

□ Specification

| Type |  | SM0601JEL-KCY-□NV | SM0601GEL-KCY-□NV | SM0602GEL-KCY-□NV |
|---|---|---|---|---|
| Recommended drive input voltage at rated speed(DC- |  | Bus) 24 | 48 | 48 |
|  | watts<br>rpm<br>rpm | 200<br>3000<br>4200 | 200<br>3000<br>3900 |  |
|  | Nm<br>Nm | 0.64<br>1.92 | 0.64<br>1.92 |  |
|  | A (rms)<br>A (rms) | 16.3<br>49 | 6.5<br>19.3 |  |
|  | V (rms) / K rpm<br>Nm / A (rms)<br>Ohm @25°C<br>mH (typ.) | 2.61<br>0.043<br>0.1<br>0.216 | 6.52<br>0.108<br>0.52<br>1.348 |  |
|  | Kg·m2<br>Kg·m2<br>N (max.)<br>N (max.)<br>kg<br>kg | 0.165×10-4<br>0.22×10-4<br>70<br>200<br>1.1<br>1.6 | 0.165×10-4<br>0.22×10-4<br>70<br>200<br>1.1<br>1.6 |  |

Note: The torque and maximum speed depend on the DC bus voltage, please choose the proper supply voltage.
□ Brake Options

□ Dimensions (unit: mm)
1) Without Brake 1) With Brake

05±0001
05±0001 05±0001
05±0001
05±0001 05±003
05±003
0.04A
110.0
0 - 6h41
110.0 0 - 6h41 YEK
M5 10
YEK
M5 10
0.04A
0.04A
0.04A 22.5
3 0 .0 0 - 9h5
3 0 .0 0 - 9h5
22.5
520.0 0 - 7h05
520.0 0 - 7h05
47
07
47
07

A 3 8 16+ -0 0 . . 1 3 1 0 8 0 30±1
A 3 16+ -0 0 . . 1 3 1 0 8 0 8
L1±1
□60
4- 5.5
L1±1 4- 5.5
30±1 □60

| Without Brake | L1 |
|---|---|

| With Brake | L1 |
|---|---|

SM0601JEL-KCY-NNV 98 SM0601JEL-KCY-BNV 137.5
SM0601GEL-KCY-NNV 98 SM0601GEL-KCY-BNV 137.5
SM0602GEL-KCY-NNV 127 SM0602GEL-KCY-BNV 166.5

□ Torque curve

SM0601G SM0602G 24VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d T T o o rq rq u u e e
SM0601J 24VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d T T o o rq rq u u e e 24VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d T T o o rq rq u u e e

| )mN(<br>euqroT | SM0601J<br>48VDC<br>2.0<br>1.6<br>1.2<br>0.8<br>0.4 | Peak Torque<br>Rated Torque |
|---|---|---|
| 0<br>0 1000 2000 3000 4000 5000 600<br>Speed (rpm) |  |  |

| 48VDC P R e a a te k d T T o o rq rq u u e e 60VDC P R e a a te k d T T o o rq rq u u e e |
|---|
| 0 1000 2000 3000 4000 5000 6000<br>Speed (rpm) |

3.0

2.4

**)mN(**
1.8
**euqroT**
1.2

0.6

0 00
0

<!-- page 19 -->

##### 2.3.3.5 □ 80mm specifications and dimensions (unit: mm)

□ Specification

| Type |  | SM0803GEL-KCY-NNV | SM0803GEL-KCY-BNV |
|---|---|---|---|
| Recommended drive input voltage at rated speed(DC-B |  | us) 48 | 48 |
|  | watts<br>rpm<br>rpm | 750<br>3000<br>3600 |  |
|  | Nm<br>Nm | 2.4<br>7.2 |  |
|  | A (rms)<br>A (rms) | 18.8<br>56.7 |  |
|  | V (rms) / K rpm<br>Nm / A (rms)<br>Ohm @25°C<br>mH (typ.) | 8.36<br>0.138<br>0.094<br>0.366 |  |
|  | Kg·m2<br>N (max.)<br>N (max.)<br>kg | 0.89×10-4<br>90<br>270<br>2.6 |  |

Note: The torque and maximum speed depend on the DC bus voltage, please choose the proper supply voltage.
□ Dimensions (unit: mm)
1) Without Brake 1) With Brake

05±0001 05±0001
05±00 05±000
05±003
05±0 05±003
0.04A
0.04A
310.0

| 310.0<br>0 - 6h91 |  |  |
|---|---|---|
|  | 3<br>40±1 | 49<br>9.5<br>L1±1 |

0 - 6h91
4- 6.5 M5 10
M5 10
0.04A
4- 6.5
45°
45°
22
30.0
0 - 7h07
30.0 0 - 7h07

| 3<br>40±1 | 9.5<br>L1±1 |
|---|---|

09□
49
09□
A

21.5+ -0 0 . . 1 3 1 0 8 0
YEK
YEK
21.5+
0
.
3
0
0
-0
.
1
1
8
3 0 .0 0 - 9h6
3 0 .0 0 - 9h6

| With Brake | L1 |
|---|---|

Without Brake L1
SM0803GEL-KCY-NNV 130.8 SM0803GEL-KCY-BNV 178.8

□ Torque curve

| 24VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d T T o o rq rq u<br>SM0803G 48VDC P R e a a te k d T T o o rq rq u u e e 60VDC P R e a a te k d T T o o rq rq u<br>8.0<br>7.0<br>6.0 )mN(<br>5.0 euqroT<br>4.0<br>3.0<br>2.0<br>1.0<br>0<br>0 1000 2000 3000 4000 5000 | 4VDC P R e a a te k d T T o o rq rq u u e e 36VDC P R e a a te k d T T o o rq rq u<br>8VDC P R e a a te k d T T o o rq rq u u e e 60VDC P R e a a te k d T T o o rq rq u |
|---|---|

e
ue
e
ue

6000
**Speed (rpm)**

<!-- page 20 -->

### 2.4 General Motor Specifications

| Insulation class | Class B (130 ℃) |
|---|---|
| Protection class | IP65 (except shaft penetration) |
| Installation conditions | Indoor installation, avoid direct sunlight, corrosive and flammable gases |
| Ambient temperature | Operating temperature: 0 ℃ - 40 ℃; storage temperature: -20 ℃ - 60 ℃ |
| Wet Spend | Storage and use: 20 - 85%RH (No condensation) |
| Ocean pull | Altitude below 1000m |
| Vibrate verb: move | 49m/s 2 or less, 10 - 60Hz (Not sustainable at resonance point) |

### 2.5 Brake Specifications

The motor brake is used to prevent the motor from turning when the brake is de-energized. The
most common way of using it is when the motor is used to control a vertical load, when the motor
is not enabled or powered off, in order to prevent the mechanical mechanism driven by the motor
from shifting due to gravity and other reasons, it is necessary to use a servo motor with a brake.
When the brake is energized, the armature is adsorbed, the brake piece is released, and the motor
can run normally; when the brake is powered off, the armature will be released, the brake piece is
locked, and the motor cannot rotate normally.

| Frame Size | 40mm | 60mm | 80mm |
|---|---|---|---|
| Static friction torque (Nm) | 0.32 | 1.5 |  |
| Rated Voltage (VDC) | 24 |  |  |
| Power (W)(20℃) | 6.9 | 7.2 |  |
| Rated Current (A) | 0.26 | 0.3 | 0.42 |
| Brake Time | < 70ms (Standard air gap, at 20℃) |  |  |
| Release Time | <25ms |  |  |
| Release Voltage | 18.5VDC max.(at 20℃) |  |  |

Motor-intergrated
Motor holding brake Encoder

Coil
Spring
Friction Lining

During normal operation, do not use the motor's brake to decelerate the motor, it will cause
damage to the brake.

<!-- page 21 -->

## 3 Installation

### 3.1 Storage conditions

Please note the following when storing:
- Please put this drive in the packing box and store it in a dry, dust-free place away from direct
sunlight
- Storage ambient temperature between -20 ℃ to +65 ℃
- Storage environment humidity is in the range of 10% to 85% without condensation
- Avoid storage in corrosive atmospheres

### 3.2 Installation conditions

The environmental conditions for the driver of this product are:
1. Temperature is 0 ℃ ~ 50 °C. If the ambient temperature exceeds 45 ℃, please place it in a
well-ventilated place. It is recommended to operate at 45 ℃ for a long time
2. Below ambient temperature to ensure reliable performance of the product.
3. If this product is installed in a distribution box, the distribution box must be sized and ventilated
so that there is no danger of overheating of all electronic devices used inside.
4. Ambient humidity is 10%~85% RH, no condensation
5. Vibration below 9.8m/s 2
6. Do not use the driver near corrosive gas, flammable gas or combustible material
7. Please install the driver in an indoor electrical control box without water and direct sunlight
8. Please avoid using these drive-in dusty places

### 3.3 Drive installation space

- When installing the drive, please reserve enough space up, down, left and right for the drive to
ensure good air circulation.
- Do not block the cooling vents of the drive.
- To ensure the temperature in the electrical control box, it is recommended to install a cooling
fan in the electrical control box.
- Please ground the drive well when installing.

### 3.4 Motor Installation Precautions

#### 3.4.1 Protection of encoders and bearings

- To prevent damage to the encoder and bearing, do not knock the motor body and shaft during
installation.

<!-- page 22 -->

- It is recommended to use a disruptive coupling specially designed for servo motors, which can
provide some buffering during eccentricity or deflection.
- When installing the coupling, please wipe clean the anti-rust oil on the shaft end of the motor.
- When using the keyway, please use the standard key in the motor box.
- When installing a pulley on a servo motor with a keyway, use the threaded hole of the motor
shaft to squeeze the pulley into the motor shaft with a screw.

- When removing the pulley, please use professional tools such as pulley remover to prevent
bearing injury.
- When connecting the shafts, make sure to achieve the desired concentricity. If concentricity is
poor, the resulting vibration can damage bearings and encoders.
- The axial or radial load applied to the motor should not exceed the range specified in the
specifications, please refer to the specification table of each servo motor.
- The material of the output shaft of the servo motor has no rust protection. Although grease
has been used for anti-rust protection before leaving the factory, if the storage time exceeds
six months, in order to ensure that the motor shaft is not rusted, please check the condition of
the motor shaft regularly every three months. And timely supplement the appropriate anti-rust
grease.

#### 3.4.2 Precautions for motor in oil and water environment

- Do not allow oil and water to enter the inside of the motor
- Do not place cables in water or oil
- Since the through part of the motor shaft and the motor lead wires are not IP65 protected,
please ensure that no water or oil enters the motor from such parts
- The motor industrial grade skeleton oil seal can block pollutants (oil, impurities) to prolong the
life of the motor. When leaving the factory, the oil seal will be attached to the box, but will not be
installed on the motor output shaft. After the oil seal is installed, the oil seal will cause a certain
resistance and torque loss to the rotation of the motor shaft. It is recommended that the motor
be dated.
- In the application with liquid, please install the motor wiring port downward

Oil,Fluids,and Fine Dust Oil,Fluids,and Fine Dust

Motor Shaft and Motor Shaft and
End Bell are Exposed End Bell are Protected
to Containments Form Containments

Shaft Seal Installation Shaft Seal Installation
is Recommended is Not Recommended
Sealant between Motor
and Machine(Optional)

- When installing the oil seal, make sure that the lip of the oil seal faces outwards

<!-- page 23 -->

#### 3.4.3 Wiring

- If using a cable chain, use a super flexible cable. And ensure that there is a bending diameter
of more than 100mm.
- Do not twist the cable.
- When moving the motor, do not pull on the cable.
- Do not use the same sleeve for the main circuit cable and the input/output signal cable /
encoder cable, and do not bundle them together. Wiring in this case, the main circuit cable and
the input/output signal cable / encoder cable should be separated by at least 30cm.

#### 3.4.4 Motor temperature rise

Servo motors are rated for continuous operation when mounted on a standard heat sink and in an
ambient temperature of 40 °C. When the servo motor is installed in a small device, the temperature
may rise significantly due to the reduced heat dissipation area of the servo motor.
The dimensions of the standard cooling plate of the servo motor are as follows:

| Base series | Power | Heat sink size |
|---|---|---|
| 40mm | 100W | 200*200*6 aluminum |
| 60mm | 200W, 400W | 250*250*6 aluminum |
| 80mm | 750W | 250*250*6 aluminum |

If the installation environment makes it difficult to use a large heat sink, or if the ambient
temperature exceeds the specification requirements, the following requirements need to be
followed:
- Do not work at rated power, choose a motor that is 1~2 times larger than the actual motor
power required.
- Reduce the acceleration and deceleration of the duty cycle to reduce the motor load.
- Reduced duty cycle for work.
- External forced air cooling of the servo motor using a cooling fan or other means.
- When using a motor with an oil seal, the oil seal will cause a certain resistance and torque loss
to the rotation of the motor shaft, and heat is generated due to the friction between the two. The
required load torque needs to be 70% of the rated torque of the motor.
**Note: Do not put any thermal insulation material between the servo motor and the metal**
**heat sink, so as to avoid the failure of the motor to dissipate heat and cause the motor**
**temperature to rise, which may cause the motor to malfunction.**

<!-- page 24 -->

## 4 System Configuration and Wiring

### 4.1 Electromagnetic Compatibility (EMC)

| MBDV servo driver uses high-speed switching elements inside, which will<br>generate high frequency or low frequency during normal operation interference,<br>and interfere with peripheral equipment through conduction or radiation. |
|---|
| There is also a low-voltage unit inside the servo drive, which is likely to be<br>disturbed by the noise of the peripheral equipment of the drive. A disturbed signal<br>may cause the device to behave unexpectedly. |

Follow the electromagnetic compatibility regulations described in this manual during installation
and wiring. This product can comply with the following regulations: EN 61800-3
To prevent mutual electromagnetic interference between the servo drive and its peripheral devices,
the following countermeasures can be taken.
- Make sure that the driver and motor are well grounded, and the grounding wire should
preferably be a cable of AWG10 or above.
- Do not use the same sleeve for the main circuit cable and the input / output signal cable /
encoder cable, and do not bundle them together. When wiring, the main circuit cable and the
input / output signal cable / encoder cable should be separated by more than 30cm.
- For input/output signal cables and encoder cables, use twisted-pair wires or multi-core twisted-
pair shielded wires.
- The length of the input and output signal cables is less than 3m, and the encoder cable is less
than 5m.
- Do not use the same power source as an electric welding machine, electric discharge machine,
etc. Even if it is not the same power supply, when there is a high-frequency generator nearby,
connect a noise filter to the input side of the main circuit power supply cable and the control
power supply cable.

#### 4.1.1 Ground

Good grounding treatment can give full play to the effect of EMI filter and greatly reduce
interference.
- Must be grounded at a single point in parallel.
- Power extension cable between driver and motor, use cable with shielded mesh.
- The shielding net of the motor power line must be grounded and connected to the ground
terminal of the drive.

<!-- page 25 -->

#### 4.1.2 Shielding of motor cables

The selection of motor cables with shielding nets and properly installed shielding nets can achieve
better EMC effects and interference suppression effects. Please note the following:
- Use shielded cable (Better if there is a double-layer isolation layer)
- The shielding nets at both ends of the motor cable must be grounded with the shortest distance
and maximum contact area. Use clamps to fix the shielding nets at both ends of the motor
cable to the metal plane as shown in the figure below. Please refer to the connection method in
the figure below.
- The protective paint should be removed from the fixing place between the clamp and the metal
plane to ensure good contact, see the figure below.

Clamp Shielding nets

Metal plate

#### 4.1.3 Ferrite Ring

Ferrite magnetic ring, also referred to as the magnetic ring, can effectively absorb the radiation
interference of the wire beam.
The magnetic ring has different impedance characteristics at different frequencies. Generally, the
impedance is very small at low frequencies. When the signal frequency increases, the impedance
of the magnetic ring increases sharply, making it easy for normal and useful signals to pass
through, and can effectively suppress high frequencies. The path of interference signals solves the
problem of high-frequency interference suppression of power lines, signal lines and connectors.
When the magnetic ring suppresses common mode interference, the eddy current loss of the
magnetic ring to the high-frequency signal converts the high-frequency component into heat loss,
so that a low-pass filter can be formed, so that the high-frequency noise can be greatly attenuated,
and the impedance of useful signals at low frequencies can be ignored and does not affect the
normal operation of the circuit.
The wire passing through the magnetic ring can be repeatedly wound on the magnetic ring to
increase the inductance, thereby enhancing the use effect of the magnetic ring. But too many
turns will make the loss too large and the temperature of the magnetic ring will rise too high. The
recommended winding method and number of turns are as follows:

| Digital signal Cables | Wrap the signal wire around the magnetic ring 2-3 times. |
|---|---|
| Motor Power Cables | Wind the U/V/W phase of the motor 2-3 times around the magnetic ring.<br>The ground wire and the shielding net cannot be wound into the magnetic ring. |
| Encoder Cables | Wrap the encoder wire around the magnetic ring 2-3 times. |

Magnetic ring recommended model:

| MOONS' optional models | Manufacturer's model | Manufacturer |
|---|---|---|
| M2-OP3035 | ZCAT3035-1330 | TDK |

<!-- page 26 -->

### 4.2 External circuit wiring

#### 4.2.1 Interface introduction

##### 4.2.1.1 MBDV-520AC

Wireless
M In o te d r u fa le ce S D w I it P ch
USB Settings

STO Interface
CANopen/RS485
Communication Interface

I/O Interface
2-digital LED display

Auxiliary Encoder Interface
power
supply
p M o a w i e n r Brake Motor Interface
supply
Braking resistance

##### 4.2.1.2 MBDV-2X-520AC

Wireless
Module
Interface
DIP
STO Interface USB S S e w tt i i t n c g h s

Encoder Signal
Output Interface

CANopen/RS485
Communication Interface
I/O Interface

2-digital LED display

Auxiliary Encoder Interface Encoder Interface
power
supply
Main Brake Motor Interface Brake Motor Interface
power
supply Braking resistance

<!-- page 27 -->

#### 4.2.2 DIP switch function description

The MBDV driver has a key DIP switch, which is used to set the node address, baud rate and
optional terminal matching resistance of CAN open communication. The specific definitions are as
follows.
- MBDV-520AC
**Node address:**

| SW1 | SW2 | SW3 | Function Description |
|---|---|---|---|
| 0 | 0 | 0 | The node address is set by Luna software, the default is 1 |
| 1 | 0 | 0 | Node address is 1 |
| 0 | 1 | 0 | Node address is 2 |
| 1 | 1 | 0 | Node address is 3 |
| 0 | 0 | 1 | Node address is 4 |
| 1 | 0 | 1 | Node address is 5 |
| 0 | 1 | 1 | Node address is 6 |
| 1 | 1 | 1 | Node address is 7 |

**Baud rate:**

| SW4 | Function Description |
|---|---|
| 0 | The baud rate is set by Luna software, the default is 1Mbps |
| 1 | 500kpbs |

**Select the terminal matching resistor:**

| SW5 | Function Description |
|---|---|
| 0 | 120 Ω resistor connected between CAN_H and CAN_L |
| 1 | Connect a 120 Ω resistor between CAN_H and CAN_L |

- MBDV-2X-520AC
**1# Node address:**

| SW1 | SW2 | SW3 | Function Description |
|---|---|---|---|
| 0 | 0 | 0 | The node address is set by Luna software, the default is 1 |
| 1 | 0 | 0 | Node address is 1 |
| 0 | 1 | 0 | Node address is 2 |
| 1 | 1 | 0 | Node address is 3 |
| 0 | 0 | 1 | Node address is 4 |
| 1 | 0 | 1 | Node address is 5 |
| 0 | 1 | 1 | Node address is 6 |
| 1 | 1 | 1 | Node address is 7 |

**2# Node address:**

| SW4 | SW5 | SW6 | Function Description |
|---|---|---|---|
| 0 | 0 | 0 | The node address is set by Luna software, the default is 1 |
| 1 | 0 | 0 | Node address is 1 |
| 0 | 1 | 0 | Node address is 2 |
| 1 | 1 | 0 | Node address is 3 |
| 0 | 0 | 1 | Node address is 4 |
| 1 | 0 | 1 | Node address is 5 |
| 0 | 1 | 1 | Node address is 6 |
| 1 | 1 | 1 | Node address is 7 |

<!-- page 28 -->

**Baud rate:**

| SW7 | Function Description |
|---|---|
| 0 | The baud rate is set by Luna software, the default is 1Mbps |
| 1 | 500kpbs |

**Select the terminal matching resistor:**

| SW8 | Function Description |
|---|---|
| 0 | No resistor connector between CAN_H and CAN_L |
| 1 | Connect a 120 Ω resistor between CAN_H and CAN_L |

#### 4.2.3 Driver terminal description

| Type | Name | Description |  |  |  |
|---|---|---|---|---|---|
| Main | V+、V- | V+ | positive terminal of Main power supply |  |  |
|  |  | V- | negative terminal of Main power supply |  |  |
| AUX | 24V、GND | 24V | Positive terminal of logic power supply |  | Can be<br>disconnected |
|  |  | GND | negative terminal of logic power supply |  |  |
| REG | REG | Regenerative resistor connector |  |  |  |
| Motor | U、V、W | U | Red | Three-phase power supply for the motor |  |
|  |  | V | Yellow |  |  |
|  |  | W | Blue |  |  |
|  | Brake | Motor brake connector |  |  |  |
| Encoder | Encoder signal input interface | Motor encoder connector |  |  |  |
| I/O | I/O connection | Input and output signal connector |  |  |  |
| Encoder Output | Encoder signal output interface | Encoder signal output connector |  |  |  |
| STO | STO interface | Safe torque off function connector |  |  |  |
| Wireless | Wireless debugging module<br>interface | Wireless debugging module connection port |  |  |  |
| USB | USB debugging interface | Connect to PC |  |  |  |
| COM1 / COM2 | CAN open / RS-485 interface | CAN open / RS-485 communication interface |  |  |  |

#### 4.2.4 Considerations when Wiring

- Please make sure that the driver and motor are well grounded, and the best grounding wire is
AWG10 above cables.
- The ground must be a single point ground.
- Check whether the V+ and V- connections are correct and the correct voltage is connected.
- If using an auxiliary power supply, connect the positive connector of the 24V power supply to
24V and the negative connector of the power supply to GND.
- Make sure the order of U, V, W is red, yellow, blue, wrong order will cause the motor to not
rotate or rotate randomly.
- An emergency stop circuit must be provided to ensure that the power can be cut off
immediately in the event of a fault.
- There is a large-capacity capacitor in the servo drive, even after the power is off, the high
voltage will still be maintained. 5 Do not touch the exposed parts of the drive and motor
terminals for a few minutes.
- Do not use the same sleeve for the main power cable and the input/output signal / encoder
cable, and do not bundle them together. When wiring, the main power cable and the input/
output signal cable / encoder cable should be separated by at least 30cm. Getting too close will
result in malfunction.
- Use twisted-pair or multi-core twisted-pair shielded cables for input/output signal cables and
encoder cables.
- The maximum wiring length of the input/output signal cable is 3m, and the maximum wiring
length of the encoder cable is 5m.

<!-- page 29 -->

#### 4.2.5 Precautions for the use of drag chain cables

When the motor cable needs to be moved or the cable is installed in a drag chain, please use a
special flexible and bend-resistant cable. Ordinary cables are easily damaged during repeated
bending, causing the servo motor to fail to work properly.
When using drag chain cables, make sure that:
- Correctly select the cable that meets the required bending times
- Bending radius of the cable R generally be more than 10 times the diameter of the cable
- Avoid pulling the cable. When wiring inside the drag chain, do not fix or bundle it, so as to avoid
pulling the cable due to insufficient bending radius when bending.
- Please bundle the cables at the two ends of the towline and where the mechanical parts are
fixed

two ends fixed
Banding radius

cable
- The wiring in the drag chain should not be too dense, and ensure that the cables occupy less
than 60% of the internal space of the drag chain
- Mixed wiring of cables with large differences in outer diameter should be avoided. If mixed
wiring is indeed required, please set up baffles

#### 4.2.6 Recommended Wires

- It is recommended to use an insulated wire with a withstand voltage of 600V and a temperature
above 75 ℃ for the main circuit
- Be sure to choose a wire that uses the corresponding current to prevent the wire from
overheating

<!-- page 30 -->

##### 4.2.6.1 Recommended Wires

The recommended cables for each part of the driver are as follows:

| Drive and matching servo<br>motor |  | Rated<br>power<br>(W) | Wire Diameter (AWG) |  |  |  |  |  |  |
|---|---|---|---|---|---|---|---|---|---|
|  |  |  | Connector<br>Main | Connector<br>AUX | Connector<br>REG | Connector Motor |  | Connect<br>or<br>Encoder | --- |
|  |  |  | --- | 24V/GND | REG | U/V/W | Brake | Encoder | Power<br>ground |
| MBDV-<br>520AC | SM0402GSL-<br>KCY-□NV | 100 | 1.0 ~ 1.5mm2<br>AWG16 ~ 18 | 1.0 ~ 1.5mm2<br>AWG16 ~ 18 | 1.0 ~ 1.5mm2<br>AWG16 ~ 18 | 1.0 ~ 1.5mm2<br>AWG16 ~ 18 | 0.5mm2<br>AWG20 | 0.1mm2<br>AWG26 | 2.0 ~<br>5.3mm2<br>AWG10<br>~ 14 |
|  | SM0601GSL-<br>KCY-□NV | 200 | 2.0 ~ 3.5mm2<br>AWG12 ~ 14 |  |  | 2.0 ~ 3.5mm2<br>AWG12 ~ 14 |  |  |  |
|  | SM0602GSL-<br>KCY-□NV | 400 |  |  |  |  |  |  |  |
|  | SM0803GSL-<br>KCY-□NV | 750 | 3.5 ~ 5.3mm2<br>AWG10 ~ 12 |  |  | 4.0 ~ 6.0mm2<br>AWG10 ~ 12 |  |  |  |
| MBDV-2X-<br>520AC | SM0401GSL-<br>KCY-□NV | 50 | 2.0 ~ 3.5mm2<br>AWG12 ~ 14 |  |  | 1.0 ~ 1.5mm2<br>AWG16 ~ 18 |  |  |  |
|  | SM0402GSL-<br>KCY-□NV | 100 |  |  |  |  |  |  |  |
|  | SM0601GSL-<br>KCY-□NV | 200 | 4.0 ~ 6.0mm2<br>AWG10 ~ 12 |  |  | 2.0 ~ 3.5mm2<br>AWG12 ~ 14 |  |  |  |
|  | SM0602GSL-<br>KCY-□NV | 400 |  |  |  |  |  |  |  |
|  | SM0803GSL-<br>KCY-□NV | 750 | 8.0mm2<br>AWG8 |  |  | 4.0 ~ 6.0mm2<br>AWG10 ~ 12 |  |  |  |

◇Represents the encoder type □Represents whether there is a brake

#### 4.2.7 Crimp terminal

Use insulated pin terminals for the power connector AUX.

| ◆ Pin type | ◆ Sleeve type | ◆ Parallel terminal: suitable for when two wires<br>are combined into one terminal |
|---|---|---|

- Please select the appropriate size pin terminal according to the recommended wire material.
Wire Types for Connectors: AWG16 ~ Applicable wire outer diameter for AWG18 connector:
ø1.0 ~ 1.5mm

#### 4.2.8 Ground terminal

- For better EMC effect, please use 5.3mm 2 /AWG10 special copper conductor cable
- Ground terminal tightening torque

| Drive model | Ground screw |  |
|---|---|---|
|  | Specification | Tightening torque |
| MBDV-520AC<br>-H01 | M4 | 1.4 N.m |

Note:
- Exceeding the maximum tightening torque can cause damage to the screw holes
- Do not install the ground screw with power on, it may cause electrical sparks
- Please check regularly whether the ground screw is loose

<!-- page 31 -->

### 4.3 Main & AUX - Driver power wiring method

MBDV series DC servo has two power supplies.

|  | Pin | Function | Input specification |
|---|---|---|---|
| Main power V | +, V- | Drive main power input | 24 ~ 60VDC |
| Auxiliary Power /<br>24V<br>Control Power | , GND | In the event of a mains power failure, the following two<br>applications require auxiliary power to be turned on:<br>a) When the DSP part of the driver is required to work<br>normally<br>b) Use the dedicated brake output port on the driver to<br>directly drive the brake of the motor<br>When the main circuit resumes power supply, the upper<br>computer controller can quickly restore the position control. | 24VDC±10% |

MBDV Servo Drive

U Red
DC in+ V+

| Yellow |
|---|
| Blue |
| Y/G |

Main
Main Power
rotoM
20~60VDC V
DC GND V-
W

AUX redocnE DC in+
24V
Auxiliary Power
Control Power
24VDC±10% Encoder Interface
DC GND GND

### 4.4 Motor connection method

#### 4.4.1 Block diagram of the connection between the driver and the motor power line

PE
W
V
U

#### 4.4.2 Definition of motor power line wiring

| nal U | V | W | PE |
|---|---|---|---|
| me U phase | V phase | W phase | Motor ground wire |
| lor Red | Yellow | Blue | Yellow /Blue |

Sig
Nam
Co

<!-- page 32 -->

### 4.5 Encoder -the connection method between the driver and the motor

### encoder line

#### 4.5.1 The block diagram of the connection between the driver and the motor encod-

**er line**

#### 4.5.2 Encoder Pinout

总线型编码器 A/B/Z + Hall
SPI type encoder A/B/Z + Hall

2 1 2 1
NC NC U- U+

| 1 |
|---|
| 1 |
| 1 |

4 3 4 3
NC NC V- V+
6 5 6 5

MISO- MISO+ W- W+
8 7 8 7
MOSI- MOSI+ A- A+
10 9 10 9
SCK- SCK+ B- B+
12 11 12 11
CS- CS+ Z- Z+
14 13 14 3
DGND +5V DGND +5V
16 15 16 5
Shield NC Shield NC

| PIN | Bus type encoder | A/B/Z + Hall |
|---|---|---|
|  | Signal | Signal |
| 1 | NC | U+ |
| 2 | NC | U- |
| 3 | NC | V+ |
| 4 | NC | V- |
| 5 | MISO+ | W+ |
| 6 | MISO- | W- |
| 7 | MOSI+ | A+ |
| 8 | MOSI- | A- |
| 9 | SCK+ | B+ |
| 10 | SCK- | B- |
| 11 | CS+ | Z+ |
| 12 | CS- | Z- |
| 13 | 5V | 5V |
| 14 | DGND | DGND |
| 15 | NC | NC |
| 16 | Shield | Shield |

<!-- page 33 -->

#### 4.5.3 Encoder line interface definition

Housing: 501646-1600(Molex)
D Crimp: 501648-1000(Molex)
D
2 1

16 15

| Applicable motor encoder type | Pin | Signal | Color |
|---|---|---|---|
| Communication encoder | 1 | NC | —— |
|  | 2 | NC | —— |
|  | 3 | NC | —— |
|  | 4 | NC | —— |
|  | 5 | MISO+ | Blue |
|  | 6 | MISO- | Blue / Black |
|  | 7 | MOSI+ | Green |
|  | 8 | MOSI- | Green / Black |
|  | 9 | SCK+ | Yellow |
|  | 10 | SCK- | Yellow / Black |
|  | 11 | CS+ | Brown |
|  | 12 | CS- | Brown / Black |
|  | 13 | +5V | Red |
|  | 14 | DGND | Black |
|  | 15 | NC | —— |
|  | 16 | Shield | —— |

**Note: Do not make any connections for undefined pins.**

<!-- page 34 -->

### 4.6 Using an electromagnetic brake

Servo motors are used in loads such as vertical axes. When the motor is not enabled or powered
off, in order to prevent the mechanical mechanism driven by the motor from falling due to gravity
and other reasons, it is necessary to use a servo motor with an electromagnetic brake.
Note: The brake of the servo motor can only be used to maintain the position of the motor when
the motor is not enabled or powered off. Do not use it as a brake during deceleration, otherwise the
motor will be damaged.

#### 4.6.1 Connection diagram

Servo Drive

| Brake |
|---|

1

| Brake |
|---|

Brake
2

#### 4.6.2 Precautions for the use of brake motors

- The special output interface of the driver to control the brake can directly drive the brake of the
motor, and the auxiliary power supply must be connected to provide 24VDC.
- The electromagnetic brake is a normally closed type, and the motor shaft cannot rotate without
power supply to the brake.
- The brake will make a clicking sound during the braking / release action, which will not affect
the use.
- It is recommended to use a cable of 0.5mm 2 (AWG20) or more to prevent the cable from
being too thin and causing voltage drop.
◆ Brake specifications are as follows

| Motor model | Motor<br>Power | Holding<br>torque<br>(Nm) | Continuous<br>current when<br>released<br>(A) | Continuous<br>power<br>consumption<br>when released<br>(W@20℃) | Rated<br>voltage<br>(VDC) | Release<br>time<br>(ms) | Release<br>voltage<br>(VDC) | Braking<br>time<br>(ms) |
|---|---|---|---|---|---|---|---|---|
| SM0402JSL-KCY-BNV<br>SM0402GSL-KCY-BNV | 100W | 0.32 | 0.26 | 6.3 | 24 | < 25 | 18.5 | < 70 |
| SM0601J*L-KCY-BNV<br>SM0601G*L-KCY-BNV | 200W | 1.5 | 0.3 | 7.2 |  | < 25 | 18.5 | < 70 |
| SM0602G*L-KCY-BNV | 400W | 1.5 | 0.3 | 7.2 |  | < 25 | 18.5 | < 70 |
| SM0803G*L-KCY-BNV | 750W | 3.2 | 0.4 | 9.6 |  | < 25 | 18.5 | < 70 |

<!-- page 35 -->

#### 4.6.3 Brake action sequence

Since the brake has an action delay when releasing and braking, in order to avoid damage to the
brake, it is necessary to pay attention to the action sequence during use.

ON

S_ON Signal
OFF
ON
Motor Enable
OFF
ON
Brake Signal
OFF
ON
Brake Release Signal
OFF

ON
Motion Command
OFF

ON
Actual Movement
OFF

Release Delay Time Brake Delay Time
P5-24 P5-25

Release delay and braking delay time can be set using Luna software.

### 4.7 REG- Regenerative energy absorption resistor wiring method

When the torque output of the motor is in the opposite direction of motion, the motor is being back
driven. When the motor is in this state, the motor behaves as a generator and the mechanical
energy of the motor is converted into voltage which is fed back to the driver and power supply.
This results in increased bus voltage for the servo drive and is known as regenerative energy. This
phenomenon is common during the deceleration of high speed applications.
This excessive bus voltage can damage the servo drive as well as the power supply unit of the
system. For this reason, it is common to use a regenerative energy absorption resistor that will
consume any excessive voltage and prevent overvoltage situations.
The MBDV servo family does not have built-in regenerative energy absorption resistors, known as
regen resistors, but allows for external ones to be used. The following shows the recommended
resistor specifications for the MBDV family of drives:

| Driver | Minimum resistance value of<br>external absorption resistor<br>(Ohm) |
|---|---|
| MBDV-520AC | 10 |
| MBDV-2X-520AC | 10 |

<!-- page 36 -->

#### 4.7.1 Calculation method of regenerative energy

**A. Reciprocating motion**
When the motor decelerates, the kinetic energy during deceleration will be converted into electric
energy and fed back to the bus capacitor. The energy during deceleration is divided into two parts:
A) Energy generated when the motor decelerates
B) Energy generated during deceleration of external load
The following provides a simple method to quickly calculate the required regenerative energy
absorption resistor.
**1) Calculate the energy E M when the motor decelerates**
The following table shows the energy generated by the MBDV series servo motor decelerating
from 3000rpm to 0rpm without external load.

| Motor<br>series | Motor<br>Power (W) | Servo motor model | Rotor inertia<br>J (10-4Kgm)<br>M | The energy<br>generated by<br>decelerating from<br>3000rpm to 0rpm<br>E (J)<br>M | Maximum energy absorbed<br>by the driver capacitor<br>E (J)<br>C |
|---|---|---|---|---|---|
| M2 | 100 | SM0402JSL****<br>SM0402GSL**** | 0.043 | 0.21 | 8.7 |
|  | 200 | SM0601J*L****<br>SM0601G*L**** | 0.152 | 0.75 | 8.7 |
|  | 400 | SM0602G*L**** | 0.243 | 1.20 | 13 |
|  | 750 | SM0803G*L**** | 0.856 | 4.22 | 27 |

**2) Calculate the energy EL produced by the dragged load during deceleration**
N times the inertia of the motor, the energy generated when the dragged load decelerates from
3000rpm to 0rpm is:
E =NxE
L M
if E M + E L < E C, that is, during the deceleration process, the energy generated by the motor and
the dragged load is less than the energy absorbed by the drive capacitor, so there is no need to
worry about the problem of regenerative energy absorption.
**3) Calculate the required average power P of the regenerative energy absorption resistor**
**AV**

Note: t is the deceleration time + the interval time between two decelerations
dec

<!-- page 37 -->

**4) Analyze**
When P < 0, that is, the total power generated during the deceleration process is less than the
AV
power absorbed by the bus capacitor in the driver, so no external absorption resistor is required.
When P > 0, that is, the total power generated during the deceleration process is greater than the
AV
power absorbed by the bus capacitor in the driver, and an external absorption resistor is required
to reasonably control the temperature rise of the external absorption resistor. The minimum value
of the resistance power is P / 0.5.
AV
◆ Calculation example: 400W SM0602G*L**** is selected, the load inertia is 15 times the motor
inertia, assuming t (deceleration time + interval time between two decelerations) is 0.5s, and
dec
each movement is decelerated from 3000rpm to 0rpm, then the required power of the absorption
resistor is calculated as:

Since the power absorbed by the bus capacitor in the driver is less than the total power generated
during the deceleration process, an external absorption resistor is required.
**B. The external load torque drives the motor, and the motor continuously outputs negative**
**power.**
Most motors are doing positive work, that is, the torque output direction of the motor is consistent
with the rotation direction. In some special applications, the torque output direction of the motor
is opposite to the rotation speed direction, and the external energy will be fed back to the drive.
such as during the downward, vertical movement of a large load. in order to meet the position
requirements and speed requirements of the servo system, the motor will output the opposite force
to overcome the gravity of the external load. When running in this manner for a long time, the bus
capacitor will become saturated and will fail to continue absorbing regenerative energy. At this time,
the regenerative resistor needs to absorb the energy. The power calculation formula is as follows:

T : Motor output torque. Unit: Nm
M
N Motor speed. Unit: rps
M
◆ Calculation example:
The torque output direction of the motor is opposite to the rotation speed direction. When the motor
output torque is 0.6Nm and the speed is 2400rpm, the power is:

At this time, an external absorption resistor is required, and the minimum power is 150.72
watts. In order to reasonably control the temperature rise of the external absorption resistor, the
recommended minimum power of this resistor is 300 watts.

<!-- page 38 -->

#### 4.7.2 Wiring method

The following is the block diagram depicting how a regen resistor is wired to an MBDV servo drive.

MBDV Servo Drive

Red
U
DC in+ V+

| Yellow |
|---|
| Blue |
| Y/G |

Main
Main Power
rotoM
20~60VDC V
DC GND V-
W

| 24V<br>GND |
|---|
| 1<br>REG<br>2 |
| 1<br>2 |

Auxiliary Power DC in+
Control Power
24VDC±10% DC GND Encoder Interface

Regenerative
resistor

#### 4.7.3 Device setting parameters

Related parameters

| Parameter | Instruction | Name | Value range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P1-19 | ZR | Regenerative<br>resistor resistance | 10 ~ 32000 | 10 | Ohm | Set the resistance value of the regenerative<br>energy absorption resistor |
| P1-20 | ZC | Regenerative<br>resistor power<br>rating | 1 ~ 32000 | 40 | Watt | Set the power of the regenerative energy<br>absorption resistor |
| P1-21 | ZT | Regenerative<br>resistor time<br>constant | 0 ~ 8000 | 1000 | ms | Sets the sustainable absorption time of the<br>regenerative energy absorption resistor |

**Note:**
Please set the resistance value, power and absorption time of the absorption resistor correctly,
otherwise it will affect the use of this function and cause the drive to generate alarms such as
overvoltage and regenerative energy absorption failure.
When connecting an external absorbing resistor, please make sure that the total resistance value
cannot be less than the minimum allowable resistance value of the driver. If multiple resistors are
used in series or in parallel, please calculate the total resistance and total power correctly.

| External: 100, 200W resistor | Parameter setting:<br>P1-19 = 100<br>P1-20 = 200 |
|---|---|
| External: Two 50, 200W resistors in series | Parameter setting:<br>P1-19 = 100<br>P1-20 = 400 |
| External: Two 100, 200W resistors in<br>parallel | Parameter setting:<br>P1-19 = 50<br>P1-20 = 400 |

<!-- page 39 -->

### 4.8 Communication Interface

#### 4.8.1 USB- Host computer debugging interface

The USB port is used for establashing communication between the MBDV servo drive and the PC
based Luna software. Luna software allows users to set control modes, modify parameters, tune
the PID loop and perform other configuration tasks.

| Pin | Logo | Function |
|---|---|---|
| 1 | +5V | USB power |
| 2 | D- | Data- |
| 3 | D+ | Data + |
| 4 | — | Reserve |
| 5 | GND | Power ground |

#### 4.8.2 Wireless - Host computer wireless debugging interface

Wireless The port is used for the drive to connect the wireless debugging module to communicate
with the PC. Using Luna software, you can set the control mode, modify parameters, online
automatic tuning and other operations.
**Wireless**

GNDTX +5V R X

| Pin | Logo | Function |
|---|---|---|
| 1 | GND | Power ground |
| 2 | TX | Data sending |
| 3 | +5V | Power |
| 4 | RX | Data reception |

#### 4.8.3 COM1/2 – CANOpen / RS485 Communication interface

COM1 and COM 2 are used to connect the MBDV drive to an external controller communicating
over CANopen or Modbus RTU.
COM1 COM2

1 5 1 5

| COM1/2 Pin | CANopen | RS485 |
|---|---|---|
| 1 | NC | RS485+ |
| 2 | NC | RS485- |
| 3 | CAN_H | NC |
| 4 | CAN_L | NC |
| 5 | GND | GND |

<!-- page 40 -->

### 4.9 I/O Input and output signal wiring (MBDV-520AC)

#### 4.9.1 I/O Input and output signal specifications

I/O port of MBDV-520AC low-voltage servo driver is used to connect input and output signals. The
pin definitions are as follows:

2 1
AOUT- AOUT+

4 3
BOUT- BOUT+
6 5

ZOUT- ZOUT+
8 7
GND GND
10 9
X2 X1
12 11
X4 X3
14 13
YCOM XCOM
16 15
Y2 Y1

The input and output signal specifications are as follows:

|  | Classification | Description |
|---|---|---|
| Digital<br>Signals | Inputs | 4 optically isolated inputs, can be configured by parameters, 24VDC, maximum current<br>20mA |
|  | Outputs | 2 optically isolated outputs, can be configured by parameters, maximum 30 VDC, maximum<br>output current 30 mA |
| Pulse<br>Signals | Outputs | 3 line driver outputs: Encoder A/B/Z signals |

#### 4.9.2 I/O Signal Pin Block Diagram

DGND 7 16 Y2

DGND 8

15 Y1

| 14 |
|---|

X1 9

2.4KΩ
YCOM

X2 10 2.4KΩ

| 11 | 2.4KΩ |
|---|---|
|  | 2.4KΩ |

1 AOUT+
10Ω

X3 2 AOUT-
10Ω

3 BOUT+
2.4KΩ 10Ω 编码器
X4 12 反馈输出
2.4KΩ 4 BOUT-

| 13 |
|---|

10Ω
XCOM
2.4KΩ 5 ZOUT+
10Ω

6 ZOUT-
10Ω

<!-- page 41 -->

#### 4.9.3 Input and Output Pinout

| Pin NO. | Signal | Description |
|---|---|---|
| 1 | AOUT+ | Encoder output A+ |
| 2 | AOUT- | Encoder output A- |
| 3 | BOUT+ | Encoder output B+ |
| 4 | BOUT- | Encoder output B- |
| 5 | ZOUT+ | Encoder output Z+ |
| 6 | ZOUT- | Encoder output Z- |
| 7 | DGND | Digital ground |
| 8 | DGND | Digital ground |
| 9 | X1 | Digital input 1 |
| 10 | X2 | Digital input 2 |
| 11 | X3 | Digital input 3 |
| 12 | X4 | Digital input 4 |
| 13 | XCOM | Digital input common terminal |
| 14 | YCOM | Digital output common terminal |
| 15 | Y1 | Digital output 1 |
| 16 | Y2 | Digital output 2 |

Note: MBDV-520AC does not support encoder frequency division output function, if you need this
function, please select MBDV-520AC-H01.

##### 4.9.3.1 Encoder Feedback Output

The MBDV-520AC can output A/B/Z phase signals via line driver, differential outputs with a
maximum of 5 V.
The host computer must use a differential line receiver to be able to accept the signals from the
drive. We recommend that a twisted pair, shielded wire be used for the transmission of encoder
feedback signals.

| I/O - pin NO. | Signal name |  | Description |
|---|---|---|---|
| 1 | AOUT+ | Encoder Feedback<br>Outputs | The encoder feedback is output in the form of<br>A/B/Z differential signals. A/B/Z output signals<br>and the frequency division ratio that affects the<br>output frequency of the encoder feedback can<br>be configured via parameters. |
| 2 | AOUT- |  |  |
| 3 | BOUT+ |  |  |
| 4 | BOUT- |  |  |
| 5 | ZOUT+ |  |  |
| 6 | ZOUT- |  |  |
| 7,8 | DGND |  |  |

<!-- page 42 -->

◆ A/B/Z differential signal connection example

Servo Drive Master
1 AOUT+ A+

| AOUT- | A- |
|---|---|
| BOUT+ | B+ |
| BOUT- | B- |
| ZOUT+ | Z+ |
| ZOUT- | Z- |
| DGND | DGND |

10Ω

2
10Ω

3
10Ω

4
10Ω

5
10Ω

6
10Ω
7,8

FG FG

Note: Make sure to connect the host computer to the digital ground of the drive.

##### 4.9.3.2 Digital Inputs

The MBDV-520AC low voltage servo driver has 4 digital input signals. Each digital input signal can
be configured to have a special fixed function. If a fixed function is assigned to an input, you may
also configure the logic of the input.
◆ Predefined Functions
For example, alarm clear, limit sensor input, origin sensor input, emergency stop, etc.
◆ General Purpose Function
As a general-purpose input signal, there is no specific function，and the input may be used for
application specific purposes.

| Signal |  |  |  |  | Factory default |  |  |
|---|---|---|---|---|---|---|---|
| I/O pin NO. | Signal name | Signal description | Corresponding<br>parameters | Instruction | Signal name | Input logic*1 | Defaults |
| 9 | X1 | Digital input 1 | P5-00 | MU1 | CCW-LMT | Closed | 7 |
| 10 | X2 | Digital input 2 | P5-01 | MU2 | CW-LMT | Closed | 5 |
| 11 | X3 | Digital input 3 | P5-02 | MU3 | HOM-SW | Closed | 39 |
| 12 | X4 | Digital input 4 | P5-03 | MU4 | E-STOP | Closed | 13 |
| 13 | XCOM | Digital input COM | - | - | X1, X2, X3, X4 input common terminal |  |  |

<!-- page 43 -->

**Note:**
*1. The definition of logic states for inputs and outputs is as follows:
Closed: The driver digital input circuit forms a loop, and current flows in or out of the input pins.
Open: The driver digital input circuit does not form a loop and no current flows into or out of the
input pins.
2. For details, please see: 7.1.1 Input signal setting
◆ Digital Input and Output Wiring Instructions (MBDV-520AC)
The MBDV-520AC has 4 optically isolated, single-ended inputs with a common voltage (COM)
point These inputs require that they be powered separately. If using a PLC, you may use the 24 V
output from the PLC. If connected to a relay or mechanical switch, a 24 VDC power supply will be
required. The maximum withstand current for each input is 20 mA.What is COM?
Common (COM) represents an a common voltage level for inputs (XCOM) or outputs (YCOM). If
using sourcing signals (PNP), COM should be connected to ground (power source negative). If
using sinking signals (NPN), COM should be connected to positive voltage (positive probe from
power source).
◆ The internal circuit block diagram of X1 ~ X4 is shown in the figure below.

X1 9

2.4KΩ

2.4KΩ
X2 10

| 2.4KΩ |
|---|

X3 11 2.4KΩ

| 2.4KΩ |
|---|

2.4KΩ
X4 12

| 13 | 2.4KΩ |
|---|---|

XCOM
2.4KΩ

<!-- page 44 -->

◆ Digital input X1 ~X4 Wiring example

| A. Controller---SINK mode | B. Controller---Sourcing mode |
|---|---|
| 5~24VDC<br>Master/Controller Servo Drive<br>XCOM<br>2.4K<br>X1 ~ X4<br>2.4K<br>0VDC | Master/Controller<br>5~24VDC<br>Servo Drive<br>X1 ~ X4<br>2.4K<br>XCOM<br>2.4K<br>0VDC |
| C. Connect to Relay or Switch(XCOM connect to +24V) | D. Connect to Relay or Switch(XCOM connect to 0V) |
| Servo Drive<br>24VDC XCOM<br>Power Supply 2.4K<br>X1 ~ X4<br>0V<br>2.4K<br>Relay or Switch | Servo Drive<br>XCOM<br>0V<br>2.4K<br>X1 ~ X4<br>24VDC<br>Power Supply 2.4K<br>Relay or Switch |
| E. Connect a PNP sensor | F. Connect a NPN sensor |
| 5~24VDC<br>Servo Drive<br>PNP Signal out X1 ~ X4<br>Sensor<br>2.4K<br>XCOM<br>2.4K<br>0VDC | 5~24VDC<br>Servo Drive<br>XCOM<br>2.4K<br>NPN<br>Signal out X1 ~ X4<br>Sensor<br>2.4K<br>0VDC |

##### 4.9.3.3 Digital Outputs

The MBDV-520AC low voltage servo driver has 2 digital output signals with a common voltage
(COM) point. Each digital output signal can be configured to have a special fixed function. If a fixed
function is assigned to an output, you may also configure the logic of the output.
The maximum input voltage for the outputs is 30 VDC, and the maximum withstand current is 100
mA per output.

| Signal |  |  |  |  | Factory default |  |  |
|---|---|---|---|---|---|---|---|
| CN2-pin<br>number | Signal name | Signal descripti | Corresponding<br>on<br>parameters | Instruction | Signal name | Output logic<br>*1 | Defaults |
| 15 | Y1 | Digital output 1 | P5-12 | MO1 | ALM | Open | 2 |
| 16 | Y2 | Digital output 2 | P5-13 | MO2 | SON-ST | Closed | 7 |
| 14 | YCOM | Digital output CO | M - |  | Y1, Y2 Output common |  |  |

<!-- page 45 -->

◆ The internal circuit block diagram of Y1 ~ Y2 are as shown below.

Y2 16

Y1 15

| 14 |
|---|

YCOM 14

◆ Y1 ~ Y2 connection examples

| A. Connected to the control's optocoupler | B. Connected to resistor load |
|---|---|
| Master/Controller<br>24VDC<br>Servo Drive<br>Y2 16<br>Y1 15<br>YCOM 14<br>0VDC | Servo Drive<br>24VDC<br>Y1 / Y2<br>YCOM<br>0VDC |
| C.Connected to inductive load(Relay.etc.) |  |
| 24VDC<br>Servo Drive<br>Y1 / Y2<br>Relay<br>1. Be sure to install the YCOM<br>freewheeling diode,<br>Otherwise, the output port will<br>be damaged<br>2. Pay attention to the<br>freewheeling diode polarity 0VDC |  |

| Y1 |  |
|---|---|
|  | 14 |

### 4.10 Input and Output Signals (MBDV-2X-520AC)

#### 4.10.1 Input and Output Specifications

I/O port of MBDV-2X-520AC low-voltage servo driver is used to connect input and output signals.
The pin definitions are as follows:
2 1
1_X2 1_X1

4 3
1_X4 1_X3
6 5

1_YCOM 1_XCOM
8 7
1_Y2 1_Y1
10 9
2_X2 2_X1
12 11
2_X4 2_X3
14 13
2_YCOM 2_XCOM
16 15
2_Y2 2_Y1

The input and output signal specifications are as follows:

<!-- page 46 -->

| Axis | Classification |  | Describe |
|---|---|---|---|
| 1 | Digital<br>signals | Inputs | 4 optically isolated inputs, configurable via parameters, max 24 VDC, max withstand<br>current of 20 mA |
|  |  | Outputs | 2 optically isolate outputs, configurable via parameters, max 30 VDC, max withstand<br>current of 100 mA |
| 2 | Digital<br>signals | Inputs | 4 optically isolated inputs, configurable via parameters, max 24 VDC, max withstand<br>current of 20 mA |
|  |  | Outputs | 2 optically isolate outputs, configurable via parameters, max 30 VDC, max withstand<br>current of 100 mA |

#### 4.10.2 I/O Signal Pin Block Diagram

1_X1 1 9 2_X1

2.4KΩ 2.4KΩ

1_X2 2 2.4KΩ 2.4KΩ 10 2_X2

2.4KΩ 2.4KΩ

1_X3 3 2.4KΩ 2.4KΩ 11 2_X3

2.4KΩ 2.4KΩ

2.4KΩ 2.4KΩ
1_X4 4 12 2_X4

2.4KΩ 2.4KΩ

1_XCOM 5 13 2_XCOM
2.4KΩ 2.4KΩ

1_Y2 8 16 2_Y2

1_Y1 7 15 2_Y1

1_YCOM 6 14 2_YCOM

<!-- page 47 -->

#### 4.10.3 Input and Output Pinout

| Pin NO. | Signal | Description |
|---|---|---|
| 1 | 1_X1 | Axis1_input 1 |
| 2 | 1_X2 | Axis1_input 2 |
| 3 | 1_X3 | Axis1_input 3 |
| 4 | 1_X4 | Axis1_input 4 |
| 5 | 1_XCOM | Axis1_input COM |
| 6 | 1_YCOM | Axis1_output COM |
| 7 | 1_Y1 | Axis1_output 1 |
| 8 | 1_Y2 | Axis1_output 2 |
| 9 | 2_X1 | Axis2_input 1 |
| 10 | 2_X2 | Axis2_input 2 |
| 11 | 2_X3 | Axis2_input 3 |
| 12 | 2_X4 | Axis2_input 4 |
| 13 | 2_XCOM | Axis2_input COM |
| 14 | 2_YCOM | Axis2_output COM |
| 15 | 2_Y1 | Axis2_output 1 |
| 16 | 2_Y2 | Axis2_output 2 |

##### 4.10.3.1 Digital Inputs

The MBDV-2X-520AC low voltage servo driver has 4 digital input signals for each axis. Each digital
input signal can be configured to have a special fixed function. If a fixed function is assigned to an
input, you may also configure the logic of the input.

| Signal |  |  |  |  | Factory default |  |  |
|---|---|---|---|---|---|---|---|
| I/O-pin<br>number | Signal<br>name | Signal description | Corresponding<br>parameters | Instruction | Signal name | Input logic<br>*1 | Defaults |
| 1 | 1_X1 | Axis1_input 1 | P5-00 | MU1 | GPIN | Closed | 0 |
| 2 | 1_X2 | Axis1_input 2 | P5-01 | MU2 | GPIN | Closed | 0 |
| 3 | 1_X3 | Axis1_input 3 | P5-02 | MU3 | GPIN | Closed | 0 |
| 4 | 1_X4 | Axis1_input 4 | P5-03 | MU4 | E-STOP | Closed | 13 |
| 5 | 1_XCOM | Axis1_input COM | —— | —— | X1, X2, X3, X4 input common terminal |  |  |
| 9 | 2_X1 | Axis2_input 1 | P5-00 | MU1 | GPIN | Closed | 0 |
| 10 | 2_X2 | Axis2_input 2 | P5-01 | MU2 | GPIN | Closed | 0 |
| 11 | 2_X3 | Axis2_input 3 | P5-02 | MU3 | GPIN | Closed | 0 |
| 12 | 2_X4 | Axis2_input 4 | P5-03 | MU4 | E-STOP | Closed | 13 |
| 13 | 2_XCOM | Axis2_input COM | —— | —— | X1, X2, X3, X4 input common terminal |  |  |

**Note:**
*1. The level logic of the pin input is as follows:
Closed: The driver digital input circuit forms a loop, and current flows in or out of the input pins.
Open: The driver digital input circuit does not form a loop, and no current flows into or out of the input pins.
2. For details, please see: 7.1.1 Input signal setting

<!-- page 48 -->

◆ Digital Input and Output Wiring Instructions
The MBDV-2X-520AC has 4 optically isolated, single-ended inputs with a common voltage (COM)
point for each axis. These inputs require that they be powered separately. If using a PLC, you may
use the 24 V output from the PLC. If connected to a relay or mechanical switch, a 24 VDC power
supply will be required. The maximum withstand current for each input is 20 mA.
**What is COM?**
Common (COM) represents an a common voltage level for inputs (XCOM) or outputs (YCOM). If
using sourcing signals (PNP), COM should be connected to ground (power source negative). If
using sinking signals (NPN), COM should be connected to positive voltage (positive probe from
power source).
◆ The internal block diagram of X1-X4, for each axis, is shown in the figure below.

1_X1 1 9 2_X1

2.4KΩ 2.4KΩ

1_X2 2 2.4KΩ 2.4KΩ 10 2_X2

2.4KΩ 2.4KΩ

1_X3 3 2.4KΩ 2.4KΩ 11 2_X3

2.4KΩ 2.4KΩ

2.4KΩ 2.4KΩ
1_X4 4 12 2_X4

2.4KΩ 2.4KΩ

1_XCOM 5 13 2_XCOM
2.4KΩ 2.4KΩ

<!-- page 49 -->

◆ Digital input X1 ~X4 Wiring example

| A. Controller---SINK mode | B. Controller---Sourcing mode |
|---|---|
| 5~24VDC<br>Master/Controller Servo Drive<br>XCOM<br>2.4K<br>X1 ~ X4<br>2.4K<br>0VDC | Master/Controller<br>5~24VDC<br>Servo Drive<br>X1 ~ X4<br>2.4K<br>XCOM<br>2.4K<br>0VDC |
| C. Connect to Relay or Switch(XCOM connect to +24V) | D. Connect to Relay or Switch(XCOM connect to 0V) |
| Servo Drive<br>24VDC XCOM<br>Power Supply 2.4K<br>X1 ~ X4<br>0V<br>2.4K<br>Relay or Switch | Servo Drive<br>XCOM<br>0V<br>2.4K<br>X1 ~ X4<br>24VDC<br>Power Supply 2.4K<br>Relay or Switch |
| E. Connect a PNP sensor | F. Connect a NPN sensor |
| 5~24VDC<br>Servo Drive<br>PNP Signal out X1 ~ X4<br>Sensor<br>2.4K<br>XCOM<br>2.4K<br>0VDC | 5~24VDC<br>Servo Drive<br>XCOM<br>2.4K<br>NPN<br>Signal out X1 ~ X4<br>Sensor<br>2.4K<br>0VDC |

<!-- page 50 -->

##### 4.10.3.2 Digital output Y1 ~ Y2

The MBDV-520AC low voltage servo driver has 2 digital output signals with a common voltage
(COM) point per axis. Each digital output signal can be configured to have a special fixed function.
If a fixed function is assigned to an output, you may also configure the logic of the output.
Maximum withstand voltage is 30VDC, and the current is 100mA.

| Signal |  |  |  |  | Factory default |  |  |
|---|---|---|---|---|---|---|---|
| I/O-pin<br>number | Signal name | Signal description | Corresponding<br>parameters | Instruction | Signal name | Output logic | Defaults |
| 7 | Y1 | Axis1_output 1 | P5-12 | MO1 | ALM | Open | 2 |
| 8 | Y2 | Axis1_output 2 | P5-13 | MO2 | SON-ST | Closed | 7 |
| 6 | YCOM | Axis1_output COM | - |  | Y1, Y2 output common |  |  |
| 15 | Y1 | Axis2_output 1 | P5-12 | MO1 | ALM | Open | 2 |
| 16 | Y2 | Axis2_output 2 | P5-13 | MO2 | SON-ST | Closed | 7 |
| 14 | YCOM | Axis2_output COM | - |  | Y1, Y2 output common |  |  |

◆ The internal circuit block diagrams of Y1-Y2 for each axis are shown below

1_Y2 8 16 2_Y2

1_Y1 7 15 2_Y1

| 14 |
|---|

1_YCOM 6 2_YCOM

<!-- page 51 -->

◆ Connection examples

| A. Connected to the control's optocoupler | B. Connected to resistor load |
|---|---|
| Master/Controller<br>24VDC<br>Servo Drive<br>Y2 16<br>Y1 15<br>YCOM 14<br>0VDC | Servo Drive<br>24VDC<br>Y1 / Y2<br>YCOM<br>0VDC |
| C.Connected to inductive load(Relay.etc.) |  |
| 24VDC<br>Servo Drive<br>Y1 / Y2<br>Relay<br>1. Be sure to install the YCOM<br>freewheeling diode,<br>Otherwise, the output port will<br>be damaged<br>2. Pay attention to the<br>freewheeling diode polarity 0VDC |  |

| Y1 |  |
|---|---|
|  | 14 |

#### 4.10.4 Encoder Feedback Output

The MBDV-2X-520AC can output A/B/Z phase signals via line driver, differential outputs with a
maximum of 5 V. The driver is able to output encoder feedback for each axis.
The host computer must use a differential line receiver to be able to accept the signals from the
drive. We recommend that a twisted pair, shielded wire be used for the transmission of encoder
feedback signals.
2 1
1_AOUT- 1_AOUT+

4 3
1_BOUT- 1_BOUT+
6 5
1_ZOUT- 1_ZOUT+
8 7
DGND DGND
10 9
2_AOUT- 2_AOUT+
12 11
2_BOUT- 2_BOUT+
14 13
2_ZOUT- 2_ZOUT+

| Encoder - Pin NO. | Signal name |  | Description |
|---|---|---|---|
| 1 | 1_AOUT+<br>1_AOUT-<br>1_BOUT+<br>1_BOUT-<br>1_ZOUT+<br>1_ZOUT- | Axis 1 Encoder<br>Feedback Outputs | The encoder feedback is output in the form of<br>A/B/Z differential signals. The number of pulses per<br>revolution can be configured via the Pulse Output<br>Gear Ratio. |
| 2 |  |  |  |
| 3 |  |  |  |
| 4 |  |  |  |
| 5 |  |  |  |
| 6 |  |  |  |
| 7,8 | DGND | Digitally |  |
| 9 | 2_AOUT+<br>2_AOUT-<br>2_BOUT+<br>2_BOUT-<br>2_ZOUT+<br>2_ZOUT- | Axis 2 Encoder<br>Feedback Outputs | The encoder feedback is output in the form of<br>A/B/Z differential signals. The number of pulses per<br>revolution can be configured via the Pulse Output<br>Gear Ratio. |
| 10 |  |  |  |
| 11 |  |  |  |
| 12 |  |  |  |
| 13 |  |  |  |
| 14 |  |  |  |

<!-- page 52 -->

◆ A/B/Z phase connection example

伺服驱动器 上位机

AOUT+ A+

| AOUT- | A- |
|---|---|
| BOUT+ | B+ |
| BOUT- | B- |
| ZOUT+ | Z+ |
| ZOUT- | Z- |
| DGND | DGND |

10Ω

10Ω

10Ω

10Ω

10Ω

10Ω

FG FG

Note: Make sure to connect the host computer to the digital ground of the drive.

### 4.11 STO - Safe torque off function

The MBDV series servo drives support Safe Torque Off functionality. Each drive comes with an
STO port for users to connect their STO trigger mechanism to.
Safe Torque Off is a hardware level safety function. While the STO function is enabled, the
physical connection between motor and drive is disabled, thereby preventing the motor from
being energized. This function is meant to protect personnel as well as equipment in emergency
situations.
When the STO function is enabled, the servo motor becomes disabled, the drive enters an alarm
state and the LED display on the driver panel display the alarm code " ".

#### 4.11.1 Safe Torque Off Precautions

1) If STO is not required in the application, the premade STO connector provided with the drive
should be connected in the STO port on the drive.
2) Before using the STO function, users should ensure that they are familiar with how STO works
and the scenarios in which it is appropriate to use it.
3) When the STO function is working, due to the existence of external force (such as vertical shaft
load), the motor will rotate due to the external force. Therefore, please make sure to use the servo
motor with brake in this case, and connect the brake control circuit correctly.
4) When the STO function is triggered, the motor's shaft rotates freely. If the motor is in motion or
under the influence of an external load, its stopping distance will be affected by the inertia of the
system.
5) While the STO function is enabled, the output power from drive to motor is cut off. However, the
power supply powering the servo drive remains on. If users need to troubleshoot the servo drive
and motor, they must ensure that the power supply powering the entire system (motor and drive) is
powered off.
6) While the STO function is enabled, the drive will be in an alarm state and the motor will be
disabled.
7) When the STO function is disabled (no longer in use), the STO alarm status of the drive is
automatically cleared. The drive will also output a Servo Ready signal but the motor will be
disabled.

<!-- page 53 -->

#### 4.11.2 STO Function Connector

##### 4.11.2.1 STO Internal Circuit Diagram

The pinout of the STO connector is as follows:

##### 4.11.2.2 STO Signal Definitions

The pin definition of driver CN5 is as follows:

Connector and terminal models are as follows:

|  | Model | manufacturer |
|---|---|---|
| Connector | 43025-1000 | MOLEX |
| PIN | 43030-0005 | MOLEX |

##### 4.11.2.3 STO Signal Definitions

STO function input and output signals are as follows

| Signal name | Logo | Pin | Description | Applicable mode |
|---|---|---|---|---|
| Safety signal<br>input 1 | SF1+ | 1 | When the SF1 signal is removed, that is, SF1 is off, the STO<br>function is activated (enabled). | All control<br>modes |
|  | SF1- | 5 |  |  |
| Safety signal<br>input 2 | SF2+ | 3 | When the SF2 signal is removed, that is, SF2 is off, the STO<br>function is activated (enabled). |  |
|  | SF2- | 2 |  |  |
| Safety signal<br>output | EDM+ | 6 | When the STO function works, this signal is output |  |
|  | EDM- | 4 |  |  |
| Digital Ground | DGND | 7,8 | +5VDC power ground |  |
| +5V | +5V | 9,10 | +5VDC power source |  |

Note: When any safety input SF1 and SF2 are OFF, the STO function will start to work.

<!-- page 54 -->

##### 4.11.2.4 STO connection example

◆ Connection with safety switch

Safety Switch
Servo Drive

Safety Switch 1

Safety Switch 2

Safety Output

◆ Connection with safety light curtains

24VDC

Servo Drive

Safety Switch 1

Safety Switch 2

0VDC

Safety Output

<!-- page 55 -->

## 5 LED Display

### 5.1 Display content

#### 5.1.1 Decimal point meaning

| Display content | Description |
|---|---|
| Enable identification bit | ◆ Motor Enabled Flag: The decimal point in the lower right corner of the<br>LED panel is the identification bit alerting users to the servo motor's status<br>Solid: Motor enabled<br>Blank: Motor is disabled |

### 5.2 Alarm Codes and Definitions

When the MBDV drive experiences abnormal operation, it will enter an alarm state and display one
of the following codes. Their definitions are included below for reference.

| Alarm Code | Description | Alarm<br>Type | Drive status after alarm |
|---|---|---|---|
|  | Drive over temperature | Fault | Servo off |
|  | Internal voltage | Fault | Servo off |
|  | Drive overvoltage | Fault | Servo off |
|  | Overcurrent | Fault | Servo off |
|  |  | Fault | Servo off |
|  |  | Fault | Servo off |
|  | Encoder feedback error | Fault | Servo off |
|  | Position following error | Fault | Servo off |
|  | Low voltage | Fault | Servo off |
|  | Over speed | Fault | Servo off |
|  | Limit switch triggered | Warning | Does not change the current state |
|  | Positive limit switch triggered | Warning | Does not change the current state, the motor cannot<br>rotate negatively. |
|  | Positive limit switch triggered | Warning | Does not change the current state, the motor cannot<br>rotate positively. |
|  | Current limit | Warning | Does not change the current state |
|  | Communication error | Warning | Does not change the current state |
|  | Parameter save failed | Warning | Does not change the current state |

<!-- page 56 -->

| Alarm Code | Description | Alarm<br>Type | Drive status after alarm |
|---|---|---|---|
|  | STO Active | Warning | Servo off |
|  | Regeneration failed | Fault | Servo off |
|  | Undervoltage warning | Warning | Does not change the current state |
|  | Empty Q Program | Warning | Does not change the current state |
|  | Motion command received while<br>motor disabled | Warning | Does not change the current state |
|  | Internal voltage error | Fault | Servo off |
|  |  | Fault | Servo off |
|  | Emergency stopped | Warning | Motor decelerates to stop |
|  | Memory error | Fault | Servo off |
|  | Motor over temperature | Fault | Servo off |
|  | Drive Processor over temperature | Fault | Servo off |
|  | Motor stalled | Fault | Servo off |
|  | Homing parameters configuration<br>error | Warning | Does not change the current state |
|  | Motor collision alarm | Fault | Servo off |
|  | Encoder communication error | Fault | Servo off |
|  | IO signal used is not general<br>purpose | Warning | Does not change the current state |
|  | Bus watchdog trigger | Warning | Does not change the current state |

<!-- page 57 -->

## 6 Commissioning

During trial operation, it is recommended that the motor be operated without a load coupled to its
shaft.

### 6.1 Inspection before commissioning

To protect the servo drive and servo motor, it is recommended that users inspect the following
before commissioning these two components.
1) Wiring
Check that Main power, AUX power (if used), motor phases and motor encoder are securely and
properly wired to the drive. Check that no short circuits exist between connections. Verify that
motor and drive are properly grounded . Ensure that the USB connection is secure to ensure
proper communication with the host PC.
2) Power supply voltage
Verify that the voltage applied to Main power is within the specification of the drive. Verify that the
voltage applied to AUX power is 24 VDC.
3) Make sure the motor and drive are securely mounted
4) Make sure the motor shaft is not loaded

### 6.2 Commissioning Steps

| Step | Content | Description |
|---|---|---|
| 1 | Securely mount the motor | 1) Servo motor can be mounted on the machine<br>2) Please do not connect the load to the servo motor |
| 2 | Ensure proper and secure<br>connection between motor and<br>drive | 1) MBDV Servo motor phases are labeled U, V, W for the red, yellow and blue<br>cables respectively. If using a third party motor, this wiring could be different and<br>its performance is not guaranteed with the MBDV.<br>2) Ensure that the motor encoder cable is properly connected to the encoder<br>port on the MBDV drive. |
| 3 | Make sure the power circuit wiring<br>is correct | Refer to chapter 4.3 Wiring of External Main Circuit to confirm whether the<br>power input circuit is correct |
| 4 | Power on | Input 24 ~60VDC power supply |
| 5 | During normal operation, the drive<br>displays its Node ID<br>If an alarm occurs, it will display<br>the alarm code in blinking fashion | 1) Normally, the drive has no alarm display and is disabled<br>2) If r09 alarm occurs, it indicates that there is a problem with the cable<br>connection of the encoder. Please check whether the wiring is correct after<br>power off.<br>3) For other alarms, please refer to Chapter 9 Troubleshooting |
| 6 | If a motor brake is used, the brake<br>control circuit should be set up<br>before use. | Refer to 4.6 Connection of Motor with Electromagnetic Brake |
| 7 | JOG mode operation | If there is no abnormality in the above steps, you can start a trial operation in<br>JOG mode |

Note: The drive must be configured for the proper motor model. Please refer to the following
steps before initiating motion with the motor.

<!-- page 58 -->

### 6.3 Drive Configuration

In order to integrate the MBDV drive and motor into a system, it is necessary for users to configure
the drive so that its inputs, outputs, feedback and motor interact with the system in the desired
way. The following are examples of things users can configure:
1. Control Mode
2. Assign functions to inputs and outputs
3. Tune PID loop parameters for improved motor control (if necessary)
Connection method

Install the setup software. Please go
to our website to download:
www.moons.com.cn

For detailed instructions of Luna, contact Applied Motion support..

<!-- page 59 -->

## 7 Control Modes and Functions

### 7.1 I/O signal setting

Input and output signals can be assigned pre-defined functions (e.g. CW/CCW limits, Fault
Output, In Position etc.), can be configured as general purpose and can have their logic state
configured according to application requirements. Parameters provided in this section's tables are
referencing the "Parameter Table" in the Luna configuration software.

#### 7.1.1 Input Signal Configuration

##### 7.1.1.1 Assignable input functions

The functions and logic that can be assigned to the input signal are listed below.

| Signal Functions | Symbol | Setup value and corresponding input logic<br>state |  |
|---|---|---|---|
|  |  | Valid when closed | Valid when open |
| General Purpose Input | GPIN | 0 | - |
| Servo On | S-ON | 1 | 2 |
| Alarm Reset | A-CLR | 3 | 4 |
| CW Limit | CW-LMT | 5 | 6 |
| CCW Limit | CCW-LMT | 7 | 8 |
| Gain Select | GAIN-SEL | 11 | 12 |
| Emergency stop | E-STOP | 13 | 14 |
| Start Homing | S-HOM | 15 | 16 |
| Torque Limit | TQ-LMT | 19 | 20 |
| Zero Speed Clamp | ZCLAMP | 21 | 22 |
| Speed limit select | V-LMT | 37 | 38 |
| Home Switch | HOM-SW | 39 | 40 |
| Start Q program | START-Q | 45 | 46 |

The definitions of logic states for inputs are as follows:
Closed: The input's internal circuit is completed, and current flows in or out of the input pin.
Open: The input's internal circuit is not completed, and current does NOT flow in or out of the input
pin.

##### 7.1.1.2 Default input functions

The default functions of inputs signals are listed below. The parameters to configure each
respective input are also listed.

| Signal |  |  |  | Factory default |  |  |
|---|---|---|---|---|---|---|
| Input NO. | Signal description | Parameters | Command | Signal name | Input Logic | Defaults |
| X1 | Digital input 1 | P5-00 | MU1 | CCW-LMT | Closed | 7 |
| X2 | Digital input 2 | P5-01 | MU2 | CW-LMT | Closed | 5 |
| X3 | Digital input 3 | P5-02 | MU3 | HOM-SW | Closed | 39 |
| X4 | Digital input 4 | P5-03 | MU4 | E-STOP | Closed | 13 |
| XCOM | Digital input COM terminal | - | - | X input common |  |  |

<!-- page 60 -->

#### 7.1.2 Output signal configuration

##### 7.1.2.1 Assignable output functions

The functions and logic assignable to outputs are list below:

| Signal name | Signal Symbol | Logic and set value when output signal is<br>valid |  |
|---|---|---|---|
|  |  | Output when the<br>signal is valid<br>Closed | Output when the<br>signal is valid<br>Open |
| General Purpose Output | Output when the<br>signal is valid | 0 | - |
| Fault Output | ALM | 1 | 2 |
| Warning Output | WARN | 3 | 4 |
| Brake Release Output | BRK | 5 | NONE |
| Servo-on Status Output | SON-ST | 7 | 8 |
| In-position Output | IN-POS | 9 | 10 |
| Dynamic Position Output | DYM-LMT | 11 | 12 |
| Torque Reach Output | TQ-REACH | 13 | 14 |
| Torque Limit Output | T-LMT | 15 | 16 |
| Speed Coincidence Output | V-COIN | 17 | 18 |
| Speed Reaches Output | AT-SPD | 19 | 20 |
| Velocity Limit Output | V-LMT | 21 | 22 |
| Servo Ready Output | S-RDY | 23 | 24 |
| Homing Finished Output | HOMED | 25 | 26 |
| Near Target Position Output | P-COIN | 31 | 32 |
| Zero Speed Detected | Z-SPD | 33 | 34 |
| Torque Coincidence output | I-COIN | 35 | 36 |

The definitions of logic states for outputs are as follows:
Closed: The output's internal circuit is completed, and current flows in or out of the output pin.
Open: The output's internal circuit is not completed, and current does NOT flow in or out of the
output pin.

##### 7.1.2.2 Default output functions

The default functions for outputs are listed below. The parameters for configuring each respective
output are also listed.

| Signal |  |  |  | Factory default |  |  |
|---|---|---|---|---|---|---|
| Signal name | Description | Parameters | Command | Signal name | Output logic | Defaults |
| Y1 | Digital output 2 | P5-12 | MO1 | ALM | Open | 2 |
| Y2 | Digital output 3 | P5-13 | MO2 | SON-ST | Closed | 7 |
| YCOM | Digital output | - |  | Y1, Y2 Output common |  |  |

<!-- page 61 -->

#### 7.1.3 Servo Enable

Enable/disable signal for energizing the motor windings. If the motor is enabled, the user may
execute motion at the motor. If the motor is disabled, the user cannot execute motion at the motor.
◆ Signal logic

| Type | Signal name | Setup value | Signal logic | Function |
|---|---|---|---|---|
| Input | S-ON | 1 | Closed | When the input state is Closed, the driver is enabled |
|  |  | 2 | Open | When the input state is Open state, the driver is enabled |

#### 7.1.4 Alarm Reset

Used to clear warnings or alarms.
◆ Signal logic

| Set Value | Input Logic | Instructions |  |
|---|---|---|---|
| 3 | Closed | In normal state, the input must remain in OPEN(High level) state. This is an edge trigger signal,that<br>means the alarm will be cleared only when the input changes from OPEN(High level) to CLOSED(Low<br>level). |  |
|  |  | Closed<br>A-CLR<br>Open<br>Occurs<br>Alarm<br>No-alarm<br>A | Closed<br>A-CLR<br>Open<br>Occurs<br>Alarm<br>No-alarm<br>A B |
|  |  | When alarm occurs:<br>1) A-CLR input logic is OPEN, alarm is not reset.<br>2) At point A, A-CLR changes from OPEN to<br>CLOSED, the alarm is cleared. | When alarm occurs:<br>1) A-CLR input logic is CLOSED, alrm is not reset.<br>2) At point A, A-CLR changes from CLOSED to<br>OPEN, the alarm is NOT reset.<br>3) At point B, A-CLR changes from Open to<br>Closed, the alarm is cleared. |
| 4 | Open | In normal state, the input must remain in CLOSED(Low level) state. This is an edge trigger signal,that<br>means the alarm will be cleared only when the input changes from CLOSED(Low level) to OPEN(High<br>level). |  |
|  |  | Closed<br>A-CLR<br>Open<br>Occurs<br>Alarm<br>No-alarm<br>A B | Closed<br>A-CLR<br>Open<br>Occurs<br>Alarm<br>No-alarm<br>A B |
|  |  | When alarm occurs:<br>1) A-CLR input logic is CLOSED, alrm is NOT<br>reset.<br>2) At point A, A-CLR changes from CLOSED to<br>OPEN, the alarm is reset.<br>3) At point B, A-CLR changes from OPEN to<br>CLOSED, the alarm is NOT cleared. | When alarm occurs:<br>1) A-CLR input logic is OPEN, alrm is NOT reset.<br>2) At point A, A-CLR changes from OPEN to<br>CLOSED, the alarm is NOT cleared.<br>3) At point B, A-CLR changes from CLOSED to<br>OPEN, the alarm is reset. |

| A |
|---|

Note: When none of the input pins of the drive are not configured with the "servo enable" function,
"alarm reset" can be used to enable the drive, as shown below:

| Closed<br>A-CLR<br>Open<br>Occurs<br>Alarm<br>No-alarm<br>A B<br>Enable<br>Enable Drive<br>Disable | 1) At point A, the rising edge of A-CLR input clears the alarm.<br>2) At point B, the alarm has been cleard. The falling edge of A-CLR enables the<br>drive. |
|---|---|

| A |
|---|

<!-- page 62 -->

#### 7.1.5 CW, CCW Limit

In order to prevent the movable parts of the machine from exceeding the movable range and avoid
accidents, it is necessary to set CW and CCW limit switches.
◆ Signal logic

| Type | Signal name | Setting | Signal logic | Function |
|---|---|---|---|---|
| Input | CCW-LMT | 7 | Closed | When the input state is CLOSED, the drive shows a Negative Limit alarm,<br>motor cannot continue rotating in negative direction. |
|  |  | 8 | Open | When the input state is OPEN, the drive shows a Negative Limit alarm,<br>motor cannot continue rotating in negative direction. |
|  | CW-LMT | 5 | Closed | When the input state is CLOSED, the drive shows a Positive Limit alarm,<br>motor cannot continue rotating in positive direction. |
|  |  | 6 | Open | When the input state is OPEN, the drive shows a Positive Limit alarm,<br>motor cannot continue rotating in positive direction. |

◆ Default settings for MBDV series drives

| Signal<br>name | Input | PIN NO. | Parameter | Command | Setup<br>value | Function | Support<br>mode |  |  |
|---|---|---|---|---|---|---|---|---|---|
| CCW-LMT | X1 | 9 | P5-00 | MU1 | 7 | Motor CW limit signal input | P | V | T |
|  | XCOM | 13 |  |  |  |  |  |  |  |
| CW-LMT | X2 | 10 | P5-01 | MU2 | 5 | Motor CCW limit signal input |  |  |  |
|  | XCOM | 13 |  |  |  |  |  |  |  |

#### 7.1.6 Gain Select

Use the Gain Select function to meet performance requirements of varying loads. There are
different methods of using the Gain Select function. One of these is through an digital input.
Another is called automatic gain switching. These are explained in this section.
1) Increasing the gain can decrease and suppress vibration when doing position control.
2) Reducing the gain can decrease the settling time when the motor comes to a stop.
3) When the motor is running, increasing the gain can improve command following performance.
Example:
When the motor is running at low speed or stopped, a lower gain can be used to reduce noise,
but when the motor is running at high speed or positioning, switch to a higher gain to improve
command following performance.

Motor Speed

1500

| Speed higher tha<br>P0-35<br>Gain Group 2 |  |
|---|---|
|  | n |

Speed lower than
P0-35
P0-35
0

Gain Se

Gain Group 1
Lower gain

Switch delay time from Switch delay time from
Group 1 to Group2 Group 2 to Group1

<!-- page 63 -->

1) Tuning Parameters

| Parameter | Command | Description | Class | Defaults | Unit |
|---|---|---|---|---|---|
| P0-05 | KP | 1st position loop gain | 1st Gain Group | 52 | 0.1Hz |
| P0-07 | KD | 1st position loop derivative time constant |  | 0 | ms |
| P0-08 | KE | 1st position loop derivative filter |  | 20000 | 0.1Hz |
| P0-11 | KF | 1st velocity command gain |  | 10000 | 0.01% |
| P0-12 | VP | 1st velocity loop gain |  | 183 | 0.1Hz |
| P0-13 | VI | 1st speed loop integral time constant |  | 189 | ms |
| P0-16 | KC | 1st command torque filter frequency |  | 1099 | 0.1Hz |
| P0-17 | UP | 2nd position loop gain | 2nd Gain<br>Group | 52 | 0.1Hz |
| P0-19 | UD | 2nd position loop derivative time constant |  | 0 | ms |
| P0-20 | UE | 2nd position loop derivative time constant |  | 20000 | 0.1Hz |
| P0-21 | UF | 2nd velocity command gain |  | 10000 | 0.01% |
| P0-22 | UV | 2nd velocity loop gain |  | 183 | 0.1Hz |
| P0-23 | UG | 2nd velocity loop integral time constant |  | 189 | ms |
| P0-24 | UC | 2nd command torque filter frequency |  | 1099 | 0.1Hz |
| P0-33 | SD | Automatic gain switching method | - | 0 |  |
| P0-34 | PN | Gain switching condition - position error | - | 0 | counts |
| P0-35 | VN | Gain switching condition - actual velocity | - | 0 | 0.025rps |
| P0-36 | TN | Gain switching condition - actual torque | - | 10 | 0.1% |
| P0-37 | SE1 | Delay time - 2nd Group Gains to 1st Group Gains | - | 10 | ms |
| P0-38 | SE2 | Delay time - 1st Group Gains to 2nd Group Gains | - | 10 | ms |

2) Digital Input Signal
Servo drive will switch the first gain to the second gain, when the the digital input signal GAIN-SEL
input is valid.

| Type | Signal Symbol | Setting | Signal logic | Description |
|---|---|---|---|---|
| Input | GAIN-SEL | 11 | Closed | When GAIN-SEL input is CLOSED, 2nd Gain Group takes effect.<br>When GAIN-SEL input is OPEN, 1nd Gain Group takes effect. |
|  |  | 12 | Open | When GAIN-SEL input is OPEN, 2nd Gain Group takes effect.<br>When GAIN-SEL input is CLOSED, 1nd Gain Group takes effect. |

**Note:**
◆ Automatic gain switching is invalid when the gain switching method is set to external input
switching, via the GAIN-SEL signal. That means no matter how P0-33 is set, the gain switching
determined by the external input signal.
3) Automatic Gain Switch
Parameter P0-33 is used to set the method of automatic gain switching

| Parameter | Setting | Condition | Switching Delay Time |
|---|---|---|---|
| P0-33 | 0 (Default) | Fix at 1st Gain Group | - |
|  | 1 | Condition for switching to 2nd Gain Group: Absolute Position following<br>error ≥ P0-34 | P0-38 |
|  |  | Condition for switching to 1st Gain Group: Absolute Position following<br>error < P0-34 | P0-37 |
|  | 2 | Condition for switching to 2nd Gain Group: Absolute value of motor<br>speed ≥ P0-35 | P0-38 |
|  |  | Condition for switching to 1st Gain Group: Absolute value of motor speed<br>< P0-35 | P0-37 |
|  | 3 | Condition for switching to 2nd Gain Group: Absolute value of motor<br>torque ≥ P0-36 | P0-38 |
|  |  | Condition for switching to 1st Gain Group: Absolute value of motor torque<br>< P0-36 | P0-37 |
|  | 4 | Condition for switching to 2nd Gain Group: the positioning is not<br>completed. | P0-38 |
|  |  | Return to 1st Gain Group: the positioning is kept in completed. | P0-37 |

<!-- page 64 -->

Auto switch mode

P0-33
Switching condition
1st Gain Group is met Delay time for switching from the 2nd Gain Group
P0-05 1st Gain Group to 2nd Gain Group P0-17
P0-07 P0-38 P0-19
P0-08 P0-20
P0-11 P0-33 P0-21
P0-12 Switching condition P0-22
P0-13 Delay time for switching from the is not met P0-23
2st Gain Group to 1nd Gain Group
P0-16 P0-24
P0-37

4) Gain Switch Delay Time
To avoid jitter caused by immediate gain switch, when the switch condition is met, the 1st gain
group will gradually switch to the 2nd gain group with the gain switch delay time P0-38.
In the same way, when the inverse switch condition is met, the 2st gain group will gradually switch
to the 1nd gain group with the gain switch delay time P0-37.
As shown below.

Met

| Gain Group 1 |
|---|

Switching Condition

NOT MET

Effective Gain

Gain Group 2 Gain Group 2
Lower gain Lower gain

Switch delay time from Switch delay time from
Group 2 to Group 1 Group 1 to Group 2
P0-37 P0-38

#### 7.1.7 Emergency stop

Emergency stop is a function to forcibly stop the servo motor rotating through an external digital
input.
The E-stop signal needs to be assigned to a digital input when using emergency stop function.
When emergency stop input signal is triggered, the motor will stop to zero with a quick stop
deceleration(P2-01), then and become disabled. Meanwhile an "Emergency Stop" fault is reported.

| Type | Signal name | Setting | Signal logic | Function |
|---|---|---|---|---|
| Input | E-STOP | 13 | Closed | When E-STOP input is CLOSED, the motor is emergency stopped.<br>When E-STOP input is OPEN, the motor runs normally. |
|  |  | 14 | Open | When E-STOP input is CLOSED, the motor runs normally.<br>When E-STOP input is OPEN, the motor is emergency stopped. |

◆ Default settings for MBDV series drives

| Signal<br>Symbol | Input NO. | PIN NO. | Parameter | Command | Setting | Signal<br>logic | Description | Support mode |  |  |
|---|---|---|---|---|---|---|---|---|---|---|
| E-STOP | X4 | 12 | P5-04 | MU5 | 39 | Closed | Motor emergency stop. | P | V | T |
|  | XCOM | 13 |  |  |  | Open | Motor runs normally. |  |  |  |

<!-- page 65 -->

#### 7.1.8 Fault output

When the drive fails, the drive will have a fault alarm output and the servo system will change from
the enabled state to the disable state.
To use this function, a digital output of the servo drive is configured as ALM function.
Parameters P5-12 ~ P5-13 set the function of the digital output Y1 ~ Y2 of the drive. Should either
of these inputs be assigned as the fault output.

| Type | Signal Symbol | Setup Value | Signal logic | Function |
|---|---|---|---|---|
| Output | ALM | 1 | Closed | When the output is CLOSED, it means that the drive has a fault |
|  |  |  | Open | When the output is OPEN, it means the drive is in normal state and no<br>fault is occurred. |
|  |  | 2 | Open | When the output is OPEN, it means that the drive has a fault. |
|  |  |  | Closed | When the output is CLOSED, it means the drive is in normal<br>state and no fault are occurred. |

state and no fault are occurred.

| LED Display | Description | Alarm<br>Type | Drive status<br>after the<br>alarm occurs | LED Display | Description | Alarm<br>Type | Drive status<br>after the<br>alarm occurs |
|---|---|---|---|---|---|---|---|
|  | Drive over temperature<br>Internal voltage error | Fault<br>Fault | Servo off<br>Servo off |  | Regeneration failed | Fault<br>Fault | Servo off<br>Servo off |
|  | Over-voltage | Fault<br>Fault | Servo off<br>Servo off |  | Internal voltage error<br>Full-closed loop<br>position following error | Fault<br>Fault | Servo off<br>Servo off |
|  | Over current<br>Encoder feedback | Fault<br>Fault | Servo off<br>Servo off |  | External encoder error<br>Memory error<br>Motor over | Fault<br>Fault | Servo off<br>Servo off |
|  | error<br>Position following error | Fault<br>Fault | Servo off<br>Servo off |  | temperature<br>Drive MCU over | Fault<br>Fault | Servo off<br>Servo off |
|  | Low voltage<br>Over speed | Fault<br>Fault | Servo off<br>Servo off |  | temperature<br>Absolute encoder multi-<br>turn error<br>Motor stalled | Fault<br>Fault | Servo off<br>Servo off |
|  | Phase loss of main<br>circuit<br>Safe torque off | Fault<br>Fault | Servo off<br>Servo off |  | Motor collision alarm<br>Encoer<br>Communications error | Fault<br>Fault | Servo off<br>Servo off |

Communications error

◆ Default settings for MBDV series drives

| Signal<br>name | Output<br>NO. | PIN NO. | Parameter | Command | Setting | Signal<br>logic | Description | Support mode |  |  |
|---|---|---|---|---|---|---|---|---|---|---|
| ALM | Y1 | 15 | P5-12 | MO1 | 2 | Closed | The drive has a fault. | P | V | T |
|  | YCOM | 14 |  |  |  | Open | The drive is in normal state<br>and no fault is occurred. |  |  |  |

<!-- page 66 -->

#### 7.1.9 Warning output

When a warning occurs, the drive will have a warning output and the servo system maintains
current working status.
To use this function, a digital output of the servo drive is configured as WARN function.
Parameters P5-12 ~ P5-17 set the function of the digital output Y1 ~ Y2 of the drive.

| Type | Signal name | Setting | Signal logic | Function |
|---|---|---|---|---|
| outpu | t WARN | 3 | Closed | The drive generates an abnormal warning, and the output is in<br>the Closed state |
|  |  |  | Open | The drive is normal, there is no abnormal warning, and the<br>output is in the Open state |
|  |  | 4 | Open | The drive generates an abnormal warning, and the output is in<br>the Open state |
|  |  |  | Closed | The drive is normal, there is no abnormal warning, and the<br>output is in the Closed state |

output is in the Closed state

| LED Display | Description | Alarm<br>Type | Drive status after the alarm occurs |
|---|---|---|---|
|  | Limit switch trigger alarm | Warning | Does not change the current state. |
|  | Negative limit alarm | Warning | Does not change the current state, the motor<br>cannot rotate in the negative direction. |
|  | Positive limit alarm | Warning | Does not change the current state, the motor<br>cannot rotate in the positive direction. |
|  | Current limit | Warning | Does not change the current state. |
|  | Communication Error | Warning | Does not change the current state. |
|  | Parameter save failed | Warning | Does not change the current state. |
|  | Under-voltage warning | Warning | Does not change the current state. |
|  | No Q program warning | Warning | Does not change the current state. |
|  | Motion command received while<br>motor disabled. | Warning | Does not change the current state. |
|  | Emergency stopped | Warning | Motor decelerates to stop. |
|  | Absolute encoder battery<br>undervoltage | Warning | Does not change the current state. |
|  | Absolute position lost | Warning | Does not change the current state. |
|  | Absolute position overflow | Warning | Does not change the current state. |
|  | Homing parameters configuration<br>error | Warning | Does not change the current state. |
|  | IO signal used is not general<br>purpose | Warning | Does not change the current state. |
|  | Bus watchdog trigger | Warning | Does not change the current state. |

<!-- page 67 -->

#### 7.1.10 Motor brake control

In order to maintain a fixed position when the drive power is OFF or the motor is disabled, a servo
motor with a brake needs to be used to ensure that the mechanical mechanism driven by the motor
will not move due to its own weight or external force.
When using a servo motor with a brake, one of the digital outputs of the servo drive must be
configured for the BRK function.

| Type | Signal Symbol | Setup Value | Signal Logic | Function |
|---|---|---|---|---|
| Output | BRK | 5 | Closed | When the servo is enabled, the BRK signal is output, and<br>the output state is CLOSED |
|  |  |  | Open | When the servo is disabled, the BRK signal is not output,<br>and the output status is OPEN. |

and the output status is OPEN.

Since the brake has an action delay when it works (brake or release), the timing sequence should
be calculated to avoid damage to brake.

ON

S_ON Signal
OFF

ON
Motor Engage
OFF

ON
Brake Signal
OFF
ON
Brake Release
OFF

ON
Motion Command
OFF
ON
Actual Movement
OFF

Release Delay Time Brake Delay Time
P5-24 P5-25

| Signal<br>Symbol | Output<br>NO. | PIN<br>NO. | Parameter | Command | Setup<br>Value | Signal<br>Logic | Description | Support mode |  |  |
|---|---|---|---|---|---|---|---|---|---|---|
| BRK | BRK | BRK | P5-14 | MO3 | 5 | Closed | When the servo is enabled,<br>the BRK signal is output | P | V | T |

Note: For wiring instructions and precautions when using electromagnetic brake, please refer to
4.6 Using an electromagnetic brake

<!-- page 68 -->

#### 7.1.11 Servo Ready Output

When the servo drive is power on and there is no alarm, the drive will output a Servo Ready signal,
which means that the servo is ready for operation. Servo Ready refers to the situation that all of the
following conditions are met.
1) The drive has no alarms.
2) Main power input is ready.
3) STO is not triggered.
4) Emergency stop(E-STOP) is not triggered.
When the servo system is not ready, even if the drive receives servo-on input signal, the drive will
not be enabled or start to work.

| Type | Signal Symbol | Settings | Signal logic | Function |
|---|---|---|---|---|
| Output | S-RDY | 23 | Closed | When the servo is ready for operation, the S-RDY signal is output,<br>and the output state is CLOSED. |
|  |  |  | Open | When the servo is NOT ready for operation, the S-RDY signal will<br>NOT output, and the output state is OPEN. |
|  |  | 24 | Open | When the servo is ready for operation, the S-RDY signal is output,<br>and the output state is OPEN. |
|  |  |  | Closed | When the servo is NOT ready for operation, the S-RDY signal will<br>NOT output, and the output state is CLOSED. |

NOT output, and the output state is CLOSED.

#### 7.1.12 Servo-on Status Output

The Servo-on Status output signal reflects whether the servo motor is in enabled status.
To use this function, a digital output of the servo drive is configured as SON-ST function.
Parameters P5-12 ~ P5-13 set the function of the digital output Y1 ~ Y2 of the drive.

| Type | Signal Symbol | Settings | Signal Logic | Function |
|---|---|---|---|---|
| Output | SON-ST | 7 | Closed | When the servo is enabled, the SON-ST signal is output, and<br>the output state is CLOSED |
|  |  |  | Open | When the servo is not enabled, the SON-ST signal will not<br>output, and the output state is OPEN. |
|  |  | 8 | Open | When the servo is enabled, and the output state is OPEN. |
|  |  |  | Closed | When the servo is NOT enabled, and the output state is<br>CLOSED. |

CLOSED.
◆ Default settings for MBDV series drives

| Signal<br>Symbol | Output<br>NO. | PIN<br>NO. | Parameter | Command | Setup<br>Value | Signal<br>Logic | Description | Support mode |  |  |
|---|---|---|---|---|---|---|---|---|---|---|
| SON-ST | Y2 | 16 | P5-13 | MO2 | 7 | Closed | Drive is enabled | P | V | T |
|  | YCOM | 14 |  |  |  | Open | Drive is not enabled |  |  |  |

<!-- page 69 -->

#### 7.1.13 Dynamic position error output

The dynamic position error following output refers to the output of this signal when the difference
between the actual position of the motor and the command position is greater than P5-38
(Dynamic position error threshold) during the operation of the motor.

| Type | Signal name | Setup Value | Signal Logic | Function |
|---|---|---|---|---|
| Output | DYM-LMT | 11 | Closed | When the dynamic following error exceeds the setting of P5-38, the DYM-<br>LMT signal is output, and the output state is CLOSED. |
|  |  |  | Open | When the dynamic following error does not exceed the setting of<br>P5-38, the DYM-LMT signal will not output, and the output state is OPEN. |
|  |  | 12 | Open | When the dynamic following error exceeds the setting of P5-38, the DYM-<br>LMT signal will NOT output, and the output state is OPEN. |
|  |  |  | Closed | When the dynamic following error does not exceed the setting<br>of P5-38, the DYM-LMT signal is output, and the output state is<br>CLOSED. |

The following figure is a timing diagram of the setting value of 11, that is, the error exceeds the
setting of P5-38, dynamic position error threshold, and the output state is Closed.

Output setup value is 11
1500
Motor Speed
0

P5-38

Dynamic position error 0

-(P5-38)

Output
Dynamic position
following error output
Not output

<!-- page 70 -->

#### 7.1.14 Software limit output

Software limit output refers to the output of this signal when the motor encounters or triggers the
limit switch oin the current direction of motion, and the motor cannot continue to run in the current
direction. This output has two conditions:
1) Output SLCW when positive direction limit encountered.
2) Output SLCCW when negative direction limit encountered.

| Type | Signal name | Setup value | Signal Logic | Function |
|---|---|---|---|---|
| Output | SLCW | 27 | Closed | The positive limit is triggered, the signal is output, and the output state is<br>Closed. |
|  |  |  | Open | The positive limit is not triggered, the signal is not output, and the output<br>state is Open. |
|  |  | 28 | Open | The positive limit is triggered, the signal is not output, and the output state<br>is Open. |
|  |  |  | Closed | The positive limit is not triggered, the signal is output, and the output state<br>is Closed. |
| Output | SLCCW | 29 | Closed | The negative limit is triggered, the signal is output, and the output state is<br>Closed. |
|  |  |  | Open | The negative limit is not triggered, the signal is not output, and the output<br>state is Open. |
|  |  | 30 | Open | The negative limit is triggered, the signal is not output, and the output state<br>is Open. |
|  |  |  | Closed | The negative limit is not triggered, the signal is output, and the output state<br>is Closed. |

In the absolute value system, the types of limit switches are as follows. When any of the following
conditions are satisfied, the corresponding rotation limit signal will be output.
1) Limit switches trigger digital inputs
2) Conditions meeting parameters P5-47 or P5-48 are met

<!-- page 71 -->

#### 7.1.15 Timing diagram

##### 7.1.15.1 Timing chart for turning on the power

ON
Control Power OFF
(L1C,L2C)

Done
Initialization OFF
about
2s
ON
Main Power OFF OFF
(L1,L2,L3)
Servo Ready Output OFF about Signal Output about OFF
300ms
(Note:1) 1ms
about 1ms
ON
Servo ON Input OFF OFF
about 1ms
ON
Servo ON States Output OFF OFF
about 2ms
ON
Motor Engaged OFF OFF

Warning Output ON
OFF
(Note: 2)
Low-Voltage Output ON OFF ON
(Note:3)

Note 1：When main power is cut off, it may take 1s or longer to stop outputting the Servo Ready
signal due to the capacitor in the drive.
Note 2：If cut off main power input when the drive is enabled, possible alarms may occur as
following, under-voltage alarm (Warning), low-voltage alarm (Fault), position following error.
Note 3：When main power is not applied, the Servo Ready will not output. There will be a low-
voltage alarm if trying to enable the servo.

<!-- page 72 -->

##### 7.1.15.2 Timing chart when fault alarm occurs

ON
Fault Status No fault

Output
Servo-on Status Output No Output

Engaged
Not Engaged
Molor Engaged

Output
Servo Ready Output No Output

Output
Fault Output No Output

### 7.2 Position control mode

#### 7.2.1 Position control mode configuration

In the position control mode, position control is performed by the position command input from the
host controller. The following describes the basic settings for position control.
◆ Block diagram

Servo Drive

Position Command ( Pulse ) Pulse command Electronic Command
input type setting gear ratio smoothing filter
C-CLR Input
Clear the position error counter
INHP Input
Pulse input inhibit
S-HOM Input
Host Controller Homing start
Encoder Output
OCZ Output Encoder divided ouput

IN-POS Output
In-position ouput

<!-- page 73 -->

◆Position Control Mode
Position mode is widely used in equipment that requires precise positioning. MBDV series supports
command position mode. Set the following values to parameter P1-00 through the Luna software,
and the servo drive will work in position control mode.

| Parameter | Command | Setting | Mode | Command |
|---|---|---|---|---|
| P1-00 | CM | 21 | Position control<br>mode | ◆ Q Program<br>◆ Modbus/RTU |

Description
Use Q programming or Modbus/RTU
communication commands for position
control
◆ Command smoothing filter
Smooths the transition in position or speed caused by motion commands. This can reduce jitter
and vibrations in the mechanical system.
Related parameters

| Parameter | Command | Description | Range | Defaults | Unit | Instructions |
|---|---|---|---|---|---|---|
| P2-05 | JT | Jerk time | 0 ~ 50 | 10 | ms | Smoothing filter for internal motion command. |
| P2-28 | KJ | Jerk Filter | 0 ~ 1000 | 10 | ms | Set the time constant of the low-pass filter of the<br>position command or speed command. |

For detailed parameter setting, please refer to chapter 7.2.5 Command smoothing filter setting
◆ Positioning complete signal
In position mode, the positioning complete signal is used to indicate the current positioning state of
the servo motor. When the absolute value of the difference between the commanded position and
the actual position of the servo motor, that is, the position error is smaller than the set value of the
parameter, the positioning completion signal will be output.
Related Parameters

| Type | Signal Symbol | Setup Value | Signal logic | Function |
|---|---|---|---|---|
| Output | IN-POS | 9 | Closed | Positioning completed, IN-POS condition satisfied and signal is output.<br>Output state is Closed. |
|  |  |  | Open | Positioning not completed, IN-POS condition not satisfied and signal is not<br>output. Output state is Open. |
|  |  | 10 | Open | Positioning completed, IN-POS condition satisfied and signal is output.<br>Output state is Open. |
|  |  |  | Closed | Positioning not completed, IN-POS condition not satisfied and signal is not<br>output. Output state is Closed. |

For detailed parameter setting, please refer to chapter 7.2.8 Positioning complete signal
◆ Encoder feedback output
The encoder feedback output function of the servo driver is a function of outputting the position
information fed back by the motor encoder via differential A/B/Z signals. A/B signals have a 90°
difference. .
Related parameters

| Parameter | Command | Description | Range | Defaults | Unit | Instructions |
|---|---|---|---|---|---|---|
| P3-12 | PO | Encoder feedback output mode | 0 ~ 256 | 1 | - | Pulse frequency division output<br>mode setting |
| P3-13 | ON | Pulse Output Gear Ratio-<br>Numerator | 0 ~ 65535 | 10000 | - | Set the numerator of the pulse<br>output distribution ratio |
| P3-14 | OD | Pulse Output Gear Ratio-<br>Denominator | 0 ~ 65535 | 65535 | - | Set the denominator of the pulse<br>output distribution ratio |

For detailed parameter setting, please refer to chapter 7.6 Encoder frequency division output

<!-- page 74 -->

#### 7.2.2 Command smoothing filter setting

When the position command or speed command of the servo system changes significantly, it is
easy to cause the whole system to vibrate, and the system noise will also increase. Command filter
smooths the transition in position or speed caused by motion commands. This can reduce jitter and
vibrations in the mechanical system.
Related parameters

| Parameter | Instruction | Name | Value range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P2-05 | JT | jerk time | 0 ~ 125 | 10 | ms | Time constant for smoothing filtering in<br>internal trajectory mode |
| P2-28 | KJ | low pass<br>smoothing filter | 0 ~ 10000 | 10 | ms | Set the time constant of the low-pass filter of<br>the position command or speed command |

Note: When set to 0, the filtering function is invalid

##### 7.2.2.1 Jerk time

Parameter P2-05 can be used in various drive control modes (position, speed, torque) and when
streaming commands to the drive from an external controller. The effect of jerk smoothing on the
input command is shown in the figure below.

| Speed<br>Target Speed Curves<br>Filtered speed Curves<br>Time<br>T T T T=P2-05 | ◆ Jerk time filter will produce a certain delay T for the command,<br>but it will not affect the final positioning accuracy.<br>◆ The larger the P2-05 time constant, the more obvious the<br>smoothing effect will be, and the delay of instruction response will<br>increase, so users should set an appropriate filter time constant<br>according to the application. |
|---|---|

##### 7.2.2.2 Low pass smoothing filter

Parameter P2-28 can be used in various drive control modes (position, speed, torque) and when
streaming commands to the drive from an external controller. The smoothing effect of the low-pass
filter on the input command is shown in the figure below.

| Motor Speed Target Speed Instruction<br>VC Filtered Speed<br>VC×0.632<br>VC×0.368<br>P2-28 P2-28 Time | ◆ Low-pass smoothing filter will produce a certain delay T for<br>the motion command, but it will not affect the final positioning ac<br>curacy.<br>◆ The larger the time constant of P2-28, the more obvious the<br>smoothing effect will be, and the delay of instruction response<br>will be increased. Users should set an appropriate filter time<br>constant according to the application. |
|---|---|

<!-- page 75 -->

#### 7.2.3 Positioning complete signal

In the position control mode, use the positioning completion signal output to indicate the current
positioning status of the servo motor. When the absolute value of the difference between the
commanded position and the actual position of the servo motor, that is, the position error is smaller
than the set value of the parameter, the positioning completion signal will be output.

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| Output | IN-POS | 9 | Closed | Positioning completed, IN-POS condition satisfied and signal is output. Output<br>state is Closed. |
|  |  |  | Open | Positioning not completed, IN-POS condition not satisfied and signal is not<br>output. Output state is Open. |
|  |  | 10 | Open | Positioning completed, IN-POS condition satisfied and signal is output. Output<br>state is Open. |
|  |  |  | Closed | Positioning not completed, IN-POS condition not satisfied and signal is not<br>output. Output state is Closed. |

Related parameters

| Parameter | Instruction | Name | Value range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P5-39 | PD | Positioning complete error<br>margin | 0 ~ 32000 | 40 | pulses | When motion is completed, the<br>position error must be within this<br>set range. If this is satisfied, the<br>positioning complete signal is output. |
| P5-40 | PE | Motion condition timer | 0 ~ 32000 | 10 | ms | If the motion complete condition<br>is satisfied for this length of time,<br>positioning will be considered<br>complete and IN-POS will be output. |

As shown below

Command Curve
Actual Position Feedback

P5-39

(Positioning complete
signal position error )
P5-40
(Count time for positioning
completion)
Position arrival signal output

Moving

#### 7.2.4 position reached output

Position arrival output P-COIN signal indicates that the actual position of the motor is equal to the
position set by parameter P5-46.
◆ Position arrival output P-COIN setting

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| output | P-COIN | 31 | Closed | the position reaches the P-COIN judgment condition, the output signal is output, and<br>the output state is closed. |
|  |  |  | Open | the position reaches the P-COIN judgment condition is not established, no signal is<br>output, and the output state is open |
|  |  | 32 | Open | the position reaches the P-COIN judgment condition is established, the output signal<br>is output, and the output state is open |
|  |  |  | Closed | the position reaches P-COIN, the judgment condition is not established, no signal is<br>output, and the output state is closed. |

output, and the output state is closed.
Related parameter settings

| Parameter | Instruction | Name | Value range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P5-46 | DG | Absolute arrival position | -2147483647 ~<br>+2147483647 | 10000 | pulses | Determine the target position of the<br>output position coincidence signal |

<!-- page 76 -->

◆ Position arrival P-COIN judgment condition
When the actual position is equal to the setting of parameter P5-46, the output position reaches the
P-COIN signal. The fluctuation range is ± 100pulses.

Actual Position

(P5-46)+100pulses

Parameters (P5-46)
(P5-46)-100pulses

Time

Output
Position Reached Outp
No O No Output

#### 7.2.5 Gain parameter in position mode

In position mode, application appropriate gain parameters can make the servo system run
more smoothly and accurately, and have excellent positioning performance. The following gain
parameters in position mode can be automatically adjusted using Luna.

| Parameter | Command | Parameter name | Type | Defaults | Unit |
|---|---|---|---|---|---|
| P0-05 | KP | 1st position loop gain | First set of<br>gains | 52 | 0.1Hz |
| P0-07 | KD | 1st position loop derivative time constant |  | 0 | ms |
| P0-08 | KE | 1st position loop derivative filter |  | 20000 | 0.1Hz |
| P0-11 | KF | 1st velocity command gain |  | 10000 | 0.01% |
| P0-12 | VP | 1st velocity loop gain |  | 183 | 0.1Hz |
| P0-13 | VI | 1st speed loop integral time constant |  | 189 | ms |
| P0-16 | KC | 1st command torque filter frequency |  | 1099 | 0.1Hz |
| P0-17 | UP | 2nd position loop gain | Second set of<br>gains | 52 | 0.1Hz |
| P0-19 | UD | 2nd position loop derivative time constant |  | 0 | ms |
| P0-20 | UE | 2nd position loop derivative time constant |  | 20000 | 0.1Hz |
| P0-21 | UF | 2nd velocity command gain |  | 10000 | 0.01% |
| P0-22 | UV | 2nd velocity loop gain |  | 183 | 0.1Hz |
| P0-23 | UG | 2nd velocity loop integral time constant |  | 189 | ms |
| P0-24 | UC | 2nd command torque filter frequency |  | 1099 | 0.1Hz |
| P0-33 | SD | Automatic gain switching method | - | 0 |  |
| P0-34 | PN | Gain switching condition - position error | - | 0 | pulses |
| P0-35 | VN | Gain switching condition - actual velocity | - | 0 | rps |
| P0-36 | TN | Gain switching condition - actual torque | - | 10 | 0.1% |
| P0-37 | SE1 | Delay time - 2nd Group Gains to 1st Group Gains | - | 10 | ms |
| P0-38 | SE2 | Delay time - 1st Group Gains to 2nd Group Gains | - | 0 | ms |

<!-- page 77 -->

### 7.3 Velocity control mode

#### 7.3.1 Velocity control mode configuration

Velocity control mode is used for precise speed control.
◆ Block diagram

Servo Drives
Speed command Instruction
Smoothing Limit Instruction limit

| SPD-DIR Direction Switching |
|---|
| ZCLAMP Zero Speed Clamp |
| Z-SPD Zero Speed Signal Output |
| AT-SPD Speed Reach Output |
| V-COIN Speed Consistent Output |
| T-LMT Torque Limit Output |

| Motor rotation direction<br>switching |
|---|
| Zero speed clamp function |
| Zero speed, signal output<br>during motion |

Host
Controller

Speed arrival signal output

Speed coincidence signal
output

Torque limited output

◆ Come back to this subsection
MBDV series servo drive command speed mode.
Command speed mode: Use MOONS' unique Q programming to control the motor, or use Modbus
commands to control the motor speed.

| Model | Control signal | Parameter P1-00<br>setting | Description |
|---|---|---|---|
| Velocity control<br>mode | Internal speed<br>command,<br>communication<br>command or Q<br>programming | 15 | ◆ Internal speed mode<br>◆ Control using the Q programming function<br>◆ Use Modbus command to control When using<br>Modbus command to directly control the motor<br>operation, P1-00 must be set to 21 |

<!-- page 78 -->

#### 7.3.2 Zero speed clamp function

In velocity control mode, the zero speed clamp function can be configured in two ways:
**◆ P5-51 (zero speed clamp function) set to 0**
Activation of the zero speed clamp function becomes dependent on the ZCLAMP input signal. If the
ZCLAMP input is valid and the commanded velocity falls below the zero speed threshold (P5-42), the servo
motor enters a position lock state. This state maintains the motor shaft in position and should the shaft be
caused to move by external forces, the shaft will return to the position in which it was initially locked.
Activation of the zero speed clamp function becomes dependent on the ZCLAMP input signal. If the
ZCLAMP input is valid and the commanded velocity falls below the zero speed threshold (P5-42), the servo
motor enters a position lock state. This state maintains the motor shaft in position and should the shaft be
caused to move by external forces, the shaft will return to the position in which it was initially locked.
电机转速 Motor speed
Motor speed

P5-42

| Input is valid<br>Invalid |
|---|

0

Zero speed
clamp input signal Invalid input

Take effect
Zero speed
clamp function Invalid

If the servo moto电r 机v转ib速rates when it is in a locked stated due to the zero speed clamp function, the
position loop gain needs to be adjusted. It is necessary to set an appropriate zero speed threshold.
If the value is too high, it will cause significant vibrations in the system due to rapid deceleration.
**ZCLAMP input signal configuration**
To use the zero speed clamp function as mentioned above (setting of 0), users will need to assign
the ZCLAMP function to an input. See below for configuration methods of the ZCLAMP input:

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| Input | ZCLAMP | 21 | Closed | The input signal is valid and the speed command is less than P5-42, the ZCLAMP<br>function is valid |
|  |  |  | Open | Input signal is invalid, even if the speed command is less than P5-42, the ZCLAMP<br>function will not be valid |
|  |  | 22 | Open | The input signal is valid and the speed command is less than P5-42, the ZCLAMP<br>function is valid |
|  |  |  | Closed | Input signal is invalid, even if the speed command is less than P5-42, the ZCLAMP<br>function will not be valid |

function will not be valid
**◆ P5-51 (zero speed clamp function) set to 1**
The zero-speed clamp state is independent of the zero-speed clamp input signal ZCLAMP.
Activation of the zero speed clamp function becomes independent of the ZCLAMP input signal. If
the commanded velocity is 0 and the actual velocity falls below the zero speed threshold (P5-42)
for a duration of time equivalent to P5-40, the servo motor enters a position lock state. This state
maintains the motor shaft in position and should the shaft be caused to move by external forces,
the shaft will return to the position in which it was initially locked.
If the commanded velocity is not zero, the servo motor exits the position lock state and accelerates
to the current commanded velocity with an acceleration equivalent to P2-03.
Related parameters

| Parameter | Instruction | Name | Value range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P5-40 | PE | Motion condition timer | 0 ~ 30000 | 10 | ms | MS=0<br>When the command speed is less than<br>or equal to the zero-speed judgment<br>threshold, the drive considers that it is in<br>the zero-speed state at this time |
| P5-42 | ZV | Zero speed threshold | 0.1 ~ 2 | 0.5 | rps |  |
| P5-51 | MS | Zero speed clamp function | 0 ~ 1 | 0 | - |  |

<!-- page 79 -->

#### 7.3.3 Rotation direction switch

In velocity control, direction of rotation can be selected via a digital input. When a digital input
has been configured with the SPD-DIR function, the servo drive will use the magnitude of the
commanded velocity and determine direction of motion based on the logic state of the SPD-DIR
input.
**SPD-DIR input signal configuration**
The SPD-DIR signal can be configured as follows when assigned to a digital input:

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| Input | SPD-DIR | 35 | Closed | When the input signal is valid, the direction of the speed command is<br>reversed |
|  |  |  | Open | The input signal is invalid, the rotation direction of the motor is<br>determined by the direction of the speed command |
|  |  | 36 | Open | When the input signal is valid, the direction of the speed command is<br>reversed |
|  |  |  | Closed | The input signal is invalid, the rotation direction of the motor is<br>determined by the direction of the speed command |
|  | GP | 0 | - | When all input pins of the driver are not configured with this function, the<br>rotation direction of the motor is determined by the positive and negative<br>of the command speed |

Determined by parameter P1-11 motor rotation direction, speed command (communication
command), and speed command direction switching SPD-DIR. The detailed relationship is shown
in the table below.
◆ When all input pins of the driver are not configured with this function

| Parameter P1-11 Motor | Speed command<br>(Communication） | Speed command direction<br>switching SPD-DIR input | Actual motor rotation direction |
|---|---|---|---|
| 0 | just | This input function is not set | CW Clockwise |
| 0 | just | This input function is not set |  |
| 0 | burden | This input function is not set | CCW counterclockwise |
| 0 | burden | This input function is not set |  |
| 1 | just | This input function is not set | CCW counterclockwise |
| 1 | just | This input function is not set |  |
| 1 | burden | This input function is not set | CW Clockwise |
| 1 | burden | This input function is not set |  |

◆ When the drive input pin is configured as speed command direction switch SPD-DIR

| Parameter P1-11 Motor<br>rotation direction set value | Speed command<br>(Communication<br>command) | Speed command direction<br>switching SPD-DIR input | Actual motor rotation direction |
|---|---|---|---|
| 0 | just | invalid | CW Clockwise |
| 0 | burden | invalid |  |
| 0 | just | efficient | CCW counterclockwise |
| 0 | burden | efficient |  |
| 1 | just | invalid | CCW counterclockwise |
| 1 | burden | invalid |  |
| 1 | just | efficient | CW Clockwise |
| 1 | burden | efficient |  |

<!-- page 80 -->

#### 7.3.4 Zero speed output

When the absolute value of the actual speed of the motor is less than P5-42 (zero speed judgment
threshold), and the duration reaches the set time of P5-40 , the servo drive outputs the zero-speed
signal Z-SPD. If the absolute value of the actual speed of the motor is greater than P5-42, the
zero-speed signal Z-SPD will not be output.
The zero speed output is not dependent on the control mode or the state of the servo motor. This
allows users to use this output signal as notifier of ongoing motion at the motor.

Actual Speed
Speed

P5-42
Time

P5-40

Zero Speed Signal

| Output |
|---|

No Output No Output

**◆ Z-SPD output signal configuration**
When using Z-SPD, a digital output pin needs to be assigned this function.

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| Output | Z-SPD | 33 | Closed | The Z-SPD judgment condition is satisfied, the Z-SPD signal is output,<br>and the output state is closed |
|  |  |  | Open | The Z-SPD judgment condition is not satisfied, the Z=SPD signal is not<br>output, and the output state is open |
|  |  | 34 | Open | The Z-SPD judgment condition is satisfied, the Z-SPD signal is output,<br>and the output state is open |
|  |  |  | Closed | The Z-SPD judgment condition is not satisfied, the Z-SPD signal is not<br>output, and the output state is closed |

Related parameters

| Parameter | Instruction | Name | Value range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P5-40 | PE | Motion condition timer | 0 ~ 30000 | 10 | ms | When the speed is less than or<br>equal to the set value of P5-42, and<br>the duration reaches the set time of<br>P5-40, the drive considers that it is<br>in the zero-speed state at this time |
| P5-42 | ZV | Zero speed threshold<br>range | 0.1 ~ 2 | 0.5 | rps |  |

<!-- page 81 -->

#### 7.3.5 Velocity reached output

In velocity control mode mode, when the absolute value of the actual motor speed exceeds P5-44
(Velocity Reached - Minimum threshold threshold), for the duration of time specified in P5-40 P5-
40, the velocity reached signal AT-SPD will be output.
If the actual speed of the motor after filtering does not exceed P5-44, the velocity reached signal
AT-SPD will not be output.
**◆ AT-SPD output signal configuration**
When using the velocity reached output AT-SPD, a digital output pin needs to be assigned this
function.

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| Output | AT-SPD | 19 | Closed | The AT-SPD judgment condition is satisfied, the output signal is output,<br>and the output state is closed |
|  |  |  | Open | The AT-SPD judgment condition is not established, no signal is output,<br>and the output state is open |
|  |  | 20 | Open | The AT-SPD judgment condition is satisfied, the output signal is output,<br>and the output state is open |
|  |  |  | Closed | The AT-SPD judgment condition is not established, no signal is output,<br>and the output state is closed |

Speed

P5-44

| Output |
|---|

0 Time
P5-40

Speed Arrival Signal No Output No Output

Related parameters

| Parameter | Instruction | Name | Value<br>range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P5-40 | PE | Motion condition timer | 0 ~ 30000 | 10 | ms | When the absolute value of the actual<br>speed of the motor exceeds P5-44 and<br>the time reaches P5-40, it will output the<br>speed reaching signal AT-SPD |
| P5-44 | VV | Velocity Reached -<br>Minimum threshold | 0 ~ 100 | 10 | rps |  |

<!-- page 82 -->

#### 7.3.6 Velocity consistent output

In velocity control mode, when the fluctuation of the actual velocity from the commanded velocity
is within the margins set by P5-43, for the duration of time specified by P5-40, then it is determined
that the actual speed of the motor is consistent with the commanded velocity and the velocity
consistent signal, V-COIN, is output.
If the actual velocity falls outside of P5-43, the speed consistent signal V-COIN will not be output.
**◆ V-COIN output signal configuration**
When using the speed consistent output V-COIN, the digital output pin needs to be assigned this
function.

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| Output | V-COIN | 17 | Closed | V-COIN judgment condition is established, output signal, output<br>state is closed |
|  |  |  | Open | The V-COIN judgment condition is not established, no signal is<br>output, and the output state is open |
|  |  | 18 | Open | V-COIN judgment condition is established, output signal, output<br>state is open |
|  |  |  | Closed | If the judgment condition is not established, no signal is output, and<br>the output state is closed |

Command Speed
Command Speed After Acceleration
Speed P5-43 and Deceleration Processing
Actual Speed

| P5-40<br>No Output | Output | No Output |
|---|---|---|

P5-43 P5-43
Time

-40

ut
Speed Coincidence Signal

Related parameters

| Parameter | Instruction | Name | Value<br>range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P5-40 | PE | Motion condition timer | 0 ~ 32000 | 10 | ms | If the speed error is within the setting<br>of P5-43, and the duration reaches<br>P5-40, it is determined that the actual<br>speed of the motor is consistent with<br>the command speed, and the speed<br>consistent signal V-COIN will be output. |
| P5-43 | VR | Velocity Reached<br>- Permissible<br>fluctuation range | 0.1 ~ 100 | 0.2 | rps |  |

<!-- page 83 -->

#### 7.3.7 Velocity control Mode methods

In velocity control mode, there are two control types:
1. Position over time
2. Speed control only (default setting)
Related parameters

| Parameter | Instruction | Name | Value<br>range | Defaults | Unit | Describe |
|---|---|---|---|---|---|---|
| P1-03 | JM | Velocity<br>control method | 1，2 | 2 | - | Set the control type in speed mode<br>1. Position over time<br>2. Speed control only (default setting) |
| P0-12 | VP | Speed loop<br>proportional | 0 ~ 30000 | 183 | 0.1Hz | In the set speed mode, when P-15(JM) is 2, the<br>speed loop proportional coefficient |
| P0-13 | VI | Speed loop<br>integral tim | 0 ~ 30000 | 189 | ms | In the set speed mode, when P-15(JM) is 2, the<br>speed loop integral coefficient |

A) P1-03 = 1, the position error is detected in real time.
Under this control type, the position error will be detected in real time. When the absolute value of
the difference between the actual position fed back by the encoder and the command position, that
is, the position error exceeds P3-04 (position error alarm limit), The drive will generate a fault alarm
that the position error exceeds the limit.
B) P1-03 = 2, speed control only
Under this control type, no position error is detected, and no alarm will be generated even if the
motor is locked. In this control mode, the speed loop gain parameter is set by P0-12 speed loop
proportional gain and P0-13 speed loop integral time.

<!-- page 84 -->

### 7.4 Torque control mode

#### 7.4.1 Commanded torque control

Torque control mode is used for precise torque control. MBDV servo drives support command
torque mode. Command torque mode uses communication commands to control the motor.

| Model | Control signal | P1-00(CM)<br>definition | Description |
|---|---|---|---|
| Command torque<br>mode | communication command | 1 | Using Modbus command |

#### 7.4.2 Torque command filter

Filtering is performed on torque commands to ensure changes in torque output are gradual and
thereby reduce mechanical vibrations in the system.
◆ Jerk time
Parameter P2-05 jerk time takes effect in internal trajectory modes (position, speed, torque) and
communication based control mode. The effect of jerk smoothing on the input command is shown
in the figure below.

| Speed<br>Target Speed Curves<br>Filtered speed Curves<br>Time<br>T T T T=P2-05 | ◆ Jerk time filter will produce a certain delay T for the<br>instruction, but it will not affect the final positioning accuracy.<br>◆ The larger the P2-05 time constant, the more obvious the<br>smoothing effect will be, and the delay of instruction response<br>will increase, so users should set the appropriate filter time<br>constant according to the application. |
|---|---|

◆ Low pass filter
Parameter P2-28 low-pass filter can take effect in internal trajectory modes. (position, speed,
torque), and communication based control mode.
The smoothing effect of the low-pass smoothing filter on the input command is shown in the figure
below.

| Motor Speed Target Speed Instruction<br>VC Filtered Speed<br>VC×0.632<br>VC×0.368<br>P2-28 P2-28 Time | ◆ Low-pass filtering will produce a certain delay T for the<br>instruction pulse, but it will not affect the final positioning<br>accuracy.<br>◆ The larger the time constant of P2-28, the more obvious the<br>smoothing effect will be, and the delay of instruction response<br>will be increased. Users should set the appropriate filter time<br>constant according to the application. |
|---|---|

<!-- page 85 -->

#### 7.4.3 Speed limit in torque mode

It is recommended that users set the maximum speed of the motor prior to configuring the drive for
torque control mode. The motor will run at this maximum set speed when it is used in torque control
mode which, depending on the application, could cause damage to the load if left at a high value.
◆ In torque control mode, the set jog speed (P2-02) can be used as the speed limit for the motor.
After the speed limiting function is enabled, the motor will accelerate and be limited to the set value.
The maximum speed setting should be based on application requirements. This limit can be
configured in two ways. See the following:

| Speed limit source | Describe |
|---|---|
| Internal Limits | 1. Directly limited by parameter P2-30(VT)<br>2. Limited by parameter P2-02 (JS). To select this speed limit, a digital input must be assigned the velocity<br>limit function (V-LMT). Once the desired input logic is satisfied, this speed limit will take effect. |

Speed limit set by P2-30

Speed limiting
function enabled
V-LMT Input
Any input (P5-00-P5-03) assigned the Speed Limit set by P2-02
V-LMT function (code 37 or 38)

Related parameters

| Parameter | Instruction | Name | Value range | Defaults | Unit | Describe |
|---|---|---|---|---|---|---|
| P2-02 | JS | Jog Speed | -100 ~ 100 | 10 | rps | In torque control, the jog speed can be used<br>the speed limit of the motor. A digital input<br>must be assigned the V-LMT function. Once<br>the desired input logic is satisfied, V-LMT is<br>enabled and the Jog Speed value behaves<br>as the speed limit. |
| P2-03 | JA | Jog Acceleration | 0.167~ 5000 | 100 | rps/s | Acceleration in torque mode |
| P2-04 | JL | Jog Deceleration | 0.167~ 5000 | 100 | rps/s | Deceleration in torque mode |
| P2-30 | VT | Speed limit in torque<br>mode | 0 ~ 100 | 80 | rps | In torque control mode, this parameter can<br>be used as the direct speed limit. No input is<br>required to enable this speed limit. It will be<br>enabled as soon as torque control mode is<br>enabled at the drive. |
| P5-00-<br>P5-03 | MU1~MU4 | Digital Input (X1-X4)<br>function assignment | 37 ~ 38 | —— | —— | Assigning a value of 37 or 38 to an input<br>(P5-00 - P5-03) will assign the V-LMT<br>function to that input. |
| P5-12-<br>P5-13 | MO1~MO2 | Digital Output (Y1-Y2)<br>function assignment | 21 ~ 22 | —— | —— | Assigning a value of 21or 21 to an output<br>(P5-12 - P5-13) will assign the "Velocity<br>Limit Reached" function to that output. |

◆ V-LMT function
When a digital input is assigned the V-LMT function, the drive will in essence have two speed limits
when operating in Torque Control. The primary one is set by P2-30. However, once the desired
V-LMT input logic is satisfied, the speed limit set by P2-02 will be enabled and will take higher
priority than the limit set by P2-30.

| V-LMT input is invalid | V-LMT input is valid |
|---|---|
| Speed<br>Motor Maximum Speed<br>P2-30 Limit Value<br>P2-02 Limit Value<br>Time<br>-P2-02 Limit Value<br>-P2-30 Limit Value<br>Motor Maximum Speed | Speed<br>Motor Maximum Speed<br>P2-30 Limit Value<br>P2-02 Limit Value<br>Time<br>-P2-02 Limit Value<br>-P2-30 Limit Value<br>Motor Maximum Speed |

<!-- page 86 -->

#### 7.4.4 Output during speed limiting

In torque control mode, when the maximum speed limit is reached, an output signal will be output
notifying the user. The following are the parameters of this signal:

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| Output | V-LMT | 21 | Closed | The output speed of the motor is limited, the output signal, the output state is<br>closed |
|  |  |  | Open | The output speed of the motor is not limited, no signal is output, and the output<br>state is open |
|  |  | 22 | Open | The output speed of the motor is limited, the output signal, the output state is open |
|  |  |  | Closed | The output speed of the motor is not limited, no signal is output, and the output<br>state is closed |

state is closed
**Note: Please refer to 7.1.2 Output signal setting**

#### 7.4.5 Torque reaches output

When operating in torque control, if the absolute value of the actual torque reaches the target
torque (P1-07), remains within the permissible fluctuation range (P5-45) for the amount of time
specified by P5-40, the Torque Reached output signal (TQ-REACH) will be output.
If any of the above conditions is not satisfied, the TQ-REACH signal will not be output.
TQ-REACH is applicable in all supported control modes (Torque, Velocity, Position etc.)
**◆ Setting of torque arrival signal TQ-REACH**
When using the torque arrival signal TQ-REACH, a digital output pin needs to be assigned this
function.

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| Output | TQ-REACH | 13 | Closed | TQ-REACH conditions satisfied, the signal is output and the state of the output is<br>closed. |
|  |  |  | Open | TQ-REACH conditions not satisfied, the signal is not output and the state of the<br>output is open. |
|  |  | 14 | Open | TQ-REACH conditions satisfied, the signal is output and the state of the output is<br>open. |
|  |  |  | Closed | TQ-REACH conditions not satisfied, the signal is not output and the state of the<br>output is closed. |

Torque

P1-07+P5-45

| P5-40<br>Output |
|---|

P1-07
P1-07-P5-45

0 Time

Torque Arrival Signal
No Output

<!-- page 87 -->

Related Parameters

| Parameter | Instruction | Name | Value<br>range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P1-07 | CV | Target torque | 0~3000 | 0 | 0.1% | When operating in torque control, if<br>the absolute value of the actual torque<br>reaches the target torque threshold<br>(P1-07), remains within the permissible<br>fluctuation range (P5-45) for the amount<br>of time specified by P5-40, the Torque<br>Reached output signal (TQ-REACH) will<br>be output. |
| P5-40 | PE | Motion condition<br>timer | 0 ~ 30000 | 10 | ms |  |
| P5-45 | TV | Permissible torque<br>fluctuation range | 0~3000 | 10 | 0.1% |  |

#### 7.4.6 Torque consistent output

When fluctuations of the actual torque from the target torque are within P5-45 for the duration
specified by P5-40, the actual torque is considered consistent with the target torque. The I-COIN
signal is output to notify users of this.
If the torque fluctuation exceeds P5-45, the torque consistent signal I-COIN will not be output.
◆ Setting of torque coincidence signal I-COIN
When using the torque consistent output, I-COIN, the digital output pin needs to be assigned this
function.

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| output | I-COIN | 35 | Closed | I-COIN conditions satisfied, signal is output, output state is closed |
|  |  |  | Open | I-COIN conditions not satisfied, signal is not output, output state is open |
|  |  | 36 | Open | I-COIN conditions satisfied, signal is output, output state is open |
|  |  |  | Closed | I-COIN conditions not satisfied, signal is not output, output state is closed |

Motor Torque
P5-45 Command Torque
Actual Torque

| No<br>Output | P5-40 | No<br>Output |
|---|---|---|

P5-45
Time
P5-40

Torque
Consistent Output
Output

Related parameters

| Parameter | Instruction | Name | Value<br>range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P5-40 | PE | Motion condition timer | 0 ~ 30000 | 10 | ms | When fluctuations of the actual torque from<br>the target torque are within P5-45 for the<br>duration specified by P5-40, the actual<br>torque is considered consistent with the<br>target torque. The I-COIN signal is output<br>to notify users of this. |
| P5-45 | TV | Permissible torque<br>fluctuation range | 0 ~ 3000 | 10 | 0.1% |  |

<!-- page 88 -->

### 7.5 Torque limit

Torque limit is to limit the output torque of the servo motor. This function is applicable in all control
modes, such as position, speed, torque, etc.
◆ method of torque limitation
Parameter P1-10 defines 5 kinds of torque limit methods, each limit method is as follows.

| P1-10<br>Torque limit method setting | Positive torque limit source | Reverse torque limit source |
|---|---|---|
| 0 | Register Y | Register Z |
| 1 | Parameter P1-06 |  |
| 2 | Parameter P1-06 | Parameter P1-25 |
| 3 | TQ-LMT input is valid: P1-06 |  |
|  | TQ-LMT input is invalid: P1-25 |  |
| 5 | TQ-LMT input is valid: P1-06 | TQ-LMT input is valid: P1-25 |
|  | TQ-LMT input is invalid: P1-26 | TQ-LMT input is invalid: P1-27 |

Related parameters

| Parameter | Instruction | Name | Value range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P1-10 | LD | Torque Limit Method | 0 ~ 3，5 | 1 | - | Torque limit method setting, please refer<br>to the above description for details |
| P1-06 | CC | first torque limit | 0~3000 | 3000 | 0.1% | First torque limit of the motor |
| P1-25 | CX | Second torque limit | 0~3000 | 3000 | 0.1% | Second torque limit for motor |
| P1-26 | CY | Third torque limit | 0~3000 | 3000 | 0.1% | Third torque limit for motor |
| P1-27 | CZ | Fourth torque limit | 0~3000 | 3000 | 0.1% | Fourth torque limit for motor |

#### 7.5.1 Torque Limiting Methods

##### 7.5.1.1 Register based torque limits --- effective immediately

When P1-10 = 0, the forward torque limit is determined by Register Y, and the reverse torque limit
is determined by parameter Register Z.
Related parameters

| Parameter | Modbus<br>address | Value<br>range | Defaults | Unit | Description |
|---|---|---|---|---|---|
| Register Y | 40065 | 0~3000 | 0 | 0.1% | Forward torque limit for the motor, effective<br>immediately |
| Register Z | 40066 | 0~3000 | 0 | 0.1% | Reverse torque limit for the motor, effective<br>immediately |

Note: If torque limits are set too low, there may be insufficient torque available for acceleration and
deceleration.

| Torque Limits Disabled | Torque Limits Enabled |
|---|---|
| Motor Maximum Torque<br>Time<br>Motor maximum torque | Motor Maximum Torque<br>Register Y<br>Time<br>-Register Z<br>Motor maximum torque |
| Without torque limits, the motor can reach its maximum<br>output torque. | When torque limits are active and P1-10 = 0, the forward and<br>reverse torque limits of the motor are set by Register Y and<br>Register Z. |

<!-- page 89 -->

##### 7.5.1.2 Parameter based torque limits (same limit value for forward and reverse directions)

When P1-10 = When 1, the forward and reverse torque limits are determined by parameter P1-06.
Related parameters

| Parameter | Instruction | Name | Value<br>range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P1-06 | CC | First torque limit | 0~3000 | 3000 | 0.1% | First torque limit of the motor |

Note: If torque limits are set too low, there may be insufficient torque available for acceleration and
deceleration.

| Torque limits disabled | Torque limits enabled |
|---|---|
| Motor Maximum Torque<br>Time<br>Motor maximum torque | Motor Maximum Torque<br>P1-06<br>Time<br>-P1-06<br>Motor maximum torque |
| Without torque limits, the motor can reach its maximum<br>output torque. | When P1-10 = 1, both the forward and reverse torque limits<br>are set by P1-06.. |

##### 7.5.1.3 Forward and reverse torques are limited by different parameters

When P1-10 = 2, the forward torque limit is determined by parameter P1-06, and the reverse
torque limit is determined by parameter P1-25.
Related parameters

| Parameter | Instruction | Name | Value<br>range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P1-06 | CC | First torque limit | 0~3000 | 3000 | 0.1% | First torque limit of the motor |
| P1-25 | CX | Second torque limit | 0~3000 | 3000 | 0.1% | Second torque limit for motor |

Note: If torque limits are set too low, there may be insufficient torque available for acceleration and
deceleration.

| Torque limits disabled | Torque limits enabled |
|---|---|
| Motor Maximum Torque<br>Time<br>Motor maximum torque | Motor Maximum Torque<br>P1-06<br>Time<br>-P1-25<br>Motor maximum torque |
| Without torque limits, the motor can reach its maximum<br>output torque. | When P1-10 = 2. the forward torque limit is set by P1-06 and<br>the reverse torque limit is set by P1-25. |

<!-- page 90 -->

##### 7.5.1.4 Torque limiting via TQ-LMT input (same limit value for forward and reverse

**directions)**
When P1-10 =3, the forward and reverse torque limit is determined by the logic state of the torque limit
input TQ-LMT.
◆ Users will need to configure the logic of the TQ-LMIT input according to their application (normally open/
closed). However, when the primary TQ-LMT logic state is established (valid), forward and reverse torque
limits are defined by P1-06. If the primary TQ-LMT logic state is not established (invalid), forward and reverse
torque limits are defined by P1-25.
◆ Users will need to configure the logic of the TQ-LMIT input according to their application (normally open/
closed). However, when the primary TQ-LMT logic state is established (valid), forward and reverse torque
limits are defined by P1-06. If the primary TQ-LMT logic state is not established (invalid), forward and reverse
torque limits are defined by P1-25.
Related parameters

| Parameter | Instruction | Name | Value range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P1-06 | CC | First torque limit | 0~3000 | 3000 | 0.1% | First torque limit of the motor |
| P1-25 | CY | Second torque limit | 0~3000 | 3000 | 0.1% | Second torque limit for motor |

Note: If torque limits are set too low, there may be insufficient torque available for acceleration and
deceleration.

| TQ-LMT input valid |  | TQ-LMT input invalid |
|---|---|---|
| Motor Maximum Torque<br>P1-06<br>-P1-06<br>Motor maximum torque | Time | Motor Maximum Torque<br>P1-25<br>Time<br>-P1-25<br>Motor maximum torque |
| When P1-10 = 3 and the TQ-LMT input is valid, the forward<br>and reverse torque limits are defined by P1-06. |  | When P1-10 = 3 and the TQ-LMT input is invalid, the forward and<br>reverse torque limits are defined by P1-25. |

##### 7.5.1.5 Torque limiting via TQ-LMT input (forward and reverse limits assigned differing values)

When P1-10=5, the forward and reverse torque limits are determined by the logic state of the TQ-LMT input.
Users will need to configure the primary logic of the TQ-LMT beforehand (normally open/closed). Regardless
of which logic they choose, when the TQ-LMT input is valid or invalid, based on the selected logic, the
forward and reverse limits can be assigned different values from one another in each state of the TQ-LMT
input. This means users will have 2 possible limits in the forward direction and 2 possible limits in the reverse
direction.
Related parameters

| Parameter | Instruction | Name | Value range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P1-06 | CC | First torque limit | 0~3000 | 3000 | 0.1% | First torque limit of the motor |
| P1-25 | CX | Second torque limit | 0~3000 | 3000 | 0.1% | Second torque limit for motor |
| P1-26 | CY | Third torque limit | 0~3000 | 3000 | 0.1% | Third torque limit for motor |
| P1-27 | CZ | Fourth torque limit | 0~3000 | 3000 | 0.1% | Fourth torque limit for motor |

Note: If torque limits are set too low, there may be insufficient torque available for acceleration and
deceleration.

| TQ-LMT torque limit input valid |  | TQ-LMT torque limit input is invalid |  |
|---|---|---|---|
| Motor Maximum Torque<br>P1-06<br>-P1-25<br>Motor maximum torque | Time | Motor Maximum Torque<br>P1-26<br>-P1-27<br>Motor maximum torque | Time |
| When P1-10 = 5. The TQ-LMT torque limit input is invalid, the<br>motor output forward torque is limited to the set value of P1-06,<br>and the reverse torque is limited to the set value of P1-25. |  | When P1-10 = 5. The TQ-LMT torque limit input is valid, the motor<br>output forward torque is limited to the set value of P1-26, and the<br>reverse torque is limited to the set value of P1-27 |  |

<!-- page 91 -->

#### 7.5.2 Torque limit reached output

Related parameters

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| Output | T-LMT | 15 | Closed | The torque limit is reached, the signal is output, the state of the output is<br>closed. |
|  |  |  | Open | The torque limit is not reached, the signal is not output, the state of the<br>output is open. |
|  |  | 16 | Open | The torque limit is reached, the signal is output, the state of the output is<br>open. |
|  |  |  | Closed | The torque limit is not reached, the signal is not output, the state of the<br>output is closed. |

output is closed.
Note: Please refer to 7.1.2 Output signal configuration

### 7.6 Encoder feedback output function

The encoder feedback is output in the form of A/B/Z differential signals. A/B/Z output signals and
the frequency division ratio that affects the output frequency of the encoder feedback can be
configured via parameters.
Related parameters

| Parameter | Instruction | Name | Value range | Defaults | Description |
|---|---|---|---|---|---|
| P3-12 | PO | Encoder feedback output mode | 0 ~ 256 | 1 | Pulse frequency division output<br>setting |
| P3-13 | ON | Pulse output gear ratio -<br>numerator | 0 ~ 65535 | 10000 | Set the numerator of the pulse<br>frequency division output ratio |
| P3-14 | OD | Pulse output gear ratio -<br>denominator | 0 ~ 65535 | 65535 | Set the denominator of the pulse<br>frequency division output ratio |

#### 7.6.1 Pulse frequency division output signal pin

| Encoder Output-<br>pin number | Signal<br>name | Description | Wiring |
|---|---|---|---|
| 1 | AOUT+ | The encoder feedback is output in the form of<br>A/B/Z differential signals. The number of pulses<br>per revolution and the frequency division ratio<br>that affects the output frequency of the encoder<br>feedback can be set via parameters. | Refer to chapter 4.10.4<br>Encoder feedback output |
| 2 | AOUT- |  |  |
| 3 | BOUT+ |  |  |
| 4 | BOUT- |  |  |
| 5 | ZOUT+ |  |  |
| 6 | ZOUT- |  |  |

**Note:**
1. The output circuit is line driver type and the signals are 5V differential. To accept these
differential signals, users will need to use a differential receiver. If this is not possible, then users
will need to use a differential-to single-ended signal converter. Do not connect the OUT+ or OUT-
to a power supply.
2. For reliable noise immunity, the cable harness used to transmit the encoder signals should be
shielded, twisted pair wires. The shield cable should be connected to the PE port and the digital
digital ground (DGND) connected to the digital ground of the computer.
3. Output signals are 5V differential. The maximum allowable current for each output pin is 20 mA.

<!-- page 92 -->

#### 7.6.2 Pulse frequency division output mode setting

When using the encoder feedback output function, its important that users configure the sequence
of output phases ( A-leads-B or B-leads-A), the polarity of the Z index output and the frequency
division ratio (both numerator and denominator set individually). The frequency division ratio
impacts the pulses per revolution output by the drive.
Use parameter P3-12 to set the output pulse source, output pulse phase, and Z pulse output
polarity type. The functions corresponding to each bit are as follows.

| Parameter P3-12 Pulse frequency division output mode |  |  |  |  |  |  |  |
|---|---|---|---|---|---|---|---|
| bit7 | bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0 |
| 0 | 0 | 0 | 0 | Z pulse output polarity | A and B phase<br>relationship during<br>forward rotation | output pulse source |  |
|  |  |  |  | 0: positive polarity | 0: A leads B 90 ° | bit1=0, bit0=1: Motor encoder |  |
|  |  |  |  | 1: Negative polarity | 1: B leads A 90 ° |  |  |

##### 7.6.2.1 Encoder feedback output settings

| Z Pulse<br>Polarity | A/B phase<br>relationship | Source of<br>feedback | Direction of Motion (Forward) | Direction of motion (reverse) | P3-12<br>set<br>value |
|---|---|---|---|---|---|
| bit3 | bit2 | bit1=0，bit0=1 |  |  |  |
| 0 | 0 | Motor encoder | Phase A<br>Phase B<br>Z Phase | Phase A<br>Phase B<br>Z Phase | 1 |
| 0 | 1 | Motor encoder | Phase A<br>Phase B<br>Z Phase | Phase A<br>Phase B<br>Z Phase | 5 |
| 1 | 0 | Motor encoder | Phase A<br>Phase B<br>Z Phase | Phase A<br>Phase B<br>Z Phase | 9 |
| 1 | 1 | Motor encoder | Phase A<br>Phase B<br>Z Phase | Phase A<br>Phase B<br>Z Phase | 13 |

<!-- page 93 -->

#### 7.6.3 Pulse output gear ratio

The number of pulses output from the MBDV drive per motor revolution can be configured by
parameters P3-13 and P3-14.
P3-13 Pulse output gear ratio - numerator
Number of output pulses per revolution = x 65536
P3-14 Pulse output gear ratio - denominator

Note: The number of output pulses per revolution refers to the frequency multiplied by 4 of the A/B
phase
Related parameters

| Parameter | Instruction | Name | Value<br>range | Defaults | Description |
|---|---|---|---|---|---|
| P3-13 | ON | Pulse output gear ratio -<br>numerator | 0 ~ 65535 | 10000 | Sets the numerator of the pulse output<br>gear ratio |
| P3-14 | OD | Pulse output gear ratio -<br>denominator | 0 ~ 65535 | 65535 | Sets the denominator of the pulse output<br>gear ratio |

**Note:**
1). The numerator P3-13 must be less than the denominator P3-14
2). When P3-13 > P3-14, the number of pulses output per motor revolution (post quadrature) = P3-13
Example:
The following are different ways to attain a output pulse resolution of 1000 pulses per revolution of
the motor. Pulses per revolution are taken post quadrature.
**1). If A/B is counted at the same time, and the frequency is multiplied by 4:**
P3-13 = 1000
P3-14 = 65535 or P3-14 = 1
**2). If A/B are counted at the same time, and only the rising edge or falling edge is counted**
**when counting**
Then: P3-13 = 2000
P3-14 = 65535 or P3-14 = 1
**3). If only the A -phase output is counted, and only the rising edge or falling edge is counted**
**when counting**
Then: P3-13 = 4000
P3-14 = 65535 or P3-14 = 1

<!-- page 94 -->

### 7.7 Dynamic braking

Dynamic braking can be used when a fault occurs at the motor or at the drive.. Dynamic braking is
implemented by short circuiting the U/V/W phases of the motor. This brings the motor to a stop at
the highest deceleration rate and is meant to protect personnel and equipment.

| Stopping without dynamic braking | Stopping with dynamic braking |
|---|---|
| Command Speed<br>Speed[rpm] Actual Speed<br>1250<br>1000<br>750<br>500<br>250<br>0<br>-250<br>0 250 500 750 1000 1250 1500<br>Time[ms] | Speed[rpm] Com A m c a tu n a d l S S p p e e e e d d<br>1250<br>1000<br>750<br>500<br>250<br>0<br>-250<br>0 200 400 600 800<br>Time[ms] |
| In this scenario, the driver exhibits a fault and is disabled. This<br>results in the motor coming to an uncontrolled deceleration that<br>is influenced purely by external factors such as the speed of the<br>motor before fault, inertia of the system and the friction present in<br>the system. | In this scenario, the driver exhibits a fault and the driver is<br>disabled. The phases (U/V/W) of the servo motor are shorted and<br>the current generated by the back EMF of the motor windings is<br>used to stop the motor. This greatly reduces deceleration time and<br>protects personnel as well as equipment. |

| Command Speed<br>Actual Speed |
|---|

| Command Speed<br>Actual Speed |
|---|

**Note:**
1. Dynamic Braking with Quick Stop
- Do not start and stop the motor through the servo enable /off function.
2. Dynamic braking is only suitable for short-term use, and it is only used in the case of abnormal
servo OFF, drive error, etc.
- After the dynamic brake is used to stop at high speed, it can be used again after an interval
of 10 minutes.
3. Dynamic Braking function disabled in case of drive power loss
Related parameters

| Parameter | Instruction | Name | Value<br>range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P1-29 | YV | The action of the dynamic<br>brake when it is disabled | 0 ~ 5 | 0 | - | Dynamic braking sequence when drive<br>receives Servo Off signal |
| P1-30 | YR | The action of the dynamic<br>brake when an error is<br>reported | 0 ~ 3 | 0 | - | Dynamic braking sequence when fault<br>occurs at the drive. |
| P1-31 | YM | The maximum action time<br>of the dynamic brake<br>during the deceleration<br>process of the deactivation | 0 ~ 30000 | 500 | ms | Maximum time of dynamic braking when<br>drive receives Servo Off signal. |
| P1-32 | YN | The longest action time of<br>the dynamic brake during<br>the deceleration process of<br>the error | 0 ~ 30000 | 0 | ms | Maximum time of dynamic braking when<br>a fault occurs at the drive. |
| P1-37 | DV | Dynamic braking action<br>speed | 0 ~100 | 50 | rps | Maximum motor velocity at which<br>dynamic braking will be activated. |
| P5-42 | ZV | Zero speed judgment<br>threshold | 0.1 ~ 2 | 0.5 | rps | When the speed is less than or equal to<br>this set value, the drive considers that it<br>is in the zero-speed state at this time |

<!-- page 95 -->

#### 7.7.1 Dynamic braking when Servo Off signal triggered

Servo is OFF, the dynamic braking action is set by parameter P1-29, and the longest action time
during deceleration is set by parameter P1-31, please refer to the table below. The deceleration
process means that the actual speed of the motor decelerates from the speed of parameter P1-37
to within the zero-speed threshold of parameter P5-42, or the deceleration time reaches the set
time of P1-31 when the dynamic braking takes effect.

| Value | Description |  |
|---|---|---|
|  | Deceleration process | After stop |
| 0 | According to the setting of<br>parameter P2-01 | Remain in free spin |
| 1 | According to the setting of<br>parameter P2-01 | Dynamic braking |
| 2 | Free spin | Remain in free spin |
| 3 | Free spin | Dynamic braking |
| 4 | Dynamic braking | Remain in free spin |
| 5 | Dynamic braking | Dynamic braking |

#### 7.7.2 Dynamic braking when driver fault present

Servo reports an error, the dynamic braking action is set by parameter P1-30, and the longest
action time in the deceleration process is set by P1-32, please refer to the table below. The
deceleration process means that when the dynamic braking takes effect, the actual speed of
the motor decelerates from the speed of parameter P1-37 to within the zero-speed threshold of
parameter P5-42, or the deceleration time reaches the set time of P1-31.

| Value | Description |  |
|---|---|---|
|  | Deceleration process | Stop |
| 0 | Free spin | Remain in free spin |
| 1 | Freespin | Dynamic braking |
| 2 | Dynamic braking | Remain in free spin |
| 3 | Dynamic braking | Dynamic braking |

### 7.8 Home function

When the home function is executed, the drive generates a motion profile based on parameters
such as homing acceleration, deceleration, velocity, the home position, and the home offset
position. The MBDV supports up to 39 different homing modes.

Homing Velocity
Restricted
Homing Acceleration/Deceleration
Function

Maximum Acceleration(P2-01), Maximum Velocity(P2-00) Restricted Motion Command
Function
Profile Position
Generation

Homing Method(P5-49), Origin Offset(P2-27)

Home Sensor (P5-00 ~ P5-03)

<!-- page 96 -->

There are three ways to enable homing:
◆ Digital input start (S-HOM)

| Type | Signal name | Set value | Signal logic | Function |
|---|---|---|---|---|
| Input | S-HOM | 15 | Closed | The S-HOM function is enabled on the rising edge of the signal and<br>starts to return to the origin |
|  |  |  | Open | S-HOM function is not enabled |
|  |  | 16 | Open | The S-HOM function is enabled on the falling edge of the signal and<br>starts to return to the origin |
|  |  |  | Closed | S-HOM function is not enabled |

◆ Using the Q program command
◆ Using streamed commands (CANOpen or SCL)
Related Parameters

| Parameter | Instruction | Name | Value range | Defaults | Unit | Description |
|---|---|---|---|---|---|---|
| P5-49 | HE | Homing mode | -4 ~ 35 | 1 | - | Choose a return to origin mode |
| P2-18 | HA1 | 1st Homing acceleration/<br>deceleration | 0.167 ~ 5000 | 100 | rps/s | Set acceleration/deceleration<br>during return to origin |
| P2-24 | HV1 | Homing Velocity 1 | 0.0042 ~ 100 | 10 | rps | Set the first speed of the return to<br>the origin. |
| P2-25 | HV2 | Homing Velocity 2 | 0.0042 ~ 100 | 1 | rps | Set the first speed of the return to<br>the origin. |
| P2-27 | HO | Home offset | -2147483647 ~<br>+2147483647 | 0 | pulses | Sets the offset position after the<br>origin is found when returning to<br>the origin |
| P2-00 | VM | Maximum speed | 0 ~ 100 | 80 | rps | Maximum speed limit, limiting<br>motor speed in all control modes |
| P2-01 | AM | Maximum Acceleration/<br>Deceleration | 0.167 ~ 5000 | 3000 | rps/s | maximum deceleration during<br>emergency stop |
| P5-00 ~<br>P5-03 | MU1 ~ MU4 | Digital input port function | 39 ~ 40 | - | - | Set one of the numeric inputs<br>X1~X4 as the origin sensor |

<!-- page 97 -->

#### 7.8.1 Back to the origin basic concept

Back-to-origin is used to find the mechanical origin and locate the positional relationship between
the mechanical origin and the mechanical zero. Mechanical origin: a fixed position on the machine,
which can be a certain sensor or the Z - phase signal of the motor. Mechanical zero point: the
absolute 0 position on the machine.
After returning to the origin, the position where the motor stops are the machine origin. By setting the
origin offset P2-27, the relationship between the machine origin and the machine zero can be set:
Mechanical origin = mechanical zero + P2-27
When P2-27=0, the mechanical origin coincides with the mechanical zero.

H
Movement track H

Z-Phase

Signal Home

Positive Limit Switch Signal

Negative Limit Switch Signa
H: The first speed of returning to the origin P2-24
L: The first speed of returning to the origin P2-25
Origin switch signal: set digital input X1 ~ One of the inputs in X4 is the origin switch, HOM-SW=0
means the origin signal is invalid, HOM-SW=1 means the origin signal is valid.
Positive limit switch signal: set digital input X1 ~ One of the inputs in X4 is the positive limit switch,
POT=0 means the positive limit signal is invalid, POT=1 means the positive limit signal is valid.
Negative limit switch signal: set digital input X1 ~ One of the inputs in X4 is the negative limit
switch, NOT=0 means the negative limit signal is invalid, and NOT=1 means the negative limit signal
is valid.

#### 7.8.2 Introduction to the way of returning to the origin

◆ The back-to-origin mode -4~-1 is the manufacturer-defined back-to-origin mode. The driver does
not need an external switch signal as an auxiliary signal for back-to-origin, but limits the torque of
the motor during the process of back-to-origin. When the mechanical hard limit is connected to the
motor-driven When the load is contacted and blocked, the thrust generated by the motor-driven load
is equal to the blocking force, and when the motor is stationary, the position is considered as the
mechanical origin. The torque limit of the motor during the back-to-origin process is set through P0-
08 (torque limit of the hard limit back-to-origin mode), 100% corresponds to 1 time the rated torque of
the motor; set the value of this object according to the actual application, too small setting value may
result in inaccurate return-to-origin position, and too large setting value may damage mechanical
equipment.
Note: When using the back-to-origin method -4~-1, it is necessary to set a suitable back-to-origin
offset P2-27, so that after finding the mechanical origin during the process of back-to-origin, the
distance of the origin offset P2-27 is reversed, and the load is away from the mechanical hardware.
Limit, the actual position after the motor stops is 0.
◆ The homing mode 1~35 is the homing mode defined according to the CiA402 motion control
protocol.
Note: When using the back-to-origin method 1~35, after the motor back-to-origin is completed, the
actual position of the motor is the value of the origin offset P2-27.

<!-- page 98 -->

##### 7.8.2.1 Return-to-origin method -4: forward return, look for forward mechanical hard limit

H

| H |
|---|
| Offset |

Movement Track

Positive Mechanical Hard Limit

a) Start the return at positive high speed, decelerate to stop when the mechanical hard limit meets
the torque equal to the blocking force and the motor limit, run at the negative high speed by the
distance of the origin offset P2-27, and the position of the motor is 0 after stop.

##### 7.8.2.2 Return-to-origin method -3: Negative return, looking for negative mechanical hard

**limit**

H

| H |
|---|
| Offset |

Movement Track

Offset

Negative mechanical hard limit

a) Start the return at a negative high speed, decelerate and stop when the mechanical hard limit
meets the torque equal to the blocking force and the motor limit, run at a positive high speed by the
distance of the origin offset P2-27, and the position of the motor is 0 after stopping.

##### 7.8.2.3 Return-to-origin method -2: forward return, look for forward mechanical hard limit

**and Z -phase pulse signal**

Movement Track
H

| H | L |
|---|---|
| Offset |  |

Z-phase Signal

Positive Mechanical Hard Limit

a) Start the return at a positive high speed, decelerate and stop when the mechanical hard limit
meets the torque equal to the blocking force and the motor limit, run at a negative low speed, stop
when encountering the first Z pulse, and run at a negative high-speed origin offset the distance of
P2-27, the position of the motor after stop is 0.

<!-- page 99 -->

##### 7.8.2.4 Back-to-origin mode -1: Negative return, looking for negative mechanical hard limit

**and Z -phase pulse signal**

H
Movement Track

| L | H |
|---|---|
|  | Offest |

Z-phase Signal

Negative mechanical hard limit

a) Start the return at a negative high speed, decelerate and stop when the mechanical hard limit
meets the torque limit of the blocking force and the motor limit, run at a positive low speed, stop at
the first Z pulse, and run at a positive high-speed origin offset the distance of P2-27, the position of
the motor after stop is 0.

##### 7.8.2.5 Return to origin mode 1: Negative return, looking for negative limit and Z pulse

**signal**

H

| L |
|---|

Movement Track
L

Z-phase Signal

Negative Limit Signal

a) When starting the return, NOT=0, start the return at a high speed in the negative direction, after
encountering the rising edge of NOT, decelerate, reverse, and run at a low speed in the forward
direction.
The first Z pulse after the falling edge of NOT stops.
b) When starting the return, NOT=1, start the return at a forward low speed, and stop at the first Z
pulse after encountering the falling edge of NOT.

<!-- page 100 -->

##### 7.8.2.6 Back-to-origin method 2: forward return, look for positive limit and Z pulse signal

H

| L |
|---|

Movement Track
L

Z-phase Signal

Positive Limit Signal

a) POT=0 when starting to return, start returning at high speed in positive direction, after
encountering the rising edge of POT, decelerate, reverse, and run at low speed in negative
direction.
The first Z pulse after the falling edge of POT stops.
b) When starting to return, POT=1, start to return at a negative low speed, and stop at the first Z
pulse after encountering the falling edge of POT.

##### 7.8.2.7 Back-to-origin method 3: forward return, look for the origin sensor falling edge and

**Z pulse signal**

H

| L |
|---|

Movement Track

L

Z-phase Signal

Home Switch Signal

a) When starting to return, HOM-SW=0, start the return at high speed in the positive direction,
after encountering the rising edge of HOM-SW, decelerate, reverse, and run at low speed in the
negative direction, and stop at the first Z pulse after encountering the falling edge of HOM-SW.
b) When starting to return, HOM-SW=1, start the return at a negative low speed, and stop at the
first Z pulse after encountering the falling edge of HOM-SW.

<!-- page 101 -->

##### 7.8.2.8 Return to origin mode 4: forward return, look for the rising edge of the origin sensor

**and the Z pulse signal**

H

Movement Track
L

| L |
|---|
| L |

Z-phase Signal

Home Switch Signal

a) When starting to return, HOM-SW=0, start the return at high speed in positive direction, after
encountering the rising edge of HOM-SW, decelerate, reverse, and run at low speed in negative
direction to the position where HOM-SW is invalid, then decelerate to stop, and then move forward
again Run at low speed and stop at the first Z pulse after encountering the rising edge of HW.
b) When starting to return, HOM-SW=1, start the return at low speed in negative direction, after
encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in the forward
direction, and stop at the first Z pulse after encountering the rising edge of HOM-SW.

##### 7.8.2.9 Return to origin mode 5: Negative return, look for the falling edge of the origin

**sensor and the Z pulse signal**

H

| L |
|---|

Movement Track
L

Z-phase Signal

Home Switch Signal

a) When starting to return, HOM-SW=0, start returning at high speed in negative direction, after
encountering the rising edge of HOM-SW, decelerate, reverse, and run at low speed in the forward
direction, and stop at the first Z pulse after encountering the falling edge of HOM-SW.
b) When starting to return, HOM-SW=1, start the return at a forward low speed, and stop at the first
Z pulse after encountering the falling edge of HOM-SW.

<!-- page 102 -->

##### 7.8.2.10 Return to origin mode 6: Negative return, look for the rising edge of the origin

**sensor and the Z pulse signal**

H
Movement Track
L

| L |
|---|

L

Z-phase Signal

Home Switch Signal

a) When starting to return, HOM-SW=0, start the return at high speed in negative direction, after
encountering the rising edge of HOM-SW, decelerate, reverse, and run at low speed in the forward
direction to the position where HOM-SW is invalid, then decelerate and stop, and then go negative
again Run at low speed and stop at the first Z pulse after encountering the rising edge of HW.
b) When starting to return, HOM-SW=1, start returning at low speed in positive direction, after
encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in negative
direction, and stop at the first Z pulse after encountering the rising edge of HOM-SW.

##### 7.8.2.11 Return to origin mode 7: forward return, look for the falling edge of the origin

**sensor and Z pulse signal, and automatically reverse when encountering the positive limit**

H

| L |  |
|---|---|
|  | L |

Movement Track

Z-phase Signal

Home Switch Signal

Positive Limit Switch Signal

a) When starting to return, HOM - SW=0 and it is located on the negative side of the origin sensor
position, and starts to return at a positive high speed. The first Z pulse after the falling edge of SW
stops.
b) When starting to return, HOM -SW=0 and it is located on the positive side of the origin sensor,
and starts to return at high speed in the positive direction. After encountering the rising edge of POT,
decelerate, reverse, and run at high speed in the negative direction; After the rising edge, decelerate,
reverse, forward and run at low speed to HOM-SW After the invalid position, it will decelerate and stop,
and then run at a low speed in the negative direction, and stop at the first Z pulse after encountering
the falling edge of HOM-SW.
c) When starting to return, HOM-SW=1, start the return at a negative low speed, and stop at the first Z
pulse after encountering the falling edge of HOM-SW.

<!-- page 103 -->

##### 7.8.2.12 Return to origin mode 8: forward return, look for the rising edge of the origin sensor

**and Z pulse signal, and automatically reverse when encountering the positive limit**

H

L

| L |
|---|
| L |
| L |

Movement Track

Z-phase Signal

Home Switch Signal

Positive Limit Switch Signal

a) When starting to return, HOM-SW=0 and it is located on the negative side of the origin sensor
position, and starts to return at high speed in the positive direction. After encountering the rising
edge of HOM-SW, decelerate, reverse, and run at low speed in the negative direction until the
HOM-SW is invalid. After the position, it decelerates to stop, and then runs forward at a low speed,
and stops at the first Z pulse after the rising edge of HOM-SW.
b) When starting to return, HOM -SW=0 and it is located on the positive side of the origin sensor,
and starts to return at high speed in the positive direction. After encountering the rising edge of
POT, decelerate, reverse, and run at high speed in the negative direction; Decelerate after falling
edge, reverse and forward at low speed, encounter HOM-
The first Z pulse after SW rises stops.
c) When starting to return, HOM-SW=1, start the return at low speed in negative direction, after
encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in the forward
direction, and stop at the first Z pulse after encountering the rising edge of HOM-SW.

<!-- page 104 -->

##### 7.8.2.13 Return-to-origin method 9: forward return, look for the rising edge of the origin

**sensor and Z pulse signal, and automatically reverse when encountering the positive limit**

H

| L |
|---|
| L |
| L |
| L |

Movement Track

Z-phase Signal

Home Switch Signal

Positive Limit Switch Signal

a) When starting to return, HOM-SW=0 and it is located on the negative side of the position of
the origin sensor, start the return at a positive high speed, and encounter a drop in HOM-SW
Decelerate after the edge, reverse, and run at low speed in the negative direction, and stop at the
first Z pulse after the rising edge of HOM-SW.
b) When starting to return, HOM-SW=0 and it is located on the positive side of the position of the
origin sensor, start the return at a positive high speed, and encounter the rising edge of POT Then,
decelerate, reverse, and run at high speed in negative direction; after encountering the rising
edge of HOM-SW, decelerate, reverse, and run at low speed in forward direction until HOM-SW
is invalid. After the position, it decelerates to stop, and then runs at a low speed in the negative
direction, and stops at the first Z pulse after encountering the rising edge of HOM-SW.
c) HOM-SW=1 when starting to return, start returning at low speed in positive direction, after
encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in negative
direction, The first Z pulse after encountering the rising edge of HOM-SW stops.

<!-- page 105 -->

##### 7.8.2.14 Return to origin mode 10: Forward return, look for the falling edge of the origin

**sensor and Z pulse signal, and automatically reverse when encountering the positive limit**

H

L

| L |  |
|---|---|
|  | L |

Movement Track

Z-phase Signal

Home Switch Signal

Negative limit switch signal

a) When starting to return, HOM-SW=0 and it is located on the negative side of the origin sensor
position, start the return at a positive high speed, and encounter HOM-SW rising After the edge,
decelerate, reverse, and run at a low speed in the negative direction to the position where HOM-
SW is invalid, then decelerate to stop, and then run at a low speed in the positive direction.
The first Z pulse after the falling edge of SW stops.
b) When starting to return, HOM-SW=0 and it is located on the positive side of the position of
the origin sensor, start the return at a positive high speed, and encounter the rising edge of POT
Then, decelerate, reverse, and run at high speed in the negative direction; after encountering the
rising edge of HOM-SW, decelerate, reverse, and run at low speed in the forward direction, and
encounter the lower speed of HOM-SW. The first Z pulse after the falling edge stops.
c) When starting to return, HOM-SW=1, start the return at a forward low speed, and stop at the first
Z pulse after encountering the falling edge of HOM-SW.

<!-- page 106 -->

##### 7.8.2.15 Return-to-origin method 11: Negative return, look for the falling edge of the origin

**sensor and Z pulse signal, and automatically reverse in case of negative limit**

H

|  | L |
|---|---|
| L |  |

Movement Track

Z-Phase Signal

Home Switch Signal

Negative Limit Switch Signal

a) When starting to return, HOM-SW=0 and it is located on the positive side of the position of the
origin sensor, and starts to return at a high speed in the negative direction. After encountering the
rising edge of HOM-SW, Decelerate, reverse, and run at low speed in the forward direction, and
stop at the first Z pulse after the falling edge of HOM-SW is encountered.
b) HOM-SW=0 when starting to return and it is on the negative side of the position of the origin
sensor , start the return at a high speed in the negative direction , after encountering the rising
edge of NOT , reduce Speed , reverse , and forward high-speed operation ; after encountering the
rising edge of HOM-SW , decelerate , reverse , and negatively run at low speed to the position
where HOM-SW is invalid, then reduce Stop at high speed , then run forward at low speed , and
stop at the first Z pulse after encountering the falling edge of HOM-SW .
c) When starting to return, HOM-SW=1, start the return at a forward low speed, and stop at the first
Z pulse after encountering the falling edge of HOM-SW.

<!-- page 107 -->

##### 7.8.2.16 Return-to-origin mode 12: Negative return, look for the rising edge of the origin

**sensor and Z pulse signal, and automatically reverse the direction of the negative limit**

H

| L |
|---|
| L |
| L |
| L |

Movement Track

Z-Phase Signal

Home Switch Signal

Negative Limit Switch Signal

a) When starting to return, HOM-SW=0 and it is located on the positive side of the position of the
origin sensor, and starts to return at a high speed in the negative direction. After encountering
the rising edge of HOM-SW, Decelerate, reverse, and run at low speed in the forward direction to
the position where HOM-SW is invalid, then decelerate to stop, and then run at low speed in the
negative direction. After encountering the rising edge of HOM-SW the first Z pulse stops.
b) HOM-SW=0 when starting to return and it is on the negative side of the position of the origin
sensor, start the return at a high speed in the negative direction, after encountering the rising
edge of NOT, reduce Speed, reverse, forward high-speed operation; after encountering the falling
edge of HOM-SW, decelerate, reverse, and run at low speed in negative direction, the first after
encountering the rising edge of HOM-SW Z pulses stop.
c) When starting to return, HOM-SW=1, start the return at low speed in the positive direction,
after encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in the
negative direction.
The first Z pulse after the rising edge of HOM-SW stops.

<!-- page 108 -->

##### 7.8.2.17 Return to origin mode 13: Negative return, look for the rising edge of the origin

**sensor and the Z pulse signal, and automatically reverse when the negative limit is**
**encountered**

H

| L |
|---|
| L<br>L |

Movement Track

Z-Phase Signal

Home Switch Signal

Negative Limit Switch Signal

a) When starting to return, HOM-SW=0 and it is located on the positive side of the position of the
origin sensor, and it starts to return at a high speed in the negative direction. When it encounters
the lower part of HOM-SW After the falling edge, decelerate, reverse, and run at low speed in the
forward direction, and stop at the first Z pulse after the rising edge of HOM-SW.
b) HOM-SW=0 when starting to return, and it is located on the negative side of the origin sensor
position, and starts to return at a high speed in negative direction, and encounters the rising edge
of NOT. Then, decelerate, reverse, and run at high speed in forward direction; after encountering
the rising edge of HOM-SW, decelerate, reverse, and run at low speed in negative direction until
HOM-SW is invalid. After the position, it decelerates and stops, and then runs forward at a low
speed, and stops at the first Z pulse after the rising edge of HOM-SW.
c) HOM-SW=1 when starting to return, start returning at low speed in negative direction, after
encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in forward
direction, The first Z pulse after encountering the rising edge of HOM-SW stops.

<!-- page 109 -->

##### 7.8.2.18 Return to origin mode 14: Negative return, look for the falling edge of the origin

**sensor and Z pulse signal, and automatically reverse when encountering the negative limit**

H

H L

| L<br>L |
|---|

Movement Track

Z-Phase Signal

Home Switch Signal

Negative Limit Switch Signal

a) When starting to return, HOM-SW=0 and it is located on the positive side of the position of
the origin sensor. It starts to return at a high speed in the negative direction, and when the HOM-
SW rises After the edge, decelerate, reverse, and run at low speed in the forward direction to
the position where HOM -SW is invalid, then decelerate to stop, and then run at low speed in the
negative direction.
The first Z pulse after the falling edge of SW stops.
b) HOM-SW=0 when starting to return, and it is located on the negative side of the origin sensor
position, and starts to return at a high speed in negative direction, and encounters the rising
edge of NOT. Then, decelerate, reverse, and run at high speed in the forward direction; after
encountering the rising edge of HOM-SW, decelerate, reverse, and run at low speed in the
negative direction, and encounter the lower speed of HOM-SW. The first Z pulse after the falling
edge stops.
c) When starting to return, HOM-SW=1, start the return at a negative low speed, and stop at the
first Z pulse after encountering the falling edge of HOM-SW.

<!-- page 110 -->

##### 7.8.2.19 Return to origin mode 15, 16 reserved

##### 7.8.2.20 Return to origin mode 17: Negative return, looking for negative limit signal

H
Movement Track L
L

Negative Limit Switch Signal

a) When starting the return, NOT=0, start the return at a high speed in the negative direction, after
encountering the rising edge of NOT, decelerate, reverse, and run at a low speed in the forward
direction.
Stop after falling edge of NOT.
b) When starting the return, NOT=1, start the return at a forward low speed, and stop after
encountering the falling edge of NOT.

##### 7.8.2.21 Return to origin mode 18: forward return, look for positive limit signal

H
Movement Track L

L

Positive limit switch signal

a) POT=0 when starting to return, start returning at high speed in positive direction, after
encountering the rising edge of POT, decelerate, reverse, and run at low speed in negative
direction, when encountering POT down Stop after falling edge.
b) When starting to return, POT=1, start the return at low speed in negative direction, and stop
after encountering the falling edge of POT.

<!-- page 111 -->

##### 7.8.2.22 Return to origin mode 19: forward return, look for the origin sensor falling edge

**signal**

H
L

Movement Track
L

Home Switch Signal

a) When starting to return, HOM-SW=0, start returning at high speed in positive direction, after
encountering the rising edge of HOM-SW, decelerate, reverse, and run at low speed in negative
direction, and stop after encountering the falling edge of HOM-SW.
b) When starting to return, HOM-SW=1, start the return at low speed in negative direction, and stop
after encountering the falling edge of HOM-SW.

##### 7.8.2.23 Return to origin mode 20: forward return, look for the rising edge signal of the

**origin sensor**

H

Movement Track L

L
L

Home Switch Signal

a) When starting to return, HOM-SW=0, start returning at high speed in positive direction, after
encountering the rising edge of HOM-SW, decelerate, reverse, and run at low speed in negative
direction to the position where HOM-SW is invalid, then decelerate and stop, and then move
forward again It runs at low speed and stops after encountering the rising edge of HOM-SW.

b) When starting to return, HOM-SW=1, start returning at low speed in negative direction, after
encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in the forward
direction, and stop when encountering the rising edge of HOM-SW.

<!-- page 112 -->

##### 7.8.2.24 Return to origin mode 21: Negative return, look for the falling edge signal of the

**origin sensor**

H
L
Movement Track
L

Home Switch Signal

a) When starting to return, HOM-SW=0, start the return at high speed in negative direction, after
encountering the rising edge of HOM-SW, decelerate, reverse, and run at low speed in the forward
direction, and stop after encountering the falling edge of HOM-SW.

b) When starting to return, HOM-SW=1, start the return at a forward low speed, and stop after
encountering the falling edge of HOM-SW.

##### 7.8.2.25 Return to origin mode 22: Negative return, look for the rising edge signal of the

**origin sensor**

H

Movement Track L
L
L

Home Switch Signal

a) When starting to return, HOM-SW=0, start the return at high speed in negative direction, after
encountering the rising edge of HOM-SW, decelerate, reverse, and run at low speed in the forward
direction to the position where HOM-SW is invalid, then decelerate and stop, and then go negative
again It runs at low speed and stops after encountering the rising edge of HOM-SW.
b) When starting to return, HOM-SW=1, start the return at low speed in the positive direction,
after encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in the
negative direction, and stop after encountering the rising edge of HOM-SW.

<!-- page 113 -->

##### 7.8.2.26 Return to origin mode 23: Forward return, look for the falling edge signal of the

**origin sensor, and automatically reverse when encountering the positive limit**

H
L
Movement Track H
H

L

L

Home Switch Signal

Positive limit switch signal

a) When starting to return, HOM-SW=0 and it is located on the negative side of the origin sensor.
Start returning at high speed in positive direction. After encountering the rising edge of HOM-SW,
decelerate, reverse, and run at low speed in negative direction. When encountering HOM-SW Stop
after falling edge.
b) When starting to return, HOM -SW=0 and it is located on the positive side of the origin sensor,
and starts to return at high speed in the positive direction. After encountering the rising edge of
POT, decelerate, reverse, and run at high speed in the negative direction; After the rising edge,
decelerate, reverse, forward and run at low speed to HOM-SW After the invalid position, it will
decelerate and stop, and then run at a low speed in the negative direction, and stop when it
encounters the falling edge of HOM-SW.
c) When starting to return, HOM-SW=1, start the return at low speed in negative direction, and stop
after encountering the falling edge of HOM-SW.

##### 7.8.2.27 Return to origin mode 24: forward return, look for the rising edge signal of the

**origin sensor, and automatically reverse when encountering the positive limit**

H

L
H
Movement Track
H

| L<br>L<br>L |
|---|

Home Switch Signal

Positive limit switch signal

a) When starting to return, HOM-SW=0 and it is located on the negative side of the origin sensor.
Start returning at high speed in positive direction. After encountering the rising edge of HOM-SW,
decelerate, reverse, and run at low speed in negative direction until HOM-SW is invalid. After the
position, it decelerates to stop, and then runs forward at low speed, and stops when it encounters
the rising edge of HOM-SW.

<!-- page 114 -->

b) When starting to return, HOM -SW=0 and it is located on the positive side of the origin sensor,
and starts to return at high speed in the positive direction. After encountering the rising edge of
POT, decelerate, reverse, and run at high speed in the negative direction; Decelerate after falling
edge, reverse and forward at low speed, encounter HOM-
SW rises and stops.
c) When starting to return, HOM-SW=1, start returning at low speed in negative direction, after
encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in the forward
direction, and stop when encountering the rising edge of HOM-SW.

##### 7.8.2.28 Return to origin mode 25: forward return, look for the rising edge signal of the

**origin sensor, and automatically reverse when encountering the positive limit**

H
L

H
H
Movement Track
L

L
L

Home Switch Signal

Positive limit switch signal

a) When starting to return, HOM-SW=0 and it is located on the negative side of the origin sensor.
Start returning at high speed in the positive direction, decelerate after encountering the falling edge
of HOM-SW, run in the reverse direction, and run at low speed in the negative direction, and rise
when HOM-SW is encountered. stop after the edge.
b) When starting to return, HOM -SW=0 and it is located on the positive side of the origin sensor,
and starts to return at high speed in the positive direction. After encountering the rising edge of
POT, decelerate, reverse, and run at high speed in the negative direction; After the rising edge,
decelerate, reverse, forward and run at low speed to HOM-SW After the invalid position, it will
decelerate and stop, and then run at a low speed in the negative direction, and stop when it
encounters the rising edge of HOM-SW.
c) When starting to return, HOM-SW=1, start the return at low speed in the positive direction,
after encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in the
negative direction, and stop after encountering the rising edge of HOM-SW.

<!-- page 115 -->

##### 7.8.2.29 Return-to-origin mode 26: Forward return, look for the falling edge signal of the

**origin sensor, and automatically reverse when encountering the positive limit**

H

L
Movement Track H
H
L

L

Home Switch Signal

Positive limit switch signal

a) When starting to return, HOM-SW=0 and it is located on the negative side of the origin sensor
position, start the return at a positive high speed, and encounter the rising edge of HOM-SW After
that, decelerate, reverse, and run at low speed in negative direction to the position where HOM-
SW is invalid, then decelerate to stop, and then run at low speed in positive direction.
Stop after falling edge of SW.
b) When starting to return, HOM-SW=0 and it is located on the positive side of the position of
the origin sensor, start the return at a positive high speed, and encounter the rising edge of POT
Then, decelerate, reverse, and run at high speed in the negative direction; after encountering the
rising edge of HOM-SW, decelerate, reverse, and run at low speed in the forward direction, and
encounter the lower speed of HOM-SW. Stop after falling edge.
c) When starting to return, HOM-SW=1, start the return at a forward low speed, and stop after
encountering the falling edge of HOM-SW.

<!-- page 116 -->

##### 7.8.2.30 Return to origin mode 27: Negative return, look for the falling edge signal of the

**origin sensor, and automatically reverse when the negative limit is encountered.**

H
L
H
Movement Track H

L
L

Home Switch Signal

Negative limit switch signal

a) When starting to return, HOM-SW=0 and it is located on the positive side of the origin sensor,
and starts to return at a high speed in the negative direction. After encountering the rising edge of
HOM-SW, decelerate, reverse, and run at low speed in the forward direction. When encountering
HOM-SW Stop after falling edge.
b) When starting to return, HOM - SW =0 and it is located on the negative side of the origin sensor,
and it starts to return at a high speed in the negative direction. After the rising edge, decelerate,
reverse, and run at low speed in the negative direction to HOM-SW After the invalid position, it will
decelerate to stop, then run forward at low speed, and stop when it encounters the falling edge of
HOM-SW.
c) When starting to return, HOM-SW=1, start the return at a forward low speed, and stop after
encountering the falling edge of HOM-SW.

<!-- page 117 -->

##### 7.8.2.31 Return to origin mode 28: Negative return, look for the rising edge signal of the

**origin sensor, and automatically reverse when the negative limit is encountered**

H

L
Movement Track H
H

| L<br>L<br>L |
|---|

Home Switch Signal

Negative limit switch signal

a) When starting to return, HOM-SW=0 and it is located on the positive side of the position of the
origin sensor, start the return at a high speed in the negative direction, and encounter the rising
edge of HOM-SW After that, decelerate, reverse, and run at low speed in the forward direction to
the position where HOM -SW is invalid, then decelerate to stop, and then run at low speed in the
negative direction.
Stop after the rising edge of SW.
b) HOM-SW=0 when starting to return, and it is located on the negative side of the origin sensor
position, and starts to return at a high speed in negative direction, and encounters the rising
edge of NOT. Then, decelerate, reverse, and run at high speed in the forward direction; after
encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in the
negative direction, and encounter the falling edge of HOM-SW. Stop after rising edge.
c) HOM-SW=1 when starting to return, start returning at low speed in positive direction, after
encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in negative
direction. Stop after the rising edge of HOM -SW.

<!-- page 118 -->

##### 7.8.2.32 Return to origin mode 29: Negative return, look for the rising edge signal of the

**origin sensor, and automatically reverse when encountering the negative limit**

H
L
H
H
Movement Track
L

L
L

Home Switch Signal

Negative limit switch signal

a) When starting to return, HOM - SW=0 and it is located on the positive side of the origin sensor,
and starts to return at a high speed in the negative direction. Stop after the rising edge of SW.
b) When starting to return, HOM - SW =0 and it is located on the negative side of the origin sensor,
and it starts to return at a high speed in the negative direction. After the rising edge, decelerate,
reverse, and run at low speed in the negative direction to HOM-SW After the invalid position, it will
decelerate to stop, then run forward at low speed, and stop when it encounters the rising edge of
HOM-SW.
c) When starting to return, HOM-SW=1, start the return at low speed in negative direction, after
encountering the falling edge of HOM-SW, decelerate, reverse, and run at low speed in the forward
direction, and stop after encountering the rising edge of HOM-SW.

<!-- page 119 -->

##### 7.8.2.33 Return to origin mode 30: Negative return, look for the falling edge signal of the

**origin sensor, and automatically reverse when the negative limit is encountered.**

H

L
H
Movement Track
H
L
L

Home Switch Signal

Negative limit switch signal

a) When starting to return, HOM-SW=0 and it is located on the positive side of the position of
the origin sensor. It starts to return at a high speed in the negative direction, and when the HOM-
SW rises After the edge, decelerate, reverse, and run at low speed in the forward direction to
the position where HOM -SW is invalid, then decelerate to stop, and then run at low speed in the
negative direction.
Stop after falling edge of SW.
b) HOM-SW=0 when starting to return, and it is located on the negative side of the origin sensor
position, and starts to return at a high speed in negative direction, and encounters the rising
edge of NOT. Then, decelerate, reverse, and run at high speed in the forward direction; after
encountering the rising edge of HOM-SW, decelerate, reverse, and run at low speed in the
negative direction, and encounter the lower speed of HOM-SW. Stop after falling edge.
c) When starting to return, HOM-SW=1, start the return at low speed in negative direction, and stop
after encountering the falling edge of HOM-SW.

<!-- page 120 -->

Back to origin mode 31, 32, 34, 35
◆ Return to origin mode 31, 32 reserved
◆ Return to origin mode 33: Negative return, looking for the first Z pulse signal

L
Movement Track

Z-Phase Signal

a) Start the return at a negative low speed and stop when the first Z pulse signal is encountered.
◆ Return to origin mode 34: forward return, look for the first Z pulse signal

L
Movement Track

Z-Phase Signal

a) Start the return at a forward low speed and stop when the first Z pulse signal is encountered.
◆ Return to origin mode 35: Take the current position as the machine origin

| Offset |
|---|

Negative mechanical hard limit Mechanical Origin Mechanical Zero Positive Mechanical Hard Limit

<!-- page 121 -->

## 8 Parameter setting

### 8.1 Parameter classification

MBDV series low voltage servo has 5 groups of parameters.

| Parameter group | Type | Function |
|---|---|---|
| P0-XX | PID Tuning | Set the gain parameters of the servo |
| P1-XX | Drive Configuration | Configure functions and set various functional parameters<br>of the drive |
| P2-XX | Motion Profile Configuration | Configure motion profile parameters for each control mode. |
| P3-XX | Encoder Settings | Configure parameters related to the encoder and encoder<br>output. |
| P5-XX | Digital I/O Signal Settings | Configure the functions and parameters of digital inputs<br>and outputs. |

and outputs.

### 8.2 List of parameters

**P0-XX: PID Tuning**

| Serial<br>number | Command | Function | Defaults | Range | Unit | Effective |
|---|---|---|---|---|---|---|
| P0-00 | UM | Tuning Mode | 0 | 0 ~ 2 | —— |  |
| P0-01 | LY | Load type | 0 | 0 ~ 10 | —— |  |
| P0-02 | NR | Load inertia ratio | 0 | 0 ~ 100 | —— |  |
| P0-03 | KG | 1st mechanical stiffness level | 5 | 0 ~ 20 | —— |  |
| P0-04 | KX | 2nd mechanical stiffness level | 5 | 0 ~ 20 | —— |  |
| P0-05 | KP | 1st position loop gain | 52 | 0 ~ 20000 | 0.1Hz |  |
| P0-07 | KD | 1st position loop derivative time constant | 2000 | 0 ~ 30000 | ms |  |
| P0-08 | KE | 1st position loop derivative filter | 20000 | 0 ~ 40000 | 0.1Hz |  |
| P0-09 | KL | Velocity feedforward gain | 10000 | -30000 ~ 3000 | 0.01% |  |
| P0-10 | KR | Velocity feedforward filter frequency | 20000 | 0 ~ 40000 | 0.1Hz |  |
| P0-11 | KF | 1st velocity command gain | 10000 | -30000 ~ 3000 | 0.01% |  |
| P0-12 | VP | 1st velocity loop gain | 183 | 0 ~ 30000 | 0.1Hz |  |
| P0-13 | VI | 1st speed loop integral time constant | 189 | 0 ~ 30000 | ms |  |
| P0-14 | KK | Acceleration feed forward gain | 3000 | 0 ~ 10000 | 0.01% |  |
| P0-15 | KT | Acceleration feedforward filter frequency | 20000 | 0 ~ 40000 | 0.1Hz |  |
| P0-16 | KC | 1st command torque filter frequency | 1099 | 0 ~ 40000 | 01Hz |  |
| P0-17 | UP | 2nd position loop gain | 52 | 0 ~ 20000 | 0.1Hz |  |
| P0-19 | UD | 2nd position loop derivative time constant | 2000 | 0 ~ 30000 | ms |  |
| P0-20 | UE | 2nd position loop derivative time constant | 15000 | 0 ~ 40000 | 0.1Hz |  |
| P0-21 | UF | 2nd velocity command gain | 10000 | -30000 ~ 3000 | 0.01% |  |
| P0-22 | UV | 2nd velocity loop gain | 183 | 0 ~ 30000 | 0.1Hz |  |
| P0-23 | UG | 2nd velocity loop integral time constant | 189 | 0 ~ 30000 | ms |  |
| P0-24 | UC | 2nd command torque filter frequency | 1099 | 0 ~ 40000 | 01Hz |  |
| P0-33 | SD | Automatic gain switching method | 0 | 0 ~ 4 | —— |  |
| P0-34 | PN | Gain switching condition - position error | 0 | 0 ~ 2147483647 | Pulses |  |
| P0-35 | VN | Gain switching condition - actual velocity | 0 | 0 ~ 100 | rps |  |
| P0-36 | TN | Gain switching condition - actual torque | 10 | 0 ~ 3000 | 0.1% |  |
| P0-37 | SE1 | Delay time - 2nd Group Gains to 1st Group Gains | 10 | 0 ~ 10000 | ms |  |
| P0-38 | SE2 | Delay time - 1st Group Gains to 2nd Group Gains | 0 | 0 ~ 10000 | ms |  |
| P0-39 | LR | Velocity Feedback Filter | 0 | 0 ~ 3 | —— |  |

<!-- page 122 -->

**Group P1-XX: Drive Configuration**

| Serial number | SCL<br>instruction | Function | Defaults | Range | Unit | Effective |
|---|---|---|---|---|---|---|
| P1-00 | CM | Control mode | 21 | 1,15,21 | - | Effective<br>immediately |
| P1-02 | PM | Control Mode on power up | 10 | 8 ~ 10，13 | - |  |
| P1-03 | JM | Jog Mode | 2 | 1 ~ 2 | - |  |
| P1-05 | GC | Current command for Torque Control | 0 | -3000 ~ 3000 | 0.1% |  |
| P1-06 | CC | First torque limit | 3000 | 0 ~ 3000 | 0.1% |  |
| P1-07 | CV | Target Torque | 0 | 0 ~ 3000 | 0.1% |  |
| P1-08 | HC | Hard stop home current limit | 200 | 0 ~ 3000 | 0.1% |  |
| P1-09 | CL | Current foldback timer | 2000 | 0 ~ 30000 | ms |  |
| P1-10 | LD | Torque limit method | 1 | 1 ~ 5 | - |  |
| P1-11 | DR | Rotation direction | 0 | 0，1 | - |  |
| P1-13 | PR | Protocol | 5 | 1 ~ 511 | - |  |
| P1-14 | TD | Transmit delay | 2 | 0 ~ 20 | ms |  |
| P1-15 | BR | RS-485 communication baud rate | 1 | 1 ~ 5 | - |  |
| P1-16 | DA | RS-485 communication address | 32 | 1 ~ 32 | - |  |
| P1-17 | CO | CAN open cnode ID | 1 | 1 ~ 127 | - |  |
| P1-18 | CB | CAN open communication baud rate | 0 | 0 ~ 7 | - |  |
| P1-19 | ZR | Regenerative resistor resistance | 200 | 10 ~ 32000 | Ω |  |
| P1-20 | ZC | Regenerative resistor power rating | 40 | 1 ~ 32000 | W |  |
| P1-21 | ZT | Regenerative resistor time constant | 1000 | 0 ~ 8000 | ms |  |
| P1-24 | MA | Alarm mask | 4294967295 | 0 ~ 4294967295 | - |  |
| P1-25 | CX | Second torque limit | 3000 | 0 ~ 3000 | 0.1% |  |
| P1-26 | CY | Third torque limit | 3000 | 0 ~ 3000 | 0.1% |  |
| P1-27 | CZ | Fourth torque limit | 3000 | 0 ~ 3000 | 0.1% |  |
| P1-28 | HT | Motor stall protection time | 0 | 0 ~ 30000 | ms |  |
| P1-29 | YV | Motor stall sequence when Servo Off | 0 | 0 ~ 5 |  |  |
| P1-30 | YR | Motor stall sequence when fault occurs | 0 | 0 ~ 3 |  |  |
| P1-31 | YM | The maximum action time of the dynamic<br>brake during the deceleration process of<br>the deactivation | 500 | 0 ~ 30000 | ms |  |
| P1-32 | YN | Maximum dynamic deceleration time<br>when fault occurs | 0 | 0 ~ 30000 | ms |  |
| P1-34 | RT | Current ramp limit | 1000 | 0 ~ 3000 | 0.1% |  |
| P1-37 | DV | Dynamic brake velocity | 50 | 0 ~ 100 | rps |  |
| P1-38 | DW | Dumping voltage trigger value | 650 | 20 ~ 800 | 0.1V |  |
| P1-39 | ZS | Watchdog trigger time | 500 | 0 ~ 10000 | ms |  |
| P1-40 | ZA | Action after watchdog is triggered | 1 | 1 ~ 16 | 1 |  |

<!-- page 123 -->

**P2-XX: Motion Profile Configuration**

| Serial<br>number | SCL<br>command | Function | Defaults | Range | Unit | Effective |
|---|---|---|---|---|---|---|
| P2-00 | VM | Maximum velocity | 80 | 0 ~ 100 | rps |  |
| P2-01 | AM | Maximum acceleration/deceleration | 3000 | 0.167 ~ 5000 | rps/s |  |
| P2-02 | JS | Jog velocity | 10 | -100 ~ 100 | rps |  |
| P2-03 | JA | Jog acceleration | 100 | 0.167 ~ 5000 | rps/s |  |
| P2-04 | JL | Jog deceleration | 100 | 0.167 ~ 5000 | rps/s |  |
| P2-05 | JT | Jerk time | 10 | 0 ~ 125 | ms |  |
| P2-06 | VE | Target Velocity (point-to-point) | 10 | 0.0042 ~ 100 | rps |  |
| P2-07 | AC | Target acceleration (point-to-point) | 100 | 0.167 ~ 5000 | rps/s |  |
| P2-08 | DE | Target deceleration (point-to-point) | 100 | 0.167 ~ 5000 | rps/s |  |
| P2-09 | VC | Velocity change (point-to-point) | 2 | 0 ~ 100 | rps |  |
| P2-10 | JC | Multi-Speed setting (Jog mode) | 2 | -100 ~ 100 | rps |  |
| P2-18 | HA1 | 1st Homing acceleration/deceleration | 100 | 0.167 ~ 5000 | rps/s |  |
| P2-24 | HV1 | Homing velocity 1 | 10 | 0.0042 ~ 100 | rps |  |
| P2-25 | HV2 | Homing velocity 2 | 1 | 0.0042 ~ 100 | rps |  |
| P2-27 | HO | Homing offset | 0 | -2147483647 ~<br>+2147483647 | pulses |  |
| P2-28 | KJ | Jerk filter | 0 | 0 ~ 1000 | ms |  |
| P2-29 | FF | Interpolation filter | 10 | 0 ~ 10 | ms |  |
| P2-30 | VT | Speed limit in torque mode | 80 | 0 ~ 100 | rps |  |

**P3-XX: Encoder Settings**

| Serial<br>number | SCL<br>command | Function | Defaults | Range | Unit | Effective |
|---|---|---|---|---|---|---|
| P3-04 | PF | Position error limit | 100000 | 0 ~ 2147483647 | pulses |  |
| P3-05 | EG | Electronic Gearing | 10000 | 200 ~ 131072 | pulses/rev |  |
| P3-12 | PO | Encoder feedback output mode | 1 | 0 ~ 256 | - |  |
| P3-13 | ON | Pulse output gear ratio - numerator | 10000 | 0 ~ 13107200 | - |  |
| P3-14 | OD | Pulse output gear ratio - denominator | 131072 | 0 ~ 13107200 | - |  |
| P3-15 | ES | Absolute encoder setting | 1 | 0 ~ 1 | - |  |

<!-- page 124 -->

**P5-XX Digital I/O Signal Settings**

| Serial<br>number | SCL<br>command | Function | Defaults | Range | Unit | Effective |
|---|---|---|---|---|---|---|
| P5-00 | MU1 | Digital input 1 function |  | 0 ~ 46 | - |  |
| P5-01 | MU2 | Digital input 2 function |  | 0 ~ 46 | - |  |
| P5-02 | MU3 | Digital input 3 function |  | 0 ~ 46 | - |  |
| P5-03 | MU4 | Digital input 4 function |  | 0 ~ 46 | - |  |
| P5-12 | MO1 | Digital output 1 function |  | 0 ~ 34 | - |  |
| P5-13 | MO2 | Digital output 2 function |  | 0 ~ 34 | - |  |
| P5-14 | MO3 | Digital output 3 function |  | 0 ~ 34 | - |  |
| P5-24 | BD | Movement waiting time after brake release | 200 | 0 ~ 32000 | ms |  |
| P5-25 | BE | Motor disable delay time after brake applied | 200 | 0 ~ 32000 | ms |  |
| P5-27 | HX | Home sensor | 5 | 1 ~ 10 | - |  |
| P5-28 | FI1 | Digital input 1 filter | 1 | 0 ~ 8000 | ms |  |
| P5-29 | FI2 | Digital input 2 filter | 1 | 0 ~ 8000 | ms |  |
| P5-30 | FI3 | Digital input 3 filter | 1 | 0 ~ 8000 | ms |  |
| P5-31 | FI4 | Digital input 4 filter | 1 | 0 ~ 8000 | ms |  |
| P5-38 | PL | Dynamic position error threshold | 10 | 0 ~ 2147483647 | pulses |  |
| P5-39 | PD | Positioning complete error margin | 40 | 0 ~32000 | pulses |  |
| P5-40 | PE | Motion condition timer | 10 | 0 ~ 30000 | ms |  |
| P5-42 | ZV | Zero speed judgment threshold | 0.5 | 0.1 ~ 2 | rps |  |
| P5-43 | VR | Velocity Reached - Permissible fluctuation range | 0.1 | 0 ~ 100 | rps |  |
| P5-44 | VV | Velocity Reached - Minimum threshold | 10 | 0 ~ 100 | rps |  |
| P5-45 | TV | Torque Reached - Permissible fluctuation range | 10 | 0 ~ 3000 | 0.1% |  |
| P5-46 | DG | Absolute arrival position | 10000 | -2147483647 ~<br>+2147483647 | pulses |  |
| P5-47 | LP | Positive software limit | 0 | -2147483647 ~<br>+2147483647 | pulses |  |
| P5-48 | LM | Negative software limit | 0 | -2147483647 ~<br>+2147483647 | pulses |  |
| P5-49 | HE | Homing method | 1 | -4 ~ 40 | - |  |
| P5-50 | EO | E-Stop method | 1 | 1 ~ 8 | - |  |
| P5-51 | MS | zero speed clamp function | 1 | 0 ~ 1 | - |  |

<!-- page 125 -->

### 8.3 Parameter Description

#### 8.3.1 Group P0-XX: PID gain setting

| Parameter | Instruction | Name | Defaults | Range | Unit | Related Patterns |  |
|---|---|---|---|---|---|---|---|
| P0-00 | UM | Tuning Mode | 0 | 0 ~ 2 | --- | P | S |
| Set the parameter tuning method.<br>Set value Parameter setting mode Description note<br>Set the gain value of servo system by In this mode, only modification of P0-03 is<br>0 Tuning Free setting P0-03. valid.Manual adjustment of all other gain<br>and tuning parameters is not allowed.<br>Parameters are automatically set (load In this mode, only P0-03 stiffness<br>inertia, mechanical stiffness and gains). level and P0-02 load inertia ratio are<br>1 Auto Tuning Only the 1st Mechanical Stiffness (P0-03) valid. Manual adjustment of other gain<br>and the Load Inertia Ratio (P0-02) can be parameters is invalid.<br>manually edited.<br>Fine tuning allows users to manually<br>configure all gain, filter and load In this mode, all gain parameters are<br>Fine Tuning characteristics for tuning. A recommended valid.<br>2 method is to first use auto-tuning to get<br>a close estimate of the required tuning<br>parameter values and then manually adjust<br>those estimates until a more accurate<br>tuning profile is reached. |  |  |  |  |  |  |  |

| Set value | Parameter setting mode | Description | note |
|---|---|---|---|
| 0 | Tuning Free | Set the gain value of servo system by<br>setting P0-03. | In this mode, only modification of P0-03 is<br>valid.Manual adjustment of all other gain<br>and tuning parameters is not allowed. |
| 1 | Auto Tuning | Parameters are automatically set (load<br>inertia, mechanical stiffness and gains).<br>Only the 1st Mechanical Stiffness (P0-03)<br>and the Load Inertia Ratio (P0-02) can be<br>manually edited. | In this mode, only P0-03 stiffness<br>level and P0-02 load inertia ratio are<br>valid. Manual adjustment of other gain<br>parameters is invalid. |
| 2 | Fine Tuning | Fine tuning allows users to manually<br>configure all gain, filter and load<br>characteristics for tuning. A recommended<br>method is to first use auto-tuning to get<br>a close estimate of the required tuning<br>parameter values and then manually adjust<br>those estimates until a more accurate<br>tuning profile is reached. | In this mode, all gain parameters are<br>valid. |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-01 | LY | Load type | 0 | 0 ~ 10 | --- | P | S | T |
| Set the current load type. In auto-tuning mode and advanced tuning mode, setting the load type reasonably is conducive to accurate<br>identification and optimization of system gain parameters.<br>Set value Parameter setting mode Description<br>0 General load Ex: horizontally placed screw class load.<br>1 Rigid load Ex: rigid mechanism, such a horizontal installation like a turntable.<br>2 Flexible load Ex: Belt driven load |  |  |  |  |  |  |  |  |

| Set value | Parameter setting mode | Description |
|---|---|---|
| 0 | General load | Ex: horizontally placed screw class load. |
| 1 | Rigid load | Ex: rigid mechanism, such a horizontal installation like a turntable. |
| 2 | Flexible load | Ex: Belt driven load |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-02 | NR | Load inertia ratio | 0 | 0 ~ 100 | --- | P | S | T |
| The current load inertia ratio. Set the ratio of load inertia to motor rotor inertia.<br>When auto-tuning is in progress, the load inertia ratio of the current system can be identified in real time, and this parameter will be<br>automatically saved after auto-tuning is completed. When the load inertia ratio is set correctly, P0-05 can accurately represent the<br>current system gain. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-03 | KG | 1st Mechanical Stiffness Level | 5 | 1 ~ 20 | --- | P | S | T |
| The first stiffness value of the current system.<br>When the parameter tuning mode P0-00 is set to free tuning and automatic tuning, the higher the mechanical Stiffness level, the<br>stronger the gain of the servo system and the faster the response, An excessively large value will cause system vibrations. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-04 | KX | 2nd Mechanical Stiffness<br>Level | 5 | 1 ~ 20 | --- | P | S | T |
| The second stiffness value of the current system. When the gain switch is turned on, the second stiffness level will be effective under<br>the corresponding conditions. For details on gain switching, please refer to 7.1.6 Gain switching function.<br>When the parameter tuning mode P0-00 is set to free tuning and automatic tuning, the higher the mechanical Stiffness level, the<br>stronger the gain of the servo system and the faster the response, An excessively large value will cause system vibrations. |  |  |  |  |  |  |  |  |

<!-- page 126 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-05 | KP | 1st position loop gain | 52 | 0 ~ 20000 | 0.1Hz | P | S | T |
| Set the proportional gain for position control. 0 means not used, 20000 means the proportional effect is maximized. Increasing this<br>parameter can improve the responsiveness of the system, reduce the position error, and shorten the positioning time.<br>When the proportional gain of the position loop is too small, the system response will be delayed and position errors will decrease<br>slowly. If set too high, system vibrations may occur. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-07 | KD | 1st position loop derivative time<br>constant | 0 | 0 ~ 30000 | ms | P | S | T |
| ◆ Set the position loop differential time constant for position control.<br>◆ 0 means no derivative effect, the smaller the set value, the stronger the effect of the derivative term. When the set value of the<br>differential time constant (KD) is too large, the system's ability to suppress vibration is insufficient, and obvious oscillation will occur<br>during the acceleration / deceleration process, the uniform speed process and after the stop, Oscillations will decrease gradually and<br>eventually stabilize.<br>◆ When the set value of the differential time constant (KD) is reasonable, the system's ability to suppress vibration is significantly<br>enhanced, and it tends to stabilize quickly.<br>◆ When the differential time constant (KD) is set too small, the motion system will be too sensitive, easily vibrate and generate noise. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-08 | KE | 1st position loop derivative filter | 20000 | 0 ~ 40000 | 0.1Hz | P | S | T |
| Set the position loop differential low-pass filter for position control.<br>0 means no filtering effect. The differential control parameters filter frequency. This filter is a simple one-pole, low-pass filter intended<br>for attenuating high frequency oscillations. This value is a constant that must be calculated from the desired roll off frequency. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-09 | KL | Velocity feedforward gain | 10000 | -30000 ~ 3000 | 0.01% | P | S | T |
| Higher values will reduce system noise and eliminate overshoot, but will reduce the system's dynamic following performance. Lower<br>values will raise system stiffness, but may cause system noise. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-10 | KR | Velocity feedforward filter frequency | 20000 | 0 ~ 40000 | 0.1Hz | P | S | T |
| Sets the low-pass filtering for velocity feedforward.<br>0 means no filtering effect. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-11 | KF | 1st Command Velocity Gain | 10000 | -30000 ~ 3000 | 0.01% | P | S | T |
| The speed command from the position loop control process is multiplied by the ratio of this parameter to the system. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-12 | VP | 1st velocity loop gain | 183 | 0 ~ 30000 | 0.1Hz | P | S | T |
| Proportional gain term used to increase stiffness of motor response in direct proportion to the velocity error |  |  |  |  |  |  |  |  |

<!-- page 127 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-13 | VI | 1st speed loop integral time<br>constant | 189 | 0 ~ 30000 | ms | P | S | T |
| Set the integral time constant of the speed loop.<br>0 means no integral effect, the smaller the set value, the stronger the integral effect.<br>This integral gain term is used to increase stiffness and reduce steady-state velocity errors |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-14 | KK | Acceleration feedforward gain | 3000 | 0 ~ 10000 | 0.01% | P | S | T |
| Acceleration feedforward gain in servo control. A value of 0 means that the feedforward is not used, and a value of 10000 means that<br>the feedforward effect is maximized. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-15 | KT | Acceleration feedforward filter<br>frequency | 20000 | 0 ~ 40000 | 0.1Hz | P | S | T |
| Low-pass filter for acceleration feedforward gain in servo control. A value of 0 means that the filter is not used, and<br>40000 means that the filter effect is maximized. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-16 | KC | 1st command torque filter<br>frequency | 1099 | 0 ~ 40000 | 01Hz | P | S | T |
| Filter the command torque.<br>The filter is a single-output low-pass filter, which is used to low-pass filter the output of the PID controller (that is, the reference<br>current). System operation needs to be considered when setting this value.<br>The smaller the value, the lower the filtering frequency and the more obvious the filtering effect. The default value of 1099 works for<br>most applications. This value can be modified in cases of motor vibrations or abnormal audible noise.<br>An example use case is when a system is prone to mechanical resonance. The low pass filter cutoff frequency can be set below the<br>resonance frequency of the system to prevent the motor control loop from exciting the system into its resonance frequency. In large<br>inertia applications, increasing KP can help improve the system response but a KP value set too high can induce vibrations. To reduce<br>those vibrations, this filter's frequency may be reduced. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-17 | UP | 2nd position loop gain | 52 | 0 ~ 20000 | 0.1Hz | P | S | T |
| Set the proportional gain for position control. 0 means not used, 20000 means the proportional effect is maximized. Increasing this<br>parameter can improve the responsiveness of the system, reduce the position error, and shorten the positioning time.<br>When the proportional gain of the position loop is too small, the system response will be delayed and position errors will decrease<br>slowly. If set too high, system vibrations may occur. |  |  |  |  |  |  |  |  |

<!-- page 128 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-19 | UD | 2nd position loop differential time<br>constant | 0 | 0 ~ 30000 | ms | P | S | T |
| ◆ Set the position loop differential time constant for position control.<br>◆ 0 means no derivative effect, the smaller the set value, the stronger the effect of the derivative term. When the set value of the<br>differential time constant (UD) is too large, the system's ability to suppress vibration is insufficient, and obvious oscillation will occur<br>during the acceleration / deceleration process, the uniform speed process and after the stop, Oscillations will decrease gradually and<br>eventually stabilize.<br>◆ When the set value of the differential time constant (UD) is reasonable, the system's ability to suppress vibration is significantly<br>enhanced, and it tends to stabilize quickly.<br>◆ When the differential time constant (UD) is set too small, the motion system will be too sensitive, easily vibrate and generate noise. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-20 | UE | 2nd position loop derivative filter | 20000 | 0 ~ 40000 | 0.1Hz | P | S | T |
| Set the position loop differential low-pass filter for position control.<br>0 means no filtering effect. The differential control parameters filter frequency. This filter is a simple one-pole, low-pass filter intended<br>for attenuating high frequency oscillations. This value is a constant that must be calculated from the desired roll off frequency. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-21 | UF | 2nd velocity command gain | 10000 | -30000 ~ 3000 | 0.01% | P | S | T |
| The speed command from the position loop control process is multiplied by the ratio of this parameter to the system. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-22 | UP | 2nd velocity loop gain | 183 | 0 ~ 30000 | 0.1Hz | P | S | T |
| Set the proportional gain of the speed loop. In order to improve the overall response of the servo system, it is necessary to increase<br>the speed loop gain value. Setting the value too high will cause vibration. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-23 | UI | 1st velocity loop integral time<br>constant | 189 | 0 ~ 30000 | ms | P | S | T |
| Set the integral time constant of the speed loop.<br>0 means no integral effect, the smaller the set value, the stronger the integral effect.<br>This integral gain term is used to increase stiffness and reduce steady-state velocity errors |  |  |  |  |  |  |  |  |

<!-- page 129 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-24 | UC | 1st command torque filter frequency | 1099 | 0 ~ 40000 | 01Hz | P | S | T |
| Filter the command torque.<br>The filter is a single-output low-pass filter, which is used to low-pass filter the output of the PID controller (that is, the reference<br>current). System operation needs to be considered when setting this value.<br>The smaller the value, the lower the filtering frequency and the more obvious the filtering effect. The default value of 1099 works for<br>most applications. This value can be modified in cases of motor vibrations or abnormal audible noise.<br>An example use case is when a system is prone to mechanical resonance. The low pass filter cutoff frequency can be set below the<br>resonance frequency of the system to prevent the motor control loop from exciting the system into its resonance frequency. In large<br>inertia applications, increasing KP can help improve the system response but a KP value set too high can induce vibrations. To reduce<br>those vibrations, this filter's frequency may be reduced. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name |  | Defaults | Range | Unit |  | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|---|---|
| P0-33 | SD | Automatic gain switching method |  | 0 | 0 ~ 4 | —— |  | P | S | T |
| Set the parameter tuning method. |  |  |  |  |  |  |  |  |  |  |
| Set value | Switching mode |  | Switching condition |  |  |  | Switching latency |  |  |  |
| 0 | Fixed in Group One |  | Fixed in Group One |  |  |  | - |  |  |  |
| 1 | According to positional<br>error |  | Switch to Group 2: Absolute position error ≥ P0-34 setting |  |  |  | P0-37 |  |  |  |
|  |  |  | Switch back to Group 1: Absolute position error < P0-34 setting |  |  |  | P0-38 |  |  |  |
| 2 | According to the actual<br>speed of the motor |  | Switch to group 2 conditions: absolute value of actual speed 35 P0-35<br>set value |  |  |  | P0-37 |  |  |  |
|  |  |  | Switch back to Group 1: Absolute value of actual speed < P0-35 set<br>value |  |  |  | P0-38 |  |  |  |
| 3 | According to the actual<br>output torque of the<br>motor |  | Switch to Group 2: Absolute value of actual torque P P0-36 set value |  |  |  | P0-37 |  |  |  |
|  |  |  | Switch back to Group 1: Absolute value of actual torque < P0-36 set<br>value |  |  |  | P0-38 |  |  |  |
| 4 | Position arrival signal |  | Switch to Group 2 condition: Position arrival condition is valid. |  |  |  | P0-37 |  |  |  |
|  |  |  | Switch back to Group 1 condition: Position arrival condition not valid |  |  |  | P0-38 |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-34 | PN | Gain switching condition - position error | 0 | 0 ~ 2147483647 | Pulses | P | S | T |
| Set the judgment condition for position error-based gain switching.<br>When in position control, P0-33 Gain parameter switching method is set to " 1 ", this parameter is used to set the judgment condition<br>for switching. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-35 | VN | Gain switching condition - actual velocity | 0.000 | 0 ~ 100 | rps | P | S | T |
| Set the gain switching judgment condition based on the actual motor speed.<br>When in position control, velocity or torque control, P0-33 gain parameter switching method is set to " 2 ", this parameter is used to set<br>the judgment condition for switching. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-36 | TN | Gain switching condition - actual torque | 10 | 0 ~ 3000 | 0.1% | P | S | T |
| Set the gain switching judgment condition based on the actual output torque of the motor.<br>When in position control, velocity or torque control, P0-33 gain parameter switching method is set to " 3 ", this parameter is<br>used to set the judgment condition for switching. |  |  |  |  |  |  |  |  |

<!-- page 130 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-37 | SE1 | Delay time - 2nd Group Gains to<br>1st Group Gains | 10 | 0 ~ 10000 | ms | P | S | T |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-38 | SE2 | Delay time - 1st Group Gains to<br>2nd Group Gains | 10 | 0 ~ 10000 | ms | P | S | T |
| When switching gains is required in an application, users will need to configure two delay periods. P0-37 and P0-38 define the delay<br>times for switching to and from Group 1 and Group 2 of gains.<br>P0-33<br>Set corresponding<br>Group 1 Gain conditions are valid First gain switching to Group 2 Gain<br>P0-05 the second gain delay time P0-17<br>P0-07 P0-38 P0-19<br>P0-08 P0-20<br>P0-11 P0-33 P0-21<br>P0-12 Set corresponding P0-22<br>P0-13 Second gain switching to conditions are not valid P0-23<br>the first gain delay time<br>P0-16 P0-37 P0-24<br>The switching transition time is shown below<br>◆ When gain switching conditions are valid, Group 1 will gradually transition to Group 2 based on the time defined by P0-37.<br>◆ When gain switching conditions are not valid or transition to invalid, Group 2 will gradually transition back to Group 1 based on the<br>time defined by P0-38.<br>Established<br>Switching Condition<br>Not Established<br>2nd gain<br>High gain<br>Gain<br>1st gain 1st gain<br>Low gain Low gain<br>1st gain benefit switch to the second Second gain switching to<br>P0-38 first gain delay time<br>PO-37 |  |  |  |  |  |  |  |  |

| 2nd gain |
|---|

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-39 | LR | Velocity Feedback Filter | 0 | 0 ~ 3 | —— | P | S | T |
| PID controller, speed loop speed feedback low-pass filter.<br>Set value Filtering frequency<br>0 Not in use<br>1 8KHz<br>2 2KHz<br>3 1KHz |  |  |  |  |  |  |  |  |

| Set value | Filtering frequency |
|---|---|
| 0 | Not in use |
| 1 | 8KHz |
| 2 | 2KHz |
| 3 | 1KHz |

<!-- page 131 -->

#### 8.3.2 Group P1-XX: Configuration --- Configuration class parameters

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-00 | CM | Control mode | 21 | 1,15,21 | - | P | S | T |
| Parameter P1-00 can be used to set the main control mode of the drive.<br>Set value Mode Control signal Description<br>Communication Use communication commands to control motor<br>1 Torque Control<br>command output torque<br>Internal 8 speed mode with parameters P2-10~P2-<br>15 Velocity Control Digital input signal 17setting speed values for each of the 1-8 speed<br>settings, respectively.<br>Communication Point-to-point position mode control using<br>21 Position Control<br>command communication instructions |  |  |  |  |  |  |  |  |

| Set value | Mode | Control signal | Description |
|---|---|---|---|
| 1 | Torque Control | Communication<br>command | Use communication commands to control motor<br>output torque |
| 15 | Velocity Control | Digital input signal | Internal 8 speed mode with parameters P2-10~P2-<br>17setting speed values for each of the 1-8 speed<br>settings, respectively. |
| 21 | Position Control | Communication<br>command | Point-to-point position mode control using<br>communication instructions |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-02 | PM | Control Mode on power up | 10 | 8 ~ 10 | - | P | S | T |
| Parameter P1-02 can be used to set the communication mode and working status of the drive after power-on.<br>Set value Mode<br>8 Power-on running in Modbus/RTU mode, servo automatically enabled upon power-on<br>Running in Q mode that supports Modbus/RTU communication, the servo automatically<br>9<br>enables and automatically executes the Q program when powered on.<br>Power-on running in Q mode that supports Modbus/RTU communication, servo is not<br>10<br>enabled automatically after power-on, Q program is not executed automatically |  |  |  |  |  |  |  |  |

| Set value | Mode |
|---|---|
| 8 | Power-on running in Modbus/RTU mode, servo automatically enabled upon power-on |
| 9 | Running in Q mode that supports Modbus/RTU communication, the servo automatically<br>enables and automatically executes the Q program when powered on. |
| 10 | Power-on running in Q mode that supports Modbus/RTU communication, servo is not<br>enabled automatically after power-on, Q program is not executed automatically |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-03 | JM | Jog Mode | 2 | 1 ~ 2 | - | P | S | T |
| When jogging the motor, users can configure the drive for pure velocity control or for velocity and position control.<br>Set value Mode Mode<br>Position and velocity tracking are monitored for errors. If the motor incurs<br>1 Position over time a greater position error than defined by P3-04, the drive will fault out with a<br>position error fault.<br>2 Velocity control only Only velocity tracking is monitored for errors. |  |  |  |  |  |  |  |  |

| Set value | Mode | Mode |
|---|---|---|
| 1 | Position over time | Position and velocity tracking are monitored for errors. If the motor incurs<br>a greater position error than defined by P3-04, the drive will fault out with a<br>position error fault. |
| 2 | Velocity control only | Only velocity tracking is monitored for errors. |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-05 | GC | Current command for Torque Control | 0 | -3000 ~ 3000 | 0.1% | P | S | T |
| When the drive is operated in torque control (command based), this parameter defines the commanded current limit. The current draw<br>dictates the output torque of the motor so therefore, this limit defines the maximum output torque allowable from the motor. The motor<br>will stop motion when it reaches this current value and will try to maintain position.<br>Note: This current value is not maintained after power cycles. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-06 | CC | 1st Torque Limit | 3000 | 0 ~ 3000 | 0.1% | P | S | T |
| Sets the maximum continuous current level for the servo motor when operating in Point-to-Point mode and Velocity Control Mode. It<br>sets the upper limit on the continuous current range the motor may draw. However, users must be aware that the motor can still draw<br>more than the set continuous current, therefore output higher torque, but they will be operating in what is known as the Peak Current<br>which is only allowed for a limited amount of time.<br>Behaves as the first torque limit when operating in Torque Control. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-07 | CV | Target Torque | 0 | 0 ~ 3000 | 0.1% | P | S | T |
| Torque arrival signal judgment value. Please refer to chapter 7.4.9 Torque arrives |  |  |  |  |  |  |  |  |

<!-- page 132 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-08 | HC | Hard Stop Home Current Limit | 200 | 0 ~ 3000 | 0.1% | P | S | T |
| Sets the hard stop homing current limit. While hard stop homing, once the motor draws this current level, the motor drive determines<br>that the mechanical limit of the system has been reached. Hard stop homing does not require homing switches or limit switches to<br>home. It functions entirely off of the HC value, a mechanical hard stop and the home offset value (HO). Refer to section 7.８ for more<br>information. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-09 | CL | Current Foldback Timer | 0 | 0 ~ 30000 | ms | P | S | T |
| Servo motors have the ability to provide peak torque (overload torque), above that set by the continuous current value (P1-06). This<br>parameter sets the duration of torque overload.<br>● When the overload time exceeds this set value, " r16CL " overload alarm will be generated.<br>● If the set value is too large, long-term overload will easily cause the motor to overheat and damage.<br>● During torque control, this function is invalid.<br>● When the set value is " 0 ", the " r16CL " overload alarm will not be generated, and the servo motor provides an overload output<br>capability of 2 seconds.<br># The torque output is limited to the motor rated torque<br>Torque Constant torque<br># If the reference torque output is still greater than Motor rated<br>torque setting value, actual limit value of P1-06 will always<br>be limited to the motor<br>Limit Value of P1-06 Rated torque<br># Only when the reference torque is less than the motor rating<br>rated torque of the motor is constant torque, after a certain time,<br>to be overloaded again<br>Motor Rated Torque<br>Time<br>Output<br>Signal output during No Output No Output<br>torque limit<br>T-LMT<br>Output<br>r16CL Overload Alarm No Output<br>The time has not yet Fixed value, no alarm<br>reached the time set setting is generated,<br>by P1-09 and the time r16CL is generated<br>set by P1-09 has exceeded Afterenerated,<br>the time set by P1-09. r16CL is generated |  |  |  |  |  |  |  |  |

| Output |
|---|

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-10 | LD | Torque limit method | 1 | 1 ~ 5 | ms | P | S | T |
| Parameter P1-10 defines 5 torque limit modes, each of which is as follows.<br>Set value Forward direction Reverse direction<br>0 Register Y Register Z<br>1 Parameter P1-06<br>2 Parameter P1-06 Parameter P1-25<br>TQ-LMT input is valid: P1-06<br>3<br>TQ-LMT input is valid: P1-25<br>If TQ-LMT input is valid：P1-06 If TQ-LMT input is valid：P1-25<br>5<br>If TQ-LMT input is invalid: P1-26 If TQ-LMT input is invalid：P1-27<br>Please refer to chapter 7.5 Torque limit |  |  |  |  |  |  |  |  |

| Set value | Forward direction | Reverse direction |  |
|---|---|---|---|
| 0 | Register Y |  | Register Z |
| 1 | Parameter P1-06 |  |  |
| 2 | Parameter P1-06 | Parameter P1-25 |  |
| 3 | TQ-LMT input is valid: P1-06 |  |  |
|  | TQ-LMT input is valid: P1-25 |  |  |
| 5 | If TQ-LMT input is valid：P1-06 | If TQ-LMT input is valid：P1-25 |  |
|  | If TQ-LMT input is invalid: P1-26 | If TQ-LMT input is invalid：P1-27 |  |

<!-- page 133 -->

| Parameter | Instruction | Name | Defaults | Range | Unit |
|---|---|---|---|---|---|
| P1-11 | DR | Rotation Direction | 0 | 0，1 | - |
| Set the relationship between the direction of the command and the rotation direction of the motor:<br>Set value Direction of rotation Description<br>When the commanded direction of motion is<br>positive, the direction of rotation of the motor<br>0 Clockwise rotation shaft is clockwise when viewed from the front<br>when direction positive<br>end of the motor.<br>When the commanded direction of motion is<br>Counterclockwise rotation positive, the direction of rotation of the motor<br>1 when direction positive<br>shaft is counterclockwise when viewed from<br>the front end of the motor. |  |  |  |  |  |

| Set value | Direction of rotation | Description |
|---|---|---|
| 0 | Clockwise rotation<br>when direction positive | When the commanded direction of motion is<br>positive, the direction of rotation of the motor<br>shaft is clockwise when viewed from the front<br>end of the motor. |
| 1 | Counterclockwise rotation<br>when direction positive | When the commanded direction of motion is<br>positive, the direction of rotation of the motor<br>shaft is counterclockwise when viewed from<br>the front end of the motor. |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-12 | IF | Immediate Format | H | D,H | - | P | S | T |
| This parameter sets the format of data communicated via the SCL protocol.<br>Set value Mode<br>D Decimal system<br>H Hexadecimal system<br>Ex: If reading the encoder position, using EP command, and the current position value is 20000 counts but we switch P1-12 to setting<br>"H", the return value of EP would be 4E20 (hex) |  |  |  |  |  |  |  |  |

| Set value | Mode |
|---|---|
| D | Decimal system |
| H | Hexadecimal system |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-13 | PR | Protocol | 5 | 1 ~ 511 | - | P | S | T |
| This parameter configures serial communication using a binary setting. The following describes the effect of each bit:<br>Bit 0 = by default set to 1, prescribes standard SCL communication<br>Bit 1 = Always use address character<br>Bit 2 = use Ack/Nack<br>Bit 3 = use Checksum<br>Bit 4 = (set if communication is RS-485)<br>Bit 5 = 3-digit numeric register addressing<br>Bit 6 = Checksum Type<br>Bit 7 = Little/Big Endian Format (Modbus)<br>Bit 8 = 2-wire or 4-wire (RS-485) |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-14 | TD | Transmit Delay | 2 | 0 ~ 20 | ms | P | S | T |
| Sets or requests the time delay used by the drive when responding to a command that requests a response.<br>Typically this is needed when using the 2-wire RS-485 interface (Half-duplex). Because the same wires are used<br>for both receive and transmit a time delay is usually needed to allow transition time. The Host device’s RS-485<br>specification must be understood to determine the time delay needed. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-15 | BR | RS-485 communication baud rate | 1 | 1 ~ 5 | - | P | S | T |
| The baud rate that takes effect after power-on in serial communication. This value will be saved immediately after being configured<br>but will not take effect immediately until the next power-on, so the upper computer software can configure this value at any time.<br>Set value Speed<br>1 9600bps<br>2 19200bps<br>3 38400bps<br>4 57600bps<br>5 115200bps |  |  |  |  |  |  |  |  |

| Set value | Speed |
|---|---|
| 1 | 9600bps |
| 2 | 19200bps |
| 3 | 38400bps |
| 4 | 57600bps |
| 5 | 115200bps |

<!-- page 134 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-16 | DA | RS-485 communication address | 32 | 1 ~ 32 | - | P | S | T |
| In Modbus/RTU communication mode, set the node address of the drive |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-17 | CO | CAN open communication node<br>address | 1 | 1 ~ 127 | - | P | S | T |
| In CAN open communication mode, set the drive node address |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-18 | CB | CAN open communication baud rate | 0 | 0 ~ 7 | - | P | S | T |
| When the DIP switch for setting the CANopen communication baud rate on the drive is set to OFF, this parameter can be configured<br>via the Luna Software to set the CANopen communication baud rate. After this value is configured, it will be saved immediately but will<br>not take effect until the next power-on which means the host computer software can configure this value at any time.<br>Set value Speed<br>0 1 Mbps<br>1 800 kbps<br>2 500 kbps<br>3 250 kbps<br>4 125 kbps<br>5 50 kbps<br>6 20 kbps<br>7 12.5 kbps |  |  |  |  |  |  |  |  |

| Set value | Speed |
|---|---|
| 0 | 1 Mbps |
| 1 | 800 kbps |
| 2 | 500 kbps |
| 3 | 250 kbps |
| 4 | 125 kbps |
| 5 | 50 kbps |
| 6 | 20 kbps |
| 7 | 12.5 kbps |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-19 | ZR | Regenerative resistor resistance | 200 | 10 ~ 32000 | Ω | P | S | T |
| Set the resistance value of the regenerative energy absorption resistor, and the driver calculates the discharge power on the<br>regeneration resistor according to the current discharge voltage and resistance value. The default value is the resistance value of the<br>regenerative energy absorption resistor built into the driver. When using an external regenerative energy absorption resistor value, the<br>correct resistance value must be set. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-20 | ZC | Regenerative resistor power rating | 40 | 1 ~ 32000 | W | P | S | T |
| Sets or requests the regeneration resistor wattage value. MBDV drives dynamically calculate the continuous wattage induced into<br>an external regeneration resistor and must know the continuous wattage rating of the regen resistor to do this effectively and avoid<br>damaging the resistor. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-21 | ZT | Regenerative resistor time constant | 1000 | 0 ~ 8000 | ms | P | S | T |
| Sets or requests the regeneration resistor time constant. Decides the peak time that the resistor can tolerate full regeneration voltage. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-24 | MA | Alarm mask | 4294967295 | 0 ~ 4294967295 | - | P | S | T |
| When the driver generates some non-serious warning information, the corresponding bit of this parameter can shield the LED alarm<br>display function of the corresponding warning information. When the shielded warning information is generated, it will no longer flash<br>on the two 7 - segment digital tubes. |  |  |  |  |  |  |  |  |

<!-- page 135 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-25 | CX | Second torque limit | 3000 | 0 ~ 3000 | 0.1% | P | S | T |
| Sets the second limit value of the motor output torque. Please refer to chapter 7.5 Torque limit |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-26 | CY | Third torque limit | 3000 | 0 ~ 3000 | 0.1% | P | S | T |
| Set the third limit value of motor output torque. Please refer to chapter 7.5 Torque limit |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-27 | CY | Fourth torque limit | 3000 | 0 ~ 3000 | 0.1% | P | S | T |
| Set the fourth limit value of motor output torque. Please refer to chapter 7.5 Torque limit |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-28 | HT | Motor stall protection time | 0 | 0 ~ 30000 | ms | P | S | T |
| In position mode or position-based speed mode, stalling causes the drive to always output the rated torque of the motor. Stalling for a<br>long time can cause the motor to overheat. This parameter sets the protection time of the motor stall, when the actual output current<br>of the motor is equal to the rated current of the motor, and the time exceeds the setting of this parameter. r37ST (motor stall alarm)<br>will be generated and the motor will be disabled. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-29 | YV | The action of the dynamic brake<br>when it is disabled | 0 | 0 ~ 5 | --- | P | S | T |
| Servo is OFF, the dynamic braking action is set by parameter P1-29, and the longest action time during deceleration is set by<br>parameter P1-31, please refer to the table below. The deceleration process means that when the dynamic braking takes effect, the<br>actual speed of the motor decelerates from the speed when it takes effect to within the zero-speed threshold of parameter P5-42, or<br>the deceleration time reaches the set time of P1-31.<br>Description<br>Value<br>Deceleration process After stopping<br>0 Slow down with parameter P2-01 Remain free-moving state<br>1 Slow down with parameter P2-01 Dynamic braking action<br>2 Free-moving state Remain free-moving state<br>3 Free-moving state Dynamic braking action<br>4 Dynamic braking action Remain free-moving state<br>5 Dynamic braking action Dynamic braking action |  |  |  |  |  |  |  |  |

| Value | Description |  |
|---|---|---|
|  | Deceleration process | After stopping |
| 0 | Slow down with parameter P2-01 | Remain free-moving state |
| 1 | Slow down with parameter P2-01 | Dynamic braking action |
| 2 | Free-moving state | Remain free-moving state |
| 3 | Free-moving state | Dynamic braking action |
| 4 | Dynamic braking action | Remain free-moving state |
| 5 | Dynamic braking action | Dynamic braking action |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-30 | YR | The action of the dynamic brake<br>when an error is reported | 0 | 0 ~ 3 |  | P | S | T |
| When the servo reports an error, the dynamic braking action is set by parameter P1-30, and the longest action time during<br>deceleration is set by P1-32, please refer to the table below. The deceleration process means that when the dynamic braking takes<br>effect, the actual speed of the motor decelerates from the speed when it takes effect to within the zero-speed threshold of parameter<br>P5-42, or the deceleration time reaches the set time of P1-31.<br>Description<br>Value<br>Deceleration process After stopping<br>0 Free-moving state Remain free-moving state<br>1 Free-moving state Dynamic braking action<br>2 Dynamic braking action Remain free-moving state<br>3 Dynamic braking action Dynamic braking action |  |  |  |  |  |  |  |  |

| Value | Description |  |
|---|---|---|
|  | Deceleration process | After stopping |
| 0 | Free-moving state | Remain free-moving state |
| 1 | Free-moving state | Dynamic braking action |
| 2 | Dynamic braking action | Remain free-moving state |
| 3 | Dynamic braking action | Dynamic braking action |

<!-- page 136 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |  |
|---|---|---|---|---|---|---|---|---|---|
| P1-31 | YM | Dynamic brake The longest<br>movement during a deenabled<br>deceleration Work time | 500 | 0 ~ 30000 | ms | P | S | T | F |
| This parameter sets the longest action time of dynamic braking during deceleration when the servo is OFF.<br>The deceleration process means that when the dynamic braking takes effect, the actual speed of the motor decelerates from the<br>speed when it takes effect to within the zero-speed threshold of parameter P5-42, or the deceleration time reaches the setting of P1-<br>31.time.<br>● When the deceleration time exceeds the setting of P1-31, even if the actual speed of the motor is still greater than that of P5-42,<br>the dynamic brake will no longer work.<br>The figure below is when P1-29 = 4 ms.<br>Motor Speed<br>P5-42<br>Time<br>Enable<br>Motor Enable State<br>Disable<br>Activation of Dynamic Braking Activation<br>P1-29 = 4 actions when Inactivated<br>Reach P5-42 setting<br>The time of value is less than P1-31<br>P1-31 |  |  |  |  |  |  |  |  |  |

| on |
|---|

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |  |
|---|---|---|---|---|---|---|---|---|---|
| P1-32 | YN | The longest action time of the brake<br>during an errored deceleration | 0 | 0 ~ 30000 | ms | P | S | T | F |
| This parameter sets the longest action time of the dynamic brake during the deceleration process after the servo reports an error.<br>The deceleration process means that the actual speed of the motor decelerates from the effective speed to within the zero-speed<br>threshold of parameter P5-42 when the dynamic brake is effective, or the deceleration time reaches the setting of P1-32.time.<br>● When the deceleration time exceeds with the setting of P1-32, even if the actual speed of the motor is still greater than that of P5-<br>42, the dynamic brake will no longer work.<br>The figure below is when P1-30 = 2 ms.<br>Motor Speed<br>P5-42<br>Time<br>Enable<br>Motor Enable State<br>Disable<br>Activation of Dynamic Braking Activation<br>P1-30 = 2 actions when Inactivated<br>Reach P5-42 setting<br>The time of value is less than P1-31<br>P1-32 |  |  |  |  |  |  |  |  |  |

| n |
|---|

<!-- page 137 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-34 | RT | Current Ramp Limit | 1000 | 00 ~ 3000 | 0.1% | P | S | T |
| The first speed value in multi-speed mode for multi-speed control, please refer to the chapter 7.2.13 multi-stage speed control mode |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-37 | DV | Dynamic brake velocity | 50 | 0 ~50 | rps | P | S | T |
| The maximum speed at which the dynamic brake can be activated. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-38 | DW | Dumping Voltage Setting | 650 | 200 ~ 850 | 0.1V | P | S | T |
| Sets or request dumping voltage level for MBDV drives. When voltage is above set level, regen resistor will be triggered |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-39 | ZS | Watchdog trigger time | 500 | 0 ~ 10000 | ms | P | S | T |
| Sets or requests the amount of time before a communication loss is detected. The internal timer is reset when a new network<br>application message is received and then proceeds to counts down. When the internal timer reaches zero, the action set by<br>ZA is performed. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P1-40 | ZA | Action after the watchdog is triggered | 1 | 1 ~ 16 | —— | P | S | T |
| Sets or requests the action taken when communication loss has been detected. |  |  |  |  |  |  |  |  |

<!-- page 138 -->

#### 8.3.3 P2-XX: Trajectory--- trajectory planning

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |  |
|---|---|---|---|---|---|---|---|---|---|
| P2-00 | VM | Maximum Velocity | 80 | 0 ~ 100 | rps | P | S | T | F |
| Set the maximum running speed of the motor.<br>When the actual speed of the motor exceeds the set value of P2-00, r12OV (motor stall alarm) will be generated. |  |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |  |
|---|---|---|---|---|---|---|---|---|---|
| P2-01 | AM | Maximum acceleration/deceleration | 3000 | 0.167 ~ 5000 | rps/s | P | S | T | F |
| Sets or requests the maximum acceleration/deceleration allowed. When the set acceleration / deceleration is greater than the set maximum<br>value, the actual running acceleration / deceleration will be limited to the maximum value.<br>At the same time, this value is also the maximum braking deceleration value after the emergency stop command or hitting the limit switch. |  |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P2-02 | JS | Jog velocity | 10 | -100 ~ 100 | rps | P | S | T |
| Sets or requests the speed for Jog moves in rev/sec. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P2-03 | JA | Jog acceleration | 100 | 0.167 ~ 5000 | rps/s | P | S | T |
| Sets or requests the accel/decel rate for Jog moves in rev/sec/sec. Sending JA with no parameter causes drive to respond with<br>present jog accel/decel rate. Setting JA overwrites the both the last JA and JL values. This means that to have different jog accel and<br>jog decel values, you should first send JA to set the jog accel and then send JL to set the jog decel. The JA value cannot be changed<br>while jogging. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P2-04 | JL | Jog deceleration | 100 | 0.167 ~ 5000 | rps/s | P | S | T |
| Sets or requests the decel rate for Jog moves and velocity (oscillator) modes in rev/sec/sec. The JL value cannot be changed while<br>jogging. JA sets both the JA and JL values, so when a different JL value is required set JA first, then set JL |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |  |
|---|---|---|---|---|---|---|---|---|---|
| P2-05 | JT | Jerk time | 10 | 0 ~ 125 | ms | P | S | T | F |
| Parameter P2-05 jerk time takes effect in internal trajectory mode (position, speed, torque), analog position, analog speed, analog<br>torque, or communication command control.<br>The effect of jerk smoothing on the input command is shown in the figure below.<br>Speed ◆ The acceleration filter will cause a certain<br>Target Speed Curves delay to the command T, but will not affect the<br>Filtered speed Curves final positioning accuracy<br>◆ The larger the time constant of P2-05 is, the<br>more obvious the smoothing effect is, and the<br>Time command response delay will also increase.<br>T T T T=P2-05<br>A reasonable filter time constant should be set<br>according to the actual application. |  |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |  |
|---|---|---|---|---|---|---|---|---|---|
| P2-06 | VE | Target Velocity (point-to-point) | 10 | 0.0042 ~ 100 | rps | P | S | T | F |
| The target speed command in point-to-point command position mode. |  |  |  |  |  |  |  |  |  |

<!-- page 139 -->

| Parameter | Instruction | Name Defau | lts Range | Unit | Related<br>Patterns |  |  |  |
|---|---|---|---|---|---|---|---|---|
| P2-07 | AC | Target acceleration (point-to-point) 10 | 0 0.167 ~ 5000 | rps/s | P | S | T | F |
| Acceleration value in point-to-point command position mode. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name Defa | ults Range | Unit | Related<br>Patterns |  |  |  |
|---|---|---|---|---|---|---|---|---|
| P2-08 | DE | Target deceleration (point-to-point) 10 | 0 0.167 ~ 5000 | rps/s | P | S | T | F |
| Deceleration value in point-to-point command position mode. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name Defa | ults Range | Unit | Related<br>Patterns |  |  |  |
|---|---|---|---|---|---|---|---|---|
| P2-09 | VC | Velocity change (point-to-point) 2 | 0 ~ 100 | rps | P | S | T | F |
| The internal position mode has point-to-point positioning control with variable speed. This parameter is used to set the speed value of<br>the second stage. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name Defa | ults Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|
| P2-18 | HA1 | 1st Homing acceleration/deceleration 10 | 0 0.167 ~ 5000 | rps/s | P | S | T |
| Sets or requests the acceleration/deceleration rate used in the homing function. |  |  |  |  |  |  |  |

| Parameter | Instruction | Name Defa | ults Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|
| P2-24 | HV1 | Homing velocity 1 1 | 0 0.0042 ~ 100 | rps | P | S | T |
| Sets the velocity used in the first state of a homing operation. |  |  |  |  |  |  |  |

| Parameter | Instruction | Name Defa | ults Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|
| P2-25 | HV2 | Homing velocity 2 1 | 0.0042 ~ 100 | rps | P | S | T |
| Sets the velocity used in the second stage of a homing operation. |  |  |  |  |  |  |  |

| Parameter | Instruction | Name Defa | ults Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|
| P2-27 | HO | Homing offset 0 | -2147483647 ~<br>+2147483647 | pulses | P | S | T |
| Sets the offset value during a homing operation. As an example, this value dictates the distance that the motor will back away from a<br>hard stop during a hard stop home operation.<br>Left Limit Origin Right Limit<br>Find travel switch, parameter<br>P2-18(HA1),P2-24(HV1)<br>Find origin switch, parameter<br>P2-18(HA1),P2-24(HV1)<br>After passing the origin, return to<br>the origin switch, parameter<br>P2-18(HA1),P2-25(HV2)<br>P2-18, P2-24, P2-25, P2-27 parameters are the configuration parameters of the built-in homing function of the drive. For detailed<br>functions of homing, please refer to 7.10 _ return to origin can |  |  |  |  |  |  |  |

<!-- page 140 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P2-28 | KJ | Jerk filter | 0 | 0 ~ 1000 | ms | P | S | T |
| Parameter P2-28 low-pass smoothing filter can take effect in the control mode used, such as: internal trajectory mode (position,<br>speed, torque), analog position, analog speed, analog torque, communication command control, etc.<br>The smoothing effect of the low-pass filter on the input command is shown in the figure below.<br>Motor Speed Target Speed Instruction<br>VC Filtered Speed<br>◆ Low-pass smoothing filter will produce a certain delay T for<br>VC×0.632 the instruction, but it will not affect the final positioning accuracy.<br>VC×0.368 ◆ The larger the time constant of P2-28, the more obvious the<br>smoothing effect, and the longer the instruction response delay.<br>Therefore, we should set the reasonable filter time constant<br>according to the practical application.<br>P2-28 P2-28 Time |  |  |  |  |  |  |  |  |

| Motor Speed Target Speed Instruction<br>VC Filtered Speed<br>VC×0.632<br>VC×0.368<br>P2-28 P2-28 Time | ◆ Low-pass smoothing filter will produce a certain delay T for<br>the instruction, but it will not affect the final positioning accuracy.<br>◆ The larger the time constant of P2-28, the more obvious the<br>smoothing effect, and the longer the instruction response delay.<br>Therefore, we should set the reasonable filter time constant<br>according to the practical application. |
|---|---|

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P2-30 | VT | Speed limit in torque mode | 80 | 0 ~ 100 | rps | P | S | T |
| In torque control mode, this parameter can be used as the direct speed limit. No input is required to enable this speed limit. It will be<br>enabled as soon as torque control mode is enabled at the drive. |  |  |  |  |  |  |  |  |

#### 8.3.4 P3-XX group: Encoder & Step/Dir--- Encoder and input pulse settings

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P3-04 | PF | Position error limit | 100000 | 0 ~ 2147483647 | pulses | P | S | T |
| The threshold for position error overrun error.<br>During the movement, when the deviation between the target position and the actual position fed back by the encoder exceeds the<br>threshold, an error over-limit error will occur. The drive LED display panel will display an error code |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P3-05 | EG | Electronic Gearing | 10000 | 200 ~ 131072 | pulses/rev | P | S | T |
| Set the number of pulses per revolution of the motor.<br>When parameter P3-16 = 1, this parameter setting is invalid and has no effect on the resolution of the motor. The number of pulses<br>required for each revolution of the motor will be set by the parameter P3-00 electronic gear ratio numerator and P3-01 Motor gear<br>ratio denominator. |  |  |  |  |  |  |  |  |

<!-- page 141 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P3-12 | PO | Encoder feedback output mode | 1 | 0 ~ 256 | - | P | S | T |
| Sets the output pulse source, output pulse phase, and Z pulse output polarity type. The functions corresponding to each bit are as<br>follows.<br>Parameter P3-03 input pulse setting<br>bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0<br>Z pulse output phase relationship between<br>Output pulse source<br>polarity A and B at forward rotation<br>0 0 0 0 0: positive polarity 0：A leads B 90°<br>bit1=0, bit0=1: motor encoder<br>1: Negative polarity 1：B leads A 90°<br>get rid of text and remove this box on the chart<br>◆ Output source setting<br>The pulse frequency division output function supports the output of the following three signal sources through the frequency division<br>function<br>Set value Description<br>bit1=0,bit0=1 Motor encoder<br>Note: When the signal source is external pulse command, parameter P3-13 and parameter P3-14 are invalid, and the command<br>pulse does not do any processing and is directly output by-pass. P3-12 's bit2 bit<br>And the setting of bit3 will also be invalid.<br>◆ A/B phase relationship in forward rotation<br>When setting forward rotation, the external second encoder<br>Set value Description<br>0 A leads B 90°<br>1 B leads A 90°<br>◆ Set the polarity of the Z-phase pulse<br>Set the polarity of the Z-phase pulse<br>Set value Description<br>0 Rising edge<br>1 Descending edge<br>For detailed pulse output frequency division settings, please refer to the chapter 7.6 Encoder frequency division output function |  |  |  |  |  |  |  |  |

| Parameter P3-03 input pulse setting |  |  |  |  |  |  |  |
|---|---|---|---|---|---|---|---|
| bit7 | bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0 |
| 0 | 0 | 0 | 0 | Z pulse output<br>polarity | phase relationship between<br>A and B at forward rotation | Output pulse source |  |
|  |  |  |  | 0: positive polarity | 0：A leads B 90° | bit1=0, bit0=1: motor encoder |  |
|  |  |  |  | 1: Negative polarity | 1：B leads A 90° |  |  |
| get rid of text and remove this box on the chart |  |  |  |  |  |  |  |

| Set value | Description |
|---|---|
| bit1=0,bit0=1 | Motor encoder |

| Set value | Description |
|---|---|
| 0 | A leads B 90° |
| 1 | B leads A 90° |

| Set value | Description |
|---|---|
| 0 | Rising edge |
| 1 | Descending edge |

<!-- page 142 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P3-13 | ON | Pulse output gear ratio - numerator | 10000 | 0 ~ 13107200 | - | P | S | T |
| Set the encoder frequency division output numerator |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P3-14 | OD | Pulse output gear ratio - denominator | 131072 | 0 ~ 13107200 | - | P | S | T |
| Set the denominator of the encoder frequency division output.<br>When the source of the output pulse is the motor encoder or the second encoder, by setting the numerator and denominator of the<br>frequency division output gear ratio of the encoder, you can set the number of pulses output.<br>P3-13 Pulse output gear ratio - numerator<br>Number of output pulses per revolution = x 65536<br>P3-14 Pulse output gear ratio - denominator<br>Note:<br>● The numerator of the frequency division ratio of P3-13 must be less than the denominator of the frequency division ratio of P3-14<br>● When P3-13 > P3-14, the number of pulses output by the motor per revolution (after the A/B phase is multiplied by 4) = P3-13<br>For detailed pulse output frequency division settings, please refer to the chapter 7.6 Encoder frequency division output function |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P3-15 | ES | Absolute encoder setting | 1 | 0 ~ 3 | - | P | S | T |
| Set the use mode of the motor absolute encoder:<br>Set value liiustrate Note<br>Used as an incremental encoder, no multi-laps of lost alarm<br>0 Incremental encoder<br>will be generated if the battery is not plugged in.<br>1 Single-turn absolute encoder Absolute position within one revolution of the feedback motor |  |  |  |  |  |  |  |  |

| Set value | liiustrate | Note |
|---|---|---|
| 0 | Incremental encoder | Used as an incremental encoder, no multi-laps of lost alarm<br>will be generated if the battery is not plugged in. |
| 1 | Single-turn absolute encoder | Absolute position within one revolution of the feedback motor |

<!-- page 143 -->

#### 8.3.5 Group P5-XX: IO settings

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-00 | MU1 | Digital input 1 function |  | 0 ~ 46 |  | P | S | T |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-01 | MU2 | Digital input 2 function |  | 0 ~ 46 |  | P | S | T |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-02 | MU3 | Digital input 3 function |  | 0 ~ 46 |  | P | S | T |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-03 | MU4 | Digital input 4 function |  | 0 ~ 46 |  | P | S | T |

Parameters P5-00~P5-03 sequentially set the function of digital input X1~X4. The following functions and logic states
can be assigned to the inputs.

| Signal name | Shorthand notation | Set value and effective logic |  |
|---|---|---|---|
|  |  | Valid when closed | Valid when Open |
| Universal input | GPIN | 0 | - |
| Servo enable | S-ON | 1 | 2 |
| Alarm clear | A-CLR | 3 | 4 |
| Forward rotation prohibition<br>limit | CW-LMT | 5 | 6 |
| Reverse prohibition limit | CCW-LMT | 7 | 8 |
| control mode switch | CM-SEL | 9 | 10 |
| Gain switching | GAIN-SEL | 11 | 12 |
| emergency stop | E-STOP | 13 | 14 |
| Return-to-origin start | S-HOM | 15 | 16 |
| Torque limit input | TQ-LMT | 19 | 20 |
| Zero speed clamp input | ZCLAMP | 21 | 22 |
| Multi-speed start | SP-STA | 33 | 34 |
| speed command direction | SPD-DIR | 35 | 36 |
| Speed limit input | V-LMT | 37 | 38 |
| Origin switch signal input | HOM-SW | 39 | 40 |
| Execute the Q program | START-Q | 45 | 46 |

<!-- page 144 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-12 | MO1 | Digital output 1 function |  | 0 ~ 34 |  | P | S | T |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-13 | MO2 | Digital output 2 function |  | 0 ~ 34 |  | P | S | T |

Parameters P5-12~P5-13 sequentially set the digital output port Y1~Y2 function.
The following functions and logic states can be assigned to the inputs.

| Signal name | Shorthand notation | Logic and set value when output signal is<br>valid |  |
|---|---|---|---|
|  |  | Output when the<br>signal is valid<br>Closed | Output when the<br>signal is valid<br>Open |
| Universal output | GPOUT | 0 | - |
| Fault output (error) | ALM | 1 | 2 |
| Warning output (alarm) | WARN | 3 | 4 |
| Brake release output | BRK | 5 | Not Supported |
| Servo-on status output | SON-ST | 7 | 8 |
| Positioning complete output | IN-POS | 9 | 10 |
| Dynamic position error<br>overrun | DYM-LMT | 11 | 12 |
| Torque reaches output | TQ-REACH | 13 | 14 |
| Output during torque limit | T-LMT | 15 | 16 |
| Speed consistent output | V-COIN | 17 | 18 |
| Speed reaches output | AT-SPD | 19 | 20 |
| Output in speed limit | V-LMT | 21 | 22 |
| Servo Ready output | S-RDY | 23 | 24 |
| Return-to-origin completion<br>signal | HOMED | 25 | 26 |
| Same location | P-COIN | 31 | 32 |
| Zero speed signal | Z-SPD | 33 | 34 |
| Torque consistent output | T-COIN | 35 | 36 |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-14 | MO3 | Digital output 3 function |  | 0 ,5 |  | P | S | T |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-24 | BD | Movement waiting time after brake<br>release | 200 | 0 ~ 32000 | ms | P | S | T |
| Set the waiting time for the first movement after the driver is enabled, to ensure that the brake release output pin has successfully<br>released the motor brake. |  |  |  |  |  |  |  |  |

<!-- page 145 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-25 | BE | Motor disable delay time after brake applied | 200 | 0 ~ 32000 | ms | P | S | T |
| The 5-25 parameter sets the delay time between the moment the brake is applied to the motor and the moment the<br>motor is disabled. This ensures that the brake is fully applied, before disabling the motor to make sure the load does<br>not move unexpectedly.<br>ON<br>S_ON Signal<br>OFF<br>ON<br>Energization of motor<br>OFF<br>ON<br>BRK Signal<br>OFF<br>ON<br>Brake Action<br>OFF<br>ON<br>Motion Instructions<br>OFF<br>ON<br>Actaul Motion<br>OFF<br>Release delay P5-24 settings Brake delay P5-25 settings |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-27 | HX | Home sensor | 4 | 1 ~ 4 | - | P | S | T |
| This parameter is simply to monitor which input pin has been assigned the "Home Sensor" function. To assign the<br>"Homing Sensor" function to an input, this must be configured separately via the P5-00 ~ P5-03 parameters.<br>Display value Digital input pin<br>1 X1<br>2 X2<br>... Sequential analogy |  |  |  |  |  |  |  |  |

| Display value | Digital input pin |
|---|---|
| 1 | X1 |
| 2 | X2 |
| ... | Sequential analogy |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-28 | FI1 | Digital input 1 filtering | 2 | 0 ~ 8000 | ms | P | S | T |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-29 | FI2 | Digital input 2 filtering | 2 | 0 ~ 8000 | ms | P | S | T |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-30 | FI3 | Digital input 3 filtering | 2 | 0 ~ 8000 | ms | P | S | T |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-31 | FI4 | Digital input 4 filtering | 2 | 0 ~ 8000 | ms | P | S | T |

<!-- page 146 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-38 | PL | Dynamic error threshold | 10 | 0 ~<br>2147483647 | pulses | P | S | T |
| Position mode, parameter P5-38 sets the judgment condition for dynamic position error overrun output.<br>◆ Dynamic position error overrun output judgment conditions:<br>Dynamic position error refers to the output of this signal when the difference between the actual position of the motor and the<br>commanded position is greater than P5-38 during the operation of the motor.<br>1500<br>Motor Speed<br>0<br>P5-38<br>Position Error Curve0<br>-(P5-38)<br>Output Signal<br>Dynamic position error<br>Overrun output<br>No Output |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Default | s Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-39 | PD | Positioning complete error margin | 40 | 0 ~32000 | pulses | P | S | T |
| The position mode, parameter P5-39 sets the judgment condition of the positioning completion signal output.<br>Command Curve<br>Actual Position Feedback<br>P5-39<br>(Positioning complete<br>signal position error )<br>P5-40<br>(Count time for positioning<br>completion)<br>Position arrival signal output<br>Moving |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-40 | PE | Motion condition timer | 10 | 0 ~ 30000 | ms | P | S | T |
| Sets the timer for various motion conditions. Depending on the control mode used, this timer will be relevant to different motion<br>characteristics. Please see the following sections for detailed information.<br>7.2.3 Position Complete Signal<br>7.3.5 Velocity Reached Output<br>7.3.6 Velocity Consistent Output |  |  |  |  |  |  |  |  |

<!-- page 147 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-42 | ZV | Zero speed judgment threshold | 0.5 | 0.1 ~ 2 | rps | P | S | T |
| P5-42 parameter is set to judge whether the motor has zero speed or not.<br>When the absolute value of the actual speed of the motor is less than P5-42 (zero speed judgment threshold), the servo drive outputs<br>the zero-speed signal Z-SPD. On the contrary, if the absolute value of the actual speed of the motor<br>If it is larger than this value, the zero-speed signal Z-SPD will not be output.<br>The judgment of the zero-speed signal is not affected by the control mode and servo state. Therefore, this signal can also be used as<br>the moving signal of the motor.<br>Speed Motor Speed<br>P5-42<br>Time<br>-P5-42<br>Zero Speed Signal<br>Output Output<br>No Output No Output |  |  |  |  |  |  |  |  |

| No Output |
|---|

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-43 | VR | Velocity Reached - Permissible<br>fluctuation range | 0.1 | 0 ~ 100 | rps | P | S | T |
| In velocity control mode, when the fluctuation of the actual velocity from the commanded velocity is within the margins set by P5-43,<br>for the duration of time specified by P5-40, then it is determined that the actual speed of the motor is consistent with the commanded<br>velocity and the velocity consistent signal, V-COIN, is output.<br>If the actual velocity falls outside of the margins set by P5-43, V-COIN will not be output.<br>Speed<br>P5-43 Command Speed<br>Actual Speed<br>P5-43<br>Time<br>P5-40 P5-40<br>Speed Consistent Output<br>Output Output Output<br>No Output No Output |  |  |  |  |  |  |  |  |

| No Output | P5-40 | No Output |
|---|---|---|

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-44 | VV | Velocity Reached - Minimum<br>threshold | 10 | 0 ~ 100 | rps | P | S | T |
| In velocity control mode, when the actual speed of the motor after filtering exceeds P5-44, and the time exceeds P5-40 (counting time<br>for positioning completion), the signal for velocity reached, AT-SPD, is output.<br>the signal for velocity reached, AT-SPD, is output.<br>If the actual speed of the motor after filtering does not exceed P5-44 ", the signal AT-SPD will not be output"<br>Speed<br>Motor Speed<br>P5-44<br>Time<br>P5-40<br>Speed Reaches Output<br>Output<br>No Output No Output |  |  |  |  |  |  |  |  |

| P5-40 |
|---|

<!-- page 148 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-45 | TV | Torque Reached - Permissible<br>fluctuation range | 10 | 0 ~ 3000 | 0.1% | P | S | T |
| When operating in torque control, if the absolute value of the actual torque reaches the target torque (P1-07), remains within the<br>permissible fluctuation range (P5-45) for the amount of time specified by P5-40, the Torque Reached output signal (TQ-REACH) will<br>be output.<br>If any of the above conditions is not satisfied, the TQ-REACH signal will not be output.<br>Torque<br>P5-45<br>P1-07<br>-P5-45<br>Time<br>Torque reaches output<br>Output<br>No Output No Output |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-46 | DG | Absolute position | 10000 | -2147483647 ~<br>+2147483647 | pulses | P | S | T |
| When the actual position of the motor is equal to the setting of parameter P5-46, it will output the same P-COIN signal. The fluctuation<br>range is ± 100pulses. .<br>Actual Position<br>(P5-46)+100pulses<br>Parameters (P5-46)<br>(P5-46)-100pulses<br>Time<br>Output<br>Position Reached Output<br>No Output No Output |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-47 | LP | Positive software limit | 0 | -2147483647 ~<br>+2147483647 | pulses | P | S | T |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-48 | LM | Negative software limit | 0 | -2147483647 ~<br>+2147483647 | pulses | P | S | T |
| Parameter P5-47 ~ P5-48 set the internal software limits of the drive.<br>When the motor is moving in the positive direction, and the current position is equal to or exceeds the set value of P5-47, the positive<br>software limit reached alarm will be generated<br>When the motor is moving in the negative direction, and the current position is equal to or exceeds the set value of P5-48, the<br>negative software limit reached alarm will be generated<br>Note:<br>◆ When the encoder type is incremental encoder, after the drive is powered on, parameters P5-47 and P5-48 can be set and the<br>software limit can work normally, but cannot be saved after power off. After power on again, the parameter returns to the default value<br>" 0 " |  |  |  |  |  |  |  |  |

<!-- page 149 -->

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-49 | HE | Homing method | 1 | -4 ~ 40 | - | P | S | T |
| Set the method of homing.<br>For details on how to home, please refer to the chapter 7.8 Home Function |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-50 | EO | E-Stop method | 5 | 1 ~ 8 | - | P | S | T |
| Set the emergency stop method when emergency stop is triggered by digital input. |  |  |  |  |  |  |  |  |

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P5-51 | MS | zero speed clamp function | 0 | 0 ~ 1 | - | P | S | T |
| See Section 7.3.2 Zero Speed Clamp Function for detailed information on this parameter. |  |  |  |  |  |  |  |  |

<!-- page 150 -->

## 9 Troubleshooting

### 9.1 List of drive alarms

When an alarm occurs in the driver, the LED digital display on the driver flashes to display the
alarm code. The alarm code is defined as follows:

| Display<br>content | Description | Type of alert | Drive status after alarm | Resettable |
|---|---|---|---|---|
|  | Drive over temperature alarm | Fault | Servo off | Yes |
|  | Internal voltage alarm | Fault | Servo off | no |
|  | Drive overvoltage alarm | Fault | Servo off | Yes |
|  | Overcurrent | Fault | Servo off | Yes |
|  |  | Fault | Servo off | Yes |
|  |  | Fault | Servo off | Yes |
|  | Encoder signal error | Fault | Servo off | no |
|  | Position error overrun | Fault | Servo off | Yes |
|  | Drive low voltage alarm | Fault | Servo off | Yes |
|  | Stall Alarm | Fault | Servo off | Yes |
|  | Limit switch triggered | Warning | The motor cannot rotate without<br>changing the current state | Yes |
|  | Negative limit switch triggered | Warning | Without changing the current state, the<br>motor cannot continue to reverse | Yes |
|  | Positive limit switch triggered | Warning | Without changing the current state,<br>the motor cannot continue to rotate<br>forward | Yes |
|  | Current Limit | Warning | Do not change the current state | Yes |
|  | Communication error | Warning | Do not change the current state | Yes |
|  | Parameter save failed | Warning | Do not change the current state | Yes |
|  | Safe torque off | Warning | Servo off | Yes |
|  | Regeneration Failed | Fault | Servo off | Yes |
|  | Undervoltage warning | Warning | Do not change the current state | Yes |
|  | No Q program warning | Warning | Do not change the current state | Yes |
|  | Motion command received while<br>motor disabled | Warning | Do not change the current state | Yes |
|  | Internal voltage error | Fault | Servo off | no |
|  |  | Fault | Servo off | no |
|  | Emergency stop | Warning | Motor decelerates to stop | Yes |

<!-- page 151 -->

| Display<br>content | Description | Type of alert | Drive status after alarm | Resettable |
|---|---|---|---|---|
|  | Memory error | Fault | Servo off | Yes |
|  | Motor overtemperature | Fault | Servo off | Yes |
|  | Drive processor<br>overtemperature | Fault | Servo off | Yes |
|  | Motor stall | Fault | Servo off | Yes |
|  | Homing parameters<br>configuration error | Warning | Do not change the current state | Yes |
|  | Motor Collision Alarm | Fault | Servo off | Yes |
|  | Encoder communication error | Fault | Servo off | no |
|  | IO signal used is not general<br>purpose | Warning | Do not change the current state | no |
|  | Bus watchdog trigger | Warning | Do not change the current state | no |

<!-- page 152 -->

### 9.2 Driver Alarm Causes and Solutions

| Display<br>content | Description | Alarm Causes | Alarm Fixes | Alarm Status |
|---|---|---|---|---|
|  | Drive over<br>temperature | The temperature of the driver's heat<br>sink and power element exceeds the<br>specified value.<br>1. Ambient temperature is too high<br>2. The operating temperature of the<br>driver exceeds the specified value;<br>3. Overload, exceeding the rated<br>load of the drive and connecting<br>continue to use | 1. Reduce drive operating temperature and<br>improve cooling conditions;<br>2. Increase the capacity of the driver and<br>motor, and extend the acceleration and<br>deceleration time time, reduce the load.<br>3. Replace the fan or send in the servo drive<br>for repair | Alarm clear |
|  | Internal voltage<br>error | The internal voltage of the driver is<br>lower than the normal value | Check the voltage of the power supply and<br>replace the drive if there is still a problem. | Alarm clear |
|  | Drive<br>overvoltage | The drive DC bus voltage is too high,<br>higher than 80VDC<br>1. The power supply voltage exceeds<br>the allowable input voltage range<br>2. The regenerative discharge<br>resistor is disconnected;<br>3. The external regenerative<br>discharge resistor does not match,<br>resulting in<br>Inability to absorb regenerative<br>potential;<br>4. Drive failure (circuit failure). | 1. Check the input voltage;<br>2. Check whether the external absorption<br>resistance is set properly;<br>3. Detect the resistance value of the external<br>absorption resistor, if it is ∞, it is off If the<br>wire is damaged, please replace the external<br>absorption resistor;<br>4. If the above does not solve the problem,<br>replace the drive. | Alarm clear |
|  | Overcurrent | 1. Drive has failed and is damaged.<br>2. Motor phases are shorted.<br>3. Motor is burned out.<br>4. Poor motor power cable contacts.<br>6. The load is too heavy and required<br>torque higher than the motor's rated,<br>continuous operation torque.<br>7. Improper gain values are affecting<br>motor performance.<br>8. The load has encountered a hard<br>stop.<br>9. The E-Brake is active.<br>10. Improper wiring e.g. motor power<br>cables are connected to the wrong<br>component on a machine. | 1. Remove the motor cable, turn on the servo, if<br>it still happens If it fails, it needs to be replaced<br>with a new drive;<br>2. Check motor cable connections U, V, W<br>Whether there is a short circuit, whether the<br>connector wire has burrs, etc., connect the<br>motor cable correctly;<br>3. Check motor cables U, V, W sequence is<br>correct, U- red, V -yellow, W - blue;<br>4. Check the U, V, W of the motor cable<br>Insulation resistance between the motor and the<br>ground wire of the motor, if the insulation is poor,<br>please replace the motor with a new one;<br>5. Increase the power of the motor. Extend the<br>acceleration and deceleration time, reduce load;<br>6. Check the motor connections U, V, W Check<br>whether the connector plug is off, if it is loose or<br>off, it should be tightened; 7. Whether the gain<br>parameters are properly debugged;<br>8. Measure the voltage of the brake terminals;<br>9. Correctly connect the motor cable and<br>encoder cable to their respective on the<br>corresponding axis. | Alarm clear |
|  | Encoder not<br>connected | Motor encoder not connected | 1. Make sure the encoder cable is connected<br>to the driver correctly<br>2. Replace the encoder cable | Re-power on to<br>clear |
|  | Position error<br>overrun | Position error exceeds the value in<br>parameter P3-04 (PF)<br>"Position error limit" setting | 1. Check parameter P3-04 (PF) "Position error<br>alarm limit value" is set too small;<br>2. Whether the gain parameters are properly<br>debugged;<br>3. Whether the motor selection matches the<br>actual load and whether the acceleration and<br>deceleration are too big;<br>4. Whether an unreasonable torque limit is used;<br>5. The mechanical part of the motor drive is<br>stuck, the motor is blocked<br>6. Whether the motor power line is connected<br>correctly, there are multiple motors when the<br>power cable is connected to the correct drive. | Alarm clear |

<!-- page 153 -->

| Display<br>content | Description | Alarm Causes | Alarm Fixes | Alarm Status |
|---|---|---|---|---|
|  | Drive low<br>voltage alarm | DC bus voltage is too low. Input<br>power has fallen below 18 VDC.<br>1. The power supply voltage is low<br>causing a power failure.<br>2. The power supply capacity is not<br>enough to operate the MBDV and<br>Servo motor.<br>3. Drive has failed (circuit failure). | Measuring input voltage<br>1. Increase the power supply voltage capacity<br>and replace the power supply;<br>2. Connect the power supply correctly;<br>3. If the above does not solve the problem,<br>replace the drive. | Alarm clear |
|  | Speed exceeds<br>limit | motor speed exceeds the limit value<br>of P2-00 | Check whether the motor speed command is<br>within a reasonable range<br>1. Avoid excessive speed commands;<br>2. When overshoot occurs due to poor gain<br>adjustment, adjust the gain;<br>3. Connect the encoder cable correctly<br>according to the wiring diagram;<br>4. Check motor cable U, V, W sequence is<br>correct. | Alarm clear |
|  | Limit switch<br>triggered | 1. Positive rotation prohibition limit<br>and negative rotation of digital input<br>Turn inhibit limit is triggered<br>2. In the absolute value system, the<br>actual position of the motor touches<br>the to positive and negative software | 1. The external limit switch has been triggered;<br>2. The limit input function is not set correctly,<br>please refer to 7.1.5 Positive Reverse Limits<br>chapter.<br>3. In the absolute value system, the software<br>limit setting is unreasonable | Automatically<br>clear after<br>detachment |
|  | Negative limit<br>switch triggered | 1. Negative limit function trigger<br>2. In the absolute value system, the<br>actual position of the motor touches |  | Automatically<br>clear after<br>detachment |
|  | Positive limit<br>switch triggered | 1. Positive rotation prohibit limit<br>function trigger<br>2. In the absolute value system, the<br>actual position of the motor touches<br>the to the positive software limit |  |  |
|  | Current Limit | 1. The load is too heavy, and the<br>effective torque exceeds the rated<br>speed<br>torque, continuous operation for a<br>long time;<br>2. Poor gain adjustment leads to<br>oscillation, vibration,<br>The motor has vibration and<br>abnormal sound;<br>3. The machine is collided and the<br>load suddenly becomes heavier,<br>Torsion entanglement occurs. | 1. Whether the gain parameters are properly<br>debugged;<br>2. Whether the motor selection matches the<br>actual load and whether the acceleration and<br>deceleration<br>is too big;<br>3. Check motor cable U, V, W sequence is<br>correct. U-<br>red, V -yellow, W - blue;<br>4. Increase the capacity of the driver and<br>motor, and prolong the acceleration and<br>deceleration<br>time, reduce the load. | less than the<br>motor rating<br>at constant<br>current<br>auto clear |
|  | Communication<br>error | Check the communication error<br>when the drive is connected to the<br>host computer | 1. The Luna software is trying to establish<br>communication with the drive (this<br>is a normal alarm)<br>2. Check the communication line and<br>communication address, whether the baud<br>rate is set<br>correct | Automatically<br>clear after<br>normal<br>communication |
|  | Parameter save<br>failed | Failed to save parameters | Please try saving again | auto clear |
|  | Safe torque off | Safe Torque Off STO function<br>is activated, please reword this<br>correctly so that it makes sense. | Confirm safety input 1, 2 The status of the<br>input wiring or the setting of the safety sensor<br>is triggered. | Automatically<br>clear after STO<br>input is normal |

<!-- page 154 -->

| Display<br>content | Description | Alarm Causes | Alarm Fixes | Alarm Status |
|---|---|---|---|---|
|  | Regeneration<br>Failed | The regeneration energy exceeds<br>the regeneration absorption<br>resistance can<br>capacity.<br>1. Due to the large load inertia,<br>the generate energy, causing the<br>bus voltage to rise, regeneration<br>Abnormality due to insufficient<br>energy absorption of the absorbing<br>resistor The detected value rises;<br>2. The motor speed is too high to<br>decelerate at the specified speed<br>Regenerate energy is completely<br>absorbed within time. | 1. The external regenerative discharge<br>resistance does not match, resulting in failure<br>to absorb regeneration potential;<br>2. Reduce the running speed of the<br>equipment and increase the acceleration and<br>deceleration time.<br>3. Refer to chapter 4.7 REG- regenerative<br>energy absorption resistor connection line | Alarm clear |
|  | Undervoltage<br>warning | Driver undervoltage, below 20VDC 1.<br>The power supply voltage is low, and<br>an instant power failure occurs;<br>2. The power supply capacity is<br>insufficient, when the main power<br>supply is turned on the impact of the<br>inrush current, resulting in the supply<br>voltage under drop;<br>3 Drive failure (circuit failure) | Check the input voltage<br>1. Increase the power supply voltage capacity<br>and replace the power supply;<br>2. Connect the power supply correctly, please<br>refer to 4.3 Main -Driver Electric source wiring<br>method;<br>3. Check the Main terminal of the driver and<br>the | Alarm clear,<br>automatically<br>clear when<br>the voltage is<br>normal |
|  | No Q program<br>warning | The drive is running in Q mode, but<br>no Q program is running | 1. Check if there is a Q program;<br>2. Check whether the working mode is correct;<br>3. Check whether the Q program is written<br>incorrectly and cannot be run in a loop. | Alarm clear |
|  | Motion<br>command<br>received while<br>motor disabled | When the motor is not enabled, the<br>operation command is received | Please enable the motor first, and then send<br>the running command | Alarm clear<br>automatic clear |
|  | Internal voltage<br>error | The internal voltage of the driver is<br>lower than the normal value | Check the voltage of the power supply, if<br>there is any problem, please contact the<br>manufacturer. | Re-power on to<br>clear |
|  | emergency stop | Digital input emergency stop function<br>is triggered | 1. Confirm the emergency stop input switch<br>2. Confirm whether the emergency stop input<br>logic setting is reasonable | Automatically<br>cleared when<br>emergency<br>stop input is<br>released |
|  | memory error | Abnormal internal memory of the<br>drive | If it cannot be cleared after powering on again,<br>please contact the manufacturer. | Re-power on to<br>clear |
|  | Motor<br>overtemperature | The drive detects that the motor<br>temperature exceeds the allowable<br>value | 1. Check if the ambient temperature where the<br>motor is located is too high<br>2. Reduce the ambient temperature of the motor<br>and improve the cooling conditions;<br>3. Increase the capacity of the driver and motor,<br>and extend the acceleration and deceleration<br>time<br>time, reduce the load.<br>4. Whether the motor is rubbed by the load<br>5. When using a motor with an oil seal, please<br>derate it. Electricity The output torque of the<br>motor should be 70% of the rated torque of the<br>motor<br>6. The temperature rise and torque of the motor<br>is the motor installed in the standard cooling<br>measured on the board, when the motor<br>mounting board is small, in order to prevent<br>The motor is overheated, please use with<br>derating<br>7. The temperature of the motor is normal and<br>cannot be cleared by re-powering on, please<br>update<br>change the motor | Re-power on to<br>clear |

<!-- page 155 -->

| Display<br>content | Description | Alarm Causes | Alarm Fixes | Alarm Status |
|---|---|---|---|---|
|  | Drive<br>processor<br>over<br>temperature | Drive processor temperature is too<br>high | 1. Detect whether the temperature of the drive<br>installation environment is too high<br>2. Reduce the ambient temperature of the<br>drive and improve the cooling conditions<br>3. The drive needs to be mounted on a metal<br>backplane with good heat dissipation<br>4. Increase the capacity of the driver and<br>motor, and extend the acceleration and<br>deceleration time<br>time, reduce the load.<br>5. Replace the fan or send in the servo drive<br>for repair<br>6. Alarm after drive heatsink temperature is<br>normal and power cycle<br>still exists, please replace the drive | Alarm clear |
|  | Motor stall | Working in non-torque mode, when<br>the motor is stalled<br>time exceeds the time set by P1-28 | 1. Check whether the mechanical parts driven<br>by the motor are stuck<br>2. Check if the electromagnetic brake is open | Alarm clear |
|  | Homing<br>parameters<br>configuration<br>error | Origin returns parameter<br>configuration error<br>1. Use the back-to-origin method<br>with limit signal,<br>Limit<br>Signal not configured<br>2. Use the back-to-origin method<br>with the origin signal,<br>origin<br>Signal not configured | Check whether the origin return parameters<br>are fully configured | Alarm clear |
|  | Motor Collision<br>Alarm | The servo system detects that the<br>motor current is abnormal<br>mutation<br>1. Motor-driven loads with other fixed<br>negatives load collision<br>2. The servo gain setting is<br>unreasonable and the gain is too<br>large<br>3. Motor UVW phase sequence error,<br>motor speeding | 1. Check motor UVW phase sequence<br>2. Check whether the servo gain parameters<br>are reasonable<br>3. Check the load | Alarm clear |
|  | Encoder<br>communication<br>error | The servo system detected an<br>abnormality in the communication<br>with the servo motor encoder<br>1. Coded wires are not wired as<br>correctly defined<br>2. There is no connection code<br>between the drive and the motor<br>device line<br>3. The encoder wire is in poor<br>contact or disconnected<br>4. Interference causes abnormal<br>encoder communication<br>5. Encoder damaged | 1. Check that the encoder wiring is according<br>to the correct definition<br>2. Check the connection between the encoder<br>cable and the driver and motor<br>3. Make sure the motor and driver are well<br>grounded<br>4. The encoder cable uses a twisted pair with<br>good anti-interference ability shielded wire<br>5. Swap the motor and encoder wiring harness<br>respectively, and confirm that the motor is no<br>exception | Re-power on to<br>clear |
|  | IO signal used<br>is not general<br>purpose | 1. The function of the I/O signal used<br>in the Q program is not a general-<br>purpose function<br>2. Functions of I/O Signals Used in<br>SCL Instructions<br>for non-generic functions | 1. Configure the relevant I/O signal function as<br>a general function<br>2. Use I/O signals whose functions are<br>general-purpose functions | Alarm clear |
|  | Bus watchdog<br>trigger | bus watchdog function is turned on<br>1. Within the set time, the drive does<br>not receive the specified message | 1. Check if the CANopen communication line<br>is well connected<br>2. Check that the time set in the drive to detect<br>a specific message is no too short<br>3. Check whether the time when the controller<br>side sends a specific message too long | Alarm clear |

<!-- page 156 -->

## 10 Servo gain setting

Servo gain tuning is a function to optimize the response of the servo unit.
The servo drive needs to operate the motor according to the commanded requirements with
as little delay as possible. In order to match more closely the commanded motion, it might be
necessary for users to adjust tuning parameters. The requirement to tune is based on operation
requirements and load characteristics and should not be considered a blanket requirement for
all applications. The factory default tuning parameters can meet a wide range of applications,
especially when there exists a mechanical reduction in the system.

Instruction Instruction
Instruction
Target Curves Target Curves Target Curves

Actual Curves Actual Curves Actual Curves

Time Time Time

The target curve is inconsistent with the actual curve Adjust the gain parameters. Improve the gain parameters,
The target curve is different the target curve and the actual
from the actual curve. curve basically coincides
The actual curve gradually
approximates

### 10.1 Servo tuning process and mode introduction

#### 10.1.1 Servo tuning flow chart

The servo tuning flow chart is as follows. Before starting the servo tuning, make sure that the servo
system can run normally according to the test run in Chapter 6.

No-load operation has been
determined to be normal

No-Tuning Mode 10.2 No tuning completed

Does the performance Yes Tuning Completed
meet the requirements?

No

Auto tuning mode See Section 10.3 Auto Tuning Mode

Does the performance Yes Tuning Completed
meet the requirements?

No

Advanced Tuning Mode See Section 10.4 Advanced Tuning Mode

Yes
Does the performance Tuning Completed
meet the requirements

No

<!-- page 157 -->

#### 10.1.2 Introduction to parameter tuning mode

There are three Tuning Modes. The desired tuning mode is selected by parameter P0-00. See below for
details.

| Parameter P0-00<br>set value | Parameter tuning<br>mode | Modifiable Tuning<br>Parameters | Introduce |
|---|---|---|---|
| 0 | No tuning | P0-03 1st mechanical<br>stiffness level<br>P0-04 2nd mechanical<br>stiffness level | When No Tuning is selected, the available parameters for<br>performance modification are limited. Users will only have<br>access to the mechanical stiffness parameters. |
| 1 | Auto-tuning | P0-03 1st mechanical<br>stiffness level P0-04 2nd<br>mechanical stiffness<br>level P0-02 Load inertia<br>ratio | In "auto-tuning mode", the servo system will automatically<br>identify the external load inertia<br>ratio, automatically select the appropriate stiffness level, and<br>automatically adjust the following<br>◆ (Manual modification is invalid):<br>◆ Gain (position loop, velocity loop)<br>◆ Filter (torque filter)<br>◆ Vibration suppression and other parameters |
| 2 | Advanced tuning | P0-05, P0-07<br>P0-08, P0-11<br>P0-12, P0-13<br>P0-16<br>P0-17, P0-19<br>P0-20, P0-21<br>P0-22, P0-23<br>P0-24<br>P0-25, P0-27<br>P0-28, P0-29<br>P0-30, P0-31<br>P0-32 | In advanced tuning mode, the user will have access to all<br>servo tuning parameters and can modify them to meet their<br>application needs. |

P0-32

<!-- page 158 -->

### 10.2 No-Tuning Mode

" Tuning-free mode " is the default mode of the servo when it leaves the factory. The servo system
is in a relatively stable state with low mechanical Stiffness. It can be powered on and run after
installation, which meets most application requirements.
You can try to select the initial mechanical Stiffness that can make the servo system move
normally, and gradually adjust the mechanical Stiffness level to make the servo mechanical
Stiffness meet the application requirements.

Tuning free mode

Modify po-03 first
Mechanical Stiffness Level*1

Yes
Whether performance End of setting
meets requirements

No
Save parameters

**Note in this mode:**
◆ The inertia ratio P0-02 is forced to the default value of 0 and cannot be modified.
◆ Modification of other gain parameters is not available.
◆ When the gain is switched, the second group of mechanical Stiffness level PO-04 is valid

<!-- page 159 -->

### 10.3 Auto tuning mode

In "auto-tuning mode", the servo system will automatically identify the external load inertia ratio,
automatically select the appropriate mechanical Stiffness level, and automatically optimize and
adjust the following contents:
- Gain (position loop, velocity loop)
- Filter (torque filter) During auto-tuning, the parameters in the table below will be changed
automatically. After the auto-tuning is completed, the parameters are automatically stored in the
drive.

| Parameter | Name | Whether manual modification is valid in<br>auto-tuning mode |
|---|---|---|
| P0-02 | Load inertia ratio | Yes |
| P0-03 | 1st mechanical stiffness level | Yes |
| P0-05 | 1st position loop gain | no |
| P0-07 | 1st position loop derivative time constant | no |
| P0-08 | 1st position loop derivative filter | no |
| P0-09 | Velocity feedforward gain | no |
| P0-10 | Velocity feedforward filter frequency | no |
| P0-11 | 1st velocity command gain | no |
| P0-12 | 1st velocity loop gain | no |
| P0-13 | 1st speed loop integral time constant | no |
| P0-14 | Acceleration feed forward gain | no |
| P0-15 | Acceleration feedforward filter frequency | no |
| P0-16 | 1st command torque filter frequency | no |

#### 10.3.1 Auto-tuned motion profile conditions

In order to accurately complete the automatic tuning of parameters, it is necessary to set a
reasonable motion trajectory, including distance, running speed, running time, acceleration and
deceleration, and interval time between two movements.
Running time: greater than 0.5 seconds
Running speed: greater than 180rpm
Acceleration and deceleration: greater than 30rps/s
Interval time: greater than 1.5 seconds

Speed Running Time Interval Time Running Time

| ng | Acceleration Deceleration |
|---|---|
| ng |  |
| Acceleration |  |

Runnin
Speed

Time

Before starting auto-tuning, it is recommended that the mechanical Stiffness level of P0-03 be 5

<!-- page 160 -->

#### 10.3.2 Auto-Tuning Flowchart

Users can perform automatic parameter tuning and debugging through Luna software or the
operation panel on the driver. The flow chart of automatic tuning is as follows

Auto Tunning

There’s a host computer There is no host computer
The host computer Use the software to plan
plans the motion the trajectory and turn
trajectory on auto-tuning

Use the software to
turn on auto-tuning

Automatic calculation
of load inertia ratio

Real-time automatic
stiffness rating

Determination of No
auto-setting Automatic setting
preconditions met failure alarm
Reset a reasonable trajectory
and judgment conditions
Yes

Auto-tuning is completed,
and the tuning parameters
are automatically saved

After completing the automatic tuning, you can continue to use parameters P0-03 and P0-04 to
adjust the response and mechanical Stiffness of the servo system.

<!-- page 161 -->

#### 10.3.3 Start auto-tuning -- software operation on

It is recommended that Luna software be used for automatic tuning mode. The steps are as
follows.
Step 1: Use the connection wizard ---- select the drive to be connected ---- click " Next " to
establish communication with the drive

Step 2: Set the control mode to position control

<!-- page 162 -->

Step 3: Select the " Tuning " function in the left tree interface

Step 4: In the debugging interface, set the parameter tuning mode to " Auto tuning "

- First mechanical Stiffness grade:
Set the appropriate first mechanical Stiffness level (P0-03), the general recommended value is
“5” when running for the first time
- Load type
According to the current load, select the corresponding load type

| Load type | Description |
|---|---|
| normal load | Suitable for most loads except belt loads |
| Rigid load | Horizontal turntable, ball screw, etc. with good mechanical<br>mechanical Stiffness |
| Flexible load | Suitable for loads with poor mechanical Stiffness such as belts<br>and chains |

and chains

<!-- page 163 -->

- Load inertia ratio
If the current load inertia ratio is known, input it into " Load inertia ratio (P0-02) “, which can
improve the system mechanical Stiffness and speed up the auto-tuning speed. If you do not know
the current load inertia ratio, you do not need to fill in, the system will automatically identify the load
inertia ratio.
- Motion control source custom trajectory planning: use the software's " custom trajectory
planning " to generate the trajectory
External motion command: select this item when using the host computer to send the motion track
- Limitations of Auto Tuning:
Vibration Level：The maximun torque vibration value that the servo system needs to be met
when in auto tuning process. The larger the set value, the higher the system stiffness after auto
tuning is finished.
Positioning Error：The maximum position following error that the servo system needs to be
met.The smaller the set value, the higher the system stiffness after auto tuning is finished.
Setting Time：The longest positioning setting time that the servo system needs to be met. The
smaller the set value, the higher the system stiffness after auto tuning is finished.
The default value of those parameters above can be used for most applications, they don't
need to be set unless excellent stiffness and system response are needed. Note that too
extreme conditions may cause servo system vibration easily and unstable.
**Step 5: Start the Auto Tuning**
After the above configurations are set, set the motion profile that meets the following conditions,
and then click the "Start Auto Tuning" button to start the tuning process. The source of motion
command can be choose by "My own profile" or "Outer motion profile".
Required Motion Profile：
Travel Time：> 0.5 seconds
Travel Speed：> 180 rpm
Acc./Dec.：> 30rps/s
Interval Time：> 1.5 seconds
- Use external motion command
Click the " Start Auto-Tuning " button, and use the host computer to send motion commands
directly.
- Custom trajectory planning
Users can also use custom trajectory planning. Set a reasonable motion trajectory according to the
above motion conditions, and click the " Start Auto-Tuning " button.

<!-- page 164 -->

- Complete automatic tuning
After completion, the following dialog box will prompt. After confirming the upload, you can see that
the first mechanical Stiffness level and the load inertia ratio have been updated.

- Error prompt
If the Auto Tuning process cannot be comleted, the following error message box may be displayed,
which means:

| Error code | Cause | Measure |
|---|---|---|
| 01 | Positioning time out. | Increase the initial stiffness<br>and the value of Setting<br>Time. |
| 02 | The Interval time of two<br>motion profile is too short. | Increase the Interval Time. |
| 03 | The stiffness is reduce to<br>the smallest value during<br>the tuning process. | Increase the Vibration Level. |
| 04 | Error control mode. | Set the control mode to<br>Position Control. |
| 05 | Servo is not enabled. | Enable the servo before<br>starting the auto tuning. |
| 06 | Error tuning mode. | Set the Tuning Mode(P0-00)<br>to "Auto Tuning" |

to "Auto Tuning"

### 10.4 Advanced Tuning Mode

Advanced tuning mode is suitable for the following situations:
1) When auto-tuning cannot be completed all the time
2) After automatic tuning, by adjusting the mechanical Stiffness of P0-03 and the inertia ratio of P0-
02, the response of the servo system still cannot meet the requirements
3) The characteristics of the parameters of each control loop of the servo have been fully
understood, and the servo gain parameters can be determined by themselves
Using advanced tuning can fine-tune the servo system gain to meet the needs of higher servo
system mechanical Stiffness, faster response time and minimum tuning time.

#### 10.4.1 Introduction to Advanced Tuning Mode

The PID loop of the servo system can be configured via multiple parameters such as position loop
gain, position loop, velocity feed forward gain, velocity loop gain and a collection of filters and other
gains.
1) After the tuning mode is switched from " Auto tuning " to " Advanced tuning “, it will inherit the
parameter values after the automatic tuning is completed, and it needs to be saved manually after
the tuning is completed.
2) If directly switching from "No Tuning Mode" to "Fine Tuning", the servo system will inherit the
factory default tuning parameters. This might require that users manually tune the servo system.

<!-- page 165 -->

#### 10.4.2 Parameters in Advanced Tuning Mode

| Parameter | Command | Function | Type |
|---|---|---|---|
| P0-01 | LY | load type |  |
| P0-02 | NR | Load inertia ratio |  |
| P0-03 | KG | 1st mechanical stiffness level | First set of gains |
| P0-04 | KX | 2nd mechanical stiffness level |  |
| P0-05 | KP | 1st position loop gain |  |
| P0-07 | KD | 1st position loop derivative time constant |  |
| P0-08 | KE | 1st position loop derivative filter |  |
| P0-09 | KL | Velocity feedforward gain |  |
| P0-10 | KR | Velocity feedforward filter frequency |  |
| P0-11 | KF | 1st velocity command gain | First set of gains |
| P0-12 | VP | 1st velocity loop gain |  |
| P0-13 | VI | 1st speed loop integral time constant |  |
| P0-14 | KK | Acceleration feed forward gain |  |
| P0-15 | KT | Acceleration feedforward filter frequency |  |
| P0-16 | KC | 1st command torque filter frequency | First set of gains |
| P0-17 | UP | 2nd position loop gain | The second set of gains<br>when using gain switching |
| P0-19 | UD | 2nd position loop derivative time constant |  |
| P0-20 | UE | 2nd position loop derivative time constant |  |
| P0-21 | UF | 2nd velocity command gain |  |
| P0-22 | UV | 2nd velocity loop gain |  |
| P0-23 | UG | 2nd velocity loop integral time constant |  |
| P0-24 | UC | 2nd command torque filter frequency |  |
| P0-39 | LR | Velocity Feedback Filter |  |

**Note:**
1) When using gain switching, the second group of gains is valid.

<!-- page 166 -->

#### 10.4.3 Servo system parameter description

The servo system is composed of current loop, speed loop and position loop.
When you need to improve your response
1) Increase the mechanical Stiffness level
2) Increase the position loop gain
3) Increase the speed loop gain
4) Reduce the integral time parameter of the speed loop
when the system has overshoot and vibrations
1) Reduce the mechanical Stiffness level
2) Reduce the position loop gain
3) Reduce the speed loop gain
4) Reduce the speed loop integral time parameter
5) Reduce the torque filter frequency
6) Appropriately adjust the differential filter frequency
If one parameter is changed, other parameters also need to be re-adjusted. Please don't make
major changes to just one parameter. Generally, about 5 % is used as a rough standard, and
each servo gain is slightly adjusted. Regarding the procedure for changing the servo parameters,
generally observe the following.

##### 10.4.3.1 Gain parameters of the position loop:

◆ Position loop gain

| Parameter | Instruction | Name | Defaults | Range | Unit | Related Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-05 | KP | 1st position loop gain | 52 | 0 ~ 20000 | 0.1Hz | P | S | T |

Set the proportional gain for position control. Increasing this parameter can improve the
responsiveness of the system, reduce the position error, and shorten the positioning time. 0 means
not used, 20000 means the proportional effect is maximized. When the proportional gain of the
position loop is too small, the response of the system will not be fast enough, and the decreasing
trend of the position error will be slow. However, if the setting is too large, it may cause positioning
overshoot or machine vibration. Generally speaking, the position loop gain cannot be greater than
the velocity loop gain.
◆ Position loop differential gain

| Parameter | Instruction | Name | Defaults | Range | Unit | Related Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-07 | KD | 1st position loop derivative time<br>constant | 0 | 0 ~ 30000 | ms | P | S | T |

Set the position loop differential time constant for position control.
0 means no derivative effect, the smaller the set value, the stronger the effect of the derivative term.
Set value of the derivative time constant (KD) is too large, the system's ability to suppress vibration
is insufficient, and obvious oscillation will occur during the acceleration / deceleration process, the
uniform speed process and after stopping, and there will be a trend of decreasing oscillation. finally
stabilized.
When the set value of the differential time constant (KD) is reasonable, the system's ability to
suppress vibration is significantly enhanced, and it tends to stabilize quickly. When the differential
time constant (KD) is set too small, the motion system will be too sensitive, easily vibrate and
generate noise. When there is vibration in the system, the differential time constant can be adjusted
appropriately, and the recommended initial value is 2000.

<!-- page 167 -->

#### 10.4.4 Gain parameter of speed loop

◆ Speed loop gain

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-12 | VP | 1st velocity loop gain | 183 | 0 ~ 30000 | 0.1Hz | P | S | T |

Set the parameters of the speed loop responsiveness. The larger the set value, the faster the
speed loop response of the servo system, it is necessary to increase the speed loop gain value
without causing system vibration. Setting the value too high will cause vibration.
The gain of the velocity loop must be larger than the position loop by 4 ~ 6 times. However,
when the gain of the position loop is larger than that of the speed loop, it will cause vibration or
positioning overshoot.
◆ Speed loop integral time constant

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-13 | VI | 1st speed loop integral time constant | 189 | 0 ~ 30000 | ms | P | S | T |

Set the integral time constant of the speed loop.
0 means no integral effect, the smaller the set value, the stronger the integral effect.
Under proportional gain control, the speed error may not return to zero, or it may take a long time
to return to zero. The integral time constant accumulates all errors and acts together with the
proportional gain. A smaller integral time constant (VI) setting value can improve the response and
responsiveness of the servo system and reduce the following error.
set value of the integral time constant (VI) is too large, the system response will be slow and the
followability will be poor.
integral time constant (VI) is too small, the excessive mechanical Stiffness of the system will cause
vibration and noise of the entire servo system. This vibration and noise occur during the entire
movement process, and it is always in an oscillating state, which cannot be stabilized.

<!-- page 168 -->

### 10.5 Resonance suppression

The machanical system has an inherent resonance frequency. If the whole system runs at this
mechanical resonance frequency poit, vibration and nosie may be caused.
M3 series provide 4 methods to suppress mechanical resonance.
1）Torque Command Filter
2）Notch Filters
3）End Effector Suppress
4）Lod Disturbance Suppress

#### 10.5.1 Torque Command Filter

| Parameter | Instruction | Name | Defaults | Range | Unit | Related<br>Patterns |  |  |
|---|---|---|---|---|---|---|---|---|
| P0-16 | KC | 1st Torque Command Filter | 1099 | 0 ~ 40000 | 01Hz | P | S | T |

Filter the command torque of Current Loop.
The filter is a single-output low-pass filter, which is used to filter the output of the PID controller
(that is, the reference current). When setting this value, consider the cutoff frequency required for
system operation. The default value of 1099 can be used in most applications.
Used in some particular applications, such as:
1) Make sure the frequency of Torque Command Filter(KC) should 3 time higher than Velocity Loop Gain.
2) There are vibration with audible noise in the mechanical system. You can try reducing this value.
3) There is a mechanical resonance, the low-pass filter cutoff frequency can be set below the
resonance frequency point so that the output of the control loop will not excite the resonance.
4) In a large inertia load system, increasing the position loop gain KP can obtain a good system
response. However, excessive gain will cause jitter, and this filter could be used to be reduced to
prevent jitter and vibrati

Torque Cmd P0-16 N Fi o lt t e c r h 1 Fi N lte o r t c 2 h ~4 C L u i r m re i n t t C C u o r n r t e r n o t l
Torque Command LPF

<!-- page 169 -->

#### 10.5.2 Notch Filters

Reducing the Torque Command Filter could solve the resonance, but it also reduce the system
response bandwidth and phase margin, and thus the system becomes unstable. In some case, it
may cause a counter-action that the resonance may not be suppressed.
If you know the resonance frequency, the notch filter can be used to suppress the resonance. The
notch filter suppresses mechanical resonance by reducing the gain at a specific frequency for
highfrequency mechanical resonance.

Phase

Computer system Amplitude Mechanical resonance frequency
-frequency characteristics

Frequency

Magnitude

Notch feature

Bandwidth frequency class

Resonance depth level

Notch center frequency Frequency

This resonance frequency can be detected through open-loop mechanical analysis. If the
resonance frequency drifts significantly with time or due to other cause, using notch filter is not
suggested.
There are 4 notch filters in M3 series and each notch filter has three parameters, which are:
◆ Center frequency of Notch Fitler
◆ Notch Bandwidth
◆ Notch Depth Level(Notch Filter Attenuation Level)
The first group and the second group are user-defined notch filters, and all parameters need to
be set bt user. The third group and the fourth group can be set manually or be set as an adaptive
notch filter which all parameters are detected by the drive in real-time and automatically set.
**Note: The center point frequency of the Notch Filter must be greater than 2 times of the**
**Torque. Command Filter(P0-16).**

<!-- page 170 -->

##### 10.5.2.1 Adaptive Notch Filter

When the servo system resonates and needs to use the notch filter, it's recommended to use the
Adaptive Notch Filter.
**◆ Scope of application and precautions：**
- Applicable to all control mode except Torque Mode
**◆ Conditions may affect nomal operation of the Adaptive Notch Filter:**
- The resonance frequency is lower than 3 times the Velocity Loop Gain
- The frequency between two resonance points is less than 100Hz
**◆ Steps for usage**
- In the "Anti-resonance" interface of Luna software, change the usage mode of "Notch Filter
3" to"Adaptive" , and then click the "Download" button to enable a self-adaptive notch filter.
- When the servo system is running, it will automatically detect resonance frequency and
take effect to suppress.
- If there is a new resonance, enable the "Notch Filter 4" with same operation.
- When the system is running ,the third and fourth goups of notch filte parameters are
automatically updated, but will not be displayed in the software interface.
- Although these parameters are updated automatically, they will not be saved automatically.
After the servo system is powered on again, the system will automatically update these
parameters when the servo is enabled and running.
This setting can prevent abnormal movement of the servo system during operation, resulting in
the notch filter parameters being updated to wrong values, which may increases vibration.
**◆ Software settings for Adaptive Notch Filter**

Step 1: In the tree list on the left, open " Anti-Resonance ", and click the " Upload " button in the
Anti-Resonance interface

<!-- page 171 -->

Step 2: Change the method of " Resonance Suppression Filter 3 " to " Adaptive ", and then click to
download

Step 3: After the download is complete, the drive will automatically detect vibration and run
immediately.

#### 10.5.3 Setting the notch filter manually

Analyze resonance frequencies
To manually set the notch filter, it is necessary to measure the actual frequency when resonance
occurs. You can use the " Mechanical Analysis " function in the Luna software.
Analysis types:

| Analysis type | Applicable load | Method Description | Precautions |
|---|---|---|---|
| Mechnical Open-loop | Horizontal load | Analyzing the resonance of a servo<br>system in open loop mode allows users<br>to analyze the real time resonance<br>frequency of the entire system. This is<br>because open loop mode does not use<br>the PID of the controller which might<br>otherwise try to react to resonance and<br>thereby provide a skewed analysis. | For mechanical open-loop analysis,<br>the drive needs to be disabled, so it<br>cannot be used for vertical loads |
| Velocity Closed-loop | Horizontal load<br>Vertical load | Analyzing the resonance of a servo<br>system in velocity mode can also<br>be used. However, it is necessary to<br>ensure that PID loop parameters are<br>set reasonably so that the closed loop<br>nature of the system will not skew<br>analysis of the resonance frequency. | ◆ The control mode of the drive needs<br>to be in the command speed mode, that<br>is, the set value of P1-00 is 10.<br>◆ The drive needs to be enabled when<br>speed closed-loop analysis is performed<br>◆ For vertical loads, ensure that there is<br>mechanical protection against falling. |

<!-- page 172 -->

##### 10.5.3.1 Using Mechanical Open Loop Resonance Analysis

**Step 1**
Before performing a mechanical open-loop analysis, ensure that
◆ The drive has passed the trial operation described in Section 6 Trial Operation.
◆ Servo system has completed parameter tuning
◆ Make sure the drive is not enabled
**Step 2**
Select an appropriate amplitude to start the system, be aware that an excessively large amplitude
may cause motion.may cause mechanical movement.

Smoothing factor:
Impacts the sampling frequency of the waveform function. The larger the value, the smoother the
waveform appears. This parameter can be convenient during resonance analysis.

<!-- page 173 -->

**Step 3**
Click the " Start Analysis " button, the servo system starts the mechanical open-loop analysis and
displays the resulting curve. Click the icon in the upper right corner of the drawing area to optimize
the display curve.

**Step 4**
Move the reference line in the " Amplitude and Phase Curves " to the place where the amplitude
curve (blue curve in the figure below) has abnormal protrusions

<!-- page 174 -->

The resonance suppression filter in the red area will display the resonance frequency of the current
reference line in real time. Click " Set as 1st Resonance Suppression Filter " or " Set as 2nd
Resonance Suppression Filter " to set the resonance frequency to Resonance Suppression Filter 1
Or the resonance frequency point of the resonance suppression filter 2.
**Step 5**
On the resonance suppression interface, select " Use " to enable the corresponding resonance
suppression filter, set the appropriate " bandwidth frequency level " and " resonance depth
level ", and click " download " to set the resonance suppression notch filter. To set a resonance
suppression notch filter, select the "Anti-Resonance" option in the device tree to the left of the
Luna Software. On the desired filter, select "use" to enable the filter, set the appropriate bandwidth
frequency level and resonance depth level. Then click download.

Note: Because mechanical open loop analysis does not include the closed loop of the servo
controller, any previously downloaded Notch Filters will not be active during the analysis. If users
wish to actually test the Notch Filters impact on the resonance of the system, they will need to run
the motor in the "Velocity Closed Loop" mode. Otherwise, the system will run in open loop without
the Notch Filter Equipped.

<!-- page 175 -->

The image below is the result viewed using the Velocity Closed Loop analysis.

##### 10.5.3.2 Using the Velocity Closed Loop Mode to analyze resonance frequency

**Step 1**
Before performing a velocity closed-loop analysis, make sure that
- The drive has passed the trial operation described in Section 6 Trial Operation.Servo
system has completed parameter tuning
- The control mode of the drive is: command speed mode
- Drive is enabled
- For vertical axis loads, it is best to use a motor with a brake to avoid accidental load drop
**Step 2**
- Select an appropriate amplitude to allow the system to vibrate. Note that an excessively
large amplitude may cause mechanical movement.
- Open " SCL Terminal " in the " Tools " menu
- Enter CM10 in the input box of the SCL terminal, and set the control mode of the drive to:
command speed mode
- Enable the driver

<!-- page 176 -->

**Step 3**
- Click the " Start Analysis " button, the servo system starts the speed closed-loop analysis, and
the curve of the result is displayed.
- Click the icon in the upper right corner of the graph area to optimize the display curve.
- Move the reference line in " Amplitude and Phase Curve " to the place where the amplitude
curve (blue curve in the figure below) has abnormal protrusions
The following figure shows obvious vibration at 1200Hz. Click " Set as the first group of resonance
suppression filters " or " Set as the second group of resonance suppression filters " to set the
resonance frequency as the resonance suppression filter 1 or the resonance suppression filter 2.

<!-- page 177 -->

**Step 4**
To set a resonance suppression notch filter, select the "Anti-Resonance" option in the device
tree to the left of the Luna Software. On the desired filter, select "use" to enable the filter, set the
appropriate bandwidth frequency level and resonance depth level. Then click download.

**Step 5**
The below waveforms showcase the results of velocity closed loop analysis

<!-- page 178 -->

### 10.6 End Vibration Suppression

Figure below, due to the long length of the end of the mechanical load, it is easy to generate low-
frequency vibration during operation and stop. This vibration is often low in frequency, generally
within 100Hz, but it will affect the positioning accuracy and settling time of the end.

Vibration at the end

The use of end vibration suppression can better suppress such vibrations, thereby improving the
positioning accuracy of the mechanical system and shortening the positioning and settling time.
Instructions to implement end vibration suppression
Step 1: Analyze Frequency
Use the oscilloRange function of Luna software to observe the curves of " target speed " and "
position error " during the motor stop phase.

Using the figure below as an example, analyze the frequency of position error fluctuations after the
target speed is zero.

Time interval T Calculate the frequency of vibration
1
F =
T Target Speed
Positional Error

<!-- page 179 -->

Step 2: Set and enable end vibration suppression
Select the Anti-Resonance interface on Luna Software. Select the End Effector Suppression Tab,
enter teh vibration frequency as measured in Step 1.

**Note:**
◆ Wrong vibration frequency will cause the end vibration suppression effect to become worse or
even increase the vibration
◆ Only the vibration frequency within 1-300Hz can be well suppressed
◆ This function may not work for vibrations due to reasons other than those related to reaching the
mechanical end of a mechanism.

### 10.7 External Vibration Suppression

Servo Systems can disturbed by external factors, such as sudden changes in load or sudden
changes in external forces such as friction caused by mechanical problems, resulting in system
instability and abnormal vibration.
Suppression of these disturbances can significantly improve system response.
To use Select the " Anti-Resonance " function in the tree menu on the left side of the Luna
software, click " End effector & Load Disturbance Suppression ", and check " Use " in the Load
Disturbance Suppression. After downloading to the driver, the end disturbance suppression will
take effect.

<!-- page 180 -->

Appendix 1: LED display character comparison table

| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|---|
| A | B | C | D | E | F | G | H | I | J |
| K | L | M | N | O | P | Q | R | S | T |
| U | V | W | X | Y | Z |  |  |  |  |

<!-- page 181 -->

## 11 Connect us

### Customer Service Center

### +86-400-820-9661

**MOONS’ Headquarter North America Company**
168 Mingjia Road, Minhang District, Shanghai 201107, P.R. China MOONS' Industries (AMERICA), Inc. (Chicago)
1113 North Prospect Avenue, Itasca, IL 60143，USA
**MOONS’ Taicang**
No. 18 Yingang Rd, Fuqiao Town, Taicang City Jiangsu Province, MOONS’ INDUSTRIES (AMERICA), INC. (Boston)
215434, P.R. China 36 Cordage Park Circle, Suite 310 Plymouth, MA 02360，USA
**Domestic Offices APPLIED MOTION PRODUCTS, INC. (Morgan Hill)**
Shenzhen 18645 Madrone Parkway, Morgan Hill, CA 95037，USA
Room 3901, Building A, Zhongguan Times Square,No 4168 Liuxian LIN ENGINEERING, INC. (Morgan Hill)
Avenue, Nanshan District, Shenzhen, Guangdong Province, 16245 Vineyard Blvd., Morgan Hill, CA 95037，USA
518000, P.R. China
**Beijing European Company**
Room 1206, Jing Liang Mansion, No.16 Middle Road of East,3rd Ring,
**MOONS’ INDUSTRIES (EUROPE) S.R.L.**
Chaoyang District, Beijing 100022, P.R. China
Via Torri Bianche n.1 20871 Vimercate(MB) Italy
**Nanjing**
Room 1101-1102, Building 2, New Town Development Center, No.126 AMP & MOONS’ AUTOMATION (GERMANY) GMBH
Tianyuan Road , Moling Street, Jiangning District, Jiangsu Province, Kaiserhofstr. 15
China, 211106, P.R. China 60313 Frankfurt am Main Germany
**Qingdao**
**Technosoft SA**
Rm1913,Scientific and Technological Innovation Builing,Floor19,
Avenue des Alpes 20
No.171, ShanDong Road,Shibei District,QingDao, Shangdong Province,
CH 2000 Neuchâtel Switzerland
266033, P.R. China
**Wuhan MOONS' INDUSTRIES (UK), LIMITED**
Room 3001, World Trade Tower, 686 Jiefang Avenue, Jianghan District, Reading, Berkshire, UK
Wuhan, Hubei Province, 430022, P.R. China
**Singapore Company**
**Chengdu**
Room. 3907, Maoye Plaza, No.19, Dongyu Street, Jinjiang Distrit, MOONS’ INDUSTRIES (SOUTH-EAST ASIA) PTE. LTD.
Chengdu Sichuan Province, 610066, P.R. China 33 Ubi Avenue 3 #08-23 Vertex Singapore 408868
**Xi ’ an**
**Japan Company**
Room 1006, Tower D, Wangzuo International City, 1 Tangyan Road,
Xi ’ an, Shanxi Province, 710065, P.R. China MOONS’ INDUSTRIES JAPAN CO., LTD.
Ningbo Room 602, 6F, Shin Yokohama Koushin Building,
Rm 309, Tower B, Taifu Plaza, 565 Jiangjia Road,Jiangdong District, 2-12-1, Shin-Yokohama, Kohoku-ku, Yokohama,
Ningbo, Zhejiang Province, 315040, P.R. China Kanagawa, 222-0033, Japan
**Guangzhou**
**India Company**
Rm 4006, Tower B, China Shine Plaza, 9 Linhe Xi Road, Tianhe District,
Guangzhou, Guangdong Province, 510610, P.R. China MOONS' INTELLIGENT MOTION SYSTEM INDIA PVT. LTD.
Chongqing Rm. 908, 9th Floor, Amar Business Park,
Room. 2108, South yuanzhu Buliding 20, No.18 Fuquan Rd., Tal. Haveli, Baner, Pune-411045, Maharashtra, India
Jiangbei District, Chongqing, 400000, P.R. China
**Vietnam Company**
**Hefei**
Room 1521, Building B, CBC Tuoji Plaza, Jinggang Road, Shushan MOONS' INDUSTRIES (VIETNAM) CO., LTD.
District, Hefei, Anhui Province, 230088, P.R. China Factory C1&D1, Lot IN3-11*A, VSIP Hai Phong Industrial
Suzhou Park in Dinh Vu – Cat Hai Economic Zone,Lap Le Commune,
Thuy Nguyen District, Hai Phong City, Vietnam 04359
Rm 1103-1105, North Building 4, Huizu Plaza, 758 Nanhuan East Rd,
Gusu District, Suzhou,Jiangsu Province, 215007, P.R. China
**Dongguan**
Room. 1106-1207, Building 5, Linrunzhigu, No.1 RD 5th Rd, Songshan
Lake, Dongguan, Guangdong Province, 523000, P.R. China

https://www.moonsindustries.com/
E-mail:ama-info@moons.com.cn

- All the specifications, technical parameters of the products provided in this catalog are for reference only, subject to change without notice.
For the latest details, please contact our sales department.
