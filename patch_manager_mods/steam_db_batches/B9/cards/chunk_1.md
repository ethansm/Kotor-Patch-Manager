# Review cards B9 chunk 1

Answer each card with one TSV row in `steam_db_batches/B9/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B9-1-01

GOG function: CServerExoApp::GetSWSMessage  gog 0x0064bd00  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c2e0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 29 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 29 (callers: UpdateUsedActiveProperties 0x603860, AddItem 0x605090, OpenInventory 0x6062c0)
- best decoy (neighbour/other candidate) scores: 0x0051c600:STR0/CALLEE15/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bce0 CServerExoApp::GetNetLayer -> 0x0051c2c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bd20 CServerExoApp::GetSysAdminList -> 0x0051c300  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (29 of 118): the card's A-CALLEE credit may come from these, not from callees
  0x00667090 CSWSItem::UpdateUsedActiveProperties -> Steam 0x00603860  in candidate callers: yes
  0x006688c0 CItemRepository::AddItem -> Steam 0x00605090  in candidate callers: yes
  0x00669af0 CSWSItem::OpenInventory -> Steam 0x006062c0  in candidate callers: yes
  0x0066a8d0 CSWSItem::DecrementUses -> Steam 0x006070a0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c2e0: 16-byte aligned: YES
  0051c2e0  55             PUSH EBP
  0051c2e1  8B EC          MOV EBP,ESP
  0051c2e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c2e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10010 [dec 65552]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c2e0 FUN_0051c2e0 size=31 ret=0 callers=118 callees=0

undefined4 __fastcall FUN_0051c2e0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10010);
}
```

### decoy: 0x0051c600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c600: 16-byte aligned: YES
  0051c600  55             PUSH EBP
  0051c601  8B EC          MOV EBP,ESP
  0051c603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c600
string literals (0):
direct callees (1):
  0x0052F330  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-02

GOG function: CServerExoApp::GetModuleName  gog 0x0064b7d0  callconv=(blank)  param_size_bytes=4  gog_size=49  gog_retn=4
Candidate Steam address: 0x0051bdb0 (size 49, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051bd90:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b7b0 CServerExoApp::GetModule -> 0x0051bd90  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b810 CServerExoApp::SaveGame -> 0x0051bdf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b2920 CServerExoAppInternal::GetModuleName -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x00798f70 Global::ExecuteCommandGetModuleName -> Steam 0x006839a0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051bdb0: 16-byte aligned: YES
  0051bdb0  55             PUSH EBP
  0051bdb1  8B EC          MOV EBP,ESP
  0051bdb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bdb0
string literals (0):
direct callees (1):
  0x0052F5A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bdb0 FUN_0051bdb0 size=49 ret=4 callers=6 callees=1

undefined4 FUN_0051bdb0(undefined4 param_1)

{
  FUN_0052f5a0(param_1);
  return param_1;
}
```

### decoy: 0x0051bd90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bd90: 16-byte aligned: YES
  0051bd90  55             PUSH EBP
  0051bd91  8B EC          MOV EBP,ESP
  0051bd93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bd90
string literals (0):
direct callees (1):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-1-03

GOG function: CServerExoApp::GetPlayerList  gog 0x0064bcc0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c2a0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 16 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 16 (callers: CloseItemForAllPlayers 0x606d30, DistributeExperience 0x5fe0c0, SwitchPlayerCharacter 0x5fe4b0)
- best decoy (neighbour/other candidate) scores: 0x0051c2e0:STR0/CALLEE8/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bca0 CServerExoApp::GetServerAIMaster -> 0x0051c280  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bce0 CServerExoApp::GetNetLayer -> 0x0051c2c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (16 of 44): the card's A-CALLEE credit may come from these, not from callees
  0x0066a560 CSWSItem::CloseItemForAllPlayers -> Steam 0x00606d30  in candidate callers: yes
  0x00703a70 CSWPartyTable::DistributeExperience -> Steam 0x005fe0c0  in candidate callers: yes
  0x00703e60 CSWPartyTable::SwitchPlayerCharacter -> Steam 0x005fe4b0  in candidate callers: yes
  0x00721900 CSWSArea::SetCurrentWeather -> Steam 0x0052c820  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c2a0: 16-byte aligned: YES
  0051c2a0  55             PUSH EBP
  0051c2a1  8B EC          MOV EBP,ESP
  0051c2a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c2a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10064 [dec 65636]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c2a0 FUN_0051c2a0 size=31 ret=0 callers=44 callees=0

undefined4 __fastcall FUN_0051c2a0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10064);
}
```

### decoy: 0x0051c2e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c2e0: 16-byte aligned: YES
  0051c2e0  55             PUSH EBP
  0051c2e1  8B EC          MOV EBP,ESP
  0051c2e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c2e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10010 [dec 65552]  x1  via ECX
```

## Card B9-1-04

GOG function: CServerExoApp::GetWorldTimer  gog 0x0064bd90  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c370 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 30 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 30 (callers: MainLoop 0x781be0, LoadGame 0x853a00, AIUpdate 0x600f20)
- best decoy (neighbour/other candidate) scores: 0x0051c280:STR0/CALLEE7/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bd70 CServerExoApp::GetActiveTimer -> 0x0051c350  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bdb0 CServerExoApp::GetTimestopTimer -> 0x0051c390  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (30 of 70): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
  0x00664750 CSWSItem::AIUpdate -> Steam 0x00600f20  in candidate callers: yes
  0x0066a8d0 CSWSItem::DecrementUses -> Steam 0x006070a0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c370: 16-byte aligned: YES
  0051c370  55             PUSH EBP
  0051c371  8B EC          MOV EBP,ESP
  0051c373  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c370
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10048 [dec 65608]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c370 FUN_0051c370 size=31 ret=0 callers=70 callees=0

undefined4 __fastcall FUN_0051c370(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10048);
}
```

### decoy: 0x0051c280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c280: 16-byte aligned: YES
  0051c280  55             PUSH EBP
  0051c281  8B EC          MOV EBP,ESP
  0051c283  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c280
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10044 [dec 65604]  x1  via ECX
```

## Card B9-1-05

GOG function: CServerExoApp::UnloadModule  gog 0x0064bf20  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c500 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: MainLoop 0x781be0, LoadPCAutoSave 0x8529e0)
- best decoy (neighbour/other candidate) scores: 0x0051c8d0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bf00 CServerExoApp::Shutdown -> 0x0051c4e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bf40 CServerExoApp::LoadModule -> 0x0051c520  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b4960 CServerExoAppInternal::UnloadModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c500: 16-byte aligned: YES
  0051c500  55             PUSH EBP
  0051c501  8B EC          MOV EBP,ESP
  0051c503  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c500
string literals (0):
direct callees (1):
  0x005315E0  CServerExoAppInternal::UnloadModule
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c500 FUN_0051c500 size=22 ret=0 callers=2 callees=1
THUNK -> 0x005315E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c500(undefined4 param_1)

{
  FUN_005315e0(param_1);
  return;
}
```

### decoy: 0x0051c8d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c8d0: 16-byte aligned: YES
  0051c8d0  55             PUSH EBP
  0051c8d1  8B EC          MOV EBP,ESP
  0051c8d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c8d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-1-06

GOG function: CServerExoApp::GetAutoSavePending  gog 0x0064b9a0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051bf80 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051be80:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b970 CServerExoApp::SetAutoSavePending -> 0x0051bf50  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b9c0 CServerExoApp::SetMoveToModuleStartWaypoint -> 0x0051bfa0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051bf80: 16-byte aligned: YES
  0051bf80  55             PUSH EBP
  0051bf81  8B EC          MOV EBP,ESP
  0051bf83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bf80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B8 [dec 65720]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bf80 FUN_0051bf80 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0051bf80(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x100b8);
}
```

### decoy: 0x0051be80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051be80: 16-byte aligned: YES
  0051be80  55             PUSH EBP
  0051be81  8B EC          MOV EBP,ESP
  0051be83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051be80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10080 [dec 65664]  x1  via ECX
```

## Card B9-1-07

GOG function: CServerExoApp::GetPlayerLanguage  gog 0x0064bc60  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c240 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c220:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bc40 CServerExoApp::GetSoundObjectByGameObjectID -> 0x0051c220  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bc80 CServerExoApp::GetModuleLanguage -> 0x0051c260  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bb290 CServerExoAppInternal::GetPlayerLanguage -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c240: 16-byte aligned: YES
  0051c240  55             PUSH EBP
  0051c241  8B EC          MOV EBP,ESP
  0051c243  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c240
string literals (0):
direct callees (1):
  0x00538040  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c240 FUN_0051c240 size=28 ret=4 callers=4 callees=1
THUNK -> 0x00538040 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c240(undefined4 param_1)

{
  FUN_00538040(param_1);
  return;
}
```

### decoy: 0x0051c220 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c220: 16-byte aligned: YES
  0051c220  55             PUSH EBP
  0051c221  8B EC          MOV EBP,ESP
  0051c223  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c220
string literals (0):
direct callees (1):
  0x00537F60  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-08

GOG function: CServerExoApp::PlayerListChange  gog 0x0064beb0  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0051c490 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c470:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x0064be90 CServerExoApp::MainLoop -> 0x0051c470  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bee0 CServerExoApp::ContinueMessageProcessing -> 0x0051c4c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b8f70 CServerExoAppInternal::PlayerListChange -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0051c490: 16-byte aligned: YES
  0051c490  55             PUSH EBP
  0051c491  8B EC          MOV EBP,ESP
  0051c493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c490
string literals (0):
direct callees (1):
  0x00535C30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c490 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=0 callees=1
THUNK -> 0x00535C30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_00535c30(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x0051c470 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c470: 16-byte aligned: YES
  0051c470  55             PUSH EBP
  0051c471  8B EC          MOV EBP,ESP
  0051c473  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c470
string literals (0):
direct callees (1):
  0x00533830  Game save state machine (per tick)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-1-09

GOG function: CServerExoApp::GetMoveToWorldTime  gog 0x0064ca50  callconv=(blank)  param_size_bytes=28  gog_size=52  gog_retn=28
Candidate Steam address: 0x0051cff0 (size 58, retn 28)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051d030:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064ca10 CServerExoApp::SetMoveToWorldTime -> 0x0051cff0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064ca90 CServerExoApp::GetModulePreviouslyVisited -> 0x0051d070  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bfa90 CServerExoAppInternal::GetMoveToWorldTime -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}

### abi
```
GOG param bytes: 28; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(28/4)*4 = 28 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=28 ; asm RET = 28
Stack Slot Match: TRUE (GOG 28 bytes -> x86 slots 28; ASM RET 28)
```

### prologue
```
prologue for 0x0051cff0: 16-byte aligned: YES
  0051cff0  55             PUSH EBP
  0051cff1  8B EC          MOV EBP,ESP
  0051cff3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cff0
