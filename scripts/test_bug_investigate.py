#!/usr/bin/env python3
"""Tests for scripts/bug_investigate.py (stdlib unittest; synthetic fixture bundle and a FAKE runner only).

Run: python3 -I scripts/test_bug_investigate.py
Prints a final `bug_investigate tests: N/N PASS` line and exits non-zero on any failure.

No test calls a model or the real swarm_run.py: run_stage / read_status are replaced by FakeRunner, and the
real run_stage / read_status are exercised with a patched subprocess.run. Run dirs live in temp dirs only; the
module-level ROOT is patched so nothing can reach ~/.cache/bug_investigate/ (the last test checks that too).
"""
import contextlib
import copy
import fcntl
import hashlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
import unittest
from pathlib import Path
from unittest import mock

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
REPO = HERE.parent
sys.path.insert(0, str(HERE))
import bug_investigate as bi  # noqa: E402

SCRIPT = HERE / "bug_investigate.py"
ROLES = REPO / ".claude" / "skills" / "bug-investigate" / "roles"
REAL_CASE = REPO / ".claude" / "skills" / "bug-investigate" / "cases" / "hotbar_charkey.json"
FAKE_SWARM = "/nonexistent/swarm_run.py"
CACHE_ROOT = os.path.expanduser("~/.cache/bug_investigate")
NO_STUB_SCRIPT = "the stub engine needs --stub-script FILE; without it every unit ends error_no_script"

# ---------------------------------------------------------------- fixture bundle (exact id/hash scheme)
A_LINES = [(10, "Mod init ok"), (11, "Loaded 12 entries"), (12, "WARN slot  3 missing   texture"),
           (13, "ERROR  null pointer in  handler"), (14, "Shutdown complete")]
B_LINES = [(40, "Frame hook  attached"), (41, "Queue length 0"), (42, "Tick 99 done")]
C_LINES = [(70, "retry count exceeded limit"), (71, "giving up on request")]


def sha(s):
    return hashlib.sha256(s.encode("utf-8")).hexdigest()


def make_excerpt(log, launch, kind, anchor_line, pairs):
    lines = [{"n": n, "text": t} for n, t in pairs]
    tsha = sha("\n".join(t for _, t in pairs))
    a, b = pairs[0][0], pairs[-1][0]
    return {"id": "ev:%s:L%d-%d:%s" % (log, a, b, tsha[:8]), "log": log, "launch": launch,
            "anchor": {"kind": kind, "line": anchor_line}, "start": a, "end": b,
            "lines": lines, "text_sha256": tsha, "line_cut": False}


def finalize(bundle):
    body = {k: v for k, v in bundle.items() if k != "bundle_sha256"}
    canon = json.dumps(body, sort_keys=True, separators=(",", ":"), ensure_ascii=False)
    bundle["bundle_sha256"] = sha(canon)
    return bundle


def make_bundle(kind="logic", symptom="slots fail to bind when controlling the main character", extra_pairs=None):
    evidence = [make_excerpt("playlog", 1, "alert", 13, A_LINES),
                make_excerpt("playlog", 1, "banner", 41, B_LINES),
                make_excerpt("otherlog", 2, "signature", 70, C_LINES)]
    if extra_pairs:
        evidence.append(make_excerpt("biglog", 1, "signature", 1, extra_pairs))
    return finalize({"format": "playtest-bundle/1",
                     "intake": {"symptom": symptom, "kind": kind, "mods": ["m"], "save": "s"},
                     "logs": [], "evidence": evidence, "caps": {}, "truncated": False,
                     "dropped": [], "scrub": {}, "gaps": []})


BUNDLE = make_bundle()
ID_A, ID_B, ID_C = (e["id"] for e in BUNDLE["evidence"])
Q_A1, Q_A2, Q_B, Q_C1, Q_C2 = ("Loaded 12 entries", "null pointer in handler", "Queue length 0",
                               "retry count exceeded limit", "giving up on request")

CASE = {"case_id": "t", "symptom_text": "slots fail to bind when controlling the main character", "kind": "logic",
        "true_root_cause": "TRUTHMARKER9X the real root cause", "blind_forbidden_terms": ["zzleak", "Secret Sauce"]}


# ---------------------------------------------------------------- model-output builders
def cit(eid, quote):
    return {"evidence_id": eid, "quote": quote}


def hyp(hid, cits, conf="low", **kw):
    h = {"id": hid, "claim": "claim " + hid, "mechanism": "mech " + hid,
         "evidence": [cit(e, q) for e, q in cits], "confidence": conf, "falsifier": "fals " + hid}
    h.update(kw)
    return h


def blind_out(uid, *hyps):
    return {"unit_id": uid, "hypotheses": list(hyps)}


def reason(eid, quote, text="because"):
    return {"text": text, "evidence_id": eid, "quote": quote}


def null_out(verdict, *reasons, uid="null-1"):
    return {"unit_id": uid, "verdict": verdict, "reasons": list(reasons)}


def verify_out(uid, hid, verdict, *reasons):
    return {"unit_id": uid, "hypothesis_id": hid, "verdict": verdict, "reasons": list(reasons)}


def write_json(path, obj):
    Path(path).write_text(json.dumps(obj), encoding="utf-8")


def read_json(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


# ---------------------------------------------------------------- fake runner
class FakeRunner:
    """Stands in for bi.run_stage / bi.read_status. Findings are scripted per unit id."""

    def __init__(self):
        self.findings = {}      # unit_id -> output dict (the model's structured output)
        self.incomplete = {}    # unit_id -> reason (status complete:false)
        self.stale = {}         # unit_id -> raw text written to findings/<id>.json even though incomplete
        self.stage_calls = []
        self.status_calls = []
        self.spent = 0.0        # what read_spent answers (USD already spent in the stage's out dir)
        self.rc = 0

    def run_stage(self, role_dir, units_dir, out_dir, engine, cap_total, *, cap_call, stub_script, parallel,
                  timeout, backoff_scale, swarm_run, claude_bin=None, yes_spend=None):
        ids = sorted(p.stem for p in Path(units_dir).glob("*.json"))
        self.stage_calls.append({"role_dir": role_dir, "units_dir": units_dir, "out_dir": out_dir, "engine": engine,
                                 "cap_total": cap_total, "cap_call": cap_call, "stub_script": stub_script,
                                 "parallel": parallel, "timeout": timeout, "backoff_scale": backoff_scale,
                                 "swarm_run": swarm_run, "claude_bin": claude_bin, "yes_spend": yes_spend,
                                 "units": ids})
        out = Path(out_dir)
        (out / "findings").mkdir(parents=True, exist_ok=True)
        status = {}
        for uid in ids:
            if uid in self.stale:
                (out / "findings" / (uid + ".json")).write_text(self.stale[uid], encoding="utf-8")
            if uid in self.incomplete:
                status[uid] = [False, self.incomplete[uid]]
            elif uid in self.findings:
                body = dict(self.findings[uid])
                body.setdefault("unit_id", uid)
                write_json(out / "findings" / (uid + ".json"), body)
                status[uid] = [True, None]
            else:
                status[uid] = [False, "no record"]
        write_json(out / "fake_status.json", status)
        return self.rc

    def read_status(self, out_dir, swarm_run):
        self.status_calls.append(out_dir)
        data = read_json(Path(out_dir) / "fake_status.json")
        units = [{"unit_id": u, "complete": c, "reason": r} for u, (c, r) in sorted(data.items())]
        done = sum(1 for u in units if u["complete"])
        return {"units": units, "complete": done, "total": len(units), "all_complete": done == len(units)}

    def read_spent(self, out_dir, cap_call, swarm_run):
        return self.spent

    def calls_for(self, role):
        return [c for c in self.stage_calls if c["role_dir"].rstrip("/").endswith("roles/" + role)]


def run_cli(args):
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        try:
            code = bi.main(args)
        except SystemExit as e:                       # argparse errors
            code = e.code
    return code, out.getvalue(), err.getvalue()


def tree(path):
    """All file paths below path (relative), [] when it does not exist. The run's .lock file is left out: it is the
    flock target of every locked command (TestRunLock checks it directly), not stage data."""
    p = Path(path)
    if not p.exists():
        return []
    return sorted(str(q.relative_to(p)) for q in p.rglob("*") if str(q.relative_to(p)) != ".lock")


_CACHE_BEFORE = None


def setUpModule():
    global _CACHE_BEFORE
    _CACHE_BEFORE = tree(CACHE_ROOT)


def tearDownModule():
    after = tree(CACHE_ROOT)
    if after != _CACHE_BEFORE:
        raise AssertionError("tests touched %s: %r -> %r" % (CACHE_ROOT, _CACHE_BEFORE, after))


class Base(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="bi_test_")
        self.addCleanup(shutil.rmtree, self.tmp, True)
        self.run_dir = os.path.join(self.tmp, "run")
        self.stub_script = os.path.join(self.tmp, "stub.json")      # R6.1: every stub stage needs a script
        write_json(self.stub_script, {})
        self.fake = FakeRunner()
        for name, fn in (("run_stage", self.fake.run_stage), ("read_status", self.fake.read_status)):
            p = mock.patch.object(bi, name, fn)
            p.start()
            self.addCleanup(p.stop)
        p = mock.patch.object(bi, "read_spent", self.fake.read_spent)
        p.start()
        self.addCleanup(p.stop)
        p = mock.patch.object(bi, "ROOT", os.path.join(self.tmp, "cache_root"))
        p.start()
        self.addCleanup(p.stop)

    # ---- helpers
    def cli(self, *args):
        return run_cli(["--swarm-run", FAKE_SWARM, *args])

    def write_case(self, **over):
        case = copy.deepcopy(CASE)
        case.update(over)
        for k in [k for k, v in over.items() if v is None]:
            case.pop(k)
        path = os.path.join(self.tmp, "case.json")
        write_json(path, case)
        return path

    def new_run(self, kind="logic", case=None, bundle=None, symptom="slots fail to bind when controlling the main character"):
        args = ["new", "--run", self.run_dir, "--log", "playlog.txt@1", "--mod", "m1", "--save", "s1"]
        if case:
            args += ["--case", case]
        else:
            args += ["--kind", kind, "--symptom", symptom]
        rc, out, err = self.cli(*args)
        self.assertEqual(rc, 0, err)
        self.import_bundle(bundle if bundle is not None else BUNDLE)

    def import_bundle(self, bundle):
        src = os.path.join(self.tmp, "bundle_in.json")
        write_json(src, bundle)
        rc, out, err = self.cli("bundle", "--run", self.run_dir, "--from", src)
        self.assertEqual(rc, 0, err)

    def stage(self, sub, *extra):      # R6.1: the stub engine is always given a script here
        return self.cli(sub, "--run", self.run_dir, "--engine", "stub", "--cap-total", "1.0",
                        "--stub-script", self.stub_script, *extra)

    def blind(self):
        return self.stage("blind")

    def null(self):
        return self.stage("null")

    def verify(self):
        return self.stage("verify")

    def report(self, *extra):
        return self.cli("report", "--run", self.run_dir, *extra)

    def report_json(self):
        return read_json(os.path.join(self.run_dir, "report.json"))

    def hyp_by_id(self, rep, hid):
        for h in rep["hypotheses"]:
            if h["id"] == hid:
                return h
        self.fail("no hypothesis %s in %r" % (hid, [h["id"] for h in rep["hypotheses"]]))

    def run_all(self, blind_findings, null_findings=None, kind="logic", case=None):
        """new + bundle + scripted findings + blind + null; returns nothing."""
        self.new_run(kind=kind, case=case)
        for uid, out in blind_findings.items():
            self.fake.findings[uid] = out
        self.fake.findings["null-1"] = null_findings if null_findings is not None else null_out("insufficient")
        self.assertEqual(self.blind()[0], 0)
        self.assertEqual(self.null()[0], 0)

    def empty_blind(self):
        return {u: blind_out(u) for u in ("blind-skeptic", "blind-advocate", "blind-evidence")}


# ---------------------------------------------------------------- CLI surface / hygiene
class TestHelp(unittest.TestCase):
    SUBS = ["new", "bundle", "blind", "null", "check", "verify", "report"]

    def test_help_runs_fast_without_isolation(self):
        for argv in ([], *([s] for s in self.SUBS)):
            t0 = time.time()
            p = subprocess.run([sys.executable, str(SCRIPT), *argv, "--help"], capture_output=True, text=True,
                               timeout=10, stdin=subprocess.DEVNULL, env=dict(os.environ, PYTHONDONTWRITEBYTECODE="1"))
            self.assertEqual(p.returncode, 0, (argv, p.stderr))
            self.assertIn("usage:", p.stdout)
            self.assertLess(time.time() - t0, 10)

    def test_top_help_lists_every_subcommand_and_global_option(self):
        p = subprocess.run([sys.executable, str(SCRIPT), "--help"], capture_output=True, text=True, timeout=10)
        for s in self.SUBS:
            self.assertIn(s, p.stdout)
        self.assertIn("--swarm-run", p.stdout)

    def test_help_imports_nothing_heavy(self):
        child = (
            "import os, sys\n"
            "sys.dont_write_bytecode = True\n"
            "sys.path.insert(0, sys.argv[1])\n"
            "import bug_investigate as bi\n"
            "for c in ([], ['new'], ['bundle'], ['blind'], ['null'], ['check'], ['verify'], ['report']):\n"
            "    try:\n"
            "        bi.main(c + ['--help'])\n"
            "    except SystemExit as e:\n"
            "        assert e.code == 0, (c, e.code)\n"
            "heavy = [m for m in sys.modules if m.split('.')[0] in ('evidence_quote_check', 'playtest_bundle', 'scrub',\n"
            "         'swarm_run', 'budget', 'run_record', 'schema_min', 'kotor2_logs', 'log_triage')]\n"
            "lib = os.path.expanduser('~/.claude/swarm')\n"
            "assert not heavy, heavy\n"
            "assert not [p for p in sys.path if p.startswith(lib)], sys.path\n"
            "print('CHILD-OK')\n")
        p = subprocess.run([sys.executable, "-I", "-c", child, str(HERE)], capture_output=True, text=True, timeout=30,
                           stdin=subprocess.DEVNULL)
        self.assertEqual(p.returncode, 0, p.stdout + p.stderr)
        self.assertIn("CHILD-OK", p.stdout)

    def test_blind_requires_engine_and_cap_total(self):      # R6.1: stub calls pass a script, so only argparse fails
        self.assertEqual(run_cli(["blind", "--run", "/tmp/x", "--cap-total", "1"])[0], 2)
        self.assertEqual(run_cli(["blind", "--run", "/tmp/x", "--engine", "stub", "--stub-script", "/s.json"])[0], 2)
        self.assertEqual(run_cli(["blind", "--run", "/tmp/x", "--engine", "stub", "--cap-total", "-1",
                                  "--stub-script", "/s.json"])[0], 2)
        self.assertEqual(run_cli(["blind", "--run", "/tmp/x", "--engine", "stub", "--cap-total", "nan",
                                  "--stub-script", "/s.json"])[0], 2)
        self.assertEqual(run_cli(["blind", "--engine", "stub", "--cap-total", "1", "--stub-script", "/s.json"])[0], 2)
        # --run is required

    def test_no_environment_variable_selects_a_binary_or_the_runner(self):
        src = SCRIPT.read_text(encoding="utf-8")
        self.assertNotIn("environ.get", src)
        self.assertNotIn("getenv", src)
        self.assertNotIn("environ[", src)


# ---------------------------------------------------------------- new
class TestNew(Base):
    def test_new_writes_intake_and_prints_run_dir(self):
        rc, out, err = self.cli("new", "--run", self.run_dir, "--kind", "crash", "--symptom", "it crashed",
                                "--log", "a_log.txt", "--log", "b_log.txt@2", "--mod", "m1", "--mod", "m2",
                                "--save", "slot3", "--game-dir", "/some/game")
        self.assertEqual(rc, 0, err)
        self.assertEqual(out.strip(), os.path.realpath(self.run_dir))
        intake = read_json(os.path.join(self.run_dir, "intake.json"))
        self.assertEqual((intake["kind"], intake["symptom"]), ("crash", "it crashed"))
        self.assertEqual(intake["logs"], ["a_log.txt", "b_log.txt@2"])
        self.assertEqual(intake["mods"], ["m1", "m2"])
        self.assertEqual((intake["save"], intake["game_dir"], intake["case"]), ("slot3", "/some/game", None))

    def test_new_refuses_non_empty_run_dir_but_accepts_empty_one(self):
        os.makedirs(self.run_dir)
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "s", "--log", "a_log.txt")[0], 0)
        rc, out, err = self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "s2", "--log", "a_log.txt")
        self.assertEqual(rc, 2)
        self.assertEqual(read_json(os.path.join(self.run_dir, "intake.json"))["symptom"], "s")

    def test_new_needs_symptom_or_case_and_a_kind(self):
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "logic")[0], 2)
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--symptom", "s")[0], 2)
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "bogus", "--symptom", "s")[0], 2)
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "  ")[0], 2)
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "x" * 2001)[0], 2)
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "s", "--log", "../evil")[0], 2)
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "s", "--mod", "bad mod")[0], 2)
        self.assertEqual(tree(self.run_dir), [])

    def test_new_refuses_a_symptom_that_contains_the_home_directory(self):
        home = os.path.expanduser("~")
        rc, out, err = self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom",
                                "crash after loading %s/Games/save1" % home, "--log", "a_log.txt")
        self.assertEqual(rc, 2)
        self.assertIn("home", err.lower())
        self.assertEqual(tree(self.run_dir), [])

    def test_new_with_case_takes_symptom_and_kind_and_never_reads_ground_truth(self):
        case = self.write_case()
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--case", case, "--log", "a_log.txt")[0], 0)
        intake = read_json(os.path.join(self.run_dir, "intake.json"))
        self.assertEqual(intake["symptom"], CASE["symptom_text"])
        self.assertEqual(intake["kind"], "logic")
        self.assertEqual(intake["case"], os.path.abspath(case))
        for f in tree(self.run_dir):
            p = Path(self.run_dir) / f
            if p.is_file():
                self.assertNotIn("TRUTHMARKER9X", p.read_text(encoding="utf-8"))

    def test_new_explicit_symptom_and_kind_override_the_case(self):
        case = self.write_case()
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--case", case, "--kind", "crash", "--symptom", "mine")[0], 0)
        intake = read_json(os.path.join(self.run_dir, "intake.json"))
        self.assertEqual((intake["symptom"], intake["kind"]), ("mine", "crash"))

    def test_new_unreadable_or_malformed_case_is_refused(self):
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--case", os.path.join(self.tmp, "nope.json"))[0], 2)
        bad = os.path.join(self.tmp, "bad.json")
        Path(bad).write_text("{not json", encoding="utf-8")
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--case", bad)[0], 2)
        write_json(bad, {"kind": "logic"})                       # no symptom_text, no --symptom
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--case", bad)[0], 2)
        self.assertEqual(tree(self.run_dir), [])

    def test_new_default_run_dir_is_root_stamp_and_kind(self):
        rc, out, err = run_cli(["new", "--kind", "visual", "--symptom", "wrong colour", "--log", "a_log.txt"])
        self.assertEqual(rc, 0, err)
        run = Path(out.strip())
        self.assertEqual(run.parent, Path(os.path.realpath(bi.ROOT)))
        self.assertRegex(run.name, r"^\d{8}T\d{6}Z-visual$")
        self.assertTrue((run / "intake.json").is_file())

    def test_run_dir_inside_the_repo_is_refused_and_nothing_is_created(self):
        inside = REPO / "zz_bug_investigate_guard_test_dir"
        self.addCleanup(shutil.rmtree, str(inside), True)
        rc, out, err = self.cli("new", "--run", str(inside), "--kind", "logic", "--symptom", "s", "--log", "a_log.txt")
        self.assertEqual(rc, 2)
        self.assertIn("repo", err.lower())
        self.assertFalse(inside.exists())
        link = os.path.join(self.tmp, "link")
        os.symlink(str(REPO / "scripts"), link)                  # a symlink into the repo is the same thing
        rc, out, err = self.cli("new", "--run", os.path.join(link, "zz_run"), "--kind", "logic", "--symptom", "s",
                                "--log", "a_log.txt")
        self.assertEqual(rc, 2)
        self.assertFalse((REPO / "scripts" / "zz_run").exists())
        stub_blind = ["blind", "--engine", "stub", "--cap-total", "1", "--stub-script", "/s.json"]   # R6.1
        for sub in (stub_blind, ["check"], ["report"]):
            rc, out, err = self.cli(sub[0], "--run", str(REPO / "scripts"), *sub[1:])
            self.assertEqual(rc, 2, sub)
            self.assertIn("repo", err.lower())

    def test_filesystem_errors_are_exit_2_not_a_traceback(self):
        afile = os.path.join(self.tmp, "afile")
        Path(afile).write_text("x", encoding="utf-8")
        rc, out, err = self.cli("new", "--run", os.path.join(afile, "sub"), "--kind", "logic", "--symptom", "s",
                                "--log", "a_log.txt")
        self.assertEqual(rc, 2)
        self.assertIn("error", err)

    def test_other_subcommands_need_an_existing_run(self):
        missing = os.path.join(self.tmp, "no_such_run")
        for sub in (["check"], ["report"], ["bundle"],          # R6.1: the stub blind call carries a script
                    ["blind", "--engine", "stub", "--cap-total", "1", "--stub-script", "/s.json"]):
            self.assertEqual(self.cli(sub[0], "--run", missing, *sub[1:])[0], 2, sub)
        self.assertFalse(os.path.exists(missing))


