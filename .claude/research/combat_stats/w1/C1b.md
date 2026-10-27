# C1b Defense breakdown and damage mitigation (Steam swkotor2.exe)

Static recon from decompile dumps (`ghidra_dumps/steam_6a522e71`). All addresses are Steam. `stats+X` = CSWSCreatureStats, `creature+X` = CSWSCreature (`stats+0x24`).

| stat | what it means for the player | formula / parts | computed at | storage | shown in vanilla UI? | how a patch reads it | conf |
|---|---|---|---|---|---|---|---|
| Defense total | number the attack roll must beat | sum of all rows below; -4 if IsDebilitated (FUN_006daa00) | 0x006a5110 (return low word) | return value | via feedback only | call GetArmorClassVersus(attacker, touch=0) | high |
| Breakdown record | per-part AC for the current attack | +0x105 armor, +0x106 dex(1), +0x107 dex(0), +0x108 class+ArmorMult, +0x109 natural, +0x10a dodge+effects, +0x10b feat AC, +0x10c flat-footed | 0x006a5110 writes always; text only if DAT_00a7e084 | record from 0x005908c0 (name map: CSWSCombatRound::GetAttack, steam DB verified) | feedback text only | read record bytes after the call | med |
| Armor AC | worn armour | stats+0xfa + (+0xfc - +0xfd); normal attacks only (param_3 == 0) | 0x006a5110 | stats bytes +0xfa,+0xfc,+0xfd | yes "(Armor AC)" | read stats bytes | med |
| Shield AC | off-hand shield | stats+0xfb + (+0x100 - +0x101); normal only | 0x006a5110 | stats bytes +0xfb,+0x100,+0x101 | yes "(Shield AC)" | read stats bytes | med |
| Natural AC | hide / natural armour | stats+0x102 - +0x103 + FUN_006bad60(); normal only | 0x006a5110 | stats bytes +0x102,+0x103 | yes "(Natural AC)" | read stats bytes | med |
| Class / ArmorMult base | class armour + float table mult | sum over class slots (stats+0xab + i*0x28, count +0x8d) of FUN_006f2ed0(GetClassLevel); plus FUN_0091c860 (_ftol2) of ArmorMult if stats+0x11ac==0 and +0x250!=0 | 0x006a5110 | record +0x108 only | no | recompute, or read record | low |
| Dex mod | DEX contribution | GetDEXMod(1) added only in visible branch (or invisible and negative); GetDEXMod(0) stored +0x107 but NOT summed | 0x006a5110 -> 0x006b5db0 | record +0x106/+0x107 | yes "(Dex Mod)" | call GetDEXMod | med |
| Dodge mod | defender dodge vs this attacker | min(10, stats+0x104 - stats+0x105), only if GetVisibleListElement(attacker)!=0 and flag bit 1 | 0x006a5110 -> 0x005d1750 | stats bytes +0x104,+0x105 | yes "(Dodge Mod)" | read stats bytes | med |
| Feat dodge | feat-based dodge | local_a8 hard-set 0, so never added or shown | 0x006a5110 | none | no | none (dead path) | high |
| Effect AC (dodge/natural/armor/shield/deflection) | AC granted by effects | scan creature effects (ptr +0x148, count +0x14c); type ids DAT_00996170/74/78/7c/80 = iprp_acmodtype rows 0..4; sign = (ushort+8 == DAT_00995fdc) | 0x006a5110 | effect ints via CGameEffect::GetInteger 0x005e4e60 | yes "(Effect AC ... Bonus)" | scan same effects | med |
| Stance bonus | Total Defense stance | +4, or +6 if a class slot (ids 5/0x0d/0x10/0x02) has level > 3; needs IsInTotalDefense (0x006daac0) | 0x006a5110 | record +0x10a | yes "(Total Defense Stance Bonus)" | call IsInTotalDefense | med |
| Lightsaber form mod | unlabeled AC from saber form | if IsWieldingLightsaber (0x005763e0): switch on creature+0x1110 (0x102..0x108) gives +3/+2/-2/-5/+1/-4 style offsets | 0x006a5110 | not labelled | no (" + %d") | recompute from creature+0x1110 | med |
| Damage reduction (DR) | flat per-hit cut, bypass by power | effect DAT_00995f4c: ints[0]=amount, [1]=damage power (clamp <= DAT_0099616c), [2]=limit | cmd 0x00673e10; item prop 0x005b04f0 | creature effect list | not confirmed | GetInteger on the effect | med |
| Damage resistance (per type) | flat per-hit cut per damage type, optional pool | effect type 12 (08_shield_bar.md:322): ints[0]=type (iprp_damagetype 0..12), [1]=per-hit, [2]=limit pool, 0=unlimited | cmd 0x00674020; item prop 0x005b0820 | effect ints | partly (shield bar) | GetInteger; type 12 | med |
| Damage immunity (%) | percent cut for a damage type | vtable slot 0xa8 call in OnApplyDamage, 6 args; target unidentified (candidate: GOG DoDamageImmunity 0x006a9ae0, Steam addr unmapped) | 0x005959e0 call at 0x00595d28 | not located | no | needs Steam address of DoDamageImmunity | low |
| Per-type damage totals | damage by type on a hit | 15 int slots on damage effect: [0..12] per type, -1 = absent, [14] = total; mitigation runs on total | 0x005959e0 | effect ints; reduced copy to creature+0x18c | no | GetInteger 0..14 on damage effect | med |
| Energy shield (absorb pool) | pool that soaks hits | effect type 2 sub 9: ints[2]=pool, decremented in place per hit | absorb FUN_00541fd0 (08_shield_bar.md:316-322) | effect ints[2] | yes (shield bar mod) | GetInteger(2) on type 2 effect | med |
| Damage shield (reflect) | attacker takes damage back | ResolveDamageShields builds a 15-slot damage effect for the attacker; RollDice(NumDice) | 0x006e3650 (called from 0x00595efb) | new effect via ApplyEffect 0x00544210 | no | none | med |

