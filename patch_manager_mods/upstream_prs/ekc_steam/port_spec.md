# ExpandedKeyboardControl (EKC): Steam Windows port spec (task R2, read-only research)

Target: KOTOR II Steam Aspyr, `swkotor2.exe` sha256 `6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF` (image base 0x400000; the local vanilla copy hashes to exactly this).
Source: upstream/master export `Patches/ExpandedKeyboardControl` (manifest 1.18.0, GOG K2 `777BEE23...`, K1 GOG `9C10E045...`).
Deliverables beside this file: `kotor2-steam-aspyr.hooks.toml.draft` (complete), `ExpandedKeyboardControl.cpp.diff` (the C++ change, compile-checked, see section 4).

## 0. Method and headline

- No GOG exe on disk and no GOG disassembly. GOG facts come from the server exports in `ghidra_dumps/upstream_server/k2_win_gog_aspyr_swkotor2.exe/` (`fp.tsv` = per-function size, `ret N`, insn count, callees, string refs, **every `[reg+disp]` and immediate**; `swkotor2.exe_symbols.tsv`; `strings.tsv`) and the GOG original_bytes in upstream's toml. Steam facts come from the exe itself, the Steam dump `decomp/*.asm|.c`, and the Steam server export.
- Matching rule: a GOG function and a Steam function are "the same" only when size, `ret N`, instruction count, the full sorted `disp` list and the full sorted `imm` list are all identical, plus callee topology agrees. Every function below passes (zero tolerance). The (size, ret, insn, disp, imm) signature is unique in the Steam binary for ProcessInput, Draw, BufferEvent, HandleInputEvent (manager and 4 panels), GetIsSelectable, SetActiveControl, HitCheckMouse, activate, cancelLastAction, the movie wndproc and StartMovie. It is NOT unique for the four 170/167-byte prev/next functions (each pair shares one shape) and for the 44-byte destructor thunk (~230 identical shapes); those are pinned by callee identity instead (see the constants table and hook 5).
- **GOG->Steam is NOT a constant delta for this patch.** CSWGuiMainInterface is +0x22E880 (the delta the hotbar mod found), GetEvents +0x119300, BufferEvent +0x119250, but CSWGuiPanel/CSWGuiControl/CSWGuiManager moved to 0x40xxxx-0x41xxxx, ProcessInput to 0x7AC470 and the movie window proc to 0x40C4A0. All were found by fingerprint, not by delta.
- Result: 6 of 6 hooks found, each with byte-identical `original_bytes`, all on instruction boundaries; all 16 K2 address/vtable values resolved (5 vtables + keyboard index + 10 functions); every struct offset and GUI event id is unchanged (identical `disp`/`imm` sets in every fingerprinted function).
- Two of the six hook sites (0x0072CB63, 0x0074B3F0) are already live Steam detours in our ForcePowerHotbar mod (same bytes, same registers), which is independent corroboration for those two.

## 1. Hook table (GOG -> Steam)

"Uniq" = hits of an exact byte pattern over the whole `.text` (0x401000..0x985... , 0x5846A8 bytes). The bare original_bytes are too short to be unique for five of the six (counts in column "bytes only"); the context pattern is original_bytes plus neighbouring exact bytes.

