"""Tests for fake_rea_mcp.py. REA itself is never started; the fake is driven over pipes."""

import json
import os
import queue
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
FAKE = os.path.join(HERE, "fake_rea_mcp.py")
FIXTURES = os.path.join(HERE, "fixtures", "rea")

EXPECTED_TOOLS = [
    "open_binary",
    "close_binary",
    "procedure_pseudo_code",
    "procedure_info",
    "procedure_assembly",
    "procedure_callers",
    "procedure_callees",
    "procedure_references",
    "xrefs",
    "analyze_function",
    "search_procedures",
    "search_strings",
    "read_bytes",
    "read_function_instructions",
    "resolve_containing_procedure",
    "batch_decompile",
    "resolve_native_call_targets",
    "set_addresses_names",
    "set_address_name",
    "annotate_native_function",
]


def result_text(reply):
    return reply["result"]["content"][0]["text"]


def result_payload(reply):
    return json.loads(result_text(reply))


class FakeRea:
    """One fake_rea_mcp.py subprocess. Every read has a timeout, so a hang fails the test."""

    def __init__(self, env, stderr_path):
        full_env = {key: value for key, value in os.environ.items() if not key.startswith("FAKE_REA_")}
        full_env.update(env)
        self._stderr = open(stderr_path, "wb")
        self.stderr_path = stderr_path
        self.proc = subprocess.Popen(
            [sys.executable, FAKE],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=self._stderr,
            env=full_env,
        )
        self._lines = queue.Queue()
        self._next_id = 1
        self._reader = threading.Thread(target=self._pump, daemon=True)
        self._reader.start()

    def _pump(self):
        for raw in iter(self.proc.stdout.readline, b""):
            self._lines.put(raw.decode("utf-8", "replace").rstrip("\n"))
        self._lines.put(None)

    def send(self, message):
        text = message if isinstance(message, str) else json.dumps(message)
        self.proc.stdin.write((text + "\n").encode("utf-8"))
        self.proc.stdin.flush()

    def read_line(self, timeout=5.0):
        """Next stdout line, None at EOF, TimeoutError when nothing arrives in time."""
        try:
            return self._lines.get(timeout=timeout)
        except queue.Empty:
            raise TimeoutError("no line from fake_rea_mcp within %.1f s" % timeout)

    def read_json(self, timeout=5.0):
        line = self.read_line(timeout)
        if line is None:
            raise AssertionError("fake_rea_mcp closed stdout (EOF) where a reply was expected")
        return json.loads(line)

    def request(self, method, params=None, timeout=5.0):
        request_id = self._next_id
        self._next_id += 1
        message = {"jsonrpc": "2.0", "id": request_id, "method": method}
        if params is not None:
            message["params"] = params
        self.send(message)
        return self.read_json(timeout)

    def handshake(self, params=None):
        reply = self.request("initialize", params if params is not None else {})
        self.send({"jsonrpc": "2.0", "method": "notifications/initialized"})
        return reply

    def call(self, name, arguments=None, timeout=5.0):
        arguments = arguments if arguments is not None else {}
        return self.request("tools/call", {"name": name, "arguments": arguments}, timeout)

    def send_call(self, request_id, name, arguments):
        self.send({"jsonrpc": "2.0", "id": request_id, "method": "tools/call",
                   "params": {"name": name, "arguments": arguments}})

    def shutdown(self, timeout=5.0):
        """Close stdin, wait for exit (kill on timeout), return the exit code."""
        try:
            self.proc.stdin.close()
        except OSError:
            pass
        try:
            code = self.proc.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            self.proc.kill()
            code = self.proc.wait()
        self._reader.join(timeout=2)
        self.proc.stdout.close()
        self._stderr.close()
        return code

    def stderr_text(self):
        with open(self.stderr_path, encoding="utf-8", errors="replace") as handle:
            return handle.read()


class FakeReaTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.mkdtemp(prefix="fake_rea_test_")
        self.addCleanup(shutil.rmtree, self._tmp, True)

    def start(self, **env):
        handle, stderr_path = tempfile.mkstemp(dir=self._tmp, suffix=".stderr")
        os.close(handle)
        fake = FakeRea(env, stderr_path)
        self.addCleanup(fake.shutdown)
        return fake

    def test_handshake_reply_shape_and_initialized_notification(self):
        fake = self.start()
        reply = fake.handshake()
        self.assertEqual(reply["jsonrpc"], "2.0")
        self.assertEqual(reply["id"], 1)
        self.assertEqual(reply["result"], {
            "protocolVersion": "2025-06-18",
            "capabilities": {"tools": {}},
            "serverInfo": {"name": "fake-rea", "version": "0"},
        })
        self.assertEqual(fake.shutdown(), 0)
        self.assertIn("notifications/initialized received", fake.stderr_text())

    def test_initialize_echoes_protocol_version(self):
        fake = self.start()
        reply = fake.request("initialize", {"protocolVersion": "2024-11-05", "capabilities": {}})
        self.assertEqual(reply["result"]["protocolVersion"], "2024-11-05")

    def test_tools_list_has_all_20_tools(self):
        fake = self.start()
        fake.handshake()
        reply = fake.request("tools/list")
        names = [tool["name"] for tool in reply["result"]["tools"]]
        self.assertEqual(len(names), 20)
        self.assertEqual(names, EXPECTED_TOOLS)

    def test_tools_drop_env_omits_named_tools(self):
        fake = self.start(FAKE_REA_TOOLS_DROP="xrefs, read_bytes")
        fake.handshake()
        names = [tool["name"] for tool in fake.request("tools/list")["result"]["tools"]]
        self.assertNotIn("xrefs", names)
        self.assertNotIn("read_bytes", names)
        self.assertEqual(len(names), 18)

    def test_procedure_info_known_address_returns_fixture(self):
        fake = self.start()
        fake.handshake()
        reply = fake.call("procedure_info", {"procedure": "0x776db0"})
        self.assertNotIn("isError", reply["result"])
        self.assertEqual(result_payload(reply)["normalized_result"]["entrypoint"], "0x776db0")

    def test_unknown_address_is_iserror(self):
        fake = self.start()
        fake.handshake()
        reply = fake.call("procedure_info", {"address": "0xdeadbeef"})
        self.assertTrue(reply["result"]["isError"])
        self.assertEqual(result_text(reply), "unknown address 0xdeadbeef for procedure_info")

    def test_batch_decompile_mixed_known_and_unknown(self):
        fake = self.start()
        fake.handshake()
        envelope = result_payload(fake.call("batch_decompile", {"addresses": ["0x776db0", "0x12345678"]}))
        self.assertEqual(envelope["operation"], "batch_decompile")
        payload = envelope["normalized_result"]
        self.assertEqual(payload["total"], 2)
        self.assertEqual(payload["succeeded"], 1)
        self.assertEqual(payload["failed"], 1)
        self.assertEqual(payload["items"][0]["status"], "ok")
        self.assertIn("void FUN_00776db0(int param_1)", payload["items"][0]["pseudocode"])
        self.assertEqual(payload["items"][1]["status"], "error")
        self.assertEqual(payload["items"][1]["error"], "unknown address")

    def test_set_names_then_pseudo_code_renames_token(self):
        fake = self.start()
        fake.handshake()
        applied = result_payload(fake.call("set_addresses_names", {
            "names": {"0x776db0": "sub_renamed", "zzz": "bad_key"},
        }))
        self.assertEqual(applied, {"0x00776db0": True, "zzz": False})
        code = result_payload(fake.call("procedure_pseudo_code", {"address": "0x776db0"}))["normalized_result"]
        self.assertIn("void sub_renamed(int param_1)", code)
        self.assertNotIn("FUN_00776db0", code)

    def test_name_substitution_respects_word_boundaries(self):
        fixtures = os.path.join(self._tmp, "fixtures")
        shutil.copytree(FIXTURES, fixtures)
        path = os.path.join(fixtures, "procedure_pseudo_code__0x00776db0.json")
        with open(path, encoding="utf-8") as handle:
            data = json.load(handle)
        data["normalized_result"] = "void FUN_00776db0(void)\n{\n  FUN_00776db0b();\n  return;\n}\n"
        with open(path, "w", encoding="utf-8") as handle:
            json.dump(data, handle)

        fake = self.start(FAKE_REA_FIXTURES=fixtures)
        fake.handshake()
        fake.call("set_addresses_names", {"names": {"0x776db0": "my_entry"}})
        code = result_payload(fake.call("procedure_pseudo_code", {"address": "0x776db0"}))["normalized_result"]
        self.assertIn("void my_entry(void)", code)
        self.assertIn("FUN_00776db0b();", code)
        self.assertNotIn("FUN_00776db0(", code)

        batch = result_payload(fake.call("batch_decompile", {"addresses": ["0x776db0"]}))["normalized_result"]
        batch_code = batch["items"][0]["pseudocode"]
        self.assertIn("void my_entry(void)", batch_code)
        self.assertIn("FUN_00776db0b();", batch_code)

    def test_annotate_then_pseudo_code_shows_name_with_word_boundary(self):
        fixtures = os.path.join(self._tmp, "fixtures")
        shutil.copytree(FIXTURES, fixtures)
        path = os.path.join(fixtures, "procedure_pseudo_code__0x00776db0.json")
        with open(path, encoding="utf-8") as handle:
            data = json.load(handle)
        data["normalized_result"] = "void FUN_00776db0(void)\n{\n  FUN_00776db0b();\n  return;\n}\n"
        with open(path, "w", encoding="utf-8") as handle:
            json.dump(data, handle)
        fake = self.start(FAKE_REA_FIXTURES=fixtures)
        fake.handshake()
        reply = fake.call("annotate_native_function", {"procedure": "0x776db0", "name": "my_entry", "comment": "c"})
        self.assertNotIn("isError", reply["result"])
        annotations = result_payload(reply)["normalized_result"]["annotations"]
        self.assertEqual(annotations["name"], "my_entry")
        self.assertEqual(annotations["address"], "0x776db0")
        code = result_payload(fake.call("procedure_pseudo_code", {"address": "0x776db0"}))["normalized_result"]
        self.assertIn("void my_entry(void)", code)
        self.assertIn("FUN_00776db0b();", code)
        self.assertNotIn("FUN_00776db0(", code)

    def test_annotate_rejects_unknown_keys(self):
        fake = self.start()
        fake.handshake()
        reply = fake.call("annotate_native_function", {"procedure": "0x776db0", "name": "x", "bogus": 1})
        self.assertTrue(reply["result"]["isError"])
        self.assertIn("unrecognized key", result_text(reply))
        self.assertIn("bogus", result_text(reply))

    def test_annotate_fail_env_errors_only_listed_addresses(self):
        fake = self.start(FAKE_REA_ANNOTATE_FAIL="0x775930, 0x775950")
        fake.handshake()
        bad = fake.call("annotate_native_function", {"procedure": "0x775930", "name": "a"})
        self.assertTrue(bad["result"]["isError"])
        self.assertIn("0x00775930", result_text(bad))
        good = fake.call("annotate_native_function", {"procedure": "0x776db0", "name": "b"})
        self.assertNotIn("isError", good["result"])
        self.assertEqual(result_payload(good)["normalized_result"]["annotations"]["name"], "b")
        again = fake.call("annotate_native_function", {"procedure": "0x775950", "name": "c"})
        self.assertTrue(again["result"]["isError"])

    def test_real_names_mode_rejects_set_name_tools_with_capability_unavailable(self):
        fake = self.start(FAKE_REA_MODE="real_names")
        fake.handshake()
        many = fake.call("set_addresses_names", {"names": {"0x776db0": "x"}})
        self.assertTrue(many["result"]["isError"])
        self.assertIn("capability_unavailable", result_text(many))
        one = fake.call("set_address_name", {"address": "0x776db0", "name": "x"})
        self.assertTrue(one["result"]["isError"])
        self.assertIn("capability_unavailable", result_text(one))
        # annotate still works in real_names mode
        ok = fake.call("annotate_native_function", {"procedure": "0x776db0", "name": "x"})
        self.assertNotIn("isError", ok["result"])

    def test_batch_decompile_returns_evidence_envelope(self):
        fake = self.start()
        fake.handshake()
        envelope = result_payload(fake.call("batch_decompile", {"addresses": ["0x776db0"]}))
        self.assertEqual(set(envelope), {"operation", "parameters", "normalized_result"})
        self.assertEqual(envelope["operation"], "batch_decompile")
        self.assertEqual(envelope["parameters"], {"addresses": ["0x776db0"]})
        result = envelope["normalized_result"]
        self.assertEqual((result["total"], result["succeeded"], result["failed"]), (1, 1, 0))
        self.assertEqual(result["items"][0]["address"], "0x00776db0")

    def test_crash_after_1_gives_eof_and_exit_code_3(self):
        fake = self.start(FAKE_REA_MODE="crash_after:1")
        fake.handshake()
        self.assertNotIn("isError", fake.call("procedure_info", {"procedure": "0x776db0"})["result"])
        fake.send_call(50, "procedure_info", {"procedure": "0x776db0"})
        self.assertIsNone(fake.read_line())
        self.assertEqual(fake.shutdown(), 3)

    def test_crash_once_after_1_crashes_first_process_only(self):
        state = os.path.join(self._tmp, "state.json")
        env = {"FAKE_REA_MODE": "crash_once_after:1", "FAKE_REA_STATE": state}

        first = self.start(**env)
        first.handshake()
        self.assertNotIn("isError", first.call("procedure_info", {"procedure": "0x776db0"})["result"])
        first.send_call(50, "procedure_info", {"procedure": "0x776db0"})
        self.assertIsNone(first.read_line())
        self.assertEqual(first.shutdown(), 3)

        second = self.start(**env)
        second.handshake()
        for _ in range(2):
            self.assertNotIn("isError", second.call("procedure_info", {"procedure": "0x776db0"})["result"])
        self.assertEqual(second.shutdown(), 0)

    def test_slow_adds_delay_to_tools_call(self):
        fake = self.start(FAKE_REA_MODE="slow:0.2")
        fake.handshake()
        started = time.monotonic()
        fake.call("procedure_info", {"procedure": "0x776db0"})
        self.assertGreaterEqual(time.monotonic() - started, 0.2)

    def test_no_init_reply_never_answers_initialize(self):
        fake = self.start(FAKE_REA_MODE="no_init_reply")
        fake.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
        with self.assertRaises(TimeoutError):
            fake.read_line(timeout=1.0)
        self.assertIsNone(fake.proc.poll())

    def test_require_open_rejects_until_open_binary(self):
        fake = self.start(FAKE_REA_MODE="require_open")
        fake.handshake()
        reply = fake.call("procedure_info", {"procedure": "0x776db0"})
        self.assertTrue(reply["result"]["isError"])
        self.assertIn("No app is open", result_text(reply))
        opened = fake.call("open_binary", {"path": "/games/swkotor2.exe"})
        self.assertNotIn("isError", opened["result"])
        self.assertNotIn("isError", fake.call("procedure_info", {"procedure": "0x776db0"})["result"])

    def test_garbage_line_comes_before_first_reply(self):
        fake = self.start(FAKE_REA_MODE="garbage_line")
        fake.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}})
        first = fake.read_line()
        with self.assertRaises(ValueError):
            json.loads(first)
        self.assertEqual(fake.read_json()["id"], 1)

    def test_stale_reply_precedes_first_tool_reply(self):
        fake = self.start(FAKE_REA_MODE="stale_reply")
        fake.handshake()
        fake.send_call(42, "procedure_info", {"procedure": "0x776db0"})
        self.assertEqual(fake.read_json()["id"], 999999)
        real = fake.read_json()
        self.assertEqual(real["id"], 42)
        self.assertNotIn("isError", real["result"])

    def test_close_binary_rejects_snapshot_path(self):
        fake = self.start()
        fake.handshake()
        fake.call("open_binary", {"path": "/games/swkotor2.exe"})
        reply = fake.call("close_binary", {"snapshot_path": "/tmp/x.snap"})
        self.assertTrue(reply["result"]["isError"])
        self.assertEqual(result_text(reply), "snapshot_path not allowed in tests")

    def test_close_binary_rejects_unknown_key(self):
        fake = self.start()
        fake.handshake()
        reply = fake.call("close_binary", {"read_only": True})
        self.assertTrue(reply["result"]["isError"])
        self.assertEqual(result_text(reply), "unrecognized key(s) in object: read_only")

    def test_open_binary_rejects_read_only_key(self):
        fake = self.start()
        fake.handshake()
        reply = fake.call("open_binary", {"path": "/games/swkotor2.exe", "read_only": True})
        self.assertTrue(reply["result"]["isError"])
        self.assertEqual(result_text(reply), "unrecognized key(s) in object: read_only")

    def test_open_binary_accepts_path_and_provider_id(self):
        fake = self.start()
        fake.handshake()
        reply = fake.call("open_binary", {"path": "/games/swkotor2.exe", "provider_id": "ghidra"})
        self.assertNotIn("isError", reply["result"])
        payload = result_payload(reply)
        self.assertEqual(payload["path"], "/games/swkotor2.exe")
        self.assertNotIn("read_only", payload)

    def test_call_log_records_tool_order_and_arguments(self):
        log_path = os.path.join(self._tmp, "calls.jsonl")
        fake = self.start(FAKE_REA_CALL_LOG=log_path)
        fake.handshake()
        fake.call("open_binary", {"path": "/games/swkotor2.exe"})
        fake.call("procedure_info", {"procedure": "0x776db0"})
        fake.call("batch_decompile", {"addresses": ["0x776db0"]})
        fake.call("close_binary", {})
        self.assertEqual(fake.shutdown(), 0)
        with open(log_path, encoding="utf-8") as handle:
            entries = [json.loads(line) for line in handle if line.strip()]
        self.assertEqual([entry["tool"] for entry in entries],
                         ["open_binary", "procedure_info", "batch_decompile", "close_binary"])
        self.assertEqual(entries[0]["arguments"], {"path": "/games/swkotor2.exe"})
        self.assertEqual(entries[2]["arguments"], {"addresses": ["0x776db0"]})

    def test_errors_before_init_unknown_method_and_non_json_lines(self):
        fake = self.start()
        fake.send("{not json")
        reply = fake.request("tools/list")
        self.assertEqual(reply["error"]["code"], -32002)
        self.assertEqual(reply["error"]["message"], "not initialized")
        fake.handshake()
        reply = fake.request("resources/list")
        self.assertEqual(reply["error"]["code"], -32601)


if __name__ == "__main__":
    unittest.main()
