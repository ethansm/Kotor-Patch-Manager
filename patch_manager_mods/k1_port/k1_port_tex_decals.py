#!/usr/bin/env python3
"""Procedural decal alpha masks for the texture-diversity overlay (P1 U1).

Twelve shapes inspired by sci-fi insignia motifs; original geometry, no copied artwork. Each shape is drawn with PIL
at 4x supersample and reduced with LANCZOS to a float32 (size, size) alpha mask in [0, 1], row 0 = top of the image.
Pure geometry with no RNG, so output is deterministic. render_decal() rotates with PIL BICUBIC about the centre and
keeps the shape size (corners outside the source frame are alpha 0).
"""
import functools
import math
import sys
from pathlib import Path
from typing import Callable

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import numpy as np  # noqa: E402
from PIL import Image, ImageDraw  # noqa: E402

SUPERSAMPLE = 4
PaintFn = Callable[[ImageDraw.ImageDraw, float], None]


def _px(s: float, pts):
    """Map unit-square points in [-1, 1] to canvas pixels; s is the half-canvas size in pixels."""
    return [(s + u * s, s + v * s) for u, v in pts]


def _poly(draw, s: float, pts, fill: int = 255) -> None:
    draw.polygon(_px(s, pts), fill=fill)


def _disc(draw, s: float, r: float, fill: int = 255, cx: float = 0.0, cy: float = 0.0) -> None:
    draw.ellipse([s + (cx - r) * s, s + (cy - r) * s, s + (cx + r) * s, s + (cy + r) * s], fill=fill)


def _line(draw, s: float, pts, width: float) -> None:
    draw.line(_px(s, pts), fill=255, width=max(1, round(width * s)), joint="curve")


def _lens(p0, p1, half_w: float, n: int = 40):
    """Closed leaf/slit outline from p0 to p1 with maximum half-thickness half_w at mid-length."""
    (x0, y0), (x1, y1) = p0, p1
    dx, dy = x1 - x0, y1 - y0
    length = math.hypot(dx, dy)
    nx, ny = -dy / length, dx / length
    top, bot = [], []
    for i in range(n + 1):
        t = i / n
        bulge = math.sin(math.pi * t) * half_w
        cx, cy = x0 + dx * t, y0 + dy * t
        top.append((cx + nx * bulge, cy + ny * bulge))
        bot.append((cx - nx * bulge, cy - ny * bulge))
    return top + bot[::-1]


def _radial_rect(r0: float, r1: float, half_t: float, theta: float):
    """Rectangle spanning radius r0..r1 and tangential +-half_t at angle theta."""
    c, sn = math.cos(theta), math.sin(theta)
    return [(r * c - t * sn, r * sn + t * c) for r, t in ((r0, -half_t), (r1, -half_t), (r1, half_t), (r0, half_t))]


def _paint_cog8(draw, s):
    pts = []
    n = 8 * 16  # eight teeth, 16 samples per tooth period
    for i in range(n):
        th = 2.0 * math.pi * i / n
        r = 0.85 if (i % 16) / 16.0 < 0.5 else 0.66
        pts.append((r * math.cos(th), r * math.sin(th)))
    _poly(draw, s, pts)
    _disc(draw, s, 0.28, fill=0)


def _paint_sprout(draw, s):
    _line(draw, s, [(0.0, 0.76), (0.0, -0.14)], width=0.14)
    _poly(draw, s, _lens((0.0, -0.14), (-0.70, -0.72), half_w=0.20))
    _poly(draw, s, _lens((0.0, -0.14), (0.70, -0.72), half_w=0.20))


def _paint_hawk(draw, s):
    wing = [(0.08, -0.26), (0.86, 0.00), (0.90, 0.06), (0.52, 0.30), (0.10, 0.14)]
    _poly(draw, s, wing)
    _poly(draw, s, [(-u, v) for u, v in wing])
    _poly(draw, s, _lens((0.0, -0.30), (0.0, 0.42), half_w=0.12))
    _poly(draw, s, [(-0.10, 0.30), (0.10, 0.30), (0.20, 0.58), (0.0, 0.50), (-0.20, 0.58)])
    _disc(draw, s, 0.13, cx=0.0, cy=-0.40)


def _paint_chevrons3(draw, s):
    for y in (-0.66, 0.00, 0.66):
        _line(draw, s, [(-0.72, y - 0.22), (0.0, y + 0.22), (0.72, y - 0.22)], width=0.16)


