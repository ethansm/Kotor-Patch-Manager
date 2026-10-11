#!/usr/bin/env python3
"""Generate aspect-ratio corrected GUI overrides for KOTOR II ultrawide displays.

In KOTOR II, menus and 2D elements are defined in an 800x600 virtual coordinate
space that the engine stretches across the full display resolution. On an ultrawide
display (e.g. 3840x1600, 24:10 / 2.4:1 aspect ratio), this causes a 1.80x horizontal
stretch relative to the vanilla 4:3 base.

This tool counteracts the horizontal stretch by applying an inverse horizontal
scaling. Two anchor modes exist:
    cluster anchor a:  L' = round(a + (L - a) / M),        W' = round(W / M)
    "self" (in place): L' = floor(L + (W - W') / 2 + 0.5), W' = round(W / M)
where M = (width / height) / (4 / 3). A cluster keeps a group of controls laid out
relative to one point (a portrait column, a tab row). In-place mode shrinks each
control about its own centre, for icons pinned to painted slots or scattered over a
map. Vertical coordinates (T, H) remain untouched.

Frame-bar rule (--frame-edges): once generate_ultrawide_textures.py `frame` has
remapped pnl_pause_pc (the gold frame behind 29 menus) its inner edge moves out
from x=61 to E_left. The teal separators (fill uibit_brdr_16we) that touched the
old inner edge are extended to the new one: L <= 62 -> L = E_left, and a right edge
>= 738 -> 800 - E_left. Only GUIs whose root BORDER.FILL is pnl_pause_pc get it.

The in-game HUD (mipc28x6_p) is NOT resized: on the Aspyr build it is drawn 1:1
and anchored to the screen edges (measured 2026-09-24: action tray 672x181 px =
LBL_MOULDING3 252x68 at a uniform 2.667 px/unit). SPECS_HUD is kept, unused,
behind --include-hud for other builds.

Safety Features:
- Preserves live Aspyr/mod customizations (e.g. CB_CLOUDSAVE in saveload_p.gui).
- Template fallback hierarchy: $STATE_DIR/originals/ -> live override/ -> chitin.key.
- Full self-verification: restores original coordinates on reload and asserts GFF equivalence.
- Aborts deploy if swkotor2.exe is running or if a foreign mod replaced the live file.
- Manifest records source (override|chitin) and original/deployed SHA256; --rollback
  restores override-sourced files from originals/ and deletes chitin-sourced ones,
  leaving any file someone else changed since the deploy.
"""

import argparse
import datetime
import hashlib
import json
import logging
import math
import os
from pathlib import Path
import re
import subprocess
import sys

# Suppress pykotor debug logging before imports
logging.disable(logging.CRITICAL)

try:
    from pykotor.extract.chitin import Chitin
    from pykotor.resource.formats.gff import GFF, GFFFieldType, bytes_gff, read_gff
    from pykotor.resource.type import ResourceType
except ImportError:
    print("Error: pykotor must be installed in python environment.", file=sys.stderr)
    sys.exit(1)


DEFAULT_KOTOR2_ROOT = os.environ.get(
    "KOTOR2_ROOT",
    os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"),
)
DEFAULT_WORKSPACE_DIR = os.environ.get(
    "KOTOR2_WORKSPACE",
    os.path.expanduser("~/Games/KOTOR2_Mod_Workspace"),
)

SELF = "self"  # cluster anchor meaning "scale each control about its own centre"

FRAME_TEXTURE = "pnl_pause_pc"
FRAME_BAR_FILL = "uibit_brdr_16we"
FRAME_BAR_LEFT_MAX = 62     # bar L <= this touches the frame's left inner edge (vanilla 61)
FRAME_BAR_RIGHT_MIN = 738   # bar L+W >= this touches the right inner edge (vanilla 739)

# Frame textures whose teal separators touch the painted inner edge:
# texture -> (left_max, right_min, bar FILL). The rule matches on FILL only: many
# interior buttons and boxes use uibit_brdr_16we as their EDGE and are not bars.
# pnl_pause_pc: 29 menus. pnl_store_pc: store_p LBL_BAR1/2/3 at L61 R739 (checked
# 2026-09-24). computer/workbench/upgrade panels have no frame-touching bars.
FRAME_BAR_RULES = {
    "pnl_pause_pc": (FRAME_BAR_LEFT_MAX, FRAME_BAR_RIGHT_MIN, FRAME_BAR_FILL),
    "pnl_store_pc": (FRAME_BAR_LEFT_MAX, FRAME_BAR_RIGHT_MIN, FRAME_BAR_FILL),
}

# Content layer (--content-aspect): full-screen menus whose root fill is one of these
# panel textures, plus the overlays/panels in CONTENT_EXTRA (no root fill), get their
# controls mapped into a centred region of the given aspect.
CONTENT_FILLS = ("pnl_pause_pc", "pnl_store_pc", "pnl_computer_pc", "pnl_workbench_pc", "pnl_upgrade_pc")
CONTENT_EXTRA = ("top_p", "custpnl_p", "leveluppnl_p", "quickpnl_p", "name_p", "qorcpnl_p")
CONTENT_EXCLUDE = {
    "map_p": "the engine anchors the area-map texture at LBL_Map's left edge and scales it "
             "from the box height; any box change shifts or clips the map (in-game 2026-09-24)",
    "loadscreen_p": "full-screen loading image, pillarboxed by the texture tool instead",
    "galaxymap_p": "controls are pinned to the painted galaxy art",
    "pazaakgame_p": "card slots are pinned to the painted board",
    "pazaaksetup_p": "card slots are pinned to the painted board",
    "mainmenu8x6_p": "candidate for later; main menu has its own full-screen 3D view",
}
# Excluded GUIs whose listed controls are still mapped like the content layer maps them
# on the other pause menus (frame-touching bar ends to the frame edge, interior ends and
# other controls by fx). map_p's bottom strip must frame the top_p party portraits, which
# ARE content-mapped; left vanilla, the bar gap and tab notches sit 30-60 units off them
# and leave stray teal edges (2026-09-29). LBL_Map and everything else stay as today.
CONTENT_EXCLUDE_PARTIAL: dict[str, tuple[str, ...]] = {
    "map_p": ("LBL_BAR3", "LBL_BAR4", "LBL_BAR5"),
}
# Controls whose position moves (centre by fx) but whose width is kept. Empty: the
# 3D views (character LBL_3DCHAR/BTN_3DCHAR, classsel 3D_MODEL1-6, maincg MODEL_LBL,
# partyselect LBL_3D, upgrade 3D_MODEL/3D_MODEL_LS) were kept at first, but a kept
# box overshoots its frame by W(1 - k)/2 per side into the neighbouring content (the
# character view covered the stats column, in game 2026-09-25). The engine renders a
# 3D view at the viewport's real pixel aspect (the character was undistorted in a box
# 1.8x too wide), so a view mapped like content only loses backdrop at the sides, and
# it is still k*M = 1.33x wider on screen than at 4:3. The role stays for any control
# that proves to need it.
CONTENT_KEEP: dict[str, tuple[str, ...]] = {}
# Engine-painted pictures. The engine stretches a save's Screen.tga (the whole frame
# squeezed into 256x256, so its content aspect is the screen aspect = M * 4/3) into the
# control rect. Stretch M makes the on-screen box aspect (W/H) * M, so the box must be
# W:H = 4:3 in units to show the thumbnail undistorted. Content mapping narrows the box
# by k (in game 2026-09-29 it was 0.74x squeezed), so the picture keeps its mapped width,
# derives its height from it (letterbox), stays centred on its old vertical band, and the
# overlays named in top/bottom keep their vanilla inset from the picture's top/bottom edge.
CONTENT_PICTURE: dict[str, dict] = {
    "saveload_p": {"picture": "LBL_SCREENSHOT", "top": ("LBL_PCNAME",), "bottom": ("LBL_PLANETNAME",)},
}
PICTURE_ASPECT = 4.0 / 3.0


def picture_extent(top: int, height: int, new_width: int) -> tuple[int, int]:
    """Letterbox a picture box: W:H = 4:3 in units, centred on the old vertical band."""
    new_h = _rh(new_width / PICTURE_ASPECT)
    return _rh(top + height / 2 - new_h / 2), new_h


# Board GUIs (--board-4x3): every control sits on painted slots of the root texture.
# generate_ultrawide_textures.py `frame` squeezes that texture's plate to its true
# 4:3 shape, centred; every control then maps by the same x -> 400 + (x - 400) / M
# (content with k = 1/M, no roles), so cards, text and slots stay registered.
# galaxymap_p: its planet icons sit over 3D_PlanetDisplay, a 3D view the engine draws
# at the real pixel aspect; at k = 1/M the view is its vanilla 4:3 shape again, so the
# icons land on their planets (the part-1 in-place icon spec is replaced).
BOARD_GUIS = {"pazaakgame_p": "pnl_pazaakgam_pc", "pazaaksetup_p": "pnl_pazaakset_pc",
              "galaxymap_p": "pnl_galaxy_pc"}
BACKGROUND_MIN_WIDTH = 760  # a control this wide is a full-screen backdrop: left alone
LIST_EXTENTS = ("PROTOITEM.EXTENT", "SCROLLBAR.EXTENT")

# Popup GUIs (--popup-scale): small non-800x600-root popups the engine renders undersized
# relative to the confirm/loot popups' plain screen/800x600 stretch (datapad_p measured
# ~2.2x smaller on screen than that model predicts, 2026-09-27). Root and every child are
# magnified in place about the popup's own centre instead of the content layer's fx map.
# name -> expected root (L, T, W, H), read live and checked before any edit is made.
POPUP_GUIS: dict[str, tuple[int, int, int, int]] = {
    "datapad_p": (134, 88, 400, 300),
}

# Every GUI whose root BORDER.FILL is pnl_pause_pc (chitin + override scan, 2026-09-24).
# Kept as the regression reference for discover_root_fills(); the generator now
# discovers frame GUIs from the templates and re-checks the root fill at run time.
FRAME_GUIS = [
    "abchrgen_p", "abilities_p", "character_p", "classsel_p", "equip_p", "ftchrgen_p",
    "inventory_p", "journal_p", "maincg_p", "map_p", "messages_p", "optautopause_p",
    "optfeedback_p", "optgameplay_p", "optgraphicadv_p", "optgraphics_p", "optionsingame_p",
    "optionsmain_p", "optkeymapping_p", "optmouse_p", "optsound_p", "optsoundadv_p",
    "partyselect_p", "portcust_p", "pwrlvlup_p", "saveload_p", "skchrgen_p",
    "titlemovie_p", "titlemusic_p",
]

_EQUIP_SLOTS = [
    "IMPLANT", "HEAD", "HANDS", "ARM_L", "BODY", "ARM_R", "BELT",
    "WEAP_L", "WEAP_R", "WEAP_L2", "WEAP_R2",
]
_CHRGEN_ATTRS = ["STR", "DEX", "CON", "INT", "WIS", "CHA"]
_CHRGEN_SKILLS = ["COM", "DEM", "STE", "AWA", "PER", "REP", "SEC", "TRE"]

