# Engine-AI atlas: synthesis review (Opus, 2026-10-02)

Scope: README.md, AI1..AI8, bug_candidates.md, hook_catalogue.md against GATE_wave1/2, LEAD_SPOTCHECKS, lessons 212-330 (DB
read-only), the shipped companion-ai 0.1.0 / ranged-ai 0.3.0 sources, and the dump `.asm` / vanilla exe. Every check below was
re-done on disk in this review (quoted lines are from the dump). Nothing committed, DB not written, game folder not written (the
scratch window checker read the live `patch_config.toml` read-only, as `check_atlas_windows.py` does).

**Verdict.** The synthesis is sound. All 10 re-checked claim groups hold (about 40 addresses: pipeline, action table, event
table, counts). **8 errors** (2 substantive, 6 small), plus 4 stale DB lessons. The substantive ones are about who the "PC" and the "controlled" creature are (E1, E2). They change
P2's wording and one ranged-ai assumption. They do not affect the gated mechanisms (FM2/FM2b, A1, slot 6, +0x11AC).

## 1. Consistency (26 claims re-checked; errors first)

### Errors (file:line, what is wrong, correct text)
| # | where | wrong | correct text / fix |
|---|---|---|---|
| E1 | bug_candidates.md:16 and :150 (AI-P2); lesson 230 ("the controlled one every 200 ms") | the 200 ms cadence goes to the **IsPC** creature, not to the controlled one | `+0x10E8` = IsPC = `stats+0x6C` (`005768f5..005768fe MOV [EDX+0x10e8],EAX` from `[stats+0x6C]` in PostProcess 0x576750). Its only other writers are the ctor `00562d7d` and SwitchPlayerCharacter `005fe604/005fe951/005feae6` (script cmd 11); SetLeaderBySlot 0x7E5DF0 never writes it. Text: "Companion perception refresh every 4 s; only the IsPC creature (the Exile, also while AI-driven) gets 200 ms, so a player-controlled companion is also on 4 s". Same correction for lesson 230. |
| E2 | AI5_follow_party.md:63 and :103, GATE_wave2.md:51, lesson 319 ("OR it with id == PC id `0x51C8F0`") | `0x51C8F0` is the **controlled leader**, not the PC | `0x51C8F0` (thiscall serverApp) -> `0x539D60` -> player `0x632A20` = `GetGameObject(player+0x28)`. SetLeaderBySlot rewrites player+0x28 (`007e631d CALL 0x632830`, `006328d8 MOV [EDX+0x28],EAX`). So `0x5FB070(id) || id == 0x51C8F0()` misses the Exile while the player controls a companion. Text: "With only an id: resolve it (`0x51C0A0`) and test `+0x11AC`. `0x5FB070` covers NPCs only, and `0x51C8F0` returns the controlled leader." (conf med-high; probe PARTY settles it) |
| E3 | README.md:37 (pipeline) | "0x3D FollowLeader 0x5BF7F0 -> follow driver 0x5C6340" (no such edge) | "0x3D FollowLeader 0x5BF7F0 sets `+0x11B0 = 3`; the follow driver 0x5C6340 runs only from QuickWalk 0x5C3F70 (`005c4151`, needs `+0x220 && +0x11B0 == 3 && +0x11AC`), which runs from the scheduler pre-pass `0051d9bc` and MoveToPoint `005c216c`" (AI5 page line 3 already has it right). |
| E4 | README.md:75, AI4_scheduler.md:104 | "66 (distinct) handlers" | "67 typed slots (67 distinct case blocks), **65 distinct handler functions** (`0x5D1990` serves 0x29/0x2B/0x36, `0x5D17D0` 0x2A)". Re-decoded all 0x47 entries from the vanilla exe plus the first CALL of each case block. Every README row matches. |
| E5 | bug_candidates.md:11, :53 and DB row AI-MEDPAC-FORCECLASS ("a Jedi-only PC") | understated | "the Exile **always**". Every K2 PC class is in {3,4,5,11..16}: chargen offers 3/4/5 only and the prestige classes are 11-16. So the AI-driven Exile never uses medpacs through the HasTalent rule. |
| E6 | bug_candidates.md:44 and :170 (AI-EXIT5M) | "script must tolerate" | shipped FM4 mode 1 restores the whole 0x3C-byte snapshot, `+0x38` InSafetyRange included (`CompanionAiFm4Restore`). Status: "mitigated live for party members (fm4=1); vanilla otherwise". |
| E7 | hook_catalogue.md:43 (FM2c) | "[EBP-0x20] first code" | `[EBP-0x20]` is the value the fallback test reads (`0055b4fb`), but it is rewritten to 1 / -4 on some paths (`0055b0e0`, `0055b0e9`). The cached start->goal code is `PI+0x19C` (`0055af92`). A probe logs both. |
| E8 | README.md:28 | "L0 drop 0x56C01C (FM7)" listed as an AIUpdate site | `0056c01c` is in SpawnInHeartbeatPerception 0x56B8A0 block 3, reached from AIUpdate via `005694e3` (wording only). |

