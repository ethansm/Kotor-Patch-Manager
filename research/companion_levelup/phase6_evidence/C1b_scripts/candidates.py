"""C1b candidate list (R10_findings.md identities).  Data only; shared by build_packet.py and ../C1b_insert.py.

Each function candidate: addr, name, purpose, conf (R10 confidence, capped by the gate later), nb (neighbour address used
as the wrong-address decoy), sib (address whose name is borrowed for the wrong-name decoy), ev (evidence pointer into R10 files).
Grid class prefix "CSWGuiLevelUpIconGrid" is a LABEL (no RTTI name known); the DB row says so.
"""
BIN = "6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef"  # steam pristine (ghidra dump steam_6a522e71)
SRC = "level-up-C1b-R10-haiku-verified"
DATE = "2026-10-03"
R10 = "research/companion_levelup/phase6_evidence/R10_findings.md"

G = "CSWGuiLevelUpIconGrid"
F = "CSWGuiFeatsCharGen"
P = "CSWGuiPowersLevelUp"
S = "CSWGuiSkillsCharGen"
L = "CSWCLevelUpStats"

def fn(addr, name, purpose, conf, nb, sib, ev, anchor):
    return dict(addr=addr, name=name, purpose=purpose, conf=conf, nb=nb, sib=sib, ev=ev, anchor=anchor)

