#!/usr/bin/env python3
"""npc-diversity log checker (npc-diversity route). Python 3, stdlib only; read-only (--new writes only its bookmark).

Usage: npc_div_log_check.py [LOG | --log LOG] [--new [--no-advance] [--state PATH]]
  Default log: ~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/npc-diversity_log.txt
  Analyses the LAST launch: the lines from the last `BANNER v=` line to the end of the window.
  --new: only lines after the bookmark shared with log_triage (same state file, keyed by absolute path), then advance it.
    A --new window with no BANNER v= line is a continuation: the banner check is a WARN, not a FAIL.
  Output: at most 40 lines. One PASS/WARN/FAIL line per check plus indented detail. Raw log lines are never echoed.
  Exit 0 = no FAIL (WARN allowed), 1 = at least one FAIL, 2 = log missing.

Grammar: GOLDEN block at the top of patch_manager_mods/npc_diversity_src/diversity_log_test.cpp (authoritative), formatters in
NpcDivLog.h, background in .claude/review/npc_div/E_logging.md 4.2. The KpLog prefix and fallback sink shapes are parsed by
scripts/kplog_parse.py parse_line; a bare body is accepted as-is.
0.1.2-probe additions: RMP `tv=<b>><a>` (after app=; absent on older lines), CRE0 `off=<hex>` (absent = 000), STAT remap
`tvchg=`. BANNER sha= is a label (steam|gog|none) or a hash in new builds, an 8-hex value in old ones; it is not checked.
"""
from __future__ import annotations

import argparse
import os
import re
import sys
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import kplog_parse as KP  # noqa: E402  (parse_line only; the prefix is not re-implemented here)
import log_triage as LT  # noqa: E402  (--new bookmark: same state file and helpers; log_triage is not modified)

DEFAULT_LOG = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II/npc-diversity_log.txt"
MAX_LINES = 40
TOP_ROWS = 8                      # (app,g) rows in the TEX slot table
HOOK_COUNT = 2                    # BANNER hook lines expected: tex and remap
BANNER_PRE_OK = ("ok", "jmp")     # jmp = detour already installed; anything else is a FAIL
LOG_ONLY_FROM = (0, 1, 2)         # hook 1 is log-only (TEX sw=0 expected) from banner v=0.1.2-probe on
TV_MIN_LINES = 20                 # tv-logged RMP lines an app group needs before its tv spread is judged
TV_MIN_DISTINCT = 3               # distinct tv after-values a judged group must reach
LIST_MAX = 5                      # groups / oids listed per FAIL
# skip:dm allowed on commoner-row RMP lines: at most 1 in 5 (checked as 5 * skip_dm > rows, integers only).
# Commoner rows = DiversityTables.h IsGenericMale/FemaleCommoner + IsGenericTwilekMale/Female (RMP app is the pre-remap row).
COMMONER_APPS = frozenset(
    [17, 190, 191, 192, 193, 194, 345, 346, 347, 348, 349, 423, 424, 428, 430, 591, 592, 593, 594, 595, 596, 615, 616, 617]
    + [14, 185, 186, 187, 188, 189, 339, 340, 341, 342, 343, 344, 429, 431, 585, 586, 587, 588, 589, 590, 609, 610, 611,
       612, 613, 614]
    + [416, 417, 445, 449, 450, 606, 618, 619]
    + [280, 446, 447, 448, 620, 621, 622])
PASS, WARN, FAIL, INFO = "PASS", "WARN", "FAIL", "INFO"
EXCLUDED_TAG = re.compile(
    r"^(\d{3}_)?(n_|npc_|c_)?(kreia|atton|bao_dur|baodur|mira|hanharr|mandalore|visas|disciple|handmaiden|t3m4|hk47|hk_47"
    r"|g0t0|player|chodo|luxa|grenn|opo_chano|opochano|chano|lorso|b4d4|ralon|moza|vrook|zreztik|geeda|falt|batono)(_|\d|$)",
    re.IGNORECASE)
