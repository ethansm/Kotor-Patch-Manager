#!/usr/bin/env python3
"""Tests for scripts/check_subclassers.py (doc 20 Phase 1). Pure stdlib; fixtures are tiny repo roots under
scripts/testdata/static/subclass/<case>/ (copied to a temp dir whenever a test mutates them)."""
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import _static_common as sc  # noqa: E402
import check_subclassers as css  # noqa: E402

FX = HERE / "testdata" / "static" / "subclass"
A = "patch_manager_mods/a_src/A.cpp"
B = "patch_manager_mods/a_src/B.cpp"


def sevs(rep, sev):
    return [f for f in rep.findings if f.sev == sev]


def check(case_or_root):
    root = case_or_root if isinstance(case_or_root, Path) else FX / case_or_root
    return css.run(root, reporter=sc.Reporter(out=None))


class SubclasserTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)

    def copy(self, case):
        dst = Path(self._tmp.name) / case
        shutil.copytree(FX / case, dst)
        return dst

    def test_all_good_passes_and_excluded_files_are_ignored(self):
        # good/ also holds a subclass call in A_test.cpp and testdata/ (unregistered, must be ignored)
        rep = check("good")
        self.assertEqual(sevs(rep, "FAIL"), [])
        self.assertEqual(sevs(rep, "WARN"), [])
        self.assertEqual(len(sevs(rep, "PASS")), 1)
        self.assertEqual(rep.exit_code(), 0)

    def test_hint_drift_is_info_only(self):
        rep = check("good")                            # hints 3/5 vs real lines 2/3
        infos = [f.msg for f in sevs(rep, "INFO")]
        self.assertEqual(len(infos), 1)
        self.assertIn("hints drifted", infos[0])

    def test_unregistered_subclasser_fails(self):
        rep = check("unregistered")
        fails = sevs(rep, "FAIL")
        self.assertEqual([f.msg.split(" ")[0] for f in fails], [f"{B}:2", f"{B}:3"])   # W/A and Ptr variants both match
        for f in fails:
            self.assertIn("new WndProc subclasser: use Common WindowHook.h (kp::WindowHook); "
                          "or register it here", f.msg)
        self.assertEqual(rep.exit_code(), 1)

    def test_missing_restore_fails(self):
        rep = check("missing_restore")
        fails = sevs(rep, "FAIL")
        self.assertEqual(len(fails), 1)
        self.assertIn("restore pattern not found", fails[0].msg)
        self.assertIn(A, fails[0].msg)

    def test_restore_without_ours_check_fails(self):
        root = self.copy("good")
        text = (root / A).read_text().replace("== wheelProc", "== nullptr")   # restores unconditionally
        (root / A).write_text(text)
        rep = check(root)
        self.assertEqual(len(sevs(rep, "FAIL")), 1)
        self.assertIn("restore", sevs(rep, "FAIL")[0].msg)

    def test_missing_install_fails(self):
        root = self.copy("good")
        text = (root / A).read_text().replace("g_old = (WNDPROC)SetWindowLongPtrA", "(void)SetWindowLongPtrA")
        (root / A).write_text(text)
        rep = check(root)
        self.assertEqual([f.msg for f in sevs(rep, "FAIL") if "install" in f.msg].__len__(), 1)

    def test_stale_registration_warns(self):
        rep = check("stale")                           # A.cpp only touches GWL_STYLE; Gone.cpp does not exist
        self.assertEqual(sevs(rep, "FAIL"), [])
        msgs = [f.msg for f in sevs(rep, "WARN")]
        self.assertEqual(len(msgs), 2, msgs)
        self.assertTrue(any(A in m and "no SetWindowLong" in m for m in msgs))
        self.assertTrue(any("Gone.cpp" in m for m in msgs))
        self.assertEqual(rep.exit_code(), 0)

    def test_registry_override_and_missing_registry(self):
        rep = css.run(FX / "good", registry=FX / "nope.toml", reporter=sc.Reporter(out=None))
        self.assertEqual(len(sevs(rep, "FAIL")), 2)    # nothing registered: both calls in A.cpp are new
        rep = css.run(FX / "unregistered", registry=FX / "good/tests/static/subclassers.toml", reporter=sc.Reporter(out=None))
        self.assertEqual(len(sevs(rep, "FAIL")), 2)

    def test_main_exit_codes(self):
        def run(*args):
            return subprocess.run([sys.executable, str(HERE / "check_subclassers.py"), *args], capture_output=True, text=True)
        ok, bad = run("--root", str(FX / "good"), "--quiet"), run("--root", str(FX / "unregistered"))
        self.assertEqual((ok.returncode, bad.returncode), (0, 1))
        self.assertNotIn("PASS subclasser", ok.stdout)
        self.assertIn("FAIL subclasser", bad.stdout)

    def _wh_root(self, src):
        root = Path(self._tmp.name) / "wh"
        (root / "patch_manager_mods/a_src").mkdir(parents=True, exist_ok=True)
        (root / "tests/static").mkdir(parents=True, exist_ok=True)
        (root / A).write_text(src)
        (root / "tests/static/subclassers.toml").write_text(
            '[[subclasser]]\nfile = "' + A + '"\nuses = "WindowHook"\n'
            "install = 'Win::install\\s*\\('\nrestore = 'Win::uninstall\\s*\\('\n")
        return root

    def test_uses_windowhook_passes_without_direct_call(self):
        rep = check(self._wh_root("struct T; using Win = kp::WindowHook<T>;\nvoid f(){ Win::install(h, p); }\nvoid g(){ Win::uninstall(p); }\n"))
        self.assertEqual(sevs(rep, "FAIL"), [])
        self.assertEqual(sevs(rep, "WARN"), [])                 # no stale-entry warning without a GWLP_WNDPROC call
        self.assertIn("WindowHook install and restore present", [f.msg for f in sevs(rep, "INFO")][0])

    def test_uses_windowhook_requires_windowhook_and_calls(self):
        rep = check(self._wh_root("void f(){ Win::install(h, p); }\nvoid g(){ Win::uninstall(p); }\n"))   # no WindowHook< alias
        self.assertEqual(len(sevs(rep, "FAIL")), 1)
        self.assertIn("WindowHook", sevs(rep, "FAIL")[0].msg)
        rep = check(self._wh_root("struct T; using Win = kp::WindowHook<T>;\nvoid f(){ Win::install(h, p); }\n"))   # no uninstall
        fails = sevs(rep, "FAIL")
        self.assertEqual(len(fails), 1)
        self.assertIn("WindowHook restore pattern not found", fails[0].msg)

    def test_regex_covers_variants(self):
        for s in ("SetWindowLongPtrA(h, GWLP_WNDPROC, 0)", "SetWindowLongW (h, GWL_WNDPROC, 0)", "SetWindowLong(a.b,\n  GWLP_WNDPROC,x)",
                  "SetWindowLongPtrW(g_hwnd,GWLP_WNDPROC"):
            self.assertTrue(css.SUBCLASS_RE.search(s), s)
        for s in ("SetWindowLongPtrA(h, GWL_STYLE, 0)", "GetWindowLongPtrA(h, GWLP_WNDPROC)"):
            self.assertFalse(css.SUBCLASS_RE.search(s), s)


if __name__ == "__main__":
    unittest.main()
