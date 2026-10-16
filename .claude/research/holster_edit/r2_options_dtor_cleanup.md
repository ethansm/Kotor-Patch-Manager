# R2: Options panel dtor cleanup for holster-editor controls (static recon)

Read-only. Static dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71` (decomp/*.c|.asm), vtables read from `swkotor2_vanilla_6a522e71.exe` (.rdata, PE parse). No live run, no repo edits. Hook ids as in `patch_manager_mods/holster_editor_src/HolsterEditor.cpp`.

## OUTPUT

Recipe for `OptDestroy` (0x8A1930 entry, before the stolen bytes). Panel and list are still intact there.

1. Preview-3d `Destroy(token)` (already done in `OptDestroy`).
2. Read `C = P+0x28` (count) and array `A = P+0x24` fresh. Let `V = e.vanillaCount`. For `i` in `V..C-1`, check `A[i]` against our 5 pointers (compare only, no deref). Skip any that fail to match.
3. Clear live references to ours (all compares, no deref):
   - `P+0x20` (focus) == ours: call `P->vt[0x08](0,0)` (vanilla SetActiveControl(0), it calls `ours->vt[0x3c](1,1)` and stores 0). Or set `P+0x20 = 0` after that call.
   - `M = P+0x1C` (CSWGuiManager). `M+0x08` (hover) == ours: call `ours->vt[0x0C](0)`, then `M+0x08 = 0`.
   - `M+0x10` (pressed/captured) == ours: `M+0x10 = 0` and `M+0x0C = 0`. Its release `vt[0x14]` is a no-op.
4. Truncate: `P+0x28 = V`, zero `A[V..C-1]`. Do not call listAdd or other list APIs.
5. Free ours with `vt[0](1)` only after the vtable check (label 0x9878BC; view 0x9A3D34 is gated, see VERDICT). Clear pointers before the free.
6. Return. The stolen bytes and vanilla body (0x8A1960, then 0x40E860) then see the vanilla count and no references to ours.

## EVIDENCE

**(a) Panel dtor chain**
- 0x8A1930, scalar deleting dtor, slot 0 of vtable 0x9A701C. Prologue `55 8B EC 83 EC 20`, next insn `89 4D E0` at 0x8A1936. Calls body 0x8A1960, then `operator delete` 0x9196FD if flag&1.
- 0x8A1960 body (673 B, EH frame). Destroys only inline members, no heap frees:
  - eh-vector 5 x 0x148 at `this+0x1E70` (dtor 0x74AC30);
  - 0x41A3D0 at `+0x1CA0` and `+0x1AD0`; 0x41D190 at `+0x17E0`;
  - CSWGuiLabel x7 inline at `+0xEE8, +0x1030, +0x1178, +0x12C0, +0x1408, +0x1550, +0x1698` (vt 0x9878BC, text 0x417290 at +0xD8, border 0x414C60 at +0x60, base 0x418640);
  - eh-vector 8 x 0x1D0 at `+0x68` (dtor 0x41A3D0);
  - then `CALL 0x40E860`.
- 0x40E860, base CSWGuiPanel dtor (vt 0x98739C, then 0x987410):
  - `P+0x60` (CSWGuiBorder, 0x78 B): vt[0](1).
  - 0x40F5A0 (StopLoadFromLayout): only if `P+0x48` bit 0x2 is set. Then CRes release on `P+0x30`, clears bit, calls 0x40FB70.
  - `P+0x30` (layout handle): vt[0](1) if nonzero.
  - If `P+0x1C` is registered (0x410800 != -1): 0x410670, which removes the panel from the manager and calls `panel->vt[0x4C]`. In this path the vtable is base, so that is 0x40F4C0.
  - `P+0x24`: CAutoVectorPtr free (0x64DDB0, operator delete on the array).
  - Ends with `RET` (no delete of `this`).
- Conclusion: no vanilla path frees a listed child. Our 5 entries leak because nothing in this chain frees list entries.

**Control list**: `P+0x24` = array ptr, `P+0x28` = count, `P+0x2C` = cap (GuiKit.h:207; K2Addr Panel_CtlArray 0x24, Panel_CtlCount 0x28). Iterators reading count `P+0x28`: 0x40EAE0 HitCheck, 0x40EC90 Draw, 0x40FB70 restore, 0x40F4C0 remove-scan. HitCheck (0x40EAE0) loops from `count-1` down, so **our tail entries are hit-tested first**.

**(b) Hover, focus, pressed pointers**
- `P+0x20`: focus/active control. Written only by SetActiveControl 0x40EC00 (vt[0x08]). It calls old `vt[0x3C](1,1)` and new `vt[0x3C](0,1)`. Forwarding 0x40EAA0 reads it. Base dtor does not clear it. Options `vt[0x4C]` 0x8A2590 clears it via `SetActiveControl(0,0)` before 0x40F4C0.
- `M = P+0x1C` (CSWGuiManager; global 0xA1B49C is the manager).
- `M+0x08`: **hover** control. Readers: 0x410900 (PushModal; if non-null, `vt[0x0C](0)` unconditionally). 0x40F4C0 (clears it only if it matches a list entry, then `vt[0x0C](0)`). Control `vt[0x0C]` = 0x419060: arg 1 = enter (0x413F90), arg 0 = leave (0x419710). Writer not identified.
- `M+0x10` (`M+0x0C`): **captured/pressed** control. Writer CaptureMouse 0x413420 (`vt[0x14]` on old, then store). Control `vt[0x14]` = 0x623320 = RET.
- `M+0x18`: active panel (vt[0x7C](0)). Compared to P in 0x40F4C0, not a control.
- Control vtables (label and view share these slots): `vt[0x3C]` 0x418C50 (focus), `vt[0x44]` 0x418D20 (hit), `vt[0x4C]` 0x4558A0 (returns 0, so 0x40FB70 skips it).

**(c) Removing our tail entries**: truncation `P+0x28 = V` is enough for every iterator above. Read `P+0x24` fresh (listAdd 0x83EA60 may reallocate). No remover API was found.

**(d) Free slots**
- Label: vt 0x9878BC, slot 0 = 0x7510B0. Flag bit0 set: 0x417290, 0x414C60, 0x418640, then `operator delete`. `GuiKit::releaseUnlinkedLabel` already uses this (GuiKit.h:163).
- View: vt 0x9A3D34, slot 0 = 0x84DE70. Calls 0x418130 on `view+0x60` (scene body), then 0x418640 on view, then delete if flag. Scene body frees:
  - model array at `view+0x7C`, count `view+0x80`, each `vt[0](1)`;
  - camera `view+0x78` (vt 0x98C45C), `vt[0](1)`;
  - `view+0x74` (Scene*), `vt[0](1)`;
  - if `resman(0xA1B490)+0x34` bit 0x2: "RIMS:MAINMENU" release; bit 0x1: "RIMS:CHARGEN" release (global side effects).
- The F3 leak is because nothing calls the view's slot 0. Freeing it would also release the scene models.
- preview-3d: `destroyCreature` (Preview3dProvider.cpp:330-338) calls the creature's `vt[0](1)`. `freeUnlistedBlock` (:455-471) frees a view only while id == -1 (unlisted). After `appendToPanel` the view is listed, so that precedent does not apply.
- cg.py callers of SceneRemoveModel 0x418430: 0x8988B0 and 0x8CD2C0 only. Neither is a creature dtor.

## FINDING

1. The vanilla dtor never touches list entries. The hook point is the only place to free ours, and it is safe if the list and references are fixed first.
2. Unfixed hazards after a free:
   - `M+0x08` (hover) is the highest risk. Tail-first hit-test makes our controls the likely hover target. If it matches a list entry, 0x40F4C0 calls `vt[0x0C]` on freed memory. If truncation hides it, the next PushModal (0x410900) calls it on freed memory.
   - `P+0x20` (focus) is not cleared by the base dtor. Holster code never calls SetActiveControl on ours, so this is low risk but still guard it.
   - `M+0x10` (pressed) is not cleared. Set to 0; release is a no-op.
3. Labels: the free path matches the GuiKit precedent and touches only their own inline children. Safe once references are cleared and the count is truncated.
4. View: freeing it frees the scene's model array with `vt[0](1)`. This is a double-free risk if preview-3d's creature Gobs are also freed elsewhere, which the static pass did not rule out.

## OPEN

1. Writers of `M+0x08` (hover) and `M+0x10` (pressed). Check callers of 0x413420 and hover-enter writes. Affects whether the clear step is complete.
2. Whether the creature's Gobs live in `view+0x7C` and are freed only by the scene body. Needs the CSWCCreature dtor and Setup3DScene's SceneAddModel use, or a live `modelCount` before and after the free.
3. Value of `resman+0x34` bits 0x1/0x2 at Options dtor time. If set, the scene body runs the RIMS side effects. Live log.
4. Whether any child event fires during panel death. We register no events on ours, so this is low.
5. Not cross-checked against the knowledge ledger (`query_ghidra.py address 0x8a1930` returned no results).

## VERDICT

NEEDS-LIVE-PROBE.

Label-only cleanup (clear references, truncate `P+0x28`, `vt[0](1)` with the 0x9878BC gate) is statically SAFE. The view free waits on OPEN 2 and 3. Probe plan: log `modelCount(view)` and `resman+0x34` before the free, then free labels and view and confirm no crash in the next PushModal.
