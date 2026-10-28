#!/usr/bin/env python3
"""Offline render of the quick-loot HUD list block (doc 52 QL2L 0.4.0-dev, restyled in QL2V 0.5.0-dev): judge fit, scrolling, the vanilla look and the
row-height choice before play testing.

Reuses character_page_mockup.py (cpm): the dialogfont10x10 bitmap font (glyph boxes from the font TXI, advances from
_shared_src/FontWidths.inc, DLL-calibrated 2.75 px/texel at 3840x1600), the real draw_text() and the real draw_border() (the same call that
draws the vanilla tooltip at character_page_mockup.py ~:553). Reads the live install read-only through pykotor (textures, baseitems.2da, the
vanilla inventory_p gui); writes PNGs only to OUTDIR.

What the mod draws (patch_manager_mods/quick_loot_src/QuickLoot.cpp blockDraw + QuickLootLogic.h), all engine px (guikit::setRect writes engine px;
3840x1600 on the dev box; NOT 800x600 gui units):
  one BLOCK = frame panel + header row + up to ListRows item rows (default 8, 1..12) + a scrollbar when the list has more rows than that.
  Frame: a label cloned from the HUD template LBL_MOULDING3 (the vanilla tooltip frame: FILL uibit_fill_2bt, EDGE uibit_brdr_16bet, CORNER
  uibit_brdr_16bct, DIMENSION 16 taken as px, tint TEAL_FRAME) -- the engine draws the edges and corners from the .gui, GuiKit only sets tint
  and alpha. The fill is black at alpha 0.78 (texture alpha 199/255), alpha 1.0.
  Rows (QL2V): each item is a vanilla protoitem button look: its own LBL_MOULDING3 clone (same bct / bet / 2bt, teal), ROW_H 64 px (56 / 72 are the
  alternatives rendered side by side), ROW_GAP 4 px between rows, the item icon (the real texture, "i" + baseitems itemclass + "_%03d" like 0x6D4E60) as
  a square at the left, the name after it (dialogfont10x10, teal, vertically centred); the hovered row = the vanilla HILIGHT (same textures, tint
  HILIGHT (0.929, 1, 0.929)) + cream text. Header cream, indicator dim teal at the right end of the header row.
  Scrollbar (QL2V, only when the list overflows): SB_W 36 px column right of the rows: teal outline (uibit_fill_16g darkened) under a dark inset
  (uibit_fill_2bt), thumb uibit_fill_16g sized visible/total and placed by the offset, up arrow uibit_uparrowg, down arrow lbl_mapdown (white, tinted
  teal; there is NO down-arrow texture in the uibit set and a label cannot rotate the up arrow), arrows dimmed at the ends. No HUD template carries
  the vanilla 16g scrollbar border, so these are fill-only labels (comment in QuickLoot.cpp).
  Text: align 9 (top-left) in a 32 px text box placed (ROW_H - 28) / 2 below the row top. Rows: name cut at 40 chars (first 37 + "..."), " xN" for
  stacks 2..999, cut further to the text budget; items past the 64 captured become one last "+K more" row (no icon). Block width = widest row (ALL
  rows, so scrolling never resizes it) + icon chrome + scrollbar reserve + 2 x 28 px pad, 300..1000 px.
  Placement: position A under the target name (x centred on the name, clamped to the panel), B at the fixed spot (centre of the panel, 62 %
  down); bottom clamp: A flips above the name, B moves up. The mod latches A or B when the list first shows.
Target menu mock (doc 39 R6 observed): name label local (0,2) w 533 h 32 (health bar local y 35 h 16).

The one-line formatters (fmtList / fmtTook / fmtPeek, the 0.3.1 fallback text) and the list layer (rowName / rowText / ListData / ListView /
buildListDraw / blockRect / sbLayout / sbHitAt) are ports of QuickLootLogic.h, checked against quick_loot_logic_test.cpp vectors in test_quick_loot_mockup.py.

usage: quick_loot_mockup.py OUTDIR
Outputs: ql2v_<id>_full.png (A and B on one frame), ql2v_<id>_zoom.png (crops of both blocks), ql2v_rowh_A.png / ql2v_rowh_B.png (RowH 56 / 64 / 72
side by side), ql2v_vs_vanilla.png (the mod block next to the vanilla inventory_p list rebuilt from its gui), ql2v_fit_table.txt
"""
import math
import os
import re
import struct
import sys
from pathlib import Path

# ---- constants mirrored from QuickLootLogic.h ----------------------------------------------------------------
LABEL_MAX_CHARS = 56
TOAST_MAX_CHARS = PEEK_MAX_CHARS = LABEL_MAX_CHARS
NAME_RAW_MAX = 40       # sanitizeName (log lines)
NAME_MAX = 20           # ToastNameMax: longer entries are cut to 17 chars + "..."
MAX_ITEMS = 16
LABEL_W = 1200
TOAST_W, TOAST_H = LABEL_W, 20
PEEK_W = LABEL_W
GAP_BELOW_NAME = 20
SCREEN_MARGIN = 24
BACK_PAD_X, BACK_PAD_Y = 10, 4
BACK_ALPHA = 0.55
PX_PER_CHAR_EST = 24
TOAST_MS, TOAST_LONG_MS, TOAST_CAP_MS = 3000, 3500, 4000
PEEK_TOP_PERCENT = 62
FONT = "dialogfont10x10"
ALIGN = 18
COLOR = (0.9, 0.85, 0.5)   # 0.3.1 one-line colour (the 0.4.0 block uses TEAL_TEXT / CREAM)

# screen + target-menu mock (engine px)
SCREEN_W, SCREEN_H = 3840, 1600
NAME_LOCAL = (0, 2, 533, 32)      # doc 39 R6 observed
BAR_LOCAL = (0, 35, 533, 16)
MENU_ORIGIN = (1700, 520)         # arbitrary target screen position for the mock


# ---- formatter ports -------------------------------------------------------------------------------------------
def sanitize_name(name):
    """QuickLootLogic.h sanitizeName: stop at NUL, cut at NAME_RAW_MAX, non-printable -> '?', trim trailing spaces, empty -> 'item'."""
    out = []
    if name is not None:
        for ch in name:
            if ch == "\0" or len(out) >= NAME_RAW_MAX:
                break
            out.append(ch if 0x20 <= ord(ch) <= 0x7E else "?")
    s = "".join(out).rstrip(" ")
    return s if s else "item"


def entry_name(name):
    """QuickLootLogic.h entryName: names over NAME_MAX are cut to NAME_MAX-3 chars, trailing space/punctuation trimmed, '...' appended."""
    s = sanitize_name(name)
    if len(s) > NAME_MAX:
        s = s[:NAME_MAX - 3]
        while s and not s[-1].isalnum():
            s = s[:-1]
        s += "..."
    return s


