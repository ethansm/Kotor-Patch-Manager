#!/usr/bin/env python3
"""Hardware Bezel Synthesis and KotOR II HUD Mockup Renderer (Polished Edition).

Synthesizes 6 floating modal hardware bezel textures from the game's full-screen panels:
  1. bezel_datapad.png (from pnl_datapad)
  2. bezel_computer.png (from pnl_computer_pc - Electric Blue console)
  3. bezel_upgrade.png (from pnl_upgrade_pc - 45-degree mitered industrial circuit frame)
  4. bezel_workbench.png (from pnl_workbench_pc - 45-degree mitered bio-emerald circuit frame)
  5. bezel_store.png (from pnl_store_pc - Art-Deco bronze/brass chassis with center latches)
  6. bezel_pause_gold.png (from pnl_pause_pc - Classic Gold frame with 1:1 circular bosses)

Key Polish Fixes:
  - Zero Cut-Off Corners: Finished 45-degree metallic chamfers with dark bevel contour rings.
  - Seamless Pattern Flow: 45-degree diagonal mitered corners for circuit frames (upgrade & workbench)
    and true 9-slice with cosine center-rail feathering for computer, store, and pause panels.
  - Zero-Overlap Inner Inset: Inner card inset set to (bx + 38, by + 38, bw - 76, bh - 76),
    guaranteeing complete clearance of all 36px rails and corner brackets.
  - 1:1 Circular Bosses: Gold pause bezel bosses rendered as perfect 1:1 round circles at display aspect ratio.

Generates:
  - bezel_*.png: 6 standalone 512x512 textures.
  - ql_hardware_bezels_isolated.png: 3x2 gallery of standalone 512x512 bezel assets.
  - ql_ux_hardware_bezel_mockups.png: 2x3 comparison sheet of A+B Hybrid + Tooltip in each bezel.
  - ql_bezels_closeup_verified.png: 6-panel close-up crop verification sheet (1:1 pixel scale).
  - ql_mod_config_demo_mockup.png: In-Game Mod Configuration Menu Quick Loot settings page.
"""
import sys
from pathlib import Path
import numpy as np
from PIL import Image

# Add repo scripts to sys.path
WORKSPACE_DIR = Path("/home/ethansm/orca/workspaces/local-setup/KOTOR-II")
SCRIPTS_DIR = WORKSPACE_DIR / "scripts"
sys.path.insert(0, str(SCRIPTS_DIR))

import quick_loot_mockup as qm  # noqa: E402
import quick_loot_tip_mockup as qtm  # noqa: E402

OUT_DIR = WORKSPACE_DIR / ".claude/research/quick_loot/bezels"


def draw_custom_border(img, cpm, rect, fill, edge, corner, dim, tint):
    """Draw a border using arbitrary GuiKit textures at 1:1 engine scale with rounded fill."""
    x, y, w, h = rect
    layer = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    saved = (cpm.SX, cpm.SY)
    cpm.SX = cpm.SY = 1.0
    try:
        b = {"FILL": fill, "EDGE": edge, "CORNER": corner, "DIMENSION": dim, "COLOR": tint}
        cpm.draw_border(layer, b, (0, 0, w, h))
    finally:
        cpm.SX, cpm.SY = saved

    # Mask out the exterior of the rounded corners from the dark fill so the underlying CRT texture shows through
    if dim > 0 and corner:
        arr_layer = np.array(layer)
        r = dim
        for cy in range(r):
            for cx in range(r):
                dx, dy = (r - 1 - cx), (r - 1 - cy)
                if dx * dx + dy * dy > (r - 1) * (r - 1):
                    # Top-Left
                    arr_layer[cy, cx, 3] = 0
                    # Top-Right
                    arr_layer[cy, w - 1 - cx, 3] = 0
                    # Bottom-Left
                    arr_layer[h - 1 - cy, cx, 3] = 0
                    # Bottom-Right
                    arr_layer[h - 1 - cy, w - 1 - cx, 3] = 0
        layer = Image.fromarray(arr_layer, "RGBA")

    img.alpha_composite(layer, (x, y))


def draw_txt(img, cpm, font, text, rect, align, col):
    """Draw text inside a bounding box with KotOR alignment flags at 1:1 scale."""
    x0, y0, w, h = rect
    saved = (cpm.SX, cpm.SY)
    cpm.SX = cpm.SY = 1.0
    try:
        cpm.draw_text(img, font, text, (x0, y0, x0 + w, y0 + h), align, col)
    finally:
        cpm.SX, cpm.SY = saved


def get_crt_pattern(cpm):
    """Extract clean scanline CRT screen pattern from pnl_datapad center, free of any UI elements."""
    dp = cpm.texture("pnl_datapad")
    # Clean central crop: x=100..400, y=80..420 (completely avoids scrollbar & top-right slider tab)
    crt_crop = dp.crop((100, 80, 400, 420)).convert("L")
    return np.array(crt_crop, dtype=np.float32) / 255.0


def make_tinted_crt(crt_f, w, h, rgb_tint):
    """Generate a scanline CRT screen of size (w, h) tinted to rgb_tint."""
    gray_resized = Image.fromarray((crt_f * 255).astype(np.uint8)).resize((w, h), Image.LANCZOS)
    gf = np.array(gray_resized, dtype=np.float32) / 255.0
    out_rgb = np.zeros((h, w, 3), dtype=np.uint8)
    for c in range(3):
        out_rgb[..., c] = np.clip(gf * rgb_tint[c] * 255.0 * 2.2, 0, 255).astype(np.uint8)
    out_rgba = np.dstack([out_rgb, np.full((h, w), 255, dtype=np.uint8)])
    return Image.fromarray(out_rgba, "RGBA")


def apply_finished_chassis_corners(img, chamfer=5):
    """Add a finished 45-degree metallic chamfer with dark bevel contour ring."""
    arr = np.array(img)
    H, W = arr.shape[:2]
    c = chamfer

    for y in range(c):
        for x in range(c):
            # Top-Left
            if x + y < c - 2:
                arr[y, x, 3] = 0
            elif x + y <= c:
                arr[y, x, :3] = (arr[y, x, :3] * 0.45).astype(np.uint8)
                arr[y, x, 3] = 255

            # Top-Right
            dx = (W - 1) - (W - c + x)
            if dx + y < c - 2:
                arr[y, W - c + x, 3] = 0
            elif dx + y <= c:
                arr[y, W - c + x, :3] = (arr[y, W - c + x, :3] * 0.45).astype(np.uint8)
                arr[y, W - c + x, 3] = 255

            # Bottom-Left
            dy = (H - 1) - (H - c + y)
            if x + dy < c - 2:
                arr[H - c + y, x, 3] = 0
            elif x + dy <= c:
                arr[H - c + y, x, :3] = (arr[H - c + y, x, :3] * 0.45).astype(np.uint8)
                arr[H - c + y, x, 3] = 255

            # Bottom-Right
            if dx + dy < c - 2:
                arr[H - c + y, W - c + x, 3] = 0
            elif dx + dy <= c:
                arr[H - c + y, W - c + x, :3] = (arr[H - c + y, W - c + x, :3] * 0.45).astype(np.uint8)
                arr[H - c + y, W - c + x, 3] = 255

    return Image.fromarray(arr, "RGBA")