FUNCS = [
 fn(0x909E00, F+"::Constructor", "feats level-up panel ctor (size 0x1C08, grid ptr at +0x1BF4)", "high", 0x90A970, 0x9074E0, "R10 sec 0 table; R10_out_strings.txt, R10_out_callmap.txt", "strings FTCHRGEN_p/LB_FEATS; vftable 0x9AA434 written at 0x909E4A; calls 0x90AEA0"),
 fn(0x9074E0, P+"::Constructor", "powers level-up panel ctor (grid ptr at +0x1BF8)", "high", 0x908040, 0x90C6A0, "R10 sec 0; R10_raw_dec6.txt:35", "strings pwrlvlup_p/LB_POWERS; vftable 0x9AA34C at 0x90752A; 0x907DFE call 0x89AAA0"),
 fn(0x90C6A0, S+"::Constructor", "skills level-up panel ctor (8 fixed skill rows)", "high", 0x90E410, 0x909E00, "R10 sec 0; R10_raw_dec3.txt:1488", "strings SKCHRGEN_p, *_PLUS_BTN/*_MINUS_BTN; vftable 0x9AA714 at 0x90C6EA"),
 fn(0x902F60, "CSWGuiLevelUpPanel::Constructor", "level-up step panel ctor (+0x68 = live client creature)", "high", 0x903AA0, 0x8F4AF0, "R10 sec 0; R10_raw_dec5.txt:397", "string LEVELUPPNL_p at 0x9030D9; vftable 0x9AA1AC; +0x68 = 0x73F550(creatureId)"),
 fn(0x8F4AF0, "CSWGuiLevelUpCharGen::Constructor", "level-up outer panel ctor (+0x68 temp baseline creature, +0x2CB4 creature id)", "high", 0x8F7FE0, 0x902F60, "R10 sec 0; R10_raw_dec6.txt:272", "string MAINCG_p; existing DB class CSWGuiLevelUpCharGen::BeginPlayGame 0x8F8070"),
 fn(0x90AEA0, F+"::InitFeatLists", "feats panel init: known/granted vectors, pick counts, then grid 0x89B600, choosable 0x90C230, repaint 0x90BD50", "high", 0x90AE20, 0x908540, "R10 sec (a) feats; R10_raw_dec2.txt:339", "callees 0x89B600, 0x90C230, 0x90BD50, CalcNumberFeats 0x847E50; called from ctor 0x909E00"),
 fn(0x90C230, F+"::BuildChoosableFeatVector", "builds panel+0x1BC0 choosable feat vector, CanChooseFeat per id", "high", 0x90C120, 0x9098D0, "R10 sec (a); R10_out_disasm_90C230.txt", "CanChooseFeat call 0x90C349 (e8 62 bd f3 ff); 0x593450 push into +0x1BC0 at 0x90C360"),
 fn(0x90BD50, F+"::RefreshGridStates", "repaint feat grid slot states: 0x89C220(3) then known->1 selected->4 choosable->0 granted->2", "high", 0x90BF90, 0x909360, "R10 sec (c); R10_raw_dec2.txt:734", "callees 0x89C220 then 0x89C170 x4 (one per vector)"),
 fn(0x908540, P+"::BuildKnownPowerLists", "builds known-power vector (+0x1BC8) alphabetically, granted powers, picks left, then choosable 0x9098D0", "high", 0x9095F0, 0x90AEA0, "R10 sec (a) powers; R10_raw_dec3.txt:35", "callees 0x847F90 ComputeNumberKnownSpellsLeft, 0x847A70, 0x9098D0; called at 0x907E09 from ctor"),
 fn(0x9098D0, P+"::BuildChoosablePowerVector", "builds panel+0x1BD4 choosable power vector, 0x848660 per spell id", "high", 0x9092D0, 0x90C230, "R10 sec (a) powers; R10_out_disasm_9099xx.txt", "call 0x848660 at 0x9099DB (e8 80 ec f3 ff); loop bound [rules+0x104]"),
 fn(0x909360, P+"::RefreshGridStates", "repaint power grid slot states (spells 6 and 0xE forced to 3 when panel+0x1BAC bit0 == 0)", "high", 0x908F70, 0x90BD50, "R10 sec (c); R10_raw_dec3.txt:370", "callees 0x89C220/0x89C170; panel+0x1BAC bit0 test"),
 fn(0x89B600, G+"::BuildFeatRows", "builds feat icon grid from sorted feat list, rows via 0x89BED0 (call 0x89BE91)", "high", 0x89B320, 0x89AAA0, "R10 sec (a); R10_out_disasm_89B660_body.txt", "call 0x89BED0 at 0x89BE91; tag strings handmaiden/baodur/kreia/hanharr/atton/g0t0 compared via 0x734F60"),
 fn(0x89B100, G+"::BuildSortedFeatList", "sorts feat ids by tier then id, caches at GUI state +0x103C", "high", 0x89A0E0, 0x89A760, "R10 sec (a); R10_raw_dec4.txt:219", "skips 0xCF/0xEE and rows with +8==0; tier from +0x38/+0x3A == -1; insert 0x5934C0 / append 0x593450"),
 fn(0x89AAA0, G+"::BuildPowerRows", "builds power icon grid from sorted spell list, rows via 0x89AF30 (0x89ADD4, 0x89AEB3)", "high", 0x89B320, 0x89B600, "R10 sec (a) powers; R10_raw_dec4.txt:35", "sort 0x89A760; row add 0x89AF30 at 0x89ADD4/0x89AEB3; PlayerCreated owner+0x1200 branch"),
 fn(0x89A760, G+"::BuildSortedPowerList", "sorts spells.2da rows (type +0x14C in {1,6}, name strref != 0) by multi-digit key, caches at +0x1048", "high", 0x89B100, 0x89B100, "R10 sec (a) powers; R10_raw_dec7.txt:35", "column getter 0x6C11C0; key%10, key/1000, (key%1000)/10 ordering"),
 fn(0x89BED0, G+"::AddFeatRow", "allocs 0x324 grid row for (baseFeat, tier2, tier3); ids at row+0x108 + c*0xB4", "high", 0x89AEF0, 0x89AF30, "R10 sec (a); R10_raw_dec4.txt:145", "alloc 0x324; only callers 0x89BE91 and 0x89B5CD"),
 fn(0x89AF30, G+"::AddPowerRow", "allocs grid row for (spell, tier2, tier3, flag); power twin of 0x89BED0", "medium", 0x89BED0, 0x89BED0, "R10 sec (a) powers; R10_raw_dec7.txt:135 (body only skimmed)", "only callers 0x89ADD4/0x89AEB3 inside 0x89AAA0"),
 fn(0x89A160, G+"::SetSlotState", "sets grid slot state byte/colour: 0 normal 1/2 known-granted 3 dimmed 4 selected 5/6 misc", "high", 0x89A0E0, 0x89C220, "R10 sec (c); R10_raw_dec4.txt:284", "switch 0-6 over colour constants; state 3 = DAT_00A7FDD0"),
 fn(0x89C170, G+"::SetSlotStateById", "finds the slot holding an id by scanning rows and sets its state; silent no-op when absent", "high", 0x89C070, 0x89C070, "R10 sec (c); R10_raw_dec3.txt:2290", "row scan by id; calls 0x89A160"),
 fn(0x89C070, G+"::SelectSlotById", "finds the slot holding an id and selects it; silent no-op when absent", "high", 0x89C170, 0x89C170, "R10 sec (c); R10_raw_dec3.txt:2346", "row scan by id; cursor column/row bytes (+0xC/+0xD)"),
 fn(0x89C220, G+"::SetAllSlotStates", "sets every slot of every row to the given state (called with 3 = dimmed)", "high", 0x89C290, 0x89A160, "R10 sec (c); R10_raw_dec3.txt:2273", "loops rows x 3 slots calling 0x89A160"),
 fn(0x89C350, G+"::HandleNavKey", "arrow-key navigation over rows/columns, skipping empty (-1) slots", "high", 0x89C290, 0x89C070, "R10 sec (c); R10_raw_dec3.txt:2382", "key codes 0x2F-0x32 / 0x3D-0x40 from event switch"),
 fn(0x848660, L+"::CanChooseSpell", "power availability: spell type in {1,5}, class has powers, picks left, not known, not selected, prereq 0x846DD0", "high", 0x8480B0, 0x8480B0, "R10 sec (b); R10_raw_dec3.txt:690", "callers 0x848BF8 and 0x9099DB only; calls 0x846DD0, 0x84A170"),
 fn(0x8468C0, L+"::DecrementSkillRank", "skill rank-down; refunds 1 (class skill) or 2 (cross-class) points", "high", 0x846690, 0x846690, "R10 sec (b); R10_raw_dec3.txt:789; R10_out_disasm_8468C0.txt", "sole caller 0x90EB8E (skills panel); refund via 0x846350"),
 fn(0x846350, L+"::GetSkillPointCost", "returns 1 for class skill, 2 cross-class (2 - classSkill)", "high", 0x846F00, 0x8468C0, "R10 sec (b); R10_raw_dec4.txt:1385", "callers 0x90DBEE, 0x90F064, 0x90F89F (all skills panel)"),
 fn(0x846690, L+"::IncrementSkillRank", "skill rank-up on the stats object: cap (level+3, halved cross-class), +1 rank via 0x845C10, deducts 1 or 2 points, returns bool", "medium", 0x8468C0, 0x846350, "R10 sec (b)", "called from the 0x90EDE0 rank-up handler (call 0x90F160); sets rank via 0x845C10, point deduction 1 or 2"),
 fn(0x90EDE0, S+"::OnRankUpButton", "skills rank-up button handler: cost via 0x846350, rank change via 0x846690", "high", 0x90EAC0, 0x90F610, "R10 sec (b); R10_raw_dec4.txt:835", "calls 0x846350 (cost) and 0x846690 (rank-up, call 0x90F160); g0t0 tag string"),
 fn(0x90F610, S+"::RefreshRowStyling", "skills row dimming: disabled = !(skillRow+0x1C bit4), colour DAT_00A7FD4C vs DAT_00A7FDAC", "high", 0x90FBD0, 0x90EDE0, "R10 sec (b); R10_raw_dec3.txt:1047", "colour constants DAT_00A7FD4C / DAT_00A7FDAC"),
 fn(0x90B200, F+"::OnInputEvent", "vtable slot 15 event switch: 0x27/0x2D accept, 0x28/0x2E back, 0x29 click, 0x2A recommended, arrows", "high", 0x90B360, 0x908920, "R10 sec 0; R10_raw_dec2.txt:437", "case table calls 0x90B430 (accept), 0x90B890, 0x90B6B0, 0x89C350"),
 fn(0x908920, P+"::OnInputEvent", "vtable slot 15 event switch for powers panel", "high", 0x908B50, 0x90B200, "R10 sec 0; R10_out_vtables.txt", "vtable 0x9AA34C slot 15; case table incl. accept 0x908B50"),
 fn(0x90E850, S+"::OnInputEvent", "vtable slot 15 event switch for skills panel (R10 text says 0x90E860, vtable says 0x90E850)", "medium", 0x90E810, 0x90B200, "R10 sec 0 + R10_out_vtables.txt slot 15", "vtable 0x9AA714 slot 15; thiscall ret 8"),
 fn(0x90B6B0, F+"::OnRecommendedButton", "feats Recommended button: AddDefaultPackageFeats path (ignores grid filter)", "high", 0x90B360, 0x90B890, "R10 sec (c)/(b); R10_raw_dec2.txt:513", "callees 0x84A640, 0x844E40, AddDefaultPackageFeats 0x8489F0, 0x90C120; called from event case 0x2A of 0x90B200"),
 fn(0x90B890, F+"::OnSlotClicked", "feats slot click: refusal popups strref 0xA4C8 (state 4) / 0xA4C7 / 0xA4C6 (state 2/3)", "high", 0x90B9B0, 0x90B6B0, "R10 sec (c); R10_raw_dec2.txt:574", "popup strrefs 0xA4C8/0xA4C7/0xA4C6; calls 0x90BF90, 0x90BD50, 0x90C120"),
]

