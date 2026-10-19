#!/usr/bin/env python3
"""F1b deterministic gates for fixer patches and integration batches (no LLM). Contract: ../INTERFACES.md.

Usage:
  fix_checks.py check <tid> --attempt N [--slot S]   -> $FIX_DIR/checks/<tid>.aN.check.json   (exit 0 ok, 1 gate failed)
  fix_checks.py stack <batch>                        dry --3way apply of the batch series -> integ/<batch>.stack.json
  fix_checks.py verify-series <batch>                fresh slot + final series from integ/<batch>.json -> integ/<batch>.verify.json
  fix_checks.py --self-test
Exit 3 (loud message) when the PRODUCTION ledger changed other than benign appends (CONTRACT_CHECK / other-plan SESSION_HANDOFF lines) during a command.

Gates (in order; a failed apply or static gate skips what needs a patched tree):
  scope, excluded, binary (patch text only) | apply (slot, sandboxed fixgate) | assertion_delta, skip_scan |
  build | tests (only NEW failures vs a cached clean-base baseline fail) | compat (when _shared_src changed)
All repo code runs through $FIX_HOME/bin/fixgate.sh (bwrap gate sandbox; FIX_SANDBOX=none only in self-tests).
Env: FIX_SLOTS (default 4), FIX_SLOT_WAIT seconds (default 900), FIX_GATE_TIMEOUT seconds (default 3600),
FIX_LEDGER_GUARD (self-test only: file whose hash replaces the production ledger in the tripwire).
"""
from __future__ import annotations

import argparse
import fcntl
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fcommon as fc  # noqa: E402
import fix_workspace as fw  # noqa: E402
import assertion_delta as ad  # type: ignore  # noqa: E402

GATES = ["apply", "scope", "excluded", "binary", "assertion_delta", "skip_scan", "build", "tests", "compat"]
LEDGER = Path(os.environ.get("FIX_LEDGER_GUARD") or fc.PRODUCTION_LEDGER)
MAX_BUILDS = 6
MAX_FILTERS = 6
SKIP_RE = re.compile(r"unittest\.skip|pytest\.skip|pytest\.mark\.skip|skipTest|skip_reason|xfail|expectedFailure|"
                     r"^\s*#\s*if\s+0\b|^\s*#\s*if\s+false\b")
SECRET_RE = re.compile(r"-----BEGIN [A-Z ]*PRIVATE KEY-----|\bsk-[A-Za-z0-9_-]{20,}|\bghp_[A-Za-z0-9]{30,}|\bAKIA[0-9A-Z]{16}\b")
HUNK_RE = re.compile(r"^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@")
STRUCT_RE = re.compile(r"^(?:new file mode|deleted file mode|old mode|new mode|rename |copy |similarity )")


class Tripwire(Exception):
    pass


LEDGER_BENIGN_EVENTS = {"CONTRACT_CHECK", "SESSION_HANDOFF"}
LEDGER_SUSPECT_SUBSTR = ("fix_pipeline", "kotor2_review", ".claude/review")


def ledger_hash() -> str:
    """Baseline token: "<size>:<sha256>" of the ledger bytes, or "absent"."""
    try:
        b = LEDGER.read_bytes()
    except OSError:
        return "absent"
    return f"{len(b)}:{hashlib.sha256(b).hexdigest()}"


def ledger_same(token: str, any_event: bool = False) -> bool:
    """True when the ledger still matches the baseline token, allowing benign appended lines after it.
    any_event: also allow appended lines of any event type (other sessions installing or planning while the self-test runs); lines that
    mention the fix pipeline are still refused."""
    if token == "absent":
        return not LEDGER.exists()
    try:
        size_s, sha = token.split(":", 1)
        size = int(size_s)
    except ValueError:
        return False
    try:
        b = LEDGER.read_bytes()
    except OSError:
        return False
    if len(b) < size or hashlib.sha256(b[:size]).hexdigest() != sha:
        return False
    try:
        appended = b[size:].decode("utf-8")
    except UnicodeDecodeError:
        return False
    for line in appended.splitlines():
        if not line.strip():
            continue
        try:
            obj = json.loads(line)
        except ValueError:
            return False
        if not isinstance(obj, dict) or (not any_event and obj.get("event") not in LEDGER_BENIGN_EVENTS):
            return False
        low = line.lower()
        if any(s in low for s in LEDGER_SUSPECT_SUBSTR):
            return False
    return True


def tail(s: str, n: int = 400) -> str:
    s = (s or "").strip()
    return s[-n:]


BUILD_TAIL = 6000
_DIAG = re.compile(r"error|cannot|undefined reference|multiple definition|\bFAIL\b", re.I)


def build_diags(out: str) -> set[str]:
    """Normalized diagnostic lines of a build log (line/column numbers and timings dropped)."""
    ds = set()
    for ln in (out or "").splitlines():
        if _DIAG.search(ln) and not ln.lstrip().startswith(("build_check:", "INFO time")):
            ds.add(re.sub(r":\d+(:\d+)?", ":N", re.sub(r"\d+(\.\d+)?s\b", "Ts", ln.strip())))
    return ds


def new_build_errors(patched_out: str, base_out: str) -> list[str]:
    """Diagnostics of a still-failing patched build that the failing clean-base build did not have."""
    return sorted(build_diags(patched_out) - build_diags(base_out))


# ------------------------------------------------------------------ patch parsing
def parse_patch(text: str) -> dict:
    """-> {"files": {path: [hunk]}, "paths": [header paths], "struct": [odd lines], "bad_headers": [lines]};
    hunk = {"lines": [(tag, text, new_lineno|None)]}. Hunk bodies are consumed by their line counts."""
    files, paths, struct, bad = {}, [], [], []
    cur, hunk, old_left, new_left, new_ln = None, None, 0, 0, 0
    for ln in text.split("\n"):
        if hunk is not None and (old_left > 0 or new_left > 0):
            tag = ln[:1]
            if tag == " " or ln == "":
                old_left -= 1
                new_left -= 1
                new_ln += 1
            elif tag == "-":
                old_left -= 1
                hunk["lines"].append(("-", ln[1:], None))
            elif tag == "+":
                new_left -= 1
                hunk["lines"].append(("+", ln[1:], new_ln))
                new_ln += 1
            continue
        if ln.startswith("diff --git "):
            m = re.match(r"^diff --git a/(\S+) b/(\S+)$", ln)
            if not m:
                bad.append(ln[:120])
                cur = None
                continue
            if m.group(1) != m.group(2):
                struct.append(ln[:120])
            cur = m.group(2)
            files.setdefault(cur, [])
            paths += [m.group(1), m.group(2)]
            hunk = None
            continue
        m = HUNK_RE.match(ln)
        if m and cur is not None:
            old_left = int(m.group(2) if m.group(2) is not None else 1)
            new_left = int(m.group(4) if m.group(4) is not None else 1)
            new_ln = int(m.group(3))
            hunk = {"lines": []}
            files[cur].append(hunk)
            continue
        m = re.match(r"^(?:--- a/|\+\+\+ b/)(\S+)", ln)
        if m:
            paths.append(m.group(1))
        elif ln.startswith(("--- ", "+++ ")) and not ln.startswith(("--- a/", "+++ b/", "--- /dev/null", "+++ /dev/null")):
            bad.append(ln[:120])
        elif STRUCT_RE.match(ln) or ln.startswith(("--- /dev/null", "+++ /dev/null")):
            struct.append(ln[:120])
    return {"files": files, "paths": sorted(set(paths)), "struct": struct, "bad_headers": bad}


