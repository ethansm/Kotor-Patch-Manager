# D2b recon: CSWGuiInGameOptions hook sites (Steam swkotor2.exe, sha 6a522e71)

Read-only. Static dumps `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71` (`decomp/<xx>/<addr>.asm`, `callgraph.tsv`,
`engine_ai_atlas/tools/cg.py indirect --slot`), knowledge DB via `scripts/query_ghidra.py`, raw bytes and vtable read from the
dump's `swkotor2_vanilla_6a522e71.exe` (.rdata). No live REA, no edits. "GUESS" = inferred from shape, not confirmed.

Object: panel = 0x24D8 bytes (`PUSH 0x24D8; CALL new` at 0x7BF7A7 and 0x7D0A58). Ctor `0x8A1170` is thiscall, `RET 4`,
arg = `[parent+0x38]`. Vtable 0x9A701C, written at 0x8A11BA (`MOV dword ptr [ECX],0x9A701C`); 28 slots (0x00-0x6C).
Slot 0x70+ is .rdata string data (BTN_* names), not vtable.

## 1. Ctor hook
- 0x8A1704 = `E8 97 DE B6 FF` = `CALL 0x0040F5A0` = CSWGuiPanel::StopLoadFromLayout (DB, verified B0). It is a rel32 CALL,
  so it FAILS the "no rel32 inside the stolen span" rule. Panel is ECX there because of 0x8A16FE.
- **Proposed site 0x008A16FE**: `8B 8D 54 FF FF FF` = `MOV ECX,dword ptr [EBP-0xAC]` (6 bytes, one insn, disp32 only, no rel32).
  It follows the last InitControl `CALL 0x0040F620` at 0x8A16EA and the CExoString dtor at 0x8A16F9. Same shape as equip H-E1
  (0x8AA16A `8B 8D 28 FE FF FF`).
- No dumped branch lands in 0x8A16FF..0x8A1703 (grep of whole dump, 0 hits). The ctor's 3 internal jumps are at 0x8A1845+.
- ORDER: the vanilla click wiring (AddEvent calls 0x8A1749..0x8A18E9) and initial focus (`CALL 0x8A1C10(1)` at 0x8A18F6) run
  AFTER 0x8A1704. Our hook runs before both, so our own control's AddEvent is safe to do there.

## 2. Per-frame and input
- **Per-frame = vtable slot 0x34 (13)**, value 0x0040EC90 = CSWGuiPanel::Draw (base; options panel does not override it).
  Caller: CSWGuiManager::Draw 0x4113A0 (`MOV EAX,[EDX+0x34]` ... `CALL EAX` at 0x4115EB..0x4115EE; cg.py indirect lists 3 sites
  there). Equip's "Update" H-E3 (0x8ABA50) is the same slot, overridden by equip.
  - Hook 0x0040EC90 entry, prologue `55 8B EC 83 EC 3C` (6 bytes; next insn `89 4D C8` at 0x40EC96), ECX = panel.
    It is the base for EVERY CSWGuiPanel: filter `ECX == our panel`.
- **Input = slot 0x3C (15) = 0x008A2070** (options override, `handler(code,flag) RET 8`, ECX = panel). This is the Steam twin of
  GOG CSWGuiOptionsMain::HandleInputEvent (0x568550), not a per-frame call.
  - Prologue `55 8B EC 83 EC 18` (6 bytes; next `89 4D EC` at 0x8A2076). Stack args [ebp+8]=code, [ebp+0xC]=flag.
  - Logic: if flag==0 or code outside 0x28..0xDF, default at 0x8A2125 = CSWGuiPanel::HandleInputEvent 0x0040EAA0. That one
    forwards (code,flag) to the ACTIVE control's vtable slot 15 (`[this+0x20]`).
  - Switch: index = code-0x28; byte table 0x8A214C, jump table 0x8A213C.
    - codes 0x28, 0x2D, 0x2E (case A, 0x8A20D9): GetInGameGui 0x73F750, HideSWInGameGui 0x7CA060; if true, SetInputClass 0x73FEE0(0,1); then SaveOptions 0x740B40.
    - code 0x2A (case B, 0x8A20B0): PlayGuiSound 0x4122A0, then 0x740B80 (ChangeCharacterToNextLivingPartyMember, DB).
    - codes 0x35, 0x36 (case C, 0x8A2111): SaveOptions 0x740B40 only.
  - Esc key code: UNKNOWN (top-bar CSWGuiInGameMenu::HandleInputEvent has no Steam name in the DB).
