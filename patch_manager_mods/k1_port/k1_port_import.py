#!/usr/bin/env python3
"""Install-time K1 -> K2 character importer (doc 38 s26): one `.k2pack` per character, built from YOUR K1 install.

  python3 k1_port_import.py doctor  [--config C]
  python3 k1_port_import.py build   [--config C] [--only ID] [--dry-run] [--reassign]
  python3 k1_port_import.py install [--config C] [--only ID] [--dry-run]
  python3 k1_port_import.py remove  ID [--config C]
  python3 k1_port_import.py list    [--config C] [--json]
  python3 k1_port_import.py npcs    [--config C] [--grep PATTERN] [--limit N] [--json]   list K1 creature templates
  python3 k1_port_import.py scan    UTC_RESREF [--id ID] [--write [DIR]] [--config C]     derive a recipe from a K1 UTC

The config (see k1_imports.example.toml; copy it to a PRIVATE place) names the K1 dir, the K2 dir, the output dir
and the characters to build. A character is a committed recipe (recipes/<id>.toml, always wins) or ANY K1 creature
template: `utc:<resref>` (or a plain id with no recipe file that is a K1 UTC resref). The recipe of such an auto
character is derived from your K1 install at build time (k1_port_scan.py), written to <out>/recipes_auto/<id>.toml
and recorded in the manifest. npcs lists the K1 creature templates; scan prints (or writes) the derived recipe.
The driver is stdlib only; it chains the existing step scripts as subprocesses, each in the interpreter it needs:
  k1port venv python (env K1PORT_PY)   k1_port_resolve, k1_port_ingest, k1_port_char_build, k1_port_mdlops extract/pass2
  repo .venv python  (env REPO_VENV_PY) k1_port_tex_tpc (numpy)
  this python (env K1PORT_SYS_PY)      k1_port_prune gate/adopt, k1_port_validate, k1_port_mdlops rename/pass1

build, per character in config order (every step logs to <out>/<id>/logs/):
  1 resolve  every K1 model (mdl+mdx) and texture (tpc or tga) the recipe names -> <out>/<id>/licence_report.json
  2 gate     the out dir must be PRIVATE (not in a git work tree, not under steamapps/common or compatdata, no
             symlink on the path, not inside the K1/K2 dir), else exit 2. Flagged sources (K1 Override files, an HD
             overlay, Characters Revisited / anything pending_author_approval) are allowed in a private out only:
             WARN + recorded in the manifest. k1_port_prune gate runs (non-strict) on the staged files.
  3 models   (+ audio, when the recipe has [audio]: the K1 SSF is read from the K1 install and checked against both
             dialog.tlk files (every strref must carry the same voiceover in K2), the K1 voice wavs are copied from
             <K1>/streamsounds/ (read only) to <out>/<id>/audio/streamsounds/, a missing wav is a WARN, none = failure;
             every other <K1>/streamsounds/<k1_ssf>_*.wav is served too, recorded as `extra`, capped by the pack limits)
             converter pykotor -> k1_port_ingest (per model, with its textures); converter mdlops -> k1_port_mdlops
             extract/pass1/rename/pass2, then validate + copy into the stage (the script's own `finish` writes into
             the shared hand-path stage, so the driver does that step itself)
             (+ dialog, when the recipe has [dialog]: the character's K1 conversation is read from the K1 install and converted
             by k1_port_dialog.py: every K1 script reference is stripped, only starters/links without a condition survive,
             the K1 TLK text is embedded literally; the voice-over wavs of the first `max_voiced` lines (breadth-first) are
             copied from <K1>/streamwaves/ (read only) to <out>/<id>/dialog/streamvoice/ as STREAM assets, their lips from
             <K1>/lips/*.mod to dialog/lips/ (only lips that pass kvoice_lip.check_lip); the pack serves <k2_dlg>.dlg, and the
             UTC conversation / default_dlg become it)
  4 textures <k2>.tpc per recipe texture: DXT1 (k1_port_tex_tpc tga-to-tpc for TGA input, --force-opaque, --txi
             with the recipe text; a staged DXT1 TPC is used as is, its TXI trailer replaced by the recipe txi when one is set)
  5 rows     (+ a new soundset.2da row for a recipe with [audio], assigned at the live row count in config order among
             the audio recipes and chained like appearance/heads; the UTC gets it, the pack serves <k2_ssf>.ssf as a
             RESMAN asset and the wavs as STREAM assets (flags 1, res_type 4), plus the row's 2DA cells)
             appearance/heads row ids come from the live K2 tables (<K2>/override/appearance.2da, heads.2da) in
             config order, chained with --chain-from (a pad step covers [build] first_appearance_row/first_head_row
             above the live count; a head-less single-model recipe, i.e. body_a only, gets no heads row and no heads cells);
             ids are persisted in <out>/assignments.json and reused. If the live counts moved
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
import base64
import datetime
import hashlib
import json
import os
import re
import shlex
import shutil
import struct
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))  # python -I drops the script dir from sys.path
sys.path.insert(0, str(HERE.parent / "kvoice"))

import k1_port_dialog as kdialog  # noqa: E402  stdlib only; pykotor is used in the _dialog helper subprocess
import k1_port_pack as kpack  # noqa: E402  stdlib only
import k1_port_recipe as krecipe  # noqa: E402  stdlib only (tomllib at load time)
import k1_port_resolve as kresolve  # noqa: E402  stdlib only; pykotor is used in the resolve subprocess
import k1_port_scan as kscan  # noqa: E402  stdlib only; pykotor is used in the _scan helper subprocess
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
SSF_SIZE, SSF_MAGIC, SSF_SLOTS, SSF_EMPTY = 172, b"SSF V1.1", 28, 0xFFFFFFFF
STREAM_DIR = "streamsounds"
STREAMWAVES_DIR = "streamwaves"  # K1 dialog voice-over, nested <module>/<speaker>/<NAME>.wav
DIALOG_DIR, STREAMVOICE_DIR, LIPS_DIR = "dialog", "streamvoice", "lips"
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
    return any(r.get(k) is not None and int(r.get(k, -1)) in JUHANI_ROWS[k] for r in assigned.values() for k in JUHANI_ROWS)


# ------------------------------------------------------------------------------------------- config

@dataclass
class AutoRecipe:
    """A recipe derived from a K1 creature template at load time (never committed). `path` is where build writes `text`."""
    id: str
    utc: str
    text: str
    path: Path


@dataclass
class Loaded:
    cfg: krecipe.Config
    recipes: list
    out: Path
    auto: dict = field(default_factory=dict)  # {id: AutoRecipe} for the characters without a committed recipe file

    @property
    def k2_imports(self) -> Path:
        return self.cfg.k2_dir / IMPORT_DIR

    @property
    def auto_dir(self) -> Path:
        return self.out / kscan.AUTO_DIR


def load_all(config_arg: str | None, *, cached: bool = False) -> Loaded:
    """Config + recipes in config order. A character is its committed recipes/<id>.toml, else (auto) a recipe derived
    from the K1 creature template `utc:<resref>` / of the same name as the id. cached=True (list, install, remove)
    reuses an already written <out>/recipes_auto/<id>.toml instead of reading the K1 install again."""
    path = Path(config_arg or DEFAULT_CONFIG).expanduser()
    if not path.is_file():
        raise Failure(str(path), "config file not found",
                      "copy patch_manager_mods/k1_port/k1_imports.example.toml to a private place, edit it, "
                      "and pass --config")
    try:
        cfg = krecipe.load_config(path)
        out = Path(os.path.abspath(cfg.out))
        recipes, auto = [], {}
        plan = []  # (id, committed recipe file | cached auto file | None, resref, apath)
        for cid in cfg.characters:
            f = cfg.recipes_dir / f"{cid}.toml"
            apath = out / kscan.AUTO_DIR / f"{cid}.toml"
            if f.is_file():  # a committed recipe is an override: it always wins over auto-derivation
                plan.append((cid, f, None, apath))
            elif cached and apath.is_file():
                plan.append((cid, apath, cfg.utc_entries.get(cid, cid), apath))
            else:
                plan.append((cid, None, cfg.utc_entries.get(cid, cid), apath))
        todo = [p[2] for p in plan if p[1] is None]
        facts = {}
        if todo:
            try:
                k1 = kresolve.find_k1_dir(str(cfg.k1_dir), None, env={})
            except kresolve.ResolveError as e:
                raise Failure("[k1] dir", str(e), "set [k1] dir to the folder that holds chitin.key") from None
            facts = scan_facts_many(Runner(None), k1, todo)
        for cid, src, resref, apath in plan:
            if src is not None and resref is None:
                recipes.append(krecipe.load_recipe(src))
            elif src is not None:  # cached auto recipe
                recipes.append(krecipe.load_recipe(src))
                auto[cid] = AutoRecipe(cid, resref, src.read_text(encoding="utf-8"), apath)
            else:
                text = derive_auto_text(facts.get(resref), resref, cid, hint_file=cfg.recipes_dir / f"{cid}.toml")
                recipes.append(krecipe.parse_recipe(_toml_loads(text), apath))
                auto[cid] = AutoRecipe(cid, resref, text, apath)
        krecipe.check_collisions(recipes)
    except krecipe.RecipeError as e:
        raise Failure(str(path), str(e), "fix the config or recipe named in the message") from None
    except kscan.ScanError as e:
        raise Failure(f"{path}: {e.location}", e.cause, e.fix) from None
    return Loaded(cfg=cfg, recipes=recipes, out=Path(os.path.abspath(cfg.out)), auto=auto)


def _toml_loads(text: str) -> dict:
    return krecipe._toml_module().loads(text)


def select(loaded: Loaded, only: str | None) -> list:
    if only is None:
        return list(loaded.recipes)
    chosen = [r for r in loaded.recipes if r.id == only]
    if not chosen:
        raise Failure("--only", f"{only!r} is not in [build] characters {list(loaded.cfg.characters)}",
                      "use one of the configured ids")
    return chosen


# ------------------------------------------------------------------------------------------- K1 creature templates (hidden _scan)

def scan_call(runner: Runner, label: str, *args) -> dict:
    """Run the hidden `_scan` helper under the k1port venv; parse the JSON object on its last stdout line."""
    proc = runner.run(label, [str(k1port_py()), "-I", "-B", str(DRIVER), "_scan", *[str(a) for a in args]],
                      fix="run the importer's doctor: the k1port venv (pykotor) must work and [k1] dir must be a full K1 install")
    lines = [ln for ln in proc.stdout.splitlines() if ln.strip()]
    try:
        return json.loads(lines[-1])
    except (IndexError, json.JSONDecodeError) as e:
        raise Failure(label, f"helper printed no JSON: {e}", "re-run doctor") from None


def scan_facts_many(runner: Runner, k1: Path, resrefs: list) -> dict:
    """{resref: facts or None} from ONE pykotor start (it costs seconds)."""
    got = scan_call(runner, "scan " + " ".join(resrefs), "derive", k1, *resrefs)
    return got["facts"]


def scan_facts(runner: Runner, k1: Path, resref: str) -> dict | None:
    return scan_facts_many(runner, k1, [resref])[resref]


def derive_auto_text(facts: dict | None, resref: str, rid: str | None, hint_file: Path | None = None) -> str:
    """The derived recipe TOML for K1 creature template `resref` from its facts. ScanError when it cannot be built; a
    template that does not exist names both ways a recipe could have been provided."""
    if facts is None:
        raise kscan.ScanError(f"K1 creature template {resref!r}",
                              "not found in the K1 install (templates.bif, Override or the module archives)"
                              + (f", and there is no recipe file {hint_file}" if hint_file else ""),
                              "use a recipe id from recipes/ or a K1 UTC resref: npcs --grep NAME lists them")
    return kscan.derive_text(facts, rid)[1]


def write_auto(loaded: Loaded) -> list[Path]:
    """Write each derived recipe to <out>/recipes_auto/<id>.toml (only when the bytes changed). Call after the out dir
    was checked private. Returns the files written."""
    written = []
    if not loaded.auto:
        return written
    d = loaded.auto_dir
    if d.is_symlink() or (d.exists() and not d.is_dir()):
        raise Refusal(str(d), "is a symlink or not a directory", "remove it and let build create a real dir")
    d.mkdir(parents=True, exist_ok=True)
    for a in loaded.auto.values():
        if a.path.parent != d:
            raise Refusal(str(a.path), "auto recipe path is outside <out>/recipes_auto", "report this")
        if a.path.is_symlink():
            raise Refusal(str(a.path), "is a symlink", "remove it by hand")
        old = a.path.read_text(encoding="utf-8") if a.path.is_file() else None
        if old != a.text:
            tmp = a.path.with_name(f".{a.path.name}.tmp{os.getpid()}")
            try:
                tmp.write_text(a.text, encoding="utf-8")
                os.replace(tmp, a.path)
            finally:
                with _suppress(OSError):
                    tmp.unlink()
            written.append(a.path)
    return written


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
        elif op == "soundset":  # soundset BASE OUT ROW LABEL RESREF GENDER: pad to ROW, append the character's row
            from pykotor.resource.formats.twoda import bytes_2da, read_2da
            table = read_2da(Path(argv[1]).read_bytes())
            headers = list(table.get_headers())
            row = int(argv[3])
            if table.get_height() > row:
                raise ValueError(f"base table has {table.get_height()} rows, above the planned row {row}")
            while table.get_height() < row:
                table.add_row(str(table.get_height()), {c: "" for c in headers})
            cells = {"label": argv[4], "resref": argv[5], "strref": "-1", "gender": argv[6]}
            table.add_row(str(row), {h: cells.get(h, "") for h in headers})
            Path(argv[2]).write_bytes(bytes_2da(table))
            print(json.dumps({"height": table.get_height()}))
        else:
            print(f"unknown _twoda op {op!r}", file=sys.stderr)
            return 2
    except Exception as e:  # noqa: BLE001  pykotor raises several error types
        print(f"error: _twoda {op}: {type(e).__name__}: {e}", file=sys.stderr)
        return 1
    return 0


def _audio_main(argv: list) -> int:
    """Hidden helper, run under the k1port venv python: ssf K1DIR RESREF | tlk FILE STRREF... (pykotor, read only).
    Prints one JSON object as its LAST stdout line (pykotor may log above it)."""
    op = argv[0]
    try:
        if op == "ssf":
            import logging
            logging.disable(logging.CRITICAL)
            from pykotor.extract.installation import Installation, SearchLocation
            from pykotor.resource.type import ResourceType
            res = Installation(str(Path(argv[1]))).resource(argv[2], ResourceType.SSF,
                                                            [SearchLocation.CHITIN, SearchLocation.OVERRIDE])
            if res is None:
                print(f"error: _audio ssf: {argv[2]}.ssf not found in the K1 chitin or Override", file=sys.stderr)
                return 1
            print(json.dumps({"data_b64": base64.b64encode(bytes(res.data)).decode("ascii")}))
        elif op == "tlk":
            from pykotor.resource.formats.tlk import read_tlk
            tlk = read_tlk(Path(argv[1]).read_bytes())
            out = {}
            for ref in argv[2:]:
                entry = tlk.get(int(ref))
                out[ref] = None if entry is None else str(entry.voiceover)
            print(json.dumps(out))
        else:
            print(f"unknown _audio op {op!r}", file=sys.stderr)
            return 2
    except Exception as e:  # noqa: BLE001  pykotor raises several error types
        print(f"error: _audio {op}: {type(e).__name__}: {e}", file=sys.stderr)
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
    """(planned {id: {appearance, head[, soundset]}}, live {appearance, head[, soundset]}, stored). Refuses when stored
    ids no longer fit. A head-less (single-model) recipe gets head = None and does not advance the heads counter, so the
    heads rows of the other characters are the same as if it were not in the list. A soundset row is planned only for
    recipes with [audio] (live soundset.2da height, then +1 per audio recipe in config order)."""
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
    headless = {r.id for r in loaded.recipes if r.head is None}
    planned, next_h = {}, start_h
    for i, cid in enumerate(cfg.characters):
        planned[cid] = {"appearance": start_a + i, "head": None if cid in headless else next_h}
        next_h += cid not in headless
    audio_ids = [r.id for cid in cfg.characters for r in loaded.recipes if r.id == cid and r.audio is not None]
    if audio_ids:
        live["soundset"] = twoda_height(runner, live_dir / "soundset.2da")
        start_s = live["soundset"] if cfg.first_soundset_row is None else cfg.first_soundset_row
        if start_s < live["soundset"]:
            raise Failure(f"[build] first_soundset_row = {start_s}", f"below the live soundset.2da row count ({live['soundset']}); it would overwrite rows",
                          f"use {live['soundset']} or higher, or remove the key")
        for i, cid in enumerate(audio_ids):
            planned[cid]["soundset"] = start_s + i
    stored = read_json(loaded.out / "assignments.json", "assignments") or {}
    if not isinstance(stored, dict):
        raise Failure(str(loaded.out / "assignments.json"), "not a JSON object", "delete it (rows are reassigned on the next build)")
    clash = []
    for cid in cfg.characters:
        old = stored.get(cid)
        if old is not None and rows_differ(old, planned[cid]):
            clash.append(f"{cid}: stored appearance {old.get('appearance')}/head {head_txt(old.get('head'))}/soundset {old.get('soundset')}, "
                         f"plan now {planned[cid]['appearance']}/{head_txt(planned[cid]['head'])}/{planned[cid].get('soundset')}")
    if clash and not reassign:
        warn("assigned rows no longer fit the live K2 tables or the config order: " + "; ".join(clash))
        raise Failure(str(loaded.out / "assignments.json"),
                      f"stored row ids differ from the plan (live appearance.2da has {live['appearance']} rows, heads.2da {live['head']}"
                      + (f", soundset.2da {live['soundset']}" if "soundset" in live else "") + ")",
                      "remove the installed packs and rebuild everything with --reassign, or restore the tables/config order")
    return planned, live, stored


def head_txt(head) -> str:
    """A heads row id for messages; a head-less (single-model) character has none."""
    return "none" if head is None else str(head)


def rows_differ(old: dict, new: dict) -> bool:
    """True when a stored assignment no longer matches the plan. A soundset row the stored entry never had (the recipe
    gained [audio] since the last build) is new, not a clash; a stored soundset the plan no longer wants is ignored."""
    return any(old.get(k) != v for k, v in new.items() if not (k == "soundset" and "soundset" not in old))


def save_assignment(loaded: Loaded, stored: dict, cid: str, rows: dict) -> dict:
    stored = dict(stored)
    stored[cid] = dict(rows)
    write_json(loaded.out / "assignments.json", stored)
    return stored


# ------------------------------------------------------------------------------------------- resolve step

def model_refs(recipe) -> list:
    return [m for m in (recipe.body_a, recipe.body_b, recipe.head) if m is not None]


def ingest_models(recipe) -> list:
    """Model refs in pykotor conversion order: body B, body A, head. Body A and the head have B as their K2 supermodel,
    and `--supernode chain` reads the just-written B from <stage>/models, so B must be converted first."""
    return [m for m in (recipe.body_b, recipe.body_a, recipe.head) if m is not None]


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
    if ctx.recipe.converter == "pykotor" and not no_model:
        # K2 chain supernode numbering; the supermodel (body B) is looked up in the stage first, then the K2 install
        argv += ["--supernode", "chain", "--supermodel-dir", ctx.stage / "models"]
        if ctx.recipe.keep_dangly:
            argv.append("--keep-dangly")  # keep danglymesh nodes (needs the pykotor_k2_dangly venv patch: setup_venv.sh)
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
    warn_supernode(label, data)
    return data


def supernode_report(data: dict) -> dict | None:
    """The model converter's "supernode" record from an ingest result (None for legacy output or no model)."""
    man = (data.get("model") or {}).get("manifest")
    rep = man.get("supernode") if isinstance(man, dict) else None
    return rep if isinstance(rep, dict) else None


