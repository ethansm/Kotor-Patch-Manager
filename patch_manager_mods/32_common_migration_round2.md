# 32 — Common extraction round 2: per-mod migration plan

Date: 2026-10-05. Master plan: doc 24 rows C7-C10 + "MG1-MG5 → doc 32".
Direction: "start extracting the common patch logic and then plan to migrate the patches after."

This round added Common APIs only (additive, host-tested, mutation-checked). **No mod was edited.**
Each batch below is run later by the session that owns the mod. Status starts TODO.

## RESUME HERE

MG1 DONE (source) 2026-10-05: texmem aeb60ff, equip-icon 62001fe, activation f8788ce (KPM 3c8ef31/dd9a37d/7b0d9c7), all 1.1.1, INSTALLED 10-05 03:26 (user GO) by DLL hot-swap (shield-bar/perf-lab/mouse-probe live DLLs match no kpatch, so no KPatchLauncher): live dlls texmem b1427c71, equip-icon b67d59e0, activation f538eb4d (KPM kpatch builds; gate md5s differ by build path); config 818ba851 unchanged; backup backups/mg1_swap_20261005_0326; stage mg1_stage_1005 head 7b0d9c72. Live check next launch: `kotor2_logs.py check` = same 1.1.0 load lines (texmem DISABLED). Next: MG2; perf-lab cave migration deferred (owner editing). Handoff: `.claude/handoffs/32_common_migration_round2.handoff.md`.

## 1. What landed (round 2 headers)

| Row | Header | API | KOTOR-II sha | KPM sha |
|---|---|---|---|---|
| C7 | `IniKit.h` (new) | `inikit::trim`, `parseFloats`, `splitIniLine`, `forEachLine` (+`LoadStats`, `IniVerdict`), `enabledFlag`, `enabledFile` | 67ef3c3 | 564d315 |
| C9 | `PanelSlots.h` (new) | `panelslots::PanelSlots<Slot,N>`: `find`, `claim(panel, seq, &fresh)`, `release`, `forEach`, `count`, `onEvict` hook | 21d5c9b | 4df4a29 |
| C10 | `K2Addr.h`, `CodePatch.h`, `Gui2D.h` | `k2addr::featureOk`, `codepatch::allocCave`, `gui2d::Rect` / `vfillClip` / `drawVFill` | 60b61b4 | 6cfa7e2 |
| C8 | `GuiKit.h` | `cursorEngineF`, `syncScreen`, `WheelHook` + `wheelInstall`/`wheelUninstall`, `bindByTag` (+ pure helpers) | a3286fe | c060999 |
| C11 | `WindowHook.h` (new; F28 design A, note 20f) + GuiKit wheel on it | `kp::WindowHook<Tag>`: `install(hwnd, proc)` (pins module once), `active()`, `forward(h,m,w,l)`, `uninstall(proc)` (restore if on top, else pass-through); registry `uses = "WindowHook"` | 7d41942, 7aca7fd | d46a699, f03c258 (`_shared` forwarder) |
| C12 | `StrFmt.h` (new; F26) + `GameAddr.h`/`K2Addr.h` names (F20) | `strfmt::appendf` (byte-identical to `bh::`/`sb::appendf`, drift-tested); `gameaddr::GetBaseItem`, `ServerObjectGetArea`, `AppActiveFlag`, `MovieRestoreSkipFlag`, `ClientCreatureVtable`, `WorldTimerGetCurrentTime` (+ K2AddrTable fields) | 84e9a7c, dca7bfc | d18015c (+`_shared` forwarder), d6aaaec |

Semantics worth knowing before migrating:
- **IniKit covers the trio dialect only** (inv3d / equip3d / chareffects) plus the `.txt` kill switch.
  Do not move other mods' INI code onto it. Their dialects differ in bool meanings, comment rules, sections and float grammar.
- `enabledFlag` keeps the `configEnabled()` quirks exactly. Prefix match: `enabledX=0` matches key `enabled`, and its value char is `X`, so it turns the flag **ON** and overrides an earlier `enabled=0`.
  `enabled=off` is ON. `enabled = 00` is off.
- `enabledFile` reads the whole file (64 KB cap); the old copies used `fgets` with a 128-byte line buffer.
  The only behaviour difference is for lines longer than 127 chars, where the old copies split the line. No real toggle file has such lines.
- `PanelSlots::claim` on a panel that already has a slot returns that slot unchanged and sets `*fresh=false`; it does not call `onEvict`.
  A mod that tears down on re-bind (equip3d) checks `fresh`. `release()` never calls `onEvict`.
  Panel 0 is an ordinary key, so mods that treat 0 as "no panel" must reject it first.
