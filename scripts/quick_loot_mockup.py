#!/usr/bin/env python3
"""Offline render of the quick-loot HUD list block (doc 52 QL2L, 0.4.0-dev): judge fit, scrolling and the vanilla look before play testing.

Reuses character_page_mockup.py (cpm): the dialogfont10x10 bitmap font (glyph boxes from the font TXI, advances from
_shared_src/FontWidths.inc, DLL-calibrated 2.75 px/texel at 3840x1600), the real draw_text() and the real draw_border() (the same call that
draws the vanilla tooltip at character_page_mockup.py ~:553). Reads the live install read-only through pykotor; writes PNGs only to OUTDIR.

What the mod draws (patch_manager_mods/quick_loot_src/QuickLoot.cpp blockDraw + QuickLootLogic.h):
  one BLOCK = frame panel + header row + up to ListRows item rows (default 8, 1..12), all engine px (guikit::setRect writes engine px;
  3840x1600 on the dev box; NOT 800x600 gui units).
  Frame: a label cloned from the HUD template LBL_MOULDING3 (the vanilla tooltip frame: FILL uibit_fill_2bt, EDGE uibit_brdr_16bet, CORNER
  uibit_brdr_16bct, DIMENSION 16 taken as px, tint TEAL_FRAME) -- the engine draws the edges and corners from the .gui, GuiKit only sets tint
  and alpha. The fill is black at alpha 0.78 (texture alpha 199/255), alpha 1.0.
  Text: dialogfont10x10, align 9 (top-left) in a one-row rect. Header cream (CREAM), rows TEAL_TEXT, hovered row CREAM, scroll indicator dim
  teal at the right end of the header row. Row pitch RowH = 32 px (glyph box 27.5 px; tooltip line pitch 34 px).
  Rows: name cut at 40 chars (first 37 + "..."), " xN" for stacks 2..999, cut further to the block's max inner width; items past the 64 captured
  become one last "+K more" row. Block width = widest row (ALL rows, so scrolling never resizes it) + 2 x 28 px pad, 300..900 px.
  Placement: position A under the target name (x centred on the name, clamped to the panel), B at the fixed spot (centre of the panel, 62 %
  down); bottom clamp: A flips above the name, B moves up. The mod latches A or B when the list first shows.
Target menu mock (doc 39 R6 observed): name label local (0,2) w 533 h 32 (health bar local y 35 h 16).

The one-line formatters (fmtList / fmtTook / fmtPeek, the 0.3.1 fallback text) and the list layer (rowName / rowText / ListData / ListView /
buildListDraw / blockRect) are ports of QuickLootLogic.h, checked against quick_loot_logic_test.cpp vectors in test_quick_loot_mockup.py.

usage: quick_loot_mockup.py OUTDIR
Outputs: ql2l_<id>_full.png (A and B on one frame), ql2l_<id>_zoom.png (crops of both blocks), ql2l_fit_table.txt
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


# ---- 0.4.0-dev list layer: ports of QuickLootLogic.h (QL2L) -------------------------------------------------------
NAME_CAP = 48
LIST_MAX_ITEMS = 64
LIST_ROWS_DEFAULT, LIST_ROWS_MIN, LIST_ROWS_MAX = 8, 1, 12
ROW_NAME_MAX = 40
ROW_H = 32                       # row pitch px (glyph box 27.5 px at 2.7467 px/texel; tooltip line pitch 34)
HEADER_GAP = 6
BLOCK_PAD_X, BLOCK_PAD_Y = 28, 20
BLOCK_MIN_W, BLOCK_MAX_W = 300, 900
LABEL_SLACK = 12
INDICATOR_GAP = 36
ROW_INNER_MAX_PX = BLOCK_MAX_W - 2 * BLOCK_PAD_X
WHEEL_STEP = 120
TOOK_BASE_MS, TOOK_ROW_MS, TOOK_CAP_MS, TOOK_FREE_ROWS = 3000, 500, 6000, 3
FRAME_ALPHA = 1.0
FRAME_DIM = 16                   # template DIMENSION, taken as engine px
FRAME_TEMPLATE = {"FILL": "uibit_fill_2bt", "EDGE": "uibit_brdr_16bet", "CORNER": "uibit_brdr_16bct"}
FONT_TEXTK, FONT_SY = 1.03, 1600.0 / 600.0   # textfit::TextCtx for dialogfont10x10
# vanilla palette (character_page_mockup.py:45-47); the mod's constants in QuickLoot.cpp
TEAL_FRAME = (0.05098, 0.34902, 0.27059)
TEAL_TEXT = (0.10196, 0.69804, 0.54902)
CREAM = (0.8, 0.8, 0.69804)
TEAL_DIM = (0.0663, 0.4537, 0.3569)
TEXT_ALIGN = 9                   # top-left

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


def row_text(name, stack, max_px=ROW_INNER_MAX_PX):
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
    """ListData: items = captured rows, more = items past the capture (one last '+K more' row)."""

    def __init__(self, names, stacks, total=None):
        count = min(len(names), LIST_MAX_ITEMS)
        self.rows = [row_text(names[i], stacks[i] if stacks else -1) for i in range(count)]
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


class ListView:
    """ListView: rows visible, total list rows, off first visible row; the wheel moves WHEEL_STEP per row (+ = up)."""

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


def build_list_draw(d, v, took, key="F"):
    """buildListDraw -> dict(header, indicator, rows, vis, off, w, h, scrollable, header_px, indicator_px) or None for an empty list."""
    if d.items == 0:
        return None
    total = d.rows_total()
    rows = max(LIST_ROWS_MIN, min(LIST_ROWS_MAX, v.rows))
    vis = min(total, rows)
    max_off = total - rows if total > rows else 0
    off = min(v.off, max_off)
    header = "Took (%u)" % d.total_items() if took else "[%s] Take all (%u)" % (key, d.total_items())
    o = dict(took=took, off=off, vis=vis, rows_total=total, items=d.total_items(), scrollable=total > rows, header=header,
             header_px=text_px10(header), indicator="", indicator_px=0)
    need = d.max_row_px
    if o["scrollable"]:
        last = d.total_items() if off + vis > d.items else off + vis
        o["indicator"] = "%u-%u of %u" % (off + 1, last, d.total_items())
        o["indicator_px"] = text_px10(o["indicator"])
        worst = "%u-%u of %u" % (max_off + 1, d.total_items(), d.total_items())
        need = max(need, o["header_px"] + INDICATOR_GAP + text_px10(worst))
    else:
        need = max(need, o["header_px"])
    o["w"] = max(BLOCK_MIN_W, min(BLOCK_MAX_W, need + 2 * BLOCK_PAD_X))
    o["h"] = 2 * BLOCK_PAD_Y + ROW_H + HEADER_GAP + vis * ROW_H
    o["rows"] = [d.row_at(off + i) for i in range(vis)]
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
    return (BLOCK_PAD_X, BLOCK_PAD_Y, w - 2 * BLOCK_PAD_X + LABEL_SLACK, ROW_H)


def row_rect(i, w):
    return (BLOCK_PAD_X, BLOCK_PAD_Y + ROW_H + HEADER_GAP + i * ROW_H, w - 2 * BLOCK_PAD_X + LABEL_SLACK, ROW_H)


def indicator_rect(w, ind_px):
    return (w - BLOCK_PAD_X - ind_px, BLOCK_PAD_Y, ind_px + LABEL_SLACK, ROW_H)


# ---- sample lists --------------------------------------------------------------------------------------------------
REALISTIC = ["Medpac", "Blaster Pistol", "Adhesive Grenade", "Lightsaber Crystal: Opila", "Frag Grenade", "Credit Chip", "Vibroblade",
             "Combat Suit", "Stimulant", "Glow Rod", "Repair Kit", "Security Spike", "Power Cell", "Ion Blaster", "Datapad", "Cure Pack",
             "Mandalorian Heavy Armor Mk IV", "Jedi Robe", "Energy Shield", "Comlink", "Targeting Scope", "Stun Baton", "Bacta Tank",
             "Ancient Sith Holocron Fragment", "Droid Probe"]
REALISTIC_STACKS = [2, 1, 3, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 1, 1]
LONG_NAMES = ["Lightsaber Crystal: Opila", "Adhesive Grenade", "Mandalorian Heavy Armor Mk IV",
              "Ancient Sith Holocron Fragment of the Old Republic Era", "Medpac"]
LONG_STACKS = [1, 3, 1, 1, 999]


def numbered(n):
    return ["Salvage Part %d" % (i + 1) for i in range(n)]


def samples():
    """[(id, desc, took, ListData, scroll_off, hover_row, rows)] -- scroll_off in list rows."""
    r25 = ListData(REALISTIC, REALISTIC_STACKS)
    r70 = ListData(numbered(64), None, total=70)
    return [
        (1, "Peek, 1 item", False, ListData(["Medpac"], None), 0, None, 8),
        (2, "Peek, 4 items", False, ListData(REALISTIC[:4], REALISTIC_STACKS[:4]), 0, None, 8),
        (3, "Peek, 8 items (fills the list)", False, ListData(REALISTIC[:8], REALISTIC_STACKS[:8]), 0, 1, 8),
        (4, "Peek, 25 items, off 0", False, r25, 0, None, 8),
        (5, "Peek, 25 items scrolled to off 10, hover row 2", False, r25, 10, 2, 8),
        (6, "Peek, long names (cut at 40 chars, stacks)", False, ListData(LONG_NAMES, LONG_STACKS), 0, None, 8),
        (7, "Took, 6 items", True, ListData(REALISTIC[:6], REALISTIC_STACKS[:6]), 0, None, 8),
        (8, "Peek, 70 items (64 captured) scrolled to the end", False, r70, 99, None, 8),
        (9, "Peek, 25 items, ListRows 12", False, r25, 0, None, 12),
        (10, "Peek, 25 items, ListRows 4", False, r25, 0, 0, 4),
    ]


def sample_draw(sample):
    sid, desc, took, d, off, hover, rows = sample
    v = ListView(d.rows_total(), rows)
    v.off = min(off, v.max_off())
    return build_list_draw(d, v, took), v


PANEL = (0, 0, SCREEN_W, SCREEN_H)


def placements(o, name_origin=MENU_ORIGIN):
    """([(label, rect)] for position A (target name at name_origin) and B (fixed spot), name rect)."""
    name = (name_origin[0] + NAME_LOCAL[0], name_origin[1] + NAME_LOCAL[1], NAME_LOCAL[2], NAME_LOCAL[3])
    a = choose_anchor(True, True, name, True, PANEL)
    b = choose_anchor(False, True, name, True, PANEL)
    return [("A", block_rect(a, o["w"], o["h"], PANEL)), ("B", block_rect(b, o["w"], o["h"], PANEL))], name


def fit_row(sample):
    """Fit-table row: every row fits the block's inner width, header + gap + indicator fit, block inside the panel with the margin."""
    sid, desc, took, d, off, hover, rows = sample
    o, v = sample_draw(sample)
    inner = o["w"] - 2 * BLOCK_PAD_X
    rows_fit = all(text_px10(d.row_at(i)) <= inner for i in range(d.rows_total()))
    head_fit = o["header_px"] + (INDICATOR_GAP + o["indicator_px"] if o["scrollable"] else 0) <= inner
    (pa, ra), (pb, rb) = placements(o)[0]
    inside = all(r[0] >= SCREEN_MARGIN and r[1] >= SCREEN_MARGIN and r[0] + r[2] <= SCREEN_W - SCREEN_MARGIN and r[1] + r[3] <= SCREEN_H - SCREEN_MARGIN
                 for r in (ra, rb))
    return dict(id=sid, desc=desc, kind="took" if took else "peek", items=d.total_items(), list_rows=d.rows_total(), rows=o["vis"],
                w=o["w"], h=o["h"], rows_fit=rows_fit, head_fit=head_fit, inside=inside, a=ra, b=rb, header=o["header"], indicator=o["indicator"],
                dur=took_duration_ms(d.rows_total()) if took else 0)


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


