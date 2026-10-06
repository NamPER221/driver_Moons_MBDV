---
name: rosbag-analysis
description: Phân tích ROS 1 bag và ROS 2 bag2 để kiểm tra topics, timing, frame consistency, sensor quality và debug perception/model inputs. Dùng khi người dùng có file .bag hoặc bag2 directory, yêu cầu 'phân tích bag', 'debug từ recording', 'kiểm tra sensor data', 'replay bag', 'xem topic rate', hoặc debug recorded failures.
---

# Rosbag Analysis

## Mục tiêu

Dùng bag data để verify runtime behavior, topic contracts, timing, frame consistency, và perception/model inputs, tránh phỏng đoán về robot.

## Khi nào dùng

Dùng khi debug recorded failures, replay differences, sensor timing, dropped data, perception outputs, model inputs, hoặc control behavior.

## Quy trình

### Bước 1: Đọc context

Đọc project context và topic/frame maps nếu có.

### Bước 2: Xác định bag format

Xác định ROS 1 rosbag hay ROS 2 rosbag2 từ files hoặc user input.

### Bước 3: Inventory bag contents

Topics, message types, start/end time, rates, frame IDs, sizes.

### Bước 4: Kiểm tra format-specific details

**ROS 1:** `rosbag info`, `rostopic hz`, playback clock, latching, connection metadata

**ROS 2:** `ros2 bag info`, storage backend, serialization format, QoS override, simulated time

### Bước 5: Kiểm tra data quality

Synchronization, timestamp monotonicity, dropped frames, image encodings, calibration topics/files, TF coverage, model input/output topics.

## Checklist

- [ ] Bag format và metadata đã xác minh
- [ ] Topic types và rates đã ghi nhận
- [ ] TF availability và timestamp range đã kiểm tra
- [ ] Sensor payload quality và encodings đã kiểm tra
- [ ] Replay assumptions đã document
- [ ] Unknowns mark `Unknown / needs confirmation`

## Ví dụ minh hoạ

**Ví dụ 1: Debug tại sao detection thất bại ở một thời điểm trong bag**

Người dùng nói: "Trong bag này, detection bị miss object từ giây 45-60, cần hiểu tại sao"

Hành động:
1. `ros2 bag info` để xem topics và time range
2. Kiểm tra `/camera/image_raw` rate trong khoảng 45-60s: có drop không?
3. Kiểm tra `/camera/camera_info` có publish parallel không
4. Kiểm tra TF: frame `camera_link` → `base_link` có available không
5. Đề xuất replay với specific time range để observe

Kết quả: Timeline của events, likely cause (camera drop? TF gap?), reproduce steps.

**Ví dụ 2: Validate sensor data quality**

Người dùng nói: "Kiểm tra bag này xem LiDAR và camera có sync không"

Hành động:
1. List camera topics và LiDAR topics với rates
2. So sánh timestamps: camera header.stamp vs LiDAR header.stamp
3. Kiểm tra delta: difference trong 100ms là ok, 500ms+ là vấn đề
4. Kiểm tra TF tree có cover cả LiDAR frame và camera frame không

Kết quả: Sync quality report với max/avg/min delta, TF coverage status.

## Khắc phục sự cố

**Lỗi:** `ros2 bag play` báo lỗi QoS incompatible
**Nguyên nhân:** Bag được record với QoS settings khác node đang subscribe
**Cách xử lý:** Dùng `--qos-profile-overrides-path` hoặc override reliability thành BEST_EFFORT

**Lỗi:** Timestamp không monotonic trong bag
**Nguyên nhân:** Clock jump, simulated time bị reset, hoặc sensor internal clock drift
**Cách xử lý:** Kiểm tra có dùng `/clock` topic không, verify sensor driver timestamp source

**Lỗi:** Bag thiếu TF data ở một thời điểm
**Nguyên nhân:** TF broadcaster bị crash hoặc không record `/tf_static`
**Cách xử lý:** Kiểm tra `/tf_static` có trong bag không, replay với `--clock` flag

## Output Format

- Bag identity và format (ROS 1/2, storage backend)
- Topic summary: names, types, rates, count
- Timing và TF findings
- Sensor/CV/model observations
- Likely causes hoặc risks
- Reproducible next commands hoặc checks
