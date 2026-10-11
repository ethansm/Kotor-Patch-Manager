#!/usr/bin/env bash
# F1: idempotent Fixer runner. One locked-down `claude -p` call per task attempt (Haiku 5.5 for auto-fixable /
# needs-review, $FIX_DESIGN_MODEL for needs-design, never human-only). Mirrors ../verify/run_verify.sh: same
# lockdown flags, rate-limit backoff, xargs waves, canary precondition, idempotency, budget caps.
#
#   run_fix.sh --first-only          run the first pending task alone (schema check + cost baseline)
#   run_fix.sh                       run all pending tasks wave by wave in chunks of $PARALLEL (default 8)
#   run_fix.sh --only T..,T..        just these tasks (re-queue)
#   run_fix.sh --class LIST          classes to run (default auto-fixable,needs-review)
#   run_fix.sh --wave N              only queue wave N
#   run_fix.sh --revise              run the next revise round (a3..a2+FIX_REVISE_ROUNDS, default 1) for tasks whose latest review says revise
#   run_fix.sh --dry-run             render the first prompt, print pending ids, the exact command, a cost estimate
#
# Per task (run_one): a1 = initial attempt; a2 = ONE retry after failed checks / scope violation (the check json is
# appended to the prompt); a3.. = revise rounds (--revise, FIX_REVISE_ROUNDS, default 1) seeded from the reviewed patch. Each attempt:
# fix_workspace make-task -> prompts -> claude (cwd = task copy, bwrap when variant A) -> record_fix fix ->
# fix_workspace diff -> fix_checks check (only for a non-empty in-scope patch of a "fixed" answer).
# Never two tasks sharing an allowed file run at once. Total spend is checked before every chunk.
#
# Env: PARALLEL (8)  FIX_BUDGET_USD (0.15/call, Haiku)  FIX_DESIGN_BUDGET_USD (0.60/call)  FIX_DESIGN_MODEL
#      (claude-sonnet-5-5)  FIX_LADDER_MODEL (claude-haiku-5-5)  FIX_LADDER_BUDGET_USD (0.15/call; ladder variants
#      with a "style" field always run here, whatever their class, except human-only)  FIX_TOTAL_BUDGET_USD (6)  FIX_TIMEOUT_S (600)  FIX_SANDBOX=bwrap|none (none only with the
#      stub claude or canary variant B + ALLOW_LAYER1_ONLY=1)  FIX_DIR FIX_WORK KOTOR2_REVIEW_HOME  FIX_CANARY_JSON
#      FIX_WORKSPACE_CMD FIX_CHECKS_CMD FIX_SETTINGS_CMD (test hooks)  FIX_BACKOFF_SCALE_S (test hook: fixed sleep).
# Writes only under $FIX_DIR, $FIX_WORK and $KOTOR2_REVIEW_HOME/fix (never the repo).
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
FIX_BUDGET_USD="${FIX_BUDGET_USD:-0.15}"
FIX_DESIGN_BUDGET_USD="${FIX_DESIGN_BUDGET_USD:-0.60}"
FIX_DESIGN_MODEL="${FIX_DESIGN_MODEL:-claude-sonnet-5-5}"
FIX_LADDER_BUDGET_USD="${FIX_LADDER_BUDGET_USD:-0.15}"
FIX_LADDER_MODEL="${FIX_LADDER_MODEL:-claude-haiku-5-5}"
FIX_TOTAL_BUDGET_USD="${FIX_TOTAL_BUDGET_USD:-6}"
FIX_TIMEOUT_S="${FIX_TIMEOUT_S:-600}"
HAIKU="claude-haiku-5-5"
PY="python3 -I"
CANARY_JSON="${FIX_CANARY_JSON:-$CODE_DIR/canary.json}"
WS="${FIX_WORKSPACE_CMD:-$PY $CODE_DIR/fix_workspace.py}"
CHK="${FIX_CHECKS_CMD:-$PY $CODE_DIR/fix_checks.py}"
SET="${FIX_SETTINGS_CMD:-$PY $CODE_DIR/fix_settings.py}"
export FIX_DIR FIX_WORK="$WORK" KOTOR2_REVIEW_HOME="$RHOME"

