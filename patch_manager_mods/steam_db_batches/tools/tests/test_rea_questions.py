"""Unit tests for steam_db_batches/tools/rea_questions.py. Every fixture lives in a temp dir.

Run:  cd <repo>/patch_manager_mods/steam_db_batches/tools && python3 -m unittest tests.test_rea_questions
"""
import contextlib
import hashlib
import io
import re
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))
import rea_questions as rq  # noqa: E402

BATCH = "B13a"
KEY_HEADER = "card_id\tchunk\tgog\tsteam_addr\tkind\texpected\n"
LEDGER_HEADER = "addr\tkind\tname\tconfidence\tevidence\ttopic\tdate\n"
BULLET_RE = re.compile(r"^- 0x[0-9a-f]{8}$", re.M)


def key_row(card_id, gog, steam, kind="real", chunk="1"):
    return f"{card_id}\t{chunk}\t{gog}\t{steam}\t{kind}\t\n"


def ledger_row(addr, confidence, topic, kind="function"):
    return f"{addr}\t{kind}\tsome behaviour\t{confidence}\tprocedure_assembly {addr}: RET\t{topic}\t2026-10-15\n"


class QuestionFixture(unittest.TestCase):
    def setUp(self):
        self._td = tempfile.TemporaryDirectory()
        self.root = Path(self._td.name)
        self.batches = self.root / "batches"
        self.batch_dir = self.batches / BATCH
        (self.batch_dir / "cards").mkdir(parents=True)
        self.ledger = self.root / "ledger"
        self.ledger.mkdir()
        self.out = self.root / "out"

    def tearDown(self):
        self._td.cleanup()

    def write_key(self, name, rows):
        path = self.batch_dir / "cards" / name
        path.write_text(KEY_HEADER + "".join(rows), encoding="utf-8")
        return path

    def write_ledger(self, name, rows):
        (self.ledger / name).write_text(LEDGER_HEADER + "".join(rows), encoding="utf-8")

    def run_cli(self, *args):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = rq.main(list(args))
        return rc, out.getvalue(), err.getvalue()

    def cli(self, *extra, out=None, checked=False):
        """Run the CLI with the exe check skipped, unless checked=True (then pass --exe in extra)."""
        target = self.out if out is None else out
        args = [BATCH, "--batches-dir", str(self.batches), "--ledger-dir", str(self.ledger),
                "--out", str(target), *extra]
        if not checked:
            args.append("--skip-exe-check")
        return self.run_cli(*args)

    def question_files(self, out=None):
        target = self.out if out is None else out
        return sorted(target.glob("q_*.md"))

    def bullets(self, path):
        return BULLET_RE.findall(path.read_text(encoding="utf-8"))


class DedupeTests(QuestionFixture):
    def test_same_steam_addr_on_two_cards_is_one_entry(self):
        self.write_key("key.tsv", [
            key_row("B13a-1-01", "0x00410000", "0x00500000"),
            key_row("B13a-1-02", "0x00410010", "0x00500000", kind="decoy_prev"),
        ])
        rc, out, _ = self.cli()
        self.assertEqual(rc, 0)
        self.assertIn("1 addresses (0 skipped, ledger) -> 1 files", out)
        files = self.question_files()
        self.assertEqual(len(files), 1)
        self.assertEqual(self.bullets(files[0]), ["- 0x00500000"])


class BlindnessTests(QuestionFixture):
    def test_output_carries_no_gog_address_card_id_or_kind_label(self):
        gog_addrs = ["0x00410000", "0x00410010", "0x00410020"]
        self.write_key("key.tsv", [
            key_row("B13a-1-01", gog_addrs[0], "0x00500000", kind="real"),
            key_row("B13a-1-02", gog_addrs[1], "0x00500010", kind="decoy_prev"),
            key_row("B13a-1-03", gog_addrs[2], "0x00500020", kind="decoy_next"),
        ])
        rc, _, _ = self.cli()
        self.assertEqual(rc, 0)
        files = self.question_files()
        self.assertEqual(len(files), 1)
        text = files[0].read_text(encoding="utf-8")
        lowered = text.lower()
        self.assertNotIn("decoy", lowered)
        self.assertNotIn("real", lowered)
        for gog in gog_addrs:
            self.assertNotIn(gog, text)
            self.assertNotIn(gog[2:], text)
        for card_id in ("B13a-1-01", "B13a-1-02", "B13a-1-03"):
            self.assertNotIn(card_id, text)

    def test_file_shape(self):
        self.write_key("key.tsv", [key_row("B13a-1-01", "0x00410000", "0x00500000")])
        self.cli()
        text = self.question_files()[0].read_text(encoding="utf-8")
        self.assertIn("# REA questions B13a-q1\n", text)
        self.assertIn(f"Binary: {rq.BINARY}\n", text)
        self.assertIn("Ledger topic: B13a-q1\n", text)
        self.assertIn("## Addresses", text)
        self.assertIn("- 0x00500000", text)
        for n in range(1, 7):
            self.assertIn(f"{n}. ", text)


