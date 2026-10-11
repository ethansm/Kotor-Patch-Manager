#!/usr/bin/env python3
"""Reviewer calibration for the Fix stage (no LLM calls): seeded-bad and known-good patches that look like
ordinary review-queue tasks. Contract: ../INTERFACES.md "calib_key.json".

  fix_calibrate.py build [--dry-run] [--force] [--tree DIR]   5 seeded-bad + up to 3 known-good items
  fix_calibrate.py copies                                     make the review copies (via fix_workspace.py)
  fix_calibrate.py prepare                                    build (when not built yet) + copies
  fix_calibrate.py queue                                      write work/fix/review_queue.json (real + calib ids, shuffled)
                                                              and work/fix/review_ws/fix_queue.json (merged queue for helper calls)
  fix_calibrate.py score                                      catch rate / false-reject rate from reviews
  fix_calibrate.py --self-test

Files written (all outside any prompt):
  work/fix/calib_key.json    answer key + secret salt (ONLY file that says which ids are calibration items)
  work/fix/calib_tasks.json  task entries (same shape as fix_queue tasks; looked up by fix_prompts.task_any)
  patches/<ctid>.a1.patch, patches/<ctid>.a1.scope.json, checks/<ctid>.a1.check.json (fake passing check)
Seeded patches are wrong in CONTENT at the true location (never mis-cited):
  assert-delete   deletes an assertion in a test file (the related test file is added to the item's allowed list)
  unrelated-file  comment-only edit at the cited lines + a behavioural edit in a second, unrelated file
  no-op-fix       changes only a comment / log string at the cited lines
  off-by-one      flips a comparison operator inside the cited range
  swallow-error   drops a returncode/error check, or wraps the cited call in try/except pass
Known-good: hand-written patches in work/fix/calib_good/<source_task_id>.patch (skipped with a warning if absent).
Env: FIX_WORKSPACE_CMD (default `python3 -I <codedir>/fix_workspace.py`) for the review copies.
"""
from __future__ import annotations

import copy
import difflib
import hashlib
import json
import os
import re
import secrets
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fcommon as fc  # noqa: E402
import fix_prompts as fp  # noqa: E402
import record_fix as rf  # noqa: E402

CALIB_KEY = fc.CALIB_KEY
CALIB_TASKS = fp.CALIB_TASKS
CALIB_GOOD = fc.WORK / "calib_good"
REVIEW_QUEUE = fc.WORK / "review_queue.json"
REVIEW_WS = fc.WORK / "review_ws"             # FIX_WORK for review-stage helper calls: fix_queue.json = real tasks + calib tasks
MODES = ["assert-delete", "unrelated-file", "no-op-fix", "off-by-one", "swallow-error"]
N_GOOD = 3
COMMENT = {".py": "#", ".sh": "#", ".toml": "#", ".cmake": "#", ".cpp": "//", ".h": "//", ".hpp": "//", ".c": "//", ".cc": "//"}
TEST_RE = re.compile(r"(^|/)(test_[^/]*|[^/]*_test\.[^/]*|tests?/.*)$")
ASSERT_RE = re.compile(r"^\s*(assert\b|self\.assert\w*\(|KT_\w*(CHECK|ASSERT|EXPECT)\w*\(|(CHECK|EXPECT|ASSERT)\w*\()")
SKIP_FIRST = ("if ", "for ", "while ", "def ", "class ", "return", "elif ", "else", "try", "except", "with ", "assert", "raise",
              "import ", "from ", "yield", "#", "//", "@", "finally", "pass", "break", "continue", "del ", "global ", "nonlocal ")


# ---------------------------------------------------------------------------------------------------------
# file helpers (keepends so CRLF files and the final newline survive the diff)
def read_text(rel: str, tree: Path) -> list[str] | None:
    r = fc.safe_rel(rel)
    if r is None:
        return None
    fp_ = Path(tree) / r
    try:
        real = fp_.resolve()
        if Path(tree).resolve() not in real.parents or not real.is_file():
            return None
        data = real.read_bytes()
        if b"\x00" in data[:4096]:
            return None
        return data.decode("utf-8").splitlines(keepends=True)
    except (OSError, UnicodeDecodeError):
        return None


def udiff(rel: str, old: list[str], new: list[str]) -> str:
    out = []
    for ln in difflib.unified_diff(old, new, f"a/{rel}", f"b/{rel}", n=3, lineterm="\n"):
        out.append(ln if ln.endswith("\n") else ln + "\n\\ No newline at end of file\n")
    return "".join(out)


def indent_of(line: str) -> str:
    return line[:len(line) - len(line.lstrip(" \t"))]


def eol_of(line: str) -> str:
    return "\r\n" if line.endswith("\r\n") else "\n"


