"""Region-limited layer ops and recipe rendering for texture-diversity overlays (P1 U2).

Colours are float32 0..1 RGB (h,w,3); regions are float32 0..1 (h,w) with soft edges. Every op returns
pixels where region == 0 exactly unchanged. A recipe (JSON) picks grid cells and islands of a parent atlas
and lists one to three layers; render_recipe applies them in order. Decal shapes come from k1_port_tex_decals (U1).
"""
import inspect
import json
import re
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from k1_port_tex_decals import DECALS, render_decal  # noqa: E402

GRID = 4
MAX_LAYERS = 3
LAYER_KINDS = ("recolor", "trim", "emblem", "wear")
RECIPE_KEYS = ("id", "atlas", "model", "layers")
WEAR_KINDS = ("dust", "fade", "stain", "scuff")
WEAR_DIRECTIONS = ("down", "up", "none")
BAND_BASE_DEG = {"horizontal": 90.0, "vertical": 0.0, "diagonal": 45.0}
BLEND_MODES = ("normal", "multiply")
DUST_COLOUR = (0.62, 0.58, 0.50)  # grey-brown
REQUIRED_PARAMS = {"decal": ("name", "cx", "cy", "size_px"), "tint": ("colour",)}
_CELL_RE = re.compile(r"([A-D])([1-4])")
_COLOUR_RE = re.compile(r"#[0-9a-fA-F]{6}")


def _cell_index(name):
    match = _CELL_RE.fullmatch(name) if isinstance(name, str) else None
    if match is None:
        raise ValueError(f"bad cell name {name!r}: want A1..D4")
    return ord(match.group(1)) - ord("A"), int(match.group(2)) - 1


