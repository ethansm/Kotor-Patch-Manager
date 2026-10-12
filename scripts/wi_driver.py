#!/usr/bin/env python3
"""wi_driver.py - claim, run, verify, document, commit and audit one dev work item per call. Python 3, stdlib only.

Usage:
  wi_driver.py next [WI-NN] [--adopt] [--dry-run] [--session S]
  wi_driver.py verify WI-NN [--dry-run]
  wi_driver.py docs WI-NN [--dry-run]
  wi_driver.py close WI-NN [--dry-run] | close WI-NN --manual --paths P [P ...] --reason TEXT
  wi_driver.py report WI-NN
  wi_driver.py park WI-NN --reason TEXT [--dry-run]
  wi_driver.py rescue WI-NN [--collect] [--dry-run]
  wi_driver.py unpark WI-NN --reason TEXT [--dry-run]
  wi_driver.py release WI-NN [--force]
  wi_driver.py status
  wi_driver.py lint
  wi_driver.py audit [--apply] [--dry-run]
  wi_driver.py backlog {add,list,promote} ...      add --title T --area A --why W [--files F;F] [--origin O] [--state S]
                                                    promote WI-NN --state {ready,verify-only,hold}

One session = one item: `next` claims the first eligible row and prints the Agent spawn line for ONE Haiku worker; `verify`
runs the gate and prints the checker spawn line; `docs` marks plan rows and may print a docs-worker line; `close` commits only
this item's paths; `report` prints the Saved State block. `park` and `release` never leave a claim behind.
`audit` reads runs.jsonl/state.jsonl, prints findings, and with --apply writes lessons (echoed by `next`), holds items that keep
failing, and pushes follow-up items onto the backlog (state hold, promote with `backlog promote`).
Every mutating command accepts --dry-run. Claims live under <git-common-dir>/kotor2-wi-claims/ (see wi_state.py).
Exit: 0 ok; 1 usage or failed step (retry/park printed); 2 parked or hard stop; 3 nothing claimable.
"""
import argparse
import json
import os
import re
import shlex
import statistics
import subprocess
import sys
import tempfile
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPTS_DIR))
import agent_contract as ac  # noqa: E402
import wave_budget  # noqa: E402
import wave_tools  # noqa: E402
import wi_model as wm  # noqa: E402
import wi_state as ws  # noqa: E402

TRAILER = "Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>"
GATE_PROGRAMS = {"python3", "rg", "bash", "test", "sed", "wc", "stat"}
GATE_TIMEOUT = 1500
DOC_PAD_LINES = 15  # context lines kept around each cited plan-doc row in a brief entry
DOC_MAX_SPAN = 150
RESCUE_UNITS = 7
WAVE_TOOLS = SCRIPTS_DIR / "wave_tools.py"
PLAN_MANAGER = SCRIPTS_DIR / "plan_manager.py"
RUN_TESTS_SUMMARY = re.compile(r"(\d+) passed, (\d+) failed, (\d+) error, (\d+) skipped \((\d+) items\)")
BUILD_CHECK_SUMMARY = re.compile(r"FAIL=(\d+) WARN=\d+ INFO=\d+ PASS=(\d+) SKIP=(\d+)")


class DriverError(Exception):
    def __init__(self, msg, code=1):
        super().__init__(msg)
        self.code = code


def session_id(arg=None):
    return arg or os.environ.get("WI_SESSION") or os.environ.get("CLAUDE_CODE_SESSION_ID") or f"ppid-{os.getppid()}"


def run(argv, root, env=None, timeout=None):
    return subprocess.run(argv, cwd=str(root), capture_output=True, text=True, env=env, timeout=timeout)


def last_lines(text, n=12):
    return "\n".join(text.strip().splitlines()[-n:])


def is_game_running():
    return subprocess.run(["pgrep", "-f", r"^S:.*swkotor2\.exe"], stdout=subprocess.DEVNULL).returncode == 0


def ledger(root, event, **fields):
    """Best-effort audit event; never fails the command."""
    try:
        run([sys.executable, str(SCRIPTS_DIR / "master_ledger.py"), "log", event] + [f"{k}={v}" for k, v in fields.items()], root, timeout=30)
    except (OSError, subprocess.SubprocessError):
        pass


def need_claim(st, wi_id):
    c = st.read_claim(wi_id)
    if not c:
        raise DriverError(f"{wi_id} has no live claim; run: python3 scripts/wi_driver.py next {wi_id}")
    return c


def get_item(items, wi_id):
    if wi_id not in items:
        raise DriverError(f"unknown work item {wi_id}")
    return items[wi_id]


# ---------------------------------------------------------------- waves
def wave_cmd(root, *args):
    return run([sys.executable, str(WAVE_TOOLS), *args, "--repo-root", str(root)], root)


def lessons_for(st, wi_id, limit=3):
    out = []
    seen = set()
    for lesson in st.lessons():
        if lesson.get("scope") in ("global", wi_id) and lesson["key"] not in seen:
            seen.add(lesson["key"])
            out.append(lesson["text"])
    return out[-limit:]


def make_wave(st, wi, claim, suffix, kind, task, entries, scope, unit="u01", wave_id=None, create=True):
    """Create wave <prefix>-<suffix> (or `wave_id`) and add unit `unit` to it. create=False adds a unit to an existing wave.

    Returns (wave_id, description, prompt_line) or raises DriverError. When the entries bust the budget, trailing entries are dropped."""
    root = st.root
    wave_id = wave_id or f"{claim['wave_prefix']}-{suffix}"
    if create:
        r = wave_cmd(root, "new", wave_id, "--kind", kind)
        if r.returncode != 0:
            raise DriverError(f"wave_tools new {wave_id} failed: {last_lines(r.stderr or r.stdout, 3)}")
    present = [e for e in entries if (root / e.split(":")[0]).is_file()]
    out = None
    for k in range(len(present), 0, -1):
        cmd = ["add", wave_id, "--id", unit, "--task", task, "--entries", *present[:k]]
        if scope:
            cmd += ["--scope", *scope]
        out = wave_cmd(root, *cmd)
        if out.returncode == 0:
            break
    if out is None or out.returncode != 0:
        raise DriverError(f"wave_tools add {wave_id} failed: {last_lines((out.stdout + out.stderr) if out else 'no entries', 4)}")
    desc = next((ln.split(": ", 1)[1] for ln in out.stdout.splitlines() if ln.startswith("description: ")), wave_id)
    prompt = next((ln for ln in out.stdout.splitlines() if ln.startswith("Follow the brief")), "")
    return wave_id, desc, prompt


def spawn_lines(desc, prompt, model="haiku"):
    return [f"SPAWN 1 agent: model={model} subagent_type=general-purpose",
            f"  description: {desc}", f"  prompt: {prompt}"]


DOC_LINES_RE = re.compile(r"(\S+?\.md):lines? ([0-9][0-9, ]*)")


def doc_read_ranges(root, wi, pad=DOC_PAD_LINES, span=DOC_MAX_SPAN):
    """`path:lo-hi` read entries for the plan-doc rows this item cites (Source rows and `lines N, N` on the docs: header)."""
    cited = {}
    for r in wi.source_rows:
        if r.line:
            cited.setdefault(wm.norm_path(r.doc), []).append(r.line)
    for path, nums in DOC_LINES_RE.findall(wi.docs):
        cited.setdefault(wm.norm_path(path), []).extend(int(n) for n in re.findall(r"\d+", nums))
    out = []
    for path, lines in cited.items():
        f = root / path
        if not f.is_file():
            continue
        total = len(f.read_text(encoding="utf-8", errors="replace").splitlines())
        lo = max(1, min(lines) - pad)
        hi = min(total, max(lines) + pad, lo + span)
        if lo <= hi:
            out.append(f"{path}:{lo}-{hi}")
    return out


def worker_wave(st, wi, claim, retry_note=""):
    n = claim.get("attempt", 0) + 1
    claim["attempt"] = n
    kind = "recon" if wi.state == "verify-only" else "worker"
    wi_md = f"{wm.WORK_DIR.as_posix()}/{wi.id}.md"
    files = [f for f in wi.files if (st.root / f).is_file()]
    docs = doc_read_ranges(st.root, wi)[:1]
    entries = ([wi_md] + docs + files)[:wave_budget.MAX_ENTRIES]  # the brief reader takes 3 entries; the cited doc rows outrank extra files
    unread = [f for f in files if f not in entries]
    deferred = ",".join(wi.deferred) or "none"
    if kind == "worker":
        task = (f"Execute the Steps in {entries[0]} for {wi.id}: {wi.title}. Write only these files: {'; '.join(wi.files)}. "
                f"Do not touch deferred rows: {deferred}. Do not install, hot-swap, commit or run the game. "
                f"End by writing your 5-line reply to the reply file.")
        if unread:
            task += f" Write Scope files not in your read list (read only the ranges you need, with rg or sed -n): {'; '.join(unread)}."
        scope = list(wi.files)
    else:
        task = (f"Verify-only: run the Verify steps in {entries[0]} for {wi.id}: {wi.title}. Edit no repo file. "
                f"Write evidence to your OUTPUT file and end by writing your 5-line reply to the reply file.")
        scope = []
    extra = lessons_for(st, wi.id)
    if extra:
        task += " Lessons: " + " | ".join(extra)
    if retry_note:
        task += " RETRY, previous attempt failed: " + retry_note
    wave_id, desc, prompt = make_wave(st, wi, claim, f"a{n}", kind, task, entries, scope)
    claim["wave"], claim["stage"] = wave_id, "worker"
    return claim, spawn_lines(desc, prompt)


