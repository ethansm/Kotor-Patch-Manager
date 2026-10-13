#!/usr/bin/env python3
"""Orchestrator of the blind bug-investigation pipeline (deterministic code only, stdlib only, no model call).

Pipeline (each step is one subcommand, all take --run DIR; a run dir never lives inside the repo):

  new      create RUN/intake.json from a symptom (or a case file) and the logs/mods the player used
  bundle   RUN/bundle.json: scrubbed, hash-checked log excerpts (scripts/playtest_bundle.py, or --from a prebuilt file)
  blind    3 blind units (skeptic, advocate, evidence styles) -> RUN/blind/{units,out}
  null     1 null-hypothesis unit ("is this a bug at all?")      -> RUN/null/{units,out}
  check    quote-check every complete blind/null output against the bundle -> RUN/check.json
  verify   one verify unit per hypothesis that survived the quote check -> RUN/verify/{units,out}
  report   RUN/report.json + a short summary; exit 1 while a blind or null unit is incomplete or a verify unit has no verdict

Model phases go through ONE injectable function, run_stage(), that shells out to swarm_run.py map; unit state is read
ONLY through read_status() (swarm_run.py status --out O --json), never from a glob of findings/. --engine real runs
only with BOTH --claude-bin ABS and --yes-spend USD (after a swarm_run.py probe); --engine stub refuses both flags.
--engine stub needs --stub-script FILE, and blind, null and verify refuse it without one.
Any other combination exits 2 before anything is written.
The orchestrator never reads a case file's ground truth (true_root_cause); the case's blind_forbidden_terms refuse
any blind/null unit prompt that contains one (matched after norm_terms: case, spacing, underscores, hyphens,
zero-width characters and L<n>| line markers ignored; a term under 3 characters is refused). `new` refuses a symptom
that contains one. Verify prompts are not checked (they hold model-written claims). A human must review every report
before a fix starts.
bundle, blind, null, verify, check and report hold RUN/.lock (flock, non-blocking): a second one on the same run exits 2.
Each unit records the bundle_sha256 it was built from (a verify unit also its claim_sha256); a stage built from another
bundle reads as incomplete. `bundle` refuses a run that already has stage data, and a bundle with no excerpts.

Exit: 0 ok; 1 report/units incomplete; 2 refused, bad input, tampered bundle or runner failure.
"""
from __future__ import annotations

import argparse
import contextlib
import fcntl
import hashlib
import json
import math
import os
import re
import subprocess
import sys
import unicodedata
from datetime import datetime, timezone
from pathlib import Path

sys.dont_write_bytecode = True

HERE = Path(__file__).resolve().parent
REPO = HERE.parent
ROOT = os.path.expanduser("~/.cache/bug_investigate")
DEFAULT_SWARM_RUN = "~/.claude/swarm/lib/swarm_run.py"
ROLES = REPO / ".claude" / "skills" / "bug-investigate" / "roles"
# B1/R1.2: expected USD per real call per stage (M4 measurement) for the budget preflight; REAL_STAGE_CEILING mirrors
# the runner's REAL_MAX_TOTAL_USD (swarm_run.py) for --cap-total on --engine real.
EXPECT_CALL = {"blind": 0.006, "null": 0.004, "verify": 0.04}
REAL_STAGE_CEILING = 1.00
NOT_RUN_REASON = "not run (cap reached or stage unfinished)"
LOCK_NAME = ".lock"                  # RUN/.lock: created by `new` (the run claim), flocked by every locked command
LOCKED_CMDS = frozenset(("bundle", "blind", "null", "verify", "check", "report"))

KINDS = ("crash", "logic", "visual", "ux")
HUMAN_KINDS = frozenset(("visual", "ux"))
BLIND_STYLES = ("skeptic", "advocate", "evidence")
BLIND_IDS = tuple("blind-" + s for s in BLIND_STYLES)
NULL_ID = "null-1"
UNIT_ID_RE = re.compile(r"^[a-z0-9][a-z0-9_-]{0,62}$")
HID_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9_-]{0,15}$")
LOG_SPEC_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,127}(?:@[0-9]{1,9})?$")
MOD_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$")
CTRL_RE = re.compile(r"[\x00-\x08\x0b-\x1f\x7f]")
SYMPTOM_MAX = 2000
SAVE_MAX = 200
MAX_UNIT_BYTES = 200 * 1000          # the runner's unit-file limit is 200 KB; stay strictly under it
MAX_BUNDLE_BYTES = 16 << 20
MAX_FINDINGS_BYTES = 1 << 20
CASE_KEYS = ("symptom_text", "kind", "blind_forbidden_terms")      # the ONLY case fields this tool ever keeps
CONFIDENCE = {"high": 3, "medium": 2, "low": 1}
HUMAN_GATE = "a human must review this report before any fix is started"
DROP_ENV = ("DR_CLAUDE_BIN", "CLAUDE_BIN", "CLAUDE_CODE_EFFORT_LEVEL")


class Fail(Exception):
    """A refusal or bad input: message on stderr, exit 2."""


class StageError(Exception):
    """The runner (map/status) failed or returned something unusable: message on stderr, exit 2."""


# ---------------------------------------------------------------- small helpers

def _eqc():
    """Lazy import of the sibling quote checker (python -I does not put the script dir on sys.path)."""
    if str(HERE) not in sys.path:
        sys.path.insert(0, str(HERE))
    import evidence_quote_check
    return evidence_quote_check


def clean_env():
    env = {k: v for k, v in os.environ.items() if k not in DROP_ENV}
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    return env


def dumps(obj, indent=1) -> bytes:
    return (json.dumps(obj, sort_keys=True, indent=indent) + "\n").encode("ascii")


def write_atomic(path, data: bytes):
    d, base = os.path.split(path)
    tmp = os.path.join(d, ".%s.tmp-%d" % (base, os.getpid()))
    try:
        with open(tmp, "wb") as f:
            f.write(data)
            f.flush()
            os.fsync(f.fileno())
        os.replace(tmp, path)
    finally:
        if os.path.exists(tmp):
            os.unlink(tmp)


@contextlib.contextmanager
def run_lock(run):
    """Hold RUN/.lock (exclusive, non-blocking flock) for one command. A second command on the same run is refused
    (exit 2) instead of interleaving its writes. The lock file is never removed, only released (no unlink race)."""
    fd = os.open(os.path.join(run, LOCK_NAME), os.O_RDWR | os.O_CREAT, 0o600)
    try:
        try:
            fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            raise Fail("another bug_investigate command is running on this run") from None
        yield
    finally:
        os.close(fd)


