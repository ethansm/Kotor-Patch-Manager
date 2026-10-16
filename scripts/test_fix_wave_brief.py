#!/usr/bin/env python3
"""Host-only tests for fix_wave_brief.py. Every run uses a temp repo root and a temp waves root; wave_tools, git and the
agent contract text are stubbed, so nothing touches the real .claude/waves/."""
import contextlib
import io
import json
import re
import shutil
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

SCRIPTS = Path(__file__).resolve().parent
REPO = SCRIPTS.parent
sys.path.insert(0, str(SCRIPTS))
import fix_wave_brief as fb  # noqa: E402

TEMPLATE_SRC = REPO / ".claude" / "skills" / "variant-fix-wave"
TEMPLATES = ("variants.md", "consolidate.md", "review.md")
ITEM_COLS = ["id", "kind", "size", "group", "after", "fids", "write", "tests", "mods", "entries", "anchors",
             "constraint", "context", "doc"]
BATCH_COLS = ["batch", "wave_id", "units", "agents", "test_usd", "ids"]
ITEMS = [
    {"id": "S1", "kind": "bug", "size": "S", "group": "-", "after": "", "fids": "", "write": "scripts/tiny.py",
     "tests": "py:scripts/test_tiny.py", "mods": "", "entries": "scripts/tiny.py:1-2", "anchors": "def tiny",
     "constraint": "tiny() must return 2.", "context": "scripts/notes.md#Tiny", "doc": ""},
    {"id": "M1", "kind": "bug", "size": "M", "group": "-", "after": "", "fids": "F-1",
     "write": "scripts/tiny.py;scripts/test_tiny.py", "tests": "py:scripts/test_tiny.py", "mods": "",
     "entries": "scripts/tiny.py:1-2;scripts/test_tiny.py:1-3;scripts/extra.py:1-3;scripts/tiny.py:2-2",
     "anchors": "def tiny;return", "constraint": "Keep tiny() pure.", "context": "",
     "doc": "scripts/bugboard.md#Doc"},
]
FILES = {
    "scripts/tiny.py": "def tiny():\n    return 1\n",
    "scripts/test_tiny.py": "import unittest\n\nclass T(unittest.TestCase):\n    pass\n",
    "scripts/extra.py": "x = 1\n",
    "scripts/notes.md": "## Tiny\ntiny note line\n## Other\nother line\n",
    "scripts/bugboard.md": "### Doc\ndoc body line\n### Doc2\nsecond\n",
    "findings.tsv": "fid\treal\tseverity\nF-1\ttiny() returns 1\thigh\nF-2\tother\tlow\n",
}


