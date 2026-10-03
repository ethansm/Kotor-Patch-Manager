#!/usr/bin/env python3
"""Build the C1b blind verification packet + answer key (read-only on the dump root; stdlib only).

Evidence comes only from the sanctioned tool tools/re_card_prep.py (annotate / anchors / prologue) over the committed dump
root ~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/ plus its per-function .asm for site windows.
Deterministic: fixed shuffle seed.  Usage: build_packet.py [--root DUMP_ROOT] [--batch 22]
"""
import argparse, json, os, random, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.dirname(HERE)
KB = os.path.abspath(os.path.join(HERE, "../../../../patch_manager_mods/ghidra_knowledge/tools"))
sys.path.insert(0, KB); sys.path.insert(0, HERE)
import re_card_prep as rp
from candidates import *

ap = argparse.ArgumentParser(); ap.add_argument("--root", default=None); ap.add_argument("--batch", type=int, default=22)
args = ap.parse_args()
ROOT = rp.expand_root(args.root)
CARD_TOOL = os.path.join(KB, "re_card_prep.py")
_cache = {}

def tool(sub, addr, *extra):
    k = (sub, addr, extra)
    if k not in _cache:
        r = subprocess.run([sys.executable, CARD_TOOL, "--root", ROOT, sub, "0x%X" % addr, *extra], capture_output=True, text=True)
        if r.returncode != 0 or r.stdout.startswith("ERROR"):
            sys.exit("tool failed %s 0x%X: %s%s" % (sub, addr, r.stdout[:200], r.stderr[:200]))
        _cache[k] = r.stdout.rstrip()
    return _cache[k]

def fn_evidence(addr):
    """Tier-0 facts + anchors + annotated decompile.  Contains no proposed name."""
    hdr = rp.parse_header(rp.read_text(rp.c_path(ROOT, addr)))
    pro = tool("prologue", addr)
    anc = tool("anchors", addr).split("\n")
    anc = anc[:34]
    dec = tool("annotate", addr, "--max-lines", "48")
    return ("TIER-0 FACTS (script-computed, trust these):\n"
            "- entry 0x%08X, size %s, header ret=%s (0 = no stack args popped / fastcall-or-thiscall w/o stack args)\n"
            "- %s\n" % (addr, hdr and hdr["size"], hdr and hdr["ret"], pro.split("\n")[1].replace("16-byte aligned: ", "16-byte aligned: ")) +
            "- %s\n" % [l for l in pro.split("\n") if l.startswith("standard prologue")][0] +
            "ANCHORS (script):\n" + "\n".join(anc) + "\n\nDECOMPILE (offsets annotated; truncated):\n" + dec)

def asm_window(parent, addr, before=7, after=7):
    asm = rp.read_asm(ROOT, parent)
    if not asm: sys.exit("no asm for 0x%X" % parent)
    idx = [i for i, a in enumerate(asm) if a[0] == addr]
    if not idx: sys.exit("0x%X not an instruction start in 0x%X" % (addr, parent))
    i = idx[0]
    return i, asm, "\n".join("%s%08X  %-18s %s" % (">> " if asm[j][0] == addr else "   ", asm[j][0], " ".join(asm[j][1][k:k+2] for k in range(0, len(asm[j][1]), 2)), asm[j][2])
                             for j in range(max(0, i-before), min(len(asm), i+after+1)))

by_addr = {c["addr"]: c for c in FUNCS}
cards = []   # dict(kind real/decoy, decoy_type, cand, claim, evidence)

def add(kind, dtype, cand, claim, ev, expected):
    cards.append(dict(kind=kind, decoy_type=dtype, cand=cand, claim=claim, evidence=ev, expected=expected))

for c in FUNCS:
    a = c["addr"]
    add("real", None, a, "Function at 0x%08X is `%s`: %s." % (a, c["name"], c["purpose"]), fn_evidence(a), "match")
    nb = c["nb"]
    add("decoy", "wrong_address", a, "Function at 0x%08X is `%s`: %s." % (nb, c["name"], c["purpose"]), fn_evidence(nb), "mismatch")
    sb = by_addr.get(c["sib"]) or ({"name": EXTRA_NAMES[c["sib"]][0], "purpose": EXTRA_NAMES[c["sib"]][1]} if c["sib"] in EXTRA_NAMES else None)
    assert sb, "no sibling name for 0x%X" % a
    add("decoy", "wrong_name", a, "Function at 0x%08X is `%s`: %s." % (a, sb["name"], sb["purpose"]) if sb else "", fn_evidence(a), "mismatch")

