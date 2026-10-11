## Specialty lens: hook-asm (reviewer)
- Check the export name, hooks.toml `function` and the definition agree after the patch.
- Any new code in a detour: does it preserve the calling convention and the registers the stub expects? Does it read through a pointer that can be null?
- A patch that changes addresses, original_bytes or offsets without a quoted source is a reject.
