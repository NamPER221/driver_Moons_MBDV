---
name: patch-preview-before-edit
description: Hiển thị diff thay đổi được đề xuất dạng unified diff và chờ xác nhận rõ ràng từ người dùng trước khi sửa đổi bất kỳ file nào trong repository. Dùng trước khi sửa source code, config, launch files, scripts, build files, docs. Kích hoạt khi người dùng muốn 'xem trước thay đổi', 'preview diff', 'show me what will change', hoặc khi làm việc với code nhạy cảm.
---

# Patch Preview Before Edit

## Mục tiêu

Trước khi thay đổi nội dung repository, hiển thị additions và deletions được đề xuất dạng diff, giải thích impact và risks, và chờ xác nhận rõ ràng trước khi apply bất kỳ thay đổi nào.

## Khi nào dùng

Dùng trước khi sửa source code, configuration, launch files, scripts, build files, documentation, generated templates, hoặc bất kỳ file nào trong repository.

## Rules

- **Không** edit, create, delete, rename, overwrite files trước khi user xác nhận
- **Không** chạy commands sửa đổi files, dependencies, caches, lockfiles trước khi xác nhận
- Read-only inspection commands được phép trước khi xác nhận
- Các lệnh formatting, code generation, dependency install, migration, autofix đều là modifying commands
- Preview phải đủ cụ thể để user hiểu change trước khi approve
- Nếu change quá rộng, chia thành patches nhỏ hơn hoặc hỏi clarification

**Từ xác nhận hợp lệ:** `confirm`, `apply`, `ok`, `yes`, `dong y`, `đồng ý`, `tiep tuc`, `tiếp tục`, `sửa đi`

## Quy trình

### Bước 1: Inspect

Dùng read-only commands và file reads để hiểu repository.

### Bước 2: Xác định files sẽ thay đổi

List từng file và lý do cần thay đổi.

### Bước 3: Chuẩn bị preview

Tạo diff preview mà không apply.

### Bước 4: Hiển thị preview theo format

Xem Preview Format bên dưới.

### Bước 5: Giải thích impact và risks

Nêu rõ expected impact, risks, assumptions, unverified details.

### Bước 6: Dừng và chờ

Stop và chờ explicit user confirmation.

### Bước 7: Apply sau khi xác nhận

Apply chỉ những changes đã được approve.

### Bước 8: Tóm tắt sau khi apply

Report changes đã thực hiện và validation results.

## Preview Format

````text
Proposed changes:
- <file path>: <short reason>
- <file path>: <short reason>

Preview:
```diff
diff --git a/<file path> b/<file path>
--- a/<file path>
+++ b/<file path>
@@
-<line to remove or replace>
+<line to add or replace>
```

Impact:
- <expected behavior or documentation change>

Risks:
- <risk, or "No major risk identified">

Assumptions:
- <verified fact or "No project-specific assumptions">

Waiting for confirmation before editing. Confirm with: confirm, apply, ok, yes, đồng ý, tiếp tục, or sửa đi.
````

## Ví dụ minh hoạ

**Ví dụ 1: Sửa launch file parameter**

Người dùng nói: "Đổi camera_fps từ 30 thành 60 trong launch file"

Hành động:
1. Đọc launch file, tìm parameter `camera_fps`
2. Hiển thị diff preview
3. Chờ xác nhận

Kết quả: Chỉ apply sau khi user nói "ok" hoặc "đồng ý".

**Ví dụ 2: Refactor nhiều files**

Người dùng nói: "Rename topic /camera/image thành /camera/image_raw trong toàn bộ codebase"

Hành động:
1. Grep tất cả files có string `/camera/image`
2. Chia thành 3 groups: source code, launch files, config files
3. Hiển thị từng group diff, giải thích risk (runtime mismatch nếu thiếu file nào)
4. Chờ xác nhận cho từng group hoặc tất cả

Kết quả: Có thể apply tuần tự từng group sau khi xác nhận.

## Khắc phục sự cố

**Lỗi:** User xác nhận nhưng thay đổi thực tế khác preview
**Nguyên nhân:** File đã bị thay đổi giữa lúc preview và apply
**Cách xử lý:** Dừng lại, hiển thị updated preview, chờ xác nhận lại

**Lỗi:** Diff quá lớn, user không đọc được
**Nguyên nhân:** Change scope quá rộng
**Cách xử lý:** Chia nhỏ thành từng file riêng biệt, hiển thị tuần tự

## After-Apply Summary

Sau khi apply, report:
- Files đã thay đổi
- Những gì đã add, remove, hoặc update
- Deviations so với approved preview (nếu có)
- Validation checks đã chạy và kết quả
- Checks chưa chạy và lý do
