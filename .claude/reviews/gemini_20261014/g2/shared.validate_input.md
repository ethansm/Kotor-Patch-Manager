# G2.3 skeptic input: shared (18 fids)

## SH-T03 (P1): Does codepatch::writeChecked memcpy n bytes from the target address before any committed/readable check or protect call, and is n <= 0 not guarded in write/writeChecked?
- **SH-3a-04** v2=CONFIRMED (G1 CONFIRMED) | claimed high memory-safety | writeChecked memcpy reads target before protect/commit check; n<=0 unguarded | file patch_manager_mods/_shared_src/CodePatch.h fn codepatch::writeChecked
  - G1 evidence: patch_manager_mods/_shared_src/CodePatch.h:91 (read), :92 (compare), :94 (protect), :70-74 (committed, unused here) | G1 note: Read precedes committed()/protect; no n<=0 guard in write or writeChecked. Gap real; caller reach unverified.
  - v2 evidence: patch_manager_mods/_shared_src/CodePatch.h:91 (read), :92 (compare), :94 (protect), :70-74 (committed, unused here) | v2 notes: 

## SH-T04 (P2): Does guikit::imageFor store a null loadImage result in its static pool and return it on later calls, so hudkit::image retrying after ensure() never calls loadImage again, and does hudkit_test.cpp neither call hudkit::ensure() nor cover this?
- **SH-3a-05** v2=CONFIRMED (G1 CONFIRMED) | claimed high logic | imageFor pools failed (null) lookups so hudkit::image retry never reloads | file patch_manager_mods/_shared_src/HudKit.h fn hudkit::image
  - G1 evidence: patch_manager_mods/_shared_src/GuiKit.h:296,298-300; patch_manager_mods/_shared_src/HudKit.h:45 | G1 note: Null loadImage result is pooled; retry hits pooled null without reload. Un-pooling re-runs loadImage per call.
  - v2 evidence: patch_manager_mods/_shared_src/GuiKit.h:296,298-300; patch_manager_mods/_shared_src/HudKit.h:45 | v2 notes: 
- **SH-3c-01** v2=CONFIRMED (G1 CONFIRMED) | claimed medium test-gap | hudkit_test never calls hudkit::ensure and misses negative-caching in imageFor | file patch_manager_mods/_shared_src/hudkit_test.cpp fn main
  - G1 evidence: patch_manager_mods/_shared_src/hudkit_test.cpp:60,67,68 | G1 note: No ensure() call and no repeat-null assertion; line 60 is the no-pool path.
  - v2 evidence: patch_manager_mods/_shared_src/hudkit_test.cpp:60,67,68 | v2 notes: 

## SH-T07 (P2): In guikitWheelCb with iniW/iniH set, are ev->screen_w/screen_h passed as the cw/ch (client-size) arguments of clientToEngineF with no >0 guard, rather than the OS client rect size?
- **SH-3a-01** v2=PARTIAL (G1 PARTIAL) | claimed high logic | Wheel callback passes engine screen size as client rect size to clientToEngineF | file patch_manager_mods/_shared_src/GuiKit.h fn guikit::detail::guikitWheelCb
  - G1 evidence: patch_manager_mods/_shared_src/GuiKit.h:583-588 | G1 note: Guard at :586 covers iniW/iniH only; 'engine size' label unproven, ev->screen_w is pre-sync raw.
  - v2 evidence: patch_manager_mods/gui_input_src/GuiInputProvider.cpp:84-93,101-105; patch_manager_mods/_shared_src/GuiKit.h:442 | v2 notes: [gem2 PARTIAL] u039: Provider screen_w is engine size when readable, not client size; cw unguarded in helper; call-site args out of range.

## SH-T09 (P2): Does EffectIcons.h read Effect_Type/IntCount/Ints/SpellId from compile-time constants (not k2addr::off) and write seen[nSeen++] without an nSeen < MaxEffects bound, and is the scanShieldPools zero-offset check in checkEffects vacuous because cnt already equals 0?
- **SH-3a-10** v2=CONFLICT (G1 PARTIAL) | claimed medium logic | EffectIcons bypasses K2AddrTable offsets; seen[] write lacks MaxEffects bound | file patch_manager_mods/_shared_src/EffectIcons.h fn k2fx::collectBuffs / k2fx::iconForEffectIconEffect
  - G1 evidence: patch_manager_mods/_shared_src/EffectIcons.h:31,32,43,47,60 | G1 note: Bare Effect_* names and unguarded seen[nSeen++] are real; overflow needs count>MaxEffects (:42, outside range).
  - v2 evidence: patch_manager_mods/_shared_src/EffectIcons.h:31 ; patch_manager_mods/_shared_src/Effects.h:64 | v2 notes: [det CONFIRMED] d048: Bare Effect_* names in EffectIcons.h; no k2addr::off there, unlike Effects.h:116. || [det REFUTED] d049: effectListNonEmpty clamps count<=MaxEffects; EffectIcons.h:42 returns early on failure.

