#!/usr/bin/env python3
"""Build a deterministic, scrubbed, size-capped "playtest bundle" (JSON) from a player symptom plus existing KOTOR II mod logs.

Usage:
    playtest_bundle.py --symptom TEXT --kind {crash,logic,visual,ux} --log NAME[@N]... [--mod ID]... [--save NAME]
                       [--game-dir G] [--max-bytes N] --out FILE

  --log NAME[@N]   bare mod-log file name in the game dir (repeatable). N = launch number of THAT log (1-based, default: the last
                   launch). Names with path separators or ".." are refused. Launches are split per file by log_triage.py.
  --mod ID         mod id the player was using (repeatable, recorded only; there is no mod-to-log map).
  --save NAME      save slot / name the player was on (recorded only; saves are never read).
  --game-dir G     default: $KOTOR2_ROOT or the Steam path used by kotor2_logs.py (read-only here).
  --max-bytes N    cap for the serialized bundle (default 49152). Lowest-priority excerpts are dropped (and listed) to fit.
  --out FILE       bundle path, written atomically (tmp + os.replace). Nothing is written when the tool exits 2.

How it works: each log is SNAPSHOT-copied to a private temp dir (so a game that is still writing cannot make the numbers disagree),
log_triage.main() runs on the snapshot with --json (never --new, --state points into the temp dir and is never created), and
bounded excerpts (context 4 lines, <= 30 lines, <= 240 chars per line) are cut around alerts, then the banner, then top signatures.
Every excerpt line, the symptom, the mod ids and the save name go through scrub_text from ~/.claude/swarm/lib/scrub.py (loaded lazily;
a missing lib is exit 2, never an unscrubbed bundle), then a project pass for Wine/Proton paths (Z:\\home\\<user>, C:\\users\\<user>),
then a residual check of the final bytes against forbidden terms (the home dir and the user name); any hit is exit 2.
No absolute path is ever copied into the bundle.

Priority and caps (frozen contract): alerts, then the banner, then top signatures; items_per_log 12, max_items 40 (fair share across
logs, one tier at a time), max_bytes. Anything dropped by a cap is listed in dropped[] and sets truncated=true. Excerpts that lie
fully inside an already kept excerpt of the same launch are skipped (lossless, not listed).

bundle_sha256 = sha256 of the canonical JSON (sort_keys, separators (",",":"), ensure_ascii=False, utf-8) of the bundle without that
key. The file is that canonical JSON plus one newline. Evidence id: ev:<log>:L<a>-<b>:<sha8>.

Exit: 0 written; 2 refused or failed (message on stderr, no output file).
"""
from __future__ import annotations

import argparse
import getpass
import hashlib
import io
import json
import os
import re
import shutil
import sys
import tempfile
import time
from datetime import datetime, timezone
from pathlib import Path

HERE = Path(__file__).resolve().parent
LIB_DIR = "~/.claude/swarm/lib"
FORMAT = "playtest-bundle/1"
KINDS = ("crash", "logic", "visual", "ux")
CONTEXT = 4
ITEM_LINES = 30
LINE_CHARS = 240
ITEMS_PER_LOG = 12
MAX_ITEMS = 40
DEFAULT_MAX_BYTES = 49152
SYMPTOM_MAX = 2000
SAVE_MAX = 200
STALE_SECONDS = 7 * 86400
MAX_LOG_BYTES = 128 << 20
TOP = 12
TIER = {"alert": 0, "banner": 1, "signature": 2}

RE_LOG_SPEC = re.compile(r"^([A-Za-z0-9][A-Za-z0-9._-]{0,127})(?:@([0-9]{1,9}))?$")
RE_MOD = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$")
RE_CTRL = re.compile(r"[\x00-\x08\x0b-\x1f\x7f]")
# '/home/<name>', '\home\<name>', 'Z:\\home\\<name>', '\users\<name>' (Wine/Proton, any case, any run of separators)
RE_HOMEISH = re.compile(r"(?i)([\\/]+(?:home|users)[\\/]+)[^\\/\s\"'<>|:*?,;()\[\]{}]+")


class Fail(Exception):
    """A refusal: message goes to stderr, exit code 2, no output file."""


# ---------------------------------------------------------------- scrubbing