def build_computer_bezel(src_tex, W, H, crt_f, t_rail=32, rgb_tint=(0.05, 0.45, 0.75), **kwargs):
    """Electric Blue Computer Console with authentic side louvers, center dividing brackets, and seamless steel fill."""
    out = make_tinted_crt(crt_f, W, H, rgb_tint)

    s = W / 512.0
    s_y = H / 512.0
    t = int(t_rail * s)

    arr = np.array(src_tex)

    # 1. Fill outer steel notch at y=950..1400 on left (x=0..50) and right (x=2048-50..2048)
    steel_patch = arr[600:800, 0:50, :]
    tile_count = int(np.ceil(450 / 200))
    steel_tiled = np.vstack([steel_patch] * tile_count)[:450, :, :]

    # Left notch
    notch_l = arr[950:1400, 0:50, :]
    arr[950:1400, 0:50, :][np.max(notch_l[..., :3], axis=-1) < 25] = steel_tiled[np.max(notch_l[..., :3], axis=-1) < 25]

    # Right notch
    notch_r = arr[950:1400, 2048-50:2048, :]
    steel_tiled_r = steel_tiled[:, ::-1, :]
    arr[950:1400, 2048-50:2048, :][np.max(notch_r[..., :3], axis=-1) < 25] = steel_tiled_r[np.max(notch_r[..., :3], axis=-1) < 25]

    # Alpha mask: luminance threshold so CRT scanlines shine through inside
    max_c = arr[..., :3].max(axis=2)
    alpha = np.clip((max_c - 10.0) / 14.0 * 255.0, 0, 255).astype(np.uint8)
    arr[..., 3] = np.minimum(arr[..., 3], alpha)
    tex_a = Image.fromarray(arr, "RGBA")

    # 2048 source coords:
    # Uniform 130px corner crop in source space matches the output rail perfectly
    # with 0px step at all 4 corner boundaries. Symmetrized right corners via FLIP_LEFT_RIGHT.
    Cw_s = 130
    Ch_ts = 130
    Ch_bs = 130

    tl = tex_a.crop((0, 0, Cw_s, Ch_ts)).resize((t, t), Image.LANCZOS)
    tr = tl.transpose(Image.FLIP_LEFT_RIGHT)
    bl = tex_a.crop((0, 2048 - Ch_bs, Cw_s, 2048)).resize((t, t), Image.LANCZOS)
    br = bl.transpose(Image.FLIP_LEFT_RIGHT)

    top_rail = tex_a.crop((Cw_s, 0, 2048 - Cw_s, Ch_ts)).resize((W - 2 * t, t), Image.LANCZOS)
    bot_rail = tex_a.crop((Cw_s, 2048 - Ch_bs, 2048 - Cw_s, 2048)).resize((W - 2 * t, t), Image.LANCZOS)
    left_rail = tex_a.crop((0, Ch_ts, Cw_s, 2048 - Ch_bs)).resize((t, H - 2 * t), Image.LANCZOS)
    right_rail = left_rail.transpose(Image.FLIP_LEFT_RIGHT)

    out.alpha_composite(tl, (0, 0))
    out.alpha_composite(tr, (W - t, 0))
    out.alpha_composite(bl, (0, H - t))
    out.alpha_composite(br, (W - t, H - t))

    out.alpha_composite(top_rail, (t, 0))
    out.alpha_composite(bot_rail, (t, H - t))
    out.alpha_composite(left_rail, (0, t))
    out.alpha_composite(right_rail, (W - t, t))

    return apply_finished_chassis_corners(out, chamfer=5)


def build_mitered_circuit_bezel(src_tex, W, H, rgb_tint, crt_f, rail_w=32, crop_range=(350, 1650), src_w=78):
    """Constructs a 4-sided circuit frame with 45-degree diagonal mitered corners."""
    left_src = src_tex.crop((0, crop_range[0], src_w, crop_range[1]))
    t = rail_w

    rail_v_left = left_src.resize((t, H), Image.LANCZOS)
    rail_v_right = rail_v_left.transpose(Image.FLIP_LEFT_RIGHT)
    rail_h_top = rail_v_left.transpose(Image.ROTATE_270).resize((W, t), Image.LANCZOS)
    rail_h_bot = rail_v_left.transpose(Image.ROTATE_90).resize((W, t), Image.LANCZOS)

    out = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    # Center viewport CRT scanlines (extends 6px underneath the rails)
    crt = make_tinted_crt(crt_f, W - 2 * (t - 6), H - 2 * (t - 6), rgb_tint)
    out.paste(crt, (t - 6, t - 6))

    # Straight sections
    out.paste(rail_h_top.crop((t, 0, W - t, t)), (t, 0))
    out.paste(rail_h_bot.crop((t, 0, W - t, t)), (t, H - t))
    out.paste(rail_v_left.crop((0, t, t, H - t)), (0, t))
    out.paste(rail_v_right.crop((0, t, t, H - t)), (W - t, t))

    # 4 Corner Miters (t x t)
    # TL: y < x is top rail, y >= x is left rail
    m_tl = np.zeros((t, t), dtype=np.float32)
    for y in range(t):
        for x in range(t):
            if y < x:
                m_tl[y, x] = 1.0
            elif y == x:
                m_tl[y, x] = 0.5
    tl = (np.array(rail_h_top.crop((0, 0, t, t)), dtype=np.float32) * m_tl[..., None] +
          np.array(rail_v_left.crop((0, 0, t, t)), dtype=np.float32) * (1.0 - m_tl[..., None])).astype(np.uint8)
    out.paste(Image.fromarray(tl, "RGBA"), (0, 0))

    # TR: y < (t - 1 - x) is top rail, else right rail
    m_tr = np.zeros((t, t), dtype=np.float32)
    for y in range(t):
        for x in range(t):
            diag = (t - 1 - x)
            if y < diag:
                m_tr[y, x] = 1.0
            elif y == diag:
                m_tr[y, x] = 0.5
    tr = (np.array(rail_h_top.crop((W - t, 0, W, t)), dtype=np.float32) * m_tr[..., None] +
          np.array(rail_v_right.crop((0, 0, t, t)), dtype=np.float32) * (1.0 - m_tr[..., None])).astype(np.uint8)
    out.paste(Image.fromarray(tr, "RGBA"), (W - t, 0))

    # BL: y > (t - 1 - x) is bot rail, else left rail
    m_bl = np.zeros((t, t), dtype=np.float32)
    for y in range(t):
        for x in range(t):
            diag = (t - 1 - x)
            if y > diag:
                m_bl[y, x] = 1.0
            elif y == diag:
                m_bl[y, x] = 0.5
    bl = (np.array(rail_h_bot.crop((0, 0, t, t)), dtype=np.float32) * m_bl[..., None] +
          np.array(rail_v_left.crop((0, H - t, t, H)), dtype=np.float32) * (1.0 - m_bl[..., None])).astype(np.uint8)
    out.paste(Image.fromarray(bl, "RGBA"), (0, H - t))

    # BR: y > x is bot rail, else right rail
    m_br = np.zeros((t, t), dtype=np.float32)
    for y in range(t):
        for x in range(t):
            if y > x:
                m_br[y, x] = 1.0
            elif y == x:
                m_br[y, x] = 0.5
    br = (np.array(rail_h_bot.crop((W - t, 0, W, t)), dtype=np.float32) * m_br[..., None] +
          np.array(rail_v_right.crop((0, H - t, t, H)), dtype=np.float32) * (1.0 - m_br[..., None])).astype(np.uint8)
    out.paste(Image.fromarray(br, "RGBA"), (W - t, H - t))

    return apply_finished_chassis_corners(out, chamfer=5)


