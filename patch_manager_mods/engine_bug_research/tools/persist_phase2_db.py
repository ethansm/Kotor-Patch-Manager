#!/usr/bin/env python3
"""Persist the doc-15 Phase 2 (2026-10-02, builds L1 k2-dialogue-letterbox-fix + L4 k2-wrap-measure-fix) lessons into ghidra_knowledge/kotor2_ghidra_knowledge.db.
Idempotent: lessons by topic (update if present, insert otherwise); name_map rows are added only where the address has no name yet (existing names are kept)."""
import os, sqlite3
DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
DATE = "2026-10-02"
SRC = "doc15-phase2-2026-10-02"
LESSONS = [
    ("kpatch-framework", "KPM simple hooks write any address",
     "A KPM `simple` hook is a byte write at runtime after VerifyBytes: no instruction-boundary rule and no minimum length (only `Replace` needs >= 5 B). .rdata/.text operand targets work. "
     "k2-wrap-measure-fix 1.0.0 uses one 6-byte simple hook at 0x0047B232 (`DC 05 D8 C1 98 00` -> `DC 05 08 7F 98 00`, FADD operand re-pointed to the 0.5 double at 0x987F08) with no DLL. See doc 17."),
    ("dialogue-letterbox", "L1 writer site: SetScreenSize 0x411A80 is the only writer of GUI-manager W/H; hook 0x411AC1 (7 B), manager [ebp-0xC], reached only on change; 4 callers",
     "GUI manager W/H shorts +0x6C/+0x6E are written only by CSWGuiManager::SetScreenSize 0x411A80 (plus the ctor 0x4100C0 zeroing them). 4 callers: startup 0x78A20F, options Apply 0x78DDA7, "
     "credits 640x480 force 0x89222B and restore 0x892756. Hook 0x411AC1 = `8B 55 F4 83 7A 7C 00`, after both stores and the rescale CALL 0x413DD0; manager = [ebp-0xC] (ECX saved at 0x411A86); "
     "reached only when W or H changed (early-out 0x411AA1), exactly when the 0x9A7D30 double must change. Source: engine_research_findings/V7.md; build doc 17."),
    ("kpatch-framework", "KPM detour wrapper runs the handler BEFORE the stolen bytes",
     "wrapper_x86.cpp: the handler is called first, then state is restored, then 'EXECUTE STOLEN ORIGINAL BYTES' runs. So a log hook on a store instruction (e.g. `MOV [ECX+0x88],1` at 0x8BAD7C) sees "
     "the memory BEFORE the store (old value). Used by k2-dialogue-letterbox-fix SETTOP/SETBOTTOM logging (mode +0x88 shows the previous mode)."),
    ("method", "Smoke-testing a DLL that writes a fixed game VA: 32-bit test exe at base 0x400000 with a 7 MB .bss pad",
     "Fixed-VA writes (e.g. .rdata 0x9A7D30) need that VA to be ordinary writable memory in the harness: link the 32-bit test exe at image base 0x400000 with a ~7 MB .bss pad so the VA lies inside it "
     "(ActivationFix harness pattern; K2DialogueLetterboxFix smoke_dialogueletterbox.cpp, 9 modes pass under Wine, protection restored to PAGE_READONLY)."),
    ("dialogue-letterbox", "c = 1.5*W/H double: trunc(W/c) can land one below 2H/3 for H divisible by 3; multiply by (1-1e-9)",
     "With the naive double c = (1.5*W)/H, trunc(W/c) is 2H/3 - 1 for 388,370 of 27 M (W,H) pairs, which shifts the bar/panel by one px; multiplying c by (1 - 1e-9) gives 0 failures. "
     "Source V7 Q5. 3840x1600 -> bar 267 (the plan's 266 was an arithmetic slip)."),
]
NAMES = [
    ("0x00411a80", "CSWGuiManager::SetScreenSize", "V7; only writer of GUI-manager W/H shorts +0x6C/+0x6E; 4 callers; hook 0x411AC1"),
]
c = sqlite3.connect(DB); cur = c.cursor()
have = {r[0] for r in cur.execute("select topic from lessons")}
nxt = (cur.execute("select max(id) from lessons").fetchone()[0] or 0) + 1
ins = upd = 0
for scope, topic, note in LESSONS:
    if topic in have:
        cur.execute("update lessons set note=? where topic=?", (note, topic)); upd += 1; continue
    cur.execute("insert into lessons (id,scope,topic,note,date) values (?,?,?,?,?)", (nxt, scope, topic, note, DATE)); nxt += 1; ins += 1
added = 0
for addr, name, ev in NAMES:
    row = cur.execute("select name,source from name_map where lower(steam_addr)=?", (addr.lower(),)).fetchone()
    if row:
        print("name_map keeps existing", addr, row[0], "(%s)" % row[1]); continue
    cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)", (addr, name, SRC, "medium", ev, DATE)); added += 1
c.commit()
print("lessons added", ins, "updated", upd, "| names added", added, cur.execute("pragma integrity_check").fetchone(), "lessons:", cur.execute("select count(*) from lessons").fetchone()[0], "name_map:", cur.execute("select count(*) from name_map").fetchone()[0])
