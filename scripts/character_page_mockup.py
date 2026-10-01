#!/usr/bin/env python3
"""Render the character page "thirds" layout + Mod 14 Active Effects panel mock-up (doc 14, Phase 1).

Reads the LIVE override/character_p.gui and draws it at 3840x1600 with the REAL GUI resources: every BORDER
fill/edge/corner texture (pykotor Installation.texture, TPC rows are bottom-up so they are flipped), tinted by the
control's BORDER COLOR (multiply), the root pnl_pause_pc frame, and the dialogfont16x16/10x10 bitmap fonts (glyph
boxes from the font TXI upperleft/lowerright coords) in the control's TEXT COLOR. Texts with a STRREF come from
dialog.tlk. Sample effect rows use real spell icons (EffectIconTable.inc / DebuffIconTable.inc resrefs) and real
spells.2da names; the times are fake.

Geometry: GUI units are 800x600, the engine stretches them to the screen, so screen px = units * (4.8, 8/3) at
3840x1600. A square on screen is therefore 10 x 18 units (48 x 48 px). Text is drawn at a UNIFORM 8/3 px per font
texel (assumption: the engine does not stretch glyphs horizontally; --text-hstretch draws them at 4.8 px instead,
for comparison with a screenshot).

Thirds (columns as equip_p layout A: 48..284 | 294..526 | 536..752):
  left   stats group      dx -353 (x 407 -> 54)
  middle 3D group         dx +129 (x 166 -> 295), view shortened to end at y 470 (above the LBL_BAR5 notch)
  right  effects panel    [536,134,216,336] + header [536,104,216,24]
Fixed: LBL_BAR1..5, LBL_TITLE, BTN_EXIT.

usage: character_page_mockup.py OUTDIR [--text-hstretch] [--names-font=dialogfont16x16|dialogfont10x10]
Outputs: char_before.png, char_after.png, char_after_empty.png, char_after_tooltip.png, char_after_panel_zoom.png
"""
import logging, re, sys
from functools import lru_cache
from pathlib import Path
from PIL import Image, ImageDraw
logging.disable(logging.CRITICAL)
from pykotor.extract.installation import Installation
from pykotor.resource.formats.gff import read_gff
from pykotor.resource.formats.tpc import TPCTextureFormat
from pykotor.resource.formats.twoda import read_2da
from pykotor.resource.type import ResourceType

GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
REPO = Path(__file__).resolve().parent.parent
BUFF_SRC = REPO / "patch_manager_mods/buff_duration_hud_src"
W, H = 3840, 1600
SX, SY = W / 800, H / 600

# ---- palette taken from character_p.gui (BORDER / TEXT COLOR fields) -----------------------------------------
TEAL_FRAME = (0.05098, 0.34902, 0.27059)   # every frame/fill tint
TEAL_TEXT = (0.10196, 0.69804, 0.54902)    # stat labels
CREAM = (0.8, 0.8, 0.69804)                # LBL_TITLE, button text
ORANGE = (0.94902, 0.65882, 0.30196)       # LBL_FORCEMASTERY ("Light Mastery: +3 STR")

# ---- thirds layout ------------------------------------------------------------------------------------------------
STATS_DX = 54 - 407
VIEW_DX = 295 - 166
GROUP_3D = ["LBL_3DCHAR", "BTN_3DCHAR", "LBL_BEVEL", "LBL_BEVEL2", "SLD_ALIGN", "LBL_LIGHT", "LBL_DARK",
            "BTN_AUTO", "BTN_LEVELUP", "LBL_FORCEMASTERY"]
