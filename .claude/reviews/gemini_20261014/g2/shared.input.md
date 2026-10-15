# G2 resolver input: batch shared (15 open fids, 13 topics)

## SH-T07 (P2) — G1 unit u050, facts file recon/SH-T07.md
Question: In guikitWheelCb with iniW/iniH set, are ev->screen_w/screen_h passed as the cw/ch (client-size) arguments of clientToEngineF with no >0 guard, rather than the OS client rect size?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/GuiKit.h:578-600;patch_manager_mods/_shared_src/GuiKit.h:20-50
G1 OPEN: Does K2WheelEvent.screen_w hold the OS client rect size? Needs the K2WheelEvent definition and the syncScreen body (outside read ranges).
- SH-3a-01 [PARTIAL] Wheel callback passes engine screen size as client rect size to clientToEngineF | file patch_manager_mods/_shared_src/GuiKit.h | fn guikit::detail::guikitWheelCb | cat logic | G1 evidence patch_manager_mods/_shared_src/GuiKit.h:583-588 | G1 note: Guard at :586 covers iniW/iniH only; 'engine size' label unproven, ev->screen_w is pre-sync raw. | anchor now patch_manager_mods/_shared_src/GuiKit.h:587 drift none

## SH-T09 (P2) — G1 unit u051, facts file recon/SH-T09.md
Question: Does EffectIcons.h read Effect_Type/IntCount/Ints/SpellId from compile-time constants (not k2addr::off) and write seen[nSeen++] without an nSeen < MaxEffects bound, and is the scanShieldPools zero-offset check in checkEffects vacuous because cnt already equals 0?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/EffectIcons.h;patch_manager_mods/_shared_src/Effects.h:105-135;patch_manager_mods/_shared_src/k2addr_test.cpp:425-450
G1 OPEN: Does effectListNonEmpty/readEffectList clamp count to MaxEffects, and is collectBuffs tested elsewhere in k2addr_test.cpp?
- SH-3a-10 [PARTIAL] EffectIcons bypasses K2AddrTable offsets; seen[] write lacks MaxEffects bound | file patch_manager_mods/_shared_src/EffectIcons.h | fn k2fx::collectBuffs / k2fx::iconForEffectIconEffect | cat logic | G1 evidence patch_manager_mods/_shared_src/EffectIcons.h:31,32,43,47,60 | G1 note: Bare Effect_* names and unguarded seen[nSeen++] are real; overflow needs count>MaxEffects (:42, outside range). | anchor now patch_manager_mods/_shared_src/EffectIcons.h:47 drift none
- SH-3b-03 [PARTIAL] checkEffects scanShieldPools zero-offset assertion is vacuous; collectBuffs untested | file patch_manager_mods/_shared_src/k2addr_test.cpp | fn checkEffects | cat test-gap | G1 evidence patch_manager_mods/_shared_src/k2addr_test.cpp:434,439,443,445; patch_manager_mods/_shared_src/Effects.h:116-119 | G1 note: :445 never reaches :119 guard (only Creature_EffectCount zeroed); cnt=0 true but not the cause. collectBuffs untested: unverified. | anchor now patch_manager_mods/_shared_src/k2addr_test.cpp:445 drift none

## SH-T10 (P2) — G1 unit u052, facts file recon/SH-T10.md
Question: Does party::vitals use hardcoded SlotMaxHp=0x98/SlotCurHp=0x9C instead of K2AddrTable Creature_VtMaxHp/CurHp, and does party_test.cpp substitute g_callVital everywhere so the real callVitalSlot (short return) is never run?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/Party.h:90-125;patch_manager_mods/_shared_src/party_test.cpp:90-110;patch_manager_mods/_shared_src/party_test.cpp:190-200
G1 OPEN: Does K2AddrTable hold Creature_VtMaxHp/CurHp, and does any party_test block outside lines 190-200 run real callVitalSlot?
- SH-3a-11 [PARTIAL] party::vitals hardcodes HP vtable slots instead of K2AddrTable Creature_VtMaxHp/CurHp | file patch_manager_mods/_shared_src/Party.h | fn party::table / partyCount / vitals | cat logic | G1 evidence patch_manager_mods/_shared_src/Party.h:99-100,113-114 | G1 note: Slots hardcoded and documented; K2AddrTable vital entries not in range | anchor now patch_manager_mods/_shared_src/Party.h:113 drift none
- SH-3c-08 [PARTIAL] party_test never runs real callVitalSlot (signed short return); K2Addr vital offsets untested | file patch_manager_mods/_shared_src/party_test.cpp | fn main | cat test-gap | G1 evidence patch_manager_mods/_shared_src/party_test.cpp:198,95-100 | G1 note: Block swaps in fakeVital (int, no short); real callVitalSlot skipped there; \"everywhere\" unverified | anchor now patch_manager_mods/_shared_src/party_test.cpp:95 drift none

