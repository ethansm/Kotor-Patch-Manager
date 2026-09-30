#!/usr/bin/env python3
"""Add a 3D_MODEL scene-view control (+ arc cover labels) to the live override/equip_p.gui (Mod 5).

Pattern copied from scripts/add_inventory_3dview_control.py (Mod 1). Post-step for
scripts/generate_ultrawide_gui_overrides.py: the generator's manifest records equip_p.gui's deployed sha256, so a
hand-modified live file makes a generator re-run refuse ("foreign file"). Run `--rollback` here first, re-run the
generator, then re-apply this script.

The engine only creates controls the panel constructor binds by tag, so the GUI is safe to deploy before the DLL
(the equipment page just looks re-laid-out; without the DLL the reserved column is empty). The EquipCharPreview patch
binds `3D_MODEL`, `LBL_BEVEL`, `LBL_BEVEL2` by tag in the CSWGuiInGameEquip constructor.

Layouts (units, see scripts/equip3d_layout_mockup.py for pictures):
  R  view LEFT [48,134,216,336]; list side +113, slot/description block +113   (chosen by the user 2026-09-30)
  A  view RIGHT [536,134,216,336]; list side -113, block -113
  G  right gutter only [650,134,102,336]; nothing moves

  --layout X  apply layout X (idempotent; refuses when a control's rect is neither vanilla nor the layout target)
  --dry-run   print the plan, write nothing
  --rollback  restore the OLDEST backup taken by this script (= the pre-Mod-5 file)
"""
import argparse
import copy
import datetime
import hashlib
import shutil
import sys
from pathlib import Path

from pykotor.resource.formats.gff import read_gff, write_gff

GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
OVERRIDE = GAME / "override"
BACKUP_ROOT = Path.home() / "Games/KOTOR2_Mod_Workspace/backups"
TAG = "3D_MODEL"
EXT_KEYS = ("LEFT", "TOP", "WIDTH", "HEIGHT")

# controls of the slot/description block and of the list side; everything else (LBL_BAR1..5 with the portrait notch
# in BAR5, LBL_TITLE, LBL_SLOTNAME, BTN_PREVNPC/NEXTNPC) stays vanilla in every layout.
BLOCK = ["LB_DESC", "LBL_BACK1", "LBL_INV_WEAP_R2", "LBL_INV_WEAP_L2", "LBL_INV_WEAP_R", "LBL_INV_WEAP_L",
         "LBL_INV_BELT", "LBL_INV_ARM_R", "LBL_INV_BODY", "LBL_INV_ARM_L", "LBL_INV_HANDS", "LBL_INV_HEAD",
         "LBL_INV_IMPLANT", "LBL_DEF_BACK", "LBL_DEF", "LBL_DEF_TEXT", "LBL_TOHITR", "LBL_TOHITL", "LBL_TOHIT",
         "LBL_ATKL", "LBL_ATKR", "LBL_DAMAGE", "LBL_ATTACKMOD", "LBL_DAMTEXT", "BTN_SWAPWEAPONS",
         "BTN_INV_HEAD", "BTN_INV_BODY", "BTN_INV_ARM_L", "BTN_INV_WEAP_L", "BTN_INV_WEAP_L2", "BTN_INV_WEAP_R2",
         "BTN_INV_BELT", "BTN_INV_WEAP_R", "BTN_INV_ARM_R", "BTN_INV_HANDS", "BTN_INV_IMPLANT"]
LIST = ["LB_ITEMS", "LBL_CANTEQUIP", "BTN_BACK", "BTN_EQUIP"]
CHILDREN = ("PROTOITEM", "SCROLLBAR")

# layout -> (list dx, block dx, 3D rect, covers)
LAYOUTS = {
    "R": (113, 113, (48, 134, 216, 336), {"LBL_BEVEL": (44, 130, 224, 38), "LBL_BEVEL2": (44, 436, 224, 38)}),
    "A": (-113, -113, (536, 134, 216, 336), {"LBL_BEVEL": (532, 130, 224, 38), "LBL_BEVEL2": (532, 436, 224, 38)}),
    "G": (0, 0, (650, 134, 102, 336), {}),
}
BEVEL_TAGS = ("LBL_BEVEL", "LBL_BEVEL2")
# Control-toolbar buttons (Mod 5): text labels the DLL binds by tag; it toggles text/fill and polls the cursor for clicks.
# IDs follow the covers (3D_MODEL=N, covers N+1,N+2, buttons N+3..). Rects are relative to the 3D view rect (x offset, y offset, w, h).
BTN_TAGS = ("LBL_E3D_ANIM", "LBL_E3D_SPIN", "LBL_E3D_WEAP")
TIP_TAGS = ("LBL_E3D_TIPBG", "LBL_E3D_TIP")   # hover tooltip frame + text; the DLL sizes/positions/shows them
BTN_TEXT = ("Animate", "Spin", "Weapons")
BTN_TEX = ("e3d_btn", "e3d_btn_hi", "e3d_btn_off")   # on/normal, hover, off
BTN_TEMPLATE = ("mipc28x6_p.gui", "LBL_QUEUE1")
BTN_FONT = "dialogfont10x10"
KNOWN_DX = {0} | {v[0] for v in LAYOUTS.values()} | {v[1] for v in LAYOUTS.values()}


