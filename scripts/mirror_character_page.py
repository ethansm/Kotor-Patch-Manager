#!/usr/bin/env python3
"""Character page "thirds" layout + Mod 14 Active Effects panel controls in the live override/character_p.gui.

Layout (approved by the user 2026-10-01 from scripts/character_page_mockup.py, which holds the shared numbers):
  left third   stats group (every control with 405 <= LEFT < 640 except LBL_BAR4/5) dx -353
  middle third 3D group dx +129, view [295,134,230,336] ends at y 470 (LBL_BAR5 notch), covers and slider group lifted
  right third  Active Effects panel [536,134,216,336] + header [536,104,216,24]
Fixed: LBL_BAR1..5, LBL_TITLE, BTN_EXIT.

New controls (IDs 69.. contiguous, appended in draw order; the CharEffectsPanel DLL binds them by tag, positions the
section/row labels per tick and sets texts/icons). The engine only creates controls the panel constructor binds by tag,
so without the DLL the page shows the new layout with an empty right third.
  LBL_EFX_BACK    black fill (uibit_fill_2bt) + thin frame (LBL_MORE_BACK edge/corner)   [PANEL]
  LBL_EFX_HEADER  LBL_BAR6 clone, text "Active Effects"                                  [HEADER]
  LBL_EFX_SECG/B  section header: dark teal band (uibit_fill_2wt), "BENEFICIAL" cream / "HARMFUL" orange
  LBL_EFX_Dnn     item-ability disc behind the icon (fill set by the DLL)            nn = 00..MAX_ROWS-1
  LBL_EFX_Inn     icon (fill set by the DLL)
  LBL_EFX_Nnn     name, dialogfont16x16 teal, middle-left
  LBL_EFX_Tnn     remaining time, dialogfont16x16 cream, middle-right
  LBL_EFX_MORE    "No active effects" (empty state), cream, centred
  LBL_EFX_SBTRK/SBTHM/SBUP/SBDN  scrollbar, shown by the DLL only when the list overflows (vanilla listbox look:
                  track edge/corner uibit_brdr_16ge/16gc dim 4, thumb uibit_fill_16g, arrows uibit_uparrowg + efx_dnarrowg.tga
                  written by this script); whole-entry scrolling, model in character_page_mockup.scroll_state()
  LBL_EFX_TIPBG / LBL_EFX_TIPT / LBL_EFX_TIP   tooltip frame (mipc28x6_p LBL_MOULDING3) + title + body (optional stage)

Generator interplay: character_p.gui is in scripts/generate_ultrawide_gui_overrides.py; its manifest records the deployed
sha256, so this hand-modified file makes a generator re-run refuse ("foreign file"). Run `--rollback` here first, re-run
the generator, then re-apply this script (the next run takes a fresh pristine backup only if none exists: delete the old
charmirror_* backups after a generator re-run so the new generator output becomes the reference).

  (no args)   apply (idempotent: rebuilt from the pristine reference rects; refuses a rect that is neither vanilla nor target)
  --dry-run   print the plan, write nothing
  --rollback  restore the OLDEST backup taken by this script (= the pre-mirror file)
Refuses to write while swkotor2.exe runs.
"""
import argparse
import copy
import datetime
import hashlib
import shutil
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import character_page_mockup as L  # noqa: E402  (shared layout numbers)
from pykotor.common.misc import ResRef, Vector3  # noqa: E402
from pykotor.resource.formats.gff import read_gff, write_gff  # noqa: E402

GAME = L.GAME
OVERRIDE = GAME / "override"
BACKUP_ROOT = Path.home() / "Games/KOTOR2_Mod_Workspace/backups"
EXT_KEYS = ("LEFT", "TOP", "WIDTH", "HEIGHT")
FIRST_NEW_ID = 69
ROW_TAGS = [f"LBL_EFX_{k}{i:02d}" for k in "DINT" for i in range(L.MAX_ROWS)]
NEW_TAGS = (["LBL_EFX_BACK", "LBL_EFX_HEADER", "LBL_EFX_SECG", "LBL_EFX_SECB"]
            + [f"LBL_EFX_D{i:02d}" for i in range(L.MAX_ROWS)] + [f"LBL_EFX_I{i:02d}" for i in range(L.MAX_ROWS)]
            + [f"LBL_EFX_N{i:02d}" for i in range(L.MAX_ROWS)] + [f"LBL_EFX_T{i:02d}" for i in range(L.MAX_ROWS)]
            + ["LBL_EFX_MORE", "LBL_EFX_SBTRK", "LBL_EFX_SBTHM", "LBL_EFX_SBUP", "LBL_EFX_SBDN",
               "LBL_EFX_TIPBG", "LBL_EFX_TIPT", "LBL_EFX_TIP"])
