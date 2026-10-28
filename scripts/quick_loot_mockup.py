#!/usr/bin/env python3
"""Offline render of the quick-loot HUD label (doc 52 PV1 step 1): judge text fit and placement before play testing.

Reuses character_page_mockup.py (cpm): the dialogfont10x10 bitmap font (glyph boxes from the font TXI, advances from
_shared_src/FontWidths.inc, DLL-calibrated 2.75 px/texel at 3840x1600) and the real draw_text(). Reads the live install
read-only through pykotor; writes PNGs only to OUTDIR.

What the mod draws (patch_manager_mods/quick_loot_src/QuickLoot.cpp labelDraw + QuickLootLogic.h, 0.3.1-dev):
  one label cloned from the HUD template LBL_QUEUE1, font dialogfont10x10, align 18 (centre/middle), colour (0.9,0.85,0.5), on a backing
  panel (a second label created first: black, alpha 0.55, pad 10 px x / 4 px y, width follows the text, centred on the label rect).
  Rect units: ENGINE PIXELS (guikit::setRect writes the control rect directly; GuiKit.h getExtent "engine px"; the name rect
  is read as menu origin + name rect, "all px"; doc 39 R6: absolute display px, 3840x1600 on the dev box). They are NOT
  800x600 GUI units: 420 px = 87.5 units at 3840x1600. Label height 20 px is thinner than the 10x10 glyph cell (~44 px at
  2.75 px/texel), so the text is vertically centred on a 20 px rect and overflows it above and below.
  Overflow: the engine word-wraps text wider than the label rect (doc 14 build 4 finding), it does not clip; the mock draws one
  unwrapped line and outlines an overflowing rect in red.
  Widths: peek and Took share ONE rect, LabelW 1200 px (31 % of 3840), 56 chars max on one line, item names cut at 20 chars
  ("Lightsaber Crysta...", " xN" added after the cut).
  Position A (name-anchored): x centred on the target-menu name label (clamped to [24, screenW-24-w]), y = name bottom + 20.
  Position B (fixed): x centred on the HUD panel, y = 62 % of the panel height. The mod latches A or B when the line first shows.
Target menu mock (doc 39 R6 observed): name label local (0,2) w 533 h 32 (= LBL_NAME 200 units * 8/3), health bar local y 35 h 16.
The live mipc28x6_p.gui LBL_NAME/PB_HEALTH rects (0,1,200,26)/(0,27,200,6) units are only the template; the engine re-lays
the menu, so the observed px values are used.

The formatters (fmtList / fmtTook / fmtPeek / sanitizeName) are ports of QuickLootLogic.h, checked against
quick_loot_logic_test.cpp vectors in test_quick_loot_mockup.py.

usage: quick_loot_mockup.py OUTDIR
Outputs: ql_<id>_full.png (A and B on one frame), ql_<id>_zoom.png (2x crops of both labels), ql_fit_table.txt
"""
import os
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
COLOR = (0.9, 0.85, 0.5)

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