- `featureOk` logs shield_bar's text: `K2ADDR <feature> disabled (...)`. Buff also writes its own `logf` line, so buff must wrap `log` to keep that line.
- `vfillClip` rounds half up in float. Buff already does; shield did it in double on `cur/mx`, which can differ by 1 px only at exact .5 boundaries.
- `allocCave` is never freed; host build is calloc-backed.
- Reload stays on `filewatch::changed` (FileWatch.h). No new reload code.

## 2. Migration batches

| Batch | Mods | Adopt | Owner / session | Gate | Expected log diff | Behaviour-neutral checks | Status |
|---|---|---|---|---|---|---|---|
| MG1 quiet mods | texmem, equip-icon, activation (`.txt` toggles); texmem, perf-lab (cave) | `inikit::enabledFile` replaces `configEnabled()`; `codepatch::allocCave` replaces the VirtualAlloc scaffold | free (C5a session done) | per-mod `common_contract_test.cpp` pins `enabledFlag` + `allocCave`; `pre_install_gate` x3; `--wine` tier | none | toggle file with `enabled=0` → disabled; missing file → enabled; cave bytes identical (dump first 32 bytes before/after) | DONE (source) 10-05: texmem aeb60ff (dll bb13becd), equip-icon 62001fe (bf53f073), activation f8788ce (a6d89cbe); manifests 1.1.1, load lines still say 1.1.0 (perf_lab_src/harness smokes pin " 1.1.0 loaded: "); gate PASS x3; perf-lab deferred (owner editing); INSTALLED 10-05 03:26 by DLL hot-swap (live b1427c71/b67d59e0/f538eb4d; backup mg1_swap_20261005_0326); live log check pending |
| MG2 split HUD | shield-bar, buff-duration-hud | `k2addr::featureOk`, `gui2d::drawVFill` / `vfillClip`, `strfmt::appendf` replaces `bh::`/`sb::appendf` (F26; then drop shield_logic_test section 8 drift guard) | buff/shield split session (doc 21) | contract tests; F12 screenshot compare of full / half / empty bars | buff: same lines if `log` is wrapped; shield: none | fill heights equal at 0 / 0.5 / 1 and an odd height (±1 px only at .5 edges) | DONE (source) 10-05: `strfmt::appendf` in both (buff 2ba0875, shield df9fa9e), drift guards dropped f73d632; `k2addr::featureOk` + `gui2d` fill adopted via F34 (int-ratio `drawVFill` 52e6e81, shield 6d51383, buff wrapper d516e1f; KPM 8618433, c4c02ec, ef15824); strfmt part INSTALLED 10-05 (BD3), F34 DLLs INSTALLED 10-05 |
| MG3 inv3d | inventory_3d_viewport | IniKit `forEachLine` + FileWatch reload, GuiKit `WheelHook` / `cursorEngineF` / `syncScreen` / `bindByTag`, `PanelSlots`, KpLog | doc 23 session | inv3d host tests + contract test; live: open inventory, wheel-zoom, drag, close/reopen | KpLog prefix change only | zoom step per notch unchanged; drag guard still blocks wheel while dragging; slot reused on reopen | TODO |
| MG4 equip3d | equipment_3d_character | MG3 set + SafeRead for its 14 `IsBadReadPtr` sites (= C3, after S1d) | doc 23 session | as MG3 + `--wine` | IsBadReadPtr fault storms gone under Wine | destroy-on-rebind via `fresh=false`; eviction calls `onEvict` once | TODO |
| MG5 chareffects | active_effects_panel | MG3 set + GuiKit setters + `party::` leader + `k2fx::currentTime` / `isItemAbility` (= C4) | doc 14 session | contract test; doc 14 build-4c checklist | KpLog prefix change only | `bound` flag lives in the Slot payload; same rows / icons on the effects page | TODO |
| MG6 WindowHook | inv3d, equip3d, chareffects, action hotbar (one commit per mod) | `kp::WindowHook<Tag>` replaces each mod's `g_oldProc` + SetWindowLongPtr install / restore-if-on-top (or GuiKit `wheelInstall` where MG3-MG5 already adopt `WheelHook`); proc = `if (Hook::active()) {...} return Hook::forward(...)` | owning sessions: doc 23 (inv3d, equip3d), doc 14 (chareffects), doc 31 (hotbar) | `check_subclassers.py` after flipping the mod's `subclassers.toml` entry to `uses = "WindowHook"`; the LAST mod also flips the F28 CHARACTERIZATION asserts in `tests/wndproc_chain`; `--build` | none | wheel / input behaviour unchanged with that mod alone; unload order irrelevant (pass-through) | DONE (source) 10-05 c2be15c 39d2921 a104576 1af7130 + registry c368b5e, F28 relabel 9134fe7; KPM mirror complete 10-05 (inv3d 272efc2, equip3d b0a5ac6); NOT installed, play check pending |
| MG7 GameAddr names | action hotbar, inventory sort, companion-ai, ranged-ai, activation-fix, load-hang-probe, chareffects, equip3d (one commit per mod) | local `constexpr` copies of 0x6D6E30 / 0x5453C0 / 0xA1B4E0 / 0xA1B76C / 0x99EE14 / 0x51AD20 -> `gameaddr::` names (C12, F20) | owning sessions per mod | `check_raw_addresses.py` (the mod's INFO lines go away), `--build` | none | same constants, so code is identical: compare `objdump -d` of the rebuilt DLL before/after | DONE (source) 10-05 ec4f6b6 a964f40 9760788 157af10 35e0146 + hotbar c2be15c, chareffects 39d2921, equip3d 1af7130; KPM mirror complete 10-05 (equip3d b0a5ac6, ranged-ai 8d46009); NOT installed |

Rules for every batch:
- One mod per commit; commit with explicit paths (`git commit -- <paths>`).
- Add each newly used API to that mod's `common_contract_test.cpp` ("<mod> relies on: ...").
- Commit the mod in `ethansm/local-mods` before staging (`stage_and_verify_install.py`).

## 3. Traps (record, don't "fix" in passing)

- `guikit::setVisible` is **not** a drop-in for the mods' `setCtlVisible`: it clears bit 0x20 on show (ActionHotbar.cpp:688 note). Check each call site before swapping.
- The `WheelHook` window subclass is per-DLL. L4 `k2.guiinput` is the long-term owner of input subclassing. Once it exists, MG3-MG5 move to it, and the per-DLL hooks go.
- `check_shared_headers.py --sync` pushes **every** drifted `_shared_src` header to KPM, including other sessions' uncommitted ones. Mirror by hand when others are active.

## 4. Recorded, no action this round

- letterbox: `Rd` → `saferead`, `VirtualProtect` → `codepatch`.
- companion_ai: check its controlled-creature lookup is equivalent to `party::controlledCreature` before swapping.
- 12 hand-rolled `logf`s → KpLog (doc 19f; C5b ranged_ai, C5c perf-lab + robustness).
- Rect dedupe across mods: skip (small win, name clashes). `gui2d::Rect` is namespaced. No mod does `using namespace gui2d`; only shield/buff contract tests do, and they pass.

### Findings to forward (owners)

1. **selfheal** (`companion_selfheal_src/CompanionSelfHeal.cpp:87-93`): `IniPath = ".\\companion_selfheal.ini"`, but `GetPrivateProfileIntA(..., "companion_selfheal.ini")` at :90 and :93 uses the bare name.
   A bare name resolves to the Windows directory, so `[General] Log` and `[probe] call` are read from the wrong file and always take their defaults.
2. **perf_lab** has no `common_contract_test.cpp`, although it uses 5 Common headers.
3. **chareffects** (`CharEffectsPanel.cpp:36`) and **buff** (`BuffDurationHUD.cpp:25-30`) include `../_shared/...`. That path exists only in the KPM tree, so these files don't build from `patch_manager_mods/` alone. Switch them to `#include "<X>.h"` when migrating.

## 5. GuiKit round (C8)

- `bindByTag(panel, const char* tagName, expectId, labelSize = 0, BindInfo* info = nullptr)`. The tag is the layout tag **string**, as in the copies, not an int.
  `labelSize <= 0` uses the K2Addr table's `LabelSize` (the copies hard-coded 0x148). `BindInfo` carries the fields the copies logged (`H1 ...`), so mods keep their log line.
- `WheelHook` / `g_wheel`: one per DLL. Per tick: `touch(GetTickCount())`, `setRect(i, ...)` (up to 4 rects, engine px), then `take()`.
  Copy differences are kept as fields: `enabled`, `freshMs = 300`, `scoped` (inv3d calls `saferead::beginScope()`), `onEvent` + `eventLogMax = 20`.
- None of A/I/E had a drag guard in the wheel proc (inv3d's drag runs in its tick). `dragGuard` is optional; while it returns true the wheel is chained to the game, not consumed.
- New compared with the copies:
  - the accumulator saturates at ±2^20 (the copies could wrap);
  - `wheelUninstall()` restores the proc only if ours is still on top (`tests/static/subclassers.toml` rule).
- `cursorEngineF` falls back to client px when no screen size is known, like `cursorEngine`. The copies did not fall back that way, so check each MG3-MG5 call site.
- Portability: the wheel code uses GCC `__atomic_*` builtins. Fine for the mingw mod build; MSVC upstream would need `Interlocked*`. Raise it before any upstream PR that carries GuiKit.h.
- Only the pure core is host-tested; the Win32 proc glue passed a mingw `-fsyntax-only` check. MG3 is its first live exercise: log wheel events (`onEvent`) on the first install.