## SH-T10 (P2): Does party::vitals use hardcoded SlotMaxHp=0x98/SlotCurHp=0x9C instead of K2AddrTable Creature_VtMaxHp/CurHp, and does party_test.cpp substitute g_callVital everywhere so the real callVitalSlot (short return) is never run?
- **SH-3a-11** v2=PARTIAL (G1 PARTIAL) | claimed medium logic | party::vitals hardcodes HP vtable slots instead of K2AddrTable Creature_VtMaxHp/CurHp | file patch_manager_mods/_shared_src/Party.h fn party::table / partyCount / vitals
  - G1 evidence: patch_manager_mods/_shared_src/Party.h:99-100,113-114 | G1 note: Slots hardcoded and documented; K2AddrTable vital entries not in range
  - v2 evidence: patch_manager_mods/shield_bar_src/ShieldBar.cpp:382 | v2 notes: [det PARTIAL] d051: Party.h hardcodes 0x98/0x9C, but Creature_VtMaxHp has non-test consumers (ShieldBar.cpp:382).
- **SH-3c-08** v2=CONFIRMED (G1 PARTIAL) | claimed medium test-gap | party_test never runs real callVitalSlot (signed short return); K2Addr vital offsets untested | file patch_manager_mods/_shared_src/party_test.cpp fn main
  - G1 evidence: patch_manager_mods/_shared_src/party_test.cpp:198,95-100 | G1 note: Block swaps in fakeVital (int, no short); real callVitalSlot skipped there; \\\"everywhere\\\" unverified
  - v2 evidence: patch_manager_mods/_shared_src/party_test.cpp:198 | v2 notes: [det CONFIRMED] d052: callVitalSlot never referenced; all party::vitals calls (214-313) sit inside fake swap 198-335.

## SH-T11 (P3): Are appendToPanel, setRect, setFill, setAlpha, setText, setFont, setAlign, getTextColor, setTextColor never called on a success path in guikit_test.cpp?
- **SH-3b-01** v2=CONFIRMED (G1 PARTIAL) | claimed medium test-gap | guikit_test never exercises many GuiKit setters/getters or playHoverSound success paths | file patch_manager_mods/_shared_src/guikit_test.cpp fn main
  - G1 evidence: guikit_test.cpp:24-80; GuiKit.h:215,247,256,263,269,276 | G1 note: 1-80 call none of the nine; 81+ unread; setFill/setAlpha/appendToPanel/playHoverSound defs outside range
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d053: None of the nine guikit free functions is called in guikit_test.cpp.

## SH-T12 (P3): Do checkSteamFields/checkGogFields omit Border_FillStyle (kSteam 0x34) and the SceneView.h fields, and are STEAM(Menu_Char2/Menu_Char3) checked twice in checkSteamFields?
- **SH-3b-02** v2=CONFIRMED (G1 UNSURE) | claimed medium test-gap | k2addr_test omits Border_FillStyle and SceneView fields; Menu_Char2/3 checked twice | file patch_manager_mods/_shared_src/k2addr_test.cpp fn checkSteamFields / checkGogFields
  - G1 evidence: patch_manager_mods/_shared_src/k2addr_test.cpp:25-60; k2addr_test.cpp:150-170; k2addr_test.cpp:189-190; K2Addr.h:106 | G1 note: Steam read ranges lack Border_FillStyle, SceneView, Menu_Char2/3; 61-149 unread. GOG SceneView omission is intentional.
  - v2 evidence: no-hits ; patch_manager_mods/_shared_src/k2addr_test.cpp:122 | v2 notes: [det CONFIRMED] d054: Border_FillStyle and SceneView never referenced in k2addr_test.cpp. || [det CONFIRMED] d055: Menu_Char2 and Menu_Char3 each STEAM-checked twice (lines 122-126).

