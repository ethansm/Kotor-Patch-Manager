#!/usr/bin/env python3
"""Build the NPC-add species UTC templates (task B1).

Clones vanilla n_telf01 (read only, from 202tel.mod) once per [[species]] row of
patch_manager_mods/npc_add_src/species.toml and writes out/override/npcadd_<slug>.utc.
The clone keeps the vanilla commoner body (FactionID 5, Plot/Min1HP/NoPermDeath 0, IsPC 0,
Interruptable 1, Disarmable 0, classes, feats, skills, HP 8, equipment, WalkRate 7,
PerceptionRange 11). Rows override only identity, appearance, sound set, conversation and
the k_def_* spawn/heartbeat scripts. Output is byte-stable: same template + table -> same bytes.

Usage:
  python3 scripts/npc_add_build_utc.py --vanilla-mod <202tel.mod> [--check]

  --check   build in memory and compare with --out-dir. Writes nothing. Exit 1 on a missing,
            changed or stale npcadd_*.utc.
  Exit codes: 0 ok, 1 drift (--check only), 2 bad input or guardrail/name-collision failure.

Name collision scan (read only): chitin.key UTC entries, Override/*.utc, and UTC entries of every
Modules/*.mod|*.rim|*.erf archive under --game-dir (about 40 s on the Steam install). Skipped with a
warning when --game-dir is absent.

pykotor is required. The script re-runs itself under the repo .venv when the system python lacks it.
"""
import argparse
import hashlib
import os
import re
import sys
import tomllib
from dataclasses import dataclass
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent

try:
    import pykotor  # noqa: F401
except ImportError:  # the system python has no pykotor: re-run under the repo venv
    _venv = REPO / ".venv" / "bin" / "python3"
    if _venv.exists() and Path(sys.prefix).resolve() != _venv.parent.parent.resolve():
        os.execv(str(_venv), [str(_venv), __file__] + sys.argv[1:])
    sys.exit("npc_add_build_utc: pykotor not importable (run with the repo .venv/bin/python3)")

sys.path.insert(0, str(HERE))
import _common  # noqa: E402  (GAME_DIR, honours KOTOR2_ROOT)
from pykotor.common.language import Gender, Language, LocalizedString  # noqa: E402
from pykotor.common.misc import ResRef  # noqa: E402
from pykotor.resource.formats.erf import read_erf  # noqa: E402
from pykotor.resource.formats.key import read_key  # noqa: E402
from pykotor.resource.formats.rim import read_rim  # noqa: E402
from pykotor.resource.generics.utc import bytes_utc, read_utc  # noqa: E402
from pykotor.resource.type import ResourceType  # noqa: E402

TABLE = REPO / "patch_manager_mods" / "npc_add_src" / "species.toml"
OUT_DIR = REPO / "patch_manager_mods" / "npc_add_src" / "out" / "override"
DEFAULT_VANILLA_MOD = _common.GAME_DIR / "Modules" / "202tel.mod"

VANILLA_RESREF = "n_telf01"
PREFIX = "npcadd_"
MAX_RESREF = 16
FACTION_NEUTRAL = 5
SOUNDSET = 35
FIRST_NAME = "Telos Resident"
SLUG_RE = re.compile(r"^[a-z][a-z0-9]*$")
BEHAVIOURS = {  # behaviour -> (ScriptSpawn, ScriptHeartbeat); "" = blank
    "stand": ("", ""),
    "walk": ("k_def_spawn01", ""),
    "idle": ("k_def_ambient", "k_def_heartbt01"),
}
SCRIPT_FIELDS = ("on_spawn", "on_heartbeat", "on_notice", "on_spell", "on_attacked", "on_damaged",
                 "on_disturbed", "on_end_round", "on_dialog", "on_end_dialog", "on_death",
                 "on_user_defined", "on_blocked", "on_rested")


class BuildError(Exception):
    """Bad table, bad template, guardrail failure or name collision. CLI exit 2."""


@dataclass(frozen=True)
class Row:
    slug: str
    appearance: int
    race: int
    gender: int
    behaviour: str
    canary: bool = False
    p2_line: int = 0

    @property
    def resref(self) -> str:
        return PREFIX + self.slug


def validate_row(row: Row) -> None:
    if not SLUG_RE.match(row.slug):
        raise BuildError(f"slug {row.slug!r}: must match {SLUG_RE.pattern}")
    if len(row.resref) > MAX_RESREF:
        raise BuildError(f"{row.resref}: {len(row.resref)} chars, max {MAX_RESREF}")
    if row.behaviour not in BEHAVIOURS:
        raise BuildError(f"{row.resref}: behaviour {row.behaviour!r} not in {sorted(BEHAVIOURS)}")
    if row.gender not in (0, 1):
        raise BuildError(f"{row.resref}: gender {row.gender} must be 0 or 1")
    if not (0 <= row.race <= 255):
        raise BuildError(f"{row.resref}: race {row.race} out of byte range")
    if not (0 <= row.appearance <= 65535):
        raise BuildError(f"{row.resref}: appearance {row.appearance} out of word range")


