#!/usr/bin/env python3
"""R1 pilot cards: the COMMITTED B13a card text (what the Sonnet reviewers saw) for the cards in key_pilot.tsv, with
and without the `### Live evidence (REA, report-only)` section taken from `cards B13a --rea --out cards_full`.

    python3 ../../tools/batch.py cards B13a --rea --out B13a/rea_pilot/cards_full   (from steam_db_batches/)
    python3 B13a/rea_pilot/build_pilot_cards.py

Writes cards_rea/pilot_<k>.md and cards_norea/pilot_<k>.md (same cards, same order, 2 files)."""
import csv
import random
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
B = HERE.parent
CARD_RE = re.compile(r"^## Card (\S+)\s*$", re.M)
REA_HDR = "### Live evidence (REA, report-only)"
HEADER = """# Review cards B13a R1 pilot part %d

Follow steam_db_batches/REVIEWER_BRIEF.md. One TSV row per card, card order, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | label_suspect | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence.
"""


def blocks(text):
    marks = list(CARD_RE.finditer(text))
    return {m.group(1): text[m.start():(marks[i + 1].start() if i + 1 < len(marks) else len(text))]
            for i, m in enumerate(marks)}


def main():
    committed, full = {}, {}
    for f in sorted((B / "cards").glob("chunk_*.md")):
        committed.update(blocks(f.read_text(encoding="utf-8")))
    for f in sorted((HERE / "cards_full").glob("chunk_*.md")):
        full.update(blocks(f.read_text(encoding="utf-8")))
    ids = [r["card_id"] for r in csv.DictReader(open(HERE / "key_pilot.tsv"), delimiter="\t")]
    random.Random("B13a-pilot-cards").shuffle(ids)
    half = (len(ids) + 1) // 2
    for k, part in enumerate((ids[:half], ids[half:]), 1):
        rea, norea = [HEADER % k], [HEADER % k]
        for cid in part:
            body = committed[cid].rstrip("\n") + "\n"
            sec = full[cid][full[cid].index(REA_HDR):].rstrip("\n")
            norea.append(body)
            rea.append(body + "\n" + sec + "\n")
        for d, parts in (("cards_rea", rea), ("cards_norea", norea)):
            (HERE / d).mkdir(exist_ok=True)
            (HERE / d / ("pilot_%d.md" % k)).write_text("\n".join(parts), encoding="utf-8")
        print("part %d: %d cards" % (k, len(part)))


if __name__ == "__main__":
    main()
