# Mod Idea 3: Force Power Hotbar (1-0 keybound, customizable)

**Status (2026-09-29, updated with Mod 1 & Mod 2 lessons; UX redesigned
same day to a 3-edge/30-generic-slot layout):** Pre-Ghidra research and
architectural specification. Informed by the completed Phase 2/3 static
analysis of [[02_buff_duration_hud.md]] and the render-pipeline resolution
of [[01_inventory_3d_viewport.md]]. Key framework addresses have already
been mined from the shipped `AddressDatabases/*.db` (including
`CSWSCreatureStats::HasSpell` at Steam `0x006BD9C0`), the persistence model has
been settled (external config, not savegames), and the input context-gating
hazard has been identified and designed. **UX redesign (user-directed,
2026-09-29, same session as Buff Duration HUD's item-ability discovery,
superseding the earlier "2nd row for items" framing): 30 total GENERIC
slots across three screen edges** — bottom (bare `1`-`0`), left
(`Ctrl+1`-`Ctrl+0`), right (`Alt+1`-`Alt+0`) — every slot independently
assignable to ANY single vanilla quick-action (a power OR an item OR
anything else the native quickbar itself can hold), not pre-restricted by
which edge/row it's on. See "Slot layout (3 edges, 30 generic slots)"
section below — this shares its icon-resolution problem directly with
[[02_buff_duration_hud.md]]'s open "Item 5" research item (both need to
know how the engine's own quickbar picks an icon for an item-ability
quick-slot, since `spells.2da` has no icon for item-ability rows). **Update
(2026-09-29, Buff Duration HUD's 9th session ran this shared research):
the premise was corrected, not resolved as hoped** -- the vanilla
action-bar's 6 categories never include "item," so item-slot invocation
and icon resolution both remain genuinely open, not just "reuse the
quickbar's mechanism." See "Slot layout" section's icon-resolution note
below for the full finding. Phase 1 (Ghidra mapping of invocation & input
hooks), Phase 2 (scaffold C++), and Phase 3 (live install) will proceed
once Mod 2 finishes development or when this mod is prioritized.

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
