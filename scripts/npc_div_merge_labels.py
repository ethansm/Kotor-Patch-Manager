#!/usr/bin/env python3
"""Merge texture votes + UV features into patch_manager_mods/npc_diversity_src/texture_labels.json (stdlib only).

Inputs : .claude/npc_div/tex/features.json (npc_div_texture_sheets.py build), .claude/npc_div/labels/vote*_pool_*.json
         (one object per sheet, keyed by lower-case slot name, e.g. "commm12"), provenance = MALE_SLOTS / FEMALE_SLOTS in
         scripts/normalize_diversity_textures.py (slot N came from list entry N-1).
Output : texture_labels.json: {"meta": {...}, "slots": {"n_commm12": {...}}} + a rejected-files list.

Decision per slot (deterministic):
  layout evidence  = uv_corr >= UV_MIN (calibrated: vanilla min 0.28, decoy max 0.09) AND every vote says layout 'yes'
  clothing evidence = every vote says category commoner_clothing
  native fat slot   = provenance name contains 'fat' (fat-mesh UV space): allowed for FatM/FatF only, never M/F
  M/F allowlist     = vanilla slots 01-11 always (real commoner art) + any slot with layout and clothing evidence, not fat
  FatM/FatF         = native fat slots only
  review            = a vote with confidence <= 2, a layout 'unsure', or votes disagreeing: listed for the human spot-check
Slots 01-11 are never rejected (vanilla default tv of every commoner is 06 / 09).
"""
from __future__ import annotations

import argparse
import glob
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
FEATURES = REPO / ".claude/npc_div/tex/features.json"
VOTES_GLOB = str(REPO / ".claude/npc_div/labels/vote*_pool_*.json")
OUT = REPO / "patch_manager_mods/npc_diversity_src/texture_labels.json"
UV_MIN = 0.25
VANILLA_SLOTS = range(1, 12)
# Orchestrator adjudications (recorded, reviewed in the human spot-check): slot -> {family: verdict}.
ADJUDICATED = {"n_commm34": {"FatM": "yes"},
               # n_commm34: fat-native provenance (n_fatcomm01) + uv 0.41; the vote was unsure only because the fat atlas differs
               # 10-16 user veto after the in-game check (dirty K1 rag textures look bad on commoners): both trial slots removed.
               "n_commm16": {"M": "no"},    # was "yes" (spot-check trial); n_commmd01 ragged blood-stained clothes
               "n_commf17": {"F": "no"},    # was "yes" (spot-check trial); n_commfd01 tattered patchwork rags
               # 10-17 user veto: vanilla N_CommM10 (dark tunic, diagonal orange stripe, chest badge) reads as a Czerka uniform on a civilian Telosian
               "n_commm10": {"M": "no"}}
# Textures authored for OTHER body meshes (user liked them: robes, armor, uniforms). Not valid on the plain N_CommM/F mesh;
# usable through appearance rows that use that mesh (row remap, later tier). provenance prefix -> (mesh, rows).
ALT_BODY = {
    "n_ondcomm": ("N_OndComM1", [472, 473, 474, 475, 476, 477, 478]),
    "n_ondcomf": ("N_OndComF1", [502, 503, 504, 505, 506]),
    "n_sithcomm": ("N_SithComM", [43, 296, 297, 298, 299, 300, 302]),
    "n_sithcomf": ("N_SithComF", [42, 291, 292, 293, 294, 295, 301]),
}
FLAGGED = {"dirty_poor": ("ragged", "blood", "tattered", "rags")}


def provenance():
    sys.path.insert(0, str(REPO / "scripts"))
    import normalize_diversity_textures as N
    return {"M": list(N.MALE_SLOTS), "F": list(N.FEMALE_SLOTS)}


def load_votes(pattern=VOTES_GLOB):
    """{slot: [vote dict, ...]}"""
    votes = {}
    for p in sorted(glob.glob(pattern)):
        for slot, v in json.loads(Path(p).read_text()).items():
            votes.setdefault(slot.lower(), []).append(v)
    return votes


