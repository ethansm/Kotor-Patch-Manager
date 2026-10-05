# Session Handoff & Plan State: patch_manager_mods/20_TESTING_PLAN.md

## 0. Next-Phase Direction
persist context for a safe clear (doc 20 testing plan: Phases 2-3 done; next = Medium/Low mods §5, T2/T3 tiers)

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/20_TESTING_PLAN.md`
- **Branch:** `KOTOR-II` (38d2090 doc 24: C5b installed by DLL hot-swap (ranged-ai 0.4.0 live))
- **Last Landed Commit:**
```
38d2090 doc 24: C5b installed by DLL hot-swap (ranged-ai 0.4.0 live)
 patch_manager_mods/24_master_plan_status.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```
- **Dirty Files (139):**
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

## 4. Doc 20 session state (fix-phase1-expand-test-coverage, closed 10-05)
- **Done (all committed):** Phase 2 F1/F7/F9/F10/F11 (abc01d8); EquipCharPreview T1 (1d0ccab); T0 raw-address + subclasser gates (670905a); BuffDurationHUD 2.0.0 T1 (ce37f3e); F2 deleted; F13 fixed on KPM fork f743ac4 (ethansm/local-mods, create-patch.py skips *_test.cpp / t32_*.cpp; upstream PR NOT sent - needs user approval). Doc 20d is the findings ledger (F1-F26; F22-F24 resolved by doc21 session 22b9ba2).
- **Still open in doc 20 §5 (checked 10-05):**
  - #4 CharEffectsPanel: check_effect_decode.py still tests a PYTHON PORT; no C++ decoder test (no *_test.cpp in active_effects_panel_src).
  - #7 Inventory3DViewport: only Inv3dQuat_test + common_contract_test; viewport rect / model-fit (parity with inv3d_model_fit.py) / INI untested.
  - #9 RangedAI: still has its own parseMask (RangedAI.cpp:130) and inWeaponRange (:195) instead of cai:: copies.
  - #5 CompanionAI T2 fake-creature tests: not done.
  - T2 Common TestHarness.h: not built. T3: only _shared_src/vtable_override_test.cpp exists; WndProc chain test + struct static_asserts not done.
  - Low: K2WrapMeasureFix README, LoadHang/MouseProbe smokes assert, H6b in check_hook_bytes.
- **Done by others:** #6 LevelUp (scenarios.py), #8 PerfLab, #10 SafeRead/GuiKit (coverage 100/98.5), hotbar Phase T (1.1.0), --coverage/--mutate tooling (c6500c1).

## 5. Dead Ends & Traps (Do NOT Attempt)
- ❌ Plan F10 formula `2^32 - a`: wraps to 0 in 32-bit and still hangs; use UINTPTR_MAX - a.
- ❌ Editing mod .cpp files other sessions are refactoring without a claim: buff/shield split clobbered our edit via `git checkout --`. Check `git status`/ListAgents and send a claim notice first.
- ❌ `git add` of a file holding another session's WIP: stage only your hunk via `git hash-object -w` + `git update-index --cacheinfo` (used for PerfLab F9).
- ❌ `python3 -m py_compile` on KPM files: writes Patches/__pycache__ into the read-only KPM tree.
- ❌ Recreating KPM Patches/K2DialogueLetterboxFixProbe/: duplicate id breaks kpm-validate (probe lives in letterbox_probe_stage/).
- ❌ Running `check_raw_addresses.py --write-baseline` while other sessions have uncommitted addresses: it allowlists their WIP. Regenerate only your files' entries or wait.
- Plan facts that were wrong: "bc34464 stale" (it is the ForceArrowCounters commit); "create-patch.py excludes tests" (fixed now on the fork only).

## 6. Verification Commands
- `python3 scripts/run_tests.py` (default ~127 items, ~45 s; red items usually = other sessions' WIP - check git status first; at 10-05 `py:scripts/test_gen_queuehud_textures.py` was failing, queue-hud session's)
- `python3 scripts/run_tests.py --selftest`; `--build` (build_check); `--wine` (~3 min); `--coverage`; `--mutate <header>`
- `python3 scripts/build_check.py --mod <dir>`; `(cd $KPM_DIR && python3 tools/validate-patches.py)`
- Read-only proof: `touch` a marker, then `find $KPM_DIR -newer marker -not -path '*/.git/*'`
