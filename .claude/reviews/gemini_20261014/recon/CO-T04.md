# CO-T04
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| CO-2c-05 | CONFIRMED | patch_manager_mods/companion_ai_src/CompanionAI.cpp:589 | yes | Line 589 stores area unconditionally when have is true; area 0 erases baseline. Anchor exact at 587. |

- CompanionAI.cpp:587 is the anchor line, text matches the brief verbatim; it needs both area and g_lastCtrlArea nonzero to emit why="area".
- CompanionAI.cpp:583 sets area = cobj ? FnGetArea(cobj) : 0, so area is 0 when gameObject(ctrl) returns 0 or the engine returns 0.
- CompanionAI.cpp:589 runs inside `if (have)` and writes g_lastCtrlArea = area with no nonzero guard. This is the defect.
- Sequence A, 0, B: the 0 tick writes 0 at 589; the B tick fails the 587 guard (baseline 0), so no area emit; 589 then stores B.
- CompanionAI.cpp:585 load branch: first load tick sets why="load" and skips 587, but the 589 store still runs.
- CompanionAI.cpp:588 pf branch is independent of the area baseline; pf emits are unaffected by the gap.
- Gemini's implied fix (skip the store when area is 0, e.g. guard 589 with area) keeps A, so B emits why=area. Fix is sound for the in-range logic.
- Fix caveat: a reload with area 0 would keep a stale nonzero baseline from the prior session, giving at most one spurious area trigger. Reset points are outside range.
- Trigger frequency is unverified: whether FnGetArea returns 0 during real area transitions is engine knowledge, not in range.
- Out of range, not read: declarations of g_lastCtrlArea and g_partyLoaded and their reset sites.