FIXED = ["LBL_BAR1", "LBL_BAR2", "LBL_BAR3", "LBL_BAR4", "LBL_BAR5", "LBL_TITLE", "BTN_EXIT"]
# explicit rect overrides AFTER the group shift (view ends at y 470, covers follow, mastery text fits the column)
OVERRIDES = {
    "LBL_3DCHAR": (295, 134, 230, 336), "BTN_3DCHAR": (295, 134, 230, 336),
    "LBL_BEVEL": (293, 130, 234, 38), "LBL_BEVEL2": (293, 436, 234, 38),
    "LBL_FORCEMASTERY": (294, 105, 232, 21),
    # slider group lifted 13 and slider shortened 14 so LBL_DARK clears the bottom arc (LBL_BEVEL2 starts at y 436);
    # vanilla relation kept: LBL_DARK top = slider end - 15, LBL_LIGHT top = slider top - 7
    "LBL_LIGHT": (304, 170, 52, 21), "SLD_ALIGN": (310, 177, 36, 242), "LBL_DARK": (304, 404, 52, 21),
}

# ---- effects panel (new controls; Phase 2 adds them to character_p.gui with IDs 69..) ----------------------------
PANEL = (536, 134, 216, 336)
HEADER = (536, 104, 216, 24)
PAD_X, PAD_TOP = 8, 6
ROW_H, SEC_H, SEC_GAP = 22, 20, 6
ICON_W, ICON_H = 10, 18            # square on screen (48x48 px)
TIME_W = 54                        # fits "Permanent" in dialogfont16x16 (52.2 units)
MAX_ROWS = 14                      # label pool: rows that fit the scroll window at once
SB_W, SB_ARROW_H, SB_GAP = 10, 18, 3   # scrollbar (shown only when the list overflows): width, arrow height, gap to rows
SB_THUMB_MIN = 16

# sample data: (spell row, remaining seconds or None for permanent, item ability?)
SAMPLE_GOOD = [(36, 161, False), (34, 47, False), (135, 72, False), (99, 18, True), (69, 295, True),
               (262, None, False)]
SAMPLE_BAD = [(45, 9, False), (7, 22, False), (16, 6, False)]


def inst():
    return _inst()


@lru_cache(None)
def _inst():
    return Installation(GAME)


@lru_cache(None)
def texture(name):
    """RGBA PIL image of a game texture, top row first; None if missing. Engine order: override .tpc, override .tga,
    then the texture packs. TPC mip data is stored bottom-up (flip); TGA is decoded by PIL with its own origin."""
    if not name:
        return None
    ov = {p.name.lower(): p for p in (GAME / "override").iterdir()}
    if f"{name}.tpc".lower() not in ov and f"{name}.tga".lower() in ov:
        return Image.open(ov[f"{name}.tga".lower()]).convert("RGBA")
    t = inst().texture(name)
    if t is None:
        return None
    t.convert(TPCTextureFormat.RGBA)
    m = t.get(0, 0)
    return Image.frombytes("RGBA", (m.width, m.height), bytes(m.data)).transpose(Image.FLIP_TOP_BOTTOM)


def tint(img, col):
    r, g, b, a = img.split()
    r = r.point(lambda v: int(v * col[0])); g = g.point(lambda v: int(v * col[1])); b = b.point(lambda v: int(v * col[2]))
    return Image.merge("RGBA", (r, g, b, a))


@lru_cache(None)
def font(name):
    tex = texture(name)
    txi = inst().texture(name).txi.splitlines()
    i = txi.index("upperleftcoords"); j = txi.index("lowerrightcoords")
    ul = [tuple(map(float, l.split()[:2])) for l in txi[i + 1:i + 257]]
    lr = [tuple(map(float, l.split()[:2])) for l in txi[j + 1:j + 257]]
    tw, th = tex.size
    boxes = [(round(u0 * tw), round((1 - v0) * th), round(u1 * tw), round((1 - v1) * th)) for (u0, v0), (u1, v1) in zip(ul, lr)]
    return tex, boxes


def text_size(name, s, hscale, vscale):
    tex, boxes = font(name)
    wid = sum(boxes[ord(c) if ord(c) < 256 else 63][2] - boxes[ord(c) if ord(c) < 256 else 63][0] for c in s)
    ht = boxes[ord("A")][3] - boxes[ord("A")][1]
    return wid * hscale, ht * vscale


