#!/usr/bin/env python3
"""Generate the Companion Self-Heal 0.4.0 textures and embed them in the DLL source.

Writes patch_manager_mods/companion_selfheal_src/SelfHealTextures.inc: one uncompressed 32-bit TGA byte array per texture.
  csh_cross                32x32 cross v3: wrench-family mint->teal shaded plus, ~22 px arm span, rounded ends (drawn untinted)
  csh_disc                 48x48 circular button backdrop: (0,0,0,175) fill, 2 px (10,96,80) rim, radius 21 of 24 (centre 24,24),
                           8x8 supersampled, transparent outside ~22 px.
  csh_disc_hi              48x48 same shape and fill, beige (208,204,170) rim, plus a soft outer halo (same colour, alpha 200 just
                           outside r=21 fading linearly to 0 at r=24). Radial only, so exactly circular.
  csh_tog_off              128x32 rounded-pill toggle track (radius 16), (0,0,0,175) fill, 2 px (60,90,85) rim, grey (150,160,155)
                           knob r=12 at the LEFT cap centre (16,16). 8x8 supersampled, corners transparent.
  csh_tog_off_hi           same knob and fill; beige (208,204,170) rim; pill inset 3 px (radius 13) so a 2 px faint beige halo
                           (alpha 110 at the edge fading to 0) stays inside the canvas.
  csh_tog_on               as csh_tog_off with a (10,96,80) rim, a faint mint inner glow (4 px inside the rim) and a mint
                           (140,255,215) knob at the RIGHT cap centre (112,16).
  csh_tog_on_hi            csh_tog_on with the csh_tog_off_hi beige rim and halo.
  csh_ring0 .. csh_ring7   48x48 busy-ring animation, 8 frames = one revolution: faint full ring + bright comet arc
                           (~70 deg, alpha 255 at the head fading to 0 along the tail) + soft sparkle dot at the head.
                           ANGLE CONVENTION: 0 deg = up (12 o'clock), increasing clockwise; the head of frame k is at
                           k*45 deg and the tail trails counter-clockwise behind it. All white; the engine tints gold.
  (the shared HUD kit keeps its own kit_disc: scripts/gen_hud_kit_textures.py -> _shared_src/HudKitTextures.inc)
Deterministic: fixed-seed noise only, no timestamps. Re-run after changing the art, then rebuild the patch.

The .inc uses embtex::Tex (EmbedTex.h) and the names kSelfHealTextures / kSelfHealTextureCount.

  --out-dir DIR   also write the .tga files to DIR (for inspection)
"""
import argparse
import math
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
INC = REPO / "patch_manager_mods/companion_selfheal_src/SelfHealTextures.inc"
CROSS = "csh_cross"
DISC, DISC_HI = "csh_disc", "csh_disc_hi"
TOG_OFF, TOG_OFF_HI, TOG_ON, TOG_ON_HI = "csh_tog_off", "csh_tog_off_hi", "csh_tog_on", "csh_tog_on_hi"
TOGGLES = (TOG_OFF, TOG_OFF_HI, TOG_ON, TOG_ON_HI)
RING_FRAMES = 8
RING = tuple(f"csh_ring{k}" for k in range(RING_FRAMES))
ALL_NAMES = (CROSS, DISC, DISC_HI) + TOGGLES + RING
RING_SIZE, CROSS_SIZE, DISC_SIZE = 48, 32, 48
TOG_W, TOG_H = 128, 32
TOG_RIM = 2.0                              # rim thickness, every toggle variant
TOG_RIM_OFF, TOG_RIM_ON = (60, 90, 85), (10, 96, 80)
TOG_FILL = (0, 0, 0, 175)
TOG_KNOB_R, TOG_KNOB_Y = 12.0, TOG_H / 2
TOG_KNOB_X_OFF, TOG_KNOB_X_ON = 16.0, 112.0   # knob centres = cap centres of the full-size pill (radius 16)
TOG_KNOB_OFF, TOG_KNOB_ON = (150, 160, 155), (140, 255, 215)
TOG_GLOW_RGB, TOG_GLOW_W, TOG_GLOW_PEAK = (40, 210, 170), 4.0, 90.0   # ON inner glow, starts just inside the rim, fades inward
TOG_HI_INSET = 3.0                         # *_hi pills are inset so the halo has room inside the 128x32 canvas
TOG_HALO_W, TOG_HALO_PEAK = 2.0, 110.0     # *_hi halo: alpha TOG_HALO_PEAK at the pill edge, 0 at TOG_HALO_W outside it
CROSS_SPAN = 22   # tip-to-tip arm span of the cross glyph, px
RING_RADIUS, RING_WIDTH = 21.0, 2.5
ARC_DEG = 70.0
DISC_RADIUS, DISC_RIM = 21.0, 2.0          # same radius as the busy ring
DISC_FILL = (0, 0, 0, 175)
DISC_RIM_RGB = (10, 96, 80)
HI_RGB = (208, 204, 170)                   # csh_disc_hi rim and halo colour
HALO_END, HALO_PEAK = 24.0, 200.0          # csh_disc_hi halo: alpha HALO_PEAK just outside DISC_RADIUS, 0 at HALO_END


