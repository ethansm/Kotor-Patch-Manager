# sh_hidden: why self-heal UX was not seen in 0.7.0-dev (recon, read-only)

## Log evidence (new vs .1)
- New log: BUILD n=20/20 and P1/P3 lines present. UI widgets were built (14 textures, 20 controls; was 16 before).
- New log: zero MAP lines, zero REFUSE lines, zero gate lines. Check verdict says PASS only because it tests build and P1/P3.
- .1 log: MAP member=1/2 lines present (refreshCache found both companion cards). These are missing from the new log.
- Interpretation: draw path never reaches refreshCache, so the gate fails silently every frame.
- Anomaly: new P1 +0x48=ff8fb44f (old run was 0x47). Panel flags look like garbage in this session.

## Code path
- uiFrame gate: CompanionSelfHeal.cpp:799 `if (!gateOpen(ctx)) { hideAll(); return; }`. This branch logs nothing.
- gateOpen = menuInfo && !inMenu && gameplayHudActive(g_hud) && foreground(): CompanionSelfHeal.cpp:271.
- gameplayHudActive (copy of hotbar gate): CompanionSelfHeal.cpp:223-240. Fails on any non-ignored modal, or any non-whitelisted panel above hud.
- MAP log (only after gate): CompanionSelfHeal.cpp:823. P1 log: CompanionSelfHeal.cpp:1090.
- Same gate logic as the action-hotbar, which also stayed hidden this session. Common cause is likely the shared gate.

## Git
- 4519b96 (Agy WIP, quota stop): "doc33 SH2 self-heal leader card". Commit has no test and was never finished. It is in the 0.7.0-dev build.
- 4519b96 changes leader paths: CompanionSelfHeal.cpp:354-356 (strip slot 0 registration), :363 (leader resolveState), :341 (popover reset).
- SelfHealUi.h gained leaderButtonRect (58x58 at card x,y) in the same commit. Nothing hides the button here.
- queue-hud (QueueHud.cpp:36, QueueHudLogic.h:20) uses shui::buttonRect/stripReserve only. It does not draw the self-heal card.

## INI
- companion_selfheal.ini has only [defaults]/[companions] keys: auto, hp, keep. No show/hide/enabled key exists in the code.
- Config is NOT the cause. Per-companion auto=1 rows are present.

## Most likely root cause (PARTIAL)
- gateOpen() returns false in this session, so hideAll() runs every frame. The shared gameplayHudActive check is the same one that hid the hotbar.
- The leader card (SH2) is in unfinished WIP code from 4519b96.

## Suggested fix
1. Add one edge-triggered log in uiFrame at line 799 that names the failing sub-check (menuInfo, inMenu, modal ptr, panel vtable, foreground). Log before changing logic, per feedback-log-thoroughly.
2. Log the vtable of the first non-whitelisted panel that makes gameplayHudActive false.
3. Compare the hotbar gate log in the same session. If both fail with the same panel, fix the shared gate in one place.

OPEN: Which sub-check of gameplayHudActive/foreground fails? Needs step 1 log. Not determinable from current logs.