FIRST_ONLY=0; DRY=0; ONLY=""; CLASSES="auto-fixable,needs-review"; WAVE=""; REVISE=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    --first-only) FIRST_ONLY=1 ;;
    --dry-run) DRY=1 ;;
    --revise) REVISE="--revise" ;;
    --only) ONLY="${2:?--only needs ids}"; shift ;;
    --class) CLASSES="${2:?--class needs a list}"; shift ;;
    --wave) WAVE="${2:?--wave needs N}"; shift ;;
    *) echo "unknown arg $1" >&2; exit 2 ;;
  esac
  shift
done
[[ "$CLASSES" =~ ^(auto-fixable|needs-review|needs-design)(,(auto-fixable|needs-review|needs-design))*$ ]] || { echo "run_fix: --class takes auto-fixable,needs-review,needs-design (never human-only)" >&2; exit 2; }
[[ -z "$WAVE" || "$WAVE" =~ ^[0-9]+$ ]] || { echo "run_fix: --wave takes a number" >&2; exit 2; }
if [[ -n "$ONLY" ]]; then
  [[ "$ONLY" =~ ^T[0-9a-f]{10}(,T[0-9a-f]{10})*$ ]] || { echo "run_fix: bad --only list" >&2; exit 2; }
fi

refuse() {   # preconditions: refuse, except in --dry-run where they are reported and the dry run goes on
  if (( DRY )); then echo "run_fix: [dry-run] WOULD REFUSE: $1" >&2; else echo "run_fix: $1" >&2; exit 1; fi
}

# ---- preconditions -----------------------------------------------------------------------------------------
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
for f in fixer_prompt.md fix_schema.json fix_prompts.py record_fix.py; do
  [[ -f "$CODE_DIR/$f" ]] || refuse "$CODE_DIR/$f missing"
done
[[ -f "$WORK/fix_queue.json" ]] || refuse "$WORK/fix_queue.json missing; run fix_intake.py"
if ! $WS verify-base >/dev/null 2>&1; then refuse "fix_workspace verify-base failed (base missing, dirty or holds an excluded path)"; fi

ISOLATION="--restricted --safe-mode"
mkdir -p "$FIX_DIR"/{raw,results,patches,checks} "$WORK/prompts"
export CODE_DIR REVIEW_DIR FIX_HOME FIX_BUDGET_USD FIX_DESIGN_BUDGET_USD FIX_DESIGN_MODEL FIX_LADDER_BUDGET_USD FIX_LADDER_MODEL FIX_TIMEOUT_S HAIKU PY WS CHK SET WORK
export SANDBOX ISOLATION REVISE

