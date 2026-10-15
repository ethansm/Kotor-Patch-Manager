# CO-T02
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| CO-2c-04 | PARTIAL | patch_manager_mods/companion_ai_src/CompanionAI.cpp:1231-1232, :1219, :1201 | yes | Gap real; 1201 read is guarded, stale-not-crash; flag-side fix safe but moot for crash |
- CompanionAI.cpp:1197-1198: LevelEnd gate; returns unless g3.active and level (EBP-0x34) == g3.level.
- CompanionAI.cpp:1199: g3.active cleared before the sentinel read at :1201.
- CompanionAI.cpp:1201: nowLevel = rd<int>(g3.sentinel, 0x78, -1); the read runs before g3.removed is consulted.
- CompanionAI.cpp:1202: returns unless sentinel set and cai::fm3Lost(level, nowLevel, g3.removed) is true.
- CompanionAI.cpp:1218-1219: Fm3Remove reads level = rd<int>(obj, 0x78, -1) at removal time.
- CompanionAI.cpp:1231-1232: removed set only when g3.active && obj == g3.sentinel && level == g3.level. Gap confirmed.
- CompanionAI.cpp:1195, :1216, :1224: saferead::beginScope() and saferead::probeRead guard reads; rd<> body is outside range.
- CompanionAI.cpp:1209-1210: LevelEnd logs removed and nowLevel only; no control effect beyond counters and log.
- CompanionAI.cpp:1199 vs Fm3Remove order: if removal runs after LevelEnd cleared g3.active, isSentinel is false; ordering outside range.
- Gemini anchor at CompanionAI.cpp:1201 matches the source text exactly.
