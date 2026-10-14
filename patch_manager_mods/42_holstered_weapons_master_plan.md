# 42 — Holstered weapons: master plan and sub-plans (tracking)

Feature doc (research, facts, gates): `patch_manager_mods/42_holstered_weapons.md`. Design and locked requirements: `.claude/plans/holstered_weapons.md`.
Mod source: `patch_manager_mods/holster_src/` (manifest id `holster`, 0.x.y until the KPM upstream PR). Handoff: `.claude/handoffs/42_holstered_weapons_master_plan.handoff.md`.
Resume: `python3 scripts/plan_manager.py resume patch_manager_mods/42_holstered_weapons_master_plan.md`. Mark: `python3 scripts/plan_manager.py mark-done <Id> --state <STATE> --commit <sha> --notes "<txt>"`.

## Goal (user, locked)
Characters visibly stow weapons out of combat. ALL lightsabers on the hip, other weapons by category on hip or back, two weapon sets on four mounts (hip L/R, back L/R). Self-contained DLL (no shipped model/2DA/anim files), works with any weapon-override mod, draw never delays the first attack. Scope P1 = PC + party; NPCs later.

## Traps (read before any batch)
- Real KPM tree `Patches/Common` lacks the new K2Addr/GameAddr fields: `build_check` / `compat_check` against it FAIL (~44, holster compile error). Verify on a scratch copy: copy `Patches/Common`, `Patches/_shared`, `lib`, `create-patch.py` into the scratchpad, `KPM_DIR=<scratch> python3 scripts/check_shared_headers.py --sync`, then `python3 scripts/build_check.py --kpm-dir <scratch> --mod holster_src`. Never run `--sync` against the real tree inside an install step.
- cmdq-probe owns the 0x5DF920 entry; HolsterSwap stays at 0x5DF926. Two hooks on one site fail `check_hook_spans`. The two FAILs area-hook-probe vs k1-spawn-probe are not holster's.
- Never `cat`/`grep` raw mod logs: `python3 scripts/kotor2_logs.py status | check <log> [--new]`.
- Live install needs explicit user GO and the game closed. A new mod id with hooks means REINSTALL, not hot-swap. Play tests are the user's.
- Many unrelated dirty files from other sessions: commit by explicit paths only.
- The offline preview renders the REST pose; in game the weapon follows the animated bone. Preview answers placement, origin and end orientation, not G6 (engine name lookup) or world-vs-local node space.

## Master plan table

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| HW0 | Research waves 1-3 (33 Haiku/REA agents), decision Option A (DLL only, attach to existing bones) | - | - | DONE | e6a8bb0 | - |
| HW1 | P0 probe scaffold: 6 log-only detours, `HolsterLogic.h` + tests, contract test, K2Addr/GameAddr holster addresses | HW0 | - | DONE | e9fef28 | - |
| HW2 | Flag-file `bone_test` (rhand/lhand attach-node literal repoint, default off), `code_patches.toml`, contract tests, play-test checklist | HW1 | - | DONE | 4e85582 | host tests + hook bytes PASS, scratch build 7 exports |
| HW3 | Offline weapon-on-bone preview: `weapons` command in `k1_port_tex_preview.py` (sub-plan A) | HW1 | - | PARTIAL | | mesh dump P_BastilaBA failed (model not in K2 install); verify body names; weapons cmd not written |
| HW4 | P0 install: Common sync into the real KPM tree, `pre_install_gate.sh --mod holster_src` GATE PASS, `kpm_hotswap` dry run + `--prepare`, REINSTALL (sub-plan B) | HW2 | user GO | TODO | | ask user for GO with the game closed |
| HW5 | P0 play runs 1 (probe) and 2 (bone test) by the user; read logs | HW4 | user play | TODO | | `kotor2_logs.py check holster_log.txt --new` |
| HW6 | Answer gates G2-G7 in doc 42 from logs + preview; choose placement offsets and bone per category | HW3, HW5 | - | TODO | | write doc 42 section "P0 results" |
| HW7 | P1 MVP: instant hip<->hand swap, PC then party, all weapons on hip (sub-plan C) | HW6 | - | TODO | | plan after HW6 |
| HW8 | P2 placement: per-category mounts, hilt-only bounds, alt-set mounts | HW7 | - | TODO | | |
| HW9 | P3 pseudo-draw slide (cosmetic, capped, never delays an attack) | HW8 | - | TODO | | |
| HW10 | P4 stretch: arm pose via per-frame node rotation; authored clip only on user request | HW9 | user GO | DEFERRED | | |
| HW11 | Hardening: log tightening, drop probe detours, release notes; KPM PR only after the public-PR decision | HW9 | user GO | TODO | | stage 4 of the milestone lifecycle (MASTER_KEY section 4) |