**Stale DB lessons** (lead: append "UPDATED 2026-10-02" lines):
- 230: P2 wording (E1).
- 275: "the door branch sends 0x1E to the door (0x5C0732)" is wrong. That site is the trigger case; the door case posts OnBlocked at `0x5C1240` (lesson 309). "Unproven: fail vs partial" is settled by lesson 305.
- 277: "FM5 (3 ... post sites)" should be 6 (lesson 309).
- 319: the 0x51C8F0 caveat (E2).

### Claims that hold (re-checked on disk)
1. Pipeline call sites: `0053396e CALL 0x51d790`, `005694e3 CALL 0x56b8a0`, `00569578 CALL 0x571030`, `005695b1 CALL 0x5d56b0`,
   `005c1f18 CALL 0x55ab30`, `005711a8 CALL 0x6dcbe0`, `005d5c28 JMP [EDX*4+0x5d7038]`, `0051d9bc CALL 0x5c3f70`, levels
   `005694d6`/`0056a0b5` -> `0x51E120`.
2. Slot-6 sites: `0058f50f`, `00569a67`, `0059e904` are all `CALL 0x6fd8d0`, with reset `00569a72 MOV [ECX+0x570],0xbb8`. The event table run
   sites `00570640 0057069a 00570745 005709f0 00570a72 00570d08 00570f55 0056faa1 005966d0` are all among the 142 RunScript call sites.
3. Action table: all rows of README:78-100 match the decoded table (types 0xD/0x39/0x3B/0x42 -> `0x5D688E`).
4. All 67 case blocks `JMP 0x5D688E`, and every path from there reaches `0x5D6A60` (RA-R sees every dispatched action).
5. A1 root cause `006d740f MOV [EAX+0x570],0xbb8`; approach requeue `006d8729 CALL 0x540ca0`.
6. The DB counts in README:5 (367/3080/328/39) and the coverage TOTAL row (691/46/54/267/295) both add up.
7. hook_catalogue counts: 24 gate-checked + 77 batch-only windows.
8. +0x11AC writers: only the ctor and `0058a35e`; 15 `CALL 0x58a330` sites, the same as GATE_wave2 1.1.
9. Shout loop exits: `00547180 JZ`, `005471b5 JL`, `005471c4 JGE`, `00547203 JNZ`, `00547266 JMP`, all to `0x5475E8`, the single
   `RET 0xc`. So the abort reading in GATE_wave2 1.4 holds.
10. The 0x11 handler returns 2 on the in-range path (`005bc5ee JNZ 0x5bc6ef` -> `MOV EAX,[0x9944b8]`), and the LOS block is dead (lead).

