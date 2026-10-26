# R10: emitters, lights, frame pacing (Steam swkotor2.exe 6a522e71, static only)

Scope: read-only static recon from `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` (decomp, asm, imports, globals, PE image). No live run. Addresses are Steam. Unproven items are marked SPECULATIVE.

## 5-line summary
1. Pacing: the main loop 0x407920 has no Sleep, WaitMessage or focus check on any path. The only Sleep (0x408479) is behind `[0xA1B704]`, which is 0 at load and has no writer, so it is dormant. Frame pacing is SwapBuffers only (0x408434), and the QPC limiter (0x40847F) is dormant because `[0xA1B710]` is 0.0.
2. Minimised window: WM_SIZE sets `[0xA1B4D4]=2` (consumed in-frame by 0x409750), `GetClientRect` width 0 makes 0x436F70 return 0, so SwapBuffers (0x408422..0x408429) and the scene draw (0x46C325) are skipped. The loop keeps spinning at full CPU with no sleep. This is the strongest idle-hardware lever.
3. Interval: 0x4317D0 only fetches `wglSwapIntervalEXT` into `[0xA32AA8]` (0x4329CE). The call is `0x478D70(V-Sync != 0)`, fed from ini key V-Sync via 0x7B88B0 (PUSH "V-Sync" 0x7BAB92, CALL 0x7BC520 0x7BAC5E), so V-Sync=1 gives interval 1. A second call site, 0x4778B5 in 0x476BE0, re-applies the cached value without the extension gate.
4. Emitters: per frame, only frustum-visible emitters are simulated and drawn (list built in 0x499EB0 at CALL 0x494200, 0x49AB43). Off-screen emitters get no per-frame simulation. A one-time 100x preroll per scene (0x46C190) is not frustum-gated. No per-emitter cap or global birthrate was found; birth count is `(int)(f[+0xD0]*f[+0x100]+0.5)`. A distance cut fits at 0x49AB43 with no extra state.
5. Lights: 0x508CA0 walks all lights, filters, qsorts and selects 3. It has no distance float (only 0.0 and 1.0 constants). Its per-frame caller is not proven (vtable-only). Light candidates per call are the only unknown.

## Table: anchors

Address conventions: `0xADDR: insn` is from `decomp/<bb>/<addr>.asm`. "vt" = vtable-only, no direct callgraph edge.

