#!/usr/bin/env python3
"""Generate aspect-corrected full-screen textures for KOTOR II ultrawide displays.

Companion to generate_ultrawide_gui_overrides.py. Some stretched art is not a
control's extent but a texture that fills the whole 800x600 virtual screen, so it
is stretched by M = (width / height) / (4 / 3) (1.8 at 3840x1600) no matter what
the GUI says. Two such cases are fixed here, by rewriting the texture itself:

loadscreens  Loading screens (loadscreen_p's root fill, swapped by the engine for
             the load_* image of loadscreens.2da). Each image is pillarboxed: the
             source is resized to (W/M, H) and pasted centred over a blurred,
             darkened copy of itself (GaussianBlur radius W/40, brightness 0.35).
             The output keeps the source size, so on screen the centre band has
             the source's aspect and texel density; the sides are blurred filler.
frame        pnl_pause_pc, the gold/brown frame behind 29 menus. Its ornaments
             (the edge columns with their knobs and corner curves, the top notch
             and bottom grille in the centre) are squeezed by 1/M so the knobs are
             round again; the plain brushed-metal rails between them are stretched
             linearly to fill. Corners stay pinned to the outer edges and the
             centre span stays centred. The spans live in FRAME_SPECS so the result
             is reproducible (`frame --detect` re-derives a suggestion from the
             texture). The remapped inner edge is written to frame_edges.json
             ({"E_left": ...} in 800-wide virtual units) for the GUI generator's
             frame-bar rule. A preview (original vs output, both at 2.4:1) is
             written first; deploy it only after a human has looked at it.
unsqueeze    Mod textures whose art was pre-squeezed for a 16:9 stretch (the
             PRESQUEEZED table, matched by exact sha256), e.g. the mod150 TSLRCM
             logo at 0.75x width. Once the GUI narrows their box they would look
             squished, so the art is widened back about its centre. Unknown files
             are skipped.
upscale      Real-ESRGAN upscales of a texture's pristine source, recorded in
             upscaled/upscaled.json (preview only; `frame --upscaled`,
             `loadscreens --upscaled` and `cards` deploy them). Opaque textures by
             default; `--alpha model|lanczos` handles alpha: transparent texels get
             the RGB of the nearest visible texel (no dark fringes), RGB goes through
             the model, and alpha either through the model as (A,A,A) (falling back
             to LANCZOS per texture when it diverges) or by LANCZOS.
             `--target-width N` upscales to exactly N wide (integer factor >= 2);
             `--loadscreens` picks every loading screen narrower than the target.
             One run at a time: it holds a lock on upscaled/.lock (the holder's
             PID inside). `--skip-done` resumes a batch (see upscale_done).
cards        The pazaak card faces (CARD_PREFIX, PC `_p` variants) from their
             recorded alpha-aware upscales, as TGA + TXI sidecar.

A source TXI (the text embedded in a TPC) is carried over: into an output TPC, or
as a <stem>.txi sidecar beside an output TGA (tracked in the manifest; rollback
deletes it only while its sha is ours).

Source precedence per texture name mirrors the engine: override .tpc, then
override .tga, then the TexturePacks ERFs (swpc_tex_gui.erf first). The same rule
as dedup_tga_tpc.py: a .tpc beside a .tga wins, so this tool never leaves a pair.
- override .tga  -> replaced by a .tga of the same name; the original is kept in
                    $STATE/originals_tex/.
- override .tpc  -> replaced by an uncompressed .tpc when a pykotor round trip
                    decodes to the same pixels; otherwise a .tga is written and
                    the .tpc moves to originals_tex/ (no pair remains).
- ERF only       -> a new override .tga; rollback deletes it.
TGAs are written uncompressed, 32-bit, bottom-left origin (like the mod TGAs).

State ($WORKSPACE/ultrawide_gui): originals_tex/, sources_tex.json, out_tex/,
manifest_tex.json (source, target, original and deployed sha256 per file),
frame_edges.json. --deploy refuses while swkotor2.exe runs and refuses to
overwrite a file that is neither the recorded original nor our previous deploy.
--rollback restores override-sourced files, deletes ERF-sourced ones, and leaves
any file changed since the deploy.

Usage:
  generate_ultrawide_textures.py frame [--detect] [--deploy]
  generate_ultrawide_textures.py loadscreens [--upscaled MODEL] [--target-width 2048] [--deploy]
  generate_ultrawide_textures.py unsqueeze [--deploy]
  generate_ultrawide_textures.py upscale [NAMES] [--loadscreens | --cards] [--target-width N]
                                         [--alpha model|lanczos] [--models M1,M2]
  generate_ultrawide_textures.py upscale --world PREFIX|all --alpha model [--skip-done]
  generate_ultrawide_textures.py cards --upscaled MODEL [--deploy]
  generate_ultrawide_textures.py world [--category PREFIX|all] [--upscaled MODEL [--all-sheets]]
                                       [--only A,B|@FILE] [--deploy [--wave NAME]]
  generate_ultrawide_textures.py world --rollback [--wave NAME]
  generate_ultrawide_textures.py --rollback
"""

import argparse
from collections.abc import Callable
import datetime
import fcntl
import hashlib
import io
import json
import logging
import os
from pathlib import Path
import random
import re
import shutil
import struct
import subprocess
import sys
import tempfile

logging.disable(logging.CRITICAL)

try:
    from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter, ImageMath, ImageOps, ImageStat
    from pykotor.resource.formats.erf import read_erf
    from pykotor.resource.formats.gff import read_gff
    from pykotor.resource.formats.tpc import TPC, TPCTextureFormat, bytes_tpc, read_tpc
    from pykotor.resource.formats.twoda import read_2da
except ImportError:
    print("Error: pykotor and Pillow must be installed in python environment.", file=sys.stderr)
    sys.exit(1)

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from generate_ultrawide_gui_overrides import (  # noqa: E402
    _CHITIN_GUI_CACHE,
    DEFAULT_KOTOR2_ROOT,
    DEFAULT_WORKSPACE_DIR,
    HUD_DESC_GUI,
    chitin_gui_bytes,
    compute_stretch_factor,
    find_ci,
    is_game_running,
    sha256,
)

ERF_ORDER = ["swpc_tex_gui.erf", "swpc_tex_tpa.erf", "swpc_tex_tpb.erf", "swpc_tex_tpc.erf"]

BLUR_DIVISOR = 40
SIDE_BRIGHTNESS = 0.35

# Ornament spans of each frame texture, in texel columns [x0, x1), found with
# `frame --detect` (horizontal-gradient energy of the top rows 0-96 and bottom rows
# 928-1023; rails are horizontal brushed-metal lines with near-zero d/dx) and then
# made symmetric. The edge spans include the side columns out past the inner edge
# (x=96 at the knob bulge), the corner curves, and the bottom grille's end caps.
# edge_row is a texel row through the teal separators (virtual y=112) where the
# inner edge is x=78, i.e. the vanilla bar edge 61 = 78 * 800 / 1024.
# Spans and edge_row are in the vanilla (1024) texel space; `frame --upscaled`
# scales them to the upscaled width. "bars" marks textures whose GUIs have teal
# separators touching the frame (the GUI generator's FRAME_BAR_RULES): only their
# E_left is used. For the others edge_row = 512 is recorded for information only.
FRAME_SPECS = {
    "pnl_pause_pc": {
        "spans": [(0, 190), (300, 724), (834, 1024)],
        "edge_row": 191,
        "bars": True,
    },
    # store_p: bars LBL_BAR1/2/3 at L61 R739, TOP 104 -> row (104 + 7) * 1024 / 600 = 191.
    # Auto-detected (0,124),(277,740),(901,1024), made symmetric about 512.
    "pnl_store_pc": {
        "spans": [(0, 124), (277, 747), (900, 1024)],
        "edge_row": 191,
        "bars": True,
    },
    # Auto-detected (0,104),(274,744),(920,1024), made symmetric about 512.
    "pnl_computer_pc": {
        "spans": [(0, 104), (274, 750), (920, 1024)],
        "edge_row": 512,
        "bars": False,
    },
    # Workbench and upgrade are the same art (they differ only in rows 807-845).
    # Detection fails (the rails carry vertical greebles); hand-measured corner pods:
    # the pod's diagonal meets the top rail at x=168 (mirror 856), no centre ornament.
    "pnl_workbench_pc": {
        "spans": [(0, 170), (854, 1024)],
        "edge_row": 512,
        "bars": False,
    },
    "pnl_upgrade_pc": {
        "spans": [(0, 170), (854, 1024)],
        "edge_row": 512,
        "bars": False,
    },
    # Pazaak boards ("board": the GUI generator's --board-4x3 maps every control of
    # the GUI by the same 1/M about x=400). The whole raised plate is ONE centre span,
    # squeezed by 1/M to its true 4:3 shape; the plain horizontally brushed margins
    # between it and the thin rim grooves (kept as pinned 8-texel spans) stretch to
    # fill the screen. Plate groove measured at x=60-68 / 956-964 (game) and x=55-62
    # (setup), symmetric about 512.
    "pnl_pazaakgam_pc": {
        "spans": [(0, 8), (58, 966), (1016, 1024)],
        "edge_row": 512,
        "bars": False,
        "board": True,
    },
    "pnl_pazaakset_pc": {
        "spans": [(0, 8), (54, 970), (1016, 1024)],
        "edge_row": 512,
        "bars": False,
        "board": True,
    },
    # Galaxy map (board: its planet icons sit over a 3D view drawn at the real pixel
    # aspect). `--detect` finds no rails: the art has vertical structure everywhere, and
    # it is asymmetric (a cyan monitor cut by the left edge, dark panels on the right).
    # Hand-picked from column statistics (2026-09-25): the plate 16-1008 (symmetric about
    # 512, so it maps like the GUI) is squeezed; the rails are the monitor's smooth
    # interior 0-16 (d/dx 0.1-4.6) and the uniform dark metal 1008-1010 (d/dx < 1.5,
    # vertical std 7); the fine border lines 1010-1024 stay pinned to the right edge.
    "pnl_galaxy_pc": {
        "spans": [(16, 1008), (1010, 1024)],
        "edge_row": 512,
        "bars": False,
        "board": True,
        "asymmetric_edges": True,
    },
}

# Real-ESRGAN (ncnn-vulkan build) models for `upscale`, output at UPSCALE_FACTOR x the
# source. realesrgan-x4plus is the primary: the panels are brushed metal and bevels,
# and the anime models flatten grain. Its 4x output is LANCZOS-resized to 2x.
# realesr-animevideov3 at native 2x is a comparison candidate.
UPSCALE_FACTOR = 2
UPSCALE_MODELS = {
    "realesrgan-x4plus": 4,
    "realesr-animevideov3": 2,
}
PRIMARY_UPSCALE_MODEL = "realesrgan-x4plus"
DEFAULT_REALESRGAN = os.path.join(DEFAULT_WORKSPACE_DIR, "tools", "realesrgan", "realesrgan-ncnn-vulkan")
# `upscale --loadscreens` / `loadscreens --upscaled`: sources narrower than this are
# upscaled to it (the biggest mod loading screens are 2048 wide).
LOADSCREEN_TARGET_WIDTH = 2048

# Alpha-aware upscaling (`upscale --alpha`). The RGB bleed is a pure-Python BFS, so it
# is limited to textures of at most ALPHA_MAX_TEXELS (about 1 s at the limit). With
# --alpha model, the model's alpha is compared with a LANCZOS resize of the source
# alpha; a mean or max absolute difference above ALPHA_DIVERGENCE (in 0..255 units)
# means the model invented or lost mask detail, and that texture uses LANCZOS alpha.
ALPHA_MAX_TEXELS = 512 * 512
ALPHA_DIVERGENCE = {"mean": 10.0, "max": 128}
ALPHA_MODES = ("model", "lanczos")

# Pazaak card faces: pcards_<face>_p in the TexturePacks ERFs, 128x128 with alpha and
# TXI "mipmap 0". The pcards_<face>_x variants are 64x64 Xbox textures, never drawn on
# PC. A card control is 44 units wide at board k (Method 5), about 211 px at 1600 px
# high, so 2x (256) is the right size; 4x would only be minified.
CARD_PREFIX = "pcards_"
CARD_XBOX_SUFFIX = "_x"

# GUI texture discovery (`scan`). Every PC GUI (live override copy first, else chitin)
# is walked: the root, its controls and their PROTOITEM/SCROLLBAR, in each of
# SCAN_STATES. A FILL's magnification is (on-screen px) / (texture px), the larger of
# the two axes; a texture is a candidate when its largest use reaches SCAN_MIN_MAG and
# none of the exclusions applies (each recorded as the texture's `reason`).
SCAN_STATES = ("BORDER", "HILIGHT", "SELECTED", "HILIGHTSELECTED")
SCAN_MIN_MAG = 1.5
# Tile-like: a texture this small drawn more than SCAN_TILE_STRETCH x in one axis is a
# flat fill or an edge strip (or a checkbox icon in a wide box): no detail to gain.
SCAN_TILE_MAX_SIDE = 32
SCAN_TILE_STRETCH = 8.0
SCAN_FLAT_TOLERANCE = 4          # flat: every texel within +-4 of the mean, per channel
SCAN_NAME_EXCLUDE = re.compile(r"^fnt_|^lbl_map|_lm\d*$")    # fonts, area maps, lightmaps
GUI_XBOX_SUFFIX = "_x"
# One HUD GUI per resolution family; the engine draws the one for this resolution 1:1
# (both axes height / 600, measured on mipc28x6_p at 3840x1600). The others never show.
HUD_GUI_PATTERN = re.compile(r"^mi(pc)?2?\d+x\d+_p$|^maininterface_p$")
SCAN_FILE = "scan_gui_textures.json"

# Named upscale sets (`upscale --set`, `set`). ui: the scan's candidates, minus the
# other sets' members, root fills (panels: `frame` or the popup layer own those),
# popup-only textures (popup scale not measured) and UI_EXCLUDE.
SET_NAMES = ("cards", "ui", "galaxy_icons", "reticles", "portraits")
# Textures dropped from `ui` after the preview sheets, name -> reason.
UI_EXCLUDE: dict[str, str] = {}
GALAXY_GUI = "galaxymap_p"
GALAXY_ICON_PREFIXES = ("gal_", "gui_")
# Drawn by the engine over targets (no GUI control), all in swpc_tex_gui.erf with alpha.
RETICLE_TEXTURES = ("combatreticle", "friendlyreticle", "friendlyreticle2", "hostilereticle",
                    "hostilereticle2", "whitetarget", "yellowtarget")
# On-screen size of the reticles for the sheet (px; None: shown at 100% of the upscale).
RETICLE_SCREEN_PX: int | None = None
PORTRAIT_PREFIX = "po_"
PORTRAIT_EXCLUDE = {"po_pjolee": "a KOTOR I portrait (Jolee Bindo), never shown in TSL"}
# The largest portrait control (portcust_p LBL_PORTRAIT, 90x90 units, 2026-09-25; maincg
# 78, partyselect 80, top_p 70, HUD 72): at height / 600 px per unit a portrait whose
# longest side is below 90 * height / 600 / SCAN_MIN_MAG is magnified enough to upscale.
PORTRAIT_MAX_UNITS = 90
SHEET_PER_PAGE = 12
SHEET_TILE_MAX = 384

