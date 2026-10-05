# R6 - where RMB moves the camera, and the cheapest swallow point

Static, Steam exe (sha 6A522E71...). Dump = ghidra_dumps/steam_6a522e71/decomp; copies in `dump/`. Names from Steam DB (B11a) and R9 map.
Builds on doc 18 sections E2/E3 and the 2026-10-04 click chain trace.

## VERDICT: swallow

Hook `0x007B3817` (inside `CClientExoAppInternal::PerformRButtonDownAction 0x7B3810`), detour, `skip_original_bytes = true`.
The handler writes the hold flag `DAT_00A7FCBC = 1` itself unless the cursor is over a selfheal rect. The press is then never seen as a camera drag.

## Q1 - Where RMB reaches the camera (anchors in `dump/`)

1. `0x40A750` WndProc: WM_RBUTTONDOWN 0x204 is built at 0040a750.c:315 and forwarded via `FUN_0072fbd0` (c:320); 0x206 dblclk at c:344. With `options+0x5C` (Reverse Mouse Buttons; ini = 0 here) the message is rewritten to 0x206 (c:340-345).
2. `CExoInputInternal::OnMouseMessage 0x72E030`, `case 0x204` (0072e030.c:66-80): queues mouse X/Y and sets `local_18 = DAT_0099C8A0` (RMB axis), value 1; queued via `0x72E730` (0072e730.c, builds event desc, `FUN_0054aa10`).
3. `GetEvents 0x72C7E0` maps axis -> action: in-game class RMB = action **0x44** (GUI class = 0x1A) (doc 18 trace, `SetEventDescriptions 0x785DF0`).
4. `ProcessInput 0x7AC470`: action 0x44 is outside 0x27..0x40, so line 485-489 `local_38 = FUN_007b12c0(local_8c, local_94)` (asm `007ace0a CALL 0x007b12c0`) with no GUI pre-filter (007ac470.c:485-489; the GUI-only filter list 0xB4..0xBB/0xCE/0xDF/0xF3/0xF4/0xFE..0x106 does not contain 0x44).
5. `HandleInputEvent 0x7B12C0` `case 0x44` (007b12c0.c:135-144; asm `007b13e1`..`007b13ff`): value != 0 -> `CALL 0x007b3810` (asm `007b13ed`), value == 0 -> `CALL 0x007b3830` (asm `007b13fa`).
6. `PerformRButtonDownAction 0x7B3810` (007b3810.c:6-8): `DAT_00a7fcbc = 1; FUN_00413350(gui mgr)`. Release `0x7B3830` sets it 0 and calls `FUN_004133c0`.
7. Consumer: camera director `0x78C350` reads the flag at 0078c350.c:139 (`local_28 = DAT_00a7fcbc`, asm `0078c6e9 MOV EDX,[0xA7FCBC]`), XORs with options bit1 (Mouse Look) at c:~160, and only if the result is nonzero feeds dx/dy to `AddYawInput 0x7F3EA0` / `AddPitchInput 0x7F3C70` (else cursor-at-edge rotation). `ProcessInput` reads it at 007ac470.c:319 (`local_2c`) and :1084 for the cursor-hide / re-center path (c:~497-507 use via `FUN_0072e840`).
8. No other RMB camera reader: `rg a7fcbc` over the dump hits only 007b3810/3830, 7b12c0 (0xDB/0xDC), 7b3560, 7ac470, 78c350, 40a460 (deactivate clear), 75/7d/7e UI-side readers.

So RMB-down => action 0x44 => `0x7B3810` => hold flag => camera follows mouse delta. The flag is the single gate.

## Q2 - Does a GUI control consume RButtonDown?

- Routed, not consumed. `0x7B3810` calls `CSWGuiManager::HandleRMouseDown 0x413350` (00413350.c:6-28) with ECX = `[0xA1B49C]` (asm `007b3821`). It forwards to the hovered control `[mgr+8]` via vtable slot 0x84 (c:20) when `mgr+0x10 == 0`; the return value is discarded by 0x7B3810 (void; asm `007b3827` result unused).
- The flag is set BEFORE the GUI call (c: `DAT_00a7fcbc = 1` precedes it), and nothing clears it on "handled". So a control receiving RMB does NOT stop the camera. This differs from LMB, where `0x7B3680` checks the GUI result (doc 18 step 7).
- `0x418D20` is only the hit-test (00418d20.c:7-14): returns 0 when click-through bit 0x20 of `ctl+0x48` is set or bit 1 is clear. It is not consulted by the RMB path, so the bit-0x20 trick has no effect on RMB.
- Release path `0x4133C0` (004133c0.c) only calls vtable +0x24 on the captured control when `mgr+0xC == 4`.

