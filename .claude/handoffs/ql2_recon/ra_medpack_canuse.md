# ra_medpack_canuse: static recon, swkotor2.exe (REA provider ghidra 12.1.4)

Binary sha256 7751f7d8b06c63a21eacbef6e5ded6cb4f497fa960cca6a6cf645552701dc04f (PE x86). Static only, nothing runtime-verified.
All names are Ghidra FUN_* labels. No semantic name below is confirmed.
Evidence ids (REA): callers 0x775600 ev_0695ac82; refs-in 0x775600 ev_39badfbe; asm 0x775600 ev_794e2be1; asm 0x77c780 ev_48e50e74; asm 0x77cc80 ev_d3bd8906; decompile set ev_665b5873, ev_f9cd2928, ev_771a21de, ev_c617ee23, ev_38348148; xrefs 0x752050 ev_14221ee5; info 0x74547e ev_e496f9be; decompile 0x7450e0 ev_ba33a941.

## Notation
- X = [DAT_00A1B4A4]+4 (game object, used by 0x775600 gate and 0x77c780).
- PS = FUN_0073F750(X)+0x98 (pending-state struct used by 0x77C780). Fields: +0x15230 pending flag, +0x15234 slot (-1 = none), +0x15238 pending item handle.
- this = ECX on entry to FUN_0077C780 (UNKNOWN object). this+4 = target id, this+0x24 = position.
- rec = FUN_006D5200(item+0xC) (item record, 200-byte stride). rec+0xAC = base-type byte.
- DAT_00A7EAD4 (byte) selects branch at 0x77C798.

## Q1. Callers of FUN_00775600 (0x775600, 99 bytes, 3 blocks)
Body: 0x775609-0x775617 ECX=X, CALL FUN_0073F810 = *(*(X+4)+0x150) (gate). 0x775623 JZ exit if 0. 0x775658 CALL FUN_00879AF0(item, low16(arg2), byte(arg2>>16), target, pos), ECX = gate value.
procedure_callers(0x775600): 0x77C780, 0x77CC80, 0x842B90, 0x842BD0, 0x899380.
Call sites (procedure_references incoming; 0x775623 is an internal JZ):
1. 0x77C9A2 in FUN_0077C780. Path A. Target this+4, pos this+0x24. CanUseItem check at 0x77C973.
2. 0x77CB26 in FUN_0077C780. Path C2 (pending). Target = selected slot object +4, pos +0x24. CanUseItem check at 0x77CAD8.
3. 0x77CBA3 in FUN_0077C780. Path B (base byte 0x14, no pending). Target this+4. NO CanUseItem check.
4. 0x77CCC3 in FUN_0077CC80. ECX=param_2 (0x77CCC0), target this+4, pos this+0x24.
5. 0x77CCEA in FUN_0077CC80. Zero-vector pos (0x77CCCA-0x77CCE3).
6. 0x842BC1 in FUN_00842B90. Target this+4, pos this+0x24.
7. 0x842CED in FUN_00842BD0. Reached when FUN_00740A20(2,this+4,item,0)==0.
8. 0x8993D6 in FUN_00899380. Target 0x7F000000 (no object), pos = FUN_0073F550([this+0x9C8])+0x24.

Callers of the callers:
- FUN_0077C780 (0x77C780): direct caller 0x752050 only.
- FUN_0077CC80 (0x77CC80): caller 0x77C834 (inside FUN_0077C780, DAT_00A7EAD4 != 0 branch); FUN_0077CDD0 (0x77CDD0), whose callers are 0x77CD00 and 0x899300.
- FUN_00752050 (0x752050): no resolved direct caller. Indirect: xref 0x74547E inside FUN_007450E0 (0x7450E0), which calls FUN_00418AF0(0x27, param_1, FUN_00752050). Event 0x27 meaning UNKNOWN.
- FUN_00842B90, FUN_00842BD0, FUN_00899380: no resolved direct callers (indirect or unknown).
- FUN_00842BD0 also has an alternate branch (FUN_0052E270==0) that calls vtable+0x70 with 0xBAF5 and returns without calling 0x775600. Meaning UNKNOWN.
- Medpac-specific path: none identified by name.

