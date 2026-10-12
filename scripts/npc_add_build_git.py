#!/usr/bin/env python3
"""Build out/modules/204tel.mod for the npc_add canary (task B2).

Reads the live 204tel.mod (read only), appends K creature structs (GIT 'Creature List', struct 4) and the R1
route waypoints (GIT 'WaypointList', struct 5, from scripts/npc_add_waypoints.py), embeds the matching
npcadd_<slug>.utc resources (from out/override/, written by npc_add_build_utc.py) and the route's UTW resources,
and writes only the out path. GFF-level append only: no construct_git / dismantle_git (those are lossy).

Canary: Wookiee M (npcadd_wookm), Twi'lek F (npcadd_twilekf), Bith (npcadd_bith) and the Rodian (npcadd_rodian),
which walks route R1 from its first point. Creature orientation is the unit vector toward the ward bed centroid
(-63.35, 27.00). Expected result: Creature List 17 -> 21, WaypointList 48 -> 51, resources 186 -> 193 (186 kept,
4 UTC and 3 UTW added), only the GIT resource changed among the originals.

Usage:
  python3 scripts/npc_add_build_git.py [--live-mod <204tel.mod>] [--out <out/modules/204tel.mod>]
                                       [--override-dir <out/override>] [--check]

  --check   build in memory and compare with --out. Writes nothing. Exit 1 on a missing or changed file.
  Exit codes: 0 ok, 1 drift (--check only), 2 bad input, guardrail or verification failure.
  The output path may not sit under the game directory. The builder never writes there.

Build is deterministic: same live mod + same UTC blobs -> same bytes (sha256 printed).
pykotor is required. The script re-runs itself under the repo .venv when the system python lacks it.
"""
import argparse
import hashlib
import math
import os
import struct
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent

try:
    import pykotor  # noqa: F401
except ImportError:  # the system python has no pykotor: re-run under the repo venv
    _venv = REPO / ".venv" / "bin" / "python3"
    if _venv.exists() and Path(sys.prefix).resolve() != _venv.parent.parent.resolve():
        os.execv(str(_venv), [str(_venv), __file__] + sys.argv[1:])
    sys.exit("npc_add_build_git: pykotor not importable (run with the repo .venv/bin/python3)")

sys.path.insert(0, str(HERE))
import _common  # noqa: E402  (GAME_DIR, honours KOTOR2_ROOT)
import npc_add_waypoints as wp  # noqa: E402
from pykotor.common.language import LocalizedString  # noqa: E402
from pykotor.common.misc import ResRef  # noqa: E402
from pykotor.resource.formats.erf import ERFType, bytes_erf, read_erf  # noqa: E402
from pykotor.resource.formats.gff import bytes_gff, read_gff  # noqa: E402
from pykotor.resource.formats.gff.gff_data import GFFList, GFFStruct  # noqa: E402
from pykotor.resource.type import ResourceType  # noqa: E402

LIVE_MOD = _common.GAME_DIR / "Modules" / "204tel.mod"
OUT_MOD = REPO / "patch_manager_mods" / "npc_add_src" / "out" / "modules" / "204tel.mod"
OVERRIDE_DIR = REPO / "patch_manager_mods" / "npc_add_src" / "out" / "override"

MOD_RESREF = "204tel"
CREATURE_LIST = "Creature List"
WAYPOINT_LIST = "WaypointList"
CREATURE_STRUCT_ID = 4
WAYPOINT_STRUCT_ID = 5
CREATURE_FIELDS = (
    ("XPosition", "Single"),
    ("YPosition", "Single"),
    ("ZPosition", "Single"),
    ("XOrientation", "Single"),
    ("YOrientation", "Single"),
    ("TemplateResRef", "ResRef"),
)
BASELINE_CREATURES = 17
BASELINE_WAYPOINTS = 48
TARGET_XY = (-63.35, 27.00)  # ward bed centroid; creatures face it

ROUTE = wp.ROUTES["R1"]
WALKER_TAG = "npcadd_rodian"
# (creature template ResRef, (x, y)); the walker starts on route point 1. z is 0.0.
CANARIES = (
    ("npcadd_wookm", (-76.95, 27.80)),
    ("npcadd_twilekf", (-73.45, 31.55)),
    ("npcadd_bith", (-68.70, 22.30)),
    (WALKER_TAG, ROUTE[0]),
)


class BuildError(Exception):
    """Bad input, guardrail failure or verification failure. The CLI exits 2."""


