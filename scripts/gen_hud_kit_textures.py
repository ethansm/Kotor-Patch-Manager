#!/usr/bin/env python3
"""Generate the shared HUD-kit textures (queue-hud + companion-selfheal) and embed them in _shared_src/HudKitTextures.inc.

One uncompressed 32-bit TGA byte array per texture:
  kit_frame      64x64 hotbar slot tile (border (0,44,26), fill (0,0,0,175), radius 22/128 of size)
  kit_frame_hi   64x64 same, beige border (208,204,170)
  kit_cancel     64x64 kit_frame + shaded X glyph (wrench-family mint->teal recipe from gen_hotbar_textures.make_edit_textures)
  kit_cancel_hi  64x64 same on the beige frame
  kit_disc       48x48 backing disc (0,0,0,175) with 2 px (10,96,80) rim, radius 21, drawn untinted
Pixel-identical to the former qh_frame / qh_frame_hi / qh_cancel / qh_cancel_hi (gen_queuehud_textures.py) and csh_disc (gen_selfheal_textures.py).
Deterministic: no noise, no timestamps. Re-run after changing the art, then rebuild both patches.

The .inc emits only the data arrays and `kHudKitTextures[]` of embtex::Tex (struct defined in _shared_src/EmbedTex.h, which must be included
first) plus kHudKitTextureCount; _shared_src/HudKit.h includes it.

  --out-dir DIR   also write the .tga files to DIR (for inspection)
"""
import argparse
import math
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
INC = REPO / "patch_manager_mods/_shared_src/HudKitTextures.inc"
FRAME, FRAME_HI, CANCEL, CANCEL_HI, DISC = "kit_frame", "kit_frame_hi", "kit_cancel", "kit_cancel_hi", "kit_disc"
ALL_NAMES = (FRAME, FRAME_HI, CANCEL, CANCEL_HI, DISC)
SIZE = 64
SS = 4
BORDER, BORDER_HI = (0, 44, 26), (208, 204, 170)
X_SPAN = 38     # tip-to-tip extent of the X on each axis, px (~60% of the tile)
X_WIDTH = 7.0   # bar width, px
DISC_SIZE = 48
DISC_RADIUS, DISC_RIM = 21.0, 2.0   # radius == the selfheal busy-ring radius