# ------------------------------------------------------------------ static gates
def gate_scope(task, scope, patch_text, pp) -> dict:
    if not isinstance(scope, dict):
        return {"status": "fail", "detail": "scope.json missing (run fix_workspace.py diff)"}
    probs = []
    if not scope.get("ok"):
        probs.append(f"scope.json not ok: violations={scope.get('violations')} over_cap={scope.get('over_cap')}")
    if scope.get("empty") or not patch_text.strip():
        probs.append("empty patch")
    allowed = set(task["allowed_files"])
    extra = sorted(set(pp["paths"]) - allowed)
    if extra:
        probs.append(f"patch touches non-allowed paths: {extra[:5]}")
    if pp["struct"]:
        probs.append(f"patch creates/deletes/renames/changes mode: {pp['struct'][:3]}")
    if pp["bad_headers"]:
        probs.append(f"unparseable headers: {pp['bad_headers'][:3]}")
    never = [p for p in pp["paths"] if fc.never_edit(p)]
    if never:
        probs.append(f"never-edit paths: {never[:3]}")
    return {"status": "fail" if probs else "pass", "detail": "; ".join(probs) or
            f"{scope.get('lines_changed')} line(s) changed, cap {scope.get('line_cap')}"}


def gate_excluded(patch_text, pp) -> dict:
    probs = [p for p in pp["paths"] if fc.safe_rel(p) is None or fc.is_excluded(p)]
    added = [ln[1:] for ln in patch_text.split("\n") if ln.startswith("+") and not ln.startswith("+++ ")]
    if any(SECRET_RE.search(x) for x in added):
        probs.append("added line looks like a credential literal")
    return {"status": "fail" if probs else "pass", "detail": "; ".join(map(str, probs[:5])) or "no excluded paths"}


def gate_binary(patch_bytes: bytes, pp) -> dict:
    probs = []
    if b"\0" in patch_bytes:
        probs.append("NUL byte in patch")
    if re.search(rb"^(GIT binary patch|Binary files )", patch_bytes, re.M):
        probs.append("binary hunk")
    probs += [f"binary extension: {p}" for p in pp["paths"] if Path(p).suffix.lower() in fw.prep.BINARY_EXT]
    return {"status": "fail" if probs else "pass", "detail": "; ".join(probs[:5]) or "text only"}


# ------------------------------------------------------------------ post-apply gates
def is_test_path(p: str) -> bool:
    return ad.is_test_file(p) or (p.startswith("tests/") and p.endswith((".py", ".cpp", ".c")))


def gate_assertion_delta(slot: Path, scope, pp) -> dict:
    notes, probs, n_files = [], [], 0
    for rel in scope.get("changed_files", []):
        if not is_test_path(rel):
            continue
        n_files += 1
        try:
            pre = (fc.BASE / rel).read_text(encoding="utf-8", errors="replace")
            post = (slot / rel).read_text(encoding="utf-8", errors="replace")
        except OSError as e:
            probs.append(f"{rel}: unreadable ({e})")
            continue
        cp, cq = ad.count_assertions(pre, rel), ad.count_assertions(post, rel)
        tp, tq = sum(cp.values()), sum(cq.values())
        if tq < tp:
            probs.append(f"{rel}: assertion count {tp} -> {tq}")
        dec = {k: (cp[k], cq.get(k, 0)) for k in cp if cq.get(k, 0) < cp[k]}
        if dec and tq >= tp:
            notes.append(f"{rel}: macro mix changed {dec}")
        for h in pp["files"].get(rel, []):
            rem = [t for tag, t, _ in h["lines"] if tag == "-" and ad.is_assert_line(t, rel)]
            add = [t for tag, t, _ in h["lines"] if tag == "+" and ad.is_assert_line(t, rel)]
            if len(rem) > len(add):
                probs.append(f"{rel}: hunk removes {len(rem)} assertion line(s), adds {len(add)}: {rem[0].strip()[:80]}")
            elif rem:
                notes.append(f"{rel}: {len(rem)} assertion line(s) rewritten")
    if not n_files:
        return {"status": "skip", "detail": "no test file changed"}
    return {"status": "fail" if probs else "pass", "detail": "; ".join(probs or notes)[:600] or f"{n_files} test file(s), no assertion lost"}


def gate_skip_scan(slot: Path, pp) -> dict:
    probs = []
    for rel, hunks in sorted(pp["files"].items()):
        for h in hunks:
            for tag, t, ln in h["lines"]:
                if tag != "+":
                    continue
                if SKIP_RE.search(t):
                    probs.append(f"{rel}:{ln}: added skip/disable: {t.strip()[:80]}")
                elif rel.endswith("_test.cpp") and re.match(r"^\s*return\s+0\s*;", t) and _unreachable_after(slot / rel, ln):
                    probs.append(f"{rel}:{ln}: added early `return 0;` in test main (rest of main unreachable)")
    return {"status": "fail" if probs else "pass", "detail": "; ".join(probs[:5]) or "no skip/disable added"}


def _unreachable_after(path: Path, ln: int) -> bool:
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").split("\n")
    except OSError:
        return True
    i = ln - 1
    j = i
    while j >= 0 and not re.match(r"^\s*(?:int|auto|void)\s+(\w+)\s*\(", lines[j]):
        j -= 1
    m = re.match(r"^\s*(?:int|auto|void)\s+(\w+)\s*\(", lines[j]) if j >= 0 else None
    if not m or m.group(1) != "main":
        return False
    for k in range(i + 1, len(lines)):
        s = lines[k].strip()
        if s == "}" and not lines[k].startswith((" ", "\t")):
            return False
        if s and s != "}" and not s.startswith(("//", "/*", "*")):
            return True
    return False


