# M0 - save layout: where KOTOR II loads party creatures from (2026-10-03, read-only)

Method: copies of `000003 - Game2` (Oct 2 22:25) and `000000 - QUICKSAVE` (Oct 3 01:18) in the scratchpad `m0/`; originals untouched (md5 of copy == original verified).
Scripts: `research/companion_levelup/phase6_evidence/M0_scripts/` (`scan.py` all slots, `utcs.py` creature summary, `diff.py` copy diff, `guid.py` GUID decode, `roundtrip.py`, `nested_edit.py`).
All claims below are from files on disk unless marked ENGINE-INFERRED.

## 1. Archive layout (SAVEGAME.sav = outer ERF, header `MOD V1.0`)

Game2 resources: `003ebo.sav, 201tel.sav, 202tel.sav, 203tel.sav` (one nested ERF per VISITED module, restype SAV), `availnpc0/6/7/8.utc`, `inventory.res`, `pc.utc`, `repute.fac`.
QUICKSAVE: `101per..106per.sav, 151har..153har.sav`, same loose resources. Only NPC slots that were ever filled have an `availnpc<n>.utc` (n = npc.2da row: 0 Atton, 6 Kreia, 7 = 3C-FD placeholder/Mira slot, 8 T3-M4).
Nested module `.sav` = ERF with exactly 3 resources: `<mod>.git`, `<mod>.are`, `module.ifo`.

## 2. Which resource holds what

| Character | Resource (live data) | Notes |
|---|---|---|
| **PC** | `module.ifo` of the nested module `.sav` -> `Mod_PlayerList[0]` (133-field creature struct). Use the module named by `savenfo.LASTMODULE` (`203TEL` in Game2, `106PER` in QS). | Every cached module's ifo carries a PC snapshot from the time it was left (Game2: 003ebo XP 33272, 201tel 34302, 203tel 34534 Jedi Guardian-class 5 lvl 8). Only the LASTMODULE copy is the live one. |
| PC stale | `pc.utc` in SAVEGAME.sav | **Stale/template**: XP 1830, class 5 level 2, byte-identical (size 16993) in both saves. Not the live PC. Has `LvlStatList` (PC only; Mod_PlayerList copy has 8 entries, `LvlStatList` entry fields: LvlStatHitDie, LvlStatForce, LvlStatClass, SkillPoints, SkillList, FeatList). Do not rely on it; edit the ifo copy (and optionally keep pc.utc consistent). |
| **ACTIVE party member** (listed in PARTYTABLE `PT_MEMBERS`) | `availnpc<n>.utc` in SAVEGAME.sav ONLY. | QUICKSAVE (members 0 and 8): LASTMODULE git has 1 creature (`3cfdbroke`); Atton / T3-M4 appear in NO nested git or ifo. So the load path can only build them from AVAILNPC (ENGINE-INFERRED, see section 6). |
| **INACTIVE / standing NPC in an area** (Game2: Atton and Kreia, `PT_NUM_MEMBERS` = 0) | BOTH `availnpc<n>.utc` and a `Creature List` entry in the LASTMODULE `<mod>.git` (`Tag` atton / kreia). | Game2 203tel.git: Atton XP 30014 Scoundrel(2) 3, Kreia XP 30014 Consular(4) 6, identical to AVAILNPC. |
| Unrecruited / never-filled NPC | No `availnpc` file, or placeholder (`availnpc7` Tag `3cfd`, class 7 lvl 3, XP 3380; do not match on it) | |

Copy diff (Game2 Atton and Kreia: AVAILNPC vs 203tel.git creature, flattened over all ~320 fields): **identical except `ObjectId` (git only; AVAILNPC has none)**. (The `LocalizedString` object repr differences in my output are Python object identities, not data.) So in a snapshot written by one save both copies agree; the data cannot say which one the engine prefers when both exist. Forge rule: when both exist, edit both identically.
No slot on disk has a member that is in PT_MEMBERS AND in the git, so "prefers git vs avail" cannot be proven from files. Cheap live test if needed: forge one slot with the git copy changed only.

