# u5 alt-tab / wndproc (Haiku rea-recon, 2026-10-15; written by Opus from the agent reply, Write was refused to the agent)
Binary: Steam path swkotor2.exe (layout matches GameAddr.h).
| address | what | evidence |
|---|---|---|
| 0x0040a750 | main WndProc; cases 0x1C (WM_ACTIVATEAPP) and 0x05 (WM_SIZE) read | procedure_pseudo_code |
| DAT_00a1b480 | outer "KotOR2" HWND: never destroyed or recreated on any path read | pseudocode of 0x40a750, 0x409750, 0x40cb20, 0x40cc70 |
| 0x00409750 | destroys + recreates render HWND DAT_00a1b484 (via 0x408af0) | pseudocode |
| 0x0040cb20 / 0x0040cc70 | movie player: its own movie window only, no DAT_00a1b480 reference | pseudocode |
| 0x0078e250 | unread; decides whether the activate path reaches 0x409750 | session dropped |
VERDICT: PARTIAL
