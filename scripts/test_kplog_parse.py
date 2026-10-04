#!/usr/bin/env python3
"""Tests for scripts/kplog_parse.py (fixtures in scripts/testdata/kplog/, see README.txt there)."""
import io
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import kplog_parse as K  # noqa: E402

FX = HERE / "testdata" / "kplog"


def fx(name):
    return str(FX / name)


def cli(*args):
    buf = io.StringIO()
    rc = K.main([str(a) for a in args], out=buf)
    return rc, buf.getvalue().splitlines()


class ParseLine(unittest.TestCase):
    def test_plain_line_no_frame(self):
        p = K.parse_line("T=0.309 f=- logger-fixture/main info line")
        self.assertEqual((p["kind"], p["t_ms"], p["frame"], p["patch"], p["channel"], p["level"], p["msg"], p["truncated"]),
                         ("line", 0.309, None, "logger-fixture", "main", "INFO", "info line", False))

    def test_frame_number_and_big_t(self):
        p = K.parse_line("T=2000.001 f=12 mypatch/draw drew 12 quads in 3 batches")
        self.assertEqual((p["t_ms"], p["frame"], p["msg"]), (2000.001, 12, "drew 12 quads in 3 batches"))

    def test_warn_and_err(self):
        p = K.parse_line("T=0.307 f=- logger-fixture/main WARN hello from KPatchInit")
        self.assertEqual((p["level"], p["msg"]), ("WARN", "hello from KPatchInit"))
        p = K.parse_line("T=1.000 f=3 a.b-c_d/x.y ERR boom")
        self.assertEqual((p["level"], p["patch"], p["channel"], p["msg"]), ("ERR", "a.b-c_d", "x.y", "boom"))

    def test_level_word_inside_text_is_not_a_token(self):
        p = K.parse_line("T=1.000 f=- a/b the WARN word")
        self.assertEqual((p["level"], p["msg"]), ("INFO", "the WARN word"))

    def test_empty_message(self):
        p = K.parse_line("T=1.000 f=- a/b ")
        self.assertEqual((p["kind"], p["msg"]), ("line", ""))

    def test_trailing_newline_stripped(self):
        self.assertEqual(K.parse_line("T=1.000 f=- a/b hi\r\n")["msg"], "hi")

    def test_truncated(self):
        p = K.parse_line("T=0.816 f=- kplog-a/alpha " + "s" * 227 + "[...]")
        self.assertTrue(p["truncated"])
        self.assertFalse(K.parse_line("T=0.100 f=- a/b short")["truncated"])

    def test_header_lines(self):
        p = K.parse_line("# kplog session 2026-10-03 16:58:01 | kpatch.log v1 | patcher ABI 1")
        self.assertEqual((p["kind"], p["session"]), ("header", "2026-10-03 16:58:01"))
        p = K.parse_line("# KOTOR_VERSION_SHA=smoke")
        self.assertEqual((p["kind"], p["msg"]), ("header", "KOTOR_VERSION_SHA=smoke"))
        self.assertNotIn("session", p)
        self.assertEqual(K.parse_line("#   kpatch.log v1 by patcher")["kind"], "header")
        self.assertEqual(K.parse_line("# session info:")["kind"], "header")

    def test_mark(self):
        p = K.parse_line("T=2100.000 f=1 MARK before the cutscene")
        self.assertEqual((p["kind"], p["t_ms"], p["frame"], p["patch"], p["channel"], p["msg"]),
                         ("mark", 2100.0, 1, None, None, "before the cutscene"))
        self.assertEqual(K.parse_line("T=1.000 f=- MARK x")["frame"], None)

    def test_dropped(self):
        p = K.parse_line("T=3.000 f=2 hand/kplog DROPPED 7 (ring 4, rate 3)")
        self.assertEqual((p["kind"], p["patch"], p["channel"], p["dropped"], p["ring"], p["rate"]),
                         ("dropped", "hand", "kplog", 7, 4, 3))

    def test_kplog_channel_other_text_is_plain_line(self):
        self.assertEqual(K.parse_line("T=3.000 f=2 hand/kplog something else")["kind"], "line")

    def test_fallback_shapes(self):
        p = K.parse_line("57843211 old-mod/main WARN fell back")
        self.assertEqual((p["kind"], p["t_ms"], p["frame"], p["patch"], p["channel"], p["level"], p["msg"]),
                         ("fallback", 57843211.0, None, "old-mod", "main", "WARN", "fell back"))
        self.assertEqual(K.parse_line("12 p/c ERR x")["level"], "ERR")
        p = K.parse_line("12 p/c plain text")
        self.assertEqual((p["level"], p["msg"]), ("INFO", "plain text"))

    def test_malformed_returns_none(self):
        for s in ["", "   ", "garbage", "T=1.0 f=- a/b nofraction3", "T=1.000 a/b missing frame",
                  "T=1.000 f=x a/b bad frame", "--- log cap reached ---", "T=1.000 f=- nochannel text"]:
            self.assertIsNone(K.parse_line(s), s)


