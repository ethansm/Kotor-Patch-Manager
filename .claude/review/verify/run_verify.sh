#!/usr/bin/env bash
# Idempotent Verify runner: one locked-down Haiku 5.5 `claude -p` call per queued finding.
# Same preconditions, lockdown flags, rate-limit backoff and xargs waves as ../run_map.sh.
#
#   run_verify.sh --first-only      run the first pending finding alone (schema check + cost baseline)
#   run_verify.sh                   run all pending findings in waves of $PARALLEL (default 8)
#   run_verify.sh --only id[,id]    run just these findings (re-queue)
#   run_verify.sh --dry-run         render the first prompt, print pending ids and the exact command; no model call
#
# Input : work/verify_queue.json (verify/select.py). Output: $VERIFY_DIR/raw/<id>.aN.{out,err},
#         $VERIFY_DIR/results/<id>.json (+ .check.json from quote_check.py), work/verify_prompts/<id>.{txt,sys.md}.
# Env: PARALLEL (8), VERIFY_BUDGET_USD (0.12 per call), VERIFY_TIMEOUT_S (300),
#      VERIFY_TOTAL_BUDGET_USD (6, API-equivalent cap across all recorded verifier runs),
#      VERIFY_DIR (data dir, default this directory), VERIFY_WORK (default ../work), KOTOR2_REVIEW_HOME,
#      ALLOW_LAYER1_ONLY=1 (proceed when canary.json says deny rules failed but the tree is clean).
#      VERIFY_BACKOFF_SCALE_S (test hook: fixed sleep instead of the 30-60s+ rate-limit backoff).
# A finding is COMPLETE when results/<id>.json has status ok and a schema-valid output for that id.
# Workers write nothing to the repo; this script writes only under $VERIFY_DIR and the work dir.
set -euo pipefail

CODE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REVIEW_DIR="$(cd "$CODE_DIR/.." && pwd)"
RHOME="${KOTOR2_REVIEW_HOME:-$HOME/.cache/kotor2_review}"
TREE="$RHOME/worktree"
VERIFY_DIR="$(mkdir -p "${VERIFY_DIR:-$CODE_DIR}" && cd "${VERIFY_DIR:-$CODE_DIR}" && pwd)"
if [[ -n "${VERIFY_WORK:-}" ]]; then WORK="$VERIFY_WORK"
elif [[ "$VERIFY_DIR" == "$CODE_DIR" ]]; then WORK="$REVIEW_DIR/work"
else WORK="$VERIFY_DIR/work"; fi
PARALLEL="${PARALLEL:-8}"
VERIFY_BUDGET_USD="${VERIFY_BUDGET_USD:-0.12}"
VERIFY_TIMEOUT_S="${VERIFY_TIMEOUT_S:-300}"
VERIFY_TOTAL_BUDGET_USD="${VERIFY_TOTAL_BUDGET_USD:-6}"
MODEL="claude-haiku-5-5"
PY="python3 -I"
export VERIFY_DIR VERIFY_WORK="$WORK" KOTOR2_REVIEW_HOME="$RHOME"

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

# ---- preconditions (same as run_map.sh) ---------------------------------------------------------
[[ -f "$REVIEW_DIR/canary.json" ]] || { echo "run_verify: canary.json missing; run tools/canary_test.sh first" >&2; exit 1; }
VARIANT="$($PY - "$REVIEW_DIR/canary.json" <<'PYEOF'
import json, os, sys
c = json.load(open(sys.argv[1]))
if c.get("pass"):
    print(c.get("variant", "A"))
elif c.get("layer1_only") and os.environ.get("ALLOW_LAYER1_ONLY") == "1":
    print(c.get("variant", "A"))
else:
    print("FAIL")
