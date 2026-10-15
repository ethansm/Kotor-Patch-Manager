# HN-T04

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HN-6h-04 | UNSURE | patch_manager_mods/holster_src/Holster.cpp:161-163; patch_manager_mods/_shared_src/SafeRead.h:115-119 | no | Depends on rdU32 failure value (outside ranges). Sentinel alone still gives edge=1 when new=0. |

- Holster.cpp:161: oldState = rdU32(...) when off is nonzero, 0xFFFFFFFF only when off is 0. No sentinel branch exists for read failure.
- Holster.cpp:162-163: edge = (int(old)==0) != (new==0). A failure value of 0 gives edge=1 whenever new is nonzero, which matches the claim if rdU32 returns 0.
- Holster.cpp:163: sentinel 0xFFFFFFFF casts to -1, so edge=1 whenever new==0. The existing off==0 path already shows this.
- Holster.cpp:156-158: g_onComb gate, KP_ENABLED and takeLine() run before the read. The COMB line is emitted whether or not the read succeeded.
- Holster.cpp:159-163: oldState feeds only the KP_LOG call. No branch in the handler reads it, so the defect is log-only.
- SafeRead.h:115-119: readAt returns false on failure and never assigns *out. The failure value is whatever rdU32 initialised.
- SafeRead.h:116: base==0 returns false before any probe (null creature path).
- SafeRead.h:90-93 and 101: a failed probe is written to the failure ring only when g_ringOn. HolsterCombat does not read the ring or flag the failure in its COMB line.
- rdU32 definition: not in either listed range. Whether it zero-initialises on failure is unknown (see OPEN in reply).
- Gemini's implied fix (sentinel instead of 0) is incomplete: the edge column also needs a read-success check.
