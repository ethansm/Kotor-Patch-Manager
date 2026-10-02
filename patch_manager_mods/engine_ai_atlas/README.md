# Engine-AI atlas: KOTOR II Steam `swkotor2.exe` (sha 6a522e71), 2026-10-02

Static map of the engine's AI: scheduler, perception, action queue, pathfinding, combat round, follow/party, faction, script interface, effects. No game launch, no installs; every claim is an address in the Ghidra dump or a byte of the vanilla exe. Written to answer "which engine rule produces this companion/enemy behaviour, and where can a mod hook it".

**How it was made.** Phase 0: tools (`cg.py` call graph, `field_census.py`, GOG->Steam name mapper, `atlas_coverage.py`) and mapper TSVs. Wave 1 (AI1-AI4) and wave 2 (AI5-AI8): 2 waves x 4 Sonnet batches (`findings/AI<n>.md`, raw), each followed by an Opus gate (`GATE_wave1.md`, `GATE_wave2.md`: re-read the asm, resolved every batch-vs-prior-doc contradiction, checked windows) and lead spot-checks on disk (`LEAD_SPOTCHECKS.md`). The 8 pages `AI1..AI8` are the gate-reconciled result: where batch and gate differ the page says "batch said X; gate corrected". Confidence words: CONFIRMED = quoted asm line with address; SPECULATIVE = no runtime evidence. DB (read-only here) holds the persisted rows: `struct_fields` 367, `name_map` 3080, `lessons` 328, `candidates` 39.

**How to resume.** Read this README, then the page of the subsystem, then its GATE section for corrections. Windows for hooks: `hook_catalogue.md`. Bugs and their evidence standard: `bug_candidates.md`. Next steps are user-run, log-only probes (hook_catalogue "Phase-4 probe set"), then the user decisions in "Open items". The lead still has to persist the new `candidates` rows listed at the end of `bug_candidates.md`. Regenerate the derived blocks: `python3 tools/atlas_coverage.py --md --classes ...` and `python3 tools/gen_struct_tables.py --pack 2 --splice README.md` (both read the DB with `mode=ro`).

## File index
| path | what |
|---|---|
| [AI1_pathfinding.md](AI1_pathfinding.md) / [AI2_combat_round.md](AI2_combat_round.md) / [AI3_perception.md](AI3_perception.md) / [AI4_scheduler.md](AI4_scheduler.md) | wave 1 pages |
| [AI5_follow_party.md](AI5_follow_party.md) / [AI6_faction.md](AI6_faction.md) / [AI7_script_interface.md](AI7_script_interface.md) / [AI8_effects.md](AI8_effects.md) | wave 2 pages |
| [GATE_wave1.md](GATE_wave1.md), [GATE_wave2.md](GATE_wave2.md), [LEAD_SPOTCHECKS.md](LEAD_SPOTCHECKS.md) | Opus verdicts, name_map corrections, lessons, lead line checks |
| [hook_catalogue.md](hook_catalogue.md) | 101 usable detour windows + simple patches, overlap-checked (`check_atlas_windows_output_2026-10-02.txt`, `verify_atlas_windows_2026-10-02.txt`) |
| [bug_candidates.md](bug_candidates.md) | every AI bug, ranked, evidence standard of docs 15/16, proposed DB rows |
| [SCHEMA.md](SCHEMA.md), [CONTRACT_AI.md](CONTRACT_AI.md), [briefs/](briefs/) | DB schema additions, analyst contract, wave briefs |
| [findings/](findings/) | raw batch output `AI1..AI8.md`, `AI0_mapper_summary.md`, `AI0_map_*.tsv` (mapper) |
| [tools/](tools/) | `cg.py`, `field_census.py`, `atlas_coverage.py`, `gen_struct_tables.py`, `check_atlas_windows.py`, `check_atlas_supp.py`, `persist_atlas_wave{1,2}_db.py`, `tests/` |

