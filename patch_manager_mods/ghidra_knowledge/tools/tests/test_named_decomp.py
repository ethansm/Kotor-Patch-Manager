#!/usr/bin/env python3
"""Tests for named_decomp.py. Live calls go to tests/fake_rea_mcp.py, never the real REA."""

import contextlib
import io
import json
import re
import shlex
import shutil
import sqlite3
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

TOOLS_DIR = Path(__file__).resolve().parents[1]
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

import named_decomp as nd  # noqa: E402

FAKE = str(TOOLS_DIR / "tests" / "fake_rea_mcp.py")
FIXTURES = TOOLS_DIR / "tests" / "fixtures" / "rea"
DUMP_MINI = TOOLS_DIR / "tests" / "fixtures" / "dump_mini"

# All three have procedure_pseudo_code fixtures; each body names itself in its signature line.
ADDR_A = 0x007680A0  # steam name CSWFoo::Foo
ADDR_B = 0x00776DB0  # steam name CSWFoo::Bar
ADDR_C = 0x00775970  # steam name CSWBar::Baz
ADDR_NOFIX = 0x00123456  # no fixture: the fake answers an error item


def make_kdb(path):
    con = sqlite3.connect(path)
    con.execute("DROP TABLE IF EXISTS name_map")
    con.execute("CREATE TABLE name_map (steam_addr TEXT, name TEXT, source TEXT, confidence TEXT)")
    con.commit()
    con.close()


def make_steam(path, rows):
    con = sqlite3.connect(path)
    con.execute("DROP TABLE IF EXISTS functions")
    con.execute("CREATE TABLE functions (id INTEGER PRIMARY KEY, class_name TEXT, function_name TEXT, "
                "address INTEGER)")
    con.executemany("INSERT INTO functions (class_name, function_name, address) VALUES (?,?,?)", rows)
    con.commit()
    con.close()


class DecompCase(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.cache = self.root / "cache"
        self.lock = self.root / "lock"
        self.call_log = self.root / "calls.log"
        self.exe = self.root / "swkotor2.exe"
        self.exe.write_bytes(b"not a real exe; the fake never reads it")
        self.fixtures = self.root / "fixtures"
        shutil.copytree(FIXTURES, self.fixtures)
        self.kdb = self.root / "kdb.db"
        make_kdb(self.kdb)
        self.steam = self.root / "steam.db"
        make_steam(self.steam, [("CSWFoo", "Foo", ADDR_A), ("CSWFoo", "Bar", ADDR_B),
                                ("CSWBar", "Baz", ADDR_C)])
        self.out = self.root / "out"
        self.addr_file = self.root / "addrs.txt"
        self.addr_file.write_text("0x007680a0\n0x00776db0\n0x00775970\n", encoding="utf-8")

    def tearDown(self):
        self._tmp.cleanup()

    def cli(self, *argv, out=None, env=None):
        fake_env = {"FAKE_REA_CALL_LOG": str(self.call_log), "FAKE_REA_FIXTURES": str(self.fixtures)}
        fake_env.update(env or {})
        overrides = {"exe": self.exe, "env": fake_env, "warm": None,
                     "cache_dir": self.cache, "lock_dir": self.lock}
        base = ["run", "--out", str(out or self.out), "--dump-root", str(DUMP_MINI),
                "--kdb", str(self.kdb), "--steam-db", str(self.steam),
                "--no-preflight", "--cmd", shlex.join([sys.executable, FAKE])]
        out_buf, err_buf = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out_buf), contextlib.redirect_stderr(err_buf):
            code = nd.main(base + [str(x) for x in argv], session_overrides=overrides)
        return code, out_buf.getvalue(), err_buf.getvalue()

    def calls(self):
        if not self.call_log.is_file():
            return []
        lines = self.call_log.read_text(encoding="utf-8").splitlines()
        return [json.loads(line) for line in lines if line.strip()]

    def manifest(self, out=None):
        lines = (Path(out or self.out) / "manifest.tsv").read_text(encoding="utf-8").splitlines()
        return [line.split("\t") for line in lines]

    def c_file(self, addr, out=None):
        return Path(out or self.out) / ("%08x" % addr)[2:4] / ("0x%08x.c" % addr)


