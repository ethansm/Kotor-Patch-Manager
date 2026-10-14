#!/usr/bin/env python3
"""Check whether Characters Revisited (CR) TGAs keep the K1 vanilla UV layout (not K2's).

Candidate rows come from .claude/waves/w9-38-CR/cr_vs_base_<shard>.tsv:
  k2pack_in_k1  base_src=K2pack, status=palette, stem is a K1 TPC (asset_diff_all.tsv, type=TPC, k1_hash set)
  control       base_src=K1pack, status in (palette, identical)

Two stages, because pykotor lives in its own venv (no numpy/PIL) and numpy/PIL live in the repo venv:
  stage 1 (k1port venv, -I): read each K1 TPC through k1_port_tex_preview, write raw bytes to scratch.
  stage 2 (repo .venv, -I):  decode the K1 TPC (k1_port_tex_tpc) and the CR TGA (k1_port_cr_sheets.load_cr),
                             downsample CR to the K1 size as k1_port_cr_sheets does, then mad /
                             layout_corr / classify_status (imported from k1_port_cr_sheets, not copied).

Run: python3 scripts/cr_k1_layout_check.py   (orchestrator; stdlib only, re-invokes both stages)
Output: .claude/research/k1_items/cr_k1_layout.tsv
"""
import shutil
import subprocess
import sys
import tempfile
from collections import Counter
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
K1PORT_DIR = REPO / "patch_manager_mods" / "k1_port"
WAVE_DIR = REPO / ".claude" / "waves" / "w9-38-CR"
ASSET_DIFF = REPO / ".claude" / "research" / "k1_items" / "asset_diff_all.tsv"
OUT_TSV = REPO / ".claude" / "research" / "k1_items" / "cr_k1_layout.tsv"
CR_ROOT = Path.home() / "Games" / "KOTOR2_Mod_Workspace" / "k1_port_extract" / "characters_revisited"
K1_GAME = Path.home() / ".local" / "share" / "Steam" / "steamapps" / "common" / "swkotor"
K1_VENV_PY = Path.home() / "Games" / "KOTOR2_Mod_Workspace" / "venvs" / "k1port" / "bin" / "python"
REPO_VENV_PY = REPO / ".venv" / "bin" / "python"

HEADER = ["stem", "shard", "cr_dims", "k1_dims", "corr_vs_k1", "status", "set"]
SET_K2 = "k2pack_in_k1"
SET_CONTROL = "control"
CONTROL_STATUSES = ("palette", "identical")

sys.path.insert(0, str(K1PORT_DIR))
import k1_port_cr_sheets as cs  # noqa: E402  stdlib-only at import time; numpy/PIL/pykotor load lazily


def shard_of(tsv_path):
    """'cr_vs_base_body_player.tsv' -> 'body_player'."""
    name = Path(tsv_path).name
    prefix, suffix = "cr_vs_base_", ".tsv"
    if not (name.startswith(prefix) and name.endswith(suffix)):
        raise ValueError(f"not a cr_vs_base_<shard>.tsv name: {name}")
    return name[len(prefix):-len(suffix)]


def read_tsv(path):
    """Rows as dicts keyed by the header line. Blank lines skipped; short rows yield missing keys."""
    with open(path, encoding="utf-8") as fh:
        lines = [ln.rstrip("\n") for ln in fh if ln.strip()]
    if not lines:
        return []
    header = lines[0].split("\t")
    return [dict(zip(header, ln.split("\t"))) for ln in lines[1:]]


def k1_tpc_stems(asset_diff_path):
    """Lower-cased resrefs with type=TPC and a non-empty k1_hash."""
    return {
        (row.get("resref") or "").strip().lower()
        for row in read_tsv(asset_diff_path)
        if row.get("type") == "TPC" and (row.get("k1_hash") or "")
    }


def build_rows(wave_dir, asset_diff_path):
    """(k2pack_in_k1 rows, control rows); each row is (stem, shard, set), de-duplicated, file order kept."""
    k1_stems = k1_tpc_stems(asset_diff_path)
    k2_rows, control_rows = [], []
    for tsv in sorted(Path(wave_dir).glob("cr_vs_base_*.tsv")):
        shard = shard_of(tsv)
        for row in read_tsv(tsv):
            stem = (row.get("stem") or "").strip()
            if not stem:
                continue
            base_src, status = row.get("base_src"), row.get("status")
            if base_src == "K2pack" and status == "palette" and stem.lower() in k1_stems:
                k2_rows.append((stem, shard, SET_K2))
            elif base_src == "K1pack" and status in CONTROL_STATUSES:
                control_rows.append((stem, shard, SET_CONTROL))
    return list(dict.fromkeys(k2_rows)), list(dict.fromkeys(control_rows))


def cr_tga_path(cr_root, shard, stem):
    return Path(cr_root) / shard / f"{stem}.tga"


def format_row(stem, shard, cr_dims, k1_dims, corr, status, set_name):
    corr_txt = "-" if corr is None else f"{corr:.4f}"
    return "\t".join([stem, shard, cr_dims or "-", k1_dims or "-", corr_txt, status, set_name])


def decode_k1_tpc(tpc_bytes):
    """RGBA image of K1 TPC mip 0, flipped to top-down like k1_port_cr_sheets._tpc_to_image."""
    import numpy as np
    from PIL import Image
    import k1_port_tex_tpc as tpc

    info = tpc.parse(tpc_bytes)
    arr = tpc.decode_mip(tpc_bytes, info, 0)
    return Image.fromarray(np.ascontiguousarray(arr[::-1]))