def _f32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", float(value)))[0]


def facing_toward_target(x: float, y: float) -> tuple[float, float]:
    """Unit vector from (x, y) toward TARGET_XY (XOrientation, YOrientation)."""
    dx, dy = TARGET_XY[0] - x, TARGET_XY[1] - y
    length = math.hypot(dx, dy)
    if length == 0.0:
        raise BuildError(f"creature at {x}, {y} sits on the target point; no facing")
    return dx / length, dy / length


def _read_mod(src: bytes):
    try:
        erf = read_erf(src)
    except Exception as e:  # noqa: BLE001  (any parse failure is bad input)
        raise BuildError(f"not a readable ERF/MOD: {e}") from e
    if erf.erf_type != ERFType.MOD:
        raise BuildError(f"expected a MOD archive, got {erf.erf_type}")
    return erf


def _git_root(erf):
    raw = erf.get(MOD_RESREF, ResourceType.GIT)
    if raw is None:
        raise BuildError(f"{MOD_RESREF} GIT not in archive")
    g = read_gff(raw)
    for label in (CREATURE_LIST, WAYPOINT_LIST):
        if g.root.get_list(label) is None:
            raise BuildError(f"GIT has no '{label}' list")
    return g


def load_utc_blobs(override_dir: Path, canaries=CANARIES) -> dict[str, bytes]:
    """Read out/override/<name>.utc for each canary. Fails closed on a missing or non-UTC file."""
    blobs = {}
    for name, _ in canaries:
        path = override_dir / f"{name}.utc"
        if not path.is_file():
            raise BuildError(f"missing UTC for {name}: {path} (run scripts/npc_add_build_utc.py first)")
        data = path.read_bytes()
        if data[:4] != b"UTC ":
            raise BuildError(f"{path} is not a UTC resource (header {data[:4]!r})")
        blobs[name] = data
    return blobs


def _put(s: GFFStruct, label: str, typ: str, value) -> None:
    if typ == "Single":
        s.set_single(label, float(value))
    elif typ == "UInt8":
        s.set_uint8(label, int(value))
    elif typ == "String":
        s.set_string(label, str(value))
    elif typ == "ResRef":
        s.set_resref(label, ResRef(str(value)))
    elif typ == "LocString":
        s.set_locstring(label, LocalizedString(int(value)))
    else:
        raise BuildError(f"no writer for GFF type {typ} ({label})")


def _creature_values(x: float, y: float, template: str) -> dict:
    ux, uy = facing_toward_target(x, y)
    return {"XPosition": x, "YPosition": y, "ZPosition": 0.0,
            "XOrientation": ux, "YOrientation": uy, "TemplateResRef": template}


def build_mod(src: bytes, utc_blobs: dict[str, bytes], *, canaries=CANARIES, route=ROUTE,
              expect_creatures=BASELINE_CREATURES, expect_waypoints=BASELINE_WAYPOINTS) -> bytes:
    """Return the new 204tel.mod bytes. Pure: no file I/O. Raises BuildError on any guardrail failure."""
    erf = _read_mod(src)
    g = _git_root(erf)
    root = g.root
    creatures = root.get_list(CREATURE_LIST)
    waypoints = root.get_list(WAYPOINT_LIST)
    if len(creatures) != expect_creatures or len(waypoints) != expect_waypoints:
        raise BuildError(f"baseline mismatch: {len(creatures)} creatures / {len(waypoints)} waypoints, "
                         f"expected {expect_creatures} / {expect_waypoints}")
    walker = [(xy) for name, xy in canaries if name == WALKER_TAG]
    if not walker or tuple(walker[0]) != tuple(route[0]):
        raise BuildError(f"{WALKER_TAG} must be canary at route point 1 {route[0]}")
    for name, _ in canaries:
        if erf.get(name, ResourceType.UTC) is not None:
            raise BuildError(f"archive already holds UTC {name}; name collision")
        if name not in utc_blobs:
            raise BuildError(f"missing UTC blob for {name} (fail closed)")
        if utc_blobs[name][:4] != b"UTC ":
            raise BuildError(f"UTC blob for {name} has a bad header")
    existing_tags = [str(s.get_string("Tag") or "") for s in waypoints]
    existing_resrefs = [str(s.get_resref("TemplateResRef") or "") for s in waypoints]
    existing_resrefs += [str(r.resref) for r in erf if r.restype == ResourceType.UTW]
    try:
        utw_files, wp_structs = wp.build_route(WALKER_TAG, list(route),
                                               existing_tags=existing_tags, existing_resrefs=existing_resrefs)
    except ValueError as e:
        raise BuildError(str(e)) from e

    for name, (x, y) in canaries:
        vals = _creature_values(x, y, name)
        s = creatures.add(CREATURE_STRUCT_ID)
        for label, typ in CREATURE_FIELDS:
            _put(s, label, typ, vals[label])
    root.set_list(CREATURE_LIST, creatures)

    for fields in wp_structs:
        s = waypoints.add(WAYPOINT_STRUCT_ID)
        for label, typ in wp.WAYPOINT_FIELDS:
            _put(s, label, typ, fields[label])
    root.set_list(WAYPOINT_LIST, waypoints)

    erf.set_data(MOD_RESREF, ResourceType.GIT, bytes_gff(g))
    for name, _ in canaries:
        erf.set_data(name, ResourceType.UTC, utc_blobs[name])
    for resref, data in utw_files.items():
        erf.set_data(resref, ResourceType.UTW, data)
    return bytes_erf(erf, ResourceType.MOD)


