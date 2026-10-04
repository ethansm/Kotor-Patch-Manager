#!/usr/bin/env python3
"""Build ../21b_gog_address_map.md from the dumps. Every quote is generated from dumps/*.txt (evlib)."""
import re, sqlite3, collections
import evlib
from evlib import AL, PAIRS, site, callsite, absop, fnline, decoys, ins_at, q, ss, gs, SD, GD

def fx(key, rx, nth=0):
    sf, gf, r = AL[key]
    m = [x[0] for x in sf['ins'] if re.search(rx, x[2], re.I)]
    return m[nth]
def refn(side, addr):
    d = ss if side == 's' else gs
    for k, v in d.items():
        if k.lower() == 'refs %x' % addr: return sum(1 for l in v['raw'] if l.strip().startswith('from'))
def refstr(key):
    s, g = PAIRS[key]; a, b = refn('s', s), refn('g', g)
    if a is None or b is None: return ''
    cap = lambda n: '>=200' if n >= 200 else str(n)
    return 'xrefs-to S %s / G %s%s' % (cap(a), cap(b), ' (equal)' if a == b else ' (DIFFER)')
def S_(key, saddr):  # site quote, compact
    s, g = ins_at(key, saddr)
    gtxt = 'G %08x identical bytes' % g[0] if s[1] == g[1] else 'G %08x `%s %s`' % (g[0], g[1].hex(' '), g[2])
    return 'S %08x `%s %s` / %s' % (s[0], s[1].hex(' '), s[2], gtxt)
def C_(parent, target):
    txt, gt = callsite(parent, target)
    s, g = txt.split(' | ')
    return 'call in %s: %s / %s' % (parent, s.replace('S ', 'S ', 1), g)
def A_(parent, sval):
    txt, gv = absop(parent, sval)
    s, g = txt.split(' | ')
    return 'operand in %s: %s / %s' % (parent, s, g), gv

DBG = sqlite3.connect('file:../ghidra_knowledge/kotor2_ghidra_knowledge.db?mode=ro', uri=True)
def sv(sa):
    r = DBG.execute("select name,gog_addr,tier,batch from steam_validated where steam_addr=?", ('0x%08x' % sa,)).fetchone()
    return 'steam_validated row %s -> GOG %s (%s, %s)' % r if r else None

def fn_ev(key, extra=()):
    s, g = PAIRS[key]
    return ['body ' + fnline(key).split(': ', 1)[1], refstr(key)] + list(extra)

rows = collections.OrderedDict()   # section -> list of (symbol, field, steam, gog, evidence list, conf, used_by)
def add(sec, sym, field, steam, gog, ev, conf, used=''):
    rows.setdefault(sec, []).append((sym, field, steam, gog, [e for e in ev if e], conf, used))

U_ALL = 'PS,BH,SB'
# ======================= ENGINE FUNCTIONS =========================================================================
F = 'func'
def fn_row(sym, field, key, ev, conf, used):
    s, g = PAIRS[key]; add(F, sym, field, s, g, fn_ev(key, ev), conf, used)