## Pipeline (page in brackets; addresses as in the pages)
```
server tick: CServerExoApp 0x533830 (0053396e CALL 0x51D790)
  v
CServerAIMaster::UpdateState 0x51D790 [AI4]  10 ms budget (L4..L1 take 60% of the rest, L0 the remainder), levels 4,3,2,1,0 (obj+0x78)
  |  per-iteration pre-pass 0x51D842..0x51D9EC (AIQ; QuickWalk 0x5C3F70 for +0x220 creatures), event pump 0x51ED10 (AddEventDeltaTime 0x51E170)
  v  vtbl+0x70 per object
CSWSCreature::AIUpdate 0x569320 [AI4]  party/PC forced to L4 (0x5694D6), combat -> L2 (0x56A0B5), L0 drop 0x56C01C (FM7)
  |--> perception [AI3]: SpawnInHeartbeatPerception 0x56B8A0 (005694e3) -> UpdatePerception 0x5D0F30 -> DoPerceptionUpdateOnCreature 0x576DC0
  |      -> LOS 0x52BB60 / spot 0x578850 (sign bug 0x57900F) / listen 0x577FA0 -> list +0x1028 -> SEEN/HEARD events (delta 0, 0x51E170)
  |--> effects [AI8]: UpdateEffectList 0x544AD0 (expiry; state +0xFF5, mask +0x1114, commandable +0xEC gate the Add*Actions)
  |--> round timer [AI2]: UpdateCombatRoundTimer 0x571030 (00569578) -> deferred damage SignalDamage 0x6DCBE0 (005711a8) -> OnApplyDamage 0x5959E0
  |--> RunActions 0x5D56B0 (005695b1) [AI4]: pop node, `JMP [EDX*4+0x5D7038]` (005d5c28); result 1 requeue head, 2/3 done, 4 tail; 1000 us budget
  |       |- 0x01 AIActionMoveToPoint 0x5BFC00 [AI1] -> PlotPath 0x55AB30 (005c1f18) -> 0x55A900 -> 0x55AC80 -> grid 0x550E30 / path-points 0x5532F0
  |       |      -> walker 0x5C3460 / QuickWalk 0x5C3F70 -> step tests 0x5CB430, 0x5CC870 -> BumpFriends 0x586B70 (FM1)
  |       |- 0x11 CheckMoveToObjectRadius 0x5BC1D0 [AI1] (re-approach loop, FM2/FM2b, no LOS)
  |       |- 0x3D FollowLeader 0x5BF7F0 [AI5] (+0x11B0 = 3, needs +0x11AC) -> follow driver 0x5C6340 (CSWParty slot state machine)
  |       |- 0x0C PhysicalAttacks 0x6D7390 / 0x3F Combat 0x6D9500 [AI2] -> StartCombatRound 0x58E730 -> ResolveAttack 0x6DACF0 -> EndCombatRound 0x58E990
  |--> slot-6 3 s fallback [AI2]: +0x570 <= 0 in combat, RunScript at 0x569A67, reset 0x569A72
events back to scripts: CSWSCreature::EventHandler 0x56EFD0 -> slot table below -> CVirtualMachine::RunScript 0x6FD8D0 [AI7]
  OnNotice slot 1 (0x576DC0 posts) | OnAttacked slot 3 (event 0xF, 0056faa1) | OnDamaged slot 4 (0x5966d0, not for the leader) [AI2]
  EndRound slot 6: THREE sites 0x58F50F (EndCombatRound), 0x569A67 (fallback), 0x59E904 (SetState removal, ungated) [AI2/AI8]
  Dialogue slot 7 (shout 0x547100 -> 0x57074F, 0x5709f0) [AI6] | OnBlocked slot 12 (event 10 payload 0x1F, 6 post sites, 0x5706A4) [AI1]
scripts back into the engine: NCS -> ExecuteCommand 0x668FD0 -> handler (ActionAttack 0x66DAF0, ...) -> AddAttackActions 0x568540
  -> AddAction 0x53F7F0 / AddActionToFront 0x540CA0 -> the queue above [AI7, AI4]; hostility everywhere: GetReputation 0x5E13C0 -> 0x5D0D00 -> 0x664580 [AI6]
```

## Per-class coverage (`python3 tools/atlas_coverage.py --md --classes CServerAIMaster,CPathfind,CPathfindInformation,CSWSFaction,CFactionManager,CSWSCombatRound,CSWSCreature,CSWSObject,CSWSArea,CSWSAreaOfEffectObject,CSWSEffectListHandler,CGameEffect,CVirtualMachine,CSWVirtualMachineCommands,CSWPartyTable,CSWParty,CFollowInfo,CAction`)
`mapped` = name_map rows `Class::*`; `high` = of those high confidence; `verified_in_atlas` = source `engine-ai-atlas*`; `open` = mapper rows without a name_map row; `struct_fields` = rows in the DB table (all confidences).
| class | mapped | high | verified_in_atlas | open | struct_fields |
|---|---:|---:|---:|---:|---:|
| CServerAIMaster | 7 | 6 | 5 | 9 | 10 |
| CPathfind | 0 | 0 | 0 | 0 | 0 |
| CPathfindInformation | 4 | 1 | 4 | 0 | 62 |
| CSWSFaction | 1 | 1 | 1 | 27 | 8 |
| CFactionManager | 3 | 3 | 3 | 12 | 4 |
| CSWSCombatRound | 51 | 1 | 2 | 0 | 47 |
| CSWSCreature | 348 | 16 | 13 | 0 | 62 |
| CSWSObject | 114 | 1 | 0 | 0 | 27 |
| CSWSArea | 111 | 2 | 2 | 0 | 14 |
| CSWSAreaOfEffectObject | 3 | 0 | 1 | 17 | 10 |
| CSWSEffectListHandler | 8 | 3 | 8 | 80 | 0 |
| CGameEffect | 3 | 1 | 2 | 18 | 10 |
| CVirtualMachine | 2 | 0 | 0 | 20 | 0 |
| CSWVirtualMachineCommands | 21 | 1 | 0 | 0 | 3 |
| CSWPartyTable | 8 | 7 | 6 | 69 | 9 |
| CSWParty | 6 | 2 | 6 | 15 | 14 |
| CFollowInfo | 1 | 1 | 1 | 0 | 9 |
| CAction | 0 | 0 | 0 | 0 | 6 |
| TOTAL | 691 | 46 | 54 | 267 | 295 |