# ---- one claude call -----------------------------------------------------------------------------------------
# call_claude <tid> <attempt> <model> <budget> <settings> <sysfile> <promptfile> <copy-dir>   -> sets CALL_* in the caller's env
call_claude() {
  local tid="$1" n="$2" model="$3" budget="$4" settings="$5" sysf="$6" prompt="$7" copy="$8"
  local tries=0 rl=0 sr=0 rc=0 out err wait
  CALL_RL=0; CALL_TRIES=0; CALL_RC=1
  while (( tries < 6 )); do
    tries=$((tries + 1))
    out="$FIX_DIR/raw/$tid.a$n.t$tries.out"; err="$FIX_DIR/raw/$tid.a$n.t$tries.err"
    local -a BW=()
    if [[ "$SANDBOX" == "bwrap" ]]; then
      mapfile -d '' -t BW < <($SET bwrap-argv claude "$copy" 2>/dev/null || true)
      if (( ${#BW[@]} == 0 )); then echo "$tid: bwrap-argv returned nothing; refusing to run unsandboxed" >&2; CALL_OUT="$out"; CALL_ERR="$err"; : > "$out"; : > "$err"; CALL_TRIES=$tries; return 1; fi
      if [[ "${BW[${#BW[@]}-1]}" == "--" ]]; then unset 'BW[${#BW[@]}-1]'; fi
      BW+=(--chdir "$copy" --)
      [[ "$(basename -- "${BW[0]}")" == "bwrap" ]] || BW=(bwrap "${BW[@]}")   # bwrap-argv prints args only
    fi
    set +e
    # shellcheck disable=SC2086
    (cd "$copy" && timeout "$FIX_TIMEOUT_S" "${BW[@]}" claude -p --model "$model" $ISOLATION \
        --settings "$settings" \
        --tools "Read,Grep,Glob,Edit,Write" --permission-mode dontAsk \
        --max-budget-usd "$budget" \
        --json-schema "$(cat "$CODE_DIR/fix_schema.json")" \
        --strict-mcp-config --mcp-config '{"mcpServers":{}}' \
        --disable-slash-commands --no-session-persistence \
        --output-format stream-json --verbose \
        --append-system-prompt "$(cat "$sysf")" \
        < "$prompt" > "$out" 2> "$err")
    rc=$?
    set -e
    CALL_OUT="$out"; CALL_ERR="$err"; CALL_RC=$rc; CALL_TRIES=$tries
    # only stderr and the final result event: tool results inside the stream can contain any text
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

model_for() {   # class [style] -> "model budget" (human-only and unknown classes: empty)
  case "$1" in
    auto-fixable|needs-review) echo "$HAIKU $FIX_BUDGET_USD" ;;
    needs-design)
      if [[ -n "${2:-}" ]]; then echo "$FIX_LADDER_MODEL $FIX_LADDER_BUDGET_USD"   # ladder variant: always Haiku-budget
      else echo "$FIX_DESIGN_MODEL $FIX_DESIGN_BUDGET_USD"; fi ;;
    *) echo "" ;;
  esac
}

# do_fix <tid> <attempt> <kind plain|error|revise> <base attempt>
do_fix() {
  local tid="$1" n="$2" kind="$3" base="$4" copy t0 started dur
  local cls spec allowed style mb model budget
  IFS=$'\t' read -r cls spec allowed style < <($PY "$CODE_DIR/record_fix.py" info "$tid")
  mb="$(model_for "$cls" "$style")"
  [[ -n "$mb" ]] || { echo "$tid: class $cls is never sent to a fixer"; return 0; }
  read -r model budget <<<"$mb"
  copy="$FIX_HOME/tasks/$tid.a$n"
  local -a from=() extra=()
  local prompt="$WORK/prompts/$tid.a$n.txt" sysp="$WORK/prompts/$tid.a$n.sys.md"
  if [[ "$kind" == "revise" ]]; then from=(--from-patch "$FIX_DIR/patches/$tid.a$base.patch"); fi
  if ! $WS make-task "$tid" --attempt "$n" "${from[@]}" >/dev/null 2>"$FIX_DIR/raw/$tid.a$n.make.err"; then
    $WS clean "$tid" >/dev/null 2>&1 || true
    if ! $WS make-task "$tid" --attempt "$n" "${from[@]}" >/dev/null 2>>"$FIX_DIR/raw/$tid.a$n.make.err"; then
      echo "$tid: make-task a$n failed (see raw/$tid.a$n.make.err)" >&2; return 1
    fi
  fi
  if [[ "$kind" == "error" ]]; then
    extra=(--error-file "$($PY "$CODE_DIR/record_fix.py" error-file "$tid" "$base")")
  elif [[ "$kind" == "revise" ]]; then
    $PY "$CODE_DIR/record_fix.py" revise-text "$tid" "$base" > "$WORK/prompts/$tid.a$n.revise.txt"
    extra=(--revise-file "$WORK/prompts/$tid.a$n.revise.txt")
  fi
  $PY "$CODE_DIR/fix_prompts.py" fixer "$tid" --attempt "$n" "${extra[@]}" > "$prompt"
  $PY "$CODE_DIR/fix_prompts.py" fixer-system "$tid" > "$sysp"
  local settings
  settings="$($SET fixer "$tid" --attempt "$n" | tail -n 1)"
  [[ -f "$settings" ]] || { echo "$tid: fix_settings produced no settings file ($settings)" >&2; return 1; }
  started="$(date -u +%Y-%m-%dT%H:%M:%SZ)"; t0=$(date +%s)
  call_claude "$tid" "$n" "$model" "$budget" "$settings" "$sysp" "$prompt" "$copy" || true
  dur=$(( $(date +%s) - t0 ))
  $PY "$CODE_DIR/record_fix.py" fix "$tid.a$n" "$CALL_OUT" "$CALL_ERR" "$CALL_RC" "$CALL_TRIES" "$CALL_RL" "$started" "$dur" "$prompt"
}

# finish <tid> <attempt>: diff and (when worthwhile) deterministic checks; both idempotent
finish() {
  local tid="$1" n="$2" o rc
  o="$($PY "$CODE_DIR/record_fix.py" outcome "$tid" "$n")"
  if [[ "$o" == "needs-diff" ]]; then
    $WS diff "$tid" --attempt "$n" >"$FIX_DIR/raw/$tid.a$n.diff.out" 2>"$FIX_DIR/raw/$tid.a$n.diff.err" || true
    o="$($PY "$CODE_DIR/record_fix.py" outcome "$tid" "$n")"
  fi
  if [[ "$o" == "needs-check" ]]; then
    set +e
    $CHK check "$tid" --attempt "$n" >"$FIX_DIR/raw/$tid.a$n.check.out" 2>"$FIX_DIR/raw/$tid.a$n.check.err"
    rc=$?
    set -e
    if (( rc == 3 )); then
      echo "$tid: fix_checks exit 3 (production ledger changed). ABORTING the whole run." >&2
      : > "$WORK/ABORT"
    fi
    o="$($PY "$CODE_DIR/record_fix.py" outcome "$tid" "$n")"
  fi
  echo "$tid: a$n -> $o"
}

run_one() {   # $1 = task id
  local tid="$1" act n kind base guard="" key i
  for (( i = 0; i < 5; i++ )); do
    [[ ! -e "$WORK/ABORT" ]] || { echo "$tid: abort flag set; skipping"; return 3; }
    read -r act n kind base < <($PY "$CODE_DIR/record_fix.py" next "$tid" $REVISE)
    [[ "$act" != "done" ]] || break
    key="$act $n ${kind:-}"
    if [[ "$guard" == *"|$key|"* ]]; then echo "$tid: '$key' made no progress; leaving it pending"; break; fi
    guard+="|$key|"
    case "$act" in
      fix) do_fix "$tid" "$n" "$kind" "${base:-0}" || break ;;
      finish) finish "$tid" "$n" ;;
      *) echo "$tid: unexpected action '$act'" >&2; break ;;
    esac
  done
}
export -f call_claude model_for do_fix finish run_one

