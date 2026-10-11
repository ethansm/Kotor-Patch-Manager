#!/usr/bin/env python3
"""NPC-diversity texture sheet + feature builder (static; reads game/override files, never writes them).

Renders the 70 pool textures (override N_CommM01-35 / N_CommF01-35), 30 pinned decoys and 22 vanilla N_CommM/F01-11
references to 512 px PNGs, builds 12-up contact sheets, and computes per-texture features (dims, format, alpha, mean
colour, skin-tone pixel fraction, UV-layout signature correlation vs the vanilla commoner template). pykotor MDX UVs are
unusable, so the layout is judged from the texture itself. Calibration report: AUC of the correlation, vanilla vs decoys.

Usage (needs pykotor + Pillow + numpy: use .venv/bin/python):
  npc_div_texture_sheets.py build [--out DIR] [--seed N] [--limit N]
  npc_div_texture_sheets.py report [--out DIR]          # AUC from DIR/features.json
Outputs under DIR (default .claude/npc_div/tex): png/<name>.png, sheets/*.png, features.json, calib_key.json, sheets.json
"""
from __future__ import annotations

import argparse
import json
import random
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

REPO = Path(__file__).resolve().parent.parent
OVERRIDE = REPO / "patch_manager_mods/npc_diversity_src/override"
GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
DEFAULT_OUT = REPO / ".claude/npc_div/tex"

SIG = 64          # layout signature grid
PNG_PX = 512
TILE_PX = 256
SHEET_COLS, SHEET_ROWS = 4, 3   # 12-up
SHEET_N = SHEET_COLS * SHEET_ROWS

# Pinned decoys (truth categories). Names exist in the vanilla texture packs (checked in this session).
DECOYS = [
    ("pmbc01", "armor", "male"), ("pmbc05", "armor", "male"), ("pmbc09", "armor", "male"), ("pmbd02", "armor", "male"),
    ("pmbd05", "armor", "male"), ("pfbc01", "armor", "female"), ("pfbc05", "armor", "female"),
    ("pfbc09", "armor", "female"), ("pfbd02", "armor", "female"), ("pfbd05", "armor", "female"),
    ("pfha01", "face_head", "female"), ("pfha03", "face_head", "female"), ("pfha04", "face_head", "female"),
    ("pfha05", "face_head", "female"), ("pfha01d1", "face_head", "female"), ("p_atrish01", "face_head", "either"),
    ("p_attnh1", "face_head", "either"), ("p_bao_durh", "face_head", "either"), ("p_bastilah04", "face_head", "female"),
    ("n_azkulh01", "face_head", "either"),
    ("c_bantha01", "creature_alien_droid", "either"), ("c_dewback01", "creature_alien_droid", "either"),
    ("c_drdmouse01", "creature_alien_droid", "either"), ("c_drdprot01", "creature_alien_droid", "either"),
    ("c_brith01", "creature_alien_droid", "either"),
    ("n_darkjedim01", "robe", "male"), ("n_darkjlowm01", "robe", "male"), ("n_czerkaoff01", "uniform", "male"),
    ("n_darthband01", "armor", "male"), ("n_child_m", "creature_alien_droid", "male"),
]
VANILLA_M = ["n_commm%02d" % i for i in range(1, 12)]
VANILLA_F = ["n_commf%02d" % i for i in range(1, 12)]


# ---- pure feature maths (unit-tested without game files) ----

def to_gray(rgb):
    a = np.asarray(rgb, dtype=np.float32)
    return 0.299 * a[..., 0] + 0.587 * a[..., 1] + 0.114 * a[..., 2]


def layout_mask(img, n=SIG, edge_q=0.6):
    """n x n binary-ish layout signature: downsample to n x n, gradient magnitude, 1 where above the edge_q quantile,
    blended with a non-flat (local deviation) term. Float32 in [0,1]."""
    g = to_gray(img.convert("RGB").resize((n, n), Image.BILINEAR))
    gy, gx = np.gradient(g)
    mag = np.hypot(gx, gy)
    thr = np.quantile(mag, edge_q)
    edge = (mag > max(thr, 1e-3)).astype(np.float32)
    return edge