DN_ARROW = "efx_dnarrowg"   # vertically flipped uibit_uparrowg (no down-arrow texture exists; the engine rotates DIR)


def get_rect(e):
    return tuple(e.get_int32(k) for k in EXT_KEYS)


def set_rect(c, r):
    e = c.get_struct("EXTENT")
    for k, v in zip(EXT_KEYS, r):
        e.set_int32(k, v)
    c.set_struct("EXTENT", e)


def sha(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()


def find_ci(d: Path, name: str) -> Path:
    for p in d.iterdir():
        if p.name.lower() == name.lower():
            return p
    raise FileNotFoundError(name)


def controls(g):
    return g.root.get_list("CONTROLS")


def pristine_backup():
    dirs = sorted(BACKUP_ROOT.glob("charmirror_*"))
    return dirs[0] if dirs else None


def game_running() -> bool:
    return subprocess.run(["pgrep", "-x", "swkotor2.exe"], capture_output=True).returncode == 0


def set_text(c, text=None, font=None, align=None, color=None):
    t = c.get_struct("TEXT")
    if text is not None:
        t.set_string("TEXT", text)
        t.set_uint32("STRREF", 0xFFFFFFFF)
    if font is not None:
        t.set_resref("FONT", ResRef(font))
    if align is not None:
        t.set_int32("ALIGNMENT", align)
    if color is not None:
        t.set_vector3("COLOR", Vector3(*color))
    c.set_struct("TEXT", t)


def set_border(c, fill=None, edge=None, corner=None, dim=None, color=None):
    b = c.get_struct("BORDER")
    for k, v in (("FILL", fill), ("EDGE", edge), ("CORNER", corner)):
        if v is not None:
            b.set_resref(k, ResRef(v))
    if dim is not None:
        b.set_int32("DIMENSION", dim)
    if color is not None:
        b.set_vector3("COLOR", Vector3(*color))
    c.set_struct("BORDER", b)


def target_rects(ref):
    """tag -> target rect for every moved control, from the vanilla reference rects."""
    out = {}
    base = [{"TAG": t, "RECT": get_rect(c.get_struct("EXTENT"))} for t, c in ref.items()]
    for c in L.thirds(base):
        out[c["TAG"]] = c["RECT"]
    return out


def new_controls(by_tag, hud_tip):
    """Effects-panel controls in draw order, placeholder rects where the DLL lays them out."""
    stat = by_tag["LBL_STR"]
    out = []

    def add(tag, src, rect):
        c = copy.deepcopy(src)
        c.set_string("TAG", tag)
        c.set_int32("ID", FIRST_NEW_ID + len(out))
        set_rect(c, rect)
        out.append(c)
        return c

    c = add("LBL_EFX_BACK", by_tag["LBL_MORE_BACK"], L.PANEL)
    set_border(c, fill="uibit_fill_2bt")
    c = add("LBL_EFX_HEADER", by_tag["LBL_BAR6"], L.HEADER)
    set_text(c, text="Active Effects", align=18, color=L.CREAM)
    x0 = L.PANEL[0] + L.PAD_X
    inner_w = L.PANEL[2] - 2 * L.PAD_X
    for tag, txt, col, y in (("LBL_EFX_SECG", "BENEFICIAL", L.CREAM, L.PANEL[1] + L.PAD_TOP),
                             ("LBL_EFX_SECB", "HARMFUL", L.ORANGE, L.PANEL[1] + L.PAD_TOP + 200)):
        c = add(tag, stat, (x0 - 4, y, inner_w + 8, L.SEC_H))
        set_border(c, fill="uibit_fill_2wt", edge="", corner="", dim=0, color=L.TEAL_FRAME)
        set_text(c, text=txt, align=17, color=col, font="dialogfont16x16")
    ry = lambda i: L.PANEL[1] + L.PAD_TOP + L.SEC_H + 2 + i * L.ROW_H
    for k in "DI":
        for i in range(L.MAX_ROWS):
            c = add(f"LBL_EFX_{k}{i:02d}", stat, (x0, ry(i) + (L.ROW_H - L.ICON_H) // 2, L.ICON_W, L.ICON_H))
            set_border(c, fill="", color=(1, 1, 1))
            set_text(c, text="")
    name_x = x0 + L.ICON_W + 4
    name_w = inner_w - (L.ICON_W + 4) - L.TIME_W - 2
    for i in range(L.MAX_ROWS):
        c = add(f"LBL_EFX_N{i:02d}", stat, (name_x, ry(i), name_w, L.ROW_H))
        set_text(c, text="", align=17, color=L.TEAL_TEXT)
    for i in range(L.MAX_ROWS):
        c = add(f"LBL_EFX_T{i:02d}", stat, (x0 + inner_w - L.TIME_W, ry(i), L.TIME_W, L.ROW_H))
        set_text(c, text="", align=19, color=L.CREAM)
    c = add("LBL_EFX_MORE", stat, (x0, L.PANEL[1] + L.PANEL[3] - L.PAD_TOP - L.ROW_H, inner_w, L.ROW_H))
    set_text(c, text="", align=18, color=L.CREAM)
    top, win = L.PANEL[1] + L.PAD_TOP, L.PANEL[3] - 2 * L.PAD_TOP
    sx = x0 + inner_w - L.SB_W
    c = add("LBL_EFX_SBTRK", stat, (sx, top + L.SB_ARROW_H + 2, L.SB_W, win - 2 * L.SB_ARROW_H - 4))
    set_border(c, fill="", edge="uibit_brdr_16ge", corner="uibit_brdr_16gc", dim=4, color=(1, 1, 1)); set_text(c, text="")
    c = add("LBL_EFX_SBTHM", stat, (sx + 2, top + L.SB_ARROW_H + 2, L.SB_W - 4, L.SB_THUMB_MIN))
    set_border(c, fill="uibit_fill_16g", edge="", corner="", dim=0, color=(1, 1, 1)); set_text(c, text="")
    c = add("LBL_EFX_SBUP", stat, (sx, top, L.SB_W, L.SB_ARROW_H))
    set_border(c, fill="uibit_uparrowg", edge="", corner="", dim=0, color=(1, 1, 1)); set_text(c, text="")
    c = add("LBL_EFX_SBDN", stat, (sx, top + win - L.SB_ARROW_H, L.SB_W, L.SB_ARROW_H))
    set_border(c, fill=DN_ARROW, edge="", corner="", dim=0, color=(1, 1, 1)); set_text(c, text="")
    tip = (L.PANEL[0] + 4, L.PANEL[1] + 40, L.PANEL[2] - 8, 104)
    add("LBL_EFX_TIPBG", hud_tip, tip)
    c = add("LBL_EFX_TIPT", stat, (tip[0] + 8, tip[1] + 6, tip[2] - 16, 20))
    set_text(c, text="", align=9, color=L.CREAM)
    c = add("LBL_EFX_TIP", stat, (tip[0] + 8, tip[1] + 28, tip[2] - 16, tip[3] - 34))
    set_text(c, text="", align=9, color=L.TEAL_TEXT, font="dialogfont10x10")
    assert [c.get_string("TAG") for c in out] == NEW_TAGS, "tag order drift"
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--rollback", action="store_true")
    a = ap.parse_args()
    ch = find_ci(OVERRIDE, "character_p.gui")
    if not a.dry_run and game_running():
        raise SystemExit("swkotor2.exe is running: refusing to write (close the game first)")

    if a.rollback:
        b = pristine_backup()
        if b is None:
            raise SystemExit("no charmirror_* backup found")
        print(f"restore {b / 'character_p.gui'} -> {ch}")
        if not a.dry_run:
            shutil.copy2(b / "character_p.gui", ch)
            print(f"sha256 {sha(ch)}")
        return 0

    g = read_gff(str(ch))
    cs = controls(g)
    pb = pristine_backup()
    if not pb and any(c.get_string("TAG") in NEW_TAGS for c in cs):
        raise SystemExit("effects-panel controls present but no pristine backup: refusing")
    ref_cs = controls(read_gff(str(pb / "character_p.gui"))) if pb else cs
    ref = {c.get_string("TAG"): c for c in ref_cs}
    if any(t in ref for t in NEW_TAGS):
        raise SystemExit(f"pristine backup {pb} already contains effects-panel controls: refusing")

    # rebuild from the reference: drop our additions, then set every moved control to its target
    for i in reversed([i for i, c in enumerate(cs) if c.get_string("TAG") in NEW_TAGS]):
        del cs[i]
    if sorted(c.get_string("TAG") for c in cs) != sorted(ref):
        raise SystemExit("live control set differs from the reference (someone else changed character_p.gui): refusing")
    want = target_rects(ref)
    changes = []
    for c in cs:
        tag = c.get_string("TAG")
        vr = get_rect(ref[tag].get_struct("EXTENT"))
        cur = get_rect(c.get_struct("EXTENT"))
        tr = want[tag]
        if cur not in (vr, tr):
            raise SystemExit(f"refusing: {tag} rect {cur} is neither reference {vr} nor target {tr}")
        if cur != tr:
            set_rect(c, tr)
            changes.append(f"  {tag:<22} {list(cur)} -> {list(tr)}")
    errs = L.check([{"TAG": c.get_string("TAG"), "RECT": get_rect(c.get_struct("EXTENT"))} for c in cs])
    if errs:
        raise SystemExit("layout check failed: " + "; ".join(errs))
    ids = {c.get_int32("ID") for c in cs}
    assert max(ids) < FIRST_NEW_ID, f"existing ids reach {max(ids)}"
    by_tag = {c.get_string("TAG"): c for c in cs}
    hud = read_gff(str(find_ci(OVERRIDE, "mipc28x6_p.gui")))
    tip = [c for c in controls(hud) if c.get_string("TAG") == "LBL_MOULDING3"]
    assert len(tip) == 1, "LBL_MOULDING3 not found in mipc28x6_p.gui"
    add = new_controls(by_tag, tip[0])
    cs.extend(add)
    g.root.set_list("CONTROLS", cs)
    print(f"{len(changes)} rect change(s):")
    print("\n".join(changes))
    print(f"add {len(add)} effects-panel controls, IDs {FIRST_NEW_ID}..{FIRST_NEW_ID + len(add) - 1}")
    print(f"plan: {len(cs)} controls total in {ch}")
    if a.dry_run:
        return 0

    stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    bdir = BACKUP_ROOT / f"charmirror_{stamp}"
    bdir.mkdir(parents=True)
    shutil.copy2(ch, bdir / "character_p.gui")
    (bdir / "character_p.gui.sha256").write_text(sha(ch) + "\n")
    dn = OVERRIDE / f"{DN_ARROW}.tga"
    if not dn.exists():
        L.texture("uibit_uparrowg").transpose(L.Image.FLIP_TOP_BOTTOM).save(dn, format="TGA")
        print(f"wrote {dn}")
    tmp = ch.with_suffix(".gui.tmp")
    write_gff(g, str(tmp))
    vc = controls(read_gff(str(tmp)))
    vt = {c.get_string("TAG"): c for c in vc}
    assert len(vc) == len(cs) and len(vt) == len(vc), "control count / duplicate tag"
    for tag, r in want.items():
        assert get_rect(vt[tag].get_struct("EXTENT")) == r, tag
    for i, tag in enumerate(NEW_TAGS):
        assert vt[tag].get_int32("ID") == FIRST_NEW_ID + i, tag
    assert sorted(c.get_int32("ID") for c in vc) == sorted(ids | set(range(FIRST_NEW_ID, FIRST_NEW_ID + len(add))))
    tmp.replace(ch)
    print(f"deployed. backup: {bdir}\nnew sha256: {sha(ch)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
