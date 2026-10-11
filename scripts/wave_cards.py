#!/usr/bin/env python3
"""Verify wave cards (`### Task N` blocks in *.md files) before subagents are launched.

Stdlib only; the token check is delegated to wave_budget. Verification commands are parsed, never
executed. Files named *credential* or *secret* are never read.

Usage: python3 scripts/wave_cards.py verify <cards_dir> [--repo-root DIR]
Exit: 0 all cards PASS, 1 any FAIL (or no cards), 2 usage error or missing directory.
"""
import argparse
import fnmatch
import re
import shlex
import sys
from itertools import combinations
from pathlib import Path

# Keep the sibling module importable under `python3 -I`, which drops the script directory from sys.path.
sys.path.insert(0, str(Path(__file__).resolve().parent))
import wave_budget as W  # noqa: E402

DEFAULT_ROOT = Path(__file__).resolve().parent.parent
REQUIRED = {"contract": "Contract", "write scope": "Write Scope", "target files": "Target Files",
            "objective": "Objective", "verification": "Verification"}
CONTRACTS = ("worker", "recon")
KNOWN_BINARIES = ("python3", "git", "rg", "bash", "make", "cmake")
SCRIPT_RUNNERS = ("python3", "bash")
SENSITIVE = ("*credential*", "*secret*")
QUOTE_RADIUS = 5
REASON_WORDS = 25

HEADER_RE = re.compile(r"^###\s+Task\s+([^\s:]+?)\s*:?\s*(.*?)\s*$")
HEADING_RE = re.compile(r"^#{1,3}\s")
FIELD_RE = re.compile(r"^-\s+\*\*(.+?)\*\*\s*:\s*(.*?)\s*$")
BACKTICK_RE = re.compile(r"`([^`\n]+)`")
WINDOW_RE = re.compile(r"(.+):(\d+)-(\d+)")
# path.ext:NNN anchor; `path:lo-hi` ranges are excluded by the lookaheads.
ANCHOR_RE = re.compile(r"(?<![\w./-])((?:[\w.-]+/)*[\w.-]+\.[A-Za-z0-9_]+):(\d+)(?!\d)(?!-\d)")
# Backticked quote right after an anchor, optionally after a closing backtick and separators.
QUOTE_AFTER_RE = re.compile(r"[\s:,;()\-–—]*`([^`\n]+)`")
PATHLIKE_RE = re.compile(r"[\w./-]+\.\w+(?::\d+(?:-\d+)?)?")


class Card:
    def __init__(self, file_name, number, name):
        self.file = file_name
        self.number = number
        self.name = name
        self.lines = []
        self.fields = {}
        self.reasons = []
        self.globs = []

    @property
    def label(self):
        return f"{self.file}#Task {self.number}"

    def value(self, key):
        return "\n".join(self.fields.get(key, [])).strip()


def parse_cards(text, file_name):
    cards, card, field = [], None, None
    for line in text.splitlines():
        header = HEADER_RE.match(line)
        if header:
            card = Card(file_name, header.group(1), header.group(2))
            card.lines.append(line)
            cards.append(card)
            field = None
            continue
        if card is None:
            continue
        if HEADING_RE.match(line):
            card, field = None, None
            continue
        card.lines.append(line)
        bullet = FIELD_RE.match(line)
        if bullet:
            key = " ".join(bullet.group(1).lower().split())
            if key in card.fields:  # keep the first occurrence; ignore its continuation lines
                field = None
            else:
                card.fields[key] = [bullet.group(2)]
                field = key
        elif field and line.strip():
            card.fields[field].append(line.strip())
    return cards


def _backticked(value):
    return [t.strip() for t in BACKTICK_RE.findall(value) if t.strip()]


def _squash(text):
    return " ".join(text.split())


def _is_sensitive(rel):
    low = rel.replace("\\", "/").lower()
    return any(fnmatch.fnmatchcase(low, pat) for pat in SENSITIVE)


def _resolve(root, rel):
    """Absolute path under root, or None when rel escapes the repo root."""
    try:
        path = (root / rel).resolve()
        path.relative_to(root)
    except (ValueError, OSError):
        return None
    return path


