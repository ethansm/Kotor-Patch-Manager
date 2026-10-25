# R2: 3D far clip plane, swkotor2.exe (Steam, OpenGL)

Static evidence only (dump `steam_6a522e71` + byte checks on the installed exe). No live REA.
Caveat: dump dir label says sha 6a522e71, installed exe sha256 is 7751f7d8. Spot-checked bytes at 0x47f160, 0x47f6d7, 0x47ecd0 match the dump asm, so Steam addresses hold.

## 1. Projection site (CONFIRMED, high)
- `FUN_0047f320` (Camera method, `__fastcall`, size 4362). Only perspective path in the dump; no `glFrustum` import exists.
- Perspective: `gluPerspective(fovy=[this+0x204], aspect=vp_w/vp_h, zNear=[this+0x210], zFar=[this+0x214])`.
  - Call: `0047f6d7  e85abd4b00  CALL 0x0093b436` (gluPerspective IAT thunk, the only gluPerspective call).
  - decomp/47/0047f320.c:189 `(double)*(float *)(param_1 + 0x214)`.
- Ortho branch (taken when fov<=0, `*(float*)(this+0x204) <= 0.0`): `glOrtho(0,w,0,h,[+0x210],[+0x214])`, decomp line 162, asm `0047fc77 CALL dword ptr [0x0098629c]`.
- Far is read from the object every frame. No constant at the projection site.

## 2. Storage and setter (CONFIRMED, high)
- Camera vtable `Camera::vftable` = VA 0x98c45c (ctor `FUN_0047ecd0` writes it at 0x47ed20).
- Setter `FUN_0047f160` (thiscall, near, far) = vtable slot 16 (0x40), VA 0x98c49c.
  - asm `0047f16d FSTP float ptr [EAX + 0x210]`, `0047f179 FSTP float ptr [ECX + 0x214]`.
- Getter `FUN_0047efa0` = slot 15 (0x3c), VA 0x98c498.
- Setter has no direct callers in callgraph.tsv, so all calls are virtual.
- `FUN_0047f190` = SetFOV (slot 17, 0x44), writes +0x204.

## 3. Defaults and values (CONFIRMED, high; floats decoded from exe)
- Camera ctor `FUN_0047ecd0` (allocated by `FUN_00481ac0`, size 0x268): near = VA 0x9876fc = 0.1, far = VA 0x98c270 = 100.0, fov = VA 0x98c274 = 45.0.
- World-init writes far = VA 0x987700 = 10000.0 with near 0.1, via slot 16:
  - `FUN_00417f40` (scene + camera creation, camera at param_1[6]).
  - `FUN_007f3040` (camera create, or re-set when it exists).
  - `FUN_007f67f0` (called from `FUN_007c3f50`, callers 007cbf60, 007c7d60).
- So the far plane for normal world scenes is a fixed constant 10000 m. It is not per-area in the static evidence.

## 4. Minigame far (MEDIUM)
- `FUN_0083db30` reads a "MiniGame" section, keys `Far_Clip` (default 100.0), `Near_Clip` (default 0.1), `CameraViewAngle` (default VA 0x9a34e0).
- `FUN_0083c350` stores (min,max) at this+0x68/+0x6c and calls slot 0x40 (SetNearFar) on this+0x20.
- `FUN_0083c3f0` calls slot 0x44 (SetFOV) with CameraViewAngle.
- Data-driven per minigame. The source file name for the MiniGame section is not traced.

## 5. Per-area (.are) and CameraStyle (MEDIUM)
- `FUN_00523870` (area loader) reads `CameraStyle` (line 308), `MoonFogNear/Far`, `SunFogNear/Far` (defaults VA 0x987700). It has no far or clip key.
- `FUN_006ea6d0` reads "CameraStyle" and writes only object+0x78. No clip write found.
- Conclusion: .are carries fog, not far. CameraStyle consumers not traced.

## 6. Excluded writers of +0x214
- `FUN_0055b830` stores a heap pointer at +0x214 (different struct).
- `FUN_00836350` stores a float at +0x214 (Minimum/Maximum_Speed config, different struct).

## 7. OPEN conflict (LOW)
- `FUN_007f4610` switch on byte at `FUN_0072fb00()+0x75`:
  - case 0 `FUN_007f5b20` (decomp/7f/007f5b20.c:67),
  - case 1 `FUN_007f6030` (line 31),
  - case 2 `FUN_007f61b0` (line 53).
  - Each calls slot 0x40 with `_DAT_0098c274` = 45.0 as the second arg.
  - If the object at +0x40 is the Camera, far = 45 m in those modes. Class not verified.

## 8. Fog (not traced)
- `Scene::SetFogRange` Steam 0x464c80: start at this+100 (decimal, = +0x64), end at +0x68. Callers are virtual, not traced.

## Mod implication (for the caller)
- Hooking Camera::SetNearFar (VA 0x47f160, vtable slot 16) can clamp far higher and covers every writer above.
- Patching the float at VA 0x987700 changes 9 decompiled users, not just the camera.
