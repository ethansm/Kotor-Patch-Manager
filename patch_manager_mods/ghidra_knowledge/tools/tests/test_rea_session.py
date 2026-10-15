#!/usr/bin/env python3
"""Tests for rea_session.py. Every session talks to tests/fake_rea_mcp.py, never the real REA."""

import contextlib
import hashlib
import io
import json
import os
import shlex
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

TOOLS_DIR = Path(__file__).resolve().parents[1]
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

import rea_session as rs  # noqa: E402

FAKE = str(Path(__file__).resolve().with_name("fake_rea_mcp.py"))
ADDR_A = "0x00776db0"
ADDR_B = "0x007680a0"
WRITE_ARGS = {"names": {ADDR_A: "renamed_probe"}}


class FakeSessionCase(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.cache_dir = self.root / "cache"
        self.lock_dir = self.root / "lock"
        self.call_log = self.root / "calls.log"
        self.state = self.root / "state.json"
        self.exe = self.root / "swkotor2.exe"
        self.exe.write_bytes(b"not a real exe; the fake never reads it")
        self.sessions = []

    def tearDown(self):
        for session in self.sessions:
            try:
                session.close()
            except Exception:
                pass
        self._tmp.cleanup()

    def make(self, mode="", *, state=False, **kwargs):
        env = {"FAKE_REA_CALL_LOG": str(self.call_log)}
        if mode:
            env["FAKE_REA_MODE"] = mode
        if state:
            env["FAKE_REA_STATE"] = str(self.state)
        env.update(kwargs.pop("env", None) or {})
        opts = dict(cmd=[sys.executable, FAKE], env=env, cache_dir=self.cache_dir,
                    lock_dir=self.lock_dir, exe=self.exe, warm=None)
        opts.update(kwargs)
        session = rs.ReaSession(**opts)
        self.sessions.append(session)
        return session

    def logged(self):
        if not self.call_log.exists():
            return []
        return [json.loads(line) for line in self.call_log.read_text().splitlines() if line.strip()]

    def logged_tool(self, tool):
        return [entry for entry in self.logged() if entry["tool"] == tool]


class HandshakeTests(FakeSessionCase):
    def test_start_registers_the_20_offered_tools(self):
        s = self.make()
        s.start()
        self.assertEqual(len(s.tools), 20)
        self.assertIn("open_binary", s.tools)
        self.assertIn("set_addresses_names", s.tools)

    def test_tool_not_offered_is_refused_without_sending(self):
        s = self.make()
        s.start()
        with self.assertRaisesRegex(rs.ReaError, "tool not offered by server: no_such_tool"):
            s.call("no_such_tool", {})
        self.assertEqual(self.logged(), [])

    def test_no_initialize_reply_times_out_and_releases_lock(self):
        s = self.make("no_init_reply", open_timeout_s=1)
        with self.assertRaisesRegex(rs.ReaError, "timeout"):
            s.start()
        # The failed start must not keep the lock: a fresh session can start.
        fresh = self.make()
        fresh.start()
        fresh.close()


class ProtocolTests(FakeSessionCase):
    def _baseline(self):
        s = self.make()
        s.start()
        s.open()
        result = s.call("procedure_info", {"procedure": ADDR_A}, use_cache=False)
        s.close()
        return result

    def test_stale_reply_is_skipped(self):
        expected = self._baseline()
        s = self.make("stale_reply")
        s.start()
        s.open()
        self.assertEqual(s.call("procedure_info", {"procedure": ADDR_A}, use_cache=False), expected)

    def test_garbage_line_is_skipped(self):
        expected = self._baseline()
        s = self.make("garbage_line")
        s.start()
        s.open()
        self.assertEqual(s.call("procedure_info", {"procedure": ADDR_A}, use_cache=False), expected)

    def test_slow_reply_times_out(self):
        s = self.make("slow:2", call_timeout_s=0.5)
        s.start()
        s.open()
        with self.assertRaisesRegex(rs.ReaError, r"timeout after 0\.5s: procedure_info"):
            s.call("procedure_info", {"procedure": ADDR_A}, use_cache=False)

    def test_crash_once_reconnects_and_retries(self):
        s = self.make("crash_once_after:1", state=True)
        s.start()
        s.open()
        result = s.call("procedure_info", {"procedure": ADDR_A}, use_cache=False)
        self.assertIn("normalized_result", result)
        # The reconnect re-opened the binary before retrying the call.
        self.assertEqual(len(self.logged_tool("open_binary")), 2)

    def test_crash_on_every_process_is_lost_twice(self):
        s = self.make("crash_after:0")
        s.start()
        with self.assertRaisesRegex(rs.ReaError, "REA connection lost twice: procedure_info"):
            s.call("procedure_info", {"procedure": ADDR_A}, use_cache=False)

    def test_call_before_open_reports_no_app(self):
        s = self.make("require_open")
        s.start()
        with self.assertRaisesRegex(rs.ReaError, "No app is open"):
            s.call("procedure_info", {"procedure": ADDR_A}, use_cache=False)
        s.open()
        self.assertIn("normalized_result", s.call("procedure_info", {"procedure": ADDR_A}, use_cache=False))


class CallTests(FakeSessionCase):
    def test_identical_call_is_served_from_cache(self):
        s = self.make()
        s.start()
        s.open()
        first = s.call("procedure_info", {"procedure": ADDR_A})
        sent = len(self.logged_tool("procedure_info"))
        second = s.call("procedure_info", {"procedure": ADDR_A})
        self.assertEqual(len(self.logged_tool("procedure_info")), sent)
        self.assertEqual(first, second)
        self.assertTrue(any((self.cache_dir / "procedure_info").glob("*.json")))

    def test_open_binary_sends_exactly_path_and_provider(self):
        s = self.make()
        s.start()
        s.open()
        opened = self.logged_tool("open_binary")
        self.assertEqual(len(opened), 1)
        self.assertEqual(opened[0]["arguments"], {"path": str(self.exe), "provider_id": "ghidra"})

    def test_close_never_sends_snapshot_path(self):
        s = self.make()
        s.start()
        s.open()
        s.close()
        closed = self.logged_tool("close_binary")
        self.assertEqual(len(closed), 1)
        self.assertEqual(closed[0]["arguments"], {})

    def test_read_only_refuses_write_tool_without_sending(self):
        s = self.make()
        s.start()
        s.open()
        with self.assertRaisesRegex(rs.ReaError, "read_only"):
            s.call("set_addresses_names", WRITE_ARGS)
        self.assertEqual(self.logged_tool("set_addresses_names"), [])

    def test_writable_session_passes_write_tool_through_uncached(self):
        s = self.make(read_only=False)
        s.start()
        s.open()
        s.call("set_addresses_names", WRITE_ARGS)
        s.call("set_addresses_names", WRITE_ARGS)
        self.assertEqual(len(self.logged_tool("set_addresses_names")), 2)

    def test_decode_prefers_structured_content_then_json_text_then_plain_text(self):
        self.assertEqual(
            rs._decode_tool_result({"structuredContent": {"a": 1}, "content": [{"type": "text", "text": "{}"}]}, "t"),
            {"a": 1})
        self.assertEqual(rs._decode_tool_result({"content": [{"type": "text", "text": '{"b": 2}'}]}, "t"), {"b": 2})
        self.assertEqual(rs._decode_tool_result({"content": [{"type": "text", "text": "plain"}]}, "t"),
                         {"text": "plain"})

    def test_is_error_raises_with_text(self):
        with self.assertRaisesRegex(rs.ReaError, "No app is open"):
            rs._decode_tool_result(
                {"isError": True, "content": [{"type": "text", "text": "No app is open. Call open_binary first."}]},
                "procedure_info")


class LockTests(FakeSessionCase):
    def test_second_session_is_blocked_until_first_closes(self):
        first = self.make()
        first.start()
        self.assertTrue(any(self.lock_dir.glob("*.lock")))
        second = self.make()
        with self.assertRaises(rs.ReaGuardError):
            second.start()
        first.close()
        second.start()
        second.close()

    def test_lock_released_after_exception_inside_with(self):
        with self.assertRaises(RuntimeError):
            with self.make() as s:
                s.call("procedure_info", {"procedure": ADDR_A})
                raise RuntimeError("boom")
        fresh = self.make()
        fresh.start()
        fresh.close()


class GuardTests(unittest.TestCase):
    """ps_lines are `ps -eo pid,ppid,args`; one session is one process tree."""

    HEADER = "    PID    PPID COMMAND"
    TREE_A = [
        "  100  1 npm exec rea-agents@6.3.0 mcp",
        "  101  100 node /x/node_modules/.bin/rea-agents --foo mcp",
        "  102  101 java -cp x GhidraClassLoader ghidra.GhidraRun",
    ]
    TREE_B = [
        "  200  1 npm exec rea-agents@6.3.0 mcp",
        "  201  200 node /x/node_modules/.bin/rea-agents --foo mcp",
        "  202  201 java -cp x GhidraClassLoader ghidra.GhidraRun",
    ]

    def test_one_process_tree_is_one_session(self):
        lines = [self.HEADER] + self.TREE_A + ["  7  1 bash"]
        roots = rs.live_rea_procs(lines)
        self.assertEqual(len(roots), 1)
        self.assertIn("npm exec", roots[0])
        rs.guard(ps_lines=lines)

    def test_two_independent_trees_are_refused(self):
        lines = [self.HEADER] + self.TREE_A + self.TREE_B
        self.assertEqual(len(rs.live_rea_procs(lines)), 2)
        with self.assertRaisesRegex(rs.ReaGuardError, "2 live REA/Ghidra sessions"):
            rs.guard(ps_lines=lines)

    def test_lone_analyze_headless_plus_npm_tree_is_refused(self):
        lines = [self.HEADER, "  300  1 /ghidra/support/analyzeHeadless /proj Odyssey"] + self.TREE_A
        self.assertEqual(len(rs.live_rea_procs(lines)), 2)
        with self.assertRaises(rs.ReaGuardError):
            rs.guard(ps_lines=lines)

    def test_own_guard_command_line_is_ignored(self):
        lines = ["  401  1 python3 rea_session.py guard", "  402  1 java -cp x GhidraClassLoader"]
        self.assertEqual(len(rs.live_rea_procs(lines)), 1)
        rs.guard(ps_lines=lines)

    def test_rea_session_parent_does_not_hide_child_tree(self):
        lines = ["  500  1 python3 rea_session.py probe"] + [l.replace("  100  1", "  100  500") for l in self.TREE_A]
        self.assertEqual(len(rs.live_rea_procs(lines)), 1)

    def test_own_pid_is_excluded(self):
        lines = [f"  {os.getpid()}  1 python3 unrelated GhidraClassLoader", "  5  1 java -cp x GhidraClassLoader"]
        self.assertEqual(len(rs.live_rea_procs(lines)), 1)

    def test_ppid_cycle_does_not_hang(self):
        lines = ["  10  11 java GhidraClassLoader", "  11  10 java GhidraClassLoader"]
        self.assertIsInstance(rs.live_rea_procs(lines), list)  # impossible in real ps; just must terminate

    def test_no_matches_passes(self):
        rs.guard(ps_lines=[self.HEADER, "  1  0 /sbin/init"])


class LifecycleTests(FakeSessionCase):
    def test_close_is_idempotent_and_reaps_the_child(self):
        s = self.make()
        s.start()
        s.open()
        proc = s._proc
        reader = s._reader
        s.close()
        s.close()
        self.assertIsNotNone(proc.poll())
        self.assertTrue(reader.daemon)
        self.assertFalse(reader.is_alive())

    def test_close_releases_lock_when_close_binary_raises(self):
        s = self.make()
        s.start()
        s.open()
        with mock.patch.object(s, "_request", side_effect=rs.ReaError("boom")):
            s.close()
        fresh = self.make()
        fresh.start()
        fresh.close()

    def test_close_releases_lock_when_process_already_dead(self):
        s = self.make()
        s.start()
        s.open()
        s._proc.kill()
        s._proc.wait()
        s.close()
        fresh = self.make()
        fresh.start()
        fresh.close()

    def test_close_releases_lock_even_if_stop_proc_raises(self):
        s = self.make()
        s.start()
        with mock.patch.object(s, "_stop_proc", side_effect=OSError("x")):
            with self.assertRaises(OSError):
                s.close()
        s._stop_proc = rs.ReaSession._stop_proc.__get__(s)
        fresh = self.make()
        fresh.start()
        fresh.close()

    def test_reconnect_reaps_old_process_and_keeps_lock(self):
        s = self.make("crash_once_after:1", state=True)
        s.start()
        s.open()
        old = s._proc
        s.call("procedure_info", {"procedure": ADDR_A}, use_cache=False)
        self.assertIsNot(s._proc, old)
        self.assertIsNotNone(old.poll())
        with self.assertRaises(rs.ReaGuardError):
            self.make().start()

    def test_handshake_sequence_on_the_wire(self):
        s = self.make()
        sent = []
        orig = rs.ReaSession._send

        def spy(self_, message):
            sent.append(message)
            return orig(self_, message)

        with mock.patch.object(rs.ReaSession, "_send", spy):
            s.start()
        self.assertEqual([m["method"] for m in sent], ["initialize", "notifications/initialized", "tools/list"])
        self.assertNotIn("id", sent[1])
        self.assertEqual(sent[0]["params"]["protocolVersion"], "2025-06-18")
        self.assertEqual(sent[0]["params"]["capabilities"], {})

    def test_failed_calls_are_not_cached(self):
        s = self.make("require_open")
        s.start()
        with self.assertRaises(rs.ReaError):
            s.call("procedure_info", {"procedure": ADDR_A})
        self.assertFalse(list((self.cache_dir / "procedure_info").glob("*.json")))

    def test_cache_key_is_sha1_of_tool_and_canonical_json(self):
        s = self.make()
        path = s._cache_path("procedure_info", {"b": 1, "a": 2})
        key = hashlib.sha1(b'procedure_info\n{"a":2,"b":1}').hexdigest()
        self.assertEqual(path, self.cache_dir / "procedure_info" / f"{key}.json")


class PreflightTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.payload = b"abc"
        self.sha = hashlib.sha256(self.payload).hexdigest()

    def tearDown(self):
        self._tmp.cleanup()

    def _write(self, path):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(self.payload)
        return path

    def test_matching_digest_passes(self):
        rs.preflight(exe=self._write(self.root / "swkotor2.exe"), sha=self.sha)

    def test_wrong_digest_is_refused(self):
        with self.assertRaisesRegex(rs.ReaError, "sha256 mismatch"):
            rs.preflight(exe=self._write(self.root / "swkotor2.exe"), sha="0" * 64)

    def test_steamapps_path_is_refused(self):
        exe = self._write(self.root / "steamapps" / "common" / "swkotor2.exe")
        with self.assertRaisesRegex(rs.ReaError, "steamapps"):
            rs.preflight(exe=exe, sha=self.sha)

    def test_missing_file_is_refused(self):
        with self.assertRaisesRegex(rs.ReaError, "not found"):
            rs.preflight(exe=self.root / "missing.exe", sha=self.sha)


class TelemetryTests(FakeSessionCase):
    def test_header_once_and_six_field_rows(self):
        row = rs.telemetry_row(self.cache_dir, "procedure_info", ADDR_A, 0.12345, True, pid=None)
        rs.telemetry_row(self.cache_dir, "open_binary", "", 1.0, False)
        lines = (self.cache_dir / "runs.tsv").read_text().splitlines()
        self.assertEqual(lines[0], rs.TELEMETRY_HEADER)
        self.assertEqual(len(lines), 3)
        self.assertEqual(lines[1], row)
        for line in lines[1:]:
            fields = line.split("\t")
            self.assertEqual(len(fields), 6)
            self.assertIn(fields[4], {"0", "1"})
        self.assertEqual(lines[1].split("\t")[3], "0.123")
        self.assertEqual(lines[1].split("\t")[5], "")

    def test_rss_is_read_from_proc_for_a_live_pid(self):
        if not Path("/proc/self/status").exists():
            self.skipTest("no /proc")
        row = rs.telemetry_row(self.cache_dir, "procedure_info", ADDR_A, 0.1, True, pid=os.getpid())
        self.assertTrue(row.split("\t")[5].isdigit())

    def test_session_calls_write_rows(self):
        s = self.make()
        s.start()
        s.open()
        s.call("procedure_info", {"procedure": ADDR_A}, use_cache=False)
        rows = [line.split("\t") for line in (self.cache_dir / "runs.tsv").read_text().splitlines()[1:]]
        tools = [r[1] for r in rows]
        self.assertEqual(tools, ["open_binary", "procedure_info"])
        self.assertEqual(rows[1][2], ADDR_A)
        self.assertTrue(all(r[4] == "1" for r in rows))


class EnvTests(unittest.TestCase):
    def test_child_env_has_rea_keys_and_extras(self):
        env = rs._child_env({"FAKE_EXTRA": "1"})
        for key, value in rs.REA_ENV.items():
            self.assertEqual(env[key], value)
        self.assertEqual(env["FAKE_EXTRA"], "1")
        self.assertEqual(rs._child_env()["JAVA_HOME"], rs.REA_ENV["JAVA_HOME"])


class CliTests(FakeSessionCase):
    def _cli(self, argv):
        out, err = io.StringIO(), io.StringIO()
        extra = ["--no-preflight", "--cmd", shlex.join([sys.executable, FAKE]),
                 "--cache-dir", str(self.cache_dir), "--lock-dir", str(self.lock_dir)]
        # The child inherits os.environ; patch.dict restores it after each test.
        with mock.patch.dict(os.environ, {"FAKE_REA_CALL_LOG": str(self.call_log)}):
            with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
                code = rs.main(argv + extra)
        return code, out.getvalue(), err.getvalue()

    def test_call_prints_json_result(self):
        code, out, _ = self._cli(["call", "procedure_info", "--args", json.dumps({"procedure": ADDR_A})])
        self.assertEqual(code, 0)
        self.assertIn("normalized_result", json.loads(out))

    def test_call_write_tool_without_flag_exits_1(self):
        code, _, err = self._cli(["call", "set_addresses_names", "--args", json.dumps(WRITE_ARGS)])
        self.assertEqual(code, 1)
        self.assertIn("read_only", err)

    def test_probe_prints_summary_line(self):
        code, out, _ = self._cli(["probe", "--addr", ADDR_A])
        self.assertEqual(code, 0)
        self.assertTrue(out.splitlines()[-1].startswith("median_s="))
        self.assertTrue(out.splitlines()[-1].endswith("calls=3"))

    def test_probe_median_line_and_exit_zero(self):
        code, out, _ = self._cli(["probe", "--addr", ADDR_A, "--addr", ADDR_B])
        self.assertEqual(code, 0)
        last = out.splitlines()[-1]
        self.assertRegex(last, r"^median_s=\d+\.\d{3} p95_s=\d+\.\d{3} calls=6$")

    def test_probe_and_call_run_preflight_and_guard_unless_skipped(self):
        for argv in (["probe"], ["call", "procedure_info"]):
            with mock.patch.object(rs, "guard") as g, mock.patch.object(rs, "preflight",
                                                                         side_effect=rs.ReaError("bad exe")) as p:
                err = io.StringIO()
                with contextlib.redirect_stderr(err):
                    code = rs.main(argv + ["--cmd", shlex.join([sys.executable, FAKE]),
                                           "--cache-dir", str(self.cache_dir), "--lock-dir", str(self.lock_dir)])
            self.assertEqual(code, 1, argv)
            g.assert_called_once()
            p.assert_called_once()

    def test_guard_failure_exits_3(self):
        with mock.patch.object(rs, "guard", side_effect=rs.ReaGuardError("busy")):
            with contextlib.redirect_stderr(io.StringIO()):
                code = rs.main(["probe", "--cache-dir", str(self.cache_dir)])
        self.assertEqual(code, 3)

    def test_guard_cli_prints_ok(self):
        with mock.patch.object(rs, "_ps_lines", return_value=["  1  0 /sbin/init"]):
            out = io.StringIO()
            with contextlib.redirect_stdout(out):
                code = rs.main(["guard"])
        self.assertEqual(code, 0)
        self.assertEqual(out.getvalue().strip(), "ok")



class WarmTests(FakeSessionCase):
    def test_open_runs_the_warm_call_under_open_timeout(self):
        s = self.make(warm=rs.WARM_CALL, call_timeout_s=0.01, open_timeout_s=10)
        s.start()
        s.open()
        self.assertEqual([e["tool"] for e in self.logged()], ["open_binary", "procedure_info"])
        self.assertIsNotNone(s.warm_wall_s)

    def test_reconnect_repeats_open_and_warm(self):
        s = self.make("crash_once_after:2", state=True, warm=rs.WARM_CALL)
        s.start()
        s.open()
        s.call("procedure_info", {"procedure": ADDR_A}, use_cache=False)
        self.assertEqual([e["tool"] for e in self.logged()],
                         ["open_binary", "procedure_info", "procedure_info",
                          "open_binary", "procedure_info", "procedure_info"])


if __name__ == "__main__":
    unittest.main()
