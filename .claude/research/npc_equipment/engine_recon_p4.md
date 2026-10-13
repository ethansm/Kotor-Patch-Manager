# Engine recon p4: baseitems row fields used by CanEquipItem

Binary: /home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe.unpatched_vanilla
Steam vanilla, PE x86, sha256 6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef, Ghidra 12.1.4 via REA (provider ghidra). All addresses are Steam vanilla.

Status: PARTIAL. The REA session dropped mid-run (see OPEN). Unresolved items are marked UNKNOWN.

## 1. Procedure roles

| proc | role | evidence |
|---|---|---|
| FUN_005bd6a0 (CanEquipItem) | calls FUN_005bda40(item, baseitem_id), FUN_005d3650(item, 1), then a slot-mask test | pseudo CONFIRMED (call-site instruction addresses not resolved) |
| FUN_005bda40 | required-feat test: returns 0 if row+0x04 == 0; returns 1 if FUN_006b83f0(0x5d) != 0; otherwise every feat from FUN_006d4fc0(i), i < row[+0x80], must pass FUN_006b83f0 | pseudo CONFIRMED; FUN_006b83f0 semantics UNKNOWN |
| FUN_005d3650(item, strict) | race rule on row+0xB4, gender test on row+0xB8, override scan at item+0x28C | pseudo CONFIRMED |
| FUN_006d4fc0(i) | returns uint16 at *(row+0x7C) + 2*i if i < byte row[+0x80], else 0xFFFF | pseudo CONFIRMED (ECX = row) |
| FUN_006d5200(idx) | row accessor. table[+0x00] = count, table[+0x04] = data. Row = data + idx*0xC8 when 0 <= idx < count, else 0 | pseudo CONFIRMED |
| FUN_006d6e30 | returns FUN_006d5200(item[+0x0C]) using table *(0x00A1B4D0)+0x34 | asm CONFIRMED: 0x6d6e3c MOV ECX,[EAX+0xC]; 0x6d6e42 MOV EDX,[0xA1B4D0]; 0x6d6e52 MOV ECX,[ECX+0x34]; 0x6d6e55 CALL 0x6d5200 |
| FUN_006d5240 | baseitems loader. Literal "BASEITEMS" at 0x99a890, xref 0x6d5292. Count written to table+0x00. Rows are 200 bytes (0xC8) at table+0x04 | pseudo CONFIRMED; xref CONFIRMED |

## 2. Field table

| offset | column name | string literal | loader push | loader store (pseudo) | consumer | evidence quality |
|---|---|---|---|---|---|---|
| +0x04 | EquipableSlots | 0x99a880 | 0x6d541e (PUSH 0x99a880) | `*(uint*)(row+4) = value` (block local_8=4) | FUN_005bda40: row+4 == 0 rejects. FUN_005bd6a0 end test: `(row+4 & slotmask) != 0` | literal-to-store INFERRED (block order). Store and consumer CONFIRMED |
| +0x1A | UNKNOWN | UNKNOWN | UNKNOWN | `*(byte*)(row+0x1A) = value`, or 0 if column absent (block local_8=0x11) | none of CanEquipItem, FUN_005bda40, FUN_005d3650 reads +0x1A | literal UNKNOWN; store CONFIRMED |
| +0x7C / +0x80 | ReqFeat0..ReqFeat4 (format "ReqFeat%d") | 0x99a654 | 0x6d6971 and 0x6d6a26 (two 5-iteration loops; the second appends via FUN_006d4f80) | +0x7C = uint16 array pointer; +0x80 = byte count | FUN_005bda40 loop; FUN_006d4fc0 | consumer CONFIRMED; literal-to-loop INFERRED (format string plus loop structure) |
| +0xB4 | DroidOrHuman | 0x99a668 | 0x6d67ad (PUSH 0x99a668) | `*(byte*)(row+0xB4) = value`, default 0 if absent (block local_8=0x31) | FUN_005d3650 race rule | literal-to-offset INFERRED (alternative: block 0x30 at +0xAC; brief label agrees). Consumer CONFIRMED |
| +0xB8 | UNKNOWN | UNKNOWN | UNKNOWN | `*(uint*)(row+0xB8) = value` if column present (block local_8=0x38, after the ReqFeat loops) | FUN_005d3650 gender test | store CONFIRMED; literal UNKNOWN |

