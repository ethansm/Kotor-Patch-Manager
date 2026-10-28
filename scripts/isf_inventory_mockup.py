#!/usr/bin/env python3
"""Offline render of the KOTOR II inventory screen (inventory_p) as the inventory-sort-filter mod (ISF 0.3.1-dev) changes it: judge the filter
caption fit, the tab highlight, the row order per sort order, the "cannot equip" tint and the three filters before play testing.

Reuses character_page_mockup.py (cpm: real dialogfont bitmap fonts with DLL-calibrated widths at 3840x1600, uibit textures, draw_text, draw_border)
and quick_loot_mockup.py (qlm: vanilla row frame / icon helpers, "i" + baseitems itemclass + "_%03d" icon lookup, baseitems.2da reading). Reads
the live install read-only through pykotor (inventory_p.gui from Override, textures, baseitems.2da); writes PNGs and a text table to OUTDIR only.

What the mod changes on the vanilla panel (patch_manager_mods/inventory_sort_filter_src/):
  * LBL_FILTER caption (SortLogic.h captionWithCount :134-148, captionOf :124-131): All -> "Filter"; otherwise "Filter: <Name>", then " P/N" (1-based
    place among the enabled non-All states in cycle order New, Usable, Upgradeable; only when N >= 2), then " (rows)". Longest text
    "Filter: Upgradeable 3/3 (1024)" = 30 chars. The label keeps the gui's rect (LBL_FILTER 149,104,72,22 units) and font (dialogfont16x16, align 18).
  * Tab highlight: the cycle states (All -> New -> Usable -> Upgradeable -> All, advanced by re-clicking All) run on the All tab, so All stays the
    highlighted tab (HILIGHT COLOR 0.8,0.8,0.698 = cream text, the button's gui hilight) in every state.
  * Row order (SortLogic.h beforeBy :64-72, sortEntries :73-76, categoryOf :217-226): isNew first, usable next, then per sort_order
    Category (category 0 melee, 1 ranged, 2 armour, 3 other gear, 4 upgrades + crystals, 5 consumables, 6 misc/quest; then base item, name),
    Base item (base item, name), Name (name only), Vanilla (untouched); ties fall to the vanilla index. Names compare ASCII case-insensitively.
  * Tint (InventorySortFilter.cpp createRow :350-363 -> SetCanUse(row, 2)): equippable items the shown character cannot equip use the SetCanUse
    mode 2 colour (0.74, 0.11, 0.0) on frame and text (tintRow = equippable && !canEquip, SortLogic.h :217).
  * Filters: New = isNew rows (engine mode 1); Usable = canEquip || activatable (usableForCycle); Upgradeable = items with an open upgrade slot.

Geometry: every control comes from the live Override inventory_p.gui (the ultrawide thirds layout), units -> px with cpm.SX / SY (4.8 / 2.667 at
3840x1600). Row look = the vanilla PROTOITEM (200 x 50 units, 16bct/16bet/2bt frame, DIMENSION taken as px, teal), as quick_loot_mockup
vanilla_inventory_list builds it. The visible window is as many whole rows as fit in LB_ITEMS (6 at this layout); the strip under the panel
lists the complete order so the whole list can be judged.

usage: isf_inventory_mockup.py OUTDIR
Outputs: isf_all_category.png, isf_all_name.png, isf_all_base.png, isf_all_vanilla.png (state All, each sort order), isf_new.png, isf_usable.png,
isf_upgrade.png (sort Category), isf_caption_zoom.png (caption + tab strip, 3x, all 4 states + the 1024-row worst case), isf_sheet.png (contact sheet),
isf_fit_table.txt (caption widths vs the label, rows visible vs total, full order per render).
"""
import functools
import sys
from dataclasses import dataclass
from pathlib import Path

SCREEN_W, SCREEN_H = 3840, 1600

