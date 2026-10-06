---
name: caveman-review
description: Review code cực ngắn gọn với format L<line>: severity problem. fix. — tối đa signal, zero filler. Dùng khi người dùng yêu cầu 'review code này', 'check PR', '/caveman-review', 'feedback ngắn gọn', hoặc muốn code review không có lời khen và hedging, chỉ có findings và fixes.
---

# Caveman Review

## Mục tiêu

Review code với maximum signal, zero filler. Mỗi finding đi kèm line number chính xác và fix cụ thể.

## Output Format

```
L<line>: [severity] <problem>. <fix>.
```

**Severity prefixes:**
- 🔴 bug — gây incorrect behavior
- 🟡 risk — tiềm ẩn vấn đề tùy context
- 🔵 nit — style hoặc minor improvement
- ❓ q — câu hỏi cần hỏi trước

## Rules

- Không hedging qualifiers ("I noticed", "you might want to consider", "có thể bạn nên")
- Không preamble ("Here's my review", "Tôi đã xem xét code của bạn")
- Không lời khen ("Great job on the overall structure")
- Luôn ghi: line number + symbol/variable + giải pháp cụ thể
- Code không bị sửa trong review output

## Ví dụ minh hoạ

**Ví dụ 1: Bug rõ ràng**

Bad: "I noticed that on line 42 you're not checking if the user object is null, which could potentially cause issues down the line."

Good: `L42: 🔴 bug: user null after .find(). Add guard before .email.`

**Ví dụ 2: Review robotics code**

```
L15: 🔴 bug: no encoding check on image_msg. Assert rgb8 or convert.
L31: 🟡 risk: queue_size=1 drops messages at 30fps. Increase to 10.
L48: 🔵 nit: hardcoded threshold 0.5. Move to ROS param.
L67: ❓ q: why lookup TF every callback? Cache transform.
```

## Khi nào expand

Viết đầy đủ hơn cho:
- CVE-level security vulnerabilities
- Architectural decisions cần rationale
- Onboarding context cho team member mới

Resume terse mode sau đó.

## Khắc phục sự cố

**Vấn đề:** Line number không chính xác trong review
**Cách xử lý:** Cung cấp file với line numbers rõ ràng, hoặc paste đoạn code cụ thể

**Vấn đề:** Muốn cả caveman-review VÀ robotics-code-review
**Cách xử lý:** Dùng `robotics-code-review` để tìm findings, format output theo `caveman-review` style

## Scope

Chỉ tạo review comments. Không viết fixes, không approve changes, không chạy linters.

## Deactivation

Nói "stop caveman-review" hoặc "normal mode".
