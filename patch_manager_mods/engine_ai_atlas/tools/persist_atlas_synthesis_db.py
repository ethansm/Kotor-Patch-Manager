#!/usr/bin/env python3
"""Persist the engine-AI atlas synthesis (Opus REVIEW_synthesis, 2026-10-02) into
ghidra_knowledge/kotor2_ghidra_knowledge.db.  Python 3, stdlib only.

Sources of truth: engine_ai_atlas/bug_candidates.md (section 1 ranked list, section 3 fix ideas, section 5 proposed rows, as
amended by REVIEW_synthesis.md section 3) and REVIEW_synthesis.md (sections 1, 2, 4).  The 27 proposed candidate rows are parsed
from bug_candidates.md at run time (count asserted; AI-SEENSTAYS was dropped by the review and is not inserted).

Idempotent (one transaction; --dry-run rolls it back):
  * candidates: keyed by id; insert if missing, update in place when any field differs.  The existing row AI-MEDPAC-FORCECLASS is
    corrected once for E5 ("a Jedi-only PC" -> the Exile always) and gets one REVIEW_synthesis evidence sentence.
  * lessons: stale lessons 230, 275, 277, 319 (+324, same E5 wording) get one " UPDATED 2026-10-02 (REVIEW_synthesis): ..." sentence
    appended (skipped when the tag is already present); 4 new lessons are keyed by (scope, topic), note updated in place.
  * name_map: the review gives no renames, nothing is written.
"""
import argparse, os, re, sqlite3, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ATLAS = os.path.normpath(os.path.join(HERE, ".."))
DEFAULT_DB = os.path.normpath(os.path.join(ATLAS, "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db"))
BUGS = os.path.join(ATLAS, "bug_candidates.md")

DATE = "2026-10-02"
SRC = "engine-ai-atlas synthesis"
EV = "engine_ai_atlas/REVIEW_synthesis.md"
EV_SUFFIX = " Evidence: " + EV
UPD_TAG = " UPDATED 2026-10-02 (REVIEW_synthesis):"
NOTE_MAX = 600
EXPECT_ROWS = 27
DROPPED = {"AI-SEENSTAYS"}  # review section 3: design behaviour, not a defect (documented in bug_candidates section 4 only)

# stale lessons: (scope, topic prefix, correction)
LESSON_UPDATES = [
    ("combat-ai", "Perception throttle timestamp never stored (review P1)",
     "P2 wording (E1): the 200 ms cadence goes to the IsPC creature (+0x10E8 = stats+0x6C, 005768f5..005768fe; the Exile, also while "
     "AI-driven), not to the controlled one; a player-controlled companion is also on 4 s."),
    ("combat-ai", "FM2 verified: pathfind failure is treated as success",
     "the door branch does NOT send 0x1E: 0x5C0732 is the trigger case; the door case posts OnBlocked (0x1F) at 0x5C1240 (lesson 309). "
     "'Unproven: fail vs partial' is settled by lesson 305 (partial 2 in most cases, FAIL on residual cases; fail-fast acts at 0x5BC5F4, "
     "consumed exit 0x5BC6EF)."),
    ("combat-ai", "FM3/FM5/FM6/FM7/T1/R1/P2 re-check",
     "FM5 has 6 unthrottled OnBlocked post sites, not 3 (lesson 309: 0x5C1240, 0x5C3D7D, 0x5C5A68, 0x5CCBB4, 0x5C8BB4, 0x5C9F8F); P2: only "
     "the IsPC creature gets the 200 ms cadence (see the lesson 230 update)."),
    ("party-ai", "`+0x11AC` = PC or CSWPartyTable member",
     "0x51C8F0 is the CONTROLLED LEADER, not the PC (0x51C8F0 -> 0x539D60 -> player 0x632A20 = GetGameObject(player+0x28); SetLeaderBySlot "
     "rewrites it at 0x6328D8), so 0x5FB070(id) || id == 0x51C8F0() misses the Exile while a companion is controlled. With only an id: "
     "resolve it (0x51C0A0) and test +0x11AC (conf med-high; probe PARTY settles it)."),
    ("party-ai-scripts", "medpac talent fails for Force-only classes",
     "E5: the Exile ALWAYS gets FALSE (every K2 PC class is in {3,4,5,11..16}; chargen offers 3/4/5, prestige classes are 11-16), not only "
     "a 'Jedi PC'. Party rule = GetIsTalentValid(tSpe11_05) && GN_PartyMedpacStock() >= 1; keep GetCommandable() (UseItem tests +0xEC, 0057d8e3)."),
]

