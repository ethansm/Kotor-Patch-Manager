# Session Handoff & Plan State: patch_manager_mods/42_holstered_weapons_master_plan.md

## 0. Next-Phase Direction
user play run (bone_test=1 pelvis_g), then read holster_log.txt; run 3 place=1 live tuning per category; then T4/HW6

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/42_holstered_weapons_master_plan.md`
- **Waves:** .claude/waves/WI-42-a1/
- **Waves:** .claude/waves/WI-42-a2/
- **Waves:** .claude/waves/WI-42-b2/
- **Waves:** .claude/waves/w1-42-HWB/
- **Branch:** `KOTOR-II` (021b2c3 WI-24: Active effects panel: logging and shared-helper migration (manual close))
- **Generated:** 2026-10-15 20:38:45 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
021b2c3 WI-24: Active effects panel: logging and shared-helper migration (manual close)
 .claude/work_items/dev_work_items.md                             | 2 +-
 .claude/work_items/runs.jsonl                                    | 1 +
 .claude/work_items/state.jsonl                                   | 1 +
 patch_manager_mods/PLAY_TEST_QUEUE.md                            | 1 +
 patch_manager_mods/active_effects_panel_src/CharEffectsPanel.cpp | 6 +++---
 5 files changed, 7 insertions(+), 4 deletions(-)
```
- **Dirty Files (154):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/42_holstered_weapons_master_plan.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M patch_manager_mods/34_npc_diversity_dispatcher.md`
  - ... +144 more (run `git status --short`)

## 2. Immediate Objective (HW6)
**Answer gates G2-G7 in doc 42 from logs + preview; choose placement rotation + offsets and bone per category**

Status: TODO
Task: Answer gates G2-G7 in doc 42 from logs + preview; choose placement rotation + offsets and bone per category

## 2b. Upcoming Batches in Pipeline
- **HW5** : P0 play runs 1 (probe) and 2 (bone test) by the user; read logs
- **HW7** : P1 MVP: instant hip<->hand swap, PC then party, all weapons on hip (sub-plan C)
- **HW8** : P2 placement: per-category mounts, hilt-only bounds, alt-set mounts

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/42_holstered_weapons_master_plan.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate HW6` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- Dead end: `k1_port_tex_preview.py mesh --models P_BastilaBA ...` -> "MDL/MDX not found" (model name not in the K2 install; note head_wearability.tsv also spells it `P_BastillaBB`/`P_BastilaBA` inconsistently). The batch aborted on the first model, so NO weapon/body JSON exists yet; check each name alone.
- Dead end: check_hook_bytes rejects a 4-byte immediate inside .text (kind "data" needs .data/.rdata); patch the whole 5-byte `push` with kind "code" (done in 4e85582).
- Preview commands: mesh dump `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I patch_manager_mods/k1_port/k1_port_tex_preview.py mesh --textures --models <names> --out <scratchpad>/hm`; render with `.venv/bin/python -I ...`. Weapon model names to confirm from `.claude/research/k1_items/items_k2.tsv` column model_resref.
- Verification: `python3 scripts/run_tests.py holster k2addr`; `python3 scripts/check_hook_bytes.py --mod holster`; scratch-tree `build_check --kpm-dir <scratch> --mod holster_src`.
- RESOLVED (HW3, 8abfc22): the `P_BastilaBA` dead end above. `mesh` now prints NOT FOUND and continues. Working bodies are `n_commm`, `n_commf`; P_*BA, PMBA, PFBA are not in the K2 install. Pass LOWERCASE model names (HelmetKit/WeaponKit look up `<name>.json` in lowercase).
- Dead end: identity-rotation weapon placement (the first HW3 sheets) says nothing about how a holstered weapon sits: the bone node supplies no weapon rotation, and skin bones `pelvis_g/torso_g/torsoUpr_g` have identity rotation, thighs about 6 deg. A weapon keeps its model axes (saber hilt +z emitter up, blaster barrel +y forward, grip at origin; double saber origin at the LOWER end). `--hand-rot rhand` is also wrong: the rest pose is a T-pose, so the held rotation points forward/outward.
- Pose presets that look right (N_CommM rest pose, `weapons --rot/--offset`): blaster barrel-down on thigh `--rot -90 0 0 --offset 0.12 0 -0.03` (bones rthigh_g/lthigh_g; mirror x for left); saber emitter-down on hip `--rot 180 0 0 --offset 0.2 0 0` (pelvis_g, double saber reaches the knee). x = character's right, y = forward, z = up, applied in the bone frame.
- P1 consequence: the detour of 0x5047B0 (UpdateDependentPosition) must apply ROTATION + offset to the substituted-bone transform, not only an offset (sub-plan C wording is stale).
- Animation data is reachable offline: `n_commm` has 0 anims, supermodel S_Female02 (232 anims in s_male02, 88 in s_female02: names like `g3r1`, `g4r1`, `pause1`), chain continues to S_Female01. Probe script pattern: `venvs/k1port/bin/python -I <script> <game> <model...>` with `k1_port_uvmask.install_get` + `pk.read_mdl`, then `mdl.anims[i].name`. Not yet used to pose a body.
- Install facts (HW4): holster is a NEW id, so `kpm_hotswap.py` refuses it and `scripts/kpm_reinstall.py` does NOT exist. Install used `stage_and_verify_install.py stage <dir> <Holster.kpatch> --allow-dirty` + `KPATCH_NO_LAUNCH=1 KPatchLauncher ... --patches <dir> <ids>` (backup `$G/backups/holster_full_20261013_225517`). `Holster.kpatch` is gitignored; build it with `kpm_hotswap.mirror` + `run_create_patch` (python import). Default-tier gate fails 3 suites in peers (npc_world/voice_probe raw-addr, area-hook-probe vs k1-spawn-probe spans, kvoice lip): not holster; holster-scoped gate used `pre_install_gate.py --mod holster_src --skip-default`.
- State now: holster 0.1.0-dev LIVE but INERT (no `holster.ini` in the game dir, probe defaults 0, nothing logged). No play test has run. Haiku sheet check (a.md, b.md in `.claude/research/holstered_weapons/haiku_check/`) found nothing refuted; N_CommF double saber also shows above the neck at torso_g.
- Contract caveat: `agent_contract.py check` needs the reply as a positional argument (`check --scope <globs> -` with stdin); I ran it without `-` and it errored, so the two Haiku replies were NOT contract-checked.

