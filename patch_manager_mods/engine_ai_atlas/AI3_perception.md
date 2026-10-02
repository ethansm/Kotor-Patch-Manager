# AI3 Perception, LOS and stealth (KOTOR II Steam swkotor2.exe 6a522e71)

Pipeline position: server tick -> `CServerAIMaster::UpdateState 0x51D790` -> `CSWSCreature::AIUpdate 0x569320` -> `SpawnInHeartbeatPerception 0x56B8A0(1)` (once per AIUpdate, `005694e3`) -> `UpdatePerception 0x5D0F30(mode)` -> per (perceiver P, target T) `DoPerceptionUpdateOnCreature 0x576DC0` -> LOS ray `0x52BB60` + spot `0x578850` / listen `0x577FA0` rolls -> perception list `creature+0x1028` update -> SEEN/HEARD/VANISHED/INAUDIBLE events posted (`0x51E170`, delta 0, event id 10, payload type 1) -> `EventHandler 0x56EFD0` case 1 -> script slot 1 ON_NOTICE. Perception is the aggro and stealth gate: enemies notice the party through mode 1 (every AIUpdate for non-party creatures, because the mode-1 stamp is never stored, P1); companions refresh through blocks 3 (mode 0 / 2). Stealth is a cached-dice roll compared against skill ranks, not a per-tick roll.

## Status
- Sources: batch `findings/AI3.md` (Sonnet) + `GATE_wave1.md` (section 1 AI3 verdicts, 1.2 spot sign verdict, 3 exceptions, 4 names, 5 windows P2 and AI3-B1, 6 lessons 6/7) + `LEAD_SPOTCHECKS.md` (AI3: spot/listen asm lines, E3 `00552dd2/00552dd8`, P1 field census confirmed).
- Date: 2026-10-02. Static only. Gate-verified: spot sign BUG, R1 scan-abort reach (modes 0/2 only), 176 calls/s mechanism, P1 missing store, `+0xFF5` correction, DB name corrections, E3 (med).
- Full struct set in DB table `struct_fields`.

