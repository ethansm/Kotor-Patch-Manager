# C7 recon: readers of [animbase+0xC5] in the combat and flourish path (Steam swkotor2.exe 6a522e71)

Method: static dumps (decomp/*.asm, .c), cg.py, field_census.py, static_facts.py. No live REA.
Base: `0x007ED830` = CSWCObject::GetAnimationBase, asm `MOV EAX,[EAX+0x68]` (007ed83a), so every base is the CLIENT creature's anim base.

## The four functions

| fn | +0xC5 read (asm) | base object | use of the byte |
|---|---|---|---|
| 0x0079C9C0 GetClientMeleeAnimation | 0x0079CA3A `MOV DL,[ECX+0xC5]` | client attacker, via 0x798160 + 0x78BDF0 then 0x7ED830 | stack arg5 into 0x0076F300 (melee attack anim); read only when param_5==0 |
| 0x0079CA90 GetClientRangedAnimation | 0x0079CAEC `MOV AL,[EDX+0xC5]` | same as melee | param_2 into 0x0076F940 (ranged attack anim) |
| 0x0079F430 PlayerFlourishWeapons | 0x0079F520 `MOV AL,[EDX+0xC5]` | client creature from 0x58AE90 (server to client creature) | gate: 1 or 8 returns at once |
| 0x0066B8A0 Global::ExecuteCommandCreatureFlourishWeapon (VM 847) | 0x0066B9CD `MOV AL,[EDX+0xC5]` | client creature from 0x58AE90 (server object) | gate: 1 or 8 returns 0 at once |

Callers: melee wrapper 0x6DB410 <- CSWSCreature::ResolveMeleeAttack 0x6DEA90 <- ResolveAttack 0x6DACF0 (path verified, 3 hops). Ranged wrapper 0x6DB460 <- ResolveRangedAttack 0x6DAF00. Flourish PlayerFlourishWeapons <- HandleInputEvent 0x7B12C0 and 0x8993F0.

## What each selects
- Melee 0x0076F300 switch on style: 1, 2, 3, 4, 8 pick attack families. Style 8 (unarmed) uses `0xF7+idx` or table DAT_00A10610/18, with feat 0xCF gating. No case 0. Corroboration: patch_manager_mods/10_equipment_3d_character.md line 32 (N8 uses g8a1..2 attack clips).
- Ranged 0x0076F940 switch on style: 5, 6, 7, 9 only. Style 8 hits no case and returns 0, unless 0x76FE00 is nonzero (returns 0x120).
- Flourish (both): styles 1 and 8 return before the weapon-slot flourish, the generic 0x27AE/0x27AD flourish and the +0x464 cooldown (=1000). Any other style plays the weapon flourish.

## Does a wrong (unarmed) value hit the first attack?
- Yes, by static reading. The melee read is per attack call, with no cache. A value of 8 at attack time selects the unarmed melee family.
- Ranged attacks with 8 get no anim from 0x76F940.
- Flourish with 8 is silently skipped (no error, no cooldown set).

## Ordering hint (combat entry)
- Combat entry = CSWSCreature::SetCombatState 0x0057A560. No direct or depth-6 path to UpdateWeaponClass 0x76D590. It writes no +0xC5.
- It calls client 0x7682B0 at 0x57A93D (names suspect: sets client +0x138 combat flag). That calls 0x7680A0 with arg 0, which returns before any +0xC5 read.
- 0x7682B0 reads equipped item IDs (0x76E610 at 0x768368). It reads item IDs, not the wield byte.
- So combat entry does NOT recompute +0xC5. The value persists from the last write.

## Whole-program census of +0xC5 (field_census --all, 15 hits)
- Writers (2): UpdateWeaponClass 0x76D7D2 `MOV [EDX+0xC5],AL` (base = GetAnimationBase); CSWCAnimBase::Constructor 0x85F8C3 (=0).
- UpdateWeaponClass callers: EquipToInventorySlot 0x76E130, FUN_00804F10, FUN_0085A1F0 (via SetAppearance 0x76D200), CreateSimple.
- Other readers (13): ResolveRangedSpecialAttack 0x6DB546; FUN_007680A0 0x768265 (only when arg==1; flourish and footstep callers pass 0, so no read); UpdateAttackData 0x76F0E5, 0x76F1AB (-> melee and ranged workers, only caller AddFireForgetAnimation 0x770310); PlayHitSound 0x7721C0; SetOverlayAnimation_2 0x8602EB; FUN_00863650 0x863670, 0x863D3C (ready-idle row, holster-relevant).

## Naming caveats
- 0x6DB460 is labelled CSWSCreature::IsInTotalDefense_2 [wrong]. It is the ranged wrapper called from ResolveRangedAttack.
- 0x7682B0 is labelled UpdateRollingFootstepSound [low]. It sets the combat flag.
