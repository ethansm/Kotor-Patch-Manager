# Mod Idea 2: Visual Buff Duration HUD

**Status (2026-09-29, 7th session, current):** Phase 2 and Phase 3
research are now **effectively COMPLETE**. R3's residual, item 2
(multi-slot creature resolution, all-party-members scope), and R4 (both
hook sites, byte patterns, wipe mechanism) are all **RESOLVED**. Item 3
(spellID <-> spells.2da row-index cross-check) is **PARTIALLY resolved**:
the field's identity/sentinel behavior is decompile-confirmed, but its
exact numeric ID space is NOT decompile-confirmed -- flagged for
empirical (in-game) verification rather than further static tracing, per
this session's own effort budget. See "Item 1-4 findings (R3 residual +
multi-slot creature resolution + R4)" (bottom of the UX sections, right
before "## Goal") for full detail; it supersedes everything below it for
anything it re-addresses. **Development phase (real hook C++ +
hooks.toml + `.kpatch`) was NOT started this session** -- item 3's
open numeric-ID-space assumption is directly load-bearing for the
render hook's icon lookup, so per this project's gating discipline this
is a research-handoff stopping point, not a build-it point, absent an
explicit user decision to accept that one flagged risk.

**Status as of the previous (6th) session, superseded above for R3/R4 but
otherwise still accurate:** Phase 2 (Ghidra static analysis) **RESOLVED**.
Phase 3 (render redesign + data-layer research) **NEARLY ALL RESOLVED**:

- R1 (creature-per-slot), R6 (max/total duration field): **RESOLVED**.
- UX pivot render approach (U1(a)/U2/U4/U5): **RESOLVED** (prior
  session).
- **U3 (on-screen layout of portrait sub-widgets): FULLY RESOLVED** —
  all 8 of `Adjust`'s (`0x00745560`) sub-control array slots matched
  1:1 to their GUI tag names via `FUN_007450e0`'s storage offsets.
- **U1 residual (which embedded border is frame vs. fill): RESOLVED**
  — `CSWGuiLabel::Load` found (`0x004198d0`), profiler-scope strings
  prove `self+0x60`="BORDER" (frame), `self+0xd8`="TEXT" (content/fill,
  the one to retarget).
- **U5 residual (literal resref-bind mechanism): RESOLVED** — full
  chain traced (`FUN_0047eb60`→`FUN_0047d070`→`FUN_004da820`, param_2
  confirmed literal C-string resref); recommended mod entry point is
  `FUN_0047eb60` itself (already fully understood), not the deeper
  `FUN_004da3f0` (turned out to be a resource-file line parser, not a
  simple bind).
- **R2 (icon-to-effect matching): DISSOLVED**, as predicted — confirmed
  from existing decompile (no new Ghidra work) that `FUN_007457e0`
  repositions+redraws a SINGLE shared widget instance N times, with no
  engine-side per-icon identity to match. The mod assigns its own
  active-effect list to iteration order.
- **R3 (game time source): RESOLVED** (mechanism + exact call chain),
  one small residual (the world-timer `this`-pointer's ultimate source,
  one more decompile hop away, not blocking).
- **R5 (spell-ID mapping): RESOLVED** — `HasSpellEffectApplied` found,
  active-effect array offsets and the per-effect spell-ID field
  identified.
- R4 (precise hook site): **still deliberately deferred** per this
  plan's own gating (R3 has a small flagged residual) — next session's
  focus.

A real scaffold with decompile-confirmed constants exists at
`Patches/BuffDurationHUD/` in the `kotor-patch-manager` clone (not
committed to this repo). **No `.kpatch`, no `hooks.toml`, no
live-game changes yet** — development phase remains out of scope until
R4 resolves (now the only remaining open item). See "Resource-bind
completion + icon/effect correspondence findings" (bottom of the UX
sections, right before "## Goal") for the most current findings; it
supersedes everything above it in this status block for anything it
re-addresses.

## Phase 2 findings (2026-09-29)

