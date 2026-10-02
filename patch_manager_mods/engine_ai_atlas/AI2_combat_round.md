# AI2 Combat round and attack pipeline (KOTOR II Steam swkotor2.exe 6a522e71)

Pipeline position: server tick -> `CServerAIMaster::UpdateState 0x51D790` -> `CSWSCreature::AIUpdate 0x569320` -> (perception) -> `UpdateCombatRoundTimer 0x571030` (`00569578`) -> `RunActions 0x5D56B0` (`005695b1`) -> action 0x3F `AIActionCombat 0x6D9500` / action 0xC `AIActionPhysicalAttacks 0x6D7390` -> approach via pathfinding (AI1) or resolve attacks (`0x6DACF0`) -> deferred damage events (`0x6DCBE0` from the round timer) -> events/effects back to creature scripts (slot 3 OnAttacked, slot 4 OnDamaged, slot 6 EndRound). The combat round (`CSWSCombatRound`) is a separate 0xB2C-byte heap object at creature `+0x10DC` (`0056223f PUSH 0xb2c`, `00562290 MOV [EDX+0x10dc],EAX`), so all `+0xA84..+0xB28` offsets are round-relative. One `AIActionPhysicalAttacks` call resolves all attacks of the round and returns 2; damage is delivered later when the pause timer runs out. Target choice is script-driven for everybody except the player-controlled leader.

## Status
- Sources: batch `findings/AI2.md` (Sonnet) + `GATE_wave1.md` (section 1 AI2 verdicts, section 3 confidence exceptions, section 4 names, 6 lessons 11/12) + `LEAD_SPOTCHECKS.md` (AI2: third slot-6 site, deferred-damage callers, leader-only pick, round allocation all CONFIRMED).
- Date: 2026-10-02. Static only. Gate-verified: round allocation, attack/approach metric, retarget metric, slot-6 site 3 (and that it fires mid-combat for party members), `+0xA9C/+0xAB8`. Q2 rows 1, 8, 9 high (gate). The hook catalogue (AI2-*) was NOT part of the gate window check except installed E2 `0x569A08`.
- Full struct set is in the DB table `struct_fields`.