PYEOF
)"
[[ "$VARIANT" != "FAIL" ]] || { echo "run_verify: canary.json does not record a pass; refusing" >&2; exit 1; }
$PY "$REVIEW_DIR/prep.py" verify-snapshot >/dev/null || { echo "run_verify: worker tree != snapshot.json; rebuild with prep.py build" >&2; exit 1; }
[[ -f "$REVIEW_DIR/worker_settings.json" ]] || { echo "run_verify: worker_settings.json missing" >&2; exit 1; }
[[ "$TREE" != "$(git -C "$REVIEW_DIR" rev-parse --show-toplevel 2>/dev/null)"* ]] || { echo "run_verify: tree inside repo" >&2; exit 1; }
for f in verify_worker_prompt.md verify_schema.json; do
  [[ -f "$CODE_DIR/$f" ]] || { echo "run_verify: $CODE_DIR/$f missing" >&2; exit 1; }
done
[[ -f "$WORK/verify_queue.json" ]] || { echo "run_verify: $WORK/verify_queue.json missing; run verify/select.py" >&2; exit 1; }

ISOLATION=(--restricted --safe-mode)
[[ "$VARIANT" == "B" ]] && ISOLATION=(--restricted)

mkdir -p "$VERIFY_DIR"/{raw,results} "$WORK/verify_prompts"
export REVIEW_DIR CODE_DIR TREE MODEL VERIFY_BUDGET_USD VERIFY_TIMEOUT_S PY WORK
export ISOLATION_STR="${ISOLATION[*]}"

run_one() {   # $1 = finding id
  local id="$1" attempt=0 rc=0 rl=0 started dur t0
  if $PY "$CODE_DIR/vcommon.py" is-complete "$id"; then echo "$id: already complete, skipped"; return 0; fi
  local prompt="$WORK/verify_prompts/$id.txt" sysp="$WORK/verify_prompts/$id.sys.md"
  $PY "$CODE_DIR/vcommon.py" render "$id" > "$prompt"
  $PY "$CODE_DIR/vcommon.py" system "$id" > "$sysp"
  started="$(date -u +%Y-%m-%dT%H:%M:%SZ)"; t0=$(date +%s)
  local out err
  while (( attempt < 3 )); do
    attempt=$((attempt + 1))
    out="$VERIFY_DIR/raw/$id.a$attempt.out"; err="$VERIFY_DIR/raw/$id.a$attempt.err"
    set +e
    # shellcheck disable=SC2086
    (cd "$TREE" && timeout "$VERIFY_TIMEOUT_S" claude -p --model "$MODEL" $ISOLATION_STR \
        --settings "$REVIEW_DIR/worker_settings.json" \
        --tools "Read,Grep,Glob" --permission-mode dontAsk \
        --max-budget-usd "$VERIFY_BUDGET_USD" \
        --json-schema "$(cat "$CODE_DIR/verify_schema.json")" \
        --strict-mcp-config --mcp-config '{"mcpServers":{}}' \
        --disable-slash-commands --no-session-persistence \
        --output-format stream-json --verbose \
        --append-system-prompt "$(cat "$sysp")" \
        < "$prompt" > "$out" 2> "$err")
    rc=$?
    set -e
    # only stderr and the final result event: tool results inside the stream can contain any text
    if (( rc != 0 )) && { cat "$err"; tail -n 1 "$out"; } 2>/dev/null | grep -qiE '(\b429\b|\b529\b|rate.?limit|overloaded)'; then
      rl=$((rl + 1))
      local wait=$(( (2 ** attempt) * 15 + RANDOM % 15 ))
      echo "$id: rate limited (attempt $attempt), backing off ${wait}s"
      sleep "${VERIFY_BACKOFF_SCALE_S:-$wait}"
      continue
    fi
    break
  done
  dur=$(( $(date +%s) - t0 ))
  $PY "$REVIEW_DIR/tools/record_verify.py" verify "$id" "$out" "$err" "$rc" "$attempt" "$rl" "$started" "$dur" "$prompt"
  $PY "$CODE_DIR/quote_check.py" "$id" || true
}
export -f run_one