string literals (0):
direct callees (1):
  0x0053CC80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cff0 FUN_0051cff0 size=58 ret=28 callers=2 callees=1
THUNK -> 0x0053CC80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051cff0(undefined4 param_1,undefined1 param_2,undefined1 param_3,undefined1 param_4,
                 undefined2 param_5,undefined2 param_6,undefined2 param_7)

{
  FUN_0053cc80(param_1,param_2,param_3,param_4,param_5,param_6,param_7);
  return;
}
```

### decoy: 0x0051d030 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051d030: 16-byte aligned: YES
  0051d030  55             PUSH EBP
  0051d031  8B EC          MOV EBP,ESP
  0051d033  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051d030
string literals (0):
direct callees (1):
  0x0053CCF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-10

GOG function: CServerExoApp::GetPlaceableByGameObjectID  gog 0x0064bb80  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c160 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=ServerApp::GetPlaceableById | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 12 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 12 (callers: SetPossessor 0x600380, AddAttackActions 0x568540, PackAreaIntoMessage 0x528910)
- best decoy (neighbour/other candidate) scores: 0x0051c1a0:STR0/CALLEE3/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bb60 CServerExoApp::GetTriggerByGameObjectID -> 0x0051c140  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bba0 CServerExoApp::GetStoreByGameObjectID -> 0x0051c180  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007baf80 CServerExoAppInternal::GetPlaceableByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (13 of 18): the card's A-CALLEE credit may come from these, not from callees
  0x00663bb0 CSWSItem::SetPossessor -> Steam 0x00600380  in candidate callers: yes
  0x00681170 CSWSCreature::AddAttackActions -> Steam 0x00568540  in candidate callers: yes
  0x006f4030 CSWSCreatureStats::GetSkillRank -> Steam 0x006b7bb0  in candidate callers: yes
  0x0071dac0 CSWSArea::PackAreaIntoMessage -> Steam 0x00528910  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c160: 16-byte aligned: YES
  0051c160  55             PUSH EBP
  0051c161  8B EC          MOV EBP,ESP
  0051c163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c160
string literals (0):
direct callees (1):
  0x00537D30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c160 FUN_0051c160 size=28 ret=4 callers=18 callees=1
THUNK -> 0x00537D30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c160(undefined4 param_1)

{
  FUN_00537d30(param_1);
  return;
}
```

### decoy: 0x0051c1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c1a0: 16-byte aligned: YES
  0051c1a0  55             PUSH EBP
  0051c1a1  8B EC          MOV EBP,ESP
  0051c1a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c1a0
string literals (0):
direct callees (1):
  0x00537DA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-11

GOG function: CServerExoApp::GetObjectArray  gog 0x0064baa0  callconv=__thiscall  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c080 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 187 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 187 (callers: SetPossessor 0x600380, AcquireItem 0x606e20, StopDialog 0x546900)
- best decoy (neighbour/other candidate) scores: 0x0051c280:STR0/CALLEE19/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064ba80 CServerExoApp::LoadCharacterFinish -> 0x0051c060  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bac0 CServerExoApp::GetGameObject -> 0x0051c0a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (187 of 261): the card's A-CALLEE credit may come from these, not from callees
  0x00663bb0 CSWSItem::SetPossessor -> Steam 0x00600380  in candidate callers: yes
  0x0066a650 CSWSItem::AcquireItem -> Steam 0x00606e20  in candidate callers: yes
  0x006ad5e0 CSWSObject::StopDialog -> Steam 0x00546900  in candidate callers: yes
  0x006ad8b0 CSWSObject::GetNearestObjectByName -> Steam 0x00546bd0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c080: 16-byte aligned: YES
  0051c080  55             PUSH EBP
  0051c081  8B EC          MOV EBP,ESP
  0051c083  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c080
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1005C [dec 65628]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c080 FUN_0051c080 size=31 ret=0 callers=261 callees=0

undefined4 __fastcall FUN_0051c080(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x1005c);
}
```

### decoy: 0x0051c280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c280: 16-byte aligned: YES
  0051c280  55             PUSH EBP
  0051c281  8B EC          MOV EBP,ESP
  0051c283  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c280
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10044 [dec 65604]  x1  via ECX
```

## Card B9-1-12

GOG function: CServerExoApp::SetLoadingModule  gog 0x0064b940  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051bf20 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051bfa0:STR0/CALLEE1/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b920 CServerExoApp::GetLoadingModule -> 0x0051bf00  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b970 CServerExoApp::SetAutoSavePending -> 0x0051bf50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051bf20: 16-byte aligned: YES
  0051bf20  55             PUSH EBP
  0051bf21  8B EC          MOV EBP,ESP
  0051bf23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bf20
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B4 [dec 65716]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bf20 FUN_0051bf20 size=36 ret=4 callers=2 callees=0

void __thiscall FUN_0051bf20(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x100b4) = param_2;
  return;
}
```

### decoy: 0x0051bfa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bfa0: 16-byte aligned: YES
  0051bfa0  55             PUSH EBP
  0051bfa1  8B EC          MOV EBP,ESP
  0051bfa3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bfa0
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-1-13

GOG function: CServerExoApp::GetSoundObjectByGameObjectID  gog 0x0064bc40  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c220 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 10 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 10 (callers: ExecuteCommandSoundObjectGetFixedVariance 0x693120, ExecuteCommandSoundObjectGetPitchVariation 0x6931a0, ExecuteCommandSoundObjectGetVolume 0x693220)
- best decoy (neighbour/other candidate) scores: 0x0051c200:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bc20 CServerExoApp::GetEncounterByGameObjectID -> 0x0051c200  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bc60 CServerExoApp::GetPlayerLanguage -> 0x0051c240  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bb1b0 CServerExoAppInternal::GetSoundObjectByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (10 of 10): the card's A-CALLEE credit may come from these, not from callees
  0x007a8770 Global::ExecuteCommandSoundObjectGetFixedVariance -> Steam 0x00693120  in candidate callers: yes
  0x007a87f0 Global::ExecuteCommandSoundObjectGetPitchVariation -> Steam 0x006931a0  in candidate callers: yes
  0x007a8870 Global::ExecuteCommandSoundObjectGetVolume -> Steam 0x00693220  in candidate callers: yes
  0x007a88f0 Global::ExecuteCommandSoundObjectPlay -> Steam 0x006932a0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c220: 16-byte aligned: YES
  0051c220  55             PUSH EBP
  0051c221  8B EC          MOV EBP,ESP
  0051c223  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c220
string literals (0):
direct callees (1):
  0x00537F60  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c220 FUN_0051c220 size=28 ret=4 callers=10 callees=1
THUNK -> 0x00537F60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c220(undefined4 param_1)

{
  FUN_00537f60(param_1);
  return;
}
```

### decoy: 0x0051c200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c200: 16-byte aligned: YES
  0051c200  55             PUSH EBP
  0051c201  8B EC          MOV EBP,ESP
  0051c203  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c200
string literals (0):
direct callees (1):
  0x00537EF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-14

GOG function: CServerExoApp::GetMoveToModulePending  gog 0x0064b8a0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051be80 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051bf80:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b870 CServerExoApp::SetMoveToModulePending -> 0x0051be50  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b8c0 CServerExoApp::SetMoveToModuleString -> 0x0051bea0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051be80: 16-byte aligned: YES
  0051be80  55             PUSH EBP
  0051be81  8B EC          MOV EBP,ESP
  0051be83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051be80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10080 [dec 65664]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051be80 FUN_0051be80 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0051be80(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10080);
}
```

### decoy: 0x0051bf80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bf80: 16-byte aligned: YES
  0051bf80  55             PUSH EBP
  0051bf81  8B EC          MOV EBP,ESP
  0051bf83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bf80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B8 [dec 65720]  x1  via ECX
```

## Card B9-1-15

GOG function: CServerExoAppInternal::UpdateShutdownTimer  gog 0x007bcab0  callconv=(blank)  param_size_bytes=8  gog_size=541  gog_retn=8
Candidate Steam address: 0x00539a90 (size 286, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00539860:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bca50 CServerExoAppInternal::AddCharListRequest -> 0x00539800  gate_class=REVIEW
- GOG-order neighbour after:  0x007bcce0 Global::UpdateLoadScreen  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007e3eb0 CSWSMessage::SendServerToPlayerShutDownServer -> Steam ?  in candidate callees: n/a
  0x00924e50 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0x30,0xf4240,0x1c9c380,0x3938700} | Steam {0x8*,0x14}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00539a90: 16-byte aligned: YES
  00539a90  55             PUSH EBP
  00539a91  8B EC          MOV EBP,ESP
  00539a93  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00539a90
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x100E4 [dec 65764]  x6  via ECX,EDX
  +0x100E8 [dec 65768]  x6  via ECX,EDX
  +0x100DC [dec 65756]  x5  via ECX,EDX
  +0x100E0 [dec 65760]  x5  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00539a90 FUN_00539a90 size=286 ret=8 callers=1 callees=0

undefined4 __thiscall FUN_00539a90(int param_1,uint param_2,int param_3)

