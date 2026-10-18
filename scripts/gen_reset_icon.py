#!/usr/bin/env python3
"""Generate the holster-editor reset-arrow textures (hse_reset, hse_reset_hi) and embed them in the DLL source (stdlib only, deterministic).

Writes patch_manager_mods/_shared_src/ResetIconTex.inc (uncompressed 32-bit TGAs, same layout/format as GearIconTex.inc):
  kResetIconTex / kResetIconHiTex  32x32 counter-clockwise circular arrow, thin line, transparent background
  kResetTex[] / kResetTexCount     embtex::Tex table: hse_reset (teal, the gear/wrench palette) and hse_reset_hi (the gold hover variant)

Art: signed-distance shape (open ring arc + arrowhead triangle), 4x4 supersampled, shaded like the gear/wrench glyphs
(glow, dark outline, mint->teal vertical gradient, lower-right shade, upper-left rim light). Palettes are gen_luo_textures.py
PAL_NORMAL / PAL_HI (HudKit kAccent {10,96,80} is the same teal family).

  (no args)  write the .inc
  --check    regenerate in memory and compare with the existing .inc (exit 0 identical, 1 otherwise)
"""
import argparse
import math
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from gen_shared_icons import tga32   # same TGA writer as GearIconTex.inc  # noqa: E402

REPO = Path(__file__).resolve().parent.parent
INC = REPO / "patch_manager_mods/_shared_src/ResetIconTex.inc"
N = 32
SS = 4                       # supersample per axis
PAL_NORMAL = dict(top=(175, 252, 228), bot=(28, 150, 122), outline=(6, 60, 50), glow=(60, 210, 175), shade=(10, 90, 74), light=(235, 255, 246))
PAL_HI = dict(top=(255, 246, 205), bot=(196, 156, 70), outline=(74, 56, 14), glow=(235, 200, 110), shade=(130, 98, 30), light=(255, 252, 235))
VARIANTS = (("hse_reset", "kResetIconTex", PAL_NORMAL), ("hse_reset_hi", "kResetIconHiTex", PAL_HI))

# Geometry in pixels (y down). The arc runs counter-clockwise on screen from START through the top, left and bottom, ending at
# START + SWEEP (lower right) with the arrowhead pointing back up into the gap on the right.
CX = CY = N / 2.0
R = 10.0                     # ring radius (centre of the stroke)
HW = 1.25                    # stroke half width -> 2.5 px line
START = math.radians(50.0)   # screen-CCW angle (0 = +x, 90 = up)
SWEEP = math.radians(280.0)
HEAD_LEN, HEAD_HALF, HEAD_BACK = 4.5, 4.0, 0.5
OUTLINE, GLOW_SIGMA, GLOW_GAIN, EDGE = 1.0, 1.7, 0.55, 1.6


def _pt(ang, r):
    return (CX + r * math.cos(ang), CY - r * math.sin(ang))


def _tri():
    """Arrowhead triangle at the arc end: tip ahead along the CCW tangent, base straddling the ring just behind the end."""
    e = START + SWEEP
    ex, ey = _pt(e, R)
    tx, ty = -math.sin(e) , -math.cos(e)       # screen-space CCW tangent (y down)
    nx, ny = math.cos(e), -math.sin(e)         # radial outward (y down)
    tip = (ex + tx * HEAD_LEN, ey + ty * HEAD_LEN)
    b0 = (ex - tx * HEAD_BACK + nx * HEAD_HALF, ey - ty * HEAD_BACK + ny * HEAD_HALF)
    b1 = (ex - tx * HEAD_BACK - nx * HEAD_HALF, ey - ty * HEAD_BACK - ny * HEAD_HALF)
    return (tip, b0, b1)


TRI = _tri()
P_START, P_END = _pt(START, R), _pt(START + SWEEP, R)


def _seg_dist(px, py, a, b):
    ax, ay = a
    dx, dy = b[0] - ax, b[1] - ay
    t = max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy)))
    return math.hypot(px - ax - t * dx, py - ay - t * dy)


def _tri_sd(px, py):
    d = min(_seg_dist(px, py, TRI[i], TRI[(i + 1) % 3]) for i in range(3))
    s = []
    for i in range(3):
        a, b = TRI[i], TRI[(i + 1) % 3]
        s.append((b[0] - a[0]) * (py - a[1]) - (b[1] - a[1]) * (px - a[0]))
    inside = all(v >= 0 for v in s) or all(v <= 0 for v in s)
    return -d if inside else d


