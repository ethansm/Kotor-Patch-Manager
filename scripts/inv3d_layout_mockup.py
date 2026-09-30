#!/usr/bin/env python3
"""Render layout candidates (R/T/L/K) for the inventory 3D view (Mod 1 v2).

Schematic 3840x1600 PNG per candidate (GUI units * (4.8, 8/3)), plus an overlay on a user
screenshot (calibrated by affine fit: LB_ITEMS / LB_DESCRIPTION / v1 3D_MODEL rects).
usage: inv3d_layout_mockup.py OUTDIR [screenshot.png]
"""
import logging, sys
from pathlib import Path
from PIL import Image, ImageDraw
from pykotor.resource.formats.gff import read_gff
logging.disable(logging.CRITICAL)

GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
SX, SY = 4.8, 8 / 3
# tag -> new rect (L,T,W,H)
LAYOUTS = {
    "K": {},
    "V": {"LB_ITEMS": (48, 134, 236, 329), "BTN_USEITEM": (48, 470, 227, 28), "BTN_EXIT": (48, 516, 89, 28),
          "LB_DESCRIPTION": (294, 134, 246, 300), "LBL_CREDITS": (294, 442, 246, 28),
          "LBL_CREDITS_VALUE": (469, 442, 63, 28), "LBL_BAR5": (407, 476, 227, 32),
          "LBL_BEVEL": (546, 130, 210, 38), "LBL_BEVEL2": (546, 436, 210, 38),
          "3D_MODEL": (550, 134, 202, 336)},
    "U": {"LB_ITEMS": (45, 134, 236, 329), "BTN_USEITEM": (45, 470, 227, 28), "BTN_EXIT": (45, 516, 89, 28),
          "LB_DESCRIPTION": (291, 134, 256, 300), "LBL_CREDITS": (291, 442, 256, 28),
          "LBL_CREDITS_VALUE": (476, 442, 63, 28), "LBL_BAR5": (407, 476, 227, 32),
          "3D_MODEL": (557, 134, 209, 336)},
    "S": {"LB_ITEMS": (41, 134, 236, 329), "BTN_USEITEM": (41, 470, 227, 28), "BTN_EXIT": (39, 516, 89, 28),
          "LB_DESCRIPTION": (287, 134, 260, 300), "LBL_CREDITS": (287, 442, 260, 28),
          "LBL_CREDITS_VALUE": (476, 442, 63, 28), "LBL_BAR5": (287, 476, 260, 32),
          "3D_MODEL": (557, 134, 209, 364)},
    "R": {"LB_DESCRIPTION": (407, 134, 180, 300), "LBL_CREDITS": (407, 442, 180, 28),
          "LBL_CREDITS_VALUE": (524, 442, 63, 28), "LBL_BAR5": (407, 476, 180, 32),
          "3D_MODEL": (593, 134, 173, 364)},
    "T": {"3D_MODEL": (645, 134, 121, 364)},
    "L": {"3D_MODEL": (38, 139, 108, 360)},
}
V1 = (645, 134, 116, 300)


def load():
    g = read_gff(str(next(p for p in (GAME / "override").iterdir() if p.name.lower() == "inventory_p.gui")))
    out = {}
    for c in g.root.get_list("CONTROLS"):
        e = c.get_struct("EXTENT")
        out[c.get_string("TAG")] = tuple(e.get_int32(k) for k in ("LEFT", "TOP", "WIDTH", "HEIGHT"))
    return out


def layout(base, name):
    d = dict(base)
    d["3D_MODEL"] = V1
    d.update(LAYOUTS[name])
    return d


def draw_rects(im, rects, changed, mapper, view_fill=(255, 200, 0, 90)):
    ov = Image.new("RGBA", im.size, (0, 0, 0, 0))
    dr = ImageDraw.Draw(ov)
    for tag, r in rects.items():
        x0, y0, x1, y1 = mapper(r)
        if tag == "3D_MODEL":
            dr.rectangle([x0, y0, x1, y1], fill=view_fill, outline=(255, 200, 0, 255), width=4)
        else:
            col = (255, 60, 60, 255) if tag in changed else (0, 220, 200, 200)
            dr.rectangle([x0, y0, x1, y1], outline=col, width=3 if tag in changed else 1)
        if tag in changed or tag == "3D_MODEL" or tag.startswith("LB_"):
            dr.text((x0 + 4, y0 + 3), tag, fill=(255, 255, 255, 255))
    return Image.alpha_composite(im.convert("RGBA"), ov)


def main():
    out = Path(sys.argv[1]); out.mkdir(parents=True, exist_ok=True)
    shot = Path(sys.argv[2]) if len(sys.argv) > 2 else None
    base = load()
    for name in LAYOUTS:
        rects = layout(base, name)
        changed = {t for t, r in rects.items() if base.get(t) != r}
        w, h = rects["3D_MODEL"][2] * SX, rects["3D_MODEL"][3] * SY
        im = Image.new("RGB", (3840, 1600), (10, 20, 22))
        im = draw_rects(im, rects, changed, lambda r: (r[0] * SX, r[1] * SY, (r[0] + r[2]) * SX, (r[1] + r[3]) * SY))
        d = ImageDraw.Draw(im)
        d.text((20, 20), f"layout {name}: 3D view {w:.0f}x{h:.0f}px aspect {w/h:.3f}", fill=(255, 255, 255, 255))
        p = out / f"layout_{name}_schematic.png"; im.convert("RGB").save(p); print(p, f"{w:.0f}x{h:.0f} aspect {w/h:.3f}")
        if shot:
            s = Image.open(shot)
            # affine fit measured on the 3428x1528 screenshot: x = ax*g+bx, y = ay*g+by
            ax, bx, ay, by = 2.76 * 1.714, -229 * 1.714, 1.544 * 1.714, -32 * 1.714
            o = draw_rects(s, rects, changed, lambda r: (ax * r[0] + bx, ay * r[1] + by, ax * (r[0] + r[2]) + bx, ay * (r[1] + r[3]) + by))
            p = out / f"layout_{name}_on_screenshot.png"; o.convert("RGB").save(p); print(p)


if __name__ == "__main__":
    main()
