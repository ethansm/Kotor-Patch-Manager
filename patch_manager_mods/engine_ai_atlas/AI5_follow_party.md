# AI5 Follow, party flag and formation (KOTOR II Steam swkotor2.exe 6a522e71)

Pipeline position: `CSWSCreature::AIUpdate 0x569320` -> `RunActions 0x5D56B0` -> action `0x3D` `AIActionFollowLeader 0x5BF7F0` (sets `+0x11B0 = 3`, returns 1) -> scheduler pre-pass / `WalkUpdateLocation_QuickWalk 0x5C3F70` (AI4) -> **follow driver `0x5C6340`** (11-state machine on the client `CSWParty` slot element, states handled by `0x5C7370..0x5CB190`) -> movement (AI1). Around that sit the party flag `creature+0x11AC` (`SetPartyMemberFlag 0x58A330`), the server `CSWPartyTable` (max 2 followers + PC), the client `CSWParty` (3 slots, leader trail ring, formation presets), the leader switch (`SetLeaderBySlot 0x7E5DF0`), and the save path (`SaveCreature 0x574500`, FM4). Engine follow runs only while a `0x3D` node stays queued: the engine queues it at two sites only (`AddCharacter 0x7E5A6E`, script `ActionFollowLeader 0x66A04E`).

## Status
- Sources: batch `findings/AI5.md` (Sonnet) + `GATE_wave2.md` (Opus gate, 2026-10-02; section 1 AI5 verdicts, 1.1 party flag, 1.2 FM4 second byte, section 2 reconciliation, 3 confidence exceptions, 4 names, 5 windows, 6 lessons 1-3) + `LEAD_SPOTCHECKS.md` (Wave 2 AI5: `+0x11AC` writers, FM6 consumer lines, SaveCreature restore lines all CONFIRMED).
- Date: 2026-10-02. Static analysis only. Gate-verified: `+0x11AC` semantics and test, 15 call sites (gate re-read sites 3, 6, 7, 11, 12, 14, 15), FM4 + patch B dependency, `0x5FB070` PC caveat, `0x5FE4B0`, `0x66B770/0x66B8A0`, FM6 consumer constants, SpawnNPC. Speed thresholds and 2DA numbers stay low (2DA not on disk).
- Conflicts resolved to the gate version are marked "batch said X; gate corrected".
- Full CFollowInfo / CSWParty / slot-element / CSWPartyTable field sets are in the batch (e) tables and DB `struct_fields`; this page lists the important rows.