def warn_supernode(label: str, data: dict) -> None:
    rep = supernode_report(data)
    if rep and rep.get("origin") == "override":
        warn(f"{label}: supernode numbering follows supermodel {rep.get('supermodel')!r} from the K2 Override folder "
             "(a user-modded file, sha256 " + str(rep.get("supermodel_sha256"))[:16] + "), not vanilla K2")


def summarize_ingest(data: dict) -> dict:
    return {"resref": data.get("resref"), "new_resref": data.get("new_resref"),
            "textures": [{"name": t["name"], "new": t["new"], "source_id": t["chosen"]["source_id"],
                          "requires_author_approval": t["chosen"].get("requires_author_approval")}
                         for t in data.get("textures", [])],
            "model_source": (data.get("model") or {}).get("chosen", {}).get("source_id"),
            **({"supernode": rep} if (rep := supernode_report(data)) else {}),
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


# ------------------------------------------------------------------------------------------- audio (K1 barks)

TLK_HINT = "this K2 TLK lacks the K1 row; the importer needs the K2 dialog.tlk that still carries the K1 character rows"


def find_child_ci(parent: Path, name: str) -> Path | None:
    """Case-insensitive child of parent (any kind), or None."""
    try:
        with os.scandir(parent) as it:
            for e in it:
                if e.name.lower() == name.lower():
                    return Path(e.path)
    except OSError:
        return None
    return None


def audio_call(runner: Runner, label: str, *args) -> dict:
    """Run the hidden `_audio` helper under the k1port venv; parse the JSON object on its last stdout line."""
    proc = runner.run(label, [str(k1port_py()), "-I", "-B", str(DRIVER), "_audio", *[str(a) for a in args]],
                      fix="run the importer's doctor: the k1port venv (pykotor) must work and the K1 install must hold the SSF")
    lines = [ln for ln in proc.stdout.splitlines() if ln.strip()]
    try:
        return json.loads(lines[-1])
    except (IndexError, json.JSONDecodeError) as e:
        raise Failure(label, f"helper printed no JSON: {e}", "re-run doctor") from None


def tlk_voiceovers(runner: Runner, tlk: Path, strrefs: list, which: str) -> dict:
    got = audio_call(runner, f"audio tlk {which}", "tlk", tlk, *strrefs)
    return {int(k): v for k, v in got.items()}


def audio_inspect(runner: Runner, recipe, k1: Path, k2: Path, reserved: tuple = (0, 0)) -> dict:
    """Read-only audio resolution: the K1 SSF bytes, its strrefs checked against both dialog.tlk files, and the K1
    voice wavs in <K1>/streamsounds/. `reserved` = (assets, bytes) the [dialog] part of the same pack needs, kept free of
    the extra barks. Raises Failure on anything unusable. Writes nothing."""
    a, cid = recipe.audio, recipe.id
    got = audio_call(runner, f"audio ssf {a.k1_ssf}", "ssf", k1, a.k1_ssf)
    try:
        data = base64.b64decode(got["data_b64"], validate=True)
    except (KeyError, ValueError) as e:
        raise Failure(f"audio {cid}", f"bad SSF payload from the helper: {e}", "re-run doctor") from None
    if len(data) != SSF_SIZE or data[:8] != SSF_MAGIC:
        raise Failure(f"audio {cid}: K1 {a.k1_ssf}.ssf", f"is {len(data)} bytes / magic {data[:8]!r}, expected {SSF_SIZE} bytes {SSF_MAGIC!r}",
                      "check [audio] k1_ssf names a K1 soundset file")
    strrefs = struct.unpack_from(f"<{SSF_SLOTS}I", data, 12)
    wanted = sorted({x for x in strrefs if x != SSF_EMPTY})
    if not wanted:
        raise Failure(f"audio {cid}: K1 {a.k1_ssf}.ssf", "holds no strrefs (all 28 slots empty)", "check [audio] k1_ssf")
    k1_tlk, k2_tlk = find_child_ci(k1, "dialog.tlk"), find_child_ci(k2, "dialog.tlk")
    for which, path, root in (("K1", k1_tlk, k1), ("K2", k2_tlk, k2)):
        if path is None or not path.is_file():
            raise Failure(f"audio {cid}", f"no dialog.tlk in the {which} dir {root}", f"set [{which.lower()}] dir to a full install")
    vo1 = tlk_voiceovers(runner, k1_tlk, wanted, "K1")
    vo2 = tlk_voiceovers(runner, k2_tlk, wanted, "K2")
    for ref in wanted:
        if vo1.get(ref) is None:
            raise Failure(f"audio {cid}: strref {ref}", f"not in the K1 dialog.tlk {k1_tlk}", "check [k1] dir is a full KOTOR 1 install")
        if vo2.get(ref) is None:
            raise Failure(f"audio {cid}: strref {ref}", f"missing in the K2 dialog.tlk {k2_tlk}", TLK_HINT)
        if vo1[ref].lower() != vo2[ref].lower():
            raise Failure(f"audio {cid}: strref {ref}", f"voiceover differs: K1 {vo1[ref]!r}, K2 {vo2[ref]!r}", TLK_HINT)
    voiceovers = sorted({vo1[r].lower() for r in wanted if vo1[r]})
    for vo in voiceovers:
        if not krecipe.RESREF_RE.fullmatch(vo):
            raise Failure(f"audio {cid}: voiceover {vo!r}", "is not a valid resref [a-z0-9_]{1,16}", "the pack cannot serve this wav")
    sdir = find_child_ci(k1, STREAM_DIR)
    index = {}
    if sdir is not None and sdir.is_dir() and not sdir.is_symlink():
        for e in os.scandir(sdir):
            if e.is_file(follow_symlinks=False):
                index[e.name.lower()] = Path(e.path)
    found = {vo: index[f"{vo}.wav"] for vo in voiceovers if f"{vo}.wav" in index}
    missing = [f"{vo}.wav" for vo in voiceovers if vo not in found]
    if not found:
        raise Failure(f"audio {cid}", f"none of the {len(voiceovers)} K1 voice wavs are in {sdir or k1 / STREAM_DIR}",
                      "check [k1] dir is a full KOTOR 1 install with a streamsounds/ folder")
    extra, dropped = extra_barks(recipe, index, found, set(voiceovers), reserved)
    return {"ssf": data, "strrefs": list(strrefs), "voiceovers": voiceovers, "found": found, "missing": missing,
            "extra": extra, "extra_dropped": dropped}


def extra_barks(recipe, index: dict, found: dict, referenced: set, reserved: tuple = (0, 0)) -> tuple[dict, list]:
    """The K1 voice wavs named `<k1_ssf>_*.wav` that the SSF does not reference (Carth: p_carth_tia), as {resref: path},
    plus the names dropped to stay under the pack limits (asset count, half the pack byte cap). Sorted by name, so the
    cut is deterministic. A name that is not a valid resref is dropped too. `reserved` = (assets, bytes) left free for the
    recipe's [dialog]."""
    prefix = recipe.audio.k1_ssf.lower() + "_"
    cands = sorted((n[:-4], p) for n, p in index.items() if n.startswith(prefix) and n.endswith(".wav")
                   and n[:-4] not in referenced)
    base = 2 * len(model_refs(recipe)) + len(recipe.textures) + 2  # models mdl+mdx, tpc, the utc and the ssf
    room = kpack.MAX_ASSETS - base - len(found) - reserved[0]
    budget = kpack.MAX_FILE_BYTES // 2 - sum(p.stat().st_size for p in found.values()) - reserved[1]
    extra, dropped = {}, []
    for stem, path in cands:
        size = path.stat().st_size
        if not krecipe.RESREF_RE.fullmatch(stem) or len(extra) >= room or size > budget:
            dropped.append(f"{stem}.wav")
            continue
        extra[stem] = path
        budget -= size
    return extra, dropped


def stage_audio(ctx: "Ctx", info: dict) -> dict:
    """Copy the SSF and the K1 wavs into <cdir>/audio (the K1 dir is only read). Returns the manifest record + file lists."""
    a = ctx.recipe.audio
    adir = ctx.cdir / "audio"
    sdir = adir / STREAM_DIR  # a path part named streamsounds is what makes collect_assets mark the wavs STREAM
    sdir.mkdir(parents=True, exist_ok=True)
    ssf = adir / f"{a.k2_ssf}.ssf"
    ssf.write_bytes(info["ssf"])
    wavs, rec = [], []
    for vo, src in sorted(info["found"].items()):
        dst = sdir / f"{vo}.wav"
        shutil.copyfile(src, dst)
        wavs.append(dst)
        rec.append({"resref": vo, "file": f"{STREAM_DIR}/{src.name}", "sha256": sha256_file(dst), "size": dst.stat().st_size,
                    "origin": VANILLA_ID, "pack_flags": kpack.FLAG_STREAM})
    extra_rec = []
    for vo, src in sorted(info.get("extra", {}).items()):  # K1 barks the SSF does not reference (e.g. p_carth_tia)
        dst = sdir / f"{vo}.wav"
        shutil.copyfile(src, dst)
        wavs.append(dst)
        extra_rec.append({"resref": vo, "file": f"{STREAM_DIR}/{src.name}", "sha256": sha256_file(dst), "size": dst.stat().st_size,
                          "origin": VANILLA_ID, "pack_flags": kpack.FLAG_STREAM})
    for w in info["missing"]:
        warn(f"{ctx.recipe.id}: K1 voice wav {w} is not in {STREAM_DIR}/; that bark stays silent (recorded in the manifest)")
    if info.get("extra_dropped"):
        warn(f"{ctx.recipe.id}: {len(info['extra_dropped'])} extra K1 bark wav(s) left out to stay under the pack limits: "
             + ", ".join(info["extra_dropped"][:8]) + (" ..." if len(info["extra_dropped"]) > 8 else ""))
    return {"ssf": ssf, "wavs": wavs,
            "manifest": {"k1_ssf": a.k1_ssf, "k2_ssf": a.k2_ssf, "ssf_sha256": sha256_bytes(info["ssf"]), "ssf_size": len(info["ssf"]),
                         "strrefs": info["strrefs"], "origin": VANILLA_ID, "wavs": rec, "extra": extra_rec,
                         "extra_dropped": list(info.get("extra_dropped", [])), "missing_optional": list(info["missing"])}}


def soundset_row_cmd(ctx: "Ctx", base: Path, row: int) -> list:
    r = ctx.recipe
    return [str(k1port_py()), "-I", "-B", str(DRIVER), "_twoda", "soundset", str(base), str(ctx.cdir / "override" / "soundset.2da"),
            str(row), label_for(r), r.audio.k2_ssf, str(r.gender)]


def prepare_soundset_base(ctx: "Ctx", order: list, idx: int, row: int) -> Path:
    """The table the new soundset row is appended to: the nearest previous audio character's staged table, else the
    live <K2>/override/soundset.2da. It must not be taller than the planned row (the helper pads below it)."""
    cands = []
    for prev in reversed(order[:idx]):
        rec = next((x for x in ctx.loaded.recipes if x.id == prev), None)
        f = ctx.loaded.out / prev / "override" / "soundset.2da"
        if rec is not None and rec.audio is not None and f.is_file():
            cands.append(f)
            break
    cands.append(live_tables(ctx.loaded.cfg.k2_dir) / "soundset.2da")
    for f in cands:
        if twoda_height(ctx.runner, f) <= row:
            return f
    raise Failure(f"{ctx.recipe.id} soundset row", f"no base soundset.2da fits the planned row {row}", "build with --reassign")


# ------------------------------------------------------------------------------------------- dialog (K1 conversation, voice, lips)

def dialog_call(runner: Runner, label: str, *args) -> dict:
    """Run the hidden `_dialog` helper under the k1port venv; parse the JSON object on its last stdout line."""
    proc = runner.run(label, [str(k1port_py()), "-I", "-B", str(DRIVER), "_dialog", *[str(a) for a in args]],
                      fix="run the importer's doctor: the k1port venv (pykotor) must work and the K1 install must hold the conversation")
    lines = [ln for ln in proc.stdout.splitlines() if ln.strip()]
    try:
        return json.loads(lines[-1])
    except (IndexError, json.JSONDecodeError) as e:
        raise Failure(label, f"helper printed no JSON: {e}", "re-run doctor") from None


def resolve_k1_dlg(runner: Runner, loaded: Loaded, recipe, k1: Path) -> str:
    """[dialog] k1_dlg, else the conversation of the character's K1 creature template (an auto character knows its UTC;
    a recipe file is tried as p_<id>, then <id>)."""
    d = recipe.dialog
    if d.k1_dlg:
        return d.k1_dlg
    a = loaded.auto.get(recipe.id)
    cands = [a.utc] if a is not None else [f"p_{recipe.id}", recipe.id]
    facts = scan_facts_many(runner, k1, cands)
    for c in cands:
        conv = str((facts.get(c) or {}).get("conversation") or "").strip().lower()
        if conv and krecipe.RESREF_RE.fullmatch(conv):
            return conv
    raise Failure(f"dialog {recipe.id}", f"no [dialog] k1_dlg, and the K1 creature template(s) {', '.join(cands)} "
                  "do not exist or have no conversation", "set k1_dlg in the recipe's [dialog] table")


def dialog_inspect(runner: Runner, loaded: Loaded, recipe, k1: Path, *, write_dlg: bool) -> dict:
    """Read-only dialog conversion: the K1 DLG + TLK text, the streamwaves index and the lips, converted by
    k1_port_dialog.convert_dialog. write_dlg=False (dry run) skips building the DLG bytes (the asset count is the same).
    Raises Failure on anything unusable. Writes nothing under the K1/K2 dirs."""
    d, cid = recipe.dialog, recipe.id
    k1_dlg = resolve_k1_dlg(runner, loaded, recipe, k1)
    tlk = find_child_ci(k1, "dialog.tlk")
    if tlk is None or not tlk.is_file():
        raise Failure(f"dialog {cid}", f"no dialog.tlk in the K1 dir {k1}", "set [k1] dir to a full KOTOR 1 install")
    got = dialog_call(runner, f"dialog read {k1_dlg}", "read", k1, tlk, k1_dlg)
    if not got.get("found"):
        raise Failure(f"dialog {cid}: K1 {k1_dlg}.dlg", "not found in the K1 install (Override, chitin or modules)",
                      "check [dialog] k1_dlg names a K1 conversation, and [k1] dir is a full KOTOR 1 install")
    graph, strings = got["graph"], {int(k): v for k, v in (got.get("strings") or {}).items()}
    sdir = find_child_ci(k1, STREAMWAVES_DIR)
    vo_index, vinfo = kdialog.build_vo_index(sdir) if sdir is not None else ({}, {"files": 0, "duplicates": [], "skipped_symlinks": 0})
    if not vo_index:
        warn(f"{cid}: no voice wavs found under {sdir or k1 / STREAMWAVES_DIR}; the whole dialog will be silent")
    lip_src: dict[str, str] = {}

    def lip_lookup(names: list) -> dict:
        lg = dialog_call(runner, "dialog lips", "lips", k1, *names)
        out = {}
        for n, v in (lg.get("lips") or {}).items():
            if v is None:
                out[n] = None
            else:
                out[n] = base64.b64decode(v["b64"], validate=True)
                lip_src[n] = v["mod"]
        return out

    def write(graph_out: dict) -> bytes:
        with tempfile.TemporaryDirectory(prefix="k1port_dialog_") as td:
            f = Path(td) / "graph.json"
            f.write_text(json.dumps(graph_out), encoding="utf-8")
            wg = dialog_call(runner, "dialog write", "write", f)
        return base64.b64decode(wg["data_b64"], validate=True)

    try:
        res = kdialog.convert_dialog(graph, strings.get, vo_index, lip_lookup, max_voiced=d.max_voiced, k2_dlg=d.k2_dlg,
                                     write_dlg=write if write_dlg else None)
    except kdialog.DialogError as e:
        raise Failure(f"dialog {cid}: {e.location}", e.cause, e.fix) from None
    res.update({"k1_dlg": k1_dlg, "k1_dlg_source": got.get("source"), "k1_dlg_sha256": got.get("sha256"),
                "lip_sources": lip_src, "vo_index": vo_index, "vo_index_info": vinfo})
    return res


def dialog_reserved(info: dict | None) -> tuple:
    """(assets, bytes) the dialog needs in the pack, for audio_inspect's extra-bark cap."""
    if info is None:
        return (0, 0)
    return (info["report"]["asset_count"], info["report"]["wav_bytes"])


def pack_asset_total(recipe, audio: dict | None, dialog: dict | None) -> int:
    """Assets the pack will hold: models (mdl+mdx), tpc, utc, + ssf and its wavs, + dlg/wavs/lips."""
    total = 2 * len(model_refs(recipe)) + len(recipe.textures) + 1
    if audio is not None:
        total += 1 + len(audio["found"]) + len(audio.get("extra", {}))
    if dialog is not None:
        total += dialog["report"]["asset_count"]
    return total


def check_pack_budget(recipe, audio: dict | None, dialog: dict | None) -> int:
    total = pack_asset_total(recipe, audio, dialog)
    if total > kpack.MAX_ASSETS:
        rep = (dialog or {}).get("report", {})
        raise Failure(f"dialog {recipe.id}", f"the pack would hold {total} assets, over the {kpack.MAX_ASSETS}-asset pack cap "
                      f"(dialog adds {rep.get('asset_count', 0)}: 1 dlg + {rep.get('voiced_lines', 0)} wav + {rep.get('lips_ok', 0)} lip)",
                      "lower [dialog] max_voiced (each voiced line is a wav plus a lip in the pool shared with every mod)")
    return total


def warn_dialog(cid: str, rep: dict) -> None:
    if rep["lips_missing"]:
        warn(f"{cid}: {len(rep['lips_missing'])} voiced line(s) have no lip in <K1>/lips/*.mod and ship without one: "
             + ", ".join(rep["lips_missing"][:8]) + (" ..." if len(rep["lips_missing"]) > 8 else ""))
    for f in rep["lips_failed"]:
        warn(f"{cid}: lip {f['name']} fails the LIP structure check ({'; '.join(f['problems'])}); that line ships without a lip")
    if rep["silent_no_wav"]:
        warn(f"{cid}: {rep['silent_no_wav']} node(s) name a voice wav that is not in {STREAMWAVES_DIR}/ and stay silent: "
             + ", ".join(rep["silent_no_wav_names"][:6]) + (" ..." if len(rep["silent_no_wav_names"]) > 6 else ""))
    if rep["tlk_missing"]:
        warn(f"{cid}: {len(rep['tlk_missing'])} text strref(s) are not in the K1 dialog.tlk; those lines are empty")
    if rep["camera_angle_reset"]:
        warn(f"{cid}: {rep['camera_angle_reset']} node(s) used a static/animated camera angle; reset to the default camera")


def say_dialog(cid: str, info: dict, d, total: int) -> None:
    rep = info["report"]
    say(f"[{cid}]   dialog: K1 {info['k1_dlg']}.dlg -> {d.k2_dlg}.dlg (UTC conversation + default_dlg), voice budget {d.max_voiced}")
    for line in kdialog.report_lines(rep):
        say(f"[{cid}]   dialog: {line}")
    say(f"[{cid}]   dialog: voice wavs -> {STREAMVOICE_DIR}/ STREAM assets (bytes unchanged), lips + dlg -> RESMAN assets; "
        f"pack would hold {total}/{kpack.MAX_ASSETS} assets")


def stage_dialog(ctx: "Ctx", info: dict) -> dict:
    """Write the DLG, the voice wavs and the lips into <cdir>/dialog (the K1 dir is only read). Returns the file list and
    the manifest record."""
    r, d = ctx.recipe, ctx.recipe.dialog
    ddir = ctx.cdir / DIALOG_DIR
    sub = {"dlg": ddir, "wav": ddir / STREAMVOICE_DIR, "lip": ddir / LIPS_DIR}  # a path part named streamvoice marks the wavs STREAM
    files, dlg_rec, wav_rec, lip_rec = [], None, [], []
    by_name = {v["name"]: v for v in info["voiced"]}
    for name, data, kind in info["assets"]:
        dst = sub[kind] / name
        dst.parent.mkdir(parents=True, exist_ok=True)
        dst.write_bytes(data)
        files.append(dst)
        stem = name.rsplit(".", 1)[0]
        base = {"resref": stem, "sha256": sha256_bytes(data), "size": len(data), "origin": VANILLA_ID}
        if kind == "dlg":
            dlg_rec = {**base, "k1_dlg": info["k1_dlg"], "k1_sha256": info["k1_dlg_sha256"], "converted": True}
        elif kind == "wav":
            src = info["vo_index"].get(by_name[stem]["k1"])
            try:
                rel = str(Path(src).relative_to(ctx.k1)) if src is not None else ""
            except ValueError:
                rel = str(src)
            wav_rec.append({**base, "k1_name": by_name[stem]["k1"], "file": rel, "pack_flags": kpack.FLAG_STREAM})
        else:
            lip_rec.append({**base, "file": f"{LIPS_DIR}/{info['lip_sources'].get(by_name[stem]['k1'], '?')}", "check": "kvoice_lip.check_lip ok"})
    if dlg_rec is None:
        raise Failure(f"dialog {r.id}", "the converter produced no DLG bytes", "report this")
    rep = info["report"]
    warn_dialog(r.id, rep)
    return {"files": files, "manifest": {"k1_dlg": info["k1_dlg"], "k1_dlg_source": info["k1_dlg_source"], "k2_dlg": d.k2_dlg,
                                         "max_voiced": d.max_voiced, "policy": kdialog.POLICY, "stats": rep, "dlg": dlg_rec,
                                         "wavs": wav_rec, "lips": lip_rec, "voiced": info["voiced"],
                                         "vo_index": info["vo_index_info"]}}


# ------------------------------------------------------------------------------------------- per-character context

@dataclass
class Ctx:
    loaded: Loaded
    recipe: object
    k1: Path
    cdir: Path
    runner: Runner
    env: dict
    audio: dict | None = None  # stage_audio() result when the recipe has [audio]
    dialog: dict | None = None  # stage_dialog() result when the recipe has [dialog]

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
    soundset = rows["soundset"] if r.audio is not None else r.soundset  # [audio]: the new row; else the recipe fallback
    argv = py_cmd(k1port_py(), CHAR_BUILD_TOOL, "--game-dir", ctx.loaded.cfg.k2_dir, "--tag", r.id,
                  "--label", label_for(r), "--new-row", rows["appearance"], "--body-a", r.body_a.k2, "--soundset", soundset,
                  "--gender", r.gender, "--name", r.display_name, "--utc", r.utc, "--template-row", r.template_row)
    if r.head is not None:  # B-type character: body B (body A when the recipe has none) + a heads.2da row
        argv += ["--new-head-row", rows["head"], "--body-b", (r.body_b or r.body_a).k2, "--head", r.head.k2]
    if r.dlg:
        argv += ["--conversation", r.dlg]
    if chain_from is not None:
        argv += ["--chain-from", chain_from]
    if r.audio is not None:
        argv += ["--soundset-table", ctx.cdir / "override" / "soundset.2da"]
    return argv + ["--out", ctx.cdir / "override", "--json"]


def prepare_base(ctx: Ctx, order: list, idx: int, rows: dict) -> Path | None:
    """The --chain-from dir for char_build: the previous character's tables, else the live tables padded to the
    planned row (a pad copy in <cdir>/base). None = no chain (the live tables already have exactly the planned count).
    The heads table is only needed (and only chained) for a character with a heads row: a head-less recipe writes no
    heads.2da, so the heads source is the nearest earlier character dir that has one, else the live table."""
    live = live_tables(ctx.loaded.cfg.k2_dir)
    want_h = rows["head"]
    prev = [ctx.loaded.out / cid / "override" for cid in reversed(order[:idx])]

    def pick(name: str, target: int, dirs: list):
        for cand in [*dirs, None]:
            d = cand or live
            if cand is not None and not (d / name).is_file():
                continue
            h = twoda_height(ctx.runner, d / name)
            if h <= target:
                return cand, d, h
        return None

    # appearance.2da always chains from the previous character; heads.2da from the nearest one that wrote it
    app = pick("appearance.2da", rows["appearance"], prev[:1])
    heads = pick("heads.2da", want_h, prev) if want_h is not None else None
    if app is None or (want_h is not None and heads is None):
        raise Failure(f"{ctx.recipe.id} rows", "no base table fits the planned rows", "build with --reassign")
    exact = app[2] == rows["appearance"] and (want_h is None or heads[2] == want_h)
    if exact and (want_h is None or app[0] == heads[0]):
        return app[0]
    base = ctx.cdir / "base"
    base.mkdir(parents=True, exist_ok=True)
    sources = [("appearance.2da", rows["appearance"], app)] + ([("heads.2da", want_h, heads)] if want_h is not None else [])
    for name, target, (_cand, d, height) in sources:
        if height == target:
            shutil.copyfile(d / name, base / name)
        else:
            twoda_call(ctx.runner, f"2da-pad {name}", "pad", d / name, base / name, target)
    return base


def cells_for(ctx: Ctx, rows: dict) -> list:
    r = ctx.recipe
    from k1_port_vfs_pack import TwoDaCell
    tds = []
    specs = [("appearance", "appearance.2da", rows["appearance"], r.template_row)]
    if rows["head"] is not None:  # a head-less (single-model) character ships no heads.2da cells
        specs.append(("heads", "heads.2da", rows["head"], -1))
    if r.audio is not None:  # the new soundset row: no template, every other column stays empty
        specs.append(("soundset", "soundset.2da", rows["soundset"], -1))
    for table, fname, row, template in specs:
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
    if ctx.audio is not None:
        files += [ctx.audio["ssf"], *ctx.audio["wavs"]]
    if ctx.dialog is not None:
        files += ctx.dialog["files"]
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
        for m in ingest_models(r):
            own = textures_owned.get(m.k1) or []
            cmds.append((f"ingest {m.k1}", ingest_argv(ctx, m.k1, m.k2, own, no_model=False,
                                                       supermodel_map=[] if m is r.body_b else sm)))
        if textures_owned.get(""):
            cmds.append(("ingest textures", ingest_argv(ctx, r.body_a.k1, None, textures_owned[""], no_model=True, supermodel_map=[])))
    else:
        cmds += mdlops_steps(ctx, r)
        cmds.append(("ingest textures", ingest_argv(ctx, r.body_a.k1, None, list(r.textures), no_model=True, supermodel_map=[])))
    chain = ctx.loaded.out / order[idx - 1] / "override" if idx else None
    audio = dialog = None
    if r.dialog is not None:
        dialog = dialog_inspect(ctx.runner, ctx.loaded, r, ctx.k1, write_dlg=False)  # read only
    if r.audio is not None:
        audio = audio_inspect(ctx.runner, r, ctx.k1, ctx.loaded.cfg.k2_dir, dialog_reserved(dialog))  # read only
        base = ctx.loaded.out / order[idx - 1] / "override" / "soundset.2da" if idx else live_tables(ctx.loaded.cfg.k2_dir) / "soundset.2da"
        cmds.append(("soundset row", soundset_row_cmd(ctx, base, rows["soundset"])))
    cmds.append(("char_build", char_build_argv(ctx, rows, chain)))
    for t in r.textures:
        cmds.append((f"tex {t.k2}", tpc_argv(ctx, t, ctx.stage / "override" / f"{t.k2.lower()}.<tga>", ctx.cdir / "tpc" / f"{t.k2.lower()}.tpc")))
    say(f"[{r.id}] dry run: appearance row {rows['appearance']}, heads row {head_txt(rows['head'])}"
        + (f", soundset row {rows['soundset']}" if audio else "") + f", converter {r.converter}" + (" (keep_dangly)" if r.keep_dangly else "") + ", "
        f"texture_source {ctx.loaded.cfg.texture_source}")
    if audio:
        refs = [x for x in audio["strrefs"] if x != SSF_EMPTY]
        say(f"[{r.id}]   audio: K1 {r.audio.k1_ssf}.ssf (172 bytes, {len(refs)} strrefs {min(refs)}..{max(refs)}, same voiceovers in the K2 dialog.tlk) "
            f"-> {r.audio.k2_ssf}.ssf; {len(audio['found'])}/{len(audio['voiceovers'])} K1 voice wavs from {STREAM_DIR}/ -> STREAM assets"
            + (f" (+{len(audio['extra'])} extra {r.audio.k1_ssf}_*.wav" + (f", {len(audio['extra_dropped'])} left out by the pack limits"
                                                                          if audio["extra_dropped"] else "") + ")" if audio["extra"] or audio["extra_dropped"] else "")
            + (f"; missing (WARN): {', '.join(audio['missing'])}" if audio["missing"] else ""))
        say(f"[{r.id}]   audio: new soundset.2da row {rows['soundset']} (label {label_for(r)}, resref {r.audio.k2_ssf}, gender {r.gender}); "
            f"the UTC uses it instead of recipe soundset {r.soundset}")
    if dialog:
        say_dialog(r.id, dialog, r.dialog, check_pack_budget(r, audio, dialog))
        warn_dialog(r.id, dialog["report"])
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
    say(f"[{cid}] 1/6 resolve K1 sources")
    resolver = Runner(None)  # logged into the manifest only: the work dir is wiped below
    resolved = resolve_character(resolver, loaded, recipe, k1)
    audio_info = dialog_info = None
    if recipe.dialog is not None and not dry_run:  # read only; fails before the work dir is wiped
        dialog_info = dialog_inspect(resolver, loaded, recipe, k1, write_dlg=True)
    if recipe.audio is not None and not dry_run:  # read only; fails before the work dir is wiped
        audio_info = audio_inspect(resolver, recipe, k1, loaded.cfg.k2_dir, dialog_reserved(dialog_info))
    if not dry_run and dialog_info is not None:
        check_pack_budget(recipe, audio_info, dialog_info)
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
        for m in ingest_models(recipe):
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
    audio_man = None
    if recipe.audio is not None:
        say(f"[{cid}] 3/6 audio: K1 {recipe.audio.k1_ssf}.ssf + K1 voice wavs -> {recipe.audio.k2_ssf}.ssf, soundset row {rows['soundset']}")
        ctx.audio = stage_audio(ctx, audio_info)
        audio_man = ctx.audio["manifest"]
        say(f"[{cid}]   {len(ctx.audio['wavs'])} wav(s) staged as STREAM assets ({len(audio_man['extra'])} extra), "
            f"{len(audio_man['missing_optional'])} missing")
    dialog_man = None
    if recipe.dialog is not None:
        say(f"[{cid}] 3/6 dialog: K1 {dialog_info['k1_dlg']}.dlg + voice-over + lips -> {recipe.dialog.k2_dlg}.dlg")
        ctx.dialog = stage_dialog(ctx, dialog_info)
        dialog_man = ctx.dialog["manifest"]
        say_dialog(cid, dialog_info, recipe.dialog, pack_asset_total(recipe, audio_info, dialog_info))
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

    say(f"[{cid}] 5/6 rows {rows['appearance']}/{head_txt(rows['head'])}" + (f" + soundset {rows['soundset']}" if recipe.audio else "") + " + UTC")
    chain = prepare_base(ctx, order, idx, rows)
    if recipe.audio is not None:
        (cdir / "override").mkdir(parents=True, exist_ok=True)
        runner.run(f"soundset row {cid}", soundset_row_cmd(ctx, prepare_soundset_base(ctx, order, idx, rows["soundset"]), rows["soundset"]),
                   env=ctx.env, fix="the live K2 soundset.2da changed? re-run; if the count moved use --reassign")
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
                                   head_id=rows["head"] if rows["head"] is not None else 0,  # pack field is a u32: unused without a heads row
                                   soundset_id=rows["soundset"] if recipe.audio is not None else recipe.soundset)
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
        "recipe": {"path": recipe.source, "sha256": recipe_file_sha(recipe), "converter": recipe.converter,
                   "keep_dangly": recipe.keep_dangly,
                   "auto": ({"k1_utc": loaded.auto[cid].utc, "sha256": sha256_bytes(loaded.auto[cid].text.encode("utf-8"))}
                            if cid in loaded.auto else None)},
        "k1_dir": str(k1), "k2_dir": str(loaded.cfg.k2_dir), "texture_source": loaded.cfg.texture_source,
        "sources": resolved["rows"],
        "missing_optional": resolved["missing"] + (audio_man["missing_optional"] if audio_man else []),
        "audio": audio_man,
        "dialog": dialog_man,
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
        f"rows {rows['appearance']}/{head_txt(rows['head'])}" + (f"/{rows['soundset']}" if recipe.audio else "") + f"), sha256 {summary['sha256'][:16]}")
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
    for r in chosen:
        a = loaded.auto.get(r.id)
        if a is not None:
            say(f"[{r.id}] auto recipe: derived from K1 creature template {a.utc} ({'would write' if args.dry_run else 'writing'} {a.path})")
    if not args.dry_run:
        write_auto(loaded)
    plan_runner = Runner(None)
    rows, live, stored = plan_assignments(loaded, plan_runner, args.reassign)
    ids = installed_juhani_char(loaded.cfg.k2_dir)
    if ids and juhani_rows_clash(rows):
        warn(JUHANI_WARN.format(ids=", ".join(ids)))
    order = list(loaded.cfg.characters)
    say(f"live K2 tables: appearance.2da {live['appearance']} rows, heads.2da {live['head']} rows"
        + (f", soundset.2da {live['soundset']} rows" if "soundset" in live else ""))
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
        warn_asset_budget(loaded)
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