DEC = r"-?\d+"
HEXV = r"[0-9a-fA-F]+"
PAIR_RE = re.compile(r"(\d+):(\d+)")
KEYWORDS = ("BANNER", "TEX", "RMP", "CEN", "STAT", "ABI", "READ", "HOOKBYTES", "DSC0", "CRE0", "DUMP")
SUBBED = ("STAT", "BANNER")       # heads whose second word is a sub-kind (STAT tex, BANNER rows)
# kind -> req: keys that must be present; dec: keys read as decimal ("*" = all); hx: keys read as hex.
# Values are kept as strings in rec["f"] and converted in rec["v"]. Lines with a kind not listed here are ignored.
SCHEMA = {
    "BANNER:v": dict(req=("v", "kpl", "sha", "base", "ts"), dec=("kpl",), hx=("base", "ts")),
    "BANNER:hook": dict(req=("hook", "addr", "pre"), dec=(), hx=("addr",)),
    "TEX": dict(req=("pd", "fr", "ret", "app", "g", "tv", "dm", "hd", "ph", "cur", "h", "st", "slot", "new", "sw"),
                dec=("app", "g", "tv", "dm", "hd", "ph", "st", "slot", "sw"), hx=("pd", "fr", "ret", "h")),
    # tv (0.1.2) is optional and read in _typed; g is read as decimal only when a line carries it (no RMP line has g today).
    "RMP": dict(req=("this", "pd", "oid", "tp", "tl", "tag", "xy", "xyb", "app", "h", "why"),
                dec=("tl", "g"), hx=("this", "pd", "oid", "tp", "h")),
    "CEN": dict(req=("k", "app", "g", "tv"), dec=("app", "g", "tv", "dm"), hx=()),
    "STAT:tex": dict(req=("calls", "comm", "swap", "frd", "retd"), dec="*", hx=()),
    "STAT:remap": dict(req=("calls", "chg", "nochg", "excl", "tagun"), dec="*", hx=()),
    "STAT:abi": dict(req=("app", "g", "dm"), dec="*", hx=()),
    "STAT:xy": dict(req=("x", "y"), dec=(), hx=()),
    "STAT:census": dict(req=("keys", "ovf"), dec="*", hx=()),
    "STAT:app": dict(req=("k",), dec=(), hx=()),
    "STAT:slot": dict(req=("g",), dec=("g",), hx=()),
    "STAT:read": dict(req=(), dec=(), hx=()),
    "ABI": dict(req=("app", "g", "dm", "tv", "pd", "where"), dec=("app", "g", "dm", "tv"), hx=("pd",)),
    "READ": dict(req=("site", "addr", "len", "caller", "n"), dec=("site", "len", "n"), hx=("addr", "caller")),
    "HOOKBYTES": dict(req=("addr", "want", "got"), dec=(), hx=("addr",)),
    "DSC0": dict(req=("pd", "hex"), dec=(), hx=("pd",)),
    "CRE0": dict(req=("this", "hex"), dec=(), hx=("this", "off")),
    "DUMP": dict(req=("trigger", "path"), dec=(), hx=()),
}


class Bad(Exception):
    pass


def body_of(raw):
    """Message text of one log line (KpLog prefix or fallback sink stripped; a bare line is taken as-is). None for headers."""
    p = KP.parse_line(raw)
    if p is None:
        return raw.strip()
    if p["kind"] == "header":
        return None
    return p["msg"].strip()