def load_rows(path: Path = TABLE) -> list[Row]:
    try:
        data = tomllib.loads(path.read_text(encoding="utf-8"))
    except (OSError, tomllib.TOMLDecodeError) as e:
        raise BuildError(f"cannot read species table {path}: {e}") from e
    rows = []
    for raw in data.get("species", []):
        try:
            row = Row(slug=str(raw["slug"]), appearance=int(raw["appearance"]), race=int(raw["race"]),
                      gender=int(raw["gender"]), behaviour=str(raw["behaviour"]),
                      canary=bool(raw.get("canary", False)), p2_line=int(raw.get("p2_line", 0)))
        except KeyError as e:
            raise BuildError(f"species row missing key {e}") from e
        validate_row(row)
        rows.append(row)
    if not rows:
        raise BuildError(f"no [[species]] rows in {path}")
    seen = set()
    for row in rows:
        if row.slug in seen:
            raise BuildError(f"duplicate slug {row.slug!r}")
        seen.add(row.slug)
    return rows


def _open_archive(path: Path):
    return read_rim(path) if path.suffix.lower() == ".rim" else read_erf(path)


def read_template(vanilla_mod: Path) -> bytes:
    """Vanilla n_telf01 UTC bytes from the module archive (read only)."""
    if not vanilla_mod.is_file():
        raise BuildError(f"vanilla template archive not found: {vanilla_mod} (pass --vanilla-mod)")
    data = _open_archive(vanilla_mod).get(VANILLA_RESREF, ResourceType.UTC)
    if not data:
        raise BuildError(f"{VANILLA_RESREF} UTC not in {vanilla_mod}")
    return bytes(data)


def taken_utc_names(game_dir: Path) -> set[str]:
    """Lowercase UTC resrefs already in chitin.key, Override/ or any Modules/ archive. Read only."""
    taken: set[str] = set()
    key = game_dir / "chitin.key"
    if key.is_file():
        taken |= {e.resref.get().lower() for e in read_key(key).key_entries if e.restype == ResourceType.UTC}
    override = game_dir / "Override"
    if override.is_dir():
        taken |= {p.stem.lower() for p in override.glob("*.utc")}
    modules = game_dir / "Modules"
    if modules.is_dir():
        for arc in sorted(modules.iterdir()):
            if arc.suffix.lower() not in (".mod", ".rim", ".erf"):
                continue
            try:
                archive = _open_archive(arc)
            except Exception as e:  # an unreadable archive could hide a collision: fail closed
                raise BuildError(f"cannot read {arc.name} for the name scan: {e}") from e
            taken |= {r.resref.get().lower() for r in archive if r.restype == ResourceType.UTC}
    return taken


def build_utc(template: bytes, row: Row) -> bytes:
    """Clone the template, apply the row overrides, verify the written bytes against the guardrails."""
    utc = read_utc(template)
    utc.resref = ResRef(row.resref)
    utc.tag = row.resref
    first = LocalizedString.from_english(FIRST_NAME)
    first.set_data(Language.ENGLISH, Gender.FEMALE, FIRST_NAME)  # female-gendered rows keep the name too
    utc.first_name = first
    utc.race_id = row.race
    utc.gender_id = row.gender
    utc.appearance_id = row.appearance
    utc.soundset_id = SOUNDSET
    utc.conversation = ResRef.from_blank()
    spawn, heartbeat = BEHAVIOURS[row.behaviour]
    utc.on_spawn = ResRef(spawn) if spawn else ResRef.from_blank()
    utc.on_heartbeat = ResRef(heartbeat) if heartbeat else ResRef.from_blank()
    data = bytes_utc(utc)
    problems = utc_problems(read_utc(data), row)
    if problems:
        raise BuildError(f"{row.resref}: " + "; ".join(problems))
    return data


