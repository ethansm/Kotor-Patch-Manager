# L1 sub-agent probe, variant A (2026-10-10)
Cost **$0.00196** (ceiling $0.10). claude 2.1.296, `--restricted --safe-mode`, run rl1. Raw: `~/.cache/deep_haiku_review/kotor2-pilot/rl1/subagent_probe.json` + `subagent_probe/stream.jsonl`.

| check | result |
|---|---|
| agent_tool_available | ok |
| subagent_model_is_haiku | FAIL (no sub-agent ran) |
| deny_holds_in_subagent | FAIL (not tested) |
| cost_includes_subagent | FAIL |
| no_nested_agent | FAIL |
| allowed_file_readable | FAIL (not tested) |

**Verdict: FAIL.** Cause (from the stream): init lists agents = claude, Explore, general-purpose, Plan only; the probe's custom `evidence` agent (passed via `--agents`) is NOT loaded under `--safe-mode`. The Agent call returned "no agent type named evidence"; subagent_events = 0.
**Decision:** keep `escalate_subagents = false`. Variant B (drop `--safe-mode`) would be a new $0.10 go: NOT run, ask the user.
