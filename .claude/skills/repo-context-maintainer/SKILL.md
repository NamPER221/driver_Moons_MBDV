---
name: repo-context-maintainer
description: Cập nhật và duy trì PROJECT_CONTEXT.md, ROS_TOPIC_MAP.md, TF_FRAME_MAP.md, MODEL_DEPLOYMENT_CONTEXT.md với facts đã được xác minh từ repository. Dùng khi context files bị stale, sau code changes lớn, trước tasks phức tạp, hoặc khi người dùng yêu cầu 'cập nhật context', 'refresh project map', 'update topic map', 'sync PROJECT_CONTEXT'.
---

# Repo Context Maintainer

## Mục tiêu

Giữ project context files chính xác, cập nhật và hữu ích mà không thêm unverified robotics assumptions.

## Khi nào dùng

Dùng sau onboarding, sau major code changes, trước large tasks, hoặc khi context files bị missing, stale, hoặc incomplete.

## Quy trình

### Bước 1: Đọc context files hiện có

Đọc `PROJECT_CONTEXT.md`, `ROS_TOPIC_MAP.md`, `TF_FRAME_MAP.md`, `MODEL_DEPLOYMENT_CONTEXT.md` nếu có.

### Bước 2: Inspect repository

Inspect: manifests, docs, launch/config, source, tests, CI, model assets, deployment files.

### Bước 3: Update chỉ những gì xác minh được

Chỉ update fields có thể verify từ repository evidence hoặc explicit user input.

### Bước 4: Preserve unknowns

Giữ `Unknown / needs confirmation` cho package names, topics, frames, commands, hardware, model paths, build details chưa xác minh.

### Bước 5: Ghi nguồn xác minh

Ghi rõ source files dùng để verify và open questions.

## Checklist

- [ ] Context changes có evidence từ repo
- [ ] Commands đã verify trước khi document
- [ ] Topics, services, actions, frames, models, hardware không được phỏng đoán
- [ ] CV/AI deployment contracts đã document nếu có
- [ ] Stale hoặc contradictory facts đã được flag

## Ví dụ minh hoạ

**Ví dụ 1: Update sau khi thêm camera mới**

Người dùng nói: "Tôi vừa thêm depth camera vào hệ thống, cần cập nhật context"

Hành động:
1. Tìm camera driver node mới trong source/launch
2. Xác minh topics: `/depth/image_raw`, `/depth/camera_info`, `/depth/points`
3. Tìm frame ID trong URDF hoặc launch
4. Update PROJECT_CONTEXT.md phần Sensors và Runtime Interfaces
5. Update ROS_TOPIC_MAP.md với camera topics mới

Kết quả: Updated files với verified entries, chỉ rõ files đã check.

**Ví dụ 2: Sync sau refactor lớn**

Người dùng nói: "Tôi vừa refactor detection module, context files cũ rồi"

Hành động:
1. Compare context hiện tại với code thực tế
2. Tìm topics/nodes đã đổi tên hoặc xóa
3. Tìm topics/nodes mới chưa có trong context
4. Update MODEL_DEPLOYMENT_CONTEXT.md nếu model path/format đổi
5. Flag những changes có thể ảnh hưởng integration

Kết quả: Updated context với diff summary và stale entries đã xóa.

## Khắc phục sự cố

**Lỗi:** Không biết topic có còn active không
**Nguyên nhân:** Code có thể conditional publish hoặc topic được rename
**Cách xử lý:** Grep cả codebase cho topic string, kiểm tra conditional blocks, ghi `Needs runtime verification` trong context

**Lỗi:** Build command trong context không còn chạy được
**Nguyên nhân:** Build system đã thay đổi (catkin → colcon, pip → uv)
**Cách xử lý:** Tìm CI scripts (`.github/workflows/`), Makefile, hoặc README mới nhất để lấy command đúng

## Output Format

- Files đã update
- Facts đã add hoặc correct
- Evidence files đã inspect
- Remaining unknowns
- Suggested next context improvements
