#!/usr/bin/env python3
"""npcvar_script_scan.py - per-script NWScript primitive census for the NPC variety research.

Scans every .claude/scratch_npcvar/nss_all/*.nss with a comment- and string-aware tokenizer and
counts calls to the routines in ROUTINES. nwscript.nss (the engine prototype header, not a script)
is skipped. Writes .claude/research/npc_variety/tables/script_primitives.tsv: one row per
(script, routine) with count > 0, columns script, routine, count. Read-only on the input dir,
stdlib only. Run `python3 scripts/npcvar_script_scan.py` to regenerate. The pure functions below
are imported by scripts/test_npcvar_script_scan.py.
"""
import csv
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
TABLE_DIR = REPO_ROOT / ".claude" / "research" / "npc_variety" / "tables"
NSS_DIR = REPO_ROOT / ".claude" / "scratch_npcvar" / "nss_all"
PRIMITIVES_OUT = TABLE_DIR / "script_primitives.tsv"

COLUMNS = ["script", "routine", "count"]

ROUTINES = [
    "ExecuteScript", "DelayCommand", "AssignCommand", "ActionDoCommand",
    "ActionForceMoveToLocation", "ActionMoveToObject", "ActionMoveToLocation",
    "ActionAttack", "CutsceneAttack", "ActionCastFakeSpellAtObject", "EffectHaste",
    "SetPlotFlag", "SetMinOneHP", "SetCommandable", "ChangeFaction", "CreateObject",
    "DestroyObject", "ActionSpeakStringByStrRef", "PlaySound",
]
LOCAL_ROUTINES = ("GetLocalNumber", "GetLocalBoolean", "GetLocalObject")
TAG_ROUTINES = ("GetObjectByTag", "GetWaypointByTag")
# nwscript.nss declares every routine as a prototype; scanning it would count declarations as calls.
EXCLUDED_SCRIPTS = {"nwscript"}

# One alternation scanned left to right: a comment or string literal is consumed as a unit, so
# "//" inside a string and quotes inside a comment are handled correctly.
TOKEN_RE = re.compile(
    r'(?P<line>//[^\n]*)'
    r'|(?P<block>/\*.*?(?:\*/|\Z))'
    r'|(?P<str>"(?:\\[^\n]|[^"\\\n])*(?P<close>")?)',
    re.S,
)
STR_LIT_RE = re.compile(r'"((?:\\[^\n]|[^"\\\n])*)"')


def _blank(text):
    """Replace every character with a space, keeping newlines so offsets and lines stay aligned."""
    return "".join("\n" if ch == "\n" else " " for ch in text)


def mask_source(text):
    """Return (code, masked), both the same length as text.

    code: comments blanked, string literals intact (tag arguments are read from here).
    masked: comments blanked and string contents blanked, quotes kept (calls are found here).
    """
    code, masked = [], []
    last = 0
    for m in TOKEN_RE.finditer(text):
        plain = text[last:m.start()]
        code.append(plain)
        masked.append(plain)
        last = m.end()
        tok = m.group()
        if m.group("str") is not None:
            closed = m.group("close") is not None
            inner = tok[1:-1] if closed else tok[1:]
            tail = '"' if closed else ""
            code.append('"' + inner + tail)
            masked.append('"' + _blank(inner) + tail)
        else:
            blank = _blank(tok)
            code.append(blank)
            masked.append(blank)
    code.append(text[last:])
    masked.append(text[last:])
    return "".join(code), "".join(masked)


def _matching_paren(masked, open_idx):
    """Index of the ')' matching the '(' at open_idx in masked text, or -1 if unclosed."""
    depth = 0
    for i in range(open_idx, len(masked)):
        ch = masked[i]
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                return i
    return -1


def normalise_call(text):
    """Collapse whitespace: 'GetLocalNumber( OBJECT_SELF ,29 )' -> 'GetLocalNumber(OBJECT_SELF, 29)'."""
    s = re.sub(r"\s+", " ", text).strip()
    s = re.sub(r"\(\s*", "(", s)
    s = re.sub(r"\s*\)", ")", s)
    return re.sub(r"\s*,\s*", ", ", s)


def scan_script(text):
    """Count primitive calls and collect local-variable and tag arguments for one script.

    Returns {'counts': {routine: int} (only >0), 'local_args': [normalised call text, ...] in source
    order, 'tags': [string literal, ...] in source order}. Comments and string contents never
    produce a call match.
    """
    code, masked = mask_source(text)
    counts = {}
    for name in ROUTINES:
        n = len(re.findall(r"\b%s\s*\(" % re.escape(name), masked))
        if n:
            counts[name] = n
    local_args = []
    local_re = re.compile(r"\b(%s)\s*\(" % "|".join(LOCAL_ROUTINES))
    for m in local_re.finditer(masked):
        close = _matching_paren(masked, m.end() - 1)
        if close != -1:
            local_args.append(normalise_call(code[m.start():close + 1]))
    tags = []
    tag_re = re.compile(r'\b(%s)\s*\(\s*"' % "|".join(TAG_ROUTINES))
    for m in tag_re.finditer(masked):
        lit = STR_LIT_RE.match(code, m.end() - 1)
        if lit:
            tags.append(lit.group(1))
    return {"counts": counts, "local_args": local_args, "tags": tags}


def scan_dir(nss_dir):
    """Scan every *.nss in nss_dir except EXCLUDED_SCRIPTS. Returns {file stem: scan_script result}."""
    results = {}
    for path in sorted(nss_dir.glob("*.nss")):
        if path.stem in EXCLUDED_SCRIPTS:
            continue
        results[path.stem] = scan_script(path.read_text(encoding="utf-8", errors="replace"))
    return results


def tsv_rows(results):
    """One row per (script, routine) with count > 0, sorted by routine, then count desc, then script."""
    rows = [
        {"script": script, "routine": routine, "count": count}
        for script, res in results.items()
        for routine, count in res["counts"].items()
    ]
    rows.sort(key=lambda r: (r["routine"], -r["count"], r["script"]))
    return rows


def write_tsv(path, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=COLUMNS, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def main():
    if not NSS_DIR.is_dir():
        print(f"skip: {NSS_DIR} absent, nothing scanned")
        return 0
    results = scan_dir(NSS_DIR)
    rows = tsv_rows(results)
    write_tsv(PRIMITIVES_OUT, rows)
    with_prim = sum(1 for res in results.values() if res["counts"])
    print(f"scripts {len(results)} with_primitives {with_prim}")
    print(f"script_primitives: {len(rows)} rows -> {PRIMITIVES_OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
