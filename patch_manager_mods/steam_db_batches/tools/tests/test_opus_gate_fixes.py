"""F1 (decoys must never be the true function) and F2 (`batch.py opus-parts`) tests.  Reuses test_batch fixtures."""
import json
import random
import sys
import unittest
from pathlib import Path
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_batch import B1_SPEC, GOG_BASE, Base, batch, bd, h, run, write_candidates, write_reviews_and_overrides  # noqa: E402


class ProofSourceTests(unittest.TestCase):
    def test_is_proof_source(self):
        for t in ("cmdtable", "cmdtable_overwritten", "server", "recheck", "name_map:high:steam-db-B1",
                  "name_map:high:gog_cluster_delta_map.py_x"):
            self.assertTrue(batch.is_proof_source(t), t)
        for t in ("near", "mapper:vote", "branch", "name_map:medium:x", "name_map:high:batch3_foo",
                  "name_map:high:gog_foo_cluster", "name_map:high:gog_cluster_delta_map.py", "name_map:high", "card"):
            self.assertFalse(batch.is_proof_source(t), t)

    def test_proof_candidates_of_a_row(self):
        ev = {"cand_sources": "0x00510000|near+cmdtable;0x00510010|name_map:high:steam-db-B1;"
                              "0x00510020|name_map:high:batch2_x;0x00510030|near"}
        self.assertEqual(batch.proof_candidates(ev), {0x510000, 0x510010})
        self.assertEqual(batch.proof_candidates({}), set())


class DecoyPickerTests(unittest.TestCase):
    """Fake corpus: Steam starts 0x1000..0x1050 step 0x10, all retn 8, so every host has prev and next neighbours."""

    def corpus(self):
        starts = [0x1000 + 0x10 * i for i in range(6)]
        return SimpleNamespace(steam={a: SimpleNamespace(retn=8) for a in starts}, steam_starts=starts)

    def pick(self, cand_sources):
        host = {"gog": "0x00400000", "steam_addr": "0x00001020", "cand_sources": cand_sources}
        out = batch.pick_decoy_cards(self.corpus(), [host], 10, random.Random(1), set())
        return sorted(a for _k, _e, a in out)

    def test_neighbour_that_is_a_cmdtable_or_trusted_name_map_candidate_is_skipped(self):
        self.assertEqual(self.pick("0x00001020|mapper:vote;0x00001010|cmdtable;0x00001030|near"), [0x1030])
        self.assertEqual(self.pick("0x00001020|mapper:vote;0x00001030|name_map:high:steam-db-B1"), [0x1010])
        self.assertEqual(self.pick("0x00001020|x;0x00001010|server;0x00001030|recheck"), [])

    def test_near_untrusted_and_medium_candidates_are_still_picked(self):
        self.assertEqual(self.pick("0x00001020|mapper:vote;0x00001010|near;0x00001030|name_map:medium:x"), [0x1010, 0x1030])
        self.assertEqual(self.pick("0x00001020|x;0x00001010|name_map:high:batch3_q;0x00001030|name_map:high:gog_a_cluster"),
                         [0x1010, 0x1030])

    def test_deterministic(self):
        spec = "0x00001020|x;0x00001010|cmdtable"
        self.assertEqual(self.pick(spec), self.pick(spec))


