#!/usr/bin/env bash
# Idempotent Map runner: one locked-down Haiku 5.5 `claude -p` call per pending packet.
#
#   run_map.sh --first-only      run the first pending packet alone (schema check + resize baseline)
# All calls use --output-format stream-json --verbose so per-request usage (exact >100K count) is saved.
#   run_map.sh                   run all pending packets in waves of $PARALLEL (default 8)
#   run_map.sh --only id[,id]    run just these packets (re-queue)
#   run_map.sh --dry-run         print what would run and the exact command; no model call
#
# Env: PARALLEL (8), WORKER_BUDGET_USD (0.30 per call), WORKER_TIMEOUT_S (900),
#      MAP_TOTAL_BUDGET_USD (15, API-equivalent estimate cap across all runs), KOTOR2_REVIEW_HOME,
#      ALLOW_LAYER1_ONLY=1 (proceed when canary.json says deny rules failed but the tree is clean).
# Refuses to start unless canary.json records a pass and the worker tree matches snapshot.json.
# Workers write nothing to the repo; this script writes only .claude/review/{raw,runs,findings,work/prompts}.
set -euo pipefail

REVIEW_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RHOME="${KOTOR2_REVIEW_HOME:-$HOME/.cache/kotor2_review}"
TREE="$RHOME/worktree"
PARALLEL="${PARALLEL:-8}"
WORKER_BUDGET_USD="${WORKER_BUDGET_USD:-0.30}"
WORKER_TIMEOUT_S="${WORKER_TIMEOUT_S:-900}"
MAP_TOTAL_BUDGET_USD="${MAP_TOTAL_BUDGET_USD:-15}"
MODEL="claude-haiku-5-5"
PY="python3 -I"

FIRST_ONLY=0; DRY=0; ONLY=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    --first-only) FIRST_ONLY=1 ;;
    --dry-run) DRY=1 ;;
    --only) ONLY="$2"; shift ;;
    *) echo "unknown arg $1" >&2; exit 2 ;;
  esac
  shift
done

# ---- preconditions ---------------------------------------------------------------------------
[[ -f "$REVIEW_DIR/canary.json" ]] || { echo "run_map: canary.json missing; run tools/canary_test.sh first" >&2; exit 1; }
VARIANT="$($PY - "$REVIEW_DIR/canary.json" <<'EOF'
import json, os, sys
c = json.load(open(sys.argv[1]))
if c.get("pass"):
    print(c.get("variant", "A"))
elif c.get("layer1_only") and os.environ.get("ALLOW_LAYER1_ONLY") == "1":
    print(c.get("variant", "A"))
else:
    print("FAIL")
EOF
)"
[[ "$VARIANT" != "FAIL" ]] || { echo "run_map: canary.json does not record a pass; refusing" >&2; exit 1; }
$PY "$REVIEW_DIR/prep.py" verify-snapshot >/dev/null || { echo "run_map: worker tree != snapshot.json; rebuild with prep.py build" >&2; exit 1; }
[[ -f "$REVIEW_DIR/worker_settings.json" ]] || { echo "run_map: worker_settings.json missing" >&2; exit 1; }
[[ "$TREE" != "$(git -C "$REVIEW_DIR" rev-parse --show-toplevel 2>/dev/null)"* ]] || { echo "run_map: tree inside repo" >&2; exit 1; }

ISOLATION=(--restricted --safe-mode)
[[ "$VARIANT" == "B" ]] && ISOLATION=(--restricted)

mkdir -p "$REVIEW_DIR"/{raw,runs,findings,work/prompts}
export REVIEW_DIR TREE MODEL WORKER_BUDGET_USD WORKER_TIMEOUT_S PY
export ISOLATION_STR="${ISOLATION[*]}"

