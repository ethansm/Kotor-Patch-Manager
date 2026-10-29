#!/usr/bin/env python3
"""Offline render of the Options > Mod Configuration > "Inventory Sort & Filter" page with the proposed LIST STYLE rows and a live preview list.

Owner request: make the inventory list UX configurable (icon / font size, row height, ...) from the mod config menu, with a mock list on the
page that shows what a setting renders like. This script renders that page at several settings, so the knobs and their ranges can be judged
before any C++ is written. Nothing here is in the mod yet; the real page schema is patch_manager_mods/inventory_sort_filter_src/SortOrderPage.h
(two rows: Sort order, Debug logging).

Page rows (left column, in this order, PAGE_ROWS):
  Sort order        Category / Base item / Name / Vanilla                (exists)
  Row height        Compact (96 px) / Vanilla (133 px = PROTOITEM 50 units * 2.667) / Large (168 px)
  Icon size         Small (64 px) / Vanilla (125 px = vanilla row - 8) / Large (160 px); clamped to row height - 8
  List font         Small (dialogfont10x10) / Large (dialogfont16x16 = the vanilla list font)
  Category dividers On / Off                                             (only has an effect in the Category sort)
  Debug logging     Off / On / Verbose                                   (exists)
The brief's example numbers (48 / 80 px) are below the measured vanilla row of 133 px at 3840x1600, so Compact and Large are 0.72x and 1.26x of it.

Reuses (no edits to them): isf_inventory_mockup.py (sample backpack, Category order, row / divider / New / tint look, v4b screen) and
character_page_mockup.py (cpm: real dialogfont bitmaps, uibit textures, gui units -> px at 3840x1600). The page frame is the real
optgameplay_p.gui (the panel mod-config clones its BTN_GAMEPLAY rows from): LBL_TITLE, LBL_BAR1-4, BTN_BACK; rows sit in the 284-unit column
pane with MenuUiLogic.h row geometry (slot k at y + k * h*36/284, height h*32/284, label + two 20 % arrow cells, gap 6); the preview takes the top
85 % of LB_DESC (kDescSplitPermille 850) and the hover description the bottom 15 %.

The full inventory screens (inventory_compact.png / inventory_large.png) call isf_inventory_mockup.render(variant "v4b") with its row loop
switched off (build_elements -> []) and its scroll thumb suppressed, then draw the list again here with the chosen row height / icon / font.

usage: isf_config_page_mockup.py OUTDIR
Outputs: page_default.png page_compact.png page_large.png page_dividers_off.png preview_strip.png inventory_compact.png inventory_large.png fit_table.txt
Approximations: arrow cells are plain teal frames with the arrow texture centred; the page hover frame is the cream button frame; the preview pane
is not clipped by the engine (rows are whole elements only, as in the inventory mock); the stack count sits bottom-right on the icon in dialogfont10x10.
"""
import sys
from dataclasses import dataclass, replace
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import isf_inventory_mockup as isf   # noqa: E402  (stdlib-only at import; PIL / pykotor are loaded lazily by the render functions)

SCREEN_W, SCREEN_H = isf.SCREEN_W, isf.SCREEN_H
SX, SY = SCREEN_W / 800, SCREEN_H / 600
VANILLA_ROW_PX = int(isf.PROTO[3] * SY)                 # 133 at 3840x1600
ICON_PAD = isf.ICON_PAD
TEXT_GAP = 10                                           # px between the icon and the name (vanilla: row_h + 6 = pad 4 + icon + 10)
NAME_RIGHT_PAD = 16
PREVIEW_W, PREVIEW_ROW_X = 1080, 10                      # preview list width (LB_DESC is 1113 px wide, the real LB_ITEMS 1132) and row offset in it
SMALL_FONT, LARGE_FONT = "dialogfont10x10", "dialogfont16x16"
DESC_SPLIT_PERMILLE = 850
DESC_TEXT = (37 / 255, 200 / 255, 164 / 255)
OPTIONS_GUI = "optgameplay_p.gui"
COLUMN_UNITS = (175, 142, 207, 284)                     # BTN_DIFFICULTY x / y / w and the 284-unit column pane (MenuUiLogic.h kRowRefH)
ROW_PITCH_REF, ROW_SLOT_REF, ROW_REF_H, ROW_GAP, ROW_ARROW_PCT = 36, 32, 284, 6, 20

