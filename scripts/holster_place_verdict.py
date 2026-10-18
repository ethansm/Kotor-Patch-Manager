#!/usr/bin/env python3
"""Holster place verdict (read-only, cursor-free). Python 3, stdlib only; writes no files.

Usage: holster_place_verdict.py [--log PATH] [--stage baseline|place] [--game-dir DIR]
  Defaults: --game-dir "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II",
            --log <game-dir>/holster_log.txt, --stage baseline.
  Run: lines from the LAST "loaded (P0 probe" line to EOF of the live log. If the live log has no loaded line,
  the tail of <log>.1 from its last loaded line is prepended (RUN source=live+.1). No loaded line anywhere: loaded=no.
  Output: at most 40 lines, each at most 120 chars. Exit 0 = PASS or WARN, 1 = FAIL, 2 = usage error or missing log.
  Import API: parse_run(live_lines, prev_lines) -> (run, source); analyze(run, source, stage) -> (lines, exit_code).
Note: with holster.ini log_verbose=0 (default, quiet mode) PLACE lines are rate-limited and change-biased (first 3, every 200th, any signature or
placement-delta change), so mean_move is computed over a change-biased sample, not every placement; use log_verbose=1 for an unbiased mean.
Keywords are found with re.search, so any log prefix (T=, f=, holster/main) is fine.
"""
import argparse
import math
import os
import re
import sys

DEFAULT_GAME_DIR = "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
MAX_COLS = 120
MAX_RELOAD_LINES = 12
MAX_SEG_LINES = 12
UNIT_TOL = 1e-3  # PLACE quat1 must satisfy |norm - 1| < UNIT_TOL

NUM = r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?"
VEC3 = r"\(\s*(%s)\s*,\s*(%s)\s*,\s*(%s)\s*\)" % (NUM, NUM, NUM)
VEC4 = r"\(\s*(%s)\s*,\s*(%s)\s*,\s*(%s)\s*,\s*(%s)\s*\)" % (NUM, NUM, NUM, NUM)

LOADED_RE = re.compile(r"loaded \(P0 probe")
LOADED_FIELDS = {k: re.compile(r"\b%s=(\S+)" % k) for k in ("probe", "channels", "max_lines", "table")}
BT_PATCHED_RE = re.compile(r"BONETEST patched\s+(.+?)\s*$")
BT_REFUSED_RE = re.compile(r"BONETEST refused")
TID_GOB_RE = re.compile(r"\bTID gobupdate=(\S+)")
TID_PLACE_RE = re.compile(r"\bTID place=(\S+)")
RELOAD_RE = re.compile(r"\bRELOAD place=(\S+)")
RELOAD_FULL_RE = re.compile(r"\bRELOAD place=(\S+)\s+rot=" + VEC3 + r"\s+off=" + VEC3)
PLACE_RE = re.compile(r"\bPLACE n=")
POS0_RE = re.compile(r"\bpos0=" + VEC3)
QUAT0_RE = re.compile(r"\bquat0=" + VEC4)
POS1_RE = re.compile(r"\bpos1=" + VEC3)
QUAT1_RE = re.compile(r"\bquat1=" + VEC4)
COMB_RE = re.compile(r"\bCOMB creature=")
ATTACH_RE = re.compile(r"\bATTACH obj=")
SWAP_RE = re.compile(r"\bSWAP exec=")
CAP_RE = re.compile(r"\bcap reached max_lines=")


def clip(text):
    return text if len(text) <= MAX_COLS else text[:MAX_COLS - 3] + "..."


def read_lines(path):
    with open(path, encoding="utf-8", errors="replace") as fh:
        return [ln.rstrip("\r\n") for ln in fh]


def last_loaded_index(lines):
    found = None
    for i, ln in enumerate(lines):
        if LOADED_RE.search(ln):
            found = i
    return found


def parse_run(live, prev):
    """Return (run_lines, source). Run = last loaded line to EOF; falls back to the .1 tail if live has none."""
    i = last_loaded_index(live)
    if i is not None:
        return live[i:], "live"
    j = last_loaded_index(prev)
    if j is not None:
        return prev[j:] + live, "live+.1"
    return live, "live"