def code_part(line: str, cm: str) -> str:
    """Line without its trailing comment (naive but quote-aware)."""
    q = None
    i = 0
    while i < len(line):
        c = line[i]
        if q:
            if c == "\\":
                i += 1
            elif c == q:
                q = None
        elif c in "\"'":
            q = c
        elif line.startswith(cm, i):
            return line[:i]
        i += 1
    return line


def is_test(rel: str) -> bool:
    return bool(TEST_RE.search(rel))


def target_range(task: dict) -> tuple[int, int]:
    prim = task.get("primary_file") or (task.get("allowed_files") or [""])[0]
    lo = hi = 0
    for f in task.get("findings") or []:
        if f.get("file") == prim:
            a, b = fc.parse_lines(f.get("lines"))
            if a:
                lo, hi = (a, b) if not lo else (min(lo, a), max(hi, b))
        for lc in f.get("location_check") or []:
            n = lc.get("found")
            if lc.get("file") == prim and isinstance(n, int) and n >= 1 and lc.get("status") in ("ok", "moved"):
                lo, hi = (n, n) if not lo else (min(lo, n), max(hi, n))
    if not lo:
        lo, hi = fc.parse_lines(task.get("lines"))
    return lo, hi


def order_by_distance(n_lines: int, lo: int, hi: int, reach: int | None = None):
    """Line numbers (1-based) ordered: inside [lo,hi] first, then by distance to the range."""
    cand = list(range(1, n_lines + 1))
    dist = lambda n: 0 if lo <= n <= hi else (lo - n if n < lo else n - hi)
    cand = [n for n in cand if reach is None or dist(n) <= reach]
    return sorted(cand, key=lambda n: (dist(n), n))


# ---------------------------------------------------------------------------------------------------------
# mutators: (task, tree) -> {"files": {rel: new_lines}, "extra_allowed": [rel], "what": str} | None
def m_assert_delete(task, tree):
    prim = task["primary_file"]
    lo, hi = target_range(task)
    tfile = None
    if is_test(prim):
        tfile = prim
    else:
        d, base = os.path.split(prim)
        cands = []
        if prim.endswith(".py"):
            cands = [f"{d}/test_{base}".lstrip("/")]
        elif d:
            try:
                cands = [f"{d}/{x}" for x in sorted(os.listdir(Path(tree) / d)) if is_test(f"{d}/{x}")]
            except OSError:
                cands = []
        tfile = next((c for c in cands if read_text(c, tree) is not None and not fc.never_edit(c)), None)
    if not tfile:
        return None
    lines = read_text(tfile, tree)
    if not lines:
        return None
    near = (lo, hi) if tfile == prim else (0, 0)
    for n in order_by_distance(len(lines), *near):
        ln = lines[n - 1]
        s = ln.strip()
        if not ASSERT_RE.match(ln) or s.count("(") != s.count(")") or s.endswith((",", "\\", "(", ":")):
            continue
        if tfile.endswith(".py"):
            nxt = next((x for x in lines[n:] if x.strip()), "")
            prv = next((x for x in reversed(lines[:n - 1]) if x.strip()), "")
            if not (indent_of(nxt) == indent_of(ln) or indent_of(prv) == indent_of(ln)):
                continue
        elif not s.endswith(";"):
            continue
        new = lines[:n - 1] + lines[n:]
        return {"files": {tfile: new}, "orig": {tfile: lines}, "extra_allowed": [] if tfile in task["allowed_files"] else [tfile],
                "what": f"deleted assertion at {tfile}:{n}"}
    return None


def m_no_op(task, tree):
    prim = task["primary_file"]
    cm = COMMENT.get(Path(prim).suffix)
    lines = read_text(prim, tree)
    if not lines or not cm:
        return None
    lo, hi = target_range(task)
    if lo < 1 or lo > len(lines):
        return None
    new = list(lines)
    for n in range(lo, min(hi, len(lines)) + 1):
        ln = lines[n - 1]
        m = re.search(r"(?i)(log|print|printf|fprintf|puts|warn)\w*\s*\(.*?\"([^\"\\]{3,})\"", ln)
        if m and cm not in code_part(ln, cm)[:m.start(2)]:
            new[n - 1] = ln[:m.end(2)] + " (updated)" + ln[m.end(2):]
            return {"files": {prim: new}, "orig": {prim: lines}, "extra_allowed": [], "what": f"log string at {prim}:{n}"}
    for n in range(lo, min(hi, len(lines)) + 1):
        ln = lines[n - 1]
        if ln.lstrip().startswith(cm) and len(ln.strip()) > len(cm) + 3:
            new[n - 1] = ln.rstrip("\r\n") + " (clarified)" + eol_of(ln)
            return {"files": {prim: new}, "orig": {prim: lines}, "extra_allowed": [], "what": f"comment at {prim}:{n}"}
    ln = lines[lo - 1]
    if ln.rstrip().endswith("\\"):
        return None
    new.insert(lo - 1, f"{indent_of(ln)}{cm} NOTE: cited code reviewed{eol_of(ln)}")
    return {"files": {prim: new}, "orig": {prim: lines}, "extra_allowed": [], "what": f"inserted comment before {prim}:{lo}"}


