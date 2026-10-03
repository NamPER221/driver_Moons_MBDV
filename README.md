# Moons' MBDV Dual-Axis CANopen Master (lely-core C++)

Điều khiển đồng thời 2 trục servo **Moons' MBDV-2X-520AC** theo chuẩn **CiA 402 / CANopen**
bằng thư viện **lely-core** (`liblely-coapp`), kèm hệ thống log theo từng **giai đoạn
(stage)** để xác định chính xác lỗi xảy ra ở bước nào.

---

## 0. Nguồn tài liệu đã đọc

| Tài liệu | Nội dung dùng cho dự án |
|---|---|
| `docx/CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds` | Object Dictionary, ánh xạ PDO mặc định của nhà sản xuất, các object chẩn đoán của hãng (`0x200F`, `0x2020`, `0x2021`, `0x2070`, `0x2A30` …) |
| `docx/MBDV-Hardware-Manual-EN20230926-MOONS.pdf` | §4.2.2 DIP switch, §4.8.3 chân CAN, §5.2 mã cảnh báo, §6 commissioning, §8.2/§8.3 tham số (P1-00, P1-17, P1-18, P3-05 …), §9.1 danh mục alarm |
| `docx/MBDV-2X-520AC.pdf` | Bản vẽ kích thước 2D (không có thông tin điều khiển) |

---

## 1. Kiến trúc hệ thống & Địa chỉ hóa 2 trục

Theo MBDV Hardware Manual §4.2.2 (bảng **MBDV-2X-520AC**):

* **Trục 1 (Axis 1)**: `SW1..SW3` đặt Node-ID — mặc định `1`
* **Trục 2 (Axis 2)**: `SW4..SW6` đặt Node-ID — mặc định `2`
* **Baudrate**: `SW7` — `0` = cài đặt qua Luna (mặc định **1 Mbps**), `1` = **500 kbps**
* **Trở kết thúc 120 Ω**: `SW8` — `0` = không có, `1` = có (đặt ở thiết bị cuối bus)

Cả hai trục là 2 Node CANopen độc lập trên cùng một bus CAN vật lý, dùng chung file EDS.

| DIP | Axis 1 (Node 1) | Axis 2 (Node 2) | Vai trò |
|---|---|---|---|
| Node-ID | `SW1=1, SW2=0, SW3=0` | `SW4=0, SW5=1, SW6=0` | Địa chỉ CANopen |
| Baudrate 500 kbps | `SW7=1` | *(dùng chung)* | Tốc độ bus |
| Trở 120 Ω | `SW8=1` (nếu ở cuối bus) | — | Đóng bus |

> **Lưu ý**: `SW1..SW3 = 0,0,0` (hoặc `SW4..SW6 = 0,0,0`) nghĩa là **không dùng DIP switch**,
> Node-ID lấy từ phần mềm Luna và **mặc định là 1**. Đây là nguyên nhân rất hay gặp khiến
> cả 2 trục cùng trả lời ở Node 1 — chương trình sẽ báo lỗi ở **giai đoạn S08**.

---

## 2. Quy trình 4 bước kỹ thuật

### Bước 1 — Import EDS & cấu hình Master

`dcfgen` được tích hợp trong CMake, đọc `config/master.yaml` + file EDS, sinh ra:
`config/master.dcf` (OD của Master), `config/single_axis_500k/master.dcf` và các
`slave_*.bin` (gói SDO cấu hình từng slave).

> **Cảnh báo quan trọng về `dcfgen`** — xác minh bằng `dcfgen -r -v`:
> 0. **`dcf-tools` 2.3.5 ghi DCF bị ĐẢO COB-ID của RPDO và TPDO.** `0x14xx:01` và
>    `0x18xx:01` bị hoán đổi giá trị trong file `master.dcf` — ngay cả khi YAML không
>    khai báo PDO nào. Lệnh SDO vẫn đúng (được tạo từ object trong RAM) nên drive cấu
>    hình đúng, nhưng **lely đọc text DCF** nên sẽ gửi RPDO đi COB-ID của TPDO và
>    lắng nghe TPDO trên COB-ID của RPDO ⇒ **SDO chạy được, PDO chết cả hai chiều**.
>    CMake chạy `tools/fix_dcf_pdo_cobid.py` ngay sau `dcfgen` để sửa và kiểm chứng.
> 1. Nếu YAML không khai báo `transmission:`, `dcfgen` **không** ghi `0x140x:02` /
>    `0x180x:02`. Driver MBDV mặc định đặt RPDO2/RPDO3 là **0xFE**, mà 2 bit thấp của
>    transmission type kiểu CiA 301 là `10b` = **"chỉ RTR"**. RPDO ở chế độ này **âm thầm
>    bỏ qua** khung hình được gửi thường lệ → lệnh vị trí / vận tốc **không bao giờ được thực thi**.
>    → Đã sửa: mọi PDO trong YAML đều khai báo `transmission: 255` (`0xFF` = event driven).
> 2. `dcfgen` phát `0x140x:01` / `0x180x:01` với **bit 31 = 0** ("PDO disabled") ở lần ghi cuối
>    ngay cả khi YAML ghi `enabled: true` → cờ `enabled` bị đảo ngược trong `.bin` sinh ra.
>    → Đã xử lý ở tầng runtime: **giai đoạn S09** tự cấu hình lại và **kiểm chứng đọc ngược**
>      toàn bộ ánh xạ PDO theo đúng thứ tự CiA 301, nên trạng thái cuối cùng trên drive luôn đúng.

