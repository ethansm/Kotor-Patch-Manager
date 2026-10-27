#!/usr/bin/env python3
"""Render the proposed "Stats" tab for the character page + two tooltip mock-ups (doc 51, offline).

Reuses character_page_mockup.py (cpm) for the real GUI: load_gui() reads the LIVE override/character_p.gui and
render() draws it at 3840x1600 with the real BORDER textures and the dialogfont16x16/10x10 bitmap fonts.
Nothing edits the live GUI; the Stats tab is a layout mock-up only.

Geometry: GUI units are 800x600 (screen px = units * (4.8, 8/3)). The tab takes the effects panel rect
cpm.PANEL = (536,134,216,336) with the header cpm.HEADER reading "Stats | Effects". Inside the panel, from the top
(PAD 8 units): four sections (DEFENSE, OFFENSE, FORCE, CHARACTER), each a 20-unit teal band, a 2-unit gap, then
18-unit rows (label left in TEAL_TEXT, value right in CREAM), with a 6-unit gap between sections. A row uses
dialogfont16x16 when label + value fit, else dialogfont10x10.

Sample values: combat_stats_test vector a = AJD + Force Redirection, BAB 10.

Tooltips mirror the reference in character_page_mockup.py (pad 5 units, flipped above the row when it would pass
the panel bottom). The vanilla tooltip sits right of LBL_DEFENSE_STAT on the unchanged live page, clamped to 800x600.

usage: stats_page_mockup.py OUTDIR
Outputs: stats_tab.png, stats_tab_tip_deflection.png, vanilla_tip_defense.png,
         stats_tab_zoom.png, stats_tab_tip_deflection_zoom.png, vanilla_tip_defense_zoom.png
"""
import argparse
import copy
import sys
from pathlib import Path

from PIL import ImageDraw

sys.path.insert(0, str(Path(__file__).parent))
import character_page_mockup as cpm  # noqa: E402

PANEL = cpm.PANEL
PAD = 8          # top padding inside the panel (units)
SEC_H = 20       # section band height (units)
SEC_GAP = 6      # gap between sections (units)
ROW_H = 18       # row height (units)
EDGE = 8         # label/value inset from the panel sides (units)
VAL_GAP = 6      # minimum gap between label and value (units)
TIP_PAD = 5      # tooltip inner pad (units)
TIP_W = PANEL[2] - 8
ROW_W_PX = (PANEL[2] - 2 * EDGE) * cpm.SX
GAP_PX = VAL_GAP * cpm.SX
TAB_CROP = (520, 90, 770, 480)     # zoom box as (x0, y0, x1, y1) units
VANILLA_CROP = (40, 290, 480, 480)  # zoom box, bottom extended below if the tooltip needs it

# sample: combat_stats_test vector a = AJD + Force Redirection, BAB 10
SAMPLE = [
    ("DEFENSE", [("Deflection", "16"), ("Defense", "18"), ("Damage resist", "none")]),
    ("OFFENSE", [("Attack (main hand)", "+14"), ("Critical (main hand)", "19-20 x2")]),
    ("FORCE", [("Power DC", "26"), ("Light power cost", "x0.85"), ("Dark power cost", "x1.15"), ("Force resist", "0")]),
    ("CHARACTER", [("Alignment", "72 / 100")]),
]
DEFLECTION_TIP_TITLE = "Deflection 16"
DEFLECTION_TIP_LINES = ["Base attack bonus +10", "Advanced Jedi Defense +3", "Force Redirection +3", "",
                        "Attacker Precise Shot lowers this by 2-10."]
DEFLECTION_TIP_DESC = ("Your d20 roll plus this total is checked against each blaster bolt; "
                       "beat it by 6 to reflect.")
DEFENSE_TIP_TITLE = "Defense 18"
DEFENSE_TIP_LINES = ["Base 10", "Dexterity +4", "Armor +3", "Effects +1", ""]
DEFENSE_TIP_DESC = ("Attackers roll d20 + attack bonus against this. "
                    "Total Defense stance adds +4 (+6 for some classes).")