def load_json_file(path, limit):
    """-> parsed JSON; raises OSError / ValueError / RecursionError."""
    with open(path, "rb") as f:
        data = f.read(limit + 1)
    if len(data) > limit:
        raise ValueError("file is larger than %d bytes" % limit)
    return json.loads(data.decode("utf-8"))


def _s(x) -> str:
    return x if isinstance(x, str) else ""


def one_line(x) -> str:
    return re.sub(r"[\r\n]+", " ", str(x))


def utc_now():
    return datetime.now(timezone.utc)


# ---------------------------------------------------------------- run dir, intake, case, bundle

def resolve_run(path) -> str:
    """Resolve a run dir path; a run dir inside the repo is refused (run data never goes into the repo)."""
    real = os.path.realpath(os.path.expanduser(path))
    repo = os.path.realpath(str(REPO))
    if real == repo or real.startswith(repo + os.sep):
        raise Fail("run dir %s is inside the repo; run data must live outside it (default %s)" % (real, ROOT))
    return real


def existing_run(path) -> str:
    run = resolve_run(path)
    if not os.path.isfile(os.path.join(run, "intake.json")):
        raise Fail("%s is not a bug_investigate run dir (no intake.json); create one with `new`" % run)
    return run


def load_intake(run) -> dict:
    try:
        d = load_json_file(os.path.join(run, "intake.json"), 1 << 20)
    except (OSError, ValueError, RecursionError) as e:
        raise Fail("cannot read intake.json: %s" % e)
    ok = (isinstance(d, dict) and d.get("kind") in KINDS and isinstance(d.get("symptom"), str) and d["symptom"].strip()
          and isinstance(d.get("logs"), list) and all(isinstance(x, str) for x in d["logs"])
          and isinstance(d.get("mods"), list) and all(isinstance(x, str) for x in d["mods"])
          and (d.get("case") is None or isinstance(d.get("case"), str))
          and (d.get("save") is None or isinstance(d.get("save"), str))
          and (d.get("game_dir") is None or isinstance(d.get("game_dir"), str)))
    if not ok:
        raise Fail("intake.json is malformed")
    return d


def read_case(path) -> dict:
    """Keep ONLY symptom_text, kind and blind_forbidden_terms. Everything else (true_root_cause included) is dropped."""
    try:
        raw = load_json_file(path, 4 << 20)
    except (OSError, ValueError, RecursionError) as e:
        raise Fail("cannot read case file %s: %s" % (path, e))
    if not isinstance(raw, dict):
        raise Fail("case file %s is not a JSON object" % path)
    keep = {k: raw[k] for k in CASE_KEYS if k in raw}
    del raw
    return keep


_LINE_MARK_RE = re.compile(r"L\d+\|")          # a rendered excerpt line marker: `L13| ` (see render_excerpts)
_SEPARATOR_RE = re.compile(r"[\s_-]+")


def norm_terms(s: str) -> str:
    """The comparison form of a forbidden term or of a prompt: NFKC, format characters (category Cf, e.g. zero-width
    space) removed, `L<n>|` line markers removed, runs of whitespace, underscores and hyphens removed, casefolded.
    So `invalid creature` matches `invalid\\u00a0creature`, `inval\\u200bid creature`, `invalid\\nL13| creature`,
    and `party_count` matches `partycount`."""
    t = unicodedata.normalize("NFKC", s)
    t = "".join(ch for ch in t if unicodedata.category(ch) != "Cf")
    return _SEPARATOR_RE.sub("", _LINE_MARK_RE.sub("", t)).casefold()


def forbidden_terms(case: dict) -> list:
    """The case's blind_forbidden_terms as written. Each term must be at least 3 characters once normalised
    (norm_terms), so a short word cannot refuse every prompt; the Fail names the term."""
    terms = case.get("blind_forbidden_terms")
    if not isinstance(terms, list) or not terms:
        raise Fail("case blind_forbidden_terms must be a non-empty list of strings")
    out = []
    for t in terms:
        if not isinstance(t, str) or not t.strip():
            raise Fail("case blind_forbidden_terms entries must be non-empty strings")
        if len(norm_terms(t)) < 3:
            raise Fail("case blind_forbidden_terms entry %r is shorter than 3 characters (spacing, case ignored)" % t)
        out.append(t)
    return out


def run_terms(intake: dict):
    """None for an ad-hoc run (no case: the forbidden-term check is skipped), else the case's term list (as written)."""
    if not intake.get("case"):
        return None
    return forbidden_terms(read_case(intake["case"]))


def verify_bundle(bundle):
    eqc = _eqc()
    try:
        eqc.index_bundle(bundle)
    except eqc.BundleTampered as e:
        raise Fail("bundle tampered or malformed: %s" % e)
    return bundle


def require_excerpts(bundle) -> dict:
    if not bundle["evidence"]:
        raise Fail("bundle has no excerpts: the log is missing or the launch number does not exist")
    return bundle


def stage_data_present(run) -> bool:
    """True once a stage has written into the run: blind/units, blind/out, or anything under null/ or verify/."""
    if os.path.exists(os.path.join(run, "blind", "units")) or os.path.exists(os.path.join(run, "blind", "out")):
        return True
    return any(os.listdir(os.path.join(run, s)) for s in ("null", "verify") if os.path.isdir(os.path.join(run, s)))


def claim_run_dir(run):
    """Create the run dir for `new`. A fresh path is made with mkdir, so of two racing calls only one gets it. An
    existing EMPTY dir is still accepted. Either way the O_EXCL create of RUN/.lock is the claim: a racer that sees the
    dir empty before the winner has written its intake still loses."""
    os.makedirs(os.path.dirname(run), mode=0o700, exist_ok=True)
    try:
        os.mkdir(run, 0o700)
    except FileExistsError:
        if not os.path.isdir(run) or os.listdir(run):
            raise Fail("run dir %s already exists and is not empty" % run) from None
    try:
        os.close(os.open(os.path.join(run, LOCK_NAME), os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600))
    except FileExistsError:
        raise Fail("run dir %s was claimed by another `new` call" % run) from None


def load_run_bundle(run) -> dict:
    path = os.path.join(run, "bundle.json")
    if not os.path.isfile(path):
        raise Fail("no bundle.json in %s; run `bundle` first" % run)
    try:
        bundle = load_json_file(path, MAX_BUNDLE_BYTES)
    except (OSError, ValueError, RecursionError) as e:
        raise Fail("cannot read bundle.json: %s" % e)
    return verify_bundle(bundle)


def quote_check(bundle, output) -> list:
    eqc = _eqc()
    try:
        return eqc.check(bundle, output)
    except eqc.BundleTampered as e:
        raise Fail("bundle tampered or malformed: %s" % e)


# ---------------------------------------------------------------- prompts and units

