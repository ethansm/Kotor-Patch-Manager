# Pre-install review: companion-ai 0.2.0-probe (commit 8736ec4), 2026-10-02

Scope: `git diff 8736ec4~1 8736ec4 -- companion_ai_src mods678_research/analyze_play_logs.py mods678_research/check_windows.py`,
the full CompanionAI.cpp / CompanionAiLogic.h / companion_ai.ini / hooks.toml / exports.def / manifest.toml, PROBE_DESIGN.md, PROBE_BUILD.md,
and the dump asm `steam_6a522e71/decomp/<bb>/<fn>.asm` around every window. Static only: nothing built, installed or launched.

## Verdict: READY AFTER FIXES (0 BLOCKER, 5 SHOULD-FIX, 9 NIT)

Nothing here can crash the game or change game state. The windows, stolen bytes, parameter sources and handler signatures are all
correct, and an INI with every probe off behaves like 0.1.0. The SHOULD-FIX items are about the data, not safety. Two C++ items
(SF1, SF2) would otherwise lose or mislabel data on the one 40-min session the probe round is built for. SF3 makes the E9 answer
usable. SF4/SF5 are analyzer-only and can also be fixed after the session, because the analyzer runs offline on the copied logs.
If you deploy before the fixes (the rollback is safe), apply SF4 and SF5 before reading any `PR-*` FAIL.

## 1. Windows / toml / parameters (checked against the .asm)
| id | stolen bytes = asm | inbound targets | frame / register facts confirmed in asm | flags after the window |
|---|---|---|---|---|
| FM2C 0x55B69A | `8b0da0b59900` ok | 0x55B407 JNZ, 0x55B623 JZ, fallthrough 0x55B697 | res `[EBP-0x18]`; code `[EBP-0x20]` set on both 0x55AF89/0x55AFA3 paths; PI `[EBP+8]` +0x2C passed as an id to the server lookup (0x55B176..0x55B194) | none read |
| FM2F 0x5C2438 | `8b956cfaffff 895170` ok | - | cr `[EBP-0x5A8]` = ECX at 0x5BFC28; `[EBP-0x598]` = cr+0x380 (pathfind info) = ECX at the window; node+0x6C u16 at 0x5C2479 | none |
| FM2R 0x5BC5F4 | `8b4de4 8b9124010000` ok | 0x5BC5E2 JNZ, 0x5BC5E8 JZ | cr `[EBP-0xA4]` (0x5BC1D9); tgt `[EBP-0x28]` <- node+0x38, range `[EBP-0x3C]` <- node+0x3C (memcpy 0x5BC25B..0x5BC27B); other `[EBP-0x40]` (0x5BC457); inR `[EBP-0x2C]` (0x5BC4B5) | none |
| FM3S 0x51DBE1 | `8b4dd8 894dc4` ok | ends at the 0x51DBDF JNZ target 0x51DBE7 | level `[EBP-0x34]` (0x51DBC3 SHL 4); first obj `[EBP-0x28]` = object pointer from 0x51D1C0 (or 0) | none |
| FM3E 0x51DD33 | `8b4dd0 2b4df0` ok | 0x51DCF3, 0x51DCF9, 0x51DD22 | now `[EBP-0x30/-0x2C]`, start `[EBP-0x10/-0xC]`, slice `[EBP-0x20/-0x1C]` | stolen SUB feeds the SBB at 0x51DD3C (stolen bytes are last: ok) |
| FM3R 0x51E0C9 | `8b4508 8b4878` ok | - | list = master+4+16*level: +0 id array, +4 count, +0xC cursor (0x51D1CC..0x51D20B); ids are object ids (0x51D224) | none |
| RS 0x6FD8D0 | `55 8bec 83ec08` ok | - | `[EBP+8]` CExoString* (0x6FD8D9), `[EBP+0xC]` object id; all 7 call sites push (slot ptr, `[obj+4]` id, 1); RET 0xC | SUB re-sets |
| P2 0x56BE79 | `8b852cffffff` ok | 0x56BE68 JZ, 0x56BE6E JNZ | cr `[EBP-0xD4]`, mode `[EBP+8]`, elapsed `[EBP-0x18]` | none |
| PF 0x58A35E | `8988ac110000` ok | - | cr `[EBP-0x48]` = EAX, new `[EBP+8]` = ECX, ret `[EBP+4]`; 15 `CALL 0x58A330` in the dump = the 15 pfSite returns (each checked) | CMP re-sets |
| RAR 0x5D6A60 | `8b55e4 3b15c0449900` ok | - | obj `[EBP-0x1A8]` = this (0x5D56D9; the engine wrote +0xAC/+0xB0 just before); type `[EBP-0x18]`; res `[EBP-0x1C]` | stolen CMP feeds JNZ 0x5D6A69 (last: ok) |
| SHX 0x5475E8 | `8b4df4 64890d00000000` ok | 5 exits: 0x547180, 0x5471B5, 0x5471C4, 0x547203, 0x547266 | speaker `[EBP-0xC0]`, area `[EBP-0x30]`, list `[EBP-0x14]`, idx `[EBP-0x1C]` (incremented at 0x5471D6 before the 0x547203 test), last `[EBP-0x10]` | none (POP/RET) |
| SS1 0x59D966 | `8882f50f0000` ok | - | cr `[EBP-0x20]`, new `[EBP-0x1C]` (AL); old u8 +0xFF5 read before the store | ADD re-sets |
| GCA 0x67B204 | `8b4df8 51 8b0da8b4a100` ok | 0x67B14C JZ, 0x67B1FB JMP, fallthrough 0x67B1FD | id `[EBP-4]`, res `[EBP-8]` (0xFFFF at 0x67B109, 0xFFFE at 0x67B1FD); `[EBP-0xC/-0x14/-0x20]` not read | none |