# ---------------------------------------------------------------- bundle
class TestBundle(Base):
    def new_only(self):
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "s",
                                  "--log", "a_log.txt@2", "--mod", "m1", "--save", "-5 slots", "--game-dir", "/g")[0], 0)

    def test_bundle_from_imports_a_verified_bundle(self):
        self.new_only()
        self.import_bundle(BUNDLE)
        self.assertEqual(read_json(os.path.join(self.run_dir, "bundle.json")), BUNDLE)

    def test_bundle_from_tampered_excerpt_is_refused(self):
        self.new_only()
        b = copy.deepcopy(BUNDLE)
        b["evidence"][1]["lines"][0]["text"] = "edited text"
        finalize(b)
        src = os.path.join(self.tmp, "t.json")
        write_json(src, b)
        rc, out, err = self.cli("bundle", "--run", self.run_dir, "--from", src)
        self.assertEqual(rc, 2)
        self.assertIn("tamper", err.lower())
        self.assertFalse(os.path.exists(os.path.join(self.run_dir, "bundle.json")))

    def test_bundle_from_tampered_bundle_hash_or_garbage_is_refused(self):
        self.new_only()
        b = copy.deepcopy(BUNDLE)
        b["intake"]["symptom"] = "edited without re-hashing"
        src = os.path.join(self.tmp, "t.json")
        write_json(src, b)
        self.assertEqual(self.cli("bundle", "--run", self.run_dir, "--from", src)[0], 2)
        Path(src).write_text("not json at all", encoding="utf-8")
        self.assertEqual(self.cli("bundle", "--run", self.run_dir, "--from", src)[0], 2)
        self.assertEqual(self.cli("bundle", "--run", self.run_dir, "--from", os.path.join(self.tmp, "absent.json"))[0], 2)
        self.assertFalse(os.path.exists(os.path.join(self.run_dir, "bundle.json")))
        self.assertEqual([f for f in tree(self.run_dir) if f != "intake.json"], [])      # no tmp litter

    def test_bundle_builder_runs_with_intake_args_and_result_is_verified(self):
        self.new_only()
        seen = {}

        def fake_builder(cmd):
            seen["cmd"] = cmd
            out = [a for a in cmd if a.startswith("--out=")][0][len("--out="):]
            Path(out).write_text(json.dumps(BUNDLE), encoding="utf-8")
            return 0, ""
        with mock.patch.object(bi, "run_builder", fake_builder):
            rc, out, err = self.cli("bundle", "--run", self.run_dir)
        self.assertEqual(rc, 0, err)
        cmd = seen["cmd"]
        self.assertEqual(cmd[0], sys.executable)
        self.assertTrue(cmd[1].endswith("playtest_bundle.py"))
        for want in ("--symptom=s", "--kind=logic", "--log=a_log.txt@2", "--mod=m1", "--save=-5 slots", "--game-dir=/g"):
            self.assertIn(want, cmd)
        self.assertEqual(read_json(os.path.join(self.run_dir, "bundle.json")), BUNDLE)
        self.assertEqual([f for f in tree(self.run_dir) if f not in ("intake.json", "bundle.json")], [])

    def test_bundle_builder_failure_or_tampered_result_writes_nothing(self):
        self.new_only()
        with mock.patch.object(bi, "run_builder", lambda cmd: (2, "playtest_bundle: error: game dir not found")):
            rc, out, err = self.cli("bundle", "--run", self.run_dir)
        self.assertEqual(rc, 2)
        self.assertIn("game dir not found", err)

        def tampering_builder(cmd):
            out = [a for a in cmd if a.startswith("--out=")][0][len("--out="):]
            b = copy.deepcopy(BUNDLE)
            b["evidence"][0]["lines"][0]["text"] = "forged"
            Path(out).write_text(json.dumps(b), encoding="utf-8")
            return 0, ""
        with mock.patch.object(bi, "run_builder", tampering_builder):
            rc, out, err = self.cli("bundle", "--run", self.run_dir)
        self.assertEqual(rc, 2)
        self.assertEqual([f for f in tree(self.run_dir) if f != "intake.json"], [])

    def test_bundle_builder_needs_a_log_in_the_intake(self):
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "s")[0], 0)
        called = []
        with mock.patch.object(bi, "run_builder", lambda cmd: called.append(cmd) or (0, "")):
            self.assertEqual(self.cli("bundle", "--run", self.run_dir)[0], 2)
        self.assertEqual(called, [])


