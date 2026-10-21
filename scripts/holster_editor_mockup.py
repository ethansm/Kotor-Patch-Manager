#!/usr/bin/env python3
"""Offline PNG renderer of the KOTOR II "Holstered Weapons" editor page (holster-editor mod).

Predicts where BUTTON captions land before a DLL fix is installed. Boxes come from the real layout code: the host tool
patch_manager_mods/holster_editor_src/tools/hse_layout_dump.cpp (HseLayoutV3.h pageLayoutV3 + pageBoxesV3) is compiled on demand
and its JSON read back. Fonts, border textures and the glyph atlas are imported from scripts/character_page_mockup.py.

  .venv/bin/python scripts/holster_editor_mockup.py --panes 3840x1600 --mode engine-model --caption-fix 0 --out /tmp/p.png
  .venv/bin/python scripts/holster_editor_mockup.py --panes 1920x1080 --mode ideal --out /tmp/p.png --crop
  .venv/bin/python scripts/holster_editor_mockup.py --panes 3840x1600 --remove-open --crop --out /tmp/p_remove.png   (the "..." menu open: Remove saved placement shown)

Modes:
  engine-model  buttons get the engine's inner rect (research/holstered_weapons/ux_v3/F3_R1_inset.txt section 1(c)); a line taller than
                that rect is DROPPED (--no-drop keeps it). Labels get their full box, centred.
  ideal         every caption centred in its full box.
--caption-fix 1 (engine-model): caption rect = (engine x, button y, engine w, button h); engine w <= 0 -> (X+4, Y, W-8, H).
Word wrap (both modes): the engine WORD-WRAPS a caption wider than its text rect (buttons: w-14 at x+7; labels: the box) and centres the n
lines vertically, offset (B - n*h)/2 (h = line height); lines past the box edges are drawn, not clipped. The wrap measure is the engine's
glyph advance, 1.98 px per texel at 3840 (ENGINE_PX_PER_TEXEL_3840).
Fit (mirrors the DLL textfit::fitText): the caption is cut with "..." to (box w - pad) at texels * text_k * 0.75 px (TextFit.h TextK;
HolsterEditor.cpp textCtx sy = ini text_k, no resolution scaling). --button-fit-pad 20, --label-fit-pad 8, --text-k 2.65 are the new DLL
defaults; --legacy-fit = pad 8 for all + text_k 2.5 (the 0.4.3 build).
Description boxes: the page has a LEFT green box (PI_HINT, text PI_HINT+1) bottom-aligned on the description pane and a RIGHT one (PI_HINTR, text
PI_HINTR+1) that is the description pane itself (mod-config's shrunk LB_DESC). --desc X,Y,W,H sets that pane (engine px). Default derivation (default_desc):
mod-config splits the original description pane (kDescSplitPermille 850) into the preview (top 85%, the rect the mockup already has) and LB_DESC (bottom
15%); so original height = preview.h / 0.85, LB_DESC = (preview.x, preview.y + preview.h, preview.w, preview.h * 150 / 850) = 132 px tall at the measured
3840 set (about the ~100 px the real screenshot shows at its 2000 px wide crop: 132 * 2000 / 2374). Frames use edge 0,105,81 / fill 0,17,11, texts 37,200,164
top-left aligned and word wrapped in the inset text area (no "..." cut).
Writes <out>.json beside the PNG: per item {i, kind, text, box, caption_rect, drawn, text_y0, lines, wraps, text_rect}.
Use the repo venv (PIL + pykotor); the game install is needed for fonts and border textures.
"""
import argparse
import json
import re
import subprocess
import sys
import tempfile
from functools import lru_cache
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent
sys.path.insert(0, str(HERE))

SRC_DIR = REPO / "patch_manager_mods/holster_editor_src"
DUMP_CPP = SRC_DIR / "tools/hse_layout_dump.cpp"
SHARED = REPO / "patch_manager_mods/_shared_src"
FONT_WIDTHS = SHARED / "FontWidths.inc"