## Q3 - Swallow site

Preferred: **H-R6a, `0x007B3817`** (in `0x7B3810`, after its prologue).

| item | value |
|---|---|
| function | `0x7B3810` thiscall (ECX = CClientExoAppInternal, unused afterwards), fn-entry aligned 16, `RET` no args (007b3810.asm:12) |
| patched bytes | `C7 05 BC FC A7 00 01 00 00 00` (`MOV dword [0x00A7FCBC],1`), 10 bytes, 007b3810.asm:6 |
| boundaries | previous insn ends at 0x7B3817 (`894DFC` 0x7B3814-16); next insn `8B0D 9CB4A100` at 0x7B3821. Span 0x7B3817..0x7B3821 holds exactly one instruction, no rel branch, no branch target inside (the function is straight-line, 11 asm lines) |
| hook type | detour, `skip_original_bytes = true` (precedent: ranged_ai `0x0074DBDD`, `0x006D850F`), `exclude_from_restore = []`, resume `0x7B3821` |
| live regs | EBP frame valid, `[EBP-4]` = this, ECX still this; EAX/ECX/EDX are dead (next insn reloads ECX; EAX/EDX not read before RET). Wrapper's usual saved set is sufficient; no flags live (next insn MOV) |
| handler | `if (!cursorOverSelfhealRect()) *(int*)0x00A7FCBC = 1;` then return 0 |
| effect | with the cursor over our rects: flag stays 0, GUI `HandleRMouseDown 0x413350` still runs (our HUD control still gets vt+0x84, harmless), release `0x7B3830` writes 0 again (idempotent) |
| hook-span check | no existing hook in `*/*.hooks.toml` or code_patches touches 0x7B38xx (rg); mouse_probe uses 0x7B3560/0x7B3CA0/0x7B3E3C, none overlap. Run `python3 scripts/check_hook_spans.py` after adding |

Alternative (heavier): hook the call site `0x007B13ED` (`CALL 0x007b3810`, 5 bytes `E8 1E 24 00 00`, rel32 so patched bytes contain a relative CALL; stealing it needs the wrapper to re-emit it) with `consumed_exit_address = 0x007B13F2`. This also skips the GUI notification. Not preferred: more fragile than H-R6a.

Cursor position at the hook: `OnMouseMessage` stores the last WM_MOUSEMOVE at `CExoInputInternal+0x3E4/+0x3E8` (y flipped, H-1-y per 0x40A750) and sets them to -1 on button events (0072e030.c:70-71), so do NOT read those. Use the mod's own per-frame hit result (cursor from `GetCursorPos`+`ScreenToClient`, same as the popover code) or call `GetCursorPos` in the handler; the handler runs on the game main thread (same thread as ProcessInput).

## Caveats

- Only the in-game class path (action 0x44). In GUI-mode input classes (`this+0x9C` = 2/3) the engine uses axis action 0x1A, which `7B12C0` case 0x1A (c:359-372) handles as a dialog/cutscene skip-like action; selfheal buttons live on the HUD so 0x44 applies. Untested for menus.
- If the user ever sets Mouse Look = 1 (`options+8` bit1), the camera semantic inverts (`local_28 = (local_28 == 0)`, 0078c350.c:~165), so flag 0 would mean free-look. Ini is 0 on this machine (doc 18 section 1); make the handler no-op (write 1 as vanilla) if bit1 is set, or document it.
- If RMB is pressed elsewhere then dragged over the rect, the flag is already 1 and unaffected (correct).
- The polled `rclickEdge` still sees the physical press, so popover logic is unchanged; only the camera is suppressed.
- Release path unchanged: a press swallowed over the rect and released elsewhere calls `0x7B3830` (flag := 0, `4133C0`), harmless.
- Not tested live; [V] = read from asm/decomp this session, nothing run. Fallback (Shift+LMB) is not needed.