| # | function (export) | GOG addr | Steam addr | original_bytes (both, identical) | bytes only | context pattern (hits) |
|---|---|---|---|---|---|---|
| 1 | CaptureActionBarInputK2 | 0x00613863 = `CExoInputInternal::GetEvents`(0x6134E0)+0x383 | **0x0072CB63** = FUN_0072C7E0+0x383 | `89 85 BC FE FF FF` | 83 | `14 03 84 0D 04 FF FF FF` + orig + `83 7D F0 02 7D 0B 8B 4D` (**1**) |
| 2 | MapMovieDeleteToEscapeK2 | 0x0091F696 = `ExoMoviePlayerWindowProc`(0x91F690)+6 | **0x0040C4A6** = FUN_0040C4A0+6 | `8B 45 0C 89 45 CC` | 1 | orig alone (**1**) |
| 3 | DispatchContextCancelK2 | 0x00459080 = `CClientExoAppInternal::ProcessInput` entry | **0x007AC470** = FUN_007AC470 entry | `55 8B EC 6A FF` | 2803 | orig + `68 F7 83 96 00 64 A1 00` (**1**) |
| 4 | UpdateActionBarControlsK2 | 0x0051CB70 = `CSWGuiMainInterface::Draw` entry | **0x0074B3F0** = FUN_0074B3F0 entry | `55 8B EC 6A FF` | 2803 | orig + `68 E8 30 96 00 64 A1 00` (**1**) |
| 5 | ClearActionBarControlsK2 | 0x0051BEC0 = `~CSWGuiMainInterface` entry | **0x0074A750** = FUN_0074A750 entry | `55 8B EC 51 89 4D FC` | 2406 | orig + `8B 4D FC E8 31 06 00 00` (**1**) |
| 6 | CancelActionBarKeyboardFocusOnMouseMoveK2 | 0x00614E6C = `CExoInputInternal::BufferEvent`(0x614DE0)+0x8C | **0x0072E0BC** = FUN_0072E030+0x8C | `B8 01 00 00 00` | 2359 | `55 E4 89 8A E8 03 00 00` + orig + `E9 33 06 00 00 8B 45 E4` (**1**) |

Note: the context patterns for 3/4/5 contain the build's SEH-handler/rel32 operands, so they prove uniqueness inside the Steam exe only (they differ from GOG by design). No hook needs `skip_original_bytes = true`; all six are plain detours that re-run the stolen bytes, exactly as in the GOG toml.

### Per-hook evidence, instruction-boundary and frame/register check

**Hook 1, 0x0072CB63 (GetEvents DirectInput record loop).**
- GOG function: `CExoInputInternal::GetEvents`, which Ghidra splits into four chunks on GOG (0x6134E0 sz 2768, 0x613FB2, 0x614520, 0x614678). Steam keeps it as one 5830-byte function FUN_0072C7E0 (ret 12, 1233 insns; string refs 0x99C810/0x99C828 = the `.\exoinputinternal.cpp` / `CExoInputInternal::GetEvents()` assertion strings, GOG 0x9974A0/0x9974B8). Offset +0x383 is identical in both builds.
- Steam disassembly (hook insn is the 6-byte store, one whole instruction):
  ```
  72cb52  mov  eax,[ebp+edx*4-0x134]   ; dword index
  72cb59  imul eax,eax,0x14            ; record size 0x14
  72cb5c  add  eax,[ebp+ecx-0xfc]      ; + base pointer
  72cb63  mov  [ebp-0x144],eax         ; HOOK (6 bytes)   EAX = &record {dwOfs @+0, dwData @+4, ...}
  72cb69  cmp  dword ptr [ebp-0x10],2  ; [ebp-0x10] = device loop index
  ```
- Frame check: C++ reads `eax` (record pointer) and `ebp`, then `*(int*)(ebp-0x10)` as the device index. On Steam `[ebp-0x10]` is the same device loop counter (`local_14` in the decompile, used as `auStack_138[local_14]` / `local_100[local_14*3]`, and `mov dword [ebp-0x10],2` at 0x72C98B restarts it for devices >= 2). The ForcePowerHotbar detour at this exact site already passes `eax` and `[ebp-0x10]` and works live. Differences: none.
- Keyboard device index global (see constants) is read inside this same function (0x72C96A `mov eax,[0x99C884]; imul eax,eax,0xc` = index into the per-device 12-byte structs), which confirms it is the same kind of constant.

**Hook 2, 0x0040C4A6 (movie window proc).**
- Steam FUN_0040C4A0 (sz 693, ret 16, 186 insns, imm/disp lists identical to GOG 0x91F690). It is the `lpfnWndProc` stored by FUN_0040CC70 (the function that registers class "SWMovieWindow", title "SW Movie Player Window"), and FUN_0040CC70 fingerprints identically to GOG `CExoMoviePlayerInternal::StartMovie` 0x91FE60 (sz 1975, 468 insns). Only one 4-byte reference to 0x40C4A0 exists in the image (0x40CCD2, inside FUN_0040CC70).
- Disassembly: `40c4a0 push ebp; mov ebp,esp; sub esp,0x3c` then `40c4a6 mov eax,[ebp+0xc]` / `40c4a9 mov [ebp-0x34],eax` (the 6 stolen bytes = 2 whole instructions). No branch targets 0x40C4A9 anywhere in the dump.
- Frame check: C++ reads `ebp`, then `[ebp+0xC]` (uMsg) and `[ebp+0x10]` (wParam) and rewrites wParam. Steam: `[ebp+0xC]` is uMsg (`mov eax,[ebp+0xc]; cmp ...,0x201`), and the `case 0x100` (WM_KEYDOWN) branch tests `param_3 == 0x1b` (VK_ESCAPE) then calls FUN_0040CC30(0,0) (stop movie). So Delete(0x2E) -> 0x1B behaves as on GOG. Differences: none.

