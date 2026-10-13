#!/usr/bin/env python3
"""Compare Characters Revisited (CR) 2048px TGAs with the K1 and K2 base textures.

Subcommands (S = shard dir name under the CR dir, or 'all' for every subdirectory):
  diff   --shard S [--limit N]  -> <out>/cr_vs_base_<S>.tsv, one row per *.tga
  sheets --shard S              -> <out>/sheets/<S>_NN.png (rows K1 | K2 | CR, 5 per sheet)
                                   plus rows appended to <out>/sheets_index.tsv

Env overrides: K1PORT_CR_DEST (CR dir), K1PORT_CR_OUT (output dir).
Run: .venv/bin/python -u k1_port_cr_sheets.py diff --shard S

numpy, Pillow and pykotor are imported lazily inside functions so this module
loads under a bare python3.
"""
import argparse
import os
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
HOME = Path.home()
CR_DIR_DEFAULT = HOME / "Games/KOTOR2_Mod_Workspace/k1_port_extract/characters_revisited"
K1_GAME = HOME / ".local/share/Steam/steamapps/common/swkotor"
K2_GAME = HOME / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
OUT_REL = ".claude/waves/w9-38-CR"

PROGRESS_EVERY = 25
SHEET_N = 5
PANEL = 256
GREY = (128, 128, 128)
TSV_HEADER = "stem\tbase_src\tcr_dims\tbase_dims\tmad\thist_dist\tcorr\tstatus"
TSV_COLS = TSV_HEADER.split("\t")
STATUSES = ("identical", "palette", "redesign", "nobase", "error")

_PACKS = {}  # game dir str -> {resref: resource}; built once per game dir


# ---- paths ----

def repo_root(start=HERE):
    for p in [start, *start.parents]:
        if (p / ".git").exists():
            return p
    return start


def cr_dir_default():
    env = os.environ.get("K1PORT_CR_DEST")
    return Path(env).expanduser() if env else CR_DIR_DEFAULT


def out_dir_default():
    env = os.environ.get("K1PORT_CR_OUT")
    return Path(env).expanduser() if env else repo_root() / OUT_REL


# ---- pure functions ----

def _rgb_array(img, size=None):
    import numpy as np
    from PIL import Image
    im = img.convert("RGB")
    if size is not None:
        im = im.resize((size, size), Image.LANCZOS)
    return np.asarray(im, dtype=np.float64)


def mad(a, b, size=128):
    """Mean absolute RGB difference after Lanczos-resizing both to size x size (0..255)."""
    import numpy as np
    return float(np.mean(np.abs(_rgb_array(a, size) - _rgb_array(b, size))))


def _hist_vec(img):
    import numpy as np
    px = np.asarray(img.convert("RGB"), dtype=np.uint8).reshape(-1, 3)
    counts = np.concatenate(
        [np.histogram(px[:, c], bins=16, range=(0, 256))[0] for c in range(3)]
    ).astype(np.float64)
    total = counts.sum()
    return counts / total if total else counts


def hist_dist(a, b):
    """L1 distance between normalised 16-bin-per-channel RGB histograms (0..2)."""
    import numpy as np
    return float(np.abs(_hist_vec(a) - _hist_vec(b)).sum())


def _edge_map(img, size=64):
    import numpy as np
    from PIL import Image
    g = np.asarray(img.convert("L").resize((size, size), Image.LANCZOS), dtype=np.float64)
    dx = np.pad(np.diff(g, axis=1), ((0, 0), (0, 1)))
    dy = np.pad(np.diff(g, axis=0), ((0, 1), (0, 0)))
    return np.hypot(dx, dy).ravel()


def layout_corr(a, b):
    """Pearson correlation of 64x64 grayscale gradient-magnitude maps (-1..1); 0 if either is flat."""
    import numpy as np
    x = _edge_map(a)
    y = _edge_map(b)
    x = x - x.mean()
    y = y - y.mean()
    den = float(np.sqrt(float((x * x).sum()) * float((y * y).sum())))
    if den < 1e-12:
        return 0.0
    return float(max(-1.0, min(1.0, float((x * y).sum()) / den)))


def classify_status(mad_v, corr, has_base):
    if not has_base:
        return "nobase"
    if mad_v < 2:
        return "identical"
    if corr >= 0.9:
        return "palette"
    return "redesign"


def chunk_rows(items, n=SHEET_N):
    items = list(items)
    return [items[i:i + n] for i in range(0, len(items), n)]


def _panel(img, size):
    from PIL import Image
    if img is None:
        return Image.new("RGB", (size, size), GREY)
    return img.convert("RGB").resize((size, size), Image.LANCZOS)