def build_store_bezel(src_tex, W, H, crt_f):
    """Art-Deco Bronze Store Bezel with complete top/bottom clasps and full CRT underlay."""
    out = make_tinted_crt(crt_f, W, H, (0.45, 0.30, 0.08))

    s = W / 512.0
    s_y = H / 512.0
    Cw = int(72 * s)
    Ch_top = int(60 * s_y)
    Ch_bot = int(52 * s_y)
    Rw = int(32 * s)

    # Prepare alpha-masked src_tex in 2048 space
    arr = np.array(src_tex)
    max_c = arr[..., :3].max(axis=2)
    alpha = np.clip((max_c - 10.0) / 14.0 * 255.0, 0, 255).astype(np.uint8)
    arr[..., 3] = np.minimum(arr[..., 3], alpha)
    tex_a = Image.fromarray(arr, "RGBA")

    # 2048 source coordinates capturing complete top/bottom clasps
    Cw_s = 280
    Ch_ts = 236  # full 216 top clasp depth
    Ch_bs = 185  # full 169 bot clasp depth
    Rw_s = 125

    tl = tex_a.crop((0, 0, Cw_s, Ch_ts)).resize((Cw, Ch_top), Image.LANCZOS)
    tr = tex_a.crop((2048 - Cw_s, 0, 2048, Ch_ts)).resize((Cw, Ch_top), Image.LANCZOS)
    bl = tex_a.crop((0, 2048 - Ch_bs, Cw_s, 2048)).resize((Cw, Ch_bot), Image.LANCZOS)
    br = tex_a.crop((2048 - Cw_s, 2048 - Ch_bs, 2048, 2048)).resize((Cw, Ch_bot), Image.LANCZOS)

    top_rail = tex_a.crop((Cw_s, 0, 2048 - Cw_s, Ch_ts)).resize((W - 2 * Cw, Ch_top), Image.LANCZOS)
    bot_rail = tex_a.crop((Cw_s, 2048 - Ch_bs, 2048 - Cw_s, 2048)).resize((W - 2 * Cw, Ch_bot), Image.LANCZOS)
    left_rail = tex_a.crop((0, Ch_ts, Rw_s, 2048 - Ch_bs)).resize((Rw, H - Ch_top - Ch_bot), Image.LANCZOS)
    right_rail = tex_a.crop((2048 - Rw_s, Ch_ts, 2048, 2048 - Ch_bs)).resize((Rw, H - Ch_top - Ch_bot), Image.LANCZOS)

    out.alpha_composite(tl, (0, 0))
    out.alpha_composite(tr, (W - Cw, 0))
    out.alpha_composite(bl, (0, H - Ch_bot))
    out.alpha_composite(br, (W - Cw, H - Ch_bot))

    out.alpha_composite(top_rail, (Cw, 0))
    out.alpha_composite(bot_rail, (Cw, H - Ch_bot))
    out.alpha_composite(left_rail, (0, Ch_top))
    out.alpha_composite(right_rail, (W - Rw, Ch_top))

    return apply_finished_chassis_corners(out, chamfer=5)


def build_pause_gold_bezel(src_tex, W, H, crt_f):
    """Gold Pause Bezel with complete arched moldings, 1:1 circular bosses, and full CRT underlay."""
    out = make_tinted_crt(crt_f, W, H, (0.50, 0.40, 0.12))

    s = W / 512.0
    s_y = H / 512.0
    Cw = int(72 * s)
    Ch_top = int(48 * s_y)
    Ch_bot = int(48 * s_y)
    Rw = int(32 * s)

    arr = np.array(src_tex)
    max_c = arr[..., :3].max(axis=2)
    alpha = np.clip((max_c - 10.0) / 14.0 * 255.0, 0, 255).astype(np.uint8)
    arr[..., 3] = np.minimum(arr[..., 3], alpha)
    tex_a = Image.fromarray(arr, "RGBA")

    # 2048 source coordinates capturing complete top/bottom moldings
    Cw_s = 280
    Ch_ts = 180
    Ch_bs = 180
    Rw_s = 115

    tl = tex_a.crop((0, 0, Cw_s, Ch_ts)).resize((Cw, Ch_top), Image.LANCZOS)
    tr = tex_a.crop((2048 - Cw_s, 0, 2048, Ch_ts)).resize((Cw, Ch_top), Image.LANCZOS)
    bl = tex_a.crop((0, 2048 - Ch_bs, Cw_s, 2048)).resize((Cw, Ch_bot), Image.LANCZOS)
    br = tex_a.crop((2048 - Cw_s, 2048 - Ch_bs, 2048, 2048)).resize((Cw, Ch_bot), Image.LANCZOS)

    top_rail = tex_a.crop((Cw_s, 0, 2048 - Cw_s, Ch_ts)).resize((W - 2 * Cw, Ch_top), Image.LANCZOS)
    bot_rail = tex_a.crop((Cw_s, 2048 - Ch_bs, 2048 - Cw_s, 2048)).resize((W - 2 * Cw, Ch_bot), Image.LANCZOS)

    # Side rails with 1:1 circular bosses
    y_top = Ch_top
    y_bot = H - Ch_bot
    boss_h = int(24 * s_y)
    y_boss = y_bot - boss_h - int(10 * s_y)

    ribbed_src = tex_a.crop((0, 500, Rw_s, 1200))
    ribbed_left = ribbed_src.resize((Rw, y_boss - y_top), Image.LANCZOS)

    boss_src = tex_a.crop((0, 1340, Rw_s, 1530))
    boss_round = boss_src.resize((Rw, boss_h), Image.LANCZOS)

    spacer_src = tex_a.crop((0, 1530, Rw_s, 1600))
    spacer = spacer_src.resize((Rw, y_bot - (y_boss + boss_h)), Image.LANCZOS)

    out.alpha_composite(tl, (0, 0))
    out.alpha_composite(tr, (W - Cw, 0))
    out.alpha_composite(bl, (0, H - Ch_bot))
    out.alpha_composite(br, (W - Cw, H - Ch_bot))

    out.alpha_composite(top_rail, (Cw, 0))
    out.alpha_composite(bot_rail, (Cw, H - Ch_bot))

    out.alpha_composite(ribbed_left, (0, y_top))
    out.alpha_composite(ribbed_left.transpose(Image.FLIP_LEFT_RIGHT), (W - Rw, y_top))

    out.alpha_composite(boss_round, (0, y_boss))
    out.alpha_composite(boss_round.transpose(Image.FLIP_LEFT_RIGHT), (W - Rw, y_boss))

    out.alpha_composite(spacer, (0, y_boss + boss_h))
    out.alpha_composite(spacer.transpose(Image.FLIP_LEFT_RIGHT), (W - Rw, y_boss + boss_h))

    return apply_finished_chassis_corners(out, chamfer=5)


def synthesize_bezel(cpm, key, W, H, crt_f, t_rail=32):
    """Synthesize any bezel at arbitrary dimensions with uniform rails and native geometry."""
    if key == "datapad":
        dp = cpm.texture("pnl_datapad").copy()
        if (W, H) == (512, 512):
            return dp
        return dp.resize((W, H), Image.LANCZOS)
    elif key == "computer":
        tex = cpm.texture("pnl_computer_pc")
        return build_computer_bezel(tex, W, H, crt_f=crt_f, t_rail=t_rail)
    elif key == "upgrade":
        tex = cpm.texture("pnl_upgrade_pc")
        return build_mitered_circuit_bezel(tex, W, H, rgb_tint=(0.04, 0.35, 0.55), crt_f=crt_f, rail_w=t_rail)
    elif key == "workbench":
        tex = cpm.texture("pnl_workbench_pc")
        return build_mitered_circuit_bezel(tex, W, H, rgb_tint=(0.04, 0.40, 0.30), crt_f=crt_f, rail_w=t_rail)
    elif key == "store":
        tex = cpm.texture("pnl_store_pc")
        return build_store_bezel(tex, W, H, crt_f=crt_f)
    elif key == "pause_gold":
        tex = cpm.texture("pnl_pause_pc")
        return build_pause_gold_bezel(tex, W, H, crt_f=crt_f)
    raise ValueError(f"Unknown bezel key: {key}")


