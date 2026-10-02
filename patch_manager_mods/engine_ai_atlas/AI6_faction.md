# AI6 Faction, reputation and shouts (KOTOR II Steam swkotor2.exe 6a522e71)

Pipeline position: perception (AI3) and target selection (AI2) ask "is T hostile to S" through `CSWSObject::GetReputation 0x5E13C0` / `GetAIStateReputation 0x5E1680` -> `CSWSCreature::GetCreatureReputation 0x5D0D00` -> `CFactionManager` matrix (`0x664580`). There is no per-creature reputation storage and no cache: "personal" behaviour is faction membership (`stats+0x78`), the party flag and effect type 115. The second half of this page is the **shout path**: script `SpeakString` (`0x693860`) -> `DeliverSpeech 0x63A5B0` -> listener broadcast `0x547100` -> event 10 / payload type 7 posted to each accepted listener (delta 0) -> `CSWSCreature::EventHandler 0x56EFD0` case `0x57074F` -> script slot 7 (OnDialogue). Allied-shout handling (E9) rides on this path.

## Status
- Sources: batch `findings/AI6.md` (Sonnet) + `GATE_wave2.md` (Opus gate, 2026-10-02; section 1 AI6 verdicts, 1.3 AdjustReputation, 1.4 shout path and E9, section 2 reconciliation 2, section 3 confidence, section 4 names, lessons 2, 4, 5) + `LEAD_SPOTCHECKS.md` (shout abort lines CONFIRMED; transposition confirmed by quoting both index computations).
- Date: 2026-10-02. Static analysis only. Gate-verified: AdjustReputation transposition (both index computations quoted), precedence and thresholds, exe-read radius floats, no-faction-filter / HEARD / SEEN-or-10 m rule, payload int1 = pattern number, shout abort code. Delivery timing, repute.2da content, `+0x1F31C/+0x1F320` stay low.
- Conflicts resolved to the gate version are marked "batch said X; gate corrected".
- Full CFactionManager / CSWSFaction / creature faction field tables are in the batch (e) and DB `struct_fields`.