def _file_lines(path, cache):
    key = str(path)
    if key not in cache:
        lines = path.read_bytes().decode("utf-8", errors="replace").split("\n")
        if lines and lines[-1] == "":  # trailing newline does not start a new line (matches wc -l / readlines)
            lines.pop()
        cache[key] = lines
    return cache[key]


def _check_targets(value, root, cache):
    reasons = []
    entries = _backticked(value)
    if not entries:
        return ["Target Files has no backticked path"]
    if len(entries) > W.MAX_ENTRIES:
        reasons.append(f"{len(entries)} Target Files entries > {W.MAX_ENTRIES}")
    good = []  # (display, absolute entry for wave_budget)
    for entry in entries:
        window = WINDOW_RE.fullmatch(entry)
        rel = window.group(1) if window else entry
        if _is_sensitive(rel):
            reasons.append(f"refusing to read sensitive file {rel}")
            continue
        path = _resolve(root, rel)
        if path is None:
            reasons.append(f"target outside repo root {rel}")
        elif not path.is_file():
            reasons.append(f"missing target file {rel}")
        elif not window:
            good.append((rel, str(path)))
        else:
            lo, hi = int(window.group(2)), int(window.group(3))
            n = len(_file_lines(path, cache))
            if 1 <= lo <= hi <= n:
                good.append((entry, f"{path}:{lo}-{hi}"))
            else:
                reasons.append(f"window {entry} out of range (file has {n} lines)")
    if reasons or not good:
        return reasons
    res = W.check([absolute for _, absolute in good])
    if not res["ok"]:
        for (display, _), n in zip(good, res["per_entry"]):
            if n > W.PER_ENTRY_CAP:
                reasons.append(f"{display} is ~{n} tokens > {W.PER_ENTRY_CAP}")
        if res["total"] > W.TOTAL_CAP:
            reasons.append(f"budget total {res['total']} > {W.TOTAL_CAP}")
    return reasons


def _script_reason(rel, root):
    path = _resolve(root, rel)
    if path is not None and path.is_file():
        return []
    return [f"verification script not found {rel}"]


def _check_verification(value, root):
    tokens = _backticked(value)
    command = tokens[0] if tokens else value
    try:
        argv = shlex.split(command)
    except ValueError:
        argv = command.split()
    if not argv:
        return ["Verification command is empty"]
    first = argv[0]
    if first in SCRIPT_RUNNERS:
        for arg in argv[1:]:
            if arg in ("-c", "-m"):  # inline code or module: nothing on disk to resolve
                return []
            if arg.startswith("-"):
                continue
            return _script_reason(arg, root)
        return [f"{first} has no script argument"]
    if first in KNOWN_BINARIES:
        return []
    if "/" in first or first.startswith("."):
        return _script_reason(first, root)
    return [f"unresolvable verification binary {first}"]


def _check_anchors(card, root, cache):
    reasons = []
    for line in card.lines:
        for m in ANCHOR_RE.finditer(line):
            rel, lineno = m.group(1), int(m.group(2))
            label = f"{rel}:{lineno}"
            if _is_sensitive(rel):
                reasons.append(f"refusing to read sensitive anchor file {rel}")
                continue
            path = _resolve(root, rel)
            if path is None or not path.is_file():
                reasons.append(f"anchor file missing {rel}")
                continue
            lines = _file_lines(path, cache)
            if not 1 <= lineno <= len(lines):
                reasons.append(f"anchor {label} outside file ({len(lines)} lines)")
                continue
            rest = line[m.end():]
            # a backtick right after the anchor closes the anchor's own span; it never opens the quote
            quote_match = QUOTE_AFTER_RE.match(rest[1:] if rest.startswith("`") else rest)
            if not quote_match:
                continue
            quote = _squash(quote_match.group(1))
            if not quote or PATHLIKE_RE.fullmatch(quote):
                continue
            lo = max(0, lineno - 1 - QUOTE_RADIUS)
            hi = min(len(lines), lineno + QUOTE_RADIUS)
            if quote not in _squash(" ".join(lines[lo:hi])):
                reasons.append(f"quote for {label} not within {QUOTE_RADIUS} lines")
    return reasons


