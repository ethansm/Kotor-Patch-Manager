# G2.3 skeptic input: hotbars (16 fids)

## HB-T01 (P1): In TwoDA::ParseImpl, can a malformed 2DA (cell offset into a blob with no NUL before EOF, or nrows*cols.size()/cells*2 overflowing 32-bit size_t) pass the bounds checks and cause an unbounded string copy or out-of-bounds read, and does twoda_test.cpp lack cases for both?
- **HB-1a-01** v2=PARTIAL (G1 PARTIAL) | claimed high memory-safety | Unbounded cell string scan in 2DA parse when NUL missing | file patch_manager_mods/action_hotbar_src/TwoDA.h fn hb::TwoDA::ParseImpl
  - G1 evidence: patch_manager_mods/action_hotbar_src/TwoDA.h:87-88 | G1 note: Single scan already ends at d.size(); real issue is aggregate copy cost. Rejecting NUL-less cells breaks test :177.
  - v2 evidence: patch_manager_mods/action_hotbar_src/TwoDA.h:79 | v2 notes: [det PARTIAL] d015: nrows capped at 100000 (:73) but nrows times cols.size() at :79 has no cap or overflow check
- **HB-1a-02** v2=CONFIRMED (G1 CONFIRMED) | claimed high memory-safety | size_t multiplication overflow in 2DA offset table check on 32-bit | file patch_manager_mods/action_hotbar_src/TwoDA.h fn hb::TwoDA::ParseImpl
  - G1 evidence: patch_manager_mods/action_hotbar_src/TwoDA.h:79-80,86 | G1 note: 32-bit size_t wrap of nrows*cols defeats :80 check; :86 index into rd16 is unchecked. 64-bit safe.
  - v2 evidence: patch_manager_mods/action_hotbar_src/TwoDA.h:79-80,86 | v2 notes: 
- **HB-1d-01** v2=CONFIRMED (G1 PARTIAL) | claimed medium test-gap | twoda_test lacks missing-NUL mid-blob cell case | file patch_manager_mods/action_hotbar_src/twoda_test.cpp fn main (TwoDA::ParseImpl)
  - G1 evidence: patch_manager_mods/action_hotbar_src/twoda_test.cpp:177-186 | G1 note: EOF-terminated last cell is tested. Mid-blob NUL-less case is absent in read ranges; expected value must be decided.
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d016: No NUL-less or unterminated cell test in twoda_test.cpp: gap, low severity
- **HB-1d-02** v2=CONFIRMED (G1 UNSURE) | claimed medium test-gap | twoda_test lacks high column count overflow case | file patch_manager_mods/action_hotbar_src/twoda_test.cpp fn main (TwoDA::ParseImpl)
  - G1 evidence: patch_manager_mods/action_hotbar_src/twoda_test.cpp:102-107 | G1 note: Only row-count tests in range. Anchor :24 is outside range. 64-bit host cannot wrap size_t; needs guard at TwoDA.h:79.
  - v2 evidence: patch_manager_mods/action_hotbar_src/twoda_test.cpp:102 | v2 notes: [det CONFIRMED] d017: Only implausible-row-count cases at :102-107; no many-column Build2da case found

## HB-T02 (P2): Does readCreatureTag copy engine tag bytes without rejecting ']' , newline or non-printable characters, so the tag can reach characterKeyFor and produce a broken [Profile.<tag>] section on Serialize?
- **HB-1a-03** v2=PARTIAL (G1 PARTIAL) | claimed medium logic | Creature tag copied raw; ']' or newline corrupts INI section header | file patch_manager_mods/action_hotbar_src/ActionHotbar.cpp fn readCreatureTag
  - G1 evidence: patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:373-375; patch_manager_mods/action_hotbar_src/HotbarLogic.h:97-100 | G1 note: Raw copy, no ]/newline filter (373-375) confirmed; Serialize corruption unverified; readExo filter misses ']'.
  - v2 evidence: patch_manager_mods/action_hotbar_src/HotbarConfig.h:269-273, :409 | v2 notes: [gem2 PARTIAL] u013: Sink confirmed: profile() stores raw name, emit at :409 unescaped. Source and reparse outside range.

