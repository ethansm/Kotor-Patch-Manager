# REA recon: 0x007356B0 (CExoLocString-style GetString), swkotor2.exe Steam 6a522e71

Binary: `/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe` (REA open_binary sha256 6a522e71...ffef, ghidra 12.1.4, x86 PE). REA session closed.
Sources: REA live (procedure_info, procedure_assembly, search_procedures, find_xrefs_to_name, read_bytes, procedure_info 0x733780, xrefs 0xa1b4b0/0xa1b4a4 (too large, not used)). Static dumps (`ghidra_dumps/steam_6a522e71/decomp/*.c|*.asm`, `strings.tsv`, `inputs/name_map.tsv`) read with Read/Grep. Ledger consulted before open: no prior rows for 0x7356b0 except static `steam_functions.tsv` line 7013 (`FUN_007356b0`, size 311, RET 12).

## OUTPUT

### Q1. Calling convention, parameters, return
- Convention: `__thiscall`. ECX = `this` (a CExoLocString-shaped handle). Evidence: asm `0x7356d8: MOV dword ptr [EBP + -0x40],ECX` (REA procedure_assembly 0x7356b0).
- Return: `RET 0xC` (3 stack args). Evidence: REA procedure_assembly `0x7357e4: RET 0xc`.
- Stack params:
  - `[EBP+8]` int `param_2`: language-like id. Cache key = `2*param_2 + (param_2 ? gender : 0)` (asm 0x7356f7-0x735701). Label "language" is probable, not confirmed.
  - `[EBP+0xC]` undefined4 `param_3`: `CExoString*` out-param (8 bytes: `char*` at +0, buffer size at +4; dump 0x7337c0 lines 25-29).
  - `[EBP+0x10]` byte `param_4`: gender, normalized to `(param_2 ? param_4 : 0)` at 0x7356f4.
- `this` layout: `[this+0]` = pointer to cache container (dereferenced first, asm 0x735872 and 0x73579a); `[this+4]` = StrRef (asm 0x735755 `MOV ECX,[EAX+4]`).
- Return type: **bool in EAX**, not a CExoString, not `char*`, no hidden sret pointer (RET 0xC pops only 3 slots). EAX=1 on cache hit (asm 0x735718 `MOV EAX,1`). On the miss path EAX=`[EBP-0x38]` = 0/1 (asm 0x7357c9). Ghidra's prototype is missing (REA `undefined FUN_007356b0()`) and the dump's `void` is wrong.
- Language/talk-table dependence: yes, through a global.
  - Cache hit: `FUN_0073b470` (see Q3 notes).
  - Miss: asm 0x735740 `CMP dword ptr [0x00a1b4b0],0`. If the talk manager is null, return 0.
  - Otherwise asm 0x73575f: `CALL 0x0071df90` with ECX = `[0x00a1b4b0]`, args (`[this+4]` StrRef, `&tmp CExoString`, gender byte).
  - `param_2` (language) is NOT passed to the fetch. The text source is whatever talk table is in `DAT_00a1b4b0`.
  - `FUN_0071df90` writes gender into the manager at `+0x3C` (dump 0071df90.c line 11) and calls `FUN_0071d970` ("Invalid STRREF %d passed to Fetch", dump 0071d970.c line 53).
  - Empty-string sentinel `DAT_00986925` = "" (REA read_bytes 0x986925: `00000058999b0080`).

### Q2. Simpler accessor and item name
- No simpler item-name getter found. The local DB (`inputs/name_map.tsv`, grep) lists CSWSItem Icon/Description/Properties getters only, no `GetName`/`GetDisplayName`. REA `search_procedures("GetString")` returned only CRT/GL externals.
- Item name is a CExoLocString at **item+0x2C0**: CONFIRMED by `CSWSItem::LoadDataFromGff` (`0x601740`, DB name):
  - asm `0x6019c4: PUSH 0x99754c` (`"LocalizedName"`), read via `CALL 0x7190e0` at 0x6019d4.
  - asm `0x6019ff: ADD ECX,0x2c0` then `0x601a05: CALL 0x00735400` (copy into item+0x2C0).
  - Same loader: `+0x2B8` = `"Description"` (0x996c40, copy at 0x601904/0x60190a); `+0x2B0` = `"DescIdentified"` (0x99755c, copy at 0x601999/0x60199f).
- Callers that read item-related loc-strings (static asm):
  1. `0x607790` CSWSItem::GetPropertyDescription (DB name): `this` from `[EBP-0x74]` (asm 0x6077b5), reads **this+0x2B8 (Description, not the name)**. Sequence at 0x60796b-0x60797b.
  2. `0x8b1250` CSWGuiInGameItemEntry::SetItem (DB name): asm `0x8b1354: ADD ECX,0x2c0` on `[EBP-0x2C]`, which is `*(DAT_00a1b4a4+8)` (decomp 008b1250.c line 85), not the item (item is `local_30`). Probable mismatch. Not an item-name read.
  3. `0x8bf280` FUN_008bf280 (BaseItem and LocalizedName loader, class unconfirmed): reads `LocalizedName` into a temp handle `[EBP-0x58]` (asm 0x8bf507 PUSH 0x99754c; 0x8bf517 CALL 0x7190e0; 0x8bf510 LEA EAX,[EBP-0x58]), then calls 0x7356b0 on it. Exact sequence:

