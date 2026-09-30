#!/usr/bin/env python3
"""Add the Force Power Hotbar button labels to the live override/mipc28x6_p.gui (Mod 3) and write the two frame textures.

Per slot (key 1..9,0) two type-4 labels: a frame LBL_HOTBARBG<d> (IDs 111..120, FILL hotbar_slot; the DLL swaps in hotbar_slot_hi on
hover) and the icon+digit label LBL_HOTBAR<d> (IDs 121..130) inset inside it. Frames are drawn first (lower ID). The textures
override/hotbar_slot.tga / hotbar_slot_hi.tga (rounded square, teal / beige like the native lbl_mibox00 / 00a) are generated here with PIL.

The HUD panel only binds controls its constructor asks for by tag, so the GUI edit alone does nothing
(same as Mod 1's 3D_MODEL). The ForcePowerHotbar patch (hook 0x00748502, inside the HUD constructor)
allocates a CSWGuiLabel per entry and binds it by tag; the panel then draws it with the HUD.
An unbound control is ignored by the engine, so this file is safe to deploy before the DLL.

Each control is a copy of the HUD's own LBL_QUEUE1 (type 4 label: empty FILL, FILLSTYLE 2, empty TEXT); the icon label carries the
key digit as TEXT (top-left, white dialogfont16x16). Layout from --x/--y/--frame/--inset/--spacing.
The DLL only swaps the FILL image (icon), the border alpha (dim) and the visible flag at runtime, so
the row position is defined ONLY here (800x600 GUI units; the Aspyr HUD is drawn 1:1, anchored to the screen edges,
2.667 px/unit at 3840x1600; default row sits just above the native action bar, bottom-left).

Generator interaction: scripts/generate_ultrawide_gui_overrides.py records mipc28x6_p.gui's deployed sha256
(HUD action-description fix). A hand-modified live file makes a generator re-run refuse ("foreign file"), so:
    --rollback  ->  re-run the generator  ->  re-apply this script.

  --dry-run     print the plan, write nothing
  --rollback    restore the newest backup without LBL_HOTBAR* controls (the pre-hotbar file) and delete the two textures
  --replace     replace existing LBL_HOTBAR* controls (e.g. to change the layout)
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
GUI = "mipc28x6_p.gui"
TEMPLATE_TAG = "LBL_QUEUE1"
DIGITS = "1234567890"
BG_TAGS = [f"LBL_HOTBARBG{d}" for d in DIGITS]  # slot 0..9 <-> key 1..9,0; IDs FIRST_ID..FIRST_ID+9
TAGS = [f"LBL_HOTBAR{d}" for d in DIGITS]       # icon labels, IDs FIRST_ID+10..
ALL_TAGS = BG_TAGS + TAGS
TEX = ("hotbar_slot", "hotbar_slot_hi")
# Defaults in 800x600 GUI units: a row above the native action bar (top y=532), bottom-left.
DEFAULT_X, DEFAULT_Y, DEFAULT_FRAME, DEFAULT_INSET, DEFAULT_SPACING = 12, 484, 42, 5, 46
FIRST_ID = 111  # frames; icons at FIRST_ID+10. Must equal FirstButtonId in ForcePowerHotbar.cpp
TEXT_ALIGN_TOP_LEFT = 9
EXT_KEYS = ("LEFT", "TOP", "WIDTH", "HEIGHT")


def make_textures(out: Path) -> list[Path]:
    """Rounded-square slot frames, 128x128 RGBA TGA (no TXI: plain alpha blending)."""
    from PIL import Image, ImageDraw
    made = []
    for name, col in ((TEX[0], (18, 110, 85)), (TEX[1], (208, 204, 170))):
        S = 4  # supersample
        im = Image.new("RGBA", (128 * S, 128 * S), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.rounded_rectangle([0, 0, 128 * S - 1, 128 * S - 1], radius=22 * S, fill=col + (255,))
        d.rounded_rectangle([9 * S, 9 * S, 119 * S, 119 * S], radius=14 * S, fill=(0, 0, 0, 175))
        im = im.resize((128, 128), Image.LANCZOS)
        p = out / f"{name}.tga"
        im.save(p, format="TGA")
        made.append(p)
    return made


def sha(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()


def find_ci(d: Path, name: str) -> Path:
    for p in d.iterdir():
        if p.name.lower() == name.lower():
            return p
    raise FileNotFoundError(name)


def pristine_backup() -> Path | None:
    """Newest backup that does not itself contain LBL_HOTBAR* = the last file state before this script touched it."""
    for d in sorted(BACKUP_ROOT.glob("hotbar_gui_*"), reverse=True):  # newest first (a generator run may have moved the base)
        try:
            tags = {c.get_string("TAG") for c in read_gff(str(d / GUI)).root.get_list("CONTROLS")}
        except Exception:
            continue
        if not tags & set(ALL_TAGS):
            return d
    return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--x", type=int, default=DEFAULT_X, help="left of button 1 (default %(default)s)")
    ap.add_argument("--y", type=int, default=DEFAULT_Y, help="top of the row (default %(default)s)")
    ap.add_argument("--frame", type=int, default=DEFAULT_FRAME, help="frame width=height (default %(default)s)")
    ap.add_argument("--inset", type=int, default=DEFAULT_INSET, help="icon inset inside the frame (default %(default)s)")
    ap.add_argument("--spacing", type=int, default=DEFAULT_SPACING, help="x step between frames (default %(default)s)")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--rollback", action="store_true")
    ap.add_argument("--replace", action="store_true")
    a = ap.parse_args()

    hud = find_ci(OVERRIDE, GUI)

    if a.rollback:
        b = pristine_backup()
        if b is None:
            print("no backup found", file=sys.stderr)
            return 1
        src = b / GUI
        print(f"restore {src} -> {hud}")
        for t in TEX:
            print(f"delete {OVERRIDE / (t + '.tga')}")
        if not a.dry_run:
            shutil.copy2(src, hud)
            for t in TEX:
                (OVERRIDE / f"{t}.tga").unlink(missing_ok=True)
        return 0

    g = read_gff(str(hud))
    cs = g.root.get_list("CONTROLS")
    have = [i for i, c in enumerate(cs) if c.get_string("TAG") in ALL_TAGS]
    if have and not a.replace:
        print(f"{len(have)} LBL_HOTBAR* control(s) already present in {GUI}; use --replace to redo")
        return 0
    old = {cs[i].get_string("TAG"): tuple(cs[i].get_struct("EXTENT").get_int32(k) for k in EXT_KEYS) for i in have}
    for i in reversed(have):
        del cs[i]

    tmpl = [c for c in cs if c.get_string("TAG") == TEMPLATE_TAG]
    assert len(tmpl) == 1, f"expected exactly one {TEMPLATE_TAG} in {GUI}, got {len(tmpl)}"
    ids = sorted(c.get_int32("ID") for c in cs)
    first_id = len(cs)
    assert ids == list(range(first_id)), f"{GUI} IDs are not 0..N-1: {ids}"
    if first_id != FIRST_ID:  # the DLL hard-codes these IDs; a mismatch would misbind (InitControl can overwrite a native slot)
        print(f"refusing: {GUI} has {first_id} controls, the DLL expects the buttons at IDs {FIRST_ID}..{FIRST_ID + 19}", file=sys.stderr)
        return 1
    assert tmpl[0].get_int32("CONTROLTYPE") == 4, "template is not a type-4 label"

    def make(cid, tag, rect, digit=None, fill=None):
        ctl = copy.deepcopy(tmpl[0])  # GFFStruct deep copy keeps struct_id
        ctl.set_int32("ID", cid)
        ctl.set_string("TAG", tag)
        ext = ctl.get_struct("EXTENT")  # get_struct returns a COPY: edit then set_struct
        for k, v in zip(EXT_KEYS, rect):
            ext.set_int32(k, v)
        ctl.set_struct("EXTENT", ext)
        if digit is not None:
            txt = ctl.get_struct("TEXT")
            txt.set_string("TEXT", digit)
            txt.set_int32("ALIGNMENT", TEXT_ALIGN_TOP_LEFT)
            ctl.set_struct("TEXT", txt)
        if fill:
            b = ctl.get_struct("BORDER")
            b.set_resref("FILL", type(b.get_resref("FILL"))(fill))
            ctl.set_struct("BORDER", b)
        cs.append(ctl)
        plan.append((tag, cid, rect))

    plan = []
    for slot, digit in enumerate(DIGITS):
        fx = a.x + slot * a.spacing
        make(first_id + slot, BG_TAGS[slot], [fx, a.y, a.frame, a.frame], fill=TEX[0])
    for slot, digit in enumerate(DIGITS):
        fx = a.x + slot * a.spacing
        make(first_id + 10 + slot, TAGS[slot], [fx + a.inset, a.y + a.inset, a.frame - 2 * a.inset, a.frame - 2 * a.inset], digit=digit)
    g.root.set_list("CONTROLS", cs)  # get_list may hand back a detached copy

    for tag, cid, rect in plan:
        o = old.get(tag)
        print(f"  {tag} ID={cid}: {list(o) if o else 'new'} -> {rect}")
    print(f"plan: {len(plan)} controls appended to {hud} ({first_id} -> {len(cs)} controls)")
    if a.dry_run:
        return 0

    stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    bdir = BACKUP_ROOT / f"hotbar_gui_{stamp}"
    bdir.mkdir(parents=True)
    shutil.copy2(hud, bdir / GUI)
    (bdir / f"{GUI}.sha256").write_text(sha(hud) + "\n")
    made = make_textures(OVERRIDE)
    print("wrote textures: " + ", ".join(str(m) for m in made))
    tmp = hud.with_suffix(".gui.tmp")
    write_gff(g, str(tmp))

    v = read_gff(str(tmp))
    vc = v.root.get_list("CONTROLS")
    vids = sorted(c.get_int32("ID") for c in vc)
    assert vids == list(range(len(vc))), vids
    for tag, cid, rect in plan:
        m = [c for c in vc if c.get_string("TAG") == tag]
        assert len(m) == 1 and m[0].get_int32("ID") == cid and m[0].get_int32("CONTROLTYPE") == 4, tag
        e = m[0].get_struct("EXTENT")
        assert [e.get_int32(k) for k in EXT_KEYS] == rect, tag
    tmp.replace(hud)
    print(f"deployed. backup: {bdir}\nnew sha256: {sha(hud)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
