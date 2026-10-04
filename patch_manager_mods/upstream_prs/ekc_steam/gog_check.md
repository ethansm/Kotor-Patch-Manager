# EKC GOG-side semantic check (task R-B, 2026-10-04)

Closes port_spec.md risk 3 ("GOG semantics inferred from fingerprints, not a GOG disassembly").
Method: real GOG raw .text (`scripts/disasm_va.py gog`) vs Steam vanilla exe (`disasm_va.py steam`), read-only.
Each hook's whole enclosing function was disassembled on both exes and normalized (absolute addresses, jump/call
targets and `[ebp+-0x1xx/0x2xx]` locals masked) then diffed; the register/slot the C++ reads was traced to its origin.

## Summary

| # | hook (C++ reads) | Steam VA | GOG VA | original_bytes == GOG raw | whole-function norm. diff | verdict |
|---|---|---|---|---|---|---|
| 1 | CaptureActionBarInputK2 (eax=&record, ebp, [ebp-0x10]) | 0x0072CB63 | 0x00613863 | yes `89 85 BC FE FF FF` | 0 diff in first 500 insns (see note) | PASS |
| 2 | MapMovieDeleteToEscapeK2 (ebp, [ebp+0xC]=uMsg, [ebp+0x10]=wParam) | 0x0040C4A6 | 0x0091F696 | yes `8B 45 0C 89 45 CC` | 0 / 186 insns | PASS |
| 3 | DispatchContextCancelK2 (ecx=this) | 0x007AC470 | 0x00459080 | yes `55 8B EC 6A FF` | 0 / 3509 insns | PASS |
| 4 | UpdateActionBarControlsK2 (ecx=this) | 0x0074B3F0 | 0x0051CB70 | yes `55 8B EC 6A FF` | 0 / 898 insns | PASS |
| 5 | ClearActionBarControlsK2 (ecx=this) | 0x0074A750 | 0x0051BEC0 | yes `55 8B EC 51 89 4D FC` | 0 / 17 insns (+callee 0 / 377) | PASS |
| 6 | CancelActionBarKeyboardFocusOnMouseMoveK2 (edx=this; x/y +0x3E4/+0x3E8) | 0x0072E0BC | 0x00614E6C | yes `B8 01 00 00 00` | 0 / 444 insns | PASS |

GOG original_bytes check: `disasm_va.py gog <VA> --bytes 8` at all six VAs returned exactly the toml bytes
(613863: `89 85 BC FE FF FF 83 7D`; 91F696: `8B 45 0C 89 45 CC 81 7D`; 459080: `55 8B EC 6A FF 68 97 B3`;
51CB70: `55 8B EC 6A FF 68 D8 5A`; 51BEC0: `55 8B EC 51 89 4D FC 8B`; 614E6C: `B8 01 00 00 00 E9 33 06`).
Steam bytes at the Steam VAs are identical for the stolen span. All six GOG hook VAs sit on instruction boundaries
(they are the first line objdump prints for each address, and the preceding instruction ends exactly there).

Overall: 6 of 6 PASS. No UNSURE. Hook 1 note: the GOG GetEvents is split in four Ghidra chunks, so only the first
chunk (2700 bytes, ~500 insns, which contains the hook at +0x383) was diffed: the only differences before the chunk
tail are `sub esp,0x254` vs `0x244` and the assertion line number `push 0x601` vs `0x5f3` (neither read by EKC).

## Hook 1: GetEvents DirectInput record loop (GOG 0x613863 / Steam 0x72CB63)

C++ (`CaptureActionBarInputK2(BufferedInputRecord* input, const void* frame)`): `input` = eax, record = {dwOfs @+0,
dwData @+4}; device index = `*(int*)(ebp-0x10)`; compared to `*(int*)K2Config().keyboardDeviceIndex`.

