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
TIP_BG_TAG, TIP_TAG = "LBL_HOTBARTIPBG", "LBL_HOTBARTIP"  # hover tooltip: frame + text; LAST two IDs so they draw above everything (panel Draw is ascending by ID)
TIP_TEMPLATE_TAG = "LBL_MOULDING3"  # the HUD's own dark teal framed panel
# ---- in-game configuration UI (0.3.0): edit toggle + picker (title, 3 category tabs, prev/next, Clear, 8 cells = frame+icon each)
EDIT_TAG = "LBL_HOTBAREDIT"                                   # ID FIRST_ID+20 (thin wrench button)
PBG_TAG = "LBL_HOTBARPBG"                                     # +21 group background (copy of the native quick-command panel LBL_MOULDING3)
TITLE_TAG = "LBL_HOTBARPTITLE"                                # +22
TAB_TAGS = [f"LBL_HOTBARTAB{i}" for i in range(3)]            # +23..25
TAB_TEXT = ["Powers", "Items", "Modes"]
PREV_TAG, NEXT_TAG, CLEAR_TAG = "LBL_HOTBARPREV", "LBL_HOTBARNEXT", "LBL_HOTBARCLEAR"   # +26..28
CELLS = 8
CELL_BG_TAGS = [f"LBL_HOTBARCELLBG{i}" for i in range(CELLS)]  # +29..36
CELL_TAGS = [f"LBL_HOTBARCELL{i}" for i in range(CELLS)]       # +37..44
ALL_TAGS = (BG_TAGS + TAGS + [EDIT_TAG, PBG_TAG, TITLE_TAG] + TAB_TAGS + [PREV_TAG, NEXT_TAG, CLEAR_TAG] + CELL_BG_TAGS + CELL_TAGS
            + [TIP_BG_TAG, TIP_TAG])   # ID order == this order
TEX = ("hotbar_slot", "hotbar_slot_hi")
EDIT_TEX = ("hotbar_edit", "hotbar_edit_hi")   # thin (72x128) frame with a wrench, normal / active
SMALL_FONT = "dialogfont10x10"
EDIT_W = 24
# Defaults in 800x600 GUI units: a row above the native action bar (top y=532), bottom-left.
DEFAULT_X, DEFAULT_Y, DEFAULT_FRAME, DEFAULT_INSET, DEFAULT_SPACING = 42, 450, 42, 5, 46   # y=450: bottom edge (492) clears the native quick-action description label (y 496-528); x leaves room for the thin edit toggle at x=12 (24 wide)
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