def render_excerpts(bundle, ids=None) -> str:
    """Header `[<id>] log=<log> launch=<n> anchor=<kind>@L<line>` then `L<n>| <text>` per line, in bundle order."""
    blocks, seen = [], set()
    for ex in bundle["evidence"]:
        if ids is not None and ex["id"] not in ids:
            continue
        if ex["id"] in seen:
            continue
        seen.add(ex["id"])
        anchor = ex.get("anchor") if isinstance(ex.get("anchor"), dict) else {}
        head = "[%s] log=%s launch=%s anchor=%s@L%s" % (ex["id"], one_line(ex.get("log")), one_line(ex.get("launch")),
                                                         one_line(anchor.get("kind")), one_line(anchor.get("line")))
        body = ["L%s| %s" % (one_line(l.get("n")), one_line(l["text"])) for l in ex["lines"]]
        blocks.append("\n".join([head] + body))
    return "\n\n".join(blocks) if blocks else "(no excerpts)"


def style_fragment(style) -> str:
    path = ROLES / "blind" / "styles" / (style + ".md")
    try:
        return path.read_text(encoding="utf-8").strip()
    except OSError as e:
        raise Fail("cannot read style fragment %s: %s" % (path, e))


def blind_prompt(symptom, style, excerpts, uid) -> str:
    return "\n".join(["Symptom reported by the tester:", symptom, "", style_fragment(style), "",
                      "Log excerpts (cite the evidence id in square brackets exactly):", excerpts, "",
                      "Return unit_id exactly as: %s" % uid])


def null_prompt(symptom, excerpts, uid) -> str:
    return "\n".join(["Symptom reported by the tester:", symptom, "",
                      "Log excerpts (cite the evidence id in square brackets exactly):", excerpts, "",
                      "Return unit_id exactly as: %s" % uid])


def verify_uid(unit_id, hid) -> str:
    uid = ("verify-%s-%s" % (unit_id, hid)).lower()
    if not UNIT_ID_RE.match(uid):
        raise Fail("cannot form a unit id from %r / %r" % (unit_id, hid))
    return uid


def verify_prompt(symptom, excerpts, h, uid) -> str:
    return "\n".join(["Symptom reported by the tester:", symptom, "",
                      "Hypothesis under review (proposed by someone else; treat it as unproven):",
                      "claim: %s" % _s(h["claim"]), "mechanism: %s" % _s(h["mechanism"]),
                      "falsifier: %s" % _s(h["falsifier"]), "hypothesis_id: %s" % h["hypothesis_id"], "",
                      "Log excerpts cited by the hypothesis (cite the evidence id in square brackets exactly):",
                      excerpts, "",
                      "Return unit_id exactly as: %s and hypothesis_id exactly as: %s" % (uid, h["hypothesis_id"])])


def make_unit(uid, prompt, meta) -> dict:
    unit = {"unit_id": uid, "prompt": prompt, "meta": meta}
    size = len(json.dumps(unit, sort_keys=True).encode("ascii"))
    if size > MAX_UNIT_BYTES:
        raise Fail("unit %s would be %d bytes, over the 200 KB unit limit; shrink the bundle" % (uid, size))
    return unit


def write_units(units_dir, units: dict):
    """Write the unit files atomically and drop stale *.json unit files of an earlier build (the dir is ours)."""
    if units or os.path.isdir(units_dir):
        os.makedirs(units_dir, exist_ok=True)
    if os.path.isdir(units_dir):
        for name in os.listdir(units_dir):
            if name.endswith(".json") and name[:-5] not in units:
                os.unlink(os.path.join(units_dir, name))
    for uid, unit in sorted(units.items()):
        if legacy_unit_kept(os.path.join(units_dir, uid + ".json"), unit):
            continue
        write_atomic(os.path.join(units_dir, uid + ".json"), json.dumps(unit, sort_keys=True).encode("ascii"))


def legacy_unit_kept(path, unit) -> bool:
    """True when `path` holds a unit built before the bundle/claim stamps existed (no bundle_sha256 in its meta) that
    equals `unit` apart from the stamps. Its bytes stay, so the runner's record (keyed on the unit file's sha256) stays
    complete: rewriting would make a resumed run re-run and re-pay every finished unit."""
    try:
        old = load_json_file(path, MAX_UNIT_BYTES)
    except (OSError, ValueError, RecursionError):
        return False
    if not isinstance(old, dict) or not isinstance(old.get("meta"), dict) or "bundle_sha256" in old["meta"]:
        return False
    stamps = ("bundle_sha256", "claim_sha256")
    strip = lambda u: {k: v for k, v in u.items() if k != "meta"} | {
        "meta": {k: v for k, v in u["meta"].items() if k not in stamps}}
    return strip(old) == strip(unit)


def list_units(units_dir) -> list:
    if not os.path.isdir(units_dir):
        return []
    return sorted(n[:-5] for n in os.listdir(units_dir) if n.endswith(".json") and UNIT_ID_RE.match(n[:-5]))


def unit_meta(units_dir, uid):
    """The meta object of a unit file, or None when the file cannot be read or has no meta object."""
    try:
        unit = load_json_file(os.path.join(units_dir, uid + ".json"), MAX_UNIT_BYTES)
    except (OSError, ValueError, RecursionError):
        return None
    meta = unit.get("meta") if isinstance(unit, dict) else None
    return meta if isinstance(meta, dict) else None


# ---------------------------------------------------------------- injectable runner (the ONLY place that starts swarm_run)

def run_stage(role_dir, units_dir, out_dir, engine, cap_total, *, cap_call, stub_script, parallel, timeout,
              backoff_scale, swarm_run, claude_bin=None, yes_spend=None) -> int:
    """swarm_run.py map ...; -> its exit code. The env never carries a binary override or an effort level.
    engine real: swarm_run.py probe first (exit != 0 is a StageError), then map with the explicit claude binary,
    the probe record and the spend approval. engine stub takes neither claude_bin nor yes_spend."""
    real = engine == "real"
    if real and (not claude_bin or yes_spend is None):
        raise StageError("engine real needs claude_bin and yes_spend")
    if not real and (claude_bin is not None or yes_spend is not None):
        raise StageError("only engine real takes claude_bin and yes_spend")
    cmd = [sys.executable, "-I", swarm_run, "map", "--role-dir", role_dir, "--units", units_dir, "--out", out_dir,
           "--engine", engine, "--cap-total", str(cap_total), "--cap-call", str(cap_call), "--parallel", str(parallel),
           "--timeout", str(timeout)]
    if stub_script is not None:
        cmd += ["--stub-script", stub_script]
    if backoff_scale is not None:
        cmd += ["--backoff-scale", str(backoff_scale)]
    if real:
        home_probe = os.path.join(out_dir, "work", "home_probe.json")
        try:
            os.makedirs(os.path.dirname(home_probe), exist_ok=True)
            probe = subprocess.run([sys.executable, "-I", swarm_run, "probe", "--out", out_dir], env=clean_env(),
                                   stdin=subprocess.DEVNULL).returncode
        except OSError as e:
            raise StageError("cannot start swarm_run probe: %s" % e)
        if probe != 0:
            raise StageError("swarm_run probe exited %d; the map was not started" % probe)
        cmd += ["--claude-bin", claude_bin, "--home-probe", home_probe, "--yes-spend", str(yes_spend)]
    try:
        return subprocess.run(cmd, env=clean_env(), stdin=subprocess.DEVNULL).returncode
    except OSError as e:
        raise StageError("cannot start swarm_run map: %s" % e)


