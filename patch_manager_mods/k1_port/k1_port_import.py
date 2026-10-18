#!/usr/bin/env python3
"""Install-time K1 -> K2 character importer (doc 38 s26): one `.k2pack` per character, built from YOUR K1 install.

  python3 k1_port_import.py doctor  [--config C]
  python3 k1_port_import.py build   [--config C] [--only ID] [--dry-run] [--reassign]
  python3 k1_port_import.py install [--config C] [--only ID] [--dry-run]
  python3 k1_port_import.py remove  ID [--config C]
  python3 k1_port_import.py list    [--config C] [--json]

The config (see k1_imports.example.toml; copy it to a PRIVATE place) names the K1 dir, the K2 dir, the output dir
and the characters to build. Each character is a committed recipe (recipes/<id>.toml). The driver is stdlib only; it
chains the existing step scripts as subprocesses, each in the interpreter it needs:
  k1port venv python (env K1PORT_PY)   k1_port_resolve, k1_port_ingest, k1_port_char_build, k1_port_mdlops extract/pass2
  repo .venv python  (env REPO_VENV_PY) k1_port_tex_tpc (numpy)
  this python (env K1PORT_SYS_PY)      k1_port_prune gate/adopt, k1_port_validate, k1_port_mdlops rename/pass1

build, per character in config order (every step logs to <out>/<id>/logs/):
  1 resolve  every K1 model (mdl+mdx) and texture (tpc or tga) the recipe names -> <out>/<id>/licence_report.json
  2 gate     the out dir must be PRIVATE (not in a git work tree, not under steamapps/common or compatdata, no
             symlink on the path, not inside the K1/K2 dir), else exit 2. Flagged sources (K1 Override files, an HD
             overlay, Characters Revisited / anything pending_author_approval) are allowed in a private out only:
             WARN + recorded in the manifest. k1_port_prune gate runs (non-strict) on the staged files.
  3 models   converter pykotor -> k1_port_ingest (per model, with its textures); converter mdlops -> k1_port_mdlops
             extract/pass1/rename/pass2, then validate + copy into the stage (the script's own `finish` writes into
             the shared hand-path stage, so the driver does that step itself)
  4 textures <k2>.tpc per recipe texture: DXT1 (k1_port_tex_tpc tga-to-tpc for TGA input, --force-opaque, --txi
             with the recipe text; a staged DXT1 TPC is used as is, its TXI trailer replaced by the recipe txi when one is set)
  5 rows     appearance/heads row ids come from the live K2 tables (<K2>/override/appearance.2da, heads.2da) in
             config order, chained with --chain-from (a pad step covers [build] first_appearance_row/first_head_row
             above the live count); ids are persisted in <out>/assignments.json and reused. If the live counts moved
             the build refuses unless --reassign (then every configured character is rebuilt).
  6 pack     <out>/<id>.k2pack (re-read with read_pack) and <out>/<id>.import_manifest.json
Whole-character rebuilds start from a clean <out>/<id>/ work dir. --dry-run resolves, gates and prints the planned
commands and writes nothing.

install copies <out>/<id>.k2pack to <K2>/k1_imports/ (the only game-dir write), verifies it and records it in
<out>/installed.json. remove deletes that one file (only if installed.json lists it) and drops the assignment.
Close the game before install/remove.

Exit: 0 ok, 1 failure (message = location, cause, fix), 2 safety refusal.
"""
from __future__ import annotations

import argparse
import datetime
import hashlib
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path
sys.path.insert(0, str(HERE.parent / "kvoice"))

import k1_port_pack as kpack  # noqa: E402  stdlib only
import k1_port_recipe as krecipe  # noqa: E402  stdlib only (tomllib at load time)
import k1_port_resolve as kresolve  # noqa: E402  stdlib only; pykotor is used in the resolve subprocess
import k1_port_sources as ksources  # noqa: E402  stdlib only

DRIVER = Path(__file__).resolve()
REPO_ROOT = HERE.parent.parent
DRIVER_VERSION = 1
EXIT_OK, EXIT_FAIL, EXIT_SAFETY = 0, 1, 2

DEFAULT_CONFIG = "~/.config/k1_port/k1_imports.toml"
DEFAULT_K1PORT_PY = "~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python"
DEFAULT_CR_DIR = "~/Games/KOTOR2_Mod_Workspace/k1_port_extract/characters_revisited"
VANILLA_ID = "vanilla_k1"
CR_ID = "characters_revisited_main"
JUHANI_CHAR_IDS = ("juhani-char", "juhanv-char", "carth-char")  # hand-path DLL bundles with virtual rows 818-820 / 249-251

RESOLVE_TOOL = HERE / "k1_port_resolve.py"
INGEST_TOOL = HERE / "k1_port_ingest.py"
MDLOPS_TOOL = HERE / "k1_port_mdlops.py"
CHAR_BUILD_TOOL = HERE / "k1_port_char_build.py"
TEX_TOOL = HERE / "k1_port_tex_tpc.py"
PRUNE_TOOL = HERE / "k1_port_prune.py"
VALIDATE_TOOL = HERE / "k1_port_validate.py"
STEP_TOOLS = (RESOLVE_TOOL, INGEST_TOOL, MDLOPS_TOOL, CHAR_BUILD_TOOL, TEX_TOOL, PRUNE_TOOL, VALIDATE_TOOL)

ID_RE = krecipe.ID_RE
GATE_RE = re.compile(r"gate: scanned (\d+) file\(s\), blocked (\d+), warned (\d+)")
PACK_SUFFIX = ".k2pack"
IMPORT_DIR = "k1_imports"


class Failure(Exception):
    """A problem the user must fix. code 1 = failure, 2 = safety refusal. str() = location: cause. Fix: ..."""

    code = EXIT_FAIL

    def __init__(self, location: str, cause: str, fix: str = ""):
        self.location, self.cause, self.fix = location, cause, fix
        super().__init__(f"{location}: {cause}" + (f". Fix: {fix}" if fix else ""))


class Refusal(Failure):
    code = EXIT_SAFETY


def warn(msg: str) -> None:
    print(f"WARN: {msg}", file=sys.stderr, flush=True)


def say(msg: str) -> None:
    print(msg, flush=True)


# ------------------------------------------------------------------------------------------- small helpers

def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    return sha256_bytes(Path(path).read_bytes())


def now_iso() -> str:
    return datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds")


def write_json(path: Path, obj) -> None:
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp = tempfile.mkstemp(dir=str(path.parent), prefix=f".{path.name}.", suffix=".tmp")
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as fh:
            json.dump(obj, fh, indent=2, sort_keys=True)
            fh.write("\n")
        os.replace(tmp, path)
    except BaseException:
        with _suppress(OSError):
            os.unlink(tmp)
        raise


class _suppress:
    def __init__(self, *exc):
        self.exc = exc

    def __enter__(self):
        return self

    def __exit__(self, et, ev, tb):
        return et is not None and issubclass(et, self.exc)


def read_json(path: Path, what: str):
    try:
        return json.loads(Path(path).read_text(encoding="utf-8"))
    except FileNotFoundError:
        return None
    except (OSError, json.JSONDecodeError) as e:
        raise Failure(str(path), f"cannot read {what}: {e}", f"fix or delete {path}") from None


def k1port_py() -> Path:
    return Path(os.environ.get("K1PORT_PY") or DEFAULT_K1PORT_PY).expanduser()


def stdlib_py() -> Path:
    """Interpreter for the stdlib-only steps (mdlops rename/pass1, validate, prune). Env K1PORT_SYS_PY overrides (tests)."""
    return Path(os.environ.get("K1PORT_SYS_PY") or sys.executable)


def repo_py() -> Path:
    return Path(os.environ.get("REPO_VENV_PY") or (REPO_ROOT / ".venv" / "bin" / "python")).expanduser()


def tail_lines(text: str, n: int = 4) -> str:
    lines = [ln for ln in (text or "").strip().splitlines() if ln.strip()]
    return " | ".join(lines[-n:])


# ------------------------------------------------------------------------------------------- subprocess runner