**Hook 3, 0x007AC470 (ProcessInput entry).**
- Steam FUN_007AC470: sz 15798, ret 4, 3509 insns, 82 callees, **disp and imm lists identical** to GOG `CClientExoAppInternal::ProcessInput` 0x459080 (sz 15798, ret 4, 3509 insns); the only Steam function with that (size, ret, insn) triple. Callers of the Steam `HandleInputEvent` FUN_00410AA0 are exactly FUN_007AC470 and FUN_007B12C0, matching the GOG role.
- Disassembly: `7ac470 push ebp; 7ac471 mov ebp,esp; 7ac473 push -1; 7ac475 push 0x9683f7; ...` (5 stolen bytes = push/mov/push, all whole instructions; no branch targets 0x7AC471/0x7AC473).
- Frame/register: hook reads only `ecx` as a pointer and null-checks it (`clientApp`). Thiscall on both. Differences: none.

**Hook 4, 0x0074B3F0 (CSWGuiMainInterface::Draw entry).** Fingerprint identical to GOG 0x51CB70 (sz 3874, ret 4, 897 insns, 36 callees, identical disp/imm). Same hook already live in ForcePowerHotbar (`HotbarHudTick`, ECX = HUD). Delta to GOG +0x22E880. Stolen 5 bytes = `push ebp; mov ebp,esp; push -1`. Differences: none.

**Hook 5, 0x0074A750 (~CSWGuiMainInterface entry).** The 44-byte scalar-deleting-destructor shape is not unique (about 230 twins), so it is pinned by two content anchors. (a) RTTI: it is slot 0 of the Steam `CSWGuiMainInterface` vftable 0x0099DCC4 (type descriptor `.?AVCSWGuiMainInterface@@`). (b) Callee: it calls 0x74AD90, which is the unique fingerprint twin of GOG `CSWGuiMainInterface::~CSWGuiMainInterface` 0x51C510 (sz 1551, 12 callees, identical disp/imm), exactly as GOG 0x51BEC0 calls 0x51C510 (the delta of this thunk is +0x22E890, `drift+0x10` in `engine_research_findings/U4_map_CSWGuiMainInterface.tsv`, so a constant delta would have been off by 0x10). Disassembly `74a750 push ebp; mov ebp,esp; push ecx; mov [ebp-4],ecx` = exactly the 7 stolen bytes (4 whole instructions). `ecx` = this on both. Differences: none.

**Hook 6, 0x0072E0BC (BufferEvent, mouse-move case).**
- Steam FUN_0072E030: sz 1743, ret 4, 444 insns, 1 callee, disp/imm identical to GOG `CExoInputInternal::BufferEvent` 0x614DE0; offset +0x8C identical.
- Disassembly:
  ```
  72e09c  mov [ecx+0x3e4],eax         ; mouse X
  72e0b3  mov edx,[ebp-0x1c]          ; edx = this (saved ECX)
  72e0b6  mov [edx+0x3e8],ecx         ; mouse Y
  72e0bc  mov eax,1                   ; HOOK (5 bytes, one instruction)
  72e0c1  jmp 0x72e6f9
  ```
- Register check: the hook sources `edx` as a pointer and the C++ reads `[edx+0x3E4]` (x) and `[edx+0x3E8]` (y); at the hook EDX = this = the CExoInputInternal, and +0x3E4/+0x3E8 are the two fields stored one instruction earlier. The stolen `mov eax,1` has no relative operand; the following `jmp` is not stolen. No jump lands in the stolen bytes. Differences: none.

