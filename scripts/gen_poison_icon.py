#!/usr/bin/env python3
"""Generate the Buff HUD poison debuff icon and embed it in the DLL source (stdlib only, deterministic).

effecticon.2da row 110 TRAP_EFFECT_POISON points at the placeholder `ip_resistenergy` (no such texture), so the engine draws its red
debuff arrow for poison. The Buff HUD maps that row to this icon instead (EffectIcons.h k2fx::g_trapPoisonIcon).

Writes patch_manager_mods/buff_duration_hud_src/PoisonIconTex.inc (uncompressed 32-bit TGA, same layout/format as ResetIconTex.inc):
  kPoisonIconTex             64x64 green droplet with a soft glow on a transparent background (the vanilla ip_* debuff look:
                             glowing silhouette, dark outline, vertical gradient, rim light upper left, shade lower right, glossy highlight)
  kPoisonTex[] / kPoisonTexCount   embtex::Tex table: bh_poison

  (no args)  write the .inc
  --check    regenerate in memory and compare with the existing .inc (exit 0 identical, 1 otherwise)
  --png P    also write a preview PNG (needs Pillow; the .inc itself is stdlib only)
"""
import argparse
import math
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from gen_shared_icons import tga32   # same TGA writer as GearIconTex.inc / ResetIconTex.inc  # noqa: E402

REPO = Path(__file__).resolve().parent.parent
INC = REPO / "patch_manager_mods/buff_duration_hud_src/PoisonIconTex.inc"
N = 64
SS = 4                       # supersample per axis
OUTLINE = 1.6                # dark outline width (px, outside the shape)
EDGE = 4.0                   # rim light / shade band inside the edge
GLOW_SIGMA = 4.5
GLOW_GAIN = 0.85
PAL = {"glow": (60, 230, 40), "outline": (6, 34, 4), "top": (190, 255, 120), "bot": (28, 120, 14),
       "light": (235, 255, 200), "shade": (8, 60, 6), "gloss": (245, 255, 230)}

# droplet: circle (centre CX, CY, radius R) joined to an apex above it by the two tangent lines
CX, CY, R = 32.0, 40.0, 15.5
AX, AY = 32.0, 8.0


def _tangent_points():
    dx, dy = AX - CX, AY - CY
    dist = math.hypot(dx, dy)
    base = math.atan2(dy, dx)
    off = math.acos(R / dist)
    return [(CX + R * math.cos(base + s * off), CY + R * math.sin(base + s * off)) for s in (1, -1)]


T1, T2 = _tangent_points()
POLY = [(AX, AY), T1, (CX, CY), T2]   # convex kite: apex, tangent, centre, tangent


def _sd_poly(px, py, v):
    """Signed distance to a simple polygon (negative inside)."""
    d = (px - v[0][0]) ** 2 + (py - v[0][1]) ** 2
    s = 1.0
    j = len(v) - 1
    for i in range(len(v)):
        ex, ey = v[j][0] - v[i][0], v[j][1] - v[i][1]
        wx, wy = px - v[i][0], py - v[i][1]
        t = max(0.0, min(1.0, (wx * ex + wy * ey) / (ex * ex + ey * ey)))
        bx, by = wx - ex * t, wy - ey * t
        d = min(d, bx * bx + by * by)
        c1, c2, c3 = py >= v[i][1], py < v[j][1], ex * wy > ey * wx
        if (c1 and c2 and c3) or (not c1 and not c2 and not c3):
            s = -s
        j = i
    return s * math.sqrt(d)


def sdist(px, py):
    return min(math.hypot(px - CX, py - CY) - R, _sd_poly(px, py, POLY))


def _mix(a, b, t):
    return tuple(a[i] + (b[i] - a[i]) * t for i in range(3))


def _over(dst, src_rgb, src_a):
    """Straight-alpha 'over' of (src_rgb, src_a) onto dst = (rgb, a)."""
    rgb, a = dst
    oa = src_a + a * (1 - src_a)
    if oa <= 0:
        return ((0.0, 0.0, 0.0), 0.0)
    return (tuple((src_rgb[i] * src_a + rgb[i] * a * (1 - src_a)) / oa for i in range(3)), oa)


