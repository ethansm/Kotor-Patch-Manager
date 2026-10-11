#!/usr/bin/env bash
# F2: idempotent blind Reviewer runner. One locked-down read-only Haiku 5.5 `claude -p` call per patch that passed
# the deterministic checks. The reviewer sees the findings, the harness-made patch and a post-image excerpt; never the
# fixer's rationale/test_plan/notes. Calibration items (seeded-bad + known-good, fix_calibrate.py) are mixed into the
# review queue as ordinary tasks; fix_calibrate.py score reports catch / false-reject rates afterwards.
#
#   run_fix_review.sh --first-only     review the first pending patch alone
#   run_fix_review.sh                  review everything pending in chunks of $PARALLEL (default 8)
#   run_fix_review.sh --only T..,T..   just these tasks
#   run_fix_review.sh --dry-run        render the first prompt, print pending ids, the exact command, cost estimate
#
# Review of attempt N -> reviews/<tid>.aN.review.json (quote-checked against tasks/<tid>.aN.review by record_fix.py;
# a fabricated quote makes the review "invalid" and it is re-run once).
# Env: PARALLEL (8)  FIX_REVIEW_BUDGET_USD (0.12/call)  FIX_REVIEW_TOTAL_BUDGET_USD (3)  FIX_TIMEOUT_S (300)
#      FIX_CALIB=0 (skip calibration items)  FIX_SANDBOX FIX_DIR FIX_WORK KOTOR2_REVIEW_HOME FIX_CANARY_JSON
#      FIX_WORKSPACE_CMD FIX_SETTINGS_CMD FIX_BACKOFF_SCALE_S (test hooks).
set -euo pipefail

CODE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REVIEW_DIR="$(cd "$CODE_DIR/.." && pwd)"
RHOME="${KOTOR2_REVIEW_HOME:-$HOME/.cache/kotor2_review}"
FIX_HOME="$RHOME/fix"
FIX_DIR="$(mkdir -p "${FIX_DIR:-$CODE_DIR}" && cd "${FIX_DIR:-$CODE_DIR}" && pwd)"
if [[ -n "${FIX_WORK:-}" ]]; then WORK="$FIX_WORK"
elif [[ "$FIX_DIR" == "$CODE_DIR" ]]; then WORK="$REVIEW_DIR/work/fix"
else WORK="$FIX_DIR/work"; fi
PARALLEL="${PARALLEL:-8}"
REVIEW_BUDGET_USD="${FIX_REVIEW_BUDGET_USD:-0.12}"
REVIEW_TOTAL_BUDGET_USD="${FIX_REVIEW_TOTAL_BUDGET_USD:-3}"
FIX_TIMEOUT_S="${FIX_TIMEOUT_S:-300}"
MODEL="claude-haiku-5-5"
PY="python3 -I"
CANARY_JSON="${FIX_CANARY_JSON:-$CODE_DIR/canary.json}"
WS="${FIX_WORKSPACE_CMD:-$PY $CODE_DIR/fix_workspace.py}"
SET="${FIX_SETTINGS_CMD:-$PY $CODE_DIR/fix_settings.py}"
export FIX_DIR FIX_WORK="$WORK" KOTOR2_REVIEW_HOME="$RHOME"

FIRST_ONLY=0; DRY=0; ONLY=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    --first-only) FIRST_ONLY=1 ;;
    --dry-run) DRY=1 ;;
    --only) ONLY="${2:?--only needs ids}"; shift ;;
    *) echo "unknown arg $1" >&2; exit 2 ;;
  esac
  shift
done
if [[ -n "$ONLY" ]]; then
  [[ "$ONLY" =~ ^T[0-9a-f]{10}(,T[0-9a-f]{10})*$ ]] || { echo "run_fix_review: bad --only list" >&2; exit 2; }
fi

refuse() {
  if (( DRY )); then echo "run_fix_review: [dry-run] WOULD REFUSE: $1" >&2; else echo "run_fix_review: $1" >&2; exit 1; fi
}

# ---- preconditions (same as run_fix.sh) -----------------------------------------------------------------------
VARIANT="FAIL"
if [[ -f "$CANARY_JSON" ]]; then
VARIANT="$($PY - "$CANARY_JSON" <<'PYEOF'
import json, os, sys
c = json.load(open(sys.argv[1]))
if c.get("pass"):
    print(c.get("variant", "A"))
elif c.get("layer1_only") and os.environ.get("ALLOW_LAYER1_ONLY") == "1":
    print("B")
else:
    print("FAIL")
