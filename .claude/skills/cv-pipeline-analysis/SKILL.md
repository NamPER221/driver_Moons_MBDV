---
name: cv-pipeline-analysis
description: Phân tích pipeline computer vision cho robot từ đầu vào camera đến đầu ra, bao gồm calibration, preprocessing, inference và postprocessing. Dùng khi người dùng yêu cầu 'phân tích camera pipeline', 'debug perception node', 'kiểm tra image encoding', 'trace CV pipeline', hoặc gặp vấn đề với object detection, segmentation, depth estimation, synchronization.
---

# CV Pipeline Analysis

## Mục tiêu

Trace và đánh giá toàn bộ computer vision pipeline trong repository từ sensor input đến outputs, sử dụng configuration và code đã được xác minh.

## Khi nào dùng

Dùng cho các vấn đề về camera ingestion, image processing, object detection, segmentation, tracking, depth, calibration, perception topics, inference nodes, hoặc visualization.

## Quy trình

### Bước 1: Đọc context

Đọc `PROJECT_CONTEXT.md` và `MODEL_DEPLOYMENT_CONTEXT.md` nếu có.

### Bước 2: Khám phá pipeline từ repo

Từ repo, discover:
- Camera topics, drivers, calibration files (camera_info, yaml, json)
- Image encodings (rgb8, bgr8, mono8, 16UC1)
- Frame IDs và synchronization policy

### Bước 3: Trace preprocessing

Kiểm tra từng bước: resize, crop, pad, color conversion, normalization, undistortion, rectification, batching.

### Bước 4: Trace inference stage

Xác minh: model/runtime, input/output shapes, thresholds, postprocessing, coordinate transforms.

### Bước 5: Trace outputs

Kiểm tra: topics, message types, frames, units, visualization markers, diagnostics, logs, saved artifacts.

### Bước 6: So sánh patterns

- ROS 1: image_transport, message_filters, CameraInfo
- ROS 2: image_transport, QoS, message_filters, composition

## Checklist

- [ ] Input image encoding và calibration đã xác minh
- [ ] Camera frame và optical frame usage đã kiểm tra
- [ ] Synchronization và timestamp policy đã kiểm tra
- [ ] Preprocessing khớp với model/algorithm contract
- [ ] Postprocessing units, frames, thresholds đã kiểm tra
- [ ] Unknowns ghi `Unknown / needs confirmation`

## Ví dụ minh hoạ

**Ví dụ 1: Debug detection node không ra kết quả**

Người dùng nói: "Object detection node chạy nhưng không publish gì"

Hành động:
1. Tìm node entry point, trace subscriber của `/image_raw` hoặc topic tương đương
2. Kiểm tra encoding: node expect `rgb8` nhưng camera publish `bgr8`?
3. Kiểm tra CameraInfo subscription và calibration loading
4. Trace inference call và threshold filter

Kết quả: Xác định mismatch encoding hoặc threshold quá cao, đề xuất fix cụ thể.

**Ví dụ 2: Phân tích độ trễ pipeline**

Người dùng nói: "Pipeline detection chạy chậm, cần biết bottleneck ở đâu"

Hành động:
1. Map toàn bộ pipeline: camera → preprocessing → inference → postprocessing → publish
2. Kiểm tra có image copy không cần thiết không
3. Kiểm tra preprocessing có resize trước inference không
4. Kiểm tra có GPU transfer overhead không

Kết quả: Pipeline map với estimated latency từng bước và top-3 optimization candidates.

## Khắc phục sự cố

**Lỗi:** Calibration không load được
**Nguyên nhân:** Path hardcode hoặc file không tồn tại ở runtime
**Cách xử lý:** Tìm calibration file path trong launch/config, xác minh file tồn tại, kiểm tra parameter load

**Lỗi:** Image encoding mismatch (CV_8UC3 vs bgr8)
**Nguyên nhân:** cv_bridge convert sai hoặc node giả định encoding khác với camera publish
**Cách xử lý:** Trace cv_bridge call, kiểm tra `encoding` parameter, so sánh với camera driver publish format

**Lỗi:** Synchronization drift giữa camera và LiDAR
**Nguyên nhân:** message_filters ApproximateTime slop quá nhỏ hoặc clock source khác nhau
**Cách xử lý:** Kiểm tra slop value trong code, xác minh cả hai sensor dùng cùng clock source

## Output Format

- Pipeline map: từng stage với input/output contracts
- Verified configuration: calibration, encoding, thresholds
- Contract checks: preprocessing ↔ model alignment
- Failure points: evidence từ file/line cụ thể
- Performance considerations: copy, resize, transfer overhead
- Next verification: commands hoặc tests cần chạy