### Bước 2 — Quản lý trạng thái NMT

1. Slave gửi **Boot-up** (`COB-ID 0x700 + NodeID`, data `0x00`) → node ở **Pre-Operational**.
2. Master nạp cấu hình PDO qua SDO download (`slave_N.bin`).
3. Master gửi **NMT Start Remote Node** (`CS = 0x01`) → **Operational**.
4. Khi **Operational**, PDO thời gian thực bắt đầu truyền/nhận.

### Bước 3 — Ánh xạ PDO (đã hiệu chỉnh theo EDS)

**RPDO (Master → Drive)**

| PDO | COB-ID | Ánh xạ | Ghi chú |
|---|---|---|---|
| RPDO1 | `0x200 + Node` | `0x6040` Controlword (16b) + `0x6060` Modes of operation (8b) | Transmission `0xFF` |
| RPDO2 | `0x300 + Node` | `0x6040` (16b) + `0x607A` Target position (32b) | **Transmission phải là `0xFF`, không phải `0xFE`** |
| RPDO3 | `0x400 + Node` | `0x6040` (16b) + `0x60FF` Target velocity (32b) | **Transmission phải là `0xFF`, không phải `0xFE`** |

**TPDO (Drive → Master)**

| PDO | COB-ID | Ánh xạ | Event timer |
|---|---|---|---|
| TPDO1 | `0x180 + Node` | `0x6041` Statusword (16b) | 10 ms |
| TPDO2 | `0x280 + Node` | `0x6064` Position actual (32b) + `0x606C` Velocity actual (32b) | 10 ms |
| TPDO3 | `0x380 + Node` | `0x603F` CiA 402 error code (16b) + `0x200F` DSP alarm code (32b) | 100 ms (chẩn đoán lỗi tức thời) |

RPDO4 (`0x500+Node`) và TPDO4 (`0x480+Node`) không dùng → **tắt** ở S09 để giảm tải bus.

### Bước 4 — Chu trình trạng thái CiA 402

| Bước | Controlword | Statusword chờ đợi |
|---|---|---|
| Shutdown | `0x0006` | `Ready to Switch ON` (`0x0021`) |
| Switch ON | `0x0007` | `Switched ON` (`0x0023`) |
| Enable Operation | `0x000F` | `Operation Enabled` (`0x0027`) |
| Chạy vị trí (PP) | `0x001F` (bit4 New setpoint + bit5 Immediate) | Chờ bit 10 `Target reached` |

**Điều kiện tiên quyết thường bị bỏ sót:** `0x6060` (Modes of operation) **chỉ có tác dụng
khi P1-00 khớp**, và P1-00 nằm ở object hãng **`0x2A30`** (không phải `0x6061`):

| P1-00 (`0x2A30`) | Chế độ drive | `0x6060` tương ứng |
|---|---|---|
| `21` *(mặc định nhà máy)* | Position Control | `1` — Profile Position |
| `15` | Velocity Control (8 tốc độ nội bộ) | `3` — Profile Velocity |
| `1` | Torque Control | `4` — Profile Torque |

Nếu ghi `0x6060 = 3` mà `0x2A30` vẫn là `21`, `0x6061` sẽ **không đổi** → chương trình báo lỗi
ở **giai đoạn S10** kèm giá trị `0x2A30` cần sửa. Dùng `--p1-00 15` để tự ghi và lưu.

---

## 3. Hệ thống log theo giai đoạn (Stage)

Mỗi dòng log có định dạng cố định:

```
[  1.234s][S12   ][AX1    ][INFO ] CiA402: S12.3 Enable Operation: CW=0x000F -> wait for Operation Enabled
   thời gian      giai đoạn   trục    mức      nội dung
```

### 3.1. Danh sách 16 giai đoạn