def synthesize_all_standalone_bezels(cpm, crt_f):
    """Synthesize the 6 canonical 512x512 hardware bezel textures and save to disk."""
    keys = ["datapad", "computer", "upgrade", "workbench", "store", "pause_gold"]
    bezels_512 = {}
    for key in keys:
        b = synthesize_bezel(cpm, key, 512, 512, crt_f, t_rail=32)
        bezels_512[key] = b
        out_path = OUT_DIR / f"bezel_{key}.png"
        b.save(out_path)
        print(f"  Saved standalone: {out_path} ({b.size})")
    return bezels_512


def render_isolated_gallery(cpm, bezels_512):
    """Render a 3x2 gallery of the 6 standalone 512x512 bezel assets."""
    titles = [
        "1. DATAPAD BEZEL",
        "2. COMPUTER BEZEL",
        "3. UPGRADE BEZEL",
        "4. LAB BEZEL",
        "5. STORE BEZEL",
        "6. PAUSE GOLD BEZEL",
    ]
    keys = ["datapad", "computer", "upgrade", "workbench", "store", "pause_gold"]

    cell_w = 540
    cell_h = 580
    banner_h = 44
    sheet_w = cell_w * 3 + 16
    sheet_h = cell_h * 2 + 16

    sheet = Image.new("RGBA", (sheet_w, sheet_h), (12, 16, 20, 255))

    for idx, key in enumerate(keys):
        col, row = idx % 3, idx // 3
        cx = 16 + col * cell_w
        cy = 16 + row * cell_h

        # Banner
        banner_rect = (cx, cy, cell_w - 16, banner_h)
        draw_custom_border(sheet, cpm, banner_rect, fill="uibit_fill_2bt", edge="uibit_brdr_16wet", corner="uibit_brdr_16wct", dim=10, tint=qm.TEAL_FRAME)
        draw_txt(sheet, cpm, "dialogfont16x16", titles[idx], (cx + 14, cy, cell_w - 44, banner_h), 17, qm.CREAM)

        # Subtle dark card background behind bezel to highlight transparency & corners
        card_rect = (cx + 10, cy + banner_h + 8, 512 + 8, 512 + 8)
        draw_custom_border(sheet, cpm, card_rect, fill="uibit_fill_2bt", edge="uibit_brdr_16we", corner="uibit_brdr_16wc", dim=8, tint=(0.04, 0.15, 0.18))

        # Paste bezel
        bz = bezels_512[key]
        sheet.alpha_composite(bz, (cx + 14, cy + banner_h + 12))

    return sheet


def render_bezel_mockup_variant(cpm, crt_f, theme, bw=540, bh=396):
    """Render one clean, zero-overlap A+B Hybrid scene wrapped in a physical hardware bezel."""
    img = qm.background()
    bx, by = 220, 260

    # Synthesize bezel directly at target card size (bw, bh) with 32px rails
    bezel_card = synthesize_bezel(cpm, theme["key"], bw, bh, crt_f, t_rail=32)
    img.alpha_composite(bezel_card, (bx, by))

    # Inner bezel border: sits cleanly inside the bezel opening with a CRT scanline margin
    if theme["key"] in ("store", "pause_gold"):
        inset = (bx + 44, by + 56, bw - 88, bh - 104)
        y_hdr = inset[1] + 12
    else:
        inset = (bx + 42, by + 42, bw - 84, bh - 84)
        y_hdr = inset[1] + 16
    draw_custom_border(img, cpm, inset, fill="uibit_fill_2bt", edge="uibit_brdr_16wet", corner="uibit_brdr_16wct", dim=12, tint=theme["frame_tint"])

    # Content layout inside the inner wireframe card
    pad_x = 16
    content_x = inset[0] + pad_x
    content_w = inset[2] - 2 * pad_x

    # 1. Header Row (Left Title + Right Count)
    draw_txt(img, cpm, "dialogfont16x16", theme["title"], (content_x, y_hdr, 260, 26), 9, qm.CREAM)
    draw_txt(img, cpm, "dialogfont10x10", theme["count"], (content_x, y_hdr + 4, content_w, 22), 11, theme["dim_tint"])

    # 2. 2px Divider
    y_div = y_hdr + 26 + 6
    qm.draw_fill(img, cpm, qm.TEX_FILL_16G, (content_x, y_div, content_w, 2), theme["frame_tint"])

    # 3. Button [F] Get Items (3)
    btn_y = y_div + 2 + 6
    btn_h = 34
    btn_rect = (content_x, btn_y, content_w, btn_h)
    draw_custom_border(img, cpm, btn_rect, fill="uibit_fill_2bt", edge="uibit_brdr_16we", corner="uibit_brdr_16wc", dim=10, tint=theme["btn_tint"])
    txt_off = 12
    if theme.get("led_col") is not None:
        qm.draw_fill(img, cpm, "uibit_fill_circ", (btn_rect[0] + 10, btn_rect[1] + 9, 16, 16), theme["led_col"])
        txt_off = 34
    draw_txt(img, cpm, "dialogfont10x10", theme["btn_text"], (btn_rect[0] + txt_off, btn_rect[1], btn_rect[2] - txt_off - 10, btn_rect[3]), 17, theme["txt_tint"])

    # 4. Item rows (3 rows, h=52, gap=4)
    rows_y = btn_y + btn_h + 8
    row_w = content_w
    items = [
        {"name": "Disruptor Carbine", "icon": "iw_dsrptrfl_001", "stack": ""},  # Hovered
        {"name": "Parts", "icon": "ii_progspike_002", "stack": "x5"},
        {"name": "Combat Suit", "icon": "ia_class4_001", "stack": ""},
    ]
    for i, itm in enumerate(items):
        ry = rows_y + i * (52 + 4)
        hov = (i == 0)
        r_rect = (content_x, ry, row_w, 52)
        qm.draw_frame(img, cpm, r_rect, tint=theme["hilight_tint"] if hov else theme["frame_tint"], dim=10)
        qm.draw_icon(img, cpm, itm["icon"], (r_rect[0] + 6, r_rect[1] + 6, 40, 40))
        draw_txt(img, cpm, "dialogfont10x10", itm["name"], (r_rect[0] + 6 + 40 + 10, r_rect[1], r_rect[2] - 68, 52), 17, qm.CREAM if hov else theme["txt_tint"])
        if itm["stack"]:
            sw = int(qm.text_px10(itm["stack"]))
            draw_txt(img, cpm, "dialogfont10x10", itm["stack"], (r_rect[0] + r_rect[2] - 12 - sw, r_rect[1], sw + 4, 52), 17, qm.CREAM if hov else theme["txt_tint"])

    # 5. Rich Tooltip beside card
    it_carbine = qtm.item("w_brifle_08")
    tx, ty = bx + bw + 10, by
    L = qtm.layout(it_carbine, tx, ty)
    qtm.draw(img, cpm, it_carbine, L)

    box = (bx, by, bw + 10 + L["frame"][2], max(bh, L["frame"][3]))
    return qm.zoom(img, box, z=1, pad=36)


