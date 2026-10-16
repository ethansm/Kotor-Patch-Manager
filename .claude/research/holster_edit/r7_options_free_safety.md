# R7: Options panel free safety for holster-editor controls (static recon)

Read-only. Dumps: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71` (decomp `.c` and `.asm`). Where Ghidra's `.c` mislabels a call (for example `FUN_00410800` shows as `IsCached`, and `0x64DDB0` shows as `Free()`), the `.asm` was used. Vtable slots were read from `swkotor2_vanilla_6a522e71.exe` (.rdata, PE parse). No game files, logs, or repo sources were edited.

## OUTPUT

Label recipe, run at `OptDestroy` entry (0x8A1930) before the stolen bytes. The panel, list, and manager are still intact there. `M = [P+0x1C]` (CSWGuiManager, also the global at 0xA1B49C). `V` = vanilla count.

1. Read `C = P+0x28` and `A = P+0x24` fresh.
2. If `P+0x20 == L` (focus): call `P->vt[0x08](0,0)` (SetActiveControl(0,0), 0x40EC00).
3. If `M+0x08 == L` (hover): call `L->vt[0x0C](0)` (leave), then `M+0x08 = 0`.
4. If `M+0x10 == L` (captured): `M+0x10 = 0` and `M+0x0C = 0`. The label's release `vt[0x14]` is RET (0x623320), so no call is needed.
5. If `M+0x14 == L` (last hit, compare-only in 0x413C50): `M+0x14 = 0` (hygiene).
6. Truncate: `P+0x28 = V`, zero `A[V..C-1]`. Do not call list APIs.
7. For each label, `vt[0](1)` only if its vtable == 0x9878BC (slot 0 = 0x7510B0). This frees its inline text/border/base and then deletes the label.

## EVIDENCE

**Panel dtor chain (open item 2)**
- 0x8A1930: scalar deleting dtor, vtable 0x9A701C slot 0. Calls body 0x8A1960, then `delete` (0x9196FD) when flag bit 0 is set.
- 0x8A1960: destroys inline members only: eh-vector 0x148 at +0x1E70, 0x41A3D0 at +0x1CA0/+0x1AD0, 0x41D190 at +0x17E0, CSWGuiLabel x7 inline at +0xEE8..+0x1698 (dtors 0x417290, 0x414C60, 0x418640, no delete), eh-vector 0x1D0 at +0x68. Then `CALL 0x40E860`.
- 0x40E860 (base CSWGuiPanel dtor): `MOV [EAX],0x98739C` at 0x40E88B (vtable reset first). Stores only to +0x60, +0x30, and the vtable. Then `LEA ECX,[P+0x24]; CALL 0x64DDB0` at 0x40E94D/0x40E950. It never stores to +0x20 or +0x28.
- 0x64DDB0 = ATL::CAutoVectorPtr::Free: `_free(*p); *p = 0` (0x9196FD = `_free`). No element loop.
- Conclusion: the chain frees the array buffer and no control. Heap controls in the list leak unless freed explicitly.

**Hover (M+0x08), open item 1**
- Writer: 0x413C50, fastcall ECX=M. `MOV [ECX+0x8],EDX` at 0x413CB3 stores the hit control. It also calls the old hover's `vt[0x0C](0)` and stores M+0x14 at 0x413CD9.
- Direct callers of 0x413C50: 0x4131E0 (site 0x41329E; L-button-down, reached from `PerformLButtonDownAction` 0x7B3560, called at 0x7B13CA from `HandleInputEvent` 0x7B12C0); 0x413350 (site 0x41337D; R-button-down, from `PerformRButtonDownAction` 0x7B3810 at 0x7B13ED); 0x40F3C0 (site 0x40F3DC, `CSWGuiPanel::OnPanelAdded`, with `MOV ECX,[0xA1B49C]` at 0x40F3D6).
- Clear writers: 0x40F4C0 (`MOV [EDX+8],0` at 0x40F576; panel-remove scan, runs via base vt[0x4C] from 0x410670); 0x4100C0 init (0x4101D1).
- Reader: PushModal 0x410900 calls `M+0x08->vt[0x0C](0)` unconditionally when non-null.
- No direct caller of 0x413C50 is a plain mouse-move handler.

**Pressed (M+0x10)**
- Writer: 0x413420 CaptureMouse (ECX=M): `M+0x10 = ctrl`, `M+0x0C = flag`, after `old->vt[0x14]()`. Callers: 0x419130 (site 0x419179; this is label `vt[0x18]`, which 0x4131E0 calls on the hover), 0x419240, 0x41B660, 0x420850, 0x421630.
- Clear: 0x413460 ReleaseCapture: if `M+0x10 == ctrl`, clear it and call `ctrl->vt[0x14]()`; always `M+0x0C = 0`. Callers: 0x419190 (label `vt[0x1C]`, L-up), 0x4192A0 (label `vt[0x24]`), 0x41AAC0, 0x420B00, 0x421690, 0x8868D0.

**Focus (P+0x20)**
- Store found in panel methods: 0x40EC00 SetActiveControl only. 33 call sites from 32 functions. No full census was run.

**Label vtable 0x9878BC**: [0]=0x7510B0 (free), [3]=0x419060 (`vt[0x0C]`: enter 0x413F90 / leave 0x419710), [5]=0x623320 (RET), [6]=0x419130, [7]=0x419190, [9]=0x4192A0.

**Holster labels are heap**: `createLabelFromTemplate` uses `g_gk.alloc` (`_shared_src/GuiKit.h:175-195`), and `releaseUnlinkedLabel` (GuiKit.h:163) already uses the same slot-0 pattern.

**3D view (open item 3)**
- View vtable 0x9A3D34 slot 0 = 0x84DE70: `FUN_00418130` (scene body) on view+0x60, then 0x418640, then delete if flag bit 0.
- Scene body 0x418130 frees, in order: the array at view+0x7C (count view+0x80), each entry by `vt[0](1)`; camera view+0x78 (vt 0x98C45C) by `vt[0](1)`; Scene* view+0x74 by `vt[0](1)`; then resman bits.
- Only SceneAddModel (0x418360 = CreateModelGob 0x462320 + insert 0x4183B0) fills view+0x7C. The provider calls it once, in `CreateView` (`preview_3d_src/Preview3dProvider.cpp:501`, rig with index -1). Static search: no direct or depth-6/7 call path from Setup3DScene 0x77EC20, SetAppearance 0x76D200, or ctor 0x7645F0 to 0x418360 or 0x418430.
- Creature: `destroyCreature` (Preview3dProvider.cpp:330-341) calls creature vt[0] = 0x765180, which calls 0x7651B0. That dtor frees part/weapon Gob slots (+0xFC..+0x128, +0x168..+0x178, +0x130/+0x134, +0x324, +0x448) and calls 0x740A50. It does not free holder +0x68 directly.
- Creature `SetScene` (vt[0x94] = 0x77D840) calls 0x7ED7F0(scene), which calls holder `vt[+4](scene)` when holder +0x68 is non-null. The creature's Gobs therefore hang off the holder and are told the Scene* (view+0x74).
- Gob dtor 0x459D00 -> 0x45A190: unlinks from owner via 0x461AA0(0) (owner `vt[0xB4]`), frees owned sub-objects, decrements refcounts (0x452CF0). It does not delete child Gobs. Gob vt[0x50] = attach 0x461670 (attach(0,0,0) detaches).

**Resman flags (open item 4)**: DAT_00A1B490+0x34.
- Bit 0x2 = RIMS:MAINMENU: set by 0x881670, cleared by 0x880740, 0x410AA0, 0x418130.
- Bit 0x1 = RIMS:CHARGEN: set by 0x8FA2F0, cleared by 0x410AA0, 0x418130.
- The scene body releases the RIMS block for each set bit (0x418130 lines 50-64).

## FINDING

1. The vanilla panel dtor never frees list entries, so the label recipe only has to clear references and truncate. Labels are heap, so their own slot 0 is a valid free once unlinked.
2. The live hazard for labels is a stale `M+0x08`. 0x40F4C0 and PushModal (0x410900) both call its `vt[0x0C]`. Hover is set on every L/R click and on panel add, so it is reachable again after the panel dies.
3. Pre-existing and out of scope: 0x40E860 runs 0x410670 after 0x8A1960 has already destroyed the vanilla inline labels. If `M+0x08` points at one of those, the vanilla remove scan calls its `vt[0x0C]`. Our recipe does not change this.
4. View: the array free never touches creature Gobs, because they are not in view+0x7C. The remaining double-free candidate is Scene* (view+0x74). The holder holds that pointer, and the scene body frees it by `vt[0](1)`. `freeViewer` runs `destroyCreature` first, so if Scene's dtor deletes holder-registered objects, that is a double free. The scene body also releases RIMS:MAINMENU or RIMS:CHARGEN when their resman bit is set.

## OPEN

1. Scene* (view+0x74) vtable and dtor are not identified statically. This decides the view verdict.
2. Whether the creature's holder (+0x68) Gobs are freed by the creature dtor or by 0x78CF50 (via 0x740A50) is not traced.
3. Whether the creature body is a child Gob of the rig (array[0]) is not established. Setup3DScene shows no attach call. Gob child-list handling is not traced.
4. The writer census for `M+0x08` and `P+0x20` covers only direct stores in the functions found. A full census was not run.
5. Live: log `resman+0x34` and `view+0x74`'s vtable at Options dtor time.

## VERDICT

- **Labels: SAFE-RECIPE.** Use the seven steps in OUTPUT, in that order, before the vanilla dtor runs.
- **3D view (vt 0x9A3D34): NEEDS-PROBE.** Do not free it from `OptDestroy` until open item 1 is resolved, or until a live probe confirms no crash after a free. Keep the F3 leak otherwise.
