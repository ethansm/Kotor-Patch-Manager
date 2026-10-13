# 42 — Holstered weapons (hip/back hardpoints) — research

Status: RESEARCH DONE, P0 probe not yet built (2026-10-13). Wave 1 = 10 Haiku web recon agents (all PARTIAL); waves 2-3 = 33 Haiku/REA agents on the Steam exe (raw: `research/holstered_weapons/wave2_3.md`). Decision: Option A (DLL only, attach to existing bones). Nothing installed.
Raw findings (URLs per claim): `research/holstered_weapons/raw/holster_*.md`.

## Goal
Characters visibly stow weapons out of combat: new attach points on hips (L/R) and back (L/R), weapon moves hand <-> hardpoint with a draw/holster animation; run/action animations while holstered.

## Bottom line
A model-only change cannot do it. Needs (1) new dummy nodes in character models, (2) a native hook (KPM) that redirects the weapon attach node, (3) a trigger on combat state, (4) optional draw/holster animations. No public KOTOR 1/TSL holster mod exists (nothing released, nothing WIP found).

## Established (with source)
- Engine reimplementations hardcode attach node literals `rhand`, `lhand`, `headhook`, `gogglehook`, `talkdummy`, `camerahook`, `hturn_g`; no 2DA column names them (reone `creature.cpp`, KotOR.js `ModuleCreature.ts`/`OdysseyModel3D.ts`). The real `swkotor2.exe` is NOT yet checked.
- Weapon model name = baseitems `itemclass` + `_NNN` (modelvariation). `weaponwield` 1..6 -> hardcoded combat anim numbers 1,2,3,5,7,9 (unarmed 8). Draw is an animation name (`g%dw%d`), not a visibility toggle (reone).
- MDL node type 0x0001 = dummy; parent index at +0x0C; nodes resolved by name table (rakata docs). Supermodel name = 32-byte header field; animation lookup walks the supermodel chain; community chain S_Female03 -> S_Female02 -> S_Female01 -> S_Male02 -> S_Male01 (K1-leaning thread).
- TSL has no visible belt slot (only body + head models), so a belt-baked static weapon is not an option; armor-baked variants would need one per armor.
- OpenMW PR #1615 is the working precedent: named sheath nodes + engine-side attach swap on draw/holster.
- KPM (LaneDibello/Kotor-Patch-Manager): runtime DLL injection (binkw32 proxy, Proton OK), hook types simple/replace/detour/static. Public K1 Ghidra share names CSWCObject/CSWCCreature; no public TSL symbols for weapon attach.

## Tool matrix (verified vs Gemini claims)
| Step | Tool | Verdict |
|---|---|---|
| BIF/KEY/ERF/RIM | xoreos-tools `unkeybif keybif unerf erf unrim rim` | real (Gemini's `nwnbif/nwnerf` do NOT exist); MOD untested |
| MDL bin<->ASCII | MDLOps (Perl, ndix UR, v1.0.0 2017, K1+K2, CLI) | real, best CLI option; MDL+MDX same dir |
| MDL | rakata (Rust, ASCII reader/writer) | candidate, docs excellent |
| MDL | mdledit (bead-v) | GUI only, no CLI documented, Windows/Wine |
| MDL | KAurora | NO ASCII export; not usable |
| Blender | KotorBlender (OpenKotOR maintained; seedhartha older) | binary MDL import/export + animations; dummy nodes and headless `blender -b` UNVERIFIED |
| Patching | HoloPatcher (PyKotor, LGPL) `--install --validate --game-dir` | headless on Linux per docs, not run |
| Mocap | AMASS | non-commercial licence: avoid |

## Gemini claims corrected
`dummy_rhand` naming (real: `rhand`/`lhand`, unverified in exe); `nwnbif`/`nwnerf`; MDLedit CLI; KAurora decompile; "Switch Weapons for K1" (unverified, no hit).

## Open questions (carry to wave 2)
1. Which `swkotor2.exe` function attaches the weapon model to rhand/lhand, and where is the string `rhand` referenced?
2. Do vanilla K2 character MDLs actually contain rhand/lhand/headconjure/handconjure/torso_g? (MDLOps dump of a real model.)
3. Supermodel vs per-model geometry: does one edit propagate to all humanoids? How many humanoid models/supermodels in K2?
4. Do animation events (event array in `newanim`) have any engine-side effect? Is weapon show/hide hardcoded?
5. Does the engine re-equip/re-attach on combat start; what is the combat-state signal to hook?
6. KotorBlender: dummy-node round trip, animation `transtime`/event fidelity, headless export.
7. Does vanilla hide/show hilts out of combat? (in-game check)

## Waves 2-3 findings (Steam addresses; details in `research/holstered_weapons/wave2_3.md`)
- Bones on all 133 humanoid bodies: `pelvis_g`, `lthigh_g`/`rthigh_g` (hip), `torso_g`, `torsoUpr_g` (back); no holster dummies anywhere; supermodel edits do not reach bodies. Match names case-insensitively.
- Weapon attach: `FUN_007637e0` (0x7637e0), node literal "rhand" pushed at 0x763af9, "lhand" at 0x763e2b; vtable +0x7c on `creature+0x68`; handles creature+0x278/+0x27c. Gob attach-by-name: vtable 0x98b5cc slot +0x50 = FUN_00461670; node transform re-copied every update by Gob handler 0x5047b0, so offsets must be applied inside a detour of it.
- Client combat flag: `FUN_007682b0` (stores this+0x138); server flag creature+0x520. Weapon refresh chain 0x76e130 -> 0x76d590 -> 0x76d890.
- Timing is ms countdowns (round 3000, attack 1500), not animation driven; first attack plays immediately. In-combat weapon swap = action 0x0E, 1000 ms; executor 0x5DF920. Alt-slot items (slots 18/19) never get models, so the DLL owns their model instances.
- No holster clip exists; `draw_weapon` event (0.03-0.57 s in 12 clips) has no known consumer. Unarmed walk/run keyframes equal weapon-class walks.
- Weapon geometry: grip at model origin; lightsaber hilt = REFERENCE subtree `ref_hilt`; hilt-only length single 0.34-0.37 m, short 0.22-0.26, double 0.52-0.56 (origin 0.21 m off grip center).

## Decision (user, 2026-10-13)
Option A: self-contained DLL, no shipped assets. All lightsabers on the hip; short one-handed blades hip, long two-handed back (threshold about 0.6 m from mesh bounds); two weapon sets on hip L/R + back L/R; draw must not delay the first attack; P1 = PC + party. Phases P0 probe (log-only) -> P1 MVP -> P2 placement -> P3 pseudo-draw slide -> P4 stretch. Blender 4.5.14 LTS installed at `~/apps/blender-4.5.14` as a learning side track (OpenKotOR kotorblender 4.0.9 symlinked, `kb.mdlimport/mdlexport` smoke-tested). Plan: `.claude/plans/holstered_weapons.md`.

## Remaining unknowns (P0 probe gates)
G2 child Gob pointer location; G3 whether 0x7682B0 fires for NPCs; G4 `draw_weapon` consumer; G5 swap-hook model refresh on client; G6 case-insensitive bone lookup; G7 lightsaber attach origin vs grip center; plus node-output space (world vs local) at 0x5047b0.
