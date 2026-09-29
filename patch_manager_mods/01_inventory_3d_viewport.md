# Mod Idea 1: Inventory 3D Viewport

**Status:** Phase 2 (Ghidra static analysis) IN PROGRESS, started 2026-09-29.
Real class names confirmed via RTTI for every class in both branches; exact
addresses for the trigger/render call functions not yet pinned down. No
hooks written, no `.kpatch` built.

## Phase 2 findings (2026-09-29)

**Methodology shortcut found — check this FIRST for any future mod using
this framework**: the framework's `AddressDatabases/kotor2_gog_aspyr.db`
already has fully-mapped function tables (real names, addresses, calling
conventions) for every class this mod needs — `CSWGuiUpgrade`,
`CSWGuiUpgradeSelection`, `CSWGuiUpgradeItemSelect`, `CSWGuiInGameInventory`,
`CSWUpgradeItemEntry`, `CSWItem`, `CSWCItem`, `CSWGuiScene`, `CAurScene`/
`Scene`, `Gob`, `CSWBaseItem` — none of which any *shipped* GOG-port
candidate actually uses. There's no GOG binary on this machine to
byte-diff against, but the class/method **names** and relative shape are
still a massive head start: confirm the same RTTI class exists on our
Steam binary (fast), then use GOG's named functions as a hypothesis list
for what a given Steam vtable slot does, verified by decompile — never
assumed from name/position alone (see the `lessons` table for a case where
that assumption was wrong). Query every `AddressDatabases/*.db`, not just
the versions being actively ported, before starting fresh RTTI work.

