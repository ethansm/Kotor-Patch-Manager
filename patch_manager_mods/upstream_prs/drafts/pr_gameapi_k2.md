Title: `GameAPI: Match K2-only wrapper signatures`
Branch: `ethansm:gameapi/k2-title-gated-signatures` (5a7821f, on master 6374f73). Independent of
`gameapi/match-engine-signatures` (no shared files). Needs no in-game test to review; static evidence below.

### Problem
Several GameAPI wrappers call K2 functions with the K1 signature. The K2 functions take more arguments and pop them
(`ret N`), so each call leaves the caller's stack off by 4-20 bytes:
- `CSWGuiPanel::StartLoadFromLayout` (K2 `ret 0xC`, K1 `ret 4`) and `InitControl` (K2 `ret 0x10`, K1 `ret 0xC`);
- `CGuiInGame::ShowPartySelection` (K2 `ret 0x14`), `ShowUpgradeScreen` (K2 `ret 0x18`) and `UpdateMessageGUI`
  (K2 `ret 4`, takes the in-game messages panel as an argument);
- the destructor row that the CRes, CResGFF, CSWGuiBorder, CSWGuiImage, CSWGuiListBox, CSWGuiScrollBar,
  CSWGuiSlider and CSWGuiText wrappers resolve is the K2 scalar deleting destructor (`ret 4`, delete flag on the
  stack). Called without a flag, it reads a stale stack slot as the flag, and if bit 0 is set the engine frees the
  object before the wrapper frees it again.

### Approach
- K1 typedefs and call paths are unchanged. Each affected function gets a K2 typedef assigned from the same
  `GetFunctionAddress` result, and the method branches on `GameVersion::GetTitle() == GameTitle::KOTOR2`. An unknown
  title takes the K1 path.
- The extra K2 arguments are the values the K2 engine itself passes at its own call sites.
- Destructors go through a small `CallGameDestructor` helper (`GameAPI/GameDestructor.h`) that passes flag 0 on K2,
  so the wrapper keeps ownership (`malloc` / `free`).
- Every K2 "Destructor" role touched here is `param_size_bytes = 4` in `kotor2_gog_aspyr.db`; on Steam the rows that
  exist (CRes, CResGFF, CSWGuiBorder) are deleting destructors too.
- Removed CResGFF's unused `Destructor_2` pointer and updated the stale CGuiInGame header note.

Evidence: disassembly of each function on K2 GOG and K2 Steam, plus K1 GOG 1.03 for the unchanged K1 path
(signatures, `ret N`, caller argument pushes, RTTI for the GUI classes).

Not in this PR: the K2 `CGuiInGame::ShutDownGlobal` database rows point at an unrelated deleting destructor
(the real function is GOG 0x4D8230 / Steam 0x7C7B40). That is a database fix; it is dormant today because
`CGuiInGame::InitializeFunctions` stops at an earlier missing K2 row.

### Testing
- [x] `python3 tools/validate-patches.py` (0 errors)
- [x] `python3 publish-patches.py --target windows_x86`: 25/25 packaged, including the K1 users ModOptions and Snake
- [x] no new `-Wall -Wextra` warnings
- [ ] Tested in game: not needed for review; K2 Steam test TBD

🤖 Generated with [Claude Code](https://claude.com/claude-code)