### Other hook-field checks
- `skip_original_bytes = false`, `exclude_from_restore = []` for all six, same as GOG. The draft toml's six hook tables are field-for-field identical to the GOG toml except `address` and the `target_versions` hash (checked by script; every original_bytes also re-read from the exe).
- Stolen-byte lengths are all >= 5 and end exactly on instruction boundaries (hooks 1, 6: one instruction; 2, 3, 4: two or three; 5: four). Verified against the Ghidra `.asm` listing and with objdump.
- No two hooks of this patch overlap each other.
- **Local conflict (not an upstream issue):** ForcePowerHotbar (`force_power_hotbar_src/kotor2-steam-aspyr.hooks.toml`) already detours 0x0072CB63 and 0x0074B3F0. EKC and ForcePowerHotbar cannot both be installed on this machine unless one of them is changed (`scripts/check_hook_spans.py` would FAIL). LoadHangProbe mentions 0x74B3F0 only in a comment.

## 2. Constants

Everything below is in the K2 path of `ExpandedKeyboardControl.cpp`. "Evidence" for functions is the fingerprint match (size, ret, insns, disp, imm identical) plus callee topology; for vtables it is the Steam RTTI (COL -> TypeDescriptor decoded from the exe, all `offset = 0` primary vtables) and the constructor writing the vtable.

### 2a. Vtables (K2_CONFIG panel fields and the two stand-alone K2_* constants)

| name | GOG | meaning on GOG | Steam | evidence on Steam |
|---|---|---|---|---|
| K2_INVENTORY_PANEL_VTABLE | 0x00992994 | `vtable_for_CSWGuiInGameInventory` (server symbol) | **0x009A72AC** | RTTI TD 0xA115C8 `.?AVCSWGuiInGameInventory@@`; ctor FUN_008A6170 writes it at 0x8A61BF (and 0x8A709D) |
| K2_JOURNAL_PANEL_VTABLE | 0x00992274 | `vtable_for_CSWGuiInGameJournal` | **0x009A24DC** | TD 0xA10B6C `.?AVCSWGuiInGameJournal@@`; written at 0x7FAEAC and 0x7FBAFD (slot 0 = 0x7FBAA0) |
| inGameMessagePanelVtable | 0x0098E1EC | `vtable_for_CSWGuiInGameMessages` | **0x0099E5EC** | TD 0xA104B8 `.?AVCSWGuiInGameMessages@@` |
| inGameFadePanelVtable | 0x0099313C | `vtable_for_CSWGuiFade` | **0x009A044C** | TD 0xA10744 `.?AVCSWGuiFade@@` |
| inGamePausePanelVtable | 0x00993334 | `vtable_for_CSWGuiInGamePause` | **0x009A7BAC** | TD 0xA116E4 `.?AVCSWGuiInGamePause@@` |

Not added (stays GOG-parity): `CSWGuiInGameAutoPause` Steam 0x009A8FCC (GOG 0x0099580C) is **not** in EKC's GOG gate list; ForcePowerHotbar whitelists it. Left out so GOG and Steam behave identically (see risks).

### 2b. Addresses (GameConfig fields)