def make_cross(out: Path) -> list[Path]:
    """v3: wrench-family cross (recipe ported from gen_hotbar_textures.make_edit_textures): mint->teal gradient body, dark outline, soft glow,
    lower-right shade, upper-left rim light, recessed centre channel along each arm (inner shadow + lip), specular glint. Rounded arm ends,
    arm span CROSS_SPAN px, centred. Drawn untinted by the engine."""
    from PIL import Image, ImageDraw, ImageFilter, ImageChops
    S, N = 4, CROSS_SIZE
    Z = (N * S, N * S)
    c, half, wd = N / 2, CROSS_SPAN / 2, 8.0
    def L(): return Image.new("L", Z, 0)
    im = Image.new("RGBA", Z, (60, 210, 175, 0))
    def put(col, mask):
        nonlocal im
        lay = Image.new("RGBA", Z, col + (255,))
        lay.putalpha(mask)
        im = Image.alpha_composite(im, lay)
    def cap_line(dr, p0, p1, w, fill=255):
        dr.line([(p0[0] * S, p0[1] * S), (p1[0] * S, p1[1] * S)], fill=fill, width=int(w * S))
        for q in (p0, p1):
            dr.ellipse([(q[0] - w / 2) * S, (q[1] - w / 2) * S, (q[0] + w / 2) * S, (q[1] + w / 2) * S], fill=fill)
    def plus(w, reach):
        m = L(); md = ImageDraw.Draw(m)
        cap_line(md, (c - reach, c), (c + reach, c), w)
        cap_line(md, (c, c - reach), (c, c + reach), w)
        return m
    m = plus(wd, half - wd / 2)
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
    ch = plus(3.4, half - 4.5)   # recessed centre channel along each arm
    im.paste(vgrad((52, 160, 134), (12, 92, 76)), (0, 0), ch)
    put((5, 60, 50), edge(ch, 1.3, 1.3).filter(ImageFilter.GaussianBlur(S * 0.4)))       # inner shadow (top-left)
    put((170, 244, 222), edge(ch, -1.1, -1.1).filter(ImageFilter.GaussianBlur(S * 0.4)))  # lower-right lip
    hl = L(); hd = ImageDraw.Draw(hl)
    hd.line([((c - half + 4) * S, (c - 3.4) * S), ((c - 4.5) * S, (c - 3.4) * S)], fill=190, width=int(1.1 * S))   # rim streak, left arm
    gx, gy = c - 2.8, c - half + 3.5
    hd.ellipse([(gx - 1.5) * S, (gy - 1.5) * S, (gx + 1.5) * S, (gy + 1.5) * S], fill=230)   # glint, top arm
    put((245, 255, 250), ImageChops.multiply(hl, m).filter(ImageFilter.GaussianBlur(S * 0.5)))
    im = im.resize((N, N), Image.BOX)
    p = out / f"{CROSS}.tga"
    im.save(p, format="TGA")
    return [p]