| addr | role | per-frame? | gate | evidence |
|---|---|---|---|---|
| 0x407920 | main loop; `while([0xA13C5C]==0)` head at 0x407E30 | yes | quit flag | 0x407E3D `CALL [0x986590]` (SteamAPI_RunCallbacks); 0x407E65 `CALL [0x986448]` PeekMessageA(PM_NOREMOVE); 0x407E79/0x407E96 GetMessage/Dispatch only when a message is pending |
| 0x407920 | swap gate: movie flag, then `[0xA1B4D4]` | yes | `IsMoviePlaying==0 && [0xA1B4D4]==0` | 0x408402 `JNZ 0x00408455`; 0x408404 `MOV [0x00A1B748],1` |
| 0x407920 | client-width gate, then swap | yes | `0x436F70()==1` | 0x40841D `CALL 0x00436F70`; 0x408425 `TEST EDX,EDX`; 0x408429 `JMP 0x0040843A` (skips 0x40842B `CALL 0x436B90` and the swap) |
| 0x407920 | SwapBuffers (IAT `[0x0098601C]`) | yes, when gate passes | above | 0x408434 `CALL dword ptr [0x0098601C]` (import SwapBuffers, GDI32) |
| 0x407920 | post-frame: `FUN_00741090`, then Sleep(0) block | yes | `[0xA1B704]!=0` | 0x408469 `CALL 0x00741090`; 0x40846E `CMP dword ptr [0x00A1B704],0x0`; 0x408475 `JZ 0x0040847F`; 0x408477 `PUSH 0x0`; 0x408479 `CALL [0x009860D4]` (Sleep) |
| 0x407920 | limiter: target ms = 1000/fps | yes | `[0xA1B710] > 0.0` | 0x40847F `FLD [0x00A1B710]`; 0x408485 `FCOMP [0x00986B38]` (0.0); 0x40849C `FDIVR [0x00986B48]` (1000.0) |
| 0x407920 | limiter spin (QPC busy-wait, no yield) | yes when active | same | 0x40852A..0x4085C5; QPC call 0x40854B `CALL 0x00736220`; 0x4085C5 `JMP 0x0040852A`; exit 0x4085CA; loop back 0x4085E3 `JMP 0x00407E30` |
| 0x436F70 | client-width test (0 when minimised) | yes | none | `GetClientRect`; returns 0 iff width==0 (decomp 43/00436f70.c) |
| 0x46C2B0 | Scene::Render: draw gate | yes | `[0x009F5D7C]!=0 && 0x436F70()!=0` | 0x46C31C `CMP [0x009F5D7C],0`; 0x46C325 `CALL 0x00436F70`; 0x46C32F `JNZ 0x0046C359` (draw) else skip |
| 0x46C190 | UpdateEmitters preroll: 100 x AnimateParticles(0.1) per emitter, no frustum test | no (once per scene) | `[scene+0x14]==0` | 0x46C2B0 gate on `[param_1+0x14]`; increments each Render (decomp 46/0046c2b0.c line 1-10) |
| 0x469950 | DoEmitterBucket: qsort, animate, draw | yes | `[0x009F5D60]!=0`, `[0x009F6304]!=0` (both 1 at load) | `_qsort(DAT_00a732c4,DAT_00a732c8,4,0x467330)`; 0x469A88 `CALL 0x004EC080` (AnimateParticles); 0x469A99 `CALL 0x004E92A0` (RenderFunc, unconditional) |
| 0x469950 | per-emitter animate guard | yes | `scene.vt[0xA8]() != 0 || owner.vt[0x1B8]() != 0` | decomp 46/00469950.c; dt = min(frame, 0.1) via `[0x009876FC]` (0.1) |
| 0x4EC080 | PartEmitter::AnimateParticles (only callers: 0x469950, 0x46C190) | yes (via 0x469950) | owner `[+0x40]!=0` (0x4EC096 `CMP [EAX+0x40],0`); warm-up `[owner+0x19c].+0x94 >= 1.0` (0x4EC080 decomp line 102) | prologue 0x4EC080: `PUSH EBP / MOV EBP,ESP / SUB ESP,0x4D8` |
| 0x4EC080 | birth count per tick | yes | inside AnimateParticles | 0x4EC3E3 `FLD [EDX+0xD0]`; 0x4EC3EF `FMUL [EAX+0x100]`; 0x4EC3F5 `FADD [0x00987F08]` (0.5); 0x4EC3FB `CALL 0x0091C860` (float->int); 0x4EC400 `MOV [EBP-0x74],EAX` |
| 0x4EC080 | birth loop bound | yes | none (no cap) | 0x4EC696 `CMP EAX,[EBP-0x74]`; 0x4EC699 `JG 0x004ECAF0` (decomp line 205 `local_84 <= local_78`) |
| 0x4EC080 | pool capacity reserve (6x per tick) | yes | none | 0x4EC413 `CALL 0x004292D0` (and 0x4EC428, 0x4EC43D, 0x4EC452, ...) with arg count+2 |
| 0x499EB0 | scene octree leaf callback: emitter list add | yes | frustum test | 0x499B29 `PUSH 0x499EB0`; 0x499B50 `CALL 0x519BA0` (octree walker); 0x49AB43 `CALL 0x00494200`; 0x49AB4B `TEST EAX,EAX`; 0x49AB4D `JNZ 0x0049AB5D` (skip add when culled); 0x49AB58 `CALL 0x004491C0` (Add to 0xA732C4) |
| 0x499EB0 | second add path (UniquePush, BAODUR exemption) | yes | frustum test, name check | 0x49ABA3 `CALL 0x00494200`, `JZ 0x49ABE2` then UniquePush 0x474000 (R1 decomp lines 236-244) |
| 0x494200 | sphere vs frustum; returns 1 = culled | yes | none | decomp 49/00494200.c; no camera-distance term (R1) |
| 0x499610 | CollectAndTraverse: `CALL 0x495760` clears list (0x499775), octree walk | yes | none | 0x499B50 / 0x499B87 `CALL 0x00519BA0` |
| 0x40A750 | WndProc | event | message | case 5 (WM_SIZE): wParam==2 on main window sets `[0xA1B4D4]=2` (decomp 40/0040a750.c lines 70-79). case 0x1C (WM_ACTIVATEAPP): wParam==0 calls 0x40BC40 (lines 95-120) |
| 0x409750 | window-mode/resize, consumes `[0xA1B4D4]` | yes, when flag set (main loop top) | `[0xA1B4D4]!=0` | 0x409B6B `CMP [0x00A1B4D4],0`; 0x409B74 `MOV [0x00A1B4D4],0` (cleared in the same frame) |
| 0x40BC40 | deactivate: `[0xA1B4E0]=0` | event | `[0xA1B4E0]!=0` | 0x40BCE1 `MOV dword ptr [0x00A1B4E0],0x0`; also 0x0073FE40 (SPECULATIVE: game clock/pause) |
| 0x40BCF0 | activate: `[0xA1B4E0]=1` | event | `[0xA1B4E0]==0` | 0x40BD04 `MOV dword ptr [0x00A1B4E0],0x1` |
| 0x781BE0 | client MainLoop: reads `[0xA1B4E0]` | yes | flag | 0x781D8A, 0x781E0B, 0x781E14 `CMP [0x00A1B4E0],0`. Gates movie/UI-side calls (SPECULATIVE on exact effect). No sleep |
| 0x4317D0 | GL extension loader: fetches swap pointers | once, init | none | 0x4329C3 `PUSH 0x988814` ("wglSwapIntervalEXT"); 0x4329CE `MOV [0x00A32AA8],EAX` (pointer only, no call) |
| 0x4844E0 | ext gate: WGL_EXT_swap_control bit | once per call | `(DAT_009F60DC & DAT_00A32DF8)==DAT_009F60DC` | decomp 48/004844e0.c |
| 0x478D70 | swap-interval setter | on ini apply | `0x4844E0()!=0` | 0x478D7D `JZ 0x00478D99`; 0x478D7F..0x478D87 `NEG/SBB/NEG` = (arg!=0); 0x478D8A `CALL dword ptr [0x00A32AA8]`; 0x478D93 `MOV [0x009F48F8],DL` (cache) |
| 0x7B88B0 | options/ini apply: reads "V-Sync" | on options apply | ini read result `[EBP-0x4A4] != 0` | 0x7BAB92 `PUSH 0x9A0184` ("V-Sync"); 0x7BABEA `CALL 0x00736320` (ini read); 0x7BABEF `MOV [EBP-0x4A4],EAX`; 0x7BAC22 `CMP [EBP-0x4A4],0`; 0x7BAC35 `CALL 0x0091CF18` (atoi); 0x7BAC5E `CALL 0x007BC520` |
| 0x7BC520 | stores `[this+0x48]`, calls 0x478D70 | on ini apply | none | decomp 7b/007bc520.c: `FUN_00478d70(value != 0)` |
| 0x7B88B0 callers | 0x79F6F0 (callers 0x788D90, 0x740B60) | not per frame | none | callgraph |
| 0x476BE0 | window/GL (re)setup: SetWindowLongA 0x476D8E, ShowWindow 0x476E1A | SPECULATIVE one-time | none | 0x4778B5 `CALL dword ptr [0x00A32AA8]` with arg `(DAT_009F48F8 != 0)` (no ext gate). Callers vt-only, so frequency not proven |
| 0x409ED0 | secondary present loop (loading or modal) | yes when used | `0x436F70()` | 0x409FC9 `CALL [0x0098601C]` (SwapBuffers); callers include 0x55FA90 and 0x87FE20 (both have Sleep call sites in imports.tsv) |
| 0x508C10 | LightManager::UpdateLights (vt slot 4 of table at VA 0x991AB0, SPECULATIVE vtable) | SPECULATIVE per frame | `light+0x40 != 0` | decomp 50/00508c10.c: loop over `[this+0xC]` lights, vcall `+0xC` on `light+0x94`; no sort, no distance |
| 0x508CA0 | LightManager candidate select + qsort + pick | SPECULATIVE per frame (vt slot 16, VA 0x991AF0) | `light+0x58 != 0`, and (`param2` or `light+0x80 <= 0.0`), and position != origin (0x517980) | decomp 50/00508ca0.c: O(N) scan, push via 0x473F40, `_qsort(...,0x508160)`, loop breaks at `local_14 > 2` |
| 0x508160 | candidate comparator | with 0x508CA0 | none | compares int fields `+0x74/+0x90/+0x70` (priority/area), not camera distance (SPECULATIVE) |
| 0x508F70 | EnableHighestPriorityLights (from U4) | SPECULATIVE per frame | `min(n, [this+0x48])` | glEnable(GL_LIGHT0+i) for up to N lights; U4 H4 probe not run |

