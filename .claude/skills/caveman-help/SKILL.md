---
name: caveman-help
description: Hiển thị bảng tham chiếu nhanh cho toàn bộ hệ thống caveman bao gồm modes, skills, lệnh, và cách tắt. Dùng khi người dùng gõ '/caveman-help', hỏi 'có những caveman mode nào', 'list lệnh caveman', 'quên cú pháp caveman', hoặc cần reminder về các tùy chọn compression.
---

# Caveman Help

## Mục tiêu

Hiển thị một lần (one-shot) bảng tham chiếu đầy đủ hệ thống caveman. Không thay đổi mode hay session state.

## Bảng Tham Chiếu Nhanh

### Modes

| Mode | Style | Savings |
|---|---|---|
| `lite` | Bỏ filler, giữ câu đầy đủ | ~30% |
| `full` | Fragments, bỏ articles (default) | ~60% |
| `ultra` | Telegraphic, cực ngắn | ~75% |
| `wenyan-lite` | Semi-classical Chinese | ~70% |
| `wenyan-full` | Classical Chinese tối đa | ~85% |

Bật: `/caveman [mode]` — default là `full`

### Skills

| Skill | Trigger | Chức năng |
|---|---|---|
| `caveman` | `/caveman [mode]` | Nén toàn bộ replies |
| `caveman-commit` | `/caveman-commit` | Commit messages ngắn |
| `caveman-review` | `/caveman-review` | Code review nén |
| `caveman-stats` | `/caveman-stats` | Token usage stats |
| `caveman-compress` | `/caveman-compress <file>` | Nén memory files |
| `cavecrew` | "dùng cavecrew" | Delegation subagents |
| `caveman-help` | `/caveman-help` | Card này |

### Tắt

Nói "stop caveman", "normal mode", hoặc "thôi caveman".
Resume bất cứ lúc nào với `/caveman`.

### Quy tắc bất biến

- Giữ nguyên ngôn ngữ của user
- Code, commands, error strings, technical terms: **không bao giờ** bị nén
- Security warnings và irreversible actions: tự động revert về normal

## Ví dụ minh hoạ

**Ví dụ: User quên cú pháp**

User: "/caveman-help"

Kết quả: Card này hiển thị một lần, session state không đổi.

## Scope

One-shot display. Không set hay thay đổi bất kỳ mode nào.
