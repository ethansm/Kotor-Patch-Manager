# Plan: holstered weapons (doc 42) — self-contained DLL mod, staged build

Status 2026-10-13: research waves 1-3 persisted (`research/holstered_weapons/`), Blender installed, P0 scaffold next. Doc: `patch_manager_mods/42_holstered_weapons.md`.

## Locked requirements (user)
1. Self-contained Patch Manager DLL (no shipped model/2DA/anim files); works with ANY weapon-override mod; reuse Common (`_shared_src`): K2Addr.h (+ k2addr_test), SafeRead.h, CodePatch.h, SceneView.h, ActQueue.h, ItemRead.h, Party.h, NpcWorld.h, KpLog.h.
2. ALL LIGHTSABERS (itemclass stems `w_Lghtsbr`, `w_ShortSbr`, `w_Dblsbr`; baseitems rows 8, 9, 10, live 106) on the HIP; vibroblades/other swords are NOT lightsabers. Hilt-only extent for placement.
3. Draw must not delay the first attack.
4. Two weapon sets: the unused set sits on the other mounts (hip L/R + back L/R); a swap animates stowing one and drawing the other.
5. Blender headless animation = learning side track, off the critical path.

## Decisions (user, 2026-10-13)
1. P1 scope = PC + party (NPCs later, after probe shows combat-hook behavior for them).
2. Other melee by length: short one-handed blades (vibroblade, short/long sword, stun baton, battleaxe by size check) hip; long two-handed (quarterstaff rows 0/107, force pike, double-bladed swords, warblade) back. Threshold from mesh bounds (~0.6 m), tuned in P2.
3. Blender 4.5.14 LTS installed: `~/apps/blender-4.5.14`, kotorblender 4.0.9 at `~/apps/kotorblender`, symlinked into `~/.config/blender/4.5/extensions/user_default/io_scene_kotor`. DONE.
4. P0 live install GO still required at P0 exit.

## Approach A (DLL only, attach to existing bones)
Bones on all 133 humanoid bodies: `pelvis_g`, `lthigh_g`, `rthigh_g` (hip); `torso_g`, `torsoUpr_g` (back). Case-insensitive match. Rejected: B (shipped MDL/anims: collides with 133 bodies and body mods), C (DLL-served embedded assets: no resman hook found; stretch only).

### Engine facts (Steam addresses)
- Weapon attach `FUN_007637e0` (void(u32, char* tag, int* creature)); node literal "rhand" pushed at 0x763af9, "lhand" at 0x763e2b; calls vtable +0x7c on `creature+0x68` with (obj, name, isRight, scale); handles creature+0x278/+0x27c; unequip = same call with placeholder 0x7f000000 + "". Callers: CreateBTypeBody 0x85cc00, 0x80efb0 case 3.
- Gob attach-by-name: vtable 0x98b5cc slot +0x50 = FUN_00461670(parent, nodeName, kind); miss = silent no-attach; copies node transform into child +0xa4 (vec3)/+0xb0 (quat). SetPosition 0x461490, SetOrientation 0x4614e0. Loader 0x462320. Re-copy each update in Gob handler 0x5047b0 (UpdateDependentPosition) so offsets must be applied inside a detour of it (one-time Set* calls lapse).
- Client combat flag `FUN_007682b0` (0x7682B0, __thiscall(newState), this+0x138, returns old); server flag creature+0x520. Refresh chain 0x76e130 -> 0x76d590 -> 0x76d890 (primary slots 0x10/0x20 only).
- Timing: ms countdowns (round 3000, attack 1500); first attack 0x5bb5a0 plays immediately. In-combat swap = round action 0x0E, 1000 ms (0x5917E0); out-of-combat swap immediate (0x47); executor 0x5DF920 swaps 0x10<->0x40000, 0x20<->0x80000 via unequip 0x5676D0 + equip 0x566650. Alt-slot items (creature +0x264/+0x268, slots 18/19) never get models: DLL owns model instances (item model resref at item+0x128).
- Animation: no holster clip; `draw_weapon` event has no known consumer; unarmed walk/run equals weapon-class walks.
- Weapons: grip at model origin; hilt = REFERENCE `ref_hilt`; hilt-only length single 0.34-0.37 m, short 0.22-0.26, double 0.52-0.56 (origin 0.21 m off grip center). Mesh bounds, not header.

## Design
Pure logic (unit-testable): category classify (itemclass stem -> LIGHTSABER/MELEE/PISTOL/RIFLE/HEAVY/QSTAFF; unknown -> weaponwield fallback), mount choice, per-category offset/orientation table in bone frame, state machine (DRAWN/HOLSTERED/MOVING per creature and set).
Engine glue (`Holster.cpp`):
1. H-ATTACH: wrap 0x7637e0; when HOLSTERED substitute bone name; apply offset in detour of 0x5047b0 only for Gobs in the mod registry. Any failure -> original hand attach.
2. H-COMBAT: hook 0x7682B0; 0->nonzero attach to hand BEFORE first swing; nonzero->0 re-attach to hip after exit countdown; "AppearanceForceUpdate" re-run of 0x7637e0 rebuilds.
3. H-SWAP: observe 0x5DF920 (or 0x566650/0x5676D0); stow outgoing, draw incoming; alt set on DLL-owned model instances.
4. Pseudo-draw (P3): interpolate child offset mount -> hand over min(swap 1000 ms, ~350 ms entry); skip when not visible.

## Phases
- P0 PROBE (log-only, `patch_manager_mods/holster_src/`, manifest 0.1.0-dev): logs 0x7682B0 old/new, 0x7637e0 tag/creature, 0x5DF920, 0x863650/0x5644b0 anim ids, creature fields, child Gob pointer; flag-file-gated visual experiment (one saber on pelvis_g with a fixed offset). Needs user GO to install.
- P1 MVP: instant hip<->hand swap, PC then party, all weapons on hip.
- P2 placement: per-category mounts + hilt-only bounds; alt-set mounts.
- P3 pseudo-draw slide.
- P4 stretch: arm pose via per-frame node rotation; authored clip only on user request.
Each phase: `python3 scripts/compat_check.py <files>`, scoped `run_tests.py holster`, `scripts/pre_install_gate.sh --mod holster_src` GATE PASS before `kpm_hotswap --apply`; play tests are the user's.

## Gates (answered by P0 probe)
G1 resolved by R1 (offsets lapse; hook 0x5047b0; open: world vs local space of node output, which per-frame path fires UpdateDependentPosition (vtable +0x1b4), recursion guard FUN_004dd0e0). G2 child Gob pointer location (creature+0x68 internals / +0x7c body). G3 does 0x7682B0 fire for NPCs or party only (0x8079B0 undecoded). G4 `draw_weapon` consumer. G5 swap hook refreshes models on client? G6 case-insensitive bone lookup. G7 saber attach origin vs grip center in-engine.
Wave 4 Haiku recon (optional, after P0): decode 0x8079B0/0x80DEB0; creature+0x68 class and +0x7c body; animations.2da rows 0x152-0x155; retry C2 resman hook.

## Process notes
Haiku agents hard stop 80k tokens; split REA briefs. REA sessions drop: retry open_binary with `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe`. `query_ghidra.py` is GOG-keyed and misses Steam addresses; Steam names in `ghidra_knowledge/steam_functions.tsv`.

## Verification
Offline: Common tests (k2addr, saferead), `holster_test.cpp` (classify, mount, offsets, state machine, swap pairing), compat_check, gate. Live (user): P0 logs via `kotor2_logs.py`; play checklist (saber hip in/out of combat, swap in combat does not delay swings, weapon-replacement mod works, party members, save/load, area transition).