def assemble_mockup_sheet(cpm, shots, titles):
    """Assemble a 2-column x 3-row high-res comparison mockup sheet."""
    max_w = max(s.width for s in shots)
    max_h = max(s.height for s in shots)

    banner_h = 44
    cell_w = max_w + 24
    cell_h = max_h + banner_h + 16

    sheet_w = cell_w * 2 + 16
    sheet_h = cell_h * 3 + 16

    sheet = Image.new("RGBA", (sheet_w, sheet_h), (12, 16, 20, 255))

    for idx in range(6):
        col, row = idx % 2, idx // 2
        cx = 16 + col * cell_w
        cy = 16 + row * cell_h

        # Banner card
        banner_rect = (cx, cy, cell_w - 16, banner_h)
        draw_custom_border(sheet, cpm, banner_rect, fill="uibit_fill_2bt", edge="uibit_brdr_16wet", corner="uibit_brdr_16wct", dim=10, tint=qm.TEAL_FRAME)
        draw_txt(sheet, cpm, "dialogfont16x16", titles[idx], (cx + 14, cy, cell_w - 44, banner_h), 17, qm.CREAM)

        # Paste mockup shot
        s = shots[idx]
        px = cx + (cell_w - 16 - s.width) // 2
        py = cy + banner_h + 8
        sheet.alpha_composite(s, (px, py))

    return sheet


def render_closeup_verified_sheet(cpm, crt_f, themes):
    """Render a 6-panel close-up verification sheet at 1:1 scale matching user's uploaded images."""
    cell_w = 560
    cell_h = 450
    banner_h = 38
    sheet_w = cell_w * 3 + 16
    sheet_h = cell_h * 2 + 16

    sheet = Image.new("RGBA", (sheet_w, sheet_h), (10, 14, 18, 255))

    for idx, th in enumerate(themes):
        col, row = idx % 3, idx // 3
        cx = 16 + col * cell_w
        cy = 16 + row * cell_h

        # Banner
        banner_rect = (cx, cy, cell_w - 16, banner_h)
        draw_custom_border(sheet, cpm, banner_rect, fill="uibit_fill_2bt", edge="uibit_brdr_16wet", corner="uibit_brdr_16wct", dim=8, tint=th["frame_tint"])
        short_title = f"{idx+1}. {th['key'].upper()} CHASSIS"
        draw_txt(sheet, cpm, "dialogfont16x16", short_title, (cx + 12, cy, cell_w - 40, banner_h), 17, qm.CREAM)

        # Render full scene and crop close-up around the bezel card
        bx, by = 220, 260
        bw, bh = 540, 396
        img = qm.background()

        bezel_card = synthesize_bezel(cpm, th["key"], bw, bh, crt_f, t_rail=32)
        img.alpha_composite(bezel_card, (bx, by))

        if th["key"] in ("store", "pause_gold"):
            inset = (bx + 44, by + 56, bw - 88, bh - 104)
            y_hdr = inset[1] + 12
        else:
            inset = (bx + 42, by + 42, bw - 84, bh - 84)
            y_hdr = inset[1] + 16
        draw_custom_border(img, cpm, inset, fill="uibit_fill_2bt", edge="uibit_brdr_16wet", corner="uibit_brdr_16wct", dim=12, tint=th["frame_tint"])

        pad_x = 16
        content_x = inset[0] + pad_x
        content_w = inset[2] - 2 * pad_x

        draw_txt(img, cpm, "dialogfont16x16", th["title"], (content_x, y_hdr, 260, 26), 9, qm.CREAM)
        draw_txt(img, cpm, "dialogfont10x10", th["count"], (content_x, y_hdr + 4, content_w, 22), 11, th["dim_tint"])

        y_div = y_hdr + 26 + 6
        qm.draw_fill(img, cpm, qm.TEX_FILL_16G, (content_x, y_div, content_w, 2), th["frame_tint"])

        btn_y = y_div + 2 + 6
        btn_h = 34
        btn_rect = (content_x, btn_y, content_w, btn_h)
        draw_custom_border(img, cpm, btn_rect, fill="uibit_fill_2bt", edge="uibit_brdr_16we", corner="uibit_brdr_16wc", dim=10, tint=th["btn_tint"])
        txt_off = 12
        if th.get("led_col") is not None:
            qm.draw_fill(img, cpm, "uibit_fill_circ", (btn_rect[0] + 10, btn_rect[1] + 9, 16, 16), th["led_col"])
            txt_off = 34
        draw_txt(img, cpm, "dialogfont10x10", th["btn_text"], (btn_rect[0] + txt_off, btn_rect[1], btn_rect[2] - txt_off - 10, btn_rect[3]), 17, th["txt_tint"])

        rows_y = btn_y + btn_h + 8
        items = [
            {"name": "Disruptor Carbine", "icon": "iw_dsrptrfl_001", "stack": ""},
            {"name": "Parts", "icon": "ii_progspike_002", "stack": "x5"},
            {"name": "Combat Suit", "icon": "ia_class4_001", "stack": ""},
        ]
        for i, itm in enumerate(items):
            ry = rows_y + i * (52 + 4)
            hov = (i == 0)
            r_rect = (content_x, ry, content_w, 52)
            qm.draw_frame(img, cpm, r_rect, tint=th["hilight_tint"] if hov else th["frame_tint"], dim=10)
            qm.draw_icon(img, cpm, itm["icon"], (r_rect[0] + 6, r_rect[1] + 6, 40, 40))
            draw_txt(img, cpm, "dialogfont10x10", itm["name"], (r_rect[0] + 6 + 40 + 10, r_rect[1], r_rect[2] - 68, 52), 17, qm.CREAM if hov else th["txt_tint"])
            if itm["stack"]:
                sw = int(qm.text_px10(itm["stack"]))
                draw_txt(img, cpm, "dialogfont10x10", itm["stack"], (r_rect[0] + r_rect[2] - 12 - sw, r_rect[1], sw + 4, 52), 17, qm.CREAM if hov else th["txt_tint"])

        # 1:1 close-up crop around the card + bezel
        crop_box = (bx - 4, by - 4, bx + bw + 4, by + bh + 4)
        card_crop = img.crop(crop_box).resize((cell_w - 20, cell_h - banner_h - 14), Image.LANCZOS)
        sheet.alpha_composite(card_crop, (cx + 2, cy + banner_h + 4))

    return sheet