**Real class names confirmed via RTTI on our Steam binary (overturning
every SDK-style guess in this doc's earlier drafts):**
- `CSWGuiInGameInventory` (vftable `0x009a72ac`) — the real inventory
  screen class. **Not `CUIInventory`.**
- `CSWGuiUpgrade` (`0x009a88a4`), `CSWGuiUpgradeSelection` (`0x009a86cc`),
  `CSWGuiUpgradeItemSelect` (`0x009a876c`) — the three real Workbench
  screen classes (main panel / category-filter sub-screen / item-to-upgrade
  sub-screen). Per GOG, `CSWGuiUpgrade` owns `Update3DView`,
  `GetArmorModelAndTextureResRef`, `OnSlotSelected`, `OnUpgradeSelected`,
  `VerifyUpgrade`, `InsertUpgrade` as real named member functions —
  **`GetArmorModelAndTextureResRef` existing directly on the Workbench
  class is a significant simplification of the Branch A/B split below**:
  the Workbench itself already resolves armor model+texture together, not
  just standalone weapon models.
- `CSWUpgradeItemEntry` (`0x009a899c`) — per-slot item-entry widget.
- `CSWItem` (`0x0099a8a0`) — UI-side item wrapper (`GetBaseItem()` per
  GOG). `CSWCItem` (two RTTI hits, `0x009a25fc` and `0x009a2748`,
  relationship not yet resolved) — the in-world/rendered item object
  (`LoadModel`, `SpawnVisualEffect`, `PowerItem`, etc. per GOG).
- `CSWGuiScene` (`0x0098771c`) — the scene embedded in a `CONTROL_3DVIEW`.
  **Confirmed render-call target**: GOG has
  `CSWGuiScene::AddModel(CExoString* model, int insertIndex)` — takes a
  model **resref string directly**. Only a 5-slot vtable (mostly
  `CSWGuiObject` base overrides) — `AddModel` is very likely a plain
  (non-virtual) member function, so it won't show up in a vtable dump;
  needs a direct xref/string-anchor search on Steam, not attempted yet.
- `CSWGui3DSceneView` (`0x009a3d34`) — the `CONTROL_3DVIEW` control itself
  (wraps a `CSWGuiScene`, per the framework's own `CSWGui3DSceneView.h`).
- `CAurScene` (`0x0098bc7c`, mostly `__purecall` — an abstract interface)
  and `Gob` (`0x0098b5cc`, 60-slot vtable — the generic renderable-object
  class `AddModel` returns) — both confirmed present, not yet needed at
  function level.

**Vtable-shape finding**: `CSWGuiUpgrade`/`CSWGuiUpgradeSelection`/
`CSWGuiUpgradeItemSelect`/`CSWGuiInGameInventory` all share an *identical*
29-slot vtable shape (common GUI-panel base class), diverging only at
slots 13/15/18/19 — each class's own overrides. On `CSWGuiUpgrade`
specifically:
- **Slot 13** (`FUN_008cce70`, `(this, float dt)`) — confirmed per-frame
  `Update`. Gated by a flag at instance `+0x3d2c`; when set, computes
  `-dt * (a constant)` — consistent with animating the previewed item's
  spin/rotation in the 3D view every frame.
- **Slot 19** (`FUN_008cca70`) — likely `OnPanelRemoved`/teardown: zeroes
  the same `+0x3d2c` flag and frees a handle at `+0x3d38` (likely the
  preview item/model handle).
- **Slot 15** (`FUN_008cfc10`, `(this, int, int)`) — likely
  `OnControlEntered`, toggles two related UI control states.
- **Slot 18** (`FUN_008cbde0`, 3215 bytes, the largest override) — **ruled
  out as `HandleInputEvent`** despite being the biggest function (no
  switch/case shape in the decompile). Instead resolves `baseitems.2da`
  model-variant columns by literal name (`"DoubleMdlVar"`,
  `"ShortMdlVar"`, `"LongMdlVar"`, `"UpgradeType"`, `"Template"`) —
  real, useful confirmation that this class does exactly the kind of
  `baseitems.2da`-driven model resolution Branch A needs, but not yet
  matched to one specific GOG-named method with confidence.

**Not yet resolved (concrete next steps for whoever resumes)**:
- Exact Steam addresses for `CSWGuiUpgrade::Update3DView`,
  `GetArmorModelAndTextureResRef`, `OnSlotSelected`, `OnUpgradeSelected` —
  not yet matched from GOG's named list to specific Steam addresses.
  Recommended next move: string-anchor search for the literal 2DA column
  names found in `FUN_008cbde0` (`"DoubleMdlVar"` etc.) and for
  `"UpgradeType"`/`"Template"`, following each into its containing
  function, same technique that has worked repeatedly on this binary —
  likely faster than continuing to guess from vtable slot position.
- `CSWGuiScene::AddModel`'s Steam address — needs a direct search (it's
  non-virtual), not a vtable dump.
- Full detail, addresses, and evidence trail: `ghidra_knowledge` DB's
  `classes`/`functions`/`stack_offsets`/`lessons` tables.

## Goal

Render a live 3D model (`.mdl`/`.mdx`) of whatever item is currently
selected in the inventory list, in the empty ultrawide space beside the 2D
inventory panel.

**Better reference than the character screen: the workbench upgrade
screen (`upgrade_p.gui`, `CUIUpgrade` or similar).** The Character/
Equipment screen's 3D view renders a *creature* (the PC/party member
wearing gear) — that's a different data path (creature model + equipped
item attachments) from what we want. The **workbench item-modification
screen renders the selected *item itself*** — you pick an item from a
list, and it shows that item's standalone `.mdl` in a 3D view, with no
creature involved. That's exactly the trigger → resolve → render sequence
the inventory viewport needs, just in a different GUI. Two things make
this doubly useful here:
- It's already present in this exact build and already ultrawide-adjusted
  — `upgrade_p` is one of the four 3D-view GUIs fixed in Phase 4 of our
  ultrawide pass (see [[kotor2-ultrawide-gui]]: "3D views
  (classsel_p/maincg_p/partyselect_p/upgrade_p)"), so its overshoot/
  positioning problem is already solved and documented — no need to
  rediscover that fix for the inventory case.
- Mod #153 (Workbench Upgrade Screen Camera Tweak, DarthParametric,
  `deadlystream.com/files/file/1743-`) is a **loose-file mod that already
  edits this screen's camera/3D-view data** — worth pulling apart as a
  known-good precedent for how the item 3D view's camera/positioning GFF
  fields work, and as a compatibility check (our new control must not
  collide with what that mod already touches on `upgrade_p`).

