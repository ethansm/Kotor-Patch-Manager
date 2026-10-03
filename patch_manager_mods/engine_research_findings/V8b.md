# V8b - Dialogue letterbox draw path: why could bars of target h 267 be invisible? (K2 Steam swkotor2.exe, sha256 6a522e71...)

Summary (5 lines)
1. The static path draws the bars: the manager calls letterbox `Draw` 0x8BB260 (vtable +0x34) every frame; it slides each bar linearly (~0.5 s, ~34 frames at 60 fps), pushes the rect through vtable +4 `SetExtent` 0x8BAC30 -> 0x40E980 and ends in the base panel `Draw` 0x40EC90, which calls the fill 0x479460 (a black quad, `glViewport` + unit quad). No clamp rejects h 267 (rule: x,y >= 0, w,h > 0, x+w <= W, y+h <= H).
2. Vanilla -22 IS skipped (`CMP [ECX+0x10],0 / JLE` at 0x40ECC8): top bar not drawn. Refinement of V1 B5: the bottom bar is NOT a 22 px strip; `SetExtent` mode 2 recomputes h = H - y, so vanilla draws a 200 px band (y 1400..1600) under the replies, and the fix draws 267 px (y 1333..1600).
3. Bar alpha is `[this+0x50]`: 1.0 (vanilla ctor/ResetFade) or 0.5 with the live `semi-transparent-letterbox` (alpha byte 128). Nothing in the dump writes 0 to a mode-1/2 bar. So the likeliest reading of "didn't notice" is a 50% black dimming of the top/bottom 267 px, not opaque bars (UNCLEAR until a screenshot).
4. The "letterbox only for cinematic conversations" idea is REFUTED: all conversations except `ConversationType == 1` (Computer) use panel `[+0x40]` and call SetTop/SetBottom (0x7BFB10); the 12 logged pairs prove it for the user's talks.
5. Probe windows (log-only, all clean, no live overlap, nearest hook `0x8BB3C3` is >= 149 bytes away): 0x8BB461 (6 B), 0x8BB52B (6 B), 0x8BB538 (9 B), 0x40ED9F (5 B), 0x7CEB95 (7 B).

ROOT = `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` (decomp/<bb>/<addr8>.{c,asm}). Constants read from the vanilla exe (`backups/vanilla_clean/swkotor2.exe`, PE section map). Evidence log: `engine_bug_research/fixtures/letterbox_log_1003_0112.txt`. Names below are hypotheses unless a content anchor is quoted. The bar formula, constants and fs are V1/V7 and are not re-derived.

---------------------------------------------------------------------------------------------------

## (a) Function table

| addr | proposed name | conf | purpose | evidence |
|---|---|---|---|---|
| 0x008BB260 | CSWGuiDialogLetterbox::Draw | high | per-frame slide of the bar rect, then base draw | vtable slot +0x34 of the class vtable 0x9A7CBC = 0x8BB260 (exe bytes at 0x9A7CF0 `60 b2 8b 00`); manager loop `0x4113a0` calls `(**(code **)(*local_10 + 0x34))(param_2)` |
| 0x008BAC30 | CSWGuiDialogLetterbox::SetExtent | high | vtable +4: forces y=0 (mode 1) or h = H - y (mode 2), then base SetExtent | vtable 0x9A7CC0 = `30 ac 8b 00`; decomp `if (*(int *)(param_1 + 0x88) == 1) { local_10 = 0; }` `else if (... == 2) { local_8 = *(short *)(*(int *)(param_1 + 0x1c) + 0x6e) - local_10; }` |
| 0x0040E980 | CSWGuiPanel::SetExtent | high | stores rect at +4/+8/+0xC/+0x10, forwards (0,0,w,h) to child `[+0x60]` | decomp `*(undefined4 *)(param_1 + 0x10) = param_2[3];` ... `(**(code **)(**(int **)(param_1 + 0x60) + 4))(&local_14);` |
| 0x008BB550 / 0x008BB590 | slide step (top / bottom) | high | `ftol((target - start) / 0.5 * dt)` | decomp `FUN_0091c860(*(int *)(param_1 + 0x74) - *(int *)(param_1 + 0x84));` / `(*(int *)(param_1 + 0x6c) - *(int *)(param_1 + 0x7c))`; asm `008bb56e dc35087f9800 FDIV double ptr [0x00987f08]` (=0.5), `008bb577 d808 FMUL float ptr [EAX]` (dt) |
| 0x0040EC90 | CSWGuiPanel::Draw (base) | high | validity gate, fill, children | name_map `CSWGuiPanel::Draw` medium; decomp `if (((((*(uint *)(param_1 + 0x48) >> 6 & 1) != 0) && (-1 < *(int *)(param_1 + 4))) && (-1 < *(int *)(param_1 + 8))) && ((0 < *(int *)(param_1 + 0x10) && (0 < *(int *)(param_1 + 0xc))))) {` |
| 0x00479460 | GUI fill rect (glViewport + unit quad) | high | draws a coloured/alpha quad; returns 0 if rejected | decomp `glViewport((int)local_14,local_3c,param_3,param_4);` `glColor4ubv(&local_60);` `glVertex4f(0,0,0,0x3f800000);` ; rejects `(param_3 < 1)`, `(param_4 < 1)`, `(DAT_009f42a4 <= param_1)`, `(DAT_009f42a8 <= param_2)` |
| 0x008BAB20 | letterbox "resize to text" (name SPECULATIVE) | med | grows target (slide pending) or sets rect directly (slide done) | decomp `if (*(int *)(param_1 + 0x8c) == 0) {...} else { *(undefined4 *)(param_1 + 0x68) = param_2; ... *(int *)(param_1 + 0x10) = param_5; }` |
| 0x008BA980 | CSWGuiDialogLetterbox::Constructor | high | vtable 0x9A7CBC, `[+0x88]=0`, `[+0x8C]=0`, colour `[+0x54..]` from `DAT_00a7fd34..3c` | asm `008baa6e c7808c00000000000000 MOV dword ptr [EAX + 0x8c],0x0` |
| 0x008BAFD0 | CSWGuiDialogLetterbox::ResetFade | high | alpha `[+0x50]` := 0 for mode 3, 1.0 for modes 0/1/2 (0.5 with the live patch) | asm `008bb043 d9e8 FLD1`..`008bb09c d95a50 FSTP float ptr [EDX + 0x50]` (path replaced by hook 0x8BB043) |
| 0x007BFB10 | CGuiInGame::<ShowDialogPanel> (SPECULATIVE) | med | adds SetFull/SetTop/SetBottom panels to the manager | asm `007bfc23 e878b00f00 CALL 0x008baca0`, `007bfc44 e857b10f00 CALL 0x008bada0` |
| 0x007C6A20 | CGuiInGame::CloseDialog (mapper name, medium) | med | resets done flag + ResetFade on the 3 panels at conversation end | asm `007c6dff c7808c00000000000000 MOV dword ptr [EAX + 0x8c],0x0` (x3: 0x7C6DFF/0x7C6E29/0x7C6E53) |
| 0x006C53F0 | dialog-open handler (anchor: string "ConversationType" 0x99A16C) | med | reads ConversationType, selects panel `[+0x44]` (Computer) or `[+0x40]` | strings.tsv `0099a16c ConversationType 1 006c53f0`; asm `006c55d2 686ca19900 PUSH 0x99a16c` |
| 0x007BFA50 | CGuiInGame::<SelectDialogPanel> (SPECULATIVE) | med | `[+0x3C]` := `[+0x44]` (arg 1) or `[+0x40]` (else) | decomp `*(undefined4 *)(param_1 + 0x3c) = *(undefined4 *)(param_1 + 0x44);` / `... = *(undefined4 *)(param_1 + 0x40);` |
| 0x007CEB80 | CGuiInGame::SetGlobalDialogState (mapper, medium) | med | `DAT_00a309bc = (arg == 1)` | asm `007ceb99 0f94c2 SETZ DL`, `007ceb9c 8815bc09a300 MOV byte ptr [0x00a309bc],DL` |
| 0x00410530 | GuiManager::AddPanel (SPECULATIVE) | med | add to manager list, no flag bit 2/6 change | decomp `iVar1 = FUN_00410890(param_2); if (iVar1 == -1) { param_2[0x12] = param_2[0x12] & 0xfffffff7U | (uint)((param_3 & 2) != 0) << 3;` |
| 0x00410800 | GuiManager::Contains (name_map; Ghidra FID label "IsCached" is wrong) | med | `FUN_00410890(panel) != -1` | decomp `iVar1 = FUN_00410890(param_1); return (uint)(iVar1 != -1);` |

