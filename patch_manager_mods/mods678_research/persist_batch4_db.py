#!/usr/bin/env python3
"""Persist the Mod 8 Stage 0 review lesson (2026-09-30) into ghidra_knowledge/kotor2_ghidra_knowledge.db. Idempotent by topic."""
import os, sqlite3
DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
DATE = "2026-09-30"
LESSONS = [
    ("kpatch-framework", "consumed_exit_address stolen bytes must not use EAX",
     "wrapper_x86.cpp order: handler call -> restore regs (EAX excluded) -> run stolen bytes -> TEST EAX,EAX -> jump consumed exit or resume. "
     "During the stolen bytes EAX = handler return value, NOT the game's EAX. A window whose stolen bytes read EAX (e.g. MOV ECX,[EAX+0x10E8] at 0x6D7C73) faults on first hit; "
     "one that writes EAX overrides the verdict (0x77ACBC ends MOV EAX,[EBP-0xF8]; 0x77D8D4 has MOV EAX/SUB EAX). Site the hook at the first EAX-free boundary "
     "(E-H2 moved to 0x6D7C79). check_windows.py warns on M8 windows that touch EAX. Found by independent Opus review before install."),
]
c = sqlite3.connect(DB); cur = c.cursor()
have = {r[0] for r in cur.execute("select topic from lessons")}
nxt = (cur.execute("select max(id) from lessons").fetchone()[0] or 0) + 1
for scope, topic, note in LESSONS:
    if topic in have: cur.execute("update lessons set note=? where topic=?", (note, topic)); continue
    cur.execute("insert into lessons (id,scope,topic,note,date) values (?,?,?,?,?)", (nxt, scope, topic, note, DATE)); nxt += 1
c.commit(); print(cur.execute("pragma integrity_check").fetchone())
