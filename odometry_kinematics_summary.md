# Tài liệu Tổng quan về Động học và Odometry của Robot

Tài liệu này tổng hợp cấu trúc triển khai, công thức động học thuận (Forward Kinematics) để tính toán Odometry và động học nghịch (Inverse Kinematics) để điều khiển robot trong dự án (bao gồm cả môi trường mô phỏng và robot thực tế).

---

## 1. Cấu trúc Triển khai Odometry trong Dự án

Dự án hỗ trợ cả hai môi trường: mô phỏng (Gazebo) và chạy trên robot thực tế thông qua các cơ chế tính toán khác nhau.

### 1.1. Môi trường mô phỏng (Simulation)
- **[SimpleController](file:///home/namnc/V-SLam/moblie/src/controller_robot/controller_robot/simple_controller.py#L15)**: Đọc thông tin phản hồi từ khớp bánh xe thông qua topic `/joint_states`. Hàm callback [jointCallback](file:///home/namnc/V-SLam/moblie/src/controller_robot/controller_robot/simple_controller.py#L77) tính toán động học thuận vi sai để xuất bản Odometry lên topic `mobile_controller/odom` và truyền TF `odom` $\rightarrow$ `base_footprint`.
- **Cách tính khoảng thời gian `dt`**: 
  `dt` là hiệu số thời gian giữa hai thông điệp nhận được liên tiếp từ topic `/joint_states` dựa trên timestamp của thông điệp:
  ```python
  dt = Time.from_msg(msg.header.stamp) - self.prev_time_
  ```

### 1.2. Môi trường thực tế (Real Robot)
- **[MobileInterface](file:///home/namnc/V-SLam/moblie/src/robot_firmware/src/mobile_interface.cpp#L7)**: Là lớp phần cứng (hardware interface) kết nối trực tiếp với vi điều khiển (MCU) của robot qua cổng Serial. Lớp này đọc vận tốc bánh xe do MCU gửi lên, sau đó tích phân để cập nhật góc xoay khớp bánh xe (`position_states_`).
- **Cách tính khoảng thời gian `dt`**:
  Do dữ liệu nhận được trực tiếp qua giao tiếp Serial, `dt` được đo bằng đồng hồ thời gian thực của máy tính điều khiển PC tại hàm [read](file:///home/namnc/V-SLam/moblie/src/robot_firmware/src/mobile_interface.cpp#L139):
  ```cpp
  auto dt = (rclcpp::Clock().now() - last_run_).seconds();
  ```
  Sau đó, góc quay bánh xe được tích lũy theo công thức:
  ```cpp
  position_states_.at(i) += velocity_states_.at(i) * dt;
  ```

### 1.3. Bộ điều khiển chuyển động (`diff_drive_controller`)
- File cấu hình [mobile_controller.yaml](file:///home/namnc/V-SLam/moblie/src/controller_robot/config/mobile_controller.yaml#L22) cấu hình plugin `diff_drive_controller/DiffDriveController` để điều khiển robot vi sai khi chạy thực tế (được kích hoạt trong [real_robot.launch.py](file:///home/namnc/V-SLam/moblie/src/robot_bringup/launch/real_robot.launch.py#L45)).
- Nó đọc góc quay tích lũy (`position_states_`) từ `MobileInterface` và tự động tính toán Odometry dựa trên chu kỳ điều khiển (`publish_rate` = 50.0 Hz).
- Tham số `enable_odom_tf` đặt thành `false` nhằm nhường việc truyền TF cho bộ lọc Kalman.

### 1.4. Bộ lọc Kalman (Sensor Fusion)
- **EKF Node (`robot_localization`)**: Cấu hình tại [ekf.yaml](file:///home/namnc/V-SLam/moblie/src/mobile_localization/config/ekf.yaml) kết hợp dữ liệu odom thô từ bánh xe và vận tốc góc từ IMU để xuất bản ước lượng vị trí chính xác hơn và truyền TF `odom` $\rightarrow$ `base_footprint`.

---

## 2. Công thức Động học Thuận (Tính Odometry từ Encoder)

### Đầu vào (Inputs)
- Bán kính bánh xe: `r` (0.07333 m)
- Khoảng cách hai bánh xe: `L` (0.4544 m)
- Góc quay hiện tại từ encoder: `phi_left`, `phi_right`
- Góc quay ở chu kỳ trước: `phi_left_prev`, `phi_right_prev`
- Chu kỳ thời gian trôi qua: `dt` (Lấy từ timestamp của `/joint_states` trong mô phỏng, hoặc đo bằng PC clock trong chạy thực tế).

### Công thức tính toán
1. **Lượng dịch chuyển góc bánh xe:**
   ```text
   dp_left = phi_left - phi_left_prev
   dp_right = phi_right - phi_right_prev
   ```
2. **Vận tốc góc bánh xe tức thời:**
   ```text
   fi_left = dp_left / dt
   fi_right = dp_right / dt
   ```
3. **Vận tốc tuyến tính (`linear`) và vận tốc góc (`angular`) của robot:**
   ```text
   linear = r * (fi_right + fi_left) / 2
   angular = r * (fi_right - fi_left) / L
   ```
4. **Lượng dịch chuyển quãng đường (`d_s`) và góc xoay (`d_theta`) của robot:**
   ```text
   d_s = r * (dp_right + dp_left) / 2
   d_theta = r * (dp_right - dp_left) / L
   ```
5. **Cập nhật tọa độ tư thế mới (`x`, `y`, `theta`):**
   ```text
   theta = theta_old + d_theta
   x = x_old + d_s * cos(theta)
   y = y_old + d_s * sin(theta)
   ```

### Đầu ra (Outputs)
- **Tọa độ tư thế:** `x`, `y`, `theta` (truyền qua TF `odom` -> `base_footprint`).
- **Vận tốc tức thời:** Vận tốc tuyến tính `linear` và vận tốc góc `angular`.

---

## 3. Công thức Động học Nghịch (Điều khiển Bánh xe từ cmd_vel)

Khi điều khiển robot từ bàn phím (hoặc node điều hướng), robot nhận lệnh vận tốc dài `v` và vận tốc góc `omega` qua topic `/cmd_vel`. 

### Công thức chuyển đổi sang vận tốc bánh xe
Để tính vận tốc góc cần đặt cho bánh xe trái (`omega_left`) và bánh xe phải (`omega_right`):

```text
omega_left = (2 * v - omega * L) / (2 * r)
omega_right = (2 * v + omega * L) / (2 * r)
```

### Đầu ra (Outputs)
- Mảng vận tốc gửi tới driver điều khiển động cơ: `[omega_left, omega_right]` qua topic `simple_velocity_controller/commands`.

---

## 4. Triển khai Điều khiển theo Động học vi sai với Driver Moons' MBDV Dual-Axis (CANopen / lely-core)

Phần này mô tả chi tiết kiến trúc điều khiển động học (Kinematic Control) tích hợp trực tiếp với bộ điều khiển **Moons' MBDV Dual-Axis** thông qua giao thức thời gian thực CANopen / lely-core trong thư mục dự án `driver_Moons_MBDV`.

### 4.1. Kiến trúc Luồng Dữ liệu Động học (Bidirectional Kinematic Pipeline)

```
                            [ Lệnh điều hướng: cmd_vel (v, w) ]
                                            │
                                            ▼
                    ┌───────────────────────────────────────────────┐
                    │    Động học nghịch (Inverse Kinematics)       │
                    │   v, w ──> omega_L, omega_R (rad/s)           │
                    └───────────────────────┬───────────────────────┘
                                            │ Quy đổi đơn vị: counts/rad
                                            ▼
                    ┌───────────────────────────────────────────────┐
                    │   CANopen RPDO3 (Master Send -> Drive Recv)   │
                    │   Node 1 (Axis 1): 0x6040 (CW) + 0x60FF_L     │
                    │   Node 2 (Axis 2): 0x6040 (CW) + 0x60FF_R     │
                    └───────────────────────┬───────────────────────┘
                                            │ Bus CAN (1 Mbps)
                                            ▼
                             [ Động cơ Moons' MBDV-2X ]
                                            │ Phản hồi Encoder
                                            ▼
                    ┌───────────────────────────────────────────────┐
                    │   CANopen TPDO2 (Drive Send -> Master Recv)   │
                    │   Node 1 (Axis 1): 0x6064_L (pos) + 0x606C_L  │
                    │   Node 2 (Axis 2): 0x6064_R (pos) + 0x606C_R  │
                    └───────────────────────┬───────────────────────┘
                                            │ Delta ticks: Δp_L, Δp_R
                                            ▼
                    ┌───────────────────────────────────────────────┐
                    │    Động học thuận (Forward Kinematics)        │
                    │   Tích phân Runge-Kutta bậc 2 (Midpoint)      │
                    │   ──> Odometry Pose: x, y, theta              │
                    │   ──> Feedback Twist: v_act, w_act            │
                    └───────────────────────────────────────────────┘
```

---

### 4.2. Tham số Hình học & Hệ số Quy đổi Xung

Bảng tham số chuẩn của khung gầm vi sai kết hợp động cơ servo Moons':

| Tham số | Ký hiệu | Giá trị mặc định | Đơn vị | Ghi chú |
| :--- | :---: | :---: | :---: | :--- |
| Bán kính bánh xe | $r$ | `0.07333` | $\text{m}$ | Bán kính hiệu dụng lăn |
| Chiều rộng cơ sở | $L$ | `0.4544` | $\text{m}$ | Khoảng cách giữa 2 vệt tiếp đất |
| Tỉ số truyền hộp số | $N$ | `1.0` | - | Tỉ số giảm tốc cơ khí |
| Độ phân giải encoder | $\text{CPR}$ | `10000` | $\text{counts/rev}$ | Xung encoder động cơ / vòng |
| Giới hạn vận tốc dài | $v_{\max}$ | `1.5` | $\text{m/s}$ | Kẹp vận tốc an toàn (Clamping) |
| Giới hạn vận tốc góc | $\omega_{\max}$ | `3.0` | $\text{rad/s}$ | Kẹp vận tốc xoay an toàn |

Hệ số quy đổi góc quay trục bánh xe sang xung encoder của driver:

$$k_{\text{pulse}} = \frac{\text{CPR} \cdot N}{2\pi} \quad \left[\frac{\text{counts}}{\text{rad}}\right]$$

Hệ số quãng đường trên mỗi xung đếm:

$$k_{\text{dist}} = \frac{2\pi \cdot r}{\text{CPR} \cdot N} \quad \left[\frac{\text{m}}{\text{count}}\right]$$

---

### 4.3. Công thức Động học Nghịch & Điều khiển Vận tốc (Inverse Kinematics)

Nhận lệnh vận tốc tổng quát của robot từ bộ lập quỹ đạo $\mathbf{u} = \begin{bmatrix} v & \omega \end{bmatrix}^T$:

1. **Kẹp vận tốc an toàn (Velocity Clamping):**
   $$v_{\text{cmd}} = \text{clamp}(v, -v_{\max}, v_{\max})$$
   $$\omega_{\text{cmd}} = \text{clamp}(\omega, -\omega_{\max}, \omega_{\max})$$

2. **Chuyển đổi sang vận tốc góc của từng bánh xe:**
   $$\begin{bmatrix} \omega_L \\ \omega_R \end{bmatrix} = \begin{bmatrix} \frac{1}{r} & -\frac{L}{2r} \\ \frac{1}{r} & \frac{L}{2r} \end{bmatrix} \begin{bmatrix} v_{\text{cmd}} \\ \omega_{\text{cmd}} \end{bmatrix}$$

   Khai triển:
   $$\omega_L = \frac{v_{\text{cmd}} - \frac{\omega_{\text{cmd}} \cdot L}{2}}{r} \quad [\text{rad/s}]$$
   $$\omega_R = \frac{v_{\text{cmd}} + \frac{\omega_{\text{cmd}} \cdot L}{2}}{r} \quad [\text{rad/s}]$$

3. **Chuyển đổi sang giá trị đặt cho thanh ghi `0x60FF` (Target Velocity qua RPDO3):**
   $$\text{Target\_Vel}_L = \text{round}\left(\omega_L \cdot k_{\text{pulse}}\right) = \text{round}\left(\omega_L \cdot \frac{\text{CPR} \cdot N}{2\pi}\right) \quad [\text{counts/s}]$$
   $$\text{Target\_Vel}_R = \text{round}\left(\omega_R \cdot k_{\text{pulse}}\right) = \text{round}\left(\omega_R \cdot \frac{\text{CPR} \cdot N}{2\pi}\right) \quad [\text{counts/s}]$$

4. **Gửi lệnh đồng thời qua CANopen:**
   - Đặt chế độ hoạt động: `0x6060:00` = `3` (**Profile Velocity Mode - PV**).
   - Bật servo: Controlword `0x6040:00` = `0x000F` (**Operation Enabled**).
   - Truyền giá trị `Target_Vel_L` tới Trục 1 (Node 1) và `Target_Vel_R` tới Trục 2 (Node 2) trong cùng một chu kỳ bus CAN.

---

### 4.4. Công thức Động học Thuận & Tích phân Odometry (Forward Kinematics)

Đọc phản hồi từ đối tượng `0x6064:00` (**Position Actual Value**) của từng trục qua TPDO2 tại thời điểm chu kỳ $k$:

1. **Hiệu số xung đếm (Delta Ticks):**
   $$\Delta \text{ticks}_L = p_L(k) - p_L(k-1)$$
   $$\Delta \text{ticks}_R = p_R(k) - p_R(k-1)$$

2. **Dịch chuyển tịnh tiến của từng bánh xe:**
   $$\Delta s_L = \Delta \text{ticks}_L \cdot k_{\text{dist}} = \frac{2\pi \cdot r \cdot \Delta \text{ticks}_L}{\text{CPR} \cdot N} \quad [\text{m}]$$
   $$\Delta s_R = \Delta \text{ticks}_R \cdot k_{\text{dist}} = \frac{2\pi \cdot r \cdot \Delta \text{ticks}_R}{\text{CPR} \cdot N} \quad [\text{m}]$$

3. **Quãng đường di chuyển của tâm robot và biến thiên góc hướng:**
   $$\Delta s = \frac{\Delta s_R + \Delta s_L}{2} \quad [\text{m}]$$
   $$\Delta \theta = \frac{\Delta s_R - \Delta s_L}{L} \quad [\text{rad}]$$

4. **Tích phân tư thế Runge-Kutta bậc 2 (Midpoint Integration):**
   Nhằm triệt tiêu sai số tích phân tuyến tính bậc 1 (Euler) khi robot vừa tịnh tiến vừa quay:
   $$\theta_{\text{mid}} = \theta_{k-1} + \frac{\Delta \theta}{2}$$
   $$x_k = x_{k-1} + \Delta s \cdot \cos\left(\theta_{\text{mid}}\right)$$
   $$y_k = y_{k-1} + \Delta s \cdot \sin\left(\theta_{\text{mid}}\right)$$
   $$\theta_k = \text{atan2}\left(\sin(\theta_{k-1} + \Delta \theta), \cos(\theta_{k-1} + \Delta \theta)\right)$$

5. **Ước lượng vận tốc thực tế (Velocity Derivation):**
   $$v_{\text{act}} = \frac{\Delta s}{\Delta t} \quad [\text{m/s}], \quad \omega_{\text{act}} = \frac{\Delta \theta}{\Delta t} \quad [\text{rad/s}]$$

---

### 4.5. Lớp C++ Triển khai trong Dự án

Dự án cung cấp bộ điều khiển động học hoàn chỉnh:
- **Header:** [`include/mbdv/diff_drive_kinematics.hpp`](file:///home/namnc/driver_motor/driver_Moons_MBDV/include/mbdv/diff_drive_kinematics.hpp)
- **Source:** [`src/diff_drive_kinematics.cpp`](file:///home/namnc/driver_motor/driver_Moons_MBDV/src/diff_drive_kinematics.cpp)
- **Tích hợp vào Master Controller:** [`include/mbdv/dual_axis_controller.hpp`](file:///home/namnc/driver_motor/driver_Moons_MBDV/include/mbdv/dual_axis_controller.hpp) và [`src/dual_axis_controller.cpp`](file:///home/namnc/driver_motor/driver_Moons_MBDV/src/dual_axis_controller.cpp)
  - `controller.SetCmdVel(linear_v, angular_w)`: Tự động tính toán động học nghịch và gửi xung vận tốc tới cả 2 trục.
  - `controller.UpdateOdometry(dt_sec)`: Tích phân động học thuận cập nhật tư thế robot $(x, y, \theta)$ từ dữ liệu xung encoder TPDO2.
  - `controller.GetRobotPose()`, `controller.GetRobotTwist()`: Cung cấp vị trí và vận tốc thời gian thực cho hệ thống điều hướng / ROS2.

