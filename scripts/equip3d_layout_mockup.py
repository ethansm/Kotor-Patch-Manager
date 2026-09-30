#!/usr/bin/env python3
"""Render layout candidates (A/R/G) for the equipment-page 3D character view (Mod 5, research R7 / Phase 2).

Reads the LIVE override/equip_p.gui, applies each candidate's rect changes and draws a 3840x1600 schematic
(GUI units * (4.8, 8/3)), plus an optional overlay on a user screenshot of the equipment page (affine calibration
from two known vanilla rects: --calib "gx0,gy0,px0,py0,gx1,gy1,px1,py1" = GUI point -> screenshot pixel).
usage: equip3d_layout_mockup.py OUTDIR [screenshot.png] [--calib ...]

Shared candidate facts (units, x 34..766 usable; 14-unit edge gaps, 10-unit panel gaps; LBL_BAR5 stays vanilla,
it carries the party-portrait notch):
  vanilla: LB_ITEMS 161..397 | gap 10 | slot/description block 407..639 | free gutters 34..160 and 640..766
  A  view on the RIGHT (mirror of Mod 1 layout V): list + block shifted -113 -> LB_ITEMS 48..284, block 294..526,
     view [536,134,216,336] (aspect 0.643 ~ character page 230x360 = 0.639, so the engine's own framing fits)
  R  view on the LEFT: view [48,134,216,336], list + block shifted +113 -> list 274..510, block 520..752
  G  right gutter only, nothing moves: view [650,134,102,336] (aspect 0.30; zero disruption, cramped)
"""
import logging, sys
from pathlib import Path
from PIL import Image, ImageDraw
from pykotor.resource.formats.gff import read_gff
logging.disable(logging.CRITICAL)

GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
SX, SY = 4.8, 8 / 3

# controls of the slot/description block (move together) and of the list side (move together)
BLOCK = ["LB_DESC", "LBL_BACK1", "LBL_INV_WEAP_R2", "LBL_INV_WEAP_L2", "LBL_INV_WEAP_R", "LBL_INV_WEAP_L",
         "LBL_INV_BELT", "LBL_INV_ARM_R", "LBL_INV_BODY", "LBL_INV_ARM_L", "LBL_INV_HANDS", "LBL_INV_HEAD",
         "LBL_INV_IMPLANT", "LBL_DEF_BACK", "LBL_DEF", "LBL_DEF_TEXT", "LBL_TOHITR", "LBL_TOHITL", "LBL_TOHIT",
         "LBL_ATKL", "LBL_ATKR", "LBL_DAMAGE", "LBL_ATTACKMOD", "LBL_DAMTEXT", "BTN_SWAPWEAPONS",
         "BTN_INV_HEAD", "BTN_INV_BODY", "BTN_INV_ARM_L", "BTN_INV_WEAP_L", "BTN_INV_WEAP_L2", "BTN_INV_WEAP_R2",
         "BTN_INV_BELT", "BTN_INV_WEAP_R", "BTN_INV_ARM_R", "BTN_INV_HANDS", "BTN_INV_IMPLANT"]
LIST = ["LB_ITEMS", "LBL_CANTEQUIP", "BTN_BACK", "BTN_EQUIP"]
# stay vanilla in every layout: LBL_BAR1..5 (BAR5 portrait notch), LBL_TITLE, LBL_SLOTNAME, BTN_PREVNPC/NEXTNPC

# candidate name -> (list dx, block dx, 3D_MODEL rect, cover rects)
CANDS = {
    "A": (-113, -113, (536, 134, 216, 336), {"LBL_BEVEL": (532, 130, 224, 38), "LBL_BEVEL2": (532, 436, 224, 38)}),
    "R": (+113, +113, (48, 134, 216, 336), {"LBL_BEVEL": (44, 130, 224, 38), "LBL_BEVEL2": (44, 436, 224, 38)}),
    "G": (0, 0, (650, 134, 102, 336), {}),
}


