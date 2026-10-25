# F4 unlink: CSWSCombatRound action list (Steam swkotor2.exe)

Basis: static dumps only, `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/<bb>/<addr>.{c,asm}`, read in place. `copy_dump_excerpts.sh` not run (it writes into the repo, outside scope). No REA.

## 1. List layout (corrects the brief)

- `round+0xB00` is NOT an array. It holds pointer W to a wrapper. Empty/unallocated test: `CMP dword ptr [EAX + 0xb00],0x0` (0x59027c, AddAction).
- W[0] = internal `CExoLinkedListInternal*`. Wrappers all do `MOV ECX,dword ptr [ECX]` first: `0x58c37e`, `0x56143e`, `0x7f2a9a`, `0x5210fc` (`MOV ECX,dword ptr [EAX]`).
- internal: `+0` head, `+4` tail, `+8` count (decomp 0x73a200 `param_1[2] = param_1[2] + -1`; 0x739f20 `param_1[2] = param_1[2] + 1`).
- node (12 B, `operator new(0xc)` in 0x739e60): `+0` prev, `+4` next, `+8` payload (action ptr). `0x73a010` returns `*(param_1 + 8)`.
- Count is `internal+8`, read by `0x5210f0`: `MOV EDX,dword ptr [ECX + 0x8]` (0x5210fe). The brief's `round+0xB18` is NOT referenced in AddAction, RemoveAll, RemoveLast, AddSWSpellAction, or the wrappers (grep 0xb18/0xb1c: 0 hits).
- Capacity: `CMP EAX,0x4` / `JNC` at 0x5902d7-0x5902da. Full branch frees the new node (0x5903ed, 0x590400).
- AddAction writes no round field. Insert is `0x739f20` / append is `0x739e60`, both internal-only.
- Sort key is `*new` vs `*existing` (`+0` dword): `CMP EAX,dword ptr [EDX]` / `JGE` at 0x590318-0x59031c. Spell nodes keep `+0 = 0`: base ctor writes `MOV dword ptr [EAX],0x0` (0x58c81a) and AddSWSpellAction never writes `+0`. So spell-only lists are FIFO.

## 2. RemoveLastAction 0x5904C0 and RemoveAllActions 0x590430

RemoveLast (returns 1 if it removed, else 0):
1. `if W==0 return 0` (0x5904d3-0x5904da). `CALL 0x7a1780` = IsEmpty (`CMP dword ptr [EDX],0x0`, `SETZ`, 0x7a1796-0x7a1799); `JNZ` exit at 0x5904ec.
2. `CALL 0x5936d0` -> `CALL 0x73a2a0` (RemoveTail). It frees the LIST node: `CALL 0x009196fd` (0x73a31e).
3. Action freed by caller: `CALL 0x00623320` (0x590514, no-op stub, ECX=node), then `PUSH EAX` / `CALL 0x009196fd` (0x590523-0x590527), `ADD ESP,4`.
4. `MOV dword ptr [EBP + -0x4],0x1` (0x590545).

RemoveAll: `while (IsEmpty(W)==0) { p = CALL 0x7f2a90 (RemoveHead via 0x73a200, frees list node at 0x73a279); if p { CALL 0x623320; PUSH p; CALL 0x9196fd } }` (0x590445-0x5904ae). Frees every action. W and internal stay allocated, count goes to 0.

Free pattern (engine, everywhere): `MOV ECX,node; CALL 0x00623320; PUSH node; CALL 0x009196FD; ADD ESP,4`. `0x9196FD` = operator delete (`JMP 0x0091C6B5` = `_free`, file 0x91c6b5 header).

## 3. Design: remove ONE node at index i (sub-type 9)

No compaction is needed: there is no index array. Unlink keeps order. Primitive: `CExoLinkedListInternal::Remove(pos)` at 0x73a130 (thiscall, ECX=internal, 1 stack arg = list node, `RET 4`). It unlinks pos, fixes head/tail, frees the list node (`CALL 0x009196fd` at 0x73a1d4), `count--`, returns payload (`iVar3 = param_2[2]`). Wrapper 0x64dde0 (`MOV ECX,[ECX]; CALL 0x73a130`) has no static callers in the dump. Call 0x73a130 directly.

```
internal = *(round+0xB00)              ; W[0]
node = internal[+0]                    ; head
for k < i: node = node[+4]             ; walk next
if node == 0: return 0                 ; index out of range
act = node[+8]
if act[+0x10] != 9: return 0           ; sub-type 9 = spell cast
act = Remove(internal, node)           ; ECX=internal; PUSH node; CALL 0x73a130
ECX = act; CALL 0x00623320             ; no-op dtor stub
PUSH act; CALL 0x009196FD; ADD ESP,4   ; operator delete
```

## 4. Hook placement

Preferred: call site inside AddSWSpellAction, right before the append.
- `0x5917C1: 6A FF` PUSH -1; `0x5917C7: 8B 4D D8` MOV ECX,[EBP-0x28] (= round); `0x5917CA: E8 A1 EA FF FF` CALL 0x00590270 (AddAction).
- Original 5 bytes at 0x5917CA: `e8 a1 ea ff ff`. Patch to `E8 <stub - 0x5917CF>`.
- Node is fully built at this point: `+0x10=9` (0x5916df), `+0x24` (0x59170b), `+0x84` (0x5916ff).
- Stub must keep ECX (round), restore the stack so `[esp+4]=node, [esp+8]=-1`, then `JMP 0x00590270`. AddAction's `RET 8` returns to 0x5917CF.

Alternatives (weaker):
- Call site 0x56538D in AddCastSpellActions 0x565310: `e8 de c2 02 00` = CALL 0x591670 (AddSWSpellAction). Node not built yet, 16 stack args.
- Entry 0x591670: `55 8b ec 6a ff` (PUSH EBP / MOV EBP,ESP / PUSH -1), boundary at 0x591675. Stub must replay the SEH prolog.

## Verdict fields
- Brief's `+0xB18` count is wrong: the count is at internal+8.
- Not verified: actor field on the action node (none identified here); no other code holding a node pointer across the hook.

## Orchestrator check (2026-10-25)
- `round+0xB18` is real but not part of the list: it is the current (in-progress) action pointer, read by AIActionCombat (`MOV EAX,[EDX+0xb18]` 0x6d95a9, 0x6d95da) and 0x58e520 (0x58e618/0x58e624, cleared at 0x58e669). The earlier "cur +0xB18" fact holds in that sense; F4 must never free the node held there.
- GetAction 0x590220 = RemoveHead (`CALL 0x7f2a90` at 0x590242) gated by `[round+0xa94]` vs `node[+0]` (0x590256-0x59025c): oldest first, confirms FIFO (fact 2).
- The round is per creature (cr+0x10DC), so every node in its list belongs to that actor: no actor field is needed.
