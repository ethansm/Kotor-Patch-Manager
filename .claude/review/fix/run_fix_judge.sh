#!/usr/bin/env bash
# Ladder judge runner: one locked-down read-only Haiku 5.5 `claude -p` call per PARENT task whose variants include
# >= 2 check-ok candidates (fix_ladder.py judge-queue). The judge sees the finding, each candidate's diff + gate summary
# under blind labels (## Candidate A/B/C) and a read-only copy of the ORIGINAL files; never variant ids, style names or
# the label key (fix_ladder.py judge_key stays outside every prompt and every sandbox-visible path).
#
# FIX_WORK MUST be set by the caller (normally $WORK/ladder_ws, the merged queue of real tasks + variants); the
# runner refuses to guess. FIX_DIR / KOTOR2_REVIEW_HOME as for run_fix_review.sh.
#
#   run_fix_judge.sh --dry-run          render every pending prompt into $WORK/prompts/judge_<parent>.txt, print the
#                                       command and a cost estimate; no model call, no writes outside prompts/
#   run_fix_judge.sh                    judge everything pending in chunks of $PARALLEL (default 4)
#   run_fix_judge.sh --only P,P         just these parents
#   run_fix_judge.sh --first-only       judge the first pending parent alone
#   run_fix_judge.sh --calib [--only item,..]
#                                       judge the calibration items (fix_ladder.py judge-calib build first); prompts are
#                                       judge_calib_<item>.txt; prints the `judge-calib score` JSON afterwards
#
# Result per parent: $FIX_DIR/judge/<parent>.json (fix_ladder.py record-judge): pick label mapped back to a variant id.
# A parent with exactly one check-ok candidate is auto-picked by `judge-queue`; with none it stays residue.
# Env: PARALLEL (4)  JUDGE_BUDGET_USD (0.08/call)  JUDGE_TOTAL_BUDGET_USD (0.5, sum of judge/*.json + judge/calib/*.json)
#      FIX_TIMEOUT_S (300)  FIX_SANDBOX FIX_DIR FIX_WORK KOTOR2_REVIEW_HOME FIX_CANARY_JSON FIX_WORKSPACE_CMD
#      FIX_LEDGER_GUARD (ledger path to hash, default the production master ledger) FIX_BACKOFF_SCALE_S (test hooks).
set -euo pipefail

CODE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REVIEW_DIR="$(cd "$CODE_DIR/.." && pwd)"
RHOME="${KOTOR2_REVIEW_HOME:-$HOME/.cache/kotor2_review}"
FIX_HOME="$RHOME/fix"
FIX_DIR="$(mkdir -p "${FIX_DIR:-$CODE_DIR}" && cd "${FIX_DIR:-$CODE_DIR}" && pwd)"
PARALLEL="${PARALLEL:-4}"
JUDGE_BUDGET_USD="${JUDGE_BUDGET_USD:-0.08}"
JUDGE_TOTAL_BUDGET_USD="${JUDGE_TOTAL_BUDGET_USD:-0.5}"
FIX_TIMEOUT_S="${FIX_TIMEOUT_S:-300}"
MODEL="claude-haiku-5-5"
PY="python3 -I"
CANARY_JSON="${FIX_CANARY_JSON:-$CODE_DIR/canary.json}"
WS="${FIX_WORKSPACE_CMD:-$PY $CODE_DIR/fix_workspace.py}"
SET="${FIX_SETTINGS_CMD:-$PY $CODE_DIR/fix_settings.py}"
LADDER="$PY $CODE_DIR/fix_ladder.py"
LEDGER="${FIX_LEDGER_GUARD:-$(cd "$REVIEW_DIR/../.." && pwd)/.claude/master_ledger.jsonl}"

DRY=0; ONLY=""; CALIB=0; FIRST_ONLY=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --dry-run) DRY=1 ;;
    --calib) CALIB=1 ;;
    --first-only) FIRST_ONLY=1 ;;
    --only) ONLY="${2:?--only needs ids}"; shift ;;
    *) echo "unknown arg $1" >&2; exit 2 ;;
  esac
  shift