**Community database check, per the effort-estimate note below**: no
external GZF/SARIF database was consulted — a better one was already on
disk. The Patch Manager framework itself ships `Patches/Common/GameAPI/`,
~50 already-reverse-engineered class headers (`CSWSCreatureStats.h`,
`CSWSObject.h`, `CGuiInGame.h`, `CSWGui3DSceneView.h`, etc.) built from
prior community/framework-author RE work — check this before any fresh
Ghidra RTTI search; it already ruled out one dead end (`CGuiInGame` is
K1-only, becomes a safe no-op on K2 — don't target it) and, via a sibling
already-shipped patch's own source comment
(`K2AspyrControllerFixes/HelpMenuInputFix.cpp`, native-Linux-Aspyr build),
confirmed the real HUD class name below without needing to guess.

**Target 1 (data layer) — real class names found, exact duration offset
still open.**
- `CUIHUD`/`CGameEffect` (the spec's original SDK-style guesses) are
  **not real RTTI class names in this binary** — confirmed by a 0-hit
  keyword search. The real classes: `CSWSEffectListHandler` (vftable
  `0x00992490`) and its base `CGameEffectApplierRemover` (vftable
  `0x009924a4`), both RTTI-confirmed. Constructed together in
  `FUN_0051d2f0`, embedded in a larger (not-yet-named) owning object at
  `+0x58` (immediately followed by a parallel
  `CSWSItemPropertyHandler`/`CItemPropertyApplierRemover` pair at `+0x5c`).
- Separately, `FUN_006a5110` (a ~7.7KB AC-bonus recompute function, found
  by following the `"+ %d (Effect_AC_Shield_Bonus)"`-style debug strings)
  confirmed the actual active-effect list's on-the-wire shape: an array
  header at `ownerObj+0x24 → +0x148` (`{basePtr, count}`), 4-byte pointer
  elements, each effect object carrying a `ushort` type/subtype tag at
  `+0x8` and further integer parameters read through an accessor call
  `FUN_005e4e60(index)` rather than raw struct offsets.
- **Continuation session (2026-09-29): Apply/Remove dispatch architecture
  fully mapped.** Dumped both `CSWSEffectListHandler::vftable` and
  `CGameEffectApplierRemover::vftable`. The base class declares 3 pure
  virtuals (Init, Apply, Remove); `CSWSEffectListHandler` overrides them
  with: an Init routine (`FUN_00593a30`) that builds two 118-entry (`0x76`,
  matching this engine's `SWEFFECTTYPE` count) per-effect-type
  function-pointer tables at `this+4` (Apply) and `this+8` (Remove); a
  generic `Apply(effectPtr)` dispatcher (`FUN_00594740`) that reads
  `effectPtr+8` (ushort type tag — independently reconfirms the `+8` offset
  from the AC-bonus work) and calls the matching per-type handler; and the
  matching `Remove(effectPtr)` dispatcher (`FUN_005947a0`). These are
  generic type-dispatch only — neither reads a duration field itself.
  Decompiled `FUN_005e4e60` (a per-effect field accessor): a trivial
  `*(this+0x34)[index]` generic accessor used from 525 call sites across 154
  functions — too broad to hand-scan, but turned out to be the first member
  of a much larger, fully-resolved accessor cluster (see below).
- **3rd session (2026-09-29): TARGET 1 RESOLVED.** A pre-session query of
  the framework's `AddressDatabases/kotor2_gog_aspyr.db` found `CGameEffect`'s
  full 20-method function table (GOG-only, decimal addresses) — including
  `SetExpiryTime`/`GetExpiryTime`, both 8-byte/2-explicit-arg signatures,
  sitting in the same tight source-order cluster as `GetInteger`/`SetInteger`/
  `GetFloat`/`SetFloat` (already-confirmed `FUN_005e4e60` on Steam). Using
  this as a positional-clustering search (new script `ListFunctionsInRange.java`,
  window `0x005e4800`–`0x005e5200`), found a run of 10 more small functions
  (23–53 bytes) immediately after `FUN_005e4e60`, at consistent +32-byte
  deltas. Decompiling all 10 in one batch produced an exact 1:1
  positional-and-content match against GOG's 11-function `CGameEffect`
  table, no folding needed:
  - `0x005e4e60` = `GetInteger(index)` — `*(int*)(this+0x34)[index]`
  - `0x005e4e80` = `SetInteger(index,value)` — same `+0x34` array, write
  - `0x005e4ea0` = `GetFloat(index)` — `*(float*)(this+0x38)[index]`
  - `0x005e4ec0` = `SetFloat(index,value)` — same `+0x38` array, write
  - `0x005e4ee0` = `GetObjectID(index)` — `*(this+0x78)[index]`
  - `0x005e4f00` = `SetObjectID(index,value)` — same `+0x78` array, write
  - `0x005e4f20` = `GetString(index)` — `CExoString` copy-ctor on a
    stride-8 array at `+0x48`
  - `0x005e4f60` = `SetString(index,value)` — `CExoString::operator=` on
    the same `+0x48` array
  - **`0x005e4f80` = `SetExpiryTime(this, day, time)`** — writes two FIXED
    (non-indexed) fields: `this+0x10 = day`, `this+0x14 = time` ***— THE
    DURATION FIELD ***
  - **`0x005e4fa0` = `GetExpiryTime(this, int* outDay, int* outTime)`** —
    reads the same `this+0x10`/`this+0x14` pair via two output pointers
    ***— THE DURATION FIELD, READ PATH***
  - `0x005e4fd0` = `CopyEffect` (651 bytes, the one large function
    immediately after the cluster) — independently corroborates the
    `+0x10`/`+0x14` fields by copying them (`param_1[4]`/`param_1[5]`)
    during a full-object copy.

  `SetExpiryTime`/`GetExpiryTime` are the only pair in the cluster that use
  **fixed, non-indexed** offsets instead of the generic indexed-array
  pattern every sibling accessor shares — a clean structural fingerprint,
  not just a name-based guess. **`DurationRemaining` for the HUD = the
  effect's `ExpiryTime` (day, time-of-day pair) minus the engine's current
  game time** — an absolute-time comparison, not a raw countdown float;
  both components are needed. Since `CGameEffect` has no RTTI/vtable
  (non-polymorphic, consistent with the project's established
  small-vtable-means-non-virtual-API pattern), a HUD hook can read
  `effectPtr+0x10`/`+0x14` directly with no function call, for a read-only
  display. Full field layout (all 4 indexed arrays + the 2 fixed fields) is
  in the `ghidra_knowledge` DB's `classes`/`functions` tables (`CGameEffect`
  row) and `lessons` (topic "Target 1 RESOLVED").

**Target 2 (render layer) — strong candidate functions found, not yet
disambiguated Update-vs-Render.**
- Confirmed `CSWGuiMainInterface` (not a guessed `CUIHUD`) is the real
  main-HUD class — same class already known from `K2AspyrMapAspectFix`'s
  minimap work, independently reconfirmed via
  `K2AspyrControllerFixes/HelpMenuInputFix.cpp`'s own
  `CSWGuiMainInterface::HideHelp` reference.
- Its vtable slot [13], `FUN_0074b3f0` (`__thiscall(this, float dt)` —
  per-frame signature), is a strong per-frame Update candidate: early-outs
  on a visibility bitflag, decrements several countdown-timer floats by
  `dt`, builds tooltip/status text from StrRefs `0x1e61e`-`0x1e621`, and
  ends with 3 calls to `FUN_00745770(dt, isFirstSlot)` — a party-of-3
  portrait-update loop.
- Its vtable slot [14], `FUN_0074c320` (same `(this, float dt)` shape), is
  a second per-frame pass that walks **two parallel fixed-size widget-slot
  arrays inside the instance**: an action-bar-shaped array (base `+0x733c`,
  7 entries, stride `0x750`) and a party-portrait-shaped array (base
  `+0x120`, 3 entries, same `0x750` stride), each slot having 4 notable
  sub-offsets at `+0`/`+0x1d0`/`+0x3a0`/`+0x570` — a pattern consistent
  with icon-quad / cooldown-overlay-quad / label / tooltip sub-widgets per
  slot. **Promising lead, not yet confirmed**: this suggests the action
  bar *already implements* a per-slot cooldown-wipe overlay for force-power
  recharge timers — if so, the buff-duration HUD could structurally mirror
  (or literally reuse) that existing draw primitive instead of building a
  new one from scratch. Not decoded field-by-field yet.
- Vtable slot [15], `FUN_007469f0`, was checked and **ruled out** — it's an
  on-demand GUI-event dispatcher (radial menu / quick-menu actions), not a
  per-frame tick.
- **Continuation session (2026-09-29): all 7 divergent slots ruled out,
  `Draw` confirmed non-virtual.** The 4 previously-undecompiled slots
  (16/18/19/27) were read: none is D3D-shaped (slot 16 = a lookup
  dispatcher, slot 18 = a focus/selection transfer routine shared with
  `CSWGuiInGameMap`, slot 19 = sub-object teardown, slot 27 = a small
  wrapper) — none of `CSWGuiMainInterface`'s 7 divergent vtable slots is a
  plausible `Draw()`. Cross-checking against the framework's GOG address
  database confirmed why: GOG names `Draw` as a function separate from
  `Update` — non-virtual, same pattern as `CSWGuiScene::AddModel`. GOG also
  named 3 sub-widget classes not located on Steam via RTTI (zero hits —
  non-polymorphic embedded structs): `CSWGuiMainInterfaceChar`
  (`DrawEffects`/`DrawEffects_2`, `DrawStack`), `CSWGuiMainInterfaceAction`
  (`Draw`/`Update`/`SetIcon`), `CSWGuiMainInterfaceStatus`. Slot [14]'s own
  ~20 helper calls were all bookkeeping, not Draw-shaped, though it does
  compute a fill-ratio at `this+0x7334`→`this+0x706c` (a cooldown-wipe-ratio
  computation, whose consumer was not located that session).
- **3rd session (2026-09-29): TARGET 2 RESOLVED.** Full re-decompile of
  `FUN_0074b3f0` (vtbl[13]/`Update`) plus first decompile of
  `FUN_00745770` (its 3×-called party-portrait sub-call) — this time
  reading the raw disassembly at each call site, not just the pretty-print
  (which hides the implicit `this`/ECX argument for `thiscall` calls).
  This revealed a **4th, previously-unknown array** inside
  `CSWGuiMainInterface`: base `this+0x1f34`, stride `0xde4` (3556 bytes), 4
  entries — distinct from both arrays already found via slot [14]
  (`+0x733c` action-bar, `+0x120` portrait). `Update` calls
  `FUN_00745770(slotPtr, dt, isFirstSlot)` for `slotPtr = this+0x1f34 +
  i*0xde4`, `i` = 0..2 in a loop plus one more explicit call at `i=3`
  (`this+0x48e0`, confirmed arithmetically: `0x1f34 + 3*0xde4 = 0x48e0` —
  a 4th "extra" slot, likely the player/self, separate from the 3
  party-follower slots).

  `FUN_00745770` is a thin gate: if a bitflag at `slotPtr+0x590` is set, it
  checks two byte counters at `slotPtr+0xddc`/`slotPtr+0xddd` and
  conditionally calls `FUN_007457e0(slotPtr, dt, isFirstSlot, 1-or-0)` once
  per nonzero counter.

  **`FUN_007457e0` (Steam `0x007457e0`) is the render hook point.** It:
  reads a screen rect (`slotPtr+0x54c..0x558`), sets up a clip/viewport
  (matching the render-state bracket pattern used around the whole HUD
  pass), reads an icon **count** from the same `0xddc`/`0xddd` byte
  (clamped to 5 or 9), computes per-icon X spacing scaled by a UI-scale
  factor call (`FUN_00479ab0()` — relevant to
  [[kotor2-ultrawide-gui]]'s `HUD_DESC_COEFF` conventions), then **loops
  icon-count times calling two virtual methods on an embedded polymorphic
  sub-object** at `slotPtr+0xab0` or `slotPtr+0x968`: vtbl slot 1
  (`SetPosition`-shaped, a computed per-icon rect) then vtbl slot 14
  (`Draw(dt)`-shaped — the established `(this, float)` per-frame Draw
  fingerprint) for each icon.

  **Hook plan (Phase 3): detour `FUN_007457e0` itself**, `skip_original_bytes
  = false`, and after the original icon row draws, draw the cooldown-wipe
  overlay reusing the same per-icon rect math the engine just computed
  (available directly off `slotPtr`) — matches this mod's original
  additive-only design exactly ("draw after the engine's own HUD pass,
  same frame"). A more surgical alternative (hook the inner vtbl-slot-14
  `Draw(dt)` call directly) is possible but needs the icon-widget
  sub-object's concrete class/vtable identified first — not done, not
  required for the recommended approach.

  Full addresses, the vtable dump, and both `functions`/`classes` rows are
  in the `ghidra_knowledge` DB (`lessons` topic "Target 2 RESOLVED").

**Open item for Phase 3 (not a Phase 2 blocker):** the render-side icon
count (`slotPtr+0xddc`/`+0xddd`) was not traced back to its writer this
session — Phase 3 work should confirm it's actually driven by the Target-1
`CGameEffect` active-effect list (`ownerObj+0x24`→`+0x148`) for the
corresponding party member, not some other status-icon source (e.g. combat
feat icons), before wiring a hook that assumes a 1:1 icon↔effect mapping.

**Effect ID table (data lookup, not Ghidra) — DONE.** Pulled directly from
this install's live `override/spells.2da` (binary V2.b format, parsed via
`pykotor`; confirmed genuinely install-specific — its MD5 differs from
both the pre-compatch backup and the raw TSLRCM copy). Row index in this
2DA is each power's real `SPELL_*` constant/ID. Self-buff (duration-based)
force-power rows found in this install:

| Row | Label | Icon |
|---|---|---|
| 8 | `FORCE_POWER_SPEED_BURST` | `ip_speedburst` |
| 34 | `FORCE_POWER_KNIGHT_SPEED` | `ip_knightspeed` |
| 37 | `FORCE_POWER_SPEED_MASTERY` | `ip_masterspeed` |
| 22 | `FORCE_POWER_FORCE_VALOR` | `ip_mind` |
| 33 | `FORCE_POWER_KNIGHT_VALOR` | `ip_knightmind` |
| 36 | `FORCE_POWER_MASTER_VALOR` | `ip_mastermind` |
| 17 | `FORCE_POWER_FORCE_ARMOR` | `ip_armor` |
| 18 | `FORCE_POWER_FORCE_AURA` | `ip_aura` |
| 24 | `FORCE_POWER_FORCE_SHIELD` | `ip_shield` |
| 135 | `FORCE_POWER_FORCE_BARRIER` | `ip_frcbarrier01` |
| 136 | `FORCE_POWER_IMPROVED_FORCE_BARRIER` | `ip_frcbarrier02` |
| 137 | `FORCE_POWER_MASTER_FORCE_BARRIER` | `ip_frcbarrier03` |
| 138 | `FORCE_POWER_BATTLE_MEDITATION_PC` | `ip_battlemed01` |
| 139 | `FORCE_POWER_IMPROVED_BATTLE_MEDITATION_PC` | `ip_battlemed02` |
| 140 | `FORCE_POWER_MASTER_BATTLE_MEDITATION_PC` | `ip_battlemed03` |

Icon `resref`s confirm the "reuse existing power icons" plan in the
Implementation pipeline below is viable as-is — every row already has a
`.tga`-backed icon resref shipped in this build.

## Phase 3 findings (2026-09-29, planning session)

Framework-mechanics research (hook types, the richer detour
parameter-source syntax added in commits #272–#275, the raw-offset-vs-
GameAPIObject architecture decision, and a splice-timing correction) is
persisted in the `ghidra_knowledge` DB's `lessons` table rather than
duplicated here — see scope `general Ghidra methodology / kotor-patch-manager
framework` and `Buff Duration HUD`, all dated 2026-09-29. Summary: this
mod uses raw hardcoded Steam offsets (`K2AspyrMapAspectFix`'s proven
pattern), not the framework's `GameAPIObject`/`GameVersion` typed-wrapper
system (which has zero rows for any class this mod needs in the K2 LAA
runtime address DB); and a detour's `skip_original_bytes=false` runs
**our** code first, then the original bytes after — the opposite of what
an earlier draft of this plan assumed, which matters for R4's site choice
below.

### R1 + R2 (creature identity half) — RESOLVED

`CSWGuiMainInterface::Update` (`0x0074b3f0`) does **not** itself bind
creatures to portrait slots — full re-decompile showed it only calls
`FUN_00745770` per slot for the icon-draw pass. Following its earlier
call to `FUN_0074ee90()` (0 explicit args, matching GOG's
`UpdatePortraits`, found via a cross-reference search for the portrait
array's base offset `0x1f34`) found the real per-frame resolution chain,
every hop decompile-confirmed:

1. `rosterObj = FUN_0073fb90(hudThis)` = `*(*(int*)(hudThis+4)+0x270)`
2. For raw roster position `p` in 0..2: `handle = FUN_007e5da0(rosterObj, p)`
   — bounds-checked against `*rosterObj` (a count), array at
   `rosterObj+0x24` stride `0x88`
3. If nonzero: `creaturePtr = FUN_0077d800(handle)` — resolves to a
   `CSWSCreature`-shaped polymorphic object

**The resolved `creaturePtr` is never stored into the portrait slot
struct** — only two small scalar bookkeeping fields get written there
(neither is identity). A render-time hook cannot read "which creature
owns this slot" directly off `slotPtr`. Also confirmed: raw roster
position ≠ display slot index — `UpdatePortraits` computes a separate
display-slot remap (gated on `*rosterObj == 2`), meaning the game already
handles party-control switches correctly by recomputing this every frame
— our hook just needs to call the same 3-function chain, not reimplement
the remap rule.

**Recommended hook design**: one small additional detour at
`CSWGuiMainInterface::Update`'s entry (or anywhere per-frame before our
render hook runs) that calls the three existing functions above directly
(no need to hook any of them) to populate a small static 4-entry table
keyed by the absolute `slotPtr` value → `creaturePtr`. Our Target-2
render hook (which does receive `slotPtr`) then looks up `creaturePtr` by
matching `slotPtr` — avoids computing a slot index or splicing into
`UpdatePortraits`'s own loop body.

**R2's other half is still open**: matching a specific drawn icon within
`FUN_007457e0`'s per-icon loop to a specific `CGameEffect` in that
creature's active-effect list. Not moot under the one-hook design — even
reading the effect list fresh every frame, we still need to know which
on-screen icon slot corresponds to which effect. Lead:
`CSWSEffectListHandler::OnApplyEffectIcon`/`OnRemoveEffectIcon` (GOG
`7630000`/`7631264`) suggest a separate icon-application subsystem, not a
direct 1:1 walk — not traced this session.

### R6 (max/total duration field) — RESOLVED, definitively

`CGameEffect::LoadGameEffect` (Steam `0x005e5510`, found as a direct
caller of `SetExpiryTime`) reads a GFF field via the **literal embedded
string `"Duration"`** and stores it as a float directly at
`CGameEffect+0xC`, immediately before `"ExpireDay"`/`"ExpireTime"` →
`SetExpiryTime`. This is a name-confirmed identification, not structural
inference. **Wipe ratio = `(ExpiryTime − currentGameTime) / (effect+0xC)`,
fully derivable with no persistent state of our own.** One residual,
non-blocking gap: confirmed for the save-load path; the runtime
apply-a-freshly-cast-spell path (one of 118 per-type `Apply` handlers)
was not independently traced to confirm it also populates `+0xC` the same
way (near-certain — the constructor defaults it to `0.0` and the engine's
own expiry math depends on it too — but not independently verified).

### R7 (drawing primitive) — partially resolved

Cross-reference search for the icon-widget sub-offsets (`0xab0`/`0x968`)
found `FUN_007450e0`, a GUI-control-lookup-by-tag function that resolves
`"LBL_CMBTEFCTINC%d"` (Combat Effect **Inc**rease = buffs) to `+0x968`
and `"LBL_CMBTEFCTRED%d"` (Combat Effect **Red**uce = debuffs) to
`+0xab0` — the icon-tray sub-objects are ordinary `CSWGuiLabel`
(`CSWGuiControl`-derived) GUI controls, not a bespoke primitive. The
framework's own `Patches/Common/GameAPI/CSWGuiControl.h` has a **named**
vtable-slot enum that exactly matches the two virtual calls
`FUN_007457e0` already makes: `Draw=14` (offset `+0x38`), `SetExtent=1`
(offset `+4`) — independent, name-based confirmation of what was
previously just an empirical `(this,float)` fingerprint guess.

**Important caveat, verified against the `.cpp` not assumed**:
`CSWGuiControl::VTableSlotCount()` explicitly checks
`GameVersion::GetTitle()==KOTOR1 && GetPlatform()==Windows` and disables
vtable overriding for anything else — `CSWGuiControl::OverrideDraw()`
will **not** work out of the box for this K2 mod. The underlying
`VTableOverride`/`VirtualFunctionCall` primitives (`Patches/Common/
VTableOverride.h`) are not themselves version-gated and remain a viable
alternative hook mechanism (per-instance vtable override with full
control over before/after/replace timing, unlike a detour's binary
`skip_original_bytes` choice) if R4's plain-detour site selection runs
into an obstacle — but it needs solving "when do we install the
override on 4 instances," more plumbing than a static-address detour, so
it's not currently recommended over the existing plan.

**Remaining gap**: the icon itself already renders correctly via its own
`Draw` (untouched by our additive hook) — what's still needed is the
**wipe overlay** (a semi-transparent partial-height quad on top). Strong
new lead, not yet followed: the same parent object (via the same
`FUN_007450e0`) also binds `"PB_VIT%d"`/`"PB_FORCE%d"` — vitality/force
**progress bars**, a live, confirmed-working partial-fill widget in this
exact GUI system, on the exact same portrait panel. Tracing their own
`Draw`/fill-percentage logic is a far more concrete next step than
starting from raw D3D primitives.

### R3 (game time) + R5 (spell ID mapping) — not yet resolved, concrete next step recorded

`CSWSObject` has real RTTI on Steam (vftable `0x0099450c`), and its
likely `Constructor` (`0x0053e830`, positional match, not fully
decompile-verified) was found. But it's a 144-function GOG class, and
both R3's lead (`CSWSObject::UpdateEffectList`, GOG `6993840`) and R5's
lead (`CSWSObject::HasSpellEffectApplied`, GOG `7005088`) sit 25–36KB /
100+ functions away from `Constructor` — too far for the cheap
positional-clustering walk that resolved `CGameEffect`. Tried finding
direct callers of the already-known `Apply`/`Remove` dispatchers
(`0x00594740`/`0x005947a0`) as a shortcut anchor — **negative result**,
confirmed virtual-dispatch-only (0 direct callers), don't retry. Recorded
next step: `CSWSObject`'s effect-handling cluster is comparatively tight
in GOG (`ApplyEffect`@6991600 through `RemoveEffectTarget`@6995872, only
4272 bytes / 8 functions) — finding any *one* Steam address in just that
sub-cluster (not via `Constructor`) would let `ListFunctionsInRange.java`
cheaply resolve the rest, the same technique that worked for
`CGameEffect`. Not attempted this session — no promising anchor found yet.

### R4 (precise hook site) — deliberately not attempted

Sequenced last per the plan, and correctly still blocked: R3/R5 aren't
resolved yet (site choice may depend on what data the hook needs to read
inline vs. via a side-table), and the **architecture correction**
(`skip_original_bytes=false` runs our code *before* the original bytes,
not after) means the naive "hook `FUN_007457e0`'s entry" site from the
original Phase-2-era hook plan is wrong — it would draw the overlay
*underneath* the icons. Real candidates (before the final `ret`, vs.
after the inner per-icon `Draw` call inside the loop — the latter risks
splicing across a loop-internal jump target) are recorded but not
evaluated.

### Initial scaffold

`Patches/BuffDurationHUD/` now exists in the `kotor-patch-manager` clone
(never committed to this repo): `manifest.toml` (Steam/LAA only, no GOG
entry — this mod has no GOG counterpart), `BuffDurationHUD.cpp` (every
constant above as a labeled `constexpr int`, tagged STEAM-CONFIRMED /
POSITIONAL MATCH / GOG-ONLY-NOT-YET-FOUND, plus `TODO(R2)` through
`TODO(R7)` markers at the exact point each open item matters), and a
minimal `DllMain` stub. Compiles cleanly with the project's MinGW
cross-compiler (`i686-w64-mingw32-g++-posix -std=c++17 -c`, direct
invocation — not the full `create-patch.py` packaging pipeline, since
`hooks.toml`/`exports.def` are deliberately absent until R4 settles real
hook addresses). No `.kpatch` built, no install attempted.

## UX reconsideration (2026-09-29, user-provided screenshots)

**The existing render target (`FUN_007457e0`, this mod's Target 2 hook
point) is confirmed — by live in-game screenshots the user provided, not
just decompile — to draw a generic buff/debuff TALLY, not per-power
icons.** Three screenshots (player portrait, `PC_Handmaiden`-looking
character) show a teal upward-pointing triangle centered over the
portrait, appearing as 1 small arrow, 1 larger/highlighted arrow, and 3
stacked arrows in the three shots. This is exactly the
`LBL_CMBTEFCTINC%d` widget found in Phase 3's R7 research (`FUN_007450e0`,
"Combat Effect **Inc**rease") — its icon **count** (read from
`slotPtr+0xddc`, R2's still-open "icon count source" question) directly
controls how many arrows stack, each rendered via the same
`SetPosition`+`Draw(dt)` virtual-call loop in `FUN_007457e0`. There is
presumably a mirrored red/downward-pointing stack for
`LBL_CMBTEFCTRED%d` (debuffs), not captured in these screenshots but
following the same mechanism. **This is a tally of how many
stat-increasing effects are active, not an icon identifying which
specific power** — the original Phase 2/3 assumption that this loop's
per-icon rects could be reused to place *named* power icons (Force
Speed's icon, Force Valor's icon, etc.) does not hold; the loop draws the
same arrow glyph N times, it has no concept of "which power."

**User's proposed redesign, not yet decided in detail**: instead of (or
in addition to) this generic arrow tally, show the actual per-power icons
(already identified — see the Effect ID table above, e.g. `ip_speedburst`,
`ip_mind`, `ip_armor`) fitted onto/around the portrait, each with an
integrated visual time indicator directly on the icon itself (rather than
a separate bar or the arrow tally), so the player can see *which* buffs
are active and *how much time is left on each* at a glance, without
needing to interpret an abstract stacked-arrow count.

**Implication for R2**: this is actually a simplification, not a new
blocker. R2's original framing ("match a drawn icon in the existing loop
to a specific `CGameEffect`") assumed we'd be extending the engine's own
icon loop — since that loop turns out to be a content-free tally, there's
nothing there to "match" for a per-power-icon UX. Instead, the icon list
this mod draws would be built entirely from data already resolved (R1's
creature-per-slot chain + the already-known active-effect array at
`ownerObj+0x24`→`+0x148`, filtered by the Effect ID table's spell IDs),
independent of the engine's own arrow-count mechanism. The open question
shifts from "how does the engine decide icon-to-effect" to "how do we lay
out and draw OUR OWN icon row," which is squarely a Phase 3
research/design question, not yet answered:

- **Placement**: the portrait's screen space is already crowded — GUI
  tags found via `FUN_007450e0` include `LBL_CHAR%d` (the headshot
  itself), `PB_VIT%d`/`PB_FORCE%d` (vitality/force bars — very likely the
  red and cyan side bars visible in the screenshots, flanking the
  portrait left/right), `LBL_LEVELUP%d`, `LBL_BACK%d`,
  `LBL_DEBILITATED%d`, `LBL_DISABLED%d`, `LBL_CMBTEFCTINC%d`/
  `LBL_CMBTEFCTRED%d` (the arrow tally, centered over the portrait),
  `BTN_CHAR%d`. A new per-power icon row needs screen space that doesn't
  collide with these — likely a strip along the top or bottom edge of the
  portrait frame, not decided.
- **Coexistence with the arrow tally**: keep the existing
  `LBL_CMBTEFCTINC`/`RED` arrows running untouched (simplest, safest,
  matches this project's established "additive only, never replace
  engine logic" pattern — see `hooks_ported`/every prior hook in this
  framework) vs. suppressing them since they'd become redundant once real
  per-power icons exist (would require an invasive replace-style hook,
  a real departure from this mod's design so far). Leaning toward
  "leave them running" for v1 but not decided.
- **How many icons fit / overflow behavior**: the Effect ID table lists
  15 tracked self-buff powers; portrait real estate is small. Needs a
  concrete max-simultaneous-icons decision and a rule for what happens
  when more are active than fit (unlikely in practice for one character,
  but not zero — e.g. Force Valor + Force Speed + Force Armor
  simultaneously is a realistic 3).
- **What "visual time indicator directly on the icon" means concretely**:
  candidates are (a) a radial/pie "clock sweep" wipe masking the icon —
  visually the cleanest match to the user's ask, but likely the hardest
  to implement on this era's fixed-function-ish render pipeline (would
  need a custom shader, a pre-baked sprite-sheet of wipe stages, or a
  stencil/scissor trick — none confirmed feasible yet); (b) a rectangular
  partial-height/width wipe directly over the small icon (simpler,
  directly reuses the original Phase 2 "cooldown wipe" concept, just
  scaled down onto a per-power icon instead of a separate bar); (c) a
  thin depleting border/ring; (d) numeric countdown text overlaid on the
  icon. Not decided — this is the central open question for the next
  planning session.
- **R7's progress-bar lead is now more directly relevant than before**:
  R7 (Phase 3 findings above) already identified `PB_VIT%d`/`PB_FORCE%d`
  as live, confirmed-working partial-fill bar widgets on this same
  portrait panel, as the recommended next lead for "how does this engine
  draw a partial-fill visual" — tracing their exact draw approach (a
  rectangular wipe, most likely, given typical Aurora-engine progress
  bars) will directly inform which of the (a)-(d) options above is
  actually feasible to build, and is the natural starting point for
  resolving this UX question with real engine data rather than guessing.

No Ghidra work has been done yet against this specific redesign — the
three screenshots are the only new evidence this session. Everything
above is a design question for the next planning session to work
through, not a decision already made.

## UX redesign findings (2026-09-29, follow-on session — item 1 + U1 step 0/U2/U4)

Executing the "Native-Widget UX Redesign" plan that followed the section
above. Static research only — no hooks, no `.kpatch`, no live-game
changes. Full decompile evidence and addresses are in the
`ghidra_knowledge` DB (`lessons` ids 37-39, `classes`/`class_methods`
rows for the 5 GUI widget classes below, `functions` rows for
`00415a60`/`00415d20`) — summarized here.

**Item 1 (naming re-check) RESOLVED**: `0x0074b3f0` (previously recorded
as `CSWGuiMainInterface::Update`, vtable slot 13) is actually
**`CSWGuiMainInterface::Draw`** — confirmed by decompile, not just the
byte-delta arithmetic that first raised the question. The function
itself calls a child sub-object's vtable-slot-14 `Draw(dt)`, calls
`FUN_00745770` (→ `FUN_007457e0`, the confirmed icon-tally render
function) 4×, and loops a 7-slot action-bar array calling `Draw`-shaped
virtuals 7× — i.e. it's the thing that *drives* this panel's whole
render pass, not something driven by a separate Draw elsewhere. It also
decrements several dt-based countdown timers in the same body — this
engine's GUI panels appear to combine per-frame animation-state-advance
and paint into one `Draw(dt)` virtual, not a split Update/Draw pair.
Corollary: since this sits at a real vtable slot (13), the earlier
"Draw is very likely non-virtual, same pattern as `AddModel`" guess
(Phase 3 findings, R7/lesson 25) is **overturned** for
`CSWGuiMainInterface` specifically — Draw is virtual here. The call
graph and every address involved (R1's chain, Target 2's
`FUN_007457e0` hook site) are unaffected — only the name was wrong.
Corrected everywhere it appeared: the `ghidra_knowledge` DB
(`class_methods` row + 4 `lessons` rows got correction addenda) and
`Patches/BuffDurationHUD/BuffDurationHUD.cpp`'s
`CSWGuiMainInterface_Update` constant, renamed to `CSWGuiMainInterface_Draw`
(scaffold still compiles cleanly after the rename).

**U1 step 0 (icon widget's own Draw + wipe-primitive feasibility)
ANSWERED, with a real correction along the way.** The icon widget at
`slotPtr+0x968`/`+0xab0` is confirmed genuinely **embedded** (not a
heap pointer) — `FUN_007457e0`'s own decompile shows the classic
single-dereference virtual-call idiom (`**(code**)(*local_24+4)` where
`local_24 = slotPtr+0xab0`), and reads its own x/y/w/h fields directly
at `+4/+8/+0xc/+0x10`, right after the vtable pointer — a standard
`CSWGuiControl`-shaped layout. Rather than chase this specific
instance's construction site (an expensive, ultimately unnecessary
detour — see below), the shortcut was to note that **vtables are
per-class, not per-instance**: since the icon widget is already
identified as `CSWGuiLabel`-family (R7), finding `CSWGuiLabel`'s own
*static* vtable resolves this for every instance at once, no
per-object construction tracing needed.

**Major finding: all 5 of the framework's own shared GameAPI GUI widget
classes (the ones `Patches/Common/GameAPI/*.h` already model) have real
RTTI on Steam** — unlike the panel-specific embedded structs
(`CSWGuiMainInterfaceChar`/`Action`/`Status`, confirmed zero RTTI in an
earlier session). Found via a plain keyword symbol search, no decompile
needed:

| Class | Steam `vftable` | slots | notes |
|---|---|---|---|
| `CSWGuiControl` | `0x009877d4` | 38 | base; mostly boilerplate, few real overrides |
| `CSWGuiLabel` | `0x009878bc` | 40 | derives Control; slot14 Draw = `0x00419880` (the icon-tally's actual per-icon draw — **not yet decompiled**, next step) |
| `CSWGuiProgressBar` | `0x00987bb4` | 40 | derives Control; slot14 Draw = `0x0041bb20` (PB_VIT%d/PB_FORCE%d's real draw — **not yet decompiled**) |
| `CSWGuiBorder` | `0x009875bc` | 5 | derives `CSWGuiObject`, not `Control` |
| `CSWGuiImage` | `0x0098764c` | 5 | derives `CSWGuiObject`, not `Control` |

This **resolves U2** (Steam addresses for the shared widget classes) far
more cheaply than the planned positional-clustering fallback — RTTI
search first, for any class the framework's own headers already model.

**`CSWGuiBorder`'s Draw/Load/FillTile/FillCenter identities CONFIRMED by
decompile, correcting a same-session mis-match from raw
GOG-address-delta size matching alone**: an initial delta-based guess
mislabeled `CSWGuiBorder`'s vtable slot 4 (`0x00415080`, 639 bytes) as
`Draw` — a real decompile showed it reads GFF/layout keys (`CORNER`,
`FILLSTYLE`, `DIMENSION`, `INNEROFFSET`, `INNEROFFSETY`, `COLOR`,
`PULSING`) into `self`'s fields — it's actually **`Load`**. The real
**`Draw`** is vtable slot 3 (`0x00415300`, 1886 bytes): it draws 4
corner images + tiled edges via two embedded image sub-objects
(`self+0x6c`/`self+0x70`), then dispatches the center fill by a
`fillstyle` field (`self+0x34 & 3`): `0` → `FillTile` (`0x00415a60`,
non-virtual, tiles an image across the rect), `1` → `FillCenter`
(`0x00415d20`, non-virtual, centers one image, no tiling), `2` → a
direct single-stretch virtual draw. (Lesson: GOG-address-delta size
estimation is off-by-one-prone — the gap from `function[i]` to
`function[i+1]` estimates `function[i]`'s size, not `function[i+1]`'s;
always confirm by decompile, never trust delta-size matching alone.)

**U4 (wipe-visual shape) ANSWERED: rectangular, confirmed by decompile,
no radial primitive exists anywhere in this widget family.** Every
fill/draw call found in `CSWGuiBorder::Draw`/`FillTile`/`FillCenter`
takes a plain axis-aligned `(x, y, w, h)` rect plus a color-tint
parameter — no rounding, masking, stencil, or pie-slice primitive
anywhere in the chain. A per-icon time wipe should be a **rectangular**
wipe (shrink one dimension of the drawn rect proportional to the
time-remaining ratio, reusing R6's existing
`(ExpiryTime-now)/Duration` formula) — a radial/pie wipe would need
genuinely new rendering logic (a custom shader or a pre-baked sprite
sheet), nothing reachable from this engine's existing GUI primitives.

**Correction to U1(b)'s framing**: `FillCenter`/`FillTile` are always
**image draws with a color tint**, never flat-color rect fills — U5's
`CSWGuiImageParams::SetImage`/`CSWGuiBorderParams::SetFillImage` become
more directly load-bearing than originally scoped: the wipe mechanism
needs the icon's own resref texture set as the fill image, with the
*rect* shrunk for the wipe, not a separate flat-fill call.

**U1(a) vs (b) tension**: still not explicitly resolved — genuinely
open for the next session. `CSWGuiBorder::FillCenter` being a real,
concrete, decompile-confirmed rect+tint-color image draw makes **(b)**
(draw new per-power icon+wipe elements, K1 `OptionsEditBox.h`-style
manual-draw-from-an-override) considerably more concrete than before —
it's no longer "construct some `CSWGuiImage`/`CSWGuiBorder` and hope,"
it's "construct a `CSWGuiBorder`, `Load` it (or set its fields
directly) with `fillstyle=1`, and call its now-identified `Draw` with a
shrunk rect." Whether to also reuse/repurpose the existing
`LBL_CMBTEFCTINC%d`/`RED%d` tally (option a) is still a live, unresolved
design choice.

**Not yet done this round**: `CSWGuiLabel::Draw` (`0x00419880`) and
`CSWGuiProgressBar::Draw` (`0x0041bb20`) — the *actual* draw functions
for the existing arrow-tally icon and the vitality/force bars
respectively — were found (RTTI-confirmed addresses) but not yet
decompiled. Decompiling `CSWGuiLabel::Draw` next would show exactly
what the current arrow tally renders (confirms/denies whether it's
already texture-based, which would make U1(a) reuse simpler) and
`CSWGuiProgressBar::Draw` remains the fallback lead R7 originally
flagged, in case the `CSWGuiBorder::FillCenter` approach above proves
insufficient. `CSWGuiImage`'s own slots 3/4 (`0x004168b0`/`0x00416720`)
are also still undecompiled (candidates `GetImageExtent`/`Draw`/`Load`).

## UX redesign findings continued (2026-09-29, render-primitive resolution session)

Executing the "Render-Primitive Resolution + Layout/Texture Assignment"
plan that followed the section above. Static research only — no hooks,
no `.kpatch`, no live-game changes. Full decompile evidence in the
`ghidra_knowledge` DB (`lessons` ids 39-43 — id 39 is U4 from last
round, 40-43 are new this round — `functions`/`classes`/
`strings_of_interest` rows for every address below) — summarized here.

**U1(a) vs (b) RESOLVED: (a) — reuse the existing labels, by
repurposing their visual, wins.** Decompiled `CSWGuiLabel::Draw`
(`0x00419880`, 67 bytes) — it is a 2-line forwarder:

```c
void __thiscall FUN_00419880(int param_1,undefined4 param_2)
{
  (**(code **)(*(int *)(param_1 + 0x60) + 0xc))(param_2);
  (**(code **)(*(int *)(param_1 + 0xd8) + 0xc))(param_2);
  return;
}
```

Each call dispatches through vtable slot 3 (`+0xc`) of an embedded
sub-object — the exact same slot `CSWGuiBorder::Draw` (`0x00415300`)
itself occupies. Also decompiled `CSWGuiProgressBar::Draw`
(`0x0041bb20`) for comparison — identical 2-line shape, just shifted
offsets (`self+0x6c`/`self+0xe4`, same 0x78-byte spacing). Conclusion:
**both the arrow-tally label and the vitality/force progress bars are
thin wrappers around two embedded `CSWGuiBorder`-shaped sub-widgets
apiece** — the existing arrow icon is *already* rendered entirely
through the fully-understood `CSWGuiBorder` Draw/Load/CORNER/EDGE/FILL
tag-resource mechanism from last round. This makes reuse (U1a) concrete
and low-effort: retarget one embedded border's FILL-tag resource to a
specific power's icon resref (same setter mechanism as U5 below),
instead of constructing new `CSWGuiBorder`/`CSWGuiImage` objects from
scratch (option b — still documented/feasible from last round, no
longer necessary). **Repurpose-vs-add sub-question, resolved: this
REPURPOSES/REPLACES the tally's own visual per-instance** (the border
currently showing a generic arrow gets its FILL image redirected to a
specific power icon) — it does not draw a new element alongside the
arrow. This directly matches the user's original ask (show *which*
power is active, not an abstract count). Not yet decided: which of the
two embedded borders (`self+0x60` vs `self+0xd8`) is background/frame
vs. fill/icon, and whether the second one doubles as the wipe-overlay
surface (U4's rect-shrink approach) without needing new widgets — next
step if/when development starts.

**U5 steps 1-2 DONE, step 3 CONCLUDED.** String dump confirmed the two
previously-unresolved `CSWGuiBorder::Load` tag literals: `0098762c` =
`"EDGE"`, `00987624` = `"FILL"` (alongside the already-known
`"CORNER"`). All 3 setters `Load` calls right after each tag lookup
were decompiled — `0x00414680` (CORNER), `0x00414760` (EDGE),
`0x00414840` (FILL) — identical shape: release the old resource at
`[self+0x54]+{0x6c,0x70,0x74}` (a vtable-slot-1 call on it, a
refcount-decrement/release idiom) if one is bound, then if the tag
re-resolved, acquire a new one via `FUN_0047eb60` and store it. This is
a **generic release-old/acquire-new resource-swap pattern**, not a
`CSWGuiBorderParams` struct as the framework header models. Also
decompiled both remaining `CSWGuiImage` vtable slots: slot 3
(`0x004168b0`) is `CSWGuiImage::Draw` — confirms vtable slot 3 = Draw
is the universal `CSWGuiObject`-family entry point (matches
`CSWGuiBorder::Draw` at the same slot); slot 4 (`0x00416720`) is
`CSWGuiImage::Load` — reads GFF keys `IMAGE`/`DRAWSTYLE`/`FLIPSTYLE`/
`ALIGNMENT`/`ROTATE(STYLE)` via the same `FUN_00718xxx` helper family
`CSWGuiBorder::Load` uses, and passes the resolved `"IMAGE"` tag result
to `FUN_00416090` (not yet decompiled) — **the single most direct
candidate for "bind a literal resref to an image," the natural next
decompile for a development-phase session.** U5 step 3 conclusion:
**`CSWGuiImageParams`/`CSWGuiBorderParams` as separate framework
classes do not correspond to anything in this decompile** — fields
(`ROTATE`, flip/draw-style bits, etc.) are stored directly on the
widget object itself, and the real mechanism is uniform tag/GFF-key
resource binding across both `CSWGuiBorder` and `CSWGuiImage`.

**Architectural pattern, now clean across the whole widget family**:
`CSWGuiObject`-derived leaf primitives (`CSWGuiBorder`, `CSWGuiImage` —
5-slot vtables) draw via vtable slot 3. `CSWGuiControl`-derived widgets
(`CSWGuiControl`/`Label`/`ProgressBar` — 38-40 slot vtables) draw via
vtable slot 14 (the per-frame dt-based entry point the rest of the
engine calls), and the two leaf `Control`-family widgets examined so
far (`Label`, `ProgressBar`) implement their own slot-14 Draw as
nothing more than 1-2 forwarding calls into embedded `CSWGuiObject`-
family sub-widgets' slot-3 Draw.

**U3 partially resolved.** The `Adjust` candidate
(`0x00745560`, 519 bytes, GOG-named lead) is CONFIRMED a real
sub-control layout-adjustment function — it iterates 8 sub-controls
total: 5 at `this+0x28/0x2c + i*0x148` (i=0..4), 2 more at
`this+0x690/0x694 + i*0x16c` (i=0..1), and 1 more at `this+0xbf8..`,
reading each one's CURRENT `(x,y,w,h)` as plain ints and calling
`SetRect` (`CSWGuiControl` vtable slot 1, `0x00414b70`) with an added
viewport offset. **This is a concrete, decompile-confirmed location to
read every one of this panel's 8 laid-out sub-controls' on-screen
rects** — the actual U3 answer, modulo matching each array index to
its GUI tag name (next step: trace `FUN_007450e0`'s tag-assignment
order, or a live memory read). The `DrawEffects_2` candidate
(`0x00745cc0`) was RULED OUT — it's a visibility/state-toggle setter
(flips a bit, sets a global color/timestamp on activation), not a
layout function; don't retry this lead.

**R3/R5: RTTI-first shortcut tried, negative result, standing next step
unchanged.** Per the carried-forward note, tried the RTTI-first
shortcut before falling back to positional clustering: dumped full
vtables for `CSWSEffectListHandler` (4 slots, all already known:
dtor/Init/Apply/Remove), `CGameEffectApplierRemover` (4 slots, an
abstract base — dtor + 3 pure virtuals), and `CSWSObject` (56 slots
dumped, mostly boilerplate). Neither `UpdateEffectList` nor
`HasSpellEffectApplied` appears in any of these — both are apparently
ordinary (non-virtual) member functions, same pattern as `CGameEffect`'s
own accessors, not reachable via a vtable dump. R3/R5 remain
unresolved; the standing recommended next step (find a Steam address
inside GOG's tight `ApplyEffect`–`RemoveEffectTarget` 8-function/
4272-byte cluster) is unchanged and still the correct path.

**Not yet done**: `FUN_0047eb60` (the resource-acquire call all 3
`CSWGuiBorder::Load` setters use) and `FUN_00416090` (the `CSWGuiImage`
`"IMAGE"`-tag binder) — both are the concrete next-decompile targets
once development starts, for actually writing a resref into one of
these widgets. Which of `CSWGuiLabel`'s two embedded borders
(`self+0x60` vs `self+0xd8`) is which (frame vs. fill) is also not yet
determined — likely needs either a decompile of whatever calls
`CSWGuiLabel::Load` to see construction order, or a live memory dump
of a real instance.

## Resource-bind completion + icon/effect correspondence findings (2026-09-29, follow-on session)

Executed the "Resource-Bind Completion + Icon/Effect Correspondence +
R2/R3/R5/R4" plan. Static research only — no hooks, no `.kpatch`, no
live-game changes. Full decompile evidence in the `ghidra_knowledge` DB
(`lessons` table + `functions`/`classes`/`class_methods` rows for every
address below) — summarized here.

**Item 1 (U5 residual — literal resref-bind mechanism) RESOLVED, with a
recommended entry point.** `FUN_0047eb60` (called by all 3
`CSWGuiBorder::Load` setters and by `CSWGuiImage::Load`) does NOT itself
take a literal resref — it dispatches on whether its one param (a
resolved tag/16-byte-resref-shaped buffer) is null, calling
`FUN_0047cd50()` (blank/default) or `FUN_0047d070(param)` (real). Traced
2 more hops: `FUN_0047d070` placement-constructs a **newly-discovered
real class**, `CAurGUIImageInternal` (derives `CAurGUIImage`, both with
real Steam RTTI — vftables `0x0098c1fc`/`0x0098c228`), then calls
`FUN_004da820(param_2,0,0,0)`. **`FUN_004da820`'s `param_2` is
confirmed a literal `char*` resref** — guarded by
`param_2 != NULL && *param_2 != '\0'` then `strcmp("NULL", param_2) != 0`
(the standard Aurora-engine "NULL"-sentinel-string idiom for "no
resource") before calling `FUN_004da3f0(param_2)` for the real load.
Decompiled `FUN_004da3f0` too (849 bytes) — it turned out to be a
**line-oriented resource-file parser** (opens the resref as a resource,
reads it line by line, trims whitespace, calls a per-line handler), not
a simple texture-handle bind — traced far enough to identify this shape,
deliberately not chased further (disproportionate to the mod's actual
need). **Recommendation: the mod's hook should call `FUN_0047eb60`
directly** (or the already-understood `CSWGuiBorder::Load` FILL setter,
`0x00414840`) with a manufactured resref buffer, going through the
correct release-old/acquire-new dance automatically — not hand-replicate
`FUN_004da3f0`'s undocumented parsing. Also decompiled the small
`FUN_00710xxx` helper family (`00710980`/`00710a70`/`00710b40`/
`00710a10`/`00710ca0`) that prepares literal 16-char resref buffers one
level up from `FUN_0047eb60` — confirms literal-resref-string handling
genuinely exists in this chain, just a couple of hops from where U5
originally stopped.

**Item 2 (U1 residual — which embedded border is frame vs. fill)
RESOLVED.** Found `CSWGuiLabel`'s own `Load`/Initialize override at
vtable slot 18 (`0x004198d0`, decompile-confirmed by ruling out the
other 4 own-override slots 34/35/38/39 as rect/property helpers, not
Load). It calls `Load` (vtable slot 4) on both embedded sub-widgets,
each bracketed by a profiler-scope name call
(`FUN_00733570`/`FUN_00733780`): `self+0xd8` first, scoped by the string
at `0x009876d0`, dumped and confirmed = `"TEXT"`; then `self+0x60`,
scoped by the literal string `"BORDER"`. **Conclusion: `self+0x60` =
"BORDER" (frame/background, leave alone), `self+0xd8` = "TEXT"
(content/fill, the one to retarget for icon-resref repurposing)** —
resolved by decompile, no live memory dump needed.

**Item 3 (U3 residual — tag-to-array-slot matching) FULLY RESOLVED, a
perfect match.** Re-decompiled `FUN_007450e0` in full (previously only
tag lookup *order* had been read) to extract each tag's storage offset.
Converting each to a byte offset and comparing against `Adjust`'s 3
array groups gives an exact 1:1 match for all 8 slots: group 1 (5
controls, stride `0x148`, base `+0x28`) = `LBL_BACK%d`(i0),
`LBL_DEBILATATED%d`[sic — literal game-data spelling, confirmed by the
decompiled string bytes, not a transcription slip](i1),
`LBL_DISABLED%d`(i2), `LBL_LEVELUP%d`(i3), `LBL_CHAR%d`(i4); group 2 (2
controls, stride `0x16c`, base `+0x690`) = `PB_VIT%d`(i0),
`PB_FORCE%d`(i1); singleton (`+0xbf8`) = `BTN_CHAR%d`. (The
`LBL_CMBTEFCTINC%d`/`RED%d` icon-row tags at `+0x968`/`+0xab0` are
confirmed NOT among `Adjust`'s 8 slots — consistent with them being
dynamically iterated by `FUN_007457e0` rather than statically laid out.)

**Item 4 (R2 reopened) DISSOLVED, from existing decompile — no new
Ghidra work needed.** Re-reading the already-recorded `FUN_007457e0`/
`FUN_007450e0` notes directly answered the premise question: the render
loop calls `SetPosition`+`Draw(dt)` repeatedly on ONE embedded object at
a *fixed* offset (`slotPtr+0xab0`/`+0x968`), and `FUN_007450e0` confirms
that offset holds exactly one `CSWGuiLabel`-shaped tag lookup per
portrait (`%d`=portraitIndex only, no per-icon index) — i.e. a single
shared widget stamped at N positions per frame, not N distinct
instances. R2 dissolves exactly as predicted: the hook can freely assign
its own tracked active-effect list to iteration indices 0..N-1 in its
own order, since the engine has no per-icon identity to match against.

**R3 (game time source) RESOLVED (mechanism + call chain), one small
residual.** Rather than retry the RTTI-vtable-dump shortcut (already
ruled out last round), searched Steam **callers of the already-confirmed
non-virtual `CGameEffect` accessors** (`CopyEffect`/`SetExpiryTime`/
`GetExpiryTime`) — this succeeds where a virtual-dispatch-only target's
own xref search fails. Found `FUN_0059de30`, a compound-effect
decomposition helper that calls `FUN_00544210(newEffect,target,0)`
repeatedly — content- and size-matched (1067 bytes vs. GOG's ~1072) to
**`CSWSObject::ApplyEffect`** (GOG `0x6aaef0`). Walking forward from
there, 4 more consecutive functions size-match GOG's named siblings
almost exactly (within ~12 bytes each): `RemoveEffect`(`0x00544640`),
**`RemoveEffectById`(`0x00544790`, also content-confirmed — called by
UpdateEffectList on an expired effect)**, and
**`UpdateEffectList`(`0x00544ad0`, content-confirmed)**. `UpdateEffectList`
iterates the active-effect array (`self+0x148` ptr/`self+0x14c` count),
calls `GetExpiryTime` per effect, and compares the result against **its
own caller-supplied `param_2`/`param_3`** — a (day, timeOfDay) pair.
Tracing `UpdateEffectList`'s 3 callers found the literal time source:
**`FUN_0051ad20(this=worldTimerObj, int* outDay, uint* outTimeOfDay)`**
— decompile-confirmed as an elapsed-tick-based day/timeOfDay clock (or a
cached value when a paused-mode flag is set) with day-boundary-wraparound
normalization, called immediately before `UpdateEffectList` in all 3
sites. This is R3's answer, in the same (day,timeOfDay) units
`CGameEffect::ExpiryTime` (R6) already uses. **Residual**: the
`worldTimerObj this` pointer's ultimate source (`FUN_0051c350`→
`FUN_00538e00`, one more un-decompiled hop) isn't traced — no RTTI found
under WorldTimer/Calendar/GameClock/CExoTimer keywords (0 hits). Concrete
next step: decompile `FUN_00538e00` to see whether it's a fixed global
singleton (ideal) or a handle/id lookup.

**R5 (spell-ID mapping) RESOLVED.** Projecting GOG's offset from
`ApplyEffect` to `HasSpellEffectApplied` (`0x34b0` bytes) onto the
confirmed Steam `ApplyEffect` address landed almost exactly on a real
function boundary: `FUN_005476c0` (101 bytes vs. GOG's 112).
Content-confirmed: it iterates the *same* `self+0x148`/`self+0x14c`
active-effect array `UpdateEffectList` uses, checking each effect's
`+0x1c` field (an int) against a passed-in spell ID, short-circuiting
false if the ID is `-1` — classic `HasSpellEffectApplied(spellId)`
semantics, and an independent cross-confirmation of the array offsets.
**New fields for the mod**: `CSWSObject`'s active-effect array =
`self+0x148`(ptr)/`self+0x14c`(count); effect-struct `+0x1c` = the
spell/feat ID that produced the effect. The render hook can read this
per-effect (alongside the already-known `+0x8` type tag, `+0xC`
duration, `+0x10`/`+0x14` `ExpiryTime`) to decide which power's icon to
show — spellID→icon-resref is now a data lookup (e.g. `spells.2da`), not
a further reverse-engineering problem.

**R4 (precise hook site): still deliberately deferred**, correctly, per
this plan's own gating — R3 has one small flagged residual (not a
re-opened unknown, a single concrete next decompile). Next session:
close R3's residual (cheap, one hop), then attempt R4 — the hook design
is now well-specified: detour `FUN_007457e0`, and *before* each
iteration's native `Draw(dt)` call, retarget the shared icon widget's
`self+0xd8` ("TEXT") embedded border's FILL resource via
`FUN_0047eb60` to the resref for `thisMod'sActiveEffectList[i]`'s power
(read via the effect's `+0x1c` spell-ID field → a resref lookup),
computing wipe state from `CGameEffect::ExpiryTime` vs. `FUN_0051ad20`'s
current time.

## Item 1-4 findings (R3 residual + multi-slot creature resolution + R4) (2026-09-29, 7th session)

Executed the "Close R3 Residual, Multi-Slot Creature Resolution, R4 Hook
Site" plan. Static research only — no hooks, no `.kpatch`, no live-game
changes. Full decompile/disasm evidence in the `ghidra_knowledge` DB
(`lessons` 46-49 + `functions` rows for every address below) — summarized
here.

**Item 1 (R3 residual) RESOLVED — fully global, no per-object context
needed.** Raw disassembly of `FUN_0051c350` (28 bytes) revealed it's
`thiscall` and dereferences its OWN `this+0x4` to get
`FUN_00538e00`'s `this` — the decompiler had hidden BOTH implicit `ECX`
arguments (same class of issue as the R2-era lesson about hidden
`this`/ECX args). Tracing backward from the heartbeat's call site
(`0x00569525`) found the single write feeding `FUN_0051c350`'s own
`this` is a load from a **fixed global pointer**, not anything
`CSWSObject`-specific:
```
this_for_00538e00 = *( *(int*)0x00a1b4a4 + 0x8 ) + 0x4, dereferenced
worldTimerObj = FUN_00538e00(this_for_00538e00, 0x7f000000)   // sentinel forces the default/main slot
```
Passing the sentinel `0x7f000000` as `FUN_00538e00`'s 2nd param
unconditionally selects the default world-timer slot
(`this+0x10048`), skipping its mode/handle check entirely. This is the
**ideal** case the plan hoped for: the render hook can read game time
from anywhere in the process, no `CSWSObject`/creature/handle needed at
all.

**Item 2 (multi-slot creature resolution) RESOLVED — new work, not a
carried-forward TODO.** Full decompile of `UpdatePortraits`
(`0x0074ee90`) found the exact raw-position → display-slot swap logic
(previously only understood at the "gated on `*rosterObj==2`" level):
`local_1c` (the resolved DISPLAY slot, 0..2) and `local_3c`
(`creaturePtr`) are both live right before the function computes the
slot pointer `hudThis + 0x1f34 + local_1c*0xde4` — the exact same
`slotPtr` value `PortraitIconRowDraw` (`0x007457e0`) receives as its own
`this` later the same frame (one HUD instance, identical constants on
both sides). **Design decision: key the side-table by the raw `slotPtr`
address itself, never by a derived index** — only 4 possible `slotPtr`
values exist per session (3 loop slots + 1 "extra" slot), so a 4-entry
linear-scan table needs no reverse index math and no access to `hudThis`
from `PortraitIconRowDraw`'s own call frame, sidestepping the plan's own
open sub-question entirely. **Bonus finding**: the 4th "extra" portrait
slot (`PortraitExtraSlotOffset`, the larger selected-character card the
user's screenshot confirmed as visually bigger) is **not** an
independent roster position — decompile shows it's gated on
`local_1c==0` and simply mirrors whichever creature occupies display
slot 0. The snapshot hook writes both the normal-slot entry and, only
when `local_1c==0`, the extra-slot entry, using the same `creaturePtr`.

**Item 3 (spellID ↔ `spells.2da` sanity check) PARTIALLY RESOLVED,
flagged for empirical verification.** Decompiled `FUN_0059de30`
(compound-effect decomposition — builds sub-effects via
`CGameEffect::Constructor_2`/copy-ctor, so `+0x1c` flows through by
copy, not set fresh here), then `Constructor_2` itself (`0x005e4ac0` —
confirms `+0x1c` is a real copied field, default/unset sentinel is
`-1`/`0xFFFFFFFF`, matching `HasSpellEffectApplied`'s own `-1`
short-circuit exactly), then the primary constructor (`0x005e4920` —
same sentinel init, no explicit spellId parameter). None of these three
functions is where a **real** (non-sentinel) spell ID first gets
written — that happens further upstream (likely the NWScript
command-execution layer, e.g. the `ExecuteCommandApplyEffect*` family,
GOG-only addresses, Steam counterparts not searched). Per the plan's own
effort budget ("don't sink a full session into it"), stopped here.
**What's confirmed**: `+0x1c` is a real, deliberately-propagated field
with the right sentinel semantics — strong circumstantial evidence it's
the spell/feat ID. **What's not confirmed**: whether its numeric value
space is literally `spells.2da` row indices (vs. some other ID space).
**Recommended verification**: once a test hook exists, cast one known
force power, read that effect's live `+0x1c` value, compare to
`spells.2da`'s row for that power — strictly cheaper and stronger than
further static tracing.

**Item 4 (R4: both hook sites, byte patterns, wipe mechanism)
RESOLVED.** Two splice sites found via raw disassembly, both
byte-verified:

1. **Snapshot hook** (item 2's side-table writer), two sites inside
   `UpdatePortraits`: `0x0074f25a` (normal slots 0-2, 6 original bytes
   `89 85 44 ff ff ff`, `EAX`=slotPtr freshly computed, `[EBP-0x38]`=
   creaturePtr stable all iteration) and `0x0074f8ab` (extra/mirror
   slot, only reached when `local_1c==0`, 6 original bytes
   `89 95 a4 fe ff ff`, `EDX`=extraSlotPtr, same `[EBP-0x38]`
   creaturePtr).
2. **Icon-retarget + wipe hook**, inside `PortraitIconRowDraw`'s
   (`0x007457e0`) per-icon loop, immediately before the native per-icon
   `Draw(dt)` virtual call: splice at `0x00745a84`, a clean 13-byte
   instruction-boundary window (`8b 4d e0 8b 11 8b 4d e0 8b 42 38 ff
   d0`) — comfortably enough room for a `JMP rel32` trampoline. Every
   value the retarget logic needs sits in stable `EBP`-relative stack
   slots at this point (the function prologue spills `this` to
   `[EBP-0x4c]` at entry): `slotPtr=[EBP-0x4c]`, loop index (which icon)
   `=[EBP-0x18]`, icon widget ptr `=[EBP-0x20]`, whichRow
   `=[EBP+0x10]` (0=debuff/RED, nonzero=buff/INC — only retarget
   nonzero), `dt=[EBP+0x8]`.

**Wipe mechanism RESOLVED** — decompiling `CSWGuiBorder::Draw`
(`0x00415300`) showed its very first instruction reads `self+0xc`/
`self+0x10` (width/height) directly and both gates and drives its
*entire* layout computation off them — these are **live fields re-read
every `Draw` call**, not cached or passed in by the caller. This settles
U4/R4's last open design question in favor of the simplest option: the
wipe needs no second draw call and no `FillCenter`-parameter
replication — just overwrite the retargeted `self+0xd8` ("TEXT") fill
border's own `+0xc`/`+0x10` dimension fields with a shrunk value
(ratio from `CGameEffect::ExpiryTime` vs. item 1's global time chain,
over `CGameEffect+0xC` duration) before the native `Draw` executes.
Because only `self+0xd8` is touched (not `self+0x60`, "BORDER"/frame),
the decorative frame stays full-size while only the icon image
shrinks — exactly the intended "cooldown wipe over the icon" look.

**Hand-off**: items 1, 2, and 4 are fully resolved with byte-level
precision. Item 3 is flagged, not decompile-confirmed — its assumption
is load-bearing for the render hook's icon lookup, so per this project's
gating discipline, real hook C++/`hooks.toml`/a `.kpatch` build were
**not** started this session absent an explicit user call to accept that
one flagged risk and proceed anyway. Scaffold
(`Patches/BuffDurationHUD/BuffDurationHUD.cpp`) updated with every new
confirmed constant from items 1, 2, and 4 (item 3 documented as an open
TODO in place), recompiles cleanly.

## Goal

A per-frame HUD overlay showing remaining duration on active buffs (Force
Speed, Force Valor, etc.) as a "cooldown wipe" over the power's icon —
native-looking, 60 FPS, no NWScript involved.

## Why this is a Patch Manager task

The Odyssey engine has no concept of "buff" as a first-class HUD object —
Force Speed etc. are `CGameEffect` instances attached to the owning
creature, with an internal `ExpiryTime` (day, time-of-day pair, at
`CGameEffect+0x10`/`+0x14` — confirmed by decompile, see Phase 2 findings)
the game already tracks (that's how the effect expires) but never exposes
to any UI element. Prior
community attempts used NWScript polling to fake duration extensions
(the "long-buff" mods already common in this space) precisely *because*
script execution can't draw arbitrary UI — it's too slow and has no
draw-primitive commands. This only becomes viable with raw C++ hooked
directly into the native render cycle, which is exactly what Patch Manager
provides.

## Implementation pipeline

**Section 2 (render layer) below predates the "UX reconsideration"
section above and describes the ORIGINAL plan (reuse `FUN_007457e0`'s
per-icon loop, draw a wipe over each drawn icon) — now known to be
unworkable as originally conceived, since that loop draws a generic
arrow tally with no per-power icon concept. Kept for historical
reference; the next planning session should treat the render layer as
open, per "UX reconsideration" above, not follow section 2 literally.
Section 1 (data layer) is unaffected and still accurate.**

### 1. Data layer — read the effect list
- Detour into the game's main per-frame update loop.
- Each frame, walk the owner object's active-effect array
  (`ownerObj+0x24`→`+0x148`, `{basePtr, count}` of 4-byte `CGameEffect*`).
- Filter by each `CGameEffect`'s type tag at `+0x8` for the spell IDs we
  care about (full ID table pulled from the installed `spells.2da`, see
  the Effect ID table above, not hardcoded).
- Read each matching effect's duration directly: `CGameEffect+0x10` (day)
  and `+0x14` (time-of-day) — the `SetExpiryTime`/`GetExpiryTime` fields
  (Steam `0x005e4f80`/`0x005e4fa0`), confirmed by decompile. Remaining
  duration = this expiry (day, time) minus the engine's current game time;
  both components are needed, not a single countdown float. No function
  call needed — the fields are directly readable (`CGameEffect` is
  non-virtual/non-polymorphic).

### 2. Render layer — draw the overlay
- Detour `FUN_007457e0` (Steam `0x007457e0`) — confirmed the per-portrait
  icon-row draw function on `CSWGuiMainInterface`'s embedded portrait
  sub-objects (`this+0x1f34 + i*0xde4`, `i`=0..3). Called from `Update`
  (vtbl[13], `FUN_0074b3f0`) via a thin per-slot gate (`FUN_00745770`), so
  hooking it with `skip_original_bytes = false` runs strictly after the
  engine's own per-portrait icon row draws, same frame — matches the
  "draw after the engine's own HUD pass" design exactly. Reuse the
  per-icon rect math `FUN_007457e0` already computes (it loops calling a
  `SetPosition`+`Draw(dt)` virtual pair per icon, scaled by the same
  UI-scale factor `FUN_00479ab0()` our ultrawide `HUD_DESC_COEFF` work
  already accounts for) rather than recomputing icon positions from
  scratch.
- Draw a `.tga` icon for each active tracked power at fixed HUD
  coordinates.
- Draw a semi-transparent quad over it, height scaled by
  `DurationRemaining / MaxDuration`, for the cooldown-wipe effect.

**Open item before Phase 3 hook-writing**: confirm the icon-count bytes
`FUN_007457e0` reads (`slotPtr+0xddc`/`+0xddd`) are actually driven by this
same active-effect list per party member, not a different status-icon
source — not traced this session (see Phase 2 findings above).

### 3. Hook construction (`.kpatch`)
- Two hooks needed (update-loop tick to read effect data + `FUN_007457e0`
  detour for render), likely in the same patch package.
- `skip_original_bytes = false` on both — we are strictly additive, never
  replacing engine logic.
- New `.tga` icon assets: can mostly reuse existing power icons already
  shipped in the game/mod textures; only the overlay quad is drawn
  procedurally (no new art needed for that part).

## Open risks specific to our build

- **HUD coordinate system.** [[kotor2-ultrawide-gui]] already remapped HUD
  description backgrounds and action-description coordinates for
  3840x1600 (`HUD_DESC_COEFF`), and there's an open item there about slot 0
  / slot 5 calibration. Any new HUD overlay must use the *same* coordinate
  transform our ultrawide fix established, not raw 800x600 assumptions —
  otherwise the buff icons will sit in the wrong place only on this build.
- **Effect ID coverage.** Spell/effect IDs must be pulled from this
  install's actual `spells.2da` (TSLRCM, Thematic Sith Lords, Thematic
  Jedi Masters, and other mechanics mods in this build add/change rows —
  mods #8, #102-109, #117-119, #139-142 all touch mechanics/force powers).
  A hardcoded vanilla ID table will silently miss or mis-track powers this
  install has changed. Needs to read the *installed* 2DA, not a reference
  copy.
- **Overlap with "long buff" mods.** None currently installed in this
  build (checked `neocities_mods.json` — no duration-extension mod
  present), so no conflict today, but worth flagging: if one is ever added
  later, this mod becomes strictly better than that workaround and the two
  shouldn't coexist (the long-buff mod would make the timer overlay
  pointless, not simplify it).
- **Multiplayer/party members**: **DECIDED (2026-09-29, 7th session) —
  v1 tracks ALL party members shown in the portrait row, not just the
  player character**, overriding this doc's original player-only
  recommendation (the user's explicit choice). This is what drove item
  2's multi-slot creature resolution work — see the findings section
  above.

## Effort estimate (rough, pre-Ghidra)

Lower than the 3D viewport idea: no new GFF/GUI struct needed (pure
procedural draw), and the `DurationRemaining` field read is a straight
memory offset once `CGameEffect`'s layout is known — likely already
documented in existing community Ghidra databases given how central
effects are to the whole engine. The update-loop and `CUIHUD::Render()`
hook points are also the kind of "well-trodden" addresses more likely to
already be in a shared GZF (worth checking before reversing from
scratch — this is `03_framework_research.md`'s job to confirm).

**Post-Ghidra reality (2026-09-29):** no pre-existing community database had
this — both targets needed reversing from scratch across 3 sessions, taking
noticeably longer than this estimate assumed (`CUIHUD` doesn't exist as a
class at all; the real hook points, `CGameEffect::GetExpiryTime`/
`SetExpiryTime` and `FUN_007457e0`, are documented above and in the
`ghidra_knowledge` DB). Effort was still lower than the 3D viewport mod in
the sense that no new GUI struct was ever needed, matching the original
estimate's core claim.
