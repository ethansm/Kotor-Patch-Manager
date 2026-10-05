# RE: client queue panel (0x74DC80) and icon resolver (0x74E0B0), Steam swkotor2.exe sha256 6a522e71 (unpatched_vanilla)

Static only. Asm from `ghidra_dumps/steam_6a522e71/decomp`. Jump tables and strings read from the exe with `scripts/disasm_va.py steam VA --bytes N` (same file). "UNCONFIRMED" = not proven by quoted asm.
Names in [brackets] come from query_ghidra (DB) and are hypotheses; the quoted asm is the evidence.

## 1. CExoLinkedList walk on the creature queue (+0x100)

CONFIRMED: `creature+0x100` is an INLINE list object whose first dword points to an internal struct whose first dword is the head node. Not a pointer to a heap list object.
```
0074dda9  8b4d80    MOV ECX,dword ptr [EBP + -0x80]        ; ECX = server creature
0074ddac  81c100010000 ADD ECX,0x100                      ; ECX = &creature->queueList (the address, not a load)
0074ddb2  894d88    MOV dword ptr [EBP + -0x78],ECX
0074ddb8  e863390500 CALL 0x007a1720                       ; GetHeadPosition(this = &creature+0x100)
```
0x7A1720 (thiscall, no stack args, plain RET):
```
007a172c  8b08      MOV ECX,dword ptr [EAX]               ; internal = [this+0]
007a1731  8b55fc    MOV EDX,dword ptr [EBP + -0x4]
007a1734  8b02      MOV EAX,dword ptr [EDX]               ; head = [internal+0]
007a173f  c3        RET
```
So list object = `{ Internal* @0 }`, Internal = `{ head @0 ... }`. Matches Common header (`CExoLinkedListInternal{head@0,tail@4,count@8}`). tail@4 and count@8 are NOT touched by any function in this batch: UNCONFIRMED from these asm (taken from the Common header only).

Node layout: next @4 and data @8 CONFIRMED; prev @0 UNCONFIRMED here (no function in this batch reads +0; header says prev@0).
GetAt (0x561430, `RET 4`, ECX = list object) -> 0x73A010 (ignores ECX), returns node data:
```
00561438..00561440  MOV ECX,[ECX] ; CALL 0x0073a010          ; ECX = internal (ignored by callee)
0073a040  8b4d08    MOV ECX,dword ptr [EBP + 0x8]           ; pos (node ptr)
0073a040+3 8b4108   MOV EAX,dword ptr [ECX + 0x8]           ; return node->data  (+0x8)
0073a017  837d0800  CMP dword ptr [EBP + 0x8],0x0 ; JNZ ...  ; pos==0 -> logs, returns 0
```
GetNext (0x58C370, `RET 4`, ECX = list object, arg = `Node**`) -> 0x73A050: ADVANCES pos to next, returns the NEW node's data (not the old one):
```
0073a085  83780400  CMP dword ptr [EAX + 0x4],0x0           ; node->next == 0 ?
0073a08e  c70100000000 MOV dword ptr [ECX],0x0              ; yes: *pos = 0 ; return 0
0073a0a0  8b5004    MOV EDX,dword ptr [EAX + 0x4]
0073a0a3  8911      MOV dword ptr [ECX],EDX                 ; *pos = node->next
0073a0aa  8b4108    MOV EAX,dword ptr [ECX + 0x8]           ; return (*pos)->data
```
The panel ignores GetNext's return value and re-reads via GetAt each iteration:
```
0074ddde  8b4d88 MOV ECX,[EBP-0x78] ; push pos(-0x24) ; 0074dde2 CALL 0x00561430   ; action = GetAt(&creature+0x100, pos)
0074e01f  8d55dc LEA EDX,[EBP + -0x24] ; PUSH EDX ; MOV ECX,[EBP-0x78] ; 0074e026 CALL 0x0058c370 ; GetNext(&pos)
0074e02b  e997fdffff JMP 0x0074ddc7                                                        ; loop test: pos != 0
```
Loop guards: `0074ddc7 CMP [EBP-0x24],0 / JZ 0x0074e030` (pos==0 ends) and `0074ddd1 CMP [EBP-0x30],4 / JGE 0x0074e030` (slot counter -0x30 >= 4 ends).

