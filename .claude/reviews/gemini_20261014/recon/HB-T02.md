# HB-T02
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HB-1a-03 | PARTIAL | patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:373-375; patch_manager_mods/action_hotbar_src/HotbarLogic.h:97-100 | yes | Raw copy, no ]/newline filter (373-375) confirmed; Serialize corruption unverified; readExo filter misses ']'. |
- patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:374 is the anchor; exact text match. memcpy copies len raw bytes with no character check.
- ActionHotbar.cpp:366-377 readCreatureTag: rejects only null cr, unreadable fields, len==0, len>=cap (64), or failed probeRead (369-373).
- ActionHotbar.cpp:373-375: len<=63 enforced by cap, so out[len]=0 stays inside tag[64]; no overflow.
- ActionHotbar.cpp:400-402: haveTag ? tag : nullptr passed to hb::characterKeyFor; the raw tag becomes the key.
- HotbarLogic.h:97-100: tag branch returns lower(trim(tag)) with no character check; only "player"/"exile" are special-cased.
- HotbarLogic.h:103-115: without a tag the key falls back to objfffffffd / slotN / objXXXXXXXX, so a rejected tag degrades safely.
- _shared_src/SafeRead.h:138-142: readExo printable filter rejects bytes outside 0x20-0x7E (newline included); ']' (0x5D) passes it.
- ActionHotbar.cpp:366-377 does not call readExo, so the shared printable filter is not applied to creature tags.
- trim() and lower() are defined outside the ranges; whether trim strips edge newlines is unverified.
- The Serialize / Config::Save [Profile.<key>] writer is outside the ranges; corruption on Serialize is UNSURE.
