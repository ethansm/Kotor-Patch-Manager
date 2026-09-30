# Mod Idea 7: Empty Container Indicators

**STATUS (2026-09-30, research complete, no code):** definitive build plan below. Evidence:
`mods678_research/dump_excerpts/<addr>.{c,asm}`, `check_windows_output_2026-09-30.txt`, knowledge DB (`mods678 batch3`).

Two findings changed the design from the first draft:
1. **KOTOR II is a two-step interface.** Hovering an unselected object shows the generic *select* cursor (`0x2D`); the
   action cursor appears only once the object is already the HUD target. The vanilla "Empty"/"Nothing" name label is
   also **selection-only** (Q1 answered statically, see M7-3).
2. **There is no highlight-everything key in this exe.** `HiliteAreaObjects(on=1)` is complete engine code that nothing
   calls (one call site in the whole image, `0x797EAE`, passes 0). The mod therefore supplies its own trigger and
   tints empties in the engine's own colour code.

## Goal

Show that a container is empty **before** the player walks to it or opens it: a distinct cursor on hover and a grey
tint in the highlight mode.

## Research items (all closed)

- [x] **M7-3 (Q1)** *Is the Empty label shown on hover?* **No.** `UpdateNameLabel 0x742A00` has one caller (`0x743540`), which has one
  caller (`0x74CFE0`, runs from the HUD draw `0x74B3F0`) and reads the HUD target id at `HUD+0x68`. The only setter that writes a non-zero
  target is `OnObjectSelected 0x798D70` (via `0x796B50 → 0x7CE710 → 0x74CFB0`); every other caller clears it (`0,0`).
  `HandleMouseOver 0x7B3A00` has no label-building callee. So "Empty" appears **after the first click** (selection), which is
  vanilla behaviour and needs no change. Evidence: `dump_excerpts/00742a00.c`, `0074cfe0.c`, `00796b50.c`, `00798d70.c`, `007b3a00.c`.
  *One optional in-game confirmation below (check 1).*
- [x] **M7-R1 cursor splice** `SetMouseCursor 0x7B3870` (`thiscall(app; id, a, b)` `RET 0xC`; callers `7B3A00`, `788D90`). Hooking it (not the call in
  `HandleMouseOver`) covers both cursor decisions: `0x2D` before selection and `ActionIconToMouseCursor` result after (`0x3F7 → 0x19`).
  Hovered id = `*(uint*)(app+0x4B0)`; ECX at entry = the same `app` (`CClientExoAppInternal`).
- [x] **M7-R2 highlight tint** `HiliteAreaObjects 0x7AA460`: loop over `area+0x210` (ids, count `+0x214`); `obj = 0x73F4D0(clientApp)(id)->vt[0xC]()`;
  type byte `obj[8]` (5 creature, 6 item, 9 placeable, 10 door); case 9 stores colour (0.3, 1, 1) at `[ebp-0x64..-0x5C]` then
  calls `obj->vt[0xA4](&rgb, 2, 0.0f, 1)`. `[ebp-0xC]` holds `obj`.
- [x] **M7-R3 trigger** No caller passes `on=1` (scan of every `E8`/`E9` rel32 and 4-byte data reference in the image: only `0x797EAE`, `on=0`). Trigger = ours.
  Area object: `appMgr=*(u32*)0xA1B4A4; clientApp=*(u32*)(appMgr+4); internal=*(u32*)(clientApp+4); area=*(u32*)(*(u32*)(internal+0x18)+0x48)` (asm `0x797E8B-0x797EAE`, `0x73FE63-0x73FE6D`, `0x40A6A4-0x40A6B2`).
- [x] **M7-R4 predicate edge cases** `GetItemCount 0x5F67B0` = 0 if `+0x37C` (HasInventory) is 0, else counts every item in the repository `+0x3C4` (count at repo `+0x10`, items via `0x606100(i)`); with `onlyFlagged` it counts item `+0x2C8` bit 3. The only other caller (`0x5499D5`) treats 0 as "no items". Locked/trapped containers use the same rule as vanilla (show empty only if truly zero items). The 11 "loot on open" containers (`kasm22ab`) look empty until opened, exactly like vanilla's label.
- [x] **M7-R5 label on hover** **Not needed for v1** (the cursor is the cue and vanilla already labels on selection). Optional later: change the strref chosen in `BuildHoverActions 0x7B40F0` (`0x16D` Open / `0xBC36` Search).

## Definitive build plan

### 1. Hooks (new DLL `empty-container-indicators`; both Steam SHAs)

| # | Address | Len | Original bytes | skip | Inputs → action |
|---|---|---|---|---|---|
| H7a | `0x007B3870` | 7 | `55 8B EC 51 89 4D FC` | false | `ecx`=app, `esp+4`=address of the `id` stack arg. If `id ∈ {0x2D, 0x19}` and `IsEmptyContainer(*(u32*)(app+0x4B0))` ⇒ `*(int*)p = 1` (plain pointer). Log once per new hovered id |
| H7b | `0x007AA747` | 10 | `6A 01 51 D9 EE D9 1C 24 6A 02` | false | `[ebp-0xC]`=obj, `ebp`. If `IsEmptyContainer(obj)` ⇒ overwrite the three floats at `ebp-0x64/-0x60/-0x5C` with the grey tint (default 0.35, 0.35, 0.35; INI). Runs before the original pushes, so the engine's own `vt+0xA4` call uses the new colour |
| H7c | `0x007AC470` | 5 | `55 8B EC 6A FF` | false | per-frame tick (`CClientExoAppInternal::Update`, `thiscall(app, float)`): poll the hotkey, toggle highlight, refresh |

