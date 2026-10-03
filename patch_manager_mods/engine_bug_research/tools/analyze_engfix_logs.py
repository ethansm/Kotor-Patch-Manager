#!/usr/bin/env python3
"""Read-only offline analyzer for dialogue_letterbox_fix_log.txt (mod k2-dialogue-letterbox-fix 1.0.0 / 1.0.1 / 1.0.1-probe).

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
log=0 launches (1.0.1 default): only the first APPLY pair and the first SETTOP/SETBOTTOM are logged, so later APPLYs
(resolution change, credits 640x480 + restore) are invisible. Such launches are NOT compared against the last APPLY; each
SETTOP/SETBOTTOM line is checked for self-consistency instead (bar = trunc((H - trunc(W/c))/2) == target h, SETBOTTOM
start y == H and target y == H - max(target h, trunc(100*fs))). A c-vs-target-h inconsistency is a WARN there, not a FAIL.
`PROBE W1..W5` lines (1.0.1-probe, probe=1) are informational: they are bucketed per dialogue (a dialogue starts at a
SETTOP line; earlier lines go to a "pre-dialogue" bucket) and summarised with a one-line diagnosis per dialogue
(V8b (c): draw not called / predicate fails / alpha 0 / fill rejected / bars drawn). They never fail a launch.
Exit code 0 = PASS (including "no dialogue yet"), 1 = FAIL. Nothing is written anywhere.
"""
import json
import re
import sys

VANILLA_BYTES = "00 00 00 80 aa aa 02 40"