def load():
    g = read_gff(str(next(p for p in (GAME / "override").iterdir() if p.name.lower() == "equip_p.gui")))
    out = {}
    for c in g.root.get_list("CONTROLS"):
        e = c.get_struct("EXTENT")
        out[c.get_string("TAG")] = tuple(e.get_int32(k) for k in ("LEFT", "TOP", "WIDTH", "HEIGHT"))
    return out


def layout(base, name):
    ldx, bdx, view, covers = CANDS[name]
    d = dict(base)
    for t in LIST:
        if t in d:
            l, tp, w, h = d[t]; d[t] = (l + ldx, tp, w, h)
    for t in BLOCK:
        if t in d:
            l, tp, w, h = d[t]; d[t] = (l + bdx, tp, w, h)
    d.update(covers)
    d["3D_MODEL"] = view
    return d


def draw_rects(im, rects, changed, mapper):
    ov = Image.new("RGBA", im.size, (0, 0, 0, 0))
    dr = ImageDraw.Draw(ov)
    for tag, r in rects.items():
        x0, y0, x1, y1 = mapper(r)
        if tag == "3D_MODEL":
            dr.rectangle([x0, y0, x1, y1], fill=(255, 200, 0, 90), outline=(255, 200, 0, 255), width=4)
        elif tag.startswith("LBL_BEVEL"):
            dr.rectangle([x0, y0, x1, y1], outline=(255, 140, 0, 255), width=2)
        else:
            col = (255, 60, 60, 255) if tag in changed else (0, 220, 200, 200)
            dr.rectangle([x0, y0, x1, y1], outline=col, width=3 if tag in changed else 1)
        if tag in ("3D_MODEL", "LB_ITEMS", "LB_DESC", "LBL_BAR5", "BTN_BACK", "BTN_EQUIP", "BTN_SWAPWEAPONS") or tag.startswith("LBL_BAR"):
            dr.text((x0 + 4, y0 + 3), tag, fill=(255, 255, 255, 255))
    return Image.alpha_composite(im.convert("RGBA"), ov)


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--calib")]
    out = Path(args[0]); out.mkdir(parents=True, exist_ok=True)
    shot = Path(args[1]) if len(args) > 1 else None
    calib = next((a.split("=", 1)[1] for a in sys.argv if a.startswith("--calib=")), None)
    base = load()
    for name in ["V"] + list(CANDS):
        rects = dict(base) if name == "V" else layout(base, name)   # "V" = vanilla reference
        changed = {t for t, r in rects.items() if base.get(t) != r}
        note = "vanilla (no 3D view)"
        if "3D_MODEL" in rects:
            w, h = rects["3D_MODEL"][2] * SX, rects["3D_MODEL"][3] * SY
            note = f"3D view {w:.0f}x{h:.0f}px aspect {w/h:.3f}"
        im = Image.new("RGB", (3840, 1600), (10, 20, 22))
        im = draw_rects(im, rects, changed, lambda r: (r[0] * SX, r[1] * SY, (r[0] + r[2]) * SX, (r[1] + r[3]) * SY))
        ImageDraw.Draw(im).text((20, 20), f"equip layout {name}: {note}", fill=(255, 255, 255, 255))
        p = out / f"equip_layout_{name}_schematic.png"; im.convert("RGB").save(p); print(p, note)
        if shot and calib:
            gx0, gy0, px0, py0, gx1, gy1, px1, py1 = [float(x) for x in calib.split(",")]
            ax = (px1 - px0) / (gx1 - gx0); bx = px0 - ax * gx0
            ay = (py1 - py0) / (gy1 - gy0); by = py0 - ay * gy0
            o = draw_rects(Image.open(shot), rects, changed, lambda r: (ax * r[0] + bx, ay * r[1] + by, ax * (r[0] + r[2]) + bx, ay * (r[1] + r[3]) + by))
            p = out / f"equip_layout_{name}_on_screenshot.png"; o.convert("RGB").save(p); print(p)


if __name__ == "__main__":
    main()