# ---- page schema (what the brief specifies) -----------------------------------------------------------------------------
PAGE_ROWS = [
    {"id": "sort_order", "label": "Sort order", "choices": ["Category", "Base item", "Name", "Vanilla"], "default": 0,
     "desc": "Category, Base item, Name, or Vanilla (the game's own order)."},
    {"id": "row_height", "label": "Row height", "choices": ["Compact", "Vanilla", "Large"], "default": 1,
     "desc": "Height of each item row. Compact fits more items on screen."},
    {"id": "icon_size", "label": "Icon size", "choices": ["Small", "Vanilla", "Large"], "default": 1,
     "desc": "Size of the item icon. Small leaves more room for long names."},
    {"id": "list_font", "label": "List font", "choices": ["Small", "Large"], "default": 1,
     "desc": "Font of the item names. Large is the game's own list font."},
    {"id": "dividers", "label": "Category dividers", "choices": ["On", "Off"], "default": 0,
     "desc": "Category headings in the Category sort. Off saves half a row each."},
    {"id": "log_level", "label": "Debug logging", "choices": ["Off", "On", "Verbose"], "default": 0,
     "desc": "Off: startup and errors. On: each refresh and tab click. Verbose: every item too."},
]
ROW_IDS = [r["id"] for r in PAGE_ROWS]
ROW_PX = {"Compact": 96, "Vanilla": VANILLA_ROW_PX, "Large": 168}
ICON_REQ_PX = {"Small": 64, "Vanilla": VANILLA_ROW_PX - 2 * ICON_PAD, "Large": 160}
FONT_NAME = {"Small": SMALL_FONT, "Large": LARGE_FONT}


def default_values():
    return {r["id"]: r["default"] for r in PAGE_ROWS}


@dataclass(frozen=True)
class ListStyle:
    row: str = "Vanilla"
    icon: str = "Vanilla"
    font: str = "Large"
    dividers: str = "On"

    @property
    def row_h(self):
        return ROW_PX[self.row]

    @property
    def icon_px(self):
        return min(ICON_REQ_PX[self.icon], self.row_h - 2 * ICON_PAD)

    @property
    def font_name(self):
        return FONT_NAME[self.font]

    @property
    def dividers_on(self):
        return self.dividers == "On"

    def label(self):
        return "Rows %s | Icon %s | Font %s | Dividers %s" % (self.row, self.icon, self.font, self.dividers)


def style_from_values(vals):
    pick = {r["id"]: r["choices"][vals.get(r["id"], r["default"])] for r in PAGE_ROWS}
    return ListStyle(pick["row_height"], pick["icon_size"], pick["list_font"], pick["dividers"])


COMBOS = [ListStyle(r, i, f, "On") for r in ("Compact", "Vanilla", "Large") for i in ("Small", "Vanilla", "Large") for f in ("Small", "Large")]
STRIP_STYLES = [ListStyle("Vanilla", "Vanilla", "Large"), ListStyle("Compact", "Small", "Small"), ListStyle("Compact", "Vanilla", "Large"),
                ListStyle("Vanilla", "Small", "Small"), ListStyle("Large", "Large", "Large"), ListStyle("Large", "Vanilla", "Small")]


# ---- pure list logic (no PIL / game) ------------------------------------------------------------------------------------
def rows_capacity(avail, row_h):
    """Whole item rows that fit a window of avail px (no dividers)."""
    return avail // row_h


