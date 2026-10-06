---
name: project-onboarding
description: Khám phá và lập bản đồ cấu trúc repository robotics/ROS chưa biết, xác định stack, commands, interfaces và tạo PROJECT_CONTEXT.md với facts đã xác minh. Dùng khi người dùng mới bắt đầu với repo, yêu cầu 'onboard repo mới', 'tìm hiểu project này', 'map cấu trúc codebase', 'PROJECT_CONTEXT.md chưa có', hoặc bắt đầu làm việc sau khi clone repo.
---

# Project Onboarding

## Mục tiêu

Xây dựng bản đồ ban đầu đáng tin cậy của một repository robotics mà không giả định package names, topics, frames, hardware, models, hoặc commands.

## Khi nào dùng

Dùng khi bắt đầu làm việc trong một repository robotics, ROS, ROS 2, CV, simulation, embedded, hoặc edge AI mới.

## Quy trình

### Bước 1: Đọc context có sẵn

Đọc `CLAUDE.md` và `PROJECT_CONTEXT.md` nếu có.

### Bước 2: Khám phá top-level structure

List top-level files, detect: manifests, docs, launch/config folders, packages, tests, CI.

### Bước 3: Xác định ROS version

ROS 1 indicators: `package.xml`, `CMakeLists.txt`, `.launch`, `catkin`, `rosbag`, `nodelet`, `dynamic_reconfigure`

ROS 2 indicators: `package.xml`, `setup.py`, `setup.cfg`, `ament`, `colcon`, launch Python, components, lifecycle nodes, QoS config

### Bước 4: Locate CV/AI assets

Camera calibration, model files, conversion scripts, preprocessing code, deployment configs.

### Bước 5: Ghi vào PROJECT_CONTEXT.md

Chỉ ghi verified facts. Để `Unknown / needs confirmation` cho bất kỳ field nào chưa xác minh được.

## Checklist

- [ ] Repository role đã xác định từ docs hoặc manifests
- [ ] Build, test, lint, launch commands đã xác minh trước khi ghi
- [ ] Packages, nodes, topics, services, actions, frames, parameters, models đã discover từ files
- [ ] Hardware và sensor assumptions không được phỏng đoán
- [ ] Open questions đã liệt kê rõ ràng

## Ví dụ minh hoạ

**Ví dụ 1: Onboard repo ROS 2 mới**

Người dùng nói: "Tôi vừa clone repo này, bạn có thể giúp tôi hiểu cấu trúc không?"

Hành động:
1. `ls` top-level, tìm `package.xml`, `colcon.meta`, `setup.py`
2. Detect ROS 2 từ `ament_cmake` trong CMakeLists.txt
3. List tất cả packages, trace main nodes
4. Tìm launch files, extract topic remaps và parameters
5. Tạo/cập nhật PROJECT_CONTEXT.md

Kết quả: PROJECT_CONTEXT.md với stack, packages, main interfaces, build command, open questions.

**Ví dụ 2: Onboard repo CV/AI không có ROS**

Người dùng nói: "Project này có camera và detection, giúp tôi hiểu pipeline"

Hành động:
1. Tìm entry points (main.py, inference.py, pipeline.py)
2. Trace: camera source → preprocessing → model → postprocessing → output
3. Tìm model files (.pt, .onnx, .trt) và config files
4. Ghi vào PROJECT_CONTEXT.md phần CV And AI

Kết quả: CV pipeline map với verified paths, model format, open questions về hardware.

## Khắc phục sự cố

**Lỗi:** Không tìm được build command
**Nguyên nhân:** Project dùng custom build system hoặc Makefile không standard
**Cách xử lý:** Tìm `Makefile`, `justfile`, `taskfile.yml`, CI scripts (`.github/workflows/`), ghi `Unknown / needs confirmation` nếu vẫn không tìm được

**Lỗi:** Quá nhiều packages, không biết bắt đầu từ đâu
**Nguyên nhân:** Monorepo với nhiều components
**Cách xử lý:** Tìm top-level README hoặc workspace launch file, identify "main" package từ dependency graph

## Output Format

- Snapshot: repository purpose và tech stack
- Verified layout: key directories và manifests
- Runtime map: known nodes/interfaces/frames/models
- Commands: verified hoặc `Unknown / needs confirmation`
- Updated files: ghi rõ PROJECT_CONTEXT.md đã được update
- Open questions: danh sách ngắn gọn