| Mã | Tên | Ý nghĩa — giai đoạn này chứng minh điều gì |
|---|---|---|
| `S01` | `CONFIG` | Đọc được file DCF, tham số hợp lệ |
| `S02` | `CAN_LINK` | Mở được SocketCAN controller + channel |
| `S03` | `MASTER_LOAD` | `AsyncMaster` dựng được, tạo được driver 2 trục |
| `S04` | `EVENT_LOOP` | Thread event loop chạy, `master.Reset()` được đăng |
| `S05` | `BOOTUP` | Nhận được bản tin Boot-up (`0x700+Node = 0x00`) |
| `S06` | `PREOP` | SDO upload `0x6041` trả lời → node đã rời Boot-up |
| `S07` | `NMT_START` | NMT Start → **OPERATIONAL** |
| `S08` | `IDENTITY` | Vendor/Node-ID/Baudrate/P1-00 khớp mong đợi |
| `S09` | `PDO_VERIFY` | Ánh xạ + transmission type PDO đúng và **đọc ngược xác nhận** |
| `S10` | `MODE_OF_OP` | Ghi `0x6060` và `0x6061` xác nhận khớp |
| `S11` | `FAULT_RESET` | Xóa lỗi đang kẹt (nếu có) |
| `S12` | `SERVO_ON` | Chuỗi Shutdown → SwitchOn → EnableOp thành công |
| `S13` | `MOTION_CMD` | Ghi được Target position / Target velocity |
| `S14` | `MOTION_TRACK` | `Target reached` bật, hoặc phản hồi bám theo lệnh |
| `S15` | `SERVO_OFF` | Tắt servo an toàn |
| `S16` | `SHUTDOWN` | Giải phóng tài nguyên CAN |

### 3.2. Báo cáo chẩn đoán khi lỗi

Khi chương trình dừng, nó in ra **đúng giai đoạn hỏng** cùng nguyên nhân và cách khắc phục:

```
==============================================================================
  DIAGNOSIS - FIRST FAILING STAGE
==============================================================================
  Stage      : S08  IDENTITY
  Purpose    : verify identity + bus params vs expectation
  Axis       : AX2
  Elapsed    : 41 ms
  Reason     : node-ID mismatch: the master addresses node 2 but the drive reports 0x2020 = 1 (1)
  Hint       : MBDV-2X-520AC DIP switches: axis 1 node-ID = SW1..SW3, axis 2 node-ID = SW4..SW6.
              A reported value of 1 while you expected 2 means both axes are configured with all
              switches OFF, which selects the Luna software address (default 1).
  Last OK    : S07 NMT_START
  Conclusion : everything up to and including S07 worked; the fault is isolated to S08 IDENTITY.
==============================================================================

==============================================================================
  MBDV DUAL-AXIS BRING-UP : FAILURE
==============================================================================
  Stage  Name            Axis   Result  Elapsed   Reason
  ----------------------------------------------------------------------------
  S01    CONFIG          -      PASS    0ms       DCF 'config/master.dcf' is readable
  S02    CAN_LINK        -      PASS    1ms       SocketCAN 'can0' open
  ...
  S08    IDENTITY        AX1    PASS    38ms      vendor 0x000002D9, node-ID 1, 500 kbps, P1-00=21
  S08    IDENTITY        AX2    FAIL    41ms      node-ID mismatch: the master addresses node 2 but...
  ----------------------------------------------------------------------------
  >>> FIRST FAILING STAGE : S08  IDENTITY   (axis AX2)
  >>> REASON              : node-ID mismatch: ...
  >>> HINT                : MBDV-2X-520AC DIP switches: ...
  >>> LAST GOOD STAGE     : S07 NMT_START
  >>> VERDICT             : bring-up is blocked at S08 IDENTITY
==============================================================================
```

Mã thoát: `0` = mọi giai đoạn PASS, `1` = có giai đoạn FAIL.

### 3.3. Giải mã chẩn đoán lấy từ EDS

Khi có lỗi, chương trình in ra "drive object dictionary snapshot":

| Object | Nội dung | Nguồn |
|---|---|---|
| `0x6041` | Statusword + giải mã toàn bộ bit + tên trạng thái CiA 402 | CiA 402 |
| `0x603F` | Error code, giải mã theo bảng chuẩn CiA 402 | CiA 402 |
| `0x1001` | Error register, giải mã từng bit | CiA 301 |
| `0x200F` | **Mã alarm chính thức** — byte thấp **chính là số 2 chữ LED hiển thị** (đã xác minh: `0x200F = …20` ⇔ LED `"20"`) | EDS + Manual §6.2 + đo thực tế |
| `0x2020` / `0x2021` | Node-ID thực tế / tốc độ bus thực tế (theo **kbps**) | EDS + đo thực tế |
| `0x2070` | Bitmap DIP switch thật (SW1..SW8) | EDS + Manual §4.2.2 |
| `0x200B` | DSP status code | EDS |
| `0x2AC0` | Mã phụ (sub-alarm). **Đính chính: tôi từng nói nó chỉ là bitmask không liên quan alarm — sai.** Nó đổi theo alarm: `0x04000000` khi khoẻ → `0x02010000` khi lỗi | EDS + đo thực tế |
| `0x2030` | Điện áp bus DC — hiển thị **cả hai cách hiểu** vì EDS không ghi đơn vị; đối chiếu với dải `24–60 VDC` của Manual §4.3 | EDS + đo thực tế |
| `0x6041` bit 4 | `Voltage_enabled` — tín hiệu **chuẩn CiA 402** cho biết nguồn chính có hay không. Đây mới là nguồn quyết định, không phải `0x2030` | CiA 402 |
| `0x6078` | Dòng thực tế | CiA 402 |
| `0x60F4` | Sai số vị trí (following error) | CiA 402 |

