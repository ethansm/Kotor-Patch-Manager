# D: anim resolver 0x00863650 - vtable slots, callers, row consumers (static, $0)

Verdict: PARTIAL. Vtables, slot, and consumers confirmed statically. Animations 2DA row data and the gate-1 flag source are not in the dump.

## Q1 Vtables and call sites
- All 7 data refs sit at slot 0xE0 (index 56) of CSWCAnimBase-family vtables. vtable start = ref - 0xE0. Names from RTTI COL (x86, sig 0):
  - 0x9a462c -> vt 0x9a454c CSWCAnimBase (COL off 0)
  - 0x9a4954 -> vt 0x9a4874 CSWCAnimBaseHeadWield (COL off 8)
  - 0x9a4a74 -> vt 0x9a4994 CSWCAnimBaseWield (COL off 92)
  - 0x9a4bb4 -> vt 0x9a4ad4 CSWCAnimBaseHead (COL off 80)
  - 0x9a4cbc -> vt 0x9a4bdc CSWCAnimBaseTW (COL off 0)
  - 0x9a982c -> vt 0x9a974c CSWCAnimBaseVisualEffect (COL off 0)
  - 0x9aa11c -> vt 0x9aa03c CSWCAnimBaseProjectile (COL off 0)
- COL off != 0 rows (HeadWield/Wield/Head) are secondary MI vtables; name = COL type (medium confidence on the name mapping).
- NOT resolver: CSWCAnimBasePlaceable (vt 0x9a940c, slot 0xE0 = FUN_008f4840), Door (vt 0x9a985c, FUN_008f8d50), Trigger (vt 0x9a99c4, FUN_008f91f0).
- Slot 0x40 (name getter): CSWCAnimBase vt -> FUN_00860700 (inherited by HeadWield, Wield, Head, TW, Projectile). Overridden by Placeable FUN_008f41d0, Door FUN_008f89a0, Trigger FUN_008f9120, VisualEffect FUN_008f8640.
- Slot-0xE0 call sites, classified by asm/decomp arg count and receiver:
  - Resolver, receiver = this+0x68 (anim base): CSWCObject::Animation* at 007ef2f2 (Stationary), 007ef382 (Pause), 007ef412 (Walking), 007ef4a2 (Running), 007ef576 (Looping), 007ef650 (FireAndForget), 007ef6d4 (Overlay), 007ef822 (Parry), 007ef8b2 (Attack), 007ef942 (HideEquippedItems). Asm: `MOV EDX,[EAX]; MOV EAX,[EDX+0xe0]; CALL EAX` with ECX=[this+0x68].
  - Resolver, receiver = this: FUN_00860700 sites 00860742 (arg = id) and 008609dc (arg = 0x2710).
  - Not resolver, 2 args: 0066bb0a, 0068f536, 0077fe06, 0079f66b (push 1 + id).
  - Not resolver, 0 args: 0076d217 (SetAppearance), 00784670, 0085e6b9/0085e711/0085e787/0085e804/0085e881, 00806569, 0046a6c5, 00873545.
  - Not resolver, Placeable/Door/Trigger name getters 008f4212, 008f89e2, 008f9162 (receiver's own 0xE0).

## Q2 What the caller does with the u16 (row index)
- Yes: the u16 is a ROW index into the "Animations" 2DA. Loader FUN_006eb670 calls `FUN_00710840("Animations")` and stores the table at `*(obj+0xcc)`, obj = CSWRules(DAT_00a1b4d0)+0x130.
- Column globals set in FUN_006eb670 (lines 75-225): Name=DAT_00a0fe44; Stationary=a0fe28; Pause=a0fe2c; Walking=a0fe30; Running=a0fe34; Looping=a0fe24; FireForget=a0fe1c; Overlay=a0fe20; Parry=a0fe3c; Attack=a0fe4c; HideEquippedItems=a0fe50; Damage=a0fe38; Dialog=a0fe40; Dodge=a0fe48.
- Animation* predicates: `FUN_0071b8b0(row, flagcol, &out)`; return is `out != 0` (007ef3e0 end: `return iVar1 != 0 && local_8 != 0`).
- Name getter FUN_00860700: `FUN_0071afa0(row, DAT_00a0fe44, &name)`. If the name is empty and id != 0x2772, it retries with `vfn[0xE0](10000)`. Literal "pause1" (strings 0x99f35c) is used in 5 functions as a name, row unverified.
- Raw 0x2711 with wield 0: the wield table has no case, so the row stays 10001. FUN_0071b8b0 and FUN_0071afa0 reject `row >= *(tbl+0x24)` and return the default cell. Whether the 2DA has >=10002 rows is not in the dump (OPEN).
- Wield 0 is NOT unarmed. FUN_0076d590 (UpdateWeaponClass) sets `local_9 = 8` when both weapon slots (0x10, 0x20) hold 0x7f000000 (empty). Wield 8 is unarmed and has its own idle row: case 8 gives 0xf9, or 0x217 when creature+0xD4 != 0. Wield 0 = right empty, left item present (local_9 starts at 0).
- Gate creature+0xC6: ==1 selects the wield table. Otherwise (00863650.c 706-725) the constant table gives 0x2711/0x27a4 -> 0x116 with no wield dependence. The gate is written by FUN_0076d160 (1 or 2) from FUN_0085a1f0 (lines 97/105). Which appearance flag drives it is OPEN.

## Q3 Idle / walk / run ids
- Idle: 0x2711 (alias 0x27a4). Wield table rows: 1->0x5c, 2->0x85, 3->0xae, 4->0xd7, 5->0xdf, 6->0xed, 7->0xf5, 8->0xf9/0x217, 9->0xf5. Constant table: 0x116.
- Generic pause row: 0x2710 (alias 0x27a5, 0x276c). Wield table: row 6. Constant table: 0x100.
- Walk: 0x2712 (alias 0x2713). Set in CSWCCreature::DriveAnimationUpdate (00776fe0 decomp line 158, `local_14 = 0x2712`). Speed divisor = DriveAnimWalk (appearance+0x60, column DAT_00a0fdb0).
- Run: 0x2714. Set at 00776fe0 decomp lines 107-108 and 132. Taken when `fVar1 * k < speed`, where fVar1 = appearance+0x64 = DriveAnimRun_PC (DAT_00a0fdb4). Walk/run naming is medium confidence (from column names + branch).
- Resolver rows for walk/run: wield table 0x2712/0x2713 -> row 0 default, 0x152/0x153/0x155/0x154 by wield and +0xe8. 0x2714 -> row 2 default, 0x159/0x157/0x156/3 by wield. Constant table: 0x2712/0x2713 -> 0xfd; 0x2714 -> 0xff if +0xcc==0, else 0xfd.
- Filter scope: touch only 0x2711, 0x27a4, 0x2712, 0x2713, 0x2714. Leave 0x2710 family untouched. Not analysed: 0x2795 alt, 0x276d/0x276e.

## Open
- Animations 2DA row count and the Name/flag values for rows 6, 0x116, 0x0f9, 0x2, 0x0 (need the 2DA file).
- What sets creature+0xC6 to 1 vs 2 (FUN_0085a1f0 local_94 at line 80).
- Walk vs run confirmation: need Walking/Running flag values for rows 0, 2, 0xfd, 0xff.

## Evidence anchors
- 0x9a462c..0x9aa11c (slots); 0x9a454c, 0x9a4874, 0x9a4994, 0x9a4ad4, 0x9a4bdc, 0x9a974c, 0x9aa03c (vtable starts)
- 007ef40c MOV EAX,[EDX+0xe0]; 007ef412 CALL EAX
- 00860742 / 008609dc (FUN_00860700 slot 0xE0)
- 006eb670.c lines 75-225 (column names and globals)
- 00776fe0.c lines 107-158 (0x2714 / 0x2712)
- 0076d590.c `local_9 = 8` (both slots empty)
- 00863650.c lines 706-725 (constant table 0x2711 -> 0x116)