def load_scrub():
    """Import scrub_text from the swarm lib (lazily; sys.path gets the lib dir appended). Any problem is a Fail."""
    import importlib.util
    lib = os.path.expanduser(LIB_DIR)
    if lib not in sys.path:
        sys.path.append(lib)
    path = os.path.join(lib, "scrub.py")
    old = sys.dont_write_bytecode
    sys.dont_write_bytecode = True  # never write a __pycache__ into the lib dir
    try:
        spec = importlib.util.spec_from_file_location("playtest_bundle_scrub", path)
        if spec is None or spec.loader is None:
            raise ImportError("no spec")
        mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)
        fn = getattr(mod, "scrub_text")
    except Exception as e:  # noqa: BLE001 - every failure mode means "do not emit"
        raise Fail("scrub library unavailable (%s from %s): refusing to emit an unscrubbed bundle" % (type(e).__name__, LIB_DIR))
    finally:
        sys.dont_write_bytecode = old
    return fn


def check_scrub(fn):
    """Self-check of a scrub_text callable: it must really rewrite a home path (fail closed on a stub)."""
    try:
        got = fn("/home/playtest_probe_user/x")
    except Exception as e:  # noqa: BLE001
        raise Fail("scrub library self-check raised %s" % type(e).__name__)
    if got != "/home/USER/x":
        raise Fail("scrub library self-check failed: refusing to emit an unscrubbed bundle")
    return fn


def project_scrub(s):
    """Project-side pass for Wine/Proton style home paths that scrub_text does not know. Idempotent."""
    return RE_HOMEISH.sub(r"\1USER", s)


class Scrubber:
    def __init__(self, scrub_text):
        self.f = scrub_text

    def line(self, s):
        """-> (text, control chars replaced, lib changed it, project pass changed it)."""
        t, ctrl = RE_CTRL.subn(" ", s)
        u = self.f(t)
        if not isinstance(u, str):
            raise Fail("scrub library returned a non-string")
        v = project_scrub(u)
        return v, ctrl, u != t, v != u

    def field(self, s):
        v, _, _, _ = self.line(s)
        return v


def default_forbidden_terms():
    """Strings that must not survive in the final bundle: the home dir and the user name (case-insensitive check)."""
    home = os.path.expanduser("~")
    cands = [home]
    try:
        real = os.path.realpath(home)
        cands.append(real)
    except OSError:
        pass
    try:
        cands.append(getpass.getuser())
    except Exception:  # noqa: BLE001
        pass
    cands += [os.environ.get("USER", ""), os.environ.get("LOGNAME", "")]
    out = []
    for t in cands:
        t = (t or "").strip()
        if t and t != "/" and t not in out:
            out.append(t)
    return out


def residual_hits(text, terms):
    low = text.lower()
    return [i for i, t in enumerate(terms) if t and t.lower() in low]


# ---------------------------------------------------------------- helpers

def canonical(obj):
    return json.dumps(obj, sort_keys=True, separators=(",", ":"), ensure_ascii=False)