def make_edit_textures(out: Path) -> list[Path]:
    """Thin rounded frame (72x128) with a wrench, normal and active (beige frame) variants."""
    from PIL import Image, ImageDraw, ImageFilter, ImageChops
    import math
    made = []
    for name, col in ((EDIT_TEX[0], (18, 110, 85)), (EDIT_TEX[1], (208, 204, 170))):
        S = 4
        W, H = 72, 128
        im = Image.new("RGBA", (W * S, H * S), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.rounded_rectangle([0, 0, W * S - 1, H * S - 1], radius=22 * S, fill=col + (255,))
        d.rounded_rectangle([9 * S, 9 * S, (W - 9) * S, (H - 9) * S], radius=14 * S, fill=(0, 0, 0, 175))
        # wrench mask (smaller than build 10), then shaded like the native force-power icons: mint->teal vertical gradient,
        # dark outline, bright upper-left rim light
        m = Image.new("L", (W * S, H * S), 0)
        md = ImageDraw.Draw(m)
        hx, hy, tx, ty = 29, 50, 45, 88   # head centre, tail end
        md.line([(hx * S, hy * S), (tx * S, ty * S)], fill=255, width=9 * S)
        md.ellipse([(tx - 4.5) * S, (ty - 4.5) * S, (tx + 4.5) * S, (ty + 4.5) * S], fill=255)
        md.ellipse([(hx - 12) * S, (hy - 12) * S, (hx + 12) * S, (hy + 12) * S], fill=255)
        ang = math.atan2(ty - hy, tx - hx) + math.pi   # jaw opens away from the handle
        def pt(r, a): return ((hx + r * math.cos(a)) * S, (hy + r * math.sin(a)) * S)
        md.polygon([pt(0, ang), pt(18, ang - 0.42), pt(18, ang + 0.42)], fill=0)
        grad = Image.new("RGBA", (W * S, H * S))
        gp = grad.load()
        top, bot = (175, 252, 228), (28, 150, 122)
        y0, y1 = (hy - 12) * S, (ty + 5) * S
        for y in range(H * S):
            t = min(1.0, max(0.0, (y - y0) / (y1 - y0)))
            c = tuple(int(top[i] + (bot[i] - top[i]) * t) for i in range(3)) + (255,)
            for x in range(W * S): gp[x, y] = c
        outline = m.filter(ImageFilter.MaxFilter(2 * S + 1))
        d.bitmap((0, 0), outline, fill=(6, 60, 50, 255))
        shifted = ImageChops.offset(m, int(2.2 * S), int(2.2 * S))
        rim = ImageChops.subtract(m, shifted)                         # edges facing the upper-left light
        im.paste(grad, (0, 0), m)
        im.paste(Image.new("RGBA", (W * S, H * S), (235, 255, 246, 255)), (0, 0), rim.filter(ImageFilter.GaussianBlur(S * 0.6)))
        im = im.resize((W, H), Image.LANCZOS)
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
        for t in TEX + EDIT_TEX:
            print(f"delete {OVERRIDE / (t + '.tga')}")
        if not a.dry_run:
            shutil.copy2(src, hud)
            for t in TEX + EDIT_TEX:
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
        print(f"refusing: {GUI} has {first_id} controls, the DLL expects the buttons at IDs {FIRST_ID}..{FIRST_ID + len(ALL_TAGS) - 1}", file=sys.stderr)
        return 1
    assert tmpl[0].get_int32("CONTROLTYPE") == 4, "template is not a type-4 label"

    tip_t = [c for c in cs if c.get_string("TAG") == TIP_TEMPLATE_TAG]
    assert len(tip_t) == 1, f"expected exactly one {TIP_TEMPLATE_TAG} in {GUI}"

    def make(cid, tag, rect, digit=None, fill=None, src=None, text=None, align=None, font=None):
        ctl = copy.deepcopy(src if src is not None else tmpl[0])  # GFFStruct deep copy keeps struct_id
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
        if text is not None:
            txt = ctl.get_struct("TEXT")
            txt.set_string("TEXT", text)
            txt.set_int32("ALIGNMENT", align)
            if font:
                txt.set_resref("FONT", type(txt.get_resref("FONT"))(font))
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
    # tooltip (DLL positions/sizes/fills them at runtime; placeholder rects here): frame first, text on top
    # edit toggle (thin wrench button left of slot 1) and picker (above the bar): frame texture + static text on one label; the DLL
    # shows/hides/swaps them. Picker rows (units): title 22 / tabs 24 / cells 42, 4 apart, 8 padding inside the group background.
    n = first_id + 20
    make(n, EDIT_TAG, [12, a.y, EDIT_W, a.frame], fill=EDIT_TEX[0]); n += 1
    py = a.y - 10 - 96 - 4   # content top; content is 96 tall, box adds 8 padding each side
    bw = 424
    make(n, PBG_TAG, [a.x - 8, py - 8, bw + 16, 96 + 16], src=tip_t[0]); n += 1   # native panel look (fill uibit_fill_2bt + teal edge/corner)
    make(n, TITLE_TAG, [a.x, py, bw, 22], text="", align=17, font=SMALL_FONT); n += 1   # no fill: text sits on the group background
    for i, t in enumerate(TAB_TEXT):
        make(n, TAB_TAGS[i], [a.x + i * 88, py + 26, 84, 24], fill=TEX[0], text=t, align=18, font=SMALL_FONT); n += 1
    cx0 = a.x + 32   # cells start right of the prev arrow
    make(n, PREV_TAG, [a.x, py + 54, 28, a.frame], fill=TEX[0], text="<", align=18); n += 1
    make(n, NEXT_TAG, [a.x + bw - 28, py + 54, 28, a.frame], fill=TEX[0], text=">", align=18); n += 1
    make(n, CLEAR_TAG, [a.x + bw - 70, py + 26, 70, 24], fill=TEX[0], text="Clear", align=18, font=SMALL_FONT); n += 1
    for i in range(CELLS):
        make(n, CELL_BG_TAGS[i], [cx0 + i * a.spacing, py + 54, a.frame, a.frame], fill=TEX[0]); n += 1
    for i in range(CELLS):
        make(n, CELL_TAGS[i], [cx0 + i * a.spacing + a.inset, py + 54 + a.inset, a.frame - 2 * a.inset, a.frame - 2 * a.inset]); n += 1
    assert n == first_id + len(ALL_TAGS) - 2, n
    make(n, TIP_BG_TAG, [12, 380, 300, 90], src=tip_t[0]); n += 1
    make(n, TIP_TAG, [12, 380, 300, 90], text="", align=18)
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
    made = make_textures(OVERRIDE) + make_edit_textures(OVERRIDE)
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