`0x603F` và `0x1001` là hai trường **chuẩn, có định nghĩa chính thức** nên được giải mã
đầy đủ. Riêng `0x200F` trả về mã nội bộ của DSP và **EDS không công bố bảng ánh xạ
mã ↔ tên**; mã này hiển thị trên LED 2 chữ số của drive với tiền tố `r` (Manual §6.2 có
nhắc `r09` cho lỗi cáp encoder). Vì vậy chương trình in ra **mã dạng `rNN`** để đối chiếu
với LED và với danh mục alarm ở Manual §9.1 — không tự suy đoán ánh xạ không có căn cứ.

---

## 4. Cấu trúc mã nguồn

```text
driver_Moons_MBDV/
├── CMakeLists.txt
├── tools/
│   └── fix_dcf_pdo_cobid.py             # Vá lỗi đảo COB-ID RPDO↔TPDO của dcf-tools
├── config/
│   ├── CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds
│   ├── master.yaml                     # Cấu hình PDO 2 trục (có transmission type)
│   ├── master.dcf                      # sinh bởi dcfgen
│   ├── slave_1.bin / slave_2.bin       # sinh bởi dcfgen
│   └── single_axis_500k/               # Cùng cấu hình nhưng chỉ 1 trục
├── docx/                               # Tài liệu gốc
├── include/mbdv/
│   ├── cia402_defs.hpp                 # Enum + giải mã trạng thái CiA 402
│   ├── config_path.hpp                 # Phân giải đường dẫn DCF (chạy được mọi nơi)
│   ├── diagnostics.hpp                 # Stage, Logger, DiagnosticReport
│   ├── drive_errors.hpp                # Chỉ số OD + giải mã lỗi theo EDS
│   ├── diff_drive_kinematics.hpp       # Động học vi sai phân
│   ├── mbdv_axis_driver.hpp            # Driver 1 trục + các giai đoạn S05..S15
│   ├── dual_axis_controller.hpp        # Điều phối 2 trục
│   └── single_axis_controller.hpp      # Điều khiển 1 trục
├── src/
│   ├── config_path.cpp
│   ├── diagnostics.cpp
│   ├── drive_errors.cpp
│   ├── mbdv_axis_driver.cpp
│   ├── dual_axis_controller.cpp
│   ├── single_axis_controller.cpp
│   ├── diff_drive_kinematics.cpp
│   ├── main.cpp
│   └── main_single_axis.cpp
└── build/
```

---

## 5. Biên dịch & chạy

### 5.1. Biên dịch

```bash
cd /home/namnc/driver_motor/driver_Moons_MBDV
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### 5.2. Thiết lập SocketCAN

```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000      # khớp DIP SW7 = 1
sudo ip link set can0 up
```

Nếu `CanController` báo *Operation not permitted*, không cần sửa `tx_queue_len` — chương trình
đã tự đọc `tx_queue_len` hiện có của kernel và thử lại với giá trị `1`.

**Chạy được từ mọi thư mục.** Đường dẫn DCF mặc định (`config/master.dcf`) được phân giải
theo thứ tự: đúng như gõ → so với thư mục gốc dự án (được `CMake` nạp sẵn vào binary) →
so với thư mục cha của executable (layout `build/`) → cạnh executable. Nên chạy được cả:

```bash
cd /home/namnc/driver_motor/driver_Moons_MBDV && sudo ./build/mbdv_dual_axis_node --selftest
cd /home/namnc/driver_motor/driver_Moons_MBDV/build && sudo ./mbdv_dual_axis_node --selftest
```

Nếu DCF thật sự không tồn tại, S01 liệt kê **toàn bộ đường dẫn đã thử** kèm lệnh `dcfgen` cần
chạy để sinh lại.

> DCF lưu đường dẫn tới `slave_N.bin` **tương đối so với thư mục của chính DCF**, và lely tự
> resolve theo thư mục đó — nên chỉ cần tìm đúng file DCF là đủ.

### 5.3. Quy trình chạy khuyến nghị

**Bước 1 — Chẩn đoán bus (chưa cấp động cơ), đây là bước an toàn nhất:**

```bash
sudo ./build/mbdv_dual_axis_node --selftest --diag
```

Chạy S01→S11 rồi thoát. Nếu `SUCCESS` thì bus, định danh, PDO đều tốt.
Dùng `--log-file mbdv.log --log-level debug` để lưu lại toàn bộ dấu vết.

**Bước 2 — Kiểm tra vị trí (PP mode), tháo liên kết cơ khí trước:**

```bash
sudo ./build/mbdv_dual_axis_node --test-motion --step 10000 --p1-00 21
```

**Bước 3 — Kiểm tra vận tốc (PV mode):**

```bash
sudo ./build/mbdv_dual_axis_node --test-velocity --velocity 5000
```

**Chạy song song 2 trục cùng chiều / động học vi sai phân / giám sát liên tục:**

```bash
sudo ./build/mbdv_dual_axis_node --test-motion-parallel
sudo ./build/mbdv_dual_axis_node --test-kinematics
sudo ./build/mbdv_dual_axis_node --monitor
```

**Phiên bản 1 trục (Node 1, 500 kbps):**

```bash
sudo ./build/mbdv_single_axis_node --selftest --diag \
     -d config/single_axis_500k/master.dcf -1 1