# ------------------------------------------------------------------ gate runner
class Ctx:
    """One locked slot + helpers around fixgate.sh."""

    def __init__(self, slot_arg, lh0: str):
        self.lh0 = lh0
        self.slot, self.lock_fd = self.acquire(slot_arg)
        self.root = fc.SLOTS / str(self.slot)
        self.tmp = fc.FIX_HOME / "tmp" / str(self.slot)
        self.gate_sh = fc.FIX_HOME / "bin" / "fixgate.sh"
        if not self.gate_sh.exists():
            fw.install_bin([])
        self.timeout = int(os.environ.get("FIX_GATE_TIMEOUT") or 3600)
        self.base_commit = fw.base_info()["base_commit"]

    @staticmethod
    def acquire(want):
        n = int(os.environ.get("FIX_SLOTS") or 4)
        cands = [int(want)] if want else list(range(1, n + 1))
        fc.SLOTS.mkdir(parents=True, exist_ok=True)
        deadline = time.time() + int(os.environ.get("FIX_SLOT_WAIT") or 900)
        while True:
            for s in cands:
                fd = open(fc.SLOTS / f"{s}.lock", "w")
                try:
                    fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
                    return s, fd
                except BlockingIOError:
                    fd.close()
            if time.time() > deadline:
                sys.exit("fix_checks: no free slot before FIX_SLOT_WAIT")
            time.sleep(1)

    def release(self):
        try:
            fcntl.flock(self.lock_fd, fcntl.LOCK_UN)
            self.lock_fd.close()
        except OSError:
            pass

    def reset(self):
        fw.slot_reset(self.slot, take_lock=False)

    def trip(self):
        if not ledger_same(self.lh0):
            raise Tripwire()

    def run(self, *args):
        """-> (rc, output). Raises Tripwire when the production ledger changed."""
        try:
            p = subprocess.run([str(self.gate_sh), str(self.root), *args], capture_output=True, text=True,
                               errors="replace", timeout=self.timeout, env=os.environ)
            rc, out = p.returncode, (p.stdout or "") + (p.stderr or "")
        except subprocess.TimeoutExpired as e:
            rc, out = 124, f"timeout after {self.timeout}s: {e}"
        self.trip()
        return rc, out

    def put_patch(self, name: str, src: Path) -> str:
        d = self.root / "_fix"
        d.mkdir(exist_ok=True)
        (d / name).write_bytes(Path(src).read_bytes())
        return f"_fix/{name}"

    def rt_failing(self):
        j = fc.rj(self.tmp / "rt.json")
        if not isinstance(j, dict):
            return None, {}
        bad = sorted(r["id"] for r in j.get("results", []) if r.get("status") in ("FAIL", "ERROR"))
        return bad, j.get("counts", {})

    def run_tests(self, flt: str):
        """-> (failing ids, counts, rc, out). A crash with no rt.json yields the pseudo id 'crash:<filter>'."""
        rc, out = self.run("test", flt)
        bad, counts = self.rt_failing()
        if bad is None:
            bad = [f"crash:{flt}"] if rc else []
        return bad, counts, rc, out

    def baseline_tests(self, flt: str) -> dict:
        key = fc.CHECKS / "baseline" / (fc.sha_hex(flt)[:24] + ".json")
        c = fc.rj(key)
        if isinstance(c, dict) and c.get("base_commit") == self.base_commit and c.get("filter") == flt:
            return c
        self.reset()
        bad, counts, rc, out = self.run_tests(flt)
        c = {"filter": flt, "base_commit": self.base_commit, "failing": bad, "counts": counts, "rc": rc}
        fc.write_json(key, c)
        return c

    def baseline_build(self, m: str) -> dict:
        """Clean-base build of mod m (cached per base commit). Resets the slot: call after the patched runs."""
        key = fc.CHECKS / "baseline" / f"build_{m}.json"
        c = fc.rj(key)
        if isinstance(c, dict) and c.get("base_commit") == self.base_commit and c.get("v") == 2:
            return c
        self.reset()
        rc, out = self.run("build", m)
        c = {"mod": m, "base_commit": self.base_commit, "v": 2, "rc": rc, "out": tail(out, BUILD_TAIL)}
        fc.write_json(key, c)
        return c

    def compat_failing(self):
        j = fc.rj(self.tmp / "compat.json")
        if isinstance(j, dict) and isinstance(j.get("results"), list):
            return sorted(str(r.get("id")) for r in j["results"] if isinstance(r, dict) and r.get("status") in ("FAIL", "ERROR"))
        return None

    def plan_mods(self, files: list[str]) -> list[str]:
        rc, out = self.run("compat", "--list", *files)
        j = fc.rj(self.tmp / "compat.json")
        mods = []
        if isinstance(j, dict):
            for it in j.get("plan", []):
                m = re.match(r"^build_check:([A-Za-z0-9_.-]+)$", str(it.get("id", "")))
                if it.get("kind") == "build" and m and m.group(1) not in mods:
                    mods.append(m.group(1))
        return sorted(mods)


def filters_for(changed: list[str], base_set: set[str]) -> list[str]:
    out = []
    for p in changed:
        f = None
        if p.startswith("scripts/") and p.endswith(".py"):
            name = p.rsplit("/", 1)[-1]
            if name.startswith("test_"):
                f = p
            elif f"scripts/test_{name}" in base_set:
                f = f"scripts/test_{name}"
        elif p.startswith("patch_manager_mods/"):
            m = fc.mod_of(p)
            if m and m != "_shared_src":
                f = m[:-4] if m.endswith("_src") else m
        elif p.startswith("tests/"):
            f = Path(p).stem
        if f and f not in out:
            out.append(f)
    return out[:MAX_FILTERS]


def write_check(path: Path, res: dict) -> None:
    fc.write_json(path, res)


