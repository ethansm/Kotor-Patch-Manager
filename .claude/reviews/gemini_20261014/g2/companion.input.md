# G2 resolver input: batch companion (8 open fids, 8 topics)

## CO-T02 (P1) — G1 unit u001, facts file recon/CO-T02.md
Question: Can CompanionAiProbeFm3LevelEnd dereference g3.sentinel at +0x78 after the object was removed, because Fm3Remove sets g3.removed only when level == g3.level (read at removal time)?
G1 windows (do NOT reuse): patch_manager_mods/companion_ai_src/CompanionAI.cpp:1190-1245
G1 OPEN: Is rd<int> at 1201 fault-safe on freed memory? (rd and fm3Lost bodies are out of range)
- CO-2c-04 [PARTIAL] FM3 sentinel isSentinel requires level match; may leave removed false and read freed sentinel | file patch_manager_mods/companion_ai_src/CompanionAI.cpp | fn CompanionAiProbeFm3LevelEnd / CompanionAiProbeFm3Remove | cat memory-safety | G1 evidence patch_manager_mods/companion_ai_src/CompanionAI.cpp:1231-1232, :1219, :1201 | G1 note: Gap real; 1201 read is guarded, stale-not-crash; flag-side fix safe but moot for crash | anchor now patch_manager_mods/companion_ai_src/CompanionAI.cpp:1201 drift none

## CO-T03 (P2) — G1 unit u009, facts file recon/CO-T03.md
Question: Can e1Evaluate/E1 END state go stale (t.spinning stays true, or t.cr dangling) when a tracked creature dies or the area changes with no further AiTraceAttackEntry for its ObId, and is t.cr dereferenced in e1Evaluate?
G1 windows (do NOT reuse): patch_manager_mods/companion_ai_src/CompanionAI.cpp:324-335;patch_manager_mods/companion_ai_src/CompanionAI.cpp:688-725;patch_manager_mods/companion_ai_src/CompanionAI.cpp:780-792
G1 OPEN: Does track() (786) or any out-of-range sweep clear spinning or cr on death or area change?
- CO-2c-03 [PARTIAL] CrTrack caches raw creature pointer; stale spinning state and dangling cr after death or area change | file patch_manager_mods/companion_ai_src/CompanionAI.cpp | fn e1Evaluate / AiTraceAttackEntry | cat logic | G1 evidence patch_manager_mods/companion_ai_src/CompanionAI.cpp:690 anchor exact; :707-712 END; :787-790 cr set then e1Evaluate | G1 note: Spinning goes stale on death/area change; dangling cr not dereferenced in range | anchor now patch_manager_mods/companion_ai_src/CompanionAI.cpp:690 drift none

## CO-T05 (P2) — G1 unit u011, facts file recon/CO-T05.md
Question: Do the frame offsets read in AiTraceAurPostString (-0x20 ptr, -0x1C len, -0x10 x, -0x14 y, -0x18 life) match the stack layout of ExecuteCommandAurPostString at hook 0x6F62F2 per hooks.toml and the T2 PrintString handler?
G1 windows (do NOT reuse): patch_manager_mods/companion_ai_src/CompanionAI.cpp:755-775;patch_manager_mods/companion_ai_src/kotor2-steam-aspyr.hooks.toml
G1 OPEN: Do the 0x6F6200 decompile and T2 AiTracePrintString frame reads confirm -0x10 through -0x20?
- CO-2c-06 [PARTIAL] AurPostString hook frame offsets (-0x10..-0x20) unverified against 0x6F6200 decompilation | file patch_manager_mods/companion_ai_src/CompanionAI.cpp | fn AiTraceAurPostString | cat hook/ABI | G1 evidence patch_manager_mods/companion_ai_src/CompanionAI.cpp:759,772; patch_manager_mods/companion_ai_src/kotor2-steam-aspyr.hooks.toml:41-48 | G1 note: No in-range verification record; offset correctness needs 0x6F6200 decompile and T2 frame, both outside ranges. | anchor now patch_manager_mods/companion_ai_src/CompanionAI.cpp:772 drift none

