# re2: follow-up rea-recon for WI-87 and SH-3a-08 (2026-10-15, 9 Haiku rea-recon runs, 6 useful)
Binary: `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe` (sha256 7751f7d8...c04f).
Its layout matches `_shared_src/GameAddr.h` (operator new 0x919723, LabelCtor 0x419740, InitControl 0x40F620), NOT the `K2Addr.h` `t.*` table
(there t.New 0x921347 is `__vsnprintf` and t.LabelCtor 0x9179E0 sits inside 0x916ef0 in this binary). Briefs must name this path and the GameAddr.h addresses.
Copies tracked in `.claude/reviews/gemini_20261014/fx3/re2/`. Unit files: u1_label_dtor, u2_allocator, u3b_panel_release, u4_mode_change, u5_alttab, u6_render_style (u1/u4/u5 written by Opus from the reply: the
rea-recon agent has no Read tool, so Write to an existing file is refused; always give it a NEW file name).

## WI-87 (BG40 label leak): answered, unpark
- Label vftable 0x9878bc slot 0 = 0x7510b0 (read_bytes), scalar deleting dtor: thiscall, RET 4 (`flags`), dtor chain 0x417290 / 0x414c60 / 0x418640
  releases its own sub-objects via their slot1(1); if `flags & 1` it calls `_free` 0x9196fd (DB: operator delete, high).
- operator new 0x919723 -> `_malloc`; 0x9196fd -> `_free`; both use heap global DAT_00a806c8: same heap, so slot0(1) on a GuiKit-allocated label is correct.
- InitControl 0x40F620 with addToList=0 writes no panel list, focus or hover; it only sets ctl+0x38 = panel. addToList!=0 stores ctl at panel+0x24[ctl+0x54].
- No engine routine calls child slot0(1) over panel+0x24 (panel dtor 0x40e860 frees only its +0x60/+0x30 objects): engine children are embedded members.
- Fix rule: on the CF_Template failure (vtable == labelVt) call the label's own vtable slot 0 with flags=1 (read at runtime from the object: no new address).
  On CF_Vtable (vtable != labelVt) the object is not a known label: do not call through an unknown vtable (keep the leak, or log only).
  Never slot0(1) an embedded (engine-owned) label.

## SH-3a-08 (window subclass lost): likely real, runtime check names the window
- Outer window class "KotOR2" (0x9f42c8), HWND DAT_00a1b480: never destroyed or recreated on any path read (WndProc 0x40a750, 0x409750, movie 0x40cb20/0x40cc70).
- "Render Window" (0x408af0 CreateWindowExA, HWND DAT_00a1b484): dwStyle 0x02CA0000, parent NULL, so TOP-LEVEL; 0x409750 destroys + recreates it.
- 0x409750 has 7 direct callers; WndProc 0x40a750 activate path reaches it via 0x78e250 -> 0x40be50 / 0x40bf40 (alt-tab / mode change), not startup only.
- Mod side: gui_input `findGameWindow` (GuiInputProvider.cpp:50) and action_hotbar `findGameWnd` (ActionHotbar.cpp:902) pick the LARGEST VISIBLE
  top-level window of the pid via EnumWindows. If that is the Render Window, the subclass dies on recreate and `ensureSubclass` returns early on `Hook::active()`.
- Host fix candidate: `ensureSubclass` re-installs when `!IsWindow(Hook::hwnd)` (WindowHook host test). Runtime check: log the class name of the subclassed HWND.
- OPEN: which UI actions drive 0x7d5580 / 0x7d7290 / 0x7d8ff0 (callers of 0x409750 with no resolved caller).
