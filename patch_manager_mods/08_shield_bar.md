# Mod Idea 4: Shield Bar (party portrait HUD)

**STATUS (2026-09-29, end of session 2): data layer FULLY DECODED (pool = `ints[2]` of the type-0x02 effect, see "Session 2 decode" at the bottom); draw site chosen + verified live (`0x00745774`); probe build 1.1.0-probe in `buff_duration_hud_src/` (source of record; the STOCK 16-patch set is what is installed live right now); style plan = re-use the vitality bar's own texture. OPEN: a blackout of the 3D scene while any item shield is active (see bottom); temp-HP (Fury) type id; leader toggle-button `.gui` shift. Older session-1 status follows.**

**Session-1 status: research-only, no code,
no live changes.** The Buff HUD (the prerequisite, Mod 2) is complete and
live. Resolved: data layer (`forceshields.2da`), S3 (which effects have a
pool), S5 (layout is GUI data, and a collision was found), and the
per-shield VFX colors (one Ghidra run, `0x00878f10`). Open: S1/S2/S4/S6/S7
(needs one live probe session) and the draw-site research (R-D below). The
next-phase plan lives in the Claude plan file written at the end of this
session; this doc is the research record. Reuses most of
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
  GUI edit in the ultrawide override generator.
  **Collision found:** left of the leader card is the toggle-button row
  `TB_STEALTH` 548,562 | `TB_SOLO` 584,562 | `BTN_SWAPWEAPONS` 620,562 |
  `TB_PAUSE` 656,562 (all 36x36). `TB_PAUSE` (x 656–692) already overlaps
  `PB_VIT1` (x 677–716) at y 562–598, which is harmless today only because
  the bar texture is padded. A leader shield bar at about x 660–676 would
  sit on `TB_PAUSE`, so the leader layout needs the 4 buttons shifted left
  by the bar width + gap (about 16–20 units). The small cards 2/3 have free
  space left of `PB_VIT2/3` (x < 737, y 443–518); the only overlap there is
  the invisible `LBL_ARROW_MARGIN` (3,129 794x333).
  The live HUD file is `override/mipc28x6_p.gui`, byte-identical (md5
  `53d2035832a81cb8688eb33add8b198f`) to
  `~/Games/KOTOR2_Mod_Workspace/ultrawide_gui/out/mipc28x6_p.gui`. The
  vanilla original is `ultrawide_gui/originals/mipc28x6_p.gui`, md5
  `d043845b...`. The generator is `scripts/generate_ultrawide_gui_overrides.py`.
  Its docstring says the Aspyr HUD is drawn 1:1, anchored to the screen
  edges (2.667 px/unit measured), and not resized, and its HUD specs are
  behind `--include-hud`. So check how the live `out/` copy was produced
  before editing it.
  **Still open**: a new
  `PB_SHIELD%d` control in the `.gui` would NOT be bound (`FUN_007450e0`
  binds fixed tags by name), so the bar itself still needs the draw hook.
  The GUI edit only reserves the space.
- **S6** Actual vitality/Force bar colors, from a live screenshot.
- **R-D (draw site)**: the Buff HUD's draw hook `0x00745AEC` sits inside
  `PortraitIconRowDraw` (`0x007457e0`), which the per-slot gate
  `FUN_00745770` only calls when the engine's own arrow count
  (`slot+0xddc`/`+0xddd`) is non-zero. It also runs inside the card's
  clip viewport, while the shield bar sits outside the card. So the shield
  bar needs its own every-frame, per-portrait draw site in full-HUD
  coordinates. Candidates, in order: (a) inside `FUN_00745770` before its
  arrow-count test; (b) near the end of `CSWGuiMainInterface::Draw`
  (`0x0074b3f0`) after the portrait panel draws. Note the **entry**
  `0x0074B3F0` is already hooked by Force Power Hotbar (`HotbarHudTick`).
  The PB_VIT on-screen rect is hypothesized at `slotPtr+0x690`
  (`CSWGuiControl` header: x `+4`, y `+8`, w `+0xc`, h `+0x10`), where
  `slotPtr = hudThis+0x1f34+i*0xde4`. This is not yet verified, because
  `Adjust`'s `this` may be the slot object or something else.
- **S7** Whether a GUI `CSWGuiBorder` fill animates a `proceduretype
  cycle` texture (decides animated fill vs. flat hue). Cheapest test:
  live, with Mod 2's existing fill-swap pointed at `fx_tex_14`.