def read_status(out_dir, swarm_run) -> dict:
    """swarm_run.py status --out O --json -> the payload dict. Runner exit != 0 or unparseable output is a StageError."""
    cmd = [sys.executable, "-I", swarm_run, "status", "--out", out_dir, "--json"]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, env=clean_env(), stdin=subprocess.DEVNULL, timeout=120)
    except (OSError, subprocess.TimeoutExpired) as e:
        raise StageError("cannot run swarm_run status: %s" % e)
    if p.returncode != 0:
        why = ((p.stderr or "").strip().splitlines() or [""])[-1][:200]
        raise StageError("swarm_run status exited %d: %s" % (p.returncode, why))
    try:
        payload = json.loads(p.stdout)
    except ValueError:
        raise StageError("swarm_run status printed unparseable JSON")
    if not isinstance(payload, dict):
        raise StageError("swarm_run status payload is not a JSON object")
    return payload


def read_spent(out_dir, cap_call, swarm_run) -> float:
    """swarm_run.py spent --out O --cap-call X -> USD already spent in this stage's out dir; 0.0 before the first map
    (no work/manifest.json yet). Runner exit != 0 or a number that is not finite and >= 0 is a StageError."""
    if not os.path.isfile(os.path.join(out_dir, "work", "manifest.json")):
        return 0.0
    cmd = [sys.executable, "-I", swarm_run, "spent", "--out", out_dir, "--cap-call", str(cap_call)]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, env=clean_env(), stdin=subprocess.DEVNULL, timeout=120)
    except (OSError, subprocess.TimeoutExpired) as e:
        raise StageError("cannot run swarm_run spent: %s" % e)
    if p.returncode != 0:
        why = ((p.stderr or "").strip().splitlines() or [""])[-1][:200]
        raise StageError("swarm_run spent exited %d: %s" % (p.returncode, why))
    try:
        spent = float(((p.stdout or "").strip().splitlines() or [""])[0])
    except ValueError:
        raise StageError("swarm_run spent printed no number")
    if not math.isfinite(spent) or spent < 0:
        raise StageError("swarm_run spent printed %r, not a USD amount" % spent)
    return spent


def pending_units(out_dir, units, swarm_run) -> int:
    """How many of the stage's units the runner does not hold as complete (all of them before the first map)."""
    if not os.path.isfile(os.path.join(out_dir, "work", "manifest.json")):
        return len(units)
    status = parse_status(read_status(out_dir, swarm_run))
    return sum(1 for uid in units if not status.get(uid, (False, ""))[0])


def parse_status(payload) -> dict:
    """-> {unit_id: (complete, reason)}. Anything malformed is a StageError (fail closed)."""
    if not isinstance(payload, dict) or not isinstance(payload.get("units"), list):
        raise StageError("status payload has no units list")
    out = {}
    for u in payload["units"]:
        if not (isinstance(u, dict) and isinstance(u.get("unit_id"), str) and isinstance(u.get("complete"), bool)):
            raise StageError("status payload has a malformed unit entry")
        out[u["unit_id"]] = (u["complete"], u["reason"] if isinstance(u.get("reason"), str) else "")
    return out


def run_builder(cmd):
    """Run scripts/playtest_bundle.py -> (rc, stderr text). The only log reader of the pipeline."""
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, env=clean_env(), stdin=subprocess.DEVNULL, timeout=300)
    except (OSError, subprocess.TimeoutExpired) as e:
        return 2, str(e)
    return p.returncode, (p.stderr or "").strip()


# ---------------------------------------------------------------- stage evaluation (status -> findings -> quote check)

def evaluate_stage(run, stage, expected, bundle, swarm_run) -> dict:
    """-> {unit_id: {complete, reason, verdicts, output, meta}} for the stage's units plus the `expected` ones.

    Unit state comes ONLY from read_status(). The findings file of an incomplete unit is never opened. A complete unit
    whose meta stamps another bundle's bundle_sha256 is incomplete; a unit without that stamp (built before stamps
    existed) is accepted."""
    units_dir = os.path.join(run, stage, "units")
    out_dir = os.path.join(run, stage, "out")
    present = list_units(units_dir)
    status = {}
    if present:
        try:
            status = parse_status(read_status(out_dir, swarm_run))
        except StageError:
            if os.path.isdir(out_dir):
                raise
            # built but never started: no out dir, so the runner has no manifest to report on
    result = {}
    for uid in sorted(set(present) | set(expected)):
        entry = {"complete": False, "reason": "", "verdicts": [], "output": None, "meta": {}}
        if uid not in present:
            entry["reason"] = "unit file missing (stage not built)"
        elif uid not in status:
            entry["reason"] = "not in runner status" if os.path.isdir(out_dir) else "stage not run"
        elif not status[uid][0]:
            entry["reason"] = status[uid][1] or "incomplete"
        else:
            meta = unit_meta(units_dir, uid)
            if meta is None:
                entry["reason"] = "unit file unreadable"
            elif "bundle_sha256" in meta and meta["bundle_sha256"] != bundle["bundle_sha256"]:
                entry["reason"] = "unit built from another bundle"
            else:
                entry["meta"] = meta
                path = os.path.join(out_dir, "findings", uid + ".json")
                try:
                    output = load_json_file(path, MAX_FINDINGS_BYTES)
                except (OSError, ValueError, RecursionError) as e:
                    entry["reason"] = "findings unreadable: %s" % e
                else:
                    if not isinstance(output, dict):
                        entry["reason"] = "findings is not a JSON object"
                    elif output.get("unit_id") != uid:
                        entry["reason"] = "findings unit_id %r != %s" % (output.get("unit_id"), uid)
                    else:
                        entry.update(complete=True, reason=status[uid][1], output=output,
                                     verdicts=quote_check(bundle, output))
        result[uid] = entry
    return result


