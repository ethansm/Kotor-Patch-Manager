#!/usr/bin/env python3
"""Persist the engine-AI atlas wave 2 (AI5-AI8 + Opus gate, 2026-10-02) into
ghidra_knowledge/kotor2_ghidra_knowledge.db.  Python 3, stdlib only.

Sources of truth: engine_ai_atlas/GATE_wave2.md (sections 3, 4, 6, 6a; it OVERRIDES the batches) and the (e) struct
field tables of engine_ai_atlas/findings/AI5.md .. AI8.md.  The gate's name_map tables (section 4) and lesson list
(section 6) are parsed from GATE_wave2.md at run time (counts are asserted: 15 corrections, 34 new rows, 12 lessons).

Idempotent:
  * name_map: gate corrections update an existing row once (skipped when the row already has the new name and SRC);
    new rows are inserted only where steam_addr is absent (a differing existing name = CONFLICT line, nothing written).
  * lessons: keyed by (scope, topic), note updated in place.  Superseded older lessons (277 FM6, 280 medpac) get one
    " UPDATED 2026-10-02 (GATE_wave2): ..." sentence appended (skipped when already present); nothing is deleted.
  * struct_fields: CREATE TABLE IF NOT EXISTS; insert if missing; update only when the new confidence rank >= the old
    one and something changed (readers/writers are merged as unions, never dropped).
  * candidates: keyed by id, updated in place.
Everything runs in one transaction; --dry-run rolls it back.
"""
import argparse, os, re, sqlite3, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ATLAS = os.path.normpath(os.path.join(HERE, ".."))
DEFAULT_DB = os.path.normpath(os.path.join(ATLAS, "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db"))
FINDINGS = os.path.join(ATLAS, "findings")
GATE = os.path.join(ATLAS, "GATE_wave2.md")

DATE = "2026-10-02"
SRC = "engine-ai-atlas wave2"
EV = "engine_ai_atlas/GATE_wave2.md"
EV_SUFFIX = " Evidence: engine_ai_atlas/GATE_wave2.md"
UPD_TAG = " UPDATED 2026-10-02 (GATE_wave2):"
NOTE_MAX = 600
SAME = "(same)"

# --------------------------------------------------------------------------------------------------------------------
# older lessons superseded by the gate: (scope, topic, one-line correction)
LESSON_UPDATES = [
    ("combat-ai", "FM3/FM5/FM6/FM7/T1/R1/P2 re-check (2026-10-02)",
     "FM6 consumer is now known: the state-5 follow handler 0x5CA6A0 reads the per-slot hold flag E+0x70 (set at 0x7E6546/56/66, "
     "cleared 0x5CA802) and holds within 7 m (49.0 at [0x996520]); FM6 is partly refuted (AI5 FM6 vs this lesson, GATE_wave2 sec 2)."),
    ("party-ai-scripts", "Counting party medpacs from a script",
     "the implication holds only one way: GetCreatureHasTalent(tSpe11_05) is FALSE for Force-only class lists (Kreia class 4, Visas 5, "
     "a Jedi-only PC) even with stock (HasTalent 0x6B8840 skips the item scan 0x6C0330 for IsForceUserClass 0x6F2A30 = {3,4,5,11..16}); "
     "R4 row 4 'verified' is wrong for them. Use GetIsTalentValid + stock >= 1 (see lesson 'medpac talent fails for Force-only classes')."),
]

