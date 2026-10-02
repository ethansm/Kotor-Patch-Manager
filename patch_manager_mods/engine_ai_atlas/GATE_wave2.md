# GATE wave 2 (AI5-AI8), Opus verification, 2026-10-02

## Summary (8 lines)
1. Party flag: `+0x11AC` = "PC or one of the (max 2) CSWPartyTable members" (PC + selected followers), CONFIRMED. companion-ai's `+0x11AC` test is the right "active party" test; Hawk standees are 0 by design. AI5's server path (`0x51C8D0` -> inline table `*(serverApp+4)+0x1F0B4`, `0x5FB070`) is CORRECT, but it covers NPCs only: it returns 0 for the PC.
2. FM4 extension CONFIRMED. Every SaveCreature call sends a non-party creature to AI level 2 (vanilla, and also with the known fix at `0x574529`). Patch B (`0x575160` 01->00) is right, but it must ship WITH patch A. B alone leaves party members with no FollowInfo after a save, and follow (0x3D) dies.
3. AdjustReputation: BUG (latent). Get reads `data[b*N+a-1]` and Set writes `data[row*N+col-1]`, both called with (own, p2). So the new value = M(T->S) + delta, written to the S->T cell. Only script cmd 209 reaches it, only for NPC-faction targets: targets in the PC faction (PC, and companions, which SpawnNPC puts in the PC faction) are not affected. No vanilla reach.
4. Medpac contradiction settled for AI7. `0x6F2A30` = IsForceUserClass {3,4,5,11..16}, i.e. exactly the classes.2da rows with `forcedie` > 0 (Jedi Guardian/Consular/Sentinel, Weapon Master, Jedi Master, Watchman, Sith Marauder/Lord/Assassin). Kreia (class 4 only), Visas (5 only) and a Jedi-only PC get FALSE from `GetCreatureHasTalent(medpac)`, so the R4 guard never fires for them.
5. +0xEC is CONFIRMED with one REFUTATION. ClearAllActions is a full no-op unless `+0xEC == 1`. But ActionMoveToLocation / ActionForceMoveToLocation (`0x68CEE0`) are NOT gated: they still queue a move on a non-commandable creature. Also, AI8's `+0xEC := 0` writer `0x596A7A` sits in dead code.
6. GetCurrentAction: an engine attack approach reports 3 (0xC group) or 39 (0x3F round group), never MOVETOPOINT. A follower reports 38. So an E4 key on MOVETOPOINT sees only script moves. `nCRMax` is dead (CONFIRMED). AI7's exclusion reading is REFUTED: (0,-1) still runs the spell group once, and (1,-1) marks the spell group done instead of the feat group.
7. Shout E9: SILENT_TALK reaches a listener within 35 m only if its perception entry has the HEARD bit and it either sees the shouter or is < 10 m away. The engine applies no faction filter. A stale id makes the shout fail silently, but reach is LOW (the same failure would blind perception and GetNearestCreature). Delivery to a companion is therefore conditional, not reliable; each round retries it.
8. Results: 7 contradictions resolved. DB: 15 corrections, 34 new rows, 12 lessons, 5 candidates. Probe windows: all 8 are OK (boundary, bytes = exe, no relative branch inside, no inbound target inside, jump tables scanned). No overlap except AI8-H10 = perf-lab `0x544210`.

Method: `.asm`/`.c` from `ROOT/decomp`; constants and tables read from `backups/vanilla_clean/swkotor2.exe` with a private PE reader. The command table was re-derived independently of AI6/AI7's scratch files: 774 `MOV [reg+off],imm` stores in `0x665F50`, every instruction byte-compared with the exe (0 mismatches), and index = off/4 joined to the 877 `nwscript.nss` prototypes. classes.2da, spells.2da and the companion .utc files were read from workspace copies (`replay/pre_checkpoint_2da`, `replay/scratch/override`); the row ids are vanilla. Window checker: 212,133 direct branch rows from every `.asm`, plus each containing function's jump tables read from the exe. The lead's line checks were not redone.

## 1. Verdict table