# ---------------------------------------------------------------- next
def cmd_next(a, st):
    root = st.root
    session = session_id(a.session)
    with st.flock(".select.lock"):
        items = wm.load_all(root)
        done, parked = st.state_maps()
        claims = st.all_claims()
        stale = {w: c for w, c in claims.items() if st.is_stale(c)}
        live = {w: c["locks"] for w, c in claims.items()}
        out = []
        if a.adopt:
            if not a.wi or a.wi not in claims:
                raise DriverError("--adopt needs WI-NN with an existing claim")
            if a.wi not in stale:
                raise DriverError(f"{a.wi} claim is live (heartbeat under 2 h); wait, or: release {a.wi} --force")
            wi = get_item(items, a.wi)
            claim = claims[a.wi]
            claim["session"] = session
            claim["trace"].append({"ts": ws.now_iso(), "phase": "adopt", "status": "OK", "detail": "stale claim adopted"})
            claim["heartbeat_at"] = ws.now_iso()
            skipped = []
        else:
            dirty = set(ac.git_dirty(root))
            wi, skipped = wm.pick_next(items, done, live, dirty, a.wi, parked)
            if wi is None:
                reasons = [f"  {w}: {r}" for w, r in skipped[:8]]
                print("NOTHING CLAIMABLE" + (f" for {a.wi}" if a.wi else ""))
                print("\n".join(reasons))
                for w, c in stale.items():
                    print(f"  STALE claim {w} (session {c['session']}, last heartbeat {c['heartbeat_at']}): next {w} --adopt | release {w} --force")
                return 3
            claim = None
        header = [f"CLAIM {wi.id} [{wi.prio} {wi.state} size {wi.size}] {wi.title}",
                  f"  write scope: {'; '.join(wi.files) or '(none: verify/out-of-repo)'}"]
        if wi.deferred:
            header.append(f"  deferred rows (do not touch): {','.join(wi.deferred)}")
        if skipped:
            header.append(f"  passed over {len(skipped)} earlier row(s); first: {skipped[0][0]}: {skipped[0][1][:70]}")
        for w, c in stale.items():
            header.append(f"  STALE claim {w} by {c['session']}: next {w} --adopt | release {w} --force")
        if is_game_running():
            header.append("  note: game is running; installs, hot-swaps and play tests stay the user's")
        lessons = lessons_for(st, wi.id)
        header += [f"  LESSON: {t}" for t in lessons]
        if a.dry_run:
            print("DRY RUN (nothing claimed)")
            print("\n".join(header))
            return 0
        if claim is None:
            prefix = wi.id
            k = 1
            while (root / ".claude" / "waves" / f"{prefix}-a1").exists() or (root / ".claude" / "waves" / prefix).exists():
                k += 1
                prefix = f"{wi.id}r{k}"
            base_head = ws.git(root, "rev-parse", "--short", "HEAD").stdout.strip()
            claim = st.create_claim(wi.id, session, wm.lockset(wi), base_head, prefix)
            if claim is None:
                print(f"RACE: {wi.id} was claimed by another session; run next again")
                return 3
            claim["skipped_at_claim"] = [f"{w}: {r[:60]}" for w, r in skipped[:5]]
            base = root / ".claude" / "waves" / prefix / "base.tsv"
            n = ac.write_snapshot(root, base)
            claim["base"] = base.relative_to(root).as_posix()
            claim["base_dirty"] = n
        try:
            claim, spawn = worker_wave(st, wi, claim)
        except DriverError:
            st.release(wi.id)
            raise
        claim["trace"].append({"ts": ws.now_iso(), "phase": "next", "status": "OK", "detail": claim["wave"]})
        st.write_claim(wi.id, claim)
    ledger(root, "WI_CLAIM", wi=wi.id, session=session)
    print("\n".join(header + spawn))
    print(f"THEN: python3 scripts/wi_driver.py verify {wi.id}")
    return 0


# ---------------------------------------------------------------- verify
def expand(token):
    return os.path.expandvars(os.path.expanduser(token))


def run_gate_cmd(root, cmd, tmpdir):
    """Run one gate command. Returns {cmd, status: PASS|FAIL, secs, detail}. SKIP and zero-match count as FAIL."""
    import time
    try:
        argv = [expand(t) for t in shlex.split(cmd)]
    except ValueError as e:
        return {"cmd": cmd, "status": "FAIL", "secs": 0, "detail": f"unparseable gate: {e}"}
    if not argv or argv[0] not in GATE_PROGRAMS:
        return {"cmd": cmd, "status": "FAIL", "secs": 0, "detail": f"gate program not allowed: {argv[:1]}"}
    if any("kpm_hotswap" in t or t == "--apply" for t in argv):
        return {"cmd": cmd, "status": "FAIL", "secs": 0, "detail": "gate may not install or hot-swap"}
    tmpdir.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, TMPDIR=str(tmpdir))
    t0 = time.time()
    try:
        r = run(argv, root, env=env, timeout=GATE_TIMEOUT)
    except subprocess.TimeoutExpired:
        return {"cmd": cmd, "status": "FAIL", "secs": GATE_TIMEOUT, "detail": "timeout"}
    except OSError as e:
        return {"cmd": cmd, "status": "FAIL", "secs": 0, "detail": f"cannot launch: {e}"}
    secs = round(time.time() - t0, 1)
    out = r.stdout + r.stderr
    status, detail = ("PASS", "") if r.returncode == 0 else ("FAIL", f"exit {r.returncode}: {last_lines(out, 6)}")
    if any(p.endswith("run_tests.py") for p in argv):
        m = RUN_TESTS_SUMMARY.search(out)
        if not m:
            status, detail = "FAIL", f"no run_tests summary: {last_lines(out, 4)}"
        elif int(m.group(5)) == 0:
            status, detail = "FAIL", "filter matched no tests (zero items)"
        elif int(m.group(4)) > 0:
            status, detail = "FAIL", f"{m.group(4)} test(s) SKIPPED (skip counts as fail)"
        elif status == "PASS" and (int(m.group(2)) or int(m.group(3))):
            status, detail = "FAIL", last_lines(out, 6)
    elif any(p.endswith("build_check.py") for p in argv):
        m = BUILD_CHECK_SUMMARY.search(out)
        if m and int(m.group(3)) > 0:
            status, detail = "FAIL", f"build_check SKIP={m.group(3)} (skip counts as fail)"
        elif not m and status == "PASS":
            status, detail = "FAIL", "no build_check summary"
    return {"cmd": cmd, "status": status, "secs": secs, "detail": detail}


# wave_tools/agent_contract codes for a changed path that the reply does not claim (see wave_tools.check_unit)
FOREIGN_CODES = ("UNCLAIMED", "OUT_OF_SCOPE")


def problem_path(code, detail):
    """Repo-relative path named by a changed-path problem: '<path> changed since baseline ...' or '<path> outside ...'."""
    return detail.split(" changed since baseline", 1)[0] if code == "UNCLAIMED" else detail.split(" outside ", 1)[0]


def wave_reply_text(root, wave_id):
    """Text of the wave's single unit reply, found through the wave manifest ('' when unreadable)."""
    try:
        manifest = json.loads((root / ".claude" / "waves" / wave_id / "manifest.json").read_text(encoding="utf-8"))
        return (root / manifest["units"][0]["reply"]).read_text(encoding="utf-8", errors="replace")
    except (OSError, ValueError, KeyError, IndexError, TypeError):
        return ""


def filter_foreign(problems, wi, reply_text, kind):
    """Split a wave check's problems [(level, code, detail)] into (kept, foreign paths).

    Other sessions edit the shared worktree while a unit runs. A changed path is FOREIGN when this item does not own
    it: it is outside the item lockset and the reply does not list it (CREATED, MODIFIED or OUTPUT). Foreign problems
    are noise, not worker failures. Everything else is kept: contract errors, NOT_DIRTY, NO_PATH, and any changed path
    that is inside the lockset or listed in the reply."""
    _, fields, _ = ac.parse(reply_text, kind)
    listed = set()
    for key in ("CREATED", "MODIFIED", "OUTPUT"):
        if key in fields:
            listed.update(ac.split_paths(fields[key][0]))
    locks = wm.lockset(wi)
    kept, foreign = [], []
    for level, code, detail in problems:
        if code in FOREIGN_CODES:
            path = problem_path(code, detail)
            if path not in listed and not any(wm.paths_overlap(path, lk) for lk in locks):
                foreign.append(path)
                continue
        kept.append((level, code, detail))
    return kept, sorted(set(foreign))