def cmd_check(a) -> int:
    fw.guard_home()
    tid, n = a.tid, a.attempt
    t0 = time.time()
    lh0 = ledger_hash()
    task = fc.task_by_id(tid)
    scope = fc.rj(fc.PATCHES / f"{tid}.a{n}.scope.json")
    pf = fc.PATCHES / f"{tid}.a{n}.patch"
    out_path = fc.CHECKS / f"{tid}.a{n}.check.json"
    res = {"task_id": tid, "attempt": n, "ok": False, "gates": {g: {"status": "skip", "detail": "not run"} for g in GATES},
           "baseline": {}, "new_failures": [], "ledger_unchanged": True, "duration_s": 0.0}
    G = res["gates"]
    ctx = None
    code = 1
    try:
        pbytes = pf.read_bytes() if pf.is_file() else b""
        ptext = pbytes.decode("utf-8", errors="replace")
        pp = parse_patch(ptext)
        G["scope"] = gate_scope(task, scope, ptext, pp)
        G["excluded"] = gate_excluded(ptext, pp)
        G["binary"] = gate_binary(pbytes, pp)
        if any(G[g]["status"] == "fail" for g in ("scope", "excluded", "binary")):
            G["apply"] = {"status": "skip", "detail": "static gate failed; patch not applied"}
        else:
            ctx = Ctx(a.slot, lh0)
            ctx.reset()
            name = f"{tid}.a{n}.patch"
            rel = ctx.put_patch(name, pf)
            rc, out = ctx.run("apply", rel)
            G["apply"] = {"status": "pass" if rc == 0 else "fail", "detail": tail(out) if rc else f"applied cleanly in slot {ctx.slot}"}
            if rc == 0:
                run_post_gates(ctx, task, scope, pp, res)
    except Tripwire:
        res["ledger_unchanged"] = False
        res["ok"] = False
        res["duration_s"] = round(time.time() - t0, 2)
        write_check(out_path, res)
        sys.stderr.write("\n" + "!" * 72 + f"\nLEDGER TRIPWIRE: the production ledger {LEDGER} changed during the checks of "
                         f"{tid}.a{n}. Aborting; inspect and restore it before any further run.\n" + "!" * 72 + "\n")
        return 3
    finally:
        if ctx:
            ctx.release()
    if not ledger_same(lh0):
        res["ledger_unchanged"] = False
        res["duration_s"] = round(time.time() - t0, 2)
        write_check(out_path, res)
        sys.stderr.write(f"LEDGER TRIPWIRE: production ledger {LEDGER} changed during {tid}.a{n}\n")
        return 3
    res["ok"] = all(g["status"] != "fail" for g in G.values())
    res["duration_s"] = round(time.time() - t0, 2)
    write_check(out_path, res)
    print(fc.dumps({"task_id": tid, "attempt": n, "ok": res["ok"], "gates": {k: v["status"] for k, v in G.items()},
                    "new_failures": res["new_failures"]}), end="")
    return 0 if res["ok"] else 1


def run_post_gates(ctx: Ctx, task: dict, scope: dict, pp: dict, res: dict) -> None:
    G = res["gates"]
    changed = list(scope.get("changed_files", []))
    G["assertion_delta"] = gate_assertion_delta(ctx.root, scope, pp)
    G["skip_scan"] = gate_skip_scan(ctx.root, pp)
    shared = [p for p in changed if p.startswith("patch_manager_mods/_shared_src/")]
    # ---- build
    if shared:
        mods = ctx.plan_mods(shared)[:MAX_BUILDS]
    else:
        mods = []
        for p in changed:
            m = fc.mod_of(p)
            if m and m not in mods:
                mods.append(m)
        if task.get("mod") and task["mod"] != "_shared_src" and task["mod"] not in mods:
            mods.append(task["mod"])
        mods = mods[:MAX_BUILDS]
    failed_builds, ok_builds = [], []
    for m in mods:
        rc, out = ctx.run("build", m)
        (ok_builds if rc == 0 else failed_builds).append((m, tail(out, BUILD_TAIL)))
    # ---- tests
    flags = task.get("flags") or {}
    unsafe = [p for p in changed if p in fc.LEDGER_UNSAFE]
    base_set = set(fw.base_files())
    filters = filters_for(changed, base_set)
    patched = {}
    if shared:
        G["tests"] = {"status": "skip", "detail": "_shared_src change: consumers covered by the compat gate"}
    elif (flags.get("sandbox_tests_only") or unsafe) and os.environ.get("FIX_SANDBOX", "bwrap") != "bwrap":
        G["tests"] = {"status": "skip", "detail": "sandbox_tests_only task but FIX_SANDBOX!=bwrap: tests not run on the host"}
    elif not filters:
        G["tests"] = {"status": "skip", "detail": "no test filter derives from the changed files"}
    else:
        for flt in filters:
            bad, counts, rc, out = ctx.run_tests(flt)
            patched[flt] = {"failing": bad, "counts": counts, "rc": rc, "out": tail(out, 300)}
    # ---- compat
    comp = None
    if shared:
        rc, out = ctx.run("compat", *shared)
        comp = {"rc": rc, "failing": ctx.compat_failing(), "out": tail(out)}
    # ---- baselines (only where something failed)
    probs, news = [], []
    for flt, r in patched.items():
        if r["failing"]:
            b = ctx.baseline_tests(flt)
            res["baseline"][flt] = {"failing": b["failing"]}
            nf = sorted(set(r["failing"]) - set(b["failing"]))
            news += nf
            if nf:
                probs.append(f"{flt}: new failures {nf[:5]}")
        else:
            c = fc.rj(fc.CHECKS / "baseline" / (fc.sha_hex(flt)[:24] + ".json"))
            if isinstance(c, dict) and c.get("filter") == flt:
                res["baseline"][flt] = {"failing": c.get("failing", [])}
    if patched:
        G["tests"] = {"status": "fail" if probs else "pass",
                      "detail": "; ".join(probs) or "; ".join(f"{f}: {sum((r['counts'] or {}).values())} item(s), no new failure" for f, r in patched.items())}
    res["new_failures"] = sorted(set(news))
    if failed_builds or ok_builds:
        really, pre = [], []
        for m, o in failed_builds:
            c = ctx.baseline_build(m)
            nb = new_build_errors(o, c["out"]) if c["rc"] != 0 else None
            if c["rc"] == 0:
                really.append(f"{m}: {o[-200:]}")
            elif nb:     # base already fails, but the patch adds diagnostics of its own
                really.append(f"{m}: new build errors on top of a pre-existing failure: {nb[:5]}")
            else:
                pre.append(m)
        G["build"] = {"status": "fail" if really else "pass",
                      "detail": "; ".join(really) or ("built " + ", ".join(m for m, _ in ok_builds) + (f"; pre-existing failures on base: {pre}" if pre else ""))}
    else:
        G["build"] = {"status": "skip", "detail": "no mod source changed"}
    if comp is not None:
        if comp["rc"] == 0:
            G["compat"] = {"status": "pass", "detail": "compat_check clean"}
        else:
            key = fc.CHECKS / "baseline" / ("compat_" + fc.sha_hex(" ".join(sorted(shared)))[:24] + ".json")
            c = fc.rj(key)
            if not (isinstance(c, dict) and c.get("base_commit") == ctx.base_commit):
                ctx.reset()
                rc, out = ctx.run("compat", *shared)
                c = {"files": shared, "base_commit": ctx.base_commit, "rc": rc, "failing": ctx.compat_failing()}
                fc.write_json(key, c)
            if comp["failing"] is not None and c.get("failing") is not None:
                nf = sorted(set(comp["failing"]) - set(c["failing"]))
                res["new_failures"] = sorted(set(res["new_failures"]) | set(nf))
                G["compat"] = {"status": "fail" if nf else "pass", "detail": f"new failures {nf[:5]}" if nf else "same failures as clean base"}
            elif c.get("rc", 0) != 0:
                G["compat"] = {"status": "pass", "detail": "compat_check also fails on clean base (pre-existing)"}
            else:
                G["compat"] = {"status": "fail", "detail": comp["out"]}
    else:
        G["compat"] = {"status": "skip", "detail": "no _shared_src file changed"}


