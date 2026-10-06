---
name: manual-build-handoff
description: Cấm Claude tự chạy build, compile, install, launch, train, flash hoặc bất kỳ lệnh chạy ngầm nào. Thay vào đó Claude in ra lệnh chính xác để người dùng tự chạy trong terminal, dừng lại chờ, rồi phân tích kết quả người dùng báo về. Kích hoạt khi cần colcon build, catkin_make, cmake, make, npm/cargo/pip install, docker build, roslaunch, ros2 launch/run, rosbag play, trtexec, train script, flash firmware, hoặc khi người dùng nói 'đừng tự chạy', 'đưa lệnh tôi chạy', 'don't run it yourself', 'give me the command'.
---

# Manual Build Handoff

## Mục tiêu

Claude **không bao giờ** tự thực thi lệnh build/run. Claude chỉ soạn lệnh chính xác, bàn giao cho người dùng chạy trong terminal thật, dừng lại chờ, rồi đọc kết quả người dùng báo về để phân tích tiếp.

Lý do: build robotics/CV tốn nhiều phút, chiếm CPU/GPU, đụng phần cứng thật, ghi đè `install/`, `build/`, `devel/`, hoặc khởi động node điều khiển robot. Người dùng phải là người bấm nút.

## Khi nào dùng

Luôn luôn, trong mọi session của repo này. Không cần người dùng gọi tên.

## Rules

### Lệnh BỊ CẤM tự chạy

Claude **không** chạy, kể cả ở foreground lẫn background (`run_in_background`, `&`, `nohup`, `tmux`, `screen`):

| Nhóm | Ví dụ |
|---|---|
| Build ROS | `colcon build`, `catkin_make`, `catkin build`, `. install/setup.bash` kèm build |
| Build chung | `make`, `ninja`, `cmake --build`, `bazel build`, `cargo build`, `go build` |
| Package manager | `pip install`, `npm install`, `npm run build`, `apt install`, `rosdep install` |
| Container | `docker build`, `docker run`, `docker compose up` |
| Chạy robot | `roslaunch`, `ros2 launch`, `ros2 run`, `rosrun`, `rosbag play/record` |
| AI/Model | `train.py`, `trtexec`, `onnx export`, `tflite convert`, quantization script |
| Phần cứng | flash firmware, `dfu-util`, `openocd`, ghi SD card, `dd` |
| Test nặng | `colcon test`, `pytest` toàn repo, integration test cần hardware |
| Bất kỳ lệnh nào | chạy > ~10s, cần sudo, cần GPU, hoặc sửa file ngoài repo |

### Lệnh VẪN được tự chạy

Read-only inspection, chạy nhanh, không sửa state:

`ls`, `cat`, `head`, `tail`, `grep`, `rg`, `find`, `tree`, `wc`
`git status`, `git log`, `git diff`, `git show`, `git branch`
`ros2 pkg list`, `ros2 topic list`, `ros2 param list` (chỉ liệt kê, không echo lâu)
`python -c "import x; print(x.__version__)"`, `nvidia-smi`, `uname -a`, `pkg-config --modversion`

Nghi ngờ → coi như bị cấm, bàn giao cho người dùng.

### Quy tắc bàn giao

- Một lần bàn giao = **một lệnh** (hoặc một chuỗi lệnh liền mạch chạy một lượt). Không nhồi 5 lệnh rời rạc.
- Luôn nêu **thư mục làm việc** và **điều kiện trước** (`source` môi trường nào).
- Luôn nói rõ **cần báo lại gì** — full log, 30 dòng cuối, hay chỉ exit code.
- Sau khi in block bàn giao, **dừng hẳn**. Không đoán trước kết quả, không viết tiếp code dựa trên giả định build pass.
- Không hỏi "bạn có muốn tôi chạy không?" — mặc định là không chạy.

## Quy trình

### Bước 1: Xác định lệnh cần chạy

Discover lệnh build thật từ repo (`package.xml`, `CMakeLists.txt`, `Makefile`, `pyproject.toml`, `docker-compose.yml`, CI config), không bịa.

### Bước 2: Thu hẹp phạm vi lệnh

Ưu tiên lệnh nhỏ nhất đủ để kiểm chứng. Ví dụ `colcon build --packages-select my_pkg` thay vì build cả workspace.

### Bước 3: In block bàn giao

Theo đúng Handoff Format bên dưới.

### Bước 4: Dừng và chờ

Kết thúc lượt trả lời. Không gọi thêm tool chạy lệnh.

### Bước 5: Nhận kết quả

Người dùng dán output, hoặc chỉ nói `pass` / `fail`, hoặc trỏ tới file log.

### Bước 6: Phân tích

Đọc log, xác định lỗi thật (dòng error đầu tiên, không phải dòng cuối), đề xuất fix. Nếu cần chạy lại → quay lại Bước 3.

## Handoff Format

````text
## Lệnh cần chạy

Thư mục: <đường dẫn tuyệt đối>
Chuẩn bị: <source ... / export ... / hoặc "không cần">

```bash
<lệnh chính xác, copy-paste được>
```

Mục đích: <kiểm chứng cái gì>
Thời gian dự kiến: <~2 phút / vài giây / lâu>
Kỳ vọng: <dấu hiệu thành công, ví dụ "Summary: 3 packages finished">

Báo lại cho tôi: <full output / 30 dòng cuối / exit code / nội dung file log>

Đang chờ kết quả — tôi không tự chạy lệnh này.
````

## Mẹo: ghi log để Claude tự đọc

Thay vì dán output dài, cho lệnh ghi ra file rồi bảo Claude đọc file (đọc file là read-only, được phép):

```bash
colcon build --packages-select my_pkg 2>&1 | tee /tmp/build.log
```

