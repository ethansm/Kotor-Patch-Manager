# Companion AI logging audit (0.3.1-probe)

Source: `patch_manager_mods/companion_ai_src/CompanionAI.cpp` (1727 lines). Cites are `CompanionAI.cpp:N`, or `h:N` for `CompanionAiLogic.h`. Hook ids: header comment `CompanionAI.cpp:9-30` and `kotor2-steam-aspyr.hooks.toml`.

Helpers: `tline` :218 (trace channel, dedupe key + per-second cap), `eline` :220 (main channel, bypasses per-second cap), `ewarn` :222 (WARN), `pline` :487 (probe channel, probe budget via `pbudget` :482). All go through `vemit` :194 (`KP_ENABLED` at :195) -> `g_gate.admit` (h:442-496: dedupe, max_lines_per_sec, max_file_mb) -> KpLog. tline/pline/eline are KPLOG_INFO. Every handler calls `tick()` (:791) first, except where Safety item 1 says otherwise.

## Handler table

Logs decision: yes = each decision has a line; partial = some decisions or only a throttled subset; no = counters or state only. "-" in Level = no log line.

| # | Handler (line) | Hook | Logs decision | Helper | Bound | Silent drop paths | Level |
|---|---|---|---|---|---|---|---|
| 1 | AiTraceShipBuild (:890) | T1 0x692C50 | partial: flip counted only (S flips), one-time tid line :901 | eline :901 | S counter; first flip once | cfg off :893; vmCtx :894; ai-scope mismatch :895; objPasses :896 (no counter) | INFO |
| 2 | AiTracePrintString (:906) | T2 0x68F301 | partial: P line :918; engine-print suppression (return 1, :919) not marked | tline :918 | dedupe_ms 1000 + 200/s | not traced :912; objPasses :914 | INFO |
| 3 | AiTraceAurPostString (:1021) | T3 0x6F62F2 | yes (log-only event) | tline :1032 | dedupe + 200/s | log_aur=0 and filters :1024-1027 | INFO |
| 4 | AiTraceAttackEntry (:1041) | E1 0x6D740F | partial: SPIN/END only (:836, :846); RATE :850 (engine_verbose) | eline; tline | <=1 eval/s per track (:1051); eline bypasses 200/s | cfg or cr null :1043; non-party with no log_all/tags :1045 | INFO |
| 5 | AiTraceAttackApproachExit (:1055) | E1b 0x6D8BE2 | no: snapshot only, shows inside E1 SPIN exit{} (:836) | none | state write per call | cr null :1059; non-party :1061 | - |
| 6 | AiTraceFallbackFires (:1071) | E2 0x569A08 | yes per fire, throttled | tline :1088 | SubjectThrottle 20 then 1/10 s (:1087); e2 counted before filter (:1079) | !crPasses :1080; throttle :1087 | INFO |
| 7 | AiTracePerceptionMode1 (:1094) | E3 0x56BDBD | partial: per-creature detail (:1111); aggregate via reportP1 (:429, :432) | eline | p1_detail_lines + 64 slots (:1106); aggregate per summary_sec | duplicate creature (:1107-1108) | INFO |
| 8 | CompanionAiA1Gate (:1125) | A1 0x6D853E | yes: apply/skip + reason (:1194) | eline :1194 | a1Log LineThrottle 10, then sig change or 2 s (:1189); A1 summary :455 | non-candidate :1129 (a1NonCand); unread :1132 (a1Unread); controlled :1138 (a1Ctrl); log=0 or !crPasses: no line | INFO |
| 9 | CompanionAiP1Interval (:1204) | P1i 0x56BDB4 | no: counters p1Checks/Allowed/Throttled (S p1{}) | none | counters only | cr null :1208; p1=0 parity reset :1210-1212 (counter only) | - |
| 10 | CompanionAiP1Stamp (:1224) | P1s 0x56BDE3 | no: counter p1Runs, counted even when p1=0 (:1225) | none | counter | p1=0 :1226 | - |
| 11 | CompanionAiBumpClear (:1235) | FM1 0x586F43 | yes: keep/clear + why (:1278) | tline :1278; RETHINK tline :1271 | SubjectThrottle 20, then change or 2 s (:1276); RETHINK per-second cap | logOn=0 or !crPasses :1256 (fm1Keep/Clear still counted; fm1PartySkip :1257 non-party only) | INFO |
| 12 | CompanionAiFm4Snap (:1289) | FM4 snap 0x57451F | no: no line at snap; outcome logged at restore | none | counter fm4Snap (:1297) | fm4=0 and log=0 :1292; non-party :1293; FollowInfo null/unreadable :1295 (no commit, so restore is silent too) | - |
| 13 | CompanionAiFm4Restore (:1300) | FM4 restore 0x575183 | yes: restored / reset kept / no FollowInfo (:1315) | eline :1315 | unbounded per call (only after committed snap; bypasses 200/s) | take() false :1304 | INFO |
| 14 | CompanionAiA4 (:1323) | A4 0x6D9E2A | yes for guarded rows (:1350); a4=0 NULL row: ewarn :1329 | eline :1350; ewarn :1329 | SubjectThrottle 5, then 1/10 s (:1349); a4 counter counts all | readable row = vanilla by design | INFO (a4=1); WARN (a4=0, NULL) |
| 15 | CompanionAiCastGuard (:925) | G1 0x6D9ADB | drops only (:952); no counter | eline :952 | 200 lines per session (g_g1Logs :950), then silent, no marker, no S field | cfg off :929; untracked spell :932; reads :933-945 (return 0); non-party :938; no family match :945 | INFO |
| 16 | CompanionAiCastQueueNewest (:969) | F4 0x5917C1 | drops only (:1013); no counter | eline :1013 | 200 lines per session (g_f4Logs :1011), then silent, no marker, no S field | cfg off :972; owner/leader/party :974-980; list reads :987, :995 (return) | INFO |
| 17 | CompanionAiC1 (:1359) | C1 0x5868B9 / 0x5868F1 / 0x586929 | no: counters c1 restart/vanilla/capped (S c1{}) | none | RestartCap 256 per (obj,frame) in 50 ms (h:597-607; c1Capped :1369) | c1=0 gives vanilla | - |
| 18 | CompanionAiA3 (:1380) | A3 0x6D78B7 | partial: applied clears only (:1391); skips uncounted | eline :1391 | LineThrottle 20 per creature, then change or 5 s | a3=0 or dead flag :1383; !party or no id :1386; controlled/unknown :1388 | INFO |
| 19 | CompanionAiProbeFm2c (:1401) | FM2C 0x55B69A | yes per call (res 2/3), throttled | pline :1413 | probe_lines 400/probe (pbudget :482); pair LineThrottle 10, then change or 5 s (:1411) | probe off (handler top); objPasses :1407 | INFO |
| 20 | CompanionAiProbeFm2Fail (:1418) | FM2F 0x5C2438 | yes per fail leg, throttled | pline :1431 | probe_lines + g_psubj 20, then n change or 60 s (:1427) | probe off (handler top); !crPasses :1424 | INFO |
| 21 | CompanionAiProbeFm2Reapproach (:1437) | FM2R 0x5BC5F4 | yes: first 20 per pair, then every 5th n (:1455) | pline :1457 | probe_lines + n%5 sampling | probe off (handler top); !crPasses :1443 | INFO |
| 22 | CompanionAiProbeFm3Sentinel (:1462) | FM3S 0x51DBE1 | no: state only, consumed by FM3E/FM3R | none | none | probe off (handler top) | - |
| 23 | CompanionAiProbeFm3LevelEnd (:1470) | FM3E 0x51DD33 | partial: FM3L on sentinel loss (:1486); lost= counted in PRS FM3 (:714) | pline :1486 | probe_lines + 200-line silent cap (:1481), no marker | level mismatch :1475; not lost :1479 | INFO |
| 24 | CompanionAiProbeFm3Remove (:1491) | FM3R 0x51E0C9 | yes: FM3X first 50, then 1/s (:1510-1511) | pline :1511 | LineThrottle 50, then 1/s; probe_lines | probe off (handler top) | INFO |
| 25 | CompanionAiProbeRunScript (:1516) | RS 0x6FD8D0 | partial: per-site line (:1558); site counts in PRS RS (:719) | pline :1558 | per-site 20 per creature (:1551-1554); S7 50/session (:1552); E9 300/session (:1551); probe_lines | rsProbeIdle :1517; unprobed site: RSX counter only (:1520-1521); !objPasses :1531 | INFO |
| 26 | CompanionAiProbeP2 (:1563) | P2 0x56BE79 | partial: first 10 per creature (:1586) | pline :1588 | 10 per creature, silent, no marker; probe_lines | probe off; cr null :1566; !party :1568; p2Runs :1572; !crPasses :1575 | INFO |
| 27 | CompanionAiProbePartyFlag (:1592) | PF 0x58A35E | yes: non-transient always; transient only on real party-flag change (:1608) | pline :1610 | probe_lines + 200/s; PRS PF counters (:739) | transient no-change :1608 (still counted in pfc) | INFO |
| 28 | CompanionAiProbeRunActions (:1615) | RAR 0x5D6A60 | partial: REQ and MOVESTATE only (:1629-1635); histogram PRS RAR (:745) | pline :1631, :1635 | REQ 10 per creature; MOVESTATE 20 per creature; probe_lines | probe off; not creature :1618; !party with no log_all/tags :1620 | INFO |
| 29 | CompanionAiProbeShout (:1641) | SHX 0x5475E8 | yes for aborts (eline :1671); others partial | eline :1671; pline :1672 | aborts: first 100, then 1/s per kind (:1664); others: 30, then silent, no marker (:1665) | probe off (handler top) | INFO |
| 30 | CompanionAiProbeSetState (:1677) | SS1 0x59D966 | partial: state commit lines | pline :1685 | 50 per session, silent (:1682); no counter | !crPasses or cap (:1682) | INFO |
| 31 | CompanionAiProbeGetCurrentAction (:1691) | GCA 0x67B204 | yes on result change | pline :1707 | 40 per creature, silent (:1704); PRS GCA counts (:757) | !objPasses :1697 | INFO |