def _paint_crate_stencil(draw, s):
    outer = 0.62
    stroke = 0.13
    draw.rectangle(
        [s - outer * s, s - outer * s, s + outer * s, s + outer * s], outline=255, width=max(1, round(stroke * s))
    )
    _line(draw, s, [(-outer, -outer), (outer, outer)], width=stroke * 0.8)
    _line(draw, s, [(-outer, outer), (outer, -outer)], width=stroke * 0.8)
    for y in (-0.86, 0.86):
        _line(draw, s, [(-0.40, y), (0.40, y)], width=0.12)


def _paint_patch_round(draw, s):
    _disc(draw, s, 0.82)
    _disc(draw, s, 0.56, fill=0)
    _disc(draw, s, 0.46)


def _paint_jaig_eyes(draw, s):
    _poly(draw, s, _lens((-0.86, 0.10), (-0.10, -0.24), half_w=0.17))
    _poly(draw, s, _lens((0.86, 0.10), (0.10, -0.24), half_w=0.17))


def _paint_medallion(draw, s):
    _disc(draw, s, 0.86)
    _disc(draw, s, 0.68, fill=0)
    _disc(draw, s, 0.50)
    for k in range(4):
        _poly(draw, s, _radial_rect(0.64, 0.92, 0.08, k * math.pi / 2), fill=0)


def _paint_star5(draw, s):
    outer = 0.88
    # Single point up: bbox runs from -outer (top tip) to outer*cos(36 deg) (lower tips); shift down to centre the bbox.
    dy = outer * (1.0 - math.cos(math.pi / 5)) / 2.0
    pts = []
    for i in range(10):
        r = outer if i % 2 == 0 else 0.38
        th = -math.pi / 2 + i * math.pi / 5
        pts.append((r * math.cos(th), r * math.sin(th) + dy))
    _poly(draw, s, pts)


def _paint_bar_stack(draw, s):
    for y, half_w in ((-0.52, 0.80), (0.0, 0.60), (0.52, 0.40)):
        _poly(draw, s, [(-half_w, y - 0.12), (half_w, y - 0.12), (half_w, y + 0.12), (-half_w, y + 0.12)])


def _paint_sun_disc(draw, s):
    _disc(draw, s, 0.42)
    for k in range(12):
        th = k * math.pi / 6
        p0 = (0.55 * math.cos(th), 0.55 * math.sin(th))
        p1 = (0.90 * math.cos(th), 0.90 * math.sin(th))
        _line(draw, s, [p0, p1], width=0.12)


def _paint_swoop_stripes(draw, s):
    n = 40
    for base in (-0.21, 0.15, 0.51):
        top, bot = [], []
        for i in range(n + 1):
            t = i / n
            x = -0.86 + 1.72 * t
            yc = base - 0.22 * (1.0 - (2.0 * t - 1.0) ** 2)
            half = 0.05 + 0.07 * math.sin(math.pi * t)
            top.append((x, yc - half))
            bot.append((x, yc + half))
        _poly(draw, s, top + bot[::-1])


def _render(size: int, paint: PaintFn) -> np.ndarray:
    size = int(size)
    big = size * SUPERSAMPLE
    canvas = Image.new("L", (big, big), 0)
    paint(ImageDraw.Draw(canvas), big / 2.0)
    small = canvas.resize((size, size), Image.Resampling.LANCZOS)
    return np.asarray(small, dtype=np.float32) / np.float32(255.0)


_PAINTERS: dict[str, PaintFn] = {
    "cog8": _paint_cog8,
    "sprout": _paint_sprout,
    "hawk": _paint_hawk,
    "chevrons3": _paint_chevrons3,
    "crate_stencil": _paint_crate_stencil,
    "patch_round": _paint_patch_round,
    "jaig_eyes": _paint_jaig_eyes,
    "medallion": _paint_medallion,
    "star5": _paint_star5,
    "bar_stack": _paint_bar_stack,
    "sun_disc": _paint_sun_disc,
    "swoop_stripes": _paint_swoop_stripes,
}

DECALS: dict[str, Callable[[int], np.ndarray]] = {
    name: functools.partial(_render, paint=paint) for name, paint in _PAINTERS.items()
}


def list_decals() -> list[str]:
    return sorted(DECALS)


def render_decal(name: str, size: int, rotation_deg: float = 0.0) -> np.ndarray:
    """Return a (size, size) float32 alpha mask in [0, 1]. Unknown names raise KeyError."""
    alpha = DECALS[name](size)
    if rotation_deg % 360.0 == 0.0:
        return alpha
    u8 = np.rint(alpha * 255.0).astype(np.uint8)
    rotated = Image.fromarray(u8).rotate(rotation_deg, resample=Image.Resampling.BICUBIC)
    return np.asarray(rotated, dtype=np.float32) / np.float32(255.0)
