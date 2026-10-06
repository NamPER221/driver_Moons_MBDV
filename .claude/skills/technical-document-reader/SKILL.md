---
name: technical-document-reader
description: Đọc tài liệu kỹ thuật robotics, ROS, CV, AI, hardware, vendor và trích xuất engineering facts đã xác minh mà không thêm thông tin project chưa biết. Dùng khi người dùng upload datasheet, API docs, paper, calibration note, model card, hardware manual, hoặc yêu cầu 'đọc tài liệu này', 'extract specs từ PDF', 'tóm tắt technical doc', 'tìm thông số kỹ thuật'.
---

# Technical Document Reader

## Mục tiêu

Trích xuất facts có thể hành động, có nguồn gốc rõ ràng từ technical documents và chỉ connect với target project khi repository confirm relationship.

## Khi nào dùng

Dùng cho datasheets, API docs, papers, READMEs, design notes, calibration notes, ROS package docs, model cards, hardware manuals, vendor deployment guides.

## Quy trình

### Bước 1: Xác định document

Document source, version, date, và scope.

### Bước 2: Đọc project context

Đọc `PROJECT_CONTEXT.md` nếu có. Nếu không, mark project context là `Unknown / needs confirmation`.

### Bước 3: Extract facts với citations

Extract facts với citations đến sections, files, hoặc page numbers khi available.

### Bước 4: Phân tách document facts và repo facts

Tách biệt rõ facts từ document với facts từ target repo.

### Bước 5: Kiểm tra theo domain

**ROS 1:** topics, nodelets, parameters, launch XML, catkin, rosbag, dynamic reconfigure

**ROS 2:** nodes, components, lifecycle, parameters, launch Python/XML/YAML, QoS, rosbag2, colcon

**CV/AI:** camera model, calibration, encoding, preprocessing, model runtime, precision, memory

### Bước 6: Flag risks

Unsupported assumptions, missing versions, ambiguous units, timing constraints, calibration requirements, hardware dependencies.

## Checklist

- [ ] Source identity và version đã capture
- [ ] Units, coordinate frames, timestamp expectations, message/schema contracts đã kiểm tra
- [ ] Camera model, calibration, image encoding, preprocessing đã kiểm tra khi relevant
- [ ] Model runtime, precision, accelerator, memory, latency constraints đã kiểm tra khi relevant
- [ ] Unknown target-specific fields là `Unknown / needs confirmation`

## Ví dụ minh hoạ

**Ví dụ 1: Đọc camera datasheet để cấu hình driver**

Người dùng nói: "Đây là datasheet của Intel RealSense D435i, tôi cần biết thông số để configure ROS driver"

Hành động:
1. Extract: resolution options, frame rate, depth range, color/IR specs
2. Extract: interface (USB 3.0), SDK version required
3. Map sang ROS parameters: `color_width`, `color_height`, `color_fps`, `depth_fps`
4. Flag: calibration files cần thiết, `depth_scale` parameter
5. Note: ROS 1 dùng `realsense2_camera`, ROS 2 dùng `realsense2_camera_node`

Kết quả: Parameter list với values và source citations, missing info list.

**Ví dụ 2: Đọc model card để verify deployment**

Người dùng nói: "Model card của YOLOv8n, tôi cần biết preprocessing requirements"

Hành động:
1. Extract input spec: `[1, 3, 640, 640]`, RGB, normalized `[0, 1]`, letterbox resize
2. Extract output spec: `[1, 84, 8400]`, XYXY format, confidence scores
3. Compare với node preprocessing trong repo nếu có
4. Flag: letterbox vs simple resize difference, normalization order (mean/std vs /255)

Kết quả: Verified preprocessing contract, mismatch warnings nếu có.

## Khắc phục sự cố

**Lỗi:** Document dùng đơn vị ambiguous (mm hay m? rad hay deg?)
**Nguyên nhân:** Vendor docs không nhất quán
**Cách xử lý:** Flag ambiguity rõ ràng, không assume, đề xuất user verify với hardware test

**Lỗi:** Document version cũ không match với SDK hiện tại
**Nguyên nhân:** API thay đổi giữa versions
**Cách xử lý:** Note version mismatch, recommend kiểm tra changelog hoặc latest docs

## Output Format

- Document: name/version/source
- Verified facts: concise bullets với source references (section/page)
- Repository matches: facts đã confirm trong target repo
- Unknowns: `Unknown / needs confirmation`
- Risks: integration, safety, timing, compatibility, deployment concerns
- Next checks: specific files, commands, hoặc questions
