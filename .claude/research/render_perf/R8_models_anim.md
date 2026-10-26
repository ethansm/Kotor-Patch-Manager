# R8: model (Gob) animation and skinning cost, Steam swkotor2.exe

## 5-line summary
1. Scene::Animate 0x46D230 (Scene vtbl +0x1C) calls obj vtbl +0x14 (Gob::Animate 0x4E1D40) on EVERY object in list +0x54. No visibility or distance test on that path.
2. Gob::Animate has no in-view gate. Its only global gate is DAT_00A04A20 (default 1). The in-view byte Gob+0x11 is written by the scene callback but never read in Gob::Animate.
3. The MdlNode tree walker 0x4DFED0 has the only model LOD: a depth cap (8/12/100) from Gob+0x58 (distance to parent) vs (Gob+0x164 or 0.5)/1.6 x {30, 12}.
4. Skinning is inferred GPU vertex-program with CPU per-draw palette build: 0x4C9640 runs quaternion products per bone and glProgramEnvParameter4fARB uploads, only for drawn meshes (via 0x4CD380, vtbl +0x64). Shader side not inspected.
5. Frame caller of Scene::Animate is unresolved. Shadow cut 35 m (0x9F5D50) is the only distance cull found for models; no draw-level distance cull found.

Caveat: static dumps are steam_6a522e71. Constants and byte checks were read from backups_phase6_1250/swkotor2.exe (sha 7751f7d8). 203 sampled function-entry byte sequences match the dump asm with 0 mismatches, so layout is assumed identical. Vtable slots were found by scanning exe data for function pointers.

## Table: functions