def _typed(kind, f):
    """Typed copy of the string fields f: dec/hx keys converted, the rest kept as strings. Bad when a field is malformed."""
    spec = SCHEMA[kind]
    for k in spec["req"]:
        if k not in f:
            raise Bad("missing %s" % k)
    val = dict(f)
    for k in (f if spec["dec"] == "*" else spec["dec"]):
        if k in f:
            if not re.fullmatch(DEC, f[k]):
                raise Bad("malformed %s" % k)
            val[k] = int(f[k])
    for k in spec["hx"]:
        if k in f:
            if not re.fullmatch(HEXV, f[k]):
                raise Bad("malformed %s" % k)
            val[k] = int(f[k], 16)
    if kind == "RMP":
        m = re.fullmatch(r"(\d+)>(\d+)", f["app"])
        if not m:
            raise Bad("malformed app")
        val["before"], val["after"] = int(m.group(1)), int(m.group(2))
        val["tv_before"], val["tv_after"] = None, None       # pre-0.1.2 lines carry no tv=
        if "tv" in f:
            m = re.fullmatch(r"(\d+)>(\d+)", f["tv"])
            if not m:
                raise Bad("malformed tv")
            val["tv_before"], val["tv_after"] = int(m.group(1)), int(m.group(2))
    if kind == "CRE0":
        val.setdefault("off", 0)                              # pre-0.1.2 CRE0 has no off=: first window, off 000
    return val


def version_tuple(v):
    """'0.1.2-probe' -> (0, 1, 2); None when the banner version has no X.Y.Z head."""
    m = re.match(r"(\d+)\.(\d+)\.(\d+)", v or "")
    return tuple(int(x) for x in m.groups()) if m else None


def _changed(r):
    """RMP line shows an app change, or a tv change when the line carries tv= (old lines: app only)."""
    v = r["v"]
    if v["before"] != v["after"]:
        return True
    return v["tv_before"] is not None and v["tv_before"] != v["tv_after"]


def parse_body(body):
    """(kind, string fields, typed fields, pairs) for a known npc-diversity line; None for any other line.
    Raises Bad when a known line is malformed. pairs = the 'N:M' tokens (STAT slot/app/read chunks)."""
    toks = body.split()
    if not toks or toks[0] not in KEYWORDS:
        return None
    head, rest = toks[0], toks[1:]
    sub = ""
    if head in SUBBED and rest and "=" not in rest[0] and not PAIR_RE.fullmatch(rest[0]):
        sub, rest = rest[0], rest[1:]
    kind = head + (":" + sub if sub else "")
    if head == "BANNER" and not sub:
        first = rest[0].partition("=")[0] if rest else ""
        kind = {"v": "BANNER:v", "hook": "BANNER:hook"}.get(first, "BANNER")
    if kind not in SCHEMA:
        return None
    f, pairs = {}, []
    for t in rest:
        m = PAIR_RE.fullmatch(t)
        if m:
            pairs.append((int(m.group(1)), int(m.group(2))))
            continue
        k, eq, v = t.partition("=")
        if not eq or not k or k in f:
            raise Bad("bare or duplicate token")
        f[k] = v
    return kind, f, _typed(kind, f), pairs


def parse_lines(window):
    """window: [(line_no, raw)] -> (records in log order, malformed count). Record: line, kind, f (strings), v (typed), pairs."""
    recs, bad = [], 0
    for n, raw in window:
        body = body_of(raw)
        if not body:
            continue
        try:
            got = parse_body(body)
        except Bad:
            bad += 1
            continue
        if got is not None:
            kind, f, v, pairs = got
            recs.append({"line": n, "kind": kind, "f": f, "v": v, "pairs": pairs})
    return recs, bad


def _of(recs, kind):
    return [r for r in recs if r["kind"] == kind]


def _fields(f):
    return " ".join("%s=%s" % kv for kv in f.items())[:100]


def _emit(num, name, msgs, ok, details):
    """msgs: [(level, text)] with FAIL before WARN. The first message (or the ok text) is the verdict line; the rest follow it."""
    if msgs:
        level, head = msgs[0]
    else:
        level, head = PASS, ok
    out = ["%s %d %s: %s" % (level, num, name, head)]
    out += ["  " + d for d in details]
    out += ["  %s %s" % m for m in msgs[1:]]
    return level, out


