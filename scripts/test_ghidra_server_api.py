#!/usr/bin/env python3
"""Tests for ghidra_server_api.py.

Security: CRED_FILE is always patched to a temp path and load_credentials is patched
where needed. The real ~/.config/ghidra/.server_credentials is never opened. No network,
no Ghidra, no real subprocess (Popen/run are mocked and their argv/env/stdin recorded).
"""
import contextlib
import io
import json
import os
import sys
import tempfile
import types
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ghidra_server_api as gsa  # noqa: E402

FAKE_SECRET = "FAKE-SECRET-xyz123"
SUBCOMMANDS = ["list-tree", "export", "export-script", "export-types", "audit", "status"]


def fake_creds(password=FAKE_SECRET):
    return {"server": "fake.example", "port": 1234, "repo": "FakeRepo", "user": "FakeUser", "password": password}


class FakePopen:
    """Records constructor args and communicate() input; returns canned output."""
    instances = []
    stdout_text = ""
    stderr_text = ""
    returncode_value = 0

    def __init__(self, cmd, **kwargs):
        self.cmd = cmd
        self.kwargs = kwargs
        self.returncode = type(self).returncode_value
        self.input = None
        type(self).instances.append(self)

    def communicate(self, input=None, timeout=None):
        self.input = input
        return type(self).stdout_text, type(self).stderr_text


def make_popen(stdout="", stderr="", returncode=0):
    return type("P", (FakePopen,), {"instances": [], "stdout_text": stdout,
                                    "stderr_text": stderr, "returncode_value": returncode})


class Base(unittest.TestCase):
    def setUp(self):
        self._td = tempfile.TemporaryDirectory()
        self.addCleanup(self._td.cleanup)
        self.tmp = Path(self._td.name)
        self.cred_file = self.tmp / "nonexistent_vault"  # never created, never real
        self.runs = []
        env = {"PATH": os.environ.get("PATH", "/usr/bin:/bin"), "HOME": str(self.tmp)}
        patches = [
            mock.patch.dict(os.environ, env, clear=True),
            mock.patch.object(gsa, "CRED_FILE", self.cred_file),
            mock.patch.object(gsa, "DUMP_DIR", self.tmp / "dumps"),
            mock.patch.object(gsa.subprocess, "run", side_effect=self._fake_run),
            mock.patch.object(gsa.subprocess, "check_output", side_effect=AssertionError("unexpected")),
            mock.patch.object(gsa.os, "system", side_effect=AssertionError("unexpected os.system")),
        ]
        for p in patches:
            p.start()
            self.addCleanup(p.stop)

    def _fake_run(self, cmd, *a, **kw):
        self.runs.append((cmd, kw))
        return types.SimpleNamespace(returncode=1, stdout="", stderr="")

    def patch_popen(self, **kw):
        cls = make_popen(**kw)
        p = mock.patch.object(gsa.subprocess, "Popen", cls)
        p.start()
        self.addCleanup(p.stop)
        return cls

    def run_main(self, argv):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = gsa.main(argv)
        return rc, out.getvalue(), err.getvalue()


