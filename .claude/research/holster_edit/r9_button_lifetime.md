# r9 recon: CSWGuiButton lifetime, options-panel description, hover dispatch

Status: read-only recon, no code changed, nothing committed.
Dump root: `$D` = `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71`
Exe: `$D/swkotor2_vanilla_6a522e71.exe` (image base 0x400000; vtable reads via PE section table, scratch script `scratchpad/r9/pe.py`).
Convention: `vt[N]` = byte offset N from vtable base (slot = N/4), matching the prompt.

Vtables read from the exe:
- CSWGuiButton 0x987A1C: slot0 0x885AB0, slot3 (0x0C) 0x418750, slot5 (0x14) 0x623320, slot8 (0x20) 0x623320, slot15 (0x3C) 0x41A110, slot16 (0x40) 0x41A290, slot17 (0x44) 0x418D20.
- CSWGuiNavigable (base) 0x987734: slot16 (0x40) 0x4185D0, slot17 (0x44) 0x418D20.
- Options panel P 0x9A701C: slot3 (0x0C) 0x40E770, slot4 (0x10) 0x681880, slot9 (0x24) 0x623320, slot13 (0x34) 0x40EC90, slot16 (0x40) 0x40EAE0, slot17 (0x44) 0x40E7B0.

---

## Q1. Button vt slot 0 (dtor): scalar deleting?

- `$D/decomp/88/00885ab0.asm:7` CALL 0x41A3D0 (ECX = this). `:8-10` test arg & 1. `:13` CALL 0x9196FD. `:18` RET 4 (thiscall, 1 stack arg).
- `$D/decomp/91/009196fd.asm:6` 0x9196FD = `JMP 0x91C6B5` (delete thunk; 0x91C6B5 not traced).
- `$D/decomp/41/0041a3d0.asm:2-39` 0x41A3D0 = dtor body only (fastcall ECX, RET 0 at `:39`): writes vptr 0x987A1C, tears down sub-objects, finally writes vptr 0x987734 and calls 0x418640. No free in the body.

ANSWER: Yes. Slot 0 = scalar deleting dtor 0x885AB0 (thiscall RET 4): calls body 0x41A3D0, then frees via 0x9196FD when arg&1.

## Q2. Button vt slot 5 (0x14): capture release

- Exe: vt+0x14 = 0x623320.
- `$D/decomp/62/00623320.asm:1-8`: PUSH EBP; MOV EBP,ESP; PUSH ECX; MOV [EBP-4],ECX; MOV ESP,EBP; POP EBP; RET at `0x62332A` (`:8`, plain RET, 0 bytes). No callees.
- Shared no-op stub (also vt+0x20 of the button and many other vtables).

ANSWER: RET only, RET 0, no real work. Button capture release does nothing.

## Q3. Options-panel description sequence at 0x8A2320

Calling convention: thiscall, ECX = P (saved to `[EBP-0x40]`), one stack arg = text, RET 4 (`$D/decomp/8a/008a2320.asm:134`, 0x8A24EC).