# ---------------------------------------------------------------- blind / null stages
class TestStages(Base):
    def test_blind_builds_three_units_and_calls_the_runner(self):
        self.new_run()
        rc, out, err = self.cli("blind", "--run", self.run_dir, "--engine", "stub", "--cap-total", "2.5",
                                "--cap-call", "0.2", "--parallel", "3", "--timeout", "77", "--stub-script", "/s.json",
                                "--backoff-scale", "0")
        self.assertEqual(rc, 0, err)
        units = Path(self.run_dir, "blind", "units")
        self.assertEqual(sorted(p.name for p in units.iterdir()), ["blind-advocate.json", "blind-evidence.json",
                                                                    "blind-skeptic.json"])
        self.assertEqual(len(self.fake.stage_calls), 1)
        c = self.fake.stage_calls[0]
        self.assertTrue(c["role_dir"].endswith("roles/blind") and os.path.isdir(c["role_dir"]))
        self.assertEqual(c["units_dir"], str(units))
        self.assertEqual(c["out_dir"], str(Path(self.run_dir, "blind", "out")))
        self.assertEqual((c["engine"], c["cap_total"], c["cap_call"], c["parallel"], c["timeout"]),
                         ("stub", 2.5, 0.2, 3, 77))
        self.assertEqual((c["stub_script"], c["backoff_scale"], c["swarm_run"]), ("/s.json", 0.0, FAKE_SWARM))
        self.assertEqual(c["units"], ["blind-advocate", "blind-evidence", "blind-skeptic"])

    def test_blind_unit_files_hold_symptom_style_excerpts_and_reminder(self):
        self.new_run()
        self.assertEqual(self.blind()[0], 0)
        for style in ("skeptic", "advocate", "evidence"):
            uid = "blind-" + style
            unit = read_json(Path(self.run_dir, "blind", "units", uid + ".json"))
            self.assertEqual(sorted(unit), ["meta", "prompt", "unit_id"])
            self.assertEqual(unit["unit_id"], uid)
            p = unit["prompt"]
            self.assertIn("slots fail to bind when controlling the main character", p)
            frag = (ROLES / "blind" / "styles" / (style + ".md")).read_text(encoding="utf-8").strip()
            self.assertIn(frag, p)
            for other in {"skeptic", "advocate", "evidence"} - {style}:
                self.assertNotIn((ROLES / "blind" / "styles" / (other + ".md")).read_text(encoding="utf-8").strip(), p)
            self.assertIn("[%s] log=playlog launch=1 anchor=alert@L13" % ID_A, p)
            self.assertIn("[%s] log=playlog launch=1 anchor=banner@L41" % ID_B, p)
            self.assertIn("[%s] log=otherlog launch=2 anchor=signature@L70" % ID_C, p)
            self.assertIn("L13| ERROR  null pointer in  handler", p)
            self.assertIn("L71| giving up on request", p)
            self.assertIn(uid, p.splitlines()[-1])                     # the unit_id reminder is the last line
            self.assertLess(p.index("slots fail to bind"), p.index(ID_A))     # symptom first
            self.assertLessEqual(len(json.dumps(unit)), 200 * 1000)

    def test_rendered_excerpt_lines_round_trip_through_the_quote_checker(self):
        import evidence_quote_check as eqc
        self.new_run()
        self.assertEqual(self.blind()[0], 0)
        p = read_json(Path(self.run_dir, "blind", "units", "blind-skeptic.json"))["prompt"]
        line = [l for l in p.splitlines() if l.startswith("L12|")][0]
        out = blind_out("blind-skeptic", hyp("h1", [(ID_A, line)]))
        self.assertTrue(eqc.check(BUNDLE, out)[0]["valid"])

    def test_null_builds_one_unit_with_no_style(self):
        self.new_run()
        rc, out, err = self.null()
        self.assertEqual(rc, 0, err)
        self.assertEqual([p.name for p in Path(self.run_dir, "null", "units").iterdir()], ["null-1.json"])
        c = self.fake.stage_calls[0]
        self.assertTrue(c["role_dir"].endswith("roles/null"))
        self.assertEqual(c["out_dir"], str(Path(self.run_dir, "null", "out")))
        unit = read_json(Path(self.run_dir, "null", "units", "null-1.json"))
        self.assertEqual(unit["unit_id"], "null-1")
        self.assertIn(ID_A, unit["prompt"])
        self.assertIn("slots fail to bind", unit["prompt"])
        self.assertNotIn("Working style", unit["prompt"])
        self.assertIn("null-1", unit["prompt"].splitlines()[-1])

    def test_real_engine_is_refused_before_anything_is_written(self):
        self.new_run()
        before = tree(self.run_dir)
        for sub in ("blind", "null", "verify"):
            rc, out, err = self.cli(sub, "--run", self.run_dir, "--engine", "real", "--cap-total", "5")
            self.assertEqual(rc, 2, sub)
            self.assertIn("real", err)
            self.assertEqual(tree(self.run_dir), before, sub)
        self.assertEqual(self.fake.stage_calls, [])
        # also with a run dir that does not exist: still exit 2, nothing created
        gone = os.path.join(self.tmp, "gone")
        self.assertEqual(self.cli("blind", "--run", gone, "--engine", "real", "--cap-total", "5")[0], 2)
        self.assertFalse(os.path.exists(gone))

    def test_real_engine_needs_both_claude_bin_and_yes_spend(self):
        self.new_run()
        before = tree(self.run_dir)
        cases = {"no flags": [], "only --yes-spend": ["--yes-spend", "5"],
                 "only --claude-bin": ["--claude-bin", sys.executable]}
        for label, extra in cases.items():
            for sub in ("blind", "null", "verify"):
                rc, out, err = self.cli(sub, "--run", self.run_dir, "--engine", "real", "--cap-total", "5", *extra)
                self.assertEqual(rc, 2, (label, sub))
                self.assertIn("real", err)
                self.assertEqual(tree(self.run_dir), before, (label, sub))
        self.assertEqual(self.fake.stage_calls, [])

    def test_real_engine_with_both_flags_reaches_the_runner_once(self):
        self.new_run()
        rc, out, err = self.cli("blind", "--run", self.run_dir, "--engine", "real", "--cap-total", "5",
                                "--claude-bin", sys.executable, "--yes-spend", "5")
        self.assertEqual(rc, 0, err)
        self.assertEqual(len(self.fake.stage_calls), 1)
        c = self.fake.stage_calls[0]
        self.assertEqual((c["engine"], c["claude_bin"], c["yes_spend"]), ("real", sys.executable, 5.0))

    def test_stub_engine_refuses_claude_bin_and_yes_spend(self):
        self.new_run()
        before = tree(self.run_dir)
        cases = {"--yes-spend": ["--yes-spend", "5"], "--claude-bin": ["--claude-bin", sys.executable]}
        for label, extra in cases.items():
            for sub in ("blind", "null", "verify"):
                rc, out, err = self.stage(sub, *extra)
                self.assertEqual(rc, 2, (label, sub))
                self.assertIn("stub", err)
                self.assertEqual(tree(self.run_dir), before, (label, sub))
        self.assertEqual(self.fake.stage_calls, [])

    def test_stub_blind_without_stub_script_is_refused_before_anything_is_written(self):      # R6.1
        self.new_run()
        before = tree(self.run_dir)
        rc, out, err = self.cli("blind", "--run", self.run_dir, "--engine", "stub", "--cap-total", "1.0")
        self.assertEqual(rc, 2, err)
        self.assertIn(NO_STUB_SCRIPT, err)
        self.assertEqual(tree(self.run_dir), before)
        self.assertEqual(self.fake.stage_calls, [])

    def test_stub_null_without_stub_script_is_refused_before_anything_is_written(self):       # R6.1
        self.new_run()
        before = tree(self.run_dir)
        rc, out, err = self.cli("null", "--run", self.run_dir, "--engine", "stub", "--cap-total", "1.0")
        self.assertEqual(rc, 2, err)
        self.assertIn(NO_STUB_SCRIPT, err)
        self.assertEqual(tree(self.run_dir), before)
        self.assertEqual(self.fake.stage_calls, [])

    def test_stub_verify_without_stub_script_is_refused_before_anything_is_written(self):    # R6.1
        self.new_run()
        before = tree(self.run_dir)
        rc, out, err = self.cli("verify", "--run", self.run_dir, "--engine", "stub", "--cap-total", "1.0")
        self.assertEqual(rc, 2, err)
        self.assertIn(NO_STUB_SCRIPT, err)
        self.assertEqual(tree(self.run_dir), before)
        self.assertEqual(self.fake.stage_calls, [])

    def test_stub_blind_with_a_stub_script_is_accepted(self):                                # R6.1
        self.new_run()
        rc, out, err = self.cli("blind", "--run", self.run_dir, "--engine", "stub", "--cap-total", "1.0",
                                "--stub-script", "/s.json")
        self.assertEqual(rc, 0, err)
        self.assertEqual(self.fake.calls_for("blind")[0]["stub_script"], os.path.abspath("/s.json"))

    def test_refused_stub_blind_leaves_no_blind_units_dir(self):                             # R6.1
        self.new_run()
        self.assertEqual(self.cli("blind", "--run", self.run_dir, "--engine", "stub", "--cap-total", "1.0")[0], 2)
        self.assertFalse(os.path.exists(os.path.join(self.run_dir, "blind", "units")))
        self.assertFalse(os.path.exists(os.path.join(self.run_dir, "blind")))

    def test_claude_bin_must_be_absolute_and_yes_spend_positive(self):
        self.new_run()
        for extra in (["--claude-bin", "claude"], ["--claude-bin", "./claude"], ["--yes-spend", "0"],
                      ["--yes-spend", "nan"], ["--yes-spend", "-1"]):
            rc, out, err = self.cli("blind", "--run", self.run_dir, "--engine", "real", "--cap-total", "5", *extra)
            self.assertEqual(rc, 2, extra)
        self.assertEqual(self.fake.stage_calls, [])

    def test_forbidden_term_refuses_blind_and_null_and_nothing_runs(self):
        leaky = make_bundle(extra_pairs=[(1, "this line mentions ZZLeak in the log")])
        self.new_run(case=self.write_case(), bundle=leaky)
        before = tree(self.run_dir)
        for sub in ("blind", "null"):
            rc, out, err = self.stage(sub)
            self.assertEqual(rc, 2, sub)
            self.assertIn("zzleak", err.lower())
        self.assertEqual(tree(self.run_dir), before)
        self.assertFalse(os.path.exists(os.path.join(self.run_dir, "blind")))
        self.assertFalse(os.path.exists(os.path.join(self.run_dir, "null")))
        self.assertEqual(self.fake.stage_calls, [])

    def test_forbidden_term_in_the_symptom_is_caught_case_insensitively(self):
        case = self.write_case(symptom_text="it broke when the SECRET sauce kicked in")
        rc, out, err = self.cli("new", "--run", self.run_dir, "--case", case, "--log", "playlog.txt@1")
        self.assertEqual(rc, 2, err)                    # `new` refuses it before any run dir exists
        self.assertFalse(os.path.exists(self.run_dir))
        self.new_run(case=self.write_case())            # a run whose intake was edited after `new` ...
        intake_path = os.path.join(self.run_dir, "intake.json")
        intake = read_json(intake_path)
        intake["symptom"] = "it broke when the SECRET sauce kicked in"
        write_json(intake_path, intake)
        self.assertEqual(self.blind()[0], 2)            # ... is still refused by blind and null
        self.assertEqual(self.null()[0], 2)
        self.assertEqual(self.fake.stage_calls, [])

    def leak_stage_rc(self, terms, pairs):
        """A case with these forbidden terms and a bundle whose excerpt holds pairs -> (blind rc, null rc)."""
        self.new_run(case=self.write_case(blind_forbidden_terms=list(terms)), bundle=make_bundle(extra_pairs=pairs))
        return self.blind()[0], self.null()[0]

    def test_forbidden_term_matches_through_nbsp_zero_width_and_line_markers(self):
        variants = {"nbsp": [(1, "the invalid creature showed")],
                    "zero-width": [(1, "the inval​id creature showed")],
                    "line-marker": [(13, "the invalid"), (14, "creature showed")]}
        for label, pairs in variants.items():
            shutil.rmtree(self.run_dir, True)
            self.assertEqual(self.leak_stage_rc(["invalid creature"], pairs), (2, 2), label)
            self.assertFalse(os.path.exists(os.path.join(self.run_dir, "blind", "units")), label)
        self.assertEqual(self.fake.stage_calls, [])

    def test_forbidden_term_matches_joined_and_separated_spellings(self):
        for term, text in (("partycount", "party_count is 3"), ("party_count", "partycount is 3"),
                           ("one-member party", "one member_party here")):
            shutil.rmtree(self.run_dir, True)
            self.assertEqual(self.leak_stage_rc([term], [(1, text)]), (2, 2), (term, text))
        self.assertEqual(self.fake.stage_calls, [])

    def test_two_character_forbidden_term_is_refused_and_named(self):
        self.new_run(case=self.write_case(blind_forbidden_terms=["zzleak", "ab"]))
        rc, out, err = self.blind()
        self.assertEqual(rc, 2)
        self.assertIn("'ab'", err)
        self.assertEqual(self.null()[0], 2)
        self.assertFalse(os.path.exists(os.path.join(self.run_dir, "blind")))
        self.assertEqual(self.fake.stage_calls, [])

    def test_new_refuses_a_symptom_with_a_forbidden_term_and_leaves_no_run_dir(self):
        case = self.write_case()
        rc, out, err = self.cli("new", "--run", self.run_dir, "--case", case, "--log", "playlog.txt@1",
                                "--symptom", "the Secret Sauce broke")
        self.assertEqual(rc, 2, err)
        self.assertIn("Secret Sauce", err)
        self.assertFalse(os.path.exists(self.run_dir))
        case = self.write_case(symptom_text="zz_leak shows in the HUD")
        rc, out, err = self.cli("new", "--run", self.run_dir, "--case", case, "--log", "playlog.txt@1")
        self.assertEqual(rc, 2, err)
        self.assertFalse(os.path.exists(self.run_dir))

    def test_verify_prompt_with_a_forbidden_term_is_not_refused(self):
        # verify prompts hold model-written claims, so the forbidden-term check does not apply to them
        sc = self.empty_blind()
        sc["blind-skeptic"] = blind_out("blind-skeptic", hyp("h1", [(ID_A, Q_A1)], "high", claim="the zzleak claim"))
        self.run_all(sc, null_out("insufficient"), case=self.write_case())
        rc, out, err = self.verify()
        self.assertEqual(rc, 0, err)
        unit = read_json(os.path.join(self.run_dir, "verify", "units", "verify-blind-skeptic-h1.json"))
        self.assertIn("zzleak", unit["prompt"])

    def test_case_without_a_usable_forbidden_list_is_refused(self):
        for bad in (None, [], "zzleak", {"a": 1}, [1, 2], ["ok", ""], ["ok", "   "]):
            shutil.rmtree(self.run_dir, True)
            self.new_run(case=self.write_case(blind_forbidden_terms=bad))
            self.assertEqual(self.blind()[0], 2, bad)
            self.assertEqual(self.null()[0], 2, bad)
            self.assertFalse(os.path.exists(os.path.join(self.run_dir, "blind")), bad)
        self.assertEqual(self.fake.stage_calls, [])

    def test_missing_case_file_after_new_is_refused(self):
        case = self.write_case()
        self.new_run(case=case)
        os.unlink(case)
        self.assertEqual(self.blind()[0], 2)
        self.assertEqual(self.fake.stage_calls, [])

    def test_adhoc_run_without_a_case_skips_the_forbidden_check(self):
        leaky = make_bundle(extra_pairs=[(1, "this line mentions ZZLeak in the log")])
        self.new_run(bundle=leaky)
        self.assertEqual(self.blind()[0], 0)

    def test_real_case_file_is_usable_with_the_fixture_bundle(self):
        # the pilot's case file must not trip its own forbidden terms through symptom, style fragments or the reminder
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--case", str(REAL_CASE), "--log", "a_log.txt")[0], 0)
        self.import_bundle(BUNDLE)
        rc, out, err = self.blind()
        self.assertEqual(rc, 0, err)
        self.assertEqual(self.null()[0], 0)
        with open(os.path.join(self.run_dir, "intake.json"), encoding="utf-8") as f:
            self.assertNotIn("characterKeyFor", f.read())

    def test_oversized_unit_is_refused_before_writing(self):
        big = make_bundle(extra_pairs=[(1, "x" * 210000)])
        self.new_run(bundle=big)
        before = tree(self.run_dir)
        rc, out, err = self.blind()
        self.assertEqual(rc, 2)
        self.assertIn("200", err)
        self.assertEqual(tree(self.run_dir), before)
        self.assertEqual(self.fake.stage_calls, [])

    def test_stage_needs_a_verified_bundle(self):
        self.new_run()
        b = read_json(os.path.join(self.run_dir, "bundle.json"))
        b["evidence"][0]["lines"][0]["text"] = "edited after import"
        write_json(os.path.join(self.run_dir, "bundle.json"), b)
        self.assertEqual(self.blind()[0], 2)
        os.unlink(os.path.join(self.run_dir, "bundle.json"))
        self.assertEqual(self.blind()[0], 2)
        self.assertEqual(self.fake.stage_calls, [])

    def test_runner_exit_codes(self):
        self.new_run()
        self.fake.rc = 1                                            # unit incomplete / refused: tolerated
        self.assertEqual(self.blind()[0], 0)
        for rc_bad in (2, 3, 7):
            self.fake.rc = rc_bad
            rc, out, err = self.blind()
            self.assertEqual(rc, 2, rc_bad)

    def test_stage_error_from_the_runner_is_exit_2(self):
        self.new_run()

        def boom(*a, **k):
            raise bi.StageError("runner exploded")
        with mock.patch.object(bi, "run_stage", boom):
            rc, out, err = self.blind()
        self.assertEqual(rc, 2)
        self.assertIn("runner exploded", err)


