# Moons' MBDV-2X-520AC: CANopen master hai trục (lely-core, C++14)

`mbdv_dual_axis_node` điều khiển **cả hai trục** của một drive **Moons' MBDV-2X-520AC** qua
**CANopen (CiA 301 / CiA 402)**, dùng thư viện **lely-core** (`liblely-coapp` 2.3.x). Hai trục là
hai bánh của một robot **vi sai (differential drive)**. Chương trình nhận lệnh `(v, ω)` rồi đổi ra
tốc độ từng bánh. Nó đọc encoder để tính odometry ở 200 Hz, giám sát heartbeat của từng node và
tự phục hồi khi mất kết nối hoặc khi drive báo lỗi.

Mỗi bước khởi động được ghi log theo **16 giai đoạn (S01…S16)**. Khi có lỗi, chương trình in ra
giai đoạn hỏng, lý do và việc cần kiểm tra.

> ⚠️ **An toàn.** Chương trình điều khiển động cơ thật. Khi thử nghiệm, hãy kê bánh khỏi mặt đất
> và để tay gần E-stop cho tới khi xử lý xong các rủi ro ở **mục 12**.

## Mục lục

1. Trạng thái hiện tại
2. Phần cứng và địa chỉ hoá
3. Kiến trúc phần mềm
4. Ánh xạ PDO
5. CiA 402 và chế độ drive (P1-00)
6. Biên dịch
7. Chạy chương trình
8. Cấu hình runtime (`config/params.yaml`)
9. Log theo giai đoạn và báo cáo chẩn đoán
10. Tra cứu lỗi theo giai đoạn
11. Đã triển khai và mức độ kiểm chứng
12. Hạn chế và rủi ro đã biết
13. Các phát hiện trên phần cứng thật
14. Cấu trúc thư mục
15. Tài liệu tham khảo

---

## 1. Trạng thái hiện tại (2026-10-06)

Bàn thử gồm 1 drive MBDV-2X-520AC, nguồn chính 48,2 V, bus `can0` 500 kbps qua adapter
PEAK PCAN-USB (driver `peak_usb`). Node 1 là **AX1, bánh trái**; node 2 là **AX2, bánh phải**.

| Hạng mục | Kết quả |
|---|---|
| Bring-up S01→S12, cả hai trục | PASS, servo ON sau khoảng 1,2 s |
| `--test-kinematics` (4 pha, khoảng 15 s) | PASS; in `All recorded stages passed.` và tự thoát |
| Quy đổi lệnh | `v = 0,2 m/s` → 4341 counts/s mỗi bánh; `ω = 0,5 rad/s` → AX1 −2466, AX2 +2466 counts/s |
| Đo thực tế | Đi thẳng: `v` 0,19–0,21 m/s, đi được khoảng 0,58 m sau 3 s. Quay tại chỗ: `ω` 0,46–0,54 rad/s. Đi cung: `v` ≈ 0,15 m/s, `ω` ≈ 0,3 rad/s |
| Dừng giữa các pha | Hai trục dừng cùng lúc: tới lượt kiểm tra thì AX2 đã về 0 |
| S15 tắt servo | PASS cả hai trục (Statusword `0233`, Switched On) |

Các vấn đề còn mở nằm ở **mục 12**.

---

## 2. Phần cứng và địa chỉ hoá

Theo MBDV Hardware Manual §4.2.2 (MBDV-2X-520AC):

| DIP | Ý nghĩa |
|---|---|
| `SW1..SW3` | Node-ID của trục 1; `0` = lấy từ phần mềm Luna |
| `SW4..SW6` | Node-ID của trục 2; `0` = lấy từ phần mềm Luna |
| `SW7` | `1` = ép 500 kbps; `0` = tốc độ đặt trong Luna (P1-18). `0` **không** có nghĩa là bus chạy 1 Mbps |
| `SW8` | `1` = bật trở 120 Ω (chỉ bật ở thiết bị cuối bus) |

Hai trục là **hai node CANopen độc lập** trên cùng một bus và dùng chung một file EDS.

> Trên bàn thử hiện tại, **mọi DIP đều tắt** (`0x2070 = 0x00000000`): node-ID 1/2 và tốc độ 500 kbps
> đều được đặt trong Luna. S08 kiểm tra giá trị thật qua `0x2020` (node-ID) và `0x2021` (tốc độ,
> đơn vị kbps).

---

## 3. Kiến trúc phần mềm

```text
 main thread                   lely event-loop thread                sniffer thread
 ───────────────────────────   ───────────────────────────────────   ─────────────────────────
 CLI + config/params.yaml      AsyncMaster: NMT, SDO, heartbeat      socket SocketCAN thứ hai,
 DualAxisController            MbdvAxisDriver AX1 (FiberDriver)      chỉ nghe; giải mã TPDO
   vòng 200 Hz: Supervise(),   MbdvAxisDriver AX2 (FiberDriver)      0x18x/0x28x/0x38x theo
   SetCmdVel(), odometry,        stage S05..S15 chạy trong fiber     COB-ID → statusword,
   in telemetry                  RPDO tự dựng → CanChannel::write    vị trí, vận tốc từng trục
```

