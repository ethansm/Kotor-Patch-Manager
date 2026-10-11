# Wave example ACC1 (live acceptance run, 2026-10-11)

One real Haiku-first evidence wave for the `/plan-handoff` skill: a synthetic phase ("add `--json` to `wave_budget.py check` and `wave_tools.py list`, document it"), run through W0 to W3 with `scripts/wave_tools.py`. Use it as a worked example of what each stage produces. Recipe and caps: `patch_manager_mods/SUBAGENT_ROLES.md` "Claude Code Haiku swarm (waves)"; skill: `.claude/skills/plan-handoff/SKILL.md` Step 1b.

The originals live untracked in `.claude/waves/w1-ACC1/` and `w2-ACC1/` (gitignored); this folder is the curated copy. The `--json` change itself was never implemented: the phase was a test vehicle.

## Files
| Path | Stage | What it shows |
|---|---|---|
| `phase.txt` | input | The phase text (stands in for `plan_manager.py phase-text <plan> <ID>`). |
| `w0_scout/scout.tsv` | W0 Scout | `task<TAB>entry<TAB>question` rows. Reconstructed from the scout's inline reply with the questions shortened; the real reply was longer. The scout listed task `a` twice (two entries), which became one recon unit. |
| `w1_recon/manifest.json` | W1 Recon | Manifest schema: 3 units, each with entries, budget estimate, out/reply/brief paths. |
| `w1_recon/u01.brief.md`, `u01.reply.txt` | W1 Recon | A generated brief (contract, scope, ranges, stop condition, schema) and the unit's 5-line recon reply. |
| `w1_recon/u0{1,2,3}.facts.md` | W1 Recon | The facts files (24 to 62 lines): signatures, anchors, hazards. |
| `w2_cards/a.md`, `b.md`, `c.md` | W2 Draft | Worker cards in the plan template. `b.md` is the second attempt (see below); `a.md` is kept as drafted. |
| `w2_cards/manifest.json` | W2 Draft | Worker-wave manifest (scope = the unit's card path). |
| `w2_cards/semantic_check.txt` | W3 Check | The Haiku semantic cross-check verdicts. |
| `check_and_report.txt` | W3 Check | Output of `wave_tools.py check`/`report` and `wave_cards.py verify`. |

## What happened
- Scout: 1 Haiku, 7 rows. Recon: 3 Haiku units, each estimated 10.6k to 11.7k tokens. Draft: 3 Haiku units.
- `wave_cards.py verify` failed card b: the drafter wrote `**Contract** (worker)` instead of `- **Contract**: worker`. One Haiku retry with the failure excerpt fixed it (escalation step 1).
- The semantic check marked card a MISMATCH: its Objective adds a `verdict` key and omits `over_cap` and `too_many`, while the phase says to print the `check()` dict. This is the class of drift the deterministic verifier cannot see; the Opus gate (or one retry) decides. `a.md` was left as drafted to show it.
- Measured peaks (`wave_tools.py report`): 46.5k to 57.2k per unit, against estimates of 10.6k to 11.7k. The fixed Haiku context before any read is about 45k, so `WAVE_OVERHEAD_TOKENS` now defaults to 45000.
- Cost: 9 Haiku agents including the scout, about $0.12.

## Bugs this run found (fixed in `591ab72`, each with a regression test)
1. `wave_tools check` marked a unit FAIL for another session's edit in the same directory. Only the unit's own entry files can FAIL now; unrelated paths are WARN.
2. Worker waves always FAILed because `.claude/waves/` is gitignored and `agent_contract.py` reads git. Claimed files inside the wave dir are verified by existence instead.
3. `wave_cards.py` read an anchor's closing backtick as the start of a quote.
4. The 10000-token overhead default was far below the measured fixed context.

## Reading the numbers
`est_tokens` in the manifests was computed with the old 10000 overhead; the report column "peak" is the real number. Re-estimating today gives about 46k to 47k per unit.