| field | GOG | GOG meaning | Steam | evidence |
|---|---|---|---|---|
| keyboardDeviceIndex | 0x00997514 | `.rdata` int 0 = keyboard slot in the per-device array; GOG GetEvents string refs 0x9974A0/0x9974B8, +0x5C | **0x0099C884** | holds 0; Steam GetEvents strings 0x99C810/0x99C828, the same +0x5C; used at 0x72C96A as `[0x99C884]*0xC` device-array index; neighbours 0x99C888=1, 0x99C88C=2 |
| getIsSelectable (thiscall, ret 0) | 0x00917340 | `CSWGuiControl::GetIsSelectable` sz 135 | **0x004190A0** | only Steam fn with sz 135/ret 0/55 insns; same disp/imm; vtable-slot calls +0x2C, +0x4C, flags bit 3 |
| setActiveControl (thiscall, ret 8) | 0x0090D080 | `CSWGuiPanel::SetActiveControl` sz 143 | **0x0040EC00** | unique match; body: `[this+0x20]` active control, `[this+0x1C]` manager, calls FUN_004122A0 (GUI sound) |
| getControlAt (thiscall, ret 8) | 0x0051CB20 | `CSWGuiMainInterface::HitCheckMouse` sz 68 | **0x0074B3A0** | match; callees 0x7433B0 (= GOG `CSWGuiTargetActionMenu::HitCheckMouse` 0x5149B0, sz 388) and 0x40EAE0 (= GOG `CSWGuiPanel::HitCheckMouse` 0x90CF60, sz 287), both fingerprint-identical |
| personalPrevious (ret 4) | 0x005233B0 | sz 170, calls `SelectPrevPersonalAction` 0x523D50 (sz 387) | **0x00751C30** | callee 0x7525D0 = the unique fingerprint twin of 0x523D50 |
| personalNext (ret 4) | 0x00523460 | sz 170, calls `SelectNextPersonalAction` 0x523BD0 (sz 371) | **0x00751CE0** | callee 0x752450 = the unique twin of 0x523BD0 |
| targetPrevious (ret 4) | 0x00523510 | sz 167, calls `CSWGuiTargetActionMenu::SelectPrevAction` 0x515A10 (sz 436) | **0x00751D90** | callee 0x744410 = the unique twin of 0x515A10 (sz 436) |
| targetNext (ret 4) | 0x005235C0 | sz 167, calls `CSWGuiTargetActionMenu::SelectNextAction` 0x515860 (sz 420) | **0x00751E40** | callee 0x744260 = the unique twin of 0x515860 (sz 420) |
| activate (ret 4) | 0x00522E30 | sz 84, calls 0x522E90/0x522ED0 | **0x007516B0** | callees 0x751710 / 0x751750 (+0x22E880); equals the "activate button" address the hotbar doc recorded |
| cancelLastAction (ret 0) | 0x00523AE0 | sz 75, calls ClearOneAction 0x522930 | **0x00752360** | callee 0x7511B0 = ClearOneAction (U4 map); `__fastcall` = ECX |
| handleGuiInputEvent (thiscall, ret 8) | 0x0090EF20 | `CSWGuiManager::HandleInputEvent` sz 1306 | **0x00410AA0** | unique match; iterates panel list `+0x88/+0x8C` or top modal `+0x94/+0x98`, calls panel vtable slot `+0x3C` with (event, flag); callers = FUN_007AC470 (ProcessInput) and 0x7B12C0 |

Order of the prev/next pairs: each pair shares one shape, so the order is fixed by which callee each calls, not by position. GOG 0x5233B0/0x523460/0x523510/0x5235C0 call Prev/Next Personal and Prev/Next target actions respectively; the Steam functions at +0x22E880 call the fingerprint twins of those same callees in the same order (table above). The positional delta agrees.

### 2c. Struct offsets and literals (all unchanged)

Verified by identical `disp`/`imm` sets in the fingerprints of the functions that use them (Draw, SetActiveControl, HandleInputEvent, BufferEvent, GetEvents, the HandleInputEvent of every filter panel) and by reading the Steam `.asm`.

| literal | value (both builds) | Steam evidence |
|---|---|---|
| panelManagerOffset / panelActiveControlOffset | 0x1C / 0x20 | SetActiveControl 0x40EC00 reads `[this+0x20]`, `[this+0x1C]` |
| panelFlagsOffset | 0x48 | GetIsSelectable `param_1[0x12]` (= byte 0x48) bit 3; Draw flag tests |
| targetActionMenuOffset 0xCC, TARGET_ACTIONS_OFFSET 0x54 | | Draw `0x74C1C4 add ecx,0xcc`, `0x74C1E9 add ecx,0x54` |
| personalActionsOffset 0x733C, actionGroupSize 0x750 | | Draw `0x74C14E imul edx,edx,0x750`, `0x74C15A lea ecx,[eax+edx+0x733c]` (loop to 7, EKC uses 6 like GOG) |
| actionControlOffsets {0,0x1D0,0x3A0,0x570} | | identical Draw/UpdateActionMenu `disp`/`imm` sets (group stride 0x750 inner layout is a GOG/Steam-identical class) |
| MANAGER_PANEL_LIST/COUNT 0x88/0x8C, MODAL_LIST/COUNT 0x94/0x98 | | FUN_00410AA0 body: `param_1+0x88/0x8c` loop, `param_1+0x94/0x98` modal top |
| panel vtable slot 0x3C (DispatchPanelInput) | | FUN_00410AA0 calls `(*(*panel + 0x3c))(event,flag)`; Steam slot 15 of Inventory/Journal/Messages vtables = 0x8A8880/0x7FBD90/0x759E20 |
| mouse X/Y in input object 0x3E4 / 0x3E8, mainInterface+0x10 | | BufferEvent stores at `[this+0x3e4]`/`[this+0x3e8]`; Draw passes panel `+4,+8,+0xC,+0x10` = x,y,w,h so +0x10 = height |
| movie wndproc frame +0x0C / +0x10 | | see hook 2 |
| WM_KEYDOWN 0x100, VK_DELETE 0x2E, VK_ESCAPE 0x1B, DIK_* | OS/DirectInput constants | `case 0x100: if (param_3 == 0x1b)` in FUN_0040C4A0 |