done
if [[ -n "$ONLY" ]]; then
  if (( CALIB )); then [[ "$ONLY" =~ ^[a-z][a-z0-9]{2,15}(,[a-z][a-z0-9]{2,15})*$ ]] || { echo "run_fix_judge: bad --only list" >&2; exit 2; }
  else [[ "$ONLY" =~ ^T[0-9a-f]{10}(,T[0-9a-f]{10})*$ ]] || { echo "run_fix_judge: bad --only list" >&2; exit 2; }
  fi
fi

refuse() {
  if (( DRY )); then echo "run_fix_judge: [dry-run] WOULD REFUSE: $1" >&2; else echo "run_fix_judge: $1" >&2; exit 1; fi
}

# ---- preconditions (same as run_fix_review.sh) ---------------------------------------------------------------
[[ -n "${FIX_WORK:-}" ]] || { echo "run_fix_judge: FIX_WORK must be set by the caller (normally <work>/ladder_ws)" >&2; exit 2; }
WORK="$FIX_WORK"
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
for f in judge_prompt.md judge_schema.json fix_ladder.py fix_prompts.py record_fix.py fix_settings.py; do
  [[ -f "$CODE_DIR/$f" ]] || refuse "$CODE_DIR/$f missing"
done
[[ -f "$WORK/fix_queue.json" ]] || refuse "$WORK/fix_queue.json missing; run fix_ladder.py variants"
if ! $WS verify-base >/dev/null 2>&1; then refuse "fix_workspace verify-base failed (base missing, dirty or holds an excluded path)"; fi

ISOLATION="--restricted --safe-mode"
mkdir -p "$FIX_DIR"/{raw,judge} "$WORK/prompts"
CF=""; PFX="judge_"
if (( CALIB )); then CF="--calib"; PFX="judge_calib_"; fi
export CODE_DIR REVIEW_DIR FIX_HOME FIX_DIR FIX_WORK="$WORK" KOTOR2_REVIEW_HOME="$RHOME" JUDGE_BUDGET_USD FIX_TIMEOUT_S MODEL PY WS SET LADDER \
       WORK SANDBOX ISOLATION CF PFX LEDGER JUDGE_TOTAL_BUDGET_USD CALIB

ledger_sha() { [[ -f "$LEDGER" ]] && sha256sum "$LEDGER" | cut -d' ' -f1 || echo none; }
LEDGER0="$(ledger_sha)"
export LEDGER0
ledger_ok() {
  if [[ "$(ledger_sha)" != "$LEDGER0" ]]; then
    echo "run_fix_judge: production ledger changed (was $LEDGER0). ABORTING." >&2; : > "$WORK/ABORT"; return 1
  fi
}
spent() { $PY "$CODE_DIR/fix_ladder.py" judge-spent; }
over_budget() { $PY -c "import sys; sys.exit(0 if float('$(spent)') >= float('$JUDGE_TOTAL_BUDGET_USD') else 1)"; }
rec_status() {   # <id> -> record status or "none"
  local f="$FIX_DIR/judge/$1.json"; (( CALIB )) && f="$FIX_DIR/judge/calib/$1.json"
  $PY -c "import json,sys
try: print(json.load(open(sys.argv[1])).get('status','none'))
except Exception: print('none')" "$f"
}

# ---- pending list ----------------------------------------------------------------------------------------------
if (( CALIB )); then
  [[ -n "$($LADDER judge-calib list --all)" ]] || refuse "no judge calibration items; run: fix_ladder.py judge-calib build"
  RAWLIST="$($LADDER judge-calib list)"
else
  QARGS=(judge-queue); (( DRY )) && QARGS+=(--no-write)
  [[ -z "$ONLY" ]] || QARGS+=(--only "$ONLY")
  RAWLIST="$($LADDER "${QARGS[@]}")"
fi
PENDING=()
if [[ -n "$RAWLIST" ]]; then
  IFS=',' read -r -a _ALL <<< "$RAWLIST"
  for x in "${_ALL[@]}"; do
    if (( CALIB )) && [[ -n "$ONLY" ]] && [[ ",$ONLY," != *",$x,"* ]]; then continue; fi
    PENDING+=("$x")
  done
