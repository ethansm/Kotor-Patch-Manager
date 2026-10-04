#!/usr/bin/env python3
"""Tests for scripts/log_triage.py (fixtures in scripts/testdata/log_triage/, see README.txt there)."""
import io
import json
import os
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import log_triage as T  # noqa: E402

FX = HERE / "testdata" / "log_triage"


def fx(name):
    return str(FX / name)


def cli(*args):
    buf = io.StringIO()
    rc = T.main([str(a) for a in args], out=buf)
    return rc, buf.getvalue()


def jload(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def slurp(path, mode="r"):
    with open(path, mode) as f:
        return f.read()


def lines_of(text):
    return text.splitlines()


class TmpCase(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="log_triage_test_")
        self.addCleanup(shutil.rmtree, self.tmp, ignore_errors=True)
        self.state = os.path.join(self.tmp, "state", "bm.json")

    def copy(self, name, as_name=None):
        dst = os.path.join(self.tmp, as_name or name)
        shutil.copy(fx(name), dst)
        return dst

    def write(self, name, text):
        p = os.path.join(self.tmp, name)
        with open(p, "w", encoding="utf-8") as f:
            f.write(text)
        return p

    def append(self, path, text):
        with open(path, "a", encoding="utf-8") as f:
            f.write(text)


class PrefixAndSignature(unittest.TestCase):
    def test_prefix_strip_each_format(self):
        cases = {
            "[2963888425 f0] hello 1": "hello 1",
            "[t=2963909817 f=0] hello": "hello",
            "18:47:35.711 hello": "hello",
            "[19:14:56.972 2771081893] hello": "hello",
            "[19:14:56.970 2771081892 f3] hello": "hello",
            "[2769107855] hello": "hello",
            "[2663275041 t75849] hello": "hello",
            "T=0.304 f=- mouse/raw RAW 1": "mouse/raw RAW 1",
            "T=9810.025 f=12 a/b x": "a/b x",
            "no prefix here": "no prefix here",
            "buffhud loaded": "buffhud loaded",
        }
        for raw, want in cases.items():
            self.assertEqual(T.strip_prefix(raw), want, raw)

    def test_signature_numbers_hex_whitespace(self):
        self.assertEqual(T.signature("[1 f0] tick  5 pos=(1.5,-2.25) addr=0829ef5c flags=0x1F"),
                         "tick # pos=(#,#) addr=# flags=#")
        self.assertEqual(T.signature("T=1.000 f=- a/b v=-3 w=2.50e"), "a/b v=# w=#e")
        self.assertEqual(T.signature("[t=1 f=0] data ff 00 3c 7a end"), "data # end")

    def test_signature_truncated_to_100(self):
        s = T.signature("x" * 300)
        self.assertEqual(len(s), 100)

    def test_hexdump_lines_one_signature(self):
        self.assertEqual(T.signature("    +00: 0d 0f 00 00 00 00 00 00 02 00 03 00 00 00 00 00"), T.HEXDUMP_SIG)
        self.assertEqual(T.signature("    +a0: 95 02"), T.HEXDUMP_SIG)
        rc, out = cli(fx("shieldprobe_excerpt.txt"))
        self.assertTrue(any(l.endswith("  +# hexdump") for l in lines_of(out)))
        self.assertEqual(sum(1 for l in lines_of(out) if l.endswith("+# hexdump")), 1)

    def test_signature_blank(self):
        self.assertEqual(T.signature(""), "<blank>")
        self.assertEqual(T.signature("[1 f0] "), "<blank>")


class LaunchSplit(unittest.TestCase):
    def split(self, name):
        ls = T.read_lines(fx(name))
        return ls, T.split_launches(ls, 1)

    def test_synth_multi_three_launches(self):
        ls, sp = self.split("synth_multi.txt")
        self.assertEqual(sp, [(0, 7), (7, 10), (10, 16)])

    def test_default_reports_last_launch_only(self):
        rc, out = cli(fx("synth_multi.txt"))
        self.assertEqual(rc, 0)
        L = lines_of(out)
        self.assertEqual(L[1], "launches: 3 (showing #3, lines 11-16)")
        self.assertEqual(L[2], "[300 f0] demo-mod 1.0.1 loaded; pid=3")
        self.assertNotIn("demo-mod 1.0.0", out)

    def test_launch_n_and_all(self):
        rc, out = cli(fx("synth_multi.txt"), "--launch", 1)
        self.assertIn("launches: 3 (showing #1, lines 1-7)", out)
        rc, out = cli(fx("synth_multi.txt"), "--all-launches")
        self.assertIn("launch #1 lines 1-7", out)
        self.assertIn("launch #2 lines 8-10", out)
        self.assertIn("launch #3 lines 11-16", out)
        self.assertEqual(lines_of(out)[-1].split()[:2], ["VERDICT:", "ALERTS(3)"])

    def test_bad_launch_number(self):
        rc, out = cli(fx("synth_multi.txt"), "--launch", 9)
        self.assertEqual(rc, 0)
        self.assertIn("no such launch: 9 (1..3)", out)
        self.assertTrue(lines_of(out)[-1].startswith("VERDICT: EMPTY"))

    def test_loaded_lines_that_are_not_banners(self):
        for line in ["[1 f0] cfg: loaded force_power_hotbar.ini: invoke=1", "[1 f0] img: loaded 'fph_slot' -> 0864e4c8",
                     "18:47:35.713 INI loaded: 13 keys ok", "[2769107872] CONFIG loaded mode=log", "[2] TABLE loaded x.ini",
                     "plain text without it"]:
            self.assertFalse(T.is_banner(line), line)

    def test_banner_shapes(self):
        for line in ["[2963888425 f0] force-power-hotbar 1.0.0 loaded; cwd=S:\\x pid=320", "buffhud loaded",
                     "18:47:35.711 chareffects 0.1.0-probe loaded", "inv3d v2 loaded",
                     "[19:14:56.972 2771081893] k2-equip-icon-fix 1.0.0 loaded: 8 sites",
                     "[2615764535 f0] load-hang-probe 1.1.0 loaded: attach tid=316",
                     "[2769107855] == LevelUpOverride 0.1.0-probe loaded; first LevelUp call =="]:
            self.assertTrue(T.is_banner(line), line)

    def test_attach_banner_needs_version(self):
        self.assertTrue(T.is_banner("[2663244995 t0] perf-lab 0.2.0 attach exe=00400000-00ab9000 game=1"))
        self.assertFalse(T.is_banner("[1 f0] handler attach failed"))

    def test_real_fixtures_launch_counts(self):
        expect = {"hotbar_excerpt.txt": 1, "buffhud_excerpt.txt": 1, "shieldprobe_excerpt.txt": 1,
                  "chareffects_excerpt.txt": 2, "equipiconfix_excerpt.txt": 2, "hangprobe_excerpt.txt": 3,
                  "levelup_excerpt.txt": 2, "kplog_mouseprobe_excerpt.txt": 1, "kplog_letterbox_excerpt.txt": 1}
        for name, k in expect.items():
            ls, sp = self.split(name)
            self.assertEqual(len(sp), k, name)

    def test_shieldprobe_has_no_banner_whole_file_is_one_launch(self):
        rc, out = cli(fx("shieldprobe_excerpt.txt"))
        self.assertIn("launches: 1 (showing #1, lines 1-35)", out)

    def test_kplog_header_is_the_only_split_even_with_loaded_line(self):
        rc, out = cli(fx("kplog_letterbox_excerpt.txt"))
        L = lines_of(out)
        self.assertEqual(L[1], "launches: 1 (showing #1, lines 1-20)")
        self.assertTrue(L[2].startswith("# kplog session 2026-10-04"))

    def test_two_kplog_sessions(self):
        text = "# kplog session 2026-10-04 00:00:01 | v1\nT=1.000 f=- a/b hi\n# kplog session 2026-10-04 01:00:01 | v1\nT=2.000 f=- a/b yo\n"
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, "k.txt")
            Path(p).write_text(text)
            rc, out = cli(p)
        self.assertIn("launches: 2 (showing #2, lines 3-4)", out)

    def test_preamble_before_first_banner_is_own_launch(self):
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, "a.txt")
            Path(p).write_text("junk 1\njunk 2\nmod 1.0 loaded\nline\n")
            rc, out = cli(p, "--all-launches")
        self.assertIn("launches: 2 (showing all)", out)
        self.assertIn("launch #1 lines 1-2", out)

    def test_each_format_banner_line_shown(self):
        want = {"hotbar_excerpt.txt": "force-power-hotbar 1.0.0 loaded", "buffhud_excerpt.txt": "buffhud loaded",
                "chareffects_excerpt.txt": "chareffects 0.1.0-probe loaded", "equipiconfix_excerpt.txt": "k2-equip-icon-fix 1.0.0 loaded",
                "levelup_excerpt.txt": "LevelUpOverride 0.1.0-probe loaded", "hangprobe_excerpt.txt": "load-hang-probe 1.1.0 loaded"}
        for name, frag in want.items():
            rc, out = cli(fx(name))
            self.assertIn(frag, lines_of(out)[2], name)


