# EKC x Action Hotbar coexistence: design (task R-A, 2026-10-04)

Read-only research. No code changed. Target: KOTOR II Steam Aspyr, sha256 `6A522E71...BFFEF`, image base 0x400000.
Sources: `K` = `~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-ekc` (branch `port/k2-expandedkeyboard-steam`, HEAD c90fd4f; upstream-based, has no `KPatchApi`).
`H` = `patch_manager_mods/action_hotbar_src/`. Exe facts come from the vanilla Steam exe (`scripts/disasm_va.py steam`), exact byte search over the whole `.text` (0x401000, 5,785,256 bytes).

## 0. Answer first

Recommendation: **option (c)**. Move the hotbar's two hooks off the shared addresses, by 33 bytes and by 5 bytes:

| Hotbar hook | Now | Move to | Stolen bytes |
|---|---|---|---|
| CaptureHotbarKey | 0x0072CB63 | **0x0072CB84** | `8B 55 10 C1 E2 06` (6) |
| HotbarHudTick | 0x0074B3F0 | **0x0074B3F5** | `68 E8 30 96 00` (5) |

Both are straight-line successors of the EKC sites, run once per event / per frame, and carry the same register inputs. No KPM change, no upstream change, about 0.5 day plus one 15-minute live check. Section 4 has the evidence, section 5 the other options.

## 1. What KPM does when two patches hook one address

