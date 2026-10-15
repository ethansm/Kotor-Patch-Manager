# Triage: companion (bundles 2c, 2d), prefix CO

15 findings (2c: 6, 2d: 9), 13 topics, 0 dropped.

- CO-T01 P1: wr<T> in CompanionAI.cpp and guikit::detail::wr guard stores with probeRead, not probeWrite (CO-2c-02, CO-2d-07)
- CO-T02 P1: FM3 probe may read freed sentinel at +0x78 when Fm3Remove's level==g3.level check fails (CO-2c-04)
- CO-T03 P2: CrTrack caches raw creature pointer; stale spinning state / dangling cr (CO-2c-03)
- CO-T04 P2: partyTick overwrites g_lastCtrlArea with 0 during transitions, why=area missed (CO-2c-05)
- CO-T05 P2: AiTraceAurPostString frame offsets unverified (Gemini confidence unsure) (CO-2c-06)
- CO-T06 P2: keyFor(0) returns "slot0", settings follow slot not creature after leader swap (CO-2d-03)
- CO-T07 P2: pending/auto/cooldown state keyed by party slot, wrong after leader swap (CO-2d-04)
- CO-T08 P2: refreshCache runs before per-frame `seen` is computed (CO-2d-05)
- CO-T09 P2: LMB on FullHp/Pending companion button does not close open popover, no noneed log (CO-2d-06)
- CO-T10 P2: native tooltip under overlapping popover; g_shown[8] capacity (CO-2d-02, CO-2d-01)
- CO-T11 P3: SafeRead readAt base+off 32-bit wraparound (CO-2d-08)
- CO-T12 P3: embtex::heal logs "giving up" on the 20th refill even if it succeeded (CO-2d-09)
- CO-T13 P3: `#include "../_shared/SafeRead.h"` stale path vs `_shared_src` (CO-2c-01)

## Duplicates (within batch) and how folded
- CO-2c-02 and CO-2d-07 are the same root cause (read probe before write) in two files; folded into CO-T01.
- CO-2d-01 (g_shown overflow) rambles and retracts itself (7 rects max vs capacity 8); folded with CO-2d-02 into CO-T10 since both read uiFrame widget/rect code. CO-2d-02 text is also muddled (tooltip claim only).

## Findings citing files outside the bundle manifest
- None. Both CO-2d-08 (SafeRead.h) and CO-2d-09 (EmbedTex.h) and CO-2d-07 (GuiKit.h) are in the 2d manifest as deps. CO-2c-01 notes `../_shared/SafeRead.h` is not in the repo (`patch_manager_mods/_shared` does not exist; verified with ls).
- Note: companion_ai_src/ contains PULL_REQUEST.md and common_contract_test.cpp (omitted by manifest); not cited.

## Cross-batch candidates (shared / library code)
- patch_manager_mods/_shared_src/GuiKit.h `wr` (CO-2d-07): other GuiKit-using mods' batches may flag it.
- patch_manager_mods/_shared_src/SafeRead.h `readAt` wraparound (CO-2d-08): any batch using saferead.
- patch_manager_mods/_shared_src/EmbedTex.h `heal` (CO-2d-09): texture-embedding mods (hotbar, HUD batches).
- Probe-vs-write pattern (CO-2c-02) likely repeated as local `wr<T>` helpers in other mods.

## Backlog (not assessed / omitted)
- companion_ai_src/common_contract_test.cpp (manifest-omit, both bundles' companions).
- _shared_src/HudKitTextures.inc and companion_selfheal_src/SelfHealTextures.inc (over max_file_tokens).
- RangedAI.cpp and kite_routine.nss: discussed in 13_ranged_ai_spacing.md but not in the 2c bundle.
- kpatch_api.h and KpLog.h: external includes not in bundle.
- Gemini's 2c focus question on the SafeRead.h include is answered by CO-2c-01 (claims doc 13 notes the include was meant to be "SafeRead.h").
