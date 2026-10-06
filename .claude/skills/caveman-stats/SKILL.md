---
name: caveman-stats
description: Hiển thị thống kê token usage và estimated savings trong session hiện tại. Dùng khi người dùng gõ '/caveman-stats', hỏi 'tiết kiệm được bao nhiêu tokens', 'show token savings', 'token usage của session này', hoặc muốn đánh giá hiệu quả compression.
---

# Caveman Stats

## Mục tiêu

Hiển thị số liệu tokens thực tế đã dùng và ước tính savings trong session hiện tại.

## Trigger

`/caveman-stats` hoặc user hỏi về token usage stats.

## Behavior

Khi không có hook system (standalone skill), estimate session stats:

1. Đếm số exchanges có caveman mode active
2. Ước tính baseline tokens (không có compression) vs actual
3. Báo cáo savings percentage

## Output Format

```
Session stats:
- Caveman active: N exchanges
- Est. tokens used: ~X
- Est. baseline (no compression): ~Y
- Est. savings: ~Z% (~W tokens)
- Current mode: [lite/full/ultra/off]
```

## Ví dụ minh hoạ

**Ví dụ 1: Check sau session dài**

User: "/caveman-stats"

Kết quả:
```
Session stats:
- Caveman active: 12/15 exchanges
- Est. tokens used: ~4,200
- Est. baseline (no compression): ~12,000
- Est. savings: ~65% (~7,800 tokens)
- Current mode: full
```

## Khắc phục sự cố

**Vấn đề:** Số liệu không chính xác
**Nguyên nhân:** Standalone skill chỉ estimate, không đọc session log trực tiếp
**Cách xử lý:** Để có số liệu chính xác 100%, cần cài `hooks/caveman-stats.js` từ repo caveman gốc (juliusbrussee/caveman)

**Vấn đề:** Skill không biết caveman đã active bao lâu
**Cách xử lý:** Mention khi nào bật caveman để context rõ hơn

## Note

Đầy đủ nhất khi cài `hooks/caveman-stats.js` — đọc trực tiếp từ Claude Code session log, không cần estimate. Không có hook, con số là heuristic.

## Scope

Read-only display. Không thay đổi session state hay bật/tắt modes.
