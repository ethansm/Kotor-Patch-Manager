#!/usr/bin/env python3
"""Ingest one KOTOR 1 model and its textures into the K1 port stage, end to end.

  ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I patch_manager_mods/k1_port/k1_port_ingest.py \
      --resref n_commm --new-resref k1p_commm --texture N_CommM04=k1p_commm04 [--also-hd] [--json]

Candidates (read only):
  texture  vanilla_k1 (the K1 install resolves OLD: override, texture packs, chitin; pykotor lookup) and every
           other registered source whose local_dir holds OLD.tpc or OLD.tga (recursive, case-insensitive).
           Quality = width * height (pykotor read_tpc); the byte size when the file does not read as TPC.
  model    vanilla_k1 (the install resolves <resref>.mdl AND .mdx) and every other registered source whose
           local_dir holds <resref>.mdl AND .mdx in one directory. Quality 0.
Choice: k1_port_sources.pick(). An approval-free source always beats a flagged one, then the class rank,
then the higher quality. Every candidate and the reason it lost is printed.
Inside one source, a file under an Optional* or Upscale* folder sorts last unless --allow-variant-folders is
given; a variant pick that still wins is noted.

Staging (the subtools write files and ledger entries; this driver writes nothing itself):
  <stage>/override/<NEW>.tpc|.tga   chosen texture   (k1_port_textures.py; vanilla -> --from-install)
  <stage>/_hd/override/<NEW>...     --also-hd: a higher-quality REJECTED variant that needs approval (flagged)
  <stage>/models/<NEW>.mdl|.mdx     chosen model    (k1_port_model.py; vanilla -> --install-k1)
Defaults: --stage-dir ~/Games/KOTOR2_Mod_Workspace/k1_port_stage. Model NEW default: k1p_ + resref, max 16 chars.
Ledger: --ledger, else env K1PORT_LEDGER, else <stage>/ledger.json. Passed to every subtool.

Re-runs: an output that already exists is reported as already_ingested and not rewritten.
Validation: the model pair this run writes is checked by k1_port_validate.validate (K2 layout, lowercase names).
Any finding rejects the model: its files and ledger entries are removed (staged textures stay), the report lists
each finding, exit 1. --skip-validate registers it anyway with a loud warning and a note (debugging only).
--dry-run: plans and prints the decisions; writes nothing (pykotor logs go to the K1PORT_LOG_DIR cache).
--no-model: textures only (the tests use it; no model conversion).
Exit: 0 ok, dry-run, or already ingested; 1 refused or failed; 2 bad arguments.
"""
from __future__ import annotations

import argparse
import contextlib
from dataclasses import asdict
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.dont_write_bytecode = True  # no __pycache__ under the repo
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path
import k1_port_sources as ks  # noqa: E402  stdlib only
import k1_port_textures as kt  # noqa: E402  stdlib only at import; pykotor is imported on use
import k1_port_validate as kv  # noqa: E402  stdlib only

TEXTURE_TOOL = HERE / "k1_port_textures.py"
MODEL_TOOL = HERE / "k1_port_model.py"
VANILLA = "vanilla_k1"
DEFAULT_INSTALL = "~/.local/share/Steam/steamapps/common/swkotor"
HD_DIR = "_hd"  # <stage>/_hd/override/ holds flagged higher-quality variants
MODEL_DIR = "models"  # <stage>/models/ holds the converted MDL/MDX pair
NEW_PREFIX = "k1p_"
RESREF_RE = re.compile(r"[A-Za-z0-9_]{1,16}")
NEW_RESREF_RE = re.compile(r"[a-z0-9_]{1,16}")
VARIANT_DIR_RE = re.compile(r"^(optional|upscale)", re.IGNORECASE)  # e.g. "Optional - Upscaled Textures"
VARIANT_NOTE = "chosen from variant folder; pass --allow-variant-folders"
MODEL_EXTS = (".mdl", ".mdx")
SKIP_WARNING = "!!! WARNING: --skip-validate: {name} is registered WITHOUT structural validation (debugging only)"
SKIP_NOTE = "{name}: structural validation skipped (--skip-validate, debugging only)"
EXIT_OK, EXIT_FAILED, EXIT_BAD_ARGS = 0, 1, 2


