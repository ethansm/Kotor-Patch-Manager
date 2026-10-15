# SH-T09

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3a-10 | PARTIAL | patch_manager_mods/_shared_src/EffectIcons.h:31,32,43,47,60 | yes | Bare Effect_* names and unguarded seen[nSeen++] are real; overflow needs count>MaxEffects (:42, outside range). |
| SH-3b-03 | PARTIAL | patch_manager_mods/_shared_src/k2addr_test.cpp:434,439,443,445; patch_manager_mods/_shared_src/Effects.h:116-119 | yes | :445 never reaches :119 guard (only Creature_EffectCount zeroed); cnt=0 true but not the cause. collectBuffs untested: unverified. |

- SH-3a-10 anchor matches EffectIcons.h:47 exactly. That line is inside k2fx::collectBuffs (:40), not iconForEffectIconEffect (:28); only collectBuffs writes seen[].
- EffectIcons.h contains no k2addr token. Effect_Type (:31), Effect_IntCount and Effect_Ints (:32), and Effect_SpellId (:47) are bare identifiers read via rd(). Whether they are constexpr is defined outside range.
- EffectIcons.h:43 declares int seen[MaxEffects] with nSeen starting at 0.
- EffectIcons.h:60: the iconSeen write is guarded by nIcon < MaxEffects; the seen[nSeen++] write has no bound.
- EffectIcons.h:45 loop is bounded by count and maxOut, and nSeen grows at most once per iteration, so nSeen <= count. Overflow requires count > MaxEffects from effectListNonEmpty (:42), which is outside range.
- Effects.h:116-119: scanShieldPools reads five offsets via k2addr::off and returns 0 if any is zero. The guard at :119 runs before effectListNonEmpty at :120.
- k2addr_test.cpp:434 fixture: effect array at +0x148 = 0x4000, effect count at +0x14c = 0.
- k2addr_test.cpp:439: noCount copies kSteam and zeroes only Creature_EffectCount. None of the five :119 offsets is zeroed, so :119 cannot be the branch that returns 0 at :445.
- k2addr_test.cpp:443: readEffectList fails under noCount (arr stays -1), so the :445 zero comes from this path. collectBuffs is not called in :425-450.

OPEN: Does effectListNonEmpty/readEffectList clamp count to MaxEffects, and is collectBuffs tested elsewhere in k2addr_test.cpp?