# vtable cards: (ctor addr, vftable addr, class name, wrong-vftable decoy)
VTABLES = [
 (0x909E00, 0x9AA434, F, 0x9AA34C),
 (0x9074E0, 0x9AA34C, P, 0x9AA714),
 (0x90C6A0, 0x9AA714, S, 0x9AA1AC),
 (0x902F60, 0x9AA1AC, "CSWGuiLevelUpPanel", 0x9AA434),
]

# hook sites (hooks_ported rows).  exit_bad = wrong exit used by the decoy.
SITES = [
 dict(id="F1_FeatGridRowSkip", addr=0x89B6E4, parent=0x89B600, bytes="89 8d 3c ff ff ff", exit=0x89BE96, exit_bad=0x89BE91,
      role="loop body of feat grid builder right after lea ecx,[eax+edx*2]: ecx = &featId (ushort) of the current candidate; EAX and flags untouched",
      exit_role="loop increment (mov edx,[ebp-0x28]); nothing pushed yet"),
 dict(id="F2_FeatChoosableSkip", addr=0x90C2BE, parent=0x90C230, bytes="0f b7 55 e4 52", exit=0x90C365, exit_bad=0x90C349,
      role="choosable-feat loop before the 4 vector probes: id = word [ebp-0x1C], panel [ebp-0x28], S [ebp-0x10]",
      exit_role="jmp back to loop head 0x90C27F; nothing pushed yet"),
 dict(id="P1_PowerGridRowSkip", addr=0x89AB29, parent=0x89AAA0, bytes="89 4d b8 8b 15 d0 b4 a1 00", exit=0x89AEB8, exit_bad=0x89AEB3,
      role="power grid builder loop after mov ecx,[eax]: ecx = spellId (uint); window EAX-free",
      exit_role="loop increment (mov eax,[ebp-0x10])"),
 dict(id="P2_PowerChoosableSkip", addr=0x9099A2, parent=0x9098D0, bytes="0f b7 4d e4 51", exit=0x909B51, exit_bad=0x9099DB,
      role="choosable-power loop after the 4 vector probes, before the 0x848660 args are pushed: id = word [ebp-0x1C], panel [ebp-0x68]",
      exit_role="jmp back to loop head (e9 c8 fd ff ff region); nothing pushed yet"),
]