## Q1 answers (emitters)
- Simulation point: 0x469950 (DoEmitterBucket) calls AnimateParticles 0x4EC080 at 0x469A88 once per visible emitter per frame, then RenderFunc 0x4E92A0 at 0x469A99.
- Off-screen emitters: no per-frame simulation. They never enter the 0xA732C4 list because 0x499EB0 skips the add when 0x494200 returns 1 (0x49AB4D `JNZ`). Exception: the UniquePush path at 0x49ABA3 keeps names containing "BAODUR" (R1). Their state is frozen, not caught up.
- Preroll: 0x46C190 runs AnimateParticles(0.1) x100 for every registered emitter on the first Scene::Render of a scene (not frustum-gated). One-time, not per frame.
- Particle cap / birthrate global: none found. Birth count is per emitter: `count = (int)(f[+0xD0]*f[+0x100] + 0.5)` (0x4EC3E3..0x4EC400). The birth loop has no clamp (0x4EC696). Pool growth is by 0x4292D0 (count+2). Global cap, if any, is inside 0x4291C0 (not checked, SPECULATIVE).
- Camera-distance cut fit: best at 0x49AB43. The 0x494200 call is 5 bytes (`E8 B8 96 FF FF`), with the emitter pushed at 0x49AB42 (`PUSH EAX`, EAX = `[EBP-0x38]`). A stub there can return 1 for far emitters and call 0x494200 otherwise. Same site culls both animate and draw. Second site 0x49ABA3 (`E8 58 96 FF FF`) needs the same cut, keeping the BAODUR exemption.
- Function-entry alternative: 0x4EC080 prologue is `55 8B EC 81 EC D8 04 00 00 56 89 8D 50 FB FF FF` (10 + 6 bytes). An early return must replicate the epilogue, so the list-add site is cleaner.

