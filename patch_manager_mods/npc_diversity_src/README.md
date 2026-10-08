# Master NPC Diversity Dispatcher (`npc-diversity` 1.0.0)

Deterministic runtime NPC appearance & clothing texture diversity dispatcher for KOTOR II (Steam Aspyr and GOG Aspyr).

## What it does

Eliminates repetitive NPC clone appearances by dynamically varying commoner clothing textures (slots 01–35), heavy-set body proportions (15% fat body roll), and alien head assignments without altering creature templates (`.utc`) in savegames or conflicting with TSLRCM.

1. **Deterministic Dispatcher**: Computes an FNV-1a hash of the creature's spawn coordinates `(X, Y)`, area ResRef, and creature tag.
2. **Dynamic Texture Slot Rotation**: Diverts `CSWCCreatureAppearance::GetBTypeModelAndTextureResRef` to select textures across `N_CommM01..35` and `N_CommF01..35`.
3. **Body & Head Remapping**: Diverts `CSWCCreature::SetAppearance` to remap commoners to heavy body models (`N_CommM_Fat`, `N_CommF_Fat`) and alien head variants.
4. **Story & Companion Protection**: Comprehensive exclusion list passes companions (Kreia, Atton, etc.) and named story NPCs through unchanged.

## Engine Hooks & Signatures

| Target | Hook Function | Function / Purpose | Address |
|---|---|---|---|
| Steam Aspyr | `OverrideNpcTexture` | `CSWCCreatureAppearance::GetBTypeModelAndTextureResRef` | `0x0085E601` |
| Steam Aspyr | `RemapNpcAppearance` | `CSWCCreature::SetAppearance` | `0x0076D200` |
| GOG Aspyr | `OverrideNpcTexture` | `CSWCCreatureAppearance::GetBTypeModelAndTextureResRef` | `0x0048E691` |
| GOG Aspyr | `RemapNpcAppearance` | `CSWCCreature::SetAppearance` | `0x00419070` |

## Build & Verification

```bash
# Verify MinGW DLL build & PE exports
python3 scripts/build_check.py --mod npc_diversity_src

# Run logic & common contract tests
python3 scripts/run_tests.py diversity

# Check hook bytes against game binary
python3 scripts/run_tests.py suite:check-hook-bytes
```

## Packaging (.kpatch)

KPM package `NpcDiversity.kpatch` is built via `Patches/create-patch.py` under the KPM tree (`$K/Patches/NpcDiversity`):
- `manifest.toml`: Patch metadata, supported versions, build definitions.
- `kotor2-steam-aspyr.hooks.toml`: Steam Aspyr hook definitions.
- `kotor2-gog-aspyr.hooks.toml`: GOG Aspyr hook definitions.
- `binaries/windows_x86.dll`: MinGW-compiled 32-bit PE DLL exporting `OverrideNpcTexture`, `RemapNpcAppearance`, and `NpcDiversity_Init`.

Static asset overrides (textures `N_CommM01..35`/`N_CommF01..35`, models, `appearance.2da`, `heads.2da`) reside in `override/` for game directory installation.
