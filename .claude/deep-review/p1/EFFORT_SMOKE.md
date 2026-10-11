# L0 effort smoke (2026-10-10)

Cost: **$0.00244** total (ceiling $0.05, 4 calls, per-call cap $0.0125). Model `claude-haiku-5-5`, isolation `--restricted --safe-mode`, prompt "reply OK" with a one-field JSON schema. `CLAUDE_CODE_EFFORT_LEVEL` was **unset**. Script: `.claude/deep-review/phase3b/l0_effort_smoke.sh`; raw: `phase3b/l0/out/`.

| arm | output_tokens | cost USD | duration_ms | turns | subtype |
|---|---|---|---|---|---|
| none (no flag) | 245 | 0.001548 | 1352 | 2 | success |
| low | 65 | 0.000427 | 1732 | 3 | success |
| medium | 143 | 0.000143 | 1262 | 2 | success |
| xhigh | 262 | 0.000323 | 1686 | 2 | success |

## Decisions (plan rules)
- (a) `--effort` is **honoured** under `-p --safe-mode`: explicit arms rise low 65 < medium 143 < xhigh 262 output tokens.
- (b) **Leak likely**: arm `none` (245) is close to `xhigh` (262) and far above `low` (65). The user's global `modelSettings` haiku=`xhigh` seems to reach `--safe-mode` workers. Pin explicit levels in profiles; do not rely on inheriting.
- (c) Not applicable (arms differ; env var unset).

## Caveats
- n=1 per arm on a trivial prompt: the none-vs-xhigh closeness is suggestive, not proof. Re-check on real stage output during L2.
- Cost is NOT monotonic in effort (medium cheaper than low): at this size cost is dominated by input/cache pricing, not output tokens. No usable Haiku $/1k-output-token figure came out of L0; `EFFORT_COST_MULT` stays 1.0 until L2 measures real per-style cost.