| addr | name/role | per-frame? | gate | evidence |
|---|---|---|---|---|
| 0x0046D230 | Scene::Animate (Scene vtbl +0x1C, `float dt`) | Yes, runs for all list objects | Shadow branch only: `DAT_009F5D40 && DAT_00A2004C` (+0x110 list first, then +0x54 with obj+0x16==0). Default path 0x46D320 has no test | 0x46D241 `CMP [0x009F5D40],0`; 0x46D24E `MOVZX ECX,[0x00A2004C]`; default loop bound 0x46D33F `CMP EAX,[EDX+0x58]`; call 0x46D364 `MOV EDX,[EAX+0x14]`, 0x46D367 `CALL EDX` |
| 0x00464D10 | Scene::AddObject thunk: `(**(code**)(*this+0xB0))(obj)` (= 0x465BB0) | No (insert) | none | decompile 0x464d10; Scene vtbl slot +0xAC = 0x464D10, slot +0xB0 = 0x465BB0 (vtable dump at 0x98BB44) |
| 0x00465BB0 | Scene::ChildAdd: inserts obj into +0x54 unconditionally; into +0x10C if obj vtbl+0x1A8 != 0 | No (insert) | Insert into +0x54 always; obj+0x84 != 0 triggers 0x495F20 + 0x4659F0 | 0x465bec `ADD ECX,0x54; CALL 0x473F40`; 0x465bfc `MOV EAX,[EDX+0x1A8]`; 0x465c0f `ADD ECX,0x10C` |
| 0x004558A0 | Returns 0. Gob vtbl +0x1A8 | n/a | Gob never enters the +0x10C list | 0x4558a7 `XOR EAX,EAX` |
| 0x004E1D40 | Gob::Animate (Gob vtbl +0x14, slot 5) | Yes, per list object | Whole body under `DAT_00A04A20 != 0` (default 1; no writer in dumps). Parent-counter dedupe: `param_1[0x6E]` (+0x1B8) vs parent+0x50 | 0x4E1D7C `CMP [0x00A04A20],0`; walker calls 0x4E2ED1, 0x4E2F36, 0x4E2F9A; byte +0x11 not read in this function |
| 0x004DFED0 | MdlNode animation tree walker (recursive), called from Gob::Animate | Yes, per Gob | Depth cap `local_10`: 8 if d > 30*s (`DAT_00A04A00`=30.0, `DAT_00A04A04`=8); 12 if d > 12*s (`DAT_00A049FC`=12.0, `DAT_00A04A08`=12); else 100. Hard cap `DAT_00A049F8`=1000. Enable `DAT_00A04A0C`=1. s = (+0x164 if >=0 else 0.5)/1.6 (double at 0x991350) | 0x4DFF77 `CMP [0x00A04A0C],0`; 0x4DFFD8 `FDIV double [0x00991350]`; 0x4E0003..0x4E002F; d = owner+0x58 |
| 0x004DD380 / 0x004DFBA0 | Other Gob::Animate helpers (not analysed) | Yes | n/a | callers in Gob::Animate |
| 0x00499EB0 | Scene per-object callback (3956 B) | Visible-only (inside frustum walk) | `CALL 0x4943B0` (test) then vtbl +0x1C0 if passes | 0x499F8E `MOV EAX,[EDX+0x1C0]; CALL EAX` at 0x499F94; second site 0x49A378 uses +0x1C8 |
| 0x0045A000 | Gob vtbl +0x1C0: sets in-view byte Gob+0x11=1, calls vtbl +0x1B4 ("propagateviewvolume") | Visible-only | Reached only from the callback | 0x45a00a `MOV byte ptr [EAX+0x11],1` |
| 0x004598D0 | Gob vtbl +0x1C8: returns byte Gob+0x11 | n/a (getter) | n/a | 0x4598DA `MOV AL,byte ptr [EAX+0x11]` |
| 0x004CD380 | Mesh draw (vtbl 0x98B08C slot +0x64), per DRAWN mesh | Drawn only | `DAT_00A04924`=1, `DAT_00A04928`=1, `FUN_004844C0` (=`DAT_009F60F0`==2 default and `FUN_004845E0` GL caps), `DAT_00A20020` (scene active). Else fallback 0x4C2000 | 0x4CD387 `CMP [0x00A04924],0`; 0x4CD395 `CMP [0x00A04928],0`; calls 0x4CD3CF (0x4C3430), 0x4CD3ED (0x4C9640), 0x4CD424 (0x4C2000) |
| 0x004C9640 | Skin palette build + vertex-program uploads (9743 B). Per bone: quaternion products on CPU, then `glProgramEnvParameter4fARB` (`DAT_00A32DEC`, target 0x8620) with index `0x12 + 3*bone` | Per draw of a skinned mesh, not cached | Only when reached from 0x4CD380 | 0x4CA2F9 `CMP ECX,[EAX+0x1BC]` (bone count); quaternion math 0x4CA691 `ADD ECX,[EDX+0x1C0]`; 0x4CA99A `CALL [0x00A32DEC]`; .c line 441 loop, line 527 `local_e8 = local_ec*3+0x12` |
| 0x004C3430 | Vertex-program path, param_2==0 (25 KB, 18 env uploads, indices 0x10/0x11) | Per drawn mesh | Same as 0x4CD380 | 0x4CD3CF `CALL 0x4C3430`; role unresolved (dangly or other) |
| 0x004C2000 | Fallback path: `glDisableClientState` + fixed path | Per drawn mesh | `DAT_00A04928==0` or no caps or no scene | 0x4CD424 `CALL 0x4C2000` |
| 0x0044EF20 | MdlNodeAnimMesh::InternalGenVertices | No (load-time setup) | Reached only via thunk 0x4549A0 (no direct callers) | 0 FMUL instructions; allocs (0x919723) and push_back |
| 0x0044BD70 | MdlNodeTriMesh/Skin ProcessVerts (2857 B) | No (post-process) | Callers: 0x44EED0, 0x4506C0, 0x44F650, 0x44E3E0 | callgraph callers |
| 0x0046C2B0 | Scene::Render (Scene vtbl +0x18) | Yes | none | calls CollectAndTraverse 0x499610 (known fact) |
| 0x004317D0 | GLRender::InitExtensions | No (init) | none | `DAT_00A32DEC = wglGetProcAddress("glProgramEnvParameter4fARB")` (.c line 216); strings GL_ARB_vertex_program (0x989B30) |
| 0x00781BE0 | Candidate frame driver (8599 B) with vtbl +0x1C call sites | Yes (SPECULATIVE) | n/a | 0x782B51 `MOV EAX,[EDX+0x1C]; CALL EAX`; 0x782BEC same; not proven to be Scene::Animate |

## Answers