## Function table
| addr | name | conf | purpose |
|---|---|---|---|
| 0x58A330 | CSWSCreature::SetPartyMemberFlag(flag, apply) | high | writes `+0x11AC` (`0058a35e`); apply!=0: flag 0 frees `+0x500` and `SetAILevel(2)` (`0058a448/0058a458`), flag 1 allocs FollowInfo (`0058a384 PUSH 0x3C`, `0058a3a1 CALL 0x5752C0`) and `SetAILevel(4)`; 15 call sites (below) |
| 0x5752C0 / 0x575340 | CFollowInfo::CFollowInfo / default init | high | renamed by gate (was RemoveMindTrickEffects, low); `[0]=0x7F000000`, `[0xC]=2`, `[0xE]=1` |
| 0x575200 / 0x561D80 / 0x5751B0 | LoadFollowInfo / GFF loader / SaveFollowInfo | high | struct "FollowInfo"; loader allocs `+0x500` without touching `+0x11AC` |
| 0x574500 | CSWSCreature::SaveCreature | high | `0x58A330(0,1)` at `0x574532`, GFF write, `0x58A330(old,1)` at `0x57516B`, then `0x5751B0` (FM4) |
| 0x5BF7F0 | AIActionFollowLeader (type 0x3D) | high | never completes: 3 on any gate failure, else `+0x11B0 = 3` (`005bf925`) and returns 1 |
| 0x5BF940 / 0x5BCDC0 | AIActionFollowOwner (0x46) / AIActionCheckMoveToFollowRadius (0x40) | med | puppet follow (`+0x11FC`) / post-move check, dist^2 > (r+0.01)^2 -> slot state 5 (`005bd09e`) |
| 0x5C3F70 | WalkUpdateLocation_QuickWalk | high | calls the follow driver iff `+0x220 != 0 && +0x11B0 != 1 && vt+0x94 == 0 && +0x11AC` and `+0x11B0 == 3` (`005c4142`, `005c4151 CALL 0x5C6340`) |
| 0x5C6340 | FollowLeaderDriver (QuickWalk_FollowLeader) | high | per-follower state machine, slot via `0x7E6BE0(clientId)`; slot 0 returns 2; switch `005c6ace JMP [ECX*4+0x5C7334]` |
| 0x5C7370 | CSWSCreature::FollowState0_PlaceBehindTrail | med | trail point + R(heading) * offset; queues 0x40 + run MoveToPoint; state 6/7. Gate: queues in the 0x3D node's group (`FUN_005d5120(0x3d)`), so GetCurrentAction reports 38 |
| 0x5C7AD0 / 0x5C87E0 / 0x5C8C20 / 0x5CA000 | FollowState1 walk trail / 2 blocked / 3 offset walk / 4 to trail end | low-med | OnBlocked posts in states 2 and 3 (`0x5C8BB4`, `0x5C9F8F`, AI1) |
| 0x5CA6A0 | CSWSCreature::FollowState5_Hold (FM6 consumer) | high | `005ca7c6 MOV ECX,[EAX+0x70]`, `005ca7e8 FCOMP [0x996520]` (f64 49.0), `005ca802 MOV [ECX+0x70],0` |
| 0x5CACD0..0x5CB060 / 0x5CB190 | FollowState6..9 wait trail / 10 skip to walkable | med | transitions only |
| 0x7E5180 / 0x7E5610 | CSWParty ctor / slot-element init | high | 3 slots of 0x88 at `P+0x24` |
| 0x7E5730 | CSWParty::AddCharacter(id) | high | first free slot (max 3), queues `0x3D` for slot > 0 (`007e5a69 PUSH 0x3D`, `007e5a6e CALL 0x53F7F0`) |
| 0x7E5C40 | CSWParty::IsPartyMember(clientId) | high | scan `P[slot*0x22+9]` (client; includes the PC, 3 slots) |
| 0x7E5DF0 | CSWParty::SetLeaderBySlot(slot) | med | rotates ids, server `SetLeader`, `k_hen_leadchng`, sets FM6 flags (`0x7E6546/6556/6566`) |
| 0x7E6CE0 | CSWParty::Update(dt) | med | 5 m edges, `TestAndResetPath`, `RecalcFollowPoint`, retreat; gate renamed from ClientParty::RangeMonitor; only caller `0x781BE0` |
| 0x7E8350 | CSWParty::ResetAllFollowData(pos, heading, flag) | med | refills 100-entry ring, all slot states := -1; UI path only (`00797fb9`) |
| 0x7E7D70 | CSWParty::GetFollowRange | med | ranges.2da row 0xE (0xF when table `+0x110 == 1`), default `[0x986B68]` = 5.0 |
| 0x7E8720 / 0x7E8A70 | RecalcFollowPoint / CSWPartyFollowDataArray::RecordLeaderPoint | high / med | slot target = trail point at (range -/+ spacing); ring append from the leader's own walker |
| 0x51C8D0 | CServerExoApp::GetPartyTable | high | inline object at `*(this+4)+0x1F0B4` (`0051c8da MOV EAX,[EAX+0x4]` / `0051c8dd ADD EAX,0x1F0B4`); no deref |
| 0x5FB070 | CSWPartyTable::GetIsNPCObjectMember(objId) | high | gate rename (batch GetIsMember_2); 12 NPC slots `005fb09a CMP EAX,[EDX+ECX*4+0x1C]` then member list `005fb0b8`; PC -> 0 |
| 0x5FAFD0 / 0x5F8D80 / 0x5F9220 | GetIsMember(npcIdx) / AddMember(npcIdx, objId) / CreateParty | high / high / med | AddMember refuses when `*tbl == 2` |
| 0x5FD480 | CSWPartyTable::SpawnNPC | high | `005fd4c9 CALL 0x5FAAF0`; `005fd535 CALL 0x6D3330` joins the PC faction; no flag write; `+0xEC` forced 1 around `005fd558 CALL 0x541080` |
| 0x5FE4B0 | CSWPartyTable::SwitchPlayerCharacter(npcIdx) | high | sole caller cmd 11 `0x694400`; DB MakePlayerCharacter was the wrong class |
| 0x66B770 / 0x66B8A0 / 0x66B640 | cmd 850 ChangeObjectAppearance / 847 CreatureFlourishWeapon / 834 SetCreatureAILevel | high | mapper "WalkUpdateLocation_QuickWalk_FollowLeader_*" names are false |
| 0x669020 | FUN_00669020 (unknown) | low | batch said "RunScriptCallback-like"; gate: not in the command table, no direct callers, not follow |
| 0x599A10 | effect-89 apply handler (creates a creature) | low | `if (*CSWParty < 3 && param_2 != 0)` -> `new(0x1220)` -> `0x58A330(1,1)`; identity open; AI7 "raise/resurrect" unsupported |
| 0x5D4B60 | script SetFormation worker (cmd 729 `0x696CC0`) | med | NPC group offsets from a 2DA row, not the party formation |