def make_row(k1, k2, cr, stem, panel=PANEL):
    """K1 | K2 | CR panels side by side (3*panel x panel); None panel = grey; yellow stem label top-left."""
    from PIL import Image, ImageDraw
    row = Image.new("RGB", (3 * panel, panel), GREY)
    for i, im in enumerate((k1, k2, cr)):
        row.paste(_panel(im, panel), (i * panel, 0))
    d = ImageDraw.Draw(row)
    box = d.textbbox((4, 2), stem)
    d.rectangle([0, 0, box[2] + 4, box[3] + 2], fill=(0, 0, 0))
    d.text((4, 2), stem, fill=(255, 255, 0))
    return row


def make_sheet(rows):
    """Stack 1..SHEET_N row images vertically."""
    from PIL import Image
    rows = list(rows)
    if not 1 <= len(rows) <= SHEET_N:
        raise ValueError(f"make_sheet takes 1..{SHEET_N} rows, got {len(rows)}")
    w = max(r.width for r in rows)
    h = sum(r.height for r in rows)
    sheet = Image.new("RGB", (w, h), (0, 0, 0))
    y = 0
    for r in rows:
        sheet.paste(r.convert("RGB"), (0, y))
        y += r.height
    return sheet


# ---- decode and base loaders (tests monkeypatch load_k1 / load_k2) ----

def _tpc_to_image(t):
    from pykotor.resource.formats.tpc import TPCTextureFormat
    from PIL import Image
    t.convert(TPCTextureFormat.RGBA)
    m = t.get(0, 0)
    return Image.frombytes("RGBA", (m.width, m.height), bytes(m.data)).transpose(Image.FLIP_TOP_BOTTOM)


def _decode_path(path):
    from PIL import Image
    if path.suffix.lower() == ".tga":
        return Image.open(path).convert("RGBA")
    from pykotor.resource.formats.tpc import read_tpc
    return _tpc_to_image(read_tpc(str(path)))


def _pack(game_dir):
    """{resref: resource} over TexturePacks swpc_tex_*.erf (no 'gui'). Built once, cached."""
    key = str(game_dir)
    if key not in _PACKS:
        import logging
        logging.disable(logging.CRITICAL)
        from pykotor.resource.formats.erf import read_erf
        pack = {}
        for f in sorted((Path(game_dir) / "TexturePacks").glob("swpc_tex_*.erf")):
            if "gui" in f.name:
                continue
            try:
                for r in read_erf(str(f)):
                    pack.setdefault(str(r.resref).lower(), r)
            except Exception as exc:
                print(f"warn: cannot read {f}: {exc}", file=sys.stderr, flush=True)
        _PACKS[key] = pack
    return _PACKS[key]


def load_k1(stem):
    """K1 base texture from the K1 packs as an RGBA image, or None (decode failure included)."""
    res = _pack(K1_GAME).get(stem)
    if res is None:
        return None
    try:
        from pykotor.resource.formats.tpc import read_tpc
        return _tpc_to_image(read_tpc(bytes(res.data)))
    except Exception:
        return None


def load_k2(stem):
    """(image or None, src) with src 'K2pack' | 'K2override' | None. Pack first, then override/<stem>.tpc|.tga."""
    res = _pack(K2_GAME).get(stem)
    if res is not None:
        try:
            from pykotor.resource.formats.tpc import read_tpc
            return _tpc_to_image(read_tpc(bytes(res.data))), "K2pack"
        except Exception:
            pass
    for ext in (".tpc", ".tga"):
        p = K2_GAME / "override" / f"{stem}{ext}"
        if p.is_file():
            try:
                return _decode_path(p), "K2override"
            except Exception:
                return None, None
    return None, None


def load_cr(path):
    return _decode_path(path)


def _base_for(stem):
    """(image or None, base_src): K2pack | K2override | K1pack | none."""
    img, src = load_k2(stem)
    if img is not None:
        return img, src
    img = load_k1(stem)
    if img is not None:
        return img, "K1pack"
    return None, "none"


# ---- shards ----

def shard_names(cr_dir, shard):
    if shard == "all":
        return sorted(p.name for p in cr_dir.iterdir() if p.is_dir() and not p.name.startswith("."))
    return [shard]


def shard_dir(cr_dir, shard):
    d = cr_dir / shard
    if not d.is_dir():
        raise SystemExit(f"error: shard dir missing: {d}")
    return d


def shard_tgas(sdir):
    """{stem (lowercase): path} for *.tga in the shard dir, sorted by stem."""
    files = sorted((p for p in sdir.glob("*.tga") if p.is_file()), key=lambda p: p.stem.lower())
    return {p.stem.lower(): p for p in files}


def _dims(img):
    return f"{img.width}x{img.height}" if img is not None else "-"


def _diff_one(stem, path):
    base, src = _base_for(stem)
    base_dims = _dims(base)
    try:
        cr = load_cr(path)
    except Exception:
        return [stem, src, "-", base_dims, "-", "-", "-", "error"]
    cr_dims = _dims(cr)
    if base is None:
        return [stem, src, cr_dims, base_dims, "-", "-", "-", "nobase"]
    m = mad(cr, base)
    h = hist_dist(cr, base)
    c = layout_corr(cr, base)
    status = classify_status(m, c, True)
    return [stem, src, cr_dims, base_dims, f"{m:.4f}", f"{h:.6f}", f"{c:.6f}", status]


