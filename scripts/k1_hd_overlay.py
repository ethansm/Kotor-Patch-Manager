#!/usr/bin/env python3
"""Build and check the K1 HD texture overlay (Characters Revisited TGA/TXI copies) for the K1 port.

  k1_hd_overlay.py build --k1 <install> --cr <CR extract> --select <tsv> [--select more.tsv ...] --out <dir> [--rebuild]
  k1_hd_overlay.py check --out <dir>

build writes into <out>:
  <name>            symlink to <install>/<name> for LINK_NAMES only (never Override; missing names are skipped)
  override/<stem>.tga  REAL copy from the CR extract (required per selected stem)
  override/<stem>.txi  REAL copy from the CR extract (optional: skipped when the manifest has no txi row)
  K1_HD_OVERLAY.json   marker, written last; overlay_manifest.tsv lists the copies

CR files are located through <CR>/manifest.tsv (stem ext size sha256 shard) at <CR>/<shard>/<stem>.<ext>,
falling back to a case-insensitive recursive search of <CR>/<shard>/. Every copy is verified by sha256.

Exit codes: build 0 ok; 2 refused or bad input (nothing written); 1 a copy was not found or failed sha256.
            check 0 OK; 1 problems listed on stdout.
"""
import argparse
import csv
import hashlib
import json
import os
import shutil
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
MARKER = "K1_HD_OVERLAY.json"
OVERLAY_MANIFEST = "overlay_manifest.tsv"
OVERRIDE = "override"
SOURCE_ID = "characters_revisited_main"
LINK_NAMES = ("chitin.key", "dialog.tlk", "data", "modules", "rims", "TexturePacks",
              "lips", "patch.erf", "swkotor.exe", "swkotor.ini")
STEAM_K1 = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"


class Refusal(Exception):
    """Bad input or unsafe --out. Raised before anything is written."""


class BuildFailure(Exception):
    """A CR file could not be located or failed sha256 verification."""


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def read_tsv(path, required):
    with open(path, newline="", encoding="utf-8-sig") as f:
        reader = csv.DictReader(f, delimiter="\t")
        missing = [c for c in required if c not in (reader.fieldnames or [])]
        if missing:
            raise Refusal(f"{path} header lacks column(s): {', '.join(missing)}")
        return [{k: (v or "").strip() for k, v in row.items() if k} for row in reader]


def read_selection(paths):
    stems = set()
    for p in paths:
        for row in read_tsv(p, ("stem",)):
            stem = row["stem"]
            if not stem:
                continue
            if "/" in stem or "\\" in stem or stem in (".", ".."):
                raise Refusal(f"bad stem in {p}: {stem!r}")
            stems.add(stem)
    return sorted(stems)


def is_within(path, root):
    return path == root or root in path.parents


def has_steam_common(path):
    parts = path.parts
    return any(parts[i] == "steamapps" and parts[i + 1] == "common" for i in range(len(parts) - 1))


def check_out(out, k1):
    if is_within(out, REPO):
        raise Refusal(f"--out is inside the repo: {out}")
    if has_steam_common(out):
        raise Refusal(f"--out is under a steamapps/common path: {out}")
    if is_within(out, STEAM_K1.resolve()):
        raise Refusal(f"--out is inside the Steam K1 install: {out}")
    if is_within(out, k1):
        raise Refusal(f"--out equal to or inside --k1: {out}")
    if is_within(k1, out):
        raise Refusal(f"--k1 is inside --out, so --rebuild could delete the install: {out}")


def has_marker(out):
    m = out / MARKER
    return m.is_file() and not m.is_symlink()


def prepare_out(out, rebuild):
    if out.is_file():
        raise Refusal(f"--out is a file: {out}")
    if not out.is_dir():
        return
    if rebuild:
        if not has_marker(out):
            raise Refusal(f"--rebuild refused: {out} has no {MARKER}")
        shutil.rmtree(out)  # does not follow symlinks, so K1 files behind the links survive
    elif next(out.iterdir(), None) is not None:
        raise Refusal(f"--out exists and is not empty (use --rebuild on a marked overlay): {out}")


def locate(shard_dir, name):
    direct = shard_dir / name
    if direct.is_file():
        return direct
    hits = []
    if shard_dir.is_dir():
        wanted = name.lower()
        for dirpath, _dirs, files in os.walk(shard_dir):
            hits.extend(Path(dirpath) / f for f in files if f.lower() == wanted)
    return min(hits) if hits else None


def copy_verified(cr, row, override):
    ext = row["ext"].lower()
    name = f"{row['stem']}.{ext}"
    src = locate(cr / row["shard"], name)
    if src is None:
        raise BuildFailure(f"not found under {cr / row['shard']}: {name}")
    dst = override / name
    shutil.copyfile(src, dst)
    want = row["sha256"].lower()
    got = sha256_file(dst)
    if got != want:
        raise BuildFailure(f"sha256 mismatch for {name}: manifest {want or '(empty)'} copied {got}")
    return {"stem": row["stem"], "ext": ext, "sha256": got, "shard": row["shard"],
            "name": f"{OVERRIDE}/{name}"}


