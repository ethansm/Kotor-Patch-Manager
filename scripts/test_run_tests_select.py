#!/usr/bin/env python3
"""
Tests for run_tests.py selection and metadata hooks (Doc 22 U2):
- Item.src / covers / consumer metadata
- covers extraction from test.toml
- --id exact matching (repeatable)
- --ids-file matching
- --list --json serialization
"""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent
sys.path.insert(0, str(HERE))
import run_tests as rt  # noqa: E402


class ItemMetadataTests(unittest.TestCase):
    def test_item_default_fields(self):
        it = rt.Item(id="test:1", kind="test")
        self.assertIsNone(it.src)
        self.assertEqual(it.covers, [])
        self.assertEqual(it.consumer, "")

    def test_cpp_item_src_and_consumer(self):
        # Non-contract test has empty consumer
        gui = rt.cpp_item(REPO / "patch_manager_mods/_shared_src/gui2d_test.cpp")
        self.assertEqual(gui.src, REPO / "patch_manager_mods/_shared_src/gui2d_test.cpp")
        self.assertEqual(gui.consumer, "")

        # Contract test derives consumer mod dir
        contract = rt.cpp_item(REPO / "patch_manager_mods/portrait_slots_src/common_contract_test.cpp")
        self.assertEqual(contract.consumer, "portrait_slots_src")

        # Nested contract test derives relative mod dir
        nested = rt.cpp_item(REPO / "patch_manager_mods/perf_lab_src/perflab/common_contract_test.cpp")
        self.assertEqual(nested.consumer, "perf_lab_src/perflab")

    def test_cpp_item_covers_from_cfg(self):
        cov = ["force_power_hotbar_src/ForcePowerHotbar.cpp", "k2.portraitslots@1"]
        it = rt.cpp_item(REPO / "patch_manager_mods/_shared_src/gui2d_test.cpp", cfg={"covers": cov})
        self.assertEqual(it.covers, cov)

        # Single string normalized to list
        it2 = rt.cpp_item(REPO / "patch_manager_mods/_shared_src/gui2d_test.cpp", cfg={"covers": "single/path.h"})
        self.assertEqual(it2.covers, ["single/path.h"])

    def test_py_and_unittest_items_have_src(self):
        py_it = rt.py_item(REPO / "scripts/test_compat_graph.py")
        self.assertEqual(py_it.src, REPO / "scripts/test_compat_graph.py")
        self.assertEqual(py_it.covers, [])
        self.assertEqual(py_it.consumer, "")

        unit_it = rt.unittest_item(REPO / "patch_manager_mods/engine_ai_atlas/tools/tests")
        self.assertEqual(unit_it.src, REPO / "patch_manager_mods/engine_ai_atlas/tools/tests")


