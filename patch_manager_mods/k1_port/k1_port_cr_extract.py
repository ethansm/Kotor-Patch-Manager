#!/usr/bin/env python3
"""Shard-wise extraction of the Characters Revisited RAR packs to a dir outside the repo.

Stdlib only. Subcommands:
  list  files and MB per shard
  plan  same, plus per-pack rar path and the unrar command line (nothing executed)
  run   extract with unrar, flatten to <dest>/<shard>/<lowercase basename>, update <dest>/manifest.tsv.
        Resumable: files already present with the expected size are skipped.

Env: K1PORT_CR_DEST (dest), K1PORT_CR_RAR_MAIN / K1PORT_CR_RAR_CREATURES / K1PORT_CR_RAR_UPDATES (rar paths).
Tests: python3 test_k1_port_cr_extract.py
"""
from __future__ import annotations

import argparse
import hashlib
import os
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
DEFAULT_FILE_LIST = Path(".claude/waves/w9-38-CR/cr_files.tsv")  # relative to repo root
DEFAULT_DEST = Path.home() / "Games" / "KOTOR2_Mod_Workspace" / "k1_port_extract" / "characters_revisited"
KNOWN_PACKS = ("main", "creatures", "updates")
ALL_SHARDS = (
    "models", "updates", "creature", "head_player", "body_player", "species",
    "faction_npc", "commoner", "party", "other", "portrait", "item",
)
DEFAULT_SHARDS = tuple(s for s in ALL_SHARDS if s not in ("portrait", "item"))
MANIFEST_NAME = "manifest.tsv"
MANIFEST_HEADER = "\t".join(("stem", "ext", "size", "sha256", "shard"))
NAMES_FILE = ".names.txt"
MODEL_EXTS = ("mdl", "mdx")

# Same family table as .claude/waves/w9-38-CR/classify.py; first match wins.
_FAMILY_RULES = tuple(
    (re.compile(pat), fam)
    for pat, fam in (
        (r"^po_", "portrait"),
        (r"^(pfh|pmh)[abc]\d", "head_player"),
        (r"^(pfb|pmb)", "body_player"),
        (r"^(n_comm|comm_)", "commoner_or_head"),
        (r"^(n_wookie|n_rodian|n_tusken|n_duros|n_selkath|n_walrus|n_mandal|n_rakata|p_trn|n_twilek|twilek|n_bith|c_gam|c_hutt|c_ithor)", "species"),
        (r"^c_", "creature_droid"),
        (r"^i_", "item"),
        (r"^(n_smug|n_swoop|n_sith|n_jedi|n_dark|n_repsold|n_repoff)", "faction_npc"),
        (r"^p_", "party"),
    )
)

_MAIN_SHARD = {
    "head_player": "head_player",
    "body_player": "body_player",
    "species": "species",
    "faction_npc": "faction_npc",
    "commoner_or_head": "commoner",
    "creature_droid": "creature",
    "party": "party",
    "other": "other",
    "portrait": "portrait",
    "item": "item",
}

_RAR_NAMES = {
    "main": "Characters Revisited-1639-1-0-1715980455.rar",
    "creatures": "Characters Revisited_Creatures_Addon-1639-1-0-1716349899.rar",
    "updates": "2_Characters Revisited_Updates-1639-Up-07-1723138875.rar",
}


def family(stem: str) -> str:
    name = stem.lower()
    for rx, fam in _FAMILY_RULES:
        if rx.match(name):
            return fam
    return "other"


def shard_of(pack: str, stem: str, ext: str) -> str | None:
    if pack == "updates":
        return "updates"
    if pack == "creatures":
        return "creature"
    if pack != "main":
        return None
    if ext.lower().lstrip(".") in MODEL_EXTS:
        return "models"
    return _MAIN_SHARD[family(stem)]