## HB-T04 (P2): In ensureWheelHook, when g_guiInput.connected is true and wheel_subscribe keeps returning <=0, does the function return every frame without ever falling back to WheelHook::install?
- **HB-1a-05** v2=CONFIRMED (G1 CONFIRMED) | claimed medium logic | Wheel hook never falls back to WindowHook when gui-input subscribe keeps failing | file patch_manager_mods/action_hotbar_src/ActionHotbar.cpp fn ensureWheelHook
  - G1 evidence: patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:1199-1208 | G1 note: Connected branch returns unconditionally; WheelHook fallback unreachable. Naive fallthrough can double-count wheel input.
  - v2 evidence: patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:1199-1208 | v2 notes: 

## HB-T05 (P2): Does normalizeItemName truncate at the FIRST ' (' (not just a trailing stack-count suffix), so two items like 'X (Red)' and 'X (Blue)' normalize to the same name?
- **HB-1a-06** v2=UNSURE (G1 PARTIAL) | claimed medium logic | normalizeItemName truncates at first ' (' merging distinct item names | file patch_manager_mods/action_hotbar_src/HotbarConfig.h fn hb::normalizeItemName
  - G1 evidence: patch_manager_mods/action_hotbar_src/HotbarConfig.h:77-78; :74 | G1 note: Mechanism real; merge matches :74 identity rule; defect needs engine item data.
  - v2 evidence: patch_manager_mods/07_force_power_hotbar.md:922, :927; patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:241 | v2 notes: [gem2 UNSURE] u014: Identity truncation documented (07:927); merge needs base names with ' ('; wholesale removal keeps count in identity.

## HB-T06 (P3): Does 07_force_power_hotbar.md 'Layout and controls' still describe the obj<hexid> character key and old hook list while characterKeyFor now uses creature tag / party slot keys and a 0xFFFFFFF0..FE Exile range?
- **HB-1b-01** v2=CONFIRMED (G1 PARTIAL) | claimed low doc | Design doc character key and hook list out of sync with code | file patch_manager_mods/07_force_power_hotbar.md fn characterKeyFor (doc vs HotbarLogic.h)
  - G1 evidence: patch_manager_mods/07_force_power_hotbar.md:95; patch_manager_mods/action_hotbar_src/HotbarLogic.h:97-117 | G1 note: Character-key part confirmed; hook list (doc line 94) not verifiable from ranges
  - v2 evidence: patch_manager_mods/action_hotbar_src/kotor2-steam-aspyr.hooks.toml:8 | v2 notes: [det CONFIRMED] d018: Active hook is 0x0072CB84 (hooks.toml:8); 07_force_power_hotbar.md:94 still lists 0x0072CB63

## HB-T07 (P2): Can ed.off go negative (clampOffset has no lower bound, edPicker uses (size_t)ed.off+i) or can chooseTarget select a candidate with cur<=0 and down=false, and is either case untested in hotbar_logic_test.cpp?
- **HB-1b-02** v2=CONFIRMED (G1 PARTIAL) | claimed low test-gap | clampOffset has no lower clamp and no negative-offset test | file patch_manager_mods/action_hotbar_src/HotbarLogic.h fn hb::clampOffset
  - G1 evidence: patch_manager_mods/action_hotbar_src/HotbarLogic.h:270 (no lower bound); patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:1423 (called only when off > maxOff) | G1 note: Lower clamp is safe; HotbarLogic.h:274 scrollClamp already does [0,maxOff].
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d019: No clampOffset negative-offset test in hotbar_logic_test.cpp: test gap, harmless since setters clamp to zero || [gem2 UNSURE] u015: Lower clamp to 0 is harmless; clampOffset body and callers unread.
- **HB-1b-03** v2=PARTIAL (G1 PARTIAL) | claimed low test-gap | chooseTarget accepts negative cur HP with down=false; no test | file patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp fn hb::chooseTarget / main
  - G1 evidence: patch_manager_mods/action_hotbar_src/HotbarLogic.h:29-30 (no cur check; negative cur passes); hotbar_logic_test.cpp not in range | G1 note: Test-only fix safe; a cur<=0 reject guard could drop real lowest-HP targets.
  - v2 evidence: patch_manager_mods/_shared_src/Party.h:118 | v2 notes: [det PARTIAL] d020: Party.h:118 marks cur<=0 as down (production fine); only negative-cur test at hotbar_logic_test.cpp:523 uses max=0: test gap only