def parse_loaded(run):
    info = {"probe": "n/a", "channels": "n/a", "max_lines": "n/a", "table": "n/a"}
    i = last_loaded_index(run)
    if i is not None:
        for key, rx in LOADED_FIELDS.items():
            m = rx.search(run[i])
            if m:
                info[key] = m.group(1)
    return info


def parse_reload(ln):
    m = RELOAD_FULL_RE.search(ln)
    if m:
        rot, off = m.group(2, 3, 4), m.group(5, 6, 7)
        return {"place": m.group(1), "rot": rot, "off": off,
                "nonzero": any(float(v) != 0.0 for v in rot + off)}
    # RELOAD line whose rot/off do not parse: still counted, never treated as moving
    return {"place": RELOAD_RE.search(ln).group(1), "rot": ("?", "?", "?"),
            "off": ("?", "?", "?"), "nonzero": False}


def parse_place(ln):
    """Return (move, quat_ok) for one PLACE line. move = max(|pos1-pos0|, |quat1-quat0|); None if unparsed."""
    m_p0, m_q0, m_p1, m_q1 = POS0_RE.search(ln), QUAT0_RE.search(ln), POS1_RE.search(ln), QUAT1_RE.search(ln)
    if not (m_p0 and m_q0 and m_p1 and m_q1):
        return None, False
    pos0 = [float(v) for v in m_p0.groups()]
    quat0 = [float(v) for v in m_q0.groups()]
    pos1 = [float(v) for v in m_p1.groups()]
    quat1 = [float(v) for v in m_q1.groups()]
    move = max(math.dist(pos0, pos1), math.dist(quat0, quat1))
    quat_ok = abs(math.sqrt(sum(v * v for v in quat1)) - 1.0) < UNIT_TOL
    return move, quat_ok


def _new_seg(idx, reload):
    return {"idx": idx, "place": reload["place"] if reload else "?",
            "nonzero": bool(reload and reload["nonzero"]),
            "n": 0, "move_sum": 0.0, "moves": 0, "quat_ok": True}


def _seg_mean(seg):
    return seg["move_sum"] / seg["moves"] if seg["moves"] else 0.0


