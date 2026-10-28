# Scene

25 functions in the Steam DB for Scene.

## 0x00464470 Scene::SetFogColor

- Size 143 B, RET 4
- Callers (0 total): none
- Callees (1):
  - 0x004644b8 CALL 0x0044b8a0
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x00464c30 Scene::SetFog

- Size 22 B, RET 4
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

## 0x00464c50 Scene::GetFogRange

- Size 35 B, RET 8
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

## 0x00464c80 Scene::SetFogRange

- Size 31 B, RET 8
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

## 0x00464ca0 Scene::SetSceneFocus

- Size 38 B, RET 12
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

## 0x00464cd0 Scene::IsGeomFadeEnabled

- Size 25 B, RET 0
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

## 0x004651b0 Scene::EnableAnimations

- Size 21 B, RET 0
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

## 0x004651d0 Scene::DisableAnimations

- Size 21 B, RET 0
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

## 0x004651f0 Scene::IsAnimating

- Size 20 B, RET 0
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

## 0x00465210 Scene::SetBlindLights

- Size 30 B, RET 4
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

## 0x00465290 Scene::GetObjectA

- Size 102 B, RET 4
- Callers (0 total): none
- Callees (1):
  - 0x004652d2 CALL 0x009370c7 _stricmp
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x00465bb0 Scene::ChildAdd

- Size 112 B, RET 4
- Callers (0 total): none
- Callees (5):
  - 0x00465bcb CALL 0x00495f20
  - 0x00465be0 CALL 0x004659f0
  - 0x00465bef CALL 0x00473f40
  - 0x00465c02 indirect
  - 0x00465c15 CALL 0x00473f40
- Imports: none
- Indirect slots (1):
  - 0x00465c02 slot +0x1a8 (00465bfc MOV EAX,dword ptr [EDX + 0x1a8])
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x00465c20 Scene::ChildRemove

- Size 112 B, RET 4
- Callers (0 total): none
- Callees (5):
  - 0x00465c3b CALL 0x00495fb0
  - 0x00465c50 CALL 0x00465ae0
  - 0x00465c5f CALL 0x00449170
  - 0x00465c72 indirect
  - 0x00465c85 CALL 0x00449170
- Imports: none
- Indirect slots (1):
  - 0x00465c72 slot +0x1a8 (00465c6c MOV EAX,dword ptr [EDX + 0x1a8])
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x0046c190 Scene::UpdateEmitters

- Size 281 B, RET 0
- Callers (1 total):
  - 0x0046c2b0 Scene::Render (site 0x0046c2de)
- Callees (2):
  - 0x0046c22d CALL 0x004ec080
  - 0x0046c292 CALL 0x004ec080
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x0046fb10 Scene::EnableVisibilityGraph

- Size 44 B, RET 0
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

## 0x0046fb40 Scene::DisableVisibilityGraph

- Size 26 B, RET 0
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

## 0x0046fb60 Scene::GetIgnoreVisibilityGraph

- Size 21 B, RET 0
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

## 0x0046fbd0 Scene::LoadVisibility

- Size 330 B, RET 4
- Callers (0 total): none
- Callees (11):
  - 0x0046fbef CALL 0x0046fb80
  - 0x0046fc01 CALL 0x00475ab0
  - 0x0046fc19 CALL 0x00475f70
  - 0x0046fc47 CALL 0x0091c7ad
  - 0x0046fc64 indirect
  - 0x0046fc98 CALL 0x00475f70
  - 0x0046fcb0 CALL 0x0091c7ad
  - 0x0046fcd0 indirect
  - 0x0046fceb CALL 0x0046feb0
  - 0x0046fcfd CALL 0x00475ea0
  - 0x0046fd0f CALL 0x009196ee __security_check_cookie
- Imports: none
- Indirect slots (2):
  - 0x0046fc64 slot +0x74 (0046fc61 MOV EDX,dword ptr [EAX + 0x74])
  - 0x0046fcd0 slot +0x74 (0046fccd MOV EDX,dword ptr [EAX + 0x74])
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x0046ff70 Scene::GetNumRooms

- Size 20 B, RET 0
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

## 0x0046ffb0 Scene::SetCurrentRoom_2

- Size 72 B, RET 4
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

## 0x00470290 Scene::SpawnRoom