spent() { $PY "$CODE_DIR/fcommon.py" spent fix; }
pending_args() {   # prints the shared pending-selection args
  local a=(--stage fix --class "$CLASSES")
  [[ -z "$WAVE" ]] || a+=(--wave "$WAVE")
  [[ -z "$ONLY" ]] || a+=(--only "$ONLY")
  [[ -z "$REVISE" ]] || a+=(--revise)
  printf '%s\n' "${a[@]}"
}
TRIED="$WORK/.fix_tried"; : > "$TRIED"
mapfile -t PARGS < <(pending_args)
mapfile -t PENDING < <($PY "$CODE_DIR/record_fix.py" pending "${PARGS[@]}")
echo "run_fix: ${#PENDING[@]} pending task(s); variant $VARIANT; sandbox $SANDBOX; parallel $PARALLEL; per-call cap \$$FIX_BUDGET_USD haiku / \$$FIX_DESIGN_BUDGET_USD design; total cap \$$FIX_TOTAL_BUDGET_USD"
(( ${#PENDING[@]} )) || exit 0

if (( DRY )); then
  printf '%s\n' "${PENDING[@]}" | head -20
  first="${PENDING[0]}"
  read -r act n kind base < <($PY "$CODE_DIR/record_fix.py" next "$first" $REVISE)
  [[ "$act" == "fix" ]] || { n=1; kind=plain; base=0; }
  ex=()
  if [[ "$kind" == "error" ]]; then ex=(--error-file "$($PY "$CODE_DIR/record_fix.py" error-file "$first" "$base")"); fi
  if [[ "$kind" == "revise" ]]; then $PY "$CODE_DIR/record_fix.py" revise-text "$first" "$base" > "$WORK/prompts/$first.a$n.revise.txt"; ex=(--revise-file "$WORK/prompts/$first.a$n.revise.txt"); fi
  $PY "$CODE_DIR/fix_prompts.py" fixer "$first" --attempt "$n" "${ex[@]}" > "$WORK/prompts/$first.a$n.txt"
  $PY "$CODE_DIR/fix_prompts.py" fixer-system "$first" > "$WORK/prompts/$first.a$n.sys.md"
  echo "rendered: $WORK/prompts/$first.a$n.txt ($(wc -c < "$WORK/prompts/$first.a$n.txt") bytes), .sys.md ($(wc -c < "$WORK/prompts/$first.a$n.sys.md") bytes)"
  # worst-case cost: every pending task at its per-call cap (retries/revise not included)
  est=0
  for id in "${PENDING[@]}"; do
    IFS=$'\t' read -r cls _ _ style < <($PY "$CODE_DIR/record_fix.py" info "$id")
    read -r _ b <<<"$(model_for "$cls" "$style")"; est=$($PY -c "print(round($est + float('${b:-0}'), 4))")
  done
  echo "cost: worst case \$$est for ${#PENDING[@]} first attempts (every call at its cap; a2/a3 retries extra); already spent \$$(spent); total cap \$$FIX_TOTAL_BUDGET_USD"
  $PY -c "import sys; sys.exit(0 if $est > float('$FIX_TOTAL_BUDGET_USD') else 1)" && echo "cost: worst case exceeds the total cap; the cap stops the run between chunks and the rest stays pending"
  echo "cmd: (cd \$FIX_HOME/tasks/<tid>.a<N> && timeout $FIX_TIMEOUT_S $( [[ $SANDBOX == bwrap ]] && echo "bwrap <fix_settings.py bwrap-argv claude <copy>> --chdir <copy> --" ) claude -p --model <$HAIKU|$FIX_DESIGN_MODEL> $ISOLATION --settings <fix_settings.py fixer <tid> --attempt N> --tools Read,Grep,Glob,Edit,Write --permission-mode dontAsk --max-budget-usd <$FIX_BUDGET_USD|$FIX_DESIGN_BUDGET_USD> --json-schema <fix_schema.json> --strict-mcp-config --mcp-config '{\"mcpServers\":{}}' --disable-slash-commands --no-session-persistence --output-format stream-json --verbose --append-system-prompt <fixer_prompt.md + specialty note> < $WORK/prompts/<tid>.a<N>.txt)"
  exit 0
fi

if (( FIRST_ONLY )); then
  run_one "${PENDING[0]}"
  echo "run_fix: first task ${PENDING[0]}: inspect results/${PENDING[0]}.a*.json, patches/, checks/ then run_fix.sh"
  $PY "$CODE_DIR/record_fix.py" next "${PENDING[0]}" $REVISE | grep -q '^done' || { echo "first task is not complete" >&2; exit 1; }
  exit 0
fi

while :; do
  s="$(spent)"
  if [[ -e "$WORK/ABORT" ]]; then echo "run_fix: ABORT flag set; stopping" >&2; exit 3; fi
  if $PY -c "import sys; sys.exit(0 if float('$s') >= float('$FIX_TOTAL_BUDGET_USD') else 1)"; then
    echo "run_fix: total budget reached (\$$s >= \$$FIX_TOTAL_BUDGET_USD); stopping. Remaining stay pending." >&2
    break
  fi
  mapfile -t CH < <($PY "$CODE_DIR/record_fix.py" pending "${PARGS[@]}" --exclude-file "$TRIED" --chunk "$PARALLEL")
  (( ${#CH[@]} )) || break
  printf '%s\n' "${CH[@]}" >> "$TRIED"
  echo "run_fix: chunk of ${#CH[@]} (parallel $PARALLEL), spent so far \$$s"
  t0=$(date +%s)
  printf '%s\n' "${CH[@]}" | xargs -P "$PARALLEL" -I{} bash -c 'run_one "$1"' _ {} || true
  rl_total="$($PY "$CODE_DIR/record_fix.py" rl-hits --since "$t0" "${CH[@]}")"
  if (( rl_total > 2 && PARALLEL > 2 )); then PARALLEL=$(( PARALLEL / 2 )); (( PARALLEL < 2 )) && PARALLEL=2; echo "run_fix: $rl_total rate-limit hits; PARALLEL -> $PARALLEL"; fi
done

if [[ -e "$WORK/ABORT" ]]; then echo "run_fix: ABORT flag set (production ledger changed during a check)" >&2; exit 3; fi
echo "run_fix: done. Outcomes:"
$PY "$CODE_DIR/record_fix.py" summary fix
left="$($PY "$CODE_DIR/record_fix.py" pending "${PARGS[@]}" | paste -sd, - || true)"
if [[ -n "$left" ]]; then echo "run_fix: done. Not complete (re-queue with --only):"; echo "$left"; else echo "run_fix: done."; fi
