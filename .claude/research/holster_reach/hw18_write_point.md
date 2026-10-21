# HW18 write-point recon (swkotor2.exe Steam, static only)

Sources: decomp/asm dumps under ~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/ ; RTTI/vtable slots resolved from the exe by script; no live REA.

## Premise correction
- Gob+0xA4/+0xB0 is the OWNER OBJECT's world pose, not a bone's. A bone is a part node P (returned by Gob::GetPart).
- Bone local pose lives at P+0x08 (pos, 3 floats) and P+0x14 (quat w,x,y,z). Scale at P+0x24.
- A procedural arm write belongs on P+0x14 (bicep part), not Gob+0xB0.

## (1) Writer of bone poses each frame: CONFIRMED (writer); seam PARTIAL
- Sampler chain: CAurObject::AnimateExternal 0x451E60 calls Gob::Animate via vt+0x14 (`CALL EAX` line in decomp/45/00451e60.c). Gob primary vtable slot +0x14 (addr 0x98B5E0) = 0x4E1D40.
- Second entry: 0x481073 `CALL 0x004E1D40` inside 0x480E40 (decomp/48/00480e40.asm:160); 0x480E40 is reached via thunk 0x481D50 (secondary vtable 0x98C290 slot 0).
- Gob::Animate 0x4E1D40 calls 0x4DFED0 with (Gob+0x98 = root part, anim, dt, t, weight, flag=0) (decomp/4e/004e1d40.c).
- 0x4DFED0 recurses over child parts matched by node id (+0x28 vs track +0x2) (decomp/4d/004dfed0.c).
- 0x4DFF58 `CALL 0x00453860` (MdlNode::ControlDispatch), gated by 0x4DFF2E `CMP [node+0x48],0` and flag byte [EBP+0x1C] (asm 0x4DFF34-0x4DFF3A).
- 0x453916 / 0x453AD2 `CALL 0x004DEF00` (InternalControl). 0x4DEF86 `CALL 0x004DEA20` is the ONLY caller of 0x4DEA20 (loop over [this+0x3C] entries, entry = [this+0x38] + i*16).
- 0x4DEA20 (1242 B): writes target + entry.offset; type nibble at entry+0xC (1 scalar, 2/4 quat, 3 vec3) (decomp/4d/004dea20.c, asm 0x4DEA20-0x4DEA35).
- Entry offsets set by parser 0x486AA0: position -> +0x08, orientation -> +0x14, scale -> +0x24 (decomp/48/00486aa0.c, lines with `local_c + 8`, `local_c + 0x14`, `local_c + 0x24`).
- Detour options:
  - 0x481073: 5-byte relocatable `E8 rel32` CALL to Gob::Animate; next insn 0x481078 `MOV ESP,EBP`. Re-point rel32, no displaced bytes. Covers the 0x480E40 path only.
  - Gob vt slot 0x98B5E0: 4-byte pointer swap covers the AnimateExternal path.
  - No safe detour inside 0x4DEA20 (per-controller, mid-blend).
  - 0x457510 (world resolver) entry: 9-byte span 0x457510-0x457518 = PUSH EBP | MOV EBP,ESP | SUB ESP,0x130 (instruction boundaries +1,+3,+9). Usable, but it runs on demand, so it can read stale local pose before our write.
- Gob::Animate entry 0x4E1D40: boundaries +1,+3,+5,+10 (PUSH imm32 SEH frame); first >=5 boundary is +10.

## (2) Child follow / world derivation: CONFIRMED
- No cached world field. 0x457510 (shared vt+0xC of all part classes) recomputes world on every call from local pose.
- Algorithm (decomp/45/00457510.c): start from P local; loop node=P, node=node+0x3C (parent link): out_pos = R(node.q)*(out_pos*node.scale(+0x24)) + node.pos(+0x08); out_quat = node.q (+0x14) * out_quat.
- Then owner = P+0x40 (`local_c`): if owner==0 the function returns LOCAL only (no parent walk). Else owner scale +0x1E0, owner pos +0xA4, owner quat +0xB0 applied last.
- Consequence: writing bicep P+0x14 (and P+0x08) moves forearm/hand on their next 0x457510 call. Their own locals are untouched.
- Part vtable slot +0x0C = 0x457510 for PartTriMesh 0x98B414, PartSkin 0x98B08C, PartDanglyMesh 0x98AFEC, PartAnimMesh 0x98B22C, PartReference 0x98B1AC, PartLight 0x98B544, PartEmitter 0x9914CC (vt.py slot dump).
- Gob::GetPartPosition 0x462B90 calls part vt+0x0C (`CALL EAX` at 0x462C04) and returns world (decomp/46/00462b90.c).
- Gob::GetPart 0x4587F0: "root" -> Gob+0x98, else tree search (decomp/45/004587f0.c).

## (3) Skinning reads: PARTIAL
- Verified: the attach refresh reads the bone WORLD via vt+0xC of N+0x18 (0x504EF0 `MOV EAX,[EDX+0xC]`, 0x504EF3 `CALL EAX`), writing child+0xA4 / child+0xB0 as out-args. Consistent with 0x457510 signature.
- Bone pose slots read by the resolver: P+0x08, P+0x14, P+0x24, P+0x3C, P+0x40; owner +0xA4, +0xB0, +0x1E0. Bone +0xA4/+0xB0 are NOT read by the resolver.
- Per-vertex skin consumer NOT located statically. Skin bone list is built in 0x449640 CreateSkinNodeList (CExoPtrList::Add at 0x4496EE) which never calls vt+0xC. Unverified candidates with indirect vt+0x0C sites: 0x45C810 (site 0x45C8B6, no direct callers), 0x4626D0 (site 0x462796).

## Caveats for the write (all from asm/decomp)
- SetPartLocalPosition 0x462C50 sets P+0x48=1 (decomp/46/00462c50.c). 0x4DFF2E skips controller writes when [node+0x48] != 0, so the bone would freeze under animation. Do NOT use the Gob API for the arm; write P+0x14 directly.
- Blend base: 0x4DEA20 one branch reads the target's current quat (`local_7c` = target+0x14) as the slerp base, so a persisted write can feed back into the next frame. Re-derive from sampled value each frame.
- Gate for ControlDispatch: flag set in 0x4DFF0E-0x4DFF25 only when node[+4] matches a compare against FUN_00452830(...)+0xA8. Gob::Animate passes 0. UNSURE whether NPC Gobs pass it.
- Throttle: 0x4DFED0 recursion stops at depth DAT_00A7BC20 vs DAT_00A049F8 / local_10 (distance LOD). Deep children of far Gobs skip this frame.
- Attach refresh 0x504D40 may read the hand's world before a post-Animate write, so a weapon can lag one frame.

## Open / needs live REA
- OPEN-1: which function reads bone world per vertex (skin draw). Live REA: decompile 0x45C810 and 0x4626D0; ask whether a loop calls vt+0x0C on the bone list from 0x449640.
- OPEN-2: flag gate at 0x4DFF1C-0x4DFF25 for NPC Gobs (live REA of 0x4DFED0 gate, or runtime probe of P+0x14 change).
- Mapping offset 0x08/0x14/0x24 into the controller record dword is inferred from parser arguments (`local_c+8` etc.); the record write itself was not traced.