def check_banner(sess, new_mode):
    vs, hooks = _of(sess, "BANNER:v"), _of(sess, "BANNER:hook")
    msgs, details = [], []
    if vs:
        v = vs[-1]["f"]
        details.append("version v=%s kpl=%s sha=%s" % (v["v"], v["kpl"], v["sha"]))
    elif new_mode:
        msgs.append((WARN, "no BANNER v= in --new window (continuation; banner not judged)"))
    else:
        msgs.append((FAIL, "no BANNER v= line: mod not loaded or log cut"))
    if hooks:
        details.append("hooks " + " ".join("%s=%s" % (h["f"]["hook"], h["f"]["pre"]) for h in hooks))
    for h in hooks:
        if h["f"]["pre"] not in BANNER_PRE_OK:
            msgs.append((FAIL, "%s pre=%s (want ok or jmp)" % (h["f"]["hook"], h["f"]["pre"])))
    if (vs or not new_mode) and len(hooks) != HOOK_COUNT:
        msgs.append((FAIL, "expected %d BANNER hook lines, found %d" % (HOOK_COUNT, len(hooks))))
    ok = "v=%s, %d hooks pre ok/jmp" % (vs[-1]["f"]["v"] if vs else "?", len(hooks))
    return _emit(1, "banner", _sorted_msgs(msgs), ok, details)


def _sorted_msgs(msgs):
    return sorted(msgs, key=lambda m: m[0] != FAIL)


def check_read_abi(sess):
    rows = sorted(_of(sess, "READ") + _of(sess, "ABI"), key=lambda r: r["line"])
    msgs = []
    if rows:
        msgs.append((FAIL, "%d READ and ABI line(s): hook read or ABI check failed" % len(rows)))
    details = ["L%d %s %s" % (r["line"], r["kind"], _fields(r["f"])) for r in rows[:2]]
    return _emit(2, "READ/ABI", msgs, "no READ or ABI lines", details)


def check_hookbytes(sess):
    hb = _of(sess, "HOOKBYTES")
    msgs = [(FAIL, "%d HOOKBYTES line(s): hook bytes differ from the expected prologue" % len(hb))] if hb else []
    details = ["L%d %s" % (r["line"], _fields(r["f"])) for r in hb[:1]]
    return _emit(3, "HOOKBYTES", msgs, "no HOOKBYTES lines", details)


def check_rmp(sess):
    rmp = _of(sess, "RMP")
    if not rmp:
        return _emit(4, "RMP", [(FAIL, "0 RMP lines: hook 2 did not fire for existing creatures: use decision-gate branch 3")],
                     "", [])
    n = len(rmp)
    chg = sum(1 for r in rmp if _changed(r))
    oids = len({r["f"]["oid"] for r in rmp})
    xys = len({r["f"]["xy"] for r in rmp})
    tagged = sum(1 for r in rmp if r["f"]["tag"])
    why = " ".join("%s=%d" % kv for kv in Counter(r["f"]["why"] for r in rmp).most_common())
    commoner = [r for r in rmp if r["v"]["before"] in COMMONER_APPS]
    skip_dm = sum(1 for r in commoner if r["f"]["why"] == "skip:dm")
    msgs = []
    if oids <= 1 or xys <= 1:
        msgs.append((FAIL, "offsets invalid (oid/xy constant): oid distinct=%d xy distinct=%d" % (oids, xys)))
    if tagged == 0:
        msgs.append((WARN, "tag offset unverified: tag empty on all %d RMP lines" % n))
    if skip_dm * 5 > len(commoner):
        msgs.append((WARN, "skip:dm on %d of %d commoner-row RMP lines (%.0f%%, want <=20%%)" % (
            skip_dm, len(commoner), 100.0 * skip_dm / len(commoner))))
    details = ["n=%d chg=%d (%.1f%%) why: %s" % (n, chg, 100.0 * chg / n, why),
               "oid distinct=%d xy distinct=%d tag non-empty %d/%d (%.0f%%)" % (oids, xys, tagged, n, 100.0 * tagged / n)]
    return _emit(4, "RMP", _sorted_msgs(msgs), "n=%d chg=%d" % (n, chg), details)