| step | asm address | call | conv / RET | args (ECX, stack) | effect |
|---|---|---|---|---|---|
| 1 | 0x8A2355 | CALL 0x416EA0 | thiscall, RET 4 (verified) | ECX=P+0x1120, arg=text | string assign into P+0x1120 |
| 2 | 0x8A2367 | CALL 0x7335F0 | thiscall, RET 4 (verified) | ECX=&temp[EBP-0x24], arg=P+0x1120 | temp copy; temp dword0 = char* |
| 3 | 0x8A237C | CALL 0x41D290 | fastcall ECX=LB=P+0x17E0, RET 0 (verified) | none | body loops LB+0x2B0 count; "clear" name NOT verified by body |
| 4 | 0x8A2381-0x8A239B | (inline) | - | - | copy 4 dwords P+0x1034,+0x1038,+0x103C,+0x1040 to rect [EBP-0x1C..-0x10] |
| 5 | 0x8A23A1-0x8A23F2 | CALL 0x919F10 (_strlen, cdecl) then either CALL [vptr+0x08] or CALL 0x4178B0 | 0x4178B0 fastcall ECX, RET 0 (verified) | ECX=P+0x1108 | len > 0: rect.h = (P+0x1108)->vt+0x08 (slot 2, not slot 8); else rect.h = 0x4178B0(P+0x1108) |
| 6 | 0x8A23F5-0x8A240E | CALL [vptr+0x04] on embedded object P+0x1030 | thiscall, callee RET not read | ECX=P+0x1030, arg=&rect | label SetRect-like call; vptr = [P+0x1030] |
| 7 | 0x8A2418-0x8A2430 | CALL 0x41FD90 | thiscall, RET 0x14 (verified `0041fed3`) | ECX=LB, args (&[EBP-0x28], 1, 0, 0, 0) | [EBP-0x28] = P+0x1030 (label address) |
| 8 | 0x8A2435-0x8A244A | CALL 0x41E9A0 | thiscall (uses ECX=LB), RET 8 (verified `0041e9c9`) | ECX=LB, args (P+0x1030, 0) | internal: 0x4142E0 on LB+0x2AC then 0x41E870 on LB |
| 9 | 0x8A245A-0x8A249B | (inline) | - | - | flag -> `[EBP-0x3C]` (formula below) |
| 10 | 0x8A24AB-0x8A24CC | (inline) | - | - | F = P+0x1698; [F+0x48] = ([F+0x48] & ~2) \| flag<<1, i.e. P+0x16E0 bit 1 |
| 11 | 0x8A24D9 | CALL 0x733780 | fastcall ECX=&temp, RET 0 | - | temp dtor; `0x8A24EC` RET 4 |

Flag formula (verified, `:94-111`): flag = (byte[LB+0x2D0] + dword[LB+0x2C4] > dword[LB+0x2A8], signed) AND (dword[LB+0x2B0] > 0, signed) AND NOT (bit 8 of dword[LB+0x2CC]).

Corrections to the prompt's claimed sequence:
- Step 1 is wrong: 0x416EA0 stores the string into P+0x1120. The label P+0x1030 gets only a rect (step 6). Text object P+0x1108 is only measured (step 5), never given text here.
- "vt[8]" is byte offset 8 = slot 2. Prompt's byte-offset convention holds.
- Step 3 name "clear" is unverified; body only shows a loop over LB+0x2B0.
- Step 6 callee RET size not read (no ESP fixup after the call, so callee must pop its arg).

ANSWER: Order and args of steps 2-11 confirmed; step 1 corrected to a string store on P+0x1120 only; the sequence is 0x416EA0, 0x7335F0, 0x41D290, rect copy, strlen+measure, label vt+4(&rect), 0x41FD90(&label,1,0,0,0), 0x41E9A0(label,0), flag to P+0x16E0 bit 1.

## Q4. Who calls a control's vt+0x0C (enter/leave) on mouse move?