# ---- the sample texts (the review examples plus worst cases) -----------------------------------------------------
def sample_texts():
    n_pair = ["Medpac", "Blaster Pistol"]
    realistic = ["Medpac", "Blaster Pistol", "Adhesive Grenade", "Lightsaber Crystal: Opila", "Frag Grenade", "Credit Chip",
                 "Vibroblade", "Combat Suit", "Stimulant", "Glow Rod", "Repair Kit", "Security Spike", "Power Cell",
                 "Ion Blaster", "Datapad", "Cure Pack"]
    st16 = [2, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
    long_pair = ["Adhesive Grenade", "Lightsaber Crystal: Opila"]
    wide = ["Mandalorian Heavy Armor Plating", "Lightsaber Crystal: Opila", "Ancient Sith Holocron Fragment"]
    return [
        (1, "Took, 2 items", "took", fmt_took(n_pair, [2, 1], 2)),
        (2, "Took, 25 items", "took", fmt_took(realistic, st16, 16, 9)),
        (3, "Peek, 4 items, +2 more", "peek", fmt_peek("F", 4, n_pair, [2, 1], 2)),
        (4, "Peek, 25 items", "peek", fmt_peek("F", 25, realistic, st16, 16)),
        (5, "Peek, 1 item", "peek", fmt_peek("F", 1, ["Medpac"], None, 1)),
        (6, "Took, long item names", "took", fmt_took(long_pair, [3, 1], 2)),
        (7, "Peek, 3 very long names", "peek", fmt_peek("F", 3, wide, [1, 1, 999], 3)),
        (8, "Took, worst case (20-char names, x999)", "took", fmt_took(["W" * 20] * 16, [999] * 16, 16, 9)),
    ]


def label_w(kind):
    return LABEL_W


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


def draw_label(img, cpm, text, rect, overflow):
    from PIL import Image, ImageDraw
    x, y, w, h = rect
    bx, by, bw, bh = back_rect(rect, len(text))       # backing panel: black, alpha 0.55 (drawn first = behind the text)
    panel = Image.new("RGBA", (bw, bh), (0, 0, 0, int(round(255 * BACK_ALPHA))))
    img.alpha_composite(panel, (max(0, bx), max(0, by)))
    d = ImageDraw.Draw(img)
    d.rectangle([x, y, x + w - 1, y + h - 1], outline=(200, 70, 70, 140) if overflow else (140, 140, 140, 60))
    cpm.draw_text(img, FONT, text, (x, y, x + w, y + h), ALIGN, COLOR)


def caption(img, lines):
    from PIL import ImageDraw, ImageFont
    f = ImageFont.load_default(size=26)
    d = ImageDraw.Draw(img)
    y = 20
    for ln in lines:
        d.text((24, y), ln, fill=(235, 235, 235, 255), font=f, stroke_width=2, stroke_fill=(0, 0, 0, 255))
        y += 34


def zoom(img, rect, z=2, pad_x=40, pad_y=40, min_w=0):
    from PIL import Image
    x, y, w, h = rect
    cx = x + w // 2
    cw = max(w, min_w) + 2 * pad_x
    box = (max(0, cx - cw // 2), max(0, y - pad_y), min(SCREEN_W, cx + cw // 2), min(SCREEN_H, y + h + pad_y))
    c = img.crop(box)
    return c.resize((c.width * z, c.height * z), Image.LANCZOS)


def measure(cpm):
    """[(id, desc, kind, text, chars, width_px, label_w, fits)]"""
    rows = []
    for tid, desc, kind, text in sample_texts():
        wpx = cpm.text_size(FONT, text)[0]
        lw = label_w(kind)
        rows.append((tid, desc, kind, text, len(text), wpx, lw, wpx <= lw))
    return rows


def render_all(outdir):
    from PIL import Image
    cpm = _cpm()
    outdir = Path(outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    rows = measure(cpm)
    written = []
    for tid, desc, kind, text, nch, wpx, lw, ok in rows:
        img = background()
        name_rect = draw_target_tag(img, cpm, MENU_ORIGIN)
        ra = toast_rect(*name_rect, w=lw, screen=(0, SCREEN_W))
        rb = fixed_rect(0, 0, SCREEN_W, SCREEN_H, w=lw)
        draw_label(img, cpm, text, ra, not ok)
        draw_label(img, cpm, text, rb, not ok)
        caption(img, [
            f"#{tid} {desc}: {nch} chars, text {wpx:.0f} px, label {lw} px -> {'fits' if ok else 'OVERFLOWS (engine word-wraps; red outline)'}",
            "A: under target name (name bottom + 20, x clamped to [24, W-24-w]).   B: x centred, y = 62% of HUD height (992 px).",
            "Backing: black alpha 0.55, pad 10x/4y, width follows the text. Rect units = engine px (setRect/getExtent), NOT 800x600 GUI units.",
        ])
        full = outdir / f"ql_{tid}_full.png"
        img.save(full)
        za, zb = zoom(img, ra, min_w=lw), zoom(img, rb, min_w=lw)
        sheet = Image.new("RGBA", (max(za.width, zb.width), za.height + zb.height + 6), (0, 0, 0, 255))
        sheet.alpha_composite(za, (0, 0))
        sheet.alpha_composite(zb, (0, za.height + 6))
        zp = outdir / f"ql_{tid}_zoom.png"
        sheet.save(zp)
        written += [full, zp]
    table = format_table(rows)
    (outdir / "ql_fit_table.txt").write_text(table + "\n")
    return rows, table, written


def format_table(rows):
    lines = ["id  chars  text_px  label_px  back_px  fits  kind  text"]
    for tid, desc, kind, text, nch, wpx, lw, ok in rows:
        lines.append(f"{tid:<3} {nch:<6} {wpx:<8.0f} {lw:<9} {back_rect((0, 0, lw, TOAST_H), nch)[2]:<8} {'yes' if ok else 'NO':<5} {kind:<5} {text}")
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
