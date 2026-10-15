# CO-T06

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| CO-2d-03 | PARTIAL | patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:707; :887 | no | Early slot0 return is real (707). Creature-swap consequence unproven: needs party::members slot-0 behaviour. |

- CompanionSelfHeal.cpp:707 `if (mi == 0) return "slot0";` runs before any tag check. Anchor matches exactly; Gemini's line 707 is correct.
- CompanionSelfHeal.cpp:708 for mi>0 with non-empty g_card[mi].tag, keys by creature tag. Only member 0 is position-keyed.
- CompanionSelfHeal.cpp:716 for mi>0 with empty tag, falls back to "slot"+k.
- CompanionSelfHeal.cpp:887 refreshCache looks up leader config with keyFor(0), so saved leader settings live under the "slot0" key.
- CompanionSelfHeal.cpp:886 leader tag is stored in g_card0.tag; keyFor(0) never reads it.
- CompanionSelfHeal.cpp:879 vs :857 leader state sits in g_card0; companions sit in g_card[i]. keyFor reads g_card[mi].tag, so removing the early return alone would read g_card[0]. Aliasing of g_card[0] and g_card0 is outside the ranges (unverified).
- CompanionSelfHeal.cpp:285-292 empty creature tag is read as "player", not empty. Comment at :286 says the empty tag occurs when leader control is switched to a follower.
- CompanionSelfHeal.cpp:882 LEADER gone/back transitions are logged when v[0] is not ok, so the leader slot can go absent.
- CompanionSelfHeal.cpp:883 when the leader is absent, c0.cfg resets to CompCfg(); the config is re-looked-up when the leader returns.
- Fix soundness: keying member 0 by tag would change the INI key from slot0 to the creature tag. Saved leader settings under slot0 would be orphaned unless a migration exists (not in range). Owner decision needed.
- Not in ranges: party::members (what creature occupies slot 0 after a leader swap) and other keyFor callers. The swap question depends on these.
