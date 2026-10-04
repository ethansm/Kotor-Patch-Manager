# Level-Up Override 1.0.0

Target: KOTOR II Steam (Aspyr) build, sha256 `6A522E71...`, with Kotor Patch Manager. Steam only.

## What it does

When you press Auto on a companion's level-up screen (or the game auto-levels a companion), this patch makes the Auto
choice follow a build preset you pick for that companion: feats, Force powers, skills, attributes. It can also hold
("bank") a companion's levels until they become a Jedi, and it can make level-ups instant. All of this is per companion
and per save. A companion you have not configured levels exactly as in vanilla. The player character is never touched.
If you pick feats and powers by hand, your picks are kept.

## Install

1. Install the `.kpatch` (`LevelUpOverride.kpatch`, id `level-up-override`) with Kotor Patch Manager.
2. Put `levelup_paths.ini` (the preset table) in the game folder, next to `swkotor2.exe`. The patch reads it from there.
   Without the table the Configure button does nothing useful and no companion can be configured.
3. Start the game. `levelup_override.ini` is optional: a missing file means `mode=rewrite`, which changes nothing
   until you configure a companion.

## Quick start

1. In game, open the character screen of a companion.
2. Press **Configure**. (If the companion can level up and is not configured yet, the button reads **Plan level-ups**
   and is highlighted. Once configured it reads **Change path**.)
3. Pick a preset with the option rows, or press **Use recommended**.
4. Press **Apply**. The page closes. A status line on the character screen shows the active path.

A companion is only configurable when the table has presets for it. The player character never shows the button.

## The options page

| Control | What it does |
|---|---|
| Option rows | Jedi (when the companion converts), Role, Weapons, Powers (light/dark/neutral), Skills. Click a row to cycle its value. A row appears only if the companion's presets differ in it. Changing a row picks the closest matching preset. |
| Variant | Shown only when several presets match all the rows above. Click to cycle between them. |
| Build line | The short name of the current preset. |
| Instant: On/Off | Level-ups for this companion happen at once, without the level-up screen. Off by default. |
| Bank for Jedi / Pause at level N | Controls holding levels for the Jedi class. See below. |
| Use recommended | Switches to the recommended preset for this companion. |
| Apply | Saves the choice for the current playthrough. |
| Vanilla | Removes the companion's configuration. Shown only when the companion is configured. |
| Cancel | Closes the page without saving. |

Status line on the character screen:

- `Auto path: <short name>` (plus ` (instant)`) when configured.
- `Holding N level(s) for <Jedi class> - <short name>` when levels are banked. N is the number of levels banked
  from the XP held.

When a level is banked, a message `<Name>: level banked - waiting for <Jedi class> (level-up path)` appears in the
game's message list. It shows once per banked count and at most every 5 seconds.

## Recommended builds

These are the presets marked `recommended = true`. **Use recommended** selects them.

| Companion | Preset | Short description | Jedi class at |
|---|---|---|---|
| Atton Rand | `atton__melee_dual_light_early` | Dual sabers, light side, Jedi at 8 | Sentinel, level 8 |
| Mira | `mira__ranged_sentinel` | Pistol Scout, light Sentinel at 17 | Sentinel, level 17 |
| Bao-Dur | `baodur__early_guardian` | Heavy-armor saber tank, light, Jedi at 12 | Guardian, level 12 |
| Handmaiden | `handmaiden__unarmed_jedi` | Unarmed Guardian, light side, Jedi at 7 | Guardian, level 7 |
| Disciple | `disciple__caster` | Light Force healer and buffer, Consular at 7 | Consular, level 7 |
| Kreia | `kreia__support_none_neutral` | Neutral buffer, universal powers only | none |
| Visas Marr | `visas__duelist` | Single saber duelist, light buffs | none |
| Mandalore | `mand__ranged_rifle_none_na` | Blaster rifle / repeater Soldier | none |
| Hanharr | `hanharr__melee_single_none_na` | Two-handed melee Scout | none |
| HK-47 | `hk47__ranged_rifle_none_na` | Blaster rifle combat droid | none |
| T3-M4 | `t3m4__support_single_none_na` | Utility droid: workbench, spikes, repair | none |
| G0-T0 | `g0t0__support_pistols_none_na` | Tech utility droid with dual blasters | none |

Other presets exist for each companion (other weapons, dark side, convert late or never). Use the option rows to
find them.

## Pause at level N

Some presets make a companion a Jedi at a set character level (for example Atton at 8). The patch plans the
companion's levels around that. The Bank control decides what happens between the companion's current level and the
conversion.

For presets with a Jedi class, the control cycles through these states each click:

1. `Pause at level <convert level - 1>`: the default. The companion levels on the base class up to the level before
   conversion. Levels after that are banked until the Jedi class exists.
2. `Pause at level` lower values, one step per click.
3. `Bank for Jedi: Off`: no banking. The companion keeps levelling on the base class.
4. Back to the default.