## Evidence
- 0x006a5110: `local_7c = local_2c + local_24 + local_80 + local_3c + local_1c + local_7c;` (dodge, deflection, stance, misc; no natural or armour term)
- 0x006a5110: `else if (((param_3 == 0) && (local_e8 == DAT_0099617c)) && (local_80 < local_dc))` (negative deflection branch keyed to shield type constant)
- 0x006a5110: `if (local_a8 != 0)` after the only assignment `local_a8 = 0;` (feat dodge dead)
- 0x0067127c: caller is Global::ExecuteCommandDoTouchAttack (cg.py callers), so the param_3 flag is the touch switch (hypothesis from name)
- 0x005959e0: `local_5c = (**(code **)(*local_6c + 0xac))(local_58,local_5c,local_7c & 0xffff,0,0);` then 0xa8 (6 args) then 0xa4 (type arg); mitigation on total
- 0x005959e0: `*(int *)(local_6c[99] + local_54 * 4) = aiStack_50[local_54];` (per-type copy stored to creature+0x18c)
- 0x00673e10 / 0x00674020: arg order `FUN_005e4e80(0,local_20)`, `(1,local_24)`, `(2,local_18)` (DR) and type-first for DRes
- 0x005b0820: `FUN_005e4e80(0, 1 << ((byte)local_18 & 0x1f))` (item-prop DRes stores a type BITMASK in arg0; script path stores an INDEX)
- 0x006e1540 (CSWSCreature::ResolveDamage): callees GetDamageRoll 0x006adec0, ResolveOnHitEffect 0x006e2fa0; no mitigation here
- iprp_acmodtype.tsv: 0 AC_Dodge, 1 AC_Natural, 2 AC_Armor, 3 AC_Shield, 4 AC_Deflection; iprp_damagetype.tsv: 13 types 0 Bludgeoning .. 12 Energy

## Anomalies (candidate bugs, not live-verified)
- Negative deflection branch tests DAT_0099617c (shield) not DAT_00996180 (deflection); its max feeds the deflection sum with a plus sign.
- Effect natural bonus (local_28) is shown in feedback but never summed into the total.
- Feat dodge line is dead code (local_a8 always 0).

## Open questions
- Names of vtable slots 0xa4/0xa8/0xac on CSWSCreature; 0xa4 is passed type 0 only when flags & 0x4007 set. Meaning unresolved.
- Do stats+0xfc/+0xfd/+0x100/+0x101/+0x102/+0x103 always equal the effect scan maxima? Live check needed.
- Numeric values of DR effect type (DAT_00995f4c) and DRes type (DAT_00995f24 = 12?) not resolved; iprp_damagered and damage-immunity percent tables not exported.
- Meaning of damage-effect slots 14..21 beyond [14]=total; slot 16 is set to 1000 in ResolveDamageShields.
- Damage-immunity percent source (vtable 0xa8 target) not yet mapped to a Steam address.