Chain (each step verified in the dump unless marked):
1. Event codes 0x41 / 0x42 in HandleInputEvent 0x7B12C0: `$D/decomp/7b/007b12c0.c:113-121` call 0x7B33F0 / 0x7B34A0. 0x7B33F0 (`$D/decomp/7b/007b33f0.c:1-30`) stores a clamped value into this+0x3C8 and returns a changed flag. 0x7B34A0 not read in full (assumed Y analog, +0x3CC).
2. Per-frame caller 0x7AC470: `$D/decomp/7a/007ac470.asm:560-565` ECX = [X+0x274], push [X+0x3C8], CALL 0x412C00 (RET 8). X = input-handler this; not resolved further.
3. 0x412C00 (`$D/decomp/41/00412c00.c:5-90`): stores x,y into its this (`asm 0x412C09-0x412C17`), then `asm 0x412CBE-0x412CD3` CALL 0x411030 with ECX = this and args (x, y, &out3, &out4, 0).
4. 0x411030 (`$D/decomp/41/00411030.asm:16, :32, :39, :41`): walks list this+0x88 (count this+0x8C; secondary list this+0x98 if nonzero), calls item vt+0x40(x,y). out3 ([EBP+0x10]) = hit item, out4 ([EBP+0x14]) = vt+0x40 return value.
5. Options panel P: vt+0x40 = 0x40EAE0 (`$D/decomp/40/0040eae0.c:13-42`). If cursor is in P's rect (P+4..+0x10; centred offset when P+0x48 bit0 is 0), it walks P+0x24 backwards (count P+0x28) and calls child vt+0x44. For a CSWGuiButton child that is 0x418D20 (`$D/decomp/41/00418d20.c:2-14`): pure rect + flag test (+0x48 bit1 set, bit5 clear), no side effects, returns child pointer.
6. 0x412C00 takes the innermost hit (out4) into G+0x08 (decompiler `local_1c` = asm [EBP-0x18]). On change: old vt+0x0C(0) and new vt+0x0C(1), gated by new control's +0x48 bits 2/3 (`$D/decomp/41/00412c00.c:45-52, 79, 103-111`).
7. Button vt+0x0C = 0x418750 (`$D/decomp/41/00418750.c:2-16`): flag 0 -> vt+0x3C(1,1). Flag 1 and +0x48 bit0 clear -> vt+0x3C(0,1) and CALL 0x4122A0(1) (enter-only call; likely the hover sound, callee 0x70AD50 not read). Then 0x419060(flag).
8. Options panel vt+0x0C = 0x40E770 = no-op (`$D/decomp/40/0040e770.c`). The panel itself never gets enter/leave.

Other callers of 0x412C00 (not traced): `$D/decomp/41/00410670.asm:81`, `$D/decomp/41/00414180.asm:53`, `$D/decomp/41/00414230.asm:26`.

Click paths also write G+0x08 (not only clicks, not only 0x413C50):
- 0x413C50 (`$D/decomp/41/00413c50.c:4-22`): fresh 0x411030 hit, writes G+0x08. Called by 0x4131E0 (`$D/decomp/41/004131e0.c`, from 0x7B3560 on event 0x43 press, ECX=[this+0x274], `asm 0x7B3670`) and by 0x413350 (from 0x7B3810 on event 0x44 press, ECX=[0xA1B49C], `$D/decomp/7b/007b3810.asm:8`).
- 0x4134A0 (`$D/decomp/41/004134a0.c`, from 0x7B3850 on event 0x46): writes G+0x08 too.
- G = object written to 0xA1B49C by 0x4100C0 at `$D/decomp/41/004100c0.asm:152`. That it equals [X+0x274] is inferred, not proven.

Premise check: the prompt says M+0x08 is written "by 0x413C50 on clicks". Refuted: 0x412C00 writes it on every mouse move.

Hover-sound conclusion for a mod button listed in P+0x24/P+0x28: YES, via P vt+0x40 (0x40EAE0) to child vt+0x44 (0x418D20), then 0x412C00 to vt+0x0C(1). Pre-conditions: P (or an ancestor) must be an item of G+0x88, and the button must pass 0x418D20 (bit1 set, bit5 clear, cursor inside its rect relative to P).

Caveat: 0x41A290 (button vt+0x40) is a 1-arg highlight setter, not a hit test. A button placed directly in G+0x88 would have the setter called in the 0x411030 loop instead of a hit test. Do not put buttons directly into G+0x88.

ANSWER: Mouse move reaches a P+0x24 child via 0x412C00 -> 0x411030 -> 0x40EAE0 -> 0x418D20, and that child then gets vt+0x0C(1) (highlight + 0x4122A0 sound), provided P is in G+0x88. The click path (0x413C50/0x4134A0) does not add highlight.

---

## Open items
- Which code inserts P (or its ancestor) into G+0x88 (count G+0x8C)? This decides whether the child walk is reached at all.
- Is [HandleInputEvent this+0x274] the same object as the 0xA1B49C global (G)?
- 0x7B34A0 body (Y clamp, +0x3CC?) and 0x4122A0 / 0x70AD50 (hover sound identity) not read.
- Step 6 callee (vptr+0x04 on P+0x1030) and step 3 body (0x41D290) not read.