"Pause at level N" means: N is the last character level the companion may take before banking. Every level above N is
held (XP still counts) until the companion has the Jedi class. After conversion the levels are taken normally.

The lowest N you can choose (minPause) is the preset's first planned character level minus 1 (at least 1). Atton's presets
start planning at character level 4, so his lowest pause is 3. The highest is the conversion level minus 2.

**Bank everything until Jedi** (for example Atton, to get the most Jedi levels): press the Bank control until it shows
the lowest value (for Atton, `Pause at level 3`), then press **Apply**. Every level after his current one is banked
until he becomes a Jedi (through the story, mid-game). If you would rather keep him a Scoundrel longer, leave
the default or turn banking off.

**Clamp:** when you change a row or press Use recommended and the new preset has a different pause range, the pause
resets to the preset's default (0 in the ini). The value you typed is not remembered.

Presets with no Jedi class (Kreia, HK-47 and so on) have no pause range. Their control just shows
`Bank for Jedi: On` or `Off`.

**Early conversion keeps legal picks (carry-over).** When the companion becomes a Jedi earlier than the preset
planned (a lower Pause), the Jedi record is applied at a lower character level than it was designed for, and the base-class
picks in between were skipped. The mod then reads what the creature already knows and fills each feat/power slot with a
legal pick: the Jedi record's own planned picks keep their slots; a skipped earlier pick is taken only when a planned pick is
illegal because a prerequisite is missing (the prerequisite is taken first, the planned pick follows) or when no planned pick
is legal for the slot. A slot with no legal pick keeps the engine's own pick. A level taken on time is unchanged. This needs the
table format v3 (`[featrules]`/`[powerrules]`, shipped with the DLL); with an older table carry-over is off (one log line).

## Instant levels and banked levels

- **Instant** (opt-in, off by default): the companion levels at once with the Auto path, with no level-up screen. It
  also works for the companion you are currently controlling (the party leader).
- **Banked:** the level-up indicator on the portrait is hidden while levels are held, and Level Up / Auto on the
  character screen are blocked for that companion. The XP still counts. The status line tells you what is held.
- A companion that is banked is not instant. Banking comes first, and instant resumes after the Jedi class exists.

## Per-save configuration

Your choices belong to the playthrough, not to the game install. The first time you press Apply in a save, the patch
creates a playthrough id (a 32-bit number) and stores it inside the save (32 boolean globals named `LUO_G00` to
`LUO_G31`). Your companion settings are kept in `levelup_playthroughs.ini` under a section named after that id.

- Loading a save finds its id and uses its settings.
- A **new game** has no id, so every companion is unconfigured (vanilla) until you configure it.
- A save made before you ever pressed Apply has no id and is unconfigured.
- Two saves from the same playthrough share the id and the settings.

## The player character

The patch never changes the player character's level-ups. The Configure button does not appear for the PC.

## Manual picks

The patch only rewrites the **Auto** level-up (the Auto button, Auto Level Up NPCs, and the game's own auto-levelling
of companions). If you open the level-up screen and choose by hand, your picks are applied as you made them.

## Configuration reference

### `levelup_override.ini` (game folder, optional)

The file is re-read when it changes. Missing file or missing key means the default.

| Key | Default | Meaning |
|---|---|---|
| `mode` | `rewrite` | `rewrite`: apply configured paths, holds and instant. `log`: write the log and dry-run only, change nothing. `off`: patch inert. |
| `apply` | `1` | Second kill switch. With `mode=rewrite` and `apply=0` the rewrite is a dry run (`WOULD REWRITE` in the log). |
| `verbose` | `0` | `1` adds detailed per-call dumps to the log. |
| `table` | `levelup_paths.ini` | File name of the preset table. |
| `dev` | `0` | `1` enables the developer keys below. They are ignored otherwise. |

Developer keys (only with `dev=1`; the options page is the normal way to configure):

| Key | Default | Meaning |
|---|---|---|
| `preset.<tag>=<preset id>` | none | Configure companion `<tag>` with a preset. |
| `instant.<tag>=0\|1` | `0` | Instant levels. |
| `hold.<tag>=0\|1` | `1` | Bank for the Jedi class. |
| `pause.<tag>=N` | `0` | Pause at level N (0 = preset default). |
| `tags=atton,kreia` | none | Legacy (0.2.1): each tag without a preset key gets preset `<tag>__live`, instant 0, hold 0. |

Precedence: if the current save's playthrough id has a section in `levelup_playthroughs.ini`, only that section
applies (a companion not listed there is unconfigured, even if a dev key names it). Otherwise the dev keys apply.

### `levelup_playthroughs.ini` (game folder, written by the patch)

```
[g11b3b802]
atton=atton__melee_dual_light_early|instant=0|hold=1|pause=3
```

