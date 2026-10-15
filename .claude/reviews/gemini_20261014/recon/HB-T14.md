# HB-T14
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HB-1c-06 | CONFIRMED | patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:650,653-656,703 | yes | Removing dead tOk/aOk checks is safe; keep forced red at 653 (deliberate per 652). |

- patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:650 - getBorderTint result written to tOk[g] behind a >0.01 channel gate; the only tOk write before the override.
- patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:651 - getAlpha result gated to [0.05,1.0] into aOk[g]; a valid read is kept in alp[g].
- patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:653 - tint[g] reset to fixed red, discarding the vanilla value read at 650; comment at 652 says intentional.
- patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:654 - tOk[g]=true unconditionally; no later tOk write in the read ranges.
- patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:655 - if aOk is false, alp reset to 0.95 and aOk set true; the live alpha default path.
- patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:656 - the !tOk[g] log branch and its g_contState reset are unreachable.
- patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:657,660,661 - aOk[g] guards are redundant (always true); -1.f ternary at 661 is dead.
- patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:703 - aOk[g] ? alp[g] : 0.85f always takes alp[g]; the 0.85f fallback is unreachable.
- patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:700-702 - bTint is built from forced tint[g]; the vanilla tint never reaches the draw call.
- patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:644 - tOk/aOk/tint/alp declared; in the read ranges tOk/aOk are written only at 650-655.
