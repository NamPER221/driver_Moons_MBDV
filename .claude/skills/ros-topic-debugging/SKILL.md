---
name: ros-topic-debugging
description: Debug vấn đề ROS 1/ROS 2 topic như thiếu messages, wrong types, rate bất thường, stale timestamps, queue drops, QoS mismatch, encoding errors. Dùng khi người dùng báo 'topic không có data', 'subscriber không nhận messages', 'rate quá thấp', 'QoS incompatible', 'message type mismatch', 'latching không hoạt động', hoặc 'timestamp cũ'.
---

# ROS Topic Debugging

## Mục tiêu

Chẩn đoán topic-related failures như missing messages, wrong types, bad rates, stale timestamps, queue drops, QoS mismatches, hoặc encoding/contract errors.

## Khi nào dùng

Dùng cho publisher/subscriber issues, perception input failures, control command gaps, synchronization problems, bag replay differences, hoặc bridge problems.

## Quy trình

### Bước 1: Đọc context

Đọc `PROJECT_CONTEXT.md` và `ROS_TOPIC_MAP.md` nếu có.

### Bước 2: Discover topics

Tìm topic names và message types từ source, launch, config, docs, hoặc user logs.

### Bước 3: Kiểm tra theo ROS version

**ROS 1:** remaps, namespaces, queue sizes, latching, `message_filters`, transport hints, `rostopic` evidence

**ROS 2:** remaps, namespaces, QoS profiles, durability, reliability, history depth, lifecycle state, `ros2 topic` evidence

### Bước 4: Verify message contract

Message fields, timestamps, frame IDs, units, rates, image encodings.

### Bước 5: Phân biệt evidence

Phân biệt verified runtime evidence từ expected behavior.

## Checklist

- [ ] Publisher và subscriber code paths đã xác định
- [ ] Message type và schema đã verify
- [ ] Topic names, namespaces, remaps đã trace
- [ ] QoS hoặc queue settings đã kiểm tra
- [ ] Time synchronization và frame IDs đã kiểm tra
- [ ] Unknowns mark `Unknown / needs confirmation`

## Ví dụ minh hoạ

**Ví dụ 1: Subscriber không nhận messages**

Người dùng nói: "Node của tôi subscribe `/lidar/points` nhưng callback không bao giờ được gọi"

Hành động:
1. Trace publisher của `/lidar/points` trong codebase
2. Kiểm tra topic name exact match (case, namespace, leading slash)
3. ROS 2: Kiểm tra QoS mismatch (publisher dùng BEST_EFFORT, subscriber dùng RELIABLE?)
4. Kiểm tra lifecycle: publisher node đã trong ACTIVE state chưa?
5. Kiểm tra có remap ẩn trong launch file không

Kết quả: Root cause với specific file:line, fix commands.

**Ví dụ 2: Image topic có rate thấp bất thường**

Người dùng nói: "Camera publish 30fps nhưng `/detections` chỉ ra 5Hz"

Hành động:
1. Kiểm tra subscriber queue size: đang drop messages?
2. Kiểm tra callback processing time: quá lâu so với message interval?
3. Kiểm tra SingleThreadedExecutor: bị block bởi slow callback khác?
4. Kiểm tra image_transport: compressed transport có được decode đúng không?

Kết quả: Bottleneck identification với executor/callback fix hoặc queue adjustment.

## Khắc phục sự cố

**Lỗi:** QoS incompatible warning trong ROS 2
**Nguyên nhân:** Publisher và subscriber có QoS profile không match (Reliability, Durability, History)
**Cách xử lý:** Kiểm tra cả hai phía, set subscriber Reliability = BEST_EFFORT nếu publisher là BEST_EFFORT

**Lỗi:** Topic tồn tại nhưng timestamp luôn cũ (stale)
**Nguyên nhân:** Publisher dùng `ros::Time(0)` hoặc `rclpy.time.Time()` thay vì current time
**Cách xử lý:** Trace timestamp assignment trong publisher code, thay bằng `node.get_clock().now()`

**Lỗi:** `message_filters` ApproximateTimeSynchronizer không trigger
**Nguyên nhân:** Slop quá nhỏ hoặc messages không arrive trong cùng time window
**Cách xử lý:** Tăng slop value, kiểm tra header.stamp của từng topic, verify clock source match

## Output Format

- Symptom: mô tả cụ thể vấn đề
- Verified topic map: publishers, subscribers, types, namespaces
- Likely causes: ordered by evidence strength
- ROS 1/ROS 2 checks applicable
- Commands: chỉ dùng nếu verified hoặc clearly generic
- Next action: cụ thể nhất có thể
