#!/usr/bin/env bash
# Idempotent Adjudicate runner: one locked-down Sonnet 5.5 `claude -p` session per dossier batch.
# Lockdown identical to ../tools/run_reduce.sh (same settings file, tools, isolation variant, MCP off,
# model claude-sonnet-5-5); output is stream-json (--verbose) so cost can be priced per request.
#
#   run_adjudicate.sh --dry-run        list pending batches and the exact command; no model call
#   run_adjudicate.sh                  adjudicate every pending batch, one session at a time
#   run_adjudicate.sh --only batch_03[,batch_07]
#
# Input : work/adjudicate/batch_NN.{md,json} (build_dossiers.py).
# Output: $VERIFY_DIR/adjudications/batch_NN.json (+ .check.json from quote_check.py), $VERIFY_DIR/adj_raw/.
# Env: ADJUDICATE_BUDGET_USD (1.50 per session), ADJUDICATE_TOTAL_BUDGET_USD (5; the estimate for the full
#      queue is higher, raise it deliberately), ADJUDICATE_TIMEOUT_S (1800), ADJUDICATE_PARALLEL (1),
#      VERIFY_DIR, VERIFY_WORK, KOTOR2_REVIEW_HOME, ALLOW_LAYER1_ONLY=1.
set -euo pipefail

CODE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REVIEW_DIR="$(cd "$CODE_DIR/.." && pwd)"
RHOME="${KOTOR2_REVIEW_HOME:-$HOME/.cache/kotor2_review}"
TREE="$RHOME/worktree"
VERIFY_DIR="$(mkdir -p "${VERIFY_DIR:-$CODE_DIR}" && cd "${VERIFY_DIR:-$CODE_DIR}" && pwd)"
if [[ -n "${VERIFY_WORK:-}" ]]; then WORK="$VERIFY_WORK"
elif [[ "$VERIFY_DIR" == "$CODE_DIR" ]]; then WORK="$REVIEW_DIR/work"
else WORK="$VERIFY_DIR/work"; fi
BUDGET="${ADJUDICATE_BUDGET_USD:-1.50}"
TOTAL="${ADJUDICATE_TOTAL_BUDGET_USD:-5}"
TIMEOUT_S="${ADJUDICATE_TIMEOUT_S:-1800}"
PARALLEL="${ADJUDICATE_PARALLEL:-1}"
MODEL="claude-sonnet-5-5"
PY="python3 -I"
export VERIFY_DIR VERIFY_WORK="$WORK" KOTOR2_REVIEW_HOME="$RHOME"

DRY=0; ONLY=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    --dry-run) DRY=1 ;;
    --only) ONLY="${2:?--only needs batch ids}"; shift ;;
    *) echo "unknown arg $1" >&2; exit 2 ;;
  esac
  shift
done

# ---- gates (same as run_reduce.sh) ----------------------------------------------------------------
[[ -f "$REVIEW_DIR/canary.json" ]] || { echo "refusing: canary.json missing" >&2; exit 1; }
VARIANT="$($PY - "$REVIEW_DIR/canary.json" <<'PYEOF'
import json, os, sys
c = json.load(open(sys.argv[1], encoding="utf-8"))
if isinstance(c, dict) and c.get("pass") is True:
    print(c.get("variant", "A"))
elif isinstance(c, dict) and c.get("layer1_only") and os.environ.get("ALLOW_LAYER1_ONLY") == "1":
    print(c.get("variant", "A"))
else:
    print("FAIL")
PYEOF
)"
[[ "$VARIANT" != "FAIL" ]] || { echo "refusing: canary.json does not have \"pass\": true" >&2; exit 1; }
$PY "$REVIEW_DIR/prep.py" verify-snapshot >/dev/null || { echo "refusing: snapshot verification failed" >&2; exit 1; }
[[ -f "$REVIEW_DIR/worker_settings.json" ]] || { echo "abort: worker_settings.json missing" >&2; exit 1; }
[[ -f "$CODE_DIR/adjudicate_prompt.md" && -f "$CODE_DIR/adjudicate_schema.json" ]] || { echo "abort: adjudicate_prompt.md or adjudicate_schema.json missing" >&2; exit 1; }
[[ -d "$TREE" ]] || { echo "abort: sanitized worktree not found: $TREE" >&2; exit 1; }
[[ "$TREE" != "$(git -C "$REVIEW_DIR" rev-parse --show-toplevel 2>/dev/null)"* ]] || { echo "abort: tree inside repo" >&2; exit 1; }
[[ -d "$WORK/adjudicate" ]] || { echo "abort: $WORK/adjudicate missing; run build_dossiers.py" >&2; exit 1; }

ISOLATION=(--restricted --safe-mode); [[ "$VARIANT" == "B" ]] && ISOLATION=(--restricted)
mkdir -p "$VERIFY_DIR"/{adjudications,adj_raw}
export REVIEW_DIR CODE_DIR TREE MODEL BUDGET TIMEOUT_S PY WORK
export ISOLATION_STR="${ISOLATION[*]}"