class Refused(Exception):
    """A request that cannot be planned or finished. Exit 1."""


def default_new_resref(resref: str) -> str:
    return (NEW_PREFIX + resref.lower())[:16]


def make_cand(sid, kind, name, path, pack_dir, ext, quality, basis, width, height) -> dict:
    return {"source_id": sid, "kind": kind, "name": name, "path": path, "pack_dir": pack_dir, "ext": ext,
            "quality": quality, "quality_basis": basis, "width": width, "height": height}


def texture_quality(read_tpc, raw: bytes):
    """(quality, basis, width, height): width * height when pykotor reads the texture, else the byte size."""
    try:
        w, h = read_tpc(raw).dimensions()
        if w > 0 and h > 0:
            return w * h, "pixels", w, h
    except Exception:  # noqa: BLE001  pykotor raises several error types on bad data
        pass
    return len(raw), "bytes", None, None


def facts(reg: dict, sid: str) -> dict:
    src = reg["sources"][sid]
    flag = bool(src["requires_author_approval"])
    return {"license_class": src["license_class"], "requires_author_approval": flag,
            "removal_group": ks.REMOVAL_PENDING if flag else None, "credits": list(src["credits"])}


def view(reg: dict, cand: dict) -> dict:
    return {**cand, **facts(reg, cand["source_id"])}


def walk_names(root: Path, wanted: set) -> dict:
    """Recursive, case-insensitive: lowercased file name -> matching paths."""
    found: dict = {}
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames.sort()
        for fn in filenames:
            if fn.lower() in wanted:
                found.setdefault(fn.lower(), []).append(Path(dirpath) / fn)
    return found


def vanilla_lookup(install: Path, resref: str, olds: list, want_model: bool, read_tpc, notes: list,
                   overlay: dict | None = None) -> dict:
    """What the K1 install holds, read only. Same search order as the converters."""
    out = {"present": False, "model": None, "textures": {}}
    if not install.is_dir():
        notes.append(f"K1 install not found at {install}; {VANILLA} not searched")
        return out
    errs: list = []
    api = kt.import_install_api(errs)
    if api is None:
        raise Refused(errs[0])
    from pykotor.extract.installation import SearchLocation  # noqa: E402  after the import check
    root = install.resolve()
    try:
        with contextlib.redirect_stdout(sys.stderr), kt.pykotor_cwd():
            inst = kt.open_install(install, api, errs)
            if inst is None:
                raise Refused(errs[-1])
            out["present"] = True
            for old in olds:
                res, _txi = inst.texture_resource_result(old)
                if res is None:
                    continue
                data = bytes(res.data)
                ext = ".tga" if res.restype == api.ResourceType.TGA else ".tpc"
                q, basis, w, h = texture_quality(read_tpc, data)
                label = kt.install_label(res.filepath, root)
                if overlay is not None and kt.is_override_label(label):
                    # An overlay's override/ holds flagged files: never vanilla_k1. Staged like a pack file.
                    sid = kt.overlay_source_id(overlay)
                    out["textures"][old] = make_cand(sid, "texture", old, str(res.filepath),
                                                     str(Path(res.filepath).parent), ext, q, basis, w, h)
                else:
                    out["textures"][old] = make_cand(VANILLA, "texture", old, label, None, ext, q, basis, w, h)
            if want_model:
                order = [SearchLocation.OVERRIDE, SearchLocation.CHITIN, SearchLocation.MODULES]
                mdl = inst.resource(resref, api.ResourceType.MDL, order)
                mdx = inst.resource(resref, api.ResourceType.MDX, order)
                if mdl is not None and mdx is not None:
                    out["model"] = make_cand(VANILLA, "model", resref, kt.install_label(mdl.filepath, root),
                                             None, ".mdl", 0, "none", None, None)
    except OSError as e:
        raise Refused(f"cannot prepare pykotor log dir: {e}") from e
    return out