- All 13 are `skip_original_bytes=false`, `exclude_from_restore=[]`, no `consumed_exit_address` (tomllib dump: 31 hooks). The handler
  signatures match the toml: `(int ebp)` x12 and `RunScript(ret, script, self)` = `[esp+0]`, `[esp+4]`, `[esp+8]` in toml order, the
  same convention as the live LevelUpOverrideEntry and PerfLabResFreeData.
- Item 7 (what the checker cannot see): every window either has no flags reader before the next flags writer, or its own stolen
  instruction is the flags producer (FM3E, RAR). So no window depends on PUSHFD/POPFD. ECX/EDX/AL payloads (FM2F ECX, PF EAX/ECX,
  SS1 EDX/AL, RS ECX=VM) are restored by the plain-detour wrapper before the stolen bytes run. GCA's stolen `PUSH ECX` runs in the
  trampoline after the restore. FXRSTOR restores the x87 state, and no window sits inside a live x87 sequence.
- P2 nuance (no defect): the window is not reached on the `+0x38C==0` path (first-run flag, once per spawn, AI3 atlas) or on the
  IsPC 200 ms path (0x56BE77 JNC). The probe measures the non-PC 4 s cadence, which is the question.

## 2-5. Crash safety, log volume, INI, behaviour
- Crash safety: OK. Hot handlers (P2, RAR) dereference only the creature the engine touches in the next instruction, after the
  type/null tests. Everything else goes through `rd()`/`probeRead` (FM3R bounds the array at 4096 ids, then 1 probe). `copyStr` caps
  at 32 chars. The snprintf accumulators (rarFormat 400/48, GCA 160/24, PARTY 300/48) stop before a write can overflow. Engine calls
  are pure getters (0x51C0A0, 0x5453C0, 0x51C8D0, 0x51C8F0 -> 0x539D60 -> 0x51C620, client chain), under the same server-pointer
  assumptions 0.1.0 already makes.
