# u4 mode change (Haiku rea-recon, 2026-10-15; written by Opus from the agent reply, Write was refused to the agent)
Binary: Steam path swkotor2.exe (layout matches GameAddr.h).
| address | what | evidence |
|---|---|---|
| 0x00409750 | DestroyWindow(old) after FUN_00408af0 recreates the "Render Window" | procedure_pseudo_code |
| 0x00408af0 | CreateWindowExA "Render Window"; stores new HWND in DAT_00a1b484 | procedure_pseudo_code |
| 0x009f42c8 | "KotOR2" = outer window class; outer HWND DAT_00a1b480 is NOT recreated by 0x409750 / 0x478300 | read_bytes |
| 0x0040a750 | WndProc; reaches 0x409750 (via 0x40bf40 / 0x73fce0), so recreate is not startup-only | callers |
Mod side (Opus, rg): gui_input findGameWindow (GuiInputProvider.cpp:50) and action_hotbar findGameWnd (ActionHotbar.cpp:902) use EnumWindows = top-level windows, i.e. the outer "KotOR2" window, if Render Window is a child.
OPEN: which message routes 0x40a750 -> 0x40bf40/0x73fce0; is Render Window WS_CHILD of the outer window?
VERDICT: PARTIAL
