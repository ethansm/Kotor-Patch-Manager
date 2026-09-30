#!/usr/bin/env python3
"""Add a 3D_MODEL scene-view control to the live override/inventory_p.gui (Mod 1).

Post-step for scripts/generate_ultrawide_gui_overrides.py: the generator's manifest
records inventory_p.gui's deployed sha256, so a hand-modified live file makes a generator
re-run refuse ("foreign file"). Run `--rollback` here first, re-run the generator, then
re-apply this script.

The control is a copy of upgrade_p.gui's 3D_MODEL (CONTROLTYPE 4). The engine ignores an
unbound control, so the GUI is safe to deploy before the DLL (inventory looks unchanged).
The Inventory3DViewport patch binds it by tag in the CSWGuiInGameInventory constructor.

  --dry-run   print the plan, write nothing
  --rollback  restore the OLDEST backup taken by this script (the pre-3D_MODEL file)
  --replace   replace an existing 3D_MODEL (e.g. to change --rect)
  --layout X  S|R|T|L|K (to leave a layout, --rollback first): also resize neighbouring controls (see LAYOUTS) and set the 3D_MODEL
              rect; implies --replace. Every change prints old -> new; refuses if a control's
              current rect is neither its expected original nor the layout target.
"""
import argparse
import copy
import datetime
import hashlib
import shutil
import sys
from pathlib import Path

from pykotor.resource.formats.gff import read_gff, write_gff

GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
OVERRIDE = GAME / "override"
BACKUP_ROOT = Path.home() / "Games/KOTOR2_Mod_Workspace/backups"
TAG = "3D_MODEL"
DEFAULT_RECT = (645, 134, 116, 300)  # L,T,W,H in 800x600 GUI units; right of LB_DESCRIPTION


# Layouts: TAG -> {"orig": rect, "new": rect, "PROTOITEM"/"SCROLLBAR": (orig, new)} (L,T,W,H).
# 3D_MODEL is handled separately (added/replaced with LAYOUT_3D[name]).
DESC_R = {
    "LB_DESCRIPTION": {"orig": (407, 134, 232, 300), "new": (407, 134, 180, 300),
                       "PROTOITEM": ((419, 150, 197, 42), (419, 150, 145, 42)),
                       "SCROLLBAR": ((623, 134, 16, 300), (571, 134, 16, 300))},
    "LBL_CREDITS": {"orig": (407, 442, 223, 28), "new": (407, 442, 180, 28)},
    "LBL_CREDITS_VALUE": {"orig": (559, 442, 63, 28), "new": (524, 442, 63, 28)},
    "LBL_BAR5": {"orig": (407, 476, 227, 32), "new": (407, 476, 180, 32)},
}
# S: list shifted left 120 units into the empty strip; description 260 wide; 3D view 209x364 (aspect 1.03)
SHIFT_S = {
    "LB_ITEMS": {"orig": (161, 134, 236, 329), "new": (41, 134, 236, 329),
                 "PROTOITEM": ((173, 150, 200, 50), (53, 150, 200, 50)),
                 "SCROLLBAR": ((381, 134, 16, 329), (261, 134, 16, 329))},
    "BTN_USEITEM": {"orig": (161, 470, 227, 28), "new": (41, 470, 227, 28)},
    "BTN_EXIT": {"orig": (159, 516, 89, 28), "new": (39, 516, 89, 28)},
    "LB_DESCRIPTION": {"orig": (407, 134, 232, 300), "new": (287, 134, 260, 300),
                       "PROTOITEM": ((419, 150, 197, 42), (299, 150, 225, 42)),
                       "SCROLLBAR": ((623, 134, 16, 300), (531, 134, 16, 300))},
    "LBL_CREDITS": {"orig": (407, 442, 223, 28), "new": (287, 442, 260, 28)},
    "LBL_CREDITS_VALUE": {"orig": (559, 442, 63, 28), "new": (476, 442, 63, 28)},
    "LBL_BAR5": {"orig": (407, 476, 227, 32), "new": (287, 476, 260, 32)},
}
# U: S refined after the first live look: list 4 units further from the gold border (gap = list->description gap),
# LBL_BAR5 back at its vanilla rect (its notch lines up with the party portraits), 3D view ends above it (y 470).
# "alt" = the S value, accepted as a current state so U can be applied over a live S.
SHIFT_U = {
    "LB_ITEMS": {"orig": (161, 134, 236, 329), "new": (45, 134, 236, 329), "alt": (41, 134, 236, 329),
                 "PROTOITEM": ((173, 150, 200, 50), (57, 150, 200, 50), (53, 150, 200, 50)),
                 "SCROLLBAR": ((381, 134, 16, 329), (265, 134, 16, 329), (261, 134, 16, 329))},
    "BTN_USEITEM": {"orig": (161, 470, 227, 28), "new": (45, 470, 227, 28), "alt": (41, 470, 227, 28)},
    "BTN_EXIT": {"orig": (159, 516, 89, 28), "new": (45, 516, 89, 28), "alt": (39, 516, 89, 28)},
    "LB_DESCRIPTION": {"orig": (407, 134, 232, 300), "new": (291, 134, 256, 300), "alt": (287, 134, 260, 300),
                       "PROTOITEM": ((419, 150, 197, 42), (303, 150, 221, 42), (299, 150, 225, 42)),
                       "SCROLLBAR": ((623, 134, 16, 300), (531, 134, 16, 300), (531, 134, 16, 300))},
    "LBL_CREDITS": {"orig": (407, 442, 223, 28), "new": (291, 442, 256, 28), "alt": (287, 442, 260, 28)},
    "LBL_CREDITS_VALUE": {"orig": (559, 442, 63, 28), "new": (476, 442, 63, 28), "alt": (476, 442, 63, 28)},
    "LBL_BAR5": {"orig": (407, 476, 227, 32), "new": (407, 476, 227, 32), "alt": (287, 476, 260, 32)},
}
LAYOUT_3D = {"U": (557, 134, 209, 336), "S": (557, 134, 209, 364), "R": (593, 134, 173, 364), "T": (645, 134, 121, 364), "L": (38, 139, 108, 360), "K": DEFAULT_RECT}
LAYOUT_CTRLS = {"U": SHIFT_U, "S": SHIFT_S, "R": DESC_R, "T": {}, "L": {}, "K": {}}
EXT_KEYS = ("LEFT", "TOP", "WIDTH", "HEIGHT")