class StripPrefix(unittest.TestCase):
    def test_core_shapes(self):
        self.assertEqual(K.strip_prefix("T=0.309 f=- logger-fixture/main info line"), "info line")
        self.assertEqual(K.strip_prefix("T=1.000 f=7 a/b WARN [tag] x y"), "[tag] x y")
        self.assertEqual(K.strip_prefix("T=1.000 f=7 a/b ERR boom"), "boom")
        self.assertEqual(K.strip_prefix("T=3.000 f=2 hand/kplog DROPPED 7 (ring 4, rate 3)"), "DROPPED 7 (ring 4, rate 3)")

    def test_mark(self):
        self.assertEqual(K.strip_prefix("T=2100.000 f=1 MARK before"), "before")

    def test_fallback_shape(self):
        self.assertEqual(K.strip_prefix("57843211 old-mod/main WARN fell back"), "fell back")
        self.assertEqual(K.strip_prefix("57843211 old-mod/main plain"), "plain")

    def test_unprefixed_unchanged(self):
        for s in ["[tag] already bare", "# kplog session x", "", "12 not a prefix"]:
            self.assertEqual(K.strip_prefix(s), s)

    def test_only_one_prefix_removed(self):
        self.assertEqual(K.strip_prefix("T=1.000 f=- a/b T=2.000 f=- c/d x"), "T=2.000 f=- c/d x")


class RealFiles(unittest.TestCase):
    def test_logger_fixture_real_output(self):
        segs = K.read_segments(fx("logger-fixture_log.txt"))
        self.assertEqual(len(segs), 1)
        self.assertEqual(segs[0]["session"], "2026-10-03 16:58:14")
        kinds = [p["kind"] for _, p in segs[0]["entries"]]
        self.assertEqual(kinds, ["header"] * 5 + ["line"] * 2)
        self.assertEqual(segs[0]["malformed"], 0)

    def test_truncated_real_output(self):
        ps = [p for _, p in K.read_segments(fx("trunc_kplog-a_log.txt"))[0]["entries"] if p["kind"] == "line"]
        self.assertEqual([p["truncated"] for p in ps], [False, True, True])
        self.assertEqual(len(ps[1]["msg"]), 232)

    def test_hand_file(self):
        seg = K.read_segments(fx("hand_markdrop_log.txt"))[0]
        self.assertEqual(seg["malformed"], 1)
        kinds = [p["kind"] for _, p in seg["entries"]]
        self.assertEqual(kinds.count("mark"), 1)
        self.assertEqual(kinds.count("dropped"), 2)

    def test_fallback_file(self):
        seg = K.read_segments(fx("fallback_log.txt"))[0]
        self.assertIsNone(seg["session"])
        self.assertEqual([p["level"] for _, p in seg["entries"]], ["WARN", "INFO", "ERR"])
        self.assertEqual(seg["malformed"], 1)  # the "--- log cap reached ---" line