def get_rect(e):
    return tuple(e.get_int32(k) for k in EXT_KEYS)


def set_rect(e, r):
    for k, v in zip(EXT_KEYS, r):
        e.set_int32(k, v)


def sha(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()


def find_ci(d: Path, name: str) -> Path:
    for p in d.iterdir():
        if p.name.lower() == name.lower():
            return p
    raise FileNotFoundError(name)


def controls(g):
    return g.root.get_list("CONTROLS")


def pristine_backup() -> Path | None:
    dirs = sorted(BACKUP_ROOT.glob("equip3d_*"))
    return dirs[0] if dirs else None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--layout", choices=sorted(LAYOUTS), help="R|A|G (see docstring)")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--rollback", action="store_true")
    a = ap.parse_args()
    eq = find_ci(OVERRIDE, "equip_p.gui")

    if a.rollback:
        b = pristine_backup()
        if b is None:
            print("no backup found", file=sys.stderr)
            return 1
        src = b / "equip_p.gui"
        print(f"restore {src} -> {eq}")
        if not a.dry_run:
            shutil.copy2(src, eq)
        return 0
    if not a.layout:
        ap.error("--layout is required (or --rollback)")

    ldx, bdx, rect, covers = LAYOUTS[a.layout]
    g = read_gff(str(eq))
    cs = controls(g)

    # vanilla reference rects come from the pristine backup when there is one (so re-application is exact),
    # otherwise from the current file, which must then be vanilla (no 3D_MODEL yet).
    pb = pristine_backup()
    ref_cs = controls(read_gff(str(pb / "equip_p.gui"))) if pb else cs
    if not pb and any(c.get_string("TAG") == TAG for c in cs):
        raise SystemExit("3D_MODEL present but no pristine backup found: refusing (run --rollback logic manually)")
    ref = {c.get_string("TAG"): c for c in ref_cs}

    # rebuild from vanilla: drop previous additions, set every moved control to vanilla+dx
    for i in reversed([i for i, c in enumerate(cs) if c.get_string("TAG") in (TAG,) + BEVEL_TAGS + BTN_TAGS + TIP_TAGS + ("LBL_E3D_RESET",)]):
        del cs[i]
    changes = []
    for tags, dx in ((LIST, ldx), (BLOCK, bdx)):
        for tag in tags:
            c = next((c for c in cs if c.get_string("TAG") == tag), None)
            r = ref.get(tag)
            if c is None or r is None:
                raise SystemExit(f"control {tag} not found")
            for sub in (None,) + CHILDREN:
                if sub is not None and not r.exists(sub):
                    continue
                rh = r if sub is None else r.get_struct(sub)
                vr = get_rect(rh.get_struct("EXTENT"))
                holder = c if sub is None else c.get_struct(sub)
                e = holder.get_struct("EXTENT")
                cur = get_rect(e)
                want = (vr[0] + dx, vr[1], vr[2], vr[3])
                name = tag if sub is None else f"{tag}.{sub}"
                if cur[1:] != vr[1:] or cur[0] - vr[0] not in KNOWN_DX:
                    raise SystemExit(f"refusing: {name} rect {cur} is neither vanilla {vr} nor a known layout offset")
                if cur != want:
                    set_rect(e, want)
                    holder.set_struct("EXTENT", e)
                    if sub is not None:
                        c.set_struct(sub, holder)
                    changes.append(f"  {name}: {list(cur)} -> {list(want)}")

    ids = sorted(c.get_int32("ID") for c in cs)
    new_id = len(cs)
    assert ids == list(range(new_id)), f"equip IDs are not 0..N-1: {ids}"
    up = read_gff(str(find_ci(OVERRIDE, "upgrade_p.gui")))
    src = [c for c in controls(up) if c.get_string("TAG") == TAG]
    assert len(src) == 1, f"expected exactly one {TAG} in upgrade_p.gui, got {len(src)}"
    ctl = copy.deepcopy(src[0])
    ctl.set_int32("ID", new_id)
    ctl.set_string("TAG", TAG)
    ext = ctl.get_struct("EXTENT")
    set_rect(ext, rect)
    ctl.set_struct("EXTENT", ext)
    cs.append(ctl)
    print(f"  add {TAG} ID={new_id} {list(rect)}")
    if covers:
        chp = read_gff(str(find_ci(OVERRIDE, "character_p.gui")))
        for i, (btag, brect) in enumerate(covers.items()):
            bs = [c for c in controls(chp) if c.get_string("TAG") == btag]
            assert len(bs) == 1, f"expected one {btag} in character_p.gui"
            bc = copy.deepcopy(bs[0])
            bc.set_int32("ID", new_id + 1 + i)
            be = bc.get_struct("EXTENT")
            set_rect(be, brect)
            bc.set_struct("EXTENT", be)
            cs.append(bc)
            print(f"  add {btag} ID={new_id + 1 + i} {list(brect)} (copied from character_p.gui)")
    if covers:   # toolbar under the view's bottom arc: 3 buttons across the view width
        tsrc = [c for c in controls(read_gff(str(find_ci(OVERRIDE, BTN_TEMPLATE[0])))) if c.get_string("TAG") == BTN_TEMPLATE[1]]
        assert len(tsrc) == 1, f"expected one {BTN_TEMPLATE[1]} in {BTN_TEMPLATE[0]}"
        bw, bh, gap = 62, 22, 4   # compact column at the view's right edge, below the top arc (empty backdrop, clear of the character)
        for i, (btag, btxt) in enumerate(zip(BTN_TAGS, BTN_TEXT)):
            bc = copy.deepcopy(tsrc[0])
            bc.set_int32("ID", new_id + 3 + i)
            bc.set_string("TAG", btag)
            brect = (rect[0] + rect[2] - bw - 6, rect[1] + 40 + i * (bh + gap), bw, bh)
            be = bc.get_struct("EXTENT"); set_rect(be, brect); bc.set_struct("EXTENT", be)
            tx = bc.get_struct("TEXT"); tx.set_string("TEXT", btxt); tx.set_int32("ALIGNMENT", 18)
            tx.set_resref("FONT", type(tx.get_resref("FONT"))(BTN_FONT)); bc.set_struct("TEXT", tx)
            bb = bc.get_struct("BORDER"); bb.set_resref("FILL", type(bb.get_resref("FILL"))(BTN_TEX[0])); bc.set_struct("BORDER", bb)
            cs.append(bc)
            print(f"  add {btag} ID={new_id + 3 + i} {list(brect)} '{btxt}'")
    if covers:   # tooltip: dark framed panel (the HUD's LBL_MOULDING3) + text label, placeholder rects
        hud = read_gff(str(find_ci(OVERRIDE, BTN_TEMPLATE[0])))
        bgs = [c for c in controls(hud) if c.get_string("TAG") == "LBL_MOULDING3"]
        assert len(bgs) == 1, "LBL_MOULDING3 not found in mipc28x6_p.gui"
        for j, (ttag, tsrc0) in enumerate(zip(TIP_TAGS, (bgs[0], tsrc[0]))):
            tc = copy.deepcopy(tsrc0)
            tc.set_int32("ID", new_id + 3 + len(BTN_TAGS) + j)
            tc.set_string("TAG", ttag)
            te = tc.get_struct("EXTENT"); set_rect(te, (10, 10, 100, 40)); tc.set_struct("EXTENT", te)
            if j == 1:
                tx = tc.get_struct("TEXT"); tx.set_string("TEXT", ""); tx.set_int32("ALIGNMENT", 18); tc.set_struct("TEXT", tx)
                tb = tc.get_struct("BORDER"); tb.set_resref("FILL", type(tb.get_resref("FILL"))("")); tc.set_struct("BORDER", tb)
            cs.append(tc)
            print(f"  add {ttag} ID={new_id + 3 + len(BTN_TAGS) + j}")
    g.root.set_list("CONTROLS", cs)
    print(f"layout {a.layout}: {len(changes)} control rect change(s)")
    print("\n".join(changes))
    print(f"plan: {len(cs)} controls total in {eq}")
    if a.dry_run:
        return 0

    stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    bdir = BACKUP_ROOT / f"equip3d_{stamp}"
    bdir.mkdir(parents=True)
    shutil.copy2(eq, bdir / "equip_p.gui")
    (bdir / "equip_p.gui.sha256").write_text(sha(eq) + "\n")
    if covers:
        from PIL import Image, ImageDraw
        for name, col in zip(BTN_TEX, ((18, 110, 85), (208, 204, 170), (70, 78, 78))):
            S = 4; im = Image.new("RGBA", (128 * S, 48 * S), (0, 0, 0, 0)); d = ImageDraw.Draw(im)
            d.rounded_rectangle([0, 0, 128 * S - 1, 48 * S - 1], radius=16 * S, fill=col + (255,))
            d.rounded_rectangle([6 * S, 6 * S, 122 * S, 42 * S], radius=11 * S, fill=(0, 0, 0, 175))
            im.resize((128, 48), Image.LANCZOS).save(OVERRIDE / f"{name}.tga", format="TGA")
        print("wrote textures " + ", ".join(BTN_TEX))
    tmp = eq.with_suffix(".gui.tmp")
    write_gff(g, str(tmp))
    v = read_gff(str(tmp))
    vc = controls(v)
    vids = sorted(c.get_int32("ID") for c in vc)
    assert vids == list(range(len(vc))), vids
    nc = [c for c in vc if c.get_string("TAG") == TAG]
    assert len(nc) == 1 and nc[0].get_int32("ID") == new_id
    assert get_rect(nc[0].get_struct("EXTENT")) == rect
    assert nc[0].get_int32("CONTROLTYPE") == 4
    tmp.replace(eq)
    print(f"deployed. backup: {bdir}\nnew sha256: {sha(eq)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