# new lessons: (scope, topic, note)
NEW_LESSONS = [
    ("party-ai", "IsPC +0x10E8 is not the controlled creature",
     "+0x10E8 = IsPC = stats+0x6C (the Exile). Writers: ctor 0x562D7D (=0), load 0x5768FE (from stats), SwitchPlayerCharacter 0x5FE4B0 "
     "(0x5FE604/0x5FE951 =0, 0x5FEAE6 =1; script cmd 11), 0x628375 (=0). The UI leader switch SetLeaderBySlot 0x7E5DF0 never writes it, so "
     "IsPC stays on the Exile while a companion is controlled. Effects: the 200 ms perception cadence (P2) goes to IsPC, not the controlled "
     "one; ranged-ai E-H1/E-H2 '!isPC' also admits the controlled stance-16 companion: test id != controlled id. Anchor: REVIEW_synthesis E1."),
    ("party-ai", "0x51C8F0 returns the controlled leader; party test from an id",
     "UI leader switch SetLeaderBySlot 0x7E5DF0 -> 0x632830 (007e631d) stores player+0x28 at 0x6328D8. 0x51C8F0 (thiscall serverApp) -> "
     "0x539D60 -> player 0x632A20 = GetGameObject(player+0x28), i.e. the controlled leader, not the PC; 0x5FB070(id) || id == 0x51C8F0() "
     "misses the Exile while a companion is controlled. A correct party test resolves the id (0x51C0A0) and reads +0x11AC (pointer in hand: "
     "*(cr+0x11AC)). Conf med-high; probe PARTY settles it. Anchor: REVIEW_synthesis E2, LEAD_SPOTCHECKS."),
    ("combat-ai", "Shipped companion-ai assumptions changed by the atlas",
     "E2 0x569A08 sees slot-6 site 2 only (EndCombatRound 0x58F50F and SetState removal 0x59E904 also re-run k_ai_master): use probe RS for "
     "A1 FOLLOWUP/ORPHAN. A1 a1_retarget=1 picks via 0x57C990 (3D minus both radii) but the attack tests 2D centre dist2 vs (max+0.1)^2; "
     "InitializeNewAttackTarget sets +0x52C (006dacdb), so a too-far candidate ends silently at 0x6D83F6..0x6D83FF (ranged-ai E-H2 006d7d24 "
     "too). Plan: log-only probe A1X, then a1_retarget_strict (2D bound [ebp-0x2C]). Anchor: REVIEW_synthesis section 2/4."),
    ("party-ai", "Medpac rule: the Exile always fails HasTalent",
     "Every K2 PC class is in {3,4,5,11..16}, so GetCreatureHasTalent(tSpe11_05) is FALSE for the Exile always (plus Kreia, Visas): HasTalent "
     "0x6B8840 skips the item scan 0x6C0330 (006b89ce CALL 0x6F2A30 / 006b89d8 JZ). Party rule = GetIsTalentValid(tSpe11_05) && "
     "GN_PartyMedpacStock() >= 1, never HasTalent. ActionUseTalentOnObject -> UseItem 0x57D8B0 -> 0x57B4C0 needs no HasTalent but tests +0xEC "
     "(0057d8e3): keep GetCommandable(). Anchor: REVIEW_synthesis E5 / section 4."),
]

