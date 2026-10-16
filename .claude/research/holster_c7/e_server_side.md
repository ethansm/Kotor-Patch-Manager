# Holster C7e: does a client-only +0xC5 override reach the server? (swkotor2.exe Steam 6a522e71, static $0)

Tags: VERIFIED = decompile/asm/census read this run. GUESS = inferred. Build: Steam 6a522e71 (not 7751f7d8). "Unarmed" = wield 0 (per C6 notes).
Method: static dumps, field_census, cg.py, vanilla exe vtable bytes. No live REA.

## Q1 Server state vs +0xC5
1. VERIFIED +0xC5 lives in CSWCAnimBase, reached as CSWCObject::GetAnimationBase 0x7ED830 = `*(this+0x68)` (decomp/7e/007ed830.c).
2. VERIFIED field_census --offset 0xc5 --all: 15 hits. Readers: 0x66B8A0, 0x6DB4A0, 0x7680A0, 0x76F070, 0x772020, 0x79C9C0, 0x79CA90, 0x79F430. Writers: 0x76D590 UpdateWeaponClass, 0x85F7E0 ctor (writes 0). Zero CSWSCreature-owned hits. Caveat: exact displacement only.
3. VERIFIED server weapon truth = server inventory: RangeWeaponEquipped 0x56E1E0 -> CSWInventory::GetItemInSlot(0x10) 0x6D0620; IsWieldingLightsaber 0x5763E0 -> CSWSItem::IsLightsaber 0x615C80. Slot 0x10 = right hand: GUESS.
4. VERIFIED server style byte is CSWSCreature+0x504, not +0xC5. Writer 0x58B780; readers SetAnimation 0x56D350, SaveCreature 0x574500, RecomputeAmbientAnimationState 0x57A990.
5. VERIFIED +0x504 input is server +0xFF5 (SetState effect byte; writers 0x59CBA0, 0x59DB50), not a weapon class.

## Q2 Who picks anim ids
6. VERIFIED idle/ready is server-chosen: SetAnimation 0x56D350 maps 0x2711/0x2710 through +0x504 to 0x274A..0x28B2, then SetDirtyFlag 0x54A430. Client applies the id in ClientServerSyncHandler_Animation 0x769620 / 0x7F2380. Client +0xC5 is not read on this path.
7. VERIFIED vtable 0x994F5C (vanilla exe bytes): +0x7C = 0x56D350 SetAnimation, +0x94 = 0x563460 GetDead, +0x9C = 0x5413C0. RecomputeAmbientAnimationState calls SetAnimation(10000).
8. VERIFIED attack ids are chosen in client code but called synchronously by the server. ResolveRangedAttack 0x6DAF00 -> 0x6DB460 -> 0x7404D0 -> 0x79CA90 -> reads +0xC5 -> 0x76F940. ResolveMeleeAttack 0x6DEA90 -> 0x6DB410 -> 0x740500 -> 0x79C9C0 -> reads +0xC5 -> 0x76F300.
9. VERIFIED server consumes that id. Ranged shot loop count = GetWeaponDischargeShots 0x6DE660 (2DA "Shots" column via 0x6DE350). Melee impact time = GetWeaponImpactTime 0x6DE2B0.
10. VERIFIED ResolveRangedSpecialAttack 0x6DB4A0: asm 0x6DB524 CALL 0x58AE90 (GetClientCreature) -> 0x6DB535 CALL 0x7ED830 -> 0x6DB546 MOV CL,[EAX+0xC5]. Wield 5/6/7 plus feat 0xE4/E5/E6 gates an extra shot (CGameEffect 0x5E4920, queue 0x6E4630). Extra shot is 20% when +0x5C==2. Runs on first shot only.
11. VERIFIED ranged selector 0x76F940: switch on wield yields ids only for 5/6/7/9; wield 0-4 and 8 fall to 0 unless flag 0x76FE00 is set (GUESS on flag meaning).
12. VERIFIED script cmd 847 FlourishWeapon 0x66B8A0 reads +0xC5 via 0x7ED830; wield 1 or 8 returns early. Otherwise it calls server SetCombatState 0x57A560. Cmd table 0x665F50 implies VM-side (GUESS).

## Client-only consumers (cosmetic)
13. VERIFIED UpdateVisibleWeapons 0x76D890 (via EquipToInventorySlot 0x76E130), PlayHitSound 0x772020, UpdateAttackData 0x76F070, AIUpdate_2 0x7664B0 -> 0x7680A0, PlayerFlourishWeapons 0x79F430.
14. VERIFIED UpdateWeaponClass 0x76D590 rewrites +0xC5 from EquipToInventorySlot 0x76E130. A holster override is lost on any client equip.

## Risks (in-combat unless noted)
R1. Holstered ranged weapon, still server-equipped: shot count from wield-0 id (row not read) differs from weapon row. Live if holster keeps server slot 0x10 equipped.
R2. Melee impact timing from wield-0 id (0x6DE2B0) desyncs hit timing.
R3. Wield 5/6/7 special-shot gate closes while holstered: extra ranged shot lost.
R4. Out of combat: FlourishWeapon gate (wield 1/8) opens while holstered: hidden weapon can flourish (cosmetic).
R5. Client equip rewrites +0xC5 (item 14): override state flips without server change.
R6. Save/load: SaveCreature reads +0x504, not +0xC5: no save-state change (GUESS, no overlapping reads checked).

## Not affected
- Server idle/ready anim selection (SetAnimation 0x56D350, RecomputeAmbientAnimationState 0x57A990) does not read +0xC5.

## Verdict
PARTIAL. Out of combat the override is mostly cosmetic (flourish gate and client visuals only). "Cosmetic only" is REFUTED for combat: server resolvers read the client byte through GetClientCreature.
