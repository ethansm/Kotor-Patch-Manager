# R1 Emitter distance cull, Steam swkotor2.exe (6a522e71)

VERDICT: PARTIAL. No emitter-specific camera-distance constant. Emitters are gated by the view frustum, so the effective range is the frustum far plane (area Far_Clip). Far-plane membership in the frustum is unverified.

## Steam twins (GOG -> Steam)
- Scene::UpdateEmitters 0x46c190 (GOG 0x85e510, both 281 B). One-time preroll: Scene::Render 0x46c2b0 calls it only when `param_1[0x14]==0`. Runs AnimateParticles(0.1) 100x per emitter.
- Scene::DoEmitterBucket 0x469950 (GOG 0x85bcd0). Per-frame draw. No direct CALL in dump (fn-pointer dispatch).
- EmitterOrderCmp 0x467330 (GOG 0x859710, 540 B match). Camera-relative sort, not a cull.
- PartEmitter::AnimateParticles 0x4ec080. PartEmitter::RenderFunc 0x4e92a0.

## Per-frame pipeline
1. Scene::Render 0x46c2b0 -> CollectAndTraverse 0x499610 -> `CALL 0x495760` clears emitter list count DAT_00a732c8 = 0 each frame.
2. Octree walk `FUN_00519ba0(root, cam+0xa8, cam+0x22c, FUN_00499eb0, scene)`, using frustum planes at cam+0x22c (ptr) / cam+0x230 (count).
3. Leaf callback 0x499eb0 appends emitters to list 0xa732c4 only if the frustum test passes:
   - `0049ab43 e8b896ffff CALL 0x00494200` / `85c0 TEST EAX,EAX` / `750e JNZ` / `b9c432a700 MOV ECX,0xa732c4` / `CALL 0x004491c0` (node+0x24 list, non-unique Add).
   - `0049aba3 e85896ffff CALL 0x00494200` / `JZ 0x49abe2` then UniquePush `0x00474000` (node+0x8c copy). Culled items are still pushed if the name contains "BAODUR" (decompile lines 236-244).
4. FUN_00494200 (162 B): sphere vs frustum. For each plane, `FUN_00514050(center, item+0xd0, plane) > 0.0` returns 1 (culled). No camera-distance term.
5. DoEmitterBucket 0x469950: `qsort(0xa732c4, EmitterOrderCmp)`. Per emitter:
   - `AnimateParticles(dt)` runs only if `scene->vtbl[0xa8]() != 0 || owner->vtbl[0x1b8]() != 0` (`00469a88 CALL 0x004ec080`).
   - `RenderFunc` runs unconditionally (`00469a99 CALL 0x004e92a0`). So an emitter failing both checks is drawn but not animated.
   - dt = min(frame, 0.1): `_DAT_00987df0 (0.1) <= _DAT_009f62b0 ? 0.1 : frame`.

## Other gates checked
- AnimateParticles entry: body skipped when owner (emitter+0x40) has +0x19c != 0 and `*(float*)(+0x19c+0x94) < 1.0`. Constant 0x986b50 is double 1.0 (`FCOMP double ptr [0x00986b50]`). Not distance. Shape matches a 0.25 s warm-up timer (node timer in 0x499eb0 at 0049abfb-0049ac62 does `+0x94 += dt/0.25`, clamp 1.0). Identity of owner+0x19c is unverified.
- Flag predicates FUN_004e5270 / FUN_004e52b0 test emitter+0x12c bits 0x100 / 0x400. No distance.
- 900.0 (30^2) at 0xa0f074 (225.0 at 0xa0f070) is grass, not emitters. FUN_004986b0 collects cell+0x60 items within 30 units into list 0xa7330c; FUN_0050da00 frees grass buffers (+0x38/+0x3c). Not on the emitter path.
- Far_Clip read at 0x83db30 via `FLD [0x0098c270]`, default 100.0. Near_Clip default 0.1 (0x9876fc). CameraViewAngle default 65.0 (0x9a34e0). 0x83c350 stores near/far at clip-obj +0x68/+0x6c and forwards to vtbl +0x20 -> +0x40. Script SetCameraClipping 0x6f7120 calls 0x83c350 (runtime lever, global not emitter-only).

## Candidate patch points (not constants)
- 0x49ab43 `E8 B8 96 FF FF` and 0x49aba3 `E8 58 96 FF FF` (CALL 0x494200). Replacing with `B8 00 00 00 00` (MOV EAX,0, 5 B, stack-neutral) makes all listed emitters pass. Caveat: the 0x49ab43 path uses non-unique Add, so duplicates are possible.
- No fixed-distance float immediate for emitters found.

## Confidence
- No emitter-specific distance constant in update/draw: MEDIUM-HIGH. List 0xa732c4 is touched by 0x499eb0 (append), 0x469950 (sort/draw), 0x495760 (clear), 0x4723e0 (remove, from PartEmitter dtor 0x4e8f60 and 0x4e3dc0; not decompiled). 0x9839a0 and 0x985090 are unchecked.
- Emitter range == frustum far plane: UNVERIFIED (frustum builder for cam+0x22c not located).

## Open
1. Does the cam+0x22c frustum include a far plane (ties range to Far_Clip)?
2. Identity of owner+0x19c struct and whether owner vtbl+0x1b8 is distance-based (only hit is a data field in 0x4e1d40).