# ------------------------------------------------------------------ batches
def load_batch(batch: str) -> dict:
    cands = [Path(batch), fc.BATCHES / f"{batch}.json"]
    for c in cands:
        j = fc.rj(c)
        if isinstance(j, dict) and isinstance(j.get("tasks"), list):
            j.setdefault("batch", Path(c).stem)
            return j
    sys.exit(f"fix_checks: batch {batch} not found (looked in {fc.BATCHES})")


def resolve_patch(p: str) -> Path:
    q = Path(p)
    for c in ([q] if q.is_absolute() else [fc.FIX_DIR / q, fc.REPO / q, Path.cwd() / q]):
        if c.is_file():
            return c
    sys.exit(f"fix_checks: patch {p} not found")


def cmd_stack(a) -> int:
    fw.guard_home()
    lh0 = ledger_hash()
    b = load_batch(a.batch)
    ctx = Ctx(a.slot, lh0)
    try:
        applied, conflicts, ok_idx = [], [], []
        ctx.reset()
        for i, t in enumerate(b["tasks"], 1):
            p = resolve_patch(t["patch"])
            rel = ctx.put_patch(f"{i:02d}-{t['task_id']}.patch", p)
            rc, out = ctx.run("apply", rel)
            if rc == 0:
                applied.append(t["task_id"])
                ok_idx.append(i)
            else:
                conflicts.append({"task_id": t["task_id"], "patch": str(t["patch"]), "detail": tail(out, 600)})
                ctx.reset()
                for j in ok_idx:
                    u = b["tasks"][j - 1]
                    r2 = ctx.put_patch(f"{j:02d}-{u['task_id']}.patch", resolve_patch(u["patch"]))
                    rc2, _ = ctx.run("apply", r2)
                    if rc2:
                        sys.exit("fix_checks: re-apply of an earlier patch failed after rollback (internal)")
        res = {"batch": b["batch"], "ok": not conflicts, "applied": applied, "conflicts": conflicts,
               "ledger_unchanged": ledger_same(lh0)}
        fc.write_json(fc.INTEG / f"{b['batch']}.stack.json", res)
        print(fc.dumps(res), end="")
        return 0 if res["ok"] else 1
    except Tripwire:
        sys.stderr.write(f"LEDGER TRIPWIRE: {LEDGER} changed during stack {a.batch}\n")
        return 3
    finally:
        ctx.release()


def integ_series(batch: str) -> list[dict]:
    for name in (f"{batch}.json", f"{batch}.run.json"):
        j = fc.rj(fc.INTEG / name)
        if isinstance(j, dict):
            if isinstance(j.get("series"), list):
                return j["series"]
            if isinstance(j.get("output"), dict) and isinstance(j["output"].get("series"), list):
                return j["output"]["series"]
    sys.exit(f"fix_checks: no integrator output with a series in {fc.INTEG}/{batch}[.run].json")


def cmd_verify_series(a) -> int:
    fw.guard_home()
    lh0 = ledger_hash()
    t0 = time.time()
    b = load_batch(a.batch)
    series = integ_series(b["batch"])
    ctx = Ctx(a.slot, lh0)
    res = {"batch": b["batch"], "ok": False, "applied": [], "dropped": [], "problems": [], "gates": [],
           "new_failures": [], "ledger_unchanged": True, "duration_s": 0.0}
    out_path = fc.INTEG / f"{b['batch']}.verify.json"
    try:
        orig = {(t["task_id"]): t["patch"] for t in b["tasks"]}
        listed = [s["task_id"] for s in series]
        if sorted(listed) != sorted(orig):
            res["problems"].append(f"series task ids {sorted(listed)} != batch task ids {sorted(orig)}")
        ctx.reset()
        for i, s in enumerate(series, 1):
            tid = s["task_id"]
            if s.get("action") == "drop":
                res["dropped"].append(tid)
                continue
            if s.get("action") == "modified":
                rp = s.get("resolved_patch") or ""
                src = fc.INTEG_HOME / b["batch"] / rp
                if fc.safe_rel(rp) is None or not src.is_file() or src.is_symlink():
                    res["problems"].append(f"{tid}: resolved_patch {rp!r} missing under integ/{b['batch']}")
                    continue
            elif tid in orig:
                src = resolve_patch(orig[tid])
            else:
                res["problems"].append(f"{tid}: not in batch")
                continue
            text = src.read_text(encoding="utf-8", errors="replace")
            pp = parse_patch(text)
            bad = [p for p in pp["paths"] if fc.safe_rel(p) is None or fc.is_excluded(p)]
            if bad or pp["bad_headers"] or b"\0" in src.read_bytes():
                res["problems"].append(f"{tid}: patch has excluded/unsafe/binary content {bad[:3]}")
                continue
            rel = ctx.put_patch(f"{i:02d}-{tid}.patch", src)
            rc, out = ctx.run("apply", rel)
            if rc:
                res["problems"].append(f"{tid}: apply failed: {tail(out, 300)}")
            else:
                res["applied"].append(tid)
        gates = b.get("gates") or {}
        bl = b.get("baseline") if isinstance(b.get("baseline"), dict) else {}

        def rec(cmd, rc, status, detail):
            res["gates"].append({"cmd": cmd, "exit_code": rc, "status": status, "detail": detail})

        if not res["problems"]:
            bfail = []
            for m in gates.get("build", []):
                rc, out = ctx.run("build", m)
                if rc == 0:
                    rec(f"build {m}", rc, "pass", tail(out, 300))
                else:
                    bfail.append((m, rc, tail(out, BUILD_TAIL)))
            failing = []
            for flt in gates.get("tests", []):
                bad, counts, rc, out = ctx.run_tests(flt)
                if bad:
                    failing.append((flt, bad, rc))
                else:
                    rec(f"test {flt}", rc, "pass", f"{sum((counts or {}).values())} item(s)")
            if gates.get("compat"):
                rc, out = ctx.run("compat", *gates["compat"])
                rec("compat " + " ".join(gates["compat"]), rc, "pass" if rc == 0 else "fail", tail(out, 300))
            rc, out = ctx.run("diffcheck")
            rec("diffcheck", rc, "info", tail(out, 200))
            for flt, bad, rc in failing:       # baselines last: they reset the slot to the clean base
                base_bad = bl.get(flt)
                base_bad = base_bad.get("failing") if isinstance(base_bad, dict) else base_bad
                if not isinstance(base_bad, list):
                    base_bad = ctx.baseline_tests(flt)["failing"]
                nf = sorted(set(bad) - set(base_bad))
                res["new_failures"] += nf
                rec(f"test {flt}", rc, "fail" if nf else "pass", f"new failures {nf[:5]}" if nf else "only pre-existing failures")
            for m, rc, out in bfail:           # same rule as the per-task build gate
                c = ctx.baseline_build(m)
                nb = new_build_errors(out, c["out"]) if c["rc"] != 0 else None
                if c["rc"] != 0 and not nb:
                    rec(f"build {m}", rc, "pass", "only pre-existing build failures on base: " + tail(out, 200))
                else:
                    rec(f"build {m}", rc, "fail", (f"new build errors {nb[:5]}; " if nb else "") + tail(out, 300))
    except Tripwire:
        res["ledger_unchanged"] = False
        res["duration_s"] = round(time.time() - t0, 2)
        fc.write_json(out_path, res)
        sys.stderr.write(f"LEDGER TRIPWIRE: {LEDGER} changed during verify-series {a.batch}\n")
        return 3
    finally:
        ctx.release()
    res["ok"] = not res["problems"] and all(g["status"] != "fail" for g in res["gates"])
    res["new_failures"] = sorted(set(res["new_failures"]))
    res["ledger_unchanged"] = ledger_same(lh0)
    res["duration_s"] = round(time.time() - t0, 2)
    fc.write_json(out_path, res)
    print(fc.dumps(res), end="")
    if not res["ledger_unchanged"]:
        return 3
    return 0 if res["ok"] else 1