def window(anchor, lo, hi, context, max_lines):
    """Line window [s, e] around `anchor`, clamped to the launch [lo, hi] and to `max_lines` lines (anchor stays inside)."""
    s = max(lo, anchor - context)
    e = min(hi, anchor + context)
    if e - s + 1 > max_lines:
        s = max(lo, min(anchor - max_lines // 2, hi - max_lines + 1))
        e = s + max_lines - 1
    return s, e


def split_lines(data):
    """Same line model as log_triage.read_lines (utf-8 replace, split on \\n, drop one trailing empty piece and trailing \\r)."""
    parts = data.decode("utf-8", errors="replace").split("\n")
    if parts and parts[-1] == "":
        parts.pop()
    return [p[:-1] if p.endswith("\r") else p for p in parts]


def parse_log_spec(spec):
    if "/" in spec or "\\" in spec:
        raise Fail("--log %r: path separators are not allowed, pass a bare log file name" % spec)
    if ".." in spec:
        raise Fail("--log %r: '..' is not allowed" % spec)
    m = RE_LOG_SPEC.match(spec)
    if not m:
        raise Fail("--log %r: expected NAME or NAME@N (letters, digits, . _ -; N = launch number >= 1)" % spec)
    name, n = m.group(1), m.group(2)
    launch = None
    if n is not None:
        launch = int(n)
        if launch < 1:
            raise Fail("--log %r: launch number must be >= 1" % spec)
    return name, launch


def utc_stamp(epoch):
    return datetime.fromtimestamp(int(epoch), timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def load_tools():
    """Lazy import of the sibling tools (python -I does not put the script dir on sys.path)."""
    if str(HERE) not in sys.path:
        sys.path.insert(0, str(HERE))
    import kotor2_logs
    import log_triage
    return log_triage, kotor2_logs


# ---------------------------------------------------------------- per-log collection

def run_triage(triage_main, snap, launch, state_path):
    argv = [snap, "--json", "--top", str(TOP), "--state", state_path]  # never --new: the bookmark file is never touched
    if launch:
        argv += ["--launch", str(launch)]
    buf = io.StringIO()
    try:
        rc = triage_main(argv, out=buf)
    except SystemExit as e:
        raise Fail("log_triage exited (%s)" % e.code)
    text = buf.getvalue().strip()
    if rc != 0:
        raise Fail("log_triage failed (rc=%s)" % rc)
    try:
        obj = json.loads(text)
    except ValueError:
        raise Fail("log_triage output is not JSON")
    if not isinstance(obj, dict) or not all(k in obj for k in ("total_lines", "launches", "shown", "verdict_word")):
        raise Fail("log_triage output has an unexpected shape")
    return obj


def collect_log(name, launch_req, idx, game_dir, snapdir, state_path, triage_main, parser_label, scrubber, now):
    src = os.path.join(game_dir, name)
    if not os.path.isfile(src):
        raise Fail("log not found in the game dir: %s" % name)
    if os.path.dirname(os.path.realpath(src)) != os.path.realpath(game_dir):
        raise Fail("log %s resolves outside the game dir" % name)
    try:
        with open(src, "rb") as f:
            st = os.fstat(f.fileno())
            data = f.read(MAX_LOG_BYTES + 1)
    except OSError as e:
        raise Fail("cannot read log %s (%s)" % (name, type(e).__name__))
    if len(data) > MAX_LOG_BYTES:
        raise Fail("log %s is larger than %d MiB" % (name, MAX_LOG_BYTES >> 20))
    sub = os.path.join(snapdir, str(idx))
    os.mkdir(sub)
    snap = os.path.join(sub, name)
    with open(snap, "wb") as f:
        f.write(data)
    lines = split_lines(data)
    obj = run_triage(triage_main, snap, launch_req, state_path)
    if obj["total_lines"] != len(lines):
        raise Fail("log %s: line count mismatch between log_triage (%s) and the snapshot (%d)" % (name, obj["total_lines"], len(lines)))
    entry = {"name": name, "parser": parser_label(snap), "total_lines": len(lines), "verdict_word": obj["verdict_word"],
             "file_sha256": hashlib.sha256(data).hexdigest(), "mtime_utc": utc_stamp(st.st_mtime),
             "stale": (now - st.st_mtime) > STALE_SECONDS}
    shown = obj["shown"]
    gaps, dropped, items = [], [], []
    if not shown:
        if launch_req:
            raise Fail("log %s: no such launch @%d (the file has %s launch(es))" % (name, launch_req, obj["launches"]))
        if obj["launches"] != 0 or lines:
            raise Fail("log %s: log_triage returned no launch for a non-empty file" % name)
        entry.update(launch=None, start=None, end=None, alerts_total=0)
        gaps.append("log %s is empty: no launch and no excerpt" % name)
        return entry, items, dropped, gaps
    if len(shown) != 1:
        raise Fail("log %s: log_triage returned %d launches, expected 1" % (name, len(shown)))
    L = shown[0]
    if launch_req and L.get("n") != launch_req:
        raise Fail("log %s: log_triage returned launch %s, wanted @%d" % (name, L.get("n"), launch_req))
    lo, hi = L["start"], L["end"]
    if not (1 <= lo <= hi <= len(lines)):
        raise Fail("log %s: launch range %s-%s is outside the file" % (name, lo, hi))
    launch = L["n"]
    alerts_total = L.get("alerts_total", len(L["alerts"]))
    entry.update(launch=launch, start=lo, end=hi, alerts_total=alerts_total)
    if alerts_total > len(L["alerts"]):
        dropped.append({"log": name, "launch": launch, "reason": "alerts_over_triage_cap",
                        "count": alerts_total - len(L["alerts"])})
    cands = [("alert", r["line"]) for r in L["alerts"]] + [("banner", lo)] + [("signature", r["line"]) for r in L["top"]]
    accepted = []  # (start, end)
    per_tier = {0: 0, 1: 0, 2: 0}
    for rank, (kind, line) in enumerate(cands):
        if not (lo <= line <= hi):
            raise Fail("log %s: anchor line %s is outside launch %s" % (name, line, launch))
        s, e = window(line, lo, hi, CONTEXT, ITEM_LINES)
        if any(a <= s and e <= b for a, b in accepted):
            continue  # fully inside a kept excerpt: lossless skip
        if len(accepted) >= ITEMS_PER_LOG:
            dropped.append({"log": name, "launch": launch, "anchor": {"kind": kind, "line": line}, "reason": "items_per_log"})
            continue
        accepted.append((s, e))
        texts, cut_any = [], False
        ctrl = lib = proj = 0
        out_lines = []
        for n in range(s, e + 1):
            t, c, l, p = scrubber.line(lines[n - 1])
            ctrl += c
            lib += 1 if l else 0
            proj += 1 if p else 0
            if len(t) > LINE_CHARS:
                t = t[:LINE_CHARS - 3] + "..."
                cut_any = True
            texts.append(t)
            out_lines.append({"n": n, "text": t})
        digest = hashlib.sha256("\n".join(texts).encode("utf-8")).hexdigest()
        ev = {"id": "ev:%s:L%d-%d:%s" % (name, s, e, digest[:8]), "log": name, "launch": launch,
              "anchor": {"kind": kind, "line": line}, "start": s, "end": e, "lines": out_lines,
              "text_sha256": digest, "line_cut": cut_any}
        tier = TIER[kind]
        items.append({"ev": ev, "tier": tier, "tier_idx": per_tier[tier], "rank": rank,
                      "sc": {"lines": len(out_lines), "ctrl": ctrl, "lib": lib, "proj": proj}})
        per_tier[tier] += 1
    return entry, items, dropped, gaps


# ---------------------------------------------------------------- assembly

BASE_GAPS = (
    "no wall-clock window: log lines carry no absolute time, so a launch cannot be placed relative to when the symptom happened",
    "no save parsing: the save name is recorded as given, save contents are never read",
    "no mod-to-log map beyond kotor2_logs.py routing: --mod ids are recorded, not linked to logs",
)


def dropped_key(d):
    a = d.get("anchor")
    return (d["log"], d["launch"] or 0, a["line"] if a else 0, d["reason"])


def assemble(intake, intake_changed, logs, kept, dropped, gaps, nterms, max_bytes):
    kept_out = sorted(kept, key=lambda it: (it["log_idx"], it["rank"]))
    sc = lambda k: sum(it["sc"][k] for it in kept)  # noqa: E731
    bundle = {
        "format": FORMAT,
        "intake": intake,
        "logs": logs,
        "evidence": [it["ev"] for it in kept_out],
        "caps": {"context": CONTEXT, "item_lines": ITEM_LINES, "line_chars": LINE_CHARS, "items_per_log": ITEMS_PER_LOG,
                 "max_items": MAX_ITEMS, "max_bytes": max_bytes},
        "truncated": bool(dropped),
        "dropped": sorted(dropped, key=dropped_key),
        "scrub": {"lib": "scrub_text", "lines_scanned": sc("lines"), "lines_changed_lib": sc("lib"),
                  "lines_changed_project": sc("proj"), "control_chars_replaced": sc("ctrl"),
                  "intake_fields_changed": intake_changed, "forbidden_terms_checked": nterms, "residual_check": "pass"},
        "gaps": gaps,
    }
    digest = hashlib.sha256(canonical(bundle).encode("utf-8")).hexdigest()
    bundle["bundle_sha256"] = digest
    raw = (canonical(bundle) + "\n").encode("utf-8")
    return bundle, raw


def write_atomic(path, data):
    d = os.path.dirname(os.path.abspath(path))
    fd, tmp = tempfile.mkstemp(prefix=".playtest_bundle.", suffix=".tmp", dir=d)
    try:
        with os.fdopen(fd, "wb") as f:
            f.write(data)
            f.flush()
            os.fsync(f.fileno())
        os.replace(tmp, path)
    except OSError as e:
        try:
            os.unlink(tmp)
        except OSError:
            pass
        raise Fail("cannot write the output file (%s)" % type(e).__name__)


def build_bundle(*, symptom, kind, log_specs, mods, save, game_dir, max_bytes, scrubber, forbidden, triage_main, parser_label, now):
    """Return (bundle dict, serialized bytes). Raises Fail. Pure apart from a private temp dir."""
    intake_changed = 0
    s_raw = symptom
    symptom = scrubber.field(symptom)
    intake_changed += symptom != s_raw
    if len(symptom) > SYMPTOM_MAX:
        raise Fail("symptom is longer than %d chars after scrubbing" % SYMPTOM_MAX)
    mods_out = []
    for m in sorted(set(mods)):
        v = scrubber.field(m)
        intake_changed += v != m
        mods_out.append(v)
    save_out = None
    if save is not None:
        save_out = scrubber.field(save)
        intake_changed += save_out != save
    intake = {"symptom": symptom, "kind": kind, "mods": mods_out, "save": save_out}

    tmpdir = tempfile.mkdtemp(prefix="playtest_bundle_")
    try:
        state_path = os.path.join(tmpdir, "state", "bookmarks.json")  # handed to log_triage, never created (no --new)
        snapdir = os.path.join(tmpdir, "snap")
        os.mkdir(snapdir)
        results = []
        for i, (name, launch) in enumerate(log_specs):
            results.append(collect_log(name, launch, i, game_dir, snapdir, state_path, triage_main, parser_label, scrubber, now))
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)

    results.sort(key=lambda r: (r[0]["name"], r[0]["launch"] or 0))
    seen = set()
    logs, all_items, dropped, extra_gaps = [], [], [], []
    for log_idx, (entry, items, drops, gaps) in enumerate(results):
        key = (entry["name"], entry["launch"])
        if key in seen:
            raise Fail("duplicate log/launch: %s@%s" % key)
        seen.add(key)
        logs.append(entry)
        for it in items:
            it["log_idx"] = log_idx
        all_items += items
        dropped += drops
        extra_gaps += gaps
    ids = [it["ev"]["id"] for it in all_items]
    if len(set(ids)) != len(ids):
        raise Fail("internal error: duplicate evidence ids")
    gaps = list(BASE_GAPS) + sorted(extra_gaps)

    prio = lambda it: (it["tier"], it["tier_idx"], it["log_idx"], it["rank"])  # noqa: E731
    ordered = sorted(all_items, key=prio)
    kept = ordered[:MAX_ITEMS]
    for it in ordered[MAX_ITEMS:]:
        ev = it["ev"]
        dropped.append({"log": ev["log"], "launch": ev["launch"], "anchor": ev["anchor"], "reason": "max_items"})

    while True:
        bundle, raw = assemble(intake, int(intake_changed), logs, kept, dropped, gaps, len(forbidden), max_bytes)
        if len(raw) <= max_bytes:
            break
        if not kept:
            raise Fail("--max-bytes %d is too small to hold even the bundle skeleton (%d bytes)" % (max_bytes, len(raw)))
        victim = kept.pop()  # lowest priority first
        ev = victim["ev"]
        dropped.append({"log": ev["log"], "launch": ev["launch"], "anchor": ev["anchor"], "reason": "max_bytes"})

    hits = residual_hits(raw.decode("utf-8"), forbidden)
    if hits:
        raise Fail("residual check failed: %d forbidden term(s) (index %s) survive scrubbing in the bundle; refusing to emit"
                   % (len(hits), ",".join(str(h) for h in hits)))
    return bundle, raw


# ---------------------------------------------------------------- CLI

def build_parser():
    ap = argparse.ArgumentParser(prog="playtest_bundle.py", description="Build a scrubbed, size-capped playtest bundle from a "
                                 "symptom plus mod logs (see the module docstring).")
    ap.add_argument("--symptom", required=True, help="what the player saw (1..2000 chars)")
    ap.add_argument("--kind", required=True, choices=KINDS)
    ap.add_argument("--log", action="append", default=[], metavar="NAME[@N]", help="mod log file name in the game dir, "
                    "optional launch number (default: last launch); repeatable")
    ap.add_argument("--mod", action="append", default=[], metavar="ID", help="mod id (repeatable, recorded only)")
    ap.add_argument("--save", default=None, help="save slot/name (recorded only)")
    ap.add_argument("--game-dir", default=None, help="default: $KOTOR2_ROOT or the Steam path from kotor2_logs.py")
    ap.add_argument("--max-bytes", type=int, default=DEFAULT_MAX_BYTES, help="cap for the serialized bundle (default %d)" % DEFAULT_MAX_BYTES)
    ap.add_argument("--out", required=True, help="bundle file to write (atomic)")
    return ap


def run(args, forbidden, scrub_loader, triage_main, now):
    sym = args.symptom
    if len(sym) > SYMPTOM_MAX:
        raise Fail("--symptom is longer than %d chars (%d)" % (SYMPTOM_MAX, len(sym)))
    sym = sym.strip()
    if not sym:
        raise Fail("--symptom is empty")
    if not args.log:
        raise Fail("at least one --log NAME[@N] is required")
    specs = [parse_log_spec(s) for s in args.log]
    if len(set(specs)) != len(specs):
        raise Fail("duplicate --log entries")
    for m in args.mod:
        if not RE_MOD.match(m):
            raise Fail("--mod %r: expected letters, digits, . _ - (1..64 chars)" % m)
    if args.save is not None:
        if not args.save.strip() or len(args.save) > SAVE_MAX or RE_CTRL.search(args.save):
            raise Fail("--save: expected 1..%d printable chars" % SAVE_MAX)
    if args.max_bytes < 1:
        raise Fail("--max-bytes must be a positive integer")
    out = os.path.abspath(args.out)
    if os.path.isdir(out):
        raise Fail("--out is a directory")
    out_dir = os.path.dirname(out)
    if not os.path.isdir(out_dir):
        raise Fail("--out: the parent directory does not exist")

    triage, klogs = load_tools()
    for name, _ in specs:
        if not (klogs.is_candidate(name) or name in klogs.ROUTES):
            raise Fail("--log %s: not a recognised mod log file name (expected *log*.txt, *metrics*.txt, kpatch.log or a routed log)"
                       % name)
    try:
        fn = check_scrub((scrub_loader or load_scrub)())
    except Fail:
        raise
    except Exception as e:  # noqa: BLE001 - an injected or broken loader must still end in a refusal, never a traceback
        raise Fail("scrub library unavailable (%s): refusing to emit an unscrubbed bundle" % type(e).__name__)
    game_dir = args.game_dir or klogs.DEFAULT_GAME_DIR
    game_dir = os.path.expanduser(game_dir)
    if not os.path.isdir(game_dir):
        raise Fail("game dir not found")
    real_game = os.path.realpath(game_dir)
    real_out_dir = os.path.realpath(out_dir)
    if real_out_dir == real_game or real_out_dir.startswith(real_game + os.sep):
        raise Fail("--out must not be inside the game dir")
    terms = default_forbidden_terms() if forbidden is None else [t for t in forbidden if t]
    bundle, raw = build_bundle(symptom=sym, kind=args.kind, log_specs=specs, mods=args.mod, save=args.save, game_dir=game_dir,
                               max_bytes=args.max_bytes, scrubber=Scrubber(fn), forbidden=terms,
                               triage_main=triage_main or triage.main, parser_label=klogs.parser_label,
                               now=(now or time.time)())
    write_atomic(out, raw)
    print("playtest_bundle: ok bytes=%d evidence=%d dropped=%d truncated=%s bundle_sha256=%s"
          % (len(raw), len(bundle["evidence"]), len(bundle["dropped"]), str(bundle["truncated"]).lower(), bundle["bundle_sha256"]))
    return 0


def main(argv=None, *, _forbidden=None, _scrub_loader=None, _triage_main=None, _now=None):
    """_forbidden / _scrub_loader / _triage_main / _now are injection points for tests (forbidden terms, scrub import, triage, clock)."""
    args = build_parser().parse_args(argv)
    try:
        return run(args, _forbidden, _scrub_loader, _triage_main, _now)
    except Fail as e:
        print("playtest_bundle: error: %s" % e, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