- Section name: `g` plus 8 lowercase hex digits (the playthrough id).
- Line: `<companion tag>=<preset id>|instant=0|1|hold=0|1|pause=N`. Missing fields default to `instant=0`, `hold=1`,
  `pause=0`. `|pause=N` is written only when N is above 0.
- Comments, blank lines and anything the patch cannot parse are kept as they are when the patch rewrites the file.
  You can edit it by hand with the game closed. If the file holds more than the patch's limits allow, the patch
  refuses to rewrite it.

## Uninstall

Remove the patch in Kotor Patch Manager. Saves made while it was installed still load. They keep the 32 `LUO_G00` to
`LUO_G31` globals. These are plain booleans that no game script reads, so they do nothing. This was checked offline
with the fake-engine scenarios S7 (id removed) and S5 (partial id). You can delete `levelup_override.ini`,
`levelup_playthroughs.ini`, `levelup_paths.ini` and the log files if you want.

Companions that were banked get their level-up indicator back, and the levels you held are available again
(the XP was never removed).

## Troubleshooting

The patch writes `levelup_override_log.txt` in the game folder (the previous log is kept as
`levelup_override_log.old.txt`).

- Check the banner line: `== LevelUpOverride 1.0.0 loaded ... crc32=XXXXXXXX ==`. The crc32 identifies the installed
  DLL.
- **Rollback:** set `mode=off` in `levelup_override.ini`, restart the game. The patch then does nothing. `mode=log`
  records what it would do without changing anything.
- To check a session automatically, run from the repository:
  `python3 scripts/luo_log_check.py --from N`, where N is the log line count before you started plus one.
  It reports PASS / FAIL / PARTIAL / NOT-EXERCISED for each check. `--scenario ID` checks a named scenario and
  `--expect-crc` compares the banner crc32 with a DLL. See `patch_manager_mods/26_FORCED_STATE_TESTING.md`.
- No Configure button: the character has no presets in `levelup_paths.ini`, it is the PC, or the table failed to load.
- A companion shows the level-up icon even though you configured a path: the icon is hidden only when the NEXT level
  is held. Levels before the conversion point follow the path and are not held.

## Compatibility

- The patch adds its button, status line and page to the character screen. Other Patch Manager mods that draw on the
  same screen (for example the Buff Duration HUD) work alongside it; the patch's character-screen hooks leave the
  controls those mods create in place.
- The vanilla level-up indicator, when shown, stops buff icons from drawing. Holding a level hides it, so banked
  levels do not hide buff icons.
- The presets are built for the mod #142 rules (Thematic Companions style): light-side powers are favoured, dark
  powers are marked, and Heal builds expect Treat Injury. They also fit a vanilla game, but the power choices were
  planned around those mods.
- Steam Aspyr `swkotor2.exe` only (see the sha256 above).

<!--
Reviewer notes (claims and sources):
- What it does, modes, defaults mode=rewrite/apply=1/verbose=0/table/dev=0, dev keys, precedence, playthroughs format:
  LevelUpOverride.cpp header lines 1-65; Playthrough.h header lines 1-9; manifest.toml description.
- Configure/Plan level-ups/Change path button text: LevelUpUi.h cfgButtonText (~31-33). Controls and captions: LevelUpUi.h
  UiCtl/uiRenderPage/uiClick (~41, 296-346, 370-395); row labels uiRowText ~300-312; option row logic OptionsLogic.h.
- Status line strings: LevelUpUi.h buildStatus. Banked message text and "once per (GUID, tag, N), at most every 5 s":
  LevelUpOverride.cpp header "R11 banked message".
- Pause: LevelUpUi.h uiCanPause/uiMinPause/uiClampPause/uiBankCycle/uiBankText (~236-262); levelNow comment ~554-575;
  doc 08 Phase 6 RESUME HERE (Atton minPause 3, clamp to 0 on preset change, user value not remembered).
- Recommended table: `recommended = true` in research/companion_levelup/paths/presets/*.toml (short, convert_at, hold_until_class).
  Jedi class names derived from hold_until_class ids (5 Sentinel, 3 Guardian, 4 Consular) - see presets .md for atton/Sentinel; others inferred.
- Instant leader: doc 08 "U5"; LevelUpOverride.cpp leaderInstant ~1565. Held GUI guards and indicator hiding: header hooks 2-5; doc 08 R5/R11.
- Per-save, PC unaffected, manual picks: doc 08 Status R1/R2, header hooks 6-8 and "pre-save/post-load/new server"; PC = empty tag, LevelUpUi.h ~414.
- Uninstall safety S7/S5: doc 26 section 6, doc 08 Phase 6b M3 notes, A4 adoption notes (removal safety).
- Troubleshooting: log name LevelUpOverride.cpp LogName; log.old rotation per luo_log_check.py header; banner line cpp ~934; luo_log_check.py usage; doc 26.
- Compatibility: Mod 14 hook 0x84D871 note in cpp header (Hook 9); indicator suppresses buff icons: doc 08 R5, R11; mod #142: presets/atton.md.
-->
