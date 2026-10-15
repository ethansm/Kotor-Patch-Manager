#!/usr/bin/env python3
"""rea_questions.py - turn a Steam DB batch's review-card keys into blind rea-recon question files.

Usage:
  python3 tools/rea_questions.py <B> [--per-run 8] [--out DIR] [--keys FILE ...]
                                 [--ledger-dir DIR] [--batches-dir DIR]

Reads <B>/cards/key*.tsv (or the --keys files), collects the distinct Steam addresses, drops
addresses already confirmed in the REA ledger under topic <B>-q*, shuffles the rest with a seed
derived from <B>, and writes <out>/q_<k>.md (default <out> = <B>/rea/).

Blind by construction: question files hold no GOG address, no card id, no kind label and no name.
Stdlib only. Exit codes: 0 ok, 2 usage or data error.
"""

import argparse
import csv
import datetime as dt
import importlib.util
import random
import re
import sys
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
BATCHES_DIR = TOOLS.parent
REPO = Path(__file__).resolve().parents[3]
QUERY_GHIDRA = REPO / "scripts" / "query_ghidra.py"
DEFAULT_LEDGER_DIR = REPO / "patch_manager_mods" / "ghidra_knowledge" / "rea_findings"
BINARY = "/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe"
BATCH_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9_-]*")


class DataError(Exception):
    """Bad usage or bad input data; reported on stderr with exit code 2."""


def _load_query_ghidra():
    spec = importlib.util.spec_from_file_location("_rea_query_ghidra", QUERY_GHIDRA)
    if spec is None or spec.loader is None:
        raise DataError(f"cannot load {QUERY_GHIDRA}")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def read_steam_addrs(paths):
    """Distinct steam_addr values (int) over every row of the key TSV files."""
    addrs = set()
    for path in paths:
        with path.open(encoding="utf-8", newline="") as fh:
            reader = csv.DictReader(fh, delimiter="\t")
            if not reader.fieldnames or "steam_addr" not in reader.fieldnames:
                raise DataError(f"{path}: header has no steam_addr column")
            for row in reader:
                raw = (row.get("steam_addr") or "").strip()
                try:
                    val = int(raw, 16)
                except ValueError:
                    raise DataError(f"{path}:{reader.line_num}: bad steam_addr {raw!r}") from None
                if not 0 < val <= 0xFFFFFFFF:
                    raise DataError(f"{path}:{reader.line_num}: steam_addr out of range {raw!r}")
                addrs.add(val)
    return addrs


def confirmed_ledger_addrs(rows, batch):
    """Addresses with a confirmed ledger row whose topic starts with <batch>-q."""
    prefix = f"{batch}-q"
    done = set()
    for row in rows:
        if row.get("confidence") != "confirmed" or not str(row.get("topic", "")).startswith(prefix):
            continue
        try:
            done.add(int(row["addr"], 16))
        except (KeyError, TypeError, ValueError):
            continue
    return done


def render_question(batch, k, addrs, ledger_path, today):
    topic = f"{batch}-q{k}"
    lines = [
        f"# REA questions {topic}",
        "",
        f"Binary: {BINARY}",
        f"Ledger topic: {topic}",
        f"Ledger file: {ledger_path}",
        "",
        "## Rules",
        "",
        "1. Call `open_binary` once with provider_id `ghidra` and the Binary path above. Pass no snapshot_path.",
        "2. Answer every address below in that one session. Do not reopen the binary, except once: if a call fails with \"Connection closed\" or \"No app is open\", call close_binary, then open_binary again and continue.",
        "3. Call `close_binary` (no snapshot_path) before you reply.",
        "4. Report observations only. Write \"unknown\" when Ghidra does not show the answer.",
        "5. Answer each address as its own block, headed by its address.",
        "6. Write the ledger file above. Columns in order (tab-separated): addr, kind, name, confidence, evidence, topic, date.",
        "7. One row per address with kind = function (behaviour summary), plus one row with kind = xref whose name is `callers N: <first 5>; callees: <list or none>; size <n> B RET <n>`. Extra rows are allowed for strings and callsites. Budget: at most 4 REA calls per address; skip pseudo-code unless the asm is unclear.",
        "8. name = a short behavioural summary of what you observed (strings, callees, fields). "
        "Do NOT copy names from steam_functions.tsv, other ledger files or any other source. "
        "Do not guess class or method names unless a string literal in the body names them.",
        "9. confidence = confirmed (quoted anchor), probable (shape only), or refuted (a claim proven wrong).",
        "10. evidence must quote at least one asm line as `0x<addr>: <instr>`. Max 200 characters, no tabs.",
        f"11. topic = {topic}. date = {today.isoformat()}.",
        "",
        "## Addresses",
        "",
    ]
    lines += [f"- 0x{a:08x}" for a in addrs]
    lines += [
        "",
        "## Questions (answer all six for every address)",
        "",
        "1. Function start? yes/no. If no, the start address of the containing procedure.",
        "2. Size in bytes, and the RET operand (RET n, or RET if none).",
        "3. Callers: total count, then the first 5 caller addresses.",
        "4. Callees: their addresses. Add a name only when Ghidra shows an import or library name.",
        "5. Strings pushed or referenced: address and text for each.",
        "6. Vtable or data xref users: addresses, or none.",
        "",
    ]
    return "\n".join(lines)