# ---- ported from SortLogic.h -----------------------------------------------------------------------------------
S_ALL, S_NEW, S_USABLE, S_UPGRADE, S_COUNT = 0, 1, 2, 3, 4
STATE_NAMES = {S_ALL: "All", S_NEW: "New", S_USABLE: "Usable", S_UPGRADE: "Upgradeable"}
CYCLE_ORDER = (S_NEW, S_USABLE, S_UPGRADE)
CAPTION_BYTES = 32
ALL3 = (1 << S_NEW) | (1 << S_USABLE) | (1 << S_UPGRADE)
SO_CATEGORY, SO_BASE, SO_NAME, SO_VANILLA = 0, 1, 2, 3
SORT_NAMES = {SO_CATEGORY: "category", SO_BASE: "base", SO_NAME: "name", SO_VANILLA: "vanilla"}
BASE_LIGHTSABER_CRYSTALS, BASE_UPGRADE_ITEMS = 11, 101


def caption_of(s):
    return {S_NEW: "Filter: New", S_USABLE: "Filter: Usable", S_UPGRADE: "Filter: Upgradeable"}.get(s, "Filter")


def caption_with_count(s, enabled_mask, rows, cap=CAPTION_BYTES):
    """captionWithCount: returns the string the C++ writes into a cap-byte buffer (snprintf truncation: cap-1 chars), None when cap <= 0."""
    if cap <= 0:
        return None
    total = place = 0
    for c in CYCLE_ORDER:
        if not (enabled_mask & (1 << c)):
            continue
        if c == s:
            place = total + 1
        total += 1
    if s == S_ALL or total == 0:
        return "Filter"[:cap - 1]
    pos = " %d/%d" % (place, total) if place > 0 and total > 1 else ""
    cnt = " (%d)" % rows if rows >= 0 else ""
    return (caption_of(s) + pos + cnt)[:cap - 1]


def next_state(s, enabled_mask):
    for i in range(1, S_COUNT + 1):
        t = (s + i) % S_COUNT
        if t == S_ALL or (enabled_mask & (1 << t)):
            return t
    return S_ALL


def category_of(upgrade_type, slot_mask, activatable, base_item):
    if upgrade_type == 1:
        return 0
    if upgrade_type == 2:
        return 1
    if upgrade_type == 3:
        return 2
    if upgrade_type == 4 or slot_mask:
        return 3
    if base_item in (BASE_LIGHTSABER_CRYSTALS, BASE_UPGRADE_ITEMS):
        return 4
    return 5 if activatable else 6


def tint_row(equippable, can_equip):
    return equippable and not can_equip


def usable_for_cycle(can_equip, activatable):
    return can_equip or activatable


def fold_cmp(a, b):
    """ASCII-only fold, raw bytes >= 0x80 (foldCmp)."""
    def key(s):
        return bytes(c + 32 if 65 <= c <= 90 else c for c in s.encode("latin-1", "replace"))
    ka, kb = key(a), key(b)
    return -1 if ka < kb else (1 if ka > kb else 0)


@dataclass
class Entry:
    item: int
    id: int
    orig: int
    is_new: bool
    usable: bool
    base_item: int
    tint: bool
    name: str = ""
    category: int = 6


def before_by(order, a, b):
    if order == SO_VANILLA:
        return a.orig < b.orig
    if a.is_new != b.is_new:
        return a.is_new
    if a.usable != b.usable:
        return a.usable
    if order == SO_CATEGORY and a.category != b.category:
        return a.category < b.category
    if order != SO_NAME and a.base_item != b.base_item:
        return a.base_item < b.base_item
    c = fold_cmp(a.name, b.name)
    return c < 0 if c != 0 else a.orig < b.orig


def sort_entries(entries, order=SO_BASE):
    """Stable sort like std::stable_sort(beforeBy); returns a new list (Vanilla keeps the input order)."""
    e = list(entries)
    if order == SO_VANILLA:
        return e
    return sorted(e, key=functools.cmp_to_key(lambda a, b: -1 if before_by(order, a, b) else (1 if before_by(order, b, a) else 0)))


# ---- sample backpack ---------------------------------------------------------------------------------------------
# baseitems.2da equipableslots (K2, read from the install): the slot masks of the rows used, so the logic needs no install.
SLOT_MASK = {1: 0x30, 5: 0x30, 8: 0x30, 12: 0x30, 13: 0x30, 77: 0x10, 38: 2, 42: 2, 45: 8, 47: 0x400, 50: 0x200,
             11: 0, 101: 0, 25: 0, 29: 0, 53: 0, 54: 0, 55: 0, 59: 0, 64: 0}


