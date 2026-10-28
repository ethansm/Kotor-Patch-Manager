# CSWGuiControl

10 functions in the Steam DB for CSWGuiControl.

## 0x004185d0 CSWGuiControl::SetActive

- Size 36 B, RET 4
- Callers (0 total): none
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x00418640 CSWGuiControl::Destructor

- Size 126 B, RET 0
- Callers (432 total, first 5 shown):
  - 0x00418600 (site 0x00418613)
  - 0x004186c0 (site 0x004186ca)
  - 0x0041a3d0 CSWGuiButton::Destructor (site 0x0041a448)
  - 0x0041a460 (site 0x0041a473)
  - 0x0041ac10 (site 0x0041ac8b)
- Callees (3):
  - 0x0041867c CALL 0x0064ddb0
  - 0x0041868b CALL 0x00733780 CExoString::Destructor
  - 0x0041869a CALL 0x0064ddb0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x00418990 CSWGuiControl::Constructor

- Size 347 B, RET 0
- Callers (16 total, first 5 shown):
  - 0x00418570 (site 0x0041857c)
  - 0x00419740 (site 0x00419769)
  - 0x0041b7f0 (site 0x0041b819)
  - 0x0041bfd0 (site 0x0041bff9)
  - 0x0084c3a0 CSWGuiInGameCharacter::Constructor (site 0x0084c7fd)
- Callees (3):
  - 0x00418a09 CALL 0x0084a640
  - 0x00418a18 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x00418a29 CALL 0x008a5f70
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: base ctor fastcall(ECX=ctrl)
- Named pseudo-C: (no named decompile)

## 0x00418af0 CSWGuiControl::AddEvent

- Size 177 B, RET 12
- Callers (87 total, first 5 shown):
  - 0x00741940 (site 0x00741c32)
  - 0x007450e0 (site 0x00745492)
  - 0x00747210 (site 0x00748a40)
  - 0x00754ed0 (site 0x00755c84)
  - 0x00757c40 (site 0x007586d4)
- Callees (2):
  - 0x00418b59 CALL 0x007a1940
  - 0x00418b96 CALL 0x008a5fb0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: thiscall(ctrl,eventId,target,handler)
- Named pseudo-C: (no named decompile)

## 0x00418df0 CSWGuiControl::Load_2

- Size 300 B, RET 16
- Callers (1 total):
  - 0x0040f620 CSWGuiPanel::InitControl (site 0x0040f651)
- Callees (11):
  - 0x00418e28 CALL 0x00718810 CResGFF::GetListCount
  - 0x00418e33 CALL 0x00733540 CExoString::DefaultConstructor
  - 0x00418e6c CALL 0x00718890 CResGFF::GetListElement
  - 0x00418e79 CALL 0x00733570 CExoString::CStrConstructor
  - 0x00418e9a CALL 0x00718f90 CResGFF::ReadFieldCExoString
  - 0x00418eb3 CALL 0x007337c0 CExoString::operator=
  - 0x00418ebf CALL 0x00733780 CExoString::Destructor
  - 0x00418ecb CALL 0x00733780 CExoString::Destructor
  - 0x00418ed7 CALL 0x007339d0
  - 0x00418ef3 indirect
  - 0x00418f06 CALL 0x00733780 CExoString::Destructor
- Imports: none
- Indirect slots (1):
  - 0x00418ef3 slot +0x48 (00418ef0 MOV EDX,dword ptr [EAX + 0x48])
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: thiscall(ctl, panel, gff, gffList, CExoString* tag)
- Named pseudo-C: (no named decompile)

## 0x004190a0 CSWGuiControl::GetIsSelectable

- Size 135 B, RET 0
- Callers (0 total): none
- Callees (4):
  - 0x004190c3 indirect
  - 0x004190d4 indirect
  - 0x004190f2 indirect
  - 0x00419117 indirect
- Imports: none
- Indirect slots (4):
  - 0x004190c3 slot +0x2c (004190c0 MOV EDX,dword ptr [EAX + 0x2c])
  - 0x004190d4 slot +0x2c (004190d1 MOV EAX,dword ptr [EDX + 0x2c])
  - 0x004190f2 slot +0x4c (004190ef MOV EAX,dword ptr [EDX + 0x4c])
  - 0x00419117 slot +0x4c (00419114 MOV EAX,dword ptr [EDX + 0x4c])
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x00419330 CSWGuiControl::AddChildControl

- Size 28 B, RET 4
- Callers (3 total):
  - 0x00418f20 (site 0x00418fce)
  - 0x00741940 (site 0x00741b01)
  - 0x009017b0 (site 0x009018a7)
- Callees (1):
  - 0x00419341 CALL 0x0083ea60 CExoArrayList<uint32>::Add
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x00419350 CSWGuiControl::GetIsChild

- Size 72 B, RET 4
- Callers (1 total):
  - 0x00412c00 (site 0x0041308e)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x004193a0 CSWGuiControl::GetSelectableParent

- Size 72 B, RET 0
- Callers (9 total, first 5 shown):
  - 0x00412c00 (site 0x00412d54)
  - 0x004134a0 (site 0x004135ff)
  - 0x00419130 (site 0x00419150)
  - 0x00419190 (site 0x004191cd)
  - 0x00419240 (site 0x00419260)
- Callees (1):
  - 0x004193d5 indirect
- Imports: none
- Indirect slots (1):
  - 0x004193d5 slot +0x80 (004193cf MOV EDX,dword ptr [EAX + 0x80])
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x004193f0 CSWGuiControl::SetEnabled

- Size 39 B, RET 4
- Callers (3 total):
  - 0x00419a10 (site 0x00419a1e)
  - 0x0041a210 (site 0x0041a21e)
  - 0x00421300 (site 0x0042130e)
- Callees (0): none
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)
