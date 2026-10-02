#!/usr/bin/env python3
"""Python twin of patch_manager_mods/level_up_override_src/WizardLogic.h plus the trace CLI used by wizard_parity.py.

Trace lines are plain; stats lines start with '# ' (ignored by the parity diff).
Usage: wizard_twin.py [--selftest] [--trace-only] [ini_path]
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_INI = os.path.join(HERE, "runtime", "levelup_paths.ini")
MAX_ITEMS, MAX_OPTIONS, NUM_Q = 16, 8, 5
ROLE, STYLE, ALIGNMENT, CONVERT, SKILLS, PICK = range(6)
PAGE_NAMES = ["Role", "Style", "Alignment", "Convert", "Skills", "Pick"]
KEYS = ["role", "style", "alignment", "convert", "skills", "pick"]
MAX_UI_OPTIONS = 6

QUESTIONS = ["Combat role?", "Weapon style?", "Force alignment?", "Become a Jedi when?", "Skill focus?",
             "Choose a build:"]
LABELS = {"melee": "Melee", "ranged": "Ranged", "caster": "Force caster", "support": "Support / healer",
          "tank": "Tank", "single": "One weapon", "dual": "Two weapons", "double": "Double-bladed",
          "unarmed": "Unarmed", "pistols": "Pistols", "rifle": "Rifle", "none": "None", "light": "Light side",
          "dark": "Dark side", "neutral": "Neutral powers", "early": "Early", "late": "Late", "never": "Never",
          "n/a": "n/a", "combat": "Combat skills", "tech": "Tech skills", "stealth": "Stealth skills",
          "social": "Social skills"}


def question_text(page):
    return QUESTIONS[page] if 0 <= page <= PICK else ""


def answer_label(page, value):
    return LABELS.get(value.lower(), value)


def ieq(a, b):
    return a.lower() == b.lower()


def selectable(ans):
    return all(a and a != "-" for a in ans)


class Item:
    def __init__(self, id_, ans, short):
        self.id, self.ans, self.short = id_, list(ans), short


def parse_presets(path):
    """-> {tag: [Item]} in file order; dev presets (any answer '-') excluded."""
    out = {}
    in_sec = False
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line.startswith("["):
                in_sec = line == "[presets]"
                continue
            if not in_sec or not line or line[0] in "#;" or "=" not in line:
                continue
            pid, val = line.split("=", 1)
            f8 = val.split("|", 7)
            if len(f8) < 8:
                continue
            ans = f8[:5]
            if not selectable(ans) or "__" not in pid:
                continue
            out.setdefault(pid.split("__", 1)[0], []).append(Item(pid, ans, f8[7]))
    for tag in out:
        out[tag] = out[tag][:MAX_ITEMS]
    return out


def _matches(items, answers, i):
    for page, ref in answers:
        if page == PICK:
            if i != ref:
                return False
        elif not ieq(items[i].ans[page], items[ref].ans[page]):
            return False
    return True


def _cands(items, answers):
    return [i for i in range(len(items)) if _matches(items, answers, i)]


def view(items, answers):
    v = {"result": False, "item": -1, "page": -1, "options": [], "optionItem": []}
    cand = _cands(items, answers)
    if not cand:
        return v
    if len(cand) == 1:
        v["result"], v["item"] = True, cand[0]
        return v
    start = answers[-1][0] + 1 if answers else 0
    for p in range(start, NUM_Q):
        vals = []
        for c in cand:
            x = items[c].ans[p]
            if not any(ieq(y, x) for y in vals):
                vals.append(x)
        if len(vals) >= 2:
            v["page"], v["options"], v["optionItem"] = p, vals[:MAX_OPTIONS], [-1] * min(len(vals), MAX_OPTIONS)
            return v
    v["page"] = PICK
    v["options"] = [items[c].id for c in cand[:MAX_OPTIONS]]
    v["optionItem"] = cand[:MAX_OPTIONS]
    return v


def choose(items, answers, option):
    v = view(items, answers)
    if v["result"] or v["page"] < 0 or not (0 <= option < len(v["options"])) or len(answers) >= 6:
        return False
    if v["page"] == PICK:
        ref = v["optionItem"][option]
    else:
        ref = next((c for c in _cands(items, answers) if ieq(items[c].ans[v["page"]], v["options"][option])), -1)
    if ref < 0:
        return False
    answers.append((v["page"], ref))
    return True


def back(answers):
    if not answers:
        return False
    answers.pop()
    return True


def _path_str(items, answers):
    parts = []
    for page, ref in answers:
        parts.append("pick=" + items[ref].id if page == PICK else "%s=%s" % (KEYS[page], items[ref].ans[page]))
    return " ".join(parts)


def _line(tag, items, answers, v):
    p = _path_str(items, answers)
    if v["result"]:
        return "%s | %s | RESULT %s" % (tag, p, items[v["item"]].id)
    if v["page"] < 0:
        return "%s | %s | EMPTY" % (tag, p)
    return "%s | %s | page=%s opts=%s" % (tag, p, PAGE_NAMES[v["page"]], ",".join(v["options"]))


def enumerate_paths(tag, items):
    """DFS over every option; -> (trace lines, stats dict)."""
    lines = []
    st = {"presets": len(items), "maxopts": 0, "deepest": 0, "reached": set(), "bad": []}
    ans = []
    lines.append("%s | back-at-root=%s" % (tag, "true" if back(ans) else "false"))

    def dfs():
        v = view(items, ans)
        lines.append(_line(tag, items, ans, v))
        if v["result"]:
            st["reached"].add(items[v["item"]].id)
            st["deepest"] = max(st["deepest"], len(ans))
            return
        if v["page"] < 0:
            return
        st["maxopts"] = max(st["maxopts"], len(v["options"]))
        if len(v["options"]) > MAX_UI_OPTIONS:
            st["bad"].append("%s: page %s offers %d options" % (tag, PAGE_NAMES[v["page"]], len(v["options"])))
        for o in range(len(v["options"])):
            choose(items, ans, o)
            dfs()
            back(ans)
            v2 = view(items, ans)
            same = (v2["result"], v2["page"], v2["options"]) == (v["result"], v["page"], v["options"])
            lines.append("%s | %s | back %s" % (tag, _path_str(items, ans), "ok" if same else "MISMATCH"))
            if not same:
                st["bad"].append("%s: back mismatch" % tag)

    dfs()
    return lines, st


def synthetic_tags():
    """Mixed-case fixture exercising the case-insensitive rules (mirrored in wizard_test.cpp)."""
    return {"_case": [
        Item("_case__a", ["Melee", "single", "none", "n/a", "combat"], "A"),
        Item("_case__b", ["melee", "dual", "none", "n/a", "combat"], "B"),
        Item("_case__c", ["MELEE", "Dual", "none", "n/a", "Combat"], "C")]}


def run_cli(ini, trace_only):
    tags = parse_presets(ini)
    tags.update(synthetic_tags())
    fail = False
    stats = []
    for tag in tags:   # file order, synthetic last
        lines, st = enumerate_paths(tag, tags[tag])
        print("\n".join(lines))
        stats.append((tag, st))
        missing = [it.id for it in tags[tag] if it.id not in st["reached"]]
        for m in missing:
            print("# FAIL unreachable preset %s" % m)
            fail = True
        for b in st["bad"]:
            print("# FAIL " + b)
            fail = True
    if not trace_only:
        print("# tag presets maxopts deepest")
        for tag, st in stats:
            print("# %-12s %3d %3d %3d" % (tag, st["presets"], st["maxopts"], st["deepest"]))
    return 1 if fail else 0


def selftest():
    def it(i, a, s="s"):
        return Item(i, a.split("|"), s)
    # single preset -> immediate result
    one = [it("x__a", "melee|dual|light|early|combat")]
    assert view(one, [])["result"] and view(one, [])["item"] == 0
    # empty
    v = view([], [])
    assert not v["result"] and v["page"] == -1 and v["options"] == []
    # two identical answers -> Pick
    two = [it("g__a", "ranged|pistols|none|n/a|combat"), it("g__b", "ranged|pistols|none|n/a|combat")]
    v = view(two, [])
    assert v["page"] == PICK and v["options"] == ["g__a", "g__b"] and v["optionItem"] == [0, 1]
    a = []
    assert choose(two, a, 1) and view(two, a)["item"] == 1
    assert back(a) and view(two, a)["page"] == PICK
    # back on first page
    assert back([]) is False
    # skip + record semantics
    mix = [it("m__a", "melee|dual|light|early|combat"), it("m__b", "melee|dual|dark|early|combat"),
           it("m__c", "ranged|pistols|light|early|combat")]
    a = []
    assert view(mix, a)["page"] == ROLE and view(mix, a)["options"] == ["melee", "ranged"]
    assert choose(mix, a, 0) and len(a) == 1
    v = view(mix, a)
    assert v["page"] == ALIGNMENT and v["options"] == ["light", "dark"]   # Style auto-skipped
    assert choose(mix, a, 1) and view(mix, a)["item"] == 1
    assert back(a) and view(mix, a)["page"] == ALIGNMENT and back(a) and view(mix, a)["page"] == ROLE
    assert not choose(mix, a, 2) and not choose(mix, a, -1)
    # case-insensitive matching
    cs = synthetic_tags()["_case"]
    v = view(cs, [])
    assert v["page"] == STYLE and v["options"] == ["single", "dual"]
    a = []
    assert choose(cs, a, 1)
    v = view(cs, a)
    assert v["page"] == PICK and v["options"] == ["_case__b", "_case__c"]
    assert ieq("Light", "lIGHT") and not ieq("a", "ab")
    assert selectable("a|b|c|d|e".split("|")) and not selectable("a|-|c|d|e".split("|")) \
        and not selectable("a||c|d|e".split("|"))
    assert answer_label(0, "caster") == "Force caster" and answer_label(0, "zzz") == "zzz"
    print("wizard_twin selftest OK")
    return 0


def main(argv):
    if "--selftest" in argv:
        return selftest()
    args = [a for a in argv if not a.startswith("--")]
    return run_cli(args[0] if args else DEFAULT_INI, "--trace-only" in argv)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
