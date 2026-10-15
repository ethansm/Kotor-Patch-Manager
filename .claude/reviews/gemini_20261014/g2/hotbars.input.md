# G2 resolver input: batch hotbars (11 open fids, 8 topics)

## HB-T01 (P1) — G1 unit u002, facts file recon/HB-T01.md
Question: In TwoDA::ParseImpl, can a malformed 2DA (cell offset into a blob with no NUL before EOF, or nrows*cols.size()/cells*2 overflowing 32-bit size_t) pass the bounds checks and cause an unbounded string copy or out-of-bounds read, and does twoda_test.cpp lack cases for both?
G1 windows (do NOT reuse): patch_manager_mods/action_hotbar_src/TwoDA.h;patch_manager_mods/action_hotbar_src/twoda_test.cpp:95-110;patch_manager_mods/action_hotbar_src/twoda_test.cpp:170-186
G1 OPEN: Does twoda_test.cpp:24 (outside read range) already exercise high column counts?
- HB-1a-01 [PARTIAL] Unbounded cell string scan in 2DA parse when NUL missing | file patch_manager_mods/action_hotbar_src/TwoDA.h | fn hb::TwoDA::ParseImpl | cat memory-safety | G1 evidence patch_manager_mods/action_hotbar_src/TwoDA.h:87-88 | G1 note: Single scan already ends at d.size(); real issue is aggregate copy cost. Rejecting NUL-less cells breaks test :177. | anchor now patch_manager_mods/action_hotbar_src/TwoDA.h:88 drift none
- HB-1d-01 [PARTIAL] twoda_test lacks missing-NUL mid-blob cell case | file patch_manager_mods/action_hotbar_src/twoda_test.cpp | fn main (TwoDA::ParseImpl) | cat test-gap | G1 evidence patch_manager_mods/action_hotbar_src/twoda_test.cpp:177-186 | G1 note: EOF-terminated last cell is tested. Mid-blob NUL-less case is absent in read ranges; expected value must be decided. | anchor now patch_manager_mods/action_hotbar_src/twoda_test.cpp:177 drift none
- HB-1d-02 [UNSURE] twoda_test lacks high column count overflow case | file patch_manager_mods/action_hotbar_src/twoda_test.cpp | fn main (TwoDA::ParseImpl) | cat test-gap | G1 evidence patch_manager_mods/action_hotbar_src/twoda_test.cpp:102-107 | G1 note: Only row-count tests in range. Anchor :24 is outside range. 64-bit host cannot wrap size_t; needs guard at TwoDA.h:79. | anchor now patch_manager_mods/action_hotbar_src/twoda_test.cpp:24,patch_manager_mods/action_hotbar_src/twoda_test.cpp:96,patch_manager_mods/action_hotbar_src/twoda_test.cpp:110 drift none

## HB-T02 (P2) — G1 unit u017, facts file recon/HB-T02.md
Question: Does readCreatureTag copy engine tag bytes without rejecting ']' , newline or non-printable characters, so the tag can reach characterKeyFor and produce a broken [Profile.<tag>] section on Serialize?
G1 windows (do NOT reuse): patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:360-420;patch_manager_mods/action_hotbar_src/HotbarLogic.h:85-125;patch_manager_mods/_shared_src/SafeRead.h:120-150
G1 OPEN: Does the Serialize/Config::Save writer emit [Profile.<key>] unescaped, so an interior ']' or newline breaks the section?
- HB-1a-03 [PARTIAL] Creature tag copied raw; ']' or newline corrupts INI section header | file patch_manager_mods/action_hotbar_src/ActionHotbar.cpp | fn readCreatureTag | cat logic | G1 evidence patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:373-375; patch_manager_mods/action_hotbar_src/HotbarLogic.h:97-100 | G1 note: Raw copy, no ]/newline filter (373-375) confirmed; Serialize corruption unverified; readExo filter misses ']'. | anchor now patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:374 drift none

## HB-T05 (P2) — G1 unit u019, facts file recon/HB-T05.md
Question: Does normalizeItemName truncate at the FIRST ' (' (not just a trailing stack-count suffix), so two items like 'X (Red)' and 'X (Blue)' normalize to the same name?
G1 windows (do NOT reuse): patch_manager_mods/action_hotbar_src/HotbarConfig.h:70-100;patch_manager_mods/action_hotbar_src/HotbarLogic.h:538-552;patch_manager_mods/action_hotbar_src/HotbarConfig.h:178-188
G1 OPEN: Does the engine emit distinct items that share text before the first ' ('? Needs item data outside ranges.
- HB-1a-06 [PARTIAL] normalizeItemName truncates at first ' (' merging distinct item names | file patch_manager_mods/action_hotbar_src/HotbarConfig.h | fn hb::normalizeItemName | cat logic | G1 evidence patch_manager_mods/action_hotbar_src/HotbarConfig.h:77-78; :74 | G1 note: Mechanism real; merge matches :74 identity rule; defect needs engine item data. | anchor now patch_manager_mods/action_hotbar_src/HotbarConfig.h:77,patch_manager_mods/action_hotbar_src/HotbarConfig.h:167,patch_manager_mods/action_hotbar_src/HotbarConfig.h:317 drift none