ASSET_POOL = 192  # asset-vfs static asset pool: ONE pool for ALL registrants (a pack alone may also hold 192)


def warn_asset_budget(loaded: Loaded) -> int:
    """Sum of assets over every built pack of the config; WARN when it exceeds the shared asset-vfs pool."""
    total = 0
    for cid in loaded.cfg.characters:
        man = read_json(loaded.out / f"{cid}.import_manifest.json", "manifest") or {}
        total += int(man.get("pack", {}).get("asset_count") or 0)
    if total > ASSET_POOL:
        warn(f"asset budget: the built packs need {total} assets, the shared asset-vfs pool holds {ASSET_POOL} for ALL mods "
             f"(juhani-char alone uses about 51). Assets past the limit are not served. Lower [dialog] max_voiced, drop extra "
             f"barks, or build fewer characters.")
    else:
        say(f"asset budget: {total}/{ASSET_POOL} assets (shared with every other asset-vfs client, e.g. juhani-char)")
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
    loaded = load_all(args.config, cached=True)
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
    loaded = load_all(args.config, cached=True)
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
    dman = (read_json(loaded.out / f"{args.id}.import_manifest.json", "manifest") or {}).get("dialog")
    if dman:
        say(f"the pack also served dialog {dman.get('k2_dlg')} ({len(dman.get('wavs', []))} voice wav(s), {len(dman.get('lips', []))} lip(s)); "
            f"it is gone with the pack (nothing else was written to the game dir)")
    stored = read_json(loaded.out / "assignments.json", "assignments") or {}
    if isinstance(stored, dict) and args.id in stored:
        old = stored.pop(args.id)
        write_json(loaded.out / "assignments.json", stored)
        say(f"dropped the assignment (appearance {old.get('appearance')}, heads {head_txt(old.get('head'))}"
            + (f", soundset {old['soundset']}" if "soundset" in old else "") + "); those row ids stay unused "
            f"until a rebuild of {args.id} (rows are positional in config order)")
    say("Close the game before relaunching.")
    return EXIT_OK