## Function table
| addr | name | conf | purpose |
|---|---|---|---|
| 0x5E13C0 | CSWSObject::GetReputation(sourceId, flag) | high | master lookup; `005e13c9 CMP [0x00A0F6E0],-1` override, result clamped 0..100 |
| 0x5D0D00 | CSWSCreature::GetCreatureReputation(otherId, otherFaction) | high | party-flag remap then `FUN_00664580(S, T)` = cell (row T, col S) |
| 0x5E1680 | CSWSObject::GetAIStateReputation | high | 2 enemy (<= 10), 1 friend (>= 90), 0 neutral; consumers `0x57C990`, `0x568540`, `0x585480`, `0x57BB90` and others |
| 0x664580 | CFactionManager::GetNPCFactionReputation(a,b) = `data[b*N+a-1]` | high | `006645f2 IMUL ECX,[EAX+0x1C]` on `[EBP+0xC]`; 50 if neither valid; swaps if only b valid |
| 0x6644D0 | CFactionManager::SetNPCFactionReputation(row,col,v) = `data[row*N+col-1]` | high | `00664534 IMUL ECX,[EAX+0x1C]` on `[EBP+0x8]`; needs 0 <= row <= N, 1 <= col <= N; clamp 0..100 |
| 0x664650 | CFactionManager::GetIsNPCFaction(id) | high | 1 iff `1 <= id <= N` (faction 0 = PC is not "NPC") |
| 0x664490 / 0x51CC80 | GetFaction(id) / CServerExoApp::GetFactionManager | high | `[[app+4]+0x10054]` |
| 0x664680 / 0x664AE0 / 0x664F30 | LoadFactions / LoadReputations / CreateDefaultFactions | high / high / med | one CSWSFaction per repute.2da row; matrix `(N+1)*N` default 100; defaults only when LoadFactions fails |
| 0x664CB0 / 0x664E40 | LoadReputationsFromSaveGame / SaveReputations | high | save keeps only cells != 100 (FactionID1 = row, FactionID2 = col) |
| 0x6D3330 | CSWSFaction::AddMember(objId, bSuppressMsg) | high | removes from old faction, appends, `006d33c8 MOV [EDX+0x78],EAX`; no perception/combat call; gate new row |
| 0x5D0DF0 | CSWSCreature::AdjustReputation(srcFaction, delta) | high | transposed Get/Set (see Bugs); sole caller cmd 209 `0x66D1B0` |
| 0x66F0A0 | cmd 173 / 412 ChangeFaction / ChangeToStandardFaction | high | AddMember only; PC stats refused |
| 0x696E10 / 0x5D51A0 / 0x6942F0 | ChangeFactionByFaction / SurrenderToEnemies / SurrenderToEnemies cmds | med / high / high | see flip propagation |
| 0x5951A0 / 0x5953E0 | effect type 115 (faction modifier) apply / remove | high | AddMember + ClearAllActions(1) + perception flush |
| 0x586750 / 0x5867E0 / 0x5864D0 | CancelCombat / ClearHostileActionsVersus / PacifyCreature | high | combat cancel family |
| 0x6CF5D0 | DialogStartPacifyHostiles | med | gate rename; every non-party hostile (<= 10) creature gets ClearAllActions(1) at dialogue start |
| 0x693860 | cmd 221 SpeakString / 39 ActionSpeakString | high | volume -> internal type {0,1,2,3,4} -> {1,3,2,0xD,0xE} |
| 0x63A5B0 | DeliverSpeech(type, speakerId, CExoString) ret 24 | high | gate name; radii pushes `0063a763`/`0063a7f1`/`0063a9bb`/`0063ab24`/`0063ab7e` |
| 0x547100 | CSWSObject::BroadcastDialog (listener loop) | high | the shout dispatch; this = speaker, `[ESP+4]` &string, `[ESP+8]` radius, `[ESP+0xC]` bRequireSeen |
| 0x546F10 / 0x546E40 / 0x546DD0 | TestListenExpression / SetListenExpression / GetListenPatternEntry | high | first matching pattern number else -1 |
| 0x5409F0 | CSWSObject::LoadListenData | high | gate new fact: listen patterns persist in saves (`00540b06` SetListenExpression; "Listening" in Save/LoadCreature) |
| 0x692090 / 0x692130 / 0x683740 / 0x682900 | cmd 175 SetListening / 176 SetListenPattern / 195 GetListenPatternNumber / 254 GetLastSpeaker | high | `+0x1C0` / patterns / `+0x1BC` / creature `+0x1A0`; `0x683740` DB name GetMetaMagicFeat corrected |
| 0x57074F | EventHandler event-10 payload-type-7 case (shout receive) | high | sets `+0x1A0`, `+0x1BC`, runs slot 7 (`005709f0 CALL 0x6FD8D0`) |
| 0x67C430 | cmd 235/236/237 GetIsEnemy / GetIsFriend / GetIsNeutral | high | gate name_map (DB said GetEmotions); thresholds `0067c564 CMP EAX,0xA`, `0067c5a3 CMP EAX,0x5A` |
| 0x687E00 | cmd 208 GetReputation | high | `00687e91 CALL 0x5E13C0` |
| 0x522AF0 | CSWSArea::GetFirstObjectIndiceByX | high (AI3) | start index of the X-sorted area list; return ignored by the shout loop |