def diff_shard(cr_dir, shard, out_dir, limit=None):
    files = shard_tgas(shard_dir(cr_dir, shard))
    items = list(files.items())
    if limit:
        items = items[:limit]
    total = len(items)
    counts = {s: 0 for s in STATUSES}
    lines = [TSV_HEADER]
    for i, (stem, path) in enumerate(items, 1):
        row = _diff_one(stem, path)
        counts[row[-1]] += 1
        lines.append("\t".join(row))
        if i % PROGRESS_EVERY == 0 or i == total:
            print(f"[diff {shard}] {i}/{total}", flush=True)
    out_dir.mkdir(parents=True, exist_ok=True)
    tsv = out_dir / f"cr_vs_base_{shard}.tsv"
    tsv.write_text("\n".join(lines) + "\n", encoding="utf-8")
    summary = " ".join(f"{s}={counts[s]}" for s in STATUSES)
    print(f"[diff {shard}] wrote {tsv} ({total} files) {summary}", flush=True)
    return counts


def _read_tsv(tsv):
    rows = []
    with open(tsv, encoding="utf-8") as f:
        next(f)  # header
        for line in f:
            line = line.rstrip("\n")
            if line:
                rows.append(dict(zip(TSV_COLS, line.split("\t"))))
    return rows


def _update_index(index, shard, new_rows):
    """Drop this shard's old rows from sheets_index.tsv, then append new_rows."""
    pat = re.compile(rf"^sheets/{re.escape(shard)}_\d+\.png\t")
    kept = []
    if index.is_file():
        kept = [ln for ln in index.read_text(encoding="utf-8").splitlines() if ln and not pat.match(ln)]
    index.write_text("".join(f"{ln}\n" for ln in kept + new_rows), encoding="utf-8")


def sheets_shard(cr_dir, shard, out_dir):
    sdir = shard_dir(cr_dir, shard)
    tsv = out_dir / f"cr_vs_base_{shard}.tsv"
    if not tsv.is_file():
        raise SystemExit(f"error: {tsv} missing; run diff --shard {shard} first")
    tgas = shard_tgas(sdir)
    rows = sorted((r for r in _read_tsv(tsv) if r["status"] != "identical"), key=lambda r: r["stem"])
    total = len(rows)
    sheets_dir = out_dir / "sheets"
    sheets_dir.mkdir(parents=True, exist_ok=True)
    for old in sheets_dir.iterdir():
        if re.fullmatch(rf"{re.escape(shard)}_\d+\.png", old.name):
            old.unlink()
    index_rows = []
    done = 0
    for n, group in enumerate(chunk_rows(rows, SHEET_N), 1):
        row_imgs = []
        for r in group:
            stem = r["stem"]
            k1 = load_k1(stem)
            k2, _src = load_k2(stem)
            cr = None
            if stem in tgas:
                try:
                    cr = load_cr(tgas[stem])
                except Exception:
                    cr = None
            row_imgs.append(make_row(k1, k2, cr, stem))
            done += 1
            if done % PROGRESS_EVERY == 0 or done == total:
                print(f"[sheets {shard}] {done}/{total}", flush=True)
        name = f"{shard}_{n:02d}.png"
        make_sheet(row_imgs).save(sheets_dir / name)
        index_rows.append(f"sheets/{name}\t{','.join(r['stem'] for r in group)}")
    _update_index(out_dir / "sheets_index.tsv", shard, index_rows)
    print(f"[sheets {shard}] wrote {len(index_rows)} sheet(s) for {total} rows", flush=True)
    return len(index_rows)


# ---- CLI ----

def main(argv=None):
    ap = argparse.ArgumentParser(description="CR vs K1/K2 base texture diff and contact sheets")
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("diff", "sheets"):
        sp = sub.add_parser(name)
        sp.add_argument("--shard", required=True, help="shard dir name, or 'all'")
        sp.add_argument("--cr-dir", default=None, help="CR dir (default: K1PORT_CR_DEST or built-in)")
        sp.add_argument("--out", default=None, help="output dir (default: K1PORT_CR_OUT or .claude/waves/w9-38-CR)")
        if name == "diff":
            sp.add_argument("--limit", type=int, default=None, help="first N files per shard")
    args = ap.parse_args(argv)
    cr_dir = Path(args.cr_dir).expanduser() if args.cr_dir else cr_dir_default()
    out_dir = Path(args.out).expanduser() if args.out else out_dir_default()
    if not cr_dir.is_dir():
        raise SystemExit(f"error: CR dir missing: {cr_dir}")
    for shard in shard_names(cr_dir, args.shard):
        if args.cmd == "diff":
            diff_shard(cr_dir, shard, out_dir, args.limit)
        else:
            sheets_shard(cr_dir, shard, out_dir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