class Report(unittest.TestCase):
    def test_header_has_no_mtime_and_counts(self):
        rc, out = cli(fx("buffhud_excerpt.txt"))
        L = lines_of(out)
        size = os.path.getsize(fx("buffhud_excerpt.txt"))
        self.assertEqual(L[0], "FILE buffhud_excerpt.txt | %d B | 25 lines" % size)
        self.assertEqual(L[3], "patterns: 4 unique of 25 lines")
        self.assertEqual(L[-1], "VERDICT: CLEAN  launches=1 lines=25")

    def test_top_sorted_count_desc_then_first_line(self):
        rc, out = cli(fx("buffhud_excerpt.txt"))
        rows = [l.split("  ", 2) for l in lines_of(out)[4:8]]
        self.assertEqual([(r[0], r[1]) for r in rows], [("12", "3"), ("11", "4"), ("1", "1"), ("1", "2")])

    def test_top_option(self):
        rc, out = cli(fx("synth_multi.txt"), "--top", 2)
        L = lines_of(out)
        i = L.index("patterns: 5 unique of 6 lines")
        self.assertTrue(L[i + 3].startswith("alerts:"))

    def test_alerts_dedup_by_signature_and_sorted_by_line(self):
        rc, out = cli(fx("synth_multi.txt"), "--launch", 1)
        L = lines_of(out)
        a = L.index("alerts:")
        self.assertEqual(L[a + 1], "2  5  WARN slot 7 bad value 0x1F")
        self.assertTrue(L[-1].startswith("VERDICT: ALERTS(1)"))

    def test_zero_counters_do_not_alert(self):
        self.assertFalse(T.is_alert("[1] dropped=0 errors: 0 fail=0.0"))
        self.assertFalse(T.is_alert("[1] saferead: 0 failed probes (showing last 0)"))
        self.assertFalse(T.is_alert("[1 f0] cfg: loaded x.ini: warnings=0"))
        self.assertTrue(T.is_alert("[1] dropped=3"))
        self.assertTrue(T.is_alert("[1] The thing is NULL here"))
        self.assertFalse(T.is_alert("    +00: 00 ff null"))
        self.assertFalse(T.is_alert("[1] net_drop=5 timeouts=1"))

    def test_alert_keywords(self):
        for w in ["warn", "WARNING", "err", "Error", "fail", "failed", "failure", "assert", "exception", "crash", "abort",
                  "timeout", "hang", "null", "bad", "invalid", "overflow", "drop", "dropped"]:
            self.assertTrue(T.is_alert("[1] something %s here" % w), w)

    def test_banner_containing_hang_does_not_alert(self):
        rc, out = cli(fx("hangprobe_excerpt.txt"), "--launch", 1)
        self.assertNotIn("alerts:", out)

    def test_real_alert_in_hotbar(self):
        rc, out = cli(fx("hotbar_excerpt.txt"))
        L = lines_of(out)
        a = L.index("alerts:")
        self.assertTrue(L[a + 1].startswith("1  35  character: creature 0cd83410 has invalid object id"))
        self.assertTrue(L[-1].startswith("VERDICT: ALERTS(1)"))

    def test_zero_failed_probes_real_fixture_is_clean(self):
        rc, out = cli(fx("shieldprobe_excerpt.txt"))
        self.assertTrue(lines_of(out)[-1].startswith("VERDICT: CLEAN"))

    def test_alerts_cap_10_and_sorted(self):
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, "a.txt")
            Path(p).write_text("mod 1.0 loaded\n" + "".join("error kind%s here\n" % chr(97 + i) for i in range(14)))
            rc, out = cli(p, "--max-bytes", 100000)
        L = lines_of(out)
        a = L.index("alerts: (+4 more)")
        self.assertEqual(len([l for l in L[a + 1:a + 11] if "error kind" in l]), 10)
        self.assertTrue(L[-1].startswith("VERDICT: ALERTS(14)"))

    def test_cap_heuristic_round_lines(self):
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, "a.txt")
            Path(p).write_text("mod loaded\n" + "".join("row %d\n" % i for i in range(999)))
            self.assertIn("cap: likely", cli(p)[1])
            Path(p).write_text("mod loaded\n" + "".join("row %d\n" % i for i in range(998)))
            self.assertNotIn("cap:", cli(p)[1])
            Path(p).write_text("mod loaded\nrow\nwriting stopped: limit reached\n")
            self.assertIn("cap: likely", cli(p)[1])

    def test_cap_real_buffhud_shape_not_hit_on_excerpt(self):
        self.assertNotIn("cap:", cli(fx("buffhud_excerpt.txt"))[1])

    def test_tail_five_raw_lines_each_at_most_160(self):
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, "a.txt")
            Path(p).write_text("mod loaded\n" + "".join("[%d f0] row %s\n" % (i, "z" * 300) for i in range(8)))
            rc, out = cli(p, "--max-bytes", 100000)
        L = lines_of(out)
        t = L.index("tail:")
        tail = L[t + 1:-1]
        self.assertEqual(len(tail), 5)
        self.assertTrue(all(len(x) <= 160 for x in tail))
        self.assertTrue(tail[0].startswith("[3 f0] row "))

    def test_banner_at_most_160(self):
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, "a.txt")
            Path(p).write_text("mod 1.0 loaded; " + "q" * 400 + "\n")
            rc, out = cli(p)
        self.assertLessEqual(len(lines_of(out)[2]), 160)

    def test_empty_file(self):
        rc, out = cli(fx("empty.txt"))
        self.assertEqual(rc, 0)
        L = lines_of(out)
        self.assertEqual(L, ["FILE empty.txt | 0 B | 0 lines", "VERDICT: EMPTY  launches=0 lines=0"])

    def test_multiple_files_blank_line_between(self):
        rc, out = cli(fx("buffhud_excerpt.txt"), fx("synth_multi.txt"))
        self.assertEqual(rc, 0)
        blocks = out.rstrip("\n").split("\n\n")
        self.assertEqual(len(blocks), 2)
        self.assertTrue(blocks[0].startswith("FILE buffhud_excerpt.txt"))
        self.assertTrue(blocks[1].startswith("FILE synth_multi.txt"))

    def test_missing_file_rc2_others_still_reported(self):
        rc, out = cli(fx("buffhud_excerpt.txt"), "/nonexistent/zzz.txt")
        self.assertEqual(rc, 2)
        self.assertIn("missing: /nonexistent/zzz.txt", out)
        self.assertIn("FILE buffhud_excerpt.txt", out)

    def test_crlf_and_invalid_utf8_tolerated(self):
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, "a.txt")
            Path(p).write_bytes(b"mod 1.0 loaded\r\nrow \xff\xfe 5\r\nlast\r\n")
            rc, out = cli(p)
        self.assertEqual(rc, 0)
        self.assertIn("3 lines", out)