### Session 10-15 lessons (play run 1 + hot-swap)
- LANDED: B1 pose evaluator `9da9585` (`k1_anim_pose.py table`; pelvis moves <=12 deg walk/run, thighs 32-39 deg walk / 72-74 deg run => saber on pelvis_g, blasters on thigh only if swing acceptable); A2 analyzer `1fbadd5` (`holster_log_check.py` + `kotor2_logs.py` route `holster_log.txt`; GOB quat order is w-first, doc 01 line 85); A3 recon `dc11242` (`.claude/research/holster_a3_5644B0.md`: 0x5644B0 is NOT an anim sender, it is a 17-dword-arg action enqueue, ret 0x44, arg1 = 16-bit sentinel 0xFFFE/0xFFFF, so ANIMSEND lines are meaningless; hook is a detour that returns to the original, harmless).
- Run 1 (10-15 00:17) was USELESS: 3930 `ANIMRES id=0x2710` lines (idle id, every frame) + kplog rate limiter (DROPPED ~2000, "ring 0, rate") ate the budget; log covers 26 s of load only: COMB 19 lines all edge=0, ATTACH 51 all appearance/Loading obj=0 handleR=7f000000, SWAP 0, GOB 0. Do NOT conclude GOB/SWAP hooks are dead from this run.
- FIX `2e804b8`: cap id 0x2710 at 20 lines (`g_idleAnim`). Hot-swapped LIVE 10-15 00:45 (kpm_hotswap prepare/apply, KPM 82aae6f, backup `$G/backups/holster_swap_20261015_004524`, md5 c3c9bea2). holster.ini (probe=1, all ch_*=1, gob_every=120, max_lines=6000, bone_test=0) + `[holster]` kplog block are in the game dir. Run 1 must be REPEATED by the user.
- Unverified: whether kplog's per-patch rate limit can still starve gob/comb lines; if the repeat log again ends early, lower gob_every pressure or set rate in kplog.ini.
- Hot-swap works for holster now (it is no longer a new id): `scripts/pre_install_gate.sh --mod holster_src --skip-default` -> `kpm_hotswap.py --mod holster --prepare` -> commit KPM side -> `--apply`. Only HOOK changes need a full reinstall.
- Planning decision (user asked 10-15): build pure logic now (placement math, table seed, ini keys/reload, draw slide), do static recon C1 now, batch all hook changes into ONE reinstall after run-1/2 data.

