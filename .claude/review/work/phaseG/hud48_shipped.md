# Phase G: HUD panel +0x48 and +4 controls (range 78807d4..HEAD)

Scope: queue_hud_src, action_hotbar_src, companion_selfheal_src, _shared_src (read-only).

## Ranked suspects
1. companion_selfheal_src/CompanionSelfHeal.cpp:479-489 enum NWidgets: 16 -> 20. Added W_DiscLeader, W_FrameLeader, W_BtnLeader, W_RingLeader. Matches the +4 controls exactly (213 -> 217).
2. CompanionSelfHeal.cpp:1052-1075 buildWidgets(): the 4 new add() calls create the leader controls via makeLabel in the HUD ctor (K2HUD_BUILD, layout still open).
3. _shared_src/GuiKit.h:150-175 createLabelFromTemplate(): the +0x48 write (Ctl_Flags) targets the new control `c`, not the panel. No panel +0x48 write found here.
4. queue_hud_src/QueueHud.cpp:457-478 onHudBuild(): builds 3 cancel labels (unchanged from 78807d4). Not a +4 source.
5. action_hotbar_src/ActionHotbar.cpp:106 ControlCount = TipOff+1 = 86 (unchanged). Not a +4 source.

## Checked and cleared
- Repo-wide added lines in range: no write to Panel_Flags (0x48), 0x400/0x600 on panels, or Mgr_Panels.
- queue_hud_src/QueueHud.cpp:513, 536, 665, 1122: hud+HudSlotBase (0x5F2C, stride 0x148) and HudCombatFlags (0xA904) are read-only (getExtent / rd).
- QueueHud.cpp:369-448 (setVisible/setClickable/setFill/setRect): all on own CW/label `w.c`, not the hud.
- 2c071d8 (QH5): colour constants in cwLookLeader only, applied to own control. No panel write.
- CompanionSelfHeal.cpp:925 and 1088: Panel_Flags read only. isIgnored() at :224 reads it.

## Assessment
- +4 controls: confirmed as selfheal leader widgets (commit range, NWidgets 16 -> 20).
- +0x48 = 0xFF8FB44F: no in-range code found that writes it. The value is probably engine-side, changed by the extra control appends (list growth / realloc), or by a mod outside these 4 dirs.

## Open
- Mechanism for +0x48 change is unproven. Needs a live check: dump +0x48 before and after the 4 leader creates (or with selfheal's leader widgets skipped).
- Not in scope: save-events, gui-input (GuiInputProvider.cpp), inventory-3d-viewport, shield-bar, level-up-override. Not audited here.