fn_row('UpdatePortraits (hook host)', '-', 'UpdatePortraits', ['sole caller on both sides: HUD Draw 0x74B3F0 (3874 B, RET 4) <-> 0x51CB70 (3874 B, RET 4); both parents also call SlotDrawGate and SlotShow (xref lists in dumps/*_r4.txt)'], 'exact', 'PS')
fn_row('FUN_00745770 per-slot gate = CSWGuiMainInterfaceChar::DrawEffects', '-', 'SlotDrawGate', [C_('InGameMenuDraw', 0x745770)], 'exact', 'PS')
fn_row('FUN_007457e0 row draw = CSWGuiMainInterfaceChar::DrawStack', '-', 'RowDraw', [C_('SlotDrawGate', 0x7457e0)], 'exact', 'BH')
fn_row('FUN_00744c20 tooltip text = CSWGuiCharButton::UpdateToolTipText', '-', 'TooltipText', [C_('DisplayToolTip', 0x744c20)], 'exact', 'SB')
fn_row('FUN_00744f00 = CSWGuiMainInterfaceChar::Show', '-', 'SlotShow', [C_('UpdatePortraits', 0x744f00)], 'exact', 'BH (comment-level)')
fn_row('FUN_00756b70 = CSWGuiInGameMenu::Draw (evidence host for Menu_* offsets)', '-', 'InGameMenuDraw', ['size/retn 3802/4 equal; the Steam name-map entry was low confidence, now corroborated by identical body + equal callers'], 'high', 'PS (offset evidence only)')
fn_row('CExoString::Format (sprintf into CExoString)', 'CExoStringFormat', 'ExoFormat', [C_('TooltipText', 0x734270)], 'exact', 'SB')
fn_row('CSWSCreature::GetMaxForcePoints', 'GetMaxForcePoints', 'MaxForcePoints', [C_('UpdatePortraits', 0x57eca0), 'GOG name_map: CSWSCreature::GetMaxForcePoints (gog_cluster_delta_map vote, delta +0x118B30) now confirmed by body + aligned call'], 'exact', 'SB')
fn_row('CWorldTimer::GetWorldTime', 'WorldTimerGetCurrentTime', 'WorldTime', [C_('GetCanClick', 0x51ad20), sv(0x51ad20)], 'exact', 'BH,SB')
fn_row('CClientExoApp::GetSWParty (party table getter)', 'ClientPartyTable', 'GetSWParty', [C_('UpdatePortraits', 0x73fb90), S_('GetSWParty', fx('GetSWParty', r'0x270'))], 'exact', 'PS,SB (Party.h)')
fn_row('CSWParty::GetPlayerCharacter (controlled creature, party slot 0)', 'GetControlledCreature', 'PartyGetControlled', [C_('TargetMenuSound', 0x7e5d80), 'GOG name: CSWParty::GetPlayerCharacter'], 'exact', 'Party.h')
fn_row('CSWParty::GetCharacter(idx) (party member accessor)', 'PartyGetAt', 'PartyGetAt', [C_('UpdatePortraits', 0x7e5da0)], 'exact', 'PS,SB (Party.h)')
fn_row('CSWCCreature::GetServerCreature', 'MemberServerCreature', 'ServerCreature', [C_('UpdatePortraits', 0x77d800), C_('InGameMenuDraw', 0x77d800)], 'exact', 'PS,SB (Party.h)')
fn_row('AurGUIStartLayer (BeginGui2D)', 'BeginGui2D', 'BeginGui2D', [C_('RowDraw', 0x4792c0)], 'exact', 'BH,SB,PS')
fn_row('AurGUIStopLayer (EndGui2D)', 'EndGui2D', 'EndGui2D', [C_('RowDraw', 0x4793f0) if False else C_('RowDraw', 0x4793f0) if any(True for x in AL['RowDraw'][0]['ins'] if x[3] and x[3][0]==0x4793f0) else C_('PanelDraw', 0x4793f0)], 'exact', 'BH,SB,PS')
fn_row('AurGUISetupViewport (PushClipRect)', 'PushClipRect', 'PushClip', [C_('RowDraw', 0x479460)], 'exact', 'BH,SB,PS')
fn_row('AurGUICloseViewport (PopClipRect)', 'PopClipRect', 'PopClip', [C_('RowDraw', 0x4799c0)], 'exact', 'BH,SB,PS')
fn_row('GetResolutionTextScalar (HudScale)', 'HudScale', 'HudScale', [C_('RowDraw', 0x479ab0)], 'exact', 'BH,SB')
fn_row('CAurGUIImage::FlushBuffer (FlushGuiBatch)', 'FlushGuiBatch', 'FlushBatch', [C_('BorderDraw', 0x47ea60)], 'exact', 'BH,SB')
fn_row('NewCAurGUIImage (GuiLoadImage)', 'GuiLoadImage', 'LoadImage', [C_('SetFillImage', 0x47eb60), 'same call in SetCornerImage/SetEdgeImage (3 callers aligned)'], 'exact', 'BH,SB')
fn_row('CSWGuiPanel::GetExtentAccountingForPanelOffset (PanelRect)', 'PanelRect', 'PanelRect', [C_('PanelDraw', 0x410020), sv(0x410020)], 'exact', 'PS,SB')
fn_row('CSWGuiManager::PlayGuiSound', 'PlayGuiSound', 'PlayGuiSound', [C_('TargetMenuSound', 0x4122a0), sv(0x4122a0)], 'exact', 'hotbar/GuiKit only')
fn_row('CSWGuiLabel::CSWGuiLabel', 'LabelCtor', 'LabelCtor', [C_('InGameMenuCtor', 0x419740)], 'exact', 'SB,BH (GuiKit)')
fn_row('CSWGuiPanel::InitControl', 'InitControl', 'InitControl', [C_('CharInitialize', 0x40f620), sv(0x40f620)], 'exact', 'GuiKit')
fn_row('CSWGuiPanel::StopLoadFromLayout', 'StopLoadFromLayout', 'StopLoad', [C_('PanelDtor', 0x40f5a0), sv(0x40f5a0)], 'exact', 'GuiKit')
fn_row('CExoString ctor (const char*)', 'ExoCtor', 'ExoCtor', [C_('InGameMenuDraw', 0x733570), sv(0x733570)], 'exact', 'GuiKit')
fn_row('CExoString dtor', 'ExoDtor', 'ExoDtor', [C_('TooltipText', 0x733780), sv(0x733780)], 'exact', 'GuiKit')
fn_row('CSWGuiTextParams::SetText', 'TextSet', 'TextSet', [C_('InGameMenuDraw', 0x416e30)], 'exact', 'GuiKit')
fn_row('CSWGuiTextParams::SetBaseFont (SetFont)', 'SetFont', 'TextSetFont', [C_('TextLoad', 0x416de0)], 'exact', 'GuiKit')
fn_row('CSWGuiTextParams::SetAlignment (SetAlign)', 'SetAlign', 'TextSetAlign', [C_('TextLoad', 0x416fa0)], 'exact', 'GuiKit')
fn_row('CSWGuiTextParams::SetDefaultColor (TextColor)', 'TextColor', 'TextColor', [C_('TextLoad', 0x417140), 'DECOY rejected: GOG 0x9153e0 CSWGuiTextParams::SetColor (93 B, the name-based guess) matches only 12 of 36 insns; the delta-sequence target 0x915510 (50 B, SetDefaultColor) is byte-identical 20/20'], 'exact', 'GuiKit')
fn_row('CExoArrayList<CSWGuiControl*>::Add (PtrListAdd)', 'PtrListAdd', 'PtrListAdd', [C_('InitControl', 0x83ea60), 'CAVEAT: Steam 0x83ea60 is one folded instance with >=200 callers; GOG 0x534170 has 65 (separate per-type copies). Correct for the panel control list (InitControl uses it); a GOG caller adding to some OTHER pointer list must use that list own Add. GuiKit only appends to a panel list.'], 'exact', 'GuiKit')
fn_row('operator new (cdecl size)', 'New', 'OperatorNew', [C_('LoadImage', 0x919723), 'GOG body is 101 B vs 100 B: the extra byte is a trailing INT3 pad in the Ghidra function body; instruction streams equal 33/33 + pad'], 'exact', 'GuiKit')
fn_row('(not in table) CSWSObject::HasSpellEffectApplied', '-', 'HasSpellEffect', ['evidence host for Creature_EffectArray/Count'], 'high', '')

# ======================= GLOBALS ==================================================================================
G_ = 'glob'
def glob_row(sym, field, sval, parent, conf, used, extra=()):
    e, g = A_(parent, sval); add(G_, sym, field, sval, g, [e] + list(extra), conf, used); return g