def sample(px, py):
    """One shaded sub-sample -> (rgb, a) in 0..255 / 0..1."""
    d = sdist(px, py)
    out = (PAL["glow"], 0.0)
    if d > 0:
        out = _over(out, PAL["glow"], GLOW_GAIN * math.exp(-(d * d) / (2 * GLOW_SIGMA * GLOW_SIGMA)))
    if d <= OUTLINE:
        out = _over(out, PAL["outline"], 1.0)
    if d <= 0:
        t = min(1.0, max(0.0, (py - AY) / (CY + R - AY)))
        col = _mix(PAL["top"], PAL["bot"], t)
        if d > -EDGE:   # rim light on the upper-left edge, shade on the lower-right edge (outward normal from the distance gradient)
            e = 0.25
            gx = sdist(px + e, py) - sdist(px - e, py)
            gy = sdist(px, py + e) - sdist(px, py - e)
            gl = math.hypot(gx, gy)
            if gl > 1e-9:
                facing = (gx * 0.7071 + gy * 0.7071) / gl      # + : outward normal points lower right
                w = (1.0 - (-d) / EDGE) * 0.75 * abs(facing)
                col = _mix(col, PAL["shade"] if facing > 0 else PAL["light"], w)
        # glossy highlight: a soft tilted ellipse in the upper-left of the bulb
        hx, hy = px - (CX - 6.5), py - (CY - 5.0)
        ca, sa = math.cos(-0.6), math.sin(-0.6)
        u, v = hx * ca - hy * sa, hx * sa + hy * ca
        g = (u / 3.2) ** 2 + (v / 6.0) ** 2
        if g < 1.0:
            col = _mix(col, PAL["gloss"], 0.85 * (1.0 - g))
        out = _over(out, col, 1.0)
    return out


def render():
    """-> pixel(x, y) callable for tga32 (y=0 top)."""
    px_cache = {}
    n = SS * SS
    for y in range(N):
        for x in range(N):
            acc = [0.0, 0.0, 0.0]
            asum = 0.0
            for sy in range(SS):
                for sx in range(SS):
                    rgb, a = sample(x + (sx + 0.5) / SS, y + (sy + 0.5) / SS)
                    asum += a
                    for k in range(3):
                        acc[k] += rgb[k] * a
            a = asum / n
            rgb = tuple(acc[k] / asum for k in range(3)) if asum > 0 else PAL["glow"]
            px_cache[(x, y)] = tuple(max(0, min(255, int(round(c)))) for c in rgb) + (max(0, min(255, int(round(a * 255)))),)
    return lambda x, y: px_cache[(x, y)]


def build():
    """(resref, identifier, bytes, pixel) for the icon texture."""
    pixel = render()
    return [("bh_poison", "kPoisonIconTex", tga32(N, N, pixel), pixel)]


def render_inc(blobs) -> str:
    out = ["// GENERATED by scripts/gen_poison_icon.py -- do not edit. Uncompressed 32-bit TGA image served from memory by the DLL.",
           "// Requires embtex::Tex { const char* resref; const unsigned char* data; unsigned size; } (EmbedTex.h).", ""]
    for _, ident, data, _ in blobs:
        out.append(f"static const unsigned char {ident}[{len(data)}] = {{")
        for i in range(0, len(data), 24):
            out.append("    " + ",".join(f"0x{b:02x}" for b in data[i:i + 24]) + ",")
        out.append("};")
    out.append("")
    out.append("static const embtex::Tex kPoisonTex[] = {")
    for name, ident, _, _ in blobs:
        out.append(f'    {{ "{name}", {ident}, sizeof {ident} }},')
    out.append("};")
    out.append(f"static const unsigned kPoisonTexCount = {len(blobs)};")
    return "\n".join(out) + "\n"


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--png", type=Path)
    a = ap.parse_args(argv)
    blobs = build()
    text = render_inc(blobs)
    if a.png:
        from PIL import Image
        im = Image.new("RGBA", (N, N))
        im.putdata([blobs[0][3](x, y) for y in range(N) for x in range(N)])
        im.save(a.png)
    if a.check:
        if INC.exists() and INC.read_text() == text:
            return 0
        print(f"{INC} is missing or differs from regenerated output; run scripts/gen_poison_icon.py")
        return 1
    INC.write_text(text)
    print(f"wrote {INC} ({', '.join(f'{n} {len(d)} B' for n, _, d, _ in blobs)})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