Not hooks: `KPatchInit` :1710 (KpLog_Init, startSession), `DllMain` (no logging). `pollModConfig` KP_WARN on schema parse failure is WARN.

## Gaps

1. G1 and F4 drops (:952, :1013) have no counter: `Stats` :365-370 and the S line :445-450 have no g1/f4 field. After 200 lines (:950, :1011) the fix keeps acting with no line and no marker.
2. `logConfig` (:224-241) omits `cast_family_guard` and `cast_queue_newest` (declared h:225-226, parsed h:284-285). A reload that changes only these keys triggers a config line (`sameConfig` h:313) that does not show the change.
3. Silent caps with no marker: PARTY 100 (:626), FM3L 200 (:1481), SS1 50 (:1682, no counter), RS S7 50 and E9 300 (:1551-1552), SHX non-abort 30 (:1665). Only `pbudget` prints `-- probe X line budget reached --` (:484).
4. Uncounted skips: A3 non-party or unknown/controlled target (:1386-1388); FM4 snapshot failure (:1295) and its restore (:1304) are both silent; G1/F4 doubtful-read returns (:932-945, :974-995) have no counter, so "no fix" and "unreadable" look the same.
5. C1 (:1359-1378) acts on every group delete with no line. Only S counters exist, and the RestartCap cutoff (h:597-607) is silent apart from c1Capped.
6. A1 reason label (:1194) prints "spin (not listed)" also when a1_party_only blocks (h:612-614 joins stance and party), so the log names the wrong cause.
7. T2 (:918-919): with engine_print=0 the engine PRINTSTRING is suppressed, but the P line has no suppressed flag.
8. eline bypasses the per-second cap for E1 SPIN/END (:836, :846), A1 (:1194), FM4 (:1315), A3 (:1391). Volume is bounded only by per-subject throttles and max_file_mb (default 64, h:211).
9. `pbudget` consumes the probe budget (:482-486) before `vemit` checks KP_ENABLED (:195). With the probe channel off in kplog.ini, the budget is spent with no output.