{
  undefined4 uVar1;
  uint uVar2;
  uint uVar3;
  
  if (*(int *)(param_1 + 0x100e4) == 0 && *(int *)(param_1 + 0x100e8) == 0) {
    *(uint *)(param_1 + 0x100e4) = param_2;
    *(int *)(param_1 + 0x100e8) = param_3;
    *(char **)(param_1 + 0x100dc) = "B";
    *(undefined4 *)(param_1 + 0x100e0) = 0;
    uVar1 = 0;
  }
  else {
    uVar3 = (param_3 - *(int *)(param_1 + 0x100e8)) - (uint)(param_2 < *(uint *)(param_1 + 0x100e4))
    ;
    if ((uVar3 < *(uint *)(param_1 + 0x100e0)) ||
       ((uVar3 == *(uint *)(param_1 + 0x100e0) &&
        (param_2 - *(uint *)(param_1 + 0x100e4) < *(uint *)(param_1 + 0x100dc))))) {
      uVar2 = param_2 - *(uint *)(param_1 + 0x100e4);
      uVar3 = *(uint *)(param_1 + 0x100dc);
      *(uint *)(param_1 + 0x100dc) = uVar3 - uVar2;
      *(uint *)(param_1 + 0x100e0) =
           (*(int *)(param_1 + 0x100e0) -
           ((param_3 - *(int *)(param_1 + 0x100e8)) - (uint)(param_2 < *(uint *)(param_1 + 0x100e4))
           )) - (uint)(uVar3 < uVar2);
      *(uint *)(param_1 + 0x100e4) = param_2;
      *(int *)(param_1 + 0x100e8) = param_3;
      uVar1 = 0;
    }
    else {
      *(char **)(param_1 + 0x100dc) = "B";
      *(undefined4 *)(param_1 + 0x100e0) = 0;
      *(uint *)(param_1 + 0x100e4) = param_2;
      *(int *)(param_1 + 0x100e8) = param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00539860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00539860: 16-byte aligned: YES
  00539860  55             PUSH EBP
  00539861  8B EC          MOV EBP,ESP
  00539863  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00539860
string literals (0):
direct callees (2):
  0x0091D220  (unnamed)
  0x0064D790  (unnamed)
struct offsets touched (top 7):
  +0x10028 [dec 65576]  x10  via EAX,ECX,EDX
  +0x1002C [dec 65580]  x10  via EAX,ECX,EDX
  +0x10030 [dec 65584]  x6  via EAX,ECX,EDX
  +0x10034 [dec 65588]  x6  via EAX,ECX,EDX
  +0x10010 [dec 65552]  x1  via EDX
  +0x1001C [dec 65564]  x1  via ECX
  +0x10020 [dec 65568]  x1  via EAX
```

## Card B9-1-16

GOG function: CServerExoApp::GetModuleDescription  gog 0x0064b780  callconv=(blank)  param_size_bytes=0  gog_size=46  gog_retn=0
Candidate Steam address: 0x0051bd60 (size 46, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051bd40:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b760 CServerExoApp::GetServerInfo -> 0x0051bd40  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b7b0 CServerExoApp::GetModule -> 0x0051bd90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0064b7b0 CServerExoApp::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0x4c*} | Steam {0x8*,0x4c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051bd60: 16-byte aligned: YES
  0051bd60  55             PUSH EBP
  0051bd61  8B EC          MOV EBP,ESP
  0051bd63  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bd60
string literals (0):
direct callees (1):
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051bd60 FUN_0051bd60 size=46 ret=0 callers=0 callees=1

int __fastcall FUN_0051bd60(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_0051bd90(param_1);
  if (iVar1 == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = FUN_0051bd90(param_1);
    iVar1 = iVar1 + 0x4c;
  }
  return iVar1;
}
```

### decoy: 0x0051bd40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bd40: 16-byte aligned: YES
  0051bd40  55             PUSH EBP
  0051bd41  8B EC          MOV EBP,ESP
  0051bd43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bd40
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10004 [dec 65540]  x1  via ECX
```

## Card B9-1-17

GOG function: CServerExoApp::SetDebugMode  gog 0x0064bdf0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051c3d0 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c560:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bdd0 CServerExoApp::GetPauseTimer -> 0x0051c3b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064be20 CServerExoApp::GetDebugMode -> 0x0051c400  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c3d0: 16-byte aligned: YES
  0051c3d0  55             PUSH EBP
  0051c3d1  8B EC          MOV EBP,ESP
  0051c3d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c3d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1006C [dec 65644]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c3d0 FUN_0051c3d0 size=36 ret=4 callers=1 callees=0

void __thiscall FUN_0051c3d0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x1006c) = param_2;
  return;
}
```

### decoy: 0x0051c560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c560: 16-byte aligned: YES
  0051c560  55             PUSH EBP
  0051c561  8B EC          MOV EBP,ESP
  0051c563  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c560
string literals (0):
direct callees (1):
  0x005384B0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-18

GOG function: CServerExoApp::GetLoadingModule  gog 0x0064b920  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051bf00 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051be80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b8f0 CServerExoApp::SetExportAllCharactersPending -> 0x0051bed0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b940 CServerExoApp::SetLoadingModule -> 0x0051bf20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051bf00: 16-byte aligned: YES
  0051bf00  55             PUSH EBP
  0051bf01  8B EC          MOV EBP,ESP
  0051bf03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bf00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B4 [dec 65716]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bf00 FUN_0051bf00 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0051bf00(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x100b4);
}
```

### decoy: 0x0051be80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051be80: 16-byte aligned: YES
  0051be80  55             PUSH EBP
  0051be81  8B EC          MOV EBP,ESP
  0051be83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051be80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10080 [dec 65664]  x1  via ECX
```

## Card B9-1-19

GOG function: CServerExoApp::GetEncounterByGameObjectID  gog 0x0064bc20  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c200 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callers: OnApplyDeath 0x597d90, ExecuteCommandGetEncounterData 0x67c610, ExecuteCommandSetEncounterData 0x691280)
- best decoy (neighbour/other candidate) scores: 0x0051c280:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bc00 CServerExoApp::GetWaypointByGameObjectID -> 0x0051c1e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bc40 CServerExoApp::GetSoundObjectByGameObjectID -> 0x0051c220  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bb140 CServerExoAppInternal::GetEncounterByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x0073a680 CSWSEffectListHandler::OnApplyDeath -> Steam 0x00597d90  in candidate callers: yes
  0x00791bf0 Global::ExecuteCommandGetEncounterData -> Steam 0x0067c610  in candidate callers: yes
  0x007a68d0 Global::ExecuteCommandSetEncounterData -> Steam 0x00691280  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c200: 16-byte aligned: YES
  0051c200  55             PUSH EBP
  0051c201  8B EC          MOV EBP,ESP
  0051c203  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c200
string literals (0):
direct callees (1):
  0x00537EF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c200 FUN_0051c200 size=28 ret=4 callers=5 callees=1
THUNK -> 0x00537EF0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c200(undefined4 param_1)

{
  FUN_00537ef0(param_1);
  return;
}
```

### decoy: 0x0051c280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c280: 16-byte aligned: YES
  0051c280  55             PUSH EBP
  0051c281  8B EC          MOV EBP,ESP
  0051c283  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c280
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10044 [dec 65604]  x1  via ECX
```

## Card B9-1-20

GOG function: CServerExoApp::ContinueMessageProcessing  gog 0x0064bee0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c4c0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c490:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064beb0 CServerExoApp::PlayerListChange -> 0x0051c490  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bf00 CServerExoApp::Shutdown -> 0x0051c4e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007baa80 CServerExoAppInternal::ContinueMessageProcessing -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c4c0: 16-byte aligned: YES
  0051c4c0  55             PUSH EBP
  0051c4c1  8B EC          MOV EBP,ESP
  0051c4c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c4c0
string literals (0):
direct callees (1):
  0x00537830  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c4c0 FUN_0051c4c0 size=22 ret=0 callers=0 callees=1
THUNK -> 0x00537830 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c4c0(undefined4 param_1)

{
  FUN_00537830(param_1);
  return;
}
```

### decoy: 0x0051c490 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c490: 16-byte aligned: YES
  0051c490  55             PUSH EBP
  0051c491  8B EC          MOV EBP,ESP
  0051c493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c490
string literals (0):
direct callees (1):
  0x00535C30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-1-21

GOG function: CSWSModule::non_virtual_thunk_to_dtor_CSWSModule  gog 0x00733ef0  callconv=(blank)  param_size_bytes=0  gog_size=8  gog_retn=None
Candidate Steam address: 0x00561190 (size 164, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00561140:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00733d10 CExoArrayList__FILETIME_::Allocate  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00733f00 CSWSRoom::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007294a0 CSWSModule::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8}
imm sets (* = in both, 0 common): GOG {0x1c} | Steam {0xc,0x10}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00561190: 16-byte aligned: YES
  00561190  55             PUSH EBP
  00561191  8B EC          MOV EBP,ESP
  00561193  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00561190
string literals (0):
direct callees (3):
  0x00710AD0  (unnamed)
  0x007111C0  CRes::CancelRequest
  0x00713010  CExoResMan::ReleaseResObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00561190 FUN_00561190 size=164 ret=0 callers=4 callees=3

void __fastcall FUN_00561190(undefined4 *param_1)

{
  int iVar1;
  
  *param_1 = CResHelper<CResIFO,2014>::vftable;
  if ((param_1[2 = byte +0x8] != 0) && (iVar1 = FUN_00710ad0(&DAT_00986925), iVar1 != 0)) {
    if (param_1[1 = byte +0x4] != 0) {
      FUN_007111c0();
      param_1[1 = byte +0x4] = 0;
    }
    iVar1 = FUN_00713010(param_1[2 = byte +0x8]);
    if (iVar1 == 0) {
      if ((undefined4 *)param_1[2 = byte +0x8] != (undefined4 *)0x0) {
        (*(code *)**(undefined4 **)param_1[2 = byte +0x8])(1);
      }
      param_1[2 = byte +0x8] = 0;
    }
  }
  return;
}
```

### decoy: 0x00561140 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00561140: 16-byte aligned: YES
  00561140  55             PUSH EBP
  00561141  8B EC          MOV EBP,ESP
  00561143  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00561140
string literals (0):
direct callees (2):
  0x007107F0  (unnamed)
  0x00710EB0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

## Card B9-1-22

GOG function: CServerExoApp::GetDebugMode  gog 0x0064be20  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c400 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c3b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bdf0 CServerExoApp::SetDebugMode -> 0x0051c3d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064be40 CServerExoApp::HandleMessage -> 0x0051c420  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c400: 16-byte aligned: YES
  0051c400  55             PUSH EBP
  0051c401  8B EC          MOV EBP,ESP
  0051c403  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c400
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1006C [dec 65644]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c400 FUN_0051c400 size=31 ret=0 callers=2 callees=0

undefined4 __fastcall FUN_0051c400(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x1006c);
}
```

### decoy: 0x0051c3b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c3b0: 16-byte aligned: YES
  0051c3b0  55             PUSH EBP
  0051c3b1  8B EC          MOV EBP,ESP
  0051c3b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c3b0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10050 [dec 65616]  x1  via ECX
```