def ring_alpha(x: float, y: float, k: int) -> int:
    """Alpha (0..255) of ring frame k at pixel-space point (x, y)."""
    c = RING_SIZE / 2
    dx, dy = x - c, y - c
    r = math.hypot(dx, dy)
    ang = math.degrees(math.atan2(dx, -dy)) % 360.0   # 0 = up, clockwise
    head = k * 360.0 / RING_FRAMES
    a = 0.0
    if abs(r - RING_RADIUS) <= RING_WIDTH / 2:
        a = 70.0
        delta = (head - ang) % 360.0   # degrees behind the head, counter-clockwise
        if delta <= ARC_DEG:
            a = max(a, 255.0 * (1.0 - delta / ARC_DEG))
    hx = c + RING_RADIUS * math.sin(math.radians(head))
    hy = c - RING_RADIUS * math.cos(math.radians(head))
    dd = (x - hx) ** 2 + (y - hy) ** 2
    a = max(a, 255.0 * math.exp(-dd / (2 * 1.9 ** 2)))   # soft sparkle
    return int(round(min(255.0, a)))


def make_ring_textures(out: Path) -> list[Path]:
    from PIL import Image
    S, N = 4, RING_SIZE
    made = []
    for k in range(RING_FRAMES):
        im = Image.new("RGBA", (N * S, N * S), (255, 255, 255, 0))
        px = im.load()
        for j in range(N * S):
            for i in range(N * S):
                a = ring_alpha((i + 0.5) / S, (j + 0.5) / S, k)
                if a:
                    px[i, j] = (255, 255, 255, a)
        im = im.resize((N, N), Image.LANCZOS)
        p = out / f"{RING[k]}.tga"
        im.save(p, format="TGA")
        made.append(p)
    return made


def disc_sample(r: float, hi: bool) -> tuple[tuple[int, int, int], float]:
    """Colour and alpha (0..255) of the backing disc at r px from the centre (24,24). Depends on r only, so it is exactly circular."""
    if r <= DISC_RADIUS - DISC_RIM:
        return DISC_FILL[:3], float(DISC_FILL[3])
    if r <= DISC_RADIUS:
        return (HI_RGB if hi else DISC_RIM_RGB), 255.0
    if hi and r < HALO_END:
        return HI_RGB, HALO_PEAK * (HALO_END - r) / (HALO_END - DISC_RADIUS)
    return (0, 0, 0), 0.0


def make_disc(out: Path, name: str, hi: bool) -> Path:
    """8x8 supersampled disc (same approach as gen_hud_kit_textures.disc_img): colour averaged weighted by alpha, alpha = mean coverage."""
    from PIL import Image
    S, N = 8, DISC_SIZE
    c = N / 2
    im = Image.new("RGBA", (N, N), (0, 0, 0, 0))
    px = im.load()
    for y in range(N):
        for x in range(N):
            acc = [0.0, 0.0, 0.0]
            aa = 0.0
            for sy in range(S):
                for sx in range(S):
                    r = math.hypot(x + (sx + 0.5) / S - c, y + (sy + 0.5) / S - c)
                    col, al = disc_sample(r, hi)
                    for i in range(3):
                        acc[i] += col[i] * al
                    aa += al
            if aa:
                px[x, y] = (int(round(acc[0] / aa)), int(round(acc[1] / aa)), int(round(acc[2] / aa)), int(round(aa / (S * S))))
    p = out / f"{name}.tga"
    im.save(p, format="TGA")
    return p


def _over(dst, src):
    """Straight-alpha 'over' of two (rgb, alpha) layers, alpha in 0..1."""
    (dc, da), (sc, sa) = dst, src
    oa = sa + da * (1.0 - sa)
    if oa <= 0.0:
        return (0.0, 0.0, 0.0), 0.0
    return tuple((sc[i] * sa + dc[i] * da * (1.0 - sa)) / oa for i in range(3)), oa