# Menu overrides. Only controls that hold textures or icons are listed: text-only
# labels keep their centre anyway and need the width for their glyphs.
SPECS_MENUS = {
    "portcust_p": {
        "clusters": [
            (231.0, ["LBL_PORTRAIT", "BTN_ARRL", "BTN_ARRR"]),
        ],
    },
    "top_p": {
        "clusters": [
            # Top navigation tabs (right-anchored to keep right margin aligned)
            (
                729.0,
                [
                    "LBLH_EQU", "BTN_EQU", "LBLH_INV", "BTN_INV",
                    "LBLH_CHA", "BTN_CHAR", "LBLH_ABI", "BTN_ABI",
                    "LBLH_MSG", "BTN_MSG", "LBLH_JOU", "BTN_JOU",
                    "LBLH_MAP", "BTN_MAP", "LBLH_OPT", "BTN_OPT",
                ],
            ),
            # Party portrait cluster on top bar
            (
                583.0,
                [
                    "LBL_BACK1", "LBL_CHAR1", "LBL_BACK2", "LBL_CHAR2", "BTN_CHANGE2",
                    "LBL_BACK3", "LBL_CHAR3", "BTN_CHANGE3",
                    "PB_VIT1", "PB_FORCE1",
                    "LBL_LEVELUP1", "LBL_LEVELUP2", "LBL_LEVELUP3",
                    "LBL_DISABLE1", "LBL_DEBILATATED1",
                    "LBL_CMBTEFCTINC1", "LBL_CMBTEFCTRED1",
                ],
            ),
        ],
    },
    "partyselect_p": {
        "clusters": [
            # Center of the 5-column honeycomb grid (Col centers: 78, 118, 201, 284, 325)
            (
                241.0,
                [f"LBL_CHAR{i}" for i in range(12)]
                + [f"BTN_NPC{i}" for i in range(12)]
                + [f"LBL_NA{i}" for i in range(12)],
            ),
        ],
    },
    "maincg_p": {
        "clusters": [
            (124.0, ["LBL_PORTBORDER", "PORTRAIT_LBL"]),
        ],
    },
    "saveload_p": {
        "clusters": [
            # Party member portraits (LBL_SCREENSHOT is letterboxed to 4:3 units by
            # CONTENT_PICTURE, so the squeezed save thumbnail un-squeezes on screen)
            (574.0, ["LBL_PM1", "LBL_PM2", "LBL_PM3"]),
        ],
    },
    "equip_p": {
        "clusters": [
            (
                SELF,
                [f"{p}_INV_{s}" for s in _EQUIP_SLOTS for p in ("BTN", "LBL")]
                + ["LBL_DEF_BACK"],
            ),
        ],
    },
    "character_p": {
        "clusters": [
            (128.0, ["SLD_ALIGN"]),
        ],
    },
    "upgrade_p": {
        "clusters": [
            (
                SELF,
                [f"{p}_UPGRADE{i}_LS" for i in range(31, 37) for p in ("BTN", "LBL")]
                + [f"{p}_UPGRADE{i}" for i in range(31, 34) for p in ("BTN", "LBL")],
            ),
        ],
    },
    "galaxymap_p": {
        "clusters": [
            (
                SELF,
                [
                    "LBL_Planet_MalachorV", "LBL_Planet_Dantooine", "LBL_Planet_NarShaddaa",
                    "LBL_Planet_PeragusII", "LBL_Planet_Dxun", "LBL_Planet_Korriban",
                    "LBL_Planet_Telos", "LBL_Planet_Onderon", "LBL_Planet_M478",
                    "LBL_Planet_Citadel", "LBL_Tutorial", "LBL_EbonHawk",
                    # added by mod136 / Echoes of the Republic
                    "LBL_Planet_COR", "GUI_slehe_1", "gui_yavin_1", "lbl_imana", "lbl_itato",
                ],
            ),
        ],
    },
    "statussummary_p": {
        "clusters": [
            # Root is a (159,56,322,586) popup; icons sit at L=10, descriptions at L=52.
            (
                10.0,
                [
                    "LBL_JOURNAL", "LBL_CREDITS", "LBL_XP", "LBL_STEALTH", "LBL_DARKSIDE",
                    "LBL_LIGHTSIDE", "LBL_NETSHIFT", "LBL_INFLUENCE_RECV", "LBL_INFLUENCE_LOST",
                    "LBL_MAX_FP_GAINED", "LBL_MAX_FP_LOST", "LBL_LOST", "LBL_RECEIVED",
                ],
            ),
        ],
    },
    "swoopup_p": {
        # Root is 640x480; still 4:3, so M is the same.
        "clusters": [
            (
                SELF,
                [f"BTN_UPGRADE{i}" for i in range(31, 37)]
                + [f"LBL_UPGRADE{i}" for i in range(31, 34)]
                + [f"LBL_UPGRADE{i}" for i in range(41, 45)],
            ),
        ],
    },
    "pazaakgame_p": {
        # Board art has painted card slots: scale each card in place, never regrid.
        "clusters": [
            (
                SELF,
                [f"{p}_{side}{i}" for side in ("PLR", "NPC") for i in range(9) for p in ("BTN", "LBL")]
                + [f"{p}_{side}SIDE{i}" for side in ("PLR", "NPC") for i in range(4) for p in ("BTN", "LBL")]
                + [f"LBL_{side}SCORE{i}" for side in ("PLR", "NPC") for i in range(3)]
                + ["LBL_PLRTURN", "LBL_NPCTURN"],
            ),
        ],
    },
    "pazaaksetup_p": {
        "clusters": [
            (
                SELF,
                [f"{p}_AVAIL{r}{c}" for r in range(4) for c in range(6) for p in ("BTN", "LBL")]
                + [f"{p}_CHOSEN{i}" for i in range(10) for p in ("BTN", "LBL")],
            ),
        ],
    },
    "abchrgen_p": {
        "clusters": [
            (SELF, [f"{a}_{d}_BTN" for a in _CHRGEN_ATTRS for d in ("MINUS", "PLUS")]),
        ],
    },
    "skchrgen_p": {
        "clusters": [
            (SELF, [f"{s}_{d}_BTN" for s in _CHRGEN_SKILLS for d in ("MINUS", "PLUS")]),
        ],
    },
    "optgameplay_p": {
        "clusters": [(SELF, ["BTN_DIFFLEFT", "BTN_DIFFRIGHT"])],
    },
    "optgraphicadv_p": {
        "clusters": [
            (
                SELF,
                [f"BTN_{o}{d}" for o in ("ANTIALIAS", "ANISOTROPY", "TEXQUAL") for d in ("LEFT", "RIGHT")],
            ),
        ],
    },
    "optsoundadv_p": {
        "clusters": [(SELF, ["BTN_EAXLEFT", "BTN_EAXRIGHT"])],
    },
    "pazaakwager_p": {
        "clusters": [(SELF, ["BTN_LESS", "BTN_MORE"])],
    },
    "custpnl_p": {
        "clusters": [(SELF, [f"LBL_{i}" for i in range(1, 7)])],
    },
    "leveluppnl_p": {
        "clusters": [(SELF, [f"LBL_{i}" for i in range(1, 6)])],
    },
    "quickpnl_p": {
        "clusters": [(SELF, [f"LBL_{i}" for i in range(1, 4)])],
    },
    "computer_p": {
        "clusters": [(SELF, [f"LBL_BAR{i}" for i in range(3, 7)])],
    },
    "areatrans_p": {
        "clusters": [(SELF, ["LBL_ICON"])],
    },
    "map_p": {
        # LBL_Map is NOT listed. The engine scales the area-map texture from the box
        # HEIGHT (uniformly, 294 units per 256 texels) and anchors it at the box's LEFT
        # edge; the width only clips. Narrowing the box in place (106/W588 -> 237/W327)
        # shifted the map ~131 units right and cut it off at 564 (in-game 2026-09-24).
        # Squeezing the lbl_map* textures instead would misplace notes and the party
        # arrow and squeeze the 1:1 HUD minimap, so the area map stays stretched.
        "clusters": [(SELF, ["BTN_UP", "BTN_DOWN"])],
    },
    "loadscreen_p": {
        # Anchor 400 keeps the logo's place on the pillarboxed loading image.
        "clusters": [(400.0, ["LBL_LOGO"])],
    },
    "mainmenu8x6_p": {
        # Aspyr override. Same kotor2logo texture as LBL_LOGO above, which renders at the
        # texture's own aspect once its box is narrowed (in-game 2026-09-24). Unfixed, the
        # main-menu logo measured 1.80x wide. LBL_BW/LBL_LUCAS have no fill here (the
        # engine sets them), so they wait for an in-game check.
        "clusters": [(SELF, ["LBL_GAMELOGO"])],
    },
}
SPECS_PHASE1 = SPECS_MENUS  # backward-compatible name

# top_p companion alternative, NOT deployed by default (--top-companion-alt).
# Gate: a pause-menu screenshot with 2+ companions. The default compresses the
# companions toward the leader's centre (583), putting CHAR2 at 615; this variant
# scales them about 683 instead, closer to the right tab of the stretched LBL_BAR5.
SPECS_TOP_COMPANION_ALT = {
    "top_p": {
        "clusters": [
            SPECS_MENUS["top_p"]["clusters"][0],
            (
                583.0,
                [
                    "LBL_BACK1", "LBL_CHAR1", "PB_VIT1", "PB_FORCE1", "LBL_LEVELUP1",
                    "LBL_DISABLE1", "LBL_DEBILATATED1", "LBL_CMBTEFCTINC1", "LBL_CMBTEFCTRED1",
                ],
            ),
            (
                683.0,
                [
                    "LBL_BACK2", "LBL_CHAR2", "BTN_CHANGE2", "LBL_LEVELUP2",
                    "LBL_BACK3", "LBL_CHAR3", "BTN_CHANGE3", "LBL_LEVELUP3",
                ],
            ),
        ],
    },
}