## Card B9-1-23

GOG function: CServerExoApp::LoadModule  gog 0x0064bf40  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0051c4e0 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c4c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bf20 CServerExoApp::UnloadModule -> 0x0051c500  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bf60 CServerExoApp::RunModule -> 0x0051c540  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b4fd0 CServerExoAppInternal::LoadModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0051c4e0: 16-byte aligned: YES
  0051c4e0  55             PUSH EBP
  0051c4e1  8B EC          MOV EBP,ESP
  0051c4e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c4e0
string literals (0):
direct callees (1):
  0x00537870  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c4e0 FUN_0051c4e0 size=32 ret=8 callers=1 callees=1
THUNK -> 0x00537870 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c4e0(undefined4 param_1,undefined4 param_2)

{
  FUN_00537870(param_1,param_2);
  return;
}
```

### decoy: 0x0051c4c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c4c0: 16-byte aligned: YES
  0051c4c0  55             PUSH EBP
  0051c4c1  8B EC          MOV EBP,ESP
  0051c4c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c4c0
string literals (0):
direct callees (1):
  0x00537830  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-1-24

GOG function: CServerExoApp::MainLoop  gog 0x0064be90  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c470 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c450:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064be70 CServerExoApp::Initialize -> 0x0051c450  gate_class=REVIEW
- GOG-order neighbour after:  0x0064beb0 CServerExoApp::PlayerListChange -> 0x0051c490  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b6bb0 CServerExoAppInternal::MainLoop -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c470: 16-byte aligned: YES
  0051c470  55             PUSH EBP
  0051c471  8B EC          MOV EBP,ESP
  0051c473  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c470
string literals (0):
direct callees (1):
  0x00533830  Game save state machine (per tick)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c470 FUN_0051c470 size=22 ret=0 callers=2 callees=1
THUNK -> 0x00533830 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c470(undefined4 param_1)

{
  FUN_00533830(param_1);
  return;
}
```

### decoy: 0x0051c450 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c450: 16-byte aligned: YES
  0051c450  55             PUSH EBP
  0051c451  8B EC          MOV EBP,ESP
  0051c453  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c450
string literals (0):
direct callees (1):
  0x005315A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-1-25

GOG function: CServerExoApp::GetModule  gog 0x0064b7b0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051bd90 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=GetModule | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 68 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 68 (callers: GetSurfaceMaterial 0x7a79f0, OnPanelAdded 0x894320, Draw 0x895cc0)
- best decoy (neighbour/other candidate) scores: 0x0051c160:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b780 CServerExoApp::GetModuleDescription -> 0x0051bd60  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b7d0 CServerExoApp::GetModuleName -> 0x0051bdb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b2840 CServerExoAppInternal::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (68 of 123): the card's A-CALLEE credit may come from these, not from callees
  0x004543f0 CSWCArea::GetSurfaceMaterial -> Steam 0x007a79f0  in candidate callers: yes
  0x00563290 CSWGuiInGameMap::OnPanelAdded -> Steam 0x00894320  in candidate callers: yes
  0x00564c30 CSWGuiMapHider::Draw -> Steam 0x00895cc0  in candidate callers: yes
  0x00663910 CSWSItem::Destructor_2 -> Steam 0x006000e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051bd90: 16-byte aligned: YES
  0051bd90  55             PUSH EBP
  0051bd91  8B EC          MOV EBP,ESP
  0051bd93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bd90
string literals (0):
direct callees (1):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bd90 FUN_0051bd90 size=22 ret=0 callers=123 callees=1
THUNK -> 0x0052F4C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051bd90(undefined4 param_1)

{
  FUN_0052f4c0(param_1);
  return;
}
```

### decoy: 0x0051c160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c160: 16-byte aligned: YES
  0051c160  55             PUSH EBP
  0051c161  8B EC          MOV EBP,ESP
  0051c163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c160
string literals (0):
direct callees (1):
  0x00537D30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-26

GOG function: CServerExoApp::GetServerMode  gog 0x0064bd40  callconv=(blank)  param_size_bytes=0  gog_size=34  gog_retn=0
Candidate Steam address: 0x0051c320 (size 34, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c300:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bd20 CServerExoApp::GetSysAdminList -> 0x0051c300  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bd70 CServerExoApp::GetActiveTimer -> 0x0051c350  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 7): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c320: 16-byte aligned: YES
  0051c320  55             PUSH EBP
  0051c321  8B EC          MOV EBP,ESP
  0051c323  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c320
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10008 [dec 65544]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c320 FUN_0051c320 size=34 ret=0 callers=7 callees=0

undefined2 __fastcall FUN_0051c320(int param_1)

{
  return *(undefined2 *)(*(int *)(param_1 + 4) + 0x10008);
}
```

### decoy: 0x0051c300 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c300: 16-byte aligned: YES
  0051c300  55             PUSH EBP
  0051c301  8B EC          MOV EBP,ESP
  0051c303  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c300
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10068 [dec 65640]  x1  via ECX
```

## Card B9-1-27

GOG function: CServerExoApp::GetAreaByGameObjectID  gog 0x0064bb40  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c120 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 12 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 12 (callers: GetServerArea 0x7aae00, OnApplySummonCreature 0x599500, OnApplySummonParty 0x599a10)
- best decoy (neighbour/other candidate) scores: 0x0051c280:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> 0x0051c100  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bb60 CServerExoApp::GetTriggerByGameObjectID -> 0x0051c140  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bae30 CServerExoAppInternal::GetAreaByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (12 of 23): the card's A-CALLEE credit may come from these, not from callees
  0x00457800 CSWCArea::GetServerArea -> Steam 0x007aae00  in candidate callers: yes
  0x0073bdf0 CSWSEffectListHandler::OnApplySummonCreature -> Steam 0x00599500  in candidate callers: yes
  0x0073c300 CSWSEffectListHandler::OnApplySummonParty -> Steam 0x00599a10  in candidate callers: yes
  0x007489a0 CSWSEffectListHandler::OnApplyDisappearAppear -> Steam 0x005a60b0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c120: 16-byte aligned: YES
  0051c120  55             PUSH EBP
  0051c121  8B EC          MOV EBP,ESP
  0051c123  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c120
string literals (0):
direct callees (1):
  0x00537BE0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c120 FUN_0051c120 size=28 ret=4 callers=23 callees=1
THUNK -> 0x00537BE0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c120(undefined4 param_1)

{
  FUN_00537be0(param_1);
  return;
}
```

### decoy: 0x0051c280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c280: 16-byte aligned: YES
  0051c280  55             PUSH EBP
  0051c281  8B EC          MOV EBP,ESP
  0051c283  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c280
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10044 [dec 65604]  x1  via ECX
```

## Card B9-1-28

GOG function: CServerExoApp::LoadCharacterFinish  gog 0x0064ba80  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0051c060 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c080:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064ba30 CServerExoApp::LoadCharacterStart -> 0x0051c010  gate_class=AUTO
- GOG-order neighbour after:  0x0064baa0 CServerExoApp::GetObjectArray -> 0x0051c080  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b4240 CServerExoAppInternal::LoadCharacterFinish -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0051c060: 16-byte aligned: YES
  0051c060  55             PUSH EBP
  0051c061  8B EC          MOV EBP,ESP
  0051c063  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c060
string literals (0):
direct callees (1):
  0x00530EC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c060 FUN_0051c060 size=32 ret=8 callers=1 callees=1
THUNK -> 0x00530EC0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c060(undefined4 param_1,undefined4 param_2)

{
  FUN_00530ec0(param_1,param_2);
  return;
}
```

### decoy: 0x0051c080 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c080: 16-byte aligned: YES
  0051c080  55             PUSH EBP
  0051c081  8B EC          MOV EBP,ESP
  0051c083  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c080
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1005C [dec 65628]  x1  via ECX
```

## Card B9-1-29

GOG function: CServerExoApp::GetNetLayer  gog 0x0064bce0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c2c0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c2a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bcc0 CServerExoApp::GetPlayerList -> 0x0051c2a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bd00 CServerExoApp::GetSWSMessage -> 0x0051c2e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c2c0: 16-byte aligned: YES
  0051c2c0  55             PUSH EBP
  0051c2c1  8B EC          MOV EBP,ESP
  0051c2c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c2c0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10040 [dec 65600]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c2c0 FUN_0051c2c0 size=31 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_0051c2c0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10040);
}
```

### decoy: 0x0051c2a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c2a0: 16-byte aligned: YES
  0051c2a0  55             PUSH EBP
  0051c2a1  8B EC          MOV EBP,ESP
  0051c2a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c2a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10064 [dec 65636]  x1  via ECX
```

## Card B9-1-30

GOG function: CServerExoAppInternal::UpdateShutdownTimer  gog 0x007bcab0  callconv=(blank)  param_size_bytes=8  gog_size=541  gog_retn=8
Candidate Steam address: 0x005394c0 (size 220, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005393b0:STR0/CALLEE0/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bca50 CServerExoAppInternal::AddCharListRequest -> 0x00539800  gate_class=REVIEW
- GOG-order neighbour after:  0x007bcce0 Global::UpdateLoadScreen  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007e3eb0 CSWSMessage::SendServerToPlayerShutDownServer -> Steam ?  in candidate callees: n/a
  0x00924e50 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0x30,0xf4240,0x1c9c380,0x3938700} | Steam {0x2,0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005394c0: 16-byte aligned: YES
  005394c0  55             PUSH EBP
  005394c1  8B EC          MOV EBP,ESP
  005394c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005394c0
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0064DDE0  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0x10070 [dec 65648]  x4  via EAX,ECX,EDX
  +0x10074 [dec 65652]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005394c0 FUN_005394c0 size=220 ret=8 callers=1 callees=4

void __thiscall FUN_005394c0(undefined4 param_1,int param_2,byte param_3)

{
  int iVar1;
  int local_8;
  
  if (param_3 != 0) {
    if ((param_3 & 1) == 0) {
      if ((param_3 & 2) != 0) {
        local_8 = FUN_007a1720(param_1);
        while (local_8 != 0) {
          iVar1 = FUN_00561430(local_8);
          if (iVar1 == param_2) {
            FUN_0064dde0(local_8);
            return;
          }
          FUN_0058c370(&local_8);
        }
      }
    }
    else {
      local_8 = FUN_007a1720(param_1);
      while (local_8 != 0) {
        iVar1 = FUN_00561430(local_8);
        if (iVar1 == param_2) {
          FUN_0064dde0(local_8);
          return;
        }
        FUN_0058c370(&local_8);
      }
    }
  }
  return;
}
```

