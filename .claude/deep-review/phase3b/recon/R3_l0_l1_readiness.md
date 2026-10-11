# R3 L0/L1 live-smoke readiness (no model calls run)

## L0 effort smoke (proposed script, save as $SCR/l0_effort_smoke.sh)
```bash
#!/usr/bin/env bash
# Run by a human. 4 real haiku calls, $0.02 cap each. Needs SCR=<empty scratch dir>.
set -euo pipefail
B=/home/ethansm/.claude/skills/deep-haiku-review/bundle
SCR="${SCR:?set SCR to an empty scratch dir}"
source "$B/engine/verify/runlib.sh"   # sourcing sets set -euo pipefail; dr_claude needs no dr_init
export TREE="$SCR/tree" SETTINGS="$B/templates/worker_settings.json" CLAUDE_BIN="${DR_CLAUDE_BIN:-claude}"
export ISOLATION_STR="--restricted --safe-mode"
unset CLAUDE_CODE_EFFORT_LEVEL                  # env would override --effort
mkdir -p "$TREE" "$SCR/out"
echo "reply OK" > "$SCR/prompt.txt"
echo "Answer with the JSON object only." > "$SCR/sys.txt"
echo '{"type":"object","properties":{"answer":{"type":"string"}},"required":["answer"]}' > "$SCR/schema.json"
for E in none low medium xhigh; do
  if [[ $E == none ]]; then unset EFFORT_DEFAULT; else export EFFORT_DEFAULT=$E; fi
  dr_claude haiku 0.02 120 "$SCR/schema.json" "$SCR/sys.txt" "$SCR/prompt.txt" \
    "$SCR/out/$E.jsonl" "$SCR/out/$E.err" || echo "$E rc=$?"
done
python3 -I - "$SCR" <<'PYEOF'
import glob, json, sys
for p in sorted(glob.glob(sys.argv[1] + "/out/*.jsonl")):
    r = None
    for line in open(p, encoding="utf-8", errors="replace"):
        if line.startswith("{"):
            try: o = json.loads(line)
            except ValueError: continue
            if o.get("type") == "result": r = o
    if r is None: print(p, "NO RESULT EVENT"); continue
    u = r.get("usage") or {}
    print(p.rsplit("/", 1)[-1], "out_tok=", u.get("output_tokens"), "cost=", r.get("total_cost_usd"),
          "dur_ms=", r.get("duration_ms"), "turns=", r.get("num_turns"), "subtype=", r.get("subtype"),
          "models=", sorted((r.get("modelUsage") or {}).keys()))
PYEOF
```
Field check: a raw result event (kotor2-pilot/p1/verify/refute_raw/batch_01.a1.out) has `total_cost_usd`, `duration_ms`, `num_turns`, `subtype`, `usage` (with `output_tokens`), and `modelUsage` (keys are model IDs, e.g. `claude-sonnet-5-5`). All names exist.

## L1 probe (subagent_probe.sh)
- CLI: `DR_RUN=r1 DR_PROFILE=<profile> bash .../bundle/engine/tools/subagent_probe.sh --variant A|B` (`--dry-run` first, $0).
- Writes `$RUN/subagent_probe.json` plus `subagent_probe/stream.jsonl` and stderr.
- Cost: one haiku call, hard cap `DR_PROBE_BUDGET` (default $0.10).
- Pass: all of agent_tool_available, subagent_model_is_haiku, deny_holds_in_subagent (canary absent), cost_includes_subagent, no_nested_agent, plus allowed_file_readable.
- Variant A is `--restricted --safe-mode`; B is `--restricted` only. Unlike canary_test.sh, the probe does NOT auto-fall-back, so a failed A needs a manual rerun with B.
- Open: whether `--agents` works under `--safe-mode` is not stated in the code; a no-subagent FAIL under A is the expected symptom.