## SH-T11 (P3) — G1 unit u090, facts file recon/SH-T11.md
Question: Are appendToPanel, setRect, setFill, setAlpha, setText, setFont, setAlign, getTextColor, setTextColor never called on a success path in guikit_test.cpp?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/guikit_test.cpp:1-80;patch_manager_mods/_shared_src/GuiKit.h:205-285
G1 OPEN: Does guikit_test.cpp lines 81+ call any of the nine functions on a success path (outside brief range)?
- SH-3b-01 [PARTIAL] guikit_test never exercises many GuiKit setters/getters or playHoverSound success paths | file patch_manager_mods/_shared_src/guikit_test.cpp | fn main | cat test-gap | G1 evidence guikit_test.cpp:24-80; GuiKit.h:215,247,256,263,269,276 | G1 note: 1-80 call none of the nine; 81+ unread; setFill/setAlpha/appendToPanel/playHoverSound defs outside range | anchor now patch_manager_mods/_shared_src/guikit_test.cpp:1 drift none

## SH-T12 (P3) — G1 unit u091, facts file recon/SH-T12.md
Question: Do checkSteamFields/checkGogFields omit Border_FillStyle (kSteam 0x34) and the SceneView.h fields, and are STEAM(Menu_Char2/Menu_Char3) checked twice in checkSteamFields?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/k2addr_test.cpp:25-60;patch_manager_mods/_shared_src/k2addr_test.cpp:150-230;patch_manager_mods/_shared_src/K2Addr.h:100-110
G1 OPEN: Do Steam lines 61-149 of checkSteamFields hold Border_FillStyle, SceneView, or the Menu_Char2/3 checks?
- SH-3b-02 [UNSURE] k2addr_test omits Border_FillStyle and SceneView fields; Menu_Char2/3 checked twice | file patch_manager_mods/_shared_src/k2addr_test.cpp | fn checkSteamFields / checkGogFields | cat test-gap | G1 evidence patch_manager_mods/_shared_src/k2addr_test.cpp:25-60; k2addr_test.cpp:150-170; k2addr_test.cpp:189-190; K2Addr.h:106 | G1 note: Steam read ranges lack Border_FillStyle, SceneView, Menu_Char2/3; 61-149 unread. GOG SceneView omission is intentional. | anchor now patch_manager_mods/_shared_src/k2addr_test.cpp:25 drift none

## SH-T13 (P3) — G1 unit u092, facts file recon/SH-T13.md
Question: Does effects_test.cpp call iconFx only with rows 28 and 5 and never with an ii_/iw_ resref row (item==true) or a null-resref row?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/effects_test.cpp:335-375;patch_manager_mods/_shared_src/EffectIcons.h:20-45;patch_manager_mods/_shared_src/EffectIconRowTable.inc
G1 OPEN: Do effects_test.cpp lines outside 335-375 route ii_/iw_ rows through iconForEffectIconEffect? Not read.
- SH-3b-04 [PARTIAL] effects_test never covers ii_/iw_ item icons or null-resref rows | file patch_manager_mods/_shared_src/effects_test.cpp | fn main (iconForEffectIconEffect item prefix) | cat test-gap | G1 evidence patch_manager_mods/_shared_src/effects_test.cpp:353,362-364,369; patch_manager_mods/_shared_src/EffectIcons.h:33,35 | G1 note: Half-true: iconFx also gets rows 2, 0, -3, 5000, 120; no ii_/iw_ or nullptr harmful row tested via iconFx. | anchor now patch_manager_mods/_shared_src/effects_test.cpp:353 drift none

