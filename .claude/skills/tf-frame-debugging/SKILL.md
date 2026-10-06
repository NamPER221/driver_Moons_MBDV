---
name: tf-frame-debugging
description: Debug vấn đề TF/TF2 frame trong ROS bao gồm transforms, timestamps, naming, static/dynamic broadcasters và frame conventions. Dùng khi người dùng báo 'missing transform', 'extrapolation error', 'TF tree broken', 'sensor không align', 'navigation thất bại do frame', 'URDF frame không đúng', hoặc 'visualization lệch'.
---

# TF Frame Debugging

## Mục tiêu

Tìm TF tree, transform timing, frame naming, và coordinate convention issues mà không phỏng đoán robot frames.

## Khi nào dùng

Dùng cho missing transforms, extrapolation errors, incorrect visualization, sensor alignment, navigation failures, perception frame mismatches, hoặc calibration suspicion.

## Quy trình

### Bước 1: Đọc context

Đọc `PROJECT_CONTEXT.md` và `TF_FRAME_MAP.md` nếu có.

### Bước 2: Discover frames

Tìm frame IDs từ source, URDF/Xacro, launch, config, bags, logs, message headers.

### Bước 3: Kiểm tra theo ROS version

**ROS 1:** `tf`/`tf2_ros`, static transform publishers, URDF robot state publisher, launch XML

**ROS 2:** `tf2_ros`, static transform publishers, robot state publisher, lifecycle/composition, launch Python/YAML

### Bước 4: Verify transforms

Parent-child direction, timestamp source, broadcast rate, static vs dynamic intent, optical frame conventions, map/odom/base relationships.

### Bước 5: Mark unknowns

Absent hoặc ambiguous frames là `Unknown / needs confirmation`.

## Checklist

- [ ] Frame names đã verify từ repo hoặc runtime evidence
- [ ] Static và dynamic transforms đã tách biệt
- [ ] Timestamp và clock source đã kiểm tra
- [ ] Camera optical frame và REP-103/REP-105 conventions đã xem xét
- [ ] Calibration files và URDF/Xacro consistency đã kiểm tra
- [ ] Unknowns đã mark rõ

## Ví dụ minh hoạ

**Ví dụ 1: "Cannot transform from camera_frame to base_link"**

Người dùng nói: "Tôi bị lỗi: 'Could not transform camera_link to base_link at time X'"

Hành động:
1. Tìm `camera_link` broadcaster: static transform publisher hay robot state publisher?
2. Kiểm tra URDF: `camera_link` có được define không? Parent frame là gì?
3. Kiểm tra timestamp: broadcaster có dùng đúng clock không (sim time vs real time)?
4. Kiểm tra tree connectivity: `base_link` → ... → `camera_link` có path không?
5. Đề xuất debug: `ros2 run tf2_tools view_frames` hoặc `tf2_echo`

Kết quả: Missing link trong TF tree, specific broadcaster file/line cần fix.

**Ví dụ 2: Camera point cloud lệch so với LiDAR**

Người dùng nói: "Khi overlay camera và LiDAR trong RViz, chúng bị lệch ~10cm"

Hành động:
1. Kiểm tra `camera_optical_frame` vs `camera_link` - có missing REP-103 rotation không?
2. Kiểm tra extrinsic calibration: giá trị trong URDF có đúng không?
3. Kiểm tra timestamp của camera và LiDAR có sync không
4. Kiểm tra có `base_link` → `camera_link` → `camera_optical_frame` chain đúng không

Kết quả: Identify xem là calibration error, frame convention error, hay timestamp issue.

## Khắc phục sự cố

**Lỗi:** "ExtrapolationException: Lookup would require extrapolation into the future"
**Nguyên nhân:** TF lookup time mới hơn latest available transform (clock skew hoặc lookup too early)
**Cách xử lý:** Dùng `rclpy.time.Time()` (latest) thay vì specific timestamp, hoặc tăng timeout trong `lookup_transform`

**Lỗi:** Static transform bị override liên tục
**Nguyên nhân:** Hai nodes publish cùng static transform với values khác nhau
**Cách xử lý:** Grep tất cả nodes publish transform đó, xác định node nào "win", tắt node redundant

**Lỗi:** TF tree bị broken sau khi node restart
**Nguyên nhân:** Dynamic transform broadcaster restart, TF buffer chưa populated lại
**Cách xử lý:** Thêm delay hoặc wait-for-transform trước lookup, publish `/tf_static` cho static transforms

## Output Format

- Symptom: error message cụ thể
- Verified frame facts: broadcasters, rates, parent-child
- Suspected issue và evidence
- ROS 1 hoặc ROS 2 verification steps
- Risk to perception/navigation/control
- Next action: commands hoặc code changes cụ thể