- Other slots: 0x08 = 0x40EC00 SetActiveControl (DB). 0x40 = 0x40EAE0 HitCheckMouse (DB). 0x4C = 0x8A2590 (SetActiveControl(0,0),
  then CSWGuiPanel::OnPanelRemoved 0x40F4C0; GUESS: unfocus-and-remove). 0x58 = 0x8878D0 (close wrapper, below). 0x48 = 0x8A24F0 (not read).

## 3. Destroy
- Scalar deleting dtor = slot 0 = **0x008A1930**: `55 8B EC 83 EC 20` (6 bytes; next `89 4D E0` at 0x8A1936). ECX = this at entry.
  It calls the body 0x8A1960 (`CALL` at 0x8A193C), then `CALL 0x9196FD` (operator delete) if flag&1.
- Body 0x8A1960 tears down the 8 array elements (CSWGuiButton::Destructor 0x41A3D0, loops at 0x8A19B6..0x8A1BC2), the +0x1CA0
  control, then base CSWGuiPanel::Destructor 0x0040E860 (DB verified; `CALL` at 0x8A1BED).
- Only caller of 0x8A1960 is 0x8A1930 (callgraph). Hook H-O5 at the 0x8A1930 entry runs before any child is freed.

## 4. Button dispatch and back
- No tag/id switch exists. Each control keeps its own event list: CSWGuiControl::AddEvent 0x418AF0 (ECX=ctrl, [ebp+8]=event id,
  [ebp+0xC]=data, [ebp+0x10]=cb; cb==0 removes). Entries are 12 bytes {+0 data, +4 cb, +8 event}; list at ctrl+0x3C, count at ctrl+0x40.
- Options registrations, event 0x27 = click. Array = `this+0x68 + i*0x1D0`, 8 elements; plus `this+0x1CA0`:
  - i0 (+0x68) -> 0x8A1C50: SaveOptions, then a 0x1E44-byte dialog if ctrl+0x50 != 0 (GUESS: SAVEGAME).
  - i1 (+0x238) -> 0x8A1D20 (GUESS: LOADGAME). Initial focus = `0x8A1C10(1)`, so elem 1 is active at open.
  - i2 (+0x408) -> 0x7D2D30 gameplay; i3 (+0x5D8) -> 0x7D2DF0 feedback; i4 (+0x7A8) -> 0x7D2EB0 autopause (all match the UX note).
  - i5 (+0x978) -> 0x8A1DF0 graphics; i6 (+0xB48) -> 0x8A1EB0 sound (match UX note).
  - i7 (+0xD18) -> 0x8A1F70 (GUESS: QUIT; not read).
  - +0x1CA0 (BTN_EXIT, GUESS) -> thunk 0x762E90 = `mov eax,[ecx]; jmp [eax+0x58]` -> slot 22 = 0x8878D0 = `slot15(0x28, 1)`.
  - Every element also gets event 0 -> 0x8A2210 (loop 0x8A1856..0x8A1877).
- Unknown control id: it has no entry in its event list, so nothing fires. Nothing falls through to a switch. The click dispatcher
  itself (reads ctrl+0x3C) was not traced: UNKNOWN address.
- BTN_EXIT / back chain: 0x762E90 -> 0x8878D0 -> slot 15 (0x28, flag 1) -> case A -> HideSWInGameGui 0x7CA060 (closes the whole
  in-game GUI) + SaveOptions.
- Intercept options while our editor is up:
  - (a) hook 0x8878D0 (`55 8B EC 51 89 4D FC`, 7 bytes; next `6A 01` at 0x8878D7; ECX = panel). Close our editor from the hook.
    The original body still runs and hides the GUI unless the framework can skip it. OPEN.
  - (b) hook 0x8A2070 and rewrite the flag to 0 for codes 0x28/0x2D/0x2E/0x35/0x36. The switch is skipped and the call falls
    to the base handler. Needs arg access at entry (before PUSH EBP). OPEN.