## Function table
| addr | name | conf | purpose |
|---|---|---|---|
| 0x56B8A0 | CSWSCreature::SpawnInHeartbeatPerception(mode) | high | 3 timed blocks: spawn+heartbeat, mode-1 party scan, mode 0/2 refresh + AI-level drop; callers `0x5694E3` (mode 1), `0x56F022` (mode 0) |
| 0x5D0F30 | CSWSObject::UpdatePerception(mode) | high | mode 0 all, 1 party, 2 non-party; refresh list then discover via sorted-X scan (`005d1287 CALL 0x00522af0`) |
| 0x576DC0 | CSWSCreature::DoPerceptionUpdateOnCreature(P=this, T=arg) | high | dice reroll, LOS, seen/heard decision, list update, 6 event posts (`0x5778A7..0x577E36`) |
| 0x577ED0 | DoSeenHeardChecks (DB: DoStealthDetection) | med | seen = spot only if LOS; heard = listen regardless |
| 0x577FA0 | CSWSCreature::DoListenDetection | high | hearing roll; result store `0057883f` |
| 0x578850 | CSWSCreature::DoSpotDetection | high | seeing roll; contains the sign bug; result store `00579021` |
| 0x585420 | CSWSCreature::GetIsBlindOrInDarkness | med | 1 iff `+0xFF4 & 0x10` or (`&8` and not `&2`); batch said IsBlindOrDark; DB name BroadcastAttackOfOpportunity wrong; gate named |
| 0x5802C0 | CSWSCreature::GetDetectModeState | med | `(+0xD8 == 10000 && +0x510 != 1) ? 2 : +0x510`; DB name UpdateAppearanceForEquippedItems_2 wrong |
| 0x585480 | CSWSCreature::GetInvisible(T, P) | med | scans T effects for invisibility / sanctuary-style, 2-bit cache in entry |
| 0x58B140 | GetSpotCheckDistance (PrimaryRange) | high | `+0x101C`, or 2DA row 0x12 when `+0x520 != 0` and not controlled (`0058b1e0`) |
| 0x58B210 | GetListenCheckDistance (SecondaryRange) | high | `+0x1020`, same override |
| 0x5D1400 | AddToVisibleList / AddPerceptionEntry(id,seen,heard,invis,hidden) | med | `new(8)` entry, bit packing (gate name_map row 0x5D1400, med) |
| 0x5D15F0 / 0x5D1510 / 0x5D1750 | Remove / Clear / Find perception entry | high | linear scan of `+0x1028/+0x102C` |
| 0x522AF0 | CSWSArea::GetFirstObjectIndiceByX | high | binary search of `area+0x194/+0x198`; returns 0 on an unresolvable id (R1 abort) |
| 0x52BB60 | CSWSArea::ClearLineOfSight(...) ret 0x20 | high | door AABB test then room-mesh rays; creatures never enumerated |
| 0x52D130 | CSWSArea::LosObjectBoxTest | high | loops `area+0x48` doors; ray via `0x552D90` |
| 0x52B9C0 | DoorBlockSightRetest (area vt+0x18) | med | door 2DA `BlockSight`; if 0 and arg != 0 exclude door and re-test |
| 0x552D90 | DoorOrPlaceableRayTest | high | types 9 / 10 only (`00552dd2 CMP [EBP-0x18],0x9` / `00552dd8 CMP ..,0xa`), else 0 |
| 0x5FF950 | RoomMeshRayClear | med | ray vs room walkmesh unless `room+0x38 != 0` |
| 0x5867E0 | ClearHostileActionsVersus | med | called when T becomes hidden to P (`005773ae`) |
| 0x53DFB0 | GetObjectByIdHash (returns 0 if FOUND) | high | `[0x99447F]` = 0 |
| 0x51AD20 / 0x51AF60 / 0x51AFE0 | GetCurrentTime / TimeDiff / TimeCompare | high | `/1000`; TimeDiff outputs only valid when now >= stamp |
| 0x51E170 | CServerAIMaster::AddEventDeltaTime | high | event posts (gate rename) |
| 0x6B7BB0 | CSWSCreatureStats::GetSkillRank(skill,vs,flag) | high | skill 2 Stealth, 3 Awareness |
| 0x6BADA0 / 0x78EB80 | stubs (return 0 / return 1) | high | listen extra score term always 0; "sound path" test always 1 |
| 0x6AFED0 | stats LoadFromGff | med | PerceptionRange default 0xB, PERCEPTIONDIST, BlindSpot |
| 0x57E280 | (unresolved; DB name SetLastInstantEquip does not fit) | low | called for `+0xFF5 in {0x12,0x13}` near a non-hostile T; not read; gate demoted |
| 0x59CBA0 | CSWSEffectListHandler::OnApplySetState | med | writes creature `+0xFF5` = SetState id (gate); explains the `{0x12,0x13}` tests |

