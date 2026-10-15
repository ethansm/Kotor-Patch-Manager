# CO-T07
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| CO-2d-04 | PARTIAL | patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:394,757,762,790 | yes | Slot-keyed state confirmed; leader-swap effect needs party::members slot reassignment (outside range) |

- :394 anchor matches exactly (`else if (mi == 0) { g_pend0 = shui::Pending(); ...`); enclosing function name (useNow) is above range, not verified.
- :393-394 store preHp, preStack, pendKey (item key), since, lastUseMs and auto state; no creature server pointer goes into pending state.
- :391 g_lastUseMs[mi] debounce is keyed by slot index; cooldown read sites are outside range.
- :395 `out->server = m.server` is the only creature pointer kept, and only in the output struct.
- :737 pendingTick calls party::members(v,3) every tick; v[i] is whoever occupies slot i at that moment.
- :757 and :762 hp0 = v[0].cur compared to g_pend0.preHp from the previous occupant; queue read on v[0].server; no identity check.
- :744 and :759 stack evidence matched by item key in ItemGroup, not by creature.
- :790-791 kick loop covers slots 1-2 only; leader slot 0 is never kicked. Kick misattribution needs a slot 1/2 occupant change.
- :799, :804-805 and :812: starved timer resets only when the current occupant is not starved; a still-starved new occupant inherits the old timer and can give up.
- :753 and :768 clear per-slot state on DONE; nothing clears on occupant change. The 6 s window is in shui::pendingDone2 (outside range).