def wave_check(st, wave_id, wi):
    """The wave's unit check from wave_tools, with foreign changed paths filtered out (see filter_foreign).

    Status: FAIL when a FAIL remains; PARTIAL when an in-lockset or self-listed stray edit remains (WARN); else PASS."""
    r = wave_cmd(st.root, "check", wave_id, "--json")
    try:
        data = json.loads(r.stdout)
    except ValueError:
        return {"status": "ERROR", "verdict": "-", "finding": last_lines(r.stderr or r.stdout, 3), "problems": []}
    u = data["units"][0] if data.get("units") else {"status": "MISSING", "verdict": "-", "finding": "no units", "problems": []}
    if not any(p["code"] in FOREIGN_CODES for p in u.get("problems", [])):
        return u
    probs = [(p["level"], p["code"], p["detail"]) for p in u["problems"]]
    kept, foreign = filter_foreign(probs, wi, wave_reply_text(st.root, wave_id), data.get("kind", "worker"))
    if foreign:
        st.trace(wi.id, "foreign", "NOTE", "not this item's, ignored: " + ", ".join(foreign), paths=foreign)
    fails = [p for p in kept if p[0] == "FAIL"]
    warns = [p for p in kept if p[0] == "WARN"]
    stray = [p for p in warns if p[1] in FOREIGN_CODES]
    if fails:
        status, finding = "FAIL", "; ".join(f"{code} {detail}" for _, code, detail in fails)
    elif warns:
        status, finding = ("PARTIAL" if stray else "PASS"), f"no FAIL; {len(warns)} WARN, first: {warns[0][1]} {warns[0][2]}"
    else:
        status, finding = "PASS", "clean"
    return {**u, "status": status, "finding": " ".join(finding.split()[:25]),
            "problems": [{"level": lvl, "code": code, "detail": detail} for lvl, code, detail in kept]}


def fail_path(st, wi, claim, reason, excerpt, session):
    """One Haiku retry with the failure excerpt, then park."""
    st.trace(wi.id, "fail", "FAIL", f"{reason}: {excerpt}"[:300])
    claim = st.read_claim(wi.id)
    if claim["retries"] < 1:
        claim["retries"] += 1
        claim, spawn = worker_wave(st, wi, claim, retry_note=f"{reason}: {excerpt}"[:600].replace("\n", " "))
        st.write_claim(wi.id, claim)
        print(f"FAIL {wi.id}: {reason}")
        print(excerpt)
        print("RETRY (1 of 1):")
        print("\n".join(spawn))
        print(f"THEN: python3 scripts/wi_driver.py verify {wi.id}")
        return 1
    print(f"FAIL {wi.id}: {reason}\n{excerpt}")
    return park(st, wi, claim, f"{reason}: {excerpt}"[:200], session, rescue=reason.startswith("worker reply FAIL"))


def cmd_verify(a, st):
    root = st.root
    items = wm.load_all(root)
    wi = get_item(items, a.wi)
    claim = need_claim(st, wi.id)
    session = claim["session"]
    if claim["stage"] not in ("worker", "gated"):
        raise DriverError(f"{wi.id} is at stage {claim['stage']}; verify runs after the worker (stage worker)")
    chk = wave_check(st, claim["wave"], wi)
    if chk["status"] == "PASS" and chk.get("verdict") == "FAIL":
        # the reply is well-formed, but the worker itself reported STATUS: FAIL
        chk = {**chk, "status": "FAIL", "finding": "worker STATUS: FAIL"}
    if chk["status"] != "PASS":
        if a.dry_run:
            print(f"DRY RUN: wave check {chk['status']}: {chk['finding']}")
            return 1
        return fail_path(st, wi, claim, f"worker reply {chk['status']}", chk["finding"], session)
    if wi.state == "verify-only" and chk.get("verdict") == "REFUTED":
        pass  # a refuted claim is a result, not a failure; the gate below still runs
    cmds, why = wm.derive_gate(wi, root)
    results = []
    tmpdir = root / ".claude" / "waves" / claim["wave"] / "tmp"
    if a.dry_run:
        print(f"DRY RUN gate ({why}):")
        for c in cmds:
            print(f"  {c}")
        return 0
    if not cmds and not why.startswith("none:"):
        return fail_path(st, wi, claim, "no gate", why, session)
    for c in cmds:
        res = run_gate_cmd(root, c, tmpdir)
        results.append(res)
        st.trace(wi.id, "gate", res["status"], f"{c} :: {res['detail']}"[:300], secs=res["secs"])
        if res["status"] != "PASS":
            return fail_path(st, wi, claim, "gate failed", f"{c}\n{res['detail']}", session)
    claim = st.read_claim(wi.id)
    diff_path = root / ".claude" / "waves" / claim["wave"] / "item.diff"
    paths = [f for f in wi.files if (root / f).exists() or f in ac.git_dirty(root)]
    d = ws.git(root, "diff", "HEAD", "--", *paths, check=False).stdout if paths else ""
    untracked = [p for p, s in ac.git_dirty(root).items() if s == "??" and any(p == f or p.startswith(f.rstrip("/") + "/") for f in wi.files)]
    diff_path.write_text(d + ("\n# new files:\n" + "\n".join(untracked) if untracked else "") or "(no repo diff: verify-only)\n", encoding="utf-8")
    task = (f"Check {wi.id}: {wi.title}. Read the diff and the WI Goal and Steps. Reply VERDICT CONFIRMED if the diff does what the Goal "
            f"says and touches nothing else, REFUTED if it does not, PARTIAL or UNSURE otherwise. Write findings to your OUTPUT file.")
    entries = [f"{wm.WORK_DIR.as_posix()}/{wi.id}.md", diff_path.relative_to(root).as_posix()]
    wave_id, desc, prompt = make_wave(st, wi, claim, f"b{claim.get('attempt', 1)}", "recon", task, entries, [])
    claim.update(stage="gated", checker_wave=wave_id)
    claim["trace"].append({"ts": ws.now_iso(), "phase": "verify", "status": "PASS", "detail": f"{len(results)} gate cmd(s) {why}"})
    st.write_claim(wi.id, claim)
    print(f"GATE PASS {wi.id} ({len(results)} cmd, {why}); worker reply {chk['status']} verdict {chk['verdict']}")
    print("\n".join(spawn_lines(desc, prompt)))
    print(f"THEN: python3 scripts/wi_driver.py docs {wi.id}")
    return 0


# ---------------------------------------------------------------- docs
RANGE_ID_RE = re.compile(r"^([A-Za-z]+)(\d+)-([A-Za-z]+)(\d+)$")


def expand_range_id(plan_id):
    """`HP2-HP7` -> ['HP2'..'HP7'], `T6-T8` -> ['T6','T7','T8']. None when the id is not an ascending same-prefix range."""
    m = RANGE_ID_RE.match(plan_id or "")
    if not m or m.group(1).lower() != m.group(3).lower():
        return None
    lo, hi = int(m.group(2)), int(m.group(4))
    if hi < lo:
        return None
    return [f"{m.group(1)}{n}" for n in range(lo, hi + 1)]


def plan_table_ids(text):
    import plan_manager as pm
    ids, hdr = set(), None
    for line in text.split("\n"):
        if not line.strip().startswith("|"):
            hdr = None
            continue
        cells = [c.strip() for c in line.strip().split("|")[1:-1]]
        if pm.is_separator_row(cells):
            continue
        h = pm.parse_table_header(cells)
        if h:
            hdr = h
            continue
        if hdr is not None and len(cells) > max(hdr["status"], hdr["id"]):
            ids.add(cells[hdr["id"]].strip("*_ ").lower())
    return ids


