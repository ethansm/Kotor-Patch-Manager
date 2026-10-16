# Doc 22. Force Power Hotbar and shared services (registry / service-provider plan; `kind = "provider"`), 2026-10-03

Status: **design only, nothing implemented.** Input for the 1.0.0 follow-up and the split session's library work (doc 21). Facts come from a read-only
scan on 2026-10-03; raw tables (all 246 hooks with their enclosing engine function) are in the session scratchpad `task9_facts.md`. Every claim
below cites its source file.

Related: doc 19 (logging/registry core), 19b (sharing survey, sections A and D), 19d (registry Phase 1 log), 20 (testing plan), 21 (portrait-slots
split, library category), KPM `docs/PatchRegistry.md`, `Patches/Common/kpatch_api.h`, `Patches/Common/PortraitSlots.h`.

## 1. Where the hotbar stands

- The hotbar uses **no registry API today**: no `KPatchInit`, no `KPatchApi` (grep over `force_power_hotbar_src`).
- The live patcher supports the registry. The installed `KotorPatcher.dll` is md5 `36f10b7f`, byte-identical to the clone's build. It contains
  `KPatchInit` / `Registry::Provide`. (Plan question 6a answered: the pre-registry 2a706ac9 is only in `bin/NoLaunch_old_0928/`.)
- Compile-time sharing is done (19b section D items 2, 3, 7, 8). Consumer contract test: `force_power_hotbar_src/common_contract_test.cpp` (10dfc08).

## 2. Hook ownership map

The hotbar has 8 detours (`kotor2-steam-aspyr.hooks.toml`; Steam only, no GOG hooks file):

| Site | Hook | Engine function |
|---|---|---|
| 0x72CB63 | CaptureHotbarKey | 0x72C7E0 `CExoInputInternal::GetEvents` (DInput) |
| 0x74B3F0 | HotbarHudTick | 0x74B3F0 entry, `CSWGuiMainInterface` per-frame (the toml and code say Draw, the plan said Update; same address) |
| 0x748502 | HotbarBuildButtons | 0x747210 `CSWGuiMainInterface` ctor |
| 0x534527 | HotbarOnSaved | 0x533830 server main loop (save done) |
| 0x53C5AA | HotbarOnAutosaved | 0x53AF20 DoPCAutosave |
| 0x52FD1E | HotbarOnLoadRequest | 0x52FB80 `CServerExoAppInternal::LoadGame` |
| 0x8529E0 | HotbarOnAutosaveSlotLoad | 0x8529E0 entry |
| 0x8FACA0 | HotbarOnNewGame | 0x8FACA0 entry, chargen Play commit |

Findings:
1. **No other patch hooks any of these 8 engine functions.** This holds for every hooks.toml in the stage, repo and clone, and for every
   `code_patches.toml` byte write. Nothing lies within 16 bytes of a hotbar site.
2. The portrait-slots hooks (0x74F25A, 0x74F8AB in UpdatePortraits 0x74EE90; 0x745774 in 0x745770) are in other functions, more than 0x70 bytes
   away from 0x74B3F0.
3. KPM's `HookValidator` (`HookValidator.cs:116-175`) rejects only an **identical address**. Byte-range overlap is not checked there. The repo's
   `scripts/check_hook_spans.py` is the stricter gate (fails on overlap, warns at a gap of 8 bytes or less) and stays the one to run.
4. Side finding (install safety, not hotbar-owned): `hotbar_stage18` carries buff-duration-hud **1.1.0**. That version hooks 0x745774, 0x74F25A,
   0x74F8AB and 0x744EEC, the same addresses as portrait-slots + shield-bar. The stage and the split set are mutually exclusive, so the next hotbar
   install must start from the split session's stage. This is enforced by `kpm_stage_check.py` WARN lines (9d5c843) and the doc 07 install recipe.

Conclusion: the hotbar's hooks are uncontested. A library is only worth it where a **second consumer** needs the same event.

## 3. Consuming `k2.portraitslots@1`

What the interface gives (`PortraitSlots.h`):
- six phases;
- `CreatureForSlot(slot)` and `SlotAt(i, &slot, &creature)`.

Phases fire from the portrait-draw hook, so FRAME_BEGIN only runs while the HUD portrait cards are drawn.

