# Robotics / CV / ROS Repository Guidance

This repository may contain robotics, ROS, ROS 2, computer vision, edge AI, embedded, simulation, or deployment code. Treat every project-specific detail as unknown until verified from this repository or from user-provided context.

## Project Context

- Read `PROJECT_CONTEXT.md` if present before making assumptions.
- Prefer verified repository facts over generic robotics assumptions.
- If `ROS_TOPIC_MAP.md`, `TF_FRAME_MAP.md`, or `MODEL_DEPLOYMENT_CONTEXT.md` exists, read the relevant file before analyzing related behavior.
- If a context file is missing or incomplete, use `Unknown / needs confirmation` for unverified facts.

## Discovery Rules

- Discover package names, node names, launch files, parameters, topics, services, actions, TF frames, model paths, robot names, hardware, and build/test commands from the target repo at usage time.
- Do not invent commands, topics, frames, model paths, hardware, calibration files, accelerators, middleware, operating systems, or deployment targets.
- Generic examples are allowed only when clearly marked as examples.
- Consider both ROS 1 and ROS 2 conventions where relevant.

## Robotics Engineering Checks

- Check message contracts, timestamps, coordinate frames, units, calibration, synchronization, QoS or queue settings, lifecycle behavior, launch parameters, and runtime configuration.
- Consider safety, watchdogs, failure modes, degraded sensor input, timing, resource limits, and hardware interaction boundaries.
- For CV and AI code, check image encodings, camera models, preprocessing, postprocessing, model input/output contracts, conversion precision, accelerator support, and runtime memory pressure.

## Change Discipline

- Keep edits scoped to the requested task.
- Do not rewrite unrelated launch, configuration, model, or hardware files.
- Preserve user changes and repository conventions.
- At the end, summarize changed files and verification performed.

## Edit Approval Workflow

- Before modifying source code, configs, launch files, scripts, build files, docs, templates, or other repository files, present the proposed additions and deletions in diff-style format.
- Explain the expected impact, risks, assumptions, and files that would change.
- Do not edit, create, delete, rename, overwrite, reformat, regenerate, or run modifying commands before explicit user confirmation.
- Read-only inspection commands are allowed before confirmation.
- Treat formatting, code generation, dependency installation, migration, autofix, cleanup, and build-system update commands as modifying commands when they can change files or state.
- Accepted confirmation words include `confirm`, `apply`, `ok`, `yes`, `dong y`, `đồng ý`, `tiep tuc`, `tiếp tục`, and `sửa đi`.
- After applying approved changes, summarize files changed, what changed, validation checks run, and checks not run.

## Build & Run Handoff Workflow

- Never execute build, compile, install, launch, train, flash, or long-running commands automatically — foreground or background (`run_in_background`, `&`, `nohup`, `tmux`, `screen`).
- This covers `colcon build`, `catkin_make`, `cmake --build`, `make`, `ninja`, `cargo build`, `pip install`, `npm install`, `docker build`, `docker run`, `roslaunch`, `ros2 launch`, `ros2 run`, `rosbag play/record`, `trtexec`, training scripts, firmware flashing, and any command that needs sudo, GPU, hardware, or more than ~10 seconds.
- Instead, print the exact command for the user to run in their own terminal: working directory, environment prerequisites, the copy-pasteable command, expected success signal, and what to report back. Then stop and wait.
- Read-only inspection commands (`ls`, `cat`, `grep`, `git status`, `git diff`, `ros2 pkg list`, `nvidia-smi`) may still be run directly.
- Suggest `2>&1 | tee /tmp/<name>.log` so the result can be read back from the log file instead of pasted.
- Do not continue writing code that assumes a build passed. Wait for the actual result.
- If the user explicitly asks Claude to run a build command, that override is allowed — run it in the foreground, never backgrounded.
- Full details in the `manual-build-handoff` skill.

## Output Style — Caveman Mode (Always On)

Caveman mode is **always active** in this project. Apply `full` level by default to every response.

**Drop:** articles (a/an/the), filler words (just, really, basically, simply), pleasantries (certainly, great question), hedging (I think, it seems, có lẽ)
**Keep:** all technical substance, code blocks, exact error strings, API names, acronyms, command flags
**Pattern:** `[thing] [action] [reason]. [next step].`
**Language:** match user's language, compress style only. Never announce mode.

Auto-revert to normal for: security warnings, irreversible action confirmations, multi-step sequences where brevity risks misunderstanding, and text produced by the `humanizer` skill (the rewritten prose is the deliverable — return it in full). Resume caveman after.

Override anytime: `/caveman lite` (softer) · `/caveman ultra` (harder) · `normal mode` (off)

## Input Compression — Caveman MCP

The `caveman` MCP server (`mcp__caveman__*`) compresses large payloads before they enter context. It does not run automatically — call it proactively:

- Output > ~200 lines or > ~10 KB (candump / `can_sniffer` logs, build logs, long test output, EDS/DCF dumps): pass through `caveman_compress` before analyzing.
- Uniform JSON arrays (e.g. decoded PDO/SDO records): use `caveman_toon_encode`.
- Need an exact detail that was dropped (exact error string, frame bytes, line number): `caveman_retrieve` with the `recovery_handle`. Never guess dropped values.
- Do not compress source code being edited, diffs being reviewed, or short outputs.
- `caveman_stats` reports savings for this session.

## Available Slash Commands

Use these slash commands for structured robotics engineering tasks:

- `/project-onboarding` — onboard into this repository and populate PROJECT_CONTEXT.md
- `/read-source-code` — trace source code behavior using repo evidence
- `/robotics-code-review` — review code for behavioral bugs, timing, and integration risks
- `/patch-preview-before-edit` — preview diffs before any file is modified
- `/manual-build-handoff` — hand build/run commands to the user instead of executing them
- `/ros-package-analysis` — analyze ROS package structure and interfaces
- `/ros-topic-debugging` — debug topic issues (publishers, subscribers, QoS, rates)
- `/tf-frame-debugging` — debug TF frame naming, transforms, and timing
- `/rosbag-analysis` — analyze ROS bag data for topics, timing, and sensor quality
- `/cv-pipeline-analysis` — trace CV pipelines from camera input to outputs
- `/edge-ai-deployment` — review model deployment path for edge devices
- `/model-conversion-debugging` — debug ONNX/TRT/TFLite/OpenVINO conversion issues
- `/real-time-performance-profiling` — profile latency, jitter, and resource usage
- `/repo-context-maintainer` — update PROJECT_CONTEXT.md with verified facts
- `/technical-document-reader` — extract engineering facts from technical documents
- `/humanizer` — rewrite AI-sounding prose (README, docs, commit/PR text) without changing what it says
- `/caveman [mode]` — adjust compression level (lite/full/ultra/wenyan)
- `/caveman-commit` — generate terse conventional commit message
- `/caveman-review` — ultra-compressed code review (L<line>: problem. fix.)
- `/caveman-stats` — show estimated token savings this session
- `/caveman-compress <file>` — compress a markdown memory file
- `/caveman-help` — show all caveman modes and commands