### 2d. GUI event ids (data, not addresses): K2_GUI_CONTROLLER_X_EVENT 0x29, _LEFT 0x2F, _RIGHT 0x30

**Unchanged.** The Steam panel `HandleInputEvent` handlers (vtable slot 15) are byte-for-byte fingerprint twins of GOG's: Inventory 0x8A8880 = GOG 0x59C6D0 (sz 464), Journal 0x7FBD90 = GOG 0x5910C0 (sz 1041), Messages 0x759E20 = GOG 0x52B520 (sz 770), Pause 0x8B9BB0 = GOG 0x5AE300 (sz 297), all with identical `imm` lists. Steam code: Inventory `case 0x2f`/`case 0x30` (previous/next filter via 0xFC1 cycle), Journal and Messages `case 0x29`/`0x2f`/`0x30`. The manager dispatcher FUN_00410AA0 also has `case 0x2f`/`0x30`.

### 2e. K1 constants
Out of scope (K1 path untouched; K1 toml, K1_* constants, and the `kotor1_gog_103` target are not modified).

## 3. One module for both builds: recommendation

Facts about the framework (from upstream/master):
- `create-patch.py` always links `Common/GameAPI` + sqlite into the `windows_x86` module (`links_game_api=True`), so EKC *could* call `GameVersion` without build changes. EKC currently includes no GameAPI header, opens no `addresses.db`, and its `exports.def` is committed (and is left alone by create-patch).
- `GameVersion::GetVersionSha()` returns a string that `GameVersion::Initialize()` fills from the `KOTOR_VERSION_SHA` environment variable, after it also opens `addresses.db` and finds a matching `game_version` row. If the DB is missing, `Initialize()` returns false. (Precedent: PartyRosterLimit32K1 compares `GetVersionSha()` to a constant.)
- `KotorPatcher` (`src/KotorPatcher/src/core/patcher.cpp:122`) calls `SetEnv("KOTOR_VERSION_SHA", ...)` from `patch_config.toml`'s `target_version_sha` before it applies any patch or loads any patch module, and the KotorPatcher README documents this as the way for "patch binaries to query the game version and adjust behavior accordingly".
- K2AspyrVisibleConsole (#257) cannot use that because it picks between two *data* tables; it probes memory content. EKC has no equivalent safe content probe (the keyboard-device constant is 0 at both addresses), so use the version string.

**Recommendation: read `KOTOR_VERSION_SHA` directly and select a `const GameConfig` at first use; leave exports and the GOG toml untouched.** It is the smaller diff and keeps GOG independent of `addresses.db`:
- one new `K2_STEAM_CONFIG` table next to the GOG one (27 initializers: the 25 of the GOG table with Steam values, plus the two vtables);
- two vtables that are not in the existing struct move into it (`inventoryPanelVtable`, `journalPanelVtable`, appended at the end; K1_CONFIG gets `0, 0`);
- `K2_CONFIG` -> `K2Config()` (20 mechanical replacements). `MapMovieDeleteToEscapeK2` and `ClearActionBarControlsK2` need no change in behaviour (no addresses), and `exports.def` is unchanged.
- Fallback rule: anything that is not exactly the Steam sha uses the GOG table (which is what the module has always done), so GOG behaviour is byte-identical (the 25 original initializers were machine-compared against `K2_GOG_CONFIG`, and the two vtables 0x992994/0x992274 that used to be stand-alone constants are appended unchanged).

Alternative if a reviewer insists on the GameAPI path: replace `IsSteamBuild()` by
```cpp
#include "GameAPI/GameVersion.h"
bool IsSteamBuild() { return GameVersion::Initialize() && GameVersion::GetVersionSha() == K2_STEAM_SHA; }
```
but then a GOG user whose `addresses.db` is missing/unreadable would silently fall to GOG anyway, and a Steam user in the same state would get GOG addresses (crash). Reading the env var has no such dependency; mention this when proposing the diff.

Rejected: separate `...K2Steam` exports (would need the K2 helpers to take a config argument, 6 new exports and an `exports.def` edit: larger diff).

## 4. C++ change outline (exact diff in `ExpandedKeyboardControl.cpp.diff`)

Compiles clean with `i686-w64-mingw32-g++ -std=c++17 -Wall -Wextra -fno-exceptions -fno-rtti` (original and patched both warning-free). Not run (no Wine/game); behaviour on GOG is by construction identical.

1. Includes: add `#define WIN32_LEAN_AND_MEAN`, `<windows.h>`, `<cstring>` (for `_stricmp`).
2. Delete the two stand-alone constants `K2_INVENTORY_PANEL_VTABLE` / `K2_JOURNAL_PANEL_VTABLE`.
3. `GameConfig`: append `std::uintptr_t inventoryPanelVtable; std::uintptr_t journalPanelVtable;`. `K1_CONFIG`: append `0, 0,`.
4. Rename `K2_CONFIG` -> `K2_GOG_CONFIG` and append `0x00992994, 0x00992274,`.
5. Add the Steam table and selector:
```cpp
// Steam Aspyr (6A522E71...): same struct layouts and event ids as GOG, different addresses.
constexpr GameConfig K2_STEAM_CONFIG = {
    0x1C, 0x20, 0x48, 0xCC, 0x733C, 0x750,
    {0x000, 0x1D0, 0x3A0, 0x570},
    6,
    0x0099E5EC,   // CSWGuiInGameMessages
    0x009A044C,   // CSWGuiFade
    0x009A7BAC,   // CSWGuiInGamePause
    0x0099C884,   // keyboard device index
    0x004190A0,   // CSWGuiControl::GetIsSelectable
    0x0040EC00,   // CSWGuiPanel::SetActiveControl
    0x0074B3A0,   // CSWGuiMainInterface::HitCheckMouse
    0x00751C30,   // personalPrevious
    0x00751CE0,   // personalNext
    0x00751D90,   // targetPrevious
    0x00751E40,   // targetNext
    0x007516B0,   // activate
    0x00752360,   // cancelLastAction
    0x00410AA0,   // CSWGuiManager::HandleInputEvent
    0x009A72AC,   // CSWGuiInGameInventory
    0x009A24DC,   // CSWGuiInGameJournal
};

constexpr char K2_STEAM_SHA[] =
    "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF";

bool IsSteamBuild()
{
    char sha[128] = {};
    const DWORD length = GetEnvironmentVariableA("KOTOR_VERSION_SHA", sha, sizeof(sha));
    return length == sizeof(K2_STEAM_SHA) - 1 && _stricmp(sha, K2_STEAM_SHA) == 0;
}

// Anything that is not the Steam build keeps the GOG addresses it always used.
const GameConfig& K2Config()
{
    static const GameConfig* config = nullptr;   // constant-initialised: no thread-safe-static guard needed
    if (!config) {
        config = IsSteamBuild() ? &K2_STEAM_CONFIG : &K2_GOG_CONFIG;
    }
    return *config;
}
```
6. `IsK2FilterPanel`: use `K2Config().inventoryPanelVtable / journalPanelVtable / inGameMessagePanelVtable`.
7. `sed 's/K2_CONFIG/K2Config()/'` on the remaining 18 uses.

The struct-field order must match the initializer order (checked: the Steam table is the GOG table with different values; K1/K2 GOG value runs were compared programmatically to the original).

## 5. Manifest change

`Patches/ExpandedKeyboardControl/manifest.toml`: add one line to `[patch.supported_versions]` (same key format as K2AspyrMapAspectFix/K2AspyrVisibleConsole) and bump the minor version (precedent: #257, #292 1.0.0->1.1.0, MapAspectFix 1.2.0):
```toml
version = "1.19.0"            # was 1.18.0
...
[patch.supported_versions]
kotor1_gog_103 = "9C10E0450A6EECA417E036E3CDE7474FED1F0A92AAB018446D156944DEA91435"
kotor2_gog_aspyr = "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914"
kotor2_steam_aspyr = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF"
```
New file: `kotor2-steam-aspyr.hooks.toml` (the draft, renamed). `exports.def` unchanged. No `additional/` files, no DB rows needed (EKC resolves nothing via `addresses.db`).

## 6. Risks and unknowns

1. **Not run.** Nothing here was executed in game or under Wine; all claims are static. The two shared hook sites (hook 1 and 4) are live in ForcePowerHotbar; hooks 2, 3, 5, 6 and the new config are untested.
2. **Prev/next orientation** is fixed by callee fingerprints (section 2b), and positionally agrees. Still worth one live key test (Up = previous action, Down = next) because a swap would only show up as reversed cycling.
3. **GOG-side semantics are inferred from server labels and fingerprints, not a GOG disassembly.** The GOG names for the small functions are server auto/hand labels (e.g. 0x5233B0 is labelled `CSWGuiActionMenuIos::HandleInputEvent` there but `CSWGuiMainInterface` in the mapper); the Steam identity does not rely on the labels, only on identical fingerprints. If a GOG listing is available, spot-check that hook 6's EDX is `this` and hook 2's `[ebp+0xC]` is uMsg there.
4. **Keyboard device index 0x0099C884** was marked "unverified live" in the hotbar notes; the static evidence above (device-array index constant inside GetEvents, +0x5C from the assertion strings, value 0) is strong but not a runtime test.
5. **Version selection depends on `KOTOR_VERSION_SHA` being set** (always the case through KotorPatcher; a failed `SetEnv` only logs a warning). If it were missing on Steam, the module would use GOG addresses. A stricter variant: treat "unknown" as disabled, but that changes GOG failure behaviour; I kept GOG-identical semantics. Upstream may prefer the `GameVersion` API; see section 3.
6. **EKC <-> ForcePowerHotbar overlap** on 0x72CB63 and 0x74B3F0 (local only; second installer's `original_bytes` check will refuse). Needs a decision before local live testing (e.g. hotbar reads input elsewhere, or EKC and hotbar are alternatives).
7. **Possible existing upstream gap (not verified in play):** EKC's `IsGameplayHudActive` whitelists Messages, Fade and Pause but not `CSWGuiInGameAutoPause` (GOG 0x0099580C, Steam 0x009A8FCC); the hotbar added it. Parity kept; could be a separate upstream issue if the auto-pause banner blocks the arrow keys. Unverified.
8. `ExpandedKeyboardControl` was previously evaluated and skipped as a Steam port (doc 06 section 4, 2026-09-29) on user preference; the user has since chosen it as the next PR candidate (SESSION_CONTEXT 2026-10-03).
9. Memory note: KPM refuses a detour if `original_bytes` do not match at apply time, so a wrong hook address fails safe (patch refuses), whereas a wrong *constant* in the C++ would crash at first use. That is why the constants table is the part to re-verify.

## 7. What the lead should re-verify (highest value first)

1. Hook 6: confirm `[edx+0x3e4]`/`[edx+0x3e8]` are x/y at the hook (read `decomp/72/0072e030.asm` lines around 0x72E09C-0x72E0BC).
2. Callee identity behind the four prev/next entries: GOG 0x515A10/0x515860 (sz 436/420) vs Steam 0x744410/0x744260 (sz 436/420, ret 4, 130/125 insns, identical disp/imm), and 0x523D50/0x523BD0 vs 0x7525D0/0x752450.
3. The ProcessInput identification of FUN_007AC470 (hook 3): fingerprint is exact (15798 B, 3509 insns, identical disp/imm lists); check the single `0x410AA0` call pair in the caller list.
4. `activate` 0x7516B0 vs `HudInvokeGroup` 0x751750 relation (activate reads `[button+0x50]`, `[button+0x5C]` then dispatches to 0x751750 or 0x751710).
5. Compile the diff in the real KPM tree (`create-patch.py`), run the K1+K2 module build, and check `exports.def` still lists the same 11 functions.