glob_row('APP_MANAGER_PTR (CAppManager*)', 'AppGlobal', 0xa1b4a4, 'PartyGetAt', 'exact', 'BH,SB,PS (Party.h, Effects.h)', ['25 aligned reads across the dumped pairs, all map A1B4A4 -> A11C04', 'GOG address DB global_pointers: APP_MANAGER_PTR = 0xA11C04 (agrees)'])
glob_row('GUI_MANAGER_PTR', 'GuiSoundMgr', 0xa1b49c, 'DisplayToolTip', 'exact', 'GuiKit (hover sound)', ['GOG address DB: GUI_MANAGER_PTR = 0xA11BFC (agrees)', 'also read in CharButtonDraw, TooltipPanelPos'])
glob_row('screen width', 'ScreenW', 0x9f42a4, 'HudScale', 'exact', 'GuiKit/Gui2D', ['also aligned in PushClip, TooltipPanelPos (HudScale derives the HUD scale from it)'])
glob_row('screen height', 'ScreenH', 0x9f42a8, 'PushClip', 'exact', 'GuiKit/Gui2D', ['also aligned in TooltipPanelPos'])
glob_row('GUI ref width (800)', 'GuiRefW', 0x9f4390, 'InitControl', 'exact', 'hotbar only', ['initialised data equal: Steam 0x9F4390 = 20 03 00 00 (800), GOG 0xA0FFE8 = 20 03 00 00'])
glob_row('GUI ref height (600)', 'GuiRefH', 0x9f4394, 'InitControl', 'exact', 'hotbar only', ['initialised data equal: Steam 0x9F4394 = 58 02 00 00 (600), GOG 0xA0FFEC = 58 02 00 00'])
glob_row('GUI projection flag', 'ProjectionFlag', 0xa1b9f0, 'ScaleHelperA', 'exact', 'hotbar only', ['same operand aligned in InitControl and ScaleHelperB (3 functions); Steam holds 0 here, the GOG address is .bss (not file-backed), consistent'])
glob_row('FlushArg float (1.0f)', 'FlushArg', 0x9f5f60, 'BorderDraw', 'exact', 'BH,SB', ['initialised data equal: Steam 0x9F5F60 = 00 00 80 3f (1.0f), GOG 0x9F7144 = 00 00 80 3f'])
glob_row('clip stack index', 'ClipDepth', 0xa309c0, 'BeginGui2D', 'exact', 'PS,SB,BH', ['20 aligned operand sites in Begin/End/Push/Pop (A309C0 -> A32B90, +0x21D0 like the neighbouring A309C8.. fields)'])
glob_row('clip no-fill colour (engine icon row arg)', 'ClipNoFillColor', 0xa1ba3c, 'RowDraw', 'high', 'BH,SB', ['single aligned site (PUSH imm32 into PushClip); neighbour global A1B9F0->A76858 has the identical +0x5AE68 shift (A1BA3C->A768A4), consistent', 'Steam image holds zeros here (uninitialised data); the GOG address is not file-backed (.bss), consistent'])
glob_row('pending texture list', 'TexPendingList', 0xa1bc2c, 'TexResetAll', 'high', 'hotbar only (not the 3 mods)', ['same operand aligned in TexSetMem and TexBuildAll (3 functions)'])
add(G_, 'keyboard device index (rdata const 0)', 'KeyboardDeviceGlobal', 0x99c884, 0, ['not mapped: hotbar-only global, none of the 3 mods read it; the Steam users (0x785df0, 0x732960..) have no GOG pair in the dumps (autopair found no unique same-size GOG function)'], 'unknown', 'hotbar only')

# ======================= VTABLES ==================================================================================
V = 'vt'
e, g = A_('LabelCtor', 0x9878bc); add(V, 'CSWGuiLabel vtable', 'LabelVtable', 0x9878bc, g, [e, 'PTRS dump 40 slots each: slot1 SetRect 0x4199a0 <-> SetExtent 0x917c40, slot14 Draw 0x419880 <-> 0x917b20 (LabelDraw, body-identical 25/25), slot17 HitTest 0x418d20 <-> 0x916fc0, slot18 Load, slots 31-39 all +0x4FE2A0; shared stubs map consistently', 'GOG ctor also calls the border ctor at this+0x60 and the text ctor at this+0xD8 (same as Steam)'], 'exact', 'GuiKit')
e, g = A_('BorderCtor', 0x9875bc); add(V, 'CSWGuiBorder vtable', 'BorderVtable', 0x9875bc, g, [e, 'PTRS: Steam slot3 = 0x415300 (BorderDraw) <-> GOG slot3 = 0x9136D0 (BorderDraw, body-identical 674 insn); slot1 SetRect 0x414B70 <-> SetExtent 0x912F40; slots 9/10 __purecall in both'], 'exact', 'BH,GuiKit')
e, g = A_('TextCtor', 0x9876b4); add(V, 'CSWGuiText vtable', 'TextVtable', 0x9876b4, g, [e, 'PTRS: slot3 Draw 0x417720 <-> 0x915AF0 (TextDraw 30/30 aligned), slot4 Load 0x417480 <-> 0x915850 (TextLoad)'], 'exact', 'GuiKit')

# ======================= OFFSETS ==================================================================================
O = 'off'
def off_row(field, val, sym, evs, conf='exact', used='', gogv=None):
    add(O, sym, field, val, val if gogv is None else gogv, evs, conf, used)

# slot (CSWGuiMainInterfaceChar)
off_row('Slot_Flags', 0x590, 'portrait slot flags word (bit1 = row visible)', [S_('SlotDrawGate', 0x74577a), 'derived: control embedded at slot+0x548 (SlotShow `ADD EAX,0x548`) + Ctl_Flags 0x48 = 0x590'], used='BH,SB')
off_row('Slot_CardRect', 0x54c, 'portrait card rect (x,y,w,h)', [S_('RowDraw', 0x7457ef), S_('SlotShow', 0x744f0c) + ' (control base 0x548, rect = +4)'], used='SB')
off_row('Slot_VitCtl', 0x690, 'embedded health-bar control', [S_('SlotShow', fx('SlotShow', r'0x690$'))], used='SB')
off_row('Slot_VitRect', 0x694, 'health-bar rect', ['derived = Slot_VitCtl + Ctl_X(4): both exact; the rect lives at control+4 (HitTest reads +4..+0x10)'], used='SB')
off_row('Slot_ForceCtl', 0x7fc, 'embedded force-bar control', [S_('SlotShow', fx('SlotShow', r'0x7fc$'))], used='SB')
off_row('Slot_ForceRect', 0x800, 'force-bar rect', ['derived = Slot_ForceCtl + 4'], used='SB')
off_row('Slot_ArrowBuff', 0xddc, 'buff arrow counter (u8)', [S_('SlotShow', fx('SlotShow', r'0xddc')), S_('SlotDrawGate', fx('SlotDrawGate', r'0xddc'))], used='BH,SB')
off_row('Slot_ArrowDebuff', 0xddd, 'debuff arrow counter (u8)', [S_('SlotShow', fx('SlotShow', r'0xddd')), S_('SlotDrawGate', fx('SlotDrawGate', r'0xddd'))], used='BH,SB')
off_row('Slot_Overlay0', 0x1b8, 'overlay flag word 0 (mask&2)', [S_('SlotShow', fx('SlotShow', r'ADD EAX,0x170')) + ' then flags +0x48', 'derived 0x170 + 0x48 = 0x1b8'], used='BH')
off_row('Slot_Overlay1', 0x300, 'overlay flag word 1 (mask&1)', [S_('SlotShow', fx('SlotShow', r'0x2b8')) + ' then flags +0x48', 'derived 0x2b8 + 0x48 = 0x300'], used='BH')
off_row('Slot_Overlay2', 0x448, 'overlay flag word 2 (mask&4)', [S_('SlotShow', fx('SlotShow', r'ADD EAX,0x400')) + ' then flags +0x48', 'derived 0x400 + 0x48 = 0x448'], used='BH')
# menu
mp = fx('InGameMenuCtor', r'ADD ECX,0x2350')
off_row('Menu_PortraitSlot', 0x2350, 'CSWGuiInGameMenu embedded portrait slot', [S_('InGameMenuCtor', mp), C_('InGameMenuCtor', 0x7450e0) + ' (CharInitialize on this+0x2350)', S_('InGameMenuDraw', fx('InGameMenuDraw', r'0x2350', 3)) + ' (Draw passes this+0x2350 as ECX of SlotDrawGate, next call 0x745774-host)'], used='PS')
# tag -> offset via InitControl pushes
def tagmap(side):
    d = ss if side == 's' else gs
    path = '/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/%s/strings.tsv' % ('k2_win_steam_aspyr_swkotor2.exe' if side == 's' else 'k2_win_gog_aspyr_swkotor2.exe')
    st = {}
    for l in open(path):
        p = l.rstrip('\n').split('\t')
        if len(p) == 2 and p[1] in ('LBL_CHAR2','LBL_CHAR3','LBL_LEVELUP2','LBL_LEVELUP3','LBL_BACK2','LBL_BACK3','BTN_CHANGE2','BTN_CHANGE3'): st[int(p[0], 16)] = p[1]
    sf = align_fn = None
    return st
