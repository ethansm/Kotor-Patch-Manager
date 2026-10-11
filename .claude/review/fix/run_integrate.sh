#!/usr/bin/env bash
# Idempotent Integrate runner (Fix stage F3): one locked-down Sonnet 5.5 `claude -p` session per integration batch,
# in its own scratch clone (integ/<batch>) of the pinned base. Mirrors ../verify/run_adjudicate.sh.
# The model's claims are never trusted: `fix_checks.py verify-series` re-applies the final series on a fresh slot
# and re-runs the batch gates; only a verified series is published to $KOTOR2_FIX_OUT/<batch>/.
#
#   run_integrate.sh --dry-run          list pending batches, the exact command, a cost estimate; no model call
#   run_integrate.sh                    integrate every pending batch (INTEG_PARALLEL at a time)
#   run_integrate.sh --only batch_03[,batch_07]
#   run_integrate.sh --first-only       just the first pending batch (smoke run)
#   run_integrate.sh --force            with --only: redo a batch that already has a run/verify record
#
# Input : work/fix/batches/batch_NN.{md,json} (build_integration_batches.py).
# Output: $FIX_DIR/integ/<batch>.{run,verify}.json (+ .stack.txt), $FIX_DIR/raw/integ_<batch>.aN.{out,err},
#         $KOTOR2_FIX_OUT/<batch>/NNNN-<tid>.patch + series.json (verified batches only).
# Env: INTEG_BUDGET_USD (1.50 per session), INTEG_TOTAL_BUDGET_USD (8), INTEG_PARALLEL (3, max 5),
#      INTEG_TIMEOUT_S (2400), INTEG_BACKOFF_SCALE_S (test hook), FIX_SANDBOX=bwrap|none (default bwrap),
#      FIX_DIR, FIX_WORK, KOTOR2_REVIEW_HOME, KOTOR2_FIX_OUT, FIX_CANARY_JSON (fix/canary.json override),
#      FIX_WORKSPACE_CMD / FIX_CHECKS_CMD / FIX_SETTINGS_CMD / FIX_RECORD_CMD (stand-ins for the sibling scripts;
#      default `python3 -I <this dir>/<script>.py`; commands must not contain spaces in their paths).
set -euo pipefail

CODE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REVIEW_DIR="$(cd "$CODE_DIR/.." && pwd)"
RHOME="${KOTOR2_REVIEW_HOME:-$HOME/.cache/kotor2_review}"
FIX_HOME="$RHOME/fix"
FIX_DIR="$(mkdir -p "${FIX_DIR:-$CODE_DIR}" && cd "${FIX_DIR:-$CODE_DIR}" && pwd)"
if [[ -n "${FIX_WORK:-}" ]]; then WORK="$FIX_WORK"
elif [[ "$FIX_DIR" == "$CODE_DIR" ]]; then WORK="$REVIEW_DIR/work/fix"
else WORK="$FIX_DIR/work"; fi
BUDGET="${INTEG_BUDGET_USD:-1.50}"
TOTAL="${INTEG_TOTAL_BUDGET_USD:-8}"
PARALLEL="${INTEG_PARALLEL:-3}"
TIMEOUT_S="${INTEG_TIMEOUT_S:-2400}"
SANDBOX="${FIX_SANDBOX:-bwrap}"
MODEL="claude-sonnet-5-5"
PY="python3 -I"
[[ "$PARALLEL" =~ ^[0-9]+$ ]] && (( PARALLEL >= 1 )) || PARALLEL=3
(( PARALLEL <= 5 )) || PARALLEL=5
[[ "$SANDBOX" == "bwrap" || "$SANDBOX" == "none" ]] || { echo "run_integrate: FIX_SANDBOX must be bwrap or none" >&2; exit 2; }

