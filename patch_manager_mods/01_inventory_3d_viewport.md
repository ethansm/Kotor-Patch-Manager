# Mod Idea 1: Inventory 3D Viewport

**Status:** ACTIVE — research (Phases 0-1) DONE and GUI control (Phase 2) DEPLOYED as of 2026-09-29. **NEXT: Phase 3 build** (model table + `Inventory3DViewport` patch), then live test. User paused execution here on 2026-09-29; the full self-contained plan is the approved plan file for the next phase.
Render pipeline resolved (see Phase 2 findings); the planning session
(Opus, 2026-09-29) changed the design — see "Planning session findings".
No hooks written yet, no `.kpatch` built.

## Planning session findings (2026-09-29, Opus)

Four read-only Ghidra runs (raw output: `inventory_3d_viewport_src/research/ghidra_run{1..4}_2026-09-29.txt`) changed the design:

1. **Adding a `CONTROL_3DVIEW` to `inventory_p.gui` alone does nothing.** Panels bind fixed members by tag in their constructor (`StartLoadFromLayout` -> `InitControl` -> `StopLoadFromLayout`). The inventory has no scene-view member, so the DLL allocates and constructs one and binds it inside the ctor layout window.
2. **`008cd2c0` can be reused unchanged** with a fake `this` = `block - 0x3a30` (reads only `+0x3a30`, `+0x3ac4`, `+0x3d2c`, `+0x3d34`). **R1 must verify from raw disasm before any code.**
3. **Inventory item objects match** what the render function expects (`+0xc` base item id).

| What | Address | Notes |
|---|---|---|
| `CSWGuiUpgrade` ctor | `008c9e10` | 2 embedded `CSWGui3DSceneView` at `+0x3a30`, `+0x3ac4` (0x94 bytes each) |
| `CSWGuiInGameInventory` ctor | `008a6170` | binds 21 controls, no scene view |
| `CSWGuiControl` base ctor | `00418990` | fastcall(ECX=ctrl) |
| `CSWGui3DSceneView::vftable` | `009a3d34` | |
| `CSWGuiScene` ctor | `00417f40` | scene = view+0x60 (verify) |
| `StartLoadFromLayout` / `InitControl` / `StopLoadFromLayout` | `0040ee40` / `0040f620` / `0040f5a0` | InitControl stores at panel+0x24 index = ctrl+0x54 |
| `AddEvent` | `00418af0` | |
| `CExoString` ctor/dtor, operator new | `00733570` / `00733780` / `00919723` | |
| `AddModel` | `00418360` | |
| Workbench render | `008cd2c0` | fastcall(ECX=this); gated by `*(int*)0x009f42b4` |
| armor resolver | `008cd060` | called with row 0x89 = 137 = `P_MAL_A_MED_01` (fixed male medium body) |
| non-armor resref builder / base item lookup | `006d5000` / `006d6e30` | |
| Workbench Update (vt13) | `008cce70` | does NOT call the render; only spins model 1 |
| Inventory vtable `009a72ac` | slots 0/13/15/18/19 = `008a7030`/`008a7350`/`008a8880`/`008a74c0`/`008a7520` | |
| Inventory members | `LB_DESCRIPTION` +0x878, `LB_ITEMS` +0x588, entries +0x24b0 | |
| Inventory populate / CreateItemEntry | `008a7ba0` / `008a75f0` | |

**Design (v1):** GUI script adds `3D_MODEL` (ID 21, rect `[645,134,116,300]`) to `inventory_p.gui`; a DLL hooks (H1) the inventory ctor to allocate+construct+bind a view and init the scene, (H2) the item-shown/description-refresh path to call `008cd2c0` with fake this, (H3) Update for logging/poll fallback.

**Superseded:** the old "Phase 1 — copy CONTROL_3DVIEW" step is insufficient on its own (see 1 above). The "PC-own-appearance-row" Branch B is superseded: v1 uses the Workbench's fixed row 137. A PC-specific body is a possible v2, by calling `008cd060` directly.

## Phase 1 results (2026-09-29, raw disasm; evidence in `inventory_3d_viewport_src/research/R*_*.txt`)