Mỗi quyết định thiết kế dưới đây sửa một lỗi đã gặp trên phần cứng:

1. **RPDO được dựng tay cho từng node.** OD của master chỉ có một bản `0x6040`/`0x607A`/`0x60FF`,
   còn lely phát mọi PDO chứa object vừa ghi. Vì vậy, ghi lệnh cho một trục khiến **cả hai node**
   nhận lệnh của nhau. Trên bus đo được AX1 `+1500` và AX2 `-1500` triệt tiêu về 0, nên robot không
   quay được. `MbdvAxisDriver::SendRpdo()` tự dựng frame `0x200/0x300/0x400 + node` và gửi qua
   `CanChannel::write()`. RPDO1 mang cả `0x6060`, nên byte thứ 3 phải là mode đang chọn.
2. **Phản hồi đọc qua `CanSniffer`.** Cũng vì OD dùng chung, TPDO của hai node ghi đè lên cùng một
   object. Phản hồi được giải mã thô theo COB-ID trên một socket chỉ nghe; socket chính của lely
   dành cho SDO, NMT và heartbeat.
3. **`tools/fix_master_dcf.py`.** File `master.dcf` do `dcfgen` sinh ra trỏ PDO vào object gương
   nội bộ (`0x2001`…) và thiếu hẳn các object CiA 402. Hậu quả: lely không phát được khung RPDO nào,
   và lỗi này im lặng vì SDO vẫn chạy. Script chèn các object thật, viết lại ánh xạ, và **từ chối
   ghi file** nếu còn ánh xạ không resolve hoặc quyền truy cập sai (`AccessType` phải là `rw`).
   CMake chạy script ngay sau `dcfgen`.
4. **Giám sát và dừng an toàn.** Khi một node mất heartbeat (producer 100 ms, consumer 300 ms) hoặc
   servo rời trạng thái Operation Enabled, chương trình bật *stop output* và ra lệnh 0 cho mọi trục
   còn liên lạc được. Sau đó nó tự kết nối lại hoặc phục hồi lỗi. Chi tiết ở **mục 8** và trong
   `docx/MBDV-dual-axis-liveness-and-recovery.md`.

> **Quy tắc thread:** lệnh tới một trục đi qua strand của driver đó (`Defer`). Đừng thêm thread mới
> gửi lệnh tới driver. Lý do ở **mục 12**, rủi ro 1.

---

## 4. Ánh xạ PDO

RPDO (master → drive), transmission `0xFF` (event-driven):

| PDO | COB-ID | Ánh xạ |
|---|---|---|
| RPDO1 | `0x200 + node` | `0x6040` Controlword (16 bit) + `0x6060` Modes of operation (8 bit) |
| RPDO2 | `0x300 + node` | `0x6040` + `0x607A` Target position (32 bit) |
| RPDO3 | `0x400 + node` | `0x6040` + `0x60FF` Target velocity (32 bit) |

TPDO (drive → master), transmission `0xFF` kèm event timer:

| PDO | COB-ID | Ánh xạ | Event timer |
|---|---|---|---|
| TPDO1 | `0x180 + node` | `0x6041` Statusword | 5 ms |
| TPDO2 | `0x280 + node` | `0x6064` Position actual + `0x606C` Velocity actual | 5 ms |
| TPDO3 | `0x380 + node` | `0x603F` Error code + `0x200F` DSP alarm code | 50 ms |

- S09 tắt RPDO4/TPDO4 để giảm tải bus.
- Drive mặc định đặt RPDO2/RPDO3 ở `0xFE` (chỉ RTR), nên phải ghi `0xFF`.
- **S09 kiểm tra trước, sửa sau.** Nó đọc ngược từng PDO và chỉ nạp lại PDO sai, theo đúng thứ tự
  CiA 301: tắt (bit 31) → xoá số phần tử ánh xạ → ghi ánh xạ → transmission type → event timer →
  bật lại.
- Event timer được khai báo **ở hai nơi** và hai nơi phải khớp nhau:
  - `config/master.yaml`: nạp vào drive lúc boot qua `slave_N.bin`;
  - `rates.tpdo*_event_ms` trong `config/params.yaml`: giá trị S09 dùng để so sánh.

  Nếu lệch nhau, lần boot nào S09 cũng phải nạp lại PDO.
- Tải bus ở 200 Hz là khoảng 1200 TPDO + 480 RPDO = 1680 frame/s, tức khoảng 41% của 500 kbps. Con
  số này được in ra lúc khởi động.

---

## 5. CiA 402 và chế độ drive (P1-00)

| Bước | Controlword | Trạng thái chờ |
|---|---|---|
| Shutdown | `0x0006` | Ready to Switch On |
| Switch On | `0x0007` | Switched On |
| Enable Operation | `0x000F` | Operation Enabled (servo ON) |
| Disable Operation (S15) | `0x0007` | Switched On |

`0x6060` (Modes of operation) **chỉ có tác dụng khi P1-00 khớp**. P1-00 nằm ở object hãng `0x2A30`:

| P1-00 (`0x2A30`) | Chế độ drive | `0x6060` tương ứng |
|---|---|---|
| `21` (mặc định nhà máy) | Position Control | `1`, Profile Position |
| `15` | Velocity Control | `3`, Profile Velocity |
| `1` | Torque Control | `4`, Profile Torque |

