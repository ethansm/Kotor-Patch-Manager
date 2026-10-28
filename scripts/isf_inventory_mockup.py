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

usage: isf_inventory_mockup.py OUTDIR [--variant {current,v1,v2,v3,all}]   (default all)
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


# ---- redesign variants (offline mock only; nothing here is in the mod yet) ----------------------------------------------
VARIANTS = ("current", "v1", "v2", "v3")
VNAMES = {S_ALL: "All", S_NEW: "New", S_USABLE: "Can Equip", S_UPGRADE: "Upgradeable"}   # the Usable filter is renamed "Can Equip" (vanilla has its own Useable tab)
VCAPTION = {S_ALL: "Filter", S_NEW: "New", S_USABLE: "Can Equip", S_UPGRADE: "Upgradeable"}   # LBL_FILTER (345 px label)
SORT_CYCLE = (SO_CATEGORY, SO_BASE, SO_NAME, SO_VANILLA)                  # SortLogic.h SortOrder enum order = the click cycle
SORT_LABELS = {SO_CATEGORY: "Category", SO_BASE: "Base item", SO_NAME: "Name", SO_VANILLA: "Vanilla"}
FILTER_SEGMENTS = (S_ALL,) + CYCLE_ORDER                                    # All | New | Can Equip | Upgradeable
DIV_LABEL = {0: "Weapons", 1: "Weapons", 2: "Armour", 3: "Gear", 4: "Upgrades", 5: "Consumables", 6: "Other"}
NEW_DIV = "New"
# render states per variant: (file tag, filter state, sort order)
VSTATES = [("all_category", S_ALL, SO_CATEGORY), ("canequip_category", S_USABLE, SO_CATEGORY), ("new_name", S_NEW, SO_NAME),
           ("upgrade_category", S_UPGRADE, SO_CATEGORY)]


def next_sort(o):
    return SORT_CYCLE[(SORT_CYCLE.index(o) + 1) % len(SORT_CYCLE)]


def filter_counts(items=None):
    items = items if items is not None else SAMPLE
    es = build_entries(items)
    return {s: len(visible_entries(s, es, items)) for s in (S_ALL, S_NEW, S_USABLE, S_UPGRADE)}


def build_elements(rows, order, variant):
    """List of ("row", Entry) and, for v3 in Category sort only, ("div", label) category dividers (non-selectable, half a row high).
    A divider opens each run of the same (isNew, usable, category label): the New block gets one "New" divider, categories 0 and 1 share "Weapons"."""
    els, prev = [], None
    for e in rows:
        if variant == "v3" and order == SO_CATEGORY:
            key = (True, True, NEW_DIV) if e.is_new else (False, e.usable, DIV_LABEL[e.category])
            if key != prev:
                els.append(("div", key[2], key))
                prev = key
        els.append(("row", e))
    return els


def fit_count(hts, start, avail):
    n = used = 0
    for h in hts[start:]:
        if used + h > avail:
            break
        used += h
        n += 1
    return n


def max_scroll(hts, avail):
    for s0 in range(len(hts) + 1):
        if sum(hts[s0:]) <= avail:
            return s0
    return len(hts)


def scroll_to_unusable_divider(els, hts, avail, label="Armour"):
    """Scroll index that shows the previous row, the (unusable-block) divider `label` and the tinted rows below it."""
    i = next(i for i, k in enumerate(els) if k[0] == "div" and k[2][1] is False and k[2][2] == label)
    return min(max(0, i - 1), max_scroll(hts, avail))


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


PANEL_X = (163, 3676)           # inner panel span (LBL_BAR1/2 left edge, LBL_BAR4 right edge), px
PANEL_BOTTOM = 1452             # the frame art ends ~1456; the legend strip of this mock starts there
BTN_PAD = 28                    # text padding inside the added buttons (px); the 16 px frame is on top of it
BTN_FRAME = 16
SMALL = "dialogfont10x10"


def rects_overlap(a, b):
    return a[0] < b[0] + b[2] and b[0] < a[0] + a[2] and a[1] < b[1] + b[3] and b[1] < a[1] + a[3]


def rect_inside(a, b):
    return a[0] >= b[0] and a[1] >= b[1] and a[0] + a[2] <= b[0] + b[2] and a[1] + a[3] <= b[1] + b[3]