MEDPAC_ID = "AI-MEDPAC-FORCECLASS"
MEDPAC_OLD = "a Jedi-only PC"
MEDPAC_NEW = "the Exile (always)"
MEDPAC_TAG = "REVIEW_synthesis 2026-10-02 (E5)"
MEDPAC_EVIDENCE = (" " + MEDPAC_TAG + ": 'a Jedi-only PC' was understated, the Exile ALWAYS gets FALSE (every K2 PC class is in {3,4,5,11..16}: chargen "
                   "offers 3/4/5 only, prestige classes are 11-16), so the AI-driven Exile never uses medpacs through the HasTalent rule. Design: "
                   "GetIsTalentValid(tSpe11_05) && GN_PartyMedpacStock() >= 1, never HasTalent; keep GetCommandable() (UseItem tests +0xEC, "
                   "0057d8e3); medpac_guard_check must accept the second, party-gated tSpe11_05 (lesson 280). Evidence: " + EV + ".")


# --------------------------------------------------------------------------------------------------------------------
def split_row(line):
    cells = re.split(r"(?<!\\)\|", line.strip())
    return [c.strip().replace("\\|", "|") for c in cells[1:-1]]


def section(text, start_pat, end_pat):
    m = re.search(start_pat, text, re.M)
    assert m, "section not found: " + start_pat
    n = re.search(end_pat, text[m.end():], re.M)
    return text[m.end(): m.end() + n.start()] if n else text[m.end():]


def table_rows(sec, ncols, id_col):
    out = {}
    for line in sec.split("\n"):
        if not line.startswith("|"):
            continue
        c = split_row(line)
        if len(c) == ncols and c[id_col].startswith("AI-"):
            out[c[id_col]] = c
    return out


def risk_word(c):
    return {"L": "low", "M": "medium", "H": "high"}.get(c, "n/a (not ranked)")


def parse_candidates():
    text = open(BUGS, encoding="utf-8").read()
    ranked = table_rows(section(text, r"^## 1\. Ranked list", r"^## 2\."), 8, 1)
    fixes = table_rows(section(text, r"^## 3\. Fix, mod item", r"^## 4\."), 4, 0)
    rows5 = table_rows(section(text, r"^## 5\. DB candidates rows", r"^New ids"), 5, 0)
    assert len(rows5) == EXPECT_ROWS, "expected %d proposed rows, parsed %d" % (EXPECT_ROWS, len(rows5))
    assert not (set(rows5) & DROPPED), "dropped candidate still in section 5: %s" % (set(rows5) & DROPPED)
    out = []
    for cid, (_, title, status, hyp, anchor) in rows5.items():
        assert cid in ranked, "no section-1 row for " + cid
        assert cid in fixes, "no section-3 row for " + cid
        rk = ranked[cid]
        fx = fixes[cid]
        risk = "%s; fix: %s; owner: %s" % (risk_word(rk[4]), fx[1], fx[3])
        out.append((cid, title, "ai", hyp, anchor + " Evidence: engine_ai_atlas/bug_candidates.md (REVIEW_synthesis 2026-10-02 applied).",
                    rk[6], risk, status, None))
    return out


class Stats(dict):
    def __missing__(self, k):
        return 0

    def inc(self, k):
        self[k] = self[k] + 1


def do_candidates(cur, st):
    cols = [r[1] for r in cur.execute("pragma table_info(candidates)")]
    want = ["id", "title", "subsystem", "hypothesis", "evidence", "expected_win", "risk", "status", "decision_date"]
    assert cols == want, "candidates columns changed: %s" % cols
    for c in parse_candidates():
        old = cur.execute("select * from candidates where id=?", (c[0],)).fetchone()
        if old is None:
            cur.execute("insert into candidates values (?,?,?,?,?,?,?,?,?)", c)
            st.inc("cand_added")
        elif tuple(old) != tuple(c):
            cur.execute("update candidates set title=?,subsystem=?,hypothesis=?,evidence=?,expected_win=?,risk=?,status=?,"
                        "decision_date=? where id=?", c[1:] + (c[0],))
            st.inc("cand_updated")
        else:
            st.inc("cand_skipped")
    # existing row: AI-MEDPAC-FORCECLASS (E5)
    row = cur.execute("select title,hypothesis,evidence,expected_win,risk from candidates where id=?", (MEDPAC_ID,)).fetchone()
    if row is None:
        print("MISSING candidate " + MEDPAC_ID)
        st.inc("cand_missing")
        return
    title, hyp, ev, win, risk = row
    nhyp, nwin = hyp.replace(MEDPAC_OLD, MEDPAC_NEW), win.replace(MEDPAC_OLD, MEDPAC_NEW)
    nev = ev if MEDPAC_TAG in ev else ev.replace(MEDPAC_OLD, MEDPAC_NEW) + MEDPAC_EVIDENCE
    if (nhyp, nev, nwin) != (hyp, ev, win):
        cur.execute("update candidates set hypothesis=?,evidence=?,expected_win=? where id=?", (nhyp, nev, nwin, MEDPAC_ID))
        st.inc("cand_existing_updated")
        print("UPDATED candidate %s (E5)" % MEDPAC_ID)
    else:
        st.inc("cand_existing_skipped")