class Runner:
    """Runs step scripts as subprocesses, logs each to a file and records command line + exit code."""

    def __init__(self, log_dir: Path | None = None):
        self.log_dir = log_dir
        self.records: list[dict] = []

    def run(self, label: str, argv: list, *, env: dict | None = None, check: bool = True,
            fix: str = "") -> subprocess.CompletedProcess:
        argv = [str(a) for a in argv]
        full_env = dict(os.environ)
        full_env.update(env or {})
        try:
            proc = subprocess.run(argv, capture_output=True, text=True, env=full_env)
        except OSError as e:
            self.records.append({"label": label, "argv": argv, "rc": None})
            raise Failure(label, f"cannot start {argv[0]}: {e}", fix or "check K1PORT_PY / REPO_VENV_PY and the venv setup (doctor)") from None
        log = None
        if self.log_dir is not None:
            self.log_dir.mkdir(parents=True, exist_ok=True)
            log = self.log_dir / f"{len(self.records) + 1:02d}_{re.sub(r'[^A-Za-z0-9_.-]', '_', label)}.log"
            log.write_text(f"$ {shlex.join(argv)}\nrc={proc.returncode}\n--- stdout\n{proc.stdout}\n--- stderr\n{proc.stderr}\n",
                           encoding="utf-8")
        self.records.append({"label": label, "argv": argv, "rc": proc.returncode, "log": str(log) if log else None})
        if check and proc.returncode != 0:
            raise Failure(label, f"exit {proc.returncode}: {tail_lines(proc.stderr or proc.stdout)}",
                          fix or (f"see {log}" if log else "re-run the failing step by hand"))
        return proc


def py_cmd(py: Path, script: Path, *args) -> list:
    return [str(py), "-I", "-B", "-u", str(script), *[str(a) for a in args]]


# ------------------------------------------------------------------------------------------- safety checks

def out_safety(out: Path, protected: list[Path] = ()) -> list[str]:
    """Reasons the out dir is NOT private (empty list = private). Creates nothing."""
    reasons: list[str] = []
    raw = Path(os.path.abspath(Path(out).expanduser()))
    for p in (raw, *raw.parents):
        if p.is_symlink():
            reasons.append(f"{p} is a symlink on the out path")
            break
    real = raw.resolve()
    for p in (real, *real.parents):
        if (p / ".git").exists():
            reasons.append(f"inside a git work tree ({p})")
            break
    parts = [x.lower() for x in real.parts]
    if any(a == "steamapps" and b == "common" for a, b in zip(parts, parts[1:])):
        reasons.append("under steamapps/common (a game install)")
    if "compatdata" in parts:
        reasons.append("under compatdata (a Proton prefix)")
    for prot in protected:
        pr = Path(prot).expanduser().resolve()
        if real == pr or pr in real.parents:
            reasons.append(f"inside the game dir {pr}")
    return reasons


def require_private_out(out: Path, protected: list[Path]) -> None:
    reasons = out_safety(out, protected)
    if reasons:
        raise Refusal(f"[build] out = {out}", "not a private output dir: " + "; ".join(reasons)
                      + " (packs hold converted game data)",
                      "set [build] out to a private dir outside any git work tree and game install, "
                      "for example ~/.cache/k1_port/packs")


def k2_has_game(k2: Path) -> bool:
    try:
        names = {e.name.lower() for e in os.scandir(k2)}
    except OSError:
        return False
    return "swkotor2.exe" in names or "chitin.key" in names


def require_k2(k2: Path) -> None:
    if not k2_has_game(k2):
        raise Failure(f"[k2] dir = {k2}", "not a KOTOR II install (no swkotor2.exe or chitin.key)",
                      "set [k2] dir in the config to the folder that holds swkotor2.exe")


def installed_juhani_char(k2: Path) -> list[str]:
    """Hand-path character DLLs present in the K2 install (read only): patch_config.toml ids and patches/<id>.dll."""
    found: list[str] = []
    cfg = k2 / "patch_config.toml"
    if cfg.is_file():
        try:
            import tomllib
            ids = {p.get("id") for p in tomllib.loads(cfg.read_text(encoding="utf-8")).get("patches", [])}
        except Exception:  # noqa: BLE001  unreadable config: fall back to the DLL check only
            ids = set()
        found += [i for i in JUHANI_CHAR_IDS if i in ids]
    for i in JUHANI_CHAR_IDS:
        dll = k2 / "patches" / f"{i}.dll"
        if dll.is_file() and i not in found:
            found.append(i)
    return found


JUHANI_WARN = ("{ids} is installed in the K2 dir: it registers virtual appearance rows 818-820 and heads rows "
               "249-251, which collide with the rows this importer assigns. Remove it (KPM uninstall) or set "
               "[build] first_appearance_row = 821 and first_head_row = 252 (above its rows).")
JUHANI_ROWS = {"appearance": range(818, 821), "head": range(249, 252)}


def juhani_rows_clash(assigned: dict | None) -> bool:
    """True when any assigned row falls on juhani-char's virtual rows (or nothing is assigned yet: warn to be safe)."""
    if not assigned:
        return True
    return any(int(r.get(k, -1)) in JUHANI_ROWS[k] for r in assigned.values() for k in JUHANI_ROWS)


# ------------------------------------------------------------------------------------------- config

@dataclass
class Loaded:
    cfg: krecipe.Config
    recipes: list
    out: Path

    @property
    def k2_imports(self) -> Path:
        return self.cfg.k2_dir / IMPORT_DIR


def load_all(config_arg: str | None) -> Loaded:
    path = Path(config_arg or DEFAULT_CONFIG).expanduser()
    if not path.is_file():
        raise Failure(str(path), "config file not found",
                      "copy patch_manager_mods/k1_port/k1_imports.example.toml to a private place, edit it, "
                      "and pass --config")
    try:
        cfg = krecipe.load_config(path)
        recipes = krecipe.load_recipes(cfg.recipes_dir, cfg.characters)
    except krecipe.RecipeError as e:
        raise Failure(str(path), str(e), "fix the config or recipe named in the message") from None
    return Loaded(cfg=cfg, recipes=recipes, out=Path(os.path.abspath(cfg.out)))


def select(loaded: Loaded, only: str | None) -> list:
    if only is None:
        return list(loaded.recipes)
    chosen = [r for r in loaded.recipes if r.id == only]
    if not chosen:
        raise Failure("--only", f"{only!r} is not in [build] characters {list(loaded.cfg.characters)}",
                      "use one of the configured ids")
    return chosen


# ------------------------------------------------------------------------------------------- 2DA helpers (hidden _twoda)

def _twoda_main(argv: list) -> int:
    """Hidden helper, run under the k1port venv python: height | rows | pad (pykotor, via the vfs_pack helpers)."""
    op = argv[0]
    try:
        if op == "height":
            from pykotor.resource.formats.twoda import read_2da
            print(json.dumps({"height": read_2da(Path(argv[1]).read_bytes()).get_height()}))
        elif op == "rows":
            import k1_port_vfs_pack as vp
            spec = vp.parse_2da_row_spec(argv[1])
            print(json.dumps({t: {str(r): cols for r, cols in rows.items()} for t, rows in spec.items()}))
        elif op == "pad":
            from pykotor.resource.formats.twoda import bytes_2da, read_2da
            table = read_2da(Path(argv[1]).read_bytes())
            headers = list(table.get_headers())
            target = int(argv[3])
            while table.get_height() < target:
                table.add_row(str(table.get_height()), {c: "" for c in headers})
            Path(argv[2]).write_bytes(bytes_2da(table))
            print(json.dumps({"height": table.get_height()}))
        else:
            print(f"unknown _twoda op {op!r}", file=sys.stderr)
            return 2
    except Exception as e:  # noqa: BLE001  pykotor raises several error types
        print(f"error: _twoda {op}: {type(e).__name__}: {e}", file=sys.stderr)
        return 1
    return 0


def twoda_call(runner: Runner, label: str, *args) -> dict:
    proc = runner.run(label, [str(k1port_py()), "-I", "-B", str(DRIVER), "_twoda", *[str(a) for a in args]],
                      fix="run the importer's doctor: the k1port venv (pykotor) must work")
    try:
        return json.loads(proc.stdout)
    except json.JSONDecodeError as e:
        raise Failure(label, f"helper printed no JSON: {e}", "re-run doctor") from None


def twoda_height(runner: Runner, path: Path) -> int:
    if not Path(path).is_file():
        raise Failure(str(path), "2DA table not found",
                      "the K2 install needs override/appearance.2da and override/heads.2da (char_build reads them)")
    return int(twoda_call(runner, f"2da-height {Path(path).name}", "height", path)["height"])