class ArgparseTest(Base):
    def test_top_level_help_exits_zero(self):
        out = io.StringIO()
        with contextlib.redirect_stdout(out), self.assertRaises(SystemExit) as cm:
            gsa.main(["--help"])
        self.assertEqual(cm.exception.code, 0)
        for sub in SUBCOMMANDS:
            self.assertIn(sub, out.getvalue())

    def test_each_subcommand_help_exits_zero(self):
        for sub in SUBCOMMANDS:
            with self.subTest(sub=sub):
                with contextlib.redirect_stdout(io.StringIO()), self.assertRaises(SystemExit) as cm:
                    gsa.main([sub, "--help"])
                self.assertEqual(cm.exception.code, 0)

    def test_required_args_enforced(self):
        for argv in (["export"], ["export-script", "p.exe"], ["export-script", "p.exe", "ExportX.java"],
                     ["export-types", "p.exe"]):
            with self.subTest(argv=argv):
                with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as cm:
                    gsa.main(argv)
                self.assertEqual(cm.exception.code, 2)

    def test_unknown_subcommand_exits_2(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as cm:
            gsa.main(["bogus"])
        self.assertEqual(cm.exception.code, 2)

    def test_no_command_prints_help_returns_zero(self):
        rc, out, _ = self.run_main([])
        self.assertEqual(rc, 0)
        self.assertIn("usage:", out)


class StatusTest(Base):
    def test_status_never_leaks_secret(self):
        popen = self.patch_popen()
        with mock.patch.object(gsa, "load_credentials", return_value=fake_creds()):
            rc, out, err = self.run_main(["status"])
        self.assertEqual(rc, 0)
        self.assertNotIn(FAKE_SECRET, out)
        self.assertNotIn(FAKE_SECRET, err)
        self.assertEqual(popen.instances, [])
        self.assertEqual(self.runs, [])
        data = json.loads(out)
        self.assertTrue(data["authenticated_ready"])
        self.assertEqual(data["server"], "fake.example")
        self.assertEqual(data["port"], 1234)
        self.assertEqual(data["vault_file"], str(self.cred_file))
        self.assertFalse(data["vault_exists"])
        self.assertNotIn("password", data)

    def test_status_missing_credentials(self):
        rc, out, _ = self.run_main(["status"])  # real load_credentials, empty env, tmp CRED_FILE
        self.assertEqual(rc, 0)
        data = json.loads(out)
        self.assertFalse(data["authenticated_ready"])
        self.assertEqual(data["server"], gsa.DEFAULT_SERVER)
        # load_credentials fell back to secret-tool via (mocked) subprocess.run only
        self.assertEqual([c[0][0] for c in self.runs], ["secret-tool"])

    def test_status_env_password_not_printed(self):
        with mock.patch.dict(os.environ, {"GHIDRA_PASS": FAKE_SECRET}):
            rc, out, err = self.run_main(["status"])
        self.assertEqual(rc, 0)
        self.assertTrue(json.loads(out)["authenticated_ready"])
        self.assertNotIn(FAKE_SECRET, out + err)
        self.assertEqual(self.runs, [])

    def test_load_credentials_reads_patched_tmp_file(self):
        vault = self.tmp / "tmp_vault_file"  # name avoids credential/secret patterns
        vault.write_text(f"# c\nexport GHIDRA_PASS='{FAKE_SECRET}'\nGHIDRA_USER=u1\nGHIDRA_PORT=99\n")
        with mock.patch.object(gsa, "CRED_FILE", vault):
            creds = gsa.load_credentials()
        self.assertEqual((creds["password"], creds["user"], creds["port"]), (FAKE_SECRET, "u1", 99))
        self.assertEqual(self.runs, [])


class MissingCredentialsTest(Base):
    def assert_missing(self, res):
        self.assertIn("error", res)
        self.assertTrue(res["error"].startswith("Missing credentials"))

    def test_list_remote_tree(self):
        popen = self.patch_popen()
        self.assert_missing(gsa.list_remote_tree("/TSL"))
        self.assertEqual(popen.instances, [])

    def test_export_program_symbols(self):
        popen = self.patch_popen()
        self.assert_missing(gsa.export_program_symbols("p.exe", output_dir=str(self.tmp / "o")))
        self.assertEqual(popen.instances, [])
        self.assertFalse((self.tmp / "o").exists())

    def test_main_list_tree_prints_error_rc0(self):
        popen = self.patch_popen()
        rc, out, _ = self.run_main(["list-tree"])
        self.assertEqual(rc, 0)
        self.assert_missing(json.loads(out))
        self.assertEqual(popen.instances, [])

    def test_main_export_prints_error_rc0(self):
        popen = self.patch_popen()
        rc, out, _ = self.run_main(["export", "p.exe", "--out", str(self.tmp / "o")])
        self.assertEqual(rc, 0)
        self.assert_missing(json.loads(out))
        self.assertEqual(popen.instances, [])

    def test_main_export_types_missing_creds_rc1(self):
        popen = self.patch_popen()
        rc, out, _ = self.run_main(["export-types", "p.exe", "--out", str(self.tmp / "o")])
        self.assertEqual(rc, 1)  # no returncode key in error dict -> not 0
        self.assert_missing(json.loads(out))
        self.assertEqual(popen.instances, [])


class CommandConstructionTest(Base):
    def setUp(self):
        super().setUp()
        p = mock.patch.object(gsa, "load_credentials", return_value=fake_creds())
        p.start()
        self.addCleanup(p.stop)

    def test_export_argv_and_password_via_stdin_only(self):
        popen = self.patch_popen(stdout="Password for FakeUser:\nline ***\nok line\n", returncode=0)
        out_dir = self.tmp / "out"
        rc, out, err = self.run_main(["export", "prog.exe", "--folder", "/K1/", "--out", str(out_dir)])
        self.assertEqual(rc, 0)
        (p,) = popen.instances
        self.assertEqual(p.cmd[0], str(gsa.GHIDRA_DIR / "support" / "analyzeHeadless"))
        self.assertEqual(p.cmd[1], "ghidra://fake.example:1234/FakeRepo/K1")
        for tok in ("-process", "prog.exe", "-readOnly", "-noanalysis", "-connect", "FakeUser", "-p"):
            self.assertIn(tok, p.cmd)
        i = p.cmd.index("-postScript")
        self.assertEqual(p.cmd[i + 1:i + 3], ["ExportProgramSymbols.java", str(out_dir)])
        self.assertEqual(p.cmd[p.cmd.index("-scriptPath") + 1], str(gsa.SCRIPT_PATH))
        self.assertEqual(p.input, FAKE_SECRET + "\n")
        self.assertTrue(out_dir.is_dir())
        # secret absent from argv, env, stdout, stderr
        self.assertFalse(any(FAKE_SECRET in a for a in p.cmd))
        self.assertNotIn("env", p.kwargs)
        self.assertNotIn(FAKE_SECRET, out + err)
        data = json.loads(out)
        self.assertEqual(data["stdout_summary"], "ok line")  # password prompt / *** lines filtered
        self.assertEqual(data["program"], "prog.exe")

    def test_export_default_folder_is_tsl(self):
        popen = self.patch_popen()
        self.run_main(["export", "prog.exe", "--out", str(self.tmp / "o")])
        self.assertTrue(popen.instances[0].cmd[1].endswith("/FakeRepo/TSL"))

    def test_export_stdout_summary_truncated(self):
        self.patch_popen(stdout="x" * 5000)
        _, out, _ = self.run_main(["export", "p.exe", "--out", str(self.tmp / "o")])
        self.assertEqual(len(json.loads(out)["stdout_summary"]), 1500)

    def test_export_script_valid(self):
        popen = self.patch_popen(returncode=0)
        rc, out, _ = self.run_main(["export-script", "p.exe", "ExportFingerprints.java", "--out", str(self.tmp / "o")])
        self.assertEqual(rc, 0)
        cmd = popen.instances[0].cmd
        self.assertEqual(cmd[cmd.index("-postScript") + 1], "ExportFingerprints.java")

    def test_export_script_nonzero_returncode_propagates(self):
        self.patch_popen(returncode=3)
        rc, out, _ = self.run_main(["export-script", "p.exe", "ExportFingerprints.java", "--out", str(self.tmp / "o")])
        self.assertEqual(rc, 1)
        self.assertEqual(json.loads(out)["returncode"], 3)

    def test_export_script_rejects_non_export_scripts(self):
        popen = self.patch_popen()
        for bad in ("Evil.java", "ExportX.py", "../ExportX.java", "sub/ExportX.java", "ExportX"):
            with self.subTest(script=bad):
                rc, out, _ = self.run_main(["export-script", "p.exe", bad, "--out", str(self.tmp / "o")])
                self.assertEqual(rc, 2)
                self.assertIn("refusing script", json.loads(out)["error"])
        self.assertEqual(popen.instances, [])

    def test_export_types_uses_datatypes_script(self):
        popen = self.patch_popen()
        rc, _, _ = self.run_main(["export-types", "p.exe", "--folder", "TSL", "--out", str(self.tmp / "o")])
        self.assertEqual(rc, 0)
        cmd = popen.instances[0].cmd
        self.assertEqual(cmd[cmd.index("-postScript") + 1], "ExportDataTypes.java")

    def test_list_tree_jshell_secret_only_in_env(self):
        popen = self.patch_popen(stdout='noise\nJSON_START\n{"subfolders":["a"],"items":[]}\nJSON_END\n')
        rc, out, err = self.run_main(["list-tree", "TSL"])
        self.assertEqual(rc, 0)
        self.assertEqual(json.loads(out), {"subfolders": ["a"], "items": []})
        (p,) = popen.instances
        self.assertEqual(p.cmd, [str(gsa.GHIDRA_DIR / "support" / "jshellRun")])
        self.assertEqual(p.kwargs["env"]["GHIDRA_PASS"], FAKE_SECRET)  # env, not argv
        self.assertFalse(any(FAKE_SECRET in a for a in p.cmd))
        self.assertNotIn(FAKE_SECRET, p.input)  # jshell source reads System.getenv
        self.assertIn('String targetFolder = "/TSL";', p.input)  # leading slash added
        self.assertIn('"FakeUser"', p.input)
        self.assertIn('getRepositoryServer("fake.example", 1234', p.input)
        self.assertIn('getRepository("FakeRepo")', p.input)
        self.assertNotIn(FAKE_SECRET, out + err)

    def test_list_tree_default_folder_root(self):
        popen = self.patch_popen(stdout="JSON_START\n{}\nJSON_END")
        self.run_main(["list-tree"])
        self.assertIn('String targetFolder = "/";', popen.instances[0].input)

    def test_list_tree_failure_and_parse_error(self):
        self.patch_popen(stdout="nothing", stderr="API_ERROR: boom")
        rc, out, _ = self.run_main(["list-tree"])
        self.assertEqual(json.loads(out), {"error": "Failed to list remote folder", "stderr": "API_ERROR: boom"})
        self.patch_popen(stdout="JSON_START\nnot json\nJSON_END")
        _, out, _ = self.run_main(["list-tree"])
        data = json.loads(out)
        self.assertEqual(data["raw_output"], "not json")
        self.assertIn("parse_error", data)

    def test_audit_delegates_to_module(self):
        fake = types.ModuleType("audit_server_knowledge")
        fake.main = mock.Mock(return_value=7)
        with mock.patch.dict(sys.modules, {"audit_server_knowledge": fake}):
            rc, _, _ = self.run_main(["audit"])
        self.assertEqual(rc, 7)
        fake.main.assert_called_once_with()


if __name__ == "__main__":
    unittest.main()