- Hot paths: with the flag off, P2/RAR/RS cost 1 compare. With it on, a non-party creature costs 3-4 loads (P2/RAR) and RS costs 7
  compares, then return. The party/site path costs GetTickCount + tick, then bounded tables (16/32 LRU, MRU for RAR). No allocation;
  no I/O except through vemit.
- Threads: the same single-main-thread assumption as 0.1.0 (N8); vemit's critical section is unchanged.
- Log volume: PR lines are capped at 13 x `probe_lines` = 5200 per session (about 0.8 MB). PRS output is at most about 83 lines per
  10 s with log_all=1 (about 125 KB/min), and about 15-30 KB/min with party_only. Exempt from every budget and per-second cap: the
  SHX ABORT lines (SF1) and the PRS lines. Every eline/pev line, PRS included, also calls fflush (N8).
- INI: the 13 `probe_*` keys use getBool, and `probe_lines` uses `clampi(getInt, 0, 100000)`. All are in sameConfig and logConfig.
  Hot reload works, and so does raising probe_lines after a budget marker. The shipped INI documents every key; defaults == file.
- All probes off: identical behaviour to 0.1.0. The A1 line suffix is "" (same text). E2, A1 STRICT/pending, partyTick and
  orphanTick are flag-gated. probeSummary only does memcmp work. The only new outputs are the `# config probes` line and an extra
  `+0x10E8` read in objInfo. No probe writes frame or engine memory: A1X/PARTY only read, and FM3 never writes `[ebp-0x40]` or the
  cursor.

