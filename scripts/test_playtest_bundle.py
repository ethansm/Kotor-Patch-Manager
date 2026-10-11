#!/usr/bin/env python3
"""Tests for scripts/playtest_bundle.py. Fixtures are SYNTHETIC logs created in temp dirs: no real log and no real game dir is read.

Run: python3 -I scripts/test_playtest_bundle.py
The tests that build a bundle use the real ~/.claude/swarm/lib/scrub.py (the tool refuses to run without it).
"""
import contextlib
import getpass
import hashlib
import io
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.dont_write_bytecode = True  # python -I ignores PYTHONDONTWRITEBYTECODE; keep scripts/__pycache__ untouched
sys.path.insert(0, str(HERE))
import log_triage as LT  # noqa: E402
import playtest_bundle as PB  # noqa: E402

TOOL = HERE / "playtest_bundle.py"
MTIME = 1_760_000_000  # 2025-10-09T08:53:20Z
NOW = MTIME + 3600
SENTINEL = ["zzz-forbidden-sentinel"]
ID_RE = re.compile(r"^ev:(?P<log>[A-Za-z0-9._-]+):L(?P<a>\d+)-(?P<b>\d+):(?P<h>[0-9a-f]{8})$")
TIER = {"alert": 0, "banner": 1, "signature": 2}


def canon(obj):
    return json.dumps(obj, sort_keys=True, separators=(",", ":"), ensure_ascii=False)


def sha(text):
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def two_launch_lines():
    """Launch 1 = lines 1-7, launch 2 = lines 8-96 (alerts at 70 and 90, heartbeat sig at 49, shutdown at 96)."""
    L = ["[100 f0] demo-mod 1.0.0 loaded; pid=1"]
    L += ["[%d f1] tick %d pos=(0.5,0.5)" % (100 + i, i) for i in range(1, 5)]
    L += ["[110 f2] ERROR early failure one", "[111 f3] shutdown"]
    L += ["[200 f0] demo-mod 1.0.1 loaded; pid=2"]
    L += ["[%d f1] tick %d pos=(1.5,1.5)" % (200 + i, i) for i in range(1, 41)]
    L += ["[%d f2] heartbeat ok slot %d" % (300 + i, i) for i in range(1, 7)]
    L += ["[%d f1] tick %d pos=(1.5,1.5)" % (400 + i, i) for i in range(1, 16)]
    L += ["[500 f3] WARN slot 7 bad value 0x1F"]
    L += ["[%d f1] tick %d pos=(1.5,1.5)" % (600 + i, i) for i in range(1, 20)]
    L += ["[700 f3] ERROR load failed for 'x.tga'"]
    L += ["[%d f1] tick %d pos=(1.5,1.5)" % (800 + i, i) for i in range(1, 6)]
    L += ["[900 f4] shutdown"]
    return L


