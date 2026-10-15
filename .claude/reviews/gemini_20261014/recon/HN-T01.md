# HN-T01
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HN-6h-01 | REFUTED | patch_manager_mods/holster_src/HolsterLogic.h:184, :191-195 | n/a | Anchor exact (195). Stow cannot start mid-draw: 1500 ms delay exceeds 350/1000 ms draws. Reset claim outside range. |
| HN-6h-05 | PARTIAL | patch_manager_mods/holster_src/holster_logic_test.cpp:140-170; patch_manager_mods/holster_src/HolsterLogic.h:189 | yes | Test gap true in range. Anchor 157 tests combat re-entry, not onSwap. Out-of-combat swap reachability outside range. |

- patch_manager_mods/holster_src/HolsterLogic.h:184: onCombat(0) on a 1->0 edge sets stowPending and stowAt = now + 1500 ms.
- patch_manager_mods/holster_src/HolsterLogic.h:182: onCombat(1) on a 0->nonzero edge clears stowPending and starts a 350 ms draw.
- patch_manager_mods/holster_src/HolsterLogic.h:189: onSwap sets PhDrawing for 1000 ms and clears stowPending; it never checks lastCombat.
- patch_manager_mods/holster_src/HolsterLogic.h:191-192: tick promotes PhDrawing to PhDrawn before the stow check runs.
- patch_manager_mods/holster_src/HolsterLogic.h:193-195: stow fires at stowAt; with monotonic now a draw always ends first, since draw start <= exit time and stowAt >= exit + 1500.
- patch_manager_mods/holster_src/HolsterLogic.h:189 + :192: after out-of-combat onSwap the phase reaches PhDrawn, nothing re-arms a stow, and it stays drawn until the next combat entry (code-level).
- patch_manager_mods/holster_src/HolsterLogic.h:150-199: Fsm has no reset method; only onCombat, onSwap and tick mutate state (range-local fact).
- patch_manager_mods/holster_src/holster_logic_test.cpp:157: asserts combat re-entry cancels a pending stow; no onSwap call on that path.
- patch_manager_mods/holster_src/holster_logic_test.cpp:162-170: testFsmSwapAndRollover swaps only with lastCombat==1 and no pending stow.
- patch_manager_mods/holster_src/holster_logic_test.cpp:126-182: no combat==2 call and no out-of-combat onSwap test in range.