class Determinism(unittest.TestCase):
    def test_byte_identical_repeated_output(self):
        for name in sorted(os.listdir(FX)):
            if not name.endswith(".txt") or name == "README.txt":
                continue
            a = cli(fx(name))
            b = cli(fx(name))
            self.assertEqual(a, b, name)
        self.assertEqual(cli(fx("synth_multi.txt"), "--all-launches"), cli(fx("synth_multi.txt"), "--all-launches"))

    def test_no_wall_clock_or_mtime_in_output(self):
        p = tempfile.NamedTemporaryFile(delete=False, suffix=".txt")
        p.write(slurp(fx("hotbar_excerpt.txt"), "rb"))
        p.close()
        try:
            a = cli(p.name)[1]
            os.utime(p.name, (1, 1))
            self.assertEqual(a, cli(p.name)[1])
        finally:
            os.unlink(p.name)

    def test_subprocess_run_equal(self):
        import subprocess
        outs = [subprocess.run([sys.executable, str(HERE / "log_triage.py"), fx("chareffects_excerpt.txt")],
                               capture_output=True).stdout for _ in range(2)]
        self.assertEqual(outs[0], outs[1])


class Trimming(unittest.TestCase):
    def big(self, d):
        p = os.path.join(d, "big.txt")
        rows = ["mod 1.0 loaded; pid=1"]
        for i in range(60):
            rows += ["[%d f0] event%s alpha beta gamma delta %d" % (i, "abcdefghijklmnopqrstuvwxyz"[i % 26] * 5 + str(i), i)]
            rows += ["error problem%s happened here ok" % ("xyzzy" * 3 + chr(97 + i % 26) + str(i))]
        Path(p).write_text("\n".join(rows) + "\n")
        return p

    def test_default_fits_1800_and_keeps_verdict(self):
        with tempfile.TemporaryDirectory() as d:
            rc, out = cli(self.big(d))
        self.assertLessEqual(len(out.encode()), 1800)
        self.assertTrue(lines_of(out)[-1].startswith("VERDICT: ALERTS("))

    def test_tight_max_bytes_trims_and_keeps_verdict(self):
        with tempfile.TemporaryDirectory() as d:
            p = self.big(d)
            for n in (900, 500, 300, 150, 10):
                rc, out = cli(p, "--max-bytes", n)
                L = lines_of(out)
                self.assertTrue(L[-1].startswith("VERDICT: ALERTS("), n)
                self.assertTrue(L[0].startswith("FILE big.txt"))
                if n >= 300:
                    self.assertLessEqual(len(out.encode()), n + 1, n)
                self.assertIn("(trimmed)", out)

    def test_untrimmed_when_it_fits(self):
        rc, out = cli(fx("buffhud_excerpt.txt"), "--max-bytes", 100000)
        self.assertNotIn("(trimmed)", out)

    def test_top_shrinks_before_alerts(self):
        with tempfile.TemporaryDirectory() as d:
            p = self.big(d)
            full = cli(p, "--max-bytes", 100000)[1]
            mid = cli(p, "--max-bytes", 1200)[1]
        count_top = lambda t: sum(1 for l in lines_of(t) if l.split("  ")[0].isdigit() and len(l.split("  ")) == 3)
        self.assertLess(count_top(mid), count_top(full))
        self.assertIn("alerts:", mid)