## Key behaviours
1. `+0x11AC` = **PC or one of the (max 2) CSWPartyTable members** (PC + selected followers), CONFIRMED. Writers: ctor `0056214C` (0) and `0058A35E MOV [EAX+0x11AC],ECX` only. Equals cmd 576 IsObjectPartyMember (`0068bc2e MOV EDX,[ECX+0x11AC]`). Not serialized (`SaveCreature` restores it from `[EBP-0x38]`). Hawk standees are 0 by design; the 02:38 zeros (atton/t3m4/kreia on the Hawk) are the engine's truth.
2. The 15 `SetPartyMemberFlag` call sites (flag, apply), grouped by the gate:

| group | site | caller | args | condition |
|---|---|---|---|---|
| Set 1 | `0x5F8EB4` | AddMember `0x5F8D80` | (1,1) | `*tbl != 2`, NPC available (`0x5FA960`), creature exists |
| Set 1 | `0x66CA55` | cmd 574 AddPartyMember | (1,1) | only if `FUN_005f8d80(...) != 0` |
| Set 1 | `0x5F93AF` | CreateParty `0x5F9220` | (1,1) | per member after SpawnNPC |
| Set 1 | `0x8A0509` | party-select Accept `0x89FBD0` | (1,1) | after `FUN_005f8d80`, `ClearAllActions(1)`, PathInfo reset |
| Set 1 | `0x599AEF` | effect-89 creature `0x599A10` | (1,1) | `*CSWParty < 3` |
| Set 1 (PC) | `0x5FEAC2` / `0x6306A0` / `0x630B12` | SwitchPlayerCharacter / chargen "TEMP_CHAR" / player-list load | (1,1) | PC creature |
| Clear | `0x6900BC` | cmd 575 RemovePartyMember | (0,1) | `if (iVar5 != 0)` |
| Clear | `0x69B8FC` | cmd 846 RemoveNPCFromPartyToBase | (0,1) | then KillNPCObject |
| Clear | `0x89FE40` | party-select de-select | (0,1) | after client `0x775860(0)` |
| Transient | `0x574532` / `0x57516B` | SaveCreature | (0,1) / (old,1) | every save |
| Transient | `0x5F89A5` / `0x5F8A0E` | TransferInventory `0x5F8960` | (0,0) / (old,0) | `if (param_2 != 0)`, no side effects |

   Never set by: SpawnNPC `0x5FD480`, GetNPCObject `0x5FAAF0`, cmd 698 SpawnAvailableNPC, module templates, GFF reload.