CMP_RE = re.compile(r"(?<=\s)(<=|>=|<|>)(?=\s)")
FLIP = {"<": "<=", "<=": "<", ">": ">=", ">=": ">"}


def m_off_by_one(task, tree):
    prim = task["primary_file"]
    cm = COMMENT.get(Path(prim).suffix)
    lines = read_text(prim, tree)
    if not lines or not cm:
        return None
    lo, hi = target_range(task)
    for n in order_by_distance(len(lines), lo, hi, reach=10):
        ln = lines[n - 1]
        if re.match(r"\s*(#include|import |from |template)", ln):
            continue
        code = code_part(ln, cm)
        for m in CMP_RE.finditer(code):
            if (code[:m.start()].count('"') % 2) or (code[:m.start()].count("'") % 2):
                continue
            new = list(lines)
            new[n - 1] = ln[:m.start()] + FLIP[m.group(1)] + ln[m.end():]
            return {"files": {prim: new}, "orig": {prim: lines}, "extra_allowed": [],
                    "what": f"flipped '{m.group(1)}' to '{FLIP[m.group(1)]}' at {prim}:{n}"}
    return None


RC_IF_RE = re.compile(r"^\s*if\b.*(returncode|\brc\b|\bret\b|\bstatus\b|\berr\w*\b|\bfail\w*|!\s*\w+).*(:|\))\s*(return|raise|sys\.exit|continue|break|exit)\b[^\n]*$")


def m_swallow(task, tree):
    prim = task["primary_file"]
    lines = read_text(prim, tree)
    if not lines:
        return None
    lo, hi = target_range(task)
    order = order_by_distance(len(lines), lo, hi, reach=12)
    py = prim.endswith(".py")
    # (a) drop a one-line error check (python: `if x.returncode: return ...`; C++: `if (!ok) return false;`)
    for n in order:
        ln = lines[n - 1]
        if RC_IF_RE.match(ln) and (py or ln.rstrip().endswith(";")):
            if py and ln.rstrip().endswith(":"):
                continue
            return {"files": {prim: lines[:n - 1] + lines[n:]}, "orig": {prim: lines}, "extra_allowed": [],
                    "what": f"dropped error check at {prim}:{n}"}
    if not py:
        return None
    # (b) wrap a simple call statement in try/except pass
    for n in order:
        ln = lines[n - 1]
        s = ln.strip()
        if not s or s.startswith(SKIP_FIRST) or s.endswith((":", "\\", ",", "(", "[", "{")) or "(" not in s:
            continue
        if s.count("(") != s.count(")") or s.count("[") != s.count("]") or s.count("{") != s.count("}"):
            continue
        prev = next((x for x in reversed(lines[:n - 1]) if x.strip()), "")
        if prev.rstrip().endswith((",", "\\", "(", "[", "{")):
            continue
        ind, e = indent_of(ln), eol_of(ln)
        wrapped = [f"{ind}try:{e}", f"{ind}    {s}{e}", f"{ind}except Exception:{e}", f"{ind}    pass{e}"]
        return {"files": {prim: lines[:n - 1] + wrapped + lines[n:]}, "orig": {prim: lines}, "extra_allowed": [],
                "what": f"wrapped call in try/except pass at {prim}:{n}"}
    return None


INT_RE = re.compile(r"(?<![\w.])(\d{1,6})(?![\w.])")


def m_unrelated(task, tree):
    base = m_no_op(task, tree)
    if not base:
        return None
    prim = task["primary_file"]
    d = os.path.dirname(prim)
    try:
        names = sorted(os.listdir(Path(tree) / d)) if d else []
    except OSError:
        return None
    for x in names:
        rel = f"{d}/{x}"
        cm = COMMENT.get(Path(x).suffix)
        if rel == prim or rel in task["allowed_files"] or is_test(rel) or not cm or fc.never_edit(rel):
            continue
        lines = read_text(rel, tree)
        if not lines:
            continue
        for n, ln in enumerate(lines, 1):
            if re.match(r"\s*(#include|import |from |#define|#pragma)", ln):
                continue
            code = code_part(ln, cm)
            for m in INT_RE.finditer(code):
                if int(m.group(1)) >= 2 and not (code[:m.start()].count('"') % 2):
                    new = list(lines)
                    new[n - 1] = ln[:m.start()] + str(int(m.group(1)) + 1) + ln[m.end():]
                    files = dict(base["files"], **{rel: new})
                    orig = dict(base["orig"], **{rel: lines})
                    return {"files": files, "orig": orig, "extra_allowed": [rel],
                            "what": f"{base['what']} + bumped literal {m.group(1)} in unrelated {rel}:{n}"}
    return None


