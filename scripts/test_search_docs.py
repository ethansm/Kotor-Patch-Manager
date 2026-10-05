#!/usr/bin/env python3
"""test_search_docs.py - Unit test suite for search_docs.py"""

import array
import math
import sqlite3
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT / "scripts"))
import search_docs as sd  # noqa: E402


class TestSearchDocs(unittest.TestCase):
    def test_cosine_similarity(self):
        v1 = array.array("f", [1.0, 0.0, 0.0])
        v2 = [1.0, 0.0, 0.0]
        self.assertAlmostEqual(sd.cosine_similarity(v1, v2), 1.0, places=5)

        v_ortho = [0.0, 1.0, 0.0]
        self.assertAlmostEqual(sd.cosine_similarity(v1, v_ortho), 0.0, places=5)

        v_opp = [-1.0, 0.0, 0.0]
        self.assertAlmostEqual(sd.cosine_similarity(v1, v_opp), -1.0, places=5)

        v_zero1 = array.array("f", [0.0, 0.0, 0.0])
        v_zero2 = [0.0, 0.0, 0.0]
        self.assertEqual(sd.cosine_similarity(v_zero1, v_zero2), 0.0)

    def test_chunk_markdown(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            md_file = Path(tmpdir) / "sample.md"
            md_file.write_text(
                "# Title\n"
                "This is the introductory overview section with enough text to exceed thirty characters.\n\n"
                "## Section 1: Features\n"
                "Here are the core features of the system described in full detail.\n\n"
                "### Sub-feature A\n"
                "Too short\n"
                "### Sub-feature B\n"
                "Sub-feature B has plenty of descriptive documentation text exceeding thirty characters.\n"
            )
            chunks = sd.chunk_markdown(md_file)
            self.assertGreaterEqual(len(chunks), 2)
            headers = [c["header"] for c in chunks]
            self.assertIn("Title", headers)
            self.assertIn("Section 1: Features", headers)
            self.assertIn("Sub-feature B", headers)

    def test_init_db(self):
        con = sqlite3.connect(":memory:")
        sd.init_db(con)
        tables = [row[0] for row in con.execute("SELECT name FROM sqlite_master WHERE type='table'").fetchall()]
        self.assertIn("doc_chunks", tables)
        self.assertIn("doc_fts", tables)

    def test_fts5_fallback_search(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp_path = Path(tmpdir)
            doc = tmp_path / "test_doc.md"
            doc.write_text(
                "# Detour Hook Architecture\n"
                "The DetourHookManager installs trampoline byte hooks directly into the target binary image.\n"
            )

            con = sqlite3.connect(":memory:")
            sd.init_db(con)

            # Ensure get_embeddings_batch returns None to test FTS5 fallback
            with patch.object(sd, "get_embeddings_batch", return_value=None):
                sd.index_docs(con, [tmp_path])

            # Search with get_embedding returning None -> FTS5 fallback mode
            with patch.object(sd, "get_embedding", return_value=None):
                results = sd.search(con, "DetourHookManager", limit=3)
                self.assertGreaterEqual(len(results), 1)
                self.assertEqual(results[0]["mode"], "fts5_fallback")
                self.assertIn("Detour Hook Architecture", results[0]["header"])

    def test_vector_search(self):
        con = sqlite3.connect(":memory:")
        sd.init_db(con)

        # Mock embeddings: 3 dimensions
        emb_doc = [1.0, 0.0, 0.0]
        emb_query = [0.95, 0.05, 0.0]

        chunks = [("doc1.md", "Vector Header", "Some lengthy content that describes vector mechanics in detail.", 1000.0)]
        with patch.object(sd, "get_embeddings_batch", return_value=[emb_doc]):
            sd._flush_chunks(con, chunks)

        with patch.object(sd, "get_embedding", return_value=emb_query):
            results = sd.search(con, "vector mechanics", limit=1)
            self.assertEqual(len(results), 1)
            self.assertEqual(results[0]["mode"], "vector")
            self.assertGreater(results[0]["score"], 0.9)
            self.assertEqual(results[0]["header"], "Vector Header")


if __name__ == "__main__":
    unittest.main()
