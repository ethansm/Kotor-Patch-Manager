#!/usr/bin/env bash
# test_agy_gate.sh - offline tests for agy_gate.sh. Every gate call runs stub_agy/agy;
# the real agy binary is never executed.
# Usage: bash .claude/agy_eval/test_agy_gate.sh
#   AGY_TEST_SCRATCH=<dir>  optional base for per-mode temp dirs (default: ${TMPDIR:-/tmp})
set -euo pipefail

HERE=$(cd -P -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
GATE="$HERE/agy_gate.sh"
STUB="$HERE/stub_agy/agy"
MODEL="stub-model-1"
export GATE STUB MODEL

# Drop gate knobs inherited from the caller so each mode starts from the documented defaults.
for knob in SWARM_AGY AGY_BIN AGY_GATE_STATE_DIR AGY_GATE_CWD AGY_MAX_CALLS \
  AGY_WEEKLY_RESERVE AGY_TIMEOUT AGY_BACKOFF_SCALE AGY_TRANSIENT_WAIT \
  AGY_QUOTA_PATTERN AGY_AUTH_PATTERN AGY_RATE_PATTERN AGY_RUN_ID STUB_AGY_MODE STUB_AGY_LOG; do
  unset "$knob"
done

PASS_N=0
FAIL_N=0
SKIP_N=0
MODE_FAIL=""
MODE_SKIP=""
GATE_RC=0
CASE_DIR=""

if [ -n "${AGY_TEST_SCRATCH:-}" ]; then
  mkdir -p -- "$AGY_TEST_SCRATCH"
  BASE=$(mktemp -d "$AGY_TEST_SCRATCH/agy_gate.XXXXXX")
else
  BASE=$(mktemp -d "${TMPDIR:-/tmp}/agy_gate.XXXXXX")
fi

cleanup() {
  if [ "$FAIL_N" -eq 0 ]; then
    chmod -R u+rwx -- "$BASE" 2>/dev/null || true
    rm -rf -- "$BASE"
  fi
}
trap cleanup EXIT

# ---- assertion helpers -----------------------------------------------------

add_fail() { MODE_FAIL="${MODE_FAIL:+$MODE_FAIL; }$1"; }

expect_eq() { [ "$2" = "$3" ] || add_fail "$1: got $2, want $3"; }

calls() { wc -l <"$1" | tr -d ' '; }

one_line() { [ "$(wc -l <"$1" | tr -d ' ')" = 1 ]; }

stderr_is() { [ "$(cat -- "$1")" = "$2" ]; }

# json_expr <file> <python expression over d>: file parses as JSON and expression is true
json_expr() {
  python3 -I -c 'import json, sys, time
d = json.load(open(sys.argv[1], encoding="utf-8"))
sys.exit(0 if eval(sys.argv[2]) else 1)' "$1" "$2"
}

# json_ok <file> <expr>: exactly one line, and the line satisfies the expression
json_ok() { one_line "$1" && json_expr "$1" "$2"; }

# forbidden_ok <stub.log>: no forbidden flag appears as a stub argv token (independent list)
forbidden_ok() {
  python3 -I -c 'import sys
banned = ["--dangerously-skip-permissions", "--add-dir", "--settings", "--yolo", "-i", "--prompt-interactive"]
for line in open(sys.argv[1], encoding="utf-8"):
    if " argv=" not in line:
        continue
    for tok in line.split(" argv=", 1)[1].rstrip("\n").split(" "):
        for flag in banned:
            if tok == flag or tok.startswith(flag + "="):
                sys.exit(1)
sys.exit(0)' "$1"
}

# cwd_matches <stub.log> <physical cwd>: every stub invocation ran in that directory
cwd_matches() {
  python3 -I -c 'import sys
want = "cwd=" + sys.argv[2] + " argv="
for line in open(sys.argv[1], encoding="utf-8"):
    if line.strip() and not line.startswith(want):
        sys.exit(1)
sys.exit(0)' "$1" "$2"
}

# ---- fixtures ----------------------------------------------------------------

# new_case <name>: fresh case dir with state/, empty cwd/, prompt, schema and empty stub log
new_case() {
  CASE_DIR=$(mktemp -d "$BASE/$1.XXXXXX")
  mkdir -p -- "$CASE_DIR/state" "$CASE_DIR/cwd"
  printf 'Reproduce the crash and report the verdict.\n' >"$CASE_DIR/prompt.txt"
  printf '{"type": "object", "required": ["verdict"]}\n' >"$CASE_DIR/schema.json"
  : >"$CASE_DIR/stub.log"
}

# gate_call <case-dir> <run-id> [KEY=VAL ...]: runs the gate with stub defaults; sets GATE_RC
gate_call() {
  local d=$1 rid=$2
  shift 2
  GATE_RC=0
  env AGY_BIN="$STUB" SWARM_AGY=on AGY_MAX_CALLS=10 AGY_WEEKLY_RESERVE=100 \
    AGY_BACKOFF_SCALE=0 AGY_TRANSIENT_WAIT=0 AGY_TIMEOUT=3 \
    AGY_GATE_STATE_DIR="$d/state" AGY_GATE_CWD="$d/cwd" AGY_RUN_ID="$rid" \
    STUB_AGY_LOG="$d/stub.log" \
    "$@" bash "$GATE" "$d/prompt.txt" "$d/schema.json" "$MODEL" \
    >"$d/stdout.$rid" 2>"$d/stderr.$rid" || GATE_RC=$?
}

# common <case-dir> <run-id> <total stub invocations>: checks shared by every mode
common() {
  local phys
  phys=$(cd -P -- "$1/cwd" && pwd -P)
  expect_eq "exit code" "$GATE_RC" 0
  forbidden_ok "$1/stub.log" || add_fail "stub received a forbidden flag"
  cwd_matches "$1/stub.log" "$phys" || add_fail "stub cwd differs from AGY_GATE_CWD"
  expect_eq "stub invocations" "$(calls "$1/stub.log")" "$3"
}

# expect_call <case-dir> <run-id> <stdout expr> <stderr line> <total stub invocations>
expect_call() {
  common "$1" "$2" "$5"
  if ! json_ok "$1/stdout.$2" "$3"; then
    add_fail "stdout wrong: $(head -c 160 -- "$1/stdout.$2" | tr '\n' ' ')"
  fi
  if ! stderr_is "$1/stderr.$2" "$4"; then
    add_fail "stderr wrong: $(head -c 160 -- "$1/stderr.$2" | tr '\n' ' ')"
  fi
}

# ---- modes -------------------------------------------------------------------

mode_success() {
  new_case success
  local d=$CASE_DIR rid=t-success
  gate_call "$d" "$rid" STUB_AGY_MODE=success
  expect_call "$d" "$rid" \
    'd["agy_verdict"] == {"verdict": "reproduced"} and d["agy_meta"]["class"] == "ok" and d["agy_meta"]["attempts"] == 1' \
    'agy_gate: ok ok' 1
  grep -q -F -e '--mode plan --sandbox --output-format json --json-schema ' "$d/stub.log" \
    || add_fail "argv shape differs from the spec"
  grep -q -F -e '--print-timeout 120' "$d/stub.log" || add_fail "print-timeout 120 missing"
}

mode_quota() {
  new_case quota
  local d=$CASE_DIR rid=t-quota
  gate_call "$d" "$rid" STUB_AGY_MODE=quota AGY_QUOTA_PATTERN=STUB_QUOTA_SIGNATURE
  expect_call "$d" "$rid" 'd == {"agy_skipped": "quota"}' 'agy_gate: quota quota' 1
  json_expr "$d/state/state.json" \
    'd["status"] == "cooldown" and abs(d["cooldown_until"] - time.time() - 43200) < 120' \
    || add_fail "state is not in a 12h quota cooldown"
}

mode_rate_limit() {
  new_case rate_limit
  local d=$CASE_DIR rid=t-rate_limit
  gate_call "$d" "$rid" STUB_AGY_MODE=rate_limit AGY_RATE_PATTERN=STUB_RATE_SIGNATURE
  expect_call "$d" "$rid" 'd == {"agy_skipped": "rate_limit"}' 'agy_gate: rate_limit rate_limit' 3
}

mode_auth() {
  new_case auth
  local d=$CASE_DIR rid=t-auth
  gate_call "$d" "$rid" STUB_AGY_MODE=auth AGY_AUTH_PATTERN=STUB_AUTH_SIGNATURE
  expect_call "$d" "$rid" 'd == {"agy_skipped": "auth"}' 'agy_gate: auth auth' 1
  json_expr "$d/state/state.json" 'd["status"] == "disabled"' || add_fail "state not disabled"
  gate_call "$d" "$rid" STUB_AGY_MODE=auth AGY_AUTH_PATTERN=STUB_AUTH_SIGNATURE
  expect_call "$d" "$rid" 'd == {"agy_skipped": "disabled"}' 'agy_gate: skip disabled' 1
}

mode_transient() {
  new_case transient
  local d=$CASE_DIR rid=t-transient
  # Counter moves per failed attempt: call 1 = 2 failures, call 2 hits 3 on its first
  # attempt and trips the 30 min cooldown, so call 3 is refused without an agy call.
  gate_call "$d" "$rid" STUB_AGY_MODE=transient
  expect_call "$d" "$rid" 'd == {"agy_skipped": "transient"}' 'agy_gate: transient transient' 2
  gate_call "$d" "$rid" STUB_AGY_MODE=transient
  expect_call "$d" "$rid" 'd == {"agy_skipped": "transient"}' 'agy_gate: transient transient' 3
  gate_call "$d" "$rid" STUB_AGY_MODE=transient
  expect_call "$d" "$rid" 'd == {"agy_skipped": "cooldown"}' 'agy_gate: skip cooldown' 3
  json_expr "$d/state/state.json" \
    'd["status"] == "cooldown" and abs(d["cooldown_until"] - time.time() - 1800) < 120' \
    || add_fail "no 30 min cooldown after repeated transient failures"
}

mode_hang() {
  new_case hang
  local d=$CASE_DIR rid=t-hang start end
  start=$(date +%s)
  gate_call "$d" "$rid" STUB_AGY_MODE=hang AGY_TIMEOUT=3
  end=$(date +%s)
  expect_call "$d" "$rid" 'd == {"agy_skipped": "timeout"}' 'agy_gate: timeout timeout' 2
  [ $((end - start)) -lt 20 ] || add_fail "gate call took $((end - start))s, limit is 20s"
}

mode_malformed() {
  new_case malformed
  local d=$CASE_DIR rid=t-malformed
  gate_call "$d" "$rid" STUB_AGY_MODE=malformed
  expect_call "$d" "$rid" 'd == {"agy_skipped": "invalid"}' 'agy_gate: invalid invalid' 1
}

mode_agy_missing() {
  new_case agy_missing
  local d=$CASE_DIR rid=t-agy_missing
  gate_call "$d" "$rid" AGY_BIN=/nonexistent/agy STUB_AGY_MODE=success
  expect_call "$d" "$rid" 'd == {"agy_skipped": "config"}' 'agy_gate: config config' 0
  json_expr "$d/state/state.json" 'd["status"] == "disabled"' || add_fail "state not disabled"
}

mode_concurrent() {
  new_case concurrent
  local d=$CASE_DIR i bad="" calls_week
  export -f gate_call
  seq 1 8 | xargs -P 4 -I{} bash -c 'gate_call "$0" "t-concurrent-$1" STUB_AGY_MODE=concurrent' "$d" {} \
    || add_fail "xargs pipeline failed"
  for i in 1 2 3 4 5 6 7 8; do
    json_ok "$d/stdout.t-concurrent-$i" 'd.get("agy_verdict") == {"verdict": "reproduced"}' || bad="$bad $i"
  done
  [ -z "$bad" ] || add_fail "no verdict for runs:$bad"
  json_expr "$d/state/state.json" 'True' || add_fail "state.json is not valid JSON"
  json_expr "$d/state/state.json" 'all(d["runs"].get("t-concurrent-%d" % i) == 1 for i in range(1, 9))' \
    || add_fail "per-run counters are not 1 each"
  calls_week=$(python3 -I -c 'import json, sys; print(json.load(open(sys.argv[1]))["calls_week"])' \
    "$d/state/state.json" 2>/dev/null || echo missing)
  expect_eq "calls_week vs stub invocations" "$calls_week" "$(calls "$d/stub.log")"
  expect_eq "stub invocations" "$(calls "$d/stub.log")" 8
}

mode_corrupt_state() {
  new_case corrupt_state
  local d=$CASE_DIR rid=t-corrupt_state
  printf 'garbage{{{ not json' >"$d/state/state.json"
  gate_call "$d" "$rid" STUB_AGY_MODE=success
  common "$d" "$rid" 1
  json_ok "$d/stdout.$rid" 'd.get("agy_verdict") == {"verdict": "reproduced"}' || add_fail "no verdict returned"
  grep -q -x -F 'agy_gate: state_corrupt' "$d/stderr.$rid" || add_fail "no state_corrupt on stderr"
  grep -q -x -F 'agy_gate: ok ok' "$d/stderr.$rid" || add_fail "no final ok line on stderr"
  ls -- "$d"/state/state.json.corrupt-* >/dev/null 2>&1 || add_fail "no state.json.corrupt-* file"
  json_expr "$d/state/state.json" 'True' || add_fail "state.json is not valid JSON after recovery"
}

mode_unwritable_state_dir() {
  if [ "$(id -u)" -eq 0 ]; then
    MODE_SKIP="running as root, so chmod 500 does not block writes"
    return 0
  fi
  new_case unwritable_state_dir
  local d=$CASE_DIR rid=t-unwritable_state_dir
  chmod 500 "$d/state"
  gate_call "$d" "$rid" STUB_AGY_MODE=success
  expect_call "$d" "$rid" 'd == {"agy_skipped": "state_unwritable"}' 'agy_gate: skip state_unwritable' 0
}

# ---- runner ------------------------------------------------------------------

run_mode() {  # run_mode <name> <function>
  MODE_FAIL=""
  MODE_SKIP=""
  "$2"
  if [ -n "$MODE_SKIP" ]; then
    SKIP_N=$((SKIP_N + 1))
    printf 'SKIP %s %s\n' "$1" "$MODE_SKIP"
  elif [ -n "$MODE_FAIL" ]; then
    FAIL_N=$((FAIL_N + 1))
    printf 'FAIL %s %s\n' "$1" "$MODE_FAIL"
  else
    PASS_N=$((PASS_N + 1))
    printf 'PASS %s\n' "$1"
  fi
}

run_mode success mode_success
run_mode quota mode_quota
run_mode rate_limit mode_rate_limit
run_mode auth mode_auth
run_mode transient mode_transient
run_mode hang mode_hang
run_mode malformed mode_malformed
run_mode agy_missing mode_agy_missing
run_mode concurrent mode_concurrent
run_mode corrupt_state mode_corrupt_state
run_mode unwritable_state_dir mode_unwritable_state_dir

printf 'summary: %d pass, %d fail, %d skip\n' "$PASS_N" "$FAIL_N" "$SKIP_N"
if [ "$FAIL_N" -ne 0 ]; then
  printf 'artifacts kept under %s\n' "$BASE"
fi
[ "$FAIL_N" -eq 0 ]
