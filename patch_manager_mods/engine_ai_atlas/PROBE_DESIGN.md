# Phase-4 probe round: design (companion-ai probe build), 2026-10-02

**Goal.** Add log-only probes to companion-ai that answer the open atlas questions before any fix is built: FM2/FM2b, FM3, FM5,
P2, party flag, action histogram, the A1 follow-up, E9 reach, SetState/slot 6 and GetCurrentAction. Behaviour stays vanilla.
All new windows are **plain detours** (no consumed exit). Each probe has an INI flag `probe_<name>`, a `PR`/`PRS` trace marker
(`PR`, not `P`, so `analyze_play_logs.PA_RE` never matches a probe line) and per-subject plus per-session caps. Hot handlers test
the cheapest field first and return. Inputs: hook_catalogue.md "Phase-4 probe set", REVIEW_synthesis.md, R1 "Hook designs".

**Verified (this design, 2026-10-02).**
- Scratch copy of `check_atlas_windows.py` with the 13 rows of section 6 plus the 33 shipped reference rows: **46 [OK], 0 WARN/FAIL, pairwise overlap none, TOML cross-check ok**.
  - live snapshot `live_hooks_2026-09-30g.tsv` (120 hooks) + live `patch_config.toml` (28 ids / 153 hooks, read-only).
- `check_atlas_supp.py` (whole-dump inbound scan + jump tables), re-pointed at the 13 rows: **0 problems, no live hook within 12 B**.
- Bytes = vanilla `6a522e71` and LAA `4ab72fc1` (workspace backup copies): **ALL MATCH**. Nearest sibling self-patch site: 1.1 MB.
- Nearest shipped window: FM3S ends exactly at the unshipped `0x51DBE7` probe (never ship that one); P2 is 144 B from `0x56BDE3`;
  every other window is > 1 KB away. No probe window is within 12 B of a live hook (perf-lab `0x55AB30/0x55AC26/0x51DC70/0x51DC86/0x544210`).