def _variant_folder(p: Path, root: Path) -> bool:
    """True when a directory between root and the file is a variant folder. The file name never counts."""
    return any(VARIANT_DIR_RE.match(part) for part in p.relative_to(root).parts[:-1])


def chosen_from_variant(reg: dict, chosen: dict) -> bool:
    """True when a non-vanilla pick sits under a variant folder of its own source (vanilla is never penalised)."""
    if chosen["source_id"] == VANILLA:
        return False
    local = reg["sources"][chosen["source_id"]].get("local_dir")
    return bool(local) and _variant_folder(Path(chosen["path"]), Path(local).expanduser())


def pack_candidates(reg: dict, resref: str, olds: list, want_model: bool, read_tpc, notes: list,
                    allow_variants: bool = False) -> dict:
    """Candidates from every registered non-vanilla source whose local_dir exists.

    Within a source, variant folders (Optional*/, Upscale*) sort last unless allow_variants is set."""
    wanted = set()
    if want_model:
        wanted |= {f"{resref}.mdl".lower(), f"{resref}.mdx".lower()}
    for old in olds:
        wanted |= {f"{old}.tpc".lower(), f"{old}.tga".lower()}
    out: dict = {"model": [], "textures": {old: [] for old in olds}, "searched": []}
    for sid, src in reg["sources"].items():
        if src["license_class"] == "vanilla":
            continue
        local = src.get("local_dir")
        if not local:  # not extracted here (a registry entry with no files): nothing to search, nothing to report
            continue
        root = Path(local).expanduser()
        if not root.is_dir():
            notes.append(f"{sid}: local_dir {root} is missing; not searched")
            continue
        out["searched"].append(sid)
        found = walk_names(root, wanted)
        if want_model:
            mdx_dirs = {p.parent for p in found.get(f"{resref}.mdx".lower(), [])}
            for mdl in sorted(found.get(f"{resref}.mdl".lower(), []),
                              key=lambda p: (not allow_variants and _variant_folder(p, root), p.as_posix())):
                if mdl.parent in mdx_dirs:
                    out["model"].append(make_cand(sid, "model", resref, str(mdl), str(mdl.parent), ".mdl",
                                                  0, "none", None, None))
                    break
        for old in olds:
            paths = found.get(f"{old}.tpc".lower(), []) + found.get(f"{old}.tga".lower(), [])
            if not paths:
                continue
            # Same rule as the texture stager: a .tpc anywhere beats a .tga, then the shallowest path.
            # A variant folder (Optional*/, Upscale*) is penalised ahead of both, unless allow_variants.
            # An updates/ folder (Characters Revisited patch files) beats the same name in the base shard.
            best = min(paths, key=lambda p: (not allow_variants and _variant_folder(p, root),
                                             p.suffix.lower() != ".tpc",
                                             not any(part.lower() == "updates" for part in p.relative_to(root).parts[:-1]),
                                             len(p.relative_to(root).parts),
                                             p.as_posix()))
            q, basis, w, h = texture_quality(read_tpc, best.read_bytes())
            if basis == "bytes":
                notes.append(f"{sid}: {best.name} does not read as TPC; quality is its byte size")
            out["textures"][old].append(make_cand(sid, "texture", old, str(best), str(best.parent),
                                                  best.suffix.lower(), q, basis, w, h))
    return out


