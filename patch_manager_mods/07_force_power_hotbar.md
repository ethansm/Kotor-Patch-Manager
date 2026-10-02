# Mod Idea 3: Force Power Hotbar (1-0 keybound, customizable)

**STATUS (2026-10-02 ~00:45, end of session 5) -- READ THIS FIRST. User verdict on the live build: "looks excellent!" -- the mod is FEATURE-COMPLETE; the remaining work is the final review/polish/documentation phase (see `~/.claude/plans/` plan "Mod 3 final review and finalization", summarised in the last section of this doc).**
- **1.0.0 finalization in progress (2026-10-02 ~01:40):** Phase R done (Gate R passed statically, section "1.0.0 finalization, Phase R" at the end). Phase B PROTOTYPE
  installed = DLL md5 `4f6d235b` (stage `hotbar_stage9/`, 28 ids, backup `backups/inst9_1002_0138`): the wrench is created at runtime from the LBL_QUEUE1 template
  and its art `fph_edit`/`fph_edit_hi` is served from DLL memory (all 4 `fph_*` textures registered); the other 56 controls still bind from the script `.gui`.
  **PROTOTYPE PROVEN LIVE (user 2026-10-02: "The wrench button looks great!")**: log `H3 LBL_HOTBAREDIT: created at index 131 from LBL_QUEUE1 rect=[11,1274,53,93]`,
  `tex fph_* (frame 120): gl=4166..4169 constructed=1 missing=0 pixels=00000000` (uploaded + buffer freed by the engine). NEXT = Phase B full: all 57 controls + 4 textures
  at runtime (append, no .gui binds), then `--rollback` the live .gui and delete `override/hotbar_*.tga` (game closed), user test.
  Rollback = reinstall build 20 (`6c8af8c0`, kpatch in `stage_engfix_1002_0121/`).
- **Previous install:** `force-power-hotbar` 0.3.0-dev = **build 20**, DLL md5 `6c8af8c0` (26 patch ids installed, siblings own the other 25 -- always re-read `kpm_install_state.json`);
  GUI deployed = **168 controls** (`override/mipc28x6_p.gui`), textures `override/hotbar_slot*.tga`, `override/hotbar_edit*.tga`. Game closed at the end of the session.