## Engine facts reused from Mod 2 (verified, see 02_buff_duration_hud.md)

- `CGameEffect`: `+0x8` ushort type, `+0xa` ushort subtype/flags (the Mod 2
  probe logged `+0x8` as an int, e.g. `0x90028`), `+0xc` float duration (s),
  `+0x10`/`+0x14` expiry day/ms, `+0x18` creator id, `+0x1c` spells.2da row,
  int params: count `+0x30`, POINTER `+0x34` (`GetInteger(i)` = `((int*)*(int*)(e+0x34))[i]`, `0x005e4e60`; the earlier text here was wrong),
  float params `GetFloat` = likely the same pointer scheme (unverified), object ids
  `+0x78`, strings `+0x48` (stride 8).
- Creature active effects: `creature+0x148` (`CGameEffect**`), `+0x14c` count.
- Slot → creature: Buff HUD hooks 1a/1b (`SnapshotPortraitSlot`, in
  `UpdatePortraits`) keep a 4-entry `slotPtr → creaturePtr` side table,
  refreshed every frame.
- Drawing: `FUN_0047eb60(char resref[17])` → `CAurGUIImageInternal*`, draw
  = its vtable slot 6 (thiscall `x,y,w,h,flags=0,rot,float* rgb,alpha`,
  pixels of the current viewport, full-texture UVs). Clip/fill rect =
  `FUN_00479460(x,y,w,h,float* color,flag,alpha)`, pop with `FUN_004799c0`
  (a non-"no fill" color fills the rect, which gives a solid-color bar with
  no texture). Batch flush = `FUN_0047ea60(float [0x009f5f60])` before and
  after. HUD scale = `FUN_00479ab0()` (2.0 here).
- Reference implementation: `buff_duration_hud_src/BuffDurationHUD.cpp`
  (identical to the framework clone's `Patches/BuffDurationHUD/`).

## Live patch state + hook addresses already taken (2026-09-29)

16 patches live (`patch_config.toml` in the game dir): k2-visible-console,
good-alignment-heads, k2-aspyr-normal-map-fix,
k2-aspyr-saber-hilt-texture-fix, k2-finesse-melee-fix, k2_grass_fog_fix,
max-lights, semi-transparent-letterbox, k2-mp3-playback-fix,
loading_screen_line_fix_k2, k2-aspyr-map-aspect-fix,
post-combat-movement-fix-v1, additional-console-commands,
buff-duration-hud, force-power-hotbar, inventory-3d-viewport. Newest full
staging dir: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/inv3d_live_patches/`
(16 `.kpatch`). Addresses already hooked; a new hook must not reuse
these or overlap their byte windows: `0x0070CDCB`, `0x0070CF34`,
`0x00885197`, `0x00894435`, `0x00895E6D`, `0x007508AA`, `0x007AF198`,
`0x0091D52D`, `0x004752D8`, `0x0074F25A`, `0x0074F8AB`, `0x00745A84`,
`0x00745A97`, `0x00745AEC`, `0x0072CB63`, `0x0074B3F0`, `0x008A69AD`,
`0x008A8172`, `0x008A7350` (plus the Max Lights / letterbox / normal-map /
saber / finesse / grass hooks, which are in their `.kpatch` hooks files).
Get the complete list from the steam log's `Applied ... hook at` lines.

## Relationship to other mods

Shares Mod 2's creature/effect snapshot. Mod 2 is done, so build it now,
as a feature of the same DLL/patch (`buff-duration-hud`) (one
effect walk per frame, one set of portrait hooks) rather than a second
patch hooking the same functions.

## Phase A0 result (2026-09-29): draw site chosen

`FUN_00745770` is `thiscall(slotPtr, float dt, int idx, int isFirst)`, `RET 8`... args at
`[ebp+8]/[ebp+0xc]/[ebp+0x10]`. Called from `Draw` at `0x74c2dd` (3 slots) and `0x74c2f9`
(4th/mirror slot, `isFirst=0`) after the panel-control loop and outside its clip viewport, so it
is full-HUD coordinates. Splice `0x00745774`, bytes `89 4D FC 8B 45 FC`, params `ecx`, `[ebp+0x10]`.
Probe build `1.1.0-probe` (`ShieldProbeSlot`) uses it; flag file `shieldprobe.txt`.

## Phase A results, probe 1 (2026-09-29, log `shield_bar_src/logs/shieldprobe_live1.txt`)

- **R-D**: works. Hook `0x00745774` runs every frame per slot in full-HUD pixels (3840x1600). Rects: leader
  vit `(3511,1386,103,208)`, force `(3762,1386,103,208)`, card `(3591,1394,191,192)`; small cards vit `(3671,1285,47,96)`,
  card `(3711,..,85,85)`. `isLarge` is `[ebp+0xc]` (not 0x10). The 4th slot has creature 0 and zero rects (skip it).
- **Geometry**: the bar art has ~47% transparent padding on the rect's left; the visible capsule spans
  `x+0.47w .. x+0.77w` (about 31 px on the leader, 14 px on small cards). Rect A at `x-gap-W` landed 48 px too far left.
  Anchor the shield bar at `x + 0.47w - gap - W`, `W ~ 0.30w`. The leader toggle buttons (`TB_PAUSE` ends ~3545) then
  collide: shift them left (user confirmed, Phase D).
- **S7 = NO**: GUI-drawing `fx_tex_14/16` gave garbled flicker AND coincided with the scene going black during shield
  activation (cleared when the shield ended; menus and mouse-look were also odd). Flat colors only. Probe 2 has the textured
  draws removed to confirm the black screen goes away.
- **S2 (partial)**: shield activation = 4 effects sharing spell id (99 Energy, 132 Heat): types `0x02/sub 9`, `0x1e`, `0x43`, `0x6b`, dur 224 s.
- **S1 (open)**: `+0x34`=count(8), `+0x38`=pointer to the int array (params are not inline). Probe 2 logs the array. No drain was seen yet
  because only the pointer was compared.
- **S4/S6**: S6: health bar is red (`~#C0300F`-ish rim), Force bar cyan (`#00D0E8`-ish), portrait frame teal. S4 not observed.

## Session 2 decode (2026-09-29): everything the implementation needs about the data

**Force shield mechanics (all decompiled; DB lessons 94-98, functions `00594810`, `00541fd0`, `00593a30`).**
- Effect **type `0x6b`** = `EffectForceShield`. Its apply handler `FUN_00594810` reads the `forceshields.2da` row and applies two
  child effects that share the shield's spell id (`+0x1c`): **type `0x1e` sub 1** (`ints[0]` = `VisualEffectDef`, the visualeffects.2da
  row 2040..2048/2060, i.e. the color key!) and **type `0x02` sub 9** (`ints[0]=DamageFlags, [1]=Resistance(300), [2]=Amount = THE POOL,
  [3]=VulnerFlags(2048 ion), [4]=1`). Activation also leaves a type `0x43` sub 1 icon effect and the `0x6b` wrapper (dur 224 s in the log).