## Safety

1. `tick()` (:791) runs before any `beginScope()` in E1 (:1042), E1b (:1056), E3 (:1095), P1i (:1205) and RAR (:1621, logAll/party path). `tick` reaches `a1xClock` -> `actq::playerPaused` (`_shared_src/ActQueue.h:79-85`, uses `saferead::readAt`). That can read regions cached by the previous hook, which stay valid only until `beginScope` (`_shared_src/SafeRead.h:6-8`). It needs probe_a1x and a pending A1, sampled at most 4/s. Fix: move `beginScope()` above `tick()`.
2. Raw pointer reads with no SafeRead, in handlers the hot-list comment does not name (`CompanionAI.cpp:47-48` lists E1, E3, P1i/P1s only): P2 reads `cr` at :1567, :1569, :1570, :1576, and :1588 (inside the pline argument, evaluated even when the budget refuses); E1b :1060; RAR reads `obj` at :1618, :1619, :1623, :1625. Validity is assumed, not checked.
3. Raw writes outside `wr()`: P1i :1211-1212 (parity zeroing with p1=0) and P1s :1229-1230 write the creature's stamp fields. Hot-listed; safe only if UpdatePerception(1) just dereferenced `c` (E3 comment :1097).
4. A4, a4=0, NULL row: `fopen("companion-ai_crash.txt")` and `Sleep(100)` (:1332-1336) run before the deliberate fault (:1338). The side file bypasses KpLog and its caps. One-shot before the crash, so low.
5. E2 (:1074) and RS (:1526) call `a1xFollowup` -> `pev` before `tick()` (:1075, :1528). A pev line needs a track with pendingA1, which needs a prior tick, so this is practically safe.

Open: does the engine dereference the P2 creature (`cr`, :1565) and RAR object (`obj`, :1617) before these hooks fire? If not, the raw reads at :1567 and :1618 need SafeRead.