def decide(reg: dict, name: str, new: str, cands: list, searched: list, prefer: tuple = ()) -> dict:
    if not cands:
        where = ", ".join(searched) if searched else "no source"
        raise Refused(f"no candidates for {name} -> {new}: searched {where}; none holds it. "
                      "Check the name, --install-k1, or the local_dir of a source in sources.json.")
    preferred = [c for c in cands if c["source_id"] in prefer]
    if preferred:  # --prefer-source: a preferred source outranks the rest, even unflagged vanilla
        res = ks.pick(preferred)
        res["rejected"] += [{"candidate": c, "reason": f"--prefer-source {res['chosen']['source_id']!r} outranks it"}
                            for c in cands if c["source_id"] not in prefer]
    else:
        res = ks.pick(cands)
    return {
        "name": name,
        "new": new,
        "chosen": view(reg, res["chosen"]),
        "rejected": [{**view(reg, r["candidate"]), "reason": r["reason"]} for r in res["rejected"]],
        "candidate_count": len(cands),
        "warning": res["warning"],
    }


def existing_named(folder: Path, stem: str) -> list:
    """Files in folder whose stem matches (case-insensitive): the texture stager's collision rule."""
    if not folder.is_dir():
        return []
    return sorted(p for p in folder.iterdir() if p.is_file() and p.stem.lower() == stem.lower())


def row(path: Path, kind: str, status: str, source_id, removal_group, credits, approved: bool = False) -> dict:
    return {"path": str(path), "kind": kind, "status": status, "source_id": source_id,
            "removal_group": removal_group, "flagged": bool(removal_group) and not approved,
            "credits": list(credits or [])}


def run_tool(cmd: list, what: str) -> dict:
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.stderr:
        sys.stderr.write(proc.stderr)
        sys.stderr.flush()
    if proc.returncode != 0:
        tail = (proc.stderr or proc.stdout).strip().splitlines()[-3:]
        raise Refused(f"{what} failed (exit {proc.returncode}): {' | '.join(tail)}")
    try:
        return json.loads(proc.stdout)
    except json.JSONDecodeError as e:
        raise Refused(f"{what} printed no JSON manifest: {e}") from e


def stage_texture(stage: Path, ledger: Path, install: Path, cand: dict, old: str, new: str, tmp: Path,
                  allow_overlay: bool = False) -> dict:
    fd, map_name = tempfile.mkstemp(suffix=".json", dir=str(tmp))
    with os.fdopen(fd, "w", encoding="utf-8") as fh:
        json.dump({old: new}, fh)
    cmd = [sys.executable, "-I", "-B", "-u", str(TEXTURE_TOOL), "--source-id", cand["source_id"],
           "--map-json", map_name, "--stage-dir", str(stage), "--ledger", str(ledger),
           "--pack-name", cand["source_id"]]
    if cand["source_id"] == VANILLA:
        cmd += ["--from-install", str(install)]
        if allow_overlay:
            cmd.append(kt.OVERLAY_FLAG)
    else:
        cmd += ["--pack-dir", cand["pack_dir"]]
    return run_tool(cmd, f"texture {old} -> {new} from {cand['source_id']}")


def run_model(stage: Path, ledger: Path, install: Path, cand: dict, resref: str, new: str, pairs: list,
              allow_overlay: bool = False, supermodel_map: list | None = None) -> dict:
    cmd = [sys.executable, "-I", "-B", "-u", str(MODEL_TOOL), "--resref", resref, "--new-resref", new,
           "--tex-map", *[f"{o}={n}" for o, n in pairs], "--out-dir", str(stage / MODEL_DIR),
           "--ledger", str(ledger)]
    if supermodel_map:
        cmd += ["--supermodel-map", *supermodel_map]
    if cand["source_id"] == VANILLA:
        cmd += ["--install-k1", str(install), "--source-id", VANILLA]
        if allow_overlay:
            cmd.append(kt.OVERLAY_FLAG)
    else:
        cmd += ["--pack-dir", cand["pack_dir"], "--source-id", cand["source_id"]]
    return run_tool(cmd, f"model {resref} -> {new} from {cand['source_id']}")