def frame_img(col):
    """Slot tile at SS x supersample (hotbar recipe scaled 128 -> SIZE: radius 22/128, inset 6/128, inner radius 16/128)."""
    from PIL import Image, ImageDraw
    k = SIZE / 128.0
    N = SIZE * SS
    im = Image.new("RGBA", (N, N), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([0, 0, N - 1, N - 1], radius=22 * k * SS, fill=col + (255,))
    d.rounded_rectangle([6 * k * SS, 6 * k * SS, N - 6 * k * SS, N - 6 * k * SS], radius=16 * k * SS, fill=(0, 0, 0, 175))
    return im


def x_glyph(frame):
    """Composite the shaded X (wrench recipe, as in gen_selfheal_textures.make_cross but with diagonal bars) onto a SS x frame."""
    from PIL import Image, ImageDraw, ImageFilter, ImageChops
    S, N = SS, SIZE
    Z = (N * S, N * S)
    c, half, wd = N / 2, X_SPAN / 2, X_WIDTH
    def L(): return Image.new("L", Z, 0)
    im = frame
    def put(col, mask):
        nonlocal im
        lay = Image.new("RGBA", Z, col + (255,))
        lay.putalpha(mask)
        im = Image.alpha_composite(im, lay)
    def cap_line(dr, p0, p1, w, fill=255):
        dr.line([(p0[0] * S, p0[1] * S), (p1[0] * S, p1[1] * S)], fill=fill, width=int(w * S))
        for q in (p0, p1):
            dr.ellipse([(q[0] - w / 2) * S, (q[1] - w / 2) * S, (q[0] + w / 2) * S, (q[1] + w / 2) * S], fill=fill)
    def xmask(w, reach):
        m = L(); md = ImageDraw.Draw(m)
        cap_line(md, (c - reach, c - reach), (c + reach, c + reach), w)
        cap_line(md, (c - reach, c + reach), (c + reach, c - reach), w)
        return m
    m = xmask(wd, half - wd * 0.35)
    def vgrad(top, bot):
        g = Image.new("RGBA", Z); gp = g.load()
        y0, y1 = (c - half) * S, (c + half) * S
        for y in range(Z[1]):
            t = min(1.0, max(0.0, (y - y0) / (y1 - y0)))
            col = tuple(int(top[i] + (bot[i] - top[i]) * t) for i in range(3)) + (255,)
            for x in range(Z[0]): gp[x, y] = col
        return g
    edge = lambda mk, dx, dy: ImageChops.subtract(mk, ImageChops.offset(mk, int(dx * S), int(dy * S)))
    outline = m.filter(ImageFilter.MaxFilter(2 * S + 1))
    glow = m.filter(ImageFilter.MaxFilter(4 * S + 1)).filter(ImageFilter.GaussianBlur(S * 2.2)).point(lambda v: int(v * 0.55))
    put((60, 210, 175), glow)
    put((6, 60, 50), outline)
    im.paste(vgrad((175, 252, 228), (28, 150, 122)), (0, 0), m)
    put((10, 90, 74), edge(m, -2.4, -2.4).filter(ImageFilter.GaussianBlur(S * 0.7)))    # lower-right shade
    put((235, 255, 246), edge(m, 2.2, 2.2).filter(ImageFilter.GaussianBlur(S * 0.6)))    # upper-left rim light
    ch = xmask(2.8, half - 4.5)   # recessed centre channel along each bar
    im.paste(vgrad((52, 160, 134), (12, 92, 76)), (0, 0), ch)
    put((5, 60, 50), edge(ch, 1.3, 1.3).filter(ImageFilter.GaussianBlur(S * 0.4)))       # inner shadow (top-left)
    put((170, 244, 222), edge(ch, -1.1, -1.1).filter(ImageFilter.GaussianBlur(S * 0.4)))  # lower-right lip
    hl = L(); hd = ImageDraw.Draw(hl)
    hd.line([((c - half + 3) * S, (c - half + 2) * S), ((c - half + 6) * S, (c - half + 5) * S)], fill=190, width=int(1.1 * S))   # rim streak, upper-left tip
    gx, gy = c + half - 4.5, c - half + 4.5
    hd.ellipse([(gx - 1.5) * S, (gy - 1.5) * S, (gx + 1.5) * S, (gy + 1.5) * S], fill=230)   # glint, upper-right tip
    put((245, 255, 250), ImageChops.multiply(hl, m).filter(ImageFilter.GaussianBlur(S * 0.5)))
    return im


def disc_img():
    """Dark backing disc under the cross: (0,0,0,175) fill, 2 px (10,96,80) rim, radius == RING_RADIUS, untinted."""
    from PIL import Image
    S, N = 8, DISC_SIZE
    c = N / 2
    im = Image.new("RGBA", (N, N), (0, 0, 0, 0))
    px = im.load()
    for y in range(N):
        for x in range(N):
            ar = ag = ab = aa = 0.0
            for sy in range(S):
                for sx in range(S):
                    r = math.hypot(x + (sx + 0.5) / S - c, y + (sy + 0.5) / S - c)
                    if r > DISC_RADIUS:
                        continue
                    col, al = ((10, 96, 80), 255.0) if r > DISC_RADIUS - DISC_RIM else ((0, 0, 0), 175.0)
                    ar += col[0] * al; ag += col[1] * al; ab += col[2] * al; aa += al
            if aa:
                px[x, y] = (int(round(ar / aa)), int(round(ag / aa)), int(round(ab / aa)), int(round(aa / (S * S))))
    return im


def make_all(out: Path) -> list[Path]:
    from PIL import Image
    made = []
    for name, im in ((FRAME, frame_img(BORDER)), (FRAME_HI, frame_img(BORDER_HI)),
                     (CANCEL, x_glyph(frame_img(BORDER))), (CANCEL_HI, x_glyph(frame_img(BORDER_HI)))):
        p = out / f"{name}.tga"
        im.resize((SIZE, SIZE), Image.LANCZOS if name.startswith("kit_frame") else Image.BOX).save(p, format="TGA")
        made.append(p)
    p = out / f"{DISC}.tga"
    disc_img().save(p, format="TGA")
    made.append(p)
    return made


def render_inc(blobs) -> str:
    out = ["// GENERATED by scripts/gen_hud_kit_textures.py -- do not edit. Uncompressed 32-bit TGA images served from memory by the DLLs.",
           "// Requires embtex::Tex from _shared_src/EmbedTex.h (include it before this file).",
           ""]
    for name, data in blobs:
        out.append(f"static const unsigned char HKTex_{name}[{len(data)}] = {{")
        for i in range(0, len(data), 24):
            out.append("    " + ",".join(f"0x{b:02x}" for b in data[i:i + 24]) + ",")
        out.append("};")
    out.append("")
    out.append("static const embtex::Tex kHudKitTextures[] = {")
    for name, data in blobs:
        out.append(f'    {{ "{name}", HKTex_{name}, {len(data)}u }},')
    out.append("};")
    out.append(f"static const unsigned kHudKitTextureCount = {len(blobs)}u;")
    return "\n".join(out) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out-dir", type=Path)
    a = ap.parse_args()
    with tempfile.TemporaryDirectory() as td:
        made = make_all(Path(td))
        blobs = [(p.stem, p.read_bytes()) for p in made]
        if a.out_dir:
            a.out_dir.mkdir(parents=True, exist_ok=True)
            for name, data in blobs:
                (a.out_dir / f"{name}.tga").write_bytes(data)
    INC.write_text(render_inc(blobs))
    print(f"wrote {INC} ({', '.join(f'{n} {len(d)} B' for n, d in blobs)})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