WS_CMD="${FIX_WORKSPACE_CMD:-$PY $CODE_DIR/fix_workspace.py}"
CHECKS_CMD="${FIX_CHECKS_CMD:-$PY $CODE_DIR/fix_checks.py}"
SETTINGS_CMD="${FIX_SETTINGS_CMD:-$PY $CODE_DIR/fix_settings.py}"
if [[ -n "${FIX_RECORD_CMD:-}" ]]; then RECORD_CMD="$FIX_RECORD_CMD"
elif [[ -f "$CODE_DIR/record_fix.py" ]]; then RECORD_CMD="$PY $CODE_DIR/record_fix.py"
else RECORD_CMD="$PY $REVIEW_DIR/tools/record_fix.py"; fi
BUILDER="$PY $CODE_DIR/build_integration_batches.py"
export FIX_DIR FIX_WORK="$WORK" KOTOR2_REVIEW_HOME="$RHOME"

DRY=0; ONLY=""; FIRST=0; FORCE=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --dry-run) DRY=1 ;;
    --first-only) FIRST=1 ;;
    --force) FORCE=1 ;;
    --only) ONLY="${2:?--only needs batch ids}"; shift ;;
    *) echo "unknown arg $1" >&2; exit 2 ;;
  esac
  shift
done

# ---- preconditions -------------------------------------------------------------------------------------
CANARY="${FIX_CANARY_JSON:-$CODE_DIR/canary.json}"   # the FIX canary, not the audit one in ../
[[ -f "$CANARY" ]] || { echo "refusing: canary.json missing ($CANARY)" >&2; exit 1; }
$PY - "$CANARY" <<'PYEOF' || { echo "refusing: canary.json needs \"pass\": true and \"integrator_ok\": true (tools/fix_canary.sh)" >&2; exit 1; }
import json, sys
c = json.load(open(sys.argv[1], encoding="utf-8"))
sys.exit(0 if isinstance(c, dict) and c.get("pass") is True and c.get("integrator_ok") is True else 1)
PYEOF
[[ -f "$CODE_DIR/integrate_schema.json" && -f "$CODE_DIR/integrator_prompt.md" ]] || { echo "abort: integrate_schema.json or integrator_prompt.md missing" >&2; exit 1; }
[[ "$FIX_HOME" != "$(git -C "$REVIEW_DIR" rev-parse --show-toplevel 2>/dev/null)"* ]] || { echo "abort: FIX_HOME inside the repo" >&2; exit 1; }
# shellcheck disable=SC2086
$WS_CMD verify-base >/dev/null || { echo "refusing: base workspace verification failed (fix_workspace.py verify-base)" >&2; exit 1; }
[[ -d "$FIX_HOME/base" ]] || { echo "abort: $FIX_HOME/base missing" >&2; exit 1; }
compgen -G "$WORK/batches/batch_*.json" >/dev/null || { echo "abort: no batches in $WORK/batches; run build_integration_batches.py" >&2; exit 1; }
if [[ "$SANDBOX" == "bwrap" ]]; then command -v bwrap >/dev/null || { echo "abort: bwrap not installed (FIX_SANDBOX=none is for self-tests only)" >&2; exit 1; }; fi

mkdir -p "$FIX_DIR"/{integ,raw} "$WORK/prompts" "$FIX_HOME"/{tmp,ledger,integ}
export CODE_DIR REVIEW_DIR RHOME FIX_HOME WORK MODEL BUDGET TIMEOUT_S PY SANDBOX WS_CMD CHECKS_CMD SETTINGS_CMD RECORD_CMD BUILDER FORCE

spent() { $PY "$CODE_DIR/fcommon.py" spent integrate; }

