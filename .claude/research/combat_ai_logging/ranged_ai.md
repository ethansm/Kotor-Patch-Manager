# Ranged AI logging audit (recon, read-only)

Sources: `patch_manager_mods/ranged_ai_src/RangedAI.cpp` (0.4.0, 420 lines), `ranged_ai.ini`, `kotor2-steam-aspyr.hooks.toml`. Anchors are `cpp:line`.
KpLog.h is not in this worktree (no copy found by rg). KpLog core behaviour below comes from design docs 19d / 19f / 19g and is UNVERIFIED against the header.

## Hook handlers

| handler (anchor) | decides | logs decision | log call (cpp) | bound | silent early returns | level |
|---|---|---|---|---|---|---|
| RangedAiHold (cpp:209; hooks.toml:12) | end attack: shot blocked, in range, ranged, party, stance in hold_stances | partial | logf 243 inside throttle 237-241; summary() 249 | per creature: first 16, then sig change or >2 s; summary every 10 s | 212 null attacker; 217 non-party (log_all=0); unchanged holds within 2 s | INFO |
| RangedAiRetarget (cpp:255; hooks.toml:32) | retarget gate (act as IsPC) | yes | logf 266 inside 264 | UNBOUNDED per call | 258 null; 260 non-party | INFO |
| RangedAiStanceClamp (cpp:277; hooks.toml:51) | keep stance 16 in menu build | partial (kept event only) | logf 281 | 20 total (g_s1Logged), then SILENT | 279 non-16 (by design) | INFO |
| RangedAiStanceRow (cpp:287; hooks.toml:66) | writes action id 0x415 + icon into the aistate-16 row | yes on success; failures WARN | logf 300; logw 292, 294 | success: row-change dedupe, 3 first, cap 20 then SILENT; failures UNBOUNDED, ignore `log=` | 289 non-16; 298 after budget | INFO / WARN |
| RangedAiStanceClick (cpp:306; hooks.toml:82) | id 0x415 -> stance 16 + msg + tutorial row; else vanilla | yes | logf 312 (vanilla ids), logw 316 (null stats), logf 323 (switch) | UNBOUNDED per click (user rate) | 320-322 wr() results ignored | INFO / WARN |
| RangedAiHudStance (cpp:395; hooks.toml:97) | HUD marker 0x415 for stance 16, else -1 | yes | logf 399 | UNBOUNDED per HUD refresh | 398 wr() unchecked; non-16 (by design) | INFO |
| RangedAiStanceRules (cpp:362; hooks.toml:113) | eff stance (11 = no approach) for 16 when RaiMapsToRanged maps it | partial | logf 388 inside 386 | first 20 (g_h3Logged), then change-only and >=250 ms per creature | 378 non-16; no party/log_all gate; RaiMapsToRanged (341) never logs | INFO |
| KPatchInit (not a hook; cpp:403) | provider registration, KpLog_Init, load line | yes (load line) | emitLoadLine 94-99 | once | 404 provider skipped silently if api check fails | INFO |

## Gaps

1. RangedAiRetarget (cpp:264-268) writes one INFO line per call for every party creature; log=1 is the default (cpp:154). Only the global cap bounds it. Call rate not measured here (hook fires on the second blocked check, cpp:254).
2. MaxLogLines=6000 (cpp:81-84) counts every line type and has no WARN reserve. After the single cap marker (cpp:84), logw lines (cpp:292, 294, 316) vanish. KpLog's WARN exemption never sees them, because logv returns first.
3. S2 failure WARNs (cpp:292, 294) are unthrottled and ignore `log=` (logw, cpp:105, never checks g_cfg.log). The menu rebuilds "several times a second" (cpp:272), so a persistent bad frame reaches the cap in about 20 min at 5/s, after which H1/H2/H3 lines stop.
4. MenuLogBudget=20 (cpp:274) stops S1 (cpp:281) and S2 (cpp:298) with no marker. The counters never reset within the process.
5. Slot collisions defeat the throttles. trackFor (cpp:180-183) and the H3 slot reset (cpp:383) evict state on every call when two ids share a slot. The 8 slots are never freed, so with log_all=1 every call logs, and the summary counters reset (cpp:219).
6. loadIni compares hold, retarget, masks, msg, icon, ranged and approach (cpp:168-169) but not `log` or `log_all`. A runtime `log=0` or `log_all=1` writes no config line and no "off" marker.
7. Silent non-log failures: RaiMapsToRanged read failure returns 0 (cpp:348-352); rd() silently returns its default on a failed probe (cpp:110-114), so CrIsParty=0 reads as non-party; SafeRead's failure ring is never enabled or dumped here (SafeRead.h:35, 202); KPatchInit skips the provider silently (cpp:404-405); the 600-byte vsnprintf buffer truncates with no marker (cpp:85).
8. Stale docs and two switches: ranged_ai.ini:1 says 0.3.0 (code loads 0.4.0, cpp:414), and ranged_ai.ini:21 names ranged_ai_log.txt (core file is ranged-ai_log.txt, cpp:70-73). On a new core, INFO lines need kplog.ini `[ranged-ai]` enabled=1, because the effective level is WARN unless enabled (doc 19d). ranged_ai.ini `log=1` alone is not enough.

## Safety

1. cpp:379-380, RangedAiStanceRules log branch: raw derefs `*(const int*)(ebp+F_Attacker)` and `*(const int*)(attacker+CrId)`, guarded only by a null check. No probe, unlike every other read in the file. Runs for every stance-16 creature when log=1. Fix: `rd<int>(ebp, F_Attacker)` and `rd<int>(attacker, CrId)`. Slot validity at 0x006D850F is not verified here.
2. Pre-init logging: static init (cpp:78) and the first logf before KPatchInit (cpp:94-99, 101-103) go to the KpLog fallback by design (doc 19f:48). KpLog.h is absent, so pre-init behaviour of KP_LOGV and KpLog_SetFallbackFile is UNVERIFIED. DllMain writes no log line; it only formats (cpp:417, 137-140).
3. Unsynchronised globals: g_logLines (cpp:80-84), g_s1Logged / g_s2Logged (cpp:273), g_tr (cpp:179), g_h3 (cpp:333), g_cfg. Safe only if every hook runs on the game's main thread. The code neither states nor enforces this.
4. Format strings checked, no issue: all fmt strings are literals; %lu gets a DWORD tick; %.2f gets promoted floats; buffers are bounded (vsnprintf cpp:85, snprintf cpp:414-415). cpp:96 sets g_loadEmitted before writing, so a load line that hits a not-ready sink is never re-emitted.

## Debug / verbose level switch

None. The code uses only KPLOG_INFO (4 uses) and KPLOG_WARN (1 use); there is no DEBUG or VERBOSE level. Switches: ranged_ai.ini `log` (cpp:154, default 1) and `log_all` (cpp:155, default 0), plus kplog.ini `[ranged-ai]` (enabled, level, channels) for the core sink. With log=0, the load and config lines (cpp:417, 96-99) are still written.