# In-game HUD compact layout. UNUSED on the Aspyr build (HUD measured 1:1, see
# module docstring); only emitted with --include-hud.
SPECS_HUD = {
    "mipc28x6_p": {
        "explicit": {
            # Action tray compact layout (stride 22, slot width 20)
            **{f"BTN_ACTION{i}": (12 + 22 * i, 20) for i in range(6)},
            **{f"BTN_ACTIONUP{i}": (12 + 22 * i, 20) for i in range(6)},
            **{f"BTN_ACTIONDOWN{i}": (12 + 22 * i, 20) for i in range(6)},
            **{f"LBL_ACTION{i}": (14 + 22 * i, 17) for i in range(6)},
            "LBL_MOULDING3": (12, 142),
            "LBL_ACTIONDESCBG": (12, 129),
            "LBL_ACTIONDESC": (12, 129),
        },
        "clusters": [
            # Minimap & notifications
            (
                77.0,
                [
                    "LBL_MAPBORDER", "LBL_MAPVIEW", "BTN_MINIMAP", "LBL_ARROW",
                    "BTN_TARGET1", "BTN_TARGET2", "BTN_TARGET3",
                    "LBL_TARGET1", "LBL_TARGET2", "LBL_TARGET3",
                    "LBL_JOURNAL", "LBL_CASH", "LBL_PLOTXP", "LBL_STEALTHXP",
                    "LBL_ITEMRCVD", "LBL_ITEMLOST", "LBL_DARKSHIFT", "LBL_LIGHTSHIFT",
                ],
            ),
            # Party & toggle buttons
            (
                743.0,
                [
                    "LBL_BACK1", "LBL_BACK2", "LBL_BACK3",
                    "LBL_CHAR1", "LBL_CHAR2", "LBL_CHAR3",
                    "BTN_CHAR1", "BTN_CHAR2", "BTN_CHAR3",
                    "LBL_DISABLE1", "LBL_DISABLE2", "LBL_DISABLE3",
                    "LBL_DEBILATATED1", "LBL_DEBILATATED2", "LBL_DEBILATATED3",
                    "LBL_LEVELUP1", "LBL_LEVELUP2", "LBL_LEVELUP3",
                    "PB_VIT1", "PB_VIT2", "PB_VIT3",
                    "PB_FORCE1", "PB_FORCE2", "PB_FORCE3",
                    "LBL_CMBTEFCTINC1", "LBL_CMBTEFCTINC2", "LBL_CMBTEFCTINC3",
                    "LBL_CMBTEFCTRED1", "LBL_CMBTEFCTRED2", "LBL_CMBTEFCTRED3",
                    "TB_STEALTH", "TB_SOLO", "BTN_SWAPWEAPONS", "TB_PAUSE",
                    "LBL_MOULDING1",
                ],
            ),
            # Menu bar buttons
            (
                791.0,
                [
                    "BTN_EQU", "BTN_INV", "BTN_CHAR", "BTN_ABI",
                    "BTN_MSG", "BTN_JOU", "BTN_MAP", "BTN_OPT",
                    "LBL_MENUBG",
                ],
            ),
        ],
    },
}


# HUD action-description fix (--fix-hud-desc). LBL_ACTIONDESC ("Aggressive" etc.) and
# its background are drawn 1:1 like the rest of the HUD but anchored at the screen
# CENTRE: x_px = (L - 400) * s + screen_w / 2 with s = height / 600 px per unit. The
# tray they belong to is anchored at the left edge (x_px = L * s). Measured 2026-09-24
# at 3840x1600 (Screenshot_2026-09-24_21-20-00.png): vanilla L=14 box at x~885 px, the
# formula gives 890. To put the box at L*s again: L' = L - 400 * (M - 1), coefficient
# 1.0 per tag. Achilles' 16:9 Aspyr fix (mod 47832) moved LBL_ACTIONDESC 14 -> -120
# (formula: -119.3) but LBL_ACTIONDESCBG only to -50; our screenshot shows both boxes
# drawn at the same x at vanilla L, so the same shift is applied to both.
HUD_DESC_GUI = "mipc28x6_p"
HUD_DESC_COEFF = {"LBL_ACTIONDESC": 1.0, "LBL_ACTIONDESCBG": 1.0}


def hud_desc_shift(tag: str, stretch_factor: float) -> int:
    """Units to add to the tag's LEFT so the centre-anchored label lines up with the tray."""
    return round(-HUD_DESC_COEFF[tag] * 400.0 * (stretch_factor - 1.0))


def build_hud_desc_spec(stretch_factor: float) -> dict:
    return {HUD_DESC_GUI: {"shift": {t: hud_desc_shift(t, stretch_factor) for t in HUD_DESC_COEFF}}}


# HUD action-tray magnify (--hud-action-scale). The action tray (its moulding, the six
# action buttons, their up/down stack-cycle arrows and icons) is anchored at the
# screen's left edge (x_px = L * s, per the HUD_DESC comment above) and, by observation
# of the same 1:1-scaled render, at the BOTTOM edge (the tray always sits flush with the
# bottom of the screen regardless of resolution): virtual (0, HUD_ANCHOR_Y) is the point
# that maps onto that literal screen corner. Magnifying about that point (not each
# button's own centre, and not the tray's own bounding box) keeps the corner fixed and
# grows the tray up and to the right, per the user's choice 2026-09-28. LBL_ACTIONDESC/BG
# (a different, screen-CENTRE-anchored pair, already handled by --fix-hud-desc) are
# deliberately not included here.
HUD_ACTION_GUI = "mipc28x6_p"
HUD_ANCHOR_Y = 600  # root HEIGHT; the tray's bottom-left corner in virtual space
HUD_ACTION_TAGS = ("LBL_MOULDING3",) + tuple(
    f"{prefix}{i}" for i in range(6) for prefix in ("BTN_ACTION", "BTN_ACTIONUP", "BTN_ACTIONDOWN", "LBL_ACTION")
)


def hud_action_extent(left: int, top: int, width: int, height: int, scale: float) -> tuple[int, int, int, int]:
    """Magnify an action-tray control about virtual (0, HUD_ANCHOR_Y), edge-to-edge like
    popup_child_extent so siblings that touch (e.g. an icon inside its button) keep touching."""
    new_l = _rh(left * scale)
    new_r = _rh((left + width) * scale)
    new_t = HUD_ANCHOR_Y - _rh((HUD_ANCHOR_Y - top) * scale)
    new_b = HUD_ANCHOR_Y - _rh((HUD_ANCHOR_Y - (top + height)) * scale)
    return new_l, new_t, new_r - new_l, new_b - new_t


def apply_hud_action_scale(gff: GFF, scale: float) -> tuple[dict[int, dict[str, tuple[int, int, int, int]]], list[str]]:
    """Magnify every HUD_ACTION_TAGS control in place. Returns ({index: {"EXTENT": original}}, lines)."""
    controls = list(gff.root.get_list("CONTROLS"))
    tag_indices: dict[str, list[int]] = {}
    for idx, control in enumerate(controls):
        if control.exists("TAG"):
            tag_indices.setdefault(control.get_string("TAG"), []).append(idx)
    missing = set(HUD_ACTION_TAGS) - set(tag_indices)
    if missing:
        raise KeyError(f"{HUD_ACTION_GUI}: HUD_ACTION_TAGS not found: {sorted(missing)}")
    dupes = sorted(t for t in HUD_ACTION_TAGS if len(tag_indices[t]) > 1)
    if dupes:
        raise ValueError(f"{HUD_ACTION_GUI}: tags occur more than once: {dupes}")

    originals: dict[int, dict[str, tuple[int, int, int, int]]] = {}
    lines = []
    for tag in HUD_ACTION_TAGS:
        idx = tag_indices[tag][0]
        control = controls[idx]
        old = _extent(control)
        new = hud_action_extent(*old, scale)
        if old == new:
            continue
        originals[idx] = {"EXTENT": old}
        _set_extent4(control, "EXTENT", *new)
        lines.append(
            f"  {tag:<22} L:{old[0]:>4} -> {new[0]:>4} | W:{old[2]:>3} -> {new[2]:>3} | "
            f"T:{old[1]:>4} -> {new[1]:>4} | H:{old[3]:>3} -> {new[3]:>3}"
        )
    return originals, lines


def compute_stretch_factor(width: int, height: int) -> float:
    """Calculate horizontal stretch factor relative to 4:3."""
    return (float(width) / float(height)) / (4.0 / 3.0)


def transform_extent(left: int, width: int, anchor: float, stretch_factor: float) -> tuple[int, int]:
    """Compute aspect-corrected LEFT and WIDTH extents about a cluster anchor."""
    new_left = round(anchor + (left - anchor) / stretch_factor)
    new_width = round(width / stretch_factor)
    return new_left, new_width


def transform_extent_inplace(left: int, width: int, stretch_factor: float) -> tuple[int, int]:
    """Shrink a control about its own centre (half-up rounding, so 236.5 -> 237)."""
    new_width = round(width / stretch_factor)
    new_left = math.floor(left + (width - new_width) / 2 + 0.5)
    return new_left, new_width


def _rh(v: float) -> int:
    """Round half up (236.5 -> 237), unlike Python's round-half-to-even."""
    return math.floor(v + 0.5)


def parse_aspect(text: str) -> float:
    """'16:9' -> 1.777..., also accepts a plain ratio like '1.7778'. Raises ValueError."""
    parts = text.split(":")
    try:
        if len(parts) == 2:
            w, h = float(parts[0]), float(parts[1])
            if w <= 0 or h <= 0:
                raise ValueError
            value = w / h
        elif len(parts) == 1:
            value = float(parts[0])
        else:
            raise ValueError
    except ValueError:
        raise ValueError(f"bad aspect {text!r}; use W:H such as 16:9") from None
    if not math.isfinite(value) or value <= 0:
        raise ValueError(f"bad aspect {text!r}")
    return value


def content_factor(width: int, height: int, aspect: float) -> float:
    """k: the content region's width as a fraction of the 800-unit canvas (clamped to 1).

    The canvas spans the screen aspect width/height; content of `aspect` fills
    aspect / (width / height) of it, e.g. 16:9 at 3840x1600 -> 0.7407.
    """
    if aspect < 4.0 / 3.0 - 1e-9:
        raise ValueError(f"content aspect {aspect:.4f} is narrower than 4:3")
    return min(1.0, aspect / (float(width) / float(height)))


def fx(x: float, k: float) -> float:
    """Map a virtual x into the centred content region."""
    return 400.0 + (x - 400.0) * k


def content_edges(left: int, width: int, k: float) -> tuple[int, int]:
    """Edge map for a content control: both edges by fx."""
    new_l = _rh(fx(left, k))
    new_r = _rh(fx(left + width, k))
    return new_l, new_r - new_l


def icon_extent(left: int, width: int, anchor, k: float, stretch_factor: float) -> tuple[int, int]:
    """Aspect-correct an icon about fx(anchor); anchor SELF means the control's centre.

    At k = 1 this is exactly transform_extent / transform_extent_inplace (same
    rounding), so the content layer adds nothing at 4:3 content.
    """
    new_w = round(width / stretch_factor)
    if anchor == SELF:
        return _rh(fx(left + width / 2, k) - new_w / 2), new_w
    return round(fx(anchor, k) + (left - anchor) / stretch_factor), new_w


def keep_extent(left: int, width: int, k: float) -> tuple[int, int]:
    """Engine-drawn control: move its centre by fx, keep its width."""
    return _rh(fx(left + width / 2, k) - width / 2), width


def popup_root_extent(left: int, top: int, width: int, height: int, scale: float) -> tuple[int, int, int, int]:
    """Magnify a popup's root box by `scale`, keeping its own on-screen centre fixed."""
    new_w = _rh(width * scale)
    new_h = _rh(height * scale)
    new_l = _rh(left - (new_w - width) / 2)
    new_t = _rh(top - (new_h - height) / 2)
    return new_l, new_t, new_w, new_h


def popup_child_extent(left: int, top: int, width: int, height: int, scale: float) -> tuple[int, int, int, int]:
    """A popup child's root-relative extent: every edge maps by a plain *scale about 0.

    Each edge (not each edge plus a separately-rounded width) is rounded, the same way
    content_edges keeps adjoining controls touching after rounding: two children that
    share an edge before scaling (e.g. LB_MESSAGE's right edge and SCROLLBAR's, both at
    left+width) still share it after.
    """
    new_l = _rh(left * scale)
    new_r = _rh((left + width) * scale)
    new_t = _rh(top * scale)
    new_b = _rh((top + height) * scale)
    return new_l, new_t, new_r - new_l, new_b - new_t


