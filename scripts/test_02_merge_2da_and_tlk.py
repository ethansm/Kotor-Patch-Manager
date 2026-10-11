#!/usr/bin/env python3
"""scripts/test_02_merge_2da_and_tlk.py - tests for the live-table guard in 02_merge_2da_and_tlk.py (BG01).

Synthetic TwoDA objects only; main() runs against a tmp dir with every reader and writer faked,
so the real game directory is never touched.
"""
from __future__ import annotations

import copy
import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

from pykotor.resource.formats.twoda import TwoDA  # noqa: E402

_spec = importlib.util.spec_from_file_location("merge_2da_and_tlk", SCRIPT_DIR / "02_merge_2da_and_tlk.py")
M = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(M)

PLAN_HEADERS = ["label", "model", "icon", "guitag", "name", "description", "lockedoutreason"]


def make(headers, rows):
    t = TwoDA(list(headers))
    for label, cells in rows:
        i = t.add_row()
        t.set_label(i, label)
        for h in headers:
            t.set_cell(i, h, cells.get(h, "x"))
    return t


def plan(overrides=()):
    rows = [(f"p{i}", {"label": f"p{i}", "name": str(i)}) for i in range(6)]
    t = make(PLAN_HEADERS, rows)
    for (r, h), v in dict(overrides).items():
        t.set_cell(r, h, v)
    return t


class TestTableConflicts(unittest.TestCase):
    def test_identical(self):
        self.assertEqual(M.table_conflicts(plan(), plan()), [])

    def test_extra_live_row(self):
        live = plan()
        i = live.add_row()
        live.set_label(i, "other_mod")
        out = M.table_conflicts(live, plan())
        self.assertEqual(len(out), 1)
        self.assertIn("height", out[0])

    def test_changed_non_tlk_cell(self):
        out = M.table_conflicts(plan({(2, "model"): "mine"}), plan(), ("name",))
        self.assertEqual(len(out), 1)
        self.assertIn("row 2 p2 model", out[0])

    def test_changed_label(self):
        live = plan()
        live.set_label(3, "renamed")
        out = M.table_conflicts(live, plan())
        self.assertTrue(any("row 3 label" in line for line in out))

    def test_tlk_column_ignored(self):
        live = plan({(1, "name"): "999", (1, "description"): "888"})
        self.assertEqual(M.table_conflicts(live, plan(), ("name", "description", "lockedoutreason")), [])

    def test_skipped_row_ignored(self):
        live = plan({(4, "model"): "****", (4, "icon"): "****"})
        self.assertEqual(M.table_conflicts(live, plan(), (), (4,)), [])
        self.assertTrue(M.table_conflicts(live, plan()))

    def test_header_mismatch(self):
        out = M.table_conflicts(make(["a"], [("r", {})]), make(["a", "b"], [("r", {})]))
        self.assertEqual(len(out), 1)
        self.assertIn("headers", out[0])


class TestLoadGuard(unittest.TestCase):
    def test_missing_live_files_skipped(self):
        with tempfile.TemporaryDirectory() as d:
            live, stage = Path(d, "live"), Path(d, "stage")
            live.mkdir()
            stage.mkdir()
            with mock.patch.object(M, "LIVE_OVERRIDE", live), mock.patch.object(M, "STAGING_OVERRIDE", stage):
                self.assertEqual(M.load_live_tables_to_guard(plan()), [])