def fit(name, s, max_px, hscale, vscale):
    if text_size(name, s, hscale, vscale)[0] <= max_px:
        return s
    while s and text_size(name, s + "...", hscale, vscale)[0] > max_px:
        s = s[:-1]
    return s.rstrip() + "..."


def draw_text(canvas, name, s, rect_px, align, col, hscale, vscale):
    """align = KotOR TEXT ALIGNMENT: low bits 1/2/3 = left/centre/right, 8 top, 16 middle, 32 bottom."""
    tex, boxes = font(name)
    tw, th = text_size(name, s, hscale, vscale)
    x0, y0, x1, y1 = rect_px
    h = align & 3
    x = x0 if h == 1 else (x1 - tw if h == 3 else (x0 + x1 - tw) / 2)
    y = y0 if align & 8 else (y1 - th if align & 32 else (y0 + y1 - th) / 2)
    tinted = tint(tex, col)
    for c in s:
        b = boxes[ord(c) if ord(c) < 256 else 63]
        g = tinted.crop(b)
        gw, gh = max(1, round(g.width * hscale)), max(1, round(g.height * vscale))
        if c != " ":
            canvas.alpha_composite(g.resize((gw, gh), Image.LANCZOS), (int(round(x)), int(round(y))))
        x += g.width * hscale


def px(r):
    l, t, w, h = r
    return (l * SX, t * SY, (l + w) * SX, (t + h) * SY)


def paste_stretched(canvas, img, box):
    x0, y0, x1, y1 = [int(round(v)) for v in box]
    if x1 - x0 < 1 or y1 - y0 < 1:
        return
    canvas.alpha_composite(img.resize((x1 - x0, y1 - y0), Image.LANCZOS), (x0, y0))


def draw_border(canvas, b, rect):
    """Fill over the whole rect, then the 4 edges + 4 corners at DIMENSION units (corner texture = top-left)."""
    col = b.get("COLOR", (1, 1, 1))
    x0, y0, x1, y1 = px(rect)
    fill = texture(b.get("FILL"))
    if fill is not None:
        paste_stretched(canvas, tint(fill, col), (x0, y0, x1, y1))
    dim = b.get("DIMENSION", 0)
    edge, corner = texture(b.get("EDGE")), texture(b.get("CORNER"))
    if not dim or (edge is None and corner is None):
        return
    dx, dy = dim * SX, dim * SY
    dx = min(dx, (x1 - x0) / 2); dy = min(dy, (y1 - y0) / 2)
    if edge is not None:
        e = tint(edge, col)
        paste_stretched(canvas, e, (x0 + dx, y0, x1 - dx, y0 + dy))                                    # top
        paste_stretched(canvas, e.transpose(Image.FLIP_TOP_BOTTOM), (x0 + dx, y1 - dy, x1 - dx, y1))   # bottom
        left = e.transpose(Image.ROTATE_90)                                                            # line -> left
        paste_stretched(canvas, left, (x0, y0 + dy, x0 + dx, y1 - dy))
        paste_stretched(canvas, left.transpose(Image.FLIP_LEFT_RIGHT), (x1 - dx, y0 + dy, x1, y1 - dy))
    if corner is not None:
        c = tint(corner, col)
        paste_stretched(canvas, c, (x0, y0, x0 + dx, y0 + dy))
        paste_stretched(canvas, c.transpose(Image.FLIP_LEFT_RIGHT), (x1 - dx, y0, x1, y0 + dy))
        paste_stretched(canvas, c.transpose(Image.FLIP_TOP_BOTTOM), (x0, y1 - dy, x0 + dx, y1))
        paste_stretched(canvas, c.transpose(Image.ROTATE_180), (x1 - dx, y1 - dy, x1, y1))


