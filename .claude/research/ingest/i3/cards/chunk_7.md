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

## C073
kind: function
address: 0x008b2f70
candidate name: `CSWGuiContainer::SetupGiveMode`
facts:
- size 1410 B, RET 4
- callers 2: 0x008b2850, 0x008b35d0
- callees: 0x0084a640 (CALL), 0x0051c8d0 (CALL), 0x005f7a30 CSWPartyTable::GetPartyItems (CALL), 0x00606160 CItemRepository::ItemListGetItemObjectID (CALL), 0x0073fec0 CClientExoApp::ServerToClientObj
- indirect: 0x008b3177 slot 0xa0, 0x008b3398 slot 0x88, 0x008b344c slot 0x8, 0x008b346e slot 0x8
- twins: none

## C074
kind: function
address: 0x0065afd0
candidate name: `CSWSMessage::HandlePlayerToServerGuiContainerMessage`
facts:
- size 289 B, RET 8
- callers 1: 0x0065f8a0
- callees: 0x00632ef0 (CALL), 0x005e36a0 (CALL), 0x005e3de0 (CALL), 0x005e3e40 (CALL), 0x0051c080 CServerExoApp::GetObjectArray (CALL), 0x0053dfb0 CGameObjectArray::GetGameObject (CALL), ...
- indirect: 0x0065b08d slot 0x48, 0x0065b0d6 slot 0x38
- globals: 0x00a1b4a4 READ
- twins: none

## C075
kind: function
address: 0x0041fc30
candidate name: `CSWGuiListBox::AddControls`
facts:
- size 345 B, RET 16
- callers 38: 0x00758e30, 0x007592f0, 0x00759820, 0x0075a7c0, 0x007d3750, ...
- callees: 0x0041d290 (CALL), 0x004142e0 (CALL), 0x0083ea60 CExoArrayList<uint32>::Add (CALL), 0x00421720 (CALL), 0x0041ea50 (CALL)
- twins: none

## C076
kind: function
address: 0x005e39d0
candidate name: `CSWMessage::ReadDWORD`
facts:
- size 144 B, RET 4
- callers 32: 0x005e3c90, 0x0065bd50, 0x0065be70, 0x00662060, 0x006626d0, ...
- callees: 0x005e3620 (CALL)
- twins: none

## C077
kind: function
address: 0x0075bde0
candidate name: `CSWGuiMessageBox::SetMessage_2`
facts:
- size 55 B, RET 4
- callees: 0x0071dee0 (CALL)
- indirect: 0x0075be0f slot 0x74
- globals: 0x00a1b4b0 READ
- twins: none

## C078
kind: function
address: 0x0041d290
candidate name: `CSWGuiListBox::ClearItems`
facts:
- size 408 B, RET 0
- callers 68: 0x0041d190, 0x0041fc30, 0x0041fd90, 0x00758a70, 0x0075a7c0, ...
- indirect: 0x0041d2d2 slot 0x6c, 0x0041d335 slot 0x0, 0x0041d3c1 slot 0x4
- twins: none

## C079
kind: function
address: 0x00419880
candidate name: `CSWGuiLabel::Draw`
facts:
- size 67 B, RET 4
- callers 1: 0x0084c1e0
- indirect: 0x0041989d slot 0xc, 0x004198bb slot 0xc
- twins: none

## C080
kind: function
address: 0x00744c20
candidate name: `CSWGuiCharButton::UpdateToolTipText`
facts:
- size 731 B, RET 0
- callers 2: 0x00744b40, 0x00744b90
- callees: 0x0071dee0 (CALL), 0x00734010 (CALL), 0x00734270 CExoString::Format (CALL), 0x00733780 CExoString::Destructor (CALL)
- strings: 0x0099d7c0 "%s %s\n%s: %d/%d\n%s: %d/%d\n%s: %d\n%s: %d"
- globals: 0x00a1b4b0 READ
- twins: none

## C081
kind: global
address: 0x0098764c
candidate name: `CSWGuiImage`
facts:
- referenced from 2 function(s)
- in 0x00416290 (CSWGuiImage::Constructor): 004162fb c7004c769800 MOV dword ptr [EAX],0x98764c
- in 0x00416410: 0041643b c7004c769800 MOV dword ptr [EAX],0x98764c

## C082
kind: function
address: 0x00476b20
candidate name: `CAurInternalGL::RestoreDesktopMode`
facts:
- size 89 B, RET 0
- callees: 0x00000124 (CALL)
- indirect: 0x00476b41 slot 0x24
- imports: ChangeDisplaySettingsA (USER32.DLL)
- twins: none

## C083
kind: function
address: 0x0040e480
candidate name: `CClientExoApp::GetCharacterChangeInProgress`
facts:
- size 285 B, RET 0
- callers 1: 0x008690a0
- twins: none

## C084
kind: function
address: 0x00435720
candidate name: `GLRender::FrameBufferModifications`
facts:
- size 1885 B, RET 0
- callers 1: 0x00478300
- callees: 0x004848b0 (CALL), 0x00000023 (CALL), 0x00000045 (CALL), 0x00000044 (CALL), 0x00484970 (CALL), 0x00000121 (CALL)
- indirect: 0x0043573f slot 0x28, 0x0043576f unresolved, 0x00435796 unresolved, 0x004357aa unresolved, 0x004357b6 unresolved, ...
- globals: 0x00a1b484 READ
- imports: ReleaseDC (USER32.DLL), glDeleteTextures (OPENGL32.DLL), wglDeleteContext (OPENGL32.DLL), ...
- twins: none