# World textures (`upscale --world PREFIX`, `world --category PREFIX`): vanilla-only
# TPCs of WORLD_ERF whose name starts with the category. They are minified at a
# distance, so the output keeps a mip chain and the source's block compression (DXT1
# opaque, DXT5 with alpha; each level LANCZOS from the upscale and encoded by Pillow's
# BC1/BC3 encoder: pykotor's pure-Python DXT encoder errs by 20-76/255 on average, Pillow
# by 1-4; uncompressed sources stay uncompressed, see WORLD_UNCOMPRESSED). Excluded,
# each detected from the TPCs themselves (world_exclusion).
WORLD_ERF = "swpc_tex_tpa.erf"
WORLD_MAX_SIDE = 1024            # the upscale factor is capped so the long side stays <= this
WORLD_SMALL_SIDE = 128           # a texture this small (long side) gets 4x, others 2x
WORLD_MANIFEST = "manifest_world.json"
WORLD_OUT = "out_world"
WORLD_LIGHTMAP_NAME = re.compile(r"_lm\d*$")
WORLD_ALL = "all"                # `--world all`, `--category all`: every eligible texture (the empty prefix)
# A source stored uncompressed (RGB/RGBA), or whose TXI says `compresstexture 0`, stays
# uncompressed: RGB when opaque, else RGBA, with the same mip rule (the 11 such eligible
# textures are gradients, HUD strips and digits, where DXT would band or smear).
WORLD_UNCOMPRESSED = ("RGB", "RGBA")
# Excluded from the source image (world_image_exclusion), calibrated on the 1,084 textures
# eligible before these rules (2026-09-25):
# - tiny: long side <= WORLD_TINY_SIDE (12 textures; too small to gain anything).
# - lookup table: uncompressed (source RGB/RGBA or TXI compresstexture 0) and long side <=
#   WORLD_LOOKUP_SIDE: exactly lmg_grad01 (a 4x4 16-colour table the model smeared, the worst
#   tone diff at 56.9), mg_grad01 and mg_line. The larger uncompressed ones are HUD art.
# - flat: RGB flat (SCAN_FLAT_TOLERANCE) and alpha max - min <= WORLD_FLAT_ALPHA_RANGE. The
#   alpha ranges of the RGB-flat textures jump from 35 to 59: 18 have range 0, per_eh2 11,
#   per_fccagea/plc_bldpdlsm/plc_oil 35; mg_ondr2 (59) and up (dan_shad, blood, evilspot)
#   carry their content in the alpha and stay eligible.
WORLD_TINY_SIDE = 16
WORLD_LOOKUP_SIDE = 32
WORLD_FLAT_ALPHA_RANGE = 40
# Review triage (`world --upscaled`): each built texture gets the WORLD_FLAG_RULES that
# apply, in priority order (the flagged sheet sorts by the first). dxt-error: the mean
# absolute difference per channel (RGB, + A for DXT5, 0..255) between the encoded level 0
# and the post-processed upscale, above WORLD_DXT_ERROR_FLAG. grain-loss: the model kept
# less than WORLD_GRAIN_KEPT_MAX of the vanilla detail but back-projection is guarded
# (backproject_guard), so only the tone fix ran. The unflagged textures are reviewed
# through WORLD_SAMPLE of them, drawn with random.Random(WORLD_SAMPLE_SEED) from the
# sorted names.
WORLD_FLAG_RULES = ("alpha-lanczos", "blending", "decal", "uncompressed", "dxt-error", "grain-loss", "clamp")
# Calibrated on the 54 plc pilot outputs (2026-09-25): dxt_error min 0.67, median 4.07,
# p95 10.19, max 12.86 (plc_hypersc01/02). The p95 rounded up to 0.5.
WORLD_DXT_ERROR_FLAG = 10.5
# Post-processing of the model output (world_postprocess), measured on the 1,084 upscales
# (2026-09-25): 859 came out darker than vanilla (median -1.37 levels, 145 by more than 5),
# because the model smooths away bright grain and speckle.
# - Tone fix, on every texture: the model's detail above WORLD_TONE_RADIUS source px, the
#   (LANCZOS) vanilla's tone below it. At 2 source px the luma shift was under 0.6 on every
#   prototype texture and the model's detail stayed (reviewed on a sheet).
# - Back-projection, only where the model lost the grain (grain_kept < WORLD_GRAIN_KEPT_MAX,
#   with enough vanilla detail to measure: WORLD_GRAIN_MIN_HF) and nothing guards it:
#   WORLD_BACKPROJECT_ITERS rounds of out += BICUBIC_up(vanilla - LANCZOS_down(out)), blended
#   in by WORLD_BACKPROJECT_STRENGTH. It restores grain exactly (lkr_flr03a diff 29.1 -> 3.3)
#   but also reinjects source dither and 4x block steps, so model skins (heads, bodies,
#   creatures, weapons, items), clamp/blending/decal textures and WORLD_BACKPROJECT_EXCLUDE
#   never get it.
# Calibrated on sheets of the 131 unguarded textures with grain_kept < 0.8 (the user's choice,
# 2026-09-26): below 0.6 the model turned grain to mush or painted another surface
# (lkr_flr03a, tmp_metal01, ebo_nast, cliff02, tmp_sand); above it the look is kept. Strength
# 1.0 restores vanilla's grain exactly (s = 0.5 left it half-smooth). Below a vanilla detail
# energy of 1.5 the ratio is noise (leh_cyanglow 0.93, plc_starmap_01 1.07, mycube6 0.17);
# 1.5 still takes ond_nwall04 (1.84, kept 0.23).
WORLD_TONE_RADIUS = 2.0
WORLD_BACKPROJECT_ITERS = 3
WORLD_BACKPROJECT_STRENGTH = 1.0
WORLD_GRAIN_KEPT_MAX = 0.6
WORLD_GRAIN_MIN_HF = 1.5
# The vanilla TSL names of player heads/bodies, NPCs, creatures, weapons and items: model
# skins, where the model's smoothing of old compression noise is wanted. world_textures
# reads only the vanilla WORLD_ERF, so vanilla naming holds on any install.
WORLD_BACKPROJECT_SKIP_PREFIXES = ("pfh", "pmh", "pfb", "pmb", "n_", "c_", "w_", "i_")
WORLD_BACKPROJECT_EXCLUDE: dict[str, str] = {}
# Overrides of backproject_guard, name -> reason, from the Phase 6 sheet review (the user's choice).
WORLD_BACKPROJECT_ALLOW: dict[str, str] = {
    "c_cann01": "creature skin the model smeared; back-projection restores its grain (Phase 6 review)",
    "c_maal01": "creature skin the model smeared; back-projection restores its grain (Phase 6 review)",
}
WORLD_SAMPLE = 36
WORLD_SAMPLE_SEED = 0
WORLD_TOP_PREFIXES = 15           # prefixes listed in the `world --upscaled` log (all are in the flags file)

# Mod textures whose art was pre-squeezed to look right under a 16:9 stretch of
# the unfixed 800x600 GUI, keyed by the exact source sha256. Once the GUI box is
# narrowed by 1/M the art shows at its own shape, so it is widened back by
# `factor` about its centre (canvas size kept; the widened art must still fit).
# Only known files are touched: an unknown sha is reported, never guessed at.
PRESQUEEZED = {
    "48734ba793fa3eadcab5e965af0c32ae49e18930f750cbe9e58eeeaa36b1bfeb": {
        "name": "kotor2logo",
        "factor": 4 / 3,
        "origin": "Main Menu Widescreen Fix v1.2 (mod150), TSLRCM logo: gold art 1.51:1 vs 2.01:1 in TSLRCM's own",
    },
}

DETECT_BANDS = ((0, 96), (928, 1024))
DETECT_THRESHOLD = 3.0
DETECT_SMOOTH = 6
DETECT_MERGE_GAP = 32


# ---------------------------------------------------------------- image helpers