def free_x(rects, y0, y1, xmin, xmax):
    """Free x intervals of the horizontal strip y0..y1 (control rects in px subtracted)."""
    iv = [(xmin, xmax)]
    for (x, y, w, h) in rects:
        if y < y1 and y0 < y + h:
            out = []
            for a, b in iv:
                if x + w <= a or x >= b:
                    out.append((a, b))
                    continue
                if x > a:
                    out.append((a, x))
                if x + w < b:
                    out.append((x + w, b))
            iv = out
    return [(a, b) for a, b in iv if b - a > 0]


def free_report(cpm, ctls):
    """Free rects measured from the gui (px). Returns {"bands": {...}, "list": [(name, rect, used)]}. Band A = right of BTN_USEITEM under LBL_CREDITS,
    band B = the strip under LBL_BAR3/4 right of BTN_EXIT; both are bounded by the control rects (LBL_BAR5 starts right of band A)."""
    px = {c["TAG"]: unit_px(cpm, c["RECT"]) for c in ctls}
    rects = list(px.values())

    def bot(t):
        return px[t][1] + px[t][3]
    ay0, ay1 = bot("LBL_CREDITS") + 10, px["LBL_BAR3"][1] - 10
    by0, by1 = max(bot("LBL_BAR3"), bot("LBL_BAR4")) + 5, PANEL_BOTTOM
    a_iv = free_x(rects, ay0, ay1, PANEL_X[0], PANEL_X[1])
    b_iv = free_x(rects, by0, by1, PANEL_X[0], PANEL_X[1])
    a_main = max(a_iv, key=lambda t: t[1] - t[0] if t[0] > px["BTN_USEITEM"][0] else -1)
    b_main = max(b_iv, key=lambda t: t[1] - t[0])
    ma = 24
    band_a = (a_main[0] + ma, ay0, a_main[1] - a_main[0] - 2 * ma, ay1 - ay0)
    band_b = (b_main[0] + ma, by0, b_main[1] - b_main[0] - 2 * ma, by1 - by0)
    lst = [("A  right of BTN_USEITEM, under LBL_CREDITS, left of LBL_BAR5 (free x %d..%d)" % a_main, (a_main[0], ay0, a_main[1] - a_main[0], ay1 - ay0), "sort button (v2, v3)"),
           ("B  under LBL_BAR3/4, right of BTN_EXIT (free x %d..%d)" % b_main, (b_main[0], by0, b_main[1] - b_main[0], by1 - by0), "status + legend (v1); segmented filter + status + legend (v2, v3)")]
    for iv in a_iv:
        if iv != a_main and iv[1] - iv[0] > 100:
            lst.append(("A2 same strip, free x %d..%d (beside LBL_BAR5 / panel edge)" % iv, (iv[0], ay0, iv[1] - iv[0], ay1 - ay0), "unused"))
    d, cr = px["LB_DESCRIPTION"], px["LBL_CREDITS"]
    lst.append(("gap LB_DESCRIPTION bottom .. LBL_CREDITS top", (d[0], d[1] + d[3], d[2], cr[1] - d[1] - d[3]), "unused (%d px tall)" % (cr[1] - d[1] - d[3])))
    lst.append(("LB_DESCRIPTION interior below the text (a control rect, text uses the top ~25 %)", (d[0] + 24, d[1] + 360, d[2] - 48, d[3] - 384), "unused (control, not claimed)"))
    return {"band_a": band_a, "band_b": band_b, "list": lst, "px": px}