## SH-T13 (P3): Does effects_test.cpp call iconFx only with rows 28 and 5 and never with an ii_/iw_ resref row (item==true) or a null-resref row?
- **SH-3b-04** v2=PARTIAL (G1 PARTIAL) | claimed medium test-gap | effects_test never covers ii_/iw_ item icons or null-resref rows | file patch_manager_mods/_shared_src/effects_test.cpp fn main (iconForEffectIconEffect item prefix)
  - G1 evidence: patch_manager_mods/_shared_src/effects_test.cpp:353,362-364,369; patch_manager_mods/_shared_src/EffectIcons.h:33,35 | G1 note: Half-true: iconFx also gets rows 2, 0, -3, 5000, 120; no ii_/iw_ or nullptr harmful row tested via iconFx.
  - v2 evidence: patch_manager_mods/_shared_src/effects_test.cpp:363 | v2 notes: [det PARTIAL] d056: Rows 56/105 never passed; ii_/iw_ identity of rows 28/5/2 not in output.

## SH-T14 (P3): Is pendingDone2 in selfheal_ui_test.cpp only ever called with now >= 1100, never with now==0 and unqueuedSince==0 and queued==false?
- **SH-3b-05** v2=CONFIRMED (G1 CONFIRMED) | claimed low test-gap | pendingDone2 now==0 sentinel branch untested | file patch_manager_mods/_shared_src/selfheal_ui_test.cpp fn main (pendingDone2 now==0)
  - G1 evidence: patch_manager_mods/_shared_src/SelfHealUi.h:382; patch_manager_mods/_shared_src/selfheal_ui_test.cpp:335-354 | G1 note: Gap real; the anchor sits in SelfHealUi.h:382. Test is right but fails: now==0 wraps and times out at once.
  - v2 evidence: patch_manager_mods/_shared_src/SelfHealUi.h:382; patch_manager_mods/_shared_src/selfheal_ui_test.cpp:335-354 | v2 notes: 

## SH-T15 (P3): Does sceneview_test.cpp's installFakes overwrite every default call lambda (callSceneInit/SetPos/SetOri/GetVec/GetParent/CamAttach/CamFov) before use, with no test of the defaults?
- **SH-3b-06** v2=UNSURE (G1 PARTIAL) | claimed low test-gap | sceneview_test replaces all default call lambdas so defaults are never tested | file patch_manager_mods/_shared_src/sceneview_test.cpp fn main (default vtable-slot lambdas)
  - G1 evidence: patch_manager_mods/_shared_src/sceneview_test.cpp:67-68; patch_manager_mods/_shared_src/SceneView.h:47-72 | G1 note: Overwrite of all 7 call lambdas true; \\\"defaults never tested\\\" true only within read ranges
  - v2 evidence: patch_manager_mods/_shared_src/sceneview_test.cpp:158-160; patch_manager_mods/_shared_src/sceneview_test.cpp:156; patch_manager_mods/_shared_src/SceneView.h:81-86 | v2 notes: [gem2 UNSURE] u041: In range: no default asserted; rebind then installFakes before use; all-7 overwrite outside range.

## SH-T16 (P3): Does gui2d_test.cpp set clipNoFill, flushArgAddr, clipDepthAddr and all function pointers up front so that pushClip's null-colour return and the K2AddrTable-backed defaults are never exercised?
- **SH-3c-02** v2=PARTIAL (G1 PARTIAL) | claimed medium test-gap | gui2d_test overrides all engine hooks; K2AddrTable-backed default paths untested | file patch_manager_mods/_shared_src/gui2d_test.cpp fn main / installFakes
  - G1 evidence: patch_manager_mods/_shared_src/gui2d_test.cpp:47-48; patch_manager_mods/_shared_src/Gui2D.h:83,86,95,107 | G1 note: Up-front overrides and unreached defaults hold in range; hook setup and rest of file unread. Removing overrides reads engine addrs on host; unsafe.
  - v2 evidence: patch_manager_mods/_shared_src/gui2d_test.cpp:27-34,88,129-131,147-150; patch_manager_mods/_shared_src/k2addr_test.cpp:492-495,508-509 | v2 notes: [gem2 PARTIAL] u042: Visible test blocks use overrides; checkGui2D asserts table-backed clipNoFill and empty-table neutrals; dropping overrides is unsafe

## SH-T17 (P3): Is actq::playerPaused untested in actqueue_test.cpp (and not called by QueueHud's tests), and does the test omit asserting s.rn[0].spellId and the kMaxEntries/kMaxRound truncation?
- **SH-3c-04** v2=CONFIRMED (G1 PARTIAL) | claimed medium test-gap | actq::playerPaused untested anywhere; truncation bounds and rn spellId unasserted | file patch_manager_mods/_shared_src/actqueue_test.cpp fn main
  - G1 evidence: patch_manager_mods/_shared_src/actqueue_test.cpp:90,101; patch_manager_mods/_shared_src/ActQueue.h:98,120 | G1 note: Gaps real in-range (rn spellId, truncation); playerPaused needs fixed-address stub; QueueHud unverified
  - v2 evidence: patch_manager_mods/companion_selfheal_src/common_contract_test.cpp:289 ; no-hits | v2 notes: [det CONFIRMED] d057: playerPaused only in a comment and a fn-pointer cast; never called. || [det CONFIRMED] d058: actqueue_test.cpp has no rn spellId asserts and no >16 or >4 feeds.