def claim_hash(h) -> str:
    """sha256 of one hypothesis's claim, mechanism and falsifier: a verify unit stamped with another hash is stale."""
    return hashlib.sha256((h["claim"] + "\x1f" + h["mechanism"] + "\x1f" + h["falsifier"]).encode("utf-8")).hexdigest()


def collect_hypotheses(blind: dict) -> list:
    """Flatten the complete blind units into hypothesis records (quote-check result applied, ids validated)."""
    hyps = []
    for uid in sorted(blind):
        r = blind[uid]
        items = r["output"].get("hypotheses") if r["complete"] else None
        if not isinstance(items, list):
            continue
        by_index = {v.get("index"): v for v in r["verdicts"][:len(items)]}
        seen = set()
        for i, h in enumerate(items):
            d = h if isinstance(h, dict) else {}
            has_id = isinstance(d.get("id"), str) and d.get("id") != ""
            hid = d["id"] if has_id else "#%d" % (i + 1)
            v = by_index.get(i)
            valid = bool(v and v["valid"]) and isinstance(h, dict)
            reason = "" if valid else (v["reason"] if v else "no quote-check verdict")
            if isinstance(h, dict) and not has_id:
                valid, reason = False, "missing hypothesis id"
            if valid and not HID_RE.match(hid):
                valid, reason = False, "invalid hypothesis id %r" % hid
            if valid and hid.lower() in seen:
                valid, reason = False, "duplicate hypothesis id within the unit"
            if valid:
                seen.add(hid.lower())
            ids = list(v["valid_evidence_ids"]) if valid else []
            distinct = v.get("distinct_texts", len({e.rsplit(":", 1)[-1] for e in ids})) if valid else 0
            hyps.append({"id": "%s:%s" % (uid, hid), "unit_id": uid, "hypothesis_id": hid, "claim": _s(d.get("claim")),
                         "mechanism": _s(d.get("mechanism")), "falsifier": _s(d.get("falsifier")),
                         "confidence": _s(d.get("confidence")), "quote_valid": valid, "evidence_ids": ids,
                         "distinct_texts": distinct, "status": "supported" if valid else "rejected", "reason": reason,
                         "verifier": None})
    return hyps


def apply_verifiers(hyps: list, verify: dict):
    """A complete, quote-valid `rejected` verdict about THIS hypothesis flips it; anything else keeps it."""
    for h in hyps:
        if not h["quote_valid"]:
            continue
        r = verify.get(verify_uid(h["unit_id"], h["hypothesis_id"]))
        if r is None:
            continue
        ver = {"unit_id": verify_uid(h["unit_id"], h["hypothesis_id"]), "complete": r["complete"], "verdict": None,
               "quote_valid": None, "applied": False, "reason": r["reason"]}
        if r["complete"]:
            out = r["output"]
            ver["verdict"] = _s(out.get("verdict")) or None
            ver["quote_valid"] = all(v["valid"] for v in r["verdicts"])
            meta = r.get("meta") or {}
            if out.get("hypothesis_id") != h["hypothesis_id"]:
                ver["reason"] = "verifier answered for another hypothesis"
            elif "claim_sha256" in meta and meta["claim_sha256"] != claim_hash(h):
                ver["reason"] = "verifier answered for an older claim (stale)"
            elif ver["verdict"] == "rejected" and r["verdicts"] and ver["quote_valid"]:
                ver["applied"] = True
                h["status"], h["reason"] = "rejected", "verifier rejected"
            elif ver["verdict"] == "rejected":
                ver["reason"] = "rejection ignored: its citations are not quote-valid"
        h["verifier"] = ver


def rank_key(h):
    """Most distinct excerpt texts first (a twin excerpt with the same text adds nothing), then confidence, then id."""
    return (-h["distinct_texts"], -CONFIDENCE.get(h["confidence"], 0), h["unit_id"], h["hypothesis_id"])


def unit_refusal(stage, r):
    """None when a blind or null unit can be used. Else why not: the runner's reason for an incomplete unit, or, for a
    complete unit, a missing shape (a blind unit needs a hypotheses list, a null unit a non-empty string verdict)."""
    if not r["complete"]:
        return r["reason"]
    out = r["output"]
    if stage == "blind" and not isinstance(out.get("hypotheses"), list):
        return "hypotheses is not a list"
    if stage == "null" and not (isinstance(out.get("verdict"), str) and out["verdict"].strip()):
        return "null verdict missing"
    return None


def build_report(run, intake, bundle, swarm_run) -> dict:
    blind = evaluate_stage(run, "blind", BLIND_IDS, bundle, swarm_run)
    null = evaluate_stage(run, "null", (NULL_ID,), bundle, swarm_run)
    verify = evaluate_stage(run, "verify", (), bundle, swarm_run)
    hyps = collect_hypotheses(blind)
    apply_verifiers(hyps, verify)
    supported = sorted((h for h in hyps if h["status"] == "supported"), key=rank_key)
    rejected = [h for h in hyps if h["status"] != "supported"]

    incomplete = []
    for stage, res in (("blind", blind), ("null", null)):
        for uid, r in sorted(res.items()):
            why = unit_refusal(stage, r)
            if why is not None:
                incomplete.append({"unit_id": uid, "reason": why, "stage": stage})
    nres = null[NULL_ID]
    null_verdict = _s(nres["output"].get("verdict")) or None if nres["complete"] else None
    null_valid = bool(nres["complete"] and all(v["valid"] for v in nres["verdicts"]))
    bundle_intake = bundle.get("intake") if isinstance(bundle.get("intake"), dict) else {}
    kinds = {intake["kind"], _s(bundle_intake.get("kind"))}      # either side saying visual/ux needs a human

    if kinds & HUMAN_KINDS:
        decision = "needs_human_repro"
    elif incomplete:
        decision = "incomplete"
    elif null_valid and null_verdict == "not_a_bug" and not supported:
        decision = "likely_not_a_bug"
    elif supported:
        decision = "ranked"
    else:
        decision = "no_supported_hypothesis"
    return {
        "format": "bug-investigate-report/1", "run_id": os.path.basename(run), "kind": intake["kind"],
        "decision": decision, "human_gate": HUMAN_GATE, "ranked": [h["id"] for h in supported],
        "hypotheses": supported + rejected, "null_complete": bool(nres["complete"]), "null_verdict": null_verdict,
        "null_valid": null_valid, "incomplete": incomplete,
        "verify_incomplete": [{"unit_id": uid, "reason": r["reason"]} for uid, r in sorted(verify.items())
                              if not r["complete"]],
        "verify": {"queued": len(verify), "complete": sum(1 for r in verify.values() if r["complete"]),
                   "pending": [{"unit_id": uid, "reason": NOT_RUN_REASON if r["reason"] == "no record" else r["reason"]}
                               for uid, r in sorted(verify.items()) if not r["complete"]]},
        "bundle_sha256": _s(bundle.get("bundle_sha256")),
    }


