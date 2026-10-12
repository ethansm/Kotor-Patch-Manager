# P-F: play 2026-10-12 engine-fix and probe logs (recon)

Session launch: pid 324, kplog session 2026-10-12 00:16:27. The check counts `launches=29` cover the whole shared `perflab_metrics.txt` history; row 29 (pid 324) is this session.

## F1 per-log verdicts (check output)

| log | route | verdict | launches | counts |
|---|---|---|---|---|
| k2-robustness-fixes_log.txt | analyze_runs (shared table) | PASS hangs=0 gateB=PASS, robust FIRED 0 | 29 (shared) | own log: 4 INFO, 0 WARN; COUNTERS all 0 |
| k2-activation-fix_log.txt | analyze_runs (shared table) | PASS, act IGN/PASS 1/0 | 29 (shared) | own log: 2 INFO, 0 WARN; 1 IGNORED (foreground ours) |
| texmem_log.txt | analyze_runs (shared table) | PASS, texmem DISABLED in launch 29 | 29 (shared) | own log: 18 INFO, 0 WARN, 11 kplog-malformed lines (not triaged) |
| k2-equip-icon-fix_log.txt | analyze_runs (shared table) | PASS | 29 (shared) | own log: 23 INFO, 0 WARN |
| k2-dialogue-letterbox-fix_log.txt | analyze_engfix_logs | PASS, 4 dialogues, 0 errors | 1 | 0 errs |
| perf-lab_log.txt | analyze_runs (shared table) | PASS hangs=0 gateB=PASS | 29 (shared) | see F4 |
| ranged-ai_log.txt | analyze_play_logs | 0 FAIL, 0 WARN, 1 INFO; 33 checklist items needs observation | 1 | see F3 |
| cmdq-probe_log.txt | cmdq_probe_log_check | 826 of 836 parsed, 0 malformed, C1 = 93 | n/a | see F3 |
| mouse-probe_log.txt | log_triage + kplog | ALERTS(1) | 1 | SYNTHDBL x1 (see F2) |

Caveat: `analyze_runs` ignores the log name. Five different names (k2-robustness, k2-activation, k2-equip-icon-fix, texmem, perf-lab) print the same shared table. Their PASS lines are not per-log evidence. The per-log lines above come from `kplog_parse.py --stats` and `--level WARN`.

## F2 mouse-probe SYNTHDBL (benign, not a lost click)

- Count: 1 SYNTHDBL in the session. 0 MISSEDDBL, 0 DBLGUARD, 0 "SYNTHDBL skipped".
- Sequence (ms since process start): `T=136644.616 CLICK msg=0x201 x=3586 y=334 dtLmbDown=345 dpos=4.0` then `T=136644.619 SYNTHDBL` then `T=136655.374 DBLACT isDbl=1`.
- The engine received the double. DBLACT isDbl=1 count is 16 = 15 OS-delivered 0x203 + 1 synthesized.
- Button balance: 0x201 count 194 + OS 0x203 count 15 = 209 = 0x202 count 209. No unpaired LMB down.
- Probe doc: `manifest.toml` says it "synthesizes dropped double-clicks under Wine/Wayland". `MouseFixLogic.h:83` says the same. `ClickLogic.h:11-12` sets MissedDblMs=500 and MissedDblPx=4.0; the comparison at `ClickLogic.h:45` is inclusive.
- Concern: dpos=4.0 sits exactly on the inclusive 4 px limit. The Windows default double-click rectangle is about 4 px wide (SM_CXDOUBLECLK; from memory, not verified in this repo). If so, the OS may also have rejected this pair, and "dropped" is unproven. Either way no click was lost.
- Wording defect: `MouseProbe.cpp:310` says "synthesized from dropped 0x201". The dropped message was the 0x203; 0x201 is only the trigger.

## F3 ranged-ai and cmdq-probe

- ranged-ai INFO (the only one): `[ai_trace.txt] not present`. The optional companion-AI trace is not in the game dir. Not a defect. Also 16 INFO kplog lines; 5 H1 lines truncated at the line-length cap.
- cmdq-probe: 836 lines, 826 parsed, 10 unknown (9 `#` lines plus the header), 0 malformed. ROUND ev=start = 167; ROUND ev=end = 197 (channel total 364). CLR = 195. Every start has a preceding CLR.
- CLR-to-start gap (nearest preceding CLR): min 0.040 ms (L177 after L174); max 100,185.6 ms (L48 after L35).
- Gaps <= 100 ms: 93, all C1 hits, max 99.3 ms. Gaps 100 ms to 100 s: 74.
- Gaps > 500 ms: 64 of 167 starts. Largest: 100,185.6 ms (L48<-L35), 91,377.3 ms (L68), 88,511.2 ms (L41). These fall outside the 100 ms C1 window, so they are not combat-start wipes. Flagged per request; not a defect signal on their own.

## F4 perf-lab

- Check counters: run B gate PASS (other_tpc_retained 0); hangs 0; KT still_in_demand 0; KT uaf_write 0; robust FIRED 0; ctrl_tpc_ret 1.
- The check output has no frame-time or stall column, so no counter can be shown over a gate.
- Ungated figure: first load 8991 ms for launch 29. The analyzer prints no gate for it. Earlier notes (`patch_manager_mods/10_engine_optimization_research.md`, Oct 1-2) record first loads of 3.7-5.0 s. Roughly 2x; worth a look, not a stall verdict.

## Verdict

PARTIAL. No lost click and no gate breach. Open items: the four engine-fix PASS lines are shared-table output; the 64 cmdq gaps over 500 ms need a read on whether they are expected; the SYNTHDBL boundary case is unverified.