spent() { $PY "$CODE_DIR/vcommon.py" spent verify; }

if [[ -n "$ONLY" ]]; then
  [[ "$ONLY" =~ ^[A-Za-z0-9_.-]+(,[A-Za-z0-9_.-]+)*$ ]] || { echo "run_verify: bad --only list" >&2; exit 2; }
  mapfile -t PENDING < <(tr ',' '\n' <<<"$ONLY")
else
  mapfile -t PENDING < <($PY "$CODE_DIR/vcommon.py" pending)
fi
echo "run_verify: ${#PENDING[@]} pending finding(s); variant $VARIANT; parallel $PARALLEL; per-call cap \$$VERIFY_BUDGET_USD; total cap \$$VERIFY_TOTAL_BUDGET_USD"
(( ${#PENDING[@]} )) || exit 0

if (( DRY )); then
  printf '%s\n' "${PENDING[@]}" | head -20
  $PY "$CODE_DIR/vcommon.py" render "${PENDING[0]}" > "$WORK/verify_prompts/${PENDING[0]}.txt"
  $PY "$CODE_DIR/vcommon.py" system "${PENDING[0]}" > "$WORK/verify_prompts/${PENDING[0]}.sys.md"
  echo "rendered: $WORK/verify_prompts/${PENDING[0]}.txt ($(wc -c < "$WORK/verify_prompts/${PENDING[0]}.txt") bytes), .sys.md ($(wc -c < "$WORK/verify_prompts/${PENDING[0]}.sys.md") bytes)"
  echo "spent so far: \$$(spent)"
  echo "cmd: (cd $TREE && timeout $VERIFY_TIMEOUT_S claude -p --model $MODEL $ISOLATION_STR --settings $REVIEW_DIR/worker_settings.json --tools Read,Grep,Glob --permission-mode dontAsk --max-budget-usd $VERIFY_BUDGET_USD --json-schema <verify_schema.json> --strict-mcp-config --mcp-config '{\"mcpServers\":{}}' --disable-slash-commands --no-session-persistence --output-format stream-json --verbose --append-system-prompt <verify_worker_prompt.md + specialty note> < work/verify_prompts/<id>.txt)"
  exit 0
fi

if (( FIRST_ONLY )); then
  run_one "${PENDING[0]}"
  $PY "$CODE_DIR/vcommon.py" is-complete "${PENDING[0]}" && echo "first finding OK: inspect results/${PENDING[0]}.json and .check.json, then run_verify.sh" \
    || { echo "first finding did not complete; inspect results/${PENDING[0]}.json" >&2; exit 1; }
  exit 0
fi

i=0
while (( i < ${#PENDING[@]} )); do
  s="$(spent)"
  if $PY -c "import sys; sys.exit(0 if float('$s') >= float('$VERIFY_TOTAL_BUDGET_USD') else 1)"; then
    echo "run_verify: total budget reached (\$$s >= \$$VERIFY_TOTAL_BUDGET_USD); stopping. Remaining stay pending." >&2
    break
  fi
  wave=("${PENDING[@]:i:PARALLEL}")
  echo "run_verify: wave of ${#wave[@]} (from #$i), spent so far \$$s"
  printf '%s\n' "${wave[@]}" | xargs -P "$PARALLEL" -I{} bash -c 'run_one "$1"' _ {} || true
  rl_total="$($PY "$CODE_DIR/vcommon.py" rl-hits "${wave[@]}")"
  if (( rl_total > 2 && PARALLEL > 2 )); then PARALLEL=$(( PARALLEL / 2 )); (( PARALLEL < 2 )) && PARALLEL=2; echo "run_verify: $rl_total rate-limit hits; PARALLEL -> $PARALLEL"; fi
  i=$(( i + ${#wave[@]} ))
done

echo "run_verify: done. Not complete (re-queue with --only):"
$PY "$CODE_DIR/vcommon.py" pending | paste -sd, - || true