class TestMainGuard(unittest.TestCase):
    """Run main() with synthetic inputs; record every write instead of performing it."""

    def run_main(self, live_plan, argv=()):
        names = list(M.ECHOES_FIRST_ROW) + ["spells.2da", "effecticon.2da", "planetary.2da"]
        with tempfile.TemporaryDirectory() as d:
            live, stage = Path(d, "live", "override"), Path(d, "stage", "Override")
            live.mkdir(parents=True)
            stage.mkdir(parents=True)
            tables = {}
            for n in names + M.COPIED_2DAS:
                (stage / n).write_bytes(b"")
                tables[stage / n] = plan() if n == "planetary.2da" else make(["name"], [(f"r{i}", {"name": f"n{i}"}) for i in range(3)])
                if n != "planetary.2da":
                    (live / n).write_bytes(b"")
                    tables[live / n] = copy.deepcopy(tables[stage / n])
            (live / "planetary.2da").write_bytes(b"")
            tables[live / "planetary.2da"] = live_plan
            (live / "genericdoors.2da").write_bytes(b"")
            (stage / "dialog.tlk").write_bytes(b"")
            (Path(d, "live") / "dialog.tlk").write_bytes(b"")
            (stage / "ekunholo.uti").write_bytes(b"")
            (stage / "ekun_holo.ncs").write_bytes(b"")
            writes = []
            fake_tlk = mock.MagicMock()
            fake_tlk.__len__.return_value = 0
            fake_first = {k: 1 for k in M.ECHOES_FIRST_ROW}
            patches = [
                mock.patch.object(M, "LIVE_ROOT", Path(d, "live")),
                mock.patch.object(M, "LIVE_OVERRIDE", live),
                mock.patch.object(M, "STAGING_ROOT", Path(d, "stage")),
                mock.patch.object(M, "STAGING_OVERRIDE", stage),
                mock.patch.object(M, "ECHOES_TLK_START", 0),
                mock.patch.object(M, "ECHOES_FIRST_ROW", fake_first),
                mock.patch.object(M, "ECHOES_SPELL_ROW", 0),
                mock.patch.object(M, "ECHOES_EFFECTICON_ROW", 0),
                mock.patch.object(M, "UTC_APPEARANCE", {}),
                mock.patch.object(M, "already_applied", lambda *a: None),
                mock.patch.object(M, "read_tlk", lambda p: fake_tlk),
                mock.patch.object(M, "read_2da", lambda p: copy.deepcopy(tables[Path(p)])),
                mock.patch.object(M, "read_gff", lambda p: mock.MagicMock()),
                mock.patch.object(M, "read_ncs", lambda p: mock.MagicMock(instructions=[])),
                mock.patch.object(M, "write_2da", lambda t, p: writes.append(("2da", Path(p).name))),
                mock.patch.object(M, "write_gff", lambda o, p: writes.append(("gff", Path(p).name))),
                mock.patch.object(M, "write_ncs", lambda o, p: writes.append(("ncs", Path(p).name))),
                mock.patch.object(M, "write_tlk", lambda o, p: writes.append(("tlk", Path(p).name))),
                mock.patch.object(M.shutil, "copy2", lambda s, t: writes.append(("copy", Path(t).name))),
                mock.patch.object(sys, "argv", ["02_merge_2da_and_tlk.py", *argv]),
            ]
            for p in patches:
                p.start()
            try:
                try:
                    M.main()
                    code = None
                except SystemExit as e:
                    code = e.code
            finally:
                for p in reversed(patches):
                    p.stop()
        return code, writes

    def test_matching_live_planetary_writes(self):
        # live == staging except in the ignored TLK columns and the M4-78 row
        live = plan({(1, "name"): "777", (4, "model"): "****"})
        code, writes = self.run_main(live)
        self.assertIsNone(code)
        self.assertIn(("2da", "planetary.2da"), writes)
        self.assertIn(("copy", "soundset.2da"), writes)

    def test_conflict_exits_and_writes_nothing(self):
        live = plan({(2, "model"): "other_mod_planet"})
        code, writes = self.run_main(live)
        self.assertIsInstance(code, str)
        self.assertIn("--replace-tables", code)
        self.assertEqual(writes, [])

    def test_extra_live_planet_row_blocks(self):
        live = plan()
        i = live.add_row()
        live.set_label(i, "other_mod")
        code, writes = self.run_main(live)
        self.assertIsInstance(code, str)
        self.assertEqual(writes, [])

    def test_replace_tables_flag_overrides(self):
        live = plan({(2, "model"): "other_mod_planet"})
        code, writes = self.run_main(live, ["--replace-tables"])
        self.assertIsNone(code)
        self.assertIn(("2da", "planetary.2da"), writes)


if __name__ == "__main__":
    unittest.main()
