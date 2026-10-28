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

## C049
kind: function
address: 0x00432fa0
candidate name: `CSWGuiListBox::OrganizeUnequal`
facts:
- size 883 B, RET 0
- callers 1: 0x004265e0 CAurTextureBasic::glImage
- callees: 0x00484100 (CALL), 0x004264b0 (CALL), 0x00000042 (CALL), 0x00000041 (CALL), 0x009196fd operator delete (CALL), 0x00000021 (CALL)
- indirect: 0x00432fcd slot 0x48, 0x00432fdd slot 0x4c, 0x00432fed slot 0x54, 0x0043301d slot 0x3c, 0x0043303e slot 0x5c, ...
- imports: glTexImage2D (OPENGL32.DLL), glTexParameteri (OPENGL32.DLL), glTexSubImage2D (OPENGL32.DLL), ...
- twins: none

## C050
kind: global
address: 0x009877d4
candidate name: `CSWGuiControl`
facts:
- referenced from 2 function(s)
- in 0x00418640 (CSWGuiControl::Destructor): 00418669 c700d4779800 MOV dword ptr [EAX],0x9877d4
- in 0x00418990 (CSWGuiControl::Constructor): 004189fb c700d4779800 MOV dword ptr [EAX],0x9877d4

## C051
kind: function
address: 0x005f6b90
candidate name: `CSWGuiControl::GetHeight`
facts:
- size 548 B, RET 48
- callers 1: 0x00523870 CSWSArea::LoadAreaHeader
- callees: 0x0091c860 (CALL), 0x00919723 operator new (CALL), 0x0091c630 (CALL)
- twins: none

## C052
kind: function
address: 0x00880740
candidate name: `CSWGuiInGameCharacter::Constructor`
facts:
- size 3814 B, RET 4
- callers 1: 0x00781590 CClientExoAppInternal::DisplayMainMenu
- callees: 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x0041ce90 (CALL), 0x0040def0 (CALL), 0x00883f10 (CALL), 0x00418990 CSWGuiControl::Constructor (CALL), 0x00417f40 (CALL), ...
- indirect: 0x00880fa6 slot 0x70, 0x008810bd slot 0x18, 0x008810d0 slot 0x14, 0x008810e8 slot 0x74, 0x008810ff slot 0x44, ...
- strings: 0x009a5908 "HD0:CURRENTGAME", 0x009a5918 "HD0:GAMEINPROGRESS", 0x009a592c "mainmenu05", ...
- globals: 0x00a1b490 READ, 0x00a1b494 READ, 0x00a1b4a4 READ, ...
- twins: none

## C053
kind: function
address: 0x00783d80
candidate name: `CClientExoAppInternal::StallServerCode`
facts:
- size 49 B, RET 4
- callers 4: 0x00781be0 CClientExoAppInternal::MainLoop, 0x007ac470 CClientExoAppInternal::ProcessInput, 0x00839ad0, 0x0083a5b0
- callees: 0x007335f0 CExoString::CExoString_copy (CALL)
- twins: none

## C054
kind: function
address: 0x008d4f60
candidate name: `CSWGuiMessageBox::SetMessage`
facts:
- size 108 B, RET 4
- callers 1: 0x008d4260
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x008d65d0 (CALL), 0x008d6850 (CALL)
- twins: none

## C055
kind: function
address: 0x0075bf60
candidate name: `CSWGuiMessageBox::AddMessageLabel_2`
facts:
- size 278 B, RET 0
- callers 1: 0x0075c080
- callees: 0x00919f10 _strlen (CALL), 0x004178b0 (CALL), 0x0041fd90 (CALL), 0x0041e9a0 (CALL)
- indirect: 0x0075bfbe slot 0x4, 0x0075c00d slot 0x8, 0x0075c038 slot 0x4
- twins: none

## C056
kind: function
address: 0x0071cf80
candidate name: `CSWGuiProgressBar::Draw`
facts:
- size 273 B, RET 8
- callers 1: 0x0071c3d0
- twins: none

## C057
kind: function
address: 0x00750790
candidate name: `CSWGuiMainInterface::DrawMap`
facts:
- size 1144 B, RET 4
- callers 1: 0x0074b3f0
- callees: 0x0072fb00 CClientExoApp::GetClientOptions (CALL), 0x0051c8d0 (CALL), 0x0051bd90 CServerExoApp::GetModule (CALL), 0x00710c20 (CALL), 0x00750c10 (CALL), 0x005f7350 (CALL), ...
- indirect: 0x007509eb slot 0x4, 0x00750a1f slot 0x24, 0x00750b2c slot 0x38, 0x00750b5f slot 0x38
- twins: none

## C058
kind: function
address: 0x00656af0
candidate name: `CSWSCombatRound::EndCombatRound`
facts:
- size 13697 B, RET 8
- callers 1: 0x0065f8a0
- callees: 0x005e3de0 (CALL), 0x005e36a0 (CALL), 0x005e3e40 (CALL), 0x00704800 (CALL), 0x0051c3d0 CServerExoApp::SetDebugMode (CALL), 0x0064b210 (CALL), ...
- indirect: 0x00656b68 slot 0x10, 0x00656eeb slot 0x98, 0x00656f3e slot 0x30, 0x00656f4f slot 0x30, 0x00656f74 slot 0x30, ...
- strings: 0x0099876c "b_death", 0x00998774 "b_damage", 0x00998780 "b_attacked", ...
- globals: 0x00a1b490 READ, 0x00a1b4a4 READ, 0x00a1b4a4 READ, ...
- twins: none

## C059
kind: function
address: 0x008b2850
candidate name: `CSWGuiContainer::HandleInputEvent`
facts:
- size 357 B, RET 8
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x0073f810 CClientExoApp::GetSWCMessage (CALL), 0x0087a550 (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x007cad30 CGuiInGame::HideC
- twins: none

## C060
kind: function
address: 0x0044ef20
candidate name: `MdlNodeAnimMesh::InternalGenVertices`
facts:
- size 1665 B, RET 0
- callers 1: 0x004549a0
- callees: 0x00428f80 (CALL), 0x00429330 (CALL), 0x00919723 operator new (CALL), 0x004cd940 (CALL), 0x004cd6c0 (CALL), 0x004cda60 (CALL), ...
- twins: none