# ------------------------------------------------------------------------------------------- row assignment

def live_tables(k2: Path) -> Path:
    return k2 / "override"


def plan_assignments(loaded: Loaded, runner: Runner, reassign: bool) -> tuple[dict, dict, dict]:
    """(planned {id: {appearance, head}}, live {appearance, head}, stored). Refuses when stored ids no longer fit."""
    cfg = loaded.cfg
    live_dir = live_tables(cfg.k2_dir)
    live = {"appearance": twoda_height(runner, live_dir / "appearance.2da"),
            "head": twoda_height(runner, live_dir / "heads.2da")}
    start_a = live["appearance"] if cfg.first_appearance_row is None else cfg.first_appearance_row
    start_h = live["head"] if cfg.first_head_row is None else cfg.first_head_row
    for what, start, height, key in (("appearance", start_a, live["appearance"], "first_appearance_row"),
                                     ("heads", start_h, live["head"], "first_head_row")):
        if start < height:
            raise Failure(f"[build] {key} = {start}", f"below the live {what}.2da row count ({height}); it would overwrite rows",
                          f"use {height} or higher, or remove the key")
    planned = {cid: {"appearance": start_a + i, "head": start_h + i} for i, cid in enumerate(cfg.characters)}
    stored = read_json(loaded.out / "assignments.json", "assignments") or {}
    if not isinstance(stored, dict):
        raise Failure(str(loaded.out / "assignments.json"), "not a JSON object", "delete it (rows are reassigned on the next build)")
    clash = []
    for cid in cfg.characters:
        old = stored.get(cid)
        if old is not None and old != planned[cid]:
            clash.append(f"{cid}: stored appearance {old.get('appearance')}/head {old.get('head')}, "
                         f"plan now {planned[cid]['appearance']}/{planned[cid]['head']}")
    if clash and not reassign:
        warn("assigned rows no longer fit the live K2 tables or the config order: " + "; ".join(clash))
        raise Failure(str(loaded.out / "assignments.json"),
                      f"stored row ids differ from the plan (live appearance.2da has {live['appearance']} rows, heads.2da {live['head']})",
                      "remove the installed packs and rebuild everything with --reassign, or restore the tables/config order")
    return planned, live, stored


def save_assignment(loaded: Loaded, stored: dict, cid: str, rows: dict) -> dict:
    stored = dict(stored)
    stored[cid] = dict(rows)
    write_json(loaded.out / "assignments.json", stored)
    return stored


# ------------------------------------------------------------------------------------------- resolve step

def model_refs(recipe) -> list:
    return [m for m in (recipe.body_a, recipe.body_b, recipe.head) if m is not None]


def resolve_items(recipe) -> list[str]:
    items = []
    for m in model_refs(recipe):
        items += [f"{m.k1}.mdl", f"{m.k1}.mdx"]
    for t in recipe.textures:
        items += [f"{t.k1}.tpc", f"{t.k1}.tga"]
    return items


def resolve_character(runner: Runner, loaded: Loaded, recipe, k1: Path) -> dict:
    """Runs k1_port_resolve (k1port venv) for every item; returns {rows, missing, flagged}. Raises when a model is missing."""
    items = resolve_items(recipe)
    scan = [a for t in recipe.textures for a in ("--scan", t.k1)]
    proc = runner.run(f"resolve {recipe.id}", py_cmd(k1port_py(), RESOLVE_TOOL, "--k1-dir", k1, "--json", *scan, *items),
                      check=False)
    try:
        data = json.loads(proc.stdout)
    except json.JSONDecodeError:
        raise Failure(f"resolve {recipe.id}", f"no JSON from k1_port_resolve (exit {proc.returncode}): "
                      f"{tail_lines(proc.stderr or proc.stdout)}",
                      "run doctor; the k1port venv (pykotor) must work and [k1] dir must hold chitin.key") from None
    rows, missing = data.get("items", []), set(data.get("missing", []))
    have = {(r["resref"], r["ext"]) for r in rows}
    problems = []
    for m in model_refs(recipe):
        for ext in ("mdl", "mdx"):
            if (m.k1, ext) not in have:
                problems.append(f"{m.k1}.{ext}")
    for t in recipe.textures:
        if (t.k1, "tpc") not in have and (t.k1, "tga") not in have:
            if loaded.cfg.texture_source == "cr":
                warn(f"{recipe.id}: texture {t.k1} is not in the K1 install; it must come from Characters Revisited")
            else:
                problems.append(f"{t.k1}.tpc/.tga")
    if problems:
        raise Failure(f"resolve {recipe.id}", f"not found in the K1 install {k1}: {', '.join(problems)}",
                      "check [k1] dir points at a full KOTOR 1 install (not K2), and the recipe resrefs")
    flagged = [{"item": f"{r['resref']}.{r['ext']}", "origin": r["origin"], "licence": r["licence"],
                "reason": "K1 Override file (unverified)" if r["origin"] == "override" else r["licence"]}
               for r in rows if r.get("flagged")]
    mesh = {r["resref"]: list(r.get("contains") or []) for r in rows if r["ext"] == "mdl"}
    return {"rows": rows, "missing": sorted(missing), "flagged": flagged, "mesh": mesh}


def textures_by_model(recipe, resolved: dict) -> dict[str, list]:
    """{model k1 name: recipe textures its K1 mesh references}, plus "" for textures no model references.
    k1_port_ingest refuses a --texture OLD the mesh does not use, and a mesh texture left unmapped fails validation
    (Carth body B also uses the head texture P_CarthH01), so each model gets exactly the textures it references."""
    mesh = resolved.get("mesh", {})
    owned: dict[str, list] = {m.k1: [t for t in recipe.textures if t.k1 in mesh.get(m.k1, [])] for m in model_refs(recipe)}
    used = {t.k1 for ts in owned.values() for t in ts}
    owned[""] = [t for t in recipe.textures if t.k1 not in used]
    return owned


# ------------------------------------------------------------------------------------------- registry + ingest

def cr_dir() -> Path:
    return Path(os.environ.get("K1PORT_CR_DEST") or DEFAULT_CR_DIR).expanduser()


