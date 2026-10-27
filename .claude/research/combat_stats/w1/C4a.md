# C4a: Force power DC, saving throws, Force resistance (Steam swkotor2.exe)

stats = creature+0x1198 (CSWSCreatureStats). Offsets decimal in .c, hex in asm. "asm" = checked in .asm dump.

| stat | what it means for the player | formula / parts | computed at | storage (stats+off) | shown in vanilla UI? | how a patch reads it | conf |
|---|---|---|---|---|---|---|---|
| Force DC | DC a Force power's victim must beat | GetLevel(1) + 5 + CHA mod + WIS mod + Force Focus bonus, +5 only if spell id 159/160/161 (Force Scream line). Force Focus: 90 gives +4, else 89 gives +3, else 88 gives +2. No spell-level term. Example: L5, CHA+2, WIS+3, FF = 17 | CSWSCreature::CalculateSpellSaveDC 0x0058a1c0 (asm) | none, computed on demand | UNSURE (not traced) | NWScript GetSpellSaveDC() (nwscript.nss:2603) -> 0x00688750; reads current spell (creature+0x1e4); default DC 14 when no caster | high (formula); med (spell-id source) |
| Caster level term | class levels, drained levels removed | sum GetClassLevel(i,flag=1) for i < class count | CSWSCreatureStats::GetLevel 0x006a4e00; GetClassLevel 0x006ad2f0 | class count +0x8d; slot i class id +0xab+i*0x28; level +0xad+i*0x28 | no | sum those bytes; PCs skip the NPC scaling branch (stats+0x6c IsPC) | high |
| CHA mod | DC term | signed byte | set by 0x006b69a0 (validated KDB) | +0xf8 | no | read +0xf8 | med-high |
| WIS mod | DC term and Will term | signed byte | set by 0x006b6960 (validated KDB) | +0xf6 | no | read +0xf6 | med-high |
| Fortitude total | one total for sheet/scripts | base Fort + CON mod + stored byte + effects (effects only when arg==0) | CSWSCreatureStats::GetFortSavingThrow 0x006ba620 | CON mod +0xf2; stored byte +0x1b0 (name unconfirmed) | UNSURE | script GetFortitudeSavingThrow(oTarget) nwscript.nss:4389 | high (parts); low (stored byte name) |
| Base Fort | per-class save + feat bonus | sum over classes of table[class level] (FUN_006f0920: class rec +0x56+lvl, stride 0x238) + NPC term (creature+0x250/+0x251) + Conditioning feats 22:+3, 21:+2, 13:+1 (first hit) | GetBaseFortSavingThrow 0x006b73a0 | class table in class record | no | not exposed; recompute | med |
| Reflex total | one total | base Ref + DEX mod + stored byte (+0x1b2) + effects (arg==0) | GetReflexSavingThrow 0x006ba740 | DEX mod +0xf0 (GetDEXMod 0x006b5db0) | UNSURE | script GetReflexSavingThrow(oTarget) nwscript.nss:4399 | med |
| Will total | one total | base Will + WIS mod + stored byte (+0x1b1) + effects (arg==0) | GetWillSavingThrow 0x006ba6b0 | WIS mod +0xf6 | UNSURE | script GetWillSavingThrow(oTarget) nwscript.nss:4394 | med |
| Force resistance (FR) | strength vs Force powers | net = max(0, raw - second) | GetForceResistance 0x006bad10 | raw +0x12b; second +0x12c (writer not found) | no script getter | read bytes +0x12b and +0x12c | high (formula); low (second byte) |
| FR source effects | how raw FR is set | OnApply: amount = effect int[0] clamped 0..128; if net < amount, raw = amount. OnRemove: raw = max(Increase amounts) - Decrease amount | 0x0059f710 / 0x0059f7d0; setter 0x006bc3d0 | raw +0x12b | no | EffectForceResistanceIncrease/Decrease (nwscript.nss:3145, 4227) | high |
| Force resist roll | whether one power is resisted | resist only if not absorbed/immune and spell+0x14c==1 and FR>=1; resisted iff d20 + CL < FR. CL = caster GetLevel(0), or caster+0x1070 if caster+0x106c==1 | ResistForce handler 0x006900f0 (GOG-side name ExecuteCommandResistSpell) | spell via creature+0x1e4 | no | not a stat; compute from rows above | med |
| Force immunity | no separate stat found | DoSpellImmunity 0x00543ae0, then DoSpellLevelAbsorption 0x005433a0 run before the roll (-1 = not applicable) | names from gog_cluster_delta_map (medium) | none | no | none found | low |

## Evidence
- 0x0058a1c0 asm: `LEA EAX,[ECX+EDX+0x5]` = GetLevel + CHA(+0xf8) + 5; `CMP 0x9f/0xa0/0xa1` adds 5; `PUSH 0x5a/0x59/0x58` adds 4/3/2.
- 0x00688750 asm (~line 99): `PUSH [EBP-8]` (spell id, init 0xffffffff) then `CALL 0x0058a1c0`; default DC `0xe`.
- 0x0062ee30 asm (~line 39): caller passes a real spell id from `vtable+0xb8`.
- 0x006ba620 decomp: `return cVar2 + cVar1 + cVar3 + local_8`; `local_8` (effects) only set `if (param_2 == 0)`.
- 0x006b73a0 decomp: feat ids 0x16/0x15/0xd give +3/+2/+1; NPC term from creature+0x250/+0x251 (`SaveMult`).
- 0x006bad10 decomp: `*(char*)(p+299) - *(char*)(p+300)`, floored at 0.
- 0x006900f0 asm: `MOV ECX,[..+0x1198]; CALL 0x006bad10` (target stats); `CALL 0x006a2000(1,0x14)` = d20; `CMP EDX,0x2` on spell+0x14c.
- feat.tsv rows 88/89/90 (Force Focus +2/+3/+4) and 13/21/22 (Conditioning +1/+2/+3).
- nwscript.nss:2907 `int ResistForce(object oSource, object oTarget);`

## Open questions
1. Stored save bytes +0x1b0/+0x1b1/+0x1b2: who writes them, and is it the template fort/ref/will bonus? Not traced.
2. Second FR byte +0x12c: no writer found. A second GetForceResistance variant at 0x006a7d40 is unverified.
3. ResistForce returns -1 (truthy in NWScript) when spell+0x14c is neither 1 nor 2 and no absorption/immunity (asm 0x0069043d-0x00690451). Check the real flag values in the spell data.
4. Will/Reflex per-class tables and the NPC SaveMult/LevelMult term (0x0091c860 is named _ftol2_sse in the DB) are not resolved.
5. Vanilla UI: no evidence found for whether the sheet shows saves, Force DC, or FR.
6. Negative-level count, the DEX cap gate FUN_006daa00(1), and the object+0x298 fallback DC in GetSpellSaveDC are not traced.