### decoy: 0x005393b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005393b0: 16-byte aligned: YES
  005393b0  55             PUSH EBP
  005393b1  8B EC          MOV EBP,ESP
  005393b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005393b0
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
  0x0054AA30  (unnamed)
struct offsets touched (top 2):
  +0x10070 [dec 65648]  x4  via EAX,ECX,EDX
  +0x10074 [dec 65652]  x4  via EAX,ECX,EDX
```

## Card B9-1-31

GOG function: CServerExoApp::GetCreatureByGameObjectID  gog 0x0064bb20  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c100 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 218 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 218 (callers: ShowPartySelection 0x7cb920, MenuActionSetPlaceableMine 0x842bd0, OnPanelAdded 0x894320)
- best decoy (neighbour/other candidate) scores: 0x0051c280:STR0/CALLEE16/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bb00 CServerExoApp::GetItemByGameObjectID -> 0x0051c0e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bb40 CServerExoApp::GetAreaByGameObjectID -> 0x0051c120  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007badc0 CServerExoAppInternal::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (218 of 396): the card's A-CALLEE credit may come from these, not from callees
  0x004dc020 CGuiInGame::ShowPartySelection -> Steam 0x007cb920  in candidate callers: yes
  0x00536450 CSWCPlaceable::MenuActionSetPlaceableMine -> Steam 0x00842bd0  in candidate callers: yes
  0x00563290 CSWGuiInGameMap::OnPanelAdded -> Steam 0x00894320  in candidate callers: yes
  0x0059b250 CSWGuiInGameInventory::Draw -> Steam 0x008a7350  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c100: 16-byte aligned: YES
  0051c100  55             PUSH EBP
  0051c101  8B EC          MOV EBP,ESP
  0051c103  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c100
string literals (0):
direct callees (1):
  0x00537B70  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c100 FUN_0051c100 size=28 ret=4 callers=395 callees=1
THUNK -> 0x00537B70 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c100(undefined4 param_1)

{
  FUN_00537b70(param_1);
  return;
}
```

### decoy: 0x0051c280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c280: 16-byte aligned: YES
  0051c280  55             PUSH EBP
  0051c281  8B EC          MOV EBP,ESP
  0051c283  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c280
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10044 [dec 65604]  x1  via ECX
```

## Card B9-1-32

GOG function: CServerExoApp::GetTriggerByGameObjectID  gog 0x0064bb60  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c140 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: GetNumMinesSetByPlayerParty 0x52e1c0)
- best decoy (neighbour/other candidate) scores: 0x0051c120:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bb40 CServerExoApp::GetAreaByGameObjectID -> 0x0051c120  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bb80 CServerExoApp::GetPlaceableByGameObjectID -> 0x0051c160  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007baf10 CServerExoAppInternal::GetTriggerByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007232a0 CSWSArea::GetNumMinesSetByPlayerParty -> Steam 0x0052e1c0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c140: 16-byte aligned: YES
  0051c140  55             PUSH EBP
  0051c141  8B EC          MOV EBP,ESP
  0051c143  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c140
string literals (0):
direct callees (1):
  0x00537CC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c140 FUN_0051c140 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00537CC0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c140(undefined4 param_1)

{
  FUN_00537cc0(param_1);
  return;
}
```

### decoy: 0x0051c120 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c120: 16-byte aligned: YES
  0051c120  55             PUSH EBP
  0051c121  8B EC          MOV EBP,ESP
  0051c123  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c120
string literals (0):
direct callees (1):
  0x00537BE0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-33

GOG function: CServerExoApp::GetActiveTimer  gog 0x0064bd70  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c350 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c6b0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bd40 CServerExoApp::GetServerMode -> 0x0051c320  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bd90 CServerExoApp::GetWorldTimer -> 0x0051c370  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bc050 CServerExoAppInternal::GetActiveTimer -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x00747660 CSWSEffectListHandler::OnApplyTimestop -> Steam 0x005a4d70  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c350: 16-byte aligned: YES
  0051c350  55             PUSH EBP
  0051c351  8B EC          MOV EBP,ESP
  0051c353  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c350
string literals (0):
direct callees (1):
  0x00538E00  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c350 FUN_0051c350 size=28 ret=4 callers=12 callees=1
THUNK -> 0x00538E00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c350(undefined4 param_1)

{
  FUN_00538e00(param_1);
  return;
}
```

### decoy: 0x0051c6b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c6b0: 16-byte aligned: YES
  0051c6b0  55             PUSH EBP
  0051c6b1  8B EC          MOV EBP,ESP
  0051c6b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c6b0
string literals (0):
direct callees (1):
  0x00538C20  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-34

GOG function: CServerExoApp::GetStoreByGameObjectID  gog 0x0064bba0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c180 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c160:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bb80 CServerExoApp::GetPlaceableByGameObjectID -> 0x0051c160  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bbc0 CServerExoApp::GetDoorByGameObjectID -> 0x0051c1a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bb220 CServerExoAppInternal::GetStoreByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 10): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c180: 16-byte aligned: YES
  0051c180  55             PUSH EBP
  0051c181  8B EC          MOV EBP,ESP
  0051c183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c180
string literals (0):
direct callees (1):
  0x00537FD0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c180 FUN_0051c180 size=28 ret=4 callers=10 callees=1
THUNK -> 0x00537FD0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c180(undefined4 param_1)

{
  FUN_00537fd0(param_1);
  return;
}
```

### decoy: 0x0051c160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c160: 16-byte aligned: YES
  0051c160  55             PUSH EBP
  0051c161  8B EC          MOV EBP,ESP
  0051c163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c160
string literals (0):
direct callees (1):
  0x00537D30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-35

GOG function: CServerExoApp::GetAreaOfEffectByGameObjectID  gog 0x0064bbe0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c1c0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c1a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bbc0 CServerExoApp::GetDoorByGameObjectID -> 0x0051c1a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bc00 CServerExoApp::GetWaypointByGameObjectID -> 0x0051c1e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bb060 CServerExoAppInternal::GetAreaOfEffectByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c1c0: 16-byte aligned: YES
  0051c1c0  55             PUSH EBP
  0051c1c1  8B EC          MOV EBP,ESP
  0051c1c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c1c0
string literals (0):
direct callees (1):
  0x00537E10  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c1c0 FUN_0051c1c0 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00537E10 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c1c0(undefined4 param_1)

{
  FUN_00537e10(param_1);
  return;
}
```

### decoy: 0x0051c1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c1a0: 16-byte aligned: YES
  0051c1a0  55             PUSH EBP
  0051c1a1  8B EC          MOV EBP,ESP
  0051c1a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c1a0
string literals (0):
direct callees (1):
  0x00537DA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-36

GOG function: CServerExoApp::HandleMessage  gog 0x0064be40  callconv=__thiscall  param_size_bytes=16  gog_size=40  gog_retn=16
Candidate Steam address: 0x0051c420 (size 40, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c400:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: autolabel:DoHillClimbing
- GOG-order neighbour before: 0x0064be20 CServerExoApp::GetDebugMode -> 0x0051c400  gate_class=REVIEW
- GOG-order neighbour after:  0x0064be70 CServerExoApp::Initialize -> 0x0051c450  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b4860 CServerExoAppInternal::HandleMessage -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0051c420: 16-byte aligned: YES
  0051c420  55             PUSH EBP
  0051c421  8B EC          MOV EBP,ESP
  0051c423  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c420
string literals (0):
direct callees (1):
  0x005314E0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c420 Concurrency::details::SchedulerProxy::DoHillClimbing size=40 ret=16 callers=0 callees=1
THUNK -> 0x005314E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: unsigned int __thiscall Concurrency::details::SchedulerProxy::DoHillClimbing(unsigned
   int,unsigned int,unsigned int,unsigned int)
   
   Libraries: Visual Studio 2010 Debug, Visual Studio 2012 Debug, Visual Studio 2015 Debug, Visual
   Studio 2015 Release */

uint __thiscall
Concurrency::details::SchedulerProxy::DoHillClimbing
          (SchedulerProxy *this,uint param_1,uint param_2,uint param_3,uint param_4)

{
  uint uVar1;
  
  uVar1 = FUN_005314e0(param_1,param_2,param_3,param_4);
  return uVar1;
}
```

### decoy: 0x0051c400 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c400: 16-byte aligned: YES
  0051c400  55             PUSH EBP
  0051c401  8B EC          MOV EBP,ESP
  0051c403  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c400
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1006C [dec 65644]  x1  via ECX
```

## Card B9-1-37

GOG function: CServerExoApp::UpdateSubareasOnMoveTo  gog 0x0064c500  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0051caa0 (size 53, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cae0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c4c0 CServerExoApp::UpDatePlayerPosition -> 0x0051caa0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c530 CServerExoApp::ClearAllActions -> 0x0051cb10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bd080 CServerExoAppInternal::UpdateSubareasOnMoveTo -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0x8,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0051caa0: 16-byte aligned: YES
  0051caa0  55             PUSH EBP
  0051caa1  8B EC          MOV EBP,ESP
  0051caa3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051caa0
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x00538C50  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051caa0 FUN_0051caa0 size=53 ret=12 callers=1 callees=2

void FUN_0051caa0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0(param_1);
  FUN_00538c50(uVar1,param_2,param_3);
  return;
}
```

