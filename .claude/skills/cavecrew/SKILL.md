---
name: cavecrew
description: Framework giao việc cho 3 subagents chuyên biệt với output nén ~60% — investigator tìm code, builder sửa file, reviewer audit diffs. Dùng khi người dùng muốn 'tìm nơi function X được gọi', 'sửa nhỏ ở file này', 'review diff này', hoặc khi context budget đang cạn và cần delegate subtasks hiệu quả.
---

# Cavecrew

## Mục tiêu

Giao subtasks cho subagents chuyên biệt với output nhỏ hơn ~60% so với vanilla agents, phù hợp khi context budget hạn chế.

## Ba Agents

### cavecrew-investigator
Tìm code definitions, callers, usage patterns.

**Output contract:** `path:line — symbol — note`

**Dùng cho:** tìm function định nghĩa ở đâu, ai gọi nó, pattern xuất hiện ở đâu.

### cavecrew-builder
Sửa chính xác tối đa 2 files.

**Output contract:** `path:line-range — change` + verification status hoặc terminal codes (`too-big`, `ambiguous`, `done`)

**Dùng cho:** targeted changes khi scope hẹp và rõ ràng.

### cavecrew-reviewer
Audit diffs và branches để tìm defects.

**Output contract:** emoji-flagged findings theo file và line, với severity totals

**Dùng cho:** review diff sau khi thay đổi.

## Pattern Chuẩn

```
1. Spawn investigator → tìm target locations
2. Đưa paths cụ thể cho builder → apply changes
3. Chạy reviewer trên diff → verify kết quả
```

## Khi nào dùng Cavecrew vs Vanilla

| Cavecrew | Vanilla |
|---|---|
| Muốn findings ngắn gọn | Cần architectural commentary |
| Tối đa 2 files, scope hẹp | Xử lý 3+ files |
| Context budget hạn chế | Cần deep rationale |
| Pattern lặp lại nhiều lần | Phức tạp, nhiều unknowns |

## Ví dụ minh hoạ

**Ví dụ 1: Tìm và fix bug**

User: "Tìm chỗ nào gọi `process_image()` rồi thêm null check"

Hành động:
1. Spawn investigator: tìm tất cả callers của `process_image`
   → `src/detection.py:45 — process_image — called without null guard`
2. Spawn builder: thêm null check tại line 45
   → `src/detection.py:44-46 — added: if img is None: return — done`
3. Spawn reviewer: check diff
   → `L45: ✅ null guard added. No other callers affected.`

**Ví dụ 2: Investigate trước khi refactor**

User: "Tôi muốn rename topic `/camera/image` trước khi sửa"

Spawn investigator: "tìm tất cả chỗ dùng `/camera/image` trong codebase"
→ 3 files với exact locations, sẵn sàng cho builder

## Khắc phục sự cố

**Vấn đề:** Builder báo `too-big` — thay đổi quá lớn
**Cách xử lý:** Chia nhỏ task, giao từng phần cho builder riêng biệt

**Vấn đề:** Investigator trả về quá nhiều results
**Cách xử lý:** Làm hẹp query (thêm file path, function context) trước khi spawn lại

## Tradeoff

Output của cavecrew có thể "structured nhưng đôi khi terse đến mức cryptic". Dùng vanilla agents khi cần full explanation.