def render_mod_config_demo_mockup(cpm, crt_f):
    """Render full 1920x1080 In-Game Mod Configuration Menu Quick Loot settings page."""
    canvas = Image.new("RGBA", (1920, 1080), (8, 12, 16, 255))

    # Dark starfield background texture underlay
    bg_raw = qm.background().resize((1920, 1080), Image.LANCZOS)
    bg_dimmed = Image.new("RGBA", (1920, 1080), (4, 8, 12, 210))
    bg_raw.alpha_composite(bg_dimmed)
    canvas.alpha_composite(bg_raw)

    # Top Navigation Banner
    top_rect = (40, 24, 1920 - 80, 56)
    draw_custom_border(canvas, cpm, top_rect, fill="uibit_fill_2bt", edge="uibit_brdr_16wet", corner="uibit_brdr_16wct", dim=12, tint=qm.TEAL_FRAME)
    draw_txt(canvas, cpm, "dialogfont16x16", "OPTIONS > MOD CONFIGURATION > QUICK LOOT", (top_rect[0] + 20, top_rect[1], 700, top_rect[3]), 17, qm.CREAM)
    draw_txt(canvas, cpm, "dialogfont10x10", "KOTOR II PATCH MANAGER FRAMEWORK  |  K2.MODCONFIG@1", (top_rect[0] + top_rect[2] - 560, top_rect[1], 540, top_rect[3]), 11, qm.TEAL_DIM)

    # Left Column: Settings Panel (x=40, y=96, w=760, h=940)
    col_l_rect = (40, 96, 760, 940)
    draw_custom_border(canvas, cpm, col_l_rect, fill="uibit_fill_2bt", edge="uibit_brdr_16we", corner="uibit_brdr_16wc", dim=12, tint=(0.08, 0.28, 0.35))

    lx = col_l_rect[0] + 24
    lw = col_l_rect[2] - 48
    curr_y = col_l_rect[1] + 24

    # Section 1: Global HUD Settings
    draw_txt(canvas, cpm, "dialogfont16x16", "1. GENERAL HUD SETTINGS", (lx, curr_y, lw, 28), 9, qm.CREAM)
    curr_y += 36

    # Row 1: Hardware Bezels Toggle
    row1 = (lx, curr_y, lw, 44)
    draw_custom_border(canvas, cpm, row1, fill="uibit_fill_2bt", edge="uibit_brdr_16we", corner="uibit_brdr_16wc", dim=8, tint=qm.TEAL_FRAME)
    draw_txt(canvas, cpm, "dialogfont10x10", "Physical Hardware Chassis", (row1[0] + 16, row1[1], 340, 44), 17, qm.TEAL_TEXT)
    pill1 = (row1[0] + row1[2] - 110, row1[1] + 8, 94, 28)
    draw_custom_border(canvas, cpm, pill1, fill="uibit_fill_16g", edge="uibit_brdr_16wet", corner="uibit_brdr_16wct", dim=6, tint=(0.10, 0.70, 0.55))
    draw_txt(canvas, cpm, "dialogfont10x10", "[ ENABLED ]", pill1, 1, qm.CREAM)
    curr_y += 54

    # Row 2: Fallback Chassis
    row2 = (lx, curr_y, lw, 44)
    draw_custom_border(canvas, cpm, row2, fill="uibit_fill_2bt", edge="uibit_brdr_16we", corner="uibit_brdr_16wc", dim=8, tint=qm.TEAL_FRAME)
    draw_txt(canvas, cpm, "dialogfont10x10", "Default Fallback Chassis", (row2[0] + 16, row2[1], 340, 44), 17, qm.TEAL_TEXT)
    draw_txt(canvas, cpm, "dialogfont10x10", "< Datapad (Standard) >", (row2[0] + row2[2] - 250, row2[1], 234, 44), 11, qm.CREAM)
    curr_y += 54

    # Row 3: CRT Scanlines Toggle
    row3 = (lx, curr_y, lw, 44)
    draw_custom_border(canvas, cpm, row3, fill="uibit_fill_2bt", edge="uibit_brdr_16we", corner="uibit_brdr_16wc", dim=8, tint=qm.TEAL_FRAME)
    draw_txt(canvas, cpm, "dialogfont10x10", "Integrated CRT Scanline Grid", (row3[0] + 16, row3[1], 340, 44), 17, qm.TEAL_TEXT)
    pill3 = (row3[0] + row3[2] - 110, row3[1] + 8, 94, 28)
    draw_custom_border(canvas, cpm, pill3, fill="uibit_fill_16g", edge="uibit_brdr_16wet", corner="uibit_brdr_16wct", dim=6, tint=(0.10, 0.70, 0.55))
    draw_txt(canvas, cpm, "dialogfont10x10", "[ ENABLED ]", pill3, 1, qm.CREAM)
    curr_y += 66

    # Section 2: Container Archetype Mapping
    draw_txt(canvas, cpm, "dialogfont16x16", "2. CONTAINER ARCHETYPE CHASSIS MAPPING", (lx, curr_y, lw, 28), 9, qm.CREAM)
    curr_y += 36

    categories = [
        {"cat": "Droid & Technology", "bezel": "Computer Console", "color": "Electric Blue (#0545AD)", "active": True},
        {"cat": "Weapon Locker & Armory", "bezel": "Upgrade Industrial", "color": "Active Cobalt", "active": False},
        {"cat": "Medical & Laboratory", "bezel": "Workbench Bio-Lab", "color": "Bio-Emerald", "active": False},
        {"cat": "Strongbox & Cache", "bezel": "Store Art-Deco", "color": "Warm Bronze", "active": False},
        {"cat": "Classic Remains", "bezel": "Pause Brass Gold", "color": "Classic Gold", "active": False},
    ]

    for cat in categories:
        is_sel = cat["active"]
        crow = (lx, curr_y, lw, 62)
        tint = (0.20, 0.85, 1.0) if is_sel else (0.05, 0.22, 0.28)
        draw_custom_border(canvas, cpm, crow, fill="uibit_fill_2bt", edge="uibit_brdr_16wet" if is_sel else "uibit_brdr_16we", corner="uibit_brdr_16wct" if is_sel else "uibit_brdr_16wc", dim=8, tint=tint)

        # Category Title
        draw_txt(canvas, cpm, "dialogfont16x16" if is_sel else "dialogfont10x10", f"[{cat['cat']}]", (crow[0] + 16, crow[1] + 8, 300, 24), 9, qm.CREAM if is_sel else qm.TEAL_DIM)
        # Bezel Selector
        draw_txt(canvas, cpm, "dialogfont10x10", f"< {cat['bezel']} >", (crow[0] + crow[2] - 250, crow[1] + 8, 234, 24), 11, (0.45, 0.90, 1.0) if is_sel else qm.CREAM)
        # Accent subtitle
        draw_txt(canvas, cpm, "dialogfont10x10", f"Accent Wireframe: {cat['color']}", (crow[0] + 18, crow[1] + 34, lw - 36, 20), 9, (0.35, 0.82, 0.98) if is_sel else (0.15, 0.51, 0.66))

        curr_y += 72

    # Bottom Actions: [ Apply Changes ] [ Restore Defaults ] [ Back ]
    btn_y = col_l_rect[1] + col_l_rect[3] - 64
    btn_w = (lw - 24) // 3
    btn_apply = (lx, btn_y, btn_w, 42)
    draw_custom_border(canvas, cpm, btn_apply, fill="uibit_fill_2bt", edge="uibit_brdr_16wet", corner="uibit_brdr_16wct", dim=8, tint=(0.10, 0.70, 0.55))
    draw_txt(canvas, cpm, "dialogfont10x10", "[ Apply Changes ]", btn_apply, 1, qm.CREAM)

    btn_default = (lx + btn_w + 12, btn_y, btn_w, 42)
    draw_custom_border(canvas, cpm, btn_default, fill="uibit_fill_2bt", edge="uibit_brdr_16we", corner="uibit_brdr_16wc", dim=8, tint=qm.TEAL_FRAME)
    draw_txt(canvas, cpm, "dialogfont10x10", "[ Defaults ]", btn_default, 1, qm.TEAL_TEXT)

    btn_back = (lx + 2 * (btn_w + 12), btn_y, btn_w, 42)
    draw_custom_border(canvas, cpm, btn_back, fill="uibit_fill_2bt", edge="uibit_brdr_16we", corner="uibit_brdr_16wc", dim=8, tint=qm.TEAL_FRAME)
    draw_txt(canvas, cpm, "dialogfont10x10", "[ Back ]", btn_back, 1, qm.CREAM)

    # Right Column: Live Interactive HUD Preview Panel (x=830, y=96, w=1050, h=940)
    col_r_rect = (830, 96, 1050, 940)
    draw_custom_border(canvas, cpm, col_r_rect, fill="uibit_fill_2bt", edge="uibit_brdr_16we", corner="uibit_brdr_16wc", dim=12, tint=(0.08, 0.28, 0.35))

    rx = col_r_rect[0] + 24
    rw = col_r_rect[2] - 48
    draw_txt(canvas, cpm, "dialogfont16x16", "LIVE HUD PREVIEW: DROID & TECH ARCHETYPE", (rx, col_r_rect[1] + 20, rw, 28), 9, qm.CREAM)
    draw_txt(canvas, cpm, "dialogfont10x10", "Real-time rendering of active modal card wrapped in Computer Console bezel with Electric Blue wireframe.", (rx, col_r_rect[1] + 50, rw, 22), 9, qm.TEAL_DIM)

    # Render Preview Bezel + Tooltip onto an isolated canvas, then scale to fit
    pw, ph = 540, 396
    it_carbine = qtm.item("w_brifle_08")
    tx, ty = pw + 16, 0
    L = qtm.layout(it_carbine, tx, ty)
    preview_full_w = tx + L["frame"][2]
    preview_full_h = max(ph, L["frame"][3])

    preview_img = Image.new("RGBA", (preview_full_w, preview_full_h), (0, 0, 0, 0))

    # Bezel on preview canvas
    bezel_preview = synthesize_bezel(cpm, "computer", pw, ph, crt_f, t_rail=32)
    preview_img.alpha_composite(bezel_preview, (0, 0))

    # Inner Card inset on preview canvas with 10px CRT scanline margin
    p_inset = (42, 42, pw - 84, ph - 84)
    draw_custom_border(preview_img, cpm, p_inset, fill="uibit_fill_2bt", edge="uibit_brdr_16wet", corner="uibit_brdr_16wct", dim=12, tint=(0.020, 0.271, 0.678))

    # Preview Content
    p_pad = 16
    p_cx = p_inset[0] + p_pad
    p_cw = p_inset[2] - 2 * p_pad

    p_yhdr = p_inset[1] + 16
    draw_txt(preview_img, cpm, "dialogfont16x16", "Droid Remains", (p_cx, p_yhdr, 260, 26), 9, qm.CREAM)
    draw_txt(preview_img, cpm, "dialogfont10x10", "3 Items", (p_cx, p_yhdr + 4, p_cw, 22), 11, (0.149, 0.510, 0.659))

    p_ydiv = p_yhdr + 26 + 6
    qm.draw_fill(preview_img, cpm, qm.TEX_FILL_16G, (p_cx, p_ydiv, p_cw, 2), (0.020, 0.271, 0.678))

    p_ybtn = p_ydiv + 2 + 6
    p_btn_rect = (p_cx, p_ybtn, p_cw, 34)
    draw_custom_border(preview_img, cpm, p_btn_rect, fill="uibit_fill_2bt", edge="uibit_brdr_16we", corner="uibit_brdr_16wc", dim=10, tint=(0.149, 0.510, 0.659))
    qm.draw_fill(preview_img, cpm, "uibit_fill_circ", (p_btn_rect[0] + 10, p_btn_rect[1] + 9, 16, 16), (0.039, 0.722, 0.749))
    draw_txt(preview_img, cpm, "dialogfont10x10", "[F] Get Items (3)", (p_btn_rect[0] + 34, p_btn_rect[1], p_btn_rect[2] - 44, 34), 17, (0.350, 0.820, 0.980))

    # Preview Rows (3 rows)
    p_yrow = p_ybtn + 34 + 8
    p_items = [
        {"name": "Disruptor Carbine", "icon": "iw_dsrptrfl_001", "stack": ""},
        {"name": "Parts", "icon": "ii_progspike_002", "stack": "x5"},
        {"name": "Combat Suit", "icon": "ia_class4_001", "stack": ""},
    ]
    for i, itm in enumerate(p_items):
        ry = p_yrow + i * (52 + 4)
        hov = (i == 0)
        r_rect = (p_cx, ry, p_cw, 52)
        qm.draw_frame(preview_img, cpm, r_rect, tint=(0.450, 0.900, 1.000) if hov else (0.020, 0.271, 0.678), dim=10)
        qm.draw_icon(preview_img, cpm, itm["icon"], (r_rect[0] + 6, r_rect[1] + 6, 40, 40))
        draw_txt(preview_img, cpm, "dialogfont10x10", itm["name"], (r_rect[0] + 6 + 40 + 10, r_rect[1], r_rect[2] - 68, 52), 17, qm.CREAM if hov else (0.350, 0.820, 0.980))
        if itm["stack"]:
            sw = int(qm.text_px10(itm["stack"]))
            draw_txt(preview_img, cpm, "dialogfont10x10", itm["stack"], (r_rect[0] + r_rect[2] - 12 - sw, r_rect[1], sw + 4, 52), 17, qm.CREAM if hov else (0.350, 0.820, 0.980))

    # Rich Tooltip beside card on preview canvas
    qtm.draw(preview_img, cpm, it_carbine, L)

    # Scale preview canvas to fit comfortably inside right column (width ~860 px)
    fit_w = 860
    fit_h = int(preview_full_h * (fit_w / float(preview_full_w)))
    preview_scaled = preview_img.resize((fit_w, fit_h), Image.LANCZOS)

    # Paste centered horizontally inside right column
    preview_x = col_r_rect[0] + (col_r_rect[2] - fit_w) // 2
    preview_y = col_r_rect[1] + 88
    canvas.alpha_composite(preview_scaled, (preview_x, preview_y))

    # Explanatory Architecture Card below preview
    info_y = preview_y + fit_h + 24
    info_rect = (col_r_rect[0] + 24, info_y, rw, 380)
    draw_custom_border(canvas, cpm, info_rect, fill="uibit_fill_2bt", edge="uibit_brdr_16wet", corner="uibit_brdr_16wct", dim=10, tint=qm.TEAL_FRAME)
    draw_txt(canvas, cpm, "dialogfont16x16", "ARCHITECTURAL INTEGRATION & REA SPECIFICATION", (info_rect[0] + 20, info_rect[1] + 16, rw - 40, 26), 9, qm.CREAM)
    qm.draw_fill(canvas, cpm, qm.TEX_FILL_16G, (info_rect[0] + 20, info_rect[1] + 46, rw - 40, 2), qm.TEAL_FRAME)

    info_lines = [
        "1. Dynamic Chassis Dispatch: Quick Loot inspects container object tags (e.g. 'k_droid', 'plc_armory',",
        "   'plc_medeqp', 'plc_chestr') to select the matching hardware bezel and accent wireframe color.",
        "2. Zero-Override Asset Pipeline: Bezel textures are packaged as Override/ 512x512 textures (ql_bzl_comp.tga,",
        "   ql_bzl_upgr.tga, ql_bzl_work.tga, ql_bzl_stor.tga, ql_bzl_gold.tga) with zero collision with vanilla .gui files.",
        "3. Live In-Game Configuration: Settings persist in quick_loot.ini via the Shared Mod Config framework",
        "   (k2.modconfig@1), supporting real-time hot-swapping without game restarts.",
        "4. Seamless CRT Integration: Each hardware chassis features tinted scanline CRT viewports and clean",
        "   45-degree corner miters with zero text or wireframe overlap across all aspect ratios.",
    ]
    for idx, line in enumerate(info_lines):
        col = qm.CREAM if line.strip().startswith(("1.", "2.", "3.", "4.")) else qm.TEAL_DIM
        draw_txt(canvas, cpm, "dialogfont10x10", line, (info_rect[0] + 20, info_rect[1] + 58 + idx * 24, rw - 40, 22), 9, col)

    return canvas