def popup_scrollbar_extent(left: int, top: int, width: int, height: int, scale: float) -> tuple[int, int, int, int]:
    """A popup scrollbar: keeps its own width, stays right-aligned to the scaled listbox."""
    new_l = _rh((left + width) * scale) - width
    new_t = _rh(top * scale)
    new_h = _rh(height * scale)
    return new_l, new_t, width, new_h


def bar_extent(left: int, width: int, k: float, e_left: int | None, rule: tuple) -> tuple[int, int]:
    """Frame separator: ends touching the frame go to the frame's inner edge
    (E_left / 800 - E_left, or stay put when the frame is not remapped); interior
    ends follow the content by fx. At k = 1 this is frame_bar_extent."""
    left_max, right_min, _ = rule
    right = left + width
    if left <= left_max:
        new_l = e_left if e_left is not None else left
    else:
        new_l = _rh(fx(left, k))
    if right >= right_min:
        new_r = 800 - e_left if e_left is not None else right
    else:
        new_r = _rh(fx(right, k))
    return new_l, new_r - new_l


def is_game_running() -> bool:
    """Check if swkotor2.exe process is currently active."""
    try:
        output = subprocess.check_output(
            ["ps", "-eo", "pid,args"],
            text=True,
            stderr=subprocess.DEVNULL,
        )
        for line in output.splitlines():
            if re.search(r"[s]wkotor2\.exe", line, re.IGNORECASE):
                return True
    except Exception:
        pass
    return False


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def find_ci(directory: Path, filename: str) -> Path | None:
    """Return the existing file in directory matching filename case-insensitively."""
    exact = directory / filename
    if exact.exists():
        return exact
    if not directory.is_dir():
        return None
    low = filename.lower()
    for p in directory.iterdir():
        if p.name.lower() == low:
            return p
    return None


_CHITIN_GUI_CACHE: dict[str, dict[str, bytes]] = {}


def chitin_gui_bytes(game_dir: Path, gui_name: str) -> bytes | None:
    """Return the chitin (BIF) copy of gui_name.gui, or None. Indexes chitin once per game_dir."""
    key = str(game_dir)
    if key not in _CHITIN_GUI_CACHE:
        index = {}
        chitin_path = game_dir / "chitin.key"
        if chitin_path.exists():
            for res in Chitin(chitin_path, game=None):
                if res.restype().extension.lower() == "gui":
                    index[res.resname().lower()] = res
        _CHITIN_GUI_CACHE[key] = index
    res = _CHITIN_GUI_CACHE[key].get(gui_name.lower())
    return None if res is None else res.data()


def _sources_path(originals_dir: Path) -> Path:
    return originals_dir / "sources.json"


def load_sources(originals_dir: Path) -> dict:
    path = _sources_path(originals_dir)
    if path.exists():
        return json.loads(path.read_text(encoding="utf-8"))
    return {}


def record_source(originals_dir: Path, gui_name: str, source: str) -> None:
    sources = load_sources(originals_dir)
    sources[gui_name] = source
    _sources_path(originals_dir).write_text(json.dumps(sources, indent=2, sort_keys=True), encoding="utf-8")


def template_source(gui_name: str, originals_dir: Path, game_dir: Path) -> str:
    """Where the cached original came from: 'override' or 'chitin'.

    Originals cached before sources.json existed (part 1) are classified by comparing
    them with the chitin copy: identical bytes means deleting the override on rollback
    restores exactly the same content.
    """
    recorded = load_sources(originals_dir).get(gui_name)
    if recorded:
        return recorded
    orig = originals_dir / f"{gui_name}.gui"
    chitin = chitin_gui_bytes(game_dir, gui_name)
    source = "chitin" if chitin is not None and orig.exists() and orig.read_bytes() == chitin else "override"
    record_source(originals_dir, gui_name, source)
    return source


def resolve_template(
    gui_name: str,
    originals_dir: Path,
    override_dir: Path,
    game_dir: Path,
    manifest: dict | None = None,
) -> tuple[bytes, str]:
    """Locate template GUI file using hierarchy: originals/ -> live override/ -> chitin.

    If originals/ lost the file but the live override is still our own deploy
    (its sha equals the manifest's deployed_sha256), refuse: using it as the
    template would shrink every control a second time.
    """
    orig_file = originals_dir / f"{gui_name}.gui"
    if orig_file.exists():
        return orig_file.read_bytes(), f"cached original ({orig_file.name})"

    live_file = find_ci(override_dir, f"{gui_name}.gui")
    if live_file is not None:
        data = live_file.read_bytes()
        deployed = (manifest or {}).get(gui_name, {}).get("deployed_sha256")
        if deployed and sha256(data) == deployed:
            raise RuntimeError(
                f"{live_file.name} is this tool's own deploy but {orig_file} is missing; "
                "restore originals/ or run --rollback and reinstall the file's source first"
            )
        originals_dir.mkdir(parents=True, exist_ok=True)
        orig_file.write_bytes(data)
        record_source(originals_dir, gui_name, "override")
        return data, f"live override backed up to originals ({live_file.name})"

    data = chitin_gui_bytes(game_dir, gui_name)
    if data is not None:
        originals_dir.mkdir(parents=True, exist_ok=True)
        orig_file.write_bytes(data)
        record_source(originals_dir, gui_name, "chitin")
        return data, "chitin.key extraction"

    raise FileNotFoundError(f"Could not locate template for {gui_name}.gui in originals, override, or chitin")


def _border_fill(struct) -> str:
    if struct.exists("BORDER"):
        border = struct.get_struct("BORDER")
        if border.exists("FILL"):
            return str(border.get_resref("FILL"))
    return ""


def root_fill(gff: GFF) -> str:
    return _border_fill(gff.root)


def _set_extent(control, new_l: int, new_w: int) -> None:
    extent = control.get_struct("EXTENT")
    extent.set_int32("LEFT", new_l)
    extent.set_int32("WIDTH", new_w)
    control.set_struct("EXTENT", extent)


def _extent(control) -> tuple[int, int, int, int]:
    e = control.get_struct("EXTENT")
    return e.get_int32("LEFT"), e.get_int32("TOP"), e.get_int32("WIDTH"), e.get_int32("HEIGHT")


def _get_lw(control, path: str) -> tuple[int, int]:
    s = control
    for part in path.split("."):
        s = s.get_struct(part)
    return s.get_int32("LEFT"), s.get_int32("WIDTH")


def _set_lw(control, path: str, new_l: int, new_w: int) -> None:
    if path == "EXTENT":
        _set_extent(control, new_l, new_w)
        return
    parent_label, _ = path.split(".")
    parent = control.get_struct(parent_label)
    _set_extent(parent, new_l, new_w)
    control.set_struct(parent_label, parent)


def _get_extent4(control, path: str) -> tuple[int, int, int, int]:
    s = control
    for part in path.split("."):
        s = s.get_struct(part)
    return s.get_int32("LEFT"), s.get_int32("TOP"), s.get_int32("WIDTH"), s.get_int32("HEIGHT")


def _set_extent4(control, path: str, l: int, t: int, w: int, h: int) -> None:
    if path == "EXTENT":
        extent = control.get_struct("EXTENT")
        extent.set_int32("LEFT", l)
        extent.set_int32("TOP", t)
        extent.set_int32("WIDTH", w)
        extent.set_int32("HEIGHT", h)
        control.set_struct("EXTENT", extent)
        return
    parent_label, _ = path.split(".")
    parent = control.get_struct(parent_label)
    extent = parent.get_struct("EXTENT")
    extent.set_int32("LEFT", l)
    extent.set_int32("TOP", t)
    extent.set_int32("WIDTH", w)
    extent.set_int32("HEIGHT", h)
    parent.set_struct("EXTENT", extent)
    control.set_struct(parent_label, parent)


def _extent_paths(struct, prefix: str = "") -> list[str]:
    """Every nested struct path ending in EXTENT under struct (e.g. SCROLLBAR.EXTENT)."""
    out = []
    for label, ftype, val in struct:
        if ftype != GFFFieldType.Struct:
            continue
        if label == "EXTENT":
            out.append(prefix + label)
        else:
            out += _extent_paths(val, prefix + label + ".")
    return out


def spec_rules(gff: GFF, spec: dict) -> tuple[dict, dict, dict, dict]:
    """Validate a spec against a GUI. Returns (tag -> control, explicit, shift, tag -> anchor).

    Every spec tag must exist exactly once and be listed once.
    """
    explicit = spec.get("explicit", {})
    shift = spec.get("shift", {})
    if set(shift) & set(explicit):
        raise ValueError(f"Tags both shifted and explicit: {sorted(set(shift) & set(explicit))}")
    tag_to_anchor = {}
    for anchor, tags in spec.get("clusters", []):
        for tag in tags:
            if tag in tag_to_anchor or tag in explicit or tag in shift:
                raise ValueError(f"Tag {tag} listed twice in spec")
            tag_to_anchor[tag] = anchor

    controls = gff.root.get_list("CONTROLS")
    tag_counts: dict[str, int] = {}
    for c in controls:
        if c.exists("TAG"):
            tag_counts[c.get_string("TAG")] = tag_counts.get(c.get_string("TAG"), 0) + 1
    tag_map = {c.get_string("TAG"): c for c in controls if c.exists("TAG")}

    # Verify all spec tags exist, exactly once
    all_spec_tags = set(explicit) | set(shift) | set(tag_to_anchor)
    missing = all_spec_tags - set(tag_map.keys())
    if missing:
        raise KeyError(f"Specified tags not found in GUI: {sorted(missing)}")
    dupes = sorted(t for t in all_spec_tags if tag_counts[t] > 1)
    if dupes:
        raise ValueError(f"Specified tags occur more than once in GUI: {dupes}")
    return tag_map, explicit, shift, tag_to_anchor


def apply_gui_overrides(
    gff: GFF,
    spec: dict,
    stretch_factor: float,
) -> tuple[dict[str, tuple[int, int]], list[str]]:
    """Apply transformation to GFF controls. Returns (modified_tag_dict, report_lines)."""
    tag_map, explicit, shift, tag_to_anchor = spec_rules(gff, spec)
    all_spec_tags = set(explicit) | set(shift) | set(tag_to_anchor)
    modified_originals = {}
    report_lines = []

    for tag in sorted(all_spec_tags):
        control = tag_map[tag]
        old_l, old_t, old_w, old_h = _extent(control)

        if tag in explicit:
            new_l, new_w = explicit[tag]
            rule = "explicit"
        elif tag in shift:
            new_l, new_w = old_l + shift[tag], old_w
            rule = f"shift {shift[tag]:+d}"
        elif tag_to_anchor[tag] == SELF:
            new_l, new_w = transform_extent_inplace(old_l, old_w, stretch_factor)
            rule = "in place"
        else:
            anchor = tag_to_anchor[tag]
            new_l, new_w = transform_extent(old_l, old_w, anchor, stretch_factor)
            rule = f"anchor {anchor:g}"

        modified_originals[tag] = (old_l, old_w)
        _set_extent(control, new_l, new_w)

        report_lines.append(
            f"  {tag:<22} L:{old_l:>4} -> {new_l:>4} | W:{old_w:>3} -> {new_w:>3} | T:{old_t:>4} H:{old_h:>3} ({rule})"
        )

    return modified_originals, report_lines


