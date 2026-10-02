# Level-up wizard questions -> preset mapping (v1, 2026-10-02)

The in-game wizard (doc 08 R8, user decision (e)) asks a few questions per companion and selects the preset whose `[meta.wizard]` answers match
(nearest match if no exact one). Later versions may compute per-answer values dynamically; v1 = curated presets only.

## Questions (a character offers only the answers that make sense for it)
| key | question | answers |
|---|---|---|
| `role` | Combat role | `melee`, `ranged`, `caster` (offensive Force), `support` (heal/buff/control), `tank` |
| `style` | Weapon style | `single` (one weapon), `dual` (two weapons), `double` (double-bladed), `unarmed`, `pistols`, `rifle` ; `none` for pure casters |
| `alignment` | Force alignment of powers | `light`, `dark`, `neutral` (universal powers only) ; `none` for no-Force builds |
| `convert` | When to become a Jedi (convertible companions only) | `early` (as soon as the story allows; mod holds levels until the Jedi class appears), `late` (keep base class for its feats first), `never` ; `n/a` for Kreia/Visas |
| `skills` | Skill focus | `combat` (Awareness/Treat Injury), `tech` (Repair/Computer Use/Security/Demolitions), `stealth`, `social` (Persuade) |

## Preset file rules
- File `paths/presets/<tag>__<role>_<style>_<alignment>_<convert>.toml` (use the answer words; skip `convert` part for n/a characters). Existing variant
  names may stay if `[meta.wizard]` is added.
- `[meta]` keys: `tag`, `name`, `utc`, `variant`, `summary`, `convert_at` (char level, 0 = never), optional `hold_until_class`, and a table
  `[meta.wizard]` with all five keys above.
- Engine rules (doc 08 Phase 3 Item 3): after conversion every level is the Jedi class (auto level-up levels the NEWEST class only), max 2 classes,
  the Jedi class arrives only via the story script. `check_paths.py` must report 0 errors.
- Light-side presets avoid dark-side powers (mod #142 Alignment Affects Force Powers); dark presets are flagged as such.
- Aim for 5-8 presets per character covering the LOGICAL combinations (do not enumerate nonsense such as `caster` + `unarmed` for a Consular healer).
