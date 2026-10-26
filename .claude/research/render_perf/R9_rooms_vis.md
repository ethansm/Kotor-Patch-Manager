# R9 rooms / VIS / bucket drains (Steam swkotor2.exe, static only)

OUTPUT: /home/ethansm/orca/workspaces/local-setup/KOTOR-II/.claude/research/render_perf/R9_rooms_vis.md
EVIDENCE: 0x499300 `CMP dword ptr [0x009f624c],0` (0x499341); 0x46FB40 `MOV word ptr [ECX+0xe4],AX` with AX=1 (0x46FB4F); 0x427970 entry `55 8B EC 83 EC 10`, called 3x/frame (0x466033, 0x46630E, 0x46673F)
FINDING: Room VIS is per-frame: 0x499300 frustum-tests neighbour rooms only. No occlusion or distance cull. Texture-id scan 0x427970 reruns unconditionally 3x/frame and is cacheable.
OPEN: Who calls scene vtable slot +0x8C (0x46FB40, VIS-off) and what writes room index DAT_009F6254 (no absolute-address writer in the exe)?
VERDICT: PARTIAL

Basis: decompile + asm in `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/`, exe byte scans (file offset = VA - 0x400C00 for .text, spot-checked on 4 hits: 0x98A41, 0x937FA, 0x99319, 0x26C8B), `callgraph.tsv`, `imports.tsv`. Callgraph has no direct callers for most render functions (vtable/indirect dispatch); absence below is not proof.

## Per-frame call chain

1. `0x46C2B0` Scene::Render: `0x46C3AD CALL 0x427F30` (area-set update), then `0x46C3C4 CALL 0x499610` (CollectAndTraverse).
2. `0x499610`: `0x499635 MOV dword ptr [0x00a0f044],1`. Then `0x49963F CMP dword ptr [0x009f6250],0` / `0x499646 JNZ` / `0x499648 JMP 0x499E99`. When DAT_009F6250 == 0 the whole walk is skipped. This is a global gate, not VIS-specific.
3. `0x49964D CMP dword ptr [0x009f6254],0` / `0x499654 JL 0x499686`: room index < 0 skips the current-room update.
4. `0x499675 MOV word ptr [EAX+0xe4],DX` (DX=0) and `0x49967C MOV dword ptr [0x009f624c],1`: scene+0xE4 cleared, VIS flag set. Neither is cleared again in the per-frame path.
5. `0x499695 CMP dword ptr [EDX+0xd8],0`: room count. Zero means octree-root path. Non-zero means room path.
6. Room path: `0x499A90 CALL 0x499300` (visible-room gather), then `0x499B50 CALL 0x519BA0` per visible room. Args: planes `DAT_00A32A48+0x22C`, camera `DAT_00A32A48+0xA8`, callback `0x499EB0`. Second `0x519BA0` call site is `0x499B87` (root path, arg setup not re-read).
7. `0x519BA0` walks the tree. Leaves go through `0x519900`, which calls the callback. `0x499EB0` (per object) calls `0x4943B0` (box-vs-planes cull) then adds to buckets via `0x4946B0`.

## Answers

**Q1. Is room VIS culling applied per frame?** Yes. `0x499300` (VIS path) takes the current room `scene+0xE0` and its neighbour list `room+0x5C` (count `room+0x60`). It always keeps the current room. Each neighbour is tested against the 6 planes at `DAT_00A32A48+0x22C` (count `+0x230`): `0x499379 ADD EDX,0x22C`, `0x49940D CALL 0x513FA0`, `0x499415 FCOMP [0x00986B38]`. Positive dot means culled. Gates:
- `0x499336 MOVSX ECX,word ptr [EAX+0xE4]` and `0x499341 CMP [0x009F624C],0`. If scene+0xE4 != 0 or the flag is 0, the function falls to the all-rooms copy `0x473FB0(room array +0xD4)`. So VIS is off when +0xE4 is set.
- `0x46FB40` (scene vtable slot +0x8C, base 0x58A744, medium confidence) sets `scene+0xE4 = 1` (`0x46FB4F`). This is the only VIS-off writer found. Its caller is not found (SPECULATIVE: vtable/indirect).
- `0x46FB10` (slot +0x88) clears +0xE4 and returns `scene+0xE0 != 0` (`0x46FB1C`, `0x46FB26`).
- DAT_009F6254 and DAT_009F6250: the only exe hits are reads (`0x49964D`, `0x499662`, `0x49963F`). If no computed-address writer exists, room index stays 0 (room[0] becomes "current room") and DAT_009F6250 stays 0 (the walk never runs). Neither reading fits a working game, so a writer through a base register must exist. OPEN.
- The .VIS adjacency (room+0x5C lists) is not loaded by a function found here. `0x46FD20` WRITES `"%s/%s.VIS"` (room name +0x78, neighbours +0x5C/+0x60) and is an export path. `0x470000` is the LYT (`roomcount`/`trackcount`, `%s %f%f%f`) loader. The VIS reader is not located.