The round list at `round+0xB00` is a POINTER to a list object (double indirection), unlike +0x100:
```
0074deec  8b45a8 MOV EAX,[EBP-0x58]                 ; round
0074deef  8b88000b0000 MOV ECX,dword ptr [EAX + 0xb00]   ; ECX = list OBJECT ptr (heap), tested for 0
0074df1d  e8fe370500 CALL 0x007a1720                     ; GetHeadPosition(this = [round+0xb00])
0074df5c  e8cf34e1ff CALL 0x00561430                     ; GetAt(this = [round+0xb00], pos)
0074dfbd  e8aee3e3ff CALL 0x0058c370                     ; GetNext(this = [round+0xb00], &pos(-0x64))
```
(ECX = `[round+0xb00]`, so 0x7A1720 does `[[round+0xb00]]` = internal, `[internal]` = head.) The pointer at +0xB00 may be NULL (checked: `0074defb CMP [..ff70],0 / JZ 0x0074e01f`).

## 2. 0x74E0B0 (icon resolver)

Convention: `__thiscall`-looking, 6 stack args, `RET 0x18` (`0074e734 c21800 RET 0x18`). ECX is saved to `[EBP+0xfffffd00]` (`0074e0b9`) and never read again (only that one occurrence in the file): ECX is IGNORED. Callers pass the HUD (`0074de66 MOV ECX,[EBP+0xffffff5c]`, the entry ECX of 0x74DC80).
Args (cdecl-order in [EBP+N]); the caller pushes right-to-left (0074de4b..0074de6c):

| arg | [EBP+] | caller push (case 0074de4b) | meaning | used in callee? |
|---|---|---|---|---|
| 1 | +0x08 | `0074de62 MOV EDX,[EBP-0x58] / PUSH` = combat round (= `[creature+0x10DC]`) | round | NO (no `[EBP+0x8]` read anywhere in 0x74E0B0) |
| 2 | +0x0C | `0074de5e MOV ECX,[EBP-0x54]` = queue action (data of the queue node) | action; `[arg2]` = type dword; `[arg2+0x38]` = param | YES |
| 3 | +0x10 | `0074de5a MOV EAX,[EBP-0x3c]` = round action node (`[round+0xB18]` or an item of the `[round+0xB00]` list) | round node: byte `[+0x10]` = sub-type, `[+0x24]` spell id, `[+0x5C]` feat id, `[+0x64]` item object id | YES |
| 4 | +0x14 | `0074de53 LEA EDX,[EBP+0xffffff7c]` = &int (init -1, or &[EBP-0x44] init 0) | out int*: UNUSED (no `[EBP+0x14]` read in the file) | NO |
| 5 | +0x18 | `0074de4f LEA ECX,[EBP+-0x74]` | `CResRef*` OUT 1 (16 bytes) = icon | writes |
| 6 | +0x1C | `0074de4b LEA EAX,[EBP+-0x20]` | `CResRef*` OUT 2 (16 bytes) = "m" (mask-ish) variant | writes |

Return value: none used (EAX not read by callers); function returns nothing meaningful. Both outs are first reset to blank:
```
0074e0e2  6825699800 PUSH 0x986925     ; "" (empty C string, byte 00 at 0x986925)
0074e0ee  e8bd2dfcff CALL 0x00710eb0     ; ECX=[EBP+0x18]: out1 = CResRef("")
0074e0ff  e8ac2dfcff CALL 0x00710eb0     ; ECX=[EBP+0x1C]: out2 = CResRef("")
```
Helper semantics (all CONFIRMED from asm):
- 0x710EB0 (`RET 8`, ECX=dest `CResRef*`, args `(CResRef* retCopy, const char* src)`): `memset(dest,0,0x10)` (`00710eb9 PUSH 0x10 / PUSH 0 / PUSH dest / CALL 0x0091c630`), copies up to 16 chars lowercased via 0x91E01A, then copies the 16 bytes to `retCopy`. So it is `CResRef::operator=(const char*)`-like, writes into ECX.
- 0x710CA0 (`RET 8`, ECX=dest, args `(CResRef* retCopy, const CResRef* src)`): same, 16-byte lowercased copy of `src` into ECX (`00710cbb CMP [EBP-4],0x10 ; MOVZX EDX,byte [src+i] ; CALL 0x0091e01a ; MOV [ECX+i],AL`).
- 0x710840 (`RET 4`, ECX=this): `CResRef(const char*)` ctor, calls 0x710eb0.
Outputs are 16-byte CResRef (NUL padded, lowercase), not texture pointers.