Q1 (where is animation advanced):
- Scene::Animate (0x46D230) runs over all list objects every time it is called. The default path loop is 0x46D320..0x46D369 (CALL EDX at 0x46D367).
- Gob::Animate (0x4E1D40) has no visibility test. Its only global gate is DAT_00A04A20 at 0x4E1D7C.
- Who calls Scene::Animate per frame: UNRESOLVED. No direct CALLs (vtable slot). Candidate frame driver 0x781BE0 is SPECULATIVE.
- Frame order (Animate vs CollectAndTraverse 0x499610): OPEN. This decides whether Gob+0x11 is current-frame or one frame stale.

Q2 (skinning CPU vs GPU):
- Skinning is GPU vertex-program. Evidence: ARB vertex/fragment program strings, `glProgramEnvParameter4fARB` pointer DAT_00A32DEC, env index 0x12+3*bone (0x4C9640).
- CPU does quaternion products per bone at draw time (0x4C9640, loop bound 0x4CA2F9). Cost is per drawn skinned mesh, not per object in the area.
- Dangly: NOT resolved. 0x4C3430 is the param_2==0 vertex-program path (18 env uploads), role SPECULATIVE.
- Animation itself (node transforms) is CPU and runs per Gob via 0x4DFED0 for all Gobs (see Q1).

Q3 (existing model LOD or distance cull):
- MdlNode walker 0x4DFED0 (LOD-like, depth cap): thresholds 30.0 (0x4A04A00) and 12.0 (0x4A049FC), scale 1.6 (double 0x991350), default radius 0.5 (0x98C014). Distance is Gob+0x58 = max(|dx|,|dy|) to parent position (+0xB4/+0xB8), or 0.0 if the Gob has no parent. Most top-level Gobs get cap 100 (no throttle). The meaning of Gob+0x164 is UNSURE.
- Shadows: 35.0 m (0x9F5D50) in DoGobShadows path (known fact).
- Draw-level distance cull for model draws: NOT found. The callback 0x499EB0 uses frustum test 0x4943B0 only (body not checked for distance; SPECULATIVE).

## Candidate levers

| # | lever | value | risk | anchor |
|---|---|---|---|---|
| L1 | Skip Gob::Animate when Gob is not in view last frame: detour at Gob::Animate entry (bytes `55 8B EC 6A FF`) checking Gob+0x11 and a catch-up dt accumulator. Alternative: gate the Scene::Animate call loop at 0x46D35F (`MOV EAX,[EDX]`) for Gob-class objects only | high | medium: scripts, events, emitters and controller time for off-screen Gobs would stop until re-entry; Gob::Animate also handles callbacks (+0x6C); one-frame staleness depends on frame order (OPEN) | 0x4E1D40, 0x46D35F |
| L2 | Lower animation depth cap for far Gobs: patch `.data` DAT_00A04A04 (default 8) to 4 | low-med | low-medium: deep bones freeze on far Gobs (visual) | 0x4DFFF9 `MOV EAX,[0x00A04A04]` |
| L3 | Cache skin palette in 0x4C9640: skip quaternion loop and env uploads when node bone array (+0x1C0) unchanged since last upload (per-node stamp); re-upload after any program bind | medium (shadow + main passes draw the same mesh twice per frame) | low-medium: GL program env is global state | 0x4CA2F9, 0x4CA99A |
| L4 | Batch env-param uploads (one call for all bones instead of one per bone) | low-medium | low; availability of batch entry point SPECULATIVE (not in import table) | 0x4CA99A |
| L5 | Frustum-cull also by distance in 0x499EB0 before vtbl +0x1C0 (skip Gob mark and draw beyond N m) | high if effective | medium: needs camera or focus distance source (Scene::SetSceneFocus 0x464CA0); body of 0x4943B0 not verified | 0x499F94 |

Not levers: per-mesh draw is already gated by bucket membership (0x4CD380 runs only for drawn meshes). Dangly and skin palette are not built for hidden meshes.

## Open items
1. Who calls Scene::Animate each frame, and whether it runs before or after CollectAndTraverse (0x499610).
2. Identify 0x4C3430 (dangly vs other).
3. Confirm owner of MdlNode+0x40 is the Gob (inferred from field matches).
4. Read 0x4943B0 for distance tests.
