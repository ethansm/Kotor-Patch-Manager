# FixtureStats

4 functions in the Steam DB for FixtureStats.

## 0x007680a0 FixtureStats::Dispatch

- Size 71 B, RET 0
- Callers (0 total): none
- Callees (4):
  - 0x007680a6 indirect
  - 0x007680ad CALL 0x007679a0
  - 0x007680d5 CALL 0x007679a0
  - 0x007680e1 indirect
- Imports: none
- Indirect slots (2):
  - 0x007680a6 slot +0x14 (direct [eax + 0x14])
  - 0x007680e1 slot +0x8 (007680de MOV EAX,dword ptr [ECX + 0x8])
- Ledger callsites in range (1):
  - 0x007680e1 vtbl slot 8 dispatch
- Strings: none
- Globals: none
- Struct fields touched (1):
  - 0x8 FixtureStats::m_pOwner (ptr) RW
- KDB purpose: Dispatches through a vtable slot.
- Named pseudo-C (first 4 of 4 lines):
```c
// NAMED Dispatch
void Dispatch(int this) {
  this->vtbl[8]();
}
```

## 0x00775930 FixtureStats::GetStat

- Size 25 B, RET 0
- Callers (0 total): none
- Callees (1):
  - 0x00775940 CALL 0x0085d260
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched (1):
  - 0x224 FixtureStats::m_nStat (int32) R
- KDB purpose: Reads the stat field and dispatches to the helper.
- Named pseudo-C (first 4 of 4 lines):
```c
// NAMED GetStat
int GetStat(int this) {
  return this->m_nStat;
}
```

## 0x00776db0 FixtureStats::Permov

- Size 84 B, RET 4
- Callers (3 total):
  - 0x007b3050 (site 0x007b31f4)
  - 0x00865250 (site 0x008652a4)
  - 0x008652c0 (site 0x00865303)
- Callees (1):
  - 0x00776dc0 import
- Imports: timeGetTime (WINMM.DLL)
- Indirect slots: none
- Ledger callsites in range: none
- Strings (1):
  - 0x009868a0 "permov06"
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)

## 0x00778200 FixtureStats::Recurse

- Size 15 B, RET 0
- Callers (1 total):
  - 0x00778200 (site 0x00778209)
- Callees (1):
  - 0x00778209 CALL 0x00778200
- Imports: none
- Indirect slots: none
- Ledger callsites in range: none
- Strings: none
- Globals: none
- Struct fields touched: none
- KDB purpose: (none)
- Named pseudo-C: (no named decompile)