class ChunkingTests(QuestionFixture):
    def test_seventeen_addresses_make_eight_eight_one(self):
        rows = [key_row(f"B13a-1-{i:02d}", f"0x{0x410000 + i * 16:08x}", f"0x{0x500000 + i * 16:08x}")
                for i in range(17)]
        self.write_key("key.tsv", rows)
        rc, out, _ = self.cli("--per-run", "8")
        self.assertEqual(rc, 0)
        self.assertIn("17 addresses (0 skipped, ledger) -> 3 files", out)
        files = self.question_files()
        self.assertEqual([p.name for p in files], ["q_1.md", "q_2.md", "q_3.md"])
        self.assertEqual([len(self.bullets(p)) for p in files], [8, 8, 1])
        all_bullets = [b for p in files for b in self.bullets(p)]
        self.assertEqual(len(set(all_bullets)), 17)

    def test_stale_question_files_are_removed(self):
        self.write_key("key.tsv", [key_row("B13a-1-01", "0x00410000", "0x00500000")])
        self.out.mkdir(parents=True)
        (self.out / "q_9.md").write_text("stale\n", encoding="utf-8")
        self.cli()
        self.assertFalse((self.out / "q_9.md").exists())
        self.assertEqual([p.name for p in self.question_files()], ["q_1.md"])


class DeterminismTests(QuestionFixture):
    def test_two_runs_write_identical_bytes(self):
        rows = [key_row(f"B13a-1-{i:02d}", f"0x{0x410000 + i * 16:08x}", f"0x{0x500000 + i * 16:08x}")
                for i in range(20)]
        self.write_key("key.tsv", rows)
        out1, out2 = self.root / "run1", self.root / "run2"
        self.assertEqual(self.cli(out=out1)[0], 0)
        self.assertEqual(self.cli(out=out2)[0], 0)
        names1 = [p.name for p in self.question_files(out1)]
        self.assertEqual(names1, [p.name for p in self.question_files(out2)])
        self.assertTrue(names1)
        for name in names1:
            self.assertEqual((out1 / name).read_bytes(), (out2 / name).read_bytes())


class LedgerSkipTests(QuestionFixture):
    def test_only_confirmed_same_batch_q_topic_rows_skip(self):
        self.write_key("key.tsv", [
            key_row("B13a-1-01", "0x00410000", "0x00500000"),  # confirmed, B13a-q1 -> skipped
            key_row("B13a-1-02", "0x00410010", "0x00500010"),  # probable, B13a-q1 -> kept
            key_row("B13a-1-03", "0x00410020", "0x00500020"),  # confirmed, other batch topic -> kept
            key_row("B13a-1-04", "0x00410030", "0x00500030"),  # confirmed, B13a-x not -q -> kept
        ])
        self.write_ledger("20261015_test.tsv", [
            ledger_row("0x00500000", "confirmed", "B13a-q1"),
            ledger_row("0x00500010", "probable", "B13a-q1"),
            ledger_row("0x00500020", "confirmed", "B14-q1"),
            ledger_row("0x00500030", "confirmed", "B13a-x"),
        ])
        rc, out, _ = self.cli()
        self.assertEqual(rc, 0)
        self.assertIn("4 addresses (1 skipped, ledger) -> 1 files", out)
        text = self.question_files()[0].read_text(encoding="utf-8")
        self.assertNotIn("- 0x00500000", text)
        for addr in ("0x00500010", "0x00500020", "0x00500030"):
            self.assertIn(f"- {addr}", text)


class TopicNumberingTests(QuestionFixture):
    def test_rerun_continues_after_highest_ledger_topic(self):
        self.write_key("key.tsv", [key_row("B13a-1-%02d" % i, "0x%08x" % (0x410000 + 16 * i),
                                           "0x%08x" % (0x500000 + 16 * i)) for i in range(10)])
        self.write_ledger("20261015_test.tsv", [ledger_row("0x00500000", "confirmed", "B13a-q1"),
                                                ledger_row("0x00600000", "probable", "B13a-q3")])
        rc, _, _ = self.cli()
        self.assertEqual(rc, 0)
        self.assertEqual([p.name for p in self.question_files()], ["q_4.md", "q_5.md"])
        self.assertIn("Ledger topic: B13a-q4", self.question_files()[0].read_text(encoding="utf-8"))