### Session 10-15 afternoon (buildable-work plan executed; reinstall LIVE)
- LANDED (repo): T7 docs `cb08818`; T2 `HolsterTune.h` `66958fb`; T1 `HolsterQuat.h` `5b4e6be` (applyPlacement = node.q*qbase, pos = node.pos + rot(node.q, off)); T8 analyzer PLACE kind `bc653a5` (quat1 tol 2e-3 because %.3f printing); T3 `HolsterSlide.h` `4c1c70f`; R1 back sheets `ce1c514`; R2 C3 recon `d2ac7c6`; T5 `HolsterPlace` `b8bb28e`; T6 toml H7 + drop H5/AnimSend `ad0444a`; hotkey/auto-mode recon `56fc5f0`. KPM side `b1e59b3`.
- GO#1 hot-swap was SKIPPED (folded into the reinstall: repo moved past the prepared swap, status went MIRROR). GO#2 reinstall DONE 10-15 16:20: `kpm_reinstall.py --mod holster`, 45 patches, holster LIVE md5 49fa2bc1, 0 peer DLLs restored, backup `$G/backups/holster_reinstall_20261015_162001` (rollback `kpm_reinstall.py --rollback latest`).
- LIVE game-dir holster.ini at install time: probe=1, all ch_*=1 (no ch_place), gob_every=120, max_lines=6000, **bone_test=1**, bone_name=pelvis_g (the user/other session set bone_test=1, so the next play run merges run 1 + run 2). Run 3 adds `ch_place=1 place=1 place_rot=180,0,0 place_off=0.20,0,0`; tune live (reload <= 0.5 s); stop rule: `place=0` live first, rollback only if that fails.
- R2 facts (`.claude/research/holster_c3_nodeclass.md`): N->vtbl+0xC = 0x457510 returns the WORLD pose (parent chain + owner Gob pose); node layout name handle +0x04, pos +0x08, quat +0x14, children +0x30/+0x34, parent +0x3C, owner Gob +0x40; name = table[index] via H=[N+4], idx=[H+4], owner=[H+8], t=[owner+0x4C]&0x7F (2: [owner+0xB8]; 5: [[owner+0x84]+0xB8]); FindPart 0x4587F0 thiscall RET 4 (stricmp, "root" special). R2 ran on exe sha 7751f7d8 (toml target 6A522E71; check_hook_bytes PASSed 0x5047F4). OPEN: per-frame vs on-move delivery; anim vs bind pose of node fields.
- HolsterPlace filter: name compare vs g_boneName, cache 128 {node,gob,handle[N+4],isTarget}; optional `place_gobvt`; writes 28 bytes gob+0xA4 only with probe=1 bone_test=1 place=1; logs `TID gobupdate=`/`TID place=` once (check same thread in run log), `RELOAD ...` on ini change.
- Back-mount seeds (user asked "higher + flush", raised z +0.11, y ~+0.05; Opus viewed renders): bone torsoUpr_g; rifle 45,0,-90 / -0.20,-0.09,-0.10 (muzzle slightly over shoulder); heavy 110,0,-25 / 0.03,-0.11,-0.10; double saber 0,42,0 / -0.106,-0.06,-0.10. No shoulder bone exists above torsoUpr_g except the arm chain (moves with arms). Seeds are commented in holster_src/holster.ini.
- Latent bug (CONFIRMED, not fixed, unwired): `Fsm::onSwap` (HolsterLogic.h:200) out of combat sets Drawing and clears stowPending; no combat edge follows, so the weapon stays Drawn forever. Fix in HW7: when lastCombat==0 schedule stow at now+kSwapMs+kStowDelayMs; add a host test.
- Hotkey / auto mode (user asked; `.claude/research/holstered_weapons/hotkey_*.md`, `automode_*.md`): no shared keyboard lib (gui-input = wheel/cursor/focus/HWND only; action-hotbar polls GetAsyncKeyState, combat-hotbar hooks DirectInput; F9-F11 there has no modal gate, unverified live). Keymap menu likely from keymap.2da (loader Steam 0x6EA570) + swkotor2.ini [Keymapping]; a 2da row is display-only, the DLL must act on the key; OPEN: does CSWGuiInGameOptKeyMappings loop 2da rows or a constant (needs REA decompile). Gameplay page optgameplay_p.gui (CSWGuiInGameGameplay): a .gui checkbox is inert without a DLL handler; main-menu Options page identity unconfirmed. Toggle template = companion-selfheal Auto pill (SelfHealUi.h:244) + FileWatch::atomicWrite. Recommendation: ini `auto=`/`hotkey=` with HW7 (cosmetic until hip<->hand works), real menu entries as a later phase starting with the REA keymap decompile.
- Tooling notes: `agent_contract.py check - --scope <globs>` (the `-` must come BEFORE --scope; --scope is nargs+). pre_install_gate's check-exports reads the stale KPM kpatch before `--prepare`: a removed handler shows as FAIL until mirrored (BUILD_AND_INSTALL.md:81); re-gate after prepare. holster.ini sample is not mirrored to KPM.
- Still pending in plan: play runs (user), read log with `kotor2_logs.py check holster_log.txt --new`, HW6 gates, T4 seed placementFor with TUNED numbers, GameAddr.h:77-78 comment fix (separate commit + compat_check), doc 42 "Pose" section, mark-done HW5/HW6.
- PLAY RUN 2 (10-15 evening, bone_test=1 pelvis_g; user screenshots): bone_test WORKS in game: PC sword + Atton blaster attach to pelvis_g and follow the body while running; orientation is the weapon's raw model axes (saber blade straight up past the head, blaster barrel forward), as the preview predicted => place_rot is required. One unexplained pistol floats beside the PC's head in screenshot 1 (open). Log: cap 6000 hit, ANIMRES 0x2711 x3976 / 0x2718 x1900 flooded it, GOB 0 lines (starved, not dead): `TID gobupdate=324 place=324` => both hooks fire, SAME thread. COMB 29 edges / 11 creatures; ATTACH 37; SWAP 0.
- FIX `5661376`: ANIMRES capped at 20 lines per id; analyzer status shows TID + RELOAD. Prepared as SWAP (kpatch md5 b5aa5e13, KPM 1fed956), fresh GATE PASS after prepare; `kpm_hotswap.py --mod holster --apply` awaits user GO (game closed).
- PEER LIVE TEST (session holster-live-test-plan, 10-15 ~23:55; report `.claude/isolated/holster_test/RESULTS.md`, tool `scripts/holster_place_verdict.py` 0dec082): with place=1 from boot, PLACE n=0 (HolsterPlace called, TID place=328, but refused silently); attach handler 0x5047B0 fired <24 times per session (GOB lines=12) => it is EVENT-driven, not per frame; RELOAD never fired. G3: Atton's blaster also lands at pelvis_g. Peer left G/holster.ini at place=0 ch_place=1 ch_anim=0 max_lines=30000 (backups holster.ini.bak_*_20261015).
- RESPONSE `dc45940` (KPM 47a8e6c, kpatch dd330a9a, SWAP prepared, fresh GATE PASS, NOT applied, needs user GO): PLACEREFUSE per reason, PLACENODE name/type/step per new node, TUNE boot line, maybeReload also polled from the per-frame HolsterAnimResolve. Analyzer summarises them. OPEN after next run: which refusal fires; if the pose is written, does it persist between attach events (if the engine does not rewrite per frame, the gob pose may be parent-local after all).
- TODO: pure placement-gate decision function in HolsterLogic.h for host tests (peer F6).