fi
echo "run_fix_judge: ${#PENDING[@]} pending judgement(s)$( (( CALIB )) && echo ' (calibration)' ); variant $VARIANT; sandbox $SANDBOX; parallel $PARALLEL; per-call cap \$$JUDGE_BUDGET_USD; total cap \$$JUDGE_TOTAL_BUDGET_USD"

call_claude() {   # <id> <tag> <settings> <sysfile> <promptfile> <copy>
  local id="$1" tag="$2" settings="$3" sysf="$4" prompt="$5" copy="$6"
  local tries=0 rl=0 sr=0 rc=0 out err wait
  CALL_RL=0; CALL_TRIES=0; CALL_RC=1
  while (( tries < 6 )); do
    tries=$((tries + 1))
    out="$FIX_DIR/raw/$PFX$id.$tag.t$tries.out"; err="$FIX_DIR/raw/$PFX$id.$tag.t$tries.err"
    local -a BW=()
    if [[ "$SANDBOX" == "bwrap" ]]; then
      mapfile -d '' -t BW < <($SET bwrap-argv claude "$copy" 2>/dev/null || true)
      if (( ${#BW[@]} == 0 )); then echo "$id: bwrap-argv returned nothing; refusing to run unsandboxed" >&2; CALL_OUT="$out"; CALL_ERR="$err"; : > "$out"; : > "$err"; CALL_TRIES=$tries; return 1; fi
      if [[ "${BW[${#BW[@]}-1]}" == "--" ]]; then unset 'BW[${#BW[@]}-1]'; fi
      BW+=(--chdir "$copy" --)
      [[ "$(basename -- "${BW[0]}")" == "bwrap" ]] || BW=(bwrap "${BW[@]}")   # bwrap-argv prints args only
    fi
    set +e
    # shellcheck disable=SC2086
    (cd "$copy" && timeout "$FIX_TIMEOUT_S" "${BW[@]}" claude -p --model "$MODEL" $ISOLATION \
        --settings "$settings" \
        --tools "Read,Grep,Glob" --permission-mode dontAsk \
        --max-budget-usd "$JUDGE_BUDGET_USD" \
        --json-schema "$(cat "$CODE_DIR/judge_schema.json")" \
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
      echo "$id: rate limited (try $tries), backing off ${wait}s" >&2
      sleep "${FIX_BACKOFF_SCALE_S:-$wait}"
      continue
    fi
    if (( sr == 0 )) && tail -n 1 "$out" 2>/dev/null | grep -q 'error_max_structured_output_retries'; then
      sr=1; echo "$id: structured-output retries exhausted; one re-queue" >&2
      continue
    fi
    break
  done
  return 0
}

run_one() {   # $1 = parent id (or calibration item id)
  local id="$1" pass copy settings prompt sysp t0 dur st
  [[ ! -e "$WORK/ABORT" ]] || return 3
  mapfile -t CS < <($LADDER judge-copy "$id" $CF) || { echo "$id: judge-copy failed" >&2; return 1; }
  copy="${CS[0]:-}"; settings="${CS[1]:-}"
  [[ -d "$copy" && -f "$settings" ]] || { echo "$id: judge-copy produced no copy/settings" >&2; return 1; }
  for pass in 1 2; do
    [[ ! -e "$WORK/ABORT" ]] || return 3
    if over_budget; then echo "$id: total budget reached; leaving pending" >&2; return 0; fi
    prompt="$WORK/prompts/$PFX$id.txt"; sysp="$WORK/prompts/$PFX$id.sys.md"
    $LADDER judge-prompt "$id" $CF > "$prompt"
    $LADDER judge-prompt "$id" --system > "$sysp"
    t0=$(date +%s)
    call_claude "$id" "j$pass" "$settings" "$sysp" "$prompt" "$copy" || true
    dur=$(( $(date +%s) - t0 ))
    $LADDER record-judge "$id" "$CALL_OUT" $CF --rc "$CALL_RC" --dur "$dur"
    st="$(rec_status "$id")"
    [[ "$st" == "invalid" ]] || break           # ok -> done; failed -> stays pending; invalid -> one re-run
    echo "$id: invalid judgement (pass $pass); re-queueing once" >&2
  done
  ledger_ok || return 3
}
export -f call_claude run_one ledger_sha ledger_ok spent over_budget rec_status

(( ${#PENDING[@]} )) || { (( CALIB )) && $LADDER judge-calib score || true; exit 0; }

if (( DRY )); then
  printf '%s\n' "${PENDING[@]}" | head -20
  for id in "${PENDING[@]}"; do
    $LADDER judge-prompt "$id" $CF > "$WORK/prompts/$PFX$id.txt"
    $LADDER judge-prompt "$id" --system > "$WORK/prompts/$PFX$id.sys.md"
  done
  first="${PENDING[0]}"
  echo "rendered ${#PENDING[@]} prompt(s) into $WORK/prompts/$PFX<id>.txt (first: $PFX$first.txt, $(wc -c < "$WORK/prompts/$PFX$first.txt") bytes; system $(wc -c < "$WORK/prompts/$PFX$first.sys.md") bytes)"
  est="$($PY -c "print(round(${#PENDING[@]} * float('$JUDGE_BUDGET_USD'), 4))")"
  echo "cost: worst case \$$est for ${#PENDING[@]} judgement(s) (re-runs extra); already spent \$$(spent); total cap \$$JUDGE_TOTAL_BUDGET_USD"
  echo "cmd: (cd \$FIX_HOME/tasks/$PFX<id> && timeout $FIX_TIMEOUT_S $( [[ $SANDBOX == bwrap ]] && echo "bwrap <fix_settings.py bwrap-argv claude <copy>> --chdir <copy> --" ) claude -p --model $MODEL $ISOLATION --settings <fix_ladder.py judge-copy settings> --tools Read,Grep,Glob --permission-mode dontAsk --max-budget-usd $JUDGE_BUDGET_USD --json-schema <judge_schema.json> --strict-mcp-config --mcp-config '{\"mcpServers\":{}}' --disable-slash-commands --no-session-persistence --output-format stream-json --verbose --append-system-prompt <judge_prompt.md> < $WORK/prompts/$PFX<id>.txt)"
  exit 0
fi

if (( FIRST_ONLY )); then
  run_one "${PENDING[0]}"
  [[ "$(rec_status "${PENDING[0]}")" == "ok" ]] && echo "run_fix_judge: first judgement OK: inspect judge/${PENDING[0]}.json" \
    || { echo "first judgement did not complete" >&2; exit 1; }
  exit 0
fi

TRIED="$WORK/.judge_tried"; : > "$TRIED"
idx=0
while (( idx < ${#PENDING[@]} )); do
  if [[ -e "$WORK/ABORT" ]]; then echo "run_fix_judge: ABORT flag set; stopping" >&2; exit 3; fi
  if over_budget; then
    echo "run_fix_judge: total budget reached (\$$(spent) >= \$$JUDGE_TOTAL_BUDGET_USD); stopping. Remaining stay pending." >&2
    break
  fi
  CH=("${PENDING[@]:idx:PARALLEL}"); idx=$((idx + ${#CH[@]}))
  printf '%s\n' "${CH[@]}" >> "$TRIED"
  echo "run_fix_judge: chunk of ${#CH[@]} (parallel $PARALLEL), spent so far \$$(spent)"
  printf '%s\n' "${CH[@]}" | xargs -P "$PARALLEL" -I{} bash -c 'run_one "$1"' _ {} || true
done
ledger_ok || exit 3

echo "run_fix_judge: done."
if (( CALIB )); then
  $LADDER judge-calib score || true
else
  left="$($LADDER judge-queue --no-write | tr -d '\n')"
  if [[ -n "$left" ]]; then echo "run_fix_judge: not complete (re-run with --only):"; echo "$left"; fi
fi
