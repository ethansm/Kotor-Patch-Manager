# Session Handoff & Plan State: patch_manager_mods/21_portrait_slots_and_split.md

## 0. Next-Phase Direction
go with b and v1 ok

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/21_portrait_slots_and_split.md`
- **Branch:** `KOTOR-II` (85b2dbd doc 24 L3 DONE (built, not installed; site list corrected; hotbar migration owner-gated))
- **Last Landed Commit:**
```
85b2dbd doc 24 L3 DONE (built, not installed; site list corrected; hotbar migration owner-gated)
 patch_manager_mods/24_master_plan_status.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```
- **Dirty Files (128):**
  - `M .claude/skills/plan-handoff/SKILL.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/README.md`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/atlas_coverage.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/cg.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/export_dump_inputs.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/field_census.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/helpers.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_cg.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_export_and_coverage.cpython-312.pyc`

## 2. Immediate Objective (W2 shared headers)
**85eb91f; **live** `buff-duration-hud.dll` `feed7a4b` (single DLL, same behaviour; backup `backups/shield_split_w2_install_20261003_121921`); rollback `kotor-patch-manager/shield_live_patches/` (b7dbebe9 + art)**

Status: 85EB91F; **LIVE** `BUFF-DURATION-HUD.DLL` `FEED7A4B` (SINGLE DLL, SAME BEHAVIOUR; BACKUP `BACKUPS/SHIELD_SPLIT_W2_INSTALL_20261003_121921`); ROLLBACK `KOTOR-PATCH-MANAGER/SHIELD_LIVE_PATCHES/` (B7DBEBE9 + ART)
Task: 85eb91f; **live** `buff-duration-hud.dll` `feed7a4b` (single DLL, same behaviour; backup `backups/shield_split_w2_install_20261003_121921`); rollback `kotor-patch-manager/shield_live_patches/` (b7dbebe9 + art)

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

## Session additions (doc21 follow-ups, 10-04 23:30)
- User: alignment option (b) chosen, PortraitSlots keep-v1 OK. Buff 2.0.1 LIVE 23:24 (5af10089, backup backups/buff_201_install_20261004_232401).
- Dead ends / traps: `pgrep -f swkotor2` matches your own bash command line -> use `pgrep -af "^S:.*swkotor2"`. KPM Patches/Common has sibling WIP (GuiKit.h FillStyle, K2Addr.h Border_FillStyle): build kpatches from a clean `git worktree add --detach <scratch> <sha>`, then `git worktree remove`. Live config changes under you (67da27ea at 23:24): compare the patch's [[patches]] block, not the whole md5.
- pytest isn't installed: use PYTHONPATH=<scratchpad>/pt or pip --target into the scratchpad.
- Verify: python3 scripts/run_tests.py -k shield_logic / -k common_contract; python3 scripts/build_check.py --mod shield_bar_src