class Cli(unittest.TestCase):
    def test_default_prints_lines_not_headers(self):
        rc, lines = cli(fx("kplog-a_log.txt"))
        self.assertEqual(rc, 0)
        self.assertEqual(lines, ["T=0.997 f=- kplog-a/alpha a-alpha", "T=1.019 f=- kplog-a/beta WARN a-beta",
                                 "T=1.056 f=- kplog-a/debug a-debug"])

    def test_headers_flag(self):
        _, lines = cli(fx("kplog-a_log.txt"), "--headers")
        self.assertEqual(len(lines), 6)
        self.assertTrue(lines[0].startswith("# kplog session"))

    def test_patch_and_channel(self):
        _, lines = cli(fx("kplog-a_log.txt"), fx("kplog-b_log.txt"), "--patch", "kplog-b")
        self.assertEqual(len(lines), 3)
        self.assertTrue(all("kplog-b/" in l for l in lines))
        _, lines = cli(fx("kplog-a_log.txt"), fx("kplog-b_log.txt"), "--channel", "beta")
        self.assertEqual([l.split()[2] for l in lines], ["kplog-a/beta", "kplog-b/beta"])

    def test_level(self):
        _, lines = cli(fx("hand_markdrop_log.txt"), "--level", "err")
        self.assertEqual(lines, ["T=2.250 f=2 hand/main ERR hook returned null"])  # MARK is WARN level, so filtered too
        _, lines = cli(fx("hand_markdrop_log.txt"), "--level", "WARN")
        self.assertEqual(len(lines), 3)  # WARN, MARK, ERR
        _, lines = cli(fx("hand_markdrop_log.txt"), "--level", "TRACE")
        self.assertEqual(len(lines), 8)

    def test_since(self):
        _, lines = cli(fx("hand_markdrop_log.txt"), "--since", "2.25")
        self.assertEqual(len(lines), 4)
        self.assertTrue(lines[0].startswith("T=2.250"))

    def test_mark_passes_patch_filter(self):
        _, lines = cli(fx("hand_markdrop_log.txt"), "--patch", "hand", "--channel", "draw")
        self.assertEqual(sum("MARK" in l for l in lines), 1)

    def test_bare(self):
        _, lines = cli(fx("hand_markdrop_log.txt"), "--bare", "--channel", "draw")
        self.assertIn("drew 12 quads", lines)
        self.assertIn("texture 4 missing", lines)
        self.assertIn("before the cutscene", lines)

    def test_fallback_file_cli(self):
        _, lines = cli(fx("fallback_log.txt"), "--level", "WARN", "--bare")
        self.assertEqual(lines, ["fell back to the file sink", "something broke"])

    def test_stats(self):
        _, lines = cli(fx("hand_markdrop_log.txt"), "--stats")
        self.assertIn("hand/draw INFO 2", lines)
        self.assertIn("hand/draw WARN 1", lines)
        self.assertIn("hand/main ERR 1", lines)
        self.assertIn("DROPPED hand total 9 (ring 4, rate 5)", lines)
        self.assertIn("truncated 0", lines)
        self.assertIn("marks 1", lines)
        self.assertIn("malformed 1", lines)
        _, lines = cli(fx("trunc_kplog-a_log.txt"), "--stats")
        self.assertIn("truncated 2", lines)

    def test_missing_file(self):
        err = io.StringIO()
        old, sys.stderr = sys.stderr, err
        try:
            rc, _ = cli(fx("nope.txt"))
        finally:
            sys.stderr = old
        self.assertEqual(rc, 2)

    def test_subprocess(self):
        r = subprocess.run([sys.executable, str(HERE / "kplog_parse.py"), fx("kplog-a_log.txt"), "--level", "WARN"],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0)
        self.assertEqual(r.stdout.splitlines(), ["T=1.019 f=- kplog-a/beta WARN a-beta"])