def stage1(list_path, scratch):
    """k1port venv: write raw K1 TPC bytes for each stem in list_path to scratch/<stem>.tpc."""
    import k1_port_tex_preview as tp

    pk, inst = tp.open_install(K1_GAME)
    found = missing = 0
    for stem in Path(list_path).read_text(encoding="utf-8").split():
        try:
            res = tp._texture_get(pk, inst, stem, pk.ResourceType.TPC)
        except Exception as exc:  # keep going; the stem is reported as missing
            print(f"stage1 error {stem}: {exc}", file=sys.stderr, flush=True)
            res = None
        if res is None:
            missing += 1
            continue
        (Path(scratch) / f"{stem}.tpc").write_bytes(bytes(res.data))
        found += 1
    print(f"stage1: found={found} missing={missing}", flush=True)
    return 0


def stage2(rows_path, tpc_dir, out_path):
    """repo .venv: classify each row. Rows with a missing CR TGA are skipped and counted."""
    from PIL import Image

    rows = [ln.split("\t") for ln in Path(rows_path).read_text(encoding="utf-8").splitlines() if ln]
    out = ["\t".join(HEADER)]
    skipped = Counter()
    for stem, shard, set_name in rows:
        cr_path = cr_tga_path(CR_ROOT, shard, stem)
        if not cr_path.is_file():
            skipped["missing_cr"] += 1
            continue
        try:
            cr_img = cs.load_cr(cr_path)
        except Exception as exc:
            print(f"stage2 cr decode {shard}/{stem}: {exc}", file=sys.stderr, flush=True)
            skipped["cr_decode"] += 1
            continue
        cr_dims = f"{cr_img.width}x{cr_img.height}"

        k1_img = None
        tpc_path = Path(tpc_dir) / f"{stem}.tpc"
        if tpc_path.is_file():
            try:
                k1_img = decode_k1_tpc(tpc_path.read_bytes())
            except Exception as exc:  # mirrors load_k1: a decode failure means no K1 base
                print(f"stage2 k1 decode {stem}: {exc}", file=sys.stderr, flush=True)
        if k1_img is None:
            status = cs.classify_status(None, None, False)  # "nobase"
            out.append(format_row(stem, shard, cr_dims, None, None, status, set_name))
            continue

        kw, kh = k1_img.size
        cr_small = cr_img.resize((kw, kh), Image.LANCZOS)  # same resample as k1_port_cr_sheets._rgb_array
        mad_v = cs.mad(cr_small, k1_img, size=kw)
        corr = cs.layout_corr(cr_small, k1_img)
        status = cs.classify_status(mad_v, corr, True)
        out.append(format_row(stem, shard, cr_dims, f"{kw}x{kh}", corr, status, set_name))

    Path(out_path).write_text("\n".join(out) + "\n", encoding="utf-8")
    print(f"stage2: rows_in={len(rows)} rows_out={len(out) - 1} skipped={dict(skipped)}", flush=True)
    return 0


def _run(cmd, label):
    proc = subprocess.run(cmd, check=False)
    if proc.returncode != 0:
        raise SystemExit(f"{label} failed with exit {proc.returncode}")


def _summary(out_path, rows_in):
    lines = Path(out_path).read_text(encoding="utf-8").splitlines()[1:]
    counts = Counter((ln.split("\t")[6], ln.split("\t")[5]) for ln in lines if ln)
    parts = []
    for set_name in (SET_K2, SET_CONTROL):
        per = " ".join(f"{st}={n}" for (s, st), n in sorted(counts.items()) if s == set_name)
        parts.append(f"{set_name}[{per or 'none'}]")
    return f"cr_k1_layout rows_in={rows_in} rows_out={len(lines)} " + " ".join(parts) + f" -> {OUT_TSV}"


def orchestrate():
    k2_rows, control_rows = build_rows(WAVE_DIR, ASSET_DIFF)
    rows = k2_rows + control_rows
    if not rows:
        raise SystemExit("no candidate rows found under " + str(WAVE_DIR))
    scratch = Path(tempfile.mkdtemp(prefix="cr_k1_layout_"))
    try:
        stems = sorted({stem for stem, _, _ in rows})
        list_file = scratch / "stems.txt"
        list_file.write_text("\n".join(stems) + "\n", encoding="utf-8")
        rows_file = scratch / "rows.tsv"
        rows_file.write_text("".join(f"{s}\t{sh}\t{st}\n" for s, sh, st in rows), encoding="utf-8")
        tpc_dir = scratch / "k1_tpc"
        tpc_dir.mkdir()
        _run([str(K1_VENV_PY), "-I", __file__, "--stage1", str(list_file), str(tpc_dir)], "stage1")
        out_tmp = scratch / "cr_k1_layout.tsv"
        _run([str(REPO_VENV_PY), "-I", __file__, "--stage2", str(rows_file), str(tpc_dir), str(out_tmp)], "stage2")
        OUT_TSV.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(out_tmp, OUT_TSV)
    finally:
        shutil.rmtree(scratch, ignore_errors=True)
    print(_summary(OUT_TSV, len(rows)), flush=True)
    return 0


def main(argv):
    if len(argv) == 4 and argv[1] == "--stage1":
        return stage1(argv[2], argv[3])
    if len(argv) == 5 and argv[1] == "--stage2":
        return stage2(argv[2], argv[3], argv[4])
    return orchestrate()


if __name__ == "__main__":
    sys.exit(main(sys.argv))
