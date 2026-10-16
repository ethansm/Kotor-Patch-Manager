# D2e recon: route V1 (second layout handle inside the Options ctor)

Method: static only. Dumps `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/<bb>/<addr>.c|.asm`, plus a read-only PE parse of `swkotor2_vanilla_6a522e71.exe` (vtables, strings, globals). No live run. Options ctor = `0x8A1170` (hook `0x8A16FE`, checked).

## 1. Opening and freeing a layout handle

**StartLoadFromLayout 0x40EE40**: `__thiscall`, ECX = panel P, stack = resref (+8), scaleA (+0xC), scaleB (+0x10); RET 12.
- Guard: if P+0x48 bit1 (0x2) is set, it does nothing (asm 0x40EE74-0x40EE7E).
- Handle: `operator new(0xA0)` (0x919723), then CResGFF load `0x716FD0` with ECX = the new block, stack = type 0x7FF, ext `"GUI "` (0x987478), resref (asm 0x40EE83-0x40EEE1). Result is stored at **P+0x30**.
- The loader returns `this` even on failure (`return param_1` at the end of 0x716FD0). A missing file therefore still leaves a non-null P+0x30.
- Loaded test: `0x7111E0(obj)` (`__fastcall`, wraps 0x711C20; 0 when the descriptor is missing or the load failed). Only when it is non-zero does the code load CONTROLS and set P+0x48 bit1 (`OR 2` at 0x40F16B).
- Object vtable 0x99C460. Slot 0 (byte 0x00) = 0x716FA0, the scalar deleting dtor (body 0x7171B0, then free 0x9196FD if flag&1).
- Panel state set from the file:
  - Root struct = index 0. Zeroed by 0x7189D0 (asm 0x40EF00-0x40EF07).
  - P+0x34..0x47 = list ref `{uint32 parent struct index (0), char name[16]="CONTROLS"}` (0x718760, `*p=*cell; strncpy(p+1,name,16)`). The ref holds an index and a name, not a pointer to the GFF.
  - P+4..P+0x10 = x,y,w,h from root "EXTENT" via 0x40E2C0 (LEFT/TOP/WIDTH/HEIGHT).
  - P+0x50 = ALPHA; P+0x54..0x5C = COLOR; BORDER/BACKGROUND go to the P+0x60 object.
  - Panel rect is scaled (ftol(x*sx) etc.) and passed to P->vt[1] and P+0x60->vt[1] (asm 0x40F31F-0x40F353).
  - scaleA scales x,y and scaleB scales w,h. Both are flags, not factors. sx = video_w/800, sy = video_h/600 (see section 2).
  - P+0x48 bit0 is set at the end, always (0x40F35B-0x40F36F).

**StopLoadFromLayout 0x40F5A0**: `__fastcall`, ECX = P, no args (asm 0x40F5A0-0x40F614).
- If P+0x48 bit1 is set: `0x7112B0(obj)` (`__fastcall`, to 0x7130B0 release), then `obj->vt[0](1)`, then P+0x30 = 0, clear bit1, then `0x40FB70(P)` (post-pass over the P+0x24 list).
- If bit1 is clear (failed load), nothing is freed and the object leaks.

**Mod-owned handle**: a mod can do the same sequence itself:
- `new(0xA0)`, `0x716FD0(ECX=block; push 0x7FF, "GUI ", resref)`, then `0x7111E0(obj)` to check.
- Free with `0x7112B0(obj)` then `obj->vt[0](1)`.
- Nothing else in the vanilla code pushes type 0x7FF or uses the "GUI " string besides 0x40EE40 (scan of all asm dumps).

## 2. InitControl 0x40F620 and the tag search

**InitControl 0x40F620**: `__thiscall`, ECX = P; stack = ctl (+8), tag (+0xC), addToList (+0x10), scale (+0x14); RET 16.
- Calls `0x418DF0` with **ECX = ctl** and stack (P, P+0x30, P+0x34, tag) (asm 0x40F62F-0x40F651).
- **0x418DF0 (tag search)**: sets ctl+0x38 = P. Count = `0x718810(list ref)` with ECX = handle. For each index i: `0x718890` gets the struct index, then `0x718F90(handle, idx, "TAG")` and `0x7339D0` (strcmp). The first match calls `ctl->vt[0x48](handle, &idx)` (asm 0x418E97-0x418EF3).
- Search uses the handle passed in (P+0x30) and the index-based list ref. The vanilla list ref is never dereferenced as a pointer. **So swapping P+0x30 alone redirects the tag search.** P+0x34 does not need to change.
- **ctl vt[0x48] = 0x418F20** (PE-verified for CSWGui3DSceneView vt 0x9A3D34), the generic control loader:
  - Rect: `0x40E2C0(ctl+4, handle, idx)` writes raw LEFT/TOP/WIDTH/HEIGHT ints from the matched struct's EXTENT.
  - ID: int field "ID" (0x98789C) into ctl+0x54, default -1.
  - "Obj_ParentID" (optional) is the only use of ctl+0x38 (P). Absent, no use.
  - CONTROLTYPE and every 3D-specific field are never read.
