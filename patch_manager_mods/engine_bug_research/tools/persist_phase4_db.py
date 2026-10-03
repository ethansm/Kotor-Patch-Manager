#!/usr/bin/env python3
"""Persist the doc-15 Phase 4 results (V11 + L1 1.0.1 + L4 drop; 2026-10-03; Opus-verified) into ghidra_knowledge/kotor2_ghidra_knowledge.db.

Idempotent, same approach as persist_phase3_db.py:
  * lessons are keyed by topic: inserted when absent, updated only when the stored note/scope differs, otherwise left alone;
  * name_map rows are added only where the address has no name yet (existing names are kept and printed).
A second run therefore adds 0 and changes 0.

Usage: persist_phase4_db.py [--db PATH]   (default: the real knowledge DB next to this tool tree)
"""
import argparse, os, sqlite3

DEFAULT_DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
DATE = "2026-10-03"
SRC = "doc15-phase4-2026-10-03"

# (scope, topic, note). Topic is the idempotency key. Source: engine_research_findings/V11.md (its "Opus verification (2026-10-03)"
# section overrides the body), 17_dialogue_letterbox_and_wrap_fixes.md, dialogue_letterbox_fix_src/.
LESSONS = [
    ("dialogue-letterbox", "V11: letterbox globals 0xA309B4/B8/BC are read ONLY by the video-effect composite quads; L1 moves/clips nothing else (NO BUG)",
     "Exhaustive grep (asm operand + bytes column + .c + globals.tsv, 8/8/6 hits): writers SetTop 0x8BACA0, SetBottom 0x8BADA0, Draw 0x8BB260 (0x8BB467/0x8BB531), ResizeToText 0x8BAB20; [0xA309BC] only 0x7CEB80. "
     "Readers = 5 copies of one quad (0x4426F0, 0x443190, 0x445310 live; 0x4421F0, 0x442C90 DEAD because 0x484910 always returns 0: `00484955 MOV [0x9F6124],0` unconditional). The quad is y-up [b4-25, b8+25] while the flag is set "
     "(25.0 double at 0x989ED0, read from the vanilla exe). Vanilla 3840x1600: b4 = -22 (top-bar h is NOT clamped: 0x8BB42A JGE ends the slide on frame 1, 0x40E9A5 stores -22), b8 1400 -> band [-47,1425]; L1: b4 267 (262 after SetReply), b8 1333 -> "
     "band [242,1358], i.e. edges under the bars. Passes fire only for clairvoyance/full (videoeffects.2da rows 3-6 -> [0xA1DD4A]) or Fury (rows 7-9 -> [0xA1DD54]) set by a conversation node's CamVidEffect (42 DLGs / 308 nodes here, incl. override kreia.dlg) "
     "with Frame Buffer=1 (dispatcher 0x4373A0 from 0x46C5C0) and GL-capability gates (SPECULATIVE on Proton). Not a viewport/scissor/subtitle/HUD/camera/picking input. Cosmetic residue only (effect image 70 % vs 92 % height, seam inside each 50 % bar)."),
    ("dialogue-letterbox", "SetReply 0x8BB8D0 does nothing while the top letterbox bar's done flag [+0x8C] is 0 (UNCLEAR, low risk with L1)",
     "0x8BB8D0 body is `if (*(int *)(*(int *)(FUN_0073f750() + 0x60) + 0x8c) != 0) { SetText 0x416E30; resize top panel; 0x8BAB20 }` (asm 008bb8fb MOV EDX,[ECX+0x8C]); callers 0x8BC230/0x8BC320 are virtual. "
     "Vanilla 3840x1600 finishes the top slide on frame 1 (target h -22), L1 slides ~0.5 s, so an NPC line set during the slide would skip the resize/text set. Every vanilla resolution below 7:3 (e.g. 1920x1080, bar 129) has the same slide, "
     "and 12 L1 dialogues produced no missing-text report: low risk. T1 add-on: confirm the first NPC line of a conversation shows its text."),
    ("textures", "V11: GUI image fill drains pending texture builds BEFORE binding (0x47D3F3 before 0x47D401): no first-draw white on that path",
     "CSWGuiBorder::Draw 0x415300 -> CAurGUIImageInternal vtbl+0x18 0x47D210 -> vtbl+0x14 0x47D2F0, whose first calls are `0047d3f3 CALL 0x427F30` (drain every pending build via 0x427810) then `0047d401 CALL 0x4DAE60` (bind: 0x4DA750 NULL-texture fallback, "
     "texture vtbl+0xB8 = MakeCurrent 0x422ED0). Id 0 / untextured only for the by-design NULL texture, [0xA1BBDC] != 0 (never written) or the load window [0xA1BB9D] = 1 (set 0x428F30 at 0x7A4E46, cleared 0x428F40 at 0x7A55A6 inside 0x7A2FB0; whether GUI fills draw there not traced). "
     "Resolves V10's UNCLEAR as NO BUG. Cost: the first fill after a screen opens pays for all queued builds (hitch)."),
    ("movies", "V11: Bink offset for 3440x1440 on 3840x1600 is a SIGNED -3 (CDQ/SUB/SAR), not clamped; whole popup covered; nothing visible today",
     "Both BinkBufferSetOffset sites (0x40D392 in 0x40CC70, 0x40DD3D in refresh 0x40DD10) receive (0,-3); the unsigned SHR huge value exists only on the equal-width fast path (L17, 3840-wide movies, unreachable). "
     "Movie popup = WS_POPUP|WS_VISIBLE (0x90000000) over the client rect; the scaled blit covers it for the installed 3440x1440 set and for 1920x1080 movies, so the NULL class brush never shows. BinkBufferOpen flags 0x5D000000 = bits 24,26,27,28,30, names UNKNOWN (no SDK header). "
     "Whether Bink clips a negative offset is SDK-internal (UNCLEAR). Notes on L16/L17."),
    ("dialogue-letterbox", "k2-dialogue-letterbox-fix 1.0.1 (built 2026-10-03, not installed): log=0 default, first-event logging; L4 dropped at the next install",
     "1.0.1 = 1.0.0 hooks byte-identical (0x411AC1, 0x8BAD7C, 0x8BAF01), logging only: log default 0 writes the load line, the first non-problem APPLY pair, the first SETTOP and first SETBOTTOM per process, and every problem line (INVALID/FOREIGN/protect/MISMATCH/null, LATE notice). "
     "kpatch f1e59f7b12b339f2bb879cfb480a9b0c, DLL cf45a03af96881362e586c8baf0654b9 (KERNEL32 + msvcrt), Wine smoke 10 modes. Analyzer: for log=0 launches SETTOP/SETBOTTOM are checked for self-consistency (bar from their own W/H/c), not against the last logged APPLY "
     "(later APPLYs are not logged). User decision 2026-10-03: k2-wrap-measure-fix (L4, no-op at fs 1/2/3) is dropped at the next install; source kept for fs 1.5."),
]