def toggle_sample(x: float, y: float, on: bool, hi: bool) -> tuple[tuple[float, float, float], float]:
    """Colour (0..255) and alpha (0..255) of a toggle track at pixel-space point (x, y). Layers bottom-up: halo (hi only), fill,
    ON inner glow, rim, knob. The pill is a capsule (signed distance d, negative inside), so every edge is exact."""
    i = TOG_HI_INSET if hi else 0.0
    x0, x1 = i, TOG_W - i
    y0, y1 = i, TOG_H - i
    R = (y1 - y0) / 2                       # full height / 2 = corner radius
    cy = (y0 + y1) / 2
    qx = min(max(x, x0 + R), x1 - R)        # nearest point on the pill's core segment
    d = math.hypot(x - qx, y - cy) - R
    px = ((0.0, 0.0, 0.0), 0.0)
    if d >= 0:
        if hi and d < TOG_HALO_W:
            px = _over(px, (HI_RGB, TOG_HALO_PEAK / 255.0 * (1.0 - d / TOG_HALO_W)))
    else:
        depth = -d
        if depth < TOG_RIM:
            px = _over(px, (HI_RGB if hi else (TOG_RIM_ON if on else TOG_RIM_OFF), 1.0))
        else:
            px = _over(px, (TOG_FILL[:3], TOG_FILL[3] / 255.0))
            if on and depth < TOG_RIM + TOG_GLOW_W:
                glow = TOG_GLOW_PEAK / 255.0 * (1.0 - (depth - TOG_RIM) / TOG_GLOW_W)
                px = _over(px, (TOG_GLOW_RGB, glow))
    kx = TOG_KNOB_X_ON if on else TOG_KNOB_X_OFF
    if math.hypot(x - kx, y - TOG_KNOB_Y) <= TOG_KNOB_R:
        px = _over(px, (TOG_KNOB_ON if on else TOG_KNOB_OFF, 1.0))
    return px[0], px[1] * 255.0


def make_toggle(out: Path, name: str, on: bool, hi: bool) -> Path:
    """128x32 toggle track, 8x8 supersampled with the same accumulation as make_disc (colour weighted by alpha, alpha = mean coverage)."""
    from PIL import Image
    S = 8
    im = Image.new("RGBA", (TOG_W, TOG_H), (0, 0, 0, 0))
    px = im.load()
    for y in range(TOG_H):
        for x in range(TOG_W):
            acc = [0.0, 0.0, 0.0]
            aa = 0.0
            for sy in range(S):
                for sx in range(S):
                    col, al = toggle_sample(x + (sx + 0.5) / S, y + (sy + 0.5) / S, on, hi)
                    for c in range(3):
                        acc[c] += col[c] * al
                    aa += al
            if aa:
                px[x, y] = (int(round(acc[0] / aa)), int(round(acc[1] / aa)), int(round(acc[2] / aa)), int(round(aa / (S * S))))
    p = out / f"{name}.tga"
    im.save(p, format="TGA")
    return p


def make_toggles(out: Path) -> list[Path]:
    """csh_tog_off, csh_tog_off_hi, csh_tog_on, csh_tog_on_hi (in that order)."""
    return [make_toggle(out, TOG_OFF, False, False), make_toggle(out, TOG_OFF_HI, False, True),
            make_toggle(out, TOG_ON, True, False), make_toggle(out, TOG_ON_HI, True, True)]


def make_all(out: Path) -> list[Path]:
    return (make_cross(out) + [make_disc(out, DISC, False), make_disc(out, DISC_HI, True)]
            + make_toggles(out) + make_ring_textures(out))


def render_inc(blobs) -> str:
    out = ["// GENERATED by scripts/gen_selfheal_textures.py -- do not edit. Uncompressed 32-bit TGA images served from memory by the DLL.",
           "// Requires embtex::Tex { const char* resref; const unsigned char* data; unsigned size; } (EmbedTex.h).",
           ""]
    for name, data in blobs:
        out.append(f"static const unsigned char SHTex_{name}[{len(data)}] = {{")
        for i in range(0, len(data), 24):
            out.append("    " + ",".join(f"0x{b:02x}" for b in data[i:i + 24]) + ",")
        out.append("};")
    out.append("")
    out.append("static const embtex::Tex kSelfHealTextures[] = {")
    for name, data in blobs:
        out.append(f'    {{ "{name}", SHTex_{name}, {len(data)}u }},')
    out.append("};")
    out.append(f"static const unsigned kSelfHealTextureCount = {len(blobs)}u;")
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