def next_topic_index(rows, batch):
    """1 + the highest k of any ledger topic `<batch>-q<k>`, so a re-run never reuses a topic already in the ledger."""
    pat = re.compile(re.escape(batch) + r"-q(\d+)$")
    ks = [int(m.group(1)) for m in (pat.match(str(r.get("topic", "")).strip()) for r in rows) if m]
    return max(ks, default=0) + 1


def _run(args):
    if args.per_run < 1:
        raise DataError("--per-run must be >= 1")
    batch = args.batch
    if not BATCH_RE.fullmatch(batch):
        raise DataError(f"bad batch name {batch!r}")

    batch_dir = Path(args.batches_dir) / batch
    if args.keys:
        key_paths = [Path(k) for k in args.keys]
    else:
        key_paths = sorted((batch_dir / "cards").glob("key*.tsv"))
    if not key_paths:
        raise DataError(f"no key files for {batch} (looked in {batch_dir / 'cards'})")
    for path in key_paths:
        if not path.is_file():
            raise DataError(f"key file not found: {path}")

    steam = read_steam_addrs(key_paths)
    ledger_dir = Path(args.ledger_dir)
    ledger_rows = _load_query_ghidra().load_ledger(ledger_dir)
    done = confirmed_ledger_addrs(ledger_rows, batch)
    first = next_topic_index(ledger_rows, batch)
    queue = sorted(steam - done)
    skipped = len(steam) - len(queue)
    random.Random(batch).shuffle(queue)
    chunks = [queue[i:i + args.per_run] for i in range(0, len(queue), args.per_run)]

    out = Path(args.out) if args.out else batch_dir / "rea"
    out.mkdir(parents=True, exist_ok=True)
    for stale in out.glob("q_*.md"):
        stale.unlink()

    today = dt.date.today()
    stamp = today.strftime("%Y%m%d")
    for k, chunk in enumerate(chunks, start=first):
        ledger_path = ledger_dir / f"{stamp}_{batch}-q{k}.tsv"
        text = render_question(batch, k, chunk, ledger_path, today)
        with (out / f"q_{k}.md").open("w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)

    print(f"{batch}: {len(steam)} addresses ({skipped} skipped, ledger) -> {len(chunks)} files in {out.resolve()}")
    return 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Write blind rea-recon question files for a Steam DB batch.")
    ap.add_argument("batch", help="batch name, e.g. B13a")
    ap.add_argument("--per-run", type=int, default=8, help="addresses per question file (default 8)")
    ap.add_argument("--out", help="output dir (default <B>/rea/)")
    ap.add_argument("--keys", action="append", metavar="FILE",
                    help="key TSV to use instead of cards/key*.tsv (repeatable)")
    ap.add_argument("--ledger-dir", default=str(DEFAULT_LEDGER_DIR), help="REA ledger dir (confirmed rows are skipped)")
    ap.add_argument("--batches-dir", default=str(BATCHES_DIR), help="parent of the batch dirs")
    args = ap.parse_args(argv)
    try:
        return _run(args)
    except DataError as exc:
        print(f"rea_questions: error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