---------------------------------------------------------------------------------------------------

## (b) Findings

Object layout used below (letterbox `this`, all offsets hex bytes, from the asm): `+4 x, +8 y, +0xC w, +0x10 h` (current rect, written by base SetExtent); `+0x1C` GUI manager; `+0x48` panel flags; `+0x50` alpha (float); `+0x54/58/5C` colour; `+0x68 x, +0x6C y, +0x70 w, +0x74 h` target rect; `+0x7C` bottom slide origin y (= H); `+0x84` top slide origin h (= 0); `+0x88` mode; `+0x8C` slide-done flag; `+0x98..0xA4` fade state (mode 3). CGuiInGame: `+0x60` top bar, `+0x64` bottom bar, `+0x68` full panel, `+0x3C` current dialog panel, `+0x40` cinematic panel, `+0x44` computer panel.

### Q1. Draw 0x8BB260: modes, slide step, finish, SetExtent target (CONFIRMED)

Mode values (`[this+0x88]`): 0 = constructor default (asm `008baa61 c7828800000000000000 MOV dword ptr [EDX + 0x88],0x0`), 1 = SetTop (`008bad7c c7818800000001000000 MOV dword ptr [ECX + 0x88],0x1`), 2 = SetBottom (`008baf01 c7818800000002000000 MOV dword ptr [ECX + 0x88],0x2`), 3 = SetFull (`008baf3f c7828800000003000000 MOV dword ptr [EDX + 0x88],0x3`, which also calls ResetFade `008baf4c e87f000000 CALL 0x008bafd0`).

Draw structure (decomp 0x8BB260):
```
if ((param_1[0x22] == 3) && ...) { ... param_1[0x14] = (int)local_54; }     // mode 3 only: alpha fade
if (param_1[0x23] == 0) {                                                  // slide not finished
  if ((float)_DAT_00987f08 < param_2) { param_2 = _DAT_0098c014; }          // dt clamp
  if (param_1[0x22] == 1) { ... } else if (param_1[0x22] == 2) { ... }
}
FUN_0040ec90(param_2);                                                      // base panel Draw, ALWAYS
```
asm: `008bb26c 83b88800000003 CMP dword ptr [EAX + 0x88],0x3` / `008bb273 0f8553010000 JNZ 0x008bb3cc` (skip the mode-3 alpha block); the semi-transparent hook 0x8BB3C3 `d945b0 FLD [EBP-0x50]` / `d95950 FSTP [ECX+0x50]` lies inside that mode-3 block (0x8BB27C..0x8BB3C9), so it does not touch bar alpha. Slide gate: `008bb3cf 83ba8c00000000 CMP dword ptr [EDX + 0x8c],0x0` / `008bb3d6 0f855b010000 JNZ 0x008bb537`. dt clamp: `008bb3df dc1d087f9800 FCOMP double ptr [0x00987f08]` (0.5), `008bb3ec d90514c09800 FLD float ptr [0x0098c014]` / `008bb3f2 d95d08 FSTP float ptr [EBP + 0x8]` (dt := 0.5f when dt > 0.5). `0x987F08` bytes `00 00 00 00 00 00 e0 3f` = 0.5 (vanilla exe).

Top (mode 1), asm 0x8BB3F8..0x8BB467: `008bb3f8 83b88800000001 CMP dword ptr [EAX + 0x88],0x1`; target rect copied from `+0x68..+0x74` into `[EBP-0x18..-0xC]` (`008bb404 83c168 ADD ECX,0x68` ... `008bb418 8b410c MOV EAX,[ECX+0xC]`, `008bb41b 8945f4 MOV [EBP-0xC],EAX`); `008bb427 3b4274 CMP EAX,dword ptr [EDX + 0x74]` / `008bb42a 7d17 JGE 0x008bb443` (current h `[+0x10]` < target h `[+0x74]`: still sliding): `008bb433 e818010000 CALL 0x008bb550` then `008bb43b 034210 ADD EAX,dword ptr [EDX + 0x10]` / `008bb43e 8945f4 MOV [EBP-0xC],EAX` (new h = old h + step); else `008bb446 c7808c00000001000000 MOV dword ptr [EAX + 0x8c],0x1` (done, h = target). Then `008bb45f ffd2 CALL EDX` (EDX = `[EAX+4]`, vtable +4 = SetExtent) and `008bb467 890db409a300 MOV dword ptr [0x00a309b4],ECX` (ECX = `[this+0x10]`).