## Q2 answers (lights)
- LightManager per frame: 0x508CA0 does a full O(N) pass over all lights (`[this+0x30]`), filters, pushes candidates, then `_qsort(...,0x508160)`. It is not a per-light distance sort.
- Distance cut float: none. The only float constants in 0x508CA0/0x508C10 are 0.0 (0x986B38) and 1.0 (0x986B50). The 0x517980 call is a vector-equality test against the origin, not a range test.
- Per-frame caller: not proven. Vtable-only (no callgraph edge). The table at VA 0x991AB0 (file 0x5906B0) lists 0x508980 AddLight, 0x508BE0 AddRenderLight, 0x508C10 UpdateLights, 0x508CA0 select, 0x508E00/0x508EC0 prio, 0x508790/0x5087D0 setup. The writer of that vtable is not located (SPECULATIVE that it is the LightManager vtable). U4 H4 probe would settle call frequency.

## Q3 answers (pacing)
- wglSwapIntervalEXT: pointer fetched at 0x4329CE (`MOV [0x00A32AA8],EAX`). Called from 0x478D8A with `(arg != 0)`, so the interval is 1 when V-Sync=1 and 0 otherwise. The call is gated by the WGL_EXT_swap_control bit (0x478D7D, via 0x4844E0). Path: ini V-Sync (0x7BAB92) -> atoi (0x7BAC35) -> 0x7BC520 (0x7BAC5E) -> 0x478D70.
- Unfocused: no Sleep, WaitMessage or focus test in 0x407920. WM_ACTIVATEAPP false only clears `[0xA1B4E0]` (0x40BCE1). The loop still draws and SwapBuffers. Whether SwapBuffers blocks on vblank for an occluded window is driver-dependent (SPECULATIVE).
- Minimised: WM_SIZE minimise sets `[0xA1B4D4]=2`, consumed in-frame (0x409B74). The client width is 0, so 0x436F70 returns 0 and both the swap (0x408429) and scene draw (0x46C325) are skipped. No Sleep follows (0x40846E..0x408475 skipped because `[0xA1B704]==0`). Net result: a full-speed CPU spin with no present (SPECULATIVE on the CPU cost, which is not measured).