def check_tex(sess, swap):
    """swap: 'expect' (banner older than 0.1.2: sw=1 required), 'log_only' (0.1.2+: hook 1 never swaps),
    'unknown' (--new window with no BANNER v= line: cannot tell, so WARN)."""
    tex = _of(sess, "TEX")
    if not tex:
        return _emit(5, "TEX", [(WARN, "0 TEX lines: no commoner texture calls, or hook 1 did not fire")], "", [])
    sw1 = [r for r in tex if r["v"]["sw"] == 1]
    st1 = sum(1 for r in tex if r["v"]["st"] == 1)
    frd = len({r["v"]["fr"] for r in tex})
    retd = len({r["v"]["ret"] for r in tex})
    msgs = []
    if not sw1:
        if swap == "expect":
            msgs.append((FAIL, "0 of %d TEX lines with sw=1: no swap path reached" % len(tex)))
        elif swap == "unknown":
            msgs.append((WARN, "0 of %d TEX lines with sw=1: swap not judged (no BANNER v= in --new window)" % len(tex)))
    per = {}
    for r in sw1:
        per.setdefault((r["v"]["app"], r["v"]["g"]), Counter())[r["v"]["slot"]] += 1
    rows = sorted(per.items(), key=lambda kv: (-sum(kv[1].values()), kv[0]))
    if rows and all(len(c) == 1 for _, c in rows):
        msgs.append((WARN, "every (app,g) has one slot (%d pairs): seed collapse? (H1)" % len(rows)))
    details = ["n=%d sw=1:%d st=1:%.1f%% fr distinct=%d ret distinct=%d" % (
        len(tex), len(sw1), 100.0 * st1 / len(tex), frd, retd)]
    if not sw1 and swap == "log_only":
        details.append("hook 1 is log-only in this build (banner v>=0.1.2): sw=1 not expected")
    if rows:
        details.append("slot per (app,g), sw=1: app g lines distinct top slot:count")
        for (app, g), c in rows[:TOP_ROWS]:
            top = " ".join("%02d:%d" % kv for kv in c.most_common(3))
            details.append("  %5d %d %5d %3d  %s" % (app, g, sum(c.values()), len(c), top))
        if len(rows) > TOP_ROWS:
            details.append("  ... +%d more pairs" % (len(rows) - TOP_ROWS))
    return _emit(5, "TEX", _sorted_msgs(msgs), "n=%d sw=1=%d" % (len(tex), len(sw1)), details)


def check_excluded(sess):
    hits = [r for r in _of(sess, "RMP") if _changed(r) and EXCLUDED_TAG.match(r["f"]["tag"])]
    msgs = []
    if hits:
        h = hits[0]
        msgs.append((FAIL, "%d changed RMP line(s) with an excluded tag (first L%d tag=%s app=%d>%d)" % (
            len(hits), h["line"], h["f"]["tag"], h["v"]["before"], h["v"]["after"])))
    return _emit(6, "excluded-tag safety", msgs, "0 changed excluded-tag rows", [])


def check_cen(sess):
    """INFO 7: tex-side (app,g,tv) tuples and how many have an app in the final STAT app k=tex histogram."""
    tuples = {(r["v"]["app"], r["v"]["g"], r["v"]["tv"]) for r in _of(sess, "TEX")}
    stat_tex = _of(sess, "STAT:tex")
    last_tex = stat_tex[-1]["line"] if stat_tex else 0
    hist = {app for r in sess if r["kind"] == "STAT:app" and r["f"]["k"] == "tex" and r["line"] > last_tex
            for app, _ in r["pairs"]}
    hit = sum(1 for t in tuples if t[0] in hist)
    cen = sum(1 for r in _of(sess, "CEN") if r["f"]["k"] == "tex")
    return INFO, ["INFO 7 CEN: tex-side (app,g,tv)=%d, app in final STAT app k=tex=%d (CEN k=tex lines=%d)" % (
        len(tuples), hit, cen)]