Reading: most `mapped` rows predate the atlas (mapper/DB names, medium); the high-confidence set is what the gates quoted. The big `open` counts (CSWSEffectListHandler 80, CSWPartyTable 69, CSWSFaction 27) are mapper hits nobody has read; `struct_fields` for classes outside this list (CSWSCreatureStats, CSWSCombatAttackData, CServerExoAppInternal, ...) are in the DB too (367 rows total).

## Action types (RunActions table 0x5D7038, index = type-1, 0x47 entries; AI4)
Verified: **G** = gate-decoded from the vanilla exe table (types 1, 0xC, 0x11, 0x1E, 0x3D, 0x3F, 0x47); every other row has table bytes high, handler identity medium (DB/mapper names, unnamed = SPECULATIVE). `c` = guarded by creature type byte 5, a failed guard leaves result 3 (node deleted). 67 typed slots, 66 handlers, 4 constant stubs.
| type | handler | name | verified |
|---|---|---|---|
| 0x01 | 0x5BFC00 | AIActionMoveToPoint (stops the loop) | G |
| 0x02 / 0x03 / 0x0A / 0x12 | 0x5BBF40 / 0x5BB5A0 / 0x5BC700 / 0x5BCA60 | CheckMoveToObject / ToPointRadius / ToPoint / ToPoint_2 | table |
| 0x04 / 0x13 | 0x5BBB70 / 0x5BB2C0 | ChangeFacingPoint / ChangeFacingObject | table |
| 0x05 / 0x30 / 0x33 | 0x5BE950 / 0x5BE1F0 / 0x5CCC50 | JumpToPoint / JumpToObject / Drive (each stops the loop) | table |
| 0x06 / 0x0E / 0x17 / 0x21 / 0x3E | 0x5D9DA0 / 0x5D9890 / 0x5D9BF0 / 0x5D97E0 / 0x5D7480 | PlayAnimation / Speak / PlaySound / SpeakStrRef / BarkString | table |
| 0x07 / 0x08 / 0x09 / 0x0B | 0x5CF8A0 / 0x5BE0A0 / 0x5CFF50 / 0x5D0A80 | PickUpItem / EquipItem / DropItem / UnequipItem | table |
| 0x0C | 0x6D7390 | AIActionPhysicalAttacks (obj+0x88 = param0 around the call) | G |
| 0x0F / 0x2E | 0x5B96F0 (placeable 0x5F6180) / 0x5B8000 | CastSpell / ItemCastSpell | table |
| 0x11 | 0x5BC1D0 | CheckMoveToObjectRadius (same area && use range, no LOS) | G |
| 0x14 / 0x15 / 0x24 | 0x5DB0A0 / 0x5DAD10 / 0x5BD0B0 | OpenDoor / CloseDoor / EncounterCreatureDestroySelf | table |
| 0x18 / 0x25..0x28 | 0x5D75B0 / 0x5D99D0, 0x5DB8F0, 0x5DDA60, 0x5DFBE0 | DialogObject / DoCommand, Unlock, Lock, UseObject | table |
| 0x19..0x1D | 0x5B43C0, 0x5B3380, 0x5B2C10, 0x5B5360, 0x5B5AD0 | Disarm/Recover/Flag/Examine/SetMine | table |
| 0x1E | 0x5D9AA0 | AIActionWait | G |
| 0x22 / 0x23 / 0x16 | 0x5DA4D0 / 0x5DAB80 / 0x5CF7B0 | GiveItem / TakeItem / OrientCamera | table |
| 0x2C / 0x2D / 0x2F / 0x31 / 0x32 | 0x5BB920 / 0x5BF000 / 0x5E0E70 / 0x5BB470 / 0x5D25E0 | InterAreaPathing / RandomWalk / SetCommandable / MoveAwayFromLocation / CounterSpell | table |
| 0x37 / 0x3A / 0x43 / 0x44 | 0x5CEED0 / 0x5CF660 / 0x5DEC20 / 0x5D29F0 | ForceFollowObject / CheckForceFollowObject / DoorSaber / StanceTotalDefense | table |
| 0x3D | 0x5BF7F0 | AIActionFollowLeader (c + `+0x11AC`; leader INVALID returns 1) | G |
| 0x3F | 0x6D9500 | AIActionCombat (pops round actions `+0xB00`) | G |
| 0x40 / 0x46 | 0x5BCDC0 / 0x5BF940 | CheckMoveToFollowRadius (`+0x11AC`) / FollowOwner (`+0x11FC`) | table |
| 0x47 | 0x5DF920 | ReequipResumeAttack (name low, body per gate) | G |
| 0x10, 0x1F, 0x20, 0x34, 0x35, 0x38, 0x3C, 0x41, 0x45 | 0x5D0C00, 0x5D9570, 0x5D9690, 0x5D17F0, 0x5D18C0, 0x5D19B0, 0x5BEFC0, 0x5D29B0, 0x5D2EA0 | unnamed (0x38 mapper: Heal, low) | table |
| 0x29 / 0x2B / 0x36 / 0x2A | 0x5D1990 / 0x5D17D0 | constant stubs `return 3` / `return 2` | table |
| 0x0D, 0x39, 0x3B, 0x42, > 0x47 | none (`0x5D688E`) | no handler, node deleted | G (decode) |

