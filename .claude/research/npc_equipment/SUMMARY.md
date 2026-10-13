# NPC equipment scan (static)

Game dir: `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II` (read-only). Module containers scanned: 462 (symlink case-variants collapsed: 822). Override UTC: 72, chitin UTC: 284.

## Counts

- UTC total (occurrences): 5417 (unique resref: 2378)
- UTC with any equipped item: 4339
- UTC with a head item: 204 (unique resref: 122)
- Equipped entries with a headgear base item: 203
- UTC that failed to parse: 0
- UTI that failed to parse (base item unknown): 0
- Equipped entries that are HEAD slot with unresolved UTI: 1
- Blank Equip_ItemList entries skipped: 0
- Equipped entries by UTI source: MODULE=198, OVERRIDE=863, CHITIN=7447, ANY_MODULE=61, UNRESOLVED=306
- Equipped entries whose slot id is outside the EquipmentSlot values (0 = INVALID is inside): 0

## Slot id vs resolved base item (bit check; top 3 labels per slot)

- slot 0 (not a slot bit): 1 entries; Vibro_Blade 1
- slot 1 (HEAD): 204 entries; Mask 182, Droid_Targeting_Computers 21, UNRESOLVED 1
- slot 2 (ARMOR): 1577 entries; Basic_Clothing 432, Armor_Class_8 188, Jedi_Master_Robe 184
- slot 8 (GAUNTLET): 436 entries; Gauntlets 413, Droid_Computer_Spike_Mount_x 23
- slot 16 (RIGHT_HAND): 3147 entries; Lightsaber 532, Blaster_Pistol 401, Blaster_Rifle 336
- slot 32 (LEFT_HAND): 570 entries; Blaster_Pistol 217, Lightsaber 77, Disrupter_Pistol 70
- slot 128 (RIGHT_ARM): 102 entries; Droid_Utility_Device 68, Forearm_Bands 26, UNRESOLVED 8
- slot 256 (LEFT_ARM): 180 entries; Droid_Utility_Device 100, Forearm_Bands 60, UNRESOLVED 16
- slot 512 (IMPLANT): 374 entries; Implant_3 345, Droid_Computer_Spike_Mount_x 19, UNRESOLVED 8
- slot 1024 (BELT): 665 entries; Stealth_Unit 449, Belt 143, Droid_Shield 52
- slot 16384 (CLAW1): 301 entries; Creature_Item_Slash 289, UNRESOLVED 12
- slot 32768 (CLAW2): 281 entries; Creature_Item_Slash 269, UNRESOLVED 12
- slot 65536 (CLAW3): 163 entries; Creature_Item_Slash 100, Creature_Weapon_Sl_Prc 44, Creature_Item_Pierce 19
- slot 131072 (HIDE): 874 entries; Creature_Hide_Item 873, UNRESOLVED 1

## Top 10 headgear base items (equipped entries; UTC rows in brackets)

- Mask: 182 [182]
- Droid_Targeting_Computers: 21 [21]

## Appearance labels with >=1 head item (head UTC rows / total UTC rows)