# ---- GUI model ------------------------------------------------------------------------------------------------------
def _border(s):
    if s is None:
        return {}
    v = s.get_vector3("COLOR")
    return {"FILL": str(s.get_resref("FILL")), "EDGE": str(s.get_resref("EDGE")), "CORNER": str(s.get_resref("CORNER")),
            "DIMENSION": s.get_int32("DIMENSION"), "COLOR": (v.x, v.y, v.z)}


def load_gui():
    g = read_gff(str(next(p for p in (GAME / "override").iterdir() if p.name.lower() == "character_p.gui")))
    tlk = inst().talktable()
    ctls = []
    for c in g.root.get_list("CONTROLS"):
        e = c.get_struct("EXTENT")
        d = {"TAG": c.get_string("TAG"), "ID": c.get_int32("ID"), "TYPE": c.get_int32("CONTROLTYPE"),
             "RECT": tuple(e.get_int32(k) for k in ("LEFT", "TOP", "WIDTH", "HEIGHT")),
             "BORDER": _border(c.get_struct("BORDER") if c.exists("BORDER") else None)}
        if c.exists("TEXT"):
            t = c.get_struct("TEXT")
            v = t.get_vector3("COLOR")
            s = t.get_string("TEXT") if t.exists("TEXT") else ""
            sr = t.get_uint32("STRREF") if t.exists("STRREF") else 0xFFFFFFFF
            if sr != 0xFFFFFFFF:
                s = tlk.string(sr)
            d["TEXT"] = {"S": s, "FONT": str(t.get_resref("FONT")), "ALIGN": t.get_int32("ALIGNMENT"), "COLOR": (v.x, v.y, v.z)}
        if c.exists("THUMB"):
            d["THUMB"] = str(c.get_struct("THUMB").get_resref("IMAGE"))
            d["CURVALUE"], d["MAXVALUE"] = c.get_int32("CURVALUE"), c.get_int32("MAXVALUE")
        ctls.append(d)
    root = _border(g.root.get_struct("BORDER"))
    return root, ctls


def stats_group(ctls):
    return [c["TAG"] for c in ctls if 405 <= c["RECT"][0] < 640 and c["TAG"] not in FIXED]


def thirds(ctls):
    out = [dict(c) for c in ctls]
    stats = set(stats_group(ctls))
    for c in out:
        l, t, w, h = c["RECT"]
        if c["TAG"] in stats:
            c["RECT"] = (l + STATS_DX, t, w, h)
        elif c["TAG"] in GROUP_3D:
            c["RECT"] = OVERRIDES.get(c["TAG"], (l + VIEW_DX, t, w, h))
    return out


def check(ctls):
    """Layout asserts used by the mock-up and (later) the deploy script."""
    errs = []
    cols = {"left": (48, 284), "mid": (280, 535), "right": (536, 752)}
    for c in ctls:
        l, t, w, h = c["RECT"]
        if c["TAG"] in FIXED:
            continue
        if t + h > 476 and l < 634 and l + w > 407 and c["TAG"] != "LBL_BEVEL2":
            errs.append(f"{c['TAG']} {c['RECT']} reaches into the LBL_BAR5 notch")
        if c["TAG"] == "LBL_BEVEL2" and t + h > 476:
            errs.append(f"LBL_BEVEL2 {c['RECT']} overlaps BAR5")
    by = {c["TAG"]: c["RECT"] for c in ctls}
    arc = by["LBL_BEVEL2"][1]
    for t in ("SLD_ALIGN", "LBL_DARK"):
        if by[t][1] + by[t][3] > arc:
            errs.append(f"{t} {by[t]} runs into the bottom arc (y {arc})")
    for t in stats_group(ctls):
        l, _, w, _ = by[t]
        if l < cols["left"][0] or l + w > cols["left"][1]:
            errs.append(f"{t} {by[t]} outside the left third")
    return errs


