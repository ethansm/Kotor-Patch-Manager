#!/usr/bin/env python3
"""Shift the leader's four toggle buttons left to make room for the Shield Bar (Mod 4).

Edits the live override/mipc28x6_p.gui: TB_STEALTH / TB_SOLO / BTN_SWAPWEAPONS / TB_PAUSE
(all y 562, 36x36 gui units at x 548/584/620/656) move left by SHIFT units (default 16 =
0.328*39 bar width + gap, ~2.667 px/unit). Nothing else is touched (PB_VIT*, portraits, cards).

Post-step for scripts/generate_ultrawide_gui_overrides.py: the generator's manifest records
the deployed sha256, so a hand-modified live file makes a generator re-run refuse ("foreign
file"). Run `--rollback` here first, re-run the generator, then re-apply this script.

Idempotent; refuses if a button's rect is neither its expected original nor the target.

  --dry-run   print the plan, write nothing (also lists every control in the button strip)
  --rollback  restore the OLDEST backup taken by this script
  --shift N   gui units to move left (default 16)
"""
import argparse
import datetime
import hashlib
import shutil
import sys
from pathlib import Path

from pykotor.resource.formats.gff import read_gff, write_gff

GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
OVERRIDE = GAME / "override"
BACKUP_ROOT = Path.home() / "Games/KOTOR2_Mod_Workspace/backups"
GUI = "mipc28x6_p.gui"
BUTTONS = {"TB_STEALTH": 548, "TB_SOLO": 584, "BTN_SWAPWEAPONS": 620, "TB_PAUSE": 656}  # original LEFT
BUTTON_TOP, BUTTON_SIZE = 562, 36
EXT = ("LEFT", "TOP", "WIDTH", "HEIGHT")


def sha(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()


def find_ci(d: Path, name: str) -> Path:
    for p in d.iterdir():
        if p.name.lower() == name.lower():
            return p
    raise FileNotFoundError(name)


def rect(c):
    e = c.get_struct("EXTENT")
    return [e.get_int32(k) for k in EXT]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--rollback", action="store_true")
    ap.add_argument("--shift", type=int, default=16)
    a = ap.parse_args()
    gui = find_ci(OVERRIDE, GUI)

    if a.rollback:
        dirs = sorted(BACKUP_ROOT.glob("hudshift_*"))
        if not dirs:
            print("no backup found", file=sys.stderr)
            return 1
        src = dirs[0] / GUI
        print(f"restore {src} -> {gui}")
        if not a.dry_run:
            shutil.copy2(src, gui)
        return 0

    g = read_gff(str(gui))
    cs = g.root.get_list("CONTROLS")
    if a.dry_run:
        print("controls intersecting x 540-700, y 555-600:")
        for c in cs:
            r = rect(c)
            if r[0] < 700 and r[0] + r[2] > 540 and r[1] < 600 and r[1] + r[3] > 555:
                print(f"  {c.get_string('TAG'):18} {r}")
    changes = []
    for tag, orig_x in BUTTONS.items():
        c = next((c for c in cs if c.get_string("TAG") == tag), None)
        if c is None:
            raise SystemExit(f"control {tag} not found")
        cur = rect(c)
        orig, new = [orig_x, BUTTON_TOP, BUTTON_SIZE, BUTTON_SIZE], [orig_x - a.shift, BUTTON_TOP, BUTTON_SIZE, BUTTON_SIZE]
        if cur not in (orig, new):
            raise SystemExit(f"refusing: {tag} rect {cur} is neither expected original {orig} nor target {new}")
        if cur != new:
            e = c.get_struct("EXTENT")
            e.set_int32("LEFT", new[0])
            c.set_struct("EXTENT", e)
            changes.append(f"  {tag}: {cur} -> {new}")
    if not changes:
        print("already applied; nothing to do")
        return 0
    print("\n".join(changes))
    if a.dry_run:
        return 0
    g.root.set_list("CONTROLS", cs)

    stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    bdir = BACKUP_ROOT / f"hudshift_{stamp}"
    bdir.mkdir(parents=True)
    shutil.copy2(gui, bdir / GUI)
    (bdir / (GUI + ".sha256")).write_text(sha(gui) + "\n")
    tmp = gui.with_suffix(".gui.tmp")
    write_gff(g, str(tmp))
    v = read_gff(str(tmp))
    vc = v.root.get_list("CONTROLS")
    assert len(vc) == len(cs)
    for tag, orig_x in BUTTONS.items():
        c = next(c for c in vc if c.get_string("TAG") == tag)
        assert rect(c)[0] == orig_x - a.shift, tag
    tmp.replace(gui)
    print(f"deployed. backup: {bdir}\nnew sha256: {sha(gui)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
