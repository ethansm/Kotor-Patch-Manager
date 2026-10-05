#!/usr/bin/env python3
"""Generate the Companion Self-Heal 0.4.0 textures and embed them in the DLL source.

Writes patch_manager_mods/companion_selfheal_src/SelfHealTextures.inc: one uncompressed 32-bit TGA byte array per texture.
  csh_cross                32x32 plus-shaped cross v2: light->dark gradient, bevel, seeded noise, dark outline (engine tints it)
  csh_ring0 .. csh_ring7   48x48 busy-ring animation, 8 frames = one revolution: faint full ring + bright comet arc
                           (~70 deg, alpha 255 at the head fading to 0 along the tail) + soft sparkle dot at the head.
                           ANGLE CONVENTION: 0 deg = up (12 o'clock), increasing clockwise; the head of frame k is at
                           k*45 deg and the tail trails counter-clockwise behind it. All white; the engine tints gold.
  csh_disc                 48x48 backing disc (0,0,0,175) with 2 px (10,96,80) rim, radius = ring radius, drawn untinted (LAST entry)
Deterministic: fixed-seed noise only, no timestamps. Re-run after changing the art, then rebuild the patch.

The .inc uses the selfheal-private names SelfHealEmbeddedTexture / kSelfHealTextures / kSelfHealTextureCount so it can
be included next to HotbarTextures.inc (which defines struct EmbeddedTexture) without a redefinition clash.

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
RING_FRAMES = 8
RING = tuple(f"csh_ring{k}" for k in range(RING_FRAMES))
DISC = "csh_disc"
ALL_NAMES = (CROSS,) + RING + (DISC,)   # disc appended last so existing indices stay valid
RING_SIZE, CROSS_SIZE = 48, 32
RING_RADIUS, RING_WIDTH = 21.0, 2.5
ARC_DEG = 70.0
DISC_RADIUS, DISC_RIM = RING_RADIUS, 2.0


def _cross_mask(grow: float, S: int = 4):
    """Coverage (0..1) per final pixel of the rounded plus shape grown by `grow` px (supersampled)."""
    from PIL import Image, ImageDraw
    N = CROSS_SIZE
    im = Image.new("L", (N * S, N * S), 0)
    d = ImageDraw.Draw(im)
    c, arm = N / 2, N * 0.30 / 2   # arm width ~30% of the side
    h, a, r = N / 2 - 1.5 + grow, arm + grow, 2.0 + grow
    for box in ((c - h, c - a, c + h, c + a), (c - a, c - h, c + a, c + h)):
        d.rounded_rectangle([box[0] * S, box[1] * S, box[2] * S - 1, box[3] * S - 1], radius=r * S, fill=255)
    im = im.resize((N, N), Image.BOX)
    px = im.load()
    return [[px[x, y] / 255.0 for x in range(N)] for y in range(N)]


def make_cross(out: Path) -> list[Path]:
    """v2: vertical light->dark gradient body (255 -> ~170, engine tints it), 1 px bevel highlight (top-left) and shadow
    (bottom-right), small seeded noise, dark (0,0,0,200) outline."""
    import random
    from PIL import Image
    N = CROSS_SIZE
    body, outl = _cross_mask(0.0), _cross_mask(1.5)
    rng = random.Random(0x5E1F)
    im = Image.new("RGBA", (N, N), (0, 0, 0, 0))
    px = im.load()
    y0, y1 = 1.5, N - 1.5   # body vertical extent
    cov = lambda m, x, y: m[y][x] if 0 <= x < N and 0 <= y < N else 0.0
    for y in range(N):
        for x in range(N):
            noise = rng.uniform(-4.0, 4.0)
            ba, oa = body[y][x], outl[y][x] * 200.0 / 255.0
            lum = 255.0 - 85.0 * min(1.0, max(0.0, (y + 0.5 - y0) / (y1 - y0)))
            if ba > 0.5:
                if cov(body, x - 1, y) < 0.5 or cov(body, x, y - 1) < 0.5:
                    lum = min(255.0, lum + 18.0)    # top-left bevel highlight
                elif cov(body, x + 1, y) < 0.5 or cov(body, x, y + 1) < 0.5:
                    lum -= 38.0                     # bottom-right bevel shadow
            lum = max(0.0, min(255.0, lum + noise))
            a = ba + oa * (1.0 - ba)
            if a <= 0:
                continue
            col = lum * ba / a   # outline colour is black, so it only darkens
            px[x, y] = (int(round(col)),) * 3 + (int(round(a * 255.0)),)
    p = out / f"{CROSS}.tga"
    im.save(p, format="TGA")
    return [p]


def make_disc(out: Path) -> list[Path]:
    """Dark backing disc under the cross: (0,0,0,175) fill, 2 px (10,96,80) rim, radius == RING_RADIUS, untinted."""
    from PIL import Image
    S, N = 8, RING_SIZE
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
    p = out / f"{DISC}.tga"
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


def make_all(out: Path) -> list[Path]:
    return make_cross(out) + make_ring_textures(out) + make_disc(out)


def render_inc(blobs) -> str:
    out = ["// GENERATED by scripts/gen_selfheal_textures.py -- do not edit. Uncompressed 32-bit TGA images served from memory by the DLL.",
           "// Selfheal-private names (SelfHealEmbeddedTexture / kSelfHealTextures) so this can coexist with HotbarTextures.inc's EmbeddedTexture.",
           ""]
    for name, data in blobs:
        out.append(f"static const unsigned char SHTex_{name}[{len(data)}] = {{")
        for i in range(0, len(data), 24):
            out.append("    " + ",".join(f"0x{b:02x}" for b in data[i:i + 24]) + ",")
        out.append("};")
    out.append("")
    out.append("struct SelfHealEmbeddedTexture { const char* resref; const unsigned char* data; unsigned size; };")
    out.append("static const SelfHealEmbeddedTexture kSelfHealTextures[] = {")
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