def write_tsv(path, header, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = ["\t".join(header)] + ["\t".join(r.get(h, "") for h in header) for r in rows]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def snapshot(root):
    return {p.relative_to(root).as_posix(): p.read_bytes() for p in root.rglob("*") if p.is_file()}


class BriefCase(unittest.TestCase):
    def setUp(self):
        td = tempfile.TemporaryDirectory()
        self.addCleanup(td.cleanup)
        self.root = Path(td.name) / "repo"
        self.wr = self.root / ".claude" / "waves"
        tdir = self.root / ".claude" / "skills" / "variant-fix-wave"
        tdir.mkdir(parents=True)
        for name in TEMPLATES:
            shutil.copyfile(TEMPLATE_SRC / name, tdir / name)
        for rel, text in FILES.items():
            p = self.root / rel
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text(text, encoding="utf-8")
        wdir = self.wr / "W"
        write_tsv(wdir / "items.tsv", ITEM_COLS, ITEMS)
        write_tsv(wdir / "batches.tsv", BATCH_COLS,
                  [{"batch": "A", "wave_id": "W-A", "units": "4", "agents": "6", "test_usd": "0.20", "ids": "S1;M1"}])
        (wdir / "plan.json").write_text(json.dumps({
            "items": str(wdir / "items.tsv"), "findings": "findings.tsv",
            "cols": ["fid", "real", "severity"], "limits": {}}), encoding="utf-8")
        self.calls, self.added, self.rejects = [], [], set()
        for name, value in (("git_base", "abc1234"), ("contract_text", "CONTRACT-STUB")):
            patcher = mock.patch.object(fb, name, return_value=value)
            patcher.start()
            self.addCleanup(patcher.stop)

    def fake_tools(self, args, repo_root):
        """Stands in for wave_tools.py: `new` creates the batch dir and manifest, `add` records the uid unless rejected."""
        self.calls.append(list(args))
        if args[0] == "new":
            d = self.wr / args[-1]
            if d.exists():
                return 1, f"error: wave already exists: {args[-1]}"
            d.mkdir(parents=True)
            (d / "manifest.json").write_text(json.dumps({"wave": args[-1], "units": []}), encoding="utf-8")
            return 0, f"created {args[-1]}/"
        if args[0] == "add":
            uid = args[args.index("--id") + 1]
            if uid in self.rejects:
                return 1, f"error: rejected {uid} for the test"
            self.added.append(uid)
            return 0, "ok"
        return 2, "unknown command"

    def run_build(self, batch="A", dry_run=False, force=False):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = fb.build(self.wr, self.root, "W", batch, dry_run, force, run_wave_tools=self.fake_tools)
        return rc, out.getvalue(), err.getvalue()

    def add_tasks(self):
        return {c[c.index("--id") + 1]: c[c.index("--task") + 1] for c in self.calls if c[0] == "add"}

    def add_labels(self):
        return [re.match(r"Unit \S+ = (\S+)\.", t).group(1) for _, t in sorted(self.add_tasks().items())]


class EntTests(unittest.TestCase):
    def test_repo_root_dirs_keep_their_path(self):
        import fix_wave_apply as fa
        import fix_wave_plan as fp
        for ent in (fb.ent, fa.ent, fp.ent):
            for p in ("tests/a.toml", "research/x/y.py", ".claude/review/f.py", "scripts/s.py",
                      "patch_manager_mods/m/a.h"):
                self.assertEqual(ent(p), p)
            self.assertEqual(ent("m_src/a.h"), "patch_manager_mods/m_src/a.h")


class SectionTests(unittest.TestCase):
    def test_section_stops_at_next_level_two_heading(self):
        with tempfile.TemporaryDirectory() as td:
            p = Path(td) / "s.md"
            p.write_text("# Top\n## One\ntext one\n### sub\nsub text\n## Two\ntext two\n", encoding="utf-8")
            got = fb.section(p, "One")
            self.assertIn("## One", got)
            self.assertIn("text one", got)
            self.assertIn("sub text", got)
            self.assertNotIn("text two", got)
            self.assertNotIn("## Two", got)
            self.assertEqual(fb.section(p, "On"), f"(section On not found in {p})")
            self.assertEqual(fb.section(p, "Nope"), f"(section Nope not found in {p})")

    def test_variants_table(self):
        self.assertEqual(fb.variants("bug", "S"), ["A"])
        self.assertEqual(fb.variants("bug", ""), ["A", "B", "C"])
        self.assertEqual(fb.variants("bug", "L"), ["A", "B", "C"])
        self.assertEqual(fb.variants("gap", "S"), ["T2"])
        self.assertEqual(fb.variants("gap", "M"), ["T2"])
        self.assertEqual(fb.variants("gap", "L"), ["T1", "T2"])
        with self.assertRaises(ValueError):
            fb.variants("bug", "XL")

    def test_variant_texts_load_with_format_tail(self):
        var, fmt = fb.load_variants((TEMPLATE_SRC / "variants.md").read_text(encoding="utf-8"))
        self.assertEqual(sorted(var), ["A", "B", "C", "T1", "T2"])
        self.assertTrue(var["A"].startswith("VARIANT A (minimal):"))
        self.assertIn("## Design", fmt)
        self.assertIn("## Risks", fmt)
        self.assertTrue(fmt.endswith("UNSURE = could not."))


class BriefBuildTests(BriefCase):
    def test_dry_run_writes_nothing(self):
        before = snapshot(self.root)
        rc, out, err = self.run_build(dry_run=True)
        self.assertEqual(rc, 0, err)
        self.assertEqual(self.calls, [])
        self.assertEqual(snapshot(self.root), before)
        self.assertEqual(out.count("DRY "), 4)
        self.assertIn("DRY u01 S1-A ", out)
        self.assertIn("4 units; 2 items", out)

    def test_rerun_without_force_is_noop(self):
        rc, _, err = self.run_build()
        self.assertEqual(rc, 0, err)
        calls = len(self.calls)
        rc, out, _ = self.run_build()
        self.assertEqual(rc, 0)
        self.assertIn("exists: W-A (use --force)", out)
        self.assertEqual(len(self.calls), calls)

    def test_force_regenerates(self):
        self.run_build()
        marker = self.wr / "W-A" / "stale.txt"
        marker.write_text("old run", encoding="utf-8")
        calls = len(self.calls)
        rc, _, err = self.run_build(force=True)
        self.assertEqual(rc, 0, err)
        self.assertFalse(marker.exists())
        self.assertGreater(len(self.calls), calls)
        self.assertTrue((self.wr / "W-A" / "manifest.json").is_file())
        self.assertTrue((self.wr / "W-A" / "M1" / "consolidate.md").is_file())

    def test_rejected_add_exits_3_and_others_still_added(self):
        self.rejects = {"u03"}
        rc, out, _ = self.run_build()
        self.assertEqual(rc, 3)
        self.assertIn("REJECTED u03 M1-B", out)
        self.assertEqual(self.added, ["u01", "u02", "u04"])
        self.assertTrue((self.wr / "W-A" / "M1" / "pack.md").is_file())

    def test_missing_batches_refused(self):
        (self.wr / "W" / "batches.tsv").unlink()
        rc, _, err = self.run_build()
        self.assertEqual(rc, 2)
        self.assertEqual(self.calls, [])
        self.assertIn("batches.tsv", err)

    def test_unknown_batch_refused(self):
        rc, _, err = self.run_build(batch="Z")
        self.assertEqual(rc, 2)
        self.assertEqual(self.calls, [])
        self.assertIn("unknown batch", err)

    def test_unit_count_s_bug_one_m_bug_three(self):
        rc, _, err = self.run_build()
        self.assertEqual(rc, 0, err)
        self.assertEqual(self.add_labels(), ["S1-A", "M1-A", "M1-B", "M1-C"])

    def test_bad_kind_refused_and_nothing_written(self):
        bad = [dict(r, kind="feature") if r["id"] == "M1" else r for r in ITEMS]
        write_tsv(self.wr / "W" / "items.tsv", ITEM_COLS, bad)
        before = snapshot(self.root)
        rc, _, err = self.run_build()
        self.assertEqual(rc, 2)
        self.assertIn("row M1", err)
        self.assertEqual(self.calls, [])
        self.assertEqual(snapshot(self.root), before)

    def test_consolidate_has_kpm_dir(self):
        rc, _, err = self.run_build()
        self.assertEqual(rc, 0, err)
        for iid in ("S1", "M1"):
            text = (self.wr / "W-A" / iid / "consolidate.md").read_text(encoding="utf-8")
            self.assertIn("--kpm-dir", text)
            self.assertIn("CONTRACT-STUB", text)
            self.assertIn("/tmp/claude-1000/fixwave-W-" + iid, text)

    def test_review_brief_has_request_changes_rule(self):
        rc, _, err = self.run_build()
        self.assertEqual(rc, 0, err)
        text = (self.wr / "W-A" / "M1" / "review.brief.md").read_text(encoding="utf-8")
        self.assertIn("REQUEST_CHANGES", text)
        self.assertIn(".claude/waves/W-A/M1/review.md", text)
        self.assertNotIn("{", text)

    def test_pack_has_findings_doc_context_and_tail(self):
        rc, _, err = self.run_build()
        self.assertEqual(rc, 0, err)
        m1 = (self.wr / "W-A" / "M1" / "pack.md").read_text(encoding="utf-8")
        self.assertIn("| F-1 | tiny() returns 1 | high |", m1)
        self.assertNotIn("| F-2 |", m1)
        self.assertIn("doc body line", m1)
        self.assertNotIn("second", m1)
        self.assertIn("### tail of scripts/test_tiny.py", m1)
        s1 = (self.wr / "W-A" / "S1" / "pack.md").read_text(encoding="utf-8")
        self.assertIn("## Origin", s1)
        self.assertIn("tiny note line", s1)
        self.assertNotIn("other line", s1)

    def test_entries_beyond_three_become_read_windows(self):
        rc, _, err = self.run_build()
        self.assertEqual(rc, 0, err)
        m1 = [c for c in self.calls if c[0] == "add" and c[c.index("--task") + 1].startswith("Unit u02 ")][0]
        self.assertEqual(m1[m1.index("--entries") + 1:], ["scripts/tiny.py:1-2", "scripts/test_tiny.py:1-3", "scripts/extra.py:1-3"])
        task = m1[m1.index("--task") + 1]
        self.assertIn("Also read these windows (Read with offset/limit, nothing more): scripts/tiny.py:2-2", task)

    def test_main_dry_run_routes_to_build(self):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            rc = fb.main(["brief", "W", "--batch", "A", "--dry-run", "--repo-root", str(self.root)])
        self.assertEqual(rc, 0)
        self.assertIn("DRY u01 S1-A", out.getvalue())
        self.assertEqual(self.calls, [])


if __name__ == "__main__":
    unittest.main()
