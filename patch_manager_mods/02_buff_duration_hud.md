# Mod Idea 2: Visual Buff Duration HUD

**Status:** Phase 2 (Ghidra static analysis) **RESOLVED** (2026-09-29, 3rd
session). Phase 3 (hook/C++ construction: research, investigation, initial
development) **IN PROGRESS** as of 2026-09-29 (planning session) — R1
(creature-per-slot) and R6 (max/total duration field) fully resolved, R7
(drawing primitive) partially resolved with a strong next lead, R2's
remaining half / R3 / R5 / R4 have concrete next steps but are not yet
resolved. A real scaffold with decompile-confirmed constants exists at
`Patches/BuffDurationHUD/` in the `kotor-patch-manager` clone (not
committed to this repo — see "Phase 3 findings" below). **No `.kpatch`,
no `hooks.toml`, no live-game changes yet** — Phase 4 remains out of
scope. See "Phase 3 findings" below for the full research trail; it
supersedes the "Open item for Phase 3" note at the end of the Phase 2
findings (that item is now folded into R2).

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
- **Multiplayer/party members**: decide scope up front — player-only buffs
  (simplest, matches the original ask) vs. also tracking active party
  members' buffs. Recommend player-only for v1.

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