| Idea | Value | Verdict |
|---|---|---|
| Replace our HUD tick 0x74B3F0 with FRAME_BEGIN | Removes one hook | **No.** FRAME_BEGIN does not fire when portraits are hidden. The hotbar needs its tick for the picker, editor and key repeat whenever the HUD is up. It would also make the hotbar depend on a library for core function. |
| Portrait slot -> creature for target mode UI (click a portrait = target; badge on the chosen member) | Real UX gain for UB-U and later | **Yes, as an optional extra.** Use `k2ps::Client` exactly as ShieldBar/Buff do: `init` in `KPatchInit`, `connect` lazily, `poll()` from the HUD tick. No provider = no portrait click/badge, hotbar unchanged. |
| HUD_DRAW for a target badge drawn over the portrait | Draw in the provider's batch | Later; needs the Gui2D path (Buff/Shield use it). |

Requirement: the hotbar stays fully functional with no provider. The manifest gets `requires_interfaces = ["k2.portraitslots@1"]` only as
information, **not** `requires = ["portrait-slots"]`, because a hard require would force-install the library (PatchRegistry.md:296-303). Whether an
optional dependency should be expressible (`optional_requires`?) is an open question for the split session (section 7).

## 4. Interfaces the hotbar could provide (service-provider candidates)

Provider rules (the KPM kind was named "library" until 2026-10-15) (`PatchRegistry.md:305-312`, KPM 94b6b3a):
- a library provides interfaces and owns hooks;
- alone it draws nothing and changes no behaviour;
- consumers stay inert without it and never re-hook the same sites;
- library ids name the shared thing, interfaces are `k2.<name>`;
- every consumer gets all of the library's hooks.

So each candidate below would be a **new library patch** that takes the hooks out of the hotbar, not the hotbar DLL itself. The hotbar becomes its
first consumer.

All tables are pure C: `struct_size` first, append-only within a major, `KPATCH_CALL` on every pointer. The register/unregister token pattern is
copied from `K2PortraitSlotsApi`.

### 4.1 `k2.saveevents@1` (library `save-events`): strongest candidate

Source: the hotbar's 5 save/load/new-game detours (0x534527, 0x53C5AA, 0x52FD1E, 0x8529E0, 0x8FACA0) plus the save-folder parsing
(`*(*(0xA1B4A4)+0x14)`, folder CExoString at +0x18; cloud-save alias 0xA7EA88/0xA7EABC).

```c
typedef struct K2SaveEvent { uint32_t struct_size; int32_t kind; /* SAVED, AUTOSAVED, LOAD_REQUEST, NEW_GAME */
    const char* folder; /* absolute save folder, NULL for NEW_GAME */ int32_t cloud; } K2SaveEvent;
typedef void (KPATCH_CALL *K2SaveCallback)(const K2SaveEvent* ev, void* user);
typedef struct K2SaveEventsApi { uint32_t struct_size;
    int32_t (KPATCH_CALL *Register)(int32_t kindMask, K2SaveCallback fn, void* user, const char* owner); /* token > 0 */
    void    (KPATCH_CALL *Unregister)(int32_t token);
    int32_t (KPATCH_CALL *CurrentSaveFolder)(char* buf, uint32_t cap); /* last loaded/saved, 0 = none */ } K2SaveEventsApi;
```

