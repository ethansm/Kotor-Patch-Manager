# Mod Idea 2: Visual Buff Duration HUD

**Status:** concept — not started. No Ghidra analysis, no hooks written.

## Goal

A per-frame HUD overlay showing remaining duration on active buffs (Force
Speed, Force Valor, etc.) as a "cooldown wipe" over the power's icon —
native-looking, 60 FPS, no NWScript involved.

## Why this is a Patch Manager task

The Odyssey engine has no concept of "buff" as a first-class HUD object —
Force Speed etc. are `CGameEffect` instances attached to `CNWSCreature`,
with an internal `DurationRemaining` float the game already tracks
(that's how the effect expires) but never exposes to any UI element. Prior
community attempts used NWScript polling to fake duration extensions
(the "long-buff" mods already common in this space) precisely *because*
script execution can't draw arbitrary UI — it's too slow and has no
draw-primitive commands. This only becomes viable with raw C++ hooked
directly into the native render cycle, which is exactly what Patch Manager
provides.

## Implementation pipeline

### 1. Data layer — read the effect list
- Detour into the game's main per-frame update loop.
- Each frame, walk the local player `CNWSCreature`'s `ActiveEffects` list.
- Filter for the spell IDs we care about (Force Speed = ID 12, etc. — full
  ID table needs pulling from `spells.2da`, not hardcoded from memory).
- Read each matching effect's `DurationRemaining` float.

### 2. Render layer — draw the overlay
- Detour `CUIHUD::Render()` (or the equivalent draw call — exact symbol
  TBD from Ghidra) so our draw happens *after* the engine's own HUD pass,
  same frame.
- Draw a `.tga` icon for each active tracked power at fixed HUD
  coordinates.
- Draw a semi-transparent quad over it, height scaled by
  `DurationRemaining / MaxDuration`, for the cooldown-wipe effect.

### 3. Hook construction (`.kpatch`)
- Two hooks needed (update-loop tick + HUD render), likely in the same
  patch package.
- `skip_original_bytes = false` on both — we are strictly additive, never
  replacing engine logic.
- New `.tga` icon assets: can mostly reuse existing power icons already
  shipped in the game/mod textures; only the overlay quad is drawn
  procedurally (no new art needed for that part).

## Open risks specific to our build

- **HUD coordinate system.** [[kotor2-ultrawide-gui]] already remapped HUD
  description backgrounds and action-description coordinates for
  3840x1600 (`HUD_DESC_COEFF`), and there's an open item there about slot 0
  / slot 5 calibration. Any new HUD overlay must use the *same* coordinate
  transform our ultrawide fix established, not raw 800x600 assumptions —
  otherwise the buff icons will sit in the wrong place only on this build.
- **Effect ID coverage.** Spell/effect IDs must be pulled from this
  install's actual `spells.2da` (TSLRCM, Thematic Sith Lords, Thematic
  Jedi Masters, and other mechanics mods in this build add/change rows —
  mods #8, #102-109, #117-119, #139-142 all touch mechanics/force powers).
  A hardcoded vanilla ID table will silently miss or mis-track powers this
  install has changed. Needs to read the *installed* 2DA, not a reference
  copy.
- **Overlap with "long buff" mods.** None currently installed in this
  build (checked `neocities_mods.json` — no duration-extension mod
  present), so no conflict today, but worth flagging: if one is ever added
  later, this mod becomes strictly better than that workaround and the two
  shouldn't coexist (the long-buff mod would make the timer overlay
  pointless, not simplify it).
- **Multiplayer/party members**: decide scope up front — player-only buffs
  (simplest, matches the original ask) vs. also tracking active party
  members' buffs. Recommend player-only for v1.

## Effort estimate (rough, pre-Ghidra)

Lower than the 3D viewport idea: no new GFF/GUI struct needed (pure
procedural draw), and the `DurationRemaining` field read is a straight
memory offset once `CGameEffect`'s layout is known — likely already
documented in existing community Ghidra databases given how central
effects are to the whole engine. The update-loop and `CUIHUD::Render()`
hook points are also the kind of "well-trodden" addresses more likely to
already be in a shared GZF (worth checking before reversing from
scratch — this is `03_framework_research.md`'s job to confirm).
