# r10: client party member -> creature first name (Steam exe 6a522e71)

Dumps: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/<bb>/<addr>.{asm,c}` (asm is authoritative; `.c` is a hint).

## Chain (C = `party::member(t,N)`)

1. **C** = `PartyGetAt` 0x007E5DA0, thiscall(table, idx) RET 4 (`_shared_src/Party.h:80-84`, `GameAddr.h:34`). C is a client object.
2. **S** = `serverCreature(C)` = 0x0077D800 (thiscall ECX=C, no args). Body (`decomp/77/0077d800.asm`):
   - calls 0x007F2540 (`CSWCObject::GetServerObject`, ECX=C). It returns cache `[C+0xF8]`; on a miss it returns 0 if `[C+0xE4]!=0`, else `ClientToServerObjectId` 0x51C8B0 on `[C+4]`, then `GetGameObject` 0x51C0A0, then vt+0x10 (`decomp/7f/007f2540.asm`).
   - then `vt+0x30` on that object, no args (asm 0x0077D82B `CALL EAX`). For the CSWSCreature vtable 0x994F5C, +0x30 = 0x458940, an identity thunk (`decomp/45/00458940.asm` 0x458947 `MOV EAX,[EBP-4]`). So S = CSWSCreature*. Source: `findings/AI7.md`.
   - Shortcut: `Vitals.server` from `members()` in `Party.h` already holds S per party position.
3. **F** = `CSWSCreature::GetFirstName` 0x005632F0 (thiscall ECX=S, RET 0; DB row B12a verified, GOG 0x0067BF20). Asm (`decomp/56/005632f0.asm`):
   - `CMP [S+0x1198],0` (0x5632FA). If 0 it returns `S+0xC`. Otherwise it returns `stats+0x34` (0x56330E ADD 0x34).
   - `0x1198` = stats pointer (`K2Addr.h:315` Creature_Stats).
   - `stats+0x34` = first-name CExoLocString. `stats+0x3C` = last name. `CSWSCreatureStats::GetFullName` 0x006A4C30 passes `in_ECX+0x34` and `+0x3C` to 0x633620 (`steam_docs/CSWSCreatureStats.md:148-190`).
   - `S+0xC` is only the stats-less fallback. Its content is not decoded here. Do not use it.
4. **H = F**. CExoLocString is 8 bytes: `[H+0]` = internal cache object pointer, `[H+4]` = int32 TLK strref (-1 = none). Evidence: `decomp/73/007356b0.asm` 0x73575F pushes `[EAX+4]` (strref); 0x735792 uses ECX=`[H]`.
5. **L** = client language: `GetClientLanguage` 0x0073F350, fastcall ECX=X where X = `[[0xA1B4A4]+4]`. Returns `[X+4]+0x148` (`decomp/73/0073f350.c`). Same read as `InventorySortFilter.cpp:155`.
6. **GetString** = `CExoLocString::GetString` 0x007356B0. Thiscall ECX=H, stack `(int language, CExoString* out, byte gender)`, RET 0xC, returns bool (REA row, `GameAddr.h:97`).
   - key = `(L==0 ? 0 : gender) + 2*L`. Cache lookup 0x735860 (ECX=H). On a hit, `out` is filled and the TLK is skipped.
   - On a miss, if the TLK global `[0x00A1B4B0]` is non-null: 0x71DF90 is called with ECX=`[0xA1B4B0]` (the TLK object) and args `(strref=[H+4], tmp, gender)`. On success it copies into `out`, writes `[[H]+8]=1`, and caches via 0x73AED0 (ECX=`[H]`).
   - **TLK**: yes, a TLK lookup is needed, but GetString does it internally. The caller makes no TLK call.
   - `out` = CExoString `{char* p; int len}`. It is written only on success, so init to `{0,0}`. Free with ExoDtor 0x00733780 (thiscall).
7. Read `out.p` (<=255 chars, saferead), then ExoDtor.

Short form: `C -> [C+0xF8] or 0x7F2540 -> vt+0x30 (identity) = S -> [S+0x1198] = stats -> F = stats+0x34 -> GetString(F, L, &out, 0) -> out.p`

## Safety: which reads to saferead

- **C**: non-zero (`member()` returns 0 on failure). 0x7F2540 reads `[C+0xE4]`, `[C+0xF8]` and `[C+4]` unguarded.
- **S**: non-zero. Check with saferead: `[S+8]` byte == 5 (objectType creature, `CSWSObject` struct), and `[S+0]` == 0x994F5C. Then call 0x5632F0, which reads `[S+0x1198]` unguarded.
- **stats** = saferead `u32 [S+0x1198]`, must be non-zero. Prefer computing `F = stats+0x34` in mod code over calling 0x5632F0 with stats==0.
- **H=F**: saferead `[H+0]` (non-zero) and `[H+4]` (strref). Also saferead `[[H+0]+8]`, because GetString writes it on the TLK-success path.
- **L**: saferead `[AppGlobal]`, then X = `[..+4]` non-zero, then `[X+4]` non-zero. GetClientLanguage reads `[X+4]+0x148` unguarded.
- **out**: saferead `p` and `len` (cap at 255). Call ExoDtor only if `p != 0`.
- **Side effects**: the first 0x7F2540 call caches `[C+0xF8]` (already done by `Party.h`). GetString caches into H's internal object and writes `[[H]+8]=1` on success. Do not call it on a stale or garbage handle.

## Existing mod code

- No mod reads a creature first name. The only name read is item name: `inventory_sort_filter_src/InventorySortFilter.cpp:148-170` (`readItemName`, GetString on item+0x2C0). Copy its pattern: language via 0x73F350, `out` init `{0,0}`, ExoDtor after.
- Cross-version (not decoded here): GOG PartyGetAt 0x4789D0, GOG MemberServerCreature 0x429780, GOG GetFirstName 0x67BF20 (`K2Addr.h:426-427`).

## Open

- A PC with strref -1 (custom name) resolves only if its cache entry matches `key`. Not verified statically. Confirm with a live log.

ANSWER: FOUND C -> 0x77D800 (serverCreature) -> 0x5632F0 (GetFirstName, returns stats+0x34 CExoLocString) -> 0x7356B0 GetString(lang, &out, 0) -> out.p (TLK resolved inside GetString)