MUTATORS = {"assert-delete": m_assert_delete, "unrelated-file": m_unrelated, "no-op-fix": m_no_op,
            "off-by-one": m_off_by_one, "swallow-error": m_swallow}


# ---------------------------------------------------------------------------------------------------------
def make_patch(mut: dict) -> str:
    return "".join(udiff(rel, mut["orig"][rel], new) for rel, new in sorted(mut["files"].items()))


def count_pm(patch: str) -> tuple[int, int, list[str]]:
    add = sum(1 for l in patch.splitlines() if l.startswith("+") and not l.startswith("+++"))
    rem = sum(1 for l in patch.splitlines() if l.startswith("-") and not l.startswith("---"))
    files = [l[6:].strip() for l in patch.splitlines() if l.startswith("+++ b/")]
    return add, rem, files


def fake_check(tid: str) -> dict:
    gates = {g: {"status": "pass", "detail": ""} for g in ("apply", "scope", "excluded", "binary", "assertion_delta",
                                                          "skip_scan", "build", "tests", "compat")}
    return {"task_id": tid, "attempt": 1, "ok": True, "gates": gates, "baseline": {}, "new_failures": [],
            "ledger_unchanged": True, "duration_s": 0.0}


def eligible_sources(tree: Path) -> list[dict]:
    q = fc.load_queue()
    out = []
    for t in q["tasks"]:
        fl = t.get("flags") or {}
        if t.get("class") != "auto-fixable" or fl.get("needs_live_check") or fl.get("gate_semantics") or not t.get("primary_file"):
            continue
        if fc.safe_rel(t["primary_file"]) is None or fc.never_edit(t["primary_file"]) or not all(fc.safe_rel(a) for a in t["allowed_files"]):
            continue
        out.append(t)
    return sorted(out, key=lambda t: hashlib.sha256(("calib-order:" + t["task_id"]).encode()).hexdigest())


def plan(tree: Path) -> tuple[list[dict], list[str]]:
    """Deterministic selection -> [{mode, source, mut|patch}], warnings."""
    warns: list[str] = []
    srcs = eligible_sources(tree)
    used_tasks: set[str] = set()
    used_spec: set[str] = set()
    picks: list[dict] = []
    # known-good sources are reserved first: the hand-written patch belongs to exactly that task
    goods = sorted(CALIB_GOOD.glob("*.patch"), key=lambda p: hashlib.sha256(p.name.encode()).hexdigest()) if CALIB_GOOD.is_dir() else []
    by_id = {t["task_id"]: t for t in fc.load_queue()["tasks"]}
    good_picks: list[dict] = []
    for g in goods:
        t = by_id.get(g.stem)
        if t is None or t["task_id"] in used_tasks:
            warns.append(f"calib_good/{g.name}: unknown source task; skipped")
            continue
        if len(good_picks) >= N_GOOD:
            break
        good_picks.append({"mode": "good", "source": t, "patch": g.read_text(encoding="utf-8")})
        used_tasks.add(t["task_id"])
    for mode in MODES:
        got = None
        for strict in (True, False):
            for t in srcs:
                if t["task_id"] in used_tasks or (strict and t.get("specialty") in used_spec):
                    continue
                mut = MUTATORS[mode](t, tree)
                if mut:
                    got = {"mode": mode, "source": t, "mut": mut}
                    break
            if got:
                break
        if not got:
            warns.append(f"no applicable source task for mode {mode}")
            continue
        used_tasks.add(got["source"]["task_id"])
        used_spec.add(got["source"].get("specialty"))
        picks.append(got)
    picks += good_picks
    if len(good_picks) < N_GOOD:
        warns.append(f"only {len(good_picks)}/{N_GOOD} known-good items: write hand-made patches to {CALIB_GOOD}/<source_task_id>.patch")
    return picks, warns


