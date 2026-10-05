#!/usr/bin/env python3
"""test_generate_repo_map.py - Unit test suite for generate_repo_map.py"""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))
import generate_repo_map as grm  # noqa: E402


class TestGenerateRepoMap(unittest.TestCase):
    def test_parse_cpp(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            f = Path(tmpdir) / "sample.cpp"
            f.write_text(
                "class DetourHook : public BaseHook {\n"
                "public:\n"
                "    static void Install(int addr, void* target);\n"
                "    bool IsActive() const override;\n"
                "};\n"
                "struct GameAddr {\n"
                "    uint32_t offset;\n"
                "};\n"
                "void HandleInput(int code) {\n"
                "    if (code > 0) return;\n"
                "}\n"
            )
            info = grm.parse_file(f)
            self.assertIn("DetourHook : BaseHook", info["classes"])
            self.assertIn("GameAddr", info["classes"])
            fn_names = [fn.split("(")[0] for fn in info["functions"]]
            self.assertIn("Install", fn_names)
            self.assertIn("IsActive", fn_names)
            self.assertIn("HandleInput", fn_names)
            self.assertNotIn("if", fn_names)

    def test_parse_py(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            f = Path(tmpdir) / "sample.py"
            f.write_text(
                "class PlanManager:\n"
                "    def __init__(self, plan_path):\n"
                "        self.path = plan_path\n"
                "    def resume_phase(self):\n"
                "        pass\n"
                "async def fetch_updates():\n"
                "    return True\n"
            )
            info = grm.parse_file(f)
            self.assertIn("class PlanManager", info["classes"])
            fn_names = [fn.split("(")[0].replace("def ", "") for fn in info["functions"]]
            self.assertIn("__init__", fn_names)
            self.assertIn("resume_phase", fn_names)
            self.assertIn("fetch_updates", fn_names)

    def test_generate_map_and_skip_dirs(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            valid_dir = root / "src"
            valid_dir.mkdir()
            (valid_dir / "module.py").write_text("def run_task():\n    pass\n")

            skip_dir = root / "__pycache__"
            skip_dir.mkdir()
            (skip_dir / "cached.py").write_text("def cached_fn():\n    pass\n")

            out = grm.generate_map(root)
            self.assertIn("run_task", out)
            self.assertNotIn("cached_fn", out)

    def test_max_lines_truncation(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            for i in range(15):
                f = root / f"file_{i:02d}.py"
                f.write_text(f"class Cls_{i}:\n    def method_{i}():\n        pass\n")

            out = grm.generate_map(root, max_lines=10)
            lines = out.splitlines()
            self.assertLessEqual(len(lines), 16)
            self.assertTrue(any("remaining files truncated" in line for line in lines))

    def test_cli_execution(self):
        # Run CLI on scripts directory
        cmd = [sys.executable, str(REPO_ROOT / "scripts" / "generate_repo_map.py"), "scripts", "--max-lines", "50"]
        res = subprocess.run(cmd, cwd=str(REPO_ROOT), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.assertEqual(res.returncode, 0, f"Stderr: {res.stderr}")
        self.assertIn("# Structural Repo Map: scripts", res.stdout)
        self.assertIn("`scripts/", res.stdout)

    def test_cli_non_existent_dir(self):
        cmd = [sys.executable, str(REPO_ROOT / "scripts" / "generate_repo_map.py"), "non_existent_directory_xyz"]
        res = subprocess.run(cmd, cwd=str(REPO_ROOT), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.assertEqual(res.returncode, 2)


if __name__ == "__main__":
    unittest.main()