def _dialog_summary(dialog_man: dict | None) -> dict | None:
    """The few counts `list` shows from a manifest's dialog record."""
    if not dialog_man:
        return None
    st = dialog_man.get("stats", {})
    return {k: st.get(k) for k in ("nodes_total", "nodes_kept", "starters_kept", "voiced_lines", "asset_count", "lips_ok")}


def cmd_list(args) -> int:
    loaded = load_all(args.config, cached=True)
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
        rows.append({"id": r.id, "display_name": r.display_name, "converter": r.converter, "auto": r.id in loaded.auto,
                     "auto_utc": loaded.auto[r.id].utc if r.id in loaded.auto else None, "built": built,
                     "pack_sha256": sha, "stale": stale, "installed": installed,
                     "installed_matches_build": bool(installed and sha and sha256_file(dest) == sha),
                     "assigned": stored.get(r.id), "flagged": len(man.get("flagged", [])) if man else None,
                     "dialog": ({"k2_dlg": r.dialog.k2_dlg, "max_voiced": r.dialog.max_voiced,
                                 "built": _dialog_summary(man.get("dialog"))} if r.dialog is not None else None)})
    if args.json:
        print(json.dumps({"out": str(loaded.out), "k2_dir": str(loaded.cfg.k2_dir), "characters": rows}, indent=2, sort_keys=True))
        return EXIT_OK
    say(f"out: {loaded.out}")
    for x in rows:
        a = x["assigned"]
        rowtxt = (f"rows {a['appearance']}/{head_txt(a['head'])}" + (f"/{a['soundset']}" if "soundset" in a else "")) if a else "rows unassigned"
        state = ("built" + (" (stale: recipe changed)" if x["stale"] else "")) if x["built"] else "not built"
        inst_txt = ("installed" + ("" if x["installed_matches_build"] else " (differs from build)")) if x["installed"] else "not installed"
        sha = (x["pack_sha256"] or "-")[:16]
        dlg = x["dialog"]
        dtxt = ""
        if dlg:
            b = dlg["built"]
            dtxt = f"  dialog {dlg['k2_dlg']}" + (f" ({b['nodes_kept']}/{b['nodes_total']} nodes, {b['voiced_lines']} voiced, "
                                                  f"{b['lips_ok']} lips)" if b else " (not built)")
        say(f"{x['id']:<14} {state:<24} {inst_txt:<30} {rowtxt:<18} sha {sha}" + (f"  auto from K1 {x['auto_utc']}" if x["auto"] else "") + dtxt)
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
        if loaded.auto:
            ok("auto recipes", ", ".join(f"{a.id} <- K1 {a.utc}" for a in loaded.auto.values())
               + " (derived from your K1 install; a recipes/<id>.toml would override)")
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
        tables = ("appearance.2da", "heads.2da") + (("soundset.2da",) if any(r.audio is not None for r in loaded.recipes) else ())
        for fname in tables:
            f = live_tables(cfg.k2_dir) / fname
            if f.is_file():
                ok(f"K2 {fname}", str(f))
            elif k2_has_game(cfg.k2_dir):
                bad(f"K2 {fname}", f"{f} not found (k1_port_char_build reads the live table from override/)",
                    f"put the current {fname} into <K2>/override/ (extract it from the K2 BIFs with a tool such as KotOR Tool)")
        dlg_ids = [r.id for r in loaded.recipes if r.dialog is not None]
        if dlg_ids:
            label = f"[dialog] ({', '.join(dlg_ids)})"
            k1d = cfg.k1_dir.expanduser()
            tlk = find_child_ci(k1d, "dialog.tlk") if k1d.is_dir() else None
            if tlk is not None and tlk.is_file():
                ok(f"K1 dialog.tlk {label}", str(tlk))
            else:
                bad(f"K1 dialog.tlk {label}", f"not found in {k1d}", "set [k1] dir to a full KOTOR 1 install (the dialog text is embedded from it)")
            for what, fix in ((STREAMWAVES_DIR, "the dialog voice-over lives in <K1>/streamwaves/<module>/<speaker>/; without it the dialog is silent"),
                              (LIPS_DIR, "<K1>/lips/*.mod hold the lip-sync files; without them the dialog has no lips")):
                sub = find_child_ci(k1d, what) if k1d.is_dir() else None
                if sub is not None and sub.is_dir() and not sub.is_symlink():
                    ok(f"K1 {what}/ {label}", str(sub))
                else:
                    maybe(f"K1 {what}/ {label}", f"not found in {k1d}", fix)
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