# (steam_addr as stored in name_map, name, confidence, evidence). Added only where the address has NO name yet.
NAMES = [
    ("0x004373a0", "post-process video-effect dispatcher (SPECULATIVE name)", "medium", "V11; gate [9F4E74]&&[A1DD29]&&[9F48FB]; caller 0x46C5C0"),
    ("0x00437bd0", "post-process video-effect path B (SPECULATIVE name)", "medium", "V11; 0x484CA0 ? 0x447300 : 0x4426F0/0x443190"),
    ("0x00447300", "post-process video-effect path B2 (SPECULATIVE name)", "medium", "V11; calls 0x445310 arg 0 (clairvoyance) / arg 1 (Fury)"),
    ("0x004426f0", "video-effect composite quad, clairvoyance (letterbox-clipped)", "medium", "V11; reads 0xA309B4/B8/BC, band [b4-25, b8+25]"),
    ("0x00443190", "video-effect composite quad, Fury (letterbox-clipped)", "medium", "V11; reads 0xA309B4/B8/BC"),
    ("0x00445310", "video-effect composite quad, path B2 (letterbox-clipped)", "medium", "V11; reads 0xA309B4/B8/BC; arg 0/1"),
    ("0x004421f0", "video-effect composite quad, clairvoyance (DEAD: behind 0x484910)", "medium", "V11; only caller 0x437A6E in the 0x484910 != 0 branch"),
    ("0x00442c90", "video-effect composite quad, Fury (DEAD: behind 0x484910)", "medium", "V11; only caller 0x437B28"),
    ("0x00484910", "GL capability test #1 (always returns 0)", "high", "V11; 00484955 MOV [0x9F6124],0 unconditional before the load"),
    ("0x0079e550", "apply videoeffects.2da row (SPECULATIVE name)", "medium", "V11; strings EnableFury/EnableClairvoyance/EnableForceSight/Saturation_PC"),
    ("0x00442060", "EnableClairvoyance flag set ([0xA1DD4A] = 1)", "high", "V11; 00442063 MOV byte [0xA1DD4A],1"),
    ("0x00442090", "SetFuryLevel ([0xA1DD54], loads fx_Fury01..03)", "high", "V11; decomp FUN_004274e0(\"fx_Fury02\",0)"),
    ("0x0047d210", "CAurGUIImageInternal::Draw (vtbl +0x18)", "medium", "V11; vtable 0x98C1FC"),
    ("0x0047d2f0", "CAurGUIImageInternal draw quad (drain builds then bind; vtbl +0x14)", "medium", "V11; 0x47D3F3 CALL 0x427F30 then 0x47D401 CALL 0x4DAE60"),
    ("0x004dae60", "GUI image texture bind (MakeCurrent via vtbl +0xB8)", "medium", "V11; calls 0x4DA750 then texture vtbl +0xB8"),
    ("0x004da750", "GUI image NULL-texture fallback", "medium", "V11; substitutes AurTextureGetNULL 0x427410 when missing/failed"),
    ("0x00422ed0", "CAurTextureBasic::MakeCurrent", "medium", "V11; texture vtable 0x988094 + 0xB8; binds GL id, NULL texture -> id 0"),
    ("0x0040dd10", "movie window move/refresh (re-SetOffset + PatBlt)", "medium", "V9/V11; BinkBufferSetOffset at 0x40DD3D"),
]