def split_name(path_inside: str) -> tuple[str, str]:
    base = path_inside.rsplit("/", 1)[-1]
    stem, dot, ext = base.rpartition(".")
    if not dot:
        return base.lower(), ""
    return stem.lower(), ext.lower()


def parse_tsv(path: Path) -> tuple[list[tuple[str, int, str]], int]:
    """Rows are (pack, size, path_inside_rar). Returns (rows, malformed_count). Blank lines are ignored."""
    rows: list[tuple[str, int, str]] = []
    skipped = 0
    with open(path, encoding="utf-8", errors="replace") as fh:
        for raw in fh:
            line = raw.rstrip("\r\n")
            if not line.strip():
                continue
            parts = line.split("\t")
            if len(parts) != 3 or parts[0] not in KNOWN_PACKS or not parts[1].isdigit() or not parts[2]:
                skipped += 1
                continue
            rows.append((parts[0], int(parts[1]), parts[2]))
    return rows, skipped


def _entries(rows):
    """[(shard, pack, path_inside, size)] in file order. Duplicates and unknown packs are dropped."""
    seen = set()
    out = []
    for pack, size, path_inside in rows:
        if (pack, path_inside) in seen:
            continue
        if size == 0 and "." not in path_inside.rsplit("/", 1)[-1]:
            continue  # rar directory entry: handing it to unrar extracts the whole directory
        seen.add((pack, path_inside))
        stem, ext = split_name(path_inside)
        shard = shard_of(pack, stem, ext)
        if shard is not None:
            out.append((shard, pack, path_inside, size))
    return out


def group_entries(rows) -> dict[str, list[tuple[str, str, int]]]:
    """shard -> [(pack, path_inside, size)]."""
    grouped: dict[str, list[tuple[str, str, int]]] = {}
    for shard, pack, path_inside, size in _entries(rows):
        grouped.setdefault(shard, []).append((pack, path_inside, size))
    return grouped


def plan_shards(rows) -> dict[str, list[tuple[str, int]]]:
    """shard -> [(path_inside_rar, size)]. Includes portrait and item; the CLI applies the default filter."""
    return {shard: [(p, s) for _pk, p, s in items] for shard, items in group_entries(rows).items()}


def check_dest(dest: Path, repo_root: Path) -> None:
    d = Path(dest).expanduser().resolve()
    r = Path(repo_root).resolve()
    if d == r or r in d.parents:
        raise ValueError(f"dest {d} is inside the repo {r}; choose a dir outside the repo")


def check_disk(free_bytes: int, need_bytes: int) -> None:
    if free_bytes < 2 * need_bytes:
        raise ValueError(
            f"not enough free space: {free_bytes / 2**30:.2f} GiB free, "
            f"need at least {2 * need_bytes / 2**30:.2f} GiB (2x {need_bytes / 2**30:.2f} GiB)"
        )


def find_repo_root(start: Path) -> Path:
    for cand in (start, *start.parents):
        if (cand / ".git").exists():
            return cand
    raise RuntimeError(f"no .git found at or above {start}")


def rar_path(pack: str) -> Path:
    env = os.environ.get(f"K1PORT_CR_RAR_{pack.upper()}")
    if env:
        return Path(env).expanduser()
    return Path.home() / "Downloads" / _RAR_NAMES[pack]


def dest_path(cli_value: Path | None) -> Path:
    if cli_value is not None:
        return Path(cli_value).expanduser()
    env = os.environ.get("K1PORT_CR_DEST")
    if env:
        return Path(env).expanduser()
    return DEFAULT_DEST


def target_of(shard_dir: Path, path_inside: str) -> Path:
    return shard_dir / path_inside.rsplit("/", 1)[-1].lower()


def is_present(shard_dir: Path, path_inside: str, size: int) -> bool:
    t = target_of(shard_dir, path_inside)
    return t.is_file() and t.stat().st_size == size


def unrar_argv(rar: Path, listfile: Path, out_dir: Path) -> list[str]:
    return ["unrar", "x", "-o-", "-inul", str(rar), f"@{listfile}", f"{out_dir}{os.sep}"]