## Why this is a Patch Manager task, not a GUI/TSLPatcher task

`inventory.gui` has no `CONTROL_3DVIEW` control and the inventory
listbox-selection handler in `CUIInventory` only ever updates a 2D `.tga`
icon. Getting a live model on screen requires:

1. A new UI control the engine will actually render into (a GFF struct
   edit — doable today with Holocron Toolset, no C++ needed; copy the
   `CONTROL_3DVIEW` struct from `upgrade_p.gui` rather than the character
   screen, since it's already configured for a standalone item, not a
   creature).
2. **The engine to be told, at the moment of selection, to resolve the
   selected item's model and push it into that control.** This logic does
   not exist anywhere in the inventory code path today; it only exists in
   the workbench/character screens' own render calls. This part requires a
   runtime detour — there is no NWScript event or 2DA hook for "item
   selected in inventory listbox."

## Armor/robes need a different render path than weapons (confirmed on disk, 2026-09-28)

External review (Gemini) flagged that armor has no standalone 3D model in
this engine — checked against our actual installed `appearance.2da` and
`baseitems.2da`, and the claim is correct, with the exact mechanism more
precise than the original suggestion:

- `baseitems.2da` has a **`bodyvar`** column (plus `genderspecific`). An
  armor's base item type carries a *body-variation index*, not a model
  reference.
- `appearance.2da` has 14 body-slot columns per race/gender row
  (`modela`/`texa` through `modeln`/`texn`). `bodyvar` only means anything
  as an index into *one specific creature's* row — there is no
  armor-shaped `.mdl` sitting on disk waiting to be loaded standalone. The
  same armor also renders differently per race/gender since each has its
  own row.
- Net effect: an armor/robe item literally has no visual identity outside
  the context of a specific wearer. The Workbench standalone-item render
  path (Phase 1-3 below) is correct and sufficient for **weapons, shields,
  and headgear** (these use `defaultmodel`/a direct model reference, not
  `bodyvar`), but **cannot** work for armor/robes — there is nothing
  standalone to resolve and render.

**Adopted: hybrid render path.** Branch on the selected item's
`baseitems.2da` row:
- `bodyvar` unset/zero (weapon, shield, headgear, most other slots) →
  original Workbench-style standalone-item render, unchanged.
- `bodyvar` set (armor, robes) → resolve the *player character's own*
  race/gender appearance row, index `modela..n`/`texa..n` by `bodyvar`, and
  render that body mesh+texture — no separate creature object needs to be
  spawned, since the PC's own appearance row already has everything
  needed. This is simpler than "spawn a dummy `CNWSCreature`" (see below)
  and reuses data the game already resolves every time the PC's worn-armor
  slot changes in the world.

**Where the external suggestion needs Phase-2 verification, not blind
adoption:**
- *"Spawn a dummy creature, equip it, wrap it in a hologram shader"* — this
  is a bigger, riskier lift than the `appearance.2da` row-lookup approach
  above (runtime object lifecycle, must not register with
  party/faction/AI systems, must be cleaned up reliably on screen close).
  Not adopted as the primary plan; the PC's-own-appearance-row lookup
  achieves the same visual result (a body wearing the armor) without
  spawning anything. Revisit only if the direct row-lookup turns out to be
  blocked by something Ghidra finds in Phase 2.
- *`CNWSCreature` as the class name* — this is an NWN/Aurora Toolset SDK
  name. No evidence in our own `AddressDatabases`/existing patches that
  KotOR's compiled binary uses it internally; don't assume NWN symbol
  names carry over. Phase 2 must independently identify whatever struct
  layout the binary actually uses.