# ---------------------------------------------------------------- subcommands

def check_engine_flags(args):
    """engine real needs both --claude-bin and --yes-spend (the human spend approval); engine stub takes neither
    and needs --stub-script. Called before any stage file is written, so a refusal leaves no stage dir."""
    if args.engine == "real":
        if args.claude_bin is None or args.yes_spend is None:
            raise Fail("--engine real needs --claude-bin ABS and --yes-spend USD (the human spend approval)")
    elif args.claude_bin is not None or args.yes_spend is not None:
        raise Fail("--claude-bin and --yes-spend are for --engine real only; drop them for --engine stub")
    elif args.stub_script is None:
        raise Fail("the stub engine needs --stub-script FILE; without it every unit ends error_no_script")


def default_run_dir(kind) -> str:
    slug = re.sub(r"[^a-z0-9]+", "-", kind.lower()).strip("-") or "run"
    return os.path.join(ROOT, "%s-%s" % (utc_now().strftime("%Y%m%dT%H%M%SZ"), slug))


def cmd_new(args) -> int:
    case = read_case(args.case) if args.case else {}
    kind = args.kind if args.kind is not None else case.get("kind")
    symptom = args.symptom if args.symptom is not None else case.get("symptom_text")
    if kind not in KINDS:
        raise Fail("a kind (%s) is required, via --kind or the case file" % "|".join(KINDS))
    if not isinstance(symptom, str) or not symptom.strip():
        raise Fail("a non-empty symptom is required, via --symptom or the case file")
    symptom = symptom.strip()
    if len(symptom) > SYMPTOM_MAX:
        raise Fail("the symptom is longer than %d characters" % SYMPTOM_MAX)
    home = os.path.expanduser("~")
    if len(home) > 1 and home in symptom:
        raise Fail("the symptom contains the home directory path; remove it")
    for spec in args.log:
        if not LOG_SPEC_RE.match(spec):
            raise Fail("--log %r: expected a bare log file name, optionally NAME@N" % spec)
    for m in args.mod:
        if not MOD_RE.match(m):
            raise Fail("--mod %r: expected letters, digits, . _ - (1..64 chars)" % m)
    if args.save is not None and (not args.save.strip() or len(args.save) > SAVE_MAX or CTRL_RE.search(args.save)):
        raise Fail("--save: expected 1..%d printable characters" % SAVE_MAX)
    if args.case:
        try:
            terms = forbidden_terms(case)
        except Fail as e:
            print("warning: %s; blind and null will refuse to run" % e, file=sys.stderr)
            terms = []
        hit = next((t for t in terms if norm_terms(t) in norm_terms(symptom)), None)
        if hit is not None:                 # refused before any run dir is resolved or claimed
            raise Fail("refused: the symptom contains the case's forbidden term %r" % hit)

    run = resolve_run(args.run if args.run else default_run_dir(kind))
    if os.path.exists(run) and (not os.path.isdir(run) or os.listdir(run)):
        raise Fail("run dir %s already exists and is not empty" % run)
    claim_run_dir(run)
    intake = {"format": "bug-investigate-intake/1", "created": utc_now().strftime("%Y-%m-%dT%H:%M:%SZ"), "kind": kind,
              "symptom": symptom, "case": os.path.abspath(args.case) if args.case else None, "logs": list(args.log),
              "mods": list(args.mod), "save": args.save, "game_dir": args.game_dir}
    write_atomic(os.path.join(run, "intake.json"), dumps(intake))
    print(run)
    return 0


def cmd_bundle(args) -> int:
    run = existing_run(args.run)
    intake = load_intake(run)
    dest = os.path.join(run, "bundle.json")
    if stage_data_present(run):       # a rebuilt bundle would leave the stage units stamped with the old one
        raise Fail("this run already has stage data; create a new run with `new`")
    if args.from_file:
        try:
            with open(args.from_file, "rb") as f:
                data = f.read(MAX_BUNDLE_BYTES + 1)
            if len(data) > MAX_BUNDLE_BYTES:
                raise ValueError("file is larger than %d bytes" % MAX_BUNDLE_BYTES)
            bundle = json.loads(data.decode("utf-8"))
        except (OSError, ValueError, RecursionError) as e:
            raise Fail("cannot read bundle %s: %s" % (args.from_file, e))
        require_excerpts(verify_bundle(bundle))
        write_atomic(dest, data)
        print("bundle: imported %s (%d excerpt(s))" % (dest, len(bundle["evidence"])))
        return 0
    if not intake["logs"]:
        raise Fail("the intake records no --log; add logs with `new --log` or import a bundle with --from")
    tmp = os.path.join(run, ".bundle.json.build-%d" % os.getpid())
    cmd = [sys.executable, str(HERE / "playtest_bundle.py"), "--symptom=" + intake["symptom"], "--kind=" + intake["kind"],
           "--out=" + tmp]
    cmd += ["--log=" + x for x in intake["logs"]] + ["--mod=" + x for x in intake["mods"]]
    if intake["save"] is not None:
        cmd.append("--save=" + intake["save"])
    if intake["game_dir"]:
        cmd.append("--game-dir=" + intake["game_dir"])
    try:
        rc, err = run_builder(cmd)
        if rc != 0:
            raise Fail("playtest_bundle failed (rc=%d): %s" % (rc, err or "no message"))
        try:
            with open(tmp, "rb") as f:
                data = f.read(MAX_BUNDLE_BYTES + 1)
            bundle = json.loads(data.decode("utf-8"))
        except (OSError, ValueError, RecursionError) as e:
            raise Fail("playtest_bundle wrote no usable bundle: %s" % e)
        require_excerpts(verify_bundle(bundle))
        os.replace(tmp, dest)
    finally:
        if os.path.exists(tmp):
            os.unlink(tmp)
    print("bundle: built %s (%d excerpt(s))" % (dest, len(bundle["evidence"])))
    return 0


def _usd_text(x) -> str:
    """A USD amount rounded UP to 4 places without trailing zeros (0.33, 0.042, 1)."""
    return ("%.4f" % (math.ceil(x * 10000 - 1e-6) / 10000)).rstrip("0").rstrip(".")


def _fits(total, per_call) -> int:
    """How many calls of per_call USD fit in total USD. The 1e-9 keeps 0.30 / 0.10 at 3 (float // gives 2)."""
    return int(math.floor(total / per_call + 1e-9))