# gate section 6a: (id, title, subsystem, hypothesis, evidence, expected_win, risk, status, decision_date)
CANDIDATES = [
    ("AI-REPTRANSPOSE", "AdjustReputation reads the transposed matrix cell", "ai",
     "AdjustReputation 0x5D0DF0 (this = oTarget T, p2 = faction of oSource S; script cmd 209 0x66D1B0) calls Get 0x664580(own, p2) = "
     "data[b*N+a-1] (006645f2 IMUL ECX,[EAX+0x1c] on [EBP+0xc]) and Set 0x6644D0(own, p2, v) = data[row*N+col-1] (00664534 IMUL on "
     "[EBP+0x8]) with the same argument order, so the written cell (row fT, col fS = rep S->T) gets M(T->S) + delta. Wrong only for "
     "asymmetric NPC-faction pairs (PREDATOR 11 / PREY 12, GIZKA_1/2). A PC-faction target (the PC and every SpawnNPC companion) is "
     "unaffected: Get(0,fS) recurses to Get(fS,0). Skipped when fT == fS or fS is not an NPC faction (0x664650).",
     "Static, gate-verified (Opus): " + EV + " section 1.3 and lesson 'AdjustReputation reads the transposed cell'; AI6 Q2. Not reached "
     "in vanilla: the 638-file source corpus only has unused_sandper.nss (oTarget = party leader, PC faction); module-embedded NCS not "
     "scanned. Fix = swap the two pushes at 0x5D0E40 (8 B simple patch: 8B 55 08 52 8B 45 E8 50 -> 8B 45 E8 50 8B 55 08 52). Low priority.",
     "correct baseline for mod/module scripts that adjust NPC-vs-NPC asymmetric faction reputation (none in vanilla)",
     "low; behaviour changes only for asymmetric NPC rows",
     "confirmed-static", None),
    ("AI-SHOUTABORT", "Shout loop drops every listener on an unresolvable id", "ai",
     "Shout delivery 0x547100 ignores the sign of the 0x522AF0 start index: 005471ac CALL 0x522af0 / 005471b1 CMP [EBP-0x1c],0 / "
     "005471b5 JL 0x5475e8 drops all listeners when the X-bisection meets an unresolvable id (stale id in area+0x194), and 00547203 JNZ "
     "0x5475e8 drops the rest of the scan. The same 0x522AF0 serves UpdatePerception modes 0/2, GetNearestCreature 0x683D40, "
     "GetFirstObjectInShape 0x685960 and Surrender, so a stale id would also blind perception and scripts in that area.",
     "Static, gate-verified: " + EV + " section 1.4 (code CONFIRMED, reach LOW: nothing like a stale id has been observed). Fix idea: "
     "probe F4 0x547100 + F5 0x57074F first; if seen, retarget 00547203 JNZ to the loop head (0x5471B1) and seed idx 0 when 0x522AF0 fails.",
     "E9 shouts no longer lost silently in an area with a stale id",
     "low-medium (skipping stale ids)",
     "confirmed-static (code; reach low)", None),
    ("AI-FM4B", "SaveCreature sets non-party creatures to AI level 2 on each save", "ai",
     "SaveCreature calls SetPartyMemberFlag 0x58A330 twice: 0x574532 (0,1) and 0x57516B (old,1). For old == 0 (non-party) call 1 takes "
     "SetAILevel(2) (call 2 is a no-op): one level churn per creature per save (its next AIUpdate demotes it to 0/1 via 0x56A0FE/0x56A111) "
     "and script-set levels 3/4 (SetCreatureAILevel, StartConversation) are lost. For old == 1 (party) call 1 frees FollowInfo and sets 2, "
     "call 2 allocates a default FollowInfo and sets 4. Every save reaches it via SaveGIT -> CSWSArea::SaveCreatures 0x52A600 + the 8 "
     "party-table NPC save sites + 0x5FE4B0.",
     "Static, gate-verified: " + EV + " section 1.2 and lesson 'SaveCreature sets non-party creatures to AI level 2 on every save'; AI5 "
     "sec 8. Fix = byte patches 0x574529 (patch A) AND 0x575160 (patch B), imm8 01->00, TOGETHER; the save format is unchanged "
     "(FollowInfo saved live). B alone is harmful: call 1 frees FollowInfo, call 2 (old,0) never reallocates it, so 0x3D returns 3 "
     "(005bf81b JZ) until the next party change. With A+B companion-ai FM4 mode 1 becomes redundant (harmless). Not yet tested in game.",
     "no per-save AI-level churn; script-set levels 3/4 survive a save",
     "low; save format unchanged. Ship A and B together or neither (user decision, effect is small)",
     "confirmed-static", None),
    ("AI-MEDPAC-FORCECLASS", "GetCreatureHasTalent(item spell) is FALSE for Force-only classes", "ai",
     "HasTalent 0x6B8840 (type 0) loops the class slots; for a Force class ({3,4,5,11..16} = IsForceUserClass 0x6F2A30 = classes.2da "
     "rows with forcedie > 0) it tests only known spell + payable cost (0x6BD9C0 && 0x6B8F40); only non-Force slots reach the item "
     "scan 0x6C0330 (006b8a6c). Kreia (class 4 only), Visas (5 only) and a Jedi-only PC therefore get FALSE for "
     "GetCreatureHasTalent(medpac, category 0x1408) even with medpacs in stock, while GetCreatureTalentBest(0x1408,20) still returns a "
     "medpac talent for everyone (the repository pass has no class test). The R4 guard and vanilla NPC medpac use never fire for them.",
     "Static, gate-verified: " + EV + " section 1.5 (contradiction with R4 / lesson 280 settled for AI7; companion class lists read from "
     "the UTC files, classes.2da from the replay copies). Fix = script only: party branch uses GetIsTalentValid(tSpe11_05) + "
     "GN_PartyMedpacStock() >= 1 instead of GetCreatureHasTalent; ActionUseTalentOnObject with the item fields goes straight to "
     "UseItem (0x57D8B0 -> 0x57B4C0) and needs no HasTalent. Failure today is safe (no idle loop).",
     "Kreia, Visas and a Jedi-only PC use medpacs under the R4 rule",
     "low (script-only; UseTalent bypasses HasTalent)",
     "confirmed-static", None),
    ("AI-TALENTEXCL", "TalentBest exclusion flags set the wrong group", "ai",
     "GetCreatureTalentBest/Random (0x6B8BB0) ignores nCRMax (only compared with -1; 0x6BA970 never reads arg 4) and never applies "
     "nExcludeId to spells (006bb07e PUSH -1). nExcludeType (0,-1) still runs the spell group once, and (1,-1) marks the SPELL group done "
     "(006b8d2f), not the feat group; the AI7 reading 'drops the feat group' is REFUTED.",
     "Static, gate-verified: " + EV + " section 1 (AI7 table, nCRMax / nExcludeId / exclusion rows). Our party AI scripts never pass "
     "exclusion arguments (grep: 0 uses); module-embedded NCS not scanned.",
     "document only: do not rely on nExcludeType / nExcludeId",
     "none",
     "confirmed-static (no current use)", None),
]

