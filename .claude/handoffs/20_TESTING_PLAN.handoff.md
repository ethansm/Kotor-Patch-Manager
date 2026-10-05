# Session Handoff & Plan State: patch_manager_mods/20_TESTING_PLAN.md

## 0. Next-Phase Direction
start working on the initial batch of bug fixes to start iterating through all the batches

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/20_TESTING_PLAN.md`
- **Branch:** `KOTOR-II` (0f06f6c doc 20/20d: Medium #4/#7/#9 + T3 WndProc + Low round DONE; findings F27-F33)
- **Last Landed Commit:**
```
0f06f6c doc 20/20d: Medium #4/#7/#9 + T3 WndProc + Low round DONE; findings F27-F33
 patch_manager_mods/20_TESTING_PLAN.md               |  4 +++-
 patch_manager_mods/20d_phase1_session_2026-10-03.md | 13 +++++++++++++
 2 files changed, 16 insertions(+), 1 deletion(-)
```
- **Dirty Files (130):**
  - `M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/30_queue_hud.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M .claude/skills/plan-handoff/SKILL.md`
  - ` M AGENTS.md`
  - ` M CLAUDE.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/README.md`

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

## 7. Bug-fix batch roadmap (open doc 20d findings, set 10-05)
- [x] **BF1 DONE 10-05 (b25d4fa, fd34ccf; F21 closed wrong, F33 DONE 6ab426f):** F29 cfx A_REGEN overflow, F30 marqueeText off<0, F31 inv3d INI long-line split, F32 inv3d fitLookup var range, F21 (finding was wrong: inv3d + equip DO subclass; close it, fix the ActionHotbar comment wording only if harmless). Files we own, clean at 10-05.
- **BF2:** EquipCharPreview config edges F14 (screen clamp leaves screenExplicit), F15 (fov/zoom uncapped), F16 (unreadable stats -> evil idle), F19 (INI hygiene: hex floats, (int)1e30 UB, fgets(600) split, unused partLen/getModel). Check equip ownership first (doc 23 / Mod 14 gated).
- **BF3:** BuffDurationHUD F25 hygiene (dead branch :238-244, arrowRawFor slot 0) - check buff/shield split session ownership.
- **BF4 (design, no code):** F28 WndProc leak -> design note for a Common `WindowHook.h` (single dispatcher, never unhook a non-top proc: pass-through flag instead). Needs Common sync -> out of scope until approved.
- **Needs user decision:** F17 dual-wield style, F18 x87 vs SSE 1px, F20 raw-address dedupe (GameAddr.h migration), F26 StrFmt.h (Common), F3/F4 WARN-only gaps, F13 upstream PR.

## 8. Traps from the 10-05 round
- `run_tests.py -k a -k b` keeps only the LAST `-k`; run one `-k` per call.
- Bash heredoc with backticks inside an unquoted `<<EOF` runs them as commands; use `<<'EOF'`.
- `--wine` red items on 10-05 are all queue-hud (F33, reported to session cs11-install-heal-queue-fix).
- Hand-break verification: copy the header to scratch, mutate, run, copy back; then grep the fix is present.