## Function table
| addr | name | conf | purpose |
|---|---|---|---|
| 0x58E230 | CSWSCombatRound::Constructor | high | 7 attack slots of 0x17C, 2 id lists, action list `+0xB00` (`0058e262 PUSH 0x7`, `0058e264 PUSH 0x17c`) |
| 0x58E730 | StartCombatRound(target,engaged,master,word,noScript) | high | arms round: `+0xA84=1, +0xA94=0, +0xAA8=0, +0xAB0=0, +0xA9C=3000` (`0058e8da`) |
| 0x58E990 | EndCombatRound(bRunScript) | high | ends round, runs slot 6 (`0058f50f`), leader CreateNewAttackActions (`0058f5d1`) |
| 0x58F7A0 | IncrementTimer(dt) | high | `timer += dt`; every expired round ends via `0x58E990(1)` |
| 0x58FB40 | DecrementPauseTimer(dt) | high | `+0xAB0 -= dt`, unpause unless `+0xAB4` (infinite) |
| 0x58FC60 / 0x58FCD0 | SetRoundPaused / SetPauseTimer | high | `+0xAA8/+0xAAC` / `+0xAB4/+0xAB0` |
| 0x58FD50 | DecrementRoundLength(ms,bExtend) | high | shrinks remaining budget `+0xA9C`, rescales timer and queued action times |
| 0x571030 | UpdateCombatRoundTimer | high | per-AIUpdate: increment or pause countdown, sub-attack delivery -> SignalDamage (`005711a8`) |
| 0x5713F0 | ResetCombatAnimations(dt) (DB) = "end pause with leftover" | med | SetRoundPaused(0), SetPauseTimer(0,0), IncrementTimer(leftover) |
| 0x590FF0 / 0x5912B0 | ResolveEngagedState / ResolveMasterState | high | party always master (`+0x11AC`) |
| 0x590A50 | CSWSCombatRound::AddAttackAction | med | `new(0x88)` + ctor `0x58C7D0`; duration `(DAT_00a7e0b0+3000)/2` = 1500 ms (gate new row, med) |
| 0x5905F0 | CSWSCombatRound::CalculateOnHandAttacks | med | `+0xAE0 = 0x6A7170() + min(+0xAF4,2) + bonuses` (gate row, med) |
| 0x6A7170 | GetBaseAttacks stub | high | `MOV AL,1; RET` |
| 0x57A560 | SetCombatState(state,kind,flag) | high | `+0x520`, `+0x524 = 8000/0`; state 0 -> `EndCombatRound(0)` (`0057a8f5`) |
| 0x57AC30 | BroadcastCombatStateToParty(otherId) | high | SetCombatState(1,..) on self and faction mates in range (3D) |
| 0x57C990 | GetNearestVisibleEnemyInRange(range,exclude,ref) | high | retarget picker, 3D length minus both `PI+8` radii (`0057cd80`, `0057cdc0`); gate-verified |
| 0x571570 | MaxAttackRange(tid,duel,flag) | high | melee `Desired+0.5` / ranged baseitem `+0x34` (0 -> 30.0) or 20.0 |
| 0x5716A0 | DesiredAttackRange(tid,flag) | high | 1.6 + HITRADIUS(att) + HITRADIUS(tgt); 1.4 if flag |
| 0x585F10 | CSWSCreature::GetUseRange (DB) | med | door/placeable use point |
| 0x6D7390 | AIActionPhysicalAttacks (action 0xC) | high | whole attack action (gate: 2D centre test, LOS `0x52BB60`, `+0x558` memory) |
| 0x6D9500 | AIActionCombat (action 0x3F) | high | pops round actions `+0xB00`; refuses a 2nd action while `+0xAB8 >= 1` (`006d96cc`) |
| 0x568540 | AddAttackActions (ret 60) | high | round queue + one 0x3F, or one 0xC |
| 0x6DA4F0 | CreateNewAttackActions(bDead,bSpell,prevTid) | high | controlled leader only (`0058f586 CMP [EBP-0x24],0`) |
| 0x6DAC70 | InitializeNewAttackTarget | high | `+0x38=tid; +0x88=tid; +0x52C=1` (`006dacdb`) |
| 0x6DAB20 | EndAttackAction | high | forces arg 1, SetCombatMode(0,1); does not touch round or `+0x524` |
| 0x6DACF0 | ResolveAttack(tid,p2..p7) | high | `+0x548 = tid`, ranged `0x6DAF00` vs melee `0x6DEA90` dispatch |
| 0x6DEA90 | ResolveMeleeAttack | high | loop over all attacks: roll, damage, sub-attack, anims |
| 0x6E0B10 | CSWSCreature::ResolveAttackRoll | med | d20 hit roll, result `attack+0x5C`; gate row med (batch high) |
| 0x6A2000 | CSWRules::RollDice(n,sides) | med | `PUSH 0x14/PUSH 1` at `006e0bd1` |
| 0x6A93A0 / 0x6A5110 | attack bonus / defense (AC) | low | names unproven, bodies unread (gate exception) |
| 0x6DCBE0 | SignalDamage | high | deferred delivery; only callers `005711A8/005711F1` (lead confirmed) |
| 0x6E0650 / 0x6DDB00 | SignalMeleeDamage / SignalRangedDamage | high | post events 0xF, 5, 0x16, 0x13 |
| 0x5959E0 | CSWSEffectListHandler::OnApplyDamage | med | runs OnDamaged slot 4 directly (`005966d0`), stagger |
| 0x59E860 | CSWSEffectListHandler::OnRemoveSetStateInternal | high | slot-6 site 3 (effect type 9 removal, `0059e904`); gate: up from med |
| 0x59CBA0 | CSWSEffectListHandler::OnApplySetState | med | creates the type-9 effect, writes creature `+0xFF5` (gate new row) |
| 0x5867E0 | ClearHostileActionsVersus (C1) | high | `005868b9/0x5868F1/0x586929 MOV [EBP-0x10],0` |
| 0x56EFD0 | CSWSCreature::EventHandler | high | event 0xF -> slot 3 (`0056faa1`) |
| 0x569320 | AIUpdate (context) | high | calls `0x571030` at `0x569578`; 3 s slot-6 fallback `0x569A67` |

