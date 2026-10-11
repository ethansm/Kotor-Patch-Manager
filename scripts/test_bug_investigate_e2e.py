#!/usr/bin/env python3
"""Integration e2e of the blind bug-investigation pipeline: NO fakes between the pieces.

Real scripts/bug_investigate.py -> real scripts/playtest_bundle.py on a SYNTHETIC game dir -> the landed
~/.claude/swarm/lib/swarm_run.py with --engine stub and a scripted tests/stub_claude.py -> real evidence_quote_check.
The scripted answers are built from the bundle that was just made, so the quotes are real excerpt text.

Run: python3 -I scripts/test_bug_investigate_e2e.py     ($0, offline, no model, no real log, temp dirs only)
"""
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
BI = HERE / "bug_investigate.py"
SWARM_RUN = Path.home() / ".claude/swarm/lib/swarm_run.py"
LOG_NAME = "demo_mod_log.txt"
ALERT_TEXT = "[700 f3] ERROR load failed for 'x.tga'"
BANNER_TEXT = "[200 f0] demo-mod 1.0.1 loaded; pid=2"
SYMPTOM = "the demo mod shows a missing texture after loading a save"


def synthetic_log():
    """Launch 1 = lines 1-7, launch 2 = lines 8-96 (the default launch); one alert at line 70."""
    L = ["[100 f0] demo-mod 1.0.0 loaded; pid=1"]
    L += ["[%d f1] tick %d pos=(0.5,0.5)" % (100 + i, i) for i in range(1, 5)]
    L += ["[110 f2] ERROR early failure one", "[111 f3] shutdown"]
    L += [BANNER_TEXT]
    L += ["[%d f1] tick %d pos=(1.5,1.5)" % (200 + i, i) for i in range(1, 41)]
    L += ["[%d f2] heartbeat ok slot %d" % (300 + i, i) for i in range(1, 7)]
    L += ["[%d f1] tick %d pos=(1.5,1.5)" % (400 + i, i) for i in range(1, 16)]
    L += ["[500 f3] WARN slot 7 bad value 0x1F"]
    L += ["[%d f1] tick %d pos=(1.5,1.5)" % (600 + i, i) for i in range(1, 20)]
    L += [ALERT_TEXT]
    L += ["[%d f1] tick %d pos=(1.5,1.5)" % (800 + i, i) for i in range(1, 6)]
    L += ["[900 f4] shutdown"]
    return L


def cli(*args):
    p = subprocess.run([sys.executable, "-I", str(BI), "--swarm-run", str(SWARM_RUN), *map(str, args)],
                       capture_output=True, text=True, stdin=subprocess.DEVNULL, timeout=300)
    return p.returncode, p.stdout, p.stderr


def ok_out(uid, **body):
    return {"kind": "ok", "output": dict(unit_id=uid, **body), "cost": 0.01}


def hyp(hid, cits, conf="medium"):
    return {"id": hid, "claim": "claim " + hid, "mechanism": "mechanism " + hid, "evidence": cits,
            "confidence": conf, "falsifier": "falsifier " + hid}


