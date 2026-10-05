# Session Handoff & Plan State: patch_manager_mods/25_pc_levelup_filter.md

## 0. Next-Phase Direction
let's use a gear icon for settings instead a hammer.  the button is not a square in property aspect ratio, its stretched. the window for config needs a proper border styling to fully cover the menu below it.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/25_pc_levelup_filter.md`
- **Branch:** `KOTOR-II` (05a5475 doc 27/24: companion-selfheal 0.3.0-dev INSTALLED 10-04 20:44 (dll bd80fb29))
- **Last Landed Commit:**
```
05a5475 doc 27/24: companion-selfheal 0.3.0-dev INSTALLED 10-04 20:44 (dll bd80fb29)
 patch_manager_mods/24_master_plan_status.md | 4 ++--
 patch_manager_mods/27_companion_selfheal.md | 1 +
 2 files changed, 3 insertions(+), 2 deletions(-)
```
- **Dirty Files (120):**
  - `M .claude/skills/plan-handoff/SKILL.md`
  - ` M patch_manager_mods/README.md`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/atlas_coverage.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/cg.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/export_dump_inputs.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/field_census.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/helpers.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_cg.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_export_and_coverage.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_field_census.cpython-312.pyc`

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

## Dead Ends & Anti-Patterns (batch 3a/3c)
- No vanilla gear texture: `list_icons.py --filter 'gear|cog|setting|option|opt|wrench|tool|config'` -> only if_gearhead* (portraits), lbl_icn_opt2/3 (hammer). Don't search again.
- ActionHotbar embeds art via engine texture objects (ActionHotbar.cpp ensureTextures 1537 + healTextures for GL resets; HotbarTextures.inc from scripts/gen_hotbar_textures.py). Sibling-owned: read-only; extracting to _shared_src is a cross-session change.
- ShieldBar precedent: generated TGA in override/ (scripts/gen_shield_bar_art.py), installed by hand.
- GuiKit has no border edge/corner setters (only Fill/Tint/Alpha at border+0x74/0x28/0x24); CSWGuiBorder edge/corner offsets need RE or a different template tag.
- check_gui_ids registry (tests/static/gui_ids.toml) pins the UiCtl enum + count: update it with any control added.
- Scenario/ui tests assume heldResult semantics: tick(real=0) -> icon, real=1 -> Configure.
## Verification Commands
cd research/companion_levelup && python3 tools/parity/parity.py && python3 tools/parity/scenarios.py && python3 tools/parity/mutate.py --jobs 6; cd ../.. && python3 scripts/run_tests.py && .venv/bin/python scripts/check_gui_ids.py
