# Recon: 0x005644B0 (FUN_005644b0), swkotor2.exe

STATUS: full result. The repo file .claude/research/holster_a3_5644B0.md still holds the earlier BLOCKED copy. Overwrite was refused (Write needs a prior Read, and no Read tool exists here).

Binary: PE x86, sha256 7751f7d8b06c63a21eacbef6e5ded6cb4f497fa960cca6a6cf645552701dc04f. Opened via provider ghidra with the ~/.steam path (it resolves to ~/.local/share/Steam/...). Not compared to the address DB hash (UNKNOWN). No `ls` of the game dir (no shell tool).

## Result
- NOT an animation sender. The first stack arg is a 16-bit sentinel, not an animation id. No animation-play call appears in the body or in the decompiled callees.
- The body fits enqueuing an action, most likely move-to-point: Vec3 destination, object ref, float, flag word, then submit via 0x54AA10. The name AddMoveToPointActionToFront is UNCONFIRMED.

## Bytes
- 0x5644B0 (read_bytes, len 16): 55 8B EC 83 EC 0C 89 4D F4 C7 45 FC 00 00 00 00 = push ebp; mov ebp,esp; sub esp,0Ch; mov [ebp-0Ch],ecx; mov dword [ebp-4],0
- 0x5645F3 (read_bytes, len 16): E8 78 8D 01 00 (call 0x57D370) | B8 01 00 00 00 (mov eax,1) | 8B E5 | 5D | C2 44 00 (ret 0x44)

## Calling convention
- thiscall-style: ECX is spilled and used as the object base.
- 17 dword stack args, callee-cleaned (ret 0x44, verified).
- Arg offsets are Ghidra-inferred. Assumes [ebp+8] = [esp+4] at entry.

## ECX (this)
- +0x004 self object id; +0x094 self position Vec3 (both used by 0x5648D0).
- +0x11B0 written = 2. +0x380 sub-object ptr (+0x26C, +0x270, +0x274 zeroed).
- +0x104C target id slot (cleared via 0x57D370).
- Class UNKNOWN. "Creature" is unconfirmed.

## Arguments (Ghidra signature; caller evidence from 0x5648D0 and 0x565310)
- [ebp+08] p1 undefined2: caller passes local_8 & 0xFFFF (local_8 = 0xFFFF, then 0xFFFE). 0x540CA0 treats -1 as default and -2 as this+0x16. NOT an animation id.
- [ebp+0C] p2 int: pointer to Vec3 (x +0, y +4, z +8), the destination.
- [ebp+10] p3: caller passes local_3c (from sub-object +0x124). Meaning UNKNOWN.
- [ebp+14] p4: object id (caller compares it to 0x7F000000). Probably the target.
- [ebp+18] p5 uint: caller passes 1 (flag bit 0).
- [ebp+1C] p6: float from 0x5642D0 (range or tolerance?). UNKNOWN.
- [ebp+20] p7 float: caller passes 0.0.
- [ebp+24] to [ebp+38] p8 to p13: caller passes 0. These feed flag bits.
- [ebp+3C] p14: pointer to a zeroed Vec3 (x and y serialized).
- p15, p16 undefined4; p17 uint. Caller shows 14 args, not 17 (Ghidra arity mismatch).

## Behaviour (procedure_pseudo_code)
1. Packs an 11-bit flag word from p5, p8, p7, p10, p12, p11, p17, p13.
2. Sets this+0x11B0 = 2. Zeroes this+0x380 at +0x26C, +0x270, +0x274.
3. Calls 0x540CA0(opcode 1, p1, typed fields). 0x540CA0 allocates 0x74 bytes, stores the opcode at +0 and p1 at +0x6C, writes up to 13 fields, then calls 0x54AA10 -> 0x739DA0 (NOT read).
4. Calls 0x57D370(0x7F000000, 0), which clears this+0x104C.
5. Returns 1.

## Callers and callees
- Callees: 0x540CA0 and 0x57D370 (both read); 0x54AA10 (wrapper, read).
- Callers: 35 total. Checked 2: 0x5648D0 and 0x565310, both pass local_8 & 0xFFFF. 33 unchecked.
- 0x5648D0 (11 params, thiscall): takes target id p7 and copies that object's +0x94 position into the Vec3. Emits opcodes 0x2E, 0x31, 0x12, 0x13, 0x11. Calls 0x5644B0 in one branch. No anim-play call. Prologue NOT read.
- 0x565310 (18 params): same message pattern (opcodes 0x0F, 0x31, 0x12, 0x13, 0x11). Calls 0x5644B0 in one branch. No anim-play call. Prologue NOT read.

## Labels
- (a) HolsterAnimSend: REFUTED for this function. The p1 sentinel is passed at both checked callers, and there is no anim-play call.
- (b) AddMoveToPointActionToFront: PARTIAL. The shape fits a move-to-point enqueue. The name, the "ToFront" insertion, and the meaning of opcode 1 are UNCONFIRMED.

## Anim-play function
- NOT identified among the 6 functions read. OPEN.

## Limits
- Functions read (6-function budget): 0x5644B0, 0x540CA0, 0x57D370, 0x5648D0, 0x565310, 0x54AA10.
- Not read: 0x739DA0, 0x5642D0, 0x53FA80, 0x51C0A0, 0x53F7F0.