# --------------------------------------------------------------------------------------------------------------------
STRUCT_DDL = """CREATE TABLE IF NOT EXISTS struct_fields(
  class TEXT NOT NULL,          -- 'CSWSCreature', 'CSWSCombatRound', 'CPathfindInformation', 'CAction' (0x74 node), ...
  offset INTEGER NOT NULL,      -- byte offset from the class base (store int; print as hex)
  size INTEGER,                 -- bytes (1/2/4/8/N); NULL if unknown
  type TEXT,                    -- 'int', 'float', 'objid', 'ptr CSWSCombatRound', 'CExoArrayList<objid>', ...
  name TEXT,                    -- proposed name
  confidence TEXT,              -- high (quoted asm reader/writer) / medium / low
  evidence TEXT,                -- one quoted line with address, or the findings file reference
  readers TEXT,                 -- space-separated hex instruction addresses (or fn addresses)
  writers TEXT,
  source TEXT,                  -- 'engine-ai-atlas AI<n>' / 'review_ai_engine' / ...
  date TEXT,
  PRIMARY KEY(class, offset)
)"""

RANK = {"low": 1, "medium": 2, "med": 2, "high": 3}
# batch class label -> DB class (a list = the row applies to each class)
CLASS_MAP = {"E": "CSWPartySlot",  # CSWParty slot element (stride 0x88); not to be confused with CFollowInfo
             "stats": "CSWSCreatureStats", "AOE": "CSWSAreaOfEffectObject", "placeable": "CSWSPlaceable",
             "encounter": "CSWSEncounter", "trigger, door": ["CSWSTrigger", "CSWSDoor"],
             "perception entry": "PerceptionEntry", "server internal": "CServerExoAppInternal", "global": "GlobalData"}
# class-key fixes applied to a parsed row: (class, offset) -> class.  0xEC = gate names CSWSCreature (AI8 said CSWSObject);
# script slots at +0x270+8n are CSWSCreature (the AoE object has its own fields from 0x270, so CSWSObject ends below it).
CLASS_KEY_REMAP = {("CSWSObject", 0xEC): "CSWSCreature", ("CSWSObject", 0x270): "CSWSCreature"}
NAME_FIX = {
    ("CSWParty", 0xE4): "slot-1 formation offset (= slot element +0x38)",
    ("CSWParty", 0x16C): "slot-2 formation offset (= slot element +0x38)",
    ("CSWParty", 0x2C0): "smoothed leader point",
    ("CSWParty", 0x2CC): "smoothed leader orientation",
    ("CSWSArea", 0x194): "objectIds (all-objects id array sorted by X)",   # wave-1 names kept
    ("CSWSArea", 0x198): "objectCount (all-objects id array count)",
    ("CSWSCreature", 0x1A4): "last shout event int(2)",
    ("CSWSCreature", 0x1A8): "last shout event string(0)",
}
NOINFO = {"not traced", "(not traced)", "not traced here", "(not found)", "n/a", "?", "-", ""}


def norm_conf(c):
    m = re.match(r"\s*([A-Za-z]+)", c or "")
    c = (m.group(1) if m else "").lower()
    return "medium" if c == "med" else c


def hexaddr(a):
    return "0x%08x" % a


def clean(s):
    return (s or "").replace("`", "").strip()


# ---- struct table parsing ------------------------------------------------------------------------------------------
HEXTOK = re.compile(r"^\+?(?:0x)?([0-9A-Fa-f]+)$")
ADDR_RE = re.compile(r"0x[0-9A-Fa-f]{4,}(?![0-9A-Fa-fx])|\b[0-9a-f]{8}\b|\b(?=[0-9A-F]*\d)[0-9A-F]{6}\b")


def split_row(line):
    line = line.replace("\\|", "\x00")
    return [c.strip().replace("\x00", "|") for c in line.strip().strip("|").split("|")]


def split_top(s, sep):
    """re.split on sep, but only outside parentheses."""
    masked = list(s)
    depth = 0
    for i, ch in enumerate(s):
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth = max(0, depth - 1)
        if depth:
            masked[i] = "\x01"
    masked = "".join(masked)
    out, pos = [], 0
    for m in re.finditer(sep, masked):
        out.append(s[pos:m.start()])
        pos = m.end()
    out.append(s[pos:])
    return [x.strip() for x in out]


def split_parts(s, n, seps):
    for sep in seps:
        p = split_top(s, sep)
        if len(p) == n:
            return p
    return None


def parse_offsets(cell):
    """-> list of (offset:int, note:str)."""
    cell = clean(cell)
    cell = re.sub(r"\s*\([^)]*\)", "", cell)  # drop '(n=0..13)'
    out = []
    for grp in re.split(r"\s+/\s+", cell):
        grp = grp.strip()
        if not grp:
            continue
        if ".." in grp or "*" in grp:
            m = re.match(r"^\+?(?:0x)?([0-9A-Fa-f]+)", grp)
            if not m:
                raise ValueError("offset cell %r" % cell)
            out.append((int(m.group(1), 16), ("stride " if "*" in grp else "range ") + grp))
            continue
        if "," in grp:
            parts = [p.strip() for p in grp.split(",")]
            m = HEXTOK.match(parts[0])
            out.append((int(m.group(1), 16), "also " + ",".join(parts[1:])))
            continue
        subs = [p.strip() for p in grp.split("/")]
        if len(subs) > 1 and all(HEXTOK.match(p) for p in subs):
            out.extend((int(HEXTOK.match(p).group(1), 16), "") for p in subs)
            continue
        m = re.match(r"^\+?(?:0x)?([0-9A-Fa-f]+)", grp)
        if not m:
            raise ValueError("offset cell %r" % cell)
        out.append((int(m.group(1), 16), ""))
    return out


