# Opus gate B16 part r: evidence (REJECT/NOCAND rows)

Note: `B16/opus/part_r.md` is not on disk (the commit 08ecfae message says "parts a + r", but only part_a.md and
parts.json exist). I took the rows from `evidence.tsv` gate_class REJECT/NOCAND: 3 rows, which matches the lead's count.
Method as in part a: whole-binary fp search, callers, neighbour runs, Steam vftables from .rdata.

1. **NOCAND 0x4bcf10 CExoMoviePlayerInternal::GetNumberOfMoviesToPlay -> 0x40de90 verified.**
   - GOG fp 17b/ret0/9 insn, disp 0xc. The only exact Steam match is 0x40de90: `0040de9a MOV EAX,[EAX+0xc]`, `0040dea0 RET`.
   - Callers: GOG 0x4bcf10 has one caller, 0x921250 CExoMoviePlayer::GetNumberOfMoviesToPlay (33b). Steam 0x40de90 has
     one caller, 0x93b600 (33b): `0093b614 CALL 0x40de90`.
   - Wrapper run lines up: GOG ShowFadePanel 0x921210 / HideFadePanel 0x921230 / GetNumberOfMoviesToPlay 0x921250 /
     RestartMoviesAsync 0x921280 = Steam 0x93b5c0 / 0x93b5e0 / 0x93b600 / 0x93b630. They call 0x40dac0 / 0x40dc50 /
     0x40de90 / 0x40deb0, consistent with the batch's HideFadePanel 0x40dc50 and RestartMoviesAsync 0x40deb0.
   - In Steam the internal getter sits in the class run (after Resume 0x40de50). In GOG it was placed far away at 0x4bcf10.
2. **REJECT 0x5e56e0 CSWGuiInGameGameplay::OnPanelAdded -> 0x7dcea0 verified (correction of 0x8e0cc0).**
   - 0x8e0cc0 is OnAutoSave (122b, = GOG 0x5e5770).
   - There is no Steam body between HandleInputEvent 0x8e0a80 and OnAutoLevelUp 0x8e0c50. Raw bytes 0x8e0c20..0x8e0c4a
     are a switch jump table plus 0xcc padding.
   - Steam Gameplay vftable 0x9a8ee4, slot 18 (0x9a8f2c) = 0x7dcea0. Slot 18 is OnPanelAdded: in the Character vftable
     0x9a3e7c, slot 18 = 0x84e6c0 = Character::OnPanelAdded.
   - Body: `007dceaa CALL 0x40f3c0`. 0x40f3c0 is the unique Steam fp match of GOG CSWGuiPanel::OnPanelAdded 0x90d840
     (245b). GOG: 0x5e56e0 (19b) calls 0x90d840. It is the only 19b caller on each side.
   - **ICF fold:** 0x7dcea0 is also slot 18 of CSWGuiOptionsMouse's vftable 0x9a1efc (ref at 0x9a1f44). A hook here fires
     for both panels.
3. **REJECT 0x6529c0 CSWGlobalVariableTable::ReadTableWithCatalogue -> 0x655b00 verified.**
   - G6 failed because of a name string, not an address conflict. The trusted holder is name_map "CSWGlobalVariableTable::ReadTableWithCat (adds unknown names on load)", which is the same function.
   - fp: size 2481, ret 8, 621 insn, disp/imm identical. Steam has 13 str refs vs GOG 14; the difference is the GOG
     empty-string ref 0x9858f5.
   - `00655d37 PUSH 0x998720` "ReadTableWithCat(): BOOLEAN '%s' won't fit!", `00655beb PUSH 0x998684` "CatBoolean".
     ReadTable `006565d6 CALL 0x655b00`.

sample_errors: 0 (not applicable to part r)