## Spot / listen formula (decoded constants)
Constants from the vanilla exe (gate): `[0x98B968]` = 3.0 (double), `[0x987490]` = 2.0 (double). Eye offsets: spot 1.0 (`[0x986B50]`), LOS/listen rays 1.5 (`[0x989F10]`).
| term | spot `0x578850` (spotted iff margin `[EBP-0x48] > 0`, `0057901b CMP [EBP-0x48],0` / `0057901f JG`) | listen `0x577FA0` (heard iff score > 0, `00578839 CMP [EBP-0x1c],0 / JG`) |
|---|---|---|
| gates | `GetIsBlindOrInDarkness(P)` -> 0; hidden flag -> 0; `d^2 > SpotRange^2 && T+0x260 == 0` -> 0; `P+0xFF4 & 4` -> 1 | `T+0xFD0` (silenced) -> 0; hidden/blind: MaxAttackRange^2 < d^2 -> 0; `ListenRange^2 < d^2` -> 0 |
| rank | `GetSkillRank(T, 2, vs P)`; `rank == 0` or `T+0x511 == 0` -> 1 (seen) | same -> 1 (**heard through walls anywhere inside ListenRange**) |
| target stealth | `-rank` (`00578ab5 SUB EDX,[EBP-0x54]`), `- T+0x1024` (`00578ad4`) | `-rank`, `- T+0x1025` |
| perceiver | `+Awareness` full if detect mode in {1,2} else /2 (`00578b37 ADD ECX,[EBP-0x40]`), `+P+0x1026` (`00578b9b`) | same with `+0x1027` and `+ area+0x190` (ModListenCheck, `0x578639`) |
| moving / combat / cone | moving `-5` (`00578c19`), blind-spot cone `-10` (`00578e9c`), P in combat `-10` (`00578ec7`), anim -5/+5 | moving `-5`, in combat `-10`, anim -5/+5, size class from `T+0x53C` (1:-8, 2:-4, 4:+4, 5:+8) |
| distance | `d3 = dist3D(P,T)/3.0` (`005142f0`, `FDIV [0x98b968]`); if `d3 > 2.0`: `n = ftol(d3-2.0)` (`FSUB [0x987490]`, `0x0091c860`), then `00579007 NEG EAX`, `00579009 MOV [EBP-0x10],EAX` (= -n), **`0057900f SUB ECX,[EBP-0x10]` = score += n** | `- ftol(d/3.0)` (`005787ab FDIV [0x98b968]`, `005787b9 SUB EDX,EAX` = **subtract**) |
| obstruction | none (LOS ray decides before) | two rays no excluded ids, door/mesh thickness: unnoticed `-5*ftol(sqrt(thick))`, noticed `-2` |
Bug (gate 1.2, CONFIRMED): `[EBP-0x48]` is the spotter's margin (not a DC) and every other adverse term subtracts, so the distance term is a double negation. Beyond 6 m a stealthed target is EASIER to spot by up to `PrimaryRange/3 - 2` (swing 2n, 8 points at 18 m). Only reached for T in stealth mode with Stealth rank > 0 (`0x578A37/0x578A5F`). Fix: KPM simple 1-byte hook at `0x57900F` (`2B 4D F0` -> `03 4D F0`, `ADD ECX,[EBP-0x10]`; flags rewritten at `0x579015`). Balance change (stealth stronger at range): user decision. Listen is NOT affected. Batch said "bug intent SPECULATIVE"; gate confirmed BUG.

Dice (gate-agreed): re-rolled only when >= 20000 ms (`00576ed7 CMP [EBP+0xffffff7c],0x4e20`): `+0x1024/+0x1025 = rand%10+11`, `+0x1026/+0x1027 = rand%20+1` if detect mode in {1,2} else `rand%10+1`. A SEEN entry is never re-rolled while LOS holds.