sudo ./build/mbdv_single_axis_node --test-motion -1 1
```

### 5.4. Bảng tuỳ chọn CLI

| Tuỳ chọn | Ý nghĩa |
|---|---|
| `-i, --interface <dev>` | Giao diện CAN (mặc định `can0`) |
| `-d, --dcf <path>` | File `master.dcf` |
| `-b, --bin <path>` | File `.bin` tải xuống slave (tùy chọn) |
| `-1, --axis1 <id>` / `-2, --axis2 <id>` | Node-ID từng trục |
| `--baud <kbps>` | Tốc độ bus **mong đợi** để đối chiếu với `0x2021` ở S08 (`1000\|800\|500\|250\|125\|50\|20\|12`) |
| `--control-mode <n>` | Giá trị P1-00 **mong đợi** trong `0x2A30` (`1\|15\|21`) |
| `--p1-00 <n>` | **Ghi thật** giá trị này vào `0x2A30` + lưu `0x1010:01 = 1` trước S10 (dùng `--p1-00 21` khi AX2 đang để `15`) |
| `--no-pdo-program` | Chỉ kiểm chứng PDO, không ghi (chế độ chẩn đoán) |
| `--watchdog <ms\|off>` | Watchdog giao tiếp `0x2060` (Manual P1-39). `off` để tắt. **Đo thực tế trên drive này: watchdog đã tắt sẵn (`enable=0`, `timeout=0 ms`) nên KHÔNG phải nguồn gốc** của EMERGENCY `COMMUNICATION(b4)`; tuỳ chọn này chỉ để tiện khi đổi firmware |
| `--sdo-setpoints` | Gửi target position/velocity qua **SDO** thay vì RPDO. **Bắt buộc** với drive bỏ qua RPDO (mục 16). Đo được: AX2 chạy tới **-4443 / -5000 counts** |
| `--no-sdo-fallback` | Không thử lại Controlword qua SDO khi RPDO không hiệu quả. Dùng để kiểm tra riêng đường RPDO |
| `--step <counts>` | Biên độ kiểm tra vị trí (mặc định 10000) |
| `--velocity <counts/s>` | Setpoint kiểm tra vận tốc (mặc định 5000) |
| `--boot-timeout <ms>` | Timeout Boot-up / SDO mỗi node (mặc định 3000) |
| `--servo-timeout <ms>` | Timeout mỗi bước chuyển trạng thái CiA 402 (mặc định 2000) |
| `--log-level <lvl>` | `trace\|debug\|info\|warn\|error\|fatal` |
| `--log-file <path>` | Ghi thêm log ra file |
| `--no-color` | Tắt mã màu ANSI |
| `--diag` | In toàn bộ object chẩn đoán của cả 2 trục trước khi thoát |

---

## 6. Bảng tra cứu nhanh: lỗi ở giai đoạn nào → kiểm tra gì

| Giai đoạn FAIL | Triệu chứng điển hình | Kiểm tra |
|---|---|---|
| `S01 CONFIG` | `cannot find DCF file` (kèm danh sách đường dẫn đã thử) | Chạy lại `dcfgen`, hoặc truyền `-d /đường/dẫn/tuyệt/đối` |
| `S02 CAN_LINK` | `No such device` / `Operation not permitted` | `ip link show can0`; `ip link set can0 up`; đúng `bitrate`; chạy bằng root |
| `S03 MASTER_LOAD` | `AsyncMaster construction failed` | DCF lỗi cú pháp; đường dẫn EDS trong `master.yaml` |
| `S05 BOOTUP` | `SDO abort code 08000020` / `slave_1.bin: No such file` | **Gần như luôn là lỗi đường dẫn `.bin`** — xem dòng S01 "working directory changed to". Nếu không có Boot-up: nguồn 24 VDC AUX, dây `CAN_H`/`CAN_L`/GND, LED có hiện mã alarm thay vì số node |
| `S06 PREOP` | SDO upload timeout | Tốc độ bus; trở 120 Ω (`SW8=1` ở thiết bị cuối); không có master thứ hai trên bus |
| `S07 NMT_START` | Không sang OPERATIONAL | Node đã tới PRE-OP chưa; có master khác gửi NMT không |
| `S08 IDENTITY` | `vendor ID` / `node-ID mismatch` / `bit-rate mismatch` / `drive control mode mismatch` | `SW1..SW3`, `SW4..SW6`; `SW7`; đối chiếu `0x2020`, `0x2021`, `0x2070`. Với `control mode mismatch` dùng `--p1-00 21` (hoặc `--p1-00 15` cho PV). `--selftest` **bỏ qua** kiểm tra này vì nó kiểm tra bus chứ không kiểm tra chế độ chạy. Cảnh báo nguồn chỉ xuất hiện khi **bit 4 của Statusword = 0**, tức drive thật sự không thấy điện áp chính ở `V+/V−` (`24–60 VDC`, Manual §4.3) |
| `S09 PDO_VERIFY` | Liệt kê từng sai lệch `0x14xx/0x16xx/0x18xx/0x1Axx` | Ánh xạ PDO; **transmission type không được là `0xFE`**; event timer TPDO1. `0x06010002` = ghi sai **sub-index** (COB-ID là `:01`, không phải `:00`). `0x06090030` = đang cố ghi giá trị `0` vào COB-ID, điều mà drive này từ chối |
| `S10 MODE_OF_OP` | `0x6061` không đổi sau khi ghi `0x6060` | Ghi P1-00 khớp vào `0x2A30` (PP=21, PV=15, TQ=1) — dùng `--p1-00` |
| `S11 FAULT_RESET` | Không xóa được lỗi | Đọc LED + `0x603F` + `0x200F`; lỗi không reset được (điện áp nội bộ, lỗi encoder) cần kiểm tra dây và power-cycle |
| `S12 SERVO_ON` | Kẹt ở `Switch On Disabled` hoặc `Fault` | Log in ra `controlword path = ...` và `digital inputs`:<br>• `SDO fallback (RPDO did not work)` ⇒ lỗi đường truyền PDO / cấu hình `0x14xx`–`0x1Axx`, **không phải lỗi drive**<br>• `none` ⇒ drive nhận lệnh nhưng từ chối chuyển trạng thái → kiểm tra STO (§4.11), input `0x60FD`/`0x2A20`, `P1-02`<br>• Cả hai đều `Pass` nhưng Statusword không đổi ⇒ kiểm tra main power `24–60 VDC` ở `V+/V−`, encoder, `0x603F`/`0x200F` |
| `S13 MOTION_CMD` | Lệnh không được nhận | Đúng chế độ S10; RPDO2/RPDO3 có `0x6040` + `0x607A`/`0x60FF` và transmission event-driven |
| `S14 MOTION_TRACK` | Không có `Target reached` / vận tốc không bám | Đã thử tải không tải chưa; chạm giới hạn cơ khí; `P3-04` quá nhỏ; `P1-06` quá thấp; encoder không có phản hồi |
| `S15 SERVO_OFF` | Không tắt được servo | Lỗi đang kẹt, hoặc node đã rời OPERATIONAL |

---

## 7. Những điều đã xác minh trên phần cứng thật

Dưới đây là những gì đã đo được trên một cặp MBDV-2X-520AC thật, và cách mã nguồn đã thích ứng.
Các mục này **không** có trong tài liệu, chỉ có trong EDS/Manual hoặc phải đo mới biết.

| # | Phát hiện | Bằng chứng | Xử lý trong mã |
|---|---|---|---|
| 1 | **EDS không công bố tên/mã ánh xạ alarm của drive** | Cột "Display content" ở Manual §9.1 được vẽ bằng vector, trích xuất text ra rỗng | `0x603F` (chuẩn CiA 402) và `0x1001` (chuẩn CiA 301) được giải mã đầy đủ. `0x200F` chỉ in ra **mã dạng LED `rNN`** để đối chiếu với LED và danh mục §9.1 — **không** bịa ánh xạ |
| 2 | **`0x2021` trả về tốc độ theo *kbps*, không phải mã P1-18 CB** | Drive 500 kbps đọc được `500`, trong khi mã P1-18 CB sẽ là `2` | `DecodeBitRateObject()` nhận diện cả hai quy ước; so khớp S08 dùng đơn vị bit/s |
| 3 | **`0x2030` đọc `241`; đơn vị là 0,1 V và 24,1 V là hợp lệ** | `241 × 0,1 = 24,1 V`. **Manual §4.3: nguồn chính là `24 ~ 60 VDC` ở `V+/V−` (DC, không phải AC)**, nên ~24 V nằm trong đặc tả. Statusword có bit 4 `Voltage_enabled = 1` (chuẩn CiA 402 = "main voltage present") | **Dùng bit 4 của Statusword làm tín hiệu quyết định**, không đoán ngưỡng từ `0x2030`. Một bản sửa trước đây gọi mọi giá trị < 25 V là "chưa có nguồn" — **đã sai**, vì 24 V là hợp lệ. Chỉ cảnh báo khi bit 4 = 0, hoặc khi `0x2030` lệch khỏi 24–60 VDC |
| 4 | **`slave_N.bin` trong DCF được phân giải theo CWD của tiến trình, không theo thư mục DCF** | `co_sub_get_download_file()` (lely `src/co/obj.c`) trả về đúng tên file thô trong DCF, không ghép thư mục nào | S01 `chdir()` vào thư mục DCF (và khôi phục lại khi shutdown). Nếu không, SDO download abort với `slave_1.bin: No such file or directory` + abort code `08000020`, biểu hiện thành lỗi S05 **giả** |
| 5 | **RPDO2/RPDO3 mặc định `0xFE` = "chỉ RTR"** | EDS `0x1401sub2` / `0x1402sub2` mặc định `0xFE`; 2 bit thấp kiểu CiA 301 là `10b` = RTR only | YAML khai báo `transmission: 255`; S09 ghi và **đọc ngược kiểm chứng** |
| 6 | **`0x6060` bị bỏ qua nếu P1-00 không khớp** | P1-00 nằm ở object hãng `0x2A30`, mặc định `21` | S10 đọc `0x6061` để xác nhận, và nêu rõ giá trị `0x2A30` cần sửa |
| 7 | **Sảy phân giải đường dẫn cấu hình có thể nạp nhầm file** | Bản đầu tiên khớp theo *tên file*, khiến bản single-axis nạp nhầm DCF 2 trục mà **không báo lỗi** | `ResolveConfigPath()` ưu tiên các ứng viên **giữ nguyên thư mục con**; chỉ khớp theo tên ở phương án cuối |
| 8 | **Thứ tự hủy đối tượng sai gây segfault khi thoát** | `FiberDriver` destructor dùng fiber executor + `master`; bản đầu hủy `loop_` trước driver | Thứ tự cố định: dừng loop → hủy driver → hủy master → hủy I/O → hủy loop/context |
| 9 | **`0x2AC0` KHÔNG phải chỉ báo alarm** | Drive khoẻ (`0x200F=0`, `0x1001=0`, `0x603F=0`, không có bit FAULT) vẫn trả `0x2AC0 = 0x04000000`. Giá trị này là **bitmask chọn lọc**, không phải mã lỗi | In nguyên giá trị kèm giải thích; cảnh báo rõ **không** dùng làm chỉ báo lỗi. Nguồn alarm đúng là `0x200F` + `0x1001` |
| 10 | **`0x2070 = 0` là hợp lệ, và `SW7=0` KHÔNG có nghĩa là 1 Mbps** | Cả 2 node đọc `0x00000000`, tức toàn bộ DIP OFF. Node-ID vẫn là 1 và 2 (đọc đúng từ `0x2020`) ⇒ chúng được đặt bằng **Luna software**. Manual §4.2.2: `SW7=0` ⇒ "tốc độ lấy từ Luna software (mặc định 1 Mbps)", **không phải** "đang chạy 1 Mbps" | `DescribeDipSwitch()` diễn đạt đúng nghĩa ("SW7=OFF → tốc độ lấy từ Luna software"). `0x2021` mới là giá trị đo thực và là thứ được kiểm tra |
| 11 | **Sub-index của COB-ID là 1, không phải 0** | Viết `0x1401:00` trả SDO abort `0x06010002` ("Attempt to write a read only object") vì `:00` là "highest sub-index supported" (read-only) | Toàn bộ ghi trong S09 dùng `ObjRef(0x1401, 1)`. `Hex()` không còn tiền tố `0x` nên log không còn hiện `0x0x1401` |
| 12 | **Drive từ chối ghi giá trị `0` vào COB-ID** | `0x1400:01 = 0` được chấp nhận nhưng `0x1401:01 = 0` bị từ chối: SDO abort `0x06090030` ("Invalid value for parameter") | S09 **không bao giờ ghi 0**. Tắt PDO bằng cách ghi COB-ID với bit 31 = 0 (giữ nguyên giá trị COB-ID), bật bằng bit 31 = 1 |
| 13 | **`--p1-00` phải được áp dụng *bên trong* S08** | Nếu ghi `0x2A30` sau khi bring-up thì S08 đã fail trước đó và không bao giờ tới được bước ghi. Cùng lý do: `LAST GOOD STAGE` phải tính **theo trục đang lỗi** — nếu lấy max toàn cục thì AX1 đạt S11 sẽ làm báo cáo nói AX2 "đã tới S11", gây hiểu nhầm | `write_control_mode` nằm trong `BringUpOptions` và được S08 dùng; `LastSuccessfulStageForAxis()` lọc theo tag trục |
| 14 | **S12 phải phân biệt "RPDO không tới" với "drive từ chối"** | `telemetry frames` đứng yên (112 → 112) suốt 2 giây chờ S12, trong khi SDO vẫn trả lời bình thường. Gửi cùng một Controlword qua **SDO** phân biệt ngay hai khả năng | `ApplyControlword()` thử RPDO trước (nửa ngân sách thời gian), rồi **fallback sang SDO**, và ghi lại đường đi thành công vào `GetControlwordPath()`. Nếu SDO mà được còn RPDO không ⇒ lỗi nằm ở đường truyền PDO; nếu cả hai đều không ⇒ lỗi nằm bên trong drive |
| 15 | **`dcf-tools` ghi DCF bị đảo COB-ID RPDO↔TPDO — nhưng SỬA LẠI ĐÃ HỎNG** | EDS đúng: `1400sub1=$NODEID+0x200`, `1800sub1=$NODEID+0x180`. DCF sinh ra bị đảo thành `1400sub1=0x181`, `1800sub1=0x201`. Tuy nhiên khi sửa lại thì tải concise-DCF **thất bại** (`error 74`), `0x2A30` đọc sai, và `0x1600:00` ghi bị từ chối (`0x06010000`) ⇒ **lely cần đúng giá trị như dcf-tools ghi**. Đã **gỡ bản vá**; đây chỉ là ghi nhận, không phải nguyên nhân | Không có hành động — giữ nguyên DCF do `dcfgen` sinh ra |
| 16 | **KẾT LUẬN: firmware drive BỎ QUA toàn bộ khung RPDO** | Chứng minh theo 3 cách độc lập:<br>① `candump` bắt được master gửi đúng `0x301#0F0088130000` (CW=`0x000F`, Target position=5000)<br>② Drive tự báo cấu hình RPDO1 **hoàn toàn đúng**: `0x1400:01 COB-ID=0201 valid=1`, `0x1400:02=FF (event)`, `0x1600:00=2 → 60400010 60600008`<br>③ Bơm trực tiếp 61 khung `0x201#060000` bằng `cansend`, **không qua master**: Statusword `0x0250` **không đổi một lần** trong 111 khung TPDO1<br>→ Cùng giá trị Controlword ghi qua **SDO** thì chuyển trạng thái **mọi lần**. Lỗi nằm ở firmware drive, không phải master |
| 17 | **`dcfgen` để lại mọi PDO ở trạng thái *disabled*** | Xác minh bằng `dcfgen -r -v`: lần ghi cuối tới `0x140x:01` / `0x180x:01` là `01 03 00 00` (bit 31 **= 0**) ⇒ PDO bị tắt. Cờ `enabled: true` bị **đảo ngược** | S09 **kiểm tra trước, sửa sau**: (A) đọc ngược ánh xạ; (B) chỉ sửa PDO thực sự sai; (C) **bật lại** 6 PDO đang dùng và tắt RPDO4/TPDO4 (TPDO4 bị bật do cờ đảo) |