def _validate_outputs(model_dir: Path, resref: str) -> list:
    """Findings for the written MDL/MDX pair: K2 layout and lowercase names (--k1p). Empty list = clean."""
    try:
        mdl = (model_dir / f"{resref}.mdl").read_bytes()
        mdx = (model_dir / f"{resref}.mdx").read_bytes()
    except OSError as e:
        return [kv.Finding("UNREADABLE", f"cannot read the written model pair: {e}", 0)]
    return kv.validate(mdl, mdx, game="K2", require_lower_k1p=True)


def _discard_model(ledger_path: Path, model_dir: Path, resref: str) -> None:
    """Removes a rejected pair: its files and ledger entries (the same rollback k1_port_model.py does on failure)."""
    led = ks.Ledger(ledger_path)
    for ext in MODEL_EXTS:
        path = model_dir / f"{resref}{ext}"
        with contextlib.suppress(OSError):
            path.unlink()
        led.remove(path)
    led.save()


def gate_model(args, ledger_path: Path, model: dict, new_resref: str, notes: list) -> None:
    """Validates the pair this run just wrote. Any finding rejects the model: nothing from it stays registered.
    --skip-validate keeps it instead, with a loud warning on stderr and a note in the report."""
    model_dir = Path(model["manifest"]["out_dir"])
    if args.skip_validate:
        print(SKIP_WARNING.format(name=new_resref), file=sys.stderr)
        notes.append(SKIP_NOTE.format(name=new_resref))
        model["validation"] = {"status": "skipped", "findings": []}
        return
    findings = _validate_outputs(model_dir, new_resref)
    model["validation"] = {"status": "failed" if findings else "passed",
                           "findings": [asdict(f) for f in findings]}
    if findings:
        _discard_model(ledger_path, model_dir, new_resref)
        model["status"] = "rejected"


def resolve_ledger(arg, stage: Path) -> Path:
    if arg is not None:
        return arg.expanduser().absolute()
    env = os.environ.get("K1PORT_LEDGER")
    if env:
        return Path(env).expanduser().absolute()
    return stage / "ledger.json"