def cmd_docs(a, st):
    root = st.root
    items = wm.load_all(root)
    wi = get_item(items, a.wi)
    claim = need_claim(st, wi.id)
    if claim["stage"] != "gated":
        raise DriverError(f"{wi.id} is at stage {claim['stage']}; docs runs after verify (stage gated)")
    chk = wave_check(st, claim["checker_wave"], wi)
    claim = st.read_claim(wi.id)  # wave_check may have traced a foreign-path note into the claim
    if chk["status"] in ("MISSING", "FAIL", "ERROR", "PARTIAL"):
        if claim.get("checker_reruns", 0) < 1 and not a.dry_run:
            claim["checker_reruns"] = claim.get("checker_reruns", 0) + 1
            st.write_claim(wi.id, claim)
            st.trace(wi.id, "check", chk["status"], chk["finding"])
            print(f"CHECKER {chk['status']}: {chk['finding']}\nRERUN the checker once (same spawn as printed by verify), then run docs again.")
            return 1
        if not a.dry_run:
            return park(st, wi, claim, f"checker {chk['status']} twice: {chk['finding']}"[:200], claim["session"])
    verdict = chk.get("verdict", "-")
    if verdict != "CONFIRMED" and not a.dry_run:
        # PARTIAL, REFUTED or missing ("-") all stop here; only CONFIRMED lets docs mark the plan rows
        print(f"CHECKER: {verdict} -> not CONFIRMED")
        return fail_path(st, wi, claim, f"checker {verdict}", chk["finding"], claim["session"])
    base = ac.read_snapshot(root / claim["base"]) if claim.get("base") else {}
    marked, unresolved, skipped_dirty = [], [], []
    seen = set()
    with st.flock("docs.flock"):
        for row in wi.source_rows:
            key = (row.doc, row.plan_id)
            if key in seen:
                continue
            seen.add(key)
            tag = f"{row.item}@{row.doc}:{row.plan_id or '-'}"
            if row.item in wi.deferred or row.status != "open":
                unresolved.append(f"{tag} (deferred/{row.status})")
                continue
            docp = root / row.doc
            if row.doc in base:
                skipped_dirty.append(f"{tag} (doc dirty before claim)")
                continue
            if not row.plan_id or not docp.is_file():
                unresolved.append(f"{tag} (no plan id or doc)")
                continue
            table = plan_table_ids(docp.read_text(encoding="utf-8"))
            targets = expand_range_id(row.plan_id)
            if targets is None:
                if row.plan_id.lower() not in table:
                    unresolved.append(f"{tag} (no table row)")
                    continue
                targets = [row.plan_id]
            elif not all(t.lower() in table for t in targets):
                unresolved.append(f"{tag} (range id not in table)")
                continue
            notes = f"{wi.id} verified".replace("|", "/")
            for pid in targets:
                etag = f"{row.item}@{row.doc}:{pid}"
                if a.dry_run:
                    marked.append(etag)
                    continue
                r = run([sys.executable, str(PLAN_MANAGER), "mark-done", pid, "--plan", str(docp), "--state", "DONE",
                         "--commit", "pending", "--notes", notes, "--no-handoff"], root)
                ok = r.returncode == 0 and "Marked" in r.stdout
                if ok:
                    marked.append(etag)
                else:
                    unresolved.append(f"{etag} (mark-done: {last_lines(r.stdout + r.stderr, 1)})")
    lines = [f"DOCS {wi.id}: checker verdict {verdict}; marked {len(marked)} plan row(s), unresolved {len(unresolved)}, skipped-dirty {len(skipped_dirty)}"]
    lines += [f"  unresolved: {u}" for u in (unresolved + skipped_dirty)[:5]]
    spawn = []
    logs = [c for c in wm.mod_changelogs(wi) if (root / c).is_file() and c not in base] if wi.state == "ready" else []
    if logs and not a.dry_run:
        task = (f"Add exactly one bullet under the newest (top) version section of the CHANGELOG describing {wi.id}: {wi.title}. "
                f"Do not change any version number. Edit no other file.")
        wave_id, desc, prompt = make_wave(st, wi, claim, "c1", "worker", task, logs[:1], logs[:1])
        claim["docs_wave"] = wave_id
        spawn = spawn_lines(desc, prompt)
    if a.dry_run:
        print("DRY RUN\n" + "\n".join(lines))
        return 0
    claim = st.read_claim(wi.id)
    claim.update(stage="docs", docs_marked=marked, docs_unresolved=unresolved + skipped_dirty, checker_verdict=verdict,
                 docs_wave=claim.get("docs_wave"))
    claim["trace"].append({"ts": ws.now_iso(), "phase": "docs", "status": "OK", "detail": f"marked {len(marked)} unresolved {len(unresolved)}"})
    st.write_claim(wi.id, claim)
    print("\n".join(lines + (spawn or ["no docs wave"])))
    print(f"THEN: python3 scripts/wi_driver.py close {wi.id}")
    return 0


# ---------------------------------------------------------------- commit helpers
def allowed_path(p, wi):
    pats = wi.files + wm.doc_paths(wi) + wm.mod_changelogs(wi)
    return any(p == f or p.startswith(f.rstrip("/") + "/") or wm.fnmatch.fnmatch(p, f) for f in pats)


def git_commit(st, message, paths):
    """git add untracked among paths, then `git commit -o -F msg -- paths`. Caller holds commit.flock. Returns sha."""
    root = st.root
    status = ws.git(root, "status", "--porcelain", "-uall", "--", *paths, check=False).stdout.splitlines()
    new = [ln[3:] for ln in status if ln.startswith("??")]
    if new:
        ws.git(root, "add", "--", *new)
    msgfile = st.claims / f"commit.{os.getpid()}.msg"
    msgfile.write_text(message, encoding="utf-8")
    try:
        r = ws.git(root, "commit", "-o", "-F", str(msgfile), "--", *paths, check=False)
    finally:
        msgfile.unlink(missing_ok=True)
    if r.returncode != 0:
        raise DriverError(f"git commit failed: {last_lines(r.stdout + r.stderr, 6)}", 2)
    return ws.git(root, "rev-parse", "--short", "HEAD").stdout.strip()


def usage_summary(root, since):
    try:
        r = run([sys.executable, str(SCRIPTS_DIR / "agent_usage.py"), "--since", since, "--json", "--no-desc"], root, timeout=60)
        data = json.loads(r.stdout)
        rows = data.get("agents", [])
        peaks = [x["peak_ctx"] for x in rows if isinstance(x.get("peak_ctx"), int)]
        return {"agents": len(rows), "peak_ctx_max": max(peaks, default=0)}
    except (OSError, ValueError, subprocess.SubprocessError, KeyError):
        return {"agents": 0, "peak_ctx_max": 0}


def run_record(st, wi, claim, outcome, **extra):
    closed = ws.now_iso()
    dur = int((ws.parse_iso(closed) - ws.parse_iso(claim["claimed_at"])).total_seconds())
    tr = claim.get("trace", [])
    return {"wi": wi.id, "outcome": outcome, "session": claim["session"], "claimed_at": claim["claimed_at"], "closed_at": closed,
            "duration_s": dur, "retries": claim.get("retries", 0), "state": wi.state, "size": wi.size, "prio": wi.prio,
            "gates": [{"cmd": t["detail"].split(" :: ")[0][:120], "status": t["status"], "secs": t.get("secs", 0),
                       "detail": t["detail"].split(" :: ", 1)[1][:160] if " :: " in t["detail"] else ""} for t in tr if t["phase"] == "gate"],
            "failures": [{"detail": t["detail"][:200]} for t in tr if t["phase"] in ("fail", "check") and t["status"] != "OK"],
            "foreign_paths": sorted({p for t in tr if t["phase"] == "foreign" for p in t.get("paths", [])}),
            "docs_marked": len(claim.get("docs_marked") or []), "docs_unresolved": len(claim.get("docs_unresolved") or []),
            "skipped_at_claim": claim.get("skipped_at_claim", []), "checker_verdict": claim.get("checker_verdict"),
            "usage": usage_summary(st.root, claim["claimed_at"]), **extra}


# ---------------------------------------------------------------- close
def close_manual(a, st, wi):
    """Close an item whose work was done by hand (docs-only closure after a rescue): commit exactly --paths plus the state files."""
    root = st.root
    if not a.paths or not a.reason:
        raise DriverError("close --manual needs --paths P [P ...] and --reason TEXT")
    done, _ = st.state_maps()
    if wi.id in done:
        raise DriverError(f"{wi.id} is already done")
    live = st.read_claim(wi.id)
    if live and not st.is_stale(live):
        raise DriverError(f"{wi.id} has a live claim (session {live['session']}); release it first")
    dirty = set(ac.git_dirty(root))
    clean = [p for p in a.paths if p not in dirty and not any(d.startswith(p.rstrip('/') + '/') for d in dirty)]
    if clean:
        raise DriverError(f"not changed, nothing to commit: {', '.join(clean[:3])}")
    message = f"{wi.id}: {wi.title} (manual close)\n\nReason: {a.reason[:300]}\nPaths: {len(a.paths)}\n\n{TRAILER}\n"
    if a.dry_run:
        print(f"DRY RUN close --manual {wi.id}: would commit {len(a.paths)} path(s) + state files")
        print("\n".join(f"  {p}" for p in a.paths))
        return 0
    session = session_id()
    with st.flock("commit.flock"):
        state_p, runs_p, index_p = ws.STATE_FILE.as_posix(), ws.RUNS_FILE.as_posix(), ws.INDEX_FILE.as_posix()
        sizes = {p: ((root / p).stat().st_size if (root / p).exists() else None) for p in (state_p, runs_p)}
        index_text = (root / index_p).read_text(encoding="utf-8")
        st.append_event(wi.id, "done", session, f"manual: {a.reason[:120]}")
        now = ws.now_iso()
        st.append_jsonl(ws.RUNS_FILE, {"wi": wi.id, "outcome": "manual-close", "session": session, "claimed_at": now, "closed_at": now,
                                       "duration_s": 0, "reason": a.reason[:200], "paths": len(a.paths)})
        st.render_index()
        try:
            sha = git_commit(st, message, list(a.paths) + [state_p, runs_p, index_p])
        except DriverError:
            for p, sz in sizes.items():
                if sz is None:
                    (root / p).unlink(missing_ok=True)
                else:
                    with open(root / p, "r+b") as fh:
                        fh.truncate(sz)
            (root / index_p).write_text(index_text, encoding="utf-8")
            raise
    if live:
        st.release(wi.id)
    ledger(root, "WI_DONE", wi=wi.id, sha=sha, manual=1)
    print(f"COMMITTED {wi.id} {sha} (manual): {len(a.paths)} path(s) + state")
    return 0


