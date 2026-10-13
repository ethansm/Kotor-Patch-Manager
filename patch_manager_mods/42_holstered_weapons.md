# 42 — Holstered weapons (hip/back hardpoints) — research

Status: RESEARCH ONLY (2026-10-13). Nothing built. Wave 1 = 10 Haiku web recon agents, all PARTIAL.
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

## Plan
See `.claude/plans/holster_research_wave2.md` (wave 2 research plan).