def frame_bar_extent(left: int, width: int, e_left: int) -> tuple[int, int]:
    """Extend a separator that touched the vanilla frame's inner edge to the remapped one."""
    return bar_extent(left, width, 1.0, e_left, FRAME_BAR_RULES[FRAME_TEXTURE])


def _tangent_line(control, tag: str, is_bar: bool) -> str:
    old_l, old_t, old_w, old_h = _extent(control)
    fill = _border_fill(control)
    return (
        f"  {'BAR ' if is_bar else 'keep'} {tag:<22} L:{old_l:>4} T:{old_t:>4} W:{old_w:>4} H:{old_h:>3} "
        f"R:{old_l + old_w:>4} fill={fill or '-'}"
    )


def apply_frame_bars(
    gff: GFF,
    e_left: int,
    skip_tags: set[str] = frozenset(),
    rule: tuple = FRAME_BAR_RULES[FRAME_TEXTURE],
) -> tuple[dict[str, tuple[int, int]], list[str], list[str]]:
    """Apply the frame-bar rule. Returns (modified_tag_dict, report_lines, edge_dump_lines).

    edge_dump_lines lists every control touching the vanilla frame edges (the tangent
    check), marking which ones the rule changed; only uibit_brdr_16we bars should.
    """
    left_max, right_min, bar_fill = rule
    modified = {}
    lines = []
    dump = []
    for control in gff.root.get_list("CONTROLS"):
        tag = control.get_string("TAG") if control.exists("TAG") else ""
        old_l, old_t, old_w, old_h = _extent(control)
        touches = old_l <= left_max or old_l + old_w >= right_min
        if not touches:
            continue
        is_bar = _border_fill(control).lower() == bar_fill
        dump.append(_tangent_line(control, tag, is_bar))
        if not is_bar:
            continue
        if not tag or tag in skip_tags or tag in modified:
            raise ValueError(f"Frame bar {tag!r} is untagged, duplicated or also in the spec")
        new_l, new_w = bar_extent(old_l, old_w, 1.0, e_left, rule)
        modified[tag] = (old_l, old_w)
        _set_extent(control, new_l, new_w)
        lines.append(
            f"  {tag:<22} L:{old_l:>4} -> {new_l:>4} | W:{old_w:>3} -> {new_w:>3} | T:{old_t:>4} H:{old_h:>3} (frame bar)"
        )
    return modified, lines, dump


def classify_control(control, tag: str, anchors: dict, keep: set, rule: tuple | None, board: bool = False,
                     picture: str | None = None) -> str:
    """Content-layer role: icon | picture | keep | bar | background | content (always content on a board)."""
    if board:
        return "content"
    if tag in anchors:
        return "icon"
    if tag == picture:
        return "picture"
    if tag in keep:
        return "keep"
    left, _, width, _ = _extent(control)
    if rule is not None and _border_fill(control).lower() == rule[2] and (left <= rule[0] or left + width >= rule[1]):
        return "bar"
    if width >= BACKGROUND_MIN_WIDTH:
        return "background"
    return "content"


def plan_content(
    control,
    role: str,
    k: float,
    stretch_factor: float,
    anchor=None,
    e_left: int | None = None,
    rule: tuple | None = None,
) -> dict[str, tuple[int, int]]:
    """New (LEFT, WIDTH) per extent path for one root control of a content GUI."""
    paths = set(_extent_paths(control))
    unexpected = paths - {"EXTENT", *LIST_EXTENTS}
    if unexpected:
        raise ValueError(f"unexpected extents {sorted(unexpected)}")
    if role != "content" and paths - {"EXTENT"}:
        raise ValueError(f"a {role} control carries list extents {sorted(paths - {'EXTENT'})}")
    left, width = _get_lw(control, "EXTENT")
    if role == "background":
        return {}
    if role == "icon":
        return {"EXTENT": icon_extent(left, width, anchor, k, stretch_factor)}
    if role == "keep":
        return {"EXTENT": keep_extent(left, width, k)}
    if role == "picture":
        return {"EXTENT": content_edges(left, width, k)}
    if role == "bar":
        return {"EXTENT": bar_extent(left, width, k, e_left, rule)}
    if role != "content":
        raise ValueError(f"unknown role {role!r}")
    out = {"EXTENT": content_edges(left, width, k)}
    if "PROTOITEM.EXTENT" in paths:
        out["PROTOITEM.EXTENT"] = content_edges(*_get_lw(control, "PROTOITEM.EXTENT"), k)
    if "SCROLLBAR.EXTENT" in paths:
        sl, sw = _get_lw(control, "SCROLLBAR.EXTENT")
        out["SCROLLBAR.EXTENT"] = (_rh(fx(sl + sw, k)) - sw, sw)  # stays right-aligned, same width
    return out


ROLES = ("content", "icon", "bar", "keep", "background", "picture")


def apply_content(
    gff: GFF,
    gui_name: str,
    spec: dict | None,
    k: float,
    stretch_factor: float,
    e_left: int | None,
    rule: tuple | None,
    board: bool = False,
) -> tuple[dict[int, dict[str, tuple[int, int]]], list[str], dict[str, int]]:
    """Map every root control into the content region (board: every control by fx alone).

    Returns ({control_index: {path: original (L, W)}}, report_lines, role_counts).
    """
    anchors = {}
    if spec and not board:
        _, explicit, shift, anchors = spec_rules(gff, spec)
        if explicit or shift:
            raise ValueError(f"{gui_name}: explicit/shift specs are not supported with the content layer")
    keep = set() if board else set(CONTENT_KEEP.get(gui_name, ()))
    tags = [c.get_string("TAG") if c.exists("TAG") else "" for c in gff.root.get_list("CONTROLS")]
    missing = keep - set(tags)
    if missing:
        raise KeyError(f"{gui_name}: CONTENT_KEEP tags not found: {sorted(missing)}")
    pic = {} if board or stretch_factor <= 1.0 + 1e-9 else CONTENT_PICTURE.get(gui_name, {})
    pic_tags = ({pic["picture"], *pic["top"], *pic["bottom"]}) if pic else set()
    missing = pic_tags - set(tags)
    if missing:
        raise KeyError(f"{gui_name}: CONTENT_PICTURE tags not found: {sorted(missing)}")
    pic_box = _extent(gff.root.get_list("CONTROLS")[tags.index(pic["picture"])]) if pic else None
    originals: dict[int, dict[str, tuple[int, ...]]] = {}
    lines = []
    counts = {r: 0 for r in ROLES}
    for idx, control in enumerate(gff.root.get_list("CONTROLS")):
        tag = tags[idx]
        if control.exists("CONTROLS") and len(control.get_list("CONTROLS")):
            raise ValueError(f"{gui_name}: control {tag or idx} has nested CONTROLS")
        role = classify_control(control, tag, anchors, keep, rule, board, pic.get("picture"))
        counts[role] += 1
        try:
            plan = plan_content(control, role, k, stretch_factor, anchors.get(tag), e_left, rule)
        except ValueError as exc:
            raise ValueError(f"{gui_name}: {tag or idx}: {exc}") from None
        label = tag or f"#{idx}"
        note = role if role != "icon" else (f"icon, anchor {anchors[tag]:g}" if anchors[tag] != SELF else "icon, in place")
        if tag in pic_tags:
            # Picture family: horizontal by the plan, vertical letterbox; 4-tuple original.
            old4 = _get_extent4(control, "EXTENT")
            new_l, new_w = plan["EXTENT"]
            new_t, new_h = old4[1], old4[3]
            pt, ph = picture_extent(pic_box[1], pic_box[3], _rh(fx(pic_box[0] + pic_box[2], k)) - _rh(fx(pic_box[0], k)))
            if tag == pic["picture"]:
                new_t, new_h = pt, ph
            elif tag in pic["top"]:
                new_t = pt + (old4[1] - pic_box[1])
            else:
                new_t = pt + ph - (pic_box[1] + pic_box[3] - old4[1] - old4[3]) - old4[3]
            new4 = (new_l, new_t, new_w, new_h)
            if new4 != old4:
                originals.setdefault(idx, {})["EXTENT"] = old4
                _set_extent4(control, "EXTENT", *new4)
                lines.append(
                    f"  {label:<22} L:{old4[0]:>4} -> {new_l:>4} | W:{old4[2]:>3} -> {new_w:>3} | "
                    f"T:{old4[1]:>4} -> {new_t:>4} | H:{old4[3]:>3} -> {new_h:>3} (picture 4:3)"
                )
            continue
        for path, (new_l, new_w) in plan.items():
            old_l, old_w = _get_lw(control, path)
            if (old_l, old_w) == (new_l, new_w):
                continue
            originals.setdefault(idx, {})[path] = (old_l, old_w)
            _set_lw(control, path, new_l, new_w)
            owner = control if path == "EXTENT" else control.get_struct(path.split(".")[0])
            _, old_t, _, old_h = _extent(owner)
            name = label if path == "EXTENT" else f"  .{path.split('.')[0]}"
            lines.append(
                f"  {name:<22} L:{old_l:>4} -> {new_l:>4} | W:{old_w:>3} -> {new_w:>3} | T:{old_t:>4} H:{old_h:>3} ({note})"
            )
    return originals, lines, counts


def apply_popup(
    gff: GFF,
    scale: float,
) -> tuple[tuple[int, int, int, int], dict[int, dict[str, tuple[int, int, int, int]]], list[str]]:
    """Magnify a popup's root box and every child control's extents by `scale`.

    Returns (root_original (L,T,W,H), {control_index: {path: original (L,T,W,H)}}, report_lines).
    """
    root_l, root_t, root_w, root_h = _extent(gff.root)
    root_original = (root_l, root_t, root_w, root_h)
    new_root = popup_root_extent(root_l, root_t, root_w, root_h, scale)
    _set_extent4(gff.root, "EXTENT", *new_root)
    lines = [
        f"  {'ROOT':<22} L:{root_l:>4} -> {new_root[0]:>4} | W:{root_w:>3} -> {new_root[2]:>3} | "
        f"T:{root_t:>4} -> {new_root[1]:>4} | H:{root_h:>3} -> {new_root[3]:>3} (popup x{scale:g})"
    ]

    originals: dict[int, dict[str, tuple[int, int, int, int]]] = {}
    for idx, control in enumerate(gff.root.get_list("CONTROLS")):
        tag = control.get_string("TAG") if control.exists("TAG") else ""
        for path in _extent_paths(control):
            old = _get_extent4(control, path)
            new = popup_scrollbar_extent(*old, scale) if path == "SCROLLBAR.EXTENT" else popup_child_extent(*old, scale)
            if old == new:
                continue
            originals.setdefault(idx, {})[path] = old
            _set_extent4(control, path, *new)
            name = tag if path == "EXTENT" else f"  .{path.split('.')[0]}"
            lines.append(
                f"  {name:<22} L:{old[0]:>4} -> {new[0]:>4} | W:{old[2]:>3} -> {new[2]:>3} | "
                f"T:{old[1]:>4} -> {new[1]:>4} | H:{old[3]:>3} -> {new[3]:>3}"
            )
    return root_original, originals, lines