Dispatch (type = `[arg2]` if arg2 != 0, else 0xC):
```
0074e104  837d0c00  CMP dword ptr [EBP + 0xc],0x0
0074e10a  c785fcfcffff0c000000 MOV dword ptr [EBP + 0xfffffcfc],0xc   ; arg2==NULL -> type 0xC
0074e139  83e80c    SUB EAX,0xc ; 0074e142 CMP ...,0x33 ; JA 0x0074e731 ; MOVZX EDX,byte [ECX+0x74e74c] ; JMP [EDX*4+0x74e738]
```
Byte table 0x74E74C (52 entries) and dword table 0x74E738 = {0x74E163, 0x74E6C2, 0x74E651, 0x74E342, 0x74E731(default)} (both read from the exe):
type 0x0C -> byte0 -> 0x74E163; type 0x0F -> byte1 -> 0x74E6C2; type 0x2E (idx 34) -> byte2 -> 0x74E651; type 0x3F (idx 51) -> byte3 -> 0x74E342; all others -> 0x74E731 (leave blank).

Per-type behaviour:
- Type 0x0F cast spell (0x74E6C2): spell id = first dword of `[arg2+0x38]`; icon = spell row +0x14; out2 = `"i_powerm"`.
```
0074e6c4..0074e6cf  PUSH 0x4 ; MOV EDX,[EBP+0xc] ; ADD EDX,0x38 ; PUSH EDX ; LEA EAX,[EBP+-0x8] ; PUSH EAX ; CALL 0x0091dba0   ; memcpy(&id,[arg2+0x38],4)
0074e6e0  8b15d0b4a100 MOV EDX,dword ptr [0x00a1b4d0]   ; rules global
0074e6f9  8b8904010000 MOV ECX,dword ptr [ECX + 0x104]   ; rules+0x104 = spell array
0074e6ff  e84c2df7ff CALL 0x006c1450                      ; row = Spells(id)
0074e70a  83c214 ADD EDX,0x14 ; ... 0074e718 CALL 0x00710ca0 ; out1 = *(CResRef*)(row+0x14)
0074e71d  6844df9900 PUSH 0x99df44 ; ... CALL 0x00710eb0   ; out2 = "i_powerm"
```
  (no NULL check on row at 0x74E704..0x74E70A in this path: `ADD EDX,0x14` is executed even if row==0, then 0x710CA0 reads from 0x14: UNCONFIRMED crash risk for a bad spell id.)
- Type 0x2E use item (0x74E651): item object id = first dword of `[arg2+0x38]`; server GetItemByGameObjectID -> GetIcon; out2 = `"i_useitemm"`.
```
0074e666  8b15a4b4a100 MOV EDX,[0x00a1b4a4]  ; 0074e66c MOV EAX,[EDX+0x8]   ; server app (AppManager+8)
0074e679  8b8d0cfdffff MOV ECX,[..fd0c] ; 0074e67f CALL 0x0051c0e0          ; item = GetItemByGameObjectID(id)
0074e697  e82476ecff CALL 0x00615cc0 (ECX=item, push &tmp)                 ; EAX = &tmp (CResRef) -> 0x710ca0 -> out1
0074e6ac  6850df9900 PUSH 0x99df50 ; CALL 0x00710eb0                       ; out2 = "i_useitemm"
```
  (item NULL -> both outs stay blank, `0074e687 CMP [-0x10],0 / JZ 0x0074e6c0`.)