def check_last_stat(sess):
    t, m = _of(sess, "STAT:tex"), _of(sess, "STAT:remap")
    if not t and not m:
        return INFO, ["INFO 8 STAT: none in this launch (touch npc-diversity.dump in the game dir to dump)"]
    parts = []
    if t:
        v = t[-1]["v"]
        parts.append("tex calls=%d comm=%d swap=%d" % (v["calls"], v["comm"], v["swap"]))
    if m:
        v = m[-1]["v"]
        parts.append("remap calls=%d chg=%d nochg=%d excl=%d tagun=%d" % (v["calls"], v["chg"], v["nochg"], v["excl"], v["tagun"]))
        if "tvchg" in v:
            parts[-1] += " tvchg=%d" % v["tvchg"]
    return INFO, ["INFO 8 last STAT: " + " | ".join(parts)]


def check_gate(sess):
    """9 gate: at least one RMP line must change its app or its tv. Zero RMP lines fail too (nothing changed)."""
    rmp = _of(sess, "RMP")
    n_chg = sum(1 for r in rmp if _changed(r))
    if n_chg == 0:
        return _emit(9, "gate", [(FAIL, "gate still blocking: no RMP line shows an app or tv change (RMP lines=%d)" % len(rmp))],
                     "", [])
    return _emit(9, "gate", [], "%d of %d RMP lines changed (app or tv)" % (n_chg, len(rmp)), [])


def _tv_lines(sess):
    """RMP lines that carry tv= (0.1.2 grammar), as typed dicts."""
    return [r["v"] for r in _of(sess, "RMP") if r["v"]["tv_after"] is not None]


def check_tv_spread(sess):
    """10 tv spread: per final app (and g when a line has one), once TV_MIN_LINES tv-logged RMP lines exist,
    the distinct tv after-values must number at least TV_MIN_DISTINCT."""
    tv = _tv_lines(sess)
    groups = {}
    for v in tv:
        groups.setdefault((v["after"], v.get("g")), []).append(v["tv_after"])
    judged = sorted(((k, vals) for k, vals in groups.items() if len(vals) >= TV_MIN_LINES),
                    key=lambda kv: (kv[0][0], -1 if kv[0][1] is None else kv[0][1]))
    if not judged:
        return INFO, ["INFO 10 tv spread: not judged (no app group with >=%d RMP lines carrying tv=; tv lines=%d)" % (
            TV_MIN_LINES, len(tv))]
    bad = [(k, vals) for k, vals in judged if len(set(vals)) < TV_MIN_DISTINCT]
    if not bad:
        return _emit(10, "tv spread", [], "%d group(s) with >=%d RMP lines, all with >=%d distinct tv" % (
            len(judged), TV_MIN_LINES, TV_MIN_DISTINCT), [])
    msgs = [(FAIL, "%d app group(s) with >=%d RMP lines have <%d distinct tv after-values" % (
        len(bad), TV_MIN_LINES, TV_MIN_DISTINCT))]
    details = ["app=%d%s n=%d tv distinct=%d" % (k[0], "" if k[1] is None else " g=%d" % k[1], len(vals), len(set(vals)))
               for k, vals in bad[:LIST_MAX]]
    return _emit(10, "tv spread", msgs, "", details)


def check_tv_determinism(sess):
    """11 tv determinism: an oid seen more than once with the same final app must carry one tv after-value."""
    per = {}
    for v in _tv_lines(sess):
        per.setdefault((v["oid"], v["after"]), []).append(v["tv_after"])
    repeated = {k: vals for k, vals in per.items() if len(vals) >= 2}
    if not repeated:
        return INFO, ["INFO 11 tv determinism: not judged (no oid seen twice with the same final app)"]
    bad = sorted(k for k, vals in repeated.items() if len(set(vals)) > 1)
    if not bad:
        return _emit(11, "tv determinism", [], "%d repeated oid(s), tv stable across occurrences" % len(repeated), [])
    msgs = [(FAIL, "%d oid(s) got different tv after-values for the same final app" % len(bad))]
    details = ["oid=%08x app=%d tv=%s" % (oid, app, ",".join(str(x) for x in sorted(set(repeated[(oid, app)]))))
               for oid, app in bad[:LIST_MAX]]
    return _emit(11, "tv determinism", msgs, "", details)


