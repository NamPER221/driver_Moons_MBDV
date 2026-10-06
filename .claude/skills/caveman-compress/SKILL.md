---
name: caveman-compress
description: Nén prose trong các file markdown như CLAUDE.md, PROJECT_CONTEXT.md, todos thành caveman-speak ngắn gọn để giảm ~46% input tokens, giữ nguyên toàn bộ kỹ thuật và code. Dùng khi người dùng gõ '/caveman-compress <file>', yêu cầu 'nén file này', 'giảm size của CLAUDE.md', 'compress memory file', hoặc context files quá dài gây tốn tokens.
---

# Caveman Compress

## Mục tiêu

Rewrite prose sections của markdown files thành terse nhất có thể, giữ nguyên mọi technical fact, code block, và structural element.

## Trigger

`/caveman-compress <filepath>` hoặc user yêu cầu compress file cụ thể.

## Quy trình

### Bước 1: Đọc file

Đọc file target, xác nhận là markdown/text (không phải code file).

### Bước 2: Tạo backup

Lưu bản gốc thành `<filename>.original.md` trước khi overwrite.

### Bước 3: Compress prose

Áp dụng rules bên dưới cho phần prose, giữ nguyên code blocks và technical content.

### Bước 4: Validate

Kiểm tra: không mất thông tin kỹ thuật, structure còn nguyên, không sửa code blocks.

## Rules Compression

**Xóa khỏi prose:**
- Articles (a, an, the)
- Filler words (really, basically, simply, just, very)
- Hedging (it seems, I think, có thể, có lẽ)
- Redundant phrases và connective fluff

**Compress bằng cách:**
- Short synonyms
- Sentence fragments thay câu đầy đủ
- Action-focused language
- Merge redundant points (giữ một example đại diện)

**Không bao giờ sửa:**
- Nội dung trong ` ``` ... ``` ` — copy EXACTLY
- Inline code trong backticks
- URLs, file paths, commands, technical terms
- Proper nouns, dates, version numbers, env variables
- Markdown headings, lists, tables, frontmatter

## Ví dụ minh hoạ

**Ví dụ 1: Compress PROJECT_CONTEXT.md**

User: "/caveman-compress PROJECT_CONTEXT.md"

Hành động:
1. Đọc file, tạo `PROJECT_CONTEXT.original.md`
2. Compress từng section prose
3. Giữ nguyên table structures và code examples
4. Output file nén với % reduction

Trước: "The project uses ROS 2 Humble as the primary middleware framework for inter-process communication between nodes."
Sau: "ROS 2 Humble. IPC between nodes."

**Ví dụ 2: Compress CLAUDE.md dài**

User: "CLAUDE.md của tôi quá dài, giảm size đi"

Kết quả: Compress ~46% prose, giữ nguyên commands, code examples, và technical constraints.

## Khắc phục sự cố

**Vấn đề:** Thông tin bị mất sau khi compress
**Cách xử lý:** Restore từ `.original.md` backup, tôi sẽ compress lại cẩn thận hơn

**Vấn đề:** File là code file (Python, YAML config)
**Cách xử lý:** Skill chỉ xử lý .md, .txt, .tex và extensionless docs. Từ chối code/config files.

## Note

Đầy đủ nhất với Python script từ caveman repo (validation + retry tự động). Không có script, compress trực tiếp bằng model theo rules này.

## Scope

Chỉ xử lý: `.md`, `.txt`, `.typ`, `.tex`, extensionless documents. Không sửa code files, config files, env files.
