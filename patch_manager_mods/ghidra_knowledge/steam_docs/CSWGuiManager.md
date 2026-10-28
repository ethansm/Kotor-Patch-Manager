# CSWGuiManager

3 functions in the Steam DB for CSWGuiManager.

## 0x00410530 CSWGuiManager::AddPanel

- Size 317 B, RET 12
- Callers (151 total, first 5 shown):
  - 0x00407540 (site 0x0040767e)
  - 0x0040dac0 (site 0x0040dbac)
  - 0x00410900 (site 0x0041095f)
  - 0x005f5770 CSWSPlaceable::CloseInventory (site 0x005f59fb)
  - 0x0069bea0 Global::ExecuteCommandDisplayMessageBox (site 0x0069bfe5)
- Callees (9):
  - 0x0041055b CALL 0x004122a0 CSWGuiManager::PlayGuiSound
  - 0x00410567 CALL 0x00410890
  - 0x004105d0 CALL 0x00410900
  - 0x004105e0 indirect
  - 0x004105e5 CALL 0x00411170
  - 0x00410639 CALL 0x0083ea60 CExoArrayList<uint32>::Add
  - 0x0041064d CALL 0x00410900
  - 0x0041065d indirect
  - 0x00410662 CALL 0x00411170
- Imports: none
- Indirect slots (2):
  - 0x004105e0 slot +0x48 (004105dd MOV EDX,dword ptr [EAX + 0x48])
  - 0x0041065d slot +0x48 (0041065a MOV EAX,dword ptr [EDX + 0x48])
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x004109c0 CSWGuiManager::PopModalPanel

- Size 217 B, RET 0
- Callers (56 total, first 5 shown):
  - 0x00410670 (site 0x004106d1)
  - 0x0075b8e0 (site 0x0075ba21)
  - 0x007983e0 CClientExoAppInternal::HideLoadScreen (site 0x00798454)
  - 0x007caa60 CGuiInGame::HideItemCreateMenu (site 0x007caa91)
  - 0x007cab60 CGuiInGame::HideSubItemCreateMenu (site 0x007cab95)
- Callees (5):
  - 0x00410a05 CALL 0x0049b840
  - 0x00410a0d CALL 0x00411170
  - 0x00410a4e indirect
  - 0x00410a7e indirect
  - 0x00410a90 indirect
- Imports: none
- Indirect slots (3):
  - 0x00410a4e slot +0x44 (00410a4b MOV EAX,dword ptr [EDX + 0x44])
  - 0x00410a7e slot +0x44 (00410a7b MOV EAX,dword ptr [EDX + 0x44])
  - 0x00410a90 slot +0x7c (00410a8d MOV EAX,dword ptr [EDX + 0x7c])
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x004122a0 CSWGuiManager::PlayGuiSound

- Size 94 B, RET 4
- Callers (139 total, first 5 shown):
  - 0x0040ec00 CSWGuiPanel::SetActiveControl (site 0x0040ec84)
  - 0x00410530 CSWGuiManager::AddPanel (site 0x0041055b)
  - 0x00418750 (site 0x00418781)
  - 0x00418fe0 (site 0x00419036)
  - 0x0041a110 (site 0x0041a1ee)
- Callees (1):
  - 0x004122f3 CALL 0x0070ad50
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)