# ---------------------------------------------------------------- check
class TestCheck(Base):
    def scenario(self):
        return {
            "blind-skeptic": blind_out(
                "blind-skeptic",
                hyp("h1", [(ID_A, Q_A1)], "high"),                     # valid
                hyp("h2", [(ID_A, "this was never logged anywhere")]),  # fabricated quote
                hyp("h3", [])),                                          # uncited
            "blind-advocate": blind_out("blind-advocate"),
            "blind-evidence": blind_out("blind-evidence"),
        }

    def test_check_json_shape_and_verdicts(self):
        self.run_all(self.scenario(), null_out("not_a_bug", reason(ID_B, Q_B)))
        rc, out, err = self.cli("check", "--run", self.run_dir)
        self.assertEqual(rc, 0, err)
        chk = read_json(os.path.join(self.run_dir, "check.json"))
        self.assertEqual(sorted(chk), ["blind-advocate", "blind-evidence", "blind-skeptic", "null-1"])
        for entry in chk.values():
            self.assertEqual(sorted(entry), ["complete", "reason", "verdicts"])
            self.assertIs(entry["complete"], True)
        v = {x["id"]: x for x in chk["blind-skeptic"]["verdicts"]}
        self.assertTrue(v["h1"]["valid"])
        self.assertEqual(v["h1"]["valid_evidence_ids"], [ID_A])
        self.assertFalse(v["h2"]["valid"])
        self.assertEqual(v["h2"]["valid_evidence_ids"], [])
        self.assertFalse(v["h3"]["valid"])
        self.assertTrue(all(x["valid"] for x in chk["null-1"]["verdicts"]))

    def test_check_reads_unit_state_only_through_read_status(self):
        self.run_all(self.scenario())
        self.fake.status_calls.clear()
        self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)
        self.assertEqual(sorted(self.fake.status_calls), [str(Path(self.run_dir, "blind", "out")),
                                                          str(Path(self.run_dir, "null", "out"))])

    def test_stale_findings_of_an_incomplete_unit_are_never_read(self):
        sc = self.scenario()
        self.new_run()
        for uid, out in sc.items():
            self.fake.findings[uid] = out
        self.fake.findings["null-1"] = null_out("insufficient")
        # skeptic is incomplete in status, but an older findings file (valid, even "great" hypotheses) is on disk;
        # advocate's stale file is garbage that would crash a reader
        self.fake.incomplete["blind-skeptic"] = "role_sha256 changed"
        self.fake.stale["blind-skeptic"] = json.dumps(sc["blind-skeptic"])
        self.fake.incomplete["blind-advocate"] = "no record"
        self.fake.stale["blind-advocate"] = "{{{ not json"
        self.blind()
        self.null()
        self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)
        chk = read_json(os.path.join(self.run_dir, "check.json"))
        self.assertEqual(chk["blind-skeptic"], {"complete": False, "reason": "role_sha256 changed", "verdicts": []})
        self.assertEqual(chk["blind-advocate"], {"complete": False, "reason": "no record", "verdicts": []})
        self.assertTrue(chk["blind-evidence"]["complete"])
        self.assertEqual(self.report()[0], 1)
        rep = self.report_json()
        self.assertEqual(rep["hypotheses"], [])                    # the stale hypotheses never made it in
        self.assertEqual(sorted(i["unit_id"] for i in rep["incomplete"]), ["blind-advocate", "blind-skeptic"])

    def test_complete_unit_with_missing_or_unreadable_findings_is_incomplete(self):
        self.run_all(self.scenario())
        os.unlink(os.path.join(self.run_dir, "blind", "out", "findings", "blind-skeptic.json"))
        Path(self.run_dir, "blind", "out", "findings", "blind-advocate.json").write_text("{{{", encoding="utf-8")
        self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)
        chk = read_json(os.path.join(self.run_dir, "check.json"))
        for uid in ("blind-skeptic", "blind-advocate"):
            self.assertFalse(chk[uid]["complete"], uid)
            self.assertEqual(chk[uid]["verdicts"], [])
            self.assertIn("findings", chk[uid]["reason"])

    def test_findings_for_the_wrong_unit_are_not_trusted(self):
        sc = self.scenario()
        sc["blind-skeptic"]["unit_id"] = "blind-advocate"
        self.run_all(sc)
        self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)
        chk = read_json(os.path.join(self.run_dir, "check.json"))
        self.assertFalse(chk["blind-skeptic"]["complete"])
        self.assertIn("unit_id", chk["blind-skeptic"]["reason"])

    def test_unit_that_was_never_run_is_listed_incomplete(self):
        self.new_run()
        self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)
        chk = read_json(os.path.join(self.run_dir, "check.json"))
        self.assertEqual(sorted(chk), ["blind-advocate", "blind-evidence", "blind-skeptic", "null-1"])
        self.assertTrue(all(not e["complete"] and e["verdicts"] == [] for e in chk.values()))
        self.assertEqual(self.fake.status_calls, [])               # nothing ran, so there is nothing to ask

    def test_stage_built_but_never_started_is_incomplete_not_an_error(self):
        self.new_run()

        def never_started(*a, **k):                    # the runner died before it created the out dir
            return 0

        def no_manifest(out_dir, swarm_run):
            raise bi.StageError("swarm_run status exited 2: no manifest")
        with mock.patch.object(bi, "run_stage", never_started):
            self.assertEqual(self.blind()[0], 0)
        with mock.patch.object(bi, "read_status", no_manifest):
            self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)
            chk = read_json(os.path.join(self.run_dir, "check.json"))
            self.assertEqual(chk["blind-skeptic"], {"complete": False, "reason": "stage not run", "verdicts": []})
            self.assertEqual(self.report()[0], 1)

    def test_status_error_for_a_stage_that_did_start_is_still_exit_2(self):
        self.run_all(self.scenario())

        def no_manifest(out_dir, swarm_run):
            raise bi.StageError("swarm_run status exited 2")
        with mock.patch.object(bi, "read_status", no_manifest):
            self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 2)

    def test_tampered_bundle_at_check_time_is_exit_2(self):
        self.run_all(self.scenario())
        b = read_json(os.path.join(self.run_dir, "bundle.json"))
        b["evidence"][1]["lines"][0]["text"] = "edited"
        write_json(os.path.join(self.run_dir, "bundle.json"), b)
        rc, out, err = self.cli("check", "--run", self.run_dir)
        self.assertEqual(rc, 2)
        self.assertIn("tamper", err.lower())
        self.assertFalse(os.path.exists(os.path.join(self.run_dir, "check.json")))
        self.assertEqual(self.report()[0], 2)

    def test_status_error_is_exit_2(self):
        self.run_all(self.scenario())

        def bad_status(out_dir, swarm_run):
            raise bi.StageError("status exited 2")
        with mock.patch.object(bi, "read_status", bad_status):
            rc, out, err = self.cli("check", "--run", self.run_dir)
        self.assertEqual(rc, 2)
        self.assertIn("status exited 2", err)

    def test_malformed_status_payloads_are_exit_2(self):
        self.run_all(self.scenario())
        for payload in ({}, {"units": "x"}, {"units": [{"unit_id": "blind-skeptic", "complete": "yes", "reason": None}]},
                        {"units": [{"complete": True}]}, []):
            with mock.patch.object(bi, "read_status", lambda o, s, p=payload: p):
                self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 2, payload)


