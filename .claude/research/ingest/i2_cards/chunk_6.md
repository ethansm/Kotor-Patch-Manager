# Name check cards (blind)

Each card gives static facts about one address in swkotor2.exe and ONE candidate name for it.
Judge only from the facts on the card. Do not read any other file.

- consistent: the facts fit the name (what it calls, reads, compares, strings, how it is used).
- inconsistent: the facts contradict the name (e.g. "Vtable" but the address is a float divisor,
  "Iat" but it is never CALLed through, a GUI name on an audio routine).
- unsure: the facts say too little either way.

Naming noise to ignore: prefixes Addr/k/G_/g_, suffixes Global/Val/Flag. Iat = import address table
slot (CALL dword ptr [addr]); Vtable/Vtbl = vtable pointer stored into an object (MOV [reg],addr).

Write one TSV line per card: `card<TAB>verdict<TAB>quote` where quote is the one fact line
(copied verbatim, may be shortened) that decided it. Header line: `card	verdict	quote`.

## C61
kind: function
address: 0x0085d430
candidate name: `PreviewSetAlign`
facts:
- size 3704 B, RET 4
- callers 3: 0x0076d200, 0x0077d270, 0x0084e6f0 CSWGuiInGameCharacter::SetStats
- callees: 0x0077d800 (CALL), 0x00734f60 CExoString::CompareNoCase(char*) (CALL), 0x00585120 CSWSCreature::Disguise (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), 0x00733780 CExoString::Des
- indirect: 0x0085d5a0 slot 0x98, 0x0085e229 slot 0x98, 0x0085e244 slot 0x40
- strings: 0x009a4488 "TEXMEVIL", 0x009a4494 "TEXLEVIL", 0x009a44a0 "TEXIEVIL", ...
- globals: 0x00a1b490 READ, 0x00a1b4a4 READ, 0x00a1b4a4 READ, ...
- twins: none

## C62
kind: global
address: 0x009862cc
candidate name: `AddrSlotGlGetIntegerv`
facts:
- referenced from 17 function(s)
- in 0x004289d0: 00428aa0 ff15cc629800 CALL dword ptr [0x009862cc]
- in 0x00428d70: 00428e36 ff15cc629800 CALL dword ptr [0x009862cc]
- in 0x00431140: 00431150 ff15cc629800 CALL dword ptr [0x009862cc]

## C63
kind: global
address: 0x009f5d54
candidate name: `AddrCutFlag`
facts:
- referenced from 2 function(s)
- in 0x004687e0: if (DAT_009f5d54 != 0) {
- in 0x00468b50: if (DAT_009f5d54 != 0) {

## C64
kind: global
address: 0x009f5d30
candidate name: `AddrShadowsOn`
facts:
- referenced from 4 function(s)
- in 0x0046a630: if (DAT_009f5d30 != 0) {
- in 0x0046c5c0: if ((param_1[0x38] != 0) && (DAT_009f5d30 != 0)) {
- in 0x00478ca0: DAT_009f5d30 = 1;

## C65
kind: global
address: 0x009a7bac
candidate name: `PausePanelVtable`
facts:
- referenced from 1 function(s)
- in 0x008b91f0: 008b922e c701ac7b9a00 MOV dword ptr [ECX],0x9a7bac

## C66
kind: function
address: 0x00418360
candidate name: `SendContainerClose`
facts:
- size 69 B, RET 8
- callers 13: 0x00413650, 0x0084c3a0 CSWGuiInGameCharacter::Constructor, 0x00880740, 0x008973d0, 0x008988b0, ...
- callees: 0x00734010 (CALL), 0x00462320 (CALL), 0x004183b0 (CALL)
- twins: none

## C67
kind: global
address: 0x009a7bac
candidate name: `AddrShadowDist`
facts:
- referenced from 1 function(s)
- in 0x008b91f0: 008b922e c701ac7b9a00 MOV dword ptr [ECX],0x9a7bac

## C68
kind: global
address: 0x0099244c
candidate name: `EventClassGlobal`
facts:
- referenced from 5 function(s)
- in 0x0056efd0 (CSWSCreature::EventHandler): else if (param_2 == DAT_0099244c) {
- in 0x005f2fe0 (CSWSPlaceable::EventHandler): if (param_2 == DAT_0099244c) {
- in 0x00617b40 (CSWSDoor::EventHandler): if (param_2 == DAT_0099244c) {

## C69
kind: global
address: 0x0099449c
candidate name: `ServerIdMask`
facts:
- referenced from 2 function(s)
- in 0x00539d30 (CServerExoAppInternal::ClientToServerObjectId): param_1 = ~DAT_0099449c & param_1;
- in 0x00632f60: param_2 = param_2 | DAT_0099449c;

## C70
kind: function
address: 0x0073a130
candidate name: `GuiSceneCtor`
facts:
- size 196 B, RET 4
- callers 1: 0x0064dde0
- callees: 0x0073a560 (CALL), 0x009196fd operator delete (CALL)
- strings: 0x0099d39c "CExoLinkedListInternal::Remove() Position paramter is NULL"
- globals: 0x00a1b488 READ
- twins: none

## C71
kind: function
address: 0x007e5d80
candidate name: `GetControlledCreature`
facts:
- size 21 B, RET 0
- callers 65: 0x0058e990 CSWSCombatRound::EndCombatRound, 0x00656af0, 0x00670760 Global::ExecuteCommandDestroyObject, 0x00742750, 0x00744260, ...
- callees: 0x007e5da0 Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0) (CALL)
- twins: none

## C72
kind: global
address: 0x009f6100
candidate name: `G_GlFinishFlag`
facts:
- referenced from 1 function(s)
- in 0x004845a0: DAT_009f6100 = 0;
