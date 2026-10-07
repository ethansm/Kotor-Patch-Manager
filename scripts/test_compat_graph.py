#!/usr/bin/env python3
"""
scripts/test_compat_graph.py - Test suite for compat_graph.py (Doc 22 U1).

Tests:
  - Symlink node identity (realpath resolution and canonical node deduplication)
  - ../_shared/ mapping to _shared_src/
  - Stub-only angle include (<windows.h> host-only gating, no DLL builds)
  - Cross-mod include (shield_logic_test -> BuffHudLogic.h)
  - Deleted header handling (unresolved include matches deleted file)
  - Rename handling
  - .inc include chain
  - Manifest library fan-out (library -> requires/requires_interfaces consumers)
  - Nested mod key (perf_lab_src/perflab via nearest manifest.toml)
  - Golden check on live repository (SafeRead.h, Gui2D.h, PortraitSlotsProvider.cpp)
"""

import os
import shutil
import tempfile
import unittest
from pathlib import Path

from compat_graph import CompatGraph, find_repo_root


class TestCompatGraphFixtures(unittest.TestCase):
    def setUp(self):
        self.tmp_dir = tempfile.mkdtemp(prefix="compat_graph_test_")
        self.repo = Path(self.tmp_dir).resolve()

        # Build basic fixture structure
        (self.repo / "patch_manager_mods/_shared_src").mkdir(parents=True)
        (self.repo / "scripts").mkdir(parents=True)
        (self.repo / "tests").mkdir(parents=True)
        (self.repo / "research/companion_levelup/stub").mkdir(parents=True)

    def tearDown(self):
        shutil.rmtree(self.tmp_dir, ignore_errors=True)

    def test_symlink_node_identity(self):
        """Symlinked files resolve to the canonical target path and deduplicate."""
        shared_h = self.repo / "patch_manager_mods/_shared_src/Common.h"
        shared_h.write_text("// Common header\nint common();\n")

        symlink_dir = self.repo / "research/companion_levelup/stub/_shared"
        symlink_dir.mkdir(parents=True)
        symlink_h = symlink_dir / "Common.h"
        os.symlink(shared_h, symlink_h)

        test_cpp = self.repo / "tests/dummy_test.cpp"
        test_cpp.write_text('#include "Common.h"\nint main(){ return common(); }\n')

        cg = CompatGraph(repo_root=self.repo)
        plan = cg.affected(["patch_manager_mods/_shared_src/Common.h"])
        self.assertIn("cpp:dummy_test", plan.tests())

    def test_dot_dot_shared_mapping(self):
        """Includes with ../_shared/ or ../_shared_src/ map to _shared_src/."""
        header = self.repo / "patch_manager_mods/_shared_src/Gui2D.h"
        header.write_text("// Gui2D\nstruct Gui2D {};\n")

        mod_dir = self.repo / "patch_manager_mods/sample_mod_src"
        mod_dir.mkdir(parents=True)
        (mod_dir / "manifest.toml").write_text('[patch]\nid = "sample-mod"\n')
        mod_cpp = mod_dir / "Sample.cpp"
        mod_cpp.write_text('#include "../_shared/Gui2D.h"\nGui2D g;\n')

        cg = CompatGraph(repo_root=self.repo)
        plan = cg.affected(["patch_manager_mods/_shared_src/Gui2D.h"])
        self.assertIn("sample_mod_src", plan.builds())

    def test_stub_only_angle_include(self):
        """Angle includes in stub path are marked host-only; DLL builds are not scheduled."""
        stub_win = self.repo / "research/companion_levelup/stub/windows.h"
        stub_win.write_text("// stub windows.h\n")

        test_cpp = self.repo / "research/companion_levelup/stub_test.cpp"
        test_cpp.write_text('#include <windows.h>\nint main(){}\n')

        # Create a mod that does NOT use stub windows
        mod_dir = self.repo / "patch_manager_mods/unrelated_mod_src"
        mod_dir.mkdir(parents=True)
        (mod_dir / "manifest.toml").write_text('[patch]\nid = "unrelated"\n')
        (mod_dir / "Unrelated.cpp").write_text('int foo(){ return 0; }\n')

        cg = CompatGraph(repo_root=self.repo)
        plan = cg.affected(["research/companion_levelup/stub/windows.h"])
        self.assertIn("cpp:stub_test", plan.tests())
        # Must not build unrelated DLL
        self.assertEqual(plan.builds(), [])

    def test_cross_mod_include(self):
        """Relative cross-mod include (shield_logic_test -> BuffHudLogic.h) is found."""
        hud_dir = self.repo / "patch_manager_mods/buff_duration_hud_src"
        hud_dir.mkdir(parents=True)
        (hud_dir / "manifest.toml").write_text('[patch]\nid = "buff-hud"\n')
        logic_h = hud_dir / "BuffHudLogic.h"
        logic_h.write_text("int calc();\n")

        shield_dir = self.repo / "patch_manager_mods/shield_bar_src"
        shield_dir.mkdir(parents=True)
        (shield_dir / "manifest.toml").write_text('[patch]\nid = "shield-bar"\n')
        shield_test = shield_dir / "shield_logic_test.cpp"
        shield_test.write_text('#include "../buff_duration_hud_src/BuffHudLogic.h"\nint main(){ return calc(); }\n')

        cg = CompatGraph(repo_root=self.repo)
        plan = cg.affected(["patch_manager_mods/buff_duration_hud_src/BuffHudLogic.h"])
        self.assertIn("cpp:shield_logic_test", plan.tests())

    def test_deleted_header(self):
        """A deleted header reached via unresolved include triggers the includer."""
        consumer = self.repo / "patch_manager_mods/consumer_test.cpp"
        consumer.write_text('#include "DeletedHeader.h"\nint main(){}\n')

        cg = CompatGraph(repo_root=self.repo)
        plan = cg.affected(["patch_manager_mods/_shared_src/DeletedHeader.h"])
        self.assertIn("cpp:consumer_test", plan.tests())

    def test_rename(self):
        """Renamed file triggers checks for both old (deleted) and new locations."""
        consumer = self.repo / "patch_manager_mods/rename_test.cpp"
        consumer.write_text('#include "OldHeader.h"\nint main(){}\n')

        new_h = self.repo / "patch_manager_mods/_shared_src/NewHeader.h"
        new_h.write_text("int new_h();\n")

        cg = CompatGraph(repo_root=self.repo)
        plan = cg.affected([
            "patch_manager_mods/_shared_src/OldHeader.h",
            "patch_manager_mods/_shared_src/NewHeader.h",
        ])
        self.assertIn("cpp:rename_test", plan.tests())

    def test_inc_chain(self):
        """Header including .inc fans out when .inc changes."""
        inc = self.repo / "patch_manager_mods/_shared_src/Table.inc"
        inc.write_text("1, 2, 3\n")

        hdr = self.repo / "patch_manager_mods/_shared_src/Icons.h"
        hdr.write_text('#include "Table.inc"\nint table[] = { 1 };\n')

        mod_dir = self.repo / "patch_manager_mods/sample_mod_src"
        mod_dir.mkdir(parents=True)
        (mod_dir / "manifest.toml").write_text('[patch]\nid = "sample"\n')
        (mod_dir / "Sample.cpp").write_text('#include "Icons.h"\n')

        cg = CompatGraph(repo_root=self.repo)
        plan = cg.affected(["patch_manager_mods/_shared_src/Table.inc"])
        self.assertIn("sample_mod_src", plan.builds())

    def test_manifest_library_fanout(self):
        """Library touch fans out to consumer mod builds and colocated tests."""
        lib_dir = self.repo / "patch_manager_mods/my_lib_src"
        lib_dir.mkdir(parents=True)
        (lib_dir / "manifest.toml").write_text(
            '[patch]\nid = "my-lib"\nkind = "library"\nprovides = ["my.iface@1"]\n'
        )
        (lib_dir / "Lib.cpp").write_text("int lib_fn(){ return 1; }\n")

        consumer_dir = self.repo / "patch_manager_mods/consumer_mod_src"
        consumer_dir.mkdir(parents=True)
        (consumer_dir / "manifest.toml").write_text(
            '[patch]\nid = "consumer"\nrequires = ["my-lib"]\nrequires_interfaces = ["my.iface@1"]\n'
        )
        (consumer_dir / "Consumer.cpp").write_text("int cons(){ return 2; }\n")
        (consumer_dir / "consumer_logic_test.cpp").write_text("int main(){}\n")

        cg = CompatGraph(repo_root=self.repo)
        plan = cg.affected(["patch_manager_mods/my_lib_src/Lib.cpp"])
        self.assertIn("my_lib_src", plan.builds())
        self.assertIn("consumer_mod_src", plan.builds())
        self.assertIn("cpp:consumer_logic_test", plan.tests())

    def test_nested_mod_key(self):
        """Source in nested dir (perf_lab_src/perflab) resolves to nearest manifest."""
        nested_dir = self.repo / "patch_manager_mods/perf_lab_src/perflab"
        nested_dir.mkdir(parents=True)
        (nested_dir / "manifest.toml").write_text('[patch]\nid = "perf-lab"\n')
        src_file = nested_dir / "PerfLab.cpp"
        src_file.write_text("int perf();\n")

        cg = CompatGraph(repo_root=self.repo)
        plan = cg.affected(["patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp"])
        self.assertIn("perf_lab_src/perflab", plan.builds())


