# Mod Idea 3: Force Power Hotbar (1-0 keybound, customizable)

**STATUS (2026-09-29, end of session 2) -- READ THIS FIRST.**
- **Scope changed (user decision):** focus is now the **UX: ten on-screen buttons along the bottom of the screen, bound to `1`-`0`**, like
  the original plan. The Ctrl (left) and Alt (right) rows and 30 total slots are a **future iteration** -- do not build them yet.
  (Alt+digit does not reach the game under Proton anyway, see "Live results".)
- **Done and working live (prototype `ForcePowerHotbar` 0.1.0-dev, installed as the 16th patch, `Invoke=0` = log only):** key capture + swallow
  of bound digits (press and release), HUD gating, INI config + hot reload, per-character profile plumbing, resolver from an `ActionRef` to
  the HUD's own action entries, and exhaustive probe logging. The **invoke path (`Invoke=1`) has never been run live.**
- **UPDATE (session 3, same day): bottom-bar UX researched, GUI deployed, DLL 0.2.0-dev BUILT but NOT yet installed/tested live** (the game was
  running for another session's test). See the last section "Bottom-bar UX: design, build (session 3)". Install = stage `hotbar_stage2/`
  (see that section), then Phase L test.
- ~~**NEXT PHASE (not started): investigate and build the on-screen button bar**~~ (done, see above) -- adding brand-new UI elements to the HUD is new territory
  for this project (Buff HUD only reused existing labels). See "Next phase: bottom-bar UX" at the end of this file.
- Sources: `force_power_hotbar_src/` (committed). Framework clone (uncommitted, source of truth for builds):
  `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/ForcePowerHotbar/`. Live evidence excerpts: `force_power_hotbar_src/live_evidence/`.
- Sections below: original spec (partly SUPERSEDED, kept for history), then "Next-phase findings" (Ghidra), "Live results", "Environment and workflow
  gotchas", "Next phase: bottom-bar UX".

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
   (list padded with `0x0083EA60(0)`). Labels are not hit-tested (ctor clears `+0x48` bit2), so mouse/hover are untouched.
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
