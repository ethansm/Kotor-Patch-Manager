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

## C001
kind: function
address: 0x0069fee0
candidate name: `CSWBaseItemArray::Load`
facts:
- size 3948 B, RET 0
- callers 1: 0x0069dea0
- callees: 0x00919723 operator new (CALL), 0x00710840 (CALL), 0x0071ad40 (CALL), 0x0071c3d0 (CALL), 0x0091d67e (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), ...
- indirect: 0x006a0e29 slot 0x0
- strings: 0x00999080 "PrimaryAbil", 0x0099908c "AlignRstrctType", 0x0099909c "AlignRestrict", ...
- twins: none

## C002
kind: function
address: 0x00418f20
candidate name: `CSWGuiContainer::HandleInputEvent`
facts:
- size 185 B, RET 8
- callers 5: 0x004188e0, 0x004198d0, 0x0041bb70, 0x0041c870, 0x004211c0
- callees: 0x0040e2c0 (CALL), 0x00718c80 CResGFF::ReadFieldINT (CALL), 0x0040fea0 CSWGuiPanel::GetControl (CALL), 0x00419330 CSWGuiControl::AddChildControl (CALL)
- indirect: 0x00418f4e slot 0x4, 0x00418fa7 slot 0x28
- strings: 0x0098788c "Obj_ParentID"
- twins: none

## C003
kind: function
address: 0x0041bb20
candidate name: `CSWGuiProgressBar::Draw`
facts:
- size 67 B, RET 4
- indirect: 0x0041bb3d slot 0xc, 0x0041bb5b slot 0xc
- twins: none

## C004
kind: global
address: 0x0098c27c
candidate name: `Camera::vftable`
facts:
- referenced from 2 function(s)
- in 0x0047ecd0: 0047ed27 c740047cc29800 MOV dword ptr [EAX + 0x4],0x98c27c
- in 0x0047f0a0: 0047f0d2 c741047cc29800 MOV dword ptr [ECX + 0x4],0x98c27c

## C005
kind: function
address: 0x005f6b90
candidate name: `CSWMapInfo::Initialize`
facts:
- size 548 B, RET 48
- callers 1: 0x00523870 CSWSArea::LoadAreaHeader
- callees: 0x0091c860 (CALL), 0x00919723 operator new (CALL), 0x0091c630 (CALL)
- twins: none

## C006
kind: function
address: 0x0085ef00
candidate name: `C2DA::GetINTEntry`
facts:
- size 1217 B, RET 0
- callers 1: 0x00788d90 CClientExoAppInternal::StartServices
- callees: 0x00919723 operator new (CALL), 0x00710840 (CALL), 0x0071ad40 (CALL), 0x0071c3d0 (CALL), 0x007107f0 (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), ...
- indirect: 0x0085f05c slot 0x0, 0x0085f0a6 slot 0x0, 0x0085f340 slot 0x0, 0x0085f391 slot 0x0
- strings: 0x009a451c "RESREF", 0x009a4524 "SoundSetType", 0x009a4534 "SoundSet"
- twins: none

## C007
kind: function
address: 0x008d4f60
candidate name: `PowersLevelUp::OnInputEvent`
facts:
- size 108 B, RET 4
- callers 1: 0x008d4260
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x008d65d0 (CALL), 0x008d6850 (CALL)
- twins: none

## C008
kind: global
address: 0x0098c27c
candidate name: `CSWGuiControl`
facts:
- referenced from 2 function(s)
- in 0x0047ecd0: 0047ed27 c740047cc29800 MOV dword ptr [EAX + 0x4],0x98c27c
- in 0x0047f0a0: 0047f0d2 c741047cc29800 MOV dword ptr [ECX + 0x4],0x98c27c

## C009
kind: function
address: 0x0071b0b0
candidate name: `C2DA::GetInt`
facts:
- size 271 B, RET 12
- callers 12: 0x0053ad00 CServerExoAppInternal::AutoSaveOnEnteringModule, 0x0053af20 CServerExoAppInternal::DoPCAutosave, 0x0059ec70 CSWSEffectListHandler::OnApplyVisualEffect, 0x006de350 CSWSCreature
- callees: 0x0071c320 (CALL), 0x0071aef0 C2DA::GetColumnIndex (CALL), 0x007337c0 CExoString::operator= (CALL), 0x007338d0 (CALL), 0x00733db0 (CALL), 0x00733a60 (CALL), ...
- twins: none

## C010
kind: function
address: 0x009454b0
candidate name: `CResTGA::OnServiced`
facts:
- size 905 B, RET 0
- callees: 0x00945860 (CALL), 0x00945990 (CALL)
- indirect: 0x0094579a slot 0xc
- twins: none

## C011
kind: function
address: 0x0042a3d0
candidate name: `GLBufferPool::Resize`
facts:
- size 159 B, RET 4
- callees: 0x004491c0 (CALL), 0x0091c6aa operator delete[] (CALL)
- indirect: 0x0042a418 unresolved
- twins: none

## C012
kind: function
address: 0x0047a0c0
candidate name: `GetFontTexture`
facts:
- size 71 B, RET 0
- indirect: 0x0047a0f2 slot 0x40
- twins: none
