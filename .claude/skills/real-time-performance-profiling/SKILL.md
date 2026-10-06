---
name: real-time-performance-profiling
description: Phân tích bottleneck hiệu suất và rủi ro real-time trong pipeline robotics, CV, edge AI bao gồm latency, jitter, CPU/GPU load, callback contention, throughput. Dùng khi người dùng báo 'bỏ lỡ deadline', 'frame rate thấp', 'callback backlog', 'dropped messages', 'CPU/GPU quá tải', 'memory leak', 'startup chậm', hoặc cần optimize throughput.
---

# Real-Time Performance Profiling

## Mục tiêu

Xác định performance bottlenecks và determinism risks trong robotics, CV, edge AI pipelines sử dụng verified repo facts và measured evidence.

## Khi nào dùng

Dùng cho missed deadlines, low frame rate, control latency, callback backlog, dropped messages, high CPU/GPU load, memory growth, startup delays, hoặc thermal throttling.

## Quy trình

### Bước 1: Đọc context

Đọc project context files nếu có.

### Bước 2: Discover runtime structure

Từ repo: entry points, nodes, launch/config, topic rates, timers, callback structure, model runtime, target hardware.

### Bước 3: Xác định critical path

Define critical path và timing budget. Mark unknown budgets là `Unknown / needs confirmation`.

### Bước 4: Kiểm tra ROS-specific issues

**ROS 1:** callback queues, spinner type, queue sizes, nodelets, rosconsole timing, rosbag replay effects

**ROS 2:** executors, callback groups, QoS history depth, intra-process communication, lifecycle state, tracing, composition

### Bước 5: Inspect CV/AI hotspots

Image copies, encoding conversion, resize, synchronization, inference, postprocessing, GPU/accelerator transfer, memory allocation.

### Bước 6: Đề xuất measurement plan

Measurement commands phù hợp với repo hoặc clearly generic examples.

## Checklist

- [ ] Critical path đã xác định
- [ ] Rates, latencies, queue/QoS settings, resource use đã kiểm tra
- [ ] Blocking calls, locks, allocations, copies, thread contention đã xem xét
- [ ] Hardware, OS, accelerator assumptions đã xác minh hoặc mark unknown
- [ ] Measurement commands phù hợp với repo

## Ví dụ minh hoạ

**Ví dụ 1: Detection pipeline chỉ đạt 5 FPS thay vì 30 FPS**

Người dùng nói: "Detection chạy chậm hơn expected 6x, không biết bottleneck ở đâu"

Hành động:
1. Trace critical path: camera callback → preprocess → inference → postprocess → publish
2. Kiểm tra preprocessing: có resize từ 4K xuống 640 trên CPU không?
3. Kiểm tra inference: batch size, dynamic shape overhead
4. Kiểm tra có memory allocation trong hot path không (np.zeros() trong callback)
5. Đề xuất profiling: `ros2 topic hz`, `nvidia-smi dmon`, `perf stat`

Kết quả: Top 3 bottlenecks với estimated contribution và fix recommendations.

**Ví dụ 2: Control loop có jitter, robot behavior không smooth**

Người dùng nói: "Control command publish rate không ổn định, đôi khi skip cycles"

Hành động:
1. Kiểm tra executor type: SingleThreaded sẽ bị block bởi slow callbacks
2. Tìm shared callback group với heavy callbacks (sensor processing)
3. Kiểm tra timer period vs actual execution time
4. Kiểm tra có system-level jitter không (CPU governor, IRQ affinity)

Kết quả: Executor/callback group restructuring plan + OS-level recommendations.

## Khắc phục sự cố

**Lỗi:** Callback queue backup liên tục tăng
**Nguyên nhân:** Publisher rate nhanh hơn subscriber processing rate
**Cách xử lý:** Giảm queue size (drop old messages), tăng processing speed, hoặc giảm publisher rate

**Lỗi:** GPU utilization thấp mặc dù inference chậm
**Nguyên nhân:** CPU preprocessing là bottleneck hoặc CPU-GPU transfer quá nhiều
**Cách xử lý:** Move preprocessing lên GPU (CUDA/OpenCV CUDA), batch nhiều frames

**Lỗi:** Memory tăng dần theo thời gian (leak)
**Nguyên nhân:** Python objects không được free (circular references, cache không giới hạn)
**Cách xử lý:** Dùng `tracemalloc`, giới hạn cache size, kiểm tra callback capture

## Output Format

- Performance symptom
- Critical path map: từng stage với estimated time
- Evidence và unknowns
- Likely bottlenecks: ordered by impact
- Measurement plan: specific commands hoặc tools
- Optimization candidates với tradeoffs