## Range-metric table
Constants: `0x989F10`=1.5 (eye), `0x987DF0`=0.1, `0x98D898`=0.2, `0x9919F0` f32=30.0, `0x98BD6C` f32=20.0, `0x9951DC` f32=1.6, `0x9951E0` f32=1.4. Appearance columns `[0xA0FDDC]`=HITRADIUS, `[0xA0FDF0]`=RUNDIST.
| # | test (site) | metric | radii / constants | gate |
|---|---|---|---|---|
| 1 | attack vs approach (`0x6D8283..0x6D82C8`) `dist2 > (max+0.1)^2` | 2D centre dist2 (`006d7e4a FLDZ`/`006d7e4c FSTP [EBP-0x74]` zeroes z, `006d7e57 CALL 0x44b8a0`) | none subtracted; `+0.1` at `006d829a` | CONFIRMED, high |
| 2 | too close, non-party only (`0x6D838D`) `dist2 < desired^2 - 0.2` | 2D | `006d838d FSUB [0x98d898]`; party exempt | batch-only |
| 3 | door/placeable gate (`0x6D8326..0x6D8357`) | 2D | `desired^2` | batch-only |
| 4 | LOS `0x52BB60` (`0x6D7B85`) | 3D ray, eye 1.5 +/- 0.01 | exclusion ids attacker + target | batch-only |
| 5 | same-area test (`0x6D828E`) | id compare | any difference -> approach | batch-only |
| 6 | MaxAttackRange melee | n/a | `Desired(tid,0)+0.5` (`00571684`) = 1.6 + HR(a) + HR(t) + 0.5 | batch-only |
| 6b | MaxAttackRange ranged | n/a | baseitem `+0x34` (0 -> 30.0, `005715ec/005715f9`) + stat bonus `0x6B7BB0(3,0,0)`; flag != 0 -> 20.0 (`005715c4`) | batch-only |
| 7 | DesiredAttackRange | n/a | 1.6 + HITRADIUS(att `[+0x1184]`) + HITRADIUS(tgt), NOT PERSPACE; flag != 0 -> flat 1.4 (dead store `0057170c..00571739` then `0057173c`); door/placeable via `0x585F10`; else 1.5 | batch-only |
| 8 | `0x57C990` accept | 3D length (`0057cd51 CALL 0x5142f0`) minus self PI+8 and cand PI+8 (`0057cd80`, `0057cdc0`), `0057cdcf FCOMPP` | accept if `d < best`, best starts at `range` | CONFIRMED, high |
| 8b | `0x57C990` scan prune (T1) | raw X gap vs best (`0x57CEB6..0x57CED2`) | radii not applied (error <= r_a + r_t) | batch-only |
| 8c | `0x57C990` LOS | eye +1.5 both ends (`0057cdfc`, `0057ce37`) | seen bit, hostile `0x5E1680==2`, not dead | batch-only |
| 9 | A1 / E-H2 retarget radius (`0x6D7D15` -> `0x6D7D24`) | `0x57C990(range = MaxAttackRange(selfId,0,1), exclude = current target, ref = INVALID)` | ranged: 20.0; melee ~ 1.6 + 2*HR(self) + 0.5 | CONFIRMED, high |
| 10 | CreateNewAttackActions pre-check (spell round, `0x6DA70D`) | 3D dot vs MaxCleaveRange^2 (`0x571540`) | no +0.1 | batch-only |
| 11 | BroadcastCombatStateToParty | 3D `x*x+y*y+z*z <= range^2` | PrimaryRange or 30.0 for party | batch-only |
| 12 | `0x5BC1D0` (AI1) | gate: same area && GetIsInUseRange (`0x585CC0`), no LOS | not re-derived here | see AI1 |

