#!/usr/bin/env python3
"""Import K1-only models (default: the 15 p_* companion models) into the K1 port stage with CR textures.

  python3 scripts/k1_import_companions.py --dumps <dir with <model>.json from `k1_port_tex_preview.py mesh`>
                                          --models-file <one resref per line> [--apply] [--stage-dir DIR]

Per model it runs k1_port_ingest.py (K1 venv python) with --texture OLD=k1p_<old> for every texture the dump
lists, --prefer-source characters_revisited_main (textures only; the model itself still comes from vanilla K1)
and --new-resref k1p_<resref>. Dry run by default; --apply stages for real. Nothing is written into the K1/K2
installs; the stage and ledger live outside the repo, flagged CR files stay `pending_author_approval`.
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
INGEST = HERE.parent / "patch_manager_mods" / "k1_port" / "k1_port_ingest.py"
K1_PY = Path("~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python").expanduser()
PREFER = "characters_revisited_main"
NEW_PREFIX = "k1p_"
MAX_RESREF, MAX_TEX = 16, 31


def short(name: str) -> str:
    """k1p_ + name without its leading p_ (k1p_p_bastilabb02 would be 17 chars; the resref limit is 16)."""
    low = name.lower()
    return NEW_PREFIX + (low[2:] if low.startswith("p_") else low)


def texture_names(dump: dict) -> list[str]:
    """Distinct texture names of a dump, first-seen order, case kept from the first sighting."""
    seen: dict[str, str] = {}
    for node in dump.get("nodes", []):
        tex = node.get("texture")
        if tex and tex.lower() not in seen:
            seen[tex.lower()] = tex
    return list(seen.values())


def plan_model(resref: str, dump: dict) -> dict:
    """{'resref', 'new_resref', 'textures': [(OLD, NEW)]}; ValueError on a name that would not fit."""
    new_resref = short(resref)
    if len(new_resref) > MAX_RESREF:
        raise ValueError(f"{resref}: new resref {new_resref!r} is longer than {MAX_RESREF}")
    pairs = []
    for old in texture_names(dump):
        new = short(old)
        if len(new) > MAX_TEX:
            raise ValueError(f"{resref}: texture {new!r} is longer than {MAX_TEX}")
        pairs.append((old, new))
    if not pairs:
        raise ValueError(f"{resref}: dump lists no textures")
    return {"resref": resref, "new_resref": new_resref, "textures": pairs}


def ingest_cmd(plan: dict, stage: Path | None, apply: bool) -> list[str]:
    cmd = [str(K1_PY), "-I", "-B", "-u", str(INGEST), "--resref", plan["resref"],
           "--new-resref", plan["new_resref"], "--prefer-source", PREFER, "--json"]
    for old, new in plan["textures"]:
        cmd += ["--texture", f"{old}={new}"]
    if stage is not None:
        cmd += ["--stage-dir", str(stage)]
    if not apply:
        cmd.append("--dry-run")
    return cmd


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--dumps", type=Path, required=True)
    ap.add_argument("--models-file", type=Path, required=True)
    ap.add_argument("--stage-dir", type=Path, default=None)
    ap.add_argument("--apply", action="store_true", help="stage for real (default: dry run)")
    args = ap.parse_args(argv)
    models = [m.strip() for m in args.models_file.read_text(encoding="utf-8").split() if m.strip()]
    plans = []
    for m in models:
        try:
            plans.append(plan_model(m, json.loads((args.dumps / f"{m}.json").read_text(encoding="utf-8"))))
        except (OSError, ValueError) as e:
            print(f"refused: {e}", file=sys.stderr)
            return 2
    bad = 0
    for plan in plans:
        proc = subprocess.run(ingest_cmd(plan, args.stage_dir, args.apply), capture_output=True, text=True)
        try:
            out = json.loads(proc.stdout)
        except json.JSONDecodeError:
            out = {"errors": [proc.stderr.strip()[-300:] or "no JSON from ingest"]}
        chosen = sorted({t["chosen"]["source_id"] for t in out.get("textures", [])})
        model = (out.get("model") or {}).get("chosen", {}).get("source_id")
        status = "ok" if proc.returncode == 0 else "FAIL"
        bad += proc.returncode != 0
        print(f"{status} {plan['resref']} -> {plan['new_resref']} model={model} textures={chosen} "
              f"flagged={len(out.get('flagged_files', []))} errors={out.get('errors', [])}")
    print(f"{len(plans) - bad}/{len(plans)} ok ({'applied' if args.apply else 'dry run'})")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