- **R1 PASS (GATE A).** `008cd2c0` keeps `this` at `[ebp-0xac]`; it only reads `+0x3a30`, `+0x3ac4` (category==1 only), `+0x3d2c` (byte) and `+0x3d34` (item ptr, never written or freed). `008cd060` is called with `ECX=this` but only spills it (`[ebp-0x78]`, never read). Scene calls use `ECX=view+0x60`. Item fields read: `+0xc`, `+0x2a4`, `+0x2a6`. `GetModel(0)` must be `upgitem_light` (hook nodes), so scene init is mandatory. Use category 2 (never 1) so the saber view (`block+0x94`) is untouched. Fake-`this` offsets: category = `block+0x2fc`, item = `block+0x304`.
- **R2.** `00418990` ECX=`this+0x3a30`; vftable `0x9a3d34` stored at `[view]`; `00417f40` ECX=`view+0x60`; `InitControl(panel, view, &CExoString("3D_MODEL"), 1, 1)`. Scene init: `Scene*=[view+0x60+0x14]`; `(*Scene)->vt[0x70]` thiscall with 3 stack args (`"gui3D_room"` char*, float pos[3]={0,0,0}, float quat[4]={0,0,0,1}); then `AddModel(scene,&CExoString("upgitem_light"),-1)`. Gated on `*(int*)0x009f42b4`.
- **R3 (H1 site).** `0x008A69AD`, original bytes `8B 8D 08 FD FF FF`, param `[ebp-0x2f8]` = panel. Immediately before `StopLoadFromLayout` (`008a69b3`), so the GFF is still open. No branch targets. Verified against the live exe.
- **R4 (H2 site).** Entry event 0 handler `008a8100(panel=ECX, entry)`: item = `EAX` after `CALL 0051c0e0` (`008a816d`). Splice `0x008A8172`, original `89 45 D4 8D 55 CC`, params `eax` (item) and `[ebp-0x68]` (panel). Verified vs the live exe. Known gap: the "Error: Invalid item" branch (`entry+0x1d0==0x7f000000`) returns before the splice, so the preview would not clear there. The selected object id is also in `panel+0x24c8`.
- **R5 (H3 site).** `0x008A7350`, `55 8B EC 83 EC 0C`, params `ecx` and `[esp+4]` (float dt). Verified vs the live exe.
- **R6.** The panel is `0x24d4` bytes, built by factory `007d0760` / manager init `007be4c0` once per GUI init. Base dtor `0040e860` does not delete `panel+0x24` list entries, so v1 leaks the `0x400` block (log every construction) and there is no double free.
- **R7.** `CSWGui3DSceneView` vtable differs from `CSWGuiControl` at slot 0 (`0084de70`), slot 1 (`00420f50`, extent override) and slot 14 (`0084de40`, probably Draw). Informational.

## Phase 2 (GUI) DEPLOYED 2026-09-29