# ------------------------------------------------------------------------------------------- scan / npcs

def scan_context(args) -> tuple[Path, "krecipe.Config | None"]:
    """(K1 dir, config or None) for scan/npcs. The config is optional: without one the K1 dir is --k1-dir, env
    KOTOR1_ROOT or the Steam default. With one, --k1-dir then the config's [k1] dir win."""
    cfg = None
    path = Path(args.config or DEFAULT_CONFIG).expanduser()
    if path.is_file():
        try:
            cfg = krecipe.load_config(path)
        except krecipe.RecipeError as e:
            if args.config:
                raise Failure(str(path), str(e), "fix the config named in the message") from None
    elif args.config:
        raise Failure(str(path), "config file not found", "pass an existing --config or drop the option")
    try:
        if cfg is not None:
            return kresolve.find_k1_dir(args.k1_dir or str(cfg.k1_dir), None, env={}), cfg
        return kresolve.find_k1_dir(args.k1_dir, None, env=os.environ), None
    except kresolve.ResolveError as e:
        raise Failure("[k1] dir", str(e), "pass --k1-dir, set [k1] dir in the config, or set KOTOR1_ROOT") from None


def cmd_scan(args) -> int:
    resref = (args.utc or "").lower()
    if not krecipe.RESREF_RE.fullmatch(resref):
        raise Failure("scan", f"bad K1 creature template {args.utc!r}: must match [a-z0-9_]{{1,16}}", "npcs --grep NAME lists them")
    k1, cfg = scan_context(args)
    runner = Runner(None)
    facts = scan_facts(runner, k1, resref)
    if facts is None:
        raise Failure(f"K1 creature template {resref!r}", f"not found in the K1 install {k1} (templates.bif, Override or modules)",
                      "npcs --grep NAME lists the K1 creature templates")
    try:
        derived, text = kscan.derive_text(facts, args.id)
    except kscan.ScanError as e:
        raise Failure(e.location, e.cause, e.fix) from None
    for n in derived.notes:
        warn(f"{derived.id}: {n}")
    print(text, end="", flush=True)
    if args.write is None:
        return EXIT_OK
    if args.write == "":
        if cfg is None:
            raise Failure("--write", "no DIR given and no config to name the out dir", "pass --write DIR or --config C")
        dest_dir = Path(os.path.abspath(cfg.out)) / kscan.AUTO_DIR
    else:
        dest_dir = Path(os.path.abspath(Path(args.write).expanduser()))
    protected = [k1] + ([cfg.k1_dir, cfg.k2_dir] if cfg else [])
    real = dest_dir.resolve()
    for prot in protected:
        pr = Path(prot).expanduser().resolve()
        if real == pr or pr in real.parents:
            raise Refusal(str(dest_dir), f"inside the game dir {pr}", "write the recipe to a private dir outside the game installs")
    dest = dest_dir / f"{derived.id}.toml"
    if dest.is_symlink():
        raise Refusal(str(dest), "is a symlink", "remove it by hand")
    if args.write != "" and dest.is_file() and dest.read_text(encoding="utf-8") != text and not args.force:
        raise Failure(str(dest), "exists and differs from the derived recipe", "pass --force to overwrite it, or pick another DIR")
    dest_dir.mkdir(parents=True, exist_ok=True)
    dest.write_text(text, encoding="utf-8")
    print(f"wrote {dest}", file=sys.stderr, flush=True)
    return EXIT_OK


