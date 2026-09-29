# Mod Idea 4: Shield Bar (party portrait HUD)

**Status (2026-09-29): idea captured + data layer verified from the live
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
droid items). Other shield-like sources, **pool status unverified**:
Force Barrier 135-137 (`m_imp_fpaligned`), Resist Energy 40/42/133 and
Force Immunity 20 (`k_sp1_generic`), plus effect types
`DAMAGE_RESISTANCE`(1, has an optional `nLimit` pool),
`DAMAGE_REDUCTION`(7), `TEMPORARY_HITPOINTS`(9). Force Armor/Shield
(17/24) are AC/save buffs — **no pool, never on the bar**; they stay in the
Buff Duration HUD.

## Display recommendation

Two separate channels: **bar length = remaining absorption**, **color +
pips = coverage**.

- **One bar per portrait, stacked segments** — not one bar per shield.
  Overlap in practice is small (a force shield + a Force Barrier, maybe a
  resistance pool), so cap at 3 segments; merge extras. Segment order =
  engine absorption order (open item S4). Bar full-scale = sum of the
  active shields' max `amount`, so the bar drains in true proportion.
- **Segment color by shield family** (derived from `damageflags`, not per
  damage type — nearly every shield covers several):

  | Family | Rule | Color |
  |---|---|---|
  | Energy | blaster/elec, no physical | **cyan** `#4FE3FF` |
  | Kinetic | physical only (Mando melee) | **amber** `#F2B640` |
  | Hybrid | energy + physical (Mando power, Drexl) | cyan/amber diagonal stripe |
  | Force | Force Barrier / Force-type absorb | **violet** `#B07CFF` |
  | Generic pool | temp HP / resistance limit | **white** `#E8ECEF` |

  Must stay distinguishable from the existing **red vitality** and
  **blue Force** bars — hence cyan (lighter, greener) rather than plain
  blue. Verify side-by-side in a live screenshot before committing.
  This follows the widely-read Overwatch/Destiny convention
  (health / amber armor / blue shields).
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
- **S3** Do Force Barrier / Resist Energy use a pool (`nLimit`) or not —
  decompile `m_imp_fpaligned`/`k_sp1_generic` NSS (data, no Ghidra).
- **S4** Absorption order when two pools cover the same hit; whether a
  second force shield replaces the first (likely) — decides max overlap.
- **S5** Whether card/bar rects are GUI-data-driven (see engine plan 3).
- **S6** Actual vitality/Force bar colors, from a live screenshot.

## Relationship to other mods

Shares Mod 2's creature/effect snapshot, so build it **after** Mod 2's
display redesign settles and ideally as a mode of the same DLL (one
effect walk per frame, one set of portrait hooks) rather than a second
patch hooking the same functions.