def list_signature(value):
    """Order-preserving, type-aware snapshot of a GFF value, for bit-exact comparison."""
    if isinstance(value, GFFStruct):
        return ("S", value.struct_id, tuple((label, int(ftype), list_signature(v)) for label, ftype, v in value))
    if isinstance(value, GFFList):
        return ("L", tuple(list_signature(s) for s in value))
    if isinstance(value, LocalizedString):
        return ("LS", tuple(sorted(vars(value).items())))
    if isinstance(value, float):
        return ("F", repr(value))
    return ("V", type(value).__name__, value)


def _norm(typ: str, value):
    if typ == "Single":
        return _f32(value)
    if typ == "LocString":
        return value.stringref if isinstance(value, LocalizedString) else int(value)
    if typ in ("String", "ResRef"):
        return str(value)
    return value


def verify(src: bytes, out: bytes, utc_blobs: dict[str, bytes], *, canaries=CANARIES, route=ROUTE,
           expect_creatures=BASELINE_CREATURES, expect_waypoints=BASELINE_WAYPOINTS) -> list[str]:
    """Re-read both archives and check the contract. Returns problem lines; empty means it passes."""
    problems: list[str] = []
    try:
        src_erf = _read_mod(src)
        out_erf = _read_mod(out)
    except BuildError as e:
        return [str(e)]
    n_new_res = len(canaries) + len(route)
    if len(out_erf) != len(src_erf) + n_new_res:
        problems.append(f"resource count {len(out_erf)}, expected {len(src_erf) + n_new_res}")

    orig = {(str(r.resref).lower(), r.restype): r.data for r in src_erf}
    got = {(str(r.resref).lower(), r.restype): r.data for r in out_erf}
    git_key = (MOD_RESREF, ResourceType.GIT)
    for key, data in orig.items():
        if key not in got:
            problems.append(f"original resource lost: {key[0]}.{key[1].extension}")
        elif key != git_key and got[key] != data:
            problems.append(f"original resource changed: {key[0]}.{key[1].extension}")

    for name, _ in canaries:
        if got.get((name, ResourceType.UTC)) != utc_blobs[name]:
            problems.append(f"UTC {name} missing or differs from out/override")

    utw_expected, wp_expected = wp.build_route(WALKER_TAG, list(route))
    for resref, data in utw_expected.items():
        if got.get((resref.lower(), ResourceType.UTW)) != data:
            problems.append(f"UTW {resref} missing or differs")

    try:
        src_root = _git_root(src_erf).root
        out_root = _git_root(out_erf).root
    except BuildError as e:
        return problems + [str(e)]

    skip = (CREATURE_LIST, WAYPOINT_LIST)
    if tuple(l for l in list_signature(src_root)[2] if l[0] not in skip) != \
            tuple(l for l in list_signature(out_root)[2] if l[0] not in skip):
        problems.append("GIT root fields other than Creature List / WaypointList changed")

    src_c, out_c = src_root.get_list(CREATURE_LIST), out_root.get_list(CREATURE_LIST)
    want_c = expect_creatures + len(canaries)
    if len(out_c) != want_c:
        problems.append(f"Creature List {len(out_c)}, expected {want_c}")
    else:
        if [list_signature(s) for s in src_c] != [list_signature(s) for s in out_c][:expect_creatures]:
            problems.append("original creature structs changed")
        for idx, (name, (x, y)) in enumerate(canaries):
            s = out_c.at(expect_creatures + idx)
            want = _creature_values(x, y, name)
            if s.struct_id != CREATURE_STRUCT_ID or [lab for lab, _ in CREATURE_FIELDS] != [l for l, _, _ in s]:
                problems.append(f"creature {name}: bad struct id or field order")
                continue
            for label, typ in CREATURE_FIELDS:
                got_v = s.get(label)
                if _norm(typ, got_v) != _norm(typ, want[label]):
                    problems.append(f"creature {name}: {label} {got_v!r} != {want[label]!r}")

    src_w, out_w = src_root.get_list(WAYPOINT_LIST), out_root.get_list(WAYPOINT_LIST)
    want_w = expect_waypoints + len(route)
    if len(out_w) != want_w:
        problems.append(f"WaypointList {len(out_w)}, expected {want_w}")
    else:
        if [list_signature(s) for s in src_w] != [list_signature(s) for s in out_w][:expect_waypoints]:
            problems.append("original waypoint structs changed")
        for idx, fields in enumerate(wp_expected):
            s = out_w.at(expect_waypoints + idx)
            if s.struct_id != WAYPOINT_STRUCT_ID or [lab for lab, _ in wp.WAYPOINT_FIELDS] != [l for l, _, _ in s]:
                problems.append(f"waypoint {idx + 1}: bad struct id or field order")
                continue
            for label, typ in wp.WAYPOINT_FIELDS:
                if _norm(typ, s.get(label)) != _norm(typ, fields[label]):
                    problems.append(f"waypoint {idx + 1}: {label} differs")

    return problems


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def check_drift(out_path: Path, expected: bytes) -> list[str]:
    if not out_path.is_file():
        return [f"missing {out_path}"]
    have = out_path.read_bytes()
    if have != expected:
        return [f"changed {out_path} (have {sha256(have)[:12]}, want {sha256(expected)[:12]})"]
    return []


