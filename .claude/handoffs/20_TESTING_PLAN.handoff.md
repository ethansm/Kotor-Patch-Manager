# Session Handoff & Plan State: patch_manager_mods/20_TESTING_PLAN.md

## 0. Next-Phase Direction
BD2 DONE 10-05 (MG7/MG6/MG2-buff source; install pending). BD1 DONE 10-05 (open decisions F17/F18/F20/F26/F3/F4/F13 carried out; see §7). Next: user reviews the F13 PR draft; then dispatch doc 32 MG6/MG7 (owner-gated)

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/20_TESTING_PLAN.md`
- **Branch:** `KOTOR-II` (fd3b039 doc 20/20d: BF4 DONE (F28 design note e60de4a); next = user decisions)
- **Last Landed Commit:**
```
fd3b039 doc 20/20d: BF4 DONE (F28 design note e60de4a); next = user decisions
 patch_manager_mods/20_TESTING_PLAN.md               | 1 +
 patch_manager_mods/20d_phase1_session_2026-10-03.md | 2 +-
 2 files changed, 2 insertions(+), 1 deletion(-)
```
- **Dirty Files (134):**
  - `M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/20_TESTING_PLAN.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/30_queue_hud.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M .claude/skills/plan-handoff/SKILL.md`
  - ` M AGENTS.md`
  - ` M CLAUDE.md`

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.
## 7. Bug-fix batch roadmap (open doc 20d findings, set 10-05)
- [x] **BF1 DONE 10-05 (b25d4fa, fd34ccf; F21 closed wrong, F33 DONE 6ab426f):** F29 cfx A_REGEN overflow, F30 marqueeText off<0, F31 inv3d INI long-line split, F32 inv3d fitLookup var range, F21 (finding was wrong: inv3d + equip DO subclass; close it, fix the ActionHotbar comment wording only if harmless). Files we own, clean at 10-05.
- [x] **BF2 DONE 10-05 (47be017, 14d905a):** EquipCharPreview config edges F14 (screen clamp leaves screenExplicit), F15 (fov/zoom uncapped), F16 (unreadable stats -> evil idle), F19 (INI hygiene: hex floats, (int)1e30 UB, fgets(600) split, unused partLen/getModel). Check equip ownership first (doc 23 / Mod 14 gated).
- [x] **BF3 DONE 10-05 (60dd2eb):** BuffDurationHUD F25 hygiene (dead branch :238-244, arrowRawFor slot 0) - check buff/shield split session ownership.
- [x] **BF4 DONE 10-05 (e60de4a, docs fd3b039):** design note `patch_manager_mods/20f_windowhook_design.md` (design A: per-DLL pass-through + module pin; B shared dispatcher rejected). `SubPass<N>` model in tests/wndproc_chain.
- [x] **BF5 DONE 10-05 (7d41942, 7aca7fd; KPM d46a699, f03c258):** `_shared_src/WindowHook.h` + host test + KPM Common sync + GuiKit wheel on it; per-mod migration = doc 32 MG6 rows, owner-gated (TODO).
- [x] **BD2 DONE 10-05:** probe edits c0df1d8; MG7/MG6/MG2-buff source done (c2be15c, 39d2921, a104576, 1af7130, c368b5e, 2ba0875, ec4f6b6..35e0146, 9134fe7); KPM mirrored for 6 mods; inv3d/equip3d/ranged-ai/BuffDurationHUD KPM dirs stale; install + play check (wheel zoom/scroll, unload order) pending user go.
- [x] **BD1 DONE 10-05 (user decisions):** F17 note 60bc42e (styleFor already = engine UpdateWeaponClass; table pinned 34cb100), F18 integer math 34cb100, F26 StrFmt.h 84e9a7c (KPM d18015c), F3/F4 `[[adjacency]]` 4c5aaa3, F20 GameAddr names dca7bfc (KPM d6aaaec), F13 local draft `kpm-pr-test-source-filter` (branch `fix/create-patch-skip-test-sources`, 7830acc, PULL_REQUEST.md untracked; NOT pushed).
- **Next:** user reviews F13 PR text (send = explicit approval); owner-gated doc 32 MG2 (adds `strfmt::appendf`), MG6 (WindowHook), MG7 (GameAddr names).

## 8. Traps from the 10-05 round
- `run_tests.py -k a -k b` keeps only the LAST `-k`; run one `-k` per call.
- Bash heredoc with backticks inside an unquoted `<<EOF` runs them as commands; use `<<'EOF'`.
- `--wine` red items on 10-05 are all queue-hud (F33, reported to session cs11-install-heal-queue-fix).
- Hand-break verification: copy the header to scratch, mutate, run, copy back; then grep the fix is present.
- Handoff script wipes §7/§8 of this file: back it up before `plan_manager.py handoff`, re-append after.
- Host stub `research/companion_levelup/tools/parity/stub/windows.h` has NO window APIs (SetWindowLongPtrA, CallWindowProcA, GetModuleHandleExA, InterlockedExchange): WindowHook.h needs a host seam.
