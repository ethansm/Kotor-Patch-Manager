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

## C85
kind: global
address: 0x0099608c
candidate name: `EffDeflectDec`
facts:
- referenced from 4 function(s)
- in 0x005b2650 (CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionDecrease): local_28 = (undefined2)DAT_0099608c;
- in 0x005b2780 (CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionDecrease): if ((((*(ushort *)(puVar2 + 2) == DAT_0099608c) && (uVar3 = FUN_005e4e60(0), uVar3 == uVar1))
- in 0x00677c70 (Global::ExecuteCommandEffectBlasterDeflectionDecrease): local_2c = (undefined2)DAT_0099608c;

## C86
kind: global
address: 0x009862cc
candidate name: `ScreenW`
facts:
- referenced from 17 function(s)
- in 0x004289d0: 00428aa0 ff15cc629800 CALL dword ptr [0x009862cc]
- in 0x00428d70: 00428e36 ff15cc629800 CALL dword ptr [0x009862cc]
- in 0x00431140: 00431150 ff15cc629800 CALL dword ptr [0x009862cc]
