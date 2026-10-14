#!/usr/bin/env python3
"""Host-only tests for nwmove_check_config.py (synthetic data and tempfile files only; no game directory access)."""
import contextlib
import io
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import nwmove_check_config as m  # noqa: E402

SCRIPT = HERE / "nwmove_check_config.py"
REAL_HOOKS = HERE.parent / "patch_manager_mods" / "npc_world_src" / "kotor2-steam-aspyr.hooks.toml"


class SelftestCases(unittest.TestCase):
    def test_every_case(self):
        for name, cfg, state, hooks, exports, expect in m.selftest_cases():
            with self.subTest(name):
                errs = m.check(cfg, state, hooks, exports)
                if expect is None:
                    self.assertEqual(errs, [])
                else:
                    self.assertTrue(errs)
                    self.assertTrue(all(e.startswith("ERROR ") for e in errs), errs)
                    self.assertTrue(any(expect in e for e in errs), errs)

    def test_selftest_flag_reports_all_pass(self):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = m.main(["--selftest"])
        total = len(m.selftest_cases())
        self.assertEqual(rc, 0)
        self.assertEqual(buf.getvalue().strip(), f"selftest PASS {total}/{total}")

    def test_error_line_points_at_second_address_line(self):
        cfg, state = m.synthetic_config()
        appended = cfg + ('[[patches.hooks]]\naddress = 5612640\nfunction = "NW_OnLoad"\n'
                          'original_bytes = [85, 139, 236, 106, 255]\ntype = "detour"\n')
        lines = [n for n, line in enumerate(appended.splitlines(), 1) if line.startswith("address = 5612640")]
        errs = m.check(appended, state, m._SYNTH_HOOKS, m._SYNTH_EXPORTS)
        self.assertIn(f"ERROR patch_config.toml:{lines[-1]}: ", " ".join(errs))
        self.assertEqual(len(errs), 1, errs)

    def test_synthetic_fixture_matches_real_hooks_source(self):
        if not REAL_HOOKS.exists():
            self.skipTest("npc-world hooks source not in this checkout")
        cfg, state = m.synthetic_config()
        real_exports = REAL_HOOKS.with_name("exports.def").read_text(encoding="utf-8")
        errs = m.check(cfg, state, REAL_HOOKS.read_text(encoding="utf-8"), real_exports)
        self.assertEqual(errs, [])


class CliExitCodes(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.hooks = self.root / "npc_world_src" / "kotor2-steam-aspyr.hooks.toml"
        self.hooks.parent.mkdir()
        self.hooks.write_text(m._SYNTH_HOOKS, encoding="utf-8")
        self.hooks.with_name("exports.def").write_text(m._SYNTH_EXPORTS, encoding="utf-8")
        self.cfg = self.root / "patch_config.toml"
        self.state = self.root / "kpm_install_state.json"

    def tearDown(self):
        self._tmp.cleanup()

    def _run(self, cfg_text, state_obj, *extra):
        self.cfg.write_text(cfg_text, encoding="utf-8")
        self.state.write_text(json.dumps(state_obj), encoding="utf-8")
        argv = [sys.executable, str(SCRIPT), "--config", str(self.cfg), "--state", str(self.state),
                "--npcworld-hooks", str(self.hooks), *extra]
        return subprocess.run(argv, capture_output=True, text=True, timeout=60)

    def test_valid_pair_exits_zero_with_ok(self):
        cfg, state = m.synthetic_config()
        proc = self._run(cfg, state)
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        self.assertEqual(proc.stdout.strip(), "OK")

    def test_invalid_pair_exits_one_with_error_lines(self):
        cfg, state = m.synthetic_config()
        bad = cfg.replace('id = "k1-spawn-probe"', 'id = "k1-spawn-probe-x"', 1)
        proc = self._run(bad, state)
        self.assertEqual(proc.returncode, 1, proc.stdout + proc.stderr)
        self.assertRegex(proc.stdout, r"(?m)^ERROR .+:\d+: ")
        self.assertNotIn("OK", proc.stdout.split())

    def test_default_exports_is_sibling_of_hooks(self):
        cfg, state = m.synthetic_config()
        self.hooks.with_name("exports.def").write_text(m._SYNTH_EXPORTS.replace("    NW_OnTick\n", ""),
                                                       encoding="utf-8")
        proc = self._run(cfg, state)
        self.assertEqual(proc.returncode, 1)
        self.assertIn("NW_OnTick is not exported", proc.stdout)

    def test_explicit_exports_flag_overrides_default(self):
        cfg, state = m.synthetic_config()
        alt = self.root / "alt.def"
        alt.write_text(m._SYNTH_EXPORTS, encoding="utf-8")
        proc = self._run(cfg, state, "--exports", str(alt))
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

    def test_unreadable_config_exits_one(self):
        _, state = m.synthetic_config()
        self.state.write_text(json.dumps(state), encoding="utf-8")
        argv = [sys.executable, str(SCRIPT), "--config", str(self.root / "missing.toml"),
                "--state", str(self.state), "--npcworld-hooks", str(self.hooks)]
        proc = subprocess.run(argv, capture_output=True, text=True, timeout=60)
        self.assertEqual(proc.returncode, 1)
        self.assertIn("cannot read", proc.stdout)

    def test_missing_required_flag_is_usage_error(self):
        proc = subprocess.run([sys.executable, str(SCRIPT), "--config", str(self.cfg)],
                              capture_output=True, text=True, timeout=60)
        self.assertEqual(proc.returncode, 2)

    def test_selftest_flag_exit_zero(self):
        proc = subprocess.run([sys.executable, str(SCRIPT), "--selftest"],
                              capture_output=True, text=True, timeout=60)
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        self.assertRegex(proc.stdout, r"selftest PASS (\d+)/\1")


if __name__ == "__main__":
    unittest.main()
