#!/usr/bin/env python3
"""search_docs.py - Fast semantic & hybrid search for project documentation

Indexes Markdown files using local LM Studio embeddings (nomic-embed-text)
with SQLite caching and automatic FTS5 full-text search fallback.

Usage:
  python3 scripts/search_docs.py index [dirs...]
  python3 scripts/search_docs.py "query string" [--limit 3] [--json]
"""

import argparse
import array
import json
import math
import os
import re
import sqlite3
import sys
import urllib.error
import urllib.request
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DB_PATH = REPO_ROOT / ".doc_search_cache.db"
EMBED_URL = "http://127.0.0.1:1234/v1/embeddings"
MODEL_NAME = "text-embedding-nomic-embed-text-v1.5"

HEADER_RE = re.compile(r'^(#{1,4})\s+(.+)$', re.MULTILINE)

def get_embeddings_batch(texts: list[str]) -> list[list[float]] | None:
    if not texts:
        return []
    clean_texts = [t[:1500] for t in texts]
    payload = json.dumps({"model": MODEL_NAME, "input": clean_texts}).encode("utf-8")
    req = urllib.request.Request(EMBED_URL, data=payload, headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=15) as resp:
            data = json.loads(resp.read().decode("utf-8"))
            return [item["embedding"] for item in data["data"]]
    except Exception:
        return None

def get_embedding(text: str) -> list[float] | None:
    res = get_embeddings_batch([text])
    return res[0] if res else None

def init_db(con: sqlite3.Connection):
    cur = con.cursor()
    cur.execute("""
        CREATE TABLE IF NOT EXISTS doc_chunks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            file_path TEXT NOT NULL,
            header TEXT,
            content TEXT NOT NULL,
            mtime REAL NOT NULL,
            embedding BLOB
        )
    """)
    cur.execute("""
        CREATE VIRTUAL TABLE IF NOT EXISTS doc_fts USING fts5(
            file_path, header, content, content=doc_chunks, content_rowid=id
        )
    """)
    cur.execute("CREATE INDEX IF NOT EXISTS idx_doc_chunks_file ON doc_chunks(file_path)")
    con.commit()

def chunk_markdown(file_path: Path) -> list[dict]:
    try:
        text = file_path.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return []

    lines = text.split("\n")
    chunks = []
    current_header = "Overview"
    current_lines = []

    for line in lines:
        m = HEADER_RE.match(line)
        if m:
            if current_lines:
                body = "\n".join(current_lines).strip()
                if len(body) > 30:
                    chunks.append({"header": current_header, "content": body})
                current_lines = []
            current_header = m.group(2).strip()
        else:
            current_lines.append(line)

    if current_lines:
        body = "\n".join(current_lines).strip()
        if len(body) > 30:
            chunks.append({"header": current_header, "content": body})

    return chunks

def index_docs(con: sqlite3.Connection, target_dirs: list[Path], batch_size: int = 32):
    cur = con.cursor()
    md_files = []
    for d in target_dirs:
        if d.is_file() and d.suffix.lower() == ".md":
            md_files.append(d)
        elif d.is_dir():
            for p in d.rglob("*.md"):
                if not any(part.startswith(".") for part in p.parts):
                    md_files.append(p)

    indexed = 0
    skipped = 0
    print(f"Scanning {len(md_files)} markdown files...")

    pending_chunks = []

    for f in md_files:
        try:
            rel_p = str(f.relative_to(REPO_ROOT))
        except ValueError:
            rel_p = str(f)
        mtime = f.stat().st_mtime

        row = cur.execute("SELECT mtime FROM doc_chunks WHERE file_path = ? LIMIT 1", (rel_p,)).fetchone()
        if row and abs(row[0] - mtime) < 0.01:
            skipped += 1
            continue

        cur.execute("DELETE FROM doc_chunks WHERE file_path = ?", (rel_p,))
        cur.execute("DELETE FROM doc_fts WHERE file_path = ?", (rel_p,))
        chunks = chunk_markdown(f)
        for c in chunks:
            pending_chunks.append((rel_p, c["header"], c["content"], mtime))
        indexed += 1

        # Process in batches
        if len(pending_chunks) >= batch_size:
            _flush_chunks(con, pending_chunks)
            pending_chunks = []

    if pending_chunks:
        _flush_chunks(con, pending_chunks)

    con.commit()
    print(f"Indexing complete: {indexed} files updated, {skipped} unchanged.")