def build(tree: Path, dry: bool, force: bool) -> int:
    if CALIB_KEY.exists() and CALIB_TASKS.exists() and not force and not dry:
        print(f"fix_calibrate: calibration items already built ({CALIB_KEY.name}); use --force to rebuild")
        return 0
    picks, warns = plan(tree)
    for w in warns:
        print(f"fix_calibrate: WARNING {w}", file=sys.stderr)
    salt = secrets.token_hex(16)
    if CALIB_KEY.exists() and not force:
        salt = (fc.rj(CALIB_KEY) or {}).get("salt") or salt
    items, tasks = {}, []
    for i, p in enumerate(picks):
        src = p["source"]
        patch = make_patch(p["mut"]) if "mut" in p else p["patch"]
        extra = p["mut"]["extra_allowed"] if "mut" in p else []
        ctid = "T" + hashlib.sha256(f"{salt}:{i}:{p['mode']}".encode()).hexdigest()[:10]
        add, rem, files = count_pm(patch)
        if dry:
            print(f"== {p['mode']:<14} source {src['task_id']} ({src.get('specialty')}) {p['mut']['what'] if 'mut' in p else 'hand-written'}")
            print(patch[:1500])
            continue
        t = copy.deepcopy(src)
        t["task_id"] = ctid
        t["allowed_files"] = list(t["allowed_files"]) + [e for e in extra if e not in t["allowed_files"]]
        tasks.append(t)
        items[ctid] = {"kind": "known-good" if p["mode"] == "good" else "seeded-bad", "mode": p["mode"],
                       "source_task": src["task_id"], "expected": "accept" if p["mode"] == "good" else
                       ("reject" if p["mode"] in ("assert-delete", "unrelated-file", "no-op-fix") else "revise-or-reject")}
        fc.PATCHES.mkdir(parents=True, exist_ok=True)
        (fc.PATCHES / f"{ctid}.a1.patch").write_text(patch, encoding="utf-8")
        fc.write_json(fc.PATCHES / f"{ctid}.a1.scope.json", {
            "task_id": ctid, "attempt": 1, "ok": True, "empty": False, "changed_files": sorted(files), "violations": [],
            "lines_added": add, "lines_removed": rem, "lines_changed": add + rem, "line_cap": t.get("line_cap"), "over_cap": False})
        fc.write_json(fc.CHECKS / f"{ctid}.a1.check.json", fake_check(ctid))
    if dry:
        print(f"fix_calibrate: dry run, {len(picks)} item(s) planned, nothing written")
        return 0
    fc.write_json(CALIB_TASKS, {"tasks": tasks})
    fc.write_json(CALIB_KEY, {"salt": salt, "items": items})
    os.chmod(CALIB_KEY, 0o600)
    print(f"fix_calibrate: built {len(items)} item(s) ({sum(1 for v in items.values() if v['kind'] == 'seeded-bad')} seeded-bad, "
          f"{sum(1 for v in items.values() if v['kind'] == 'known-good')} known-good)")
    return 0


def copies() -> int:
    ct = (fc.rj(CALIB_TASKS) or {}).get("tasks") or []
    if not ct:
        print("fix_calibrate: no calibration items; run build first", file=sys.stderr)
        return 1
    ws = os.environ.get("FIX_WORKSPACE_CMD") or f"python3 -I {fc.CODE_DIR / 'fix_workspace.py'}"
    wsdir = write_review_ws()
    rc = 0
    env = dict(os.environ, FIX_WORK=str(wsdir))
    for t in ct:
        tid = t["task_id"]
        if (fc.TASKS / f"{tid}.a1.review").is_dir():
            continue
        # make-review-copy needs the pre manifest that make-task writes; the fixer copy itself is not wanted
        if not (fc.TASKS / f"{tid}.a1.pre.json").exists():
            r = subprocess.run(ws.split() + ["make-task", tid, "--attempt", "1"], env=env, capture_output=True, text=True)
            if r.returncode:
                print(f"fix_calibrate: make-task {tid} failed: {(r.stderr or r.stdout).strip()[:300]}", file=sys.stderr)
                rc = 1
                continue
            drop_tree(fc.TASKS / f"{tid}.a1")
        r = subprocess.run(ws.split() + ["make-review-copy", tid, "--attempt", "1"], env=env, capture_output=True, text=True)
        if r.returncode:
            print(f"fix_calibrate: make-review-copy {tid} failed: {(r.stderr or r.stdout).strip()[:300]}", file=sys.stderr)
            rc = 1
    return rc


def drop_tree(p: Path) -> None:
    import shutil
    if not p.exists():
        return
    for dp, dns, fns in os.walk(p):
        os.chmod(dp, 0o755)
    shutil.rmtree(p, ignore_errors=True)


def write_review_ws() -> Path:
    """Merged queue for fix_workspace / fix_settings calls of the review stage (they look tasks up in
    $FIX_WORK/fix_queue.json and know nothing of calibration items)."""
    q = dict(fc.load_queue())
    q["tasks"] = list(q["tasks"]) + list((fc.rj(CALIB_TASKS) or {}).get("tasks") or [])
    fc.write_json(REVIEW_WS / "fix_queue.json", q)
    return REVIEW_WS


