#!/usr/bin/env python3
"""Tests for scripts/check_hook_spans.py (doc 20 Phase 1). Pure stdlib; every input is built in memory or written to a
temp dir - no exe, KPM tree, game dir or objdump needed."""
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import _static_common as sc  # noqa: E402
import check_hook_spans as chs  # noqa: E402

SHA = "AA" * 32
OTHER = "BB" * 32


def hook(addr, n, mod="", fn="F"):
    return sc.Hook(address=addr, type="detour", orig=bytes([0x90]) * n, function=fn, mod=mod)


def cp(mod, addr, n=7, name="tx", verifies=True):
    return sc.CodePatch(name=name, address=addr, orig=bytes([0x90]) * n, verifies_original=verifies, mod=mod)


def mod(mid, hooks=(), cps=(), tv=(SHA,), conflicts=()):
    return sc.Mod(id=mid, version="1.0.0", dir=Path("."), kpm_dir=None, hooks=list(hooks), target_versions=list(tv),
                  exports=[], code_patches=list(cps), conflicts=tuple(conflicts))


def exc(addr=0x4261CF, patches=("a", "b"), tests=("wine:x[y]",)):
    return {"address": addr, "patches": list(patches), "reason": "r", "tests": list(tests)}


def adj(addr=0x1005, patches=("a", "b")):
    return {"address": addr, "patches": list(patches), "reason": "r"}


def sevs(findings, check=None):
    return [f.sev for f in findings if f.sev in ("FAIL", "WARN") and (check is None or f.check == check)]