run_batch() {   # $1 = batch id (batch_NN)
  local b="$1" attempt=0 rc=0 rl=0 started dur t0 out err
  local prompt="$WORK/adjudicate/$b.md"
  [[ -f "$prompt" ]] || { echo "$b: dossier $prompt missing" >&2; return 1; }
  if $PY "$CODE_DIR/vcommon.py" adj-pending | grep -qx "$b"; then :; else echo "$b: already complete, skipped"; return 0; fi
  started="$(date -u +%Y-%m-%dT%H:%M:%SZ)"; t0=$(date +%s)
  while (( attempt < 3 )); do
    attempt=$((attempt + 1))
    out="$VERIFY_DIR/adj_raw/$b.a$attempt.out"; err="$VERIFY_DIR/adj_raw/$b.a$attempt.err"
    set +e
    # shellcheck disable=SC2086
    (cd "$TREE" && timeout "$TIMEOUT_S" claude -p --model "$MODEL" $ISOLATION_STR \
        --settings "$REVIEW_DIR/worker_settings.json" --tools "Read,Grep,Glob" \
        --permission-mode dontAsk --max-budget-usd "$BUDGET" \
        --json-schema "$(cat "$CODE_DIR/adjudicate_schema.json")" \
        --strict-mcp-config --mcp-config '{"mcpServers":{}}' \
        --disable-slash-commands --no-session-persistence \
        --output-format stream-json --verbose \
        --append-system-prompt "$(cat "$CODE_DIR/adjudicate_prompt.md")" \
        < "$prompt" > "$out" 2> "$err")
    rc=$?
    set -e
    if (( rc != 0 )) && { cat "$err"; tail -n 1 "$out"; } 2>/dev/null | grep -qiE '(\b429\b|\b529\b|rate.?limit|overloaded)'; then
      rl=$((rl + 1))
      local wait=$(( (2 ** attempt) * 15 + RANDOM % 15 ))
      echo "$b: rate limited (attempt $attempt), backing off ${wait}s"
      sleep "${VERIFY_BACKOFF_SCALE_S:-$wait}"
      continue
    fi
    break
  done
  dur=$(( $(date +%s) - t0 ))
  $PY "$REVIEW_DIR/tools/record_verify.py" adjudicate "$b" "$out" "$err" "$rc" "$attempt" "$rl" "$started" "$dur" "$prompt"
  $PY "$CODE_DIR/quote_check.py" --adjudication "$b" || true
}
export -f run_batch

spent() { $PY "$CODE_DIR/vcommon.py" spent adjudicate; }

if [[ -n "$ONLY" ]]; then
  [[ "$ONLY" =~ ^batch_[0-9]+(,batch_[0-9]+)*$ ]] || { echo "run_adjudicate: bad --only list" >&2; exit 2; }
  mapfile -t PENDING < <(tr ',' '\n' <<<"$ONLY")
else
  mapfile -t PENDING < <($PY "$CODE_DIR/vcommon.py" adj-pending)
fi
echo "run_adjudicate: ${#PENDING[@]} pending batch(es); variant $VARIANT; per-session cap \$$BUDGET; total cap \$$TOTAL; spent so far \$$(spent)"
(( ${#PENDING[@]} )) || exit 0

if (( DRY )); then
  printf '%s\n' "${PENDING[@]}" | head -40
  for b in "${PENDING[@]:0:3}"; do echo "dossier $b: $(wc -c < "$WORK/adjudicate/$b.md") bytes, $(python3 -I -c 'import json,sys;print(len(json.load(open(sys.argv[1]))["items"]))' "$WORK/adjudicate/$b.json") item(s)"; done
  echo "cmd: (cd $TREE && timeout $TIMEOUT_S claude -p --model $MODEL $ISOLATION_STR --settings $REVIEW_DIR/worker_settings.json --tools Read,Grep,Glob --permission-mode dontAsk --max-budget-usd $BUDGET --json-schema <adjudicate_schema.json> --strict-mcp-config --mcp-config '{\"mcpServers\":{}}' --disable-slash-commands --no-session-persistence --output-format stream-json --verbose --append-system-prompt <adjudicate_prompt.md> < work/adjudicate/<batch>.md)"
  est="$($PY -c "print(round(${#PENDING[@]}*0.25,2), round(${#PENDING[@]}*0.75,2))")"
  echo "estimate for ${#PENDING[@]} batch(es) at \$0.25-0.75 each: \$${est% *} - \$${est#* }  (total cap \$$TOTAL)"
  exit 0
fi

i=0
while (( i < ${#PENDING[@]} )); do
  s="$(spent)"
  if $PY -c "import sys; sys.exit(0 if float('$s') >= float('$TOTAL') else 1)"; then
    echo "run_adjudicate: total budget reached (\$$s >= \$$TOTAL); stopping. Remaining stay pending." >&2
    break
  fi
  wave=("${PENDING[@]:i:PARALLEL}")
  echo "run_adjudicate: running ${wave[*]} (spent so far \$$s)"
  printf '%s\n' "${wave[@]}" | xargs -P "$PARALLEL" -I{} bash -c 'run_batch "$1"' _ {} || true
  i=$(( i + ${#wave[@]} ))
done
echo "run_adjudicate: done. Not complete (re-queue with --only):"
$PY "$CODE_DIR/vcommon.py" adj-pending | paste -sd, - || true
