#!/usr/bin/env python3
"""Quick Loot tooltip redesign mockup (doc 52 QL9, design only; the mod is not changed by this script).

usage: quick_loot_tip_mockup.py <outdir>

Renders the proposed tooltip next to a hovered list row for real items from the live install (read-only, pykotor + dialog.tlk + 2DA):
  header   icon + title (cream, dialogfont16x16) + one dim category line (baseitems name; the description's tag line, e.g. "Not Upgradeable")
  rules    two columns, label dim teal / value cream: computed from baseitems.2da (damage dice + type, critical, range, defense, max Dex) and the
           item's PropertiesList (itempropdef name + subtype + cost table value), plus the "Label: value" lines the description itself starts with
  flavor   the first one or two sentences of the description (teal, at most 3 lines)
  footer   dim key hints
Names and text drop BioWare's {tag} prefixes ("{08}Disruptor Carbine" -> "Disruptor Carbine"), as the engine does.
Outputs ql09_tip_<n>.png (zoomed) and ql09_sheet.png. Before / after: ql09_before_after.png (0.6.0 tooltip vs the new one, same item).
"""
import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import quick_loot_mockup as qm  # noqa: E402

ITEMS = ["w_brifle_08", "g_w_stungren01", "a_light_04", "w_melee_10", "g_i_medeqpmnt01", "w_brifle_27"]
TIP_W = 660
PAD_X, PAD_Y = 26, 20
ICON = 72
TITLE_H, SUB_H, LINE_H, GAP = 34, 28, 30, 12
LABEL_COL_MAX = 230        # px cap of the label column (measured from the longest label, + 24)
LABEL_ALIAS = {"Damage Immunity": "Immunity", "Defense Penalty": "Penalty", "User Limitation": "Only for", "Max Dex Bonus": "Max Dex",
               "Activate Item": "Use", "Area of Effect": "Area", "Save (Will)": "Will Save", "Save (Reflex)": "Reflex Save", "Save (Fort)": "Fort Save",
               "Bonus Feat": "Feat", "Damage Bonus": "Bonus Dmg", "Attack Bonus": "Attack", "Ability Bonus": "Ability", "Skill Bonus": "Skill"}
FLAVOR_LINES = 3
DIVIDER_TINT = qm.TEAL_FRAME
TAG_RE = re.compile(r"\{[^{}]{0,12}\}")


def strip_tags(s):
    """{...} developer tags out, blanks collapsed (engine behaviour for TLK strings)."""
    return re.sub(r"[ \t]{2,}", " ", TAG_RE.sub("", s or "")).strip()


# ---- data (install, read-only)
_C = {}


def _inst():
    if "inst" not in _C:
        cpm = qm._cpm()
        _C["cpm"], _C["inst"] = cpm, cpm.inst()
        _C["tlk"] = _C["inst"].talktable()
    return _C["inst"]


def twoda(name):
    from pykotor.resource.formats.twoda import read_2da
    from pykotor.resource.type import ResourceType
    key = "2da:" + name.lower()
    if key not in _C:
        r = _inst().resource(name.lower(), ResourceType.TwoDA)
        _C[key] = read_2da(r.data) if r else None
    return _C[key]


def tlk(ref):
    try:
        ref = int(ref)
    except (TypeError, ValueError):
        return ""
    return strip_tags(_C["tlk"].string(ref)) if ref >= 0 else ""


def cell(t, row, col):
    try:
        v = t.get_row(row).get_string(col)
    except Exception:
        return ""
    return "" if v in ("", "****") else v


DMG_TYPES = [(0x10, "Energy"), (0x01, "Bludgeoning"), (0x02, "Piercing"), (0x04, "Slashing"), (0x80, "Sonic"), (0x20, "Ion"), (0x08, "Universal")]