## 1. Probe set (13 windows + 2 handler-only probes)
| id | INI key | window: addr, len, stolen bytes | entry fn | live values the handler reads (asm quotes) | load |
|---|---|---|---|---|---|
| FM2C | probe_fm2c | `0x55B69A` 6 `8B 0D A0 B5 99 00` | 0x55AC80 PlotPathInArea | ebp. Result `[EBP-0x18]`: 2 via `0055b697 MOV [EBP-0x18],EAX` (partial granted), 3 via `0055b407 JNZ` / `0055b623 JZ`. Code `[EBP-0x20]` (tested `0055b4fb`; may be 1/-4, `0055b0e0/e9`). PI `[EBP+8]`: +0 client-path flag (`0055b3f8`), +0x2C creature id, +0x30 target id, +0x3C call count, +0x19C cached first code (`0055af92`) | cold: once per failed search |
| FM2F | probe_fm2f | `0x5C2438` 9 `8B 95 6C FA FF FF 89 51 70` | 0x5BFC00 MoveToPoint | ebp. Creature `[EBP-0x5A8]` (`005bfc28`), PI `[EBP-0x598]` (`005c242c`, = ECX here), node `[EBP+8]` (group = u16 node+0x6C). Plain this round; the design's consumed exit `0x5C24C8` is NOT used | cold: per FAIL leg |
| FM2R | probe_fm2r | `0x5BC5F4` 9 `8B 4D E4 8B 91 24 01 00 00` | 0x5BC1D0 action 0x11 | ebp. Creature `[EBP-0xA4]` (`005bc1d9`), target id `[EBP-0x28]` (node+0x38, `005bc25b..66`), otherArea `[EBP-0x40]` (`005bc457`), inRange `[EBP-0x2C]` (`005bc4b5`), range f32 `[EBP-0x3C]` (node+0x3C) | warm: <= 1 per 0.3 s per approaching creature |
| FM3S | probe_fm3 | `0x51DBE1` 6 `8B 4D D8 89 4D C4` | 0x51D790 UpdateState | ebp. Level `[EBP-0x34]`, first object `[EBP-0x28]` (`0051dbd8`); reached only while first `[EBP-0x38]==1` (`0051dbdb`, cleared `0051dc86`) | cold: 1 per level per tick |
| FM3E | probe_fm3 | `0x51DD33` 6 `8B 4D D0 2B 4D F0` | 0x51D790 | ebp. Sentinel `[EBP-0x3C]`, done `[EBP-0x40]`, now `[EBP-0x30]/[EBP-0x2C]`, start `[EBP-0x10]/[EBP-0xC]`, slice `[EBP-0x20]/[EBP-0x1C]` (the stolen SUB/SBB re-run after the handler) | cold: 1 per level per tick |
| FM3R | probe_fm3 | `0x51E0C9` 6 `8B 45 08 8B 48 78` | 0x51E0C0 RemoveObject | ebp. Object `[EBP+8]`, master `[EBP-0x10]` (`0051e0c6`). List = master+4+16*level: +0 id array, +4 count, +0xC cursor (`0051d1dc`) | cold: level changes |
| RS | probe_rs | `0x6FD8D0` 6 `55 8B EC 83 EC 08` | 0x6FD8D0 RunScript (thiscall, `006fd92d RET 0xc`) | `[esp+0]` return address, `[esp+4]` CExoString* slot (0 allowed: `006fd8d9 CMP [EBP+8],0`), `[esp+8]` OBJECT_SELF id. Sites (return = CALL+5): S6A `0x58F514`, S6B `0x569A6C`, S6C `0x59E909`, S7 `0x5709F5`, S12 `0x57074A`, S1 `0x570645`/`0x57069F` | hot-ish: every script run (about 50-300/s); <= 7 compares on the return address, then out |
| P2 | probe_p2 | `0x56BE79` 6 `8B 85 2C FF FF FF` | 0x56B8A0 SpawnInHeartbeatPerception | ebp. Creature `[EBP-0xD4]`, mode `[EBP+8]`, elapsed `[EBP-0x18]`. The engine runs UpdatePerception(0) iff `[cr+0x10E8]==0 && mode==1 && elapsed>=4000` (`0056be8b..0056bea9`) | **HOT**: every AIUpdate of every creature; first insn = `*(cr+0x11AC)` test |
| PF | probe_pf | `0x58A35E` 6 `89 88 AC 11 00 00` | 0x58A330 SetPartyMemberFlag | ebp. Creature `[EBP-0x48]` (= EAX, `0058a358`), new `[EBP+8]` (= ECX), apply `[EBP+0xC]`, caller `[EBP+4]`. Old = `[cr+0x11AC]` (the handler runs before the store) | cold: 2 per creature per save |
| RAR | probe_rar | `0x5D6A60` 9 `8B 55 E4 3B 15 C0 44 99 00` | 0x5D56B0 RunActions | ebp. Object `[EBP-0x1A8]` (`005d6a42`), type `[EBP-0x18]` (`005d5bfd`), result `[EBP-0x1C]` (`005d688b`), node `[EBP-0x40]`. All 67 case blocks `JMP 0x5D688E`, and every path from there reaches this window. The stolen CMP feeds `005d6a69 JNZ` and re-runs after the handler | **HOT**: per executed action; first insns = `*(u8*)(obj+8)==5`, `*(obj+0x11AC)` |
| SHX | probe_shx | `0x5475E8` 10 `8B 4D F4 64 89 0D 00 00 00 00` | 0x547100 shout delivery (all 5 exits land here; single `005475f7 RET 0xc`) | ebp. Speaker `[EBP-0xC0]` (`00547129`), &string `[EBP+8]`, radius f32 `[EBP+0xC]` (`00547192`), bSeen `[EBP+0x10]`, area `[EBP-0x30]` (`00547148`), list `[EBP-0x14]` (= area+0x194, count +4; garbage if area 0), index `[EBP-0x1C]` (init -1 `0054712f`), last candidate id `[EBP-0x10]` (`005471e4`) | warm: per SpeakString |
| SS1 | probe_ss1 | `0x59D966` 6 `88 82 F5 0F 00 00` | 0x59CBA0 OnApplySetState | ebp. Creature `[EBP-0x20]` (`0059d960`), new state `[EBP-0x1C]` (`0059d963`), old = u8 `[cr+0xFF5]` (`0059d944`). The stolen bytes read AL (the wrapper restores EAX) | cold |
| GCA | probe_gca | `0x67B204` 10 `8B 4D F8 51 8B 0D A8 B4 A1 00` | 0x67B100 cmd 522 | ebp. Object id `[EBP-4]` (`0067b13c`), result `[EBP-8]` (0xFFFF init `0067b109`, 0xFFFE empty `0067b1fd`). `[EBP-0xC]`, `[EBP-0x14]` and `[EBP-0x20]` are uninitialised on the early paths (`0067b14c`/`0067b161` JZ): never read them | warm: per script call |
| PARTY | probe_party | none (tick + PF) | - | Server table `0x51C8D0`(serverApp) = `*(serverApp+4)+0x1F0B4`: [+0] count, member npc idx at +8+4i, NPC object ids at +0x1C+4*idx. `0x51C8F0`(serverApp) = the player's object. Client controlled id = the existing `controlledId()` | event-driven |
| A1X | probe_a1x | none (CompanionAiA1Gate, E2, RS) | - | Creature +0x524 (combat-exit ms), +0x570, +0x520, +0x94 position; candidate +0x94 | per A1 event |