- Type 0x0C (0x74E163): requires arg3 (`0074e163 CMP [EBP+0x10],0 / JNZ`; arg3==0 -> blank). `type8 = byte [arg3+0x10]`, `u32 [arg3+0x5C]` saved to `[EBP-0xC]`. Switch `type8-1` (cmp 0xA) via dword table 0x74E780 = {1:0x74E1B3, 2..5:0x74E33D(none), 6:0x74E1DA, 7:0x74E204, 8:0x74E33D(none), 9:0x74E293, 10:0x74E231, 11:0x74E2FA}:
  - sub-type 1 (attack): out1 `"i_attack"` (0x99df88), out2 `"i_attackm"` (0x99df7c)
  - 6 (equip): `"i_equip"` (0x99df74) / `"i_equipm"` (0x99df68)
  - 7 (unequip): `"i_unequip"` (0x99df5c) / `"i_equipm"` (0x99df68)
  - 9 (spell): `[arg3+0x24]` spell id -> `0x006c1450` on `[[0xa1b4d0]+0x104]`, out1 = row+0x14, out2 = `"i_powerm"` (0x74E293..0x74E2F1, same pattern as above, row NULL-checked at `0074e2c6`)
  - 10 (use item): `[arg3+0x64]` item id -> 0x51C0E0 -> 0x615CC0 -> out1; out2 `"i_useitemm"` (0x74E231..0x74E289; item NULL-checked `0074e255`)
  - 11 (feat): `MOVZX ECX,word [EBP-0xC]` (= low word of `[arg3+0x5C]`) -> `0x006a20f0` with ECX = rules global `[0x00a1b4d0]` itself (not +0x104) -> out1 = feat row+0x18, out2 `"i_featm"` (0x99df3c) (0x74E2FA..0x74E338; row NULL-checked `0074e30d`)
- Type 0x3F combat (0x74E342):
  - arg3 != 0: `type8 = byte [arg3+0x10]`, switch `type8-1` (cmp 0xD) via dword table 0x74E7AC = {1:0x74E382, 2..5:0x74E56D(none), 6:0x74E3AF, 7:0x74E3DC, 8:none, 9:0x74E46A, 10:0x74E409, 11:0x74E4FD, 12:none, 13:0x74E545, 14:0x74E4D1}:
    - 1 attack `i_attack`/`i_attackm`; 6 equip `i_equip`/`i_equipm`; 7 unequip `i_unequip`/`i_equipm`
    - 9 spell: `[arg3+0x24]` -> spells table row+0x14 / `i_powerm` (0x74E46A)
    - 10 item: `[arg3+0x64]` -> 0x51C0E0 -> 0x615CC0 / `i_useitemm` (0x74E409)
    - 11 feat: `MOVZX EAX,word [EDX+0x5c]` (arg3+0x5C word) -> 0x6A20F0 -> row+0x18 / `i_featm` (0x74E4FD)
    - 13: `"i_stancedef"` (0x99df24) / `"i_stancedefm"` (0x99df14) (0x74E545; sub-type 13 = total defense, matches `[round+0xB1C]==0xD` in 0x6DAAC0)
    - 14: `"i_swtchwpn"` (0x99df30) both outs (0x74E4D1)
    - sub-types 2-5, 8, 12 and >14: blank.
  - arg3 == 0 (0x74E572): obtains party member 0's SERVER creature (same chain as section 3), then `0074e5c7 CALL 0x006daac0` ([CSWSCreature::IsInTotalDefense]): nonzero -> `"i_stancedef"`/`"i_stancedefm"`, zero -> `"i_attack"`/`"i_attackm"`; creature NULL -> `"i_attack"`/`"i_attackm"` (0x74E624). 0x6DAAC0 body (CONFIRMED): returns 1 if `byte[creature+0x110C]==0` (`006daae7 JZ 0x006dab04` -> `MOV EAX,1`), else returns 1 only if `round!=0 && byte[round+0xB1C]==0xD`, else 0. So the "total defense" name is suspect for the `+0x110C==0` case; the meaning of `+0x110C` is UNCONFIRMED.

## 3. How 0x74DC80 iterates; which object

0x74DC80: `FUN_0074dc80`, plain `RET` (no stack args), ECX = this (HUD, stored `0074dcab MOV [EBP+0xffffff5c],ECX`). Slot struct base = `this + 0x5F2C + slot*0x148` (`0074e039 IMUL EAX,EAX,0x148 ; LEA EDX,[ECX+EAX+0x5f2c]`); 4 slots (`CMP [EBP-0x30],4`).

