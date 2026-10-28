# WI-163 facts: feats/powers level-up panels (Steam exe)
Evidence tags: [SF] static_facts.py addr · [QG] query_ghidra.py address/symbol · [ND] named_decomp.py run --addrs (pseudo-C; scratch copies not kept, rerun to regenerate)

## 1. Panel identity
- 0x00909E00 = CSWGuiFeatsCharGen::Constructor [QG steam_val row, verified C1b]. Size 2914 B, RET 16 (4 stack args + ECX=this) [SF].
- 0x009074E0 = CSWGuiPowersLevelUp::Constructor [QG steam_val row, verified C1b]. Size 2903 B, RET 8 [SF].
- Callers of 0x909E00 [SF]: 0x00904510 (unnamed, size 239, RET 4; CALLs CSWCLevelUpStats::CalcNumberFeats 0x847E50; passes flag 1 [ND, prior pass]) = level-up path. 0x0090B430 CSWGuiFeatsCharGen::OnAcceptButton (size 634, RET 0; its callers are InitFeatLists 0x90AEA0 and OnInputEvent 0x90B200 [SF]). 0x00914E70 (unnamed, size 203, RET 4; passes flag 0, guarded by this+0x2120 > 2 [ND, prior pass]).
- Callers of 0x9074E0 [SF]: 0x00904420 (unnamed, size 191, RET 4; CALLs 0x9074E0 and operator new [SF]; alloc 0x1C0C [ND, prior pass]).
- DECISION: 0x909E00 is the level-up feats screen. Basis: caller 0x904510 CALLs it and CALLs CalcNumberFeats [SF]; BuildChoosableFeatVector 0x90C230 CALLs CSWCLevelUpStats::CanChooseFeat 0x8480B0 [SF]. Class name stays CSWGuiFeatsCharGen. Role of 0x914E70 OPEN.
- TRAP: [QG] address prints GOG-VA rows first (GOG 0x00909E00 = GetTextureAttrib, 65b). Use `query_ghidra.py symbol <Class>` for GOG ctor rows: CSWGuiFeatsCharGen 0x005F8C90 (2914b), CSWGuiPowersLevelUp 0x005F6370 (2903b). Both sizes match Steam [QG].

## 2. Per-frame / input sites
- Ctor writes vtable: `*in_ECX = CSWGuiFeatsCharGen::vftable` (ND 0x909E00 line 51) and `CSWGuiPowersLevelUp::vftable` (ND 0x9074E0 line 46). Symbol names only, no address. `query_ghidra.py symbol <Class>::vftable` and `struct <Class>` return no rows. OPEN.
- Draw candidates: feats 0x0090BBB0 (size 244, RET 4), powers 0x009091E0 (size 240, RET 4) [SF]. Each CALLs CSWGuiTextParams::SetText 0x416E30, SetColor 0x417010, and CSWGuiPanel::Draw 0x40EC90 [SF] (listed as CALL, not JMP). Text refresh fits a per-frame draw, but the vtable slot is unproven.
- REA callsite slot 0x34 (0x007683A4, 0x007683BF, 0x00768409) lists both as unresolved candidates [QG, prior pass]. Slot not proven.
- HandleInputEvent (event, not per-frame): feats 0x0090B200 CSWGuiFeatsCharGen::OnInputEvent, powers 0x00908920 CSWGuiPowersLevelUp::OnInputEvent. Both size 281, RET 8 [SF]. Each CALLs virtual slot 0x3c internally (0x90B2E4/0x90B301; 0x908A04/0x908A21) [SF]. That is a call made inside, not its own slot.
- Ctor-internal virtual calls run during construction, not per-frame [SF]: feats slot 0x98 at 0x90A72C and 0xA4 at 0x90A79D; powers slot 0x98 at 0x907DC1 and 0xA4 at 0x907E72.
- Deleting-dtor candidate 0x0090A970: size 44, RET 4, CALLs 0x90A9A0 and operator delete 0x9196FD [SF]. 0x90A9A0 = non-deleting dtor, size 1004 B, sole caller 0x90A970 [SF]; matches GOG ~CSWGuiFeatsCharGen 1004b [QG]. Slot 0 candidate per REA [QG, prior pass].
- Prologue bytes (first 8 for 0x90BBB0 / 0x9091E0 / 0x90B200 / 0x908920): no tool here prints them. OPEN.