def preflight_cap(args, stage, n_pending, spent):
    """B1/R1.2: cap-total must cover the pending units' expected cost plus one per-call cap. On --engine real a short
    cap (or a need above REAL_STAGE_CEILING) is a Fail before the runner starts, unless --allow-partial (verify only)
    runs what the cap covers. On --engine stub it is a warning on stderr only."""
    need = spent + max(0, n_pending - 1) * EXPECT_CALL[stage] + args.cap_call
    short = args.cap_total + 1e-9 < need
    if args.engine != "real":
        if short:
            print("warning: %s: --cap-total %s is below the %s USD a real run would need (stub engine runs anyway)"
                  % (stage, _usd_text(args.cap_total), _usd_text(need)), file=sys.stderr)
        return
    allow = stage == "verify" and bool(getattr(args, "allow_partial", False))
    hint = " or add --allow-partial to run only what the cap covers" if stage == "verify" else ""
    if short and not allow:
        raise Fail("refused: %s stage has %d pending unit(s) and %s USD already spent; --cap-total %s is below the "
                   "minimum %s (spent + %d x %s expected + --cap-call %s). Raise --cap-total to %s%s."
                   % (stage, n_pending, _usd_text(spent), _usd_text(args.cap_total), _usd_text(need),
                      max(0, n_pending - 1), EXPECT_CALL[stage], _usd_text(args.cap_call), _usd_text(need), hint))
    if need > REAL_STAGE_CEILING + 1e-9 and not allow:
        raise Fail("refused: %s stage needs %s USD, above the real-engine ceiling of %.2f USD per stage; split the "
                   "stage into smaller runs%s." % (stage, _usd_text(need), REAL_STAGE_CEILING, hint))
    if short:
        print("partial: at most %d of %d units will run"
              % (min(max(_fits(args.cap_total - spent, args.cap_call), 0), n_pending), n_pending))


def run_units(args, run, stage, role, units: dict) -> int:
    """Write the stage's unit files, preflight the budget (real and stub), then run the stage through run_stage."""
    role_dir = ROLES / role
    if not role_dir.is_dir():
        raise Fail("role dir %s not found" % role_dir)
    units_dir = os.path.join(run, stage, "units")
    out_dir = os.path.join(run, stage, "out")
    write_units(units_dir, units)
    swarm_run = os.path.expanduser(args.swarm_run)
    n_pending = pending_units(out_dir, units, swarm_run)
    if n_pending:                                           # nothing pending means nothing to refuse
        preflight_cap(args, stage, n_pending, read_spent(out_dir, args.cap_call, swarm_run))
    rc = run_stage(str(role_dir), units_dir, out_dir, args.engine, args.cap_total, cap_call=args.cap_call,
                   stub_script=os.path.abspath(args.stub_script) if args.stub_script else None,
                   parallel=args.parallel, timeout=args.timeout, backoff_scale=args.backoff_scale,
                   swarm_run=swarm_run, claude_bin=args.claude_bin,
                   yes_spend=args.yes_spend)
    if rc not in (0, 1):
        raise StageError("swarm_run map exited %s for the %s stage" % (rc, stage))
    if rc == 1:
        print("note: swarm_run map exited 1 (a unit is incomplete or refused); run `check` to see which", file=sys.stderr)
    print("%s: %d unit(s) in %s, runner exit %d" % (stage, len(units), units_dir, rc))
    return 0


def cmd_blind_or_null(args) -> int:
    stage = args.cmd
    check_engine_flags(args)
    run = existing_run(args.run)
    intake = load_intake(run)
    bundle = load_run_bundle(run)
    terms = run_terms(intake)
    excerpts = render_excerpts(bundle)
    stamp = bundle["bundle_sha256"]
    if stage == "blind":
        units = {"blind-" + s: make_unit("blind-" + s, blind_prompt(intake["symptom"], s, excerpts, "blind-" + s),
                                         {"stage": "blind", "style": s, "bundle_sha256": stamp})
                 for s in BLIND_STYLES}
        role = "blind"
    else:
        units = {NULL_ID: make_unit(NULL_ID, null_prompt(intake["symptom"], excerpts, NULL_ID),
                                    {"stage": "null", "bundle_sha256": stamp})}
        role = "null"
    if terms is not None:                       # all units are checked before anything is written or run
        for uid, unit in sorted(units.items()):
            low = norm_terms(unit["prompt"])
            for raw in terms:
                if norm_terms(raw) in low:
                    raise Fail("refused: unit %s contains the forbidden term %r" % (uid, raw))
    return run_units(args, run, stage, role, units)


def cmd_check(args) -> int:
    run = existing_run(args.run)
    bundle = load_run_bundle(run)
    swarm_run = os.path.expanduser(args.swarm_run)
    res = evaluate_stage(run, "blind", BLIND_IDS, bundle, swarm_run)
    res.update(evaluate_stage(run, "null", (NULL_ID,), bundle, swarm_run))
    proj = {uid: {"complete": r["complete"], "reason": r["reason"], "verdicts": r["verdicts"]}
            for uid, r in sorted(res.items())}
    write_atomic(os.path.join(run, "check.json"), dumps(proj))
    for uid, r in sorted(res.items()):
        if r["complete"]:
            print("%s: complete, %d verdict(s), %d valid" % (uid, len(r["verdicts"]),
                                                             sum(1 for v in r["verdicts"] if v["valid"])))
        else:
            print("%s: INCOMPLETE (%s)" % (uid, r["reason"]))
    return 0


def cmd_verify(args) -> int:
    check_engine_flags(args)
    run = existing_run(args.run)
    intake = load_intake(run)
    bundle = load_run_bundle(run)
    blind = evaluate_stage(run, "blind", BLIND_IDS, bundle, os.path.expanduser(args.swarm_run))
    survivors = [h for h in collect_hypotheses(blind) if h["quote_valid"]]
    units = {}
    for h in survivors:
        uid = verify_uid(h["unit_id"], h["hypothesis_id"])
        excerpts = render_excerpts(bundle, set(h["evidence_ids"]))
        units[uid] = make_unit(uid, verify_prompt(intake["symptom"], excerpts, h, uid),
                               {"stage": "verify", "hypothesis_unit": h["unit_id"], "hypothesis_id": h["hypothesis_id"],
                                "bundle_sha256": bundle["bundle_sha256"], "claim_sha256": claim_hash(h)})
    if not units:
        write_units(os.path.join(run, "verify", "units"), {})
        print("verify: no surviving hypothesis, nothing to run")
        return 0
    return run_units(args, run, "verify", "verify", units)