- Alien_Gand_01: 42 / 54
- Droid_Assassin: 15 / 33
- P_MAL_C_LRG_03: 11 / 17
- Alien_Mandalorian_02: 9 / 40
- Alien_Trandoshan_01: 9 / 59
- P_MAL_C_LRG_04: 8 / 8
- Alien_Mandalorian_03: 7 / 24
- Commoner_Mal_Asian_01_A: 6 / 39
- P_MAL_C_LRG_02: 6 / 6
- P_MAL_C_LRG_01: 5 / 10
- Alien_Devaronian_01: 4 / 27
- Alien_Mandalorian_01: 4 / 69
- Alien_Trandoshan_02: 4 / 29
- Commoner_Mal_Asian_01_B: 4 / 24
- Commoner_Mal_Black_01_B: 4 / 26
- P_MAL_B_MED_02: 4 / 4
- P_MAL_C_MED_03: 4 / 15
- Alien_Gran_01: 2 / 42
- Commoner_Fem_Hispanic_01_B: 2 / 14
- Commoner_Fem_White_01_C: 2 / 11
- Commoner_Mal_Old_White_01_B: 2 / 8
- Commoner_Mal_White_01_B: 2 / 42
- DP_Republic_Admiral_Cede: 2 / 5
- DP_Republic_Doctor_Fem_B01: 2 / 19
- Droid_Construction_Medium: 2 / 16
- Onderon_Commoner_Mal_01: 2 / 11
- P_FEM_C_LRG_04: 2 / 7
- P_FEM_H_LRG_01: 2 / 6
- P_MAL_A_MED_02: 2 / 4
- P_MAL_A_MED_04: 2 / 2
- P_MAL_B_MED_05: 2 / 2
- P_MAL_C_SML_01: 2 / 5
- P_MAL_C_SML_03: 2 / 2
- P_MAL_H_MED_02: 2 / 2
- Twilek_Female_03_C: 2 / 18
- Unique_Dopak: 2 / 4
- Unique_Lala: 2 / 6
- Unique_Zaerdra: 2 / 3
- Alien_Quarren_01: 1 / 21
- Czerka_Commander: 1 / 6
- Droid_War_01: 1 / 13
- Droid_War_02: 1 / 16
- Droid_War_03: 1 / 2
- Droid_War_04: 1 / 6
- P_FEM_C_LRG_01: 1 / 6
- P_FEM_C_MED_01: 1 / 1
- P_FEM_H_LRG_02: 1 / 1
- P_MAL_B_LRG_05: 1 / 1
- P_MAL_C_MED_01: 1 / 3
- P_MAL_C_SML_05: 1 / 1
- P_MAL_H_LRG_02: 1 / 2
- Twilek_Commoner_M_01_F: 1 / 10
- Twilek_Female_02_B: 1 / 19
- Unique_Droidmaster: 1 / 1

## Appearance labels with 0 equipped items across >=5 UTC rows (label only, no conclusion)

- Alien_Ithorian_01: 20 UTC rows
- Alien_Ithorian_02: 20 UTC rows
- Alien_Jawa: 18 UTC rows
- LiteL_Alien01: 9 UTC rows
- Republic_Soldier_Fem_w_Helmet: 9 UTC rows
- Unique_Opo_Chano: 9 UTC rows
- Republic_Officer_Mal_White: 8 UTC rows
- Alien_Ithorian_03: 7 UTC rows
- Republic_Captain_Fem_White: 7 UTC rows
- Republic_Captain_Mal_White_01: 7 UTC rows
- Republic_Captain_Mal_White_02: 7 UTC rows
- Republic_Officer_Fem_White: 7 UTC rows
- Onderon_Commoner_Mal_05: 6 UTC rows
- Commoner_Male_Fat: 5 UTC rows

## Headgear base item types (baseitems.2da rows with HEAD bit set)

Count: 5

- 44: Mask (equipableslots 0x00001)
- 69: Droid_Search_Scope_x (equipableslots 0x00001)
- 70: Droid_Motion_Sensors_x (equipableslots 0x00001)
- 71: Droid_Sonic_Sensors_x (equipableslots 0x00001)
- 72: Droid_Targeting_Computers (equipableslots 0x00001)

## Notes

- baseitems.2da source: `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/Override/baseitems.2da` (override wins over chitin).
- appearance.2da source: `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/Override/appearance.2da`.
- Slot bits (pykotor EquipmentSlot, same bits in Equip_ItemList struct_id and baseitems equipableslots): HEAD=0x1, ARMOR=0x2, GAUNTLET=0x8, RIGHT_HAND=0x10, LEFT_HAND=0x20, RIGHT_ARM=0x80, LEFT_ARM=0x100, IMPLANT=0x200, BELT=0x400, CLAW1=0x4000, CLAW2=0x8000, CLAW3=0x10000, HIDE=0x20000, RIGHT_HAND_2=0x40000, LEFT_HAND_2=0x80000.
- is_headgear = equipableslots has the HEAD bit (0x1). Cross-check: the slot bit check section shows which base items the game actually equips under each slot id.
- UTI lookup order: same module stem, then override, then chitin, then any module (first match by sorted container). UNRESOLVED = no UTI found in any of these.
- Race is the raw UTC Race id (racialtypes), not labelled here. Appearance label = appearance.2da label, or rowN when the label cell is blank.
- appearance.2da head/bodyvariant columns absent from the file: none.
- baseitems.2da optional columns absent from the file: none.
- Counts are per UTC occurrence (a template in two containers counts twice). Unique resref counts are given separately.
- Appearance-label 'cannot equip' categories are label-only observations; no conclusion is drawn from data.