def analyze(run, source, stage):
    """Return (output_lines, exit_code). Read-only: works only on the run lines given."""
    info = parse_loaded(run)
    loaded = last_loaded_index(run) is not None
    channels = [c for c in info["channels"].split(",") if c]
    place_on = "place" in channels

    patched, n_patched, n_refused = [], 0, 0
    tid_gob = tid_place = None
    n_comb = n_attach = n_swap = n_place = 0
    cap = False
    reloads = []
    segs = [_new_seg(0, None)]  # segs[0] = PLACE lines before any RELOAD; segs[k] = after RELOAD k

    for ln in run:
        m = BT_PATCHED_RE.search(ln)
        if m:
            n_patched += 1
            if len(patched) < 3:
                patched.append(m.group(1))
        if BT_REFUSED_RE.search(ln):
            n_refused += 1
        m = TID_GOB_RE.search(ln)
        if m and tid_gob is None:
            tid_gob = m.group(1)
        m = TID_PLACE_RE.search(ln)
        if m and tid_place is None:
            tid_place = m.group(1)
        n_comb += bool(COMB_RE.search(ln))
        n_attach += bool(ATTACH_RE.search(ln))
        n_swap += bool(SWAP_RE.search(ln))
        cap = cap or bool(CAP_RE.search(ln))
        if RELOAD_RE.search(ln):
            rel = parse_reload(ln)
            reloads.append(rel)
            segs.append(_new_seg(len(reloads), rel))
        elif PLACE_RE.search(ln):
            n_place += 1
            seg = segs[-1]
            seg["n"] += 1
            move, quat_ok = parse_place(ln)
            if move is not None:
                seg["move_sum"] += move
                seg["moves"] += 1
            if not quat_ok:
                seg["quat_ok"] = False

    moving = any(s["idx"] >= 1 and s["nonzero"] and _seg_mean(s) > 0 for s in segs)
    fails, warns = [], []
    if not loaded:
        fails.append("no loaded line")
    if n_patched < 1:
        fails.append("BONETEST patched=0")
    if n_refused > 0:
        fails.append("BONETEST refused=%d" % n_refused)
    if tid_place is None:
        fails.append("no TID place")
    if cap:
        warns.append("cap reached")
    if tid_gob is None:
        warns.append("no TID gobupdate")
    if stage == "place":
        if not place_on:
            fails.append("no place channel")
        if not any(r["place"] == "1" for r in reloads):
            fails.append("no RELOAD place=1")
        if n_place == 0:
            fails.append("PLACE n=0")
        bad = ["SEG#%d" % s["idx"] for s in segs if not s["quat_ok"]]
        if bad:
            fails.append("bad quat1 in " + ",".join(bad))
        if not moving:
            fails.append("no moving RELOAD segment")

    if fails:
        verdict, rc = "FAIL", 1
    elif warns:
        verdict, rc = "WARN", 0
    else:
        verdict, rc = "PASS", 0
    reasons = "; ".join(fails + warns) or "ok"

    out = ["RUN loaded=%s source=%s lines=%d probe=%s channels=%s table=%s" % (
        "yes" if loaded else "no", source, len(run), info["probe"], info["channels"], info["table"])]
    bt = "BONETEST patched=%d refused=%d" % (n_patched, n_refused)
    if patched:
        bt += " " + ";".join(patched)
    out.append(bt)
    out.append("TID gobupdate=%s place=%s" % (tid_gob or "missing", tid_place or "missing"))
    out.append("RELOAD n=%d" % len(reloads))
    for i, r in enumerate(reloads[:MAX_RELOAD_LINES], 1):
        out.append("RELOAD#%d place=%s rot=(%s) off=(%s)" % (
            i, r["place"], ",".join(r["rot"]), ",".join(r["off"])))
    if len(reloads) > MAX_RELOAD_LINES:
        out.append("RELOAD ... %d more" % (len(reloads) - MAX_RELOAD_LINES))
    if n_place == 0 and not place_on:
        out.append("PLACE n/a: ch_place off")
    else:
        out.append("PLACE n=%d" % n_place)
    show = [segs[0]] if segs[0]["n"] > 0 else []
    show.extend(segs[1:1 + MAX_SEG_LINES])
    for s in show:
        out.append("SEG#%d place=%s n=%d mean_move=%.4f quat_ok=%s" % (
            s["idx"], s["place"], s["n"], _seg_mean(s), "yes" if s["quat_ok"] else "no"))
    if len(segs) - 1 > MAX_SEG_LINES:
        out.append("SEG ... %d more" % (len(segs) - 1 - MAX_SEG_LINES))
    out.append("CHAN COMB=%d ATTACH=%d SWAP=%d" % (n_comb, n_attach, n_swap))
    out.append("CAP %s" % ("reached" if cap else "none"))
    out.append("VERDICT %s: %s: %s" % (stage, verdict, reasons))
    return [clip(x) for x in out], rc


def main(argv=None):
    ap = argparse.ArgumentParser(description="Read-only holster place verdict for the KOTOR II holster mod log.")
    ap.add_argument("--log", default=None, help="log path (default: <game-dir>/holster_log.txt)")
    ap.add_argument("--stage", choices=("baseline", "place"), default="baseline")
    ap.add_argument("--game-dir", default=DEFAULT_GAME_DIR)
    args = ap.parse_args(argv)
    log_path = args.log or os.path.join(args.game_dir, "holster_log.txt")
    if not os.path.isfile(log_path):
        print("no log: %s" % log_path, file=sys.stderr)
        return 2
    try:
        live = read_lines(log_path)
    except OSError as exc:
        print("no log: %s (%s)" % (log_path, exc.strerror), file=sys.stderr)
        return 2
    prev = []
    prev_path = log_path + ".1"
    if os.path.isfile(prev_path):
        try:
            prev = read_lines(prev_path)
        except OSError:
            prev = []
    run, source = parse_run(live, prev)
    lines, rc = analyze(run, source, args.stage)
    print("\n".join(lines))
    return rc


if __name__ == "__main__":
    sys.exit(main())
