# Holster C7 recon B: readers of animbase+0xC5 (static, $0)

Base getter: FUN_007ED830 = `mov eax,[ecx+0x68]; ret` (decomp/7e/007ed830.asm), so `FUN_007ED830(creature)` = CSWCAnimBase.
Method: decomp/76, 77, 86 .asm/.c dumps, static_facts, field_census --offset 0xC5 (5 hits, 4 fns, all R; matches below).

| fn | +0xC5 read (addr, asm) | base | used for | zero out of combat |
|---|---|---|---|---|
| FUN_007680A0 | 0x00768265 `8a88c5000000` MOV CL,[EAX+0xc5] | anim base (EAX=FUN_007ED830(this), 0x0076825A) | stored to [EBP-1] (0x0076826B); never loaded | no effect |
| CSWCCreature::UpdateAttackData 0x0076F070 | 0x0076F0E5 `8a82c5000000` MOV AL,[EDX+0xc5] | own anim base (EDX=FUN_007ED830(this)) | own style -> [EBP-2] -> args of FUN_0076F300 (switch param_5), FUN_0076F940 (switch param_2), FUN_0076FF60 (FUN_007702C0 returns IDs for style 2/3/4, else 0) | changes attack-data IDs for an attack anim played while holstered; no crash path seen |
| same | 0x0076F1AB `8a91c5000000` MOV DL,[ECX+0xc5] | TARGET anim base (ECX=FUN_007ED830([EBP-0x18]); [EBP-0x18] = vtbl+0xC of GetGameObject(attack+0x14)) | target style -> [EBP-1] -> FUN_0076FB50 param_3 (==8 special-cased, FUN_006B83F0(0xCF) remaps to 10); set to 0 if target vtbl+0x2C and FUN_0076FE00 both nonzero | 8 on a target takes the ==8 branch (same as a real unarmed target) |
| CSWCCreature::PlayHitSound 0x00772020 | 0x007721C0 `8a88c5000000` MOV CL,[EAX+0xc5] | own anim base (EAX=FUN_007ED830(this); this saved at 0x00772048) | ==3 -> hit category 0x10, only when attack [+0x28] is not 1 or 6; else [+0x28]==2 -> 0x20; else 0; feeds FUN_0076E610 | only changes hit category when old style was 3; no crash path seen |
| CSWCAnimBase::SetOverlayAnimation_2 0x008602C0 | 0x008602EB `8a88c5000000` MOV CL,[EAX+0xc5] | `this` itself (EAX=[EBP-0x38]=ECX at entry; no +0x68 hop) | stored to [EBP-0xD] (0x008602F1); never loaded | no effect |

## Verdicts per function
- FUN_007680A0: read confirmed, value dead. Zeroing cannot change its control flow.
- UpdateAttackData: two reads, both consumed (attacker style, target style).
- PlayHitSound: one read, consumed (hit category).
- SetOverlayAnimation_2: read confirmed, value dead.

## Call context (names from field_census, confidence in brackets)
- UpdateAttackData and PlayHitSound run from 0x00770310 (AddFireForgetAnimation [medium]). UpdateAttackData only inside branch `*param_3 == 1` (0x0077048C JNZ 0x0077096B). PlayHitSound also called from 0x007631D0 and 0x00777E30 (AnimateFireAndForget), so it is not combat-only.
- FUN_007680A0 callers: 0x007682B0 (UpdateRollingFootstepSound [low]; brief called it the combat flag setter), 0x00770310, 0x0066B8A0, 0x007664B0 (AIUpdate_2 [low]), 0x0076D890.
- Grep false positives: 0x0076F1C5 (`ffd0` CALL EAX), 0x008602C5 (PUSH), 0x00772031/0x0077203C/0x007727C5 contain "c5" only in address or XOR/PUSH bytes. None is a +0xC5 read.

## Open
1. FUN_007680A0 tests [EBP+8]==1 (asm 0x007680BC). Standard thiscall layout puts the last-pushed arg at [EBP+8]. Call sites from 0x00770310, 0x007682B0, 0x0066B8A0 push 1 then 0, so [EBP+8]=0 and the combat branch is skipped. Only 0x007664B0 pushes 1,1. Decompile prints these calls as (0,1) and (1,1), consistent with that layout. Verify before any combat-entry claim.
2. 0x0076F1A0 calls FUN_007ED830 on the vtbl+0xC result with no NULL check. Vtbl slot 0xC on the game object is not identified.

## Holster relevance
- Engine unarmed value is 8 per patch_manager_mods/10_equipment_3d_character.md:47 (repo note, not re-derived here).
- Writing 8 changes only: UpdateAttackData ID selection for an attacker, the ==8 target branch in FUN_0076FB50, and PlayHitSound category when the old style was 3.
- The two dead-store readers (FUN_007680A0, SetOverlayAnimation_2) are unaffected by the write.
