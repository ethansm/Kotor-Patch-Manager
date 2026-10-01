#!/usr/bin/env python3
"""Persist the doc-15 Phase 1 (V1-V6, 2026-10-01) Opus-verified findings into ghidra_knowledge/kotor2_ghidra_knowledge.db.
Idempotent: lessons by topic; name_map rows are added only where the address has no name yet (existing names are kept)."""
import os, sqlite3
DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
DATE = "2026-10-01"
SRC = "V1-V6-bridge-2026-10-01"
LESSONS = [
    ("dialogue-letterbox", "Dialogue letterbox bar = trunc((H - trunc(W/c))/2), c = double 2.3333330154 at 0x9A7D30",
     "Same formula as K1/KMRP K7. 7 FDIV sites (0x8BACE9 SetTop, 0x8BADE9 SetBottom, 0x8BAF6E dead, 0x8BB6A9 DialogTop ctor, 0x8BB9A6 SetReply, 0x8BBC25 Cinematic ctor, "
     "0x8BC467 Reset), each `DC 35 30 7D 9A 00`; W/H = shorts at GUI manager +0x6C/+0x6E. Negative past 7:3 (3840x1600 -> -22, 3440x1440 -> -17). Reply panel/bottom edge are "
     "clamped H-max(bar,100*fs) and scaled by fs=1+0.5*floor((W-1022)/1024); bars themselves are not. Fix idea: write 1.5*W/H over the double (bar = H/6). 0x4113A0 is the GUI draw loop "
     "(4:3 pillarbox scissor), NOT the dialogue letterbox. Source: engine_research_findings/V1.md."),
    ("gui-text", "WrapStrings 0x47AF30: 100-line cap, +0.25 glyph advance, Draw drops the last line",
     "K2 has K1's progress compare (CMP cursor,[this+0x14]=string base at 0x47B321) but a 100-line cap (0x47B0E8/0x47B109) bounds it: no OOM hang, only silent text loss. "
     "Glyph advance: FADD double [0x98C1D8]=0.25 at 0x47B232 then truncate (0x91C860) vs Draw's exact float; fix = re-point the operand (0x47B234) to the 0.5 double at 0x987F08. "
     "Real Draw is 0x47B500 (0x47A130 is a 30-byte stub). Draw drop loops 0x47B6CB/0x47B7CB can drop the last line. Font scale 0x479AB0: 1.0@1920, 1.5@2560, 2.0@3440/3840, 3.0@5120. Source V3.md."),
    ("gui-listbox", "ListBox row-height inflation (KMRP K2) does not occur in K2",
     "OrganizeControls 0x41EA50 stores inflated heights to locals, AddControls 0x41FC30 saves each row's rect in the vector at list+0x2B8 (CALL 0x421720 at 0x41FD3A), ClearItems 0x41D290 "
     "restores via SetExtent (vtbl+4) when [list+0x2CC] bit 2 set and bit 1 clear; Mod 6's PopulateItemListBox passes restore=1 (0x8A7E8C). Source V2.md."),
    ("timing", "Timer::Now 0x73B810 = int64 microseconds; all pacing subtracts int64 first",
     "QPC at 0x73B825 -> FILD qword, FMUL double [this+0x18], 0x91C896 (double->int64, EDX:EAX). Fallback GetTickCount*1000 at 0x73B856 when QPF fails. 0x736220 is the fastcall thunk (44 caller fns). "
     "timeGetTime is only the FPS overlay/pulse. Peixoto-style relative QPC would change nothing. Source V5.md."),
    ("2da", "2DA: one parser 0x71C3D0 for text V2.0 and binary V2.b; only column names reach the 256-byte stack buffer on the binary path",
     "Format chosen by CRes2DA::OnResourceServiced 0x729FD0 (magic `2DA V2.0`/`2DA V2.b` -> CRes+0x38 -> C2DA+0x3C). 0x71D360 = C2DA::Init (find/create), not a parser. Token buffer is 256 B ending "
     "exactly at the stack cookie ([EBP-0x2c]); a 256-char token breaks the cookie. Binary data-size u16 is written to a stack local at 0x71C8D2 and never read (benign). Live install: all 424 BIF + 37 "
     "override 2DAs are binary; longest column name 23. K2 caps: appearance u16, portraits <0xFFFE, soundset u16 count, classes byte count, heads byte path 1..254 (live heads.2da 249 rows). "
     "The community docs/2da_row_limits.md is K1. Scanner: engine_bug_research/tools/scan_2da_limits.py. Source V6.md."),
    ("method", "K1->K2 name bridge (engine_bug_research/tools/k1_to_k2_bridge.py)",
     "KPM kotor1_0_3.db (nearest preceding named function) -> kotor2_gog_aspyr.db same class::function (prefer the candidate whose param size equals K1's: Draw vs Draw_2) -> Steam via "
     "GOG->Steam maps (*_map_*.tsv, gog_map_mods678.tsv). Validated on ClearItems 0x41D290, OrganizeControls 0x41EA50, DrawMap 0x750790, SetText 0x416E30, WrapStrings 0x47AF30. "
     "Run gog_cluster_delta_map.py for a class first (V_map_*.tsv) when a row shows gog-only."),
]
NAMES = [
    ("0x008baca0", "CSWGuiDialogLetterbox::SetTop", "V1; bridge K1 0x6A74B0, ret 0 size 245"),
    ("0x008bada0", "CSWGuiDialogLetterbox::SetBottom", "V1; bridge K1 0x6A7540, ret 0 size 367"),
    ("0x008bb5d0", "CSWGuiDialogTop::Constructor", "V1; ret 12"),
    ("0x008bb8d0", "CSWGuiDialogTop::SetReply", "V1; ret 4"),
    ("0x008bba80", "CSWGuiDialogCinematic::Constructor", "V1; ret 4; loads dialog_p"),
    ("0x008bc3e0", "CSWGuiDialogCinematic::Reset", "V1; ret 0"),
    ("0x008bc050", "CSWGuiDialogCinematic::SetExtent", "V1; ret 4"),
    ("0x008bc230", "CSWGuiDialogCinematic::SetReplies", "V1; ret 20"),
    ("0x0047af30", "CAurGUIStringInternal::WrapStrings", "V3; ret 4 size 1475"),
    ("0x0047b500", "CAurGUIStringInternal::Draw_2", "V3; ret 4 (0x47A130 is the 30-byte stub)"),
    ("0x00416e30", "CSWGuiTextParams::SetText", "V3; ret 4"),
    ("0x0075c080", "CSWGuiMessageBox::FixMessageLabel", "V4; ret 0 size 2473; caps 440*fs x 280*fs"),
    ("0x0075ae40", "CSWGuiMessageBox::Constructor", "V4; ret 4"),
    ("0x00894cd0", "CSWGuiInGameMap::Draw", "V4; hard-coded 640x480 centring"),
    ("0x0073b810", "Timer::Now (int64 microseconds)", "V5; QPC at 0x73B825"),
    ("0x00729fd0", "CRes2DA::OnResourceServiced", "V6; magic check"),
    ("0x0071d360", "C2DA::Init", "V6; find/create CRes2DA, not a parser"),
]
c = sqlite3.connect(DB); cur = c.cursor()
have = {r[0] for r in cur.execute("select topic from lessons")}
nxt = (cur.execute("select max(id) from lessons").fetchone()[0] or 0) + 1
for scope, topic, note in LESSONS:
    if topic in have:
        cur.execute("update lessons set note=? where topic=?", (note, topic)); continue
    cur.execute("insert into lessons (id,scope,topic,note,date) values (?,?,?,?,?)", (nxt, scope, topic, note, DATE)); nxt += 1
added = 0
for addr, name, ev in NAMES:
    row = cur.execute("select name,source from name_map where steam_addr=?", (addr,)).fetchone()
    if row:
        print("name_map keeps existing", addr, row[0], "(%s)" % row[1]); continue
    cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)", (addr, name, SRC, "medium", ev, DATE)); added += 1
# Opus override: the Global::BSPNodeEnsureNodeData mapper row at 0x47B500 is wrong (see lessons 'Mapper rows can disagree').
cur.execute("update name_map set name=?,source=?,confidence=?,date=? where steam_addr='0x0047b500' and name like 'Global::BSPNodeEnsureNodeData%'",
            ('CAurGUIStringInternal::Draw_2', 'V3-Opus-override-2026-10-01', 'medium', DATE))
c.commit()
print("added names", added, cur.execute("pragma integrity_check").fetchone(), "lessons:", cur.execute("select count(*) from lessons").fetchone()[0])
