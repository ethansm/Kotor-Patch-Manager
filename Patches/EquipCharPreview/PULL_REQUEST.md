# Pull Request: Equipment 3D Character

### PR Title
`feat(equipment): live 3D character preview with equipped gear (#104, #78)`

### Commit Message
```text
feat(equipment): live 3D character preview on equipment screen

Shows a real-time 3D model of the current party member on the equipment
screen, mirroring equipped armor, robes, headgear, and held weapons.

- Patches/EquipCharPreview/manifest.toml: patch manifest (v1.0.0)
- Patches/EquipCharPreview/kotor2-steam-aspyr.hooks.toml: hook definitions
- Patches/EquipCharPreview/EquipCharPreview.cpp: character scene view,
  node visibility toggles, weapon attachment hookup
- Patches/EquipCharPreview/README.md: usage and technical documentation
```

### PR Description Body
```markdown
### Summary of Changes
Adds a real-time, interactive 3D character model to the equipment screen that dynamically updates when gear is equipped or removed.

### Motivation & Background
The vanilla equipment menu only displays 2D slot icons, forcing players to close the menu to see how armor, helmets, or weapon modifications look on their character. This patch provides immediate visual feedback and demonstrates node attachment manipulation requested in **#78 (Toggle visibility of MDL nodes based on equipped item)**.

### Technical Implementation
- **Character Rendering:** Mounts a 3D scene view displaying the current player or companion creature object.
- **Dynamic Slot Mirroring:** Hooks equipment slot change events (`0x008ABA50`) to update model meshes when armor, masks, or weapons change.
- **Weapon Attachments:** Traverses bone hierarchy to bind weapon models to `RHand` and `LHand` hook nodes with correct dual-wield/single-blade stances.
- **Flourishes & Idles:** Periodic idle animation flourishes trigger when selecting weapon slots.

### Testing & Verification
- [x] Verified companion switching (correctly loads Kreia, Atton, Droids, etc.).
- [x] Tested disguise items and full body armor models without mesh clipping.
- [x] Built and validated under MinGW-w64 and MSVC.
- [x] Passes `python3 tools/validate-patches.py`.

Closes #78
Relates to #104
```