# ---------------------------------------------------------------- report
class TestReport(Base):
    def test_one_supported_hypothesis_out_of_valid_fabricated_and_uncited(self):
        sc = self.empty_blind()
        sc["blind-skeptic"] = blind_out("blind-skeptic",
                                        hyp("h1", [(ID_A, Q_A1)], "high"),
                                        hyp("h2", [(ID_A, "this was never logged anywhere")]),
                                        hyp("h3", []))
        self.run_all(sc)
        rc, out, err = self.report()
        self.assertEqual(rc, 0, err)
        rep = self.report_json()
        self.assertEqual(rep["decision"], "ranked")
        self.assertEqual(rep["ranked"], ["blind-skeptic:h1"])
        h1, h2, h3 = (self.hyp_by_id(rep, "blind-skeptic:" + h) for h in ("h1", "h2", "h3"))
        self.assertEqual((h1["status"], h1["reason"]), ("supported", ""))
        self.assertEqual(h1["evidence_ids"], [ID_A])
        self.assertEqual((h1["claim"], h1["confidence"]), ("claim h1", "high"))
        self.assertEqual(h2["status"], "rejected")
        self.assertIn("not found verbatim", h2["reason"])
        self.assertEqual(h3["status"], "rejected")
        self.assertIn("no citations", h3["reason"])
        self.assertEqual(rep["human_gate"], "a human must review this report before any fix is started")
        self.assertEqual(rep["incomplete"], [])
        self.assertIn("ranked", out)
        self.assertIn("a human must review this report before any fix is started", out)

    def test_ranking_by_distinct_valid_evidence_then_confidence_then_id(self):
        sc = {
            "blind-skeptic": blind_out("blind-skeptic",
                                       hyp("h1", [(ID_A, Q_A1)], "high"),                           # 1 distinct
                                       hyp("h2", [(ID_A, Q_A1), (ID_B, Q_B)], "low")),             # 2 distinct
            "blind-advocate": blind_out("blind-advocate",
                                        hyp("h1", [(ID_A, Q_A1), (ID_C, Q_C1)], "medium"),          # 2 distinct
                                        hyp("h2", [(ID_A, Q_A1), (ID_B, Q_B), (ID_C, Q_C1)], "low")),   # 3 distinct
            "blind-evidence": blind_out("blind-evidence",
                                        hyp("h1", [(ID_A, Q_A1), (ID_A, Q_A2)], "high")),           # 1 distinct (same id twice)
        }
        self.run_all(sc)
        self.assertEqual(self.report()[0], 0)
        rep = self.report_json()
        self.assertEqual(rep["ranked"], ["blind-advocate:h2", "blind-advocate:h1", "blind-skeptic:h2",
                                         "blind-evidence:h1", "blind-skeptic:h1"])
        self.assertEqual([h["id"] for h in rep["hypotheses"] if h["status"] == "supported"], rep["ranked"])
        self.assertEqual(self.hyp_by_id(rep, "blind-evidence:h1")["evidence_ids"], [ID_A])

    def test_confidence_order_is_high_medium_low(self):
        sc = self.empty_blind()
        sc["blind-skeptic"] = blind_out("blind-skeptic",
                                        hyp("h1", [(ID_A, Q_A1)], "low"),
                                        hyp("h2", [(ID_A, Q_A1)], "high"),
                                        hyp("h3", [(ID_A, Q_A1)], "medium"))
        self.run_all(sc)
        self.assertEqual(self.report()[0], 0)
        self.assertEqual(self.report_json()["ranked"], ["blind-skeptic:h2", "blind-skeptic:h3", "blind-skeptic:h1"])

    def test_identical_text_excerpts_rank_below_different_text_excerpts(self):
        # the twin excerpt has ID_A's text (same sha8 suffix): citing both is ONE distinct text, not two
        bundle = make_bundle()
        bundle["evidence"].append(make_excerpt("otherlog", 2, "alert", 13, A_LINES))
        twin_id = bundle["evidence"][-1]["id"]
        finalize(bundle)
        sc = self.empty_blind()
        sc["blind-skeptic"] = blind_out("blind-skeptic", hyp("h1", [(ID_A, Q_A1), (twin_id, Q_A1)], "high"))
        sc["blind-advocate"] = blind_out("blind-advocate", hyp("h2", [(ID_A, Q_A1), (ID_B, Q_B)], "low"))
        self.new_run(bundle=bundle)
        self.fake.findings.update(sc)
        self.fake.findings["null-1"] = null_out("insufficient")
        self.assertEqual((self.blind()[0], self.null()[0]), (0, 0))
        self.assertEqual(self.report()[0], 0)
        rep = self.report_json()
        self.assertEqual(rep["ranked"], ["blind-advocate:h2", "blind-skeptic:h1"])
        self.assertEqual(self.hyp_by_id(rep, "blind-skeptic:h1")["distinct_texts"], 1)
        self.assertEqual(self.hyp_by_id(rep, "blind-advocate:h2")["distinct_texts"], 2)

    def test_blind_unit_with_non_list_hypotheses_is_incomplete(self):
        sc = self.empty_blind()
        sc["blind-skeptic"] = {"unit_id": "blind-skeptic", "hypotheses": "x"}
        self.run_all(sc, null_out("not_a_bug", reason(ID_B, Q_B)))
        self.assertEqual(self.report()[0], 1)
        rep = self.report_json()
        self.assertEqual(rep["decision"], "incomplete")
        self.assertEqual([i for i in rep["incomplete"] if i["unit_id"] == "blind-skeptic"],
                         [{"unit_id": "blind-skeptic", "reason": "hypotheses is not a list", "stage": "blind"}])

    def test_null_unit_without_a_string_verdict_is_incomplete(self):
        bad_nulls = ({"unit_id": "null-1", "reasons": [reason(ID_B, Q_B)]},
                     {"unit_id": "null-1", "verdict": 3, "reasons": [reason(ID_B, Q_B)]})
        for bad in bad_nulls:
            shutil.rmtree(self.run_dir, True)
            self.fake.findings.clear()
            self.run_all(self.empty_blind(), bad)
            self.assertEqual(self.report()[0], 1)
            rep = self.report_json()
            self.assertEqual([i for i in rep["incomplete"] if i["unit_id"] == "null-1"],
                             [{"unit_id": "null-1", "reason": "null verdict missing", "stage": "null"}])
            self.assertEqual(rep["decision"], "incomplete")

    def test_hypothesis_without_an_id_is_rejected_as_missing_hypothesis_id(self):
        no_id = hyp("x", [(ID_B, Q_B)], "high")
        del no_id["id"]
        sc = self.empty_blind()
        sc["blind-skeptic"] = blind_out("blind-skeptic", no_id)
        self.run_all(sc)
        self.assertEqual(self.report()[0], 0)
        rep = self.report_json()
        recs = [h for h in rep["hypotheses"] if h["unit_id"] == "blind-skeptic"]
        self.assertEqual([(h["status"], h["reason"]) for h in recs], [("rejected", "missing hypothesis id")])
        self.assertEqual((rep["ranked"], rep["decision"]), ([], "no_supported_hypothesis"))

    def test_null_with_empty_reasons_is_never_likely_not_a_bug(self):
        self.run_all(self.empty_blind(), {"unit_id": "null-1", "verdict": "not_a_bug", "reasons": []})
        self.assertEqual(self.report()[0], 0)
        rep = self.report_json()
        self.assertIs(rep["null_valid"], False)
        self.assertNotEqual(rep["decision"], "likely_not_a_bug")

    def test_null_not_a_bug_alone_is_likely_not_a_bug(self):
        self.run_all(self.empty_blind(), null_out("not_a_bug", reason(ID_B, Q_B)))
        self.assertEqual(self.report()[0], 0)
        rep = self.report_json()
        self.assertEqual(rep["decision"], "likely_not_a_bug")
        self.assertEqual((rep["null_verdict"], rep["null_valid"]), ("not_a_bug", True))
        self.assertEqual(rep["ranked"], [])

    def test_null_with_a_fabricated_reason_never_gives_likely_not_a_bug(self):
        self.run_all(self.empty_blind(), null_out("not_a_bug", reason(ID_B, Q_B), reason(ID_B, "never in the log at all")))
        self.assertEqual(self.report()[0], 0)
        rep = self.report_json()
        self.assertFalse(rep["null_valid"])
        self.assertEqual(rep["null_verdict"], "not_a_bug")
        self.assertEqual(rep["decision"], "no_supported_hypothesis")

    def test_null_not_a_bug_without_reasons_or_with_unknown_evidence_is_not_valid(self):
        for null in (null_out("not_a_bug"), null_out("not_a_bug", reason("ev:nope:L1-2:deadbeef", "something long"))):
            shutil.rmtree(self.run_dir, True)
            self.fake.findings.clear()
            self.run_all(self.empty_blind(), null)
            self.assertEqual(self.report()[0], 0)
            rep = self.report_json()
            self.assertFalse(rep["null_valid"])
            self.assertEqual(rep["decision"], "no_supported_hypothesis")

    def test_other_null_verdicts_do_not_make_likely_not_a_bug(self):
        for verdict in ("bug", "config", "engine_as_designed", "insufficient"):
            shutil.rmtree(self.run_dir, True)
            self.fake.findings.clear()
            self.run_all(self.empty_blind(), null_out(verdict, reason(ID_B, Q_B)))
            self.assertEqual(self.report()[0], 0)
            self.assertEqual(self.report_json()["decision"], "no_supported_hypothesis", verdict)

    def test_null_not_a_bug_plus_a_supported_hypothesis_is_ranked(self):
        sc = self.empty_blind()
        sc["blind-evidence"] = blind_out("blind-evidence", hyp("h1", [(ID_C, Q_C1)], "medium"))
        self.run_all(sc, null_out("not_a_bug", reason(ID_B, Q_B)))
        self.assertEqual(self.report()[0], 0)
        rep = self.report_json()
        self.assertEqual(rep["decision"], "ranked")
        self.assertEqual(rep["ranked"], ["blind-evidence:h1"])
        self.assertTrue(rep["null_valid"])

    def test_visual_and_ux_kinds_need_a_human_repro_but_still_list_hypotheses(self):
        for kind in ("visual", "ux"):
            shutil.rmtree(self.run_dir, True)
            self.fake.findings.clear()
            sc = self.empty_blind()
            sc["blind-skeptic"] = blind_out("blind-skeptic", hyp("h1", [(ID_A, Q_A1)], "high"))
            self.run_all(sc, null_out("not_a_bug", reason(ID_B, Q_B)), kind=kind)
            self.assertEqual(self.report()[0], 0, kind)
            rep = self.report_json()
            self.assertEqual(rep["decision"], "needs_human_repro", kind)
            self.assertEqual(rep["kind"], kind)
            self.assertEqual(self.hyp_by_id(rep, "blind-skeptic:h1")["status"], "supported")

    def test_bundle_kind_visual_is_honoured_even_if_the_run_says_logic(self):
        shutil.rmtree(self.run_dir, True)
        self.new_run(kind="logic", bundle=make_bundle(kind="visual"))
        for uid, o in self.empty_blind().items():
            self.fake.findings[uid] = o
        self.fake.findings["null-1"] = null_out("insufficient")
        self.blind()
        self.null()
        self.assertEqual(self.report()[0], 0)
        self.assertEqual(self.report_json()["decision"], "needs_human_repro")

    def test_missing_null_unit_makes_report_exit_1(self):
        self.new_run()
        for uid, o in self.empty_blind().items():
            self.fake.findings[uid] = o
        self.blind()                                              # null never ran
        rc, out, err = self.report()
        self.assertEqual(rc, 1)
        rep = self.report_json()
        self.assertIn("null-1", [i["unit_id"] for i in rep["incomplete"]])
        self.assertNotEqual(rep["decision"], "likely_not_a_bug")
        self.assertEqual(rep["human_gate"], "a human must review this report before any fix is started")

    def test_incomplete_null_unit_makes_report_exit_1(self):
        self.new_run()
        for uid, o in self.empty_blind().items():
            self.fake.findings[uid] = o
        self.fake.incomplete["null-1"] = "schema invalid"
        self.blind()
        self.null()
        rc, out, err = self.report()
        self.assertEqual(rc, 1)
        rep = self.report_json()
        self.assertEqual([i for i in rep["incomplete"] if i["unit_id"] == "null-1"],
                         [{"unit_id": "null-1", "reason": "schema invalid", "stage": "null"}])
        self.assertIs(rep["null_valid"], False)
        self.assertIn("schema invalid", out)

    def test_incomplete_blind_unit_makes_report_exit_1_and_never_claims_not_a_bug(self):
        self.new_run()
        for uid, o in self.empty_blind().items():
            self.fake.findings[uid] = o
        self.fake.findings["null-1"] = null_out("not_a_bug", reason(ID_B, Q_B))
        self.fake.incomplete["blind-evidence"] = "truncated"
        self.blind()
        self.null()
        rc, out, err = self.report()
        self.assertEqual(rc, 1)
        rep = self.report_json()
        self.assertEqual([i["unit_id"] for i in rep["incomplete"]], ["blind-evidence"])
        self.assertEqual(rep["decision"], "incomplete")

    def test_report_without_any_stage_run_is_incomplete(self):
        self.new_run()
        rc, out, err = self.report()
        self.assertEqual(rc, 1)
        rep = self.report_json()
        self.assertEqual(sorted(i["unit_id"] for i in rep["incomplete"]),
                         ["blind-advocate", "blind-evidence", "blind-skeptic", "null-1"])

    def test_report_json_flag_prints_the_report(self):
        self.run_all(self.empty_blind(), null_out("not_a_bug", reason(ID_B, Q_B)))
        rc, out, err = self.report("--json")
        self.assertEqual(rc, 0, err)
        self.assertEqual(json.loads(out), self.report_json())

    def test_ground_truth_never_reaches_any_written_file(self):
        sc = self.empty_blind()
        sc["blind-skeptic"] = blind_out("blind-skeptic", hyp("h1", [(ID_A, Q_A1)]))
        self.run_all(sc, case=self.write_case())
        self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)
        self.assertEqual(self.verify()[0], 0)
        rc, out, err = self.report("--json")
        self.assertNotIn("TRUTHMARKER9X", out + err)
        for f in tree(self.run_dir):
            p = Path(self.run_dir) / f
            if p.is_file():
                self.assertNotIn("TRUTHMARKER9X", p.read_text(encoding="utf-8"), f)
        self.assertEqual(self.report_json()["decision"], "ranked")

    def test_duplicate_hypothesis_ids_in_one_unit_reject_the_later_one(self):
        sc = self.empty_blind()
        sc["blind-skeptic"] = blind_out("blind-skeptic",
                                        hyp("h1", [(ID_A, Q_A1)]), hyp("H1", [(ID_B, Q_B)]), hyp("bad id!", [(ID_B, Q_B)]))
        self.run_all(sc)
        self.assertEqual(self.report()[0], 0)
        rep = self.report_json()
        self.assertEqual(rep["ranked"], ["blind-skeptic:h1"])
        self.assertEqual(self.hyp_by_id(rep, "blind-skeptic:H1")["status"], "rejected")
        self.assertIn("duplicate", self.hyp_by_id(rep, "blind-skeptic:H1")["reason"])
        self.assertEqual(self.hyp_by_id(rep, "blind-skeptic:bad id!")["status"], "rejected")


