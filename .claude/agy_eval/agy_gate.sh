#!/usr/bin/env bash
# agy_gate.sh - offline, unwired gate around the Antigravity `agy` CLI.
#
# Usage: agy_gate.sh <prompt-file> <schema-file> <model-id>
#
# Contract: always exits 0.
#   stdout: exactly one JSON line
#     ok:   {"agy_verdict": <structured_output>, "agy_meta": {"class": "ok", "attempts": n, "latency_s": x}}
#     skip: {"agy_skipped": "<reason>"}
#   stderr: last line "agy_gate: <class> <reason>" (a state_corrupt line may come first)
# Knobs: SWARM_AGY, AGY_BIN, AGY_GATE_STATE_DIR, AGY_GATE_CWD, AGY_MAX_CALLS,
#   AGY_WEEKLY_RESERVE, AGY_TIMEOUT, AGY_BACKOFF_SCALE, AGY_TRANSIENT_WAIT,
#   AGY_QUOTA_PATTERN, AGY_AUTH_PATTERN, AGY_RATE_PATTERN, AGY_RUN_ID.
set -euo pipefail

readonly KILL_AFTER_S=5
readonly PRINT_TIMEOUT_S=120
readonly MAX_RATE_ATTEMPTS=3
readonly MAX_TRANSIENT_ATTEMPTS=2
readonly FORBIDDEN_FLAGS=(--dangerously-skip-permissions --add-dir --settings --yolo -i --prompt-interactive)

EMITTED=0
RUN_DIR=""
STATE_LOCK=""
STATE_FILE=""
STARTED_US=0
ATTEMPT=0
OUT_FILE=""
PROMPT_TEXT=""
REQUIRED_JSON="[]"
ARGV=()

# Safety net: any unexpected failure still yields one skip line and exit 0.
on_exit() {
  if [ "$EMITTED" -eq 0 ]; then
    printf '{"agy_skipped": "internal"}\n'
    printf 'agy_gate: skip internal\n' >&2
  fi
  if [ -n "$RUN_DIR" ]; then
    rm -rf -- "$RUN_DIR" || true
  fi
  exit 0
}
trap on_exit EXIT

# Schema -> JSON array of top-level required keys. Exit non-zero on a bad schema.
IFS= read -r -d '' PY_SCHEMA <<'PY' || true
import json, sys
with open(sys.argv[1], encoding="utf-8") as fh:
    schema = json.load(fh)
if not isinstance(schema, dict):
    sys.exit(2)
required = schema.get("required", [])
if not isinstance(required, list) or not all(isinstance(k, str) for k in required):
    sys.exit(2)
print(json.dumps(required))
PY

# Envelope check: rc 0 output must carry structured_output with every required key.
IFS= read -r -d '' PY_VALIDATE <<'PY' || true
import json, sys
ok = False
try:
    required = json.loads(sys.argv[2])
    with open(sys.argv[1], encoding="utf-8") as fh:
        envelope = json.load(fh)
    verdict = envelope.get("structured_output") if isinstance(envelope, dict) else None
    ok = isinstance(verdict, dict) and all(k in verdict for k in required)
except Exception:
    ok = False
sys.exit(0 if ok else 1)
PY

# Success line: the structured_output object plus meta.
IFS= read -r -d '' PY_EMIT <<'PY' || true
import json, sys
with open(sys.argv[1], encoding="utf-8") as fh:
    verdict = json.load(fh)["structured_output"]
print(json.dumps({"agy_verdict": verdict,
                  "agy_meta": {"class": "ok", "attempts": int(sys.argv[2]),
                               "latency_s": float(sys.argv[3])}}))
PY

# State transactions (run under flock by state_op). Ops:
#   reserve <run_id> <max_calls> <weekly>  -> prints ok|disabled|cooldown|run_cap|weekly_reserve
#   record <class> <stderr_file>           -> prints trip|none (trip = transient cooldown set now)
IFS= read -r -d '' PY_STATE <<'PY' || true
import datetime, json, os, sys, time

op, state_path = sys.argv[1], sys.argv[2]
now = int(time.time())
iso = datetime.datetime.now(datetime.timezone.utc).isocalendar()
week_key = "%04d-W%02d" % (iso[0], iso[1])
ERR_CAP = 2048
RUNS_KEEP = 500