def apply_partial_content(
    gff: GFF,
    gui_name: str,
    k: float,
    stretch_factor: float,
    e_left: int | None,
    rule: tuple | None,
    modified: dict[str, tuple[int, int]],
) -> list[str]:
    """Map the CONTENT_EXCLUDE_PARTIAL controls of an otherwise excluded GUI.

    A frame-bar rule that already moved a bar is overridden, not compounded; `modified` (tag -> original (L, W)) gets the
    originals for the self-check.
    """
    tags = CONTENT_EXCLUDE_PARTIAL[gui_name]
    controls = {c.get_string("TAG"): c for c in gff.root.get_list("CONTROLS") if c.exists("TAG")}
    missing = set(tags) - set(controls)
    if missing:
        raise KeyError(f"{gui_name}: CONTENT_EXCLUDE_PARTIAL tags not found: {sorted(missing)}")
    lines = []
    for tag in tags:
        control = controls[tag]
        cur_l, cur_w = _get_lw(control, "EXTENT")
        old_l, old_w = modified.get(tag, (cur_l, cur_w))
        # bar_extent is idempotent on a frame-bar-moved bar (its ends stay past the rule's
        # thresholds), so planning from the current edges equals planning from the originals
        role = classify_control(control, tag, {}, set(), rule)
        if role == "background":
            continue
        new_l, new_w = plan_content(control, role, k, stretch_factor, None, e_left, rule)["EXTENT"]
        _set_lw(control, "EXTENT", new_l, new_w)
        modified[tag] = (old_l, old_w)
        _, old_t, _, old_h = _extent(control)
        lines.append(
            f"  {tag:<22} L:{old_l:>4} -> {new_l:>4} | W:{old_w:>3} -> {new_w:>3} | T:{old_t:>4} H:{old_h:>3} "
            f"({role}, partial content)"
        )
    return lines


def tangent_dump(gff: GFF, rule: tuple | None) -> list[str]:
    """Every control touching the vanilla frame's inner edges, marking the bars."""
    left_max, right_min, bar_fill = rule or FRAME_BAR_RULES[FRAME_TEXTURE]
    dump = []
    for control in gff.root.get_list("CONTROLS"):
        left, _, width, _ = _extent(control)
        if left <= left_max or left + width >= right_min:
            tag = control.get_string("TAG") if control.exists("TAG") else ""
            dump.append(_tangent_line(control, tag, rule is not None and _border_fill(control).lower() == bar_fill))
    return dump


def self_verify_indexed(
    template_gff: GFF,
    modified_gff: GFF,
    originals: dict[int, dict[str, tuple[int, ...]]],
    root_original: tuple[int, int, int, int] | None = None,
) -> bool:
    """Serialize and reload the output, restore every recorded extent by control index
    and extent path (2-tuple (L, W), or 4-tuple (L, T, W, H) for --popup-scale, which also
    remaps the root itself), and require the result to be byte-identical to the template."""
    reloaded_gff = read_gff(bytes_gff(modified_gff, ResourceType.GUI))
    if root_original is not None:
        _set_extent4(reloaded_gff.root, "EXTENT", *root_original)
    controls = list(reloaded_gff.root.get_list("CONTROLS"))
    for idx, paths in originals.items():
        for path, orig in paths.items():
            if len(orig) == 4:
                _set_extent4(controls[idx], path, *orig)
            else:
                _set_lw(controls[idx], path, *orig)

    # Byte comparison of both serializations. pykotor's GFFStruct.compare is not usable
    # here: its list compare returns True even when it reports entries added/removed,
    # and it passes message_type= to log_func, which print() rejects.
    same = bytes_gff(template_gff, ResourceType.GUI) == bytes_gff(reloaded_gff, ResourceType.GUI)
    if not same:
        diffs = []
        template_gff.root.compare(reloaded_gff.root, log_func=lambda *a, **k: diffs.append(" ".join(map(str, a))))
        for line in diffs[:20]:
            print(f"  self-check diff: {line.strip()}", file=sys.stderr)
    return same


def _tags_to_indices(gff: GFF, modified_tags: dict[str, tuple[int, int]]) -> dict[int, dict[str, tuple[int, int]]]:
    out = {}
    for idx, control in enumerate(gff.root.get_list("CONTROLS")):
        if control.exists("TAG") and control.get_string("TAG") in modified_tags:
            out[idx] = {"EXTENT": modified_tags[control.get_string("TAG")]}
    return out


def self_verify(template_gff: GFF, modified_gff: GFF, modified_tags: dict[str, tuple[int, int]]) -> bool:
    """Tag-keyed form of self_verify_indexed (EXTENT only)."""
    return self_verify_indexed(template_gff, modified_gff, _tags_to_indices(modified_gff, modified_tags))


def generate_gui(
    gui_name: str,
    template_bytes: bytes,
    spec: dict | None,
    stretch_factor: float,
    e_left: int | None = None,
    content_k: float | None = None,
    board: bool = False,
    popup_scale: float | None = None,
    hud_action_scale: float | None = None,
) -> tuple[bytes, list[str], list[str]]:
    """Transform one GUI. Returns (output_bytes, report_lines, frame_edge_dump_lines).

    e_left: the remapped inner edge of this GUI's frame texture (its root fill), which
    must have a FRAME_BAR_RULES entry. content_k: apply the content layer with this k.
    board: a BOARD_GUIS entry at k = 1/M; its spec is replaced by the uniform map.
    popup_scale: a POPUP_GUIS entry, magnified about its own centre; exclusive of board/content.
    hud_action_scale: magnify HUD_ACTION_GUI's action tray about its screen corner; exclusive
    of board/content/popup.
    """
    template_gff = read_gff(template_bytes)
    work_gff = read_gff(template_bytes)
    fill = root_fill(work_gff).lower()
    rule = FRAME_BAR_RULES.get(fill)
    if e_left is not None and rule is None:
        raise ValueError(f"{gui_name}: root fill is {fill!r}, which has no frame-bar rule ({sorted(FRAME_BAR_RULES)})")

    dump = []
    root_original = None
    if popup_scale is not None:
        if board or content_k is not None or hud_action_scale is not None:
            raise ValueError(f"{gui_name}: --popup-scale is exclusive of board/content/hud-action mode")
        if gui_name not in POPUP_GUIS:
            raise ValueError(f"{gui_name}: not a popup GUI ({sorted(POPUP_GUIS)})")
        root_ext = _extent(work_gff.root)
        if root_ext != POPUP_GUIS[gui_name]:
            raise ValueError(f"{gui_name}: popup layer expects root {POPUP_GUIS[gui_name]}, got {root_ext}")
        root_original, originals, report = apply_popup(work_gff, popup_scale)
    elif hud_action_scale is not None:
        if board or content_k is not None:
            raise ValueError(f"{gui_name}: --hud-action-scale is exclusive of board/content mode")
        if gui_name != HUD_ACTION_GUI:
            raise ValueError(f"{gui_name}: --hud-action-scale only applies to {HUD_ACTION_GUI}")
        originals, report = apply_hud_action_scale(work_gff, hud_action_scale)
    else:
        if board:
            if gui_name not in BOARD_GUIS or fill != BOARD_GUIS[gui_name]:
                raise ValueError(
                    f"{gui_name}: not a board GUI with root fill {BOARD_GUIS.get(gui_name)!r} (got {fill!r})")
            if content_k is None or e_left is not None:
                raise ValueError(f"{gui_name}: board mode needs content_k = 1/M and no frame bars")
        partial = content_k is not None and not board and gui_name in CONTENT_EXCLUDE_PARTIAL
        if content_k is not None and not partial:
            if gui_name in CONTENT_EXCLUDE and not board:
                raise ValueError(f"{gui_name} is excluded from the content layer: {CONTENT_EXCLUDE[gui_name]}")
            root_ext = _extent(work_gff.root)
            if root_ext != (0, 0, 800, 600):
                raise ValueError(f"{gui_name}: content layer needs an 800x600 root, got {root_ext}")
            originals, report, counts = apply_content(work_gff, gui_name, spec, content_k, stretch_factor, e_left,
                                                      rule, board)
            if fill in CONTENT_FILLS:
                dump = tangent_dump(template_gff, rule)
            if report:
                report.append("  roles: " + ", ".join(f"{r}={counts[r]}" for r in ROLES))
        else:
            modified, report = ({}, [])
            if spec:
                modified, report = apply_gui_overrides(work_gff, spec, stretch_factor)
            if e_left is not None:
                bar_mod, bar_report, dump = apply_frame_bars(work_gff, e_left, set(modified), rule)
                modified.update(bar_mod)
                report += bar_report
            if partial:
                report += apply_partial_content(work_gff, gui_name, content_k, stretch_factor, e_left, rule, modified)
            originals = _tags_to_indices(work_gff, modified)

    if not self_verify_indexed(template_gff, work_gff, originals, root_original):
        raise AssertionError(f"Self-verification failed for {gui_name}.gui! GFF altered outside target extents.")
    return bytes_gff(work_gff, ResourceType.GUI), report, dump


def load_manifest(manifest_path: Path) -> dict:
    if manifest_path.exists():
        with open(manifest_path, "r", encoding="utf-8") as f:
            return json.load(f)
    return {}


def save_manifest(manifest_path: Path, manifest: dict) -> None:
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2, sort_keys=True)


def deploy_file(
    gui_name: str,
    data: bytes,
    override_dir: Path,
    originals_dir: Path,
    manifest_path: Path,
    source: str | None = None,
    extra: dict | None = None,
) -> None:
    """Safely deploy file to override/ with conflict verification. `extra` is merged
    into the manifest entry (e.g. content_aspect)."""
    existing = find_ci(override_dir, f"{gui_name}.gui")
    target_path = existing if existing is not None else override_dir / f"{gui_name}.gui"
    out_sha = sha256(data)

    manifest = load_manifest(manifest_path)
    orig_file = originals_dir / f"{gui_name}.gui"
    orig_sha = sha256(orig_file.read_bytes()) if orig_file.exists() else None

    if target_path.exists():
        target_sha = sha256(target_path.read_bytes())
        prev_sha = manifest.get(gui_name, {}).get("deployed_sha256")

        if target_sha != orig_sha and target_sha != prev_sha and target_sha != out_sha:
            raise RuntimeError(
                f"Refusing to overwrite foreign {target_path.name} (SHA: {target_sha[:12]}). "
                f"Does not match recorded original ({orig_sha[:12] if orig_sha else 'none'}) "
                f"or previous deploy ({prev_sha[:12] if prev_sha else 'none'})."
            )

    target_path.write_bytes(data)
    manifest[gui_name] = {
        "deployed_sha256": out_sha,
        "original_sha256": orig_sha,
        "source": source or manifest.get(gui_name, {}).get("source") or "override",
        "timestamp": datetime.datetime.now().isoformat(),
        "target_path": str(target_path),
        **(extra or {}),
    }
    save_manifest(manifest_path, manifest)