for ctor, vt, cls, wrong in VTABLES:
    txt = rp.read_text(rp.asm_path(ROOT, ctor))
    lines = [l for l in txt.split("\n") if ("0x%x" % vt) in l.lower() or ("0x%x" % wrong) in l.lower()]
    hits = [l for l in txt.split("\n") if "0x9aa" in l.lower()]
    anc = "\n".join(tool("anchors", ctor).split("\n")[:12])
    ev = "ASM lines of ctor 0x%08X mentioning a 0x9AAxxx constant:\n%s\n\nANCHORS:\n%s\n\nVTABLE DUMP (R10_out_vtables.txt, slot 0 / 15 / 18 shown):\n%s" % (
        ctor, "\n".join(hits[:6]) or "(none)", anc, "(see claim)")
    vt_txt = open(os.path.join(OUT, "R10_out_vtables.txt")).read().split("## vtable ")
    vtd = {int(b.split()[0], 16): b for b in vt_txt[1:]}
    def vtev(addr):
        b = vtd.get(addr)
        return "\n".join(b.split("\n")[0:1] + [l for l in b.split("\n") if l.strip().startswith(("slot 0 ", "slot 15 ", "slot 18 "))]) if b else "(no dump)"
    base = "ASM lines of ctor 0x%08X containing a 0x9AAxxx constant:\n%s\n\nANCHORS of ctor:\n%s\n" % (ctor, "\n".join(hits[:6]) or "(none)", anc)
    add("real", None, ctor, "Class `%s` has vftable 0x%08X; its constructor is 0x%08X." % (cls, vt, ctor), base + "\nVTABLE DUMP:\n" + vtev(vt), "match")
    add("decoy", "wrong_vftable", ctor, "Class `%s` has vftable 0x%08X; its constructor is 0x%08X." % (cls, wrong, ctor), base + "\nVTABLE DUMP:\n" + vtev(wrong), "mismatch")
    other = [v for v in VTABLES if v[0] != ctor][0]
    add("decoy", "wrong_name", ctor, "Class `%s` has vftable 0x%08X; its constructor is 0x%08X." % (other[2], vt, ctor), base + "\nVTABLE DUMP:\n" + vtev(vt), "mismatch")

for s in SITES:
    i, asm, win = asm_window(s["parent"], s["addr"])
    _, _, xwin = asm_window(s["parent"], s["exit"], 3, 3)
    _, _, xbad = asm_window(s["parent"], s["exit_bad"], 3, 3)
    shifted = asm[i + 2][0]; _, _, win2 = asm_window(s["parent"], shifted)
    def claim(addr, ex):
        return ("Hook site at 0x%08X inside the function at 0x%08X: bytes `%s`. At this point %s. Consumed-exit (skip) target 0x%08X = %s."
                % (addr, s["parent"], s["bytes"], s["role"], ex, s["exit_role"]))
    add("real", None, s["addr"], claim(s["addr"], s["exit"]), "SITE WINDOW (>> = claimed address):\n%s\n\nEXIT WINDOW:\n%s" % (win, xwin), "match")
    add("decoy", "wrong_address", s["addr"], claim(shifted, s["exit"]), "SITE WINDOW (>> = claimed address):\n%s\n\nEXIT WINDOW:\n%s" % (win2, xwin), "mismatch")
    add("decoy", "wrong_exit", s["addr"], claim(s["addr"], s["exit_bad"]), "SITE WINDOW (>> = claimed address):\n%s\n\nEXIT WINDOW:\n%s" % (win, xbad), "mismatch")

for k in CALLS:
    i, asm, win = asm_window(k["parent"], k["addr"], 8, 4)
    shifted = asm[i - 2][0]; _, _, win2 = asm_window(k["parent"], shifted, 8, 4)
    cn = {0x8480B0: "CanChooseFeat (existing DB, high)", 0x848660: "unnamed-in-DB power-availability test (candidate in this packet)"}
    def claim(addr, callee):
        return "Call site at 0x%08X in the function at 0x%08X: bytes `%s` = call to 0x%08X [%s]; %s." % (addr, k["parent"], k["bytes"], callee, cn[callee], k["role"])
    add("real", None, k["addr"], claim(k["addr"], k["callee"]), "CALL WINDOW (>> = claimed address):\n" + win, "match")
    add("decoy", "wrong_address", k["addr"], claim(shifted, k["callee"]), "CALL WINDOW (>> = claimed address):\n" + win2, "mismatch")
    add("decoy", "wrong_callee", k["addr"], claim(k["addr"], k["callee_bad"]), "CALL WINDOW (>> = claimed address):\n" + win, "mismatch")

# drop degenerate cards (no sibling), shuffle, id
random.Random(20261003).shuffle(cards)
for n, c in enumerate(cards, 1): c["id"] = "C%03d" % n

HEAD = open(os.path.join(HERE, "packet_header.md")).read()
out = [HEAD]
B = args.batch
for b in range(0, len(cards), B):
    out.append("\n\n# BATCH %d (cards %s-%s)\n" % (b // B + 1, cards[b]["id"], cards[min(b + B, len(cards)) - 1]["id"]))
    for c in cards[b:b + B]:
        out.append("\n---\n## CARD %s\n**CLAIM:** %s\n\n```\n%s\n```\n" % (c["id"], c["claim"], c["evidence"]))
open(os.path.join(OUT, "C1b_verify_packet.md"), "w").write("".join(out))
key = {"seed": 20261003, "n_cards": len(cards), "n_real": sum(c["kind"] == "real" for c in cards),
       "cards": {c["id"]: dict(kind=c["kind"], decoy_type=c["decoy_type"], cand_addr="0x%08X" % c["cand"], expected=c["expected"], claim=c["claim"]) for c in cards}}
json.dump(key, open(os.path.join(OUT, "C1b_answer_key.json"), "w"), indent=1)
print("cards", len(cards), "real", key["n_real"], "decoys", len(cards) - key["n_real"])