- **Features (all live-verified unless noted):** ten native-width (35 u) square HUD buttons for keys `1`-`0` (icons at native quick-bar size, digit label up/left of the icon, dimming, hover frame swap + the native GUI hover sound,
  delayed tooltip, click + key invoke through the HUD's own path, `Invoke=1`); thin wrench toggle at the far left (x=4, aligned with the quick-action panel border, lighter native panel-border colour, thin 2 px rim, beige hover/active frame, tooltip);
  Edit mode -> click a slot -> picker box spanning wrench..slot 0 (title with range `Powers 2-9 of 12`, tabs Powers/Items/Modes, `<` `>` per-entry scroll with hold-repeat, mouse wheel scrolls one entry per notch, Clear, 8 cells, hover + tooltips)
  -> click a cell to assign; saved per character in `[Profile.<key>]` of `force_power_hotbar.ini` (Default fallback; explicit `none` = cleared; AutoFill default OFF; unassigned slots hidden outside Edit mode); Esc backs out one level;
  digits/clicks never invoke while editing; the bar follows character switches every frame and hides unresolved slots for 1.5 s while the HUD lists settle.
- **Character key:** `obj<hexid>` of the controlled creature; Exile = `objfffffffd`. Invalid id `0xFFFFFFFF`/0 = unknown (saves refused) EXCEPT a one-member party with `0xFFFFFFFF` -> `objfffffffd` (seen for a whole session on the Ebon Hawk).
  Companion ids `obj80000240`/`obj80000249` were identical across game sessions of one save (stability across DIFFERENT saves still unverified). `CSWCObject::GetPortraitId` (`+0xB8`) reads 65535 on client creatures -> NOT usable (the `port<id>` path in `computeCharacterKey` is dead code that never triggers).
- **Agy review (2026-09-30) handled:** stuck modifiers resynced via `GetAsyncKeyState` (build 13), bound keys always swallowed when the ring is full (13), WndProc restored on detach only if still on top (13); roster-index idea rejected (`PartyTable::GetMember` indexes party positions, not the 12-NPC roster).
- **Layout (800x600 GUI units, 2.667 px/unit at 3840x1600):** wrench [4,478,20,35]; slots x=29+40*i, y=478, 35x35, icon label inset 2 (31 u); digit labels [x+4,y+1,18,18]; picker box [4,py-8,420,112] with py=y-110; bar bottom (513 u) sits 9 px above the native description band like the native panel-border gap.
  GUI IDs (script `ALL_TAGS` order == DLL constants `EditId..TipId`): 111-120 frames, 121-130 icons, 131 wrench, 132 picker bg, 133 title, 134-136 tabs, 137-139 prev/next/clear, 140-147 cell frames, 148-155 cell icons, 156-165 digit labels (`LBL_HOTBARNUM<d>`), 166/167 tooltip (MUST stay the last two IDs: panel Draw is ascending by ID).
- **Crash history:** build 8 (`FUN_0073FEA0` is a thiscall, removed in 9). No crashes since.
- **Sources:** `force_power_hotbar_src/` (`ForcePowerHotbar.cpp` ~1200 lines, `HotbarConfig.h`, `TwoDA.h`, `Tlk.h`, hooks.toml/manifest/exports, host tests `config_roundtrip_test.cpp`, `twoda_test.cpp`; mirrored from the uncommitted framework clone
  `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/ForcePowerHotbar/` -- build with `python3 ../create-patch.py`, copy the .cpp there first). GUI + textures: `scripts/add_hotbar_buttons_gui.py` (57 labels; `--replace` / `--rollback` / `--dry-run`).
- **Known gaps / open items:** (1) hover-sound index 1 assumed to be the native one (user did not object); (2) companion key stability across different saves; (3) unknown INI sections/keys are refused on Save, not preserved; (4) a few function-level statics survive a HUD rebuild (stale log lines only);
  (5) arrow art is `<` `>` text; (6) no unit-test coverage of the editor/DLL logic (host tests cover only config + 2DA); (7) dead code (`port<id>` key, `scanObjectStrings`, probe/catalog dumps) and heavy debug logging still in; (8) the doc is a long chronological log -> needs consolidation. All of these are inputs to the finalization plan.
- **Sections below, in order:** original spec (partly SUPERSEDED), "Next-phase findings" (Ghidra), "Live results" (0.1), "Environment and workflow gotchas" (READ before installing), "Bottom-bar UX: design, build", dated build logs (builds 1-20, design rounds 1-3), "Next phase: in-game configuration GUI" (plan + decisions), test-coverage note, session-5 logs.
- **Install workflow (session 5, proven 4 times):** never install while the game runs (`pgrep -f '^S:.*swkotor2\.exe'`). Helper `/tmp/claude-1000/inst/` (recreate if gone: net8 console app referencing `bin/Debug/KPatchCore.dll` + `KPatchLauncher.dll`; `AppSettings.Load()`, set `DeploymentPolicy.PreferLibraryProxy` / `GameDetector.IdentifyUnrecognisedBuilds`,
  `PatchOrchestrator(stage).GetAvailablePatches()` (aborts with "missing <id>" BEFORE removing anything), `PatchRemover.RemoveAllPatches(exe, removeManagedState:false)`, `InstallPatches(exe, ids, patcherDirectory: bin/Debug/)`); run `dotnet /tmp/claude-1000/inst/out/inst.dll <gameDir> <stageDir> <ids...>` from inside `bin/Debug/` with the launcher's `KPatchLauncher.deps.json`/`.runtimeconfig.json` copied to `inst.deps.json`/`inst.runtimeconfig.json`.
  Staging script: for every id in `InstalledPatches` pick the kpatch whose DLL md5 equals the INSTALLED `patches/<id>.dll` (search `**/*.kpatch` in the framework dir); non-DLL patches from the previous stage dir; hotbar from `Patches/ForcePowerHotbar/`. If a sibling hot-copied a DLL that matches no kpatch (e.g. `buff-duration-hud`), install the usual kpatch and copy their DLL back afterwards.
  Back up `patches/` + `kpm_install_state.json` to `backups/instN_*`, install, diff the DLL md5 list before/after (only the hotbar DLL may change), `AR_ERROR.LOG` md5 `a2d887896b53eae113758dc8c3729db9`. Stage dirs used: `hotbar_stage5..8` (latest = `hotbar_stage8/`, 26 ids). GUI/texture-only changes need no reinstall (`--replace`, game closed). INI reset: back up `force_power_hotbar.ini`, write `[General] AutoFill=0` + empty `[Profile.Default]`.

> Everything from here to "## Next-phase findings" is the original pre-Ghidra spec. Where it conflicts with the later sections, the later sections win
> (notably: items ARE in the HUD action bar; the four 0x1D0 arrays are sub-controls, not per-portrait bars; the 3-edge/30-slot layout is deferred).

## Goal

30 generic hotbar slots across three screen edges — bottom (keys **1-0**),
left (**Ctrl+1**-**Ctrl+0**), right (**Alt+1**-**Alt+0**) — each
independently **customizable** to hold any single vanilla quick-action
(a Force power, an item ability, or anything else the native quickbar
itself can hold). This replaces the friction of the game's existing
single quickbar item + scroll-wheel power selection with direct one-key
access — no scrolling to cycle to the power or item you want mid-fight.

## Why this is a Patch Manager task

The existing quickbar-plus-scroll-wheel mechanic and the 2DA-driven power
list are native engine/NWScript territory, but a *new persistent row of
clickable/keybound buttons*, each independently customizable and each
needing its own icon/state render, is a new GUI/input surface the engine
doesn't have a slot for today — same category of "impossible without a
raw engine hook" as the other two mods in this doc set. Two engine-level
pieces are needed that nothing in NWScript/2DA/GFF can provide:
1. **Input**: binding physical keys 1-0 to "invoke the power assigned to
   this slot," independent of (or in addition to) however the existing
   quickbar's own input handling works.
2. **Render + persistence**: drawing 10 buttons with correct icons, and
   remembering per-player slot assignments across sessions (an in-memory-
   only solution would reset every relaunch, which isn't "customizable"
   in any durable sense).

## Relationship to the Buff Duration HUD mod ([[02_buff_duration_hud.md]])

The user's own framing, captured verbatim for whoever resumes this:
there's a natural temptation to make an active hotbar button *change
color* to show its power is still active/on cooldown (reusing exactly the
active-effect data the Buff Duration HUD mod's Target 1 research already
resolved — `CGameEffect::GetExpiryTime`, Steam `0x005e4fa0`,
`CGameEffect+0x10`/`+0x14`, filtered by spell ID per the
`02_buff_duration_hud.md` effect-ID table).

**Decision (user): keep the two mods' UX separate for v1.** Don't merge the
cooldown-wipe visual into the hotbar buttons themselves. Instead:
- The hotbar mod's own hook, when a slot is pressed, invokes the power
  through whatever call the existing quickbar/action-bar system already
  uses (see "Action invocation layer" below).
- The Buff Duration HUD mod's **existing, separately-hooked** render pass
  near the party portraits (Target 2, `FUN_007457e0`, per
  `02_buff_duration_hud.md`) picks up the resulting active effect
  automatically, because it already reads the same live active-effect
  list — no direct hotbar-to-buff-HUD hook wiring is needed, they're
  decoupled by construction as long as both read from the same
  `CGameEffect` active-effect array. If a later session wants the
  combined "color the hotbar button too" UX, it would layer on top of
  this decoupled baseline rather than requiring it.

## Key Lessons Learned from Mods 1 & 2 Applied Directly to Mod 3

The extensive Ghidra static analysis and debugging across Mod 1 and Mod 2
yielded several concrete architectural lessons that directly reshape this mod:

### Lesson 1: Native Widget Reuse vs. Rolling Custom Primitives
- **Experience from Mod 2**: Trying to build new geometry or radial shader
  sweeps stalled; repurposing native `CSWGuiLabel` borders and modifying live
  dimension fields (`self+0xc`/`self+0x10`) on `CSWGuiBorder::Draw` resolved the
  entire wipe mechanism with zero new render passes.
- **Application to Mod 3**: Do **not** attempt to inject 10 custom OpenGL/D3D
  quads or roll raw button meshes. Reuse the existing native action-bar widget
  architecture (`CSWGuiMainInterfaceAction` at `hudThis + 0x733c`, stride
  `0x750`) or native `CSWGuiButton` widgets (`vftable 0x00987974` on Steam),
  which already handle border frames, icon textures, mouse hover states, and
  disabled shading.

### Lesson 2: Mining Framework Symbol Databases Before Decompiling
- **Experience from Mods 1 & 2**: Blind cross-reference searching wasted hours
  on functions that were non-virtual and had stripped symbols. Querying the
  framework's shipped `AddressDatabases/*.db` provided real class and method
  names immediately.
- **Application to Mod 3**: A pre-investigation query has already revealed
  concrete symbols on Steam and GOG:
  - `CSWSCreatureStats::HasSpell`: Steam Aspyr address **`0x006BD9C0`** (decimal `7068096`).
  - `CSWSCreatureStats::AddKnownSpell`: Steam Aspyr address **`0x006B86A0`** (decimal `7046816`).
  - `Global::CSWGuiInterfaceAction_UseForcePower`: GOG address **`0x004254A0`** (decimal `4347040`).
  This gives exact entry points for data validation and spell casting on Day 1.

### Lesson 3: Live Install 2DA Parsing with pykotor
- **Experience from Mod 2**: KotOR II 2DA files are in binary `V2.b` format and
  heavily modified by override mods; hardcoding IDs fails.
- **Application to Mod 3**: The hotbar setup must read `override/spells.2da` via
  the worktree's `.venv` `pykotor` parser to extract icon resrefs, localized
  names, and Force point costs.

### Lesson 4: Input Interception & Context-Gating Discipline
- **Experience from Hooking & Porting**: Detour hooks with `skip_original_bytes = false`
  run our code first, followed by original instructions.
- **Application to Mod 3**: In KotOR II, keys **1, 2, and 3** are default
  bindings for party member selection and **dialogue option selection**.
  Unconditionally hijacking keys 1–0 will break conversation menus. The input
  hook must be context-gated: only swallow keys when in active exploration/combat,
  and let them pass through when in dialogue, menus, or console input.

### Lesson 5: Externalized Persistence Over Savegame Injection
- **Experience from Mods 1 & 2**: Keeping mods completely independent of `.sav`
  structures guarantees zero save corruption risk.
- **Application to Mod 3**: Slot configuration must live in an external flat
  file (`kotor_hotbar.json` or `.ini`), never inside saved game structs.

## Implementation Pipeline (Refined)

### 1. Data Layer — Known Powers & Availability
- **Known Powers**: For the active party leader / controlled creature, query
  `CSWSCreatureStats::HasSpell` (Steam `0x006BD9C0`, calling convention
  `__thiscall(statsPtr, spellList, spellId, checkUsable)`).
  - The creature's stats pointer `statsPtr` is accessible from the controlled
    creature object (`creaturePtr + 0x...` / `CSWSCreatureStats` from
    `CSWSCreature`).
  - Iterating the active self-buff and offensive force power list from
    `spells.2da` against `HasSpell(0, spellId, 1)` yields the exact subset of
    powers the player is currently eligible to cast.
- **2DA Data**: Reuses Mod 2's `pykotor` extraction pattern on `override/spells.2da`:
  - `label`: internal name.
  - `name`: StrRef for tooltip display.
  - `iconresref`: `.tga` icon to pass to widget fill.
  - `forcecost`: base Force point cost.
  - `forcepoints`: checks whether character has sufficient current FP.

### 2. Input Layer — Key Interception & Context Gating
> [!IMPORTANT]
> Keys 1, 2, and 3 are vital for dialogue choices. The input hook must strictly
> verify game state before claiming any keystroke.

- **Hook Location**: Detour into the main window message loop or
  `CSWGuiManager::HandleInputEvent` / keyboard event dispatcher.
- **Context Gate**:
  - Gated on HUD visibility (`CSWGuiMainInterface::m_bVisible`).
  - Gated on conversation state (`CServerExoApp::GetActionConversation` or
    checking if `dialog.gui` is loaded).
  - Gated on modal GUI presence (returns early if inventory, journal, or options
    menu is active).
  - Gated on cheat console visibility (checks `0x00a30828` or console active flag).
- **Key Swallowing**: When all gates pass and a mapped slot (1–0) is pressed:
  - If the slot is populated and usable, invoke the action and swallow the event
    (`skip_original_bytes = true` or set event handled).
  - If the slot is unassigned, let original bytes execute normally.

### 3. Action Invocation Layer — Dispatching the Force Power
- **Target Functions**:
  - `Global::CSWGuiInterfaceAction_UseForcePower` (GOG `0x004254A0`): Research its
    Steam equivalent. Decompilation of GOG shows it constructs an action node
    and queues it onto the controlled creature's action list.
  - Sibling lead: `CSWSObject::AddActionToFront` (Steam `0x003630A0` / decimal
    `35508256` in DB) or `ExecuteCommandActionUseTalent`.
- **Targeting**:
  - Self-buffs (Force Speed, Valor, Armor, Barrier): pass target = self (`creaturePtr`).
  - Targeted powers (Force Push, Lightning): pass current combat target from
    `CSWGuiMainInterface`'s active target pointer, or default to current hostile
    target.

### 4. Render Layer — 10 Hotbar Buttons
- **Widget Blueprint**:
  - Model after `CSWGuiMainInterfaceAction` (the 7 action-bar slots at
    `CSWGuiMainInterface + 0x733c`, stride `0x750`), which already encapsulates
    border rendering, icon display, mouse hit-testing, cooldown flash, and click
    events.
  - Or instantiate 10 `CSWGuiButton` controls dynamically linked to the main HUD.
- **Ultrawide Coordinate Alignment**:
  - Follow `popos/docs/kotor2_ultrawide.md` and `HUD_DESC_COEFF` scaling rules.
  - Position across the bottom center or aligned next to the existing action bar
    without colliding with the minimap (right) or party portraits (left).

### 5. Persistence Layer — Slot Customization
- **Storage Strategy**: Flat JSON or INI file:
  `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/kotor_hotbar.ini`.
- **Keying**: Key slot mappings by the player character's unique name or tag
  (read from `CSWSCreatureStats`), with one generic key per slot across all
  three edges, each holding a generic `type:id` value (revised 2026-09-29
  for the 3-edge/30-generic-slot layout — supersedes the earlier
  `Slot`/`ItemSlot`-section sketch below it replaced):
  ```ini
  [Revan_Exile_Player]
  Bottom1=spell:8      ; FORCE_POWER_SPEED_BURST
  Bottom2=spell:22     ; FORCE_POWER_FORCE_VALOR
  Bottom3=spell:17     ; FORCE_POWER_FORCE_ARMOR
  ...
  Bottom0=spell:135    ; FORCE_POWER_FORCE_BARRIER
  Left1=item:<tag>     ; e.g. a stim's inventory tag
  Left2=item:<tag>
  ...
  Left0=item:<tag>
  Right1=spell:24      ; any type may occupy any edge -- fully generic
  ...
  Right0=item:<tag>
  ```
- **Customization UX**:
  - v1: Edit config file or simple console command via `AdditionalConsoleCommands`
    (e.g., `sethotbar <slot> <spell_id>`).
  - v2: Drag-and-drop or right-click assignment menu directly from the Spells GUI.

## Slot layout (3 edges, 30 generic slots) -- REVISED SCOPE, 2026-09-29, not yet researched

**User's revised direction, verbatim intent (supersedes the earlier
"2nd row for items, possible 3rd row undecided" framing)**: instead of
category-based rows (a "powers row," an "items row," a speculative 3rd
row), use a **positional layout across three screen edges**, and make
**every slot generic** -- any single "quick action" the vanilla game's own
quickbar can hold (a power OR an item OR anything else in that system)
should be assignable to **any** slot, not pre-restricted by which
edge/row it's on:
- **Bottom of screen**: bare `1`-`0`, 10 slots (unchanged from the
  original design).