def rollback(
    override_dir: Path,
    originals_dir: Path,
    manifest_path: Path,
    game_dir: Path,
    names: list[str] | None = None,
) -> list[str]:
    """Undo deploys recorded in the manifest (only `names`, if given). Returns log lines."""
    manifest = load_manifest(manifest_path)
    log = []
    if names is not None:
        unknown = sorted(set(names) - set(manifest))
        log += [f"  WARN {n}: not in the manifest; nothing to undo" for n in unknown]
    for gui_name in sorted(n for n in manifest if names is None or n in names):
        entry = manifest[gui_name]
        target = Path(entry.get("target_path") or override_dir / f"{gui_name}.gui")
        source = entry.get("source") or template_source(gui_name, originals_dir, game_dir)
        if not target.exists():
            log.append(f"  WARN {gui_name}: {target.name} is gone; dropping manifest entry")
            del manifest[gui_name]
            continue
        live_sha = sha256(target.read_bytes())
        if live_sha != entry["deployed_sha256"]:
            log.append(f"  WARN {gui_name}: {target.name} changed since deploy (sha {live_sha[:12]}); left alone")
            continue
        if source == "override":
            orig = originals_dir / f"{gui_name}.gui"
            if not orig.exists():
                log.append(f"  WARN {gui_name}: no original in {originals_dir}; left alone")
                continue
            target.write_bytes(orig.read_bytes())
            log.append(f"  restored {target.name} from originals/")
        else:
            target.unlink()
            log.append(f"  deleted {target.name} (chitin copy applies again)")
        del manifest[gui_name]
    save_manifest(manifest_path, manifest)
    return log


def build_specs(
    include_hud: bool = False,
    top_companion_alt: bool = False,
    fix_hud_desc: bool = False,
    stretch_factor: float = 1.0,
) -> dict:
    if include_hud and fix_hud_desc:
        raise ValueError("--include-hud already places the action description; drop --fix-hud-desc")
    specs = dict(SPECS_MENUS)
    if top_companion_alt:
        specs.update(SPECS_TOP_COMPANION_ALT)
    if include_hud:
        specs.update(SPECS_HUD)
    if fix_hud_desc:
        specs.update(build_hud_desc_spec(stretch_factor))
    return specs


def peek_template(gui_name: str, originals_dir: Path, override_dir: Path, game_dir: Path) -> bytes | None:
    """The template resolve_template would use, read without caching anything.

    A live override that is our own deploy is fine here: only root fields are read,
    and the generator never changes the root.
    """
    orig = originals_dir / f"{gui_name}.gui"
    if orig.exists():
        return orig.read_bytes()
    live = find_ci(override_dir, f"{gui_name}.gui")
    if live is not None:
        return live.read_bytes()
    return chitin_gui_bytes(game_dir, gui_name)


def discover_root_fills(originals_dir: Path, override_dir: Path, game_dir: Path) -> dict[str, list[str]]:
    """Root fill -> sorted names of the full-screen (800x600 root) GUIs using it.

    Scans every GUI in chitin, override/ and originals/. GUIs without a root fill
    are listed under "".
    """
    chitin_gui_bytes(game_dir, "")  # build the index
    names = set(_CHITIN_GUI_CACHE.get(str(game_dir), {}))
    for d in (override_dir, originals_dir):
        if d.is_dir():
            names |= {p.stem.lower() for p in d.iterdir() if p.suffix.lower() == ".gui"}
    fills: dict[str, list[str]] = {}
    for name in sorted(names):
        data = peek_template(name, originals_dir, override_dir, game_dir)
        try:
            gff = read_gff(data)
            root_ext = _extent(gff.root)
        except Exception:
            continue
        if root_ext[2:] != (800, 600):
            continue
        fills.setdefault(root_fill(gff).lower(), []).append(name)
    return fills


def plan_targets(
    specs: dict,
    fills: dict[str, list[str]],
    frame_edges: dict[str, int] | None,
    content: bool,
    board: bool = False,
    popup: bool = False,
    hud_action: bool = False,
) -> dict:
    """Which GUIs to generate and how: {"names", "bars": {gui: texture}, "content": set,
    "board": {gui: texture}, "popup": set, "hud_action": set}."""
    bars = {}
    for texture in frame_edges or {}:
        if texture in FRAME_BAR_RULES:
            for name in fills.get(texture, []):
                bars[name] = texture
    content_guis = set()
    partial_guis: set[str] = set()
    if content:
        for fill, names in fills.items():
            content_guis |= {n for n in names if fill in CONTENT_FILLS or n in CONTENT_EXTRA}
        content_guis -= set(CONTENT_EXCLUDE)
        partial_guis = {n for n in CONTENT_EXCLUDE_PARTIAL if any(n in names for names in fills.values())}
    boards = {}
    if board:
        boards = {n: t for n, t in BOARD_GUIS.items() if n in fills.get(t, [])}
        for n in boards:
            bars.pop(n, None)
    popups = set(POPUP_GUIS) if popup else set()
    hud_actions = {HUD_ACTION_GUI} if hud_action else set()
    names = list(specs) + sorted((set(bars) | content_guis | partial_guis | set(boards) | popups | hud_actions) - set(specs))
    return {"names": names, "bars": bars, "content": content_guis, "partial": partial_guis, "board": boards, "popup": popups,
            "hud_action": hud_actions}


def generate_all(
    specs: dict,
    stretch_factor: float,
    originals_dir: Path,
    override_dir: Path,
    game_dir: Path,
    out_dir: Path,
    frame_edges: dict[str, int] | None = None,
    manifest: dict | None = None,
    content_k: float | None = None,
    only: list[str] | None = None,
    targets: dict | None = None,
    board_k: float | None = None,
    popup_scale: float | None = None,
    hud_action_scale: float | None = None,
) -> tuple[dict[str, bytes], list[str], list[str]]:
    """Generate every GUI in specs, plus the frame-bar GUIs of each texture in
    frame_edges and, with content_k, the content GUIs (found by discover_root_fills).

    only: generate just these names and leave other out/ files alone.
    Returns (name -> bytes, report lines, frame-edge dump lines).
    """
    if targets is None:
        need = frame_edges or content_k is not None or board_k is not None
        fills = discover_root_fills(originals_dir, override_dir, game_dir) if need else {}
        targets = plan_targets(specs, fills, frame_edges, content_k is not None, board_k is not None,
                               popup_scale is not None, hud_action_scale is not None)
    names = targets["names"]
    if only is not None:
        unknown = sorted(set(only) - set(names))
        if unknown:
            raise ValueError(f"--only names not generated by this run: {unknown}")
        names = [n for n in names if n in only]

    generated = {}
    report = []
    dump_all = []
    for gui_name in names:
        template_bytes, source_desc = resolve_template(gui_name, originals_dir, override_dir, game_dir, manifest)
        texture = targets["bars"].get(gui_name)
        e_left = frame_edges[texture] if texture else None
        is_board = gui_name in targets.get("board", {})
        is_popup = gui_name in targets.get("popup", set())
        is_hud_action = gui_name in targets.get("hud_action", set())
        k = board_k if is_board else (content_k if gui_name in targets["content"] | targets.get("partial", set()) else None)
        out_bytes, lines, dump = generate_gui(gui_name, template_bytes, specs.get(gui_name), stretch_factor, e_left, k,
                                              is_board, popup_scale if is_popup else None,
                                              hud_action_scale if is_hud_action else None)
        if dump:
            dump_all.append(f"{gui_name}:")
            dump_all.extend(dump)
        if not lines:
            # a frame GUI with no bars touching the edge: nothing to override
            report.append(f"\n{gui_name}.gui: no controls to change; not written")
            continue
        (out_dir / f"{gui_name}.gui").write_bytes(out_bytes)
        generated[gui_name] = out_bytes
        n_changed = sum(1 for l in lines if not l.startswith("  roles:"))
        report.append(f"\n{gui_name}.gui ({n_changed} extents modified; template: {source_desc}"
                      + (f"; {'board' if is_board else 'content'} k={k:.4f}" if k is not None else "")
                      + (f"; popup scale={popup_scale:g}" if is_popup else "")
                      + (f"; hud action scale={hud_action_scale:g}" if is_hud_action else "") + "):")
        report.extend(lines)
    if only is None:
        # drop outputs of earlier runs that this run no longer generates (keeps out/ == this run)
        for stale in out_dir.glob("*.gui"):
            if stale.stem not in generated:
                stale.unlink()
    return generated, report, dump_all


def load_frame_edges(path: Path, stretch_factor: float) -> dict[str, int]:
    """Read {texture: E_left} from frame_edges.json, refusing one made for another resolution.

    Accepts the per-texture file ({"pnl_pause_pc": {"E_left": ..., "M": ...}, ...})
    and the older flat file of one texture ({"E_left": ..., "M": ...}).
    """
    raw = json.loads(Path(path).read_text(encoding="utf-8"))
    if "E_left" in raw:
        raw = {raw.get("texture", FRAME_TEXTURE): raw}
    out = {}
    for texture, edges in raw.items():
        if "M" in edges and abs(float(edges["M"]) - stretch_factor) > 1e-4:
            raise ValueError(
                f"{path} [{texture}] was made for M={edges['M']}, this run is M={stretch_factor:.6f}; "
                "re-run generate_ultrawide_textures.py frame with the same --width/--height"
            )
        e_left = int(edges["E_left"])
        limit = FRAME_BAR_RULES[texture][0] if texture in FRAME_BAR_RULES else 400
        if not 0 <= e_left <= limit:
            raise ValueError(f"{texture}: E_left={e_left} outside 0..{limit}")
        out[texture.lower()] = e_left
    return out


def frame_texture_deployed(state_dir: Path, override_dir: Path, texture: str = FRAME_TEXTURE) -> bool:
    """True when the live texture is the remapped one recorded in manifest_tex.json."""
    path = state_dir / "manifest_tex.json"
    if not path.exists():
        return False
    entry = json.loads(path.read_text(encoding="utf-8")).get(texture)
    if not entry:
        return False
    live = Path(entry["target_path"])
    return live.exists() and sha256(live.read_bytes()) == entry["deployed_sha256"]


# Overlay registration at the content k: top_p sits over every pause menu, so its
# tabs and portraits must land on the equip_p/inventory_p geometry below them.
REGISTRATION = {
    "top_p tabs anchor": 729,
    "equip_p LB_DESC right edge": 723,
    "top_p portrait anchor": 583,
    "equip_p LBL_BAR5 left": 409,
    "equip_p LBL_BAR5 right": 716,
    "equip_p LBL_BAR3 interior end": 530,
    "equip_p LBL_BAR4 interior start": 636,
}


def registration_check(k: float) -> tuple[list[str], bool]:
    """Mapped reference points and whether the REGISTRATION constants still register.

    fx is affine and increasing for k > 0, so every ordering and every gap (in units of
    k) is preserved by the mapping: the verdict is decided on the virtual x values. This
    is a constant-only check; it does not verify the engine's real widget geometry.
    """
    if k <= 0:
        raise ValueError("content k must be positive")
    f = {name: fx(x, k) for name, x in REGISTRATION.items()}
    lines = [f"  {name:<34} {x:>4} -> {f[name]:7.1f}" for name, x in REGISTRATION.items()]
    v = REGISTRATION
    portrait = v["top_p portrait anchor"]
    ok = (
        abs(v["top_p tabs anchor"] - v["equip_p LB_DESC right edge"]) <= 6
        and v["equip_p LBL_BAR5 left"] < portrait < v["equip_p LBL_BAR5 right"]
        and v["equip_p LBL_BAR3 interior end"] < portrait < v["equip_p LBL_BAR4 interior start"]
    )
    lines.append(f"  registration: {'OK' if ok else 'MISMATCH'} (REGISTRATION constants only, NOT engine-verified; "
                 "tabs within 6 of the content's right edge; portrait inside LBL_BAR5 between the "
                 "LBL_BAR3/LBL_BAR4 interior ends)")
    return lines, ok