def cmd_baseline(a) -> int:
    """Print one clean-base line per gate of a batch (integrator dossier appendix). Build baselines are computed
    and cached when missing (resets a slot); test baselines are only read from the cache."""
    fw.guard_home()
    b = load_batch(a.batch)
    gates = b.get("gates") or {}
    base_commit = fw.base_info()["base_commit"]
    lines, ctx = [], None
    try:
        for m in gates.get("build", []):
            c = fc.rj(fc.CHECKS / "baseline" / f"build_{m}.json")
            if not (isinstance(c, dict) and c.get("base_commit") == base_commit and c.get("v") == 2):
                ctx = ctx or Ctx(a.slot, ledger_hash())
                c = ctx.baseline_build(m)
            lines.append(f"baseline: build {m} on clean base = {'PASS' if c['rc'] == 0 else 'FAIL'}"
                         " (pre-existing failures are not caused by this series)")
        bl = b.get("baseline") if isinstance(b.get("baseline"), dict) else {}
        for flt in gates.get("tests", []):
            c = bl.get(flt)
            bad = c.get("failing") if isinstance(c, dict) else c
            if not isinstance(bad, list):
                c = fc.rj(fc.CHECKS / "baseline" / (fc.sha_hex(flt)[:24] + ".json"))
                ok = isinstance(c, dict) and c.get("base_commit") == base_commit and c.get("filter") == flt
                bad = c.get("failing") if ok else None
            if isinstance(bad, list):
                lines.append(f"baseline: test {flt} on clean base = {'FAIL ' + ', '.join(bad[:5]) if bad else 'PASS'}"
                             " (pre-existing failures are not caused by this series)")
            else:
                lines.append(f"baseline: test {flt} not cached (compare against the per-task check.json `baseline` field)")
    finally:
        if ctx:
            ctx.release()
    print("\n".join(lines))
    return 0