Creature source: SERVER creature of party slot 0 (not a client object):
```
0074dcb1  a1a4b4a100  MOV EAX,[0x00a1b4a4]                  ; g_pAppManager
0074dcb6  8b4804      MOV ECX,dword ptr [EAX + 0x4]          ; CClientExoApp
0074dcc5  e8c61effff  CALL 0x0073fb90                         ; party = [[client+4]+0x270]  (0073fb9c/0073fb9f)
0074dccc  e8af800900  CALL 0x007e5d80                         ; GetMember(party, 0) -> 0x7e5da0(idx 0)
0074dce3  e818fb0200  CALL 0x0077d800                         ; client creature -> server creature
0074dcee  8b91dc100000 MOV EDX,dword ptr [ECX + 0x10dc]       ; round = creature+0x10DC (NO null check on ECX)
```
- 0x7E5DA0 (idx in `[EBP+8]`, `RET 4`): `007e5dc5 MOV ECX,[EAX+EDX+0x24]` (entry id, stride 0x88) then `007e5ddf CALL 0x0073f550` = [CClientExoApp::GetCreatureByGameObjectID] -> returns the CLIENT creature (CSWCCreature). (The DB note on 0x77D800 "party entry -> creature" is inexact: its ECX is the client creature.)
- 0x77D800 (thiscall, plain RET): `CALL 0x007f2540` ([CSWCObject::GetServerObject], caches at `[this+0xF8]`, `007f25a6 CALL 0x0051c8b0` ClientToServerObjectId + `007f25c1 CALL 0x0051c0a0` GetGameObject on server app `[0xa1b4a4]+8`), then `0077d828 MOV EAX,[EDX+0x30] ; CALL EAX` = a virtual call at vtable+0x30 on that server object; result is used as a creature (`[+0x100]`, `[+0x10DC]`). Name of vtable+0x30 (likely "AsCSWSCreature") UNCONFIRMED. If either step is NULL, 0x77D800 returns 0 and 0x74DCEE faults (no test): UNCONFIRMED whether HUD gating prevents it.
- So the queue read is `((CSWSCreature*)serverObj)+0x100`, i.e. SERVER list, in-process, party member INDEX 0 (leader), not the controlled/selected character.

Round gates and fields (server round `R = [creature+0x10DC]`):
```
0074dd01  MOV ECX,[EAX+0xa84]          ; R+0xA84 (pointer/int) tested
0074dd25  e8562be4ff CALL 0x00590880   ; byte [R+0xB24]  (0x590880: MOV AL,[EAX+0xb24]; plain RET)
0074dd31  MOV [EBP+0xffffff58],1       ; flag F = ([R+0xA84]!=0 && [R+0xB24]!=0) ? 1 : 0   (stored -0x48 and -0x5c)
```
(Note: 0x590880 returns the BYTE `[round+0xB24]`, not a node pointer; DB name "GetCurrentAction" is a bool-like getter, name UNCONFIRMED.) `[R+0xB18]` = current round action node pointer; `[R+0xB1C]` byte = its sub-type (used by 0x6DAAC0); `[R+0xB00]` = pointer to list object of pending round action nodes.

