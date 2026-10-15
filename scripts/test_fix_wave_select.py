#!/usr/bin/env python3
"""scripts/test_fix_wave_select.py - tests for fix_wave_select.py (select and draft-collect).

Each test builds a fake repo (work-item index, WI files, waves root, claims dir) under tempfile.TemporaryDirectory().
No git call and no real wave_tools run: the dirty list, HEAD and the wave_tools runner are injected.
"""
import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPTS))

import fix_wave_plan as fwp  # noqa: E402
import fix_wave_select as fws  # noqa: E402
import wi_model as wm  # noqa: E402
import wi_state as ws  # noqa: E402

W = "W1"
DRAFT = f"{W}-draft"
TEMPLATE = "Draft {WI}. Write the row to {ROW}.\n{PREFILL}\n"
INDEX_HEADER = ("# Dev work items\n\n"
                "| done | rank | id | prio | state | size | after | title | rows | files | why |\n"
                "|---|---|---|---|---|---|---|---|---|---|---|\n")

# (id, rank, prio, index state, size, title, write files, verify line)
WIS = [
    ("WI-01", 1, "P0", "ready", "S", "Alpha fix", ["scripts/alpha.py"], "python3 scripts/run_tests.py test_alpha"),
    ("WI-02", 2, "P1", "ready", "M", "Alpha and beta fix", ["scripts/alpha.py", "scripts/beta.py"],
     "python3 scripts/run_tests.py test_alpha test_beta"),
    ("WI-03", 3, "P1", "ready", "S", "Gamma fix", ["scripts/gamma.py"], "python3 scripts/run_tests.py test_gamma"),
    ("WI-04", 4, "P2", "verify-only", "S", "Delta verify", ["scripts/delta.py"],
     "python3 scripts/run_tests.py test_delta"),
    ("WI-05", 5, "P2", "hold", "S", "Epsilon held", ["scripts/epsilon.py"], "python3 scripts/run_tests.py test_epsilon"),
]