**Q2. Is the octree query frustum-culled, and is there an occlusion or distance test?** Frustum: yes. `0x519BA0` classifies each node against the plane list via `0x519B10` (dot products through `0x44B8A0`, returns mask; semantics of 1/2/3 SPECULATIVE). Straddling planes appear to be appended to a child plane list (`0x51A6F0`, SPECULATIVE). Children are ordered by camera distance (`0x44B8A0` on node center plus radius at +0x0C). This is ordering, not culling. Per-object: `0x4943B0` (`0x4943F8 CMP dword ptr [0x00a73248],0`) tests box corners against planes (`0x494608 CALL 0x44C8A0`, `0x494611 CALL 0x514050`, `0x494619 FCOMP [0x00986B38]`). DAT_00A73248 has one exe hit (the read), so the cull should be always on (SPECULATIVE). Distance: `0x499EB0` stores distance (`0x5142F0`) at object+0x4C as a sort key. No far or LOD compare found in its branches (flags only). Occlusion: none in `0x499610`, `0x499EB0`, `0x519BA0`. No `glGenQueries` import (extension-pointer path not excluded).

**Q3. GL calls per mesh in DoMeshBuckets `0x465F90` (and `0x4662E0`, `0x466700`).**
- No direct `glBindTexture` in the three drains or in `0x4DAE60`, `0x4DB1F0`. The bind (if any) lives in the vtable `+0x60` method of the mesh class, which is not resolved (OPEN).
- Per mesh (inner while of 0x465F90): vtable `+0x60` and `+0x64(0)`. Two `0x472630` map finds (`0x4660F0`, `0x466110`). Owner dedupe `0x476620` (`0x466161`, linear).
- Per bucket: `0x4660C0 CALL 0x4DAE60` and `0x4660CA CALL 0x4DB1F0`. `0x4DB1F0` issues `glBlendFunc` (`[0x0098626C]`, 0x4DB235), `glDepthMask` (`[0x00986360]`, 0x4DB25B/0x4DB265), `glAlphaFunc` (`[0x009862AC]`, 0x4DB2B1/0x4DB2CE), unconditionally. No state cache.
- Per drain: `glEnableClientState` once (`0x46624B`, `[0x00986314]`).
- `0x466700` (fog/material pass): per mesh, if owner+0x94 >= threshold, it does `glEnable(fog)`, `glFogfv`, 3x `glFogf`, vtable `+0x64`, `glDisable`, then 3x `glFogf` + `glFogfv` to restore. About 8-10 fog calls per mesh (count unmeasured).
- Sorting: buckets keyed by texture id (map at `0x472630`). Iteration order is the grid-scan order of `0x427970`, not GL-state sorted. Bucket state is not sorted by blend mode.

**Q4. Cost of `0x427970` and caching.** `0x427970` (234 bytes) loops over DAT_00A1BC70 items at DAT_00A1BC6C, skips items with byte +0xE5 != 0, then scans `(short)+0xC6 * (short)+0xC8` cells of the item grid at +0x40, pushing non-zero cells into the vector at DAT_00A1BC48 (`0x4491C0`) and tracking max into DAT_00A1BB84. Called 3x/frame: `0x466033` (in 0x465F90), `0x46630E` (in 0x4662E0), `0x46673F` (in 0x466700). Each call is followed by `0x49B760` (copies DAT_00A1BC48 into a local vector, heap growth through `0x4491C0`).
- `0x427F30` (per frame from `0x46C3AD`) calls `0x427810` only when `DAT_00A1BC30 != 0` (`0x427F82 CMP`). `0x427810` is the same build plus `0x421900` and a pending drain (`0x4260E0`). So the dirty signal exists, but `0x427970` ignores it.
- Inputs change only on area-set change. Grid-cell writers and who bumps DAT_00A1BC30 are not located: cache validity SPECULATIVE.