class OrderAndChunkTests(DecompCase):
    def test_all_annotations_precede_first_batch(self):
        code, _, err = self.cli("--addrs", str(self.addr_file))
        self.assertEqual(code, 0, err)
        tools = [c["tool"] for c in self.calls()]
        self.assertIn("open_binary", tools)
        annotate = [i for i, t in enumerate(tools) if t == "annotate_native_function"]
        batch = [i for i, t in enumerate(tools) if t == "batch_decompile"]
        self.assertTrue(annotate)
        self.assertTrue(batch)
        self.assertLess(max(annotate), min(batch))
        self.assertNotIn("set_addresses_names", tools)
        self.assertNotIn("set_address_name", tools)
        annotated = [c["arguments"]["procedure"] for c in self.calls() if c["tool"] == "annotate_native_function"]
        self.assertEqual(annotated, ["0x%08x" % a for a in (ADDR_A, ADDR_C, ADDR_B)])  # ascending

    def test_batch_chunks_follow_batch_size(self):
        code, _, err = self.cli("--addrs", str(self.addr_file), "--batch", "2")
        self.assertEqual(code, 0, err)
        sizes = [len(c["arguments"]["addresses"]) for c in self.calls() if c["tool"] == "batch_decompile"]
        self.assertEqual(sizes, [2, 1])


class PostPassTests(DecompCase):
    def test_postpass_matches_whole_tokens_only(self):
        text, count = nd.postpass("FUN_0040a0b0 FUN_0040a0b0c FUN_0040a0", {0x0040A0B0: "Named"})
        self.assertEqual(text, "Named FUN_0040a0b0c FUN_0040a0")
        self.assertEqual(count, 1)

    def test_postpass_handles_uppercase_and_dat(self):
        text, count = nd.postpass("FUN_0040A0B0 DAT_0040a0b0 DAT_0040a0b0x", {0x0040A0B0: "Named"})
        self.assertEqual(text, "Named Named DAT_0040a0b0x")
        self.assertEqual(count, 2)


class RunTests(DecompCase):
    def test_annotate_failure_is_postpassed_not_fatal(self):
        code, out, err = self.cli("--addrs", str(self.addr_file),
                                  env={"FAKE_REA_ANNOTATE_FAIL": "0x00775970"})
        self.assertEqual(code, 0, err)
        self.assertIn("annotate 0x00775970 failed", err)
        self.assertRegex(out, r"annotated 2, annotate_failed 1")
        text = self.c_file(ADDR_C).read_text(encoding="utf-8")
        first, body = text.split("\n", 1)
        self.assertRegex(first, r"^// NAMED 0x00775970 CSWBar::Baz evidence=session-\d{8}T\d{6}Z "
                                r"names=postpass exe=[0-9a-f]{12}$")
        self.assertIn("void CSWBar::Baz(void)", body)
        self.assertNotIn("FUN_00775970", body)
        rows = {row[0]: row for row in self.manifest()[1:]}
        self.assertEqual(rows["0x00775970"][2], "ok")

    def test_resume_skips_everything_without_opening(self):
        code, _, err = self.cli("--addrs", str(self.addr_file))
        self.assertEqual(code, 0, err)
        self.call_log.unlink()
        code, out, err = self.cli("--addrs", str(self.addr_file))
        self.assertEqual(code, 0, err)
        self.assertEqual(self.calls(), [])
        self.assertRegex(out, r"skipped 3 \(resume\)")

    def test_manifest_is_deterministic_and_sorted(self):
        out1, out2 = self.root / "out1", self.root / "out2"
        self.assertEqual(self.cli("--addrs", str(self.addr_file), out=out1)[0], 0)
        self.assertEqual(self.cli("--addrs", str(self.addr_file), out=out2)[0], 0)
        rows1 = [row[:4] for row in self.manifest(out1)]
        rows2 = [row[:4] for row in self.manifest(out2)]
        self.assertEqual(rows1, rows2)
        self.assertEqual(rows1[0], ["addr", "name", "status", "bytes"])
        addrs = [row[0] for row in rows1[1:]]
        self.assertEqual(addrs, sorted(addrs))
        self.assertEqual(len(addrs), 3)

    def test_exe_sha_change_returns_3(self):
        with mock.patch.object(nd, "_sha256_file", side_effect=["a" * 64, "b" * 64]) as sha:
            code, _, err = self.cli("--addrs", str(self.addr_file))
        self.assertEqual(code, 3, err)
        self.assertIn("exe sha changed", err)
        self.assertEqual(sha.call_count, 2)
        self.assertTrue((self.out / "manifest.tsv").is_file())

    def test_class_selects_steam_db_rows(self):
        code, out, err = self.cli("--class", "CSWFoo")
        self.assertEqual(code, 0, err)
        self.assertIn("targets 2, ok 2, error 0", out)
        batch = [addr for c in self.calls() if c["tool"] == "batch_decompile"
                 for addr in c["arguments"]["addresses"]]
        self.assertEqual(sorted(batch), ["0x%08x" % ADDR_A, "0x%08x" % ADDR_B])
        self.assertTrue(self.c_file(ADDR_A).is_file())
        self.assertTrue(self.c_file(ADDR_B).is_file())
        self.assertFalse(self.c_file(ADDR_C).exists())

    def test_item_error_is_recorded_and_not_fatal(self):
        bad = self.root / "addrs_bad.txt"
        bad.write_text("0x007680a0\n0x00123456\n0x00775970\n", encoding="utf-8")
        code, out, err = self.cli("--addrs", str(bad))
        self.assertEqual(code, 0, err)
        rows = {row[0]: row for row in self.manifest()[1:]}
        self.assertEqual(rows["0x00123456"][2], "error")
        self.assertEqual(rows["0x00123456"][3], "0")
        self.assertFalse(self.c_file(ADDR_NOFIX).exists())
        self.assertEqual(rows["0x007680a0"][2], "ok")
        self.assertEqual(rows["0x00775970"][2], "ok")
        self.assertIn("ok 2, error 1", out)