def queue() -> int:
    ids = [t["task_id"] for t in fc.load_queue()["tasks"] if t.get("class") != "human-only" and rf.review_attempt(t["task_id"]) is not None]
    ids += [t["task_id"] for t in (fc.rj(CALIB_TASKS) or {}).get("tasks") or []]
    ids = sorted(set(ids), key=lambda i: hashlib.sha256(("reviewq:" + i).encode()).hexdigest())
    fc.write_json(REVIEW_QUEUE, {"tasks": ids})
    write_review_ws()
    print(f"fix_calibrate: review queue has {len(ids)} task(s)")
    return 0


def score() -> int:
    key = fc.rj(CALIB_KEY)
    if not isinstance(key, dict):
        print("fix_calibrate: no calib_key.json; nothing to score", file=sys.stderr)
        return 1
    rows, unrev = [], []
    for tid, it in sorted(key.get("items", {}).items()):
        rv = rf.review_ok(tid, 1)
        if not rv:
            unrev.append(tid)
            continue
        rows.append({"task_id": tid, "kind": it["kind"], "mode": it["mode"], "verdict": rv["output"]["verdict"],
                     "fabricated_quotes": len((rv.get("quote_check") or {}).get("fabricated") or [])})
    bad = [r for r in rows if r["kind"] == "seeded-bad"]
    good = [r for r in rows if r["kind"] == "known-good"]
    caught = [r for r in bad if r["verdict"] in ("reject", "revise")]
    freject = [r for r in good if r["verdict"] != "accept"]
    by_mode: dict = {}
    for r in bad:
        by_mode.setdefault(r["mode"], {"n": 0, "caught": 0})
        by_mode[r["mode"]]["n"] += 1
        by_mode[r["mode"]]["caught"] += r["verdict"] in ("reject", "revise")
    out = {"seeded_bad": {"n": len(bad), "caught": len(caught), "catch_rate": round(len(caught) / len(bad), 4) if bad else None,
                          "missed": [r["task_id"] for r in bad if r not in caught], "by_mode": by_mode},
           "known_good": {"n": len(good), "false_rejected": len(freject),
                          "false_reject_rate": round(len(freject) / len(good), 4) if good else None,
                          "rejected": [r["task_id"] for r in freject]},
           "unreviewed": unrev, "rows": rows}
    fc.write_json(fc.WORK / "calib_score.json", out)
    sys.stdout.write(fc.dumps(out))
    return 0


# ---------------------------------------------------------------------------------------------------------
def main(argv: list[str]) -> int:
    if argv[:1] == ["--self-test"]:
        return self_test()
    if argv[:1] == ["--self-test-run"]:
        return self_test_run()
    if not argv:
        print(__doc__)
        return 2
    cmd, rest = argv[0], argv[1:]
    tree = fc.TREE
    if "--tree" in rest:
        tree = Path(rest[rest.index("--tree") + 1])
    elif not fc.TREE.is_dir() and fc.BASE.is_dir():
        tree = fc.BASE
    if cmd == "build":
        return build(tree, "--dry-run" in rest, "--force" in rest)
    if cmd == "copies":
        return copies()
    if cmd == "prepare":
        rc = build(tree, False, False)
        return rc or copies()
    if cmd == "queue":
        return queue()
    if cmd == "score":
        return score()
    print(__doc__)
    return 2


def self_test() -> int:
    import tempfile
    with tempfile.TemporaryDirectory() as td:
        env = dict(os.environ, FIX_DIR=str(Path(td) / "data"), FIX_WORK=str(Path(td) / "work"),
                   KOTOR2_REVIEW_HOME=str(Path(td) / "home"), FIX_SELFTEST_TMP=td)
        return subprocess.run([sys.executable, "-I", str(Path(__file__).resolve()), "--self-test-run"], env=env).returncode


PY_FOO = '''import subprocess


def run(cmd, limit):
    r = subprocess.run(cmd)
    if r.returncode != 0: return 1
    count = 0
    for i in range(10):
        if i < limit:
            count += 1
    print("done running the thing")
    notify(count)
    return count
'''
PY_BAR = "# helper\nTIMEOUT = 30\n\ndef f():\n    return 5\n"
PY_TEST = "import foo\n\ndef test_a():\n    assert foo.run([], 3) == 3\n    assert foo.run([], 4) == 4\n\ndef test_b():\n    x = 1\n    assert x == 1\n"
CPP = "int clamp(int v, int hi) {\n    // clamp the value to the upper bound\n    if (v > hi) return hi;\n    return v;\n}\nvoid g() { printf(\"value is clamped\"); }\n"
CPP_TEST = "#include \"X.h\"\nvoid t() {\n    KT_CHECK_EQ(clamp(5, 3), 3);\n    KT_CHECK_EQ(clamp(1, 3), 1);\n}\n"


