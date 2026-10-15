# G2.3 skeptic input: companion (8 fids)

## CO-T02 (P1): Can CompanionAiProbeFm3LevelEnd dereference g3.sentinel at +0x78 after the object was removed, because Fm3Remove sets g3.removed only when level == g3.level (read at removal time)?
- **CO-2c-04** v2=CONFIRMED (G1 PARTIAL) | claimed high memory-safety | FM3 sentinel isSentinel requires level match; may leave removed false and read freed sentinel | file patch_manager_mods/companion_ai_src/CompanionAI.cpp fn CompanionAiProbeFm3LevelEnd / CompanionAiProbeFm3Remove
  - G1 evidence: patch_manager_mods/companion_ai_src/CompanionAI.cpp:1231-1232, :1219, :1201 | G1 note: Gap real; 1201 read is guarded, stale-not-crash; flag-side fix safe but moot for crash
  - v2 evidence: patch_manager_mods/companion_ai_src/CompanionAiLogic.h:637 | v2 notes: [det CONFIRMED] d001: fm3Lost is a pure bool over removed and level; no deref, stale read only shifts a log line || [gem2 PARTIAL] u001: Crash refuted by probeRead guard; stale read consistent with in-range code; isSentinel gap outside range.

## CO-T03 (P2): Can e1Evaluate/E1 END state go stale (t.spinning stays true, or t.cr dangling) when a tracked creature dies or the area changes with no further AiTraceAttackEntry for its ObId, and is t.cr dereferenced in e1Evaluate?
- **CO-2c-03** v2=PARTIAL (G1 PARTIAL) | claimed medium logic | CrTrack caches raw creature pointer; stale spinning state and dangling cr after death or area change | file patch_manager_mods/companion_ai_src/CompanionAI.cpp fn e1Evaluate / AiTraceAttackEntry
  - G1 evidence: patch_manager_mods/companion_ai_src/CompanionAI.cpp:690 anchor exact; :707-712 END; :787-790 cr set then e1Evaluate | G1 note: Spinning goes stale on death/area change; dangling cr not dereferenced in range
  - v2 evidence: patch_manager_mods/companion_ai_src/CompanionAI.cpp:341,346,384-389 | v2 notes: [gem2 PARTIAL] u002: Stale spinning real in range; dangling cr not dereferenced in range.

## CO-T04 (P2): In partyTick, does g_lastCtrlArea = area store 0 when FnGetArea returns 0 on a tick with have==true, causing the next non-zero area to skip why=area?
- **CO-2c-05** v2=CONFIRMED (G1 CONFIRMED) | claimed medium logic | g_lastCtrlArea overwritten with 0 during transition so PARTY why=area misses | file patch_manager_mods/companion_ai_src/CompanionAI.cpp fn partyTick
  - G1 evidence: patch_manager_mods/companion_ai_src/CompanionAI.cpp:589 | G1 note: Line 589 stores area unconditionally when have is true; area 0 erases baseline. Anchor exact at 587.
  - v2 evidence: patch_manager_mods/companion_ai_src/CompanionAI.cpp:589 | v2 notes: 

## CO-T06 (P2): Does keyFor(0) always return slot0 (ignoring g_card0.tag) so that per-creature INI settings change owner when the party leader changes?
- **CO-2d-03** v2=CONFIRMED (G1 PARTIAL) | claimed high logic | keyFor(0) returns slot0 ignoring tag, so settings follow party position not creature after leader swap | file patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp fn keyFor
  - G1 evidence: patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:707; :887 | G1 note: Early slot0 return is real (707). Creature-swap consequence unproven: needs party::members slot-0 behaviour.
  - v2 evidence: patch_manager_mods/_shared_src/Party.h:76; Party.h:80-84 | v2 notes: [gem2 CONFIRMED] u003: Creature-keyed direction sound; serverCreature() Party.h:89 is per-creature handle; lifetime unverified.

## CO-T07 (P2): Are g_pend0/g_pend[1..2], g_auto*, and g_lastUseMs keyed by party slot index only (no creature server pointer reconcile), so a leader swap during the 6 s pending window misattributes DONE/timeout/kick?
- **CO-2d-04** v2=PARTIAL (G1 PARTIAL) | claimed high logic | Pending, auto and cooldown state indexed by party slot not creature; leader swap misattributes | file patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp fn useNow / pendingTick / kickTick
  - G1 evidence: patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:394,757,762,790 | G1 note: Slot-keyed state confirmed; leader-swap effect needs party::members slot reassignment (outside range)
  - v2 evidence: patch_manager_mods/_shared_src/Party.h:80-84; Party.h:76; Party.h:51-62 | v2 notes: [gem2 PARTIAL] u004: In range: positional lookup, no creature-id key. Out of range: reslotting on swap, slot-keyed state.

## CO-T08 (P2): In uiFrame, is refreshCache (which passes g_card[i].seen / g_card0.seen to shouldAutoUse) called before the slot-mapping loop that sets seen for the current frame?
- **CO-2d-05** v2=CONFLICT (G1 PARTIAL) | claimed medium logic | refreshCache runs before g_card[i].seen is set for the frame, delaying or misgating auto-heal | file patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp fn refreshCache / uiFrame
  - G1 evidence: patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:998 (refreshCache call) precedes :1001-1027 (seen reset and set); anchor exact at :872 | G1 note: Ordering is true; lag is one frame, not a cache-period delay. Simple reorder breaks c.have/c.m dependency.
  - v2 evidence: patch_manager_mods/_shared_src/HealItems.h:67 ; patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1001 | v2 notes: [det REFUTED] d002: shouldAutoUse has no seen parameter or gate; early return gates on gameplay only, per extra rg || [det CONFIRMED] d003: seen reset per frame at 1001-1002 and in hideAll (725, called at 997): both present, one-frame lag

## CO-T09 (P2): When the popover is open and LMB hits a companion button whose state is FullHp or Pending (clickAllowed false), does the onBtn>=0 branch leave g_popMi open and log nothing?
- **CO-2d-06** v2=CONFIRMED (G1 CONFIRMED) | claimed medium logic | LMB on companion button at FullHp/Pending skips popover close and noneed log | file patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp fn uiFrame (LMB branch)
  - G1 evidence: patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1166-1180 | G1 note: Logging a noneed-style line is sound; closing the popover on a no-op click is a UX choice.
  - v2 evidence: patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1166-1180 | v2 notes: 

## CO-T10 (P2): When the RMB popover overlaps a card button, does the button keep its native tooltip/click-through state so a tooltip can show under the popover; and can g_nShown ever exceed 8 in uiFrame?
- **CO-2d-02** v2=PARTIAL (G1 PARTIAL) | claimed medium logic | Leader button native tooltip may show under overlapping RMB popover when clamped at screen left | file patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp fn uiFrame / popLayoutFor / buildWidgets
  - G1 evidence: patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1091,1103-1105,1231 | G1 note: Leader tooltip, show and hover not gated by popover overlap; clamp and z-order unverified.
  - v2 evidence: patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1116-1146; :1149-1150; :703 | v2 notes: [gem2 PARTIAL] u005: Hover gate already present (1143); show/tooltip gating and left clamp unverified, outside range.