- **Consumers:** hotbar per-save INI snapshot (today), level-up GUID binding (19b D #9 "needs framework save events"), any future per-save
  config.
- **Fallback:** the hotbar keeps working with no provider, with one global INI (per-save snapshots are skipped and logged once).
- **What moves out of FPH:** 5 detours, folder parsing, cloud alias. The per-save INI copy logic stays in the hotbar.
- **Risk:** callbacks run on the engine thread inside save/load. Consumers must not block, so this goes in the doc text and is enforced by a
  test with a slow fake consumer.

### 4.1a Hotbar migration to `k2.saveevents@1` (owner-gated)

Status 2026-10-12: DONE. Owner GO given; hotbar migrated in ec95c98 (1.3.0-dev); verified by WI-33. (Original 2026-10-04 status: library BUILT and smoked (L3), NOT installed, hotbar NOT migrated.) Everything below is
the plan for the hotbar change, which changes a live mod (hotbar 1.2.1 -> 1.3.0).

**Spec deviations (read first).** `K2SaveEvent.folder` is the save folder qualified with its save root, `saves/000012 - Game11` or
`cloudsaves/<steam id>/000012 - Game11`, relative to the game directory (the process cwd), not a drive-absolute path. It is exactly what the
hotbar's `saveDirFor()` returns today, so the consumer needs no further join. `CurrentSaveFolder` returns the full folder length snprintf-style
(`>= cap` = truncated; always NUL-terminated for `cap > 0`; `0` = none). An event whose folder cannot be parsed is not dispatched (logged).
Sources: `save_events_src/` (`SeDispatch.h` registry, `SaveEventsProvider.cpp`, `kotor2-steam-aspyr.hooks.toml`), client `_shared_src/SaveEvents.h`.

**Hooks the hotbar drops (5 of its 8; Steam hooks toml, byte-identical in `save_events_src`):**

| Address | Hotbar export removed | Library export | Becomes |
|---|---|---|---|
| 0x00534527 | `HotbarOnSaved` | `SE_OnSaved` | `K2SE_SAVED` |
| 0x0053C5AA | `HotbarOnAutosaved` | `SE_OnAutosaved` | `K2SE_AUTOSAVED` |
| 0x0052FD1E | `HotbarOnLoadRequest` | `SE_OnLoadRequest` | `K2SE_LOAD_REQUEST` |
| 0x008529E0 | `HotbarOnAutosaveSlotLoad` | `SE_OnAutosaveSlotLoad` | `K2SE_LOAD_REQUEST` (autosave folder) |
| 0x008FACA0 | `HotbarOnNewGame` | `SE_OnNewGame` | `K2SE_NEW_GAME` |

The hotbar keeps 3 hooks (key capture 0x72CB63, HUD tick 0x74B3F0, button build 0x748502; those are 4.2 / later). Also removed from the hotbar:
`saveDirFor`'s folder parse + cloud alias, `SaveRequestHolder` / `SaveReq_Folder` / `CloudSavesFlag` / `CloudSavesDir`, `HotbarLogic.h`
`normalizeSaveFolder` / `joinSaveDir` (and their tests, which now live in `se_registry_test.cpp`). Kept: the per-save INI copy logic
(`snapshotToSave` / `restoreFromSave` / `replaceLiveProfiles`), now taking the already-qualified directory.

**Consumer code shape (hotbar side):**

```cpp
#include "SaveEvents.h"
static k2se::Client g_se;                                  // lazy require, like k2ps::Client

static void KPATCH_CALL onSaveEvent(const K2SaveEvent* ev, void*) {   // engine thread, inside save/load: cheap, never block
    startOnce(); saferead::beginScope();                   // what each HotbarOn* did first
    if (!K2SE_EVENT_HAS(ev, cloud)) return;                // provider older than v1.0 table (cannot happen today; keeps the gate pattern)
    switch (ev->kind) {
        case K2SE_SAVED:        snapshotToDir(ev->folder, "save written");        break;
        case K2SE_AUTOSAVED:    snapshotToDir(ev->folder, "module autosave written"); break;
        case K2SE_LOAD_REQUEST: restoreFromDir(ev->folder, "save loading");       break;
        case K2SE_NEW_GAME:     replaceLiveProfiles(nullptr, "new game", "-", "new game started"); break;
    }
}
extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_se.init(api, "action-hotbar"); g_se.onConnect = logSaveEventsConnect;
    g_se.add(K2SE_SAVED | K2SE_AUTOSAVED | K2SE_LOAD_REQUEST | K2SE_NEW_GAME, onSaveEvent);
    g_se.connect();
}
// and in the remaining hooks (HotbarHudTick): g_se.poll();   // retries if the provider's KPatchInit ran later
```

**Fallback with no provider.** `connect()` fails soft, `onConnect(false, ...)` logs one line ("k2.saveevents v1 not provided"), and the hotbar
behaves as one global INI: no per-save snapshot on save, no restore on load, no reset on new game (live profiles stay whatever was last
loaded). That is the 4.1 fallback and is strictly weaker than 1.2.1, so the hotbar manifest must declare the dependency
(`requires = ["save-events"]`, `requires_interfaces = ["k2.saveevents@1"]`) and KPM auto-adds the library; the fallback only matters for a
patcher without the registry. `g_se.currentSaveFolder()` is available for a consumer that starts after a load (none needed here).

**Install order (hard rules).**
1. `save-events` and the migrated hotbar go into ONE KPM install, library first in the patch_config order (provider `KPatchInit` runs in
   config order; `poll()` covers the opposite order, but the require window closes after init, so the library must still be installed).
2. NEVER install `save-events` next to Action Hotbar 1.2.1 (or any build that still owns the five addresses): identical hook addresses are
   rejected by KPM. Until the migration, `save_events_src/manifest.toml` declares `conflicts = ["action-hotbar"]`: KPM refuses the pair, and
   `check_hook_spans.py` reports the 5 overlaps (0x52FD1E / 0x534527 / 0x53C5AA / 0x8529E0 / 0x8FACA0) as INFO `declared-conflict` in the
   source scope (still FAIL in the installed scope). The migration commit removes the hotbar's 5 hook blocks AND that `conflicts` entry
   (the migrated hotbar `requires` the library, so a leftover conflict would block the install).
3. Order of work: (a) library already built + gated; (b) hotbar 1.3.0 source change + tests (`hotbar_logic_test` loses the folder fixtures,
   `common_contract_test` gains a `k2se` block per doc 20 4.1); (c) `pre_install_gate.sh --mod save_events_src --mod action_hotbar_src`;
   (d) stage both with `stage_and_verify_install.py`, then a play session: a save / quicksave / autosave / load / new game cycle with the
   `evt` channel (`kplog.ini` `[save-events]` level=info) showing one line per event with `consumers=1`.
4. Rollback = reinstall the previous hotbar 1.2.1 package and uninstall `save-events` together (a 1.2.1 + library mix is the rejected case).

### 4.2 `k2.hotkeys@1` (library `hotkeys`)

Source: 0x72CB63 key capture + the "gameplay HUD active" gate (panel-stack scan with `PanelWhitelist[]` vtables 0x99E5EC, 0x9A044C, 0x9A7BAC,
0x9A8FCC).

```c
typedef struct K2HotkeysApi { uint32_t struct_size;
    int32_t (KPATCH_CALL *Subscribe)(uint32_t dik, uint32_t mods, int32_t (KPATCH_CALL *fn)(uint32_t dik, int32_t down, void* user),
                                    void* user, const char* owner); /* fn returns 1 = consumed (engine never sees the key) */
    void    (KPATCH_CALL *Unsubscribe)(int32_t token);
    int32_t (KPATCH_CALL *GameplayActive)(void); /* HUD up, no blocking panel, not loading */ } K2HotkeysApi;
```

- **Consumers today:** only the hotbar. Possible later ones: mouse_probe, a quick-save or toggle mod.
- **Verdict:** **defer** until a second consumer exists (19b rule).
- `GameplayActive` overlaps A3 `k2.enginestate.modal_top`, so it should live there if A3 happens.

### 4.3 `k2.hud@1` (library `hud-host`)

Source: 0x748502 (HUD ctor: append controls with reserved ids, 75 today) + 0x74B3F0 (tick) + the HUD-rebuilt-on-every-load detail (old labels
leak about 25 KB each, doc 07 build 26 note).

Sketch: `on_build(panel, reserveIds(n) -> first id)`, `on_tick`, `on_destroy`, `active()`.

- **Consumers:** hotbar, a future shield/buff HUD button (the Shield Bar already shifts HUD buttons via .gui), any HUD widget mod.
- Overlaps 19b A2 (`k2.charscreen`, same shape for the character screen) and A3.
- **Verdict:** building now, doc 31 §5, 4 consumers (supersedes "design together with A2"; still overlaps A2's panel-host shape).

### 4.4 A1 `k2.guiinput`: the hotbar as consumer

The hotbar subclasses the game window for the wheel (ForcePowerHotbar.cpp ~738-750/1302). It is one of 4 stacked `SetWindowLongPtrA`
subclasses (inv3d, equip3d, effects, hotbar).

As a consumer it needs:
- `game_hwnd`;
- `cursor_engine(x*, y*)`;
- `wheel_subscribe(cb, ud)`;
- `foreground()` (the click-edge foreground check, Task 2).

The hotbar gives up its subclass when A1 ships, and stays inert-safe: no provider means no wheel in the picker, and arrows still work.

### 4.5 Stays a header, not a service

- Item targeting (`chooseTarget`, vanilla use-on-member route 0x77C780) becomes `Items.h` once a second consumer appears.
- TwoDA/Tlk stay as they are (19b C).
- Textures stay hotbar-only (19b D #10).

**Recommendation order:**
1. `k2.saveevents@1` (two real consumers);
2. hotbar consumes `k2.portraitslots@1` for UB-U target UI;
3. A1 consumer when A1 exists;
4. `k2.hud` together with A2.

`k2.hotkeys` waits for a second consumer.

## 5. GOG route: raw engine addresses in the hotbar

To be handed to the split session's `K2Addr.h` (B1; today untracked in `_shared_src`, table `K2AddrTable` with `kSteam` filled, `kGog` zero =
inert). `21b_gog_address_map.md` does not exist yet (referenced from `shield_bar_src/PULL_REQUEST.md:8`).

**Not in GameAddr.h / K2Addr.h:**

| Steam address | Name (FPH) | Use |
|---|---|---|
| 0x751750 | HudInvokeGroup | thiscall(hud, group, 0) RET 8 |
| 0x51C8B0 | ClientToServerObjectId | thiscall(objTable, id) RET 4 |
| 0x51C0E0 | GetItemByObjectId | thiscall(objTable, id) RET 4 |
| 0x6D6E30 | GetBaseItem | thiscall(item), no args (also InventorySortFilter.cpp:32) |
| 0x5D2F80 | CanUseItem | thiscall(server creature; item,0,1) RET 0xC |
| 0x775600 | PerformUseItem | thiscall(client creature; id,0,target,pos*) RET 0x10 |
| 0x99E5EC, 0x9A044C, 0x9A7BAC, 0x9A8FCC | PanelWhitelist | panel vtables that do not block hotkeys |
| 0x423000, 0x91D288, 0x4491C0, 0x423990 | Tex* | texture ctor, engine new[], pending list add, reset |
| 0xA7EA88, 0xA7EABC | CloudSaves* | Steam cloud flag / dir string (Steam-only by nature) |

Plus the hotbar-owned struct offsets (HUD/manager block ForcePowerHotbar.cpp:53-65, CAurTextureBasic :1197-1199) and the 8 hook sites.

The 12 `gameaddr::` names the hotbar uses are already in K2Addr.h.

**Plan Task 3 (item addresses into `GameAddr.h`) is deferred.** `GameAddr.h` has uncommitted edits by the split session, and `K2Addr.h` (theirs)
is the destination. The constants stay local in FPH, and this table is the hand-off.

## 6. Test strategy for services

1. **Core side (KPM clone):** anything added to the patcher core gets `tests/any_<service>.cpp` + plain-C fixtures in `tests/fixtures/*.c`. These
   build at both widths through `make -C src/KotorPatcher test`, the same as `any_registry` (56 checks) and `any_module_init`.
2. **Patch-provided interfaces (our repo): extract a shared `FakeKPatchApi.h` test helper.** Today the only fake is inline in
   `portrait_slots_src/ps_dispatch_test.cpp:58-80`. The helper offers `fProvide`/`fRequire` with a name+major table, a "registry closed" switch,
   and init-order control. Each library then gets these host tests:
   - layout `static_assert`s (sizes/offsets of every table at -m32 via a T3 test);
   - `struct_size` gating (a consumer given a shorter table treats it as absent);
   - consumer inert without a provider (no callback, no engine call, one log line);
   - init-order independence (consumer `KPatchInit` before the provider's, connected via `poll`);
   - `require` after close returns NULL;
   - re-entrancy (a callback that registers or unregisters);
   - for `k2.saveevents`: folder parsing on fixture CExoStrings, the kind mask, and a slow-consumer guard.
3. **Consumer contract tests:** the hotbar's `common_contract_test.cpp` gains a block per interface it consumes, pinning the fields and phases it
   reads (user rule, doc 20 §4.1).
4. **Static:** `check_hook_spans.py` over the library + all consumers (a library's hooks must not overlap anything), and the manifest's
   `provides` string must equal the `provide()` name in the source (grep check in the T0 tier).
5. **Live:** a probe build of the library + hotbar with logs: connect note, one line per event, a save/load/new-game cycle. Then the doc 20 T5
   play check.

## 7. Open questions

- **Split session:**
  1. Optional dependencies: can a patch declare it uses `k2.portraitslots@1` *if present* without the installer adding the library? Today
     `requires` forces it, and `requires_interfaces` is information only.
  2. Who owns a `save-events` library? Proposal: this session writes it (the hooks come from the hotbar), and the split session reviews it
     against the library rules.
  3. `K2Addr.h` intake format for the section 5 table (fields with Steam values, GOG zero).
- **User:**
  1. Is `k2.saveevents@1` worth doing before the hotbar 1.0.0 PR? Upstream order rule: shared code goes upstream inside the first mod PR that uses
     it (doc 19:126), so it would grow the hotbar PR.
  2. Is portrait click-to-target wanted for UB-U, or is the picker button enough for 1.0.0?
     (10-04: portrait-side buttons are now designed in doc 27 companion self-heal: FRAME_BEGIN click polling, no input swallow; a hotbar-owned click-absorbing label via `hud-host` would remove its click-through risk.)

- [x] 4.1a DONE (owner GO confirmed 2026-10-12; migration landed in ec95c98 (2026-10-08); WI-33 verify PASS 1f1f032: 5 hooks gone, `k2se::Client` consumed, conflicts cleared, tests 4/4, build_check FAIL=0).