Per queue item (action A = data of node, `[A+0]` = type dword):
```
0074ddf7  83383f CMP [EAX],0x3f ; JNZ 0x0074de02 ; 0074ddfc CMP [EBP-0x48],0 ; JNZ 0x0074de1e   ; 0x3F counts only if F != 0
0074de05  CMP [ECX],0xc / JZ 0x74de1e ; 0074de0d CMP [EDX],0xf / JZ ; 0074de15 CMP [EAX],0x2e / JNZ 0x0074dee0   ; shown types: 0x0C, 0x0F, 0x2E, (0x3F if F)
0074de1e  837d8400 CMP [EBP-0x7c],0 ; JNZ 0x0074dee0           ; only the FIRST qualifying item gets the "current node"
```
First qualifying action (flag `[EBP-0x7C]` = 0):
1. `[EBP+0xffffff7c] = -1`; `node = [R+0xB18]` (0074de35).
2. node != 0: `[-0x7c]=1`, call `0x74E0B0(R, A, node, &int, &res1, &res2)` then `0x74E7F0(&slot, &res1, &res2, F, 0)` (0074de6c / 0074de89).
3. node == 0: skip if type is 0x3F (`0074de93 CMP [ECX],0x3f / JZ 0x0074dee0`); else call `0x74E0B0(R, A, 0, &int, &res1, &res2)` + `0x74E7F0(...)` (0074deb7 / 0074ded4) and set `[-0x7c]=1`.
Then, if `A.type == 0x3F` and `[R+0xB00] != 0`: iterate that round list (GetHead/GetAt/GetNext as in section 1); for each round node N while slot<4: `0x74E0B0(R, A, N, &[EBP-0x44], &res1, &res2)` + `0x74E7F0(...)` (0074df82 / 0074df9f). If the round list is empty and `[-0x7c]==0` (0074dfcd), it renders once with `node = [R+0xB18]` (0074dfd6..0074e01a).
After the loop, unused slots are hidden: `0074e048..0074e076` clears bit 1 of `[slotBase+0x48]` (`AND EAX,0xfffffffd`).
Note: because all calls pass arg2 = A (type 0x3F for the combat entries), 0x3F entries use only the round node (arg3) for the icon. Per-item types 0x0F/0x2E use `[A+0x38]` only when arg3's path is not taken: for type 0x0F/0x2E the dispatcher (0x74E6C2/0x74E651) ignores arg3 completely.

0x74E7F0 (slot filler, `RET 0x14`, ECX = HUD): args `(int* slot, CResRef* res1, CResRef* res2, int flag, int force)`.
```
0074e7f9  CMP [EBP+0x18],0 ; JNZ ; 0074e804 MOV ECX,[EBP+0xc] ; 0074e807 CALL 0x00710ad0 ; TEST EAX ; JZ 0x0074e908   ; force==0 and res1 blank -> skip (slot not consumed)
0074e814  CALL 0x00710a10 (res1 non-blank?) ; then 0x414840(slotBase+0x5F8C+0x14, res1 or res2, 0)  ; sets the slot icon resref
0074e8c8..0074e905  slot index++ ; sets bit 1 of [slotBase(0x5F2C)+0x48] (shown); alpha float [+0x24] = 1.0 or const 0x98be34 (slot 0 with flag==0)
```
(0x710AD0 `RET 4`, ECX = CResRef, returns 0 when equal to arg string case-insensitively; 0x710A10 returns 1 if first byte != 0.) What 0x414840 does exactly is UNCONFIRMED. res2 (the "m" variant) is only used when res1 is blank, which the 0x74DC80 calls never allow (force=0 always), so res2 is effectively unused by this caller.

## 4. Signatures of the helpers