FONT = "dialogfont16x16"
TEAL_FRAME = (0.051, 0.349, 0.271)   # BTN_GAMEPLAY BORDER COLOR
TEAL_TEXT = (0.102, 0.698, 0.549)    # BTN_GAMEPLAY TEXT COLOR
CREAM = (0.8, 0.8, 0.698)            # label text
BTN_BORDER = {"CORNER": "uibit_brdr_16wc", "EDGE": "uibit_brdr_16we", "COLOR": TEAL_FRAME}   # F3_R2_border.txt (textures 32x32)

# ---- layout dump (C++) -----------------------------------------------------------------------------------------------


def build_dump(build_dir=None):
    """Compile hse_layout_dump.cpp (g++ -std=c++17, the holster_editor test.toml default flags) when missing or older than its inputs."""
    bdir = Path(build_dir) if build_dir else Path(tempfile.gettempdir()) / "hse_mockup_build"
    bdir.mkdir(parents=True, exist_ok=True)
    exe = bdir / "hse_layout_dump"
    deps = [DUMP_CPP, SRC_DIR / "HseLayoutV3.h"] + list(SHARED.glob("*.h"))
    if not exe.exists() or exe.stat().st_mtime < max(p.stat().st_mtime for p in deps if p.exists()):
        cmd = ["g++", "-std=c++17", "-O1", "-Wall", "-Wextra", "-I../_shared_src", "-Werror", str(DUMP_CPP.relative_to(SRC_DIR)), "-o", str(exe)]
        r = subprocess.run(cmd, cwd=SRC_DIR, capture_output=True, text=True)
        if r.returncode != 0:
            raise RuntimeError("hse_layout_dump build failed:\n" + r.stderr)
    return exe


def run_dump(col, view, preview, desc=None, build_dir=None, cover=None):
    """cover = ini desc_cover_px (None = the editor default hse::kV3CoverDefault, 6); only meaningful with a desc pane."""
    exe = build_dump(build_dir)
    args = [str(v) for v in (*col, *view, *preview, *(desc or ()))]
    if desc and cover is not None:
        args.append(str(cover))
    r = subprocess.run([str(exe)] + args, capture_output=True, text=True, check=True)
    return json.loads(r.stdout)


PI_REMOVE = 72   # HseLayoutV3.h PI_REMOVE: the Remove saved placement button (a button; shown only while the "..." menu is open)


# ---- pane sets -------------------------------------------------------------------------------------------------------

# The raw pane set widened the way the live editor does (HolsterEditor.cpp applyLayout: column.x -= margin_px, column.w += margin_px, preview.w += margin_px;
# holster_editor.ini margin_px 120): (652, 357, 1252, 757) and (1953, 357, 1233, 746). The dump is called with these, so the offline render matches the live page.
MARGIN_PX = 120
_RAW_PANES_3840x1600 = ((772, 357, 1132, 757), (1345, 357, 576, 896), (1953, 357, 1113, 746))
DEFAULT_PANES_3840x1600 = ((_RAW_PANES_3840x1600[0][0] - MARGIN_PX, _RAW_PANES_3840x1600[0][1], _RAW_PANES_3840x1600[0][2] + MARGIN_PX, _RAW_PANES_3840x1600[0][3]),
                           _RAW_PANES_3840x1600[1],
                           (_RAW_PANES_3840x1600[2][0], _RAW_PANES_3840x1600[2][1], _RAW_PANES_3840x1600[2][2] + MARGIN_PX, _RAW_PANES_3840x1600[2][3]))


def geom_for(W, H):
    """Port of geomFor in hse_layout_v3_test.cpp: real 3440x1440 rects scaled by W/3440 and H/1440 (integer math)."""
    sx = lambda v: v * W // 3440
    sy = lambda v: v * H // 1440
    col = (sx(150), sy(357), sx(400), sy(896))
    view = (sx(1345), sy(357), sx(576), sy(896))
    prev = (sx(1925), sy(357), sx(500), sy(700))
    return col, view, prev


def panes_for(W, H):
    if (W, H) == (3840, 1600):
        return DEFAULT_PANES_3840x1600
    return geom_for(W, H)


DESC_SPLIT_PERMILLE = 850   # HseLayoutV3.h kDescSplitPermille: the preview (inspector) is the top 85% of the description pane


