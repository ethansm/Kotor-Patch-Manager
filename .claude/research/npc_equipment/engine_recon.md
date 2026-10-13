# engine_recon: NPC equipment rules (swkotor2.exe, Steam build)

Status: PARTIAL (10-13). The rea-recon run decompiled the Steam exe but could not write its own file; this file is the orchestrator's transcription of its reply (summary only, no per-question detail survived). Re-run with a Read tool granted if the detail is needed. Addresses are Steam-build.

## Confirmed (rea-recon, Steam exe `swkotor2.exe.unpatched_vanilla`)
- Address-map correction: the earlier brief's `CanEquipWeapon 0x00405100` and `CanEquipWeapon2 0x00670c90` are WRONG (a string helper and a feet/yards helper). The real weapon, armour and miscellaneous branches are `0x005bd1d0`, `0x005bd3e0`, `0x005bd610`, each called only by `CSWSCreature::CanEquipItem 0x005bd6a0`. Fix `gog_to_steam_fn_map.tsv`/knowledge rows if they carry those names (not touched here).
- **CanEquipItem order**: gate A (message 0x62); `item+0x2c8` bit 6 (silent refuse); baseitems `EquipableSlots` and proficiency (silent refuse); item properties 0x2b, 0x2c, 0x2d/0x3d, 0x39, 0x3c, 0x3e, 0x45; the "baodur" list (message 0xeb). Return byte: 0 = refuse, nonzero = allowed (2 = slot occupied or conflict).
- **Spawn path**: `0x00573950` reads the UTC GFF `Equip_ItemList` and calls CanEquipItem with args (1,1,1), so template equipment IS subject to the checks.
- **Bypass**: inventory load and `CSWCCreature::EquipToInventorySlot 0x0076e130` do NOT call CanEquipItem.
- **Head attach**: `0x0085b703` uses node `"GoggleHook"` (consistent with doc 10).

## Not found / open
- Hair-hide field and any droid/animal hard block: not found in the decompilation.
- 8 of 9 CanEquipItem callers and all 4 EquipToInventorySlot callers unidentified.
- Slot-bit names, the 2DA/GFF field names behind the item-property checks, `HideEquippedItems 0x00773a10` target, and an `ActionEquipItem` string (absent) remain open.

## Combined with the static scan (`SUMMARY.md`, same folder)
- 204 of 5,417 UTC templates equip a HEAD-slot item (base item `Mask` 182, droid targeting computer 21, 1 unresolved); appearances with the most: Alien_Gand_01 (42/54), Droid_Assassin (15/33), P_MAL_C_LRG_03, Alien_Mandalorian_02.
- Implication: giving an NPC a helmet at spawn through the UTC equip list (or an equivalent hook call) passes CanEquipItem; granting it through `EquipToInventorySlot` bypasses the checks, so restrictions must be enforced by our own allow-list (base items with HEAD in EquipableSlots, appearances that have a head part).