## Event id -> script slot (`0x56EFD0`, slots at creature `+0x270 + 8n`; AI7)
| slot | name | engine run site | trigger / skip |
|---|---|---|---|
| 0 | Heartbeat | 0x56B8A0 | every AIUpdate pass; interval `+0x398` 3000..4199 ms; skipped if dead |
| 1 | OnNotice | 00570640 / 0057069a | event 10 payload 1 (0x576DC0); party-flag creatures only if `0x7E5C40(clientId)` |
| 2 | SpellAt | 00570d08 | event 10 payload 2 (poster not traced) |
| 3 | Attacked | 0056faa1 | event 0xF per hit; not if dead or party HP <= 0 |
| 4 | Damaged | 005966d0 (OnApplyDamage 0x5959E0, direct) | not for the party leader |
| 5 | Disturbed | 00570f55 | event 10 payload 0x1B |
| 6 | EndRound | 0x58F50F, 0x569A67, 0x59E904 | 3 sites; third ungated (SetState removal) |
| 7 | Dialogue | 005709f0 | event 10 payload 7 (shout); `+0x1BC` = pattern number; empty resref -> `k_hen_dialogue01` |
| 8 | Spawn | 0x56B8A0 first block; 0x599A10 | once per spawn |
| 9 | Rested | none found (absence, med) | - |
| 10 | Death | 0x597D90 | after `0x58E990(0)` |
| 11 | UserDefined | 00570a72 | event 10 payload 0xB (SignalEvent, EventUserDefined) |
| 12 | OnBlocked | 00570745 | event 10 payload 0x1F (6 movement sites); skipped for the player-controlled creature; sets `+0x37C` |
| 13 | EndDialogue | 0x58A500 (caller 0x6CF740) | conversation end |

## Struct fields (generated)
<!-- struct_fields:begin -->
Source: DB table `struct_fields`, confidence high/medium only (c: h = high, m = medium); offsets are relative to the class base (a sub-object keeps its own class). Regenerate with `python3 tools/gen_struct_tables.py --pack 2 --splice README.md`.

