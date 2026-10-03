#!/usr/bin/env python3
"""Read-only offline analyzer for dialogue_letterbox_fix_log.txt (mod k2-dialogue-letterbox-fix 1.0.0).

Usage: analyze_engfix_logs.py <logfile> [--json]

Log format (one event per line, `[GetTickCount] text`) is defined by
dialogue_letterbox_fix_src/K2DialogueLetterboxFix.cpp and doc 17 "Config and log (L1)".
The log is split into launches on `loaded:` lines; per launch the analyzer checks:
  * load line: vanilla start bytes `00 00 00 80 aa aa 02 40` (else FAIL)
  * APPLY lines: W, H, newBar, verify OK / unchanged / DISABLED (plus internal consistency: X, newBar, fs, panelH)
  * error lines: FOREIGN, INVALID, protect failures, MISMATCH, null, not readable, UNREADABLE
  * SETTOP: start h == 0, target h == newBar of the current APPLY (mode(+0x88) is logged BEFORE the store: not checked)
  * SETBOTTOM: start y == H, target h == newBar, target y == H - max(newBar, trunc(100*fs)),
    fs = 1 + 0.5*floor(max(W-1022,0)/1024)
  * SETTOP/SETBOTTOM W/H/c fields are cross-checked against the current APPLY
  * LATE fallback use is reported; log-cap lines set a (non-failing) cap flag
Exit code 0 = PASS (including "no dialogue yet"), 1 = FAIL. Nothing is written anywhere.
"""
import json
import re
import sys

VANILLA_BYTES = "00 00 00 80 aa aa 02 40"

LINE_RE = re.compile(r"^\[(\d+)\]\s?(.*)$")
LOAD_RE = re.compile(
    r"^(?P<ver>\S+ \S+) loaded: enabled=(?P<enabled>\d+) mode=(?P<mode>\S+) log=(?P<log>\d+) "
    r"const@0x(?P<va>[0-9a-fA-F]+)=(?P<bytes>(?:[0-9a-fA-F]{2} ){7}[0-9a-fA-F]{2}) "
    r"\((?P<val>[^,)]*)(?P<unreadable>, UNREADABLE)?\) pid=(?P<pid>\d+)$")
APPLY_HDR_RE = re.compile(
    r"^APPLY src=(?P<src>\S+) W=(?P<w>-?\d+) H=(?P<h>-?\d+) mode=(?P<mode>\S+) old=(?P<old>\S+) new=(?P<new>\S+) "
    r"vanillaBar=(?P<vbar>-?\d+) newBar=(?P<nbar>-?\d+) X=(?P<x>-?\d+) fs=(?P<fs>\S+) panelH=(?P<panel>-?\d+) "
    r"replyY=(?P<replyy>-?\d+)$")
# "APPLY src=X W=.. H=..: <reason>" (DISABLED / INVALID / FOREIGN / not readable)
APPLY_WH_RE = re.compile(r"^APPLY src=(?P<src>\S+) W=(?P<w>-?\d+) H=(?P<h>-?\d+): (?P<rest>.*)$")
# "APPLY src=X: <result>" (follow-up of a header)
APPLY_RES_RE = re.compile(r"^APPLY src=(?P<src>\S+): (?P<rest>.*)$")
WROTE_RE = re.compile(r"^wrote \(.*\); verify (?P<v>OK|MISMATCH), now ")
SET_RE = re.compile(
    r"^(?P<what>SETTOP|SETBOTTOM) lb=(?P<lb>\S+) mode\(\+0x88, before this store\)=(?P<mode>-?\d+) "
    r"start\(x,y,w,h\)=\((?P<sx>-?\d+),(?P<sy>-?\d+),(?P<sw>-?\d+),(?P<sh>-?\d+)\) "
    r"target\(x,y,w,h\)=\((?P<tx>-?\d+),(?P<ty>-?\d+),(?P<tw>-?\d+),(?P<th>-?\d+)\) "
    r"mgr=(?P<mgr>\S+) W=(?P<w>-?\d+) H=(?P<h>-?\d+) c=(?P<c>\S+)$")
