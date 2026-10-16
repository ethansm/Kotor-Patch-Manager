# D2f recon: embedded GUI layout, no override file

Method: static only. Dumps `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/<bb>/<addr>.c|.asm`. Vtable slots checked in `swkotor2_vanilla_6a522e71.exe` (CResGFF::vftable 0x99C460: +0x0 0x716FA0, +0xC 0x7175B0, +0x10 0x717300). No live run. Names in name_map.tsv are GOG-derived; behaviour below is from the decompiles and asm.

## 0. Correction to d2e
d2e route V1 step 2 needs `hse_view.gui` on disk (its Demand check must return data). The no-file goal replaces that check with the memory handle in section 4.

## 1. Memory GFF: parse-in-place exists (FEASIBILITY high)

No "GFF from pointer" API. The engine protocol is: bytes sit in the CRes data buffer, then the service calls vtable slot +0x10.
- Caller: ServiceFromDirectory 0x7133A0 (decomp ~line 227) calls `(*CRes->vt[0x10])()`, then sets CRes flag 4 only if the result is nonzero.
- **CResGFF::OnResourceServiced 0x717300**: `__fastcall`, ECX = CResGFF, no stack args (RET 0).
  - Reads the buffer at CRes+0x10. Returns 0 if it is null.
  - Checks the signature `"GUI "` (CResGFF+0x8D, set by ctor) against the buffer's first dword, and the version against "V3.2".
  - Sets header pointers: +0x40 header, +0x44 structs, +0x4C fields, +0x54 labels, +0x5C field data, +0x64 field indices, +0x70 list indices (each = buffer + offset).
  - Sets +0x94 = 1, +0x98 = 1, returns 1.
  - **Does not read CRes+0x18 (size) and does not bounds-check any offset.** Embedded bytes must be validated at build time.
- Object fields (CRes base; CResGFF extends it):
  - +0x00 vtable; +0x04 ushort refcount; +0x08 ID (bits 31..30 = source kind: 0 KEY/BIF, 1 image, 2 ERF, 3 directory; -1 = none).
  - +0x0C flags (0x4 has data, 0x10 in-flight, 0x100 on LRU); +0x10 data ptr; +0x14 key entry; +0x18 size; +0x1C attach count; +0x20/+0x24 header variant (0 for us).
  - CResGFF: +0x8D ext "GUI "; +0x94 loaded; +0x98 parsed; +0x9C held by Demand.
- Ctor **CResGFF::Constructor_2 0x716FD0**: `__thiscall`, ECX = obj; stack = type 0x7FF, ext ptr "GUI " (0x987478), resref ptr; RET 12. It zeroes fields, runs SetResObject 0x712B60, then Demand 0x7111E0, then ReleaseResObject 0x713010 if no data.
- **Allocation**: CExoResMan::Malloc 0x712F30 = `__thiscall`, ECX = resman, stack = CRes. It reads CRes+0x18, takes that size from resman+0x8 (budget) (evicting via FreeChunk 0x712160 if needed), calls `operator new` 0x919723, and stores the result in CRes+0x10.
- **Free**: CExoResMan::FreeResourceData 0x7120C0 = `__thiscall`, ECX = resman, stack = CRes. Restores budget, calls `operator delete` 0x9196FD on +0x10, zeroes +0x10/+0x18.
  - Reached from FreeChunk 0x712160 (eviction, refcount 0 only), Free 0x711FE0, and the last-release chain 0x713010 -> 0x711230 -> 0x711E60 -> 0x711FE0.
- resman = `*(DWORD*)0xA1B490` (every caller loads it from there).
- **Owner of the buffer = the CRes.** The parsed pointers alias the buffer, so it must live until the handle is freed.

## 2. Resource lookup injection: no registration API (FEASIBILITY low for a registered entry)

- **CRes::Demand 0x7111E0**: `__fastcall`, ECX = CRes. It loads resman from 0xA1B490 and calls CExoResMan::Demand 0x711C20.
  - ID == -1 -> return 0.
  - flags&4 and data -> refcount++ and return data (the "already loaded" branch).
  - Otherwise switch on ID>>30: 0 ServiceFromResFile 0x713FB0, 1 ServiceFromImage 0x713E80, 2 ServiceFromEncapsulated 0x713BF0, 3 ServiceFromDirectory 0x7133A0.