GOG:
```
61385c  add eax,[ebp+ecx-0xfc]        ; eax = base + idx*0x14 (imul eax,eax,0x14 at 613859)
613863  mov [ebp-0x144],eax           ; HOOK  EAX = &record
613869  cmp dword ptr [ebp-0x10],2    ; [ebp-0x10] = device loop index
```
Steam (same instruction sequence, VA +0x119300):
```
72cb5c  add eax,[ebp+ecx-0xfc]
72cb63  mov [ebp-0x144],eax           ; HOOK
72cb69  cmp dword ptr [ebp-0x10],2
```
Frame slot [ebp-0x10] = device index, evidence on both: initialised `mov [ebp-0x10],0` (GOG 613613 / Steam 72C913),
incremented in the loop (`add edx,1; mov [ebp-0x10],edx` 61361f-613622 / 72c91f-72c922), bounded by
`cmp ecx,[eax+0x1b8]` (613634 / 72c934), restarted `mov [ebp-0x10],2` for devices >= 2 (61368b / 72C98B). The GOG
keyboard index global is the same construct: `mov eax,ds:0x997514; imul eax,eax,0xc; lea ecx,[ebp+eax-0xfc]`
(61366a-613672) vs Steam `ds:0x99c884` (72c96a-72c972), reached through the same `cmp [edx+0x410],0` test
(613661 / 72c961). Assertion strings `.\exoinputinternal.cpp` / `GetEvents()` at 0x9974A0/0x9974B8 (GOG push at 6135bc,
6135c1) vs 0x99C810/0x99C828 (Steam 72c8bc, 72c8c1): same +0x5C to the index global. Verdict PASS.

## Hook 2: movie window proc (GOG 0x91F696 / Steam 0x40C4A6)

Function is `ExoMoviePlayerWindowProc` (GOG symbol, sz 693, ret 16 = 4 stdcall WndProc args). Prologue
`push ebp; mov ebp,esp; sub esp,0x3c` then the stolen `mov eax,[ebp+0xC]; mov [ebp-0x34],eax` (GOG 91f696-91f699).

[ebp+0xC] = uMsg, evidence (GOG): the very next code switches on it with window-message constants
`cmp [ebp-0x34],0x201` (WM_LBUTTONDOWN, 91f69c), `sub ecx,0x1c` -> table index (case 0x100..), `0x407/0x408` (WM_USER+...);
byte/jump tables at 0x91f970/0x91f95c: index (0x100-0x1c)=0xE4 -> table value 1 -> 0x91f7ba, the WM_KEYDOWN arm.
Steam identical: byte table 0x40C780, jump table 0x40C76C, idx 0xE4 -> value 1 -> 0x40C5CA.
WM_KEYDOWN arm tests wParam = [ebp+0x10] against VK_ESCAPE and stops the movie:
```
GOG   91f7ba cmp dword ptr [ebp+0x10],0x1b ; jne ; cmp ds:0xa768b8,0 ; je ; push 0 ; push 0 ; mov ecx,ds:0xa768b8 ; call 0x91fe20
Steam 40c5ca cmp dword ptr [ebp+0x10],0x1b ; jne ; cmp ds:0xa1b9e4,0 ; je ; push 0 ; push 0 ; mov ecx,ds:0xa1b9e4 ; call 0x40cc30
```
So `frame+0xC` = uMsg, `frame+0x10` = wParam (and hwnd at +8; lParam +0x14: `mov ecx,[ebp+0x14]; and ecx,0x20000000`
at 91f72f is the ALT bit in the WM_SYSKEYDOWN F4/0x73 arm) on both. It is registered as the window class proc in
`CExoMoviePlayerInternal::StartMovie` (GOG 0x91FE60, sz 1975): WNDCLASS `[ebp-0x90]=0 (style)`, `[ebp-0x8c]=0x91f690`
(lpfnWndProc, 91febc) vs Steam `[ebp-0x8c]=0x40c4a0` (40cccc). Strings "SW Movie Player Window"/"SWMovieWindow" exist at
GOG 0x9A9D50/0x9A9D68. Normalized diff of the whole 700-byte function: 0 lines. Verdict PASS.

## Hook 3: ProcessInput entry (GOG 0x459080 / Steam 0x7AC470)

C++ reads only `ecx` (clientApp, null-checked and passed nowhere). Both are `CClientExoAppInternal::ProcessInput`
(GOG symbol, sz 15798, ret 4). Entry identical: `push ebp; mov ebp,esp; push -1; push <SEH>; ...; mov [ebp-0x660],ecx`
(GOG 4590a9 / Steam 7ac499 = ECX saved as this). Callers on both exes are two `call`s from the main loop, with the
same sequence `fld [float const]; fstp [esp]` (float arg, ret 4) then `mov ecx,[ebp-0x2c8]; call`:
GOG 42eab5/42eabb and 42ebc1 (inside `CClientExoAppInternal::MainLoop` 0x42E160); Steam 782535/78253b and 782641.
Whole-function normalized diff: 0 lines of 3509 insns. Verdict PASS.

## Hook 4: CSWGuiMainInterface::Draw entry (GOG 0x51CB70 / Steam 0x74B3F0)