# ------------------------------------------------------------------ self-test
def self_test_inner(T: Path) -> int:
    bad = []
    n_chk = [0]

    def chk(c, m):
        n_chk[0] += 1
        if not c:
            bad.append(m)

    # build diagnostics: a still-failing build passes only when the patch adds no diagnostic of its own
    b0 = "a.cpp:10:3: error: x undeclared\nld: cannot export Foo: symbol not defined\nFAIL build: m after 2.5s"
    chk(new_build_errors(b0.replace(":10:3", ":12:3").replace("2.5s", "3.1s"), b0) == [], "shifted line/timing seen as new error")
    chk(new_build_errors(b0 + "\nld: cannot export stub-fix: symbol not defined", b0) == ["ld: cannot export stub-fix: symbol not defined"],
        "added link error not detected")

    env = dict(os.environ)
    guard = T / "guard_ledger.jsonl"
    guard.write_text('{"x":1}\n')
    env["FIX_LEDGER_GUARD"] = str(guard)
    wsp = str(Path(fw.__file__).resolve())
    me = str(Path(__file__).resolve())

    def ws(*args, expect=0):
        p = subprocess.run([sys.executable, "-I", wsp, *args], capture_output=True, text=True, env=env)
        if expect is not None and p.returncode != expect:
            raise AssertionError(f"ws {args}: rc {p.returncode} {p.stdout[-300:]} {p.stderr[-300:]}")
        return p

    def checks(*args):
        return subprocess.run([sys.executable, "-I", me, *args], capture_output=True, text=True, env=env)

    ws("build-base", "--no-snapshot-check", "--repo", str(T / "repo"))
    chk((fc.FIX_HOME / "bin" / "fixgate.sh").exists(), "install-bin ran")

    def mkpatch(tid, n, edit, tweak=None):
        ws("make-task", tid, "--attempt", str(n))
        c = fc.TASKS / f"{tid}.a{n}"
        edit(c)
        ws("diff", tid, "--attempt", str(n))

    def sub(path, old, new):
        def f(c):
            p = c / path
            t = p.read_text()
            assert old in t, (path, old)
            p.write_text(t.replace(old, new, 1))
        return f

    def run(tid, n):
        p = checks("check", tid, "--attempt", str(n))
        j = fc.rj(fc.CHECKS / f"{tid}.a{n}.check.json")
        return p, j

    # A: good patch
    mkpatch("T0000000001", 1, sub("scripts/a.py", "return x - y", "return x - y  # fine"))
    p, j = run("T0000000001", 1)
    chk(p.returncode == 0 and j and j["ok"], f"good patch not ok: {p.stdout[-300:]} {p.stderr[-300:]} {j}")
    if j:
        chk(j["gates"]["apply"]["status"] == "pass" and j["gates"]["tests"]["status"] == "pass"
            and j["gates"]["build"]["status"] == "skip" and j["gates"]["compat"]["status"] == "skip", f"A gates {j['gates']}")
        chk(j["ledger_unchanged"] is True and j["new_failures"] == [], "A ledger/new_failures")
    # B: new failure introduced by the patch (a.add broken)
    mkpatch("T0000000001", 2, sub("scripts/a.py", "return x + y", "return x - y"))
    p, j = run("T0000000001", 2)
    chk(p.returncode == 1 and j and not j["ok"] and j["gates"]["tests"]["status"] == "fail"
        and any("test_a" in f for f in j["new_failures"]), f"new failure not caught: {j and j['gates']['tests']} {j and j['new_failures']}")
    chk(j and "scripts/test_a.py" in j["baseline"] and j["baseline"]["scripts/test_a.py"]["failing"] == [], "baseline recorded")
    # C: assertion-deleting patch
    mkpatch("T0000000002", 1, sub("scripts/test_a.py", "assert a.add(2, 2) == 4\n", ""))
    p, j = run("T0000000002", 1)
    chk(j and not j["ok"] and j["gates"]["assertion_delta"]["status"] == "fail", f"assert delete not caught {j and j['gates']}")
    # C2: assertion rewritten (stricter) is fine
    mkpatch("T0000000002", 2, sub("scripts/test_a.py", "assert a.add(2, 2) == 4", "assert a.add(2, 2) == 4 and a.add(0, 0) == 0"))
    p, j = run("T0000000002", 2)
    chk(j and j["ok"] and j["gates"]["assertion_delta"]["status"] == "pass", f"assert rewrite wrongly failed {j and j['gates']}")
    # D: skip-adding patch
    mkpatch("T0000000002", 3, sub("scripts/test_a.py", "import a\n", "import a\nimport unittest\nunittest.skip('flaky')\n"))
    p, j = run("T0000000002", 3)
    chk(j and not j["ok"] and j["gates"]["skip_scan"]["status"] == "fail", f"skip not caught {j and j['gates']}")
    # D2: early return 0 in a *_test.cpp main
    mkpatch("T0000000003", 1, sub("patch_manager_mods/foo_src/foo_test.cpp", "    KT_CHECK(foo() == 1);\n", "    return 0;\n    KT_CHECK(foo() == 1);\n"))
    p, j = run("T0000000003", 1)
    chk(j and not j["ok"] and j["gates"]["skip_scan"]["status"] == "fail" and j["gates"]["assertion_delta"]["status"] == "pass",
        f"early return not caught {j and j['gates']}")
    # E: pre-existing baseline failure is not blamed on the patch
    mkpatch("T0000000008", 1, sub("scripts/test_old.py", "import sys\n", "import sys\n# note\n"))
    p, j = run("T0000000008", 1)
    chk(j and j["ok"] and j["new_failures"] == [] and j["baseline"].get("scripts/test_old.py", {}).get("failing") == ["py:scripts/test_old.py"],
        f"baseline failure blamed on patch: {j and j['gates']['tests']} {j and j['baseline']}")
    # F: build failure
    mkpatch("T0000000004", 1, sub("patch_manager_mods/foo_src/Foo.cpp", "return 1;", "return 1; SYNTAX_ERROR"))
    p, j = run("T0000000004", 1)
    chk(j and not j["ok"] and j["gates"]["build"]["status"] == "fail", f"build failure not caught {j and j['gates']}")
    mkpatch("T0000000004", 2, sub("patch_manager_mods/foo_src/Foo.cpp", "return 1;", "return 2;"))
    p, j = run("T0000000004", 2)
    chk(j and j["gates"]["build"]["status"] == "pass", f"good build not pass {j and j['gates']['build']}")
    # G: _shared_src -> compat plan, builds, compat
    mkpatch("T0000000005", 1, sub("patch_manager_mods/_shared_src/H.h", "#pragma once\n", "#pragma once\n// note\n"))
    p, j = run("T0000000005", 1)
    chk(j and j["ok"] and j["gates"]["build"]["status"] == "pass" and "foo_src" in j["gates"]["build"]["detail"]
        and j["gates"]["compat"]["status"] == "pass" and j["gates"]["tests"]["status"] == "skip", f"shared {j and j['gates']}")
    # H: sandbox_tests_only skipped without bwrap
    mkpatch("T0000000007", 1, sub("scripts/test_a.py", "import a\n", "import a  # x\n"))
    p, j = run("T0000000007", 1)
    chk(j and j["gates"]["tests"]["status"] == "skip" and "sandbox" in j["gates"]["tests"]["detail"], f"sandbox_tests_only {j and j['gates']['tests']}")
    # I: excluded path + binary in a hand-made patch
    fc.PATCHES.mkdir(parents=True, exist_ok=True)
    (fc.PATCHES / "T0000000001.a9.patch").write_text(
        "diff --git a/.env b/.env\nindex 1111111..2222222 100644\n--- a/.env\n+++ b/.env\n@@ -1 +1 @@\n-A=1\n+A=2\n"
        "diff --git a/scripts/x.tga b/scripts/x.tga\nGIT binary patch\nliteral 0\n")
    fc.write_json(fc.PATCHES / "T0000000001.a9.scope.json", {"ok": True, "empty": False, "changed_files": [".env"], "violations": [], "over_cap": False, "line_cap": 40})
    p, j = run("T0000000001", 9)
    chk(j and not j["ok"] and j["gates"]["excluded"]["status"] == "fail" and j["gates"]["binary"]["status"] == "fail"
        and j["gates"]["apply"]["status"] == "skip", f"excluded/binary {j and j['gates']}")
    # J: scope violation (over cap) fails the scope gate
    mkpatch("T0000000006", 1, lambda c: (c / "scripts/b.py").write_text("X = 1\nY = 2\nZ = 3\nW = 4\n"))
    p, j = run("T0000000006", 1)
    chk(j and not j["ok"] and j["gates"]["scope"]["status"] == "fail" and "over_cap" in j["gates"]["scope"]["detail"], f"over cap {j and j['gates']['scope']}")
    # K: ledger tripwire
    def evil(c):
        p_ = c / "scripts/test_a.py"
        p_.write_text(p_.read_text() + "open(open('scripts/ledger_target.txt').read().strip(), 'a').write('tamper\\n')\n")
    mkpatch("T0000000002", 4, evil)
    p, j = run("T0000000002", 4)
    chk(p.returncode == 3 and "LEDGER TRIPWIRE" in p.stderr and j and j["ledger_unchanged"] is False and not j["ok"], f"tripwire rc={p.returncode} {p.stderr[-200:]}")
    guard.write_text('{"x":1}\n')
    # L: stack + verify-series
    ws("make-task", "T0000000001", "--attempt", "3")
    sub("scripts/a.py", "return x + y", "return x + y  # b1")(fc.TASKS / "T0000000001.a3")
    ws("diff", "T0000000001", "--attempt", "3")
    ws("make-task", "T0000000002", "--attempt", "5")
    sub("scripts/test_a.py", "assert a.add(1, 2) == 3", "assert a.add(1, 2) == 3  # c")(fc.TASKS / "T0000000002.a5")
    ws("diff", "T0000000002", "--attempt", "5")
    ws("make-task", "T000000000a", "--attempt", "1")
    sub("scripts/a.py", "return x + y", "return x + y  # conflicting")(fc.TASKS / "T000000000a.a1")
    ws("diff", "T000000000a", "--attempt", "1")
    P = lambda t, n: str(fc.PATCHES / f"{t}.a{n}.patch")
    fc.BATCHES.mkdir(parents=True, exist_ok=True)
    fc.write_json(fc.BATCHES / "batch_01.json", {"batch": "batch_01", "mods": [], "files": ["scripts/a.py", "scripts/test_a.py"],
        "tasks": [{"task_id": "T0000000001", "attempt": 3, "patch": P("T0000000001", 3)},
                  {"task_id": "T0000000002", "attempt": 5, "patch": P("T0000000002", 5)}],
        "gates": {"build": [], "tests": ["scripts/test_a.py"], "compat": []}, "baseline": {}})
    p = checks("stack", "batch_01")
    chk(p.returncode == 0 and json.loads(p.stdout)["ok"], f"stack clean batch {p.stdout[-200:]} {p.stderr[-200:]}")
    fc.write_json(fc.BATCHES / "batch_02.json", {"batch": "batch_02", "mods": [], "files": ["scripts/a.py"],
        "tasks": [{"task_id": "T0000000001", "attempt": 3, "patch": P("T0000000001", 3)},
                  {"task_id": "T000000000a", "attempt": 1, "patch": P("T000000000a", 1)}],
        "gates": {"build": [], "tests": [], "compat": []}, "baseline": {}})
    p = checks("stack", "batch_02")
    s = json.loads(p.stdout) if p.stdout.strip() else {}
    chk(p.returncode == 1 and not s.get("ok") and len(s.get("conflicts", [])) == 1 and s["applied"] == ["T0000000001"], f"stack conflict {p.stdout[-300:]}")
    fc.write_json(fc.INTEG / "batch_01.json", {"batch": "batch_01", "notes": "", "gates_run": [], "series": [
        {"task_id": "T0000000001", "attempt": 3, "action": "keep", "reason": "ok"},
        {"task_id": "T0000000002", "attempt": 5, "action": "keep", "reason": "ok"}]})
    p = checks("verify-series", "batch_01")
    v = fc.rj(fc.INTEG / "batch_01.verify.json")
    chk(p.returncode == 0 and v and v["ok"] and v["applied"] == ["T0000000001", "T0000000002"]
        and any(g["cmd"] == "test scripts/test_a.py" and g["status"] == "pass" for g in v["gates"]), f"verify-series {p.stdout[-300:]} {p.stderr[-300:]} {v}")
    fc.write_json(fc.INTEG / "batch_02.json", {"batch": "batch_02", "notes": "", "gates_run": [], "series": [
        {"task_id": "T0000000001", "attempt": 3, "action": "keep", "reason": "ok"}]})
    p = checks("verify-series", "batch_02")
    v = fc.rj(fc.INTEG / "batch_02.verify.json")
    chk(p.returncode == 1 and v and not v["ok"], "verify-series with a missing task should fail")
    # M: prefix-tolerant ledger tripwire, in-process with the module LEDGER global pointed at a temp file
    global LEDGER
    saved_ledger = LEDGER
    ul = T / "unit_ledger.jsonl"
    base = b'{"x":1}\n'
    try:
        LEDGER = ul

        def fresh(content: bytes) -> str:
            ul.write_bytes(content)
            return ledger_hash()

        def app(extra: bytes):
            with open(ul, "ab") as f:
                f.write(extra)

        tok = fresh(base)
        chk(ledger_same(tok), "M(a) unchanged not same")
        app(b'{"event":"CONTRACT_CHECK","ok":true}\n')
        chk(ledger_same(tok), "M(b) appended CONTRACT_CHECK not tolerated")
        tok = fresh(base)
        app(b'{"event":"SESSION_HANDOFF","plan":"/x/10_equipment.md"}\n')
        chk(ledger_same(tok), "M(c) appended other-plan SESSION_HANDOFF not tolerated")
        tok = fresh(base)
        app(b'{"event":"SESSION_HANDOFF","plan":".claude/review/x"}\n')
        chk(not ledger_same(tok), "M(d) SESSION_HANDOFF mentioning .claude/review tolerated")
        tok = fresh(base)
        app(b'{"event":"PHASE_DONE"}\n')
        chk(not ledger_same(tok), "M(e) non-benign event tolerated")
        tok = fresh(base)
        app(b"garbage not json\n")
        chk(not ledger_same(tok), "M(f) non-JSON append tolerated")
        tok = fresh(base)
        ul.write_bytes(b'{"x":2}\n')
        chk(not ledger_same(tok), "M(g) modified original byte tolerated")
        tok = fresh(base)
        ul.write_bytes(base[:-1])
        chk(not ledger_same(tok), "M(h) truncation tolerated")
        ul.unlink()
        chk(ledger_hash() == "absent" and ledger_same("absent"), "M(i) absent not same while absent")
        app(b'{"event":"CONTRACT_CHECK"}\n')
        chk(not ledger_same("absent"), "M(i) created ledger still matched absent token")
    finally:
        LEDGER = saved_ledger

    # production ledger untouched (the real one)
    print("fix_checks self-test:", "PASS" if not bad else "FAIL " + "; ".join(bad), f"({n_chk[0]} checks)")
    return 1 if bad else 0


def main(argv: list[str]) -> int:
    if argv[:1] == ["--self-test"]:
        real0 = ledger_hash()
        r = fw.reexec_selftest(str(Path(__file__).resolve()))
        # Append-only, not byte-identical: other sessions share this worktree and append BUILD_GATE / HOTSWAP_APPLY / PHASE_DONE lines at any time.
        if not ledger_same(real0, any_event=True):
            print("fix_checks self-test: FAIL production ledger changed!")
            return 3
        return r
    if argv[:1] == ["--self-test-inner"]:
        return self_test_inner(Path(argv[1]))
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("check")
    s.add_argument("tid")
    s.add_argument("--attempt", type=int, required=True)
    s.add_argument("--slot", type=int)
    for nm in ("stack", "verify-series", "baseline"):
        s = sub.add_parser(nm)
        s.add_argument("batch")
        s.add_argument("--slot", type=int)
    a = ap.parse_args(argv)
    if a.cmd == "check":
        if not fc.TASK_ID_RE.match(a.tid):
            sys.exit(f"bad task id {a.tid}")
        return cmd_check(a)
    if a.cmd == "baseline":
        return cmd_baseline(a)
    return cmd_stack(a) if a.cmd == "stack" else cmd_verify_series(a)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