def fmt_list(prefix, max_chars, names, stacks, count, extra, cap=None):
    """detail::fmtList: '<prefix>A, B x2, C' with ', +K more' when the line would pass max_chars. count 0 -> ''."""
    if count == 0:
        return ""
    if count > MAX_ITEMS:
        extra += count - MAX_ITEMS
        count = MAX_ITEMS
    ent = []
    for i in range(count):
        e = entry_name(names[i] if names else None)
        st = stacks[i] if stacks else -1
        if 2 <= st <= 999:
            e += " x%d" % st
        ent.append(e)
    total = len(prefix) + sum(len(e) + (2 if i else 0) for i, e in enumerate(ent))
    shown, more = count, extra
    if not (extra == 0 and total <= max_chars):
        shown, used = 0, len(prefix)
        for i, e in enumerate(ent):
            add = len(e) + (2 if i else 0)
            after = count - i - 1 + extra
            suffix = len(", +%u more" % after) if after else 0
            if used + add + suffix > max_chars:
                break
            used += add
            shown = i + 1
        if shown == 0:
            shown = 1
        more = count - shown + extra
    s = prefix + ", ".join(ent[:shown])
    if more:
        s += ", +%u more" % more
    if cap is not None and len(s) >= cap:
        s = s[:cap - 1]
    return s


def fmt_took(names, stacks, count, extra=0, cap=None):
    """'Took (N): ...' (N = count + extra), 'Took: X' for a single item."""
    total = count + extra
    pre = "Took: " if total <= 1 else "Took (%u): " % total
    return fmt_list(pre, TOAST_MAX_CHARS, names, stacks, count, extra, cap)


def fmt_peek(key, total, names, stacks, count, cap=None):
    """'[F] Take all (N): ...', '[F] Take: X' for a single item; '' for an empty container."""
    if total <= 0 or count == 0:
        return ""
    k = key if key else "key"
    pre = "[%s] Take: " % k if total == 1 else "[%s] Take all (%d): " % (k, total)
    extra = total - count if total > count else 0
    return fmt_list(pre, PEEK_MAX_CHARS, names, stacks, count, extra, cap)


def toast_duration_ms(shown_names, more):
    """toastDurationMs: 3 s, 3.5 s for more than 3 names or '+K more', never above the 4 s cap."""
    d = TOAST_LONG_MS if (shown_names > 3 or more > 0) else TOAST_MS
    return min(d, TOAST_CAP_MS)


# ---- geometry --------------------------------------------------------------------------------------------------
def clamp_label_x(rect, screen_x, screen_w):
    """clampLabelX: x in [screen_x+24, screen_x+screen_w-24-w]; screen_w <= 0 = no clamp; too-wide labels are centred."""
    x, y, w, h = rect
    if screen_w <= 0:
        return rect
    lo, hi = screen_x + SCREEN_MARGIN, screen_x + screen_w - SCREEN_MARGIN - w
    if hi < lo:
        x = screen_x + (screen_w - w) // 2
    else:
        x = min(max(x, lo), hi)
    return (x, y, w, h)


