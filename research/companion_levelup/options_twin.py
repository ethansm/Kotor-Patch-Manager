#!/usr/bin/env python3
"""Python twin of patch_manager_mods/level_up_override_src/OptionsLogic.h plus the trace CLI used by options_parity.py.

Trace lines are plain; stats lines start with '# ' (ignored by the parity diff).
Usage: options_twin.py [--selftest] [--trace-only] [ini_path]
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from wizard_twin import (CONVERT, ROLE, STYLE, ALIGNMENT, SKILLS, MAX_ITEMS, Item, ieq,  # noqa: E402
                         parse_presets)

DEFAULT_INI = os.path.join(HERE, "runtime", "levelup_paths.ini")
ROW_PAGE = [CONVERT, ROLE, STYLE, ALIGNMENT, SKILLS]
NUM_ROWS, VARIANT = 5, 5
ROW_NAMES = ["Convert", "Role", "Style", "Alignment", "Skills", "Variant"]


def _val(items, i, row):
    return items[i].ans[ROW_PAGE[row]]


def row_shown(items, row):
    return any(not ieq(_val(items, i, row), _val(items, 0, row)) for i in range(1, len(items)))


def _match_before(items, i, ref, up_to):
    return all(ieq(_val(items, i, r), _val(items, ref, r)) for r in range(up_to) if row_shown(items, r))


def _same_all(items, i, ref):
    return all(ieq(_val(items, i, r), _val(items, ref, r)) for r in range(NUM_ROWS))


def _variant_group(items, cur):
    return [i for i in range(len(items)) if _same_all(items, i, cur)]


def _candidates(items, cur, row):
    first = []
    for i in range(len(items)):
        if _match_before(items, i, cur, row) and not any(ieq(_val(items, f, row), _val(items, i, row)) for f in first):
            first.append(i)
    return first


def shown(items, cur, row):
    if row == VARIANT:
        return len(_variant_group(items, cur)) > 1
    return 0 <= row < NUM_ROWS and row_shown(items, row)


def init(items, start):
    return start if 0 <= start < len(items) else 0


def options(items, cur, row):
    if not shown(items, cur, row):
        return []
    if row == VARIANT:
        return [items[i].short for i in _variant_group(items, cur)]
    return [_val(items, f, row) for f in _candidates(items, cur, row)]


def cycle(items, cur, row, d):
    """-> new cur, or None when nothing changes."""
    if not shown(items, cur, row):
        return None
    step = -1 if d < 0 else 1
    if row == VARIANT:
        g = _variant_group(items, cur)
        return g[(g.index(cur) + step) % len(g)]
    first = _candidates(items, cur, row)
    if len(first) < 2:
        return None
    at = 0
    for j, f in enumerate(first):
        if ieq(_val(items, f, row), _val(items, cur, row)):
            at = j
    want = _val(items, first[(at + step) % len(first)], row)
    best, best_score = -1, None
    for i in range(len(items)):
        if not _match_before(items, i, cur, row) or not ieq(_val(items, i, row), want):
            continue
        sc = [ieq(_val(items, i, r), _val(items, cur, r)) for r in range(row + 1, NUM_ROWS)]
        if best < 0 or sc > best_score:
            best, best_score = i, sc
    return best if best >= 0 else None


def shown_rows(items, cur=0):
    return [r for r in range(NUM_ROWS) if shown(items, cur, r)]


def run_cli(ini, trace_only):
    tags = parse_presets(ini)
    fail = False
    stats = []
    for tag, items in tags.items():
        n = len(items)
        rows = [r for r in range(NUM_ROWS) if row_shown(items, r)]
        print("%s | shown=%s" % (tag, ",".join(ROW_NAMES[r] for r in rows)))
        maxv = {}
        reach_all = True
        for s in range(n):
            allrows = rows + ([VARIANT] if shown(items, s, VARIANT) else [])
            for r in allrows:
                for d in (1, -1):
                    vals = options(items, s, r)
                    maxv[r] = max(maxv.get(r, 0), len(vals))
                    nc = cycle(items, s, r, d)
                    print("%s | start=%s | row=%s dir=%+d -> %s (vals %s)" % (
                        tag, items[s].id, ROW_NAMES[r], d, items[nc].id if nc is not None else "NOCHANGE", ",".join(vals)))
            seen, queue = {s}, [s]
            while queue:
                c = queue.pop(0)
                for r in list(range(NUM_ROWS)) + [VARIANT]:
                    for d in (1, -1):
                        nc = cycle(items, c, r, d)
                        if nc is not None and nc not in seen:
                            seen.add(nc)
                            queue.append(nc)
            print("%s | start=%s | reach %s" % (tag, items[s].id, ",".join(items[i].id for i in sorted(seen))))
            if len(seen) != n:
                print("# FAIL unreachable from %s: %s" % (items[s].id, ",".join(items[i].id for i in range(n) if i not in seen)))
                reach_all = False
        fail |= not reach_all
        stats.append((tag, rows, maxv))
    if not trace_only:
        print("# tag shown-rows max-values-per-row")
        for tag, rows, maxv in stats:
            print("# %-12s %s | %s" % (tag, ",".join(ROW_NAMES[r] for r in rows),
                                       " ".join("%s=%d" % (ROW_NAMES[r], maxv.get(r, 0)) for r in rows)))
    return 1 if fail else 0


def selftest():
    def it(i, a, s="s"):
        c, r, st, al, sk = a.split("|")   # written in row order; Item.ans is indexed by wiz page (role,style,align,convert,skills)
        return Item(i, [r, st, al, c, sk], s)
    tags = parse_presets(DEFAULT_INI)
    kreia = tags["kreia"]
    assert not row_shown(kreia, 0) and shown_rows(kreia) == [1, 2, 3, 4]   # Kreia: no Convert row
    g = tags["g0t0"]
    dup = next(i for i, x in enumerate(g) if x.id == "g0t0__ranged_pistols_none_na")
    assert shown(g, dup, VARIANT) and len(options(g, dup, VARIANT)) == 2
    nxt = cycle(g, dup, VARIANT, 1)
    assert nxt is not None and nxt != dup and cycle(g, nxt, VARIANT, 1) == dup   # wrap
    assert not shown(g, next(i for i, x in enumerate(g) if x.id.endswith("support_pistols_none_na")), VARIANT)
    # wrap-around + rows-before filter + preference
    mix = [it("m__a", "early|melee|dual|light|combat"), it("m__b", "early|melee|single|dark|combat"),
           it("m__c", "never|ranged|pistols|none|tech"), it("m__d", "early|ranged|pistols|light|combat")]
    assert shown_rows(mix) == [0, 1, 2, 3, 4]
    assert cycle(mix, 0, 0, 1) == 2 and cycle(mix, 0, 0, -1) == 2     # two values, wraps both ways
    assert cycle(mix, 2, 0, 1) == 3                                     # wrap back; d keeps Role=ranged/Style=pistols (a,b do not)
    assert options(mix, 0, 1) == ["melee", "ranged"]                    # Convert=early filter hides nothing here
    assert options(mix, 2, 1) == ["ranged"] and cycle(mix, 2, 1, 1) is None   # rows-before filter (Convert=never)
    # preference: from a(early melee dual light) Role->ranged keeps later answers: d(pistols light) has Alignment=light
    assert cycle(mix, 0, 1, 1) == 3
    # from b(single dark combat) Role->ranged: d matches combat (skills) -> d
    assert cycle(mix, 1, 1, 1) == 3
    # lexicographic preference: style row first
    pref = [it("p__a", "early|melee|dual|light|combat"), it("p__b", "early|ranged|single|light|tech"),
            it("p__c", "early|ranged|dual|dark|tech")]
    assert cycle(pref, 0, 1, 1) == 2     # style 'dual' kept beats alignment 'light' kept
    print("options_twin selftest OK")
    return 0


def main(argv):
    if "--selftest" in argv:
        return selftest()
    args = [a for a in argv if not a.startswith("--")]
    return run_cli(args[0] if args else DEFAULT_INI, "--trace-only" in argv)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