- Absorb code `FUN_00541fd0` (type 2): `pool=GetInteger(2)`; each hit `SetInteger(2, pool - damage*mult)` in place (mult 2 vs ion, +1 for
  some attacker classes); if `pool - absorbed < 1` the effect is removed (shield breaks). **So remaining = `ints[2]`; max = table `Amount`
  (look up by (`0x1e.ints[0]`, `0x02.ints[0]`) or use the largest value seen), and the pool never refills.** `FUN_00541430` is the
  sibling for type 12 (EffectDamageResistance with a limit: `[1]`=per-hit, `[2]`=limit pool, 0 = unlimited), same in-place decrement.
- Effect ints live behind a pointer: `+0x30`=count, `+0x34`=`int*`. Probe 1 (`ints` read inline) only ever saw the pointer, which is why
  no drain showed. Probe 3 (built, staged, NOT yet run) reads `+0x30/+0x34` and logs `arr@` dumps. **Still unverified live: that
  `ints[2]` actually decrements on hits (expected from the decompile).**
- Temp HP (Fury/Wookiee Rage): engine effect-type id still unknown (nwscript `EFFECT_TYPE_TEMPORARY_HITPOINTS`); find it by logging APPLY
  while casting Fury (probe 3 does this) or from the apply table in `FUN_00593a30`. Ignore for v1 if not found.

**Draw site (verified live, probe 1).** `FUN_00745770` (thiscall slot; args `dt`, `isFirst`, RET 8), splice `0x00745774`
`89 4D FC 8B 45 FC`, params `ecx`=slotPtr, `[ebp+0xc]`=isFirst(leader). In full-HUD pixels (3840x1600 here, scale 2.0). 4th slot = creature 0,
rect 0 (skip). Slot rects (`slot+0x694..`, `+0x800..`, `+0x54c..`): leader vit `(3511,1386,103,208)`, card `(3591,1394,191,192)`; small
vit `(3671,1285|1181,47,96)`.

