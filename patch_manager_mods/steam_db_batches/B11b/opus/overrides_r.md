# B11b Opus gate, part r (3 REJECT/NOCAND rows)

Method: whole-binary exact-fp match on Steam (size, retn, n_insn, disp, imm); callees checked by size and by Steam
DB names; Steam asm/decomp read; Mac x86 names used for label checks.

| # | GOG | Steam | verdict | evidence |
|---|---|---|---|---|
| 1 | 0x44ccb0 CSWGuiMainInterface::Quicksave (NOCAND) | 0x7a06b0 | label_suspect | Only exact fp in Steam (148/0/43, imm 0x18). The body builds CExoString("QUICKSAVE") twice (Steam string 0x99d9a8 = GOG 0x987304 "QUICKSAVE") and calls SendSaveGameRequest 0x73f6f0 (DB). GOG caller is CClientExoAppInternal::HandleInputEvent 0x45ded0, and the Steam caller 0x7b12c0 has the identical fp (7087/8/1560). Mac: CClientExoAppInternal::DoQuickSave 0x1a7d86 (same "QUICKSAVE" string, same callees). So the class in the GOG label is wrong; the true name is CClientExoAppInternal::DoQuickSave. |
| 2 | 0x4cdda0 CGuiInGame::ShutDownGlobal (REJECT) | 0x7bdb10 | label_suspect | Unique exact fp. Decomp: `(flags & 2)` -> `eh_vector_destructor_iterator(p, 0x10, count, elem_dtor)` (`007bdb4a PUSH 0x10` / `007bdb50 CALL 0x00919bcc`), else 2x CExoString dtor 0x733780 + free. This is a compiler vector-deleting dtor of a 16 B {CExoString,CExoString}; its only caller is the CGuiInGame dtor 0x7bdbc0. Mac ShutDownGlobal (189 B, retn 0, globals) does not match. No GOG 0x4c-0x4f function with the Mac disp set was found, so the real ShutDownGlobal is still not located. This agrees with the existing Steam DB note (B0 parity) and GOG_LABEL_SUSPECTS.md. The G6 known_wrong flag is right about the label, not about the counterpart. |
| 3 | 0x4cde50 CGuiInGame::~CGuiInGame (REJECT) | 0x7bdbc0 | verified (correction) | The gate pick 0x740070 is CClientExoApp::GetMGOArray (33 B), so it is wrong. 0x7bdbc0 is the unique exact fp (702/0/188, identical disp/imm, 14 callees matching by size). Anchors: `007bdbf2 CALL 0x007c7030` (CGuiInGame::ShutDown, DB verified), `007bdc28 CALL 0x007bdb10` (row 2 body), and it frees this+0x110/0x114/0x118 (the message buffers that AppendToMsgBuffer 0x7be090 uses). It is a function start (prolog `55 8b ec 6a ff` at 0x7bdbc0, follows 0x7bdb10+0xad, aligned), not a fragment. Single caller 0x78ac40. |

sample_errors: 0 (not applicable to this part)
