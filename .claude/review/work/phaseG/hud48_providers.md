# HUD +0x48 (CSWGuiInGame) vs providers: recon (read-only)

## Ranked suspects
1. save-events L1 store (not panel): `patch_manager_mods/save_events_src/kotor2-steam-aspyr.hooks.toml` @0x0052FD1E stolen `mov eax,[ebp-0x58]` / `mov dword [eax+0x1F254],1` writes the server app object, not a panel. Offset 0x1F254 is outside 0x40..0x4F.
2. save-events S2 SEH restore: hooks.toml @0x0053C5AA steals `mov fs:[0],ecx`. Only risk is a detour that fails to run the original bytes; skip_original_bytes=false, so it should run.
3. gui-input: no engine hooks (`hooks = []`). Only write is SetWindowLongPtrA GWLP_WNDPROC subclass on the game HWND (`GuiInputProvider.cpp:2,116,158`). Not a panel write. Likely effect is input routing, not memory corruption.

## Hook table (save-events only; gui-input has none)
- 0x00534527 detour SE_OnSaved, bytes 68 4C 3F 99 00 (push 0x993F4C), no params, exclude=[]
- 0x0053C5AA detour SE_OnAutosaved, bytes 8B 4D F4 64 89 0D 00 00 00 00, exclude=[]
- 0x0052FD1E detour SE_OnLoadRequest, 13 bytes incl. the store above, param [ebp-0x14] pointer, exclude=[]
- 0x008529E0 detour SE_OnAutosaveSlotLoad, bytes 55 8B EC 6A FF, exclude=[]
- 0x008FACA0 detour SE_OnNewGame, bytes 55 8B EC 6A FF, exclude=[]
- None of the 5 hooks writes a panel. exclude_from_restore is empty everywhere; no register is relied on across the call beyond what the stolen bytes reload.
- Same five bytes are said to be live already via Action Hotbar 1.2.1 (hooks.toml comment). Unverified against the live log.

## Memory writes in providers
- gui_input_src/GiDispatch.h:79 memcpy snapshot (DLL-local buffer).
- save_events_src/SeDispatch.h:53,105 memcpy into DLL buffers.
- No wr<>, reinterpret_cast-to-game-pointer, or 0x40..0x4F offset writes in GuiInputProvider.cpp or SaveEventsProvider.cpp.

## History and live evidence
- git log: gui-input 15c9bfd (initial), 2db77c4 (wip); save-events 57999f4, f19df9a. No live-run marker found.
- search_docs "gui-input live": no hit. "save-events live": only design doc 22 s4.1.

## Gaps
- query_ghidra.py address returned no hit for 0x534527, 0x53C5AA, 0x52FD1E, 0x8FACA0. Only 0x8529E0 maps (CSWGuiSaveLoad::LoadPCAutoSave, steam-db-B0). Engine function names for 4 of 5 hooks are unverified.