class RobustnessTests(DecompCase):
    def names_file(self, rows):
        path = self.root / "names.tsv"
        path.write_text("".join("%s\t%s\n" % row for row in rows), encoding="utf-8")
        return "file:%s" % path

    def test_set_name_tools_never_called_even_with_failures(self):
        code, _, err = self.cli("--addrs", str(self.addr_file), env={"FAKE_REA_ANNOTATE_FAIL": "0x007680a0"})
        self.assertEqual(code, 0, err)
        tools = {c["tool"] for c in self.calls()}
        self.assertTrue({"set_addresses_names", "set_address_name"}.isdisjoint(tools))
        self.assertEqual(self.calls()[-1]["tool"], "close_binary")  # one session, closed at the end
        self.assertEqual([c["tool"] for c in self.calls()].count("open_binary"), 1)

    def test_failed_callee_name_is_postpassed_in_the_callers_file(self):
        # 0x00775970 calls 0x0085d320 (FUN_0085d320 in its body); only the callee's annotate fails.
        spec = self.names_file([("0x00775970", "CSWBar::Baz"), ("0x0085d320", "CSWBar::Callee")])
        code, _, err = self.cli("--addrs", str(self.addr_file), "--names-from", spec,
                                env={"FAKE_REA_ANNOTATE_FAIL": "0x0085d320"})
        self.assertEqual(code, 0, err)
        text = self.c_file(ADDR_C).read_text(encoding="utf-8")
        self.assertIn("names=postpass", text.split("\n", 1)[0])
        self.assertIn("CSWBar::Callee();", text)
        self.assertNotIn("FUN_0085d320", text)
        self.assertFalse(self.c_file(0x0085D320).exists())  # a callee is not a target

    def test_successful_callee_annotation_needs_no_postpass(self):
        spec = self.names_file([("0x00775970", "CSWBar::Baz"), ("0x0085d320", "CSWBar::Callee")])
        code, _, err = self.cli("--addrs", str(self.addr_file), "--names-from", spec)
        self.assertEqual(code, 0, err)
        text = self.c_file(ADDR_C).read_text(encoding="utf-8")
        self.assertRegex(text.split("\n", 1)[0], r"names=2 exe=")
        self.assertIn("CSWBar::Callee();", text)

    def test_odd_names_do_not_affect_filenames_or_manifest_shape(self):
        spec = self.names_file([("0x007680a0", "Weird Name::<int>~dtor"), ("0x00776db0", "ok"),
                                ("0x00775970", "x\u00e9")])
        code, _, err = self.cli("--addrs", str(self.addr_file), "--names-from", spec,
                                env={"FAKE_REA_ANNOTATE_FAIL": "0x007680a0"})
        self.assertEqual(code, 0, err)
        self.assertTrue(self.c_file(ADDR_A).is_file())
        first = self.c_file(ADDR_A).read_text(encoding="utf-8").split("\n", 1)[0]
        self.assertIn("Weird_Name::<int>~dtor", first)
        self.assertTrue(all(len(row) == 5 for row in self.manifest()))
        names = sorted(p.name for p in self.out.rglob("*") if p.is_file())
        self.assertEqual(names, ["0x007680a0.c", "0x00775970.c", "0x00776db0.c", "manifest.tsv"])  # no .tmp left

    def test_dead_session_mid_run_keeps_finished_chunks_and_exits_nonzero(self):
        state = self.root / "fake_state.json"
        # open + 3 annotates + first batch answered, then every call dies (reconnect fails too).
        code, out, err = self.cli("--addrs", str(self.addr_file), "--batch", "2",
                                  env={"FAKE_REA_MODE": "crash_after:5", "FAKE_REA_STATE": str(state)})
        self.assertEqual(code, 1, err)
        self.assertIn("batch_decompile chunk failed", err)
        rows = {row[0]: row for row in self.manifest()[1:]}
        self.assertEqual([rows["0x00775970"][2], rows["0x007680a0"][2], rows["0x00776db0"][2]],
                         ["ok", "ok", "error"])
        self.assertTrue(self.c_file(ADDR_C).is_file())
        self.assertFalse(self.c_file(ADDR_B).exists())
        # A healthy re-run resumes: only the failed address is decompiled again.
        self.call_log.unlink()
        code, _, err = self.cli("--addrs", str(self.addr_file), "--batch", "2")
        self.assertEqual(code, 0, err)
        batch = [a for c in self.calls() if c["tool"] == "batch_decompile" for a in c["arguments"]["addresses"]]
        self.assertEqual(batch, ["0x%08x" % ADDR_B])
        self.assertEqual({row[0]: row[2] for row in self.manifest()[1:]}, {
            "0x00775970": "ok", "0x007680a0": "ok", "0x00776db0": "ok"})

    def test_reconnect_loses_annotations_so_names_are_postpassed(self):
        state = self.root / "fake_state.json"
        # The single batch call crashes the fake once; the session restarts and retries, without annotations.
        code, _, err = self.cli("--addrs", str(self.addr_file),
                                env={"FAKE_REA_MODE": "crash_once_after:4", "FAKE_REA_STATE": str(state)})
        self.assertEqual(code, 0, err)
        text = self.c_file(ADDR_A).read_text(encoding="utf-8")
        first, body = text.split("\n", 1)
        self.assertIn("names=postpass", first)
        self.assertIn("CSWFoo::Foo(", body)
        self.assertNotIn("FUN_007680a0", body)

    def test_start_failure_returns_1_without_manifest(self):
        out_buf, err_buf = io.StringIO(), io.StringIO()
        overrides = {"exe": self.exe, "warm": None, "cache_dir": self.cache, "lock_dir": self.lock}
        argv = ["run", "--out", str(self.out), "--dump-root", str(DUMP_MINI), "--kdb", str(self.kdb),
                "--steam-db", str(self.steam), "--no-preflight", "--addrs", str(self.addr_file),
                "--cmd", str(self.root / "no_such_rea_binary")]
        with contextlib.redirect_stdout(out_buf), contextlib.redirect_stderr(err_buf):
            code = nd.main(argv, session_overrides=overrides)
        self.assertNotEqual(code, 0)
        self.assertFalse((self.out / "manifest.tsv").exists())

    def test_session_open_uses_default_open_timeout(self):
        seen = {}
        real = nd.rs.ReaSession

        class Spy(real):
            def __init__(self, *a, **kw):
                seen.update(kw)
                super().__init__(*a, **kw)
                seen["open_timeout_s"] = self.open_timeout_s

        with mock.patch.object(nd.rs, "ReaSession", Spy):
            code, _, err = self.cli("--addrs", str(self.addr_file))
        self.assertEqual(code, 0, err)
        self.assertEqual(seen["open_timeout_s"], 900)
        self.assertIs(seen["read_only"], False)

    def test_unknown_class_warns_and_selects_nothing(self):
        code, out, err = self.cli("--class", "NoSuchClass")
        self.assertEqual(code, 0, err)
        self.assertIn("no targets", err)
        self.assertEqual(self.calls(), [])


if __name__ == "__main__":
    unittest.main()