## Key behaviours
1. Perception entry = `new(8)` (`005d1400`): `+0` id, `+4` bit0 SEEN, bit1 HEARD, bits2-3 invis cache (0 unknown, 1 hidden, 2 saw through), bit4 HIDDEN. List at creature `+0x1028/+0x102C`, linear lookup (`0x5D1750`).
2. Event codes (payload type 1, caller = T, target = P, delta 0): 0 HEARD (`0x577AF9`/`0x577DF1`), 1 INAUDIBLE (`0x577862`), 2 SEEN (`0x577BDA`/`0x577CE9`), 3 VANISHED (`0x577950`). Consumer `0x56EFD0` case 1 sets `+0x100C/+0x1010/+0x1014/+0x1018` and runs slot 1 (`+0x278`); party members only if in the party table. Posts are gated by `+0x11C8 == 0`.
3. P1 CONFIRMED: the mode-1 stamp `+0x3A0/+0x3A4` has no creature-side writer except the ctor (`00562366/00562376`). Reads `0056bcf9/0056bd06`, interval `0056bdb4 CMP [EBP-0x28],0xfa0`; only store after `0056bdc5 CALL 0x005d0f30` is `0056bde3 MOV [EAX+0x39c],EDX`. `[EBP-0x28]` = TimeDiff(now, stamp) = ms-of-day, so the 4000 ms test passes (batch: fails only in the first 4 s of each game day; gate: ">= 4000 all day"). Result: every non-party, non-IsPC creature runs mode 1 once per AIUpdate. Missing store: `[c+0x3A0] = [EBP-0x24]`, `[c+0x3A4] = [EBP-0x20]`. Shipped fix `0x56BDB4/0x56BDE3` (INI `p1`).
4. 176 calls/s = 4 non-party x 44 AIUpdates/s: mechanism CONFIRMED by gate (counted site `0x56BDB4` only past party skip `0056bd5b` and IsPC skip `0056bdac`); 44/s = frame rate equation SPECULATIVE.
5. Cadence per AIUpdate (blocks of `0x56B8A0`): block 1 heartbeat/spawn 3000..4199 ms (`0056ba03`); block 2 mode-1 party scan: party skipped, IsPC non-party 300..699 ms (`0056bd8b`), others 4000 ms test; block 3 mode 0/2: IsPC 200 ms (`0056be70`), others 4000 ms (`0056bea2`), level-0 creatures only after >= 150 AIUpdates (`0056bee8 CMP [ECX+0x3d0],0x96`). Counters are tick counts, not seconds.
6. Block 3 tail: if the area has no players (`0056bfc2`, `area+0x12C == 0`) and `+0x78 != 0` -> `0x51E120` level 0 (`0056c01c`): FM7 drop, party member toggles 4 -> 0 here and back to 4 in AIUpdate (`005694d6`); also one FM3 self-transition source.
7. R1 scan abort CONFIRMED but narrow: `0x522AF0` returning 0 skips only the new-object discovery of modes 0/2 (`005d128e JNZ 0x005d1295 / 005d1290 JMP 0x005d13ec`, also skips the `+0x11C8 = 0` reset `005d13e2`). Mode 1 (enemy notices party) takes the `param_2 == 1` branch, walks the party table, never calls `0x522AF0`. Enemy aggro on the party is not blockable by it. Existing-list refresh runs before the scan.
8. E3 (med, absence over callees): `0x52BB60` tests only door AABBs (+ `BlockSight` 2DA) and room-mesh rays; creatures are never enumerated, so allies/enemies/leader never block LOS. Doors/placeables only via `0x552D90` (types 9/10).
9. LOS call (`005771d5..005771fd`): P is the controlled creature (`0x51C8F0`) -> A = 1 with no ray (`00577124`); else if `d^2 <= max(spot,listen)^2` or `T+0x260 != 0` -> ray. Listen rays use `0x7F000000` ids and `p8 = 0`.
10. Party-to-party perception is forced (`0x577663 CALL 0x007e5c40` twice): `A=1, H=0, seen=heard=1`.
11. Seen-stays-seen: spot evaluated only when the entry is NOT seen; sneaking after being spotted does nothing until LOS breaks or H forces a drop (X3).
12. In combat (`+0x520 != 0`) any non-controlled creature (also companions) uses 2DA row 0x12 for Primary/Secondary range (`0058b1de PUSH 0x12 / 0058b1e3 CALL 0x0071b670`), costing a CExoString + `_atof` per call, ~6 per (enemy, party) pair per tick (`0x5D0F30` 2, `0x576DC0` 2, spot 1, listen 1) (X6, magnitude low).
13. `ModSpotCheck` (area `+0x18C`, `00524674`) is loaded but never read by the spot check (med, absence); `ModListenCheck` (`+0x190`) is read at `0x578639`.
14. `+0xFF5` is the current (highest) SetState id (`0x59CBA0`), NOT a "creature class byte / script package" (batch said class byte; gate corrected, med). `0x56BB65` switch and `{0x12,0x13}` tests are state tests.
15. Newly SEEN sets `+0x11CC = 6000` (`00577ea6 MOV [EDX+0x11cc],0x1770`) only for PC-flag stats (`+0x6C`) or the active leader; countdown at `0x56A142..0x56A21E` (name SPECULATIVE).