Cost bound. The KPM wrapper (register save + FXSAVE) is about 0.1-0.3 us per call; estimates (verify with perf-lab):
- P2 ~1.5-3.5k calls/s, RAR ~1-5k/s, RS <= 300/s, FM3S+FM3E ~440/s: total <= ~2.5 ms/s (about 0.25 % of one core).
- An INI 0 does **not** remove the wrapper. If perf-lab shows UpdateState or tick time +5 % against the last baseline, rebuild without P2/RAR.

**Dropped from the catalogue set (budget 14, used 13):**
- RA-D `0x5D5C22`: RAR sees every dispatch together with its type.
- SHOUT-RX `0x57074F`: RS S7 sees every delivered shout, with listener `+0x1A0` speaker and `+0x1BC` pattern.
- SHOUT-TX `0x547100`: SHX at the single exit sees every call with its arguments and the exit kind.
- FM5 six post sites: RS S12 counts OnBlocked script runs per creature, which is the cost a throttle would remove. Per-site split is only needed when the throttle is built (all six then anyway).
- SS2 `0x59E8F4`, AI2-S6 `0x58F48C`: RS S6C / S6A.
- FM2d `0x55B625`: redundant with FM2C.
- EV `0x56EFF9`, AI3-H5 `0x5D0F30`: HOT for every event / update; P2 + RS S1 answer the cadence.
- AI2-RT/AI2-RNG: A1X computes the strict test inside the shipped A1 handler.
- FM6 `0x7E6546`: partly refuted; no open decision.

## 2. Log lines (exact formats), caps, questions, kill/promote, analyzer checks
All lines: `tline` (per-second cap) for `PR`, `eline` for `PRS` summaries (every `summary_sec`, only if changed) and A1X events.
Per-probe session budget `probe_lines=400` (then summary only, and one marker `-- probe <ID> line budget reached --`; the probes are FM2C FM2F FM2R FM3 RS P2 PF RAR SHX SS1 GCA PARTY A1X A1XF). The analyzer judges an ABSENCE (no FM2C/FM2F, no S6C, no FOLLOWUP) only while the probe's budget held for the whole window and no per-second-cap drop marker overlaps it (PR-SS and PR-E9 use the budget-free PRS counters). Filter = `crPasses()`
(party, tags, log_all), except SHX/FM3 which log every subject. Check semantics: PASS = the data agrees with the atlas
(no action), FAIL = it contradicts the atlas or shows a bug's reach (act on it), OBSERVE = the case did not occur.