**Bar art / natural style (this is the "curved, looks like part of the health bar" answer).** The HUD `.gui` draws each bar twice with the
SAME stretched 128x256 white+alpha TGA (`uibit_bar_vp_p` vitality, `uibit_bar_fp_p` Force = mirror): BORDER = teal track `(0.051,0.349,0.271)`,
PROGRESS = red `(1,0,0)` / cyan `(0,0.8,0.898)`. The texture is a curved-left capsule: outline alpha ~1, interior alpha ~0.34, opaque
x-fraction `0.445..0.773` (w = 0.328), y `0.016..0.988`. Pictures: `shield_bar_src/uibit_bar_vp_p_on_bg.png`, `probe1_hud_crop.png`,
`probe2_hud.png`. Plan: draw `uibit_bar_vp_p` (vtable-6 image draw, tint = color, like Buff HUD icons; plain TGAs are safe) at rect
`(vit.x - round(0.328*vit.w) - gap, vit.y, vit.w, vit.h)`: first tinted teal `(0.051,0.349,0.271)` alpha 1 (empty track), then tinted with the shield
VFX color, clipped bottom-anchored to `remaining/total` of the height (PushClipRect/PopClipRect with the no-fill color `0x00a1ba3c`, same as
`DrawBuffColumn`). The curved side faces left = outer side, consistent with the vitality bar. For a second segment stack them the same way
(or clip a taller region). Leader toggle buttons (`TB_STEALTH/TB_SOLO/BTN_SWAPWEAPONS/TB_PAUSE`, x 548/584/620/656, y 562, 36x36 gui units)
must move left by about `0.328*39 + 1.5` = 14.3 -> use 16 gui units (the user asked for this); `TB_PAUSE` currently ends ~x 3545 px.

**Blackout during shield (OPEN, probably not ours).** Both probe runs: scene 3D goes black while an item shield is active, HUD still draws,
mouse-look/menus odd, clears when the shield ends. Probe 2 had no textured draws, so not the fx_tex GUI draws. Live `override/fx_tex_14.tga` (mod #35) is a
256x4096x32 uncompressed 4 MB TGA (no txi). Suspect the creature texture-shell path (`ProgFxTextureShell` 0x00878f10 -> the mod35 TGA) under
Proton, independent of our DLL. Decisive test (not yet done): stock 16 patches, activate a shield, watch. Also possible: `shieldprobe.txt`-gated per-frame
`fopen` in the probe (unlikely, the logs are small). Mouse settings: nothing of ours touches them; `Mouse Sensitivity` was found 4 vs 44 in backups and restored to 44
(ini backup `~/Games/KOTOR2_Mod_Workspace/backups/ini_pre_sensitivity_restore_20260929_202924/`).

**v2 ideas (not v1):** coverage pips, hit/break/ion flashes, "-N per hit" badges for Force Barrier/Resist (type 12 with no limit), per-hit floating numbers.

## Session 3 live results (2026-09-29, probe2 = `1.1.0-probe2`, log `shield_bar_src/logs/shieldprobe_live3.txt`)

- **Bar look: works.** The curved textured bar (`uibit_bar_vp_p`, teal track + tinted clipped fill) sits left of the vitality bar on the leader card, blue for row-2040 shields, red for row-2045 shields.
  User asked for a smoother join into the curved health bar (gap set to 0 in probe3; the square right edge vs the rounded health corner leaves corner wedges).
- **Blackout is NOT the shield bar and NOT the fx_tex_* shell textures per se.** With the probe DLL: human shields (spell 99, vfx 2040) on the leader and on a companion never blacked out.
  The scene went black only while the **droid's** shield (spell 110, `d_shield`, vfx 2045) was active; it cleared when that shield ran out. The Peragus mining shield (spell 132, also vfx 2045) on the human leader did **not**
  cause it (user correction: it was still active when the black cleared). So it is tied to the droid shield path (droid sphere model / appearance-driven VFX), not to visualeffects row 2045. Stock-set droid test still pending.
- **S1 still unverified**: no hits were taken, no CHANGE/`POOL` remaining changes in the log.
- **Companion cards inconsistent**: small-card shield bars and Buff HUD icons appear/disappear together (same slot->creature table) -> probe3 adds `DRAW slot= creature= pools= effects=` lines to explain it.
- **Fury/temp HP type id** not yet observed. Shield hover tooltip (user request): needs research of the portrait hover-text builder (v2).
