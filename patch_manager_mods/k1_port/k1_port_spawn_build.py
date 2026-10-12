#!/usr/bin/env python3
"""Build the T18-S probe spawn files from the live game. The game dir is only read.

Writes ONLY into --out (exactly two files):
  k1p_probe_npc.utc      copy of c_ithorian003.utc from modules/204tel.mod with appearance_id 812,
                         texture_variation 4, tag and template resref k1p_probe_npc, literal first name
                         'K1 Probe Citizen'. Every other field is kept from the template.
  k_def_heartbt01.ncs    WRAPPER_NSS compiled with the kit compile.sh (nwnnsscomp). The wrapper replaces the
                         engine call: on the 204_Ith_Rec heartbeat it spawns k1p_probe_npc once next to the PC,
                         then passes through to k_ai_master. No original k_def_heartbt01.ncs is copied.
Every check runs before the first write. A refusal raises BuildError (CLI: 'error: ...' on stderr, exit 1).
Row 812 of appearance.2da is NOT written here; the probe stage (k1_port_probe_build.py) provides it.

Needs pykotor. The CLI re-executes itself under the k1port venv (python -I) when pykotor is not
importable. build() imports pykotor lazily and raises BuildError when it is missing.

Usage: k1_port_spawn_build.py --game-dir DIR [--out DIR] [--json]
Exit: 0 ok, 1 refused or error. argparse usage errors exit 2.
"""
import argparse
import contextlib
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
import k1_port_probe_build as pb  # noqa: E402  (shared helpers; no pykotor import at module level)

BuildError = pb.BuildError  # one exception class for both builds
VENV_PY = pb.VENV_PY
DEFAULT_OUT = "~/Games/KOTOR2_Mod_Workspace/k1_port_stage/probe_spawn/override"
MODULE_REL = "modules/204tel.mod"
TEMPLATE_UTC = "c_ithorian003"
TEMPLATE_APPEARANCE = pb.PROBE_EXPECT  # 72: the live 204tel Greeter template
SPAWN_RESREF = "k1p_probe_npc"
SPAWN_NAME = "K1 Probe Citizen"
SPAWN_TEXTURE_VAR = 4
UTC_FILE = f"{SPAWN_RESREF}.utc"
NCS_FILE = "k_def_heartbt01.ncs"
NSS_FILE = "k_def_heartbt01.nss"
NCS_MAGIC = b"NCS V1.0B"
COMPILE_SH = Path("~/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/tools/compile.sh").expanduser()
COMPILE_TIMEOUT_S = 120

WRAPPER_NSS = """#include "k_inc_switch"
#include "k_inc_debug"
void main()
{
    object oSelf = OBJECT_SELF;
    if (GetTag(oSelf) == "204_Ith_Rec" && !GetIsObjectValid(GetObjectByTag("k1p_probe_npc")))
    {
        object oPC = GetFirstPC();
        if (GetIsObjectValid(oPC) && GetArea(oPC) == GetArea(oSelf))
        {
            vector vPos = GetPosition(oPC);
            float fFace = GetFacing(oPC);
            float fBack = fFace + 180.0;
            if (fBack >= 360.0) { fBack = fBack - 360.0; }
            vector vSpawn = Vector(vPos.x + 2.0 * cos(fFace), vPos.y + 2.0 * sin(fFace), vPos.z);
            CreateObject(OBJECT_TYPE_CREATURE, "k1p_probe_npc", Location(vSpawn, fBack));
        }
    }
    ExecuteScript("k_ai_master", OBJECT_SELF, KOTOR_DEFAULT_EVENT_ON_HEARTBEAT);
}
"""


def _reexec_under_venv(argv: list) -> None:
    """If pykotor is not importable, re-run this file under the k1port venv python -I. Does not return then."""
    try:
        import pykotor  # noqa: F401
        return
    except ImportError:
        pass
    venv_root = VENV_PY.parent.parent
    if Path(sys.prefix).resolve() == venv_root.resolve():
        raise BuildError(f"pykotor not importable inside the venv {venv_root}")
    if not VENV_PY.exists():
        raise BuildError(f"pykotor not importable and the venv python is missing: {VENV_PY}")
    os.execv(str(VENV_PY), [str(VENV_PY), "-I", str(Path(__file__).resolve()), *argv])


