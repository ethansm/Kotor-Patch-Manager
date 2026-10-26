# R7 FBO / flush: per-frame GL sync and copy cost (static, Steam exe)

Scope: static only. Dumps: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` (`decomp/<bb>/<addr>.asm|.c`, `imports.tsv`, `callgraph.tsv`). "Init" = vanilla value read from `swkotor2_vanilla_6a522e71.exe`. Live values were not read: the live ini was not located (backups show `Frame Buffer=1`, `V-Sync=1`, `Shadows=1`, `Soft Shadows` 0 or 1 depending on backup). SPEC = not proven at runtime. Callgraph absence is not proof: Scene methods are vtable-only.

## 1. Summary
1. The default path (driver exposes ARB_fragment_program and ARB_framebuffer_object) issues 0 glFlush per frame. Every glFlush site is in a dead branch, a fallback branch, or a conversation-only pass.
2. Per-frame GPU copy work is glCopyTexSubImage2D at full client size (DAT_009F42A4 x DAT_009F42A8, not 256x256): 1 per Scene render in a room scene (0x447606), plus 2 when a shadow light is in range (0x44808C, 0x44815C).
3. glReadPixels 0x4289D0 (save thumbnail) and 0x428D70 (F-key screenshot) are not per-frame. glFinish 0x4265E0 runs only while texture uploads are pending.
4. No glFlush precedes SwapBuffers: 0x408434 is preceded only by `CALL 0x436B90` at 0x40842B, which has no flush. Nothing redundant to NOP.
5. Cheapest levers need no DLL: ini `Soft Shadows=0`, then `Frame Buffer=0` (section 5). DLL glFlush NOPs are LOW value on the default path.

## 2. Gate globals and capability tests

| global / fn | init | writers (asm) | meaning |
|---|---|---|---|
| DAT_009F4E74 | 1 | 0x478D20 (1), 0x478D30 (0), via ini reader 0x7BC330 | `Frame Buffer` ini key |
| DAT_009F5D30 | 1 | 0x478CA0 (1), 0x478CB0 (0), via ini 0x7BC230 | Shadows |
| DAT_009F5DA8 | 1 | 0x478CF0 (1, ini 0x7BC260); 0x478D10 (0); 0x479200 (0, from ini reader 0x7B88B0) | Soft Shadows (0x479200 is "ATIUnlockSoftShadowsForHotFix", vendor-dependent: SPEC) |
| DAT_009F48FA | 1 | none | constant gate, shadow path |
| DAT_009F48FB | 1 | none | constant gate, dispatcher |
| DAT_009F4E5C / 4E60 / 4E68 / 4E6C / 4E70 / 4E64 | 1 | none (readers only) | constant sub-pass gates (SPEC: no writer in dump; a block copy would hide one) |
| DAT_00A1DD29 | 0 | 1 at 0x43C7D9, 0x43D7AE, 0x443C4B; 0 at 0x435BC8, 0x435C77 | GL/FBO init succeeded |
| DAT_00A1DD2A | 0 | 1 at 0x7D5D02 (GUI handler); store at 0x478E6F from 0x478E60 (`Global::AurEnableSpeedBlur`, script `ExecuteCommandSetSpeedBlurEffect`) | speed-blur flag |
| DAT_00A1DD2B | 0 | 1 at 0x478EA3, 0 at 0x478EB3 | gates the only copy in 0x447A40 and 0x438320 (default 0 = no copy) |
| DAT_00A1DD48 | 0 | 1 at 0x478EC3 (`AurEnableScanNoise`, via conversation `EnableVideoEffect` 0x79E550), 0 at 0x478ED3 | scan-noise (conversation only) |
| DAT_00A1DD49 / DAT_00A1DD94 | 0 | none | dead gates (no writer) |
| DAT_00A1DD4A / DAT_00A1DD54 | 0 | 1 at 0x442063 (clairvoyance); Fury level store at 0x44214E / 0x4421DE | conversation video effects |
| DAT_00A732BC | 0 | only 0 at 0x495778 | mask 0x3F vs 0x1F; effectively always 0x1F |
| DAT_009F5EF4 (gamma) | 1.0f | 0x478870 (gamma ramp setter) | copy in 0x436B90 only when != 1.0f |
| DAT_009F60E4 | 0x40000000 | none | ARB_fragment_program bit (OR-ed at 0x431BF3) |
| DAT_009F60BC | 0x100000 | none | ATI_text or ARB_fragment_program bit |
| DAT_009F60B4 | 0x400 | none | GL_ARB_framebuffer_object bit (0x4317D0 line 118) |
| DAT_009F60C0 | 0x2000 | none | GL_ARB_vertex_program bit |
| DAT_00A32DF8 | 0 | 0x4317D0 ORs extension bits from `FUN_00484B20(ext)` | GL capability mask |
| DAT_009F60F0 / DAT_009F60F4 | 2 / 1 | none | shader level (>=2) / FBO test enable |
| DAT_009F6124 | -1 | unconditional `MOV [0x9F6124],0` at 0x484955 | FUN_00484910 always returns 0 |

| fn | returns | used by |
|---|---|---|
| FUN_00484910 | 0 always (0x484955 store precedes the read at 0x48495F) | 0x4373A0 else-block dead (JNZ 0x4373FF never taken, 0x4373E3); 0x438530 else dead |
| FUN_00484970 (cap#2) | mask has (0x400 \| 0x100000 \| 0x2000) and FUN_00484550 (VP test) | gate for 0x437BD0 (from 0x4373F2) and 0x438320 (from 0x43856D) |
| FUN_00484CA0 (cap#3) | `(0x40000000 & mask) == 0x40000000`, i.e. ARB_fragment_program | 1 -> 0x447300 (path B2); 0 -> 0x437BD0 big branch with 9 glFlush |
| FUN_004848B0 | 1 if DAT_009F60F4 and mask has 0x100000 and 0x400 (SPEC true with FBO + frag prog) | 0 -> fallback with glFlush and 4x wglMakeCurrent (0x440CB0, 0x441610) |
| FUN_00484BB0 | glGetIntegerv(GL_STENCIL_BITS 0xD57), cached | stencil shadow volumes (SPEC true) |
| FUN_004844C0 | DAT_009F60F0 >= 2 and FUN_004845E0 (SPEC true) | soft-shadow copy gate |
| FUN_00484850 | 0 always (cache init 0) | only the OR-branch in 0x46CF20 |

## 3. Per-address table

| addr | role | per-frame? | gate | evidence |
|---|---|---|---|---|
| 0x407920 (WinMain) swap block | main-loop swap | YES | movie `0x740160`==0 (0x4083F2-0x4083F9); `[0xA1B4D4]`==0 (0x4083FB); `0x436F70` != 0 at 0x40841D (returns 0 only for zero-width window) | `0x40842B: CALL 0x00436B90`; `0x408434: CALL dword ptr [0x0098601C]` (SwapBuffers); nothing between |
| 0x409ED0 pump, `0x409FC0 CALL 0x436B90` / `0x409FC9` SwapBuffers | save/stall progress pump | NO in normal play (callers are SaveGame 0x52FB1C, StallEventSaveGame 0x5347E0, 0x40DC24: SPEC on other callers) | DAT_00A1B4D4 mode | callgraph callers of 0x409ED0 |
| 0x436B90 gamma copy + quad | post-process gamma | call YES; copy only if gamma != 1.0 | `0x436C06 FLD [0x9F5EF4]`, `0x436C0C FCOMP [0x986B50]` (double 1.0), `0x436C19 JMP 0x436F65` skips when equal | `decomp/43/00436b90.c:31`; first run `0x436C74 glCopyTexImage2D`, steady state `0x436D09 glCopyTexSubImage2D(W x H)`; **no glFlush** |
| 0x46C2B0 Scene::Render | scene render (vtable, caller unresolved) | YES per scene (SPEC: scene count per frame UNRESOLVED, W9) | `DAT_009F5D7C` (init 1) and `0x436F70` (asm 0x46C31C-0x46C32F) | Scene vtable 0x98BB44 (ctor 0x464500, `MOV [EDX],0x98BB44` at 0x464566); `0x46C49D CALL vt+0x124` (0x46A8A0, no GL copy/flush reachable); `0x46C4FD CALL vt+0xB8` (0x46C5C0) taken when `[scene+0xAC]!=0` or `[scene+0xD8]!=0` (room count) |
| 0x46C5C0 RenderSinglePass (vt+0xB8) | dispatcher for 0x46CF20 / 0x4373A0 / 0x438530 | YES (same frame as parent) | `[scene+0xE0]` current room != 0 (`0x46FFB0` SetCurrentRoom_2) | `0x46CA27 CALL vt+0xBC`; `0x46CBA1 CALL 0x4373A0` (mask 0x1F); `0x46CC03 CALL 0x438530` |
| 0x46CF20 shadow (vt+0xBC) | stencil shadow + soft-shadow copies | YES when a shadow light is in range (SPEC how often) | `[scene+0xE0]` and `DAT_009F5D30` (0x46CA02); inside: stencil bits (`FUN_00484BB0`, decomp line 14), light object + distance (line 24-25) | copies: C1 `0x46D025 CALL 0x440CB0` (guard `CMP [0x9F5DA8]` 0x46CFF3, `MOVZX [0x9F48FA]` 0x46D01A); C2 `0x46D175 CALL 0x441610` (guard 0x46D143, 0x46D16A) |
| 0x440CB0 -> 0x440C00 -> 0x448050 (C1 copy) | full-screen copy to DAT_00A1DDD0 + glClear | YES (inside 0x46CF20 guard) | `FUN_004848B0`==1 (SPEC) then `FUN_00484CA0`==1 | copy `0x44808C glCopyTexSubImage2D(W x H)`; `glClear(0x4000)`; no flush on this path. Fallback: `0x440D1D glFlush` (4848B0==0), `0x440C70 glFlush` (cap#3==0) |
| 0x441610 -> 0x440D50 -> 0x4480E0 (C2 copy) | full-screen copy to DAT_00A1DD44 | YES (inside 0x46CF20 guard) | `FUN_004848B0`==1 (SPEC), then `FUN_00484CA0`==1 | copy `0x44815C glCopyTexSubImage2D(W x H)`; no flush. Fallback (4848B0==0): `0x44168B glFlush` plus 4 wglMakeCurrent (`0x44169E, 0x4418B5, 0x441B00, 0x441C02`) and big quads; `0x440E00`/`0x441395` flushes sit in the cap#3==0 branch of 0x440D50 |
| 0x4373A0 dispatcher (mask 0x1F) | frame-buffer effect dispatch | YES (via C-row 4) | `DAT_009F4E74` (Frame Buffer), `DAT_00A1DD29`, `DAT_009F48FB`; `FUN_00484910`==0 (always) | `0x4373E3 JNZ 0x4373FF` never taken; live fallthrough `0x4373E5 CALL 0x484970`, `0x4373F2 CALL 0x437BD0`. Else-block (flushes at 0x43756D..0x437AE7, callees 0x4421F0, 0x442C90, 0x440600, 0x439A90, 0x439380, 0x438B40) is **DEAD** |
| 0x437BD0 path B | effect pass selector | YES when 0x4373A0 gate passes | `FUN_00484CA0` at `0x437BD3` / `0x437BDD JZ 0x437BF0` | cap#3==1 -> `0x437BE3 CALL 0x447300` (default). cap#3==0 -> big branch: 9 glFlush at 0x437D5E, 0x437E00, 0x437ECB, 0x437F48, 0x437FCF, 0x43806D, 0x43810C, 0x438189, 0x438237 (fallback only) |
| 0x447300 path B2 | copy passes, cap#3==1 | YES | per-pass flags (below) | copy sites: `0x447440` (4E6C & DD2A, speed blur), `0x4474CD` (DD2A else), `0x447590` (DD94: dead), **`0x447606` (4E5C & mask bit 2: default, always-on, 1 copy, then `CALL 0x4466C0` at 0x44762B)**, `0x447688` (4E68 & 0x20: dead since A732BC=0), `0x447740` (4E60 & DD48 & 4: conversation), `0x4477FA` (4E70 & DD49: dead), `0x44787B` (DD4A clairvoyance), `0x447931` (DD54 Fury). **No glFlush in 0x447300** except `CALL 0x445310` (0x4478D6 clairvoyance, 0x44798B Fury), where `0x445821 glFlush` sits |
| 0x438530 | copy path under 0x46C5C0 | YES by parent, copy NO by default | outer: 4E74 & DD29 & 9F48FB; `FUN_00484910`==0; `FUN_00484970`!=0 | `0x43856D CALL 0x438320` -> cap#3==1 -> `0x438333 CALL 0x447A40`; copy `0x447B6D` gated by `DAT_009F4E64 & DAT_00A1DD2B & (mask&8)`: **0 copies by default** |
| 0x4265E0 glFinish | texture upload | NO in steady state | reached from Scene::Render via `0x46C3AD CALL 0x427F30` -> `0x427810` -> `0x4260E0` -> `0x4265E0`; 0x427F30 needs `DAT_00A1BB9D==0` and `DAT_00A1BC30!=0` (pending uploads) | `0x4265F5 CALL [0x009862C4]` (glFinish); R3 gate on mask bit 0x100000 (ON) |
| 0x4289D0 glReadPixels | CaptureThumbnail (save) | NO | `0x401B90` <- `0x52F610` SaveGame only | `0x428BE9 CALL [glReadPixels]` |
| 0x428D70 glReadPixels | F-key screenshot | NO | `0x781BE0` MainLoop `0x783A7E CALL 0x428D70`; gate `[this+0x154]!=0 && [0xA106BC]!=0` (request flag) | `0x428ED1 CALL [glReadPixels]` |
| glCopyTexImage2D sites: 0x43BBC3, 0x43BC14, 0x43BF2C, 0x43BF7D, 0x43C325, 0x43D2A0, 0x443789, 0x447C7D, 0x447CDB | texture allocation | NO (init) | CAurInternalGL::CreateWindow `0x476BE0` -> `0x43BB50` (`0x47722A`) and `0x43C290` (`0x47722F`) -> `0x443730` (`0x43C2C7`) | callgraph callers |

## 4. Answers

Q1 (per-frame reachable, gates). From 0x407920: 0x436B90 (direct, gated by gamma DAT_009F5EF4 != 1.0). From 0x46C2B0 (via vtable): 0x46CF20 (gates DAT_009F5D30, [scene+0xE0], stencil bits), 0x4373A0 (gates DAT_009F4E74, DAT_00A1DD29, DAT_009F48FB, FUN_00484970, FUN_00484CA0), 0x438530 (same family, copy gated by DAT_00A1DD2B), 0x4265E0 (texture-pending only). Frame-buffer and shadow paths are reachable from Scene::Render only; cg.py path from 0x407920 does not reach them (BFS over CALL+TAILJMP).

Q2 (counts per Scene render, default path, SPEC on caps):
- glFlush: 0 per frame.
- glCopyTexSubImage2D: 1 always in a room scene (0x447606, full W x H). +2 when a shadow light is in range (0x44808C, 0x44815C, both full W x H). 0x436B90 adds 0 at gamma 1.0.
- Fallback (no ARB_fragment_program): 0x437BD0 big branch adds up to 9 glFlush plus several copies per frame. Fallback (no FBO): 0x441610 adds 4 wglMakeCurrent and a flush per call.

Q3. glReadPixels 0x4289D0 and 0x428D70 are not per-frame (save thumbnail, F-key screenshot). Confirmed by the callers above.

Q4. No glFlush sits before SwapBuffers. 0x408434 is preceded by `CALL 0x436B90` (0x40842B), which contains no flush. The same holds for the pump at 0x409FC9. SwapBuffers implicitly flushes (WGL spec, SPEC for Wine), so an explicit flush before it would be redundant, but none exists. Live glFlush NOP sites (default path: none) are listed in section 5 for completeness.

## 5. Candidate levers

| # | lever | effect per frame | value | risk | how |
|---|---|---|---|---|---|
| L1 | ini `Soft Shadows=0` (DAT_009F5DA8=0) | removes C1 and C2: 2 full-screen copies + 2 glClear(COLOR) + quads, only when a shadow light is in range | MED (SPEC: fraction of frames lit) | MED: visual change to soft shadows; stencil shadows unchanged (DAT_009F5DA8 guards only the two calls) | no DLL; perf-lab A/B |
| L2 | ini `Frame Buffer=0` (DAT_009F4E74=0) | removes the 0x4373A0 gate: C3 (0x447606) and the 0x438530 family (already 0 copies). Shadows unchanged | LOW-MED (1 copy/frame) | MED-HIGH: disables all frame-buffer effect composites; the output of the bit-2 pass 0x4466C0 is unknown (SPEC) | no DLL |
| L3 | DLL NOP of glFlush CALLs | 0 live sites on the default path, so ~0 gain | LOW | LOW (flush only submits; SwapBuffers flushes anyway) | 6-byte `FF 15 5C 63 98 00` (CALL dword ptr [0x0098635C]) at 0x440D1D, 0x440C70, 0x440E00, 0x441395, 0x44168B, 0x437D5E, 0x437E00, 0x437ECB, 0x437F48, 0x437FCF, 0x43806D, 0x43810C, 0x438189, 0x438237; conversation-only 0x445821 |
| L4 | DLL hook of the 0x441610 fallback (4x wglMakeCurrent + flush) | live only if GL_ARB_framebuffer_object is absent | HIGH if live, 0 otherwise | MED (changes render-target handling) | probe first (P2) |
| L5 | keep gamma at 1.0, or skip the copy in 0x436B90 | removes 1 full-screen copy + gamma quad per frame when gamma != 1.0 | LOW-MED | LOW-MED (gamma effect lost) | DLL: at 0x436C17 replace `JP 0x436C1E` (`7A 05`) with `EB 00`, which lands on the existing `JMP 0x436F65` at 0x436C19, so copy and quad are always skipped |

Rough cost (SPEC, arithmetic only): RGBA8 copy (internal format 0x8058) at 3440x1440 is about 19.8 MB read + write per copy. 3 copies per frame is about 120 MB of GPU traffic per frame, roughly 7 GB/s at 60 fps. Driver-dependent; not measured.

## 6. Unproven items and probes

- SPEC: the live driver exposes ARB_fragment_program, ARB_framebuffer_object, ARB_vertex_program, and stencil bits > 0. These select between the default path and fallbacks. Probe P2: log `glGetString(GL_EXTENSIONS)` and `glGetIntegerv(GL_STENCIL_BITS)` once.
- SPEC: scene count per frame (W9 UNRESOLVED); the scene class of Scene::Render's caller is not in the callgraph (vtable slot 0x18 of vtable 0x98BB44).
- SPEC: `[scene+0xAC]` and `[scene+0xD8]` values in a typical area (room count > 0 selects 0x46C5C0). Read via a probe, not static.
- SPEC: DAT_009F5DA8 value on the live install (0x479200 can zero it on the vendor path).
- SPEC: no writer in the dump for DAT_009F4E5C/60/64/68/6C/70, DAT_009F48FA/FB, DAT_009F60E4/BC/B4/C0. Treated as constants (a block copy could still write them).
- Not analysed: ReShade. A backup `reshade_20260924/` exists. If ReShade is loaded it adds its own per-frame copies outside this exe; the DLL cannot see them.
- Probe P1 (read-only IAT counters, 1 line per 600 frames): glFlush (IAT 0x0098635C), glCopyTexSubImage2D (0x00986354), wglMakeCurrent (0x00986344), SwapBuffers (0x0098601C). Confirms or refutes Q2 counts directly.
- Probe P2: gate globals at first frame: DAT_00A32DF8, DAT_009F60F4, DAT_009F613C, DAT_009F5D30, DAT_009F5DA8, DAT_009F4E74, DAT_00A1DD29, DAT_00A1DD2B, DAT_009F5EF4, and `[scene+0xE0]`, `[scene+0xD8]`, `[scene+0xAC]`.