def heavy_lines(tag):
    """Many distinct alerts (triage caps them at 10) plus 5 distinct repeated notes: more candidates than items_per_log."""
    L = ["[1 f0] %s 1.0.0 loaded; pid=1" % tag]
    for i in range(30):
        word = chr(97 + i % 26) + chr(97 + i // 26)
        L.append("[%d f1] ERROR fail-%s thing" % (10 + i, word))
        L += ["[%d f1] tick %d pos=(0.5,0.5)" % (50 + j, j) for j in range(11)]
    for k in range(5):
        word = "abcde"[k] * 3
        for _ in range(2):
            L.append("[2 f2] note-%s ready" % word)
            L += ["[%d f1] tick %d pos=(0.5,0.5)" % (60 + j, j) for j in range(10)]
    return L


class Base(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="playtest_bundle_test_")
        self.addCleanup(shutil.rmtree, self.tmp, ignore_errors=True)
        self.gd = os.path.join(self.tmp, "game")
        os.mkdir(self.gd)
        self.outdir = os.path.join(self.tmp, "out")
        os.mkdir(self.outdir)
        self.out = os.path.join(self.outdir, "bundle.json")

    def mk(self, name, lines, mtime=MTIME, trailing_newline=True):
        p = os.path.join(self.gd, name)
        with open(p, "w", encoding="utf-8", newline="") as f:
            f.write("\n".join(lines) + ("\n" if trailing_newline and lines else ""))
        os.utime(p, (mtime, mtime))
        return p

    def cli(self, *extra, symptom="the hotbar froze after loading", kind="logic", logs=("demo_log.txt",), out=None,
            game_dir=None, forbidden=SENTINEL, now=NOW, **inj):
        argv = ["--symptom", symptom, "--kind", kind, "--game-dir", game_dir or self.gd, "--out", out or self.out]
        for lg in logs:
            argv += ["--log", lg]
        argv += list(extra)
        so, se = io.StringIO(), io.StringIO()
        kw = dict(_forbidden=forbidden, _now=lambda: now)
        kw.update(inj)
        with contextlib.redirect_stdout(so), contextlib.redirect_stderr(se):
            try:
                rc = PB.main(argv, **kw)
            except SystemExit as e:
                rc = e.code
        return rc, so.getvalue(), se.getvalue()

    def build(self, *extra, **kw):
        rc, so, se = self.cli(*extra, **kw)
        self.assertEqual(rc, 0, se)
        with open(kw.get("out") or self.out, "rb") as f:
            raw = f.read()
        return json.loads(raw.decode("utf-8")), raw

    def assert_refused(self, *extra, expect=None, **kw):
        out = kw.get("out") or self.out
        rc, so, se = self.cli(*extra, **kw)
        self.assertEqual(rc, 2, (so, se))
        self.assertTrue(se.strip(), "message on stderr expected")
        self.assertFalse(os.path.exists(out), "no output file may be written on refusal")
        self.assertEqual([n for n in os.listdir(self.outdir)], [], "no temp leftovers")
        if expect:
            self.assertIn(expect, se)
        return se


class Contract(Base):
    def test_shape_and_intake(self):
        self.mk("demo_log.txt", two_launch_lines())
        b, raw = self.build("--mod", "z-mod", "--mod", "a-mod", "--mod", "a-mod", "--save", "slot 3")
        self.assertEqual(b["format"], "playtest-bundle/1")
        self.assertEqual(b["intake"], {"symptom": "the hotbar froze after loading", "kind": "logic",
                                       "mods": ["a-mod", "z-mod"], "save": "slot 3"})
        self.assertEqual(b["caps"], {"context": 4, "item_lines": 30, "line_chars": 240, "items_per_log": 12,
                                     "max_items": 40, "max_bytes": 49152})
        self.assertEqual(set(b), {"format", "intake", "logs", "evidence", "caps", "truncated", "dropped", "scrub", "gaps",
                                  "bundle_sha256"})
        self.assertFalse(b["truncated"])
        self.assertEqual(b["dropped"], [])
        self.assertTrue(raw.endswith(b"}\n"))
        self.assertLessEqual(len(raw), 49152)

    def test_intake_defaults(self):
        self.mk("demo_log.txt", two_launch_lines())
        b, _ = self.build()
        self.assertEqual(b["intake"]["mods"], [])
        self.assertIsNone(b["intake"]["save"])

    def test_log_entry_fields_and_launch_selection(self):
        p = self.mk("demo_log.txt", two_launch_lines())
        b, _ = self.build()
        self.assertEqual(len(b["logs"]), 1)
        e = b["logs"][0]
        self.assertEqual(set(e), {"name", "parser", "launch", "start", "end", "total_lines", "verdict_word", "alerts_total",
                                  "file_sha256", "mtime_utc", "stale"})
        self.assertEqual((e["name"], e["launch"], e["start"], e["end"], e["total_lines"]), ("demo_log.txt", 2, 8, 96, 96))
        self.assertEqual(e["parser"], "log_triage")
        self.assertEqual(e["verdict_word"], "ALERTS(2)")
        self.assertEqual(e["alerts_total"], 2)
        self.assertEqual(e["file_sha256"], hashlib.sha256(Path(p).read_bytes()).hexdigest())
        self.assertEqual(e["mtime_utc"], "2025-10-09T08:53:20Z")
        self.assertIs(e["stale"], False)
        b1, _ = self.build(logs=("demo_log.txt@1",))
        e1 = b1["logs"][0]
        self.assertEqual((e1["launch"], e1["start"], e1["end"], e1["alerts_total"]), (1, 1, 7, 1))
        self.assertTrue(all(ev["launch"] == 1 for ev in b1["evidence"]))
        b2, _ = self.build(logs=("demo_log.txt@2",))
        self.assertEqual(b2, b)

    def test_two_launches_of_one_log_and_log_order_is_sorted(self):
        self.mk("demo_log.txt", two_launch_lines())
        self.mk("aaa_log.txt", two_launch_lines())
        b, _ = self.build(logs=("demo_log.txt@2", "demo_log.txt@1", "aaa_log.txt"))
        self.assertEqual([(e["name"], e["launch"]) for e in b["logs"]],
                         [("aaa_log.txt", 2), ("demo_log.txt", 1), ("demo_log.txt", 2)])
        names = [ev["log"] for ev in b["evidence"]]
        self.assertEqual(names, sorted(names))
        self.assertEqual(len({ev["id"] for ev in b["evidence"]}), len(b["evidence"]))
        b_rev, _ = self.build(logs=("aaa_log.txt", "demo_log.txt@1", "demo_log.txt@2"))
        self.assertEqual(b_rev, b, "argument order must not change the bundle")

    def test_gaps_always_listed(self):
        self.mk("demo_log.txt", two_launch_lines())
        b, _ = self.build()
        joined = " | ".join(b["gaps"]).lower()
        self.assertIn("wall-clock", joined)
        self.assertIn("save", joined)
        self.assertIn("mod-to-log", joined)
        self.assertGreaterEqual(len(b["gaps"]), 3)

    def test_stale_rule(self):
        self.mk("demo_log.txt", two_launch_lines())
        b, _ = self.build(now=MTIME + 6 * 86400)
        self.assertIs(b["logs"][0]["stale"], False)
        b, _ = self.build(now=MTIME + 8 * 86400)
        self.assertIs(b["logs"][0]["stale"], True)
        b, _ = self.build(now=MTIME - 86400)
        self.assertIs(b["logs"][0]["stale"], False, "future mtime is not stale")

    def test_empty_log_default_is_recorded_not_refused(self):
        self.mk("demo_log.txt", [])
        b, _ = self.build()
        e = b["logs"][0]
        self.assertEqual((e["launch"], e["start"], e["end"], e["total_lines"], e["verdict_word"]),
                         (None, None, None, 0, "EMPTY"))
        self.assertEqual(b["evidence"], [])
        self.assertTrue(any("demo_log.txt" in g and "empty" in g.lower() for g in b["gaps"]))

    def test_no_absolute_paths_in_bundle(self):
        self.mk("demo_log.txt", two_launch_lines())
        _, raw = self.build()
        text = raw.decode("utf-8")
        self.assertNotIn(self.tmp, text)
        self.assertNotIn(self.gd, text)
        self.assertNotIn("/home/", text)

    def test_stdout_summary_has_no_path(self):
        self.mk("demo_log.txt", two_launch_lines())
        rc, so, se = self.cli()
        self.assertEqual(rc, 0, se)
        self.assertNotIn(self.tmp, so)
        self.assertTrue(so.strip())


class Determinism(Base):
    def test_two_builds_byte_identical(self):
        self.mk("demo_log.txt", two_launch_lines())
        self.mk("heavy-a_log.txt", heavy_lines("heavy-a"))
        _, raw1 = self.build(logs=("demo_log.txt", "heavy-a_log.txt"))
        # different temp state dir / cwd / second out file: bytes must still match
        out2 = os.path.join(self.outdir, "bundle2.json")
        with contextlib.chdir(self.tmp):
            _, raw2 = self.build(logs=("demo_log.txt", "heavy-a_log.txt"), out=out2)
        self.assertEqual(raw1, raw2)

    def test_bundle_sha256_recomputes(self):
        self.mk("demo_log.txt", two_launch_lines())
        b, raw = self.build()
        d = json.loads(raw.decode("utf-8"))
        want = d.pop("bundle_sha256")
        self.assertEqual(want, sha(canon(d)))
        self.assertEqual(raw.decode("utf-8"), canon(dict(d, bundle_sha256=want)) + "\n")

    def test_symptom_change_changes_hash(self):
        self.mk("demo_log.txt", two_launch_lines())
        a, _ = self.build(symptom="one")
        b, _ = self.build(symptom="two")
        self.assertNotEqual(a["bundle_sha256"], b["bundle_sha256"])
        self.assertEqual(a["evidence"], b["evidence"])


class Evidence(Base):
    def test_ids_hashes_and_ranges(self):
        self.mk("demo_log.txt", two_launch_lines())
        b, _ = self.build()
        self.assertGreaterEqual(len(b["evidence"]), 4)
        ids = [ev["id"] for ev in b["evidence"]]
        self.assertEqual(len(set(ids)), len(ids))
        for ev in b["evidence"]:
            m = ID_RE.match(ev["id"])
            self.assertIsNotNone(m, ev["id"])
            self.assertEqual(m.group("log"), ev["log"])
            self.assertEqual((int(m.group("a")), int(m.group("b"))), (ev["start"], ev["end"]))
            text = "\n".join(l["text"] for l in ev["lines"])
            self.assertEqual(ev["text_sha256"], sha(text))
            self.assertEqual(m.group("h"), ev["text_sha256"][:8])
            self.assertEqual([l["n"] for l in ev["lines"]], list(range(ev["start"], ev["end"] + 1)))
            self.assertLessEqual(len(ev["lines"]), 30)
            self.assertTrue(ev["start"] <= ev["anchor"]["line"] <= ev["end"])
            self.assertIn(ev["anchor"]["kind"], TIER)
            self.assertIs(ev["line_cut"], False)
            self.assertIn(ev["launch"], (1, 2))

    def test_excerpt_text_matches_log_lines(self):
        lines = two_launch_lines()
        self.mk("demo_log.txt", lines)
        b, _ = self.build()
        for ev in b["evidence"]:
            for l in ev["lines"]:
                self.assertEqual(l["text"], lines[l["n"] - 1])

    def test_priority_order_alerts_banner_signatures(self):
        self.mk("demo_log.txt", two_launch_lines())
        b, _ = self.build()
        got = [(ev["anchor"]["kind"], ev["anchor"]["line"]) for ev in b["evidence"]]
        self.assertEqual(got, [("alert", 70), ("alert", 90), ("banner", 8), ("signature", 9), ("signature", 49),
                               ("signature", 96)])
        tiers = [TIER[k] for k, _ in got]
        self.assertEqual(tiers, sorted(tiers))
        # the banner line number is smaller than the alerts', so this is priority order and not line order
        self.assertLess(got[2][1], got[0][1])

    def test_context_is_four_lines_and_clamped_to_the_launch(self):
        self.mk("demo_log.txt", two_launch_lines())
        b, _ = self.build()
        by = {(ev["anchor"]["kind"], ev["anchor"]["line"]): ev for ev in b["evidence"]}
        self.assertEqual((by[("alert", 70)]["start"], by[("alert", 70)]["end"]), (66, 74))
        self.assertEqual((by[("banner", 8)]["start"], by[("banner", 8)]["end"]), (8, 12))  # not 4..12: launch starts at 8
        self.assertEqual(by[("signature", 96)]["end"], 96)

    def test_contained_signature_excerpts_are_not_duplicated(self):
        self.mk("demo_log.txt", two_launch_lines())
        b, _ = self.build()
        rngs = [(ev["start"], ev["end"]) for ev in b["evidence"]]
        # evidence is in priority order: no later (lower priority) excerpt may lie fully inside an earlier one
        for j, (s2, e2) in enumerate(rngs):
            for i in range(j):
                s, e = rngs[i]
                self.assertFalse(s <= s2 and e2 <= e, (rngs[i], rngs[j]))

    def test_window_helper_caps_item_lines(self):
        self.assertEqual(PB.window(70, 1, 100, 4, 30), (66, 74))
        self.assertEqual(PB.window(2, 1, 100, 4, 30), (1, 6))
        self.assertEqual(PB.window(99, 1, 100, 4, 30), (95, 100))
        s, e = PB.window(50, 1, 100, 40, 30)
        self.assertEqual(e - s + 1, 30)
        self.assertTrue(s <= 50 <= e)
        s, e = PB.window(1, 1, 100, 40, 30)
        self.assertEqual((s, e), (1, 30))
        s, e = PB.window(100, 1, 100, 40, 30)
        self.assertEqual((s, e), (71, 100))

    def test_long_line_is_cut_and_flagged(self):
        lines = ["[1 f0] cut-mod 1.0.0 loaded; pid=1", "[2 f1] ERROR " + "x" * 400, "[3 f1] ok short"]
        self.mk("demo_log.txt", lines)
        b, _ = self.build()
        flagged = [ev for ev in b["evidence"] if ev["line_cut"]]
        self.assertTrue(flagged)
        for ev in b["evidence"]:
            self.assertTrue(all(len(l["text"]) <= 240 for l in ev["lines"]))
            self.assertEqual(ev["text_sha256"], sha("\n".join(l["text"] for l in ev["lines"])))
            self.assertEqual(ev["line_cut"], any(len(l["text"]) == 240 and l["text"].endswith("...") for l in ev["lines"]))
        self.assertTrue(any(len(l["text"]) == 240 for ev in flagged for l in ev["lines"]))

    def test_control_chars_replaced(self):
        lines = ["[1 f0] ctl-mod 1.0.0 loaded; pid=1", "[2 f1] ERROR bad\x1b[31mred\x00 thing", "[3 f1] ok"]
        self.mk("demo_log.txt", lines)
        b, raw = self.build()
        text = raw.decode("utf-8")
        self.assertNotIn("\\u001b", text)
        self.assertNotIn("\\u0000", text)
        self.assertGreaterEqual(b["scrub"]["control_chars_replaced"], 2)

    def test_crlf_and_no_trailing_newline(self):
        p = os.path.join(self.gd, "demo_log.txt")
        Path(p).write_bytes(b"[1 f0] crlf-mod 1.0.0 loaded; pid=1\r\n[2 f1] ERROR boom\r\n[3 f1] last line no newline")
        b, _ = self.build()
        self.assertEqual(b["logs"][0]["total_lines"], 3)
        texts = [l["text"] for ev in b["evidence"] for l in ev["lines"]]
        self.assertIn("[2 f1] ERROR boom", texts)
        self.assertTrue(all("\r" not in t for t in texts))


class Caps(Base):
    def test_items_per_log_and_max_items_set_truncated(self):
        for t in "abcde":
            self.mk("heavy-%s_log.txt" % t, heavy_lines("heavy-" + t))
        logs = tuple("heavy-%s_log.txt" % t for t in "abcde")
        b, raw = self.build(logs=logs)
        self.assertTrue(b["truncated"])
        per = {}
        for ev in b["evidence"]:
            per[ev["log"]] = per.get(ev["log"], 0) + 1
        self.assertTrue(all(n <= 12 for n in per.values()), per)
        self.assertLessEqual(len(b["evidence"]), 40)
        self.assertEqual(len(b["evidence"]), 40)
        reasons = {d["reason"] for d in b["dropped"]}
        self.assertIn("items_per_log", reasons)
        self.assertIn("max_items", reasons)
        self.assertIn("alerts_over_triage_cap", reasons)
        over = [d for d in b["dropped"] if d["reason"] == "alerts_over_triage_cap"]
        self.assertTrue(all(d["count"] == 20 for d in over))
        for d in b["dropped"]:
            self.assertIn("log", d)
            self.assertIn("launch", d)
            if d["reason"] != "alerts_over_triage_cap":
                self.assertEqual(set(d["anchor"]), {"kind", "line"})
        self.assertLessEqual(len(raw), 49152)
        # alerts outrank banners/signatures in the global cut, and the cut is a fair share across logs (8 each of 40)
        kinds_kept = [ev["anchor"]["kind"] for ev in b["evidence"]]
        self.assertEqual(kinds_kept.count("alert"), 40)
        alerts_per_log = {}
        for ev in b["evidence"]:
            alerts_per_log[ev["log"]] = alerts_per_log.get(ev["log"], 0) + 1
        self.assertEqual(sorted(alerts_per_log.values()), [8] * 5)

    def test_max_bytes_drops_lowest_priority_first(self):
        self.mk("demo_log.txt", two_launch_lines())
        full, _ = self.build()
        self.assertFalse(full["truncated"])
        b, raw = self.build("--max-bytes", "3600")
        self.assertLessEqual(len(raw), 3600)
        self.assertTrue(b["truncated"])
        self.assertEqual(b["caps"]["max_bytes"], 3600)
        self.assertTrue(any(d["reason"] == "max_bytes" for d in b["dropped"]))
        kept = [(ev["anchor"]["kind"], ev["anchor"]["line"]) for ev in b["evidence"]]
        full_kinds = [(ev["anchor"]["kind"], ev["anchor"]["line"]) for ev in full["evidence"]]
        self.assertEqual(kept, full_kinds[:len(kept)], "kept items are a priority prefix of the full bundle")
        self.assertTrue(len(kept) < len(full_kinds))
        dropped_anchor = [(d["anchor"]["kind"], d["anchor"]["line"]) for d in b["dropped"] if d["reason"] == "max_bytes"]
        self.assertEqual(sorted(kept + dropped_anchor), sorted(full_kinds))

    def test_exact_fit_is_not_truncated(self):
        self.mk("demo_log.txt", two_launch_lines())
        full, raw = self.build()
        b2, raw2 = self.build("--max-bytes", str(len(raw)))
        self.assertFalse(b2["truncated"])
        b3, raw3 = self.build("--max-bytes", str(len(raw2)))
        self.assertFalse(b3["truncated"])
        self.assertEqual(len(raw3), len(raw2))
        b4, raw4 = self.build("--max-bytes", str(len(raw3) - 1))
        self.assertTrue(b4["truncated"])
        self.assertLessEqual(len(raw4), len(raw3) - 1)

    def test_skeleton_only_when_budget_is_tight(self):
        self.mk("demo_log.txt", two_launch_lines())
        se = self.assert_refused("--max-bytes", "1000", expect="skeleton")
        skeleton = int(re.search(r"\((\d+) bytes\)", se).group(1))
        b, raw = self.build("--max-bytes", str(skeleton))
        self.assertEqual(len(raw), skeleton)
        self.assertEqual(b["evidence"], [])
        self.assertTrue(b["truncated"])
        self.assertEqual(len(b["dropped"]), 6)
        self.assertTrue(all(d["reason"] == "max_bytes" for d in b["dropped"]))
        os.remove(self.out)
        self.assert_refused("--max-bytes", str(skeleton - 1), expect="skeleton")


class Scrub(Base):
    WINE = [
        "[1 f0] wine-mod 1.0.0 loaded; pid=1",
        "[2 f1] ERROR cannot open Z:\\home\\alice_wine\\.local\\share\\Steam\\x.tga",
        "[3 f1] ERROR cannot open /home/alice_wine/.config/foo",
        "[4 f1] ERROR profile C:\\users\\alice_wine\\AppData\\x and C:\\Users\\steamuser\\y",
        "[5 f1] ERROR doubled Z:\\\\home\\\\alice_wine\\\\x",
        "[6 f1] ERROR key sk-abcdefghijklmnopqrstuvwxyz0123 steam 76561198012345678",
    ]

    def test_wine_and_posix_home_paths_scrubbed(self):
        self.mk("demo_log.txt", self.WINE)
        b, raw = self.build()
        text = raw.decode("utf-8")
        self.assertNotIn("alice_wine", text)
        self.assertNotIn("steamuser", text)
        self.assertNotIn("sk-abcdefghij", text)
        self.assertNotIn("76561198012345678", text)
        lines = {l["n"]: l["text"] for ev in b["evidence"] for l in ev["lines"]}
        self.assertEqual(lines[2], "[2 f1] ERROR cannot open Z:\\home\\USER\\.local\\share\\Steam\\x.tga")
        self.assertEqual(lines[3], "[3 f1] ERROR cannot open /home/USER/.config/foo")
        self.assertIn("C:\\users\\USER\\AppData\\x", lines[4])
        self.assertIn("STEAM64_ID", lines[6])
        self.assertIn("REDACTED_TOKEN", lines[6])
        s = b["scrub"]
        self.assertGreaterEqual(s["lines_changed_lib"], 2)
        self.assertGreaterEqual(s["lines_changed_project"], 2)
        self.assertEqual(s["residual_check"], "pass")
        self.assertEqual(s["forbidden_terms_checked"], len(SENTINEL))
        self.assertEqual(s["lines_scanned"], sum(len(ev["lines"]) for ev in b["evidence"]))

    def test_hash_is_over_scrubbed_text(self):
        self.mk("demo_log.txt", self.WINE)
        b, _ = self.build()
        for ev in b["evidence"]:
            self.assertEqual(ev["text_sha256"], sha("\n".join(l["text"] for l in ev["lines"])))
            self.assertEqual(ev["id"].rsplit(":", 1)[1], ev["text_sha256"][:8])

    def test_project_pass_is_idempotent_and_does_not_eat_plain_text(self):
        f = PB.project_scrub
        for s in ("Z:\\home\\bob\\x", "/home/bob/x", "c:\\USERS\\Bob Smith\\x", "plain text with home and users words"):
            once = f(s)
            self.assertEqual(f(once), once)
        self.assertEqual(f("plain text with home and users words"), "plain text with home and users words")
        self.assertEqual(f("Z:\\home\\bob\\x"), "Z:\\home\\USER\\x")
        self.assertEqual(f("c:\\USERS\\Bob\\x"), "c:\\USERS\\USER\\x")

    def test_symptom_mods_save_are_scrubbed_too(self):
        self.mk("demo_log.txt", two_launch_lines())
        b, _ = self.build("--save", "/home/alice_wine/saves/slot1", symptom="it died at /home/alice_wine/x")
        self.assertEqual(b["intake"]["symptom"], "it died at /home/USER/x")
        self.assertEqual(b["intake"]["save"], "/home/USER/saves/slot1")
        self.assertTrue(b["scrub"]["intake_fields_changed"] >= 2)

    def test_planted_residual_trips_exit_2(self):
        lines = two_launch_lines()
        lines[69] = "[500 f3] WARN leaked PLANTED_residual_42 value"
        self.mk("demo_log.txt", lines)
        se = self.assert_refused(forbidden=["planted_RESIDUAL_42"])
        self.assertNotIn("residual_42", se.lower(), "the stderr message must not echo the forbidden term")
        self.assertIn("residual", se.lower())

    def test_residual_in_symptom_trips_too(self):
        self.mk("demo_log.txt", two_launch_lines())
        self.assert_refused(forbidden=["secret-word"], symptom="my secret-word is here")

    def test_dropped_items_do_not_trip_residual(self):
        # the residual check is on the FINAL bundle bytes: a term only present in a dropped excerpt is not in the output
        lines = two_launch_lines()
        lines[94] = "[800 f3] note PLANTED_residual_42 here"
        self.mk("demo_log.txt", lines)
        rc, so, se = self.cli(forbidden=["PLANTED_residual_42"])
        self.assertEqual(rc, 2, "without the byte cap the planted term is in the output: refused")
        rc, so, se = self.cli("--max-bytes", "3600", forbidden=["PLANTED_residual_42"])
        self.assertEqual(rc, 0, se)
        b = json.loads(Path(self.out).read_text())
        self.assertNotIn(b"PLANTED_residual_42", Path(self.out).read_bytes())
        self.assertTrue(any(d["reason"] == "max_bytes" for d in b["dropped"]))

    def test_default_forbidden_terms_cover_home_and_username(self):
        terms = PB.default_forbidden_terms()
        self.assertIn(os.path.expanduser("~"), terms)
        try:
            user = getpass.getuser()
        except Exception:
            user = ""
        if user:
            self.assertIn(user, terms)
        self.assertNotIn("", terms)
        self.assertNotIn("/", terms)
        self.assertEqual(len(terms), len(set(terms)))

    def test_default_forbidden_terms_end_to_end(self):
        try:
            user = getpass.getuser()
        except Exception:
            self.skipTest("no username")
        if not user:
            self.skipTest("no username")
        lines = two_launch_lines()
        lines[69] = "[500 f3] WARN owner=%s slot 7" % user
        self.mk("demo_log.txt", lines)
        rc, so, se = self.cli(forbidden=None)
        self.assertEqual(rc, 2, se)
        self.assertFalse(os.path.exists(self.out))
        self.assertNotIn(user, se)

    def test_scrub_lib_unavailable_exits_2_via_loader(self):
        self.mk("demo_log.txt", two_launch_lines())

        def boom():
            raise ImportError("no lib")
        self.assert_refused(_scrub_loader=boom, expect="scrub")

    def test_scrub_lib_missing_subprocess(self):
        self.mk("demo_log.txt", two_launch_lines())
        fake_home = os.path.join(self.tmp, "nohome")
        os.mkdir(fake_home)
        env = dict(os.environ, HOME=fake_home)
        r = subprocess.run([sys.executable, "-I", "-B", str(TOOL), "--symptom", "x", "--kind", "crash", "--log", "demo_log.txt",
                            "--game-dir", self.gd, "--out", self.out], capture_output=True, text=True, env=env, timeout=60)
        self.assertEqual(r.returncode, 2, r.stderr)
        self.assertIn("scrub", r.stderr)
        self.assertFalse(os.path.exists(self.out))


class FailClosed(Base):
    def setUp(self):
        super().setUp()
        self.mk("demo_log.txt", two_launch_lines())

    def test_out_of_range_launch(self):
        self.assert_refused(logs=("demo_log.txt@3",), expect="launch")

    def test_launch_zero(self):
        self.assert_refused(logs=("demo_log.txt@0",), expect="launch")

    def test_launch_not_a_number(self):
        self.assert_refused(logs=("demo_log.txt@x",))

    def test_launch_on_empty_log(self):
        self.mk("empty_log.txt", [])
        self.assert_refused(logs=("empty_log.txt@1",), expect="launch")

    def test_missing_log(self):
        self.assert_refused(logs=("nope_log.txt",), expect="nope_log.txt")

    def test_dotdot_in_name(self):
        self.assert_refused(logs=("../demo_log.txt",))
        self.assert_refused(logs=("..",))
        self.assert_refused(logs=("a..b_log.txt",), expect="..")

    def test_path_separators(self):
        self.assert_refused(logs=("sub/demo_log.txt",), expect="separator")
        self.assert_refused(logs=("sub\\demo_log.txt",), expect="separator")
        self.assert_refused(logs=(os.path.join(self.gd, "demo_log.txt"),), expect="separator")

    def test_odd_names(self):
        for bad in ("", "@2", "demo_log.txt@", "de mo_log.txt", "demo:log.txt", "demo_log.txt@1@2", ".hidden_log.txt"):
            with self.subTest(bad=bad):
                self.assert_refused(logs=(bad,))

    def test_not_a_log_name(self):
        Path(self.gd, "swkotor2.ini").write_text("[Game]\nx=1\n")
        self.assert_refused(logs=("swkotor2.ini",), expect="log")

    def test_symlink_escaping_game_dir(self):
        elsewhere = os.path.join(self.tmp, "elsewhere_log.txt")
        Path(elsewhere).write_text("\n".join(two_launch_lines()) + "\n")
        os.symlink(elsewhere, os.path.join(self.gd, "link_log.txt"))
        self.assert_refused(logs=("link_log.txt",), expect="outside")

    def test_empty_and_overlong_symptom(self):
        self.assert_refused(symptom="")
        self.assert_refused(symptom="   \n\t ")
        self.assert_refused(symptom="x" * 2001, expect="2000")
        rc, so, se = self.cli(symptom="y" * 2000)
        self.assertEqual(rc, 0, se)

    def test_symptom_that_grows_when_scrubbed_is_checked_after(self):
        # "/home/a" -> "/home/USER" lengthens the text; the stored symptom must still be <= 2000
        s = "/home/a " * 250  # 2000 chars
        self.assertEqual(len(s), 2000)
        rc, so, se = self.cli(symptom=s)
        if rc == 0:
            self.assertLessEqual(len(json.loads(Path(self.out).read_text())["intake"]["symptom"]), 2000)
        else:
            self.assertEqual(rc, 2)
            self.assertFalse(os.path.exists(self.out))

    def test_unknown_kind(self):
        self.assert_refused(kind="explosion")

    def test_no_log_at_all(self):
        self.assert_refused(logs=())

    def test_duplicate_logs(self):
        self.assert_refused(logs=("demo_log.txt", "demo_log.txt"), expect="duplicate")
        self.assert_refused(logs=("demo_log.txt", "demo_log.txt@2"), expect="duplicate")

    def test_max_bytes_too_small_for_skeleton(self):
        self.assert_refused("--max-bytes", "200", expect="max-bytes")
        self.assert_refused("--max-bytes", "0")
        self.assert_refused("--max-bytes", "-5")
        self.assert_refused("--max-bytes", "abc")

    def test_bad_mod_id(self):
        self.assert_refused("--mod", "bad mod/../x")
        self.assert_refused("--mod", "")

    def test_save_too_long_or_control(self):
        self.assert_refused("--save", "s" * 300)
        self.assert_refused("--save", "")

    def test_game_dir_missing(self):
        self.assert_refused(game_dir=os.path.join(self.tmp, "nonexistent"))

    def test_out_parent_missing(self):
        out = os.path.join(self.tmp, "nodir", "b.json")
        rc, so, se = self.cli(out=out)
        self.assertEqual(rc, 2, se)
        self.assertFalse(os.path.exists(out))

    def test_out_inside_game_dir_refused(self):
        out = os.path.join(self.gd, "bundle.json")
        rc, so, se = self.cli(out=out)
        self.assertEqual(rc, 2, se)
        self.assertFalse(os.path.exists(out))

    def test_out_is_directory(self):
        rc, so, se = self.cli(out=self.outdir)
        self.assertEqual(rc, 2, se)
        self.assertEqual(os.listdir(self.outdir), [])

    def test_triage_failure_is_refused(self):
        def bad(argv, out):
            out.write("not json\n")
            return 0
        self.assert_refused(_triage_main=bad)

        def bad_rc(argv, out):
            out.write("{}\n")
            return 2
        self.assert_refused(_triage_main=bad_rc)

    def test_triage_line_count_mismatch_is_refused(self):
        def lying(argv, out):
            buf = io.StringIO()
            rc = LT.main(argv, out=buf)
            d = json.loads(buf.getvalue())
            d["total_lines"] += 1
            out.write(json.dumps(d) + "\n")
            return rc
        self.assert_refused(_triage_main=lying, expect="line")

    def test_existing_out_survives_a_refusal(self):
        Path(self.out).write_text("previous")
        rc, so, se = self.cli(logs=("nope_log.txt",))
        self.assertEqual(rc, 2)
        self.assertEqual(Path(self.out).read_text(), "previous")


class Safety(Base):
    def test_bookmark_never_written(self):
        self.mk("demo_log.txt", two_launch_lines())
        default_bm = os.path.expanduser(LT.DEFAULT_STATE)
        before = (os.stat(default_bm).st_mtime_ns, os.stat(default_bm).st_size) if os.path.exists(default_bm) else None
        calls = []

        def spy(argv, out):
            calls.append(list(argv))
            return LT.main(argv, out=out)
        b, _ = self.build(logs=("demo_log.txt", "demo_log.txt@1"), _triage_main=spy)
        self.assertEqual(len(calls), 2)
        for argv in calls:
            self.assertNotIn("--new", argv)
            self.assertNotIn("--no-advance", argv)
            self.assertIn("--json", argv)
            self.assertIn("--state", argv)
            state = argv[argv.index("--state") + 1]
            self.assertFalse(os.path.exists(state), "bookmark file must not exist")
            self.assertFalse(os.path.exists(os.path.dirname(state)), "temp state dir must be removed")
            self.assertTrue(state.startswith(tempfile.gettempdir()), state)
            self.assertNotIn(self.gd, state)
        after = (os.stat(default_bm).st_mtime_ns, os.stat(default_bm).st_size) if os.path.exists(default_bm) else None
        self.assertEqual(before, after, "default bookmark file untouched")

    def test_game_dir_and_log_untouched(self):
        p = self.mk("demo_log.txt", two_launch_lines())
        before = (Path(p).read_bytes(), os.stat(p).st_mtime_ns, sorted(os.listdir(self.gd)))
        self.build()
        self.assertEqual((Path(p).read_bytes(), os.stat(p).st_mtime_ns, sorted(os.listdir(self.gd))), before)

    def test_triage_runs_on_a_snapshot_not_the_live_file(self):
        p = self.mk("demo_log.txt", two_launch_lines())
        seen = []

        def spy(argv, out):
            seen.append(argv[0])
            return LT.main(argv, out=out)
        self.build(_triage_main=spy)
        self.assertNotEqual(os.path.realpath(seen[0]), os.path.realpath(p))
        self.assertEqual(os.path.basename(seen[0]), "demo_log.txt")
        self.assertFalse(os.path.exists(seen[0]))

    def test_atomic_write_and_no_tmp_leftovers(self):
        self.mk("demo_log.txt", two_launch_lines())
        self.build()
        self.assertEqual(os.listdir(self.outdir), ["bundle.json"])
        os.remove(self.out)
        with mock.patch("os.replace", side_effect=OSError("disk gone")):
            rc, so, se = self.cli()
        self.assertEqual(rc, 2, se)
        self.assertEqual(os.listdir(self.outdir), [])

    def test_overwrite_is_atomic_replace(self):
        self.mk("demo_log.txt", two_launch_lines())
        Path(self.out).write_text("old")
        b, raw = self.build()
        self.assertEqual(json.loads(raw)["format"], "playtest-bundle/1")


class CliProcess(Base):
    def test_help_without_isolated_flag_and_no_lib_import(self):
        env = {k: v for k, v in os.environ.items() if not k.startswith("PYTHON")}
        r = subprocess.run([sys.executable, str(TOOL), "--help"], capture_output=True, text=True, env=env, timeout=10)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertIn("--symptom", r.stdout)
        self.assertIn("--kind", r.stdout)
        self.assertIn("--log", r.stdout)
        self.assertIn("--max-bytes", r.stdout)

    def test_import_does_not_touch_lib(self):
        code = ("import sys; sys.path.insert(0, %r); import playtest_bundle; "
                "assert 'scrub' not in sys.modules, 'scrub imported'; "
                "assert not any('.claude/swarm' in p for p in sys.path), sys.path; "
                "assert 'log_triage' not in sys.modules; print('ok')" % str(HERE))
        r = subprocess.run([sys.executable, "-I", "-B", "-c", code], capture_output=True, text=True, timeout=10)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual(r.stdout.strip(), "ok")

    def test_end_to_end_subprocess_isolated_mode(self):
        self.mk("demo_log.txt", two_launch_lines())
        r = subprocess.run([sys.executable, "-I", "-B", str(TOOL), "--symptom", "boom", "--kind", "crash", "--log", "demo_log.txt",
                            "--game-dir", self.gd, "--out", self.out], capture_output=True, text=True, timeout=60)
        self.assertEqual(r.returncode, 0, r.stderr)
        b = json.loads(Path(self.out).read_text())
        self.assertEqual(b["logs"][0]["name"], "demo_log.txt")
        self.assertEqual(b["scrub"]["residual_check"], "pass")
        self.assertGreaterEqual(b["scrub"]["forbidden_terms_checked"], 1, "default forbidden terms are active")
        self.assertNotIn(self.tmp, r.stdout)

    def test_cli_exit_codes_for_usage_errors(self):
        r = subprocess.run([sys.executable, str(TOOL), "--kind", "crash"], capture_output=True, text=True, timeout=10)
        self.assertEqual(r.returncode, 2)
        self.assertTrue(r.stderr.strip())


if __name__ == "__main__":
    suite = unittest.defaultTestLoader.loadTestsFromModule(sys.modules[__name__])
    res = unittest.TextTestRunner(verbosity=1).run(suite)
    bad = len(res.failures) + len(res.errors)
    print("playtest_bundle tests: %d/%d PASS" % (res.testsRun - bad - len(res.skipped), res.testsRun))
    sys.exit(0 if res.wasSuccessful() else 1)