def write_registry(path: Path, k1: Path, texture_source: str) -> Path:
    """A private copy of sources.json where only vanilla_k1 (and, for texture_source cr, Characters Revisited) has a local_dir."""
    base = ksources.default_registry_path()
    try:
        reg = json.loads(base.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as e:
        raise Failure(str(base), f"cannot read the source registry: {e}", "restore sources.json") from None
    for sid, rec in reg.get("sources", {}).items():
        rec["local_dir"] = None
        if sid == VANILLA_ID:
            rec["local_dir"] = str(k1)
        elif sid == CR_ID and texture_source == "cr":
            rec["local_dir"] = str(cr_dir())
    write_json(path, reg)
    return path


def ingest_argv(ctx: "Ctx", resref: str, new: str | None, textures: list, *, no_model: bool,
                supermodel_map: list) -> list:
    argv = py_cmd(k1port_py(), INGEST_TOOL, "--resref", resref)
    if new:
        argv += ["--new-resref", new]
    for t in textures:
        argv += ["--texture", f"{t.k1}={t.k2}"]
    argv += ["--stage-dir", ctx.stage, "--ledger", ctx.ledger, "--install-k1", ctx.k1,
             "--prefer-source", CR_ID if ctx.loaded.cfg.texture_source == "cr" else VANILLA_ID]
    if kresolve.kt.overlay_marker(ctx.k1) is not None:
        argv.append(kresolve.kt.OVERLAY_FLAG)
    if supermodel_map:
        argv += ["--supermodel-map", *supermodel_map]
    if no_model:
        argv.append("--no-model")
    argv.append("--json")
    return argv


def run_ingest(ctx: "Ctx", label: str, argv: list) -> dict:
    proc = ctx.runner.run(label, argv, env=ctx.env, check=False)
    try:
        data = json.loads(proc.stdout)
    except json.JSONDecodeError:
        raise Failure(label, f"no JSON from k1_port_ingest (exit {proc.returncode}): {tail_lines(proc.stderr or proc.stdout)}",
                      f"see {ctx.runner.records[-1].get('log')}") from None
    if proc.returncode != 0 or data.get("errors"):
        raise Failure(label, f"ingest failed (exit {proc.returncode}): {'; '.join(data.get('errors') or []) or tail_lines(proc.stderr)}",
                      "fix the cause above (name collision, validation finding, missing source) and rebuild")
    for n in data.get("notes", []):
        say(f"  note: {n}")
    return data


def summarize_ingest(data: dict) -> dict:
    return {"resref": data.get("resref"), "new_resref": data.get("new_resref"),
            "textures": [{"name": t["name"], "new": t["new"], "source_id": t["chosen"]["source_id"],
                          "requires_author_approval": t["chosen"].get("requires_author_approval")}
                         for t in data.get("textures", [])],
            "model_source": (data.get("model") or {}).get("chosen", {}).get("source_id"),
            "flagged_files": data.get("flagged_files", []),
            "credits_needed": data.get("credits_needed", [])}


def ingest_flagged(data: dict) -> list:
    return [{"item": Path(r["path"]).name, "origin": r.get("source_id"), "licence": r.get("removal_group") or "flagged",
             "reason": "pending_author_approval"} for r in data.get("outputs", []) if r.get("flagged")]


# ------------------------------------------------------------------------------------------- MDLOps branch

def mdlops_steps(ctx: "Ctx", recipe) -> list:
    """(label, argv) of the MDLOps route up to pass2; the driver validates + copies the result itself."""
    run = ctx.cdir / "mdlops_run"
    models = [m.k1 for m in model_refs(recipe)]
    pairs = [f"{m.k1}={m.k2}" for m in model_refs(recipe)]
    texs = [f"{t.k1}={t.k2}" for t in recipe.textures]
    plain = [str(stdlib_py()), "-I", "-B", "-u", str(MDLOPS_TOOL)]
    return [
        ("mdlops extract", py_cmd(k1port_py(), MDLOPS_TOOL, "extract", "--run", run, "--k1", ctx.k1, "--models", *models)),
        ("mdlops pass1", [*plain, "pass1", "--run", str(run), "--models", *models]),
        ("mdlops rename", [*plain, "rename", "--run", str(run), "--map", *pairs, *(["--tex", *texs] if texs else [])]),
        ("mdlops pass2", py_cmd(k1port_py(), MDLOPS_TOOL, "pass2", "--run", run, "--k2", ctx.loaded.cfg.k2_dir, "--map", *pairs)),
    ]


def mdlops_finish(ctx: "Ctx", recipe) -> None:
    """validate + copy + ledger adopt for each model (the script's `finish` writes into the shared hand-path stage)."""
    pass2 = ctx.cdir / "mdlops_run" / "pass2"
    out = ctx.stage / "models"
    out.mkdir(parents=True, exist_ok=True)
    for m in model_refs(recipe):
        mdl, mdx = pass2 / f"{m.k2}-ascii-k2-bin.mdl", pass2 / f"{m.k2}-ascii-k2-bin.mdx"
        if not (mdl.is_file() and mdx.is_file()):
            raise Failure(f"mdlops {m.k2}", f"pass2 output missing in {pass2}", f"see {ctx.cdir / 'logs'}")
        ctx.runner.run(f"validate {m.k2}", [str(stdlib_py()), "-I", "-B", str(VALIDATE_TOOL), str(mdl), "--mdx", str(mdx),
                                             "--game", "K2", "--k1p"],
                       fix="the MDLOps output failed the structural validator; do not use it")
        for src, ext in ((mdl, ".mdl"), (mdx, ".mdx")):
            dst = out / f"{m.k2}{ext}"
            shutil.copyfile(src, dst)
            ctx.runner.run(f"adopt {m.k2}{ext}", [str(stdlib_py()), "-I", "-B", str(PRUNE_TOOL), "adopt", "--ledger", str(ctx.ledger),
                                                  "--file", str(dst), "--kind", "model", "--source-id", VANILLA_ID], env=ctx.env)


# ------------------------------------------------------------------------------------------- textures

def tpc_split(data: bytes, label: str) -> bytes:
    """Trailer bytes of a DXT1 TPC; Failure for anything the importer cannot use as is."""
    if len(data) < 128:
        raise Failure(label, f"TPC is {len(data)} bytes, shorter than the 128-byte header", "re-run with a TGA source")
    import struct
    data_size = struct.unpack_from("<I", data, 0)[0]
    w, h = struct.unpack_from("<HH", data, 8)
    ptype, mips = data[12], data[13]
    if data_size == 0 or ptype != 2:
        raise Failure(label, f"staged TPC is not DXT1 (pixel type {ptype}); DXT5/uncompressed needs an encoder this importer does not have",
                      "use [build] texture_source = \"cr\" (TGA sources) or a K1 install whose texture is DXT1")
    total = 128
    for k in range(mips):
        total += (-(-max(1, w >> k) // 4)) * (-(-max(1, h >> k) // 4)) * 8
    if total > len(data):
        raise Failure(label, f"TPC truncated: mip chain needs {total} bytes, file has {len(data)}", "re-stage the texture")
    return data[total:]


def txi_text(tex) -> str:
    """The recipe txi, newline-terminated like the hand path txi file (`clamp 1` plus a newline)."""
    return tex.txi if tex.txi.endswith("\n") else tex.txi + "\n"


def adapt_tpc(data: bytes, tex, label: str) -> bytes:
    """A staged DXT1 TPC as the pack texture. The recipe txi (when set) is authoritative: it replaces the TPC's own
    TXI trailer (K1 heads carry `clamp 3`, the recipe says `clamp 1`). No recipe txi keeps the trailer as it is."""
    trailer = tpc_split(data, label)
    if tex.txi:
        want = " ".join(tex.txi.lower().split())
        have = " ".join(trailer.decode("latin1").lower().replace("\0", " ").split())
        if want != have:
            data = data[:len(data) - len(trailer)] + txi_text(tex).encode("ascii")
    return data


def make_tpc(ctx: "Ctx", tex) -> dict:
    base = tex.k2.lower()
    tga, tpc_in = ctx.stage / "override" / f"{base}.tga", ctx.stage / "override" / f"{base}.tpc"
    dest = ctx.cdir / "tpc" / f"{base}.tpc"
    dest.parent.mkdir(parents=True, exist_ok=True)
    if tga.is_file():
        argv = tpc_argv(ctx, tex, tga, dest)
        if tex.txi:
            (ctx.cdir / "work").mkdir(parents=True, exist_ok=True)
            Path(argv[argv.index("--txi") + 1]).write_text(txi_text(tex), encoding="ascii")
        ctx.runner.run(f"tex {base}", argv, fix="the repo .venv needs numpy (doctor); a TGA with alpha needs --force-opaque (recipe force_opaque)")
        origin = "tga"
    elif tpc_in.is_file():
        dest.write_bytes(adapt_tpc(tpc_in.read_bytes(), tex, base))
        origin = "tpc"
    else:
        raise Failure(f"texture {tex.k2}", f"ingest staged neither {tga.name} nor {tpc_in.name} in {ctx.stage / 'override'}",
                      "check the ingest log for this texture")
    if not dest.is_file() or dest.stat().st_size == 0:
        raise Failure(f"texture {tex.k2}", f"{dest} was not written", f"see {ctx.cdir / 'logs'}")
    return {"k2": tex.k2, "from": origin, "sha256": sha256_file(dest), "size": dest.stat().st_size,
            "force_opaque": tex.force_opaque, "txi": tex.txi}


def tpc_argv(ctx: "Ctx", tex, src: Path, dest: Path) -> list:
    argv = py_cmd(repo_py(), TEX_TOOL, "tga-to-tpc", src, dest)
    if tex.force_opaque:
        argv.append("--force-opaque")
    if tex.txi:
        argv += ["--txi", ctx.cdir / "work" / f"{tex.k2.lower()}.txi"]
    return argv


# ------------------------------------------------------------------------------------------- per-character context

@dataclass
class Ctx:
    loaded: Loaded
    recipe: object
    k1: Path
    cdir: Path
    runner: Runner
    env: dict

    @property
    def stage(self) -> Path:
        return self.cdir / "stage"

    @property
    def ledger(self) -> Path:
        return self.stage / "ledger.json"


def label_for(recipe) -> str:
    s = re.sub(r"[^A-Za-z0-9_]", "_", recipe.id.capitalize())
    return ("K1P_" + s)[:32]


def char_build_argv(ctx: Ctx, rows: dict, chain_from: Path | None) -> list:
    r = ctx.recipe
    body_b = r.body_b or r.body_a
    argv = py_cmd(k1port_py(), CHAR_BUILD_TOOL, "--game-dir", ctx.loaded.cfg.k2_dir, "--tag", r.id,
                  "--label", label_for(r), "--new-row", rows["appearance"], "--new-head-row", rows["head"],
                  "--body-a", r.body_a.k2, "--body-b", body_b.k2, "--head", r.head.k2, "--soundset", r.soundset,
                  "--gender", r.gender, "--name", r.display_name, "--utc", r.utc, "--template-row", r.template_row)
    if r.dlg:
        argv += ["--conversation", r.dlg]
    if chain_from is not None:
        argv += ["--chain-from", chain_from]
    return argv + ["--out", ctx.cdir / "override", "--json"]


def prepare_base(ctx: Ctx, order: list, idx: int, rows: dict) -> Path | None:
    """The --chain-from dir for char_build: the previous character's tables, else the live tables padded to the
    planned row (a pad copy in <cdir>/base). None = no chain (the live tables already have exactly the planned count)."""
    live = live_tables(ctx.loaded.cfg.k2_dir)
    cands: list[Path | None] = []
    if idx > 0:
        pdir = ctx.loaded.out / order[idx - 1] / "override"
        if (pdir / "appearance.2da").is_file() and (pdir / "heads.2da").is_file():
            cands.append(pdir)
    cands.append(None)
    pick = None
    for cand in cands:
        d = cand or live
        ha, hh = (twoda_height(ctx.runner, d / "appearance.2da"), twoda_height(ctx.runner, d / "heads.2da"))
        if ha <= rows["appearance"] and hh <= rows["head"]:
            pick = (cand, d, ha, hh)
            break
    if pick is None:
        raise Failure(f"{ctx.recipe.id} rows", "no base table fits the planned rows", "build with --reassign")
    cand, d, ha, hh = pick
    if ha == rows["appearance"] and hh == rows["head"]:
        return cand
    base = ctx.cdir / "base"
    base.mkdir(parents=True, exist_ok=True)
    for name, target, height in (("appearance.2da", rows["appearance"], ha), ("heads.2da", rows["head"], hh)):
        if height == target:
            shutil.copyfile(d / name, base / name)
        else:
            twoda_call(ctx.runner, f"2da-pad {name}", "pad", d / name, base / name, target)
    return base


def cells_for(ctx: Ctx, rows: dict) -> list:
    r = ctx.recipe
    from k1_port_vfs_pack import TwoDaCell
    tds = []
    for table, fname, row, template in (("appearance", "appearance.2da", rows["appearance"], r.template_row),
                                        ("heads", "heads.2da", rows["head"], -1)):
        got = twoda_call(ctx.runner, f"2da-rows {table}", "rows", f"{table}={ctx.cdir / 'override' / fname}:{row}")
        for col, val in got[table][str(row)].items():
            tds.append(TwoDaCell(table, row, col, str(val), template_row=template))
    try:
        return kpack.cells_from_vfs_pack(tds)
    except kpack.PackError as e:
        raise Failure(f"{r.id} 2DA cells", str(e), "report this recipe") from None


def stage_files(ctx: Ctx) -> list:
    r = ctx.recipe
    files = []
    for m in model_refs(r):
        files += [ctx.stage / "models" / f"{m.k2}.mdl", ctx.stage / "models" / f"{m.k2}.mdx"]
    files += [ctx.cdir / "tpc" / f"{t.k2.lower()}.tpc" for t in r.textures]
    files.append(ctx.cdir / "override" / f"{r.utc}.utc")
    missing = [str(f) for f in files if not f.is_file()]
    if missing:
        raise Failure(f"{r.id} pack inputs", "missing: " + ", ".join(missing), f"see {ctx.cdir / 'logs'}")
    return files


def gate_staged(ctx: Ctx) -> dict:
    """k1_port_prune gate, non-strict, on the staged override/ and models/ dirs. Recorded, never fatal."""
    dirs = [d for d in (ctx.stage / "override", ctx.stage / "models") if d.is_dir()]
    if not ctx.ledger.is_file() or not dirs:
        return {"ran": False, "reason": "no ledger or no staged files"}
    proc = ctx.runner.run("prune gate", [str(stdlib_py()), "-I", "-B", str(PRUNE_TOOL), "gate", "--ledger", str(ctx.ledger),
                                         *[str(d) for d in dirs]], env=ctx.env, check=False)
    m = GATE_RE.search(proc.stdout or "")
    return {"ran": True, "rc": proc.returncode, "scanned": int(m.group(1)) if m else None,
            "blocked": int(m.group(2)) if m else None, "warned": int(m.group(3)) if m else None,
            "output": [ln for ln in (proc.stdout or "").splitlines() if ln.startswith(("BLOCKED", "gate:"))]}


# ------------------------------------------------------------------------------------------- build

def recipe_file_sha(recipe) -> str:
    try:
        return sha256_file(Path(recipe.source))
    except OSError:
        return ""


def tool_versions() -> dict:
    def ver(py: Path) -> str:
        try:
            p = subprocess.run([str(py), "--version"], capture_output=True, text=True, timeout=15)
            return (p.stdout or p.stderr).strip() or "unknown"
        except (OSError, subprocess.SubprocessError):
            return "unavailable"
    return {"driver": DRIVER_VERSION, "pack_format": kpack.VERSION, "host_python": sys.version.split()[0],
            "k1port_python": ver(k1port_py()), "repo_python": ver(repo_py()),
            "scripts": {t.name: sha256_file(t)[:16] for t in STEP_TOOLS + (DRIVER,) if t.is_file()}}


def print_plan(ctx: Ctx, idx: int, order: list, rows: dict, resolved: dict) -> None:
    r = ctx.recipe
    cmds: list[tuple[str, list]] = []
    textures_owned = textures_by_model(r, resolved)
    sm = [f"{r.body_b.k1}={r.body_b.k2}"] if r.body_b else []
    if r.converter == "pykotor":
        for m in model_refs(r):
            own = textures_owned.get(m.k1) or []
            cmds.append((f"ingest {m.k1}", ingest_argv(ctx, m.k1, m.k2, own, no_model=False,
                                                       supermodel_map=[] if m is r.body_b else sm)))
        if textures_owned.get(""):
            cmds.append(("ingest textures", ingest_argv(ctx, r.body_a.k1, None, textures_owned[""], no_model=True, supermodel_map=[])))
    else:
        cmds += mdlops_steps(ctx, r)
        cmds.append(("ingest textures", ingest_argv(ctx, r.body_a.k1, None, list(r.textures), no_model=True, supermodel_map=[])))
    chain = ctx.loaded.out / order[idx - 1] / "override" if idx else None
    cmds.append(("char_build", char_build_argv(ctx, rows, chain)))
    for t in r.textures:
        cmds.append((f"tex {t.k2}", tpc_argv(ctx, t, ctx.stage / "override" / f"{t.k2.lower()}.<tga>", ctx.cdir / "tpc" / f"{t.k2.lower()}.tpc")))
    say(f"[{r.id}] dry run: appearance row {rows['appearance']}, heads row {rows['head']}, converter {r.converter}, "
        f"texture_source {ctx.loaded.cfg.texture_source}")
    say(f"[{r.id}]   resolved {len(resolved['rows'])} K1 items, {len(resolved['flagged'])} flagged")
    for f in resolved["flagged"]:
        warn(f"{r.id}: flagged source {f['item']} ({f['reason']}); allowed because the out dir is private")
    if ctx.loaded.cfg.texture_source == "cr":
        warn(f"{r.id}: texture_source = cr: Characters Revisited textures are flagged pending_author_approval (private out only)")
    for label, argv in cmds:
        say(f"[{r.id}]   {label}: {shlex.join(str(a) for a in argv)}")
    say(f"[{r.id}]   pack -> {ctx.loaded.out / (r.id + PACK_SUFFIX)}")


def build_character(loaded: Loaded, recipe, idx: int, order: list, rows: dict, k1: Path, dry_run: bool) -> dict:
    cdir = loaded.out / recipe.id
    runner = Runner(None if dry_run else cdir / "logs")
    ctx = Ctx(loaded=loaded, recipe=recipe, k1=k1, cdir=cdir, runner=runner,
              env={"K1PORT_SOURCES": str(cdir / "work" / "sources.json"), "K1PORT_LEDGER": str(cdir / "stage" / "ledger.json")})
    cid = recipe.id
    if loaded.cfg.texture_source == "cr" and not dry_run and not cr_dir().is_dir():
        raise Failure("[build] texture_source = cr", f"Characters Revisited dir not found: {cr_dir()}",
                      "extract CR (k1_port_cr_extract.py) or set K1PORT_CR_DEST; or use texture_source = \"vanilla\"")
    if recipe.head is None:
        raise Failure(f"recipe {cid}", "no [models] head: the importer builds appearance + heads rows and needs a head model",
                      "add models.head to the recipe")
    say(f"[{cid}] 1/6 resolve K1 sources")
    resolver = Runner(None)  # logged into the manifest only: the work dir is wiped below
    resolved = resolve_character(resolver, loaded, recipe, k1)
    if dry_run:
        print_plan(ctx, idx, order, rows, resolved)
        return {}
    flagged = list(resolved["flagged"])

    # wipe the character's work dir, then start clean
    if cdir.exists():
        if cdir.is_symlink() or not cdir.is_dir() or cdir.parent != loaded.out:
            raise Refusal(str(cdir), "refusing to clean a work dir that is a symlink or outside the out dir", "remove it by hand")
        shutil.rmtree(cdir)
    (cdir / "work").mkdir(parents=True)
    runner.records.extend(resolver.records)
    write_json(cdir / "licence_report.json", {"k1_dir": str(k1), "recipe": cid, "texture_source": loaded.cfg.texture_source,
                                              "items": resolved["rows"], "missing": resolved["missing"], "flagged": flagged})
    write_registry(cdir / "work" / "sources.json", k1, loaded.cfg.texture_source)
    ctx.stage.mkdir(parents=True, exist_ok=True)

    say(f"[{cid}] 2/6 licence gate: out dir is private; {len(flagged)} flagged item(s) so far")
    ingests = []
    owned = textures_by_model(recipe, resolved)
    sm = [f"{recipe.body_b.k1}={recipe.body_b.k2}"] if recipe.body_b else []
    say(f"[{cid}] 3/6 models ({recipe.converter})")
    if recipe.converter == "pykotor":
        for m in model_refs(recipe):
            own = owned.get(m.k1) or []
            if not own:
                raise Failure(f"recipe {cid}", f"the K1 mesh {m.k1!r} references none of the recipe's [[textures]]",
                              f"add a [[textures]] entry for each texture {m.k1} uses (k1_port_ingest needs the mesh's texture list)")
            ingests.append(run_ingest(ctx, f"ingest {m.k1}", ingest_argv(ctx, m.k1, m.k2, own, no_model=False,
                                                                        supermodel_map=[] if m is recipe.body_b else sm)))
        if owned.get(""):
            ingests.append(run_ingest(ctx, "ingest textures", ingest_argv(ctx, recipe.body_a.k1, None, owned[""], no_model=True, supermodel_map=[])))
    else:
        for label, argv in mdlops_steps(ctx, recipe):
            runner.run(label, argv, env=ctx.env, fix="MDLOps needs Proton and the mdlops tools (doctor)")
        mdlops_finish(ctx, recipe)
        ingests.append(run_ingest(ctx, "ingest textures", ingest_argv(ctx, recipe.body_a.k1, None, list(recipe.textures), no_model=True, supermodel_map=[])))
    for data in ingests:
        flagged += ingest_flagged(data)
    seen, uniq = set(), []
    for f in flagged:
        key = (f["item"], f["origin"])
        if key not in seen:
            seen.add(key)
            uniq.append(f)
    flagged = uniq
    for f in flagged:
        warn(f"{cid}: flagged source {f['item']} [{f['origin']}] ({f['reason']}); allowed because the out dir is private")
    gate = gate_staged(ctx)

    say(f"[{cid}] 4/6 textures -> DXT1 TPC")
    tex_info = [make_tpc(ctx, t) for t in recipe.textures]

    say(f"[{cid}] 5/6 rows {rows['appearance']}/{rows['head']} + UTC")
    chain = prepare_base(ctx, order, idx, rows)
    proc = runner.run(f"char_build {cid}", char_build_argv(ctx, rows, chain), env=ctx.env,
                      fix="the live K2 appearance/heads tables changed? re-run; if the counts moved use --reassign")
    try:
        cb = json.loads(proc.stdout)
    except json.JSONDecodeError:
        cb = {}
    cells = cells_for(ctx, rows)

    say(f"[{cid}] 6/6 pack")
    files = stage_files(ctx)
    try:
        assets = kpack.assets_from_stage(files)
        info = kpack.CharacterInfo(character_id=recipe.id, display_name=recipe.display_name, default_tag=recipe.tag,
                                   default_utc=recipe.utc, default_dlg=recipe.dlg, appearance_id=rows["appearance"],
                                   head_id=rows["head"], soundset_id=recipe.soundset)
        pack_path = loaded.out / f"{recipe.id}{PACK_SUFFIX}"
        summary = kpack.write_pack(pack_path, info, assets, cells)
        data = pack_path.read_bytes()
        got_info, got_assets, got_cells = kpack.read_pack(data)
    except (kpack.PackError, kpack.PackFormatError) as e:
        raise Failure(f"{cid} pack", str(e), "see the message; the converted files are in " + str(cdir)) from None
    if (got_info != info or len(got_assets) != len(assets) or len(got_cells) != len(cells)
            or {(a.resref, a.res_type) for a in got_assets} != {(a.resref, a.res_type) for a in assets}):
        raise Failure(f"{cid} pack", "read_pack self-check differs from what was written", "report this; do not install the pack")
    if sha256_bytes(data) != summary["sha256"]:
        raise Failure(f"{cid} pack", "pack sha256 differs from the write summary", "rebuild")

    manifest = {
        "schema": 1, "id": cid, "built_at": now_iso(),
        "recipe": {"path": recipe.source, "sha256": recipe_file_sha(recipe), "converter": recipe.converter},
        "k1_dir": str(k1), "k2_dir": str(loaded.cfg.k2_dir), "texture_source": loaded.cfg.texture_source,
        "sources": resolved["rows"], "missing_optional": resolved["missing"],
        "ingest": [summarize_ingest(d) for d in ingests],
        "textures": tex_info,
        "flagged": flagged,
        "flag_policy": "private out dir: flagged sources allowed, never redistribute" if flagged else "none flagged",
        "prune_gate": gate,
        "commands": runner.records,
        "assigned": rows,
        "char_build": {"sha256": cb.get("sha256"), "chained_from": cb.get("chained_from")},
        "pack": {"path": str(pack_path), "sha256": summary["sha256"], "size": summary["size"], "crc32": summary["crc32"],
                 "asset_count": summary["asset_count"], "cell_count": summary["cell_count"]},
        "tools": tool_versions(),
    }
    write_json(loaded.out / f"{recipe.id}.import_manifest.json", manifest)
    say(f"[{cid}] built {pack_path} ({summary['size']} bytes, {summary['asset_count']} assets, {summary['cell_count']} cells, "
        f"rows {rows['appearance']}/{rows['head']}), sha256 {summary['sha256'][:16]}")
    return manifest


def cmd_build(args) -> int:
    loaded = load_all(args.config)
    chosen = select(loaded, args.only)
    if args.reassign and args.only:
        raise Failure("--reassign", "cannot be combined with --only: the other characters' packs would keep the old row ids",
                      "drop --only (all configured characters are rebuilt)")
    require_private_out(loaded.out, [loaded.cfg.k1_dir, loaded.cfg.k2_dir])
    try:
        k1 = kresolve.find_k1_dir(str(loaded.cfg.k1_dir), None, env={})
    except kresolve.ResolveError as e:
        raise Failure("[k1] dir", str(e), "set [k1] dir to the folder that holds chitin.key") from None
    require_k2(loaded.cfg.k2_dir)
    for r in chosen:
        for w in r.warnings:
            warn(w)
    plan_runner = Runner(None)
    rows, live, stored = plan_assignments(loaded, plan_runner, args.reassign)
    ids = installed_juhani_char(loaded.cfg.k2_dir)
    if ids and juhani_rows_clash(rows):
        warn(JUHANI_WARN.format(ids=", ".join(ids)))
    order = list(loaded.cfg.characters)
    say(f"live K2 tables: appearance.2da {live['appearance']} rows, heads.2da {live['head']} rows")
    if args.dry_run:
        say("dry run: nothing is written")
    elif not loaded.out.exists():
        loaded.out.mkdir(parents=True)
    if args.reassign:  # every configured character is rebuilt: the old ids are dropped with the first save
        stored = {k: v for k, v in stored.items() if k not in order}
    for r in chosen:
        idx = order.index(r.id)
        build_character(loaded, r, idx, order, rows[r.id], k1, args.dry_run)
        if not args.dry_run:
            stored = save_assignment(loaded, stored, r.id, rows[r.id])
    if not args.dry_run:
        warn_cell_budget(loaded)
        say(f"done: {len(chosen)} pack(s) in {loaded.out}. Next: install (close the game first).")
    return EXIT_OK


CELL_POOL = 1024  # asset-vfs >= 0.1.3-dev kMax2DaCells: one pool for ALL registrants; cells past it are dropped (WARN)


def warn_cell_budget(loaded: Loaded) -> int:
    """Sum of 2DA cells over every built pack of the config; WARN when it exceeds the shared asset-vfs pool."""
    total = 0
    for cid in loaded.cfg.characters:
        man = read_json(loaded.out / f"{cid}.import_manifest.json", "manifest") or {}
        total += int(man.get("pack", {}).get("cell_count") or 0)
    if total > CELL_POOL:
        warn(f"2DA cell budget: the built packs need {total} cells, the shared asset-vfs pool holds {CELL_POOL} for ALL mods "
             f"(about 102 per character). Packs past the limit get a partial row (REGISTER failed in character-packs_log.txt). "
             f"Build fewer characters, or uninstall juhani-char.")
    else:
        say(f"2DA cell budget: {total}/{CELL_POOL} cells (shared with every other asset-vfs client, e.g. juhani-char)")
    return total


# ------------------------------------------------------------------------------------------- install / remove / list

def installed_path(loaded: Loaded) -> Path:
    return loaded.out / "installed.json"


def read_installed(loaded: Loaded) -> dict:
    data = read_json(installed_path(loaded), "installed.json") or {}
    if not isinstance(data, dict):
        raise Failure(str(installed_path(loaded)), "not a JSON object", "delete it and re-run install")
    return data


def verify_pack_bytes(data: bytes, label: str):
    try:
        return kpack.read_pack(data)
    except kpack.PackFormatError as e:
        raise Failure(label, f"not a valid .k2pack: {e}", "rebuild the character") from None


def cmd_install(args) -> int:
    loaded = load_all(args.config)
    chosen = select(loaded, args.only)
    require_private_out(loaded.out, [loaded.cfg.k1_dir, loaded.cfg.k2_dir])
    require_k2(loaded.cfg.k2_dir)
    ids = installed_juhani_char(loaded.cfg.k2_dir)
    if ids and juhani_rows_clash(read_json(loaded.out / "assignments.json", "assignments")):
        warn(JUHANI_WARN.format(ids=", ".join(ids)))
    dest_dir = loaded.k2_imports
    if dest_dir.is_symlink() or (dest_dir.exists() and not dest_dir.is_dir()):
        raise Refusal(str(dest_dir), "is a symlink or not a directory", "remove it and let install create a real dir")
    plan = []
    for r in chosen:
        pack = loaded.out / f"{r.id}{PACK_SUFFIX}"
        if not pack.is_file():
            raise Failure(str(pack), "pack not built", f"run: build --only {r.id}")
        data = pack.read_bytes()
        verify_pack_bytes(data, str(pack))
        man = read_json(loaded.out / f"{r.id}.import_manifest.json", "manifest")
        if man and man.get("pack", {}).get("sha256") != sha256_bytes(data):
            raise Failure(str(pack), "pack changed after the build (sha256 differs from its manifest)", f"rebuild: build --only {r.id}")
        plan.append((r.id, pack, data))
    if args.dry_run:
        for cid, pack, data in plan:
            say(f"dry run: would copy {pack} -> {dest_dir / (cid + PACK_SUFFIX)} ({len(data)} bytes)")
        return EXIT_OK
    dest_dir.mkdir(exist_ok=True)
    rec = read_installed(loaded)
    for cid, pack, data in plan:
        dest = dest_dir / f"{cid}{PACK_SUFFIX}"
        tmp = dest_dir / f".{cid}{PACK_SUFFIX}.tmp{os.getpid()}"
        try:
            with open(tmp, "wb") as fh:
                fh.write(data)
                fh.flush()
                os.fsync(fh.fileno())
            os.replace(tmp, dest)
        finally:
            with _suppress(OSError):
                tmp.unlink()
        back = dest.read_bytes()
        if sha256_bytes(back) != sha256_bytes(data):
            raise Failure(str(dest), "copy does not match the built pack (sha256)", "check the disk and re-run install")
        verify_pack_bytes(back, str(dest))
        rec[cid] = {"path": str(dest), "sha256": sha256_bytes(back), "size": len(back), "installed_at": now_iso()}
        write_json(installed_path(loaded), rec)
        say(f"installed {dest} ({len(back)} bytes)")
    say("Close the game before launching with new packs; the character-packs patch reads k1_imports/ at game start.")
    return EXIT_OK


def cmd_remove(args) -> int:
    if not ID_RE.fullmatch(args.id or ""):
        raise Failure("remove", f"bad id {args.id!r}", "ids match [a-z0-9_]{1,31}")
    loaded = load_all(args.config)
    require_private_out(loaded.out, [loaded.cfg.k1_dir, loaded.cfg.k2_dir])
    rec = read_installed(loaded)
    entry = rec.get(args.id)
    expect = loaded.k2_imports / f"{args.id}{PACK_SUFFIX}"
    if entry is None:
        raise Failure(str(installed_path(loaded)), f"{args.id!r} is not listed as installed; nothing deleted",
                      "only packs installed by this tool are removed")
    if Path(entry.get("path", "")) != expect:
        raise Refusal(str(installed_path(loaded)), f"recorded path {entry.get('path')!r} is not {expect}",
                      "refusing to delete a path that is not <K2>/k1_imports/<id>.k2pack")
    if expect.is_symlink():
        raise Refusal(str(expect), "is a symlink", "remove it by hand")
    if expect.is_file():
        if sha256_file(expect) != entry.get("sha256"):
            warn(f"{expect} differs from the installed record (changed since install); deleting it anyway")
        expect.unlink()
        say(f"deleted {expect}")
    else:
        warn(f"{expect} was already gone")
    rec.pop(args.id)
    write_json(installed_path(loaded), rec)
    stored = read_json(loaded.out / "assignments.json", "assignments") or {}
    if isinstance(stored, dict) and args.id in stored:
        old = stored.pop(args.id)
        write_json(loaded.out / "assignments.json", stored)
        say(f"dropped the assignment (appearance {old.get('appearance')}, heads {old.get('head')}); those row ids stay unused "
            f"until a rebuild of {args.id} (rows are positional in config order)")
    say("Close the game before relaunching.")
    return EXIT_OK


def cmd_list(args) -> int:
    loaded = load_all(args.config)
    stored = read_json(loaded.out / "assignments.json", "assignments") or {}
    inst = read_json(installed_path(loaded), "installed.json") or {}
    rows = []
    for r in loaded.recipes:
        pack = loaded.out / f"{r.id}{PACK_SUFFIX}"
        man = read_json(loaded.out / f"{r.id}.import_manifest.json", "manifest") or {}
        built = pack.is_file()
        sha = sha256_file(pack) if built else None
        stale = bool(built and man and man.get("recipe", {}).get("sha256") != recipe_file_sha(r))
        i = inst.get(r.id)
        dest = loaded.k2_imports / f"{r.id}{PACK_SUFFIX}"
        installed = bool(i and dest.is_file())
        rows.append({"id": r.id, "display_name": r.display_name, "converter": r.converter, "built": built,
                     "pack_sha256": sha, "stale": stale, "installed": installed,
                     "installed_matches_build": bool(installed and sha and sha256_file(dest) == sha),
                     "assigned": stored.get(r.id), "flagged": len(man.get("flagged", [])) if man else None})
    if args.json:
        print(json.dumps({"out": str(loaded.out), "k2_dir": str(loaded.cfg.k2_dir), "characters": rows}, indent=2, sort_keys=True))
        return EXIT_OK
    say(f"out: {loaded.out}")
    for x in rows:
        a = x["assigned"]
        rowtxt = f"rows {a['appearance']}/{a['head']}" if a else "rows unassigned"
        state = ("built" + (" (stale: recipe changed)" if x["stale"] else "")) if x["built"] else "not built"
        inst_txt = ("installed" + ("" if x["installed_matches_build"] else " (differs from build)")) if x["installed"] else "not installed"
        sha = (x["pack_sha256"] or "-")[:16]
        say(f"{x['id']:<14} {state:<24} {inst_txt:<30} {rowtxt:<18} sha {sha}")
    return EXIT_OK


# ------------------------------------------------------------------------------------------- doctor

def cmd_doctor(args) -> int:
    results: list[tuple[str, str, str, str]] = []  # (status, name, detail, fix)

    def ok(n, d):
        results.append(("OK", n, d, ""))

    def bad(n, d, fix):
        results.append(("FAIL", n, d, fix))

    def maybe(n, d, fix):
        results.append(("WARN", n, d, fix))

    if sys.version_info >= (3, 11):
        ok("python", f"{sys.version.split()[0]} (tomllib available)")
    else:
        bad("python", f"{sys.version.split()[0]} has no tomllib", "run with python >= 3.11")
    path = Path(args.config or DEFAULT_CONFIG).expanduser()
    loaded = None
    try:
        loaded = load_all(args.config)
        ok("config", f"{path}: characters {list(loaded.cfg.characters)}, texture_source {loaded.cfg.texture_source}")
    except Failure as e:
        bad("config", f"{e.location}: {e.cause}", e.fix)
    if loaded is not None:
        cfg = loaded.cfg
        try:
            ok("K1 dir", str(kresolve.find_k1_dir(str(cfg.k1_dir), None, env={})))
        except kresolve.ResolveError as e:
            bad("K1 dir", str(e), "set [k1] dir in the config to the folder that holds chitin.key")
        if k2_has_game(cfg.k2_dir):
            ok("K2 dir", str(cfg.k2_dir))
        else:
            bad("K2 dir", f"{cfg.k2_dir}: no swkotor2.exe or chitin.key", "set [k2] dir to the KOTOR II install folder")
        for fname in ("appearance.2da", "heads.2da"):
            f = live_tables(cfg.k2_dir) / fname
            if f.is_file():
                ok(f"K2 {fname}", str(f))
            elif k2_has_game(cfg.k2_dir):
                bad(f"K2 {fname}", f"{f} not found (k1_port_char_build reads the live table from override/)",
                    f"put the current {fname} into <K2>/override/ (extract it from the K2 BIFs with a tool such as KotOR Tool)")
        reasons = out_safety(loaded.out, [cfg.k1_dir, cfg.k2_dir])
        if reasons:
            bad("out dir", f"{loaded.out}: " + "; ".join(reasons),
                "set [build] out to a private dir outside any git work tree, game install and symlink, e.g. ~/.cache/k1_port/packs")
        else:
            ok("out dir", f"{loaded.out} is private")
        if cfg.texture_source == "cr":
            if cr_dir().is_dir():
                ok("Characters Revisited dir", str(cr_dir()))
            else:
                bad("Characters Revisited dir", f"{cr_dir()} not found",
                    "extract CR with k1_port_cr_extract.py, set K1PORT_CR_DEST, or use texture_source = \"vanilla\"")
        md = [r.id for r in loaded.recipes if r.converter == "mdlops"]
        if md:
            import k1_port_mdlops as km
            for what, p, fix in (("Proton", km.PROTON, "install Steam's 'Proton - Experimental'"),
                                 ("mdlops.exe", km.TOOL_SRC, "restore the MDLOps tools dir (k1_port_mdlops.py TOOL_SRC)"),
                                 ("MDLOps Proton prefix", km.PREFIX, "restore the mdlops_t7/mdl_run1/prefix dir")):
                name = f"{what} (converter mdlops: {', '.join(md)})"
                if Path(p).exists():
                    ok(name, str(p))
                else:
                    bad(name, f"{p} not found", fix)
        ids = installed_juhani_char(cfg.k2_dir) if cfg.k2_dir.is_dir() else []
        if ids:
            maybe("hand-path characters", f"{', '.join(ids)} is installed; its virtual rows 818-820 / 249-251 collide with the assigned rows",
                  "uninstall it (KPM) or set [build] first_appearance_row = 821 and first_head_row = 252")
        else:
            ok("hand-path characters", "juhani-char is not installed")
    kp = k1port_py()
    if not kp.is_file():
        bad("k1port python", f"{kp} not found", "run patch_manager_mods/k1_port/setup_venv.sh (or set K1PORT_PY)")
    else:
        p = subprocess.run([str(kp), "-I", "-c", "import pykotor"], capture_output=True, text=True)
        if p.returncode == 0:
            ok("k1port python", f"{kp}: import pykotor works")
        else:
            bad("k1port python", f"{kp}: import pykotor failed: {tail_lines(p.stderr)}",
                "re-run patch_manager_mods/k1_port/setup_venv.sh")
    rp = repo_py()
    if not rp.is_file():
        bad("repo .venv python", f"{rp} not found", "create the repo .venv (numpy) or set REPO_VENV_PY")
    else:
        p = subprocess.run([str(rp), "-I", "-c", "import numpy"], capture_output=True, text=True)
        if p.returncode == 0:
            ok("repo .venv python", f"{rp}: numpy works")
        else:
            bad("repo .venv python", f"{rp}: import numpy failed: {tail_lines(p.stderr)}", "pip install numpy into the repo .venv")
    for st, name, detail, fix in results:
        say(f"{st:<4} {name}: {detail}")
        if fix and st != "OK":
            say(f"     fix: {fix}")
    failed = sum(1 for r in results if r[0] == "FAIL")
    say(f"doctor: {failed} failure(s), {sum(1 for r in results if r[0] == 'WARN')} warning(s)")
    return EXIT_FAIL if failed else EXIT_OK


# ------------------------------------------------------------------------------------------- CLI

def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description="K1 -> K2 character importer (doc 38 s26): doctor | build | install | remove | list",
                                 epilog="Exit: 0 ok, 1 failure, 2 safety refusal.")
    sub = ap.add_subparsers(dest="cmd", required=True)
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--config", default=None, metavar="C", help=f"config TOML (default {DEFAULT_CONFIG})")
    p = sub.add_parser("doctor", parents=[common], help="check dirs, interpreters, tools and the out dir")
    p.set_defaults(fn=cmd_doctor)
    p = sub.add_parser("build", parents=[common], help="build one .k2pack per configured character")
    p.add_argument("--only", metavar="ID")
    p.add_argument("--dry-run", action="store_true", help="resolve + gate + print the planned commands; write nothing")
    p.add_argument("--reassign", action="store_true", help="accept new appearance/heads row ids (rebuilds every character)")
    p.set_defaults(fn=cmd_build)
    p = sub.add_parser("install", parents=[common], help="copy built packs to <K2>/k1_imports/")
    p.add_argument("--only", metavar="ID")
    p.add_argument("--dry-run", action="store_true")
    p.set_defaults(fn=cmd_install)
    p = sub.add_parser("remove", parents=[common], help="delete one installed pack from <K2>/k1_imports/")
    p.add_argument("id")
    p.set_defaults(fn=cmd_remove)
    p = sub.add_parser("list", parents=[common], help="characters, build status, installed state, rows")
    p.add_argument("--json", action="store_true")
    p.set_defaults(fn=cmd_list)
    return ap


def main(argv: list | None = None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    if argv and argv[0] == "_twoda":  # hidden helper, runs under the k1port venv python
        return _twoda_main(argv[1:])
    args = build_parser().parse_args(argv)
    try:
        return args.fn(args)
    except Failure as e:
        print(f"error: {e}", file=sys.stderr)
        return e.code
    except (krecipe.RecipeError, kresolve.ResolveError) as e:
        print(f"error: {e}", file=sys.stderr)
        return EXIT_FAIL


if __name__ == "__main__":
    sys.exit(main())