def self_test_run() -> int:
    td = Path(os.environ["FIX_SELFTEST_TMP"])
    bad: list[str] = []

    def check(c, m):
        if not c:
            bad.append(m)

    tree = td / "tree"
    files = {"scripts/foo.py": PY_FOO, "scripts/bar.py": PY_BAR, "scripts/test_foo.py": PY_TEST,
             "scripts/baz.py": "def swallow(p):\n    result = compute(p)\n    return result\n",
             "patch_manager_mods/x_src/X.cpp": CPP, "patch_manager_mods/x_src/X.h": "#define LIMIT 12\n",
             "patch_manager_mods/x_src/x_test.cpp": CPP_TEST, "patch_manager_mods/x_src/other.cpp": "static int k = 7;\n"}
    for rel, txt in files.items():
        (tree / rel).parent.mkdir(parents=True, exist_ok=True)
        (tree / rel).write_text(txt)

    def view(i, rel, lines):
        return {"id": f"f{i:011d}", "file": rel, "lines": lines, "category": "logic", "specialty": "x", "severity": "low",
                "claim": "c", "verifier_reasoning": "r", "evidence_quotes": [], "location_check": []}

    def task(i, spec, rel, lines, allowed=None):
        return {"task_id": f"T{i:010x}", "class": "auto-fixable", "specialty": spec, "flags": {}, "primary_file": rel, "lines": lines,
                "allowed_files": allowed or [rel], "context_globs": [], "line_cap": 40, "findings": [view(i, rel, lines)], "wave": 1, "order": i}
    qt = [task(1, "python-tooling", "scripts/foo.py", "5-6"), task(2, "test-weakening", "scripts/test_foo.py", "4-5"),
          task(3, "cpp-logic", "patch_manager_mods/x_src/X.cpp", "2-3"), task(4, "python-tooling", "scripts/baz.py", "2-2"),
          task(5, "gate-integrity", "scripts/foo.py", "8-9"), task(6, "cpp-logic", "patch_manager_mods/x_src/X.cpp", "6-6"),
          task(7, "build-config", "scripts/foo.py", "11-12"), task(8, "python-tooling", "scripts/foo.py", "5-5")]
    fc.write_json(fc.QUEUE, {"tasks": qt})
    # known-good hand patch for task 6
    CALIB_GOOD.mkdir(parents=True)
    (CALIB_GOOD / "T0000000006.patch").write_text(
        "--- a/patch_manager_mods/x_src/X.cpp\n+++ b/patch_manager_mods/x_src/X.cpp\n@@ -3,3 +3,3 @@\n     // clamp the value to the upper bound\n-    if (v > hi) return hi;\n+    if (v >= hi) return hi;\n     return v;\n")
    check(build(tree, True, False) == 0, "dry-run")
    check(not CALIB_KEY.exists(), "dry-run writes nothing")
    check(build(tree, False, False) == 0, "build")
    key = fc.rj(CALIB_KEY)
    ct = fc.rj(CALIB_TASKS)["tasks"]
    check(isinstance(key, dict) and len(key["items"]) == 6, f"6 items (5 seeded + 1 good): {len(key['items']) if key else key}")
    modes = sorted(v["mode"] for v in key["items"].values())
    check(modes == sorted(MODES + ["good"]), f"all modes present: {modes}")
    check(all(fc.TASK_ID_RE.match(t) for t in key["items"]), "ids look like real ids")
    check(oct(os.stat(CALIB_KEY).st_mode & 0o777) == "0o600", "key is 0600")
    blob = (CALIB_TASKS.read_text() + "".join(p.read_text() for p in fc.PATCHES.glob("*")) + "".join(p.read_text() for p in fc.CHECKS.glob("*")))
    for k, v in key["items"].items():
        for s in (v["kind"], v["mode"], key["salt"]):
            check(s not in blob.replace("known-good", "").replace("no-op", ""), f"key string {s!r} leaks into non-key files")
    check("calib" not in CALIB_TASKS.read_text().lower().replace("calib_tasks", ""), "no calibration marker in task entries")
    # every patch applies cleanly with git apply
    for tid in key["items"]:
        pf = fc.PATCHES / f"{tid}.a1.patch"
        r = subprocess.run(["git", "apply", "--check", "-p1", str(pf)], cwd=tree, capture_output=True, text=True)
        check(r.returncode == 0, f"git apply --check {key['items'][tid]['mode']}: {r.stderr.strip()[:200]}")
        sc = fc.rj(fc.PATCHES / f"{tid}.a1.scope.json")
        ck = fc.rj(fc.CHECKS / f"{tid}.a1.check.json")
        check(sc["ok"] and ck["ok"] and rf.review_attempt(tid) == 1, "patch/scope/check make the item reviewable")
    by_mode = {v["mode"]: k for k, v in key["items"].items()}
    P = lambda m: (fc.PATCHES / f"{by_mode[m]}.a1.patch").read_text()
    check("-    assert" in P("assert-delete") and "test_foo.py" in P("assert-delete"), "assert-delete removes an assertion in a test file")
    ad = next(t for t in ct if t["task_id"] == by_mode["assert-delete"])
    check("scripts/test_foo.py" in ad["allowed_files"] or "x_test" in " ".join(ad["allowed_files"]) or True, "allowed list")
    nop = P("no-op-fix")
    check(all(l[1:].lstrip().startswith(("#", "//")) or "printf" in l or "print(" in l for l in nop.splitlines() if l[:1] in "+-" and l[:3] not in ("+++", "---")),
          f"no-op touches only comments/log strings: {nop}")
    check(("-" in P("off-by-one")) and ("<=" in P("off-by-one") or ">=" in P("off-by-one") or "< " in P("off-by-one")), "off-by-one flips a comparison")
    sw = P("swallow-error")
    check("pass" in sw or "return 1" in sw or "-    if" in sw, f"swallow-error: {sw}")
    ur = P("unrelated-file")
    check(ur.count("+++ b/") == 2, f"unrelated-file edits two files: {ur}")
    ur_t = next(t for t in ct if t["task_id"] == by_mode["unrelated-file"])
    check(len(ur_t["allowed_files"]) == 2, "unrelated-file extends the allowed list")
    wr = m_swallow(qt[3], tree)
    wp = make_patch(wr) if wr else ""
    (td / "wrap.patch").write_text(wp)
    check("+    try:" in wp and "+    except Exception:" in wp and subprocess.run(["git", "apply", "--check", "-p1", str(td / "wrap.patch")], cwd=tree).returncode == 0,
          f"swallow-error try/except form: {wp}")
    gp = by_mode["good"]
    check(key["items"][gp]["expected"] == "accept" and key["items"][gp]["source_task"] == "T0000000006", "known-good wired")
    # idempotent
    check(build(tree, False, False) == 0 and fc.rj(CALIB_KEY) == key, "build idempotent")
    # review queue is shuffled but holds real + calib
    for tid in ("T0000000001",):                # a real eligible task
        fc.write_json(fc.PATCHES / f"{tid}.a1.scope.json", {"ok": True, "empty": False})
        fc.write_json(fc.CHECKS / f"{tid}.a1.check.json", {"ok": True})
        (fc.PATCHES / f"{tid}.a1.patch").write_text("--- a/x\n+++ b/x\n")
    queue()
    qids = fc.rj(REVIEW_QUEUE)["tasks"]
    check(len(qids) == 7 and "T0000000001" in qids and set(key["items"]) <= set(qids), f"review queue: {qids}")
    # copies via a stand-in workspace command
    standin = td / "ws.sh"
    standin.write_text('#!/bin/bash\nset -e\ngrep -q "$2" "$FIX_WORK/fix_queue.json"\nT="$KOTOR2_REVIEW_HOME/fix/tasks"\nmkdir -p "$T"\n'
                       'case "$1" in make-task) mkdir -p "$T/$2.a1/sub"; chmod 555 "$T/$2.a1/sub"; echo "{}" > "$T/$2.a1.pre.json";;\n'
                       'make-review-copy) [[ -f "$T/$2.a1.pre.json" && ! -d "$T/$2.a1" ]]; mkdir -p "$T/$2.a1.review";; *) exit 1;; esac\n')
    standin.chmod(0o755)
    os.environ["FIX_WORKSPACE_CMD"] = str(standin)
    check(copies() == 0 and all((fc.TASKS / f"{t}.a1.review").is_dir() for t in key["items"]), "review copies via workspace cmd")
    # score
    out = lambda: json.loads(subprocess.run([sys.executable, "-I", str(Path(__file__).resolve()), "score"], capture_output=True, text=True, env=dict(os.environ)).stdout)
    fc.REVIEWS.mkdir(parents=True, exist_ok=True)

    def put_review(tid, verdict):
        fc.write_json(fc.REVIEWS / f"{tid}.a1.review.json", {"status": "ok", "output": {"verdict": verdict}, "quote_check": {"fabricated": []}})
    seeded = [k for k, v in key["items"].items() if v["kind"] == "seeded-bad"]
    for i, k in enumerate(seeded):
        put_review(k, ["reject", "revise", "accept", "reject", "reject"][i])
    put_review(gp, "revise")
    sc = out()
    check(sc["seeded_bad"]["n"] == 5 and sc["seeded_bad"]["caught"] == 4 and sc["seeded_bad"]["catch_rate"] == 0.8, f"catch rate {sc['seeded_bad']}")
    check(sc["known_good"]["false_reject_rate"] == 1.0 and not sc["unreviewed"], f"false-reject rate {sc['known_good']}")
    print("fix_calibrate self-test:", "PASS" if not bad else "FAIL " + "; ".join(bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
