---
name: read-source-code
description: Trace và giải thích hành vi của source code robotics, ROS, CV, edge AI dựa trên bằng chứng từ repository, không phỏng đoán chi tiết project. Dùng khi người dùng yêu cầu 'giải thích code này', 'trace logic của node X', 'tìm bug trong pipeline', 'làm thế nào X hoạt động', hoặc cần hiểu callback flow, executor, topic pub/sub, model inference path.
---

# Read Source Code

## Mục tiêu

Giải thích code hoạt động như thế nào, dependencies là gì, và behavior được configure ở đâu, trong khi giữ unknown project details là explicit.

## Khi nào dùng

Dùng khi cần hiểu một node, package, launch path, perception pipeline, controller, driver wrapper, model runtime, hoặc bug path.

## Quy trình

### Bước 1: Đọc context

Đọc project context files nếu có.

### Bước 2: Bắt đầu từ entry point

Bắt đầu từ file/symbol/log/topic/node/model mà user cung cấp.

### Bước 3: Trace theo hướng phù hợp

Dùng source search để trace: definitions, call sites, launch/config references, parameters, tests.

**ROS 1:** publishers/subscribers, services, actionlib, node handles, private parameters, callbacks, timers, spin model

**ROS 2:** nodes/components, executors, callback groups, lifecycle, QoS, parameters, services/actions, timers, composition

**CV/AI:** image encoding, calibration, preprocessing, inference, postprocessing, synchronization, output contracts

### Bước 4: Ghi nhận unknowns

Mark bất kỳ details nào chưa xác minh được là `Unknown / needs confirmation`.

## Checklist

- [ ] Entry points và runtime configuration đã xác định
- [ ] Message types, units, frames, timestamps, rates đã kiểm tra
- [ ] Threading, callback order, queues/QoS, shared state đã kiểm tra
- [ ] Error handling, timeouts, watchdogs, degraded sensor input đã kiểm tra
- [ ] Unknown details đã mark rõ

## Ví dụ minh hoạ

**Ví dụ 1: Trace tại sao node không publish**

Người dùng nói: "detection_node chạy nhưng /detections topic luôn trống"

Hành động:
1. Tìm node entry point, locate publisher cho `/detections`
2. Trace callback: subscriber nhận gì → xử lý → điều kiện publish
3. Tìm guard conditions: threshold filter, confidence check, state machine
4. Kiểm tra có publisher được tạo đúng không (wrong namespace?)

Kết quả: Xác định chính xác line code gây ra issue, không publish, explain logic.

**Ví dụ 2: Hiểu executor và callback groups**

Người dùng nói: "Tại sao timer callback của tôi bị block bởi subscriber callback?"

Hành động:
1. Tìm executor type: SingleThreadedExecutor hay MultiThreadedExecutor?
2. Kiểm tra callback group assignment: MutuallyExclusive hay Reentrant?
3. Trace timer và subscriber được assign group nào
4. Giải thích scheduling behavior với executor đó

Kết quả: Explanation cụ thể + fix với đúng callback group type.

## Khắc phục sự cố

**Lỗi:** Không tìm được node entry point
**Nguyên nhân:** Node được launch qua `ros2 run` với package/executable khác tên folder
**Cách xử lý:** Tìm trong `CMakeLists.txt` (ament_target_dependencies), `setup.py` (entry_points), hoặc launch file

**Lỗi:** Callback trace dừng lại tại virtual function
**Nguyên nhân:** Polymorphism, plugin architecture (pluginlib, class_loader)
**Cách xử lý:** Tìm plugin manifest (`plugins.xml`), grep registered class name, trace concrete implementation

## Output Format

- What it does: concise behavior summary (1-3 sentences)
- Evidence: file:line references
- Runtime dependencies: topics, params, frames, models, hardware nếu verified
- Risks hoặc ambiguities: practical issues
- Next verification: tests, commands, logs, hoặc questions