## HB-T06 (P3) — G1 unit u057, facts file recon/HB-T06.md
Question: Does 07_force_power_hotbar.md 'Layout and controls' still describe the obj<hexid> character key and old hook list while characterKeyFor now uses creature tag / party slot keys and a 0xFFFFFFF0..FE Exile range?
G1 windows (do NOT reuse): patch_manager_mods/07_force_power_hotbar.md:88-100;patch_manager_mods/action_hotbar_src/HotbarLogic.h:85-125
G1 OPEN: Do the 8 hook addresses in 07_force_power_hotbar.md:94 match the installed hook sites (ActionHotbar.cpp or README, outside ranges)?
- HB-1b-01 [PARTIAL] Design doc character key and hook list out of sync with code | file patch_manager_mods/07_force_power_hotbar.md | fn characterKeyFor (doc vs HotbarLogic.h) | cat doc | G1 evidence patch_manager_mods/07_force_power_hotbar.md:95; patch_manager_mods/action_hotbar_src/HotbarLogic.h:97-117 | G1 note: Character-key part confirmed; hook list (doc line 94) not verifiable from ranges | anchor now - drift noanchor

## HB-T07 (P2) — G1 unit u020, facts file recon/HB-T07.md
Question: Can ed.off go negative (clampOffset has no lower bound, edPicker uses (size_t)ed.off+i) or can chooseTarget select a candidate with cur<=0 and down=false, and is either case untested in hotbar_logic_test.cpp?
G1 windows (do NOT reuse): patch_manager_mods/action_hotbar_src/HotbarLogic.h:18-40;patch_manager_mods/action_hotbar_src/HotbarLogic.h:262-275;patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:1420-1445
G1 OPEN: Are ed.off and Candidate.cur<0 with down=false ever set in scrollPicker/setup, and is either case covered in hotbar_logic_test.cpp (outside ranges)?
- HB-1b-02 [PARTIAL] clampOffset has no lower clamp and no negative-offset test | file patch_manager_mods/action_hotbar_src/HotbarLogic.h | fn hb::clampOffset | cat test-gap | G1 evidence patch_manager_mods/action_hotbar_src/HotbarLogic.h:270 (no lower bound); patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:1423 (called only when off > maxOff) | G1 note: Lower clamp is safe; HotbarLogic.h:274 scrollClamp already does [0,maxOff]. | anchor now patch_manager_mods/action_hotbar_src/HotbarLogic.h:270 drift none
- HB-1b-03 [PARTIAL] chooseTarget accepts negative cur HP with down=false; no test | file patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp | fn hb::chooseTarget / main | cat test-gap | G1 evidence patch_manager_mods/action_hotbar_src/HotbarLogic.h:29-30 (no cur check; negative cur passes); hotbar_logic_test.cpp not in range | G1 note: Test-only fix safe; a cur<=0 reject guard could drop real lowest-HP targets. | anchor now - drift noanchor

## HB-T09 (P3) — G1 unit u059, facts file recon/HB-T09.md
Question: \\\"Does tlk_test.cpp use mkdtemp plus system(\\\"\\\"rm -rf '<dir>'\\\"\\\") instead of kt::TempDir, and does twoda_test.cpp's optional dialog.tlk step print strings without any KT_CHECK assertion?\\\"
G1 windows (do NOT reuse): patch_manager_mods/action_hotbar_src/tlk_test.cpp:60-75;patch_manager_mods/action_hotbar_src/tlk_test.cpp:220-232;patch_manager_mods/action_hotbar_src/twoda_test.cpp:185-195
G1 OPEN: Does tlk_test.cpp lines 76-219 (outside ranges) contain an early return that skips the cleanup at line 225?
- HB-1d-04 [PARTIAL] tlk_test uses system rm -rf instead of kt::TempDir, leaks on early exit | file patch_manager_mods/action_hotbar_src/tlk_test.cpp | fn main | cat tooling | G1 evidence patch_manager_mods/action_hotbar_src/tlk_test.cpp:66-68, 227-228 | G1 note: mkdtemp plus system rm -rf confirmed; early-exit leak unverifiable in ranges; kt::TempDir unchecked | anchor now - drift noanchor

## HB-T12 (P2) — G1 unit u021, facts file recon/HB-T12.md
Question: In updateButtons, can a slot reassigned to an entry with an empty g_names entry keep the previous tooltip (setNative skipped yet tipKey/tipSet updated), and can orderSig (count+keys only) match g_nameSig while g_nameValid is true so stale names are served?
G1 windows (do NOT reuse): patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:495-535;patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:695-715;patch_manager_mods/combat_hotbar_src/CombatLogic.h:455-475
G1 OPEN: Does guitip::setNative or any code outside the listed ranges clear b.bg.c's tooltip on an empty name?
- HB-1c-04 [PARTIAL] Name cache signature can match across churn giving stale names | file patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp | fn updateButtons | cat logic | G1 evidence patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:507-516; patch_manager_mods/combat_hotbar_src/CombatLogic.h:460-466 | G1 note: Sig hashes count+keys only; needs collision or same-key rename to serve stale. | anchor now patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:510 drift none

## HB-T13 (P2) — G1 unit u022, facts file recon/HB-T13.md
Question: When combat_hotbar.ini is hot-reloaded from [Arrows] Hide=1 to Hide=0 (or Replace toggled), does any code restore the arrow sub-controls' bit 1 or reset g_arrowState to -1?
G1 windows (do NOT reuse): patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:735-765;patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:105-120
G1 OPEN: Do stock or other CombatHotbar.cpp writers re-set arrow bit 1 or reset g_arrowState outside these ranges?
- HB-1c-05 [PARTIAL] Arrow hide state not restored/reset on ini hot-reload | file patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp | fn onTargetMenuPost | cat logic | G1 evidence patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:740-751,753-762 | G1 note: Gap real: no bit-1 restore, no g_arrowState reset. Visible impact unproven (stock repopulation outside range). | anchor now patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:740 drift none
