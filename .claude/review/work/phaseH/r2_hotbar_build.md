# r2 recon: HotbarBuildButtons at 0x0040F5A0 entry vs HUD +0x48

## 1. Build path (ActionHotbar.cpp)
- HotbarBuildButtons: patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:1965
- Projection flag save/clear: ActionHotbar.cpp:1976-1982 (inline, not a ProjGuard struct). Restore: ActionHotbar.cpp:1997-2000.
- Creates: createLabel loop ActionHotbar.cpp:1990; body comment ActionHotbar.cpp:1975 says the ctor clears the flag at 0x0074850D right after the hook.
- createLabelFromTemplate: patch_manager_mods/_shared_src/GuiKit.h:152-180. Calls labelCtor, then initControl(panel, mem, exo, addToList=0, scale=1) at GuiKit.h:171. Reads panel count (GuiKit.h:178) only. Writes only to the new label (id, flags).
- appendToPanel: GuiKit.h:186-199. Reads count, writes ctl+0x54 id, calls PtrListAdd(panel+0x24, ctl) at GuiKit.h:193, then verifies count == index+1 and array[index] == ctl.

## 2. Writes to the panel object (HUD)
- Direct writes by our code to panel: none. Our code only rdInt/probeRead on the panel (ActionHotbar.cpp:1970, 1995-2002).
- Engine calls that get panel as this/arg: InitControl 0x0040F620 (GuiKit.h:171; GameAddr.h:23). Its writes to panel are not visible in the repo (native code, unverified).
- PtrListAdd (CExoArrayList::Add, 0x0083EA60 Steam; GameAddr.h:31) on panel+0x24. Writes list ptr/count/cap, possibly realloc. Field offsets: ptr +0x24, count +0x28 (Panel_CtlCount, GuiKit.h:49), cap presumably +0x2C (not confirmed in repo).
- Panel +0x30 (Panel_Gff/Panel_Open): read-only in our code (ActionHotbar.cpp:66).
- Panel +0x48 (flags): read-only in our code (ActionHotbar.cpp:284). No write from our code.

## 3. ProjGuard / 0x00A1B9F0
- Save at entry (ActionHotbar.cpp:1977-1978), clear to 0 (1982), restore saved value if nonzero (1997-2000).
- Timing: the hook at HH_LayoutClosing (hudhost HudHostProvider.cpp:153) runs at entry of 0x0040F5A0. The ctor clears the flag at 0x0074850D after that call returns (ActionHotbar.cpp:1975; RuntimeGuiControls.md:73-74). So the restore re-sets a nonzero value inside the hook, and the ctor's later clear at 0x0074850D zeroes it again. Assumes the call site and 0x0074850D are unchanged (not verified in disassembly here).

## 4. Capacity / overflow
- appendToPanel never writes past capacity itself. Growth is inside engine PtrListAdd. Repo notes say CExoArrayList Add doubles from 16 (10_engine_optimization_research.md:329). Not verified for this exact function.
- Panel list fields: {ptr +0x24, count +0x28, cap probably +0x2C} (GuiKit.h:184 comment; GuiKit.h:49).

## Key finding: the value is already known to break the gates
- PanelIgnoredByInput = 0x600 (ActionHotbar.cpp:74; CompanionSelfHeal.cpp:60).
- 0xFF8FB447 & 0x600 = 0x400 (nonzero), so isIgnored() is true (ActionHotbar.cpp:284) and the hotbar hides.
- Commit d719d83 records the same family: 0xFF8FB44F in Phase G live (CompanionSelfHeal.cpp:488, :1092).
- The low byte 0x47 is intact (bits 1, 2, 6). The corruption is in the upper 24 bits (0xFF8FB400). No code in the repo writes those bits to panel+0x48. Source not found.

## Open
- Which engine write sets panel+0x48 high bits: InitControl (0x0040F620) or PtrListAdd, or a stale/wrong panel pointer at hook time. Needs disassembly of 0x0040F620 writes to this+0x48 or a watchpoint.
