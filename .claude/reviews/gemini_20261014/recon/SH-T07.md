# SH-T07
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3a-01 | PARTIAL | patch_manager_mods/_shared_src/GuiKit.h:583-588 | no | Guard at :586 covers iniW/iniH only; 'engine size' label unproven, ev->screen_w is pre-sync raw. |

- GuiKit.h:583: sw is initialised from ev->screen_w (raw event value, before syncScreen).
- GuiKit.h:584: syncScreen(iniW, iniH, &sw, &sh) rewrites sw/sh; its body is outside the read ranges.
- GuiKit.h:586: the only guard is `if (s.iniW > 0 || s.iniH > 0)`; it tests ini values, not ev->screen_w/h.
- GuiKit.h:587: anchor matches exactly; ev->screen_w and ev->screen_h are passed as the 3rd/4th (client-size) args with no >0 check.
- GuiKit.h:588: 5th/6th args are sw/sh (post-sync) cast to float, so the engine-side argument is sw, not ev->screen_w.
- GuiKit.h:43: AddrScreenW/AddrScreenH are documented as the engine framebuffer size, which supports sw being engine-side.
- GuiKit.h:585: ex/ey start from ev->engine_x/engine_y and are overwritten only when the ini guard passes.
- GuiKit.h:20-25: K2WheelEvent and clientToEngineF definitions are not in the read ranges (includes only).
- GuiKit.h:590: s.inside(ex, ey) consumes the converted values; any bad divisor would propagate here (depends on clientToEngineF, outside range).
- GuiKit.h:579-581: s.gate(now) wraps the whole path; the callback returns 0 when the gate fails.