def sdist(px, py):
    """Signed distance (px; negative inside) to the glyph: stroke arc with round caps, union the arrowhead."""
    dx, dy = px - CX, -(py - CY)
    r = math.hypot(dx, dy)
    ang = (math.atan2(dy, dx) - START) % (2 * math.pi)
    if ang <= SWEEP:
        da = abs(r - R)
    else:
        da = min(math.hypot(px - P_START[0], py - P_START[1]), math.hypot(px - P_END[0], py - P_END[1]))
    return min(da - HW, _tri_sd(px, py))


def _mix(a, b, t):
    return tuple(a[i] + (b[i] - a[i]) * t for i in range(3))


def _over(dst, src_rgb, src_a):
    """Straight-alpha 'over' of (src_rgb, src_a) onto dst = (rgb, a)."""
    rgb, a = dst
    oa = src_a + a * (1 - src_a)
    if oa <= 0:
        return ((0.0, 0.0, 0.0), 0.0)
    return (tuple((src_rgb[i] * src_a + rgb[i] * a * (1 - src_a)) / oa for i in range(3)), oa)


def sample(px, py, pal):
    """One shaded sub-sample -> (rgb, a) in 0..255 / 0..1."""
    d = sdist(px, py)
    out = (pal["glow"], 0.0)
    if d > 0:
        out = _over(out, pal["glow"], GLOW_GAIN * math.exp(-(d * d) / (2 * GLOW_SIGMA * GLOW_SIGMA)))
    if d <= OUTLINE:
        out = _over(out, pal["outline"], 1.0)
    if d <= 0:
        t = min(1.0, max(0.0, (py - 3.0) / (N - 6.0)))
        col = _mix(pal["top"], pal["bot"], t)
        if d > -EDGE:   # rim light on the upper-left edge, shade on the lower-right edge (outward normal from the distance gradient)
            e = 0.25
            gx = sdist(px + e, py) - sdist(px - e, py)
            gy = sdist(px, py + e) - sdist(px, py - e)
            gl = math.hypot(gx, gy)
            if gl > 1e-9:
                facing = (gx * 0.7071 + gy * 0.7071) / gl      # + : outward normal points lower right
                w = (1.0 - (-d) / EDGE) * 0.75 * abs(facing)
                col = _mix(col, pal["shade"] if facing > 0 else pal["light"], w)
        out = _over(out, col, 1.0)
    return out


def render(pal):
    """-> pixel(x, y) callable for tga32 (y=0 top)."""
    px_cache = {}
    n = SS * SS
    for y in range(N):
        for x in range(N):
            acc = [0.0, 0.0, 0.0]
            asum = 0.0
            for sy in range(SS):
                for sx in range(SS):
                    rgb, a = sample(x + (sx + 0.5) / SS, y + (sy + 0.5) / SS, pal)
                    asum += a
                    for k in range(3):
                        acc[k] += rgb[k] * a
            a = asum / n
            rgb = tuple(acc[k] / asum for k in range(3)) if asum > 0 else pal["glow"]
            px_cache[(x, y)] = tuple(max(0, min(255, int(round(c)))) for c in rgb) + (max(0, min(255, int(round(a * 255)))),)
    return lambda x, y: px_cache[(x, y)]


def build():
    """(resref, identifier, bytes) for the two reset textures, in table order."""
    return [(name, ident, tga32(N, N, render(pal))) for name, ident, pal in VARIANTS]


def render_inc(blobs) -> str:
    out = ["// GENERATED by scripts/gen_reset_icon.py -- do not edit. Uncompressed 32-bit TGA images served from memory by the DLL.",
           "// Requires embtex::Tex { const char* resref; const unsigned char* data; unsigned size; } (EmbedTex.h).", ""]
    for _, ident, data in blobs:
        out.append(f"static const unsigned char {ident}[{len(data)}] = {{")
        for i in range(0, len(data), 24):
            out.append("    " + ",".join(f"0x{b:02x}" for b in data[i:i + 24]) + ",")
        out.append("};")
    out.append("")
    out.append("static const embtex::Tex kResetTex[] = {")
    for name, ident, _ in blobs:
        out.append(f'    {{ "{name}", {ident}, sizeof {ident} }},')
    out.append("};")
    out.append(f"static const unsigned kResetTexCount = {len(blobs)};")
    return "\n".join(out) + "\n"


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true")
    a = ap.parse_args(argv)
    blobs = build()
    text = render_inc(blobs)
    if a.check:
        if INC.exists() and INC.read_text() == text:
            return 0
        print(f"{INC} is missing or differs from regenerated output; run scripts/gen_reset_icon.py")
        return 1
    INC.write_text(text)
    print(f"wrote {INC} ({', '.join(f'{n} {len(d)} B' for n, _, d in blobs)})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