Bottom (mode 2), asm 0x8BB472..0x8BB531: `008bb4a8 3b516c CMP EDX,dword ptr [ECX + 0x6c]` / `008bb4ab 7e29 JLE 0x008bb4d6` (current y `[+8]` > target y `[+0x6C]`: sliding up): `008bb4b4 e8d7000000 CALL 0x008bb590`, `008bb4c2 0355d4 ADD EDX,dword ptr [EBP + -0x2c]` (y := y + step, step is negative), `008bb4ce 2b4dd4 SUB ECX,dword ptr [EBP + -0x2c]` (h := h - step, overwritten later by SetExtent); else `008bb4d9 c7828c00000001000000 MOV dword ptr [EDX + 0x8c],0x1` plus `008bb4f1 e85a42e8ff CALL 0x0073f750` / `008bb4f8 e8e301f1ff CALL 0x007cb6e0` and the target rect is passed unchanged. `008bb529 ffd0 CALL EAX` (SetExtent) and `008bb531 8915b809a300 MOV dword ptr [0x00a309b8],EDX` (EDX = `[this+8]`).

Step helpers (asm 0x8BB550..0x8BB581): `008bb55f 8b5074 MOV EDX,dword ptr [EAX + 0x74]`, `008bb562 2b9184000000 SUB EDX,dword ptr [ECX + 0x84]`, `008bb56b db45f8 FILD`, `008bb56e dc35087f9800 FDIV double ptr [0x00987f08]`, `008bb574 8b4508 MOV EAX,dword ptr [EBP + 0x8]`, `008bb577 d808 FMUL float ptr [EAX]`, `008bb579 e8e2120600 CALL 0x0091c860` (truncating ftol). So step = trunc((target_h - `[+0x84]`) / 0.5 * dt) = trunc(2 * 267 * dt) for the top (`[+0x84]` = 0, set by SetTop at `008bad14 c7808400000000000000`), and trunc(2 * (1333 - 1600) * dt) for the bottom (`[+0x7C]` = H, `008bae1e 89507c MOV dword ptr [EAX + 0x7c],EDX`).

Timing: PER FRAME, scaled by the frame delta (`dt` = float at `0x009f62b0`, pushed by MainLoop `007837da d905b0629f00 FLD float ptr [0x009f62b0]` ... `007837ef e8acdbc8ff CALL 0x004113a0`; unit seconds is SPECULATIVE but the 0.5 clamp and the `/0.5` factor fit). Nominal slide = 0.5 s. Computed (python, trunc, h target 267): 30 fps step 17 / 16 frames; 60 fps step 8 / 34 frames (0.57 s); 144 fps step 3 / 89 frames; 240 fps step 2 / 134 frames; **dt < 1/534 s (>= 535 fps) gives step 0 and the slide never finishes** (h stays 0, bar never drawn; SPECULATIVE whether the user can reach that, the frame limiter `DAT_00a1b710` was not read). The last step can overshoot (e.g. h 272 for one frame) before the done frame snaps to 267.

Slide-done flag lifetime: written 0 only by the ctor (`008baa6e`) and by CloseDialog 0x7C6A20 at every conversation end (`007c6dff`, `007c6e29`, `007c6e53`, each followed by `CALL 0x008bafd0`); written 1 by Draw (`008bb446`, `008bb4d9`). `SetTop` does not reset it (decomp SetTop writes `+0x84, +0x6C, +0x68, +0x70, +0x74, +8, +4, +0xC, +0x10, +0x88` only). So a SetTop with a stale flag 1 (a conversation that ended without 0x7C6A20) would leave `[+0x10]` = 0 (`008bad72 c7401000000000 MOV dword ptr [EAX + 0x10],0x0`) with no slide, until `SetReply` sets the rect directly (below). The log shows normal closes (second and later SETTOP of the same `lb` print `mode(+0x88)=1`, i.e. only the mode persists).

vtable +4 SetExtent target = 0x8BAC30 (vtable 0x9A7CBC+4: bytes `30 ac 8b 00`; asm 0x8BAC30):
```
008bac5f 837de401   CMP dword ptr [EBP + -0x1c],0x1     ; mode 1: y := 0
008bac6d c745f400000000 MOV dword ptr [EBP + -0xc],0x0
008bac82 0fbf516e   MOVSX EDX,word ptr [ECX + 0x6e]      ; mode 2: h := H - y
008bac86 2b55f4     SUB EDX,dword ptr [EBP + -0xc]
008bac89 8955fc     MOV dword ptr [EBP + -0x4],EDX
008bac93 e8e83cb5ff CALL 0x0040e980
```
and 0x40E980 stores the rect: `0040e994 8911 MOV [ECX],EDX` ... `0040e9a5 89410c MOV [ECX+0xC],EAX` with `ECX = this+4` (`0040e98f 83c104 ADD ECX,0x4`), i.e. `+4 x, +8 y, +0xC w, +0x10 h`, then calls the child's vtable +4 (`0040e9e4 8b10 MOV EDX,[EAX]`, `0040e9e6 8b4204 MOV EAX,[EDX+4]`, `0040e9eb`..`ffd0 CALL EAX`) with (0,0,w,h). So the control rect used for drawing is exactly what Draw/SetTop/SetBottom stored.

### Q2. What renders the bar, with which colour and alpha (CONFIRMED path; colour value partly SPECULATIVE)

Draw ends with `008bb541 e84a37b5ff CALL 0x0040ec90` (base panel Draw; ret 4, arg = dt) on every frame regardless of mode. The letterbox draws through this base call: vtable slot +0x34 of 0x9A7CBC (read from the exe) is 0x8BB260, whose last act is the base Draw call; no separate fill code exists in the letterbox functions (their asm contains no GL call).

Base Draw 0x40EC90 gate (asm):
```
0040ec9c 8b4848   MOV ECX,dword ptr [EAX + 0x48]
0040ec9f c1e906   SHR ECX,0x6        0040eca2 83e101 AND ECX,0x1     0040eca5 0f848f010000 JZ 0x0040ee3a   ; flag bit 6
0040ecae 837a0400 CMP dword ptr [EDX + 0x4],0x0    ... JL 0x0040ee3a        ; x >= 0
0040ecbb 83780800 CMP dword ptr [EAX + 0x8],0x0    ... JL 0x0040ee3a        ; y >= 0
0040ecc8 83791000 CMP dword ptr [ECX + 0x10],0x0   0040eccc 0f8e68010000 JLE 0x0040ee3a   ; h > 0
0040ecd5 837a0c00 CMP dword ptr [EDX + 0xc],0x0    0040ecd9 0f8e5b010000 JLE 0x0040ee3a   ; w > 0
0040ed06 394de0   CMP dword ptr [EBP + -0x20],ECX  0040ed09 0f8f2b010000 JG 0x0040ee3a    ; x + w <= W ([mgr+0x6C])
0040ed37 3945d4   CMP dword ptr [EBP + -0x2c],EAX  0040ed3a 0f8ffa000000 JG 0x0040ee3a    ; y + h <= H ([mgr+0x6E])
```
Flag bit 6 and bit 2 come from the base ctor: `0040e6b2 83c904 OR ECX,0x4`, `0040e6c1 83c940 OR ECX,0x40`, `0040e6d0 83e1cf AND ECX,0xffffffcf`, `0040e6df 81e1fffcffff AND ECX,0xfffffcff`; neither the letterbox ctor 0x8BA980 nor SetTop/SetBottom/SetFull/ResetFade/Draw/SetExtent (grep of `0x48]` in those .asm files: only unrelated `[EBP + -0x48]` locals) write `[+0x48]`. The manager only runs `Draw` for panels with bit 2 (`4113a0`: `else if ((((uint)local_10[0x12] >> 2 & 1) != 0) && (iVar1 = FUN_00410830(local_10), iVar1 == -1)) { (**(code **)(*local_10 + 0x34))(param_2);`).