## HB-T08 (P3): Does keybif_test.cpp lack ExpectFail cases for key-too-short, bad/empty bif name size, bif name out of range, bif-too-short and resource-id-not-in-bif branches present in ReadKeyBifResource?
- **HB-1d-03** v2=CONFIRMED (G1 CONFIRMED) | claimed medium test-gap | keybif_test misses several malformed KEY/BIF error branches | file patch_manager_mods/action_hotbar_src/keybif_test.cpp fn main (ReadKeyBifResource)
  - G1 evidence: patch_manager_mods/action_hotbar_src/keybif_test.cpp:53-63; patch_manager_mods/action_hotbar_src/KeyBif.h:123,150,152,154,163,176 | G1 note: Five named branches have no ExpectFail case; no test needle matches their errors.
  - v2 evidence: patch_manager_mods/action_hotbar_src/keybif_test.cpp:53-63; patch_manager_mods/action_hotbar_src/KeyBif.h:123,150,152,154,163,176 | v2 notes: 

## HB-T09 (P3): \\\"Does tlk_test.cpp use mkdtemp plus system(\\\"\\\"rm -rf '<dir>'\\\"\\\") instead of kt::TempDir, and does twoda_test.cpp's optional dialog.tlk step print strings without any KT_CHECK assertion?\\\"
- **HB-1d-05** v2=CONFIRMED (G1 CONFIRMED) | claimed low test-gap | twoda_test real-data TLK step has no assertions | file patch_manager_mods/action_hotbar_src/twoda_test.cpp fn main
  - G1 evidence: patch_manager_mods/action_hotbar_src/twoda_test.cpp:188-191 | G1 note: TLK step is printf only, no KT_CHECK; skip branch also passes silently
  - v2 evidence: patch_manager_mods/action_hotbar_src/twoda_test.cpp:188-191 | v2 notes: 

## HB-T12 (P2): In updateButtons, can a slot reassigned to an entry with an empty g_names entry keep the previous tooltip (setNative skipped yet tipKey/tipSet updated), and can orderSig (count+keys only) match g_nameSig while g_nameValid is true so stale names are served?
- **HB-1c-03** v2=CONFIRMED (G1 CONFIRMED) | claimed medium logic | Stale tooltip kept when new slot name read is empty | file patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp fn updateButtons
  - G1 evidence: patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:708-712 | G1 note: Clear or refresh tip on empty name is sound; guitip empty handling unverified.
  - v2 evidence: patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:708-712 | v2 notes: 
- **HB-1c-04** v2=CONFIRMED (G1 PARTIAL) | claimed medium logic | Name cache signature can match across churn giving stale names | file patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp fn updateButtons
  - G1 evidence: patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:507-516; patch_manager_mods/combat_hotbar_src/CombatLogic.h:460-466 | G1 note: Sig hashes count+keys only; needs collision or same-key rename to serve stale.
  - v2 evidence: patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:710 | v2 notes: [det CONFIRMED] d023: Only setNative at :710, guarded by name[0]; no clearNative call: empty name keeps stale tooltip || [gem2 UNSURE] u016: Decisive orderSig/g_nameValid code is outside ranges; in-range hide path does not clear names.

## HB-T13 (P2): When combat_hotbar.ini is hot-reloaded from [Arrows] Hide=1 to Hide=0 (or Replace toggled), does any code restore the arrow sub-controls' bit 1 or reset g_arrowState to -1?
- **HB-1c-05** v2=UNSURE (G1 PARTIAL) | claimed low logic | Arrow hide state not restored/reset on ini hot-reload | file patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp fn onTargetMenuPost
  - G1 evidence: patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:740-751,753-762 | G1 note: Gap real: no bit-1 restore, no g_arrowState reset. Visible impact unproven (stock repopulation outside range).
  - v2 evidence: patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:466-474; :479; :130 | v2 notes: [gem2 UNSURE] u017: No bit1 restore or g_arrowState use in 451-480; Hide branch 740-762 unread.

## HB-T14 (P3): In updateButtons, are tint[g]/tOk[g]/aOk[g] unconditionally overwritten right after getBorderTint/getAlpha, making the !tOk[g] log branch and the 0.85f alpha fallback unreachable?
- **HB-1c-06** v2=CONFIRMED (G1 CONFIRMED) | claimed low logic | Vanilla tint read immediately overwritten; failure branches are dead code | file patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp fn updateButtons
  - G1 evidence: patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:650,653-656,703 | G1 note: Removing dead tOk/aOk checks is safe; keep forced red at 653 (deliberate per 652).
  - v2 evidence: patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:650,653-656,703 | v2 notes: 