class NewBookmarks(TmpCase):
    def run_new(self, path, *extra):
        return cli(path, "--new", "--state", self.state, *extra)

    def test_first_run_reads_all_then_advances(self):
        p = self.copy("synth_multi.txt")
        rc, out = self.run_new(p)
        self.assertIn("new: lines 1-16 (16 new)", out)
        st = jload(self.state)
        key = os.path.realpath(p)
        self.assertEqual(st[key]["line"], 16)
        self.assertEqual(st[key]["size"], os.path.getsize(p))
        self.assertEqual(st[key]["inode"], os.stat(p).st_ino)
        rc, out = self.run_new(p)
        self.assertEqual(lines_of(out), ["FILE synth_multi.txt | %d B | 16 lines" % os.path.getsize(p),
                                         "VERDICT: EMPTY  launches=0 lines=0"])

    def test_only_new_lines_analyzed_and_absolute_numbers(self):
        p = self.copy("synth_multi.txt")
        self.run_new(p)
        self.append(p, "[400 f0] demo-mod 1.0.2 loaded; pid=4\n[401 f1] tick 9\n[402 f1] warn: something odd\n")
        rc, out = self.run_new(p)
        L = lines_of(out)
        self.assertIn("new: lines 17-19 (3 new)", L)
        self.assertIn("launches: 1 (showing #1, lines 17-19)", L)
        self.assertIn("1  19  warn: something odd", L)
        self.assertTrue(L[-1].startswith("VERDICT: ALERTS(1)  launches=1 lines=3"))
        self.assertEqual(jload(self.state)[os.path.realpath(p)]["line"], 19)

    def test_launch_split_inside_new_region(self):
        p = self.copy("synth_multi.txt")
        self.run_new(p)
        self.append(p, "[400 f0] demo-mod 1.0.2 loaded; pid=4\n[401] a\n[500 f0] demo-mod 1.0.3 loaded; pid=5\n[501] b\n")
        rc, out = self.run_new(p)
        self.assertIn("launches: 2 (showing #2, lines 19-20)", out)

    def test_continuation_without_banner_is_one_launch(self):
        p = self.copy("synth_multi.txt")
        self.run_new(p)
        self.append(p, "[400 f1] tick 9\n[401 f1] tick 10\n")
        rc, out = self.run_new(p)
        self.assertIn("launches: 1 (showing #1, lines 17-18)", out)

    def test_no_advance_keeps_bookmark(self):
        p = self.copy("synth_multi.txt")
        self.run_new(p, "--no-advance")
        self.assertFalse(os.path.exists(self.state))
        self.run_new(p)
        self.append(p, "[400 f1] x\n")
        before = slurp(self.state)
        rc, out1 = self.run_new(p, "--no-advance")
        rc, out2 = self.run_new(p, "--no-advance")
        self.assertEqual(out1, out2)
        self.assertEqual(slurp(self.state), before)
        self.assertIn("new: lines 17-17 (1 new)", out1)

    def test_rotation_on_shrink(self):
        p = self.copy("synth_multi.txt")
        self.run_new(p)
        with open(p, "w") as f:
            f.write("[1 f0] demo-mod 2.0 loaded; pid=9\n[2] hi\n")
        rc, out = self.run_new(p)
        L = lines_of(out)
        self.assertIn("rotated: reading from line 1", L)
        self.assertIn("new: lines 1-2 (2 new)", L)
        self.assertEqual(jload(self.state)[os.path.realpath(p)]["line"], 2)

    def test_rotation_on_inode_change(self):
        p = self.copy("synth_multi.txt")
        self.run_new(p)
        new = self.copy("synth_multi.txt", "replacement.txt")
        self.append(new, "[9] more\n")
        os.replace(new, p)  # new inode, bigger size: only the inode check can notice
        rc, out = self.run_new(p)
        self.assertIn("rotated: reading from line 1", out)
        self.assertIn("new: lines 1-17 (17 new)", out)

    def test_rotation_when_bookmark_beyond_eof_same_size(self):
        p = self.copy("synth_multi.txt")
        key = os.path.realpath(p)
        os.makedirs(os.path.dirname(self.state))
        Path(self.state).write_text(json.dumps({key: dict(inode=os.stat(p).st_ino, size=1, line=500)}))
        rc, out = self.run_new(p)
        self.assertIn("rotated: reading from line 1", out)

    def test_bookmarks_per_file_independent(self):
        a = self.copy("synth_multi.txt")
        b = self.copy("buffhud_excerpt.txt")
        rc, out = cli(a, b, "--new", "--state", self.state)
        st = jload(self.state)
        self.assertEqual({v["line"] for v in st.values()}, {16, 25})
        self.append(a, "[7] more\n")
        rc, out = cli(a, b, "--new", "--state", self.state)
        self.assertIn("new: lines 17-17 (1 new)", out)
        self.assertEqual(out.count("VERDICT: EMPTY"), 1)

    def test_missing_file_does_not_touch_state(self):
        rc, out = cli(os.path.join(self.tmp, "nope.txt"), "--new", "--state", self.state)
        self.assertEqual(rc, 2)
        self.assertFalse(os.path.exists(self.state))

    def test_corrupt_state_file_ignored(self):
        os.makedirs(os.path.dirname(self.state))
        Path(self.state).write_text("{not json")
        p = self.copy("synth_multi.txt")
        rc, out = self.run_new(p)
        self.assertEqual(rc, 0)
        self.assertIn("new: lines 1-16 (16 new)", out)

    def test_no_state_written_without_new(self):
        p = self.copy("synth_multi.txt")
        cli(p, "--state", self.state)
        self.assertFalse(os.path.exists(self.state))