Fill call: `0040ed7b d94150 FLD float ptr [ECX + 0x50]` / `0040ed7e d91c24 FSTP float ptr [ESP]` (alpha), `0040ed83..0040ed89 83c254 ADD EDX,0x54` (colour pointer), then `0040ed9a e8c1a60600 CALL 0x00479460`, `0040ed9f 83c41c ADD ESP,0x1c`, `0040eda2 85c0 TEST EAX,EAX`, `0040eda4 0f848b000000 JZ 0x0040ee35` (no children if the fill was rejected). The fill 0x479460:
```
00479466 833d302aa30000 CMP dword ptr [0x00a32a30],0x0      ; global kill switch: nonzero -> return 0 (no writer found in the dump: only 8 `CMP` readers)
00479479 034510 ADD EAX,dword ptr [EBP + 0x10] ... 0047947c 782a JS ; 00479486 837d1000 CMP dword ptr [EBP + 0x10],0x0 / 0047948a 7e1c JLE   ; w < 1
0047948c 837d1400 CMP dword ptr [EBP + 0x14],0x0 / 00479490 7e16 JLE                                                                     ; h < 1
00479495 3b15a4429f00 CMP EDX,dword ptr [0x009f42a4] / 0047949b 7d0b JGE ; x >= display W   ; 004794a0 3b05a8429f00 CMP EAX,[0x009f42a8] / 004794a6 7c07 JL ; y < display H
00479590 ff156c639800 CALL dword ptr [0x0098636c]            ; glViewport(x', vpH - y - h, w, h)
0047959b dc0da0849800 FMUL double ptr [0x009884a0]           ; colour * 255.0 (bytes 00 00 00 00 00 e0 6f 40 = 255.0)
004795f5 dc0da0849800 FMUL double ptr [0x009884a0]  004795f0 db1a FISTP ; alpha * 255, FRNDINT (round to nearest even)
0047964d e82ee30900 CALL 0x00517980 ; 00479659 83c001 ADD EAX,1 ; 0047965c 0f84e3020000 JZ 0x00479945   ; colour == (-1,-1,-1) [0x987474 bytes 00 00 80 bf] -> no fill
00479662 837dfc00 CMP dword ptr [EBP + -0x4],0x0 ; 00479666 0f8ed9020000 JLE 0x00479945                     ; alpha byte must be > 0
0047968f e8ece20900 CALL 0x00517980 ; 0047969b c745e800000096 MOV dword ptr [EBP + -0x18],0x96000000       ; colour == (-2,-2,-2) [0x98bd70 = -2.0f] -> black alpha 0x96
glDisable(textures) ; glColor4ubv(&local_60) ; glBegin(quad) ; 4x glVertex4f(0|1, 0|1, 0, 1) ; glEnd   (decomp)
```
So the bar = ONE untextured colour quad over the viewport `(x, y, w, h)`: no clear, no scissor, no `FUN_004113a0` glClear (that is the 4:3 pillarbox, V1 B6). Colour = `+0x54/58/5C`: ctor `008baa3c 8b0d34fda700 MOV ECX,dword ptr [0x00a7fd34]` ... `008baa53 894808 MOV dword ptr [EAX + 0x8],ECX` copies `DAT_00a7fd34/38/3c`; those are `.bss` (VA > raw data; python read: "bss/none") and the only writer is `0x9846F0` (decomp `DAT_00a7fd34 = 0; DAT_00a7fd38 = 0; DAT_00a7fd3c = 0;` asm `009846f5 d91d34fda700 FSTP float ptr [0x00a7fd34]`; a static initializer with no caller in callgraph: that it runs before the ctor is SPECULATIVE but .bss is zero anyway) -> black (0,0,0), neither the (-1) nor the (-2) magic.

Alpha `[this+0x50]` writers for bars: base ctor `0040e700 d9e8 FLD1` / `0040e702 d95850 FSTP float ptr [EAX + 0x50]` (1.0); ResetFade 0x8BAFD0 non-mode-3 path ends with `008bb096 8b55fc MOV EDX,[EBP-4]`.. `008bb09c d95a50 FSTP float ptr [EDX + 0x50]` = 1.0 (the `FLD1`/`FCOMP double [0x00986b50]` chain: 0x986B50 = 1.0, 0x986B38 = 0.0); callers of ResetFade on the bars: ctor `008baa59 e872050000 CALL 0x008bafd0` and CloseDialog (`007c6e0f`, `007c6e39`). `SetFade` 0x8BB0E0 is reached only through `0x7CB6A0` which tests `[+0x68]` and passes the FULL panel, so it never touches the bars (decomp 007cb6a0 `if (*(int *)(param_1 + 0x68) != 0) { FUN_008bb0e0(...)`; `FUN_007cb350/007cb3f0` call ResetFade without a bar argument; which panel they pass in ECX not checked: SPECULATIVE, alpha would be 1.0/0.5 anyway).

Live alpha: `semi-transparent-letterbox` (live in `patch_config.toml`, `id = "semi-transparent-letterbox"`) hooks 0x8BAA5E (ctor, 26 B -> `MOV dword ptr [EDX+0x50],0x3F000000`), 0x8BB043 (ResetFade opaque path, 92 B -> `C7 40 50 00 00 00 3F`), 0x8BB108 (SetFade), 0x8BB3C3 (Draw, mode 3 only). So bars are black at alpha 0.5 -> alpha byte `round(0.5 * 255) = round(127.5)` = 128 (FRNDINT ties to even). Alpha 0 is impossible for a mode-1/2 bar from the code shown (the only zero-alpha path is the mode-3 branch of ResetFade, `008bafdc 83b88800000003 CMP dword ptr [EAX + 0x88],0x3`). Could a texture-less fill make it invisible? No: the fill is texture-less by design and visible (black x alpha).

