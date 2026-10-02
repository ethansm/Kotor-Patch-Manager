#!/usr/bin/env python3
"""Generate the Force Power Hotbar textures and embed them in the DLL source (Mod 3).

Writes patch_manager_mods/force_power_hotbar_src/HotbarTextures.inc: one uncompressed 32-bit TGA byte array per texture
(fph_slot / fph_slot_hi: 128x128 rounded-square slot frame, teal / beige like the native lbl_mibox00 / 00a;
fph_edit / fph_edit_hi: 72x128 thin frame with a wrench, normal / active). The DLL registers them with the engine at
start-up, so nothing is installed into override/. Re-run after changing the art, then rebuild the patch.

  --out-dir DIR   also write the .tga files to DIR (for inspection)
"""
import argparse
import io
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
INC = REPO / "patch_manager_mods/force_power_hotbar_src/HotbarTextures.inc"
TEX = ("fph_slot", "fph_slot_hi")
EDIT_TEX = ("fph_edit", "fph_edit_hi")


def make_textures(out: Path) -> list[Path]:
    """Rounded-square slot frames, 128x128 RGBA TGA (no TXI: plain alpha blending)."""
    from PIL import Image, ImageDraw
    made = []
    for name, col in ((TEX[0], (0, 44, 26)), (TEX[1], (208, 204, 170))):
        S = 4  # supersample
        im = Image.new("RGBA", (128 * S, 128 * S), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.rounded_rectangle([0, 0, 128 * S - 1, 128 * S - 1], radius=22 * S, fill=col + (255,))
        d.rounded_rectangle([6 * S, 6 * S, 122 * S, 122 * S], radius=16 * S, fill=(0, 0, 0, 175))
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
    for name, col in ((EDIT_TEX[0], (10, 96, 80)), (EDIT_TEX[1], (208, 204, 170))):
        S = 4
        W, H = 72, 128
        im = Image.new("RGBA", (W * S, H * S), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.rounded_rectangle([0, 0, W * S - 1, H * S - 1], radius=22 * S, fill=col + (255,))
        d.rounded_rectangle([3 * S, 3 * S, (W - 3) * S, (H - 3) * S], radius=19 * S, fill=(0, 0, 0, 175))
        # Wrench, shaded like the native force-power icons: mint->teal gradient body, dark outline, soft glow, upper-left rim light and
        # lower-right shade, a RECESSED channel down the handle (raised rim around a darker inset), a U jaw, and a round beveled hanging
        # ring. The hole is drawn last and separately so the directional shading never distorts it.
        Z = (W * S, H * S)
        def L(): return Image.new("L", Z, 0)
        def solid(c): return Image.new("RGBA", Z, c)
        def cap_line(dr, p0, p1, wd, fill=255):
            dr.line([(p0[0] * S, p0[1] * S), (p1[0] * S, p1[1] * S)], fill=fill, width=int(wd * S))
            for q in (p0, p1): dr.ellipse([(q[0] - wd / 2) * S, (q[1] - wd / 2) * S, (q[0] + wd / 2) * S, (q[1] + wd / 2) * S], fill=fill)
        hx, hy, tx, ty = 29, 50, 45, 88   # head centre, tail centre
        ux, uy = tx - hx, ty - hy; ln = math.hypot(ux, uy); ux, uy = ux / ln, uy / ln
        nx, ny = -uy, ux
        ang = math.atan2(uy, ux) + math.pi   # jaw opens away from the handle
        m = L(); md = ImageDraw.Draw(m)
        cap_line(md, (hx, hy), (tx, ty), 9)
        md.ellipse([(hx - 12) * S, (hy - 12) * S, (hx + 12) * S, (hy + 12) * S], fill=255)
        hw, bot_r = 4.6, 3.0   # U jaw: parallel-sided slot with a rounded bottom
        cxs, cys = hx + bot_r * math.cos(ang), hy + bot_r * math.sin(ang)
        px, py = -math.sin(ang), math.cos(ang)
        ex, ey = hx + 20 * math.cos(ang), hy + 20 * math.sin(ang)
        md.polygon([((cxs + px * hw) * S, (cys + py * hw) * S), ((ex + px * hw) * S, (ey + py * hw) * S),
                    ((ex - px * hw) * S, (ey - py * hw) * S), ((cxs - px * hw) * S, (cys - py * hw) * S)], fill=0)
        md.ellipse([(cxs - hw) * S, (cys - hw) * S, (cxs + hw) * S, (cys + hw) * S], fill=0)
        grad = Image.new("RGBA", Z); gp = grad.load()
        top, bot = (175, 252, 228), (28, 150, 122)
        y0, y1 = (hy - 12) * S, (ty + 5) * S
        for y in range(H * S):
            t = min(1.0, max(0.0, (y - y0) / (y1 - y0)))
            c = tuple(int(top[i] + (bot[i] - top[i]) * t) for i in range(3)) + (255,)
            for x in range(W * S): gp[x, y] = c
        outline = m.filter(ImageFilter.MaxFilter(2 * S + 1))
        glow = m.filter(ImageFilter.MaxFilter(4 * S + 1)).filter(ImageFilter.GaussianBlur(S * 2.2)).point(lambda v: int(v * 0.55))
        im.paste(solid((60, 210, 175, 255)), (0, 0), glow)
        d.bitmap((0, 0), outline, fill=(6, 60, 50, 255))
        im.paste(grad, (0, 0), m)
        edge = lambda mk, dx, dy: ImageChops.subtract(mk, ImageChops.offset(mk, int(dx * S), int(dy * S)))
        im.paste(solid((10, 90, 74, 255)), (0, 0), edge(m, -2.4, -2.4).filter(ImageFilter.GaussianBlur(S * 0.7)))   # lower-right shade
        im.paste(solid((235, 255, 246, 255)), (0, 0), edge(m, 2.2, 2.2).filter(ImageFilter.GaussianBlur(S * 0.6)))   # upper-left rim light
        # recessed handle channel: darker inset with a shadow on its upper-left inner edge and a light lip on its lower-right
        ch = L(); cap_line(ImageDraw.Draw(ch), (hx + ux * 15, hy + uy * 15), (tx - ux * 8, ty - uy * 8), 4.6)
        cg = Image.new("RGBA", Z); cgp = cg.load()
        ctop, cbot = (52, 160, 134), (12, 92, 76)
        for y in range(H * S):
            t = min(1.0, max(0.0, (y - y0) / (y1 - y0)))
            c = tuple(int(ctop[i] + (cbot[i] - ctop[i]) * t) for i in range(3)) + (255,)
            for x in range(W * S): cgp[x, y] = c
        im.paste(cg, (0, 0), ch)
        im.paste(solid((5, 60, 50, 255)), (0, 0), edge(ch, 1.3, 1.3).filter(ImageFilter.GaussianBlur(S * 0.4)))      # inner shadow (top-left)
        im.paste(solid((170, 244, 222, 255)), (0, 0), edge(ch, -1.1, -1.1).filter(ImageFilter.GaussianBlur(S * 0.4)))  # lower-right lip
        # specular streak on the raised rim + glint on the jaw
        hl = L(); hd = ImageDraw.Draw(hl)
        a0 = (hx + ux * 15 - nx * 3.4, hy + uy * 15 - ny * 3.4); a1 = (tx - ux * 6 - nx * 3.4, ty - uy * 6 - ny * 3.4)
        hd.line([(a0[0] * S, a0[1] * S), (a1[0] * S, a1[1] * S)], fill=190, width=int(1.1 * S))
        gx, gy = hx - 6.5, hy - 5.5
        hd.ellipse([(gx - 2.2) * S, (gy - 2.2) * S, (gx + 2.2) * S, (gy + 2.2) * S], fill=230)
        hl = ImageChops.multiply(hl, m)
        im.paste(solid((245, 255, 250, 255)), (0, 0), hl.filter(ImageFilter.GaussianBlur(S * 0.5)))
        # hanging hole: symmetric beveled ring (dark bezel, light lip on the lower-right, see-through centre)
        for r, col in ((4.0, (6, 60, 50, 255)), (3.4, (170, 244, 222, 255)), (2.8, (10, 80, 66, 255)), (2.2, (0, 0, 0, 175))):
            if r == 2.8:
                d.ellipse([(tx - r) * S, (ty - r) * S, (tx + r) * S, (ty + r) * S], fill=col)
            else:
                d.ellipse([(tx - r) * S, (ty - r) * S, (tx + r) * S, (ty + r) * S], fill=col)
        # centre must show the frame's dark interior, not paint over it
        im = im.resize((W, H), Image.LANCZOS)
        p = out / f"{name}.tga"
        im.save(p, format="TGA")
        made.append(p)
    return made


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out-dir", type=Path)
    a = ap.parse_args()
    import tempfile
    with tempfile.TemporaryDirectory() as td:
        made = make_textures(Path(td)) + make_edit_textures(Path(td))
        blobs = [(p.stem, p.read_bytes()) for p in made]
        if a.out_dir:
            a.out_dir.mkdir(parents=True, exist_ok=True)
            for name, data in blobs:
                (a.out_dir / f"{name}.tga").write_bytes(data)
    out = ["// GENERATED by scripts/gen_hotbar_textures.py -- do not edit. Uncompressed 32-bit TGA images served from memory by the DLL.", ""]
    for name, data in blobs:
        out.append(f"static const unsigned char Tex_{name}[{len(data)}] = {{")
        for i in range(0, len(data), 24):
            out.append("    " + ",".join(f"0x{b:02x}" for b in data[i:i + 24]) + ",")
        out.append("};")
    out.append("")
    out.append("struct EmbeddedTexture { const char* resref; const unsigned char* data; unsigned size; };")
    out.append("static const EmbeddedTexture EmbeddedTextures[] = {")
    for name, data in blobs:
        out.append(f'    {{ "{name}", Tex_{name}, {len(data)}u }},')
    out.append("};")
    INC.write_text("\n".join(out) + "\n")
    print(f"wrote {INC} ({', '.join(f'{n} {len(d)} B' for n, d in blobs)})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
