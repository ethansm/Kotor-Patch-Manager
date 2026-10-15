# SH-3a-08 U6 render-window recon (read-only, REA ghidra 12.1.4)

Binary sha256 7751f7d8...dc04f. Addresses in REA form (procedure 0x408af0, data 0x00a1b484).

| address | what | evidence |
|---|---|---|
| 0x408af0 | CreateWindowExA(dwExStyle=0x40000 WS_EX_APPWINDOW, lpClassName="Render Window" copied from 0x00986c00, lpWindowName=FUN_00734010() result (text UNKNOWN), dwStyle=0x02CA0000, X/Y/W/H=0x80000000 CW_USEDEFAULT, hWndParent=0 NULL, hMenu=0, hInstance=[EBP+8], lpParam=0). Ghidra labels IAT 0x00986454 as CreateWindowExA | procedure_assembly 0x408baa-0x408bdf |
| 0x408af0 | dwStyle 0x02CA0000 = WS_CLIPCHILDREN+WS_BORDER+WS_DLGFRAME(=WS_CAPTION)+WS_SYSMENU+0x20000. WS_CHILD 0x40000000, WS_POPUP 0x80000000, WS_VISIBLE 0x10000000 all CLEAR. No ShowWindow in body | procedure_assembly 0x408bc8; hand decode |
| 0x408af0 | hWndParent is literal NULL, NOT DAT_00a1b480 (PUSH 0x0 at 0x408bb2). Top-level window | procedure_assembly 0x408bb2; pseudo 0x408af0 |
| 0x408af0 | WNDCLASSA.lpfnWndProc=0x40a750 (main WndProc); RegisterClassA. Returned HWND stored to DAT_00a1b484 at 0x408c02 | assembly 0x408b57, 0x408c02 |
| 0x409750 | Recreates render window: saves DAT_00a1b484, clears it, FUN_00408af0(DAT_00a13c6c,1), vtable(DAT_00a1b4a0)+8(DAT_00a1b480, new HWND, ...), then DestroyWindow(old) | procedure_pseudo_code 0x409750 |
| 0x409750 | 7 direct callers (resolved): 0x407920, 0x40be50, 0x40bf40, 0x78dbf0, 0x7d5580, 0x7d7290, 0x7d8ff0. Calls 0x73fce0 only if param_5==1 | procedure_callers 0x409750 |
| 0x78e250 | Gate: *(ECX+0x130)!=0 AND FUN_0093b600()!=0. Then 0x7b38d0(8), 0x40be50, 0x93b630, 0x798820(1), 0x40bf40, 0x7b3930(8), 0x93b5e0 | procedure_pseudo_code 0x78e250 |
| 0x78e250 | Reaches 0x409750 at 2 levels. Via 0x40be50: 0x409750 only if vtable+4 != 0 and (DAT_00a1b6fc==0 or obj+0x1f8 or obj+0x1fc != 0). Via 0x40bf40: only if DAT_00a1b768==1, DAT_00a1b76c==0, counter DAT_00a1b764 >=0 after decrement, obj+0x4c4==0. Both pass param_5=0 | pseudo 0x40be50, 0x40bf40; callers 0x40be50, 0x40bf40 |
| 0x78e250 | Fullscreen / mode-change / device-lost are NOT tested by name. Tests use an unnamed vtable slot and globals DAT_00a1b768/6c/6fc, DAT_00a1b764 | pseudo review |
| 0x73fce0 | Wrapper; sole caller of 0x78e250. Callers: 0x409750 and 0x40a750 (WM_ACTIVATEAPP 0x1C, wParam!=0, under conditions) | procedure_callers 0x73fce0 |
| 0x407920 | Main loop: on idle (PeekMessage empty) if DAT_00a1b4d4!=0 calls 0x409750 with stored W/H/BPP, or 0x280,0x1e0,0x20 if obj+0x4c4!=0; flag DAT_00a1b4d4==2. Caller 0x91d424 | pseudo 0x407920; procedure_callers 0x407920 |
| 0x40a750 | WM_SIZE(0x0005) on DAT_00a1b484 with wParam 2 (SIZE_MAXIMIZED) sets DAT_00a1b4d4=2. Custom msg 0x40A calls 0x40bf40. No WM_DISPLAYCHANGE(0x7E) case | pseudo 0x40a750 |
| 0x78dbf0 | Mode change: EnumDisplaySettingsA(NULL, param_1); only if param_1 != this+0x1a0; calls 0x409750(W,H,BPP,flag,1); stores this+0x1a0. Callers 0x73fbd0 (from 0x7d9280, no resolved caller) and 0x78a630 (from 0x788d90, from 0x73fad0, called in 0x407920 init) | pseudo 0x78dbf0; callers 0x78dbf0, 0x73fbd0, 0x78a630, 0x788d90, 0x73fad0 (pseudo) |
| 0x7d5580, 0x7d7290, 0x7d8ff0 | Switch handlers on codes 0x28/0x2e, 0x2a, 0x2d (meaning UNKNOWN). Each calls 0x409750 only when a this-flag is set: +0x214c; +0x293c==0 and +0x2940!=0; +0x840. Resolved callers: NONE. 0x7d8ff0 has one DATA ref from 0x009a1cf8 | pseudo; procedure_callers; procedure_references 0x7d8ff0 |
| 0x40be50 / 0x40bf40 | Callers: 0x78e250, 0x7986a0, 0x7a0fb0 / 0x40a750, 0x781be0, 0x78e250, 0x7a0fb0, 0x8925f0 | procedure_callers |

UNKNOWN: UI action behind 0x7d5580/0x7d7290/0x7d8ff0 (no resolved callers). Sender of msg 0x40A. Writers of DAT_00a1b4d4 besides 0x40a750. Incoming refs for 0x7d5580 and 0x7d7290 exceeded output limit, not read.