## Struct fields
Full set in DB table `struct_fields`.
| class | offset | type | name | conf | evidence |
|---|---|---|---|---|---|
| CSWSObject | 0x78 | int | AI level | high | `0056ba45`, `0x51E120` |
| CSWSObject | 0x260 | int | ForceAlwaysUpdate (bypass range) | high | `005744c3`, `005771a5`, `005789be` |
| CSWSCreature | 0x270+8*n | slot | script slots (0 heartbeat, 1 notice, 6 EndRound, 8 spawn) | high | `lea ecx,[eax+edx+0x270]` |
| CSWSCreature | 0x38C | int | first-run done flag | high | `0056c042` |
| CSWSCreature | 0x390/0x394, 0x398 | day,ms / ms | heartbeat stamp / interval 3000..4199 | high | `0056ba03` |
| CSWSCreature | 0x39C | ms | mode-1 interval 300..699 (IsPC non-party) | high | `0056bdd7`, `0056bde3` |
| CSWSCreature | 0x3A0/0x3A4 | day,ms | mode-1 stamp, never stored after ctor (P1) | high | census: ctor only |
| CSWSCreature | 0x3BC/0x3C0 | day,ms | dice-reroll stamp | high | `00576eb6`, `00576fde` |
| CSWSCreature | 0x3C8/0x3CC, 0x3D0, 0x3D4 | stamp / counters | block-3 stamp / block-3 counter / heartbeat counter | high | `0056bf96`, `0056bee8` |
| CSWSCreature | 0x510 / 0x511 | u8 | detect mode / stealth mode | high | `005b7abf` (`+0x511`) |
| CSWSCreature | 0x520 | int | in-combat flag | high | `0057a634` |
| CSWSCreature | 0x53C | int | size class 1..5 | med | `005787c1` |
| CSWSCreature | 0xFD0 | int | silenced | high | `0x5A2E6B` |
| CSWSCreature | 0xFF4 | u8 | sight flags (bit names SPECULATIVE) | med | `00585429`, `005789e0` |
| CSWSCreature | 0xFF5 | u8 | current SetState id | med | gate: `0x59CBA0` |
| CSWSCreature | 0x1008..0x1018 | int x5 | last perceived object, heard, seen, inaudible, vanished flags | high | `0x56EFD0` case 1 |
| CSWSCreature | 0x101C / 0x1020 | float | PrimaryRange (spot) / SecondaryRange (listen) | high | `0058b16b`, `0058b23b` |
| CSWSCreature | 0x1024..0x1027 | u8 x4 | stealth dice x2 (11..20) / spot, listen dice | high | `00576f1b..00576fc9` |
| CSWSCreature | 0x1028 / 0x102C | ptr / count | perception list array / count | high | `005d1750` |
| CSWSCreature | 0x10E8 / 0x11AC / 0x11BC | int | IsPC cache / party flag / currently moving | high | `0056bd67`, `0056bd48`, `00578c2e` |
| CSWSCreature | 0x11C8 / 0x11CC | int | suppress perception events / excited timer (name low) | high / med | `005d13e2`, `00577ea6` |
| CSWSCreatureStats | 0x6C / 0x84 | int / float | IsPC / BlindSpot | med | `00578bc7`, `00578df6` |
| perception entry | +0 / +4 | u32 / bits | id / seen,heard,invis cache,hidden | high | `0x5D1400` |
| CSWSArea | 0x12C | int | players in area | high | `0056bfc2` |
| CSWSArea | 0x18C / 0x190 | int | ModSpotCheck (unread) / ModListenCheck | high | `00524674`, `0x578639` |
| CSWSArea | 0x194 / 0x198 | ptr / count | all-objects id array sorted by X | high | `00522b50` |
| CSWSArea | 0x250 / 0x254 | count / ptr | rooms (0x4C each) | high | `0052bd3e` |
| time object | 0x38 / 0x3C | u8 / u32 | minutes-per-hour / ms per day (default 7,200,000) | high | `0x51A800` |

## Bugs and candidates
| item | verdict | user impact | fix idea | mod item |
|---|---|---|---|---|
| Spot distance sign | BUG (gate) | sneaking party is easier to spot from > 6 m; up to `PrimaryRange/3 - 2` points | 1 byte at `0x57900F` `2B` -> `03` (AI3-B1, KPM simple hook); user decision (balance) | spot sign (B1), enemy-AI |
| P1 missing store (mode-1 stamp) | CONFIRMED | non-party creatures perceive every AIUpdate instead of every 4 s; LOS rays + 2DA reads per tick | store `+0x3A0/+0x3A4` after `0056bdc5` (shipped `0x56BDE3`) | P1 |
| P2 companion refresh cadence | candidate | IsPC stays on the Exile, companions 0.25/s | AI3-H1 window `0x56BE79` (HOT) | P2 |
| R1 scan abort | CONFIRMED, narrowed | affects companion refresh and enemy-vs-non-party discovery only; enemy aggro safe | none; R1 downgrade | R1 |
| E3 creatures never block LOS | CONFIRMED static, med | allies/enemies do not block ranged/perception LOS | none | E3, A1, Mod 8 |
| `ModSpotCheck` ignored | CONFIRMED, med (absence) | ARE value has no effect on spotting | none | new item |
| Seen-stays-seen / fixed 20 s dice | CONFIRMED | failed roll stays failed up to 20 s | none | enemy-AI |
| Row-0x12 range override in combat + 2DA re-reads | CONFIRMED, magnitude low | companions see at row-0x12 range in combat; per-tick string/float parse | none proposed by the batch | Mod 8, enemy-AI, perf |
| FM7 block-3 AI-level drop (`0x56C01C`) | CONFIRMED | level 4 -> 0 in player-less area, re-promoted in AIUpdate | see AI4 | FM7, FM3 |

