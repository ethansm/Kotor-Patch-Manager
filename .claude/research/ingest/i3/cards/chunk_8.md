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

## C085
kind: function
address: 0x008aa6f0
candidate name: `CSWGuiInGameEquipment::Destroy`
facts:
- size 44 B, RET 4
- callees: 0x008aa750 (CALL), 0x009196fd operator delete (CALL)
- twins: none

## C086
kind: function
address: 0x0040e480
candidate name: `CAurGLBufferPool::LockSlot`
facts:
- size 285 B, RET 0
- callers 1: 0x008690a0
- twins: none

## C087
kind: function
address: 0x00512130
candidate name: `IODispatcher::ReadSync_4`
facts:
- size 419 B, RET 8
- callers 1: 0x00511f80
- callees: 0x00475d50 (CALL), 0x004762f0 (CALL), 0x00476370 (CALL), 0x005122e0 (CALL), 0x00452830 (CALL), 0x00475f70 (CALL), ...
- twins: none

## C088
kind: function
address: 0x0075b370
candidate name: `CSWGuiMessageBox::Constructor_2`
facts:
- size 1389 B, RET 8
- callers 2: 0x007bde80, 0x007be4c0
- callees: 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x00419740 (CALL), 0x0041a510 (CALL), 0x0041ce90 (CALL), 0x0075ae40 (CALL), 0x0075b260 (CALL), ...
- indirect: 0x0075b705 slot 0x4, 0x0075b78c slot 0x98, 0x0075b7fe slot 0x4
- strings: 0x0099e670 "LB_MESSAGE", 0x0099e67c "BTN_CANCEL", 0x0099e688 "BTN_OK", ...
- twins: none

## C089
kind: function
address: 0x00416720
candidate name: `CSWGuiInGameEquipment::Destroy`
facts:
- size 372 B, RET 12
- callees: 0x00734010 (CALL), 0x00718960 CResGFF::GetStructFromStruct (CALL), 0x00710840 (CALL), 0x00718e40 CResGFF::ReadFieldCResRef (CALL), 0x00416090 (CALL), 0x00718c80 CResGFF::ReadFieldINT (CALL)
- strings: 0x00987660 "ROTATESTYLE", 0x0098766c "ROTATE", 0x00987680 "FLIPSTYLE", ...
- twins: none

## C090
kind: function
address: 0x00414ba0
candidate name: `CSWGuiControl::GetHeight`
facts:
- size 17 B, RET 0
- callers 3: 0x005f9220 CSWPartyTable::CreateParty, 0x00721bb0 CERFFile::WriteResource_2, 0x0085ee80
- twins: none

## C091
kind: function
address: 0x00432fa0
candidate name: `GLRender::CreateCubeTexture`
facts:
- size 883 B, RET 0
- callers 1: 0x004265e0 CAurTextureBasic::glImage
- callees: 0x00484100 (CALL), 0x004264b0 (CALL), 0x00000042 (CALL), 0x00000041 (CALL), 0x009196fd operator delete (CALL), 0x00000021 (CALL)
- indirect: 0x00432fcd slot 0x48, 0x00432fdd slot 0x4c, 0x00432fed slot 0x54, 0x0043301d slot 0x3c, 0x0043303e slot 0x5c, ...
- imports: glTexImage2D (OPENGL32.DLL), glTexParameteri (OPENGL32.DLL), glTexSubImage2D (OPENGL32.DLL), ...
- twins: none

## C092
kind: function
address: 0x0071d0a0
candidate name: `C2DA::Free`
facts:
- size 445 B, RET 0
- callers 16: 0x006ed7d0 CTwoDimArrays::UnLoad2DArrays, 0x006f00d0 CTwoDimArrays::UnLoadIPRPCostTables, 0x006f0440 CTwoDimArrays::UnLoadIPRPParamTables, 0x0071ae80, 0x0071c3d0, ...
- callees: 0x0040c170 (CALL), 0x009196fd operator delete (CALL), 0x007338d0 (CALL), 0x007112b0 CRes::Release (CALL)
- twins: none

## C093
kind: function
address: 0x0041bb20
candidate name: `C2DA::GetINTEntry`
facts:
- size 67 B, RET 4
- indirect: 0x0041bb3d slot 0xc, 0x0041bb5b slot 0xc
- twins: none

## C094
kind: function
address: 0x00418f20
candidate name: `CSWGuiControl::LoadFromGFF`
facts:
- size 185 B, RET 8
- callers 5: 0x004188e0, 0x004198d0, 0x0041bb70, 0x0041c870, 0x004211c0
- callees: 0x0040e2c0 (CALL), 0x00718c80 CResGFF::ReadFieldINT (CALL), 0x0040fea0 CSWGuiPanel::GetControl (CALL), 0x00419330 CSWGuiControl::AddChildControl (CALL)
- indirect: 0x00418f4e slot 0x4, 0x00418fa7 slot 0x28
- strings: 0x0098788c "Obj_ParentID"
- twins: none

## C095
kind: function
address: 0x00417340
candidate name: `CSWGuiTextParams::SetAlignment`
facts:
- size 310 B, RET 12
- callers 5: 0x004197d0, 0x00419a80, 0x00419dc0, 0x0041a8b0, 0x00421100
- callees: 0x00710980 (CALL), 0x00734010 (CALL), 0x0047ec30 (CALL), 0x00416bd0 CSWGuiTextParams::operator= (CALL), 0x00417800 (CALL), 0x009196ee __security_check_cookie (CALL)
- indirect: 0x0041737e slot 0x4, 0x0041741c slot 0x64, 0x0041743d slot 0x74, 0x0041745c slot 0x1c
- twins: none

## C096
kind: function
address: 0x006fa530
candidate name: `CPathfindInformation::ResetAtEndOfPath`
facts:
- size 72 B, RET 0
- callers 5: 0x0055e610 CSWSModule::SetIntraAreaGoal, 0x0057fce0 CSWSCreature::UpdatePersonalSpace, 0x005bfc00 CSWSCreature::AIActionMoveToPoint, 0x00656af0, 0x0065a170
- callees: 0x0091c860 (CALL)
- twins: none
