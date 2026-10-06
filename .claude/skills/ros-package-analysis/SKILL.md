---
name: ros-package-analysis
description: Phân tích cấu trúc ROS 1/ROS 2 package, manifests, dependencies, launch files, interfaces và integration points chỉ từ repository evidence. Dùng khi người dùng yêu cầu 'phân tích package', 'kiểm tra dependencies', 'map launch files', 'tìm publishers/subscribers của node X', hoặc gặp vấn đề build, dependency conflict, hoặc package boundary.
---

# ROS Package Analysis

## Mục tiêu

Lập bản đồ cấu trúc, dependencies, runtime interfaces, và integration risks của một ROS package chỉ sử dụng repository evidence.

## Khi nào dùng

Dùng khi inspect một package, dependency issue, launch path, migration risk, build problem, hoặc package boundary.

## Quy trình

### Bước 1: Đọc context

Đọc project context files nếu có.

### Bước 2: Locate package manifests

Tìm package manifests và build files.

### Bước 3: Inspect theo ROS version

**ROS 1:** `package.xml`, `CMakeLists.txt`, launch XML, `cfg`, `msg`, `srv`, `action`, nodelets, catkin conventions

**ROS 2:** `package.xml`, `CMakeLists.txt` hoặc Python packaging, launch files, `msg/srv/action`, ament exports, components, lifecycle, colcon conventions

### Bước 4: Trace entry points

Trace executables → publishers, subscribers, services, actions, parameters, TF, model/config assets.

### Bước 5: Verify dependencies

So sánh declared dependencies với code imports/includes và launch usage.

## Checklist

- [ ] Package role đã verify
- [ ] Build type và dependencies đã kiểm tra
- [ ] Runtime interfaces và parameters đã map
- [ ] Launch/config files đã link với executables
- [ ] Tests và CI coverage đã locate
- [ ] Unknowns mark `Unknown / needs confirmation`

## Ví dụ minh hoạ

**Ví dụ 1: Hiểu package trước khi sửa**

Người dùng nói: "Tôi cần sửa package `detection_ros2`, cho tôi biết structure của nó"

Hành động:
1. Đọc `package.xml`: build_type, dependencies
2. Xem `CMakeLists.txt`: executables và libraries được build
3. List launch files và nodes chúng launch
4. Trace main node: subscriptions, publications, services, parameters
5. Tìm test files

Kết quả: Package map với entry points, interfaces, parameter list, dependencies.

**Ví dụ 2: Debug build failure**

Người dùng nói: "colcon build báo 'package X not found'"

Hành động:
1. Đọc `package.xml` phần `<depend>` và `<exec_depend>`
2. Kiểm tra package X có trong workspace không
3. Kiểm tra `CMakeLists.txt` có `find_package(X REQUIRED)` không
4. Kiểm tra `package.xml` có declare dependency không
5. Xem `colcon.meta` hoặc workspace setup

Kết quả: Missing dependency root cause, fix commands.

## Khắc phục sự cố

**Lỗi:** Node launch nhưng không tìm được parameter
**Nguyên nhân:** Parameter namespace sai hoặc node name remapped
**Cách xử lý:** Trace launch file remaps, kiểm tra `~param` (private) vs `param` (global) namespace, verify `use_sim_time` nếu cần

**Lỗi:** `ament_target_dependencies` báo không tìm thấy package
**Nguyên nhân:** Package không trong `package.xml` hoặc chưa source workspace
**Cách xử lý:** Add vào `package.xml` `<depend>`, rebuild với `--packages-select` để isolate

## Output Format

- Package summary: role, build type, main executables
- Manifest/build findings: declared vs actual
- Runtime interfaces: publishers, subscribers, services, actions, params
- Launch/config map: launch files → nodes → remaps
- Dependency gaps hoặc risks
- Suggested next checks