`scripts/add_inventory_3dview_control.py` appended `3D_MODEL` (copy of `upgrade_p.gui`'s, CONTROLTYPE 4, ID 21, EXTENT `[645,134,116,300]`, empty BORDER resrefs) to the live `override/inventory_p.gui`. New sha256 `46d84b7d5a4d…` (was `5980756fa20a…` = generator manifest `deployed_sha256`). Backup: `~/Games/KOTOR2_Mod_Workspace/backups/inventory3d_20260929_183902_373234/`. Rollback: `.venv/bin/python scripts/add_inventory_3dview_control.py --rollback` (restores the OLDEST backup). Options `--rect L,T,W,H --replace --dry-run`. Idempotent. **Generator interaction:** `generate_ultrawide_gui_overrides.py` refuses to overwrite a live file whose sha is neither the recorded original, the previous deploy nor its output ("foreign file"), so after this deploy a generator re-run for `inventory_p` fails loudly: run `--rollback` here first, re-run the generator, then re-apply this script. pykotor gotchas: `GFFStruct.get_struct()` returns a COPY (edit then `set_struct`); `get_list` may return a detached list (`set_list` after append); `ID` is Int32 (`get_int32`). Effect in game before the DLL exists: none (unbound control is ignored) — not yet visually confirmed in game.

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

**RESOLVED (continuation session, 2026-09-29) — the render call, the armor
data path, and the Branch A/B merge are all now decompile-confirmed.**
Per the resume plan's Step 1 ("decompile the vtable slot's own unexamined
helper calls"), decompiling the 4 functions `FUN_008cbde0` (slot 18) calls
— `FUN_008cb2f0`, `FUN_008cb230`, `FUN_008cd2c0`, `FUN_008cd8e0` — resolved
nearly everything in one batch:

- **`CSWGuiScene::AddModel` — Steam address `0x00418360`, CONFIRMED.**
  `__thiscall(this=Scene, modelResrefString, insertIndex)`: resolves the
  model reference via a registry/factory lookup, then calls
  `FUN_004183b0` (a Gob-array insert helper: null-checks the Gob,
  bounds-checks `insertIndex` against `this+0x20` — the scene's Gob
  count — appends or inserts, then calls a `SetScene`-shaped virtual on
  the new Gob at `vtbl+0x4c`), returning the created `Gob*` or null.
- **The 3D-preview resolve+render function — Steam address `0x008cd2c0`**
  (called from `CSWGuiUpgrade::Update`, vtbl[13], when the `+0x3d2c`
  flag/category byte is set). **This one function contains the entire
  Branch A/B split, decompile-confirmed**: it reads `+0x3d2c` (item
  category 1-4); category `4` (armor) branches into
  `GetArmorModelAndTextureResRef`; every other category uses a direct
  sprintf-based model-resref builder (`FUN_006d5000`, `"%s_%03d"` /
  `"%s_%c_%03d"`). **Both paths converge on the same single call** to
  `AddModel(modelResref, insertIndex=-1)` — i.e. **the Branch A/B
  shared-render-call hypothesis is CONFIRMED, not just plausible.** Both
  paths finish by setting an animation-state string on the Gob
  (`"rotate"`/`"powered"`/`"neutral"` via `vtbl+0x18`).
- **`GetArmorModelAndTextureResRef` — Steam address `0x008cd060`,
  CONFIRMED.** Resolves **both** a model resref and a texture resref:
  clamps the `bodyvar` index (param) to 1-14 and adds `'@'` to produce a
  letter A-N, referencing the string literal `"CModelA"` — this is
  exactly `appearance.2da`'s `modelA..modelN`/`texA..texN` column
  convention, independently confirming the doc's "PC-own-appearance-row
  lookup" mechanism for armor. Texture name built via `"%s%02d"` from a
  race/variant-derived index, with 2 fallback texture-existence checks
  before settling on a final index.
- **`FUN_008cbde0` (slot 18) reclassified again**: it is neither
  `HandleInputEvent` (ruled out previously) nor the render call itself —
  it's an item-category-driven **UI-slot visibility toggler** (shows/hides
  upgrade-slot controls based on the `+0xf4b` category byte) that calls
  the 4 functions above as siblings, not as its own body. Two of those
  siblings are now named: `FUN_008cb230` = `GetSlotCountForCategory`
  (returns 6/3/3/2 slots for categories 1-4 via a jump table, reading the
  same `+0x3d2c` byte); `FUN_008cd8e0` = upgrade-slot **icon** refresh
  (2D UI, unrelated to the 3D render path, ruled out).

Full addresses, decompiled bodies, and evidence trail:
`ghidra_knowledge` DB's `classes`/`functions`/`lessons` tables (scope
`Inventory 3D Viewport`, topic "Step 1 SUCCEEDED").

**Still open, low priority**: `OnSlotSelected`/`OnUpgradeSelected`'s exact
Steam addresses are not individually pinned down (the UI-visibility and
icon-refresh functions found above are closely related but not confirmed
1:1 name matches to those two specific GOG names) — not blocking, since
the render pipeline itself (data resolution → `AddModel` → animate) is
now fully mapped end-to-end, which is what Phase 3's hook needs. The two
`CSWCItem` RTTI hits' relationship (Step 4 in the resume plan) also
remains open and low-priority.

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