class KeyErrorTests(Base):
    """An accepted decoy equal to an Opus-shipped override pair is a key error, not a decoy accept."""

    def setUp(self):
        super().setUp()
        write_candidates(self.root, "B1", B1_SPEC)
        run("gates", "B1")
        write_reviews_and_overrides(self.root)
        self.bdir = self.root / "batches" / "B1"
        # Theta (gog +0x2c0 -> 0x005102c0) is accepted by the haiku reviewer; make it a decoy card in the key
        batch.write_tsv(self.bdir / "cards" / "key.tsv", ["card_id", "chunk", "gog", "steam_addr", "kind", "expected"],
                        [{"card_id": "B1-1-01", "chunk": 1, "gog": h(GOG_BASE + 0x2C0), "steam_addr": "0x005102c0",
                          "kind": "decoy_prev", "expected": "mismatch"}])

    def add_override(self, steam, verdict):
        with open(self.bdir / "overrides.tsv", "a") as f:
            f.write("%s\t%s\t%s\tOpus: test\n" % (h(GOG_BASE + 0x2C0), steam, verdict))

    def gate(self):
        rc, out = run("finalize", "B1", "--date", "2026-10-03", "--opus-sample-errors", "0")
        self.assertEqual(rc, 0, out)
        return json.loads((self.bdir / "opus_gate.json").read_text()), (self.bdir / "opus_gate.md").read_text(), out

    def test_override_equal_to_accepted_decoy_is_a_key_error(self):
        self.add_override("0x005102c0", "verified")
        g, md, out = self.gate()
        self.assertEqual((g["key_errors"], g["key_error_cards"], g["decoys_accepted"]), (1, ["B1-1-01"], 0))
        self.assertFalse(g["decision"].startswith("ESCALATE"), g["decision"])
        sect = md.split("## Decoy key errors (decoy = Opus-shipped counterpart)")[1].split("## ")[0]
        self.assertIn("B1-1-01", sect)
        acc = md.split("## Decoys accepted (0 tolerance)")[1].split("## ")[0]
        self.assertNotIn("B1-1-01", acc)
        self.assertLess(md.index("## Decoy key errors"), md.index("## Decoys accepted"))
        self.assertIn("accepted 0, key errors 1", out)

    def test_likely_override_also_counts(self):
        self.add_override("0x005102c0", "likely")
        self.assertEqual(self.gate()[0]["key_errors"], 1)

    def test_accepted_decoy_without_matching_override_still_escalates(self):
        g, md, _ = self.gate()
        self.assertEqual((g["key_errors"], g["decoys_accepted"]), (0, 1))
        self.assertTrue(g["decision"].startswith("ESCALATE"), g["decision"])
        self.assertIn("B1-1-01", md.split("## Decoys accepted (0 tolerance)")[1].split("## ")[0])

    def test_non_matching_override_does_not_excuse_the_decoy(self):
        self.add_override("0x005102c0", "rejected")        # not shipped
        self.assertEqual(self.gate()[0]["decoys_accepted"], 1)
        self.add_override("0x00510300", "verified")        # other steam address (last override row wins per gog)
        g = self.gate()[0]
        self.assertEqual((g["key_errors"], g["decoys_accepted"]), (0, 1))
        self.assertTrue(g["decision"].startswith("ESCALATE"))

    def test_decoy_gate_is_not_fed_key_errors(self):
        self.add_override("0x005102c0", "verified")
        seen = {}
        real = bd.decoy_gate

        def spy(chunks, reviewed, decoys, *a, **kw):
            seen.update(decoys)
            return real(chunks, reviewed, decoys, *a, **kw)
        orig = bd.decoy_gate
        bd.decoy_gate = spy
        try:
            self.gate()
        finally:
            bd.decoy_gate = orig
        self.assertEqual(seen, {"1": [False]})


def rows_json(n_full, n_sample, n_auto):
    def r(i, **extra):
        d = {"class": "C", "name": "F%d" % i, "gog": h(0x400000 + 0x10 * i), "steam_addr": h(0x500000 + 0x10 * i)}
        d.update(extra)
        return d
    i = 0
    full, sample, auto = [], [], []
    for _ in range(n_full):
        full.append(r(i, why="label_suspect")); i += 1
    for _ in range(n_sample):
        sample.append(r(i, content_level="C0")); i += 1
    for k in range(n_auto):
        auto.append(r(i, route="sig" if k < 5 else "c2")); i += 1
    return {"batch": "B9", "rows_full": full, "rows_sample": sample, "rows_auto": auto}