def utc_problems(utc, row: Row) -> list[str]:
    """Guardrail failures for a built UTC. Empty list = clean."""
    bad: list[str] = []

    def need(ok: bool, msg: str) -> None:
        if not ok:
            bad.append(msg)

    need(utc.resref.get() == row.resref, f"TemplateResRef is {utc.resref.get()!r}")
    need(utc.tag == row.resref, f"Tag is {utc.tag!r}")
    need(utc.resref.get() != VANILLA_RESREF, "reuses vanilla n_telf01")
    need(not utc.plot, "Plot=1")
    need(not utc.min1_hp, "Min1HP=1")
    need(not utc.no_perm_death, "NoPermDeath=1")
    need(utc.faction_id == FACTION_NEUTRAL, f"FactionID {utc.faction_id} != {FACTION_NEUTRAL}")
    need(not utc.is_pc, "IsPC=1")
    need(utc.interruptable, "Interruptable=0")
    need(not utc.disarmable, "Disarmable=1")
    need(utc.conversation.get() == "", f"Conversation {utc.conversation.get()!r} not blank")
    need(utc.soundset_id == SOUNDSET, f"SoundSetFile {utc.soundset_id} != {SOUNDSET}")
    need(utc.race_id == row.race, f"Race {utc.race_id} != {row.race}")
    need(utc.gender_id == row.gender, f"Gender {utc.gender_id} != {row.gender}")
    need(utc.appearance_id == row.appearance, f"Appearance_Type {utc.appearance_id} != {row.appearance}")
    need(utc.first_name.stringref == -1 and utc.first_name.get(Language.ENGLISH, Gender.MALE) == FIRST_NAME,
         "FirstName is not the inline 'Telos Resident'")
    for field in SCRIPT_FIELDS:
        name = getattr(utc, field).get()
        need(name == "" or name.startswith("k_def_"), f"{field}={name!r} is not a chitin k_def_* script")
    spawn, heartbeat = BEHAVIOURS[row.behaviour]
    need(utc.on_spawn.get() == spawn, f"ScriptSpawn {utc.on_spawn.get()!r} != {spawn!r}")
    need(utc.on_heartbeat.get() == heartbeat, f"ScriptHeartbeat {utc.on_heartbeat.get()!r} != {heartbeat!r}")
    return bad


def build_all(template: bytes, rows: list[Row], taken: frozenset | set = frozenset()) -> dict[str, bytes]:
    """File name (npcadd_<slug>.utc) -> UTC bytes, in table order."""
    blobs: dict[str, bytes] = {}
    for row in rows:
        if row.resref in taken:
            raise BuildError(f"{row.resref}: name already used by chitin/Override/Modules")
        blobs[f"{row.resref}.utc"] = build_utc(template, row)
    return blobs


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def check_blobs(out_dir: Path, blobs: dict[str, bytes]) -> list[str]:
    """Drift lines: missing, changed or stale npcadd_*.utc under out_dir. Empty = in sync."""
    drift = []
    for name, data in sorted(blobs.items()):
        path = out_dir / name
        if not path.is_file():
            drift.append(f"missing {name}")
        elif path.read_bytes() != data:
            drift.append(f"changed {name} (have {sha256(path.read_bytes())[:12]}, want {sha256(data)[:12]})")
    if out_dir.is_dir():
        for path in sorted(out_dir.glob(f"{PREFIX}*.utc")):
            if path.name not in blobs:
                drift.append(f"stale {path.name}")
    return drift


def write_blobs(out_dir: Path, blobs: dict[str, bytes]) -> list[str]:
    """Write the mirror and delete stale npcadd_*.utc. Returns removed names."""
    out_dir.mkdir(parents=True, exist_ok=True)
    removed = []
    for path in sorted(out_dir.glob(f"{PREFIX}*.utc")):
        if path.name not in blobs:
            path.unlink()
            removed.append(path.name)
    for name, data in blobs.items():
        (out_dir / name).write_bytes(data)
    return removed


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Build out/override/npcadd_<slug>.utc from vanilla n_telf01.")
    ap.add_argument("--vanilla-mod", type=Path, default=DEFAULT_VANILLA_MOD,
                    help="module archive holding n_telf01 (read only)")
    ap.add_argument("--table", type=Path, default=TABLE, help="species table (TOML)")
    ap.add_argument("--out-dir", type=Path, default=OUT_DIR, help="mirror directory")
    ap.add_argument("--game-dir", type=Path, default=_common.GAME_DIR, help="game dir for the name scan")
    ap.add_argument("--check", action="store_true", help="compare only; exit 1 on drift")
    args = ap.parse_args(argv)
    try:
        rows = load_rows(args.table)
        template = read_template(args.vanilla_mod)
        if args.game_dir.is_dir():
            taken = taken_utc_names(args.game_dir)
        else:
            taken = set()
            print(f"WARN: game dir {args.game_dir} absent; name collision scan skipped", file=sys.stderr)
        blobs = build_all(template, rows, taken)
    except BuildError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 2
    if args.check:
        drift = check_blobs(args.out_dir, blobs)
        for line in drift:
            print(f"DRIFT: {line}")
        if drift:
            print(f"{len(drift)} drift(s) in {args.out_dir}; rerun without --check to rewrite", file=sys.stderr)
            return 1
        print(f"OK: {len(blobs)} files match {args.out_dir}")
        return 0
    for name in write_blobs(args.out_dir, blobs):
        print(f"removed stale {name}")
    for name, data in blobs.items():
        print(f"{name} {sha256(data)} {len(data)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