run_batch() {   # $1 = batch id (batch_NN)
  local b="$1" attempt=0 rc=0 rl=0 started dur t0 out err
  local dossier="$WORK/batches/$b.md" integ="$FIX_HOME/integ/$b" tmpd="$FIX_HOME/tmp/$b"
  local prompt="$WORK/prompts/integrate_$b.md" runj="$FIX_DIR/integ/$b.run.json" verj="$FIX_DIR/integ/$b.verify.json"
  [[ -f "$dossier" ]] || { echo "$b: dossier $dossier missing" >&2; return 1; }
  if (( FORCE )); then rm -f "$runj" "$verj"; fi
  local run_ok=0
  if [[ -f "$runj" ]] && $PY - "$runj" <<'PYEOF'
import json, sys
r = json.load(open(sys.argv[1], encoding="utf-8"))
sys.exit(0 if isinstance(r, dict) and r.get("status") == "ok" and isinstance(r.get("output"), dict) else 1)
PYEOF
  then run_ok=1; fi

  if (( ! run_ok )); then
    mkdir -p "$tmpd" "$FIX_HOME/ledger"
    # (a) scratch clone of base (create or reset), serialized on the base repo
    (
      flock 9
      # a clone, not a worktree: a worktree's .git file points into base/.git, hidden inside the gate sandbox
      if [[ -f "$integ/.git" ]]; then rm -rf "$integ"; git -C "$FIX_HOME/base" worktree prune; fi
      $WS_CMD integ-reset "$b" >/dev/null
    ) 9>"$FIX_HOME/integ.lock"
    # patch files go inside the worktree: the gate wrapper only accepts root-relative paths
    $PY - "$WORK/batches/$b.json" "$integ" <<'PYEOF'
import json, os, shutil, sys
bj = json.load(open(sys.argv[1], encoding="utf-8"))
dst = os.path.join(sys.argv[2], "_patches")
os.makedirs(dst, exist_ok=True)
for t in bj["tasks"]:
    shutil.copyfile(t["patch"], os.path.join(dst, t["task_id"] + ".patch"))
os.makedirs(os.path.join(sys.argv[2], ".git", "info"), exist_ok=True)
with open(os.path.join(sys.argv[2], ".git", "info", "exclude"), "a", encoding="utf-8") as fh:
    fh.write("/_patches/\n/_resolved/\n")   # keep the integrator's inputs/outputs out of diffcheck/status
PYEOF
    # (b) stack check first; conflicts go to the dossier appendix (a per-run copy; the builder's dossier stays pristine)
    # shellcheck disable=SC2086
    $CHECKS_CMD stack "$b" > "$FIX_DIR/integ/$b.stack.txt" 2>&1 || true
    { cat "$dossier"
      printf '\n## Appendix: stack check (dry apply of the whole series with --3way)\n\n````\n'
      head -n 200 "$FIX_DIR/integ/$b.stack.txt" | head -c 20000
      printf '\n````\n'
      printf '\n## Appendix: clean-base gate results\n\n'
      # shellcheck disable=SC2086
      $CHECKS_CMD baseline "$b" 2>/dev/null || echo "baseline: unavailable (compare against the per-task check.json \`baseline\` field)"
    } > "$prompt"
    # (c) settings + wrapper
    local settings
    # shellcheck disable=SC2086
    settings="$($SETTINGS_CMD integrator "$b" | tail -n 1)"
    if [[ ! -f "$settings" ]]; then settings="$(compgen -G "$WORK/settings/*$b*.json" | head -n 1 || true)"; fi
    [[ -f "$settings" ]] || { echo "$b: no integrator settings produced" >&2; return 1; }
    $WS_CMD install-bin --batches "$b" >/dev/null || { echo "$b: install-bin failed" >&2; return 1; }   # writes bin/fixgate_<batch>.sh
    [[ -x "$FIX_HOME/bin/fixgate_$b.sh" ]] || { echo "$b: $FIX_HOME/bin/fixgate_$b.sh missing or not executable" >&2; return 1; }
    local BW=()
    if [[ "$SANDBOX" == "bwrap" ]]; then
      # shellcheck disable=SC2086
      mapfile -d '' -t BW < <($SETTINGS_CMD bwrap-argv claude "$integ" "$tmpd" "$FIX_HOME/ledger")
      (( ${#BW[@]} )) || { echo "$b: empty bwrap argv" >&2; return 1; }
      [[ "$(basename -- "${BW[0]}")" == "bwrap" ]] || BW=(bwrap "${BW[@]}")
      BW+=(--ro-bind "$FIX_HOME/bin" "$FIX_HOME/bin")   # fixgate_<batch>.sh lives under hidden RHOME (canary binds it too)
      # ~/Games is tmpfs-hidden in the claude profile: bind KPM so the nested gate sandbox can re-bind it (bug 12)
      local kpm; kpm="$(eval echo "${KPM_DIR:-~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager}")"
      [[ -d "$kpm" ]] && BW+=(--ro-bind "$kpm" "$kpm")
    fi
    started="$(date -u +%Y-%m-%dT%H:%M:%SZ)"; t0=$(date +%s)
    while (( attempt < 3 )); do
      attempt=$((attempt + 1))
      out="$FIX_DIR/raw/integ_$b.a$attempt.out"; err="$FIX_DIR/raw/integ_$b.a$attempt.err"
      set +e
      (cd "$integ" && timeout "$TIMEOUT_S" "${BW[@]}" claude -p --model "$MODEL" --restricted --safe-mode \
          --settings "$settings" --tools "Read,Grep,Glob,Edit,Write,Bash" \
          --permission-mode dontAsk --max-budget-usd "$BUDGET" \
          --json-schema "$(cat "$CODE_DIR/integrate_schema.json")" \
          --strict-mcp-config --mcp-config '{"mcpServers":{}}' \
          --disable-slash-commands --no-session-persistence \
          --output-format stream-json --verbose \
          --append-system-prompt "$(cat "$CODE_DIR/integrator_prompt.md")" \
          < "$prompt" > "$out" 2> "$err")
      rc=$?
      if (( rc != 0 )) && { cat "$err"; tail -n 1 "$out"; } 2>/dev/null | grep -qiE '(\b429\b|\b529\b|rate.?limit|overloaded)'; then
        rl=$((rl + 1))
        local wait=$(( (2 ** attempt) * 15 + RANDOM % 15 ))
        echo "$b: rate limited (attempt $attempt), backing off ${wait}s"
        sleep "${INTEG_BACKOFF_SCALE_S:-$wait}"
        continue
      fi
      break
    done
    dur=$(( $(date +%s) - t0 ))
    # (e) record
    # shellcheck disable=SC2086
    $RECORD_CMD integrate "$b" "$out" "$err" "$rc" "$attempt" "$rl" "$started" "$dur" "$prompt" || true
  else
    echo "$b: ok integrator run already recorded; verifying only"
  fi

  # (f) never trust the model's claims: re-apply + re-gate from scratch
  if [[ -f "$runj" ]] && $PY - "$runj" <<'PYEOF'
import json, sys
r = json.load(open(sys.argv[1], encoding="utf-8"))
sys.exit(0 if isinstance(r, dict) and r.get("status") == "ok" and isinstance(r.get("output"), dict) else 1)
PYEOF
  then
    # shellcheck disable=SC2086
    $CHECKS_CMD verify-series "$b" || true
    # (g) publish only a verified series
    if [[ -f "$verj" ]] && $PY - "$verj" <<'PYEOF'
import json, sys
v = json.load(open(sys.argv[1], encoding="utf-8"))
sys.exit(0 if isinstance(v, dict) and v.get("ok") is True else 1)
PYEOF
    then $BUILDER publish "$b" || echo "$b: publish failed" >&2
    else echo "$b: verify-series not ok; series not published"; fi
  else
    echo "$b: integrator run not ok (see $runj); stays pending"
  fi
}
export -f run_batch

# ---- pending list --------------------------------------------------------------------------------------
if [[ -n "$ONLY" ]]; then
  [[ "$ONLY" =~ ^batch_[0-9]+(,batch_[0-9]+)*$ ]] || { echo "run_integrate: bad --only list" >&2; exit 2; }
  mapfile -t PENDING < <(tr ',' '\n' <<<"$ONLY")
  for b in "${PENDING[@]}"; do [[ -f "$WORK/batches/$b.json" ]] || { echo "run_integrate: unknown batch $b" >&2; exit 2; }; done
  if (( ! FORCE )); then
    mapfile -t _all < <($BUILDER pending)
    keep=(); for b in "${PENDING[@]}"; do for p in "${_all[@]}"; do [[ "$b" == "$p" ]] && keep+=("$b"); done; done
    PENDING=("${keep[@]}")
  fi
else
  mapfile -t PENDING < <($BUILDER pending)
fi
(( ! FIRST || ${#PENDING[@]} == 0 )) || PENDING=("${PENDING[0]}")
echo "run_integrate: ${#PENDING[@]} pending batch(es); sandbox $SANDBOX; per-session cap \$$BUDGET; total cap \$$TOTAL; parallel $PARALLEL; spent so far \$$(spent)"
(( ${#PENDING[@]} )) || exit 0

if (( DRY )); then
  printf '%s\n' "${PENDING[@]}" | head -40
  for b in "${PENDING[@]:0:3}"; do
    echo "dossier $b: $(wc -c < "$WORK/batches/$b.md") bytes, $($PY -c 'import json,sys;print(len(json.load(open(sys.argv[1]))["tasks"]))' "$WORK/batches/$b.json") patch(es)"
  done
  pre="(cd $FIX_HOME/integ/<batch> && timeout $TIMEOUT_S"
  [[ "$SANDBOX" == "bwrap" ]] && pre="$pre \$(fix_settings.py bwrap-argv claude $FIX_HOME/integ/<batch> $FIX_HOME/tmp/<batch> $FIX_HOME/ledger)"
  echo "per batch: fix_workspace.py integ-reset <batch>; fix_checks.py stack <batch>; fix_settings.py integrator <batch>"
  echo "cmd: $pre claude -p --model $MODEL --restricted --safe-mode --settings <integrator settings> --tools Read,Grep,Glob,Edit,Write,Bash --permission-mode dontAsk --max-budget-usd $BUDGET --json-schema <integrate_schema.json> --strict-mcp-config --mcp-config '{\"mcpServers\":{}}' --disable-slash-commands --no-session-persistence --output-format stream-json --verbose --append-system-prompt <integrator_prompt.md> < work/fix/prompts/integrate_<batch>.md)"
  echo "then: record_fix.py integrate; fix_checks.py verify-series <batch>; publish to \$KOTOR2_FIX_OUT/<batch>/ when verify ok"
  est="$($PY -c "print(round(${#PENDING[@]}*0.25,2), round(${#PENDING[@]}*0.75,2))")"
  echo "estimate for ${#PENDING[@]} batch(es) at \$0.25-0.75 each: \$${est% *} - \$${est#* }  (total cap \$$TOTAL)"
  exit 0
fi

i=0
while (( i < ${#PENDING[@]} )); do
  s="$(spent)"
  if $PY -c "import sys; sys.exit(0 if float('$s') >= float('$TOTAL') else 1)"; then
    echo "run_integrate: total budget reached (\$$s >= \$$TOTAL); stopping. Remaining stay pending." >&2
    break
  fi
  wave=("${PENDING[@]:i:PARALLEL}")
  echo "run_integrate: running ${wave[*]} (spent so far \$$s)"
  printf '%s\n' "${wave[@]}" | xargs -P "$PARALLEL" -I{} bash -c 'run_batch "$1"' _ {} || true
  i=$(( i + ${#wave[@]} ))
done
left="$($BUILDER pending | paste -sd, - || true)"
if [[ -n "$left" ]]; then echo "run_integrate: done. Not complete (re-queue with --only):"; echo "$left"; else echo "run_integrate: done."; fi