class OpusPartsTests(Base):
    def write_gate(self, **kw):
        bdir = self.root / "batches" / "B9"
        bdir.mkdir(parents=True, exist_ok=True)
        data = rows_json(**kw)
        (bdir / "opus_gate.json").write_text(json.dumps(data))
        return bdir, data

    def parts(self, bdir):
        return json.loads((bdir / "opus" / "parts.json").read_text())

    def test_split_is_lossless_ordered_and_deterministic(self):
        bdir, data = self.write_gate(n_full=7, n_sample=12, n_auto=8)
        want = [r["gog"] for k in ("rows_full", "rows_sample", "rows_auto") for r in data[k]]
        rc, out = run("opus-parts", "B9", "--rows", "10")
        self.assertEqual(rc, 0, out)
        pj = self.parts(bdir)
        self.assertEqual((pj["batch"], pj["rows_per_part"], pj["total_rows"]), ("B9", 10, 27))
        self.assertEqual([p["part"] for p in pj["parts"]], ["a", "b", "c"])
        self.assertEqual([p["n_rows"] for p in pj["parts"]], [10, 10, 7])
        flat = [g for p in pj["parts"] for g in p["gog_addrs"]]
        self.assertEqual(flat, want)                       # every row exactly once, original order
        self.assertEqual(len(set(flat)), len(flat))
        for p in pj["parts"]:
            text = (bdir / p["file"]).read_text()
            self.assertEqual(sum(1 for line in text.splitlines() if line.startswith("| ") and "0x004" in line), p["n_rows"])
            self.assertIn("Decide every row listed here; output per OPUS_GATE_BRIEF.md", text)
            self.assertIn("B9", text.splitlines()[0])
        first = (bdir / "opus" / "part_a.md").read_text()
        self.assertIn("| 1 | 100% | C::F0 |", first)
        self.assertIn("label_suspect", first)
        last = (bdir / "opus" / "part_c.md").read_text()
        self.assertIn("AUTO sig", (bdir / "opus" / "part_b.md").read_text() + last)
        self.assertIn("AUTO c2", last)
        again = {p.name: p.read_text() for p in (bdir / "opus").glob("part_*.md")}
        run("opus-parts", "B9", "--rows", "10")
        self.assertEqual(again, {p.name: p.read_text() for p in (bdir / "opus").glob("part_*.md")})

    def test_rows_option_default_and_stale_parts_removed(self):
        bdir, _ = self.write_gate(n_full=3, n_sample=4, n_auto=0)
        run("opus-parts", "B9", "--rows", "2")
        self.assertEqual([p["n_rows"] for p in self.parts(bdir)["parts"]], [2, 2, 2, 1])
        self.assertEqual(len(list((bdir / "opus").glob("part_*.md"))), 4)
        run("opus-parts", "B9")                            # default 100 -> one part, stale files gone
        self.assertEqual(self.parts(bdir)["rows_per_part"], 100)
        self.assertEqual([p.name for p in (bdir / "opus").glob("part_*.md")], ["part_a.md"])

    def test_part_letters_roll_over_after_z(self):
        self.assertEqual(batch.part_letters(28)[24:], ["y", "z", "aa", "ab"])

    def test_missing_json_or_missing_row_lists_exit_nonzero(self):
        with self.assertRaises(SystemExit) as cm:
            run("opus-parts", "B9")
        self.assertNotEqual(cm.exception.code, 0)
        bdir = self.root / "batches" / "B9"
        bdir.mkdir(parents=True)
        (bdir / "opus_gate.json").write_text(json.dumps({"batch": "B9", "full_read": 3}))
        with self.assertRaises(SystemExit) as cm:
            run("opus-parts", "B9")
        self.assertNotEqual(cm.exception.code, 0)

    def test_finalize_writes_row_lists_matching_the_md(self):
        write_candidates(self.root, "B1", B1_SPEC)
        run("gates", "B1")
        write_reviews_and_overrides(self.root)
        run("finalize", "B1", "--date", "2026-10-03")
        bdir = self.root / "batches" / "B1"
        g = json.loads((bdir / "opus_gate.json").read_text())
        md = (bdir / "opus_gate.md").read_text()
        self.assertEqual(len(g["rows_full"]), g["full_read"])
        self.assertEqual(len(g["rows_sample"]), g["sample_size"])
        for r in g["rows_full"] + g["rows_sample"] + g["rows_auto"]:
            self.assertIn("%s::%s gog %s -> %s" % (r["class"], r["name"], r["gog"], r["steam_addr"]), md)
        self.assertTrue(g["rows_full"] and g["rows_auto"])
        self.assertEqual(run("opus-parts", "B1", "--rows", "3")[0], 0)
        pj = self.parts(bdir)
        self.assertEqual(pj["total_rows"], len(g["rows_full"]) + len(g["rows_sample"]) + len(g["rows_auto"]))


if __name__ == "__main__":
    unittest.main()