# ---------------------------------------------------------------- verify
class TestVerify(Base):
    def survivors_scenario(self):
        sc = self.empty_blind()
        sc["blind-skeptic"] = blind_out("blind-skeptic",
                                        hyp("h1", [(ID_A, Q_A1)], "high"),
                                        hyp("h2", [(ID_A, "this was never logged anywhere")]),
                                        hyp("h3", []))
        sc["blind-advocate"] = blind_out("blind-advocate", hyp("H1", [(ID_B, Q_B), (ID_C, Q_C1)], "medium"))
        return sc

    def test_verify_runs_only_for_survivors_with_the_right_units(self):
        self.run_all(self.survivors_scenario())
        rc, out, err = self.verify()
        self.assertEqual(rc, 0, err)
        calls = self.fake.calls_for("verify")
        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0]["units"], ["verify-blind-advocate-h1", "verify-blind-skeptic-h1"])
        self.assertEqual(calls[0]["out_dir"], str(Path(self.run_dir, "verify", "out")))
        self.assertEqual(calls[0]["engine"], "stub")
        for uid in calls[0]["units"]:
            self.assertRegex(uid, r"^[a-z0-9][a-z0-9_-]{0,62}$")

    def test_verify_unit_prompt_has_only_the_cited_excerpts_and_the_hypothesis(self):
        self.run_all(self.survivors_scenario())
        self.assertEqual(self.verify()[0], 0)
        unit = read_json(Path(self.run_dir, "verify", "units", "verify-blind-skeptic-h1.json"))
        self.assertEqual(sorted(unit), ["meta", "prompt", "unit_id"])
        self.assertEqual(unit["unit_id"], "verify-blind-skeptic-h1")
        p = unit["prompt"]
        self.assertIn("slots fail to bind", p)
        self.assertIn("[%s] log=playlog launch=1 anchor=alert@L13" % ID_A, p)
        self.assertNotIn(ID_B, p)
        self.assertNotIn(ID_C, p)
        for needle in ("claim h1", "mech h1", "fals h1", "hypothesis_id: h1"):
            self.assertIn(needle, p)
        self.assertNotIn("claim h2", p)
        self.assertIn("verify-blind-skeptic-h1", p.splitlines()[-1])
        adv = read_json(Path(self.run_dir, "verify", "units", "verify-blind-advocate-h1.json"))["prompt"]
        self.assertIn("hypothesis_id: H1", adv)
        self.assertIn(ID_B, adv)
        self.assertIn(ID_C, adv)
        self.assertNotIn(ID_A, adv)

    def test_valid_rejected_verdict_flips_the_hypothesis(self):
        self.run_all(self.survivors_scenario())
        self.fake.findings["verify-blind-skeptic-h1"] = verify_out(
            "verify-blind-skeptic-h1", "h1", "rejected", reason(ID_A, Q_A2))
        self.fake.findings["verify-blind-advocate-h1"] = verify_out(
            "verify-blind-advocate-h1", "H1", "supported", reason(ID_B, Q_B))
        self.assertEqual(self.verify()[0], 0)
        self.assertEqual(self.report()[0], 0)
        rep = self.report_json()
        h1 = self.hyp_by_id(rep, "blind-skeptic:h1")
        self.assertEqual(h1["status"], "rejected")
        self.assertEqual(h1["reason"], "verifier rejected")
        self.assertEqual(self.hyp_by_id(rep, "blind-advocate:H1")["status"], "supported")
        self.assertEqual(rep["ranked"], ["blind-advocate:H1"])
        self.assertEqual(rep["decision"], "ranked")

    def test_rejected_verdict_with_a_fabricated_quote_does_not_flip(self):
        self.run_all(self.survivors_scenario())
        self.fake.findings["verify-blind-skeptic-h1"] = verify_out(
            "verify-blind-skeptic-h1", "h1", "rejected", reason(ID_A, "an invented log line"))
        self.fake.findings["verify-blind-advocate-h1"] = verify_out("verify-blind-advocate-h1", "H1", "rejected")
        self.assertEqual(self.verify()[0], 0)
        self.assertEqual(self.report()[0], 0)
        rep = self.report_json()
        self.assertEqual(self.hyp_by_id(rep, "blind-skeptic:h1")["status"], "supported")
        self.assertEqual(self.hyp_by_id(rep, "blind-advocate:H1")["status"], "supported")     # no reasons => not valid
        self.assertFalse(self.hyp_by_id(rep, "blind-skeptic:h1")["verifier"]["applied"])

    def test_inconclusive_and_incomplete_verifiers_keep_the_hypothesis(self):
        self.run_all(self.survivors_scenario())
        self.fake.findings["verify-blind-skeptic-h1"] = verify_out("verify-blind-skeptic-h1", "h1", "inconclusive")
        self.fake.incomplete["verify-blind-advocate-h1"] = "timed out"
        self.assertEqual(self.verify()[0], 0)
        self.assertEqual(self.report()[0], 1)           # B1/R1.2: a verify unit with no verdict makes report exit 1
        rep = self.report_json()
        self.assertEqual(sorted(rep["ranked"]), ["blind-advocate:H1", "blind-skeptic:h1"])
        self.assertEqual(self.hyp_by_id(rep, "blind-advocate:H1")["verifier"]["complete"], False)

    def test_verifier_output_for_another_hypothesis_is_ignored(self):
        self.run_all(self.survivors_scenario())
        self.fake.findings["verify-blind-skeptic-h1"] = verify_out(
            "verify-blind-skeptic-h1", "h9", "rejected", reason(ID_A, Q_A2))
        self.assertEqual(self.verify()[0], 0)
        self.assertEqual(self.report()[0], 1)           # B1/R1.2: verify-blind-advocate-h1 has no verdict -> exit 1
        self.assertEqual(self.hyp_by_id(self.report_json(), "blind-skeptic:h1")["status"], "supported")

    def test_no_survivors_means_nothing_to_run(self):
        sc = self.empty_blind()
        sc["blind-skeptic"] = blind_out("blind-skeptic", hyp("h2", [(ID_A, "this was never logged anywhere")]), hyp("h3", []))
        self.run_all(sc)
        rc, out, err = self.verify()
        self.assertEqual(rc, 0, err)
        self.assertEqual(self.fake.calls_for("verify"), [])
        self.assertEqual(tree(os.path.join(self.run_dir, "verify", "units")), [])

    def test_verify_skips_hypotheses_of_incomplete_blind_units(self):
        sc = self.survivors_scenario()
        self.new_run()
        for uid, o in sc.items():
            self.fake.findings[uid] = o
        self.fake.incomplete["blind-advocate"] = "no record"
        self.blind()
        self.assertEqual(self.verify()[0], 0)
        self.assertEqual(self.fake.calls_for("verify")[0]["units"], ["verify-blind-skeptic-h1"])

    def test_rerun_removes_stale_verify_units(self):
        self.run_all(self.survivors_scenario())
        self.assertEqual(self.verify()[0], 0)
        self.assertEqual(len(tree(os.path.join(self.run_dir, "verify", "units"))), 2)
        sc = self.survivors_scenario()
        sc["blind-advocate"] = blind_out("blind-advocate")
        self.fake.findings.update(sc)
        self.assertEqual(self.blind()[0], 0)
        self.assertEqual(self.verify()[0], 0)
        self.assertEqual(tree(os.path.join(self.run_dir, "verify", "units")), ["verify-blind-skeptic-h1.json"])

    def test_verify_runner_errors_are_exit_2(self):
        self.run_all(self.survivors_scenario())
        self.fake.rc = 3
        self.assertEqual(self.verify()[0], 2)

    def test_verify_before_blind_is_nothing_to_run_not_a_crash(self):
        self.new_run()
        rc, out, err = self.verify()
        self.assertEqual(rc, 0, err)
        self.assertEqual(self.fake.stage_calls, [])


# ---------------------------------------------------------------- B1/R1.2 budget preflight and partial-verify visibility
class TestBudgetPreflight(Base):
    def many_survivors(self, n):
        sc = self.empty_blind()
        sc["blind-skeptic"] = blind_out("blind-skeptic", *[hyp("h%d" % i, [(ID_A, Q_A1)], "high") for i in range(1, n + 1)])
        self.run_all(sc)

    def real_verify(self, cap_total, cap_call, *extra):
        return self.cli("verify", "--run", self.run_dir, "--engine", "real", "--claude-bin", sys.executable,
                        "--yes-spend", str(cap_total), "--cap-total", str(cap_total), "--cap-call", str(cap_call), *extra)

    def finish_verify(self, n_complete, n_total):
        for i in range(1, n_complete + 1):
            self.fake.findings["verify-blind-skeptic-h%d" % i] = verify_out(
                "verify-blind-skeptic-h%d" % i, "h%d" % i, "supported", reason(ID_A, Q_A1))
        self.assertEqual(self.stage("verify")[0], 0)

    def test_real_verify_below_the_minimum_cap_is_refused_before_the_runner(self):
        self.many_survivors(8)
        rc, out, err = self.real_verify(0.10, 0.05)
        self.assertEqual(rc, 2, err)
        self.assertIn("8 pending", err)
        self.assertIn("0.33", err)
        self.assertIn("--allow-partial", err)
        self.assertEqual(self.fake.calls_for("verify"), [])

    def test_allow_partial_runs_the_stage_and_says_how_many_units_will_run(self):
        self.many_survivors(8)
        rc, out, err = self.real_verify(0.10, 0.05, "--allow-partial")
        self.assertEqual(rc, 0, err)
        self.assertEqual(len(self.fake.calls_for("verify")), 1)
        self.assertIn("partial: at most 2 of 8 units will run", out)

    def test_partial_count_is_not_fooled_by_float_division(self):
        self.many_survivors(8)
        rc, out, err = self.real_verify(0.30, 0.10, "--allow-partial")
        self.assertEqual(rc, 0, err)
        self.assertIn("partial: at most 3 of 8 units will run", out)

    def test_real_verify_with_enough_cap_is_accepted_without_a_partial_note(self):
        self.many_survivors(8)
        rc, out, err = self.real_verify(0.40, 0.05)
        self.assertEqual(rc, 0, err)
        self.assertEqual(len(self.fake.calls_for("verify")), 1)
        self.assertNotIn("partial", out)

    def test_units_already_spent_count_toward_the_minimum(self):
        self.many_survivors(3)
        self.fake.spent = 0.06
        rc, out, err = self.real_verify(0.18, 0.05)
        self.assertEqual(rc, 2, err)
        self.assertIn("0.19", err)
        self.assertEqual(self.fake.calls_for("verify"), [])
        rc, out, err = self.real_verify(0.19, 0.05)
        self.assertEqual(rc, 0, err)
        self.assertEqual(len(self.fake.calls_for("verify")), 1)

    def test_need_above_the_real_ceiling_is_refused(self):
        self.many_survivors(2)
        self.fake.spent = 0.95
        rc, out, err = self.real_verify(2.0, 0.30)
        self.assertEqual(rc, 2, err)
        self.assertIn("1.00", err)
        self.assertIn("--allow-partial", err)
        self.assertEqual(self.fake.calls_for("verify"), [])

    def test_stub_engine_is_warned_never_refused(self):      # R6.1: a script is given; only the warning is tested
        self.many_survivors(8)
        rc, out, err = self.cli("verify", "--run", self.run_dir, "--engine", "stub", "--cap-total", "0.10",
                                "--cap-call", "0.05", "--stub-script", self.stub_script)
        self.assertEqual(rc, 0, err)
        self.assertIn("warning", err)
        self.assertEqual(len(self.fake.calls_for("verify")), 1)

    def test_complete_units_do_not_count_toward_the_minimum(self):
        self.many_survivors(8)
        self.finish_verify(5, 8)                                  # 5 complete, 3 pending in the runner's status
        Path(self.run_dir, "verify", "out", "work").mkdir(parents=True)
        write_json(Path(self.run_dir, "verify", "out", "work", "manifest.json"), {})
        rc, out, err = self.real_verify(0.12, 0.05)               # 3 pending: need 0.05 + 2 x 0.04 = 0.13
        self.assertEqual(rc, 2, err)
        self.assertIn("0.13", err)
        self.assertIn("3 pending", err)
        rc, out, err = self.real_verify(0.13, 0.05)
        self.assertEqual(rc, 0, err)

    def test_real_blind_below_the_minimum_cap_is_refused(self):
        self.new_run()
        rc, out, err = self.cli("blind", "--run", self.run_dir, "--engine", "real", "--cap-total", "0.04",
                                "--cap-call", "0.03", "--claude-bin", sys.executable, "--yes-spend", "0.04")
        self.assertEqual(rc, 2, err)
        self.assertIn("0.042", err)
        self.assertEqual(self.fake.calls_for("blind"), [])

    def test_real_blind_with_the_m4_flags_is_accepted(self):
        self.new_run()
        rc, out, err = self.cli("blind", "--run", self.run_dir, "--engine", "real", "--cap-total", "0.06",
                                "--cap-call", "0.03", "--claude-bin", sys.executable, "--yes-spend", "0.06")
        self.assertEqual(rc, 0, err)
        self.assertEqual(len(self.fake.calls_for("blind")), 1)

    def test_allow_partial_is_a_verify_only_flag(self):
        self.new_run()
        rc, out, err = self.cli("blind", "--run", self.run_dir, "--engine", "stub", "--cap-total", "1",   # R6.1
                                "--stub-script", self.stub_script, "--allow-partial")
        self.assertEqual(rc, 2)
        self.assertEqual(self.fake.stage_calls, [])

    def test_partial_verify_is_visible_in_the_report_and_exits_1(self):
        self.many_survivors(8)
        self.finish_verify(5, 8)
        rc, out, err = self.report()
        self.assertEqual(rc, 1, err)
        self.assertIn("VERIFY PARTIAL: 5 of 8 verify units ran; the rest keep status supported without a verdict", out)
        v = self.report_json()["verify"]
        self.assertEqual((v["queued"], v["complete"]), (8, 5))
        self.assertEqual([p["unit_id"] for p in v["pending"]],
                         ["verify-blind-skeptic-h%d" % i for i in (6, 7, 8)])
        self.assertEqual({p["reason"] for p in v["pending"]}, {"not run (cap reached or stage unfinished)"})

    def test_all_verify_units_complete_is_exit_0_with_no_verify_line(self):
        self.many_survivors(8)
        self.finish_verify(8, 8)
        rc, out, err = self.report()
        self.assertEqual(rc, 0, err)
        self.assertNotIn("VERIFY", out)
        self.assertIn("verify", self.report_json())
        v = self.report_json()["verify"]
        self.assertEqual((v["queued"], v["complete"], v["pending"]), (8, 8, []))

    def test_verify_with_no_record_at_all_is_verify_not_run(self):
        self.many_survivors(2)
        self.assertEqual(self.stage("verify")[0], 0)
        rc, out, err = self.report()
        self.assertEqual(rc, 1, err)
        self.assertIn("VERIFY NOT RUN", out)


# ---------------------------------------------------------------- stage integrity: stamps, refusals, stale verdicts
def claim_sha(h):
    """The verify unit's claim stamp, recomputed here (not through the implementation)."""
    return sha(h["claim"] + "\x1f" + h["mechanism"] + "\x1f" + h["falsifier"])


def rebuilt_bundle():
    """A valid bundle with other content (a different intake symptom, so a different bundle_sha256)."""
    b = copy.deepcopy(BUNDLE)
    b["intake"]["symptom"] = "a rebuilt bundle with other content"
    return finalize(b)