def decide(slot_no, gender, src, uv, votes):
    """Pure decision for one slot. gender 'M'|'F'; src = provenance resref; uv = uv_corr; votes = list of dicts."""
    fat = "fat" in (src or "")
    cats = {v.get("category") for v in votes}
    layouts = {v.get("layout_matches_ref") for v in votes}
    min_conf = min((int(v.get("confidence", 0)) for v in votes), default=0)
    layout_ev = bool(votes) and uv is not None and uv >= UV_MIN and layouts == {"yes"}
    clothing_ev = bool(votes) and cats == {"commoner_clothing"}
    review = []
    if not votes:
        review.append("no votes")
    if "unsure" in layouts:
        review.append("layout unsure")
    if len(cats) > 1 or len(layouts) > 1:
        review.append("votes disagree")
    if min_conf <= 2 and votes:
        review.append("low confidence")
    uv_vote_conflict = (uv is not None and uv >= UV_MIN) != (layouts == {"yes"})
    if uv_vote_conflict and votes:
        review.append("uv/vote conflict")
    vanilla = slot_no in VANILLA_SLOTS
    ok = vanilla or (layout_ev and clothing_ev)
    fam = {"M": "no", "F": "no", "FatM": "no", "FatF": "no"}
    if fat:
        if ok or (uv is not None and uv >= UV_MIN):
            fam["FatM" if gender == "M" else "FatF"] = "yes" if (layout_ev and clothing_ev) else "unsure"
    elif ok:
        fam[gender] = "yes"
    outfit = " ".join(v.get("outfit", "") for v in votes[:1])
    cat = sorted(c for c in cats if c)[0] if len(cats) == 1 else "mixed:" + "|".join(sorted(c for c in cats if c))
    flags = [k for k, words in FLAGGED.items() if any(w in outfit.lower() for w in words)]
    if flags and not fat:
        # ragged / dirty outfits are held back from the default pool (reserved for a later poor/refugee role table)
        for k in ("M", "F"):
            if fam[k] == "yes":
                fam[k] = "unsure"
        review.append("flag:" + ",".join(flags))
    adj = ADJUDICATED.get(("n_comm%s%02d" % (gender.lower(), slot_no)), {})
    for k, verdict in adj.items():
        fam[k] = verdict
    alt = next(({"mesh": m, "rows": r} for pre, (m, r) in ALT_BODY.items() if (src or "").startswith(pre)), None)
    return {"alt_body": alt, "category": cat, "layout_ok": fam, "uv_corr": uv, "outfit": outfit, "min_conf": min_conf,
            "source": src, "fat_native": fat, "vanilla": vanilla, "flags": flags, "review": review,
            "adjudicated": adj,
            "keep": bool(vanilla or any(x in ("yes", "unsure") for x in fam.values()))}


def merge(features, votes, prov):
    slots = {}
    for g, pre in (("M", "n_commm"), ("F", "n_commf")):
        for i, src in enumerate(prov[g], start=1):
            name = "%s%02d" % (pre, i)
            f = features.get(name)
            if f is None:
                continue
            slots[name] = decide(i, g, src, f.get("uv_corr"), votes.get(name[2:], []))
            slots[name]["slot"] = i
            slots[name]["gender"] = g
    return slots


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--out", default=str(OUT))
    ap.add_argument("--features", default=str(FEATURES))
    ap.add_argument("--votes", default=VOTES_GLOB)
    a = ap.parse_args(argv)
    # Features hold both pool ('n_commm12') and prefixed vanilla/decoy keys; only pool keys have no ':'.
    feats = {k: v for k, v in json.loads(Path(a.features).read_text()).items() if ":" not in k}
    slots = merge(feats, load_votes(a.votes), provenance())
    allow = {fam: sorted(s["slot"] for s in slots.values() if s["layout_ok"][fam] == "yes") for fam in ("M", "F", "FatM", "FatF")}
    rejected = sorted(n for n, s in slots.items() if not s["keep"])
    review = sorted(n for n, s in slots.items() if s["review"] and s["slot"] not in VANILLA_SLOTS)
    doc = {"meta": {"uv_min": UV_MIN, "pool_files": len(slots), "allow": allow, "rejected_files": rejected,
                    "review": review},
           "slots": {k: slots[k] for k in sorted(slots)}}
    Path(a.out).write_text(json.dumps(doc, indent=1, sort_keys=True) + "\n")
    print("slots=%d allow M=%d F=%d FatM=%d FatF=%d rejected=%d review=%d -> %s" % (
        len(slots), len(allow["M"]), len(allow["F"]), len(allow["FatM"]), len(allow["FatF"]), len(rejected),
        len(review), a.out))
    print("review:", ", ".join(review) or "none")
    return 0


if __name__ == "__main__":
    sys.exit(main())