def ncorr(a, b):
    a = a.astype(np.float64).ravel()
    b = b.astype(np.float64).ravel()
    a -= a.mean()
    b -= b.mean()
    d = np.sqrt((a * a).sum() * (b * b).sum())
    return float((a * b).sum() / d) if d > 0 else 0.0


def skin_fraction(img):
    a = np.asarray(img.convert("RGB"), dtype=np.int32)
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    mx, mn = a.max(axis=-1), a.min(axis=-1)
    m = (r > 95) & (g > 40) & (b > 20) & ((mx - mn) > 15) & (np.abs(r - g) > 15) & (r > g) & (r > b)
    return float(m.mean())


def mean_template(masks):
    return np.mean(np.stack(masks), axis=0)


def auc(pos, neg):
    """P(score_pos > score_neg) with ties 0.5 (Mann-Whitney)."""
    if not pos or not neg:
        return float("nan")
    w = 0.0
    for p in pos:
        for q in neg:
            w += 1.0 if p > q else 0.5 if p == q else 0.0
    return w / (len(pos) * len(neg))


def shuffled(items, seed):
    out = list(items)
    random.Random(seed).shuffle(out)
    return out


def chunk(items, n=SHEET_N):
    return [items[i:i + n] for i in range(0, len(items), n)]