- Size 2598 B, RET 12
- Callers (0 total): none
- Callees (49):
  - 0x004702c1 CALL 0x00428f50
  - 0x004702ca CALL 0x0048e160
  - 0x0047030c CALL 0x00919723 operator new
  - 0x00470334 CALL 0x0046f8e0
  - 0x0047036c CALL 0x00919723 operator new
  - 0x00470390 CALL 0x00495860
  - 0x0047041e CALL 0x0091cff0
  - 0x00470432 CALL 0x0091d000
  - 0x00470448 CALL 0x0046e9b0
  - 0x0047046a CALL 0x00462320
  - 0x004704d8 CALL 0x00461aa0 Gob::SetScene
  - 0x004704f4 CALL 0x00456cd0
  - 0x0047050f CALL 0x00919671
  - 0x00470524 CALL 0x0045fde0
  - 0x00470531 CALL 0x00463ed0
  - 0x00470545 CALL 0x00449230
  - 0x0047059e CALL 0x0046ea30
  - 0x0047066c indirect
  - 0x00470694 indirect
  - 0x004706bc indirect
  - 0x004706e4 indirect
  - 0x0047070a indirect
  - 0x0047071b CALL 0x0046e540
  - 0x0047072c CALL 0x004545a0
  - 0x0047078c CALL 0x00513e20
  - 0x004707ea CALL 0x00513da0
  - 0x00470819 CALL 0x00452d40
  - 0x0047082e CALL 0x00473f40
  - 0x0047085c CALL 0x004491c0
  - 0x0047089c CALL 0x00513e20
  - 0x004708f9 CALL 0x00513da0
  - 0x00470951 indirect
  - 0x00470960 CALL 0x00473f40
  - 0x00470981 CALL 0x00496080
  - 0x0047098e CALL 0x00452d40
  - 0x004709ae CALL 0x004491c0
  - 0x004709b8 CALL 0x00452d40
  - 0x004709f5 indirect
  - 0x00470a23 CALL 0x00474090
  - 0x00470a68 CALL 0x0046e390
  - 0x00470abc indirect
  - 0x00470b61 CALL 0x0050e6e0
  - 0x00470b93 CALL 0x004491c0
  - 0x00470bac CALL 0x004740d0
  - 0x00470bcd CALL 0x00473f40
  - 0x00470bd2 CALL 0x00428f60
  - 0x00470c81 CALL 0x00473e50
  - 0x00470c90 CALL 0x00463b20
  - 0x00470cab CALL 0x009196ee __security_check_cookie
- Imports: none
- Indirect slots (8):
  - 0x0047066c slot +0x2c (00470669 MOV EAX,dword ptr [EDX + 0x2c])
  - 0x00470694 slot +0x24 (00470691 MOV EAX,dword ptr [EDX + 0x24])
  - 0x004706bc slot +0x34 (004706b9 MOV EAX,dword ptr [EDX + 0x34])
  - 0x004706e4 slot +0x48 (004706e1 MOV EAX,dword ptr [EDX + 0x48])
  - 0x0047070a slot +0x30 (00470707 MOV EAX,dword ptr [EDX + 0x30])
  - 0x00470951 slot +0xc (0047094e MOV EAX,dword ptr [EDX + 0xc])
  - 0x004709f5 slot +0x0 (004709f3 MOV EAX,dword ptr [EDX])
  - 0x00470abc slot +0x12c (00470ab6 MOV EAX,dword ptr [EDX + 0x12c])
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x00472460 Scene::RemoveFromSceneManager_2

- Size 71 B, RET 0
- Callers (1 total):
  - 0x0045a190 (site 0x0045a3a5)
- Callees (4):
  - 0x0047246c CALL 0x00476620
  - 0x0047247e CALL 0x00449170
  - 0x0047248e CALL 0x00476620
  - 0x004724a0 CALL 0x00449170
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x004725b0 Scene::SetObjectScale

- Size 46 B, RET 8
- Callers (0 total): none
- Callees (1):
  - 0x004725d6 indirect
- Imports: none
- Indirect slots (1):
  - 0x004725d6 slot +0xcc (004725d0 MOV EAX,dword ptr [EDX + 0xcc])
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x004725e0 Scene::SetGeomFadeProperties

- Size 45 B, RET 4
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

## 0x00472610 Scene::GetObjectScale

- Size 22 B, RET 4
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