def variant_layout(cpm, variant, state, order, ctls, items=None, caption=None):
    """Geometry + fit checks of everything a variant adds (no drawing, needs the gui + font metrics only).
    Returns {"added": [{name, rect, kind}], "fits": [(name, text, text_px, box_px, ok)], "ops": [...], "overlaps": [str], "free": ...}."""
    items = items if items is not None else SAMPLE
    fr = free_report(cpm, ctls)
    px = fr["px"]
    L = {"variant": variant, "added": [], "fits": [], "ops": [], "free": fr["list"], "band_a": fr["band_a"], "band_b": fr["band_b"], "px": px}
    T, C = TEAL_TEXT, CREAM

    def tw(font, text):
        return cpm.text_size(font, text)[0]

    def fit(name, text, font, box_w):
        w = tw(font, text)
        L["fits"].append((name, text, w, box_w, w <= box_w))

    def add(name, rect, kind="band"):
        L["added"].append({"name": name, "rect": tuple(int(round(v)) for v in rect), "kind": kind})
    fit("LBL_FILTER caption", caption if caption is not None else VCAPTION[state], FILTER_FONT, px["LBL_FILTER"][2])
    counts = filter_counts(items)
    shown, total = counts[state], counts[S_ALL]
    bx, by, bw, bh = L["band_b"]
    x_end_limit = bx + bw
    x = bx
    if variant in ("v2", "v3"):
        ax, ay, aw, ah = L["band_a"]
        widest = max(tw(FILTER_FONT, "Sort: " + v) for v in SORT_LABELS.values())
        w = int(widest + 2 * (BTN_PAD + BTN_FRAME))
        rect = (ax, ay, min(w, aw), ah)
        add("Sort button", rect)
        segs = [("Sort: ", T), (SORT_LABELS[order], C)]
        fit("Sort button label (widest: Base item)", "Sort: Base item", FILTER_FONT, rect[2] - 2 * BTN_FRAME)
        fit("Sort button label (now)", "Sort: " + SORT_LABELS[order], FILTER_FONT, rect[2] - 2 * BTN_FRAME)
        L["ops"].append(("button", rect, segs, False))
        gap = 8
        for st in FILTER_SEGMENTS:
            text = "%s %d" % (VNAMES[st], counts[st])
            w = int(tw(FILTER_FONT, text) + 2 * BTN_PAD + 2 * BTN_FRAME)
            r = (x, by, w, bh)
            add("Filter segment " + VNAMES[st], r)
            fit("Filter segment " + VNAMES[st], text, FILTER_FONT, w - 2 * BTN_FRAME)
            L["ops"].append(("button", r, [(VNAMES[st] + " ", C if st == state else T), (str(counts[st]), C if st == state else T)], st == state))
            x += w + gap
        x += 52
    status = [("Sorted: ", T), (SORT_LABELS[order], C), ("   ", T), ("Showing: ", T), (VNAMES[state], C),
              ("  %d items" % total if state == S_ALL else "  %d of %d" % (shown, total), C)]
    sw = sum(tw(SMALL, t) for t, _ in status)
    th = cpm.text_size(SMALL, "A")[1]
    r = (x, by, int(sw) + 2, bh)
    add("Status line", r)
    L["fits"].append(("Status line (to end of band B)", "".join(t for t, _ in status), sw, x_end_limit - x, sw <= x_end_limit - x))
    L["ops"].append(("text", r, status))
    x += int(sw) + 80
    sq = 30
    cy = by + (bh - sq) // 2
    for nm, label, fr_c, fl_c in (("can't equip", "can't equip", TINT_COL, (0.20, 0.03, 0.0)), ("new", "new", NEW_FRAME, (0.16, 0.13, 0.02))):
        sr = (x, cy, sq, sq)
        add("Legend swatch " + nm, sr)
        L["ops"].append(("swatch", sr, fr_c, fl_c, NEW_TEXT if nm == "new" else TINT_COL))
        x += sq + 12
        lw = tw(SMALL, label)
        tr = (x, by, int(lw) + 2, bh)
        add("Legend text " + nm, tr)
        L["fits"].append(("Legend text " + nm, label, lw, x_end_limit - x, lw <= x_end_limit - x))
        L["ops"].append(("text", tr, [(label, T)]))
        x += int(lw) + 56
    L["fits"].append(("Band B used width (status + legend end)", "x %d" % (x - 56), x - 56, x_end_limit, x - 56 <= x_end_limit))
    L["text_h"] = th
    # v3 dividers: labels vs the row width left of the rule, rect inside LB_ITEMS
    if variant == "v3":
        lb = px["LB_ITEMS"]
        rx, ry0 = int(PROTO[0] * cpm.SX), int(PROTO[1] * cpm.SY)
        row_w, row_h = int(PROTO[2] * cpm.SX), int(PROTO[3] * cpm.SY)
        for lab in sorted(set(DIV_LABEL.values())) + [NEW_DIV]:
            fit("Divider '%s'" % lab, lab, SMALL, row_w - 2 * 14 - 80)       # leave >= 80 px for the rule
        add("Divider row", (rx, ry0, row_w, row_h // 2), "inlist")
    # overlaps: every added band rect vs every gui control and vs each other; in-list rects must sit inside LB_ITEMS
    bad = []
    bands = [a for a in L["added"] if a["kind"] == "band"]
    for a in bands:
        for tag, cr in px.items():
            if rects_overlap(a["rect"], cr):
                bad.append("%s overlaps %s %s" % (a["name"], tag, cr))
        if not (rect_inside(a["rect"], L["band_a"]) or rect_inside(a["rect"], L["band_b"])):
            bad.append("%s leaves its free band" % a["name"])
    for i, a in enumerate(bands):
        for b in bands[i + 1:]:
            if rects_overlap(a["rect"], b["rect"]):
                bad.append("%s overlaps %s" % (a["name"], b["name"]))
    for a in L["added"]:
        if a["kind"] == "inlist" and not rect_inside(a["rect"], px["LB_ITEMS"]):
            bad.append("%s leaves LB_ITEMS" % a["name"])
    L["overlaps"] = bad
    return L


def draw_segs(img, cpm, font, segs, rect, center):
    tot = sum(cpm.text_size(font, t)[0] for t, _ in segs)
    x = rect[0] + (rect[2] - tot) / 2 if center else rect[0]
    for t, col in segs:
        w = cpm.text_size(font, t)[0]
        cpm.draw_text(img, font, t, (x, rect[1], x + w + 2, rect[1] + rect[3]), 17, col)
        x += w


def draw_variant(img, cpm, L):
    from PIL import ImageDraw
    d = ImageDraw.Draw(img)
    by_tag = {}
    for op in L["ops"]:
        if op[0] == "button":
            _, rect, segs, active = op
            b = BTN_BORDER_ACTIVE if active else BTN_BORDER
            if active:
                d.rectangle([rect[0] + 8, rect[1] + 8, rect[0] + rect[2] - 8, rect[1] + rect[3] - 8], fill=(int(TEAL_FRAME[0] * 255 * 0.7), int(TEAL_FRAME[1] * 255 * 0.7), int(TEAL_FRAME[2] * 255 * 0.7), 255))
            frame_px(img, cpm, b, rect)
            draw_segs(img, cpm, FILTER_FONT, segs, rect, True)
        elif op[0] == "text":
            draw_segs(img, cpm, SMALL, op[2], op[1], False)
        elif op[0] == "swatch":
            _, r, fr_c, fl_c, _t = op
            d.rectangle([r[0], r[1], r[0] + r[2] - 1, r[1] + r[3] - 1], fill=tuple(int(v * 255) for v in fl_c) + (255,), outline=tuple(int(v * 255) for v in fr_c) + (255,), width=3)
            d.rectangle([r[0] + 8, r[1] + r[3] // 2 - 2, r[0] + r[2] - 9, r[1] + r[3] // 2 + 2], fill=tuple(int(v * 255) for v in _t) + (255,))


BTN_BORDER = {"FILL": "", "EDGE": "uibit_brdr_16we", "CORNER": "uibit_brdr_16wc", "DIMENSION": 16, "COLOR": TEAL_FRAME}   # BTN_USEITEM BORDER
BTN_BORDER_ACTIVE = dict(BTN_BORDER, COLOR=CREAM)                                                                      # its HILIGHT colour


def draw_divider(img, cpm, rect, label):
    """Category divider row (v3): cream small-caps label, a thin teal rule to the right edge."""
    from PIL import ImageDraw
    x, y, w, h = rect
    cpm.draw_text(img, SMALL, label, (x + 14, y, x + w - 14, y + h), 17, CREAM)
    tx = x + 14 + int(cpm.text_size(SMALL, label)[0]) + 20
    cy = y + h // 2
    ImageDraw.Draw(img).rectangle([tx, cy - 1, x + w - 14, cy + 1], fill=tuple(int(v * 255) for v in TEAL_TEXT) + (255,))


def render(state, order, rows_for_caption=None, outpath=None, items=None, scroll=0, variant="current"):
    """One full 3840x1600 frame. Returns (PIL image, info dict). variant: "current" (the shipped ISF 0.3.1 UX) or a redesign v1/v2/v3 (see VARIANTS)."""
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
    if variant != "current":
        caption = VCAPTION[state]
    sel = items[rows[0].item] if rows else None
    info = {"caption": caption, "rows": rows, "overflow": False, "variant": variant}
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
            if variant != "current" and state != S_ALL:
                col = CREAM        # active-filter look: the caption takes the highlighted (cream) colour
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
    # rows (+ v3 category dividers); heights in px, whole elements only
    lb = unit_px(cpm, by_tag["LB_ITEMS"]["RECT"])
    sx, sy = cpm.SX, cpm.SY
    row_w, row_h = int(PROTO[2] * sx), int(PROTO[3] * sy)
    rx, ry0 = int(PROTO[0] * sx), int(PROTO[1] * sy)
    avail = lb[1] + lb[3] - ry0
    els = build_elements(rows, order, variant)
    hts = [row_h if k[0] == "row" else row_h // 2 for k in els]
    n_vis = avail // row_h                 # window capacity in whole rows (reported)
    n_top = fit_count(hts, 0, avail)        # elements visible from the top (scrollbar thumb)
    scroll = max(0, min(scroll, max_scroll(hts, avail)))
    proto = {"FILL": "uibit_fill_2bt", "EDGE": "uibit_brdr_16bet", "CORNER": "uibit_brdr_16bct", "DIMENSION": 16}
    y = ry0
    shown_rows = 0
    for k in els[scroll:scroll + fit_count(hts, scroll, avail)]:
        if k[0] == "div":
            draw_divider(img, cpm, (rx, y, row_w, row_h // 2), k[1])
            y += row_h // 2
            continue
        e = k[1]
        it = items[e.item]
        r = (rx, y, row_w, row_h)
        y += row_h
        shown_rows += 1
        ftint, ttint = row_colours(e, e is rows[0])
        frame_px(img, cpm, proto, r, tint=ftint)
        isz = row_h - 2 * ICON_PAD
        ic = item_icon(cpm, qlm, it)
        if ic:
            qlm.draw_icon(img, cpm, ic, (r[0] + ICON_PAD, r[1] + ICON_PAD, isz, isz))
        if it.stack > 1:
            cpm.draw_text(img, "dialogfont10x10", str(it.stack), (r[0] + ICON_PAD, r[1] + ICON_PAD, r[0] + ICON_PAD + isz - 4, r[1] + ICON_PAD + isz - 2), 35, (1, 1, 1))
        cpm.draw_text(img, FILTER_FONT, it.name, (r[0] + row_h + 6, r[1], r[0] + r[2] - 16, r[1] + r[3]), 17, ttint)
    info["rows_visible"] = shown_rows
    info["rows_window"] = n_vis
    info["scroll"] = scroll
    # scrollbar (SCROLLBAR EXTENT 268,134,16,329; 16ge/16gc DIMENSION 4, uibit_uparrowg, uibit_fill_16g thumb)
    sbx, sby, sbw, sbh = int(268 * sx), int(134 * sy), int(16 * sx), lb[3]
    frame_px(img, cpm, {"EDGE": "uibit_brdr_16ge", "CORNER": "uibit_brdr_16gc", "DIMENSION": 4}, (sbx, sby, sbw, sbh), tint=SB_THUMB_TINT)
    arrow = int(16 * sy)
    up = cpm.texture("uibit_uparrowg")
    if up is not None:
        cpm.paste_stretched(img, cpm.tint(up, SB_THUMB_TINT), (sbx, sby, sbx + sbw, sby + arrow))
        cpm.paste_stretched(img, cpm.tint(up.transpose(Image.FLIP_TOP_BOTTOM), SB_THUMB_TINT), (sbx, sby + sbh - arrow, sbx + sbw, sby + sbh))
    track = sbh - 2 * arrow - 8
    th = track if len(els) <= n_top else max(24, int(track * n_top / len(els)))
    toff = 0 if len(els) <= n_top else int((track - th) * scroll / max(1, len(els) - n_top))
    qlm.draw_fill(img, cpm, "uibit_fill_16g", (sbx + 4, sby + arrow + 4 + toff, sbw - 8, th), SB_THUMB_TINT)
    if variant != "current":
        info["layout"] = variant_layout(cpm, variant, state, order, ctls, items, caption)
        draw_variant(img, cpm, info["layout"])
    # full-order strip under the panel (what scrolling reveals)
    names = []
    for e in rows:
        it = items[e.item]
        tag = ("*" if e.is_new else "") + ("!" if e.tint else "")
        names.append("%s%s%s" % (tag, it.name, " x%d" % it.stack if it.stack > 1 else ""))
    legend = "ISF 0.3.1-dev mock | filter: %s | sort_order: %s | %d of %d items, %d rows visible in LB_ITEMS | * = New, ! = cannot equip (tinted)" % (
        (STATE_NAMES if variant == "current" else VNAMES)[state], SORT_NAMES[order], len(rows), total_all, info["rows_visible"])
    if variant != "current":
        legend = legend.replace("ISF 0.3.1-dev mock", "ISF redesign mock %s" % variant)
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


def v3_scroll(cpm):
    """List scroll (in elements) for the second v3 frame: All + Category showing the unusable-block "Armour" divider and the tinted rows under it."""
    _, ctls = load_gui(cpm)
    lb = unit_px(cpm, next(c for c in ctls if c["TAG"] == "LB_ITEMS")["RECT"])
    row_h, ry0 = int(PROTO[3] * cpm.SY), int(PROTO[1] * cpm.SY)
    els = build_elements(arrange(S_ALL, SO_CATEGORY), SO_CATEGORY, "v3")
    hts = [row_h if k[0] == "row" else row_h // 2 for k in els]
    return scroll_to_unusable_divider(els, hts, lb[1] + lb[3] - ry0)


def variant_fit_table(vinfos, cpm):
    """Fit section for the redesign variants: free rects measured from the gui, then per variant / state every added element's text px vs box px
    and the overlap check (an added rect overlapping a gui control or another added rect = FAIL)."""
    lines = ["", "=" * 100, "REDESIGN VARIANTS (v1 status line, v2 visible controls, v3 in-list cues); fonts dialogfont10x10 / dialogfont16x16, 3840x1600", "=" * 100, ""]
    shown = False
    for name, info in vinfos:
        L = info["layout"]
        if not shown:
            lines.append("free rects measured from inventory_p.gui (px; x, y, w, h):")
            for nm, r, use in L["free"]:
                lines.append("  %-92s %s  -> %s" % (nm, r, use))
            lines.append("  band A box used: %s   band B box used: %s" % (L["band_a"], L["band_b"]))
            lines.append("")
            shown = True
        lines.append("%s:" % name)
        for nm, text, w, box, ok in L["fits"]:
            lines.append("  %-44s | %7.1f px text | %6.1f px box | %s | \"%s\"" % (nm, w, box, "OK" if ok else "FAIL (overflow)", text))
        for a in L["added"]:
            lines.append("  rect %-28s %s (%s)" % (a["name"], a["rect"], a["kind"]))
        lines.append("  overlap check vs %d gui controls + added rects: %s" % (len(L["px"]), "OK (none)" if not L["overlaps"] else "FAIL: " + "; ".join(L["overlaps"])))
        lines.append("  rows visible %d of %d (window %d, scroll %d)" % (info["rows_visible"], len(info["rows"]), info["rows_window"], info.get("scroll", 0)))
    allok = all(ok for _, i in vinfos for *_x, ok in i["layout"]["fits"]) and not any(i["layout"]["overlaps"] for _, i in vinfos)
    lines += ["", "VARIANT FIT SUMMARY: %s" % ("PASS (all added elements fit, no overlaps)" if allok else "FAIL")]
    return "\n".join(lines) + "\n", allok


def variants_compare(frames, cols_w=1920):
    """2x2 grid of the four variants (Can Equip + Category), each tile cols_w px wide, labelled."""
    from PIL import Image, ImageDraw, ImageFont
    h = int(1456 * cols_w / SCREEN_W)
    f = ImageFont.load_default(size=44)
    pad, gap = 64, 16
    sheet = Image.new("RGBA", (2 * cols_w + 3 * gap, 2 * (h + pad) + gap), (0, 0, 0, 255))
    d = ImageDraw.Draw(sheet)
    for i, (label, im) in enumerate(frames):
        x, y = gap + (i % 2) * (cols_w + gap), gap // 2 + (i // 2) * (h + pad)
        d.text((x + 8, y + 8), label, fill=(240, 240, 240, 255), font=f)
        sheet.alpha_composite(im.crop((0, 0, SCREEN_W, 1456)).resize((cols_w, h), Image.LANCZOS), (x, y + pad))
    return sheet


def variants_zoom(frames, z=2):
    """Per variant (stacked): tab strip + LB_ITEMS + free band B, each cropped from the full frame and upscaled z x."""
    from PIL import Image, ImageDraw, ImageFont
    crops = [("tabs", (690, 270, 1700, 345)), ("list", (220, 350, 1372, 1240)), ("bands", (660, 1245, 3260, 1456))]
    f = ImageFont.load_default(size=44)
    blocks = []
    for label, im in frames:
        parts = [im.crop(b).resize(((b[2] - b[0]) * z, (b[3] - b[1]) * z), Image.LANCZOS) for _, b in crops]
        blocks.append((label, parts))
    w = max(p.width for _, parts in blocks for p in parts)
    hh = sum(sum(p.height for p in parts) + 60 for _, parts in blocks)
    sheet = Image.new("RGBA", (w, hh), (0, 0, 0, 255))
    d = ImageDraw.Draw(sheet)
    y = 0
    for label, parts in blocks:
        d.text((12, y + 8), label, fill=(240, 240, 240, 255), font=f)
        y += 60
        for p in parts:
            sheet.alpha_composite(p, (0, y))
            y += p.height
    return sheet.convert("RGB")


def render_variants(outdir, variants):
    """Renders every VSTATES frame per variant (+ the v3 scrolled frame). Returns (vinfos [(name, info)], frames {(variant, tag): image})."""
    cpm = _cpm()
    vinfos, frames = [], {}
    for v in variants:
        for tag, st, so in VSTATES:
            im, info = render(st, so, outpath=outdir / ("isf_%s_%s.png" % (v, tag)), variant=v)
            frames[(v, tag)] = im
            vinfos.append(("%s / %s + %s" % (v, VNAMES[st], SORT_LABELS[so]), info) if v != "current" else None)
        if v == "v3":
            im, info = render(S_ALL, SO_CATEGORY, outpath=outdir / "isf_v3_all_category_scrolled.png", variant=v, scroll=v3_scroll(cpm))
            frames[(v, "all_category_scrolled")] = im
            vinfos.append(("v3 / All + Category, scrolled to the Armour divider + tinted rows", info))
    return [x for x in vinfos if x], frames


def render_all(outdir, variant="all"):
    cpm = _cpm()
    outdir = Path(outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    text = ""
    if variant in ("current", "all"):
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
        text = fit_table(infos, cpm)
    vs = [v for v in VARIANTS if v != "current" and variant in (v, "all")]
    allfr = {}
    if variant in ("current", "all"):
        _, fr = render_variants(outdir, ["current"])      # isf_current_<state>.png: the same four states as the redesigns, shipped UX
        allfr.update(fr)
    if vs:
        vinfos, fr = render_variants(outdir, vs)
        allfr.update(fr)
        t, ok = variant_fit_table(vinfos, cpm)
        text += t
    if variant == "all":
        order = [("current (ISF 0.3.1-dev, shipped)", "current"), ("v1 status line", "v1"), ("v2 visible controls", "v2"), ("v3 in-list cues", "v3")]
        variants_compare([(lab + "   |   Can Equip + Category", allfr[(v, "canequip_category")]) for lab, v in order]).convert("RGB").save(outdir / "isf_variants_compare.png")
        variants_zoom([(lab + "   |   Can Equip + Category", allfr[(v, "canequip_category")]) for lab, v in order]).save(outdir / "isf_variants_zoom.png")
    (outdir / "isf_fit_table.txt").write_text(text)
    return sorted(p.name for p in outdir.iterdir())


def main(argv):
    args = argv[1:]
    variant = "all"
    if "--variant" in args:
        i = args.index("--variant")
        if i + 1 >= len(args) or args[i + 1] not in VARIANTS + ("all",):
            print("--variant needs one of: current, v1, v2, v3, all")
            return 2
        variant = args[i + 1]
        del args[i:i + 2]
    if len(args) != 1:
        print(__doc__.split("usage:")[1].split("\n")[0].strip())
        return 2
    for n in render_all(args[0], variant):
        print(n)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