LATE_RE = re.compile(r"^(?P<what>SETTOP|SETBOTTOM): no SetScreenSize hit seen yet, applying late fallback")
SETCAP_RE = re.compile(r"^SETTOP/SETBOTTOM log cap \((?P<n>\d+) lines\) reached, further lines dropped$")
TOTCAP_RE = re.compile(r"^log cap \((?P<n>\d+) lines\) reached, further lines dropped$")
UNKMODE_RE = re.compile(r"^unknown mode '(?P<m>[^']*)' in dialogue_letterbox_fix\.ini, using h6$")
NULLMGR_RE = re.compile(r"^dlbOnScreenSize: manager pointer \[ebp-0xC\] is null, skipping$")
VPROT_RE = re.compile(r"^APPLY src=(?P<src>\S+): VirtualProtect\((?P<which>RW|restore)\) FAILED err=(?P<err>\d+)")
MISMATCH_RE = re.compile(r"^APPLY src=(?P<src>\S+): verify MISMATCH after write$")


def fs_for(w):
    return 1.0 + 0.5 * (max(w - 1022, 0) // 1024)


def trunc_div2(n):
    return int(n / 2.0)  # truncation toward zero, like the DLL's static_cast<int>(x / 2.0)


def classify_error(text):
    """Generic error-line detector, independent of the parser branches. Returns a reason or None."""
    low = text.lower()
    if "foreign" in low:
        return "FOREIGN"
    if "invalid" in low:
        return "INVALID"
    if "mismatch" in low:
        return "MISMATCH"
    if "null" in low:
        return "null pointer"
    if "protect" in low and ("ok" not in low or "fail" in low):
        return "protect failure"
    if "not readable" in low or "unreadable" in low:
        return "constant not readable"
    if " failed" in low:
        return "FAILED"
    return None


class Launch:
    def __init__(self, index, line_no):
        self.index = index
        self.line_no = line_no
        self.pid = None
        self.mode = None
        self.enabled = None
        self.log = None
        self.start_bytes = None
        self.has_load = False
        self.applies = []       # dicts
        self.settop = 0
        self.setbottom = 0
        self.pre_apply_sets = 0  # SET lines logged before any APPLY (late-fallback path)
        self.late = []          # e.g. ["SetTop"]
        self.cap = []           # cap notes
        self.errors = []        # (line_no, reason, text)
        self.warnings = []      # (line_no, text)
        self.checked_sets = 0

    def err(self, line_no, reason, text):
        self.errors.append((line_no, reason, text))

    def warn(self, line_no, text):
        self.warnings.append((line_no, text))

    def current_apply(self):
        for a in reversed(self.applies):
            if a.get("result") in ("verify_ok", "unchanged"):
                return a
        return None

    def last_apply_with_bar(self):
        for a in reversed(self.applies):
            if "newBar" in a:
                return a
        return None

    def wh(self):
        a = self.last_apply_with_bar() or (self.applies[-1] if self.applies else None)
        return (a["W"], a["H"]) if a else (None, None)

    def newbar(self):
        a = self.last_apply_with_bar()
        return a["newBar"] if a else None

    @property
    def verdict(self):
        return "FAIL" if self.errors else "PASS"


def analyze_text(text):
    launches = []
    unparsed = []
    cur = None
    total_lines = 0

    def ensure(line_no):
        nonlocal cur
        if cur is None:
            cur = Launch(len(launches) + 1, line_no)
            launches.append(cur)
        return cur

    for line_no, raw in enumerate(text.splitlines(), 1):
        if not raw.strip():
            continue
        total_lines += 1
        m = LINE_RE.match(raw.rstrip("\r"))
        if not m:
            unparsed.append((line_no, raw))
            continue
        body = m.group(2)

        if " loaded:" in body:
            lm = LOAD_RE.match(body)
            cur = Launch(len(launches) + 1, line_no)
            launches.append(cur)
            cur.has_load = True
            if not lm:
                cur.err(line_no, "unparseable load line", body)
                continue
            cur.pid = int(lm.group("pid"))
            cur.mode = lm.group("mode")
            cur.enabled = int(lm.group("enabled"))
            cur.log = int(lm.group("log"))
            cur.start_bytes = lm.group("bytes").lower()
            if lm.group("unreadable"):
                cur.err(line_no, "constant UNREADABLE at load", body)
            elif cur.start_bytes != VANILLA_BYTES:
                cur.err(line_no, "start bytes not vanilla (%s)" % cur.start_bytes, body)
            if cur.enabled == 0:
                cur.warn(line_no, "enabled=0: the fix never writes")
            continue

        L = ensure(line_no)

        # Generic error screen first; parser branches below still run so the state stays consistent.
        reason = classify_error(body)
        if reason:
            L.err(line_no, reason, body)

        # --- APPLY header (full numbers)
        hm = APPLY_HDR_RE.match(body)
        if hm:
            a = {"line": line_no, "src": hm.group("src"), "W": int(hm.group("w")), "H": int(hm.group("h")),
                 "mode": hm.group("mode"), "old": hm.group("old"), "new": float(hm.group("new")),
                 "vanillaBar": int(hm.group("vbar")), "newBar": int(hm.group("nbar")), "X": int(hm.group("x")),
                 "fs": float(hm.group("fs")), "panelH": int(hm.group("panel")), "replyY": int(hm.group("replyy")),
                 "result": None}
            L.applies.append(a)
            if a["src"].startswith("LATE("):
                L.late.append(a["src"][5:-1])
            # internal consistency of the logged numbers
            if a["new"] != 0:
                x = int(a["W"] / a["new"])
                if x != a["X"]:
                    L.err(line_no, "APPLY X=%d != trunc(W/new)=%d" % (a["X"], x), body)
                bar = trunc_div2(a["H"] - a["X"])
                if bar != a["newBar"]:
                    L.err(line_no, "APPLY newBar=%d != trunc((H-X)/2)=%d" % (a["newBar"], bar), body)
            if abs(a["fs"] - fs_for(a["W"])) > 1e-9:
                L.err(line_no, "APPLY fs=%s != formula %.1f" % (hm.group("fs"), fs_for(a["W"])), body)
            if a["panelH"] != int(100.0 * fs_for(a["W"])):
                L.err(line_no, "APPLY panelH=%d != trunc(100*fs)=%d" % (a["panelH"], int(100.0 * fs_for(a["W"]))), body)
            continue

        # --- APPLY with W/H then ": reason" (no header)
        wm = APPLY_WH_RE.match(body)
        if wm:
            rest = wm.group("rest")
            a = {"line": line_no, "src": wm.group("src"), "W": int(wm.group("w")), "H": int(wm.group("h")),
                 "result": "other"}
            if rest.startswith("DISABLED"):
                a["result"] = "disabled"
            elif rest.startswith("INVALID"):
                a["result"] = "invalid"
            elif rest.startswith("FOREIGN"):
                a["result"] = "foreign"
            elif "not readable" in rest:
                a["result"] = "unreadable"
            L.applies.append(a)
            if a["src"].startswith("LATE("):
                L.late.append(a["src"][5:-1])
            continue

        # --- APPLY follow-up lines ("APPLY src=X: ...")
        rm = APPLY_RES_RE.match(body)
        if rm:
            rest = rm.group("rest")
            src = rm.group("src")
            target = None
            for a in reversed(L.applies):
                if a["src"] == src and a.get("result") is None:
                    target = a
                    break
            res = None
            if rest.startswith("unchanged"):
                res = "unchanged"
            else:
                wr = WROTE_RE.match(rest)
                if wr:
                    res = "verify_ok" if wr.group("v") == "OK" else "verify_mismatch"
                elif VPROT_RE.match(body):
                    res = "protect_fail"
                elif MISMATCH_RE.match(body):
                    res = None  # follows the wrote line; already an error via the generic screen
                    if target is None:
                        for a in reversed(L.applies):
                            if a["src"] == src:
                                a["result"] = "verify_mismatch"
                                break
                else:
                    L.warn(line_no, "unrecognised APPLY follow-up: " + body)
            if res and target is not None:
                target["result"] = res
            elif res and target is None:
                L.warn(line_no, "APPLY result without a matching header: " + body)
            continue

        # --- SETTOP / SETBOTTOM
        sm = SET_RE.match(body)
        if sm:
            what = sm.group("what")
            g = {k: sm.group(k) for k in sm.groupdict()}
            if what == "SETTOP":
                L.settop += 1
            else:
                L.setbottom += 1
            cur_apply = L.current_apply()
            if cur_apply is None:
                if not L.applies:
                    L.pre_apply_sets += 1  # late-fallback path: bar was computed with the old constant
                    # start rect still must be right
                    if what == "SETTOP" and int(g["sh"]) != 0:
                        L.err(line_no, "SETTOP start h %s != 0" % g["sh"], body)
                    if what == "SETBOTTOM" and int(g["sy"]) != int(g["h"]):
                        L.err(line_no, "SETBOTTOM start y %s != H %s" % (g["sy"], g["h"]), body)
                else:
                    L.warn(line_no, "%s after an APPLY that did not write (%s): bar not checked"
                           % (what, L.applies[-1].get("result")))
                continue
            L.checked_sets += 1
            nbar, W, H = cur_apply["newBar"], cur_apply["W"], cur_apply["H"]
            if int(g["w"]) != W or int(g["h"]) != H:
                L.err(line_no, "%s W/H %sx%s != APPLY %dx%d" % (what, g["w"], g["h"], W, H), body)
            try:
                c = float(g["c"])
                if abs(c - cur_apply["new"]) > 1e-9 * max(1.0, abs(cur_apply["new"])):
                    L.err(line_no, "%s c=%s != APPLY new=%.17g" % (what, g["c"], cur_apply["new"]), body)
            except ValueError:
                L.err(line_no, "%s c not a number: %s" % (what, g["c"]), body)
            if int(g["th"]) != nbar:
                L.err(line_no, "%s target h %s != newBar %d" % (what, g["th"], nbar), body)
            if what == "SETTOP":
                if int(g["sh"]) != 0:
                    L.err(line_no, "SETTOP start h %s != 0" % g["sh"], body)
            else:
                if int(g["sy"]) != H:
                    L.err(line_no, "SETBOTTOM start y %s != H %d" % (g["sy"], H), body)
                exp_y = H - max(nbar, int(100.0 * fs_for(W)))
                if int(g["ty"]) != exp_y:
                    L.err(line_no, "SETBOTTOM target y %s != H - max(newBar, trunc(100*fs)) = %d" % (g["ty"], exp_y), body)
            continue

        # --- LATE fallback notice, cap lines, config notes, null manager
        lm2 = LATE_RE.match(body)
        if lm2:
            continue  # the APPLY src=LATE(...) line carries the reporting
        cm = SETCAP_RE.match(body) or TOTCAP_RE.match(body)
        if cm:
            L.cap.append("set-cap(%s)" % cm.group("n") if SETCAP_RE.match(body) else "log-cap(%s)" % cm.group("n"))
            continue
        if UNKMODE_RE.match(body):
            L.warn(line_no, body)
            continue
        if NULLMGR_RE.match(body):
            continue  # already an error via the generic screen

        if not reason:
            unparsed.append((line_no, raw))

    for L in launches:
        if L.has_load and not L.applies:
            if L.log == 0:
                L.warn(L.line_no, "no APPLY lines (log=0 suppresses them)")
            else:
                L.warn(L.line_no, "no APPLY line in this launch")
        for a in L.applies:
            if a.get("result") is None:
                L.warn(a["line"], "APPLY src=%s has no result line (log cut off?)" % a["src"])
        if L.pre_apply_sets and not L.late:
            L.warn(L.line_no, "%d SET line(s) before the first APPLY but no LATE fallback seen" % L.pre_apply_sets)

    return launches, unparsed, total_lines


def summarize(launches, unparsed, total_lines):
    dialogues = sum(L.settop for L in launches)
    errors = sum(len(L.errors) for L in launches)
    reasons = []
    if not launches:
        reasons.append("no launch / load line found")
    if errors:
        counts = {}
        for L in launches:
            for _, r, _ in L.errors:
                key = re.sub(r"[\d.]+", "#", r).split(" (")[0]
                counts[key] = counts.get(key, 0) + 1
        reasons.append("%d error line(s): %s" % (errors, ", ".join("%s x%d" % kv for kv in sorted(counts.items()))))
    failed_launches = [L.index for L in launches if L.errors]
    if failed_launches:
        reasons.append("failing launch(es) " + ",".join("#%d" % i for i in failed_launches))
    cap = any(L.cap for L in launches)
    if reasons:
        verdict = "VERDICT: FAIL (%s)" % "; ".join(reasons)
        ok = False
    elif dialogues == 0:
        verdict = "VERDICT: PASS - no dialogue yet (%d launches, 0 SETTOP lines, 0 errors)" % len(launches)
        ok = True
    else:
        verdict = "VERDICT: PASS (%d launches, %d dialogues, 0 errors)" % (len(launches), dialogues)
        ok = True
    if ok and cap:
        verdict += " [cap reached: counts may be truncated]"
    return ok, verdict, dialogues, errors


def render_text(path, launches, unparsed, total_lines, ok, verdict, dialogues):
    out = []
    out.append("File: %s (%d non-empty lines, %d launches)" % (path, total_lines, len(launches)))
    out.append("")
    hdr = ("#", "pid", "mode", "WxH", "newBar", "dlg", "applies", "late", "cap", "errs", "verdict")
    rows = []
    for L in launches:
        w, h = L.wh()
        nb = L.newbar()
        results = [a.get("result") or "?" for a in L.applies]
        ap = "/".join(sorted(set(results))) if results else "-"
        ap = "%d:%s" % (len(L.applies), ap.replace("verify_ok", "verifyOK"))
        rows.append((str(L.index), str(L.pid) if L.pid is not None else "-", L.mode or "-",
                     "%sx%s" % (w, h) if w is not None else "-", str(nb) if nb is not None else "-",
                     str(L.settop), ap, ",".join(L.late) or "no", ",".join(L.cap) or "no",
                     str(len(L.errors)), L.verdict))
    widths = [max(len(hdr[i]), *(len(r[i]) for r in rows)) if rows else len(hdr[i]) for i in range(len(hdr))]
    fmt = "  ".join("%%-%ds" % w for w in widths)
    out.append(fmt % hdr)
    for r in rows:
        out.append(fmt % r)
    out.append("")
    fails = [(L, e) for L in launches for e in L.errors]
    if fails:
        out.append("Failing lines:")
        for L, (ln, reason, text) in fails:
            out.append("  launch #%d line %d [%s]: %s" % (L.index, ln, reason, text))
        out.append("")
    warns = [(L, w) for L in launches for w in L.warnings]
    if warns:
        out.append("Warnings:")
        for L, (ln, text) in warns:
            out.append("  launch #%d line %d: %s" % (L.index, ln, text))
        out.append("")
    pre = sum(L.pre_apply_sets for L in launches)
    if pre:
        out.append("Note: %d SETTOP/SETBOTTOM line(s) precede the first APPLY (late fallback path); bar not checked for those." % pre)
        out.append("")
    if unparsed:
        out.append("Unparsed lines (%d, warning only):" % len(unparsed))
        for ln, raw in unparsed:
            out.append("  line %d: %s" % (ln, raw))
        out.append("")
    out.append(verdict)
    return "\n".join(out)


def build_json(path, launches, unparsed, total_lines, ok, verdict, dialogues):
    return {
        "file": path, "lines": total_lines, "ok": ok, "verdict": verdict,
        "launches": len(launches), "dialogues": dialogues,
        "unparsed": [{"line": ln, "text": t} for ln, t in unparsed],
        "launch_details": [{
            "index": L.index, "line": L.line_no, "pid": L.pid, "mode": L.mode, "enabled": L.enabled, "log": L.log,
            "start_bytes": L.start_bytes, "dialogues": L.settop, "setbottom": L.setbottom,
            "late": L.late, "cap": L.cap, "verdict": L.verdict,
            "applies": L.applies,
            "errors": [{"line": ln, "reason": r, "text": t} for ln, r, t in L.errors],
            "warnings": [{"line": ln, "text": t} for ln, t in L.warnings],
        } for L in launches],
    }


def analyze_file(path):
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()
    return analyze_text(text)


def main(argv):
    args = [a for a in argv[1:] if a != "--json"]
    as_json = "--json" in argv[1:]
    if len(args) != 1:
        sys.stderr.write("usage: analyze_engfix_logs.py <logfile> [--json]\n")
        return 2
    path = args[0]
    try:
        launches, unparsed, total = analyze_file(path)
    except OSError as e:
        sys.stderr.write("cannot read %s: %s\n" % (path, e))
        return 2
    ok, verdict, dialogues, _ = summarize(launches, unparsed, total)
    if as_json:
        print(json.dumps(build_json(path, launches, unparsed, total, ok, verdict, dialogues), indent=2))
    else:
        print(render_text(path, launches, unparsed, total, ok, verdict, dialogues))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