def contact_sheet(tiles, labels, tile=TILE_PX, cols=SHEET_COLS, rows=SHEET_ROWS):
    """tiles: PIL images; labels: str per tile. Returns an RGB image cols*tile x rows*tile."""
    sheet = Image.new("RGB", (cols * tile, rows * tile), (40, 40, 40))
    d = ImageDraw.Draw(sheet)
    for i, (im, lab) in enumerate(zip(tiles, labels)):
        x, y = (i % cols) * tile, (i // cols) * tile
        sheet.paste(im.convert("RGB").resize((tile, tile), Image.LANCZOS), (x, y))
        d.rectangle([x, y, x + 8 * len(lab) + 8, y + 14], fill=(0, 0, 0))
        d.text((x + 4, y + 2), lab, fill=(255, 255, 0))
        d.rectangle([x, y, x + tile - 1, y + tile - 1], outline=(90, 90, 90))
    return sheet


# ---- game / file access ----

def _pool_paths():
    out = {}
    for p in sorted(OVERRIDE.iterdir()):
        n = p.stem.lower()
        if p.suffix.lower() in (".tpc", ".tga") and (n.startswith("n_commm") or n.startswith("n_commf")) \
                and len(n) == 9 and n[7:].isdigit():
            out[n] = p
    return out


def _tpc_to_image(t):
    from pykotor.resource.formats.tpc import TPCTextureFormat
    t.convert(TPCTextureFormat.RGBA)
    m = t.get(0, 0)
    return Image.frombytes("RGBA", (m.width, m.height), bytes(m.data)).transpose(Image.FLIP_TOP_BOTTOM), m.width, m.height


def load_file(path):
    """(PIL RGBA image, fmt label, w, h) for a .tpc/.tga file."""
    if path.suffix.lower() == ".tga":
        im = Image.open(path).convert("RGBA")
        return im, "tga", im.width, im.height
    from pykotor.resource.formats.tpc import read_tpc
    t = read_tpc(str(path))
    fmt = str(getattr(t, "format", lambda: "?")()) if callable(getattr(t, "format", None)) else "tpc"
    im, w, h = _tpc_to_image(t)
    return im, fmt, w, h


_PACK = None


def _pack_index():
    global _PACK
    if _PACK is None:
        import logging
        logging.disable(logging.CRITICAL)
        from pykotor.resource.formats.erf import read_erf
        _PACK = {}
        for f in sorted((GAME / "TexturePacks").glob("swpc_tex_*.erf")):
            if "gui" in f.name:
                continue
            e = read_erf(str(f))
            for r in e:
                _PACK.setdefault(str(r.resref).lower(), (f, r))
    return _PACK


def load_game(name):
    """(image, fmt, w, h) from the vanilla texture packs, or None."""
    hit = _pack_index().get(name)
    if hit is None:
        return None
    from pykotor.resource.formats.tpc import read_tpc
    data = hit[1].data
    t = read_tpc(bytes(data))
    im, w, h = _tpc_to_image(t)
    return im, "tpc", w, h


def features_for(img, fmt, w, h):
    a = np.asarray(img.convert("RGBA"), dtype=np.float32)
    mean = a[..., :3].reshape(-1, 3).mean(axis=0)
    return {"w": w, "h": h, "fmt": fmt, "alpha_mean": round(float(a[..., 3].mean() / 255.0), 3),
            "mean_rgb": [int(round(x)) for x in mean], "skin_frac": round(skin_fraction(img), 3)}


def build(out, seed, limit=0):
    out = Path(out)
    (out / "png").mkdir(parents=True, exist_ok=True)
    (out / "sheets").mkdir(parents=True, exist_ok=True)
    entries = {}   # name -> dict(kind, gender, truth, img)
    pool = _pool_paths()
    for n, p in pool.items():
        im, fmt, w, h = load_file(p)
        entries[n] = dict(kind="pool", gender="male" if n.startswith("n_commm") else "female", truth=None,
                          img=im, fmt=fmt, w=w, h=h, src=p.name)
    # vanilla refs and decoys (the vanilla n_commm01-11 live in the packs; the override copies are 2x re-skins)
    for n in VANILLA_M + VANILLA_F:
        g = load_game(n)
        if g:
            entries["vanilla:" + n] = dict(kind="vanilla", gender="male" if n.startswith("n_commm") else "female",
                                           truth="commoner_clothing", img=g[0], fmt=g[1], w=g[2], h=g[3], src="pack")
    for n, truth, gender in DECOYS:
        g = load_game(n)
        if g:
            entries["decoy:" + n] = dict(kind="decoy", gender=gender, truth=truth, img=g[0], fmt=g[1], w=g[2], h=g[3],
                                         src="pack")
        else:
            print("WARN decoy missing from packs:", n, file=sys.stderr)
    if limit:
        entries = dict(list(entries.items())[:limit])

    masks = {k: layout_mask(v["img"]) for k, v in entries.items()}
    templ = {}
    for g in ("male", "female"):
        vm = [masks[k] for k, v in entries.items() if v["kind"] == "vanilla" and v["gender"] == g]
        templ[g] = vm
    feats = {}
    for k, v in entries.items():
        f = features_for(v["img"], v["fmt"], v["w"], v["h"])
        gs = ["male", "female"] if v["gender"] == "either" else [v["gender"]]
        best = None
        for g in gs:
            refs = [m for kk, m in ((kk, masks[kk]) for kk, vv in entries.items()
                                    if vv["kind"] == "vanilla" and vv["gender"] == g and kk != k)]
            if refs:
                c = ncorr(masks[k], mean_template(refs))
                best = c if best is None else max(best, c)
        f.update(kind=v["kind"], gender=v["gender"], truth=v["truth"], src=v["src"],
                 uv_corr=None if best is None else round(best, 3))
        feats[k] = f
        safe = k.replace(":", "__")
        v["img"].convert("RGB").resize((PNG_PX, PNG_PX), Image.LANCZOS).save(out / "png" / (safe + ".png"))
    (out / "features.json").write_text(json.dumps(feats, indent=1, sort_keys=True))

    # contact sheets: pool only, shuffled, labelled by slot; calibration sheets anonymised with a key
    sheets = {}
    pool_names = shuffled(sorted(n for n, v in entries.items() if v["kind"] == "pool"), seed)
    for i, grp in enumerate(chunk(pool_names)):
        path = out / "sheets" / ("pool_%02d.png" % i)
        contact_sheet([entries[n]["img"] for n in grp], [n[2:].upper() for n in grp]).save(path)
        sheets[path.name] = grp
    calib = shuffled(sorted(n for n, v in entries.items() if v["kind"] in ("decoy", "vanilla")), seed + 1)
    key = {}
    for i, grp in enumerate(chunk(calib)):
        path = out / "sheets" / ("calib_%02d.png" % i)
        labs = ["C%02d" % (i * SHEET_N + j + 1) for j in range(len(grp))]
        contact_sheet([entries[n]["img"] for n in grp], labs).save(path)
        for lab, n in zip(labs, grp):
            key[lab] = {"name": n, "truth": entries[n]["truth"], "kind": entries[n]["kind"]}
        sheets[path.name] = labs
    (out / "calib_key.json").write_text(json.dumps(key, indent=1, sort_keys=True))
    (out / "sheets.json").write_text(json.dumps(sheets, indent=1, sort_keys=True))
    print("built %d textures, %d sheets -> %s" % (len(entries), len(sheets), out))
    report(out)


def report(out):
    feats = json.loads((Path(out) / "features.json").read_text())
    van = [f["uv_corr"] for f in feats.values() if f["kind"] == "vanilla" and f["uv_corr"] is not None]
    dec = [f["uv_corr"] for f in feats.values() if f["kind"] == "decoy" and f["uv_corr"] is not None]
    pool = [(k, f["uv_corr"]) for k, f in feats.items() if f["kind"] == "pool" and f["uv_corr"] is not None]
    print("vanilla n=%d mean=%.3f | decoys n=%d mean=%.3f | AUC(vanilla>decoy)=%.3f" % (
        len(van), np.mean(van) if van else float("nan"), len(dec), np.mean(dec) if dec else float("nan"), auc(van, dec)))
    if pool:
        lo = sorted(pool, key=lambda kv: kv[1])[:5]
        print("lowest pool uv_corr:", ", ".join("%s=%.2f" % kv for kv in lo))


def spotcheck(out, labels_path=None, tile=160, cols=7):
    """ACCEPT / REJECT overview PNGs from texture_labels.json for the human spot-check (slot numbers on each tile)."""
    out = Path(out)
    lp = Path(labels_path) if labels_path else REPO / "patch_manager_mods/npc_diversity_src/texture_labels.json"
    slots = json.loads(lp.read_text())["slots"]
    groups = {"ACCEPT": [], "REJECT": []}
    for n, v in sorted(slots.items(), key=lambda kv: (kv[1]["gender"], kv[1]["slot"])):
        groups["ACCEPT" if v["keep"] else "REJECT"].append((n, v))
    for kind, items in groups.items():
        if not items:
            continue
        rows = -(-len(items) // cols)
        tiles, labels = [], []
        for n, v in items:
            tiles.append(Image.open(out / "png" / (n + ".png")))
            tag = ("%s%02d" % (v["gender"], v["slot"])) + ("*" if v["review"] else "")
            labels.append("%s %s" % (tag, v["category"][:10]))
        sheet = contact_sheet(tiles, labels, tile=tile, cols=cols, rows=rows)
        path = out / "sheets" / ("spot_%s.png" % kind.lower())
        sheet.save(path)
        print("%s: %d tiles -> %s (* = flagged for review)" % (kind, len(items), path))


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("cmd", choices=("build", "report", "spotcheck"))
    ap.add_argument("--out", default=str(DEFAULT_OUT))
    ap.add_argument("--seed", type=int, default=20261011)
    ap.add_argument("--limit", type=int, default=0)
    a = ap.parse_args(argv)
    if a.cmd == "build":
        build(a.out, a.seed, a.limit)
    elif a.cmd == "spotcheck":
        spotcheck(a.out)
    else:
        report(a.out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
