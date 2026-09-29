# Mod Idea 2: Visual Buff Duration HUD

**Status:** Phase 2 (Ghidra static analysis) IN PROGRESS, started 2026-09-29.
Real class names and strong candidate functions found for both targets;
exact `DurationRemaining` offset and exact per-frame hook instruction not
yet pinned down. No hooks written, no `.kpatch` built.

## Phase 2 findings (2026-09-29)

**Community database check, per the effort-estimate note below**: no
external GZF/SARIF database was consulted — a better one was already on
disk. The Patch Manager framework itself ships `Patches/Common/GameAPI/`,
~50 already-reverse-engineered class headers (`CSWSCreatureStats.h`,
`CSWSObject.h`, `CGuiInGame.h`, `CSWGui3DSceneView.h`, etc.) built from
prior community/framework-author RE work — check this before any fresh
Ghidra RTTI search; it already ruled out one dead end (`CGuiInGame` is
K1-only, becomes a safe no-op on K2 — don't target it) and, via a sibling
already-shipped patch's own source comment
(`K2AspyrControllerFixes/HelpMenuInputFix.cpp`, native-Linux-Aspyr build),
confirmed the real HUD class name below without needing to guess.

**Target 1 (data layer) — real class names found, exact duration offset
still open.**
- `CUIHUD`/`CGameEffect` (the spec's original SDK-style guesses) are
  **not real RTTI class names in this binary** — confirmed by a 0-hit
  keyword search. The real classes: `CSWSEffectListHandler` (vftable
  `0x00992490`) and its base `CGameEffectApplierRemover` (vftable
  `0x009924a4`), both RTTI-confirmed. Constructed together in
  `FUN_0051d2f0`, embedded in a larger (not-yet-named) owning object at
  `+0x58` (immediately followed by a parallel
  `CSWSItemPropertyHandler`/`CItemPropertyApplierRemover` pair at `+0x5c`).
- Separately, `FUN_006a5110` (a ~7.7KB AC-bonus recompute function, found
  by following the `"+ %d (Effect_AC_Shield_Bonus)"`-style debug strings)
  confirmed the actual active-effect list's on-the-wire shape: an array
  header at `ownerObj+0x24 → +0x148` (`{basePtr, count}`), 4-byte pointer
  elements, each effect object carrying a `ushort` type/subtype tag at
  `+0x8` and further integer parameters read through an accessor call
  `FUN_005e4e60(index)` rather than raw struct offsets.
- **Not yet resolved, and flagged as the concrete next step**: whether the
  `+0x58` `CSWSEffectListHandler` and the `+0x24→+0x148` array are the same
  subsystem viewed two ways, or two different things. The `DurationRemaining`
  float itself has not been isolated — `FUN_005e4e60` (the per-effect field
  accessor) hasn't been decompiled yet, and the actual expiration/removal
  function (which must read a duration and compare/decrement it, per the
  spec's "follow the existing read" guidance) hasn't been located. Full
  detail, addresses, and the exact next action are in the `ghidra_knowledge`
  DB's `lessons` table (scope `Buff Duration HUD`, topic "Target 1 partial").

**Target 2 (render layer) — strong candidate functions found, not yet
disambiguated Update-vs-Render.**
- Confirmed `CSWGuiMainInterface` (not a guessed `CUIHUD`) is the real
  main-HUD class — same class already known from `K2AspyrMapAspectFix`'s
  minimap work, independently reconfirmed via
  `K2AspyrControllerFixes/HelpMenuInputFix.cpp`'s own
  `CSWGuiMainInterface::HideHelp` reference.
- Its vtable slot [13], `FUN_0074b3f0` (`__thiscall(this, float dt)` —
  per-frame signature), is a strong per-frame Update candidate: early-outs
  on a visibility bitflag, decrements several countdown-timer floats by
  `dt`, builds tooltip/status text from StrRefs `0x1e61e`-`0x1e621`, and
  ends with 3 calls to `FUN_00745770(dt, isFirstSlot)` — a party-of-3
  portrait-update loop.
- Its vtable slot [14], `FUN_0074c320` (same `(this, float dt)` shape), is
  a second per-frame pass that walks **two parallel fixed-size widget-slot
  arrays inside the instance**: an action-bar-shaped array (base `+0x733c`,
  7 entries, stride `0x750`) and a party-portrait-shaped array (base
  `+0x120`, 3 entries, same `0x750` stride), each slot having 4 notable
  sub-offsets at `+0`/`+0x1d0`/`+0x3a0`/`+0x570` — a pattern consistent
  with icon-quad / cooldown-overlay-quad / label / tooltip sub-widgets per
  slot. **Promising lead, not yet confirmed**: this suggests the action
  bar *already implements* a per-slot cooldown-wipe overlay for force-power
  recharge timers — if so, the buff-duration HUD could structurally mirror
  (or literally reuse) that existing draw primitive instead of building a
  new one from scratch. Not decoded field-by-field yet.
- Vtable slot [15], `FUN_007469f0`, was checked and **ruled out** — it's an
  on-demand GUI-event dispatcher (radial menu / quick-menu actions), not a
  per-frame tick.
- **Not yet resolved**: whether slot [13] or [14] (or a further slot not
  yet examined) is closer to "the" per-frame draw entry point the spec
  wants to hook *after*, versus being upstream numeric bookkeeping that
  some separate, not-yet-found `Draw()`/`Render()` virtual actually
  consumes. Full addresses and vtable dump in the `ghidra_knowledge` DB's
  `functions`/`classes` tables.

**Effect ID table (data lookup, not Ghidra) — DONE.** Pulled directly from
this install's live `override/spells.2da` (binary V2.b format, parsed via
`pykotor`; confirmed genuinely install-specific — its MD5 differs from
both the pre-compatch backup and the raw TSLRCM copy). Row index in this
2DA is each power's real `SPELL_*` constant/ID. Self-buff (duration-based)
force-power rows found in this install:

| Row | Label | Icon |
|---|---|---|
| 8 | `FORCE_POWER_SPEED_BURST` | `ip_speedburst` |
| 34 | `FORCE_POWER_KNIGHT_SPEED` | `ip_knightspeed` |
| 37 | `FORCE_POWER_SPEED_MASTERY` | `ip_masterspeed` |
| 22 | `FORCE_POWER_FORCE_VALOR` | `ip_mind` |
| 33 | `FORCE_POWER_KNIGHT_VALOR` | `ip_knightmind` |
| 36 | `FORCE_POWER_MASTER_VALOR` | `ip_mastermind` |
| 17 | `FORCE_POWER_FORCE_ARMOR` | `ip_armor` |
| 18 | `FORCE_POWER_FORCE_AURA` | `ip_aura` |
| 24 | `FORCE_POWER_FORCE_SHIELD` | `ip_shield` |
| 135 | `FORCE_POWER_FORCE_BARRIER` | `ip_frcbarrier01` |
| 136 | `FORCE_POWER_IMPROVED_FORCE_BARRIER` | `ip_frcbarrier02` |
| 137 | `FORCE_POWER_MASTER_FORCE_BARRIER` | `ip_frcbarrier03` |
| 138 | `FORCE_POWER_BATTLE_MEDITATION_PC` | `ip_battlemed01` |
| 139 | `FORCE_POWER_IMPROVED_BATTLE_MEDITATION_PC` | `ip_battlemed02` |
| 140 | `FORCE_POWER_MASTER_BATTLE_MEDITATION_PC` | `ip_battlemed03` |

Icon `resref`s confirm the "reuse existing power icons" plan in the
Implementation pipeline below is viable as-is — every row already has a
`.tga`-backed icon resref shipped in this build.

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
- Detour the equivalent draw call on `CSWGuiMainInterface` (confirmed real
  class name, not `CUIHUD` — see Phase 2 findings above) so our draw
  happens *after* the engine's own HUD pass, same frame. Exact vtable
  slot/symbol still TBD between two strong candidates
  (`FUN_0074b3f0`/`FUN_0074c320`, vtable slots [13]/[14]) — see Phase 2
  findings.
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
