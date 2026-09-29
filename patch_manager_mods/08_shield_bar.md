# Mod Idea 4: Shield Bar (party portrait HUD)

**Status (2026-09-29): Buff HUD (the prerequisite) is complete; S3 resolved. Idea captured + data layer verified from the live
`forceshields.2da` / `spells.2da`. No Ghidra work yet.** Reuses most of
[[02_buff_duration_hud.md]]'s already-resolved engine map (portrait slot →
creature chain, `CSWSObject` active-effect array, `CSWGuiBorder` live
width shrink, `PB_VIT%d`/`PB_FORCE%d` layout slots), so the fresh research
surface is small — see "Open research" below.

## Goal

Add a **shield bar** next to each party portrait's existing vitality
(`PB_VIT%d`) and Force (`PB_FORCE%d`) bars, shifting the existing card
components over to make room. The bar shows remaining shield absorption,
what damage types it covers, and handles overlapping shields.

## Data facts (verified on the live install, not assumed)

Energy shields are **not** generic resistance effects: they are
`EffectForceShield(nRow)` (nwscript #459) → a row in `forceshields.2da`,
which carries a real **absorb pool** (`amount`) and a **coverage bitmask**
(`damageflags`, same bit values as `DAMAGE_TYPE_*`). Every row has
`vulnerflags=2048` (ion) and `resistance=300` (semantics unconfirmed).

| Row | Label | amount | covers |
|---|---|---|---|
| 0-2 | SHIELD_DROID_ENERGY_1/2/3 | 50/70/100 | energy(blaster), elec |
| 3-5 | SHIELD_DROID_ENVIRO_1/2/3 | 60/80/110 | energy, ion, sonic, fire, cold, **light-side**¹ |
| 6 | SHIELD_ENERGY | 40 | energy, elec |
| 7 | SHIELD_ENERGY_SITH | 50 | energy, elec, sonic |
| 8 | SHIELD_ENERGY_ARKANIAN | 80 | energy, elec, sonic, fire, cold |
| 9 | SHIELD_ECHANI | 100 | energy, elec |
| 10 | SHIELD_MANDALORIAN_MELEE | 50 | **physical only** (bludg/pierce/slash) |
| 11 | SHIELD_MANDALORIAN_POWER | 70 | energy + physical |
| 12 | SHIELD_DUELING_ECHANI | 130 | energy, elec |
| 13 | SHIELD_DUELING_YUSANIS | 200 | energy, elec, ion |
| 14 | SHIELD_VERPINE_PROTOTYPE | 170 | energy, elec, sonic, fire, cold |
| 15 | SHIELD_ANTIQUE_DROID | 110 | same as enviro |
| 16-18 | SHIELD_PLOT_* | 300/400/300 | energy, elec, ion |
| 19 | SHIELD_HEAT | 20 | energy, ion, fire |
| 20 | SHIELD_DREXL | 90 | **everything** (16383) |

¹ `7520` includes bit 64 (`LIGHT_SIDE`), not 16 (`ACID`) — likely a
vanilla data quirk; display what the bitmask says, don't "fix" it.

Invoking spells.2da rows: 99-107, 110-115, 132, 257 (`k_sup_bands` /
droid items). Other shield-like sources were checked in S3 below: Force
Barrier and the Resist powers have **no pool**, and Fury uses
`TEMPORARY_HITPOINTS`(9), which **is** a pool. Force Armor/Shield
(17/24) are AC/save buffs — **no pool, never on the bar**; they stay in the
Buff Duration HUD.

## Display recommendation

Two separate channels: **bar length = remaining absorption**, **color +
pips = coverage**.

- **One bar per portrait, stacked segments** — not one bar per shield.
  Overlap in practice is small (an item shield + Fury temp HP), so cap at 3 segments; merge extras. Segment order =
  engine absorption order (open item S4). Bar full-scale = sum of the
  active shields' max `amount`, so the bar drains in true proportion.
- **Segment color = the shield's own in-world VFX color** (user's idea,
  2026-09-29; supersedes the earlier family palette). Each segment matches
  the glow the player sees on the character, so bar and model are
  visibly the same shield. Table below.

### Shield → VFX → color (traced through data + exe, 2026-09-29)