| id | trace line(s) (printf) | cap | question; kill / promote | analyze_play_logs.py check (name: rule) |
|---|---|---|---|---|
| FM2C | `t=%lu PR FM2C cr=%08x tgt=%08x res=%d code=%d cached=%d cp=%d calls=%d n2=%u n3=%u` (n2/n3 = res counts for this (cr,tgt) since the last 30 s of silence for that pair, not a sliding window) | first 10 per pair, then on res change or 5 s | Is leg 1 partial (2) and the re-plan FAIL (3)? share of cached -1. Only res=2 for a looping pair = creeping-partial loop: fail-fast must act at `0x5BC5F4`. No line in the ledge fight = the grid closest result succeeds: kill FM2b | `PR-FM2B`: PASS = >= 1 line, all res in {2,3}; FAIL = other res or cr 00000000; OBSERVE = none |
| FM2F | `t=%lu PR FM2F cr=%08x tgt=%08x grp=%u n=%u span=%lums ctrl=%d` (n = FAIL legs for the pair since the last 12 s of silence, not a sliding window; the throttle signature is n itself, so after the first 20 it logs every leg, bounded by the budget) | first 20 per creature, then on n change | How often the 2-4 s wait really runs; promote the optional wait skip if n >= 2 per pair | (inside `PR-FM2`) |
| FM2R | `t=%lu PR FM2R cr=%08x tgt=%08x other=%d inr=%d range=%.2f d=%.2f prog=%.2f n=%u` (d = 3D distance via +0x94; prog = previous d - d) | first 20 per pair, then every 5th n | Loop rate. Promote the FM2 fail-fast if a party pair reaches n >= 3 in 12 s with prog < 0.5 each; kill if max n < 3 in the ledge fight | `PR-FM2`: PASS = a pair with n >= 3 and FM2C or FM2F lines for that cr in the same 30 s; FAIL = n >= 3 with no FM2C for that cr (contradicts gate 1.1); OBSERVE = no pair n >= 3 |
| FM3 | `t=%lu PR FM3L level=%d sentinel=%08x now=%d removed=%d done=%d elapsed=%llu slice=%llu`; `t=%lu PR FM3X id=%08x level=%d idx=%d cursor=%d sentinel=%d`; `t=%lu PRS FM3 passes=%u lost=%u removes=%u beforeCursor=%u` | FM3L <= 200/session; FM3X first 50, then 1/s | LOST rate. Kill if < 1 LOST/min in crowded fights; >= 1/min goes to the user (enemy scheduling) | `PR-FM3`: PASS = PRS FM3 passes > 0 (report lost/min); FAIL = an FM3L with now==level and removed=0; OBSERVE = none |
| RS | `t=%lu PR RS site=%s cr=%08x %s party=%d empty=%d`; S7 adds ` spk=%08x spkParty=%d pat=%d spkTag=%s`; `t=%lu PRS E9 deliveries=%u` (budget-free count of pat-15 deliveries from non-party creatures to party listeners; printed at the first summary even when 0, then on change); `t=%lu PRS RS cr=%08x s6a=%u s6b=%u s6c=%u s7=%u s12=%u s1=%u max12ps=%u` | first 20 per (cr, site); S7: first 10 per creature (50/session) plus every E9-case line (300/session, own budget) | (a) slot-6 site mix per companion (M8 cadence, SETSTATE-SLOT6 reach); (b) OnBlocked runs/s: kill FM5 if every creature < 5/s over any 2 s, promote if >= 5/s; (c) E9: S7 pat=15 from non-party speakers to party listeners | `PR-SLOT6`: PASS = any S6A/S6B for a party member (report the mix); OBSERVE = none. `PR-FM5`: PASS = max12ps < 5 (kill), FAIL = >= 5 (promote), OBSERVE = no S12. `PR-E9`: PASS = `PRS E9 deliveries` > 0 (the S7 speaker tags are listed so the user can mark which are allies; enemy shouts count too); FAIL = SHX shows r=35 non-party creature shouts (`type=5`) and `PRS E9 deliveries=0`; OBSERVE = no allied shouts or no PRS E9 line |
| P2 | `t=%lu PR P2 cr=%08x ctrl=%d elapsed=%u combat=%d`; `t=%lu PRS P2 cr=%08x runs=%u avg=%u max=%u ctrlRuns=%u` | first 10 per creature | Real companion cadence; is a player-controlled companion also on 4 s (review E1)? ctrlRuns > 0 supports keying p2 on the controlled id | `PR-P2`: PASS = every companion avg in [3900,4500]; FAIL = any avg < 3500; OBSERVE = none (report ctrlRuns) |
| PF | `t=%lu PR PF cr=%08x %s old=%d new=%d apply=%d site=%s`; `t=%lu PRS PF saveA=%u saveB=%u xferA=%u xferB=%u other=%u` | non-transient sites: all; transient: party members with old != new only | Who writes +0x11AC and when; transient pairs per save | `PR-PF`: PASS = saveA == saveB and no `site=?`; FAIL = otherwise; OBSERVE = no PF |
| RAR | `t=%lu PRS RAR cr=%08x ispc=%d n=%u %s` (%s = up to 8 `type:r1/r2/r3/r4` by count, plus the type-0 bucket whenever it is non-zero); `t=%lu PR RAR MOVESTATE cr=%08x type=%x res=%d state=%d` (type 1/0x11 while u8 [cr+0xFF5] != 0); `t=%lu PR RAR REQ cr=%08x type=%x n=%u` (same type returned 1 >= 200 times in a row) | MOVESTATE first 20, REQ first 10 | Histogram; does a queued move walk under a SetState (GATE_wave2 open item); stuck pending actions | `PR-RAR`: PASS = PRS RAR present (report MOVESTATE/REQ); FAIL = type 0 or > 0x47 with result != 3; OBSERVE = none |
| SHX | `t=%lu PR SHX %s spk=%08x type=%d %s party=%d r=%.0f seen=%d idx=%d count=%d last=%08x text=%s` (%s = end, xbreak, noarea, ABORT-START, ABORT-MID); `t=%lu PRS SHX shouts=%u r35=%u partySpk=%u end=%u xbreak=%u noarea=%u abortStartCr=%u abortStartOther=%u abortMid=%u` (type = speaker object type, 5 = creature; party is read only for creatures; abortStartCr/Other split by type 5) | first 100 ABORT, then 1/s per kind; first 30 others | AI-SHOUTABORT reach; attempt counts for E9 | `PR-SHOUT`: PASS = shouts > 0 and no creature ABORT; FAIL = `abortStartCr + abortMid` > 0 or an ABORT-START line with `type=5` or any ABORT-MID (reach shown: promote the `0x547203` fix); `abortStartOther` (non-creature speaker outside the area list, the vanilla no-listener case) is INFO only; OBSERVE = no shouts |
| SS1 | `t=%lu PR SS1 cr=%08x %s state %d->%d combat=%d` | party, first 50 | Which SetStates hit companions; is the S6C slot-6 run seen afterwards | `PR-SS`: PASS = every party SS1 to a state in {1,3..10,0xE..0x13} has an RS S6C for that cr within 60 s; FAIL = none while the cr lives; OBSERVE = no SS1 |
| GCA | `t=%lu PR GCA cr=%08x res=%d lastRun=%x moving=%d` (lastRun = RAR's last type for cr; moving = [cr+0x11BC]); `t=%lu PRS GCA cr=%08x %s` | on res change per creature, first 40 | E4 key: expect 3/39 during engine approaches, 38 while following, 0 only for script moves | `PR-E4`: PASS = >= 1 res in {3,39} with lastRun in {0xC,0x11,0x13,0x3F} and no FAIL; FAIL = res 0 with lastRun in that set (contradicts lesson 325); OBSERVE = none |
| PARTY | `t=%lu PR PARTY why=%s ctrl=%08x f=%d/%d pobj=%08x f=%d/%d pc=%08x tbl=%d [%s]` (f = +0x11AC/+0x10E8; members `idx:id:f11ac:ispc`; why = area, leader, pf, load) | 100/session | Party flag true for every member? Is `0x51C8F0` the controlled one (review E2)? | `PR-PARTY`: PASS = every member and ctrl f11ac=1 (outside saves); FAIL = a member/ctrl with 0; report `pobj==ctrl` after a leader switch |
| A1X | A1 line suffix ` c524=%d f570=%d combat=%d`; `t=%lu PR A1X STRICT cr=%08x new=%08x d2=%.2f lim2=%.2f -> %s` (ok / would-reject; per creature the first 10, then on an ok/would-reject change or every 5 s; always counted in the `A1 summary ... strict=N/rejected` field); `t=%lu PR A1X FOLLOWUP cr=%08x site=%s after=%ldms` (site S6A/S6B/S6C/E2); `t=%lu PR A1X ORPHAN cr=%08x since=%ldms` (FOLLOWUP/ORPHAN have their own budget `A1XF`, so STRICT cannot use it up) | events (eline) | Combat lapse after A1 end (R1); retarget metric (review section 4) | `PR-A1X`: PASS = every A1 end has a FOLLOWUP within rethink+1000 and no ORPHAN; FAIL = any ORPHAN; report the would-reject count. `PR-RAH2`: ranged-ai H2 `retarget=1`, then H1 `end52c!=0 inRange=0` for the same cr within 2 s; PASS = none, FAIL = seen |

## 3. Handler rules (Sonnet build notes)
- Entry order:
  - hot handlers (P2, RAR, RS): `if (!g_cfg.probeX) return;` -> cheapest test -> `DWORD now=GetTickCount(); tick(now);` -> work;
  - cold handlers: `saferead::beginScope(); tick(now);` first, as the shipped ones do;
  - only engine-validated pointers are read directly (P2 cr, RAR obj, SS1 cr, PF cr); everything else goes through `rd()`/objInfo.
- **RS**: `switch (rsSite(ret))`, return on 0. Then `objInfo(id, now)` (1 s cache), party test. S7 speaker = `rd(obj,0x1A0)`, pattern = `rd(obj,0x1BC)` (obj from `gameObject(id)`). A1X: a slot-6 site clears that creature's `pendingA1`.
- **FM3**: as R1 pseudocode (CompanionAiFm3Sentinel/LevelEnd/Remove), log-only. Never write `[ebp-0x40]` or the cursor.
- **SHX**: kind = `shoutExit(area != 0, idx, count, gameObject(last) != 0)`; read `count` only if area != 0. Text = `copyStr` of `[EBP+8]` (32 chars).
- **PARTY**:
  - at most 1 per trigger: area change of the controlled creature (FnGetArea, checked in tick at <= 1 Hz), a controlled-id change, a non-transient PF write, the first tick after load;
  - calls `0x51C8D0`/`0x51C8F0` (both thiscall serverApp = `[[0xA1B4A4]+8]`, no args, plain RET; serverApp passed at `00569681`) and reads the table with `rd()`;
  - `pc` = the last creature seen with `+0x10E8 == 1` (RAR/P2/RS/PARTY).
- **A1X**:
  - in `CompanionAiA1Gate` after the pick, if newId valid: `d2 = 2D dist2(+0x94 x,y)`, `lim2 = (mx+0.1)^2`; log STRICT. The behaviour is unchanged (strict=0);
  - append `c524/f570/combat` to the A1 line (A1_RE has no `$`, so it still matches);
  - set `pendingA1 = now` on end/retarget; clear it in RS S6* and in E2; ORPHAN from `tick` when `a1OrphanDue`.
- Version `0.2.0-probe` in CompanionAI.cpp and manifest.toml. Header comment lists the 13 new hooks.
- The INI gets a `; ===== probes (log only) =====` block with all 13 flags `=1` + `probe_lines=400`.
- Config defaults: probe flags **1** (the probe build ships them on; `testShippedIni` requires defaults == file).

## 4. CompanionAiLogic.h pure helpers (+ host tests in companion_ai_logic_test.cpp)
| helper | test cases |
|---|---|
| `Config` += `probeFm2c, probeFm2f, probeFm2r, probeFm3, probeRs, probeP2, probePf, probeRar, probeShx, probeSs1, probeGca, probeParty, probeA1x` (bool, default 1), `probeLines` (int 400, clamp 0..100000); `parseConfig`/`sameConfig`/`logConfig` extended | shipped INI parses with `bad==""`; `known[]` += 14 keys (`g_t.n` == 50); `probe_rs=0` -> false; `probe_lines=x` -> 400 + bad |
| `int rsSite(unsigned ret)` -> 0 none, 1 S6A, 2 S6B, 3 S6C, 4 S7, 5 S12, 6 S1; `const char* rsName(int)` | 0x58F514->1, 0x569A6C->2, 0x59E909->3, 0x5709F5->4, 0x57074A->5, 0x570645->6, 0x57069F->6, 0x58F50F->0, 0->0 |
| `int pfSite(unsigned ret)` (15 returns = the CALL sites +5: 0x574537 SAVE_A, 0x575170 SAVE_B, 0x599AF4 EFF89, 0x5F89AA XFER_A, 0x5F8A13 XFER_B, 0x5F8EB9 ADDMEMBER, 0x5F93B4 CREATEPARTY, 0x5FEAC7 SWITCHPC, 0x6306A5 CHARGEN, 0x630B17 PLAYERLOAD, 0x66CA5A CMD574, 0x6900C1 CMD575, 0x69B901 CMD846, 0x89FE45 SELECT_DEL, 0x8A050E SELECT_ADD), `pfTransient(site)` | each of the 15 maps; 0x574532 -> -1; transient only for SAVE_A/B, XFER_A/B |
| `bool p2Runs(int isPC, int mode, unsigned elapsed)` | (0,1,4000)=1, (0,1,3999)=0, (1,1,9000)=0, (0,2,9000)=0 |
| `int shoutExit(bool area, int idx, int count, bool lastOk)` -> NoArea/AbortStart/End/XBreak/AbortMid | (0,..)->NoArea; (1,-1,5,1)->AbortStart; (1,5,5,1)->End; (1,2,5,1)->XBreak; (1,2,5,0)->AbortMid |
| `const char* fm2cReason(int cp, int cached, int res)` | cp=1 -> "client"; cached=-1 -> "code-1"; res=2 -> "partial"; res=3 -> "blocked" |
| `bool fm3Lost(int level, int nowLevel, bool removed)` | (4,4,0)=0, (4,2,0)=1, (2,2,1)=1 |
| `bool a1Strict(float ax,float ay,float tx,float ty,float maxR)` = dx²+dy² <= (maxR+0.1)² | on the bound = 1; +0.01 over = 0; maxR 0, d 0.05 = 1; d 0.2 = 0 |
| `struct PairCount {int cr,tgt; unsigned n,first,last; float lastD;}` + `bool pairBump(PairCount&, int cr, int tgt, unsigned now, unsigned windowMs)` (returns true on reset) | same pair within window: n++; other tgt: reset n=1; gap > window: reset; wraparound of `now` |
| `bool a1OrphanDue(unsigned now, unsigned endAt, int rethinkMs)` | endAt 0 -> 0; now-endAt = rethink+1000 -> 0; +1001 -> 1 |
| `int rarIndex(int type, int res)` = (type in 1..0x47 ? type : 0)*5 + (res in 1..4 ? res : 0) | (1,2)=7; (0x47,4)=359; (0x48,1)=1; (5,9)=25 |
| `struct ProbeCap {unsigned used=0; bool take(unsigned budget);}` | 400 takes true, the 401st false, the marker once |
| `testExports` | funcs 18 -> **31**, distinct exports 16 -> **29**; replace `!strstr(t,"Fm3")` with `!strstr(t,"0x0051DBE7")` (the hot probe stays unshipped) |

## 5. exports.def additions (one per line, after CompanionAiA3)
`CompanionAiProbeFm2c CompanionAiProbeFm2Fail CompanionAiProbeFm2Reapproach CompanionAiProbeFm3Sentinel CompanionAiProbeFm3LevelEnd
CompanionAiProbeFm3Remove CompanionAiProbeRunScript CompanionAiProbeP2 CompanionAiProbePartyFlag CompanionAiProbeRunActions
CompanionAiProbeShout CompanionAiProbeSetState CompanionAiProbeGetCurrentAction`. Signatures are `extern "C" void __cdecl`, taking `(int ebp)`,
except `CompanionAiProbeRunScript(int ret, int script, int objId)`. Analyzer: `EXPECTED_HOOK_COUNTS["companion-ai"] = 31`.

## 6. check_windows.py rows (mods678_research/check_windows.py, after the CA FM3 NOT SHIPPED row; shipped=True)
```python
    W("CA P FM2C partial-vs-fail",      0x55B69A, 6,  0x55AC80),   # MOV ECX,[0x99B5A0]; [EBP-0x18] res 2/3
    W("CA P FM2F fail leg (log-only)",  0x5C2438, 9,  0x5BFC00),   # plain this round (no consumed exit)
    W("CA P FM2R 0x11 re-approach",     0x5BC5F4, 9,  0x5BC1D0),   # start = JNZ 0x5BC5E2 / JZ 0x5BC5E8 target
    W("CA P FM3S sentinel set",         0x51DBE1, 6,  0x51D790),   # ends at 0x51DBE7 (the unshipped hot probe)
    W("CA P FM3E level end",            0x51DD33, 6,  0x51D790),   # stolen SUB feeds the SBB (stolen bytes last)
    W("CA P FM3R RemoveObject",         0x51E0C9, 6,  0x51E0C0),
    W("CA P RS RunScript entry",        0x6FD8D0, 6,  0x6FD8D0),   # PUSH EBP; MOV EBP,ESP; SUB ESP,8
    W("CA P P2 party cadence",          0x56BE79, 6,  0x56B8A0),   # HOT; 144 B from CA P1 stamp 0x56BDE3
    W("CA P PF party flag write",       0x58A35E, 6,  0x58A330),
    W("CA P RAR RunActions result",     0x5D6A60, 9,  0x5D56B0),   # HOT; stolen CMP feeds JNZ
    W("CA P SHX shout loop exit",       0x5475E8, 10, 0x547100),   # SEH unlink, absolute FS store
    W("CA P SS1 SetState commit",       0x59D966, 6,  0x59CBA0),   # stolen reads AL
    W("CA P GCA GetCurrentAction",      0x67B204, 10, 0x67B100),   # stolen PUSH ECX
```

## 7. kotor2-steam-aspyr.hooks.toml additions (append after the A3 hook; same format; one `#` comment line per hook = the table's id+purpose)
Every hook is `skip_original_bytes = false`, `exclude_from_restore = []`, with no consumed exit. Reword the trailing FM3 comment:
the hot `0x51DBE7` stays unshipped, and the cold FM3S/E/R probes ship.
```toml
[[hooks]]
address = 0x0055B69A
type = "detour"
function = "CompanionAiProbeFm2c"
original_bytes = [0x8B, 0x0D, 0xA0, 0xB5, 0x99, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
[[hooks]]
address = 0x005C2438
type = "detour"
function = "CompanionAiProbeFm2Fail"
original_bytes = [0x8B, 0x95, 0x6C, 0xFA, 0xFF, 0xFF, 0x89, 0x51, 0x70]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
[[hooks]]
address = 0x005BC5F4
type = "detour"
function = "CompanionAiProbeFm2Reapproach"
original_bytes = [0x8B, 0x4D, 0xE4, 0x8B, 0x91, 0x24, 0x01, 0x00, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
[[hooks]]
address = 0x0051DBE1
type = "detour"
function = "CompanionAiProbeFm3Sentinel"
original_bytes = [0x8B, 0x4D, 0xD8, 0x89, 0x4D, 0xC4]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
[[hooks]]
address = 0x0051DD33
type = "detour"
function = "CompanionAiProbeFm3LevelEnd"
original_bytes = [0x8B, 0x4D, 0xD0, 0x2B, 0x4D, 0xF0]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
[[hooks]]
address = 0x0051E0C9
type = "detour"
function = "CompanionAiProbeFm3Remove"
original_bytes = [0x8B, 0x45, 0x08, 0x8B, 0x48, 0x78]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
[[hooks]]
address = 0x006FD8D0
type = "detour"
function = "CompanionAiProbeRunScript"
original_bytes = [0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "[esp+0]"
type = "int"
[[hooks.parameters]]
source = "[esp+4]"
type = "int"
[[hooks.parameters]]
source = "[esp+8]"
type = "int"
[[hooks]]
address = 0x0056BE79
type = "detour"
function = "CompanionAiProbeP2"
original_bytes = [0x8B, 0x85, 0x2C, 0xFF, 0xFF, 0xFF]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
[[hooks]]
address = 0x0058A35E
type = "detour"
function = "CompanionAiProbePartyFlag"
original_bytes = [0x89, 0x88, 0xAC, 0x11, 0x00, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
[[hooks]]
address = 0x005D6A60
type = "detour"
function = "CompanionAiProbeRunActions"
original_bytes = [0x8B, 0x55, 0xE4, 0x3B, 0x15, 0xC0, 0x44, 0x99, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
[[hooks]]
address = 0x005475E8
type = "detour"
function = "CompanionAiProbeShout"
original_bytes = [0x8B, 0x4D, 0xF4, 0x64, 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
[[hooks]]
address = 0x0059D966
type = "detour"
function = "CompanionAiProbeSetState"
original_bytes = [0x88, 0x82, 0xF5, 0x0F, 0x00, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
[[hooks]]
address = 0x0067B204
type = "detour"
function = "CompanionAiProbeGetCurrentAction"
original_bytes = [0x8B, 0x4D, 0xF8, 0x51, 0x8B, 0x0D, 0xA8, 0xB4, 0xA1, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
```

## 8. Static gates (all must pass before the Opus pre-install review)
1. `mods678_research/check_windows.py`: ALL OK, `grep -c WARN` = 0, 46 windows (33 + 13), TOML cross-check all match.
2. `verify_windows_both_exes.py`: ALL MATCH.
3. Host test `companion_ai_logic_test.cpp`: ALL PASS. `analyze_play_logs.py` tests: new fixture lines per `PR-*` check, plus the old 35.
4. `i686-w64-mingw32-g++ -fsyntax-only -Wall -Wextra`: clean.
5. exports.def == the toml functions (31 hooks / 29 names).
6. The kpatch's toml is byte-identical to the source toml, and `tomllib` loads it.
7. Sibling self-patch sites (`0x40D69E 0x8ABA8C 0x8ADA08 0x8ADA81 0x8ADFA2 0x8AE02D 0x8AE19A 0x8AE1CD 0x8AE793`) are all > 12 B from every window, and the live config is re-read at install.
8. Consumed exits: **none added** (FM2F is plain on purpose), so there is no new stack-depth hand-check. The 4 shipped consumed exits are unchanged.

## 9. Play protocol (one session, about 40 min; note the clock time at each step)
0. **Before the session (perf-lab A/B, required by section 1 "Cost bound", +5 % rule):** run perf-lab on the same save twice, once with
   `probe_p2=0 probe_rar=0 probe_rs=0` in `companion_ai.ini` and once with all probes on (hooks stay installed in both: this measures the
   handler cost). If UpdateState or tick time is > 5 % worse with the probes on, rebuild without P2/RAR before playing. Keep both perf-lab logs.
1. **Unreachable enemy (FM2):** a fight with an enemy on a ledge, turret platform or behind a fence that the party cannot path to. Two companions on Aggressive, one on Ranged (11). Give no orders for 60 s, then 30 s with the leader also attacking it.
2. **Leader switch (P2, PARTY, review E1/E2):** switch control to a companion out of combat and again mid-fight; with the controlled
   companion (Kiting 16 if available) order one shot at an enemy behind an ally; switch back.
2b. **Party-selection swap (PF non-transient sites, PARTY `why=pf`):** open the party selection screen (a bench or the Ebon Hawk), swap one
   companion out and another in, and leave; this is the only step that writes the party flag through ADDMEMBER / SELECT / the party table
   sites. Note the clock time.
3. **Save / load (PF, FM4):** quick-save in an area with many NPCs, load it, walk 30 s.
3b. **Explicit area change (PARTY `why=area`, P2 gap > 30 s):** walk through a door or transition to another area and back, with the
   party following, and stay 30 s in each. Note the clock time of each transition.
4. **Allied shout fight (E9, SHX, RS S7):** a fight where friendly NPCs fight hostiles (e.g. Telos/Onderon soldiers, Mandalorians). Keep the party idle within 15 m of the allies for about 20 s, then fight.
5. **Normal combat (RAR, GCA, SS1, A1X, FM5):** 2-3 ordinary fights, at least one with a stun, horror or choke on a companion; one
   where a companion follows through a door or a narrow corridor (OnBlocked); one with a Ranged companion whose target is out of range (A1).
6. Close the game and copy out `$G/companion-ai_log.txt` (`ai_trace.txt` on an old core), `$G/companion_ai.ini` (the config is logged, but the file settles any doubt),
   `$G/ranged_ai_log.txt`, `$G/inventory_sort_filter_log.txt`, `~/steam-208580.log` (and perf-lab's logs, both A/B runs, if present) to
   `~/Games/KOTOR2_Mod_Workspace/playlogs_probe_<MMDD_HHMM>/`. Then run
   `python3 mods678_research/analyze_play_logs.py <dir>` and read the `PR-*` rows plus the per-probe PRS summaries.

**Decisions this round feeds:** FM2 fail-fast site and N (PR-FM2/FM2B); FM3 fix yes/no (user); FM5 throttle yes/no; P2 key
(controlled id) and interval; E9 build yes/no; `a1_retarget_strict` / `a1_keep_combat_ms`; the E4 key (PR-E4); the AI-SHOUTABORT
fix; the ranged-ai IsPC -> controlled-id fix (step 2).
