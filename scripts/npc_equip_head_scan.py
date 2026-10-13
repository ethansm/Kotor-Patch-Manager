#!/usr/bin/env python3
"""Static scan: which appearance.2da rows can visibly wear a helmet (GoggleHook on the head model).

Read-only over the game directory. Writes into --out:
  head_wearability.tsv  one row per appearance.2da row, sorted by row id
  HEAD_SUMMARY.md       counts, contradictions to the GoggleHook theory, zero-head-item rows

Premise (engine fact established earlier): the helmet model attaches at node "GoggleHook" on the HEAD model.

Head model per appearance row (head_source):
  normalhead  heads.2da row named by normalhead -> 'head' column
  backuphead  heads.2da row named by backuphead -> 'head' column (only when normalhead does not resolve)
  body        neither head column resolves: the body model itself is the head (ASSUMPTION; applies to the
              single-model rows, modeltype S/F/L, that carry no separate head)
  none        no head column and no body model: has_goggle_hook = NO_HEAD

Body model = appearance 'modela' when set, else 'race' (the model name for single-model rows; the probe
showed race names resolving to Models.bif / Override MDLs such as C_Dewback and P_HK47).
body_headhook = yes when any distinct non-empty body candidate (modela, modelb, race) has a node named headhook.

has_goggle_hook: yes | no | LOAD_FAIL (model missing or unreadable) | NO_HEAD (no head and no body model).
n_utc / n_utc_with_head_item: distinct template resrefs in utc_equip.tsv with this appearance_type, and
those with has_head_item == 1. Requires scripts/npc_equip_scan.py to have written utc_equip.tsv first.

Run with the patched pykotor venv, from the repo root:
  ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I scripts/npc_equip_head_scan.py --out .claude/research/npc_equipment
"""

from __future__ import annotations

import argparse
import collections
import csv
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
GAME_DIR_DEFAULT = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
OUT_DEFAULT = REPO_ROOT / ".claude/research/npc_equipment"
HELPER_DIR = REPO_ROOT / "patch_manager_mods" / "k1_port"
HELMET_NODE = "gogglehook"  # node names compared lower-case
BODY_HOOK_NODE = "headhook"
ZERO_HEAD_MIN_TEMPLATES = 10
LIST_CAP = 15
HEAD_TSV_HEADER = ["row", "label", "modeltype", "body_model", "head_model", "head_source",
                   "has_goggle_hook", "body_headhook", "n_utc", "n_utc_with_head_item"]
STATUSES = ("yes", "no", "LOAD_FAIL", "NO_HEAD")
RESREF_RE = re.compile(r"[A-Za-z0-9_]{1,16}")

# Helper import: reuse load_pykotor() / install_get() from the k1_port UV-mask tool.
sys.path.insert(0, str(HELPER_DIR))
from k1_port_uvmask import install_get, load_pykotor  # noqa: E402


def clean(value: object) -> str:
    return str(value).replace("\t", " ").replace("\r", " ").replace("\n", " ")


def write_tsv(path: Path, header: list[str], rows: list[list[object]]) -> None:
    with path.open("w", encoding="utf-8", newline="") as fh:
        fh.write("\t".join(header) + "\n")
        for row in rows:
            fh.write("\t".join(clean(v) for v in row) + "\n")


def is_within(path: Path, parent: Path) -> bool:
    try:
        path.relative_to(parent)
        return True
    except ValueError:
        return False


def node_names(root: object) -> set[str]:
    """Every node name in the MDL tree (dummy and mesh nodes alike), lower-cased."""
    names: set[str] = set()
    stack = [root]
    while stack:
        node = stack.pop()
        name = str(getattr(node, "name", "") or "").strip().lower()
        if name:
            names.add(name)
        stack.extend(getattr(node, "children", None) or [])
    return names


