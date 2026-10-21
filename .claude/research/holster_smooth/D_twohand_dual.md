# D-claims: two-handed and dual-wield draws (holster mod, doc 42)

Scope: read-only recon. Code at `patch_manager_mods/holster_src/`, data at `research/holstered_weapons/wave2_3.md:686-724` (R6a baseitems rows, read via PyKotor).

| # | Claim | Verdict | Anchor | Note |
|---|---|---|---|---|
| D1 | 15 two-handed, 15 dual-eligible | PARTIAL | doc42:52-61; wave2_3.md:687-724 | "Two-handed" 15 = right-hand-only (slot 0x10) set: 7 ww3 (staff, dbl blades, pike, gaffi, warblade, dbl saber) + 6 rifles + 2 repeaters. Dual = 14 verified (2 sabers, 6 melee, 6 pistols); 3 creature stems carry 0x30 but are unverified. "15 dual" is FALSE. |
| D2 | Only the holding hand moves; no off-hand support | TRUE | Holster.cpp:1498, 1717, 1756; 1235; 1623 | `reachArm` has 3 call sites, all with the Gob's own `left`. Entries keyed by (parent, left). Left arm moves only if a `lhand` Gob exists. No support reach exists. |
| D3 | Right-hand HolsterPlace can also write the left arm | PARTIAL (feasible, not built) | Holster.cpp:1211-1218, 1247-1249, 1279-1283 | Bone lookup is name-based and side-agnostic; left names are already in the tables. Caches are per (parent, side), so no collision. Blockers: the target pose is the right weapon's `out` (1297); undo/persist lives only in the owning Gob's path (1281, 1717). |
| D4 | Dual draws start together, no stagger or extra clearance | TRUE | Holster.cpp:1683-1690; HolsterSlide.h:51,77-78 | Each Gob has its own `Slide` from the same `g_drawSlideMs` and gate flip. Clock is read per Gob (up to one frame apart, R2_review.md:83). No stagger or clearance term. Left differs only by tilt and x-mirror (HolsterTune.h:95, 279-292; HolsterReach.h:148-154; Holster.cpp:1313). |
| D5 | A runtime two-handed flag exists | FALSE | Holster.cpp:1453, 1609, 1862; HolsterLogic.h:86-96 | Every DLL `classify` passes `weaponWield=0`. `isLong` has no DLL caller (test only). Source of truth is baseitems `weaponwield` (3 = two-handed) and `equipableslots` (0x10 right only, 0x30 either). The DLL reads neither. |

## Detail

- **D1 counts.** Stems in `kStems` (HolsterLogic.h:59-71) total 32. Slot 0x10 (right only): Qtrstaff, Dblswrd, VbrDblswd, Dblsbr, gaffi, warblade, ForcePike, 6 rifles, 2 repeaters = 15. Slot 0x30 (either hand): 2 short/single sabers, 6 melee, 6 pistols = 14. Creature 0x1C030: 3. Total 32. The doc's rifles and repeaters being "two-handed" is an inference: the data shows right-hand-only, not two-handed grips.
- **D2 detail.** `ent->left` comes from the node name (`handFromNode` in HolsterDrawGate.h:74, used at Holster.cpp:1623). A right-only weapon attaches to `rhand` and never reaches the left-arm path.
- **D3 detail.** `reachFind` (1211-1218) checks vtable slot and owner, not side. `kBic`/`kFore`/`kHand` (1247-1249) already list `lbicep_g`, `Lforearm_g`, `lhand`. Persist/undo (1279-1283) runs only when the entry's own Gob calls with w==0 (1717).
- **D4 detail.** No `stagger`/`lateral`/`clearance` tokens in Holster.cpp, HolsterSlide.h, HolsterDelay.h, HolsterReach.h. Slide starts: `reset` at 1685 (first or unreadable), `target` at 1690 (gate flip). `t0` is set at HolsterSlide.h:51 and :78.
- **D5 detail.** `handAttach` gets `item`, reads `item+0x14` as W (Holster.cpp:1003), and logs it only. Meaning unknown. The engine's per-creature `[base+0xE8]` (hr1/R3_review.md:28,46) is a lead for dual state, not verified.

## Gaps to build

1. Off-hand target: a support-grip pose on the weapon shaft, separate from `out`. Touches `HolsterReach.h` (new helper) and `Holster.cpp` near 1756.
2. Cross-side call: from a two-hand Gob's HolsterPlace, call `reachArm(..., left=true, ...)`. Needs its own undo on slide end and on Gob removal. Touches `Holster.cpp` near 1717 and 1756.
3. Runtime two-hand flag: either read item to baseitem row (RE of the item object, `Holster.cpp:991-1005`), or derive it from "right-only Gob, no lhand Gob for this owner" using the existing hand record (`Holster.cpp:1623`, `HolsterDrawGate.h:74`).
4. Wire data: add a per-stem ww/slot or `twoHanded` field to `kStems` (`HolsterLogic.h:59-71`) and call `isLong` from `Holster.cpp` (currently never called). Test in `holster_logic_test.cpp`.
5. Doc fix: reword `42_holster_mounts_and_weapon_types.md:61` to "right-hand only (slot 0x10): 15" and "dual: 14 verified + 3 creature". Touches `patch_manager_mods/42_holster_mounts_and_weapon_types.md`.

OPEN: Does the engine actually attach two-handed weapons only to `rhand` (slot-bit rule, unverified live), and what does `item+0x14` hold?