def parse_size(s):
    s = clean(s)
    s = re.sub(r"\s*\([^)]*\)", "", s)  # '12 (+4)'
    if not s or s == "-":
        return None
    num = r"(0x[0-9A-Fa-f]+|\d+)"
    m = re.match(r"^%s\s+x\s*%s$" % (num, num), s)
    if m:
        return int(m.group(1), 0) * int(m.group(2), 0)
    m = re.match(r"^%s\s*\+\s*%s$" % (num, num), s)
    if m:
        return int(m.group(1), 0) + int(m.group(2), 0)
    m = re.match(r"^%s\*%s$" % (num, num), s)
    if m:
        return int(m.group(1), 0) * int(m.group(2), 0)
    m = re.match(r"^%s(\s+each)?$" % num, s)
    if m:
        return int(m.group(1), 0)
    return None


def addr_cell(cell):
    """readers/writers cell -> normalised addresses, or the raw text if it carries other information, or None."""
    raw = clean(cell)
    if raw.lower() in NOINFO:
        return None
    addrs = []
    for t in ADDR_RE.findall(raw):
        t = t.lower()
        a = int(t[2:], 16) if t.startswith("0x") else int(t, 16)
        h = hexaddr(a)
        if h not in addrs:
            addrs.append(h)
    return " ".join(addrs) if addrs else raw


def parse_findings_e(batch):
    txt = open(os.path.join(FINDINGS, "AI%d.md" % batch), encoding="utf-8").read().split("\n")
    start = next(i for i, l in enumerate(txt) if l.startswith("## (e)"))
    end = next((i for i in range(start + 1, len(txt)) if txt[i].startswith("## ")), len(txt))
    rows, warnings = [], []
    for l in txt[start:end]:
        if not l.startswith("|"):
            continue
        cells = split_row(l)
        if cells[0].lower() == "class" or set(cells[0]) <= set("-: "):
            continue
        if len(cells) != 9:
            warnings.append("AI%d: row with %d cells skipped: %s" % (batch, len(cells), l[:100]))
            continue
        rows.append(cells)
    return rows, warnings


def expand(batch, cells):
    cls_raw, off, size, typ, name, rd, wr, conf, ev = cells
    cls_raw = re.sub(r"\s*\(.*\)$", "", cls_raw.strip())
    cls_val = CLASS_MAP.get(cls_raw, cls_raw)
    classes = cls_val if isinstance(cls_val, list) else [cls_val]
    offs = parse_offsets(off)
    name_c = clean(name)
    n = len(offs)
    size_p = [parse_size(x) for x in split_top(clean(size), r"\s*/\s*")] if clean(size) else []
    names = split_parts(name_c, n, [r"\s+/\s+", r",\s*"]) if n > 1 else None
    split = n > 1 and (names is not None or len(size_p) == n)
    # confidence cell: 'high' | 'high / med' | 'high (1,2,4) / med (8,0x10)'
    conf_cells = split_top(clean(conf), r"\s+/\s+")
    confs = [norm_conf(c) for c in conf_cells]
    ev_c = clean(ev)
    if any("(" in c for c in conf_cells) or (len(confs) > 1 and not (split and len(confs) == n)):
        ev_c += " [batch confidence cell: %s]" % clean(conf)
    types = split_parts(clean(typ), n, [r"\s*/\s*", r",\s*"]) if (split and clean(typ)) else None
    rds = split_parts(clean(rd), n, [r"\s+/\s+"]) if split else None
    wrs = split_parts(clean(wr), n, [r"\s+/\s+"]) if split else None
    out = []
    for cls in classes:
        if not split:
            o, note = offs[0]
            nm = name_c + (" [%s]" % note if note else "")
            if n > 1:
                nm += " [offsets " + "/".join("0x%X" % x for x, _ in offs) + "]"
            if "+8n" in off:
                nm += " [stride 8, n=0..13]"
            out.append(dict(cls=cls, off=o, size=(size_p[0] if len(size_p) == 1 else parse_size(size)),
                            type=clean(typ) or None, name=nm, conf=confs[0], ev=ev_c, rd=addr_cell(rd), wr=addr_cell(wr),
                            batch=batch))
            continue
        for i, (o, note) in enumerate(offs):
            nm = names[i] if names else "%s [+0x%X]" % (name_c, o)
            if note:
                nm += " [%s]" % note
            sz = size_p[i] if len(size_p) == n else (size_p[0] if len(size_p) == 1 else None)
            e = ev_c
            r = addr_cell(rds[i]) if rds else (addr_cell(rd) if i == 0 else None)
            w = addr_cell(wrs[i]) if wrs else (addr_cell(wr) if i == 0 else None)
            if not (rds and wrs) and i == 0:
                e += " [grouped row: shared readers/writers cell kept on the first offset]"
            out.append(dict(cls=cls, off=o, size=sz, type=(types[i] if types else (clean(typ) or None)), name=nm,
                            conf=(confs[i] if len(confs) == n else confs[0]), ev=e, rd=r, wr=w, batch=batch))
    return out