class SpanTests(unittest.TestCase):
    def test_overlap_between_patches_fails(self):
        f = chs.run([mod("a", [hook(0x1000, 6)]), mod("b", [hook(0x1005, 6)])])   # share byte 0x1005
        self.assertEqual(sevs(f, "overlap"), ["FAIL"])

    def test_declared_conflict_overlap_is_info_either_direction(self):
        for a_c, b_c in ((("b",), ()), ((), ("a",))):
            f = chs.run([mod("a", [hook(0x1000, 6)], conflicts=a_c), mod("b", [hook(0x1000, 6)], conflicts=b_c)])
            self.assertEqual(sevs(f), [])
            self.assertTrue(any(x.sev == "INFO" and x.check == "declared-conflict" for x in f))

    def test_declared_conflict_with_a_third_patch_does_not_excuse(self):
        f = chs.run([mod("a", [hook(0x1000, 6)], conflicts=("c",)), mod("b", [hook(0x1000, 6)])])
        self.assertEqual(sevs(f, "overlap"), ["FAIL"])

    def test_declared_conflict_not_applied_to_installed_scope(self):
        mods = [mod("a", [hook(0x1000, 6)], conflicts=("b",)), mod("b", [hook(0x1000, 6)])]
        f = chs.run(mods, live_config={"a": [hook(0x1000, 6, "a")], "b": [hook(0x1000, 6, "b")]}, live_sha=SHA)
        self.assertEqual(sevs(f, "overlap"), ["FAIL"])

    def test_touching_spans_do_not_overlap(self):
        f = chs.run([mod("a", [hook(0x1000, 5)]), mod("b", [hook(0x1005, 5)])])   # a ends where b starts
        self.assertEqual(sevs(f, "overlap"), [])
        self.assertEqual(sevs(f, "gap"), ["WARN"])   # gap 0

    def test_overlap_within_one_patch_fails(self):
        f = chs.run([mod("a", [hook(0x1000, 6), hook(0x1002, 6)])])
        self.assertEqual(sevs(f, "overlap-self"), ["FAIL"])

    def test_gap_of_six_bytes_warns(self):
        f = chs.run([mod("a", [hook(0x1000, 5)]), mod("b", [hook(0x100B, 5)])])   # 0x1005..0x100B = 6 bytes
        self.assertEqual(sevs(f), ["WARN"])
        self.assertIn("6 byte gap", next(x.msg for x in f if x.check == "gap"))

    def test_gap_of_nine_bytes_is_quiet_and_same_patch_gap_is_ignored(self):
        f = chs.run([mod("a", [hook(0x1000, 5), hook(0x1007, 5)]), mod("b", [hook(0x1007 + 5 + 9, 5)])])
        self.assertEqual(sevs(f, "gap"), [])

    def test_exception_honoured(self):
        mods = [mod("a", cps=[cp("a", 0x4261CF)]), mod("b", cps=[cp("b", 0x4261CF)])]
        f = chs.run(mods, exceptions=[exc()])
        self.assertEqual(sevs(f), [])
        self.assertTrue(any(x.sev == "INFO" and x.check == "exception" for x in f))

    def test_unused_exception_fails(self):
        f = chs.run([mod("a", [hook(0x1000, 5)]), mod("b", [hook(0x2000, 5)])], exceptions=[exc()])
        self.assertEqual(sevs(f, "exception-stale"), ["FAIL"])

    def test_exception_without_verification_fails(self):
        mods = [mod("a", cps=[cp("a", 0x4261CF)]), mod("b", cps=[cp("b", 0x4261CF, verifies=False)])]
        f = chs.run(mods, exceptions=[exc()])
        self.assertEqual(sevs(f, "exception"), ["FAIL"])
        self.assertEqual(sevs(f, "exception-stale"), [])   # it matched, it is just not valid
        self.assertIn("verifies_original = false", next(x.msg for x in f if x.check == "exception"))

    def test_exception_does_not_cover_a_hook(self):
        mods = [mod("a", cps=[cp("a", 0x4261CF)]), mod("b", [hook(0x4261CF, 7)])]
        f = chs.run(mods, exceptions=[exc()])
        self.assertEqual(sevs(f, "exception"), ["FAIL"])

    def test_exception_does_not_cover_a_third_patch(self):
        mods = [mod("a", cps=[cp("a", 0x4261CF)]), mod("b", cps=[cp("b", 0x4261CF)]), mod("c", cps=[cp("c", 0x4261D0)])]
        f = chs.run(mods, exceptions=[exc()])
        self.assertEqual(sevs(f, "overlap"), ["FAIL", "FAIL"])   # a-c and b-c

    def test_exception_without_tests_fails(self):
        mods = [mod("a", cps=[cp("a", 0x4261CF)]), mod("b", cps=[cp("b", 0x4261CF)])]
        self.assertEqual(sevs(chs.run(mods, exceptions=[exc(tests=())]), "exception"), ["FAIL"])

    def test_exception_test_id_must_exist_in_smokes(self):
        mods = [mod("a", cps=[cp("a", 0x4261CF)]), mod("b", cps=[cp("b", 0x4261CF)])]
        with tempfile.TemporaryDirectory() as d:
            sm = Path(d) / "smokes.toml"
            sm.write_text('[[smoke]]\nname = "x"\nmodes = ["y"]\n[[smoke]]\nname = "z"\nmodes = []\n')
            self.assertEqual(sevs(chs.run(mods, exceptions=[exc()], smokes=sm)), [])
            self.assertEqual(sevs(chs.run(mods, exceptions=[exc(tests=("wine:x[nope]",))], smokes=sm), "exception-test"), ["FAIL"])
            self.assertEqual(sevs(chs.run(mods, exceptions=[exc(tests=("wine:gone[y]",))], smokes=sm), "exception-test"), ["FAIL"])
            self.assertEqual(sevs(chs.run(mods, exceptions=[exc(tests=("wine:z",))], smokes=sm), "exception-test"), [])

    def test_exception_used_only_in_source_scope_is_not_stale(self):
        # b is not installed, so the installed scope has no overlap; the source scope uses the exception
        mods = [mod("a", cps=[cp("a", 0x4261CF)]), mod("b", cps=[cp("b", 0x4261CF)])]
        f = chs.run(mods, live_config={"a": []}, exceptions=[exc()])
        self.assertEqual(sevs(f), [])

    def test_exceptions_from_toml(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "e.toml"
            p.write_text('[[exception]]\naddress = 0x004261CF\npatches = ["a", "b"]\nreason = "r"\ntests = ["wine:x[y]"]\n')
            mods = [mod("a", cps=[cp("a", 0x4261CF)]), mod("b", cps=[cp("b", 0x4261CF)])]
            self.assertEqual(sevs(chs.run(mods, exceptions=p)), [])

    def test_target_versions_filter_excludes_hook(self):
        mods = [mod("a", [hook(0x1000, 6)]), mod("b", [hook(0x1002, 6)], tv=(OTHER,))]
        self.assertEqual(sevs(chs.run(mods, live_sha=SHA), "overlap"), [])          # b targets another exe
        self.assertEqual(sevs(chs.run(mods, live_sha=OTHER), "overlap"), [])        # a targets another exe
        self.assertEqual(sevs(chs.run(mods, live_sha=None), "overlap"), ["FAIL"])   # no filter: both count
        mods2 = [mod("a", [hook(0x1000, 6)]), mod("b", [hook(0x1002, 6)], tv=())]
        self.assertEqual(sevs(chs.run(mods2, live_sha=SHA), "overlap"), ["FAIL"])   # absent target_versions = all

    def test_live_config_overlap_with_third_party_fails(self):
        live = {"a": [hook(0x1000, 6, "a")], "third-party": [hook(0x1003, 6, "third-party")]}
        f = chs.run([mod("a", [hook(0x1000, 6)])], live_config=live)
        self.assertEqual(sevs(f, "overlap"), ["FAIL"])
        self.assertIn("[installed]", next(x.msg for x in f if x.check == "overlap"))

    def test_source_vs_live_drift_warns(self):
        live = {"a": [hook(0x1000, 6, "a")]}
        f = chs.run([mod("a", [hook(0x1000, 6), hook(0x2000, 6)])], live_config=live)
        self.assertEqual(sevs(f, "drift"), ["WARN"])
        self.assertEqual(sevs(chs.run([mod("a", [hook(0x1000, 6)])], live_config=live), "drift"), [])

    def test_code_patches_of_installed_ids_join_the_installed_scope(self):
        live = {"a": [], "t": [hook(0x4261D0, 4, "t")]}
        f = chs.run([mod("a", cps=[cp("a", 0x4261CF)])], live_config=live)
        self.assertTrue(any(x.check == "overlap" and "[installed]" in x.msg for x in f))   # source scope has no t

    def test_live_config_from_file(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "patch_config.toml"
            p.write_text(f'target_version_sha = "{SHA}"\n[[patches]]\nid = "a"\ndll = "a.dll"\n'
                         '[[patches.hooks]]\naddress = 4096\ntype = "detour"\nfunction = "F"\noriginal_bytes = [144, 144, 144, 144, 144]\n'
                         '[[patches]]\nid = "b"\ndll = "b.dll"\n[[patches.hooks]]\naddress = 4098\ntype = "detour"\nfunction = "G"\n'
                         'original_bytes = [144, 144, 144, 144, 144]\n')
            f = chs.run([mod("a", [hook(0x1000, 5)])], live_config=p)
            self.assertEqual(sevs(f, "overlap"), ["FAIL"])

    def test_missing_live_config_skips_installed_scope(self):
        f = chs.run([mod("a", [hook(0x1000, 5)])], live_config="/nonexistent/patch_config.toml")
        self.assertEqual(sevs(f), [])
        self.assertTrue(any(x.sev == "SKIP" for x in f))

    def test_mod_filter(self):
        mods = [mod("a", [hook(0x1000, 6)]), mod("b", [hook(0x1002, 6)]), mod("c", [hook(0x3000, 6)]), mod("d", [hook(0x3002, 6)])]
        f = chs.run(mods, mod_ids=["c"])
        self.assertEqual(sevs(f, "overlap"), ["FAIL"])
        self.assertIn("c ", next(x.msg for x in f if x.check == "overlap"))

    # [[adjacency]]: a documented near-miss between two non-overlapping spans (WARN -> INFO)
    def test_registered_adjacency_is_info(self):
        for gap_b in (0x1005, 0x100B):   # 0-byte and 6-byte gap; the address is the lower span's end either way
            f = chs.run([mod("a", [hook(0x1000, 5)]), mod("b", [hook(gap_b, 5)])], adjacencies=[adj()])
            self.assertEqual(sevs(f), [])
            self.assertTrue(any(x.sev == "INFO" and x.check == "adjacency" and "documented adjacency" in x.msg for x in f))

    def test_registered_adjacency_matches_either_span_order(self):
        f = chs.run([mod("b", [hook(0x1000, 5)]), mod("a", [hook(0x1005, 5)])], adjacencies=[adj(patches=("a", "b"))])
        self.assertEqual(sevs(f), [])

    def test_unregistered_adjacency_still_warns(self):
        f = chs.run([mod("a", [hook(0x1000, 5)]), mod("b", [hook(0x1005, 5)])], adjacencies=[])
        self.assertEqual(sevs(f, "gap"), ["WARN"])

    def test_adjacency_with_wrong_patch_ids_does_not_match(self):
        f = chs.run([mod("a", [hook(0x1000, 5)]), mod("b", [hook(0x1005, 5)])], adjacencies=[adj(patches=("a", "c"))])
        self.assertEqual(sevs(f, "gap"), ["WARN"])
        self.assertEqual(sevs(f, "adjacency-stale"), ["FAIL"])

    def test_adjacency_with_wrong_address_does_not_match(self):
        f = chs.run([mod("a", [hook(0x1000, 5)]), mod("b", [hook(0x1005, 5)])], adjacencies=[adj(addr=0x1006)])
        self.assertEqual(sevs(f, "gap"), ["WARN"])
        self.assertEqual(sevs(f, "adjacency-stale"), ["FAIL"])

    def test_stale_adjacency_fails(self):
        f = chs.run([mod("a", [hook(0x1000, 5)]), mod("b", [hook(0x2000, 5)])], adjacencies=[adj()])
        self.assertEqual(sevs(f, "adjacency-stale"), ["FAIL"])

    def test_adjacency_used_only_in_source_scope_is_not_stale(self):
        f = chs.run([mod("a", [hook(0x1000, 5)]), mod("b", [hook(0x1005, 5)])], live_config={"a": [hook(0x1000, 5, "a")]}, adjacencies=[adj()])
        self.assertEqual(sevs(f), [])

    def test_adjacency_does_not_excuse_an_overlap(self):
        f = chs.run([mod("a", [hook(0x1000, 6)]), mod("b", [hook(0x1005, 5)])], adjacencies=[adj()])
        self.assertEqual(sevs(f, "overlap"), ["FAIL"])

    def test_adjacencies_from_toml(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "e.toml"
            p.write_text('[[adjacency]]\naddress = 0x1005\npatches = ["a", "b"]\nreason = "r"\n')
            f = chs.run([mod("a", [hook(0x1000, 5)]), mod("b", [hook(0x1005, 5)])], exceptions=p)
            self.assertEqual(sevs(f), [])
            self.assertTrue(any(x.check == "adjacency" for x in f))
            p.write_text('[[adjacency]]\npatches = ["a", "b"]\n')
            with self.assertRaises(ValueError):
                chs.run([], exceptions=p)


if __name__ == "__main__":
    unittest.main()