Side note (SPECULATIVE): `DAT_00a309b4/b8/bc` (top bar bottom edge, bottom bar top edge, active flag) are read by five post-process composite passes (`0x4426F0 004426f0.asm:271 0fb605bc09a300 MOVZX EAX,byte ptr [0x00a309bc]`, `00442b1b db05b409a300 FILD`, `00442b2c db05b809a300 FILD`; same in 0x4421F0/0x442C90/0x443190/0x445310, called from 0x437BD0), where the full-screen quad is limited to y in [b4, b8] while the flag is 1. So with the fix the screen-space effect quad is limited to y 267..1333 (or 262.. after SetReply); what the framebuffer holds outside that band is not shown by the code.

### Q3. Is the panel shown at all; which conversations reach SetTop/SetBottom (CONFIRMED reach, REFUTES the "only cinematic" explanation)

Show path 0x7BFB10 (ret 12), asm:
```
007bfbd1 e82a0cc5ff CALL 0x00410800        ; Contains(manager [+0x38], panel [+0x3C])
007bfbd8 0f85bc000000 JNZ 0x007bfc9a       ; already in the manager list: skip the whole block
007bfbee 3b4240 CMP EAX,dword ptr [EDX + 0x40] / 007bfbf1 756e JNZ 0x007bfc61   ; current panel == cinematic panel [+0x40]
007bfbf3 833d30fda70000 CMP dword ptr [0x00a7fd30],0x0 / 007bfbfa 7563 JNZ 0x007bfc5f
007bfc02 e809b30f00 CALL 0x008baf10 (SetFull on [+0x68]) ; 007bfc18 e81309c5ff CALL 0x00410530 (show, args panel,0,1)
007bfc23 e878b00f00 CALL 0x008baca0 (SetTop  on [+0x60]) ; 007bfc39 e8f208c5ff CALL 0x00410530
007bfc44 e857b10f00 CALL 0x008bada0 (SetBottom on [+0x64]); 007bfc5a e8d108c5ff CALL 0x00410530
```
`0x00A7FD30`: `.bss` (python: "bss/none"), only readers `007bfbf3` and `007c6d99`, no writer in the dump (grep of `0x00a7fd30`: 2 lines, both `CMP`); the log proves the block ran 12 times. The panel is added with `FUN_00410530(mgr, panel, 0, 1)`: it does not touch flag bits 2/6 (decomp: only bit 3 from `param_3 & 2` and the list insert `FUN_0083ea60`), then `(**(code **)(*param_2 + 0x48))()` and `FUN_00411170()`.

Which conversations: the panel `[+0x3C]` is chosen by `0x7BFA50(mode, flag)`: `if (param_2 == 1) { ... *(undefined4 *)(param_1 + 0x3c) = *(undefined4 *)(param_1 + 0x44);` (Computer panel, `FUN_008bc620` constructs it) `else { *(undefined4 *)(param_1 + 0x3c) = *(undefined4 *)(param_1 + 0x40); }` (asm `007bfade 89503c MOV dword ptr [EAX + 0x3c],EDX` / `007bfafc 894a3c MOV dword ptr [EDX + 0x3c],ECX`). Caller 0x6C53F0 picks the mode from the dialog's `ConversationType`: `006c55d2 686ca19900 PUSH 0x99a16c` ("ConversationType"), `006c55de e89d360500 CALL 0x00718c80`, `006c55e3 8945cc MOV dword ptr [EBP + -0x34],EAX`, `006c5762 837dcc01 CMP dword ptr [EBP + -0x34],0x1` / `006c5766 753d JNZ 0x006c57a5`, then `006c579b e8b0a20f00 CALL 0x007bfa50` with `PUSH 1` (type 1) versus `006c57b4 6a00 PUSH 0x0`, `006c57b6 6a00 PUSH 0x0`, `006c57c5 e886a20f00 CALL 0x007bfa50` (every other type, incl. 2 which only sets `[dlg+0x16c]=1` at `006c5758`). So: Computer (type 1) = no letterbox; **all other types (cinematic 0, type 2) use panel `[+0x40]` and SetTop/SetBottom**. The label meanings (0 cinematic, 1 computer) are SPECULATIVE (only "1 selects +0x44" is shown). The block needs no camera shot: nothing in 0x7BFB10 tests a camera flag. Since the log shows 12 SETTOP/SETBOTTOM pairs, each of those 12 conversations was a non-Computer one with the bars added to the manager; "bars are only for cinematic conversations" cannot explain the user's observation.

Active flag `DAT_00a309bc` (clip, not drawing): set by 0x7CEB80 `007ceb95 837d0801 CMP dword ptr [EBP + 0x8],0x1`, `007ceb99 0f94c2 SETZ DL`, `007ceb9c 8815bc09a300 MOV byte ptr [0x00a309bc],DL`; arg 1 pushed at dialog start `00545b9d 6a01 PUSH 0x1` / `00545bac e8cf8f2800 CALL 0x007ceb80` in 0x545760 (mapper name `CSWSObject::StartDialog`, after its `00545a7d e86ef91700 CALL 0x006c53f0`), arg 0 at close `007c6cb3 6a00 PUSH 0x0` / `007c6cbb e8c07e0000 CALL 0x007ceb80`. Other arg-1 callers (not examined further): `0x5D7BAD`, `0x66B590`, `0x77D107`.

SetReply interaction (bars change size after the slide): `0x8BB8D0` is gated on the TOP bar's done flag: `008bb8f2 8b4260 MOV EAX,dword ptr [EDX + 0x60]` / `008bb8fb 8b918c000000 MOV EDX,dword ptr [ECX + 0x8c]` / `008bb904 837dd400 CMP dword ptr [EBP + -0x2c],0x0` / `008bb908 7505 JNZ`; if set it calls `0x8BAB20` on the top bar with rect `(0, 0, W, label_h + 36)` (`008bba04 c745e400000000 MOV [EBP-0x1c],0`, `008bba22 0345fc ADD EAX,dword ptr [EBP + -0x4]`, `008bba67 e8b4f0ffff CALL 0x008bab20`), whose done-branch overwrites BOTH the target and the live rect (decomp: `*(undefined4 *)(param_1 + 0x68) = param_2; ... *(int *)(param_1 + 0x74) = param_5; *(undefined4 *)(param_1 + 4) = param_2; ... *(int *)(param_1 + 0x10) = param_5;`; gate asm `008bab77 83ba8c00000000 CMP dword ptr [EDX + 0x8c],0x0`). With the fix and a short line: label_h = bar - 41 = 226 (V1: `bar - 36 - 5`), so the top bar settles at 262 (not 267), larger for a longer line. The bottom bar is resized the same way by 0x8BC100 (`008bc1f6 8b4264 MOV EAX,dword ptr [EDX + 0x64]`, `008bc21b e800e9ffff CALL 0x008bab20`) to `y = reply panel top`, `h = H - y`. Both keep h > 0.

