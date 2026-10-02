# Mod Idea 4: Shield Bar (party portrait HUD)

**STATUS (2026-10-02, end of session 5/6): LIVE v1.1.0 (DLL `2738ec1b7e1268f23c2e867cf019c955`, art `uibit_bar_sh_p` md5 `add4566d0c32486e417b414c02585124` + `uibit_bar_sc_p` in override/; backup `backups/shield_bar20_install_*`). STAGED, NOT INSTALLED: `343aebec2583d671b43c62e0601b2c7a` = bars (health/force/shield) + spacing on the MENU companion portraits (untested). See "Session 5/6 summary (2026-10-01/02)" at the very bottom for the finished look, the menu engine facts, the open items list and the research plan pointer.**

**STATUS (2026-09-30, end of session 4): WORKING LIVE, no black screen. Installed = `1.1.0-probe4` line, DLL md5 `81fb7d0845f93827d5ef1b7c74ce0e5d` (backup `~/Games/KOTOR2_Mod_Workspace/backups/shield_bar6_install_20260930_023738`): shield bars on all 3 portrait cards (drawn from the leader's slot call), slimmer bar + lightened fills + bright seam divider, squared health edge, `Shield: cur/max` line in the native portrait hover tooltip (hook `0x00744EEC`), menu-frame guard, GL-state restore. OPEN (next phase = colour/UX polish, see "NEXT PHASE" at the very bottom): red-shield vs red-health colour separation, divider/outline look, small-card bar size, tooltip refresh while hovering, Buff HUD icons vanishing with a robot leader (unverified after the fix), shield bar in menu screens, Phase 3 (rename `ShieldProbeSlot`->`DrawShieldBar`, trim probe logging, manifest 1.1.0, rollback set, final docs). The BLACK SCREEN (scene black while shields active) is FIXED: see "Session 4 summary + root cause" at the bottom.**

**STATUS (2026-09-30, end of session 3): bar WORKS live (curved, per-VFX color, drains on hits: S1 confirmed). Installed = probe4 (`1.1.0-probe4`, seam-free square-health/shield join, NOT yet run), leader toggle buttons shifted (deployed). OPEN: droid-shield blackout attribution (stock-set test), companion small-card bars/icons missing at times, Step 4 promotion to 1.1.0, and the user's request = a shield hover tooltip (see 'Session 3 hand-off + plan input' at the bottom). Older statuses follow.**

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

## Session 3b results (log `shield_bar_src/logs/shieldprobe_live3b.txt`, probe3)

- **S1 CONFIRMED live**: `ints[2]` of the type-0x02 sub-9 effect decrements in place (Peragus mining shield spell 132: 20 -> 17 -> 15 on hits). The effect object pointer is reused across shields (spell 99 -> 103 -> 101 seen on one address), so the max cache must reset on a new spell or when `remaining` goes UP (fixed in probe4).
- Resist Energy (spell 42) = type 0x02 sub 9 with `ints[2]=0` (no pool, correctly excluded); Fury-like temp HP not observed. Force powers (Valor 22, Speed 8, Resist Energy 42) have no absorb pool -> not on the bar. Only Fury/Rage temp HP is a candidate for a later version.
- **Blackout** attributed to the droid shield only (see above); Peragus (row 2045 on a human) was a false lead.
- **Seam-free style (user request)**: health bar squared-left + shield right edge open so both read as one track. Art from `scripts/gen_shield_bar_art.py`
  (`uibit_bar_sh_p`, `uibit_bar_vp_sq` in override/); the DLL swaps the PB_VIT borders' fill images (`slot+0x690+{0x6c,0xe4}`, fill ptr `+0x74`) to the square texture in the UpdatePortraits snapshot and restores them in the slot hook.
  Leader toggle buttons shifted left 16 units by `scripts/shift_hud_leader_buttons.py` (deployed 2026-09-30, backup `backups/hudshift_20260930_002155_539301`).


## Session 3 hand-off (2026-09-30) -- everything a fresh session needs

**Live state at the end of session 3 (game CLOSED; verify with `pgrep -af "^S:.*swkotor2"`):**
- Installed by hand (NOT via `KPatchLauncher`, which launches the game): `patches/buff-duration-hud.dll` = probe4 build (md5 `201fd563...`), `patch_config.toml` has the 6th hook (`0x745774` -> `ShieldProbeSlot`, params `ecx`, `[ebp+0xc]`, added by a python edit; a future KPatchLauncher run regenerates it from the `.kpatch`).
  `shieldprobe.txt` exists in the game dir (probe logging on -> `shieldprobe_log.txt`). New override files: `uibit_bar_sh_p.tga/.txi`, `uibit_bar_vp_sq.tga/.txi` (delete to roll back the art). Rollback backups: `~/Games/KOTOR2_Mod_Workspace/backups/shield_probe2_install_20260929_233138/`, `shield_probe3_install_*`, `shield_probe4_install_*` (previous DLL each), `hudshift_20260930_002155_539301` (`scripts/shift_hud_leader_buttons.py --rollback`).
  Stock rollback set: `kotor-patch-manager/inv3d_live_patches/` (never modify). Staging dirs: `shield_probe_stage2/3/4`.
- Invariants: `AR_ERROR.LOG` md5 `a2d887896b53eae113758dc8c3729db9`; `swkotor2.ini` `Mouse Sensitivity=44` (re-check after each session).
- Commits (branch KOTOR-II): 59369a2 probe2, 97c664b table+scripts, 9f8f99d probe3 DRAW log, zero-gap, session-3 docs, probe4 (seam art + health swap), 3b docs.

**Code map (`buff_duration_hud_src/BuffDurationHUD.cpp`, build copy in `kotor-patch-manager/Patches/BuffDurationHUD/`, build with `python3 ../create-patch.py` there):**
- `collectPools` (type 0x02 sub&8 effects; remaining=`ints[2]`; color key = sibling type-0x1e `ints[0]`; max cache keyed (creature, effect ptr) + spell, reset when spell changes or remaining rises), `shieldColor` (table `ShieldTable.inc` from `scripts/gen_shield_table.py`, fallback by flags),
  `DrawShieldPools` (texture `uibit_bar_sh_p`, rect `(vit.x - round(0.328*vit.w), vit.y, vit.w, vit.h)`, teal track then clipped tinted fills, stacked <=3 segments), `swapHealthBar` (swaps the PB_VIT borders' fill image ptr `slot+0x690+{0x6c,0xe4}+0x74` to `uibit_bar_vp_sq` in the UpdatePortraits snapshot hook, restores in the slot hook), `ShieldProbeSlot` (slot hook `0x00745774`; restores swap first, logs SLOT/DRAW lines, draws the bar; no longer gated by the probe flag).
- Art: `scripts/gen_shield_bar_art.py <vp_p.tga> <out>` -> `shield_bar_src/art/`. HUD: `scripts/shift_hud_leader_buttons.py` (shift 16 units; live `mipc28x6_p.gui` md5 was `a9d9aaed...` before, sha256 after `e0f42931...`).

**Facts learned in session 3:** item shields do NOT stack (a second activation removes the first's effects; seen with spells 99/103/101/132 -> always `pools=1`); Force powers Valor(22)/Speed(8)/Resist Energy(42) have no pool (Resist = type 0x02 sub 9 with `ints[2]=0`, excluded by `remaining>0`); Fury/Rage temp-HP type id NOT yet observed.
Blackout: only while the DROID's shield (spell 110) was active; human shields incl. Peragus mining (132, vfx 2045) never blacked out. Stock-set droid-shield test still pending. Companion small cards: bars/Buff-HUD icons drop out together (same slot->creature table); probe3 `DRAW slot= creature= pools= effects=` lines exist in `shieldprobe_live3b.txt` but were not yet analysed.

**Remaining work (original plan steps):** (a) verify probe4 live (seam look, swap flicker, companion cards); (b) blackout attribution; (c) Step 4: rename `ShieldProbeSlot` -> `DrawShieldBar`, trim probe-only logging, manifest `1.1.0`, `exports.def`/hooks.toml, `shield_live_patches/` rollback set, final docs (`## FINAL IMPLEMENTATION (v1.1.0)`, pointer in 02, README, memory); (d) hover tooltip (below).

## Hover tooltip request (user, 2026-09-30)
"On portrait hover it displays data (Meetra Surik / Vitality: 90/95 / Force: 88/88 / Level: 7 / XP Needed to Level: 1578). Could we display the active shield values too?" Fallback the user proposed: build our own hover just for the shield bar, the way the Force Power Hotbar (mod 3, doc 07 "Hover tooltip" sections) does it (label pair `LBL_HOTBARTIPBG` + `LBL_HOTBARTIP` in the `.gui`, DLL polls `GetCursorPos`, shows after 450 ms, sizes/places the panel).
Plan input: first choice = append shield lines to the native portrait tooltip (find the builder of the "Vitality:/Force:/Level:/XP Needed" text; look for those TLK strings / the GUI tooltip label update in the portrait hover code, hook and append "Shield: cur/max (type)"); second choice = own hotbar-style tooltip anchored to the shield bar rect (rect known: `vit.x-0.328w`..). Research must be Ghidra headless with the game CLOSED; query `patch_manager_mods/ghidra_knowledge/` first.

## Session 4: portrait hover tooltip research + Option A build (2026-09-30)

**Ghidra result (DB functions rows + lessons 125/126; scripts `FindPortraitTooltip/DecompileList/DisasmRanges/FindOffsetUsers.java`):**
- The GOG-delta leads were wrong (`0x744AA0/0x7449C0` are mid-function). The real chain: `FUN_00744b40` (char button OnEnter, fastcall this) -> **`FUN_00744c20` (text builder)** -> `FUN_00413e90` (GuiManager ShowToolTip, mgr `[0x00a1b49c]`, tooltip = `this+0x1d4`) -> `FUN_0075ff50` (panel size = text extents + 8, at cursor+15, screen-clamped: **it auto-sizes**).
- `FUN_00744c20` formats `"%s %s\n%s: %d/%d\n%s: %d/%d\n%s: %d\n%s: %d"` (name, Vitality cur/max, Force cur/max, Level, XP Needed; a no-Force variant when `D+0xc==0`) with the engine's cdecl **sprintf-into-CExoString `FUN_00734270(CExoString* dst, fmt, ...)`** (vsnprintf to a scratch buffer, then copy, so the old text can be passed back as `%s`). Text lives in the CExoString at `charButton+0x1d4`; `D = *(charButton+0x1d0)` = the portrait slot pointer (`SetSlot` = `0x744b20`; `slot+0xde0` = refresh flag).
- Built **on hover-enter and on the refresh flag** (`0x744b90` per-frame Update), NOT every frame: values are a snapshot at hover time (updates whenever the engine flags a refresh).
- strrefs: `0x1AD29`(109865) `0x7D9A` `0x424`/`0x425` are used here (my earlier "1AC29" was a typo; 74696 `0x123C8` only appears in `FUN_008f6490`, a different screen).
- **Hook (Option A)**: `0x00744EEC` = shared RET path of `FUN_00744c20` (`8b 4d f4 64 89 0d 00 00 00 00`, 10 B, 2 whole instructions, no relative branches), param `[ebp-0x48]` = this. No overlap with the 54 live hooks (checked with tomllib over the live `patch_config.toml`, 17 patches).
- Code: `describePools()` (reuses `collectPools`) + `AppendShieldToTooltip(charButton)` in `buff_duration_hud_src/BuffDurationHUD.cpp`; `exports.def` + `kotor2-steam-aspyr.hooks.toml` updated; logs `TIP btn= slot= creature= pools= line=` (probe flag). Built clean (`create-patch.py`), export present. **Line format**: `Shield: 32/40` (several pools: `Shield: 32/40 + 10/25`); nothing when no pool.
- Option B (own tooltip on the bar) NOT needed unless Option A fails live.
- **Status: built, NOT installed.** Phase 0 (probe4 live check) still owed by the user before this install.

### Session 4 live feedback (probe4 + tooltip, log `shieldprobe_log.txt` 01:03)
- Tooltip hook works live (275 `TIP` lines, `Shield: 40/40` logged for the leader/companions); user says the squared health edge looks better.
- Complaints: (1) shield bar overlaps / too wide / too far right; (2) no colour separation from the health bar, worst for red shields; (3) Buff HUD icons of the human party members are not shown on the portraits while the droid is the selected leader [ROOT CAUSE FOUND 2026-09-30: the engine's arrow counter counts a separate icon cache (`creature+0xFFC`), so cards with counter 0 never ran our column draw; fixed by the counter bump in `ForceArrowCounters`, see doc 02 "Engine arrow counter root cause"; live verification pending]; (4) black 3D scene for the leader's energy shield when other characters activate theirs (sometimes any switch, sometimes one specific character).
- Log analysis: slot->creature table is correct for every leader choice (companion slots get the 15/24-effect creatures with pools=1), so (3) is not a mapping bug; Buff HUD depends on the engine arrow counters `slot+0xddc/0xddd` (the `DRAW` line now logs them). `SWAP` lines were 19.5k of the 20k-line cap (removed).
- Fix built (not installed): shield art drawn at 0.55 of the health width with its visible right edge exactly on the health bar's visible left edge (`vx+0.445w`), fill colours lightened 40% toward white, 3 px (2 px small card) bright divider at the seam.
- Installed by hand 01:14 (DLL md5 3b9edeee, backup shield_bar2_install_20260930_011419); live config now 18 patches (sibling added equipment-3d-character at 01:00; our hooks intact).

### Session 4c live results (build 3b9edeee, log `shieldprobe_log.txt` 01:25)
- Width/slimmer look approved by the user.
- **Blackout refined (user)**: happens with the main character's Peragus mining shield (spell 132) AND the robot's shield (spell 110) active together; Meetra+Atton shields together were fine. Both are visualeffects row 2045 = RED_01 = `fx_tex_16` (mod #35, 4 MB 256x4096 TGA). Hypothesis: two simultaneous shells/sphere using the same 4 MB flipbook. Decisive test: move `override/fx_tex_16.tga` aside (vanilla texture) and repeat; if still black, try disabling `max-lights` / test the droid model path. Not our DLL (no draw of fx_tex from the GUI since probe 2).
- **Small-card shield bar missing**: `DRAW` lines say `pools=1` for every shielded slot in every leader arrangement, slot rects identical, `engineArrows buff` is a per-creature value that does not depend on the leader. Visually the bar is drawn on the leader card always, on at most one small card (Meetra on the lower slot with Atton leading), never otherwise, while the health-bar squaring (snapshot hook) works everywhere => the slot-hook draw fails silently for some small cards. Next build adds `FILL slot= pushClip= rect=` logging (change-detected) to see if `PushClipRect` refuses.

### Session 4d live results (FILL-logging build d65312df, vanilla fx_tex_16, log `shieldprobe_log.txt` 01:48)
- **Blackout persists with the vanilla `fx_tex_16`** (so NOT the mod #35 texture) and needs only the DROID shield plus another shield (Meetra + Atton + T3 this time; no Peragus). Combined with session 3 (droid shield alone blacked out), the droid shield itself is the trigger. Untested: stock set without our DLL (remove `buff-duration-hud` from the config and activate the droid shield) to prove it is not us; then suspect the droid sphere model / max-lights.
- **Small-card bar mystery solved (hypothesis fits all 3 screenshots + `DRAW` order)**: `pushClip` never fails (`FILL` pushClip=1 on all slots, pool values fine 40/40, 50/50), the bar shows on the leader always and on a small card ONLY when the previously-drawn slot drew no Buff HUD icon row (Atton leader `buff=0` -> Meetra on the lower slot OK; leader/lower with `buff=1` -> later slots invisible). So the Buff HUD icon-row draw (`DrawBuffColumn`/engine `PortraitIconRowDraw`) leaves the GUI clip/batch state dirty for the following slot's hook. Fix built: draw ALL shield bars from the leader's slot call (first of the frame, state clean) using the `g_slots` table. The same dirty state probably explains the Buff HUD icons vanishing on later cards (check next run).
- User request: show the shield bar in the menus (character/inventory screens) too: needs its own research (which screens show vit/force bars, draw site) -> next phase.

### Session 4g: contaminated test + blackout history
- A sibling session reinstalled at 01:57 (state UpdatedAt 01:57:46): live `buff-duration-hud.dll` reverted to `d65312df` (previous FILL build), `perf-lab` added (19 patches), hotbar/hang-probe DLLs changed. The user's latest test (black screen from Meetra's, then Atton's shield, humans only) ran the OLD build. DB lesson 127.
- Blackout is NOT droid-specific and NOT `fx_tex_16`. Needs a stock-set bisect (stock 16 = `inv3d_live_patches/`, with shields activated on 2 humans + droid); suspects: our shield code, `perf-lab`, `inventory-3d-viewport`, `equipment-3d-character`, `max-lights`.

### Session 4h: blackout diagnostics build (user: "only had this issue after adding the bars")
Opus review (scratchpad `blackout_investigation.md`): renderer = native OpenGL via Wine on an RTX 5070 with ReShade 6.8.0 as `opengl32.dll`, 4x MSAA; no DXVK. Suspects ranked: engine frame-buffer-effect path (0x4373a0), ReShade depth effects, Max Lights, our DLL (static review: clip push/pop balanced). The user's observation (never happened before the bars) makes OUR draw path the prime suspect: the bar only draws while a shield is active = exactly when the scene blacks out.
Built (not yet installed): live kill switches polled every 30 frames, no restart needed: `shield_nodraw.txt` (skip bar draws), `shield_noswap.txt` (skip health-bar swap), `buffhud_off.txt` (skip Buff HUD icon column); F9 = `MARK` + GL state dump; `SCENE avg= dark=` brightness sample (glReadPixels 5x5 grid, before ReShade) every 30 frames with a `GL DARK/BRIGHT` dump on change; `SHIELD_ANY` transitions dump `GL PRE/POST` around our draws for 4 frames; g_slots now evicts stale entries after HUD rebuilds. Test recipe: get the blackout, create ONE flag file in the game dir (nodraw first), watch if the scene returns within ~1 s; then remove it, try noswap, then buffhud_off.

### Session 4i: black bar when opening Settings (user report) + first diagnostics log
- We made NO change to show anything in menus. The 02:34 `shieldprobe_log.txt` shows `SCENE dark=1` states that toggle every ~0.5-2 s (the user flipping menus): in every dark sample the GL viewport is `(0,+-91,0,0)` (zero size), `clipDepth=65535` (engine GUI clip stack `[0xa309c0]` = 0xFFFF; bright frames read 0), depth test/lighting still enabled; and our leader-slot hook still runs in those frames (the GL DARK dump is taken inside it). With a shield active (SHIELD_ANY at f=31805) our bar draw therefore ran in menu frames with a broken clip/viewport state = candidate for the black bar.
- Fix built (not installed): the leader path returns before drawing unless `(*(int*)0xa309c0 & 0xffff) == 0`; logs `MENUSTATE skip=`.
- NOTE: dark=1 samples are ambiguous (menu frames give dark too); the real gameplay blackout was not among the F9 marks (none logged) at that time.

### Session 4j: all three shields fine; GL lighting leak found
- With the leader-call build (e9d8558f: all bars drawn from the leader's call) the user ran Meetra (99/132), Atton (99) and T3 (110) shields together and rotated the leader with NO blackout. (Earlier blackouts all ran per-slot-draw builds or the sibling-overwritten build.) Settings/exit menus were black at the end of that session (menu frames: clip stack 0xFFFF, viewport 0x0) -> user killed the game.
- `GL PRE/POST` dumps: our draws leave `GL_LIGHTING` enabled (PRE light=0, POST light=1 in 8 of 12 dumps); `glGetError 0x500` in PRE dumps is from our own probe queries (0x8CA6/0x8B8D on a legacy context), ignore.
- Fixes built + installed (md5 see memory): menu-frame guard (draw only when `[0xa309c0]&0xffff == 0`) and a `CapGuard` that snapshots the 9 GL enables (scissor, stencil, depth, lighting, blend, fog, tex2d, cull, alpha) before our draws and restores them after (logs `CAPFIX`).


## Session 4 summary + root cause (2026-09-30) -- READ THIS FIRST in a new session

**Black screen root cause (strong, not fully isolated):** our own GUI draw path. Evidence: the user never saw the blackout before the bars existed; it only happened while a shield was active (our bar only draws then); ruled out: mod #35 `fx_tex_16` texture (restored vanilla, still black), droid-only theory (humans alone blacked out later), perf-lab (blackout predates it). Two defects were fixed in this order and the blackout stopped:
1. **Per-slot draws in a dirty GUI state.** The shield bar used to be drawn from every slot's hook (`0x745774`), i.e. after earlier slots had drawn their Buff HUD icon rows; bars only showed on the leader (+ at most one small card) and the state left behind was broken. Now ALL bars are drawn once per frame from the LEADER's call (`isLarge != 0`, the first slot call of the frame, GUI state clean) using the `g_slots` table (`slot+0x694..` rect per slot).
2. **Leaked GL state + menu frames.** `GL PRE/POST` dumps showed our draws leave `GL_LIGHTING` enabled (engine colour-fill clip / batch flush toggle it). `CapGuard` now snapshots 9 GL enables (scissor, stencil, depth, lighting, blend, fog, tex2d, cull, alpha) before our draws and restores them after (`CAPFIX` log if it had to correct one). Also in menu frames (Settings/Exit) the engine GUI clip stack `[0xa309c0]` reads `0xFFFF` and the GL viewport is 0x0 while our HUD hook still runs: drawing there blacked out the settings/exit menu, so the leader path now returns unless `([0xa309c0] & 0xffff) == 0` (`MENUSTATE` log).
Which of (1)/(2) fixed the scene blackout is unknown (the runs between were contaminated by a sibling install; build e9d8558f with only (1) already ran 3 shields incl. T3 110 + Peragus 132 with no blackout, but then had the black menu). Do not remove either.

**Tooltip (Option A) FINAL:** see "Session 4: portrait hover tooltip research + Option A build". Hook `0x00744EEC`, param `[ebp-0x48]`; text built once per hover-enter/refresh flag (`slot+0xde0`); values are a hover-time snapshot.

**Diagnostics kept in the DLL (all behind `shieldprobe.txt` unless noted):** `SLOT/DRAW/POOL/FILL/TIP/SCENE/GL/MARK/MENUSTATE/CAPFIX/SHIELD_ANY/HB` log lines (`shieldprobe_log.txt`, cap 20000 lines); kill-switch flag files in the game dir, polled every 30 frames, always on: `shield_nodraw.txt` (skip bar draws), `shield_noswap.txt` (skip health-bar swap), `buffhud_off.txt` (skip Buff HUD icon column); F9 = `MARK` + full GL dump; `SCENE dark=` = 5x5 glReadPixels brightness (dark also in menu frames!). Opus investigation report: renderer = native OpenGL via Wine on an RTX 5070, ReShade 6.8.0 is the game's `opengl32.dll`, 4x MSAA, no DXVK (report: `shield_bar_src/blackout_investigation.md`).

**Hazards learned (DB lessons 125-127, 141+):** sibling Claude sessions reinstall the patch set at any time (01:00, 01:15, 01:57, 02:13 today) and revert/override our DLL: ALWAYS re-md5 `patches/buff-duration-hud.dll` against the intended build and read `kpm_install_state.json` UpdatedAt before telling the user to test; install by hand (lesson 124) touching only `patches/buff-duration-hud.dll` + our hooks in `patch_config.toml`. The live config now holds 19 patches (perf-lab, equipment-3d-character added by siblings); our patch has 7 hooks (`0x74F25A`, `0x74F8AB`, `0x745A84`, `0x745A97`, `0x745AEC`, `0x745774` ShieldProbeSlot, `0x744EEC` AppendShieldToTooltip).

**Other facts from session 4 (logs):** item shields never stack (2nd activation removes the 1st, `pools=1` always); Meetra = creature with `FFFFFFFD` id; engine arrow counters `slot+0xddc/0xddd` (`buff`/`debuff` in `DRAW`) are per creature, not per leader; small-card tooltips: `TIP` fires once per hover/refresh; `pushClip` never fails; `glGetError 0x500` in PRE dumps = our own probe queries (ignore). Stock-set/no-DLL blackout bisect was never needed.

## NEXT PHASE (plan): colour polish + UX interaction with existing elements
See the Claude plan file written at the end of session 4 (also summarised in `README.md` entry 08). Items: (A) colour/lighten/divider design incl. red shield vs red health, per-row table review, empty-track colour; (B) interaction with existing elements (Buff HUD icon columns, Force Power Hotbar, leader toggle buttons, droid Force-less cards, tooltips, mirror/4th slot, cutscene/letterbox/dialog/conversation/map/menu screens, party switching, area loads, resolution/scale); (C) tooltip polish (hover refresh, multi-pool wording, colours); (D) Buff HUD icons with a robot leader; (E) Phase 3 finalisation (rename, trim, 1.1.0, rollback set); (F) shield bar on menu screens (research with Ghidra, game closed).

## Buff HUD counter bump (2026-09-30)
Complaint (3) (companion icons missing) is addressed by `ForceArrowCounters` (called at the top of `DrawShieldBar`); the DRAW probe line now also prints `raw=(b,d) tracked=(b,d)`, and new `ARROWS`/`ICONCACHE` probe lines exist. Details, status and test matrix: [[02_buff_duration_hud.md]] "Engine arrow counter root cause (2026-09-30)". Mark complaint (3) resolved once the live test confirms raw 0 -> bumped 1 on the formerly silent creature.


## Session 5/6 summary (2026-10-01/02)
**Look (user-approved):** slim bar `ShieldWidthFraction=0.55` of the health rect, fill = HSV-desaturated shield hue (`ShieldSatScale 0.5`, `ShieldMinSat 0.30`), empty part = engine teal `TrackTint` (user chose neutral teal over a dark shield hue), NO seam/divider/rim (`DrawSeam=false`), health bar keeps its stock curved left highlight (`SquareHealthEdge=false`, texture swap off), corner wedge art `uibit_bar_sc_p` (drawn on the health rect, fills the card-frame notch, top half/bottom half coloured like the shield's top/bottom edge), `uibit_bar_sh_p` re-sampled (`scripts/gen_shield_bar_art.py`) so the left outline renders at the health bar's stroke thickness and its body extends to the texture edge; the DLL clips every shield draw at the seam (`pushClip(x,y,V,H)`, V = integer visible width) so the right edge is pixel-exact; small cards (vw<70) get a 1 px top/bottom inset (optical illusion). Buff HUD: drain line only on the round item icons; icons also on the menu top-bar portrait and the two menu companion portraits.
**Engine facts (DB lessons, 2026-10-01):** menu top bar = `top_p.gui` = `CSWGuiInGameMenu` (ctor `0x754ed0`, vtable `0x99e454`, slot 14 = Draw `0x756b70`); that Draw runs `CSWGuiPanel::Draw 0x40ec90` (BeginGui2D `0x4792c0`, push panel rect via `0x410020`, controls, pop, EndGui2D `0x4793f0`) and THEN calls `0x745770(dt,1)` with ECX = portrait slot `menu+0x2350` = our hook `0x745774` (3rd hook param `[ebp]` = caller ebp; creature = `*(callerEbp-0x38)` = `FUN_0077d800(handle)`; `[ebp-0x28]` is only the handle). At that point the 2D batch is CLOSED (`[0xa309c0]=0xFFFF`, ortho popped, viewport garbage): `DrawMenuOverlay` reopens it (BeginGui2D + pushClip(panel rect) + panel-local draws + popClip + EndGui2D). The clip stack IS the glViewport stack (10-byte entries at `0xa309c8`). Menu companions: `LBL_CHAR2` = `menu+0x3134`, `LBL_CHAR3` = `menu+0x36DC` (portraits of roster members 1,2 via `FUN_0073fb90`/`FUN_007e5da0`/`FUN_0077d800`); control rect ints at `+4/+8/+0xc/+0x10`, visible = `+0x48 & 2`. HP/FP source for the HUD bars: creature vtable `+0x98(1,[c+0x10f4])` max HP, `+0x9c(0,max)` cur HP, FP = stats(`c+0x1198`) shorts `+0x130`+`+0x132`, max = `FUN_0057eca0(c)`.
**Perf:** Buff HUD `readAt` migrated from `IsBadReadPtr` (Wine page-fault + SEH storm, doc 10) to `_shared/SafeRead.h`; `beginScope()` at every hook; slots stale after 500 ms ignored (`slotFresh`).
**Shared file hazard:** `BuffDurationHUD.cpp` is also edited by a Buff HUD review session (arrow-counter bump `ForceArrowCounters`, `RestoreBuffIcon` ordering): check `git status`/`git diff` and merge; commit only your own hunks.
**Open items (research plan = the Claude plan file written at the end of session 6; summary in memory `kotor2-shield-bar`):** (1) install + live-test the staged menu-companion-bars build and tune spacing/placement; (2) green-match check with a half-drained shield (screenshots needed); (3) reported: active character's head gear missing in-game after opening/closing a menu (likely `equipment-3d-character`, unverified); (4) shield bar hover tooltip (item F: type, remaining, time left); (5) tooltip refresh while hovering (B7a); (6) coexistence audit B (hotbar, droid cards, letterbox/cutscene/map/load states, resolution); (7) Buff HUD with robot leader (C) re-verify with the sibling counter bump; (8) Phase 3: rollback set `shield_live_patches/`, doc FINAL, README, memory; (9) Mouse Sensitivity now 17 (was 44; probably user-changed).

## P1 result table (static Ghidra, 2026-10-02): which menu screens run the top-bar Draw 0x756b70

Key finding: CSWGuiInGameMenu (top_p, vtable 0x99e454) is ONE singleton (CGuiInGame ctor 0x7be4c0 stores it at gui+8; the only caller of ctor 0x754ed0 is 0x7bef23; no subclass; no screen embeds or loads its own copy). It is added to the GuiManager panel list only by OpenInGameMenu 0x7c9df0 (AddPanel at 0x7c9f6d) and stays there while tabs are switched (0x7ca550 swaps only the tab panel). So the slot is always menu+0x2350 and the hook 0x745774 fires every frame for whichever tab is showing. The Draw is vtable slot 13 counted 0-based from the vftable (vtable+0x34, the same slot GuiManager::Draw 0x4113a0 calls); older notes' "slot 14" is 1-based.

| Screen (class, ctor) | Own panel / derives top bar? | Draw override | Top-bar hook fires? | Slot | Creature local |
|---|---|---|---|---|---|
| equip_p (InGameEquip 0x8a92d0) tab 0 | separate panel; top bar = singleton | slot13 0x8aba50 (own tick, not the menu) | yes | menu+0x2350 | caller(756b70) ebp-0x38 |
| inventory_p (0x8a6170) tab 1 | separate | slot13 0x8a7350 | yes | same | same |
| character_p (0x84c3a0) tab 2 | separate | slot13 0x84fbb0 | yes | same | same |
| abilities_p (0x8a25c0) tab 3 | separate | base 0x40ec90 | yes | same | same |
| partyselect_p (0x89cf30) tab 4 (factory) | separate | base | yes (as tab) | same | same |
| journal_p (0x7fae60) tab 5 | separate (embeds messages_p 0x757c40) | slot13 0x7fbce0 | yes | same | same |
| map_p (0x893950) tab 6 | separate | slot13 0x894cd0 | yes | same | same |
| optionsingame_p (0x8a1170) tab 7 | separate | base | yes | same | same |
| leveluppnl_p / pwrlvlup_p (LevelUpCharGen 0x8f4af0, modal layer 3) | no | base | only if the menu is open beneath (Character tab); drawn in list loop 1 UNDER the modal panel (not visible); none if launched by script | same | same |
| upgrade / upgradeselect (0x8c9e10/0x8c6e90) | no | base | only if opened from the inventory tab, hidden beneath | same | same |
| store_p (0x8b4270), container_p (0x8b1ea0), standalone partyselect (0x7cb920) | no, modal layer 3, store requires menu closed | base | no | n/a | n/a |

Creature locals in 0x756b70 (real ebp offsets; decompiler local_X = ebp-(X-4)): [ebp-0x28] = client CSWCCreature* (GetMember 0x7e5da0, or 0x73f550 when menu+0x3dd0 != -1), [ebp-0x38] = 0x77d800 result (server creature). Both are set before the call at 0x757304 on every path; Draw returns early (no hook) if the client creature resolves to null. Open doubts: tab 4 factory case builds partyselect_p although the 8th tab button is BTN_MSG (messages_p); not resolved, bar is present either way. Menu object layering: menu draws in GuiManager list loop 1 before the tab panel; modal (layer 3) panels draw on top. DB: lessons 293-297, functions rows 7be4c0/7c9df0/7ca060/7ca550/7d0760/756b70/410530/4113a0/84fcd0/7ccd40/7cb920; new scripts ListGuiVtablesSlots.java, CallSiteContextBefore.java.