## 2. Shipped hooks vs the atlas
| hook | verdict | reason / action |
|---|---|---|
| CA T1 `0x692C50`, T2 `0x68F301`, T3 `0x6F62F2` | OK | VM sites; party filter = `+0x11AC` via objInfo (correct test, gate 1.1). |
| CA E1 `0x6D740F` | OK | The atlas confirms the `+0x570` re-arm in the 0xC prologue is the spin signature. |
| CA E1b `0x6D8BE2` | OK | Frame offsets match the AI2 range table (dist2 `[ebp-0xBC]`, max `[ebp-0x2C]`). |
| CA E2 `0x569A08` | **ASSUMPTION CHANGED** | E2 sees slot-6 site 2 only. Sites 1 (EndCombatRound `0x58F50F`) and 3 (SetState removal `0x59E904`, ungated) also re-run k_ai_master. So "A1 end -> E2 within rethink" is not the follow-up test; use probe RS (any slot-6 site) for `A1 FOLLOWUP/ORPHAN`, and keep E2 as is. |
| CA E3 `0x56BDBD` | OK | Mode-1 perception never calls `0x522AF0` (gate), so E3's counts are not distorted by R1. |
| CA A1 `0x6D853E` | **ASSUMPTION CHANGED** | `a1_retarget=1` calls `0x57C990` (3D minus both PI+8 radii), while the attack tests 2D centre dist2 vs (max+0.1)². `InitializeNewAttackTarget` sets `+0x52C` (`006dacdb`), so a candidate up to r1+r2 too far ends silently at `0x6D83F6..0x6D83FF` (ranged-ai E-H1's window, where it shows as `H1 ... end52c!=0 inRange=0`). Action: log-only strict test now (probe A1X), then decide `a1_retarget_strict`. `a1_rethink_ms` is right to act only on "end": a retarget re-enters the 0xC prologue, which re-arms `+0x570`. The controlled test (client chain, `[ebp-0x178]`) is correct (not IsPC). |
| CA P1i `0x56BDB4` / P1s `0x56BDE3` | OK | The stamp is never stored (gate, lead census `00562366` only). Shipped OFF; no change. |
| CA FM1 `0x586F43` | OK | Vanilla clears only a bumpee with no move queued, so a follower's queue there is `[0x3D]` (follow state 0 adds `0x40` + move, which vanilla does not clear). `onlyFollowTypes` covers that. ClearAllActions is a no-op at `+0xEC != 1` in both paths. |
| CA FM4 `0x57451F` / `0x575183` | OK | Snap reads the flag before the clear, and restore runs after `0x57516B` (gate 1.1 caveat). Bonus: it neutralizes AI-EXIT5M (E6). It is redundant only if FM4B patches A+B ship; harmless then. FM4B's non-party level-2 churn is not addressed by FM4 (by design). |
| CA A4 `0x6D9E2A` | OK | No atlas finding touches it. |
| CA C1 `0x5868B9/F1/929` | OK | Re-verified by AI2/AI3. `party_only` admits the PC (flag 1), as intended. |
| CA A3 `0x6D78B7` | OK | Off. Uses the client controlled id, correctly. |
| RA E-H1 hold `0x6D83EA` | **OK on the metric, ASSUMPTION CHANGED on "not PC"** | The range test reuses the engine's own 2D `[ebp-0xBC]` vs (max+0.1)², i.e. the same metric as the attack (no mismatch). But `!isPC` reads `+0x10E8` (the Exile), and `0x6D83EA` is reached by the controlled creature too (`006d83a0..006d83e4` all fall into it). So a player-controlled stance-16 companion can get its ordered attack held. Fix in ranged-ai: test `id != controlled id` (as A1). Play check: control a Kiting companion and order a shot past an ally. |
| RA E-H2 `0x6D7C79` | **ASSUMPTION CHANGED** | Same IsPC predicate (as E-H1). Its forced retarget is the vanilla `006d7d24 CALL 0x57C990(MaxAttackRange(self,0,1))` with the same 3D-minus-radii mismatch, and it ends at `0x6D83FF` next pass. Log-visible as H2 `retarget=1`, then H1 `end52c!=0 inRange=0`. Analyzer check (probe design). |
| RA E-H3 `0x6D850F` | OK | It feeds the engine's own controlled-aware no-approach rule. |
| RA S-H1..S-H4 | OK | UI windows; no atlas finding. |

## 3. bug_candidates ranking and the 28 proposed rows
**Ranking.** The scores are computed correctly (H/M/L x risk). For the four-way tie at 6, re-order by "interacts with a live hook /
every fight" first:
1. AI-A1RETARGET (live `a1_retarget=1`, and ranged-ai E-H2);
2. AI-MEDPAC-FORCECLASS (every fight, Kreia/Visas/the Exile);
3. AI-E4KEY (blocks the E4 build);
4. AI-SPOTSIGN (user decision, stealth only).

Also:
- Score AI-AIQ impact as `?` until perf-lab measures it.
- Group B rows are live since 2026-10-01 23:36, not "built".

**Rows** (approve 14 / fix 13 / drop 1):
| id | decision | one line |
|---|---|---|
| AI-A1RETARGET | fix | add "also reached by ranged-ai E-H2 (`006d7d24`), silent end at `0x6D83F6..0x6D83FF`"; status `designed (log-only A1X probe)` |
| AI-E4KEY | fix | hypothesis: "{0,3,39} + no progress is NOT a valid key (3 = a ranged attacker standing in range, 38 = an idle follower); E4 = {0} plus {38 with leader distance > follow range}; engine approaches go to the DLL FM2" |
| AI-FM2 | fix | evidence: "fail-fast acts at `0x5BC5F4` (consumed -> `0x5BC6EF` after `0x53F6A0(cr, group)`) so creeping-partial loops are caught; `0x5C2438` only skips the wait" |
| AI-FM2B | approve | same loop as AI-FM2; keep the row until the probe splits the phases |
| AI-P2 | fix | title "...every 4 s (IsPC creature 200 ms; a player-controlled companion also 4 s)" (E1) |
| AI-F2 | approve | |
| AI-FM3 | approve | |
| AI-FM5 | approve | evidence add: kill metric = slot-12 script runs/s per creature (probe RS) |
| AI-PATHFAIL01 | approve | |
| AI-A1 | fix | status `live 2026-10-01 (awaiting play test)` |
| AI-FM1 | fix | status `live 2026-10-01 (awaiting play test)` |
| AI-FM4 | fix | status `live`; hypothesis add "the restore also keeps FollowInfo+0x38 (AI-EXIT5M mitigated)" |
| AI-A4 | fix | status `live` |
| AI-C1 | fix | status `live` |
| AI-A3 | fix | status `live, off (a3=0)` |
| AI-P1 | fix | status `live, off (p1=0)` |
| AI-FM6 | fix | status `partly refuted (consumer 0x5CA6A0 known)` instead of `documented` |
| AI-FM7 | approve | |
| AI-T1 | approve | status `covered by AI-A1RETARGET` |
| AI-R1 | approve | |
| AI-BUMPABORT | approve | |
| AI-MODSPOT | approve | |
| AI-NOLOS | approve | |
| AI-RANGE2DA | approve | ("non-controlled" is right here: `0x58B140` is a `0x51C8F0` caller) |
| AI-EXIT5M | fix | status `mitigated live (fm4=1, party)` (E6) |
| AI-CMDABLE | approve | |
| AI-SEENSTAYS | drop | design behaviour, not a defect; keep it in AI3 and bug_candidates section 4 |
| AI-L0PATH | approve | |

Also update the existing row AI-MEDPAC-FORCECLASS (E5).

## 4. Pending track C/D items the atlas changes
**E4 (stuck move).** R2's `GN_PartyStuckMoveCheck` keys on `GetCurrentAction == ACTION_MOVETOPOINT`. GetCurrentAction reports the
last reportable node of the head group: an engine approach reports 3 (0xC group) or 39 (0x3F group), and a follow walk reports 38. MOVETOPOINT
appears only for script move chains (lesson 325), so the FM2/FM2b loops never reach E4. Widening the key to {3,39} is wrong: a
ranged companion shooting from one spot reports 3 and never moves. The design must be:
(a) keep the MOVETOPOINT branch for script moves (kite, E3/E11 stand-off);
(b) add 38 only when the member is farther than the follow range from the leader (e.g. > 8 m) and in the same 2 m cell for N
heartbeats (a stuck follow);
(c) leave engine approaches (3/39) to the DLL FM2 fail-fast, which sees the 0x11 re-approach count directly.
Probes GCA and RAR confirm the 3/39/38 values at runtime before the build.

**Medpac rule (Force-class).** R4's party guard `GetCreatureHasTalent(tSpe11_05)` is FALSE for every class list without a non-Force
slot. HasTalent `0x6B8840` sends Force slots only to `0x6BD9C0 && 0x6B8F40`; `006b89ce CALL 0x6f2a30` / `006b89d8 JZ` skips the
item scan `0x6C0330`. That covers Kreia (4), Visas (5) and **always the Exile** (E5). The design must be: party branch =
`GetIsTalentValid(tSpe11_05) && GN_PartyMedpacStock() >= 1`, never HasTalent. `ActionUseTalentOnObject` reaches UseItem
(`0x57D8B0` -> `0x57B4C0`) without HasTalent, but it does test `+0xEC` (commandable, `0057d8e3`), so keep the existing
`GetCommandable()` guard. `medpac_guard_check` must accept the second, party-gated `tSpe11_05` (lesson 280).

**E9 (shout reach).** A SILENT_TALK shout (35 m) reaches a listener only if all hold:
- it has `+0x1C0` listening and pattern 15;
- its perception entry for the shouter has HEARD;
- it SEES the shouter or is < 10 m away.
There is no faction filter (`0054734c..005473a7`), and a stale id aborts silently (LOW). Since companion perception refreshes only every 4 s (P2), a newly arrived ally may not be HEARD yet; shouts repeat about every 3 s, so delivery is retried, not lost. R2's E9 design (non-party friendly shouter, seen live enemy within 25/15 m, member within 15 m of the leader, idle) stays valid. But it must be built only after probe RS shows `S7 pat=15` deliveries from non-party speakers to party members in an allied fight. If there are none, E9 is moot without a perception change. The enemy gate (`GetIsFriend(oShouter)`) must stay, because enemies in 35 m receive the same shout.

**FM2 fail-fast.** Key it on the 0x11 re-approach at `0x5BC5F4`, per (creature, target): N re-approaches (2-3) within 12 s with no
progress (3D distance to the target shrank by < 0.5 m). Act there, not only at the FAIL leg:
1. `0x53F6A0(cr, node group)` deletes the queued 0xC/0x13 of the chain (pattern `005868ac..005868b4`);
2. a consumed exit to `0x5BC6EF` (`MOV EAX,[0x9944b8]`, the 0x11 "done" return; EAX dead; no pending pushes at either end)
   ends the 0x11 without re-queuing [wait, move, 0x11].

The gate's own reading shows that a creature that "creeps a few centimetres" can loop on partial legs (result 2) with no FAIL leg, which `0x5C2438` alone never sees. `0x5C2438` (consumed -> `0x5C24C8`) stays optional, to skip the 2-4 s wait. Validate the consumed variant of `0x5BC5F4` with `check_windows.py` (EAX and stack) before the fix round. Probes FM2R/FM2F/FM2C decide which phase dominates. R1 open question 2 (target exclusion, so that k_ai_master does not re-pick the same target) is still open.

**a1_retarget_strict (2D metric).** In `CompanionAiA1Gate`, after `0x57C990` returns a candidate:
- compute the 2D centre dist2 from attacker `+0x94` to the candidate `+0x94` (x, y only);
- accept only if it is <= (mx+0.1)², where mx = `[ebp-0x2C]`, the bound the attack itself uses (this already contains the melee radii, so there are no false rejects);
- otherwise treat the candidate as INVALID (end).

`a1_retarget_strict=0` = log `STRICT ... would-reject` only (probe A1X). Turn it on if the play logs show any would-reject followed by `H1 end52c!=0`. Retrying with a second exclude is not possible (one exclude slot). ranged-ai E-H2 has the same mismatch, but its pick is vanilla code past the window: either accept it (stance 16 only, log-visible), or add a post-pick window later.

**M8 (swap counter).** M8b decrements local 8 once per k_ai_master 2003 (EndRound). The atlas shows 2003 has three engine sources:
EndCombatRound once per round, skipped when a move heads the queue (F2: frequent for a kiting stance-16 member); the 3 s fallback in
combat without a round; and SetState removal, ungated, when a stun-like state ends. So "about two round ends" means "about two slot-6
runs" (about 3 s apart), shortened by a stun ending and lengthened by F2 skips that the fallback then covers. The design stays (it is still more
stable than per-call counting), but the marker text should say "slot-6 run" and the RS probe (S6A/S6B/S6C per creature)
measures the real cadence. The engine's zeroing of local 8 at combat exit (`0x57A5C1`, party) still bounds it.