## Findings
| # | sev | file:line | finding |
|---|---|---|---|
| SF1 | SHOULD-FIX | CompanionAI.cpp:1355-1363 | SHX ABORT lines use `eline` (force): no budget, no per-second cap, and an fflush per line. If a stale id sits in an area list, every shout in that area aborts (in combat, about 10-30 shouts/s, about 0.3 MB/min, 30 flushes/s). Also, ABORT-START counts placeable/door speakers that are not in the creature list, and `rd(spk,0x11AC)` reads non-creature memory, which inflates partySpk. |
| SF2 | SHOULD-FIX | CompanionAI.cpp:891-904 | A1X STRICT is logged on every retarget pass, unthrottled, from the same 400-line A1X budget as FOLLOWUP/ORPHAN. In a long ranged fight the budget runs out mid-session, and after that every A1 `end` has no FOLLOWUP: PR-A1X reports a false FAIL and the R1 combat-lapse data is lost. |
| SF3 | SHOULD-FIX | CompanionAI.cpp:1242-1250; analyze_play_logs.py:1261-1268 | E9 evidence is unusable. S7 lines are capped at 20 per creature and 50 per session, which the step-1 fight uses up before the step-4 allied fight. The S7 suffix cannot tell hostile from allied speakers, so a PASS can come from enemy shouts. FAIL (`nonparty35`) counts any non-party r=35 SHX line, enemies included. |
| SF4 | SHOULD-FIX | analyze_play_logs.py:1214, 1350, 1419, 1265 | The FAIL rules that depend on absence (PR-FM2 "no FM2C/FM2F", PR-SS "no S6C", PR-A1X "no FOLLOWUP", PR-E9) ignore the `-- probe X line budget reached --` markers (no t=, only a line number) and the `-- ai_trace: N lines dropped --` per-second-cap markers. Once the RS budget (400, reached early by S1/S12/S7 lines) is spent, every later SS1 becomes a false PR-SS FAIL. |
| SF5 | SHOULD-FIX | analyze_play_logs.py:1324-1336 | PR-SHOUT FAILs on any ABORT-START. 0x522AF0 also returns 0 legitimately when the speaker is not in the area list (non-creature speaker) and no creature lies at x >= sx-r. Only creature-speaker ABORT-START (stale id in the binary search, 0x522AF0 `cVar1 != DAT_0099447f`) and ABORT-MID show the bug's reach. Needs the SF1 type field. |
| N1 | NIT | CompanionAI.cpp:1138 | FM2F throttle signature = `p.fl.n`, which changes on every FAIL leg, so "then on n change" is a no-op. Bounded by the budget and the 2-4 s leg time, so harmless; document it or use `n>=2` as the signature. |
| N2 | NIT | CompanionAI.cpp:1119-1122 | FM2C n2/n3 and FM2F/FM2R n use pairBump's gap window (reset after 30 s / 12 s of silence), not a sliding 30 s window as PROBE_DESIGN section 2 says. Fine for loop detection; fix the doc text. |
| N3 | NIT | CompanionAI.cpp:1233 vs 1241 | RS `a1xFollowup` runs after the `objPasses` filter. With `a1_party_only=0` or a tags/party_only mismatch, slot-6 runs of non-passing creatures never clear pendingA1, which gives false ORPHANs. Move the call above the filter (site<=3 only). |
| N4 | NIT | CompanionAI.cpp:609-617 | ORPHAN also fires when the companion dies, is dismissed, or a save is loaded while pendingA1 is set (g_tr persists across loads). Clear pendingA1 on a PF PLAYERLOAD/SELECT_DEL write, or note this in PR-A1X. |
| N5 | NIT | CompanionAI.cpp:551-557 | partyEmit calls 0x51C8D0/0x51C8F0 when serverApp != 0, but 0x51C620 dereferences `[server+4]+0x10064`. Add `rd<int>(server,4) != 0` (one line) before the two calls; the `pf` trigger can fire with no controlled creature (CHARGEN/PLAYERLOAD). |
| N6 | NIT | analyze_play_logs.py:1375-1395 | PR-PARTY judges `why=load` lines, and the first tick with a controlled creature can come before ADDMEMBER/PLAYERLOAD sets the members' flags, which gives a possible false FAIL; judge `load` only after 2 s. The comment "why=pf lines are mid-save" is wrong: save sites are transient and never set g_partyPf; pf lines come from ADDMEMBER/SWITCHPC/SELECT/CMD sites. |
| N7 | NIT | CompanionAI.cpp:509-523; analyze_play_logs.py:1304-1322 | PRS RAR prints only the top 8 types, so the type-0 bucket (the PR-RAR FAIL input) can be cut. Always append bucket 0 when non-zero. |
| N8 | NIT | CompanionAI.cpp:620-657 | probeSummary emits up to about 80 `eline` lines per summary, each with its own fflush. Write them with force but flush once at the end (e.g. a `pflushAfter` flag in vemit). |
| N9 | NIT | PROBE_DESIGN.md:285-295 | The protocol does not exercise PF's non-transient sites or PARTY `why=pf` (no party-selection swap), and has no explicit area transition. The perf-lab cost check that PROBE_BUILD says is required (P2/RAR +5 % rule, design line 38) is not a protocol step. The copy-out omits companion_ai.ini (the config is logged, but a copy helps). |

Not findings (checked): PR-FM3 FAIL is unreachable by construction (FM3L is printed only when `fm3Lost`): a self-consistency check, fine.
The SHX boundary case (an abort or x-break on the last element gives idx==count, counted as `end`) is semantically right: nobody
was skipped. The regexes match every printf literal (13 PR/PRS formats, the A1X suffix, the budget marker), and the
format-sync tests enforce this. The changed old tests are legitimate: hook count 18->31, "1/31", the A1 literal re-anchored with
`%s` (both `""` and the suffix tested; A1_RE has no `$`), and 16->17 PASS for PR-RAH2 on the healthy fixture.