class TestStageIntegrity(Base):
    def claim_a(self, claim="claim A"):
        return {"blind-skeptic": blind_out("blind-skeptic", hyp("h1", [(ID_A, Q_A1)], "high", claim=claim)),
                "blind-advocate": blind_out("blind-advocate"),
                "blind-evidence": blind_out("blind-evidence")}

    def unit_meta(self, stage, uid):
        return read_json(os.path.join(self.run_dir, stage, "units", uid + ".json"))["meta"]

    def write_bundle_file(self, bundle):
        write_json(os.path.join(self.run_dir, "bundle.json"), bundle)

    def test_units_carry_the_bundle_hash_and_verify_units_the_claim_hash(self):
        self.run_all(self.claim_a())
        self.assertEqual(self.verify()[0], 0)
        for uid in ("blind-advocate", "blind-evidence", "blind-skeptic"):
            self.assertEqual(self.unit_meta("blind", uid).get("bundle_sha256"), BUNDLE["bundle_sha256"], uid)
        self.assertEqual(self.unit_meta("null", "null-1").get("bundle_sha256"), BUNDLE["bundle_sha256"])
        vm = self.unit_meta("verify", "verify-blind-skeptic-h1")
        self.assertEqual(vm.get("bundle_sha256"), BUNDLE["bundle_sha256"])
        self.assertEqual(vm.get("claim_sha256"), claim_sha(hyp("h1", [], claim="claim A")))

    def test_bundle_rebuilt_with_other_content_after_blind_makes_those_units_incomplete(self):
        self.run_all(self.claim_a())
        self.write_bundle_file(rebuilt_bundle())       # `bundle` refuses now, so the rebuild is written on disk
        self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)
        chk = read_json(os.path.join(self.run_dir, "check.json"))
        for uid in ("blind-advocate", "blind-evidence", "blind-skeptic", "null-1"):
            self.assertEqual((chk[uid]["complete"], chk[uid]["reason"]), (False, "unit built from another bundle"), uid)
        self.assertEqual(self.report()[0], 1)
        rep = self.report_json()
        self.assertEqual(rep["decision"], "incomplete")
        self.assertEqual(sorted(i["unit_id"] for i in rep["incomplete"]),
                         ["blind-advocate", "blind-evidence", "blind-skeptic", "null-1"])
        self.assertEqual({i["reason"] for i in rep["incomplete"]}, {"unit built from another bundle"})
        self.assertEqual(rep["hypotheses"], [])

    def test_bundle_rewritten_with_the_same_content_keeps_every_unit_complete(self):
        self.run_all(self.claim_a())
        self.write_bundle_file(copy.deepcopy(BUNDLE))
        self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)
        chk = read_json(os.path.join(self.run_dir, "check.json"))
        for uid in ("blind-advocate", "blind-evidence", "blind-skeptic", "null-1"):
            self.assertTrue(chk[uid]["complete"], uid)

    def test_unreadable_unit_file_of_a_complete_unit_is_incomplete(self):
        self.run_all(self.claim_a())
        Path(self.run_dir, "blind", "units", "blind-skeptic.json").write_text("{not json", encoding="utf-8")
        self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)
        chk = read_json(os.path.join(self.run_dir, "check.json"))
        self.assertEqual((chk["blind-skeptic"]["complete"], chk["blind-skeptic"]["reason"]),
                         (False, "unit file unreadable"))
        self.assertTrue(chk["blind-advocate"]["complete"])

    def test_legacy_units_without_the_new_stamps_are_still_accepted(self):
        self.run_all(self.claim_a())
        self.fake.findings["verify-blind-skeptic-h1"] = verify_out("verify-blind-skeptic-h1", "h1", "rejected",
                                                                   reason(ID_A, Q_A1))
        self.assertEqual(self.verify()[0], 0)
        for stage, uid in (("blind", "blind-advocate"), ("blind", "blind-evidence"), ("blind", "blind-skeptic"),
                           ("null", "null-1"), ("verify", "verify-blind-skeptic-h1")):
            path = os.path.join(self.run_dir, stage, "units", uid + ".json")
            unit = read_json(path)
            unit["meta"].pop("bundle_sha256", None)
            unit["meta"].pop("claim_sha256", None)
            write_json(path, unit)
        self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)
        chk = read_json(os.path.join(self.run_dir, "check.json"))
        for uid in ("blind-advocate", "blind-evidence", "blind-skeptic", "null-1"):
            self.assertTrue(chk[uid]["complete"], uid)
        self.assertEqual(self.report()[0], 0)
        self.assertEqual(self.hyp_by_id(self.report_json(), "blind-skeptic:h1")["status"], "rejected")

    def test_verify_verdict_for_an_older_claim_is_not_applied(self):
        self.run_all(self.claim_a("claim A"))
        self.fake.findings["verify-blind-skeptic-h1"] = verify_out("verify-blind-skeptic-h1", "h1", "rejected",
                                                                   reason(ID_A, Q_A1))
        self.assertEqual(self.verify()[0], 0)
        self.fake.findings["blind-skeptic"] = self.claim_a("claim B")["blind-skeptic"]
        self.assertEqual(self.blind()[0], 0)           # the same hypothesis id now carries claim B
        self.assertEqual(self.report()[0], 0)
        h = self.hyp_by_id(self.report_json(), "blind-skeptic:h1")
        self.assertEqual(h["claim"], "claim B")
        self.assertEqual(h["status"], "supported")
        self.assertFalse(h["verifier"]["applied"])
        self.assertEqual(h["verifier"]["reason"], "verifier answered for an older claim (stale)")

    def test_bundle_is_refused_once_the_run_has_stage_data_and_bundle_json_stays_unchanged(self):
        self.run_all(self.claim_a())
        before = Path(self.run_dir, "bundle.json").read_bytes()
        src = os.path.join(self.tmp, "other.json")
        write_json(src, rebuilt_bundle())
        rc, out, err = self.cli("bundle", "--run", self.run_dir, "--from", src)
        self.assertEqual(rc, 2)
        self.assertIn("this run already has stage data; create a new run with `new`", err)
        self.assertEqual(Path(self.run_dir, "bundle.json").read_bytes(), before)
        called = []
        with mock.patch.object(bi, "run_builder", lambda cmd: called.append(cmd) or (0, "")):
            self.assertEqual(self.cli("bundle", "--run", self.run_dir)[0], 2)
        self.assertEqual(called, [])
        self.assertEqual(Path(self.run_dir, "bundle.json").read_bytes(), before)

    def test_bundle_is_refused_for_each_stage_location_that_holds_data(self):
        src = os.path.join(self.tmp, "other.json")
        write_json(src, rebuilt_bundle())
        for rel in (("blind", "units"), ("blind", "out"), ("null", "units"), ("null", "out"), ("verify", "units")):
            with self.subTest(rel=rel):
                shutil.rmtree(self.run_dir, True)
                self.new_run()
                os.makedirs(os.path.join(self.run_dir, *rel))
                rc, out, err = self.cli("bundle", "--run", self.run_dir, "--from", src)
                self.assertEqual(rc, 2, err)
                self.assertIn("already has stage data", err)

    def test_bundle_with_no_excerpts_is_refused_from_a_file_and_from_the_builder(self):
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "s",
                                  "--log", "a_log.txt")[0], 0)
        empty = copy.deepcopy(BUNDLE)
        empty["evidence"] = []
        finalize(empty)
        src = os.path.join(self.tmp, "empty.json")
        write_json(src, empty)
        rc, out, err = self.cli("bundle", "--run", self.run_dir, "--from", src)
        self.assertEqual(rc, 2)
        self.assertIn("bundle has no excerpts: the log is missing or the launch number does not exist", err)
        self.assertFalse(os.path.exists(os.path.join(self.run_dir, "bundle.json")))

        def empty_builder(cmd):
            out = [a for a in cmd if a.startswith("--out=")][0][len("--out="):]
            Path(out).write_text(json.dumps(empty), encoding="utf-8")
            return 0, ""
        with mock.patch.object(bi, "run_builder", empty_builder):
            rc, out, err = self.cli("bundle", "--run", self.run_dir)
        self.assertEqual(rc, 2)
        self.assertIn("bundle has no excerpts", err)
        self.assertFalse(os.path.exists(os.path.join(self.run_dir, "bundle.json")))
        self.assertEqual([f for f in tree(self.run_dir) if f != "intake.json"], [])


class TestRunLock(Base):
    def test_a_held_run_lock_refuses_every_locked_command_with_exit_2(self):
        self.new_run()
        fd = os.open(os.path.join(self.run_dir, ".lock"), os.O_RDWR | os.O_CREAT, 0o600)
        try:
            fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
            rc, out, err = self.blind()
            self.assertEqual(rc, 2, err)
            self.assertIn("another bug_investigate command is running on this run", err)
            self.assertFalse(os.path.exists(os.path.join(self.run_dir, "blind")))
            self.assertEqual(self.null()[0], 2)
            self.assertEqual(self.verify()[0], 2)
            self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 2)
            self.assertEqual(self.report()[0], 2)
            src = os.path.join(self.tmp, "other.json")
            write_json(src, rebuilt_bundle())
            self.assertEqual(self.cli("bundle", "--run", self.run_dir, "--from", src)[0], 2)
            self.assertFalse(os.path.exists(os.path.join(self.run_dir, "check.json")))
        finally:
            os.close(fd)
        self.assertEqual(self.blind()[0], 0)                              # released once the holder lets go
        self.assertEqual(self.cli("check", "--run", self.run_dir)[0], 0)  # and released again when a command ends

    def test_two_new_calls_racing_for_one_fresh_path_leave_exactly_one_run(self):
        target = os.path.realpath(self.run_dir)
        real_mkdir = os.mkdir
        rivals, busy = [], [False]

        def racing_mkdir(path, *args, **kwargs):
            real_mkdir(path, *args, **kwargs)              # this `new` creates the directory ...
            if not busy[0] and os.path.realpath(path) == target:
                busy[0] = True
                try:                                       # ... and a rival `new` runs before this one writes intake
                    rivals.append(self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "rival",
                                           "--log", "a_log.txt"))
                finally:
                    busy[0] = False

        with mock.patch.object(os, "mkdir", racing_mkdir):
            rc, out, err = self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "outer",
                                    "--log", "a_log.txt")
        self.assertEqual(len(rivals), 1, "the race was not exercised")
        rival_rc = rivals[0][0]
        self.assertEqual(sorted([rc, rival_rc]), [0, 2], (rc, err, rivals[0]))
        winner = "rival" if rival_rc == 0 else "outer"
        self.assertEqual(read_json(os.path.join(self.run_dir, "intake.json"))["symptom"], winner)