def ingest(args, mapping: dict, new_resref: str, out: dict) -> None:
    """Plans every asset read only, then writes them unless --dry-run. Fills out in place."""
    stage = args.stage_dir.expanduser().absolute()
    roots = kt.repo_roots()
    reason = kt.stage_refusal(stage, roots)
    if reason:
        raise Refused(f"refused --stage-dir {stage}: {reason}")
    ledger_path = resolve_ledger(args.ledger, stage)
    reason = kt.stage_refusal(ledger_path, roots)
    if reason:
        raise Refused(f"refused --ledger {ledger_path}: {reason}")
    install = args.install_k1.expanduser()
    reason = kt.overlay_refusal(install, args.allow_overlay_install)
    if reason:
        raise Refused(reason)
    overlay = kt.overlay_marker(install)
    errs: list = []
    read_tpc = kt.import_read_tpc(errs)
    if read_tpc is None:
        raise Refused(errs[0])
    reg = ks.load_registry()
    ledger = ks.Ledger(ledger_path)
    notes: list = []
    olds = list(mapping)
    want_model = not args.no_model
    vanilla = vanilla_lookup(install, args.resref, olds, want_model, read_tpc, notes, overlay)
    packs = pack_candidates(reg, args.resref, olds, want_model, read_tpc, notes,
                            allow_variants=args.allow_variant_folders)
    searched = ([VANILLA] if vanilla["present"] else []) + packs["searched"]
    out.update(install_found=vanilla["present"], stage_dir=str(stage), ledger=str(ledger_path), notes=notes)

    tex = []
    for old in olds:
        cands = ([vanilla["textures"][old]] if old in vanilla["textures"] else []) + packs["textures"][old]
        d = decide(reg, old, mapping[old], cands, searched, tuple(args.prefer_source))
        if not args.allow_variant_folders and chosen_from_variant(reg, d["chosen"]):
            notes.append(f"{old} from {d['chosen']['source_id']}: {VARIANT_NOTE}")
        d["hd"] = None
        if args.also_hd:
            higher = [r for r in d["rejected"] if r["requires_author_approval"]
                      and r["quality"] > d["chosen"]["quality"]]
            if higher:
                best = sorted(higher, key=lambda r: (-r["quality"], r["source_id"], r["path"]))[0]
                d["hd"] = {"chosen": {k: v for k, v in best.items() if k != "reason"}}
        tex.append(d)

    model = None
    if want_model:
        cands = ([vanilla["model"]] if vanilla["model"] else []) + packs["model"]
        model = decide(reg, args.resref, new_resref, cands, searched)
        if not args.allow_variant_folders and chosen_from_variant(reg, model["chosen"]):
            notes.append(f"{args.resref} model from {model['chosen']['source_id']}: {VARIANT_NOTE}")

    # Existing outputs: reported, never rewritten.
    for d in tex:
        d["files"] = [str(p) for p in existing_named(stage / "override", d["new"])]
        d["status"] = "already_ingested" if d["files"] else "planned"
        if d["hd"]:
            d["hd"]["files"] = [str(p) for p in existing_named(stage / HD_DIR / "override", d["new"])]
            d["hd"]["status"] = "already_ingested" if d["hd"]["files"] else "planned"
    if model:
        model["files"] = [str(p) for p in existing_named(stage / MODEL_DIR, new_resref)]
        model["status"] = "already_ingested" if model["files"] else "planned"

    out.update(textures=tex, model=model)
    pairs = [(d["name"], d["new"]) for d in tex]
    if not args.dry_run:
        with tempfile.TemporaryDirectory(prefix="k1port_ingest_") as td:
            tmp = Path(td)
            for d in tex:
                if d["status"] == "planned":
                    d["manifest"] = stage_texture(stage, ledger_path, install, d["chosen"], d["name"], d["new"], tmp,
                                                    args.allow_overlay_install)
                    d["status"] = "staged"
                if d["hd"] and d["hd"]["status"] == "planned":
                    d["hd"]["manifest"] = stage_texture(stage / HD_DIR, ledger_path, install, d["hd"]["chosen"],
                                                        d["name"], d["new"], tmp, args.allow_overlay_install)
                    d["hd"]["status"] = "staged"
            if model and model["status"] == "planned":
                model["manifest"] = run_model(stage, ledger_path, install, model["chosen"], args.resref,
                                              new_resref, pairs, args.allow_overlay_install,
                                              args.supermodel_map)
                model["status"] = "staged"
    if model and model["status"] == "staged":
        gate_model(args, ledger_path, model, new_resref, notes)

    # Every output of this run (planned, staged or already there), with its provenance.
    by_file = {e.get("file"): e for e in ledger.entries()}
    rows = []

    def existing_row(path: Path, kind: str):
        e = by_file.get(str(path))
        if e is None:
            rows.append(row(path, kind, "already_ingested", None, None, [], approved=True))
            notes.append(f"{path} exists but is not in the ledger; not flagged")
            return
        rows.append(row(path, kind, "already_ingested", e.get("source_id"), e.get("removal_group"),
                        e.get("credits"), approved=bool(e.get("author_approved"))))

    def texture_rows(entry: dict, new: str, folder: Path):
        if entry["status"] == "staged":
            man = entry["manifest"]
            for e in man["files"]:
                rows.append(row(Path(man["stage_dir"]) / e["staged_rel_path"], "texture", "staged",
                                e["source_id"], e["removal_group"], e["credits"]))
        elif entry["status"] == "planned":
            c = entry["chosen"]
            rows.append(row(folder / f"{new}{c['ext']}", "texture", "planned", c["source_id"],
                            c["removal_group"], c["credits"]))
        else:
            for p in entry["files"]:
                existing_row(Path(p), "texture")

    for d in tex:
        texture_rows(d, d["new"], stage / "override")
        if d["hd"]:
            texture_rows(d["hd"], d["new"], stage / HD_DIR / "override")
    if model:
        mdir = stage / MODEL_DIR
        if model["status"] == "staged":
            man = model["manifest"]
            for ext in (".mdl", ".mdx"):
                rows.append(row(Path(man["out_dir"]) / f"{new_resref}{ext}", "model", "staged", man["source_id"],
                                man["removal_group"], man["credits"]))
            model["match"] = man["match"]
            model["failed_flags"] = sorted(k for k, v in man.get("compare", {}).items() if not v)
        elif model["status"] == "planned":
            c = model["chosen"]
            for ext in (".mdl", ".mdx"):
                rows.append(row(mdir / f"{new_resref}{ext}", "model", "planned", c["source_id"],
                                c["removal_group"], c["credits"]))
        else:
            for p in model["files"]:
                existing_row(Path(p), "model")
    out["outputs"] = rows
    out["flagged_files"] = [r["path"] for r in rows if r["flagged"]]
    out["credits_needed"] = sorted({c for r in rows for c in r["credits"]})
    if model and model["status"] == "rejected":
        found = model["validation"]["findings"]
        codes = ", ".join(sorted({f["code"] for f in found}))
        raise Refused(f"model {args.resref} -> {new_resref} rejected by validation ({len(found)} finding(s): {codes}); "
                      "its files and ledger entries were removed. Fix the source or the converter and re-run.")