## Key behaviours
1. Round is a separate heap object, `new(0xB2C)` (`0056223f`), stored `creature+0x10DC` (`00562290`); owner back-pointer `+0xB04` (`0058e47b`). (lead + gate confirmed)
2. `+0xA9C` is the REMAINING round budget (3000 at start `0058e8da`), shrunk by each attack's animation time with a proportional rescale of `+0xA94` (`0058fe85 FIDIV [ECX+0xa9c]`, `0058ff70 MOV [EDX+0xa9c],ECX`). Review listed it as a constant length.
3. One `AIActionPhysicalAttacks` call resolves all attacks and returns 2; damage/OnAttacked events are posted later from `UpdateCombatRoundTimer` when the pause timer `+0xAB0` expires (`005711a8`, `005711f1`).
4. Pause: AIActionPhysicalAttacks sets `SetRoundPaused(1,self)` (`006d904d/006d9055`) and `SetPauseTimer(dur,0)` (`006d906c/006d9078`), `+0xAB8++` (`006d90b0`). While paused (`+0xAA8 != 0`) 0xC and 0x3F return 1. Master round pauses the slave with INFINITE (`006d9180`), slave waits for the master.
5. Every expired round ends (`0x58F7E9..0x58FB0D`); engaged/master/slave only changes the log text. Negative timer -> "Timer is negative" end (`0058fa90`).
6. `+0x570 = 3000` reset sits in the PROLOGUE of AIActionPhysicalAttacks (`006d740f`) before any gate, so a second queued 0xC action that keeps returning 1 while the round is paused (or server paused) re-arms the fallback every tick (A1).
7. Slot-6 (EndRound script, `creature+0x2A0`) has exactly THREE engine sites. (1) EndCombatRound `0x58F50F` (7 gates: bRunScript `0058f343`; head not a move `0058f39c`/`0058f3b2` = review F2; `+0xA8C == 0` `0058f3c2`; `+0xA90 == 0` `0058f3d5`; not controlled leader `[EBP-0x24]` `0058f3e2`; not dead `0058f41e`; not party member with hp <= 0 `0058f440..0058f486`). (2) AIUpdate 3 s fallback `0x569A67` (`005699e1 CMP [EDX+0x570],0`, reset `00569a72`). (3) `OnRemoveSetStateInternal 0x59E860` at `0x59E904` (`0059e8e1 LEA ECX,[EAX+EDX*0x1+0x270]`). SetCombatState(0)/death end the round with bRunScript = 0.
8. Slot-6 site 3 detail (gate): effect type 9 is created only in `OnApplySetState 0x59CBA0` (int0 = state; also writes creature `+0xFF5` = current debilitating state, AI3's "class byte" reading is wrong). Remove dispatcher `0x5947A0` indexes `[handler+8][type*4]`, `0059414c MOV [ECX+0x24],0x59e860` = index 9. Byte table `0x59E958` -> slot 6 for states 1, 3..10, 0xE..0x13 (case 0), `0x57A990` only for 12/13 (case 1), nothing for 2/11 (case 2). It fires mid-combat for party members when a stun/choke/horror/whirlwind/droid-stun ends (creators: EffectDroidStun `0067754e` state 3, Choke `0067825e` 7, Horrified `0x677DFE` 8, Whirlwind `00677abe` 10) and when a stronger state replaces a weaker one (`0x59CBA0` first removes existing type-9 effects). NO leader, dead or combat gate; then refreshes mode-0 perception for non-IsPC (`0x5D1510`, `0x5D0F30(0)`).
9. Engine target choice: only the controlled leader gets `CreateNewAttackActions` (`0058f5d1`, gate `0058f586/0058f58a`) plus the IsPC-only LOS-blocked retarget in AIActionPhysicalAttacks (`006d7c5d CALL 0x517980`, `006d7c73 MOV ECX,[EAX+0x10e8]`, `006d7d24 CALL 0x0057c990`): needs two consecutive blocked checks at the same exact position. Companions/enemies are script-only (slot 6 -> k_def_combend01/k_hen_combend01 -> k_ai_master -> ActionAttack -> AddAttackActions). Existing E-H2 (`0x6D7C79 -> 0x6D7C8C`) forces the IsPC path for companions.
10. Retarget accepts by 3D distance minus radii (row 8), the attack tests the 2D centre against `(MaxAttackRange+0.1)^2` (row 1), and `InitializeNewAttackTarget` sets `+0x52C = 1` so the next pass ends via `006d83ea..006d840c` (EndAttackAction(1), return 3). This is the a1_retarget_strict mismatch.
11. `+0xAB8` = actions started this round (`006d90b0`, `005b9447`, `005bb076`; clear `0058ea53`); AIActionCombat refuses a new action while `>= 1` (`006d96cc CMP [EBP-0xfc],1 / JGE 0x6da425`), a GUI-queued 2nd action waits for round end.
12. Hit roll `0x6E0B10`: d20 `006e0bd9 CALL 0x6a2000`, nat 1 -> 4 (`006e0d3f`), nat 20 auto-hit, threat range via `0x6AFA60`, confirm roll `006e0e39`, crit result 2 (`006e0f74`). No AI-level input: party and enemy identical. Debug override `[0x00a7e11c]` never written.
13. Damage events (`0x51E170`, delta from the round): event 0xF to target per attack -> OnAttacked slot 3 (`0056faa1`) unless dead or party hp < 1; event 5 ApplyEffect damage (type 0x16) at cumulative delay; events 0x16 / 0x13 (names SPECULATIVE). OnDamaged slot 4 is run directly by `OnApplyDamage` (`005966a1`, `005966d0`) and is NOT run for the party leader (`00596673`, `0x5FE3B0`).
14. `+0xA90` = cutscene-attack flag (`+0x74 = 1` only from AddCutsceneAttackAction `005909d0`); suppresses slot 6 and CreateNewAttackActions.
15. Stance `stats+0x146`: only combat-path reader is `0x6D8508` (stances 11/12 only); `0x5BC66F` reader is dead; 0, 9, 10, 13, 16 behave identically in-engine.
16. RUNDIST mobility flag (`006d825f FCOMP [0x98c1e8]`): 0 for turrets -> no approach, return 2 when out of range. EndAttackAction does not touch the round or `+0x524` (R1 A1 vs `+0x524` stands).
17. `+0xA98` dead field (writers `0x58E409`, `0x6E051A`, `0x6DC338`, no reader). `+0xADC` NewAttackTarget dead in play.

## Struct fields
Round = object at `creature+0x10DC` (size 0xB2C). Full set (~60 rows incl. attack data) is in DB table `struct_fields`.
| class | offset | type | name | conf | evidence |
|---|---|---|---|---|---|
| CSWSCombatRound | 0x004 | 7 x 0x17C | CSWSCombatAttackData[7] (5 used) | high | `0058e264 PUSH 0x17c` |
| CSWSCombatRound | 0xA84 | int | RoundStarted (active) | high | `0058e873 MOV [EAX+0xa84],1` |
| CSWSCombatRound | 0xA88 / 0xA8C | int | SpellCastRound / spell-round param (suppresses slot 6) | high | `0058f3c2` |
| CSWSCombatRound | 0xA90 | int | cutscene-attack / no-script flag | high | `0058e8d1`, `0058f3d5` |
| CSWSCombatRound | 0xA94 | int ms | Timer | high | `0058f7ef` |
| CSWSCombatRound | 0xA98 | int | debilitated-at-attack (dead field) | med | writers `0x6E051A`, `0x6DC338`; no reader |
| CSWSCombatRound | 0xA9C | int ms | RoundLength = remaining budget (3000 at start) | high | `0058ff70`, `0058e8da` |
| CSWSCombatRound | 0xAA0 / 0xAA4 | int | OverlapAmount (cap 1000) / BleedTimer | high | `0058fd99`, Save |
| CSWSCombatRound | 0xAA8 / 0xAAC | int / id | RoundPaused / RoundPausedBy | high | `00571088`, `0x58FC6F` |
| CSWSCombatRound | 0xAB0 / 0xAB4 | int | PauseTimer ms / InfinitePause | high | `0057110c`, `005710ad` |
| CSWSCombatRound | 0xAB8 | int | actions started this round | high | `006d90b0`, `006d96cc` |
| CSWSCombatRound | 0xABC | byte | CurrentAttack index | high | Save "CurrentAttack" |
| CSWSCombatRound | 0xAC0 | int | off-hand attack flag | high | `0x59056D` |
| CSWSCombatRound | 0xADC | id | NewAttackTarget (load-only writer) | high | `006d9247`, `0x592978` |
| CSWSCombatRound | 0xAE0 / 0xAE4 | int | OnHandAttacks / OffHandAttacks | high | `0x592260` |
| CSWSCombatRound | 0xB00 | ptr | round action list (0x88-byte actions) | high | `0058e350` |
| CSWSCombatRound | 0xB04 | ptr | owner creature | high | `0058e47b` |
| CSWSCombatRound | 0xB08 / 0xB0C | int | Engaged / Master | high | StartCombatRound |
| CSWSCombatRound | 0xB10 / 0xB14 | id | MasterID / slave id | high | `0x58E7BE`, `0x58E7DA` |
| CSWSCombatRound | 0xB18 / 0xB1C | ptr / byte | current round action / its type | high | AIActionCombat |
| CSWSCombatRound | 0xB20 | id | round target | high | `0058e91c` |
| CSWSCombatRoundAction | +0x08 / +0x10 / +0x14 / +0x74 | int | duration / type (1,3,6,7,9,0xA,0xB,0xC,0xD,0xE) / target id / cutscene flag | high | `005909d0` |
| CSWSCombatAttackData | +0x5C | byte | result 1 hit, 2 crit, 3 auto-hit, 4 miss | high | `006e0c8d`, `006e0f6b` |
| CSWSCombatAttackData | +0xC1 / +0xC2 / +0xD8 / +0xDC / +0x104 | byte | d20 / bonus / nat20 / nat1 / target AC | high | `006e0bd9` |
| CSWSCreature | +0x520 / +0x524 / +0x528 | int | in combat / combat-exit ms / frozen flag | high | `0057a64f` |
| CSWSCreature | +0x52C | int | passive / end instead of approach | high | `006dacdb` |
| CSWSCreature | +0x548 / +0x550 | id | attack target / attempted attack target | high | `006dadb5`, `0x57E3D0` |
| CSWSCreature | +0x558 | vec3 | LOF-blocked position | high | `006d7c5d` |
| CSWSCreature | +0x570 | int | slot-6 fallback ms | high | `006d740f`, `00569a72` |
| CSWSCreature | +0xD0 | int | dt since last AIUpdate | high | `0056956c` |
| CSWSCreature | +0xFF5 | u8 | current SetState id (highest); AI3 "class byte" corrected | med | `0x59CBA0`, gate |
| CSWSCreature | +0x10DC / +0x10E8 / +0x11AC | ptr / int / int | round ptr / IsPC / party flag | high | `00562290` |

## Bugs and candidates
| item | verdict | user impact | fix idea | mod item |
|---|---|---|---|---|
| A1: second queued 0xC action re-arms `+0x570` while round paused | CONFIRMED structure | the 3 s slot-6 fallback is re-armed every tick for the whole pause | batch gave no fix; installed E2 `0x569A08` is the existing A1 hook | A1 |
| a1_retarget_strict mismatch | CONFIRMED (gate-verified) | picker returns candidate beyond the attack's own 2D bound; target flips then EndAttackAction | recompute the attack bound for the candidate: 2D centre dist2 vs `(MaxAttackRange(cand,duel,0)+0.1)^2` | a1_retarget_strict, A1, T1 |
| F2 head-move gate `0x58F3A1` | CONFIRMED | slot 6 does not run when a move heads the queue | AI2-F2 skip_original_bytes at `0x58F3A1` (skips the `MOV [EBP-0x1C],0` store) | FM2 (F2) |
| slot-6 SetState site 3 (`0x59E860`) | CONFIRMED + extended (gate) | extra EndRound script runs on a companion when a stun/choke/horror/whirlwind/droid-stun ends or is replaced; no leader/dead/combat gate; can interleave with the 3 s fallback | gate or accept; probe P-S6 source 3 | slot-6 SetState (A1, enemy-AI, FM7) |
| leader gets no OnDamaged | CONFIRMED | scripts keyed on slot 4 never fire for the party leader | note only | enemy-AI, E7 |
| enemies never engine-retarget | CONFIRMED | target change only via script | note only | enemy-AI, E7 (`hold_enemies`) |
| 2D vs 3D range mismatch / 0x11 use-range metric | OPEN | possible approach ping-pong | wave 2 | A1, Mod 8 |
| `+0xAB8` once-per-round action gate | CONFIRMED | second action waits for round end | none | A1, Mod 8 |

## Hook sites
Gate-checked: only the installed E2 window.
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| E2 (installed) | 0x569A08 | 6 | (unchanged) | cold | 3 s fallback; gate: no overlap |

Batch-only (not gate-checked; windows validated by the batch's `win.py` scan, none overlaps installed `0x6D7C79, 0x6D83EA, 0x6D850F, 0x6D853E, 0x6D740F, 0x6D78B7, 0x6D8BE2, 0x6D9E2A, 0x569A08, 0x586F43, 0x5868B9`):
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| AI2-SR | 0x58E730 | 6 | `55 8B EC 83 EC 40` | cold | round start |
| AI2-ER | 0x58E990 | 5 | `55 8B EC 6A FF` | cold | round end |
| AI2-S6 | 0x58F48C | 12 | `8B 85 58 FE FF FF 8B 88 04 0B 00 00` | cold | consumed exit -> `0x58F514` skips the whole slot-6 `RunScript`; EAX liveness read by hand, lead check required |
| AI2-F2 | 0x58F3A1 | 7 | `C7 45 E4 00 00 00 00` | cold | skip_original_bytes (F2 fix); not a branch target |
| AI2-RA | 0x6DACF0 | 6 | `55 8B EC 83 EC 48` | warm | ResolveAttack entry |
| AI2-ROLL-M / ROLL-R | 0x6DEC05 / 0x6DB122 | 6 / 7 | `8B 8D 14 FF FF FF` / `8B 45 F4 50 8B 4D 08` | warm | hit-roll probes; ROLL-R stolen contains `PUSH EAX` (plain detour only) |
| AI2-RNG | 0x6D8283 | 6 | `8B 8D 64 FB FF FF` | warm/hot | range numbers at the approach decision |
| AI2-RT | 0x57C990 | 9 | `55 8B EC 81 EC 70 01 00 00` | cold | retarget picker entry; return address = caller |
| AI2-TICK | 0x571030 | 9 | `55 8B EC 81 EC 90 00 00 00` | HOT | per creature per AIUpdate |
| AI2-SD | 0x6DCBE0 | 5 | `55 8B EC 6A FF` | warm | SignalDamage |
| AI2-DRL | 0x58FD50 | 6 | `55 8B EC 83 EC 28` | warm | DecrementRoundLength |
| AI2-PZ | 0x58FC60 / 0x58FCD0 | 6 / 6 | `55 8B EC 83 EC 0C` / `55 8B EC 83 EC 08` | warm | pause setters |
| AI2-SS | 0x59E860 | 6 | `55 8B EC 83 EC 18` | cold | slot-6 site 3 entry (table entry, no direct callers) |
| AI2-ENG | 0x590FF0 / 0x5912B0 | 9 / 6 | `55 8B EC 81 EC 8C 00 00 00` / `55 8B EC 83 EC 4C` | warm | engaged/master state |

## Open items
- `0x585F10` GetUseRange outputs (door/placeable/duel branch); `0x6A93A0/0x6A5110/0x6E1540/0x6DAF00` bodies unread, names low.
- `attack+0x4C`, `cr+0xFDC`, `stats+0x120`, `cr+0x59C`, `cr+0x11CC` meanings; event ids 0x13/0x16/8 payload semantics (posters of event 8: `0x5482A0`, placeable handler).
- `+0xB24/+0xB28` unused for AI; `ResolveEngagedState` rule when the target is the leader; `(*(area+0x22C))->vt+0x10(1)` from `0x57DC20` and `0x57DE70` caller.
- Gate: does `0x585CC0` GetIsInUseRange (0x11) disagree with the 2D `0xC` test (ping-pong candidate)? Does slot-6 site 3 matter in play (P-S6 source 3)? State-id -> effect map for ids 1, 2, 4-6, 9, 0xB-0x13 open.

## Mod relevance
| item | served by |
|---|---|
| A1 | `+0x570` prologue reset (key behaviour 6), fallback site `0x569A67`, `+0xAB8` gate, AI2-S6/F2 windows |
| a1_retarget_strict | range table rows 1, 8, 9 (gate-verified mismatch), AI2-RT/RNG probes |
| Mod 8 (kite) | round state `+0xA84/+0xAA8/+0xB20`, pause cadence, `+0xAB8`, range reach 2.2+HR+HR |
| enemy-AI / E7 (`hold_enemies`) | script-only target choice, OnAttacked event 0xF, no OnDamaged for leader |
| slot-6 SetState | site 3 `0x59E860` (also FM7 context) |
| FM2 (F2) | head-move gate `0x58F3A1` |
| T1 | scan prune (row 8b) |
| perf | AI2-TICK `0x571030` hot path |
| C1 | `0x5867E0` re-verified |
| medpac | none |
