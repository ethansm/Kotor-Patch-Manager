#!/usr/bin/env bash
# L0 effort smoke: 4 real haiku calls (arms none, low, medium, xhigh), per-call cap $0.0125 (worst case $0.05).
# A 5th arm "env" runs only when CLAUDE_CODE_EFFORT_LEVEL is set (it overrides --effort; no --effort flag there).
# Needs SCR=<empty scratch dir>. Run by a human. Ends by printing the markdown table and verdict via l0_parse.py.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
B=/home/ethansm/.claude/skills/deep-haiku-review/bundle
SCR="${SCR:?set SCR to an empty scratch dir}"
[[ -f "$B/engine/verify/runlib.sh" ]] || { echo "missing $B/engine/verify/runlib.sh" >&2; exit 2; }
mkdir -p "$SCR"
if [[ -n "$(ls -A "$SCR")" ]]; then echo "SCR must be empty: $SCR" >&2; exit 2; fi

source "$B/engine/verify/runlib.sh"   # defines dr_claude; sourcing sets set -euo pipefail; no dr_init needed
export TREE="$SCR/tree" SETTINGS="$B/templates/worker_settings.json" CLAUDE_BIN="${DR_CLAUDE_BIN:-claude}"
export ISOLATION_STR="--restricted --safe-mode"
# dr_effort_args resolves EFFORT_<STAGE> before EFFORT_DEFAULT; keep the stage key free of an EFFORT_L0 override
export DR_EFFORT_STAGE=L0
unset EFFORT_L0 DR_EFFORT_STYLE
mkdir -p "$TREE" "$SCR/out"
printf 'CLAUDE_CODE_EFFORT_LEVEL=%s\n' "${CLAUDE_CODE_EFFORT_LEVEL:-<unset>}" > "$SCR/env_effort.txt"
echo "reply OK" > "$SCR/prompt.txt"
echo "Answer with the JSON object only." > "$SCR/sys.txt"
echo '{"type":"object","properties":{"answer":{"type":"string"}},"required":["answer"]}' > "$SCR/schema.json"

ARMS=(none low medium xhigh)
if [[ -n "${CLAUDE_CODE_EFFORT_LEVEL:-}" ]]; then ARMS+=(env); fi
for E in "${ARMS[@]}"; do
  if [[ $E == none || $E == env ]]; then unset EFFORT_DEFAULT; else export EFFORT_DEFAULT=$E; fi
  dr_claude claude-haiku-5-5 0.0125 120 "$SCR/schema.json" "$SCR/sys.txt" "$SCR/prompt.txt" \
    "$SCR/out/$E.jsonl" "$SCR/out/$E.err" || echo "$E rc=$?"
done

python3 -I "$HERE/l0_parse.py" "$SCR"
