# RFC: Target Effects Strip — Awareness-Gated Status Icons Above the Target Banner (currently `enemy-effects-strip`)

**Type:** RFC / Feature Proposal  
**Target Component:** `Patches/EnemyEffectsStrip` (proposed rename: `TargetEffectsStrip`), `Patches/HudHost`, `Patches/ModConfig`, `Patches/Common`  
**Current Status:** `0.4.2-dev` (the version played below was `0.4.0-dev`). **Played 2026-10-18** with the test override `ForceTier=3`: Fear on an enemy (spell 16, effect type 8, state 8) shows the `ip_fear` icon above the target name and removes it on expiry; with Kreia as party leader the icon showed, and it stayed visible after switching to another party member. Other icons (a Gamorrean boss and Kreia's own buff row, five icons) also drew. **Not yet played:** the Awareness tiers without the override, the backing pill, and the Mod Configuration settings page with its live preview. The pure logic and the draw path have host tests (the draw test runs the real draw code against a recorded fake engine: 390 checks, 0 failed; the logic suite has 1001).  
**Platform:** Steam/Aspyr only for this patch. The provider it uses (`hud-host`) also ships GOG hooks, but GOG support is static-verified only and needs testing by a GOG contributor.  
**Related Discussion:** Builds on the HUD Multiplexer Provider (`hud-host`, see the Provider patches RFC (#335)) and the effect icons from the Buff HUD; the settings page uses the Mod Configuration menu RFC. Sibling of the Active Effects Panel and Combat Action Hotbar RFCs.

---

### Naming Note (feedback wanted)

The mod is registered as **`enemy-effects-strip`**, but nothing in it checks hostility. It draws for whatever creature the vanilla target banner (name + health bar) is showing, so it also appears for allies and neutrals. A name that matches the behaviour would be **`target-effects-strip`** (alternatives: `target-status-strip`, `target-effects-hud`). Because a rename means a new mod id, I would rather settle it before this goes upstream. Everything below uses the proposed name; the current id and file names are `enemy-effects-strip` / `enemy_strip.ini`.

---

### Motivation & Background

In vanilla KotOR 2, the player can see their own party's effects (the character sheet, and my Buff HUD adds an on-screen version), but **nothing shows what is affecting the creature you have targeted**. Did the Fear land? Is the droid still stunned? Has the Sith shield dropped? The player can only tell by watching the animation.

The **Target Effects Strip** adds a single row of small icons **above the target's name and vitality bar**:

* Same icons the character screen and Buff HUD already use (`ip_*` art), so players recognise them.
* A draining overlay shows remaining duration (no numbers, no text, no tooltip).
* One row, up to 5 icons (configurable); harmful effects first, then helpful ones.
* Nothing is drawn unless the target actually has a qualifying effect.

It is deliberately **low-information and lore-friendly**: what you can see depends on how observant your party leader is (see Awareness gating below), so it never becomes a free "enemy status readout".

---

<!-- SCREENSHOT PLACEHOLDER 1 -->
<!-- ![Target Effects Strip above an enemy](screenshots/target_effects_strip_fear.png) -->
> *Screenshot Placeholder 1: Enemy targeted after a Force Fear: the `ip_fear` icon sits centred above the target's name and health bar, with the duration overlay draining from the top.*

---

### Awareness Gating (Design)

The party leader's **Awareness** skill rank decides how much is shown. The thresholds are INI values (`TierObvious`, `TierOwn`, `TierFull`) and the defaults are placeholders to be tuned in play:

| Leader Awareness | Shown |
|---|---|
| below 5 (default) | Nothing: vanilla behaviour, nothing drawn |
| 5 or more | Only *obvious* conditions: the visible incapacitating states (stun, droid stun, stasis, sleep, fear/horror, and states 1, 2 and 16, which the source leaves unnamed) plus entangle, knockdown and slow. Icon only |
| 12 or more | Plus debuffs applied by a party member |
| 20 or more | Plus the target's buffs (shields, Valor, and similar) and the draining duration overlay |

The tier order is enforced when settings are loaded (`Obvious <= Own <= Full`; a later tier below an earlier one is raised to match). A `ForceTier` setting overrides the rank for testing.

---

### What Didn't Work: Dead Ends & Engineering Lessons

#### 1. The Probe Read the Wrong Creature
* The first probe printed an empty or garbage effect list for every target. Static analysis showed why: the target the HUD hands to a mod is the **client** creature, but the effect list (`+0x148/+0x14C`), the icon cache (`+0xFFC`) and the stats (`+0x1198`) live on the **server** creature. Earlier project notes saying these were "client-side" were wrong.
* **The Fix:** resolve the server creature from the client creature with the engine's own client-to-server lookup (the same call the party code uses), check the object is readable, and read from that. Awareness is read through `stats+0x174`, which is a *pointer* to a signed int8 array of **base** ranks (index 3 = `skills.2da` row 3, Awareness, confirmed from the game's script constants: `nwscript.nss` has `SKILL_AWARENESS = 3`), not an inline field.
* **Result in game:** Fear on an enemy logs as effect type 8, state 8, spell 16, and the correct icon shows. In the same session the leader's Awareness read as 5, 11 and 12 for different leaders (not yet compared against the character sheet).
* **Open:** some targets (13 of 14 in one session had target type 2, one type 0; 8 distinct objects) never resolve to a server creature. The strip hides itself for them, but I have not identified what those objects are, or whether any are creatures that should show effects.

#### 2. No New Engine Hook Was Needed — But the Target Is Not Where You'd Expect
* The target name and health bar are not a floating label. They are controls inside the target-action menu panel, drawn in a viewport the engine moves to the target's projected screen position every frame. The strip therefore needs **no hook of its own**: it is a `hud-host` consumer that creates a small pool of labels when the HUD is built and repositions them from the target-menu tail callback each frame. A mod that only consumes `hud-host` can be hot-swapped, with no install-order trouble.
* The labels have to be children of the HUD panel in absolute display pixels (the menu itself clips and walks no child list), so the row is placed from the banner's origin: `menuY + nameLocalY - gap - iconH` (the gap is a setting, default 4 px; the icon size defaults to the height of the name label).

#### 3. A "Dark Rectangle" Texture That Wasn't Shipped
* The duration overlay needs a solid dark rectangle. I first planned to tint the existing circular Buff HUD art black, which gives a disc, not a rectangle.
* **The Fix:** the vanilla texture `uibit_fill_2bt` is already a solid black fill (alpha about 0.78 baked in), so no file ships. The overlay's alpha is divided by that baked value to hit the configured opacity.
* **Follow-up from play:** with the overlay on at the default strength, the dark drain shading read as a dark box behind the icons. Drain shading is now **off by default** (`DrainAlpha=0`) and stays a setting.

#### 4. Which Effects Count as "Obvious"?
* Spell ids are not a reliable gate (they are `-1` for item-property, innate and trap sources). The strip gates on the **effect type and state** instead: Fear, Horror and Insanity all apply the "Horrified" state (state 8), stun is state 4 (droid stun 3), stasis 5, sleep 6; states 1, 2 and 16 are also treated as obvious, but the source gives them no name; entangle, knockdown and slow have their own effect types. The adhesive grenade is an Entangle, not a slow; the sonic grenade is a Dexterity decrease with no state, so it is never an "obvious" icon (checked in the vanilla grenade script, not yet seen in a live log). Today every movement-speed-decrease effect counts as obvious, which is broader than "slow" alone.
* A fixed list of effect types is never shown as an icon at all: nine bookkeeping types, the visual-sibling type, and the shield-pool absorption effect. Script-applied "effect icon" effects (the type that carries Fear/Horror/Insanity icons) are not excluded: they can draw as their own icon when the harmful row has one.

#### 5. "Restart the Game to Apply" Is Not Acceptable for Settings
* The first version built exactly `MaxIcons` labels per HUD (and none at all if the mod started disabled), so those two settings needed a game restart. For a settings menu that is a bad experience, so the label pool is now **always built at full size** (8 icon slots, hidden) and `MaxIcons` only caps how many are used. Every setting, including `Enabled` and `MaxIcons`, applies the moment you press Save. The cost is 9 extra hidden labels per HUD build.
* **Edits are staged.** The page changes a pending copy; the real HUD does not change until **Save**. **Reset** goes back to the saved values, **Defaults** loads the default values (then Save), and leaving the page drops unsaved edits. Save writes only the keys that changed, one at a time, so a failed write leaves that key marked unsaved instead of half-applied.
* Config values are clamped and validated by one pure function shared by the INI loader and the settings page, and a trailing-junk number such as `0.65abc` falls back to the default instead of being half-read.

#### 6. A Settings Page Is Not Just Buttons
* The two opacity rows started as `<` / `>` steppers. They are now **sliders** built from a native button (the track, so it keeps the bevel, hover and click sound) with a fill bar and a marker on top. The mod polls the left button: a press over the track starts a drag, the cursor position sets the value on a 0.05 grid, release ends it. The engine's own slider class needs a `.gui` layout, so this is the self-contained version.
* The example strip is drawn **zoomed** (up to 6x) because real icons are about 14 px on a large display and unreadable in a settings page.
* The example labels are clones of the Options title label, so they inherited its text ("OPTIONS") and drew it over the icons; the text is now blanked on every clone.

---

### Key Features & Design

1. **Placement:** one row of small icons centred above the target's name and health bar; icon size, gap, spacing, and an optional translucent backing pill are all settings.
2. **Icons:** reuses the shared spell-to-icon tables from the character screen and Buff HUD (harmful variant first), including the engine's "effect icon" rows (Fear, Horror, Insanity and so on).
3. **Duration overlay:** a clipped dark overlay shrinking with the remaining time, available at the highest Awareness tier; off by default.
4. **In-game settings page:** under *Options → Mod Configuration → (strip)*, using the shared Mod Configuration provider. Toggle, stepper and slider rows, **Save / Reset / Defaults** buttons (edits are staged until Save, and everything applies without a restart), and a **live example of the strip** in the top part of the right-hand pane, redrawn from your pending edits every frame, with the description in the bottom part. The provider is optional: without it the mod just has no page.
5. **Quiet logs:** with `Verbose=0` only target transitions are logged. `Verbose=1` also logs the effect list on a *change* of the list, never per frame. A log analyzer (`kotor2_logs.py check enemy-effects-strip_log.txt`) summarises build state, strip transitions, spell ids seen per effect type and the creator form.

---

<!-- SCREENSHOT PLACEHOLDER 2 -->
<!-- ![Settings page with live preview](screenshots/target_effects_strip_settings.png) -->
> *Screenshot Placeholder 2: Mod Configuration page for the strip: setting rows on the left; on the right, a mock target banner with five sample icons on top and the description below.*

---

### Technical Architecture

* **No hooks of its own.** Requires `hud-host` (`k2.hud@1`); optionally connects at run time to `k2.modconfig@1` and `k2.menuui@1`.
* **Build (`K2HUD_BUILD`, while the HUD layout is open):** clone a full pool (8 slots of icon, disc and drain overlay, plus an optional pill) from a vanilla label template. All start hidden and click-through; `MaxIcons` only caps how many are used.
* **Per frame (`K2HUD_TARGET_MENU_POST`):** read the target, resolve the server creature, read the effect list and the leader's Awareness, pick icons by tier, and write label rects, visibility and fills **only when a value changed**. A stale-HUD check drops pointers if the HUD was rebuilt, and writes nothing to the old labels.
* **Engine data used:** target and panel geometry in the target-action menu (origin, name label, health bar), the server effect list (`type`, `duration`, `creator`, `spell id`), the Awareness rank (`skills.2da` row 3, **base** rank only, so item and buff bonuses do not count), and the effect-icon table.
* **Testing:** `EnemyStripLogic.h` holds all decisions (tiers, obvious-state set, sorting, capping, layout, config clamps) with host tests (1001 checks), plus a fake-engine test of the real draw code (390 checks: build order, hide-on-no-target, change-only writes, stale HUD, missing image).

---

### Known Limitations & Unproven Points

* **Party-debuff tier (Awareness 12+) is unproven.** It needs the effect's *creator* to be matched against party members, and the creator's stored form (object id or pointer) is not yet established: in the played session no effect's creator matched the leader even though the leader cast the Fear. All play so far used `ForceTier=3`, which bypasses this.
* **Awareness tiers without the override, the backing pill, and the settings page** are untested in game.
* **Unresolved targets:** see the open point under dead end 1.
* **Trap-sourced effects** are assumed to have spell id `-1` like item and innate effects; only item and innate were counted in logs.
* **Display:** only 3840×1600 has been checked.
* Steam/Aspyr build. Placement was checked on a 3840×1600 display; other aspect ratios are untested.

---

### Community Feedback & Discussion

1. **Name:** `target-effects-strip` (matches behaviour, since it also shows for allies and neutrals) or keep `enemy-effects-strip`?
2. **Allies:** Should the strip appear for allies and neutrals at all, or only for hostile targets? If it does, should Awareness gating apply to allies, whose effects you can plainly see, or should allies always show everything?
3. **Awareness thresholds:** Do 5 / 12 / 20 feel right, or should the mod scale with character level, or ignore Awareness and use a different gate (for example, Force Sight)?
4. **Information level:** Should the strip be allowed to become a full buff and debuff readout (a settings option), or stay deliberately limited?
5. **Overlap with the Combat Hotbar:** the strip sits above the banner and the hotbar below it. Is that the right split, or should the player be able to choose where each goes?
