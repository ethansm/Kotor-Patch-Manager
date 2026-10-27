# C4b: Force point cost and regeneration (Steam swkotor2.exe)

Anchors: `D` = `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp`. Stats struct = creature `+0x1198`.

| stat | what it means for the player | formula / parts | computed at | storage | shown in vanilla UI? | how a patch could read it | confidence |
|---|---|---|---|---|---|---|---|
| Max FP | cap for current FP | Branch A: `stats+0x12e + (char)stats+0xf6 * FPlevel`; Branch B: per-level sum of class byte `+0x2d`, min 1 each; both add +40 if feat 0x74, +`creature+0x11f4`, +50 if class-gated feat (`DAT_009960cc`) | 0x0057eca0 (CSWSCreature::GetMaxForcePoints) | stats `+0x12e` (short); creature `+0x11f4` (int) | not checked | call 0x57eca0 or read stats `+0x12e`/`+0xf6` | med |
| Current FP | live FP pool | effective = `stats+0x130 + stats+0x132`; damage writes `0x130 = eff - dmg` (floor 0); heal writes `0x130 = min(eff + heal, MaxFP)` | 0x005a8560 (damage), 0x005a8660 (heal), 0x0056a800 (regen) | stats `+0x130` (short), `+0x132` (short, "current / extra" per struct tool) | not checked | read `+0x130 + +0x132` | med (meaning of 0x132 open) |
| FP regen per AI tick | FP gained each tick | rate = Out/InCombatFPBase (x `DAT_0098bb10` or `DAT_009950d8` in two state cases) + Wis mod x WisModBonus + feat 0x8f bonus + effect-ID sum; room term: rate -/+ (RoomRating x rate / DAT_0098ba38) when alignment < 41 / >= 60; delta = `creature+0xd0` x `DAT_00986b70` x (rate / FPRegenTime) x cap, cap = `0x132 + MaxFP`; integer part carried in float at `creature+0x1c0` | 0x0056a800 (HandleFPRegeneration); called from 0x00569320 (AIUpdate) line 184 | creature `+0xd0` (uint elapsed), `+0x1c0` (float carry), `+0x520` (combat ptr), `+0x11e8` (state byte), `+0x1110` (mode id) | not checked | hook AIUpdate or reproduce formula; re-read stats `+0x130` after tick | med |
| FPRegenTime | period the regen rate is normalised over | 2DA-style float cell, row `VALUE`, column `FPRegenTime`, via FUN_0071b530; fallback `DAT_00995060` if below `DAT_0098c1e8` | 0x0056a800 (strings 0x009950f4, 0x009950b0) | 2DA source file not identified | not checked | same cell lookup (file unidentified) | med |
| OutOfCombatFPBase / InCombatFPBase / WisModBonus | regen inputs | 2DA-style float cells read by name in the same lookup | 0x0056a800 (strings 0x009950b8, 0x009950c4) | same as FPRegenTime | not checked | same | med-high (names) |
| Spell base FP cost | cost before modifiers | byte `spellRow+0x15a`; row = table + id x 0x1a8 (0x006c1450) | 0x00688620 (GetSpellBaseForcePointCost); loader 0x006c1490 (write ~line 253) | spells table row (2DA-loaded) | script cmd only (nwscript.nss:5866) | `GetSpellBaseForcePointCost(id)` or read row `+0x15a` | high (value), med (2DA column name) |
| Final cast FP cost | FP actually deducted per cast | 0x5482a0: base -> 0x6c4120 alignment multiplier (2DA table by alignment/10, `G` or `E` from row `+0x15b`, floor 1.0, minus `f8 x DAT_0098bd30`) -> round -> +/- RoomRating/20 (add if alignment/10 < 5, subtract if >= 6) -> min 1 -> if effect `DAT_009960d0` present: x50/40/30 % by its int0 = 0/1/2 | 0x005482a0 (cast-event builder); 0x006c4120 (multiplier) | cast event `+0x2c` (dword index 11) = FinalForceCost; saved via 0x005859f0 / loaded 0x00585b00 | script `GetSpellForcePointCost` (nwscript.nss:5615) | read event `+0x2c` or call script cmd | med-high |
| Alignment index | light/dark scaling input | `creature+0x18a` (short) /10 for cost; `+0x18a` < 41 / >= 60 bands for regen | 0x005482a0, 0x006c4120, 0x0056a800 | stats `+0x18a` | not checked | read stats `+0x18a` | med (identity as GoodEvil is hypothesis) |
| Regeneration item property | item-granted regen effect | item prop short index 3 -> effect int0; effect type `DAT_00995f38`, int1 = 6000, applied via 0x00544210 | 0x005b21c0 (ApplyRegeneration); 0x0059fcb0 (OnApplyRegenerate: sets effect int2/int3) | CGameEffect (alloc 0x8c) | item description (not checked) | read effect int0 on target | low-med |

## Evidence
- `D/56/0056a800.c` | 0x0056a800 | `FUN_00733570("FPRegenTime")`; `local_14 = ... * (local_4c / local_f0) * local_20`; `local_dc = min(local_20, local_2c)`
- `D/54/005482a0.c` | 0x005482a0 | `local_24 = FUN_006c4120((int)sVar2, ...)`; `FUN_005e1bd0(1, *(byte*)(iVar3+0x15a))`; `local_11c[0xb] = local_24`
- `D/6c/006c4120.c` | 0x006c4120 | `local_8 = *(byte*)(param_1+0x15a)`; `*(char*)(param_1+0x15b) == 'G'` / `'E'`; `param_2 = param_2 / 10`
- `D/68/00688620.c` | 0x00688620 | `uVar1 = *(undefined1*)(iVar2+0x15a)` (GetSpellBaseForcePointCost)
- `D/57/0057eca0.c` | 0x0057eca0 | `local_24 = *(short*)(stats+0x12e) + (char)*(stats+0xf6) * bVar2`
- `D/58/00585b00.c` | 0x00585b00 | `param_1[0xb] = FUN_00718c80(..., "FinalForceCost")`
- `D/56/00569320.c` | 0x00569320 | `FUN_0056a230(); FUN_0056a800();` gated by `param_1[0x46b] != 0`
- `D/5a/005a8560.c` and `D/5a/005a8660.c` | 0x005a8560 / 0x005a8660 | damage `sum - dmg`; heal `min(sum + heal, FUN_0057eca0())`
- `nwscript.nss:5613-5615` | n/a | "Force Points that were required to cast this spell. This includes ... Room Force Ratings and the Force Body power."
- `B12a/final_rows.tsv` (steam_db_batches) | 0x0056a800 | name `CSWSCreature::HandleFPRegeneration` (GOG-mapped); callees GetMaxForcePoints, GetRoomForceRating

## Open questions
- What are `DAT_00986b70`, `DAT_0098ba38` (likely 100.0), `DAT_0098bd30`, `DAT_0098c1e8`, `DAT_00995060` as values? `static_facts.py addr` returned no value.
- Which 2DA holds row `VALUE` with the regen columns, and is the base FP column `ForcePoints` (string 0x00999884 used by loader 0x006c1490)? spells.tsv export has no FP column.
- Does `stats+0x132` count as extra FP or as max bonus? Regen caps at `0x132 + MaxFP`, heal caps at `MaxFP`.
- Unresolved gates: `creature+0x11ac` (FP alignment on/off), `stats+0xf8` (alignment multiplier discount), `creature+0x1110` ids 0x10a/0x10c, and `FUN_0059fcb0`'s effect int2/int3.
- HP regen (0x0056a230) shares the pattern (`ConModBonus`, `In/OutOfCombatHPBase`) but was not analysed here.