- *A reusable "native hologram VFX"* — partially confirmed: `Scene::` does
  have hologram-aware code (`Scene::DoHologramGobBuckets`, already named in
  Patch Manager's `Max Lights` patch,
  `Patches/Max Lights/kotor2-steam-aspyr.hooks.toml:44`), so the concept is
  real in this engine, not invented. But that specific function only tunes
  a lighting-bucket *count* — it's evidence a hologram-rendering subsystem
  exists, not proof a "make this mesh holographic" toggle is one hook
  away. Low priority since the row-lookup approach above doesn't need it,
  but worth a note if Phase 2 Ghidra work stumbles onto the real shader
  path anyway — a holographic preview would be a nice cosmetic upgrade
  later, just not on the critical path.

## Implementation pipeline (from the original spec, refined)

### Phase 1 — UI canvas prep (data layer, no C++)
1. Extract `inventory.gui` and `upgrade_p.gui` via Holocron Toolset.
2. Locate the `CONTROL_3DVIEW` struct in `upgrade_p.gui` and copy it into
   `inventory.gui`, since its camera/framing is already set up for a
   standalone item rather than a worn creature.
3. Assign an unused control ID and position it in the ultrawide margin.
   **Coordinate with `popos/docs/kotor2_ultrawide.md`** — our existing
   ultrawide pass already defines the content-vs-margin boundary for every
   menu GUI, and already solved this exact 3D-view-overshoot problem for
   `upgrade_p` itself; this control must live in the margin we've already
   reserved and reuse that fix, not re-litigate it.
4. Diff against mod #153's loose-file changes to `upgrade_p` first, so we
   know which camera/GFF fields it already touches before we copy from it.
5. This phase is independently testable/committable even before any C++
   exists — the control will just render nothing until Phase 3 wires it up.

### Phase 2 — engine mapping (Ghidra static analysis)
Now two branches, per the armor-vs-weapon split above — not one uniform
path. Community GZF/SARIF databases should be checked first for both, same
as before.

**2026-09-29 update — see "Phase 2 findings" above for the full confirmed
picture.** Real class names found for every target below (`CUIUpgrade` →
`CSWGuiUpgrade`, `CUIInventory` → `CSWGuiInGameInventory`, `CUI3DView` →
`CSWGui3DSceneView`/`CSWGuiScene`). **Branch A/B render-call question
answered: they very likely DO share one call.** `CSWGuiUpgrade` itself
owns both `Update3DView` (the render/update call) and
`GetArmorModelAndTextureResRef` (armor-specific data resolution) as real
named methods per the GOG address database — i.e. the same class already
handles both the standalone-item case (Branch A) and the
armor-model+texture case (Branch B) internally, which is stronger evidence
for "one shared render call, two data-resolution paths" than the original
spec's guess that Branch B would need the separate Character/Equipment
screen. Exact Steam addresses for these specific named methods are still
open — see "Phase 2 findings" for the recommended next search technique.

**Branch A — weapons/shields/headgear (standalone item, unchanged from
original spec).** Primary reference: the workbench item-modification flow
(`upgrade_p`/`CUIUpgrade` or equivalent) — item-selection → `.mdl`
resolution → 3D render for a standalone item:
1. **Trigger**: the workbench's item-list selection handler (structurally
   the closest analog to `CUIInventory`'s own listbox handler).
2. **Data resolution**: the routine that reads a selected item's `.uti` →
   `defaultmodel`/direct model reference (from `baseitems.2da`) — likely
   shared with the inventory 2D icon lookup.
3. **Render call**: the `CUI3DView` init/update function as called by
   `upgrade_p` for the selected item.

**Branch B — armor/robes (PC-own-appearance-row lookup, new).** No
standalone model exists (see above), so this branch needs different
addresses, likely from the *Character/Equipment screen's* own render path
(the one place the engine already does a live "PC wearing X" render) rather
than the workbench:
1. **Data resolution**: the routine that, given a `baseitems.2da` `bodyvar`
   index and the PC's own `appearance.2da` row, resolves which
   `modelX`/`texX` pair to load — this almost certainly already exists,
   since it's exactly what runs every time the PC equips armor in the
   world; find it via the Character screen's equip-slot-changed handler
   rather than writing new resolution logic.
