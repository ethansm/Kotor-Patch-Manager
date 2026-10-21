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
| HW3 | Offline weapon-on-bone preview: `weapons` command in `k1_port_tex_preview.py` (sub-plan A) | HW1 | - | DONE | 8abfc22 | weapons cmd + 2 tests; mesh skips missing; N_CommM/F bodies; skin bones via nodes_all; saber hilt via REFERENCE; identity bone R, offsets needed (doc 42 Offline preview) |
| HW4 | P0 install: Common sync into the real KPM tree, `pre_install_gate.sh --mod holster_src` GATE PASS, `kpm_hotswap` dry run + `--prepare`, REINSTALL (sub-plan B) | HW2 | user GO | INSTALLED | 007a38d | holster 0.1.0-dev INSTALLED 10-13 22:55 via KPatchLauncher (43 patches, 257 hooks; only holster.dll new, peer DLL md5s unchanged, LAA 0x23 ok); KPM 60fb05a Common + 6b6cd47 Holster; backup `$G/backups/holster_full_20261013_225517` (patches/, config, state, exe; rollback = restore these); no holster.ini in game dir yet (probe=0); gate: holster-scoped PASS, default tier 3 peer fails (npc_world/voice_probe raw-addr, area-hook-probe vs k1-spawn spans, kvoice lip) |
| HW5 | P0 play runs 1 (probe) and 2 (bone test) by the user; read logs | HW4 | user play | TODO | | `kotor2_logs.py check holster_log.txt --new` |
| HW6 | Answer gates G2-G7 in doc 42 from logs + preview; choose placement rotation + offsets and bone per category | HW3, HW5 | - | TODO | | write doc 42 section "P0 results" |
| HW7 | P1 MVP: instant hip<->hand swap, PC then party, all weapons on hip (sub-plan C) | HW6 | - | TODO | | plan after HW6 |
| HW8 | P2 placement: per-category mounts, hilt-only bounds, alt-set mounts | HW7 | - | TODO | | |
| HW9 | P3 pseudo-draw slide (cosmetic, capped, never delays an attack) | HW8 | - | TODO | | |
| HW10 | P4 stretch: arm pose via per-frame node rotation; authored clip only on user request | HW9 | user GO | DEFERRED | | |
| HW11 | Hardening: log tightening, drop probe detours, release notes; KPM PR only after the public-PR decision | HW9 | user GO | TODO | | stage 4 of the milestone lifecycle (MASTER_KEY section 4) |
| HW12 | Correctness guards: `+0x11E8` byte mask in `HolsterDelay.h`, weapon-model identity in the `Ent` cache key + `GOBREUSE` log, log-check counters (plan 42b sec 2.1) | - | - | INSTALLED | 240f320d | holster 0.5.2-dev LIVE 10-20 19:33, rollback holster_swap_20261020_193331; play rows in PLAY_TEST_QUEUE |
| HW13 | Flourish draw window (C2): per-party-slot lazy window from server state, `flourish_draw` key, hot-swap (42b 2.2) | HW12 | - | INSTALLED | b16085e8 | holster 0.5.2-dev LIVE 10-20 19:33, rollback holster_swap_20261020_193331; play rows in PLAY_TEST_QUEUE |
| HW14 | Draw/stow slide as `hs::Slide` per Ent, `draw_slide_ms`/`stow_slide_ms`, retire the unwired Fsm and its `onSwap` bug (42b 2.3); supersedes HW9 | HW13 | - | INSTALLED | baed1b86 | holster 0.5.2-dev LIVE 10-20 19:33, rollback holster_swap_20261020_193331; play rows in PLAY_TEST_QUEUE |
| HW15 | Unarmed-locomotion probe: `HolsterAnimStyle.h`, holstered set, ANIMRES `hol=`/`comb=` (SWAP) (42b 2.4) | HW14 | - | INSTALLED | f389c1cd | holster 0.5.2-dev LIVE 10-20 19:33, rollback holster_swap_20261020_193331; play rows in PLAY_TEST_QUEUE |
| HW16 | Unarmed override: detour 0x86367C writing the style byte, REINSTALL; ONLY if the HW15 probe gate passes | HW15 + play | user GO | GATED | | decide from probe logs |
| HW17 | Install HW12-HW15 as one hot-swap (0.5.2-dev), play rows FL1-FL4, SL-*, SW1/SW2, HW7a-P | HW15 | user GO | INSTALLED | c8a6eac1 | installed as 0.5.2-dev; play rows FL1-4 SL-* SW1/2 HW7a-P pending (user) |
| HW18 | Arm-reach spike recon (procedural IK, no clips): engine point that writes node transforms after animation sampling and before skinning; arm bone names and lengths on N_CommM / N_CommF | HW17 | - | DONE | | PASS w/ open Q3. Bone pose is part-local P+0x14 (quat)/P+0x08 (pos), NOT Gob+0xA4/B0. Writer chain Gob::Animate 0x4E1D40 (vt+0x14 slot 0x98B5E0; 5-byte CALL seam 0x481073). World recomputed on demand by 0x457510 so children follow. Skin consumer unlocated (0x45C810/0x4626D0). Write P+0x14 directly, never SetPartLocalPosition (freezes via +0x48). Files .claude/research/holster_reach/ |
| HW19 | Arm-reach probe build: `HolsterReach.h` (two-bone solver + blend, pure, tested) and a probe-gated detour that bends ONE arm toward the existing hip pose; log-only elsewhere (SWAP) | HW18 | user GO | DONE | c4f385dd | probe built holster 0.7.0-dev, GATE PASS, not installed; awaits install GO + play row RCH1; fallback seams in .claude/research/holster_reach/hw19_fallback_seams.md |

