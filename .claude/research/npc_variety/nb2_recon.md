# NB2 recon: server creature to client Gob, idle test, busy test (Steam swkotor2.exe 6a522e71)

Date: 2026-10-16. Binary: `/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe` (sha256 prefix 6a522e71, confirmed by REA open_binary). Read-only.
Sources: static dumps `ghidra_dumps/steam_6a522e71/` (decomp `.c`/`.asm`, `callgraph.tsv`) first. REA used for 2 calls only: `read_bytes 0x00994470 len 32` and `read_bytes 0x0099ee14 len 64`. REA session closed (`close_binary`, no snapshot).
Ledger: `patch_manager_mods/ghidra_knowledge/rea_findings/20261016_nb2-recon.tsv`.
Ledger reuse (grep before open): `20261016_holster-c5-weapon-identity.tsv` row 0x0073f530 (wrapper, confirmed, reused); `20261015_B13a-q4_2.tsv` row 0x00a1b4a4 (global, reused). `steam_functions.tsv` lists 0x004e3270, 0x00568bd0, 0x0073f4d0, 0x0073f530 by FUN_ name only. Names in `gog_map_mods678.tsv` are votes, not confirmations.

## Summary

| Q | Verdict | One-line answer |
|---|---|---|
| Q1 | CONFIRMED (this = [[G]+4]); type check PARTIAL | `GetGameObject` 0x0073F4D0 is thiscall(oid), returns node object or 0, no type check. `this` = `[[0xA1B4A4]+4]` at its one verified caller. The type-checked variant is 0x0073F530 to 0x0078BD80. |
| Q2 | UNSURE | Client key is `serverOid | 0x80000000` (0x798160). Insert path cannot populate the high-bit buckets the lookup reads, so client==server id is unproven and a live check is needed. |
| Q3 | PARTIAL | `+0x84` word = current action type, 0xFFFF = none. Action list head at `+0x100`. No count word found. `+0x78` only read/copied. |
| Q4 | PARTIAL | Gob anim list `+0xDC`, count `+0xE0`. Flags `+0x8` bits 0x28 gate "current". Client `+0x48` word has no confirmed meaning. No timer found. |
| Q5 | PARTIAL | Combat: creature `+0x520` non-zero. Conversation: `+0x54` partner oid (reader is probable). |

## Q1. GetGameObject (0x0073F4D0), this, convention, missing oid

- Convention: thiscall. `ECX = this`, one stack arg (oid), `RET 4` (asm 0x0073f508).
- Body (asm 0x0073f4d0 to 0x0073f508): `H = [this+4]`; `table = [H+0x14]`; calls `0x0053DFB0(table, oid, &out)` with `out` initialised to 0 (asm 0x0073f4e2). Returns `out`.
- `this` = `[[0xA1B4A4]+4]`: CONFIRMED at the caller CSWSObject::GetClientObject 0x0054A840. asm 0x0054a878 `MOV EAX,[0x00a1b4a4]`, 0x0054a87d `MOV ECX,[EAX+4]`, 0x0054a88a `CALL 0x0073f4d0`.
- Same `this` chain as the item lookup: 0x0073F530 passes `ECX=[this+4]` to 0x0078BD80 (asm 0x0073f53e, 0x0073f541), which reads `table=[H+0x14]` (asm 0x0078bd93).
- Missing oid: `out` stays 0 (0x0053dfb0 writes 0 when not found, asm 0x0053e00f to 0x0053e018 and 0x0053e046). Returns NULL.
- Type validation: NONE in 0x0073F4D0 (the 0x53DFB0 return code is ignored). 0x0073F530 to 0x0078BD80 DOES validate: found AND `byte[obj+0x8] == DAT_00994475` (=6, confirmed bytes), then `vt+0x34(H, table)`, else 0.
- Freed object: not resolved. CSWCCreature dtor 0x007651B0 calls 0x00740A50 to 0x0078CF50 (not decoded). Whether this unlinks the table node is UNKNOWN.
- `G` layout: `G+0x4` = client app (used as GetGameObject `this`). `G+0x8` = object manager passed to 0x0051C0A0 (asm 0x0068b843, 0x0068b850). Server/client meaning of `G+0x8` is probable.

## Q2. Client object id vs server oid

Server to client path (CONFIRMED by asm):
1. Server CSWSObject `S`. `GetClientObject(S)` = 0x0054A840 (fastcall, ECX=S). Reads `oid = S+0x04` (asm 0x0054a858).
2. `key = 0x0073FEC0(oid)` gives `0x00798160(oid)`: `oid == 0x7F000000 ? oid : oid | 0x80000000` (asm 0x00798167 to 0x00798173).
3. `obj = 0x0073F4D0(key)` (raw lookup).
4. `cache = obj->vt[+0x0C](obj)`. Slot +0x0C = 0x00458940 = `return this` (asm 0x00458947). Result stored at `S+0x264` (asm 0x0054a8a8). Call uses ECX only (asm 0x0054a898 to 0x0054a8a3).

