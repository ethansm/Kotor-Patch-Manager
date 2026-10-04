# I-3 build2 summary (kpm-pr-gameapi2, uncommitted)
- StartLoadFromLayout (K2 ret 0xC, call (resref,1,1)) and InitControl (K2 ret 0x10, extra arg 1): CSWGuiPanel gets K2 typedef + pointer from the same GetFunctionAddress result; branch on GetTitle()==KOTOR2.
- ShowPartySelection (extra arg 0) and ShowUpgradeScreen (K2 ret 0x18, OBJECT_INVALID, 0, 0, {nullptr,0} ExoStringByValue): same pattern in CGuiInGame.
- UpdateMessageGUI K2: passes the caller-owned messages panel read from CGuiInGame+0x78 (hard-coded constant, no K2 offset rows); early-returns if it is null.
- 8 deleting dtors (CRes, CResGFF, Border, Image, ListBox, ScrollBar, Slider Destructor_2, Text): new GameDestructor.h CallGameDestructor(); K2 calls (obj,0), then the existing free(obj); K1 call identical; adds a null check.
- Also: removed unused CResGFF destructor2 (and its Destructor_2 resolve); fixed stale CGuiInGame.h and Slider comments. ShutDownGlobal untouched.
- Tests: validate-patches 0 errors (64 checked); publish-patches windows_x86 Packaged 25/25 OK (incl. ModOptions, Snake); ModOptions create-patch OK; -Wall -Wextra 0 warnings old and new.
- Cleaned build artifacts; diff vs upstream/master = 13 modified files + new GameDestructor.h, all in Patches/Common/GameAPI/. Nothing run in game.