def cmd_build(args):
    k1 = Path(args.k1).resolve()
    cr = Path(args.cr).resolve()
    out = Path(args.out).resolve()
    select_paths = [Path(p).resolve() for p in args.select]
    if not k1.is_dir():
        raise Refusal(f"--k1 is not a directory: {k1}")
    if not (cr / "manifest.tsv").is_file():
        raise Refusal(f"--cr has no manifest.tsv: {cr}")
    for p in select_paths:
        if not p.is_file():
            raise Refusal(f"--select file not found: {p}")
    rows = read_tsv(cr / "manifest.tsv", ("stem", "ext", "size", "sha256", "shard"))
    stems = read_selection(select_paths)
    check_out(out, k1)
    prepare_out(out, args.rebuild)

    out.mkdir(parents=True, exist_ok=True)
    symlinks, skipped = {}, []
    for name in LINK_NAMES:
        src = k1 / name
        if src.exists():
            os.symlink(str(src), out / name)
            symlinks[name] = str(src)
        else:
            skipped.append(name)

    by_key = {}
    for row in rows:
        by_key.setdefault((row["stem"], row["ext"].lower()), []).append(row)

    def pick(stem, ext, shard=None):
        """One manifest row. A stem in several shards takes the CR `updates` shard (the newer files), else the
        first listed; a .txi follows its .tga's shard when it has one there."""
        cands = by_key.get((stem, ext), [])
        for want in (shard, "updates"):
            for row in cands:
                if want and row["shard"] == want:
                    return row
        return cands[0] if cands else None
    override = out / OVERRIDE
    override.mkdir()
    copies, missing = [], []
    for stem in stems:
        tga = pick(stem, "tga")
        if tga is None:
            missing.append(stem)
            continue
        copies.append(copy_verified(cr, tga, override))
        txi = pick(stem, "txi", tga["shard"])
        if txi is not None:
            copies.append(copy_verified(cr, txi, override))

    marker = {
        "source_id": SOURCE_ID,
        "flagged": True,
        "license_class": "unknown",
        "select_sha256": hashlib.sha256("\n".join(stems).encode("utf-8")).hexdigest(),
        "files": {c["name"]: c["sha256"] for c in copies},
        "symlinks": symlinks,
        "missing": missing,
        "skipped": skipped,
    }
    (out / MARKER).write_text(json.dumps(marker, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    with open(out / OVERLAY_MANIFEST, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, delimiter="\t", lineterminator="\n")
        w.writerow(["stem", "ext", "sha256", "shard"])
        for c in sorted(copies, key=lambda c: (c["stem"], c["ext"])):
            w.writerow([c["stem"], c["ext"], c["sha256"], c["shard"]])
    print(f"built {out}: symlinks={len(symlinks)} copies={len(copies)} "
          f"missing={len(missing)} skipped={len(skipped)}")
    return 0


def walk_no_follow(root):
    for entry in sorted(os.scandir(root), key=lambda e: e.name):
        p = Path(entry.path)
        yield p
        if entry.is_dir(follow_symlinks=False):
            yield from walk_no_follow(p)


def cmd_check(args):
    out = Path(args.out).resolve()
    try:
        marker = json.loads((out / MARKER).read_text(encoding="utf-8"))
        files = marker["files"]
        links = marker["symlinks"]
        if not isinstance(files, dict) or not isinstance(links, dict):
            raise ValueError("files and symlinks must be objects")
    except (OSError, ValueError, KeyError, TypeError) as e:
        print(f"PROBLEM: no valid {MARKER} in {out}: {e}")
        return 1

    problems = []
    for name, want in sorted(files.items()):
        p = out / name
        if p.is_symlink():
            problems.append(f"symlink where copy expected: {name}")
        elif not p.is_file():
            problems.append(f"missing copy: {name}")
        elif sha256_file(p) != want:
            problems.append(f"modified copy: {name}")
    for name, target in sorted(links.items()):
        p = out / name
        if not p.is_symlink():
            problems.append(f"not a symlink: {name}")
        elif os.readlink(p) != target:
            problems.append(f"swapped symlink: {name} -> {os.readlink(p)} (marker: {target})")
        elif not p.exists():
            problems.append(f"dangling symlink: {name} -> {target}")
    ov = out / OVERRIDE
    if ov.is_symlink() or not ov.is_dir():
        problems.append(f"{OVERRIDE}/ missing or is a symlink")
    else:
        for p in walk_no_follow(ov):
            rel = p.relative_to(out).as_posix()
            if p.is_symlink():
                problems.append(f"symlink in {OVERRIDE}/: {rel}")
            elif p.is_dir():
                problems.append(f"directory in {OVERRIDE}/: {rel}")
            elif rel not in files:
                problems.append(f"file not in marker: {rel}")

    if problems:
        for pr in problems:
            print(f"PROBLEM: {pr}")
        return 1
    print(f"OK n_files={len(files)} n_links={len(links)}")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description="Build or check the K1 HD texture overlay.")
    sub = ap.add_subparsers(dest="cmd", required=True)
    b = sub.add_parser("build", help="create the overlay under --out")
    b.add_argument("--k1", required=True, help="K1 install dir (linked, never copied)")
    b.add_argument("--cr", required=True, help="Characters Revisited extract dir (holds manifest.tsv)")
    b.add_argument("--select", action="append", required=True, metavar="TSV",
                   help="TSV with a stem column; repeatable, stems are unioned")
    b.add_argument("--out", required=True)
    b.add_argument("--rebuild", action="store_true",
                   help="replace --out, only when it already holds K1_HD_OVERLAY.json")
    c = sub.add_parser("check", help="verify an overlay against its marker")
    c.add_argument("--out", required=True)
    args = ap.parse_args(argv)
    try:
        return cmd_build(args) if args.cmd == "build" else cmd_check(args)
    except Refusal as e:
        print(f"refused: {e}", file=sys.stderr)
        return 2
    except BuildFailure as e:
        print(f"failed: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