def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(
        description="Ingest one KOTOR 1 model and its textures into the K1 port stage. "
                    "Approval-free sources are preferred; flagged files are removable later (k1_port_prune.py).")
    ap.add_argument("--resref", required=True, help="K1 model resref, e.g. n_commm")
    ap.add_argument("--new-resref", default=None, metavar="NEW",
                    help=f"K2 output resref, lowercase, max 16 chars (default: {NEW_PREFIX} + resref, cut to 16)")
    ap.add_argument("--texture", action="append", required=True, metavar="OLD=NEW",
                    help="texture rename, repeatable; NEW is lowercase, max 31 chars")
    ap.add_argument("--stage-dir", type=Path, default=kt.DEFAULT_STAGE,
                    help=f"stage root (default: {kt.DEFAULT_STAGE})")
    ap.add_argument("--install-k1", type=Path, default=Path(DEFAULT_INSTALL),
                    help=f"K1 install, read only (default: {DEFAULT_INSTALL})")
    ap.add_argument(kt.OVERLAY_FLAG, action="store_true",
                    help="allow --install-k1 to be a K1 HD overlay; its override/ files are then attributed to the "
                         "overlay marker's flagged source, never vanilla_k1 (refused by default)")
    ap.add_argument("--prefer-source", action="append", default=[], metavar="ID",
                    help="textures only, repeatable: a candidate from this source outranks all others, even "
                         "unflagged vanilla_k1 (the ledger row keeps its flag); default: none")
    ap.add_argument("--ledger", type=Path, default=None,
                    help="removal ledger (default: env K1PORT_LEDGER, else <stage>/ledger.json)")
    ap.add_argument("--also-hd", action="store_true",
                    help="also stage a higher-quality flagged variant (needs approval) under <stage>/_hd/override/")
    ap.add_argument("--supermodel-map", nargs="+", metavar="OLD=NEW", default=None,
                    help="model header supermodel rewrite, passed to k1_port_model.py (OLD = K1-only name, NEW = "
                         "staged K2 resref)")
    ap.add_argument("--no-model", action="store_true", help="textures only: skip the model conversion")
    ap.add_argument("--allow-variant-folders", action="store_true",
                    help="let Optional*/ and Upscale* folders of a source compete on equal terms "
                         "(default: their files sort after the canonical ones)")
    ap.add_argument("--skip-validate", action="store_true",
                    help="DEBUGGING ONLY: register the converted model without the structural validator "
                         "(prints a loud warning)")
    ap.add_argument("--dry-run", action="store_true", help="print the decisions; write nothing")
    ap.add_argument("--json", action="store_true", help="print the summary as JSON (default: short text)")
    return ap