3. **Correct test for our DLLs (gate).** Hot path with a creature pointer: `*(int*)(cr+0x11AC) != 0` (the engine's own test: 289 reads in 200 functions, AIUpdate promotion `005694a9`, RunActions 0x3D). With only an object id: `0x5FB070` covers NPCs only and **returns 0 for the PC** (the PC is never in `tbl[7..18]`), so OR it with "id == PC id (`0x51C8F0`)", or use the client `0x7E5C40` (3 slots, includes the PC). Server table address = `0x51C8D0` with serverApp from `0057562e MOV EAX,[0x00A1B4A4]` / `00575633 MOV ECX,[EAX+0x8]`. "Recruited but not selected" is a different predicate (`tbl[7+idx]` plus availability `0x5FA960`, med). Never use `+0x220` (player update-set flag) or `+0x10E8` (IsPC). Batch said `0x5FB070(objectId)` is the correct test; gate corrected: NPCs only, PC id caveat.
4. **Transient zeros:** the flag is 0 inside SaveCreature between `0x574532` and `0x57516B`, and inside TransferInventory. companion-ai FM4 hooks are safe (snap at `0x57451F` reads before the clear, restore at `0x575183` runs after `0x57516B`); any new hook reached from those spans (SetAILevel, FollowInfo frees) sees 0.
5. **FollowInfo `creature+0x500` (0x3C B) is almost vestigial.** Live field: `+0x38` InSafetyRange (5 m edge, `007e7838`/`007e78ae`). Walk/run-rate readers `0x575DF0`/`0x576100` are dead code (`00575e04 XOR EAX,EAX / JZ`, `00576111`). Other fields keep defaults unless a save carries values (absence claim over the census, med). `0x3D` uses `+0x500 != 0` as an extra gate (`005bf817/005bf81b`).
6. **`0x3D` rules:** returns 3 (node removed) when flag 0, `+0x500 == 0`, `vt+0x94() == 1`, flagged and HP <= 0, `+0x380 == 0`, `(+0x1114 & 2) != 2`, or the leader id equals self (`005bf916`). Batch said "`+0x1114` bit 1 meaning unknown"; gate corrected (AI8): **bit 0x2 = legs/movement**, same bit fails `0x5BC796`, `0x46` and the `0x5C172F` case of MoveToPoint. RunActions gate (`0x5D5C5B..0x5D5D35`): leader id (`0x73F470`) INVALID returns 1; creature type byte 5 and `+0x11AC != 0` before `005d5d35 CALL 0x5BF7F0`. Script `ActionFollowLeader` (cmd 730 `0x669FC0`) needs flag and `+0xEC == 1`. `ClearAllActions` on a follower writes `+0x11B0 := 1` (`0x541210`) and its slot state `-1` (`0054137d`); nothing in the engine re-queues `0x3D` (FM1).
7. **Follow state machine** (`E+4`, `E = P+0x24+s*0x88`): -1 reset (-> 5), 0 place (`0x5C7370`), 1 walk trail, 2 blocked (OnBlocked, -> 10), 3 offset walk (3000 ms timer `E+0x6C`), 4 to trail end, 5 hold/re-plan (`0x5CA6A0`), 6-9 wait for the trail to advance (-> 1/3/4/2), 10 skip to walkable. Loop guard `005c6cd4 CMP [EDX+0x44],0x32 / 005c6ce2 MOV [ECX+4],-1` (> 50 iterations resets the slot). `0x5BCDC0` forces state 5 (`005bd09e`).
8. **Leader switch:** all entry points use `SetLeaderBySlot 0x7E5DF0` (no-op unless count >= 2 and arg non-zero; accepts -3 = PC, -2 = slot 1, 1..count-1): rotates ids, `SetLeader 0x5FE410` (`tbl+0x114`), perception ranges rows 0xC/0xB, `RunScript("k_hen_leadchng" 0x9A2178)` (`007e653b`), sets the FM6 hold flag in all 3 slots. UI path `ChangeCharacter 0x797F30` then `ResetAllFollowData 0x7E8350` (`00797fb9`); script `SetPartyLeader` (cmd 13 `0x696250`), deferred `0x79ED60` and party-select do NOT reset. AI level is never changed by a leader switch (flagged creatures are already 4).
9. **Formation:** trail + leader-local offset drives walking. `RecalcFollowPoint 0x7E8720` sets slot target = trail point at (`GetFollowRange` -/+ `P+0x1C0` spacing) behind the leader; state 0 adds the rotated offset `E+0x38`. Presets at `P+0x1C8+k*0x24`; `ApplyLeaderFormation(0)` only from the ctor, so vanilla always uses preset 0 (slot 1 `(1.5,-0.7,0)`, slot 2 `(-1.5,0.8,0)`, spacing 1.5; read from the exe). `TestAndResetPath` mirrors x and sends slots 1/2 to state 4. `GetWorldFormationPosition 0x7E6960` is used only by party-select. Speed thresholds (16.0 `[0x9946D0]`, 100.0, 225.0) low.
10. **5 m edges / retreat** (`0x7E6CE0`): `d2 < 25.0` (`007e77ab FCOMP [0x989ED0]`); exit -> `k_hen_exit5m`, enter -> `k_hen_enter5m`; retreat from ranges.2da row 0x10 -> `k_hen_retreat`. Edge flags reset to 1 by every SaveCreature, so `exit5m` can re-fire after a save.
11. **FM4 (SaveCreature, gate 1.2).** `0x58A330` body: `*(+0x11AC) = flag; if (apply) { flag==0: free +0x500, SetAILevel(2) if +0x78 != 2; else: alloc FollowInfo if +0x500 == 0, SetAILevel(4) if +0x78 != 4 }`. Old == 0 (non-party): call 1 takes `SetAILevel(2)`, call 2 is a no-op (vanilla). Old == 1 (party): call 1 frees FollowInfo and sets 2, call 2 allocates a default FollowInfo and sets 4. Reach: every save via SaveGIT -> `CSWSArea::SaveCreatures 0x52A600` plus 8 party-table NPC save sites (`0x5F7F30..0x5FA3B0`) and `0x5FE4B0`. Effect: an idle non-party creature is demoted by its next AIUpdate (`0x56A0FE/0x56A111`), one FM3 self-transition per creature per save; script-set levels 3/4 on non-party creatures are lost (level 3 is never re-promoted; ResetCreatureAILevel can restore `+0x7C`).
12. **FM4 patches A/B dependency.** A = `0x574529` imm8 of `6A 01` (01 -> 00); B = `0x575160` imm8 of `6A 01` at `0x57515F` (01 -> 00). **A alone changes nothing for non-party creatures** (call 2 still applies (0,1) -> 2). **B alone is harmful:** call 1 frees FollowInfo, call 2 (old,0) never reallocates, AIUpdate re-promotes to 4 but `+0x500 == 0` makes `0x3D` return 3 (`005bf81b JZ`): follow is dead until the next party change. **A+B together:** both calls are pure flag writes, live FollowInfo kept and saved (save format unchanged, values live). Batch said "a second byte at `0x575160` fixes it"; gate corrected: B only together with A. A is 10 B after shipped hook `0x57451F` (no overlap), B is 35 B before `0x575183`. With A+B, companion-ai FM4 mode 1 is redundant but harmless.
13. **FM6 (gate: consumer CONFIRMED, FM6 partly refuted, low).** The hold flag `E+0x70` is set after every `SetLeaderBySlot` and read only by the state-5 handler `0x5CA6A0`: while 1 and the follower is < 7 m (49.0) from the new leader no re-plan; beyond 7 m it clears the flag and re-plans. Writers: `0x7E6546/6556/6566` (set), `0x5CA802` (clear), `0x7E5610` (init). Residual: slots not in state 5 keep their state/target/counter on the script/deferred/party-select paths; the trail holds old-leader points; bounded by state 6-9 waits and the 50-iteration guard. Batch said FM6 consumer unknown (lesson 277); gate corrected.
14. **SpawnNPC joins the PC faction** (`005fd535`), without the party flag: Hawk standees and non-selected companions judge reputation through the PC faction row (see AI6).
15. **Effect-89 creature** `0x599A10`: creates a flagged creature behind `*CSWParty < 3`; no nwscript EffectSummon* exists. Identity open.

## Struct fields
| class | offset | type | name | conf | evidence |
|---|---|---|---|---|---|
| CSWSCreature | 0x11AC | int | PartyMemberFlag (PC or table member) | high | writers `0056214C`, `0058A35E` |
| CSWSCreature | 0x500 | ptr | pFollowInfo (new(0x3C)) | high | `0058a384`, loader `0057528E`, dtor `0x563AC7` |
| CSWSCreature | 0x11B0 | int | walk state (1 idle, 2 move-to-point, 3 follow, -1 none, 4/5/6 other) | med | `005bf925` (:=3), `0x541210` (:=1) |
| CSWSCreature | 0x11FC | int | PuppetOwnerFlag | med | `0x58A470` |
| CSWSCreature | 0x1114 | u16 | allowed-action mask (bit 0x2 movement); batch med "flags bit1 unknown" | high | gate 1.7, `0059c409`; `005bf89e` |
| CSWSCreature | 0xEC | int | commandable (ClearAllActions no-op unless 1) | high | `005410e9` |
| CFollowInfo | 0x00 / 0x28 / 0x30 / 0x38 | objid / f32 / int / byte | FollowObject / MaxSpeed (dead readers) / Result (2) / InSafetyRange | high | `0057528E`, `007e7838`, `007e78ae` |
| CFollowInfo | 0x04 / 0x10 / 0x1C / 0x2C / 0x34 | vec3 / vec3 / vec3 / byte / int | FollowLocation / LastLeaderPos / LastFollowerPos / StickToPos / TimeElapsed | high/med | GFF loader strings, save only |
| CSWParty | 0x00 / 0x24+s*0x88 | int / slot[3] | memberCount (max 3) / slot elements E | high | `007e51c2`, `007e51db` |
| CSWParty | 0xE4 / 0x16C / 0x1BC / 0x1C0 / 0x1C4 / 0x1C8 / 0x234 | vec3 / vec3 / int / f32 / byte / preset table / f32[3] | slot-1/2 offsets / preset (-10000 init) / spacing / mirror / presets / preset spacing | high | `0x7E6720`, ctor |
| CSWParty | 0x2BC / 0x2DC | f32 / ring | retreat timer / CSWPartyFollowDataArray (100 x 0x28; head `P+0x1280`, count `P+0x1284`) | med / high | `007e89e0`, `007e8970` |
| E (slot) | 0x00 / 0x04 / 0x18 / 0x28 / 0x38 | id / int / vec3 / vec3+node / vec3 | client object id / follow state -1..10 / resolved target / trail point / formation offset | high | `007e5c40`, `0x5C6340` |
| E (slot) | 0x44 / 0x6C / 0x70 | int / int / int | loop counter (guard 0x32) / blocked timer ms / FM6 hold flag | high | `005c6cd4`, `005ca7c6` |
| CSWPartyTable | 0x00 / 0x08 / 0x1C / 0x110 / 0x114 / 0x118 / 0x238 | int / int[2] / objid[12] / int / int / int / int | member count (max 2) / member NPC idx / NPC object ids / follow state / leader idx (-1 PC) / controlled-PC idx / solo mode | high | `0x5F8D80`, `0x5FB070`, `0x5FE3B0` |

## Bugs and candidates
| item | verdict | user impact | fix idea | mod item |
|---|---|---|---|---|
| AI-FM4B: SaveCreature sends non-party creatures to AI level 2 on every save | CONFIRMED | one level churn per creature per save; script-set levels 3/4 lost; party members bounce 4 -> 2 -> 4 with a default FollowInfo | byte patches `0x574529` + `0x575160` (01 -> 00), shipped TOGETHER; B alone kills follow | FM4, FM3 |
| Follower idle after ClearAllActions (nothing re-queues 0x3D) | CONFIRMED | companion stops following until a script re-queues | FM1 hook `CompanionAiBumpClear 0x586F43` (AI1); script re-queue | FM1, E7 |
| FM6 hold flag / stale slot state after non-UI leader change | consumer found; PARTLY REFUTED, severity low | idle followers < 7 m do not re-plan; others keep stale state | none designed; probe P3 | FM6 |
| `exit5m`/`enter5m` edge flags reset by every save | CONFIRMED (code) | `exit5m` can re-fire for a companion > 5 m after a save | none yet; E7 scripts must tolerate it | E7 |
| Hawk standees / non-selected companions have `+0x11AC == 0` | CONFIRMED by design | party-gated logic skips them (IsObjectPartyMember false) | use `+0x11AC`; not a detection bug | party-flag, Mod 8, L1, L2 |
| `0x5FB070` returns 0 for the PC | CONFIRMED | an id-only party test misses the PC | OR with PC id `0x51C8F0` or use `0x7E5C40` | party-flag |
| Mapper names for `0x66B770/0x66B8A0/0x669020`, DB class of `0x5FE4B0` | CORRECTED | none (naming) | name_map rows in GATE_wave2 section 4 | none |

## Hook sites
Gate-checked windows (section 5, boundary OK, no relative branch inside, no inbound target inside):
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| AI5-H1 party-flag census | 0x58A35E | 6 | `89 88 AC 11 00 00` | cold | only runtime writer of `+0x11AC`; handler sees OLD `[EAX+0x11AC]`, new in ECX, apply `[EBP+0xC]`, return `[EBP+4]`; expect transient (0,1)/(old,1) pairs per save from `0x574537`/`0x575170` (gate) |
| FM4-A | 0x574529 | 1 | `01` -> `00` | cold | imm8 of `6A 01` at `0x574528`; simple patch; 10 B from shipped `0x57451F` |
| FM4-B | 0x575160 | 1 | `01` -> `00` | cold | imm8 of `6A 01` at `0x57515F`; ship WITH A; 35 B before shipped `0x575183` |

Batch-only (not gate-checked), all log-only, copied from `.asm` by the batch; none within 12 B of the sibling list:
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| AI5-H2 FM6 leader-change | 0x7E6546 | 10 | `C7 80 94 00 00 00 01 00 00 00` | cold | after `RunScript(k_hen_leadchng)`; EAX = CSWParty |
| AI5-H3 FM6 consumer | 0x5CA7C3 | 6 | `8B 45 08 8B 48 70` | warm | state-5 handler; `[EBP+8]` = E |
| AI5-H4a / H4b exit5m / enter5m | 0x7E7838 / 0x7E78AE | 7 / 7 | `C7 42 38 00 00 00 00` / `C7 42 38 01 00 00 00` | cold | EDX = FollowInfo, creature `[EBP-0x38]` |
| AI5-H5 follow engaged | 0x5BF925 | 10 | `C7 80 B0 11 00 00 03 00 00 00` | hot | per follower per AI tick |
| AI5-H6 follow driver entry | 0x5C6340 | 9 | `55 8B EC 81 EC 88 02 00 00` | hot | ECX = creature |
| AI5-H7 stuck guard | 0x5C6CD4 | 7 | `83 7A 44 32 0F 9F C0` | warm | stolen CMP + SETG feed the next TEST: do not clobber flags |
| AI5-H8 follow-radius re-plan | 0x5BD09E | 7 | `C7 42 04 05 00 00 00` | cold-warm | EDX = E |
| AI5-H9 / H10 | 0x7E5DF0 / 0x7E8350 | 10 / 9 | `55 8B EC 6A FF 68 21 B8 96 00` / `55 8B EC 83 EC 3C 89 4D C4` | cold | SetLeaderBySlot / ResetAllFollowData entries |
| AI5-H11 state dispatch | 0x5C6ACE | 7 | `FF 24 8D 34 73 5C 00` | hot | last stolen insn is an indirect JMP |

## Open items
- Identity of effect type 89 (`0x599A10`, vtable entry `0x5943A9`, no direct callers) and of effect-table slot 15 (`0x59ACC0`, conditional `+0xEC := 0`).
- `0x669020` identity (not follow, not a command).
- ranges.2da rows 0xB-0x10 (perception, follow range 0xE/0xF, retreat R,T row 0x10): 2DA not on disk (probe P7).
- States 1-4 internals (`0x5C7AD0`, `0x5C87E0`, `0x5C8C20`, `0x5CA000`), `SmoothFollowPath 0x7E9CC0`, `RecordLeaderPoint` body (4.7 KB), `E+0x80` meaning not read.
- Whether `CClientExoAppInternal+0x338` (deferred leader id) is written only by `0x798660`.
- Wiring decision (user): ship FM4 patches A+B together, or keep FM4 mode 1 and accept the non-party churn (the effect is small).
- Runtime probes proposed by the batch (all observation-only; settle the open items above):

| id | what | hook | expected signal |
|---|---|---|---|
| P1 | who writes `+0x11AC` | AI5-H1 `0x58A35E`: creature id, old, new, apply, return `[EBP+4]` | return addresses from the 15 sites; a Hawk standee never logs a 1; a selected follower that reads 0 shows which site wrote it |
| P2 | follower state transitions | AI5-H7 `0x5C6CD4` + AI5-H8 `0x5BD09E` | rare; confirms the 50-iteration guard and follow-radius re-plans (E4/E7) |
| P3 | FM6 flag lifetime | AI5-H2 `0x7E6546` + AI5-H3 `0x5CA7C3`: slot ids, `E+4`, `E+0x70`, dist^2 | after a script `SetPartyLeader`: idle followers < 7 m show `E+0x70 == 1` and no re-plan; beyond 7 m the flag clears |
| P4 | 5 m edges | AI5-H4a/H4b: creature, id, d2 | one exit per departure, one enter per return; a pair right after a save (FM4) |
| P5 | save side effects | log `+0x78` and `+0x500` before `0x574532` and after `0x57516B` | non-party `+0x78` becomes 2 after every save; party FollowInfo pointer changes |
| P6 | follow engaged | AI5-H5 `0x5BF925` (hot) | ticks where `0x3D` keeps the follower in follow mode; 0 after `ClearAllActions` until re-queued |
| P7 | follow range / 2DA | log `0x7E7D70` return and rows 0xE/0xF/0x10 | settles PrimaryRange / retreat numbers |

## Mod relevance
| item | served by |
|---|---|
| party-flag (`+0x11AC`) | key behaviours 1-4: PC + selected followers, correct test, PC-id caveat, transient zeros, census window `0x58A35E` |
| FM4 / AI-FM4B | key behaviours 11-12; patches A+B together; lessons 3 (non-party level 2) |
| FM6 | key behaviour 13; consumer `0x5CA6A0`, 7 m rule, low residual |
| FM1 | `0x3D` never re-queued by the engine after ClearAllActions; follow driver gate |
| FM3 / AIQ | level churn only from SetPartyMemberFlag, AIUpdate promotion, FM7 drop, SaveCreature |
| E7 (5 m hooks), E4 | `exit5m`/`enter5m`/`retreat` edges from `0x7E6CE0`; state machine loop guard `0x5C6CD4`; follow reports action 38 (AI7) |
| Mod 8, L1, L2 | script party gating = IsObjectPartyMember = `+0x11AC`, selected followers only |
| perf | `0x5C6340`, `0x7E8A70`, `0x7E6CE0` run per follower per tick/frame |