def cell_mask(w, h, cells):
    """Bool (h,w) mask: union of grid cells. A1 is top-left, D4 bottom-right; cells are w/4 x h/4 px."""
    mask = np.zeros((h, w), dtype=bool)
    for name in cells:
        col, row = _cell_index(name)
        mask[row * h // GRID:(row + 1) * h // GRID, col * w // GRID:(col + 1) * w // GRID] = True
    return mask


def skin_guard(rgb_u8):
    """Bool (h,w), True where a pixel looks like skin (copied predicate, not imported)."""
    c = np.asarray(rgb_u8).astype(np.int32)
    r, g, b = c[..., 0], c[..., 1], c[..., 2]
    mx = np.maximum(np.maximum(r, g), b)
    mn = np.minimum(np.minimum(r, g), b)
    return (r > 95) & (g > 40) & (b > 20) & ((mx - mn) > 15) & (np.abs(r - g) > 15) & (r > g) & (r > b)


def _box3(a):
    """3x3 box blur with edge padding; output stays in [0,1] for a 0..1 input."""
    h, w = a.shape
    p = np.pad(a, 1, mode="edge")
    acc = np.zeros((h, w), dtype=np.float32)
    for dy in range(3):
        for dx in range(3):
            acc += p[dy:dy + h, dx:dx + w]
    return acc / np.float32(9.0)


def region(w, h, cells, islands, island_labels, avoid_cells=()):
    """Float32 (h,w) region: cells AND union(islands[label]) AND NOT avoid_cells, then a 1-px feather.

    An empty island_labels means no island restriction. An unknown label raises KeyError.
    """
    reg = cell_mask(w, h, cells)
    if island_labels:
        inside = np.zeros((h, w), dtype=bool)
        for label in island_labels:
            inside |= np.asarray(islands[label], dtype=bool)
        reg &= inside
    if avoid_cells:
        reg &= ~cell_mask(w, h, avoid_cells)
    return _box3(reg.astype(np.float32))


def _colour_rgb(colour):
    if not isinstance(colour, str) or not _COLOUR_RE.fullmatch(colour):
        raise ValueError(f"bad colour {colour!r}: want #rrggbb")
    return np.array([int(colour[i:i + 2], 16) for i in (1, 3, 5)], dtype=np.float32) / np.float32(255.0)


def _blend(rgb, weight, new):
    """rgb + weight*(new-rgb); weight 0 returns rgb exactly. new is clipped to 0..1 before blending."""
    new = np.clip(np.asarray(new, dtype=np.float32), 0.0, 1.0)
    out = rgb + weight.astype(np.float32)[..., None] * (new - rgb)
    return out.astype(np.float32)


def _to_u8(rgb):
    return np.clip(np.rint(rgb * np.float32(255.0)), 0, 255).astype(np.uint8)


def _rgb_to_hsv(rgb):
    r, g, b = rgb[..., 0], rgb[..., 1], rgb[..., 2]
    mx = rgb.max(axis=-1)
    d = mx - rgb.min(axis=-1)
    safe_d = np.where(d > 0, d, np.float32(1.0))
    hue = np.where(mx == r, (g - b) / safe_d,
                   np.where(mx == g, (b - r) / safe_d + 2.0, (r - g) / safe_d + 4.0))
    hue = np.where(d > 0, np.mod(hue, 6.0), 0.0) / 6.0
    safe_mx = np.where(mx > 0, mx, np.float32(1.0))
    sat = np.where(mx > 0, d / safe_mx, 0.0)
    return hue.astype(np.float32), sat.astype(np.float32), mx.astype(np.float32)


def _hsv_to_rgb(h, s, v):
    h6 = np.mod(h, 1.0) * 6.0
    sector = np.floor(h6)
    f = h6 - sector
    i = sector.astype(np.int32) % 6
    p = v * (1.0 - s)
    q = v * (1.0 - s * f)
    t = v * (1.0 - s * (1.0 - f))
    conds = [i == k for k in range(6)]
    r = np.select(conds, [v, q, p, p, t, v])
    g = np.select(conds, [t, v, v, q, p, p])
    b = np.select(conds, [p, p, t, v, v, q])
    return np.stack([r, g, b], axis=-1).astype(np.float32)


def recolor_hsv(rgb, reg, hue_shift_deg=0, sat_scale=1, val_scale=1, keep_skin=True):
    """Shift hue, scale saturation and value inside the region; skin-like pixels are kept when keep_skin."""
    weight = reg
    if keep_skin:
        weight = reg * (~skin_guard(_to_u8(rgb))).astype(np.float32)
    h, s, v = _rgb_to_hsv(rgb)
    h = np.mod(h + np.float32(float(hue_shift_deg) / 360.0), 1.0)
    s = np.clip(s * np.float32(sat_scale), 0.0, 1.0)
    v = np.clip(v * np.float32(val_scale), 0.0, 1.0)
    return _blend(rgb, weight, _hsv_to_rgb(h, s, v))


def band(rgb, reg, orientation="horizontal", period_px=8, width_px=4, colour="#ffffff", opacity=1.0,
         angle_deg=0.0):
    """Stripes of width_px every period_px px. Base direction per orientation, plus angle_deg."""
    if orientation not in BAND_BASE_DEG:
        raise ValueError(f"bad band orientation {orientation!r}")
    if period_px <= 0 or width_px < 0 or width_px > period_px:
        raise ValueError("band needs period_px > 0 and 0 <= width_px <= period_px")
    h, w = reg.shape
    phi = np.deg2rad(BAND_BASE_DEG[orientation] + float(angle_deg))
    cos_p = np.round(np.cos(phi), 12)
    sin_p = np.round(np.sin(phi), 12)
    yy, xx = np.mgrid[0:h, 0:w]
    coord = xx * cos_p + yy * sin_p
    stripe = (np.mod(coord, period_px) < width_px).astype(np.float32)
    return _blend(rgb, reg * stripe * np.float32(opacity), _colour_rgb(colour))


def decal(rgb, reg, name, cx, cy, size_px, rotation_deg=0.0, colour="#ffffff", opacity=1.0, blend="normal"):
    """Paste a U1 decal centred on atlas pixel (cx,cy); alpha is clipped by the region and opacity.

    The tile covers [cx - size//2, cx - size//2 + size). Off-atlas parts are cropped, never an error.
    """
    if blend not in BLEND_MODES:
        raise ValueError(f"bad blend {blend!r}")
    size = int(size_px)
    if size <= 0:
        raise ValueError("decal size_px must be > 0")
    h, w = reg.shape
    alpha = render_decal(name, size, rotation_deg)  # KeyError on unknown name
    x0 = int(round(cx)) - size // 2
    y0 = int(round(cy)) - size // 2
    ax0, ay0 = max(0, x0), max(0, y0)
    ax1, ay1 = min(w, x0 + size), min(h, y0 + size)
    weight = np.zeros((h, w), dtype=np.float32)
    if ax1 > ax0 and ay1 > ay0:
        weight[ay0:ay1, ax0:ax1] = alpha[ay0 - y0:ay1 - y0, ax0 - x0:ax1 - x0]
    weight *= reg.astype(np.float32) * np.float32(opacity)
    col = _colour_rgb(colour)
    new = col if blend == "normal" else rgb * col
    return _blend(rgb, weight, new)


def _gradient(h, direction):
    if direction == "none":
        return np.ones((h, 1), dtype=np.float32)
    y = np.linspace(0.0, 1.0, h, dtype=np.float32)[:, None]
    return y if direction == "down" else (np.float32(1.0) - y)


def _bilinear_up(grid, h, w):
    gh, gw = grid.shape
    ys = np.linspace(0, gh - 1, h)
    xs = np.linspace(0, gw - 1, w)
    y0 = np.floor(ys).astype(int)
    y1 = np.minimum(y0 + 1, gh - 1)
    fy = (ys - y0)[:, None].astype(np.float32)
    x0 = np.floor(xs).astype(int)
    x1 = np.minimum(x0 + 1, gw - 1)
    fx = (xs - x0)[None, :].astype(np.float32)
    top = grid[y0][:, x0] * (1 - fx) + grid[y0][:, x1] * fx
    bot = grid[y1][:, x0] * (1 - fx) + grid[y1][:, x1] * fx
    return (top * (1 - fy) + bot * fy).astype(np.float32)


def _value_noise(rng, h, w, octaves=3, cells=4):
    total = np.zeros((h, w), dtype=np.float32)
    amp, norm = 1.0, 0.0
    for o in range(octaves):
        n = cells * (2 ** o)
        grid = rng.random((n + 1, n + 1)).astype(np.float32)
        total += np.float32(amp) * _bilinear_up(grid, h, w)
        norm += amp
        amp *= 0.5
    return total / np.float32(norm)


def wear(rgb, reg, kind="dust", strength=0.3, seed=0, direction="down"):
    """Seeded value-noise wear times a vertical gradient. Deterministic per seed."""
    if kind not in WEAR_KINDS:
        raise ValueError(f"bad wear kind {kind!r}")
    if direction not in WEAR_DIRECTIONS:
        raise ValueError(f"bad wear direction {direction!r}")
    if not 0.0 <= strength <= 1.0:
        raise ValueError("wear strength must be in 0..1")
    h, w = reg.shape
    rng = np.random.default_rng(int(seed))
    grad = _gradient(h, direction)
    if kind == "scuff":
        speck = rng.random((h, w)).astype(np.float32)
        factor = (np.float32(1.0) - np.float32(strength) * speck * grad).astype(np.float32)
        return _blend(rgb, reg, rgb * factor[..., None])
    noise = _value_noise(rng, h, w)
    amount = reg * np.float32(strength) * np.clip(noise * grad, 0.0, 1.0)
    if kind == "dust":
        return _blend(rgb, amount, DUST_COLOUR)
    if kind == "fade":
        lum = rgb @ np.array([0.299, 0.587, 0.114], dtype=np.float32)
        return _blend(rgb, amount, np.broadcast_to(lum[..., None], rgb.shape))
    return _blend(rgb, amount, rgb * np.float32(0.55))  # stain


def tint(rgb, reg, colour, strength=0.3):
    """Lerp towards colour inside the region."""
    if not 0.0 <= strength <= 1.0:
        raise ValueError("tint strength must be in 0..1")
    return _blend(rgb, reg * np.float32(strength), _colour_rgb(colour))


def colorize(rgb, reg, colour, strength=0.8, contrast=1.0):
    """Re-colour near-grey plates: the colour scaled by the pixel's own luminance, so plate detail survives.

    recolor_hsv cannot do this because grey has no saturation to rotate.
    """
    if not 0.0 <= strength <= 1.0:
        raise ValueError("colorize strength must be in 0..1")
    lum = rgb @ np.array([0.299, 0.587, 0.114], dtype=np.float32)
    lum = np.clip((lum - lum.mean()) * np.float32(contrast) + lum.mean(), 0.0, 1.0)
    new = _colour_rgb(colour) * (np.float32(0.25) + np.float32(1.1) * lum)[..., None]
    return _blend(rgb, reg * np.float32(strength), new)


OPS = {"recolor_hsv": recolor_hsv, "band": band, "decal": decal, "wear": wear, "tint": tint, "colorize": colorize}
EMBLEM_OPS = {"decal"}


def load_recipe(path):
    return json.loads(Path(path).read_text(encoding="utf-8"))


def _is_seed(value):
    return isinstance(value, int) and not isinstance(value, bool) and value >= 0


def _op_params(op):
    return set(inspect.signature(OPS[op]).parameters) - {"rgb", "reg"}


def _layer_problems(i, layer):
    where = f"layer {i}"
    if not isinstance(layer, dict):
        return [f"{where}: must be an object"]
    problems = []
    kind = layer.get("kind")
    op = layer.get("op")
    if kind not in LAYER_KINDS:
        problems.append(f"{where}: kind {kind!r} not in {', '.join(LAYER_KINDS)}")
    if not isinstance(op, str) or op not in OPS:
        problems.append(f"{where}: unknown op {op!r}")
    for key in ("cells", "avoid_cells"):
        names = layer.get(key, [])
        if not isinstance(names, list):
            problems.append(f"{where}: {key} must be a list")
            continue
        for name in names:
            try:
                _cell_index(name)
            except ValueError:
                problems.append(f"{where}: bad cell {name!r} in {key}")
    islands = layer.get("islands", [])
    if not isinstance(islands, list) or not all(isinstance(x, str) for x in islands):
        problems.append(f"{where}: islands must be a list of labels")
    params = layer.get("params", {})
    if not isinstance(params, dict):
        return problems + [f"{where}: params must be an object"]
    if isinstance(op, str) and op in OPS:
        for key in params:
            if key not in _op_params(op):
                problems.append(f"{where}: op {op} has no param {key!r}")
        for key in REQUIRED_PARAMS.get(op, ()):
            if key not in params:
                problems.append(f"{where}: op {op} needs param {key!r}")
        if op == "decal":
            name = params.get("name")
            if not isinstance(name, str) or name not in DECALS:
                problems.append(f"{where}: unknown decal {name!r}")
    for key, val in params.items():
        if "colour" in key or "color" in key:
            if not isinstance(val, str) or not _COLOUR_RE.fullmatch(val):
                problems.append(f"{where}: {key} {val!r} must match #rrggbb")
    if "seed" in params and not _is_seed(params["seed"]):
        problems.append(f"{where}: seed must be a non-negative int")
    return problems


def validate_recipe(recipe):
    """Return a list of problems; an empty list means the recipe is valid."""
    if not isinstance(recipe, dict):
        return ["recipe must be a JSON object"]
    problems = [f"missing key {key!r}" for key in RECIPE_KEYS if key not in recipe]
    if "seed" in recipe and not _is_seed(recipe["seed"]):
        problems.append("seed must be a non-negative int")
    layers = recipe.get("layers")
    if not isinstance(layers, list) or not 1 <= len(layers) <= MAX_LAYERS:
        return problems + [f"layers must be a list of 1 to {MAX_LAYERS} layers"]
    emblems = decals = 0
    for i, layer in enumerate(layers):
        problems += _layer_problems(i, layer)
        if isinstance(layer, dict):
            emblems += int(layer.get("kind") == "emblem")
            op = layer.get("op")
            decals += int(isinstance(op, str) and op in EMBLEM_OPS)
    if emblems > 1:
        problems.append(f"{emblems} emblem layers; at most 1")
    if decals > 1:
        problems.append(f"{decals} decal-op layers; at most 1")
    return problems


def render_recipe(recipe, parent_rgb_u8, islands):
    """Apply recipe layers to a parent (h,w,3) uint8 atlas. Returns (rgb_u8, union_mask_bool)."""
    problems = validate_recipe(recipe)
    if problems:
        raise ValueError("invalid recipe: " + "; ".join(problems))
    parent = np.asarray(parent_rgb_u8)
    if parent.ndim != 3 or parent.shape[2] != 3:
        raise ValueError("parent must be (h,w,3) RGB")
    h, w = parent.shape[:2]
    rgb = parent.astype(np.float32) / np.float32(255.0)
    base_seed = int(recipe.get("seed", 0))
    union = np.zeros((h, w), dtype=bool)
    for i, layer in enumerate(recipe["layers"]):
        reg = region(w, h, layer.get("cells", []), islands, layer.get("islands", []),
                     layer.get("avoid_cells", []))
        union |= reg > 0
        params = dict(layer.get("params", {}))
        if layer["op"] == "wear":
            params.setdefault("seed", base_seed + i)
        rgb = OPS[layer["op"]](rgb, reg, **params)
    return _to_u8(rgb), union