class JsonShape(unittest.TestCase):
    def test_one_object_per_file_sorted_keys(self):
        rc, out = cli(fx("synth_multi.txt"), fx("buffhud_excerpt.txt"), "--json")
        self.assertEqual(rc, 0)
        L = lines_of(out)
        self.assertEqual(len(L), 2)
        objs = [json.loads(x) for x in L]
        for raw, o in zip(L, objs):
            self.assertEqual(raw, json.dumps(o, sort_keys=True))
        o = objs[0]
        for k in ("file", "path", "size", "total_lines", "launches", "shown", "verdict", "verdict_word"):
            self.assertIn(k, o)
        self.assertEqual((o["file"], o["launches"], o["total_lines"], o["verdict_word"]),
                         ("synth_multi.txt", 3, 16, "ALERTS(2)"))
        s = o["shown"][0]
        for k in ("n", "start", "end", "lines", "banner", "unique", "top", "alerts", "cap", "tail", "alerts_total"):
            self.assertIn(k, s)
        self.assertEqual((s["n"], s["start"], s["end"]), (3, 11, 16))
        self.assertEqual(s["top"][0], {"count": 2, "line": 12, "sig": "tick # pos=(#,#) addr=#"})
        self.assertTrue(s["cap"])
        self.assertEqual(o["verdict"], "VERDICT: ALERTS(2)  launches=3 lines=6")

    def test_json_top_respects_option_and_new_block(self):
        rc, out = cli(fx("synth_multi.txt"), "--json", "--top", 1)
        self.assertEqual(len(json.loads(out)["shown"][0]["top"]), 1)

    def test_json_deterministic(self):
        self.assertEqual(cli(fx("hotbar_excerpt.txt"), "--json"), cli(fx("hotbar_excerpt.txt"), "--json"))

    def test_json_missing_and_new(self):
        rc, out = cli(fx("synth_multi.txt"), "/nonexistent/q.txt", "--json")
        self.assertEqual(rc, 2)
        self.assertIn("missing: /nonexistent/q.txt", out)
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, "a.txt")
            shutil.copy(fx("synth_multi.txt"), p)
            rc, out = cli(p, "--json", "--new", "--no-advance", "--state", os.path.join(d, "s.json"))
        o = json.loads(out)
        self.assertEqual(o["new"], {"count": 16, "from": 1, "rotated": False, "to": 16})


