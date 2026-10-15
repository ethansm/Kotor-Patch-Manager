# SH-T10

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3a-11 | PARTIAL | patch_manager_mods/_shared_src/Party.h:99-100,113-114 | no | Slots hardcoded and documented; K2AddrTable vital entries not in range |
| SH-3c-08 | PARTIAL | patch_manager_mods/_shared_src/party_test.cpp:198,95-100 | yes | Block swaps in fakeVital (int, no short); real callVitalSlot skipped there; "everywhere" unverified |

- Party.h:99-100: SlotMaxHp = 0x98 (arg 1) and SlotCurHp = 0x9C (arg 0) are hardcoded vtable byte offsets.
- Party.h:98: comment documents the one-stack-arg thiscall short contract (2-arg call crashed, lesson 355); constants are deliberate.
- Party.h:90: K2AddrTable::MemberServerCreature is resolved via resolveFn; K2AddrTable is used for function addresses, not vital slots, in range.
- Party.h:102-105: callVitalSlot casts fn to short(__thiscall*) and returns the short; g_callVital defaults to it, so tests must override.
- Party.h:111-114: vitals() checks g_callVital, vt, vt+SlotMaxHp and vt+SlotCurHp non-null before any call.
- Party.h:115-116: max read with arg 1, cur with arg 0, matching the comments at 99-100.
- party_test.cpp:95-100: fakeVital returns int, not short; ignores fn (only records it); returns -999 for unknown creature.
- party_test.cpp:104: mkCreature writes literal 0x98/0x9C into the fake vtable, duplicating header constants by hand.
- party_test.cpp:197-199: block saves g_callVital, swaps in fakeVital, sets fake slot fns 0x401000/0x402000; restore is outside range.
- Anchors: Party.h:113 saferead::readAt line is exact; party_test.cpp:95 fakeVital line is exact; table and partyCount are not in range.
