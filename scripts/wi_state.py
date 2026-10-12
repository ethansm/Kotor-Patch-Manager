#!/usr/bin/env python3
"""wi_state.py - claim files, flocks, durable state events, run telemetry, index render and backlog items for wi_driver.py.

Ephemeral claims live OUTSIDE the worktree in `<git-common-dir>/kotor2-wi-claims/` so they never show up as dirty files and
are shared by every worktree of the repo:
  WI-NN.claim    JSON {token, session, claimed_at, heartbeat_at, wave_prefix, locks, base_head, retries, stage, trace}
  .select.lock   flock held while choosing + creating a claim
  commit.flock   flock held around state append + index render + git commit
  docs.flock     flock held around plan_manager mark-done calls
Durable, tracked state lives in .claude/work_items/:
  state.jsonl    append-only {wi, ev: done|park|unpark, ts, session, reason?}
  runs.jsonl     one telemetry record per closed or parked attempt (read by `wi_driver.py audit`)
  lessons.jsonl  corrections written by audit and echoed back by `next`
"""
import contextlib
import datetime as dt
import fcntl
import json
import os
import re
import secrets
import subprocess
import sys
import time
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPTS_DIR))
import wi_model as wm  # noqa: E402

STALE_SECONDS = 2 * 3600
STATE_FILE = wm.WORK_DIR / "state.jsonl"
RUNS_FILE = wm.WORK_DIR / "runs.jsonl"
LESSONS_FILE = wm.WORK_DIR / "lessons.jsonl"
INDEX_FILE = wm.WORK_DIR / wm.INDEX_NAME
MARKS = {"done": "x", "park": "!"}


def now_iso():
    return dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def parse_iso(ts):
    return dt.datetime.strptime(ts, "%Y-%m-%dT%H:%M:%SZ").replace(tzinfo=dt.timezone.utc)


def git(root, *args, check=True):
    return subprocess.run(["git", *args], cwd=str(root), capture_output=True, text=True, check=check)


def claims_dir_for(root):
    common = git(root, "rev-parse", "--git-common-dir").stdout.strip()
    p = Path(common)
    p = p if p.is_absolute() else Path(root) / p
    return (p / "kotor2-wi-claims").resolve()