def pillarbox(img: Image.Image, stretch_factor: float) -> Image.Image:
    """Centre a (W/M, H) copy over a blurred, darkened full-size copy. Keeps W x H."""
    src = img.convert("RGB")
    w, h = src.size
    bg = src.filter(ImageFilter.GaussianBlur(radius=max(1.0, w / BLUR_DIVISOR)))
    bg = ImageEnhance.Brightness(bg).enhance(SIDE_BRIGHTNESS)
    fg_w = round(w / stretch_factor)
    fg = src.resize((fg_w, h), Image.LANCZOS)
    bg.paste(fg, ((w - fg_w) // 2, 0))
    return bg.convert("RGBA")


def unsqueeze(img: Image.Image, factor: float) -> Image.Image:
    """Widen the art by `factor` about the centre, keeping the canvas size.

    Raises ValueError if any visible texel (alpha > 0) would fall off the canvas.
    """
    w, h = img.size
    wide_w = round(w * factor)
    wide = img.resize((wide_w, h), Image.LANCZOS)
    x0 = (wide_w - w) // 2
    bbox = wide.getchannel("A").getbbox()
    if bbox is not None and (bbox[0] < x0 or bbox[2] > x0 + w):
        raise ValueError(f"widened art spans x {bbox[0]}-{bbox[2]} of {wide_w}; the {w}-wide canvas keeps {x0}-{x0 + w}")
    return wide.crop((x0, 0, x0 + w, h))


def detect_ornament_spans(img: Image.Image, inner_edge_row: int | None = None) -> list[tuple[int, int]]:
    """Suggest ornament spans: columns whose top/bottom band has vertical structure.

    Rails are horizontal lines, so their horizontal gradient is near zero; ornaments
    (knobs, notches, curves, grille ends) are not. The side columns out to the
    inner edge at inner_edge_row are always included.
    """
    grey = img.convert("L").filter(ImageFilter.BoxBlur(1))
    w, h = grey.size
    px = grey.load()
    energy = [0.0] * w
    for y0, y1 in DETECT_BANDS:
        y0, y1 = min(y0, h), min(y1, h)
        if y1 <= y0:
            continue
        for x in range(w):
            g = sum(abs(px[min(w - 1, x + 2), y] - px[max(0, x - 2), y]) for y in range(y0, y1)) / (y1 - y0)
            energy[x] = max(energy[x], g)
    k = DETECT_SMOOTH
    smooth = [sum(energy[max(0, x - k):x + k + 1]) / len(energy[max(0, x - k):x + k + 1]) for x in range(w)]
    mask = [v > DETECT_THRESHOLD for v in smooth]
    if inner_edge_row is not None:
        left, right = inner_edge(img, inner_edge_row)
        for x in range(0, left):
            mask[x] = True
        for x in range(right, w):
            mask[x] = True
    spans = []
    start = None
    for x, m in enumerate(mask + [False]):
        if m and start is None:
            start = x
        elif not m and start is not None:
            spans.append([start, x])
            start = None
    merged = []
    for a, b in spans:
        if merged and a - merged[-1][1] < DETECT_MERGE_GAP:
            merged[-1][1] = b
        else:
            merged.append([a, b])
    return [tuple(s) for s in merged]


def inner_edge(img: Image.Image, row: int, dark: int = 30) -> tuple[int, int]:
    """First dark column from the middle outwards on row: (left_edge, right_edge) texels."""
    rgb = img.convert("RGB")
    w, _ = rgb.size
    px = rgb.load()
    lit = [sum(px[x, row]) > dark for x in range(w)]
    mid = w // 2
    left = next((x + 1 for x in range(mid, -1, -1) if lit[x]), 0)
    right = next((x for x in range(mid, w) if lit[x]), w)
    return left, right


def remap_segments(width: int, spans: list[tuple[int, int]], stretch_factor: float) -> list[tuple[int, int, int, int]]:
    """Piecewise horizontal map: list of (src_x0, src_x1, dst_x0, dst_x1) covering [0, width).

    Ornament spans shrink by 1/M; a span starting at 0 or ending at width stays
    pinned there, any other span keeps its centre. Rails fill the gaps linearly.
    """
    spans = sorted(spans)
    placed = []
    for x0, x1 in spans:
        if not (0 <= x0 < x1 <= width):
            raise ValueError(f"bad span {(x0, x1)} for width {width}")
        w = round((x1 - x0) / stretch_factor)
        if x0 == 0:
            d0 = 0
        elif x1 == width:
            d0 = width - w
        else:
            d0 = round((x0 + x1) / 2 - w / 2)
        placed.append((x0, x1, d0, d0 + w))
    segs = []
    cursor_s, cursor_d = 0, 0
    for x0, x1, d0, d1 in placed:
        if x0 > cursor_s:
            if d0 <= cursor_d:
                raise ValueError(f"rail before span {(x0, x1)} would have no width")
            segs.append((cursor_s, x0, cursor_d, d0))
        elif d0 != cursor_d:
            raise ValueError(f"span {(x0, x1)} overlaps the previous one")
        segs.append((x0, x1, d0, d1))
        cursor_s, cursor_d = x1, d1
    if cursor_s < width:
        if cursor_d >= width:
            raise ValueError("last rail would have no width")
        segs.append((cursor_s, width, cursor_d, width))
    elif cursor_d != width:
        raise ValueError("segments do not cover the output")
    return segs


def map_x(x: float, segs: list[tuple[int, int, int, int]]) -> float:
    for s0, s1, d0, d1 in segs:
        if s0 <= x <= s1:
            return d0 + (x - s0) * (d1 - d0) / (s1 - s0)
    raise ValueError(f"x={x} outside the map")


def remap_frame(img: Image.Image, spans: list[tuple[int, int]], stretch_factor: float) -> Image.Image:
    w, h = img.size
    src = img.convert("RGBA")
    out = Image.new("RGBA", (w, h))
    for s0, s1, d0, d1 in remap_segments(w, spans, stretch_factor):
        piece = src.resize((d1 - d0, h), Image.LANCZOS, box=(s0, 0, s1, h))
        out.paste(piece, (d0, 0))
    return out


def frame_preview(original: Image.Image, output: Image.Image, aspect: float) -> Image.Image:
    """Original and output side by side, each resized to the screen aspect."""
    ph = 400
    pw = round(ph * aspect)
    a = original.convert("RGB").resize((pw, ph), Image.LANCZOS)
    b = output.convert("RGB").resize((pw, ph), Image.LANCZOS)
    canvas = Image.new("RGB", (pw * 2 + 30, ph + 30), (255, 255, 255))
    canvas.paste(a, (0, 30))
    canvas.paste(b, (pw + 30, 30))
    d = ImageDraw.Draw(canvas)
    d.text((6, 8), "original, as stretched on screen", fill=(0, 0, 0))
    d.text((pw + 36, 8), "remapped, as it would show on screen", fill=(0, 0, 0))
    return canvas


def scale_spans(spans: list[tuple[int, int]], factor: float) -> list[tuple[int, int]]:
    """Spans in source texels -> spans in an upscaled texture's texels."""
    return [(round(x0 * factor), round(x1 * factor)) for x0, x1 in spans]


def require_opaque(img: Image.Image, name: str) -> None:
    """The upscaler drops alpha; refuse a texture that has any."""
    lo, hi = img.convert("RGBA").getchannel("A").getextrema()
    if (lo, hi) != (255, 255):
        raise ValueError(f"{name}: alpha {lo}..{hi} is not fully opaque; the upscaler would drop it")


def run_realesrgan(binary: Path, model: str, native_scale: int, src_png: Path, out_png: Path, gpu: int | None) -> None:
    cmd = [str(binary), "-i", str(src_png), "-o", str(out_png), "-n", model, "-s", str(native_scale),
           "-m", str(Path(binary).parent / "models"), "-f", "png"]
    if gpu is not None:
        cmd += ["-g", str(gpu)]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0 or not out_png.exists():
        raise RuntimeError(f"{Path(binary).name} {model} failed (rc {res.returncode}): {res.stderr.strip()[-300:]}")


def upscale_factor(width: int, target_width: int | None) -> int:
    """UPSCALE_FACTOR, or target_width / width, which must be an integer >= 2."""
    if target_width is None:
        return UPSCALE_FACTOR
    factor = target_width / width
    if factor != int(factor) or factor < 2:
        raise ValueError(f"target width {target_width} is not an integer multiple >= 2 of {width}")
    return int(factor)


def run_model_rgb(rgb: Image.Image, tag: str, model: str, binary: Path, workdir: Path, gpu: int | None,
                  factor: int) -> Image.Image:
    """One RGB image through the model at its native scale, then LANCZOS to factor x."""
    native = UPSCALE_MODELS[model]
    w, h = rgb.size
    src_png = workdir / f"{tag}.src.png"
    out_png = workdir / f"{tag}.{model}.x{native}.png"
    rgb.convert("RGB").save(src_png)
    run_realesrgan(binary, model, native, src_png, out_png, gpu)
    with Image.open(out_png) as raw:
        up = raw.convert("RGB")
    if up.size != (w * native, h * native):
        raise RuntimeError(f"{model} returned {up.size}, expected {(w * native, h * native)}")
    target = (w * factor, h * factor)
    if up.size != target:
        up = up.resize(target, Image.LANCZOS)
    return up


def upscale_image(img: Image.Image, name: str, model: str, binary: Path, workdir: Path,
                  gpu: int | None = None, target_width: int | None = None) -> Image.Image:
    """Upscale an opaque texture with a Real-ESRGAN model (RGBA, alpha 255): by
    UPSCALE_FACTOR, or to exactly target_width wide (see upscale_factor)."""
    require_opaque(img, name)
    factor = upscale_factor(img.width, target_width)
    return run_model_rgb(img, name, model, binary, workdir, gpu, factor).convert("RGBA")


def bleed_rgb(img: Image.Image) -> Image.Image:
    """RGB where every alpha-0 texel takes the colour of the nearest texel with alpha > 0
    (multi-source BFS, 4-neighbour). Without it the model and the resampler pull the
    (usually black) RGB of transparent texels into the visible edge: dark fringes."""
    rgba = img.convert("RGBA")
    w, h = rgba.size
    if w * h > ALPHA_MAX_TEXELS:
        raise ValueError(f"{w}x{h} has more than {ALPHA_MAX_TEXELS} texels, the limit for the RGB bleed "
                         f"(pure-Python BFS); textures with alpha must be at most 512x512")
    raw = rgba.tobytes()
    px = [tuple(raw[i:i + 4]) for i in range(0, len(raw), 4)]
    rgb = [p[:3] for p in px]
    seen = [p[3] > 0 for p in px]
    frontier = [i for i in range(w * h) if seen[i]]
    if not frontier:
        return rgba.convert("RGB")
    while frontier:
        nxt = []
        for i in frontier:
            x, y = i % w, i // w
            for j, ok in ((i - 1, x > 0), (i + 1, x < w - 1), (i - w, y > 0), (i + w, y < h - 1)):
                if ok and not seen[j]:
                    seen[j] = True
                    rgb[j] = rgb[i]
                    nxt.append(j)
        frontier = nxt
    out = Image.new("RGB", (w, h))
    out.putdata(rgb)
    return out


def alpha_divergence(a: Image.Image, b: Image.Image) -> dict:
    """Mean and max absolute difference of two L images, in 0..255 units."""
    hist = ImageChops.difference(a, b).histogram()
    total = sum(hist)
    return {"mean": round(sum(v * n for v, n in enumerate(hist)) / total, 3),
            "max": max(v for v, n in enumerate(hist) if n)}


def upscale_alpha_image(img: Image.Image, name: str, model: str, binary: Path, workdir: Path,
                        gpu: int | None = None, target_width: int | None = None,
                        alpha: str = "model") -> tuple[Image.Image, dict]:
    """Upscale a texture with alpha. RGB (bled, see bleed_rgb) goes through the model.
    Alpha: "model" runs (A,A,A) through the model and takes channel 0, unless it
    diverges from the LANCZOS alpha by more than ALPHA_DIVERGENCE, in which case this
    texture uses "lanczos" (a plain LANCZOS resize of A). Returns (RGBA, info)."""
    if alpha not in ALPHA_MODES:
        raise ValueError(f"alpha mode {alpha!r}: choose from {ALPHA_MODES}")
    rgba = img.convert("RGBA")
    factor = upscale_factor(rgba.width, target_width)
    target = (rgba.width * factor, rgba.height * factor)
    rgb_up = run_model_rgb(bleed_rgb(rgba), name, model, binary, workdir, gpu, factor)
    a = rgba.getchannel("A")
    a_lanczos = a.resize(target, Image.LANCZOS)
    info = {"alpha_mode": alpha, "alpha_mode_used": "lanczos", "alpha_divergence": None}
    a_used = a_lanczos
    if alpha == "model":
        a_model = run_model_rgb(Image.merge("RGB", (a, a, a)), f"{name}.alpha", model, binary, workdir, gpu,
                                factor).getchannel("R")
        div = alpha_divergence(a_model, a_lanczos)
        info["alpha_divergence"] = div
        if div["mean"] > ALPHA_DIVERGENCE["mean"] or div["max"] > ALPHA_DIVERGENCE["max"]:
            info["note"] = f"model alpha diverged (mean {div['mean']}, max {div['max']}); using lanczos"
        else:
            a_used = a_model
            info["alpha_mode_used"] = "model"
    r, g, b = rgb_up.split()
    return Image.merge("RGBA", (r, g, b, a_used)), info


def _label(canvas: Image.Image, xy, text: str) -> None:
    ImageDraw.Draw(canvas).text(xy, text, fill=(255, 255, 0))


def candidates_sheet(vanilla: Image.Image, candidates: dict[str, Image.Image], box: tuple[int, int, int, int]) -> Image.Image:
    """Crops of one ornament at 2x: vanilla (LANCZOS 2x, what plain scaling gives) and
    each upscaled candidate at 100%."""
    x0, y0, x1, y1 = box
    cw, ch = (x1 - x0) * UPSCALE_FACTOR, (y1 - y0) * UPSCALE_FACTOR
    tiles = [("vanilla, LANCZOS 2x", vanilla.convert("RGB").crop(box).resize((cw, ch), Image.LANCZOS))]
    for model, up in candidates.items():
        f = up.width / vanilla.width
        tiles.append((f"{model} (100%)", up.convert("RGB").crop(tuple(round(v * f) for v in box))))
    canvas = Image.new("RGB", (len(tiles) * (cw + 10), ch + 24), (40, 40, 40))
    for i, (label, tile) in enumerate(tiles):
        canvas.paste(tile, (i * (cw + 10), 24))
        _label(canvas, (i * (cw + 10) + 4, 6), label)
    return canvas


SHEET_CROP = 512
SHEET_DETAIL_PER_GROUP = 2
SHEET_THUMB_H = 100


def loadscreen_sample(name: str, kind: str, img: Image.Image, base: Image.Image, out_img: Image.Image,
                      screen: tuple[int, int], earlier: list[dict]) -> dict:
    """What the loading-screen sheet keeps of one upscaled texture: a thumbnail of the
    pillarboxed output at the screen aspect and, for the first SHEET_DETAIL_PER_GROUP
    of each (source kind, width), a centre crop of vanilla LANCZOS vs the upscale at 100%."""
    thumb = out_img.convert("RGB").resize((round(SHEET_THUMB_H * screen[0] / screen[1]), SHEET_THUMB_H), Image.LANCZOS)
    sample = {"name": name, "kind": kind, "width": img.width, "thumb": thumb, "detail": None}
    group = [s for s in earlier if s["detail"] and (s["kind"], s["width"]) == (kind, img.width)]
    if len(group) < SHEET_DETAIL_PER_GROUP:
        f = base.width / img.width
        c = SHEET_CROP / f
        x0, y0 = (img.width - c) / 2, (img.height - c) / 2
        box = tuple(round(v) for v in (x0, y0, x0 + c, y0 + c))
        vanilla = img.convert("RGB").crop(box).resize((SHEET_CROP, SHEET_CROP), Image.LANCZOS)
        up = base.convert("RGB").crop(tuple(round(v * f) for v in box)).resize((SHEET_CROP, SHEET_CROP), Image.LANCZOS)
        sample["detail"] = (vanilla, up, f)
    return sample


def loadscreens_sheet(samples: list[dict], cols: int = 9) -> Image.Image:
    """Detail rows (vanilla LANCZOS | upscale, centre crops at the output's 100%), then a
    contact sheet of every upscaled, pillarboxed output."""
    details = [s for s in samples if s["detail"]]
    tw, th = samples[0]["thumb"].size
    rows = -(-len(samples) // cols)
    width = max(2 * SHEET_CROP + 30, cols * (tw + 6))
    height = len(details) * (SHEET_CROP + 24) + 30 + rows * (th + 18)
    canvas = Image.new("RGB", (width, height), (40, 40, 40))
    y = 0
    for s in details:
        vanilla, up, f = s["detail"]
        _label(canvas, (4, y + 6), f"{s['name']} ({s['kind']}, {s['width']} wide): vanilla LANCZOS {f:g}x")
        _label(canvas, (SHEET_CROP + 34, y + 6), "upscaled (100%)")
        canvas.paste(vanilla, (0, y + 24))
        canvas.paste(up, (SHEET_CROP + 30, y + 24))
        y += SHEET_CROP + 24
    _label(canvas, (4, y + 10), "every upscaled loading screen, pillarboxed, at the screen aspect")
    y += 30
    for i, s in enumerate(samples):
        x, yy = (i % cols) * (tw + 6), y + (i // cols) * (th + 18)
        _label(canvas, (x + 2, yy), s["name"])
        canvas.paste(s["thumb"], (x, yy + 14))
    return canvas


def txi_additive(txi: str) -> bool:
    """TXI `blending additive` (or `blending 1`): the engine adds the texture to the screen."""
    for line in txi.lower().splitlines():
        parts = line.split()
        if len(parts) >= 2 and parts[0] == "blending" and parts[1] in ("1", "additive"):
            return True
    return False


SHEET_ALPHA_TEST = 128              # a punchthrough texel is drawn when its alpha is at least this


def txi_punchthrough(txi: str) -> bool:
    """TXI `blending punchthrough`: the engine alpha-tests the texture (the only spelling in
    the vanilla TexturePacks: 130 textures)."""
    return txi_fields(txi).get("blending", "").split()[:1] == ["punchthrough"]


def composite(tile: Image.Image, bg: tuple[int, int, int], size: tuple[int, int], additive: bool,
              alpha_test: bool = False) -> Image.Image:
    """tile over a flat background: alpha blended, added (RGB x alpha) when additive, or
    alpha-tested at SHEET_ALPHA_TEST (opaque or absent, as the engine draws punchthrough)."""
    base = Image.new("RGB", size, bg)
    rgba = tile.convert("RGBA")
    if alpha_test:
        rgba.putalpha(rgba.getchannel("A").point(lambda a: 255 if a >= SHEET_ALPHA_TEST else 0))
    if additive:
        black = Image.new("RGB", rgba.size, (0, 0, 0))
        glow = Image.composite(rgba.convert("RGB"), black, rgba.getchannel("A"))
        piece = ImageChops.add(base.crop((6, 6, 6 + rgba.width, 6 + rgba.height)), glow)
        base.paste(piece, (6, 6))
        return base
    out = base.convert("RGBA")
    out.alpha_composite(rgba, (6, 6))
    return out.convert("RGB")


def set_sheet(items: list[tuple[str, Image.Image, Image.Image, dict, str]], sizes: dict[str, tuple[int, int]],
              resample=Image.LANCZOS) -> Image.Image:
    """Each texture at its on-screen size over a dark and a light background: vanilla,
    the recorded upscale (labelled with its alpha mode), and the same RGB with LANCZOS
    alpha. Items are (name, vanilla, upscale, info, txi); an additive TXI is drawn
    added to the background, a punchthrough one alpha-tested. A tile bigger than SHEET_TILE_MAX keeps its top-left
    SHEET_TILE_MAX square at on-screen scale (no minification)."""
    bgs = ((24, 24, 28), (205, 200, 190))
    tiles_per_item = []
    for name, vanilla, up, info, txi in items:
        w, h = sizes.get(name) or up.size
        up_lanczos = up.copy()
        up_lanczos.putalpha(vanilla.getchannel("A").resize(up.size, Image.LANCZOS))
        crop = (0, 0, min(w, SHEET_TILE_MAX), min(h, SHEET_TILE_MAX))
        tiles_per_item.append([img.resize((w, h), resample).crop(crop) for img in (vanilla, up, up_lanczos)])
    cw = max(t[0].width for t in tiles_per_item) + 12
    heads = ["vanilla", "upscaled", "upscaled, lanczos alpha"]
    heights = [t[0].height + 12 for t in tiles_per_item]
    canvas = Image.new("RGB", (3 * len(bgs) * cw + 10, 20 + sum(h + 18 for h in heights)), (60, 60, 60))
    for b in range(len(bgs)):
        for j, head in enumerate(heads):
            _label(canvas, ((b * 3 + j) * cw + 4, 4), head)
    y = 20
    for (name, vanilla, up, info, txi), tiles, ch in zip(items, tiles_per_item, heights):
        additive, tested = txi_additive(txi), txi_punchthrough(txi)
        w, h = sizes.get(name) or up.size
        _label(canvas, (4, y), f"{name} {vanilla.width}x{vanilla.height} -> {up.width}x{up.height}, on screen {w}x{h} px"
                               + (" (cropped)" if max(w, h) > SHEET_TILE_MAX else "")
                               + f": alpha {info.get('alpha_mode_used', '?')}"
                               + (f" (divergence mean {info['alpha_divergence']['mean']}, max {info['alpha_divergence']['max']})"
                                  if info.get("alpha_divergence") else "")
                               + (", additive" if additive else "")
                               + (", alpha-tested" if tested else "")
                               + (f"; {info['treatment']}" if info.get("treatment") else "")
                               + (f"; flags: {', '.join(info['flags'])}" if info.get("flags") else ""))
        for b, bg in enumerate(bgs):
            for j, tile in enumerate(tiles):
                canvas.paste(composite(tile, bg, (cw, ch), additive, tested), ((b * 3 + j) * cw, y + 14))
        y += ch + 18
    return canvas


def screen_crop(img: Image.Image, box_src: tuple[int, int, int, int], src_width: int,
                screen: tuple[int, int]) -> Image.Image:
    """A region (in vanilla texel coords) of a full-screen texture as it lands on screen."""
    f = img.width / src_width
    x0, y0, x1, y1 = box_src
    crop = img.convert("RGB").crop((round(x0 * f), round(y0 * f), round(x1 * f), round(y1 * f)))
    sx, sy = screen[0] / src_width, screen[1] / src_width
    return crop.resize((round((x1 - x0) * sx), round((y1 - y0) * sy)), Image.LANCZOS)


def frame_sheet(vanilla: Image.Image, remap_1x: Image.Image, remap_up: Image.Image | None,
                screen: tuple[int, int], box: tuple[int, int, int, int], title: str) -> Image.Image:
    """Approval sheet: full frame before/after at the screen aspect, plus the corner
    ornament at on-screen pixel size for the 1x remap and (if given) the upscaled remap."""
    top = frame_preview(vanilla, remap_up if remap_up is not None else remap_1x, screen[0] / screen[1])
    crops = [("remap of the vanilla 1024 texture, on-screen pixels", screen_crop(remap_1x, box, vanilla.width, screen))]
    if remap_up is not None:
        crops.append((f"remap of the {remap_up.width} upscale, on-screen pixels",
                      screen_crop(remap_up, box, vanilla.width, screen)))
    cw = max(c.width for _, c in crops)
    chh = max(c.height for _, c in crops)
    width = max(top.width, len(crops) * (cw + 20))
    canvas = Image.new("RGB", (width, 24 + top.height + 30 + chh), (40, 40, 40))
    _label(canvas, (6, 6), title)
    canvas.paste(top, (0, 24))
    y = 24 + top.height + 30
    for i, (label, c) in enumerate(crops):
        canvas.paste(c, (i * (cw + 20), y))
        _label(canvas, (i * (cw + 20) + 4, y - 18), label)
    return canvas


# ---------------------------------------------------------------- texture I/O

# TPC header: uint32 data size (0 = uncompressed), float alpha test, uint16 width and
# height, uint8 pixel type (1 grey, 2 RGB/DXT1, 4 RGBA/DXT5), uint8 mip count; 128 bytes.
# The TXI text follows the texture data. A cube map is stored 6 faces tall.
TPC_HEADER = 128


def tpc_layout(data: bytes) -> dict:
    """Size, encoding, mip count, layers and the TXI offset of a TPC, from its header
    (checked against pykotor's read_tpc on every TexturePacks and override TPC of this
    install). Refuses, as read_tpc does, a compressed header whose data size overruns the file."""
    size, _, w, h, ptype, mips = struct.unpack_from("<IfHHBB", data, 0)
    compressed = size != 0
    fmt = {(True, 2): "DXT1", (True, 4): "DXT5", (False, 1): "GREY", (False, 2): "RGB", (False, 4): "RGBA"}.get(
        (compressed, ptype))
    if fmt is None:
        raise ValueError(f"unknown TPC pixel type {ptype} (data size {size})")
    layers = 1
    if h and h == 6 * w:
        layers, h = 6, w
    total = level0 = 0
    for i in range(mips):
        mw, mh = max(1, w >> i), max(1, h >> i)
        if compressed:
            n = max(1, (mw + 3) // 4) * max(1, (mh + 3) // 4) * (8 if fmt == "DXT1" else 16)
        else:
            n = mw * mh * {"GREY": 1, "RGB": 3, "RGBA": 4}[fmt]
        level0 = level0 or n
        total += n
    # read_tpc sizes level 0 from the header's data size, not from the dimensions: a header
    # that claims more than the file holds is unreadable (vanilla script_c/script_h: 4x4 DXT1,
    # data size 16, 3 mips -> 16 + 8 + 8 bytes where the file has 24)
    if compressed and TPC_HEADER + layers * (size + total - level0) > len(data):
        raise ValueError(f"TPC header data size {size} overruns the file ({len(data)} bytes)")
    return {"width": w, "height": h, "format": fmt, "mips": mips, "layers": layers,
            "txi_offset": TPC_HEADER + layers * total}


def tpc_raw_txi(data: bytes) -> str:
    """The TPC's TXI exactly as stored. pykotor's .txi re-serializes it and changes
    values the engine reads ("blending additive" -> "blending 1", "clamp 3" -> "clamp 1")."""
    off = tpc_layout(data)["txi_offset"]
    if off > len(data):
        raise ValueError(f"TPC is {len(data)} bytes, its texture data ends at {off}")
    return data[off:].decode("ascii", "replace").replace("\x00", "").strip()


def tpc_to_image(data: bytes) -> tuple[Image.Image, str]:
    txi = tpc_raw_txi(data)
    t = read_tpc(data)
    t.convert(TPCTextureFormat.RGBA)
    mm = t.get(0, 0)
    return Image.frombytes("RGBA", (mm.width, mm.height), bytes(mm.data)), txi


def image_to_tpc(img: Image.Image, txi: str = "") -> bytes:
    """An uncompressed RGBA TPC (with pykotor's mip chain), the TXI appended verbatim."""
    rgba = img.convert("RGBA")
    t = TPC()
    t.set_single(rgba.tobytes(), TPCTextureFormat.RGBA, rgba.width, rgba.height)
    data = bytes_tpc(t)
    return data + (txi_bytes(txi) if txi.strip() else b"")


def dxt_blocks(img: Image.Image, fmt: str) -> bytes:
    """One level's DXT1/DXT5 blocks, from Pillow's DDS (BC1/BC3) encoder."""
    buf = io.BytesIO()
    img.convert("RGBA").save(buf, format="DDS", pixel_format=fmt)
    raw = buf.getvalue()
    return raw[128 + (20 if raw[84:88] == b"DX10" else 0):]


def world_mip_count(src_mips: int, factor: int, width: int, height: int) -> int:
    """The source's mip count plus log2(factor), capped at the full chain; a single-level
    source (TXI mipmap 0) stays single-level."""
    if src_mips <= 1:
        return 1
    full = max(width, height).bit_length()
    return min(full, src_mips + factor.bit_length() - 1)


def image_to_world_tpc(img: Image.Image, fmt: str, mips: int, txi: str = "") -> bytes:
    """A block-compressed (DXT1/DXT5) or uncompressed (RGB/RGBA, WORLD_UNCOMPRESSED) TPC
    with `mips` levels (each a LANCZOS resize of img), the TXI appended verbatim. The
    header float at byte 4 is level 0's mean alpha / 255, as in vanilla: over the 323
    vanilla alpha-format sources it deviates from that by median 0.005 (max 0.044), and it
    is exactly 1.0 on every DXT1/RGB and fully opaque source (pykotor writes 1.0 always)."""
    from pykotor.resource.formats.tpc.tpc_data import TPCLayer, TPCMipmap
    if fmt not in ("DXT1", "DXT5") + WORLD_UNCOMPRESSED:
        raise ValueError(f"world textures are written as DXT1, DXT5, RGB or RGBA, not {fmt}")
    rgba = img.convert("RGBA")
    tfmt = getattr(TPCTextureFormat, fmt)
    layer = TPCLayer()
    for i in range(mips):
        w, h = max(1, rgba.width >> i), max(1, rgba.height >> i)
        level = rgba if i == 0 else rgba.resize((w, h), Image.LANCZOS)
        texels = level.convert(fmt).tobytes() if fmt in WORLD_UNCOMPRESSED else dxt_blocks(level, fmt)
        layer.mipmaps.append(TPCMipmap(w, h, tfmt, bytearray(texels)))
    t = TPC()
    t.layers = [layer]
    t._format = tfmt
    t.is_cube_map = False
    t.is_animated = False
    data = bytearray(bytes_tpc(t))
    mean_alpha = 1.0 if fmt in ("DXT1", "RGB") else _fmean(rgba.getchannel("A").convert("F")) / 255
    struct.pack_into("<f", data, 4, mean_alpha)
    return bytes(data) + (txi_bytes(txi) if txi.strip() else b"")


def txi_bytes(txi: str) -> bytes:
    """A TXI sidecar file: the source's TXI lines, CRLF-terminated like the TPC-embedded text."""
    return ("\r\n".join(line.strip() for line in txi.strip().splitlines()) + "\r\n").encode("ascii", "replace")


def image_to_tga(img: Image.Image) -> bytes:
    buf = io.BytesIO()
    img.convert("RGBA").save(buf, format="TGA", orientation=-1)
    return buf.getvalue()


def tpc_roundtrip_ok(data: bytes, img: Image.Image) -> bool:
    try:
        back, _ = tpc_to_image(data)
    except Exception:
        return False
    return back.size == img.size and back.tobytes() == img.convert("RGBA").tobytes()


class ErfIndex:
    """Lazy lookup of texture resources across the TexturePacks ERFs, in ERF_ORDER."""

    def __init__(self, texpacks: Path):
        self.texpacks = texpacks
        self._index = None

    def _load(self):
        self._index = {}
        for name in ERF_ORDER:
            path = find_ci(self.texpacks, name)
            if path is None:
                continue
            for res in read_erf(path):
                key = str(res.resref).lower()
                if key not in self._index and res.restype.extension.lower() in ("tpc", "tga"):
                    self._index[key] = (name, res.restype.extension.lower(), res.data)

    def get(self, name: str):
        if self._index is None:
            self._load()
        return self._index.get(name.lower())

    def names(self) -> list[str]:
        if self._index is None:
            self._load()
        return sorted(self._index)


def card_textures(erfs) -> list[str]:
    """The PC pazaak card faces in the ERFs: CARD_PREFIX names without the Xbox suffix."""
    return [n for n in erfs.names() if n.startswith(CARD_PREFIX) and not n.endswith(CARD_XBOX_SUFFIX)]


# ---------------------------------------------------------------- GUI texture scan

def pc_gui_templates(game_dir: Path, override_dir: Path) -> dict[str, bytes]:
    """Every non-Xbox GUI: the live override copy first, else chitin's."""
    chitin_gui_bytes(game_dir, "")  # build the index
    names = set(_CHITIN_GUI_CACHE.get(str(game_dir), {}))
    if override_dir.is_dir():
        names |= {p.stem.lower() for p in override_dir.iterdir() if p.suffix.lower() == ".gui"}
    out = {}
    for name in sorted(names):
        if name.endswith(GUI_XBOX_SUFFIX):
            continue
        live = find_ci(override_dir, f"{name}.gui")
        data = live.read_bytes() if live is not None else chitin_gui_bytes(game_dir, name)
        if data:
            out[name] = data
    return out


def gui_scale(name: str, root_extent: tuple[int, int, int, int], width: int, height: int,
              hud: str) -> tuple[float | None, float | None, str]:
    """(x, y screen px per virtual unit, kind) of a GUI, or (None, None, reason) for a GUI
    not drawn at this resolution. kind: menu (800x600 root, stretched to the screen),
    hud (the HUD in use, 1:1), popup (a smaller root; menu scale assumed, not measured)."""
    left, top, w, h = root_extent
    if name == hud:
        return height / 600, height / 600, "hud"
    if HUD_GUI_PATTERN.match(name):
        return None, None, "the HUD of another resolution"
    if (left, top, w, h) == (0, 0, 800, 600):
        return width / 800, height / 600, "menu"
    if (left, top) == (0, 0) and w * 3 == h * 4:
        return None, None, f"a {w}x{h} full screen: another resolution"
    return width / 800, height / 600, "popup"


def gui_texture_uses(root) -> list[tuple[str, str, str, int, int]]:
    """(control, STATE.FIELD, resref, width, height) of every texture on the root, its
    controls and their PROTOITEM/SCROLLBAR."""
    out = []

    def walk(s, label):
        if not s.exists("EXTENT"):
            return
        e = s.get_struct("EXTENT")
        w, h = e.get_int32("WIDTH"), e.get_int32("HEIGHT")
        for state in SCAN_STATES:
            if not s.exists(state):
                continue
            b = s.get_struct(state)
            for field in ("FILL", "EDGE", "CORNER"):
                if b.exists(field):
                    ref = str(b.get_resref(field)).lower()
                    if ref:
                        out.append((label, f"{state}.{field}", ref, w, h))
        for sub in ("PROTOITEM", "SCROLLBAR"):
            if s.exists(sub):
                walk(s.get_struct(sub), f"{label}.{sub}")
        if s.exists("CONTROLS"):
            for i, c in enumerate(s.get_list("CONTROLS")):
                walk(c, (c.get_string("TAG") if c.exists("TAG") else "") or f"#{i}")

    walk(root, "(root)")
    return out


def is_flat(img: Image.Image, tolerance: int = SCAN_FLAT_TOLERANCE) -> bool:
    """Every texel within +-tolerance of the mean, in every channel."""
    st = ImageStat.Stat(img.convert("RGBA"))
    return all(hi - m <= tolerance and m - lo <= tolerance for (lo, hi), m in zip(st.extrema, st.mean))


def scan_gui_textures(game_dir: Path, override_dir: Path, width: int, height: int, state: "TextureState",
                      erfs, min_mag: float = SCAN_MIN_MAG, hud: str = HUD_DESC_GUI) -> dict:
    """Discover GUI textures worth upscaling (see SCAN_*). Returns {"guis": {skipped
    GUI: reason}, "textures": [one record per texture used as a FILL, name order]}."""
    fills: dict[str, list[dict]] = {}
    edges: dict[str, list[str]] = {}
    skipped = {}
    for gui, data in pc_gui_templates(game_dir, override_dir).items():
        try:
            root = read_gff(data).root
            e = root.get_struct("EXTENT")
            ext = (e.get_int32("LEFT"), e.get_int32("TOP"), e.get_int32("WIDTH"), e.get_int32("HEIGHT"))
        except Exception as exc:
            skipped[gui] = f"unreadable: {exc}"
            continue
        uses = gui_texture_uses(root)
        for label, field, ref, w, h in uses:
            if not field.endswith(".FILL"):
                edges.setdefault(ref, []).append(f"{gui}.{label}")
        sx, sy, kind = gui_scale(gui, ext, width, height, hud)
        if sx is None:
            skipped[gui] = kind
            continue
        for label, field, ref, w, h in uses:
            if field.endswith(".FILL") and w > 0 and h > 0:
                fills.setdefault(ref, []).append({"gui": gui, "control": label, "kind": kind,
                                                  "px": (w * sx, h * sy), "root": label == "(root)"})
    manifest = state.manifest()
    textures = []
    for name in sorted(fills):
        uses = fills[name]
        rec = {"name": name, "guis": sorted({u["gui"] for u in uses}), "kinds": sorted({u["kind"] for u in uses}),
               "root_fill": any(u["root"] for u in uses), "owner_set": manifest.get(name, {}).get("set"),
               "candidate": False, "reason": None}
        textures.append(rec)
        if SCAN_NAME_EXCLUDE.search(name):
            rec["reason"] = "font, area map or lightmap"
            continue
        try:
            src = resolve_texture(name, override_dir, erfs, state.originals, state.sources, manifest)
        except (ValueError, FileNotFoundError, RuntimeError) as exc:
            rec["reason"] = f"source: {exc}"
            continue
        if src is None:
            rec["reason"] = "not in override or TexturePacks"
            continue
        img, txi = decode(src)
        tw, th = img.size
        mags = []
        for u in uses:
            mx, my = u["px"][0] / tw, u["px"][1] / th
            mags.append((max(mx, my), "x" if mx >= my else "y", mx, my, u))
        top = max(mags, key=lambda m: m[0])
        rec.update({
            "size": [tw, th], "source": src["kind"], "alpha": img.getchannel("A").getextrema() != (255, 255),
            "txi": "; ".join(line.strip() for line in txi.strip().splitlines() if line.strip()),
            "max_mag": round(top[0], 3), "axis": top[1], "at": f"{top[4]['gui']}.{top[4]['control']}",
            "screen": [round(top[4]["px"][0]), round(top[4]["px"][1])],
            "popup_only": {u["kind"] for u in uses} == {"popup"},
        })
        if name in manifest:
            entry = manifest[name]
            rec["reason"] = "live in manifest_tex (" + (f"set {entry['set']}" if entry.get("set") else
                            f"upscaled with {entry['upscale_model']}" if entry.get("upscale_model") else
                            "remapped by another pass") + ")"
        elif name in edges:
            rec["reason"] = f"also an EDGE/CORNER ({edges[name][0]}{', ...' if len(edges[name]) > 1 else ''})"
        elif min(tw, th) <= SCAN_TILE_MAX_SIDE and any(max(m[2], m[3]) > SCAN_TILE_STRETCH for m in mags):
            rec["reason"] = f"tile-like: {tw}x{th} stretched over {SCAN_TILE_STRETCH:g}x"
        elif is_flat(img):
            rec["reason"] = "flat colour"
        elif top[0] < min_mag:
            rec["reason"] = f"magnification {top[0]:.2f} < {min_mag:g}"
        else:
            rec["candidate"] = True
    return {"guis": skipped, "textures": textures}


def scan_table(result: dict) -> list[str]:
    """The scan as text: candidates first, then the excluded textures with their reasons."""
    lines = []
    for title, want in (("candidates", True), ("excluded", False)):
        rows = [t for t in result["textures"] if t["candidate"] == want]
        lines.append(f"{title} ({len(rows)}):")
        for t in rows:
            size = "x".join(map(str, t.get("size", ["?"])))
            mag = f"{t['max_mag']:.2f}{t['axis']}" if "max_mag" in t else "-"
            flags = ("alpha" if t.get("alpha") else "opaque") + (" popup" if t.get("popup_only") else "") \
                + (" root" if t["root_fill"] else "")
            lines.append(f"  {t['name']:<20} {size:>9} {flags:<18} mag {mag:<7} at {t.get('at', '-'):<34} "
                         f"guis {len(t['guis']):>2}  txi [{t.get('txi', '')}]"
                         + (f"  -- {t['reason']}" if t["reason"] else ""))
    return lines


def load_scan(state: "TextureState") -> dict:
    path = state.state_dir / SCAN_FILE
    if not path.exists():
        raise ValueError(f"{path} is missing; run `scan` first")
    return json.loads(path.read_text(encoding="utf-8"))


def galaxy_icon_textures(game_dir: Path, override_dir: Path) -> list[str]:
    """The planet icon fills (GALAXY_ICON_PREFIXES) of the galaxy map GUI, live copy first."""
    live = find_ci(override_dir, f"{GALAXY_GUI}.gui")
    data = live.read_bytes() if live is not None else chitin_gui_bytes(game_dir, GALAXY_GUI)
    if not data:
        return []
    return sorted({ref for _, field, ref, _, _ in gui_texture_uses(read_gff(data).root)
                   if field.endswith(".FILL") and ref.startswith(GALAXY_ICON_PREFIXES)})


def portrait_textures(state: "TextureState", erfs, height: int = 1600) -> tuple[list[str], list[str]]:
    """Override portraits (PORTRAIT_PREFIX) magnified at least SCAN_MIN_MAG in the largest
    portrait control (PORTRAIT_MAX_UNITS), by their pristine source's longest side.
    Returns (names, notes); the notes carry the size survey."""
    names = sorted({p.stem.lower() for p in state.override_dir.iterdir()
                    if p.name.lower().startswith(PORTRAIT_PREFIX) and p.suffix.lower() in (".tga", ".tpc")})
    sizes, notes = {}, []
    manifest = state.manifest()
    for name in names:
        try:
            src = resolve_texture(name, state.override_dir, erfs, state.originals, state.sources, manifest)
        except (ValueError, FileNotFoundError, RuntimeError) as exc:
            notes.append(f"{name}: {exc}")
            continue
        if src is not None:
            sizes[name] = max(texture_size(src))
    screen = PORTRAIT_MAX_UNITS * height / 600
    counts: dict[int, int] = {}
    for s in sizes.values():
        counts[s] = counts.get(s, 0) + 1
    notes.append(f"{len(sizes)} override portraits by longest side: "
                 + ", ".join(f"{s}: {n}" for s, n in sorted(counts.items()))
                 + f"; drawn at most {screen:.0f} px, so members are below {screen / SCAN_MIN_MAG:.0f}")
    out = []
    for name, s in sorted(sizes.items()):
        if screen / s < SCAN_MIN_MAG:
            continue
        if name in PORTRAIT_EXCLUDE:
            notes.append(f"{name} ({s}): excluded, {PORTRAIT_EXCLUDE[name]}")
            continue
        out.append(name)
    return out, notes


def scan_members(state: "TextureState", set_name: str) -> dict[str, dict]:
    """Scan records that are candidates, or were already deployed by this set."""
    return {t["name"]: t for t in load_scan(state)["textures"]
            if t["candidate"] or t.get("owner_set") == set_name}


def set_members(set_name: str, state: "TextureState", erfs, height: int = 1600) -> tuple[list[str], list[str]]:
    """The textures of a named upscale set (SET_NAMES). Returns (names, notes)."""
    game_dir = state.override_dir.parent
    if set_name == "cards":
        return card_textures(erfs), []
    if set_name == "reticles":
        return list(RETICLE_TEXTURES), []
    if set_name == "portraits":
        return portrait_textures(state, erfs, height)
    if set_name == "galaxy_icons":
        icons = galaxy_icon_textures(game_dir, state.override_dir)
        wanted = scan_members(state, set_name)
        return ([n for n in icons if n in wanted],
                [f"{n}: not a scan candidate" for n in icons if n not in wanted])
    if set_name != "ui":
        raise ValueError(f"unknown set {set_name!r}; choose from {', '.join(SET_NAMES)}")
    others = {n: "cards" for n in card_textures(erfs)}
    others.update({n: "galaxy_icons" for n in galaxy_icon_textures(game_dir, state.override_dir)})
    others.update({n: "reticles" for n in RETICLE_TEXTURES})
    others.update({n: "portraits" for n in portrait_textures(state, erfs, height)[0]})
    names, notes = [], []
    for name, t in scan_members(state, "ui").items():
        why = (f"in set {others[name]}" if name in others else
               "a root fill (`frame` or the popup layer)" if t["root_fill"] else
               "popup only: popup scale not measured" if t.get("popup_only") else UI_EXCLUDE.get(name))
        if why:
            notes.append(f"{name}: not in ui, {why}")
        else:
            names.append(name)
    return names, notes


def txi_fields(txi: str) -> dict[str, str]:
    """TXI keyword -> the rest of its line (lower case, first occurrence)."""
    out = {}
    for line in txi.lower().splitlines():
        parts = line.split(None, 1)
        if parts:
            out.setdefault(parts[0], parts[1].strip() if len(parts) > 1 else "")
    return out


def world_exclusion(name: str, layout: dict, kv: dict, refs: dict[str, set]) -> str | None:
    """Why a world texture is not upscaled (None: eligible). refs: texture -> {"bump",
    "env"}, the roles other textures' TXIs give it."""
    if "proceduretype" in kv or "numx" in kv or "defaultwidth" in kv:
        return "animated (TXI proceduretype/flipbook)"
    if kv.get("isbumpmap") == "1" or "bump" in refs.get(name, ()):
        return "bump map"
    if kv.get("cube") == "1" or layout["layers"] > 1:
        return "cube map"
    if "env" in refs.get(name, ()):
        return "environment map target"
    if kv.get("islightmap") == "1" or WORLD_LIGHTMAP_NAME.search(name):
        return "lightmap"
    if name.startswith("fx_"):
        return "effect (fx_)"
    if "wateralpha" in kv:
        return "water surface (wateralpha: its bump map stays vanilla)"
    return None


def world_image_exclusion(img: Image.Image, layout: dict, kv: dict) -> str | None:
    """Why a world texture is not upscaled, judged from its decoded level 0 (None:
    eligible): tiny, a small uncompressed lookup table, or flat (one colour whose alpha
    barely varies). The thresholds' calibration is at WORLD_TINY_SIDE."""
    side = max(img.size)
    if side <= WORLD_TINY_SIDE:
        return f"tiny (<= {WORLD_TINY_SIDE} px)"
    uncompressed = layout["format"] in WORLD_UNCOMPRESSED or kv.get("compresstexture") == "0"
    if uncompressed and side <= WORLD_LOOKUP_SIDE:
        return f"lookup table (uncompressed, <= {WORLD_LOOKUP_SIDE} px)"
    lo, hi = img.getchannel("A").getextrema()
    if is_flat(img.convert("RGB")) and hi - lo <= WORLD_FLAT_ALPHA_RANGE:
        return "flat (one colour)"
    return None


def world_factor(width: int, height: int) -> int:
    """4x for small textures, else 2x, halved until the long side fits WORLD_MAX_SIDE."""
    side = max(width, height)
    factor = 4 if side <= WORLD_SMALL_SIDE else 2
    while factor > 1 and side * factor > WORLD_MAX_SIDE:
        factor //= 2
    return factor


def world_textures(erfs, override_dir: Path, category: str) -> tuple[list[dict], list[dict]]:
    """(eligible, excluded) vanilla-only WORLD_ERF TPCs named category*, each a dict with
    name, layout, txi, kv, data and (excluded) reason. A texture is vanilla-only when no
    override .tga/.tpc/.dds shadows it. The rules from the header and TXI
    (world_exclusion) run first; a texture that passes them is decoded for
    world_image_exclusion (about 1,100 decodes for `all`: about 3.5 minutes)."""
    shadowed = {p.stem.lower() for p in override_dir.iterdir() if p.suffix.lower() in (".tga", ".tpc", ".dds")}
    refs: dict[str, set] = {}
    records = {}
    for name in erfs.names():
        erf, ext, data = erfs.get(name)
        if ext != "tpc":
            continue
        try:
            layout = tpc_layout(data)
            txi = tpc_raw_txi(data)
        except (ValueError, struct.error) as exc:
            if name.startswith(category) and erf == WORLD_ERF:
                records[name] = {"name": name, "reason": f"unreadable: {exc}"}
            continue
        kv = txi_fields(txi)
        for key, role in (("bumpmaptexture", "bump"), ("bumpyshinytexture", "bump"), ("envmaptexture", "env")):
            if kv.get(key):
                refs.setdefault(kv[key].split()[0], set()).add(role)
        if name.startswith(category) and erf == WORLD_ERF:
            records[name] = {"name": name, "layout": layout, "txi": txi, "kv": kv, "data": data}
    eligible, excluded = [], []
    for name in sorted(records):
        rec = records[name]
        if "reason" not in rec:
            rec["reason"] = ("overridden" if name in shadowed else
                             world_exclusion(name, rec["layout"], rec["kv"], refs))
            if rec["reason"] is None and world_factor(rec["layout"]["width"], rec["layout"]["height"]) < 2:
                rec["reason"] = f"already {WORLD_MAX_SIDE} or more"
            if rec["reason"] is None:
                rec["reason"] = world_image_exclusion(tpc_to_image(rec["data"])[0], rec["layout"], rec["kv"])
        (excluded if rec["reason"] else eligible).append(rec)
    return eligible, excluded


def world_state(state: "TextureState") -> "TextureState":
    """The texture state with the world manifest and output directory."""
    ws = TextureState(state.state_dir, state.override_dir)
    ws.manifest_path = state.state_dir / WORLD_MANIFEST
    ws.out = state.state_dir / WORLD_OUT
    ws.out.mkdir(parents=True, exist_ok=True)
    return ws


def world_prefix(category: str) -> str:
    """The texture name prefix of a world category: WORLD_ALL is every name ("")."""
    return "" if category.lower() == WORLD_ALL else category.lower()


def plan_output_world(name: str, src: dict, img: Image.Image, txi: str, mips: int,
                      src_format: str | None = None) -> dict:
    """A world texture's output: <name>.tpc with `mips` levels, the TXI embedded. An
    uncompressed source (src_format RGB/RGBA, or TXI compresstexture 0) stays
    uncompressed, RGB when opaque else RGBA; the others are DXT5 when they have alpha,
    else DXT1. Only ERF sources (vanilla-only textures) are planned."""
    if src["kind"] != "erf":
        raise ValueError(f"{name}: world textures come from {WORLD_ERF} only, not {src['kind']}")
    opaque = img.getchannel("A").getextrema() == (255, 255)
    if src_format in WORLD_UNCOMPRESSED or txi_fields(txi).get("compresstexture") == "0":
        fmt = "RGB" if opaque else "RGBA"
    else:
        fmt = "DXT1" if opaque else "DXT5"
    data = image_to_world_tpc(img, fmt, mips, txi)
    return {"file": f"{name.lower()}.tpc", "data": data, "remove": None, "note": f"{fmt}, {mips} mips",
            "txi": None, "format": fmt, "mips": mips}


def dxt_error(level0: Image.Image, up: Image.Image, fmt: str) -> float:
    """How far the encoded level 0 is from the upscale it was made from: the mean
    absolute difference of each channel (RGB, + A for DXT5), the worst channel, 0..255.
    Uncompressed outputs are exact, so 0."""
    if fmt in WORLD_UNCOMPRESSED:
        return 0.0
    bands = "RGBA" if fmt == "DXT5" else "RGB"
    diff = ImageChops.difference(level0.convert("RGBA"), up.convert("RGBA"))
    means = ImageStat.Stat(diff).mean
    return round(max(means["RGBA".index(b)] for b in bands), 3)


def world_flags(info: dict, kv: dict, fmt: str, err: float, pinfo: dict) -> list[str]:
    """The WORLD_FLAG_RULES a built world texture trips, in priority order. info: its
    upscale record; kv: txi_fields of its TXI; fmt: the output format; err: its
    dxt_error; pinfo: its world_postprocess info."""
    kept = pinfo["grain_kept"]
    hit = {
        "alpha-lanczos": info.get("alpha_mode_used") == "lanczos",
        "blending": kv.get("blending", "default") not in ("default", "0"),
        "decal": kv.get("decal", "0") != "0",
        "uncompressed": fmt in WORLD_UNCOMPRESSED,
        "dxt-error": err > WORLD_DXT_ERROR_FLAG,
        "grain-loss": pinfo["treatment"] == "tone" and kept is not None and kept < WORLD_GRAIN_KEPT_MAX,
        "clamp": kv.get("clamp", "0") != "0",
    }
    return [rule for rule in WORLD_FLAG_RULES if hit[rule]]


# ---------------------------------------------------------------- world post-processing
# RGB arithmetic runs per band in mode "F" (float: signed residuals don't clip); Pillow's
# Gaussian blur takes 8-bit bands only. Alpha is never modified.

def _fmean(band: Image.Image) -> float:
    """The mean of a mode "F" band (ImageStat bins F values into 0..255, so it can't)."""
    return band.resize((1, 1), Image.BOX).getpixel((0, 0))


def _to_l(band: Image.Image) -> Image.Image:
    """A mode "F" band back to "L": rounded, and clamped to 0..255 by the conversion."""
    return ImageMath.lambda_eval(lambda d: d["x"] + 0.5, x=band).convert("L")


def tone_reference(vanilla: Image.Image) -> Image.Image:
    """The vanilla RGB the post-processing compares with, at source size: bled (bleed_rgb)
    when any texel has alpha < 255, so transparent RGB doesn't pull the edges dark."""
    rgba = vanilla.convert("RGBA")
    return bleed_rgb(rgba) if rgba.getchannel("A").getextrema()[0] < 255 else rgba.convert("RGB")


def tone_fix(up: Image.Image, ref: Image.Image, radius_px: float) -> Image.Image:
    """up's detail above radius_px, ref's tone below it: per band up - blur(up) + blur(ref)
    (Gaussian, radius_px output px). ref: the vanilla RGB at up's size. up's alpha is kept."""
    rgba = up.convert("RGBA")
    blur = ImageFilter.GaussianBlur(radius_px)
    u, ub, rb = (im.convert("RGB").split() for im in (rgba, rgba.convert("RGB").filter(blur),
                                                        ref.convert("RGB").filter(blur)))
    bands = [_to_l(ImageMath.lambda_eval(lambda d: d["u"] - d["ub"] + d["rb"],
                                         u=a.convert("F"), ub=b.convert("F"), rb=c.convert("F")))
             for a, b, c in zip(u, ub, rb)]
    return Image.merge("RGBA", (*bands, rgba.getchannel("A")))


def tone_fixed(img: Image.Image, up: Image.Image) -> Image.Image:
    """A non-world upscale (sets, loading screens, frames) with the world tone fix: the model's
    detail above WORLD_TONE_RADIUS source px, the vanilla's tone below (the model darkens:
    galaxy icons median -2.4, loading screens -1.5, measured 2026-09-26). No back-projection:
    GUI art is where the model's smoothing is wanted."""
    ref = tone_reference(img).resize(up.size, Image.LANCZOS)
    return tone_fix(up, ref, WORLD_TONE_RADIUS * up.width / img.width)


def detail_energy(img: Image.Image, radius_px: float) -> float:
    """High-frequency energy: the mean absolute difference of the grey image and its
    Gaussian blur (radius_px), 0..255."""
    grey = img.convert("RGB").convert("L")
    return ImageStat.Stat(ImageChops.difference(grey, grey.filter(ImageFilter.GaussianBlur(radius_px)))).mean[0]


def grain_kept(up: Image.Image, ref: Image.Image, radius_px: float) -> float | None:
    """How much of the vanilla detail the model kept: detail_energy(up) / detail_energy(ref)
    (ref: the vanilla at up's size), or None when vanilla has too little detail to measure
    (below WORLD_GRAIN_MIN_HF)."""
    base = detail_energy(ref, radius_px)
    if base < WORLD_GRAIN_MIN_HF:
        return None
    return detail_energy(up, radius_px) / base


def backproject_guard(name: str, kv: dict) -> str | None:
    """Why a texture must not be back-projected (None: allowed): the TXI marks it as a
    head (clamp), blended (holograms, glows, cut-outs) or a decal, its name is a model
    skin's (WORLD_BACKPROJECT_SKIP_PREFIXES), or it is in WORLD_BACKPROJECT_EXCLUDE. A name in
    WORLD_BACKPROJECT_ALLOW is always allowed."""
    if name in WORLD_BACKPROJECT_ALLOW:
        return None
    if kv.get("clamp", "0") != "0":
        return "clamp (heads)"
    if kv.get("blending", "default") not in ("default", "0"):
        return "blending"
    if kv.get("decal", "0") != "0":
        return "decal"
    for prefix in WORLD_BACKPROJECT_SKIP_PREFIXES:
        if name.startswith(prefix):
            return f"model skin ({prefix})"
    return WORLD_BACKPROJECT_EXCLUDE.get(name)


def back_project(tone: Image.Image, van: Image.Image, iters: int, strength: float) -> Image.Image:
    """Iterative back-projection from the tone-fixed image: `iters` rounds of
    out += BICUBIC_up(van - LANCZOS_down(out)) in float, so that out downscaled matches the
    vanilla (van: the tone_reference at source size); the result is
    tone + strength * (out - tone). Alpha is kept."""
    rgba = tone.convert("RGBA")
    result = []
    for t, v in zip(rgba.convert("RGB").split(), van.convert("RGB").split()):
        t, v = t.convert("F"), v.convert("F")
        out = t
        for _ in range(iters):
            resid = ImageMath.lambda_eval(lambda d: d["v"] - d["o"], v=v, o=out.resize(v.size, Image.LANCZOS))
            out = ImageMath.lambda_eval(lambda d: d["o"] + d["r"], o=out, r=resid.resize(t.size, Image.BICUBIC))
        result.append(_to_l(ImageMath.lambda_eval(lambda d: d["t"] + d["s"] * (d["o"] - d["t"]),
                                                  t=t, o=out, s=float(strength))))
    return Image.merge("RGBA", (*result, rgba.getchannel("A")))


def luma_shift(vanilla: Image.Image, img: Image.Image) -> float:
    """img's mean luma minus vanilla's, img downscaled to vanilla's size (LANCZOS, RGB only:
    Pillow premultiplies RGBA resizes) and both weighted by vanilla's alpha (unweighted
    when vanilla is fully transparent)."""
    van = vanilla.convert("RGBA")
    rgb = img.convert("RGB").resize(van.size, Image.LANCZOS)
    w = van.getchannel("A").convert("F")
    if _fmean(w) * van.width * van.height < 255:
        w = Image.new("F", van.size, 255.0)
    total = _fmean(w)

    def weighted(im):
        return _fmean(ImageMath.lambda_eval(lambda d: d["a"] * d["w"], a=im.convert("L").convert("F"), w=w)) / total

    return weighted(rgb) - weighted(van.convert("RGB"))


def world_postprocess(name: str, vanilla: Image.Image, up: Image.Image, kv: dict,
                      factor: int) -> tuple[Image.Image, dict]:
    """The world output from the model's upscale: tone_fix always (radius WORLD_TONE_RADIUS
    source px), then back_project when the model lost the grain (grain_kept, measured at one
    source px, below WORLD_GRAIN_KEPT_MAX) and backproject_guard allows it. Returns
    (RGBA, info): treatment ("tone" or "backproject"), grain_kept (of the model, and
    grain_kept_out of the result), guard, and the luma shift vs vanilla of the model
    (tone_shift_model) and of the result (tone_shift)."""
    van = tone_reference(vanilla)
    ref = van.resize(up.size, Image.LANCZOS)
    out = tone_fix(up, ref, WORLD_TONE_RADIUS * factor)
    kept = grain_kept(up, ref, factor)
    guard = backproject_guard(name, kv)
    treatment = "backproject" if kept is not None and kept < WORLD_GRAIN_KEPT_MAX and guard is None else "tone"
    if treatment == "backproject":
        out = back_project(out, van, WORLD_BACKPROJECT_ITERS, WORLD_BACKPROJECT_STRENGTH)
    kept_out = grain_kept(out, ref, factor)
    return out, {"treatment": treatment, "grain_kept": None if kept is None else round(kept, 3),
                 "grain_kept_out": None if kept_out is None else round(kept_out, 3), "guard": guard,
                 "tone_shift_model": round(luma_shift(vanilla, up), 3),
                 "tone_shift": round(luma_shift(vanilla, out), 3)}


def world_triage(records: dict[str, dict], eligible: list[str]) -> dict:
    """The flags file: records (name -> {flags, dxt_error, prefix, format, treatment,
    grain_kept, tone_shift}), the textures
    per rule (a texture counts under every rule it trips) and, per 3-letter prefix of the
    eligible names, [eligible, flagged]."""
    by_rule = {rule: sum(rule in r["flags"] for r in records.values()) for rule in WORLD_FLAG_RULES}
    by_prefix: dict[str, list[int]] = {}
    for name in eligible:
        row = by_prefix.setdefault(name[:3], [0, 0])
        row[0] += 1
        row[1] += bool(records.get(name, {}).get("flags"))
    return {"threshold": WORLD_DXT_ERROR_FLAG, "textures": records, "by_rule": by_rule, "by_prefix": by_prefix}


def flagged_order(records: dict[str, dict]) -> list[str]:
    """Flagged names, by their first rule's priority (WORLD_FLAG_RULES order), then name."""
    flagged = [n for n, r in records.items() if r["flags"]]
    return sorted(flagged, key=lambda n: (WORLD_FLAG_RULES.index(records[n]["flags"][0]), n))


def sample_unflagged(records: dict[str, dict], k: int = WORLD_SAMPLE) -> list[str]:
    """A reproducible random sample of the unflagged names (WORLD_SAMPLE_SEED)."""
    unflagged = sorted(n for n, r in records.items() if not r["flags"])
    return random.Random(WORLD_SAMPLE_SEED).sample(unflagged, min(k, len(unflagged)))


def set_screen_sizes(set_name: str, names: list[str], state: "TextureState", width: int, height: int) -> dict:
    """On-screen px (w, h) per texture for the sheet; missing names show at 100%."""
    if set_name == "cards":
        # a card control is 80 units wide in pazaak*_p, 80 / M after --board-4x3, and a
        # virtual unit is width / 800 px wide (211 px at 3840x1600)
        size = round(80 / compute_stretch_factor(width, height) * width / 800)
        return {n: (size, size) for n in names}
    if set_name == "reticles":
        return {n: (RETICLE_SCREEN_PX, RETICLE_SCREEN_PX) for n in names} if RETICLE_SCREEN_PX else {}
    try:
        scan = {t["name"]: t for t in load_scan(state)["textures"]}
    except ValueError:
        return {}
    return {n: tuple(scan[n]["screen"]) for n in names if n in scan and "screen" in scan[n]}


TEXTURE_EXTS = ("tga", "tpc", "dds")


def resolve_texture(
    name: str,
    override_dir: Path,
    erfs: ErfIndex,
    originals_dir: Path,
    sources: dict,
    manifest: dict | None = None,
) -> dict | None:
    """Find a texture's pristine source. Returns a dict or None if absent.

    Precedence: recorded original (originals_tex/, or the ERF for erf sources),
    then override .tpc, then override .tga, then the ERFs. With no recorded
    source, a live file that is still this tool's deploy (manifest sha) is
    refused: pillarboxing it again would squeeze the picture twice.
    """
    key = name.lower()
    rec = sources.get(key)
    if rec:
        if rec["kind"] == "erf":
            hit = erfs.get(key)
            if hit is None:
                raise FileNotFoundError(f"{key}: recorded ERF source vanished")
            return {"kind": "erf", "ext": hit[1], "data": hit[2], "file": rec["file"], "erf": hit[0]}
        orig = originals_dir / rec["file"]
        return {"kind": rec["kind"], "ext": rec["kind"].split("_")[1], "data": orig.read_bytes(), "file": rec["file"]}
    deployed = (manifest or {}).get(key, {}).get("deployed_sha256")
    for ext in ("tpc", "tga"):
        live = find_ci(override_dir, f"{key}.{ext}")
        if live is not None:
            data = live.read_bytes()
            if deployed and sha256(data) == deployed:
                raise RuntimeError(
                    f"{live.name} is this tool's own deploy but its source record is gone; "
                    "restore sources_tex.json/originals_tex or run --rollback first"
                )
            return {"kind": f"override_{ext}", "ext": ext, "data": data, "file": live.name}
    if find_ci(override_dir, f"{key}.dds") is not None:
        raise ValueError(f"{key}: override .dds is not supported")
    hit = erfs.get(key)
    if hit is not None:
        return {"kind": "erf", "ext": hit[1], "data": hit[2], "file": f"{key}.tga", "erf": hit[0]}
    return None


def texture_size(src: dict) -> tuple[int, int]:
    """(width, height) from the file header, without decoding (DXT decoding is slow).
    TPC: uint16 width and height at bytes 8 and 10; TGA: at 12 and 14."""
    return struct.unpack_from("<HH", src["data"], 8 if src["ext"] == "tpc" else 12)


def decode(src: dict) -> tuple[Image.Image, str]:
    if src["ext"] == "tpc":
        return tpc_to_image(src["data"])
    return Image.open(io.BytesIO(src["data"])).convert("RGBA"), ""


class TextureState:
    def __init__(self, state_dir: Path, override_dir: Path):
        self.state_dir = state_dir
        self.override_dir = override_dir
        self.originals = state_dir / "originals_tex"
        self.out = state_dir / "out_tex"
        self.sources_path = state_dir / "sources_tex.json"
        self.manifest_path = state_dir / "manifest_tex.json"
        self.originals.mkdir(parents=True, exist_ok=True)
        self.out.mkdir(parents=True, exist_ok=True)
        self.sources = self._load(self.sources_path)

    @staticmethod
    def _load(path: Path) -> dict:
        return json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}

    def save_sources(self):
        self.sources_path.write_text(json.dumps(self.sources, indent=2, sort_keys=True), encoding="utf-8")

    def manifest(self) -> dict:
        return self._load(self.manifest_path)

    def save_manifest(self, manifest: dict):
        self.manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True), encoding="utf-8")

    def remember(self, name: str, src: dict):
        """Record a first-seen source; copy override originals into originals_tex/."""
        key = name.lower()
        if key in self.sources:
            return
        if src["kind"] != "erf":
            (self.originals / src["file"]).write_bytes(src["data"])
        self.sources[key] = {"kind": src["kind"], "file": src["file"], "sha256": sha256(src["data"])}
        self.save_sources()


def tga_from_tpc(img: Image.Image) -> bytes:
    """A TGA of an image decoded from a TPC, flipped to the engine's display orientation
    (see plan_output). Found 2026-09-26: the map tab's "N" showed as a mirrored N."""
    return image_to_tga(ImageOps.flip(img))


def plan_output(name: str, src: dict, img: Image.Image, txi: str = "") -> dict:
    """Decide the output file for one texture. Returns {file, data, remove, note, txi}.

    `txi` is the source's TXI text (from decode). An output TPC embeds it; an output
    TGA gets it as a sidecar: plan["txi"] = (file name, bytes), else None.

    A TPC stores its rows bottom-up (the engine draws stored row 0 at the bottom, like a
    TGA's), but decode (pykotor) returns them top row first, so a TPC-sourced image is
    upside down: a TGA written from one is flipped back (tga_from_tpc). A TPC output
    keeps decode's row order, so its round trip is unchanged.
    """
    has_txi = bool(txi.strip())
    tga = tga_from_tpc(img) if src.get("ext") == "tpc" else image_to_tga(img)
    if src["kind"] == "override_tpc":
        data = image_to_tpc(img, txi)
        if tpc_roundtrip_ok(data, img):
            return {"file": src["file"], "data": data, "remove": None, "note": "tpc", "txi": None}
        stem = src["file"].rsplit(".", 1)[0]
        plan = {"file": f"{stem}.tga", "data": tga, "remove": src["file"],
                "note": "tpc round trip failed; tga written, tpc moved to originals_tex"}
    elif src["kind"] == "override_tga":
        plan = {"file": src["file"], "data": tga, "remove": None, "note": "tga"}
    else:
        plan = {"file": f"{name.lower()}.tga", "data": tga, "remove": None,
                "note": f"new tga (from {src['erf']})"}
    stem = plan["file"].rsplit(".", 1)[0]
    plan["txi"] = (f"{stem}.txi", txi_bytes(txi)) if has_txi else None
    if has_txi:
        plan["note"] += " + txi"
    return plan


def write_out(state: "TextureState", plan: dict) -> None:
    """Write a planned output (and its TXI sidecar) to out_tex/."""
    (state.out / plan["file"]).write_bytes(plan["data"])
    if plan.get("txi"):
        (state.out / plan["txi"][0]).write_bytes(plan["txi"][1])


def deploy_one(state: TextureState, name: str, src: dict, plan: dict, extra: dict | None = None) -> None:
    """Install one planned output into override/, refusing foreign files. `extra` is
    merged into the manifest entry (e.g. upscale_model)."""
    key = name.lower()
    manifest = state.manifest()
    prev = manifest.get(key, {})
    out_sha = sha256(plan["data"])
    orig_sha = state.sources[key]["sha256"]
    ovr = state.override_dir

    existing = find_ci(ovr, plan["file"])
    target = existing if existing is not None else ovr / plan["file"]
    allowed = {out_sha, prev.get("deployed_sha256")}
    if src["kind"] != "erf" and plan["remove"] is None:
        allowed.add(orig_sha)
    # every check runs before anything in override/ is touched
    if target.exists():
        live = sha256(target.read_bytes())
        if live not in allowed:
            raise RuntimeError(f"Refusing to overwrite foreign {target.name} (SHA: {live[:12]})")
    rm = find_ci(ovr, plan["remove"]) if plan["remove"] else None
    if rm is not None and sha256(rm.read_bytes()) != orig_sha:
        raise RuntimeError(f"Refusing to remove foreign {rm.name}")
    # never leave two texture files for one name (.tga/.tpc/.dds): the engine picks one
    stem, ext = target.name.rsplit(".", 1)
    for other in TEXTURE_EXTS:
        if other == ext.lower():
            continue
        clash = find_ci(ovr, f"{stem}.{other}")
        if clash is not None and (rm is None or clash != rm):
            raise RuntimeError(f"Deploying {target.name} would create a texture pair with {clash.name}")
    # the TXI sidecar: only ever over our own previous sidecar (a mod's .txi is foreign)
    txi_target, txi_sha = None, None
    if plan.get("txi"):
        txi_name, txi_data = plan["txi"]
        txi_sha = sha256(txi_data)
        existing_txi = find_ci(ovr, txi_name)
        txi_target = existing_txi if existing_txi is not None else ovr / txi_name
        if txi_target.exists():
            live = sha256(txi_target.read_bytes())
            if live not in {txi_sha, prev.get("txi_sha256")}:
                raise RuntimeError(f"Refusing to overwrite foreign {txi_target.name} (SHA: {live[:12]})")

    if rm is not None:
        rm.unlink()
    target.write_bytes(plan["data"])
    if txi_target is not None:
        txi_target.write_bytes(plan["txi"][1])
    manifest[key] = {
        "source": "erf" if src["kind"] == "erf" else "override",
        "source_kind": src["kind"],
        "original_file": None if src["kind"] == "erf" else src["file"],
        "removed_file": plan["remove"],
        "original_sha256": orig_sha,
        "deployed_sha256": out_sha,
        "target_path": str(target),
        "txi_file": txi_target.name if txi_target is not None else None,
        "txi_sha256": txi_sha,
        "timestamp": datetime.datetime.now().isoformat(),
        **(extra or {}),
    }
    state.save_manifest(manifest)


def rollback_txi(entry: dict, ovr: Path, log: list[str], key: str) -> None:
    """Delete our TXI sidecar, only while it is still exactly what we deployed."""
    if not entry.get("txi_file"):
        return
    txi = find_ci(ovr, entry["txi_file"])
    if txi is None:
        log.append(f"  WARN {key}: {entry['txi_file']} is gone")
    elif sha256(txi.read_bytes()) != entry["txi_sha256"]:
        log.append(f"  WARN {key}: {txi.name} changed since deploy; left alone")
    else:
        txi.unlink()
        log.append(f"  deleted {txi.name}")


def rollback(state: TextureState, select: Callable[[dict], bool] | None = None) -> list[str]:
    """Undo the deploys in the state's manifest; with `select`, only the entries it
    accepts (the others stay in the manifest untouched)."""
    manifest = state.manifest()
    log = []
    for key in sorted(manifest):
        entry = manifest[key]
        if select is not None and not select(entry):
            continue
        target = Path(entry["target_path"])
        if not target.exists():
            log.append(f"  WARN {key}: {target.name} is gone; dropping manifest entry")
            rollback_txi(entry, target.parent, log, key)
            del manifest[key]
            continue
        live = sha256(target.read_bytes())
        if live != entry["deployed_sha256"]:
            log.append(f"  WARN {key}: {target.name} changed since deploy (sha {live[:12]}); left alone")
            continue
        if entry["source"] == "override":
            orig = state.originals / entry["original_file"]
            if not orig.exists():
                log.append(f"  WARN {key}: {orig} missing; left alone")
                continue
            if entry.get("removed_file"):
                back = find_ci(target.parent, entry["removed_file"])
                if back is not None:
                    log.append(f"  WARN {key}: {back.name} exists again; left {target.name} alone")
                    continue
                target.unlink()
                (target.parent / entry["removed_file"]).write_bytes(orig.read_bytes())
                log.append(f"  restored {entry['removed_file']} (removed {target.name})")
            else:
                target.write_bytes(orig.read_bytes())
                log.append(f"  restored {target.name} from originals_tex/")
        else:
            target.unlink()
            log.append(f"  deleted {target.name} (ERF copy applies again)")
        rollback_txi(entry, target.parent, log, key)
        del manifest[key]
    state.save_manifest(manifest)
    return log


# ---------------------------------------------------------------- subcommands

def loadscreen_names(override_dir: Path, erfs: ErfIndex) -> tuple[list[str], list[str]]:
    """Unique bmpresref values of override/loadscreens.2da + load_default + other override load_*.

    Returns (names, skipped) where skipped holds malformed references.
    """
    names = set()
    skipped = []
    twoda = find_ci(override_dir, "loadscreens.2da")
    if twoda is not None:
        table = read_2da(twoda.read_bytes())
        for i in range(table.get_height()):
            ref = (table.get_cell(i, "bmpresref") or "").strip()
            if not ref or ref == "****":
                continue
            if "." in ref or len(ref) > 16:
                skipped.append(f"{ref} (row {i}: malformed resref)")
                continue
            names.add(ref.lower())
    names.add("load_default")
    for p in override_dir.iterdir():
        if p.name.lower().startswith("load_") and p.suffix.lower() in (".tga", ".tpc", ".dds"):
            names.add(p.stem.lower())
    return sorted(names), skipped


def cmd_loadscreens(args, state: TextureState, erfs: ErfIndex, m: float) -> int:
    names, skipped = loadscreen_names(state.override_dir, erfs)
    model = getattr(args, "upscaled", None)
    target_width = getattr(args, "target_width", None) or LOADSCREEN_TARGET_WIDTH
    log = [f"loadscreens: {len(names)} names, M = {m:.4f}"
           + (f", upscaled with {model} where the source is narrower than {target_width}" if model else "")]
    log += [f"  SKIP {s}" for s in skipped]
    plans = []
    samples = []
    refused = 0
    manifest = state.manifest()
    for name in names:
        try:
            src = resolve_texture(name, state.override_dir, erfs, state.originals, state.sources, manifest)
        except (ValueError, FileNotFoundError, RuntimeError) as exc:
            log.append(f"  SKIP {name}: {exc}")
            continue
        if src is None:
            log.append(f"  SKIP {name}: not in override or TexturePacks")
            continue
        state.remember(name, src)
        img, txi = decode(src)
        base, how = img, ""
        if model and img.width < target_width:
            try:
                base = load_upscaled(state, name, model, src)
            except RuntimeError as exc:
                refused += 1
                log.append(f"  REFUSED {name}: {exc}")
                continue
            if base.width != target_width or base.height * img.width != img.height * base.width:
                refused += 1
                log.append(f"  REFUSED {name}: recorded upscale is {base.width}x{base.height}, "
                           f"not a {target_width}-wide scale of {img.width}x{img.height}")
                continue
            raw, base = base, tone_fixed(img, base)
            how = (f" via {model} {base.width}x{base.height}, tone {luma_shift(img, raw):+.1f} -> "
                   f"{luma_shift(img, base):+.1f}")
        out_img = pillarbox(base, m)
        plan = plan_output(name, src, out_img, txi)
        write_out(state, plan)
        log.append(f"  {name:<18} {src['kind']:<13} {img.width}x{img.height}{how} -> {plan['file']} ({plan['note']})")
        plans.append((name, src, plan, {"upscale_model": model if how else None}))
        if how:
            samples.append(loadscreen_sample(name, src["kind"], img, base, out_img, (args.width, args.height), samples))
    log.append(f"loadscreens: {len(plans)} textures generated, {len(names) - len(plans) - refused} skipped, "
               f"{refused} refused")
    if samples:
        sheet = state.out / "loadscreens_upscale_sheet.png"
        loadscreens_sheet(samples).save(sheet)
        log.append(f"  sheet: {sheet} ({len(samples)} upscaled)")
    failed = 0
    if args.deploy:
        for name, src, plan, extra in plans:
            try:
                deploy_one(state, name, src, plan, extra)
            except RuntimeError as exc:
                failed += 1
                log.append(f"  REFUSED {name}: {exc}")
        log.append(f"deployed {len(plans) - failed} textures, refused {failed}")
    (state.state_dir / "report_tex_loadscreens.txt").write_text("\n".join(log) + "\n", encoding="utf-8")
    print("\n".join(log))
    return 1 if failed or refused else 0


def cmd_unsqueeze(args, state: TextureState, erfs: ErfIndex, m: float) -> int:
    """Widen known pre-squeezed mod textures (PRESQUEEZED) back to their true shape."""
    log = [f"unsqueeze: {len(PRESQUEEZED)} known pre-squeezed textures"]
    plans = []
    manifest = state.manifest()
    for sha, spec in sorted(PRESQUEEZED.items(), key=lambda kv: kv[1]["name"]):
        name = spec["name"]
        try:
            src = resolve_texture(name, state.override_dir, erfs, state.originals, state.sources, manifest)
        except (ValueError, FileNotFoundError, RuntimeError) as exc:
            log.append(f"  SKIP {name}: {exc}")
            continue
        if src is None or sha256(src["data"]) != sha:
            got = "absent" if src is None else f"{src['kind']} sha {sha256(src['data'])[:12]}"
            log.append(f"  SKIP {name}: source is {got}, not the known squeezed file {sha[:12]}")
            continue
        state.remember(name, src)
        img, _ = decode(src)
        try:
            out_img = unsqueeze(img, spec["factor"])
        except ValueError as exc:
            log.append(f"  SKIP {name}: {exc}")
            continue
        plan = plan_output(name, src, out_img)
        (state.out / plan["file"]).write_bytes(plan["data"])
        log.append(f"  {name:<18} {src['kind']:<13} x{spec['factor']:.4f} -> {plan['file']} ({spec['origin']})")
        plans.append((name, src, plan))
    failed = 0
    if args.deploy:
        for name, src, plan in plans:
            try:
                deploy_one(state, name, src, plan)
            except RuntimeError as exc:
                failed += 1
                log.append(f"  REFUSED {name}: {exc}")
        log.append(f"deployed {len(plans) - failed} textures, refused {failed}")
    print("\n".join(log))
    return 1 if failed else 0


def upscaled_dir(state: TextureState) -> Path:
    return state.state_dir / "upscaled"


def load_upscaled_meta(state: TextureState) -> dict:
    path = upscaled_dir(state) / "upscaled.json"
    return json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}


def load_upscaled(state: TextureState, name: str, model: str, src: dict) -> Image.Image:
    """The recorded upscale of this exact source; refuses a missing, stale or edited file."""
    meta = load_upscaled_meta(state).get(name, {}).get(model)
    if meta is None:
        raise RuntimeError(f"{name}: no {model} upscale recorded; run `upscale {name}` first")
    if meta["source_sha256"] != sha256(src["data"]):
        raise RuntimeError(f"{name}: the {model} upscale was made from another source; run `upscale {name}` again")
    data = (upscaled_dir(state) / f"{name}.{model}.png").read_bytes()
    if sha256(data) != meta["output_sha256"]:
        raise RuntimeError(f"{name}.{model}.png changed since `upscale` wrote it")
    return Image.open(io.BytesIO(data)).convert("RGBA")


def upscale_done(rec: dict | None, src: dict, png: Path, tool_sha: str, size: tuple[int, int],
                 alpha: str | None = None) -> bool:
    """`upscale --skip-done`: True when the record (one model's entry of upscaled.json)
    is what this run would make: the same source, the same realesrgan binary, the same
    output size (and alpha mode, for an --alpha run), and a PNG that is still the one
    recorded."""
    if rec is None:
        return False
    if rec["source_sha256"] != sha256(src["data"]):
        return False
    if rec["tool_sha256"] != tool_sha:
        return False
    if tuple(rec["size"]) != tuple(size):
        return False
    if alpha and rec.get("alpha_mode") != alpha:
        return False
    return png.is_file() and sha256(png.read_bytes()) == rec["output_sha256"]


UPSCALE_LOCK = ".lock"             # in upscaled/: one upscale process at a time (they all rewrite upscaled.json)


def corner_box(img: Image.Image) -> tuple[int, int, int, int]:
    return (0, 0, min(256, img.width), min(256, img.height))


def cmd_upscale(args, state: TextureState, erfs: ErfIndex) -> int:
    """Upscale each frame texture's pristine source with every model; write PNGs,
    upscaled.json (source, output and tool sha256) and a candidates sheet."""
    binary = Path(args.realesrgan)
    if not binary.is_file():
        print(f"ERROR: {binary} not found (see kotor2_ultrawide.md, 'Upscaling')", file=sys.stderr)
        return 1
    loadscreens = getattr(args, "loadscreens", False)
    set_name = "cards" if getattr(args, "cards", False) else getattr(args, "set", None)
    alpha = getattr(args, "alpha", None)
    target_width = getattr(args, "target_width", None)
    if loadscreens and target_width is None:
        target_width = LOADSCREEN_TARGET_WIDTH
    world = getattr(args, "world", None)
    if loadscreens:
        names = loadscreen_names(state.override_dir, erfs)[0]
    elif world:
        names = [r["name"] for r in world_textures(erfs, state.override_dir, world_prefix(world))[0]]
    elif set_name:
        try:
            names, notes = set_members(set_name, state, erfs, getattr(args, "height", 1600))
        except ValueError as exc:
            print(f"ERROR: {exc}", file=sys.stderr)
            return 1
        for note in notes:
            print(f"  NOTE {note}")
    else:
        names = [n.lower() for n in (args.names or FRAME_SPECS)]
    lenient = loadscreens or bool(set_name) or bool(world)   # a listed texture that exists nowhere is skipped
    models = [m.strip() for m in args.models.split(",") if m.strip()]
    bad = [m for m in models if m not in UPSCALE_MODELS]
    if bad:
        print(f"ERROR: unknown model(s) {bad}; choose from {sorted(UPSCALE_MODELS)}", file=sys.stderr)
        return 1
    out = upscaled_dir(state)
    out.mkdir(parents=True, exist_ok=True)
    # opened without O_TRUNC: the holder's PID stays readable until the lock is ours
    lock = out / UPSCALE_LOCK
    fd = os.open(lock, os.O_CREAT | os.O_RDWR)
    try:
        try:
            fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            pid = lock.read_text(encoding="ascii", errors="replace").strip() or "?"
            print(f"ERROR: another upscale process (PID {pid}) is running (upscaled/{UPSCALE_LOCK})", file=sys.stderr)
            return 1
        os.ftruncate(fd, 0)
        os.write(fd, str(os.getpid()).encode())
        return upscale_batch(args, state, erfs, names, models, binary, lenient, loadscreens, world, alpha, target_width)
    finally:
        fcntl.flock(fd, fcntl.LOCK_UN)
        os.close(fd)


def upscale_batch(args, state: TextureState, erfs: ErfIndex, names: list[str], models: list[str], binary: Path,
                  lenient: bool, loadscreens: bool, world: str | None, alpha: str | None,
                  target_width: int | None) -> int:
    """cmd_upscale's work, run while it holds the lock. With --skip-done a texture whose
    every model's upscale_done passes is skipped, and only the missing or stale models
    run for the others."""
    out = upscaled_dir(state)
    meta = load_upscaled_meta(state)
    tool_sha = sha256(binary.read_bytes())
    skip_done = getattr(args, "skip_done", False)
    failed = done = made = 0
    with tempfile.TemporaryDirectory() as tmp:
        for name in names:
            try:
                src = resolve_texture(name, state.override_dir, erfs, state.originals, state.sources, state.manifest())
            except (ValueError, FileNotFoundError, RuntimeError) as exc:
                print(f"  SKIP {name}: {exc}")
                failed += 0 if lenient else 1
                continue
            if src is None:
                print(f"  SKIP {name}: not found")
                failed += 0 if lenient else 1   # loadscreens.2da names or set members that exist nowhere
                continue
            state.remember(name, src)
            img, _ = decode(src)
            if loadscreens and img.width >= target_width:
                print(f"  = {name:<18} {img.width}x{img.height} already >= target {target_width}")
                continue
            tw = img.width * world_factor(*img.size) if world else target_width
            todo = models
            if skip_done:
                try:
                    factor = upscale_factor(img.width, tw)
                    size = (img.width * factor, img.height * factor)
                except ValueError:
                    size = None    # the upscale below reports it
                ok = {m: size is not None and upscale_done(meta.get(name, {}).get(m), src, out / f"{name}.{m}.png",
                                                           tool_sha, size, alpha) for m in models}
                if all(ok[m] for m in models):
                    print(f"  = {name} done")
                    done += 1
                    continue
                todo = [m for m in models if not ok[m]]
            cands = {}
            for model in todo:
                info = {}
                try:
                    if alpha and img.getchannel("A").getextrema() == (255, 255):
                        # an opaque member of an --alpha batch: the plain path, same result minus the bleed
                        up = upscale_image(img, name, model, binary, Path(tmp), args.gpu, tw)
                        info = {"alpha_mode": alpha, "alpha_mode_used": "opaque", "alpha_divergence": None}
                    elif alpha:
                        up, info = upscale_alpha_image(img, name, model, binary, Path(tmp), args.gpu, tw, alpha)
                    else:
                        up = upscale_image(img, name, model, binary, Path(tmp), args.gpu, tw)
                except (ValueError, RuntimeError) as exc:
                    print(f"  SKIP {name} {model}: {exc}")
                    failed += 1
                    continue
                if info.get("note"):
                    print(f"  NOTE {name}: {info['note']}")
                png = out / f"{name}.{model}.png"
                up.save(png)
                meta.setdefault(name, {})[model] = {
                    "source_sha256": sha256(src["data"]),
                    "source_kind": src["kind"],
                    "output_sha256": sha256(png.read_bytes()),
                    "tool_sha256": tool_sha,
                    "native_scale": UPSCALE_MODELS[model],
                    "factor": up.width // img.width,
                    "size": list(up.size),
                    "timestamp": datetime.datetime.now().isoformat(),
                    **{k: v for k, v in info.items() if k != "note"},
                }
                cands[model] = up
                used = f" alpha {info['alpha_mode_used']}" if info else ""
                print(f"  {name:<18} {model:<22} {img.width}x{img.height} -> {up.width}x{up.height} {png.name}{used}")
            made += bool(cands)
            if cands and not lenient:
                sheet = out / f"{name}_candidates.png"
                candidates_sheet(img, cands, corner_box(img)).save(sheet)
                print(f"  candidates sheet: {sheet}")
            # save as we go: a long batch that fails late keeps what it made
            (out / "upscaled.json").write_text(json.dumps(meta, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (out / "upscaled.json").write_text(json.dumps(meta, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"upscaled {made} textures, skipped {done} already done, failed {failed}")
    return 1 if failed else 0


def set_sheet_paths(state: TextureState, set_name: str, pages: int) -> list[Path]:
    stem = "pcards" if set_name == "cards" else set_name
    return [state.out / (f"{stem}_sheet.png" if i == 0 else f"{stem}_sheet_{i + 1}.png") for i in range(pages)]


def cmd_set(args, state: TextureState, erfs: ErfIndex, set_name: str | None = None) -> int:
    """A named upscale set (SET_NAMES) from its recorded upscales: outputs (TGA + TXI
    sidecar, or TPC with the TXI embedded), paginated sheets, and on --deploy all of
    it, or nothing when any member's upscale is refused."""
    set_name = set_name or args.set
    model = args.upscaled
    try:
        names, notes = set_members(set_name, state, erfs, args.height)
    except ValueError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    log = [f"set {set_name}: {len(names)} textures, upscaled with {model}"] + [f"  NOTE {n}" for n in notes]
    plans, items = [], []
    refused = 0
    manifest = state.manifest()
    for name in names:
        try:
            src = resolve_texture(name, state.override_dir, erfs, state.originals, state.sources, manifest)
        except (ValueError, FileNotFoundError, RuntimeError) as exc:
            log.append(f"  SKIP {name}: {exc}")
            continue
        if src is None:
            log.append(f"  SKIP {name}: no longer in override or TexturePacks")
            continue
        state.remember(name, src)
        img, txi = decode(src)
        try:
            raw = load_upscaled(state, name, model, src)
        except RuntimeError as exc:
            refused += 1
            log.append(f"  REFUSED {name}: {exc}")
            continue
        up = tone_fixed(img, raw)
        info = {**load_upscaled_meta(state)[name][model], "treatment": "tone"}
        plan = plan_output(name, src, up, txi)
        write_out(state, plan)
        log.append(f"  {name:<18} {src['kind']:<13} {img.width}x{img.height} -> {plan['file']} {up.width}x{up.height} "
                   f"({plan['note']}, alpha {info.get('alpha_mode_used', 'none')}, "
                   f"tone {luma_shift(img, raw):+.1f} -> {luma_shift(img, up):+.1f})")
        plans.append((name, src, plan))
        items.append((name, img, up, info, txi))
    if items:
        sizes = set_screen_sizes(set_name, [i[0] for i in items], state, args.width, args.height)
        pages = [items[i:i + SHEET_PER_PAGE] for i in range(0, len(items), SHEET_PER_PAGE)]
        for path, page in zip(set_sheet_paths(state, set_name, len(pages)), pages):
            set_sheet(page, sizes).save(path)
            log.append(f"  sheet: {path} ({len(page)} textures)")
    failed = 0
    if args.deploy and not refused:
        for name, src, plan in plans:
            try:
                deploy_one(state, name, src, plan, {"upscale_model": model, "set": set_name})
            except RuntimeError as exc:
                failed += 1
                log.append(f"  REFUSED {name}: {exc}")
        log.append(f"deployed {len(plans) - failed} textures, refused {failed}")
    elif args.deploy:
        log.append("nothing deployed: fix the refused upscales first")
    print("\n".join(log))
    return 1 if failed or refused else 0


def cmd_cards(args, state: TextureState, erfs: ErfIndex) -> int:
    """The pazaak card faces: `set cards`."""
    return cmd_set(args, state, erfs, "cards")


def read_only(spec: str) -> set[str]:
    """`world --only`: comma-separated names, or @FILE with one name per line (blank
    lines and # comments ignored)."""
    if spec.startswith("@"):
        names = [line.split("#", 1)[0] for line in Path(spec[1:]).read_text(encoding="utf-8").splitlines()]
    else:
        names = spec.split(",")
    return {n.strip().lower() for n in names if n.strip()}


def world_sheet_paths(ws: TextureState, category: str, kind: str, pages: int) -> list[Path]:
    """world_<category>[_<kind>]_sheet[_<n>].png"""
    stem = f"world_{category}_{kind}_sheet" if kind else f"world_{category}_sheet"
    return [ws.out / (f"{stem}.png" if i == 0 else f"{stem}_{i + 1}.png") for i in range(pages)]


def cmd_world(args, state: TextureState, erfs: ErfIndex) -> int:
    """World textures of one category (WORLD_ALL: every one): the survey; with
    --upscaled, mipmapped outputs in out_world/, the triage (world_<cat>_flags.json) and
    the flagged + sample sheets (--all-sheets: every texture too); with --deploy, those
    (or --only) into override/, recorded in WORLD_MANIFEST (all or nothing), labelled
    with --wave; --rollback undoes that manifest (--wave: only that wave's entries)."""
    ws = world_state(state)
    wave = args.wave
    if args.world_rollback:
        select = (lambda entry: entry.get("wave") == wave) if wave else None
        if select is not None and not any(select(e) for e in ws.manifest().values()):
            print(f"no entries with wave {wave}")
            return 0
        for line in rollback(ws, select):
            print(line)
        return 0
    category = args.category.lower()
    eligible, excluded = world_textures(erfs, state.override_dir, world_prefix(category))
    reasons: dict[str, int] = {}
    for r in excluded:
        key = r["reason"].split(":")[0]
        reasons[key] = reasons.get(key, 0) + 1
    label = category if category == WORLD_ALL else f"{category}*"
    log = [f"world {label}: {len(eligible) + len(excluded)} in {WORLD_ERF}, {len(eligible)} eligible; excluded "
           + ", ".join(f"{k} {n}" for k, n in sorted(reasons.items()))]
    try:
        only = read_only(args.only) if args.only else None
    except OSError as exc:
        print(f"ERROR: --only {args.only}: {exc}", file=sys.stderr)
        return 1
    if only is not None:
        unknown = sorted(only - {r["name"] for r in eligible})
        if unknown:
            print(f"ERROR: --only names that are not eligible: {', '.join(unknown)}", file=sys.stderr)
            return 1
    model = args.upscaled
    if not model:
        for r in eligible:
            lay = r["layout"]
            log.append(f"  {r['name']:<20} {lay['width']}x{lay['height']} {lay['format']:<4} mips {lay['mips']:>2} "
                       f"x{world_factor(lay['width'], lay['height'])}  txi [{'; '.join(r['txi'].splitlines())}]")
        print("\n".join(log))
        return 0
    meta = load_upscaled_meta(ws)
    plans, built, records = [], {}, {}
    refused = 0
    in_bytes = out_bytes = 0
    for r in eligible:
        name = r["name"]
        if only is not None and name not in only:
            continue
        src = resolve_texture(name, state.override_dir, erfs, ws.originals, ws.sources, ws.manifest())
        ws.remember(name, src)
        img, txi = decode(src)
        try:
            up = load_upscaled(ws, name, model, src)
        except RuntimeError as exc:
            refused += 1
            log.append(f"  REFUSED {name}: {exc}")
            continue
        factor = world_factor(*img.size)
        if up.size != (img.width * factor, img.height * factor):
            refused += 1
            log.append(f"  REFUSED {name}: recorded upscale is {up.width}x{up.height}, not {factor}x {img.width}x{img.height}")
            continue
        out, pinfo = world_postprocess(name, img, up, r["kv"], factor)
        mips = world_mip_count(r["layout"]["mips"], factor, *up.size)
        plan = plan_output_world(name, src, out, txi, mips, r["layout"]["format"])
        write_out(ws, plan)
        in_bytes += len(src["data"])
        out_bytes += len(plan["data"])
        info = meta[name][model]
        level0 = tpc_to_image(plan["data"])[0]   # the encoded level 0, as the engine sees it
        err = dxt_error(level0, out, plan["format"])
        flags = world_flags(info, r["kv"], plan["format"], err, pinfo)
        records[name] = {"flags": flags, "dxt_error": err, "prefix": name[:3], "format": plan["format"],
                         "treatment": pinfo["treatment"], "grain_kept": pinfo["grain_kept"],
                         "tone_shift": pinfo["tone_shift"]}
        log.append(f"  {name:<20} {img.width}x{img.height} {r['layout']['format']} {r['layout']['mips']} mips -> "
                   f"{plan['file']} {up.width}x{up.height} {plan['note']} ({len(plan['data']) // 1024} KiB, "
                   f"alpha {info.get('alpha_mode_used', 'none')}, dxt error {err:g}, treatment {pinfo['treatment']})"
                   + (f" FLAGS {', '.join(flags)}" if flags else ""))
        plans.append((name, src, plan))
        built[name] = (src, txi, {**info, "flags": flags, "treatment": pinfo["treatment"]}, plan["data"])
    log.append(f"  size: {in_bytes / 1048576:.1f} MiB of sources -> {out_bytes / 1048576:.1f} MiB")
    triage = world_triage(records, [r["name"] for r in eligible if only is None or r["name"] in only])
    flags_path = ws.out / f"world_{category}_flags.json"
    flags_path.write_text(json.dumps(triage, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    flagged = flagged_order(records)
    log.append(f"  flags: {len(flagged)} of {len(records)} flagged; by rule "
               + ", ".join(f"{rule} {n}" for rule, n in triage["by_rule"].items() if n) + f" ({flags_path})")
    top = sorted(triage["by_prefix"].items(), key=lambda kv: (-kv[1][0], kv[0]))[:WORLD_TOP_PREFIXES]
    log.append("  prefixes (eligible/flagged): " + ", ".join(f"{p} {e}/{f}" for p, (e, f) in top))
    sheets = [("flagged", flagged), ("sample", sample_unflagged(records))]
    if args.all_sheets:
        sheets.append(("", list(records)))
    for kind, names in sheets:
        pages = [names[i:i + SHEET_PER_PAGE] for i in range(0, len(names), SHEET_PER_PAGE)]
        for path, page in zip(world_sheet_paths(ws, category, kind, len(pages)), pages):
            # rebuilt per page: holding every texture's images at once would take gigabytes for `all`
            items = [(n, decode(built[n][0])[0], tpc_to_image(built[n][3])[0], built[n][2], built[n][1]) for n in page]
            set_sheet(items, {}).save(path)
            log.append(f"  sheet: {path} ({len(page)} {kind or 'all'} textures, 100% of the upscale)")
    failed = 0
    if args.deploy and not refused:
        for name, src, plan in plans:
            try:
                deploy_one(ws, name, src, plan, {"upscale_model": model, "world": category,
                                                 "format": plan["format"], "mips": plan["mips"],
                                                 **({"wave": wave} if wave else {})})
            except RuntimeError as exc:
                failed += 1
                log.append(f"  REFUSED {name}: {exc}")
        log.append(f"deployed {len(plans) - failed} textures, refused {failed}")
    elif args.deploy:
        log.append("nothing deployed: fix the refused upscales first")
    print("\n".join(log))
    return 1 if failed or refused else 0


def cmd_scan(args, state: TextureState, erfs: ErfIndex) -> int:
    """Discover GUI textures worth upscaling; writes SCAN_FILE and prints the table."""
    result = scan_gui_textures(state.override_dir.parent, state.override_dir, args.width, args.height, state, erfs,
                               args.min_mag, args.hud)
    result.update({"width": args.width, "height": args.height, "min_mag": args.min_mag, "hud": args.hud,
                   "timestamp": datetime.datetime.now().isoformat()})
    path = state.state_dir / SCAN_FILE
    path.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"scan: {args.width}x{args.height}, HUD {args.hud}, min magnification {args.min_mag:g}; "
          f"{len(result['textures'])} FILL textures, {sum(t['candidate'] for t in result['textures'])} candidates")
    print("\n".join(scan_table(result)))
    print(f"GUIs not scanned: {len(result['guis'])}")
    print(f"written: {path}")
    return 0


def write_frame_edges(path: Path, texture: str, edges: dict) -> None:
    """Merge one texture's entry into a per-texture frame_edges file (upgrading a flat one)."""
    data = json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}
    if "E_left" in data:
        data = {data.get("texture", "pnl_pause_pc"): data}
    data[texture] = edges
    path.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def frame_outputs(img: Image.Image, spec: dict, m: float, base: Image.Image | None = None) -> tuple[Image.Image, dict]:
    """Remap `base` (img, or an upscale of it) with spec's spans scaled to its width.

    Returns (remapped image, edges dict). E_left is measured in the remapped texel
    space of `base`, so a 2x upscale gives the same E_left as the 1x source.
    """
    base = img if base is None else base
    f = base.width / img.width
    if base.height != round(img.height * f):
        raise ValueError(f"upscale {base.size} is not a uniform scale of {img.size}")
    spans = scale_spans(spec["spans"], f)
    edge_row = min(base.height - 1, round(spec["edge_row"] * f))
    out_img = remap_frame(base, spans, m)
    segs = remap_segments(base.width, spans, m)
    left, right = inner_edge(base, edge_row)
    new_left, new_right = map_x(left, segs), map_x(right, segs)
    edges = {
        "E_left": round(new_left * 800 / base.width),
        "E_right": round(new_right * 800 / base.width),
        "edge_row": spec["edge_row"],
        "inner_edge_tex": [left, right],
        "remapped_inner_edge_tex": [round(new_left, 2), round(new_right, 2)],
        "spans": spec["spans"],
        "texture_size": list(base.size),
        "used_for_bars": spec.get("bars", False),
        "M": round(m, 6),
    }
    return out_img, edges


def cmd_frame(args, state: TextureState, erfs: ErfIndex, m: float) -> int:
    name = (getattr(args, "name", None) or "pnl_pause_pc").lower()
    if name not in FRAME_SPECS:
        print(f"ERROR: {name} has no FRAME_SPECS entry ({', '.join(FRAME_SPECS)})", file=sys.stderr)
        return 1
    spec = FRAME_SPECS[name]
    model = getattr(args, "upscaled", None)
    src = resolve_texture(name, state.override_dir, erfs, state.originals, state.sources, state.manifest())
    if src is None:
        print(f"ERROR: {name} not found", file=sys.stderr)
        return 1
    state.remember(name, src)
    img, _ = decode(src)
    if args.detect:
        print(f"detected spans: {detect_ornament_spans(img, spec['edge_row'])}")
        print(f"FRAME_SPECS:    {spec['spans']}")
    try:
        raw = load_upscaled(state, name, model, src) if model else None
        base = tone_fixed(img, raw) if model else None
        out_img, edges = frame_outputs(img, spec, m, base)
    except (RuntimeError, ValueError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    edges.update({"texture": name, "upscale_model": model})
    plan = plan_output(name, src, out_img)
    (state.out / plan["file"]).write_bytes(plan["data"])
    preview = state.out / f"{name}_preview.png"
    remap_1x = remap_frame(img, spec["spans"], m) if model else out_img
    title = f"{name}: spans {spec['spans']}" + (f", upscaled with {model}" if model else "")
    frame_sheet(img, remap_1x, out_img if model else None, (args.width, args.height),
                corner_box(img), title).save(preview)
    print(f"{name}: {src['kind']} -> {plan['file']} ({plan['note']}, {out_img.width}x{out_img.height})"
          + (f", tone {luma_shift(img, raw):+.1f} -> {luma_shift(img, base):+.1f}" if model else ""))
    print(f"  inner edge at row {edges['edge_row']}: E_left = {edges['E_left']}"
          + ("" if spec.get("bars") else " (information only: no frame bars use it)"))
    print(f"  preview: {preview}")
    if args.deploy:
        deploy_one(state, name, src, plan, {"upscale_model": model})
        print(f"  deployed {plan['file']}")
    # frame_edges.json feeds the GUI generator's bar rule, which then requires this
    # texture to be live: only a deploy records there; a preview run writes beside it.
    edges_path = state.state_dir / ("frame_edges.json" if args.deploy else "frame_edges.preview.json")
    write_frame_edges(edges_path, name, edges)
    print(f"  edges: {edges_path}")
    return 0


def main():
    parser = argparse.ArgumentParser(description="Generate KOTOR II ultrawide loading-screen and frame textures.")
    parser.add_argument("--kotor2-root", default=DEFAULT_KOTOR2_ROOT)
    parser.add_argument("--workspace-dir", default=DEFAULT_WORKSPACE_DIR)
    parser.add_argument("--state-dir", default=None, help="State directory (defaults to $WORKSPACE/ultrawide_gui).")
    parser.add_argument("--width", type=int, default=3840)
    parser.add_argument("--height", type=int, default=1600)
    parser.add_argument("--rollback", action="store_true", help="Undo every deploy recorded in manifest_tex.json.")
    sub = parser.add_subparsers(dest="cmd")
    p_ls = sub.add_parser("loadscreens", help="Pillarbox every loading screen.")
    p_ls.add_argument("--upscaled", default=None, metavar="MODEL",
                      help="Pillarbox the recorded `upscale --loadscreens` output of this model for every source "
                           "narrower than --target-width (refused if missing, stale or edited).")
    p_ls.add_argument("--target-width", type=int, default=LOADSCREEN_TARGET_WIDTH)
    p_ls.add_argument("--deploy", action="store_true")
    p_fr = sub.add_parser("frame", help="Remap a menu frame texture (FRAME_SPECS); writes a preview sheet and, "
                                        "on --deploy, its entry in frame_edges.json.")
    p_fr.add_argument("name", nargs="?", default="pnl_pause_pc", help=f"One of: {', '.join(FRAME_SPECS)}.")
    p_fr.add_argument("--upscaled", default=None, metavar="MODEL",
                      help=f"Remap the recorded `upscale` output of this model ({', '.join(UPSCALE_MODELS)}).")
    p_fr.add_argument("--detect", action="store_true", help="Print auto-detected ornament spans next to FRAME_SPECS.")
    p_fr.add_argument("--deploy", action="store_true")
    p_up = sub.add_parser("upscale", help="Real-ESRGAN upscales of textures (preview only, no deploy).")
    p_up.add_argument("names", nargs="*", help="Textures (default: every FRAME_SPECS entry).")
    which = p_up.add_mutually_exclusive_group()
    which.add_argument("--loadscreens", action="store_true",
                       help=f"Every loading screen narrower than --target-width (default {LOADSCREEN_TARGET_WIDTH}).")
    which.add_argument("--cards", action="store_true", help="The PC pazaak card faces (use with --alpha); "
                                                            "the same as --set cards.")
    which.add_argument("--set", default=None, choices=SET_NAMES, help="A named upscale set (ui needs `scan` first).")
    which.add_argument("--world", default=None, metavar="PREFIX",
                       help=f"Eligible vanilla-only {WORLD_ERF} textures named PREFIX* ({WORLD_ALL}: every one; see "
                            f"`world`), each at 4x (long side <= {WORLD_SMALL_SIDE}) or 2x, capped at {WORLD_MAX_SIDE}.")
    p_up.add_argument("--skip-done", action="store_true",
                      help="Resume: skip a texture whose every model's recorded upscale has this source, this "
                           "realesrgan binary, this output size and an unedited PNG; run only the missing models.")
    p_up.add_argument("--target-width", type=int, default=None,
                      help="Upscale to exactly this width (an integer multiple >= 2 of the source); default 2x.")
    p_up.add_argument("--alpha", nargs="?", const="model", default=None, choices=ALPHA_MODES,
                      help="Handle alpha (default mode: model, with a per-texture LANCZOS fallback); "
                           "without it, textures with alpha are refused.")
    p_up.add_argument("--models", default=",".join(UPSCALE_MODELS), help="Comma-separated models.")
    p_up.add_argument("--realesrgan", default=DEFAULT_REALESRGAN, help="realesrgan-ncnn-vulkan binary.")
    p_up.add_argument("--gpu", type=int, default=0, help="Vulkan device index (default 0; -1 = let the tool pick).")
    p_un = sub.add_parser("unsqueeze", help="Widen known pre-squeezed mod textures (e.g. the mod150 logo).")
    p_un.add_argument("--deploy", action="store_true")
    p_ca = sub.add_parser("cards", help="Pazaak card faces from their recorded upscales, as TGA + TXI; writes "
                                        "out_tex/pcards_sheet.png.")
    p_ca.add_argument("--upscaled", required=True, metavar="MODEL")
    p_ca.add_argument("--deploy", action="store_true")
    p_se = sub.add_parser("set", help="A named upscale set from its recorded upscales, as TGA + TXI (or TPC); writes "
                                      "out_tex/<set>_sheet*.png.")
    p_se.add_argument("set", choices=SET_NAMES)
    p_se.add_argument("--upscaled", required=True, metavar="MODEL")
    p_se.add_argument("--deploy", action="store_true")
    p_wo = sub.add_parser("world", help=f"World textures of one category: survey; --upscaled writes mipmapped TPCs "
                                        f"to $STATE/{WORLD_OUT}/, flags and sheets; --deploy records {WORLD_MANIFEST}.")
    p_wo.add_argument("--category", default="plc", help=f"Texture name prefix, or {WORLD_ALL} (default plc).")
    p_wo.add_argument("--upscaled", default=None, metavar="MODEL")
    p_wo.add_argument("--only", default=None, metavar="A,B,...|@FILE",
                      help="Build and deploy only these (an approved subset); @FILE: one name per line, # comments.")
    p_wo.add_argument("--all-sheets", action="store_true",
                      help="Also write the sheets of every texture (world_<category>_sheet*.png), not only the "
                           "flagged and sample ones.")
    p_wo.add_argument("--wave", default=None, metavar="NAME",
                      help="With --deploy: label the manifest entries; with --rollback: undo only that wave.")
    p_wo.add_argument("--deploy", action="store_true")
    p_wo.add_argument("--rollback", dest="world_rollback", action="store_true", help=f"Undo every deploy recorded in {WORLD_MANIFEST}.")
    p_sc = sub.add_parser("scan", help=f"Discover GUI textures worth upscaling; writes $STATE/{SCAN_FILE}.")
    p_sc.add_argument("--min-mag", type=float, default=SCAN_MIN_MAG, help="Candidate threshold (default 1.5).")
    p_sc.add_argument("--hud", default=HUD_DESC_GUI, help=f"The HUD GUI drawn at this resolution (default {HUD_DESC_GUI}).")
    args = parser.parse_args()
    if args.cmd == "upscale" and args.names and (args.loadscreens or args.cards or args.set or args.world):
        parser.error("give texture names or --loadscreens/--cards/--set/--world, not both")
    if args.width <= 0 or args.height <= 0:
        parser.error("--width and --height must be positive")

    game_dir = Path(args.kotor2_root).resolve()
    workspace_dir = Path(args.workspace_dir).resolve()
    state_dir = Path(args.state_dir).resolve() if args.state_dir else workspace_dir / "ultrawide_gui"
    state = TextureState(state_dir, game_dir / "override")

    if (args.rollback or getattr(args, "deploy", False)
            or getattr(args, "world_rollback", False)) and is_game_running():
        print("ERROR: swkotor2.exe is currently running. Close the game first.", file=sys.stderr)
        return 1
    if args.rollback:
        print("Rolling back deployed textures...")
        for line in rollback(state):
            print(line)
        return 0
    if not args.cmd:
        parser.error("choose a subcommand (loadscreens | frame | upscale | unsqueeze | cards | set | scan | world) or --rollback")
    if args.cmd == "upscale":
        if not args.realesrgan or args.realesrgan == DEFAULT_REALESRGAN:
            args.realesrgan = str(Path(args.workspace_dir) / "tools" / "realesrgan" / "realesrgan-ncnn-vulkan")
        if args.gpu is not None and args.gpu < 0:
            args.gpu = None
        return cmd_upscale(args, state, ErfIndex(game_dir / "TexturePacks"))

    erfs = ErfIndex(game_dir / "TexturePacks")
    m = compute_stretch_factor(args.width, args.height)
    if m < 1.0:
        print("ERROR: screens narrower than 4:3 are squeezed, not stretched; not supported.", file=sys.stderr)
        return 1
    if args.cmd == "frame":
        return cmd_frame(args, state, erfs, m)
    if args.cmd == "unsqueeze":
        return cmd_unsqueeze(args, state, erfs, m)
    if args.cmd == "cards":
        return cmd_set(args, state, erfs, "cards")
    if args.cmd == "set":
        return cmd_set(args, state, erfs)
    if args.cmd == "scan":
        return cmd_scan(args, state, erfs)
    if args.cmd == "world":
        return cmd_world(args, state, erfs)
    return cmd_loadscreens(args, state, erfs, m)


if __name__ == "__main__":
    sys.exit(main())