class KeysTests(QuestionFixture):
    def test_keys_flag_restricts_input_to_given_files(self):
        pilot = self.write_key("key_pilot.tsv", [key_row("B13a-1-01", "0x00410000", "0x00500000")])
        self.write_key("key.tsv", [
            key_row("B13a-1-02", "0x00410010", "0x00500010"),
            key_row("B13a-1-03", "0x00410020", "0x00500020"),
        ])
        rc, out, _ = self.cli()
        self.assertEqual(rc, 0)
        self.assertIn("3 addresses", out)
        rc, out, _ = self.cli("--keys", str(pilot))
        self.assertEqual(rc, 0)
        self.assertIn("1 addresses (0 skipped, ledger) -> 1 files", out)
        self.assertEqual(self.bullets(self.question_files()[0]), ["- 0x00500000"])


class ErrorTests(QuestionFixture):
    def test_batch_without_key_files_is_usage_error(self):
        rc, _, err = self.cli()
        self.assertEqual(rc, 2)
        self.assertIn("no key files", err)

    def test_bad_steam_addr_is_data_error(self):
        self.write_key("key.tsv", [key_row("B13a-1-01", "0x00410000", "not-hex")])
        rc, _, err = self.cli()
        self.assertEqual(rc, 2)
        self.assertIn("bad steam_addr", err)


class ExeGuardTests(QuestionFixture):
    def setUp(self):
        super().setUp()
        self.write_key("key.tsv", [key_row("B13a-1-01", "0x00410000", "0x00500000")])

    def write_exe(self, rel, data=b"not the vanilla exe\n"):
        path = self.root / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        return path

    def test_wrong_digest_is_refused_and_nothing_is_written(self):
        exe = self.write_exe("swkotor2.exe")
        rc, _, err = self.cli("--exe", str(exe), checked=True)
        self.assertEqual(rc, 2)
        self.assertIn("sha256", err)
        self.assertEqual(self.question_files(), [])
        self.assertFalse((self.out / "q_manifest.tsv").exists())
        self.assertFalse(self.out.exists())

    def test_multi_chunk_file_is_hashed_in_full(self):
        exe = self.write_exe("big/swkotor2.exe", b"x" * ((1 << 20) * 2 + 17))
        digest = hashlib.sha256(exe.read_bytes()).hexdigest()
        rq.preflight_exe(exe, sha=digest)
        with self.assertRaises(rq.DataError):
            rq.preflight_exe(exe, sha="0" * 64)

    def test_steamapps_path_is_refused_even_with_right_digest(self):
        exe = self.write_exe("steamapps/swkotor2.exe")
        digest = hashlib.sha256(exe.read_bytes()).hexdigest()
        with mock.patch.object(rq, "EXE_SHA256", digest):
            rc, _, err = self.cli("--exe", str(exe), checked=True)
        self.assertEqual(rc, 2)
        self.assertIn("steamapps", err)
        self.assertEqual(self.question_files(), [])

    def test_missing_exe_is_refused(self):
        rc, _, err = self.cli("--exe", str(self.root / "missing.exe"), checked=True)
        self.assertEqual(rc, 2)
        self.assertIn("exe not found", err)
        self.assertEqual(self.question_files(), [])

    def test_matching_digest_passes(self):
        exe = self.write_exe("vanilla/swkotor2.exe", b"vanilla stand-in\n")
        digest = hashlib.sha256(exe.read_bytes()).hexdigest()
        with mock.patch.object(rq, "EXE_SHA256", digest):
            rc, _, _ = self.cli("--exe", str(exe), checked=True)
        self.assertEqual(rc, 0)
        self.assertEqual(len(self.question_files()), 1)

    def test_running_marker_refuses_and_keeps_existing_files(self):
        self.out.mkdir(parents=True)
        keep = self.out / "q_1.md"
        keep.write_text("keep me\n", encoding="utf-8")
        marker = self.batch_dir / "rea" / "RUNNING"
        marker.parent.mkdir(parents=True)
        marker.write_text("", encoding="utf-8")
        rc, _, err = self.cli()
        self.assertEqual(rc, 2)
        self.assertIn("rea-recon agents are running", err)
        self.assertEqual(keep.read_text(encoding="utf-8"), "keep me\n")
        self.assertEqual([p.name for p in self.question_files()], ["q_1.md"])
        self.assertFalse((self.out / "q_manifest.tsv").exists())