PYEOF
)"
fi
[[ "$VARIANT" != "FAIL" ]] || refuse "fix canary.json ($CANARY_JSON) missing or not a pass; run tools/fix_canary.sh first"
SANDBOX="${FIX_SANDBOX:-bwrap}"
if [[ "$VARIANT" == "B" ]]; then SANDBOX="none"; fi
if [[ "$SANDBOX" == "none" && "$VARIANT" != "B" ]] && [[ "$(command -v claude 2>/dev/null)" != */stub_claude/* ]]; then
  refuse "FIX_SANDBOX=none is only for the stub claude or canary variant B"
fi
[[ "$SANDBOX" == "bwrap" || "$SANDBOX" == "none" ]] || refuse "FIX_SANDBOX must be bwrap or none"
TOP="$(git -C "$CODE_DIR" rev-parse --show-toplevel 2>/dev/null || true)"
if [[ -n "$TOP" ]]; then case "$FIX_HOME/" in "$TOP/"*) refuse "FIX_HOME $FIX_HOME is inside the repo" ;; esac; fi
for f in reviewer_prompt.md review_schema.json fix_prompts.py record_fix.py fix_calibrate.py; do
  [[ -f "$CODE_DIR/$f" ]] || refuse "$CODE_DIR/$f missing"
done
[[ -f "$WORK/fix_queue.json" ]] || refuse "$WORK/fix_queue.json missing; run fix_intake.py"
if ! $WS verify-base >/dev/null 2>&1; then refuse "fix_workspace verify-base failed (base missing, dirty or holds an excluded path)"; fi

ISOLATION="--restricted --safe-mode"
mkdir -p "$FIX_DIR"/{raw,reviews,patches} "$WORK/prompts"
export CODE_DIR REVIEW_DIR FIX_HOME REVIEW_BUDGET_USD FIX_TIMEOUT_S MODEL PY WS SET WORK SANDBOX ISOLATION

# ---- calibration items + review queue --------------------------------------------------------------------------
QUEUE_FILE="$WORK/review_queue.json"
RWS="$WORK/review_ws"
export RWS
if (( ! DRY )); then
  if [[ "${FIX_CALIB:-1}" != "0" ]]; then
    $PY "$CODE_DIR/fix_calibrate.py" prepare || echo "run_fix_review: WARNING calibration items not (fully) prepared; continuing" >&2
  fi
  $PY "$CODE_DIR/fix_calibrate.py" queue >/dev/null || echo "run_fix_review: WARNING review queue not written" >&2
fi

call_claude() {   # <tid> <attempt> <tag> <settings> <sysfile> <promptfile> <copy>
  local tid="$1" n="$2" tag="$3" settings="$4" sysf="$5" prompt="$6" copy="$7"
  local tries=0 rl=0 sr=0 rc=0 out err wait
  CALL_RL=0; CALL_TRIES=0; CALL_RC=1
  while (( tries < 6 )); do
    tries=$((tries + 1))
    out="$FIX_DIR/raw/$tid.a$n.$tag.t$tries.out"; err="$FIX_DIR/raw/$tid.a$n.$tag.t$tries.err"
    local -a BW=()
    if [[ "$SANDBOX" == "bwrap" ]]; then
      mapfile -d '' -t BW < <(FIX_WORK="$RWS" $SET bwrap-argv claude "$copy" 2>/dev/null || true)
      if (( ${#BW[@]} == 0 )); then echo "$tid: bwrap-argv returned nothing; refusing to run unsandboxed" >&2; CALL_OUT="$out"; CALL_ERR="$err"; : > "$out"; : > "$err"; CALL_TRIES=$tries; return 1; fi
      if [[ "${BW[${#BW[@]}-1]}" == "--" ]]; then unset 'BW[${#BW[@]}-1]'; fi
      BW+=(--chdir "$copy" --)
      [[ "$(basename -- "${BW[0]}")" == "bwrap" ]] || BW=(bwrap "${BW[@]}")   # bwrap-argv prints args only
    fi
    set +e
    # shellcheck disable=SC2086
    (cd "$copy" && timeout "$FIX_TIMEOUT_S" "${BW[@]}" claude -p --model "$MODEL" $ISOLATION \
        --settings "$settings" \
        --tools "Read,Grep,Glob" --permission-mode dontAsk \
        --max-budget-usd "$REVIEW_BUDGET_USD" \
        --json-schema "$(cat "$CODE_DIR/review_schema.json")" \
        --strict-mcp-config --mcp-config '{"mcpServers":{}}' \
        --disable-slash-commands --no-session-persistence \
        --output-format stream-json --verbose \
        --append-system-prompt "$(cat "$sysf")" \
        < "$prompt" > "$out" 2> "$err")
    rc=$?
    set -e
    CALL_OUT="$out"; CALL_ERR="$err"; CALL_RC=$rc; CALL_TRIES=$tries
    if (( rc != 0 )) && { cat "$err"; tail -n 1 "$out"; } 2>/dev/null | grep -qiE '(\b429\b|\b529\b|rate.?limit|overloaded)'; then
      rl=$((rl + 1)); CALL_RL=$rl
      if (( rl >= 3 )); then break; fi
      wait=$(( (2 ** rl) * 15 + RANDOM % 15 ))
      echo "$tid: rate limited (try $tries), backing off ${wait}s" >&2
      sleep "${FIX_BACKOFF_SCALE_S:-$wait}"
      continue
    fi
    if (( sr == 0 )) && tail -n 1 "$out" 2>/dev/null | grep -q 'error_max_structured_output_retries'; then
      sr=1; echo "$tid: structured-output retries exhausted; one re-queue" >&2
      continue
    fi
    break
  done
  return 0
}

run_one() {   # $1 = task id
  local tid="$1" n pass copy prompt sysp settings started t0 dur
  n="$($PY "$CODE_DIR/record_fix.py" review-attempt "$tid")"
  [[ -n "$n" ]] || { echo "$tid: no checked patch to review"; return 0; }
  copy="$FIX_HOME/tasks/$tid.a$n.review"
  if [[ ! -d "$copy" ]]; then
    if ! ws make-review-copy "$tid" --attempt "$n" >/dev/null 2>"$FIX_DIR/raw/$tid.a$n.rcopy.err"; then
      echo "$tid: make-review-copy a$n failed (see raw/$tid.a$n.rcopy.err)" >&2; return 1
    fi
  fi
  for pass in 1 2; do
    [[ ! -e "$WORK/ABORT" ]] || return 3
    prompt="$WORK/prompts/$tid.a$n.review.txt"; sysp="$WORK/prompts/$tid.a$n.review.sys.md"
    $PY "$CODE_DIR/fix_prompts.py" reviewer "$tid" --attempt "$n" > "$prompt"
    $PY "$CODE_DIR/fix_prompts.py" reviewer-system "$tid" > "$sysp"
    settings="$(setc reviewer "$tid" --attempt "$n" | tail -n 1)"
    [[ -f "$settings" ]] || { echo "$tid: fix_settings produced no settings file ($settings)" >&2; return 1; }
    started="$(date -u +%Y-%m-%dT%H:%M:%SZ)"; t0=$(date +%s)
    call_claude "$tid" "$n" "rv$pass" "$settings" "$sysp" "$prompt" "$copy" || true
    dur=$(( $(date +%s) - t0 ))
    $PY "$CODE_DIR/record_fix.py" review "$tid.a$n" "$CALL_OUT" "$CALL_ERR" "$CALL_RC" "$CALL_TRIES" "$CALL_RL" "$started" "$dur" "$prompt"
    # complete (status ok) -> stop; invalid (fabricated quote / schema) -> one re-run; failed -> leave pending
    if [[ -z "$($PY "$CODE_DIR/record_fix.py" pending --stage review --only "$tid")" ]]; then break; fi
    $PY -c "import json,sys; sys.exit(0 if json.load(open('$FIX_DIR/reviews/$tid.a$n.review.json')).get('status')=='invalid' else 1)" || break
    echo "$tid: invalid review (pass $pass); re-queueing once" >&2
  done
}
ws() { FIX_WORK="$RWS" $WS "$@"; }       # review-stage helper calls see the merged queue (real + calibration tasks)
setc() { FIX_WORK="$RWS" $SET "$@"; }
export -f call_claude run_one ws setc

spent() { $PY "$CODE_DIR/fcommon.py" spent review; }
PARGS=(--stage review --queue-file "$QUEUE_FILE")
[[ -z "$ONLY" ]] || PARGS+=(--only "$ONLY")
mapfile -t PENDING < <($PY "$CODE_DIR/record_fix.py" pending "${PARGS[@]}")
echo "run_fix_review: ${#PENDING[@]} pending review(s); variant $VARIANT; sandbox $SANDBOX; parallel $PARALLEL; per-call cap \$$REVIEW_BUDGET_USD; total cap \$$REVIEW_TOTAL_BUDGET_USD"
(( ${#PENDING[@]} )) || exit 0

if (( DRY )); then
  printf '%s\n' "${PENDING[@]}" | head -20
  first="${PENDING[0]}"
  n="$($PY "$CODE_DIR/record_fix.py" review-attempt "$first")"
  $PY "$CODE_DIR/fix_prompts.py" reviewer "$first" --attempt "$n" > "$WORK/prompts/$first.a$n.review.txt"
  $PY "$CODE_DIR/fix_prompts.py" reviewer-system "$first" > "$WORK/prompts/$first.a$n.review.sys.md"
  echo "rendered: $WORK/prompts/$first.a$n.review.txt ($(wc -c < "$WORK/prompts/$first.a$n.review.txt") bytes), .sys.md ($(wc -c < "$WORK/prompts/$first.a$n.review.sys.md") bytes)"
  est="$($PY -c "print(round(${#PENDING[@]} * float('$REVIEW_BUDGET_USD'), 4))")"
  echo "cost: worst case \$$est for ${#PENDING[@]} review(s) (re-runs extra); already spent \$$(spent); total cap \$$REVIEW_TOTAL_BUDGET_USD"
  echo "cmd: (cd \$FIX_HOME/tasks/<tid>.a<N>.review && timeout $FIX_TIMEOUT_S $( [[ $SANDBOX == bwrap ]] && echo "bwrap <fix_settings.py bwrap-argv claude <copy>> --chdir <copy> --" ) claude -p --model $MODEL $ISOLATION --settings <fix_settings.py reviewer <tid> --attempt N> --tools Read,Grep,Glob --permission-mode dontAsk --max-budget-usd $REVIEW_BUDGET_USD --json-schema <review_schema.json> --strict-mcp-config --mcp-config '{\"mcpServers\":{}}' --disable-slash-commands --no-session-persistence --output-format stream-json --verbose --append-system-prompt <reviewer_prompt.md + reviewer specialty note> < $WORK/prompts/<tid>.a<N>.review.txt)"
  exit 0
fi

if (( FIRST_ONLY )); then
  run_one "${PENDING[0]}"
  [[ -z "$($PY "$CODE_DIR/record_fix.py" pending --stage review --only "${PENDING[0]}")" ]] \
    && echo "run_fix_review: first review OK: inspect reviews/${PENDING[0]}.a*.review.json, then run_fix_review.sh" \
    || { echo "first review did not complete" >&2; exit 1; }
  exit 0
fi

TRIED="$WORK/.review_tried"; : > "$TRIED"
while :; do
  s="$(spent)"
  if [[ -e "$WORK/ABORT" ]]; then echo "run_fix_review: ABORT flag set; stopping" >&2; exit 3; fi
  if $PY -c "import sys; sys.exit(0 if float('$s') >= float('$REVIEW_TOTAL_BUDGET_USD') else 1)"; then
    echo "run_fix_review: total budget reached (\$$s >= \$$REVIEW_TOTAL_BUDGET_USD); stopping. Remaining stay pending." >&2
    break
  fi
  mapfile -t CH < <($PY "$CODE_DIR/record_fix.py" pending "${PARGS[@]}" --exclude-file "$TRIED" --chunk "$PARALLEL")
  (( ${#CH[@]} )) || break
  printf '%s\n' "${CH[@]}" >> "$TRIED"
  echo "run_fix_review: chunk of ${#CH[@]} (parallel $PARALLEL), spent so far \$$s"
  t0=$(date +%s)
  printf '%s\n' "${CH[@]}" | xargs -P "$PARALLEL" -I{} bash -c 'run_one "$1"' _ {} || true
  rl_total="$($PY "$CODE_DIR/record_fix.py" rl-hits --since "$t0" "${CH[@]}")"
  if (( rl_total > 2 && PARALLEL > 2 )); then PARALLEL=$(( PARALLEL / 2 )); (( PARALLEL < 2 )) && PARALLEL=2; echo "run_fix_review: $rl_total rate-limit hits; PARALLEL -> $PARALLEL"; fi
done

echo "run_fix_review: done. Verdicts:"
$PY "$CODE_DIR/record_fix.py" summary review
echo "run_fix_review: calibration (when items exist):"
$PY "$CODE_DIR/fix_calibrate.py" score 2>/dev/null | head -30 || true
left="$($PY "$CODE_DIR/record_fix.py" pending "${PARGS[@]}" | paste -sd, - || true)"
if [[ -n "$left" ]]; then echo "run_fix_review: done. Not complete (re-queue with --only):"; echo "$left"; else echo "run_fix_review: done."; fi
