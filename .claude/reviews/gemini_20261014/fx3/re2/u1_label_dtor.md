# u1 label dtor (Haiku rea-recon, 2026-10-15; written by Opus from the agent reply, Write was refused to the agent)
Binary: Steam path swkotor2.exe (layout matches GameAddr.h, not K2Addr.h t.* table).
| address | what | evidence |
|---|---|---|
| 0x009878bc | CSWGuiLabel vftable; slot 0 bytes b0107500 = 0x007510b0 | read_bytes ev_05ed5143 |
| 0x007510b0 | scalar deleting dtor: thiscall, RET 4 (flags), bit0 -> _free 0x009196fd; no panel/focus/hover unlink seen | procedure_assembly ev_d74c590d (RET at 0x751137) |
| 0x009179E0 | NOT the label ctor (inside 0x916ef0); real ctor = 0x00419740 (GameAddr.h) | procedure_info ev_076cc38b |
VERDICT: PARTIAL (dtor confirmed; ctor premise from K2Addr.h refuted for this binary)