class Store:
    """All wi_driver persistence for one repo root. `claims_dir` override is for tests."""

    def __init__(self, root=None, claims_dir=None):
        self.root = Path(root or wm.REPO).resolve()
        self.claims = Path(claims_dir) if claims_dir else claims_dir_for(self.root)
        self.claims.mkdir(parents=True, exist_ok=True)

    # ---------------------------------------------------------- locks
    @contextlib.contextmanager
    def flock(self, name):
        with open(self.claims / name, "a+") as fh:
            fcntl.flock(fh, fcntl.LOCK_EX)
            try:
                yield
            finally:
                fcntl.flock(fh, fcntl.LOCK_UN)

    # ---------------------------------------------------------- claims
    def claim_path(self, wi_id):
        return self.claims / f"{wi_id}.claim"

    def read_claim(self, wi_id):
        try:
            return json.loads(self.claim_path(wi_id).read_text(encoding="utf-8"))
        except (OSError, ValueError):
            return None

    def write_claim(self, wi_id, claim):
        p = self.claim_path(wi_id)
        tmp = p.with_name(f"{p.name}.tmp.{os.getpid()}")
        tmp.write_text(json.dumps(claim, indent=1), encoding="utf-8")
        os.replace(tmp, p)

    def create_claim(self, wi_id, session, locks, base_head, wave_prefix):
        """O_EXCL create. Returns the claim dict, or None when another session already holds it."""
        claim = {"token": secrets.token_hex(6), "session": session, "claimed_at": now_iso(), "heartbeat_at": now_iso(),
                 "wave_prefix": wave_prefix, "locks": locks, "base_head": base_head, "retries": 0, "stage": "claimed",
                 "wave": None, "trace": []}
        try:
            fd = os.open(self.claim_path(wi_id), os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o644)
        except FileExistsError:
            return None
        with os.fdopen(fd, "w", encoding="utf-8") as fh:
            json.dump(claim, fh, indent=1)
        return claim

    def all_claims(self):
        out = {}
        for p in sorted(self.claims.glob("WI-*.claim")):
            c = self.read_claim(p.stem)
            if c:
                out[p.stem] = c
        return out

    def is_stale(self, claim):
        return (dt.datetime.now(dt.timezone.utc) - parse_iso(claim["heartbeat_at"])).total_seconds() > STALE_SECONDS

    def release(self, wi_id):
        with contextlib.suppress(FileNotFoundError):
            self.claim_path(wi_id).unlink()

    def trace(self, wi_id, phase, status, detail="", **extra):
        """Append a telemetry entry to the claim and refresh its heartbeat."""
        c = self.read_claim(wi_id)
        if not c:
            return None
        c["trace"].append({"ts": now_iso(), "phase": phase, "status": status, "detail": detail[:300], **extra})
        c["heartbeat_at"] = now_iso()
        self.write_claim(wi_id, c)
        return c

    # ---------------------------------------------------------- durable state
    def _jsonl(self, rel):
        p = self.root / rel
        out = []
        if p.is_file():
            for line in p.read_text(encoding="utf-8").splitlines():
                with contextlib.suppress(ValueError):
                    out.append(json.loads(line))
        return out

    def events(self):
        return self._jsonl(STATE_FILE)

    def runs(self):
        return self._jsonl(RUNS_FILE)

    def lessons(self):
        return self._jsonl(LESSONS_FILE)

    def state_maps(self):
        """(done set, parked {wi: reason}) from the last event per WI."""
        last = {}
        for e in self.events():
            if e["ev"] != "rescue":  # a rescue marker never changes whether the item is parked or done
                last[e["wi"]] = e
        done = {w for w, e in last.items() if e["ev"] == "done"}
        parked = {w: e.get("reason", "") for w, e in last.items() if e["ev"] == "park"}
        return done, parked

    def append_jsonl(self, rel, record):
        p = self.root / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        with open(p, "a", encoding="utf-8") as fh:
            fh.write(json.dumps(record, sort_keys=True) + "\n")

    def append_event(self, wi_id, ev, session, reason=""):
        rec = {"wi": wi_id, "ev": ev, "ts": now_iso(), "session": session}
        if reason:
            rec["reason"] = reason
        self.append_jsonl(STATE_FILE, rec)

    # ---------------------------------------------------------- index render
    def render_index(self):
        """Rewrite the done cell of every index row from state events. Returns the new text (also written atomically)."""
        done, parked = self.state_maps()
        p = self.root / INDEX_FILE
        out = []
        for line in p.read_text(encoding="utf-8").split("\n"):
            m = wm.INDEX_ROW_RE.match(line)
            if m:
                mark = "x" if m.group("id") in done else ("!" if m.group("id") in parked else " ")
                line = f"| [{mark}]" + line[5:]
            out.append(line)
        text = "\n".join(out)
        tmp = p.with_name(f"{p.name}.tmp.{os.getpid()}")
        tmp.write_text(text, encoding="utf-8")
        os.replace(tmp, p)
        return text

    # ---------------------------------------------------------- backlog
    def next_wi_id(self, items):
        nums = [int(re.match(r"WI-(\d+)", w).group(1)) for w in items]
        return f"WI-{max(nums, default=0) + 1:02d}"

    def add_backlog_item(self, items, title, area, why, files=(), origin="manual", state="hold", size="S", goal=""):
        """Create WI-NN.md + an index row at the end of the table. Caller holds commit.flock and commits the two paths."""
        wid = self.next_wi_id(items)
        rank = max((w.rank for w in items.values()), default=0) + 1
        title = " ".join(title.split())[:90]
        body = (f"# {wid} {title}\narea: {area}\nsize: {size}\nready: {state}\nitems: -\n"
                f"files: {'; '.join(files) if files else 'none'}\nout-of-repo: none\n"
                f"blockers: Backlog item ({origin}); needs triage before it can run: {why}\n"
                f"docs: none\n\n## Goal\n{goal or title}\n## Steps\n1. Triage: confirm scope, files and a verify command, then set `ready:` and add a `verify:` line.\n"
                f"## Verify\n1. unknown\n## Docs\n1. none\n\n## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)\n")
        (self.root / wm.WORK_DIR / f"{wid}.md").write_text(body, encoding="utf-8")
        row = (f"| [ ] | {rank} | {wid} | P3 | {state} | {size} | - | {title} | 0 | {len(files)} | {why[:70]} |")
        ip = self.root / INDEX_FILE
        text = ip.read_text(encoding="utf-8").rstrip("\n")
        ip.write_text(text + "\n" + row + "\n", encoding="utf-8")
        return wid

    def set_state(self, wi_id, state):
        """Change `ready:` in the WI header and the state cell of its index row (promote / audit hold)."""
        f = self.root / wm.WORK_DIR / f"{wi_id}.md"
        t = f.read_text(encoding="utf-8")
        t = re.sub(r"(?m)^ready:.*$", f"ready: {state}", t, count=1)
        f.write_text(t, encoding="utf-8")
        ip = self.root / INDEX_FILE
        lines = ip.read_text(encoding="utf-8").split("\n")
        for i, line in enumerate(lines):
            m = wm.INDEX_ROW_RE.match(line)
            if m and m.group("id") == wi_id:
                cells = line.split("|")
                cells[5] = f" {'ready' if state == 'awaiting-user' else state} "
                lines[i] = "|".join(cells)
        ip.write_text("\n".join(lines), encoding="utf-8")

    def prepend_blocker(self, wi_id, note):
        f = self.root / wm.WORK_DIR / f"{wi_id}.md"
        t = f.read_text(encoding="utf-8")
        t = re.sub(r"(?m)^blockers:\s*", lambda m: f"blockers: {note} ", t, count=1)
        f.write_text(t, encoding="utf-8")