def merge_lists(a, b):
    """union of two readers/writers values; address lists win over free text."""
    if not a or not b:
        return a or b
    ia, ib = all(ADDR_RE.fullmatch(t) for t in a.split()), all(ADDR_RE.fullmatch(t) for t in b.split())
    if ia and ib:
        return " ".join(a.split() + [t for t in b.split() if t not in a.split()])
    if ia != ib:
        return a if ia else b
    return a if b in a else a + " | " + b


def build_struct_rows(log):
    rows, notes = {}, []
    for batch in (5, 6, 7, 8):
        cells_list, warns = parse_findings_e(batch)
        for w in warns:
            notes.append("WARNING " + w)
        for cells in cells_list:
            try:
                parts = expand(batch, cells)
            except Exception as e:  # report, never silently drop
                notes.append("WARNING AI%d: could not transcribe row %s (%s)" % (batch, cells[:2], e))
                continue
            for r in parts:
                new_cls = CLASS_KEY_REMAP.get((r["cls"], r["off"]))
                if new_cls:
                    notes.append("REMAP AI%d %s +0x%X -> %s" % (batch, r["cls"], r["off"], new_cls))
                    r["cls"] = new_cls
                if (r["cls"], r["off"]) in NAME_FIX:
                    r["name"] = NAME_FIX[(r["cls"], r["off"])]
                key = (r["cls"], r["off"])
                if key in rows:
                    old = rows[key]
                    new_better = RANK[r["conf"]] >= RANK[old["conf"]]  # tie -> the later batch
                    win, lose = (r, old) if new_better else (old, r)
                    merged = dict(win)
                    merged["rd"] = merge_lists(old["rd"], r["rd"])
                    merged["wr"] = merge_lists(old["wr"], r["wr"])
                    if clean(lose["name"]) != clean(win["name"]):
                        merged["ev"] = (win["ev"] + " | AI%d name: %s" % (lose["batch"], lose["name"])).strip(" |")
                    if merged["size"] is None:
                        merged["size"] = lose["size"]
                    if not merged["type"]:
                        merged["type"] = lose["type"]
                    notes.append("MERGE %s +0x%X: AI%d(%s) + AI%d(%s) -> keep AI%d '%s' (%s)" % (
                        key[0], key[1], old["batch"], old["conf"], r["batch"], r["conf"], win["batch"], win["name"], win["conf"]))
                    rows[key] = merged
                else:
                    rows[key] = r
    apply_struct_overrides(rows, notes)
    return rows, notes


def set_row(rows, notes, cls, off, batch, **kw):
    key = (cls, off)
    r = rows.get(key)
    if r is None:
        r = dict(cls=cls, off=off, size=None, type=None, name="", conf="low", ev="", rd=None, wr=None, batch=batch)
        rows[key] = r
        notes.append("ADD (gate/override) %s +0x%X" % (cls, off))
    for k, v in kw.items():
        if k == "ev_add":
            r["ev"] = (r["ev"] + " | " + v).strip(" |") if r["ev"] else v
        else:
            r[k] = v
            if k == "name":
                r["force"] = True  # gate/override name: replaces an existing same-confidence name
    return r