#### CSWSCreature (61 rows high/medium; 1 low omitted)
| offset | size | type | name | c | offset | size | type | name | c |
|---|---:|---|---|---|---|---:|---|---|---|
| 0xD0 | 4 | int | dt since last AIUpdate | h | 0xEC | 4 | int | commandable (ClearAllActions 0x541080 is... | h |
| 0x100 |  | list | action list (queue) | h | 0x170 | 4 | int | spell id | m |
| 0x1A0 | 4 | int | last speaker id | h | 0x1A4 | 4 | int | last shout event int(2) | m |
| 0x1A8 | 8 | CExoString | last shout event string(0) | m | 0x270 | 8 | script slot n | script slots (0 heartbeat, 1 notice, 8 sp... | h |
| 0x37C | 4 | id | last blocker (OnBlocked caller) | h | 0x380 | 4 | ptr CPathfind... | pathfindInfo (new(0x278)) | h |
| 0x384 | 4 | int | stats dirty (UpdateAttributes) | m | 0x38C | 4 | int | first-run done flag | h |
| 0x390 | 8 | day,ms | heartbeat stamp [offsets 0x390/0x394] | h | 0x394 | 4 | int | heartbeat stamp (day, ms); set to (0,1) b... | h |
| 0x398 | 4 | ms | heartbeat interval 3000..4199 | h | 0x39C | 4 | ms | mode-1 interval 300..699 (IsPC non-party) | h |
| 0x3A0 | 8 | day,ms | mode-1 stamp (never stored after ctor) [o... | m | 0x3BC | 8 | day,ms | dice-reroll stamp [offsets 0x3BC/0x3C0] | h |
| 0x3C8 | 8 | day,ms | block-3 stamp [offsets 0x3C8/0x3CC] | h | 0x3D0 | 4 | int | block-3 counter | h |
| 0x3D4 | 4 | int | heartbeat counter | h | 0x500 | 4 | ptr CFollowInfo | pFollowInfo | h |
| 0x510 | 1 | u8 | detect mode | h | 0x511 | 1 | u8 | stealth mode | h |
| 0x520 | 4 | int | inCombat | h | 0x524 | 4 | int | combatExitTimer | h |
| 0x528 | 4 | int | timerFrozen | h | 0x52C | 4 | int | bPassive / end instead of approach | h |
| 0x53C | 4 | int | size class (1..5, NWN-like) | m | 0x548 | 4 | objid | attack target | h |
| 0x550 | 4 | objid | attempted attack target | h | 0x558 | 12 | vec3 | last LOS-blocked position | h |
| 0x564 | 4 | objid | last attacker | h | 0x568 | 4 | objid | spell target | h |
| 0x56C | 4 | objid | ? | h | 0x570 | 4 | int | fallbackTimer | h |
| 0x580 | 4 | objid | last hostile target | h | 0xFD0 | 4 | int | silenced | h |
| 0xFF4 | 1 | u8 | sight bits 1 see-inv, 2 ultra, 4 true see... | h | 0xFF5 | 1 | u8 | current SetState id (highest wins) | h |
| 0xFF8 | 4 | int | effect spell id (spell being cast, else -... | h | 0x1008 | 20 | int | last perceived object, heard, seen, inaud... | h |
| 0x101C | 4 | float | PrimaryRange (spot) | h | 0x1020 | 4 | float | SecondaryRange (listen) | h |
| 0x1024 | 4 | u8 | stealth dice (11..20) x2 / spot,listen di... | h | 0x1028 | 4 | ptr | perception list array | h |
| 0x102C | 4 | count | count | h | 0x106C | 4 | int | spell-cast in progress | m |
| 0x1070 | 4 | int | spell-slot payload | m | 0x10BC | 32 | int | countdown timers[8] [range 0x10BC..] | m |
| 0x10DC | 4 | ptr CSWSComba... | combatRound (separate heap object, new(0x... | h | 0x10E8 | 4 | int | IsPC cache | h |
| 0x110C | 1 | u8 | stance (0xFF none, 0 total defense, 1 med... | h | 0x1114 | 2 | u16 | allowed-action mask (init 0xFFFF, death 0... | h |
| 0x1198 | 4 | ptr | stats | h | 0x11AC | 4 | int | PartyMemberFlag (1 = PC or a current CSWP... | h |
| 0x11B0 | 4 | i32 | walk state (b.Q7) | h | 0x11BC | 4 | int | currently moving (MoveToPoint) | h |
| 0x11C8 | 4 | int | suppress perception events (dead gate) | h | 0x11CC | 4 | ms | excited timer (name low) | m |
| 0x11FC | 4 | int | PuppetOwnerFlag | m |  |  |  |  |  |

#### CSWSCombatRound (46 rows high/medium; 1 low omitted)
| offset | size | type | name | c | offset | size | type | name | c |
|---|---:|---|---|---|---|---:|---|---|---|
| 0x4 | 2660 | CSWSCombatAtt... | attacks (5 used) | h | 0xA68 | 12 | ushort array... | SpecialAttack list [offsets 0xA68/0xA6C/0... | h |
| 0xA74 | 12 | ushort array | SpecialAttackId list [offsets 0xA74/0xA78... | h | 0xA80 | 2 | ushort | AttackID | h |
| 0xA84 | 4 | int | RoundStarted (active) | h | 0xA88 | 4 | int | SpellCastRound | h |
| 0xA8C | 4 | int | spell-round param (suppresses slot 6) | h | 0xA90 | 4 | int | cutscene-attack / no-script flag | h |
| 0xA94 | 4 | int ms | Timer | h | 0xA98 | 4 | int | debilitated-at-attack | m |
| 0xA9C | 4 | int ms | RoundLength (remaining budget) | h | 0xAA0 | 4 | int | OverlapAmount (cap 1000) | h |
| 0xAA4 | 4 | int | BleedTimer | h | 0xAA8 | 4 | int | RoundPaused | h |
| 0xAAC | 4 | objid | RoundPausedBy | h | 0xAB0 | 4 | int ms | PauseTimer | h |
| 0xAB4 | 4 | int | InfinitePause | h | 0xAB8 | 4 | int | actions started this round | h |
| 0xABC | 1 | byte | CurrentAttack index | h | 0xAC0 | 4 | int | off-hand attack flag | h |
| 0xAC4 | 1 | byte | AttackGroup | h | 0xAC8 | 4 | int | DeflectArrow | h |
| 0xACC | 4 | int | WeaponSucks | h | 0xAD0 | 4 | int | ParryIndex | h |
| 0xAD4 | 4 | int | NumAOOs | h | 0xAD8 | 4 | int | NumCleaves | h |
| 0xADC | 4 | objid | NewAttackTarget | h | 0xAE0 | 4 | int | OnHandAttacks | h |
| 0xAE4 | 4 | int | OffHandAttacks | h | 0xAE8 | 4 | int | OffHandTaken | h |
| 0xAEC | 4 | int | ExtraTaken | h | 0xAF0 | 4 | int | AdditAttacks | h |
| 0xAF4 | 4 | int | EffectAttacks | h | 0xAF8 | 1 | byte | ParryActions | h |
| 0xAFC | 4 | objid | DodgeTarget | h | 0xB00 | 4 | ptr | round action list (CExoLinkedList of 0x88... | h |
| 0xB04 | 4 | ptr | owner creature | h | 0xB08 | 4 | int | Engaged | h |
| 0xB0C | 4 | int | Master | h | 0xB10 | 4 | objid | MasterID | h |
| 0xB14 | 4 | objid | slave id | h | 0xB18 | 4 | ptr | current round action | h |
| 0xB1C | 1 | byte | its type | h | 0xB20 | 4 | objid | round target | h |
| 0xB24 | 1 | byte | last action type | m | 0xB28 | 4 | int | unknown | m |

#### CPathfindInformation (54 rows high/medium; 8 low omitted)
| offset | size | type | name | c | offset | size | type | name | c |
|---|---:|---|---|---|---|---:|---|---|---|
| 0x0 | 4 | u32 | bClientPath (node flag bit1) | h | 0x4 | 4 | f32 | PERSPACE radius (obstacle/walkmesh), def 0.6 | h |
| 0x8 | 4 | f32 | CREPERSPACE radius (creature spacing, bum... | h | 0x10 | 4 | f32 | CAMERASPACE (def = +0x8) | m |
| 0x14 | 4 | f32 | HEIGHT (def 1.0) | h | 0x24 | 4 | u32 | closest-result enabled (set every grid ca... | m |
| 0x2C | 4 | id | owner creature object id | h | 0x30 | 4 | id | move target object id (0x7F000000 none) | h |
| 0x3C | 4 | u32 | pathfind call counter | h | 0x58 | 4 | u32 | bNewRequest (1 = reset+search) | h |
| 0x5C | 12 | vec3 | goal position [offsets 0x5C/0x60/0x64] | h | 0x68 | 4 | f32 | approach range | h |
| 0x70 | 4 | id | goal area id | h | 0x74 | 12 | vec3 | start position [offsets 0x74/0x78/0x7C] | h |
| 0x80 | 4 | id | start area id | h | 0x84 | 4 | u32 | start/end flipped | m |
| 0x88 | 4 | u32 | search done (1) / running (0) | h | 0x8C | 4 | u32 | path length in floats (2/point) | h |
| 0x90 | 4 | ptr | path float[] | h | 0x94 | 8 | u32/ptr | grid/path-point result count + array [off... | m |
| 0x9C | 4 | u32 | path cursor | m | 0xA0 | 4 | u32 | inter-area route found | h |
| 0xA8 | 4 | u32 | areas on route (10 = planning) | h | 0xAC | 8 | ptr | route arrays [offsets 0xAC/0xB0] | m |
| 0xB4 | 8 | u32/ptr | excluded-area list (re-route) [offsets 0x... | h | 0x120 | 60 | mixed | grid transform basis [range 0x120..0x158] | m |
| 0x15C | 4 | ptr | grid temp path buffer (alloc/free per call) | h | 0x160 | 4 | u32 | grid result point count | m |
| 0x168 | 4 | i32 | grid resume depth | h | 0x16C | 4 | ptr | grid DFS table cache (freed by Shutdown) | h |
| 0x170 | 24 | vec3 x2 | start/goal snapshot (restart test) [offse... | m | 0x188 | 4 | u32 | best (closest) path point count, returned... | h |
| 0x18C | 4 | ptr | best (closest) path buffer, returned as S... | h | 0x190 | 4 | i32 | best distance-to-goal (init 700) | h |
| 0x194 | 4 | u32 | force grid search flag | h | 0x198 | 4 | u32 | straight-bisect request (node bit3) | h |
| 0x19C | 4 | i32 | cached first straight-test code | h | 0x1A4 | 8 | i32 | path-point start/goal node ids [offsets 0... | m |
| 0x1AC | 4 | ptr | path-point array (freed by Shutdown) | h | 0x1B4 | 4 | ptr | DFS table cache while pending, NOT freed | h |
| 0x1B8 | 24 | vec3 x2 | path-point start/goal snapshot [range 0x1... | m | 0x1D0 | 8 | f32 | f-limit | m |
| 0x1D4 | 8 | f32 | best (iterative deepening) | m | 0x1D8 | 4 | ptr | second array, NOT freed | h |
| 0x1E0 | 4 | u32 | path-point search state 1..5 | h | 0x200 | 4 | ptr | result ownership, NOT freed | m |
| 0x240 | 4 | u32 | bRangeStopInWalker (node bit10) | h | 0x244 | 4 | u32 | go-around side flag (mirrored between two... | m |
| 0x248 | 12 | vec3 | last target position [range 0x248..0x250] | m | 0x254 | 4 | id | last blocker creature id | h |
| 0x258 | 12 | vec3 | blocker position at block [range 0x258..0... | m | 0x264 | 4 | id | blocker for k_def_pathfail01 | m |
| 0x268 | 4 | u32 | consecutive blocked steps (abort > 5) | h | 0x26C | 12 | u32 | QuickWalk forced-move flag/deadline [offs... | m |

#### CAction (5 rows high/medium; 1 low omitted)
| offset | size | type | name | c | offset | size | type | name | c |
|---|---:|---|---|---|---|---:|---|---|---|
| 0x0 | 4 | int | type | h | 0x4 | 4 | int | param[i].kind (1 int,2 float,3 object id,... | h |
| 0x38 | 4 | int | param[i].value [stride 0x38 + 4*i] | h | 0x6C | 2 | word | groupId | h |
| 0x6E | 2 | word | paramCount (= last index+1) | h |  |  |  |  |  |

#### CSWSObject (27 rows high/medium; 0 low omitted)
| offset | size | type | name | c | offset | size | type | name | c |
|---|---:|---|---|---|---|---:|---|---|---|
| 0x4 | 4 | id | objectId | h | 0x8 | 1 | byte | objectType (4 area, 3 module, 5 creature,... | h |
| 0x14 | 2 | word | actionGroupCounter | h | 0x16 | 2 | word | currentGroup | h |
| 0x78 | 4 | int | aiLevel (-1 none) | h | 0x7C | 1 | byte | savedAiLevel | m |
| 0x84 | 2 | word | currentActionType (0xFFFF none) | h | 0x94 | 12 | vec3 | position [offsets 0x94/0x98/0x9C] | h |
| 0xA0 | 12 | vec3 | facing | m | 0xAC | 4 | int | lastRunActions day | m |
| 0xB0 | 4 | int | lastRunActions time | m | 0xC0 | 4 | int | actionStart day | h |
| 0xC4 | 4 | int | actionStart time | h | 0xD8 | 4 | u16 used | current animation id | m |
| 0x100 | 4 | list | actionQueue | h | 0x148 | 4 | CGameEffect** | effect list ptr/count (sorted by type) [+... | h |
| 0x14C | 4 | int | effect list ptr/count (sorted by type) [+... | h | 0x1B0 | 4 | array | listen pattern entries (entry: +0 number,... | h |
| 0x1B4 | 4 | count | listen pattern entries (entry: +0 number,... | h | 0x1BC | 4 | int | last listen pattern number | h |
| 0x1C0 | 4 | int | listening flag | h | 0x20C | 4 | ptr | currentActionNode | h |
| 0x210 | 2 | u16 | GetFirst/NextEffect cursor | h | 0x220 | 4 | int | playerUpdateSetFlag (1 = in player update... | h |
| 0x224 | 12 | bitset | dirtyFlags (bit i: word [+0x224+(i>>5)*4]... | m | 0x258 | 4 | int | "faction effect active" flag (type 115) | m |
| 0x260 | 4 | int | ForceAlwaysUpdate (bypass range) | h |  |  |  |  |  |

#### CFollowInfo (9 rows high/medium; 0 low omitted)
| offset | size | type | name | c | offset | size | type | name | c |
|---|---:|---|---|---|---|---:|---|---|---|
| 0x0 | 4 | objid | FollowObject (default 0x7f000000) | h | 0x4 | 12 | vec3 | FollowLocation | h |
| 0x10 | 12 | vec3 | LastLeaderPos | h | 0x1C | 12 | vec3 | LastFollowerPos | h |
| 0x28 | 4 | float | MaxSpeed | h | 0x2C | 1 | byte | StickToPos | m |
| 0x30 | 4 | int | Result (default 2) | h | 0x34 | 4 | int | TimeElapsed | m |
| 0x38 | 1 | byte | InSafetyRange (default 1) | h |  |  |  |  |  |

#### CSWParty (11 rows high/medium; 3 low omitted)
| offset | size | type | name | c | offset | size | type | name | c |
|---|---:|---|---|---|---|---:|---|---|---|
| 0x0 | 4 | int | memberCount (max 3) | h | 0x24 | 408 | slot element E | slots[3] [stride 0x24 + s*0x88] | h |
| 0xE4 | 12 | vec3 | slot-1 formation offset (= slot element +... | h | 0x16C | 12 | vec3 | slot-2 formation offset (= slot element +... | h |
| 0x1BC | 4 | int | formation preset (init -10000) | h | 0x1C0 | 4 | float | formation spacing | h |
| 0x1C4 | 1 | byte | mirror flag | h | 0x1C8 | 108 | 3 x (3 vec3) | preset table (slot1, slot2, unused) | h |
| 0x234 | 12 | float[3] | preset spacing (1.5, 1.5, 1.5) | h | 0x2BC | 4 | float | retreat timer | m |
| 0x2DC | 4012 | CSWPartyFollo... | 100 x 0x28 ring; +0xFA0 back pointer to P... | h |  |  |  |  |  |

#### CSWPartySlot (12 rows high/medium; 3 low omitted)
| offset | size | type | name | c | offset | size | type | name | c |
|---|---:|---|---|---|---|---:|---|---|---|
| 0x0 | 4 | objid | client object id (INVALID 0x7f000000) | h | 0x4 | 4 | int | follow state -1..10 | h |
| 0x18 | 12 | vec3 | resolved world target of the follower | h | 0x24 | 4 | int | trail node of E+0x18 | m |
| 0x28 | 12 | vec3 + node | trail point at follow distance, node id +... | h | 0x38 | 12 | vec3 | leader-local formation offset | h |
| 0x44 | 4 | int | loop counter (guard 0x32) | h | 0x48 | 4 | float | current follow speed | m |
| 0x5C | 12 | vec3 | creature position snapshot (reset) | m | 0x68 | 4 | int | "snapshot pending" | m |
| 0x6C | 4 | int | blocked timer (ms) | m | 0x70 | 4 | int | **FM6 hold flag** (1 after leader change) | h |
<!-- struct_fields:end -->

## Key results per page
- **AI1 pathfinding.** FM2 and FM2b are one loop: an unreachable target first gets a partial path (result 2, walk to the obstacle), then FAIL (3) + a 2-4 s wait; action 0x11 has NO LOS test (dead block `005bc4ba`), so fail-fast keys on the 0x11 re-approach count (`0x5BC5F4`). OnBlocked is posted from 6 unthrottled sites (R1 listed 3). Pathfinder has one caller (`0x5BFC00`), 1000 us slice, level-0 creatures cannot path, bump counter > 5 ends a move silently, PI Shutdown leaks 4 blocks (PFLK).
- **AI2 combat round.** Round is a separate 0xB2C object at `+0x10DC`; one 0xC call resolves all attacks, damage is deferred to `0x571030`. A1 root cause: `+0x570` re-armed in the 0xC prologue (`0x6D740F`). Retarget metric mismatch (3D-minus-radii picker vs 2D attack test) gate-verified. Only the controlled leader gets engine target choice; companions/enemies are script-only; slot 6 has exactly 3 engine sites.
- **AI3 perception.** Spot distance sign BUG (`0x57900F`); P1 mode-1 stamp never stored (fix shipped in companion-ai, INI `p1`, default off); R1 scan abort narrowed to modes 0/2 (enemy aggro safe). Companion refresh 4 s (P2), FM7 level drop at `0x56C01C`, combat uses 2DA row 0x12 ranges, creatures never block LOS (E3, med), `ModSpotCheck` is never read.
- **AI4 scheduler.** 10 ms budget with 60% per level, pre-pass quadratic in n_L (AIQ), all items sit at level 0; `+0x220` is the player update-set flag, not a party flag. Action table decoded (above); no stuck/timeout logic in RunActions; FM3 sentinel mechanism confirmed (reach low); dismissed party member lands at level 2.
- **AI5 follow/party.** `+0x11AC` = PC + selected followers (max 2 table members); correct test `*(cr+0x11AC) != 0`, `0x5FB070` returns 0 for the PC. 15 SetPartyMemberFlag sites; FM4B patches A `0x574529` + B `0x575160` only together. 11-state follow driver `0x5C6340`; the engine never re-queues 0x3D after ClearAllActions (FM1); SpawnNPC joins the PC faction without the flag.
- **AI6 faction/shouts.** Reputation = O(1) matrix lookup, no cache; flag-0 companions use the PC faction row; enemy <= 10, friend >= 90; global override `[0xA0F6E0]`. AdjustReputation reads the transposed cell (latent). Shout path: radii 1000/3/250/35/1000 by volume, no faction filter, HEARD plus (seen or < 10 m); E9 delivery is conditional.
- **AI7 script interface.** Command table re-derived (774 of 877 slots). `GetCurrentAction` = last reportable node of the head group: engine approach reports 3 / 39, follow 38, MOVETOPOINT only for script moves (E4). `0x6F2A30` = IsForceUserClass so `GetCreatureHasTalent(medpac)` is FALSE for Kreia/Visas; `nCRMax` dead, `nExcludeId` not applied to spells.
- **AI8 effects.** SetState id map and precedence (`+0xFF5`, numeric order); `+0xEC` makes ClearAllActions a no-op (ActionMoveToLocation is NOT gated); `+0x1114` bits 0x2 move, 0x4+0x80 attack, 0x8 spells. GetHasSpellEffect keys on the creator spell id (BM test correct per member, droids never report). EFFOVF has no dedup/cap; AoE objects run at level 0 with a 6 s heartbeat.

## Top open items
1. User-run log-only probes (hook_catalogue "Phase-4 probe set"): FM2c `0x55B69A` (partial vs fail split), SHOUT-TX/RX `0x547100`/`0x57074F`, party-flag census `0x58A35E`, RunActions histograms `0x5D5C22`/`0x5D6A60`, GetCurrentAction `0x67B204`; also repute.2da / ranges.2da / StateScripts.2da are not on disk.
2. User decisions: spot-sign fix (balance), FM4B patches A+B vs keep FM4 mode 1, E4 key (position progress vs 0x11 count), E9 shout design, any scheduler change (FM3, AIQ).
3. Script fixes without engine risk: medpac for Force-only members (correct lesson 280 and R4 row 4), E4 key.
4. Unread: QuickWalk `0x5C3F70` (8 KB), `0x585CC0` use-range metric vs the 2D 0xC test (approach ping-pong candidate), effect type 89 `0x599A10`, `0x669020`, unnamed handlers, who ends a conversation's level 3.
5. Lead: persist the proposed `candidates` rows, update lesson 280, re-run `check_atlas_windows.py` after any new hook set.