Chain: `forceshields.visualeffectdef` → `visualeffects.2da` row 2040–2048
(`VFX_DUR_SHIELD_*`) → `progfx_duration` 1413–1421 → exe
`ProgFxTextureShell` (`0x00878f10`), a switch that picks texture
`fx_tex_(progfx−1400)`, **skipping 13**. The shell is an animated
flipbook texture laid over the creature mesh. The row labels are
misleading: both "CHROME" rows are purple. The live textures come from
**mod #35 Animated Energy Shields** (same hues as vanilla, lighter). Hue
= brightness-weighted mean over all frames, normalized to its max channel.

| forceshields row(s) | VFX | texture | vanilla hue | **live hue (bar color)** |
|---|---|---|---|---|
| 3-5 droid enviro, 6 Energy | BLUE_01 | fx_tex_14 | `#0000FF` | **`#4CACFF`** sky blue |
| 7 Sith | GREEN_01 | fx_tex_15 | `#16FF72` | **`#7CFFC7`** mint |
| 0-2 droid energy, 8 Arkanian, 16-18 plot, 19 Heat | RED_01 | fx_tex_16 | `#FF0000` | **`#FF0A0A`** red |
| 9 Echani | BLUE_02 | fx_tex_17 | `#000AFF` | **`#385EFF`** royal blue |
| 10 Mando melee | CHROME_01 | fx_tex_18 | `#8C00FF` | **`#DD92FF`** lavender |
| 11 Mando power | CHROME_02 | fx_tex_19 | `#DB00FF` | **`#CE63FF`** violet |
| 12 Dueling Echani | BLUE_03 | fx_tex_20 | `#0009FF` | **`#6473FF`** periwinkle |
| 13 Dueling Yusanis | BLUE_04 | fx_tex_21 | `#0C00FF` | **`#0D0CFF`** deep blue |
| 14 Verpine, 20 Drexl | RED_02 | fx_tex_22 | `#FF7A01` | **`#FF9B24`** amber |
| 15 Antique droid | SPIDER_OLD (model) | fx_tex_12 | `#FF5D00` | **`#FF450B`** orange |