## SH-T18 (P3): Does filewatch_test.cpp never pass a non-null Stamp* out to filewatch::changed, and is the mtimeMs assertion in test_stamp guarded by a nanosecond-support check?
- **SH-3c-05** v2=CONFIRMED (G1 PARTIAL) | claimed low test-gap | filewatch_test never passes Stamp out param; mtimeMs check skipped on coarse FS | file patch_manager_mods/_shared_src/filewatch_test.cpp fn test_poll / test_stamp
  - G1 evidence: patch_manager_mods/_shared_src/filewatch_test.cpp:51-53,59; patch_manager_mods/_shared_src/FileWatch.h:81 | G1 note: Out-param gap true for lines 29-100 only; coarse-FS skip is deliberate, not a defect
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d059: No changed() call passes a Stamp* out-param in filewatch_test.cpp.

## SH-T19 (P3): Does guitip_test.cpp lack cases for readMgr with an unreadable mgr pointer or partially readable mgr, and for measure() with a null lines or maxLen pointer?
- **SH-3c-06** v2=CONFIRMED (G1 PARTIAL) | claimed low test-gap | guitip_test lacks unreadable-manager, partial-readability and null out-param measure cases | file patch_manager_mods/_shared_src/guitip_test.cpp fn main
  - G1 evidence: patch_manager_mods/_shared_src/guitip_test.cpp:53,66,101-105 | G1 note: Missing: null out-params for measure, non-null unreadable mgr, partial field reads; null mgr and unreadable cell covered.
  - v2 evidence: patch_manager_mods/_shared_src/guitip_test.cpp:104 | v2 notes: [det CONFIRMED] d060: All measure() calls pass &l,&mx; nullptr only as text arg. || [gem2 PARTIAL] u043: Non-null mgr<0x10000 feasible (gate :101); partial reads blocked by stub-true cache; test lines unread

## SH-T20 (P3): Does item_read_test.cpp lack cases for an unreadable entry array, count -1 or 257, and a full 16-byte non-NUL-terminated icon resref?
- **SH-3c-07** v2=CONFIRMED (G1 PARTIAL) | claimed low test-gap | item_read_test misses unreadable entries, count bounds and 16-byte icon resref | file patch_manager_mods/_shared_src/item_read_test.cpp fn main
  - G1 evidence: patch_manager_mods/_shared_src/item_read_test.cpp:51,63,65; patch_manager_mods/_shared_src/ItemRead.h:33,36,40-44 | G1 note: Anchor is in ItemRead.h:36, not the test. Only count 100000 tested; unreadable-entry, count -1/257, 16-byte icon untested in range.
  - v2 evidence: patch_manager_mods/_shared_src/item_read_test.cpp:51 ; patch_manager_mods/_shared_src/item_read_test.cpp:54 | v2 notes: [det CONFIRMED] d061: Entry bases always valid; no count -1/256/257 cases; 100000 only over-bound case. || [det CONFIRMED] d062: Only 9-byte 'i_medpac' icon tested; no 16 non-NUL resref case.

## SH-T21 (P3): In textfit_test.cpp, does the F10 marqueeText call use off=7 (not a space in the string) and are the fitText trailing-space strip, negative off and over-120-char wrapAppendW word cases absent?
- **SH-3c-09** v2=CONFIRMED (G1 PARTIAL) | claimed low test-gap | textfit_test misses trailing-space trim, off-on-space, negative off, over-120-char word | file patch_manager_mods/_shared_src/textfit_test.cpp fn main
  - G1 evidence: patch_manager_mods/_shared_src/textfit_test.cpp:52; patch_manager_mods/_shared_src/TextFit.h:31,42,43,61 | G1 note: Anchor exact. Off=7 is 'e', not a space. Off-on-space, negative off, >120 word absent in 30-70. Trailing-space trim not targeted.
  - v2 evidence: patch_manager_mods/_shared_src/textfit_test.cpp:50 | v2 notes: [det CONFIRMED] d063: Offsets 0,0,3,100,7; no negative, trailing-space, space-index or >120-char case.
