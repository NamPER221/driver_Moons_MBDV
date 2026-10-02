# Moons' MBDV Dual-Axis CANopen Master (lely-core C++)

Dự án điều khiển đồng thời 2 trục servo **Moons' MBDV Dual-Axis (MBDV-2X-520AC)** chuẩn **CiA 402 / CANopen** sử dụng thư viện **lely-core** (`liblely-coapp`).

---

## 1. Kiến trúc hệ thống & Địa chỉ hóa 2 trục (Dual Axes)

Theo tài liệu phần cứng Moons' MBDV (*MBDV Hardware Manual - Section 4.2.2*):
- **Trục 1 (Axis 1)**: Địa chỉ Node ID thiết lập qua DIP switch `SW1..SW3` (mặc định Node ID = `1`).
- **Trục 2 (Axis 2)**: Địa chỉ Node ID thiết lập qua DIP switch `SW4..SW6` (mặc định Node ID = `2`).
- Cả hai trục dùng chung file mô tả thiết bị `CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds`, hoạt động như 2 Node CANopen độc lập trên cùng một đường bus CAN vật lý.

---

## 2. Quy trình triển khai 4 yêu cầu kỹ thuật

### Bước 1: Import File EDS & Cấu hình Master
- **Trên Linux với thư viện `lely-core`**:
  Công cụ `dcfgen` được tích hợp tự động qua CMake để đọc file `config/CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds` và file cấu hình mạng `config/master.yaml`, biên dịch ra:
  - `master.dcf`: Object Dictionary của CANopen Master.
  - `slave_1.bin`: Gói lệnh SDO concise cấu hình PDO và timer cho Trục 1 (Node 1).
  - `slave_2.bin`: Gói lệnh SDO concise cấu hình PDO và timer cho Trục 2 (Node 2).
- **Trên phần mềm Master công nghiệp (TwinCAT, CODESYS, TIA Portal)**:
  - *TwinCAT 3*: Click phải `I/O -> Devices -> Add New Item -> CANopen Master`, sau đó chuột phải master chọn `Import EDS file`, chọn `CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds`, thêm 2 box lần lượt đặt Node ID = 1 và Node ID = 2.
  - *CODESYS*: Menu `Tools -> Device Repository -> Install`, chọn file `.eds`. Sau đó click chuột phải `CANopen_Manager -> Add Device`, thêm 2 drive cho Trục 1 và Trục 2.
  - *TIA Portal*: Sử dụng qua CANopen Gateway (như Anybus, Helmholz), import file GSD/GSDML hoặc EDS tương ứng.

### Bước 2: Quản lý trạng thái NMT (Network Management)
- Master tự động quản lý chu trình Boot-up của slave:
  1. Slave gửi bản tin **Boot-up** (`COB-ID 0x700 + NodeID`, Data `0x00`).
  2. Slave vào trạng thái **Pre-Operational** (`0x7F`).
  3. Master tự động nạp cấu hình PDO và Heartbeat qua SDO download (`slave_1.bin`, `slave_2.bin`).
  4. Master gửi lệnh NMT **Start Remote Node** (`CS = 0x01` trên `COB-ID 0x000`) để đưa slave vào trạng thái **Operational** (`0x05`).
- Khi ở trạng thái **Operational**, các bản tin PDO thời gian thực bắt đầu được truyền/nhận.

### Bước 3: Ánh xạ PDO (PDO Mapping)
Cấu hình ánh xạ được thiết lập chính xác trong `config/master.yaml`:
* **RPDO (Master Send -> Drive Receive - Thời gian thực)**:
  * **RPDO1** (`COB-ID 0x200 + NodeID`):
    * `0x6040:00` (**Controlword**, 16-bit): Lệnh điều khiển trạng thái.
    * `0x6060:00` (**Mode of Operation**, 8-bit): Chế độ chạy CiA 402 (1 = PP, 3 = PV, 4 = TQ, 6 = HM).
  * **RPDO2** (`COB-ID 0x300 + NodeID`):
    * `0x6040:00` (**Controlword**, 16-bit).
    * `0x607A:00` (**Target Position**, 32-bit): Vị trí đặt thời gian thực.
  * **RPDO3** (`COB-ID 0x400 + NodeID`):
    * `0x6040:00` (**Controlword**, 16-bit).
    * `0x60FF:00` (**Target Velocity**, 32-bit): Tốc độ đặt thời gian thực.