class E2E(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        assert SWARM_RUN.is_file(), "the landed LIB runner is missing: " + str(SWARM_RUN)

    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="bi_e2e_"))
        self.addCleanup(shutil.rmtree, str(self.tmp), True)
        self.game = self.tmp / "game"
        self.game.mkdir()
        (self.game / LOG_NAME).write_text("\n".join(synthetic_log()) + "\n")
        self.cache_marker = Path.home() / ".cache" / "bug_investigate"
        self.cache_before = sorted(os.listdir(self.cache_marker)) if self.cache_marker.is_dir() else None

    def tearDown(self):
        after = sorted(os.listdir(self.cache_marker)) if self.cache_marker.is_dir() else None
        self.assertEqual(self.cache_before, after, "the e2e must not write under ~/.cache/bug_investigate")

    def new_run(self, kind="logic"):
        run = self.tmp / "run"
        rc, out, err = cli("new", "--run", run, "--kind", kind, "--symptom", SYMPTOM, "--log", LOG_NAME,
                           "--mod", "demo-mod", "--game-dir", self.game)
        self.assertEqual(rc, 0, err)
        rc, out, err = cli("bundle", "--run", run)
        self.assertEqual(rc, 0, err)
        self.bundle = json.loads((run / "bundle.json").read_text())
        return run

    def evidence(self, kind):
        """The excerpt of this anchor kind that holds the ERROR alert line (kind alert) or the launch banner (kind banner)."""
        want = ALERT_TEXT if kind == "alert" else BANNER_TEXT
        return next(e for e in self.bundle["evidence"]
                    if e["anchor"]["kind"] == kind and any(l["text"] == want for l in e["lines"]))

    def script(self, data):
        p = self.tmp / "stub_script.json"
        p.write_text(json.dumps(data))
        return p

    def stage(self, run, name, script):
        rc, out, err = cli(name, "--run", run, "--engine", "stub", "--cap-total", "5", "--stub-script", script,
                           "--backoff-scale", "0")
        self.assertEqual(rc, 0, out + err)

    # ------------------------------------------------------------------ scenarios
    def test_bundle_is_built_by_the_real_builder(self):
        self.new_run()
        b = self.bundle
        self.assertEqual(b["logs"][0]["name"], LOG_NAME)
        self.assertEqual((b["logs"][0]["launch"], b["logs"][0]["start"]), (2, 8))
        al, bn = self.evidence("alert"), self.evidence("banner")
        self.assertEqual(al["lines"][4]["text"], ALERT_TEXT)           # context 4 lines before the anchor
        self.assertEqual(bn["lines"][0]["text"], BANNER_TEXT)
        for e in b["evidence"]:
            self.assertEqual(e["text_sha256"], hashlib.sha256("\n".join(l["text"] for l in e["lines"]).encode()).hexdigest())

    def test_full_pipeline_ranked_with_verifier(self):
        run = self.new_run()
        al, bn = self.evidence("alert"), self.evidence("banner")
        good = [{"evidence_id": al["id"], "quote": ALERT_TEXT}]
        good2 = [{"evidence_id": bn["id"], "quote": "demo-mod 1.0.1 loaded"},
                 {"evidence_id": al["id"], "quote": "ERROR load failed for 'x.tga'"}]
        fake = [{"evidence_id": al["id"], "quote": "texture cache was never initialised"}]
        script = self.script({
            "blind-skeptic": [ok_out("blind-skeptic", hypotheses=[hyp("h1", good, "low"), hyp("h2", fake, "high"),
                                                                  hyp("h3", [], "high")])],
            "blind-advocate": [ok_out("blind-advocate", hypotheses=[hyp("h1", [], "high")])],
            "blind-evidence": [ok_out("blind-evidence", hypotheses=[hyp("h1", good2, "low")])],
            "null-1": [ok_out("null-1", verdict="not_a_bug", reasons=[
                {"text": "normal log noise", "evidence_id": bn["id"], "quote": "demo-mod 1.0.1 loaded"}])],
        })
        self.stage(run, "blind", script)
        self.stage(run, "null", script)
        rc, out, err = cli("check", "--run", run)
        self.assertEqual(rc, 0, err)
        chk = json.loads((run / "check.json").read_text())
        self.assertTrue(all(v["complete"] for v in chk.values()))
        valid = {u: [x["valid"] for x in v["verdicts"]] for u, v in chk.items()}
        self.assertEqual(valid["blind-skeptic"], [True, False, False])    # valid, fabricated quote, uncited
        self.assertEqual(valid["blind-advocate"], [False])
        self.assertEqual(valid["blind-evidence"], [True])

        # verify runs only for the two survivors; one verifier rejects with a quote-valid reason
        vscript = self.script({
            "verify-blind-skeptic-h1": [ok_out("verify-blind-skeptic-h1", hypothesis_id="h1", verdict="rejected", reasons=[
                {"text": "contradicted", "evidence_id": al["id"], "quote": ALERT_TEXT}])],
            "verify-blind-evidence-h1": [ok_out("verify-blind-evidence-h1", hypothesis_id="h1", verdict="supported",
                                                reasons=[{"text": "direct", "evidence_id": al["id"], "quote": ALERT_TEXT}])],
        })
        self.stage(run, "verify", vscript)
        self.assertEqual(sorted(p.stem for p in (run / "verify" / "units").glob("*.json")),
                         ["verify-blind-evidence-h1", "verify-blind-skeptic-h1"])

        rc, out, err = cli("report", "--run", run, "--json")
        self.assertEqual(rc, 0, err)
        rep = json.loads(out)
        self.assertEqual(rep["decision"], "ranked")
        self.assertEqual(rep["ranked"], ["blind-evidence:h1"])
        by = {h["id"]: h for h in rep["hypotheses"]}
        self.assertEqual(by["blind-skeptic:h1"]["status"], "rejected")
        self.assertEqual(by["blind-skeptic:h1"]["reason"], "verifier rejected")
        self.assertEqual(by["blind-skeptic:h2"]["status"], "rejected")
        self.assertIn("not found verbatim", by["blind-skeptic:h2"]["reason"])
        self.assertEqual(by["blind-skeptic:h3"]["status"], "rejected")
        self.assertEqual(by["blind-advocate:h1"]["status"], "rejected")
        self.assertEqual(len(by["blind-evidence:h1"]["evidence_ids"]), 2)
        self.assertTrue(rep["null_valid"])
        self.assertEqual(rep["incomplete"], [])
        self.assertIn("human must review", rep["human_gate"])

    def test_null_alone_is_likely_not_a_bug(self):
        run = self.new_run()
        al, bn = self.evidence("alert"), self.evidence("banner")
        fake = [{"evidence_id": al["id"], "quote": "invented line that is not in the log"}]
        script = self.script({
            "blind-skeptic": [ok_out("blind-skeptic", hypotheses=[hyp("h1", fake)])],
            "blind-advocate": [ok_out("blind-advocate", hypotheses=[])],
            "blind-evidence": [ok_out("blind-evidence", hypotheses=[hyp("h1", [])])],
            "null-1": [ok_out("null-1", verdict="not_a_bug", reasons=[
                {"text": "launch banner only", "evidence_id": bn["id"], "quote": "demo-mod 1.0.1 loaded"}])],
        })
        self.stage(run, "blind", script)
        self.stage(run, "null", script)
        rc, out, err = cli("report", "--run", run, "--json")
        self.assertEqual(rc, 0, err)
        rep = json.loads(out)
        self.assertEqual((rep["decision"], rep["ranked"]), ("likely_not_a_bug", []))

    def test_incomplete_null_unit_makes_report_exit_1(self):
        run = self.new_run()
        al = self.evidence("alert")
        good = [{"evidence_id": al["id"], "quote": ALERT_TEXT}]
        script = self.script({
            "blind-skeptic": [ok_out("blind-skeptic", hypotheses=[hyp("h1", good)])],
            "blind-advocate": [ok_out("blind-advocate", hypotheses=[])],
            "blind-evidence": [ok_out("blind-evidence", hypotheses=[])],
            # null-1 has no script entry: the stub returns an error result, the unit stays incomplete
        })
        self.stage(run, "blind", script)
        rc, out, err = cli("null", "--run", run, "--engine", "stub", "--cap-total", "5", "--stub-script", script,
                           "--backoff-scale", "0")
        self.assertIn(rc, (0, 2), out + err)         # runner exit 1 (unit incomplete) is tolerated by the orchestrator
        rc, out, err = cli("report", "--run", run, "--json")
        self.assertEqual(rc, 1, out + err)
        rep = json.loads(out)
        self.assertEqual(rep["decision"], "incomplete")
        self.assertEqual([i["unit_id"] for i in rep["incomplete"]], ["null-1"])

    def test_visual_kind_needs_a_human_even_with_a_supported_hypothesis(self):
        run = self.new_run(kind="visual")
        al, bn = self.evidence("alert"), self.evidence("banner")
        script = self.script({
            "blind-skeptic": [ok_out("blind-skeptic", hypotheses=[hyp("h1", [{"evidence_id": al["id"], "quote": ALERT_TEXT}])])],
            "blind-advocate": [ok_out("blind-advocate", hypotheses=[])],
            "blind-evidence": [ok_out("blind-evidence", hypotheses=[])],
            "null-1": [ok_out("null-1", verdict="insufficient", reasons=[])],
        })
        self.stage(run, "blind", script)
        self.stage(run, "null", script)
        rc, out, err = cli("report", "--run", run, "--json")
        self.assertEqual(rc, 0, err)
        self.assertEqual(json.loads(out)["decision"], "needs_human_repro")

    def test_real_engine_is_refused_end_to_end(self):
        run = self.new_run()
        rc, out, err = cli("blind", "--run", run, "--engine", "real", "--cap-total", "5")
        self.assertEqual(rc, 2, out + err)
        self.assertFalse((run / "blind").exists(), "nothing may be written for a refused real engine")


if __name__ == "__main__":
    res = unittest.main(exit=False, verbosity=0).result
    n, bad = res.testsRun, len(res.failures) + len(res.errors)
    print("bug_investigate e2e: %d/%d %s" % (n - bad, n, "PASS" if not bad else "FAIL"))
    sys.exit(1 if bad else 0)