Other loader literals (pushes in FUN_006d5240, not mapped to an offset):
- ItemClass, 0x99a858, push 0x6d5541. Offset UNKNOWN.
- GenderSpecific, 0x99a848, push 0x6d55b3. Offset UNKNOWN. It is pushed before the ReqFeat loops (0x6d6971) in address order, so INFERRED not to be the +0xB8 reader.
- GENDER (0x99ab24) and Gender (0x999b5c) have xrefs only outside FUN_006d5240 (0x6e87ec, 0x6b02c2, and others). Not the loader literal for +0xB8. PCGender (0x9a0284) not checked.

## 3. Numeric encodings

Race rule, row+0xB4 (byte), from FUN_005d3650 (CONFIRMED from pseudo):
- 0 = no rule.
- 1 = creature race must equal 6.
- 2 = creature race must equal 5.
- Any other value = return 0 (reject).
- Race is a short at [[this+0x1198]+0xE0]. The implicit `this` (ECX) is not resolved, and the object identity is UNKNOWN.
- On mismatch: returns 0 when param_2 != 0 (CanEquipItem passes 1), returns 1 when param_2 == 0.
- The rule is also gated by local_14, which FUN_00603f60(10,0) and FUN_006042e0(...,0x81) can clear. Meaning UNKNOWN.
- Meaning of race indexes 5 and 6 UNKNOWN (racialtypes not resolved). The column name DroidOrHuman does not give the meaning.

Gender test, row+0xB8 (uint), from FUN_005d3650 (CONFIRMED from pseudo):
- Gender byte is read from [[this+0x1198]+0xEC]. Bit = 1 << (gender & 0x1F).
- Default path (no FUN_00603f60(0x2d) or FUN_00603f60(0x3d) flag): if (row+0xB8 & bit) != 0 the function returns 0. If the bit is clear it returns 1.
- As written, a SET bit rejects. The brief's "gender mask" is a name only; polarity comes from the branch structure.
- Gender enum values UNKNOWN.

EquipableSlots, row+0x04 (uint): bitmask. CanEquipItem normalises *param_2 (0x40000 gets |0x10, 0x80000 gets |0x20) and tests `(row+4 & *param_2) != 0`. Individual slot-bit names UNKNOWN.

ReqFeat, row+0x7C / row+0x80: every listed feat must pass FUN_006b83f0 (AND). Feat IDs are uint16. The count is a byte. 0xFFFF is returned for out-of-range indexes.

## 4. Evidence summary
- CONFIRMED: consumer logic in FUN_005bd6a0, FUN_005bda40, FUN_005d3650, FUN_006d4fc0, FUN_006d5200, FUN_006d6e30. Loader store offsets +0x04, +0x1A, +0x7C/+0x80, +0xB4, +0xB8. Literal addresses and xrefs. Table path *(0x00A1B4D0)+0x34.
- INFERRED: literal-to-offset links for EquipableSlots (+0x04), ReqFeat%d (+0x7C/+0x80), DroidOrHuman (+0xB4). Pseudo SEH-block numbering assumed to follow address order.
- UNKNOWN: +0x1A column name. +0xB8 column name. Race IDs 5 and 6 meaning. Gender enum. Semantics of FUN_006b83f0, FUN_00603f60, FUN_006042e0. Object at this+0x1198.

## 5. OPEN
1. REA session dropped ("No app is open") after read_function_instructions and procedure_references returned over 120 KB each. Both were saved to tool-results files this agent cannot read. The binary was not reopened, per rule. The +0x1A and +0xB8 literals are unresolved.
2. To close: dump the FUN_006d5240 listing in chunks. Map each PUSH imm32 in 0x99a600..0x99abff to its block (local_8 = 0x11 and 0x38).
3. Style reference engine_recon_p2.md not read (no file-read tool in this agent).
4. 0x8c1d05 (FUN_008c19f0) also references BASEITEMS. Not analysed.
5. FUN_006d4f80, FUN_006d4ef0, FUN_006b83f0, FUN_00603f60 not decompiled.