## Key behaviours
1. **Reputation lookup order** (`0x5E13C0`, this = judged object T, arg1 = source id S; result = how S feels about T; matches the gate's confirmed precedence):
   1. `[0xA0F6E0]` debug override (`005e13c9`/`005e13d2`), returned UNCLAMPED; 0xFFFFFFFF in the exe (off), no writer in the dump. A DLL write forces every GetReputation / GetIsEnemy / perception hostility test to that value.
   2. Lookup failures (`0x51C080`/`0x53DFB0`, no manager `+0x10054`) return 50 (`005e1413`).
   3. T an AOE (`005e1457`, `[0x99447A]` = 11): replaced by its creator (`+0x288`, `005e1483`). S an AOE: recursion with this = creator (`005e150a`).
   4. Either faction lookup fails -> 50; same object id -> 100 (`005e1576`).
   5. S faction invalid for `GetIsNPCFaction` (0 = PC) and T valid: roles swapped (`005e15a1..005e15bf`), so (PC, f) pairs are symmetric.
   6. T a creature: `0x5D0D00`: both `+0x11AC != 0` -> 100; else a flagged side's faction is replaced by 0 (`005d0d8b` for T, `005d0daa` for S); then `Get(S faction, T faction)` = cell (row T, col S).
   7. T a placeable / encounter / door-class (`vt+0x48 / 0x5C / 0x18 / 0x40`): direct matrix lookup `005e164a`, NO party remap. Other classes fall through with 0 (hostile).
   8. Clamp 0..100 (`005e1656..005e166e`), not applied to the override / 50 / 100 early returns.
2. **Flag-0 companions (gate correction).** Batch said "a companion with `+0x11AC == 0` is judged by its OWN faction row"; gate corrected: SpawnNPC adds the NPC to the PC faction (`005fd535 CALL 0x6D3330`), so non-selected companions and Hawk standees use the PC faction row. The remap only matters between two flagged members (100).
3. **Thresholds** (asm): enemy `rep <= 10` (`0067c564 SETLE`), friend `>= 90` (`0067c5a3 SETGE`), neutral 11..89. GetIsEnemy for a placeable target also needs `placeable+0x398 != 0` (`0067c545`). Arg order: `GetIsEnemy(T, S)` = "S considers T an enemy" (`this` = oTarget). `GetReputation(src, tgt)` calls `tgt.GetReputation(src)`.
4. **Faction table layout.** `CFactionManager` = `new(0x28)` (`005363f7`/`00536411`), at `[[[0xA1B4A4]+8]+4]+0x10054`. `+0x00..+0x18` seven standard slots filled by label (`0x664A00`): +0 neutral, +4 insane, +8 friendly_2, +0xC friendly_1, +0x10 hostile_1, +0x14 hostile_2, +0x18 player. `+0x1C` N = NPC factions (excludes id 0). `+0x20` -> `CExoArrayList<int>` matrix of `(N+1)*N` cells, default 100. `+0x24` -> `CExoArrayList<CSWSFaction*>`; faction id = index = repute.2da row. Storage rule: `Get(a,b) = data[b*N + a - 1]`, `Set(row,col,v) = data[row*N + col - 1]`; `rep(S -> T) = cell (row fT, col fS)`. `Set` rejects col < 1, so 2DA column 0 is never a cell. Faction 0 (PC) is addressed through row 0 via the Get swap.
5. **CSWSFaction** = `new(0x24)`: `+0` member ids, `+4` count, `+8` capacity, `+0xC` id, `+0x10` parent (-1 for 2DA rows), `+0x14` GetFirst/Next cursor, `+0x18` name, `+0x20` "FactionGlobal". Creature side: `creature+0x1198` = stats, `stats+0x78` current faction (writers `0x6D3330`, personal ctor `0x6D30F0`), `stats+0x7C` template faction (PostProcess `00576a13`), `stats+0x6C` PC-controlled stats. Non-creature faction fields: placeable `+0x27C`, trigger/door `+0x2F8`, encounter `+0x26C`.
6. **Shout dispatch and radii.** `0x693860` maps nTalkVolume; `0x63A5B0` calls `0x547100(speaker, &string, radius, bRequireSeen)`:

| nTalkVolume | internal type | radius (exe float) | bRequireSeen | asm |
|---|---|---|---|---|
| TALK 0 | 1 | 1000.0 (`0x992F08`) | 0 | `0063a7ee PUSH 0`, `0063a7f1 FLD [0x992F08]` |
| WHISPER 1 | 3 | 3.0 (`0x992F0C`) | 0 | `0063a9b8`, `0063a9bb` |
| SHOUT 2 | 2 | 250.0 (`0x992F04`) | 0 | `0063a760`, `0063a763` (also `0x63B4F0` module-wide chat) |
| SILENT_TALK 3 | 0xD | 35.0 (`0x992F18`) + seen rule | 1 | `0063ab21 PUSH 1`, `0063ab24` |
| SILENT_SHOUT 4 | 0xE | 1000.0 (`0x992F1C`) | 0 | `0063ab7b`, `0063ab7e` |

7. **Listener loop (`0x547100`)** over `area+0x194` (ids sorted by X, count `+0x198`): start index by `0x522AF0(S.x - r)`; skip the speaker; stop when `listener.x > S.x + r`; accept if 3D dist^2 <= r^2 (`005472b6`), `+0x1C0` listening (`005472c6`), and: both creatures -> perception entry for the speaker (`0x5D1750`) with the HEARD bit (`0054734c..00547351`) AND, when bRequireSeen, SEEN or dist^2 < 100.0 (`0054739c FCOMP [0x98BA38]`); listener creature + speaker non-creature -> within `0x58B210(listener)^2` (hearing range); listener non-creature -> no further test. **No faction filter** (callees contain no `0x5E13C0`/`0x664580`): enemies and neutrals in range receive the same event; the script `GetIsFriend` filter is the only faction gate. Pattern `0x546F10` must match (-1 = skip).
8. **Delivery.** Payload `new(0x34)`: word 7 (`005474d4`), int0 = param count, int1 = pattern number (`00547511..0054751a CALL 0x5E1BD0`), then `0x51E170(0,0,speakerId,listenerId,[0x99241C]=10,payload)` delay 0. Receiver (`0x57074F`): `+0x1A0` = speaker, `+0x1BC` = pattern number, `+0x1A8/+0x1A4` string/int, then slot 7 (`ECX=7`, resref at `object+0x270+7*8`); an empty or "default" resref is replaced by `k_hen_dialogue01` (`005709bc PUSH 0x9951B4`). Batch AI7 said `+0x1BC` is the "volume"; gate corrected: pattern number (cmd 195 reads `+0x1BC`).
9. **E9 reach (gate 1.4, verdict).** An ally's shout (pattern 15 GEN_COMBAT_ACTIVE, SILENT_TALK from k_ai_master EndRound, about every 3 s per combatant) reaches a companion only if all of: it is within 35 m (3D); its own perception list has the ally with HEARD (party perception refresh about 4 s, lesson 277); it sees the ally or is within 10 m; it listens (`+0x1C0`) with pattern 15 (set by `k_oei_spawn` / `GN_SetListeningPatterns`; Visas uses `m_spn_visas`, not in the corpus); it is commandable (script gate). All 13 companion dlg scripts call `GN_RespondToShout` under `GetCommandable()`; our installed build routes them to k_ai_master 2004 (lesson 261). Shouts repeat every round, so one miss is retried after about 3 s; delivery is conditional, not reliable for a companion at the far formation edge or one that does not hear the ally. Enemies in range receive the same shout; the R2 E9 design (seen enemy, idle, 25/15 m) is consistent.
10. **Shout abort (code confirmed, reach LOW).** `0054712f MOV [EBP-0x1C],-1`, `005471ac CALL 0x522AF0` (return ignored), `005471b1 CMP [EBP-0x1C],0 / 005471b5 JL 0x5475E8`: zero listeners when the X-bisection meets an unresolvable id; `00547203 JNZ 0x5475E8` drops the rest of the scan. Needs a stale id in `area+0x194`; the same `0x522AF0` serves UpdatePerception, GetNearestCreature, GetFirstObjectInShape and Surrender, so a stale id would also blind perception and scripts. Not observed.
11. **Flip propagation and costs.** No cache: hostility is recomputed at every consumer, nothing to invalidate. Script `ChangeFaction`/`ChangeToStandardFaction` (`0x66F0A0`): AddMember only (O(members) client update unless suppressed), no perception or combat flush; the creature re-evaluates at its next perception update. Effect type 115 and `ChangeFactionByFaction 0x696E10` additionally `ClearAllActions(1)` and `ClearVisibleList 0x5D1510` + `UpdatePerception(0) 0x5D0F30` (O(visible list) + area X-scan; 736 also `PacifyCreature`, O(n^2) for a big faction). Surrender `0x5D51A0`: `CancelCombat` on every hostile creature within 250.0 m (`005d51d9 FLD [0x992F04]`), then joins faction slot 0 ("neutral", `005d5395`); PC refused. `ClearHostileActionsVersus` removes only queued types 0xC/0xF/0x2E (O(q^2)); `CancelCombat` also clears combat state and `round+0xB20`. Dialog start pacifies hostile non-party creatures (O(area)). No event is posted by a faction change.
   Flip mechanisms compared (batch Q4, gate adds nothing):

| mechanism | AddMember | ClearAllActions(1) | perception flush | other |
|---|---|---|---|---|
| ChangeFaction / ChangeToStandardFaction (cmds 173 / 412, `0x66F0A0`) | yes | no | no | PC stats refused; placeable/trigger write the field directly |
| Effect type 115 apply / remove (`0x5951A0` / `0x5953E0`) | yes | yes | yes, unless (`+0x11AC != 0 && vt+0x9C(0) < 1`) or `vt+0x94() != 0` | sets `object+0x258`; refuses PC stats |
| ChangeFactionByFaction / SurrenderByFaction (736 / 737, `0x696E10`) | per creature of the area with faction == from | 736 yes | 737 yes | 736 also SetCombatState(0,1,0) + PacifyCreature |
| Surrender (`0x5D51A0`) | self -> faction slot 0 ("neutral") | CancelCombat on hostile creatures within 250 m | no | PC refused |
| Dialog start (`0x6CF5D0`) | no | yes, non-party creatures hostile (<= 10) to the speaker | no | `0x58AE90` hook |
12. **Costs.** Reputation lookup is O(1): 2x `0x51C690` (hash lookup + 1-5 vcalls), 1x `0x51C100`, 2-3x `0x664650`, 1-2x `0x664580`, ~7 virtual calls (magnitude SPECULATIVE). A 1000 m shout (TALK, SILENT_SHOUT) walks the whole X-window of the area list per call, one 0x34 + 0x18 B allocation per accepted listener and one RunScript per listener.

## Struct fields
| class | offset | type | name | conf | evidence |
|---|---|---|---|---|---|
| CFactionManager | 0x00..0x18 | CSWSFaction* x7 | standard slots (neutral, insane, friendly_2, friendly_1, hostile_1, hostile_2, player) | high | `00664a99..00664ab0`, `00664a18` |
| CFactionManager | 0x1C | int | N (NPC faction count) | high | `006d32a1 MOV [EAX+0x1C],EDX` |
| CFactionManager | 0x20 / 0x24 | CExoArrayList* | reputation matrix `(N+1)*N` / faction list | high | `00664619`, `006644a2` |
| CSWSFaction | 0x00 / 0x04 / 0x0C / 0x10 / 0x18 | int* / int / int / int / CExoString | members / count / id / parent (-1) / name | high | `006d3283`, `0x664680` |
| CSWSCreature | 0x1198 | ptr | stats | high | `005d0d13` |
| stats | 0x78 / 0x7C / 0x6C | int | current faction id / template faction / PC-controlled flag | high / med / med | `006d33c8`, `00576a13` |
| CSWSCreature | 0x11AC | int | party flag read by `0x5D0D00` (`005d0d49`) | high | semantics in AI5 |
| CSWSCreature | 0x1A0 / 0x1A4 / 0x1A8 | id / int / CExoString | last speaker / event int(2) / string(0) of the last shout | high / med | `00570758` |
| CSWSObject | 0x1B0 / 0x1B4 / 0x1BC / 0x1C0 | array / count / int / int | listen patterns / count / last pattern number / listening flag | high | `00546f43`, `0057077d`, `005472c6` |
| CSWSObject | 0x270+slot*8 | CExoString | script resrefs (slot 7 = OnDialogue) | high | `0057095d` |
| CSWSObject | 0x258 | int | faction-effect-active flag (type 115) | med | decomp |
| AOE | 0x288 | id | creator object id | high | `0062ee3f`, `005e1483` |
| server internal | 0x10054 / 0x1F31C / 0x1F320 | ptr / int / int | faction manager / last-shout bookkeeping (low) | high / low | `0x5363A0`, `0x51CD10`, `0x51CD60` |
| global | 0xA0F6E0 | int | reputation override (-1 = off) | high | `005e13c9` |

## Bugs and candidates
| item | verdict | user impact | fix idea | mod item |
|---|---|---|---|---|
| AI-REPTRANSPOSE: AdjustReputation reads the transposed matrix cell | BUG (latent), reach LOW | none in vanilla; mod/module scripts adjusting NPC-vs-NPC asymmetric factions (e.g. PREDATOR 11 / PREY 12, GIZKA_1/2) get a wrong baseline | swap the two pushes at `0x5D0E40` (8 B `8B 55 08 52 8B 45 E8 50` -> `8B 45 E8 50 8B 55 08 52`); low priority | none (enemy-AI only if a mod script uses cmd 209) |
| AI-SHOUTABORT: shout loop drops every listener on an unresolvable id | CONFIRMED code, reach LOW | E9 shouts lost silently in an area with a stale id (would also blind perception) | probe F4+F5 first; if seen, retarget `00547203 JNZ` to the loop head (`0x5471B1`) and seed idx 0 when `0x522AF0` fails | E9, R1 |
| E9 delivery is conditional | CONFIRMED (design) | far-edge companions or ones without the ally HEARD miss a shout; retried each round | script side; decide with the user (behaviour change); runtime check F4/F5 deliveries per round | E9 |
| Shout has no faction filter | CONFIRMED | enemies in range get the same SILENT_TALK / SHOUT event | script `GetIsFriend` filter only | enemy-AI, E9 |
| Hawk standees / non-selected companions use the PC faction row | CONFIRMED (gate-corrected) | none; remap matters only between two flagged members | none | party-flag |
| Global override `[0xA0F6E0]` | CONFIRMED lever | all reputation = value; handy to test E9 / hostility scripts (0 all hostile, 100 all friendly) | DLL data write for tests | enemy-AI testing |
| `k_hen_dialogue01` default OnDialogue | CONFIRMED | a creature with an empty slot 7 still runs a script on every shout it hears | cost note | perf |

Batch said the transposition "matters for any mod script"; gate corrected: only cmd 209 reaches it, only for NPC-faction targets. Targets in the PC faction (PC and every SpawnNPC companion) are not affected (Get(0,fS) recurses to Get(fS,0) = the cell Set(0,fS) writes). Skipped when fT == fS (`005d0e6b`) or fS is not an NPC faction. Vanilla corpus: only `unused_sandper.nss` (oTarget = party leader, so unaffected); module-embedded NCS not scanned.

## Hook sites
Gate-checked windows (section 5; boundary OK, bytes = exe, no relative branch inside, no inbound target inside):
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| AI6-F5 shout receive | 0x57074F | 9 | `8B 95 18 FD FF FF 8B 45 0C` | warm | EventHandler case; `[EBP-0x2E8]` = listener, `[EBP+0xC]` = speaker, `[EBP+0x10]` = payload (int1 = pattern); window start is a jump-table target (allowed) |
| AI6-F4 broadcast entry | 0x547100 | 10 | `55 8B EC 6A FF 68 AB FA 94 00` | warm in fights | ECX = speaker, `[ESP+4]` &string, `[ESP+8]` radius, `[ESP+0xC]` bRequireSeen; pair with F5: the abort shows as 0 deliveries |

Batch-only (not gate-checked; none overlaps installed hooks per the batch):
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| F1 | 0x5E13C0 | 6 | `55 8B EC 83 EC 4C` | hot | GetReputation entry; ECX = T, `[ESP+4]` = S id |
| F2 | 0x5E1671 | 6 | `8B E5 5D C2 08 00` | hot | consumed exit, EAX = result may be rewritten before `RET 8` |
| F3 | 0x67C430 | 6 | `55 8B EC 83 EC 34` | warm | GetIsEnemy/Friend/Neutral (ids 0xEB/EC/ED) |
| F6 | 0x6D3330 | 6 | `55 8B EC 83 EC 2C` | cold | faction AddMember (flip log) |
| F7 | 0x5E1680 | 6 | `55 8B EC 83 EC 1C` | hot | GetAIStateReputation entry |
| F8 / F9 | 0x693860 / 0x63A5B0 | 10 / 10 | `55 8B EC 6A FF 68 58 AC 95 00` / `55 8B EC 6A FF 68 98 86 95 00` | cold | SpeakString handler / DeliverSpeech |
| F10 / F11 / F12 | 0x5D51A0 / 0x5951A0 / 0x66F0A0 | 6 each | `55 8B EC 83 EC 7C` / `55 8B EC 83 EC 3C` / `55 8B EC 83 EC 64` | cold | Surrender / effect 115 apply / ChangeFaction |
Data lever (not a code hook): `[0x00A0F6E0]` int, -1 = disabled.

## Open items
- repute.2da content and column 0 meaning (LABEL vs PLAYER), CreateDefaultFactions slot order vs nwscript ids (harmless unless the 2DA is missing): probe P5 dump of `data[row*N + col - 1]` at module load.
- Same-tick vs next-tick delivery of the shout event (EventPending clock semantics, AI4); `+0x1F31C/+0x1F320` readers (`0x51CD10/0x51CD60`).
- `vt+0x94` / `vt+0x9C(0)` names used by the effect-115 flush; `0x6A34A0` pattern language; `0x575600` kill-reward branch (`Get(faction,0) < 0xB`).
- Whether `area+0x194` holds stale ids in practice (shout abort reach); module-embedded NCS scan for cmd 209 (AdjustReputation).
- `m_spn_visas` listening setup (module script, not in the corpus). E9 behaviour change: decide with the user; runtime check = F4/F5 deliveries per round versus companions within 35 m, HEARD-bit share.
- Reputation consumers (batch, thresholds quoted from asm), for reach questions: enemy (`<= 10`): `0x576DC0` (`005776f2..0057770c`, only for `+0xFF5` in {0x12, 0x13} creatures; calls `0x57E280`), `0x57AC30` BroadcastCombatStateToParty (`0057acb0`, `0057aef5`), `0x57BB90`, `0x6DA4F0` CreateNewAttackActions (`006da69f`, non-hostile target -> no attack), `0x5D51A0`, `0x6CF5D0`, `0x6FC5D0` AvoidCreature (`006fc806`, `006fcecd`, `006fd0c2`), `0x683D40`, `0x67C430`, `0x5E1680` callers `0x57C990`, `0x568540`, `0x585480`, `0x634B20`, `0x6424B0`, `0x6430A0`, `0x642BA0`. "Not friendly" (`< 90`): `0x56C310` UpdateMineCheck, `0x5DB0A0` OpenDoor, `0x5DB8F0` UnlockObject, `0x5DEC20` DoorSaber, `0x5DFBE0` UseObject, `0x5F2FE0`, `0x617B40`, `0x63DD30`, `0x6430A0`. Others: `0x586940` GetIsCreatureBumpable (`rep >= 10`), `0x6D3FF0` GetFactionAverageReputation, `0x575600` (called from `0x597D90`).
- Runtime probes proposed by the batch (observation-only): P1 which volume / radius / listeners per shout in a real fight (F4 + F5; zero deliveries with a non-empty area = the `0x522AF0` abort); P2 whether shouts reach allies > 35 m in the formation (expected: only inside 35 m and seen or < 10 m); P3 reputation call rate and value histogram (F2 exit `0x5E1671`; `[0xA0F6E0] == -1` always in vanilla); P4 which scripts/effects flip factions in a session (F6 `0x6D3330`); P5 one-time matrix dump at module load (`fm = [[[0xA1B4A4]+8]+4]+0x10054`, `N = [fm+0x1C]`, `data[row*N + col - 1]` next to repute.2da; expect size `(N+1)*N`); P6 reputation of a `+0x11AC == 0` companion versus enemies (F1 filtered on a companion; gate: expect the PC faction row).

## Mod relevance
| item | served by |
|---|---|
| E9 | key behaviours 6-10: radii, reach conditions, retry each round, no faction filter, abort |
| party-flag (`+0x11AC`) | `0x5D0D00` remap; flag-0 companions judged through the PC faction row |
| enemy-AI, hold_enemies research | override `[0xA0F6E0]`; enemies hear SILENT_TALK too; GetIsFriend script filter is the only gate |
| a1_retarget_strict, A1, M8 | `GetAIStateReputation <= 10` consumers (`0x57C990`, `0x568540`) |
| C1 | `ClearHostileActionsVersus` removes queued actions only (0xC/0xF/0x2E); dialog-start pacifier |
| FM1 | `GetIsCreatureBumpable` uses `rep >= 10` (AI1) |
| perf | per-shout O(n) area scan and one RunScript per listener; reputation lookup O(1) |
| AI-REPTRANSPOSE | none unless a mod script uses AdjustReputation |