def base_rules(base):
    """Computed lines from baseitems.2da (what the vanilla equip screen lists for weapons / armor)."""
    b = twoda("baseitems")
    out = []
    nd, die = cell(b, base, "numdice"), cell(b, base, "dietoroll")
    if nd and die and int(nd) > 0:
        flags = int(cell(b, base, "damageflags") or 0)
        kinds = [n for bit, n in DMG_TYPES if flags & bit]
        out.append(("Damage", f"{nd}d{die}" + (f" {'/'.join(kinds)}" if kinds else "")))
        ct, cm = int(cell(b, base, "critthreat") or 1), int(cell(b, base, "crithitmult") or 2)
        out.append(("Critical", (f"{21 - ct}-20" if ct > 1 else "20") + f" / x{cm}"))
        if cell(b, base, "rangedweapon") and cell(b, base, "rangedweapon") not in ("0",):
            mr = cell(b, base, "maxattackrange")
            out.append(("Range", f"{int(float(mr))} m" if mr else "Ranged"))
        else:
            out.append(("Range", "Melee"))
    ac = cell(b, base, "baseac")
    if ac and int(ac) > 0:
        out.append(("Defense", f"+{ac}"))
        dx = cell(b, base, "dexbonus")
        if dx and int(dx) < 99:
            out.append(("Max Dex Bonus", f"+{dx}"))
    return out


def prop_rules(uti):
    """One line per item property: itempropdef name + subtype name + cost table value name."""
    ipd, cts = twoda("itempropdef"), twoda("iprp_costtable")
    out = []
    for p in uti.get_list("PropertiesList"):
        pn = p.get_uint16("PropertyName")
        name = tlk(cell(ipd, pn, "name")) or cell(ipd, pn, "label")
        sub_t = cell(ipd, pn, "subtyperesref")
        sub = ""
        if sub_t:
            st = twoda(sub_t)
            if st:
                sub = tlk(cell(st, p.get_uint16("Subtype"), "name")) or cell(st, p.get_uint16("Subtype"), "label")
        val = ""
        ctn = cell(cts, p.get_uint8("CostTable"), "name") if p.get_uint8("CostTable") else ""
        if ctn:
            ct = twoda(ctn)
            if ct:
                val = tlk(cell(ct, p.get_uint16("CostValue"), "name")) or cell(ct, p.get_uint16("CostValue"), "label")
        v = " ".join(x for x in (sub, val) if x)
        out.append((name, v or "Yes"))
    return out


def split_desc(desc):
    """(tag, stat lines [(label, value)], flavor) from DescIdentified: a short first line without ':' is the tag ("Not Upgradeable");
    leading "Label: value" lines (a bare "Label:" takes the next line as its value) are rules; the rest is flavor."""
    lines = [strip_tags(ln) for ln in (desc or "").replace("\r", "").split("\n")]
    i, tag, stats = 0, "", []
    while i < len(lines) and not lines[i]:
        i += 1
    if i < len(lines) and ":" not in lines[i] and len(lines[i]) <= 32 and not lines[i].endswith("."):
        tag, i = lines[i], i + 1
    while i < len(lines) and lines[i]:
        ln = lines[i]
        if ":" not in ln:
            break
        k, v = ln.split(":", 1)
        v = v.strip()
        if not v and i + 1 < len(lines) and lines[i + 1] and ":" not in lines[i + 1]:
            v, i = lines[i + 1], i + 1
        stats.append((k.strip(), v))
        i += 1
    flavor = " ".join(ln for ln in lines[i:] if ln)
    return tag, stats, flavor


def short_flavor(text, max_chars=170):
    """First sentence; a second one when the first is short; never cut mid-sentence unless one sentence is longer than max_chars."""
    sents = re.findall(r"[^.!?]+[.!?]+(?:\s|$)", text + " ")
    if not sents:
        return text[:max_chars]
    out = sents[0].strip()
    if len(out) < 70 and len(sents) > 1 and len(out) + len(sents[1]) <= max_chars:
        out += " " + sents[1].strip()
    if len(out) > max_chars:
        out = out[:max_chars - 3].rsplit(" ", 1)[0] + "..."
    return out


