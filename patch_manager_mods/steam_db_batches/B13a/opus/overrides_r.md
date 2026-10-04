# Opus gate B13a part r: evidence (1 NOCAND row)

Method: whole-binary fp scan (GOG row vs every Steam fp row: size, retn, disp and imm multisets), then callee and
caller-size checks on both Windows builds, Steam asm from `steam_6a522e71/decomp`, Mac x86 fp for the symbol.

| # | GOG | GOG name | Steam | verdict |
|---|---|---|---|---|
| 1 | 0x443170 | CSWCCreature::SwitchMode | 0x796b50 | label_suspect (= CClientExoAppInternal::IndicateObject) |

**1. 0x443170 SwitchMode -> 0x796b50 label_suspect.**
- The GOG row is not in the CSWCCreature run at all. It sits between CSWTlkTable::GetObjectGender 0x443100 and
  CClientExoAppInternal::QueryExitProgram 0x4431c0, so the class delta never reached it (NOCAND).
- Body (GOG fp): 70 B, RET 8, 25 insns, disp {0x4, 0x40}, imm {0x8, 0x10, 0x7f000000}, one callee 0x4dee10
  CGuiInGame::SetMainInterfaceTarget (47 B, RET 8, disp 0x98).
- Whole-binary scan: exactly one Steam function has that size, retn, disp and imm: 0x796b50. Its callee 0x7ce710 is fp 1:1
  with GOG 0x4dee10 (47 B, RET 8, disp 0x98 x2), and 0x7ce710's only caller is 0x796b50, as GOG 0x4dee10's only caller is 0x443170.
- Caller sizes match: GOG IndicateObject (CClientExoApp) 32 / ShowObject 5780 / SetInputClass 774 = Steam 0x7401f0 32 /
  0x798d70 5780 / 0x7b3050 774. Steam 0x7401f0 is the trusted CClientExoApp::IndicateObject (steam-db-B11a).
- Asm: `00796b59 CMP [EBP+0x8],0`, `00796b68 MOV EDX,[ECX+0x4]` (object id) else `00796b70 MOV [EBP-0x10],0x7f000000`
  (invalid id), `00796b88 MOV ECX,[EAX+0x40]` (in-game GUI), `00796b8b CALL 0x7ce710`, `00796b93 RET 0x8`.
- Mac: CClientExoAppInternal::IndicateObject 0x1a325e, 53 B: disp 0x4/0x40, imm 0x7f000000, one callee
  CGuiInGame::SetMainInterfaceTarget, called by the CClientExoApp::IndicateObject wrapper. Same shape.
- The GOG label is wrong. The real CSWCCreature::SwitchMode body (Mac 59 B, disp 0x3c0 x3, imm 0x2) is GOG 0x422d70
  (FUN_00422d70, 84 B RET 4, disp 0x3c0 x3, imm 2/2/4/8, callers SetInputClass/SetEnabled/SetPlayer). It pairs fp 1:1 with
  Steam 0x776db0, whose callers have the same sizes (774/111/159). This was a decoy card in chunk 1; it is not shipped
  here because it has no gated GOG row.
- KDB: 0x796b50 is `SetHudTarget` (mods678 analyst label, medium). Renamed in `kdb_overrides.tsv` to the Mac name.
  0x798540 already holds `CClientExoAppInternal::IndicateObject` as `wrong`.

sample_errors: 0 (part r has no sample rows).