### UTC field names (same in AVAILNPC, git creature entry, Mod_PlayerList)
Top level: `Tag, Experience (UInt32), ClassList, FeatList, SkillList, Str Dex Con Int Wis Cha (UInt8), HitPoints/CurrentHitPoints/MaxHitPoints (Int16), ForcePoints/CurrentForce (Int16), BonusForcePoints (Int32), PlayerCreated, Equip_ItemList, ItemList, IsPC, PortraitId, Appearance_Type, GoodEvil, willbonus/fortbonus/refbonus, TemplateResRef` (git/ifo copies), `FirstName, LastName`.
- Class: `ClassList[i] = {Class Int32, ClassLevel Int16, SpellsPerDayList [], KnownList0 [ {Spell UInt16} ] }` (powers are in `KnownList0` of the class that knows them).
- Feats: `FeatList[j] = {Feat UInt16}`.
- Skills: `SkillList[k] = {Rank UInt8}`, 8 entries, index = skills.2da row.
- `LvlStatList` exists for the PC only (0 for companions).
- Duplicate fields: the engine writes `PlayerCreated`, `BonusForcePoints`, `AssignedPup` TWICE (equal values) in AVAILNPC structs; pykotor collapses them (see section 5).
- Snapshot data (Game2): Atton (2,3) XP 30014 HP 18/21 FP 3/0 13 feats 0 powers; Kreia (4,6) XP 30014, 15 feats, 10 powers; T3-M4 (7,7) XP 27828; 3C-FD placeholder XP 3380; PC (live, 203tel ifo) class 5 lvl 8 XP 34534 25 feats 8 powers. QS: Atton XP 24140, Kreia 18016, T3 23948, PC (106per ifo) XP 26922 lvl 7.