## Exact fixes
**SF1 (CompanionAI.cpp:1335-1365)** After `int spk = ...`:
`int stype = rd<unsigned char>(spk, ObType, 0xFF); int party = stype == ObjTypeCreature ? rd<int>(spk, CrParty, 0) : 0;`
(replaces line 1344). Replace lines 1355-1357 with:
```cpp
bool isAbort = kind == cai::ShoutAbortStart || kind == cai::ShoutAbortMid;
static unsigned s_abortLines = 0; static cai::LineThrottle s_abortLog;
if (isAbort) { if (s_abortLines >= 100 && !s_abortLog.allow(now, static_cast<unsigned>(kind), 0, 1000)) return; ++s_abortLines; }
else { if (g_shxLines >= 30) return; ++g_shxLines; }
```
Then add ` type=%d` after `spk=%08x` in `fmt`, pass `stype`, and update `PR_SHX_RE` plus its format-sync test. Split
`g_shx.abortStart` into `abortStartCr` / `abortStartOther` (by stype==5) and print both in PRS SHX (regex + fixture).

**SF2 (CompanionAI.cpp:893)** Throttle STRICT per creature and keep the counts. Add `cai::LineThrottle strictLog; unsigned strictN = 0,
strictRej = 0;` to CrTrack, always count (`++t->strictN; if (!ok) ++t->strictRej;`), and log only if
`t->strictLog.allow(now, ok ? 1u : 0u, 10, 5000)` (first 10, then on an ok/would-reject change or every 5 s). Print `strict=%u/%u` in
the `A1 summary` line, and update its regex if the analyzer reads it. Give FOLLOWUP/ORPHAN their own ProbeCap (`PiA1xF`, name "A1XF",
`PiCount` 14), so STRICT can never use up their budget.

**SF3 (CompanionAI.cpp:1242-1250 + PRS)** (a) Count E9 deliveries before the `logged` test, with no budget. In the `site == 4` branch
compute `spk/pat/so` first, then `if (pat == 15 && !so.party && o.party) ++g_e9;`, and print `t=%lu PRS E9 deliveries=%u` from
probeSummary when it changes. (b) No faction read exists, so append the speaker tag ` spkTag=%s` (`so.tag`) to the S7 suffix; a tag
then tells allied from hostile. (c) Keep S7 lines for the E9 case: after the first 10 per creature, log S7 only when
`pat == 15 && !so.party`, with a session cap of 300. Analyzer: PASS/OBSERVE from `PRS E9` plus the listed S7 speaker tags. FAIL only
when SHX shows non-party r=35 creature speakers (SF1 `type=5`) and `PRS E9 deliveries=0`. Report the speaker tags so the user can
mark which are allies.

**SF4 (analyze_play_logs.py)** In the parse loop keep `budget_ln[probe] = ln` (first marker line) and `drop_spans` (t of each
`-- ai_trace: N lines dropped` marker, via DROP_RE). Map each check to the probes whose absent line it needs: PR-FM2 -> FM2C, FM2F;
PR-SS -> RS; PR-A1X -> A1X (A1XF after SF2); PR-E9 -> RS. Treat an event as judged only if its `ln` < that probe's budget line and
no drop marker falls within its window. Better for PR-SS: judge from the budget-free `PRS RS` s6c counters (an S6C run within 60 s =
the cr's s6c count rises in a PRS RS line with t in (t0, t0+60000+summary_sec*1000]), keeping the PR lines as evidence. Add fixtures:
budget marker before an SS1 gives OBSERVE; A1X marker before an end gives that end not judged.

**SF5 (analyze_play_logs.py:1324-1336)** After SF1: FAIL on `abortStartCr + abortMid > 0` (PRS SHX) or on ABORT lines with `type=5`
(ABORT-START) / any ABORT-MID. Report `abortStartOther` as INFO (a non-creature speaker outside the list is the vanilla no-listener case).

## Gate notes for the installer
- Re-run after the fixes: host test, `analyze_play_logs` unittests (new fixtures), `-fsyntax-only -Wall -Wextra`. The windows and toml are
  unchanged by every fix, so check_windows / verify_windows_both_exes need no re-run (the toml stays byte-identical).
- Before the play session: a perf-lab A/B with all probes on vs `probe_p2=0 probe_rar=0 probe_rs=0`, per the design's +5 % rule.
- Rollback as in PROBE_BUILD (CompanionAI-0.1.0-live.kpatch, db371b9f). The INI stays backward compatible.