def draw_frame(img, cpm, rect):
    """The vanilla frame through cpm.draw_border (the call that draws the vanilla tooltip), at 1 unit = 1 engine px so DIMENSION 16 is 16 px."""
    from PIL import Image
    x, y, w, h = rect
    layer = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    saved = (cpm.SX, cpm.SY)
    cpm.SX = cpm.SY = 1.0
    try:
        cpm.draw_border(layer, dict(FRAME_TEMPLATE, DIMENSION=FRAME_DIM, COLOR=TEAL_FRAME), (0, 0, w, h))
    finally:
        cpm.SX, cpm.SY = saved
    if FRAME_ALPHA < 1.0:
        a = layer.getchannel("A").point(lambda v: int(v * FRAME_ALPHA))
        layer.putalpha(a)
    img.alpha_composite(layer, (x, y))


def draw_block(img, cpm, o, rect, hover_row=None):
    """The whole block as the mod lays it out: frame, header (cream), indicator (dim teal), rows (teal, hovered row cream)."""
    x, y, w, h = rect
    draw_frame(img, cpm, rect)

    def put(text, r, col):
        rx, ry, rw, rh = r
        cpm.draw_text(img, FONT, text, (x + rx, y + ry, x + rx + rw, y + ry + rh), TEXT_ALIGN, col)

    put(o["header"], header_rect(w), CREAM)
    if o["indicator"]:
        put(o["indicator"], indicator_rect(w, o["indicator_px"]), TEAL_DIM)
    for i, t in enumerate(o["rows"]):
        put(t, row_rect(i, w), CREAM if i == hover_row else TEAL_TEXT)