@dataclass
class Item:
    name: str
    base: int            # baseitems.2da row
    var: int = 1         # ModelVariation (icon suffix)
    stack: int = 1
    upgrade_type: int = 0
    activatable: bool = False
    is_new: bool = False
    can_equip: bool = True
    open_slot: bool = False
    desc: tuple = ()

    @property
    def slot_mask(self):
        return SLOT_MASK.get(self.base, 0)


# vanilla (acquisition) order. 22 items: 3 melee, 3 blasters (+1 rifle counted below), 2 armour, gloves/belt/implant, 3 upgrades, 2 crystals,
# 3 medpacs/stims, 2 grenades, quest + misc. 4 New, 3 with an open upgrade slot, 2 the shown character cannot equip.
SAMPLE = [
    Item("Medpac", 55, 1, 3, activatable=True, desc=("Restores 80 vitality points.",)),
    Item("Frag Grenade", 25, 1, 4, activatable=True, desc=("Thrown. 12 damage in a 4 m radius.",)),
    Item("Blaster Pistol", 12, 1, 1, 2, open_slot=True, desc=("Damage 3d6 energy", "Critical 19-20 x2", "Upgrade slots: Power Pack, Targeting")),
    Item("Targeting Scope", 101, 53, 1, is_new=True, desc=("Upgrade: ranged weapon.", "Attack bonus +2.")),
    Item("Vibroblade", 5, 1, 1, 1, desc=("Damage 2d6 slashing", "Critical 19-20 x2")),
    Item("Mandalorian Heavy Armor", 42, 1, 1, 3, can_equip=False, desc=("Armor class 9", "Requires Heavy Armor proficiency.")),
    Item("Security Spikes", 59, 1, 8, desc=("Used to bypass locked doors.",)),
    Item("Lightsaber Crystal: Opila", 11, 1, 1, desc=("Upgrade: lightsaber crystal.", "Damage +1d6.")),
    Item("Padded Gloves", 45, 1, 1, 4, open_slot=True, desc=("Armor class 1", "Upgrade slot: Gloves")),
    Item("Ionic Energy Cell", 101, 205, 1, desc=("Upgrade: ranged weapon.", "Ion damage +1d4.")),
    Item("Lightsaber", 8, 1, 1, 1, is_new=True, open_slot=True, desc=("Damage 2d8 energy", "Critical 19-20 x2", "Upgrade slots: Crystal, Lens, Energy Cell")),
    Item("Adrenaline Stimulant", 53, 1, 2, activatable=True, desc=("Strength +4 for 60 seconds.",)),
    Item("Light Combat Suit", 38, 1, 1, 3, desc=("Armor class 4", "Maximum Dex bonus +5")),
    Item("Heavy Blaster", 13, 1, 1, 2, is_new=True, desc=("Damage 3d8 energy", "Critical 20 x2")),
    Item("Flash Grenade", 29, 1, 2, activatable=True, desc=("Thrown. Stuns enemies in a 4 m radius.",)),
    Item("Implant: Reflex Mk III", 50, 1, 1, can_equip=False, desc=("Dexterity +3", "Requires character level 20.")),
    Item("Ravager Access Codes", 64, 1, 1, desc=("Quest item.", "Opens the Ravager's secured doors.")),
    Item("Lightsaber Crystal: Bondar", 11, 2, 1, desc=("Upgrade: lightsaber crystal.", "Attack bonus +1.")),
    Item("Reinforced Belt", 47, 1, 1, desc=("Constitution +2",)),
    Item("Blaster Rifle", 77, 1, 1, 2, desc=("Damage 3d8 energy", "Critical 20 x2")),
    Item("Padded Underlay", 101, 60, 1, desc=("Upgrade: armour.", "Defense +1.")),
    Item("Battle Stimulant", 54, 1, 1, is_new=True, activatable=True, desc=("Dexterity +4 for 60 seconds.",)),
]


def build_entries(items=None):
    """Entries in vanilla order, filled like the mod's populate hook (usable, tint, category)."""
    out = []
    for i, it in enumerate(items if items is not None else SAMPLE):
        usable = usable_for_cycle(bool(it.slot_mask) and it.can_equip, it.activatable)
        out.append(Entry(item=i, id=100 + i, orig=i, is_new=it.is_new, usable=usable, base_item=it.base,
                         tint=tint_row(bool(it.slot_mask), it.can_equip), name=it.name,
                         category=category_of(it.upgrade_type, it.slot_mask, it.activatable, it.base)))
    return out