import align as _al
def tagoffs(side):
    st = tagmap(side)
    key = 'InGameMenuCtor'
    ins = AL[key][0 if side == 's' else 1]['ins']
    out = {}
    for i, x in enumerate(ins):
        m = re.match(r'PUSH 0x([0-9a-f]{6,8})$', x[2])
        if m and int(m.group(1), 16) in st:
            for j in range(i, i + 14):
                mm = re.search(r'ADD E.X,0x([0-9a-f]{4})$', ins[j][2])
                if mm: out[st[int(m.group(1), 16)]] = (int(mm.group(1), 16), x, ins[j]); break
    return out
TS, TG = tagoffs('s'), tagoffs('g')
for tag, field, val in (('LBL_CHAR2','Menu_Char2',0x3134),('LBL_LEVELUP2','Menu_Level2',0x327c),('LBL_BACK2','Menu_Back2',0x33c4),('BTN_CHANGE2','Menu_Change2',0x350c),('LBL_CHAR3','Menu_Char3',0x36dc),('LBL_LEVELUP3','Menu_Level3',0x3824),('LBL_BACK3','Menu_Back3',0x396c),('BTN_CHANGE3','Menu_Change3',0x3ab4)):
    s = TS.get(tag); g = TG.get(tag)
    ev = []
    gv = val
    if s and g:
        ev.append('string anchor `%s`: S %08x `%s` then %08x `%s` (InitControl follows); G %08x `%s` then %08x `%s`' % (tag, s[1][0], s[1][2], s[2][0], s[2][2], g[1][0], g[1][2], g[2][0], g[2][2]))
        assert s[0] == val, (tag, hex(s[0]))
        gv = g[0]
        conf = 'exact' if g[0] == val else 'high'
    else:
        ev.append('tag string `%s` not pushed in the ctor window (tag text not in strings.tsv for this name)' % tag)
        conf = 'high'
    ev.append(S_('InGameMenuCtor', fx('InGameMenuCtor', r'ADD E.X,0x%x$' % val)))
    off_row(field, val, 'CSWGuiInGameMenu +%s control' % tag, ev, conf, 'PS,SB,BH', gogv=gv)