### decoy: 0x0051cae0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cae0: 16-byte aligned: YES
  0051cae0  55             PUSH EBP
  0051cae1  8B EC          MOV EBP,ESP
  0051cae3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cae0
string literals (0):
direct callees (1):
  0x00539E30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-1-38

GOG function: CServerExoApp::SetAutoSavePending  gog 0x0064b970  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051bf50 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandDoSinglePlayerAutoSave 0x6710a0)
- best decoy (neighbour/other candidate) scores: 0x0051bf20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b940 CServerExoApp::SetLoadingModule -> 0x0051bf20  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b9a0 CServerExoApp::GetAutoSavePending -> 0x0051bf80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00786680 Global::ExecuteCommandDoSinglePlayerAutoSave -> Steam 0x006710a0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051bf50: 16-byte aligned: YES
  0051bf50  55             PUSH EBP
  0051bf51  8B EC          MOV EBP,ESP
  0051bf53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bf50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B8 [dec 65720]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bf50 FUN_0051bf50 size=36 ret=4 callers=1 callees=0

void __thiscall FUN_0051bf50(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x100b8) = param_2;
  return;
}
```

### decoy: 0x0051bf20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bf20: 16-byte aligned: YES
  0051bf20  55             PUSH EBP
  0051bf21  8B EC          MOV EBP,ESP
  0051bf23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bf20
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B4 [dec 65716]  x1  via EDX
```

## Card B9-1-39

GOG function: CServerExoApp::SetMoveToModulePending  gog 0x0064b870  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051be50 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0051bea0:STR0/CALLEE4/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b840 CServerExoApp::LoadGame -> 0x0051be20  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b8a0 CServerExoApp::GetMoveToModulePending -> 0x0051be80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (4 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x00717360 CSWSArea::EventHandler -> Steam 0x005221b0  in candidate callers: yes
  0x0075e400 CSWSTrigger::EventHandler -> Steam 0x0061f170  in candidate callers: yes
  0x00762ff0 CSWSDoor::EventHandler -> Steam 0x00617b40  in candidate callers: yes
  0x007a9260 Global::ExecuteCommandStartNewModule -> Steam 0x00693c10  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051be50: 16-byte aligned: YES
  0051be50  55             PUSH EBP
  0051be51  8B EC          MOV EBP,ESP
  0051be53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051be50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10080 [dec 65664]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051be50 FUN_0051be50 size=36 ret=4 callers=6 callees=0

void __thiscall FUN_0051be50(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x10080) = param_2;
  return;
}
```

### decoy: 0x0051bea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bea0: 16-byte aligned: YES
  0051bea0  55             PUSH EBP
  0051bea1  8B EC          MOV EBP,ESP
  0051bea3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bea0
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-1-40

GOG function: CServerExoApp::SaveGame  gog 0x0064b810  callconv=__thiscall  param_size_bytes=16  gog_size=40  gog_retn=16
Candidate Steam address: 0x0051bdf0 (size 40, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: SaveGame 0x52f610)
- best decoy (neighbour/other candidate) scores: 0x0051be20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b7d0 CServerExoApp::GetModuleName -> 0x0051bdb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b840 CServerExoApp::LoadGame -> 0x0051be20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b2990 CServerExoAppInternal::SaveGame -> Steam 0x0052f610  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0051bdf0: 16-byte aligned: YES
  0051bdf0  55             PUSH EBP
  0051bdf1  8B EC          MOV EBP,ESP
  0051bdf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bdf0
string literals (0):
direct callees (1):
  0x0052F610  CServerExoAppInternal::SaveGame
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bdf0 Concurrency::details::SchedulerProxy::DoHillClimbing size=40 ret=16 callers=2 callees=1
THUNK -> 0x0052F610 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: unsigned int __thiscall Concurrency::details::SchedulerProxy::DoHillClimbing(unsigned
   int,unsigned int,unsigned int,unsigned int)
   
   Libraries: Visual Studio 2010 Debug, Visual Studio 2012 Debug, Visual Studio 2015 Debug, Visual
   Studio 2015 Release */

uint __thiscall
Concurrency::details::SchedulerProxy::DoHillClimbing
          (SchedulerProxy *this,uint param_1,uint param_2,uint param_3,uint param_4)

{
  uint uVar1;
  
  uVar1 = FUN_0052f610(param_1,param_2,param_3,param_4);
  return uVar1;
}
```

### decoy: 0x0051be20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051be20: 16-byte aligned: YES
  0051be20  55             PUSH EBP
  0051be21  8B EC          MOV EBP,ESP
  0051be23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051be20
string literals (0):
direct callees (1):
  0x0052FB80  CServerExoAppInternal::LoadGame (SAVES:%06d - %s; partytable, globals 0x656710, LoadModule)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-41

GOG function: CServerExoApp::ClientToServerObjectId  gog 0x0064c2d0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c850 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x0051c8b0 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c830:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c2b0 CServerExoApp::GetGlobalVariableTable -> 0x0051c890  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bcf80 CServerExoAppInternal::ClientToServerObjectId -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (17 of 85): the card's A-CALLEE credit may come from these, not from callees
  0x0040cec0 CClientExoApp::CreatureAcquireItem -> Steam 0x0073f9e0  in candidate callers: no
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: no
  0x00457800 CSWCArea::GetServerArea -> Steam 0x007aae00  in candidate callers: no
  0x00477660 CSWCObject::ClearAllActions -> Steam 0x007f24c0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c850: 16-byte aligned: YES
  0051c850  55             PUSH EBP
  0051c851  8B EC          MOV EBP,ESP
  0051c853  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c850
string literals (0):
direct callees (1):
  0x00539800  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c850 FUN_0051c850 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00539800 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c850(undefined4 param_1)

{
  FUN_00539800(param_1);
  return;
}
```

### decoy: 0x0051c830 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c830: 16-byte aligned: YES
  0051c830  55             PUSH EBP
  0051c831  8B EC          MOV EBP,ESP
  0051c833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c830
string literals (0):
direct callees (1):
  0x005397C0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-42

GOG function: CServerExoApp::SetExportAllCharactersPending  gog 0x0064b8f0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051bed0 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandExportAllCharacters 0x679190)
- best decoy (neighbour/other candidate) scores: 0x0051bf20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b8c0 CServerExoApp::SetMoveToModuleString -> 0x0051bea0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b920 CServerExoApp::GetLoadingModule -> 0x0051bf00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0078e770 Global::ExecuteCommandExportAllCharacters -> Steam 0x00679190  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051bed0: 16-byte aligned: YES
  0051bed0  55             PUSH EBP
  0051bed1  8B EC          MOV EBP,ESP
  0051bed3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bed0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100BC [dec 65724]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bed0 FUN_0051bed0 size=36 ret=4 callers=1 callees=0

void __thiscall FUN_0051bed0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x100bc) = param_2;
  return;
}
```

### decoy: 0x0051bf20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bf20: 16-byte aligned: YES
  0051bf20  55             PUSH EBP
  0051bf21  8B EC          MOV EBP,ESP
  0051bf23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bf20
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B4 [dec 65716]  x1  via EDX
```

## Card B9-1-43

GOG function: CServerExoApp::GetDoorByGameObjectID  gog 0x0064bbc0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c1a0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 7 (callers: AddAttackActions 0x568540, HandleTransparentDoors 0x52b9c0, GetDoorCrossed 0x52c670)
- best decoy (neighbour/other candidate) scores: 0x0051c160:STR0/CALLEE3/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bba0 CServerExoApp::GetStoreByGameObjectID -> 0x0051c180  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bbe0 CServerExoApp::GetAreaOfEffectByGameObjectID -> 0x0051c1c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007baff0 CServerExoAppInternal::GetDoorByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 9): the card's A-CALLEE credit may come from these, not from callees
  0x00681170 CSWSCreature::AddAttackActions -> Steam 0x00568540  in candidate callers: yes
  0x00720b70 CSWSArea::HandleTransparentDoors -> Steam 0x0052b9c0  in candidate callers: yes
  0x00721750 CSWSArea::GetDoorCrossed -> Steam 0x0052c670  in candidate callers: yes
  0x00721af0 CSWSArea::BudgeCreatures -> Steam 0x0052ca10  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c1a0: 16-byte aligned: YES
  0051c1a0  55             PUSH EBP
  0051c1a1  8B EC          MOV EBP,ESP
  0051c1a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c1a0
string literals (0):
direct callees (1):
  0x00537DA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c1a0 FUN_0051c1a0 size=28 ret=4 callers=9 callees=1
THUNK -> 0x00537DA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c1a0(undefined4 param_1)

{
  FUN_00537da0(param_1);
  return;
}
```

### decoy: 0x0051c160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c160: 16-byte aligned: YES
  0051c160  55             PUSH EBP
  0051c161  8B EC          MOV EBP,ESP
  0051c163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c160
string literals (0):
direct callees (1):
  0x00537D30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-44

GOG function: CServerExoApp::GetItemByGameObjectID  gog 0x0064bb00  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c0e0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 36 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 36 (callers: ShowUpgradeScreen 0x7cd7e0, OnControlEntered 0x8a8100, SetItem 0x8b1250)
- best decoy (neighbour/other candidate) scores: 0x0051c100:STR0/CALLEE7/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bac0 CServerExoApp::GetGameObject -> 0x0051c0a0  gate_class=AUTO
- GOG-order neighbour after:  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> 0x0051c100  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bad50 CServerExoAppInternal::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (38 of 109): the card's A-CALLEE credit may come from these, not from callees
  0x004ddee0 CGuiInGame::ShowUpgradeScreen -> Steam 0x007cd7e0  in candidate callers: yes
  0x0059c000 CSWGuiInGameInventory::OnControlEntered -> Steam 0x008a8100  in candidate callers: yes
  0x005a4df0 CSWGuiInGameItemEntry::SetItem -> Steam 0x008b1250  in candidate callers: yes
  0x005bcf40 CSWGuiUpgrade::OnControlEntered -> Steam 0x008ccb80  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c0e0: 16-byte aligned: YES
  0051c0e0  55             PUSH EBP
  0051c0e1  8B EC          MOV EBP,ESP
  0051c0e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c0e0
string literals (0):
direct callees (1):
  0x00537B00  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c0e0 FUN_0051c0e0 size=28 ret=4 callers=109 callees=1
