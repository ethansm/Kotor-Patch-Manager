#!/usr/bin/env python3
"""Persist the hotbar item edge-case RE lessons (Force Power Hotbar B1 research, 2026-10-04) into kotor2_ghidra_knowledge.db.
Idempotent by (scope, topic): a lesson is added once and updated in place if its note changes. --dry-run writes nothing.
Evidence: patch_manager_mods/07_force_power_hotbar_evidence/target_modes/EDGE_CASES.md"""
import os, sqlite3, sys

DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "kotor2_ghidra_knowledge.db")
DATE = "2026-10-04"
LESSONS = [
    ('hotbar-item-edge-cases',
     'CanUseItem 0x5D2F80 strict arg: server 0x57B544 passes (item,0,0) on the ACTING creature, vanilla party pick 0x77C818/0x77CAD8 passes strict=1 on the TARGET',
     "0x5D3650: baseitems(+0xB4)==1 needs race [[creature+0x1198]+0xE0] (word)==[0x99A8BC]=6, ==2 needs [0x99A8B8]=5; mismatch returns 0 only if strict, else 1; waived for CastSpell subtype 0x81. strict + baseitems+0xAC in {0x2D,0x1A} also refuses when vtbl+0x9C(0)==vtbl+0x98(1) (0x5D3023). PerformUseItem 0x775600 has 4 stack args, ECX unused; acting creature = sender's controlled creature. Evidence: target_modes/EDGE_CASES.md"),
    ('hotbar-item-edge-cases',
     'Item property list (server CSWSItem): count +0x288, array +0x290, stride 0x1C; second list +0x28C/+0x294',
     'Entry +0 int16 type (10 CastSpell), +2 subtype = spells row (0x576430 identity), +6 use-type (1 stack, 2-6 charges item+0x298, 8-12 per-day entry+0x10, 0xE-0x12 once/day), +0xC usable byte, +0x11 upgrade slot. PerformUseItem packed byte = index into +0x290 list (HUD passes 0). Charges tested at 0x5B8000 via 0x603B50 (msg 0x17), consumption 0x6070A0 at animation start. Evidence: target_modes/EDGE_CASES.md'),
    ('hotbar-item-edge-cases',
     'Player item use is a 2-phase combat-round action: UseItem 0x57B4C0 -> 0x5648D0(p12=0) -> 0x591890 + Combat(0x3F); walk/cast queued by AIActionCombat 0x6D9500 -> 0x5648D0 -> 0x2E 0x5B8000',
     'Phase 2 queues MoveToPoint(0x564770/0x5644B0), 0x11, 0x13, 0x2E when dist^2>range^2; refuses other-area targets (00564a74); 0x5B8000 cancels on dead/HP<1 target (vtbl+0x94). Perception veto in 0x57B4C0 (entry bits2-3==1) returns silently. Unproven: p12 value from 0x6D9500, combat interrupt of 0x2E. Evidence: target_modes/EDGE_CASES.md'),
]


def main():
    dry = "--dry-run" in sys.argv
    db = sys.argv[sys.argv.index("--db") + 1] if "--db" in sys.argv else DB
    c = sqlite3.connect(db)
    add = upd = same = 0
    for scope, topic, note in LESSONS:
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
