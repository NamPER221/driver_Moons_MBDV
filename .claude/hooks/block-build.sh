#!/usr/bin/env bash
# PreToolUse hook for the manual-build-handoff skill.
#
# Blocks build / install / launch commands so Claude hands them to the user
# instead of running them. Exit code 2 = block the tool call and send stderr
# back to Claude as feedback.
#
# Bypass: append "# user-approved" to the command. Only do this when the user
# explicitly asked Claude to run that command.

set -u

payload=$(cat)

extract_command() {
  if command -v python3 >/dev/null 2>&1; then
    printf '%s' "$payload" | python3 -c \
      'import json,sys; print(json.load(sys.stdin).get("tool_input",{}).get("command",""))' 2>/dev/null
  elif command -v jq >/dev/null 2>&1; then
    printf '%s' "$payload" | jq -r '.tool_input.command // ""' 2>/dev/null
  else
    return 1
  fi
}

# No JSON parser available: match against the raw payload. Slightly over-broad
# (the description field is included), which fails safe toward blocking.
cmd=$(extract_command) || cmd=$payload

case "$cmd" in
  *"# user-approved"*) exit 0 ;;
esac

PATTERN='(colcon (build|test)|catkin_make|catkin build|cmake --build|(^|[;&|("] *)make( |$)|ninja|bazel build|cargo build|go build|docker build|docker run|docker compose up|pip[0-9.]* install|npm (install|run build)|yarn (install|build)|apt(-get)? install|rosdep install|roslaunch|ros2 (launch|run)|rosrun|rosbag |trtexec|dfu-util|openocd)'

if printf '%s' "$cmd" | grep -qE "$PATTERN"; then
  cat >&2 <<'EOF'
Blocked by the manual-build-handoff hook.

Do not run build / install / launch commands yourself. Print a handoff block
instead and stop:

  - working directory
  - environment prerequisites (source ...)
  - the exact copy-pasteable command
  - expected time and success signal
  - what the user should report back

Tip: suggest `2>&1 | tee /tmp/<name>.log` so you can read the result back from
the log file instead of asking the user to paste it.

If the user explicitly asked you to run this command, append "# user-approved"
to the end of the command to bypass this hook.
EOF
  exit 2
fi

exit 0