| addr | convention | RET | args | behaviour (CONFIRMED asm) |
|---|---|---|---|---|
| 0x6C1450 | thiscall, ECX = `{int count @0, Row* data @4}` (= `[[0xa1b4d0]+0x104]`) | `RET 0x4` (`006c1486`) | `int idx` | `006c145a CMP [EAX+4],0 / JZ` ; `JL` idx<0 ; `006c146c CMP EDX,[ECX] / JGE` ; returns `idx*0x1A8 + [ECX+4]` else NULL (`006c1473 IMUL EAX,EAX,0x1a8`). Spell row: CResRef icon at `+0x14` (`0074e2cf ADD ECX,0x14`) |
| 0x615CC0 | thiscall, ECX = CSWSItem* | `RET 0x4` (`00615d50`) | `CResRef* out` (16 B) | returns `out` in EAX. `006d6e30` (`[item+0xC]` = base item idx, `[0xa1b4d0]+0x34` table, via 0x6d5200) -> BaseItem row B; B NULL -> out = CResRef("") (`00615cda PUSH 0x986925`). Else `CL=[B+0x18]`; nonzero -> `0x6d4e60(B, out, word=[item+0x2A6], 'm'(0x6d))`, zero -> `0x6d4e60(B, out, word=[item+0x2A4], 'm')`. Meaning of `[B+0x18]` UNCONFIRMED |
| 0x6D4E60 | thiscall, ECX = BaseItem row | `RET 0xC` (`006d4ee7`) | `(CResRef* out, word n, byte c)` | `sprintf(buf, "i%s_%c_%03d" (0x99a5fc), B+0x3C, c, n)` if `[B+0x74]!=0 && c!=0` else `"i%s_%03d"` (0x99a5f0); then `0x710840(out, buf)` |
| 0x51C0E0 | thiscall, ECX = CServerExoApp* (= `[0xa1b4a4]+8`) | `RET 0x4` (`0051c0f9`) | `uint objId` | `MOV ECX,[ECX+4]` (internal) `CALL 0x537b00` (lookup in `[internal+0x1005C]`, checks `byte[obj+8] == byte[0x994475]` and calls vtable `+0x38`). Returns item or NULL |
| 0x6A20F0 | thiscall, ECX = rules global `[0x00a1b4d0]` (the object itself) | `RET 0x4` (`006a213c`) | `word featId` (`MOVZX EAX,word [EBP+8]`) | `CMP EAX,word [ECX+0x11C] / JGE` NULL; row = `[ECX+0x108] + id*0x50`; requires `(dword [row+0x28] >> 4) & 1` else NULL (`006a2119..006a2123`). Feat row: CResRef icon at `+0x18` (`0074e316 ADD EDX,0x18`) |
| 0x7A1720 | thiscall, ECX = list object | plain `RET` | none | GetHeadPosition (section 1) |
| 0x561430 | thiscall, ECX = list object | `RET 0x4` | `Node* pos` | GetAt |
| 0x58C370 | thiscall, ECX = list object | `RET 0x4` | `Node** pos` | GetNext (advances, returns new data) |

Offsets: spell row stride 0x1A8, resref `+0x14`; feat row stride 0x50, resref `+0x18`, enable-bit `+0x28` bit 4; rules global `[0xA1B4D0]`: `+0x34` base items, `+0x104` spells, itself = feat table owner (`+0x108` rows, `+0x11C` word count).

## 5. Fixed resref strings (read from the exe, 0x99DF14..0x99DF88; blank = 0x986925)

| addr | string | used for |
|---|---|---|
| 0x99DF88 | `i_attack` | attack, out1 |
| 0x99DF7C | `i_attackm` | attack, out2 |
| 0x99DF74 | `i_equip` | equip, out1 |
| 0x99DF68 | `i_equipm` | equip out2, unequip out2 |
| 0x99DF5C | `i_unequip` | unequip out1 |
| 0x99DF50 | `i_useitemm` | use item out2 |
| 0x99DF44 | `i_powerm` | spell out2 |
| 0x99DF3C | `i_featm` | feat out2 |
| 0x99DF30 | `i_swtchwpn` | switch weapon (sub-type 14), both outs |
| 0x99DF24 | `i_stancedef` | total defense out1 |
| 0x99DF14 | `i_stancedefm` | total defense out2 |
| 0x99A5FC | `i%s_%c_%03d` | item icon format |
| 0x99A5F0 | `i%s_%03d` | item icon format |
| 0x986925 | `` (empty) | blank CResRef init |

Spell/feat/item out1 values come from game data (spell row +0x14, feat row +0x18, item GetIcon), not fixed strings.

## Open items (UNCONFIRMED)
- Name/meaning of vtable+0x30 in 0x77D800 (assumed AsCSWSCreature); creature NULL deref at 0x74DCEE.
- tail@4/count@8 of the list internal and node prev@0 (not read by these functions; only the Common header).
- Whether `[+0x10]` sub-type values 2-5, 8, 12 exist as real round action kinds (the HUD shows blank for them).
- Spell row NULL check: present in the 0x74E293 (`0074e2c6`) and 0x74E46A (`0074e49c`) paths, ABSENT in the type-0x0F path at 0x74E6C2 (`0074e704`..`0074e70a` adds 0x14 to the unchecked result).
- Meaning of `[creature+0x110C]` in 0x6DAAC0, `[B+0x18]` in 0x615CC0, 0x414840 effect.