class RealFixtures(unittest.TestCase):
    def test_every_fixture_runs_within_budget_and_ends_with_verdict(self):
        for name in sorted(os.listdir(FX)):
            if not name.endswith(".txt") or name == "README.txt":
                continue
            rc, out = cli(fx(name))
            self.assertEqual(rc, 0, name)
            self.assertLessEqual(len(out.encode()), 1800, name)
            self.assertTrue(lines_of(out)[-1].startswith("VERDICT: "), name)

    def test_kplog_prefix_kept_as_patch_channel(self):
        rc, out = cli(fx("kplog_mouseprobe_excerpt.txt"))
        self.assertIn("mouse-probe/raw RAW # # # #", out)

    def test_chareffects_second_launch(self):
        rc, out = cli(fx("chareffects_excerpt.txt"))
        self.assertIn("launches: 2 (showing #2, lines 23-45)", out)

    def test_equipiconfix_stats_pattern(self):
        rc, out = cli(fx("equipiconfix_excerpt.txt"), "--launch", 1)
        self.assertIn("STATS passes=# deferred=#", out)

    def test_levelup_prefix_and_banner(self):
        rc, out = cli(fx("levelup_excerpt.txt"), "--launch", 1)
        self.assertIn("launches: 2 (showing #1, lines 1-31)", out)


if __name__ == "__main__":
    unittest.main()