def default_desc(preview):
    """Shrunk LB_DESC from the preview pane: the bottom (1000 - 850) / 850 of the preview's height, directly under it, same x and width."""
    x, y, w, h = preview
    return (x, y + h, w, h * (1000 - DESC_SPLIT_PERMILLE) // DESC_SPLIT_PERMILLE)


def text_scale(W):
    """Engine text scale in px per font texel: 1.0 + 0.5 * ((W - 1022) >> 10) (2.0 at 3840 and 3440, 1.0 at 1920)."""
    return 1.0 + 0.5 * ((W - 1022) >> 10)


# ---- text measuring (port of TextFit.h textPx / fitText) -------------------------------------------------------------


@lru_cache(None)
def texel_widths():
    """{font name: 256 glyph advances in texels} from _shared_src/FontWidths.inc (the character_page_mockup table path is stale)."""
    src = FONT_WIDTHS.read_text()
    out = {}
    for var, name in (("kFontTexW", "dialogfont16x16"), ("kFont10TexW", "dialogfont10x10")):
        body = re.search(var + r"\[256\]\s*=\s*\{(.*?)\};", src, re.S).group(1)
        out[name] = [int(v) for v in re.findall(r"\d+", body)]
        assert len(out[name]) == 256, var
    return out


def install_font_fix():
    """Point character_page_mockup.texel_widths at FontWidths.inc (monkeypatch; the module itself is not edited)."""
    import character_page_mockup as cpm
    cpm.texel_widths = texel_widths


ENGINE_PX_PER_TEXEL_3840 = 1.98   # live probe: glyph advance of the engine text at 3840 (text_scale 2.0)
TEXTFIT_TEXTK = 0.75              # TextFit.h TextK: fit px = texels * ini text_k * TextK
DEFAULT_BUTTON_FIT_PAD, DEFAULT_LABEL_FIT_PAD, DEFAULT_TEXT_K = 20, 8, 2.65
LEGACY_BUTTON_FIT_PAD, LEGACY_LABEL_FIT_PAD, LEGACY_TEXT_K = 8, 8, 2.5


def engine_scale(W):
    """px per font texel the engine really draws (and wraps) with: 1.98 at 3840, scaled with text_scale."""
    return ENGINE_PX_PER_TEXEL_3840 * text_scale(W) / 2.0


def fit_scale(text_k):
    """px per texel of the DLL fit measure (textfit::textPx with sy = text_k, textScale 1)."""
    return text_k * TEXTFIT_TEXTK


def text_px(s, scale):
    tw = texel_widths()[FONT]
    return sum(tw[ord(c) if ord(c) < 256 else 63] for c in s) * scale


def fit_text(s, max_px, scale):
    """textfit::fitText: cut with "..." to max_px (max_px <= 0: no cut, as fitButton)."""
    if max_px <= 0 or text_px(s, scale) <= max_px:
        return s
    n = len(s)
    dots = text_px("...", scale)
    while n > 0 and text_px(s[:n], scale) + dots > max_px:
        n -= 1
    while n > 0 and s[n - 1] == " ":
        n -= 1
    return s[:n] + "..."


def wrap_text(s, max_px, scale):
    """Engine-model word wrap. Greedy on spaces; a single word wider than max_px is broken at character level (the live probe reports
    lines=2 for the one-word "Roll/Turn", so the engine does not keep a long word whole). -> list of lines (>= 1)."""
    lines, cur = [], ""
    for word in s.split():
        cand = (cur + " " + word) if cur else word
        if text_px(cand, scale) <= max_px:
            cur = cand
            continue
        if cur:
            lines.append(cur)
        cur = ""
        for ch in word:
            if cur and text_px(cur + ch, scale) > max_px:
                lines.append(cur)
                cur = ch
            else:
                cur += ch
    lines.append(cur)
    return lines


# ---- captions --------------------------------------------------------------------------------------------------------

ROW_NAMES = ["Slide", "Depth", "Height", "Tilt", "Roll", "Turn"]
ROW_VALUES = ["0.0 cm", "-11.0 cm", "2.5 cm", "30.5 deg", "0.0 deg", "-5.0 deg"]
HELP_LINE = "LMB turn . RMB+LMB move weapon . RMB+wheel depth . Shift fine . Ctrl snap"


# Left box text with nothing hovered (HolsterEditor.cpp leftDefaultText): the "Save changes go to ..." sentence of the current scope (hse::scopeSentence, scope 0
# with a sample model). The editor appends the short help only when the pair still fits the text area by its line estimate; at 3840x1600 (7 lines of 32 px)
# the sentence alone is 4 lines and the pair 8, so the help is not appended. With the "..." menu open the box shows the Remove sentence (hse::removeSentence).
LEFT_HELP = "Pick the weapon, the bone and where Save applies, then adjust the sliders. Hover a button for help."
LEFT_SENTENCE = "Save changes go to this weapon model only (w_vbrsw_01), on every character."
LEFT_DESC_TEXT = LEFT_SENTENCE
REMOVE_DESC_TEXT = "Removes the saved placement of w_vbrsw_01. It falls back to the melee weapons default."
RIGHT_DESC_TEXT = "Drag or click a slider. Shift fine, Ctrl snap."
DESC_EDGE = (0 / 255, 105 / 255, 81 / 255)   # description box frame colour 0,105,81
DESC_FILL = (0, 17, 11, 255)                 # description box fill
DESC_TEXT = (37 / 255, 200 / 255, 164 / 255)  # description text 37,200,164
LIT_TEXT = (1.0, 0.86, 0.35)                  # HolsterEditor.cpp kColLit (member name text)
PI_MEMBER_NAME = 1


def caption_for(i, remove_open=False):
    """Plausible caption of page item i (strings from HolsterEditor.cpp itemInitText / HseView.h; sample values for composed ones)."""
    # Items 0 / 2 (member buttons) carry the words "Prev" / "Next" (the arrow art was invisible inside the 16 px button border inset).
    fixed = {0: "Prev", 1: "Meetra Surik", 2: "Next", 3: "WEAPON", 4: "BONE", 5: "APPLIES TO", 6: "Right: Vibrosword", 7: "Left: Vibrosword",
             13: "This weapon model", 14: "All melee weapons", 15: "This body + armor", 16: "Save", 17: "Revert",
             18: "Member 1/9 | Weapons: Right+Left", 19: ">Move<", 20: "Depth", 21: "Tilt", 22: "Roll/Turn", 23: "R", 24: "<>", 25: "",
             31: HELP_LINE, 32: "POSITION", 33: "R", 34: "ROTATION", 35: "R", 66: "Left: mirrored from right",
             67: "", 68: REMOVE_DESC_TEXT if remove_open else LEFT_DESC_TEXT, 69: "...", 70: "", 71: RIGHT_DESC_TEXT,
             PI_REMOVE: "Remove saved placement"}
    if i in fixed:
        return fixed[i]
    if 8 <= i <= 12:
        return ["Upper back", "Back", "Pelvis", "R hip", "L hip"][i - 8]
    if 26 <= i <= 30:
        return ["Idle", "Walk", "Run", "Alert", "Combat"][i - 26]
    if 36 <= i <= 65:
        r, k = divmod(i - 36, 5)
        return [ROW_NAMES[r], "", "", ROW_VALUES[r], "R"][k]
    return ""


# ---- caption placement (engine model) --------------------------------------------------------------------------------


def _axis_inner(pos, size, g):
    """One axis of the engine's inner rect: (start, length) before the margin is applied."""
    inner = size - 2 * g
    if inner < 1 or (inner // g == 0 and inner < g / 2):
        return pos + size // 2, 0
    return pos + g, inner


def inner_rect(box, g=16, m=9):
    """Engine inner rect of a border / hilight of dimension g and inner offset m laid on `box` (F3_R1_inset.txt section 1(c))."""
    X, Y, W, H = box
    xi, wi = _axis_inner(X, W, g)
    yi, hi = _axis_inner(Y, H, g)
    mm = min(m, g)
    return (xi - mm, yi - m, wi + 2 * mm, hi + 2 * m)


def intersect(a, b):
    x0, y0 = max(a[0], b[0]), max(a[1], b[1])
    x1, y1 = min(a[0] + a[2], b[0] + b[2]), min(a[1] + a[3], b[1] + b[3])
    return (x0, y0, max(0, x1 - x0), max(0, y1 - y0))


def place_caption(kind, box, mode="engine-model", fix=0, g=16, m=9, line_h=32, no_drop=False, lines=1):
    """-> (caption_rect, drawn, text_y0 of the first line). `drawn` is the geometry verdict only (the caller also skips empty text).
    n lines are centred as a block: y0 = rect y + (rect h - n * line_h) // 2 (may start above the box)."""
    if mode == "ideal" or kind != "button":
        rect = tuple(box)
    else:
        rect = intersect(inner_rect(box, g, m), inner_rect(box, g, m))   # border and hilight both get the button rect
        if fix:
            X, Y, W, H = box
            rect = (X + 4, Y, W - 8, H) if rect[2] <= 0 else (rect[0], Y, rect[2], H)
    y0 = rect[1] + (rect[3] - lines * line_h) // 2
    drawn = True
    if mode != "ideal" and kind == "button" and rect[3] < line_h and not no_drop:
        drawn = False
    return rect, drawn, y0


# ---- drawing ---------------------------------------------------------------------------------------------------------


def blit(canvas, img, x, y):
    """alpha_composite with clipping to the canvas (negative or overflowing offsets are allowed)."""
    from PIL import Image
    W, H = canvas.size
    x0, y0 = max(0, x), max(0, y)
    x1, y1 = min(W, x + img.width), min(H, y + img.height)
    if x1 <= x0 or y1 <= y0:
        return
    canvas.alpha_composite(img.crop((x0 - x, y0 - y, x1 - x, y1 - y)), (x0, y0))


def wrap_width(kind, box, rect, mode):
    """Engine text width: the caption rect's width (buttons: w-14 at x+7 = the inner rect; labels: the box). Ideal buttons use w-14 too."""
    if kind == "button" and mode == "ideal":
        return box[2] - 14
    return rect[2]


def draw_line(canvas, s, rect, y0, scale, col, left=False):
    """Glyphs of s centred horizontally in rect (left=True: starting at rect x), glyph cell top at y0 (cell height = 16 texels * scale)."""
    import character_page_mockup as cpm
    from PIL import Image
    tex, boxes = cpm.font(FONT)
    tinted = cpm.tint(tex, col)
    adv = texel_widths()[FONT]
    x = rect[0] if left else rect[0] + (rect[2] - text_px(s, scale)) / 2.0
    for c in s:
        idx = ord(c) if ord(c) < 256 else 63
        if c != " ":
            g = tinted.crop(boxes[idx])
            gw, gh = max(1, round(g.width * scale)), max(1, round(g.height * scale))
            blit(canvas, g.resize((gw, gh), Image.LANCZOS), int(round(x)), int(y0))
        x += adv[idx] * scale


def draw_border_px(canvas, box, g=16, b=BTN_BORDER):
    """Button border in screen px: 4 edges + 4 corners of dimension g (same piece layout as character_page_mockup.draw_border)."""
    import character_page_mockup as cpm
    from PIL import Image, ImageDraw
    X, Y, W, H = box
    x0, y0, x1, y1 = X, Y, X + W, Y + H
    col = b["COLOR"]
    try:
        edge, corner = cpm.texture(b["EDGE"]), cpm.texture(b["CORNER"])
    except Exception:
        edge = corner = None
    if edge is None and corner is None:   # no game textures: plain outline
        ImageDraw.Draw(canvas).rectangle((x0, y0, x1 - 1, y1 - 1), outline=tuple(int(v * 255) for v in col) + (255,), width=2)
        return
    dx, dy = min(g, W // 2), min(g, H // 2)
    ps = cpm.paste_stretched
    if edge is not None:
        e = cpm.tint(edge, col)
        ps(canvas, e, (x0 + dx, y0, x1 - dx, y0 + dy))
        ps(canvas, e.transpose(Image.FLIP_TOP_BOTTOM), (x0 + dx, y1 - dy, x1 - dx, y1))
        left = e.transpose(Image.ROTATE_90)
        ps(canvas, left, (x0, y0 + dy, x0 + dx, y1 - dy))
        ps(canvas, left.transpose(Image.FLIP_LEFT_RIGHT), (x1 - dx, y0 + dy, x1, y1 - dy))
    if corner is not None:
        c = cpm.tint(corner, col)
        ps(canvas, c, (x0, y0, x0 + dx, y0 + dy))
        ps(canvas, c.transpose(Image.FLIP_LEFT_RIGHT), (x1 - dx, y0, x1, y0 + dy))
        ps(canvas, c.transpose(Image.FLIP_TOP_BOTTOM), (x0, y1 - dy, x0 + dx, y1))
        ps(canvas, c.transpose(Image.ROTATE_180), (x1 - dx, y1 - dy, x1, y1))


def line_rects(lines, rect, y0, line_h, scale, left):
    """Union box [x, y, w, h] of the drawn lines (what the glyphs really cover), None when there is no text."""
    xs = []
    for k, ln in enumerate(lines):
        if not ln:
            continue
        w = text_px(ln, scale)
        x = rect[0] if left else rect[0] + (rect[2] - w) / 2.0
        xs.append((x, x + w, y0 + k * line_h))
    if not xs:
        return None
    x0, x1 = min(a for a, _, _ in xs), max(b for _, b, _ in xs)
    ys = [y for _, _, y in xs]
    return [int(x0), min(ys), int(x1 - int(x0) + 0.999), max(ys) + line_h - min(ys)]


def render(W, H, panes, dump, mode="engine-model", fix=0, g=16, m=9, line_h=None, no_drop=False, crop=False,
           button_fit_pad=DEFAULT_BUTTON_FIT_PAD, label_fit_pad=DEFAULT_LABEL_FIT_PAD, text_k=DEFAULT_TEXT_K, remove_open=False):
    """-> (RGBA image, sidecar dict). remove_open: the "..." menu is open (the Remove button is drawn and the left box shows the Remove sentence); closed, the
    editor parks the button off screen, so it is listed (hidden) but not drawn."""
    from PIL import Image, ImageDraw
    install_font_fix()
    scale = engine_scale(W)
    fscale = fit_scale(text_k)
    if line_h is None:
        line_h = round(32 * text_scale(W) / 2)
    canvas = Image.new("RGBA", (W, H), (10, 20, 22, 255))
    d = ImageDraw.Draw(canvas)
    desc = dump["panes"].get("desc")
    for p in list(panes) + ([desc] if desc and desc[2] > 0 and desc[3] > 0 else []):
        d.rectangle((p[0], p[1], p[0] + p[2] - 1, p[1] + p[3] - 1), outline=(40, 70, 70, 255))
    items = []
    for it in dump["items"]:
        i, kind, box = it["i"], it["kind"], tuple(it["box"])
        X, Y, bw, bh = box
        text = caption_for(i, remove_open)
        rec = {"i": i, "kind": kind, "text": text, "box": list(box)}
        if i == PI_REMOVE and not remove_open:   # parked off screen by applyLayout while the menu is closed
            rec.update(caption_rect=list(box), drawn=False, text_y0=None, shown="", lines=0, wraps=False, text_rect=None, hidden=True)
            items.append(rec)
            continue
        if bw <= 0 or bh <= 0:   # absent item (e.g. no description pane / too small): nothing to draw
            rec.update(caption_rect=list(box), drawn=False, text_y0=None, shown="", lines=0, wraps=False, text_rect=None)
            items.append(rec)
            continue
        if kind == "frame":
            d.rectangle((X, Y, X + bw - 1, Y + bh - 1), fill=DESC_FILL)
            draw_border_px(canvas, box, g, {**BTN_BORDER, "COLOR": DESC_EDGE})
            rec.update(caption_rect=list(box), drawn=False, text_y0=None, shown="", lines=0, wraps=False, text_rect=None)
            items.append(rec)
            continue
        if kind == "desctext":
            lines = wrap_text(text, bw, scale) if text else []
            overflow = len(lines) * line_h > bh
            rec.update(caption_rect=list(box), drawn=bool(lines), text_y0=Y, shown=text, lines=len(lines), wraps=len(lines) > 1, overflow=overflow,
                       text_rect=line_rects(lines, box, Y, line_h, scale, True))
            for k, ln in enumerate(lines):
                draw_line(canvas, ln, box, Y + k * line_h, scale, DESC_TEXT, left=True)
            items.append(rec)
            continue
        if kind == "view":
            d.rectangle((X, Y, X + bw - 1, Y + bh - 1), fill=(6, 10, 12, 255), outline=(60, 100, 100, 255))
            rec.update(caption_rect=list(box), drawn=False, text_y0=None, text_rect=None)
            items.append(rec)
            continue
        if kind == "button":
            draw_border_px(canvas, box, g)
        elif i == PI_MEMBER_NAME:   # name plate: the hint-box frame fill + edge (SA_Box), centred lit text (HolsterEditor.cpp applyArt NAMEBOX)
            d.rectangle((X, Y, X + bw - 1, Y + bh - 1), fill=DESC_FILL)
            draw_border_px(canvas, box, g, {**BTN_BORDER, "COLOR": DESC_EDGE})
        elif 36 <= i <= 65 and (i - 36) % 5 in (1, 2):   # slider track / thumb: plain shapes
            fill = (30, 60, 56, 255) if (i - 36) % 5 == 1 else (204, 204, 178, 255)
            d.rectangle((X, Y, X + bw - 1, Y + bh - 1), fill=fill)
        pad = button_fit_pad if kind == "button" else label_fit_pad
        shown = fit_text(text, bw - pad if bw > pad else 0, fscale) if text else ""
        rect0, _, _ = place_caption(kind, box, mode, fix, g, m, line_h, no_drop)
        lines = wrap_text(shown, wrap_width(kind, box, rect0, mode), scale) if shown else []
        n = max(1, len(lines))
        rect, drawn, y0 = place_caption(kind, box, mode, fix, g, m, line_h, no_drop, lines=n)
        rec.update(caption_rect=list(rect), drawn=bool(drawn and shown), text_y0=y0, shown=shown, lines=len(lines), wraps=len(lines) > 1,
                   text_rect=line_rects(lines, rect, y0, line_h, scale, False) if (drawn and shown) else None)
        if rec["drawn"]:
            for k, ln in enumerate(lines):
                draw_line(canvas, ln, rect, y0 + k * line_h, scale, TEAL_TEXT if kind == "button" else (LIT_TEXT if i == PI_MEMBER_NAME else CREAM))
        items.append(rec)
    side = {"width": W, "height": H, "mode": mode, "caption_fix": fix, "dim": g, "margin": m, "line_h": line_h, "text_scale": text_scale(W), "engine_px_per_texel": scale,
            "button_fit_pad": button_fit_pad, "label_fit_pad": label_fit_pad, "text_k": text_k, "fit_px_per_texel": fscale,
            "panes": {"column": list(panes[0]), "view": list(panes[1]), "preview": list(panes[2]), "desc": list(desc) if desc else None}, "rows": dump["rows"], "remove_open": remove_open, "items": items}
    if crop:
        x0 = max(0, min(p[0] for p in panes) - 40)
        y0 = max(0, min(p[1] for p in panes) - 40)
        x1 = min(W, max(p[0] + p[2] for p in panes) + 40)
        y1 = min(H, max(p[1] + p[3] for p in panes) + 40)
        canvas = canvas.crop((x0, y0, x1, y1))
        side["crop"] = [x0, y0, x1 - x0, y1 - y0]
    return canvas, side


def parse_rect(s):
    v = [int(t) for t in s.split(",")]
    if len(v) != 4:
        raise argparse.ArgumentTypeError("expected x,y,w,h")
    return tuple(v)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--panes", default="3840x1600", help="WxH screen size (default 3840x1600: the measured pane set)")
    ap.add_argument("--col", type=parse_rect)
    ap.add_argument("--view", type=parse_rect)
    ap.add_argument("--preview", type=parse_rect)
    ap.add_argument("--desc", type=parse_rect, help="shrunk description pane (mod-config LB_DESC) X,Y,W,H in engine px (default: bottom 15%% of the original pane under the preview, see default_desc)")
    ap.add_argument("--no-desc", action="store_true", help="old layout without description boxes")
    ap.add_argument("--desc-cover-px", type=int, default=None, help="ini desc_cover_px: the right description box is grown this far on every side to cover LB_DESC's frame (default 6, 0 = the pane itself)")
    ap.add_argument("--mode", choices=("engine-model", "ideal"), default="engine-model")
    ap.add_argument("--caption-fix", type=int, choices=(0, 1), default=0)
    ap.add_argument("--dim", type=int, default=16, help="border DIMENSION G (px)")
    ap.add_argument("--margin", type=int, default=9, help="INNEROFFSET M")
    ap.add_argument("--line-h", type=int, default=None, help="text line height px (default 32 at text scale 2.0, scaled otherwise)")
    ap.add_argument("--no-drop", action="store_true", help="draw a caption even when its rect is shorter than the line")
    ap.add_argument("--button-fit-pad", type=int, default=None, help=f"button fit width = w - pad (default {DEFAULT_BUTTON_FIT_PAD})")
    ap.add_argument("--label-fit-pad", type=int, default=None, help=f"label fit width = w - pad (default {DEFAULT_LABEL_FIT_PAD})")
    ap.add_argument("--text-k", type=float, default=None, help=f"ini text_k: fit px per texel = text_k * 0.75 (default {DEFAULT_TEXT_K})")
    ap.add_argument("--legacy-fit", action="store_true", help=f"the 0.4.3 build: pad {LEGACY_BUTTON_FIT_PAD} for all, text_k {LEGACY_TEXT_K}")
    ap.add_argument("--remove-open", action="store_true", help="draw the page with the \"...\" menu open: the Remove saved placement button shown (item 72) and the Remove sentence in the left box")
    ap.add_argument("--crop", action="store_true", help="output the pane bounding box + 40 px margin")
    ap.add_argument("--dump-json", help="use this layout dump JSON instead of compiling/running the C++ tool")
    ap.add_argument("--build-dir", help="where the dump tool is compiled (default: system temp)")
    ap.add_argument("--out", required=True)
    a = ap.parse_args(argv)
    W, H = (int(v) for v in a.panes.lower().split("x"))
    base = panes_for(W, H)
    panes = (a.col or base[0], a.view or base[1], a.preview or base[2])
    if a.dump_json:
        dump = json.loads(Path(a.dump_json).read_text())
    else:
        desc = None if a.no_desc else (a.desc or default_desc(panes[2]))
        dump = run_dump(*panes, desc=desc, build_dir=a.build_dir, cover=a.desc_cover_px)
    if a.legacy_fit:
        defaults = (LEGACY_BUTTON_FIT_PAD, LEGACY_LABEL_FIT_PAD, LEGACY_TEXT_K)
    else:
        defaults = (DEFAULT_BUTTON_FIT_PAD, DEFAULT_LABEL_FIT_PAD, DEFAULT_TEXT_K)
    bpad, lpad, tk = (d if v is None else v for v, d in zip((a.button_fit_pad, a.label_fit_pad, a.text_k), defaults))
    img, side = render(W, H, panes, dump, a.mode, a.caption_fix, a.dim, a.margin, a.line_h, a.no_drop, a.crop, bpad, lpad, tk, a.remove_open)
    out = Path(a.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    img.convert("RGB").save(out)
    out.with_suffix(".json").write_text(json.dumps(side, indent=1))
    n = sum(1 for it in side["items"] if it["kind"] == "button")
    dropped = sum(1 for it in side["items"] if it["kind"] == "button" and it["text"] and not it["drawn"] and not it.get("hidden"))
    wrapped = [it["i"] for it in side["items"] if it.get("wraps")]
    wb = sum(1 for it in side["items"] if it.get("wraps") and it["kind"] == "button")
    print(f"{out} ({img.width}x{img.height}) buttons={n} dropped={dropped} wrapped={len(wrapped)} (buttons {wb}) items={wrapped}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
