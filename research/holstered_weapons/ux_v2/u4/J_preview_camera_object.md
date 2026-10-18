# U4 Question J: which object is the preview scene camera, and how the DLL reaches it (static recon)

Sources: Steam dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/` (paths below relative to `decomp/`), exe `swkotor2_vanilla_6a522e71.exe` in the same dump dir (vtables read from .rdata with a PE parse, scratch script only). Source: `patch_manager_mods/preview_3d_src/Preview3dProvider.cpp`, `patch_manager_mods/_shared_src/SceneView.h`, `K2Addr.h`. No REA, no live logs.

## Verdict
- The preview camera is view+0x78. The engine CSWGuiScene ctor creates it. It is the same object that RenderScene 0x47F320 and MapToScreen 0x4809D0 receive as `this`.
- The DLL needs no new engine call. `sceneview::camera(block, &cam, &camv)` (SceneView.h:187-192) already returns it. CreateView calls it at Preview3dProvider.cpp:502 but keeps it only as a local.

## 1. Owner chain (block -> scene -> camera)
- View block V: vtable 0x9A3D34 (CSWGui3DSceneView), size 0x400 (K2Addr SceneViewSize). Built by the mod's createView with `sceneCtor(V+0x60)` (SceneView.h createView).
- V+0x60 = CSWGuiScene sub-object S, vtable 0x98771C. Ctor 0x417F40 (`41/00417f40.c`): `param_1[5] = FID_Create("scene")`, `param_1[6] = FID_Create("camera")`, so S+0x14 = V+0x74 (Scene*), S+0x18 = V+0x78 (Camera*).
- Camera object C = V+0x78. Allocated 0x268 bytes by 0x481AC0 (`48/00481ac0.c`), ctor 0x47ECD0 (`47/0047ecd0.c`).
- C final vtable = Camera::vftable 0x98C45C (ctor writes CAurCamera::vftable first, then overwrites). C+0x00 and C+0x04 both hold this vtable.
- Camera->scene link: CSWGuiScene ctor calls camera vt+0x14 = 0x481120 (`48/00481120.c`) which calls 0x461AA0 with ECX = C+4. 0x461AA0 stores the scene at `param_1[0x25]` = C+0x98 (`46/00461aa0.c`).
- Scene model count: V+0x80 (SceneView.h, S+0x20). Rig `charrec_light` is added to the Scene (Preview3dProvider.cpp:497).

## 2. Draw path that renders the preview with C
- View vt+0x38 = 0x84DE40 (`84/0084de40.c`): `(*(V+0x60)->vt[+0x0C])(arg)`.
- S vt+0x0C = 0x4184F0 (`41/004184f0.c`): calls Scene vt+0x1C (S+0x14); then `FUN_00479460(S+4, S+8, S+0xC, S+0x10, S+0x28, 1, 1.0)` (viewport/clip push); if nonzero calls **camera vt+0x08** (`this+0x18` = C) = RenderScene 0x47F320; then pop 0x4799C0.
- RenderScene 0x47F320 (`47/0047f320.c`): gate `*(C+0x98) != 0` (scene); if C+0x220 and C+0x224 are both 0, aspect = GL viewport w/h (glGetIntegerv 0xBA2) and fovy-derived scale; else viewport = C+0x218..0x224. gluPerspective(C+0x204, aspect, C+0x210, C+0x214). Then `FUN_00435f00(C+0xA8, C+0xB4)` builds the modelview.
- FUN_00435f00 (`43/00435f00.c`): quat branch negates w, glMultMatrixf(R), glTranslatef(-pos). View = inverse(T(pos) R(q)).

## 3. Camera fields (all on C; same offsets in RenderScene and MapToScreen)
| offset | field | evidence |
|---|---|---|
| C+0x98 | Scene* | 0x461AA0 `param_1[0x25]`; RenderScene asm 0x47F335 / 0x47F43F |
| C+0xA8 (vec3) | pose position | RenderScene 0x47F320 `param_1+0xa8`; MapToScreen 0x4809D0 `this+0xa8` |
| C+0xB4 (quat w,x,y,z) | pose orientation | RenderScene `param_1+0xb4`; MapToScreen `this+0xb4..0xc0` |
| C+0x204 | fovy (deg, vertical) | ctor 0x47ECD0 writes 45.0; RenderScene and MapToScreen both read it |
| C+0x210 / C+0x214 | near / far | ctor: 0.1 / 100.0 |
| C+0x218..0x224 | viewport x,y,w,h | SetViewPort 0x47F2B0 (`47/0047f2b0.c`, vt+0x34) writes them; MapToScreen loop reads them |

- Pose fields are the embedded child Gob at C+4: child+0xA4 = C+0xA8, child+0xB0 = C+0xB4. Attach 0x461670 (`46/00461670.c`, case 1) calls node vt+0x0C with `(param_1+0x29, param_1+0x2c)` = (child+0xA4, child+0xB0).
- Attach wrapper: camera vt+0x74 = 0x4819E0 (`48/004819e0.c`), asm `ADD ECX,0x4` then CALL 0x461670. Camera->child is C+4.

## 4. MapToScreen (camera vt+0x28)
- Vtable slot +0x28 of 0x98C45C = 0x4809D0 (read from exe). Same `this` = C as RenderScene. Reads C+0x218..0x224 (falls back to glGetIntegerv 0xBA2 if w or h is 0), C+0x204, C+0xA8, C+0xB4..0xC0.
- Quaternion handling: w is negated (`local_4c = *(this+0xb4) * -1`), same as FUN_00435f00. Matches the HseView.h basis.
- Caveat: with C+0x220/0x224 == 0 it reads the current GL viewport. That is only the view rect while the draw is active (between 0x479460 push and 0x4799C0 pop). Outside the draw it reads the window viewport.
- Caveat: setting C+0x218..0x224 through vt+0x34 also changes RenderScene (aspect and glViewport). Do not set it casually.

## 5. How the DLL gets and calls it (no raw address needed)
- Get: `uint32_t cam = 0, camv = 0; bool ok = sceneview::camera(v->block, &cam, &camv);` (ok only if camv == SceneCameraVtable). Cache `cam` in Viewer at CreateView (after line 502). It lives as long as the block.
- Call: read `vt = *(void**)cam`, check `vt == SceneCameraVtable`, then call slot `0x28` as thiscall with ECX = cam, 5 stack args (out*, x, y, z, char normalized). Add slot offset 0x28 as a named K2Addr entry (e.g. SceneCamVt_MapToScreen) like SceneCamVt_Fov = 0x44.

## 6. Verified vs inferred
Verified (decomp body or exe bytes):
- V+0x78 is created by 0x417F40 (`param_1[6]`); S is V+0x60 by createView's `sceneCtor(view + oScene)`.
- Camera vt slots: +0x08 RenderScene, +0x14 SetScene(0x481120), +0x28 MapToScreen, +0x34 SetViewPort, +0x44 fov, +0x74 attach.
- Draw chain 0x84DE40 -> 0x4184F0 -> camera vt+0x08 (`this+0x18`).
- C+0x98 scene link and the pose/viewport offsets are read by the same functions on the same `this`.
- Attach copies the node pose into C+0xA8/C+0xB4 (0x461670 case 1; 0x4819E0 adds 4 to this).
- Per-frame call of node vt+0x0C inside 0x504D40 at asm 0x504EF0-0x504EF3.

Inferred or not verified:
- Scene vtable slot names (+0x18 render, +0x1C pre-render) are inferred from call shape. Scene class name not checked.
- Which GUI framework caller invokes view vt+0x38 each frame. Not traced.
- Per-frame refresh 0x504D40 gating (DAT_00a20020 check, FUN_005043d0 result). Its effect on the preview camera is not confirmed, so C+0xA8/0xB4 may stay at the attach-time copy.
- Translate 0x505020 default offset is zero (ctor 0x504F60 per D report, not re-read here).
- The camerahook node class: 0x457510 is slot +0x0C of 19 Part-family tables (0x98AF4C..0x98B544, 0x98D944..0x9917EC), not Gob (Gob slot +0x0C = 0x4615D0). The node is assumed Part-family (D report); not confirmed.
- Camera destruction at view teardown (CSWGuiScene dtor 0x9164C0/0x9164F0) not checked.

## 7. Live check that closes the open part (no new code path)
1. Log cam = `*(uint32_t*)(V+0x78)` and C+0xA8 (3 floats), C+0xB4 (4 floats), C+0x204, C+0x218..0x224 at each draw.
2. Log the camerahook node pose at the same time. If C+0xA8/C+0xB4 differs from the node pose, the per-frame refresh is not running for the preview.
3. Compare to MapToScreen output for a known creature point taken inside the draw window.

## Next action
- Store `cam` in the Viewer and log C+0xA8/C+0xB4 next to the hook pose in one live run.
