#!/usr/bin/env python3
"""Find compiled scripts (.ncs) whose engine calls get their arguments swapped.

NWScript pushes a call's arguments last-first, so the first argument is on top of
the stack when the engine function runs. BioWare's compiler and nwnnsscomp do this.
pykotor 2.3.12's built-in NSS compiler pushes them in source order instead, so
SetGlobalNumber("X", 1) reaches the engine with the name and value swapped. That
compiler runs whenever a TSLPatcher [CompileList] script isn't compiled with
nwnnsscomp (see pykotor_patches/README.md, patch #1). It is also what the Echoes
compatch's phase 3 used without KOTOR2_NWNNSSCOMP.

The check covers every engine call with two or more arguments. The argument
count includes the defaults the compiler fills in. The first argument is pushed
last, so it's the one pushed by the instruction right before the ACTION. When that
instruction pushes a constant (CONSTI/NEGI int, CONSTF/NEGF float, CONSTS string,
CONSTO object) whose type isn't the first parameter's but is a later parameter's,
the arguments were pushed in source order. A matching type counts as a normal call.
Any other instruction (a variable, an expression) tells us nothing and is skipped.

A call whose first parameter is a vector is skipped too: a vector is pushed as
three floats. TABLE_ERRORS lists functions whose pykotor definition disagrees with
nwscript.nss. They're skipped. Calibration, 2026-09-24: every .ncs BioWare compiled
(612 in the chitin.key BIFs, 4,510 in the 164 Modules/*.rim files) gives 0 flags
across 66,135 checked calls to 133 functions
(script_recompile_20260924/tools/calibrate_arg_order.py). Without the vector
skip, 3 Location() calls were flagged; without TABLE_ERRORS, 11 PlayPazaak calls.

Scans override/ and every .mod/.erf in Modules/ (the .rim files are the game's own,
compiled by BioWare, and mods don't write to them). Exits 1 if any script is
flagged. Read-only. To fix a flagged script, recompile its source with
nwnnsscomp. For a [CompileList] script, first put back the #2DAMEMORY#
values the install used.

Usage: check_ncs_arg_order.py [game_dir] [--override-only]
"""
import argparse
import logging
import os
import sys
from pathlib import Path

logging.disable(logging.CRITICAL)

from pykotor.common.scriptdefs import TSL_FUNCTIONS  # noqa: E402
from pykotor.resource.formats.erf import read_erf  # noqa: E402
from pykotor.resource.formats.ncs import read_ncs  # noqa: E402

KOTOR2_ROOT = os.environ.get(
    "KOTOR2_ROOT", os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"))

# Functions pykotor's table gets wrong, so the rule would misfire on correct code.
# PlayPazaak: pykotor lacks the first parameter, int nOpponentPazaakDeck; 11 of its
# calls in BioWare's scripts would be flagged.
# SetAvailableNPCId: nwscript.nss declares it (INT nNPC, OBJECT_ID oidNPC), types no
# compiler knows; pykotor reads OBJECT_ID as object.
TABLE_ERRORS = {"PlayPazaak", "SetAvailableNPCId"}

PUSH_TYPES = {"CONSTI": "int", "NEGI": "int", "CONSTF": "float", "NEGF": "float",
              "CONSTS": "string", "CONSTO": "object"}


def _param_types(fid, argc):
    """(first parameter's type, later pushed parameters' types) for an engine call,
    or None if the call can't be checked."""
    if fid >= len(TSL_FUNCTIONS) or argc < 2:
        return None
    f = TSL_FUNCTIONS[fid]
    if f.name in TABLE_ERRORS or not f.params:
        return None
    # A vector is pushed as three floats, so a CONSTF before the call can't tell a
    # first vector argument (e.g. Location(vector, float)) from a later float.
    types = ["float" if p.datatype.value == "vector" else p.datatype.value for p in f.params[:argc]]
    if f.params[0].datatype.value == "vector":
        return None
    return types[0], set(types[1:])


def classify(ins):
    """Yield (function name, 'swapped' | 'normal') for each checkable engine call."""
    for i in range(1, len(ins)):
        x = ins[i]
        if x.ins_type.name != "ACTION" or len(x.args) < 2:
            continue
        pushed = PUSH_TYPES.get(ins[i - 1].ins_type.name)
        rule = _param_types(x.args[0], x.args[1])
        if pushed is None or rule is None:
            continue
        first, later = rule
        if pushed == first:
            yield TSL_FUNCTIONS[x.args[0]].name, "normal"
        elif pushed in later:
            yield TSL_FUNCTIONS[x.args[0]].name, "swapped"


def count_calls(data):
    """(source-order calls, last-first calls) among checkable engine calls, or None if unreadable."""
    try:
        ins = read_ncs(data).instructions
    except Exception:
        return None
    swapped = normal = 0
    for _, kind in classify(ins):
        swapped += kind == "swapped"
        normal += kind == "normal"
    return swapped, normal


def scripts(game_dir, override_only=False):
    """(label, bytes) of every .ncs in override/ and, unless override_only, in Modules/*.mod/.erf."""
    game_dir = Path(game_dir)
    for p in sorted((game_dir / "override").iterdir()):
        if p.suffix.lower() == ".ncs" and p.is_file() and not p.is_symlink():
            yield f"override/{p.name}", p.read_bytes()
    if override_only or not (game_dir / "Modules").is_dir():
        return
    for p in sorted((game_dir / "Modules").iterdir()):
        ext = p.suffix.lower()
        if p.is_symlink() or ext not in (".mod", ".erf"):
            continue
        try:
            capsule = read_erf(p)
        except Exception:
            continue
        for r in capsule:
            if r.restype.extension == "ncs":
                yield f"Modules/{p.name}:{r.resref}.ncs", r.data


def check(game_dir, override_only=False):
    """[(label, swapped, normal)] of flagged scripts, and the labels of unreadable ones."""
    flagged, unreadable = [], []
    for label, data in scripts(game_dir, override_only):
        counts = count_calls(data)
        if counts is None:
            unreadable.append(label)
        elif counts[0]:
            flagged.append((label, *counts))
    return flagged, unreadable


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("game_dir", nargs="?", default=KOTOR2_ROOT)
    ap.add_argument("--override-only", action="store_true", help="Skip the scripts inside Modules/ capsules")
    args = ap.parse_args()
    flagged, unreadable = check(args.game_dir, args.override_only)
    for label in unreadable:
        print(f"  (unreadable by pykotor, not checked: {label})")
    for label, swapped, normal in flagged:
        print(f"  {label}: {swapped} call(s) with arguments in source order, {normal} last-first")
    print(f"{len(flagged)} script(s) compiled with swapped engine-call arguments")
    return 1 if flagged else 0


if __name__ == "__main__":
    sys.exit(main())