## CO-T06 (P2) — G1 unit u012, facts file recon/CO-T06.md
Question: Does keyFor(0) always return slot0 (ignoring g_card0.tag) so that per-creature INI settings change owner when the party leader changes?
G1 windows (do NOT reuse): patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:280-300;patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:704-720;patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:851-905
G1 OPEN: Does party::members slot 0 hold a different creature after a leader swap (outside the read ranges)?
- CO-2d-03 [PARTIAL] keyFor(0) returns slot0 ignoring tag, so settings follow party position not creature after leader swap | file patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp | fn keyFor | cat logic | G1 evidence patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:707; :887 | G1 note: Early slot0 return is real (707). Creature-swap consequence unproven: needs party::members slot-0 behaviour. | anchor now patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:707 drift none

## CO-T07 (P2) — G1 unit u013, facts file recon/CO-T07.md
Question: Are g_pend0/g_pend[1..2], g_auto*, and g_lastUseMs keyed by party slot index only (no creature server pointer reconcile), so a leader swap during the 6 s pending window misattributes DONE/timeout/kick?
G1 windows (do NOT reuse): patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:385-400;patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:731-830
G1 OPEN: Does party::members() reassign slot 0 on a leader swap (code outside range)?
- CO-2d-04 [PARTIAL] Pending, auto and cooldown state indexed by party slot not creature; leader swap misattributes | file patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp | fn useNow / pendingTick / kickTick | cat logic | G1 evidence patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:394,757,762,790 | G1 note: Slot-keyed state confirmed; leader-swap effect needs party::members slot reassignment (outside range) | anchor now patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:394 drift none

## CO-T08 (P2) — G1 unit u014, facts file recon/CO-T08.md
Question: In uiFrame, is refreshCache (which passes g_card[i].seen / g_card0.seen to shouldAutoUse) called before the slot-mapping loop that sets seen for the current frame?
G1 windows (do NOT reuse): patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:851-905;patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:990-1030
G1 OPEN: Does heal::shouldAutoUse hard-gate on seen, and does hideAll() reset seen on gate close? Both outside range.
- CO-2d-05 [PARTIAL] refreshCache runs before g_card[i].seen is set for the frame, delaying or misgating auto-heal | file patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp | fn refreshCache / uiFrame | cat logic | G1 evidence patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:998 (refreshCache call) precedes :1001-1027 (seen reset and set); anchor exact at :872 | G1 note: Ordering is true; lag is one frame, not a cache-period delay. Simple reorder breaks c.have/c.m dependency. | anchor now patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:872 drift none

## CO-T10 (P2) — G1 unit u016, facts file recon/CO-T10.md
Question: When the RMB popover overlaps a card button, does the button keep its native tooltip/click-through state so a tooltip can show under the popover; and can g_nShown ever exceed 8 in uiFrame?
G1 windows (do NOT reuse): patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1060-1110;patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1186-1200;patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1225-1235
G1 OPEN: Is g_shown capacity at line 720 at least 7, and does popLayoutFor's left clamp plus engine z-order put the tooltip under the popover?
- CO-2d-02 [PARTIAL] Leader button native tooltip may show under overlapping RMB popover when clamped at screen left | file patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp | fn uiFrame / popLayoutFor / buildWidgets | cat logic | G1 evidence patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1091,1103-1105,1231 | G1 note: Leader tooltip, show and hover not gated by popover overlap; clamp and z-order unverified. | anchor now patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1056 drift none

## CO-T13 (P3) — G1 unit u056, facts file recon/CO-T13.md
Question: Does patch_manager_mods/_shared (as referenced by #include ../_shared/SafeRead.h) exist or get provided by the KPM build, and does test.toml (-I../_shared_src) fail to resolve that include?
G1 windows (do NOT reuse): patch_manager_mods/companion_ai_src/CompanionAI.cpp:48-58;patch_manager_mods/companion_ai_src/test.toml;patch_manager_mods/companion_ai_src/manifest.toml
G1 OPEN: Does the KPM build create patch_manager_mods/_shared (outside my ranges)?
- CO-2c-01 [PARTIAL] CompanionAI.cpp includes ../_shared/SafeRead.h but repo dir is _shared_src | file patch_manager_mods/companion_ai_src/CompanionAI.cpp | fn (file scope include) | cat tooling | G1 evidence patch_manager_mods/companion_ai_src/CompanionAI.cpp:53; patch_manager_mods/companion_ai_src/test.toml:6 | G1 note: Mismatch and unresolved include confirmed; whether KPM provides _shared unverified. | anchor now patch_manager_mods/companion_ai_src/CompanionAI.cpp:53 drift none