def visible_entries(state, entries, items=None):
    items = items if items is not None else SAMPLE
    if state == S_NEW:
        return [e for e in entries if e.is_new]
    if state == S_USABLE:
        return [e for e in entries if e.usable]
    if state == S_UPGRADE:
        return [e for e in entries if items[e.item].open_slot]
    return list(entries)


def arrange(state, order, items=None):
    """The rows the panel shows for (state, sort order), in order."""
    return sort_entries(visible_entries(state, build_entries(items), items), order)


# ---- rendering (needs PIL + pykotor + the live install) ------------------------------------------------------------
TEAL_FRAME = (0.05098, 0.34902, 0.27059)
TEAL_TEXT = (0.10196, 0.69804, 0.54902)
CREAM = (0.8, 0.8, 0.69804)
ROW_HILIGHT = (0.92941, 1.0, 0.92941)
SB_THUMB_TINT = (0.98039, 1.0, 0.98039)
TINT_COL = (0.74, 0.11, 0.0)           # SetCanUse mode 2 (11_inventory_sort_filter.md R13)
NEW_FRAME = (0.46, 0.38, 0.05)         # vanilla "New" row colour (runtime global 0xA7FE48, value not readable offline): APPROXIMATED gold
NEW_TEXT = (0.95, 0.80, 0.18)
FILTER_FONT = "dialogfont16x16"
PROTO = (60, 150, 200, 50)             # PROTOITEM EXTENT, units
ICON_PAD = 4
CREDITS = "12450"

RUNS = [("isf_all_category", S_ALL, SO_CATEGORY), ("isf_all_name", S_ALL, SO_NAME), ("isf_all_base", S_ALL, SO_BASE),
        ("isf_all_vanilla", S_ALL, SO_VANILLA), ("isf_new", S_NEW, SO_CATEGORY), ("isf_usable", S_USABLE, SO_CATEGORY),
        ("isf_upgrade", S_UPGRADE, SO_CATEGORY)]


def _cpm():
    sys.path.insert(0, str(Path(__file__).parent))
    import character_page_mockup as cpm
    return cpm


def _qlm():
    sys.path.insert(0, str(Path(__file__).parent))
    import quick_loot_mockup as qlm
    return qlm


def load_gui(cpm):
    """inventory_p.gui from Override -> (root border dict, [control dicts with RECT, BORDER, HILIGHT, TEXT])."""
    from pykotor.resource.formats.gff import read_gff
    g = read_gff(str(next(p for p in (cpm.GAME / "override").iterdir() if p.name.lower() == "inventory_p.gui")))
    tlk = cpm.inst().talktable()

    def border(s):
        if s is None:
            return {}
        v = s.get_vector3("COLOR")
        return {"FILL": str(s.get_resref("FILL")), "EDGE": str(s.get_resref("EDGE")), "CORNER": str(s.get_resref("CORNER")),
                "DIMENSION": s.get_int32("DIMENSION"), "COLOR": (v.x, v.y, v.z)}
    ctls = []
    for c in g.root.get_list("CONTROLS"):
        e = c.get_struct("EXTENT")
        d = {"TAG": c.get_string("TAG"), "RECT": tuple(e.get_int32(k) for k in ("LEFT", "TOP", "WIDTH", "HEIGHT")),
             "BORDER": border(c.get_struct("BORDER") if c.exists("BORDER") else None),
             "HILIGHT": border(c.get_struct("HILIGHT") if c.exists("HILIGHT") else None)}
        if c.exists("TEXT"):
            t = c.get_struct("TEXT")
            v = t.get_vector3("COLOR")
            s = t.get_string("TEXT") if t.exists("TEXT") else ""
            sr = t.get_uint32("STRREF") if t.exists("STRREF") else 0xFFFFFFFF
            if sr != 0xFFFFFFFF:
                s = tlk.string(sr)
            d["TEXT"] = {"S": s, "FONT": str(t.get_resref("FONT")) or FILTER_FONT, "ALIGN": t.get_int32("ALIGNMENT"), "COLOR": (v.x, v.y, v.z)}
        ctls.append(d)
    return border(g.root.get_struct("BORDER")), ctls