### AI5 Follow, party and formation
| claim | verdict | evidence |
|---|---|---|
| `+0x11AC` = in the active party (PC + selected followers) | **CONFIRMED** | See 1.1 |
| Correct DLL test / `0x5FB070` path | **CONFIRMED with a caveat** | See 1.1. `0051c8da MOV EAX,[EAX+0x4]` / `0051c8dd ADD EAX,0x1f0b4` (inline object, no deref); callers get serverApp from `0057562e MOV EAX,[0x00a1b4a4]` / `00575633 MOV ECX,[EAX+0x8]` / `0057563c CALL 0x0051c8d0`. `0x5FB070`: `005fb09a CMP EAX,[EDX+ECX*0x4+0x1c]` over 12 NPC slots (`005fb08b CMP [EBP-0x4],0xc`), then the member list `005fb0b8 CMP EAX,[EDX]`. **The PC is never in `tbl[7..18]`, so `0x5FB070(pcId)` = 0.** |
| FM4: SaveCreature sends non-party creatures to level 2 on every save; patch B | **CONFIRMED; B only with A** | See 1.2 |
| `0x5FE4B0` = CSWPartyTable::SwitchPlayerCharacter | **CONFIRMED** (DB wrong class) | sole caller cmd 11 SwitchPlayerCharacter `0x694400`: `00694436 CALL 0x0051c8d0` / `0069443b MOV ECX,EAX` / `0069443d CALL 0x005fe4b0`; body indexes `param_1[0x46]` (`+0x118`) and `param_1[param_1[0x46]+7]` |
| `0x66B770` = cmd 850, `0x66B8A0` = cmd 847 | **CONFIRMED** | gate table: idx 850 ChangeObjectAppearance -> `0x66B770`, idx 847 CreatureFlourishWeapon -> `0x66B8A0`, idx 834 SetCreatureAILevel -> `0x66B640` |
| `0x669020` "RunScriptCallback-like" (mapper: FollowLeader_Moving_To_Path) | **UNRESOLVED**, follow name false | not in the command table (no store of `0x669020`), no direct callers, callees `0x51C280/0x733B60/0x669070` |
| FM6 consumer = state-5 handler `0x5CA6A0`, 7 m (49.0) | CONFIRMED (lead lines + constant) | `[0x996520]` f64 = 49.0 (exe) |
| 5 m edges at 25.0 | CONFIRMED | `[0x989ED0]` f64 = 25.0 (exe); agrees with lesson 279 |
| `+0x1114 & 2` (FollowLeader gate) "meaning unknown" | **RESOLVED by AI8**: bit 0x2 = legs/movement | AI8 Q1 table; same bit fails 0xA `0x5BC796`, 0x46, and the 0x5C172F case of MoveToPoint |
| `0x5FD480` = SpawnNPC (DB: AwardXPToPartyMember?) | **CONFIRMED, plus a new fact** | `005fd4c9 CALL 0x005faaf0` (GetNPCObject, create); `005fd52e CALL 0x005d0cb0` (PC's faction) / `005fd535 CALL 0x006d3330` (AddMember npc); `+0xEC` forced to 1 around `005fd558 CALL 0x00541080`. **Spawned NPCs join the PC's faction without the party flag.** |
| effect-89 creature (`0x599A10`) | CONFIRMED code, identity open | `if ((*CSWParty < 3) && ... param_2 != 0)` -> `new(0x1220)` -> `FUN_0058a330(1,1)`; there is no nwscript EffectSummon* (grep). AI7's "raise/resurrect" label is not supported. |

#### 1.1 Party flag (decides companion-ai's test)
Writers (lead census): ctor `0056214c` (0) and `0058a35e MOV [EAX+0x11ac],ECX` only. The 15 call sites of `SetPartyMemberFlag 0x58A330(flag, apply)`, summarized; the gate re-read sites 6, 7, 3, 11, 12, 14 and 15:
- **Set 1** (5 sites):
  - AddMember `0x5F8EB4`: only if `*tbl != 2` (max 2 members), the NPC is available (`0x5FA960`) and the creature exists.
  - cmd 574 AddPartyMember `0x66CA55`: only `if (local_10 = FUN_005f8d80(...)) != 0`.
  - CreateParty `0x5F93AF`: per member after SpawnNPC.
  - party-select Accept `0x8A0509`: after `FUN_005f8d80`, `ClearAllActions(1)` and PathInfo reset.
  - Effect-89 creature `0x599AEF` (party < 3).
- **Set 1 for the PC** (3 sites): SwitchPlayerCharacter `0x5FEAC2`, chargen `0x6306A0`, player-list load `0x630B12`.
- **Clear with apply** (3 sites): cmd 575 RemovePartyMember `0x6900BC` (`if (iVar5 != 0) FUN_0058a330(0,1)`), cmd 846 RemoveNPCFromPartyToBase `0x69B8FC`, party-select de-select `0x89FE40` (after client `0x775860(0)`).
- **Transient** (4 sites):
  - SaveCreature `0x574532` (0,1) / `0x57516B` (old,1).
  - TransferInventory `0x5F89A5` (0,0) / `0x5F8A0E` (old,0).

Never set: SpawnNPC (`0x5FD480`), GetNPCObject (`0x5FAAF0`), cmd 698 SpawnAvailableNPC, module templates, GFF reload. So **flag = 1 iff PC or a current party-table member** (plus the effect-89 creature). It equals cmd 576 IsObjectPartyMember (`0068bc2e MOV EDX,[ECX+0x11ac]`).

**Correct test for our DLLs:**
- Hot path, creature pointer in hand (companion-ai today): `*(int*)(cr+0x11AC) != 0`. This is the engine's own test, used by 200 functions, AIUpdate promotion and RunActions 0x3D.
- Only an object id in hand: `0x5FB070` for NPCs, OR'd with "id == PC id (`0x51C8F0`)". Or use client `0x7E5C40` (3 slots, includes the PC). **[Superseded, see Correction (REVIEW_synthesis 2026-10-02) at the end of this file: `0x51C8F0` is the controlled leader.]**
- "Recruited but not selected" is a different predicate: `tbl[7+idx]` object ids plus availability `0x5FA960` (med).
- Caveat: inside SaveCreature between `0x574532` and `0x57516B`, and inside TransferInventory, the flag is transiently 0. companion-ai's FM4 hooks are safe: the snap at `0x57451F` reads the flag before the clear, and the restore at `0x575183` runs after `0x57516B`. Any new hook reached from those spans (SetAILevel, FollowInfo frees) sees 0.
- Never use `+0x220` or `+0x10E8` as the party test.

#### 1.2 FM4 second byte (AI5 section 8)
- `0x58A330` body: `*(param_1+0x11ac) = param_2; if (param_3 != 0) { if (param_2 == 0) { free +0x500; if (+0x78 != 2) SetAILevel(2) } else { if (+0x500 == 0) new(0x3C) + 0x5752C0; if (+0x78 != 4) SetAILevel(4) } }`. Anchors: `0058a364 CMP [EBP+0xc],0` / `0058a372 JZ` / `0058a448 PUSH 0x2` ... `0058a458 CALL 0x0051e120`; `0058a384 PUSH 0x3c` / `0058a3a1 CALL 0x005752c0`.
- SaveCreature: `0057451f MOV ECX,[EAX+0x11ac]` / `00574525 MOV [EBP-0x38],ECX` / `00574528 PUSH 0x1` / `0057452a PUSH 0x0` / `00574532 CALL 0x0058a330`; GFF written (`0057515a CALL 0x00719bf0`); `0057515f PUSH 0x1` / `00575161 MOV EAX,[EBP-0x38]` / `00575164 PUSH EAX` / `0057516b CALL 0x0058a330`; then `0057517e CALL 0x005751b0` (SaveFollowInfo).
- **Under which flag value:**
  - old == 0 (non-party): call 1 takes `SetAILevel(2)` (unless already 2), call 2 is a no-op. Vanilla effect.
  - old == 1 (party): call 1 frees FollowInfo and sets 2, call 2 allocates a new default FollowInfo and sets 4.
- **Reach:** every save goes through SaveGIT -> `CSWSArea::SaveCreatures 0x52A600`, plus the 8 party-table NPC save sites (`0x5F7F30`..`0x5FA3B0`) and `0x5FE4B0`.
- **Effect:**
  - A non-party creature that is idle (`+0x520 != 1`) is demoted back to 0/1 by its next AIUpdate (`0x56A0FE/0x56A111`). That costs one FM3 self-transition per creature per save.
  - Script-set levels 3/4 (SetCreatureAILevel, StartConversation) on non-party creatures are lost. Level 3 is never re-promoted automatically. ResetCreatureAILevel can still restore the level saved at `+0x7C`.
  - Patch A alone (`0x574529`) changes nothing for non-party creatures: call 2 still applies (0,1) -> 2.
- **Patch B (`0x575160`, imm8 of `6A 01` at `0x57515F` -> `6A 00`) is correct only together with A.** With A+B both calls are pure flag writes; the live FollowInfo is kept and saved (save format unchanged, values live).
- **B without A is harmful:** call 1 frees FollowInfo, and call 2 (old,0) never reallocates it. AIUpdate re-promotes the member to level 4 (`0x5694D6`), but `+0x500 == 0` makes `0x3D` return 3 (`005bf81b JZ`). Follow is dead until the next party change.
- Distance from the shipped hooks: A is 10 B after `0x57451F` (6 B hook, no overlap); B is 35 B before `0x575183`. With A+B, companion-ai FM4 mode 1 becomes redundant (it would copy the same struct onto itself). Harmless.

### AI6 Faction, reputation, shouts
| claim | verdict | evidence |
|---|---|---|
| AdjustReputation transposition | **BUG (latent)** | See 1.3 |
| Reputation precedence (`0x5E13C0`), thresholds `<=10` / `>=90`, override `[0xA0F6E0]` | CONFIRMED | `[0xA0F6E0]` = 0xFFFFFFFF in the exe; `0x5D0D00`: both flagged -> 100, a flagged side's faction -> 0, then `FUN_00664580(local_10 /*S*/, local_8 /*T*/)` = cell (row T, col S) |
| "a flag-0 companion is judged by its OWN faction row" | **CORRECTED** | SpawnNPC adds the NPC to the PC's faction (`005fd535 CALL 0x006d3330`). Standees use the PC faction's row anyway; the remap only matters between two flagged members (100) |
| Shout radii TALK 1000 / WHISPER 3 / SHOUT 250 / SILENT_TALK 35 (+seen rule) / SILENT_SHOUT 1000 | CONFIRMED (exe floats) | `0x992F04`=250.0, `0x992F08`=1000.0, `0x992F0C`=3.0, `0x992F18`=35.0, `0x992F1C`=1000.0; pushes `0063ab21 PUSH 0x1` (SILENT_TALK) vs `PUSH 0x0` others |
| No faction filter; HEARD bit required; flag-1 needs SEEN or dist² < 100 | CONFIRMED | `0054734c MOV DL,[ECX+0x4]` / `0054734f SHR DL,1` / `00547351 AND DL,1` / `00547359 JNZ` else reject; `00547364 CMP [EBP+0x10],0` ... `00547370 AND DL,1` (SEEN) ... `0054739c FCOMP [0x0098ba38]` (100.0) / `005473a7 JNZ` keep |
| Payload int1 = pattern number -> `+0x1BC` | CONFIRMED (AI7 "volume" wrong) | `0054747c MOV [EBP-0x58],EAX` (TestListenExpression result) ... `00547511 MOV ECX,[EBP-0x58]` / `00547515 PUSH 0x1` / `0054751a CALL 0x005e1bd0` (SetInteger(1, pattern)); `005474d4 MOV ECX,0x7` / `005474dc MOV word [EDX],CX` |
| Shout abort (`0x522AF0` return ignored, unresolvable id aborts) | CONFIRMED code, reach LOW | See 1.4 |
| Listen patterns persisted | CONFIRMED (new) | `CSWSObject::LoadListenData 0x5409F0` calls `SetListenExpression` (`00540b06`); "Listening" in Save/LoadCreature |

#### 1.3 AdjustReputation (decides AI-REPTRANSPOSE)
- `0x664580` Get(a,b): `006645ef MOV ECX,[EBP+0xc]` / `006645f2 IMUL ECX,[EAX+0x1c]` (b*N) / `00664602 LEA ECX,[EAX+EDX*1-1]` (+a-1). So it reads `data[b*N + a - 1]`. If only b is an NPC faction, it recurses Get(b,a) (`006645e5`).
- `0x6644D0` Set(row,col,v): `00664531 MOV ECX,[EBP+0x8]` / `00664534 IMUL ECX,[EAX+0x1c]` (row*N) / `00664544 LEA ECX,[EAX+EDX*1-1]` (+col-1). So it writes `data[row*N + col - 1]`. It requires 0 <= row <= N and 1 <= col <= N.
- `0x5D0DF0` (this = oTarget T, p2 = faction of oSource S, from cmd 209 `0x66D1B0`): `005d0e40 MOV EDX,[EBP+0x8]` / `005d0e43 PUSH EDX` / `005d0e47 PUSH EAX (own)` / `005d0e4b CALL 0x664580`, i.e. Get(fT, fS) = cell (row fS, col fT). Then `005d0e88 ADD ECX,[EBP+0xc]` / `005d0e8b PUSH ECX` / `005d0e8f PUSH EDX (p2)` / `005d0e93 PUSH EAX (own)` / `005d0e97 CALL 0x6644d0`, i.e. Set(fT, fS) = cell (row fT, col fS).
- Orientation (from `0x5D0D00`): rep(S->T) = cell (row fT, col fS). So the written cell is the intended "S's faction feels about T" (nwscript 209), but the value = rep(T->S) + delta. **BUG** for asymmetric pairs, e.g. PREDATOR 11 / PREY 12, GIZKA_1/2.
- **Not reached** when fT is the PC faction (0): Get(0,fS) recurses to Get(fS,0) = `data[fS-1]` = Set(0,fS)'s cell. That covers the PC and every SpawnNPC companion. Skipped when fT == fS (`005d0e6b`), or when fS is not an NPC faction (`0x664650`).
- User-visible effect: none in vanilla. The 638-file source corpus has only `unused_sandper.nss` (`AdjustReputation(oParty1, oTuskan, ±50)`; oTarget = party leader, PC faction, so unaffected). Module-embedded NCS were not scanned.
- Fix: swap the two pushes at `0x5D0E40` (8 bytes `8B 55 08 52 8B 45 E8 50` -> `8B 45 E8 50 8B 55 08 52`). Then Get(fS, fT) = cell (row fT, col fS) = the written cell. Low priority.

#### 1.4 Shout path and E9 (k_ai_master answering allied shout 15)
- Chain: `SpeakString(…, TALKVOLUME_SILENT_TALK)` (k_ai_master EndRound, about every 3 s per combatant) -> `0x63A5B0` -> `0x547100(speaker, &str, 35.0, 1)`.
- For each object in `area+0x194` within x ± 35, the listener is accepted when:
  - 3D dist² <= 35² (`005472b6`);
  - `+0x1C0` listening (`005472c6`);
  - both are creatures: a perception entry for the speaker with the HEARD bit, and SEEN or dist² < 100;
  - pattern 15 is matched (`0x546F10`).
- Then event 10 / payload type 7 is posted with delay 0 -> `0x57074F` -> slot 7 (`k_hen_<name>dlg`).
- Script side: all 13 companion dlg scripts call `GN_RespondToShout` under `GetCommandable()`. Our installed build routes them to k_ai_master 2004 (lesson 261). Companions get listening and patterns from `k_oei_spawn` (`GN_SetListeningPatterns`; Visas uses `m_spn_visas`, not in the corpus, open). Patterns survive saves (`0x5409F0`).
- Abort reach:
  - `0054712f MOV [EBP-0x1c],-1` / `005471ac CALL 0x00522af0` / `005471b1 CMP [EBP-0x1c],0` / `005471b5 JL 0x5475e8`: zero listeners when the X-bisection meets an unresolvable id.
  - `00547203 JNZ 0x5475e8`: the rest of the scan is dropped.
  - Both need a stale id in `area+0x194`. The same `0x522AF0` serves UpdatePerception modes 0/2, GetNearestCreature `0x683D40`, GetFirstObjectInShape `0x685960` and Surrender, so a stale id would also blind perception and scripts in that area. Nothing like that has been observed: LOW.
- **E9 verdict:** an ally's shout reaches a companion only if:
  - the companion is within 35 m (3D);
  - its own perception list has the ally with HEARD (party perception refresh about 4 s, lesson 277);
  - it sees the ally or is within 10 m;
  - it is commandable (script).
  
  Shouts repeat every round, so one miss is retried after about 3 s. Not reliable for a companion at the far edge of the formation, or one that does not hear the ally. Enemies within range receive the same shout; the script filter (`GetIsFriend`) is the only faction gate. The R2 E9 design (seen enemy, idle, 25/15 m) is consistent with this.

### AI7 Script <-> engine
| claim | verdict | evidence |
|---|---|---|
| Command table = nwscript order, 774 handlers | **CONFIRMED (re-derived)** | 774 stores, 0 byte mismatches vs exe; prototypes 877 = 0x36D. The 8 key commands: GetNearestCreature 38 `0x683D40`, GetCreatureHasTalent 306 `0x67ACC0`, GetCreatureTalentBest 308 / Random 307 `0x67AE40`, ActionAttack 37 `0x66DAF0`, GetCurrentAction 522 `0x67B100`, GetHasSpellEffect 304 `0x67E670`, IsObjectPartyMember 576 `0x68BBE0`, ActionFollowLeader 730 `0x669FC0`. Also verified: 9 ClearAllActions `0x66F330`, 21/382 `0x68CEE0`, 22/383 `0x68CB50`, 140 GetIsDead `0x68B4B0`, 162/163 `0x690F40/0x67AC20`, 195 `0x683740`, 208/209 `0x687E00/0x66D1B0`, 221/39 `0x693860`, 235-237 `0x67C430`, 309 `0x66BDC0`, 732 `0x696D80` |
| Medpac: `0x6F2A30` gate | **CONFIRMED (contradiction with R4 / lesson 280)** | See 1.5 |
| GetCurrentAction = last reportable node of the head group; approach != MOVETOPOINT | **CONFIRMED + refined** | See 1.6 |
| `nCRMax` dead | **CONFIRMED** | the pickers compare `[EBP+0xc]` only with -1: `006bb0ca` (0x6BAEC0), `006bc23e` (0x6BC0E0), `006bb707`/`006bbc46` (0x6BB3A0), `006bd4f7` (0x6BCFA0); it is forwarded as arg 4 of `0x6BA970` (`006bb6c5`, `006bd4d4`), and `0x6BA970` never reads `[EBP+0x14]` (grep: none) |
| `nExcludeId` not applied to spells | CONFIRMED | `006bb07e PUSH -0x1` (arg 8) / `006bb080 MOV EDX,[EBP+0x24]` (arg 7); `006ba97c CMP EAX,[EBP+0x24]` compares the spell with arg 8 = -1 |
| "(0,-1) drops the spell group, (1,-1) drops the feat group" | **REFUTED** | spell coin: `006b8c59 CMP [EBP+0x30],-1` / `006b8c5f MOV [EBP-0x10],1` (spell group marked done) and then the pickers still run with exclusion -1 (`006b8ca4 CALL 0x006baec0`). Feat coin: `006b8d29 CMP [EBP+0x30],-1` / `006b8d2f MOV [EBP-0x10],1`, which marks the SPELL group done, not the feat group (`[EBP-0x14]`). Our scripts never pass exclusions (grep: 0 uses) |
| Item picker `0x6BB3A0` repository pass skips HasTalent | CONFIRMED (equip pass does call it) | equip-slot pass `006bb6db CALL 0x006ba970` (HasTalent inside); repository pass only `FUN_006bdd60(row+0x130, cat)` |
| Event table / slots (Q2) | CONFIRMED (spot) | slot-6 sites agree with GATE_wave1; slot 7 stores the pattern number (not the volume) at `+0x1BC` |
| `0x599A10` "raise/resurrect" (slot-8 site) | low | it creates a new creature (`new(0x1220)`), see the AI5 table |

#### 1.5 Medpac / Force class (decides whether the R4 rule works for all companions)
- `0x6F2A30` asm: `006f2a34 CMP [EBP+0x8],0x3` ... `006f2a64 CMP [EBP+0x8],0xc`, then 1 for {3,4,5,0x10,0xF,0xE,0xD,0xB,0xC}, else 0.
- classes.2da rows: 0 Soldier, 1 Scout, 2 Scoundrel, **3 JediGuardian, 4 JediConsular, 5 JediSentinel**, 6 CombatDroid, 7 ExpertDroid, 8 Minion, 9 TechSpecialist, 10 BountyHunter(cut), **11 JediWeaponmaster, 12 JediMaster, 13 JediWatchman, 14 SithMarauder, 15 SithLord, 16 SithAssassin**. The true set is exactly the rows with `forcedie` > 0.
- Other callers fit: GetMaxForcePoints `0x57EDD6`, AddGrantedPowers `0x847ACE`, AutoLevelup.
- Name: **IsForceUserClass**.
- `HasTalent 0x6B8840` (type 0) loops the class slots:
  - Force class: only `0x6BD9C0 && 0x6B8F40` (known spell and cost payable).
  - Non-Force class: innate -> known -> `006b8a6c CALL 0x006c0330` (item scan).
  
  The result is true if any slot succeeds, AND mask bit 8.
- Companion class lists (UTC): Kreia [(4,3)], Visas [(5,6)], Handmaiden [(0,…)], Disciple [(0,…)], Atton [(2,…)], Bao-Dur [(9,…)], Mira [(1,…)], Hanharr [(1,…)], HK-47 [(6,…)], T3-M4 [(7,…)], G0-T0 [(7,…)]. Level-ups only add Jedi or prestige classes, so a non-Force base slot persists.
- Category 0x1408 = only the items `ITEM_ABILITY_MED_PACK` 64, `ADVANCED_MED_PACK` 65, `LIFE_SUPPORT_KIT` 66, `RECOVERY_STIM` 129 (spells.2da). Force Heal is 0x2441.
- **Result:**
  - `tSpe11_05 = GetCreatureTalentBest(0x1408,20)` returns a medpac talent for everyone with stock: the repository pass has no class test.
  - `GetCreatureHasTalent(tSpe11_05)` is FALSE for Kreia, Visas and a Jedi-only PC (when AI-driven), even with medpacs.
  - The R4 rule (and vanilla NPC medpac use) never fires for them. Failure is safe (no idle loop), but they never use medpacs.
  - It works for everyone else, droids included (repair kits).
- Script fix: for party members use `GetIsTalentValid(tSpe11_05)` + `GN_PartyMedpacStock() >= 1` instead of GetCreatureHasTalent. `ActionUseTalentOnObject` with the item fields goes straight to UseItem (`0x57D8B0` -> `0x57B4C0`), so it needs no HasTalent.

#### 1.6 GetCurrentAction (decides E4)
- `0067b109 MOV [EBP-0x8],0xffff`. Head node `0067b188 CALL 0x521060`; the group is `0067b1a4 MOV AX,[EDX+0x6c]`.
- The loop `0067b1bd CMP EDX,EAX` / `0067b1bf JNZ` (stops at a new group) / `0067b1ca CALL 0x5d7190` / `0067b1df CALL 0x5d7220` / `0067b1e7 MOV [EBP-0x8],EDX` (overwrites per reportable node) / `0067b1f9 JMP 0x67b1ac`. Empty list -> `0067b1fd MOV [EBP-0x8],0xfffe`.
- Reportable set (`0x5D7190` switch) includes 1, 0xC, 0x3D (->0x26) and 0x3F (->0x27); it excludes 0x11, 0x13, 0x40.
- Groups:
  - **0xC approach:** `006d871c MOVZX EDX,word [ECX+0x6c]` / `006d8721 PUSH 0xc` / `006d8729 CALL 0x00540ca0`, so [move, 0x13, 0x11, 0xC] share one group -> **3**.
  - **0x3F round:** `006da0b5 MOVZX EAX,word [EDX+0x6c]` / `006da0ba PUSH 0x11` / `006da0c2 CALL 0x00540ca0`, so the approach shares the 0x3F group -> **39 (0x27)**. AI7's "an attack round reports ATTACKOBJECT" is wrong here.
  - **Follow:** state 0 `0x5C7370` queues 0x40 + move in the 0x3D node's group (`local_38 = FUN_005d5120(0x3d); local_8 = *(ushort*)(local_38+0x6c)`) -> **38**.
- **E4 consequence:** `== ACTION_MOVETOPOINT` holds only for script moves (ActionMoveToLocation, ActionMoveToObject chain [1,0x11,0x13]) and other script-built groups. An engine approach (FM2/FM2b) or a follow walk never trips it. Key E4 on "no position progress while the action is in {0, 3, 39}", or on the 0x11 re-approach count (GATE_wave1 1.1).

### AI8 Effects and AoE gates
| claim | verdict | evidence |
|---|---|---|
| `+0xFF5/+0xFF4` do not gate AIUpdate/RunActions | CONFIRMED (absence, med) | census per AI8 |
| SetState map, precedence `JLE` at `0x59D95A`, slot-6 set | CONFIRMED (agrees with GATE_wave1) | `0x59E860` switch: cases 1, 3-10, 0xE-0x13 -> `FUN_006fd8d0(iVar1+0x2a0,…)`; 0xC/0xD only `0x57A990`; 2/0xB nothing |
| ClearAllActions is a no-op while `+0xEC != 1` | **CONFIRMED** | `0x541080`: the whole body (queue clear, `+0x11B0 := 1`, `+0x54C`, PI reset, `0x5908F0`, party slot `-1`) is inside `if (param_1[0x3b] == 1)` (`005410e9 CMP [EAX+0xec],1` / `005410f0 JNZ 0x541385` = function end) |
| "script Action* silently dropped while non-commandable" | **PARTLY REFUTED** | See 1.7 |
| `+0xEC` writer `0x596A7A` (OnApplyDamage) | **CORRECTED: dead code** | `005969cb XOR EDX,EDX` / `005969cd JZ 0x00596b74`; every branch into `0x5969D3..0x596B74` comes from inside that block (`005969ec`, `00596a0b`, `00596a26`, `00596a3b`, `00596a59`) |
| GetHasSpellEffect = `effect+0x1C == spell`, stamped from the creator's `+0xFF8` | **CONFIRMED** | `0x5E5EA0`: `*(param_1+0x1c) = (**(code**)(*piVar3+0xb8))()` (via `vt+0x10`; an AOE creator is replaced by `+0x288`); EventHandler `0056fc5e MOV EDX,[EAX+0xbc]` / `0056fc64 CALL EDX` (set spell id) / `0056fce2 CALL 0x006fd8d0` (spell script) / `0056fce7 PUSH -0x1` / `0056fcfd CALL EAX` (reset) |
| Battle Meditation skips droids and "4th+ members" | **CONFIRMED / moot** | See 1.8 |
| `0x57E280` = RemoveMindTrickAndScrambleEffects | CONFIRMED (closes GATE_wave1 open item) | `FUN_005e4e60(0) == 0x12 \|\| == 0x13` then `FUN_00544790(...)` |
| Apply-table slots (type 0x13 death `0x597D90`, 0x17 SetAIState `0x59C3B0`/`0x59C420`, 115 faction `0x5951A0`) | CONFIRMED | `00593acb MOV [ECX+0x4c],0x597d90`, `00593cca MOV [EAX+0x5c],0x59c3b0`, `00593cd7 MOV [EDX+0x5c],0x59c420`, `005946f3 MOV [EAX+0x1cc],0x5951a0` |

#### 1.7 +0x1114 and +0xEC (reach for party members)
- **`+0x1114` mask**, CONFIRMED for bits 0x2, 0x4, 0x80 and 0x8:
  - 0x2 = movement: 0xA CheckMoveToPoint, 0x3D, 0x46, CounterSpell; MoveToPoint only in the `0x5C172F` object-type case 6/8 (`005c1745 MOVZX EDX,word [EDX+0x1114]` ... "I have no legs" `005c1783 PUSH 0x99649c`). It is not a general move gate.
  - 0x4 + 0x80 = attack (`0x6D7754/0x6D777E`).
  - 0x8 = spells in HasTalent (`006b8aa3`).
  - 0x40 med. 0x01/0x10/0x20/0x100 have no server reader (low).
- **`+0xEC` gated paths:**
  - Add* entries: AddAttackActions `0x568695` (except states 1/0x10), AddCastSpell `0x565713`, AddItemCastSpell, Equip/Unequip/Stance, AddDoCommandAction `0x5E134C`.
  - Script handlers with an explicit test: ActionFollowLeader `0066a001`/`0066a00e JNZ`, MoveToObject `0068cccc`/`0068ccd9`, MoveAwayFromObject `0068ca9d`, Wait `00695fa3`, JumpToObject/Point, MoveAwayFromLocation, RandomWalk, conversations, OpenDoor, PlayAnimation, SpeakString, CastSpell (census).
- **NOT gated:**
  - ActionMoveToLocation (21) and ActionForceMoveToLocation (382) = `0x68CEE0` -> `0x564770` -> `0x53F7F0`. The census over all three shows no `+0xEC` read, so the move node IS queued.
  - Whether it then walks under a state mask is not settled: type 1 has no general bit-2 gate (open item).
  - AI8's line "`+0xEC==0` drops the AI's ActionMoveToLocation" (Mod 8 kite) is refuted.
- **Live writers of `+0xEC := 0` that reach party members:**
  - SetState apply `0059def8` for every state except 9 (`0059debe CMP [EBP-0x10],9 / JZ` skips the block). That covers confusion, fear, droid stun, stun, paralysis, sleep, choke, horror, whirlwind, force jump, knockdown, crush, droid confusion, mind trick and scramble.
  - The heartbeat re-assert for states 1/0x10 (`0056bb0d`, `0056bc3d`).
  - Knockdown apply `0059bc27` (restored at `0059bf67`).
  - OnRemoveSetState re-elect `0059dd65` when another state remains.
  - Script SetCommandable / action 0x2F (`00691004`, `005e0f1b`), and LoadObjectState `0053fccc` (saved value).
  - Effect-table slot 15 `0x59ACC0` (`0059ade0`, conditional, type unnamed).
- **Restored by:** OnRemoveSetState `0059db95`, ClearAllEffects `0054a5fb`, `0x599137`.
- Engine code that must clear actions forces `+0xEC = 1` temporarily: SetState apply `0059dec7`, follow state 0 `005c77c4`/`005c7a18`, SpawnNPC `005fd549`/`005fd563`.

#### 1.8 Battle Meditation (R2 L2 risk)
- `k_inc_force.nss` 3972-4027: the link is built once inside the spell script (stamped with the cast id). Then `nCnt` 0..2 over `GetPartyMemberByIndex` (client CSWParty, max 3 slots, the caster included) and `GetRacialType != RACIAL_TYPE_DROID`, then Remove/Better checks, then Apply.
- "4th+ member" cannot occur: the active party has at most 3 (`*CSWParty` max 3).
- Non-selected companions never get it. Party droids never report it, so a family test on a droid target is always FALSE (recast risk only if L2 tests a droid).
- R2 L2a tests the caster itself (`GN_PartyBuffActive` on OBJECT_SELF, a Jedi): correct. L2 risk is LOW.
- Effects created later through `DelayCommand` would carry -1 (`+0xFF8` reset at `0056fcfd`). The BM tree does not do that.

## 2. Reconciliation (contradictions found and resolved)
1. **AI7 vs R4 / lesson 280** ("GetCreatureHasTalent(tSpe11_05) implies stock >= 1"): the implication holds, but the converse fails for Force-only class lists (1.5). Lesson 280 needs a correction line. R4 row 4 "verified" is wrong for Kreia, Visas and a Jedi PC.
2. **AI6 vs AI5/SpawnNPC:** AI6 said a flag-0 companion is judged by "its own faction row". SpawnNPC puts it in the PC's faction (`005fd535`), so it is the PC faction row. Corrected.
3. **AI7 internal:** Q3 "an attack round reports ATTACKOBJECT" vs its own map 0x3F -> 0x27. A round-mode approach reports 39 (1.6).
4. **AI7 vs AI6:** slot 7 `+0x1BC` "volume" (AI7) vs "pattern number" (AI6). AI6 is right (`00547515 PUSH 0x1` / `0054751a CALL 0x5e1bd0` with `[EBP-0x58]` = TestListenExpression result; cmd 195 reads `+0x1BC`).
5. **AI7 vs AI5:** `0x599A10` "raise/resurrect" (AI7) vs "creates a party-flagged creature, effect 89" (AI5). The code creates a new creature (`new(0x1220)`) behind `*CSWParty < 3`. AI5's reading stands; identity open.
6. **AI8 internal:** "script Action* dropped" vs ActionMoveToLocation `0x68CEE0` having no `+0xEC` test (1.7). The writer `0x596A7A` is dead.
7. **AI7 Q4.1** exclusion semantics vs the asm (`006b8c5f`, `006b8d2f`): refuted (table above).

Checked, no contradiction:
- AI5 vs AI7 on IsObjectPartyMember (= raw `+0x11AC`, both) and ActionFollowLeader (`+0x11AC != 0 && +0xEC == 1`, both; AI8 lists the same gate). AI7's "engine's own active party test is `0x7E5C40`" applies to the OnNotice event only. For DLLs, `+0x11AC` and `0x7E5C40` agree except during transients.
- AI8 vs AI2 / GATE_wave1 / lesson 315 on the slot-6 site (`0x59E860`, same state set; AI8 adds the creators for 1, 2, 4-6, 9, 0xD-0x13).
- AI5 vs GATE (lesson 318 extends to the save path; `+0x220` QuickWalk gate consistent with lesson 312).
- AI5 FM6 vs lesson 277 ("consumer unknown"): now known, and FM6 is partly refuted.
- AI6 E9 vs lesson 228/261: dlg scripts route shouts (installed: to k_ai_master 2004).
- AI8 0x57E280 closes the GATE_wave1 demotion.

## 3. Confidence rules (as GATE_wave1) and exceptions
Rules R-high / R-med / R-low are unchanged from GATE_wave1 section 3. Gate re-read rows are high.

| batch | row | change |
|---|---|---|
| AI5 | (a) `0x5FB070` "GetIsMember_2" | high; rename "GetIsNPCObjectMember(objId)" (PC -> 0) |
| AI5 | (b) 2.5 party test | high, with the PC caveat (1.1) |
| AI5 | (b) 8 FM4 + patch B | high; add "B only with A" |
| AI5 | (a) `0x669020` | low (not a command; no callers) |
| AI5 | (a) `0x599A10` / effect 89 identity | low |
| AI5 | (b) 4 speed thresholds, (d) ranges.2da numbers | low (runtime/2DA) |
| AI5 | (b) 1 FollowInfo "never written" | med (absence) |
| AI5 | (e) `+0x1114` bit1 "unknown" | high: bit 0x2 = movement (AI8) |
| AI5 | (a) `0x7E6CE0` CSWParty::Update, `0x7E5DF0`, `0x7E8350` | med (decompile-based names) |
| AI6 | (b) AdjustReputation | high BUG; reach corrected (PC-faction targets unaffected) |
| AI6 | (b) Q2 "flag-0 companion own faction row" | corrected (PC faction via SpawnNPC), med |
| AI6 | (b) Q1 CreateDefaultFactions order, col-0 question | low (2DA not read) |
| AI6 | (e) `+0x1F31C/+0x1F320` | low |
| AI6 | (b) Q3 delivery timing | low |
| AI6 | (b) shout abort | code high, reach low |
| AI7 | (b) Q1 command table | high (gate re-derived) |
| AI7 | (a) `0x6F2A30` IsForceClass, names "SPECULATIVE" | high, names from classes.2da |
| AI7 | (b) Q4.1 exclusion | refuted (see table) |
| AI7 | (b) Q3 round case | corrected (39) |
| AI7 | (b) Q2 slot 7 `+0x1BC` "volume" | corrected (pattern number) |
| AI7 | (b) Q2 slot 9 "no run site" | med (absence) |
| AI7 | (b) Q1 GetNearestCreature cost magnitude | low |
| AI8 | (b) Q1.5 "script Action* dropped" | partly refuted (`0x68CEE0`) |
| AI8 | (e) `+0xEC` writer `0x596A7A` | remove (dead) |
| AI8 | (b) mask bits 0x01/0x10/0x20/0x100 | low (absence) |
| AI8 | (b) states 0xB/0xC/0x11 "no creator" | med (absence) |
| AI8 | (b) Q2 "4th+ member" | moot (max 3 active) |
| AI8 | (b) Q3 AoE cadence, Q4 EFFOVF reach | low / med |

## 4. DB name_map corrections (for the persist script)
Current DB rows checked read-only on 2026-10-02.

| addr | current name (conf) | new name | conf | evidence (one line) |
|---|---|---|---|---|
| 0x005fe4b0 | CSWSCreature::MakePlayerCharacter (high) | CSWPartyTable::SwitchPlayerCharacter(npcIdx) | high | `00694436 CALL 0x51c8d0` / `0069443b MOV ECX,EAX` / `0069443d CALL 0x5fe4b0` (cmd 11) |
| 0x0066b770 | …QuickWalk_FollowLeader_ToOffsetWaitDest (med) | Global::ExecuteCommandChangeObjectAppearance (850) | high | InitializeCommands store idx 850 (gate table) |
| 0x0066b8a0 | …QuickWalk_FollowLeader_ToPathWaitDest (med) | Global::ExecuteCommandCreatureFlourishWeapon (847) | high | store idx 847 |
| 0x00669020 | …FollowLeader_Moving_To_Path (med) | FUN_00669020 (unknown; not follow, not a command) | low | no table store, no direct callers |
| 0x005752c0 | CSWSCreature::RemoveMindTrickEffects (low) | CFollowInfo::CFollowInfo | high | `0058a384 PUSH 0x3c` / `0058a386 CALL 0x919723` / `0058a3a1 CALL 0x5752c0` |
| 0x0067c430 | Global::ExecuteCommandGetEmotions (med) | Global::ExecuteCommandGetIsEnemy/Friend/Neutral (235-237) | high | stores idx 235/236/237; `0067c4f9 CMP [EBP-0x30],0xeb` |
| 0x00683740 | Global::ExecuteCommandGetMetaMagicFeat (med) | Global::ExecuteCommandGetListenPatternNumber (195) | high | store idx 195; reads `+0x1BC` |
| 0x0068b4b0 | Global::ExecuteCommandIsAIState (med) | Global::ExecuteCommandGetIsDead (140) | high | store idx 140 |
| 0x00683d40 | Global::ExecuteCommandGetNearestObject (med) | Global::ExecuteCommandGetNearestCreature/Object (38, 226-229) | high | stores idx 38, 226, 227, 228, 229 |
| 0x0068cee0 | Global::ExecuteCommandMoveToPoint (med) | Global::ExecuteCommandActionMoveToLocation (21, 382) | high | stores idx 21 and 382; no `+0xEC` read |
| 0x0067ae40 | Global::ExecuteCommandGetCreatureTalent (med) | Global::ExecuteCommandGetCreatureTalentRandom/Best (307/308) | high | stores idx 307/308 |
| 0x0057e280 | CSWSCreature::SetLastInstantEquip (low) | CSWSCreature::RemoveMindTrickAndScrambleEffects | high | `FUN_005e4e60(0) == 0x12 \|\| == 0x13` -> `FUN_00544790` |
| 0x005fd480 | CSWPartyTable::AwardXPToPartyMember? (med) | CSWPartyTable::SpawnNPC(idx, useGiven, pos, ori, flag) | high | `005fd4c9 CALL 0x5faaf0`; `005fd535 CALL 0x6d3330` (joins PC faction); no flag write |
| 0x007e6ce0 | CClientParty::RangeMonitor (med) | CSWParty::Update(dt) (5 m edges, retreat) | med | `007e77ab FCOMP [0x989ed0]` (25.0); only caller `0x781BE0` |
| 0x0058a330 | CSWSCreature::SetPartyMemberFlag (med) | (same) | high | `0058a35e MOV [EAX+0x11ac],ECX`; levels 2/4 at `0058a448`/`0058a3e6` |

New rows (absent from name_map today):
| addr | name | conf | evidence (one line) |
|---|---|---|---|
| 0x0051c8d0 | CServerExoApp::GetPartyTable (inline at *(this+4)+0x1F0B4) | high | `0051c8da MOV EAX,[EAX+0x4]` / `0051c8dd ADD EAX,0x1f0b4` |
| 0x005fb070 | CSWPartyTable::GetIsNPCObjectMember(objId) (PC -> 0) | high | `005fb09a CMP EAX,[EDX+ECX*4+0x1c]` (12 slots), `005fb0b8 CMP EAX,[EDX]` |
| 0x005fafd0 | CSWPartyTable::GetIsMember(npcIdx) | high | loop `param_1[i+2] == idx`, `i < *param_1` |
| 0x005f8d80 | CSWPartyTable::AddMember(npcIdx, objId) | high | `if (*param_1 != 2 && 0x5FA960(idx))`; `FUN_0058a330(1,1)` |
| 0x005f9220 | CSWPartyTable::CreateParty | med | per member SpawnNPC + `FUN_0058a330(1,1)` |
| 0x007e5c40 | CSWParty::IsPartyMember(clientId) | high | `param_1[local_8*0x22+9] == param_2` |
| 0x007e5730 | CSWParty::AddCharacter(id) | high | `007e5a69 PUSH 0x3d` / `007e5a6e CALL 0x53f7f0` |
| 0x007e5df0 | CSWParty::SetLeaderBySlot(slot) | med | `007e64da PUSH 0x9a2178` (k_hen_leadchng); `007e6546 MOV [..+0x94],1` |
| 0x007e8350 | CSWParty::ResetAllFollowData(pos, heading, flag) | med | slot states `-1`; caller `00797fb9` |
| 0x007e7d70 | CSWParty::GetFollowRange | med | ranges.2da row 0xE/0xF, default `[0x986b68]` 5.0 |
| 0x005ca6a0 | CSWSCreature::FollowState5_Hold (FM6 consumer) | high | `005ca7c6 MOV ECX,[EAX+0x70]`, `005ca7e8 FCOMP [0x996520]` (49.0) |
| 0x005c7370 | CSWSCreature::FollowState0_PlaceBehindTrail | med | queues 0x40 + move in the 0x3D group (`FUN_005d5120(0x3d)`) |
| 0x00664580 | CFactionManager::GetNPCFactionReputation(a,b) = data[b*N+a-1] | high | `006645f2 IMUL ECX,[EAX+0x1c]` on `[EBP+0xc]` |
| 0x006644d0 | CFactionManager::SetNPCFactionReputation(row,col,v) = data[row*N+col-1] | high | `00664534 IMUL ECX,[EAX+0x1c]` on `[EBP+0x8]` |
| 0x00664650 | CFactionManager::GetIsNPCFaction(id) (1..N) | high | `00664659 CMP [EBP+8],1` |
| 0x006d3330 | CSWSFaction::AddMember(objId, bSuppress) | high | `006d33c8 MOV [EDX+0x78],EAX` |
| 0x0066f0a0 | Global::ExecuteCommandChangeFaction/ChangeToStandardFaction (173/412) | high | stores idx 173/412 |
| 0x0063a5b0 | DeliverSpeech(type, speakerId, CExoString) | high | radii pushes `0063a763`/`0063a7f1`/`0063a9bb`/`0063ab24`/`0063ab7e` |
| 0x006cf5d0 | DialogStartPacifyHostiles | med | `006cf701 CMP EAX,0xa` / `006cf70b CALL 0x541080` |
| 0x0066daf0 | Global::ExecuteCommandActionAttack (37) | high | store idx 37 |
| 0x006b8840 | CSWSCreatureStats::HasTalent(type,id) | high | `006b89ce CALL 0x6f2a30` / `006b8a6c CALL 0x6c0330` / `006b8aa3 … +0x1114 & 8` |
| 0x006b8bb0 | CSWSCreatureStats::GetTalentBestOrRandom (coin flip) | high | `006b8c2b CALL 0x91eb8b` (rand) / `006b8c30 AND EAX,0x80000001` |
| 0x006baec0 | CSWSCreatureStats::PickKnownSpellTalent | med | `006bb09e CALL 0x6ba970`, `006bb0ca CMP [EBP+0xc],-1` |
| 0x006ba970 | CSWSCreatureStats::IsSpellTalentCandidate | high | `006ba97c CMP EAX,[EBP+0x24]` / `006ba98e CALL 0x6b8840` |
| 0x006bb3a0 | CSWSCreatureStats::PickItemSpellTalent | high | equip pass `006bb6db CALL 0x6ba970`; repository pass category only |
| 0x006bc0e0 | CSWSCreatureStats::PickFeatTalent | med | `006bc23e CMP [EBP+0xc],-1` |
| 0x006f2a30 | IsForceUserClass(classId) {3,4,5,11..16} | high | compare chain `006f2a34..006f2a68`; classes.2da forcedie |
| 0x0057d8b0 | CSWSCreature::UseTalentOnObject | med | `0057d8e3 MOV ECX,[EAX+0xec]`; item -> `0x57B4C0` |
| 0x0059c3b0 | CSWSEffectListHandler::OnApplySetAIState (type 0x17, mask AND) | high | `00593cca MOV [EAX+0x5c],0x59c3b0`; `0059c409 MOV [EDX+0x1114],AX` |
| 0x0059c420 | CSWSEffectListHandler::OnRemoveSetAIState | high | `00593cd7 MOV [EDX+0x5c],0x59c420`; `0059c536` |
| 0x00597d90 | CSWSEffectListHandler::OnApplyDeath (type 0x13) | med | `00593acb MOV [ECX+0x4c],0x597d90`; `005982d7` mask := 0 |
| 0x005e5ea0 | CGameEffect::SetCreator (stamps +0x1C spell id) | high | `*(param_1+0x1c) = creator->vt[0xB8]()` |
| 0x005e5f60 | CGameEffect::GetEffectTypeScriptConstant | med | AI8 switch (state -> script type) |
| 0x0062c630 | CSWSAreaOfEffectObject::EventHandler | med | `0062c6a2 MOV [EAX+0x290],ECX` (enter), `+0x294` (exit) |

Struct fields worth adding (struct_fields): CSWSCreature `+0xEC` commandable (high, `005410e9`); `+0x1114` allowed-action mask u16 (high, `0059c409`); `+0xFF8` effect spell id (high, `0056337d`); `+0x1A0` last speaker, CSWSObject `+0x1BC` last pattern, `+0x1C0` listening (high, AI6 quotes); CFollowInfo `+0x38` InSafetyRange (high); CSWParty slot `E+0x70` FM6 hold flag (high); CSWPartyTable `+0x00` member count / `+0x1C` NPC object ids / `+0x114` leader idx (high); CFactionManager `+0x1C` N / `+0x20` matrix (high).

## 5. Probe-window notes (checked against `.asm` and the vanilla exe; inbound scan over every `.asm` + the function's jump tables)
| window | len / stolen | boundary | rel branch inside | targets inside / at start | notes |
|---|---|---|---|---|---|
| AI5-H1 `0x58A35E` | 6 `89 88 AC 11 00 00` | OK | none | none / none | Consistent with GATE_wave1: handler sees OLD `[EAX+0x11AC]`, new in ECX, apply `[EBP+0xC]`, return address `[EBP+4]` -> site table 1.1. Cold. Expect transient (0,1)/(old,1) pairs per save from `0x574537`/`0x575170`. |
| AI6-F5 `0x57074F` | 9 `8B 95 18 FD FF FF 8B 45 0C` | OK | none | none / jump table `0x570FF4[2]` (start, allowed; 7 entries scanned) | EventHandler case shout-receive. `[EBP-0x2E8]` = listener, `[EBP+0xC]` = speaker, `[EBP+0x10]` = payload (int1 = pattern). Stolen writes EDX/EAX only. Warm (per delivered shout). |
| AI6-F4 `0x547100` | 10 `55 8B EC 6A FF 68 AB FA 94 00` | OK | none (`PUSH imm` SEH) | none / 5 CALLs from `0x63A5B0` | Entry: ECX = speaker, `[ESP+4]` &string, `[ESP+8]` radius float, `[ESP+0xC]` bRequireSeen. Warm in fights (one per combatant per round). Pair with F5 for the delivered/attempted ratio (the abort shows as 0 deliveries). |
| AI7-EV `0x56EFF9` | 6 `89 8D 18 FD FF FF` | OK | none | none / none (jump table scanned) | **HOT** (every event, every creature). ECX = creature, `[EBP+8]` event id, `[EBP+0x10]` payload (word = sub-type). Stolen writes memory only; the next insn reads it back. Keep the handler to a few compares. |
| AI7-P7ACT `0x67B204` | 10 `8B 4D F8 51 8B 0D A8 B4 A1 00` | OK | none | none / `0x67B14C`, `0x67B161`, `0x67B183`, `0x67B1FB` (start) | `[EBP-8]` = result on every path (init `0067b109` = 0xFFFF, so the invalid-object paths log 0xFFFF). Stolen includes `PUSH ECX`: fine (runs after the handler). Warm (per script call). Expect 3/39/38, not 0, during approach/follow. |
| AI8-H1 `0x59D966` | 6 `88 82 F5 0F 00 00` | OK | none | none / none | EDX = creature, AL = new state, `[EDX+0xFF5]` = old. Stolen reads AL: the wrapper must preserve EAX. Cold. |
| AI8-H3 `0x59E8F4` | 10 `6A 01 8B 55 F8 52 8B 45 F4 50` | OK | none | none / `0x59E8EB` JMP (start) | At window start no RunScript arg is pushed yet (all 3 pushes are stolen), so the handler sees a clean frame. `[EBP-4]` creature, `[EBP-8]` id, `[EBP-0xC]` slot ptr. Cold. |
| AI8-H9 `0x67E70C` | 10 `8B 4D F0 51 8B 0D A8 B4 A1 00` | OK | none | none / `0x67E6EA`, `0x67E6FB` (start) | `[EBP-0x10]` = result (init 0 at `0067e679`), `[EBP-8]` = spell. On the two early paths `[EBP-0xC]` is the unresolved object: null-check it before reading `+0x148`. Warm. |
| AI8-H10 `0x544210` | 9 | OK | — | function entry | **Collides with live perf-lab `PerfLabEffectApply` (same address/length)**: extend it, never ship both. |
| FM4-A `0x574529` / FM4-B `0x575160` | 1-byte simple patches | imm8 of `6A 01` | n/a | n/a | A is 10 B from shipped `0x57451F` (no overlap), B is 35 B from `0x575183`. Ship together (1.2). |

Overlaps: no window is within 12 B of the sibling list (nearest 1.28 MB) or of `live_hooks_2026-09-30g.tsv` / the companion-ai and ranged-ai tomls (nearest: F4 10.9 KB from live `0x544640`), except AI8-H10. The 10-02 installs (activation fix, engine-bug L1/L4) are not in the 09-30g TSV: the lead's `check_windows.py` must use the live config.

## 6. Lessons to add (scope / topic / note <= 600 chars / anchor)
1. party-ai / `+0x11AC` = PC or CSWPartyTable member / Writers: ctor and `0x58A35E` (SetPartyMemberFlag `0x58A330`, 15 sites). It is set only for the PC and for NPCs that pass AddMember (max 2) / CreateParty / party-select. SpawnNPC `0x5FD480` never sets it, so Hawk standees are 0. It equals IsObjectPartyMember. A DLL with a creature pointer should test `+0x11AC`. With only an id: `0x5FB070` (table = `*(serverApp+4)+0x1F0B4` via `0x51C8D0`) covers NPCs only, so OR it with the PC id. It is transiently 0 inside SaveCreature/TransferInventory. Anchor: GATE_wave2 1.1.
2. party-ai / SpawnNPC joins the PC faction / `0x5FD480` calls GetNPCObject(create) (`005fd4c9`), then AddMember into the PC's faction (`005fd52e`/`005fd535`). It runs ClearAllActions with `+0xEC` forced to 1 and writes no party flag. Non-selected companions therefore share the PC faction's reputation row. GetReputation `0x5D0D00` maps flagged members to faction 0 and gives two flagged creatures 100. Anchor: GATE_wave2 1 (AI6).
3. scheduler / SaveCreature sets non-party creatures to AI level 2 on every save / `0x574532` (0,1) and `0x57516B` (old,1) both apply. For old=0, SetAILevel(2) runs; the next AIUpdate demotes the creature to 0/1, and script-set levels 3/4 are lost. Party members bounce 4->2->4 and get a default FollowInfo. The fix needs BOTH bytes `0x574529` and `0x575160` (01->00). B alone frees FollowInfo forever (0x3D returns 3 until the next party change). Anchor: GATE_wave2 1.2.
4. faction / AdjustReputation reads the transposed cell / Get `0x664580` = `data[b*N+a-1]`, Set `0x6644D0` = `data[row*N+col-1]`; `0x5D0DF0` passes (own, p2) to both. So new M(S->T) = M(T->S) + delta. rep(S->T) is cell (row fT, col fS). A PC-faction target (PC, companions) is unaffected via the Get swap. Only cmd 209 reaches it; vanilla sources: only unused_sandper.nss. Fix: swap the pushes at `0x5D0E40`. Anchor: GATE_wave2 1.3.
5. faction / shout delivery rules / SpeakString -> `0x63A5B0` -> `0x547100(r, bSeen)`. Radii: TALK 1000, WHISPER 3, SHOUT 250, SILENT_TALK 35 (+bSeen), SILENT_SHOUT 1000. A listener needs `+0x1C0`, dist <= r, the perception entry HEARD bit, and with bSeen: SEEN or dist < 10 m. There is no faction filter. The start index from `0x522AF0` is unchecked and an unresolvable id ends the scan (silent). Event 10 type 7 sets `+0x1A0` speaker and `+0x1BC` pattern, then slot 7. Anchor: GATE_wave2 1.4.
6. party-ai-scripts / medpac talent fails for Force-only classes / HasTalent `0x6B8840` spell path: Force class slots ({3,4,5,11..16} via `0x6F2A30`, classes.2da forcedie > 0) test only known+affordable; only non-Force slots reach the item scan `0x6C0330`. Kreia (class 4), Visas (5) and a Jedi PC get FALSE for GetCreatureHasTalent(medpac) even with stock. TalentBest(0x1408) still returns the medpac (the repository pass has no class test). Use stock + GetIsTalentValid. Anchor: GATE_wave2 1.5.
7. party-ai-scripts / GetCurrentAction = last reportable node of the head group / `0x67B100` overwrites the result per reportable node until the group id changes. Engine approaches re-queue into the attack node's group: 0xC -> 3, 0x3F round -> 39; follow moves join the 0x3D group -> 38. MOVETOPOINT (0) appears only for script move chains. QUEUEEMPTY 0xFFFE only for an empty list; 0xFFFF when nothing is reportable. Anchor: GATE_wave2 1.6.
8. party-ai-scripts / TalentBest/Random arguments / `0x6B8BB0` flips `rand()&1` between the spell group (known -> class-granted -> item) and feats. nCRMax is only compared with -1 (-1 = Random). nExcludeId is never applied to spells (`006bb07e PUSH -1`). nExcludeType 0/-1 still runs the spell pick once, and 1/-1 marks the SPELL group done (`006b8d2f`). Rank = 2DA MaxCR, highest wins. Anchor: GATE_wave2 1 (AI7).
9. combat-ai / commandable `+0xEC` / ClearAllActions `0x541080` does nothing unless `+0xEC == 1`. SetState apply clears actions with `+0xEC` forced to 1, then sets 0 (not for state 9). The Add* entries and most Action* handlers refuse while 0. ActionMoveToLocation/ForceMoveToLocation `0x68CEE0` do NOT test it and still queue a move. Writer `0x596A7A` is dead code (`005969cb XOR EDX,EDX / JZ`). Anchor: GATE_wave2 1.7.
10. combat-ai / allowed-action mask `+0x1114` / u16, init 0xFFFF, death 0. Type-0x17 effects AND it (`0x59C3B0`) and their removal recomputes it. Bits: 0x2 movement (0xA, 0x3D, 0x46, CounterSpell; MoveToPoint only for the 0x5C172F object-type case), 0x4+0x80 attack (`0x6D7754/0x6D777E`), 0x8 spells in HasTalent, 0x40 client awake. Every SetState also clears 0x100; stun-likes use -3 (no movement), paralysis/sleep clear 4 and 8. Anchor: AI8 Q1 + GATE_wave2.
11. combat-ai / GetHasSpellEffect / `0x5476C0` scans `+0x148/+0x14C` for `effect+0x1C == spell`. `+0x1C` comes from CGameEffect::SetCreator `0x5E5EA0` = the creator's `+0xFF8`, which EventHandler sets around the spell script (`0056fc64` .. `0056fcfd` PUSH -1). Effects built outside the spell script (or via DelayCommand) carry -1. Battle Meditation goes to party slots 0..2 except droids. Anchor: GATE_wave2 1.8.
12. method / command table decode / `InitializeCommands 0x665F50` fills `malloc(0xDB4)` with 774 `MOV [reg+off],imm` stores; index = off/4 = nwscript.nss prototype order (877 = 0x36D). Re-derive it from the `.asm` and byte-compare each store with the exe instead of reusing a pickle. Checks: 0x67AE40 self-tests 0x133/0x134, 0x67C430 0xEB. DB names from the GOG mapper for command handlers were wrong 7 times. Anchor: GATE_wave2 section 4.

## 6a. New candidates (subsystem 'ai')
| id | title | verdict | impact | fix idea | risk |
|---|---|---|---|---|---|
| AI-REPTRANSPOSE | AdjustReputation reads the transposed matrix cell | BUG (latent) | none in vanilla; mod/module scripts adjusting NPC-vs-NPC asymmetric factions get a wrong baseline | swap the pushes at `0x5D0E40` (8 B simple patch) | low; behaviour changes only for asymmetric NPC rows |
| AI-SHOUTABORT | Shout loop drops every listener on an unresolvable id | CONFIRMED code, reach LOW | E9 shouts lost silently in an area with a stale id (would also blind perception) | probe F4+F5 first; if seen, retarget `00547203 JNZ` to the loop head (`0x5471B1`) and seed idx 0 when `0x522AF0` fails | low-medium (skipping stale ids) |
| AI-FM4B | SaveCreature sets non-party creatures to AI level 2 on each save | CONFIRMED | one level churn per creature per save; script-set levels 3/4 lost | byte patches `0x574529` + `0x575160` 01->00, together | low; save format unchanged (FollowInfo saved live) |
| AI-MEDPAC-FORCECLASS | GetCreatureHasTalent(item spell) FALSE for Force-only classes | CONFIRMED | Kreia, Visas, a Jedi-only PC never use medpacs via the R4 rule (and vanilla NPC rule) | script: party branch uses `GetIsTalentValid(tSpe11_05)` + stock >= 1 instead of HasTalent | low (script-only; UseTalent bypasses HasTalent) |
| AI-TALENTEXCL | TalentBest exclusion flags set the wrong group | CONFIRMED (no current use) | none for our scripts (no exclusion args) | document only; do not rely on nExcludeType/Id | none |

## 7. Open items for synthesis
- Whether a move queued while non-commandable (`0x68CEE0`) actually walks under a SetState mask: type 1 has no general bit-2 gate. Probe: count type-1 results during stun (RunActions result window `0x5D6A60`).
- E4 design: key on position progress during {0, 3, 39} or on the 0x11 re-approach count; MOVETOPOINT alone misses engine approaches.
- E9: decide with the user (behaviour change). The runtime check is F4/F5: deliveries per round vs the companions in 35 m; heard-bit share.
- Medpac script fix for Force-only members (AI-MEDPAC-FORCECLASS); update lesson 280 and R4 row 4.
- FM4B: ship A+B together, or keep FM4 mode 1 and accept the non-party churn (user decision; the effect is small).
- Identity of effect type 89 (`0x599A10`) and of effect-table slot 15 (`0x59ACC0`, conditional `+0xEC := 0`).
- `0x669020` identity; `m_spn_visas` listening setup (module script, not in the corpus).
- Module-embedded NCS scan for AdjustReputation (cmd 209) and for exclusion args to TalentBest.
- From AI5/AI6: ranges.2da rows 0xB-0x10, repute.2da orientation dump (AI6 P5), StateScripts.2da rows 1-3.

## Correction (REVIEW_synthesis 2026-10-02)
Opus review of the synthesis (REVIEW_synthesis.md); the sections above are left as written.
- **E2 (section 1.1, party-test text, and the 0x5FB070 table row):** `0x51C8F0` is the **controlled leader**, not the PC. `0x51C8F0` (thiscall serverApp) -> `0x539D60` -> player `0x632A20` = `GetGameObject(player+0x28)`; SetLeaderBySlot rewrites player+0x28 (`007e631d CALL 0x632830`, `006328d8 MOV [EDX+0x28],EAX`). `0x5FB070(id) || id == 0x51C8F0()` therefore misses the Exile while the player controls a companion. Correct test with only an id: resolve it (`0x51C0A0`) and test `+0x11AC`; `0x5FB070` covers NPCs only. Confidence med-high; probe PARTY settles it.
- **E5 (section 1.5, medpac):** "a Jedi-only PC" is understated: the Exile **always** has `GetCreatureHasTalent(tSpe11_05) == FALSE` (every K2 PC class is in {3,4,5,11..16}: chargen offers 3/4/5 and the prestige classes are 11-16), so the AI-driven Exile never uses medpacs through the HasTalent rule. Design: party branch = `GetIsTalentValid(tSpe11_05) && GN_PartyMedpacStock() >= 1`, never HasTalent; keep the `GetCommandable()` guard (`ActionUseTalentOnObject` -> UseItem tests `+0xEC`, `0057d8e3`).
- **E1 (relevant to lesson P2 in section 6):** `+0x10E8` = IsPC = `stats+0x6C`, written by the ctor `00562d7d`, PostProcess `005768fe`, SwitchPlayerCharacter `005fe604/005fe951/005feae6` and `00628375`; SetLeaderBySlot never writes it. The 200 ms perception cadence therefore goes to the IsPC creature (the Exile, also while AI-driven), not to the controlled one.
