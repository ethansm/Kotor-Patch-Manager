#!/usr/bin/env python3
"""Tests for scripts/check_provider_manifests.py. Pure stdlib; every case builds a tiny patch_manager_mods tree in a temp dir."""
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import _static_common as sc  # noqa: E402
import check_provider_manifests as clm  # noqa: E402

CORE = '#define KPATCH_LOG_IFACE   "kpatch.log"\n#define KPATCH_LOG_VERSION 1\n' \
       'typedef struct KPatchApi {\n  int (*provide)(const char* name, unsigned version, const void* iface);\n' \
       '  const void* (*require)(const char* name, unsigned version);\n} KPatchApi;\n'
SHARED = '''#pragma once
#include "kpatch_api.h"
#define K2PS_IFACE   "k2.portraitslots"   // comment with provide("fake", 9) inside
#define K2PS_VERSION 1
struct Client {
    const void* connect(const KPatchApi* api) { return api->require(K2PS_IFACE, K2PS_VERSION); }
    const void* provide(const char* name, unsigned version);
};
'''
LIB_CPP = '#include "kpatch_api.h"\n#include "PortraitSlots.h"\nvoid init(KPatchApi* api) {\n' \
          '    api->provide(K2PS_IFACE, K2PS_VERSION, &g);\n    api->require(KPATCH_LOG_IFACE, KPATCH_LOG_VERSION);\n}\n'
CON_CPP = '#include "PortraitSlots.h"\nvoid f(KPatchApi* api) { Client c; c.connect(api);\n' \
          '    api->require(KPATCH_LOG_IFACE, KPATCH_LOG_VERSION); }\n'


def manifest(id_, kind=None, provides=None, requires=None, ifaces=None):
    s = f'[patch]\nid = "{id_}"\nversion = "1.0.0"\n'
    if kind:
        s += f'kind = "{kind}"\n'
    for key, v in (("provides", provides), ("requires", requires), ("requires_interfaces", ifaces)):
        if v is not None:
            s += f"{key} = [{', '.join(repr(x).replace(chr(39), chr(34)) for x in v)}]\n"
    return s


def hooks(*addrs):
    return "".join(f'[[hooks]]\naddress = 0x{a:08X}\ntype = "detour"\noriginal_bytes = [0x55]\n\n' for a in addrs)


def sevs(rep, sev):
    return [f for f in rep.findings if f.sev == sev]


class ProviderManifestTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.root = Path(self._tmp.name) / "patch_manager_mods"
        self.core = Path(self._tmp.name) / "kpatch_api.h"
        self.core.write_text(CORE)
        self.write("_shared_src/PortraitSlots.h", SHARED)
        self.write("lib_src/manifest.toml", manifest("lib", "provider", provides=["k2.portraitslots@1"]))
        self.write("lib_src/Lib.cpp", LIB_CPP)
        self.write("lib_src/kotor2-steam-aspyr.hooks.toml", hooks(0x100, 0x200))
        self.write("con_src/manifest.toml", manifest("con", requires=["lib"], ifaces=["k2.portraitslots@1"]))
        self.write("con_src/Con.cpp", CON_CPP)
        self.write("con_src/kotor2-steam-aspyr.hooks.toml", hooks(0x300))

    def write(self, rel, text):
        p = self.root / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text)

    def check(self, core=None):
        return clm.run(self.root, core_header=core or self.core, reporter=sc.Reporter(out=None))

    def fails(self, rep):
        return [f.msg for f in sevs(rep, "FAIL")]

    def test_good_tree_passes(self):
        rep = self.check()
        self.assertEqual(sevs(rep, "FAIL"), [])
        self.assertEqual(sevs(rep, "WARN"), [])
        self.assertEqual([f.check for f in sevs(rep, "PASS")], ["provider", "consumer"])
        self.assertEqual(rep.exit_code(), 0)

    def test_wrong_interface_name_in_consumer_manifest_fails(self):
        self.write("con_src/manifest.toml", manifest("con", requires=["lib"], ifaces=["k2.portraitslot@1"]))
        f = self.fails(self.check())
        self.assertTrue(any("k2.portraitslot@1" in m and "no require(" in m for m in f), f)
        self.assertTrue(any("no manifest provides it" in m for m in f), f)
        self.assertEqual(self.check().exit_code(), 1)

    def test_provider_provides_without_provide_call_fails(self):
        self.write("lib_src/Lib.cpp", "void init() {}\n")
        f = self.fails(self.check())
        self.assertTrue(any("no provide(" in m for m in f), f)

    def test_undeclared_provide_fails(self):
        self.write("lib_src/Lib.cpp", LIB_CPP + 'void g(KPatchApi* api) { api->provide("k2.other", 2, &g); }\n')
        f = self.fails(self.check())
        self.assertEqual(len(f), 1, f)
        self.assertIn("k2.other", f[0])
        self.assertIn("not listed in provides", f[0])

    def test_consumer_missing_requires_id_fails(self):
        self.write("con_src/manifest.toml", manifest("con", requires=[], ifaces=["k2.portraitslots@1"]))
        f = self.fails(self.check())
        self.assertEqual(len(f), 1, f)
        self.assertIn("provided by lib", f[0])

    def test_duplicate_hook_address_fails(self):
        self.write("con_src/kotor2-steam-aspyr.hooks.toml", hooks(0x300, 0x200))
        f = self.fails(self.check())
        self.assertEqual(len(f), 1, f)
        self.assertIn("0x00000200", f[0])
        # same address in a differently named hooks file is not a conflict
        (self.root / "con_src/kotor2-steam-aspyr.hooks.toml").unlink()
        self.write("con_src/kotor2-gog-aspyr.hooks.toml", hooks(0x200))
        self.assertEqual(self.fails(self.check()), [])

    def test_core_log_require_is_ignored(self):
        rep = self.check()                       # both mods require(KPATCH_LOG_IFACE, ...): not listed, no warning, no failure
        self.assertEqual(sevs(rep, "WARN"), [])
        self.assertEqual(sevs(rep, "FAIL"), [])

    def test_malformed_entry_fails(self):
        self.write("con_src/manifest.toml", manifest("con", requires=["lib"], ifaces=["k2.portraitslots@x"]))
        f = self.fails(self.check())
        self.assertTrue(any("malformed requires_interfaces" in m for m in f), f)
        self.write("lib_src/manifest.toml", manifest("lib", "provider", provides=["k2.portraitslots@x"]))
        self.assertTrue(any("malformed provides" in m for m in self.fails(self.check())))

    def test_library_kind_is_rejected_as_provider(self):
        self.write("lib_src/manifest.toml", manifest("lib", "library", provides=["k2.portraitslots@1"]))
        self.assertTrue(self.fails(self.check()))

    def test_empty_provider_provides_fails(self):
        self.write("lib_src/manifest.toml", manifest("lib", "provider", provides=[]))
        self.write("lib_src/Lib.cpp", "void init() {}\n")
        self.assertTrue(any("empty provides" in m for m in self.fails(self.check())))

    def test_unlisted_noncore_require_warns(self):
        self.write("con_src/manifest.toml", manifest("con", requires=["lib"]))
        rep = self.check()
        self.assertEqual(sevs(rep, "FAIL"), [])
        self.assertEqual(len(sevs(rep, "WARN")), 1)
        self.assertIn("not listed in requires_interfaces", sevs(rep, "WARN")[0].msg)

    def test_declarations_commented_calls_and_tests_are_ignored(self):
        # SHARED holds a member declaration `provide(const char* name, ...)` and a commented provide("fake", 9); neither counts
        self.write("lib_src/Lib_test.cpp", 'void t(KPatchApi* api) { api->provide("k2.testonly", 1, 0); }\n')
        self.write("lib_src/testdata/x.cpp", 'void t(KPatchApi* api) { api->provide("k2.fixture", 1, 0); }\n')
        self.write("lib_src/Lib2.cpp", '/* api->provide("k2.block", 1, 0); */\n// api->provide("k2.line", 1, 0);\n')
        rep = self.check()
        self.assertEqual(sevs(rep, "FAIL"), [])
        self.assertEqual(sevs(rep, "WARN"), [])

    def test_missing_core_header_warns_only(self):
        rep = self.check(core=Path(self._tmp.name) / "nope.h")
        self.assertEqual(sevs(rep, "FAIL"), [])
        warns = [f.msg for f in sevs(rep, "WARN")]
        self.assertTrue(any("core header" in m and "not found" in m for m in warns), warns)
        self.assertTrue(any("KPATCH_LOG_IFACE" in m for m in warns), warns)
        self.assertEqual(rep.exit_code(), 0)

    def test_shared_header_only_counts_when_included(self):
        self.write("con_src/Con.cpp", "void f() {}\n")           # no include of PortraitSlots.h -> no require() in the sources
        self.assertTrue(any("no require(" in m for m in self.fails(self.check())))

    def test_main_exit_codes(self):
        def run(*args):
            return subprocess.run([sys.executable, str(HERE / "check_provider_manifests.py"), "--root", str(self.root),
                                   "--core-header", str(self.core), *args], capture_output=True, text=True)
        ok = run("--quiet")
        self.assertEqual(ok.returncode, 0, ok.stdout)
        self.assertNotIn("PASS provider", ok.stdout)
        self.write("con_src/manifest.toml", manifest("con", requires=[], ifaces=["k2.portraitslots@1"]))
        bad = run()
        self.assertEqual(bad.returncode, 1)
        self.assertIn("FAIL requires", bad.stdout)

    def test_real_tree_is_clean(self):
        rep = clm.run(sc.REPO / "patch_manager_mods", reporter=sc.Reporter(out=None))
        self.assertEqual(sevs(rep, "FAIL"), [])


if __name__ == "__main__":
    unittest.main()
