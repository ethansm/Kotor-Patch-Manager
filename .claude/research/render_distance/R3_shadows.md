# R3 Shadows: Steam swkotor2.exe static recon

Source: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` (decomp/*.c, decomp/*.asm). All addresses are Steam. Constant values read from `swkotor2_vanilla_6a522e71.exe` (VA = RVA + 0x400000; .data initial bytes).

## Answer
- Fixed distance gate on object shadow volumes: YES. Two gates (F1, F2).
- Fixed max count of shadow casters (objects): NO. Object loops are unbounded (F5).
- Fixed max count of lights: YES, DAT_00A0F040 = 1 (F4, medium confidence).

## Function table (Steam)
| addr | name | evidence | role |
|---|---|---|---|
| 0x007b88b0 | INI graphics loader | asm 007ba287 `PUSH 0x9a01d4` ("Shadows"); 007ba358 "AllowSoftShadows"; 007ba425 "Soft Shadows"; file `.\swkotor2.ini`, section "Graphics Options" | reads the 3 shadow options |
| 0x007bc230 | Shadows setter | .c: `this+0x2c = v; v ? 0x478ca0 : 0x478cb0` | Shadows -> DAT_009f5d30 |
| 0x007bc260 | Soft Shadows setter | .c: `this+0x30 = v; v ? 0x478cf0 : 0x478d10` | Soft Shadows -> DAT_009f5da8 |
| 0x00478cb0 / 0x00478ca0 | master shadow flag off / on | asm 00478cb3 `MOV [0x009f5d30],0` / 00478ca3 `=1` | (name map: Global::AurDisableShadows; 478ca0 = AurEnableShadows, inferred) |
| 0x00478cf0 / 0x00478d10 | soft flag on / off | asm 00478cf3 `[0x009f5da8]=1` / 00478d13 `=0` | Global::AurEnable/DisableSoftShadows |
| 0x00479200 | ATI hotfix unlock | asm 0047920d `[0x009f5da8]=0`, sets DAT_009f60f4=1 | called when AllowSoftShadows==1 |
| 0x004687e0 | Scene::DoGobShadows | asm 00468904 `CMP [0x009f5d54],0` | per object x per light; F1 gate |
| 0x00468b50 | Scene::DoGobShadowsForThisLight | asm 00468cdb `FLD [0x0098ba98]` | per light; F2 gate; CALL 0x465c90 at 00468dc1 |
| 0x00465c90 | shadow volume build | .c calls 0x4b33e0 with light pos, radius | per (object, light) |
| 0x0050a630 | candidate light selection | asm 0050a74b `CMP EDX,[0x00a0f040]` | F4 cap; caller 0x4687e0 at 00468824 |
| 0x00486fb0 | MDL light node parser | .c: "radius" -> offset 0x58, "shadowradius" -> 0x60 | F3 per-light range |
| 0x0046a630, 0x0046c5c0 | stencil setup / per-object shadow pass | asm 0046a639, 0046ca02 `CMP [0x009f5d30],0` | master flag readers |
| 0x00842590, 0x00774260 | blob shadow decal | strings ShadowSize, Fx_Shadow_S/M/L | not distance-gated |
| 0x007aa7a0 | CSWCArea::UpdateShadowingLights | [steam_val] verified | no constant in body |

## Findings
F1. Object distance cut, 35.0 units. Gate: `if (DAT_009f5d54 != 0 && length(DAT_00a20020+0xb4 - obj+0xa4) > 35.0) skip object`. Asm: 0x4687e0 @00468904 `CMP dword ptr [0x009f5d54],0`; @00468984 `FLD float ptr [0x009f5d50]` (35.0). Same pattern in 0x468b50 (@00468d47, @00468d76). DAT_009f5d54 .data initial = 1, no writer in dump, so the cut is always on. Confidence: high (code), medium (constant-ness).
F2. View-ref cut, dist^2 <= 1100 (~33.2 units), ForThisLight only. Asm 0x468b50: @00468b9a `MOV EAX,[EDX+0x118]`, @00468ba0 `MOV EDX,[EAX+0x64]` (ref point written to local_18); @00468cdb `FLD float ptr [0x0098ba98]` (1100.0, .rdata bytes 00 80 89 44); @00468ce1 `FCOMPP`; fail -> `JMP 0x00468bad`. Shadow call `CALL 0x00465c90` (@00468dc1) sits inside this gate. Confidence: high (constant, gate); medium (ref point = camera, unresolved).
F3. Light range cutoff per light. Steam 0x4687e0 @0x46897c calls length(light - obj) and compares to max(light+0x58, light+0x60 if >0). Offsets come from MDL keys "radius" (0x58) and "shadowradius" (0x60) in 0x486fb0. Per-light data, not an engine constant. Confidence: high.
F4. Light count cap = 1. 0x50a630 loops `local_14 < DAT_00a0f040` and clamps `this+0x9c` to DAT_00a0f040 (.c lines 66, 84-85; asm @0050a74b, 0050a881, 0050a950, 0050a95b). DAT_00a0f040 .data initial = 1; no textual writer in dump. Returns list at this+0x98, which 0x4687e0 consumes. Confidence: medium (static initial value; no runtime writer seen; live check needed).
F5. No object-count cap. DAT_00a732d0 list / DAT_00a732d4 count (sorted by qsort in 0x467d70) are iterated fully in 0x4687e0 and 0x468b50. Only write seen: reset `MOV [0x00a732d4],0` at 0x495760. Confidence: medium.
F6. Soft-shadow flag DAT_009f5da8 (.data initial 1) gates soft paths in 0x43bb50 (@0043bb6f, @0043be4a), 0x46cf20 (@0046cff3, @0046d143), 0x468de0 (@00468ff3). No distance involved.
F7. Master flag DAT_009f5d30 (.data initial 1) gates stencil setup 0x46a630 (@0046a639) and per-object shadow pass 0x46c5c0 (@0046ca02). INI "Shadows"=0 clears it.
F8. Side note, low confidence: 0x468de0 substitutes 50.0 (0x0098bd58) for the light radius on one branch. Semantics not resolved.

## Constants a mod would touch (read-only findings, nothing edited)
| VA | type | initial | section | read at |
|---|---|---|---|---|
| 0x009F5D50 | float | 35.0 | .data | 0x468984, 0x468d76 |
| 0x0098BA98 | float | 1100.0 (~33.2 u) | .rdata | 0x468cdb |
| 0x009F5D54 | int flag | 1 | .data | 0x468904, 0x468d47 |
| 0x00A0F040 | int | 1 | .data | 0x50a74b, 0x50a881, 0x50a950, 0x50a95b |
| 0x009F5D30 | int flag | 1 | .data | 0x46a639, 0x46ca02 |

.rdata values need a writable redirect or exe patch. A runtime INI key does not reach any of these.

## Open
- Is the Scene+0x118 reference point the camera, and does anything write DAT_00a0f040 at runtime? (Live memory read or log would settle both.)
