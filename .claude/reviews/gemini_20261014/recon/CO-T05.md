# CO-T05
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| CO-2c-06 | PARTIAL | patch_manager_mods/companion_ai_src/CompanionAI.cpp:759,772; patch_manager_mods/companion_ai_src/kotor2-steam-aspyr.hooks.toml:41-48 | yes | No in-range verification record; offset correctness needs 0x6F6200 decompile and T2 frame, both outside ranges. |
- CompanionAI.cpp:759 header comment names 0x6F6200 retail stub, hook 0x6F62F2, after four pops; no decompile citation or frame layout note.
- CompanionAI.cpp:760 signature AiTraceAurPostString(int ebp); every frame value is read via fr<>(ebp, disp).
- CompanionAI.cpp:769 copyStr uses ebp-0x20 (pointer) and ebp-0x1C (length) to fill text[400]; these are the only offsets that drive a read.
- CompanionAI.cpp:771-772 x at ebp-0x10, y at ebp-0x14, life float at ebp-0x18 feed only the tline log line; no guard or branch uses them.
- CompanionAI.cpp:772 anchor check: claimed text matches exactly at line 772.
- kotor2-steam-aspyr.hooks.toml:43-47 T3 entry: address 0x006F62F2, detour, function AiTraceAurPostString; original bytes C7 45 CC 00 00 00 00 = MOV dword [EBP-0x34], 0.
- kotor2-steam-aspyr.hooks.toml:50-51 parameter source "ebp", type "int"; matches the int ebp argument at CompanionAI.cpp:760.
- The five reads (-0x20, -0x1C, -0x18, -0x14, -0x10) and the stolen slot (-0x34) are disjoint 4-byte slots; no in-range aliasing.
- kotor2-steam-aspyr.hooks.toml:26-35 T2 entry is a different function (PrintString, stolen MOV [EBP-0x10],0 in its frame); its frame reads are outside ranges, so the T2 comparison cannot be made here.