def fresh():
    return {"status": "ok", "cooldown_until": 0, "consecutive_transient": 0,
            "calls_week": 0, "week_key": week_key, "runs": {}, "last_error": ""}


def is_int(v):
    return isinstance(v, int) and not isinstance(v, bool)


def valid(d):
    return (isinstance(d, dict)
            and d.get("status") in ("ok", "cooldown", "disabled")
            and is_int(d.get("cooldown_until"))
            and is_int(d.get("consecutive_transient"))
            and is_int(d.get("calls_week"))
            and isinstance(d.get("week_key"), str)
            and isinstance(d.get("runs"), dict)
            and all(is_int(v) for v in d["runs"].values())
            and isinstance(d.get("last_error"), str))


def load():
    if not os.path.exists(state_path):
        return fresh()
    try:
        with open(state_path, encoding="utf-8") as fh:
            d = json.load(fh)
        if valid(d):
            return d
    except Exception:
        pass
    aside = "%s.corrupt-%d" % (state_path, now)
    n = 1
    while os.path.exists(aside):
        aside = "%s.corrupt-%d-%d" % (state_path, now, n)
        n += 1
    os.rename(state_path, aside)
    sys.stderr.write("agy_gate: state_corrupt\n")
    return fresh()


def save(d):
    tmp = "%s.tmp.%d" % (state_path, os.getpid())
    with open(tmp, "w", encoding="utf-8") as fh:
        json.dump(d, fh, indent=1, sort_keys=True)
        fh.write("\n")
    os.replace(tmp, state_path)


def reserve(d, run_id, max_calls, weekly):
    if d["week_key"] != week_key:
        d["week_key"], d["calls_week"] = week_key, 0
    if d["status"] == "disabled":
        return "disabled"
    if now < d["cooldown_until"]:
        return "cooldown"
    d["status"] = "ok"
    runs = d["runs"]
    if runs.get(run_id, 0) >= max_calls:
        return "run_cap"
    if d["calls_week"] + 1 > weekly:
        return "weekly_reserve"
    runs[run_id] = runs.pop(run_id, 0) + 1
    d["calls_week"] += 1
    while len(runs) > RUNS_KEEP:
        del runs[next(iter(runs))]
    return "ok"


def record(d, cls, err_bytes):
    d["last_error"] = err_bytes[:ERR_CAP].decode("utf-8", "replace")
    tripped = False
    if cls == "ok":
        d["consecutive_transient"] = 0
    elif cls == "quota":
        d["status"], d["cooldown_until"] = "cooldown", now + 43200
    elif cls in ("auth", "config"):
        d["status"] = "disabled"
    elif cls in ("timeout", "transient"):
        d["consecutive_transient"] += 1
        if d["consecutive_transient"] >= 3:
            d["status"], d["cooldown_until"] = "cooldown", now + 1800
            d["consecutive_transient"] = 0
            tripped = True
    return tripped


try:
    if op == "reserve":
        run_id, max_calls, weekly = sys.argv[3], int(sys.argv[4]), int(sys.argv[5])
        d = load()
        verdict = reserve(d, run_id, max_calls, weekly)
        save(d)
        print(verdict)
    elif op == "record":
        cls, err_path = sys.argv[3], sys.argv[4]
        with open(err_path, "rb") as fh:
            err_bytes = fh.read(ERR_CAP)
        d = load()
        tripped = record(d, cls, err_bytes)
        save(d)
        print("trip" if tripped else "none")
    else:
        sys.exit(3)
except Exception:
    sys.exit(3)
PY