def get_rect(e):
    return tuple(e.get_int32(k) for k in EXT_KEYS)


def set_rect(e, r):
    for k, v in zip(EXT_KEYS, r):
        e.set_int32(k, v)


def resize_controls(cs, layout: str) -> list[str]:
    """Apply LAYOUT_CTRLS[layout] in place (idempotent); returns change lines. Raises on foreign rects."""
    lines = []
    for tag, spec in LAYOUT_CTRLS[layout].items():
        c = next((c for c in cs if c.get_string("TAG") == tag), None)
        if c is None:
            raise SystemExit(f"control {tag} not found")
        todo = [(None, spec["orig"], spec["new"], spec.get("alt"))] + [
            (k, *spec[k], *([None] if len(spec[k]) == 2 else [])) for k in ("PROTOITEM", "SCROLLBAR") if k in spec]
        for sub, orig, new, alt in todo:
            holder = c if sub is None else c.get_struct(sub)
            e = holder.get_struct("EXTENT")
            cur = get_rect(e)
            name = tag if sub is None else f"{tag}.{sub}"
            if cur not in (orig, new, alt):
                raise SystemExit(f"refusing: {name} rect {cur} is neither expected original {orig} nor target {new}")
            if cur != new:
                set_rect(e, new)
                holder.set_struct("EXTENT", e)
                if sub is not None:
                    c.set_struct(sub, holder)
                lines.append(f"  {name}: {list(cur)} -> {list(new)}")
    return lines