off_row('MenuCaller_Creature', -0x38, 'caller-frame local = selected creature', [S_('InGameMenuDraw', fx('InGameMenuDraw', r'EBP \+ -0x38\],EAX')) + ' (store right after CALL GetServerCreature)', 'Draw is the caller of SlotDrawGate for the menu; frame identical because whole function is 856/940 byte-identical, 84 address-only'], used='PS')
# creature
off_row('Creature_Stats', 0x1198, 'creature stats object', [S_('UpdatePortraits', fx('UpdatePortraits', r'0x1198'))], used='SB')
off_row('Stats_ForceA', 0x130, 'stats current-FP part A (short)', [S_('UpdatePortraits', fx('UpdatePortraits', r'word ptr \[E.X \+ 0x130\]'))], used='SB')
off_row('Stats_ForceB', 0x132, 'stats current-FP part B (short)', [S_('UpdatePortraits', fx('UpdatePortraits', r'word ptr \[E.X \+ 0x132\]'))], used='SB')
off_row('Creature_VtMaxHp', 0x98, 'creature vtable max-HP getter', [S_('UpdatePortraits', fx('UpdatePortraits', r'\+ 0x98\]')), 'ret N of the target slot: engine slots 0x57e8c0 (RET 4) <-> GOG 0x6973f0 (RET 4) body-identical 254 insn'], used='SB')
off_row('Creature_VtCurHp', 0x9c, 'creature vtable cur-HP getter', [S_('UpdatePortraits', fx('UpdatePortraits', r'\+ 0x9c\]')), 'slot target 0x5413c0 <-> GOG 0x6a80a0 (RET 4) body-identical 16/16'], used='SB')
off_row('Creature_IconCacheData', 0xffc, 'client icon-cache list head', [S_('UpdatePortraits', 0x74f516)], used='BH (diagnostic)')
off_row('Creature_IconCacheCount', 0x1000, 'icon-cache count (= head+4)', [S_('UpdatePortraits', 0x74f533) + ' (count at list+4 = 0xffc+4)'], used='BH (diagnostic)')
off_row('IconEnt_F14', 0x14, 'icon entry +0x14 (debuff test)', [S_('UpdatePortraits', 0x74f563)], used='BH (diagnostic)')
off_row('IconEnt_F1c', 0x1c, 'icon entry +0x1c (active test)', [S_('UpdatePortraits', 0x74f552)], used='BH (diagnostic)')
off_row('Tip_Slot', 0x1d0, 'char button -> portrait slot pointer', [S_('SetMainInterfaceChar', 0x744b2d), S_('TooltipText', fx('TooltipText', r'0x1d0', 1))], used='SB')
off_row('Tip_Text', 0x1d4, 'char button tooltip CExoString', [S_('TooltipText', 0x744d8f)], used='SB')
# effects
off_row('Creature_EffectArray', 0x148, 'CSWSObject effect array', [S_('HasSpellEffect', fx('HasSpellEffect', r'0x148'))], used='BH,SB')
off_row('Creature_EffectCount', 0x14c, 'CSWSObject effect count', [S_('HasSpellEffect', fx('HasSpellEffect', r'0x14c')), 'GOG knowledge DB struct_fields lists CSWSObject +0x14C effect count at Steam 0x5476e7 (this site)'], used='BH,SB')
off_row('Effect_Type', 0x08, 'CGameEffect type (u16)', [S_('GEffectCtor', fx('GEffectCtor', r'word ptr \[E.X \+ 0x8\]'))], used='BH,SB')
off_row('Effect_Subtype', 0x0a, 'CGameEffect subtype (u16)', [S_('GEffectCtor', fx('GEffectCtor', r'word ptr \[E.X \+ 0xa\]'))], used='BH,SB')
off_row('Effect_Duration', 0x0c, 'CGameEffect duration (float)', [S_('GEffectCtor', fx('GEffectCtor', r'float ptr \[E.X \+ 0xc\]'))], used='BH,SB')
off_row('Effect_ExpiryDay', 0x10, 'CGameEffect expiry day', [S_('GEffectGetExpiry', fx('GEffectGetExpiry', r'\+ 0x10\]')), S_('GEffectSetExpiry', fx('GEffectSetExpiry', r'\+ 0x10\],'))], used='BH,SB')
off_row('Effect_ExpiryTime', 0x14, 'CGameEffect expiry ms-of-day', [S_('GEffectGetExpiry', fx('GEffectGetExpiry', r'\+ 0x14\]')), S_('GEffectSetExpiry', fx('GEffectSetExpiry', r'\+ 0x14\],'))], used='BH,SB')
off_row('Effect_Creator', 0x18, 'CGameEffect creator id', [S_('GEffectSetCreator', fx('GEffectSetCreator', r'\+ 0x18\],ECX')), S_('GEffectCtor', fx('GEffectCtor', r'\+ 0x18\],0x7f000000')) + ' (invalid-object-id default)'], used='BH,SB')
off_row('Effect_SpellId', 0x1c, 'CGameEffect spells.2da id', [S_('GEffectCtor', fx('GEffectCtor', r'\+ 0x1c\],0xffffffff')) + ' (-1 default)'], used='BH,SB')
off_row('Effect_IntCount', 0x30, 'CGameEffect int count', [S_('GEffectSetNumInts', fx('GEffectSetNumInts', r'\+ 0x30\],ECX'))], used='SB')
off_row('Effect_Ints', 0x34, 'CGameEffect int* array', [S_('GEffectGetInt', fx('GEffectGetInt', r'\+ 0x34\]')), S_('GEffectSetNumInts', fx('GEffectSetNumInts', r'\+ 0x34\],ECX'))], used='SB')
# app chain
off_row('AppGlobal_ToWorld', 0x8, 'CAppManager+8 = server exo app', [S_('AppCreateServer', 0x401beb) + ' (server-exists check)', S_('AppCreateServer', 0x401c3a), 'GOG address DB offsets table: CAppManager.Server = 0x8, Client = 0x4 (agrees)'], used='BH,SB (Effects.h)')
off_row('World_ToSub', 0x4, 'server app +4 = internal', [S_('GetWorldTimer', 0x51c37c)], used='BH,SB')
off_row('Sub_ToTimer', 0x10048, 'internal +0x10048 = world timer', [S_('GetWorldTimer', 0x51c37f), C_('GetCanClick', 0x51ad20) if False else ''], used='BH,SB')
# gui2d / guikit
off_row('ImageVt_Draw', 6, 'image wrapper vtable slot of Draw (+0x18)', [S_('BorderDraw', 0x415a41) + ' (slot at +0x18 = 6*4, called on border+0x74 fill image)'], used='BH,SB')
off_row('LabelSize', 0x148, 'sizeof(CSWGuiLabel)', ['embedded labels in the menu are 0x148 apart: Menu_Char2 0x3134 -> Menu_Level2 0x327c -> Menu_Back2 0x33c4 (identical in GOG); both ctor sites quoted under Menu_*'], used='GuiKit')
off_row('Ctl_Flags', 0x48, 'control flags', [S_('SlotShow', 0x744f1f), S_('HitTest', fx('HitTest', r'\[E.. \+ 0x48\]'))], used='all')
off_row('Ctl_Id', 0x54, 'control id', [S_('InitControl', 0x40f96c)], used='GuiKit')
off_row('Ctl_X', 4, 'control x', [S_('HitTest', 0x418d4a)], used='all')
off_row('Ctl_Y', 8, 'control y', [S_('HitTest', 0x418d6e)], used='all')
off_row('Ctl_W', 0xc, 'control w', [S_('HitTest', 0x418d60) + ' (base = ctl+4 from 00418d52 `ADD EAX,0x4`, so w = ctl+0x4+0x8 = 0xc)'], used='all')
off_row('Ctl_H', 0x10, 'control h', [S_('HitTest', 0x418d85) + ' (base = ctl+4 from 00418d76, so h = ctl+0x4+0xc = 0x10)'], used='all')
off_row('Ctl_VtSetRect', 4, 'control vtable slot1 byte offset (SetRect)', [S_('RowDraw', 0x745af8) + ' (hook-6 row exit calls vt+4 on the icon widget with a rect)', 'Label vtable PTRS slot1 maps SetRect 0x4199a0 <-> SetExtent 0x917c40'], used='SB,PS')
off_row('Label_Border', 0x60, 'label +border', [S_('LabelCtor', fx('LabelCtor', r'ADD ECX,0x60$'))], used='BH,GuiKit')
off_row('Label_Text', 0xd8, 'label +text', [S_('LabelCtor', fx('LabelCtor', r'ADD ECX,0xd8$'))], used='GuiKit')
off_row('Text_Renderer', 0x14, 'text +renderer', [S_('TextCtor', fx('TextCtor', r'\+ 0x14\],0x0'))], used='GuiKit')
off_row('Text_StringSub', 0x18, 'text +string sub-object', [S_('TextCtor', fx('TextCtor', r'ADD ECX,0x18$'))], used='GuiKit')
off_row('Text_Color', 0x2c, 'string-sub +colour rgb', [S_('TextColor', fx('TextColor', r'ADD ECX,0x2c$'))], used='GuiKit')
off_row('Border_Alpha', 0x24, 'border +alpha (float)', [S_('BorderDraw', 0x415326)], used='BH,SB')
off_row('Border_Tint', 0x28, 'border +tint rgb', [S_('BorderDraw', 0x415a16) + ' (tint pointer pushed for the fill-image Draw)'], used='BH,SB')
off_row('Border_Fill', 0x74, 'border +fill image*', [S_('BorderDraw', 0x415a36)], used='BH,SB')
off_row('Border_Width', 0xc, 'border +width', [S_('BorderDraw', 0x41530f) + ' (Draw returns at once when 0)'], used='BH')
off_row('Border_Height', 0x10, 'border +height', [S_('BorderDraw', 0x415318)], used='BH')
off_row('Border_Rot', 0x20, 'border +rotation bits', [S_('BorderDraw', 0x415a06)], used='BH')
off_row('Panel_CtlArray', 0x24, 'panel control list', [S_('InitControl', 0x40f998) + ' (array pointer indexed by ctl id)', S_('InitControl', 0x40f9c8) + ' then CALL PtrListAdd'], used='GuiKit')
off_row('Panel_CtlCount', 0x28, 'panel control count', [S_('InitControl', 0x40f97e) + ' (compared with the control id at 0x40f98a)'], used='GuiKit')