def _build_npc_utc(api, mod_live: bytes, module_rel: str) -> bytes:
    """Clone c_ithorian003.utc into the probe NPC. Refuses unless the template has appearance_id 72."""
    erf = api.read_erf(mod_live)
    hits = [r for r in erf if r.resref.get().lower() == TEMPLATE_UTC and r.restype == api.ResourceType.UTC]
    if len(hits) != 1:
        raise BuildError(f"expected one {TEMPLATE_UTC}.utc in {module_rel}, found {len(hits)}")
    u = api.read_utc(hits[0].data)
    if u.appearance_id != TEMPLATE_APPEARANCE:
        raise BuildError(f"{TEMPLATE_UTC}.utc appearance_id is {u.appearance_id}, "
                         f"expected {TEMPLATE_APPEARANCE}; refusing")
    from pykotor.common.language import LocalizedString  # literal text, no tlk entry
    from pykotor.common.misc import ResRef
    u.appearance_id = pb.NEW_ROW
    u.texture_variation = SPAWN_TEXTURE_VAR
    u.tag = SPAWN_RESREF
    u.resref = ResRef(SPAWN_RESREF)
    u.first_name = LocalizedString.from_english(SPAWN_NAME)
    return api.bytes_utc(u)


def _compile_wrapper() -> bytes:
    """Compile WRAPPER_NSS with the kit compile.sh. Fails closed; returns the .ncs bytes."""
    if not COMPILE_SH.is_file():
        raise BuildError(f"compile kit missing: {COMPILE_SH}; refusing (no .ncs can be built)")
    with tempfile.TemporaryDirectory(prefix="k1p_spawn_") as td:
        src = Path(td) / NSS_FILE
        ncs = Path(td) / NCS_FILE
        src.write_text(WRAPPER_NSS, encoding="ascii")
        try:
            proc = subprocess.run([str(COMPILE_SH), str(ncs), str(src)], capture_output=True,
                                  timeout=COMPILE_TIMEOUT_S)
        except (OSError, subprocess.TimeoutExpired) as exc:
            raise BuildError(f"compile.sh could not run: {exc}") from exc
        if proc.returncode != 0:
            tail = (proc.stderr or proc.stdout).decode("utf-8", "replace").strip().splitlines()[-3:]
            raise BuildError(f"compile.sh exited {proc.returncode}: {' | '.join(tail)}")
        data = ncs.read_bytes() if ncs.is_file() else b""
        if not data:
            raise BuildError("compile.sh produced no .ncs (missing or empty)")
        if not data.startswith(NCS_MAGIC):
            raise BuildError(f"compiled output does not start with {NCS_MAGIC!r}")
        return data


def _write_pair(out: Path, utc_bytes: bytes, ncs_bytes: bytes) -> None:
    """Write the two files. On a write error, remove what was written so nothing is half-written."""
    written = []
    try:
        out.mkdir(parents=True, exist_ok=True)
        for name, data in ((UTC_FILE, utc_bytes), (NCS_FILE, ncs_bytes)):
            p = out / name
            p.write_bytes(data)
            written.append(p)
    except OSError as exc:
        for p in written:
            with contextlib.suppress(OSError):
                p.unlink()
        raise BuildError(f"cannot write into {out}: {exc}") from exc


def build(game_dir, out_dir) -> dict:
    """Write <out_dir>/k1p_probe_npc.utc and <out_dir>/k_def_heartbt01.ncs from the live game (read only).

    Returns {"utc", "ncs", "base_sha256", "files"}. Raises BuildError; writes nothing on refusal or compile failure.
    """
    if not pb.RESREF_RE.fullmatch(SPAWN_RESREF):
        raise BuildError(f"spawn resref must be 1-16 chars of [a-z0-9_], got {SPAWN_RESREF!r}")
    game, out = pb._resolve_dirs(game_dir, out_dir)
    api = pb._pykotor()
    _, mod_live = pb._read_live(game, MODULE_REL)
    utc_bytes = _build_npc_utc(api, mod_live, MODULE_REL)
    ncs_bytes = _compile_wrapper()
    _write_pair(out, utc_bytes, ncs_bytes)
    return {
        "utc": pb.sha256_hex(utc_bytes),
        "ncs": pb.sha256_hex(ncs_bytes),
        "base_sha256": pb.sha256_hex(mod_live),
        "files": [UTC_FILE, NCS_FILE],
    }


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Build the probe spawn NPC UTC and the heartbeat wrapper .ncs "
                                             "from the live game (game dir is read only).")
    ap.add_argument("--game-dir", required=True, metavar="DIR", help="KOTOR II install dir (read only)")
    ap.add_argument("--out", default=DEFAULT_OUT, metavar="DIR", help="output dir (default: %(default)s)")
    ap.add_argument("--json", action="store_true", help="print the result as JSON")
    args = ap.parse_args(argv)
    try:
        _reexec_under_venv(sys.argv[1:] if argv is None else list(argv))
        res = build(args.game_dir, args.out)
    except (BuildError, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    if args.json:
        print(json.dumps(res, indent=2, sort_keys=True))
    else:
        print(f"utc: {res['utc']}")
        print(f"ncs: {res['ncs']}")
        print(f"base_sha256: {res['base_sha256']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