def cmd_close(a, st):
    root = st.root
    items = wm.load_all(root)
    wi = get_item(items, a.wi)
    if a.manual:
        return close_manual(a, st, wi)
    claim = need_claim(st, wi.id)
    if claim["stage"] != "docs":
        raise DriverError(f"{wi.id} is at stage {claim['stage']}; close runs after docs (stage docs)")
    if claim.get("docs_wave"):
        chk = wave_check(st, claim["docs_wave"], wi)
        claim = st.read_claim(wi.id)  # wave_check may have traced a foreign-path note into the claim
        if chk["status"] != "PASS":
            st.trace(wi.id, "check", chk["status"], f"docs wave: {chk['finding']}")
            print(f"DOCS WORKER {chk['status']}: {chk['finding']}\nFix: rerun the docs spawn once, or skip it with: release the CHANGELOG by reverting it, then close.")
            return 1
    base = ac.read_snapshot(root / claim["base"])
    changed, now = ac.changed_since(root, base)
    item_set = sorted(p for p in changed if allowed_path(p, wi))
    stray = sorted(p for p in changed if not allowed_path(p, wi))
    pre_dirty = [p for p in item_set if p in base]
    if pre_dirty:
        st.trace(wi.id, "close", "BLOCKED", f"pre-dirty in-scope: {pre_dirty[:3]}")
        print(f"HARD STOP {wi.id}: in-scope path(s) were already dirty before the claim, committing would sweep another session's edits:")
        print("\n".join(f"  {p}" for p in pre_dirty[:5]))
        print(f"Fix: ask the user; then `park {wi.id} --reason ...` or `release {wi.id} --force`.")
        return 2
    msg_lines = [f"{wi.id}: {wi.title}", ""]
    rows = claim.get("docs_marked") or []
    unres = claim.get("docs_unresolved") or []
    msg_lines += [f"Plan rows closed: {len(rows)}" + (" (" + ", ".join(r.split("@")[0] for r in rows[:6]) + ")" if rows else ""),
                  f"Plan rows unresolved: {len(unres)}"]
    msg_lines += [f"  - {u}" for u in unres[:3]]
    if len(unres) > 3:
        msg_lines.append(f"  (+{len(unres) - 3} more)")
    msg_lines += [f"Gate: {sum(1 for t in claim['trace'] if t['phase'] == 'gate' and t['status'] == 'PASS')} command(s) PASS; "
                  f"checker verdict {claim.get('checker_verdict')}; retries {claim['retries']}", "", TRAILER, ""]
    message = "\n".join(msg_lines)
    if a.dry_run:
        print(f"DRY RUN close {wi.id}: would commit {len(item_set)} path(s)")
        print("\n".join(f"  {p}" for p in item_set[:12]))
        print(f"stray changed paths (left alone): {len(stray)}")
        print(message)
        return 0
    with st.flock("commit.flock"):
        state_p, runs_p, index_p = ws.STATE_FILE.as_posix(), ws.RUNS_FILE.as_posix(), ws.INDEX_FILE.as_posix()
        sizes = {p: ((root / p).stat().st_size if (root / p).exists() else None) for p in (state_p, runs_p)}
        index_text = (root / index_p).read_text(encoding="utf-8")
        st.append_event(wi.id, "done", claim["session"])
        rec = run_record(st, wi, claim, "done", files=len(item_set), stray=len(stray))
        st.append_jsonl(ws.RUNS_FILE, rec)
        st.render_index()
        try:
            sha = git_commit(st, message, item_set + [state_p, runs_p, index_p])
        except DriverError:
            for p, sz in sizes.items():
                if sz is None:
                    (root / p).unlink(missing_ok=True)
                else:
                    with open(root / p, "r+b") as fh:
                        fh.truncate(sz)
            (root / index_p).write_text(index_text, encoding="utf-8")
            raise
        left = ws.git(root, "status", "--porcelain", "--", *item_set, check=False).stdout.strip()
    dirty_now = len(ac.git_dirty(root))
    (st.claims / f"{wi.id}.closed.json").write_text(json.dumps({"sha": sha, "claimed_at": claim["claimed_at"], "paths": len(item_set),
                                                                 "foreign_dirty": dirty_now, "left_dirty": left, "stray": stray[:10],
                                                                 "unresolved": claim.get("docs_unresolved", [])[:5]}), encoding="utf-8")
    st.release(wi.id)
    ledger(root, "WI_DONE", wi=wi.id, sha=sha)
    print(f"COMMITTED {wi.id} {sha}: {len(item_set)} item path(s) + state; item-scoped dirty after commit: {'0' if not left else left.splitlines()[0]}")
    if stray:
        print(f"WARNING {len(stray)} changed path(s) outside this item's scope left untouched, first: {stray[0]}")
    print(f"THEN: python3 scripts/wi_driver.py report {wi.id}")
    return 0 if not left else 2


# ---------------------------------------------------------------- report
def cmd_report(a, st):
    root = st.root
    closed = st.claims / f"{a.wi}.closed.json"
    info = json.loads(closed.read_text(encoding="utf-8")) if closed.is_file() else None
    sha = info["sha"] if info else ws.git(root, "log", "-1", "--format=%h", f"--grep=^{a.wi}:", check=False).stdout.strip()
    if not sha:
        raise DriverError(f"no commit found for {a.wi}")
    print("### Saved State")
    print(f"- Commit: {sha} ({ws.git(root, 'log', '-1', '--format=%s', sha).stdout.strip()})")
    print(f"- Item-scoped dirty paths: {'0' if not info or not info['left_dirty'] else info['left_dirty']}; other sessions' dirty paths left untouched: {info['foreign_dirty'] if info else 'unknown'}")
    if info and info["unresolved"]:
        print(f"- Plan rows not closed (no plan id/table row): {len(info['unresolved'])}, first: {info['unresolved'][0]}")
    print(f"- Handoff: N/A (state in {ws.STATE_FILE.as_posix()}, run telemetry in {ws.RUNS_FILE.as_posix()})")
    print("- Safe to /clear (or /exit)")
    print("- Next Resume Command: /next-dev-work-item")
    n_done = sum(1 for r in st.runs() if r.get("outcome") == "done")
    if n_done and n_done % 5 == 0:
        print(f"AUDIT DUE: {n_done} items finished; run: python3 scripts/wi_driver.py audit")
    if info:
        r = run([sys.executable, str(SCRIPTS_DIR / "agent_usage.py"), "--since", info["claimed_at"], "--no-desc"], root, timeout=60)
        print("### Agents Used (agent_usage.py)")
        print(last_lines(r.stdout or r.stderr, 14))
    return 0


# ---------------------------------------------------------------- park / release / status
def park(st, wi, claim, reason, session, commit=True, rescue=False):
    """Record a park event, release the claim, commit state files. Partial edits stay in the tree and are reported."""
    root = st.root
    partial = []
    if claim and claim.get("base"):
        base = ac.read_snapshot(root / claim["base"])
        changed, _ = ac.changed_since(root, base)
        partial = sorted(p for p in changed if allowed_path(p, wi))
    with st.flock("commit.flock"):
        state_p, runs_p, index_p = ws.STATE_FILE.as_posix(), ws.RUNS_FILE.as_posix(), ws.INDEX_FILE.as_posix()
        sizes = {p: ((root / p).stat().st_size if (root / p).exists() else None) for p in (state_p, runs_p)}
        index_text = (root / index_p).read_text(encoding="utf-8")
        st.append_event(wi.id, "park", session, reason)
        if claim:
            st.append_jsonl(ws.RUNS_FILE, run_record(st, wi, claim, "park", reason=reason[:200], partial_paths=len(partial)))
        st.render_index()
        if commit:
            try:
                sha = git_commit(st, f"{wi.id}: parked\n\nReason: {reason[:300]}\n\n{TRAILER}\n", [state_p, runs_p, index_p])
            except DriverError:
                for p, sz in sizes.items():
                    if sz is None:
                        (root / p).unlink(missing_ok=True)
                    else:
                        with open(root / p, "r+b") as fh:
                            fh.truncate(sz)
                (root / index_p).write_text(index_text, encoding="utf-8")
                st.release(wi.id)
                raise
        else:
            sha = "-"
    st.release(wi.id)
    ledger(root, "WI_PARK", wi=wi.id, reason=reason[:80])
    print(f"PARKED {wi.id} ({sha}): {reason[:160]}")
    if partial:
        print(f"Partial edits left uncommitted in the working tree ({len(partial)}): {', '.join(partial[:4])}. Review or revert them by hand; the driver never reverts.")
    if rescue:
        print(f"THEN: python3 scripts/wi_driver.py rescue {wi.id}")
    else:
        print("Safe to /clear (or /exit). Next Resume Command: /next-dev-work-item")
    return 2


def cmd_park(a, st):
    items = wm.load_all(st.root)
    wi = get_item(items, a.wi)
    claim = st.read_claim(wi.id)
    if a.dry_run:
        print(f"DRY RUN: would park {wi.id} ({a.reason}) and release its claim")
        return 0
    return park(st, wi, claim, a.reason, claim["session"] if claim else session_id())


# ---------------------------------------------------------------- rescue
def sibling_items(items, wi):
    """Other items whose lockset overlaps this one's, or that order against it with `after:`."""
    mine = wm.lockset(wi)
    out = []
    for w in items.values():
        if w.id == wi.id:
            continue
        if wm.locksets_conflict(mine, wm.lockset(w)) or wi.id in w.after or w.id in wi.after:
            out.append(w)
    return out