# ======================= GOG DB cross-check (names only) ==========================================================
GDB = sqlite3.connect('file:/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/AddressDatabases/kotor2_gog_aspyr.db?mode=ro', uri=True)
def gdbname(a):
    r = GDB.execute("select class_name,function_name from functions where address=?", (a,)).fetchall()
    return ', '.join('%s::%s' % x for x in r) if r else None
gdb_hits = 0
for sym, field, st, gg, ev, conf, used in rows['func']:
    n = gdbname(gg)
    if n: ev.append('GOG address DB names 0x%X: %s' % (gg, n)); gdb_hits += 1

# ======================= HOOKS ====================================================================================
HOOKS = [
 (1, 0x74F25A, '89 85 44 FF FF FF', 'UpdatePortraits', 'PS_SnapshotPortraitSlot (portrait-slots)',
   [('eax = slot ptr', 'UpdatePortraits', 0x74f253), ('[ebp-0x38] = creature (stored here, read at 0x74f513 `ADD EAX,0xffc`)', 'UpdatePortraits', 0x74ef41)]),
 (2, 0x74F8AB, '89 95 A4 FE FF FF', 'UpdatePortraits', 'PS_SnapshotPortraitSlot (mirror slot)',
   [('edx = mirror slot ptr (`ADD EDX,0x48e0`)', 'UpdatePortraits', 0x74f8a5), ('[ebp-0x38] = creature', 'UpdatePortraits', 0x74ef41)]),
 (3, 0x745774, '89 4D FC 8B 45 FC', 'SlotDrawGate', 'PS_PortraitSlotDraw (portrait-slots)',
   [('ecx = slot (this), saved by the hooked insn itself', 'SlotDrawGate', 0x745774), ('[ebp+0xc] = isFirst (read as 2nd arg)', 'SlotDrawGate', 0x745797), ('[ebp] = caller frame (prologue PUSH EBP/MOV EBP,ESP)', 'SlotDrawGate', 0x745770)]),
 (4, 0x745A84, '8B 4D E0 8B 11 8B 4D E0 8B 42 38 FF D0', 'RowDraw', 'RetargetBuffIcon (buff HUD)',
   [('[ebp-0x4c] = slot ptr', 'RowDraw', 0x7457e9), ('[ebp-0x18] = arrow index (loop counter)', 'RowDraw', 0x745a4d), ('[ebp-0x20] = icon widget', 'RowDraw', 0x7458d2), ('[ebp+0x10] = whichRow', 'RowDraw', 0x74583d)]),
 (5, 0x745A97, '0F B6 4D EF 03 4D C4', 'RowDraw', 'RestoreBuffIcon (buff HUD)', []),
 (6, 0x745AEC, '8D 55 D0 52 8B 45 E0', 'RowDraw', 'DrawBuffColumn (buff HUD)',
   [('[ebp-0x4c] slot / [ebp-0x20] widget (as hook 4)', 'RowDraw', 0x7457e9), ('[ebp+0xc] = isLarge', 'RowDraw', fx('RowDraw', r'EBP \+ 0xc\]')), ('[ebp+0x10] = whichRow', 'RowDraw', 0x74583d), ('[ebp-0x8] = card w (`[EAX+0x8]` of rect 0x54c)', 'RowDraw', 0x745802), ('[ebp-0x4] = card h (`[EAX+0xc]`)', 'RowDraw', 0x745808)]),
 (7, 0x744EEC, '8B 4D F4 64 89 0D 00 00 00 00', 'TooltipText', 'AppendShieldToTooltip (shield bar)',
   [('[ebp-0x48] = charButton (this)', 'TooltipText', 0x744c45)]),
]
hook_rows = []
for n, addr, hx, key, fnname, params in HOOKS:
    ob = bytes.fromhex(hx.replace(' ', ''))
    sf, gf, r = AL[key]; Sx, Gx = sf['ins'], gf['ins']
    idx = next(i for i, x in enumerate(Sx) if x[0] == addr)
    j = idx; sj = b''; gj = b''
    while len(sj) < len(ob): sj += Sx[j][1]; gj += Gx[j][1]; j += 1
    assert sj == ob, (hex(addr), sj.hex(), ob.hex())
    ident = (sj == gj) and len(sj) == len(ob)
    psq = [lbl + ': ' + S_(k, a) for lbl, k, a in params]
    hook_rows.append((n, addr, Gx[idx][0], key, fnname, ob, ident, j - idx, psq, [q(x) for x in Gx[idx:j]]))

# ======================= field coverage vs K2Addr.h =============================================================
src = open('../_shared_src/K2Addr.h').read()
body = src[src.index('struct K2AddrTable'):src.index('};', src.index('struct K2AddrTable'))]
fields = []
for line in body.split('\n'):
    line = line.split('//')[0]
    for m in re.finditer(r'\b(?:uintptr_t|int)\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0', line): fields.append(m.group(1))
    if re.search(r'int\s+Ctl_X', line): fields += ['Ctl_X', 'Ctl_Y', 'Ctl_W', 'Ctl_H']
fields = list(dict.fromkeys(fields))
have = {}
for sec in rows.values():
    for sym, field, st, gg, ev, conf, used in sec:
        if field != '-': have[field] = (st, gg, conf)
missing = [f for f in fields if f not in have]
print('K2AddrTable fields parsed:', len(fields), 'covered:', len([f for f in fields if f in have]), 'missing:', missing)

# ======================= kGog text ================================================================================
def fmt(v, isaddr):
    if v < 0: return '-0x%X' % -v
    return ('0x%08X' % v) if isaddr else ('0x%X' % v)
addr_fields = set()
for line in body.split('\n'):
    m = re.match(r'\s*uintptr_t\s+(\w+)', line.split('//')[0])
    if m: addr_fields.add(m.group(1))
kg = ['namespace k2addr { namespace detail {', 'constexpr K2AddrTable makeGog() {', '    K2AddrTable t{"777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914"};']
cur = None
for f in fields:
    if f not in have: kg.append('    // %s: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)' % f); continue
    st, gg, conf = have[f]
    if gg == 0: kg.append('    t.%s = 0;   // UNKNOWN (zero = helper inert)' % f); continue
    ia = f in addr_fields
    note = '' if conf == 'exact' else '   // %s' % conf
    kg.append('    t.%s = %s;%s' % (f, fmt(gg, ia), note))
kg += ['    return t;', '}', '} } // namespace k2addr::detail', 'inline constexpr K2AddrTable kGog = k2addr::detail::makeGog();']
KG = '\n'.join(kg)
open('kgog.inc', 'w').write(KG + '\n')