THUNK -> 0x00537B00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c0e0(undefined4 param_1)

{
  FUN_00537b00(param_1);
  return;
}
```

### decoy: 0x0051c100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c100: 16-byte aligned: YES
  0051c100  55             PUSH EBP
  0051c101  8B EC          MOV EBP,ESP
  0051c103  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c100
string literals (0):
direct callees (1):
  0x00537B70  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-45

GOG function: CServerExoApp::SetMoveToModuleString  gog 0x0064b8c0  callconv=(blank)  param_size_bytes=4  gog_size=42  gog_retn=4
Candidate Steam address: 0x0051bea0 (size 42, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0051be50:STR0/CALLEE4/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b8a0 CServerExoApp::GetMoveToModulePending -> 0x0051be80  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b8f0 CServerExoApp::SetExportAllCharactersPending -> 0x0051bed0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
GOG callers mapped through trusted rows (4 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x00717360 CSWSArea::EventHandler -> Steam 0x005221b0  in candidate callers: yes
  0x0075e400 CSWSTrigger::EventHandler -> Steam 0x0061f170  in candidate callers: yes
  0x00762ff0 CSWSDoor::EventHandler -> Steam 0x00617b40  in candidate callers: yes
  0x007a9260 Global::ExecuteCommandStartNewModule -> Steam 0x00693c10  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x10084*} | Steam {0x4*,0x8*,0x10084*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051bea0: 16-byte aligned: YES
  0051bea0  55             PUSH EBP
  0051bea1  8B EC          MOV EBP,ESP
  0051bea3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bea0
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bea0 FUN_0051bea0 size=42 ret=4 callers=6 callees=1
THUNK -> 0x007337C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051bea0(undefined4 param_1)

{
  FUN_007337c0(param_1);
  return;
}
```

### decoy: 0x0051be50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051be50: 16-byte aligned: YES
  0051be50  55             PUSH EBP
  0051be51  8B EC          MOV EBP,ESP
  0051be53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051be50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10080 [dec 65664]  x1  via EDX
```

## Card B9-1-46

GOG function: CServerExoApp::GetPauseTimer  gog 0x0064bdd0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c3b0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c390:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bdb0 CServerExoApp::GetTimestopTimer -> 0x0051c390  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bdf0 CServerExoApp::SetDebugMode -> 0x0051c3d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c3b0: 16-byte aligned: YES
  0051c3b0  55             PUSH EBP
  0051c3b1  8B EC          MOV EBP,ESP
  0051c3b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c3b0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10050 [dec 65616]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c3b0 FUN_0051c3b0 size=31 ret=0 callers=4 callees=0

undefined4 __fastcall FUN_0051c3b0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10050);
}
```

### decoy: 0x0051c390 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c390: 16-byte aligned: YES
  0051c390  55             PUSH EBP
  0051c391  8B EC          MOV EBP,ESP
  0051c393  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c390
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1004C [dec 65612]  x1  via ECX
```

## Card B9-1-47

GOG function: CServerExoApp::GetServerInfo  gog 0x0064b760  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051bd40 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=GetServerInfo/JoiningRestrictions | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 4 (callers: CanUseItem 0x5d2f80, SetExperience 0x6b6bd0, GetPotentialLevel 0x6b7310)
- best decoy (neighbour/other candidate) scores: 0x0051bce0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b700 CServerExoApp::Destructor -> 0x0051bce0  gate_class=AUTO
- GOG-order neighbour after:  0x0064b780 CServerExoApp::GetModuleDescription -> 0x0051bd60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (4 of 9): the card's A-CALLEE credit may come from these, not from callees
  0x006ddba0 CSWSCreature::CanUseItem -> Steam 0x005d2f80  in candidate callers: yes
  0x006f3050 CSWSCreatureStats::SetExperience -> Steam 0x006b6bd0  in candidate callers: yes
  0x006f3790 CSWSCreatureStats::GetPotentialLevel -> Steam 0x006b7310  in candidate callers: yes
  0x006f5c10 CSWSCreatureStats::CanLevelUp -> Steam 0x006b9790  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051bd40: 16-byte aligned: YES
  0051bd40  55             PUSH EBP
  0051bd41  8B EC          MOV EBP,ESP
  0051bd43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bd40
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10004 [dec 65540]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bd40 FUN_0051bd40 size=31 ret=0 callers=9 callees=0

undefined4 __fastcall FUN_0051bd40(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10004);
}
```

### decoy: 0x0051bce0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bce0: 16-byte aligned: YES
  0051bce0  55             PUSH EBP
  0051bce1  8B EC          MOV EBP,ESP
  0051bce3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bce0
string literals (0):
direct callees (2):
  0x0052F000  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-48

GOG function: CServerExoApp::GetTimestopTimer  gog 0x0064bdb0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c390 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c370:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bd90 CServerExoApp::GetWorldTimer -> 0x0051c370  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bdd0 CServerExoApp::GetPauseTimer -> 0x0051c3b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c390: 16-byte aligned: YES
  0051c390  55             PUSH EBP
  0051c391  8B EC          MOV EBP,ESP
  0051c393  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c390
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1004C [dec 65612]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c390 FUN_0051c390 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0051c390(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x1004c);
}
```

### decoy: 0x0051c370 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c370: 16-byte aligned: YES
  0051c370  55             PUSH EBP
  0051c371  8B EC          MOV EBP,ESP
  0051c373  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c370
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10048 [dec 65608]  x1  via ECX
```

## Card B9-1-49

GOG function: CServerExoApp::GetSysAdminList  gog 0x0064bd20  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c300 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c2e0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bd00 CServerExoApp::GetSWSMessage -> 0x0051c2e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bd40 CServerExoApp::GetServerMode -> 0x0051c320  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c300: 16-byte aligned: YES
  0051c300  55             PUSH EBP
  0051c301  8B EC          MOV EBP,ESP
  0051c303  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c300
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10068 [dec 65640]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c300 FUN_0051c300 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0051c300(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10068);
}
```

### decoy: 0x0051c2e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c2e0: 16-byte aligned: YES
  0051c2e0  55             PUSH EBP
  0051c2e1  8B EC          MOV EBP,ESP
  0051c2e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c2e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10010 [dec 65552]  x1  via ECX
```

## Card B9-1-50

GOG function: CServerExoApp::GetWaypointByGameObjectID  gog 0x0064bc00  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c1e0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c2a0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bbe0 CServerExoApp::GetAreaOfEffectByGameObjectID -> 0x0051c1c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bc20 CServerExoApp::GetEncounterByGameObjectID -> 0x0051c200  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bb0d0 CServerExoAppInternal::GetWaypointByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007a7ae0 Global::ExecuteCommandSetMapPinEnabled -> Steam 0x00692490  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c1e0: 16-byte aligned: YES
  0051c1e0  55             PUSH EBP
  0051c1e1  8B EC          MOV EBP,ESP
  0051c1e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c1e0
string literals (0):
direct callees (1):
  0x00537E80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c1e0 FUN_0051c1e0 size=28 ret=4 callers=2 callees=1
THUNK -> 0x00537E80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c1e0(undefined4 param_1)

{
  FUN_00537e80(param_1);
  return;
}
```

### decoy: 0x0051c2a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c2a0: 16-byte aligned: YES
  0051c2a0  55             PUSH EBP
  0051c2a1  8B EC          MOV EBP,ESP
  0051c2a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c2a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10064 [dec 65636]  x1  via ECX
```

## Card B9-1-51

GOG function: CServerExoApp::GetModuleLanguage  gog 0x0064bc80  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c260 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c240:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bc60 CServerExoApp::GetPlayerLanguage -> 0x0051c240  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bca0 CServerExoApp::GetServerAIMaster -> 0x0051c280  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00410c20 Global::return_zero -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c260: 16-byte aligned: YES
  0051c260  55             PUSH EBP
  0051c261  8B EC          MOV EBP,ESP
  0051c263  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c260
string literals (0):
direct callees (1):
  0x004558A0  return 0 (proxy vtable slot 0x4)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c260 FUN_0051c260 size=22 ret=0 callers=3 callees=1
THUNK -> 0x004558A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c260(undefined4 param_1)

{
  FUN_004558a0(param_1);
  return;
}
```

### decoy: 0x0051c240 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c240: 16-byte aligned: YES
  0051c240  55             PUSH EBP
  0051c241  8B EC          MOV EBP,ESP
  0051c243  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c240
string literals (0):
direct callees (1):
  0x00538040  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-52

GOG function: CServerExoApp::Shutdown  gog 0x0064bf00  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0051c4e0 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c520:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bee0 CServerExoApp::ContinueMessageProcessing -> 0x0051c4c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bf20 CServerExoApp::UnloadModule -> 0x0051c500  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007baac0 CServerExoAppInternal::Shutdown -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0051c4e0: 16-byte aligned: YES
  0051c4e0  55             PUSH EBP
  0051c4e1  8B EC          MOV EBP,ESP
  0051c4e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c4e0
string literals (0):
direct callees (1):
  0x00537870  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c4e0 FUN_0051c4e0 size=32 ret=8 callers=1 callees=1
THUNK -> 0x00537870 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c4e0(undefined4 param_1,undefined4 param_2)

{
  FUN_00537870(param_1,param_2);
  return;
}
```

### decoy: 0x0051c520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c520: 16-byte aligned: YES
  0051c520  55             PUSH EBP
  0051c521  8B EC          MOV EBP,ESP
  0051c523  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c520
string literals (0):
direct callees (1):
  0x00531C50  CServerExoAppInternal::LoadModule
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-1-53

GOG function: CServerExoApp::GetFPS  gog 0x0064b6e0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x0051bcc0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051bc20:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b640 CServerExoApp::Constructor -> 0x0051bcc0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b700 CServerExoApp::Destructor -> 0x0051bce0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051bcc0: 16-byte aligned: YES
  0051bcc0  55             PUSH EBP
  0051bcc1  8B EC          MOV EBP,ESP
  0051bcc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bcc0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051bcc0 FUN_0051bcc0 size=17 ret=0 callers=0 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_0051bcc0(void)