def rescue_units(st, wi, items):
    """The recon units of the rescue swarm: [(key, title, task, entries)], at most RESCUE_UNITS, generic over any work item."""
    root = st.root
    wi_md = f"{wm.WORK_DIR.as_posix()}/{wi.id}.md"
    files = [f for f in wi.files if (root / f).is_file()]
    docs = doc_read_ranges(root, wi)[:1]
    rows = ", ".join(f"{r.item} {r.doc}:{r.line}" for r in wi.source_rows) or "none listed"
    flist = "; ".join(wi.files) or "none"
    tail = " Recon only: edit no repo file. Write your findings (at most 60 lines, anchors as path:line) to your OUTPUT file, then end with the 5-line recon reply."
    attempts = sorted((root / ".claude" / "waves").glob(f"{wi.id}-a*/u01.brief.md"))
    gap = [attempts[-1].relative_to(root).as_posix()] if attempts else []
    gap += [p.replace("brief.md", "reply.txt") for p in gap if (Path(root) / p.replace("brief.md", "reply.txt")).is_file()]
    sibs = sibling_items(items, wi)
    sib_md = [f"{wm.WORK_DIR.as_posix()}/{w.id}.md" for w in sibs][:2]
    ctx = f"{wi.id} ({wi.title}) was parked after its Haiku worker replied FAIL twice. Find out why and what would make it finishable."
    return [
        ("spec", "spec in cited doc rows", f"{ctx} Spec check: for each source row ({rows}) state what the cited plan doc really requires, "
         f"whether it is in scope for the current version (watch words like v2, possible, later, if), and where the Steps in {wi_md} disagree with the doc." + tail,
         [wi_md] + docs),
        ("code", "code flow at the files", f"{ctx} Code flow: in the Write Scope files ({flist}) locate the functions, hooks and call sites the Steps name. "
         f"Give path:line anchors, the current state of each row (present, absent, partial) and the exact place an edit would go. Use rg and sed -n ranges." + tail,
         [wi_md] + files[:1]),
        ("data", "data and table sources", f"{ctx} Data sources: for the files ({flist}) find generators (scripts/gen_*), tables (*.inc, 2DA, toml) and runtime readers that feed them. "
         f"Which fields exist today, which the Steps need, and who owns each. Use rg." + tail,
         [wi_md] + files[:1]),
        ("tests", "tests and gates", f"{ctx} Tests and gates: find colocated tests (*_test.cpp, test_*.py) for the files ({flist}), the scoped run_tests.py filter and any build_check.py "
         f"arguments, and whether the Verify commands in {wi_md} work as written. Do not run builds, installs or the game." + tail,
         [wi_md]),
        ("rows", "plan-row closing mechanics", f"{ctx} Row closing: do the source rows ({rows}) have plan ids and table rows that plan_manager.py mark-done can mark "
         f"(python3 scripts/plan_manager.py mark-done --help), or must they be edited by hand? Name the smallest safe edit and the Write Scope it needs." + tail,
         [wi_md] + docs),
        ("siblings", "sibling item conflicts", f"{ctx} Sibling items: {', '.join(w.id for w in sibs) or 'none found'} share files or order against this item. "
         f"Which of their rows or edits overlap this item's, what order is safe, and what each needs from this item." + tail,
         [wi_md] + sib_md),
        ("gap", "brief gap analysis", f"{ctx} Brief gap: read the failed attempt's brief and reply. List every contradiction between the Steps and the brief "
         f"(steps needing files outside the Write Scope, missing read entries, unreachable rows) and the cheapest brief that would work." + tail,
         [wi_md] + gap),
    ][:RESCUE_UNITS]


def rescue_wave_id(wi):
    return f"{wi.id}-rescue"


def cmd_rescue(a, st):
    root = st.root
    items = wm.load_all(root)
    wi = get_item(items, a.wi)
    wave_id = rescue_wave_id(wi)
    wdir = root / ".claude" / "waves" / wave_id
    if a.collect:
        return rescue_collect(st, wi, wave_id, wdir)
    done, parked = st.state_maps()
    if wi.id not in parked:
        raise DriverError(f"{wi.id} is not parked; rescue only runs on a parked item")
    if not parked[wi.id].startswith("worker reply FAIL"):
        raise DriverError(f"{wi.id} park reason is not a worker FAIL ({parked[wi.id][:60]}); rescue handles worker-FAIL parks only")
    if any(e["wi"] == wi.id and e["ev"] == "rescue" for e in st.events()):
        raise DriverError(f"{wi.id} was already rescued once; collect with: rescue {wi.id} --collect")
    units = rescue_units(st, wi, items)
    if a.dry_run:
        print(f"DRY RUN rescue {wi.id}: {len(units)} recon unit(s)")
        for i, (key, title, task, entries) in enumerate(units, 1):
            present = [e for e in entries if (root / e.split(":")[0]).is_file()]
            res = wave_budget.check([wave_tools.resolve_entry(root, e) for e in present], brief_bytes=len(task.encode("utf-8")) + 1500)
            print(f"  u{i:02d} {key}: {len(present)} entr{'y' if len(present) == 1 else 'ies'}, est {res['total']} tokens, {'OK' if res['ok'] else 'OVER BUDGET'}")
        return 0
    if wdir.exists():
        raise DriverError(f"{wdir.relative_to(root).as_posix()} already exists but no rescue was recorded; remove that directory and rerun")
    claim = {"wave_prefix": wi.id}
    spawns = []
    for i, (key, title, task, entries) in enumerate(units, 1):
        _, desc, prompt = make_wave(st, wi, claim, "rescue", "recon", task, entries, [], unit=f"u{i:02d}", wave_id=wave_id, create=(i == 1))
        spawns.append(spawn_lines(desc, prompt))
    with st.flock("commit.flock"):
        state_p = ws.STATE_FILE.as_posix()
        st.append_event(wi.id, "rescue", session_id(), f"{len(units)} recon briefs in {wave_id}")
        git_commit(st, f"{wi.id}: rescue recon briefs written\n\n{TRAILER}\n", [state_p])
    ledger(root, "WI_RESCUE", wi=wi.id, units=len(units))
    print(f"RESCUE {wi.id}: {len(units)} recon briefs in .claude/waves/{wave_id}/ (outputs uNN.md next to each brief)")
    print(f"Spawn all {len(spawns)} in ONE message, model haiku, exactly as printed:")
    for lines in spawns:
        print("\n".join(lines))
    print(f"THEN: python3 scripts/wi_driver.py rescue {wi.id} --collect")
    return 0


RESCUE_READ_ORDER = ("gap", "spec", "code", "data", "tests", "rows", "siblings")


def rescue_collect(st, wi, wave_id, wdir):
    root = st.root
    if not (wdir / "manifest.json").is_file():
        raise DriverError(f"no rescue wave for {wi.id}; run: python3 scripts/wi_driver.py rescue {wi.id}")
    manifest = json.loads((wdir / "manifest.json").read_text(encoding="utf-8"))
    r = wave_cmd(root, "check", wave_id, "--json")
    try:
        results = {u["id"]: u for u in json.loads(r.stdout)["units"]}
    except (ValueError, KeyError):
        raise DriverError(f"wave_tools check {wave_id} failed: {last_lines(r.stderr or r.stdout, 3)}")
    keys = ["spec", "code", "data", "tests", "rows", "siblings", "gap"]
    bad = []
    print(f"COLLECT {wi.id}: {len(results)} recon unit(s)")
    for u in manifest["units"]:
        res = results.get(u["id"], {"status": "MISSING", "verdict": "-", "finding": "no result"})
        out_ok = (root / u["out"]).is_file() and (root / u["out"]).stat().st_size > 0
        flag = "" if res["status"] == "PASS" and out_ok else "  <-- RERUN"
        if flag:
            bad.append(u)
        key = keys[int(u["id"][1:]) - 1] if int(u["id"][1:]) <= len(keys) else "?"
        print(f"  {u['id']} {key:9} {res['status']:7} {res['verdict']:10} {u['out']}{'' if out_ok else ' (OUTPUT missing)'}{flag}")
    if bad:
        print(f"{len(bad)} unit(s) missing or not a clean 5-line reply. Respawn only these, same prompts:")
        for u in bad:
            print("\n".join(spawn_lines(u["description"], f"Follow the brief at {root / u['brief']} exactly. Final message: the 5-line schema only.")))
        print(f"THEN: python3 scripts/wi_driver.py rescue {wi.id} --collect")
        return 1
    order = [u for k in RESCUE_READ_ORDER for u in manifest["units"] if keys[int(u["id"][1:]) - 1] == k]
    print("Read ONLY these OUTPUT files, in this order: " + ", ".join(u["out"] for u in order))
    print(f"THEN: /plan-handoff {wi.id}  (enter plan mode; the plan picks ONE outcome: split into sub-WIs via backlog add, docs-only closure, "
          f"defer to an existing WI, or a code plan with Haiku-sized steps of at most 3 files each; edit nothing before approval)")
    return 0