Windows validated (`check_windows_output_2026-09-30.txt`). Live-hook check: the only live hook inside `0x7AC470` is `0x7AF198` (post-combat-movement-fix, 31 bytes), far from the entry; no overlap with any of the 63 live hooks.
**Collision rule:** do not also hook `0x74B3F0` (Mod 3) or `0x72CB63` (Mod 3 key capture); the tick therefore uses `0x7AC470` and `GetAsyncKeyState`.

### 2. Engine calls

| Call | Address | Signature |
|---|---|---|
| GetClientObject | `0x0073F4D0` | `void*(__thiscall*)(void* clientApp, uint32_t id)` |
| obj→CSWCObject | `obj->vt[0xC/4]` | `void*(__thiscall*)(void*)` |
| obj→client placeable | `c->vt[0x4C/4]` | `void*(__thiscall*)(void*)` (null if not a placeable) |
| GetServerPlaceable | `0x00842D00` | `void*(__thiscall*)(void* cplaceable)` |
| server self | `sp->vt[0x48/4]` | `void*(__thiscall*)(void*)` (vanilla passes its result to GetItemCount) |
| GetItemCount | `0x005F67B0` | `int(__thiscall*)(void* sp, int onlyFlagged)` |
| HiliteAreaObjects | `0x007AA460` | `void(__thiscall*)(void* area, int on)` |

`IsEmptyContainer(obj)` replicates `UpdateNameLabel` `0x742D95-0x742DFD` exactly: `c = obj` (already the `vt[0xC]` result in H7b; in H7a first `o=GetClientObject(clientApp)(id); c=o->vt[0xC]()`), require `*(u8*)(c+8) == 9`; `cp = c->vt[0x4C]()`; `sp = GetServerPlaceable(cp)`; require `*(int*)(sp+0x37C) != 0`; `n = GetItemCount(sp->vt[0x48](), 0)`; empty ⇔ `n == 0`. Every null ⇒ false (never grey by mistake).

### 3. Behaviour spec

- **Cursor:** as H7a. Clicking still works; only the cue changes. Applies to chests and corpses (`UseSearch` irrelevant).
- **Highlight mode:** hotkey (INI `highlight_key`, default `Z`, VK code) toggles `HiliteAreaObjects(area, 1)` / `(area, 0)`. While on, H7c re-calls `on=1` about every 0.5 s (so a looted container turns grey on the next refresh). Auto-off when the area pointer changes, when the window is inactive (`*(int*)0xA1B4E0`), or when the game's own `0x797DF0` clears it. Non-empty placeables keep the vanilla cyan (0.3, 1, 1).
- **Off switch:** INI `enabled=0`, `cursor=0`, `highlight=0`.

### 4. Data / scripts
None. INI only: `enabled`, `cursor`, `highlight`, `highlight_key`, `grey=0.35`, `refresh_ms=500`.

### 5. Build, staged test plan, rollback
- Stage 0 (log only): H7a logs hovered id, type, `HasInventory`, `GetItemCount`, the `id` seen; H7c logs `internal` vs the area chain. Confirms the object chain before any behaviour change.
- Stage 1 cursor → Stage 2 grey tint via the hotkey → Stage 3 refresh/auto-off polish.
- Rollback: remove the DLL entry from `patch_config.toml`, or set `enabled=0`. No game data is touched.

### 6. Remaining risks and in-game checks
1. **Check 1 (optional, 1 min):** hover an empty footlocker (arrow with the mod, select cursor without), click it: the name label shows "Empty". Confirms M7-3.
2. **Check 2:** is `Z` free in the game's key map (INI-configurable if not).
3. `HiliteAreaObjects(on=1)` was never exercised by the shipped game, so it may tint creatures or items in ways the designers dropped (faction colours on creatures, cyan on items/doors). Stage 2 must watch for a stuck tint after toggling off; the engine's off path (`vt+0xA8`) is what `0x797DF0` uses at runtime, so it is exercised.
4. `SetMouseCursor` runs every frame while the mouse moves; the chain costs a few dozen pointer reads, well under a microsecond.
5. The `0x7AC470` entry hook adds a per-frame call; keep the handler to a single `GetAsyncKeyState` plus an early return.
6. Re-calling `vt[0xA4]` every 0.5 s may restart a fade or flicker (untested); if so, call `on=1` only on state change (toggle, area change, container becoming empty).
7. Case 9 tints every placeable whose `obj+0x90` byte is set; only those with `HasInventory` and zero items are greyed (predicate above). `GetItemCount` counts hidden items (bit 8), so a container holding only hidden items stays cyan, which matches vanilla's label rule.
8. Colour constants were decoded on 2026-09-30 with `read_exe_consts.py` (`0x986B60` = 0.3).


## R-A result (user in-game check 1, 2026-09-30)

User report: "containers already show (empty); broken droids show (nothing)". So the vanilla `UpdateNameLabel 0x742A00` Empty/Nothing label is already visible to the player; a K1-style " (empty)" hover-name hook is **not needed** (dropped as Mod 7 stage 1). Whether the label appears on hover or only after selection was not distinguished by the report; Mod 7 scope stays: S0 logging, S1 cursor change for empty containers, S2 highlight hotkey (`Z`, INI). Re-ask only if S1 shows the cursor hook is redundant.