Hash table layout (CONFIRMED by asm):
- Lookup 0x0053DFB0: bucket = `(bit31(key) << 12) + (key & 0xFFF)`; compare `node.key == key & 0x7FFFFFFF` (asm 0x0053dfc4 to 0x0053dff8, 0x0053e027).
- Insert 0x0053DD20: bucket = `key & 0xFFF` only (asm 0x0053dd77 `AND ECX,0xFFF`). Id from counter `table+0x4` (ascending, flag 0) or `table+0xC` (descending, flag nonzero) (asm 0x0053dd63 to 0x0053dd6f).
- Result: buckets 0x1000 to 0x1FFF (bit31 keys) are NOT written by 0x0053DD20. Callers found: 0x00806960 (client object-add handler), 0x00829850, 0x0053E830 (CSWSObject ctor), 0x00521360, 0x00556670, 0x0086D770, 0x008E54E0.
- So a `key|0x80000000` lookup never hits an object inserted by 0x0053DD20. Either another writer exists (not in the dumps) or `GetClientObject` returns NULL for normal creatures. This is the blocker for Q2 and for the injector path.

Client object-add handler 0x00806960 (name unknown; its oids come from a server message via 0x008935A0):
- `oid == 0x7F000000`: new CSWCCreature via 0x007645F0 (ctor, sets vftable 0x0099EE14, asm 0x00764635). Id = counter from 0x0053DD20, then `vt+0x04(id)`. Not the server oid.
- other `oid`: lookup `0x0073F4D0(oid)` with the RAW message oid (no bit31), callgraph 0x00806b23.
- So the client creation path does not obviously use the server oid, and the lookup path uses the raw oid while `GetClientObject` uses bit31. Equality of client and server id: UNSURE.

## Q3. Server idle test (CSWSObject, server creature)

| Field | Evidence | Status |
|---|---|---|
| `+0x84` word, current action type (ActionId) | RunActions 0x005D56B0: store `MOV word [EDX+0x84],CX` (asm 0x005d5bed); reset `MOV EDX,0xFFFF; MOV word [EAX+0x84],DX` (asm 0x005d6a30 to 0x005d6a3b) | CONFIRMED. 0xFFFF = none, as the brief said. Initial value at construction NOT read. |
| `+0x20C` current action element ptr | set asm 0x005d5bd3, cleared asm 0x005d69ea | CONFIRMED |
| `+0x100` action list container (CExoLinkedList) | SaveActionQueue 0x00540410: `ADD ECX,0x100; CALL 0x007A1720` (asm 0x0054043b, 0x00540441) | CONFIRMED |
| container first dword = head node | 0x007A1720 `MOV ECX,[EAX]; MOV EAX,[EDX]` (asm 0x007a172c, 0x007a1734) | CONFIRMED |
| empty test | 0x007A1780 `CMP dword [EDX],0; SETZ AL` (asm 0x007a1796, 0x007a1799) | CONFIRMED: empty = `[this+0x100] == 0` |
| node layout | 0x0073A050 GetNext: `node+0x4` = next, `node+0x8` = data (asm 0x0073a0a0, 0x0073a0aa) | CONFIRMED |
| element layout | SaveActionQueue: ActionId (dword, element+0), GroupActionId (short, +0x6C), NumParams (short, +0x6E), params from +0x04 (decompile) | CONFIRMED for +0x6C/+0x6E (decompile line refs); params probable |
| count field | NOT FOUND. The list container only has the head at `+0x100`. | UNKNOWN |
| `+0x78` aiLevel | ExecuteCommandSetCreatureAILevel 0x0066B640: `MOV ECX,[EAX+0x78]`, `MOV byte [EAX+0x7C],CL` (asm 0x0066b6d7, 0x0066b6e9). Writer of `+0x78` not isolated. | PARTIAL: read and copy confirmed, write location unknown |

Stock "no action" code: the script command `Global::ExecuteCommandGetCurrentAction` 0x0067B100 (vote name) does NOT read `+0x84`. It uses the combat round and action helpers (0x005D7190, 0x005D7220). So the simplest injector idle test is the raw fields above, not that command.

Working idle test (inference, not validated in game): `[S+0x84] == 0xFFFF && [S+0x100] == 0`. Initial `+0x84` value unverified.

## Q4. Client busy state