# ---- effect data -------------------------------------------------------------------------------------------------
def icon_tables():
    good, bad = {}, {}
    for l in open(BUFF_SRC / "EffectIconTable.inc"):
        m = re.search(r'/\*(\d+)\*/ "(\w+)"', l)
        if m: good[int(m.group(1))] = m.group(2)
    for l in open(BUFF_SRC / "DebuffIconTable.inc"):
        m = re.search(r'/\*(\d+)\*/ \{ "(\w+)"', l)
        if m: bad[int(m.group(1))] = m.group(2)
    return good, bad


@lru_cache(None)
def spells():
    return read_2da(inst().resource("spells", ResourceType.TwoDA).data)


def spell_text(row, col):
    v = spells().get_row(row).get_string(col)
    return inst().talktable().string(int(v)) if v.isdigit() else ""


def fmt_time(s):
    if s is None:
        return "Permanent"
    s = int(s)
    return f"{s // 3600}:{s // 60 % 60:02d}:{s % 60:02d}" if s >= 3600 else f"{s // 60}:{s % 60:02d}"


def entries(good, bad):
    """Virtual list the panel scrolls through: ('section', name, colour) / ('row', spell, secs, item, harmful)."""
    gt, bt = icon_tables()
    out = []
    # Harmful first (user 2026-10-01): debuffs stay visible without scrolling when the buff list is long
    for sec, rows, table, col in (("Harmful", bad, bt, ORANGE), ("Beneficial", good, gt, CREAM)):
        if not rows:
            continue
        out.append(("section", sec, col, bool(out)))          # 4th field: preceded by a section gap
        rows = sorted(rows, key=lambda r: (r[1] is None, r[1] or 0))   # expiring first, permanent last
        out += [("row", spell, secs, item, sec == "Harmful", table.get(spell)) for spell, secs, item in rows]
    return out


def entry_h(e, at_top=False):
    """Height in units; a section's leading gap is dropped when it is the first visible entry."""
    if e[0] == "section":
        return SEC_H + 2 + (SEC_GAP if e[3] and not at_top else 0)
    return ROW_H


def fits(ents, first, win):
    """Number of entries from `first` that fit the window."""
    n, acc = 0, 0
    while first + n < len(ents):
        h = entry_h(ents[first + n], n == 0)
        if acc + h > win:
            break
        acc += h; n += 1
    return n


def scroll_state(ents, first):
    """Whole-entry scrolling (engine labels cannot be clipped) -> (first, count, overflow, last_first)."""
    win = PANEL[3] - 2 * PAD_TOP
    if fits(ents, 0, win) == len(ents):
        return 0, len(ents), False, 0
    last_first = len(ents) - 1
    while last_first > 0 and fits(ents, last_first - 1, win) == len(ents) - (last_first - 1):
        last_first -= 1
    first = max(0, min(first, last_first))
    return first, fits(ents, first, win), True, last_first