C++ reads `ecx` = mainInterface and then `[this+0x1C]` (panel manager) and `[this+0x10]`. Both Draw prologues store
ECX as this (`mov [ebp-0x16c],ecx`, GOG 51cb98 / Steam 74b418), then `mov eax,[..]; mov ecx,[eax+0x48]` (flags).
Draw is vtable slot 13 (+0x34) of `CSWGuiMainInterface`: GOG vtable 0x98D734 holds 0x51CB70 at 0x98D768; Steam vtable
0x99DCC4 holds 0x74B3F0 at 0x99DCF8 (+0x34 on both). In-body uses of the offsets the C++ relies on are on the same
instruction lines: `[ecx+0x1c]` GOG 51cc84 / Steam 74b504, `[edx+0x10]` GOG 51d973 / Steam 74c1f3. Whole-function
diff: 0 lines of 898 insns. Verdict PASS.

## Hook 5: ~CSWGuiMainInterface scalar deleting destructor (GOG 0x51BEC0 / Steam 0x74A750)

C++ reads `ecx` = this. Both are the full 17-instruction thunk:
```
GOG   51bec0 push ebp; mov ebp,esp; push ecx; mov [ebp-4],ecx; mov ecx,[ebp-4]; call 0x51c510 ; ... and eax,1 ; je ; push ecx ; call 0x921321(operator delete) ; ret 4
Steam 74a750 push ebp; mov ebp,esp; push ecx; mov [ebp-4],ecx; mov ecx,[ebp-4]; call 0x74ad90 ; ... and eax,1 ; je ; push ecx ; call 0x9196fd(operator delete) ; ret 4
```
Slot 0 of the CSWGuiMainInterface vtable on both (GOG 0x98D734 -> 0x51BEC0, Steam 0x99DCC4 -> 0x74A750). The inner
destructors (GOG 0x51C510, Steam 0x74AD90, 377 insns) normalize to 0 diff lines. Hook is the whole 7-byte prologue
(4 instructions) on both. Verdict PASS.

## Hook 6: BufferEvent mouse-move case (GOG 0x614E6C / Steam 0x72E0BC)

C++ (`CancelActionBarKeyboardFocusOnMouseMoveK2(void* inputInternal)`): edx = this; `ReadInt(this,0x3E4)` = X,
`ReadInt(this,0x3E8)` = Y.
```
GOG                                                  Steam
614de6 mov [ebp-0x1c],ecx    ; this saved            72e036 mov [ebp-0x1c],ecx
614e49 mov ecx,[ebp-0x1c]                            72e099 mov ecx,[ebp-0x1c]
614e4c mov [ecx+0x3e4],eax   ; X = movsx(lParam lo)  72e09c mov [ecx+0x3e4],eax
614e55 mov eax,[edx+0xc]; shr eax,0x10; and ..ffff   72e0a5 (same)
614e60 movsx ecx,ax                                  72e0b0 movsx ecx,ax
614e63 mov edx,[ebp-0x1c]    ; EDX = this            72e0b3 mov edx,[ebp-0x1c]
614e66 mov [edx+0x3e8],ecx   ; Y = movsx(lParam hi)  72e0b6 mov [edx+0x3e8],ecx
614e6c mov eax,1             ; HOOK (5 bytes)        72e0bc mov eax,1
614e71 jmp 0x6154a9                                  72e0c1 jmp 0x72e6f9
```
So at the hook EDX = this (CExoInputInternal; `[ebp-0x1c]` is the ECX saved at entry, 614de6), EDX is not written
between 614e63 and 614e6c, and +0x3E4/+0x3E8 are the two fields stored one instruction earlier. Same fields are reset to
-1 elsewhere in the function (GOG 614ede/614eeb, Steam 72e12e/72e13b), consistent with "mouse x/y". Event dispatch
`sub eax,0x200` (614dfb) maps the WM_MOUSEMOVE-style event 0x200 to case 0. Only callers: GOG 0x6167e1, Steam 0x72fbe1
(one each). Whole-function diff: 0 lines of 444 insns. Verdict PASS.

## Residual caveats (not blockers)

- All evidence is static; nothing was run under Wine/in game.
- "Same meaning" for registers is shown by identical normalized instruction streams plus matching callers/vtable slots;
  the C++ constants (vtables, getControlAt, prev/next, etc.) in port_spec section 2 are a separate check.
- The 6 hooks do not read any slot other than those listed; `[ebp-0x144]`/`[ebp-0x1c]` etc. are written by the original
  code and are untouched by EKC.