def apply_struct_overrides(rows, notes):
    G = "GATE_wave2: "
    # +0x11AC (AI5 high, AI6 high, AI7 high): gate 1.1
    set_row(rows, notes, "CSWSCreature", 0x11AC, 5, conf="high", size=4, type="int",
            name="PartyMemberFlag (1 = PC or a current CSWPartyTable member, plus the effect-89 creature; = IsObjectPartyMember)",
            ev_add=G + "1.1: writers only ctor 0x56214C and 0x58A35E (SetPartyMemberFlag 0x58A330, 15 sites); never set by SpawnNPC "
                   "0x5FD480 (Hawk standees = 0); transiently 0 inside SaveCreature 0x574532..0x57516B and TransferInventory")
    # +0xEC commandable: gate 1.7 (AI8 + AI7 + dead writer removed)
    r = rows.get(("CSWSCreature", 0xEC))
    if r is not None and r["wr"]:
        toks = [t for t in r["wr"].split() if t.lower() != "0x00596a7a"]
        if len(toks) != len(r["wr"].split()):
            r["wr"] = " ".join(toks)
            notes.append("REMOVED dead writer 0x596A7A from CSWSCreature +0xEC")
    set_row(rows, notes, "CSWSCreature", 0xEC, 8, conf="high", size=4, type="int",
            name="commandable (ClearAllActions 0x541080 is a full no-op unless == 1; most Add*/Action* handlers refuse while 0; "
                 "ActionMoveToLocation 0x68CEE0 does NOT test it)",
            ev_add=G + "1.7: `005410e9 CMP [EAX+0xEC],1` / `005410f0 JNZ 0x541385`; writer 0x596A7A is dead code (`005969cb XOR EDX,EDX` / "
                   "`005969cd JZ 0x00596b74`); live clearers: SetState apply 0x59DEF8 (not state 9), 0x56BB0D/0x56BC3D, 0x59BC27, "
                   "0x59DD65, SetCommandable 0x691004; class per gate (AI8 batch said CSWSObject)")
    # +0x1114 allowed-action mask: gate 1.7, AI5 'unknown' resolved
    set_row(rows, notes, "CSWSCreature", 0x1114, 8, conf="high", size=2, type="u16",
            name="allowed-action mask (init 0xFFFF, death 0; bit 0x2 movement, 0x4+0x80 attack, 0x8 spells in HasTalent, 0x40 client awake)",
            ev_add=G + "1.7/lesson 10: `0059c409 MOV [EDX+0x1114],AX` (type-0x17 effect AND, 0x59C3B0); AI5 'bit1 unknown' resolved by AI8 Q1: "
                   "0x2 = movement (0xA 0x5BC796, 0x3D 0x5BF89E, 0x46, CounterSpell, MoveToPoint only in the 0x5C172F object-type case 6/8)")
    # creature -> FollowInfo pointer
    set_row(rows, notes, "CSWSCreature", 0x500, 5, type="ptr CFollowInfo", ev_add=G + "owner of the CFollowInfo rows (new(0x3C) + ctor 0x5752C0)")
    # the CSWParty table pointer inside the server internal object (gate sec 4 / 1.1)
    set_row(rows, notes, "CServerExoAppInternal", 0x1F0B4, 5, size=None, type="CSWPartyTable (inline object)",
            name="partyTable (inline CSWPartyTable; CServerExoApp::GetPartyTable 0x51C8D0 returns this+0x1F0B4)", conf="high",
            ev="`0051c8da MOV EAX,[EAX+0x4]` / `0051c8dd ADD EAX,0x1f0b4` (inline object, no deref) [GATE_wave2 1.1]",
            rd="0x0051c8d0", wr=None)
    # gate 'struct fields worth adding': endorsed rows are high
    for key in [("CSWSCreature", 0xFF8), ("CSWSCreature", 0x1A0), ("CSWSObject", 0x1BC), ("CSWSObject", 0x1C0),
                ("CFollowInfo", 0x38), ("CSWPartySlot", 0x70), ("CSWPartyTable", 0x00), ("CSWPartyTable", 0x1C),
                ("CSWPartyTable", 0x114), ("CFactionManager", 0x1C), ("CFactionManager", 0x20)]:
        if key not in rows:
            notes.append("WARNING gate-endorsed row %s +0x%X not found in the batches" % key)
            continue
        set_row(rows, notes, key[0], key[1], rows[key]["batch"], conf="high", ev_add=G + "sec 4 'struct fields worth adding' (endorsed, high)")
    # AI8 +0xFF5: gate-1 name kept in the DB (AI3 medium); AI8 high with the same meaning
    # AI6 0x1F31C/0x1F320 stay low (gate sec 3)
    for off in (0x1F31C, 0x1F320):
        r = rows.get(("CServerExoAppInternal", off))
        if r is not None:
            r["conf"] = "low"
            r["ev"] = (r["ev"] + " | " + G + "sec 3: low").strip(" |")


# ---- gate table parsing --------------------------------------------------------------------------------------------
def gate_section(text, start_pat, end_pat):
    lines = text.split("\n")
    s = next(i for i, l in enumerate(lines) if l.startswith(start_pat))
    e = next(i for i in range(s + 1, len(lines)) if lines[i].startswith(end_pat))
    return lines[s:e]


def parse_gate_tables():
    text = open(GATE, encoding="utf-8").read()
    corrections, new_rows = [], []
    for l in gate_section(text, "## 4.", "## 5."):
        if not re.match(r"\|\s*0x[0-9a-fA-F]{8}\s*\|", l):
            continue
        c = split_row(l)
        addr = int(c[0], 16)
        if len(c) == 5:  # addr | current name (conf) | new name | conf | evidence
            corrections.append((addr, c[2], norm_conf(c[3]), c[4], c[1]))
        elif len(c) == 4:
            new_rows.append((addr, c[1], norm_conf(c[2]), c[3]))
        else:
            raise ValueError("gate section 4 row with %d cells: %s" % (len(c), l[:80]))
    assert len(corrections) == 15, len(corrections)
    assert len(new_rows) == 34, len(new_rows)
    lessons = []
    for l in gate_section(text, "## 6. Lessons", "## 6a"):
        m = re.match(r"^(\d+)\. (.+?) / (.+?) / (.*)$", l)
        if m:
            lessons.append((m.group(2).strip(), m.group(3).strip(), m.group(4).strip()))
    assert len(lessons) == 12, len(lessons)
    return corrections, new_rows, lessons


# ---- DB operations -------------------------------------------------------------------------------------------------
class Stats(dict):
    def __missing__(self, k):
        return 0

    def inc(self, k, n=1):
        self[k] = self[k] + n