def item(resref):
    from pykotor.resource.formats.gff import read_gff
    from pykotor.resource.type import ResourceType
    r = _inst().resource(resref, ResourceType.UTI)
    if not r:
        return None
    g = read_gff(r.data).root
    base = g.get_int32("BaseItem")
    raw_desc = _C["tlk"].string(g.get_locstring("DescIdentified").stringref)
    tag, stats, flavor = split_desc(raw_desc)
    rules = base_rules(base) + prop_rules(g) + stats
    if tag:
        up = tag.replace("Fully Upgradeable", "Full").replace("Not Upgradeable", "None").replace("Upgradeable", "").strip(" ()") or "Yes"
        rules.append(("Upgrades", up))
    merged = []
    for k, v in rules:
        k = LABEL_ALIAS.get(k, k)
        hit = next((m for m in merged if m[0] == k), None)
        if hit and k not in ("Damage",):
            a, b = hit[1].rsplit(" ", 1) if " " in hit[1] else (hit[1], ""), v.rsplit(" ", 1) if " " in v else (v, "")
            hit[1] = f"{a[0]}, {b[0]} {a[1]}".strip() if a[1] == b[1] and a[1] else f"{hit[1]}, {v}"
        else:
            merged.append([k, v])
    b = twoda("baseitems")
    cat = tlk(cell(b, base, "name"))
    ic = cell(b, base, "itemclass")
    var = g.get_uint8("ModelVariation") if g.exists("ModelVariation") else 1
    return dict(resref=resref, name=strip_tags(_C["tlk"].string(g.get_locstring("LocalizedName").stringref)),
                raw_name=_C["tlk"].string(g.get_locstring("LocalizedName").stringref), raw_desc=raw_desc,
                sub=cat, rules=[tuple(m) for m in merged],
                flavor=short_flavor(flavor), icon=qm.icon_resref(ic, var or 1) if ic else "", stack=g.get_uint16("StackSize") if g.exists("StackSize") else 1)


# ---- layout + draw
def wrap(text, px):
    words, lines, cur = text.split(), [], ""
    for w in words:
        t = (cur + " " + w).strip()
        if qm.text_px10(t) <= px or not cur:
            cur = t
        else:
            lines.append(cur)
            cur = w
    if cur:
        lines.append(cur)
    if len(lines) > FLAVOR_LINES:
        lines = lines[:FLAVOR_LINES]
        lines[-1] = lines[-1].rstrip(".,;") + "..."
    return lines


def layout(it, x, y):
    inner = TIP_W - 2 * PAD_X
    flav = wrap(it["flavor"], inner) if it["flavor"] else []
    col = min(LABEL_COL_MAX, max((qm.text_px10(k) for k, _ in it["rules"]), default=0) + 24)
    rules = []
    for k, v in it["rules"]:
        vl = wrap(v, inner - col) or [""]
        rules.append((k, vl))
    n_rule_lines = sum(len(v) for _, v in rules)
    h = PAD_Y + max(ICON, TITLE_H + SUB_H) + GAP
    if rules:
        h += 2 + GAP + n_rule_lines * LINE_H + GAP
    if flav:
        h += 2 + GAP + len(flav) * LINE_H + GAP
    h += 2 + GAP + LINE_H + PAD_Y - GAP
    return dict(frame=(x, y, TIP_W, h), flav=flav, rules=rules, col=col)


def draw(img, cpm, it, L):
    x, y, w, h = L["frame"]
    qm.draw_frame(img, cpm, L["frame"])
    cy = y + PAD_Y
    ix = x + PAD_X
    qm.draw_icon(img, cpm, it["icon"], (ix, cy, ICON, ICON))
    tx = ix + ICON + 16

    def text(s, r, col, font=qm.FONT):
        cpm.draw_text(img, font, s, (r[0], r[1], r[0] + r[2], r[1] + r[3]), qm.TEXT_ALIGN, col)

    title = it["name"] + (f"  x{it['stack']}" if it["stack"] > 1 else "")
    text(title, (tx, cy + 4, w - (tx - x) - PAD_X, TITLE_H), qm.CREAM, "dialogfont16x16")
    text(it["sub"], (tx, cy + 4 + TITLE_H, w - (tx - x) - PAD_X, SUB_H), qm.TEAL_DIM)
    cy += max(ICON, TITLE_H + SUB_H) + GAP

    def divider():
        nonlocal cy
        qm.draw_fill(img, cpm, qm.TEX_FILL_16G, (x + PAD_X, cy, w - 2 * PAD_X, 2), DIVIDER_TINT)
        cy += 2 + GAP

    if L["rules"]:
        divider()
        for k, vl in L["rules"]:
            col = L["col"]
            text(k, (ix, cy, col - 8, LINE_H), qm.TEAL_DIM)
            for v in vl:
                text(v, (ix + col, cy, w - 2 * PAD_X - col, LINE_H), qm.CREAM)
                cy += LINE_H
        cy += GAP
    if L["flav"]:
        divider()
        for ln in L["flav"]:
            text(ln, (ix, cy, w - 2 * PAD_X, LINE_H), qm.TEAL_TEXT)
            cy += LINE_H
        cy += GAP
    divider()
    text("Click: Take     F: Take All", (ix, cy, w - 2 * PAD_X, LINE_H), qm.TEAL_DIM)


