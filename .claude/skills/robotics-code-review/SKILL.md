---
name: robotics-code-review
description: Review code robotics, ROS, CV, edge AI để tìm behavioral bugs, integration risks, timing issues, safety concerns và gaps trong verification. Dùng khi người dùng yêu cầu 'review code', 'check PR này', 'kiểm tra launch file', 'review perception changes', 'audit safety', hoặc trước khi merge code ảnh hưởng đến runtime behavior, safety, timing, resource management.
---

# Robotics Code Review

## Mục tiêu

Tìm defects và risks cụ thể trong robotics code, ưu tiên runtime behavior, safety, integration contracts, timing, và missing tests.

## Khi nào dùng

Dùng cho pull requests, diffs, package changes, launch/config updates, perception changes, model deployment changes, drivers, controllers, hoặc performance-sensitive code.

## Quy trình

### Bước 1: Đọc context

Đọc project context files nếu có.

### Bước 2: Review target files

Phân biệt changed behavior từ existing behavior.

### Bước 3: Kiểm tra theo ROS version

**ROS 1:** callback queues, node handles, params, remaps, launch XML, message filters, catkin/package dependencies

**ROS 2:** QoS, executors, callback groups, lifecycle, parameters, composition, launch files, ament dependencies

### Bước 4: Kiểm tra common issues

Timestamps, frame IDs, units, synchronization, calibration, resource lifetime, error handling, diagnostics, degraded input behavior.

### Bước 5: Kiểm tra CV/AI specifics

Tensor contracts, encodings, precision, thresholds, model artifact paths, runtime compatibility.

## Checklist

- [ ] Findings tied to file/line evidence
- [ ] Không phỏng đoán project-specific facts
- [ ] Safety, timing, hardware interaction, recovery paths đã xem xét
- [ ] Tests hoặc runtime verification gaps đã xác định
- [ ] Unknowns mark `Unknown / needs confirmation`

## Ví dụ minh hoạ

**Ví dụ 1: Review perception node mới**

Người dùng nói: "Review PR thêm YOLOv8 detection node này trước khi merge"

Hành động:
1. Kiểm tra callback: subscriber nhận image, có check encoding không?
2. Kiểm tra preprocessing: có tương thích với model training preprocessing không?
3. Kiểm tra postprocessing: NMS threshold, coordinate transform đúng frame chưa?
4. Kiểm tra error handling: model load thất bại xử lý thế nào?
5. Kiểm tra diagnostics: có publish `/diagnostics` khi model không ready không?

Kết quả: Findings ordered by severity, mỗi finding có file:line, issue, impact, fix.

**Ví dụ 2: Review launch/config changes**

Người dùng nói: "Review file launch này, tôi sửa một số parameters"

Hành động:
1. Kiểm tra parameter names: đúng namespace? Có typo?
2. Kiểm tra remaps: topic names khớp với subscribers/publishers không?
3. Kiểm tra node startup order: dependencies đúng thứ tự chưa?
4. Kiểm tra có hardcoded paths/IP addresses không

Kết quả: List issues với severity, deployment risks.

## Khắc phục sự cố

**Lỗi:** Không xác định được severity của finding
**Nguyên nhân:** Thiếu context về robot behavior và safety requirements
**Cách xử lý:** Mark `Unknown / needs confirmation` cho safety impact, hỏi user về use case và safety constraints

**Lỗi:** Code quá phức tạp để trace hết
**Nguyên nhân:** Large codebase, nhiều abstraction layers
**Cách xử lý:** Focus vào changed files trong diff, trace outward từ changed lines, không review toàn bộ codebase

## Output Format

- Findings: ordered by severity (Critical > High > Medium > Low)
- Mỗi finding: `file:line | issue | impact | suggested fix`
- Open questions
- Verification gaps: tests cần viết
- Brief change summary nếu relevant