## SH-T15 (P3) — G1 unit u094, facts file recon/SH-T15.md
Question: Does sceneview_test.cpp's installFakes overwrite every default call lambda (callSceneInit/SetPos/SetOri/GetVec/GetParent/CamAttach/CamFov) before use, with no test of the defaults?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/SceneView.h:40-80;patch_manager_mods/_shared_src/sceneview_test.cpp:60-75;patch_manager_mods/_shared_src/sceneview_test.cpp:100-120
G1 OPEN: Do lines outside the read ranges (sceneview_test.cpp:26,30,31 and others) exercise the default call lambdas?
- SH-3b-06 [PARTIAL] sceneview_test replaces all default call lambdas so defaults are never tested | file patch_manager_mods/_shared_src/sceneview_test.cpp | fn main (default vtable-slot lambdas) | cat test-gap | G1 evidence patch_manager_mods/_shared_src/sceneview_test.cpp:67-68; patch_manager_mods/_shared_src/SceneView.h:47-72 | G1 note: Overwrite of all 7 call lambdas true; \"defaults never tested\" true only within read ranges | anchor now patch_manager_mods/_shared_src/sceneview_test.cpp:26,patch_manager_mods/_shared_src/sceneview_test.cpp:30,patch_manager_mods/_shared_src/sceneview_test.cpp:31 drift none

## SH-T16 (P3) — G1 unit u095, facts file recon/SH-T16.md
Question: Does gui2d_test.cpp set clipNoFill, flushArgAddr, clipDepthAddr and all function pointers up front so that pushClip's null-colour return and the K2AddrTable-backed defaults are never exercised?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/Gui2D.h:75-115;patch_manager_mods/_shared_src/gui2d_test.cpp:35-70
G1 OPEN: Do test lines 71+ or installFakes (above line 35) exercise the null-colour return or table defaults, and set all hooks?
- SH-3c-02 [PARTIAL] gui2d_test overrides all engine hooks; K2AddrTable-backed default paths untested | file patch_manager_mods/_shared_src/gui2d_test.cpp | fn main / installFakes | cat test-gap | G1 evidence patch_manager_mods/_shared_src/gui2d_test.cpp:47-48; patch_manager_mods/_shared_src/Gui2D.h:83,86,95,107 | G1 note: Up-front overrides and unreached defaults hold in range; hook setup and rest of file unread. Removing overrides reads engine addrs on host; unsafe. | anchor now patch_manager_mods/_shared_src/gui2d_test.cpp:48 drift none