class RunningMarkerElsewhereTests(QuestionFixture):
    def test_marker_blocks_even_when_out_points_elsewhere(self):
        self.write_key("key.tsv", [key_row("B13a-1-01", "0x00410000", "0x00500000")])
        marker = self.batch_dir / "rea" / "RUNNING"
        marker.parent.mkdir(parents=True)
        marker.write_text("", encoding="utf-8")
        other = self.root / "elsewhere"
        rc, _, err = self.cli(out=other)
        self.assertEqual(rc, 2)
        self.assertIn("rea-recon agents are running", err)
        self.assertFalse(other.exists())


class ManifestTests(QuestionFixture):
    def setUp(self):
        super().setUp()
        self.write_key("key.tsv", [
            key_row(f"B13a-1-{i:02d}", f"0x{0x410000 + i * 16:08x}", f"0x{0x500000 + i * 16:08x}",
                    kind="real" if i % 2 == 0 else "decoy_prev")
            for i in range(20)
        ])

    def test_two_runs_write_byte_identical_manifest(self):
        out1, out2 = self.root / "run1", self.root / "run2"
        self.assertEqual(self.cli(out=out1)[0], 0)
        self.assertEqual(self.cli(out=out2)[0], 0)
        self.assertEqual((out1 / "q_manifest.tsv").read_bytes(), (out2 / "q_manifest.tsv").read_bytes())

    def test_manifest_rows_match_question_files(self):
        self.assertEqual(self.cli()[0], 0)
        lines = (self.out / "q_manifest.tsv").read_text(encoding="utf-8").splitlines()
        self.assertEqual(lines[0], "topic\tn_addr\tsha_of_list")
        rows = [line.split("\t") for line in lines[1:]]
        self.assertEqual(len(rows), len(self.question_files()))
        self.assertEqual(sum(int(row[1]) for row in rows), 20)
        for k, row in enumerate(rows, start=1):
            bullets = [b[len("- "):] for b in self.bullets(self.out / f"q_{k}.md")]
            self.assertEqual(row[0], f"B13a-q{k}")
            self.assertEqual(int(row[1]), len(bullets))
            self.assertEqual(row[2], hashlib.sha1(("\n".join(bullets) + "\n").encode("utf-8")).hexdigest())

    def test_stale_manifest_is_replaced(self):
        self.out.mkdir(parents=True)
        (self.out / "q_manifest.tsv").write_text("stale\n", encoding="utf-8")
        self.assertEqual(self.cli()[0], 0)
        text = (self.out / "q_manifest.tsv").read_text(encoding="utf-8")
        self.assertNotIn("stale", text)
        self.assertEqual(len(text.splitlines()), 1 + len(self.question_files()))


class CompositionLineTests(QuestionFixture):
    def test_addresses_line_follows_ledger_line_and_no_kind_words(self):
        self.write_key("key.tsv", [
            key_row("B13a-1-01", "0x00410000", "0x00500000", kind="real"),
            key_row("B13a-1-02", "0x00410010", "0x00500010", kind="decoy_prev"),
            key_row("B13a-1-03", "0x00410020", "0x00500020", kind="decoy_next"),
        ])
        self.assertEqual(self.cli()[0], 0)
        text = self.question_files()[0].read_text(encoding="utf-8")
        lines = text.splitlines()
        i = next(n for n, line in enumerate(lines) if line.startswith("Ledger file: "))
        self.assertEqual(lines[i + 1], "addresses: 3")
        lowered = text.lower()
        self.assertNotIn("real", lowered)
        self.assertNotIn("decoy", lowered)


RESIDUAL_Q1 = "1. Semantic purpose in at most 15 words, with one quoted asm line `0x<addr>: <instr>` as the anchor."
RESIDUAL_Q2 = ("2. Anything the scripted facts could not resolve (indirect call targets, non-contiguous body, "
               "data xrefs), or \"none\".")
RESIDUAL_RULE_7 = ("7. One row per address with kind = function (the purpose, evidence = the quoted asm line). "
                   "Budget: at most 3 REA calls per address, plus 3 spare for the whole file. "
                   "Do not re-ask static facts (size, RET, callers, callees, strings).")
FIXED_QUESTION_TEXTS = ("Function start?", "Size in bytes", "Callers: total count", "Callees: their addresses",
                        "Strings pushed or referenced", "Vtable or data xref users")


