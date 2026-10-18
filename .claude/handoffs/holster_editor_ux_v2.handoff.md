# Session Handoff & Plan State: .claude/plans/holster_editor_ux_v2.md

## 0. Next-Phase Direction
No phases parsed from .claude/plans/holster_editor_ux_v2.md; check its header levels (## Phase N / table with Id + Status columns).

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/holster_editor_ux_v2.md`
- **Branch:** `KOTOR-II` (8b74740 docs(play-queue): HUX-P0 + HUX-U1 rows (HW-UX2 U0/U1 prepared, GATE PASS))
- **Generated:** 2026-10-18 00:59:43 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
8b74740 docs(play-queue): HUX-P0 + HUX-U1 rows (HW-UX2 U0/U1 prepared, GATE PASS)
 patch_manager_mods/PLAY_TEST_QUEUE.md | 2 ++
 1 file changed, 2 insertions(+)
```
- **Dirty Files (168):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M CLAUDE.md`
  - ... +158 more (run `git status --short`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/plans/holster_editor_ux_v2.md`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_

## 10-18 state (U0 + U1 built)
- Commits: plan 57ad185; mod-config 0.1.1-dev 1f7b7cb; holster 0.1.4-dev fd6135c; holster-editor 0.2.6-dev 5662013; play rows 8b74740. KPM: 536fa3c, edd9f5b, 8059084 + GameAddr commit.
- Prepared: `kpm_hotswap.py --mod holster --mod holster-editor --mod mod-config --prepare` -> SWAP x3; GATE PASS head 566201349ea5.
- NEXT: user GO (game closed) -> `python3 scripts/kpm_hotswap.py --mod holster --mod holster-editor --mod mod-config --apply`; user plays HUX-P0 + HUX-U1 (+ HW-LH1 ini edit); Haiku log readers via kotor2_logs.py; then U2 (needs P0 camera q confirmation + ITEMNAME result).
- ITEMNAME: identified=0 default; identified=1 (probe_names=2) goes through the strref->achievement switch 0x4010E0 and an unchecked base-row deref.
- 10-18 01:17 HW-UX2 U0+U1 LIVE x3; rollback batch_swap_20261018_011740.

## 10-18 HUX-P0 log read
- ITEMNAME works with identified=0 (name0 `{06}Vibrosword`, `{05}Vibroblade`, `{02}Long Sword`); strip the `{NN}` prefix; identified=1 not needed.
- PROJ refuted S1's camera axes: hook pos (0.024,0.001,1.593), q(w,x,y,z)=(0.7071,0,0,0.7071), fovy 10.33@zoom2.2 / 21.6@1.05, ext (1345,357,576,896). Mounts at world y ~ +-0.44 project to sx 4579 / -1443 (view x range 1345..1921). Camera z follows panZ; q turns ~1 deg per yaw degree.
- NEXT U0b: show candidate-projection labels A..F in the view (six forward/up axis conventions) and ask which sits on the weapon; then fix worldToScreen. U2 mouse/mode bar and U3 rows do not depend on it; U4 markers do.