- CSWGuiControl ctor 0x418990 sets ctl+0x54 = -1 (`param_1[0x15] = 0xFFFFFFFF`).
- **Rect**: absolute, in the file's units. P+4 is not added, and scaleA/B are not used here.
  - When scale=1: sx = video_w/800 and sy = video_h/600 (0x9F4390 = 800, 0x9F4394 = 600). The video object is `0x73FEA0` on `[[0xA1B4A4]+4]`, with +0x6C/+0x6E shorts. Map: x*sx, y*sy, w*sx, h*sy (asm 0x40F6A3-0x40F751).
  - Then the letterbox branch runs if DAT_00A1B9F0 != 0 (vanilla file image 0x000C5644; runtime value not checked). It remaps through 0x40FA80/0x40FA30 (side-anchored, mode-dependent). Not fully traced.
  - Then `ctl->vt[1](&rect)` (0x40F8F1-0x40F900).
- **List placement** (addToList != 0 and ID >= 0): if ID < P->count (P+0x28), it writes `P->list[ID] = ctl`, **overwriting** the entry. Otherwise it pads with 0 up to ID and appends (0x40F96C-0x40F9A1). ID must equal the next free index.
- **Swapped handle from another file**: works. The file needs a root struct 0 with a List "CONTROLS" (type 15), each element with "TAG" (CExoString, 10), "EXTENT" (Struct, 14) holding LEFT/TOP/WIDTH/HEIGHT (Int, 5), and optionally "ID" (Int).

## 3. Engine precedents

- One function calls StartLoadFromLayout twice: 0x881970 (asm 0x881A03, 0x881A44). The two calls are exclusive branches (`JLE` at 0x8819E7), so one layout per panel. No second layout on a live panel.
- No layout is loaded without a panel: 0x40EE40 is the only caller of 0x716FD0 with type 0x7FF.
- Inventory and equip 3D views are built inside the panel ctor (repo precedent, see `ux_b_viewport.md` section 2).
- Hook site check: 0x8A16FE is `MOV ECX,[EBP-0xAC]` (6 bytes) immediately before `CALL 0x40F5A0` at 0x8A1704, in function 0x8A1170. The StartLoad is at 0x8A12F7 and all vanilla InitControl calls precede the hook. Layout is open at the hook.

## 4. Risks and minimal safe sequence

Risks:
1. **Restore before Stop.** If the swapped handle is still at P+0x30 when StopLoadFromLayout runs, the mod's object is freed and the vanilla handle leaks. Restore first, then free the mod's handle.
2. **Shared descriptor owner.** 0x712B60 attaches the new object to the (resref, type) descriptor only if the descriptor's owner slot (+0x10) is null (decomp `71/00712b60.c`). A second live open of `hse_view.gui` gets no data, so `0x7111E0` returns 0 and the view is silently empty. Release (0x713010) clears the owner. Free each handle before the next bind. Descriptor lookup 0x712360 is not read.
3. **Failed load leaks.** 0x716FD0 returns non-null on failure. Check 0x7111E0 before the swap, and free the object on failure.
4. **ID clash.** `createView` (`_shared_src/SceneView.h` 120-137) hard-codes addToList=1. Any ID < count overwrites a live list slot. Use addToList=0 plus GuiKit `appendToPanel` (sets ID = count).
5. **Missing tag.** A tag not found leaves ID = -1. With addToList=1 the view is not listed and leaks. Check ID after InitControl.
6. **Letterbox global** DAT_00A1B9F0 changes every rect. Test the live rect; it is not fixed by the mod.

Minimal safe sequence (pseudo-steps, no code):
1. Hook 0x8A16FE. Guard: P+0x48 bit1 set and P+0x30 != 0.
2. saved = P+0x30. obj = new(0xA0) and 0x716FD0(obj, 0x7FF, "GUI ", "hse_view"). If 0x7111E0(obj) == 0, free obj (0x7112B0 then vt[0](1)) and return to vanilla.
3. P+0x30 = obj.
4. view = alloc 0x400, ctl ctor 0x418990, vtable 0x9A3D34, scene ctor 0x417F40 at +0x60. InitControl(P, view, "HSE_VIEW", addToList=0, scale=1). If ID < 0, treat as missing.
5. Labels via createLabelFromTemplate, still with P+0x30 = obj.
6. P+0x30 = saved. **Before anything else touches the layout.**
7. appendToPanel(P, view) (sets ID = count and lists it).
8. Free obj: 0x7112B0(obj), then obj->vt[0](1).
9. Continue. The engine's StopLoadFromLayout frees `saved`.

Live checks: P+0x28 +1 after the bind; view ID == old count; vanilla controls unchanged; re-bind twice with no owner conflict; rect matches the file x sx.

## OPEN and UNKNOWN
- Descriptor lookup 0x712360 and 0x712B60 owner semantics (partly read). Whether a second open is refused at runtime.
- DAT_00A1B9F0 runtime value and the letterbox mapping 0x40FA30 (unread).
- Dtor body 0x7171B0 (unread): confirm it clears the descriptor owner.

FEASIBILITY: route V1 works mechanically. Swap only P+0x30. Restore before Stop. Use appendToPanel rather than addToList=1.