### Q4. Negative / clamped rects (CONFIRMED; refines V1 B5)

- No clamp in SetTop/SetBottom/Draw/SetExtent: they store and pass negative h as is (SetTop `008bad48 895174 MOV dword ptr [ECX + 0x74],EDX`; Draw's `JGE` at `008bb42a` treats `0 >= -22` as done).
- The skip is in the base Draw: `0040ecc8 83791000 CMP dword ptr [ECX + 0x10],0x0` / `0040eccc 0f8e68010000 JLE 0x0040ee3a` (h <= 0 -> nothing drawn, no children) and in the fill (`00479486 837d1000 CMP dword ptr [EBP + 0x10],0x0` / `0047948a 7e1c JLE` for w, `0047948c 837d1400 CMP dword ptr [EBP + 0x14],0x0` / `00479490 7e16 JLE` for h).
- Vanilla 3840x1600 (bar -22): TOP: Draw frame 1: `008bb427 CMP EAX,[EDX+0x74]` = `0 >= -22` -> done flag set, rect (0,0,3840,-22) through SetExtent (mode 1 keeps h) -> base Draw `h > 0` fails -> **not drawn** (V1's speculation resolved). BOTTOM: SetBottom clamps y to `min(H-bar, H-100fs)` = 1400 (V1 B3) and Draw slides y from 1600 to 1400 (`008bb4a8 CMP EDX,[ECX+0x6c]` = 1600 > 1400); SetExtent mode 2 then forces `h = H - y` (`008bac82..008bac89`), so the vanilla bottom bar is a **200 px band y 1400..1600** (h > 0, y+h = H <= H) under the reply panel (panel y 1400, h 200 per V1 B4), drawn at alpha 1.0 (0.5 live). V1 B5's "22 px strip or nothing" is superseded: no strip, a 200 px band.
- With the fix (3840x1600, bar 267): top rect (0,0,3840,h<=267): x,y >= 0, h > 0, `0 + 3840 <= 3840`, `0 + h <= 1600` pass; bottom y slides 1600 -> 1333 with h = 1600 - y: first frame y 1600 -> `004794a0 3b05a8429f00 CMP EAX,[0x009f42a8]` / `004794a6 7c07 JL` fails (y >= H: skipped, h = 0 anyway) and from frame 2 y < H, h > 0 -> drawn. Final: top 0..267, bottom 1333..1600 (log: target (0,1333,3840,267)); replies panel y 1333 h 200 (log `replyY=1333 panelH=200`) lies inside the bottom bar.
- The fill uses the display-mode globals `DAT_009f42a4/a8` for its x/y reject test and `[mgr+0x6C/0x6E]` for the base-Draw test; both equal 3840x1600 in the log (`W=3840 H=1600`).

### Q5. Probe windows: see (c)

---------------------------------------------------------------------------------------------------

## (c) Runtime measurements (log-only probes for a 1.0.1-probe build; only if screenshots show no bars)

Common conventions: KPM `detour` handler runs BEFORE the stolen bytes (doc 17), parameters per existing hook files (`source = "ebp"`, `"ecx"`, `"eax"`, `[esp+N]` are all used in the repo's `*_src/*.hooks.toml`). Branch-target check command, run over the whole dump (script in my scratch dir; equivalent shell for window W1: `grep -rEn "(J[A-Z]+|CALL|JMP) +0x0*8bb46[1-6]\b" decomp/*/*.asm`): every byte address in [start, end) was tested against the operand of every `Jcc/CALL/JMP/LOOP` line in all of `decomp/*/*.asm`, and against any other operand mention (jump tables, pushed addresses): result per window below. Overlap check = every live hook (120 in `live_hooks_2026-09-30g.tsv` + 198 in the live `patch_config.toml`, 66219 bytes, mtime Oct 3 00:24) with `addr-8 < end` and `addr+len+8 > start`: none for any window. Live hooks in the surrounding areas: 0x8BA900..0x8BC000 = `semi-transparent-letterbox` 0x8BAA5E (26 B), 0x8BB043 (92 B), 0x8BB108 (94 B), 0x8BB3C3 (9 B) and `k2-dialogue-letterbox-fix` 0x8BAD7C (10 B), 0x8BAF01 (10 B) (the L1 0x411AC1 hook is elsewhere); 0x40EC00..0x40EF00, 0x479400..0x479A00 and 0x7CEB00..0x7CEC80: no live hook.

| id | window | instructions (address bytes insn) | original_bytes | len | boundary / rel branch / branch target inside | overlap +-8 | parameter source |
|---|---|---|---|---|---|---|---|
| W1 (i, top) | 0x008BB461 | `008bb461 8b45cc MOV EAX,dword ptr [EBP + -0x34]`; `008bb464 8b4810 MOV ECX,dword ptr [EAX + 0x10]` | `8B 45 CC 8B 48 10` | 6 | whole instructions (next insn `008bb467 890db409a300`); no rel branch; whole-dump grep for `8bb461..8bb466`: no refs; no other mention | none (nearest `0x8BB3C3`, gap 149 B) | `ebp` (pointer): this = `[ebp-0x34]`; rect just passed to SetExtent = `[ebp-0x18]` x, `-0x14` y, `-0x10` w, `-0x0C` h; control rect after SetExtent = `[this+4..+0x10]`; `[this+0x88]` mode, `[this+0x8C]` done, `[this+0x50]` alpha, `[this+0x74]` target h |
| W2 (i, bottom) | 0x008BB52B | `008bb52b 8b4dcc MOV ECX,dword ptr [EBP + -0x34]`; `008bb52e 8b5108 MOV EDX,dword ptr [ECX + 0x8]` | `8B 4D CC 8B 51 08` | 6 | whole (next `008bb531 8915b809a300`); no rel branch; grep `8bb52b..8bb530`: none (the merge point 0x8BB51A is outside) | none (gap 351 B) | `ebp`: this `[ebp-0x34]`; rect passed `[ebp-0x28]` x, `-0x24` y, `-0x20` w, `-0x1C` h |
| W3 (ii, pre-draw predicate) | 0x008BB538 | `008bb538 d94508 FLD float ptr [EBP + 0x8]`; `008bb53b d91c24 FSTP float ptr [ESP]`; `008bb53e 8b4dcc MOV ECX,dword ptr [EBP + -0x34]` | `D9 45 08 D9 1C 24 8B 4D CC` | 9 | whole (next `008bb541 e84a37b5ff CALL`, not stolen); no rel branch; grep `8bb538..8bb540`: none; reached by all modes (3 jumps land on 0x8BB537, one byte earlier, outside the window) | none (gap 364 B) | `ebp`: this `[ebp-0x34]`, dt `[ebp+8]`. Alt W3alt 0x8BB537, 10 B `51 D9 45 08 D9 1C 24 8B 4D CC` (adds `PUSH ECX`): grep finds 3 refs to its FIRST byte (`JNZ` at 0x8BB3D6, `JMP` at 0x8BB46D, `JNZ` at 0x8BB47C) = allowed (start lands on our jump), none inside; prefer W3 |
| W4 (ii-b, was the fill issued) | 0x0040ED9F | `0040ed9f 83c41c ADD ESP,0x1c`; `0040eda2 85c0 TEST EAX,EAX` | `83 C4 1C 85 C0` | 5 | whole (next `0040eda4 0f848b000000 JZ 0x0040ee35`, not stolen); no rel branch; grep `40ed9f..40eda3`: none | none (no live hook in 0x40EC00..0x40EF00) | `ebp`: this `[ebp-0x38]`, rect `[ebp-0x10]` x `-0xC` y `-0x8` w `-0x4` h; plus `eax` (the fill result, do NOT list eax in `exclude_from_restore` so `TEST EAX,EAX` still sees the original). HOT PATH: runs for every panel each frame; handler must test `*(void**)this == 0x9A7CBC` first and cap the log |
| W5 (iii, 0x7CEB80 arg) | 0x007CEB95 | `007ceb95 837d0801 CMP dword ptr [EBP + 0x8],0x1`; `007ceb99 0f94c2 SETZ DL` | `83 7D 08 01 0F 94 C2` | 7 | whole (next `007ceb9c 8815bc09a300`); no rel branch; grep `7ceb95..7ceb9b`: none | none (nearest live: `level-up-override` 0x7C9F3B, 19 KB away) | `ebp`: arg = `[ebp+8]` (0/1), this = `[ebp-0x24]`. Frame already built (`007ceb81 8bec MOV EBP,ESP`) |

All five were checked by script against the `.asm` bytes (my scratch script, output identical to the table); none is a RET-containing window. Notes: W3 steals an x87 pair (`FLD`/`FSTP`): the x87 stack is empty at that point (the Draw epilogue pushes dt with this pair), the handler must leave x87 balanced. W1/W2 stolen `MOV EAX/ECX` are dead-writes (EAX/ECX reloaded right after); handlers should not exclude any register from restore.

What each logs and the expected signal at 3840x1600, h6 mode, live semi-transparent patch:
- W1: per frame while the top bar slides: mode 1, rect h 8, 16, ... (60 fps) up to 267, control rect `[this+4..+0x10]` equal to it, then one record with done = 1 (`[this+0x8C]` = 1 is written at 0x8BB446 BEFORE this hook on the last frame) and h 267, alpha `0x3F000000`. Expected 34 records at 60 fps. No W1 record at all = Draw not called on the top bar (panel not in the manager list). Records with h stuck at 0 = step 0 (fps >= 535) or stale done flag.
- W2: y 1600 -> 1333 in -8 steps, control h = 1600 - y after SetExtent (`[this+0x10]`).
- W3: log `*(void**)this == 0x9A7CBC`, `[this+0x48]` bits 2 and 6, rect `[this+4..+0x10]`, alpha `[this+0x50]`, colour `[this+0x54..+0x5C]`, `[[this+0x1C]+0x6C]` / `+0x6E` (W, H shorts), and the exact base-Draw predicate (bit 6, x >= 0, y >= 0, w > 0, h > 0, x+w <= W, y+h <= H) as "WOULD-DRAW" or the first failing term; also `DAT_00a309b4/b8/bc` (absolute addresses). Expected: WOULD-DRAW with h 262..267 (after SetReply) and alpha 0.5, colour (0,0,0).
- W4: `eax` = 1 for the bars once W3 says WOULD-DRAW; 0 would point at the fill's `DAT_00a32a30` switch or display-size reject. This answers "is a fill issued" directly.
- W5: arg 1 once per conversation start after the SETTOP lines, arg 0 at the end.

---------------------------------------------------------------------------------------------------

## (d) Open questions
1. What the user actually sees: the code says 50% black (alpha byte 128) over the scene in the top 262-267 px and bottom 267 px, sliding in over about 0.5 s. A screenshot (or W4/W3 logs) decides. With `mode=vanilla`/`enabled=0` A/B the vanilla result should be: no top bar, a 200 px bottom band.
2. Design choice, not a bug: if the user wants stronger bars, the live patch's 0.5 alpha (hooks 0x8BAA5E / 0x8BB043) would have to be 1.0 (or the patch disabled).
3. The post-process composite quads limited to `[DAT_00a309b4, DAT_00a309b8]` (Q2 side note): what the framebuffer shows in the bar band outside the quad (SPECULATIVE; could make the band look black or stale independent of the panel alpha). W3's `DAT_00a309b4/b8/bc` log plus a screenshot would show it.
4. Frame rate ceiling for the slide (step 0 at >= 535 fps); not measured.
5. Is `0x9846F0` really run at start-up (static initializer; colour assumed (0,0,0) from `.bss` zero anyway). W3 logs the colour.
6. Conversation-type labels (0 cinematic / 2 other) are not shown by code; only "type 1 => Computer panel, no bars" is.

---------------------------------------------------------------------------------------------------

## (e) Verdict

- **Q1 (Draw modes / slide / SetExtent): EXPLAINED.** Modes 1/2/3 = SetTop/SetBottom/SetFull (`008bad7c`, `008baf01`, `008baf3f`); slide is per frame, `trunc(2 * (target - origin) * dt)` (`008bb56e dc35087f9800 FDIV double ptr [0x00987f08]`, 0.5), ~0.5 s (34 frames at 60 fps), done flag `[+0x8C]` set at `008bb446` / `008bb4d9`, reset at every conversation end (`007c6dff`...); vtable +4 = 0x8BAC30 -> 0x40E980 stores `+4..+0x10`.
- **Q2 (what draws the bar, colour, alpha): EXPLAINED.** `008bb541 CALL 0x0040ec90` -> `0040ed9a CALL 0x00479460`: a black untextured quad, alpha `[this+0x50]` = 1.0 vanilla or 0.5 live (`MOV dword ptr [EDX+0x50],0x3F000000` from the semi-transparent hooks); alpha 0 / texture-less invisibility is not possible for mode 1/2 from the code shown. Colour (0,0,0) rests on a `.bss`/static-init argument (SPECULATIVE detail).
- **Q3 (is the panel shown, which conversations): EXPLAINED; the "cinematic only" hypothesis is REFUTED.** `007bfbee 3b4240 CMP EAX,[EDX+0x40]` selects every conversation whose `ConversationType` != 1 (`006c5762 837dcc01 CMP dword ptr [EBP + -0x34],0x1`); the 12 logged pairs ran SetTop/SetBottom and `00410530` adds the panels without touching flag bits 2/6.
- **Q4 (negative / clamped rects): EXPLAINED.** Skip rule lives in the base Draw (`0040eccc 0f8e68010000 JLE 0x0040ee3a`, h <= 0) and the fill (`0047948a 7e1c JLE` / `00479490 7e16 JLE`, w/h < 1; `00479495`..`004794a6` x/y >= display size). Vanilla: top not drawn, bottom a 200 px band (y 1400..1600). Fixed: 267 passes every test; nothing clamps it.
- **Q5 (probes): windows W1 0x8BB461 (6 B), W2 0x8BB52B (6 B), W3 0x8BB538 (9 B), W4 0x40ED9F (5 B), W5 0x7CEB95 (7 B); all CONTRACT-clean, no live overlap.**
- **Overall: UNCLEAR whether the user would perceive them; no static reason for h 267 to be skipped.**

Most likely reason bars may be invisible: they are not skipped (all base-Draw/fill gates pass for 267 px) but the live `semi-transparent-letterbox` makes them 50% black (alpha byte 128) quads over a still-visible scene, so they read as a mild dimming of the top and bottom ~267 px rather than opaque bars (to be confirmed by a screenshot or the W3/W4 logs; a stalled slide at >= 535 fps or a stale done flag are the unproven alternatives).

---------------------------------------------------------------------------------------------------

## (f) Unnamed functions (FUN_xxxxxxxx read or guessed) and whether a name would have changed the verdict

| FUN | inferred role | name would change verdict? |
|---|---|---|
| FUN_008bb260 | letterbox Draw (vtable +0x34) | No (anchor was the vtable read from the exe) |
| FUN_008bac30 | letterbox SetExtent (vtable +4) | Yes: knowing "mode 2 recomputes h = H - y" earlier would have avoided V1 B5's wrong "22 px strip" |
| FUN_0040e980 | base panel SetExtent | No |
| FUN_0040ec90 | base panel Draw (name_map: CSWGuiPanel::Draw, medium) | Yes: it holds the h > 0 / bounds skip rule that answers Q4 |
| FUN_00479460 | GUI fill rect (viewport + quad, colour + alpha, magic colours -1/-2) | Yes: confirmed the bar is a plain colour quad |
| FUN_004792c0 / FUN_004793f0 / FUN_004799c0 | GUI viewport/matrix push / pop / viewport pop | No |
| FUN_00410020 | panel screen-rect getter (640x480 centering flags 0x10/0x20, cleared for letterbox) | No |
| FUN_00517980 | colour-triple equality | No |
| FUN_008bab20 | letterbox resize-to-text (target grow or direct set) | Slightly: explains the bar settling at 262 after SetReply |
| FUN_00410530 / FUN_00410670 / FUN_00410800 / FUN_00410890 / FUN_00410830 | manager add / remove / contains / index / index-in-second-list; 0x410800 carries a wrong Ghidra FID label `CMFCVisualManagerBitmapCache::IsCached` | Yes for 0x410800: the wrong label hides that it is a "panel already shown" test |
| FUN_007bfb10 / FUN_007bfa50 / FUN_006c53f0 / FUN_007c6a20 / FUN_007ceb80 / FUN_00545760 | show dialog panels / select dialog panel / dialog-open handler (string anchor) / close dialog / set global dialog state / start dialog | Yes: names like `IsFadingInDialog` (0x7CB750 = "current panel == cinematic panel") and `IsCameraDialog` (0x7CB6A0 = SetFade forwarder) in `name_map.tsv` are misleading; Q3 needed the content anchors |
| FUN_008bb0e0 / FUN_007cb6a0 / FUN_007cb350 / FUN_007cb3f0 | SetFade / its forwarder (full panel only) / ResetFade callers | No |
| FUN_00437bd0 / FUN_004426f0 / FUN_004421f0 / FUN_00442c90 / FUN_00443190 / FUN_00445310 | post-process composite passes limited to the bar band (side note only) | No for Q1-Q5 |
| FUN_009846f0 | static init zeroing `DAT_00a7fd34/38/3c` (bar colour) | No |
| FUN_0073f750 | GetGuiInGame | No |

## Opus verification (2026-10-03)
Checked against `decomp/*.asm`:
- Vtable +4 `0x8BAC30`: mode 1 forces y=0, mode 2 sets `h = (short)[[this+0x1C]+0x6E] - y` (manager H - y), then `0x40E980`. **Confirmed.** Consequence (new, corrects V1 B5): vanilla at 3840x1600 has bottom target y = min(H - (-22), H - 100*fs) = min(1622, 1400) = 1400, so h = 200: **vanilla draws a 200 px bottom band and no top bar**. With L1: top 267 + bottom 267 (y 1333).
- Base Draw skip `0x40ECC8 CMP [ECX+0x10],0 / 0x40ECCC JLE 0x40EE3A` (h <= 0 -> skip). **Confirmed.**
- Windows: W1 `0x8BB461 8B 45 CC | 8B 48 10` (6 B), W2 `0x8BB52B 8B 4D CC | 8B 51 08` (6 B), W3 `0x8BB538 D9 45 08 | D9 1C 24 | 8B 4D CC` (9 B, before CALL 0x40EC90 at 0x8BB541), W4 `0x40ED9F 83 C4 1C | 85 C0` (5 B; JZ follows at 0x40EDA4 reads flags from the stolen TEST, which the KPM wrapper executes after the handler, so flags are correct), W5 `0x7CEB95 83 7D 08 01 | 0F 94 C2` (7 B). Bytes and boundaries match. Whole-dump grep for branch/call targets inside all five ranges: 0 hits.
- Side note: `0x8BB467` stores the top-bar h to global `0x00A309B4` and `0x8BB531` the bottom-bar y to `0x00A309B8` (consumers not traced; possibly the 3D view / subtitle placement). Not needed for T1.
- **Gate: PASS.** A2 checklist consequence: the enabled=0 run is NOT "no bars": it shows a 200 px bottom band and no top band. The visible A/B difference is the TOP band (0 vs 267) and a 67 px taller bottom band.