def do_name_map(cur, st, corrections, new_rows):
    def get(a):
        return cur.execute("select steam_addr,name,source,confidence,evidence,date from name_map where steam_addr=?",
                           (hexaddr(a),)).fetchone()

    for a, new_name, conf, ev, cur_cell in corrections:
        row = get(a)
        if row is None:
            print("MISSING correction target %s: inserting as new row" % hexaddr(a))
            cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)",
                        (hexaddr(a), new_name if new_name != SAME else "(unnamed)", SRC, conf, "%s section 4; %s" % (EV, ev), DATE))
            st.inc("nm_added")
            continue
        _, old_name, old_src, old_conf, old_ev, _ = row
        if new_name == SAME:
            new_name = old_name
        if old_name == new_name and old_src == SRC:
            st.inc("nm_skipped")
            continue
        if old_name == new_name:
            new_ev = "UPGRADED 2026-10-02 (GATE_wave2): name confirmed, was (%s, %s). %s [%s s4]" % (old_src, old_conf, ev, EV)
        else:
            new_ev = "RENAMED 2026-10-02 (GATE_wave2): was '%s' (%s, %s). %s [%s s4]" % (old_name, old_src, old_conf, ev, EV)
        cur.execute("update name_map set name=?, confidence=?, source=?, evidence=?, date=? where steam_addr=?",
                    (new_name, conf, SRC, new_ev, DATE, hexaddr(a)))
        st.inc("nm_renamed")
        print("%s %s '%s' -> '%s' (%s -> %s)" % ("UPGRADE" if old_name == new_name else "RENAME", hexaddr(a), old_name, new_name,
                                                 old_conf, conf))

    for a, name, conf, ev in new_rows:
        row = get(a)
        if row is None:
            cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)",
                        (hexaddr(a), name, SRC, conf, "%s section 4; %s" % (EV, ev), DATE))
            st.inc("nm_added")
        elif row[1] == name:
            st.inc("nm_skipped")
        else:
            print("CONFLICT %s: existing '%s' (%s, %s) vs gate '%s' (%s) -> not overwritten" % (
                hexaddr(a), row[1], row[2], row[3], name, conf))
            st.inc("nm_skipped")
            st.inc("nm_conflict")


def do_lessons(cur, st, lessons):
    for topic_scope, topic, note in lessons:
        scope = topic_scope
        if len(note) > NOTE_MAX:
            print("WARNING lesson '%s': note %d chars > %d, truncated" % (topic, len(note), NOTE_MAX))
            note = note[:NOTE_MAX]
        elif len(note) + len(EV_SUFFIX) <= NOTE_MAX:
            note += EV_SUFFIX
        else:
            print("note: lesson '%s' %d chars, no room for the evidence suffix" % (topic, len(note)))
        row = cur.execute("select id,note from lessons where scope=? and topic=?", (scope, topic)).fetchone()
        if row is None:
            cur.execute("insert into lessons (scope,topic,note,date) values (?,?,?,?)", (scope, topic, note, DATE))
            st.inc("les_added")
        elif row[1] != note:
            cur.execute("update lessons set note=?, date=? where id=?", (note, DATE, row[0]))
            st.inc("les_updated")
        else:
            st.inc("les_skipped")

    for scope, topic, fix in LESSON_UPDATES:
        row = cur.execute("select id,note from lessons where scope=? and topic=?", (scope, topic)).fetchone()
        if row is None:
            print("MISSING lesson to update: %s / %s" % (scope, topic))
            st.inc("les_missing")
        elif UPD_TAG in row[1]:
            st.inc("les_upd_skipped")
        else:
            cur.execute("update lessons set note=?, date=? where id=?", (row[1] + UPD_TAG + " " + fix, DATE, row[0]))
            st.inc("les_upd_applied")
            print("UPDATED lesson %d (%s / %s): +%d chars (now %d)" % (row[0], scope, topic, len(UPD_TAG + " " + fix),
                                                                     len(row[1]) + len(UPD_TAG + " " + fix)))