run_one() {   # $1 = packet id, $2 = output format (json|stream-json)
  local id="$1" fmt="${2:-stream-json}" attempt=0 rc=0 rl=0 started dur t0
  if $PY "$REVIEW_DIR/prep.py" is-complete "$id"; then echo "$id: already complete, skipped"; return 0; fi
  local prompt="$REVIEW_DIR/work/prompts/$id.txt"
  $PY "$REVIEW_DIR/prep.py" render "$id" > "$prompt"
  local extra=(); [[ "$fmt" == "stream-json" ]] && extra=(--verbose)
  started="$(date -u +%Y-%m-%dT%H:%M:%SZ)"; t0=$(date +%s)
  local out err
  while (( attempt < 3 )); do
    attempt=$((attempt + 1))
    out="$REVIEW_DIR/raw/$id.a$attempt.out"; err="$REVIEW_DIR/raw/$id.a$attempt.err"
    set +e
    # shellcheck disable=SC2086
    (cd "$TREE" && timeout "$WORKER_TIMEOUT_S" claude -p --model "$MODEL" $ISOLATION_STR \
        --settings "$REVIEW_DIR/worker_settings.json" \
        --tools "Read,Grep,Glob" --permission-mode dontAsk \
        --max-budget-usd "$WORKER_BUDGET_USD" \
        --json-schema "$(cat "$REVIEW_DIR/worker_schema.json")" \
        --strict-mcp-config --mcp-config '{"mcpServers":{}}' \
        --disable-slash-commands --no-session-persistence \
        --output-format "$fmt" "${extra[@]}" \
        --append-system-prompt "$(cat "$REVIEW_DIR/worker_system_prompt.md")" \
        < "$prompt" > "$out" 2> "$err")
    rc=$?
    set -e
    # only stderr and the final result event: tool results inside the stream can contain any text
    if (( rc != 0 )) && { cat "$err"; tail -n 1 "$out"; } 2>/dev/null | grep -qiE '(\b429\b|\b529\b|rate.?limit|overloaded)'; then
      rl=$((rl + 1))
      local wait=$(( (2 ** attempt) * 15 + RANDOM % 15 ))
      echo "$id: rate limited (attempt $attempt), backing off ${wait}s"
      sleep "$wait"
      continue
    fi
    break
  done
  dur=$(( $(date +%s) - t0 ))
  $PY "$REVIEW_DIR/tools/record_run.py" "$id" "$out" "$err" "$rc" "$attempt" "$rl" "$started" "$dur" "$prompt"
}
export -f run_one

spent() {
  $PY - "$REVIEW_DIR/runs" <<'EOF'
import json, pathlib, sys
print(round(sum((json.loads(p.read_text()).get("cost_usd") or 0) for p in pathlib.Path(sys.argv[1]).glob("wp*.json")), 4))
EOF
}

mapfile -t PENDING < <(if [[ -n "$ONLY" ]]; then tr ',' '\n' <<<"$ONLY"; else $PY "$REVIEW_DIR/prep.py" pending; fi)
echo "run_map: ${#PENDING[@]} pending packet(s); variant $VARIANT; parallel $PARALLEL; per-call cap \$$WORKER_BUDGET_USD"
(( ${#PENDING[@]} )) || exit 0

if (( DRY )); then
  printf '%s\n' "${PENDING[@]}" | head -20
  echo "cmd: (cd $TREE && timeout $WORKER_TIMEOUT_S claude -p --model $MODEL $ISOLATION_STR --settings $REVIEW_DIR/worker_settings.json --tools Read,Grep,Glob --permission-mode dontAsk --max-budget-usd $WORKER_BUDGET_USD --json-schema <worker_schema.json> --strict-mcp-config --mcp-config '{\"mcpServers\":{}}' --disable-slash-commands --no-session-persistence --output-format stream-json --verbose --append-system-prompt <worker_system_prompt.md> < work/prompts/<id>.txt)"
  exit 0
fi

if (( FIRST_ONLY )); then
  run_one "${PENDING[0]}"
  $PY "$REVIEW_DIR/prep.py" is-complete "${PENDING[0]}" && echo "first packet OK; next: prep.py resize --from runs/${PENDING[0]}.json, then run_map.sh" \
    || { echo "first packet did not complete; inspect runs/${PENDING[0]}.json" >&2; exit 1; }
  exit 0
fi

i=0
while (( i < ${#PENDING[@]} )); do
  s="$(spent)"
  if $PY -c "import sys; sys.exit(0 if float('$s') >= float('$MAP_TOTAL_BUDGET_USD') else 1)"; then
    echo "run_map: total budget reached (\$$s >= \$$MAP_TOTAL_BUDGET_USD); stopping. Remaining stay pending." >&2
    break
  fi
  wave=("${PENDING[@]:i:PARALLEL}")
  echo "run_map: wave of ${#wave[@]} (from #$i), spent so far \$$s"
  printf '%s\n' "${wave[@]}" | xargs -P "$PARALLEL" -I{} bash -c 'run_one "$1"' _ {} || true
  rl_total="$($PY - "$REVIEW_DIR/runs" "${wave[@]}" <<'EOF'
import json, pathlib, sys
d = pathlib.Path(sys.argv[1]); t = 0
for pid in sys.argv[2:]:
    f = d / f"{pid}.json"
    if f.exists():
        t += json.loads(f.read_text()).get("rate_limit_hits", 0)
print(t)
EOF
)"
  if (( rl_total > 2 && PARALLEL > 2 )); then PARALLEL=$(( PARALLEL / 2 )); (( PARALLEL < 2 )) && PARALLEL=2; echo "run_map: $rl_total rate-limit hits; PARALLEL -> $PARALLEL"; fi
  i=$(( i + ${#wave[@]} ))
done

echo "run_map: done. Not complete (re-queue with --only):"
$PY "$REVIEW_DIR/prep.py" pending | paste -sd, - || true
