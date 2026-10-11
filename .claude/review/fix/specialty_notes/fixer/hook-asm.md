## Specialty: hook-asm (fixer)
- Detour handlers are cdecl `extern "C"` functions named in `*.hooks.toml` (`function = "..."`) AND listed in the mod's `exports.def`. Export name, hooks.toml binding and definition must change together.
- Never invent addresses, original_bytes, exit addresses or struct offsets. If the fix needs one that is not quoted in the finding, report not-fixable.
- Pointer reads off a register (ebp/esp + offset) need a null/range guard in the style already used in the file.
- Always set needs_live_check: true.