def guard_out(out_path: Path) -> None:
    game = _common.GAME_DIR.resolve()
    if out_path.resolve().is_relative_to(game):
        raise BuildError(f"refusing to write under the game directory: {out_path}")


def write_out(out_path: Path, data: bytes) -> None:
    guard_out(out_path)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    tmp = out_path.with_name(out_path.name + ".tmp")
    tmp.write_bytes(data)
    os.replace(tmp, out_path)


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Build out/modules/204tel.mod: canary creatures, R1 waypoints, UTC/UTW.")
    ap.add_argument("--live-mod", type=Path, default=LIVE_MOD, help="live 204tel.mod (read only)")
    ap.add_argument("--out", type=Path, default=OUT_MOD, help="output 204tel.mod (never under the game dir)")
    ap.add_argument("--override-dir", type=Path, default=OVERRIDE_DIR, help="npcadd_*.utc source (B1 output)")
    ap.add_argument("--check", action="store_true", help="compare only; exit 1 on drift")
    args = ap.parse_args(argv)
    try:
        if not args.live_mod.is_file():
            raise BuildError(f"live mod not found: {args.live_mod} (pass --live-mod)")
        src = args.live_mod.read_bytes()
        utc_blobs = load_utc_blobs(args.override_dir)
        expected = build_mod(src, utc_blobs)
        problems = verify(src, expected, utc_blobs)
        if problems:
            raise BuildError("verification failed: " + "; ".join(problems))
        if args.check:
            drift = check_drift(args.out, expected)
            for line in drift:
                print(f"DRIFT: {line}")
            if drift:
                print(f"rerun without --check to rewrite {args.out}", file=sys.stderr)
                return 1
            print(f"OK: {args.out} matches the build ({sha256(expected)})")
            return 0
        write_out(args.out, expected)
        written = args.out.read_bytes()
        problems = verify(src, written, utc_blobs)
        if problems or written != expected:
            raise BuildError("post-write verification failed: " + "; ".join(problems or ["bytes differ"]))
    except BuildError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 2
    erf = read_erf(written)
    git = _git_root(erf).root
    print(f"wrote {args.out}")
    print(f"sha256 {sha256(written)} {len(written)} bytes")
    print(f"Creature List {BASELINE_CREATURES} -> {len(git.get_list(CREATURE_LIST))}")
    print(f"WaypointList {BASELINE_WAYPOINTS} -> {len(git.get_list(WAYPOINT_LIST))}")
    print(f"resources {len(read_erf(src))} -> {len(erf)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
