# H1: creature weapons and CatUnknown (recon, read-only)

Claim: w_crslash/w_crpierce/w_crbite are melee, go to the upper back via the melee key, should never holster or reach; CatUnknown should stay in hand.

## Verdict per sub-claim
1. Melee category: CONFIRMED. `patch_manager_mods/holster_src/HolsterLogic.h:65` maps all three stems to `CatMelee`. The stem table is checked before the weaponwield fallback (`classify`, HolsterLogic.h:86-89).
2. Upper back via melee key: NOT SUPPORTED. `isLong()` (HolsterLogic.h:94) is true for melee only when weaponWield == 3. Live callers pass 0: `Holster.cpp:1609` (`hs::classify(mname, 0)`) and `Holster.cpp:1453`. `isLong`, `preferredZone`, `chooseMount` have no caller outside HolsterLogic.h in non-test sources.
3. Never holster or reach: NOT ENFORCED. No w_cr* or CatUnknown exclusion in the Holster.cpp entry build (1598-1622) or mount/reach path (1726-1756). CatUnknown hits are loop bounds and init only (Holster.cpp:236, 620, 1573).
4. CatUnknown stays in hand: NOT ENFORCED. HolsterLogic.h:75 says unknown is "never holstered", but Holster.cpp:1605 says an unknown stem gets global place_rot/place_off, so it is placed. The only unknown gate is in the preview (HolsterPreview.h:340).
5. Creature weapon Gob visible: NO EVIDENCE. `.claude/research/k1_items/baseitems_k2.tsv:81-83` gives equip slots RIGHT_HAND|LEFT_HAND|CLAW1-3, itemclass iw_creature. `patch_manager_mods/42_holster_mounts_and_weapon_types.md:61` says their dual flag is unverified. No doc shows a rendered Gob.
6. Claim source: `patch_manager_mods/42_holster_mounts_and_weapon_types_Claude.md:35`.

## Minimal fix (not applied)
- `patch_manager_mods/holster_src/HolsterLogic.h`: add `inline bool holsterable(Category c, const char* stem)` that returns false for CatUnknown and for stems starting `w_cr`. Keep the table rows.
- `patch_manager_mods/holster_src/Holster.cpp`, entry build near line 1609: when `!holsterable`, skip mount and reach so the weapon stays in the native hand.

## Open
- Can a party member ever hold a w_cr* weapon? The CLAW slots suggest natural creature weapons, so this may be moot for PCs.