def layout():
    """Return (section list, row list) in units; asserts the stack fits the panel."""
    l, t, w, h = PANEL
    y = t + PAD
    secs, rows = [], []
    for title, items in SAMPLE:
        secs.append((title, (l + 4, y, w - 8, SEC_H)))
        y += SEC_H + 2
        for label, value in items:
            rows.append((label, value, (l + EDGE, y, w - 2 * EDGE, ROW_H)))
            y += ROW_H
        y += SEC_GAP
    bottom = y - SEC_GAP
    assert bottom <= t + h, f"stats tab overflows panel: last row bottom {bottom} > panel bottom {t + h}"
    print(f"fit margin: {t + h - bottom} units")
    return secs, rows


def panel_items(secs):
    items = [("frame", PANEL, None)]  # frame first so the sections sit on top of its black fill
    for title, rect in secs:
        items.append(("section", rect, (title, cpm.CREAM)))
    return items


def stats_ctls(ctls):
    """Drop every LBL_EFX_* control except the panel back and header; header reads "Stats | Effects"."""
    out = []
    for c in ctls:
        tag = c["TAG"]
        if tag == "LBL_EFX_HEADER":
            hdr = copy.deepcopy(c)
            txt = dict(hdr.get("TEXT") or {"FONT": "dialogfont16x16", "ALIGN": 17, "COLOR": cpm.CREAM})
            txt["S"] = "Stats | Effects"
            hdr["TEXT"] = txt
            out.append(hdr)
        elif tag.startswith("LBL_EFX_") and tag != "LBL_EFX_BACK":
            continue
        else:
            out.append(c)
    return out


def row_font(label, value):
    """Return (font, label_text): 16x16 if the pair fits the row, else 10x10 (label fitted as last resort)."""
    for font in ("dialogfont16x16", "dialogfont10x10"):
        need = cpm.text_size(font, label)[0] + cpm.text_size(font, value)[0] + GAP_PX
        if need <= ROW_W_PX:
            return font, label
    font = "dialogfont10x10"
    vw = cpm.text_size(font, value)[0]
    return font, cpm.fit(font, label, ROW_W_PX - vw - GAP_PX)


def draw_rows(im, rows):
    n10 = 0
    for label, value, r in rows:
        font, label_s = row_font(label, value)
        n10 += font == "dialogfont10x10"
        cpm.draw_text(im, font, label_s, cpm.px(r), 17, cpm.TEAL_TEXT)
        cpm.draw_text(im, font, value, cpm.px(r), 19, cpm.CREAM)
    print(f"row fonts: dialogfont16x16={len(rows) - n10} dialogfont10x10={n10}")


def tip_title_h():
    return 16 * cpm.tscale("dialogfont16x16") * 1.25


def tip_lh():
    return 12.5 * cpm.tscale("dialogfont10x10")


def tip_body(lines, desc, tw_u):
    return list(lines) + cpm.wrap(desc, "dialogfont10x10", (tw_u - 2 * TIP_PAD) * cpm.SX)


def tip_height_units(n_lines):
    px = 2 * TIP_PAD * cpm.SY + tip_title_h() + n_lines * tip_lh()
    return px / cpm.SY


def draw_tip(canvas, tip, title, body):
    """Reference tooltip (character_page_mockup.render tooltip=True): black pill, teal frame, title, body."""
    tx0, ty0, tx1, ty1 = cpm.px(tip)
    ImageDraw.Draw(canvas).rounded_rectangle([tx0 + 8, ty0 + 8, tx1 - 8, ty1 - 8], 30, fill=(0, 0, 0, 255))
    cpm.draw_border(canvas, {"FILL": "uibit_fill_2bt", "EDGE": "uibit_brdr_16bet", "CORNER": "uibit_brdr_16bct",
                             "DIMENSION": 4, "COLOR": cpm.TEAL_FRAME}, tip)
    x = tx0 + TIP_PAD * cpm.SX
    y = ty0 + TIP_PAD * cpm.SY
    th, lh = tip_title_h(), tip_lh()
    cpm.draw_text(canvas, "dialogfont16x16", title, (x, y, tx1 - TIP_PAD * cpm.SX, y + th), 9, cpm.CREAM)
    y += th
    for ln in body:
        if ln:
            cpm.draw_text(canvas, "dialogfont10x10", ln, (x, y, tx1 - TIP_PAD * cpm.SX, y + lh), 9, cpm.TEAL_TEXT)
        y += lh