def check_card(card, root, cache):
    reasons = []
    for key, title in REQUIRED.items():
        if not card.value(key):
            reasons.append(f"missing {title}")
    contract = card.value("contract").strip("`* ").lower()
    if contract and contract not in CONTRACTS:
        reasons.append(f"Contract must be worker or recon, got {contract!r}")
    if card.value("write scope"):
        card.globs = _backticked(card.value("write scope"))
        if not card.globs:
            reasons.append("Write Scope has no backticked glob")
    if card.value("target files"):
        reasons += _check_targets(card.value("target files"), root, cache)
    if card.value("verification"):
        reasons += _check_verification(card.value("verification"), root)
    reasons += _check_anchors(card, root, cache)
    return reasons


def _norm_glob(glob):
    glob = glob.strip().replace("\\", "/")
    while glob.startswith("./"):
        glob = glob[2:]
    return glob


def _literal_prefix(glob):
    for i, ch in enumerate(glob):
        if ch in "*?[":
            return glob[:i]
    return glob


def _covers(prefix, other):
    """True when prefix is empty (matches everywhere), equal to other, or an ancestor directory of it."""
    return prefix == "" or other == prefix or other.startswith(prefix.rstrip("/") + "/")


def _globs_overlap(a, b):
    a, b = _norm_glob(a), _norm_glob(b)
    if fnmatch.fnmatchcase(a, b) or fnmatch.fnmatchcase(b, a):
        return True
    pa, pb = _literal_prefix(a), _literal_prefix(b)
    return _covers(pa, pb) or _covers(pb, pa)


def check_overlaps(cards):
    """Each card pair is compared once; an overlap adds one reason to each card in the pair."""
    for a, b in combinations(cards, 2):
        if any(_globs_overlap(x, y) for x in a.globs for y in b.globs):
            a.reasons.append(f"write scope overlaps {b.label}")
            b.reasons.append(f"write scope overlaps {a.label}")


def _cap_words(text):
    return " ".join(text.split()[:REASON_WORDS])


def verify(cards_dir, root):
    root = root.resolve()
    cards = []
    for path in sorted(cards_dir.glob("*.md")):
        if not path.is_file():
            continue
        if _is_sensitive(path.name):
            print(f"wave_cards: skipped sensitive-named file {path.name}", file=sys.stderr)
            continue
        cards += parse_cards(path.read_text(encoding="utf-8", errors="replace"), path.name)
    if not cards:
        print(f"wave_cards: no ### Task cards found in {cards_dir}", file=sys.stderr)
        print("VERDICT: FAIL cards=0 fails=0")
        return 1
    cache = {}
    for card in cards:
        card.reasons = check_card(card, root, cache)
    check_overlaps(cards)
    fails = 0
    for card in cards:
        if card.reasons:
            fails += 1
            print(f"{card.label} FAIL {_cap_words('; '.join(card.reasons))}")
        else:
            print(f"{card.label} PASS all checks passed")
    print(f"VERDICT: {'FAIL' if fails else 'PASS'} cards={len(cards)} fails={fails}")
    return 1 if fails else 0


def main(argv=None):
    parser = argparse.ArgumentParser(description="Verify wave cards before dispatching subagents.")
    sub = parser.add_subparsers(dest="cmd", required=True)
    p_verify = sub.add_parser("verify", help="check every ### Task card in a directory")
    p_verify.add_argument("cards_dir", metavar="cards_dir")
    p_verify.add_argument("--repo-root", default=str(DEFAULT_ROOT), metavar="DIR")
    args = parser.parse_args(argv)

    cards_dir = Path(args.cards_dir)
    root = Path(args.repo_root)
    if not cards_dir.is_dir():
        print(f"wave_cards: not a directory: {args.cards_dir}", file=sys.stderr)
        return 2
    if not root.is_dir():
        print(f"wave_cards: repo root not a directory: {args.repo_root}", file=sys.stderr)
        return 2
    return verify(cards_dir, root)


if __name__ == "__main__":
    sys.exit(main())