- **GetKeyEntry 0x712360** (resref, type): searches in order directory (+0x14), ERF type 1 (+0x18), image (+0x1C), ERF type 2 (+0x18), KEY/BIF (+0x10). The override folder is searched first, which is why an override beats BIF.
- **SetResObject 0x712B60**: attaches only if an entry is found and entry+0x10 (owner) is null. Then entry.owner = CRes, CRes+0x14 = entry, CRes+0x1C = 1, CRes+8 = entry+0x14.
- Registration: AddResourceDirectory 0x711690, AddEncapsulatedResourceFile 0x7115C0, AddResourceImageFile 0x7115E0, AddFixedKeyTableFile 0x711670. All go through builder 0x711750 (switch 1..4 = KEY, directory, ERF, image). Each takes a file or directory name (CExoString). No memory variant exists in the 0x711xxx-0x714xxx names.
- The kind field is 2 bits and all four kinds are taken. A memory entry would need a fifth kind, or a hook on GetKeyEntry plus a fake table. **Rejected**: more invasive than section 4.
- **Precedent**: `_shared_src/EmbedTex.h` registers an engine object marked "constructed" so the engine never asks the resman. The CRes analogue is flags 4 + data, which Demand already trusts.
- A handle with ID -1 is never serviced. Section 4 depends on this.

## 3. Repo mods touching the resource manager
- `robustness_fixes_src/RobustnessFixes.cpp`: caves at ReleaseResObject 0x713010 (null guard on [[CRes+0x14]+0x10]), Malloc 0x712F30 stale-state replay, FreeChunk path (fs=2 frees like 0x711FE0). Mutating; a precedent for 0x712F30/0x7120C0 call discipline.
- `perf_lab_src/perflab/PerfLab.cpp` + `kotor2-steam-aspyr.hooks.toml`: observer hooks on Malloc 0x712F30, FreeChunk 0x71222B, FreeResourceData 0x7120C0, Release 0x7130B0, Demand 0x711C20, AsyncStart 0x71447A. G_ResMan 0xA1B490. Confirms the conventions above.
- `texture_memory_release_src`: frees CPU TPC copies, re-Demands from resman. Builds no CRes.
- `equipment_3d_character_src`: no resman calls. Ships `override/equip_p.gui` via `scripts/add_equip_3dview_control.py` (79 controls, ids 49..78 added).
- No repo mod calls the CResGFF ctor 0x716FD0, StartLoadFromLayout 0x40EE40, or parser 0x717300.

## 4. Recommended route A: temporary memory handle (no file, no lookup change)

Use for hse_view (d2e hook site 0x8A16FE, before InitControl). The handle is never registered or looked up.

Steps (pseudo, no code):
1. Build time: produce the GUI bytes (GFF V3.2, "GUI ", root CONTROLS list with TAG "HSE_VIEW", EXTENT, ID). Validate every offset+count*size <= total size. Embed bytes and size in the DLL.
2. Hook site: saved = P+0x30.
3. obj = engine_new(0xA0). Call 0x716FD0 with ECX = obj, push 0x7FF, "GUI ", and a dummy resref that no table holds (e.g. "mod_inline_gui"). Result: ID -1, no owner, no data, refcount 0.
4. obj+0x18 = size. Call 0x712F30 with ECX = [0xA1B490], push obj. If it returns false: skip the view, return to vanilla.
5. memcpy(obj+0x10, embedded, size).
6. Call 0x717300 with ECX = obj. If it returns 0: call 0x7120C0 (ECX = resman, push obj), then obj->vt[0](1), and return to vanilla.
7. On success: obj+0xC |= 4; obj+4 (ushort) = 1; obj+0x9C = 1. Leave obj+0x8 = -1 and obj+0x1C = 0.
8. P+0x30 = obj. Run InitControl and labels. Then P+0x30 = saved.
9. Free: call 0x7112B0 with ECX = obj, then obj->vt[0](1). Do not call 0x713010 by hand.