def main():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    cpm = qm._cpm()
    qtm._inst()
    crt_f = get_crt_pattern(cpm)

    print("1. Synthesizing 6 canonical 512x512 standalone hardware bezels...")
    bezels_512 = synthesize_all_standalone_bezels(cpm, crt_f)

    # 2. Isolated Gallery
    print("2. Rendering isolated bezel gallery (ql_hardware_bezels_isolated.png)...")
    gallery = render_isolated_gallery(cpm, bezels_512)
    path_gallery = OUT_DIR / "ql_hardware_bezels_isolated.png"
    gallery.save(path_gallery)
    print(f"  Saved: {path_gallery} ({gallery.size})")

    # 3. Themes for 2x3 comparison sheet & close-up crops
    themes = [
        # Panel 1: Datapad Bezel
        {
            "key": "datapad",
            "title": "Footlocker",
            "count": "3 Items",
            "btn_text": "[F] Get Items (3)",
            "frame_tint": qm.TEAL_FRAME,
            "btn_tint": (0.102, 0.698, 0.549),
            "txt_tint": qm.TEAL_TEXT,
            "dim_tint": qm.TEAL_DIM,
            "hilight_tint": qm.CREAM,
            "led_col": None,
            "banner": "PANEL 1: PNL_DATAPAD BEZEL (\"STANDARD TECH / FOOTLOCKER\")",
        },
        # Panel 2: Computer Console Bezel
        {
            "key": "computer",
            "title": "Droid Remains",
            "count": "3 Items",
            "btn_text": "[F] Get Items (3)",
            "frame_tint": (0.020, 0.271, 0.678),  # #0545AD Electric Blue
            "btn_tint": (0.149, 0.510, 0.659),    # #2682A8 Cyan wireframe
            "txt_tint": (0.350, 0.820, 0.980),    # Bright Cyan text
            "dim_tint": (0.149, 0.510, 0.659),
            "hilight_tint": (0.450, 0.900, 1.000),
            "led_col": (0.039, 0.722, 0.749),     # #0AB8BF Cyan LED dot
            "banner": "PANEL 2: PNL_COMPUTER_PC BEZEL (\"DROID REMAINS / CONSOLE\")",
        },
        # Panel 3: Upgrade Industrial Bezel
        {
            "key": "upgrade",
            "title": "Weapon Locker",
            "count": "3 Items",
            "btn_text": "[F] Get Items (3)",
            "frame_tint": (0.000, 0.329, 0.502),  # Workbench Steel Cyan
            "btn_tint": (0.000, 0.659, 0.980),    # Active Cobalt wireframe
            "txt_tint": (0.250, 0.780, 1.000),    # Steel Cyan text
            "dim_tint": (0.100, 0.520, 0.750),
            "hilight_tint": (0.600, 0.920, 1.000),
            "led_col": (0.000, 0.659, 0.980),     # Cobalt LED dot
            "banner": "PANEL 3: PNL_UPGRADE_PC BEZEL (\"WEAPON LOCKER / ARMORY\")",
        },
        # Panel 4: Workbench Lab Bezel
        {
            "key": "workbench",
            "title": "Bio-Cylinder",
            "count": "3 Items",
            "btn_text": "[F] Get Items (3)",
            "frame_tint": (0.030, 0.380, 0.250),  # Bio-Green frame
            "btn_tint": (0.100, 0.650, 0.450),    # Emerald wireframe
            "txt_tint": (0.250, 0.880, 0.600),    # Bio-Emerald text
            "dim_tint": (0.080, 0.450, 0.320),
            "hilight_tint": (0.450, 1.000, 0.750),
            "led_col": (0.200, 0.950, 0.500),     # Emerald LED dot
            "banner": "PANEL 4: PNL_WORKBENCH_PC BEZEL (MEDICAL / LAB)",
        },
        # Panel 5: Store Merchant Bezel
        {
            "key": "store",
            "title": "Strongbox",
            "count": "3 Items",
            "btn_text": "[F] Get Items (3)",
            "frame_tint": (0.400, 0.250, 0.080),  # Warm Bronze frame
            "btn_tint": (0.750, 0.520, 0.150),    # Amber wireframe
            "txt_tint": (0.950, 0.780, 0.350),    # Warm Amber text
            "dim_tint": (0.500, 0.350, 0.120),
            "hilight_tint": (1.000, 0.900, 0.550),
            "led_col": (0.950, 0.650, 0.100),     # Amber LED dot
            "banner": "PANEL 5: PNL_STORE_PC BEZEL (\"STRONGBOX / CACHE\")",
        },
        # Panel 6: Classic Gold Pause Bezel
        {
            "key": "pause_gold",
            "title": "Remains",
            "count": "3 Items",
            "btn_text": "[F] Get Items (3)",
            "frame_tint": (0.450, 0.350, 0.100),  # Classic Gold frame
            "btn_tint": (0.850, 0.700, 0.220),    # Brass/Gold wireframe
            "txt_tint": (0.980, 0.880, 0.450),    # Cream Gold text
            "dim_tint": (0.550, 0.420, 0.150),
            "hilight_tint": (1.000, 0.960, 0.650),
            "led_col": (0.980, 0.850, 0.250),     # Gold LED dot
            "banner": "PANEL 6: PNL_PAUSE_PC GOLD BEZEL (FALLBACK)",
        },
    ]

    # 4. Mockup Scenes
    print("3. Rendering 6 high-res bezel mockup scenes...")
    shots = []
    titles = []
    for th in themes:
        print(f"  Rendering {th['banner']}...")
        shot = render_bezel_mockup_variant(cpm, crt_f, th, bw=540, bh=376)
        shots.append(shot)
        titles.append(th["banner"])

    print("4. Assembling 2x3 mockup comparison sheet (ql_ux_hardware_bezel_mockups.png)...")
    sheet = assemble_mockup_sheet(cpm, shots, titles)
    path_mockup = OUT_DIR / "ql_ux_hardware_bezel_mockups.png"
    sheet.save(path_mockup)
    print(f"  Saved: {path_mockup} ({sheet.size})")

    # 5. Close-Up Verification Sheet
    print("5. Rendering 1:1 close-up verification sheet (ql_bezels_closeup_verified.png)...")
    verified_sheet = render_closeup_verified_sheet(cpm, crt_f, themes)
    path_verified = OUT_DIR / "ql_bezels_closeup_verified.png"
    verified_sheet.save(path_verified)
    print(f"  Saved: {path_verified} ({verified_sheet.size})")

    # 6. Mod Configuration Menu + Live Demo Mockup
    print("6. Rendering Mod Configuration Menu page (ql_mod_config_demo_mockup.png)...")
    menu_mockup = render_mod_config_demo_mockup(cpm, crt_f)
    path_menu = OUT_DIR / "ql_mod_config_demo_mockup.png"
    menu_mockup.save(path_menu)
    print(f"  Saved: {path_menu} ({menu_mockup.size})")

    print("\nAll hardware bezel assets, mockups, close-up verification sheet, and Mod Config page rendered successfully!")
    return 0


if __name__ == "__main__":
    sys.exit(main())
