# R1: which hand a weapon Gob is in (Steam swkotor2.exe 6a522e71)

Read-only static recon. Dump: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` (decomp/76/007637e0.c|.asm, decomp/76/0076e610.c, decomp/73/0073f530.c, decomp/78/0078bd80.c). Owner probe: `patch_manager_mods/holster_src/Holster.cpp:726-729`.

## OUTPUT
1. The hand is encoded at the in-world attach call site inside FUN_007637E0 (0x7637E0). Each armed hand calls body vtbl+0x7C with args (W, node name, flag, scale):
   - right: W = right item obj +0x14, name CExoString "rhand" (literal 0x99EDD8), flag = 1, call `CALL EAX` at **0x763B4C**.
   - left: W = left item obj +0x14, name "lhand" (literal 0x99EDD0), flag = 0, call `CALL EAX` at **0x763E7E**.
   - Stack at call (thiscall, ECX = body): `[ESP]=W, [ESP+4]=name, [ESP+8]=flag, [ESP+0xC]=scale`. Empty hand passes W = 0x7F000000.
2. Equipped item ids live at creature **+0x238 (right)** and **+0x23C (left)**, read through FUN_0076e610 (slot 0x10 / 0x20). Sentinel 0x7F000000 = empty.
3. Creature **+0x278 / +0x27C** are the ids last attached, written at 0x7637E0 lines 136 and 196. They are not the equipped ids.
4. Comparing a weapon Gob to the item ids is NOT feasible read-only: no Gob-to-item back-link was found, and resolving an id needs the engine object lookup (FUN_0073F530 -> FUN_0078BD80 -> FUN_0053DFB0, not decoded).

## EVIDENCE
- 007637e0.c:54-61: entry flags from events "AppearanceForceUpdate" (local_24) and "Loading" (local_20).
- 007637e0.c:65-68: `local_3c = FUN_0076e610(creature,0x10)` (R id), `local_14 = FUN_0076e610(creature,0x20)` (L id); `local_44`/`local_34` = FUN_0073f530(id) (item objects).
- 0076e610.c: `case 0x10: return *(param_1+0x238)`; `case 0x20: return *(param_1+0x23c)`.
- 007637e0.c:78 and 138: a hand is re-attached only if `creature[0x9e] != id` (creature+0x278) or forced. Same for 0x27C / `[0x9f]`.
- 007637e0.c:93 and 153: empty hand calls vtbl+0x7C with `0x7f000000` as W.
- 007637e0.c:119-122 (right armed), 179-182 (left armed): name, flag 1/0.
- asm 0x763AF9 `PUSH 0x99EDD8` (rhand); 0x763B21 saves W; 0x763B2E `PUSH 1`; 0x763B4C `CALL EAX`.
- asm 0x763E2B `PUSH 0x99EDD0` (lhand); 0x763E60 `PUSH 0`; 0x763E7E `CALL EAX`.
- 007ed830 = `MOV EAX,[ECX+0x68]; RET` (body object; C4 note, `.claude/research/holster_c4_weapon_attach.md`).
- Correction: `.claude/research/holster_c1_postcopy.md:51` says "creature+0x9E (rhand id) / +0x9F (lhand id)". Those are dword indexes (`local_2c[0x9e]`), i.e. +0x278 / +0x27C, the last-attached ids, not the equipped ids.
- Shared constants: `patch_manager_mods/_shared_src/K2Addr.h:385-386` (+0x278/+0x27C); `patch_manager_mods/preview_3d_src/Preview3dProvider.cpp:60` (CrRightItemOff 0x238, CrLeftItemOff 0x23c, NoItemId 0x7F000000).
- Holster status: `patch_manager_mods/holster_src/Holster.cpp:586-591` kWorldKnowsHand = false (review F1, hand unknown in world).

## FINDING
- The hand is not stored on the Gob. It exists only as arg flag and name at the 0x763B4C / 0x763E7E call. Body vtbl+0x7C (target not decoded) consumes it.
- Two read-only routes exist:
  (A) Entry hook at 0x7637E0 (already present). Compare creature+0x238 vs +0x278 and +0x23C vs +0x27C. This shows which hand is being (re)attached in the event, but not which Gob. The right hand is attached before the left, in one call, so this needs ordering.
  (B) Call-site hook at 0x763B4C and 0x763E7E. Read W, flag and name from the stack. This gives W and hand together. It needs a 9-byte detour window (2-byte CALL EAX + 7-byte MOV [EBP-4],-1) and the stolen bytes replayed. Same pattern as the bone_test push patch.
- The Gob's parent+0x1D8 (Holster.cpp:729) is the owner, so hand plus owner gives the full identity.

## OPEN
1. Is W at 0x763B4C the same pointer as the child Gob seen by the Holster GOB/ATTACH probes (behavior+0x10)? Not verified.
2. Body vtbl+0x7C target and what it does with flag (not decoded).
3. Exact 0x763E7E stack layout (only the right side is fully read; left matches by symmetry).
4. Whether every in-world weapon attach goes through 0x7637E0 (callers: 0085cc00, 0085b940, 00804f10, 0076d890, 0080efb0, 0080fe80).
5. Object lookup 0x53DFB0 would be needed for any pure-compare variant.

## VERDICT
NEEDS-LIVE-PROBE

Static answer is complete: the hand is the flag arg at 0x763B4C / 0x763E7E, and creature+0x238/+0x23C are the equipped ids. A live probe must confirm that W is the Gob the Holster sees and that the flag reads 1 right / 0 left before the holster logic depends on it.