Why step 9 is exact (vanilla chain): 0x7112B0 sets refcount 1 -> 0 and puts CRes on LRU. Dtor 0x7171B0 sees +0x98 set and calls ReleaseResource 0x717630. That calls 0x713010 (ReleaseResObject): entry null, so no owner clear. It reaches 0x711E60, which sees refcount 0 + flag 4 + data and calls 0x711FE0. That calls OnResourceFreed 0x7175B0, then LRU remove 0x64DDE0, then 0x7120C0 (budget restore, operator delete).

Feasibility: high mechanically (every call anchored; lifetime matches vanilla). Medium overall until a live probe logs the view rect, the ID and the list count.

## 5. Equip_p (same mechanism): route B, main-layout substitution

Yes, the mechanism can replace `equip_p.gui` at an open site, with the override file unread at that site. Route B hooks the Demand check inside StartLoadFromLayout 0x40EE40 instead of a panel site.
- Asm: 0x40EEE1 writes P+0x30 = handle; 0x40EEF3 is `CALL 0x7111E0` (5 bytes); 0x40EEFA JZ 0x40F177 skips CONTROLS load when Demand is 0.
- The vanilla code re-reads the handle from local [EBP-0x18] (0x40EEED, 0x40EF04), not only from P+0x30. The hook must rewrite both.
- Hook (at 0x40EEF3): if the resref at [EBP+8] == "equip_p", free the ctor's handle (dtor path), build the handle from section 4 steps 1-7 (**no** step 8-9 free), set P+0x30 and [EBP-0x18] to it, and return nonzero. Vanilla then loads CONTROLS, sets bit1 at 0x40F16B, and frees our handle through the normal StopLoadFromLayout path.
- Feasibility: medium. The equip panel's StartLoadFromLayout call site is not identified (OPEN).

**Precedence**
- Engine file order: directory override > ERF(1) > image > ERF(2) > KEY/BIF (0x712360).
- An in-memory handle at a hooked open is top precedence for that open only. Override files are not read there.
- Conflict: `generate_ultrawide_gui_overrides.py` and `add_equip_3dview_control.py` both produce override `equip_p.gui` today. If equip3d embeds bytes, the embedded bytes must be the final ultrawide + 3D layout, built at build time (generator -> add script -> embed + sha pin). Otherwise ultrawide geometry is silently dropped at the equip site.
- Recommended: one owner (embedded bytes) for equip_p; delete `override/equip_p.gui`; keep the generator as build input; log the sha used at startup. A leftover disk override is then dead at that site (log it, not an error).

## 6. Risks (route A and B)
1. **Offset trust in 0x717300** (out-of-bounds read on bad bytes). Validate offline; test with truncated copies.
2. **Double free**: exactly one Release + one dtor per handle (step 9 or the Stop path, never both). 0x7120C0 only on the parse-fail path.
3. **ID must stay -1** (route A). A valid kind plus a later Demand after eviction would service a random BIF entry.
4. **Dummy resref** must not exist in any table, or the ctor attaches to it.
5. **Budget**: Malloc may evict other resources under pressure (normal engine behaviour).
6. **Thread**: main thread only; async flag 0x10 never set.

## OPEN and UNKNOWN
- Equip panel's StartLoadFromLayout call site (resref at [EBP+8]); 0x40EE40 has ~100 callers.
- LRU helpers 0x54AA10 / 0x64DDE0 are unnamed; behaviour inferred from call pattern.
- 0x712360 mode 1/2 filter semantics for ERF type (inferred).
- Live probe needed for route A: view rect vs file EXTENT, InitControl ID, P+0x28 count.
- InitializeForWriting 0x717AA0 (in-memory GFF writer) not evaluated; build-time bytes recommended.

## FEASIBILITY summary
- Memory GFF parse without a file (section 1): **HIGH**.
- Registered memory resource or lookup injection (section 2): **LOW**.
- Route A, temporary handle for hse_view (section 4): **HIGH** mechanically, **MEDIUM** until live.
- Route B, in-memory equip_p replacement (section 5): **MEDIUM** (call site and precedence decision open).