def do_lessons(cur, st):
    for scope, prefix, fix in LESSON_UPDATES:
        rows = cur.execute("select id,note,topic from lessons where scope=? and topic like ?", (scope, prefix + "%")).fetchall()
        if len(rows) != 1:
            print("MISSING/AMBIGUOUS lesson to update: %s / %s (%d rows)" % (scope, prefix, len(rows)))
            st.inc("les_missing")
            continue
        lid, note, _ = rows[0]
        if UPD_TAG in note:
            st.inc("les_upd_skipped")
            continue
        add = UPD_TAG + " " + fix
        cur.execute("update lessons set note=?, date=? where id=?", (note + add, DATE, lid))
        st.inc("les_upd_applied")
        print("UPDATED lesson %d (%s / %s): +%d chars (now %d)" % (lid, scope, rows[0][2][:50], len(add), len(note) + len(add)))
    for scope, topic, note in NEW_LESSONS:
        if len(note) + len(EV_SUFFIX) <= NOTE_MAX:
            note += EV_SUFFIX
        assert len(note) <= NOTE_MAX, "lesson '%s' is %d chars > %d" % (topic, len(note), NOTE_MAX)
        row = cur.execute("select id,note from lessons where scope=? and topic=?", (scope, topic)).fetchone()
        if row is None:
            cur.execute("insert into lessons (scope,topic,note,date) values (?,?,?,?)", (scope, topic, note, DATE))
            st.inc("les_added")
        elif row[1] != note:
            cur.execute("update lessons set note=?, date=? where id=?", (note, DATE, row[0]))
            st.inc("les_updated")
        else:
            st.inc("les_skipped")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--db", default=DEFAULT_DB, help="knowledge DB (default: the real DB next to the repo)")
    ap.add_argument("--dry-run", action="store_true", help="report what would change; roll everything back")
    args = ap.parse_args()
    db = os.path.abspath(args.db)
    if not os.path.exists(db):
        sys.exit("DB not found: " + db)
    print("DB: %s%s" % (db, "  [DRY RUN, nothing written]" if args.dry_run else ""))
    conn = sqlite3.connect(db)
    conn.isolation_level = None  # explicit transaction so --dry-run can roll back
    cur = conn.cursor()
    cur.execute("BEGIN")
    st = Stats()
    try:
        do_candidates(cur, st)
        do_lessons(cur, st)
    except Exception:
        cur.execute("ROLLBACK")
        raise
    cur.execute("ROLLBACK" if args.dry_run else "COMMIT")
    print("\n=== SUMMARY%s (%s) ===" % (" (dry run)" if args.dry_run else "", SRC))
    print("candidates : added %d | updated %d | unchanged %d | existing rows updated %d (unchanged %d, missing %d)   [parsed %d, dropped %s]" % (
        st["cand_added"], st["cand_updated"], st["cand_skipped"], st["cand_existing_updated"], st["cand_existing_skipped"],
        st["cand_missing"], EXPECT_ROWS, ",".join(sorted(DROPPED))))
    print("lessons    : added %d | updated %d | unchanged %d   | older lessons: UPDATED-appended %d, already-appended %d, missing %d" % (
        st["les_added"], st["les_updated"], st["les_skipped"], st["les_upd_applied"], st["les_upd_skipped"], st["les_missing"]))
    print("name_map   : no renames in the review, nothing written")
    chk = sqlite3.connect(db)
    print("integrity_check:", chk.execute("pragma integrity_check").fetchone()[0])
    chk.close()


if __name__ == "__main__":
    main()