def unpark(st, wi, reason, session):
    """Record an unpark event, re-render the index (clears the [!] cell), commit state files. Refuses unless WI is parked."""
    root = st.root
    with st.flock("commit.flock"):
        state_p, index_p = ws.STATE_FILE.as_posix(), ws.INDEX_FILE.as_posix()
        if wi.id not in st.state_maps()[1]:
            raise DriverError(f"{wi.id} is not parked; nothing to unpark", 2)
        size = (root / state_p).stat().st_size if (root / state_p).exists() else None
        index_text = (root / index_p).read_text(encoding="utf-8")
        st.append_event(wi.id, "unpark", session, reason)
        st.render_index()
        try:
            sha = git_commit(st, f"{wi.id}: unparked\n\nReason: {reason[:300]}\n\n{TRAILER}\n", [state_p, index_p])
        except DriverError:
            if size is None:
                (root / state_p).unlink(missing_ok=True)
            else:
                with open(root / state_p, "r+b") as fh:
                    fh.truncate(size)
            (root / index_p).write_text(index_text, encoding="utf-8")
            raise
    ledger(root, "WI_UNPARK", wi=wi.id, reason=reason[:80])
    print(f"UNPARKED {wi.id} ({sha}): {reason[:160]}")
    print(f"Claimable again: python3 scripts/wi_driver.py next {wi.id} (or /next-dev-work-item)")
    return 0


def cmd_unpark(a, st):
    items = wm.load_all(st.root)
    wi = get_item(items, a.wi)
    if wi.id not in st.state_maps()[1]:
        raise DriverError(f"{wi.id} is not parked; nothing to unpark", 2)
    if a.dry_run:
        print(f"DRY RUN: would unpark {wi.id} ({a.reason}) and clear its [!] cell")
        return 0
    return unpark(st, wi, a.reason, session_id())


def cmd_release(a, st):
    claim = st.read_claim(a.wi)
    if not claim:
        print(f"{a.wi} has no claim")
        return 0
    if not st.is_stale(claim) and not a.force:
        raise DriverError(f"{a.wi} claim is live (heartbeat {claim['heartbeat_at']}); use --force to free it")
    st.release(a.wi)
    print(f"released {a.wi} (was session {claim['session']}); its edits, if any, stay in the working tree")
    return 0


