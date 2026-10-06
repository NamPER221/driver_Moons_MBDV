---
name: edge-ai-deployment
description: Đánh giá và review quy trình triển khai AI model cho robot và thiết bị edge, bao gồm model format, runtime, accelerator, latency và memory constraints. Dùng khi người dùng yêu cầu 'deploy model lên edge', 'review TensorRT/ONNX deployment', 'kiểm tra GPU memory', 'optimize inference', hoặc gặp vấn đề với latency, packaging, power, hoặc compatibility.
---

# Edge AI Deployment

## Mục tiêu

Đánh giá xem AI model deployment path có coherent cho robot/edge system mục tiêu hay không, dựa trên facts đã xác minh từ repo và user.

## Khi nào dùng

Dùng cho TensorRT, ONNX Runtime, TFLite, OpenVINO, Core ML, CUDA, NPU, TPU, VPU, embedded Linux, container, cross-compile, hoặc robotics inference deployment.

## Quy trình

### Bước 1: Đọc context

Đọc `PROJECT_CONTEXT.md` và `MODEL_DEPLOYMENT_CONTEXT.md` nếu có.

### Bước 2: Khám phá artifacts

Discover từ repo: model files, conversion scripts, runtime wrappers, Dockerfiles, launch/config files, hardware references.

### Bước 3: Xác minh model contract

Kiểm tra: source format, deployment format, opset/version, precision, batch size, input/output shapes, preprocessing contract.

### Bước 4: Kiểm tra runtime và hardware

Chỉ xác nhận khi có evidence từ repo hoặc user: target runtime, accelerator, driver version.

### Bước 5: Kiểm tra ROS integration

- ROS 1: node/nodelet, topics, queue sizes, timestamps, frames, diagnostics
- ROS 2: node/component, QoS, lifecycle, timestamps, frames, diagnostics

### Bước 6: Đánh giá constraints

Latency, memory footprint, startup time, thermal/power, fallback behavior, model artifact versioning.

## Checklist

- [ ] Model path và version đã xác minh
- [ ] Runtime và hardware target đã xác minh hoặc ghi unknown
- [ ] Input/output tensor contract đã kiểm tra
- [ ] Pre/postprocessing consistency đã kiểm tra
- [ ] Deployment packaging và environment đã kiểm tra
- [ ] Unknowns ghi `Unknown / needs confirmation`

## Ví dụ minh hoạ

**Ví dụ 1: Review TensorRT deployment trên Jetson**

Người dùng nói: "Tôi có model ONNX muốn deploy lên Jetson Orin, cần review path deployment"

Hành động:
1. Tìm conversion script (thường là `convert.py` hoặc `build_engine.py`)
2. Xác minh: opset version, FP16/INT8, dynamic batch hay không, workspace size
3. Kiểm tra preprocessing trong ROS node có khớp với training preprocessing không
4. Kiểm tra memory budget: activation memory + weight size vs available GPU memory

Kết quả: Report deployment risks, recommended TRT flags, memory estimates.

**Ví dụ 2: Debug inference latency quá cao**

Người dùng nói: "Model inference mất 150ms nhưng cần dưới 50ms"

Hành động:
1. Xác minh precision: đang chạy FP32 thay vì FP16?
2. Kiểm tra batch size và dynamic shape overhead
3. Kiểm tra có CPU↔GPU memory copy không cần thiết không
4. Kiểm tra preprocessing có chạy trên CPU thay vì GPU không

Kết quả: Top bottlenecks với estimated speedup cho từng fix.

## Khắc phục sự cố

**Lỗi:** TensorRT engine build thất bại với "No implementation for layer"
**Nguyên nhân:** Custom op hoặc ONNX node không được TRT hỗ trợ
**Cách xử lý:** Tìm op trong ONNX graph, xem có TRT plugin không, hoặc fallback về ONNX Runtime

**Lỗi:** Accuracy giảm sau FP16 conversion
**Nguyên nhân:** Precision loss ở softmax, normalization, hoặc small activations
**Cách xử lý:** Thêm `precision_constraints` cho sensitive layers, giữ FP32 cho output head

**Lỗi:** OOM (Out of Memory) khi load engine
**Nguyên nhân:** Activation memory + weight vượt quá GPU memory
**Cách xử lý:** Giảm batch size, bật memory pool, hoặc dùng model nhỏ hơn

## Output Format

- Deployment summary: source → target format path
- Verified artifacts và runtime
- Contract risks: preprocessing mismatch, shape issues
- Integration risks: ROS topic, timing, frame
- Performance/resource risks: latency, memory, thermal
- Next checks: commands hoặc files cần xem xét