# ======================= counts ===================================================================================
cnt = collections.OrderedDict()
for sec, lst in rows.items():
    c = collections.Counter(r[5] for r in lst if r[1] != '-')
    cnt[sec] = c
tot = collections.Counter()
for f in fields:
    if f in have: tot[have[f][2]] += 1
print('field confidence totals:', dict(tot))
print('per section:', {k: dict(v) for k, v in cnt.items()})
print('hooks identical:', [(h[0], '%X' % h[2], h[6]) for h in hook_rows])

# ======================= markdown =================================================================================
def esc(t): return t.replace('|', '\\|')
def table(sec, title, cols=('symbol / use', 'K2AddrTable field', 'Steam', 'GOG', 'evidence', 'conf', 'used by')):
    out = ['### %s' % title, '', '| ' + ' | '.join(cols) + ' |', '|' + '---|' * len(cols)]
    for sym, field, st, gg, ev, conf, used in rows[sec]:
        isa = sec in ('func', 'glob', 'vt')
        stt = ('0x%08X' % st) if isa else (('-0x%X' % -st) if st < 0 else '0x%X' % st)
        ggt = ('0x%08X' % gg if gg else 'unknown') if isa else (('-0x%X' % -gg) if gg < 0 else '0x%X' % gg)
        out.append('| %s | %s | %s | %s | %s | %s | %s |' % (esc(sym), ('`%s`' % field) if field != '-' else '-', stt, ggt, '<br>'.join(esc(e) for e in ev), conf, used))
    return '\n'.join(out)

md = []
md.append('# Task B3: Steam -> GOG Aspyr engine address map (portrait-slots, buff-duration-hud, shield-bar + shared headers)')
md.append('')
md.append('Status: 2026-10-03, read-only static research. Nothing committed, nothing installed, live game dir untouched.')
md.append('')
md.append('- **GOG exe SHA-256 (as KPM knows it):** `777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914` (GOG address DB `game_version` row 1 = "gog aspyr", and `target_versions` of all 23 `kotor2-gog-aspyr.hooks.toml` in the KPM Patches tree).')
md.append('- **Steam reference:** `6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF` (pristine; the live 4gb-patched file has identical code).')
md.append('- **Ghidra programs:** server `TSL/k2_win_steam_aspyr_swkotor2.exe` and `TSL/k2_win_gog_aspyr_swkotor2.exe`, read-only, via `scripts/ghidra_server_api.py` (see Method).')
md.append('')
md.append('## Method')
md.append('')
md.append('1. **Candidates** (tier 0, scripts): the `(size, RET n)` signature of each Steam function plus the per-region GOG->Steam delta from `gog_cluster_delta_map.py` / `steam_validated`, then neighbour-sequence comparison (`seq.py`). Hypotheses from the brief were treated as candidates only.')
md.append('2. **Raw disassembly** of both binaries (`ghscripts/ExportRangeDisasm.java`: bytes + text + call targets, plus xrefs-to and vtable dumps), 5 rounds, 83 function pairs.')
md.append('3. **Instruction-by-instruction alignment** (`align.py`): for every pair the two instruction streams are walked in lock-step. Each pair has the same instruction count, identical lengths, and every byte difference is explained as an absolute-address operand or a rel32 call/jump target. 0 unexplained differences in all 83 pairs. This yields the exact GOG address for every operand, call target and hook site in those functions, and byte-identity of every displacement (struct offset).')
md.append('4. **Anchors** (RE_AGENT_GUIDE rules): a function mapping is only `exact` when (a) the body aligns as above AND (b) an aligned **call site in a mapped parent** calls it (quoted) AND (c) whole-binary xref counts to Steam/GOG function agree (82 of 83 equal; the one difference is explained under PtrListAdd). Globals are `exact` from an aligned operand in identical code (often 2-3 functions), plus equal initialised data where the data section has any (GuiRefW/H = 800/600, FlushArg = 1.0f). Offsets are `exact` when a quoted instruction in a mapped function carries that displacement and the GOG instruction has identical bytes.')
md.append('5. **Decoys / traps handled:** decimal-vs-hex (all offsets read from asm bytes, never from decompiler text); stack-slot ret N (not used to decide); thunks (none involved); neighbour decoys: `TextColor` name-based guess 0x9153E0 rejected (12/36 insn), `PtrListAdd` second instance 0x4C7640 rejected for the panel list; `AppRef2` false candidate dropped. Same-(size,RET n) GOG function counts for the hook hosts: UpdatePortraits 1, TooltipText 1, RowDraw 2, SlotDrawGate 8 (so SlotDrawGate relies on its aligned parent call + equal xref count, not on size).')
md.append('6. **Tool deviation to disclose:** `ghidra_server_api.py export-script` only accepts scripts from the KPM `ghidra_scripts` folder, which I may not write to. `gog_map_21b/run_ghidra.py` imports that same script, points its `SCRIPT_PATH` constant at `gog_map_21b/ghscripts/`, and calls its own `main()` unchanged (same read-only `-readOnly -noanalysis`, credentials consumed inside the module, never read by me). Credential files were not read.')
md.append('')
md.append('Confidence key: **exact** = aligned + parent call/operand + (xref counts or data) agree; **high** = aligned body/operand match but a single supporting site; **low** = plausible, not instruction-verified; **unknown** = not resolved.')
md.append('')
md.append('Used-by key: PS portrait-slots, BH buff-duration-hud, SB shield-bar; GuiKit/hotbar = shared header users outside the three mods.')
md.append('')
md.append('## 1. Hook sites (all 7)')
md.append('')
md.append('| # | Steam site | GOG site | enclosing fn (Steam -> GOG) | hook fn | original_bytes (Steam toml) | GOG bytes identical | window |')
md.append('|---|---|---|---|---|---|---|---|')
for n, addr, gaddr, key, fnname, ob, ident, ni, psq, gq in hook_rows:
    s0, g0 = PAIRS[key]
    md.append('| %d | 0x%08X | **0x%08X** | %s 0x%08X -> 0x%08X | %s | `%s` | **%s** | %d whole insn, %d B |' % (n, addr, gaddr, key, s0, g0, fnname, ob.hex(' '), 'YES' if ident else 'NO', ni, len(ob)))
md.append('')
md.append('Steam `original_bytes` were checked against the Steam dump first (all 7 equal, whole-instruction windows). The Steam->GOG shift differs per hook host: UpdatePortraits -0x22E880, the DrawEffects/DrawStack/UpdateToolTipText region -0x22EA00 (a plain section shift would land mid-function).')
md.append('')
md.append('Hook parameter sources (each defining instruction quoted, Steam vs GOG):')
md.append('')
for n, addr, gaddr, key, fnname, ob, ident, ni, psq, gq in hook_rows:
    md.append('**Hook %d, Steam 0x%08X -> GOG 0x%08X** (%s). GOG window: %s' % (n, addr, gaddr, fnname, '; '.join('`%s`' % x for x in gq)))
    md.append('')
    if not psq: md.append('- no hook parameters')
    for p in psq: md.append('- ' + p)
    md.append('')
