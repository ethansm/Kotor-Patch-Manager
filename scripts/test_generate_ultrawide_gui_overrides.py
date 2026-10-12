#!/usr/bin/env python3
"""Host-only tests for generate_ultrawide_gui_overrides.py registration check (BG14).

Every run uses temp dirs; nothing touches a real game install or override/.
"""
import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

SCRIPTS = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPTS))
import generate_ultrawide_gui_overrides as gen  # noqa: E402


class RegistrationCheckTests(unittest.TestCase):
    def test_intact_constants_register_at_any_k(self):
        for k in (0.25, 0.5, 0.8, 1.0):
            with self.subTest(k=k):
                lines, ok = gen.registration_check(k)
                self.assertTrue(ok)
                self.assertIn("NOT engine-verified", lines[-1])

    def test_broken_tabs_anchor_is_mismatch_at_any_k(self):
        # Tabs 7 virtual units away from the content's right edge (tolerance is 6).
        with mock.patch.dict(gen.REGISTRATION, {"top_p tabs anchor": 730}):
            for k in (0.5, 1.0):
                with self.subTest(k=k):
                    lines, ok = gen.registration_check(k)
                    self.assertFalse(ok)
                    self.assertIn("MISMATCH", lines[-1])

    def test_portrait_outside_bar5_is_mismatch(self):
        with mock.patch.dict(gen.REGISTRATION, {"top_p portrait anchor": 760}):
            _, ok = gen.registration_check(0.6)
        self.assertFalse(ok)

    def test_nonpositive_k_rejected(self):
        with self.assertRaises(ValueError):
            gen.registration_check(0.0)


class MainHonorsRegistrationTests(unittest.TestCase):
    """main() must refuse to deploy when the registration check fails."""

    def _run_main(self, argv_extra):
        tmp = Path(tempfile.mkdtemp(prefix="bg14_"))
        argv = ["generate_ultrawide_gui_overrides.py", "--kotor2-root", str(tmp / "game"),
                "--workspace-dir", str(tmp / "ws"), "--content-aspect", "16:9"] + argv_extra
        targets = {"content": set(), "board": {}, "bars": {}, "popup": set(),
                   "hud_action": set(), "partial": set()}
        patches = [
            mock.patch.object(gen.sys, "argv", argv),
            mock.patch.object(gen, "is_game_running", return_value=False),
            mock.patch.object(gen, "discover_root_fills", return_value={}),
            mock.patch.object(gen, "plan_targets", return_value=targets),
            mock.patch.object(gen, "load_manifest", return_value={}),
            mock.patch.object(gen, "generate_all", return_value=({"fake_p": b"x"}, [], [])),
            mock.patch.object(gen, "template_source", return_value=tmp / "src.gui"),
            mock.patch.object(gen, "deploy_file"),
        ]
        for p in patches:
            p.start()
        self.addCleanup(mock.patch.stopall)
        out, err = io.StringIO(), io.StringIO()
        exit_code = None
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            try:
                gen.main()
            except SystemExit as exc:
                exit_code = exc.code
        report = (tmp / "ws" / "ultrawide_gui" / "report.txt").read_text(encoding="utf-8")
        return exit_code, out.getvalue(), err.getvalue(), report

    def test_mismatch_with_deploy_exits_nonzero_before_deploy(self):
        with mock.patch.dict(gen.REGISTRATION, {"top_p tabs anchor": 760}):
            exit_code, _, err, report = self._run_main(["--deploy"])
        self.assertNotEqual(exit_code, 0, "main must exit non-zero on a registration MISMATCH")
        self.assertIn("registration: MISMATCH", report)
        self.assertIn("refusing to deploy", err)
        gen.deploy_file.assert_not_called()

    def test_ok_registration_still_deploys(self):
        exit_code, _, _, report = self._run_main(["--deploy"])
        self.assertIsNone(exit_code)
        self.assertIn("registration: OK", report)
        gen.deploy_file.assert_called_once()


class IsGameRunningTests(unittest.TestCase):
    """BG15: a failed process probe must assume the game is running (fail closed)."""

    def _probe(self, **kwargs):
        err = io.StringIO()
        with mock.patch.object(gen.subprocess, "check_output", **kwargs), contextlib.redirect_stderr(err):
            return gen.is_game_running(), err.getvalue()

    def test_game_in_ps_output_is_running(self):
        result, _ = self._probe(return_value="  99 init\n1234 Z:\\games\\swkotor2.exe\n")
        self.assertTrue(result)

    def test_game_absent_from_ps_output_is_not_running(self):
        result, _ = self._probe(return_value="  99 init\n 100 bash\n")
        self.assertFalse(result)

    def test_missing_ps_fails_closed(self):
        result, err = self._probe(side_effect=FileNotFoundError("ps"))
        self.assertTrue(result)
        self.assertIn("assuming it is running", err)

    def test_ps_nonzero_exit_fails_closed(self):
        result, _ = self._probe(side_effect=gen.subprocess.CalledProcessError(1, "ps"))
        self.assertTrue(result)


if __name__ == "__main__":
    unittest.main()