def render(recs, bad, nlines, note, new_mode):
    """(output lines, FAIL count) for one window. The analysed unit is the last launch (from the last BANNER v= line)."""
    starts = [i for i, r in enumerate(recs) if r["kind"] == "BANNER:v"]
    sess = recs[starts[-1]:] if starts else recs
    vs = _of(sess, "BANNER:v")
    version = vs[-1]["f"]["v"] if vs else "none"
    vt = version_tuple(version)
    if not vs:
        swap = "unknown" if new_mode else "expect"
    elif vt is None:
        swap = "expect"
    else:
        swap = "log_only" if vt >= LOG_ONLY_FROM else "expect"
    results = [check_banner(sess, new_mode), check_read_abi(sess), check_hookbytes(sess), check_rmp(sess),
               check_tex(sess, swap), check_excluded(sess), check_cen(sess), check_last_stat(sess),
               check_gate(sess), check_tv_spread(sess), check_tv_determinism(sess)]
    levels = [lv for lv, _ in results]
    nf, nw = levels.count(FAIL), levels.count(WARN)
    word = FAIL if nf else WARN if nw else PASS
    out = ["NPC-DIV-LOG launches=%d (last shown) v=%s lines=%d parsed=%d malformed=%d%s" % (
        len(starts), version, nlines, len(recs), bad, note)]
    for _, ls in results:
        out += ls
    out.append("RESULT %s (%d FAIL, %d WARN)" % (word, nf, nw))
    if len(out) > MAX_LINES:
        # Keep the RESULT line (always last) and count the detail lines cut before it.
        keep = out[:MAX_LINES - 2]
        out = keep + ["... +%d more lines" % (len(out) - 1 - len(keep)), out[-1]]
    return out, nf


def read_window(path, a):
    """([(line_no, raw)], header note). --new starts after the log_triage bookmark (line 1 when rotated) and advances it
    unless --no-advance. Line numbers stay absolute. Same bookmark rules as queue_hud_log_check.read_window."""
    all_lines = LT.read_lines(path)
    total = len(all_lines)
    start, note = 0, ""
    if a.new:
        st = path.stat()
        key = str(path.resolve())
        state_path = os.path.expanduser(a.state)
        state = LT.load_state(state_path)
        bm = state.get(key)
        rotated = bool(bm) and (bm.get("inode") != st.st_ino or st.st_size < bm.get("size", 0) or bm.get("line", 0) > total)
        if bm and not rotated:
            start = bm.get("line", 0)
        note = (" new=L%d-L%d" % (start + 1, total)) if start < total else " new=none"
        if rotated:
            note += " rotated"
        if not a.no_advance:
            state[key] = dict(inode=st.st_ino, size=st.st_size, line=total)
            LT.save_state(state_path, state)
    return [(i + 1, s) for i, s in enumerate(all_lines) if i >= start], note


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("log_pos", nargs="?")
    ap.add_argument("--log")
    ap.add_argument("--new", action="store_true", help="only lines after the log_triage bookmark, then advance it")
    ap.add_argument("--no-advance", action="store_true", help="with --new: do not move the bookmark")
    ap.add_argument("--state", default=LT.DEFAULT_STATE, help="bookmark file for --new")
    a = ap.parse_args(argv)
    path = Path(a.log or a.log_pos or DEFAULT_LOG)
    if not path.is_file():
        print("no log: %s" % path)
        return 2
    window, note = read_window(path, a)
    if a.new and not window:
        print("NPC-DIV-LOG new=none (no lines since last check)")
        return 0
    recs, bad = parse_lines(window)
    lines, nf = render(recs, bad, len(window), note, a.new)
    print("\n".join(lines))
    return 1 if nf else 0


if __name__ == "__main__":
    sys.exit(main())