def sha(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()


def find_ci(d: Path, name: str) -> Path:
    for p in d.iterdir():
        if p.name.lower() == name.lower():
            return p
    raise FileNotFoundError(name)


def controls(g):
    return g.root.get_list("CONTROLS")


def pristine_backup() -> Path | None:
    """Oldest backup = the file as it was before this script first touched it."""
    dirs = sorted(BACKUP_ROOT.glob("inventory3d_*"))
    return dirs[0] if dirs else None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--rect", default=",".join(map(str, DEFAULT_RECT)), help="L,T,W,H (default %(default)s)")
    ap.add_argument("--no-border", action="store_true", help="blank BORDER resrefs (already blank in vanilla)")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--rollback", action="store_true")
    ap.add_argument("--replace", action="store_true")
    ap.add_argument("--layout", choices=sorted(LAYOUT_3D), help="U|S|R|T|L|K layout (see docstring)")
    a = ap.parse_args()
    if a.layout:
        a.replace = True
        a.rect = ",".join(map(str, LAYOUT_3D[a.layout]))

    inv = find_ci(OVERRIDE, "inventory_p.gui")

    if a.rollback:
        b = pristine_backup()
        if b is None:
            print("no backup found", file=sys.stderr)
            return 1
        src = b / "inventory_p.gui"
        print(f"restore {src} -> {inv}")
        if not a.dry_run:
            shutil.copy2(src, inv)
        return 0

    rect = [int(x) for x in a.rect.split(",")]
    assert len(rect) == 4, "--rect needs L,T,W,H"

    up = read_gff(str(find_ci(OVERRIDE, "upgrade_p.gui")))
    src = [c for c in controls(up) if c.get_string("TAG") == TAG]
    assert len(src) == 1, f"expected exactly one {TAG} in upgrade_p.gui, got {len(src)}"

    g = read_gff(str(inv))
    cs = controls(g)
    existing = [i for i, c in enumerate(cs) if c.get_string("TAG") == TAG]
    if existing and not a.replace:
        print(f"{TAG} already present in inventory_p.gui (idx {existing}); use --replace to redo")
        return 0
    old3d = get_rect(cs[existing[0]].get_struct("EXTENT")) if existing else None
    for i in reversed(existing):
        del cs[i]

    changes = resize_controls(cs, a.layout) if a.layout else []
    ids = sorted(c.get_int32("ID") for c in cs)
    new_id = len(cs)
    assert ids == list(range(new_id)), f"inventory IDs are not 0..N-1: {ids}"

    ctl = copy.deepcopy(src[0])  # GFFStruct deep copy keeps struct_id
    ctl.set_int32("ID", new_id)
    ctl.set_string("TAG", TAG)
    ext = ctl.get_struct("EXTENT")
    for k, v in zip(("LEFT", "TOP", "WIDTH", "HEIGHT"), rect):
        ext.set_int32(k, v)
    ctl.set_struct("EXTENT", ext)  # get_struct returns a copy
    if a.no_border:
        b = ctl.get_struct("BORDER")
        for k in ("CORNER", "EDGE", "FILL"):
            b.set_resref(k, type(b.get_resref(k))(""))
        ctl.set_struct("BORDER", b)
    cs.append(ctl)
    g.root.set_list("CONTROLS", cs)  # get_list may hand back a detached copy
    if a.layout:
        print(f"layout {a.layout}: {len(changes)} sub-control change(s)")
        print("\n".join(changes))
    if a.layout and old3d == tuple(rect) and not changes:
        print(f"layout {a.layout} already applied; nothing to do")
        return 0
    if old3d:
        print(f"  {TAG}: {list(old3d)} -> {rect}")
    print(f"plan: append {TAG} ID={new_id} EXTENT={rect} to {inv} (had {len(cs) - 1} controls)")
    if a.dry_run:
        return 0

    stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    bdir = BACKUP_ROOT / f"inventory3d_{stamp}"
    bdir.mkdir(parents=True)
    shutil.copy2(inv, bdir / "inventory_p.gui")
    (bdir / "inventory_p.gui.sha256").write_text(sha(inv) + "\n")
    tmp = inv.with_suffix(".gui.tmp")
    write_gff(g, str(tmp))

    # verify by re-reading
    v = read_gff(str(tmp))
    vc = controls(v)
    vids = sorted(c.get_int32("ID") for c in vc)
    assert vids == list(range(len(vc))), vids
    nc = [c for c in vc if c.get_string("TAG") == TAG]
    assert len(nc) == 1 and nc[0].get_int32("ID") == new_id
    e = nc[0].get_struct("EXTENT")
    assert [e.get_int32(k) for k in ("LEFT", "TOP", "WIDTH", "HEIGHT")] == rect
    assert nc[0].get_int32("CONTROLTYPE") == 4
    tmp.replace(inv)
    print(f"deployed. backup: {bdir}\nnew sha256: {sha(inv)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