## Hook sites
Gate-checked:
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| P2 (AI3-H1) | 0x56BE79 | 6 | `8B 85 2C FF FF FF` | HOT (every AIUpdate of every creature) | refresh-cadence window; start is a target of `JZ/JNZ 0x0056be79` (`0x56BE68/0x56BE6E`, allowed); keep handler to a few compares; 0x90 bytes from shipped `0x56BDE3` |
| AI3-B1 | 0x57900F | 1 (simple patch `2B` -> `03`) | `2B 4D F0` insn | cold | spot sign fix; no detour; user decision |

Batch-only (not gate-checked; no relative branch inside, no inbound target, installed list has no hooks in these ranges):
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| AI3-H2 | 0x5771D5 | 7 | `6A 00 6A 01 8B 45 A8` | hot | LOS probe (RP2); `[ebp+0xfffffe54]` P, `[ebp+8]` T |
| AI3-H3 | 0x579021 | 7 | `C7 45 D0 00 00 00 00` | cold | spot-fail probe (RP3); `[ebp-0x48]` score |
| AI3-H4 | 0x57883F | 7 | `C7 45 D0 00 00 00 00` | cold | listen-fail probe (RP4); `[ebp-0x1c]` score |
| AI3-H5 | 0x5D0F30 | 9 | `55 8B EC 81 EC C4 00 00 00` | hot | UpdatePerception entry (RP1) |
| AI3-H6 | 0x576DC0 | 5 | `55 8B EC 6A FF` | hot | DoPerception entry |
| AI3-H7 | 0x58B140 | 5 | `55 8B EC 6A FF` | hot in combat | 2DA re-read counter (RP5); returns float in ST0 |
Informational (shipped): `0x56BDB4` (7), `0x56BDBD` (8), `0x56BDE3` (6). R1 sites `0x5D1290` and `0x522B7F` are unsuitable (relative `JMP`, `EB 5B`); use RP6.

## Open items
- 2DA data not in the dump: Primary/SecondaryRange values (rows 0xB..0x12), `PERCEPTIONDIST`, door `BlockSight`, IFO MinPerHour.
- `+0xFF4` bit names (see-invisible/ultravision/true-seeing/darkness/blind SPECULATIVE); state-id -> effect map for ids 1, 2, 4-6, 9, 0xB-0x13 (gate); `0x57E280` purpose.
- `0x58AE90`/`+0x3F8` (PC "current action" -5), `0x540FD0` anim set L, `+0xD8 == 10000`, `0x5E2540/0x579800` (spot feedback).
- Same-tick vs next-tick delivery of perception events (`0x51D790` `EventPending` loop); delta-0 posts can cut the level pass (see AI4).
- Reachability of an unresolvable id in `area+0x194` (probe RP6); area `vt+0x18` signature vs `0x52B9C0`.
- Runtime probes RP1-RP7 (batch section c) not run; the 44 AIUpdates/s = frame rate equation stays SPECULATIVE (gate).

## Mod relevance
| item | served by |
|---|---|
| spot sign (B1, new) | spot distance sign bug, `0x57900F` |
| P1 | missing stamp store; shipped `0x56BDB4/0x56BDE3` |
| P2 | block-3 cadence, window `0x56BE79` |
| R1 | scan-abort narrowed to modes 0/2 |
| E3 | creatures never block LOS (med) |
| FM7, FM3 | block-3 tail `0x56C01C` level drop |
| enemy-AI, enemy-E7 (`hold_enemies`) | stealth formula, OnNotice event timing, dice persistence |
| Mod 8 (kite) | combat-state range override (row 0x12) |
| perf, AIQ | mode-1 per-tick cost, 2DA re-reads in combat |
| party-flag (`+0x11AC`) | block 2 skip, block 3 mode 0 vs 2, ON_NOTICE party-table test |
| C1 | hidden flag -> `ClearHostileActionsVersus` (`005773ae`) |
| medpac, T1, EFFOVF, FM1-FM6, L1-L3, M5, E4, E9, PFLK | none |