## Q2. CanUseItem candidate: FUN_005D2F80 (name UNCONFIRMED; not shown to equal repo note "R5_canuse")
Call sites: 0x77C818 (DAT!=0 branch), 0x77C973 (path A), 0x77CAD8 (path C2). Args (item_obj, 0, 1). ECX = FUN_0077D800() result at all three sites (no target argument).
Not called on path B (0x77CBA3) or path C1' (store only).
Returns 0 (refuse) if any of:
1. item_obj == 0.
2. FUN_005D3480(item)==0: tag 0x2B entries on item list (item+0x28C count, item+0x2D4 mask). Compares [ECX+0x1198]+0x18A short against bands.
3. FUN_005D32C0(item)==0: tag 0x2C vs [ECX+0x1198] table (+0x8D count, +0xAB stride 0x28).
4. FUN_005D3650(item,1)==0: item+0xB4 in {1,2} needs [ECX+0x1198]+0xE0 == 6 or 5. Other nonzero fails. If bit (1<<[ECX+0x1198]+0xEC) is set in item+0xB8, fails. Tags 0x2D/0x3D checked only when their feature flags are set.
5. FUN_005D3A20(item)==0: tag 0x39 entries via FUN_006B8440.
6. FUN_005D3B60(item)==0: tag 0x3C value vs vfunc+0x88 on ECX.
7. FUN_005D3C90(item)==0: tag 0x3E string match (stricmp). Passes if no 0x3E entries.
8. FUN_005D3EE0(item)==0: tag 0x45 value vs [ECX+0x1198] bytes +0xED,+0xEF,+0xF1,+0xF5,+0xF7.
9. FUN_005D4190(item)==0: when FUN_00734F60("baodur")!=0 and item+0xC in {0x23,0x24,0x25,0x61,0x62,0x63,0x64,0x66}.
10. [ECX+0x43A]==1 and FUN_006A4E00(0) < FUN_00606C30() and FUN_0051BD40()+0xD0 != 0.
11. rec non-null, rec+8!=8, rec+9!=0: (rec+0x1B) - [ECX+0x14F] > 1 fails. Then first set bit b (0..19) of rec+4 requires FUN_005BDA40(item)!=0.
12. rec+0xAC in {'-' (0x2D), 0x1A}: vfunc+0x9C(0)==vfunc+0x98(1) fails (param_3 is 1 at all sites).
FUN_005BDA40(item): rec+4==0 passes. Otherwise each sub-entry (rec+0x80 count, FUN_006D4FC0(i)) must pass FUN_006B83F0.
- Item-property/type dependent: rec+0xAC, rec+0xB4, rec+0xB8, rec+4, rec+8, rec+9, rec+0x1B, rec+0xC, rec+0x80, item tag list (+0x28C/+0x2D4).
- Target-dependent: NONE inside FUN_005D2F80.
- Pending/busy check: NOT inside FUN_005D2F80. It is in the caller: PS+0x15230 at 0x77C9E9 and in FUN_00752050; PS+0x15234==-1 exit at 0x77C9FA to 0x77CC71. Path C2 clears PS+0x15230 at 0x77CA6F before the CanUseItem call.
- Healing-vs-shield/stim difference inside CanUseItem: UNKNOWN (item records not read).

## Q3. Target selection and routing (FUN_0077C780, entry block 0x77C83E)
Routing:
- DAT_00A7EAD4 != 0: 0x77C79E. CanUseItem at 0x77C818, then FUN_0077CC80 (0x77C834).
- DAT_00A7EAD4 == 0:
  - Path A: *(FUN_0073FB90())==1 (0x77C8B5), or rec+0xAC==0x2F (0x77C8E9). CanUseItem at 0x77C973, then 0x77C9A2. Target this+4, no selection state.
  - PS+0x15230 != 0 (pending), 0x77C9E9: path C2. If slot==-1 at 0x77C9FA, return silently (0x77C9FC). Else clear pending, CanUseItem at 0x77CAD8, call 0x77CB26 with slot object target.
  - PS+0x15230 == 0, 0x77C9F0: rec+0xAC==0x14 (0x77CB75): path B, call 0x77CBA3 with this+4, no check.
  - Else path C1': store PS+0x15238=handle (0x77CBE8), PS+0x15230=1 (0x77CC26), no call. 0x77CC30 sets PS+0x15234=-1 (0x77CC67).
Portrait callback FUN_00752050 (0x752050; body non-contiguous, 614 bytes). Gate [param_1+0x50]. Selector [param_1+0x5C] (semantics UNKNOWN; cases 1, 2, 3):
- PS+0x15230==0: selector 1 runs the leader branch (FUN_006B9790 picks [in_ECX+0xBA9C] or [in_ECX+0xBE3C]=1, then FUN_00751580). Selector 2..3 gives slot=sel-1 and FUN_00740380(slot,1) (-> FUN_00797F30) under conditions.
- PS+0x15230!=0: PS+0x15234 = sel-1 (sel 3 becomes 1 unless FUN_0073FB90()==3). If leader FUN_007E5DA0(FUN_0073FB90(0)) is null, FUN_00754E20 cancels (slot=-1, flag=0). Else FUN_0077C780(PS+0x15238 | 0x40000000, 0), which reaches path C2 and 0x77CB26.
- The 0x40000000 bit is masked off at 0x77C870 before use. Meaning UNKNOWN.
- Assumption: decompile treats in_ECX of FUN_00752050 as the same struct PS. Offsets match, but the ECX at the 0x752050 to 0x77C780 call was not traced in assembly.
Conclusion: a path-C item (not base 0x2F, not 0x14, not party-state 1) is only stored, and 0x775600 is reached after the portrait callback fires. Path A and B use this+4 as target with no selection.

## Open / UNKNOWN
1. Base-type byte rec+0xAC for medpacks (g_i_medeqpmnt01-03) and for shield/stim: NOT read. Item table lookup not performed.
2. Whether the in-game hook sits at 0x775600 entry or after the gate. If entry, medpacks are not base 0x14 (path B), because they never reached 0x775600.
3. Stale pending: if PS+0x15230==1 with slot -1, any later non-0x2F, non-party-state click silently returns (0x77C9FC), including 0x14 items. Who sets/clears it outside these paths: UNKNOWN.
4. Portrait selector [param_1+0x5C] and event 0x27 semantics: UNKNOWN. Whether 0x752050 is ever dispatched in play: UNKNOWN.
5. Equivalence of FUN_005D2F80 and repo "R5_canuse": UNCONFIRMED.
6. Meaning of creature-side bytes [ECX+0x1198]+0xEC and +0xE0; repo note "creature +0xEC = commandable" not checked here.
7. Indirect callers of 0x842B90, 0x842BD0, 0x899380, and of FUN_0077CDD0 (callers 0x77CD00, 0x899300): UNKNOWN.
8. FUN_0073F810 flag at +0x150 meaning: UNKNOWN.
