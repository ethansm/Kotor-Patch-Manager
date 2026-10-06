# Session Handoff & Plan State: patch_manager_mods/25_pc_levelup_filter.md

## 0. Next-Phase Direction
Execute §15 in 25_pc_levelup_filter.md

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/25_pc_levelup_filter.md`
- **Branch:** `KOTOR-II` (e5ffefc doc 27: companion-selfheal 0.5.0-dev INSTALLED 10-05 00:57 (config db096abc, backup selfheal050_install_20261005_005737))
- **Last Landed Commit:**
```
e5ffefc doc 27: companion-selfheal 0.5.0-dev INSTALLED 10-05 00:57 (config db096abc, backup selfheal050_install_20261005_005737)
 patch_manager_mods/27_companion_selfheal.md | 3 +++
 1 file changed, 3 insertions(+)
```
- **Dirty Files (130):**
  - `M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/30_queue_hud.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M .claude/skills/plan-handoff/SKILL.md`
  - ` M AGENTS.md`
  - ` M CLAUDE.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/README.md`
  - ` M patch_manager_mods/_shared_src/GuiKit.h`

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.


## Batch 3d direction (user, 2026-10-05 01:01, verbatim)
"Gear looks good! It needs to have the same background as the button fill. Highlight button border on hover. When the menu is open, keep the gear
button and let it do the same as cancel. UX for the pannel that opens needs work. Solid background, give it a border with rounded corners etc,
make it look and feel vanilla colors." Screenshots: ~/Documents/Screenshot_2026-10-05_01-01-13.png (gear: black interior, no button fill),
~/Documents/Screenshot_2026-10-05_01-01-26.png (page: stats show through the EFX window and through the translucent row fills).

## Live result 1.4.0-dev (log lines 794-825)
`UI page window=efx`, `UI page window back alpha ok=1`, `tex luo_gear/luo_gear_hi: created`, `UI icon fill=luo_gear ok=1`, `UI icon emb=1 hi=1 style=1`,
lvlbtn lines now consistent (shown=1 isVisible=1 / shown=0 isVisible=0). Code behaved as designed; the problems are visual.

## Dead ends (Do NOT attempt)
- One LBL_EFX_BACK clone as the page back: alpha 1 but still see-through live (its tiled uibit_fill_2bt is not opaque on its own; the old 3-stack
  was opaque only by stacking). Need a guaranteed-opaque fill (embedded solid texture via EmbedTex.h) or stacking.
- Gear as the C_ICON fill: replaces the button's own fill, so the interior is black. Needs a second (plain, frameless) label on top or a composited texture.
- Labels have no hilight border (only CSWGuiButton has one at +0xE8); hover highlight must swap border images/tint on the label itself.

## Verification commands
cd research/companion_levelup && python3 tools/parity/parity.py && python3 tools/parity/scenarios.py && python3 tools/parity/scenarios.py --m32 && python3 tools/parity/mutate.py --jobs 6
cd ../.. && python3 scripts/run_tests.py -k "embtex or guikit or level_up" && .venv/bin/python scripts/check_gui_ids.py && python3 scripts/check_shared_headers.py && .venv/bin/python3 scripts/gen_luo_textures.py --check