# call-site facts (verified as cards; appended to the parent function's evidence if they pass)
CALLS = [
 dict(addr=0x90C349, parent=0x90C230, bytes="e8 62 bd f3 ff", callee=0x8480B0, callee_bad=0x848660,
      role="CanChooseFeat(S, feat, +0x1BDC, +0x1BDD, &selected) with ecx=S=[ebp-0x10], 4 stack args, callee-cleaned RET 0x10; true result pushes into +0x1BC0"),
 dict(addr=0x9099DB, parent=0x9098D0, bytes="e8 80 ec f3 ff", callee=0x848660, callee_bad=0x8480B0,
      role="0x848660(S, spell, +0x1BA9, &selected +0x1BE0): 3 stack args pushed at 0x9099BE-0x9099D7"),
]

# known existing-DB conflicts / skips, reported by the insert script, never written
NOT_NOMINATED = {
 0x77D800: "EXISTS in DB (name_map 'Party entry -> creature ...', functions 'ClientParty::GetCurrentCreature'); R10 'client creature -> server creature' agrees on ecx=client creature (0x7F2540 then vt+0x30) but the name differs: Opus decision, no insert",
 0x90E860: "R10 typo: not a function start in the dump; skills slot-15 handler is 0x90E850 (vtable 0x9AA714), nominated as 0x90E850",
 0x90AD90: "slot 18 'open-confirm helper' is R10 'I' (inferred), name too vague to verify", 
 0x908460: "slot 18 helper, R10 'I'",
 0x90E810: "slot 18 helper, R10 'I'",
 0x90BF90: "named only as a caller of 0x90C230 (select/deselect direction unknown)",
 0x90C120: "named only as a caller of 0x90C230",
}

# names of already-DB'd functions usable as wrong-name decoys (never inserted)
EXTRA_NAMES = {
 0x8480B0: ("CSWCLevelUpStats::CanChooseFeat", "feat availability test: not 0xCF/0xEE, not known, not selected, prereq 0x846B20, slot accounting"),
}