def crop_units(im, box):
    l, t, r, b = box
    x0, y0, x1, y1 = cpm.px((l, t, r - l, b - t))
    return im.crop((round(x0), round(y0), round(x1), round(y1)))


def save(im, path):
    im.convert("RGB").save(path)
    return path


def main():
    ap = argparse.ArgumentParser(description="Render the Stats tab mock-up, its deflection tooltip, and the vanilla Defense tooltip.")
    ap.add_argument("outdir", type=Path)
    args = ap.parse_args()
    out = args.outdir.resolve()
    out.mkdir(parents=True, exist_ok=True)

    root, ctls = cpm.load_gui()
    secs, rows = layout()

    tab = cpm.render(root, stats_ctls(ctls), panel_items(secs), "dialogfont16x16", title="stats tab").convert("RGBA")
    draw_rows(tab, rows)

    # deflection tooltip under the Deflection row (flip above the row if it would pass the panel bottom)
    deflection = next(r for lab, _, r in rows if lab == "Deflection")
    tip_im = tab.copy()
    dbody = tip_body(DEFLECTION_TIP_LINES, DEFLECTION_TIP_DESC, TIP_W)
    dth = tip_height_units(len(dbody))
    dty = deflection[1] + ROW_H + 2
    if dty + dth > PANEL[1] + PANEL[3]:
        dty = deflection[1] - 2 - dth
    draw_tip(tip_im, (PANEL[0] + 4, dty, TIP_W, dth), DEFLECTION_TIP_TITLE, dbody)

    # vanilla live page (no panel items) with a Defense tooltip right of the stat value
    van = cpm.render(root, ctls, [], "dialogfont16x16", title="vanilla").convert("RGBA")
    by = {c["TAG"]: c for c in ctls}
    anchor = "LBL_DEFENSE_STAT" if "LBL_DEFENSE_STAT" in by else "LBL_DEFENSE"
    al, at, aw, _ = by[anchor]["RECT"]
    vtw = 200
    vbody = tip_body(DEFENSE_TIP_LINES, DEFENSE_TIP_DESC, vtw)
    vth = tip_height_units(len(vbody))
    vx = min(al + aw + 4, 800 - vtw - 2)
    vy = min(at, 600 - vth - 2)
    draw_tip(van, (vx, vy, vtw, vth), DEFENSE_TIP_TITLE, vbody)
    vcrop = (VANILLA_CROP[0], VANILLA_CROP[1], VANILLA_CROP[2], max(VANILLA_CROP[3], round(vy + vth + 8)))
    print(f"vanilla anchor: {anchor} rect={by[anchor]['RECT']} tip=({vx:.1f},{vy:.1f},{vtw},{vth:.1f})")
    print(f"vanilla crop units: {vcrop}")

    paths = [
        save(tab, out / "stats_tab.png"),
        save(tip_im, out / "stats_tab_tip_deflection.png"),
        save(van, out / "vanilla_tip_defense.png"),
        save(crop_units(tab, TAB_CROP), out / "stats_tab_zoom.png"),
        save(crop_units(tip_im, TAB_CROP), out / "stats_tab_tip_deflection_zoom.png"),
        save(crop_units(van, vcrop), out / "vanilla_tip_defense_zoom.png"),
    ]
    for p in paths:
        print(p)


if __name__ == "__main__":
    main()