class ModelIndex:
    """Reads each MDL once per resref (case-insensitive) and caches the two node flags."""

    def __init__(self, pk: object, inst: object) -> None:
        self.pk = pk
        self.inst = inst
        self.cache: dict[str, dict] = {}
        self.mdx_missing: set[str] = set()

    def get(self, resref: str) -> dict:
        key = resref.strip().lower()
        if key in self.cache:
            return self.cache[key]
        info = {"ok": False, "goggle": False, "headhook": False, "error": ""}
        try:
            mdl_res = install_get(self.pk, self.inst, resref.strip(), self.pk.ResourceType.MDL)
            if mdl_res is None:
                info["error"] = "MDL not found"
            else:
                mdx_res = install_get(self.pk, self.inst, resref.strip(), self.pk.ResourceType.MDX)
                if mdx_res is None:
                    self.mdx_missing.add(key)
                mdx = bytes(mdx_res.data) if mdx_res is not None else b""
                mdl = self.pk.read_mdl(bytes(mdl_res.data), source_ext=mdx, size_ext=len(mdx))
                names = node_names(mdl.root)
                info.update(ok=True, goggle=HELMET_NODE in names, headhook=BODY_HOOK_NODE in names)
        except Exception as exc:  # noqa: BLE001 - count-only failure, reported as LOAD_FAIL
            info["error"] = f"{type(exc).__name__}: {str(exc)[:160]}"
        self.cache[key] = info
        return info