```
8bf52b  MOV ECX,[0x00a1b4a4]     ; root object
8bf531  MOV EDX,[ECX+4]          ; p
8bf534  MOV [EBP-0x11c],EDX
8bf53a  PUSH 0x0                 ; param_4 gender = 0
8bf53c  LEA EAX,[EBP-0x60]; PUSH EAX   ; param_3 = out CExoString*
8bf540  MOV ECX,[EBP-0x11c]
8bf546  CALL 0x0073f350          ; fastcall GetClientLanguage -> EAX = *(*(p+4)+0x148)
8bf54b  PUSH EAX                 ; param_2 = language
8bf54c  LEA ECX,[EBP-0x58]       ; this = LocalizedName handle
8bf54f  CALL 0x007356b0          ; bool in EAX
```
- The out buffer `[EBP-0x60]` is referenced again at 0x8bf58a. Its destructor call is not traced (OPEN).

### Q3. Memory ownership
- Destructor: `0x00733780` (DB name `CExoString::~CExoString`). REA procedure_info: body `0x733780-0x7337bb`, 60 bytes, not a thunk. Body (dump 00733780.c): `if (*p) { p[1]=0; FUN_009196fd(*p); *p=0; }`. `0x9196fd` = operator delete / `_free` (RE_AGENT_GUIDE section 4, DB row).
- Out-param ownership: caller-owned. 0x7356b0 writes it via `CExoString::operator=` (0x7337c0: free old via 0x9196fd, alloc via 0x919723, copy via 0x91cff0). After a successful fetch the out buffer is heap memory and the caller must destroy it with 0x733780.
- Internal temp `[EBP-0x34]` is destroyed by `CALL 0x00733780` at 0x7357c4 (asm).
- Empty fetched string: `0x733b60(out,"")` is false, so no cache insert. Still returns 1 (asm 0x735783 JZ -> 0x7357a9).

## EVIDENCE
- REA: procedure_info 0x7356b0 (name FUN_007356b0, body 0x7356b0-0x7357e6, 311 B, no prototype applied).
- REA: procedure_assembly 0x7356b0 (full listing, RET 0xc, MOV EAX,1 at 0x735718).
- REA: find_xrefs_to_name "CExoLocString::GetString_3" -> name_not_found.
- REA: read_bytes 0x986925 -> `00000058999b0080`.
- REA: procedure_info 0x733780 (body 60 B).
- Static: 0x735860 (`MOV ECX,[EDX]` then `CALL 0x73b470`, RET 8); 0x73b470 (list find, `CALL 0x7337c0` on hit, returns 1/0); 0x73aed0 (`*local_2c = param_2`, count+1).
- Static: 0x71df90 and 0x71d970 (TLK fetch, "Invalid STRREF" log string).
- Static: 0x73f350 body `return *(undefined4 *)(*(int *)(param_1 + 4) + 0x148);`.
- DB names (not verified by REA): 0x7356b0 `CExoLocString::GetString_3` (medium, R9-Phase2); 0x735860 `CExoLocString::GetStringLoc_2` (vote); 0x73b470 `CExoLocStringInternal::GetString_2`; 0x73aed0 `CExoLocStringInternal::AddString`; 0x735360 `CExoLocString::Destructor`; 0x73f350 `CClientExoApp::GetClientLanguage` (high).

## FINDING
- 0x7356b0 is a `__thiscall` bool lookup, with `CExoString* out`, a gender byte, and a language-like key. It is the CExoLocString cache-plus-TLK-fetch path.
- Item name storage: `item+0x2C0` = LocalizedName (confirmed by loader asm). `item+0x2B8` = Description.
- Ghidra knows this function only as `FUN_007356b0`. The name `CExoLocString::GetString_3` is a DB-only name, REA `name_not_found`. The `_3` overload index is unverified.
- The brief's claim "item+0x2C0 is a CExoLocString" is CONFIRMED for the loader. It is NOT confirmed for SetItem (which uses another base).

## OPEN
1. `FUN_0071d970` is declared `void` (dump) and `FUN_0071df90` returns its EAX. Meaning of that success value is unresolved.
2. Which code initializes `DAT_00a1b4b0` (talk manager) and `DAT_00a1b4a4` (root object). Live xrefs returned about 146 KB and were not parsed.
3. Null safety: `[this+0]` is dereferenced at the very start (0x735872). A handle whose server pointer is null likely crashes. Probable, not tested.
4. Whether `item+0x2C0` is always non-null for real items.
5. Cache invalidation on language or talk-table change. Nothing in this path clears it. Unknown whether the container's `+8` dirty flag is consumed elsewhere.
6. Ownership across the handle copy: 0x735400 calls `0x73aff0` on the server pointer, and 0x735360 frees `[this]` with no refcount. Unresolved; the FID label "CAtlWinModule" on 0x735360 looks spurious.
7. Object type behind `obj+0x2C0` in callers 0x8de2e0 and 0x8ce3f0 (not an item-name read as far as traced).
8. Destructor call for the out temp at `[EBP-0x60]` in 0x8bf280.

## VERDICT
**UNSURE** for "0x7356B0 is safely bindable as a C++ call from a patch to get an item name."

Call ABI is confirmed (`__thiscall`, 3 stack args, `RET 0xC`, bool in EAX, `CExoString*` out). Item storage is confirmed (`item+0x2C0` = LocalizedName). But binding is not safe yet:
- it must receive the correct language value from the `GetClientLanguage` chain;
- it dereferences the handle's server pointer first, with no null check;
- it has side effects: cache insert, container `+8` dirty flag, and a gender write into the talk manager at `+0x3C`;
- nothing was exercised live.

Next step if the user wants it: a live test call on one item with a known name, after checking that `item+0x2C0` is non-null.