def unit_px(cpm, rect):
    l, t, w, h = rect
    return (int(l * cpm.SX), int(t * cpm.SY), int(w * cpm.SX), int(h * cpm.SY))


def frame_px(img, cpm, b, rect, tint=None, hil=None):
    """cpm.draw_border at 1 unit = 1 px for rect (px) so a DIMENSION of 16 is 16 px (the convention of the vanilla tooltip / quick-loot frames)."""
    from PIL import Image
    x, y, w, h = rect
    if w < 1 or h < 1:
        return
    layer = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    saved = (cpm.SX, cpm.SY)
    cpm.SX = cpm.SY = 1.0
    try:
        cpm.draw_border(layer, dict(b, COLOR=tint if tint else b.get("COLOR", (1, 1, 1))), (0, 0, w, h))
    finally:
        cpm.SX, cpm.SY = saved
    img.alpha_composite(layer, (x, y))


def item_icon(cpm, qlm, it):
    for var in (it.var, 1):
        ic = qlm.baseitem_itemclass(it.base)
        if ic:
            res = qlm.icon_resref(ic, var)
            if cpm.texture(res) is not None:
                return res
    return ""


def row_colours(e, selected):
    """(frame tint, text colour) for a row: tint (SetCanUse 2) wins, then New, then teal; the selected row uses the vanilla HILIGHT frame."""
    if e.tint:
        return TINT_COL, TINT_COL
    if e.is_new:
        return NEW_FRAME, NEW_TEXT
    return (ROW_HILIGHT, CREAM) if selected else (TEAL_FRAME, TEAL_TEXT)


def draw_caption_overflow(img, cpm, label_px, text):
    """Red outline around the label rect when the text does not fit, plus a red bar under the real text extent."""
    from PIL import ImageDraw
    tw, th = cpm.text_size(FILTER_FONT, text)
    x, y, w, h = label_px
    if tw <= w:
        return False
    d = ImageDraw.Draw(img)
    d.rectangle([x, y, x + w - 1, y + h - 1], outline=(255, 40, 40, 255), width=4)
    cx = x + w / 2
    d.rectangle([cx - tw / 2, y + h - 2, cx + tw / 2, y + h + 6], fill=(255, 40, 40, 255))
    return True