abspath() {
  case $1 in
    /*) printf '%s\n' "$1" ;;
    *) printf '%s/%s\n' "$PWD" "$1" ;;
  esac
}

is_uint() { [[ $1 =~ ^[0-9]+$ ]]; }

# Empty pattern never matches. A pattern grep cannot compile (exit 2) is rejected.
valid_regex() {
  local rc=0
  if [ -z "$1" ]; then
    return 0
  fi
  printf '' | grep -E -e "$1" >/dev/null 2>&1 || rc=$?
  [ "$rc" -ne 2 ]
}

pattern_hit() {  # pattern_hit <ERE-or-empty> <file>...
  local pat=$1
  shift
  [ -n "$pat" ] || return 1
  grep -E -q -e "$pat" -- "$@"
}

# Missing, unreadable or non-empty directories all fail.
cwd_is_empty() {
  local listing
  [ -d "$1" ] || return 1
  listing=$(ls -A -- "$1" 2>/dev/null) || return 1
  [ -z "$listing" ]
}

# Returns 0 when any argv element is a forbidden flag (exact, or flag=value).
has_forbidden_flag() {
  local arg flag
  for arg in "$@"; do
    for flag in "${FORBIDDEN_FLAGS[@]}"; do
      case $arg in
        "$flag" | "$flag="*) return 0 ;;
      esac
    done
  done
  return 1
}

# The only place the agy argv is shaped. setsid -w waits for the child and returns its status.
build_argv() {  # build_argv <timeout_s> <agy_bin> <prompt> <model> <schema_abs>
  ARGV=(setsid -w timeout -k "$KILL_AFTER_S" "$1" "$2" -p "$3"
    --model "$4" --mode plan --sandbox --output-format json
    --json-schema "$5" --print-timeout "$PRINT_TIMEOUT_S")
}

# Serialises state read-modify-write through flock. The agy call itself is never under the lock.
state_op() {  # state_op <op> <op args...>
  (
    flock -x 9
    python3 -I -c "$PY_STATE" "$1" "$STATE_FILE" "${@:2}"
  ) 9>"$STATE_LOCK"
}

# classify <rc> <stdout-file> <stderr-file> -> prints one class
classify() {
  local rc=$1 out=$2 err=$3
  # Patterns read stderr only, plus stdout on failure: a verdict that happens to contain a signature word must not trip a cooldown.
  local -a diag=("$err")
  [ "$rc" -eq 0 ] || diag+=("$out")
  if [ "$rc" -eq 124 ] || [ "$rc" -eq 137 ]; then
    echo timeout
  elif pattern_hit "${AGY_QUOTA_PATTERN:-}" "${diag[@]}"; then
    echo quota
  elif pattern_hit "${AGY_AUTH_PATTERN:-}" "${diag[@]}"; then
    echo auth
  elif pattern_hit "${AGY_RATE_PATTERN:-}" "${diag[@]}"; then
    echo rate_limit
  elif [ "$rc" -eq 127 ] || [ "$rc" -eq 1 ]; then
    echo config
  elif [ "$rc" -eq 0 ]; then
    if python3 -I -c "$PY_VALIDATE" "$out" "$REQUIRED_JSON" 2>/dev/null; then
      echo ok
    else
      echo invalid
    fi
  else
    echo transient
  fi
}

finish_skip() {  # finish_skip <class> <reason>
  printf '{"agy_skipped": "%s"}\n' "$2"
  printf 'agy_gate: %s %s\n' "$1" "$2" >&2
  EMITTED=1
  exit 0
}

finish_ok() {
  local now_us elapsed_us latency
  now_us=${EPOCHREALTIME//[!0-9]/}
  elapsed_us=$((now_us - STARTED_US))
  printf -v latency '%d.%03d' $((elapsed_us / 1000000)) $(((elapsed_us % 1000000) / 1000))
  python3 -I -c "$PY_EMIT" "$OUT_FILE" "$ATTEMPT" "$latency"
  EMITTED=1
  printf 'agy_gate: ok ok\n' >&2
  exit 0
}

main() {
  STARTED_US=${EPOCHREALTIME//[!0-9]/}

  case ${SWARM_AGY:-off} in
    on | shadow) ;;
    *) finish_skip skip off ;;
  esac
  [ "$#" -eq 3 ] || finish_skip skip bad_args
  if ! command -v python3 >/dev/null 2>&1; then
    finish_skip skip config
  fi

  local prompt_file schema_file model_id
  prompt_file=$(abspath "$1")
  schema_file=$(abspath "$2")
  model_id=$3
  [[ $model_id =~ ^[A-Za-z0-9][A-Za-z0-9._:/-]*$ ]] || finish_skip skip bad_args
  if [ ! -f "$prompt_file" ] || [ ! -r "$prompt_file" ]; then
    finish_skip skip bad_args
  fi
  PROMPT_TEXT=$(cat -- "$prompt_file" 2>/dev/null) || finish_skip skip bad_args
  [ -n "$PROMPT_TEXT" ] || finish_skip skip bad_args
  REQUIRED_JSON=$(python3 -I -c "$PY_SCHEMA" "$schema_file" 2>/dev/null) || finish_skip skip bad_args

  local max_calls=${AGY_MAX_CALLS:-0} weekly=${AGY_WEEKLY_RESERVE:-0}
  local timeout_s=${AGY_TIMEOUT:-130} backoff=${AGY_BACKOFF_SCALE:-1}
  local transient_wait=${AGY_TRANSIENT_WAIT:-20} run_id=${AGY_RUN_ID:-default}
  local agy_bin=${AGY_BIN:-agy} n pat
  for n in "$max_calls" "$weekly" "$backoff" "$transient_wait"; do
    is_uint "$n" || finish_skip skip bad_args
  done
  [[ $timeout_s =~ ^[1-9][0-9]*$ ]] || finish_skip skip bad_args
  for pat in "${AGY_QUOTA_PATTERN:-}" "${AGY_AUTH_PATTERN:-}" "${AGY_RATE_PATTERN:-}"; do
    valid_regex "$pat" || finish_skip skip bad_args
  done
  case $agy_bin in
    */*) agy_bin=$(abspath "$agy_bin") ;;
  esac

  local state_dir cwd_dir probe
  state_dir=$(abspath "${AGY_GATE_STATE_DIR:-$HOME/.cache/agy_eval/gate}")
  cwd_dir=$(abspath "${AGY_GATE_CWD:-$HOME/.cache/agy_eval/cwd}")
  cwd_is_empty "$cwd_dir" || finish_skip skip cwd_not_empty

  mkdir -p -- "$state_dir" 2>/dev/null || finish_skip skip state_unwritable
  probe=$(mktemp "$state_dir/.probe.XXXXXX" 2>/dev/null) || finish_skip skip state_unwritable
  rm -f -- "$probe"
  RUN_DIR=$(mktemp -d "$state_dir/run.XXXXXX" 2>/dev/null) || finish_skip skip state_unwritable
  STATE_LOCK="$state_dir/state.json.lock"
  STATE_FILE="$state_dir/state.json"

  build_argv "$timeout_s" "$agy_bin" "$PROMPT_TEXT" "$model_id" "$schema_file"
  if has_forbidden_flag "${ARGV[@]}"; then
    finish_skip skip bad_argv
  fi
  cd -- "$cwd_dir"

  local cls="" tripped="" rc=0 verdict="" err_file=""
  while :; do
    verdict=$(state_op reserve "$run_id" "$max_calls" "$weekly") || verdict=state_error
    case $verdict in
      ok) ;;
      disabled | cooldown | run_cap | weekly_reserve) finish_skip "${cls:-skip}" "$verdict" ;;
      *) finish_skip "${cls:-skip}" state_unwritable ;;
    esac

    ATTEMPT=$((ATTEMPT + 1))
    OUT_FILE="$RUN_DIR/stdout.$ATTEMPT"
    err_file="$RUN_DIR/stderr.$ATTEMPT"
    set +e
    "${ARGV[@]}" >"$OUT_FILE" 2>"$err_file" </dev/null
    rc=$?
    set -e

    cls=$(classify "$rc" "$OUT_FILE" "$err_file")
    tripped=$(state_op record "$cls" "$err_file") || tripped=none

    case $cls in
      ok) finish_ok ;;
      rate_limit)
        if [ "$ATTEMPT" -lt "$MAX_RATE_ATTEMPTS" ]; then
          sleep "$(((2 ** ATTEMPT * 15 + RANDOM % 15) * backoff))"
        else
          finish_skip rate_limit rate_limit
        fi
        ;;
      timeout | transient)
        if [ "$ATTEMPT" -lt "$MAX_TRANSIENT_ATTEMPTS" ] && [ "$tripped" = none ]; then
          sleep "$transient_wait"
        else
          finish_skip "$cls" "$cls"
        fi
        ;;
      *) finish_skip "$cls" "$cls" ;;
    esac
  done
}

main "$@"