## Sub-plan A — HW3 offline preview (no game, no install)
Tool: `patch_manager_mods/k1_port/k1_port_tex_preview.py` (numpy + PIL software rasteriser; `mesh` runs in the k1port venv, renders in `.venv`). The `helmets` command already places one model at a named hook node of another; `weapons` reuses `HelmetKit.parts`, `hook_transform` and `compose_transforms`.
1. Pick body models that exist in the K2 install: names come from `.claude/research/npc_equipment/head_wearability.tsv` (column `body_model`; `P_BastilaBA` was NOT found). Try `P_BastillaBB`, `N_CommM`, `N_CommF`, and the PC body models; record the working list here.
2. `mesh --textures` dump of bodies and weapons: `w_lghtsbr_001`, `w_dblsbr_001`, `w_blstrpstl_001`, `w_blstrrfl_001`, `w_hvyblstr_001` (names to confirm from `items_k2.tsv` column `model_resref`) into the scratchpad.
3. Confirm the body dump `hooks` contains `pelvis_g`, `lthigh_g`, `rthigh_g`, `torso_g`, `torsoUpr_g` (they are mesh-less dummy nodes only if the dump lists them; skin bones may be missing, then read them via pykotor node walk).
4. Add a `weapons` subcommand: for each body x weapon x bone, draw the weapon with transform `compose_transforms(bone_hook, identity)`, views front / side / back, contact sheet out. Add a host test next to `test_k1_port_tex_preview.py`.
5. Optional later: pose one animation frame with Blender (`~/apps/blender-4.5.14/blender -b --python`, add-on `bl_ext.user_default.io_scene_kotor`) to compare against rest pose.
Verification: `.venv/bin/python -I -m pytest patch_manager_mods/k1_port/test_k1_port_tex_preview.py -q` (or `python3 scripts/run_tests.py k1_port_tex_preview`), then eyeball the sheet.

## Sub-plan B — HW4 install (needs user GO)
Per skill `kotor2-kpm-install`: `python3 scripts/kpm_status.py`; sync Common into the real KPM tree as its own approved step; `scripts/pre_install_gate.sh --mod holster_src` until `GATE PASS`; `python3 scripts/kpm_hotswap.py --mod holster` dry run; `--prepare`; commit repo + KPM by explicit paths; REINSTALL (ask user); `--apply`. Rollback: `python3 scripts/kpm_hotswap.py --rollback latest`.
Play checklist for HW5: `patch_manager_mods/42_holstered_weapons.md` section "P0 play-test checklist".

## Sub-plan C — HW7 P1 MVP (outline, refine after HW6)
H-ATTACH wrap of 0x7637E0 (substitute bone when HOLSTERED, offset applied inside a detour of 0x5047B0 for registry Gobs only, any failure falls back to the hand attach); H-COMBAT on 0x7682B0 (attach to hand before the first swing, back to hip after the exit countdown); H-SWAP on 0x5DF926 (stow outgoing, draw incoming). Tests: `holster_logic_test.cpp` state machine, classify, mount; `compat_check.py`; scoped `run_tests.py holster`; gate GATE PASS before `--apply`. Play checklist: saber on hip in and out of combat, swap in combat does not delay swings, weapon-replacement mod works, party members, save/load, area change.

## Open gates (from the P0 probe; answers go to doc 42)
G2 child Gob pointer location; G3 does 0x7682B0 fire for NPCs and party; G4 `draw_weapon` consumer; G5 swap hook refreshes models on client; G6 case-insensitive bone lookup (bone_test); G7 saber origin vs grip on the hip (preview + bone_test); G1 world vs local node space (bone_test).