def _flush_chunks(con: sqlite3.Connection, chunks: list[tuple]):
    cur = con.cursor()
    texts = [f"{header}: {content[:400]}" for _, header, content, _ in chunks]
    embeddings = get_embeddings_batch(texts)
    
    for i, (rel_p, header, content, mtime) in enumerate(chunks):
        emb_blob = array.array("f", embeddings[i]).tobytes() if (embeddings and i < len(embeddings)) else None
        cur.execute(
            "INSERT INTO doc_chunks (file_path, header, content, mtime, embedding) VALUES (?, ?, ?, ?, ?)",
            (rel_p, header, content, mtime, emb_blob)
        )
        chunk_id = cur.lastrowid
        cur.execute(
            "INSERT INTO doc_fts (rowid, file_path, header, content) VALUES (?, ?, ?, ?)",
            (chunk_id, rel_p, header, content)
        )
    con.commit()

def cosine_similarity(v1: array.array, v2: list[float]) -> float:
    dot = 0.0
    norm1 = 0.0
    norm2 = 0.0
    for a, b in zip(v1, v2):
        dot += a * b
        norm1 += a * a
        norm2 += b * b
    if norm1 == 0 or norm2 == 0:
        return 0.0
    return dot / (math.sqrt(norm1) * math.sqrt(norm2))

def search(con: sqlite3.Connection, query: str, limit: int = 3) -> list[dict]:
    query_emb = get_embedding(query)
    cur = con.cursor()
    results = []

    if query_emb is not None:
        # Semantic Vector Search
        rows = cur.execute("SELECT id, file_path, header, content, embedding FROM doc_chunks WHERE embedding IS NOT NULL").fetchall()
        scored = []
        for r in rows:
            blob = r[4]
            arr = array.array("f")
            arr.frombytes(blob)
            sim = cosine_similarity(arr, query_emb)
            scored.append((sim, r[1], r[2], r[3]))
        scored.sort(key=lambda x: x[0], reverse=True)
        for sim, fpath, header, content in scored[:limit]:
            results.append({
                "score": round(sim, 4),
                "file": fpath,
                "header": header,
                "snippet": content[:300].replace("\n", " ").strip() + "...",
                "mode": "vector"
            })
    else:
        # Fallback to SQLite FTS5 (BM25)
        clean_q = re.sub(r'[^a-zA-Z0-9_\s]', ' ', query).strip()
        rows = cur.execute(
            """SELECT file_path, header, snippet(doc_fts, 2, '<b>', '</b>', '...', 25), rank 
               FROM doc_fts WHERE doc_fts MATCH ? ORDER BY rank LIMIT ?""",
            (clean_q, limit)
        ).fetchall()
        for fpath, header, snip, rank in rows:
            results.append({
                "score": round(-rank, 4),
                "file": fpath,
                "header": header,
                "snippet": snip.replace("\n", " ").strip(),
                "mode": "fts5_fallback"
            })

    return results

def main():
    parser = argparse.ArgumentParser(description="Semantic and FTS5 search for project documentation")
    subparsers = parser.add_subparsers(dest="action")

    p_idx = subparsers.add_parser("index", help="Index markdown directories")
    p_idx.add_argument("dirs", nargs="*", default=["patch_manager_mods", "."], help="Directories or files to index")

    p_search = subparsers.add_parser("search", help="Search documentation")
    p_search.add_argument("query", help="Search query")
    p_search.add_argument("--limit", type=int, default=3, help="Max results")
    p_search.add_argument("--json", action="store_true", help="JSON output")

    # If first arg is not 'index' or 'search', treat it as a query directly
    if len(sys.argv) > 1 and sys.argv[1] not in ("index", "search", "-h", "--help"):
        sys.argv.insert(1, "search")

    args = parser.parse_args()
    con = sqlite3.connect(DB_PATH)
    init_db(con)

    if args.action == "index":
        target_dirs = [REPO_ROOT / d for d in args.dirs]
        index_docs(con, target_dirs)
    elif args.action == "search":
        res = search(con, args.query, limit=args.limit)
        if args.json:
            print(json.dumps(res, indent=2))
        else:
            if not res:
                print("No relevant documentation matches found.")
            for i, r in enumerate(res, 1):
                print(f"[{i}] {r['file']} -> ## {r['header']} (score: {r['score']}, mode: {r['mode']})")
                print(f"    {r['snippet']}\n")

if __name__ == "__main__":
    main()