Visual reference: `shield_bar_src/shield_vfx_textures.png` (top = vanilla frame 0, bottom = live mod #35).

Droids get a sphere model instead of the shell (`appearance_0N` →
`visualeffect_0N`, e.g. `v_fieldmrk1_dur`). Its color is the mesh's
self-illumination controller (id 100) over the grey `fx_tex_13`: the red
field marks are `#F83500`/`#F53A00`/`#F01C00`, the blue ones
`#0066FF`/`#0072FF`/`#005AFF`. These are the same hue family as the humanoid
shell for the same row (energy = red, enviro = blue), so one color per
row works for both.

**Best version: use the texture itself as the segment fill**, not just its
average color. Mod 2 already swaps a `CSWGuiBorder` fill image to an
arbitrary resref (`FUN_0047eb60`), so the bar can show the literal
`fx_tex_NN` pattern. Open question (**S7**): does a GUI border draw
advance a `proceduretype cycle` flipbook? If it does, the bar animates in
sync with the shield. If not, it shows one frame (or a squashed 16-frame
strip, for mod 35's 256x4096 TGAs); then crop via UVs or fall back to the
flat hue above.

**Conflict to check live (S6):** red shields sit next to the **red
vitality** bar, and the blue ones next to the **blue Force** bar. The
textured fill, a bright 1-unit rim and the bar's own column should
separate them. If not, lift lightness only and keep the hue so the tie
to the VFX survives.

Fury temp HP (no shield VFX): the exe's `fx_Fury01-03` textures are dark
red (`#950000`), and their only xref is `FUN_00442090` (a GL overlay,
unconfirmed). Use white `#E8ECEF` unless live testing says otherwise;
Fury-red would collide with vitality.
- **Coverage pips** (tiny squares under/beside the bar, only for the
  elemental extras a family color doesn't imply): fire orange `#FF7A2E`,
  cold ice `#BDEBFF`, sonic purple `#C77DFF`, ion yellow `#FFE14F`,
  acid green `#7BE35A`. Pips + stripes + lightness differences keep it
  colorblind-safe without relying on hue alone.
- **Events**: segment flashes white on hit, fades out on break; brief
  yellow flash when **ion** (every shield's `vulnerflags`) hits it.
- **Time-only effects are not on the bar** — no invented capacity.

## Engine plan (reuse from Mod 2 wherever possible)

1. **Data read**: per frame, walk the creature's active-effect array
   (`CSWSObject+0x148`/`+0x14c`, from Mod 2) for force-shield /
   resistance-with-limit / temp-HP effects; read max + remaining.
   Creature per portrait via Mod 2's resolved 3-hop chain / snapshot
   hooks 1a/1b.
2. **Render**: the portrait card's bars are `CSWGuiProgressBar`
   (`PB_VIT%d` = `+0x690`, `PB_FORCE%d` = `+0x690+0x16c`, laid out by
   `Adjust` `0x00745560`; `CSWGuiProgressBar::Draw` `0x0041bb20`, a 2-line
   forwarder to embedded `CSWGuiBorder`s). Candidate: after the native
   bars draw, re-position + re-color + width-shrink one of them (Mod 2's
   proven live-field technique, then restore), once per segment.
   Alternative: a dedicated widget built by the same path Mod 2 uses for
   fill images (`FUN_0047eb60`).
3. **Shift existing components**: prefer a **pure GUI-data** change if the
   card rects come from the HUD `.gui` (our ultrawide override pipeline
   already rewrites these) — widen the card/shift `LBL_CHAR`/`BTN_CHAR`/
   bars by bar width + gap. Fall back to a post-`Adjust` hook only if the
   engine recomputes those rects in code.

## Open research (S-items)

- **S1** Where the remaining pool lives on a live force-shield
  `CGameEffect` (which `GetInteger(n)` slot decrements). Cheapest path:
  the same throwaway `ApplyEffect` probe used for Mod 2's item 3 — log
  effect type + all integer params on apply, take damage, dump again.
- **S2** Effect-type ID the engine uses for `EffectForceShield` (not in
  pykotor's `EFFECT_TYPE_*` list; `ELEMENTALSHIELD`=60 is a candidate).
- **S3 RESOLVED (2026-09-29, from `k_inc_force.nss`)**: **no pool.**
  Force Barrier / Improved / Master apply
  `EffectDamageResistance(BLUDGEONING|PIERCING|SLASHING, 4/8/15)` with no
  `nLimit`. Resist Cold/Heat/Energy, Master Energy Resistance and
  Resist Poison/Disease/Sonic apply `EffectDamageResistance(cold, fire,
  sonic, blaster, elec, 12/20/6)`, also with no limit. So these are flat
  per-hit reduction for a duration, and they stay out of the bar (they are
  already Buff HUD icons). At most, show a small "−N per hit" badge on the
  card. The one force-side real pool found is **Fury / Wookiee Rage**:
  `EffectTemporaryHitpoints(nFinalVP)`, applied deliberately unlinked "since
  when the points were used up the effect was destroyed". That goes on the
  bar as the "generic pool" family. Item energy shields
  (`EffectForceShield`) are not built in `k_sup_bands.nss`; the engine
  applies them from the item property, so S1/S2 still need the live probe.
- **S4** Absorption order when two pools cover the same hit; whether a
  second force shield replaces the first (likely) — decides max overlap.
- **S5 PARTLY RESOLVED (2026-09-29)**: the card/bar rects **are** GUI data,
  in `mipc28x6_p.gui` (our ultrawide `out/` copy, 800x600 virtual units).
  Leader card: `PB_VIT1` 677,520 39x78 | `LBL_BACK1` 704,520 78x78
  (`LBL_CHAR1`/`BTN_CHAR1` 707,523 72x72) | `PB_FORCE1` 771,520 39x78. The
  bars overlap the portrait frame by about 12 units, probably because the
  bar textures have transparent padding. Small cards 2/3: `PB_VIT` 737 18x36 |
  back 750 36x36 | `PB_FORCE` 781 18x36, at y=482 / 443. Health is on the
  left and Force on the right, so the shield bar belongs **immediately left
  of `PB_VIT`**. Shifting the card group right (or `PB_VIT` left) is a pure
  GUI edit in the ultrawide override generator. **Still open**: a new
  `PB_SHIELD%d` control in the `.gui` would NOT be bound (`FUN_007450e0`
  binds fixed tags by name), so the bar itself still needs the draw hook.
  The GUI edit only reserves the space.
- **S6** Actual vitality/Force bar colors, from a live screenshot.
- **S7** Whether a GUI `CSWGuiBorder` fill animates a `proceduretype
  cycle` texture (decides animated fill vs. flat hue). Cheapest test:
  live, with Mod 2's existing fill-swap pointed at `fx_tex_14`.

## Relationship to other mods

Shares Mod 2's creature/effect snapshot, so build it **after** Mod 2's
display redesign settles and ideally as a mode of the same DLL (one
effect walk per frame, one set of portrait hooks) rather than a second
patch hooking the same functions.