---

## 8. Vài lưu ý kỹ thuật quan trọng

1. **`OnConfig()` không được bỏ qua.** Bản ghi đè của lely trên `BasicDriver::OnConfig()`
   mới là nơi khởi động việc tải SDO theo `slave_N.bin`. Driver chỉ **bọc lại** callback hoàn
   tất để quan sát kết quả, vẫn gọi xuống lớp gốc. Bỏ qua lời gọi này sẽ khiến slave không
   bao giờ nhận cấu hình.
2. **S09 là lớp bảo vệ thứ hai, không phải lớp đầu.** Nó không phụ thuộc vào nội dung `.bin`
   do `dcfgen` sinh ra, nên kể cả khi YAML sai thì vẫn chẩn đoán đúng và nêu đích danh
   object/sub-index sai.
3. **Chuỗi cấu hình lại PDO theo CiA 301** là bắt buộc: tắt PDO (xoá bit 31 của
   `0x140x:01`/`0x180x:01`) → về 0 số phần tử ánh xạ → ghi từng phần tử → đặt transmission
   type → đặt event timer → bật lại PDO. Làm sai thứ tự này sẽ bị drive từ chối.
4. **`0x2070` cho biết trạng thái DIP thật.** Chương trình luôn in ra (kể cả khi mọi thứ tốt)
   để đối chiếu nhanh với jumper thực tế.
5. **Lỗi `SDO abort code 08000020` ở S05 gần như luôn là lỗi đường dẫn `.bin`**, không phải
   lỗi DCF/EDS. Kiểm tra dòng S01 ghi "working directory changed to ...".
6. **Tương thích ngược:** các hàm không theo giai đoạn cũ (`EnableServo()`,
   `SetTargetPosition()`, `PrintTelemetry()`…) vẫn còn để các ứng dụng khác dùng được.