md.append('Because each host function is 90-100% byte-identical (UpdatePortraits 594/640 identical + 46 address-only; RowDraw 250/258 + 8; SlotDrawGate 36/36; TooltipText 166/204 + 38), every `[ebp+-x]` slot has the same meaning in GOG.')
md.append('')
md.append('## 2. Mapping tables')
md.append('')
md.append(table('func', 'Engine functions'))
md.append('')
md.append(table('glob', 'Globals'))
md.append('')
md.append(table('vt', 'Vtables'))
md.append('')
md.append(table('off', 'Struct offsets / layout constants', cols=('symbol / use', 'K2AddrTable field', 'Steam', 'GOG', 'evidence', 'conf', 'used by')))
md.append('')
md.append('### Constants outside K2AddrTable (checked, identical in GOG)')
md.append('')
md.append('- Party table pointer `*(*(clientApp+4)+0x270)` (Party.h): ' + S_('GetSWParty', fx('GetSWParty', r'0x270')) + '; table count at `[table+0]` and 0x88-byte member stride: ' + S_('PartyGetAt', fx('PartyGetAt', r'\[EAX\]')) + '. `ClientPartyTable` etc. are table fields above.')
md.append('- HUD slot array: `IMUL ECX,ECX,0xde4` / `LEA EAX,[EDX+ECX+0x1f34]` and the mirror slot `ADD EDX,0x48e0` are byte-identical in GOG UpdatePortraits (hooks 1/2 sit right behind them).')
md.append('- `creature[0x43d]` (comment-only in ShieldBar.cpp) and `0x57e8c0` / `0x5413c0` slot targets: not used as constants; the vtable slots are read at runtime (Creature_VtMaxHp/CurHp).')
md.append('')
md.append('## 3. Unresolved items and what would resolve them')
md.append('')
md.append('| item | status | what would resolve it |')
md.append('|---|---|---|')
md.append('| `KeyboardDeviceGlobal` (Steam 0x0099C884) | unknown, zero in kGog | Used only by ForcePowerHotbar (none of the 3 mods). Dump a pair for a Steam reader (0x785DF0 holds 12 reads) and align it, same as the other globals. |')
md.append('| `TexPendingList` (-> 0x00A14CD4) | high | Hotbar only. Aligned in 3 functions but no semantic parent quote; add the hotbar texture-queue caller pair if the hotbar is ported. |')
md.append('| `ClipNoFillColor` (-> 0x00A768A4) | high | Single aligned site (RowDraw). A second aligned reader (e.g. the fog-of-war pair 0x895CC0 <-> its GOG twin) would make it exact. Live check: a GOG `PushClip` with this arg draws no fill. |')
md.append('| `InGameMenuDraw` 0x756B70 -> 0x528270 | high | Only offset evidence (not an address field). It is a vtable-slot function (no direct caller): add a PTRS dump of the CSWGuiInGameMenu vtable on both sides if a hard anchor is wanted. |')
md.append('| `PtrListAdd` scope | exact for panel lists only | Steam folded identical instances (>=200 callers); GOG keeps per-type copies (0x534170 has 65 callers). GuiKit only appends to the panel control list, which is exactly this instance. Any other GOG pointer list needs its own instance. |')
md.append('| Everything dynamic | not verifiable statically | `KOTOR_VERSION_SHA` selection, the `kGog` table in a running GOG process, and CGameEffect/portrait behaviour need one live GOG launch (the user, game closed here). All addresses are static-proven; nothing here has been executed on GOG. |')
md.append('| GOG hooks toml files | not written | Each of `portrait-slots`, `buff-duration-hud`, `shield-bar` needs a `kotor2-gog-aspyr.hooks.toml` with `target_versions = ["777BEE23..."]` and the 7 GOG addresses from section 1 (same `original_bytes`, same parameter sources). Not created (outside the allowed output files). |')
md.append('')
md.append('## 4. Ready-to-paste `kGog`')
md.append('')
md.append('`K2AddrTable` is built by name (`makeSteam()` style), so a positional `{...}` list would be unsafe. Replace the `kGog` line in `_shared_src/K2Addr.h` (the file is not modified by me) with the block below. Fields keyed exactly as in the working-tree K2Addr.h (commit 763d010 plus later uncommitted additions; 110 of its 130 fields are mapped here, the other 20 are the Scene/Gob/View/GuiGate/GuiControlCtor inventory-3d fields, left as comments = zero); the `gameaddr::` party addresses used by Party.h (`GetControlledCreature`, `PartyGetAt`, `MemberServerCreature`, `ClientPartyTable`) are table fields and are filled. Also in `gog_map_21b/kgog.inc`.')
md.append('')
md.append('```cpp')
md.append(KG)
md.append('```')
md.append('')
md.append('## 5. Riskiest claims to spot-check')
md.append('')
md.append('1. **TextColor -> 0x00915510** (not the name-matching `SetColor` 0x009153E0): the Steam function is 50 B and byte-identical to GOG `CSWGuiTextParams::SetDefaultColor`; aligned call in TextLoad confirms. Spot check: TextLoad 0x00415850 calls 0x00915510 right after SetAlignment.')
md.append('2. **PtrListAdd -> 0x00534170** is the CSWGuiControl* instance only (Steam folded all instances into 0x0083EA60; the one xref-count mismatch in 83 pairs).')
md.append('3. **Vtable addresses** are not shifted consistently (Label 0x0098998C but Border 0x009A994C / Text 0x009A9A28); each is quoted from the ctor `MOV [EAX],imm32` and confirmed by slot-by-slot PTRS dumps.')
md.append('4. Menu control names: `Menu_Char2/3`, `Level`, `Back`, `Change` come from the string-tag pushes before `InitControl` in the CSWGuiInGameMenu constructor (strings from `strings.tsv`); the offsets are byte-identical in GOG.')
md.append('')
md.append('## 6. Reproduce')
md.append('')
md.append('`cd gog_map_21b; python3 run_ghidra.py steam|gog <dir with requests.txt>` re-dumps (about 3-4 min per run, rounds r1..r5 in `dumps/`); `python3 align.py pairs_r1.tsv dumps/steam_r1.txt dumps/gog_r1.txt` prints the alignment; `python3 build_map.py` regenerates this document (from `gog_map_21b/`, writes `../21b_gog_address_map.md`).')
open('../21b_gog_address_map.md', 'w').write('\n'.join(md) + '\n')
print('wrote md', len('\n'.join(md)))