## 3. PARTYTABLE.res (GFF, content `PT `)
Top: `PT_PCNAME, PT_GOLD, PT_ITEM_COMPONEN, PT_ITEM_CHEMICAL, PT_SWOOP1-3, PT_XP_POOL (Game2 37510, QS 29935), PT_PLAYEDSECONDS, PT_CONTROLLED_NP (-1 = PC), PT_SOLOMODE, PT_CHEAT_USED, PT_NUM_MEMBERS, PT_MEMBERS [ {PT_MEMBER_ID Int32 = npc.2da index, PT_IS_LEADER UInt8} ], PT_NUM_PUPPETS, PT_PUPPETS, PT_AVAIL_PUPS [3 x {PT_PUP_AVAIL, PT_PUP_SELECT}], PT_AVAIL_NPCS [12 x {PT_NPC_AVAIL, PT_NPC_SELECT}], PT_INFLUENCE [12 x {PT_NPC_INFLUENCE}], PT_AISTATE, PT_FOLLOWSTATE, GlxyMap{GlxyMapNumPnts, GlxyMapPlntMsk, GlxyMapSelPnt}, PT_PAZAAKCARDS[24], PT_PAZSIDELIST[10], PT_TUT_WND_SHOWN, PT_LAST_GUI_PNL, PT_FB_MSG_LIST[64], PT_DLG_MSG_LIST[64], journal (JNL_Entries), ...`
- The PC is NOT in `PT_MEMBERS` (it lists NPCs only, up to 2). QS and Game9: `[(0, leader 0), (8, leader 1)]` = Atton + T3-M4. Game2/Autosave/Game1..8: empty.
- `PT_XP_POOL` is the party pool (the level-up mod's "pool" is separate from per-creature `Experience`).
- Game2 `PT_AVAIL_NPCS` avail = indices 0, 6, 8 (+ select flags); 7 avail 0 although a utc exists.
- Open: QS marks member 8 (T3-M4) as `PT_IS_LEADER=1` while `PT_CONTROLLED_NP=-1`; semantics of leader flag vs PC not resolved.

## 4. savenfo.res (GFF `NFO `)
Game2: `AREANAME 'Citadel Station - Residential 082 East'` (shown in load menu), `LASTMODULE '203TEL'` (picks which nested module is live), `TIMEPLAYED 45916` (seconds), `CHEATUSED, SAVEGAMENAME '1'` (QS: `QUICKSAVE`), `TIMESTAMP 134354535169810000` (FILETIME), `PCNAME 'Meetra Surik'`, `SAVENUMBER 3`, `GAMEPLAYHINT, STORYHINT0-9, LIVE1-6 (strings), LIVECONTENT, PORTRAIT0 (ResRef po_pfhc06)`. No unique playthrough id. Screen.tga = thumbnail, `force_power_hotbar.ini` = mod sidecar.

## 5. GLOBALVARS.res (GFF `GVT `)
Fields: `CatBoolean [ {Name} ] (Game2 176, QS 208)`, `CatNumber [973]`, `CatLocation [3]`, `CatString [3]`, `ValBoolean (Binary)`, `ValNumber (Binary, 1 byte each = 973)`, `ValLocation (Binary, 2400 = 50 x 48)`, `ValString [ {String} ]`.
- `ValBoolean`: bit for catalogue index i is **byte i//8, bit (7 - i%8) = MSB-FIRST**. Length = n//8 + 1 (176 -> 23, 208 -> 27; 165 -> 21).
- **pykotor `GlobalVars` (savedata.py:900-909, 1075-1084) is LSB-first and WRONG for this engine** (writing booleans through it would scramble every flag). Forge must edit ValBoolean directly.
- `CatBoolean` order is NOT insertion order; the LUO names sit at catalogue positions (QS) G00=185, G01=176, G02=138, G03=3, ...: always look up by name.
- Engine adds unknown names on load (doc 08 Item 7, lines 514-520), so a forged save may add `LUO_G*` names to CatBoolean if values stay aligned with the Cat order.

### GUID decode (`M0_scripts/guid.py`)
GUID = 32 booleans, `LUO_G<ii>` = bit i of the uint32 (C++: `Playthrough.h:22-25` `bitName`, `unpackBits` `(guid >> i) & 1`; `LevelUpOverride.cpp:941-` writeGuid writes via engine Set by name, so storage order is the engine's: msb-first). Python: v |= bit(LUO_Gii) << ii with bit() msb-first.
| Slot | LUO_G in Cat? | GUID |
|---|---|---|
| 000000 QUICKSAVE (Oct 3 01:18) | yes, 32 | **gfb7bdbf7** (MSB-first). LSB-first would give g50040000, i.e. wrong; msb confirmed by exact match to the expected value and doc 08 line 902. |
| 000003 Game2 | **absent** (0 of 32; 176 booleans, same for AUTOSAVE, Game1-Game9 except QS) | none (pre-Apply lineage, matches doc 08 line ~841: "found=0/32 -> unconfigured") |
So `gfb7bdbf7` is the QUICKSAVE lineage; Game2 has no GUID. Game9 (Oct 3?) also none.

## 6. Engine-side evidence (static only; ghidra_knowledge DB name_map / lessons)
- Save: `CSWPartyTable::Save 0x5FD440` -> UpdateInventory 0x5F8710, UpdateMembers 0x5F7EC0 (-> SaveMember 0x5F7F30 / SavePuppet), SaveTableInfo 0x5FB1A0. `SaveMemberByObject 0x5F8130` calls `GetFilename 0x5F9100` which formats `AVAILNPC%d` (str @0x996EBC; puppet twin `AVAILPUP%d`). So AVAILNPC is written from the live creature at every save (explains slot count 4/12 filled).
- Load: `LoadGame 0x52FB80` (SAVES:%06d - %s) -> `CSWPartyTable::LoadTableInfo 0x5FBF60` (PARTYTABLE) -> GLOBALVARS load 0x656710 (post-load hook 0x65675B) -> LoadModule 0x531C50. `CreateParty 0x5F9220` checks `GetLoadFromSaveGame 0x51CDE0`, then per `tbl[2+i]` calls `SpawnNPC 0x5FD480` (-> `GetNPCObject 0x5FAAF0`). Hook sites 0x656640 / 0x65675B touch only globals.
- NOT proven statically: the decompile of the code that reads `AVAILNPC%d` on load (GetFilename readers other than SaveMemberByObject), nor whether module load replaces an area creature from AVAILNPC. Evidence for "active members come from AVAILNPC" is the on-disk fact above (party members absent from the git/ifo in QS and Game9). The knowledge DB / doc 08 contain no note that module load overrides party stats from AVAILNPC. Recommend a one-line Ghidra decompile of callers of 0x5F9100 (only Save/Load wrappers) if a definitive answer is wanted.
- `SaveCreature` sets non-party creatures to AI level 2 on save (lessons #321), irrelevant to stats.

## 7. pykotor round-trip safety (venv pykotor 2.3.12; do not modify)
- **Patch #6 is APPLIED in the venv**: `.venv/lib/python3.12/site-packages/pykotor/extract/capsule.py:271-279` (`_add_to` skips the old copy, adds new data last; identical to `pykotor_patches/README.md` section 6). Test: `LazyCapsule.add` replace of `availnpc0.utc` in a copied SAVEGAME.sav survived (XP 99999 re-read; 11 resources kept).
- Outer ERF SAVEGAME.sav (4,361,804 B): read_erf -> bytes_erf: same size, same key/resource tables, **all 11 resources byte-equal on re-read; file differs in exactly 4 bytes, header offset 20 (OffsetToLocalizedStrings 160 -> 0)**; header otherwise preserved (build year 126 / day 274, counts, offsets). Same result for all 4 nested module `.sav` files (same size, per-resource equal, 4-byte header diff).
- GFF read -> write: GLOBALVARS.res, PARTYTABLE.res, repute.fac same size, 2nd pass stable; `savenfo.res` byte-IDENTICAL. `availnpc*.utc`, `pc.utc`, `inventory.res` shrink by 48 / 8 bytes: pykotor drops the engine's duplicate fields (`PlayerCreated`, `BonusForcePoints`, `AssignedPup`; duplicates were equal-valued in availnpc0), semantic data is unchanged and a second round trip is stable. Engine tolerance of a deduplicated GFF is not proven (low risk).
- Edit test: set `Experience` = 99999 on Atton in `203tel.git` Creature List (nested) + AVAILNPC0, rewrite nested ERF -> outer ERF -> file -> re-read: both values 99999, all other outer and nested resources byte-equal. Output only in the scratchpad.
- **Verdict**: pykotor ERF/GFF read-write is safe for forging with patch #6 present. Recommended writer: `read_erf` / `set_data` / `bytes_erf(erf, ResourceType.SAV)` / `write` the whole file to a NEW slot (not LazyCapsule per resource, which re-reads/rewrites the 4-5 MB archive for each edit). For GLOBALVARS use direct GFF edit of `ValBoolean` (msb-first) and never pykotor `GlobalVars` / `SaveFolderEntry.save()` (LSB-first boolean packing, `PT_NUM_MEMBERS` recomputation, other re-serialisation risks untested). Always diff per-resource bytes after write.

## 8. Open questions
1. Does the engine, for a member that is both in `PT_MEMBERS` and in the git, use the git creature or AVAILNPC? (No such slot exists; needs a forged single-copy live test or a Ghidra look at callers of 0x5F9100 / the loader.)
2. Meaning of `PT_IS_LEADER` when the PC is on foot (QS: T3-M4 leader=1, controlled=-1).
3. Is `Mod_PlayerList` only read from the LASTMODULE ifo on load (stale PC copies in other cached modules: forge them too to be safe?). `pc.utc` use on load unknown.
4. Is a GFF without the duplicate fields accepted by the engine (expected yes)? Does a forged `ValBoolean` with added LUO names load (engine adds unknown names on load per doc 08)?
5. Adding a class to a creature by save edit stays "engine-validity unproven" until loaded once.
