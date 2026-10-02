# Pull Request: Equipment Icon Fix

### PR Title
`fix(perf): eliminate per-frame weapon icon texture reloading`

### Commit Message
```text
fix(perf): eliminate per-frame weapon icon texture reloading

Defers equipment slot placeholder resets, preventing redundant texture
destructions and re-reads (~2.6 ms/frame savings on loose assets).

- Patches/EquipIconFix/manifest.toml: patch manifest (v1.0.0)
- Patches/EquipIconFix/kotor2-steam-aspyr.hooks.toml: hook definitions
- Patches/EquipIconFix/EquipIconFix.cpp: pass deferral and batch commit
- Patches/EquipIconFix/README.md: performance profiling data
```

### PR Description Body
```markdown
### Summary of Changes
Fixes an engine inefficiency where the equipment screen unconditionally reloads and re-uploads equipped weapon icon textures every frame.

### Motivation & Background
During performance profiling with `PerfLab`, the equipment menu exhibited frame time spikes of 2.5–3.0 ms per frame. Disassembly revealed that `CSWGuiInGameEquipment::Update` (`0x008AD930`) first resets weapon slot icon controls to placeholder textures and then immediately replaces them with the equipped item's icon texture within the same frame pass. This forces continuous texture resource freeing and disk re-reads.

### Technical Implementation
- **Hook Sites:** Intercepts the 8 CALL sites in `0x008AD930` that invoke texture bindings (`CALL 0x00414840`).
- **Deferred Commit:** Placeholder resets are deferred to the end of the pass. If the same slot receives an equipped item texture during that pass, the placeholder reset is dropped. Slots end every frame with the exact texture state intended by vanilla, saving ~2.6 ms per frame when using high-resolution loose override textures.

### Testing & Verification
- [x] Verified equipment slot rendering for weapons, armor, belts, and implants.
- [x] Measured frame times with PerfLab before (16.2 ms) vs after (13.6 ms).
- [x] Passes `python3 tools/validate-patches.py`.
```
