#!/usr/bin/env python3
"""R1 pilot cards: the COMMITTED B13a card text (what the Sonnet reviewers saw) for the cards in key_pilot.tsv, with
optional report-only sections appended per arm.

    python3 ../../tools/batch.py cards B13a --rea --out B13a/rea_pilot/cards_full   (from steam_db_batches/)
    python3 B13a/rea_pilot/build_pilot_cards.py                        (no --arm: rea + norea, as before)
    python3 B13a/rea_pilot/build_pilot_cards.py --arm facts facts_rea  (needs cards_full_facts/ and cards_full_facts_rea/)

Arms share the same card order, the same two halves and the same committed body:
    norea      body only                                  -> cards_norea/pilot_<k>.md
    rea        body + REA section (cards_full/)           -> cards_rea/pilot_<k>.md
    facts      body + static facts (cards_full_facts/)    -> cards_facts/pilot_<k>.md
    facts_rea  body + facts + REA (cards_full_facts_rea/) -> cards_facts_rea/pilot_<k>.md
build() is pure; files are written only by main()."""
import argparse
import csv
import random
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
B = HERE.parent
CARD_RE = re.compile(r"^## Card (\S+)\s*$", re.M)
REA_HDR = "### Live evidence (REA, report-only)"
FACTS_HDR = "### Static facts (script output, report-only)"
# arm -> (output dir, source dir of full card blocks, or None when the arm needs only the committed body)
ARMS = {
    "norea": ("cards_norea", None),
    "rea": ("cards_rea", "cards_full"),
    "facts": ("cards_facts", "cards_full_facts"),
    "facts_rea": ("cards_facts_rea", "cards_full_facts_rea"),
}
DEFAULT_ARMS = ("rea", "norea")
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


def load_blocks(directory):
    out = {}
    for f in sorted(directory.glob("chunk_*.md")):
        out.update(blocks(f.read_text(encoding="utf-8")))
    return out


def pilot_ids():
    with open(HERE / "key_pilot.tsv", encoding="utf-8", newline="") as fh:
        ids = [r["card_id"] for r in csv.DictReader(fh, delimiter="\t")]
    random.Random("B13a-pilot-cards").shuffle(ids)
    return ids


def section(cid, block, hdr, to_next_h3=False):
    """Text from `hdr` to the end of the card block, or to the next `### ` line when to_next_h3."""
    if hdr not in block:
        raise ValueError("card %s: source has no %r section" % (cid, hdr))
    text = block[block.index(hdr):]
    if to_next_h3:
        m = re.search(r"^### ", text[len(hdr):], re.M)
        if m:
            text = text[:len(hdr) + m.start()]
    return text.rstrip("\n")


def card_text(arm, cid, body, block):
    if arm == "norea":
        return body
    if arm == "rea":
        return body + "\n" + section(cid, block, REA_HDR) + "\n"
    facts = section(cid, block, FACTS_HDR, to_next_h3=True)
    if arm == "facts":
        return body + "\n" + facts + "\n"
    # facts_rea: facts always first, then REA, whatever the order in the source block
    return body + "\n" + facts + "\n\n" + section(cid, block, REA_HDR) + "\n"


def build(arm, committed, full, ids):
    """Two part texts (part 1, part 2) for one arm. `full` maps card id -> card block from that arm's source dir
    (ignored for norea). Raises ValueError for an unknown arm or a card missing from committed/full."""
    if arm not in ARMS:
        raise ValueError("unknown arm %r" % arm)
    half = (len(ids) + 1) // 2
    parts = []
    for k, part in enumerate((ids[:half], ids[half:]), 1):
        out = [HEADER % k]
        for cid in part:
            if cid not in committed:
                raise ValueError("card %s: not in committed cards/" % cid)
            block = ""
            if arm != "norea":
                if cid not in full:
                    raise ValueError("card %s: not in the %s source" % (cid, ARMS[arm][1]))
                block = full[cid]
            out.append(card_text(arm, cid, committed[cid].rstrip("\n") + "\n", block))
        parts.append("\n".join(out))
    return parts


def main(argv=None):
    ap = argparse.ArgumentParser(description="Build the B13a R1 pilot card arms (see module docstring).")
    ap.add_argument("--arm", nargs="+", action="extend", choices=list(ARMS), metavar="ARM",
                    help="arms to build: %s. Repeat the flag or list several (default: rea norea)." % " ".join(ARMS))
    args = ap.parse_args(argv)
    arms = list(dict.fromkeys(args.arm)) if args.arm else list(DEFAULT_ARMS)
    for arm in arms:
        src = ARMS[arm][1]
        if src and not (HERE / src).is_dir():
            print("error: arm %s needs %s, which does not exist (generate it first, see module docstring)"
                  % (arm, HERE / src), file=sys.stderr)
            return 2
    try:
        committed = load_blocks(B / "cards")
        ids = pilot_ids()
        results = {}
        for arm in arms:
            src = ARMS[arm][1]
            full = load_blocks(HERE / src) if src else {}
            results[arm] = build(arm, committed, full, ids)
    except ValueError as exc:
        print("error: %s" % exc, file=sys.stderr)
        return 2
    for arm in arms:
        out_dir = HERE / ARMS[arm][0]
        out_dir.mkdir(exist_ok=True)
        for k, text in enumerate(results[arm], 1):
            (out_dir / ("pilot_%d.md" % k)).write_text(text, encoding="utf-8")
            print("%s: part %d: %d cards" % (out_dir.name, k, len(CARD_RE.findall(text))))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