## SH-T17 (P3) — G1 unit u096, facts file recon/SH-T17.md
Question: Is actq::playerPaused untested in actqueue_test.cpp (and not called by QueueHud's tests), and does the test omit asserting s.rn[0].spellId and the kMaxEntries/kMaxRound truncation?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/ActQueue.h:60-130;patch_manager_mods/_shared_src/actqueue_test.cpp:60-105
G1 OPEN: Do lines 1-59 of actqueue_test.cpp or QueueHud's tests assert rn spellId or call actq::playerPaused?
- SH-3c-04 [PARTIAL] actq::playerPaused untested anywhere; truncation bounds and rn spellId unasserted | file patch_manager_mods/_shared_src/actqueue_test.cpp | fn main | cat test-gap | G1 evidence patch_manager_mods/_shared_src/actqueue_test.cpp:90,101; patch_manager_mods/_shared_src/ActQueue.h:98,120 | G1 note: Gaps real in-range (rn spellId, truncation); playerPaused needs fixed-address stub; QueueHud unverified | anchor now patch_manager_mods/_shared_src/actqueue_test.cpp:101 drift none

## SH-T18 (P3) — G1 unit u097, facts file recon/SH-T18.md
Question: Does filewatch_test.cpp never pass a non-null Stamp* out to filewatch::changed, and is the mtimeMs assertion in test_stamp guarded by a nanosecond-support check?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/FileWatch.h:70-100;patch_manager_mods/_shared_src/filewatch_test.cpp:29-100
G1 OPEN: Does filewatch_test.cpp lines 101 onward (rest of test_poll and later tests) pass a Stamp* out to changed()?
- SH-3c-05 [PARTIAL] filewatch_test never passes Stamp out param; mtimeMs check skipped on coarse FS | file patch_manager_mods/_shared_src/filewatch_test.cpp | fn test_poll / test_stamp | cat test-gap | G1 evidence patch_manager_mods/_shared_src/filewatch_test.cpp:51-53,59; patch_manager_mods/_shared_src/FileWatch.h:81 | G1 note: Out-param gap true for lines 29-100 only; coarse-FS skip is deliberate, not a defect | anchor now patch_manager_mods/_shared_src/filewatch_test.cpp:78 drift none

## SH-T19 (P3) — G1 unit u098, facts file recon/SH-T19.md
Question: Does guitip_test.cpp lack cases for readMgr with an unreadable mgr pointer or partially readable mgr, and for measure() with a null lines or maxLen pointer?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/GuiTip.h:40-100;patch_manager_mods/_shared_src/guitip_test.cpp
G1 OPEN: Can a partial field-read failure be simulated? Depends on g_stubVirtualQueryAll (guitip_test.cpp:24), outside read ranges.
- SH-3c-06 [PARTIAL] guitip_test lacks unreadable-manager, partial-readability and null out-param measure cases | file patch_manager_mods/_shared_src/guitip_test.cpp | fn main | cat test-gap | G1 evidence patch_manager_mods/_shared_src/guitip_test.cpp:53,66,101-105 | G1 note: Missing: null out-params for measure, non-null unreadable mgr, partial field reads; null mgr and unreadable cell covered. | anchor now patch_manager_mods/_shared_src/guitip_test.cpp:28 drift none

## SH-T20 (P3) — G1 unit u099, facts file recon/SH-T20.md
Question: Does item_read_test.cpp lack cases for an unreadable entry array, count -1 or 257, and a full 16-byte non-NUL-terminated icon resref?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/ItemRead.h:20-60;patch_manager_mods/_shared_src/item_read_test.cpp:40-80
G1 OPEN: Does MaxEntries equal 256 (so 257 is the boundary), and do item_read_test.cpp lines outside 40-80 cover these cases?
- SH-3c-07 [PARTIAL] item_read_test misses unreadable entries, count bounds and 16-byte icon resref | file patch_manager_mods/_shared_src/item_read_test.cpp | fn main | cat test-gap | G1 evidence patch_manager_mods/_shared_src/item_read_test.cpp:51,63,65; patch_manager_mods/_shared_src/ItemRead.h:33,36,40-44 | G1 note: Anchor is in ItemRead.h:36, not the test. Only count 100000 tested; unreadable-entry, count -1/257, 16-byte icon untested in range. | anchor now - drift noanchor

## SH-T21 (P3) — G1 unit u100, facts file recon/SH-T21.md
Question: In textfit_test.cpp, does the F10 marqueeText call use off=7 (not a space in the string) and are the fitText trailing-space strip, negative off and over-120-char wrapAppendW word cases absent?
G1 windows (do NOT reuse): patch_manager_mods/_shared_src/TextFit.h:20-80;patch_manager_mods/_shared_src/textfit_test.cpp:30-70
G1 OPEN: Do textfit_test.cpp lines 1-29 or 71+ cover off-on-space, negative off, or >120-char words?
- SH-3c-09 [PARTIAL] textfit_test misses trailing-space trim, off-on-space, negative off, over-120-char word | file patch_manager_mods/_shared_src/textfit_test.cpp | fn main | cat test-gap | G1 evidence patch_manager_mods/_shared_src/textfit_test.cpp:52; patch_manager_mods/_shared_src/TextFit.h:31,42,43,61 | G1 note: Anchor exact. Off=7 is 'e', not a space. Off-on-space, negative off, >120 word absent in 30-70. Trailing-space trim not targeted. | anchor now patch_manager_mods/_shared_src/textfit_test.cpp:52 drift none