class ResidualModeTests(QuestionFixture):
    def setUp(self):
        super().setUp()
        self.write_key("key.tsv", [
            key_row("B13a-1-01", "0x00410000", "0x00500000", kind="real"),
            key_row("B13a-1-02", "0x00410010", "0x00500010", kind="decoy_prev"),
            key_row("B13a-1-03", "0x00410020", "0x00500020", kind="decoy_next"),
        ])

    def read_single(self):
        files = self.question_files()
        self.assertEqual(len(files), 1)
        return files[0].read_text(encoding="utf-8")

    def test_residual_file_asks_only_the_two_residual_questions(self):
        rc, _, _ = self.cli("--residual")
        self.assertEqual(rc, 0)
        text = self.read_single()
        self.assertIn("## Questions (answer both for every address)\n\n" + RESIDUAL_Q1 + "\n" + RESIDUAL_Q2 + "\n", text)
        for fixed in FIXED_QUESTION_TEXTS:
            self.assertNotIn(fixed, text)
        self.assertNotIn("answer all six", text)

    def test_residual_asks_exactly_two_numbered_questions_and_no_fixed_one(self):
        self.assertEqual(self.cli("--residual")[0], 0)
        text = self.read_single()
        section = text.split("## Questions", 1)[1].split("\n## ", 1)[0]
        numbered = re.findall(r"^\d+\. ", section, re.M)
        self.assertEqual(numbered, ["1. ", "2. "])
        self.assertEqual(len(numbered), 2)
        self.assertIn("at most 15 words", section)
        self.assertIn("quoted asm line", section)
        self.assertIn("could not resolve", section)
        for fixed in FIXED_QUESTION_TEXTS:
            self.assertNotIn(fixed, text)

    def test_residual_budget_is_three_per_address_plus_three_spare(self):
        self.assertEqual(self.cli("--residual")[0], 0)
        text = self.read_single()
        rule7 = next(line for line in text.splitlines() if line.startswith("7. "))
        self.assertIn("at most 3 REA calls per address", rule7)
        self.assertIn("plus 3 spare for the whole file", rule7)
        self.assertNotIn("at most 4", rule7)
        self.assertEqual(rq.RULE_7_RESIDUAL, RESIDUAL_RULE_7)

    def test_residual_rule_7_replaces_xref_row_rule(self):
        self.cli("--residual")
        text = self.read_single()
        self.assertIn(RESIDUAL_RULE_7 + "\n", text)
        self.assertNotIn("kind = xref", text)
        self.assertNotIn("at most 4 REA calls", text)

    def test_normal_mode_still_asks_all_six(self):
        self.cli()
        text = self.read_single()
        self.assertIn("## Questions (answer all six for every address)", text)
        for fixed in FIXED_QUESTION_TEXTS:
            self.assertIn(fixed, text)
        self.assertIn("kind = xref", text)
        self.assertNotIn(RESIDUAL_RULE_7, text)
        self.assertNotIn(RESIDUAL_Q1, text)

    def test_residual_mode_writes_manifest_matching_file(self):
        self.assertEqual(self.cli("--residual")[0], 0)
        lines = (self.out / "q_manifest.tsv").read_text(encoding="utf-8").splitlines()
        self.assertEqual(lines[0], "topic\tn_addr\tsha_of_list")
        self.assertEqual(len(lines), 1 + len(self.question_files()))
        topic, n_addr, sha = lines[1].split("\t")
        bullets = [b[len("- "):] for b in self.bullets(self.out / "q_1.md")]
        self.assertEqual(topic, "B13a-q1")
        self.assertEqual(int(n_addr), len(bullets))
        self.assertEqual(sha, hashlib.sha1(("\n".join(bullets) + "\n").encode("utf-8")).hexdigest())

    def test_residual_output_is_blind(self):
        self.cli("--residual")
        text = self.read_single()
        lowered = text.lower()
        self.assertNotIn("real", lowered)
        self.assertNotIn("decoy", lowered)
        for gog in ("0x00410000", "0x00410010", "0x00410020"):
            self.assertNotIn(gog, text)
        for card_id in ("B13a-1-01", "B13a-1-02", "B13a-1-03"):
            self.assertNotIn(card_id, text)

    def test_residual_topics_continue_after_highest_ledger_topic(self):
        self.write_key("key.tsv", [key_row("B13a-1-%02d" % i, "0x%08x" % (0x410000 + 16 * i),
                                           "0x%08x" % (0x500000 + 16 * i)) for i in range(10)])
        self.write_ledger("20261015_test.tsv", [ledger_row("0x00600000", "probable", "B13a-q3")])
        rc, _, _ = self.cli("--residual")
        self.assertEqual(rc, 0)
        self.assertEqual([p.name for p in self.question_files()], ["q_4.md", "q_5.md"])
        self.assertIn("Ledger topic: B13a-q4", self.question_files()[0].read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