def summary(rep: dict) -> str:
    lines = ["bug_investigate report: decision=%s kind=%s" % (rep["decision"], rep["kind"]),
             "hypotheses: %d supported, %d rejected" % (len(rep["ranked"]), len(rep["hypotheses"]) - len(rep["ranked"]))]
    by_id = {h["id"]: h for h in rep["hypotheses"]}
    for n, hid in enumerate(rep["ranked"], 1):
        h = by_id[hid]
        lines.append("  %d. %s [%s] %d evidence id(s): %s" % (n, hid, h["confidence"] or "?", len(h["evidence_ids"]),
                                                              one_line(h["claim"])[:100]))
    lines.append("null: %s (quote-valid: %s)" % (rep["null_verdict"] or ("n/a" if not rep["null_complete"] else "?"),
                                                  str(rep["null_valid"]).lower()))
    for i in rep["incomplete"]:
        lines.append("INCOMPLETE %s unit %s: %s" % (i["stage"], i["unit_id"], i["reason"]))
    v = rep["verify"]
    if 0 < v["complete"] < v["queued"]:
        lines.append("VERIFY PARTIAL: %d of %d verify units ran; the rest keep status supported without a verdict"
                     % (v["complete"], v["queued"]))
    elif v["complete"] == 0 < v["queued"]:
        lines.append("VERIFY NOT RUN")
    lines.append("human gate: " + rep["human_gate"])
    return "\n".join(lines)


def cmd_report(args) -> int:
    run = existing_run(args.run)
    intake = load_intake(run)
    bundle = load_run_bundle(run)
    rep = build_report(run, intake, bundle, os.path.expanduser(args.swarm_run))
    write_atomic(os.path.join(run, "report.json"), dumps(rep))
    print(json.dumps(rep, sort_keys=True, indent=1) if args.json else summary(rep))
    return 1 if rep["incomplete"] or rep["verify"]["pending"] else 0


# ---------------------------------------------------------------- CLI

def _usd(s):
    try:
        v = float(s)
    except ValueError:
        raise argparse.ArgumentTypeError("not a number: %r" % s)
    if not math.isfinite(v) or v <= 0:
        raise argparse.ArgumentTypeError("must be a finite number > 0: %r" % s)
    return v


def _abs_path(s):
    if not os.path.isabs(s):
        raise argparse.ArgumentTypeError("must be an absolute path: %r" % s)
    return s


def _posint(s):
    try:
        v = int(s)
    except ValueError:
        raise argparse.ArgumentTypeError("not an integer: %r" % s)
    if v < 1:
        raise argparse.ArgumentTypeError("must be >= 1: %r" % s)
    return v


def _nonneg(s):
    try:
        v = float(s)
    except ValueError:
        raise argparse.ArgumentTypeError("not a number: %r" % s)
    if not math.isfinite(v) or v < 0:
        raise argparse.ArgumentTypeError("must be a finite number >= 0: %r" % s)
    return v


def add_run(p, required=True):
    p.add_argument("--run", metavar="DIR", required=required, default=None,
                   help="run directory (never inside the repo)" + ("" if required else "; default %s/<UTC stamp>-<kind>" % ROOT))


def add_stage_opts(p):
    p.add_argument("--engine", required=True, choices=("stub", "real"),
                   help="stub, or real (needs --claude-bin and --yes-spend; any other combination exits 2)")
    p.add_argument("--cap-total", required=True, type=_usd, metavar="USD", help="total budget cap for the stage")
    p.add_argument("--cap-call", type=_usd, default=0.30, metavar="USD", help="per-call cap (default 0.30)")
    p.add_argument("--parallel", type=_posint, default=4, metavar="N", help="parallel units (default 4)")
    p.add_argument("--timeout", type=_posint, default=900, metavar="SECONDS", help="per-call timeout (default 900)")
    p.add_argument("--stub-script", metavar="FILE", help="scripted stub answers (JSON), stub engine only")
    p.add_argument("--backoff-scale", type=_nonneg, metavar="N", help="scale the rate-limit backoff (tests use 0)")
    p.add_argument("--claude-bin", type=_abs_path, metavar="ABS",
                   help="absolute path of the claude binary; required with --engine real, refused with stub")
    p.add_argument("--yes-spend", type=_usd, metavar="USD",
                   help="spend approval for --engine real; required with real, refused with stub")


def build_parser():
    ap = argparse.ArgumentParser(prog="bug_investigate.py", description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--swarm-run", default=DEFAULT_SWARM_RUN, metavar="PATH",
                    help="path of swarm_run.py (default %(default)s); explicit option, never an env var; goes BEFORE the subcommand")
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("new", help="create a run dir and intake.json")
    add_run(p, required=False)
    p.add_argument("--kind", choices=KINDS, help="symptom kind (default: the case file's kind)")
    p.add_argument("--symptom", metavar="TEXT", help="what the player saw (<= 2000 chars; default: the case's symptom_text)")
    p.add_argument("--case", metavar="FILE", help="case file (only symptom_text, kind and blind_forbidden_terms are read)")
    p.add_argument("--log", action="append", default=[], metavar="NAME[@N]",
                   help="mod log file name in the game dir, optional launch number; repeatable")
    p.add_argument("--mod", action="append", default=[], metavar="ID", help="mod id the player used; repeatable")
    p.add_argument("--save", metavar="NAME", help="save slot or name (recorded only)")
    p.add_argument("--game-dir", metavar="G", help="game dir for the bundle builder")
    p.set_defaults(func=cmd_new)

    p = sub.add_parser("bundle", help="build bundle.json with playtest_bundle.py, or import one with --from")
    add_run(p)
    p.add_argument("--from", dest="from_file", metavar="FILE", help="import a prebuilt bundle (verified first)")
    p.set_defaults(func=cmd_bundle)

    for name, text in (("blind", "build and run the 3 blind units"), ("null", "build and run the null-hypothesis unit")):
        p = sub.add_parser(name, help=text)
        add_run(p)
        add_stage_opts(p)
        p.set_defaults(func=cmd_blind_or_null)

    p = sub.add_parser("check", help="quote-check the complete blind and null outputs -> check.json")
    add_run(p)
    p.set_defaults(func=cmd_check)

    p = sub.add_parser("verify", help="build and run one verify unit per surviving hypothesis")
    add_run(p)
    add_stage_opts(p)
    p.add_argument("--allow-partial", action="store_true",
                   help="real engine: when --cap-total cannot cover every unit, run what it covers and say how many "
                        "(the rest keep status supported without a verdict)")
    p.set_defaults(func=cmd_verify)

    p = sub.add_parser("report", help="write report.json and print a summary (exit 1 while units are incomplete)")
    add_run(p)
    p.add_argument("--json", action="store_true", help="print the full report as JSON")
    p.set_defaults(func=cmd_report)
    return ap


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    try:
        if args.cmd in LOCKED_CMDS:
            with run_lock(existing_run(args.run)):
                return args.func(args)
        return args.func(args)
    except (Fail, StageError, OSError) as e:
        print("bug_investigate: error: %s" % e, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