## Sub-plan A — HW3 offline preview (no game, no install)
Tool: `patch_manager_mods/k1_port/k1_port_tex_preview.py` (numpy + PIL software rasteriser; `mesh` runs in the k1port venv, renders in `.venv`). The `helmets` command already places one model at a named hook node of another; `weapons` reuses `HelmetKit.parts`, `hook_transform` and `compose_transforms`.
1. Pick body models that exist in the K2 install: names come from `.claude/research/npc_equipment/head_wearability.tsv` (column `body_model`; `P_BastilaBA` was NOT found). Working list (HW3): `N_CommM`, `N_CommF`; the P_*BA and PMBA/PFBA names are not in the K2 install.
2. `mesh --textures` dump of bodies and weapons: `w_lghtsbr_001`, `w_dblsbr_001`, `w_blstrpstl_001`, `w_blstrrfl_001`, `w_hvyblstr_001` (names to confirm from `items_k2.tsv` column `model_resref`) into the scratchpad.
3. Confirm the body dump `hooks` contains `pelvis_g`, `lthigh_g`, `rthigh_g`, `torso_g`, `torsoUpr_g` (they are NOT in `hooks`: skin bones are mesh nodes with texture NULL; the dump's `nodes_all` has them, done HW3).
4. Add a `weapons` subcommand: for each body x weapon x bone, draw the weapon with transform `compose_transforms(bone_hook, identity)`, views front / side / back, contact sheet out. Add a host test next to `test_k1_port_tex_preview.py`.
5. Optional later: pose one animation frame with Blender (`~/apps/blender-4.5.14/blender -b --python`, add-on `bl_ext.user_default.io_scene_kotor`) to compare against rest pose.
Verification: `.venv/bin/python -I -m pytest patch_manager_mods/k1_port/test_k1_port_tex_preview.py -q` (or `python3 scripts/run_tests.py k1_port_tex_preview`), then eyeball the sheet.

## Sub-plan B — HW4 install (needs user GO)
Per skill `kotor2-kpm-install`: `python3 scripts/kpm_status.py`; sync Common into the real KPM tree as its own approved step; `scripts/pre_install_gate.sh --mod holster_src` until `GATE PASS`; `python3 scripts/kpm_hotswap.py --mod holster` dry run; `--prepare`; commit repo + KPM by explicit paths; REINSTALL (ask user); `--apply`. Rollback: `python3 scripts/kpm_hotswap.py --rollback latest`.
Play checklist for HW5: `patch_manager_mods/42_holstered_weapons.md` section "P0 play-test checklist".

## Sub-plan C — HW7 P1 MVP (outline, refine after HW6)
H-ATTACH wrap of 0x7637E0 (substitute bone when HOLSTERED, rotation + offset applied inside a detour of 0x5047B0 for registry Gobs only, any failure falls back to the hand attach); H-COMBAT on 0x7682B0 (attach to hand before the first swing, back to hip after the exit countdown); H-SWAP on 0x5DF926 (stow outgoing, draw incoming). Tests: `holster_logic_test.cpp` state machine, classify, mount; `compat_check.py`; scoped `run_tests.py holster`; gate GATE PASS before `--apply`. Play checklist: saber on hip in and out of combat, swap in combat does not delay swings, weapon-replacement mod works, party members, save/load, area change.

## Open gates (from the P0 probe; answers go to doc 42)
G2 child Gob pointer location; G3 does 0x7682B0 fire for NPCs and party; G4 `draw_weapon` consumer; G5 swap hook refreshes models on client; G6 case-insensitive bone lookup (bone_test); G7 saber origin vs grip on the hip (preview + bone_test); G1 world vs local node space (bone_test).
