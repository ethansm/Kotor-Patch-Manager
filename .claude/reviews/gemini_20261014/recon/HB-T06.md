# HB-T06

| fid | verdict | evidence (path:line) | fix_sound | note |
|---|---|---|---|---|
| HB-1b-01 | PARTIAL | patch_manager_mods/07_force_power_hotbar.md:95; patch_manager_mods/action_hotbar_src/HotbarLogic.h:97-117 | yes | Character-key part confirmed; hook list (doc line 94) not verifiable from ranges |

- 07_force_power_hotbar.md:95 documents the key as obj<hexid> plus Exile = objfffffffd only; no tag, slot, or 0xFFFFFFF0..FE rule.
- HotbarLogic.h:97-101 uses a non-empty creature tag first; "player"/"exile" map to objfffffffd, other tags return lowercased trimmed tag.
- HotbarLogic.h:103 Exile test covers 0xFFFFFFF0..0xFFFFFFFE, partyCount==1, isMember0, and partySlot==0.
- HotbarLogic.h:112-115 untagged companion with partySlot>0 returns "slot1"/"slot2"; doc never mentions this.
- HotbarLogic.h:117 obj%08x is only the final fallback, so doc line 95 is outdated as the general rule.
- HotbarLogic.h:105-107 id 0 returns ""; 0xFFFFFFFF maps to objfffffffd only for one-member or slot-0 cases; doc line 95 matches this clause.
- HotbarLogic.h:90 documents "" as load-in-progress; doc line 95 says "unknown (saves refused)"; save-refusal effect is outside ranges.
- 07_force_power_hotbar.md:94 lists 8 hooks with addresses; hook sites are outside the read ranges, so this part is UNSURE.
- Anchor `if (tag && *tag) {` is at HotbarLogic.h:97; NOT-FOUND in the doc is expected because the anchor lives in the code file.
- Fix: rewrite doc line 95 to list tag, slot, and sentinel-range rules as in HotbarLogic.h:97-117; doc-only edit, safe.