def scene(cpm, items, hover, it):
    """Block with the given rows, row `hover` highlighted, the new tooltip beside it."""
    img = qm.background()
    names = [i["name"] for i in items]
    icons = [i["icon"] for i in items]
    d = qm.ListData(names, [i["stack"] for i in items], icons=icons)
    o = qm.build_list_draw(d, qm.ListView(d.rows_total(), 8), False)
    name = (qm.MENU_ORIGIN[0] + qm.NAME_LOCAL[0], qm.MENU_ORIGIN[1] + qm.NAME_LOCAL[1], qm.NAME_LOCAL[2], qm.NAME_LOCAL[3])
    qm.draw_target_tag(img, cpm, qm.MENU_ORIGIN)
    rect = qm.anchor_block(0, True, name, qm.PANEL, None, o["w"], o["h"])[0]
    qm.draw_block(img, cpm, o, rect, hover)
    rr = qm.row_rect(hover, o["w"], o["sb"], o["row_h"])
    L = layout(it, rect[0] + rect[2] + qm.TIP_GAP, rect[1] + rr[1])
    draw(img, cpm, it, L)
    fx, fy, fw, fh = L["frame"]
    box = (min(rect[0], fx), min(rect[1], fy) - 120, max(rect[0] + rect[2], fx + fw) - min(rect[0], fx), max(rect[1] + rect[3], fy + fh) - min(rect[1], fy) + 120)
    return qm.zoom(img, box, 1, 30), L


def main():
    if len(sys.argv) != 2 or sys.argv[1].startswith("-"):
        print(__doc__.split("usage:")[1].split("\n")[1 - 1].strip()); return 2
    try:
        import PIL  # noqa: F401
    except ImportError:
        venv = Path(__file__).resolve().parent.parent / ".venv/bin/python"
        if venv.exists():
            os.execv(str(venv), [str(venv), os.path.abspath(__file__)] + sys.argv[1:])
        print("PIL missing"); return 1
    from PIL import Image
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    cpm = qm._cpm()
    _inst()
    items = [i for i in (item(r) for r in ITEMS) if i]
    shots = []
    for k, it in enumerate(items):
        print(f"{it['resref']}: {it['raw_name']!r} -> {it['name']!r} | sub={it['sub']!r}")
        for r in it["rules"]:
            print("   ", r)
        print("    flavor:", it["flavor"])
        z, _ = scene(cpm, items[:4] if k < 4 else items[2:], k if k < 4 else k - 2, it)
        p = out / f"ql09_tip_{k + 1}.png"
        z.save(p)
        shots.append(z)
        print("   ", p)
    W = max(s.width for s in shots)
    sheet = Image.new("RGBA", (W * 2 + 8, sum(s.height for s in shots[::2]) + 8 * 3), (0, 0, 0, 255))
    yy = [0, 0]
    for i, s in enumerate(shots):
        c = i % 2
        sheet.alpha_composite(s, (c * (W + 8), yy[c]))
        yy[c] += s.height + 8
    p = out / "ql09_sheet.png"
    sheet.save(p)
    print(p)
    return 0


if __name__ == "__main__":
    sys.exit(main())