def do_struct(cur, rows, st, per_class):
    cur.execute(STRUCT_DDL)
    for (cls, off), r in sorted(rows.items(), key=lambda kv: (kv[0][0], kv[0][1])):
        pc = per_class.setdefault(cls, [0, 0, 0, 0])  # rows, added, updated, skipped
        pc[0] += 1
        src = "engine-ai-atlas AI%d" % r["batch"]
        old = cur.execute("select size,type,name,confidence,evidence,readers,writers,source,date from struct_fields "
                          "where class=? and offset=?", (cls, off)).fetchone()
        if old is None:
            cur.execute("insert into struct_fields (class,offset,size,type,name,confidence,evidence,readers,writers,source,date) "
                        "values (?,?,?,?,?,?,?,?,?,?,?)",
                        (cls, off, r["size"], r["type"], r["name"], r["conf"], r["ev"], r["rd"], r["wr"], src, DATE))
            st.inc("sf_added")
            pc[1] += 1
            continue
        o_size, o_type, o_name, o_conf, o_ev, o_rd, o_wr, o_src, _ = old
        if RANK.get(norm_conf(o_conf), 0) > RANK[r["conf"]]:
            st.inc("sf_skipped"); st.inc("sf_skipped_lower"); pc[3] += 1
            print("KEEP-OLD %s +0x%X: existing %s '%s' outranks %s '%s'" % (cls, off, o_conf, o_name[:50], r["conf"], r["name"][:50]))
            continue
        rd, wr = merge_lists(o_rd, r["rd"]), merge_lists(o_wr, r["wr"])
        tie = RANK.get(norm_conf(o_conf), 0) == RANK[r["conf"]]
        if tie and not r.get("force"):
            # same confidence: the existing name/type/source stay; wave 2 only adds readers/writers, a missing size/type and
            # its own name as an evidence note (an override row from the gate replaces the name instead)
            name, typ, conf, o_src_keep = o_name, o_type or r["type"], o_conf, o_src
            size = o_size if o_size is not None else r["size"]
            ev = o_ev or ""
            if r["name"] != o_name:
                tag = "| AI%d name: %s" % (r["batch"], r["name"])
                if tag not in ev:
                    ev = (ev + " " + tag).strip()
            new_src = o_src_keep
        else:
            name, typ, conf = r["name"], r["type"] or o_type, r["conf"]
            size = r["size"] if r["size"] is not None else o_size
            ev = r["ev"]
            if o_src != src or o_name != r["name"]:
                ev += " | prev (%s, %s): %s" % (o_src, o_conf, o_name)
            if (o_ev or "").startswith(r["ev"]):
                ev = o_ev  # already written by an earlier run: keep the stored text (idempotent)
            new_src = src
        if (o_name == name and o_conf == conf and o_size == size and o_type == typ and o_rd == rd and o_wr == wr
                and (o_ev or "") == ev and o_src == new_src):
            st.inc("sf_skipped"); st.inc("sf_skipped_same"); pc[3] += 1
            continue
        cur.execute("update struct_fields set size=?,type=?,name=?,confidence=?,evidence=?,readers=?,writers=?,source=?,date=? "
                    "where class=? and offset=?", (size, typ, name, conf, ev, rd, wr, new_src, DATE, cls, off))
        st.inc("sf_updated")
        pc[2] += 1
        print("%s %s +0x%X: '%s' (%s, %s) -> '%s' (%s, %s)" % (
            "UPDATE" if not (tie and not r.get("force")) else "MERGE-RW", cls, off, (o_name or "")[:45], o_conf, o_src[-3:],
            name[:45], conf, new_src[-3:]))


def do_candidates(cur, st):
    cols = [r[1] for r in cur.execute("pragma table_info(candidates)")]
    want = ["id", "title", "subsystem", "hypothesis", "evidence", "expected_win", "risk", "status", "decision_date"]
    assert cols == want, "candidates columns changed: %s" % cols
    for c in CANDIDATES:
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


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--db", default=DEFAULT_DB, help="knowledge DB (default: the real DB next to the repo)")
    ap.add_argument("--dry-run", action="store_true", help="report what would change; roll everything back")
    ap.add_argument("-v", "--verbose", action="store_true", help="print every struct merge note")
    args = ap.parse_args()
    db = os.path.abspath(args.db)
    if not os.path.exists(db):
        sys.exit("DB not found: " + db)
    print("DB: %s%s" % (db, "  [DRY RUN, nothing written]" if args.dry_run else ""))

    corrections, new_rows, lessons = parse_gate_tables()
    rows, notes = build_struct_rows(None)
    for n in notes:
        if args.verbose or n.startswith(("WARNING", "SPLIT", "ADD", "REMAP", "REMOVED")):
            print(n)

    conn = sqlite3.connect(db)
    conn.isolation_level = None  # explicit transaction so --dry-run can roll back DDL too
    cur = conn.cursor()
    cur.execute("BEGIN")
    st, per_class = Stats(), {}
    try:
        do_name_map(cur, st, corrections, new_rows)
        do_lessons(cur, st, lessons)
        do_struct(cur, rows, st, per_class)
        do_candidates(cur, st)
    except Exception:
        cur.execute("ROLLBACK")
        raise
    cur.execute("ROLLBACK" if args.dry_run else "COMMIT")

    print("\n=== SUMMARY%s ===" % (" (dry run)" if args.dry_run else ""))
    print("name_map   : renamed/upgraded %d | added %d | skipped-existing %d (conflicts: %d)   [gate: %d corrections, %d new rows]" % (
        st["nm_renamed"], st["nm_added"], st["nm_skipped"], st["nm_conflict"], len(corrections), len(new_rows)))
    print("lessons    : added %d | updated %d | unchanged %d   | older lessons: UPDATED-appended %d, already-appended %d, missing %d" % (
        st["les_added"], st["les_updated"], st["les_skipped"], st["les_upd_applied"], st["les_upd_skipped"], st["les_missing"]))
    print("struct_flds: added %d | updated %d | skipped %d (same %d, lower-confidence %d)   [transcribed rows after merge: %d]" % (
        st["sf_added"], st["sf_updated"], st["sf_skipped"], st["sf_skipped_same"], st["sf_skipped_lower"], len(rows)))
    print("  per class (rows/added/updated/skipped): " + ", ".join(
        "%s %d/%d/%d/%d" % (k, *v) for k, v in sorted(per_class.items())))
    print("candidates : added %d | updated %d | unchanged %d" % (st["cand_added"], st["cand_updated"], st["cand_skipped"]))
    chk = sqlite3.connect(db)
    print("integrity_check:", chk.execute("pragma integrity_check").fetchone()[0])
    chk.close()


if __name__ == "__main__":
    main()