2. **Render call**: same `CUI3DView`-family call as Branch A, just fed the
   PC's body model/texture instead of a standalone item model. Confirm
   whether this is literally the same function as Branch A's Target 3 with
   different arguments, or a distinct one — if the former, Phase 3's C++
   collapses to one shared render call with two data-resolution paths
   feeding it.

### Phase 3 — hook construction (`.kpatch`)
1. New patch directory under Patch Manager's `/Patches`.
2. `manifest.toml` declaring K2 Steam/Aspyr build compatibility (confirm
   whether this needs the *exact* build hash we're on — see
   `03_framework_research.md` for whatever the framework's version-pinning
   story turns out to be; also see `05_build_plan.md`'s Phase 0 outcome —
   our live exe's LAA-patched hash currently matches no manifest entry,
   needs resolving before install regardless of this mod).
3. `hooks.toml`: detour on the inventory listbox-selection trigger.
   `skip_original_bytes = false` — the inventory must keep updating its 2D
   icon as before; we're augmenting, not replacing.
4. C++ detour: read selected item's `baseitems.2da` row → branch on
   `bodyvar` → (Branch A) resolve `.mdl` string directly, or (Branch B)
   resolve PC's own `modelX`/`texX` via `bodyvar` index → call the render
   function with our control ID either way.
5. Build via `create-patch.py`/`.bat` → `.kpatch`.

### Phase 4 — runtime verification
Load via `KPatchLauncher.exe`, confirm `KotorPatcher.dll` injects, select
several item types across both branches — weapon (Branch A), armor
(Branch B, check it matches the PC's actual race/gender), and a stacked
consumable with neither a model nor a `bodyvar` (neither branch — confirm
the fallback, not a crash).

## Open risks specific to our build

- **Ultrawide layout collision.** Our ultrawide pass already treats 3D
  views as a special case — see [[kotor2-ultrawide-gui]]: "kept 3D views
  overshot their frames (character view covered the stats)... The engine
  renders 3D views at real pixel aspect." A new inventory 3D view will hit
  the exact same overshoot problem and needs the same fix pattern (content
  vs. margin classification), not a fresh one.
- **Model availability.** Armor/robes are now handled by Branch B (see
  above), not a missing-model edge case. The remaining gap is items with
  neither a direct model nor a `bodyvar` — stacked consumables (medpacs,
  grenades) that may only have icon art. Need a fallback (hide the 3D
  control, or show nothing) rather than crashing on a null result from
  either branch.
- **New risk from Branch B**: rendering "PC wearing this armor" means the
  preview reflects the PC's *current* race/gender/body — correct behavior
  (that's genuinely what it'll look like worn), but confirm the render call
  doesn't require the PC to *actually* be wearing the item first, or the
  hook needs to fake that state for the render without touching the real
  equipped-item slot (which would be a gameplay-affecting bug, not just a
  UI one).
- **Texture/HD overrides.** This build has Ultimate Character Overhaul,
  HD NPC/PC Portraits, and several appearance-change mods installed (mods
  #15-16, #46-47, etc.). Whatever `.mdl`/`.mdx` the render call pulls needs
  to resolve through the same override chain those mods use, not the
  vanilla chitin path, or the 3D preview will show unmodded vanilla art.
- **Performance**: a live 3D render on every listbox selection, at 60 FPS,
  in a menu that's already being redrawn by our upscaled GUI textures —
  should be cheap (character screen already does this every frame) but
  worth a MangoHud check per [[kotor2-ultrawide-gui]]'s existing perf
  monitoring setup.

## Effort estimate (rough, pre-Ghidra)

Phase 1 (GUI struct copy): hours, once Holocron Toolset workflow is
confirmed. Phase 2 (Ghidra): the long pole — hours to days depending on
whether a community GZF database already has these three functions
labeled. Phase 3-4 (hook + C++): hours once addresses are known, assuming
the Patch Manager toolchain builds and runs under our Proton setup (open
question, see `03_framework_research.md`).