**Q5. Candidate levers.** See the table and levers below.

## Table

| addr | name / role | per-frame? | gate | evidence |
|---|---|---|---|---|
| 0x46C2B0 | Scene::Render | yes | DAT_009F5D7C != 0 and `FUN_00436F70` | `0x46C3AD CALL 0x427F30`; `0x46C3C4 CALL 0x499610` |
| 0x499610 | CollectAndTraverse, room/octree dispatch | yes | DAT_009F6250 (whole walk); room count +0xD8 at `0x499695` | `0x499A90 CALL 0x499300`; `0x499B50 CALL 0x519BA0` |
| 0x499300 | VIS neighbour gather + plane test | yes | scene+0xE0 != 0 (`0x499309`); scene+0xE4 == 0 and DAT_009F624C != 0 (`0x499336-0x499348`) | `0x49940D CALL 0x513FA0`; `0x499415 FCOMP [0x986B38]` |
| 0x46FB40 | VIS-off setter, scene vtable +0x8C (base 0x58A744, medium) | no (caller unknown) | none | `0x46FB4F MOV word [ECX+0xE4],AX` (AX=1) |
| 0x46FB10 | VIS-on / has-room query, slot +0x88 | no (caller unknown) | none | `0x46FB1C` clears +0xE4; `0x46FB26 CMP [EDX+0xE0],0` |
| 0x519BA0 | BSP/octree walk, plane classify, front-to-back | yes | none (no distance/occlusion) | `0x519B10` dot tests; cmp vs 0.0 only |
| 0x519B10 | node vs plane classify (mask) | yes | none | callee of 0x519BA0; semantics SPECULATIVE |
| 0x519900 | leaf dispatch to callback | yes | none | `(*param_5)` call in decompile |
| 0x499EB0 | per-object callback: distance key (+0x4C), bucket add | yes | DAT_00A73284, DAT_009F6230, DAT_00A0F190 | `0x5142F0` distance; no far/LOD cull found |
| 0x4943B0 | per-object box vs planes | yes | DAT_00A73248 == 0 (`0x4943F8`); no writer found, always on (SPECULATIVE) | `0x494608 CALL 0x44C8A0`; `0x494619 FCOMP` |
| 0x465F90 | DoMeshBuckets pass A | yes | DAT_00A73398 count | `0x466033 CALL 0x427970`; `0x4660F0`/`0x466110 CALL 0x472630` (2 per mesh); `0x466161 CALL 0x476620` |
| 0x4662E0 | drain B | yes | none | `0x46630E CALL 0x427970`; `0x4663A8 CALL 0x4DB1F0` per bucket |
| 0x466700 | drain C, fog/material pass | yes | per-mesh owner+0x94 vs 0x986B50, local+0x84 > 0 | `0x46673F CALL 0x427970`; per-mesh fog block (decompile lines 96-120) |
| 0x4DB1F0 | per-bucket blend/depth/alpha set (no cache) | per bucket | none | `0x4DB235 glBlendFunc`; `0x4DB25B/65 glDepthMask`; `0x4DB2B1/CE glAlphaFunc` |
| 0x4DAE60 | per-bucket state: 0x4DA750 + vtable +0xB8 + 0x426BE0 | per bucket | DAT_009F62EC | `0x4DAE70 CALL 0x4DA750`; `0x4DAECA CALL EAX` |
| 0x472630 | std::map find-or-insert, key = texture id | per call | none | callers: 0x465F90 (8 sites), 0x4662E0, 0x466700 |
| 0x476620 | linear count-of-value (owner dedupe) | per mesh | none | O(N) per mesh; sites 0x466161, 0x46621F, 0x466C3E, 0x466FD4 |
| 0x427970 | tile-grid texture-id scan, to DAT_00A1BC48 + max DAT_00A1BB84 | 3x per frame | none (no dirty gate) | `0x466033`, `0x46630E`, `0x46673F` call sites; entry `55 8B EC 83 EC 10` |
| 0x427810 | same rebuild + `0x421900` + pending drain `0x4260E0` | only when DAT_00A1BC30 != 0 | DAT_00A1BC30 | `0x427F82 CMP [0x00A1BC30],0` (in 0x427F30) |
| 0x427F30 | per-frame area-set update | yes | DAT_00A1BB9D == 0 | called from `0x46C3AD` |
| 0x49B760 | copy DAT_00A1BC48 into local vector | 3x per frame | none | push_back loop, heap growth |
| 0x495760 | per-frame object-list reset, calls 0x427A60 | yes | none | callee of 0x499610 |
| 0x46FD20 | writes `%s/%s.VIS` (room adjacency export) | no | none | string 0x0098BDC4; writer only |
| 0x470000 | LYT loader (roomcount/trackcount) | load time | none | strings 0x0098BDDC, 0x0098BDD0 |