- **S08 tự căn chỉnh P1-00.** Nếu giá trị đọc được khác chế độ cần dùng, chương trình ghi giá trị
  đúng vào `0x2A30` rồi đọc lại. Chế độ cần dùng là `15` cho teleop, `--test-velocity` và
  `--test-kinematics`, và `21` cho `--test-motion`.
- **Giá trị này không được lưu** (`0x1010:01`). Mỗi lần bật nguồn, drive quay về giá trị đã lưu; bàn
  thử hiện lưu `21`. Muốn giữ cố định thì đặt P1-00 trong Luna.
- `--p1-00 <n>` ép một giá trị cụ thể. `--control-mode 0` bỏ qua bước kiểm tra.

---

## 6. Biên dịch

Yêu cầu: CMake ≥ 3.16, trình biên dịch C++14, `lely-core` (các gói `liblely-*` qua `pkg-config`,
cùng công cụ `dcfgen`), `yaml-cpp`, Python 3.

```bash
cd /home/namnc/driver_motor/driver_Moons_MBDV
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

Khi `config/master.yaml`, file EDS hoặc `tools/fix_master_dcf.py` thay đổi, bước build tự chạy lại
`dcfgen` và `fix_master_dcf.py`. Nó **sinh lại** `config/master.dcf`, `config/slave_1.bin` và
`config/slave_2.bin`; các file này có trong git nên sẽ hiện trong `git status`.

---

## 7. Chạy chương trình

### 7.1. Thiết lập SocketCAN

```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up
```

Chương trình chạy được từ bất kỳ thư mục nào. File `config/master.dcf` được tìm lần lượt ở:
1. đúng đường dẫn đã gõ;
2. thư mục gốc dự án, ghi sẵn lúc build;
3. thư mục cha của executable;
4. thư mục chứa executable.

S01 `chdir()` vào thư mục chứa DCF để lely tìm thấy `slave_N.bin`, và khôi phục thư mục cũ khi tắt.

### 7.2. Các chế độ

| Chế độ | Làm gì | Lưu ý an toàn |
|---|---|---|
| *(không tham số)*, `-t`, `--teleop` | Lái bằng bàn phím qua động học vi sai | robot chạy theo phím |
| `--selftest` | Chỉ chạy S01→S11 (không bật servo, không kiểm tra P1-00) rồi thoát | an toàn nhất, nên chạy đầu tiên |
| `--watch` | Chạy S01→S11, rồi đọc `0x200F`/`0x1001` mỗi 250 ms và log mọi thay đổi alarm cho tới khi Ctrl+C | không bật servo |
| `--monitor` | Bring-up tới servo ON (vận tốc 0), rồi in telemetry cho tới khi Ctrl+C | không ra lệnh chuyển động |
| `--test-kinematics` | Chạy 4 pha: đi thẳng 0,2 m/s 3 s → quay trái 0,5 rad/s 3 s → đi cung 0,15 m/s + 0,3 rad/s 3 s → dừng 1 s. Tự thoát sau khoảng 15 s | robot đi khoảng 0,6 m rồi quay |
| `--test-velocity [--velocity N]` | AX1 = `+N`, AX2 = `−N` counts/s (mặc định 5000), **giữ cho tới khi Ctrl+C** | ⚠️ robot **quay tại chỗ liên tục** (khoảng 1 rad/s với N = 5000) |
| `--test-motion [--step N]` | Profile Position: AX1 `+N`, AX2 `−N` counts, dừng 1,5 s, rồi về 0 | tháo liên kết cơ khí trước khi chạy |

Trình tự khuyến nghị:

```bash
cd /home/namnc/driver_motor/driver_Moons_MBDV
sudo ./build/mbdv_dual_axis_node --selftest --diag                          # 1. kiểm tra bus
sudo ./build/mbdv_dual_axis_node --test-kinematics 2>&1 | tee /tmp/kin.log  # 2. kê bánh khỏi mặt đất
sudo ./build/mbdv_dual_axis_node                                            # 3. teleop
```

Chạy thành công khi report cuối là `MBDV DUAL-AXIS BRING-UP : SUCCESS` và có dòng `All recorded
stages passed.` Mã thoát là `0`; nếu có giai đoạn FAIL thì mã thoát là `1`.

### 7.3. Phím teleop

| Phím | Tác dụng |
|---|---|
| `W` / `↑` | tăng `v` một bước (tiến) |
| `S` / `↓` | giảm `v` một bước (lùi) |
| `A` / `←` | tăng `ω` một bước (quay trái) |
| `D` / `→` | giảm `ω` một bước (quay phải) |
| `Space` / `X` | phanh: `v = 0`, `ω = 0` |
| `+` `=` / `-` `_` | đổi bước `v` thêm ±0,02 m/s (trong khoảng 0,01…0,50; mặc định 0,05) |
| `]` `>` / `[` `<` | đổi bước `ω` thêm ±0,05 rad/s (trong khoảng 0,05…0,60; mặc định 0,15) |
| `R` | đặt lại pose odometry về (0, 0, 0) |
| `Q` / `Esc` | thoát: dừng cả hai trục rồi tắt servo |

Setpoint teleop bị giới hạn ở |v| ≤ 1,0 m/s và |ω| ≤ 2,5 rad/s.

### 7.4. Kiểm chứng PDO trên bus

`tools/verify_run.sh` chạy chương trình kèm `candump`, rồi kết luận từng tầng: master có phát RPDO
không, drive có trả TPDO không, Statusword dừng ở trạng thái nào, và vị trí có thay đổi không.

```bash
sudo ./tools/verify_run.sh                    # Profile Position, cả hai trục
sudo ./tools/verify_run.sh --test-velocity
```

### 7.5. Tuỳ chọn dòng lệnh

| Tuỳ chọn | Ý nghĩa |
|---|---|
| `-i, --interface <dev>` | Giao diện CAN (mặc định `can0`) |
| `--params <path>` | File tham số runtime (mặc định `config/params.yaml`) |
| `--dump-params` | In tham số đang dùng rồi thoát |
| `-d, --dcf <path>` | File `master.dcf` |
| `-b, --bin <path>` | Concise DCF nạp xuống slave (tuỳ chọn) |
| `-1, --axis1 <id>` / `-2, --axis2 <id>` | Node-ID từng trục (mặc định 1 / 2) |
| `--baud <kbps>` | Tốc độ bus **mong đợi**, S08 so với `0x2021` (`1000\|800\|500\|250\|125\|50\|20\|12`) |
| `--control-mode <n>` | P1-00 mong đợi (`1\|15\|21`); `0` bỏ qua kiểm tra |
| `--p1-00 <n>` | Ép ghi giá trị này vào `0x2A30` ở S08 (không lưu) |
| `--no-pdo-program` | Chỉ kiểm tra PDO, không ghi (chế độ chẩn đoán) |
| `--watchdog <ms\|off>` | Watchdog giao tiếp `0x2060` (P1-39). Xem **mục 12**, rủi ro 2 |
| `--step <counts>` | Biên độ của `--test-motion` (mặc định 10000) |
| `--velocity <counts/s>` | Setpoint của `--test-velocity` (mặc định 5000) |
| `--boot-timeout <ms>` | Timeout boot-up / SDO cho mỗi node (mặc định 3000) |
| `--servo-timeout <ms>` | Timeout cho mỗi bước chuyển trạng thái CiA 402 (mặc định 2000) |
| `--log-level <lvl>` | `trace\|debug\|info\|warn\|error\|fatal` (mặc định `info`) |
| `--log-file <path>` | Ghi thêm log ra file |
| `--no-color` | Tắt màu ANSI |
| `--diag` | In mọi object chẩn đoán của cả hai trục trước khi thoát |

---

## 8. Cấu hình runtime (`config/params.yaml`)

Mọi tham số runtime nằm trong `config/params.yaml`. Thiếu file thì chương trình dùng giá trị mặc
định kèm cảnh báo; file sai định dạng thì chương trình **từ chối chạy**. Dùng `--dump-params` để xem
giá trị đang có hiệu lực.

> `config/master.yaml` là **một file khác**: đó là đầu vào của `dcfgen` lúc build.

| Nhóm | Khoá chính (giá trị hiện tại) | Ý nghĩa |
|---|---|---|
| `bus` | `interface: can0`, `expect_bitrate_bps: 500000` | S08 so với `0x2021` |
| `nodes` | `axis1_id: 1`, `axis2_id: 2` | |
| `rates` | `control_hz: 200`, `controlword_hz: 20`, `tpdo1/2/3_event_ms: 5/5/50` | vòng điều khiển và odometry chạy 200 Hz; controlword gửi lại 20 Hz |
| `heartbeat` | `producer_ms: 100`, `consumer_ms: 300` | phát hiện mất node trong vòng 300 ms |
| `liveness` | `stop_hold_ms: 500` | thời gian tối thiểu giữ stop output |
| `reconnect` | `enabled`, `recover_can_link`, `backoff_ms: 500…5000`, `max_attempts: 0` (thử mãi) | tự kết nối lại node / CAN link |
| `fault_recovery` | `auto_reset: true`, `retry_backoff_ms: 1000`, `max_attempts: 5` | fault reset rồi bật lại servo khi nguyên nhân đã hết |
| `drive` | `expect_p1_00: 15`, `write_p1_00: 0`, `profile_accel/decel: 25000/50000`, `watchdog_ms: -1`, `watchdog_action: -1`, `program_pdos: true` | xem mục 5 và mục 12 |
| `kinematics` | `wheel_radius_m: 0.07333`, `wheel_base_m: 0.4544`, `gear_ratio: 1.0`, `encoder_cpr: 10000` | sai hình học bánh xe thì sai toàn bộ odometry |
| `odometry` | `enabled`, `callback: true`, `udp: false` (`239.255.0.10:5565`) | xuất pose/twist cho lớp phía trên (ROS 2) |
| `misc` | `dcf`, `bin`, `boot_timeout_ms: 3000`, `servo_timeout_ms: 2000` | |

**Stop output.** Khi bất kỳ trục nào bị mất hoặc lỗi, `DualAxisController::StopRequested()` bật lên
và callback `SetStopCallback()` được gọi. Nên nối tín hiệu này vào đầu vào E-STOP của drive
(1_X4 chân 4, 2_X4 chân 12, P5-03 = 14 "Open = E-stop", theo comment trong `params.yaml`): khi đã mất
CAN, chỉ phần cứng mới dừng được drive.

**STO không gỡ được bằng phần mềm** (Manual §4.11). Chương trình phát hiện và báo rõ tình trạng
này, rồi tự phục hồi ngay khi người vận hành đóng lại mạch an toàn.

---

## 9. Log theo giai đoạn và báo cáo chẩn đoán

Định dạng mỗi dòng log:

```text
[   1.166s][S12][AX1   ][INFO ] S12.3 Enable Operation reached -> 1637 [Operation Enabled (Servo ON)] ...
 thời gian  stage trục   mức    nội dung
```

| Mã | Tên | Giai đoạn này chứng minh điều gì |
|---|---|---|
| `S01` | `CONFIG` | Đọc được tham số và tìm được file DCF |
| `S02` | `CAN_LINK` | Mở được SocketCAN controller và channel |
| `S03` | `MASTER_LOAD` | Dựng được `AsyncMaster` và driver cho hai trục |
| `S04` | `EVENT_LOOP` | Thread event loop đã chạy, `master.Reset()` đã được gửi |
| `S05` | `BOOTUP` | Nhận được Boot-up (`0x700 + node = 0x00`), lely đã nạp xong concise DCF |
| `S06` | `PREOP` | SDO upload `0x6041` có trả lời |
| `S07` | `NMT_START` | NMT Start → **OPERATIONAL** |
| `S08` | `IDENTITY` | Vendor, node-ID, tốc độ bus, P1-00 khớp; ramp `0x6083`/`0x6084`; heartbeat |
| `S09` | `PDO_VERIFY` | PDO đúng ánh xạ, transmission type, event timer, đã **đọc ngược xác nhận** |
| `S10` | `MODE_OF_OP` | Đã ghi `0x6060` và `0x6061` xác nhận khớp |
| `S11` | `FAULT_RESET` | Xoá được lỗi đang kẹt (nếu có) |
| `S12` | `SERVO_ON` | Chạy xong chuỗi Shutdown → Switch On → Enable Operation |
| `S13` | `MOTION_CMD` | Ghi được target position / target velocity |
| `S14` | `MOTION_TRACK` | `Target reached` bật, hoặc vận tốc bám theo lệnh |
| `S15` | `SERVO_OFF` | Giảm tốc có kiểm soát rồi Disable Operation |
| `S16` | `SHUTDOWN` | Dừng event loop và giải phóng tài nguyên CAN |

Cuối mỗi lần chạy, chương trình in bảng kết quả theo từng giai đoạn và từng trục. Khi có lỗi, nó in
thêm khối giai đoạn hỏng đầu tiên:

```text
==============================================================================
  DIAGNOSIS - FIRST FAILING STAGE
==============================================================================
  Stage      : S08  IDENTITY
  Purpose    : verify identity + bus params vs expectation
  Axis       : AX2
  Reason     : node-ID mismatch: the master addresses node 2 but the drive reports 0x2020 = 1 (1)
  Hint       : MBDV-2X-520AC DIP switches: axis 1 node-ID = SW1..SW3, axis 2 node-ID = SW4..SW6 ...
  Last OK    : S07 NMT_START
  Conclusion : bring-up for AX2 worked up to and including S07; the fault is isolated to S08 IDENTITY.
==============================================================================
```

Các object chẩn đoán được đọc và giải mã (in đầy đủ khi chạy với `--diag`):

| Object | Nội dung |
|---|---|
| `0x6041` | Statusword, giải mã từng bit và tên trạng thái CiA 402. Bit 4 `Voltage_enabled` cho biết có nguồn chính hay không |
| `0x603F` / `0x1001` | Error code CiA 402 / error register CiA 301, giải mã đầy đủ theo chuẩn |
| `0x200F` | Mã alarm của DSP. Byte thấp **chính là số hiện trên LED** (`rNN`). EDS không công bố bảng tên lỗi, nên chương trình chỉ in mã để đối chiếu với Manual §9.1 |
| `0x2020` / `0x2021` | Node-ID thực / tốc độ bus thực (kbps) |
| `0x2070` | Bitmap DIP switch (SW1..SW8) |
| `0x2030` | Điện áp bus DC, đơn vị 0,1 V |
| `0x2AC0` | Mã phụ; **không** dùng làm chỉ báo lỗi (khi drive khoẻ vẫn trả `0x04000000`) |
| `0x6078` / `0x60F4` | Dòng thực tế / sai số vị trí |

---

## 10. Tra cứu lỗi theo giai đoạn

| Giai đoạn FAIL | Triệu chứng điển hình | Kiểm tra |
|---|---|---|
| `S01 CONFIG` | `cannot find DCF file` (kèm danh sách đường dẫn đã thử) | Build lại để sinh DCF, hoặc truyền `-d /đường/dẫn/tuyệt/đối` |
| `S02 CAN_LINK` | `No such device` / `Operation not permitted` | `ip link show can0`; bus đã `up` đúng `bitrate` chưa; chạy bằng root |
| `S03 MASTER_LOAD` | `AsyncMaster construction failed` | DCF sai cú pháp; đường dẫn EDS trong `master.yaml` |
| `S05 BOOTUP` | SDO abort `08000020` / `slave_1.bin: No such file` | **Gần như luôn là lỗi đường dẫn `.bin`**: xem dòng S01 "working directory changed to". Nếu không thấy Boot-up: nguồn 24 VDC AUX, dây `CAN_H`/`CAN_L`/GND, LED hiện mã alarm thay vì số node |
| `S06 PREOP` | SDO upload timeout | Tốc độ bus; trở 120 Ω ở thiết bị cuối; trên bus không có master thứ hai |
| `S07 NMT_START` | Không sang OPERATIONAL | Node đã tới PRE-OP chưa; có master khác gửi NMT không |
| `S08 IDENTITY` | `vendor ID` / `node-ID mismatch` / `bit-rate mismatch` / `could not write ... 0x2A30` | `SW1..SW6`, `SW7`; đối chiếu `0x2020`, `0x2021`, `0x2070`. Nếu ghi P1-00 thất bại: khoá tham số `0x2A35`. Cảnh báo về nguồn chỉ xuất hiện khi bit 4 Statusword = 0 (thiếu điện áp chính `24–60 VDC` ở `V+/V−`) |
| `S09 PDO_VERIFY` | Liệt kê từng sai lệch `0x14xx/0x16xx/0x18xx/0x1Axx` | Transmission type không được là `0xFE`. Abort `0x06010002` = ghi sai sub-index (COB-ID là `:01`). Abort `0x06090030` = ghi `0` vào COB-ID, drive này từ chối |
| `S10 MODE_OF_OP` | `0x6061` không đổi sau khi ghi `0x6060` | P1-00 trong `0x2A30` phải khớp (PP = 21, PV = 15, TQ = 1) |
| `S11 FAULT_RESET` | Không xoá được lỗi | Đọc LED, `0x603F`, `0x200F`. Lỗi không reset được (điện áp nội bộ, encoder) cần kiểm tra dây và tắt/bật nguồn |
| `S12 SERVO_ON` | Kẹt ở `Switch On Disabled` hoặc `Fault` | Xem kết quả đọc ngược `0x6040` trong log. **Không đổi** ⇒ khung RPDO không tới drive: kiểm tra ánh xạ `0x1400`/`0x1600` và dây CAN. **Đúng giá trị đã gửi** ⇒ drive nhận lệnh nhưng từ chối chuyển trạng thái: kiểm tra STO (§4.11), input `0x60FD`/`0x2A20`, `P1-02` |
| `S13 MOTION_CMD` | Lệnh không được nhận | Đúng chế độ ở S10; RPDO2/RPDO3 có `0x6040` + `0x607A`/`0x60FF`, transmission event-driven |
| `S14 MOTION_TRACK` | Không có `Target reached` / vận tốc không bám | Đã chạy thử không tải chưa; chạm giới hạn cơ khí; `P3-04` quá nhỏ; `P1-06` quá thấp; encoder không phản hồi |
| `S15 SERVO_OFF` | Không tắt được servo | Lỗi đang kẹt, hoặc node đã rời OPERATIONAL. Nếu `Disable Operation` không có tác dụng, chương trình ép `Disable Voltage` (`0x0000`) |

---

## 11. Đã triển khai và mức độ kiểm chứng

| Chức năng | Trạng thái |
|---|---|
| Bring-up S01→S16, kiểm chứng từng giai đoạn | ✅ đã chạy trên phần cứng (2026-10-06) |
| Kiểm tra và sửa PDO ở S09 | ✅ phần cứng |
| Tự căn chỉnh P1-00 (không lưu) | ✅ phần cứng (21 → 15) |
| RPDO dựng tay theo node, phản hồi qua `CanSniffer` | ✅ phần cứng |
| Động học vi sai, odometry 200 Hz | ✅ phần cứng (`--test-kinematics`) |
| Dừng hai trục cùng lúc giữa các pha | ✅ phần cứng (2026-10-06) |
| S15 tắt servo cả hai trục | ✅ phần cứng (2026-10-06) |
| Report được in và process tự thoát khi có giai đoạn FAIL | ☑️ đã sửa trong code; chưa gặp lại lần chạy có FAIL để kiểm chứng |
| `--test-velocity` giữ tốc độ cho tới Ctrl+C | ☑️ đã sửa trong code; chưa chạy lại |
| Teleop bàn phím | ⏳ có trong code; chưa chạy lại trong đợt này |
| Heartbeat 100/300 ms, stop output, reconnect, phục hồi lỗi | ⏳ có trong code; chưa kiểm chứng trong đợt này |
| Xuất odometry qua UDP | ⏳ có trong code; chưa kiểm chứng |

---

## 12. Hạn chế và rủi ro đã biết

1. **Driver lely được tạo trên sai thread (rủi ro cao).** lely yêu cầu *"The driver MUST be
   instantiated on the thread on which its task are run"*. Ở đây `MbdvAxisDriver` được tạo trên
   main thread, nhưng task của nó chạy trên event-loop thread.
   - Hậu quả: biến đếm `pending` trong fiber executor của lely bị hai thread sửa mà không có khoá.
     Chỉ cần mất một lần cập nhật là trục đó **ngừng nhận lệnh vĩnh viễn**.
   - Đã xảy ra ngày 2026-10-06, khi thử gửi lệnh từ thêm một thread: AX1 giữ lệnh quay, và vẫn tiếp
     tục quay vài phút sau khi chương trình đã nhả CAN.
   - Hướng sửa dự kiến: tạo và huỷ driver trên chính event-loop thread.
2. **Drive không tự dừng khi mất master.**
   - Watchdog `0x2060` (P1-39) đang tắt.
   - Tài liệu chỉ ghi hành động P1-40 nhận giá trị 1–16 (mặc định 1), không có bảng ý nghĩa.
   - EDS không có `0x6007` hay `0x1016` phía drive.

   Vì vậy, nếu master chết thì drive **giữ setpoint cuối**. Việc cần làm:
   - tra ý nghĩa các giá trị P1-40 trong Luna;
   - đặt `drive.watchdog_ms` và `drive.watchdog_action`. Timeout phải dài hơn khoảng trống RPDO dài
     nhất, hiện khoảng 0,4–0,8 s trong lúc kiểm tra staged;
   - nối stop output vào E-STOP của drive (mục 8).
3. **P1-00 không được lưu.** Mỗi lần bật nguồn, drive quay về giá trị lưu trong Luna; S08 sẽ ghi lại.
4. **`--test-velocity` làm robot quay tại chỗ** cho tới khi Ctrl+C.
5. Khi tắt chương trình, lely in `warning: io_can_net_fini() invoked with pending operations`. Dòng
   này vô hại.

---

## 13. Các phát hiện trên phần cứng thật

Những điều dưới đây không có trong tài liệu, hoặc chỉ biết được khi đo trên drive thật. Cột cuối là
cách code đã xử lý.

| # | Phát hiện | Xử lý trong code |
|---|---|---|
| 1 | EDS không công bố bảng tên/mã alarm của `0x200F` | Giải mã đầy đủ `0x603F` và `0x1001` theo chuẩn; `0x200F` chỉ in mã LED `rNN` |
| 2 | `0x2021` trả tốc độ theo kbps (`500`), không phải mã P1-18 | So sánh S08 theo bit/s, nhận cả hai quy ước |
| 3 | `0x2030` có đơn vị 0,1 V (đo được 241 → 24,1 V và 482 → 48,2 V) | Dựa vào bit 4 Statusword để quyết định có nguồn chính, không dựa vào ngưỡng điện áp |
| 4 | lely tìm `slave_N.bin` theo thư mục làm việc hiện tại (CWD), không theo thư mục DCF | S01 `chdir()` vào thư mục DCF, khôi phục khi tắt |
| 5 | RPDO2/RPDO3 mặc định là `0xFE` (chỉ RTR) | `master.yaml` ghi `0xFF`; S09 đọc ngược |
| 6 | `0x6060` bị bỏ qua nếu P1-00 không khớp | S08 tự căn chỉnh P1-00; S10 đọc `0x6061` xác nhận |
| 7 | Tìm file cấu hình chỉ theo tên có thể nạp nhầm file | Ưu tiên ứng viên giữ nguyên thư mục con; khớp theo tên chỉ là phương án cuối |
| 8 | Huỷ đối tượng sai thứ tự gây segfault khi thoát | Thứ tự cố định: dừng loop → huỷ driver → master → I/O → loop/context |
| 9 | `0x2AC0` không phải chỉ báo alarm | In nguyên giá trị kèm chú thích |
| 10 | `0x2070 = 0` là hợp lệ; `SW7 = 0` nghĩa là tốc độ do Luna đặt, không phải 1 Mbps | Mô tả DIP đúng nghĩa; `0x2020`/`0x2021` là nguồn tin cậy |
| 11 | COB-ID ở sub-index 1; ghi vào `:00` bị abort `0x06010002` | Mọi lần ghi ở S09 dùng `:01` |
| 12 | Drive từ chối ghi `0` vào COB-ID (abort `0x06090030`) | Tắt/bật PDO bằng bit 31, giữ nguyên COB-ID |
| 13 | P1-00 phải được xử lý ngay trong S08; "Last OK" phải tính theo từng trục | Ghi P1-00 bên trong S08; `LastSuccessfulStageForAxis()` lọc theo trục |
| 14 | Cần phân biệt "RPDO không tới" với "drive từ chối" | S12 đọc ngược `0x6040` qua SDO. Controlword chỉ gửi qua RPDO, không fallback sang SDO, để lỗi PDO không bị che |
| 15 | `master.dcf` gốc của `dcfgen` không mô tả được đường PDO tới drive | `tools/fix_master_dcf.py`, có tự kiểm chứng (kể cả `AccessType = rw`) |
| 16 | `dcfgen` để mọi PDO ở trạng thái disabled (cờ bit 31 bị đảo) | S09 kiểm tra trước, sửa sau; bật 6 PDO dùng, tắt RPDO4/TPDO4 |
| 17 | OD dùng chung của master làm hai node nhận lệnh và phản hồi của nhau | RPDO dựng tay theo node; phản hồi qua `CanSniffer` |
| 18 | *(2026-10-06)* Dừng từng trục một (cách nhau khoảng 400 ms) làm robot tự xoay 0,13–0,22 rad mỗi lần dừng | Gửi setpoint cho cả hai trục trước, sau đó mới kiểm tra lần lượt |
| 19 | *(2026-10-06)* Lệnh staged không cập nhật `last_target_velocity_`, nên lần gửi lại controlword 20 Hz phát lại setpoint cũ | Lưu setpoint trước khi gửi RPDO3 |
| 20 | *(2026-10-06)* Gửi lệnh từ thêm một thread làm strand của AX1 chết, AX1 quay vài phút | Đã gỡ bỏ; nguyên nhân gốc ở mục 12, rủi ro 1 |
| 21 | *(2026-10-06)* Report tự khoá lồng mutex, nên process treo mỗi khi có giai đoạn FAIL | `Print()` in từ bản sao; không gọi hàm khoá khi đang giữ khoá |
| 22 | *(2026-10-06)* Ở S15, `000F` (RPDO3) và `0007` (RPDO1) gửi sát nhau nên drive giữ `000F`; lệnh `Disable Voltage` khẩn bị hoãn tới sau khi S15 đã FAIL | Chờ giảm tốc xong mới gửi `0007`; lệnh khẩn gửi ngay. S15 PASS cả hai trục |
| 23 | *(2026-10-06)* Event timer TPDO trong DCF (10/10/100 ms) lệch `params.yaml` (5/5/50 ms), nên S09 nạp lại 3 PDO mỗi lần boot | Đồng bộ `master.yaml` về 5/5/50 ms (chờ kiểm chứng sau build) |
| 24 | *(2026-10-06)* Ghi `0x1017` của AX1 thất bại vì SDO đang bận nạp DCF | Ghi heartbeat sau S05 (chờ kiểm chứng sau build) |

Ghi chú thêm:
- `OnConfig()` của driver chỉ bọc callback hoàn tất và vẫn gọi lớp gốc của lely. Bỏ lời gọi đó thì
  slave không bao giờ nhận cấu hình.
- S09 là lớp bảo vệ thứ hai: nó không phụ thuộc nội dung `.bin`, nên vẫn chỉ ra đúng object sai kể
  cả khi YAML sai.

---

## 14. Cấu trúc thư mục

```text
driver_Moons_MBDV/
├── CMakeLists.txt
├── README.md
├── odometry_kinematics_summary.md     # ghi chú động học vi sai và odometry
├── config/
│   ├── CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds
│   ├── master.yaml                    # đầu vào dcfgen lúc BUILD
│   ├── params.yaml                    # cấu hình RUNTIME
│   ├── master.dcf                     # sinh bởi dcfgen + tools/fix_master_dcf.py
│   └── slave_1.bin, slave_2.bin       # sinh bởi dcfgen, nạp vào drive lúc boot
├── docx/                              # tài liệu gốc (EDS, manual) + tài liệu liveness
├── include/mbdv/
│   ├── can_sniffer.hpp                # socket chỉ nghe, giải mã TPDO theo node
│   ├── cia402_defs.hpp                # enum + giải mã trạng thái CiA 402
│   ├── config_path.hpp                # tìm file cấu hình từ mọi thư mục
│   ├── diagnostics.hpp                # Stage, Logger, DiagnosticReport
│   ├── diff_drive_kinematics.hpp      # động học vi sai
│   ├── drive_errors.hpp               # chỉ số OD + giải mã lỗi theo EDS
│   ├── dual_axis_controller.hpp       # điều phối 2 trục, supervisor, vòng 200 Hz
│   ├── mbdv_axis_driver.hpp           # driver một trục (FiberDriver), stage S05..S15
│   ├── odometry_publisher.hpp         # xuất odometry (callback / UDP)
│   └── params.hpp                     # đọc config/params.yaml
├── src/                               # phần cài đặt tương ứng + main.cpp
└── tools/
    ├── fix_master_dcf.py              # sửa + tự kiểm chứng master.dcf
    ├── verify_run.sh                  # chứng minh PDO chạy thật trên bus
    └── make_docx.py                   # sinh docx/MBDV-dual-axis-liveness-and-recovery.docx
```

---

## 15. Tài liệu tham khảo

| Tài liệu | Dùng cho |
|---|---|
| `docx/CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds` (+ `.md`) | Object Dictionary, PDO mặc định, object hãng (`0x200F`, `0x2020`, `0x2021`, `0x2060`, `0x2070`, `0x2A30`…) |
| `docx/MBDV-Hardware-Manual-EN20230926-MOONS.pdf` (+ `.md`) | §4.2.2 DIP, §4.3 nguồn, §4.11 STO, §6 commissioning, §8 tham số (P1-00, P1-39, P1-40…), §9.1 alarm |
| `docx/MBDV-2X-520AC.pdf` | Bản vẽ kích thước |
| `docx/MBDV-dual-axis-liveness-and-recovery.md` / `.docx` | Heartbeat, reconnect, phục hồi lỗi, 200 Hz |
| `odometry_kinematics_summary.md` | Động học vi sai và odometry |
