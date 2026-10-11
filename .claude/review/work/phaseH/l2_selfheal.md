# Phase H L2: companion-selfheal leader button hidden (recon)

## 1. Log summary (companion-selfheal_log.txt, 41 parsed lines)

kotor2_logs.py check: `SELFHEAL P1=PASS P3=PASS DUMP=0 blocks USE=0 DONE=0 TIMEOUT=0 VERDICT=PASS`

- Widgets built: `BUILD n=20/20 why=` (20 widgets, 14 csh/kit textures created and queued).
- HUD at build: `P1 hud built panel=08a024f8 +0x30=084715e8 +0x48=ff8fb44f layoutOpen=1 ctlList=08aad288 ctlCount=217`
- Slots: 20x `P3 slot[0..2] ... hud=08a024f8` (the HUD pointer matches the built panel).
- Member 0 rect: NOT LOGGED. No `MAP member=0` line exists anywhere in the log.
- TOGGLE / AUTO / CLICK / USE / DONE: zero lines.
- Gate open marker: none. Gate closed marker: none.
- "ignored" or refusal lines: zero. `REFUSEF` is KP_LOGV (verbosity-gated), and it only fires on a use attempt, which never happened.

## 2. Gate code and the mask

- `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:60` `PanelIgnoredByInput = 0x600`
- `:224` `isIgnored(panel)` = `(flags & 0x600) != 0`
- `:230-249` `gameplayHudActive(hud)`: loops modals then panels. `:243` `if (isIgnored(p)) continue;` runs before `:244` `if (p == hud) return true;`. An ignored HUD is skipped, so it never reaches the return true, and the function returns false.
- Arithmetic: `0xFF8FB44F & 0x600 = 0x400` (nonzero). So `isIgnored(hud)` is true, and `gameplayHudActive` returns false every frame.
- `:271` `gateOpen` = menuInfo && !inMenu && gameplayHudActive && foreground.
- `:802` `if (!gateOpen(ctx)) { hideAll(); return; }` is silent. No log.
- `:329` the useNow REFUSE line is also silent unless a use is attempted.
- `:1098-1109` (P0/P1 at build): `P0 bisect` is KP_LOGV DEBUG, so it is absent. P1 is logged.
- `:489` `kLeaderWidgets = true`, so the leader widgets are built. The problem is visibility, not construction.
- `:826` the MAP log covers members 1-2 only. There is no MAP for member 0 (leader), which is the button that should be visible.

## 3. Missing log points (proposed, state-change only)

1. `CompanionSelfHeal.cpp:802`: on gateOpen transition, log `GATE open=%d inMenu=%d hudFlags=%08x hudIgnored=%d fg=%d`.
2. `:243-249`: on gameplayHudActive false, log the reason once per change (hud-ignored / modal-ignored / panel-not-whitelisted / none).
3. Leader MAP: after the member-1..2 block near `:826`, add a state-change `MAP member=0 rect=[x,y,w,h]` for the leader button.
4. `:1098-1109`: promote the P0 line from KP_LOGV DEBUG to KP_LOG INFO so it shows in normal logs.

## 4. Open question

Does the Action Hotbar (same gate copy, `kotor2-force-power-hotbar`) also fail on this save? If it does, the HUD's 0x400 bit is a real "ignored" flag in this build, and the gate copy is wrong for it. If it does not, the hotbar uses a different flag path.