# ---------------------------------------------------------------- real run_stage / read_status (subprocess patched)
class TestRealRunner(unittest.TestCase):
    def fake_run(self, rc=0, stdout="", calls=None):
        def run(cmd, **kw):
            if calls is not None:
                calls.append((cmd, kw))
            return subprocess.CompletedProcess(cmd, rc, stdout=stdout, stderr="")
        return run

    def test_run_stage_command_line_and_clean_env(self):
        calls = []
        env = {"DR_CLAUDE_BIN": "/evil", "CLAUDE_BIN": "/evil2", "CLAUDE_CODE_EFFORT_LEVEL": "max", "KEEP_ME": "1"}
        with mock.patch.dict(os.environ, env), mock.patch.object(bi.subprocess, "run", self.fake_run(1, calls=calls)):
            rc = bi.run_stage("/r", "/u", "/o", "stub", 1.5, cap_call=0.3, stub_script="/s.json", parallel=2, timeout=60,
                              backoff_scale=0, swarm_run="/x/swarm_run.py")
        self.assertEqual(rc, 1)
        cmd, kw = calls[0]
        self.assertEqual(cmd[:4], [sys.executable, "-I", "/x/swarm_run.py", "map"])
        pairs = dict(zip(cmd[4::2], cmd[5::2]))
        self.assertEqual(pairs, {"--role-dir": "/r", "--units": "/u", "--out": "/o", "--engine": "stub",
                                 "--cap-total": "1.5", "--cap-call": "0.3", "--parallel": "2", "--timeout": "60",
                                 "--stub-script": "/s.json", "--backoff-scale": "0"})
        for k in ("DR_CLAUDE_BIN", "CLAUDE_BIN", "CLAUDE_CODE_EFFORT_LEVEL"):
            self.assertNotIn(k, kw["env"])
        self.assertEqual(kw["env"].get("KEEP_ME"), "1")

    def test_run_stage_omits_optional_flags(self):
        calls = []
        with mock.patch.object(bi.subprocess, "run", self.fake_run(0, calls=calls)):
            bi.run_stage("/r", "/u", "/o", "stub", 2.0, cap_call=0.3, stub_script=None, parallel=4, timeout=900,
                         backoff_scale=None, swarm_run="/x/swarm_run.py")
        cmd = calls[0][0]
        self.assertNotIn("--stub-script", cmd)
        self.assertNotIn("--backoff-scale", cmd)
        self.assertNotIn("--claude-bin", cmd)

    def test_read_status_ok_and_errors(self):
        payload = {"units": [{"unit_id": "a", "complete": True, "reason": None}], "complete": 1, "total": 1,
                   "all_complete": True}
        calls = []
        with mock.patch.object(bi.subprocess, "run", self.fake_run(0, json.dumps(payload), calls)):
            self.assertEqual(bi.read_status("/o", "/x/swarm_run.py"), payload)
        self.assertEqual(calls[0][0], [sys.executable, "-I", "/x/swarm_run.py", "status", "--out", "/o", "--json"])
        with mock.patch.object(bi.subprocess, "run", self.fake_run(2, "")):
            with self.assertRaises(bi.StageError):
                bi.read_status("/o", "/x/swarm_run.py")
        with mock.patch.object(bi.subprocess, "run", self.fake_run(0, "not json")):
            with self.assertRaises(bi.StageError):
                bi.read_status("/o", "/x/swarm_run.py")
        with mock.patch.object(bi.subprocess, "run", self.fake_run(0, "[1, 2]")):
            with self.assertRaises(bi.StageError):
                bi.read_status("/o", "/x/swarm_run.py")

    def test_read_spent_is_zero_before_the_first_map_and_parses_the_runner(self):
        with mock.patch.object(bi.subprocess, "run", side_effect=AssertionError("no runner call before a map")):
            self.assertEqual(bi.read_spent("/nonexistent/out", 0.05, "/x/swarm_run.py"), 0.0)
        with tempfile.TemporaryDirectory() as d:
            os.makedirs(os.path.join(d, "work"))
            Path(d, "work", "manifest.json").write_text("{}", encoding="utf-8")
            calls = []
            with mock.patch.object(bi.subprocess, "run", self.fake_run(0, "0.06\n", calls)):
                self.assertEqual(bi.read_spent(d, 0.05, "/x/swarm_run.py"), 0.06)
            self.assertEqual(calls[0][0], [sys.executable, "-I", "/x/swarm_run.py", "spent", "--out", d, "--cap-call", "0.05"])
            for rc, stdout in ((1, ""), (0, "lots\n"), (0, "nan\n"), (0, "-1\n"), (0, "")):
                with mock.patch.object(bi.subprocess, "run", self.fake_run(rc, stdout)):
                    with self.assertRaises(bi.StageError, msg=repr((rc, stdout))):
                        bi.read_spent(d, 0.05, "/x/swarm_run.py")

    def test_missing_runner_script_is_a_stage_error_not_a_traceback(self):
        with mock.patch.object(bi.subprocess, "run", side_effect=OSError("no such file")):
            with self.assertRaises(bi.StageError):
                bi.read_status("/o", "/x/swarm_run.py")
            with self.assertRaises(bi.StageError):
                bi.run_stage("/r", "/u", "/o", "stub", 1.0, cap_call=0.3, stub_script=None, parallel=1, timeout=1,
                             backoff_scale=None, swarm_run="/x/swarm_run.py")

    def real_kw(self, **over):
        kw = dict(cap_call=0.3, stub_script=None, parallel=2, timeout=60, backoff_scale=None,
                  swarm_run="/x/swarm_run.py", claude_bin="/opt/claude", yes_spend=5.0)
        kw.update(over)
        return kw

    def test_run_stage_stub_command_is_unchanged(self):
        calls = []
        with mock.patch.object(bi.subprocess, "run", self.fake_run(0, calls=calls)):
            bi.run_stage("/r", "/u", "/o", "stub", 1.5, cap_call=0.3, stub_script="/s.json", parallel=2, timeout=60,
                         backoff_scale=0, swarm_run="/x/swarm_run.py")
        self.assertEqual(len(calls), 1)                                   # no probe for the stub engine
        self.assertEqual(calls[0][0], [sys.executable, "-I", "/x/swarm_run.py", "map", "--role-dir", "/r", "--units", "/u",
                                       "--out", "/o", "--engine", "stub", "--cap-total", "1.5", "--cap-call", "0.3",
                                       "--parallel", "2", "--timeout", "60", "--stub-script", "/s.json",
                                       "--backoff-scale", "0"])

    def test_run_stage_real_probes_first_then_maps_with_the_approval(self):
        tmp = tempfile.mkdtemp(prefix="bi_real_")
        self.addCleanup(shutil.rmtree, tmp, True)
        out = os.path.join(tmp, "out")
        calls = []
        env = {"DR_CLAUDE_BIN": "/evil", "CLAUDE_BIN": "/evil2", "CLAUDE_CODE_EFFORT_LEVEL": "max", "KEEP_ME": "1"}
        with mock.patch.dict(os.environ, env), mock.patch.object(bi.subprocess, "run", self.fake_run(0, calls=calls)):
            rc = bi.run_stage("/r", "/u", out, "real", 1.5, **self.real_kw())
        self.assertEqual(rc, 0)
        self.assertEqual([c[0][3] for c in calls], ["probe", "map"])
        self.assertEqual(calls[0][0], [sys.executable, "-I", "/x/swarm_run.py", "probe", "--out", out])
        cmd = calls[1][0]
        self.assertEqual(cmd[:4], [sys.executable, "-I", "/x/swarm_run.py", "map"])
        pairs = dict(zip(cmd[4::2], cmd[5::2]))
        self.assertEqual(pairs, {"--role-dir": "/r", "--units": "/u", "--out": out, "--engine": "real",
                                 "--cap-total": "1.5", "--cap-call": "0.3", "--parallel": "2", "--timeout": "60",
                                 "--claude-bin": "/opt/claude", "--yes-spend": "5.0",
                                 "--home-probe": os.path.join(out, "work", "home_probe.json")})
        self.assertNotIn("--stub-script", cmd)
        self.assertNotIn("--backoff-scale", cmd)
        for kw in (calls[0][1], calls[1][1]):
            for k in ("DR_CLAUDE_BIN", "CLAUDE_BIN", "CLAUDE_CODE_EFFORT_LEVEL"):
                self.assertNotIn(k, kw["env"])
            self.assertEqual(kw["env"].get("KEEP_ME"), "1")

    def test_run_stage_real_probe_failure_stops_before_the_map(self):
        tmp = tempfile.mkdtemp(prefix="bi_real_")
        self.addCleanup(shutil.rmtree, tmp, True)
        out = os.path.join(tmp, "out")
        calls = []

        def run(cmd, **kw):
            calls.append(cmd)
            return subprocess.CompletedProcess(cmd, 3 if cmd[3] == "probe" else 0, stdout="", stderr="")
        with mock.patch.object(bi.subprocess, "run", run):
            with self.assertRaises(bi.StageError):
                bi.run_stage("/r", "/u", out, "real", 1.5, **self.real_kw())
        self.assertEqual([c[3] for c in calls], ["probe"])

    def test_run_stage_real_needs_both_flags_and_stub_takes_neither(self):
        calls = []
        with mock.patch.object(bi.subprocess, "run", self.fake_run(0, calls=calls)):
            for engine, over in (("real", {"claude_bin": None}), ("real", {"yes_spend": None}),
                                 ("stub", {"claude_bin": "/opt/claude", "yes_spend": None}),
                                 ("stub", {"claude_bin": None, "yes_spend": 5.0})):
                with self.assertRaises(bi.StageError, msg=(engine, over)):
                    bi.run_stage("/r", "/u", "/o", engine, 1.0, **self.real_kw(**over))
        self.assertEqual(calls, [])


# ---------------------------------------------------------------- whole CLI as a child process, tiny fake swarm_run.py
FAKE_SWARM_SCRIPT = r'''
import json, os, sys
args = sys.argv[1:]
def opt(name):
    return args[args.index(name) + 1]
if args[0] == "map":
    units, out = opt("--units"), opt("--out")
    os.makedirs(os.path.join(out, "findings"), exist_ok=True)
    seen = sorted(k for k in os.environ if k in ("DR_CLAUDE_BIN", "CLAUDE_BIN", "CLAUDE_CODE_EFFORT_LEVEL"))
    with open(os.path.join(out, "env_seen.json"), "w") as f:
        json.dump({"env": seen, "argv": args, "engine": opt("--engine")}, f)
    with open(opt("--stub-script")) as f:
        script = json.load(f)
    done = {}
    for name in sorted(os.listdir(units)):
        uid = name[:-5]
        done[uid] = uid in script
        if done[uid]:
            with open(os.path.join(out, "findings", uid + ".json"), "w") as f:
                json.dump(dict(script[uid], unit_id=uid), f)
    with open(os.path.join(out, "status.json"), "w") as f:
        json.dump(done, f)
    sys.exit(0 if all(done.values()) else 1)
if args[0] == "status":
    with open(os.path.join(opt("--out"), "status.json")) as f:
        done = json.load(f)
    units = [{"unit_id": u, "complete": c, "reason": None if c else "no record"} for u, c in sorted(done.items())]
    print(json.dumps({"units": units, "complete": sum(done.values()), "total": len(done), "all_complete": all(done.values())}))
    sys.exit(0)
sys.exit(2)
'''


class TestRoleSchemas(unittest.TestCase):
    def test_no_role_schema_carries_a_dollar_schema_key(self):
        # the claude CLI validates --json-schema offline with draft-07 only: a "$schema" of draft 2020-12 is refused
        # ("no schema with key or ref") before any model call, which made the first paid pilot fail 3 of 3 units
        for role in ("blind", "null", "verify"):
            schema = json.loads((ROLES / role / "schema.json").read_text(encoding="utf-8"))
            self.assertNotIn("$schema", schema, role)


class TestSubprocessCli(unittest.TestCase):
    """The real run_stage / read_status and `python3 -I scripts/bug_investigate.py` end to end, against a fake runner."""

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="bi_cli_")
        self.addCleanup(shutil.rmtree, self.tmp, True)
        self.fake = os.path.join(self.tmp, "fake_swarm_run.py")
        Path(self.fake).write_text(FAKE_SWARM_SCRIPT, encoding="utf-8")
        self.run_dir = os.path.join(self.tmp, "run")
        self.env = dict(os.environ, PYTHONDONTWRITEBYTECODE="1", DR_CLAUDE_BIN="/evil/claude", CLAUDE_BIN="/evil/claude2",
                        CLAUDE_CODE_EFFORT_LEVEL="max")

    def cli(self, *args, isolated=True):
        cmd = [sys.executable] + (["-I"] if isolated else []) + [str(SCRIPT), "--swarm-run", self.fake, *args]
        return subprocess.run(cmd, capture_output=True, text=True, timeout=60, env=self.env, stdin=subprocess.DEVNULL)

    def test_full_pipeline_through_real_subprocesses(self):
        bundle_file = os.path.join(self.tmp, "bundle.json")
        write_json(bundle_file, BUNDLE)
        stub = os.path.join(self.tmp, "stub.json")
        write_json(stub, {
            "blind-skeptic": blind_out("x", hyp("h1", [(ID_A, Q_A1)], "high")),
            "blind-advocate": blind_out("x"), "blind-evidence": blind_out("x"),
            "null-1": null_out("insufficient"),
            "verify-blind-skeptic-h1": verify_out("x", "h1", "supported", reason(ID_A, Q_A1))})
        p = self.cli("new", "--run", self.run_dir, "--kind", "logic", "--symptom", "slots fail", "--log", "a_log.txt")
        self.assertEqual(p.returncode, 0, p.stderr)
        self.assertEqual(self.cli("bundle", "--run", self.run_dir, "--from", bundle_file).returncode, 0)
        stage = ("--run", self.run_dir, "--engine", "stub", "--cap-total", "1.00", "--stub-script", stub, "--backoff-scale", "0")
        for sub in ("blind", "null"):
            p = self.cli(sub, *stage)
            self.assertEqual(p.returncode, 0, p.stderr)
        seen = read_json(os.path.join(self.run_dir, "blind", "out", "env_seen.json"))
        self.assertEqual(seen["env"], [])                                    # no binary / effort override reaches the runner
        self.assertEqual(seen["engine"], "stub")
        self.assertIn("--backoff-scale", seen["argv"])
        self.assertEqual(self.cli("check", "--run", self.run_dir).returncode, 0)
        self.assertEqual(self.cli("verify", *stage).returncode, 0, "verify")
        p = self.cli("report", "--run", self.run_dir, "--json")
        self.assertEqual(p.returncode, 0, p.stderr)
        rep = json.loads(p.stdout)
        self.assertEqual((rep["decision"], rep["ranked"]), ("ranked", ["blind-skeptic:h1"]))
        self.assertEqual(rep["hypotheses"][0]["verifier"]["verdict"], "supported")

    def test_incomplete_unit_through_real_status_is_report_exit_1_and_works_without_isolation(self):
        bundle_file = os.path.join(self.tmp, "bundle.json")
        write_json(bundle_file, BUNDLE)
        stub = os.path.join(self.tmp, "stub.json")
        write_json(stub, {"blind-skeptic": blind_out("x"), "blind-advocate": blind_out("x"), "null-1": null_out("insufficient")})
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "crash", "--symptom", "s", "--log", "a_log.txt",
                                  isolated=False).returncode, 0)
        self.assertEqual(self.cli("bundle", "--run", self.run_dir, "--from", bundle_file, isolated=False).returncode, 0)
        stage = ("--run", self.run_dir, "--engine", "stub", "--cap-total", "1", "--stub-script", stub)
        self.assertEqual(self.cli("blind", *stage, isolated=False).returncode, 0)     # runner exit 1 is tolerated
        self.assertEqual(self.cli("null", *stage, isolated=False).returncode, 0)
        p = self.cli("report", "--run", self.run_dir, isolated=False)
        self.assertEqual(p.returncode, 1)
        self.assertIn("blind-evidence", p.stdout)
        self.assertEqual([i["unit_id"] for i in read_json(os.path.join(self.run_dir, "report.json"))["incomplete"]],
                         ["blind-evidence"])

    def test_real_engine_through_the_cli_writes_nothing(self):
        self.assertEqual(self.cli("new", "--run", self.run_dir, "--kind", "crash", "--symptom", "s", "--log", "a_log.txt").returncode, 0)
        before = tree(self.run_dir)
        p = self.cli("blind", "--run", self.run_dir, "--engine", "real", "--cap-total", "5")
        self.assertEqual(p.returncode, 2)
        self.assertEqual(tree(self.run_dir), before)


def main():
    suite = unittest.defaultTestLoader.loadTestsFromModule(sys.modules[__name__])
    res = unittest.TextTestRunner(verbosity=1).run(suite)
    bad = len(res.failures) + len(res.errors)
    print("bug_investigate tests: %d/%d %s" % (res.testsRun - bad, res.testsRun, "PASS" if not bad else "FAIL"))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