## 3. Label and button tags (string literals in both ctors) [ND .c strings, re-checked this pass; 15 tags each]
- In BOTH: SUB_TITLE_LBL, MAIN_TITLE_LBL, REMAINING_BOX_LBL, LBL_BAR1, LBL_BAR2, LB_DESC.
- Feats only: LBL_NAME, LB_FEATS, STD_SELECTIONS_REMAINING_LBL, STD_REMAINING_SELECTIONS_LBL, BTN_SELECT, BTN_RECOMMENDED, BTN_BACK, BTN_ACCEPT. Layout literal FTCHRGEN_p.
- Powers only: LBL_POWER, LB_POWERS, SELECTIONS_REMAINING_LBL, REMAINING_SELECTIONS_LBL, SELECT_BTN, RECOMMENDED_BTN, BACK_BTN, ACCEPT_BTN. Layout literal pwrlvlup_p.
- No button tag is shared (BTN_X vs X_BTN). Count-label tags differ.
- LevelUpUi.h:142-166 not read (outside brief). Template fit OPEN.

## 4. Calling convention [SF size/RET/callers; ND signatures]
- 0x0090C230 BuildChoosableFeatVector: size 344, RET 0. Callers [SF]: InitFeatLists 0x0090AEA0, 0x0090BF90, 0x0090C120. CALLs CSWCLevelUpStats::CanChooseFeat 0x8480B0 [SF]. `void f(void)`, reads this [ND].
- 0x0090BD50 CSWGuiFeatsCharGen::RefreshGridStates: size 570, RET 0. Callers [SF]: InitFeatLists 0x0090AEA0, OnRecommendedButton 0x0090B6B0, OnSlotClicked 0x0090B890.
- 0x009098D0 CSWGuiPowersLevelUp::BuildChoosablePowerVector: size 676, RET 0. Callers [SF]: BuildKnownPowerLists 0x00908540, 0x9095F0, 0x9097A0.
- 0x00909360 CSWGuiPowersLevelUp::RefreshGridStates: size 644, RET 0. Callers [SF]: 0x00908540, 0x908CC0, 0x908E30.
- All four: RET 0 = no stack args, __thiscall, `int in_ECX` = this [SF][ND]. They read this+0x1BA8..0x1BF8 (state set by init) [ND]. Out-of-band call with only `this` is safe only after init. Implicit-ECX grid calls not shown. OPEN.
- 0x00908540 = CSWGuiPowersLevelUp::BuildKnownPowerLists [QG steam_val, verified C1b]; size 986, RET 0, sole caller 0x9074E0 [SF]. The doc 25 "feats" conflict is not reproduced.

## 5. Grid offsets
- +0x1BF4 read in CSWGuiFeatsCharGen::RefreshGridStates (0x90BD50, ND line 36): `FUN_0041fc30(this+0x1BF4,0,1,0)` only when ushort +0x1BD8 == -1; then `FUN_0089C290()` result stored in +0x1BD8 [ND]. Confirmed read. Grid role OPEN.
- +0x1BF8 read in CSWGuiPowersLevelUp::RefreshGridStates (0x909360, ND line 44): same pattern, dword guard +0x1BB0 == -1, result stored in +0x1BB0 [ND]. Confirmed read.
- Neither offset appears in BuildChoosableFeatVector 0x90C230 or BuildChoosablePowerVector 0x9098D0 (offset grep of ND text) [ND].

## OPEN (needs rea-recon)
1. vftable addresses and the Draw / per-frame slot number (0x34 unresolved).
2. First 8 prologue bytes: 0x0090BBB0, 0x009091E0, 0x0090B200, 0x00908920.
3. Object behind implicit ECX in CSWGuiLevelUpIconGrid__* calls in both RefreshGridStates.
4. Role of FUN_0041FC30 (what +0x1BF4 / +0x1BF8 point to).
5. Role of caller 0x00914E70. Pseudo-C of 0x0090B430 to confirm its call to 0x909E00 (callee list truncated [SF]).
6. LevelUpUi.h:142-166 template requirements.

## Proposed DB rows (owner files via kotor2-address-db later)
- 0x00904510 | level-up feats opener (unnamed) | medium
- 0x00904420 | level-up powers opener (unnamed) | medium
- 0x00914E70 | feats opener, flag 0 (unnamed) | low
- 0x0090A970 | CSWGuiFeatsCharGen::~CSWGuiFeatsCharGen (deleting) | high (size + sole-caller match)
- 0x0090BBB0 | CSWGuiFeatsCharGen::Draw override | low until slot proven
- 0x009091E0 | CSWGuiPowersLevelUp::Draw override | low until slot proven

## Ops notes
- named_decomp `--class C` returns "no targets selected" for these classes; use `--addrs`. [ND] annotate failed for 0x733570/0x733780/0x7337C0 and operator new 0x919723; pseudo-C still produced.
- kotor2_ghidra_knowledge.db mtime 22:49:39 overlaps another session's promote_names.py run (22:49:33). Not attributed to this worker.