def cmd_npcs(args) -> int:
    k1, _cfg = scan_context(args)
    got = scan_call(Runner(None), "npcs", "npcs", k1)
    rows = got.get("rows", [])
    if args.grep:
        rows = [r for r in rows if kscan.match_npc(r, args.grep)]
    total = len(rows)
    if args.limit and args.limit > 0:
        rows = rows[:args.limit]
    if args.json:
        print(json.dumps({"k1_dir": str(k1), "total": total, "rows": rows}, indent=2, sort_keys=True))
        return EXIT_OK
    for line in kscan.format_npcs(rows):
        say(line)
    say(f"{len(rows)} of {total} K1 creature template(s)" + (f" matching {args.grep!r}" if args.grep else "")
        + ("; --limit N / --grep PATTERN narrow it" if len(rows) < total or not args.grep else ""))
    return EXIT_OK


# ------------------------------------------------------------------------------------------- CLI

def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(description="K1 -> K2 character importer (doc 38 s26): doctor | build | install | remove | list | npcs | scan",
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
    k1opt = argparse.ArgumentParser(add_help=False)
    k1opt.add_argument("--k1-dir", default=None, metavar="DIR", help="K1 install (else the config's [k1] dir, env KOTOR1_ROOT, the Steam default)")
    p = sub.add_parser("npcs", parents=[common, k1opt], help="list the K1 creature templates (read only)")
    p.add_argument("--grep", default=None, metavar="PATTERN", help="case-insensitive; matches the UTC resref, tag or first name")
    p.add_argument("--limit", type=int, default=60, metavar="N", help="rows to print (default 60; 0 = all)")
    p.add_argument("--json", action="store_true")
    p.set_defaults(fn=cmd_npcs)
    p = sub.add_parser("scan", parents=[common, k1opt], help="derive a recipe from a K1 creature template (read only)")
    p.add_argument("utc", metavar="UTC_RESREF", help="a K1 creature template resref, e.g. p_carth (npcs lists them)")
    p.add_argument("--id", default=None, metavar="ID", help="recipe id (default: the resref without a leading p_)")
    p.add_argument("--write", nargs="?", const="", default=None, metavar="DIR",
                   help="also save <id>.toml (default DIR: <out>/recipes_auto from the config); give DIR after the options")
    p.add_argument("--force", action="store_true", help="with --write DIR: overwrite a different existing file")
    p.set_defaults(fn=cmd_scan)
    return ap


def main(argv: list | None = None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    if argv and argv[0] == "_twoda":  # hidden helper, runs under the k1port venv python
        return _twoda_main(argv[1:])
    if argv and argv[0] == "_audio":  # hidden helper (SSF / TLK reads), same interpreter
        return _audio_main(argv[1:])
    if argv and argv[0] == "_dialog":  # hidden helper (K1 DLG / TLK / lip reads, DLG writer), same interpreter
        return kdialog.helper_main(argv[1:])
    if argv and argv[0] == "_scan":  # hidden helper (K1 UTC / 2DA / MDL reads), same interpreter
        return kscan.helper_main(argv[1:])
    args = build_parser().parse_args(argv)
    try:
        return args.fn(args)
    except Failure as e:
        print(f"error: {e}", file=sys.stderr)
        return e.code
    except (krecipe.RecipeError, kresolve.ResolveError) as e:
        print(f"error: {e}", file=sys.stderr)
        return EXIT_FAIL
    except kscan.ScanError as e:
        print(f"error: {e}", file=sys.stderr)
        return EXIT_FAIL


if __name__ == "__main__":
    sys.exit(main())
