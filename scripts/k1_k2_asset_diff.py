#!/usr/bin/env python3
"""Deterministic, read-only resource-level diff of KotOR 1 vs KotOR 2 assets (MDL, MDX, TPC, TGA, WOK).

Writes into --out:
  asset_diff_all.tsv            every (type, resref) row; written only when it stays under 20 MiB
  asset_diff_focus.tsv          focus families (i_, iw, w_, a_, p_, icon), every status except both_same_hash
  asset_diff_only_k1.tsv        every family, status only_k1
  asset_diff_mdl_different.tsv  MDL both_different, with node counts, texture lists and texture-list diff
  asset_diff_SUMMARY.md         counts per type x status and per family, containers, failures, rules

Containers scanned per game (same walk as k1_item_inventory.py): override, chitin bifs, every module file in
modules/ (.rim, _s.rim, .mod; symlink case-variants collapsed by realpath), every .rim/.mod/.erf in rims/, and
every .erf in TexturePacks/ (group TEXPACK, lowest precedence). TexturePacks is outside the brief's list: K1 keeps
its TPC textures only there, so leaving it out would misreport every K1 TPC as missing.
Primary copy of a (type, resref): first readable copy in OVERRIDE > CHITIN > MODULE (sorted) > RIMS (sorted) >
TEXPACK (sorted). Every copy is listed in the containers column.

Run with the patched pykotor venv:
  ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I scripts/k1_k2_asset_diff.py --out .claude/research/k1_items
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import os
import re
import sys
from pathlib import Path
from typing import NamedTuple

REPO_ROOT = Path(__file__).resolve().parent.parent
K1_DIR_DEFAULT = Path.home() / ".local/share/Steam/steamapps/common/swkotor"
K2_DIR_DEFAULT = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
OUT_DEFAULT = REPO_ROOT / ".claude/research/k1_items"
HELPER_DIR = REPO_ROOT / "patch_manager_mods" / "k1_port"
TYPES = ("MDL", "MDX", "TPC", "TGA", "WOK")
TYPE_RANK = {t: i for i, t in enumerate(TYPES)}
GROUPS = ("OVERRIDE", "CHITIN", "MODULE", "RIMS", "TEXPACK")
GROUP_ORDER = {g: i for i, g in enumerate(GROUPS)}
STATUSES = ("only_k1", "only_k2", "both_same_hash", "both_different")
FAMILIES = ("i_", "iw", "w", "a", "p", "icon")  # labels returned by family_of()
# First match wins. The brief's literal icon regex is ICON_RE below; it matches almost no real icon names
# (K1: 0, K2: 3 resrefs), so the item-icon prefixes ia_/ii_/ip_/if_ (i + itemclass + _NNN) are also icons.
FAMILY_PREFIXES = (("i_", "i_"), ("iw_", "iw"), ("w_", "w"), ("a_", "a"), ("p_", "p"),
                   ("ia_", "icon"), ("ii_", "icon"), ("ip_", "icon"), ("if_", "icon"))
ICON_RE = re.compile(r"^i[a-z]+_\d+")
ALL_MAX_BYTES = 20 * 1024 * 1024
SIDE_COLS = ["hash", "primary", "copies", "containers", "mdl_nodes", "mdl_textures", "mdl_parse"]
ROW_COLS = (["type", "resref", "family", "status"] + [f"k1_{c}" for c in SIDE_COLS]
            + [f"k2_{c}" for c in SIDE_COLS] + ["mdl_tex_only_k1", "mdl_tex_only_k2"])

# Helper import: reuse load_pykotor() from the k1_port UV-mask tool (as k1_item_inventory.py does).
sys.path.insert(0, str(HELPER_DIR))
from k1_port_uvmask import load_pykotor  # noqa: E402


class Copy(NamedTuple):
    group: str
    container: str
    resname: str
    resource: object


class Game:
    """Everything scanned from one install. Built once by load_game()."""

    def __init__(self, tag: str, inst: object) -> None:
        self.tag = tag
        self.inst = inst
        self.copies: dict[tuple[str, str], list[Copy]] = collections.defaultdict(list)
        self.group_counts: collections.Counter = collections.Counter()  # (group, type) -> copies
        self.records: dict[tuple[str, str], dict] = {}  # (type, resref lower) -> primary record
        self.conflicts: collections.Counter = collections.Counter()  # type -> resources with differing copies
        self.mdl_status: collections.Counter = collections.Counter()
        self.hash_fail = 0
        self.unreadable = 0
        self.module_files = 0
        self.symlink_dupes = 0
        self.module_fail = 0
        self.rims_files = 0
        self.rims_fail = 0
        self.tp_packs = 0
        self.tp_fail = 0


def sha1_hex(data: bytes) -> str:
    return hashlib.sha1(data).hexdigest()


def clean(value: object) -> str:
    return str(value).replace("\t", " ").replace("\r", " ").replace("\n", " ")


def is_within(path: Path, parent: Path) -> bool:
    try:
        path.relative_to(parent)
        return True
    except ValueError:
        return False


def write_text(path: Path, text: str) -> None:
    with path.open("w", encoding="utf-8", newline="") as fh:
        fh.write(text)


def type_label(pk, rtype: object) -> str | None:
    for label in TYPES:
        if rtype == getattr(pk.ResourceType, label):
            return label
    return None


def family_of(resref: str) -> str:
    """Focus family of a resref (case-insensitive), or "" when the resref is outside the focus families."""
    name = resref.lower()
    for prefix, label in FAMILY_PREFIXES:
        if name.startswith(prefix):
            return label
    if ICON_RE.match(name):
        return "icon"
    return ""


def classify(k1_hash: str | None, k2_hash: str | None) -> str:
    """Status of one (type, resref) from the primary-copy hash of each game (None = absent in that game)."""
    if k1_hash is None and k2_hash is None:
        raise ValueError("classify needs a hash from at least one game")
    if k2_hash is None:
        return "only_k1"
    if k1_hash is None:
        return "only_k2"
    return "both_same_hash" if k1_hash == k2_hash else "both_different"


def tex_list_diff(k1_names: list[str], k2_names: list[str]) -> tuple[list[str], list[str]]:
    a, b = set(k1_names), set(k2_names)
    return sorted(a - b), sorted(b - a)


def mdl_nodes_and_textures(root: object) -> tuple[int, list[str]]:
    """Total node count of the MDL tree, and the sorted unique texture_1 names of its mesh nodes."""
    names: set[str] = set()
    nodes = 0
    stack = [root]
    while stack:
        node = stack.pop()
        nodes += 1
        mesh = getattr(node, "mesh", None)
        if mesh is not None:
            tex = str(getattr(mesh, "texture_1", "") or "").strip().lower()
            if tex and tex != "null":
                names.add(tex)
        stack.extend(list(getattr(node, "children", None) or []))
    return nodes, sorted(names)


def row_key(row: dict) -> tuple[int, str]:
    return TYPE_RANK[row["type"]], row["resref"]


def is_focus(row: dict) -> bool:
    return bool(row["family"]) and row["status"] != "both_same_hash"


def side_cols(tag: str, rec: dict | None) -> dict:
    if rec is None:
        return {f"{tag}_{c}": "" for c in SIDE_COLS}
    return {
        f"{tag}_hash": rec["hash"], f"{tag}_primary": rec["primary"], f"{tag}_copies": rec["copies"],
        f"{tag}_containers": rec["containers"], f"{tag}_mdl_nodes": rec["nodes"],
        f"{tag}_mdl_textures": ";".join(sorted(rec["textures"])), f"{tag}_mdl_parse": rec["mdl_parse"],
    }


def build_rows(k1: dict, k2: dict) -> list[dict]:
    """One row per (type, resref) in either game, sorted by type then resref. Pure: no I/O."""
    rows = []
    for key in sorted(set(k1) | set(k2), key=lambda k: (TYPE_RANK[k[0]], k[1])):
        rtype, rl = key
        a = k1.get(key)
        b = k2.get(key)
        status = classify(a["hash"] if a else None, b["hash"] if b else None)
        only1 = only2 = ""
        if status == "both_different" and rtype == "MDL" and a["mdl_parse"] == "yes" and b["mdl_parse"] == "yes":
            d1, d2 = tex_list_diff(a["textures"], b["textures"])
            only1, only2 = ";".join(d1), ";".join(d2)
        row = {"type": rtype, "resref": rl, "family": family_of(rl), "status": status,
               "mdl_tex_only_k1": only1, "mdl_tex_only_k2": only2}
        row.update(side_cols("k1", a))
        row.update(side_cols("k2", b))
        rows.append(row)
    return rows


def render_tsv(cols: list[str], rows: list[dict]) -> str:
    lines = ["\t".join(cols)]
    for row in rows:
        lines.append("\t".join(clean(row[c]) for c in cols))
    return "\n".join(lines) + "\n"


def md_table(header: list[str], rows: list[list[object]]) -> list[str]:
    out = ["| " + " | ".join(header) + " |",
           "| " + " | ".join("---" if i == 0 else "---:" for i in range(len(header))) + " |"]
    out += ["| " + " | ".join(str(v) for v in row) + " |" for row in rows]
    return out


def parse_model(pk, primary: dict, rl: str) -> dict:
    """read_mdl(primary MDL, source_ext=primary MDX). Failures are returned as a status, never raised."""
    mdl_copy = primary[("MDL", rl)]
    mdx_copy = primary.get(("MDX", rl))
    if mdx_copy is None:
        return {"mdl_parse": "no-mdx"}
    try:
        mdx = bytes(mdx_copy.resource.data())
        mdl = pk.read_mdl(bytes(mdl_copy.resource.data()), source_ext=mdx, size_ext=len(mdx))
        nodes, textures = mdl_nodes_and_textures(mdl.root)
    except Exception:  # noqa: BLE001 - counted, reported in SUMMARY
        return {"mdl_parse": "parse-fail"}
    return {"mdl_parse": "yes", "nodes": nodes, "textures": textures}


def build_records(pk, g: Game) -> None:
    primary: dict[tuple[str, str], Copy] = {}
    for key in sorted(g.copies):
        copies = sorted(g.copies[key], key=lambda c: (GROUP_ORDER[c.group], c.container.lower(), c.resname))
        digests: list[str] = []
        first: Copy | None = None
        first_digest = ""
        for c in copies:
            try:
                digest = sha1_hex(bytes(c.resource.data()))
            except Exception:  # noqa: BLE001 - counted, reported in SUMMARY
                g.hash_fail += 1
                continue
            digests.append(digest)
            if first is None:
                first, first_digest = c, digest
        if first is None:
            g.unreadable += 1
            continue
        if len(set(digests)) > 1:
            g.conflicts[key[0]] += 1
        primary[key] = first
        g.records[key] = {
            "hash": first_digest, "primary": f"{first.group}:{first.container}", "copies": len(copies),
            "containers": "; ".join(f"{c.group}:{c.container}" for c in copies),
            "nodes": "", "textures": [], "mdl_parse": "",
        }
    for key, rec in g.records.items():
        if key[0] == "MDL":
            rec.update(parse_model(pk, primary, key[1]))
            g.mdl_status[rec["mdl_parse"]] += 1


def scan_containers(pk, capsule_cls, g: Game) -> None:
    inst = g.inst

    def add(group: str, container: str, res: object) -> None:
        label = type_label(pk, res.restype())
        if label is None:
            return
        name = res.resname()
        g.copies[(label, name.lower())].append(Copy(group, container, name, res))
        g.group_counts[(group, label)] += 1

    for res in inst.override_resources():
        add("OVERRIDE", "OVERRIDE", res)
    for res in inst.chitin_resources():
        add("CHITIN", Path(str(res.filepath())).name, res)
    module_dir = str(inst.module_path())
    seen_real: set[str] = set()
    for name in sorted(inst.modules_list(), key=lambda n: (n.lower(), n)):
        real = os.path.realpath(os.path.join(module_dir, name))
        if real in seen_real:
            g.symlink_dupes += 1
            continue
        seen_real.add(real)
        g.module_files += 1
        try:
            resources = list(inst.module_resources(name))
        except Exception:  # noqa: BLE001 - counted, reported in SUMMARY
            g.module_fail += 1
            continue
        for res in resources:
            add("MODULE", os.path.basename(real), res)
    rims_dir = inst.rims_path()
    if rims_dir and Path(rims_dir).is_dir():
        for path in sorted(Path(rims_dir).iterdir(), key=lambda x: x.name.lower()):
            if path.suffix.lower() not in (".rim", ".mod", ".erf"):
                continue
            g.rims_files += 1
            try:
                resources = list(capsule_cls(path).resources())
            except Exception:  # noqa: BLE001 - counted, reported in SUMMARY
                g.rims_fail += 1
                continue
            for res in resources:
                add("RIMS", path.name, res)
    for pack in sorted(inst.texturepacks_list(), key=lambda n: (n.lower(), n)):
        try:
            resources = list(inst.texturepack_resources(pack))
        except Exception:  # noqa: BLE001 - counted, reported in SUMMARY
            g.tp_fail += 1
            continue
        g.tp_packs += 1
        for res in resources:
            add("TEXPACK", pack, res)


def load_game(pk, capsule_cls, tag: str, game_dir: Path) -> Game:
    g = Game(tag, pk.Installation(str(game_dir)))
    scan_containers(pk, capsule_cls, g)
    build_records(pk, g)
    return g


def render_summary(rows: list[dict], games: tuple[Game, Game], k1_dir: Path, k2_dir: Path, out: Path,
                   wrote_all: bool, all_bytes: int, all_max_bytes: int) -> str:
    g1, g2 = games
    type_status = collections.Counter((r["type"], r["status"]) for r in rows)
    fam_status = collections.Counter((r["family"], r["status"]) for r in rows if r["family"])
    focus_n = sum(1 for r in rows if is_focus(r))
    only_k1_n = sum(1 for r in rows if r["status"] == "only_k1")
    mdl_diff = [r for r in rows if r["type"] == "MDL" and r["status"] == "both_different"]
    mdl_compared = [r for r in mdl_diff if r["k1_mdl_parse"] == "yes" and r["k2_mdl_parse"] == "yes"]
    lines: list[str] = []
    lines.append("# K1 vs K2 resource diff (MDL, MDX, TPC, TGA, WOK)")
    lines.append("")
    lines.append(f"K1 dir: `{k1_dir}` (read-only). K2 dir: `{k2_dir}` (read-only). Output: `{out}`.")
    lines.append("")
    lines.append("## Counts per type x status")
    lines.append("")
    trows = [[t] + [type_status[(t, s)] for s in STATUSES] + [sum(type_status[(t, s)] for s in STATUSES)]
             for t in TYPES]
    trows.append(["total"] + [sum(type_status[(t, s)] for t in TYPES) for s in STATUSES]
                 + [len(rows)])
    lines += md_table(["type", *STATUSES, "total"], trows)
    lines.append("")
    lines.append("## Focus families x status (all types)")
    lines.append("")
    frows = [[f] + [fam_status[(f, s)] for s in STATUSES] + [sum(fam_status[(f, s)] for s in STATUSES)]
             for f in FAMILIES]
    frows.append(["total"] + [sum(fam_status[(f, s)] for f in FAMILIES) for s in STATUSES]
                 + [sum(fam_status[(f, s)] for f in FAMILIES for s in STATUSES)])
    lines += md_table(["family", *STATUSES, "total"], frows)
    lines.append("")
    lines.append("## Primary copy group x status (which container supplies each side's primary)")
    lines.append("")
    grp_status = collections.Counter()
    for r in rows:
        for tag in ("k1", "k2"):
            if r[f"{tag}_primary"]:
                grp_status[(tag, r[f"{tag}_primary"].split(":", 1)[0], r["status"])] += 1
    grows = []
    for tag in ("k1", "k2"):
        for grp in GROUPS:
            vals = [grp_status[(tag, grp, s)] for s in STATUSES]
            grows.append([f"{tag.upper()} {grp}"] + vals + [sum(vals)])
    lines += md_table(["side / group", *STATUSES, "total"], grows)
    lines.append("")
    lines.append("## Output files")
    lines.append("")
    lines.append(f"- asset_diff_all.tsv: {len(rows)} rows, {all_bytes} bytes, "
                 + (f"written (under {all_max_bytes} bytes)." if wrote_all
                    else f"NOT written (at or over the {all_max_bytes}-byte limit; only focus families kept)."))
    lines.append(f"- asset_diff_focus.tsv: {focus_n} rows (focus families, status other than both_same_hash)")
    lines.append(f"- asset_diff_only_k1.tsv: {only_k1_n} rows (all families)")
    lines.append(f"- asset_diff_mdl_different.tsv: {len(mdl_diff)} rows (MDL both_different)")
    lines.append("")
    lines.append("## Containers per game (copies by group x type)")
    lines.append("")
    crow = [[g.tag, grp] + [g.group_counts[(grp, t)] for t in TYPES] for g in (g1, g2) for grp in GROUPS]
    lines += md_table(["game", "group", *TYPES], crow)
    lines.append("")
    for g in (g1, g2):
        lines.append(f"- {g.tag}: module files {g.module_files} (symlink case-variants collapsed {g.symlink_dupes}, "
                     f"failed to open {g.module_fail}); rims/ files {g.rims_files} (failed {g.rims_fail}); "
                     f"TexturePacks {g.tp_packs} (failed {g.tp_fail}).")
    lines.append("")
    lines.append("## Parse failures and unreadable resources (counts)")
    lines.append("")
    for g in (g1, g2):
        lines.append(f"- {g.tag} MDL parse: yes={g.mdl_status['yes']}, no-mdx={g.mdl_status['no-mdx']}, "
                     f"parse-fail={g.mdl_status['parse-fail']}")
        lines.append(f"- {g.tag} copies whose bytes could not be read: {g.hash_fail}; "
                     f"resources with no readable copy (excluded from the diff): {g.unreadable}")
        lines.append(f"- {g.tag} container open failures: modules {g.module_fail}, rims {g.rims_fail}, "
                     f"texture packs {g.tp_fail}")
    lines.append("")
    lines.append("## Content conflicts (resources whose readable copies differ; primary chosen by precedence)")
    lines.append("")
    lines += md_table(["game", *TYPES], [[g.tag] + [g.conflicts[t] for t in TYPES] for g in (g1, g2)])
    lines.append("")
    lines.append("## MDL both_different (texture lists)")
    lines.append("")
    lines.append(f"- MDL both_different: {len(mdl_diff)}; texture lists compared (both parses yes): {len(mdl_compared)}; "
                 f"not compared (a parse failed or no MDX): {len(mdl_diff) - len(mdl_compared)}")
    lines.append(f"- with names only in K1: {sum(1 for r in mdl_compared if r['mdl_tex_only_k1'])}; "
                 f"with names only in K2: {sum(1 for r in mdl_compared if r['mdl_tex_only_k2'])}")
    lines.append("")
    lines.append("## Rules and assumptions")
    lines.append("")
    lines.append("- Containers and primary copy: see the module docstring. Copies that cannot be read are skipped "
                 "and counted; the primary is the first readable copy in precedence order.")
    lines.append("- Keys are (type, resref lower-cased). A TPC and a TGA with the same name are separate resources.")
    lines.append("- Status from the primary-copy sha1 of each game: only_k1 / only_k2 (absent in the other game's "
                 "scan), both_same_hash, both_different. Hashes cover one resource's bytes, not its MDX/MDL pair.")
    lines.append("- MDL: read_mdl(primary MDL, source_ext=primary MDX). `nodes` = every node in the tree. "
                 "`mdl_textures` = sorted unique lower-cased texture_1 of mesh nodes (null and empty skipped). "
                 "mdl_parse: yes, no-mdx (no MDX copy), parse-fail (read_mdl raised).")
    lines.append("- mdl_tex_only_k1 / mdl_tex_only_k2: texture names only in one game, for MDL both_different rows "
                 "where both parses are yes. Blank otherwise.")
    lines.append("- Families, first match wins: i_ -> i_; iw_ -> iw; w_ -> w; a_ -> a; p_ -> p; ia_/ii_/ip_/if_ -> icon; "
                 "else regex `^i[a-z]+_\\d+` -> icon; else not focus. Matching is on the lower-cased resref.")
    lines.append(f"- Literal icon regex `^i[a-z]+_\\d+` matches distinct resrefs: K1 "
                 f"{sum(1 for k in g1.records if ICON_RE.match(k[1]))}, K2 "
                 f"{sum(1 for k in g2.records if ICON_RE.match(k[1]))}; the ia_/ii_/ip_/if_ prefixes are what "
                 "the item icons actually use (see k1_item_inventory.py icon_rule).")
    lines.append("- TEXPACK order: packs are taken in sorted name order. This is not checked against the engine's pack "
                 f"lookup order. K1 has {g1.conflicts['TPC']} TPC names whose copies in different packs differ "
                 f"in bytes, so the chosen primary may not be the copy the game loads.")
    lines.append("- K2 has no rims/ folder (module rims live in modules/). K2 TGA copies: OVERRIDE "
                 f"{g2.group_counts[('OVERRIDE', 'TGA')]}, CHITIN {g2.group_counts[('CHITIN', 'TGA')]}. K2 Override "
                 "is not vanilla-only, so OVERRIDE primaries on the K2 side may be installed or imported files. "
                 "Not verified per file.")
    lines.append("- Output is sorted by type (MDL, MDX, TPC, TGA, WOK) then resref. No timestamps, so reruns on the "
                 "same installs are byte-identical.")
    lines.append("")
    return "\n".join(lines)


def write_outputs(out: Path, rows: list[dict], games: tuple[Game, Game], k1_dir: Path, k2_dir: Path,
                  all_max_bytes: int = ALL_MAX_BYTES) -> bool:
    """Write the five files. Returns whether asset_diff_all.tsv was written."""
    rows = sorted(rows, key=row_key)
    out.mkdir(parents=True, exist_ok=True)
    focus = [r for r in rows if is_focus(r)]
    only_k1 = [r for r in rows if r["status"] == "only_k1"]
    mdl_diff = [r for r in rows if r["type"] == "MDL" and r["status"] == "both_different"]
    all_text = render_tsv(ROW_COLS, rows)
    all_bytes = len(all_text.encode("utf-8"))
    wrote_all = all_bytes < all_max_bytes
    all_path = out / "asset_diff_all.tsv"
    if wrote_all:
        write_text(all_path, all_text)
    else:
        all_path.unlink(missing_ok=True)
    write_text(out / "asset_diff_focus.tsv", render_tsv(ROW_COLS, focus))
    write_text(out / "asset_diff_only_k1.tsv", render_tsv(ROW_COLS, only_k1))
    write_text(out / "asset_diff_mdl_different.tsv", render_tsv(ROW_COLS, mdl_diff))
    write_text(out / "asset_diff_SUMMARY.md",
               render_summary(rows, games, k1_dir, k2_dir, out, wrote_all, all_bytes, all_max_bytes))
    return wrote_all


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Read-only resource-level diff of KotOR 1 vs KotOR 2 assets "
                                             "(MDL, MDX, TPC, TGA, WOK).")
    ap.add_argument("--k1", type=Path, default=K1_DIR_DEFAULT, help="KotOR 1 install dir (read-only)")
    ap.add_argument("--k2", type=Path, default=K2_DIR_DEFAULT, help="KotOR 2 install dir (read-only)")
    ap.add_argument("--out", type=Path, default=OUT_DEFAULT, help="output directory")
    args = ap.parse_args(argv)

    k1 = args.k1.expanduser().resolve()
    k2 = args.k2.expanduser().resolve()
    out = args.out.expanduser().resolve()
    for game in (k1, k2):
        if is_within(out, game):
            print(f"refusing to write into the game dir: {out}", file=sys.stderr)
            return 2

    pk = load_pykotor()
    from pykotor.extract.capsule import Capsule

    g1 = load_game(pk, Capsule, "K1", k1)
    g2 = load_game(pk, Capsule, "K2", k2)
    rows = build_rows(g1.records, g2.records)
    wrote_all = write_outputs(out, rows, (g1, g2), k1, k2)
    statuses = collections.Counter(r["status"] for r in rows)
    print(f"rows={len(rows)} " + " ".join(f"{s}={statuses[s]}" for s in STATUSES))
    print(f"asset_diff_all.tsv written={wrote_all}; focus rows={sum(1 for r in rows if is_focus(r))}")
    print(f"SUMMARY written to {out / 'asset_diff_SUMMARY.md'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