def make_repo(root):
    work = Path(root) / ".claude" / "work_items"
    work.mkdir(parents=True)
    lines = [INDEX_HEADER.rstrip("\n")]
    for wid, rank, prio, state, size, title, files, _verify in WIS:
        lines.append(f"| [ ] | {rank} | {wid} | {prio} | {state} | {size} | - | {title} | 1 | {len(files)} | why |")
        body = [f"# {wid} {title}", "area: scripts", f"size: {size}", f"ready: {state}", "items: -",
                f"files: {'; '.join(files)}", "out-of-repo: none", "blockers: none", "docs: none",
                f"verify: {_verify}", "", "## Goal", f"{title}.", "", "## Steps", "1. Fix it.", "",
                "## Verify", "- run the tests"]
        (work / f"{wid}.md").write_text("\n".join(body) + "\n", encoding="utf-8")
    (work / "dev_work_items.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def row_text(wid, kind="bug", entries="", anchors="", tests="", constraint=""):
    cols = ("id", "kind", "entries", "anchors", "tests", "constraint")
    vals = (wid, kind, entries, anchors, tests, constraint)
    return "\t".join(cols) + "\n" + "\t".join(vals) + "\n"


class FakeRunner:
    """Records every wave_tools call. add_rc: return codes consumed by successive `add` calls (default 0)."""

    def __init__(self, add_rc=None):
        self.calls = []
        self.add_rc = list(add_rc or [])

    def __call__(self, argv):
        self.calls.append(list(argv))
        if argv[0] == "add" and self.add_rc:
            return self.add_rc.pop(0), "", "over budget"
        return 0, "", ""

    def adds(self):
        return [c for c in self.calls if c[0] == "add"]


def entries_of(argv):
    return argv[argv.index("--entries") + 1:argv.index("--repo-root")]


class SelectFixture(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        base = Path(tmp.name)
        self.root = base / "repo"
        self.waves = base / "waves"
        self.claims = base / "claims"
        make_repo(self.root)
        self.store = ws.Store(self.root, claims_dir=self.claims)
        self.items = wm.load_all(self.root)
        self.runner = FakeRunner()

    def select(self, **kw):
        args = dict(root=self.root, waves_root=self.waves, claims_dir=self.claims, session="S-test",
                    wave_runner=self.runner, dirty_fn=lambda r: set(), head_fn=lambda r: "abc1234",
                    template=TEMPLATE)
        args.update(kw)
        with contextlib.redirect_stdout(io.StringIO()) as out:
            kept = fws.select(W, **args)
        return kept, out.getvalue()

    def draft_collect(self):
        with contextlib.redirect_stdout(io.StringIO()) as out:
            fit = fws.draft_collect(W, root=self.root, waves_root=self.waves, claims_dir=self.claims)
        return fit, out.getvalue()

    def write_row(self, wid, body):
        path = self.waves / DRAFT / f"{wid}.row.tsv"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(body, encoding="utf-8")

    def claim_ids(self):
        return sorted(p.stem for p in self.claims.glob("WI-*.claim"))

    def candidates(self):
        return fws.read_tsv(self.waves / W / "candidates.tsv")


class TestSelect(SelectFixture):
    def test_top_cap_keeps_first_n_in_rank_order(self):
        kept, _ = self.select(top=2)
        self.assertEqual(kept, ["WI-01", "WI-02"])
        self.assertEqual([r["id"] for r in self.candidates()], ["WI-01", "WI-02"])
        self.assertEqual(self.claim_ids(), ["WI-01", "WI-02"])

    def test_top_cap_three(self):
        kept, _ = self.select(top=3)
        self.assertEqual(kept, ["WI-01", "WI-02", "WI-03"])

    def test_rerun_on_selected_wave_is_refused(self):
        self.select(top=2)
        with self.assertRaises(fws.SelectError):
            self.select(top=2)
        self.assertEqual(self.claim_ids(), ["WI-01", "WI-02"])

    def test_top_above_max_is_error(self):
        with self.assertRaises(fws.SelectError):
            self.select(top=13)
        self.assertFalse((self.waves / W).exists())
        self.assertEqual(self.runner.calls, [])

    def test_verify_only_and_hold_dropped(self):
        kept, _ = self.select(top=10)
        self.assertEqual(kept, ["WI-01", "WI-02", "WI-03"])
        self.assertNotIn("WI-04", kept)
        self.assertNotIn("WI-05", kept)

    def test_overlapping_locksets_share_group(self):
        self.select(top=10)
        groups = {r["id"]: r["group"] for r in self.candidates()}
        self.assertEqual(groups, {"WI-01": "g1", "WI-02": "g1", "WI-03": "-"})

    def test_claims_created_with_wave_prefix(self):
        self.select(top=10)
        self.assertEqual(self.claim_ids(), ["WI-01", "WI-02", "WI-03"])
        for wid in ("WI-01", "WI-02", "WI-03"):
            claim = self.store.read_claim(wid)
            self.assertEqual(claim["wave_prefix"], f"VW-{W}")
            self.assertEqual(claim["locks"], wm.lockset(self.items[wid]))
            self.assertEqual(claim["base_head"], "abc1234")
            self.assertEqual(claim["session"], "S-test")

    def test_dry_run_writes_nothing(self):
        kept, out = self.select(top=10, dry_run=True, head_fn=lambda r: self.fail("HEAD read in dry-run"))
        self.assertEqual(kept, ["WI-01", "WI-02", "WI-03"])
        self.assertEqual(self.runner.calls, [])
        self.assertEqual(self.claim_ids(), [])
        self.assertFalse((self.waves / W).exists())
        self.assertFalse((self.waves / DRAFT).exists())
        self.assertIn("WI-01 P0 S g1 scripts/alpha.py", out)
        self.assertIn("dry-run: 3 candidate(s)", out)

    def test_pre_existing_claim_excludes(self):
        self.store.create_claim("WI-01", "other-session", wm.lockset(self.items["WI-01"]), "0000000",
                                wave_prefix="VW-OTHER")
        kept, _ = self.select(top=10)
        # WI-01 is claimed; WI-02 shares scripts/alpha.py with it, so only WI-03 is left
        self.assertEqual(kept, ["WI-03"])
        self.assertEqual(self.store.read_claim("WI-01")["session"], "other-session")
        self.assertEqual(self.claim_ids(), ["WI-01", "WI-03"])

    def test_draft_wave_units_and_task(self):
        self.select(top=2)
        calls = self.runner.calls
        self.assertEqual(calls[0], ["new", DRAFT, "--kind", "recon", "--repo-root", str(self.root.resolve())])
        adds = self.runner.adds()
        self.assertEqual([c[c.index("--id") + 1] for c in adds], ["u01", "u02"])
        first = adds[0]
        self.assertEqual(entries_of(first), [".claude/work_items/WI-01.md", "scripts/alpha.py"])
        task = next(a for a in first if a.startswith("--task="))
        self.assertIn("Draft WI-01.", task)
        self.assertIn(str((self.waves / DRAFT / "WI-01.row.tsv").resolve()), task)
        self.assertIn("id=WI-01", task)
        self.assertIn("group=g1", task)
        self.assertIn("tests=test_alpha", task)
        self.assertNotIn("{PREFILL}", task)
        units = (self.waves / DRAFT / "units.map.tsv").read_text(encoding="utf-8")
        self.assertEqual(units, "unit\twi\nu01\tWI-01\nu02\tWI-02\n")

    def test_over_budget_add_retries_with_wi_file_only(self):
        self.runner = FakeRunner(add_rc=[3])
        self.select(top=1)
        adds = self.runner.adds()
        self.assertEqual(len(adds), 2)
        self.assertEqual(entries_of(adds[0]), [".claude/work_items/WI-01.md", "scripts/alpha.py"])
        self.assertEqual(entries_of(adds[1]), [".claude/work_items/WI-01.md"])

    def test_dry_run_candidate_lines_match_group_and_files(self):
        _, out = self.select(top=10, dry_run=True)
        self.assertIn("WI-02 P1 M g1 scripts/alpha.py;scripts/beta.py", out)
        self.assertIn("WI-03 P1 S - scripts/gamma.py", out)


class TestDraftCollect(SelectFixture):
    def test_merges_row_into_items_input(self):
        self.select(top=2)
        self.write_row("WI-01", row_text("WI-01", kind="bug", entries="scripts/alpha.py:10-20",
                                         anchors="alpha_fix", tests="",
                                         constraint="Fix alpha. Red test first."))
        self.write_row("WI-02", row_text("WI-02", kind="gap", entries="scripts/beta.py:1-5",
                                         tests="test_beta", constraint="Add beta coverage."))
        fit, out = self.draft_collect()
        self.assertEqual(fit, ["WI-01", "WI-02"])
        self.assertIn("fit=2 unfit=0", out)
        path = self.waves / W / "items.input.tsv"
        rows = fws.read_tsv(path)
        self.assertEqual(path.read_text(encoding="utf-8").splitlines()[0], "\t".join(fwp.COLUMNS))
        self.assertEqual(list(rows[0].keys()), list(fwp.COLUMNS))
        first = rows[0]
        self.assertEqual(first["kind"], "bug")
        self.assertEqual(first["entries"], "scripts/alpha.py:10-20")
        self.assertEqual(first["anchors"], "alpha_fix")
        self.assertEqual(first["constraint"], "Fix alpha. Red test first.")
        self.assertEqual(first["tests"], "test_alpha")  # empty drafter cell keeps the pre-fill
        self.assertEqual(first["group"], "g1")
        self.assertEqual(first["write"], "scripts/alpha.py")
        self.assertEqual(first["context"], ".claude/work_items/WI-01.md#Goal")
        self.assertEqual(self.claim_ids(), ["WI-01", "WI-02"])

    def test_unfit_file_releases_claim(self):
        self.select(top=2)
        self.write_row("WI-01", "UNFIT: needs an owner decision on the alpha API\n")
        self.write_row("WI-02", row_text("WI-02", kind="gap", entries="scripts/beta.py:1-5",
                                         constraint="Add beta coverage."))
        fit, _ = self.draft_collect()
        self.assertEqual(fit, ["WI-02"])
        unfit = fws.read_tsv(self.waves / W / "unfit.tsv")
        self.assertEqual(unfit, [{"id": "WI-01", "reason": "needs an owner decision on the alpha API"}])
        self.assertIsNone(self.store.read_claim("WI-01"))
        self.assertEqual(self.claim_ids(), ["WI-02"])

    def test_missing_row_releases_claim(self):
        self.select(top=2)
        self.write_row("WI-01", row_text("WI-01", kind="bug", entries="scripts/alpha.py:1-3",
                                         constraint="Fix alpha."))
        fit, out = self.draft_collect()
        self.assertEqual(fit, ["WI-01"])
        self.assertIn("released: WI-02", out)
        unfit = fws.read_tsv(self.waves / W / "unfit.tsv")
        self.assertEqual(unfit, [{"id": "WI-02", "reason": "no row"}])
        self.assertIsNone(self.store.read_claim("WI-02"))
        self.assertEqual(self.claim_ids(), ["WI-01"])

    def test_lint_failures_go_to_unfit_and_release(self):
        self.select(top=3)
        self.write_row("WI-01", row_text("WI-01", kind="bug", entries="scripts/alpha.py:1-3", constraint=""))
        self.write_row("WI-02", row_text("WI-02", kind="feature", entries="scripts/beta.py:1-3",
                                         constraint="Something."))
        self.write_row("WI-03", row_text("WI-03", kind="bug", entries="scripts/gamma.py:9-2",
                                         constraint="Gamma."))
        fit, out = self.draft_collect()
        self.assertEqual(fit, [])
        self.assertIn("fit=0 unfit=3", out)
        unfit = {r["id"]: r["reason"] for r in fws.read_tsv(self.waves / W / "unfit.tsv")}
        self.assertEqual(unfit["WI-01"], "lint: constraint empty")
        self.assertEqual(unfit["WI-02"], "lint: kind 'feature' is not bug|gap")
        self.assertEqual(unfit["WI-03"], "lint: entry 'scripts/gamma.py:9-2' has lo > hi")
        self.assertEqual(self.claim_ids(), [])

    def test_missing_candidates_is_error(self):
        with self.assertRaises(fws.SelectError):
            fws.draft_collect(W, root=self.root, waves_root=self.waves, claims_dir=self.claims)


class TestCli(unittest.TestCase):
    def test_select_top_over_cap_exits_2(self):
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            make_repo(base / "repo")
            err = io.StringIO()
            with contextlib.redirect_stderr(err):
                rc = fws.main(["select", "--wave", W, "--top", "13", "--repo-root", str(base / "repo"),
                               "--waves-root", str(base / "waves"), "--claims-dir", str(base / "claims")])
            self.assertEqual(rc, 2)
            self.assertIn("--top", err.getvalue())

    def test_draft_collect_without_candidates_exits_2(self):
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            make_repo(base / "repo")
            err = io.StringIO()
            with contextlib.redirect_stderr(err):
                rc = fws.main(["draft-collect", "--wave", W, "--repo-root", str(base / "repo"),
                               "--waves-root", str(base / "waves"), "--claims-dir", str(base / "claims")])
            self.assertEqual(rc, 2)
            self.assertIn("candidates not found", err.getvalue())


if __name__ == "__main__":
    unittest.main()