LINE_RE = re.compile(r"^\[(\d+)\]\s?(.*)$")
LOAD_RE = re.compile(
    r"^(?P<ver>\S+ \S+) loaded: enabled=(?P<enabled>\d+) mode=(?P<mode>\S+) log=(?P<log>\d+) (?:probe=(?P<probe>\d+) )?"
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

# ---- PROBE W1..W5 lines (formats: dialogue_letterbox_fix_probe_src/K2DialogueLetterboxFix.cpp)
PROBE_RE = re.compile(r"^PROBE (?P<tag>W[1-5]) n=(?P<n>\d+) skip=(?P<skip>\d+) (?P<rest>.*)$")
PROBE_CAP_RE = re.compile(r"^PROBE cap \((?P<n>\d+) lines\) reached, further PROBE lines dropped$")
_R4 = r"\((?P<%s>-?\d+),(?P<%s>-?\d+),(?P<%s>-?\d+),(?P<%s>-?\d+)\)"
W12_RE = re.compile(
    r"^this=0x(?P<this>[0-9a-fA-F]+) mode=(?P<mode>-?\d+) done=(?P<done>-?\d+) alpha=0x(?P<alpha>[0-9a-fA-F]+) "
    r"passedRect\(x,y,w,h\)=" + _R4 % ("px", "py", "pw", "ph") + r" ctrlRect\(x,y,w,h\)=" + _R4 % ("cx", "cy", "cw", "ch")
    + r" targetH\(\+0x74\)=(?P<th>-?\d+)(?P<unr> \[UNREADABLE\])?$")
W3_RE = re.compile(
    r"^this=0x(?P<this>[0-9a-fA-F]+) vtbl=0x(?P<vtbl>[0-9a-fA-F]+)\((?P<vkind>letterbox|OTHER)\) "
    r"flags\(\+0x48\)=0x(?P<flags>[0-9a-fA-F]+) bit2=(?P<bit2>\d+) bit6=(?P<bit6>\d+) rect\(x,y,w,h\)=" + _R4 % ("x", "y", "w", "h")
    + r" alpha=0x(?P<alpha>[0-9a-fA-F]+) color\(\+0x54\.\.\)=0x[0-9a-fA-F]+,0x[0-9a-fA-F]+,0x[0-9a-fA-F]+ "
    r"mgr=0x(?P<mgr>[0-9a-fA-F]+) W=(?P<mw>-?\d+) H=(?P<mh>-?\d+) predicate=(?P<pred>\S+) dt=0x(?P<dt>[0-9a-fA-F]+) "
    r"DAT_a309b4=(?P<d4>-?\d+) DAT_a309b8=(?P<d8>-?\d+) DAT_a309bc=(?P<dc>-?\d+)(?P<unr> \[SOME-UNREADABLE\])?$")
W3_NULL_RE = re.compile(r"^this=NULL/UNREADABLE dt=0x(?P<dt>[0-9a-fA-F]+)$")
W4_RE = re.compile(
    r"^this=0x(?P<this>[0-9a-fA-F]+) mode=(?P<mode>-?\d+) eax=(?P<eax>-?\d+) fillRect\(x,y,w,h\)=" + _R4 % ("x", "y", "w", "h")
    + r" alpha=0x(?P<alpha>[0-9a-fA-F]+)(?P<unr> \[UNREADABLE\])?$")
W5_RE = re.compile(r"^arg=(?P<arg>-?\d+) this=0x(?P<this>[0-9a-fA-F]+)(?P<unr> \[UNREADABLE\])?$")

ALPHA_LABELS = {0x3F000000: "0.5 (semi-transparent letterbox)", 0x3F800000: "1.0", 0: "0 (alpha 0)"}


def alpha_label(bits):
    return "0x%08X %s" % (bits, ALPHA_LABELS[bits]) if bits in ALPHA_LABELS else "0x%08X" % bits


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


def new_bucket(index, line_no):
    return {"index": index, "settop_line": line_no, "W1": [], "W2": [], "W3": [], "W4": [], "W5": []}


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
        self.probe = None        # probe=0|1 from the load line (1.0.1-probe), else None
        self.probe_lines = 0
        self.pre_bucket = new_bucket(0, None)   # PROBE lines before any SETTOP
        self.buckets = []        # one per SETTOP line

    def new_dialogue(self, line_no):
        self.buckets.append(new_bucket(len(self.buckets) + 1, line_no))

    def cur_bucket(self):
        return self.buckets[-1] if self.buckets else self.pre_bucket

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


def check_set_self_consistent(L, line_no, what, g, body):
    """log=0 launch: later APPLYs are not logged, so check the SETTOP/SETBOTTOM line against itself only."""
    L.checked_sets += 1
    W, H, th = int(g["w"]), int(g["h"]), int(g["th"])
    try:
        c = float(g["c"])
        if c > 0:
            bar = trunc_div2(H - int(W / c))
            if bar != th:
                L.warn(line_no, "%s (log=0): c=%s gives bar trunc((H-trunc(W/c))/2)=%d but target h=%d (W=%d H=%d); "
                       "not compared with the (unlogged) later APPLYs" % (what, g["c"], bar, th, W, H))
        else:
            L.warn(line_no, "%s (log=0): c=%s not positive" % (what, g["c"]))
    except ValueError:
        L.err(line_no, "%s c not a number: %s" % (what, g["c"]), body)
    if what == "SETTOP":
        if int(g["sh"]) != 0:
            L.err(line_no, "SETTOP start h %s != 0" % g["sh"], body)
    else:
        if int(g["sy"]) != H:
            L.err(line_no, "SETBOTTOM start y %s != H %d" % (g["sy"], H), body)
        exp_y = H - max(th, int(100.0 * fs_for(W)))
        if int(g["ty"]) != exp_y:
            L.err(line_no, "SETBOTTOM target y %s != H - max(target h, trunc(100*fs)) = %d" % (g["ty"], exp_y), body)


def parse_probe(L, line_no, body):
    """Parse one `PROBE Wn ...` line into the current dialogue bucket. False = unrecognised format."""
    pm = PROBE_RE.match(body)
    if not pm:
        return False
    tag, rest = pm.group("tag"), pm.group("rest")
    rec = {"line": line_no, "n": int(pm.group("n")), "skip": int(pm.group("skip"))}
    if tag in ("W1", "W2"):
        m = W12_RE.match(rest)
        if not m:
            return False
        rec.update(mode=int(m.group("mode")), done=int(m.group("done")), alpha=int(m.group("alpha"), 16),
                   px=int(m.group("px")), py=int(m.group("py")), pw=int(m.group("pw")), ph=int(m.group("ph")),
                   ch=int(m.group("ch")), th=int(m.group("th")), unreadable=bool(m.group("unr")))
    elif tag == "W3":
        m = W3_RE.match(rest)
        if m:
            rec.update(letterbox=m.group("vkind") == "letterbox", alpha=int(m.group("alpha"), 16), pred=m.group("pred"),
                       unreadable=bool(m.group("unr")))
        else:
            m = W3_NULL_RE.match(rest)
            if not m:
                return False
            rec.update(letterbox=False, alpha=None, pred="NULL/UNREADABLE", unreadable=True)
    elif tag == "W4":
        m = W4_RE.match(rest)
        if not m:
            return False
        rec.update(eax=int(m.group("eax")), alpha=int(m.group("alpha"), 16), unreadable=bool(m.group("unr")))
    else:
        m = W5_RE.match(rest)
        if not m:
            return False
        rec.update(arg=int(m.group("arg")), unreadable=bool(m.group("unr")))
    L.probe_lines += 1
    L.cur_bucket()[tag].append(rec)
    if rec.get("unreadable"):
        L.warn(line_no, "PROBE %s record has unreadable memory (informational)" % tag)
    return True


def probe_summary(b):
    """Per-bucket tallies + one-line diagnosis (V8b (c)). Returns a JSON-friendly dict."""
    out = {"index": b["index"], "settop_line": b["settop_line"]}
    w1, w2, w3, w4, w5 = b["W1"], b["W2"], b["W3"], b["W4"], b["W5"]
    hs = [r["ph"] for r in w1]
    out["W1"] = {"n": len(w1), "h_first": hs[:5], "h_final": hs[-1] if hs else None,
                 "done_seen": any(r["done"] == 1 for r in w1)}
    ys = [r["py"] for r in w2]
    out["W2"] = {"n": len(w2), "y_first": ys[:5], "y_final": ys[-1] if ys else None,
                 "done_seen": any(r["done"] == 1 for r in w2)}
    preds, alphas = {}, {}
    for r in w3:
        preds[r["pred"]] = preds.get(r["pred"], 0) + 1
        if r["alpha"] is not None:
            k = alpha_label(r["alpha"])
            alphas[k] = alphas.get(k, 0) + 1
    out["W3"] = {"n": len(w3), "predicate": preds, "vtbl_letterbox": sum(1 for r in w3 if r["letterbox"]),
                 "vtbl_other": sum(1 for r in w3 if not r["letterbox"]), "alpha": alphas}
    out["W4"] = {"n": len(w4), "eax1": sum(1 for r in w4 if r["eax"] == 1), "eax0": sum(1 for r in w4 if r["eax"] == 0)}
    args = [r["arg"] for r in w5]
    out["W5"] = {"n": len(w5), "args": args[:6], "start_arg1": bool(args) and args[0] == 1,
                 "end_arg0": bool(args) and args[-1] == 0}
    out["diagnosis"] = probe_diagnosis(b)
    return out


def probe_diagnosis(b):
    if b["index"] == 0:
        return "n/a (PROBE lines before the first SETTOP)"
    w1, w3, w4 = b["W1"], b["W3"], b["W4"]
    if not w1:
        return "Draw never called on the bar"
    lb = [r for r in w3 if r["letterbox"]]
    wd = [r for r in lb if r["pred"] == "WOULD-DRAW"]
    if w3 and not wd:
        pool = lb or w3
        counts = {}
        for r in pool:
            counts[r["pred"]] = counts.get(r["pred"], 0) + 1
        term = sorted(counts.items(), key=lambda kv: (-kv[1], kv[0]))[0][0]
        return "predicate fails: %s" % term + ("" if lb else " (no letterbox-vtable W3 record, all OTHER)")
    if wd and all(r["alpha"] == 0 for r in wd):
        return "alpha 0"
    eax1 = sum(1 for r in w4 if r["eax"] == 1)
    eax0 = sum(1 for r in w4 if r["eax"] == 0)
    if eax0 and not eax1:
        return "fill rejected (eax=0)"
    if wd and eax1:
        return "bars drawn (fill issued)"
    if not w3:
        return "inconclusive: W1 present but no W3 record"
    return "inconclusive: WOULD-DRAW but no W4 record (fill call not reached or rate-limited)"


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
            if lm.group("probe") is not None:
                cur.probe = int(lm.group("probe"))
            cur.start_bytes = lm.group("bytes").lower()
            if lm.group("unreadable"):
                cur.err(line_no, "constant UNREADABLE at load", body)
            elif cur.start_bytes != VANILLA_BYTES:
                cur.err(line_no, "start bytes not vanilla (%s)" % cur.start_bytes, body)
            if cur.enabled == 0:
                cur.warn(line_no, "enabled=0: the fix never writes")
            continue

        L = ensure(line_no)

        # PROBE lines are informational: handled before the generic error screen (their text contains words such as
        # UNREADABLE / NULL that the screen would flag).
        if body.startswith("PROBE "):
            if PROBE_CAP_RE.match(body):
                L.cap.append("probe-cap(%s)" % PROBE_CAP_RE.match(body).group("n"))
            elif not parse_probe(L, line_no, body):
                unparsed.append((line_no, raw))
            continue

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
                L.new_dialogue(line_no)
            else:
                L.setbottom += 1
            cur_apply = L.current_apply()
            if L.log == 0 and cur_apply is not None:
                check_set_self_consistent(L, line_no, what, g, body)
                continue
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
        if L.probe == 0 and L.probe_lines:
            L.warn(L.line_no, "%d PROBE line(s) although the load line says probe=0" % L.probe_lines)
        if L.probe == 1 and L.log == 1 and L.settop and not L.probe_lines:
            L.warn(L.line_no, "probe=1 log=1 with %d SETTOP line(s) but no PROBE line" % L.settop)
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
    out.extend(render_probe(launches))
    out.append(verdict)
    return "\n".join(out)


def _progress(vals, final):
    if not vals:
        return "-"
    txt = ",".join(str(v) for v in vals)
    return txt + (" ... %s" % final if final is not None and len(vals) >= 5 else "")


def render_probe(launches):
    out = []
    for L in launches:
        buckets = ([L.pre_bucket] if any(L.pre_bucket[t] for t in ("W1", "W2", "W3", "W4", "W5")) else []) + L.buckets
        buckets = [b for b in buckets if any(b[t] for t in ("W1", "W2", "W3", "W4", "W5"))]
        if not buckets:
            continue
        out.append("Probe (launch #%d, probe=%s, %d PROBE lines):" % (L.index, L.probe if L.probe is not None else "?", L.probe_lines))
        for b in buckets:
            ps = probe_summary(b)
            out.append("  %s:" % ("pre-dialogue" if b["index"] == 0 else "dialogue %d (SETTOP line %d)" % (b["index"], b["settop_line"])))
            w1, w2, w3, w4, w5 = ps["W1"], ps["W2"], ps["W3"], ps["W4"], ps["W5"]
            out.append("    W1 top-bar h: %d rec, h %s, final %s, done=1 %s" % (
                w1["n"], _progress(w1["h_first"], w1["h_final"]), w1["h_final"], "seen" if w1["done_seen"] else "NOT seen"))
            out.append("    W2 bottom y:  %d rec, y %s, final %s, done=1 %s" % (
                w2["n"], _progress(w2["y_first"], w2["y_final"]), w2["y_final"], "seen" if w2["done_seen"] else "NOT seen"))
            out.append("    W3 predicate: %d rec; %s; vtbl letterbox x%d OTHER x%d; alpha %s" % (
                w3["n"], ", ".join("%s x%d" % kv for kv in sorted(w3["predicate"].items())) or "-",
                w3["vtbl_letterbox"], w3["vtbl_other"],
                ", ".join("%s x%d" % kv for kv in sorted(w3["alpha"].items())) or "-"))
            out.append("    W4 fill:      %d rec; eax=1 x%d, eax=0 x%d" % (w4["n"], w4["eax1"], w4["eax0"]))
            out.append("    W5 state:     %d rec; args %s (start arg=1: %s, end arg=0: %s)" % (
                w5["n"], ",".join(str(a) for a in w5["args"]) or "-", "yes" if w5["start_arg1"] else "no",
                "yes" if w5["end_arg0"] else "no"))
            out.append("    DIAGNOSIS: %s" % ps["diagnosis"])
        out.append("")
    return out


def build_json(path, launches, unparsed, total_lines, ok, verdict, dialogues):
    return {
        "file": path, "lines": total_lines, "ok": ok, "verdict": verdict,
        "launches": len(launches), "dialogues": dialogues,
        "unparsed": [{"line": ln, "text": t} for ln, t in unparsed],
        "launch_details": [{
            "index": L.index, "line": L.line_no, "pid": L.pid, "mode": L.mode, "enabled": L.enabled, "log": L.log,
            "start_bytes": L.start_bytes, "probe": L.probe, "probe_lines": L.probe_lines,
            "probe_dialogues": [probe_summary(b) for b in L.buckets if any(b[t] for t in ("W1", "W2", "W3", "W4", "W5"))],
            "probe_pre_dialogue": (probe_summary(L.pre_bucket)
                                   if any(L.pre_bucket[t] for t in ("W1", "W2", "W3", "W4", "W5")) else None), "dialogues": L.settop, "setbottom": L.setbottom,
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