* **TPDO (Drive Send -> Master Receive - Phản hồi thời gian thực)**:
  * **TPDO1** (`COB-ID 0x180 + NodeID`, Event Timer: 10 ms):
    * `0x6041:00` (**Statusword**, 16-bit): Đọc trạng thái phản hồi drive.
  * **TPDO2** (`COB-ID 0x280 + NodeID`, Event Timer: 10 ms):
    * `0x6064:00` (**Position Actual Value**, 32-bit): Vị trí thực tế động cơ.
    * `0x606C:00` (**Velocity Actual Value**, 32-bit): Tốc độ thực tế động cơ.

### Bước 4: Điều khiển chu trình trạng thái (CiA 402 State Machine)
Chuỗi điều khiển Servo ON cho từng trục:
1. **Kiểm tra Fault**: Nếu Statusword báo Fault (`0x0008`), gửi lệnh `0x0080` (Fault Reset) để xóa lỗi.
2. **Shutdown**: Gửi Controlword `0x0006`, đợi Statusword chuyển sang `Ready to Switch ON` (`0x0021`).
3. **Switch ON**: Gửi Controlword `0x0007`, đợi Statusword chuyển sang `Switched ON` (`0x0023`).
4. **Enable Operation**: Gửi Controlword `0x000F`, đợi Statusword chuyển sang `Operation Enabled` (`0x0027`).
5. **Kích hoạt chạy vị trí (Profile Position)**:
   - Ghi giá trị vị trí vào `0x607A`.
   - Gửi Controlword `0x001F` (Bit 4 `New Setpoint` = 1, Bit 5 `Immediate` = 1).
   - Động cơ bắt đầu chuyển động; khi hoàn thành bit 10 `Target Reached` trong Statusword sẽ bật lên 1.

---

## 3. Cấu trúc mã nguồn C++

```text
driver_Moons_MBDV/
├── CMakeLists.txt                      # Cấu hình biên dịch CMake & dcfgen
├── config/
│   ├── CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds
│   ├── master.yaml                     # File cấu hình PDO, baudrate, node IDs
│   ├── master.dcf                      # Sinh tự động bởi dcfgen
│   ├── slave_1.bin                     # Sinh tự động bởi dcfgen
│   └── slave_2.bin                     # Sinh tự động bởi dcfgen
├── include/
│   └── mbdv/
│       ├── cia402_defs.hpp             # Định nghĩa CiA 402 enums, bitmasks, decoders
│       ├── mbdv_axis_driver.hpp        # Driver điều khiển từng trục (FiberDriver)
│       └── dual_axis_controller.hpp    # Quản lý Master và điều phối 2 trục đồng thời
├── src/
│   ├── mbdv_axis_driver.cpp            # Xử lý chu trình NMT, Servo ON, RPDO/TPDO
│   ├── dual_axis_controller.cpp        # Khởi tạo SocketCAN channel, vòng lặp Loop
│   └── main.cpp                        # Ứng dụng CLI điều khiển và giám sát telemetry
└── build/                              # Thư mục build
```

---

## 4. Hướng dẫn biên dịch & Thực thi

### 4.1 Biên dịch
```bash
cd /home/namnc/driver_motor/driver_Moons_MBDV
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### 4.2 Thiết lập SocketCAN interface
Ví dụ thiết lập card CAN thật với tốc độ 1 Mbps (khớp với `baudrate: 1000` trong `master.yaml`):
```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 1000000
sudo ip link set can0 up
```
*(Nếu thử nghiệm mô phỏng nội bộ, có thể dùng `vcan0`: `sudo modprobe vcan && sudo ip link add dev vcan0 type vcan && sudo ip link set up vcan0`)*

### 4.3 Chạy chế độ giám sát Telemetry (Đọc trạng thái thời gian thực)
```bash
./build/mbdv_dual_axis_node -i can0 -d config/master.dcf -1 1 -2 2
```

### 4.4 Chạy chế độ kiểm tra kích hoạt Servo ON và điều khiển chuyển động đồng thời 2 trục
```bash
./build/mbdv_dual_axis_node -i can0 -d config/master.dcf -1 1 -2 2 --test-motion
```