def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--db", default=DEFAULT_DB, help="knowledge DB path (default: the real DB)")
    args = ap.parse_args()
    db = os.path.abspath(args.db)
    c = sqlite3.connect(db); cur = c.cursor()
    have = {r[0]: (r[1], r[2]) for r in cur.execute("select topic,scope,note from lessons")}
    nxt = (cur.execute("select max(id) from lessons").fetchone()[0] or 0) + 1
    ins = upd = same = 0
    for scope, topic, note in LESSONS:
        if topic in have:
            if have[topic] == (scope, note):
                same += 1; continue
            cur.execute("update lessons set scope=?, note=?, date=? where topic=?", (scope, note, DATE, topic)); upd += 1; continue
        cur.execute("insert into lessons (id,scope,topic,note,date) values (?,?,?,?,?)", (nxt, scope, topic, note, DATE)); nxt += 1; ins += 1
    added = kept = 0
    for addr, name, conf, ev in NAMES:
        row = cur.execute("select name,source from name_map where lower(steam_addr)=?", (addr.lower(),)).fetchone()
        if row:
            print("name_map keeps existing", addr, row[0], "(%s)" % row[1]); kept += 1; continue
        cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)", (addr, name, SRC, conf, ev, DATE)); added += 1
    c.commit()
    print("db", db)
    print("lessons added", ins, "updated", upd, "unchanged", same, "| names added", added, "kept-existing", kept,
          cur.execute("pragma integrity_check").fetchone(), "lessons:", cur.execute("select count(*) from lessons").fetchone()[0],
          "name_map:", cur.execute("select count(*) from name_map").fetchone()[0])

if __name__ == "__main__":
    main()