def heights(els, row_h):
    return [row_h if k[0] == "row" else row_h // 2 for k in els]


def elements_from_top(els, style, avail):
    """(elements, item rows) visible from the top of the list in a window of avail px."""
    n = isf.fit_count(heights(els, style.row_h), 0, avail)
    return n, sum(1 for k in els[:n] if k[0] == "row")


def category_elements(style, items=None):
    """The real backpack (all 22 sample items), Category sort, with or without dividers as the style says."""
    rows = isf.arrange(isf.S_ALL, isf.SO_CATEGORY, items)
    return isf.build_elements(rows, isf.SO_CATEGORY, "v4b" if style.dividers_on else "current")


def preview_items():
    by = {i.name: i for i in isf.SAMPLE}
    return [replace(by["Battle Stimulant"], stack=3), by["Light Combat Suit"], by["Mandalorian Heavy Armor"], by["Implant: Reflex Mk III"]]


# ---- rendering (needs PIL + pykotor + the live install) -----------------------------------------------------------------
_GEO = {}


def geometry(cpm):
    if not _GEO:
        _, ctls = isf.load_gui(cpm)
        by = {c["TAG"]: c for c in ctls}
        lb = isf.unit_px(cpm, by["LB_ITEMS"]["RECT"])
        geo = isf.list_geometry(cpm, lb, "v4b")
        sb = (int(268 * cpm.SX), geo["items"][1], int(16 * cpm.SX), geo["items"][3])
        _GEO.update(border=by["LB_ITEMS"]["BORDER"], lb=lb, geo=geo, sb=sb, off=geo["ry0"] - geo["items"][1],
                    row_x_rel=int(isf.PROTO[0] * cpm.SX) - geo["items"][0], row_w=int(isf.PROTO[2] * cpm.SX))
    return _GEO


def load_options(cpm):
    from pykotor.resource.formats.gff import read_gff
    g = read_gff(str(next(p for p in (cpm.GAME / "override").iterdir() if p.name.lower() == OPTIONS_GUI)))

    def border(s):
        if s is None:
            return {}
        v = s.get_vector3("COLOR")
        return {"FILL": str(s.get_resref("FILL")), "EDGE": str(s.get_resref("EDGE")), "CORNER": str(s.get_resref("CORNER")),
                "DIMENSION": s.get_int32("DIMENSION"), "COLOR": (v.x, v.y, v.z)}
    ctl = {}
    for c in g.root.get_list("CONTROLS"):
        e = c.get_struct("EXTENT")
        ctl[c.get_string("TAG")] = {"RECT": tuple(e.get_int32(k) for k in ("LEFT", "TOP", "WIDTH", "HEIGHT")),
                                    "BORDER": border(c.get_struct("BORDER") if c.exists("BORDER") else None)}
    return border(g.root.get_struct("BORDER")), ctl


def draw_scrollbar(img, cpm, qlm, sb, n_total, n_top, scroll=0):
    from PIL import Image
    sbx, sby, sbw, sbh = sb
    isf.frame_px(img, cpm, {"EDGE": "uibit_brdr_16ge", "CORNER": "uibit_brdr_16gc", "DIMENSION": 4}, sb, tint=isf.SB_THUMB_TINT)
    arrow = int(16 * cpm.SY)
    up = cpm.texture("uibit_uparrowg")
    if up is not None:
        cpm.paste_stretched(img, cpm.tint(up, isf.SB_THUMB_TINT), (sbx, sby, sbx + sbw, sby + arrow))
        cpm.paste_stretched(img, cpm.tint(up.transpose(Image.FLIP_TOP_BOTTOM), isf.SB_THUMB_TINT), (sbx, sby + sbh - arrow, sbx + sbw, sby + sbh))
    track = sbh - 2 * arrow - 8
    th = track if n_total <= n_top else max(24, int(track * n_top / n_total))
    toff = 0 if n_total <= n_top else int((track - th) * scroll / max(1, n_total - n_top))
    qlm.draw_fill(img, cpm, "uibit_fill_16g", (sbx + 4, sby + arrow + 4 + toff, sbw - 8, th), isf.SB_THUMB_TINT)


def draw_list(img, cpm, qlm, ry0, row_x, row_w, avail, els, style, items, selected):
    """The item rows (+ dividers) from the top of the window: icon, name, stack count, New / tint colours. Returns stats."""
    hts = heights(els, style.row_h)
    n = isf.fit_count(hts, 0, avail)
    proto = {"FILL": "uibit_fill_2bt", "EDGE": "uibit_brdr_16bet", "CORNER": "uibit_brdr_16bct", "DIMENSION": 16}
    y, shown, trunc = ry0, 0, 0
    rh, ip = style.row_h, style.icon_px
    for k in els[:n]:
        if k[0] == "div":
            isf.draw_divider(img, cpm, (row_x, y, row_w, rh // 2), k[1])
            y += rh // 2
            continue
        e = k[1]
        it = items[e.item]
        r = (row_x, y, row_w, rh)
        y += rh
        shown += 1
        ftint, ttint = isf.row_colours(e, e is selected)
        isf.frame_px(img, cpm, proto, r, tint=ftint)
        iy = r[1] + (rh - ip) // 2
        ic = isf.item_icon(cpm, qlm, it)
        if ic:
            qlm.draw_icon(img, cpm, ic, (r[0] + ICON_PAD, iy, ip, ip))
        if it.stack > 1:
            cpm.draw_text(img, SMALL_FONT, str(it.stack), (r[0] + ICON_PAD, iy, r[0] + ICON_PAD + ip - 4, iy + ip - 2), 35, (1, 1, 1))
        tx = r[0] + ICON_PAD + ip + TEXT_GAP
        box = r[0] + r[2] - NAME_RIGHT_PAD - tx
        name = cpm.fit(style.font_name, it.name, box)
        trunc += name != it.name
        cpm.draw_text(img, style.font_name, name, (tx, r[1], r[0] + r[2] - NAME_RIGHT_PAD, r[1] + r[3]), 17, ttint)
    return {"elems": n, "rows": shown, "total": len(els), "truncated": trunc}


def draw_preview(img, cpm, qlm, style, x0, y0):
    """Mini inventory list (same LB_ITEMS rect and v4b scroll bar as the real screen) with its top-left group corner at (x0, y0). -> stats + group size."""
    G = geometry(cpm)
    gi = G["geo"]["items"]
    lst = (x0, y0, PREVIEW_W, gi[3])
    isf.frame_px(img, cpm, G["border"], lst)
    items = preview_items()
    rows = isf.sort_entries(isf.build_entries(items), isf.SO_CATEGORY)
    els = isf.build_elements(rows, isf.SO_CATEGORY, "v4b" if style.dividers_on else "current")
    avail = gi[1] + gi[3] - G["geo"]["ry0"]
    st = draw_list(img, cpm, qlm, y0 + G["off"], x0 + PREVIEW_ROW_X, G["row_w"], avail, els, style, items, rows[0])
    sb = (x0 + PREVIEW_W - G["sb"][2] - 4, y0, G["sb"][2], gi[3])
    draw_scrollbar(img, cpm, qlm, sb, len(els), st["elems"])
    st.update(avail=avail, capacity=rows_capacity(avail, style.row_h), total_rows=len(rows))
    return st, (PREVIEW_W, gi[3])


def _fill(c):
    return tuple(int(v * 255) for v in c) + (255,)


def render_page(vals, hover, outpath=None):
    """The Options page at the row values `vals` (row id -> choice index), row `hover` showing its description. -> (image, info)."""
    from PIL import Image
    cpm, qlm = isf._cpm(), isf._qlm()
    root, ctl = load_options(cpm)
    style = style_from_values(vals)
    img = Image.new("RGBA", (SCREEN_W, SCREEN_H), (0, 0, 0, 255))
    bg = cpm.texture(root.get("FILL"))
    if bg is not None:
        cpm.paste_stretched(img, cpm.tint(bg, root.get("COLOR", (1, 1, 1))), (0, 0, SCREEN_W, SCREEN_H))
    info = {"style": style, "label_trunc": [], "desc_lines": 0, "desc_cap": 0}
    for tag in ("LBL_BAR1", "LBL_BAR2", "LBL_BAR3"):
        b, r = ctl[tag]["BORDER"], isf.unit_px(cpm, ctl[tag]["RECT"])
        t = cpm.texture(b["FILL"])
        if t is not None:
            cpm.paste_stretched(img, cpm.tint(t, b["COLOR"]), (r[0], r[1], r[0] + r[2], r[1] + min(r[3], 16)))
    tr = isf.unit_px(cpm, ctl["LBL_TITLE"]["RECT"])
    isf.frame_px(img, cpm, ctl["LBL_TITLE"]["BORDER"], tr)
    cpm.draw_text(img, LARGE_FONT, "Inventory Sort & Filter", (tr[0], tr[1], tr[0] + tr[2], tr[1] + tr[3]), 18, isf.CREAM)
    isf.frame_px(img, cpm, ctl["LBL_BAR4"]["BORDER"], isf.unit_px(cpm, ctl["LBL_BAR4"]["RECT"]))
    br = isf.unit_px(cpm, ctl["BTN_BACK"]["RECT"])
    isf.frame_px(img, cpm, ctl["BTN_BACK"]["BORDER"], br)
    cpm.draw_text(img, LARGE_FONT, "Back", (br[0], br[1], br[0] + br[2], br[1] + br[3]), 18, isf.TEAL_TEXT)
    # rows: MenuUiLogic.h rowSlotRect / rowGeom
    col = isf.unit_px(cpm, COLUMN_UNITS)
    pitch, slot_h = col[3] * ROW_PITCH_REF // ROW_REF_H, col[3] * ROW_SLOT_REF // ROW_REF_H
    aw = col[2] * ROW_ARROW_PCT // 100
    lw = col[2] - 2 * aw - 2 * ROW_GAP
    arrows = {"L": cpm.texture("uibit_arrowneg_p"), "R": cpm.texture("uibit_arrowpos_p")}
    asz = (int(16 * cpm.SX), int(28 * cpm.SY))
    for k, row in enumerate(PAGE_ROWS):
        y = col[1] + k * pitch
        lab = (col[0], y, lw, slot_h)
        cells = {"L": (col[0] + lw + ROW_GAP, y, aw, slot_h), "R": (col[0] + lw + ROW_GAP + aw + ROW_GAP, y, aw, slot_h)}
        hot = row["id"] == hover
        brd = isf.BTN_BORDER_ACTIVE if hot else isf.BTN_BORDER
        txt = "%s: %s" % (row["label"], row["choices"][vals[row["id"]]])
        shown = cpm.fit(LARGE_FONT, txt, lab[2] - 2 * 16 - 8)
        if shown != txt:
            info["label_trunc"].append(txt)
        isf.frame_px(img, cpm, brd, lab)
        cpm.draw_text(img, LARGE_FONT, shown, (lab[0] + 16, lab[1], lab[0] + lab[2] - 16, lab[1] + lab[3]), 18, isf.CREAM if hot else isf.TEAL_TEXT)
        for key, cell in cells.items():
            isf.frame_px(img, cpm, brd, cell)
            if arrows[key] is not None:
                cpm.paste_stretched(img, cpm.tint(arrows[key], isf.TEAL_TEXT if not hot else isf.CREAM),
                                    (cell[0] + (cell[2] - asz[0]) // 2, cell[1] + (cell[3] - asz[1]) // 2,
                                     cell[0] + (cell[2] + asz[0]) // 2, cell[1] + (cell[3] + asz[1]) // 2))
    # LB_DESC: preview (top 85 %) + description (bottom 15 %)
    pane = isf.unit_px(cpm, ctl["LB_DESC"]["RECT"])
    ph = pane[3] * DESC_SPLIT_PERMILLE // 1000
    prev, desc = (pane[0], pane[1], pane[2], ph), (pane[0], pane[1] + ph, pane[2], pane[3] - ph)
    for r in (prev, desc):
        isf.frame_px(img, cpm, ctl["LB_DESC"]["BORDER"], r)
    G = geometry(cpm)
    gw = PREVIEW_W
    gx = prev[0] + max(0, (prev[2] - gw) // 2)
    gy = prev[1] + 22
    st, (w, h) = draw_preview(img, cpm, qlm, style, gx, gy)
    info.update(preview=st, preview_group=(gx, gy, w, h), preview_pane=prev, desc_pane=desc, group_w=gw)
    cap_y = gy + h + 6
    cpm.draw_text(img, SMALL_FONT, "Preview: %d of %d item rows visible" % (st["rows"], st["total_rows"]),
                  (prev[0] + 24, cap_y, prev[0] + prev[2] - 24, cap_y + 34), 9, isf.TEAL_TEXT)
    info["preview_bottom"] = cap_y + 34 + 16
    x0, y0, x1 = desc[0] + 16 + 8, desc[1] + 16 + 8, desc[0] + desc[2] - 16 - 8
    lh = 12.5 * cpm.tscale(SMALL_FONT)
    row = next(r for r in PAGE_ROWS if r["id"] == hover)
    lines = cpm.wrap(row["desc"], SMALL_FONT, x1 - x0)
    info["desc_lines"], info["desc_cap"] = len(lines), int((desc[1] + desc[3] - 16 - 8 - y0) // lh)
    for i, ln in enumerate(lines):
        cpm.draw_text(img, SMALL_FONT, ln, (x0, y0 + i * lh, x1, y0 + (i + 1) * lh), 9, DESC_TEXT)
    if outpath:
        img.save(outpath)
    return img, info


def render_strip(styles, outpath=None):
    """preview_strip.png: the preview list for each style in a 3-column grid, labelled with the values."""
    from PIL import Image
    cpm, qlm = isf._cpm(), isf._qlm()
    G = geometry(cpm)
    gw = PREVIEW_W
    gh = G["geo"]["items"][3]
    pad, head = 40, 3 * 44
    cols = 3
    cw, ch = gw + 2 * pad, head + gh + 2 * pad
    nrows = (len(styles) + cols - 1) // cols
    img = Image.new("RGBA", (cols * cw, nrows * ch), (6, 10, 12, 255))
    stats = []
    lh = int(12.5 * cpm.tscale(SMALL_FONT))
    for i, s in enumerate(styles):
        ox, oy = (i % cols) * cw, (i // cols) * ch
        st, _ = draw_preview(img, cpm, qlm, s, ox + pad, oy + pad + head)
        lines = ["%d. Rows: %s (%d px) | Icon: %s (%d px)" % (i + 1, s.row, s.row_h, s.icon, s.icon_px),
                 "Font: %s (%s) | Dividers: %s" % (s.font, s.font_name[-5:], s.dividers),
                 "%d rows visible in this window (%d fit, no dividers)" % (st["rows"], st["capacity"])]
        for j, ln in enumerate(lines):
            cpm.draw_text(img, SMALL_FONT, ln, (ox + pad, oy + pad + j * lh, ox + cw - pad, oy + pad + (j + 1) * lh), 9, isf.CREAM if j == 0 else isf.TEAL_TEXT)
        stats.append(st)
    if outpath:
        img.save(outpath)
    return img, stats, (cw, ch)


def render_inventory(style, outpath=None):
    """The v4b inventory screen with this list style: isf render with the row loop off, then the list is drawn here."""
    from PIL import ImageDraw
    cpm, qlm = isf._cpm(), isf._qlm()
    be, df = isf.build_elements, qlm.draw_fill

    def draw_fill(img, c, name, *a, **k):
        if name != "uibit_fill_16g":          # the vanilla scroll thumb is redrawn below with the right size
            return df(img, c, name, *a, **k)
    isf.build_elements, qlm.draw_fill = (lambda rows, order, variant: []), draw_fill
    try:
        img, info = isf.render(isf.S_ALL, isf.SO_CATEGORY, variant="v4b")
    finally:
        isf.build_elements, qlm.draw_fill = be, df
    G = geometry(cpm)
    gi = G["geo"]["items"]
    items = isf.SAMPLE
    rows = isf.arrange(isf.S_ALL, isf.SO_CATEGORY)
    els = be(rows, isf.SO_CATEGORY, "v4b" if style.dividers_on else "current")
    avail = G["geo"]["avail"]
    st = draw_list(img, cpm, qlm, G["geo"]["ry0"], gi[0] + G["row_x_rel"], G["row_w"], avail, els, style, items, rows[0])
    draw_scrollbar(img, cpm, qlm, G["sb"], len(els), st["elems"])
    d = ImageDraw.Draw(img)
    d.rectangle([0, 1456, SCREEN_W, SCREEN_H], fill=(0, 0, 0, 255))
    lh = 12.5 * cpm.tscale(SMALL_FONT)
    cpm.draw_text(img, SMALL_FONT, "ISF list-style mock (v4b screen) | %s | %d of %d item rows visible (%d fit without dividers) | Category sort, filter All" % (
        style.label(), st["rows"], len(rows), rows_capacity(avail, style.row_h)), (40, 1460, SCREEN_W - 40, 1460 + lh), 9, isf.CREAM)
    st.update(capacity=rows_capacity(avail, style.row_h), avail=avail)
    if outpath:
        img.save(outpath)
    return img, st


def fit_table(cpm=None):
    """Text table: per list-style combination the rows visible in the real LB_ITEMS (v4b), name px vs text box px, FAIL flags. -> (text, n_fail)."""
    cpm = cpm or isf._cpm()
    G = geometry(cpm)
    avail = G["geo"]["avail"]
    row_w = G["row_w"]
    names = [i.name for i in isf.SAMPLE]
    small_h = cpm.text_size(SMALL_FONT, "A")[1]
    out = ["ISF list-style fit table | 3840x1600 | real LB_ITEMS (v4b) items rect %s, window %d px (first row at +%d px), row width %d px" % (
        G["geo"]["items"], avail, G["off"], row_w),
        "capacity = whole item rows that fit; cat+div / cat-div = item rows shown from the top of the real 22-item backpack in Category sort with / without dividers;",
        "name px = widest sample name in the list font; box px = row width - icon column - right pad; text h = cap height; FAIL if name > box, text h > row h - 8, or font < dialogfont10x10.", ""]
    hdr = "%-8s %-8s %-6s | %4s %4s | %7s %7s | %-14s | %5s %5s | %-5s" % ("rows", "icon", "font", "row", "icon", "capac", "cat+div", "cat-div", "name", "box", "text")
    out += [hdr + " | status", "-" * (len(hdr) + 12)]
    fails = 0
    for s in COMBOS:
        cap = rows_capacity(avail, s.row_h)
        _, r_on = elements_from_top(category_elements(s), s, avail)
        _, r_off = elements_from_top(category_elements(replace(s, dividers="Off")), s, avail)
        widest = max(names, key=lambda n: cpm.text_size(s.font_name, n)[0])
        w = cpm.text_size(s.font_name, widest)[0]
        box = row_w - (ICON_PAD + s.icon_px + TEXT_GAP) - NAME_RIGHT_PAD
        th = cpm.text_size(s.font_name, "A")[1]
        bad = []
        if w > box:
            bad.append("name '%s' %d > %d px" % (widest, w, box))
        if th > s.row_h - 8:
            bad.append("text h %d > row %d" % (th, s.row_h - 8))
        if th < small_h:
            bad.append("font below dialogfont10x10")
        fails += bool(bad)
        icon_note = "%d" % s.icon_px + ("*" if s.icon_px < ICON_REQ_PX[s.icon] else "")
        out.append("%-8s %-8s %-6s | %4d %4s | %7d %7d | %-14d | %5d %5d | %-5d | %s" % (s.row, s.icon, s.font, s.row_h, icon_note, cap, r_on, r_off, w, box, th,
                                                                                   "FAIL: " + "; ".join(bad) if bad else "ok"))
    out += ["", "* = icon clamped to row height - 2 * %d px (the requested size does not fit the row)." % ICON_PAD, ""]
    # page labels and descriptions
    col = isf.unit_px(cpm, COLUMN_UNITS)
    lw = col[2] - 2 * (col[2] * ROW_ARROW_PCT // 100) - 2 * ROW_GAP
    out.append("Page label fit (label cell %d px, inner %d px, dialogfont16x16): widest 'Label: Value' per row" % (lw, lw - 2 * 16 - 8))
    for r in PAGE_ROWS:
        txt = max(("%s: %s" % (r["label"], c) for c in r["choices"]), key=lambda t: cpm.text_size(LARGE_FONT, t)[0])
        w = cpm.text_size(LARGE_FONT, txt)[0]
        ok = w <= lw - 2 * 16 - 8
        fails += not ok
        out.append("  %-26s %5d px  %s" % (txt, w, "ok" if ok else "FAIL: cut with '...'"))
    return "\n".join(out) + "\n", fails


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    out = Path(argv[1])
    out.mkdir(parents=True, exist_ok=True)
    base = default_values()

    def vals(**kw):
        v = dict(base)
        for k, name in kw.items():
            row = next(r for r in PAGE_ROWS if r["id"] == k)
            v[k] = row["choices"].index(name)
        return v
    pages = [("page_default", base, "row_height"),
             ("page_compact", vals(row_height="Compact", icon_size="Small", list_font="Small", dividers="On"), "icon_size"),
             ("page_large", vals(row_height="Large", icon_size="Large", list_font="Large"), "list_font"),
             ("page_dividers_off", vals(dividers="Off"), "dividers")]
    report = []
    for name, v, hover in pages:
        _, info = render_page(v, hover, out / (name + ".png"))
        pv = info["preview"]
        report.append("%s: %s | preview %d rows visible, %d fit | label cuts %d | desc lines %d/%d | preview bottom %d vs pane bottom %d" % (
            name, info["style"].label(), pv["rows"], pv["capacity"], len(info["label_trunc"]), info["desc_lines"], info["desc_cap"],
            info["preview_bottom"], info["preview_pane"][1] + info["preview_pane"][3]))
    _, stats, cell = render_strip(STRIP_STYLES, out / "preview_strip.png")
    report.append("preview_strip: cell %dx%d px; rows visible %s" % (cell[0], cell[1], [s["rows"] for s in stats]))
    for tag, s in (("inventory_compact", ListStyle("Compact", "Small", "Small", "On")), ("inventory_large", ListStyle("Large", "Large", "Large", "On"))):
        _, st = render_inventory(s, out / (tag + ".png"))
        report.append("%s: %s | %d rows visible, %d fit" % (tag, s.label(), st["rows"], st["capacity"]))
    text, fails = fit_table()
    (out / "fit_table.txt").write_text(text + "\n" + "\n".join(report) + "\n", encoding="utf-8")
    print("\n".join(report))
    print("fit table FAIL rows: %d" % fails)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