def flatten_shard(shard_dir: Path) -> None:
    """Move files from unrar's Override/ (or any other) subdirs to <shard_dir>/<lowercase basename>."""
    nested = [p for p in shard_dir.rglob("*") if p.is_file() and p.parent != shard_dir]
    for f in nested:
        os.replace(f, shard_dir / f.name.lower())
    subdirs = sorted((p for p in shard_dir.rglob("*") if p.is_dir()), key=lambda p: len(p.parts), reverse=True)
    for d in subdirs:
        try:
            d.rmdir()
        except OSError:
            pass


def sha256_of(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def write_manifest(dest: Path, shard: str, rows) -> None:
    """rows: [(stem, ext, size, sha256)]. Replaces this shard's manifest rows, keeps every other shard's."""
    path = dest / MANIFEST_NAME
    kept: list[str] = []
    if path.is_file():
        for line in path.read_text(encoding="utf-8").splitlines():
            if not line or line == MANIFEST_HEADER:
                continue
            if line.split("\t")[-1] != shard:
                kept.append(line)
    new = ["\t".join((stem, ext, str(size), digest, shard)) for stem, ext, size, digest in rows]
    dest.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(path.name + ".tmp")
    tmp.write_text("\n".join([MANIFEST_HEADER, *kept, *new]) + "\n", encoding="utf-8")
    os.replace(tmp, path)


def _existing_ancestor(p: Path) -> Path:
    for cand in (p, *p.parents):
        if cand.exists():
            return cand
    return Path(p.anchor)


def _names_by_pack(items) -> dict[str, list[str]]:
    out: dict[str, list[str]] = {}
    for pk, p, _s in items:
        out.setdefault(pk, []).append(p)
    return out


def _mb(items) -> float:
    return sum(s for _pk, _p, s in items) / (1024 * 1024)


def _print_commands(shard, dest, items, check_rar):
    shard_dir = dest / shard
    for pk, names in _names_by_pack(items).items():
        rar = rar_path(pk)
        note = ""
        if check_rar:
            note = " (found)" if rar.is_file() else " (MISSING)"
        print(f"  pack {pk}: {len(names)} files, rar {rar}{note}")
        print("  " + shlex.join(unrar_argv(rar, shard_dir / NAMES_FILE, shard_dir)))


def run_shard(shard, items, dest):
    """Extract the missing files of one shard. items: [(pack, path_inside, size)]. Returns (expected, present)."""
    if not items:
        return 0, 0
    shard_dir = dest / shard
    todo = [(pk, p, s) for pk, p, s in items if not is_present(shard_dir, p, s)]
    by_pack = _names_by_pack(todo)
    rars = {pk: rar_path(pk) for pk in by_pack}
    for pk, rar in rars.items():
        if not rar.is_file():
            raise RuntimeError(f"rar for pack '{pk}' not found: {rar} (set K1PORT_CR_RAR_{pk.upper()})")
    if todo:
        check_disk(shutil.disk_usage(_existing_ancestor(dest)).free, sum(s for _pk, _p, s in todo))
    shard_dir.mkdir(parents=True, exist_ok=True)
    for pk, names in by_pack.items():
        listfile = shard_dir / NAMES_FILE
        listfile.write_text("\n".join(names) + "\n", encoding="utf-8")
        proc = subprocess.run(
            unrar_argv(rars[pk], listfile, shard_dir),
            capture_output=True, encoding="utf-8", errors="replace", check=False,
        )
        if proc.returncode != 0:
            print(f"WARN: unrar exit {proc.returncode} (pack {pk}, shard {shard}): "
                  f"{proc.stderr.strip()[:300]}", file=sys.stderr)
    flatten_shard(shard_dir)
    seen: set[str] = set()
    rows = []
    present = 0
    for _pk, p, s in items:
        t = target_of(shard_dir, p)
        if not (t.is_file() and t.stat().st_size == s):
            continue
        present += 1
        if t.name in seen:
            continue
        seen.add(t.name)
        stem, ext = split_name(t.name)
        rows.append((stem, ext, s, sha256_of(t)))
    write_manifest(dest, shard, rows)
    return len(items), present


def cmd_list(rows, shards):
    grouped = group_entries(rows)
    for shard in shards:
        items = grouped.get(shard, [])
        print(f"shard {shard:<12} {len(items):>7} files {_mb(items):>10.1f} MB")


def cmd_plan(rows, shards, dest):
    grouped = group_entries(rows)
    for shard in shards:
        items = grouped.get(shard, [])
        print(f"shard {shard}: {len(items)} files, {_mb(items):.1f} MB -> {dest / shard}")
        _print_commands(shard, dest, items, check_rar=True)


def cmd_run(rows, shards, dest, dry_run) -> int:
    grouped = group_entries(rows)
    if not dry_run and shutil.which("unrar") is None:
        raise RuntimeError("unrar not found on PATH")
    all_ok = True
    for shard in shards:
        items = grouped.get(shard, [])
        if dry_run:
            _print_commands(shard, dest, items, check_rar=False)
            print(f"shard {shard}: {len(items)} files expected, 0 extracted")
            continue
        expected, extracted = run_shard(shard, items, dest)
        print(f"shard {shard}: {expected} files expected, {extracted} extracted")
        all_ok = all_ok and extracted == expected
    return 0 if all_ok else 1


def resolve_run_shards(shard_arg: str, include_all: bool):
    if shard_arg == "all":
        return ALL_SHARDS if include_all else DEFAULT_SHARDS
    if shard_arg in ALL_SHARDS:
        return (shard_arg,)
    raise ValueError(f"unknown shard {shard_arg!r}; use one of {', '.join(ALL_SHARDS)} or all")


def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description="Shard-wise extraction of Characters Revisited RARs (stdlib only).")
    sub = ap.add_subparsers(dest="cmd", required=True)

    sp = sub.add_parser("list", help="files and MB per shard")
    sp.add_argument("--all", action="store_true", help="include portrait and item shards")
    sp.add_argument("--file-list", type=Path, help="TSV: pack<TAB>size<TAB>path-inside-rar")

    sp = sub.add_parser("plan", help="rar paths and unrar command lines (nothing executed)")
    sp.add_argument("--all", action="store_true", help="include portrait and item shards")
    sp.add_argument("--file-list", type=Path)
    sp.add_argument("--dest", type=Path)

    sp = sub.add_parser("run", help="extract shard(s) and flatten them")
    sp.add_argument("--shard", required=True, help="shard name, or 'all'")
    sp.add_argument("--all", action="store_true", help="with --shard all: include portrait and item")
    sp.add_argument("--dry-run", action="store_true", help="print the plan; no unrar, no disk writes")
    sp.add_argument("--file-list", type=Path)
    sp.add_argument("--dest", type=Path)
    return ap


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    try:
        repo_root = find_repo_root(HERE)
        file_list = Path(args.file_list or (repo_root / DEFAULT_FILE_LIST)).expanduser()
        if not file_list.is_file():
            raise RuntimeError(f"file list not found: {file_list}")
        rows, skipped = parse_tsv(file_list)
        print(f"file list: {file_list}  rows={len(rows)}  malformed_skipped={skipped}")
        if args.cmd == "list":
            cmd_list(rows, ALL_SHARDS if args.all else DEFAULT_SHARDS)
            return 0
        if args.cmd == "plan":
            cmd_plan(rows, ALL_SHARDS if args.all else DEFAULT_SHARDS, dest_path(args.dest))
            return 0
        shards = resolve_run_shards(args.shard, args.all)
        dest = dest_path(args.dest)
        check_dest(dest, repo_root)
        return cmd_run(rows, shards, dest, args.dry_run)
    except (RuntimeError, ValueError, OSError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