{
  return (float10)_DAT_0098c270;
}
```

### decoy: 0x0051bc20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bc20: 16-byte aligned: YES
  0051bc20  55             PUSH EBP
  0051bc21  8B EC          MOV EBP,ESP
  0051bc23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0051bc20
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0052EBE0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-1-54

GOG function: CServerExoApp::LoadGame  gog 0x0064b840  callconv=__thiscall  param_size_bytes=16  gog_size=40  gog_retn=16
Candidate Steam address: 0x0051be20 (size 40, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: LoadGame 0x52fb80)
- best decoy (neighbour/other candidate) scores: 0x0051bdf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b810 CServerExoApp::SaveGame -> 0x0051bdf0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b870 CServerExoApp::SetMoveToModulePending -> 0x0051be50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b2f00 CServerExoAppInternal::LoadGame -> Steam 0x0052fb80  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0051be20: 16-byte aligned: YES
  0051be20  55             PUSH EBP
  0051be21  8B EC          MOV EBP,ESP
  0051be23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051be20
string literals (0):
direct callees (1):
  0x0052FB80  CServerExoAppInternal::LoadGame (SAVES:%06d - %s; partytable, globals 0x656710, LoadModule)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051be20 Concurrency::details::SchedulerProxy::DoHillClimbing size=40 ret=16 callers=1 callees=1
THUNK -> 0x0052FB80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: unsigned int __thiscall Concurrency::details::SchedulerProxy::DoHillClimbing(unsigned
   int,unsigned int,unsigned int,unsigned int)
   
   Libraries: Visual Studio 2010 Debug, Visual Studio 2012 Debug, Visual Studio 2015 Debug, Visual
   Studio 2015 Release */

uint __thiscall
Concurrency::details::SchedulerProxy::DoHillClimbing
          (SchedulerProxy *this,uint param_1,uint param_2,uint param_3,uint param_4)

{
  uint uVar1;
  
  uVar1 = FUN_0052fb80(param_1,param_2,param_3,param_4);
  return uVar1;
}
```

### decoy: 0x0051bdf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bdf0: 16-byte aligned: YES
  0051bdf0  55             PUSH EBP
  0051bdf1  8B EC          MOV EBP,ESP
  0051bdf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bdf0
string literals (0):
direct callees (1):
  0x0052F610  CServerExoAppInternal::SaveGame
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-55

GOG function: CServerExoApp::LoadModule  gog 0x0064bf40  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0051c620 (size 33, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c600:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bf20 CServerExoApp::UnloadModule -> 0x0051c500  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bf60 CServerExoApp::RunModule -> 0x0051c540  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b4fd0 CServerExoAppInternal::LoadModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0051c620: 16-byte aligned: YES
  0051c620  55             PUSH EBP
  0051c621  8B EC          MOV EBP,ESP
  0051c623  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c620
string literals (0):
direct callees (1):
  0x0052F3D0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c620 FUN_0051c620 size=33 ret=8 callers=7 callees=1
THUNK -> 0x0052F3D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c620(undefined4 param_1,undefined1 param_2)

{
  FUN_0052f3d0(param_1,param_2);
  return;
}
```

### decoy: 0x0051c600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c600: 16-byte aligned: YES
  0051c600  55             PUSH EBP
  0051c601  8B EC          MOV EBP,ESP
  0051c603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c600
string literals (0):
direct callees (1):
  0x0052F330  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-56

GOG function: CServerExoApp::ClientToServerObjectId  gog 0x0064c2d0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c9f0 (size 70, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x0051c8b0 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051ca40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c2b0 CServerExoApp::GetGlobalVariableTable -> 0x0051c890  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bcf80 CServerExoAppInternal::ClientToServerObjectId -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (17 of 85): the card's A-CALLEE credit may come from these, not from callees
  0x0040cec0 CClientExoApp::CreatureAcquireItem -> Steam 0x0073f9e0  in candidate callers: no
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: no
  0x00457800 CSWCArea::GetServerArea -> Steam 0x007aae00  in candidate callers: no
  0x00477660 CSWCObject::ClearAllActions -> Steam 0x007f24c0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {0x380}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x10}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c9f0: 16-byte aligned: YES
  0051c9f0  55             PUSH EBP
  0051c9f1  8B EC          MOV EBP,ESP
  0051c9f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c9f0
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
struct offsets touched (top 1):
  +0x380 [dec 896]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c9f0 FUN_0051c9f0 size=70 ret=4 callers=1 callees=2

undefined4 FUN_0051c9f0(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = FUN_0051c8b0(param_1);
  iVar2 = FUN_0051c100(uVar1);
  if (iVar2 == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(undefined4 *)(iVar2 + 0x380);
  }
  return uVar1;
}
```

### decoy: 0x0051ca40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051ca40: 16-byte aligned: YES
  0051ca40  55             PUSH EBP
  0051ca41  8B EC          MOV EBP,ESP
  0051ca43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051ca40
string literals (0):
direct callees (3):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EF0  (unnamed)
  0x0054B130  CSWSArea::ComputeHeight
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

## Card B9-1-57

GOG function: CSWSModule::Destructor  gog 0x007294a0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x005570d0 (size 126, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00557150:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00729480 CSWSModule::non_virtual_thunk_to_AsSWSModule -> 0x00556cd0  gate_class=REVIEW
- GOG-order neighbour after:  0x007294d0 CSWSModule::Destructor_2 -> 0x00556d20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007294d0 CSWSModule::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0x10}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005570d0: 16-byte aligned: YES
  005570d0  55             PUSH EBP
  005570d1  8B EC          MOV EBP,ESP
  005570d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005570d0
string literals (0):
direct callees (3):
  0x00735360  CExoLocString::Destructor
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005570d0 FUN_005570d0 size=126 ret=4 callers=1 callees=3

undefined4 __thiscall FUN_005570d0(undefined4 param_1,uint param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_0094fea3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 1;
  FUN_00735360(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = local_8 & 0xffffff00;
  FUN_00735360();
  local_8 = 0xffffffff;
  FUN_00733780();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00557150 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00557150: 16-byte aligned: YES
  00557150  55             PUSH EBP
  00557151  8B EC          MOV EBP,ESP
  00557153  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00557150
string literals (0):
direct callees (4):
  0x00919723  operator new
  0x00557FB0  (unnamed)
  0x00557FF0  (unnamed)
  0x0055E610  (unnamed)
struct offsets touched (top 5):
  +0xA0 [dec 160]  x4  via EAX,ECX,EDX
  +0xAC [dec 172]  x2  via EDX
  +0x80 [dec 128]  x1  via ECX
  +0xA8 [dec 168]  x1  via EAX
  +0xB0 [dec 176]  x1  via EDX
```

## Card B9-1-58

GOG function: CServerExoApp::GetServerAIMaster  gog 0x0064bca0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c280 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 71 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 71 (callers: Constructor 0x5ffd60, Destructor_2 0x6000e0, SetPossessor 0x600380)
- best decoy (neighbour/other candidate) scores: 0x0051c600:STR0/CALLEE14/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bc80 CServerExoApp::GetModuleLanguage -> 0x0051c260  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bcc0 CServerExoApp::GetPlayerList -> 0x0051c2a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (71 of 138): the card's A-CALLEE credit may come from these, not from callees
  0x006634d0 CSWSItem::Constructor -> Steam 0x005ffd60  in candidate callers: yes
  0x00663910 CSWSItem::Destructor_2 -> Steam 0x006000e0  in candidate callers: yes
  0x00663bb0 CSWSItem::SetPossessor -> Steam 0x00600380  in candidate callers: yes
  0x006648e0 CSWSItem::EventHandler -> Steam 0x006010b0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c280: 16-byte aligned: YES
  0051c280  55             PUSH EBP
  0051c281  8B EC          MOV EBP,ESP
  0051c283  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c280
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10044 [dec 65604]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c280 FUN_0051c280 size=31 ret=0 callers=138 callees=0

undefined4 __fastcall FUN_0051c280(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10044);
}
```

### decoy: 0x0051c600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c600: 16-byte aligned: YES
  0051c600  55             PUSH EBP
  0051c601  8B EC          MOV EBP,ESP
  0051c603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c600
string literals (0):
direct callees (1):
  0x0052F330  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-1-59

GOG function: CServerExoApp::Constructor  gog 0x0064b640  callconv=__thiscall  param_size_bytes=0  gog_size=147  gog_retn=0
Candidate Steam address: 0x0051bcc0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051bc20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b4d0 CAvoidCreature::IsPointInHex  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064b6e0 CServerExoApp::GetFPS -> 0x0051bcc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007b1f60 CServerExoAppInternal::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {}
imm sets (* = in both, 0 common): GOG {0x4,0x10,0x1f350} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051bcc0: 16-byte aligned: YES
  0051bcc0  55             PUSH EBP
  0051bcc1  8B EC          MOV EBP,ESP
  0051bcc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bcc0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051bcc0 FUN_0051bcc0 size=17 ret=0 callers=0 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_0051bcc0(void)

{
  return (float10)_DAT_0098c270;
}
```

### decoy: 0x0051bc20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bc20: 16-byte aligned: YES
  0051bc20  55             PUSH EBP
  0051bc21  8B EC          MOV EBP,ESP
  0051bc23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0051bc20
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0052EBE0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-1-60

GOG function: CServerExoApp::Initialize  gog 0x0064be70  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c450 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c420:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064be40 CServerExoApp::HandleMessage -> 0x0051c420  gate_class=REVIEW
- GOG-order neighbour after:  0x0064be90 CServerExoApp::MainLoop -> 0x0051c470  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b4920 CServerExoAppInternal::Initialize -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c450: 16-byte aligned: YES
  0051c450  55             PUSH EBP
  0051c451  8B EC          MOV EBP,ESP
  0051c453  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c450
string literals (0):
direct callees (1):
  0x005315A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c450 FUN_0051c450 size=22 ret=0 callers=1 callees=1
THUNK -> 0x005315A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c450(undefined4 param_1)

{
  FUN_005315a0(param_1);
  return;
}
```

### decoy: 0x0051c420 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c420: 16-byte aligned: YES
  0051c420  55             PUSH EBP
  0051c421  8B EC          MOV EBP,ESP
  0051c423  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c420
string literals (0):
direct callees (1):
  0x005314E0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```
