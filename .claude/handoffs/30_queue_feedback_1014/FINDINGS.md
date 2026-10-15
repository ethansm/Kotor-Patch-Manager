# Queue HUD play feedback 2026-10-14 22:55 (reported in the selfheal-polish-round-3 session; NOT fixed)

Screenshots (this dir, untracked, 3 MB each; originals ~/Documents/Screenshot_2026-10-14_22-55-{11,17,23}.png) are native-res crops of the bottom-right of a 3840x1600 screen: crop offset x=1840, y=985.

## User feedback (verbatim)
1. "the 'noncombat mode' needs the cancel queue button displayed under the custom queue UX"
2. "Switching team members with tab caused an issue, exile ordered to use a medpack on Kreia, after tabbing through, atton also picked up a command to use the medpack and I never gave that order"

## Findings (queue-hud log via `kotor2_logs.py check queue-hud_log.txt`, bounded sed only)
- Bug 1: log says `VIS cancel0 shown=1 why=queue rect=[1700,1519,440,64]` (leader "CANCEL QUEUE" text button, `cwLookLeader`), but screenshots 1-2 (leader strip + medpack disc at abs ~(2005,1389)) show NOTHING at crop (-140..300, 534..598). Companion X buttons (cancel1/2) DO draw in screenshot 3 but not in 1-2. Suspect: `cwApply` (QueueHud.cpp ~450) visibility/placement or draw order in the non-combat/paused state; not yet read. Check whether the widget is clipped (rect x starts at 1700, screen crop) or covered.
- Bug 2: only ONE command of ours exists (`CMD#1 m=0 kind=item id=248`, one HANDOFF at L72, one HANDOFF_ONE). Tab rotation moves the same medpack entry between slots m=0/1/2 (expected: slots are not stable creatures). The duplicate: at the soft-combat edge (right after `BANNER act=hide soft=1`) TWO slots show `ROUND ... types=3f` at once (L115-116, L153-154, L186-187), `ours=external`, no dispatch/HANDOFF from us. Logs carry slot numbers, not creature ids, so it cannot be proven whether the engine copied the command to Atton or the mod misreads a slot swap.
- Next steps: (a) add creature id (CrObjectId) to ROUND/QJ/CMD trace lines, (b) replay: queue medpack on exile, Tab through all 3, enter soft combat, check which creature ids carry 0x3F, (c) read cwApply for bug 1.

## Ownership of the uncommitted queue_hud_src edits (as of 10-14 23:00)
`QueueHud.cpp` + `QueueHudLogic.h` dirty since 10-12 17:18 (FIFO gate: `haveCur` in queueCountOf/decideAction + `shouldDispatchNext` in tickItemQueues). Owner NOT recorded anywhere; best guess session `ql8-soft-combat-banner-plan` [67c96e] (last queue-hud commit 9075166 = QL8). Not live (kpm_status: queue-hud DIRTY). Ask that session before building on or discarding them.
