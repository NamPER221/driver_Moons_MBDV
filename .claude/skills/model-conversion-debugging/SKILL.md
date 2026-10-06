---
name: model-conversion-debugging
description: Debug lỗi chuyển đổi AI model giữa các format như ONNX, TensorRT, TFLite, OpenVINO, quantization trong khi bảo toàn input/output contracts. Dùng khi người dùng báo 'ONNX export thất bại', 'TensorRT build lỗi', 'accuracy giảm sau conversion', 'unsupported op', 'shape mismatch', 'quantization error', hoặc 'output drift'.
---

# Model Conversion Debugging

## Mục tiêu

Chẩn đoán conversion failures hoặc accuracy regressions giữa training/source models và deployment formats, không phỏng đoán target runtime details.

## Khi nào dùng

Dùng cho ONNX export, TensorRT engine build, TFLite conversion, OpenVINO IR, quantization, precision changes, unsupported ops, shape mismatches, hoặc output drift.

## Quy trình

### Bước 1: Đọc context

Đọc project và model deployment context nếu có.

### Bước 2: Khám phá artifacts

Discover từ repo: source model, conversion scripts, config files, sample inputs, validation scripts, deployment runtime.

### Bước 3: Xác minh input/output contract

Kiểm tra: input names, shapes, dtypes, color order, normalization, dynamic axes, batch size, output names.

### Bước 4: Kiểm tra compatibility

Opset/runtime compatibility, unsupported operators, plugins/custom ops, calibration data, quantization settings, precision changes.

### Bước 5: Kiểm tra ROS integration

Xác minh converted model assumptions khớp với node's preprocessing và postprocessing.

### Bước 6: So sánh outputs

So sánh source và converted outputs trên representative inputs khi commands và data đã được xác minh.

## Checklist

- [ ] Source và target formats đã xác minh
- [ ] Conversion command đã xác minh trước khi dùng
- [ ] Input/output contracts đã bảo toàn
- [ ] Pre/postprocessing alignment đã kiểm tra
- [ ] Numeric tolerance và representative data đã xem xét
- [ ] Unknowns ghi `Unknown / needs confirmation`

## Ví dụ minh hoạ

**Ví dụ 1: ONNX export thất bại với custom op**

Người dùng nói: "torch.onnx.export() báo lỗi: Unsupported op: Deformable Convolution"

Hành động:
1. Tìm op trong model code, xác minh đây là custom op hay standard op
2. Kiểm tra ONNX opset version (thường cần >= 17 cho deformable conv)
3. Xem có onnx-simplifier hoặc onnxruntime-extensions hỗ trợ không
4. Xem xét fallback: export partial model hoặc fuse op trước khi export

Kết quả: Nguyên nhân cụ thể và 2-3 giải pháp với tradeoffs.

**Ví dụ 2: Accuracy giảm sau INT8 quantization**

Người dùng nói: "mAP giảm từ 0.85 xuống 0.62 sau khi quantize INT8"

Hành động:
1. Kiểm tra calibration dataset có representative không (đủ ảnh diverse?)
2. Xác định layers nhạy cảm với quantization (thường là detection head, softmax)
3. Thử mixed precision: giữ FP16 cho output layers
4. So sánh per-layer activations giữa FP32 và INT8

Kết quả: Xác định layers cần giữ FP16, calibration recommendations.

## Khắc phục sự cố

**Lỗi:** "RuntimeError: Exporting the operator... is not supported"
**Nguyên nhân:** Torch op chưa có ONNX mapping
**Cách xử lý:** Upgrade opset version, dùng symbolic override, hoặc rewrite op bằng primitive ops

**Lỗi:** Output shape mismatch giữa PyTorch và ONNX Runtime
**Nguyên nhân:** Dynamic shape không được export đúng (batch dim, spatial dim)
**Cách xử lý:** Thêm `dynamic_axes` đúng trong export call, verify với `onnxruntime.InferenceSession`

**Lỗi:** TRT engine chạy đúng nhưng output khác PyTorch
**Nguyên nhân:** FP16 precision loss hoặc layer fusion thay đổi computation order
**Cách xử lý:** Disable FP16 cho specific layers, so sánh intermediate activations

## Output Format

- Conversion path: source → intermediate → target format
- Failure/regression symptom: error message hoặc metric drop
- Verified contracts: input/output spec
- Likely causes: ordered by evidence strength
- Validation plan: commands và test inputs
- Next fix: specific change với expected result
