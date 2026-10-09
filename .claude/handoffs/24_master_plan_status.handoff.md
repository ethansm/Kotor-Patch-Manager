# Session Handoff & Plan State: patch_manager_mods/24_master_plan_status.md

## 0. Next-Phase Direction
Execute CS10: Shared HUD placement: companion queue (doc 30 queue-hud) and selfheal share one right-to-left strip next to each companion card; selfheal claims the right-most slot. Contract via hud-host (doc 31 s5, `HudHost.h`; needs a slot/ReserveIds API) — coordinate with the queue-hud / hud-host owner

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/24_master_plan_status.md`
- **Branch:** `KOTOR-II` (57837e0 feat(npc-diversity): master NPC diversity dispatcher (.kpatch & asset pool))
- **Last Landed Commit:**
```
57837e0 feat(npc-diversity): master NPC diversity dispatcher (.kpatch & asset pool)
 .../34_npc_diversity_dispatcher.handoff.md         |  34 +
 patch_manager_mods/34_npc_diversity_dispatcher.md  | 256 ++++++++
 .../npc_diversity_src/DiversityLogic.h             | 299 +++++++++
 .../npc_diversity_src/DiversityRows.h              |  31 +
 .../npc_diversity_src/NpcDiversityDispatcher.cpp   | 116 ++++
 patch_manager_mods/npc_diversity_src/README.md     |  44 ++
 .../npc_diversity_src/common_contract_test.cpp     |  76 +++
 .../npc_diversity_src/diversity_logic_test.cpp     | 275 ++++++++
 patch_manager_mods/npc_diversity_src/exports.def   |   4 +
 .../npc_diversity_src/kotor2-gog-aspyr.hooks.toml  |  39 ++
 .../kotor2-steam-aspyr.hooks.toml                  |  39 ++
 patch_manager_mods/npc_diversity_src/manifest.toml |  20 +
 .../npc_diversity_src/override/N_CommF01.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF02.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF03.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF04.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF05.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF06.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF07.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF08.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF09.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF10.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF11.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF12.tpc       | Bin 0 -> 349680 bytes
 .../npc_diversity_src/override/N_CommF13.tpc       | Bin 0 -> 349680 bytes
 .../npc_diversity_src/override/N_CommF14.tpc       | Bin 0 -> 349680 bytes
 .../npc_diversity_src/override/N_CommF15.tpc       | Bin 0 -> 349680 bytes
 .../npc_diversity_src/override/N_CommF16.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF17.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF18.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF19.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF20.tpc       | Bin 0 -> 5592591 bytes
 .../npc_diversity_src/override/N_CommF21.tpc       | Bin 0 -> 5592591 bytes
 .../npc_diversity_src/override/N_CommF22.tpc       | Bin 0 -> 5592591 bytes
 .../npc_diversity_src/override/N_CommF23.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF24.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF25.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF26.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF27.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF28.tpc       | Bin 0 -> 5592591 bytes
 .../npc_diversity_src/override/N_CommF29.tpc       | Bin 0 -> 5592591 bytes
 .../npc_diversity_src/override/N_CommF30.tpc       | Bin 0 -> 5592591 bytes
 .../npc_diversity_src/override/N_CommF31.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF32.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF33.tpc       | Bin 0 -> 5592581 bytes
 .../npc_diversity_src/override/N_CommF34.tpc       | Bin 0 -> 5592581 bytes
 .../npc_diversity_src/override/N_CommF35.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommF_Fat.mdl     | Bin 0 -> 202803 bytes
 .../npc_diversity_src/override/N_CommF_Fat.mdx     | Bin 0 -> 144672 bytes
 .../npc_diversity_src/override/N_CommM01.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM02.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM03.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM04.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM05.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM06.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM07.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM08.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM09.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM10.tga       | Bin 0 -> 1048620 bytes
 .../npc_diversity_src/override/N_CommM11.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM12.tpc       | Bin 0 -> 349680 bytes
 .../npc_diversity_src/override/N_CommM13.tpc       | Bin 0 -> 349680 bytes
 .../npc_diversity_src/override/N_CommM14.tpc       | Bin 0 -> 349680 bytes
 .../npc_diversity_src/override/N_CommM15.tpc       | Bin 0 -> 349680 bytes
 .../npc_diversity_src/override/N_CommM16.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM17.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM18.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM19.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM20.tpc       | Bin 0 -> 5592591 bytes
 .../npc_diversity_src/override/N_CommM21.tpc       | Bin 0 -> 5592591 bytes
 .../npc_diversity_src/override/N_CommM22.tpc       | Bin 0 -> 5592591 bytes
 .../npc_diversity_src/override/N_CommM23.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM24.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM25.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM26.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM27.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM28.tpc       | Bin 0 -> 5592581 bytes
 .../npc_diversity_src/override/N_CommM29.tpc       | Bin 0 -> 5592581 bytes
 .../npc_diversity_src/override/N_CommM30.tpc       | Bin 0 -> 5592581 bytes
 .../npc_diversity_src/override/N_CommM31.tpc       | Bin 0 -> 1398284 bytes
 .../npc_diversity_src/override/N_CommM32.tpc       | Bin 0 -> 1398284 bytes
 .../npc_diversity_src/override/N_CommM33.tpc       | Bin 0 -> 1398284 bytes
 .../npc_diversity_src/override/N_CommM34.tpc       | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CommM35.tpc       | Bin 0 -> 5592581 bytes
 .../npc_diversity_src/override/N_CommM_Fat.mdl     | Bin 0 -> 165037 bytes
 .../npc_diversity_src/override/N_CommM_Fat.mdx     | Bin 0 -> 139792 bytes
 .../npc_diversity_src/override/N_CzerkaOff.mdl     | Bin 0 -> 167663 bytes
 .../npc_diversity_src/override/N_CzerkaOff.mdx     | Bin 0 -> 151504 bytes
 .../npc_diversity_src/override/N_CzerkaOff01.tpc   | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_CzerkaOffC01.tpc  | Bin 0 -> 1398256 bytes
 .../npc_diversity_src/override/N_FatComF.mdl       | Bin 0 -> 202803 bytes
 .../npc_diversity_src/override/N_FatComF.mdx       | Bin 0 -> 144672 bytes
 .../npc_diversity_src/override/N_FatComM.mdl       | Bin 0 -> 165037 bytes
 .../npc_diversity_src/override/N_FatComM.mdx       | Bin 0 -> 139792 bytes
 .../npc_diversity_src/override/N_TwiDance.mdl      | Bin 0 -> 216556 bytes
 .../npc_diversity_src/override/N_TwiDance.mdx      | Bin 0 -> 142016 bytes
 .../npc_diversity_src/override/N_TwiDance_Boot.mdl | Bin 0 -> 203472 bytes
 .../npc_diversity_src/override/N_TwiDance_Boot.mdx | Bin 0 -> 150208 bytes
 .../npc_diversity_src/override/N_TwiUnd.mdl        | Bin 0 -> 219478 bytes
 .../npc_diversity_src/override/N_TwiUnd.mdx        | Bin 0 -> 154712 bytes
 .../npc_diversity_src/override/appearance.2da      | Bin 0 -> 181341 bytes
 .../override/diversity_2da_mapping.json            |  27 +
 .../npc_diversity_src/override/heads.2da           | Bin 0 -> 9207 bytes
 .../npc_diversity_src/override/llk_ahrnell.mdl     | Bin 0 -> 103774 bytes
 .../npc_diversity_src/override/llk_ahrnell.tpc     | Bin 0 -> 2796410 bytes
 .../npc_diversity_src/override/llk_harra.mdl       | Bin 0 -> 103774 bytes
 .../npc_diversity_src/override/llk_harra.tpc       | Bin 0 -> 2796410 bytes
 .../npc_diversity_src/override/llk_kehvehn.mdl     | Bin 0 -> 103774 bytes
 .../npc_diversity_src/override/llk_kehvehn.tpc     | Bin 0 -> 2796410 bytes
 .../npc_diversity_src/override/llk_luposhar.mdl    | Bin 0 -> 103774 bytes
 .../npc_diversity_src/override/llk_luposhar.tpc    | Bin 0 -> 2796410 bytes
 .../npc_diversity_src/override/n_brnmodulh.mdx     | Bin 0 -> 55264 bytes
 .../npc_diversity_src/override/n_daraalah.mdx      | Bin 0 -> 59104 bytes
 .../npc_diversity_src/override/pfhc01.mdl          | Bin 0 -> 93972 bytes
 .../npc_diversity_src/override/pfhc01.mdx          | Bin 0 -> 66720 bytes
 .../npc_diversity_src/override/pfhc03.mdx          | Bin 0 -> 66208 bytes
 .../npc_diversity_src/override/pmhc04.mdx          | Bin 0 -> 55840 bytes
 .../npc_diversity_src/override/pmhh01.mdx          | Bin 0 -> 50336 bytes
 patch_manager_mods/npc_diversity_src/test.toml     |   4 +
 scripts/generate_diversity_2da.py                  | 692 +++++++++++++++++++++
 scripts/normalize_diversity_textures.py            | 217 +++++++
 121 files changed, 2173 insertions(+)
