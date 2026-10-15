# HN-T02
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HN-6h-02 | PARTIAL | patch_manager_mods/holster_src/kotor2-steam-aspyr.hooks.toml:46-49; patch_manager_mods/holster_src/manifest.toml:8-9 | no | Ordering and span adjacency true; ECX guarantee comment-only; clobber needs cmdq wrapper (outside range). |
- patch_manager_mods/holster_src/kotor2-steam-aspyr.hooks.toml:46-47 | comment says cmdq-probe CmdqProbeReequip owns 0x005DF920-0x005DF925. Anchor text is verbatim here; NOT-FOUND only in Holster.cpp.
- patch_manager_mods/holster_src/kotor2-steam-aspyr.hooks.toml:49 | H3 hook address 0x005DF926, type detour, skip_original_bytes=false (handler runs before stolen bytes).
- patch_manager_mods/holster_src/kotor2-steam-aspyr.hooks.toml:52 | original_bytes = 6 bytes, MOV [EBP-0x6C],ECX then MOV EAX,[EBP-0x6C]; span 0x005DF926-0x005DF92B, no relative branch.
- patch_manager_mods/holster_src/kotor2-steam-aspyr.hooks.toml:57-58 | parameter source=ecx is read at the hook site; nothing checks that ECX still holds this.
- patch_manager_mods/holster_src/manifest.toml:8-9 | requires=[] and conflicts=[]; no cmdq-probe dependency and no ECX or overlap guarantee declared.
- patch_manager_mods/holster_src/Holster.cpp:152 | handlers are read-only and run before the stolen bytes; matches the toml model.
- patch_manager_mods/holster_src/Holster.cpp:182 | comment labels the swap executor at 0x005DF920; toml hooks 0x005DF926 (label mismatch, 6 bytes off).
- patch_manager_mods/holster_src/Holster.cpp:184-191 | gates run before reads; reads 8 dwords at exec+0..+28 via rdU32 under saferead. Bad this gives junk log only if saferead guards it (definition outside range).
- Inference, not verified in range: cmdq entry 0x005DF920-0x005DF925 is likely PUSH EBP / MOV EBP,ESP / SUB ESP,0x6C (matches the [EBP-0x6C] slot); if so it does not write ECX.
