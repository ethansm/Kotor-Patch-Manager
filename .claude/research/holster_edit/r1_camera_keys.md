# R1 recon: equip preview camera, equip-screen keys, window hook, input path

Method: static dumps only (`~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/`, decomp/*.c, *.asm, imports.tsv,
callgraph.tsv), exe bytes via objdump, `static_facts.py`, REA ledger rows (`rea_findings/20261015_holster-c3*`, `20261016_holster-c4*`,
`20261016_holster-c5*`). No live REA. Addresses are Steam sha 6a522e71. "ledger" = a REA row, "decomp" = Ghidra .c, "asm" = dump .asm.

## (a) Camera, rig, camerahook

Rig Gob
- Rig name default `char rig[64] = "charrec_light"` at `patch_manager_mods/equipment_3d_character_src/Equip3dLogic.h:23` (INI key `rig`, :91).
- Created by `sceneview::addModel(view, g_cfg.rig, -1)` at `EquipCharPreview.cpp:883`. Scene model 0 is `getModel(view,0)`:
  Scene = view+0x60 (`SceneView.h:98`), `SceneGetModel` = 0x4184B0 (`_shared_src/GameAddr.h:52`).
- Rig vtable = Gob::vftable 0x98B5CC (ledger C3 row `0x0098b5cc`).
- Gob vtbl +0x10C = 0x4587F0 = `Gob::FindPart(name)`: 67 B, RET 4, decomp `45/004587f0.c`. "root" returns [this+0x98];
  otherwise `FUN_00456b00` (recursive stricmp over children at +0x30/+0x34, decomp `45/00456b00.c`).
  Yes: thiscall, RET 4. Name "FindPart" is the ledger's label; the body matches.
- Other Gob slots checked by `SceneView.h:224-229`: +0x5C SetPos 0x461490, +0x60 SetOri 0x4614E0, +0x64 GetPos 0x459710, +0x68 GetOri 0x459740, +0x108 GetParent 0x461530.
- GAP: `gobCheck` (`SceneView.h:219-246`) does not verify +0x10C (FindPart). `GobVtReadable = 0x10c` (`K2Addr.h:357`) probes bytes 0..0x10B, so the slot's own 4 bytes are outside the probe.

Attach path
- Camera vtbl +0x74 = 0x4819E0 (`cameraAttach` at `SceneView.h:199-205` calls slot `SceneCamVt_Attach` = 0x74, `K2Addr.h:350`).
  asm 0x4819E0-0x481A01: ECX = cam+4; pushes flag, name, model; `CALL 0x461670`; `RET 0xC`.
- `FUN_00461670` (decomp `46/00461670.c`, RET 0xC, `this` = cam+4):
  1. node = `model->vtbl[+0x10C](name)` (the FindPart call, asm 0x4616C4 per ledger C3).
  2. `node->vtbl[+0x0C](cam+0xA8, cam+0xB4)` writes node world pos and quat into the camera pose.
  3. switch(flag): case 1 -> `FUN_00504F60` = CAurBehaviorAttachTranslate ctor (vtbl 0x991914, zeroes +0x3C..+0x44; ledger C4 row 0x00504f60).
- Node vtbl +0x0C: Part-family vtables (Part 0x98B36C, PartTriMesh 0x98B414, PartLight 0x98B544, ...) all point to 0x457510,
  "Part world pose(vec3,quat)", which walks the parent chain then adds owner Gob pose +0xA4/+0xB0/+0x1E0 (ledger C3, read_bytes).
  The camerahook node's concrete class in the charrec_light MDL is UNKNOWN (not checked).
- Per frame: listener `0x5045E0` ("UpdateDependentPosition", ledger C3) calls the attach update `0x5047B0`, which again
  calls node vtbl+0x0C into child piVar1+0xA4 / +0xB0 (ledger C3 row 0x005047b0).
  Translate refresh `0x505020` (decomp `50/00505020.c`): pos = node pos + rotated offset (+0x3C..+0x44). Offset is zero on this path.
  PROBABLE only: ledger C4 says "shape only".

Camera object layout
- Camera ctor `0x47ECD0` (decomp `47/0047ecd0.c`, asm `47/0047ecd0.asm`): [cam] = CAurCamera vt 0x98C4F4, then Gob ctor
  `0x458B70` on cam+4, then [cam] = Camera vt 0x98C45C (= `ExpectedCamVtable`, `EquipCharPreview.cpp:51`), then [cam+4] = 0x98C27C.
  So cam+4 is NOT a Gob by vtable after construction. Its pose fields sit at the Gob offsets: pos cam+0xA8 (x,y,z), quat cam+0xB4 (w,x,y,z).
- FOV: camera vtbl +0x44 = 0x47F190 (decomp `47/0047f190.c`): `[this+0x204] = fov`. `cameraFov` (`SceneView.h:207-213`) matches.
  The camera apply routine `0x47F320` feeds it to `gluPerspective(fov, aspect, +0x210, +0x214)` (decomp line ~188).

Axis convention
- `0x47F320` line 280: `FUN_00435F00(cam+0xA8, cam+0xB4)` on the normal path (DAT_00A32A54 == 0).
- `0x435F00` (decomp `43/00435f00.c`, normal path DAT_009F497C == 0): glLoadIdentity; `glRotatef(-(2*acos(w))*180/pi, axis)`;
  `glTranslatef(-x,-y,-z)`. Axis from `FUN_004360F0(quat, out)` (decomp `43/004360f0.c`; `(x,y,z)/sin(theta/2)`, theta = 2*acos(w)).
- So view = R(q)^-1 * T(-p). Camera world pose is (p = pos, q = quat). Standard GL: eye looks down its local -Z, up = +Y.
- Result: the camera sits at the camerahook node origin and looks along the camerahook node's local -Z, with local +Y up.
  The node's own axis convention (from the MDL data) is UNKNOWN.

## (b) Equip-screen keys and mouse

- Equip panel: `CSWGuiInGameEquip` ctor `0x8A92D0` (5143 B, decomp `8a/008a92d0.c`). Method list in `inputs/db_functions.tsv:610-618`:
  SetCharacter, Update (vtbl 0x34), OnPanelAdded, OnPanelRemoved (vtbl 0x4C), UpdateInventory, EquipItem, UnequipItem. None is a key/char/mouse handler.
- Literal scan: the ctor and every decomp under `8a/` and `8b/` have no X/Y/Z (0x58-0x5A), Esc (0x1B) or mouse (0x201-0x206) compare.
- Vanilla `equip_p.gui` (`~/Games/KOTOR2_Mod_Workspace/ultrawide_gui/originals/equip_p.gui`, GFF, 37553 B): tags are
  BTN_BACK, BTN_EQUIP, BTN_SWAPWEAPONS, BTN_PREVNPC, BTN_NEXTNPC, BTN_INV_* (slots), LBL_*, plus ROTATE/LEFT/RIGHT (3D rotate arrows, meaning not checked).
  No hotkey or key-binding token.
- Keymap tables: `CTwoDimArrays::Load2DArrays_Keymap` `0x6EA570` loads the "keymap" 2DA (string 0x99AE40). The 2DA is inside a BIF
  (`scan_2da_limits_output.txt:160`), not in the repo. Its rows were not read. The options screen is `CSWGuiInGameOptKeyMappings` (`BTN_KEYMAP` 0x9A8E38).
  No link from CSWGuiInGameEquip to the keymap.
- Generic key path: WndProc `0x40A750` (below) calls `0x40A160` (WM_KEYDOWN) and `0x40A460` (WM_CHAR).
  `0x40A160` maps VK 0x25-0x28 (arrows) to codes 0x1C-0x1F and calls `0x40A460`.
  `0x40A460` compares its code against state-chosen constants (0x60 / 0xB0 / 0x03, state from `0x73F350`). None is X/Y/Z. Who consumes these codes on the equip panel: UNKNOWN.
- Mouse: recorder `0x72E030` cases 0x204/0x205 (RMB down/up) store event id DAT_0099C8A0, separate from LMB DAT_0099C89C.
  WndProc `0x40A750:351-362` remaps RMB to LMB when the GUI object flag (`0x72FB00()+0x5C`) is set. No equip-specific RMB handler found. Whether RMB does anything on the equip screen: UNKNOWN.
- X/Y/Z: no X/Y/Z compare on the equip path. Only `'Y'`/`'Z'` char compares in `0x7B0E90` (lines 58, 75; it calls 0x40A160). Context not traced: a lead.
- GetAsyncKeyState: the game imports no GetAsyncKeyState, GetKeyState or GetKeyboardState (`imports.tsv`). Mod polling is safe there.
  `EquipCharPreview.cpp:403` uses VK_LBUTTON and `:403`/`:803` gate on `GetForegroundWindow() == g_hwnd`.
  VK_RBUTTON (0x02), VK_X (0x58), VK_Y (0x59), VK_Z (0x5A) can be polled the same way. Polling reads physical state, so swallowing WM_* does not affect it.

## (c) WindowHook.h and Esc

- Template `kp::WindowHook<Tag>` (`_shared_src/WindowHook.h:24-54`). Per-Tag statics: `old` (previous WNDPROC), `hwnd`, `passthrough` (volatile LONG), `pins`.
- `install(h, proc)` (:31-42): pins the DLL image once (GetModuleHandleExA FROM_ADDRESS|PIN, :32). If the stored window is dead, resets state (:34).
  If already subclassed, clears `passthrough` and returns (:35-38). Otherwise SetWindowLongPtrA(GWLP_WNDPROC) and stores `old` (:39-41).
- `active()` = `old && !passthrough` (:43). `forward()` = CallWindowProcA(old), or DefWindowProcA if no old (:44).
- `uninstall(proc)` (:45-53): if the current WNDPROC == proc, restores `old` (:47-49). Otherwise sets `passthrough=1` and keeps `old` (:51): the proc stays chained, inert, always forwarding.
- Chaining and swallowing (usage line :6): each proc handles what it wants, then `return Hook::forward(...)`.
  To swallow WM_CHAR / WM_KEYDOWN while a text field has focus: `if (Hook::active() && textFocused) return 0;` with no forward.
  That stops the message before the game WndProc (`0x40A750`), which is `old`.
- EquipCharPreview.cpp does not subclass directly. It includes GuiKit.h (`:29`), which holds the wheel subclass built on kp::WindowHook.
- Esc: game WndProc `0x40A750:57-67`, WM_KEYDOWN with VK 0x1B: only special case is a movie in progress
  (`0x740160` -> IsMoviePlaying `0x798C00` -> skip `0x740F60` -> `0x7A0500`). Then the gate `0x73F2F0` (returns [[obj+4]+0x90]; "keyboard modal" name inferred from `TurnOffKeyboardModal` 0x73FE60),
  then `0x40A160`. No equip-specific Esc code. Whether Esc closes the equip panel (BTN_BACK or a GUI key map): UNKNOWN.

## (d) Keyboard: window messages vs DirectInput

- Window messages are used. Game WndProc = `FUN_0040A750`, set as lpfnWndProc at `0x408AF0:37` and `0x4087A0:79`.
  It handles WM_KEYDOWN (0x100), WM_CHAR (0x102 -> `0x40A460`) and mouse messages. Pump `0x4094F0` = PeekMessage/GetMessage/TranslateMessage/DispatchMessage.
  SetWindowLongA calls (`0x40CC70`, `0x477990`, `0x476BE0`) use GWL_STYLE (-0x10), not GWL_WNDPROC.
- DirectInput is used too. imports.tsv:6501-6502: DirectInput8Create (DINPUT8.DLL) called from `0x730750` (asm `007307D7`, `007307FC`), DI manager init, called from `0x72A130`.
  - CreateDevice(GUID_SysKeyboard {6F1D2B61-...}, GUID bytes at 0x99CB88), vtbl +0x0C.
  - SetDataFormat(0x9B932C: 256-byte keyboard format, dwDataSize 0x100), vtbl +0x2C.
  - SetCooperativeLevel(hwnd, 6 = NONEXCLUSIVE|FOREGROUND), vtbl +0x34.
  - SetProperty(DIPROP_BUFFERSIZE = 0x100), vtbl +0x18: buffered keyboard.
  - Keyboard device stored at manager +0x24 (`param_1[9]`). Acquire (vtbl +0x1C) on [param_1+0x24] at `0x7311C0`, `0x732C40`, `0x732F40`, `0x733090` (assumes param_1 = DI manager; not confirmed per site).
  - Joystick enum class 4 (GAMECTRL) in the same init: `0x730750:54`, callback `0x7306A0`, PS3/PS4 name test `0x730E90`.
- Consequence: the game reads keys through DI as well as WM. Swallowing WM_KEYDOWN/WM_CHAR hides keys from the GUI/WndProc path only, not from a DI reader.
  Which function reads the DI keyboard state or buffer (GetDeviceState +0x24 / GetDeviceData +0x28 on mgr+0x24): UNKNOWN, not found by pattern search.
- No GetKeyboardState, GetKeyState or GetAsyncKeyState import in the game.

## Open items
- Which DI keyboard reader feeds gameplay, and whether equip GUI consumes Esc, RMB, X/Y/Z: UNKNOWN (GUI dispatch past `0x40A160`, keymap 2DA rows).
- Specific class of the `camerahook` node in charrec_light (Part subtype): not checked.
- Translate-mode position math (`0x505020`): PROBABLE only.