def load_utc_counts(path: Path) -> tuple[dict[int, set], dict[int, set], int]:
    """Distinct template resrefs per appearance_type, all and with has_head_item == 1."""
    n_all: dict[int, set] = collections.defaultdict(set)
    n_head: dict[int, set] = collections.defaultdict(set)
    rows = 0
    with path.open(encoding="utf-8", newline="") as fh:
        for rec in csv.DictReader(fh, delimiter="\t"):
            rows += 1
            try:
                app = int(rec["appearance_type"])
            except (KeyError, TypeError, ValueError):
                continue
            key = (rec.get("resref") or "").strip().lower()
            n_all[app].add(key)
            if (rec.get("has_head_item") or "").strip() == "1":
                n_head[app].add(key)
    return n_all, n_head, rows


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Scan appearance rows for GoggleHook on the head model (static, read-only).")
    ap.add_argument("--game", type=Path, default=GAME_DIR_DEFAULT, help="KOTOR II install dir (read-only)")
    ap.add_argument("--out", type=Path, default=OUT_DEFAULT, help="output directory (must already hold utc_equip.tsv)")
    args = ap.parse_args(argv)

    game = args.game.expanduser().resolve()
    out = args.out.expanduser().resolve()
    if is_within(out, game):
        print(f"refusing to write into the game dir: {out}", file=sys.stderr)
        return 2
    utc_path = out / "utc_equip.tsv"
    if not utc_path.exists():
        print(f"missing {utc_path}; run scripts/npc_equip_scan.py --out {out} first", file=sys.stderr)
        return 1
    out.mkdir(parents=True, exist_ok=True)

    pk = load_pykotor()
    from pykotor.resource.formats.twoda.twoda_auto import read_2da

    inst = pk.Installation(game)
    ap_res = install_get(pk, inst, "appearance", pk.ResourceType.TwoDA)
    hd_res = install_get(pk, inst, "heads", pk.ResourceType.TwoDA)
    if ap_res is None or hd_res is None:
        print("appearance.2da or heads.2da not found in override/chitin/modules", file=sys.stderr)
        return 1
    ap_t = read_2da(ap_res.data)
    hd_t = read_2da(hd_res.data)
    ap_h = ap_t.get_headers()
    hd_h = hd_t.get_headers()
    if "head" not in hd_h:
        print(f"heads.2da has no 'head' column (headers: {hd_h})", file=sys.stderr)
        return 1
    total = ap_t.get_height()
    heads_n = hd_t.get_height()

    def cell(t: object, headers: list[str], r: int, col: str) -> str:
        return str(t.get_cell(r, col) or "").strip() if col in headers else ""

    bad_index = 0

    def head_for(index_text: str) -> str:
        nonlocal bad_index
        if not index_text:
            return ""
        try:
            i = int(index_text)
        except ValueError:
            bad_index += 1
            return ""
        if i < 0 or i >= heads_n:
            bad_index += 1
            return ""
        return cell(hd_t, hd_h, i, "head")

    n_all, n_head, utc_rows = load_utc_counts(utc_path)
    orphan_apps = sorted(k for k in n_all if k < 0 or k >= total)

    probe = ModelIndex(pk, inst)
    records: list[dict] = []
    for r in range(total):
        label = cell(ap_t, ap_h, r, "label") or f"row{r}"
        modeltype = cell(ap_t, ap_h, r, "modeltype")
        modela = cell(ap_t, ap_h, r, "modela")
        modelb = cell(ap_t, ap_h, r, "modelb")
        race = cell(ap_t, ap_h, r, "race")
        body = modela or race

        head, source = head_for(cell(ap_t, ap_h, r, "normalhead")), "normalhead"
        if not head:
            head, source = head_for(cell(ap_t, ap_h, r, "backuphead")), "backuphead"
        if not head:
            head, source = (body, "body") if body else ("", "none")

        if not head:
            status = "NO_HEAD"
        else:
            info = probe.get(head)
            status = "LOAD_FAIL" if not info["ok"] else ("yes" if info["goggle"] else "no")

        cands: list[str] = []
        for m in (modela, modelb, race):
            if m and m.lower() not in {c.lower() for c in cands}:
                cands.append(m)
        if not cands:
            body_hook = "NO_BODY"
        else:
            infos = [probe.get(m) for m in cands]
            if any(i["ok"] and i["headhook"] for i in infos):
                body_hook = "yes"
            elif any(not i["ok"] for i in infos):
                body_hook = "LOAD_FAIL"
            else:
                body_hook = "no"

        records.append({
            "row": r, "label": label, "modeltype": modeltype, "body": body, "head": head,
            "source": source, "status": status, "body_hook": body_hook,
            "n_utc": len(n_all.get(r, ())), "n_head": len(n_head.get(r, ())),
        })

    # 3. head_wearability.tsv
    write_tsv(
        out / "head_wearability.tsv",
        HEAD_TSV_HEADER,
        [[d["row"], d["label"], d["modeltype"], d["body"], d["head"], d["source"], d["status"],
          d["body_hook"], d["n_utc"], d["n_head"]] for d in records],
    )

    # 4. HEAD_SUMMARY.md
    status_counts = collections.Counter(d["status"] for d in records)
    src_status: collections.Counter = collections.Counter((d["source"], d["status"]) for d in records)
    load_fail_heads = sorted({d["head"].lower() for d in records if d["status"] == "LOAD_FAIL"})
    body_fail_rows = sum(1 for d in records if d["body_hook"] == "LOAD_FAIL")
    contradictions = sorted(
        (d for d in records if d["n_head"] > 0 and d["status"] != "yes"),
        key=lambda d: (-d["n_utc"], -d["n_head"], d["row"]),
    )
    zero_head_yes = sorted(
        (d for d in records if d["status"] == "yes" and d["n_head"] == 0 and d["n_utc"] >= ZERO_HEAD_MIN_TEMPLATES),
        key=lambda d: (-d["n_utc"], d["row"]),
    )
    rows_with_templates = sum(1 for d in records if d["n_utc"] > 0)

    def item_line(d: dict) -> str:
        return (f"- row {d['row']} {d['label']} (modeltype {d['modeltype'] or '-'}): head `{d['head'] or '-'}` "
                f"[{d['source']}], has_goggle_hook={d['status']}, body_headhook={d['body_hook']}, "
                f"n_utc={d['n_utc']}, n_utc_with_head_item={d['n_head']}")

    cross: collections.Counter = collections.Counter((d["status"], d["body_hook"]) for d in records)
    non_resref = sum(1 for d in records if d["body"] and not RESREF_RE.fullmatch(d["body"]))

    lines: list[str] = []
    lines.append("# NPC head wearability scan (static)")
    lines.append("")
    lines.append(f"Game dir: `{game}` (read-only). appearance.2da rows: {total}. utc_equip.tsv rows read: "
                 f"{utc_rows}. UTC rows whose appearance_type is outside appearance.2da: "
                 f"{sum(len(n_all[k]) for k in orphan_apps)} (apps {orphan_apps or 'none'}).")
    lines.append("")
    lines.append("Premise: the helmet attaches at node `GoggleHook` on the head model. Output: "
                 "`head_wearability.tsv` (one row per appearance row).")
    lines.append("")
    lines.append("## Counts by has_goggle_hook")
    lines.append("")
    for s in STATUSES:
        lines.append(f"- {s}: {status_counts[s]}")
    lines.append("")
    lines.append("## has_goggle_hook by head_source")
    lines.append("")
    lines.append("normalhead / backuphead = head from heads.2da; body = no head column, the body model is the "
                 "head (assumption); none = no head and no body model.")
    lines.append("")
    for src in ("normalhead", "backuphead", "body", "none"):
        parts = ", ".join(f"{s}={src_status[(src, s)]}" for s in STATUSES if src_status[(src, s)])
        lines.append(f"- {src}: {parts or 'none'}")
    lines.append("")
    lines.append("## has_goggle_hook x body_headhook (rows)")
    lines.append("")
    for s in STATUSES:
        parts = ", ".join(f"body_headhook {b}={cross[(s, b)]}" for b in ("yes", "no", "LOAD_FAIL", "NO_BODY")
                          if cross[(s, b)])
        lines.append(f"- has_goggle_hook {s}: {parts or 'none'}")
    lines.append("")
    lines.append("## Coverage")
    lines.append("")
    lines.append(f"- appearance rows with >=1 template in utc_equip.tsv: {rows_with_templates}")
    lines.append(f"- appearance rows with >=1 head-item template: {sum(1 for d in records if d['n_head'] > 0)}")
    lines.append(f"- body_headhook: " + ", ".join(
        f"{k}={sum(1 for d in records if d['body_hook'] == k)}"
        for k in ("yes", "no", "LOAD_FAIL", "NO_BODY")))
    lines.append(f"- heads.2da index failures (non-int, out of range; treated as absent): {bad_index}")
    lines.append(f"- body_model values that are not plain resrefs (e.g. '&638' in modela; treated as the body "
                 f"model, so usually LOAD_FAIL): {non_resref}")
    lines.append("")
    lines.append(f"## Contradictions: head items on rows without GoggleHook ({len(contradictions)} rows; top {LIST_CAP} by n_utc)")
    lines.append("")
    if contradictions:
        lines.extend(item_line(d) for d in contradictions[:LIST_CAP])
    else:
        lines.append("- none")
    lines.append("")
    lines.append(f"## GoggleHook yes but zero head-item templates, n_utc >= {ZERO_HEAD_MIN_TEMPLATES} "
                 f"({len(zero_head_yes)} rows; top {LIST_CAP} by n_utc)")
    lines.append("")
    if zero_head_yes:
        lines.extend(item_line(d) for d in zero_head_yes[:LIST_CAP])
    else:
        lines.append("- none")
    lines.append("")
    lines.append("## Load failures (count only)")
    lines.append("")
    lines.append(f"- rows with has_goggle_hook=LOAD_FAIL: {status_counts['LOAD_FAIL']} "
                 f"(distinct head resrefs: {len(load_fail_heads)})")
    lines.append(f"- rows with body_headhook=LOAD_FAIL: {body_fail_rows}")
    reasons = collections.Counter(
        ("MDL not found" if info["error"] == "MDL not found" else "read error")
        for info in probe.cache.values() if not info["ok"]
    )
    lines.append(f"- distinct MDL resrefs that failed (head and body candidates), by reason: " + (", ".join(
        f"{k}={v}" for k, v in sorted(reasons.items())) or "none"))
    lines.append(f"- distinct MDL resrefs with MDX not found (read with empty MDX): {len(probe.mdx_missing)}")
    lines.append("")
    lines.append("## Notes")
    lines.append("")
    lines.append("- n_utc counts distinct template resrefs (case-insensitive) per appearance_type, so a template in "
                 "several containers counts once. n_utc_with_head_item uses has_head_item == 1 (HEAD slot or a "
                 "headgear base item; not every such item is a helmet).")
    lines.append("- Zero head-item templates are not evidence that a row cannot wear a helmet: the sample may "
                 "simply not equip one.")
    lines.append("- body_headhook checks modela, modelb and race together, so it is a union, not the single body "
                 "model in the body_model column.")
    lines.append("- head_source=body is an assumption, not a verified engine path. Its has_goggle_hook result "
                 "tests the body mesh for GoggleHook.")
    lines.append("- Not checked here: whether the node is reachable in the rendered skeleton, whether a "
                 "helmet item's model is attached there at runtime, or any engine-side visibility rule.")
    # Keep the hand-written "## Reading" section (last section) across reruns.
    summary_path = out / "HEAD_SUMMARY.md"
    text = "\n".join(lines) + "\n"
    if summary_path.exists():
        old = summary_path.read_text(encoding="utf-8")
        if "\n## Reading\n" in old:
            text += "\n## Reading\n" + old.split("\n## Reading\n", 1)[1].rstrip() + "\n"
    summary_path.write_text(text, encoding="utf-8")

    print(f"head_wearability.tsv rows={len(records)} " + " ".join(f"{s}={status_counts[s]}" for s in STATUSES))
    print(f"contradictions={len(contradictions)} zero_head_yes={len(zero_head_yes)} "
          f"load_fail_heads={len(load_fail_heads)} body_headhook_load_fail_rows={body_fail_rows}")
    print(f"HEAD_SUMMARY.md written to {out / 'HEAD_SUMMARY.md'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