def render(state, order, rows_for_caption=None, outpath=None, items=None, scroll=0):
    """One full 3840x1600 frame. Returns (PIL image, info dict)."""
    from PIL import Image, ImageDraw
    cpm, qlm = _cpm(), _qlm()
    items = items if items is not None else SAMPLE
    root, ctls = load_gui(cpm)
    by_tag = {c["TAG"]: c for c in ctls}
    rows = arrange(state, order, items)
    total_all = len(items)
    img = Image.new("RGBA", (SCREEN_W, SCREEN_H), (0, 0, 0, 255))
    bg = cpm.texture(root.get("FILL"))
    if bg is not None:
        cpm.paste_stretched(img, cpm.tint(bg, root.get("COLOR", (1, 1, 1))), (0, 0, SCREEN_W, SCREEN_H))
    caption = caption_with_count(state, ALL3, len(rows) if rows_for_caption is None else rows_for_caption)
    sel = items[rows[0].item] if rows else None
    info = {"caption": caption, "rows": rows, "overflow": False}
    for c in ctls:
        tag, b = c["TAG"], c["BORDER"]
        rect = c["RECT"]
        rpx = unit_px(cpm, rect)
        if tag == "3D_MODEL":
            d = ImageDraw.Draw(img)
            d.rectangle([rpx[0], rpx[1], rpx[0] + rpx[2], rpx[1] + rpx[3]], fill=(12, 20, 24, 255))
            ic = item_icon(cpm, qlm, sel) if sel else ""
            if ic:
                s = min(rpx[2], rpx[3]) // 2
                qlm.draw_icon(img, cpm, ic, (rpx[0] + (rpx[2] - s) // 2, rpx[1] + (rpx[3] - s) // 2, s, s))
            continue
        if tag == "LB_ITEMS":
            frame_px(img, cpm, b, rpx)
            continue
        if tag == "LB_DESCRIPTION":
            frame_px(img, cpm, b, rpx)
            if sel:
                pad = 16
                x0, y0, x1 = rpx[0] + pad + 8, rpx[1] + pad + 8, rpx[0] + rpx[2] - pad - 8
                cpm.draw_text(img, FILTER_FONT, sel.name, (x0, y0, x1, y0 + 44), 9, CREAM)
                y = y0 + 64
                lh = 12.5 * cpm.tscale("dialogfont10x10")
                for ln in sel.desc + (("Stack: %d" % sel.stack,) if sel.stack > 1 else ()):
                    for wl in cpm.wrap(ln, "dialogfont10x10", x1 - x0):
                        cpm.draw_text(img, "dialogfont10x10", wl, (x0, y, x1, y + lh), 9, TEAL_TEXT)
                        y += lh
            continue
        if tag.startswith("LBL_BAR") and tag != "LBL_BAR6" and not b.get("DIMENSION") and b.get("EDGE"):
            # decorative bars: DIMENSION 0 with an EDGE texture -> the edge strip stretched over the label (approximation, cpm draws nothing here)
            e = cpm.texture(b["EDGE"])
            if e is not None:
                cpm.paste_stretched(img, cpm.tint(e, b.get("COLOR", (1, 1, 1))), (rpx[0], rpx[1], rpx[0] + rpx[2], rpx[1] + min(rpx[3], 16)))
            continue
        if b.get("DIMENSION") and (b.get("EDGE") or b.get("CORNER")):
            frame_px(img, cpm, b, rpx)
        elif b.get("FILL"):
            frame_px(img, cpm, dict(b, DIMENSION=0), rpx)
        tx = c.get("TEXT")
        if not tx:
            continue
        s, col = tx["S"], tx["COLOR"]
        if tag == "LBL_FILTER":
            s = caption
        elif tag == "LBL_CREDITS_VALUE":
            s = CREDITS
        elif tag == "BTN_ALL":
            col = CREAM            # the highlighted tab: HILIGHT COLOR of the button (cycle states stay on All)
        if s:
            cpm.draw_text(img, tx["FONT"], s, (rpx[0], rpx[1], rpx[0] + rpx[2], rpx[1] + rpx[3]), tx["ALIGN"], col)
        if tag == "LBL_FILTER":
            info["overflow"] = draw_caption_overflow(img, cpm, rpx, s)
            info["caption_px"] = cpm.text_size(FILTER_FONT, s)[0]
            info["label_px"] = rpx[2]
    # rows
    lb = unit_px(cpm, by_tag["LB_ITEMS"]["RECT"])
    sx, sy = cpm.SX, cpm.SY
    row_w, row_h = int(PROTO[2] * sx), int(PROTO[3] * sy)
    rx, ry0 = int(PROTO[0] * sx), int(PROTO[1] * sy)
    n_vis = max(0, (lb[1] + lb[3] - ry0) // row_h)
    proto = {"FILL": "uibit_fill_2bt", "EDGE": "uibit_brdr_16bet", "CORNER": "uibit_brdr_16bct", "DIMENSION": 16}
    scroll = max(0, min(scroll, max(0, len(rows) - n_vis)))
    for i, e in enumerate(rows[scroll:scroll + n_vis]):
        it = items[e.item]
        r = (rx, ry0 + i * row_h, row_w, row_h)
        ftint, ttint = row_colours(e, i == 0 and scroll == 0)
        frame_px(img, cpm, proto, r, tint=ftint)
        isz = row_h - 2 * ICON_PAD
        ic = item_icon(cpm, qlm, it)
        if ic:
            qlm.draw_icon(img, cpm, ic, (r[0] + ICON_PAD, r[1] + ICON_PAD, isz, isz))
        if it.stack > 1:
            cpm.draw_text(img, "dialogfont10x10", str(it.stack), (r[0] + ICON_PAD, r[1] + ICON_PAD, r[0] + ICON_PAD + isz - 4, r[1] + ICON_PAD + isz - 2), 35, (1, 1, 1))
        cpm.draw_text(img, FILTER_FONT, it.name, (r[0] + row_h + 6, r[1], r[0] + r[2] - 16, r[1] + r[3]), 17, ttint)
    info["rows_visible"] = min(n_vis, len(rows) - scroll)
    info["rows_window"] = n_vis
    # scrollbar (SCROLLBAR EXTENT 268,134,16,329; 16ge/16gc DIMENSION 4, uibit_uparrowg, uibit_fill_16g thumb)
    sbx, sby, sbw, sbh = int(268 * sx), int(134 * sy), int(16 * sx), lb[3]
    frame_px(img, cpm, {"EDGE": "uibit_brdr_16ge", "CORNER": "uibit_brdr_16gc", "DIMENSION": 4}, (sbx, sby, sbw, sbh), tint=SB_THUMB_TINT)
    arrow = int(16 * sy)
    up = cpm.texture("uibit_uparrowg")
    if up is not None:
        cpm.paste_stretched(img, cpm.tint(up, SB_THUMB_TINT), (sbx, sby, sbx + sbw, sby + arrow))
        cpm.paste_stretched(img, cpm.tint(up.transpose(Image.FLIP_TOP_BOTTOM), SB_THUMB_TINT), (sbx, sby + sbh - arrow, sbx + sbw, sby + sbh))
    track = sbh - 2 * arrow - 8
    th = track if len(rows) <= n_vis else max(24, int(track * n_vis / len(rows)))
    toff = 0 if len(rows) <= n_vis else int((track - th) * scroll / (len(rows) - n_vis))
    qlm.draw_fill(img, cpm, "uibit_fill_16g", (sbx + 4, sby + arrow + 4 + toff, sbw - 8, th), SB_THUMB_TINT)
    # full-order strip under the panel (what scrolling reveals)
    names = []
    for e in rows:
        it = items[e.item]
        tag = ("*" if e.is_new else "") + ("!" if e.tint else "")
        names.append("%s%s%s" % (tag, it.name, " x%d" % it.stack if it.stack > 1 else ""))
    legend = "ISF 0.3.1-dev mock | filter: %s | sort_order: %s | %d of %d items, %d rows visible in LB_ITEMS | * = New, ! = cannot equip (tinted)" % (
        STATE_NAMES[state], SORT_NAMES[order], len(rows), total_all, info["rows_visible"])
    d = ImageDraw.Draw(img)
    d.rectangle([0, 1456, SCREEN_W, SCREEN_H], fill=(0, 0, 0, 255))   # the strip sits below the panel art, on black
    lh = 12.5 * cpm.tscale("dialogfont10x10")
    cpm.draw_text(img, "dialogfont10x10", legend, (40, 1460, SCREEN_W - 40, 1460 + lh), 9, CREAM)
    y = 1460 + lh
    order_txt = "Full order: " + " | ".join("%d %s" % (i + 1, n) for i, n in enumerate(names)) if names else "Full order: (no rows)"
    for ln in cpm.wrap(order_txt, "dialogfont10x10", SCREEN_W - 80)[:3]:
        cpm.draw_text(img, "dialogfont10x10", ln, (40, y, SCREEN_W - 40, y + lh), 9, TEAL_TEXT)
        y += lh
    info["order_text"] = order_txt
    if outpath:
        img.save(outpath)
    return img, info


def caption_zoom(imgs_by_state, worst, z=3):
    """The LBL_FILTER + tab strip area of the four state renders and the 1024-row worst case, each cropped and upscaled z x, stacked with labels."""
    from PIL import Image, ImageDraw, ImageFont
    cpm = _cpm()
    x0, x1 = 480, 2000
    y0, y1 = int(100 * cpm.SY), int(130 * cpm.SY)
    bands = []
    for label, im in imgs_by_state + [("Worst case (1024 rows)", worst)]:
        c = im.crop((x0, y0, x1, y1)).resize(((x1 - x0) * z, (y1 - y0) * z), Image.LANCZOS)
        bands.append((label, c))
    f = ImageFont.load_default(size=34)
    pad = 56
    sheet = Image.new("RGBA", (bands[0][1].width, sum(b.height + pad for _, b in bands)), (0, 0, 0, 255))
    d = ImageDraw.Draw(sheet)
    y = 0
    for label, c in bands:
        d.text((12, y + 8), label, fill=(235, 235, 235, 255), font=f)
        sheet.alpha_composite(c, (0, y + pad))
        y += c.height + pad
    return sheet


def contact_sheet(frames, scale=0.25):
    from PIL import Image, ImageDraw, ImageFont
    w, h = int(SCREEN_W * scale), int(SCREEN_H * scale)
    f = ImageFont.load_default(size=30)
    cols, pad = 2, 46
    rows = (len(frames) + cols - 1) // cols
    sheet = Image.new("RGBA", (cols * w + (cols + 1) * 12, rows * (h + pad) + 12), (0, 0, 0, 255))
    d = ImageDraw.Draw(sheet)
    for i, (label, im) in enumerate(frames):
        cx, cy = 12 + (i % cols) * (w + 12), 12 + (i // cols) * (h + pad)
        d.text((cx, cy + 6), label, fill=(235, 235, 235, 255), font=f)
        sheet.alpha_composite(im.resize((w, h), Image.LANCZOS), (cx, cy + pad - 4))
    return sheet


def fit_table(infos, cpm):
    label_px = int(72 * cpm.SX)
    lines = ["ISF inventory mock fit table (dialogfont16x16, 3840x1600, LBL_FILTER width %d px = 72 units)" % label_px, "",
             "caption text | px width | label width | fits"]
    for st in (S_ALL, S_NEW, S_USABLE, S_UPGRADE):
        rows = len(visible_entries(st, build_entries()))
        s = caption_with_count(st, ALL3, rows)
        w = cpm.text_size(FILTER_FONT, s)[0]
        lines.append("%-34s | %7.1f | %5d | %s   (state %s, %d rows)" % ('"%s"' % s, w, label_px, "yes" if w <= label_px else "NO (overflow)", STATE_NAMES[st], rows))
    s = caption_with_count(S_UPGRADE, ALL3, 1024)
    w = cpm.text_size(FILTER_FONT, s)[0]
    lines.append("%-34s | %7.1f | %5d | %s   (worst case, 1024 rows, %d chars)" % ('"%s"' % s, w, label_px, "yes" if w <= label_px else "NO (overflow)", len(s)))
    lines += ["", "render | rows visible in LB_ITEMS / rows in list / backpack items"]
    for name, info in infos:
        lines.append("%s | %d / %d / %d | window %d rows" % (name, info["rows_visible"], len(info["rows"]), len(SAMPLE), info["rows_window"]))
    lines += ["", "full row order per render (* New, ! cannot equip):"]
    for name, info in infos:
        lines.append("%s: %s" % (name, info["order_text"].replace("Full order: ", "")))
    return "\n".join(lines) + "\n"


def render_scrolled(outdir):
    """All / Category scrolled so the first tinted (cannot-equip) row is the 2nd visible one: shows the usable -> unusable boundary and the tint."""
    rows = arrange(S_ALL, SO_CATEGORY)
    first_tint = next(i for i, e in enumerate(rows) if e.tint)
    p = Path(outdir) / "isf_all_category_scrolled.png"
    render(S_ALL, SO_CATEGORY, outpath=p, scroll=max(0, first_tint - 1))
    return p


def render_all(outdir):
    cpm = _cpm()
    outdir = Path(outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    frames, infos, by_state = [], [], {}
    for name, st, so in RUNS:
        im, info = render(st, so, outpath=outdir / (name + ".png"))
        frames.append(("%s  (filter %s, sort %s)" % (name + ".png", STATE_NAMES[st], SORT_NAMES[so]), im))
        infos.append((name, info))
        if so == SO_CATEGORY:
            by_state[st] = im
    zoom_in = [("State %s: \"%s\"" % (STATE_NAMES[s], caption_with_count(s, ALL3, len(visible_entries(s, build_entries())))), by_state[s])
               for s in (S_ALL, S_NEW, S_USABLE, S_UPGRADE)]
    worst, _ = render(S_UPGRADE, SO_CATEGORY, rows_for_caption=1024)
    caption_zoom(zoom_in, worst).save(outdir / "isf_caption_zoom.png")
    contact_sheet(frames).save(outdir / "isf_sheet.png")
    render_scrolled(outdir)
    (outdir / "isf_fit_table.txt").write_text(fit_table(infos, cpm))
    return sorted(p.name for p in outdir.iterdir())


def main(argv):
    if len(argv) != 2:
        print(__doc__.split("usage:")[1].split("\n")[0].strip())
        return 2
    for n in render_all(argv[1]):
        print(n)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