```
- **Dirty Files (215):**
  - `M .claude/handoffs/18_mouse_camera_investigation.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M AGENTS.md`
  - ` M CLAUDE.md`
  - ` M patch_manager_mods/10_engine_optimization_research.md`
  - ` M patch_manager_mods/13_ranged_ai_spacing.md`
  - ` M patch_manager_mods/18_mouse_camera_investigation.md`
  - ` M patch_manager_mods/24_master_plan_status.md`

## 2. Immediate Objective (CS10)
**Shared HUD placement: companion queue (doc 30 queue-hud) and selfheal share one right-to-left strip next to each companion card; selfheal claims the right-most slot. Contract via hud-host (doc 31 s5, `HudHost.h`; needs a slot/ReserveIds API) — coordinate with the queue-hud / hud-host owner**

Status: INSTALLED
Task: Shared HUD placement: companion queue (doc 30 queue-hud) and selfheal share one right-to-left strip next to each companion card; selfheal claims the right-most slot. Contract via hud-host (doc 31 s5, `HudHost.h`; needs a slot/ReserveIds API) — coordinate with the queue-hud / hud-host owner

## 2b. Upcoming Batches in Pipeline
- **CS11** [Common Extraction] : Queue UX kit: dark-green hotbar-style panel (border (0,44,26)/(10,96,80), fill (0,0,0,175), radius like hotbar tiles) + Cancel button modelled on vanilla "Cancel Combat"; slim per-companion variant; selfheal button styled to fit the kit
- **CS12** [Common Extraction] : Companion queue as its own mod on the shared HUD dll, selfheal adopts the shared kit (no private copies)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume CS10` (or `resume 24_master_plan_status`).

## 4. Subagent Delegation Direction
- **Delegate to Subagent**: `python3 scripts/plan_manager.py delegate CS10`
- **Supported Commands**: `oc-subagent <scout|code|ghidra|verify|gate|doc>` or `oc-flow auto "<task>"`