- **Left side of screen**: `Ctrl+1`-`Ctrl+0`, 10 slots.
- **Right side of screen**: `Alt+1`-`Alt+0`, 10 slots.
- **30 total generic slots**, each independently configurable to
  whatever the player wants -- no hardcoded "this row is for items."

**Design implication**: this reframes the mod's data model from "two
typed tables (spellId-keyed powers, item-tag-keyed items)" to **one
generic slot type carrying whatever discriminated reference the vanilla
quickbar itself uses internally** (almost certainly a type tag + ID pair,
e.g. "spell, id=8" vs. "item, tag=..."). Concretely, the open research
question is no longer just "how does the quickbar resolve an ITEM icon"
(Buff Duration HUD's Item 5) but **also** "what is the vanilla quickbar's
own internal representation of *any* assignable slot's contents" --
likely the same decompile target (`CSWGuiMainInterfaceAction`'s
slot-assignment fields/`SetIcon`'s parameter shape) answers both
questions at once; whoever runs Buff Duration HUD's Phase A.5 research
should explicitly check for this and record it here too if found.

**Design sketch (not yet Ghidra-verified)**:
- **Input**: two additional key-gate layers beyond the existing bare-digit
  one -- `Ctrl+1`-`Ctrl+0` (left edge) and `Alt+1`-`Alt+0` (right edge),
  same context-gating discipline as Lesson 4 above (must not fire during
  dialogue/menus/console -- a modifier key likely makes accidental
  collision with dialogue's bare `1`/`2`/`3` bindings less likely, but
  this still needs verifying against any existing `Ctrl+<digit>`/
  `Alt+<digit>` bindings before claiming either combo is free).
- **Data layer**: a slot can hold EITHER a power (`spells.2da` row +
  `CSWSCreatureStats::HasSpell`) OR an item (party's live inventory --
  likely `CItemRepository`/`CSWItem`, real RTTI classes already in the
  `ghidra_knowledge` DB's `classes` table from the Inventory 3D Viewport
  research) rather than `CSWSCreatureStats` alone. Whatever generic
  type-tag+ID shape the vanilla quickbar itself uses (see "Design
  implication" above) should be reused directly instead of inventing a
  parallel one.
- **Action invocation**: using an item (stim, repair kit) is a different
  native call path than casting a power -- likely something in the
  `CSWSItem`/`CItemRepository` family rather than
  `CSWGuiInterfaceAction_UseForcePower`'s spell-casting path. Not yet
  traced.
- **Icon resolution: SHARED BLOCKER with [[02_buff_duration_hud.md]]'s
  "Item 5" -- premise CORRECTED 2026-09-29 (Buff Duration HUD's 9th
  session ran Phase A.5)**: the vanilla action-bar (`CSWGuiMainInterfaceAction`,
  `hudThis+0x733c`/`+0x750c`/`+0x76dc`/`+0x78ac`, one 6-category array per
  party portrait) does **NOT** let a player assign an item to any of its
  categories -- decompile of all 6 category populators (`FUN_00779510`,
  `FUN_0077c510` x3, `FUN_0077a9e0`, `FUN_0077aeb0`) shows they cover
  attack-mode/3 feat+power tabs/combat-AI-behavior-toggles/generic only,
  never inventory items. **This mod's own "use an inventory item" action
  invocation is therefore confirmed to need a genuinely separate native
  call path from `UseForcePower`/this action-bar's own invoke dispatcher
  (`FUN_00751750`, which calls each entry's own `+0xc` function pointer --
  directly reusable for THIS mod's Force-power/feat slots, since those DO
  live in this system) -- not a shared one.** The icon-resolution question
  for item slots is still open; `CGameEffect`/`CSWItem` (the latter has
  real RTTI on Steam, confirmed this session) may still be a cleaner lead
  than the quickbar ever was. Full finding: `02_buff_duration_hud.md`'s
  "Item 5" section and `ghidra_knowledge` DB `lessons` ids 54-56.
- **Persistence**: `kotor_hotbar.ini`-style config keyed by a generic slot
  identifier (`Bottom1`..`Bottom0`, `Left1`..`Left0`, `Right1`..`Right0`),
  each holding a generic `type:id` value (e.g. `spell:8`, `item:<tag>`)
  rather than separate `Slot`/`ItemSlot` sections. Items don't have a
  stable numeric ID the way `spells.2da` rows do -- items are identified
  by `BaseItems.2da` row + a unique instance tag/UUID in the player's
  inventory, a different identity problem than a spell row's simple
  integer key; not yet researched how to make a persisted item-slot
  assignment survive the item being consumed/restocked.
- **Not yet researched at all**: the exact native function(s) for
  "use this inventory item" (separate from "cast this force power"),
  how the vanilla quickbar itself distinguishes a spell-slot from an
  item-slot internally (probably a type tag alongside whatever ID it
  stores -- see "Design implication" above, this is now an explicit open
  question, not just an assumption), and whether stims/shields/repair-kits
  share one invocation path or three separate ones.

## Open Questions for Whoever Resumes

1. **Steam Address for `UseForcePower`**:
   Trace GOG's `0x004254A0` (`CSWGuiInterfaceAction_UseForcePower`) to its Steam
   Aspyr counterpart via caller xrefs or positional clustering.
2. **Target Selection for Offensive Powers**:
   How does the native quickbar acquire the current target when clicking an
   offensive power? (Inspect `CSWGuiMainInterfaceAction::HitCheckMouse` and
   action dispatch).
3. **Dialogue Context Flag**:
   Confirm the exact Steam memory offset or global pointer for "in conversation"
   to reliably gate the 1-3 keys.
4. **Vanilla quickbar's generic slot-content representation**:
   Confirm the native quickbar's own internal representation of *any*
   assignable slot's contents (type tag + ID shape, e.g. "spell vs. item")
   via the same decompile target as Buff Duration HUD's Item 5/Phase A.5
   (`CSWGuiMainInterfaceAction`'s slot-assignment fields / `SetIcon`'s
   parameter shape) — reuse that finding, don't re-derive it here.

## Revised Effort Estimate

- **Previously Estimated**: Higher than Buff Duration HUD (due to unknown input
  and known-powers paths).
- **Current Assessment**: **Moderate** (comparable to Buff Duration HUD).
  - The known-powers check is already 80% solved thanks to `CSWSCreatureStats::HasSpell`
    (`0x006BD9C0`) in the framework DB.
  - Mod 2 already solved 2DA parsing and effect decoupling.
  - The primary remaining investigation is the keyboard input context gate and
    the Steam address for `UseForcePower`.

## Next-phase findings (2026-09-29): input -> invoke prototype, no bars

Status: `ForcePowerHotbar` 0.1.0-dev built (`force_power_hotbar_src/`, framework clone `Patches/ForcePowerHotbar/`),
**not yet live-tested**. Scope: 30 slots key -> invoke, INI config (hot-reloaded), verbose logging, no on-screen bars.

### Rosetta stone
`Patches/ExpandedKeyboardControl` (GOG-only) already does capture + gating + invoke. GOG->Steam delta for
`CSWGuiMainInterface` is a constant **+0x22E880** for GOG >= 0x51cb20 (22 functions checked); `CExoInputInternal::GetEvents`
delta +0x119300 (only +0x383 verified). Steam addresses (all in the knowledge DB `functions`, `hooks_ported`):

| Role | Steam |
|---|---|
| HUD Draw / tick hook (`55 8b ec 6a ff`, ECX=hud) | `0x0074B3F0` |
| DirectInput capture hook (`89 85 bc fe ff ff`; EAX=record{dwOfs,dwData}, `[ebp-0x10]`=device) | `0x0072CB63` |
| activate button | `0x007516B0` thiscall(hud, button) RET 4 |
| personal invoke | `0x00751750` thiscall(hud, group 0..6, 0) RET 8 |
| target invoke | `0x00751710` thiscall(hud,i,0) RET 8 -> `007445d0(hud+0xCC, i, 1)` |
| keyboard device index | `0x0099C884` rdata constant = 0 (unverified live) |
| gate whitelist vtables (RTTI) | Messages `0x0099E5EC`, Fade `0x009A044C`, Pause `0x009A7BAC`, AutoPause `0x009A8FCC`; HUD `0x0099DCC4` |

Hook bytes verified against the live exe; no live hook within 64 bytes of either site.

### Entry model (corrects lesson 55 / Buff HUD 9th session)
The HUD action bar has **6 personal groups** (`hud+0x733C+g*0x750`) + 3 target groups (`hud+0x120+i*0x750`); the four
`0x1D0`-spaced arrays are the 4 sub-controls of each group, NOT one bar per portrait. Per-group entry list
`hud+0x78+g*0xC` {ptr,count,cap}, stride **0x3C**; selected key `hud+0x1C7C+g*4` (-1 none); active group `hud+0x1C98`.
Entry: `+0x8` key, `+0xC..0x18` callback blob, `+0x1C` owner id, `+0x30` flags (bit0 usable, bits1-4 why-not), `+0x0` name (CExoString, unconfirmed).
- **Items ARE in the bar**: `FUN_0077c510` (groups 1-3, mask 1/4/2) builds inventory-item entries via `FUN_0077bcf0`,
  key = `itemObjectId | 0x40000000`, name `"%s (%d)"`, callback `FUN_0077c780`/`c80` (mask `0xBFFFFFFF`).
- **Powers**: `FUN_008495c0` (groups 0 and 5) iterates the creature's known spells (spells.2da row via `FUN_006c1450`),
  key = `spellRow | 0x20000000`; specials 0x102-0x10c are mode toggles (`FUN_0077d120`). Group 4 = AI modes.
- Invoke design: set `hud+0x1C7C+g*4` = wanted key, call `0x751750(hud,g,0)`, restore selection + active group.
- Items have no stable numeric id (object id changes across loads) -> bindings store an `ActionRef` (see Configuration).
  Item identity is currently the display name (text before " ("); resref/tag offsets not found statically -- the live probe
  scans objects for strings. Character key: `CreatureTagOffset` INI setting (0 = fall back to object id) once the probe finds it.

### Configuration (schema v1)
`force_power_hotbar.ini` in the game dir, hot-reloaded (<= 1/s). `[General]` `Invoke=0|1`, `Verbose`, `KeyboardDevice`, `CreatureTagOffset`.
`[Profile.Default]` + `[Profile.<characterKey>]`; keys `Bottom1..0`, `Left1..0`, `Right1..0`; values `spell:<row>`, `feat:<row>`,
`item:<name>`, `other:<hex>`; per-slot fallback character -> Default. `HotbarConfig.h` is pure C++ (host-tested round trip:
`config_roundtrip_test.cpp`, `g++ -std=c++17`). The future in-game editor mutates a `Profile` and calls `Config::Save`
(atomic tmp+rename; Save drops free comments but keeps per-slot trailing notes). Catalog: `force_power_hotbar_available.txt`.
Key bindings, bar layout and per-save profiles are data tables in code (`kRows`, `kFirstDik`), not yet user-exposed.

### Open items
- Target-menu slots not supported this phase (`menu = hud+0xCC` confirmed via 0x751710, selection storage unverified).
- Item/creature identity fields, keyboard device constant, gate behaviour: verify from the first live probe log.
- Render phase candidate hook: end of `FUN_0074b3f0` after the action-bar draw loop (`FUN_007417d0` x7); `GetActionIcon` candidate `0x0074E0B0` -- not researched further.

## Live results (2026-09-29, builds 1-3 on the Steam/Proton install)

All evidence is in `force_power_hotbar_src/live_evidence/` (catalog, a full probe dump, key/gate events) and was gathered by the always-on probe.

**Verified live**
- Both detours apply (`Applied DETOUR hook at 0x0072CB63 / 0x0074B3F0`), no `AR_ERROR.LOG` change (md5 `a2d887896b53eae113758dc8c3729db9`).
- **Keyboard device index = 0** (constant at `0x0099C884` = 0). Every keyboard record arrives with `dev=0`; digits are DIK `0x02..0x0B`. No non-keyboard record was ever seen by the hook (mouse untouched by this patch; a "stuck right mouse button" the user saw was the game's own *mouse look* setting, not this mod).
- **Capture + swallow works**: bound digit press -> `SWALLOW`, release -> `swallow-up`, unbound digits pass through (`pass(unbound)`), Ctrl+digit works (`Left` row), Ctrl/Alt/Shift state tracked from the same stream.
- **Gate works**: `ACTIVE (hud on top)` in exploration; `inactive (modal open)` when a menu modal opens (modal vtable `0x0099E6A4`, fade panel `0x009A044C` also in the panel list, HUD `0x0099DCC4`, manager at `hud+0x1C`, panel flags `+0x48`). Digits inside dialogue/inventory/pause were **not yet tested** -- test before enabling invoke.
- **Alt+digit never arrives** under Proton (an Alt press is seen, no release, no digit) -- window-manager interception. Irrelevant now (only the bottom row is in scope); if the Alt row returns, use Shift or Ctrl+Shift instead.
- **Resolver**: `spell:8` resolved to `group 0 index 1 key=20000008`. `item:Medpac` resolves in build 2+ (build 1 could not read names).

**Entry layout (live-confirmed, 0x3C stride, `hud+0x78+g*0xC` {ptr,count,cap}, `hud+0x1C7C+g*4` selected key)**
```
+0x00 char*  display name (CExoString ptr)      +0x04 int name length INCLUDING the trailing NUL
+0x08 int    key (0x20000000|spellRow, 0x40000000|itemObjId, raw for AI modes)
+0x0C ptr    callback (0x007791A0 powers, 0x0077C780/0x0077CC80 items, 0x0077D890 AI modes)
+0x1C int    owner object id (0xFFFFFFFD for the Exile)   +0x20..0x2F  16-byte inline ICON RESREF (e.g. "ip_speedburst", "ii_medeqpmnt_002")
+0x30 int    flags: bit0 usable, bits1-4 why-not code     (higher bits vary, ignore)
```
Live groups for the Exile: g0 powers (Heal 10 `ip_cure`, Burst of Speed 8 `ip_speedburst`, Force Valor 22 `ip_mind`, Energy Resistance 42 `ip_resistenergy1`); g1 items (Adrenal Alacrity/Stamina/Strength, Battle Stimulant, Antidote Kit, Medpac, Repair Kit); g2 shields (Energy Shield, Peragus Mining Shield); g3 mines; g4 AI modes (`other:410..414`, icons `ib_*`); g5/g6 empty. Icons are 16-char resrefs -> exactly what a button image needs.
Companions appear as `obj80000249` / `obj80000240`; the Exile as `objfffffffd` (object id; `CreatureTagOffset` was never found: heuristic string scan found nothing in the first 0x800 bytes of the creature object even after the NUL-length fix -- a real tag read still needs research).

**Bugs found by live logging and fixed:** CExoString length includes NUL (names were empty); nothing else. Log helper caps at 60000 lines, frame-stamped, with heartbeat every 1800 frames and a warning for any HUD tick gap > 1.5 s.

## Environment and workflow gotchas (read before installing anything)

- Several Claude sessions develop patches at once (Buff Duration HUD, Inventory 3D viewport, this one). **Any session's install replaces the whole patch set**; a stale staging dir silently downgrades or drops another session's patch (this happened twice: my install dropped `inventory-3d-viewport`, then another session's install reverted my hotbar DLL). Always: read `kpm_install_state.json` -> `InstalledPatches` right before installing, include every installed id, and copy the **newest** `.kpatch` of each from its own `Patches/<Name>/` dir. Verify afterwards: `md5sum $G/patches/force-power-hotbar.dll` equals the DLL inside the staged kpatch (`unzip -p X.kpatch 'binaries/*.dll' | md5sum`).
- My stage dir: `.../kotor-patch-manager/hotbar_stage/` (16 kpatches). Install command (game must be CLOSED; the launcher also relaunches the game): `cd .../kotor-patch-manager/bin/Debug && ./KPatchLauncher "$G/swkotor2.exe" --patches <stage> <all ids>`. Current 16 ids: the 13 baseline + `buff-duration-hud` + `force-power-hotbar` + `inventory-3d-viewport`.
- Logs (game dir `$G`): `force_power_hotbar_log.txt` (appends across launches; older ones renamed `.buildN*.txt`), `force_power_hotbar_available.txt` (catalog), `force_power_hotbar.ini` (config, hot-reloaded). Steam/Proton log `~/steam-208580.log`.
- "Is the game running": `pgrep -f '^S:.*swkotor2\.exe'`. Commit only by explicit path (other sessions edit other files in the worktree).

## Next phase: bottom-bar UX (to be planned next; NOT started)

**Goal:** ten on-screen buttons along the bottom of the screen mapped `1`-`0` (bare digits), each showing the icon of its bound action, clicking or pressing the key invokes it (the existing capture/invoke path). Bindings come from the same INI (`Bottom1..Bottom0` in `Profile.Default` / per-character profiles). Extra rows: future.

**Why it is new work:** everything so far reused existing widgets. Creating NEW HUD elements means learning how the engine builds, positions, draws and hit-tests GUI controls at runtime. Investigate before designing:
1. **Where the vanilla action bar is drawn and how** -- the existing 6+3 action-button groups (`hud+0x733C + g*0x750`, `hud+0x120 + i*0x750`, 4 sub-controls at `{0,0x1D0,0x3A0,0x570}`); `FUN_007417d0` (per-group draw, called x7 in `FUN_0074b3f0`), `FUN_00741780` (called by the invoke for feedback), `GetActionIcon` candidate `0x0074E0B0`. Can we clone/reuse whole `CSWGuiMainInterfaceAction` widgets (constructor, `SetIcon`, border, hit test) instead of building from raw controls? Their constructor/init lives in the HUD ctor `FUN_00747210` (huge).
2. **Buff HUD's proven technique for runtime-drawn widgets:** a `CSWGuiLabel` (vtable `0x009878bc`; `+0x60` embedded `CSWGuiBorder` vt `0x009875bc`, fill image at border+0x74, size at +0xc/+0x10; `+0xd8` CSWGuiText) placed with `vtable[1]` SetRect and drawn with `vtable[14]` Draw(dt); icon by swapping the FILL image (`FUN_0047eb60` cdecl(resref buf)-> image, see `buff_duration_hud_src/BuffDurationHUD.cpp`). Question: can that pool of labels be extended to 10 icon buttons drawn at the end of `FUN_0074b3f0` (after the action-bar loop), without registering them as GUI controls?
3. **Render hook site:** a clean instruction boundary near the end of `FUN_0074b3f0` (HUD Draw) so the buttons draw with the HUD (correct z-order, clip, HUD scale `FUN_00479ab0`, ultrawide -- see `kotor2-ultrawide-gui` memory and the `HUD_DESC_COEFF` rules).
4. **Mouse input for clicks:** the hotbar needs hit-testing for its own rects (the HUD's `HitCheckMouse` `0x0074B3A0` only knows native controls). Options: own rect test in the tick using the game's mouse position, or make the widgets real controls. Also hover/pressed visuals and disabled-state shading (`entry+0x30` flags give usable/why-not).
5. **Layout:** bottom-centre strip, above/below the native action bar? avoid the minimap (right) and party portraits (left); scale with `FUN_00479ab0`; check 16:9 and the user's ultrawide setup.
6. **State display:** usable flag, item stack count in the entry name ("Medpac (7)"), and later cooldown/active-buff reuse of the Buff HUD wipe (kept separate for v1 by user decision).
7. **Config UX (later):** in-game assignment (drag/right-click) writes through `hb::Config::Save`; not part of the button-bar phase but the button widgets must expose slot identity.

**Open items carried over:** run digit tests in dialogue/inventory/pause; first `Invoke=1` test (self-buff power, medpac, stim, shield); target-menu powers (`menu=hud+0xCC`, selection storage unverified); item identity is the display name (resref/tag not found); character key is the object id until a tag read is found; Phase F independent review not yet done.


## Bottom-bar UX: design, build (session 3, 2026-09-29)

**Design (all Ghidra-verified, addresses in the knowledge DB `functions`/`lessons`/`hooks_ported`):** the ten buttons are real HUD controls,
the same pattern as Mod 1's 3D view.
1. `scripts/add_hotbar_buttons_gui.py` appends 10 type-4 labels `LBL_HOTBAR1..9,0` (IDs 111-120, copy of the HUD's `LBL_QUEUE1`, TEXT = the key digit,
   ALIGNMENT 9 = top-left) to the live `override/mipc28x6_p.gui` (111 -> 121 controls). Defaults `--x 12 --y 498 --size 28 --spacing 30` (800x600 units;
   row spans x 12..310, just above the native bar's top y=532 and left of `LBL_COMBATBG3` x>=310; the Aspyr HUD is 1:1 and edge-anchored so this is
   bottom-left of the 3840x1600 screen, 2.667 px/unit -> ~75 px icons). **Row position lives only in this script.** `--dry-run`, `--rollback` (oldest
   backup, tested: sha restored), idempotent, `--replace`. **DEPLOYED 2026-09-29 21:42**, new sha `6ca8d518508d...` (was `39e04bdc2238...` = generator
   manifest `deployed_sha256`). Backups `~/Games/KOTOR2_Mod_Workspace/backups/hotbar_gui_*`. **Generator interaction:** the generator owns
   `mipc28x6_p` (HUD desc fix): run `--rollback` here -> generator -> re-apply.
2. Hook 3 `HotbarBuildButtons(panel)` at **0x00748502** (HUD ctor `FUN_00747210`, `MOV ECX,[EBP-0x5F4]` before `StopLoadFromLayout` call; bytes
   `8B 8D 0C FA FF FF` verified in the live exe; nearest other hooks 0x745A97 / 0x74B3F0). Per slot: `FnNew(0x148)`, memset, **`CSWGuiLabel` ctor
   `0x00419740`** (fastcall ECX=this, no args; base `0x00418990`, border ctor `0x00414A50` at +0x60, text ctor `0x004171A0` at +0xD8), verify vtable
   `0x009878BC`, `InitControl(panel, ctl, CExoString("LBL_HOTBARn"), 1, 1)`. `InitControl` -> `0x00418DF0` finds the GFF struct by TAG, calls the
   control's vt+0x48 Load (label Load `0x004198D0`: TEXT, BORDER, then base `0x00418F20` = EXTENT/ID); the control lands at `panel+0x24[ID]`
   (list padded with `0x0083EA60(0)`). **CORRECTED by review:** labels ARE hit-tested (panel hit-test `0x0040EAE0` walks from the highest index; label hit-test `0x00418D20` rejects only if bit1 clear or bit5 set), and ours have the highest indices, so the DLL sets `+0x48` bit5 (0x20) at build to make the row click-through. Verify hover/click-through live.
3. Drawing is free: `CSWGuiPanel::Draw 0x0040EC90` walks the list and calls vt+0x38 (label Draw `0x00419880` = border Draw `0x00415300` then text Draw)
   for controls with `+0x48` bit1 set (base ctor sets it). Border draws its FILL image (`border+0x74`, FILLSTYLE 2 = stretched) with alpha
   `border+0x24` only if border w/h (+0xC/+0x10) are nonzero. **The tick only** swaps the icon (`FnLoadImage 0x0047EB60` image pooled per resref, stored at
   `border+0x74`), sets the alpha (usable 1.0 / unusable x0.35) and toggles visibility (hidden when unbound, entry missing or the HUD gate is closed).
   The key digit is static GFF text; the item stack count is NOT shown yet (runtime text setter not researched).
4. Lifecycle: state is one `Bar` keyed by the HUD panel pointer, replaced on every HUD constructor call. Labels are leaked (the panel base dtor
   does not free list entries; same policy as Mod 1) so there are no dangling frees; the tick refuses to touch a bar whose panel != current hud.
   **Unverified live:** whether a new game/load makes a new HUD (the log line `H3 build:` per construction answers it).
5. Logging: `H3` lines for every allocation/ctor/InitControl result, extents, flags, IDs, panel list count before/after, list-slot check, memory dump of
   button 0; `btn[i]` lines for every visibility/icon/alpha change with reason; `img:` for image loads; heartbeat carries the bar state; `buttons:` line when the tick sees a HUD with no bar.

**Build:** `force-power-hotbar` 0.2.0-dev, warning-free (`-Wall -Wextra`), exports `CaptureHotbarKey HotbarBuildButtons HotbarHudTick` == hooks.toml,
host test `config_roundtrip_test` OK. Sources snapshot in `force_power_hotbar_src/`.

**Install status:** NOT installed. At 21:44 the game was running (another session's shield-probe test). Prepared stage `.../kotor-patch-manager/hotbar_stage2/`
(16 kpatches = the sibling's `shield_probe_stage2` + this 0.2.0-dev hotbar). Note that stage carries a newer in-flight `BuffDurationHUD` (dll `753a5778`)
than the installed one (`b44986b3`) -- confirm with that session before installing. Install with the game closed:
`cd .../kotor-patch-manager/bin/Debug && ./KPatchLauncher "$G/swkotor2.exe" --patches ../../hotbar_stage2 <16 ids>`; verify steam log
`Applied DETOUR` x3 for the hotbar (0x0072CB63, 0x0074B3F0, 0x00748502), DLL md5, `AR_ERROR.LOG` md5 unchanged.

**Live test checklist (Phase L):** buttons appear bottom-left above the action bar with digits + icons; usable/dim shading; hidden in menus/dialogue; new
game/load (log `H3 build:` again, no crash); calibrate position from a screenshot (rerun the GUI script with `--replace --x/--y/--size`); then digit
tests in dialogue/inventory/pause; only then `Invoke=1`.

**Phase F review (Opus, session 3): no crash-class defect.** Hook site, conventions, 0x148 size, lifecycle (HUD destroyed+recreated on load; panel dtor never touches list entries) all verified. Fixed: (1) wrong-ID path left a visible unmanaged label -> now hidden+click-through before the ID check, script refuses unless IDs start at 111; (2) hit-test claim corrected, bit5 set; (3) `--rollback` now picks the newest backup without LBL_HOTBAR*; (5) a partial build no longer disables bound buttons; (4) shared-image ownership invariant documented in code. Left as-is: dim only affects the border alpha (digit stays bright); `resolve()` re-reads groups per button.

## Live results, 0.2.0-dev buttons (2026-09-29 ~23:36, first run; installed via helper, game launched by the user)
- **Works.** Hook 3 fired once (`H3 build: panel=04912538 controls=111`), all 10 labels constructed/bound with IDs 111-120, panel list 111 -> 121, list slots verified. Rects in pixels: `[31+80i, 1328, 74, 74]` (= 12/42/.. units x 2.667, y 498 -> 1328), so the row is 74 px squares at the left edge above the bar, as designed.
- Screenshot: slot 1 shows the Force Speed icon (`ip_speedburst`, teal chevrons) with the digit `1` top-left, above the native action bar. Only Bottom1 is bound in the INI, other buttons correctly hidden (`unbound`).
- The button hides ("entry not available") when the controlled character lacks the bound action (companions have no Force Speed) and returns on switching back -- expected from the Default profile.
- Bound digit `1` was swallowed and resolved (`Invoke=0`). No crash across character switching; no load/new-game HUD rebuild has been exercised yet (only one `H3 build` in the log).
- Still to test: dialogue/inventory/pause digits, save/load (second `H3 build`), click-through/hover over the row, more bound slots + dimming, then `Invoke=1`.

## Bigger framed buttons + hover (2026-09-29 night, 0.2.0-dev build 2; installed, NOT yet run)
- Per slot now two labels: frame `LBL_HOTBARBG<d>` (IDs 111-120, FILL `hotbar_slot`, 42x42 units) and the icon+digit label `LBL_HOTBAR<d>` (IDs 121-130, 32x32, inset 5). Frames sit at x=12+46*i, y=484 (bottom edge 526, just above the native bar at 532; slots 7-10 overlap the combat-queue backdrop `LBL_COMBATBG3` x>=310, y>=498, drawn on top of it). The `.gui` script writes `override/hotbar_slot.tga` (teal) and `hotbar_slot_hi.tga` (beige, hover) -- generated rounded squares, plain alpha; `--rollback` deletes them. Options `--x --y --frame --inset --spacing`; GUI now 131 controls (sha `a243fc23...`).
- DLL: all 10 frames + digits show whenever the HUD is active (empty/unavailable slot = frame at 0.45 alpha, no icon); the icon label's fill is the action icon, dimmed x0.35 when unusable. **Hover**: the tick polls `GetCursorPos`/`ScreenToClient` (game window found via EnumWindows) against the frame's pixel rect, and swaps the frame fill to `hotbar_slot_hi` for available slots (labels stay click-through). Log: `hover:` lines (window client size, slot transitions with cursor coords) -- check them to calibrate if the highlight is off.
- Installed set: 16 patches, hotbar dll `49dbd5f5`, Buff HUD `feef453e` (sibling's newest), Inventory 3D `6a888b8a`; game not started.

## Hover tooltip (2026-09-30 00:00, 0.2.0-dev build 3; installed, NOT yet run)
- Request: delayed hover tooltip like the native portrait tooltip, with key, name, cost and other metadata.
- **Text setter found:** `FUN_00416E30` thiscall(ECX = `CSWGuiText+0x18` = label+0xD8+0x18 = **label+0xF0**, CExoString*) RET 4 -- assigns the string, resets layout (`+8=-1`) and pushes it to the font renderer (`text+0x14`, must be non-null: loaded from the GFF FONT). Found via the HUD's action-description path `FUN_0074CC90` -> `FUN_0074CF00` (which calls it on `hud+0xA75C`, text object at `hud+0xA744`). Same function also shows the entry stack count is `entry+0x34` (game formats `"%s (%d)"` when > 1). CSWGuiText vtable `0x009876B4`: slot 2 (`0x00417840`) = text height. Label SetRect = vtable slot 1 (`0x004199A0`, takes int[4] px rect, updates border+text).
- Tooltip = 2 more labels: `LBL_HOTBARTIPBG` (ID 131, copy of `LBL_MOULDING3`: the HUD's dark teal framed panel) + `LBL_HOTBARTIP` (ID 132, text centred, ALIGNMENT 18). GUI now 133 controls (sha `e28adb9d...`). The DLL, after the cursor rests on an available slot for 450 ms, builds text, sets it, sizes the panel (21 px/char, 32 px/line, 22 px pad: estimates for `dialogfont16x16`, calibrate from the `tip: show ... textHeight(engine)=` log lines) and places it above the hovered frame, clamped to the client area; hides on leave/gate close.
- Content: title (name), `Key: n   Force Power|Item|Mode`, powers: `Force Points` (spells.2da `forcepoints`), `Range` (P/T), wrapped description (spells.2da `spelldesc` -> `dialog.tlk`, first 5 lines of 38 chars); items: `Quantity`; `Not usable right now` when the entry's usable flag is 0. Sources read from disk by the DLL: `override/spells.2da` (binary V2.b, `TwoDA.h`) and `dialog.tlk` (`Tlk.h`); host test `twoda_test.cpp <spells.2da> <dialog.tlk>` (row 8 -> Burst of Speed, 25 FP, "Universal Power Restricted By Armor ..."). Forcepoints is the base cost (talent/feat reductions not applied). Item descriptions are not available (no item resref/base item yet).

## AutoFill (2026-09-30, 0.2.0-dev build 4; built + staged in `hotbar_stage2/`, NOT installed -- game was running)
`[General] AutoFill=1` (default on; `AutoFill=0` restores pure-INI behaviour). Each 8 frames `refreshEffective` builds the effective Bottom row: a slot's configured binding wins; every unbound slot gets, in order, the character's next known Force power from HUD group 0 (`key & 0x20000000`, no items/mode toggles) that is not already bound explicitly on the bar. The effective table (`g_eff`) drives the buttons, the digit swallow (`g_slotBound`) and the invoke path, so autofilled slots work like bound ones; per-character (recomputed on character switch and level-up). Log: `eff:` header + `BottomN = spell:X (auto)` lines on every change. Left/Right (Ctrl/Alt) rows are unaffected.

## Build 5 (2026-09-30): live findings from build 3 + fixes (built + staged, NOT installed -- game running)
- **Live (build 3):** hover highlight and the delayed tooltip work (screenshot: title, `Key: 1   Force Power`, Force Points, Range, description; engine text height 288 px for 9 lines = 32 px/line, so `TipLineH=32` is right and the estimated 21 px/char fits).
- **Bug: slots 8 and 0 stretched** (rect w=964/965 px instead of 111). Cause: `InitControl` applies a non-linear aspect projection to extents while the global `0x00A1B9F0` is nonzero; the HUD ctor clears it right after our hook (`0x0074850D`). Fix: clear it around our binds and restore (logged: `H3: projection flag ...`).
- **Hotkey did nothing** because `Invoke=0` in the INI (log only) -- set to `Invoke=1` at 00:1x (hot-reload; digit-in-dialogue/inventory/pause still untested). **Click did nothing** because v1 had no click: now a left-click edge (`GetAsyncKeyState`) on an available slot queues the same request as the digit; shown frames are hit-testable (bit5 cleared) so clicks on the bar don't reach the world. Log: `click:` lines.
- Tooltip description shortened: header dropped (text before " This "), first sentence only, 3 lines.

## Build 6 (2026-09-30 00:20): aspect fix + live tooltip refresh (built + staged, NOT installed -- game running)
- **Live (build 5):** click works, auto-fill works (slots 1-4 filled with the 4 powers), slots 8/0 no longer wild -- but every frame came out ~1.8x too wide (frame ~200x110 px instead of 111x111). Cause: with the projection flag `0x00A1B9F0` cleared, `InitControl` scales x by W/800 and y by H/600 (= the 1.8 ultrawide stretch); the projection path (flag set) normally undoes that, except it misbehaved for slots 8/0. Fix: keep the flag cleared and correct after binding: `x' = x*c, w' = w*c` with `c = (H/600)/(W/800)` from the engine's own screen info (`FnScreenInfo 0x0073FEA0` -> shorts at +0x6C/+0x6E, ref size ints at `0x009F4390/94`), applied through the label's SetRect (vt slot 1). Logged: `H3: screen info ...` and `H3 <tag>: aspect fix rect [..] -> [..]` per control (expect x 31->17, w 111->62 before/after... i.e. new rect ~= 2.667 px/unit).
- **Tooltip live refresh:** while a tooltip is shown, each 8-frame refresh rebuilds its text (`buildTipText(..., false)`) and calls `showTip` again if it differs (e.g. `Not usable right now` disappears when the power becomes usable); log `tip: slot N entry changed while hovered, refreshing`.


## Live results 2026-09-30 (builds 3-5) and lessons

- Screenshots confirmed: hover highlight (frame swaps to beige `hotbar_slot_hi`), tooltip (title, `Key: 1   Force Power`, Force Points, Range, wrapped
  description; engine text height 288 px for 9 lines -> 32 px/line), auto-fill slots 1-4 = Heal, Burst of Speed... in the game's group-0 order, click and key
  both invoke (`invoke: returned group=0 key=2000000a ...`; the game's own selection state may be left changed, invoke only restores it if unchanged).
- **Lesson (InitControl projection):** while the global `0x00A1B9F0` is nonzero (set for the whole HUD ctor) `InitControl` runs a non-linear aspect
  projection that gave slots 8 and 0 a ~360-unit width. Clearing it gives linear scaling W/800 x H/600, i.e. x stretched 1.8x on 3840x1600; fix = correct
  x/width by `(H/600)/(W/800)` after each bind (build 6). If the correction looks off, read the `H3: screen info` and `aspect fix rect` log lines.
- **Lesson (hit-testing):** labels are hit-tested by the panel (bit5 of `+0x48` disables it); bg frames are made hit-testable while shown so clicks on the
  bar do not walk the character; everything else (icon, tooltip) is click-through.
- **Lesson (Invoke was 0):** "hotkey does nothing" was the INI's `Invoke=0`; the INI is hot-reloaded, `Invoke=1` is now set.
- **Lesson (multi-session installs):** see the install workflow in the STATUS block.


## Build 7 (2026-09-30 ~01:20) -- fixes from the first live look at build 6
- **Live (build 6):** frames still ~1.8x too wide, icons stretched. Log: `H3: screen info obj=00000000 size=0x0` -- the engine screen-info object (`FnScreenInfo`) is NULL during the HUD ctor, so `aspectFix()` returned 1.0 (no correction). Fix: fall back to the game window's client size (`EnumWindows` + `GetClientRect`, 3840x1600 -> c = 0.5556); logged as `H3: screen info null, window ...`.
- **Live (build 6):** slots 5-7 empty although stim/shield/mine exist. Cause: AutoFill only used group-0 Force powers. Fix: after the powers, AutoFill appends the *selected* entry (`hud+Hud_GroupSel`, else first) of each item group 1-3 (medpac/stim, shield, mine), de-duplicated; logged as `eff: autofill group N pick ...`.
- **User design input:** put the edit toggle at the very LEFT of the bar (before slot 1) rather than to the right; bar/frames should better match the native quick-button style (size + border look) -- revisit after the aspect fix is seen live.
- Installed 18 ids (incl. sibling's `equipment-3d-character`), set rebuilt from installed DLL md5s in `hotbar_stage3/`; backup `backups/pre_hotbar_b7_install/`.

## Build 8 / 0.3.0-dev (2026-09-30 ~01:45): initial in-game edit UX (built + GUI deployed, DLL NOT yet installed/tested)
- **Flow:** `Edit` toggle (far left, x=12) -> Edit mode (title "click a hotbar slot") -> click a slot -> Pick mode: title, tabs **Powers / Items / Modes**, `<` `>` paging, `Clear`, 8 cells (frame + icon, hover frame swap + tooltip, same widget pattern as the bar) built from HUD groups 0-5 (tab by key bits: 0x40000000 item, 0x20000000 power, else mode) -> click a cell = assign to the slot, save, back to Edit. Esc backs out one level (input hook swallows Esc only while the editor is open); digits pass through and never invoke in Edit/Pick; clicking bar slots does not invoke in Edit/Pick. Closes on gate close / character change.
- **Config:** new explicit `none` (`ActionRef.blank`, keeps `empty()` true; blocks the Default fallback and AutoFill), `Config::SetSlot`. Edits always go to `[Profile.Default]` (character key unstable, see review amendment 1 -> applies to every party member). First edit freezes currently autofilled slots as explicit; unset slots still autofill. Save refuses while the INI has parse warnings; one `.ini.bak` per session; `checkConfig(true)` before writing; mtime recorded after Save.
- **GUI script:** bar moved to x=58 (toggle at 12); 26 new labels IDs 131-154; tooltip pair moved to the last IDs 155/156 (draws above the picker). DLL enables the editor only if all 24 editor labels bind (log `H3: editor controls bound N/24`). Deploying the new .gui under the old DLL is harmless (tooltip label id mismatch -> hidden).
- Logging: `edit: ...` for every mode change, tab/page/cell/hover, visibility/fill/alpha/text change, assign (with freeze count), refusals.
- Stage: `hotbar_stage3/` (18 ids incl. sibling `equipment-3d-character`). Host test extended (`none`, SetSlot). Not done yet: per-character profiles, item ref beyond name, mouse wheel, arrow art, native-style look, review agent (Phase F).

### Build 9 (2026-09-30 ~02:00): crash fix -- LESSON: FUN_0073FEA0 is a THISCALL
- Build 8 crashed ~1 s after a load screen: null read at 0x73FEAF, return address = our DLL. `aspectFix()` called `FnScreenInfo` (0x0073FEA0) as cdecl; the function reads `[this+0x274]` with an implicit ECX we never set (the decompiler hid it). It "worked" in builds 6-7 only because ECX happened to be a harmless value (log `obj=00000000`). Fix: removed the call entirely; the aspect factor comes only from the game window client size. General rule (again): confirm the calling convention in raw disassembly before calling any engine function from a hook.

### Build 10 (2026-09-30 ~02:30): polish from the first live look at the picker (built, NOT yet installed)
- Toggle is now a thin wrench button (24x42 units, generated `hotbar_edit.tga` / `hotbar_edit_hi.tga`, 72x128); bar shifted to x=42. Group background `LBL_HOTBARPBG` (copy of the native quick-command panel `LBL_MOULDING3`) behind title/tabs/cells; compact (title only) in Edit mode, full in Pick mode (DLL `placePanel`, sizes in units must match the script). Small font `dialogfont10x10` for title/tabs; shorter title strings ("Click a slot to change it", "Slot 8 - Item 1/2 - Esc: back").
- Mouse wheel over the picker pages the list (window subclass like Mod 1, chains to the old proc, only consumes the wheel inside the picker rect while the editor is open). Log: `wheel: ...`, `edit: wheel acc=...`.
- IDs now: edit 131, pbg 132, title 133, tabs 134-136, prev/next/clear 137-139, cell frames 140-147, cell icons 148-155, tooltip 156/157. GUI + DLL must be deployed together (old DLL + new .gui: editor disabled, harmless).

### Build 11 (2026-09-30 ~02:20): wrench polish (built, not installed)
- Wrench art now shaded like the native force-power icons (mint->teal vertical gradient, dark outline, upper-left rim light) and ~20% smaller; toggle gets a hover tooltip ("Edit hotbar" / "Finish editing", tooltip owner id 200) via the new `showTipText()` (`showTipAt` is now a wrapper that builds the entry text). Textures are regenerated by `add_hotbar_buttons_gui.py --replace`.

### Build 12 (2026-09-30 ~02:45): live feedback on build 11 (built, not installed)
- **Bug:** Clear saved `none` correctly (log `assign ... := none ... Save ok`) but `refreshEffective` still AutoFilled blank slots (missed the planned blank check) -> fixed (`!a.blank`).
- **Start empty + per character:** `AutoFill` now defaults to 0; edits are saved to the controlled character's own profile `[Profile.obj<id>]` (falls back to Default for unset slots); the invalid object id `objffffffff` (seen transiently during loads) now yields an empty key = "unknown" and saves are refused until a real key exists. Companion keys are runtime object ids: watch whether they survive a save/load (fallback idea: portrait resref).
- **Hover:** tabs / `<` `>` / Clear highlight on hover and show short tooltips; all picker tooltips are anchored above the whole picker (they used to cover the title/tabs). Wheel paging confirmed live (`wheel: ... inside=1`).
- **Unassigned slots hidden in normal play** (frames + digits), shown in Edit/Pick so they can be filled.

## Open item: test coverage (noted 2026-09-30, user undecided -- no work started)
- **Exists:** `force_power_hotbar_src/config_roundtrip_test.cpp` (~17 asserts: INI parse/serialize/save round-trip, `none`, `SetSlot`, slot fallback, key names; build `g++ -std=c++17 config_roundtrip_test.cpp`) and `twoda_test.cpp` (smoke test only: prints `spells.2da`/`dialog.tlk` values, needs the game files, no content asserts). Neither is run by any script/gate.
- **Untested:** all of `ForcePowerHotbar.cpp` (~1100 lines, Win32/engine-bound): editor state machine, `tabOf`, paging/clamp, picker list build + dedupe, `assignSlot` (freeze/blank/refuse-on-warnings), effective-slot resolution with `none`, tooltip helpers (`wrapText`, `shortDesc`, `buildTipText`); `Tlk.h` output; `scripts/add_hotbar_buttons_gui.py`; and DLL<->GUI-script ID agreement (hard-coded IDs vs `ALL_TAGS` order; a mismatch already bit us once).
- **Options when picked up:** (1) extract the pure logic from the DLL into a Win32-free header (e.g. `HotbarLogic.h`) and host-test it; (2) give `TwoDA`/`Tlk` fixture-based asserts instead of needing game files; (3) a Python test that parses the DLL ID constants and the script's `ALL_TAGS` and fails on mismatch; (4) a single `run_tests.sh` run before each build. Best done after the current live-test round, since the DLL is still changing.

### Layout fix (2026-09-30 ~02:35)
- Bar + toggle + picker moved up 34 units (`DEFAULT_Y` 484 -> 450 in `add_hotbar_buttons_gui.py`): the bar (y 484-526) overlapped the native quick-action description label ("Energy Shield (Self)", y ~496-528). New bar span y 450-492. GUI-only change (deploy with `--replace`); the DLL derives everything from the bound rects.

### Wrench art revision (2026-09-30 ~03:05, committed, NOT yet deployed)
- Jaw is a U (parallel-sided slot, rounded bottom). Added native-icon cues: soft teal outer glow, dark outline, lower-right shading, specular streak along the handle + glint on the jaw, hanging hole in the handle end. Texture-only (`add_hotbar_buttons_gui.py --replace` rewrites `hotbar_edit*.tga`); no DLL change.

### Wrench art revision 3 (2026-09-30 ~03:30)
- Reworked from a reference photo of a real spanner: recessed handle channel (raised rim, darker inset, inner shadow top-left / light lip bottom-right), round beveled hanging ring drawn separately from the directional shading (the old hole looked triangular because rim/shade were computed on a mask with the hole cut out), rim streak on the raised edge. Texture-only.

## Next phase: in-game configuration GUI -- plan, decisions, amendments (written into the doc 2026-09-30 for the record; mostly DONE, see STATUS)
- **User decisions:** config UX = a small edit toggle beside the bar (now a thin wrench button at the far left); while editing, clicking a slot opens a picker
  (clicks do not invoke); the picker offers everything the character can use per character (powers, item abilities, modes) from the HUD group lists
  (`hud+0x78+g*0xC`, stride 0x3C, name +0, key +8, inline icon resref +0x20, flags +0x30 bit0 usable); saved per character profile through `hb::Config::Save`.
  Look = a slimmed native-style quick-button strip using the same framed-cell widget as the bar. Out of scope: Ctrl/Alt rows and 30 slots, target-menu powers, cooldowns.
  Later user requests (all done): thin wrench with shaded force-power-style art (U jaw, recessed handle, round hole), grouping background like the quick-command panel,
  wheel paging over the picker, smaller fonts, hover/tooltips on every control, hide unassigned slots outside Edit mode, per-character sets starting empty, move the bar above the quick-action label.
- **Independent-review amendments (2026-09-30) and status:** (1) character key not stable even for the Exile -> `objffffffff` treated as unknown, saves refused until a real key (DONE);
  companion keys still unverified. (2) tooltip pair must be the last IDs (DONE). (3) wheel via window subclass, Esc via the DirectInput hook (DONE). (4) entry+0x34 is not the stack
  count; modes are 0x410-0x414 (in DB lessons). (5) config model gaps: explicit `none` (DONE, blank flag), refuse Save while `warnings` non-empty (DONE), mtime after Save +
  `checkConfig(true)` before write (DONE); NOT done: preserving unknown keys/sections and free comments on Save (only refused). (6) AutoFill reshuffle during editing (mitigated:
  AutoFill off by default; first edit freezes autofilled slots when it is on). (7) state machine: picker state lives in `Bar` (reset on HUD rebuild), closes on gate close / character change,
  digits pass through in Edit/Pick (DONE); a few function-level statics (`lastOth`, `lastChar`, hover timers) still survive a HUD rebuild (only cause stale log lines). (8) control count: ONE row of 8 cells + 3
  tabs (DONE, 25 editor labels); a runtime-positioned generic cell pool was not needed. Arrow art: text `<` `>` for now. (9) toggle placement collision solved by moving it left and the bar up. (10) InitControl id-check logged.
- **Not done from the plan:** Phase R (R1 item resref identity, R2 companion key research, R5 pause), Phase F review, DB `functions` rows for new addresses (none were needed; lessons added), host tests for the DLL logic.

## Agy review follow-up (2026-09-30, build 13, built not installed)
- FIXED: stuck Ctrl/Alt/Shift now resynced from `GetAsyncKeyState` on each digit press (only clears stale flags, logs `cap: stuck modifier cleared`).
- FIXED: a bound key is always swallowed even when the 16-entry ring is full (request dropped and logged) so keys 1-3 never fall through to party switching.
- FIXED: DLL detach restores the WndProc only if `wheelProc` is still on top (same guard as Mods 1/5); otherwise it is left chained.
- OPEN (needs research, not guessed): companion profile keys `obj8000024x` are runtime ids. Agy suggests keying by CSWPartyTable roster index (0=Atton ... 11=G0-T0); the accessor/offsets are unverified. Fallback is HUD portrait resref. Ties to Phase R2; verify live by checking `character:` log lines across two saves first.
- Research result (build 14, built not installed): `PartyTable::GetMember(idx)` (0x007E5DA0) indexes party POSITIONS (stride 0x88), not the 12-entry NPC roster, so roster indices are not readable through it. Instead `CSWCObject::GetPortraitId` (0x007ED4B0) = `*(ushort*)(obj+0xB8)` (portraits.2da row, fixed per character). `computeCharacterKey` now returns `port<N>` for non-Exile characters and logs `character: oid=.. portraitId(+0xB8)=..`. LIVE CHECK NEEDED: value is sane (nonzero, <0x1000, differs per companion) and identical after save/load. Falls back to `obj<id>` if unreadable. Old `obj8000xxxx` profiles are orphaned (INI is currently empty).

## Design round 2 (2026-09-30, GUI only, deployed, not yet seen live)
Measured from the user's screenshot (icon art medpac: native 52 px vs ours 55 px; native rim ~5 px and (0,47,27), ours ~8 px and (21,123,103)). Changes in `add_hotbar_buttons_gui.py`: frame 42->40 units (same 5-unit inset, so icon 32->30 = native size), spacing 46->44, DEFAULT_Y 450->452 (bottom stays 492), rim 9/128->6/128, frame colour (18,110,85)->(0,44,26) (renders ~ native). Applies to bar, wrench and picker cells. No DLL change. Revert: `--replace` with old constants (git history).

## Build 16 (2026-10-01, built + GUI script ready, NOT deployed: GUI and DLL must go in together)
Key digits moved to their own labels `LBL_HOTBARNUM1..0` (IDs 156-165, new total 168, tip pair now 166/167, `NumId0` in the DLL) so they sit 3 units left / 4 up of the icon corner instead of on the icon label. The installed DLL at this point is still build 12 (md5 307a00af); builds 13 (modifier resync, ring-full swallow, WndProc detach), 14 (companion key `port<id>`), 15 (edit toggle hover highlight) and 16 are staged-by-build only. Deploying the new GUI with an older DLL misbinds the tooltip IDs.

## Build 17 (2026-10-01): hover sound
`playHoverSound()` calls the engine GUI sound `FUN_004122A0` (ECX = `*(void**)0x00A1B49C`, idx 1 = native control mouse-enter) on hover-enter of hotbar slots (when assigned or in edit mode), picker cells/controls and the edit toggle. Logged as `sound: hover (...)` (first 40). Built, NOT installed; check live that it is the same sound as the quick bar.

## Live results, build 17 session (2026-10-01)
- Hover sound: `sound: hover (slot|toggle)` logged, manager pointer valid -> the engine call runs (user to confirm it is the native sound).
- Portrait-id key FAILED: `portraitId(+0xB8)` = 65535 (0xFFFF) for the Exile, 80000249 and 80000240 -> the `port<id>` path never triggers, keys stay `obj<id>`. `CSWCObject::GetPortraitId` is not populated on client creatures (or +0xB8 is the wrong field for this class). The same ids 80000240/80000249 did appear in earlier game sessions' logs (same save), so they may be stable per save; across different saves/parties still unverified.
- Next candidates for a stable key: (1) server creature tag via the id -> server object lookup; (2) creature name/template resref strings seen in the object scan are item names, not identity; (3) a live test: save, reload, switch to the same companion and compare `character:` ids.
- Editor: no `edit: assign` lines this session (no assignment made); the INI holds only the Exile profile (Bottom1=Heal, Bottom2=Burst of Speed, Bottom0=none).

## Build 18 (2026-10-01): character-switch lag
Symptom: after switching character, the native buttons change first, then ours flash empty frames / old content before updating. Causes: the character key was polled every 15 frames, `g_eff` refreshed every 8, and unresolved slots (HUD action lists lag behind the switch) were drawn as empty dim frames. Fix: key polled every frame; on change `refreshEffective` runs immediately and `g_charChangeTick` opens a 1500 ms settle window during which button state is recomputed every frame and unresolved assigned slots stay HIDDEN (Normal mode only) instead of showing empty frames. Log reason: `settling after character switch`. Built, not installed, not seen live.

## Design round 3 (2026-10-02, GUI only, deployed, not seen live)
Measured from the user's screenshot (3840x1600, 2.667 px/unit): native quick-action panel border at x=10 px (~4 u), button pill width 93 px (~35 u), pitch 106 px (~40 u), description band y 224-256 with the panel border directly below, panel-border-to-button gap 9 px. Changes: frames 40->35 wide/high (native pill width), spacing 44->40, inset 5->2 (icon label 31 u, icon art stays native size), bar X 42->29 (5 u gap after the wrench, like native), bar Y 452->478 so the bar bottom (513 u = 1368 px) sits 9 px above the description band like the native panel-border gap, wrench X 12->4 (aligned with the panel border) and 24->20 wide, wrench rim colour (0,44,26)->(10,96,80) (the lighter native panel-border colour, differs from slot frames), digits 2 u further right (x = frame+4). Picker cell rows moved to py+58 to fit the 35 u cells. No DLL change; revert by restoring the old constants and `--replace`.

## Build 19 + design round 3b (2026-10-02, built/staged, NOT deployed - game running)
- BUG (user, Ebon Hawk, only the Exile in the party): picker showed "Character not identified yet - try again"; log had `cfg: profile for character ''` from frame 1 and NO `character:` line for the whole session, 7x `edit: assign REFUSED`. Cause: the controlled creature's object id stayed invalid (0xFFFFFFFF) for the entire session (earlier sessions only saw it transiently at loads), so `computeCharacterKey` kept returning "". Fix: log `character: creature .. has invalid object id` (rate-limited) and, when the id is 0xFFFFFFFF and the party has exactly ONE member (always the player character), return `objfffffffd` (the Exile's valid key). Multi-member parties with an invalid id still stay unidentified. The per-frame skip check now includes the party member count.
- Picker box now spans from the wrench's left edge (x=4) to the right edge of slot 0 (x=424) = width of the 10 buttons; content padding 8, cells centred between the arrows. GUI IDs unchanged (deploy with `--replace`; DLL install not required for the layout, required for the key fix).

## Build 20 (2026-10-02): smooth scrolling in the picker instead of pages
`Editor.page` became `Editor.off` (index of the first visible entry). Mouse wheel over the picker scrolls ONE entry per notch (up = toward the start), the `<` / `>` cells scroll one entry per click and keep scrolling while held (first repeat 350 ms, then every 90 ms), arrows dim at the ends, the title reads `Slot 3 - Powers 2-9 of 12 - Esc: back`. Offset resets on tab change / picker open and is clamped when the list shrinks. Log lines: `edit: wheel ... offset a -> b`, `edit: scroll -> n`. Built, not installed, not seen live. No GUI change.

## 1.0.0 finalization, Phase R (2026-10-02): runtime controls + textures from DLL memory -- Gate R PASSED (static)
Goal: the `.kpatch` alone delivers the mod (no `.gui` edit, no override TGAs). Evidence: decompiles in this session's scratchpad, DB lesson 283 + function rows
`0040f620 00418df0 0083ea60 00418f20 00416fa0 00416de0 00423000 004260e0 00421c40 00423990 004491c0`.
- **Label recipe.** `InitControl` (0x0040F620) = `00418df0` (parent `ctl+0x38 = panel`, find TAG in the open layout, `ctl->vt[0x48]` Load) + extent scale
  (W/800, H/600; projection if 0x00A1B9F0) + `vt[1]` SetRect + **only when `addToList`**: `controls[id] = ctl` (padding via `CExoArrayList::Add` 0x0083EA60).
  So: new 0x148, ctor 0x00419740, `InitControl(panel, ctl, "LBL_QUEUE1" | "LBL_MOULDING3", addToList=0, scale=1)` (copies a native template, no list slot
  touched), then `ctl+0x54 = index`, `vt[1]` SetRect with our rect, `0x0083EA60(ECX = panel+0x24, ctl)` to append. Text: `SetFont` 0x00416DE0
  (ECX = label+0xF0, CResRef char[16]*), `SetText` 0x00416E30, `SetAlignment` 0x00416FA0 (uint, RET 4); fill = pooled `FnLoadImage` at border+0x74.
  Templates: LBL_QUEUE1 = blank type-4 label (FILLSTYLE 2, white dialogfont16x16, align 9), LBL_MOULDING3 = native framed panel (uibit_fill_2bt + 16bet/16bct).
  Our 57 controls differ from them only in fill / text / font / alignment. If the template tag is missing, Load is never called (detected via an ID sentinel).
- **IDs become relative** (`base = panel control count at the hook`, after every native bind): the ID constants were only used in the build function.
- **Unbound layout entries are inert**: Draw/hit-test walk only the panel list; nothing instantiates a GFF struct unless a ctor asks for its tag.
  A leftover script-edited `.gui` cannot conflict (we roll our own live one back once anyway).
- **Texture recipe (T-A2, NormCubeMap-style pre-registration).** new 0xF4 + `CAurTextureBasic` ctor 0x00423000(name, name) self-registers by name
  (sorted array 0xA1BC60, case-insensitive), so every later `FnLoadImage(name)` reuses it. Fill: `+0x44` = engine-malloc (0x0091D288) RGBA buffer
  (TGA BGRA -> RGBA, rows as stored, like `CResTGA::OnServiced` 0x009454B0), `+0x68` w, `+0x6C` h, `+0x74 = 4`, `+0x78 = 0`, `+0x5C = 0`, `+0xE7 = 0`
  (buffer freed with engine `free`), `+0xE9 = 0`, `+0xDF = 0` (cube flag), `+0xE4 = 1` (constructed: no resman load); push on the pending-build list
  0xA1BC2C (0x004491C0); the texture tick uploads via the normal 2D path and frees the buffer. Never set `+0xE6` (NormCubeMap-only regen).
  Hazard: Reset (video mode / texture quality / GL init) clears `+0xE4` -> the DLL re-fills from the embedded TGA in its HUD tick (self-heal).
  Rejected: `AddResourceImageFile` (0x007115E0 = RIM from disk), resman key/CRes injection (more writes, rebuild semantics unknown).
- Embedded resrefs are renamed `fph_slot`, `fph_slot_hi`, `fph_edit`, `fph_edit_hi` (never collide with the old override TGAs).
- **Prototype live result (2026-10-02 ~01:55):** wrench created at runtime (index 131, padded slot) and all four `fph_*` textures uploaded by the engine's texture tick
  (gl names 4166-4169 at frame 120, pixel buffers freed by the engine, no missing flag); user: "The wrench button looks great!". Texture self-heal after a GL reset is still untested.