class Merge(unittest.TestCase):
    def test_two_files_one_session_sorted(self):
        _, lines = cli(fx("kplog-b_log.txt"), fx("kplog-a_log.txt"), "--merge")
        self.assertEqual(len(lines), 6)  # no separator: one session
        ts = [float(l.split()[0][2:]) for l in lines]
        self.assertEqual(ts, sorted(ts))
        self.assertEqual([l.split()[2] for l in lines][:3], ["kplog-a/alpha", "kplog-a/beta", "kplog-b/alpha"])

    def test_sessions_merged_separately_oldest_first(self):
        _, lines = cli(fx("hand_session2_c_log.txt"), fx("kplog-b_log.txt"), fx("kplog-a_log.txt"), "--merge")
        seps = [i for i, l in enumerate(lines) if l.startswith("# --- session")]
        self.assertEqual(len(seps), 2)
        self.assertIn("kplog-a_log.txt, kplog-b_log.txt", lines[seps[0]])
        self.assertIn("hand_session2_c_log.txt", lines[seps[1]])
        self.assertEqual(len(lines), 2 + 6 + 2)
        self.assertTrue(lines[seps[1] + 1].startswith("T=0.500"))

    def test_late_opened_file_same_session(self):
        # b's file was opened 2 s after a's (lazy open): header wall clock differs, but wall - first T matches.
        with tempfile.TemporaryDirectory() as d:
            a = Path(d, "a_log.txt")
            b = Path(d, "b_log.txt")
            a.write_text("# kplog session 2026-10-03 20:00:00 | kpatch.log v1 | patcher ABI 1\n"
                         "T=100.000 f=- a/x one\nT=3000.000 f=- a/x three\n")
            b.write_text("# kplog session 2026-10-03 20:00:02 | kpatch.log v1 | patcher ABI 1\n"
                         "T=2100.000 f=- b/y two\n")
            _, lines = cli(b, a, "--merge")
            self.assertEqual([l.split()[-1] for l in lines], ["one", "two", "three"])

    def test_rotated_file_continues_session(self):
        with tempfile.TemporaryDirectory() as d:
            a = Path(d, "a_log.txt")
            a.write_text("# kplog session 2026-10-03 20:00:00 | kpatch.log v1 | patcher ABI 1\nT=10.000 f=- a/x one\n"
                         "# kplog session 2026-10-03 20:00:05 | kpatch.log v1 | patcher ABI 1\nT=5010.000 f=- a/x two\n")
            segs = K.read_segments(a)
            self.assertEqual(len(segs), 2)
            self.assertEqual(len(K.group_sessions(segs)), 1)

    def test_merge_applies_filters(self):
        _, lines = cli(fx("kplog-a_log.txt"), fx("kplog-b_log.txt"), "--merge", "--level", "WARN")
        self.assertEqual([l.split()[2] for l in lines], ["kplog-a/beta", "kplog-b/beta"])

    def test_merge_dedupes_merged_sink_copy(self):
        with tempfile.TemporaryDirectory() as d:
            hdr = "# kplog session 2026-10-03 20:00:00 | kpatch.log v1 | patcher ABI 1\n"
            body = "T=1.000 f=- a/x one\nT=2.000 f=- b/y two\n"
            Path(d, "a_log.txt").write_text(hdr + "T=1.000 f=- a/x one\n")
            Path(d, "kp_log.txt").write_text(hdr + body)
            _, lines = cli(Path(d, "a_log.txt"), Path(d, "kp_log.txt"), "--merge")
            self.assertEqual(lines, ["T=1.000 f=- a/x one", "T=2.000 f=- b/y two"])

    def test_headerless_files_not_mixed(self):
        _, lines = cli(fx("fallback_log.txt"), fx("kplog-a_log.txt"), "--merge")
        self.assertEqual(sum(l.startswith("# --- session") for l in lines), 2)
        self.assertEqual(len(lines), 2 + 3 + 3)


if __name__ == "__main__":
    unittest.main()
