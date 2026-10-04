#!/usr/bin/env python3
"""Persist the companion self-heal actor-route RE lessons (doc 27 CS0, 2026-10-04) into kotor2_ghidra_knowledge.db.
Idempotent by (scope, topic): a lesson is added once and updated in place if its note changes. --dry-run writes nothing.
Evidence: patch_manager_mods/27_companion_selfheal_evidence/R1_actor_route/FINDINGS.md"""
import os, sqlite3, sys

DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "kotor2_ghidra_knowledge.db")
DATE = "2026-10-04"
E = "Evidence: 27_companion_selfheal_evidence/R1_actor_route/FINDINGS.md"
LESSONS = [
    ('companion-selfheal',
     'Companion as item user: AddUseTalentOnObjectActions 0x57D8B0 (thiscall ECX=creature, RET 0x20) args (talentType 0, talentId, targetId, byte, itemServerId 0x7F000000=none, propIdx -1=none, byte, byte)',
     'Gate +0xEC commandable at 0057d8e3; item branch 0057d964..0057d98f pushes (item, propIdx byte, 0, target, &pos, area, 0) -> UseItem 0x57B4C0 (RET 0x1C, 7 args, last=flag: msg handler 1 at 0065c5d4, here 0); UseItem result ignored (silent refusal). Script cmd 0x135/0x136 0x66BDC0 calls it at 0066beaa with ECX = OBJECT_SELF creature. ' + E),
    ('companion-selfheal',
     'UseItem 0x57B4C0 does not enqueue 0x2E: 0x5648D0(last=0) adds a round entry (0x591890) + combat action 0x3F; AIActionCombat 0x6D9500 calls 0x5648D0(last!=0) at 006d9ef6 to build 0x2E; RunActions 005d5fc4 -> 0x5B8000',
     'Trace the RunActions 0x5D56B0 case table before trusting an enqueue name. 0x563C90 = interrupt wrapper ending in ClearAllActions(1) (DB name ActionManager hides it); ClearAllActions also clears the round entry via 0x5908F0. 0x5D32C0 is a class-restriction check (mis-named). ' + E),
    ('companion-selfheal',
     'Item ids: CanUseItem 0x5D2F80 takes a CSWSItem* (via 0x51C0E0 server id -> ptr), not an id; ClientToServerObjectId 0x51C8B0 = id & ~[0x99449C]',
     'No possession check in 0x57B4C0/0x5D2F80/0x5B8000/head of 0x6070A0, so a party-inventory item should be usable by a companion (0x6070A0 tail unverified). Ghidra thiscall decomp shows ECX as param 1 and mislabeled ECX at 0066beaa; settle arg order from callsite pushes. ' + E),
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