def cmd_status(a, st):
    items = wm.load_all(st.root)
    done, parked = st.state_maps()
    claims = st.all_claims()
    print(f"done {len(done)}  parked {len(parked)}  claimed {len(claims)}  total {len(items)}")
    for w, c in claims.items():
        age = int((ws.parse_iso(ws.now_iso()) - ws.parse_iso(c["claimed_at"])).total_seconds() // 60)
        print(f"  {w} session {c['session'][:12]} stage {c['stage']} age {age}m{' STALE' if st.is_stale(c) else ''} lock {c['locks'][0] if c['locks'] else '-'}")
    live = {w: c["locks"] for w, c in claims.items()}
    dirty = set(ac.git_dirty(st.root))
    shown = 0
    print("claimable now:")
    for wi in items.values():
        if wm.eligibility(wi, items, done, live, dirty, parked).ok:
            print(f"  {wi.id} {wi.title[:60]}")
            shown += 1
            if shown >= 5:
                break
    if not shown:
        print("  (none)")
    return 0


def cmd_lint(a, st):
    items = wm.load_all(st.root)
    probs = wm.lint(items, st.root)
    for w in items.values():
        for f in (w.files if w.runnable else []):
            if f not in w.new_files and ws.git(st.root, "check-ignore", "-q", f, check=False).returncode == 0:
                probs.append(("ERROR", w.id, f"write file is gitignored: {f}"))
        for g in w.verify:
            try:
                first = expand(shlex.split(g)[0])
            except (ValueError, IndexError):
                first = ""
            if first not in GATE_PROGRAMS:
                probs.append(("ERROR", w.id, f"verify program not allowed: {g[:60]}"))
    errs = [p for p in probs if p[0] == "ERROR"]
    for lvl, w, msg in probs[:30]:
        print(f"{lvl} {w} {msg}")
    runnable = sum(1 for w in items.values() if w.runnable)
    print(f"VERDICT: {'FAIL' if errs else 'PASS'} wis={len(items)} runnable={runnable} errors={len(errs)} warns={len(probs) - len(errs)}")
    return 1 if errs else 0


# ---------------------------------------------------------------- audit
def analyze(runs, events, claims_stale, existing_keys):
    """Pure retrospective analysis. Returns findings [{key, severity, text, lesson, scope, hold, backlog}]; keys already
    recorded as lessons are dropped so audit is idempotent."""
    f = []

    def add(key, severity, text, lesson=None, scope="global", hold=None, backlog=None):
        if key not in existing_keys:
            f.append({"key": key, "severity": severity, "text": text, "lesson": lesson, "scope": scope, "hold": hold, "backlog": backlog})

    parks = {}
    for e in events:
        if e["ev"] == "park":
            parks.setdefault(e["wi"], []).append(e.get("reason", ""))
    for w, rs in parks.items():
        if len(rs) >= 2:
            add(f"repeat-park:{w}", "high", f"{w} parked {len(rs)} times; last: {rs[-1][:80]}",
                lesson=f"{w} has been parked {len(rs)} times; do not claim it until its scope or verify line is fixed.", scope=w,
                hold=(w, rs[-1][:120]))
    for r in runs:
        for g in r.get("gates", []):
            if "matched no tests" in g.get("detail", "") or "SKIPPED" in g.get("detail", ""):
                add(f"gate-selects-nothing:{r['wi']}", "high",
                    f"{r['wi']}: gate `{g['cmd'][:60]}` {g['detail'][:60]}",
                    lesson=f"{r['wi']}: the verify line selects no runnable tests; fix the filter before trusting a pass.", scope=r["wi"],
                    backlog=dict(title=f"Repair verify line for {r['wi']} (gate selects no tests)", area="work-items",
                                 why=f"{r['wi']} gate `{g['cmd'][:50]}` ran zero tests", files=[]))
    total = [r for r in runs]
    n = len(total)
    miss = sum(1 for r in total for x in r.get("failures", []) if "MISSING" in x["detail"] or "reply missing" in x["detail"])
    if miss >= 2:
        add("missing-replies", "med", f"{miss} worker/checker replies were missing",
            lesson="Workers skip the reply file when they run long: the last action must be writing the 5-line reply file named in the brief.")
    scope_hits = sum(1 for r in total for x in r.get("failures", []) if "OUT_OF_SCOPE" in x["detail"])
    if scope_hits >= 2:
        add("out-of-scope", "med", f"{scope_hits} out-of-scope edits flagged",
            lesson="Edit only the Write Scope files; log any other needed change under OPEN instead of editing it.")
    retried = sum(1 for r in total if r.get("retries"))
    if n >= 5 and retried / n > 0.4:
        add("high-retry-rate", "med", f"{retried}/{n} items needed a retry",
            lesson="Many items need a retry: the Steps are likely too vague; the worker prompt should quote the first Step's exact file:line.")
    unres = [r for r in total if r.get("docs_unresolved", 0) > 0]
    if len(unres) >= 3:
        add("plan-row-hygiene", "low", f"{len(unres)} items closed with unresolved plan rows",
            backlog=dict(title="Add plan ids to Source rows that have none", area="work-items",
                         why=f"{len(unres)} closed items left plan rows unresolved", files=[]))
    peaks = [r.get("usage", {}).get("peak_ctx_max", 0) for r in total]
    if peaks and max(peaks) > 60000:
        add("token-overrun", "med", f"worker peak context reached {max(peaks)} tokens (cap 60000)",
            lesson="A worker exceeded the 60k budget: split the item, or read only the listed line ranges.")
    closed_partial = [r for r in total if r.get("outcome") == "done" and r.get("checker_verdict") != "CONFIRMED"]
    if closed_partial:
        add("closed-on-partial", "med",
            f"{len(closed_partial)} closed run(s) had checker verdict != CONFIRMED: {', '.join(r['wi'] for r in closed_partial)}",
            lesson="Do not close on a PARTIAL checker verdict; park or retry.")
    fail_closed = []
    for r in total:
        details = [x["detail"] for x in r.get("failures", [])]
        if r.get("outcome") != "done":
            continue
        if any("STATUS: FAIL" in d for d in details) or (r.get("retries") and any("REFUTED" in d or "PARTIAL" in d for d in details)):
            fail_closed.append(r)
    if fail_closed:
        add("worker-fail-closed", "med",
            f"{len(fail_closed)} run(s) needed a retry after worker/checker failure: {', '.join(r['wi'] for r in fail_closed)}",
            lesson="Fix the Steps or verify line after a worker/checker failure before retrying; do not re-run unchanged.")
    errs = [x["detail"] for r in total for x in r.get("failures", []) if "Traceback" in x["detail"] or "CONTRACT_ERROR" in x["detail"]]
    if errs:
        add("driver-errors", "high", f"{len(errs)} driver/contract error(s), first: {errs[0][:80]}",
            backlog=dict(title="wi_driver: fix contract/driver errors seen in runs", area="work-items",
                         why=f"{errs[0][:60]}", files=["scripts/wi_driver.py"]))
    durs = [r["duration_s"] for r in total if r.get("outcome") == "done"]
    if len(durs) >= 5:
        med = statistics.median(durs)
        slow = [r["wi"] for r in total if r.get("outcome") == "done" and r["duration_s"] > 3 * med]
        if slow:
            add("slow-items", "low", f"{', '.join(slow[:4])} took over 3x the median ({int(med)}s)")
    for w in claims_stale:
        add(f"stale-claim:{w}", "med", f"claim on {w} is stale: next {w} --adopt, or release {w} --force")
    return f


def cmd_audit(a, st):
    root = st.root
    runs, events = st.runs(), st.events()
    stale = [w for w, c in st.all_claims().items() if st.is_stale(c)]
    keys = {x["key"] for x in st.lessons()}
    findings = analyze(runs, events, stale, keys)
    done = sum(1 for r in runs if r["outcome"] == "done")
    parked = sum(1 for r in runs if r["outcome"] == "park")
    print(f"AUDIT runs={len(runs)} done={done} parked={parked} findings={len(findings)} (already-recorded lessons skipped: {len(keys)})")
    for x in findings:
        acts = [k for k in ("lesson", "hold", "backlog") if x[k]]
        print(f"  [{x['severity']}] {x['key']}: {x['text']}  -> {','.join(acts) or 'info'}")
    if not findings or not (a.apply or a.dry_run):
        if findings:
            print("Apply the corrections with: python3 scripts/wi_driver.py audit --apply")
        return 0
    if a.dry_run:
        print("DRY RUN: --apply would write lessons, hold items and add backlog entries above")
        return 0
    items = wm.load_all(root)
    paths = []
    with st.flock("commit.flock"):
        for x in findings:
            if x["lesson"]:
                st.append_jsonl(ws.LESSONS_FILE, {"key": x["key"], "scope": x["scope"], "text": x["lesson"], "ts": ws.now_iso()})
            else:
                st.append_jsonl(ws.LESSONS_FILE, {"key": x["key"], "scope": "audit", "text": x["text"], "ts": ws.now_iso()})
            if x["hold"] and x["hold"][0] in items:
                wid, why = x["hold"]
                st.set_state(wid, "hold")
                st.prepend_blocker(wid, f"[audit] parked repeatedly: {why}.")
                paths.append((root / wm.WORK_DIR / f"{wid}.md").relative_to(root).as_posix())
            if x["backlog"]:
                b = x["backlog"]
                wid = st.add_backlog_item(items, b["title"] + f" [audit:{x['key']}]", b["area"], b["why"], b["files"], origin="audit")
                items = wm.load_all(root)
                paths.append(f"{wm.WORK_DIR.as_posix()}/{wid}.md")
                print(f"  backlog +{wid}: {b['title']}")
        paths += [ws.LESSONS_FILE.as_posix(), ws.INDEX_FILE.as_posix()]
        sha = git_commit(st, f"audit: {len(findings)} finding(s) recorded\n\n{TRAILER}\n", sorted(set(paths)))
    print(f"APPLIED audit corrections in commit {sha}")
    return 0


# ---------------------------------------------------------------- backlog
def cmd_backlog(a, st):
    root = st.root
    items = wm.load_all(root)
    if a.bcmd == "list":
        rows = [w for w in items.values() if "Backlog item" in w.blockers or w.rank > 100]
        for w in rows[:30]:
            print(f"{w.id} [{w.state}] {w.title[:80]}")
        print(f"{len(rows)} backlog item(s)")
        return 0
    if a.bcmd == "add":
        files = [f.strip() for f in (a.files or "").split(";") if f.strip()]
        if a.dry_run:
            print(f"DRY RUN: would add backlog item {st.next_wi_id(items)}: {a.title} ({a.state})")
            return 0
        with st.flock("commit.flock"):
            wid = st.add_backlog_item(items, a.title, a.area, a.why, files, origin=a.origin, state=a.state)
            sha = git_commit(st, f"backlog: add {wid} {a.title[:60]}\n\n{TRAILER}\n",
                             [f"{wm.WORK_DIR.as_posix()}/{wid}.md", ws.INDEX_FILE.as_posix()])
        print(f"added {wid} [{a.state}] ({sha}); promote with: backlog promote {wid} --state ready")
        return 0
    if a.bcmd == "promote":
        get_item(items, a.wi)
        if a.dry_run:
            print(f"DRY RUN: would set {a.wi} to {a.state}")
            return 0
        with st.flock("commit.flock"):
            st.set_state(a.wi, a.state)
            sha = git_commit(st, f"backlog: {a.wi} -> {a.state}\n\n{TRAILER}\n", [f"{wm.WORK_DIR.as_posix()}/{a.wi}.md", ws.INDEX_FILE.as_posix()])
        print(f"{a.wi} is now {a.state} ({sha})")
        return 0
    return 1


# ---------------------------------------------------------------- main
def build_parser():
    ap = argparse.ArgumentParser(prog="wi_driver.py", description="Claim, run, verify, commit and audit one dev work item per call.")
    ap.add_argument("--repo-root", default=None, help="repo root (default: this checkout)")
    ap.add_argument("--claims-dir", default=None, help="claims directory override (tests)")
    sub = ap.add_subparsers(dest="cmd", required=True, metavar="{next,verify,docs,close,report,park,rescue,unpark,release,status,lint,audit,backlog}")
    p = sub.add_parser("next", help="claim the next eligible item and print the worker spawn")
    p.add_argument("wi", nargs="?")
    p.add_argument("--adopt", action="store_true", help="take over a stale claim on WI-NN")
    p.add_argument("--dry-run", action="store_true", help="show the pick, claim nothing")
    p.add_argument("--session", default=None, help="session id (default $WI_SESSION, $CLAUDE_CODE_SESSION_ID, parent pid)")
    for name, h in (("verify", "run the gate, print the checker spawn"), ("docs", "mark plan rows, maybe print a docs spawn"),
                    ("close", "commit this item's paths only"), ("report", "print the Saved State block")):
        p = sub.add_parser(name, help=h)
        p.add_argument("wi")
        if name != "report":
            p.add_argument("--dry-run", action="store_true", help="show what would happen")
        if name == "close":
            p.add_argument("--manual", action="store_true", help="no claim needed: commit --paths by hand after a rescue, mark the item done")
            p.add_argument("--paths", nargs="+", default=[], metavar="PATH", help="with --manual: the exact repo paths to commit")
            p.add_argument("--reason", default="", help="with --manual: why the item is closed")
    p = sub.add_parser("park", help="record a park, release the claim")
    p.add_argument("wi")
    p.add_argument("--reason", required=True)
    p.add_argument("--dry-run", action="store_true")
    p = sub.add_parser("rescue", help="after a worker-FAIL park: write the recon swarm briefs (once), or --collect their outputs")
    p.add_argument("wi")
    p.add_argument("--collect", action="store_true", help="check the swarm replies and print the plan reading order")
    p.add_argument("--dry-run", action="store_true")
    p = sub.add_parser("unpark", help="reverse a park: clear the [!] cell so the item is claimable again")
    p.add_argument("wi")
    p.add_argument("--reason", required=True)
    p.add_argument("--dry-run", action="store_true")
    p = sub.add_parser("release", help="free a claim without a state event")
    p.add_argument("wi")
    p.add_argument("--force", action="store_true", help="free a live (non-stale) claim")
    sub.add_parser("status", help="claims, counts and what is claimable")
    sub.add_parser("lint", help="check the index and WI headers")
    p = sub.add_parser("audit", help="retrospective analysis of past runs; --apply records corrections")
    p.add_argument("--apply", action="store_true", help="write lessons, hold repeat failures, add backlog items, commit")
    p.add_argument("--dry-run", action="store_true")
    p = sub.add_parser("backlog", help="manage backlog items", formatter_class=argparse.RawDescriptionHelpFormatter,
                       epilog="add flags: --title T --why W [--area A] [--files F;F] [--origin O] [--state {hold,ready,verify-only}] [--dry-run]\n"
                              "promote flags: WI-NN --state {ready,verify-only,hold} [--dry-run]")
    bs = p.add_subparsers(dest="bcmd", required=True, metavar="{add,list,promote}")
    q = bs.add_parser("add", help="push a new item (default state hold)")
    q.add_argument("--title", required=True)
    q.add_argument("--area", default="backlog")
    q.add_argument("--why", required=True)
    q.add_argument("--files", default="", help="semicolon-separated write files")
    q.add_argument("--origin", default="manual")
    q.add_argument("--state", default="hold", choices=["hold", "ready", "verify-only"])
    q.add_argument("--dry-run", action="store_true")
    bs.add_parser("list", help="list backlog items")
    q = bs.add_parser("promote", help="change an item's state")
    q.add_argument("wi")
    q.add_argument("--state", required=True, choices=["ready", "verify-only", "hold"])
    q.add_argument("--dry-run", action="store_true")
    return ap


HANDLERS = {"next": cmd_next, "verify": cmd_verify, "docs": cmd_docs, "close": cmd_close, "report": cmd_report, "park": cmd_park,
            "rescue": cmd_rescue, "unpark": cmd_unpark, "release": cmd_release, "status": cmd_status, "lint": cmd_lint, "audit": cmd_audit, "backlog": cmd_backlog}


def main(argv=None):
    a = build_parser().parse_args(argv)
    st = ws.Store(a.repo_root, a.claims_dir)
    if hasattr(a, "wi") and a.wi and not re.fullmatch(r"WI-\d+[a-z]*", a.wi):
        print(f"error: {a.wi!r} is not a WI id like WI-48", file=sys.stderr)
        return 1
    try:
        return HANDLERS[a.cmd](a, st)
    except DriverError as e:
        print(f"error: {e}", file=sys.stderr)
        return e.code


if __name__ == "__main__":
    sys.exit(main())