def main():
    parser = argparse.ArgumentParser(description="Generate KOTOR II ultrawide GUI overrides.")
    parser.add_argument("--kotor2-root", default=DEFAULT_KOTOR2_ROOT, help="KOTOR2 game root directory.")
    parser.add_argument("--workspace-dir", default=DEFAULT_WORKSPACE_DIR, help="Mod workspace directory.")
    parser.add_argument("--state-dir", default=None, help="State directory (defaults to $WORKSPACE/ultrawide_gui).")
    parser.add_argument("--out-dir", default=None, help="Output directory (defaults to $STATE_DIR/out).")
    parser.add_argument("--width", type=int, default=3840, help="Target horizontal resolution (default: 3840).")
    parser.add_argument("--height", type=int, default=1600, help="Target vertical resolution (default: 1600).")
    parser.add_argument("--include-hud", action="store_true",
                        help="Include in-game HUD (mipc28x6_p.gui). Not needed on Aspyr: its HUD is 1:1.")
    parser.add_argument("--fix-hud-desc", action="store_true",
                        help="Move the HUD action description (mipc28x6_p LBL_ACTIONDESC/BG) back over the "
                             "action tray; it is drawn centre-anchored at 1:1.")
    parser.add_argument("--top-companion-alt", action="store_true",
                        help="Scale top_p companions about 683 instead of the leader's 583 (gated).")
    parser.add_argument("--frame-edges", default=None,
                        help="frame_edges.json from generate_ultrawide_textures.py frame; enables the frame-bar rule.")
    parser.add_argument("--content-aspect", default=None, metavar="W:H",
                        help="Map menu content into a centred region of this aspect (e.g. 16:9); "
                             "the frame and separators stay full width.")
    parser.add_argument("--board-4x3", action="store_true",
                        help="Map the board GUIs (BOARD_GUIS: pazaak, galaxy map) into a centred 4:3 band; needs their textures "
                             "remapped by generate_ultrawide_textures.py frame first.")
    parser.add_argument("--popup-scale", type=float, default=None, metavar="FACTOR",
                        help="Magnify a popup's root box and children by FACTOR about its own centre "
                             "(POPUP_GUIS: datapad_p, which renders undersized relative to the plain "
                             "screen/800x600 stretch other popups use). Piloted at 2.2 from an in-game measurement.")
    parser.add_argument("--hud-action-scale", type=float, default=None, metavar="FACTOR",
                        help="Magnify the HUD action tray (HUD_ACTION_GUI: mipc28x6_p's action buttons, their "
                             "stack up/down arrows and icons, and the tray moulding) by FACTOR about the "
                             "screen's bottom-left corner, for easier force-power/item selection on ultrawide.")
    parser.add_argument("--only", default=None, metavar="A,B,...",
                        help="Generate (and deploy) only these GUIs; other out/ files and deploys are left alone. "
                             "With --rollback, undo only these.")
    parser.add_argument("--deploy", action="store_true", help="Deploy generated GUIs directly to override/.")
    parser.add_argument("--rollback", action="store_true", help="Undo every deploy recorded in the manifest.")

    args = parser.parse_args()
    if args.width <= 0 or args.height <= 0:
        parser.error("--width and --height must be positive")

    game_dir = Path(args.kotor2_root).resolve()
    workspace_dir = Path(args.workspace_dir).resolve()
    state_dir = Path(args.state_dir).resolve() if args.state_dir else workspace_dir / "ultrawide_gui"
    out_dir = Path(args.out_dir).resolve() if args.out_dir else state_dir / "out"
    originals_dir = state_dir / "originals"
    override_dir = game_dir / "override"
    manifest_path = state_dir / "manifest.json"
    report_path = state_dir / "report.txt"

    out_dir.mkdir(parents=True, exist_ok=True)
    originals_dir.mkdir(parents=True, exist_ok=True)

    if (args.deploy or args.rollback) and is_game_running():
        print("ERROR: swkotor2.exe is currently running. Close the game first.", file=sys.stderr)
        sys.exit(1)

    only = [n.strip().lower() for n in args.only.split(",") if n.strip()] if args.only else None

    if args.rollback:
        print("Rolling back deployed GUIs..." + (f" (only {', '.join(only)})" if only else ""))
        for line in rollback(override_dir, originals_dir, manifest_path, game_dir, only):
            print(line)
        return

    m = compute_stretch_factor(args.width, args.height)
    print(f"Target Resolution: {args.width}x{args.height} | Horizontal Stretch Factor M: {m:.4f}")
    if m < 1.0:
        print("ERROR: screens narrower than 4:3 are squeezed, not stretched; not supported.", file=sys.stderr)
        sys.exit(1)

    content_k = None
    if args.content_aspect:
        try:
            aspect = parse_aspect(args.content_aspect)
            if aspect > args.width / args.height + 1e-9:
                raise ValueError(f"content aspect {args.content_aspect} is wider than the screen "
                                 f"({args.width}x{args.height})")
            content_k = content_factor(args.width, args.height, aspect)
        except ValueError as exc:
            parser.error(str(exc))
        print(f"Content layer: {args.content_aspect} -> k = {content_k:.4f}, "
              f"content x {fx(0, content_k):.1f}..{fx(800, content_k):.1f}")

    frame_edges = None
    if args.frame_edges:
        try:
            frame_edges = load_frame_edges(Path(args.frame_edges), m)
        except ValueError as exc:
            parser.error(str(exc))
        print("Frame-bar rule enabled: " + ", ".join(f"{t} E_left = {e}" for t, e in sorted(frame_edges.items())))

    popup_scale = None
    if args.popup_scale is not None:
        if args.popup_scale <= 0:
            parser.error("--popup-scale must be positive")
        popup_scale = args.popup_scale
        print(f"Popup layer: scale={popup_scale:g} ({', '.join(sorted(POPUP_GUIS))})")

    hud_action_scale = None
    if args.hud_action_scale is not None:
        if args.hud_action_scale <= 0:
            parser.error("--hud-action-scale must be positive")
        hud_action_scale = args.hud_action_scale
        print(f"HUD action-tray layer: scale={hud_action_scale:g} ({HUD_ACTION_GUI})")

    try:
        specs = build_specs(args.include_hud, args.top_companion_alt, args.fix_hud_desc, m)
    except ValueError as exc:
        parser.error(str(exc))

    board_k = content_factor(args.width, args.height, 4.0 / 3.0) if args.board_4x3 else None
    need = frame_edges or content_k is not None or board_k is not None
    fills = discover_root_fills(originals_dir, override_dir, game_dir) if need else {}
    targets = plan_targets(specs, fills, frame_edges, content_k is not None, board_k is not None,
                           popup_scale is not None, hud_action_scale is not None)
    if args.deploy:
        used = {t for n, t in {**targets["bars"], **targets["board"]}.items() if only is None or n in only}
        missing = sorted(t for t in used if not frame_texture_deployed(state_dir, override_dir, t))
        if missing:
            print(f"ERROR: --frame-edges bars and --board-4x3 boards need their remapped texture, but the live "
                  f"{', '.join(missing)} is not the deployed remap. Run generate_ultrawide_textures.py "
                  "frame NAME --deploy first.", file=sys.stderr)
            sys.exit(1)

    try:
        generated, report, dump = generate_all(
            specs, m, originals_dir, override_dir, game_dir, out_dir, frame_edges, load_manifest(manifest_path),
            content_k, only, targets, board_k, popup_scale, hud_action_scale,
        )
    except ValueError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(1)
    print(f"Generated {len(generated)} GUIs; self-verification passed for all.")

    header = [
        "KOTOR II Ultrawide GUI Override Report",
        f"Generated: {datetime.datetime.now().isoformat()}",
        f"Resolution: {args.width}x{args.height} | M = {m:.4f}"
        + ("".join(f" | {t} E_left = {e}" for t, e in sorted((frame_edges or {}).items()))),
    ]
    reg_ok = True
    if content_k is not None:
        header.append(f"Content layer: {args.content_aspect} | k = {content_k:.4f} | content x "
                      f"{fx(0, content_k):.1f}..{fx(800, content_k):.1f} | "
                      f"{len(targets['content'])} content GUIs, excluded: {', '.join(sorted(CONTENT_EXCLUDE))}")
        reg, reg_ok = registration_check(content_k)
        header += ["Overlay registration (virtual x):"] + reg
    if board_k is not None:
        header.append(f"Board layer: 4:3 | k = {board_k:.4f} | {', '.join(sorted(targets['board']))}")
    if popup_scale is not None:
        header.append(f"Popup layer: scale={popup_scale:g} | {', '.join(sorted(targets['popup']))}")
    if hud_action_scale is not None:
        header.append(f"HUD action-tray layer: scale={hud_action_scale:g} | {', '.join(sorted(targets['hud_action']))}")
    if only is not None:
        header.append(f"--only: {', '.join(only)}")
    full_report = header + ["=" * 70] + report
    if dump:
        full_report += [
            "",
            "=" * 70,
            "Frame-edge tangent check: every control with L <= 62 or L+W >= 738 in frame GUIs",
            "(BAR = changed by the frame-bar rule, keep = not a frame bar)",
        ] + dump
    report_path.write_text("\n".join(full_report) + "\n", encoding="utf-8")
    print(f"Report written to: {report_path}")

    if not reg_ok:
        print("ERROR: overlay registration MISMATCH (see 'Overlay registration' in the report); "
              "refusing to deploy.", file=sys.stderr)
        sys.exit(1)

    if args.deploy:
        print("\nDeploying to live override/...")
        failed = []
        for gui_name, data in generated.items():
            source = template_source(gui_name, originals_dir, game_dir)
            extra = {"content_aspect": args.content_aspect if gui_name in targets["content"] and content_k else None,
                     "board_4x3": gui_name in targets["board"],
                     "popup_scale": popup_scale if gui_name in targets.get("popup", set()) and popup_scale else None,
                     "hud_action_scale": hud_action_scale if gui_name in targets.get("hud_action", set())
                     and hud_action_scale else None}
            try:
                deploy_file(gui_name, data, override_dir, originals_dir, manifest_path, source, extra)
            except RuntimeError as exc:
                failed.append(gui_name)
                print(f"  REFUSED: {exc}", file=sys.stderr)
                continue
            print(f"  Deployed: {gui_name}.gui (source: {source})")
        if failed:
            print(f"Deployed {len(generated) - len(failed)}, refused {len(failed)}: {', '.join(failed)}", file=sys.stderr)
            sys.exit(1)
        print("Deployment completed successfully.")
    else:
        print("\nDry-run complete. Files written to out/. Use --deploy to install to override/.")


if __name__ == "__main__":
    main()
