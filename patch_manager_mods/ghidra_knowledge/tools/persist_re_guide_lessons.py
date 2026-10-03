#!/usr/bin/env python3
"""Persist the small-model RE lessons (RE_AGENT_GUIDE.md, 2026-10-03) into kotor2_ghidra_knowledge.db.
Idempotent by (scope, topic): a lesson is added once and updated in place if its note changes. --dry-run writes nothing."""
import os, sqlite3, sys

DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "kotor2_ghidra_knowledge.db")
DATE = "2026-10-03"
EV = " Evidence: ghidra_knowledge/RE_AGENT_GUIDE.md"
LESSONS = [
    ("method", "Small models: scripts compute, models judge; Haiku is a sieve, not a gate",
     "Haiku pilot (steam_db_pr3/haiku_eval_plan.md): 33% false accept on 40 decoys, 82 false mismatches on 434 real rows "
     "(0 DB errors). Cause: lenient matching on ret N/size plus math/convention traps. Use the tiered funnel: tier-0 "
     "scripts (delta sweep, invariants, ABI, decimal->hex, callee topology), Haiku only extracts anchors and flags, "
     "Sonnet takes unsure/mismatch, Opus+lead gate. A Haiku 'match' never promotes a row alone."),
    ("method", "x86 stack-slot promotion: GOG param bytes 1/2/5/13 pop ceil(n/4)*4",
     "GOG metadata sizes args by type (bool 1, short 2). On 32-bit x86 each stack arg is a 4-byte slot, so expected "
     "callee pop = ceil(gog_bytes/4)*4 (cdecl pops 0). 30 of the pilot's 82 false mismatches. Pre-compute it "
     "(re_card_prep.py abi) and state 'Stack Slot Match: TRUE' in the prompt."),
    ("method", "Ghidra decompile prints small offsets in DECIMAL",
     "Bare numbers in .c output are decimal: 0x5F2FE0 has '+ 100)' = byte 0x64 (458 decimal vs 12189 hex '+ N)' forms "
     "in 0x5xxxxx). Asm operands are always hex. Convert before prompting: '+ 0x64 [dec 100]' (re_card_prep.py annotate)."),
    ("method", "Thunks, auto labels and same-ret neighbours fool name matching",
     "Thiscall thunks decompile without __thiscall (judge the JMP target); Ghidra FID/library labels and mapper/DB "
     "names are hypotheses. Neighbours share ret N: 0x5644B0 (CALL AddActionToFront 0x5645E4) vs 0x564770 (CALL "
     "AddAction 0x5648A4) were both 'AddMoveToPointAction'; 0x552A90/0x552C90 GOG sizes swapped. Require a content "
     "anchor (string, confirmed callee, member offset) for any identity claim; verdict field last in the JSON."),
    ("method", "16-byte entry alignment is strong but not a hard invariant",
     "1785 of 1796 high-confidence name_map rows start 16-aligned; the 11 exceptions are CRT/library (operator delete "
     "0x9196FD, operator new[] 0x91D288, __security_check_cookie 0x9196EE, _malloc 0x9215E6). Only 56% of all Ghidra "
     "functions are aligned (fragments, thunks). Use it as a filter for game code, not as proof."),
    ("method", "Parallel analyst agents need private scratch dirs",
     "Atlas wave 2: AI6 overwrote shared scratchpad helpers (pe.py, cmds.pkl, tbl.pkl) while AI7 ran; the Opus gate had "
     "to re-derive every exe-table-byte claim of AI7 (all 774 command handlers re-checked, matched). Every brief must say "
     "'use mktemp -d for helper scripts'."),
]


def main():
    dry = "--dry-run" in sys.argv
    db = sys.argv[sys.argv.index("--db") + 1] if "--db" in sys.argv else DB
    c = sqlite3.connect(db)
    add = upd = same = 0
    for scope, topic, note in LESSONS:
        note = note + EV if len(note + EV) <= 600 else note
        row = c.execute("select id, note from lessons where scope=? and topic=?", (scope, topic)).fetchone()
        if row is None:
            add += 1
            if not dry:
                c.execute("insert into lessons(scope, topic, note, date) values (?,?,?,?)", (scope, topic, note, DATE))
        elif row[1] != note:
            upd += 1
            if not dry:
                c.execute("update lessons set note=?, date=? where id=?", (note, DATE, row[0]))
        else:
            same += 1
    if dry:
        c.rollback()
    else:
        c.commit()
    print(f"lessons: added {add} | updated {upd} | unchanged {same}{' (dry run)' if dry else ''}")
    print("integrity_check:", c.execute("pragma integrity_check").fetchone()[0])


if __name__ == "__main__":
    main()