def panel_controls(good, bad, names_font, hscale, vscale, first=0):
    """Lay out the panel the way the DLL will: header, frame, scrolled slice of [Beneficial + rows, Harmful + rows],
    scrollbar (up arrow, track, thumb, down arrow) only when the list overflows the window."""
    items = []   # (kind, rect, payload)
    items.append(("bar6", HEADER, "Active Effects"))
    items.append(("frame", PANEL, None))
    x0 = PANEL[0] + PAD_X
    inner_w = PANEL[2] - 2 * PAD_X
    top = PANEL[1] + PAD_TOP
    win = PANEL[3] - 2 * PAD_TOP
    ents = entries(good, bad)
    if not ents:
        items.append(("empty", (x0, PANEL[1], inner_w, PANEL[3]), "No active effects"))
        return items
    st = scroll_state(ents, first)
    first, n, over = st[0], st[1], st[2]
    row_w = inner_w - (SB_W + SB_GAP if over else 0)
    y = top
    for k, e in enumerate(ents[first:first + n]):
        if e[0] == "section":
            if e[3] and k > 0:
                y += SEC_GAP
            items.append(("section", (x0, y, row_w, SEC_H), (e[1], e[2])))
            y += SEC_H + 2
        else:
            _, spell, secs, item, harmful, icon = e
            items.append(("icon", (x0, y + (ROW_H - ICON_H) // 2, ICON_W, ICON_H), (icon, item)))
            nx = x0 + ICON_W + 4
            items.append(("name", (nx, y, row_w - (ICON_W + 4) - TIME_W - 2, ROW_H), spell_text(spell, "name")))
            items.append(("time", (x0 + row_w - TIME_W, y, TIME_W, ROW_H), (fmt_time(secs), harmful, secs)))
            y += ROW_H
    assert y <= top + win, f"panel overflow: y {y} > {top + win}"
    rows_drawn = sum(1 for it in items if it[0] == "name")
    assert rows_drawn <= MAX_ROWS, f"{rows_drawn} rows > label pool {MAX_ROWS}"
    if over:
        last_first = st[3]
        sx = x0 + inner_w - SB_W
        items.append(("sb_up", (sx, top, SB_W, SB_ARROW_H), first > 0))
        track = (sx, top + SB_ARROW_H + 2, SB_W, win - 2 * SB_ARROW_H - 4)
        items.append(("sb_track", track, None))
        th = max(SB_THUMB_MIN, round(track[3] * n / len(ents)))
        ty = track[1] + round((track[3] - th) * first / max(1, last_first))
        items.append(("sb_thumb", (sx + 2, ty, SB_W - 4, th), None))
        items.append(("sb_dn", (sx, top + win - SB_ARROW_H, SB_W, SB_ARROW_H), first < last_first))
    return items


# ---- rendering -----------------------------------------------------------------------------------------------------
def render(root, ctls, panel_items, names_font, hscale, vscale, tooltip=False, title=""):
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 255))
    bg = texture(root.get("FILL"))
    if bg is not None:
        paste_stretched(canvas, tint(bg, root.get("COLOR", (1, 1, 1))), (0, 0, W, H))
    by_tag = {c["TAG"]: c for c in ctls}
    # 3D view stand-in (the engine renders the creature model here)
    v = by_tag["LBL_3DCHAR"]["RECT"]
    x0, y0, x1, y1 = px(v)
    ov = Image.new("RGBA", (W, H), (0, 0, 0, 0)); dr = ImageDraw.Draw(ov)
    cx = (x0 + x1) / 2
    dr.rectangle([x0, y0, x1, y1], fill=(14, 22, 26, 255))
    dr.ellipse([cx - 55, y0 + 170, cx + 55, y0 + 300], fill=(58, 66, 72, 255))                 # head
    dr.rounded_rectangle([cx - 150, y0 + 300, cx + 150, y0 + 700], 60, fill=(50, 58, 64, 255))  # torso
    dr.rounded_rectangle([cx - 120, y0 + 680, cx - 20, y1 - 30], 30, fill=(46, 54, 60, 255))
    dr.rounded_rectangle([cx + 20, y0 + 680, cx + 120, y1 - 30], 30, fill=(46, 54, 60, 255))
    canvas.alpha_composite(ov)
    for c in ctls:
        if c["TAG"] in ("LBL_3DCHAR", "BTN_3DCHAR"):
            continue
        draw_border(canvas, c["BORDER"], c["RECT"])
        if c.get("THUMB"):
            th = texture(c["THUMB"])
            if th is not None:
                l, t, w, h = c["RECT"]
                ty = t + h * (1 - c["CURVALUE"] / max(1, c["MAXVALUE"]))
                paste_stretched(canvas, th, px((l - 2, ty - 4, w + 4, 8)))
        tx = c.get("TEXT")
        if tx and tx["S"]:
            draw_text(canvas, tx["FONT"] or "dialogfont16x16", tx["S"], px(c["RECT"]), tx["ALIGN"], tx["COLOR"], hscale, vscale)
    stats_hdr = by_tag["LBL_BAR6"]["RECT"]
    for kind, rect, pl in panel_items:
        if kind == "bar6":
            draw_border(canvas, by_tag["LBL_BAR6"]["BORDER"], rect)
            draw_text(canvas, "dialogfont16x16", pl, px(rect), 18, CREAM, hscale, vscale)
        elif kind == "frame":
            # black interior (uibit_fill_2bt, the HUD moulding fill) + thin-line frame (LBL_MORE_BACK edge/corner)
            draw_border(canvas, {"FILL": "uibit_fill_2bt", "COLOR": (0, 0, 0)}, rect)
            draw_border(canvas, by_tag["LBL_MORE_BACK"]["BORDER"], rect)
        elif kind == "section":
            # dark teal band (the XP_BACK translucent fill) only under the section header
            l, t, w, h = rect
            draw_border(canvas, {"FILL": "uibit_fill_2wt", "COLOR": TEAL_FRAME}, (l - 4, t, w + 8, h))
            draw_text(canvas, "dialogfont16x16", pl[0].upper(), px((l + 2, t, w - 2, h)), 17, pl[1], hscale, vscale)
        elif kind == "icon":
            resref, item = pl
            box = px(rect)
            if item:
                disc = Image.open(BUFF_SRC / "buffhud_circle.tga").convert("RGBA")
                paste_stretched(canvas, tint(disc, (0.25, 0.75, 0.65)), box)
            ic = texture(resref) if resref else None
            if ic is None:
                ic = texture("ip_mind")
            paste_stretched(canvas, ic, box)
        elif kind == "name":
            s = fit(names_font, pl, px(rect)[2] - px(rect)[0], hscale, vscale)
            draw_text(canvas, names_font, s, px(rect), 17, TEAL_TEXT, hscale, vscale)
        elif kind == "time":
            s, harmful, secs = pl
            col = ORANGE if harmful else (CREAM if secs is None or secs >= 30 else ORANGE)
            fnt = names_font
            draw_text(canvas, fnt, s, px(rect), 19, col if not harmful else ORANGE, hscale, vscale)
        elif kind in ("sb_up", "sb_dn"):
            arr = texture("uibit_uparrowg")
            if kind == "sb_dn":
                arr = arr.transpose(Image.FLIP_TOP_BOTTOM)
            paste_stretched(canvas, arr if pl else tint(arr, (0.35, 0.35, 0.35)), px(rect))
        elif kind == "sb_track":
            draw_border(canvas, {"EDGE": "uibit_brdr_16ge", "CORNER": "uibit_brdr_16gc", "DIMENSION": 4, "COLOR": (1, 1, 1)}, rect)
        elif kind == "sb_thumb":
            draw_border(canvas, {"FILL": "uibit_fill_16g", "COLOR": (1, 1, 1)}, rect)
        elif kind in ("more", "empty"):
            draw_text(canvas, "dialogfont16x16", pl, px(rect), 18, CREAM, hscale, vscale)
    if tooltip:
        name_rows = [r for k, r, p in panel_items if k == "name"]
        if name_rows:
            r = name_rows[0]
            tip = (PANEL[0] + 4, r[1] + ROW_H + 2, PANEL[2] - 8, 104)    # below the hovered row, inside the panel
            tx0, ty0, tx1, ty1 = px(tip)
            ImageDraw.Draw(canvas).rounded_rectangle([tx0 + 8, ty0 + 8, tx1 - 8, ty1 - 8], 30, fill=(0, 0, 0, 255))
            draw_border(canvas, {"FILL": "uibit_fill_2bt", "EDGE": "uibit_brdr_16bet", "CORNER": "uibit_brdr_16bct",
                                 "DIMENSION": 16, "COLOR": TEAL_FRAME}, tip)
            spell = SAMPLE_GOOD[0][0]
            desc = " ".join(spell_text(spell, "spelldesc").split())
            lh = text_size("dialogfont10x10", "A", hscale, vscale)[1] + 8
            draw_text(canvas, "dialogfont16x16", spell_text(spell, "name"), (tx0 + 40, ty0 + 24, tx1, ty0 + 70), 9, CREAM, hscale, vscale)
            lines = wrap(desc, "dialogfont10x10", tx1 - tx0 - 80, hscale, vscale)
            maxl = int((ty1 - ty0 - 100) // lh)
            if len(lines) > maxl:
                lines = lines[:maxl]; lines[-1] = lines[-1] + " ..."
            for i, ln in enumerate(lines):
                yy = ty0 + 84 + i * lh
                draw_text(canvas, "dialogfont10x10", ln, (tx0 + 40, yy, tx1, yy + lh), 9, TEAL_TEXT, hscale, vscale)
    if title:
        ImageDraw.Draw(canvas).text((12, H - 24), title, fill=(120, 120, 120, 255))
    return canvas.convert("RGB")


def wrap(s, fnt, max_px, hscale, vscale):
    lines, cur = [], ""
    for w in s.split():
        t = (cur + " " + w).strip()
        if text_size(fnt, t, hscale, vscale)[0] > max_px and cur:
            lines.append(cur); cur = w
        else:
            cur = t
    if cur:
        lines.append(cur)
    return lines


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    out = Path(args[0]); out.mkdir(parents=True, exist_ok=True)
    hscale = SX / 1.0 * (1 if "--text-hstretch" in sys.argv else SY / SX)
    vscale = SY
    names_font = next((a.split("=", 1)[1] for a in sys.argv if a.startswith("--names-font=")), "dialogfont16x16")
    root, ctls = load_gui()
    print(f"live character_p.gui: {len(ctls)} controls, ids {sorted(c['ID'] for c in ctls)}")
    after = thirds(ctls)
    for c0, c1 in zip(ctls, after):
        if c0["RECT"] != c1["RECT"]:
            print(f"  {c1['TAG']:<22} {c0['RECT']} -> {c1['RECT']}")
    errs = check(after)
    print("checks:", "OK" if not errs else errs)
    good = [s for s in SAMPLE_GOOD]; bad = [s for s in SAMPLE_BAD]
    outs = {
        "char_before.png": render(root, ctls, [], names_font, hscale, vscale, title="before: live character_p.gui"),
        "char_after.png": render(root, after, panel_controls(good, bad, names_font, hscale, vscale), names_font, hscale, vscale,
                                 title="after: thirds + Active Effects (sample data)"),
        "char_after_empty.png": render(root, after, panel_controls([], [], names_font, hscale, vscale), names_font, hscale, vscale,
                                       title="after: empty state"),
        "char_after_overflow.png": render(root, after, panel_controls(good + [(17, 120, False), (24, 33, False), (41, 400, False)],
                                                                      bad + [(9, 3, False), (48, 41, False)], names_font, hscale, vscale),
                                          names_font, hscale, vscale, title="after: overflow, scrolled to the top"),
        "char_after_overflow_scrolled.png": render(root, after, panel_controls(good + [(17, 120, False), (24, 33, False), (41, 400, False)],
                                                                      bad + [(9, 3, False), (48, 41, False)], names_font, hscale, vscale, first=99),
                                          names_font, hscale, vscale, title="after: overflow, scrolled to the bottom"),
        "char_after_tooltip.png": render(root, after, panel_controls(good, bad, names_font, hscale, vscale), names_font, hscale, vscale,
                                         tooltip=True, title="after: hover tooltip (optional polish stage)"),
    }
    for n, im in outs.items():
        im.save(out / n); print(out / n)
    a = outs["char_after.png"]
    a.crop((int(270 * SX), int(90 * SY), int(770 * SX), int(520 * SY))).save(out / "char_after_zoom.png")
    print(out / "char_after_zoom.png")


if __name__ == "__main__":
    main()