## Q4 design note (no code)
- Frame cap hook site: 0x40846E. It runs every iteration after the swap block (both swap and skip paths converge at 0x408455) and after `0x00741090`. Replace the 17-byte run 0x40846E..0x40847E (`CMP [A1B704],0; JZ; PUSH 0; CALL [Sleep]`) with `E8 rel32` + 12 NOPs, where the stub saves flags and registers (`pushfd; pushad`), and then computes Sleep(ms) and returns. The old Sleep(0) path is dead, so nothing else depends on it. Limiter spin reference: 0x40852A..0x4085C5. A JMP-patch at 0x4085C5 (`E9 60 FF FF FF`) would redirect the spin into the stub if `[0xA1B710]` is ever set.
- Idle stub logic: if client width==0 (0x436F70), Sleep(50). Else if window inactive (`[0xA1B4E0]==0` or GetForegroundWindow != main HWND), Sleep(16..33). Else if a user FPS cap is set, Sleep(remaining-1) while remaining > 2 ms, and keep only a short spin for the last 1-2 ms.
- Do not set `[0xA1B710]` directly: the existing spin would burn a core.

## Candidate levers
| # | lever | site | value | risk |
|---|---|---|---|---|
| 1 | Sleep(50) while minimised (client width 0) | 0x40846E stub | high (removes full-core spin while minimised; SPECULATIVE on size) | low: no present in that state; input latency up to 50 ms on restore |
| 2 | Sleep 16..33 ms while inactive | 0x40846E stub, `[0xA1B4E0]` | med (SPECULATIVE; depends on whether SwapBuffers blocks when occluded) | low-med: alt-tab latency; audio/UI cadence may change |
| 3 | User FPS cap with Sleep, not spin, for V-Sync off | 0x40846E stub | med for V-Sync off users; none if V-Sync on and driver blocks | low: spin replaced; frame pacing may jitter at 1 ms granularity |
| 4 | Distance cut on emitters | 0x49AB43 and 0x49ABA3 (call to 0x494200) | high in particle-heavy scenes (saves animate + draw) | med: far emitters pop; BAODUR exemption must be kept |
| 5 | Lower preroll 100 -> N | 0x46C190 loop bound | low (load-time only) | med: visual change at area load |
| 6 | Distance-gated light candidates | 0x508CA0 candidate push (before 0x473F40) | low-med (SPECULATIVE on per-frame caller) | med: light pops |
| 7 | Skip qsort when visible emitter count < 2 | 0x469950 before `_qsort` | low | low |

## Unproven / SPECULATIVE
- SPECULATIVE: LightManager per-frame frequency (vtable-only).
- SPECULATIVE: whether SwapBuffers blocks for occluded or inactive windows under Proton/Wine GLX.
- SPECULATIVE: 0x476BE0 call frequency (one-time vs per-mode-change).
- SPECULATIVE: minimised CPU cost (not measured; static proof only that no sleep or swap runs).
- Absence in callgraph is not proof. The statements about direct writers use the globals table and an asm grep for `[0xA1B704]` and `[0xA1B710]`, which found no other occurrences.