Install time (KPatchCore, C#): refused, game untouched.
- `PatchApplicator.cs:269-280` loads each selected patch's hooks for the detected exe hash, then calls `HookValidator.ValidateMultiPatchHooks`.
- `HookValidator.cs:140-176` groups every hook of every patch by exact `Address`. A group of 2+ fails with:
  `Hook conflicts detected:\n  - Address 0x0072CB63 used by multiple patches: action-hotbar:CaptureHotbarKey, expanded-keyboard-control:CaptureActionBarInputK2`
  (one line per shared address, so both 0x0072CB63 and 0x0074B3F0 would be listed).
- This runs before the backup (`HookTargetValidator.cs` remarks) and before any file is written.
- The manifest `conflicts = []` list is a separate, manual mechanism (`DependencyValidator.cs:87-118`). Neither manifest sets it, so the hook check is what stops the install.
- Gap: the check compares **start addresses only**. Two hooks whose byte spans overlap but start at different addresses (for example 0x74B3F0 and 0x74B3F3) pass KPatchCore. Our `scripts/check_hook_spans.py` covers that (FAIL on any overlapping byte, WARN when spans are 8 bytes or less apart).

Runtime (KotorPatcher, if a conflict slipped in, for example a hand-edited `patch_config.toml`):
- The first detour writes `E9 rel32` plus NOPs over the site (`patcher.cpp:314-321`).
- The second detour calls `VerifyBytes(hookAddress, originalBytes)` (`patcher.cpp:272-277`). It now sees the JMP, so it logs `Original bytes mismatch at hookAddress 0x... - wrong game version?` and returns false.
- `ApplyPatches` returns false on the first failure (`patcher.cpp:181-188`). **Every hook later in the config is never applied.** Hooks already applied stay applied (no rollback).
- `InitializePatcher` returns false and `DllMain` logs `ERROR: Failed to initialize` (`dllmain.cpp:16-19`). The game still starts, partly patched.
- Logging goes to `OutputDebugStringA` only (`platform_win32.cpp:14-16`), so a normal player sees nothing. The failure is silent, and it is order dependent (config order decides who loses).

## 2. How a detour works (does the stolen code still run?)

Yes. Both patches use `type = "detour"`, `skip_original_bytes = false`.

Hook types (`patcher.cpp`):
- **simple** (335-358): verify bytes, overwrite with `replacement_bytes` of equal length. No DLL, no wrapper, no state saved.
- **replace** (361-429): copy `replacement_bytes` into a new code block, append `JMP back`, write `JMP block` plus NOPs at the site. No DLL call, no state saved.
- **detour** (223-332): load the patch DLL, resolve `function`, generate a wrapper stub near the site, write `JMP wrapper` + NOPs over the stolen bytes.

Wrapper layout, x86 (`wrapper_x86.cpp`):
1. `LEA ESP,[ESP-528]` reserve the FXSAVE area; `PUSHAD`; `PUSHFD`; `MOV EBX,ESP` (anchor); `FXSAVE` (126-173). Always saved, even when not restored.
2. `AND ESP,-16`, pad, then push each `[[hooks.parameters]]` right to left, read from the saved copy via EBX (190-215). cdecl.
3. `CALL patchFunction` (222-226). The return value is ignored unless `consumed_exit_address` is set (neither patch sets it).
4. `MOV ESP,EBX`; `FXRSTOR` if `preserve_registers`; `POPFD` if `preserve_flags`; `POPAD`, skipping names in `exclude_from_restore`; EBX always restored (229-304). Defaults are true, true, empty. `LEA ESP,[ESP+528]` closes the frame (306-312).
5. The stolen original bytes are copied **verbatim** into the stub and run here (321-327). So the game sees the registers and flags it would have seen natively. Consequence: the stolen bytes must not hold a relative jump or call (no relocation).
6. Optional consumed-exit test (346-364), then `JMP hookAddress+len(originalBytes)` (371-377).

So yes: two detours at **adjacent, non-overlapping** sites both work. Wrapper A re-runs its own stolen bytes and jumps to the first byte after them. If that byte is the start of hook B, it lands on B's JMP, and B's wrapper does the same. Each hook verifies only its own bytes, and KPatchCore keys on exact start address, so adjacency is accepted by both layers.
- Cost per hook: about 570 bytes of stack and one FXSAVE/FXRSTOR pair per call.
- Limit: stolen length must be at least 5 (`Hook.cs` docs), and A's span must end at or before B's start.

## 3. What each patch does at the two shared sites

Anchors (vanilla Steam exe):
```
72cb52  8b 84 95 cc fe ff ff   mov eax,[ebp+edx*4-0x134]   ; record index of device edx=[ebp-0x10]
72cb59  6b c0 14               imul eax,eax,0x14           ; DIRECTINPUT record size 0x14
72cb5c  03 84 0d 04 ff ff ff   add eax,[ebp+ecx-0xfc]      ; + record array base
72cb63  89 85 bc fe ff ff      mov [ebp-0x144],eax         ; <- both hooks (6 bytes, one instruction)
72cb69  83 7d f0 02            cmp dword [ebp-0x10],2
74b3f0  55 8b ec 6a ff         push ebp; mov ebp,esp; push -1   ; <- both hooks (5 bytes, function entry)
74b3f5  68 e8 30 96 00         push 0x9630e8               ; SEH handler (not hooked today)
```
Function identities: 0x72C7E0 = `CExoInputInternal::GetEvents` (GOG 0x6134E0; strings at 0x99C810/0x99C828, port_spec section 1). 0x74B3F0 = `CSWGuiMainInterface::Draw` (vtable 0x0099DCC4 RTTI, fingerprint identical to GOG 0x51CB70).

| | Site 1, 0x72CB63 (key capture) | Site 2, 0x74B3F0 (HUD Draw entry) |
|---|---|---|
| Registers read | EAX = `&record{dwOfs@+0, dwData@+4}`; EBP (EKC reads `[ebp-0x10]` itself; the hotbar asks the wrapper for `[ebp-0x10]` as an int) | ECX = `this` (the HUD) |
| Frame slots | `[ebp-0x10]` = device loop index (also used at 72cb33/72cb49). `[ebp-0x144]` is written by the stolen store | none |
| EKC function | `CaptureActionBarInputK2` (EKC cpp:1023-1068) | `UpdateActionBarControlsK2` (EKC cpp:1090-1093, body 711-749) |
| EKC effect on the event | Pass through. Only write: if keyboard device and DIK_DELETE and the HUD is not the active panel, `input->offset = DIK_ESCAPE` (1031-1038). Otherwise it only sets bits in its own `g_pendingInput` for arrows, Return, Space, Delete. | Stores `g_mainInterface`; drains the pending bits; moves focus / cycles / activates action-bar buttons by calling game thiscalls (`SetActiveControl` 0x0040EC00, activate 0x007516B0). No return value used. |
| Hotbar function | `CaptureHotbarKey` (`H/ActionHotbar.cpp:1673-1707`) | `HotbarHudTick` (1789-1860) |
| Hotbar effect on the event | **Consumes** bound digit keys: `rec->dwOfs = 0; rec->dwData = 0` (1704). Tracks Ctrl/Alt/Shift and Esc (editor); queues slot requests into its own ring. Not-bound keys pass through. | Gate check, config reload, button state, `drainRing` (invokes `FUN_00751750(hud,g,0)` with a temporary selection key at `hud+0x1C7C+g*4`), vanilla action-bar resync (0x74DA40). |
| Control flow | none changed | none changed |

Both patches return `void`, set no `consumed_exit_address`, never skip the stolen bytes, and restore all registers. Neither skips original game code. The only "consume" mechanism is **editing the 0x14-byte record in place**, which the game reads later through `[ebp-0x144]` (for example 72cc1f/72cc2c compares `record->dwOfs`).

Interaction analysis (both installed):
- Key overlap: none. EKC uses arrows, Return, Space, Delete. The hotbar uses digits 1-0 (DIK 0x02-0x0B) and Esc (editor only). EKC's Delete-to-Esc rewrite happens only when the HUD is not active; the editor lives on the HUD, so the hotbar never sees a fake Esc while editing.
- Order matters only for the record: whichever runs first edits it, the second sees the edit. EKC edits only DELETE; the hotbar zeroes only digits. No conflict either way.
- No shared globals: separate DLLs, anonymous-namespace state in EKC, own state in the hotbar.
- Tick order: both run on the render thread, one after the other, before Draw's body. No race. Open item to test live: EKC focus on an action button vs the hotbar's selection-key swap and `resyncActionBar` in the same frame (see section 6).

## 4. Option (c), site evidence

### 4a. Capture: 0x0072CB84 (replaces 0x72CB63)

Bytes at 0x72CB84 (disassembled from a clean instruction boundary, preceding bytes shown for context):
```
72cb7a  c7 85 b8 fd ff ff 02 00 00 00   mov dword [ebp-0x248],2      (-16..-7)
72cb84  8b 55 10                          mov edx,[ebp+0x10]    \ stolen, 6 bytes
72cb87  c1 e2 06                          shl edx,6             /
72cb8a  8b 85 bc fd ff ff                 mov eax,[ebp-0x244]
72cb90  8d 4c 10 08                       lea ecx,[eax+edx+8]
72cb94  8b 95 b8 fd ff ff                 mov edx,[ebp-0x248]
```
Why it is equivalent:
- Dominance: 72cb69 `cmp [ebp-0x10],2 / jge 72cb7a` splits into `72cb6f..72cb78 (mov ecx,[ebp-0x10]; mov [ebp-0x248],ecx; jmp 72cb84)` and `72cb7a..72cb83 (mov [ebp-0x248],2)`. Both arms fall into 0x72CB84. So 0x72CB84 runs exactly once per run of 0x72CB63, with nothing else in between.
- Registers: neither arm writes EAX (they use ECX and memory only), so **EAX is still the record pointer** at 0x72CB84, and `[ebp-0x10]` is unchanged. The hotbar's two `[[hooks.parameters]]` (`eax` pointer, `[ebp-0x10]` int) stay as they are. As a belt-and-braces alternative, `[ebp-0x144]` also holds the pointer.
- The record is still edited before any reader: the first reader of `[ebp-0x144]` is 0x72CC1F, 155 bytes later.
- Order vs EKC: EKC runs first (0x72CB63), the hotbar second (0x72CB84). Fine, see section 3.
- Span check: EKC span [0x72CB63,0x72CB69), hotbar span [0x72CB84,0x72CB8A). Gap 27 bytes, no WARN from `check_hook_spans.py`.
- No branch lands inside the stolen span: a whole-`.text` scan of `E8/E9 rel32`, `0F 8x rel32`, `EB/7x rel8` found 0 targets strictly inside 0x72CB85..0x72CB89. 0x72CB84 itself is a join point (the two arms), which is fine for a detour.
- Stolen bytes have no relative operand (`mov edx,[ebp+0x10]` is frame-relative; `shl` is immediate).
- Uniqueness (exact search over `.text`): the 6 stolen bytes occur twice (0x72C936 is a sibling loop with the same code). The 26-byte context `C7 85 B8 FD FF FF 02 00 00 00 | 8B 55 10 C1 E2 06 | 8B 85 BC FD FF FF 8D 4C 10 08` occurs **once** (match starts 0x72CB7A). The Steam hook file does not need that context (KPM checks only the address), but the toml comment should record it.

### 4b. HUD tick: 0x0074B3F5 (replaces 0x74B3F0)

```
74b3f0  55                    push ebp            \ EKC's detour (5 bytes), unchanged
74b3f1  8b ec                 mov ebp,esp          |
74b3f3  6a ff                 push -1             /
74b3f5  68 e8 30 96 00        push 0x9630e8        ; <- hotbar, stolen 5 bytes (SEH handler address, an absolute immediate)
74b3fa  64 a1 00 00 00 00     mov eax,fs:0
74b400  50                    push eax
74b401  81 ec 00 02 00 00     sub esp,0x200
```
Why it is equivalent:
- ECX is untouched by `push ebp / mov ebp,esp / push -1` (the only instructions before it), so ECX = `this` = the HUD at 0x74B3F5, same as at 0x74B3F0. The next ECX use is 0x74B418 `mov [ebp-0x16c],ecx`.
- Same call count: the new site is the 4th instruction of the function entry block. Nothing can reach it without executing 0x74B3F0 first.
- Stolen `push imm32` has no relative operand.
- No branch lands in 0x74B3F6..0x74B3F9, nor in 0x74B3F1..0x74B3F4 (scan result: 0 hits for both).
- Uniqueness: `68 E8 30 96 00` alone hits once in `.text` (0x74B3F5). The 18-byte context `68 E8 30 96 00 64 A1 00 00 00 00 50 81 EC 00 02 00 00` also hits once.
- Chain with EKC: EKC's wrapper re-runs `55 8B EC 6A FF` and jumps to 0x74B3F5. That is the hotbar's `E9 rel32`, so it falls straight into the hotbar wrapper. Hotbar span [0x74B3F5,0x74B3FA) starts exactly where EKC's ends: no overlap, `check_hook_spans.py` prints a WARN (gap 0, within 8 bytes), not a FAIL.
- The hotbar's handler reads only ECX. At the wrapper call, the stack holds the ebp frame and one `-1` push; the wrapper saves and restores everything, so this is invisible to the game.
- No other local hook is near either site: `rg` over `patch_manager_mods/**/*.toml` finds only `action_hotbar_src` at these addresses; `load_hang_probe_src` mentions 0x74B3F0 in a comment only.

Hotbar-only installs keep working: the same behavior, just entered 5 and 33 bytes later.

### 4c. Work list for (c)

1. In `H/kotor2-steam-aspyr.hooks.toml` change the two addresses and `original_bytes`; keep functions, parameters, flags. Update the header comments (record the context bytes above).
2. Update `H/ActionHotbar.cpp` header comment (lines 6-7), doc 07 hook tables, and the `manifest.toml` comment.
3. Run `scripts/check_hook_spans.py`, `tools/validate-patches.py`, and the hotbar test gate (`H/test.toml`).
4. Add a hotbar-side byte check against the vanilla exe for the two new sites (the shared hook verifier from I-4 if it is ready).
5. Live check with EKC installed (see section 6). Effort: about 0.5 day coding and tests, 15 minutes live.

## 5. All options compared

| Option | What changes | Pros | Cons | Effort |
|---|---|---|---|---|
| (a) Hotbar calls EKC logic as a library | Copy or link EKC's 1471-line `ExpandedKeyboardControl.cpp` into the hotbar | One hook set. No framework change | Does not give coexistence: the hotbar would need EKC's other 4 hooks (0x40C4A6, 0x7AC470, 0x74A750, 0x72E0BC) to be useful, and then still collides with the real EKC patch on all six. Forks upstream code (author "J", manifest 1.19.0), drift on every EKC release, license and attribution questions, doubles test surface | 2-3 days, high risk |
| (b) EKC exports a handler the hotbar calls | New exports in EKC; EKC's two handlers call the hotbar through `GetModuleHandle`/`GetProcAddress` | Works inside the current KPM | Someone must drop the shared hooks, and that patch can then no longer run alone. KPM picks hooks by exe hash only, so a patch cannot install its hooks "only if the other is absent". Forces a second variant patch id (`action-hotbar-ekc`, `requires = ["expanded-keyboard-control"]`). Needs an upstream EKC change and a stable ABI between two authors | 2-3 days plus upstream review |
| **(c) Move the hotbar's hooks to equivalent sites** | 2 addresses and 2 byte arrays in our own toml | No framework change, no upstream change, no ABI. Each patch stays standalone. Works with any install order. Evidence in section 4 | Uses the "adjacent hook" property (section 2): a future third patch at 0x74B3F5 or 0x72CB84 would clash again. A WARN stays in `check_hook_spans.py` for the 0x74B3F5 pair | 0.5 day + 15 min live |
| (d) KPM chained detours | KotorPatcher and KPatchCore learn to share one site | General fix. Any future pair of patches could share a site | See below. Touches upstream framework code in C++ (x86 and x64 wrappers) and C#, and needs an upstream PR | 3-5 days plus upstream review |

Option (d), what would change:
- `HookValidator.ValidateMultiPatchHooks` (`HookValidator.cs:140-176`): allow a shared address only if all hooks there are `detour`, with identical `original_bytes`, equal `preserve_*` / `exclude_from_restore`, no `skip_original_bytes`, no `consumed_exit_address`. Anything else keeps today's error, with a new message naming the incompatible field.
- `ConfigGenerator.cs`: emit the shared hooks in a defined order (by patch id, or by an optional `priority` field).
- `config_reader.cpp` / `PatchInfo`: nothing structural; hooks stay separate entries.
- `patcher.cpp` `ApplyPatches` (181-188): group detour entries by `hookAddress` before applying. Verify the bytes once, load each DLL, resolve each symbol, build **one** `WrapperConfig` with a list of `(patchFunction, parameters)`, write the JMP once.
- `wrapper_base.h`, `wrapper_x86.cpp` (205-226) and `wrapper_x86_64.cpp`: loop "push params, CALL" per handler inside one save/restore frame. Stolen bytes still run once.
- Semantics to decide: handler order; each handler sees the registers as saved (not the previous handler's edits, since only the saved copy is restored); a failing handler.
- Tests: new case in the wrapper tests and in `HookValidator` tests; both platforms.
- Risk: the wrapper is the most delicate code in KPM. A bug breaks every patch.

## 6. Recommendation and live check

Pick **(c)**. It is the only option that needs no change outside our own hooks file, keeps both patches standalone, and is backed by exact-byte, dominance and no-branch-target evidence. If a third patch ever needs one of the four sites, revisit (d) then, with a concrete second user.

Live check after implementing (KPM install rule: commit sources, stage with `stage_and_verify_install.py`, game closed):
1. Install `expanded-keyboard-control` and `action-hotbar` together. Expect no "Hook conflicts" error and 6 + 8 `Applied DETOUR` lines (OutputDebugString / kpatch log).
2. In game: arrows/Return move focus and activate on the vanilla action bar (EKC), keys 1-0 fire hotbar slots and do not switch party members (hotbar), Delete cancels the queued action, and Delete in the movie window still skips (EKC hook 2).
3. Stress the open item: while an action-bar button has EKC focus, change stance via the hotbar (forces `resyncActionBar`), then press Return. Expect no crash and focus restored or cleared.
4. Reverse order and single installs: hotbar alone behaves as before (compare the hotbar log's gate and slot lines to a pre-move run).

Residual risks:
- GOG: the hotbar is Steam-only; nothing to port.
- Steam semantics for EKC hook 1 on `[ebp-0x10]` are already corroborated by the live hotbar (port_spec section 1).
- R-B (GOG semantic check) is independent of this decision.