def parse_textures(parser: argparse.ArgumentParser, items: list) -> dict:
    mapping: dict = {}
    for item in items:
        old, sep, new = item.partition("=")
        if not sep or not old or not new:
            parser.error(f"--texture {item!r} must be OLD=NEW")
        if old in mapping:
            parser.error(f"--texture OLD {old!r} is given twice")
        mapping[old] = new
    errors: list = []
    kt.validate_names(mapping, errors)
    if errors:
        parser.error("; ".join(errors))
    return mapping


def print_text(s: dict) -> None:
    mode = "dry-run: nothing written" if s["dry_run"] else s["status"]
    print(f"ingest {s['resref']} -> {s['new_resref']} [{mode}]")
    print(f"K1 install found: {'yes' if s['install_found'] else 'no'}   ledger: {s['ledger']}")
    for t in s["textures"]:
        c = t["chosen"]
        print(f"texture {t['name']} -> {t['new']}: {t['status']}  from {c['source_id']} "
              f"({'FLAGGED' if c['requires_author_approval'] else 'approval-free'}, quality {c['quality']})")
        for r in t["rejected"]:
            print(f"  rejected {r['source_id']}: {r['reason']}")
        if t["warning"]:
            print(f"  WARN {t['warning']}")
        if t["hd"]:
            h = t["hd"]
            print(f"  hd {h['chosen']['source_id']} (quality {h['chosen']['quality']}): {h['status']}")
    if s["model"]:
        m = s["model"]
        print(f"model {s['resref']} -> {s['new_resref']}: {m['status']}  from {m['chosen']['source_id']} "
              f"({'FLAGGED' if m['chosen']['requires_author_approval'] else 'approval-free'})")
        for r in m["rejected"]:
            print(f"  rejected {r['source_id']}: {r['reason']}")
        v = m.get("validation") or {}
        if v:
            print(f"  validation {v['status']}")
            for f in v["findings"]:
                print(f"  finding {f['code']} @0x{f['offset']:x}: {f['message']}")
        if "match" in m:
            print(f"  match: {m['match']}" + (f"  failed: {', '.join(m['failed_flags'])}" if m["failed_flags"] else ""))
    for r in s["outputs"]:
        flag = "FLAGGED " if r["flagged"] else ""
        print(f"  {r['status']:<17} {r['kind']:<8} {flag}{r['path']}")
    print(f"credits needed: {', '.join(s['credits_needed']) or 'none'}")
    for n in s["notes"]:
        print(f"note: {n}")
    for e in s["errors"]:
        print(f"error: {e}", file=sys.stderr)


def main(argv=None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    if not RESREF_RE.fullmatch(args.resref):
        parser.error(f"--resref {args.resref!r} must be 1-16 ASCII letters, digits or '_'")
    new_resref = args.new_resref if args.new_resref is not None else default_new_resref(args.resref)
    if not NEW_RESREF_RE.fullmatch(new_resref):
        parser.error(f"--new-resref {new_resref!r} must be 1-16 chars of [a-z0-9_]")
    mapping = parse_textures(parser, args.texture)

    out: dict = {"status": "failed", "dry_run": bool(args.dry_run), "resref": args.resref,
                 "new_resref": new_resref, "install_k1": str(args.install_k1.expanduser()),
                 "install_found": False, "stage_dir": None, "ledger": None, "textures": [], "model": None,
                 "outputs": [], "flagged_files": [], "credits_needed": [], "notes": [], "errors": []}
    code = EXIT_OK
    try:
        ingest(args, mapping, new_resref, out)
        out["status"] = "dry_run" if args.dry_run else "ok"
    except (Refused, ks.SourceError) as e:
        out["errors"].append(str(e))
        code = EXIT_FAILED
    if args.json:
        print(json.dumps(out, indent=2))
    else:
        print_text(out)
    return code


if __name__ == "__main__":
    sys.exit(main())