- Gob animation list (CONFIRMED): `Gob+0xDC` = array of entry pointers, `Gob+0xE0` = count (asm 0x004e31dc `CMP EDX,[ECX+0xE0]`, 0x004e31e7 `MOV ECX,[EAX+0xDC]`).
- Entry layout from PlayAnimation 0x004E3270 (decompile; probable): `entry[0]` = anim descriptor (name at `+8`), `entry[1]` = speed (stored as `(int)param_3`, caveat), `entry[2]` = flags, `entry[4]` = remaining time, `entry[7]` = speed product from 0x004E3230.
- Flags `+0x8` (probable): 0x004E31C0 returns the first entry whose `(flags & 0x20) == 0` and `(flags & 0x08) == 0`. Asm reads entry `+0x8` at 0x004e31f3.
- Busy test candidate: `Gob+0xE0 > 0` with at least one entry where `(flags & 0x28) == 0`. Meaning of bits 0x08/0x20 not decoded (idle or loop unknown).
- Controller at `Gob+0x1A0`: its `+0xC` points at the entry it drives (decompile 0x004E3270, probable).
- Client `CSWCObject+0x48` word: read by `CSWCObject::GetCurrentAnimation` 0x007ED7D0 (asm 0x007ed7da `MOV AX,[EAX+0x48]`). Name is a vote. Writer not found. Meaning UNKNOWN.
- `CClientExoApp::GetAnimationTimer` 0x0073F3F0 (vote name) returns `[[this+4]+0x30]`. Not used as an animation timer. Not claimed.
- Client vtable (CONFIRMED): slots +0x0C and +0x2C = identity 0x00458940. Slots +0x10..+0x28 and +0x30..+0x3C = 0x004558A0 (`return 0`). No busy getter found in the vtable.

## Q5. Server-side combat and conversation

- Combat (CONFIRMED): `CSWSCreature+0x520` non-zero = in combat. Written by 0x0057A560 (vote name SetCombatState): `MOV [EDX+0x520],EAX` (asm 0x0057a634), `+0x524 = 0x1F40` when set (asm 0x0057a64f). Read by 0x00680000 (vote name ExecuteCommandGetIsInCombat): `MOV EAX,[EDX+0x520]` (asm 0x00680078), with a `+0x11E8` gate.
- Conversation (CONFIRMED writer, PROBABLE reader): `CSWSObject+0x54` = dialog partner oid. Writer 0x00546DB0 (vote SetDialogOwner): `MOV [EAX+0x54],ECX` (asm 0x00546dbd). Reader 0x0068B810 (vote ExecuteCommandGetIsInConversation): reads `obj+0x54` (asm 0x0068b88a), resolves partner through 0x0051C0A0 (asm 0x0068b850). Final mutual-link compare is ambiguous in the decompile.
- Which object gets the owner oid in StartDialog 0x00545760 is UNKNOWN (ECX not visible in decompile).
- "None" value for `+0x54` not confirmed. `0x7F000000` is the invalid oid, which the injector should treat as none (assumption).

## Constants (REA read_bytes 0x00994470, len 32, CONFIRMED)

| Address | Value | Meaning |
|---|---|---|
| 0x00994474 | 05 | probable creature type byte (checked by 0x0066B640) |
| 0x00994475 | 06 | type byte checked by 0x0078BD80 (name unknown) |
| 0x00994478 | 09 | checked with 0x994474/79 in 0x00806960 (probable, not named) |
| 0x00994479 | 0A | same as above |
| 0x0099447F | 00 | BOOL "found" (0x0053DFB0 returns AL=0 on found) |
| 0x00994484 | 0x7F000000 (dword) | invalid/none oid |

## CSWCCreature vtable 0x0099EE14 (REA read_bytes len 64, CONFIRMED)

Dwords 0 to 3: `0x00765180`, `0x00764F60`, `0x00766490`, `0x00458940`. Dwords 4 to 10: `0x004558A0`. Dword 11 (+0x2C): `0x00458940`. Dwords 12 to 15: `0x004558A0`. Slot 11 is read by hand from hex segmentation; re-check with a REA procedure lookup before relying on it.

## Other finding (not asked)

- `0x00568BD0` (brief: "CSWSCreature::AddToArea", vote name) has an AREA-shaped `this`: list `+0x148`, object array `+0x364`/count `+0x368`, `+0x1198`, `+0x350`. Param 2 is the added object. The name looks wrong. Treat the hook's `this` as the area, not the creature (probable).

## Unverified / caveats

1. High-bit bucket gap (Q2): see the hash table layout above. A live check is needed: `GetClientObject(S)` for a normal creature, expecting non-NULL.
2. `0x0073F530` caller `this` not checked at a real caller (brief says `[[0xA1B4A4]+4]`).
3. Freed-object removal path (0x00740A50 to 0x0078CF50) not decoded.
4. Type byte 6 semantic name unknown; byte 5 creature is probable.
5. Gob entry flags meaning (0x08, 0x20, 0x200, 0x400) not decoded.
6. Action count field not found.
7. `+0x84` initial value at construction not read.
8. Vtable slot 11 position by hand from hex.
9. Function names are vote-only unless noted. Only body behaviour is confirmed.

## Injector implications (inference)

- Entry: `C = GetClientObject(S)` (0x0054A840, ECX=S). Blocked until the high-bit lookup is verified live (Q2).
- Body Gob: holder chain not verified here. The brief's `creature+0x68` / `vt[8]` claim is not checked in this recon.
- Skip if `[S+0x84] != 0xFFFF` or `[S+0x100] != 0` (Q3), or `[S+0x520] != 0` (combat, Q5), or `[S+0x54]` not 0x7F000000 (conversation, Q5, assumption).
- Skip if the Gob busy test in Q4 is true.