## Candidate levers

Entry bytes are from the `.asm` dumps (for a 5-byte JMP rel32 detour, 5 bytes must sit on instruction boundaries).

1. **Cache or reuse the texture-id scan (value HIGH, risk MED).** `0x427970` does 3 full tile-grid walks plus 3 vector copies per frame. Option A (lower risk): run the scan once per frame and reuse DAT_00A1BC48 for the other two drains. Option B: generation cache keyed on the same signal `0x427810` uses (DAT_00A1BC30). Detour: entry `55 8B EC 83 EC 10` (6 bytes), call sites `0x466033 E8 38 19 FC FF`, `0x46630E E8 5D 16 FC FF`, `0x46673F E8 2C 12 FC FF`. Risk: grid writers are not located, so invalidation is SPECULATIVE.
2. **Hoist per-mesh map lookups (value MED, risk LOW).** Two `0x472630` finds per mesh in pass A's inner loop (`0x4660F0 E8 3B C5 00 00`, `0x466110 E8 1B C5 00 00`). Cache the bucket pointer and count per texture key. Same pattern in 0x4662E0 and 0x466700. Entry `0x472630` bytes not captured.
3. **Owner dedupe as O(1) stamp (value MED, risk LOW-MED).** `0x476620` is linear per mesh. Sites `0x466161 E8 BA 04 01 00`, `0x46621F E8 FC 03 01 00`, `0x466C3E E8 DD F9 00 00`, `0x466FD4 E8 47 F6 00 00`. Entry `55 8B EC 83 EC 0C`. Use a per-frame stamp on the owner object.
4. **Fog state save/restore in pass C (value MED, risk MED).** 8-10 `glFog*` calls per mesh in the local-fog branch of `0x466700`. Entry `55 8B EC 6A FF` (5 bytes, boundary-safe). Cache the global fog state; restore only when the next mesh differs. Count of meshes taking this branch is unmeasured.
5. **Blend/depth/alpha cache in `0x4DB1F0` (value LOW-MED, risk LOW).** Keep the last (src, dst, depthmask, alphafunc) inside each drain only; reset at drain start. Entry `55 8B EC 83 EC 14` (6 bytes). Do not cache across frames: `glBlendFunc` has 170 call sites.

**Rejected:** disabling VIS (set scene+0xE4 = 1 via 0x46FB40) makes cost go up (all rooms drawn). Removing the per-object frustum test (0x4943B0) is also a net loss.

## Still open

- Callers of scene vtable +0x88 / +0x8C (0x46FB10 / 0x46FB40) and the writer of DAT_009F6250 / DAT_009F6254. Likely a computed-address write through a struct base (SPECULATIVE).
- Scene vtable base 0x58A744 is from the MSVC RTTI pointer 0x9BACC4 at file 0x58A740. The `+0x124 -> 0x46A8A0` dispatch from Scene::Render fits this base; the other base (0x58A770) gives a pure stub there. Confidence medium.
- Mesh class vtable (`+0x60`, `+0x64`), i.e. where the per-mesh texture bind is, is not resolved.
- Dispatch of the drains: they sit in the scene vtable at +0xD8 (0x465F90), +0xF0 (0x4662E0), +0xF8 (0x466700) on the same base. No `MOV reg,[reg+off]` + `CALL reg` site found for these offsets in the scene path (heuristic scan over asm). Caller chain from Scene::Render unresolved.
- Grid-cell writers for item +0x40 and who increments DAT_00A1BC30 (needed to validate lever 1).