- Tab change: no options-tab switch function found. Tabs run through the top bar: ShowSWInGameGui 0x7C9DF0, UpdateCreatedInGameGUI 0x7D0760.

## 5. Pause
- 0x87A730 reads global `[0xA1B4A4]+8` and calls CServerExoApp::TogglePauseState 0x51C6B0 with arg 2 (DB verified B9).
- Callers include ShowSWInGameGui 0x7C9DF0 at 0x7C9FE2 (guard `[CGuiInGame+0xF18]==0`, 0x7C9FC3..0x7C9FCA) and HideSWInGameGui
  0x7CA060 at 0x7CA1C9 (a branch after 0x73F810 at 0x7CA1C2). Also 15 other callers (0x781BE0, 0x7CB7C0, 0x7CD510, 0x7D1E90, ...).
- So opening the in-game GUI pauses, and hiding it runs the same toggle. Polarity and meaning of arg 2: UNKNOWN.
- Options tab switch (UpdateCreatedInGameGUI 0x7D0760): its callee list has no 0x87A730 (callees: 0x7FAE60, 0x84C3A0, 0x893950,
  0x89CF30, 0x8A1170, 0x8A25C0, 0x8A6170, 0x8A92D0, operator new). So switching tabs does NOT re-toggle pause. CONFIRMED on the
  static graph.

## Construction count (question 2)
- Site A: CGuiInGame::Initialize 0x7BE4C0: `PUSH 0x24D8` at 0x7BF7A7, `CALL 0x8A1170` at 0x7BF7DA. Chain:
  0x781840 -> 0x810CF0 -> 0x811450 (DB: CSWCMessage::HandleServerToPlayerModule, medium) -> 0x73F870 (CClientExoApp::InitializeInGameGUI,
  verified) -> 0x78C330 (verified) -> 0x7BE4C0.
- Site B: UpdateCreatedInGameGUI 0x7D0760, lazy. Gated by `CMP [0xA10760],0` (0x7D0788) and per-slot `CMP [this+0xC+arg2*4],0`
  (0x7D0806). Creates at 0x7D0A58, calls 0x8A1170 at 0x7D0A7F. Called from ShowSWInGameGui and HideSWInGameGui on every open/close.
- Net: once per CGuiInGame instance, not per menu open. CGuiInGame lifetime (per session?) NOT verified.

## Proposed hooks (toml shape of equipment_3d_character_src/kotor2-steam-aspyr.hooks.toml)
```
H-O1 address=0x008A16FE type=detour function=OptBindView   original_bytes=[0x8B,0x8D,0x54,0xFF,0xFF,0xFF]        param source="[ebp-0xac]" int
H-O2 address=0x0040EC90 type=detour function=OptTick       original_bytes=[0x55,0x8B,0xEC,0x83,0xEC,0x3C]        param source="ecx" int (filter == panel)
H-O3 address=0x008A2070 type=detour function=OptInput      original_bytes=[0x55,0x8B,0xEC,0x83,0xEC,0x18]        param ecx; code/flag = entry [esp+4]/[esp+8] (OPEN)
H-O4 address=0x008878D0 type=detour function=OptClose      original_bytes=[0x55,0x8B,0xEC,0x51,0x89,0x4D,0xFC]   param source="ecx" int (skip-original OPEN)
H-O5 address=0x008A1930 type=detour function=OptDestroy    original_bytes=[0x55,0x8B,0xEC,0x83,0xEC,0x20]        param source="ecx" int
```
Each site: next instruction is an instruction boundary (checked). No dumped branch lands inside any span (grep of decomp/).

## OPEN
1. Framework: can a detour skip the original body, or rewrite caller stack args at entry? Needed for H-O3 and H-O4 (the header of
   equip3d says "original bytes still run afterwards").
2. Esc key code (top-bar CSWGuiInGameMenu handler address).
3. Button index names i0, i1, i7 (GUESS). Check the .gui order against the idx-to-callback table above.