def caption(img, lines):
    from PIL import ImageDraw, ImageFont
    f = ImageFont.load_default(size=26)
    d = ImageDraw.Draw(img)
    y = 20
    for ln in lines:
        d.text((24, y), ln, fill=(235, 235, 235, 255), font=f, stroke_width=2, stroke_fill=(0, 0, 0, 255))
        y += 34


def zoom(img, rect, z=2, pad=40):
    from PIL import Image
    x, y, w, h = rect
    box = (max(0, x - pad), max(0, y - pad), min(SCREEN_W, x + w + pad), min(SCREEN_H, y + h + pad))
    c = img.crop(box)
    return c.resize((c.width * z, c.height * z), Image.LANCZOS)


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
        draw_block(img, cpm, o, ra, hover)
        draw_block(img, cpm, o, rb, hover)
        fr = fit_row(sample)
        rows.append(fr)
        caption(img, [
            f"#{sid} {desc}: {fr['items']} items, {fr['rows']} rows visible, block {fr['w']}x{fr['h']} px, header \"{fr['header']}\""
            + (f", indicator \"{fr['indicator']}\"" if fr["indicator"] else ""),
            f"A: under the target name {ra[:2]}.   B: fixed spot {rb[:2]} (62% of the panel).   rows fit: {'yes' if fr['rows_fit'] else 'NO'}, "
            f"header+indicator fit: {'yes' if fr['head_fit'] else 'NO'}, inside the panel: {'yes' if fr['inside'] else 'NO'}",
            "Frame: vanilla tooltip border (uibit_fill_2bt + uibit_brdr_16bet/16bct, teal), dialogfont10x10, header cream, rows teal, hover cream.",
        ])
        full = outdir / f"ql2l_{sid}_full.png"
        img.save(full)
        za, zb = zoom(img, ra), zoom(img, rb)
        sheet = Image.new("RGBA", (max(za.width, zb.width), za.height + zb.height + 6), (0, 0, 0, 255))
        sheet.alpha_composite(za, (0, 0))
        sheet.alpha_composite(zb, (0, za.height + 6))
        zp = outdir / f"ql2l_{sid}_zoom.png"
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
    p = outdir / "ql2l_11_full.png"
    img.save(p)
    written.append(p)
    fr = fit_row(sample)
    fr.update(id=11, desc="Peek, 25 items, ListRows 12, target low (A flips above)", a=ra, b=rb, inside=ra[1] + ra[3] <= SCREEN_H - SCREEN_MARGIN)
    rows.append(fr)
    table = format_table(rows)
    (outdir / "ql2l_fit_table.txt").write_text(table + "\n")
    return rows, table, written


def format_table(rows):
    lines = ["id  kind  items  rows  block_w  block_h  rows_fit  head_fit  inside  Took_ms  A_rect                  B_rect                  header / indicator"]
    for r in rows:
        lines.append(f"{r['id']:<3} {r['kind']:<5} {r['items']:<6} {r['rows']:<5} {r['w']:<8} {r['h']:<8} {'yes' if r['rows_fit'] else 'NO':<9} "
                     f"{'yes' if r['head_fit'] else 'NO':<9} {'yes' if r['inside'] else 'NO':<7} {r['dur'] or '-':<8} {str(r['a']):<23} {str(r['b']):<23} "
                     f"{r['header']} / {r['indicator'] or '-'}")
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
