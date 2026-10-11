# r3 flicker recon: green queue strip (read-only)

(1) Draw path
- drawAll QueueHud.cpp:663-677: age=now-g_capTick (665); hud read (668); shouldShow(..., nSlots=1) (669) fails on combat/menu/age -> noteHidden + stripsHidden; else drawLeader (674), drawStrips (675).
- drawLeader 497-527: gather(s) (499) -> buildSlots (501) -> shouldShow(..., n) (502): n==0 -> noteHidden("empty queue"). All icons unresolved -> "nothing drawable" (527). No cache of the last good frame.
- drawStrips 584-660: per companion gatherFor (612) -> buildSlots (614); no shouldShow; n==0 -> sd.have=false -> stripNote hidden.
- gatherFor QueueHud.cpp:226-273: engine read (229), s.engineNe=s.ne=a.ne (235-236); buffered tail appended at 256-270 from g_itemQueue[memberK].queue[0..count) (nBuffered 268).
- buildSlots QueueHudLogic.h:90-119 (only types 0x0C/0x0F/0x2E/0x3F, isQualifying line 48); shouldShow :125-127 = enabled && (showInCombat||!combat) && !inMenu && age<250 && n>0.

(2) Active command
- NOT in any slot source. pop() (QueueHudLogic.h:562-568) removes it from queue[]; QueueHud.cpp:1208-1209 stores it in activeAction. gatherFor appends queue[] only.
- Engine lacks it + count==0 -> n==0 -> leader hidden ("empty queue", :502) and strips hidden. This is the full-disappear case.
- Engine lacks it + count>0 -> panel shows the next buffered item in slot 0; active icon missing (icon swap, not a hide).
- Log L74-93 (queue-hud_log.txt, read via 20-line window): engNe 1->0 at T=54570 with cmd=1 active, cmd=2 buffered; no hidden line; VANISH eaten_paused at T=54705 (~135 ms); re-inject at T=54705.5; engine holds it again at T=54719 (~14 ms). The 59831 episode matches.
- Eaten timing: debounce 150 ms (QueueHudLogic.h:982), spacing 250 ms (:989), EATEN_WAIT while foreign/!idle (:987, idle :1020). Range ~150 ms to ~1.6 s.
- Unpaused wipe: rule :984 returns AV_UNSEEN -> resolveActive drops the active command; no re-inject. Not a flicker but a lost command.

(3) Capture gate
- QueueHudCapture QueueHud.cpp:1371-1383 sets g_capTick (1379). Only caller: onHudFrame (1365-1367) on hud-host K2HUD_FRAME, which fires per PopulateMenus call (HudHost.h:13). No pause, targeting, or menu condition inside capture. H8 hook not installed (comment 1369-1370).
- A >=250 ms gap in PopulateMenus hides leader and strips ("stale capture"). Cadence not logged; UNSURE whether the click path skips it.
- Menu flag: EDGE menu toggles in log L255-L260 (~520 ms gap at T=148212-148731); menu and paused co-toggle at T=154379. Unproven that a quickbar click sets inMenu.

(4) Minimal change
- a. gatherFor (QueueHud.cpp:256, before the buffered loop): if ib.active and the engine does not hold it (engineHoldsAction, :855 - forward-declare), append activeAction first: Spell -> T_CAST_SPELL/spellId, Item -> T_USE_ITEM/clientItem. Do not count in nBuffered. Sweep (:1042-1050) and handoff (:894) read engineNe only, so they are unaffected.
- b. drawLeader (:497-502): cache last Draw[]/nd/tick on success; if n==0 && !inMenu && (showInCombat || !combat) && now-tick < 400 ms, redraw cache. Same per k in drawStrips. Never hold during inMenu.

Tests pinning this
- shouldShow: testWaitingAndShow (queue_hud_logic_test.cpp:276-299; 249/250 boundary, nSlots 0 at :292).
- cancelVisible: testCancelVisible (:1084-1092).
- buildSlots: testEmptyAndNulls (:63), testFirstEntry (:75), testLaterEntries (:114), testExpansion (:146), testCaps (:228).
- Change (a) belongs in a pure helper in QueueHudLogic.h so a new test can pin it; gatherFor is not unit-testable. Change (b) leaves shouldShow untouched.

OPEN: does the quickbar click run PopulateMenus each frame (capture gap)? does it set inMenu? Add one trace of age/inMenu at noteHidden to settle.