class CliSelectionTests(unittest.TestCase):
    def _run_cli(self, *args):
        cmd = [sys.executable, str(HERE / "run_tests.py")] + list(args)
        return subprocess.run(cmd, cwd=REPO, capture_output=True, text=True)

    def test_id_exact_match(self):
        # Substring does not match
        res_sub = self._run_cli("--id", "gui2d", "--list")
        self.assertEqual(res_sub.returncode, 0)
        self.assertEqual(res_sub.stdout.strip(), "")

        # Exact match works
        res_exact = self._run_cli("--id", "cpp:gui2d_test", "--list")
        self.assertEqual(res_exact.returncode, 0)
        lines = [ln.strip() for ln in res_exact.stdout.splitlines() if ln.strip()]
        self.assertEqual(len(lines), 1)
        self.assertIn("cpp:gui2d_test", lines[0])

    def test_id_repeatable(self):
        res = self._run_cli("--id", "cpp:party_test", "--id", "cpp:gui2d_test", "--list")
        self.assertEqual(res.returncode, 0)
        lines = [ln.strip() for ln in res.stdout.splitlines() if ln.strip()]
        ids = [ln.split()[0] for ln in lines]
        self.assertEqual(set(ids), {"cpp:party_test", "cpp:gui2d_test"})

    def test_ids_file(self):
        with tempfile.NamedTemporaryFile("w", delete=False) as f:
            f.write("# comment\n\ncpp:party_test\ncpp:gui2d_test\n")
            fpath = f.name
        try:
            res = self._run_cli("--ids-file", fpath, "--list")
            self.assertEqual(res.returncode, 0)
            lines = [ln.strip() for ln in res.stdout.splitlines() if ln.strip()]
            ids = [ln.split()[0] for ln in lines]
            self.assertEqual(set(ids), {"cpp:party_test", "cpp:gui2d_test"})
        finally:
            Path(fpath).unlink(missing_ok=True)

    def test_ids_file_combined_with_id(self):
        with tempfile.NamedTemporaryFile("w", delete=False) as f:
            f.write("cpp:party_test\n")
            fpath = f.name
        try:
            res = self._run_cli("--ids-file", fpath, "--id", "cpp:gui2d_test", "--list")
            self.assertEqual(res.returncode, 0)
            lines = [ln.strip() for ln in res.stdout.splitlines() if ln.strip()]
            ids = [ln.split()[0] for ln in lines]
            self.assertEqual(set(ids), {"cpp:party_test", "cpp:gui2d_test"})
        finally:
            Path(fpath).unlink(missing_ok=True)

    def test_ids_file_missing_returns_error_code_2(self):
        res = self._run_cli("--ids-file", "/nonexistent/test_ids.txt", "--list")
        self.assertEqual(res.returncode, 2)
        self.assertIn("error: --ids-file not found", res.stderr)

    def test_positional_exact_id(self):
        res = self._run_cli("cpp:gui2d_test", "--list")
        self.assertEqual(res.returncode, 0)
        lines = [ln.strip() for ln in res.stdout.splitlines() if ln.strip()]
        self.assertEqual(len(lines), 1)
        self.assertIn("cpp:gui2d_test", lines[0])

    def test_positional_substring_filter(self):
        res = self._run_cli("gui2d", "--list")
        self.assertEqual(res.returncode, 0)
        lines = [ln.strip() for ln in res.stdout.splitlines() if ln.strip()]
        self.assertEqual(len(lines), 1)
        self.assertIn("cpp:gui2d_test", lines[0])


class ListJsonTests(unittest.TestCase):
    def _run_cli(self, *args):
        cmd = [sys.executable, str(HERE / "run_tests.py")] + list(args)
        return subprocess.run(cmd, cwd=REPO, capture_output=True, text=True)

    def test_list_json_stdout(self):
        res = self._run_cli("--id", "cpp:gui2d_test", "--list", "--json")
        self.assertEqual(res.returncode, 0)
        data = json.loads(res.stdout)
        self.assertIsInstance(data, list)
        self.assertEqual(len(data), 1)
        item = data[0]
        self.assertEqual(item["id"], "cpp:gui2d_test")
        self.assertEqual(item["kind"], "cpp")
        self.assertEqual(item["src"], "patch_manager_mods/_shared_src/gui2d_test.cpp")
        self.assertEqual(item["covers"], [])
        self.assertEqual(item["consumer"], "")

    def test_list_json_file(self):
        with tempfile.NamedTemporaryFile("w", delete=False) as f:
            out_file = f.name
        try:
            res = self._run_cli("--id", "cpp:gui2d_test", "--list", "--json", out_file)
            self.assertEqual(res.returncode, 0)
            data = json.loads(Path(out_file).read_text(encoding="utf-8"))
            self.assertIsInstance(data, list)
            self.assertEqual(len(data), 1)
            self.assertEqual(data[0]["id"], "cpp:gui2d_test")
        finally:
            Path(out_file).unlink(missing_ok=True)

    def test_contract_tests_have_consumer_in_list_json(self):
        res = self._run_cli("-k", "common_contract_test", "--list", "--json")
        self.assertEqual(res.returncode, 0)
        data = json.loads(res.stdout)
        self.assertGreater(len(data), 0)
        for item in data:
            self.assertTrue(item["consumer"], f"Item {item['id']} should have non-empty consumer")


if __name__ == "__main__":
    unittest.main()