Rồi chỉ cần nói: *"xong rồi, đọc /tmp/build.log"*.

Với lệnh dài, thêm exit code vào log:

```bash
{ colcon build --packages-select my_pkg; echo "EXIT=$?"; } 2>&1 | tee /tmp/build.log
```

## Ví dụ minh hoạ

**Ví dụ 1: Sửa xong code, cần build lại**

Người dùng nói: *"Sửa lỗi type mismatch trong `image_proc_node.cpp` rồi build lại"*

Hành động: Claude sửa file (theo `patch-preview-before-edit`), rồi **không** chạy `colcon build`, mà in:

````text
## Lệnh cần chạy

Thư mục: /home/user/ros2_ws
Chuẩn bị: source /opt/ros/humble/setup.bash

```bash
{ colcon build --packages-select image_proc --symlink-install; echo "EXIT=$?"; } 2>&1 | tee /tmp/build.log
```

Mục đích: xác nhận fix type mismatch compile được
Thời gian dự kiến: ~40 giây
Kỳ vọng: `Summary: 1 package finished`, `EXIT=0`

Báo lại cho tôi: nói "xong" và tôi đọc /tmp/build.log

Đang chờ kết quả — tôi không tự chạy lệnh này.
````

Kết quả: Người dùng chạy, báo `xong`, Claude đọc log và phân tích.

**Ví dụ 2: Cần chạy node để xem topic**

Người dùng nói: *"Chạy thử node xem có publish `/camera/image_raw` không"*

Hành động: Bàn giao **hai** lệnh cho **hai** terminal, đánh số rõ ràng:

````text
## Lệnh cần chạy

Terminal 1 — Thư mục: /home/user/ros2_ws · Chuẩn bị: source install/setup.bash

```bash
ros2 launch camera_driver camera.launch.py
```

Terminal 2 — Chuẩn bị: source install/setup.bash

```bash
ros2 topic hz /camera/image_raw
```

Mục đích: xác nhận node publish đúng topic và rate
Kỳ vọng: `average rate: ~30.0`

Báo lại cho tôi: 5 dòng đầu của Terminal 2, và bất kỳ error nào ở Terminal 1

Đang chờ kết quả — tôi không tự chạy lệnh này.
````

**Ví dụ 3: Người dùng chủ động yêu cầu Claude chạy**

Người dùng nói: *"Chạy `colcon build` giùm tôi đi"*

Hành động: Người dùng override rõ ràng → được phép chạy, nhưng chạy **foreground**, không `run_in_background`, và báo trước thời gian dự kiến. Skill này chỉ cấm chạy **tự động, ngầm, không ai yêu cầu**.

## Enforcement cứng

Skill là chỉ dẫn mềm — vẫn có xác suất Claude quên. Nên `install.sh` cài kèm một `PreToolUse` hook chặn ở tầng harness, không phụ thuộc model có nhớ hay không:

- `.claude/hooks/block-build.sh` — khớp regex danh sách lệnh cấm, exit code 2 = huỷ tool call và trả stderr về cho Claude đọc
- `.claude/settings.json` — đăng ký hook với `matcher: "Bash"`

Hook đọc `tool_input.command` từ JSON payload (dùng `python3`, fallback `jq`), nên chỉ khớp **lệnh thật**, không khớp nhầm phần `description`. Ví dụ `cat /tmp/build.log` với mô tả *"read log after colcon build"* vẫn được phép chạy.

Hook có hiệu lực sau khi **restart Claude Code**.

**Escape hatch:** thêm `# user-approved` vào cuối lệnh để bỏ qua hook. Chỉ dùng khi người dùng chủ động yêu cầu chạy (Ví dụ 3 ở trên):

```bash
colcon build --packages-select my_pkg  # user-approved
```

Marker hiện rõ trong lệnh nên người dùng luôn thấy được khi nào hook bị bypass.

**Tắt hook:** xoá entry `matcher: "Bash"` trỏ tới `block-build.sh` trong `.claude/settings.json`.

## Khắc phục sự cố

**Lỗi:** Claude vẫn tự chạy build
**Nguyên nhân:** Chưa restart Claude Code sau khi cài, hoặc lệnh nằm trong `permissions.allow` của `settings.json`
**Cách xử lý:** Restart Claude Code; kiểm tra `permissions.allow` không chứa `Bash(colcon build:*)`; xác nhận hook chạy được bằng `printf '{"tool_input":{"command":"colcon build"}}' | .claude/hooks/block-build.sh; echo $?` (phải in `2`)

**Lỗi:** Hook chặn nhầm lệnh vô hại
**Nguyên nhân:** Regex trong `block-build.sh` quá rộng so với repo cụ thể
**Cách xử lý:** Sửa `PATTERN` trong `.claude/hooks/block-build.sh`, hoặc thêm `# user-approved` cho lệnh đó

**Lỗi:** Lệnh bàn giao chạy lỗi "command not found"
**Nguyên nhân:** Thiếu bước `source` môi trường ROS/venv
**Cách xử lý:** Claude phải luôn ghi dòng `Chuẩn bị:` — nếu thiếu, yêu cầu bổ sung

**Lỗi:** Output build quá dài không dán được
**Nguyên nhân:** Build cả workspace
**Cách xử lý:** Dùng `tee /tmp/build.log` rồi bảo Claude đọc file; hoặc thu hẹp bằng `--packages-select`

**Lỗi:** Claude phân tích log cũ
**Nguyên nhân:** File log không bị ghi đè, hoặc người dùng chạy lệnh khác
**Cách xử lý:** Đặt tên log theo lần chạy (`/tmp/build-2.log`), hoặc `rm` log trước khi chạy lại