class TestCompatGraphLiveRepo(unittest.TestCase):
    """Golden checks against the real repository."""

    @classmethod
    def setUpClass(cls):
        cls.repo_root = find_repo_root()
        cls.cg = CompatGraph(repo_root=cls.repo_root)

    def test_golden_gui2d(self):
        """Touching Gui2D.h reaches Section A items."""
        plan = self.cg.affected(["patch_manager_mods/_shared_src/Gui2D.h"])
        builds = plan.builds()
        tests = plan.tests()
        static = plan.static()

        # Section A builds: buff_duration_hud_src, portrait_slots_src, shield_bar_src
        self.assertIn("buff_duration_hud_src", builds)
        self.assertIn("portrait_slots_src", builds)
        self.assertIn("shield_bar_src", builds)

        # Section A host tests: gui2d_test, k2addr_test
        self.assertIn("cpp:gui2d_test", tests)
        self.assertIn("cpp:k2addr_test", tests)

        # Section A static checks: check-shared-headers, check-raw-addresses
        self.assertIn("suite:check-shared-headers", static)
        self.assertIn("suite:check-raw-addresses", static)

        # KPM consumers
        self.assertIn("BuffDurationHUD", plan.kpm_consumers)
        self.assertIn("PortraitSlots", plan.kpm_consumers)
        self.assertIn("ShieldBar", plan.kpm_consumers)

    def test_golden_portrait_slots_provider(self):
        """Touching PortraitSlotsProvider.cpp reaches Section B items."""
        plan = self.cg.affected(["patch_manager_mods/portrait_slots_src/PortraitSlotsProvider.cpp"])
        builds = plan.builds()
        tests = plan.tests()
        static = plan.static()

        # Section B builds: portrait_slots_src and consumer mods buff_duration_hud_src, shield_bar_src
        self.assertIn("portrait_slots_src", builds)
        self.assertIn("buff_duration_hud_src", builds)
        self.assertIn("shield_bar_src", builds)

        # Section B tests: colocated tests
        self.assertIn("cpp:ps_dispatch_test", tests)
        self.assertIn("cpp:buff_hud_logic_test", tests)
        self.assertIn("cpp:shield_logic_test", tests)

        # Section B static
        self.assertIn("suite:check-raw-addresses", static)
        self.assertIn("suite:check-subclassers", static)

    def test_golden_saferead(self):
        """Touching SafeRead.h reaches Section C items."""
        plan = self.cg.affected(["patch_manager_mods/_shared_src/SafeRead.h"], include_wine=True)
        tests = plan.tests()
        suites = plan.suites()
        static = plan.static()

        # Section C tests include saferead_test, gui2d_test, k2addr_test, party_test
        self.assertIn("cpp:saferead_test", tests)
        self.assertIn("cpp:gui2d_test", tests)
        self.assertIn("cpp:k2addr_test", tests)
        self.assertIn("cpp:party_test", tests)

        # Section C suites: the 4 parity suites
        self.assertIn("suite:levelup-parity", suites)
        self.assertIn("suite:levelup-scenarios", suites)
        self.assertIn("suite:levelup-scenarios-m32", suites)
        self.assertIn("suite:guikit", suites)

        # Section C static: 2 checks
        self.assertIn("suite:check-shared-headers", static)
        self.assertIn("suite:check-raw-addresses", static)


if __name__ == "__main__":
    unittest.main()