def toast_rect(name_x, name_y, name_w, name_h, w=TOAST_W, screen=None):
    """Position A (toastRect / peekRectName): centred on the name label, top = name bottom + gap; x clamped to the screen (x, w)."""
    r = (name_x + name_w // 2 - w // 2, name_y + name_h + GAP_BELOW_NAME, w, TOAST_H)
    return clamp_label_x(r, *screen) if screen else r


def back_rect(rect, chars):
    """backingRect: width follows the text (chars x 24 + 2 pad, at most label + 2 pad), centred on the label; height label + 2 pad."""
    x, y, w, h = rect
    bw = min(chars * PX_PER_CHAR_EST + 2 * BACK_PAD_X, w + 2 * BACK_PAD_X)
    return (x + w // 2 - bw // 2, y - BACK_PAD_Y, bw, h + 2 * BACK_PAD_Y)


def fixed_rect(px, py, pw, ph, w=PEEK_W):
    """Position B (peekRectFixed): centred on the HUD panel, PEEK_TOP_PERCENT % down (integer division as C++), x clamped like A."""
    return clamp_label_x((px + pw // 2 - w // 2, py + (ph * PEEK_TOP_PERCENT) // 100, w, TOAST_H), px, pw)


def text_width(s, advances, scale):
    """Pixel width = sum of glyph texel advances * px per texel (cpm.text_size's width formula); non-Latin1 counts as '?'."""
    return sum(advances[ord(c) if ord(c) < 256 else 63] for c in s) * scale


# ---- list layer: ports of QuickLootLogic.h (QL2L 0.4.0-dev, restyled in QL2V 0.5.0-dev) ----------------------------------
NAME_CAP = 48
LIST_MAX_ITEMS = 64
LIST_ROWS_DEFAULT, LIST_ROWS_MIN, LIST_ROWS_MAX = 8, 1, 12
ROW_NAME_MAX = 40
RESREF_CAP = 17
ROW_H = 64                       # item row height px (QL2V; 0.4.0 was 32). Alternatives: ROW_H_ALTS
ROW_H_ALTS = (56, 72)
ROW_GAP = 4
HEADER_H = 32                    # header / indicator row (the 0.4.0 ROW_H)
HEADER_GAP = 6
ROW_ICON_PAD = 4
ICON_GAP = 12
ROW_TEXT_PAD_R = 16
TEXT_GLYPH_H = 28
TEXT_BOX_H = 32
SB_W = 36
SB_GAP = 8
SB_RESERVE = SB_W + SB_GAP
SB_INSET = 2
SB_THUMB_MIN = 24
BLOCK_PAD_X, BLOCK_PAD_Y = 28, 20
BLOCK_MIN_W, BLOCK_MAX_W = 300, 1000
LABEL_SLACK = 12
INDICATOR_GAP = 36
WHEEL_STEP = 120
TOOK_BASE_MS, TOOK_ROW_MS, TOOK_CAP_MS, TOOK_FREE_ROWS = 3000, 500, 6000, 3
FRAME_ALPHA = 1.0
FRAME_DIM = 16                   # template DIMENSION, taken as engine px
FRAME_TEMPLATE = {"FILL": "uibit_fill_2bt", "EDGE": "uibit_brdr_16bet", "CORNER": "uibit_brdr_16bct"}
FONT_TEXTK, FONT_SY = 1.03, 1600.0 / 600.0   # textfit::TextCtx for dialogfont10x10
MAX_BASE_ITEM_IDX = 1024
# vanilla palette (character_page_mockup.py:45-47, inventory_p.gui); the mod's constants in QuickLoot.cpp
TEAL_FRAME = (0.05098, 0.34902, 0.27059)
TEAL_TEXT = (0.10196, 0.69804, 0.54902)
CREAM = (0.8, 0.8, 0.69804)
TEAL_DIM = (0.0663, 0.4537, 0.3569)
ROW_HILIGHT = (0.92941, 1.0, 0.92941)       # inventory_p protoitem HILIGHT COLOR
SB_THUMB_TINT = (0.98039, 1.0, 0.98039)     # inventory_p SCROLLBAR BORDER COLOR
SB_OUTLINE_TINT = (0.6, 0.6, 0.6)
SB_INSET_TINT = (1.0, 1.0, 1.0)
ARROW_TINT = TEAL_TEXT
ALPHA_DIM = 0.4
TEXT_ALIGN = 9                   # top-left
TEX_FILL_16G, TEX_FILL_2BT, TEX_UP_ARROW, TEX_DOWN_ARROW = "uibit_fill_16g", "uibit_fill_2bt", "uibit_uparrowg", "lbl_mapdown"
ROW_TEXT_MAX_PX = BLOCK_MAX_W - 2 * BLOCK_PAD_X - (ROW_ICON_PAD + (ROW_H_ALTS[1] - 2 * ROW_ICON_PAD) + ICON_GAP + ROW_TEXT_PAD_R) - SB_RESERVE
ROW_INNER_MAX_PX = ROW_TEXT_MAX_PX   # old name

_FONT10 = None


def f32(x):
    """Round to float32 (the C++ textPx runs in float)."""
    return struct.unpack("f", struct.pack("f", x))[0]


def font10_widths():
    """kFont10TexW parsed from _shared_src/FontWidths.inc (stdlib only)."""
    global _FONT10
    if _FONT10 is None:
        inc = (Path(__file__).resolve().parent.parent / "patch_manager_mods/_shared_src/FontWidths.inc").read_text()
        body = inc[inc.index("kFont10TexW"):]
        body = body[body.index("{") + 1:body.index("}")]
        _FONT10 = [int(v) for v in re.findall(r"\d+", body)]
        assert len(_FONT10) == 256
    return _FONT10


def text_px10(s):
    """textPx10: textfit::textPx(F10) = float(sum of texel advances) * sy * textK10 * textScale, rounded up."""
    tw = font10_widths()
    w = float(sum(tw[ord(c) if ord(c) < 256 else 63] for c in s))
    return int(math.ceil(f32(f32(f32(w * f32(FONT_SY)) * f32(FONT_TEXTK)) * 1.0)))


def clamp_list_rows(v):
    return max(LIST_ROWS_MIN, min(LIST_ROWS_MAX, v))


def took_duration_ms(list_rows):
    d = TOOK_BASE_MS + (list_rows - TOOK_FREE_ROWS) * TOOK_ROW_MS if list_rows > TOOK_FREE_ROWS else TOOK_BASE_MS
    return min(d, TOOK_CAP_MS)


def sanitize_resref(raw):
    """sanitizeResref: the engine's 16-byte, non-NUL-terminated CResRef -> text: stop at NUL, bytes outside 0x20..0x7E -> '?', trailing spaces trimmed."""
    out = []
    for ch in (raw or "")[:16]:
        if ch == "\0":
            break
        out.append(ch if 0x20 <= ord(ch) <= 0x7E else "?")
    return "".join(out).rstrip(" ")


def base_item_idx_ok(idx):
    return 0 <= idx < MAX_BASE_ITEM_IDX


def row_name(name):
    """rowName: printable bytes (others '?') up to NAME_CAP-1, trailing spaces trimmed, empty -> 'item'; > 40 chars -> first 37 (trailing
    non-alnum trimmed) + '...'."""
    t = []
    if name is not None:
        for ch in name:
            if ch == "\0" or len(t) >= NAME_CAP - 1:
                break
            t.append(ch if 0x20 <= ord(ch) <= 0x7E else "?")
    s = "".join(t).rstrip(" ")
    if not s:
        s = "item"
    if len(s) > ROW_NAME_MAX:
        s = s[:ROW_NAME_MAX - 3]
        while s and not (s[-1].isascii() and s[-1].isalnum()):
            s = s[:-1]
        s += "..."
    return s


def row_text(name, stack, max_px=ROW_TEXT_MAX_PX):
    """rowText: name + ' xN' (2..999); the name is cut further (+ '...') while the text is wider than max_px."""
    nm = row_name(name)
    sfx = " x%d" % stack if 2 <= stack <= 999 else ""
    buf = nm + sfx
    if text_px10(buf) > max_px:
        n = len(nm)
        if n >= 3 and nm.endswith("..."):
            n -= 3
        while n > 1:
            n -= 1
            t = nm[:n] + "..." + sfx
            if text_px10(t) <= max_px:
                buf = t
                break
    return buf


class ListData:
    """ListData: items = captured rows, more = items past the capture (one last '+K more' row); icons = resref per item (QL2V)."""

    def __init__(self, names, stacks, total=None, icons=None):
        count = min(len(names), LIST_MAX_ITEMS)
        self.rows = [row_text(names[i], stacks[i] if stacks else -1) for i in range(count)]
        self.icons = [sanitize_resref(icons[i]) if icons else "" for i in range(count)]
        self.items = count
        t = len(names) if total is None else total
        self.more = t - count if (count and t > count) else 0
        self.max_row_px = max([text_px10(r) for r in self.rows] + ([text_px10(self.more_text())] if self.more else []) + [0])

    def more_text(self):
        return "+%u more" % self.more

    def rows_total(self):
        return self.items + (1 if self.more else 0)

    def total_items(self):
        return self.items + self.more

    def row_at(self, i):
        if i < self.items:
            return self.rows[i]
        return self.more_text() if (i == self.items and self.more) else ""

    def icon_at(self, i):
        return self.icons[i] if i < self.items else ""


class ListView:
    """ListView: rows visible, total list rows, off first visible row; the wheel moves WHEEL_STEP per row (+ = up), step() the scrollbar."""

    def __init__(self, total, rows=LIST_ROWS_DEFAULT):
        self.rows, self.total, self.off, self.acc = rows, total, 0, 0

    def max_off(self):
        return self.total - self.rows if self.total > self.rows else 0

    def scrollable(self):
        return self.total > self.rows

    def scroll(self, delta):
        if not self.scrollable():
            self.acc = 0
            return False
        self.acc += delta
        steps = int(self.acc / WHEEL_STEP)
        self.acc -= steps * WHEEL_STEP
        n = self.off - steps
        if n < 0:
            n, self.acc = 0, 0
        if n > self.max_off():
            n, self.acc = self.max_off(), 0
        ch = n != self.off
        self.off = n
        return ch

    def step(self, rows):
        """Scrollbar click: first visible row += rows (+ = down), clamped. True when off changed."""
        self.acc = 0
        if not self.scrollable():
            return False
        n = max(0, min(self.max_off(), self.off + rows))
        ch = n != self.off
        self.off = n
        return ch


# scrollbar parts
SB_NONE, SB_UP, SB_DOWN, SB_PAGE_UP, SB_PAGE_DOWN, SB_THUMB = range(6)


def sb_step_for(part, page_rows):
    return {SB_UP: -1, SB_DOWN: 1, SB_PAGE_UP: -page_rows, SB_PAGE_DOWN: page_rows}.get(part, 0)


def icon_px(row_h=ROW_H):
    return row_h - 2 * ROW_ICON_PAD


def row_chrome_px(row_h=ROW_H):
    return ROW_ICON_PAD + icon_px(row_h) + ICON_GAP + ROW_TEXT_PAD_R


def rows_top_px():
    return BLOCK_PAD_Y + HEADER_H + HEADER_GAP


def list_height_px(vis, row_h=ROW_H):
    return vis * row_h + (vis - 1) * ROW_GAP if vis else 0


def block_height_px(vis, row_h=ROW_H):
    return 2 * BLOCK_PAD_Y + HEADER_H + HEADER_GAP + list_height_px(vis, row_h)


def sb_fits(vis, row_h=ROW_H):
    return list_height_px(vis, row_h) >= 2 * SB_W + SB_THUMB_MIN


def sb_layout(w, vis, total, off, row_h=ROW_H):
    """sbLayout -> dict(shown, col, up, down, track, thumb) block-relative rects, or {'shown': False}."""
    if vis == 0 or total <= vis or not sb_fits(vis, row_h):
        return {"shown": False}
    list_h = list_height_px(vis, row_h)
    col = (w - BLOCK_PAD_X - SB_W, rows_top_px(), SB_W, list_h)
    up = (col[0], col[1], SB_W, SB_W)
    down = (col[0], col[1] + list_h - SB_W, SB_W, SB_W)
    track = (col[0], col[1] + SB_W, SB_W, list_h - 2 * SB_W)
    max_off = total - vis
    off = min(off, max_off)
    th = (track[3] * vis) // total
    th = min(max(th, SB_THUMB_MIN), track[3])
    ty = track[1] + ((track[3] - th) * off) // max_off
    return {"shown": True, "col": col, "up": up, "down": down, "track": track, "thumb": (track[0] + 4, ty, SB_W - 8, th)}


def _in(r, x, y):
    return r[0] <= x < r[0] + r[2] and r[1] <= y < r[1] + r[3]


def sb_hit_at(sb, rx, ry):
    """sbHitAt: the scrollbar part under the block-relative point."""
    if not sb["shown"]:
        return SB_NONE
    if _in(sb["up"], rx, ry):
        return SB_UP
    if _in(sb["down"], rx, ry):
        return SB_DOWN
    if not _in(sb["track"], rx, ry):
        return SB_NONE
    t = sb["thumb"]
    if ry < t[1]:
        return SB_PAGE_UP
    if ry >= t[1] + t[3]:
        return SB_PAGE_DOWN
    return SB_THUMB


def build_list_draw(d, v, took, key="F", row_h=ROW_H):
    """buildListDraw -> dict(header, indicator, rows, icons, vis, off, w, h, scrollable, sb, row_h, ...) or None for an empty list."""
    if d.items == 0:
        return None
    total = d.rows_total()
    rows = max(LIST_ROWS_MIN, min(LIST_ROWS_MAX, v.rows))
    vis = min(total, rows)
    max_off = total - rows if total > rows else 0
    off = min(v.off, max_off)
    header = "Took (%u)" % d.total_items() if took else "[%s] Take all (%u)" % (key, d.total_items())
    o = dict(took=took, off=off, vis=vis, rows_total=total, items=d.total_items(), scrollable=total > rows, header=header, row_h=row_h,
             header_px=text_px10(header), indicator="", indicator_px=0)
    o["sb"] = o["scrollable"] and sb_fits(vis, row_h)
    need = d.max_row_px + row_chrome_px(row_h) + (SB_RESERVE if o["sb"] else 0)
    if o["scrollable"]:
        last = d.total_items() if off + vis > d.items else off + vis
        o["indicator"] = "%u-%u of %u" % (off + 1, last, d.total_items())
        o["indicator_px"] = text_px10(o["indicator"])
        worst = "%u-%u of %u" % (max_off + 1, d.total_items(), d.total_items())
        need = max(need, o["header_px"] + INDICATOR_GAP + text_px10(worst))
    else:
        need = max(need, o["header_px"])
    o["w"] = max(BLOCK_MIN_W, min(BLOCK_MAX_W, need + 2 * BLOCK_PAD_X))
    o["h"] = block_height_px(vis, row_h)
    o["rows"] = [d.row_at(off + i) for i in range(vis)]
    o["icons"] = [d.icon_at(off + i) for i in range(vis)]
    return o


def choose_anchor(menu_shown, have_name, name, have_panel, panel):
    """chooseAnchor -> (pos, cx, top, name_top, has_name) or None."""
    panel_ok = have_panel and panel[2] > 0 and panel[3] > 0
    if menu_shown or (not panel_ok and have_name):
        return ("A", name[0] + name[2] // 2, name[1] + name[3] + GAP_BELOW_NAME, name[1], True)
    if panel_ok:
        return ("B", panel[0] + panel[2] // 2, panel[1] + (panel[3] * PEEK_TOP_PERCENT) // 100, 0, False)
    return None


def block_rect(anchor, w, h, panel):
    """blockRect: x clamped to the panel margins, bottom clamp (A flips above the name, B moves up), top limit panel.y + 24."""
    pos, cx, top, name_top, has_name = anchor
    r = (cx - w // 2, top, w, h)
    if panel[2] <= 0 or panel[3] <= 0:
        return r
    r = clamp_label_x(r, panel[0], panel[2])
    bottom, top_limit = panel[1] + panel[3] - SCREEN_MARGIN, panel[1] + SCREEN_MARGIN
    x, y, w, h = r
    if y + h > bottom:
        ny = bottom - h
        if pos == "A" and has_name:
            flip = name_top - GAP_BELOW_NAME - h
            if flip >= top_limit:
                ny = flip
        y = ny
    if y < top_limit:
        y = top_limit
    return (x, y, w, h)


def header_rect(w):
    return (BLOCK_PAD_X, BLOCK_PAD_Y, w - 2 * BLOCK_PAD_X + LABEL_SLACK, HEADER_H)


def row_rect(i, w, sb=False, row_h=ROW_H):
    return (BLOCK_PAD_X, rows_top_px() + i * (row_h + ROW_GAP), w - 2 * BLOCK_PAD_X - (SB_RESERVE if sb else 0), row_h)


def row_icon_rect(r, row_h=ROW_H):
    return (r[0] + ROW_ICON_PAD, r[1] + ROW_ICON_PAD, icon_px(row_h), icon_px(row_h))


def row_text_rect(r, row_h=ROW_H):
    x = r[0] + ROW_ICON_PAD + icon_px(row_h) + ICON_GAP
    return (x, r[1] + (row_h - TEXT_GLYPH_H) // 2, r[0] + r[2] - ROW_TEXT_PAD_R - x + LABEL_SLACK, TEXT_BOX_H)


def indicator_rect(w, ind_px):
    return (w - BLOCK_PAD_X - ind_px, BLOCK_PAD_Y, ind_px + LABEL_SLACK, HEADER_H)


def row_at_point(blk, vis, px, py, sb=False, row_h=ROW_H):
    """rowAtPoint: row index under the screen point, -1 for the header, the padding, a gap between rows, the scrollbar or outside."""
    x0, x1 = blk[0] + BLOCK_PAD_X, blk[0] + blk[2] - BLOCK_PAD_X - (SB_RESERVE if sb else 0)
    y0 = blk[1] + rows_top_px()
    if px < x0 or px >= x1 or py < y0:
        return -1
    pitch = row_h + ROW_GAP
    i = int((py - y0) / pitch)
    if (py - y0) - i * pitch >= row_h:
        return -1
    return i if i < vis else -1


# ---- sample items with real icons -------------------------------------------------------------------------------------
# (display name, stack, baseitems.2da row, ModelVariation): the icon resref = "i" + itemclass + "_%03d" (lowercase), mirroring 0x6D4E60's "i%s_%03d".
SAMPLE_ITEMS = [("Medpac", 2, 55, 1), ("Blaster Pistol", 1, 12, 1), ("Adhesive Grenade", 3, 31, 1), ("Lightsaber Crystal: Opila", 1, 11, 1),
                ("Frag Grenade", 1, 25, 1), ("Credit Chip", 1, 57, 1), ("Vibroblade", 1, 3, 1), ("Combat Suit", 1, 38, 1), ("Stimulant", 1, 53, 1),
                ("Glow Rod", 1, 61, 1), ("Repair Kit", 5, 56, 1), ("Security Spike", 1, 59, 1), ("Power Cell", 1, 63, 1), ("Ion Blaster", 1, 15, 1),
                ("Datapad", 1, 52, 1), ("Cure Pack", 1, 55, 2), ("Mandalorian Heavy Armor Mk IV", 1, 39, 1), ("Jedi Robe", 1, 35, 1),
                ("Energy Shield", 1, 75, 1), ("Comlink", 1, 72, 1), ("Targeting Scope", 1, 69, 1), ("Stun Baton", 1, 1, 1), ("Bacta Tank", 12, 55, 3),
                ("Ancient Sith Holocron Fragment", 1, 64, 1), ("Droid Probe", 1, 70, 1)]
REALISTIC = [n for n, _, _, _ in SAMPLE_ITEMS]
REALISTIC_STACKS = [st for _, st, _, _ in SAMPLE_ITEMS]
LONG_NAMES = ["Lightsaber Crystal: Opila", "Adhesive Grenade", "Mandalorian Heavy Armor Mk IV",
              "Ancient Sith Holocron Fragment of the Old Republic Era", "Medpac"]
LONG_STACKS = [1, 3, 1, 1, 999]
_BASEITEMS = None


def baseitem_itemclass(row):
    """baseitems.2da itemclass of a row (needs pykotor + the install); None when unavailable."""
    global _BASEITEMS
    if _BASEITEMS is None:
        try:
            cpm = _cpm()
            from pykotor.resource.formats.twoda import read_2da
            from pykotor.resource.type import ResourceType
            _BASEITEMS = read_2da(cpm.inst().resource("baseitems", ResourceType.TwoDA).data)
        except Exception:
            _BASEITEMS = False
    if not _BASEITEMS:
        return None
    return _BASEITEMS.get_row(row).get_string("itemclass")


def icon_resref(itemclass, variation):
    """0x6D4E60: sprintf("i%s_%03d", itemclass, variation), the engine lowercases on lookup."""
    return ("i%s_%03d" % (itemclass, variation)).lower()


def sample_icons():
    """Icon resref per SAMPLE_ITEMS entry (no install -> '' for every item = empty icon boxes)."""
    out = []
    for _, _, row, var in SAMPLE_ITEMS:
        ic = baseitem_itemclass(row)
        out.append(icon_resref(ic, var) if ic else "")
    return out


def numbered(n):
    return ["Salvage Part %d" % (i + 1) for i in range(n)]


def samples():
    """[(id, desc, took, ListData, scroll_off, hover_row, rows)] -- scroll_off in list rows."""
    ics = sample_icons()
    r25 = ListData(REALISTIC, REALISTIC_STACKS, icons=ics)
    r70 = ListData(numbered(64), None, total=70, icons=[ics[i % len(ics)] for i in range(64)])
    return [
        (1, "Peek, 4 items (no scrollbar)", False, ListData(REALISTIC[:4], REALISTIC_STACKS[:4], icons=ics[:4]), 0, None, 8),
        (2, "Peek, 25 items, off 0, row 2 hovered", False, r25, 0, 1, 8),
        (3, "Peek, 25 items scrolled to off 10, row 3 hovered", False, r25, 10, 2, 8),
        (4, "Peek, 70 items (64 captured) scrolled to the end", False, r70, 99, None, 8),
        (5, "Took, 6 items", True, ListData(REALISTIC[:6], REALISTIC_STACKS[:6], icons=ics[:6]), 0, None, 8),
        (6, "Peek, 1 item", False, ListData(["Medpac"], None, icons=ics[:1]), 0, None, 8),
        (7, "Peek, 8 items (fills the list, no scrollbar)", False, ListData(REALISTIC[:8], REALISTIC_STACKS[:8], icons=ics[:8]), 0, 1, 8),
        (8, "Peek, long names (cut at 40 chars, stacks)", False, ListData(LONG_NAMES, LONG_STACKS, icons=ics[:5]), 0, None, 8),
        (9, "Peek, 25 items, ListRows 12", False, r25, 0, None, 12),
        (10, "Peek, 25 items, ListRows 4", False, r25, 0, 0, 4),
    ]


def sample_draw(sample, row_h=ROW_H):
    sid, desc, took, d, off, hover, rows = sample
    v = ListView(d.rows_total(), rows)
    v.off = min(off, v.max_off())
    return build_list_draw(d, v, took, row_h=row_h), v


PANEL = (0, 0, SCREEN_W, SCREEN_H)


def placements(o, name_origin=MENU_ORIGIN):
    """([(label, rect)] for position A (target name at name_origin) and B (fixed spot), name rect)."""
    name = (name_origin[0] + NAME_LOCAL[0], name_origin[1] + NAME_LOCAL[1], NAME_LOCAL[2], NAME_LOCAL[3])
    a = choose_anchor(True, True, name, True, PANEL)
    b = choose_anchor(False, True, name, True, PANEL)
    return [("A", block_rect(a, o["w"], o["h"], PANEL)), ("B", block_rect(b, o["w"], o["h"], PANEL))], name


def fit_row(sample, row_h=ROW_H):
    """Fit-table row: every row text fits its text box, header + gap + indicator fit, the scrollbar sits inside the block, block inside the panel."""
    sid, desc, took, d, off, hover, rows = sample
    o, v = sample_draw(sample, row_h)
    inner = o["w"] - 2 * BLOCK_PAD_X
    text_budget = inner - (SB_RESERVE if o["sb"] else 0) - row_chrome_px(row_h)
    rows_fit = all(text_px10(d.row_at(i)) <= text_budget for i in range(d.rows_total()))
    head_fit = o["header_px"] + (INDICATOR_GAP + o["indicator_px"] if o["scrollable"] else 0) <= inner
    sb = sb_layout(o["w"], o["vis"], o["rows_total"], o["off"], row_h)
    sb_inside = (not sb["shown"]) or (sb["col"][0] + SB_W <= o["w"] - BLOCK_PAD_X and sb["col"][1] + sb["col"][3] <= o["h"] - BLOCK_PAD_Y)
    (pa, ra), (pb, rb) = placements(o)[0]
    inside = all(r[0] >= SCREEN_MARGIN and r[1] >= SCREEN_MARGIN and r[0] + r[2] <= SCREEN_W - SCREEN_MARGIN and r[1] + r[3] <= SCREEN_H - SCREEN_MARGIN
                 for r in (ra, rb))
    return dict(id=sid, desc=desc, kind="took" if took else "peek", items=d.total_items(), list_rows=d.rows_total(), rows=o["vis"], row_h=row_h,
                w=o["w"], h=o["h"], rows_fit=rows_fit, head_fit=head_fit, inside=inside, sb=o["sb"], sb_inside=sb_inside, a=ra, b=rb,
                header=o["header"], indicator=o["indicator"], dur=took_duration_ms(d.rows_total()) if took else 0)


# ---- rendering (needs PIL + pykotor + the live install) ----------------------------------------------------------
def _cpm():
    sys.path.insert(0, str(Path(__file__).parent))
    import character_page_mockup as cpm
    return cpm


def background():
    from PIL import Image, ImageDraw
    img = Image.new("RGBA", (SCREEN_W, SCREEN_H), (0, 0, 0, 255))
    d = ImageDraw.Draw(img)
    for y in range(SCREEN_H):                       # dark sky-to-ground gradient, world-like
        t = y / SCREEN_H
        c = (int(22 + 26 * (1 - t)), int(26 + 20 * (1 - t)), int(34 + 14 * (1 - t))) if t < 0.55 else \
            (int(40 - 14 * (t - 0.55)), int(36 - 12 * (t - 0.55)), int(30 - 10 * (t - 0.55)))
        d.line([(0, y), (SCREEN_W, y)], fill=c + (255,))
    return img


def draw_target_tag(img, cpm, origin):
    """Mock name label + health bar at the observed menu geometry. Returns the screen name rect (x, y, w, h)."""
    from PIL import ImageDraw
    d = ImageDraw.Draw(img)
    nx, ny, nw, nh = origin[0] + NAME_LOCAL[0], origin[1] + NAME_LOCAL[1], NAME_LOCAL[2], NAME_LOCAL[3]
    bx, by, bw, bh = origin[0] + BAR_LOCAL[0], origin[1] + BAR_LOCAL[1], BAR_LOCAL[2], BAR_LOCAL[3]
    cpm.draw_text(img, "dialogfont16x16", "Footlocker", (nx, ny, nx + nw, ny + nh), 18, (0.9, 0.9, 0.9))
    d.rectangle([bx, by, bx + bw - 1, by + bh - 1], fill=(20, 8, 8, 255), outline=(150, 150, 150, 255))
    d.rectangle([bx + 2, by + 2, bx + bw - 3, by + bh - 3], fill=(170, 30, 30, 255))
    return (nx, ny, nw, nh)


def draw_frame(img, cpm, rect, tint=TEAL_FRAME, dim=FRAME_DIM):
    """The vanilla frame through cpm.draw_border (the call that draws the vanilla tooltip), at 1 unit = 1 engine px so DIMENSION 16 is 16 px."""
    from PIL import Image
    x, y, w, h = rect
    layer = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    saved = (cpm.SX, cpm.SY)
    cpm.SX = cpm.SY = 1.0
    try:
        cpm.draw_border(layer, dict(FRAME_TEMPLATE, DIMENSION=dim, COLOR=tint), (0, 0, w, h))
    finally:
        cpm.SX, cpm.SY = saved
    if FRAME_ALPHA < 1.0:
        a = layer.getchannel("A").point(lambda v: int(v * FRAME_ALPHA))
        layer.putalpha(a)
    img.alpha_composite(layer, (x, y))


def draw_fill(img, cpm, resref, rect, tint, alpha=1.0):
    """A fill-only label: the texture stretched over rect, multiplied by tint, alpha scales the texture's own alpha (GuiKit setFill + setAlpha)."""
    t = cpm.texture(resref)
    x, y, w, h = rect
    if t is None or w < 1 or h < 1:
        return False
    t = cpm.tint(t, tint)
    if alpha < 1.0:
        t.putalpha(t.getchannel("A").point(lambda v: int(v * alpha)))
    cpm.paste_stretched(img, t, (x, y, x + w, y + h))
    return True


def draw_icon(img, cpm, resref, rect):
    """The item icon: the real texture stretched into the square; a missing texture leaves the box empty (the mod hides the label)."""
    return bool(resref) and draw_fill(img, cpm, resref, rect, (1.0, 1.0, 1.0))


def draw_scrollbar(img, cpm, sb, blk, off, max_off):
    """The scrollbar labels as the mod draws them: outline (clickable column), dark inset, thumb, arrows (dimmed at the ends)."""
    ab = lambda r: (blk[0] + r[0], blk[1] + r[1], r[2], r[3])
    col = ab(sb["col"])
    draw_fill(img, cpm, TEX_FILL_16G, col, SB_OUTLINE_TINT)
    draw_fill(img, cpm, TEX_FILL_2BT, (col[0] + SB_INSET, col[1] + SB_INSET, col[2] - 2 * SB_INSET, col[3] - 2 * SB_INSET), SB_INSET_TINT)
    draw_fill(img, cpm, TEX_FILL_16G, ab(sb["thumb"]), SB_THUMB_TINT)
    inset4 = lambda r: (r[0] + 4, r[1] + 4, r[2] - 8, r[3] - 8)
    draw_fill(img, cpm, TEX_UP_ARROW, inset4(ab(sb["up"])), (1.0, 1.0, 1.0), ALPHA_DIM if off == 0 else 1.0)
    draw_fill(img, cpm, TEX_DOWN_ARROW, inset4(ab(sb["down"])), ARROW_TINT, ALPHA_DIM if off >= max_off else 1.0)


def draw_block(img, cpm, o, rect, hover_row=None):
    """The whole block as the mod lays it out: frame, header (cream), indicator (dim teal), bordered rows (frame, icon, text; hovered row HILIGHT + cream),
    the scrollbar."""
    x, y, w, h = rect
    row_h = o["row_h"]
    draw_frame(img, cpm, rect)

    def put(text, r, col):
        rx, ry, rw, rh = r
        cpm.draw_text(img, FONT, text, (x + rx, y + ry, x + rx + rw, y + ry + rh), TEXT_ALIGN, col)

    put(o["header"], header_rect(w), CREAM)
    if o["indicator"]:
        put(o["indicator"], indicator_rect(w, o["indicator_px"]), TEAL_DIM)
    for i, t in enumerate(o["rows"]):
        rr = row_rect(i, w, o["sb"], row_h)
        ar = (x + rr[0], y + rr[1], rr[2], rr[3])
        hov = i == hover_row
        draw_frame(img, cpm, ar, ROW_HILIGHT if hov else TEAL_FRAME)
        draw_icon(img, cpm, o["icons"][i], row_icon_rect(ar, row_h))
        tr = row_text_rect(rr, row_h)
        put(t, tr, CREAM if hov else TEAL_TEXT)
    if o["sb"]:
        max_off = o["rows_total"] - o["vis"]
        draw_scrollbar(img, cpm, sb_layout(w, o["vis"], o["rows_total"], o["off"], row_h), rect, o["off"], max_off)


def caption(img, lines):
    from PIL import ImageDraw, ImageFont
    f = ImageFont.load_default(size=26)
    d = ImageDraw.Draw(img)
    y = 20
    for ln in lines:
        d.text((24, y), ln, fill=(235, 235, 235, 255), font=f, stroke_width=2, stroke_fill=(0, 0, 0, 255))
        y += 34


def zoom(img, rect, z=1, pad=40):
    from PIL import Image
    x, y, w, h = rect
    box = (max(0, x - pad), max(0, y - pad), min(SCREEN_W, x + w + pad), min(SCREEN_H, y + h + pad))
    c = img.crop(box)
    return c.resize((c.width * z, c.height * z), Image.LANCZOS) if z != 1 else c


# ---- vanilla inventory_p list, rebuilt from its gui (look comparison) ------------------------------------------------------------
def vanilla_inventory_list(img, cpm, origin):
    """Draws the vanilla inventory_p item list as the engine builds it from the gui file: the LB_ITEMS listbox frame (16wct / 16wet / fill 2wt),
    protoitem rows (EXTENT height 50 units, bordered 16bct / 16bet / 2bt teal, HILIGHT tint on the 2nd), the item icon square at each row's left and
    the name in dialogfont16x16, and the SCROLLBAR (16gc / 16ge track, uibit_uparrowg, uibit_fill_16g thumb). Geometry from the live Override
    inventory_p.gui (the ultrawide thirds layout: LB_ITEMS 48,134,236,329 units; protoitem 60,150,200,50), units -> px with cpm.SX / SY, border
    DIMENSION 16 taken as px (the same convention the tooltip frame uses). The icon square size is NOT in the gui (engine code), so it is approximated
    (row height minus 2 x 8 px, like the mod). Returns the listbox rect in px. None when the gui cannot be read."""
    from pykotor.resource.formats.gff import read_gff
    try:
        g = read_gff(str(next(pth for pth in (cpm.GAME / "override").iterdir() if pth.name.lower() == "inventory_p.gui")))
        lb = next(c for c in g.root.get_list("CONTROLS") if c.get_string("TAG") == "LB_ITEMS")
        e = lb.get_struct("EXTENT")
        L, T, W, H = (e.get_int32(k) for k in ("LEFT", "TOP", "WIDTH", "HEIGHT"))
    except Exception:
        return None
    sx, sy = cpm.SX, cpm.SY
    box = (int(L * sx) + origin[0], int(T * sy) + origin[1], int(W * sx), int(H * sy))
    lbx = dict(FRAME_TEMPLATE, FILL="uibit_fill_2wt", EDGE="uibit_brdr_16wet", CORNER="uibit_brdr_16wct")
    layer_img = img
    # listbox frame
    from PIL import Image
    layer = Image.new("RGBA", (box[2], box[3]), (0, 0, 0, 0))
    saved = (cpm.SX, cpm.SY)
    cpm.SX = cpm.SY = 1.0
    try:
        cpm.draw_border(layer, dict(lbx, DIMENSION=FRAME_DIM, COLOR=TEAL_FRAME), (0, 0, box[2], box[3]))
    finally:
        cpm.SX, cpm.SY = saved
    layer_img.alpha_composite(layer, (box[0], box[1]))
    row_w, row_h = int(200 * sx), int(50 * sy)
    rx = int(60 * sx) + origin[0]
    ry0 = int(150 * sy) + origin[1]
    names = ["Medpac", "Blaster Pistol", "Adhesive Grenade", "Lightsaber Crystal: Opila", "Frag Grenade", "Credit Chip"]
    ics = sample_icons()
    n_rows = min(len(names), (box[3] - 2 * FRAME_DIM) // row_h)
    for i in range(n_rows):
        r = (rx, ry0 + i * row_h, row_w, row_h)
        draw_frame(img, cpm, r, ROW_HILIGHT if i == 1 else TEAL_FRAME)
        draw_icon(img, cpm, ics[i], (r[0] + ROW_ICON_PAD, r[1] + ROW_ICON_PAD, row_h - 2 * ROW_ICON_PAD, row_h - 2 * ROW_ICON_PAD))
        cpm.draw_text(img, "dialogfont16x16", names[i], (r[0] + row_h + 6, r[1], r[0] + r[2] - 16, r[1] + r[3]), 17,
                      (0.8, 0.8, 0.69804) if i == 1 else TEAL_TEXT)
    # scrollbar (inventory_p.gui: x 268 in the thirds layout, 16 units wide, the listbox's right edge)
    sbx, sby, sbw, sbh = int(268 * sx) + origin[0], int(134 * sy) + origin[1], int(16 * sx), box[3]
    sbtex = dict(EDGE="uibit_brdr_16ge", CORNER="uibit_brdr_16gc", DIMENSION=4, COLOR=SB_THUMB_TINT)
    layer = Image.new("RGBA", (sbw, sbh), (0, 0, 0, 0))
    cpm.SX = cpm.SY = 1.0
    try:
        cpm.draw_border(layer, sbtex, (0, 0, sbw, sbh))
    finally:
        cpm.SX, cpm.SY = saved
    img.alpha_composite(layer, (sbx, sby))
    draw_fill(img, cpm, TEX_UP_ARROW, (sbx, sby, sbw, int(16 * sy)), SB_THUMB_TINT)
    draw_fill(img, cpm, TEX_FILL_16G, (sbx + 4, sby + int(40 * sy), sbw - 8, int(80 * sy)), SB_THUMB_TINT)
    return box


def render_vs_vanilla(outdir):
    """Side by side at 1:1 engine px: the mod's block (25 items, off 0, row 2 hovered, position B) next to the vanilla inventory_p list."""
    from PIL import Image
    cpm = _cpm()
    sample = samples()[1]
    o, v = sample_draw(sample)
    (pa, ra), (pb, rb) = placements(o)[0]
    ours = background()
    draw_block(ours, cpm, o, rb, sample[5])
    van = background()
    box = vanilla_inventory_list(van, cpm, (0, 0))
    if box is None:
        return None
    oc = zoom(ours, rb, pad=24)
    vc = van.crop((max(0, box[0] - 24), max(0, box[1] - 24), min(SCREEN_W, box[0] + box[2] + 24), min(SCREEN_H, box[1] + box[3] + 24)))
    sheet = Image.new("RGBA", (oc.width + vc.width + 30, max(oc.height, vc.height) + 70), (0, 0, 0, 255))
    sheet.alpha_composite(oc, (0, 70))
    sheet.alpha_composite(vc, (oc.width + 30, 70))
    caption(sheet, ["Left: quick-loot 0.5.0-dev block (RowH 64, position B). Right: vanilla inventory_p list from its gui (rows 50 units = 133 px tall).",
                    "Both at 1:1 engine px (3840x1600). Same border / hilight / scrollbar textures; the vanilla rows are taller, the mod's list is a compact HUD version."])
    p = Path(outdir) / "ql2v_vs_vanilla.png"
    sheet.save(p)
    return p


def render_rowh(outdir):
    """RowH 56 / 64 / 72 side by side (25 items, off 6, row 3 hovered), once at position A and once at B."""
    from PIL import Image, ImageDraw
    cpm = _cpm()
    out = []
    sample = samples()[2]
    for pos_i, label in ((0, "A"), (1, "B")):
        crops = []
        for rh in (ROW_H_ALTS[0], ROW_H, ROW_H_ALTS[1]):
            o, v = sample_draw(sample, rh)
            rect = placements(o)[0][pos_i][1]
            img = background()
            if pos_i == 0:
                draw_target_tag(img, cpm, MENU_ORIGIN)
            draw_block(img, cpm, o, rect, sample[5])
            crops.append((rh, o, zoom(img, rect, pad=24)))
        W = sum(c.width for _, _, c in crops) + 20 * (len(crops) - 1)
        Hh = max(c.height for _, _, c in crops) + 70
        sheet = Image.new("RGBA", (W, Hh), (0, 0, 0, 255))
        x = 0
        for rh, o, c in crops:
            sheet.alpha_composite(c, (x, 70))
            x += c.width + 20
        caption(sheet, ["Position %s, 25 items at off 10, row 3 hovered: RowH %s (blocks %s px tall; the mod ships RowH %d)." % (
            label, " / ".join(str(r) for r, _, _ in crops), " / ".join(str(o["h"]) for _, o, _ in crops), ROW_H)])
        p = Path(outdir) / ("ql2v_rowh_%s.png" % label)
        sheet.save(p)
        out.append(p)
    return out


def render_all(outdir):
    from PIL import Image
    cpm = _cpm()
    outdir = Path(outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    rows, written = [], []
    for sample in samples():
        sid, desc = sample[0], sample[1]
        o, v = sample_draw(sample)
        hover = sample[5]
        (pa, ra), (pb, rb) = placements(o)[0]
        img = background()
        draw_target_tag(img, cpm, MENU_ORIGIN)
        img_a, img_b = img.copy(), background()   # zoom crops come from one block per frame (A and B overlap on the combined frame)
        draw_block(img_a, cpm, o, ra, hover)
        draw_block(img_b, cpm, o, rb, hover)
        draw_block(img, cpm, o, ra, hover)
        draw_block(img, cpm, o, rb, hover)
        fr = fit_row(sample)
        rows.append(fr)
        caption(img, [
            f"#{sid} {desc}: {fr['items']} items, {fr['rows']} rows visible, block {fr['w']}x{fr['h']} px, header \"{fr['header']}\""
            + (f", indicator \"{fr['indicator']}\"" if fr["indicator"] else "") + (", scrollbar" if fr["sb"] else ", no scrollbar"),
            f"A: under the target name {ra[:2]}.   B: fixed spot {rb[:2]} (62% of the panel).   rows fit: {'yes' if fr['rows_fit'] else 'NO'}, "
            f"header+indicator fit: {'yes' if fr['head_fit'] else 'NO'}, inside the panel: {'yes' if fr['inside'] else 'NO'}",
            "Rows: vanilla protoitem look (uibit_brdr_16bct / 16bet + fill_2bt, teal; hover = HILIGHT tint + cream text), real item icons, vanilla scrollbar textures.",
        ])
        full = outdir / f"ql2v_{sid}_full.png"
        img.save(full)
        za, zb = zoom(img_a, ra), zoom(img_b, rb)
        sheet = Image.new("RGBA", (za.width + zb.width + 6, max(za.height, zb.height)), (0, 0, 0, 255))
        sheet.alpha_composite(za, (0, 0))
        sheet.alpha_composite(zb, (za.width + 6, 0))
        zp = outdir / f"ql2v_{sid}_zoom.png"
        sheet.save(zp)
        written += [full, zp]
    # bottom-of-screen case: the target is low on screen, A flips above the name (ListRows 12, 25 items)
    sample = samples()[8]
    o, v = sample_draw(sample)
    low = (1700, 1250)
    (pa, ra), (pb, rb) = placements(o, name_origin=low)[0]
    img = background()
    draw_target_tag(img, cpm, low)
    draw_block(img, cpm, o, ra)
    caption(img, [f"#11 Peek, 25 items, ListRows 12, target low on screen: A flips above the name (block {ra}, name top {low[1] + NAME_LOCAL[1]})"])
    p = outdir / "ql2v_11_full.png"
    img.save(p)
    written.append(p)
    fr = fit_row(sample)
    fr.update(id=11, desc="Peek, 25 items, ListRows 12, target low (A flips above)", a=ra, b=rb, inside=ra[1] + ra[3] <= SCREEN_H - SCREEN_MARGIN)
    rows.append(fr)
    # RowH variants in the fit table (25 items off 10): every variant must fit
    for k, rh in enumerate((ROW_H_ALTS[0], ROW_H_ALTS[1])):
        fr = fit_row(samples()[2], rh)
        fr.update(id=12 + k, desc="Peek, 25 items, off 10, RowH %d" % rh)
        rows.append(fr)
    written += render_rowh(outdir)
    vv = render_vs_vanilla(outdir)
    if vv:
        written.append(vv)
    table = format_table(rows)
    (outdir / "ql2v_fit_table.txt").write_text(table + "\n")
    return rows, table, written


def format_table(rows):
    lines = ["id  kind  items  rows  row_h  block_w  block_h  sb   rows_fit  head_fit  inside  Took_ms  A_rect                  B_rect                  header / indicator"]
    for r in rows:
        lines.append(f"{r['id']:<3} {r['kind']:<5} {r['items']:<6} {r['rows']:<5} {r['row_h']:<6} {r['w']:<8} {r['h']:<8} {'yes' if r['sb'] else 'no':<4} "
                     f"{'yes' if r['rows_fit'] else 'NO':<9} {'yes' if r['head_fit'] else 'NO':<9} {'yes' if r['inside'] and r['sb_inside'] else 'NO':<7} "
                     f"{r['dur'] or '-':<8} {str(r['a']):<23} {str(r['b']):<23} {r['header']} / {r['indicator'] or '-'}")
    return "\n".join(lines)


def main():
    if len(sys.argv) != 2 or sys.argv[1].startswith("-"):
        print(__doc__.split("usage:")[1].split("\n")[0].strip()); return 2
    try:
        import PIL  # noqa: F401
    except ImportError:
        venv = Path(__file__).resolve().parent.parent / ".venv/bin/python"
        if venv.exists() and Path(sys.prefix).resolve() != venv.parent.parent.resolve():
            os.execv(str(venv), [str(venv), os.path.abspath(__file__)] + sys.argv[1:])
        print("PIL missing and no repo .venv"); return 1
    rows, table, written = render_all(sys.argv[1])
    print(table)
    for p in written:
        print(p)
    return 0


if __name__ == "__main__":
    sys.exit(main())
