# Task B3: Steam -> GOG Aspyr engine address map (portrait-slots, buff-duration-hud, shield-bar + shared headers)

Status: 2026-10-03, read-only static research. Nothing committed, nothing installed, live game dir untouched.

- **GOG exe SHA-256 (as KPM knows it):** `777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914` (GOG address DB `game_version` row 1 = "gog aspyr", and `target_versions` of all 23 `kotor2-gog-aspyr.hooks.toml` in the KPM Patches tree).
- **Steam reference:** `6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF` (pristine; the live 4gb-patched file has identical code).
- **Ghidra programs:** server `TSL/k2_win_steam_aspyr_swkotor2.exe` and `TSL/k2_win_gog_aspyr_swkotor2.exe`, read-only, via `scripts/ghidra_server_api.py` (see Method).

## Method

1. **Candidates** (tier 0, scripts): the `(size, RET n)` signature of each Steam function plus the per-region GOG->Steam delta from `gog_cluster_delta_map.py` / `steam_validated`, then neighbour-sequence comparison (`seq.py`). Hypotheses from the brief were treated as candidates only.
2. **Raw disassembly** of both binaries (`ghscripts/ExportRangeDisasm.java`: bytes + text + call targets, plus xrefs-to and vtable dumps), 5 rounds, 83 function pairs.
3. **Instruction-by-instruction alignment** (`align.py`): for every pair the two instruction streams are walked in lock-step. Each pair has the same instruction count, identical lengths, and every byte difference is explained as an absolute-address operand or a rel32 call/jump target. 0 unexplained differences in all 83 pairs. This yields the exact GOG address for every operand, call target and hook site in those functions, and byte-identity of every displacement (struct offset).
4. **Anchors** (RE_AGENT_GUIDE rules): a function mapping is only `exact` when (a) the body aligns as above AND (b) an aligned **call site in a mapped parent** calls it (quoted) AND (c) whole-binary xref counts to Steam/GOG function agree (82 of 83 equal; the one difference is explained under PtrListAdd). Globals are `exact` from an aligned operand in identical code (often 2-3 functions), plus equal initialised data where the data section has any (GuiRefW/H = 800/600, FlushArg = 1.0f). Offsets are `exact` when a quoted instruction in a mapped function carries that displacement and the GOG instruction has identical bytes.
5. **Decoys / traps handled:** decimal-vs-hex (all offsets read from asm bytes, never from decompiler text); stack-slot ret N (not used to decide); thunks (none involved); neighbour decoys: `TextColor` name-based guess 0x9153E0 rejected (12/36 insn), `PtrListAdd` second instance 0x4C7640 rejected for the panel list; `AppRef2` false candidate dropped. Same-(size,RET n) GOG function counts for the hook hosts: UpdatePortraits 1, TooltipText 1, RowDraw 2, SlotDrawGate 8 (so SlotDrawGate relies on its aligned parent call + equal xref count, not on size).
6. **Tool deviation to disclose:** `ghidra_server_api.py export-script` only accepts scripts from the KPM `ghidra_scripts` folder, which I may not write to. `gog_map_21b/run_ghidra.py` imports that same script, points its `SCRIPT_PATH` constant at `gog_map_21b/ghscripts/`, and calls its own `main()` unchanged (same read-only `-readOnly -noanalysis`, credentials consumed inside the module, never read by me). Credential files were not read.

Confidence key: **exact** = aligned + parent call/operand + (xref counts or data) agree; **high** = aligned body/operand match but a single supporting site; **low** = plausible, not instruction-verified; **unknown** = not resolved.

Used-by key: PS portrait-slots, BH buff-duration-hud, SB shield-bar; GuiKit/hotbar = shared header users outside the three mods.

## Opus gate (2026-10-03)
Re-read from `gog_map_21b/dumps/gog_r*.txt`: all 7 hook windows (16 instructions) byte-equal at the claimed GOG addresses; rows GetMaxForcePoints (call 00520975 -> 006977d0), TextColor = SetDefaultColor 0x915510 (call 00915911), ScreenW 0x9F22A4 (008d03c6 in HudScale), LabelVtable 0x98998C (00917a18 in LabelCtor), Slot_ArrowBuff 0xDDC (0051667f), Menu_Char2 0x3134 (005270e1 after PUSH LBL_CHAR2 0x98DF84, string confirmed in both strings.tsv) confirmed. Accepted for B4. Knowledge-DB rows not yet persisted.

## 1. Hook sites (all 7)

| # | Steam site | GOG site | enclosing fn (Steam -> GOG) | hook fn | original_bytes (Steam toml) | GOG bytes identical | window |
|---|---|---|---|---|---|---|---|
| 1 | 0x0074F25A | **0x005209DA** | UpdatePortraits 0x0074EE90 -> 0x00520610 | PS_SnapshotPortraitSlot (portrait-slots) | `89 85 44 ff ff ff` | **YES** | 1 whole insn, 6 B |
| 2 | 0x0074F8AB | **0x0052102B** | UpdatePortraits 0x0074EE90 -> 0x00520610 | PS_SnapshotPortraitSlot (mirror slot) | `89 95 a4 fe ff ff` | **YES** | 1 whole insn, 6 B |
| 3 | 0x00745774 | **0x00516D74** | SlotDrawGate 0x00745770 -> 0x00516D70 | PS_PortraitSlotDraw (portrait-slots) | `89 4d fc 8b 45 fc` | **YES** | 2 whole insn, 6 B |
| 4 | 0x00745A84 | **0x00517084** | RowDraw 0x007457E0 -> 0x00516DE0 | RetargetBuffIcon (buff HUD) | `8b 4d e0 8b 11 8b 4d e0 8b 42 38 ff d0` | **YES** | 5 whole insn, 13 B |
| 5 | 0x00745A97 | **0x00517097** | RowDraw 0x007457E0 -> 0x00516DE0 | RestoreBuffIcon (buff HUD) | `0f b6 4d ef 03 4d c4` | **YES** | 2 whole insn, 7 B |
| 6 | 0x00745AEC | **0x005170EC** | RowDraw 0x007457E0 -> 0x00516DE0 | DrawBuffColumn (buff HUD) | `8d 55 d0 52 8b 45 e0` | **YES** | 3 whole insn, 7 B |
| 7 | 0x00744EEC | **0x005164EC** | TooltipText 0x00744C20 -> 0x00516220 | AppendShieldToTooltip (shield bar) | `8b 4d f4 64 89 0d 00 00 00 00` | **YES** | 2 whole insn, 10 B |

Steam `original_bytes` were checked against the Steam dump first (all 7 equal, whole-instruction windows). The Steam->GOG shift differs per hook host: UpdatePortraits -0x22E880, the DrawEffects/DrawStack/UpdateToolTipText region -0x22EA00 (a plain section shift would land mid-function).

Hook parameter sources (each defining instruction quoted, Steam vs GOG):

**Hook 1, Steam 0x0074F25A -> GOG 0x005209DA** (PS_SnapshotPortraitSlot (portrait-slots)). GOG window: `005209da [89 85 44 ff ff ff] MOV dword ptr [EBP + 0xffffff44],EAX`

- eax = slot ptr: S 0074f253 `8d 84 0a 34 1f 00 00 LEA EAX,[EDX + ECX*0x1 + 0x1f34]` / G 005209d3 identical bytes
- [ebp-0x38] = creature (stored here, read at 0x74f513 `ADD EAX,0xffc`): S 0074ef41 `89 4d c8 MOV dword ptr [EBP + -0x38],ECX` / G 005206c1 identical bytes

**Hook 2, Steam 0x0074F8AB -> GOG 0x0052102B** (PS_SnapshotPortraitSlot (mirror slot)). GOG window: `0052102b [89 95 a4 fe ff ff] MOV dword ptr [EBP + 0xfffffea4],EDX`

- edx = mirror slot ptr (`ADD EDX,0x48e0`): S 0074f8a5 `81 c2 e0 48 00 00 ADD EDX,0x48e0` / G 00521025 identical bytes
- [ebp-0x38] = creature: S 0074ef41 `89 4d c8 MOV dword ptr [EBP + -0x38],ECX` / G 005206c1 identical bytes

**Hook 3, Steam 0x00745774 -> GOG 0x00516D74** (PS_PortraitSlotDraw (portrait-slots)). GOG window: `00516d74 [89 4d fc] MOV dword ptr [EBP + -0x4],ECX`; `00516d77 [8b 45 fc] MOV EAX,dword ptr [EBP + -0x4]`

- ecx = slot (this), saved by the hooked insn itself: S 00745774 `89 4d fc MOV dword ptr [EBP + -0x4],ECX` / G 00516d74 identical bytes
- [ebp+0xc] = isFirst (read as 2nd arg): S 00745797 `8b 4d 0c MOV ECX,dword ptr [EBP + 0xc]` / G 00516d97 identical bytes
- [ebp] = caller frame (prologue PUSH EBP/MOV EBP,ESP): S 00745770 `55 PUSH EBP` / G 00516d70 identical bytes

**Hook 4, Steam 0x00745A84 -> GOG 0x00517084** (RetargetBuffIcon (buff HUD)). GOG window: `00517084 [8b 4d e0] MOV ECX,dword ptr [EBP + -0x20]`; `00517087 [8b 11] MOV EDX,dword ptr [ECX]`; `00517089 [8b 4d e0] MOV ECX,dword ptr [EBP + -0x20]`; `0051708c [8b 42 38] MOV EAX,dword ptr [EDX + 0x38]`; `0051708f [ff d0] CALL EAX`

- [ebp-0x4c] = slot ptr: S 007457e9 `89 4d b4 MOV dword ptr [EBP + -0x4c],ECX` / G 00516de9 identical bytes
- [ebp-0x18] = arrow index (loop counter): S 00745a4d `c7 45 e8 00 00 00 00 MOV dword ptr [EBP + -0x18],0x0` / G 0051704d identical bytes
- [ebp-0x20] = icon widget: S 007458d2 `89 45 e0 MOV dword ptr [EBP + -0x20],EAX` / G 00516ed2 identical bytes
- [ebp+0x10] = whichRow: S 0074583d `83 7d 10 00 CMP dword ptr [EBP + 0x10],0x0` / G 00516e3d identical bytes

**Hook 5, Steam 0x00745A97 -> GOG 0x00517097** (RestoreBuffIcon (buff HUD)). GOG window: `00517097 [0f b6 4d ef] MOVZX ECX,byte ptr [EBP + -0x11]`; `0051709b [03 4d c4] ADD ECX,dword ptr [EBP + -0x3c]`

- no hook parameters

**Hook 6, Steam 0x00745AEC -> GOG 0x005170EC** (DrawBuffColumn (buff HUD)). GOG window: `005170ec [8d 55 d0] LEA EDX,[EBP + -0x30]`; `005170ef [52] PUSH EDX`; `005170f0 [8b 45 e0] MOV EAX,dword ptr [EBP + -0x20]`

- [ebp-0x4c] slot / [ebp-0x20] widget (as hook 4): S 007457e9 `89 4d b4 MOV dword ptr [EBP + -0x4c],ECX` / G 00516de9 identical bytes
- [ebp+0xc] = isLarge: S 00745863 `83 7d 0c 00 CMP dword ptr [EBP + 0xc],0x0` / G 00516e63 identical bytes
- [ebp+0x10] = whichRow: S 0074583d `83 7d 10 00 CMP dword ptr [EBP + 0x10],0x0` / G 00516e3d identical bytes
- [ebp-0x8] = card w (`[EAX+0x8]` of rect 0x54c): S 00745802 `89 4d f8 MOV dword ptr [EBP + -0x8],ECX` / G 00516e02 identical bytes
- [ebp-0x4] = card h (`[EAX+0xc]`): S 00745808 `89 55 fc MOV dword ptr [EBP + -0x4],EDX` / G 00516e08 identical bytes

**Hook 7, Steam 0x00744EEC -> GOG 0x005164EC** (AppendShieldToTooltip (shield bar)). GOG window: `005164ec [8b 4d f4] MOV ECX,dword ptr [EBP + -0xc]`; `005164ef [64 89 0d 00 00 00 00] MOV dword ptr FS:[0x0],ECX`

- [ebp-0x48] = charButton (this): S 00744c45 `89 4d b8 MOV dword ptr [EBP + -0x48],ECX` / G 00516245 identical bytes

Because each host function is 90-100% byte-identical (UpdatePortraits 594/640 identical + 46 address-only; RowDraw 250/258 + 8; SlotDrawGate 36/36; TooltipText 166/204 + 38), every `[ebp+-x]` slot has the same meaning in GOG.

## 2. Mapping tables

### Engine functions

| symbol / use | K2AddrTable field | Steam | GOG | evidence | conf | used by |
|---|---|---|---|---|---|---|
| UpdatePortraits (hook host) | - | 0x0074EE90 | 0x00520610 | body 0074ee90->00520610, 640/640 insn, 594 byte-identical, 46 differ only in address operands, 0 other, size 2732/2732 B<br>xrefs-to S 1 / G 1 (equal)<br>sole caller on both sides: HUD Draw 0x74B3F0 (3874 B, RET 4) <-> 0x51CB70 (3874 B, RET 4); both parents also call SlotDrawGate and SlotShow (xref lists in dumps/*_r4.txt)<br>GOG address DB names 0x520610: CSWGuiMainInterface::UpdatePortraits | exact | PS |
| FUN_00745770 per-slot gate = CSWGuiMainInterfaceChar::DrawEffects | - | 0x00745770 | 0x00516D70 | body 00745770->00516d70, 36/36 insn, 36 byte-identical, 0 differ only in address operands, 0 other, size 99/99 B<br>xrefs-to S 3 / G 3 (equal)<br>call in InGameMenuDraw: S 00757304 [e8 67 e4 fe ff] CALL 0x00745770 / G 00528a04 [e8 67 e3 fe ff] CALL 0x00516d70<br>GOG address DB names 0x516D70: CSWGuiMainInterfaceChar::DrawEffects | exact | PS |
| FUN_007457e0 row draw = CSWGuiMainInterfaceChar::DrawStack | - | 0x007457E0 | 0x00516DE0 | body 007457e0->00516de0, 258/258 insn, 250 byte-identical, 8 differ only in address operands, 0 other, size 813/813 B<br>xrefs-to S 2 / G 2 (equal)<br>call in SlotDrawGate: S 007457a5 [e8 36 00 00 00] CALL 0x007457e0 / G 00516da5 [e8 36 00 00 00] CALL 0x00516de0<br>GOG address DB names 0x516DE0: CSWGuiMainInterfaceChar::DrawStack | exact | BH |
| FUN_00744c20 tooltip text = CSWGuiCharButton::UpdateToolTipText | - | 0x00744C20 | 0x00516220 | body 00744c20->00516220, 204/204 insn, 166 byte-identical, 38 differ only in address operands, 0 other, size 731/731 B<br>xrefs-to S 2 / G 2 (equal)<br>call in DisplayToolTip: S 00744b5b [e8 c0 00 00 00] CALL 0x00744c20 / G 0051615b [e8 c0 00 00 00] CALL 0x00516220<br>GOG address DB names 0x516220: CSWGuiCharButton::UpdateToolTipText | exact | SB |
| FUN_00744f00 = CSWGuiMainInterfaceChar::Show | - | 0x00744F00 | 0x00516500 | body 00744f00->00516500, 147/147 insn, 147 byte-identical, 0 differ only in address operands, 0 other, size 468/468 B<br>xrefs-to S 5 / G 5 (equal)<br>call in UpdatePortraits: S 0074f5e2 [e8 19 59 ff ff] CALL 0x00744f00 / G 00520d62 [e8 99 57 ff ff] CALL 0x00516500<br>GOG address DB names 0x516500: CSWGuiMainInterfaceChar::Show | exact | BH (comment-level) |
| FUN_00756b70 = CSWGuiInGameMenu::Draw (evidence host for Menu_* offsets) | - | 0x00756B70 | 0x00528270 | body 00756b70->00528270, 940/940 insn, 856 byte-identical, 84 differ only in address operands, 0 other, size 3802/3802 B<br>xrefs-to S 1 / G 1 (equal)<br>size/retn 3802/4 equal; the Steam name-map entry was low confidence, now corroborated by identical body + equal callers<br>GOG address DB names 0x528270: CSWGuiInGameMenu::Draw | high | PS (offset evidence only) |
| CExoString::Format (sprintf into CExoString) | `CExoStringFormat` | 0x00734270 | 0x00606380 | body 00734270->00606380, 115/115 insn, 91 byte-identical, 24 differ only in address operands, 0 other, size 380/380 B<br>xrefs-to S >=200 / G >=200 (equal)<br>call in TooltipText: S 00744d96 [e8 d5 f4 fe ff] CALL 0x00734270 / G 00516396 [e8 e5 ff 0e 00] CALL 0x00606380<br>GOG address DB names 0x606380: CExoString::Format | exact | SB |
| CSWSCreature::GetMaxForcePoints | `GetMaxForcePoints` | 0x0057ECA0 | 0x006977D0 | body 0057eca0->006977d0, 384/384 insn, 364 byte-identical, 20 differ only in address operands, 0 other, size 1646/1646 B<br>xrefs-to S 36 / G 36 (equal)<br>call in UpdatePortraits: S 0074f1f5 [e8 a6 fa e2 ff] CALL 0x0057eca0 / G 00520975 [e8 56 6e 17 00] CALL 0x006977d0<br>GOG name_map: CSWSCreature::GetMaxForcePoints (gog_cluster_delta_map vote, delta +0x118B30) now confirmed by body + aligned call<br>GOG address DB names 0x6977D0: CSWSCreature::GetMaxForcePoints | exact | SB |
| CWorldTimer::GetWorldTime | `WorldTimerGetCurrentTime` | 0x0051AD20 | 0x0080B200 | body 0051ad20->0080b200, 102/102 insn, 99 byte-identical, 3 differ only in address operands, 0 other, size 268/268 B<br>xrefs-to S 108 / G 108 (equal)<br>call in GetCanClick: S 007d0bbd [e8 5e a1 d4 ff] CALL 0x0051ad20 / G 004e12bd [e8 3e 9f 32 00] CALL 0x0080b200<br>steam_validated row CWorldTimer::GetWorldTime -> GOG 0x0080b200 (auto:c2, B0)<br>GOG address DB names 0x80B200: CWorldTimer::GetWorldTime | exact | BH,SB |
| CClientExoApp::GetSWParty (party table getter) | `ClientPartyTable` | 0x0073FB90 | 0x0040D070 | body 0073fb90->0040d070, 12/12 insn, 12 byte-identical, 0 differ only in address operands, 0 other, size 31/31 B<br>xrefs-to S >=200 / G >=200 (equal)<br>call in UpdatePortraits: S 0074eed9 [e8 b2 0c ff ff] CALL 0x0073fb90 / G 00520659 [e8 12 ca ee ff] CALL 0x0040d070<br>S 0073fb9f `8b 91 70 02 00 00 MOV EDX,dword ptr [ECX + 0x270]` / G 0040d07f identical bytes<br>GOG address DB names 0x40D070: CClientExoApp::GetSWParty | exact | PS,SB (Party.h) |
| CSWParty::GetPlayerCharacter (controlled creature, party slot 0) | `GetControlledCreature` | 0x007E5D80 | 0x004789B0 | body 007e5d80->004789b0, 10/10 insn, 10 byte-identical, 0 differ only in address operands, 0 other, size 21/21 B<br>xrefs-to S 78 / G 78 (equal)<br>call in TargetMenuSound: S 007442ad [e8 ce 1a 0a 00] CALL 0x007e5d80 / G 005158ad [e8 fe 30 f6 ff] CALL 0x004789b0<br>GOG name: CSWParty::GetPlayerCharacter<br>GOG address DB names 0x4789B0: CSWParty::GetPlayerCharacter | exact | Party.h |
| CSWParty::GetCharacter(idx) (party member accessor) | `PartyGetAt` | 0x007E5DA0 | 0x004789D0 | body 007e5da0->004789d0, 27/27 insn, 25 byte-identical, 2 differ only in address operands, 0 other, size 78/78 B<br>xrefs-to S 127 / G 127 (equal)<br>call in UpdatePortraits: S 0074ef13 [e8 88 6e 09 00] CALL 0x007e5da0 / G 00520693 [e8 38 83 f5 ff] CALL 0x004789d0<br>GOG address DB names 0x4789D0: CSWParty::GetCharacter | exact | PS,SB (Party.h) |
| CSWCCreature::GetServerCreature | `MemberServerCreature` | 0x0077D800 | 0x00429780 | body 0077d800->00429780, 21/21 insn, 19 byte-identical, 2 differ only in address operands, 0 other, size 53/53 B<br>xrefs-to S >=200 / G >=200 (equal)<br>call in UpdatePortraits: S 0074ef24 [e8 d7 e8 02 00] CALL 0x0077d800 / G 005206a4 [e8 d7 90 f0 ff] CALL 0x00429780<br>call in InGameMenuDraw: S 00756c53 [e8 a8 6b 02 00] CALL 0x0077d800 / G 00528353 [e8 28 14 f0 ff] CALL 0x00429780<br>GOG address DB names 0x429780: CSWCCreature::GetServerCreature | exact | PS,SB (Party.h) |
| AurGUIStartLayer (BeginGui2D) | `BeginGui2D` | 0x004792C0 | 0x008CFBD0 | body 004792c0->008cfbd0, 80/80 insn, 54 byte-identical, 26 differ only in address operands, 0 other, size 301/301 B<br>xrefs-to S 10 / G 10 (equal)<br>call in RowDraw: S 0074580b [e8 b0 3a d3 ff] CALL 0x004792c0 / G 00516e0b [e8 c0 8d 3b 00] CALL 0x008cfbd0<br>GOG address DB names 0x8CFBD0: Global::AurGUIStartLayer | exact | BH,SB,PS |
| AurGUIStopLayer (EndGui2D) | `EndGui2D` | 0x004793F0 | 0x008CFD00 | body 004793f0->008cfd00, 27/27 insn, 17 byte-identical, 10 differ only in address operands, 0 other, size 101/101 B<br>xrefs-to S 10 / G 10 (equal)<br>call in RowDraw: S 00745b02 [e8 e9 38 d3 ff] CALL 0x004793f0 / G 00517102 [e8 f9 8b 3b 00] CALL 0x008cfd00<br>GOG address DB names 0x8CFD00: Global::AurGUIStopLayer | exact | BH,SB,PS |
| AurGUISetupViewport (PushClipRect) | `PushClipRect` | 0x00479460 | 0x008CFD70 | body 00479460->008cfd70, 223/223 insn, 169 byte-identical, 54 differ only in address operands, 0 other, size 845/845 B<br>xrefs-to S 13 / G 13 (equal)<br>call in RowDraw: S 0074582d [e8 2e 3c d3 ff] CALL 0x00479460 / G 00516e2d [e8 3e 8f 3b 00] CALL 0x008cfd70<br>GOG address DB names 0x8CFD70: Global::AurGUISetupViewport | exact | BH,SB,PS |
| AurGUICloseViewport (PopClipRect) | `PopClipRect` | 0x004799C0 | 0x008D02D0 | body 004799c0->008d02d0, 59/59 insn, 38 byte-identical, 21 differ only in address operands, 0 other, size 235/235 B<br>xrefs-to S 13 / G 13 (equal)<br>call in RowDraw: S 00745afd [e8 be 3e d3 ff] CALL 0x004799c0 / G 005170fd [e8 ce 91 3b 00] CALL 0x008d02d0<br>GOG address DB names 0x8D02D0: Global::AurGUICloseViewport | exact | BH,SB,PS |
| GetResolutionTextScalar (HudScale) | `HudScale` | 0x00479AB0 | 0x008D03C0 | body 00479ab0->008d03c0, 28/28 insn, 24 byte-identical, 4 differ only in address operands, 0 other, size 92/92 B<br>xrefs-to S 46 / G 46 (equal)<br>call in RowDraw: S 0074593a [e8 71 41 d3 ff] CALL 0x00479ab0 / G 00516f3a [e8 81 94 3b 00] CALL 0x008d03c0<br>GOG address DB names 0x8D03C0: Global::GetResolutionTextScalar | exact | BH,SB |
| CAurGUIImage::FlushBuffer (FlushGuiBatch) | `FlushGuiBatch` | 0x0047EA60 | 0x008D5370 | body 0047ea60->008d5370, 67/67 insn, 46 byte-identical, 21 differ only in address operands, 0 other, size 242/242 B<br>xrefs-to S 6 / G 6 (equal)<br>call in BorderDraw: S 00415940 [e8 1b 91 06 00] CALL 0x0047ea60 / G 00913d10 [e8 5b 16 fc ff] CALL 0x008d5370<br>GOG address DB names 0x8D5370: CAurGUIImage::FlushBuffer | exact | BH,SB |
| NewCAurGUIImage (GuiLoadImage) | `GuiLoadImage` | 0x0047EB60 | 0x008D5470 | body 0047eb60->008d5470, 58/58 insn, 54 byte-identical, 4 differ only in address operands, 0 other, size 197/197 B<br>xrefs-to S 7 / G 7 (equal)<br>call in SetFillImage: S 004148fa [e8 61 a2 06 00] CALL 0x0047eb60 / G 00912cca [e8 a1 27 fc ff] CALL 0x008d5470<br>same call in SetCornerImage/SetEdgeImage (3 callers aligned)<br>GOG address DB names 0x8D5470: Global::NewCAurGUIImage | exact | BH,SB |
| CSWGuiPanel::GetExtentAccountingForPanelOffset (PanelRect) | `PanelRect` | 0x00410020 | 0x0090E4A0 | body 00410020->0090e4a0, 58/58 insn, 58 byte-identical, 0 differ only in address operands, 0 other, size 156/156 B<br>xrefs-to S 5 / G 5 (equal)<br>call in PanelDraw: S 0040ed6d [e8 ae 12 00 00] CALL 0x00410020 / G 0090d1ed [e8 ae 12 00 00] CALL 0x0090e4a0<br>steam_validated row CSWGuiPanel::GetExtentAccountingForPanelOffset -> GOG 0x0090e4a0 (auto:c2, B0)<br>GOG address DB names 0x90E4A0: CSWGuiPanel::GetExtentAccountingForPanelOffset | exact | PS,SB |
| CSWGuiManager::PlayGuiSound | `PlayGuiSound` | 0x004122A0 | 0x00910720 | body 004122a0->00910720, 29/29 insn, 28 byte-identical, 1 differ only in address operands, 0 other, size 94/94 B<br>xrefs-to S >=200 / G >=200 (equal)<br>call in TargetMenuSound: S 007443f8 [e8 a3 de cc ff] CALL 0x004122a0 / G 005159f8 [e8 23 ad 3f 00] CALL 0x00910720<br>steam_validated row CSWGuiManager::PlayGuiSound -> GOG 0x00910720 (auto:c2, B0)<br>GOG address DB names 0x910720: CSWGuiManager::PlayGuiSound | exact | hotbar/GuiKit only |
| CSWGuiLabel::CSWGuiLabel | `LabelCtor` | 0x00419740 | 0x009179E0 | body 00419740->009179e0, 38/38 insn, 33 byte-identical, 5 differ only in address operands, 0 other, size 131/131 B<br>xrefs-to S >=200 / G >=200 (equal)<br>call in InGameMenuCtor: S 00754f35 [e8 06 48 cc ff] CALL 0x00419740 / G 00526635 [e8 a6 13 3f 00] CALL 0x009179e0<br>GOG address DB names 0x9179E0: CSWGuiLabel::Constructor | exact | SB,BH (GuiKit) |
| CSWGuiPanel::InitControl | `InitControl` | 0x0040F620 | 0x0090DAA0 | body 0040f620->0090daa0, 273/273 insn, 243 byte-identical, 30 differ only in address operands, 0 other, size 970/970 B<br>xrefs-to S >=200 / G >=200 (equal)<br>call in CharInitialize: S 00745140 [e8 db a4 cc ff] CALL 0x0040f620 / G 00516740 [e8 5b 73 3f 00] CALL 0x0090daa0<br>steam_validated row CSWGuiPanel::InitControl -> GOG 0x0090daa0 (auto:c2, B0)<br>GOG address DB names 0x90DAA0: CSWGuiPanel::InitControl | exact | GuiKit |
| CSWGuiPanel::StopLoadFromLayout | `StopLoadFromLayout` | 0x0040F5A0 | 0x0090DA20 | body 0040f5a0->0090da20, 40/40 insn, 39 byte-identical, 1 differ only in address operands, 0 other, size 117/117 B<br>xrefs-to S 73 / G 73 (equal)<br>call in PanelDtor: S 0040e8dd [e8 be 0c 00 00] CALL 0x0040f5a0 / G 0090cd5d [e8 be 0c 00 00] CALL 0x0090da20<br>steam_validated row CSWGuiPanel::StopLoadFromLayout -> GOG 0x0090da20 (auto:c2, B0)<br>GOG address DB names 0x90DA20: CSWGuiPanel::StopLoadFromLayout | exact | GuiKit |
| CExoString ctor (const char*) | `ExoCtor` | 0x00733570 | 0x00605680 | body 00733570->00605680, 44/44 insn, 40 byte-identical, 4 differ only in address operands, 0 other, size 126/126 B<br>xrefs-to S >=200 / G >=200 (equal)<br>call in InGameMenuDraw: S 0075703f [e8 2c c5 fd ff] CALL 0x00733570 / G 0052873f [e8 3c cf 0d 00] CALL 0x00605680<br>steam_validated row CExoString::CStrConstructor -> GOG 0x00605680 (opus, B0)<br>GOG address DB names 0x605680: CExoString::CStrConstructor, CExoString::Constructor_2 | exact | GuiKit |
| CExoString dtor | `ExoDtor` | 0x00733780 | 0x00605890 | body 00733780->00605890, 21/21 insn, 20 byte-identical, 1 differ only in address operands, 0 other, size 60/60 B<br>xrefs-to S >=200 / G >=200 (equal)<br>call in TooltipText: S 00744da5 [e8 d6 e9 fe ff] CALL 0x00733780 / G 005163a5 [e8 e6 f4 0e 00] CALL 0x00605890<br>steam_validated row CExoString::Destructor -> GOG 0x00605890 (opus, B0)<br>GOG address DB names 0x605890: CExoString::Destructor | exact | GuiKit |
| CSWGuiTextParams::SetText | `TextSet` | 0x00416E30 | 0x00915200 | body 00416e30->00915200, 36/36 insn, 34 byte-identical, 2 differ only in address operands, 0 other, size 103/103 B<br>xrefs-to S >=200 / G >=200 (equal)<br>call in InGameMenuDraw: S 00756f10 [e8 1b ff cb ff] CALL 0x00416e30 / G 00528610 [e8 eb cb 3e 00] CALL 0x00915200<br>GOG address DB names 0x915200: CSWGuiTextParams::SetText | exact | GuiKit |
| CSWGuiTextParams::SetBaseFont (SetFont) | `SetFont` | 0x00416DE0 | 0x009151B0 | body 00416de0->009151b0, 14/14 insn, 13 byte-identical, 1 differ only in address operands, 0 other, size 36/36 B<br>xrefs-to S 6 / G 6 (equal)<br>call in TextLoad: S 00417506 [e8 d5 f8 ff ff] CALL 0x00416de0 / G 009158d6 [e8 d5 f8 ff ff] CALL 0x009151b0<br>GOG address DB names 0x9151B0: CSWGuiTextParams::SetBaseFont | exact | GuiKit |
| CSWGuiTextParams::SetAlignment (SetAlign) | `SetAlign` | 0x00416FA0 | 0x00915370 | body 00416fa0->00915370, 38/38 insn, 38 byte-identical, 0 differ only in address operands, 0 other, size 103/103 B<br>xrefs-to S 13 / G 13 (equal)<br>call in TextLoad: S 00417604 [e8 97 f9 ff ff] CALL 0x00416fa0 / G 009159d4 [e8 97 f9 ff ff] CALL 0x00915370<br>GOG address DB names 0x915370: CSWGuiTextParams::SetAlignment | exact | GuiKit |
| CSWGuiTextParams::SetDefaultColor (TextColor) | `TextColor` | 0x00417140 | 0x00915510 | body 00417140->00915510, 20/20 insn, 20 byte-identical, 0 differ only in address operands, 0 other, size 50/50 B<br>xrefs-to S 113 / G 113 (equal)<br>call in TextLoad: S 00417541 [e8 fa fb ff ff] CALL 0x00417140 / G 00915911 [e8 fa fb ff ff] CALL 0x00915510<br>DECOY rejected: GOG 0x9153e0 CSWGuiTextParams::SetColor (93 B, the name-based guess) matches only 12 of 36 insns; the delta-sequence target 0x915510 (50 B, SetDefaultColor) is byte-identical 20/20<br>GOG address DB names 0x915510: CSWGuiTextParams::SetDefaultColor | exact | GuiKit |
| CExoArrayList<CSWGuiControl*>::Add (PtrListAdd) | `PtrListAdd` | 0x0083EA60 | 0x00534170 | body 0083ea60->00534170, 36/36 insn, 35 byte-identical, 1 differ only in address operands, 0 other, size 102/102 B<br>xrefs-to S >=200 / G 65 (DIFFER)<br>call in InitControl: S 0040f9cb [e8 90 f0 42 00] CALL 0x0083ea60 / G 0090de4b [e8 20 63 c2 ff] CALL 0x00534170<br>CAVEAT: Steam 0x83ea60 is one folded instance with >=200 callers; GOG 0x534170 has 65 (separate per-type copies). Correct for the panel control list (InitControl uses it); a GOG caller adding to some OTHER pointer list must use that list own Add. GuiKit only appends to a panel list.<br>GOG address DB names 0x534170: CExoArrayList::Add_2 | exact | GuiKit |
| operator new (cdecl size) | `New` | 0x00919723 | 0x00921347 | body 00919723->00921347, 33/34 insn, 23 byte-identical, 10 differ only in address operands, 0 other, size 100/101 B<br>xrefs-to S >=200 / G >=200 (equal)<br>call in LoadImage: S 0047eb8d [e8 91 ab 49 00] CALL 0x00919723 / G 008d549d [e8 a5 be 04 00] CALL 0x00921347<br>GOG body is 101 B vs 100 B: the extra byte is a trailing INT3 pad in the Ghidra function body; instruction streams equal 33/33 + pad<br>GOG address DB names 0x921347: Global::operator_new_uint | exact | GuiKit |
| (not in table) CSWSObject::HasSpellEffectApplied | - | 0x005476C0 | 0x006AE3A0 | body 005476c0->006ae3a0, 34/34 insn, 34 byte-identical, 0 differ only in address operands, 0 other, size 101/101 B<br>evidence host for Creature_EffectArray/Count<br>GOG address DB names 0x6AE3A0: CSWSObject::HasSpellEffectApplied | high |  |

### Globals

| symbol / use | K2AddrTable field | Steam | GOG | evidence | conf | used by |
|---|---|---|---|---|---|---|
| APP_MANAGER_PTR (CAppManager*) | `AppGlobal` | 0x00A1B4A4 | 0x00A11C04 | operand in PartyGetAt: S 007e5dcc [8b 15 a4 b4 a1 00] MOV EDX,dword ptr [0x00a1b4a4] / G 004789fc [8b 15 04 1c a1 00] MOV EDX,dword ptr [0x00a11c04]<br>25 aligned reads across the dumped pairs, all map A1B4A4 -> A11C04<br>GOG address DB global_pointers: APP_MANAGER_PTR = 0xA11C04 (agrees) | exact | BH,SB,PS (Party.h, Effects.h) |
| GUI_MANAGER_PTR | `GuiSoundMgr` | 0x00A1B49C | 0x00A11BFC | operand in DisplayToolTip: S 00744b6a [8b 0d 9c b4 a1 00] MOV ECX,dword ptr [0x00a1b49c] / G 0051616a [8b 0d fc 1b a1 00] MOV ECX,dword ptr [0x00a11bfc]<br>GOG address DB: GUI_MANAGER_PTR = 0xA11BFC (agrees)<br>also read in CharButtonDraw, TooltipPanelPos | exact | GuiKit (hover sound) |
| screen width | `ScreenW` | 0x009F42A4 | 0x009F22A4 | operand in HudScale: S 00479ab6 [a1 a4 42 9f 00] MOV EAX,[0x009f42a4] / G 008d03c6 [a1 a4 22 9f 00] MOV EAX,[0x009f22a4]<br>also aligned in PushClip, TooltipPanelPos (HudScale derives the HUD scale from it) | exact | GuiKit/Gui2D |
| screen height | `ScreenH` | 0x009F42A8 | 0x009F22A8 | operand in PushClip: S 004794a0 [3b 05 a8 42 9f 00] CMP EAX,dword ptr [0x009f42a8] / G 008cfdb0 [3b 05 a8 22 9f 00] CMP EAX,dword ptr [0x009f22a8]<br>also aligned in TooltipPanelPos | exact | GuiKit/Gui2D |
| GUI ref width (800) | `GuiRefW` | 0x009F4390 | 0x00A0FFE8 | operand in InitControl: S 0040f6e9 [8b 15 90 43 9f 00] MOV EDX,dword ptr [0x009f4390] / G 0090db69 [8b 15 e8 ff a0 00] MOV EDX,dword ptr [0x00a0ffe8]<br>initialised data equal: Steam 0x9F4390 = 20 03 00 00 (800), GOG 0xA0FFE8 = 20 03 00 00 | exact | hotbar only |
| GUI ref height (600) | `GuiRefH` | 0x009F4394 | 0x00A0FFEC | operand in InitControl: S 0040f733 [8b 0d 94 43 9f 00] MOV ECX,dword ptr [0x009f4394] / G 0090dbb3 [8b 0d ec ff a0 00] MOV ECX,dword ptr [0x00a0ffec]<br>initialised data equal: Steam 0x9F4394 = 58 02 00 00 (600), GOG 0xA0FFEC = 58 02 00 00 | exact | hotbar only |
| GUI projection flag | `ProjectionFlag` | 0x00A1B9F0 | 0x00A76858 | operand in ScaleHelperA: S 0040fa49 [83 3d f0 b9 a1 00 02] CMP dword ptr [0x00a1b9f0],0x2 / G 0090dec9 [83 3d 58 68 a7 00 02] CMP dword ptr [0x00a76858],0x2<br>same operand aligned in InitControl and ScaleHelperB (3 functions); Steam holds 0 here, the GOG address is .bss (not file-backed), consistent | exact | hotbar only |
| FlushArg float (1.0f) | `FlushArg` | 0x009F5F60 | 0x009F7144 | operand in BorderDraw: S 00415937 [d9 05 60 5f 9f 00] FLD float ptr [0x009f5f60] / G 00913d07 [d9 05 44 71 9f 00] FLD float ptr [0x009f7144]<br>initialised data equal: Steam 0x9F5F60 = 00 00 80 3f (1.0f), GOG 0x9F7144 = 00 00 80 3f | exact | BH,SB |
| clip stack index | `ClipDepth` | 0x00A309C0 | 0x00A32B90 | operand in BeginGui2D: S 00479313 [66 a3 c0 09 a3 00] MOV [0x00a309c0],AX / G 008cfc23 [66 a3 90 2b a3 00] MOV [0x00a32b90],AX<br>20 aligned operand sites in Begin/End/Push/Pop (A309C0 -> A32B90, +0x21D0 like the neighbouring A309C8.. fields) | exact | PS,SB,BH |
| clip no-fill colour (engine icon row arg) | `ClipNoFillColor` | 0x00A1BA3C | 0x00A768A4 | operand in RowDraw: S 00745818 [68 3c ba a1 00] PUSH 0xa1ba3c / G 00516e18 [68 a4 68 a7 00] PUSH 0xa768a4<br>single aligned site (PUSH imm32 into PushClip); neighbour global A1B9F0->A76858 has the identical +0x5AE68 shift (A1BA3C->A768A4), consistent<br>Steam image holds zeros here (uninitialised data); the GOG address is not file-backed (.bss), consistent | high | BH,SB |
| pending texture list | `TexPendingList` | 0x00A1BC2C | 0x00A14CD4 | operand in TexResetAll: S 00427d99 [b9 2c bc a1 00] MOV ECX,0xa1bc2c / G 0081a499 [b9 d4 4c a1 00] MOV ECX,0xa14cd4<br>same operand aligned in TexSetMem and TexBuildAll (3 functions) | high | hotbar only (not the 3 mods) |
| keyboard device index (rdata const 0) | `KeyboardDeviceGlobal` | 0x0099C884 | unknown | not mapped: hotbar-only global, none of the 3 mods read it; the Steam users (0x785df0, 0x732960..) have no GOG pair in the dumps (autopair found no unique same-size GOG function) | unknown | hotbar only |

### Vtables

| symbol / use | K2AddrTable field | Steam | GOG | evidence | conf | used by |
|---|---|---|---|---|---|---|
| CSWGuiLabel vtable | `LabelVtable` | 0x009878BC | 0x0098998C | operand in LabelCtor: S 00419778 [c7 00 bc 78 98 00] MOV dword ptr [EAX],0x9878bc / G 00917a18 [c7 00 8c 99 98 00] MOV dword ptr [EAX],0x98998c<br>PTRS dump 40 slots each: slot1 SetRect 0x4199a0 <-> SetExtent 0x917c40, slot14 Draw 0x419880 <-> 0x917b20 (LabelDraw, body-identical 25/25), slot17 HitTest 0x418d20 <-> 0x916fc0, slot18 Load, slots 31-39 all +0x4FE2A0; shared stubs map consistently<br>GOG ctor also calls the border ctor at this+0x60 and the text ctor at this+0xD8 (same as Steam) | exact | GuiKit |
| CSWGuiBorder vtable | `BorderVtable` | 0x009875BC | 0x009A994C | operand in BorderCtor: S 00414abb [c7 00 bc 75 98 00] MOV dword ptr [EAX],0x9875bc / G 00912e8b [c7 00 4c 99 9a 00] MOV dword ptr [EAX],0x9a994c<br>PTRS: Steam slot3 = 0x415300 (BorderDraw) <-> GOG slot3 = 0x9136D0 (BorderDraw, body-identical 674 insn); slot1 SetRect 0x414B70 <-> SetExtent 0x912F40; slots 9/10 __purecall in both | exact | BH,GuiKit |
| CSWGuiText vtable | `TextVtable` | 0x009876B4 | 0x009A9A28 | operand in TextCtor: S 0041720b [c7 00 b4 76 98 00] MOV dword ptr [EAX],0x9876b4 / G 009155db [c7 00 28 9a 9a 00] MOV dword ptr [EAX],0x9a9a28<br>PTRS: slot3 Draw 0x417720 <-> 0x915AF0 (TextDraw 30/30 aligned), slot4 Load 0x417480 <-> 0x915850 (TextLoad) | exact | GuiKit |

### Struct offsets / layout constants

| symbol / use | K2AddrTable field | Steam | GOG | evidence | conf | used by |
|---|---|---|---|---|---|---|
| portrait slot flags word (bit1 = row visible) | `Slot_Flags` | 0x590 | 0x590 | S 0074577a `8b 88 90 05 00 00 MOV ECX,dword ptr [EAX + 0x590]` / G 00516d7a identical bytes<br>derived: control embedded at slot+0x548 (SlotShow `ADD EAX,0x548`) + Ctl_Flags 0x48 = 0x590 | exact | BH,SB |
| portrait card rect (x,y,w,h) | `Slot_CardRect` | 0x54C | 0x54C | S 007457ef `05 4c 05 00 00 ADD EAX,0x54c` / G 00516def identical bytes<br>S 00744f0c `05 48 05 00 00 ADD EAX,0x548` / G 0051650c identical bytes (control base 0x548, rect = +4) | exact | SB |
| embedded health-bar control | `Slot_VitCtl` | 0x690 | 0x690 | S 00744f52 `81 c1 90 06 00 00 ADD ECX,0x690` / G 00516552 identical bytes | exact | SB |
| health-bar rect | `Slot_VitRect` | 0x694 | 0x694 | derived = Slot_VitCtl + Ctl_X(4): both exact; the rect lives at control+4 (HitTest reads +4..+0x10) | exact | SB |
| embedded force-bar control | `Slot_ForceCtl` | 0x7FC | 0x7FC | S 00744f77 `05 fc 07 00 00 ADD EAX,0x7fc` / G 00516577 identical bytes | exact | SB |
| force-bar rect | `Slot_ForceRect` | 0x800 | 0x800 | derived = Slot_ForceCtl + 4 | exact | SB |
| buff arrow counter (u8) | `Slot_ArrowBuff` | 0xDDC | 0xDDC | S 0074507f `88 91 dc 0d 00 00 MOV byte ptr [ECX + 0xddc],DL` / G 0051667f identical bytes<br>S 0074578a `0f b6 82 dc 0d 00 00 MOVZX EAX,byte ptr [EDX + 0xddc]` / G 00516d8a identical bytes | exact | BH,SB |
| debuff arrow counter (u8) | `Slot_ArrowDebuff` | 0xDDD | 0xDDD | S 007450a3 `88 82 dd 0d 00 00 MOV byte ptr [EDX + 0xddd],AL` / G 005166a3 identical bytes<br>S 007457ad `0f b6 82 dd 0d 00 00 MOVZX EAX,byte ptr [EDX + 0xddd]` / G 00516dad identical bytes | exact | BH,SB |
| overlay flag word 0 (mask&2) | `Slot_Overlay0` | 0x1B8 | 0x1B8 | S 00744fba `05 70 01 00 00 ADD EAX,0x170` / G 005165ba identical bytes then flags +0x48<br>derived 0x170 + 0x48 = 0x1b8 | exact | BH |
| overlay flag word 1 (mask&1) | `Slot_Overlay1` | 0x300 | 0x300 | S 00744ffd `05 b8 02 00 00 ADD EAX,0x2b8` / G 005165fd identical bytes then flags +0x48<br>derived 0x2b8 + 0x48 = 0x300 | exact | BH |
| overlay flag word 2 (mask&4) | `Slot_Overlay2` | 0x448 | 0x448 | S 00745040 `05 00 04 00 00 ADD EAX,0x400` / G 00516640 identical bytes then flags +0x48<br>derived 0x400 + 0x48 = 0x448 | exact | BH |
| CSWGuiInGameMenu embedded portrait slot | `Menu_PortraitSlot` | 0x2350 | 0x2350 | S 00755242 `81 c1 50 23 00 00 ADD ECX,0x2350` / G 00526942 identical bytes<br>call in InGameMenuCtor: S 007559b7 [e8 24 f7 fe ff] CALL 0x007450e0 / G 005270b7 [e8 24 f6 fe ff] CALL 0x005166e0 (CharInitialize on this+0x2350)<br>S 007572fe `81 c1 50 23 00 00 ADD ECX,0x2350` / G 005289fe identical bytes (Draw passes this+0x2350 as ECX of SlotDrawGate, next call 0x745774-host) | exact | PS |
| CSWGuiInGameMenu +LBL_CHAR2 control | `Menu_Char2` | 0x3134 | 0x3134 | string anchor `LBL_CHAR2`: S 007559bc `PUSH 0x99e384` then 007559e1 `ADD ECX,0x3134` (InitControl follows); G 005270bc `PUSH 0x98df84` then 005270e1 `ADD ECX,0x3134`<br>S 00755257 `81 c1 34 31 00 00 ADD ECX,0x3134` / G 00526957 identical bytes | exact | PS,SB,BH |
| CSWGuiInGameMenu +LBL_LEVELUP2 control | `Menu_Level2` | 0x327C | 0x327C | string anchor `LBL_LEVELUP2`: S 00755a02 `PUSH 0x99e374` then 00755a27 `ADD EAX,0x327c` (InitControl follows); G 00527102 `PUSH 0x98df74` then 00527127 `ADD EAX,0x327c`<br>S 0075526c `81 c1 7c 32 00 00 ADD ECX,0x327c` / G 0052696c identical bytes | exact | PS,SB,BH |
| CSWGuiInGameMenu +LBL_BACK2 control | `Menu_Back2` | 0x33C4 | 0x33C4 | string anchor `LBL_BACK2`: S 00755a47 `PUSH 0x99e368` then 00755a6c `ADD EDX,0x33c4` (InitControl follows); G 00527147 `PUSH 0x98df68` then 0052716c `ADD EDX,0x33c4`<br>S 00755281 `81 c1 c4 33 00 00 ADD ECX,0x33c4` / G 00526981 identical bytes | exact | PS,SB,BH |
| CSWGuiInGameMenu +BTN_CHANGE2 control | `Menu_Change2` | 0x350C | 0x350C | string anchor `BTN_CHANGE2`: S 00755a8d `PUSH 0x99e35c` then 00755ab2 `ADD ECX,0x350c` (InitControl follows); G 0052718d `PUSH 0x98df5c` then 005271b2 `ADD ECX,0x350c`<br>S 00755296 `81 c1 0c 35 00 00 ADD ECX,0x350c` / G 00526996 identical bytes | exact | PS,SB,BH |
| CSWGuiInGameMenu +LBL_CHAR3 control | `Menu_Char3` | 0x36DC | 0x36DC | string anchor `LBL_CHAR3`: S 00755ad3 `PUSH 0x99e350` then 00755af8 `ADD EAX,0x36dc` (InitControl follows); G 005271d3 `PUSH 0x98df50` then 005271f8 `ADD EAX,0x36dc`<br>S 007552ab `81 c1 dc 36 00 00 ADD ECX,0x36dc` / G 005269ab identical bytes | exact | PS,SB,BH |
| CSWGuiInGameMenu +LBL_LEVELUP3 control | `Menu_Level3` | 0x3824 | 0x3824 | string anchor `LBL_LEVELUP3`: S 00755b18 `PUSH 0x99e340` then 00755b3d `ADD EDX,0x3824` (InitControl follows); G 00527218 `PUSH 0x98df40` then 0052723d `ADD EDX,0x3824`<br>S 007552c0 `81 c1 24 38 00 00 ADD ECX,0x3824` / G 005269c0 identical bytes | exact | PS,SB,BH |
| CSWGuiInGameMenu +LBL_BACK3 control | `Menu_Back3` | 0x396C | 0x396C | string anchor `LBL_BACK3`: S 00755b5e `PUSH 0x99e334` then 00755b83 `ADD ECX,0x396c` (InitControl follows); G 0052725e `PUSH 0x98df34` then 00527283 `ADD ECX,0x396c`<br>S 007552d5 `81 c1 6c 39 00 00 ADD ECX,0x396c` / G 005269d5 identical bytes | exact | PS,SB,BH |
| CSWGuiInGameMenu +BTN_CHANGE3 control | `Menu_Change3` | 0x3AB4 | 0x3AB4 | string anchor `BTN_CHANGE3`: S 00755ba4 `PUSH 0x99e328` then 00755bc9 `ADD EAX,0x3ab4` (InitControl follows); G 005272a4 `PUSH 0x98df28` then 005272c9 `ADD EAX,0x3ab4`<br>S 007552ea `81 c1 b4 3a 00 00 ADD ECX,0x3ab4` / G 005269ea identical bytes | exact | PS,SB,BH |
| caller-frame local = selected creature | `MenuCaller_Creature` | -0x38 | -0x38 | S 00756c58 `89 45 c8 MOV dword ptr [EBP + -0x38],EAX` / G 00528358 identical bytes (store right after CALL GetServerCreature)<br>Draw is the caller of SlotDrawGate for the menu; frame identical because whole function is 856/940 byte-identical, 84 address-only | exact | PS |
| creature stats object | `Creature_Stats` | 0x1198 | 0x1198 | S 0074f1ab `8b 88 98 11 00 00 MOV ECX,dword ptr [EAX + 0x1198]` / G 0052092b identical bytes | exact | SB |
| stats current-FP part A (short) | `Stats_ForceA` | 0x130 | 0x130 | S 0074f1c1 `0f bf 88 30 01 00 00 MOVSX ECX,word ptr [EAX + 0x130]` / G 00520941 identical bytes | exact | SB |
| stats current-FP part B (short) | `Stats_ForceB` | 0x132 | 0x132 | S 0074f1e3 `0f b7 91 32 01 00 00 MOVZX EDX,word ptr [ECX + 0x132]` / G 00520963 identical bytes | exact | SB |
| creature vtable max-HP getter | `Creature_VtMaxHp` | 0x98 | 0x98 | S 0074f16d `8b 82 98 00 00 00 MOV EAX,dword ptr [EDX + 0x98]` / G 005208ed identical bytes<br>ret N of the target slot: engine slots 0x57e8c0 (RET 4) <-> GOG 0x6973f0 (RET 4) body-identical 254 insn | exact | SB |
| creature vtable cur-HP getter | `Creature_VtCurHp` | 0x9C | 0x9C | S 0074f183 `8b 90 9c 00 00 00 MOV EDX,dword ptr [EAX + 0x9c]` / G 00520903 identical bytes<br>slot target 0x5413c0 <-> GOG 0x6a80a0 (RET 4) body-identical 16/16 | exact | SB |
| client icon-cache list head | `Creature_IconCacheData` | 0xFFC | 0xFFC | S 0074f516 `05 fc 0f 00 00 ADD EAX,0xffc` / G 00520c96 identical bytes | exact | BH (diagnostic) |
| icon-cache count (= head+4) | `Creature_IconCacheCount` | 0x1000 | 0x1000 | S 0074f533 `8b 42 04 MOV EAX,dword ptr [EDX + 0x4]` / G 00520cb3 identical bytes (count at list+4 = 0xffc+4) | exact | BH (diagnostic) |
| icon entry +0x14 (debuff test) | `IconEnt_F14` | 0x14 | 0x14 | S 0074f563 `83 78 14 00 CMP dword ptr [EAX + 0x14],0x0` / G 00520ce3 identical bytes | exact | BH (diagnostic) |
| icon entry +0x1c (active test) | `IconEnt_F1c` | 0x1C | 0x1C | S 0074f552 `83 7a 1c 00 CMP dword ptr [EDX + 0x1c],0x0` / G 00520cd2 identical bytes | exact | BH (diagnostic) |
| char button -> portrait slot pointer | `Tip_Slot` | 0x1D0 | 0x1D0 | S 00744b2d `89 88 d0 01 00 00 MOV dword ptr [EAX + 0x1d0],ECX` / G 0051612d identical bytes<br>S 00744c5b `8b 91 d0 01 00 00 MOV EDX,dword ptr [ECX + 0x1d0]` / G 0051625b identical bytes | exact | SB |
| char button tooltip CExoString | `Tip_Text` | 0x1D4 | 0x1D4 | S 00744d8f `81 c1 d4 01 00 00 ADD ECX,0x1d4` / G 0051638f identical bytes | exact | SB |
| CSWSObject effect array | `Creature_EffectArray` | 0x148 | 0x148 | S 005476f2 `8b 88 48 01 00 00 MOV ECX,dword ptr [EAX + 0x148]` / G 006ae3d2 identical bytes | exact | BH,SB |
| CSWSObject effect count | `Creature_EffectCount` | 0x14C | 0x14C | S 005476e7 `3b 91 4c 01 00 00 CMP EDX,dword ptr [ECX + 0x14c]` / G 006ae3c7 identical bytes<br>GOG knowledge DB struct_fields lists CSWSObject +0x14C effect count at Steam 0x5476e7 (this site) | exact | BH,SB |
| CGameEffect type (u16) | `Effect_Type` | 0x8 | 0x8 | S 005e49cd `66 89 4a 08 MOV word ptr [EDX + 0x8],CX` / G 0073468d identical bytes | exact | BH,SB |
| CGameEffect subtype (u16) | `Effect_Subtype` | 0xA | 0xA | S 005e49d6 `66 89 41 0a MOV word ptr [ECX + 0xa],AX` / G 00734696 identical bytes | exact | BH,SB |
| CGameEffect duration (float) | `Effect_Duration` | 0xC | 0xC | S 005e49df `d9 5a 0c FSTP float ptr [EDX + 0xc]` / G 0073469f identical bytes | exact | BH,SB |
| CGameEffect expiry day | `Effect_ExpiryDay` | 0x10 | 0x10 | S 005e4fad `8b 51 10 MOV EDX,dword ptr [ECX + 0x10]` / G 00734c6d identical bytes<br>S 005e4f8d `89 48 10 MOV dword ptr [EAX + 0x10],ECX` / G 00734c4d identical bytes | exact | BH,SB |
| CGameEffect expiry ms-of-day | `Effect_ExpiryTime` | 0x14 | 0x14 | S 005e4fb8 `8b 51 14 MOV EDX,dword ptr [ECX + 0x14]` / G 00734c78 identical bytes<br>S 005e4f96 `89 42 14 MOV dword ptr [EDX + 0x14],EAX` / G 00734c56 identical bytes | exact | BH,SB |
| CGameEffect creator id | `Effect_Creator` | 0x18 | 0x18 | S 005e5ec9 `89 48 18 MOV dword ptr [EAX + 0x18],ECX` / G 00735b89 identical bytes<br>S 005e49f9 `c7 42 18 00 00 00 7f MOV dword ptr [EDX + 0x18],0x7f000000` / G 007346b9 identical bytes (invalid-object-id default) | exact | BH,SB |
| CGameEffect spells.2da id | `Effect_SpellId` | 0x1C | 0x1C | S 005e4a21 `c7 40 1c ff ff ff ff MOV dword ptr [EAX + 0x1c],0xffffffff` / G 007346e1 identical bytes (-1 default) | exact | BH,SB |
| CGameEffect int count | `Effect_IntCount` | 0x30 | 0x30 | S 005e4ddf `89 48 30 MOV dword ptr [EAX + 0x30],ECX` / G 00734a9f identical bytes | exact | SB |
| CGameEffect int* array | `Effect_Ints` | 0x34 | 0x34 | S 005e4e6a `8b 48 34 MOV ECX,dword ptr [EAX + 0x34]` / G 00734b2a identical bytes<br>S 005e4e32 `89 48 34 MOV dword ptr [EAX + 0x34],ECX` / G 00734af2 identical bytes | exact | SB |
| CAppManager+8 = server exo app | `AppGlobal_ToWorld` | 0x8 | 0x8 | S 00401beb `83 78 08 00 CMP dword ptr [EAX + 0x8],0x0` / G 00401bab identical bytes (server-exists check)<br>S 00401c3a `89 42 08 MOV dword ptr [EDX + 0x8],EAX` / G 00401bfa identical bytes<br>GOG address DB offsets table: CAppManager.Server = 0x8, Client = 0x4 (agrees) | exact | BH,SB (Effects.h) |
| server app +4 = internal | `World_ToSub` | 0x4 | 0x4 | S 0051c37c `8b 48 04 MOV ECX,dword ptr [EAX + 0x4]` / G 0064bd9c identical bytes | exact | BH,SB |
| internal +0x10048 = world timer | `Sub_ToTimer` | 0x10048 | 0x10048 | S 0051c37f `8b 91 48 00 01 00 MOV EDX,dword ptr [ECX + 0x10048]` / G 0064bd9f identical bytes | exact | BH,SB |
| image wrapper vtable slot of Draw (+0x18) | `ImageVt_Draw` | 0x6 | 0x6 | S 00415a41 `8b 42 18 MOV EAX,dword ptr [EDX + 0x18]` / G 00913e11 identical bytes (slot at +0x18 = 6*4, called on border+0x74 fill image) | exact | BH,SB |
| sizeof(CSWGuiLabel) | `LabelSize` | 0x148 | 0x148 | embedded labels in the menu are 0x148 apart: Menu_Char2 0x3134 -> Menu_Level2 0x327c -> Menu_Back2 0x33c4 (identical in GOG); both ctor sites quoted under Menu_* | exact | GuiKit |
| control flags | `Ctl_Flags` | 0x48 | 0x48 | S 00744f1f `8b 42 48 MOV EAX,dword ptr [EDX + 0x48]` / G 0051651f identical bytes<br>S 00418d2c `8b 48 48 MOV ECX,dword ptr [EAX + 0x48]` / G 00916fcc identical bytes | exact | all |
| control id | `Ctl_Id` | 0x54 | 0x54 | S 0040f96c `8b 48 54 MOV ECX,dword ptr [EAX + 0x54]` / G 0090ddec identical bytes | exact | GuiKit |
| control x | `Ctl_X` | 0x4 | 0x4 | S 00418d4a `3b 51 04 CMP EDX,dword ptr [ECX + 0x4]` / G 00916fea identical bytes | exact | all |
| control y | `Ctl_Y` | 0x8 | 0x8 | S 00418d6e `3b 51 08 CMP EDX,dword ptr [ECX + 0x8]` / G 0091700e identical bytes | exact | all |
| control w | `Ctl_W` | 0xC | 0xC | S 00418d60 `03 50 08 ADD EDX,dword ptr [EAX + 0x8]` / G 00917000 identical bytes (base = ctl+4 from 00418d52 `ADD EAX,0x4`, so w = ctl+0x4+0x8 = 0xc) | exact | all |
| control h | `Ctl_H` | 0x10 | 0x10 | S 00418d85 `03 50 0c ADD EDX,dword ptr [EAX + 0xc]` / G 00917025 identical bytes (base = ctl+4 from 00418d76, so h = ctl+0x4+0xc = 0x10) | exact | all |
| control vtable slot1 byte offset (SetRect) | `Ctl_VtSetRect` | 0x4 | 0x4 | S 00745af8 `8b 42 04 MOV EAX,dword ptr [EDX + 0x4]` / G 005170f8 identical bytes (hook-6 row exit calls vt+4 on the icon widget with a rect)<br>Label vtable PTRS slot1 maps SetRect 0x4199a0 <-> SetExtent 0x917c40 | exact | SB,PS |
| label +border | `Label_Border` | 0x60 | 0x60 | S 00419781 `83 c1 60 ADD ECX,0x60` / G 00917a21 identical bytes | exact | BH,GuiKit |
| label +text | `Label_Text` | 0xD8 | 0xD8 | S 00419790 `81 c1 d8 00 00 00 ADD ECX,0xd8` / G 00917a30 identical bytes | exact | GuiKit |
| text +renderer | `Text_Renderer` | 0x14 | 0x14 | S 00417223 `c7 41 14 00 00 00 00 MOV dword ptr [ECX + 0x14],0x0` / G 009155f3 identical bytes | exact | GuiKit |
| text +string sub-object | `Text_StringSub` | 0x18 | 0x18 | S 00417214 `83 c1 18 ADD ECX,0x18` / G 009155e4 identical bytes | exact | GuiKit |
| string-sub +colour rgb | `Text_Color` | 0x2C | 0x2C | S 0041714d `83 c1 2c ADD ECX,0x2c` / G 0091551d identical bytes | exact | GuiKit |
| border +alpha (float) | `Border_Alpha` | 0x24 | 0x24 | S 00415326 `d9 42 24 FLD float ptr [EDX + 0x24]` / G 009136f6 identical bytes | exact | BH,SB |
| border +tint rgb | `Border_Tint` | 0x28 | 0x28 | S 00415a16 `83 c0 28 ADD EAX,0x28` / G 00913de6 identical bytes (tint pointer pushed for the fill-image Draw) | exact | BH,SB |
| border +fill image* | `Border_Fill` | 0x74 | 0x74 | S 00415a36 `8b 42 74 MOV EAX,dword ptr [EDX + 0x74]` / G 00913e06 identical bytes | exact | BH,SB |
| border +width | `Border_Width` | 0xC | 0xC | S 0041530f `83 78 0c 00 CMP dword ptr [EAX + 0xc],0x0` / G 009136df identical bytes (Draw returns at once when 0) | exact | BH |
| border +height | `Border_Height` | 0x10 | 0x10 | S 00415318 `83 79 10 00 CMP dword ptr [ECX + 0x10],0x0` / G 009136e8 identical bytes | exact | BH |
| border +rotation bits | `Border_Rot` | 0x20 | 0x20 | S 00415a06 `d9 42 20 FLD float ptr [EDX + 0x20]` / G 00913dd6 identical bytes | exact | BH |
| panel control list | `Panel_CtlArray` | 0x24 | 0x24 | S 0040f998 `8b 42 24 MOV EAX,dword ptr [EDX + 0x24]` / G 0090de18 identical bytes (array pointer indexed by ctl id)<br>S 0040f9c8 `83 c1 24 ADD ECX,0x24` / G 0090de48 identical bytes then CALL PtrListAdd | exact | GuiKit |
| panel control count | `Panel_CtlCount` | 0x28 | 0x28 | S 0040f97e `8b 42 28 MOV EAX,dword ptr [EDX + 0x28]` / G 0090ddfe identical bytes (compared with the control id at 0x40f98a) | exact | GuiKit |

### Constants outside K2AddrTable (checked, identical in GOG)

- Party table pointer `*(*(clientApp+4)+0x270)` (Party.h): S 0073fb9f `8b 91 70 02 00 00 MOV EDX,dword ptr [ECX + 0x270]` / G 0040d07f identical bytes; table count at `[table+0]` and 0x88-byte member stride: S 007e5db5 `3b 08 CMP ECX,dword ptr [EAX]` / G 004789e5 identical bytes. `ClientPartyTable` etc. are table fields above.
- HUD slot array: `IMUL ECX,ECX,0xde4` / `LEA EAX,[EDX+ECX+0x1f34]` and the mirror slot `ADD EDX,0x48e0` are byte-identical in GOG UpdatePortraits (hooks 1/2 sit right behind them).
- `creature[0x43d]` (comment-only in ShieldBar.cpp) and `0x57e8c0` / `0x5413c0` slot targets: not used as constants; the vtable slots are read at runtime (Creature_VtMaxHp/CurHp).

## 3. Unresolved items and what would resolve them

| item | status | what would resolve it |
|---|---|---|
| `KeyboardDeviceGlobal` (Steam 0x0099C884) | unknown, zero in kGog | Used only by ForcePowerHotbar (none of the 3 mods). Dump a pair for a Steam reader (0x785DF0 holds 12 reads) and align it, same as the other globals. |
| `TexPendingList` (-> 0x00A14CD4) | high | Hotbar only. Aligned in 3 functions but no semantic parent quote; add the hotbar texture-queue caller pair if the hotbar is ported. |
| `ClipNoFillColor` (-> 0x00A768A4) | high | Single aligned site (RowDraw). A second aligned reader (e.g. the fog-of-war pair 0x895CC0 <-> its GOG twin) would make it exact. Live check: a GOG `PushClip` with this arg draws no fill. |
| `InGameMenuDraw` 0x756B70 -> 0x528270 | high | Only offset evidence (not an address field). It is a vtable-slot function (no direct caller): add a PTRS dump of the CSWGuiInGameMenu vtable on both sides if a hard anchor is wanted. |
| `PtrListAdd` scope | exact for panel lists only | Steam folded identical instances (>=200 callers); GOG keeps per-type copies (0x534170 has 65 callers). GuiKit only appends to the panel control list, which is exactly this instance. Any other GOG pointer list needs its own instance. |
| Everything dynamic | not verifiable statically | `KOTOR_VERSION_SHA` selection, the `kGog` table in a running GOG process, and CGameEffect/portrait behaviour need one live GOG launch (the user, game closed here). All addresses are static-proven; nothing here has been executed on GOG. |
| GOG hooks toml files | not written | Each of `portrait-slots`, `buff-duration-hud`, `shield-bar` needs a `kotor2-gog-aspyr.hooks.toml` with `target_versions = ["777BEE23..."]` and the 7 GOG addresses from section 1 (same `original_bytes`, same parameter sources). Not created (outside the allowed output files). |

## 4. Ready-to-paste `kGog`

`K2AddrTable` is built by name (`makeSteam()` style), so a positional `{...}` list would be unsafe. Replace the `kGog` line in `_shared_src/K2Addr.h` (the file is not modified by me) with the block below. Fields keyed exactly as in the working-tree K2Addr.h (commit 763d010 plus later uncommitted additions; 110 of its 130 fields are mapped here, the other 20 are the Scene/Gob/View/GuiGate/GuiControlCtor inventory-3d fields, left as comments = zero); the `gameaddr::` party addresses used by Party.h (`GetControlledCreature`, `PartyGetAt`, `MemberServerCreature`, `ClientPartyTable`) are table fields and are filled. Also in `gog_map_21b/kgog.inc`.

```cpp
namespace k2addr { namespace detail {
constexpr K2AddrTable makeGog() {
    K2AddrTable t{"777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914"};
    t.AppGlobal = 0x00A11C04;
    t.GuiSoundMgr = 0x00A11BFC;
    t.PlayGuiSound = 0x00910720;
    t.ScreenW = 0x009F22A4;
    t.ScreenH = 0x009F22A8;
    t.GuiRefW = 0x00A0FFE8;
    t.GuiRefH = 0x00A0FFEC;
    t.ProjectionFlag = 0x00A76858;
    t.New = 0x00921347;
    t.LabelCtor = 0x009179E0;
    t.InitControl = 0x0090DAA0;
    t.StopLoadFromLayout = 0x0090DA20;
    t.ExoCtor = 0x00605680;
    t.ExoDtor = 0x00605890;
    t.TextSet = 0x00915200;
    t.SetFont = 0x009151B0;
    t.SetAlign = 0x00915370;
    t.TextColor = 0x00915510;
    t.PtrListAdd = 0x00534170;
    t.GuiLoadImage = 0x008D5470;
    t.GetControlledCreature = 0x004789B0;
    t.PartyGetAt = 0x004789D0;
    t.MemberServerCreature = 0x00429780;
    t.ClientPartyTable = 0x0040D070;
    t.LabelVtable = 0x0098998C;
    t.BorderVtable = 0x009A994C;
    t.TextVtable = 0x009A9A28;
    t.KeyboardDeviceGlobal = 0;   // UNKNOWN (zero = helper inert)
    t.TexPendingList = 0x00A14CD4;   // high
    t.WorldTimerGetCurrentTime = 0x0080B200;
    t.AppGlobal_ToWorld = 0x8;
    t.World_ToSub = 0x4;
    t.Sub_ToTimer = 0x10048;
    t.Creature_EffectArray = 0x148;
    t.Creature_EffectCount = 0x14C;
    t.Effect_Type = 0x8;
    t.Effect_Subtype = 0xA;
    t.Effect_Duration = 0xC;
    t.Effect_ExpiryDay = 0x10;
    t.Effect_ExpiryTime = 0x14;
    t.Effect_Creator = 0x18;
    t.Effect_SpellId = 0x1C;
    t.Effect_IntCount = 0x30;
    t.Effect_Ints = 0x34;
    t.BeginGui2D = 0x008CFBD0;
    t.EndGui2D = 0x008CFD00;
    t.PushClipRect = 0x008CFD70;
    t.PopClipRect = 0x008D02D0;
    t.FlushGuiBatch = 0x008D5370;
    t.FlushArg = 0x009F7144;
    t.HudScale = 0x008D03C0;
    t.ClipDepth = 0x00A32B90;
    t.ClipNoFillColor = 0x00A768A4;   // high
    t.PanelRect = 0x0090E4A0;
    t.ImageVt_Draw = 0x6;
    t.LabelSize = 0x148;
    t.Ctl_Flags = 0x48;
    t.Ctl_Id = 0x54;
    t.Ctl_X = 0x4;
    t.Ctl_Y = 0x8;
    t.Ctl_W = 0xC;
    t.Ctl_H = 0x10;
    t.Ctl_VtSetRect = 0x4;
    t.Label_Border = 0x60;
    t.Label_Text = 0xD8;
    t.Text_Renderer = 0x14;
    t.Text_StringSub = 0x18;
    t.Text_Color = 0x2C;
    t.Border_Alpha = 0x24;
    t.Border_Tint = 0x28;
    t.Border_Fill = 0x74;
    t.Panel_CtlArray = 0x24;
    t.Panel_CtlCount = 0x28;
    t.CExoStringFormat = 0x00606380;
    t.GetMaxForcePoints = 0x006977D0;
    t.Slot_CardRect = 0x54C;
    t.Slot_Flags = 0x590;
    t.Slot_VitCtl = 0x690;
    t.Slot_VitRect = 0x694;
    t.Slot_ForceCtl = 0x7FC;
    t.Slot_ForceRect = 0x800;
    t.Slot_ArrowBuff = 0xDDC;
    t.Slot_ArrowDebuff = 0xDDD;
    t.Slot_Overlay0 = 0x1B8;
    t.Slot_Overlay1 = 0x300;
    t.Slot_Overlay2 = 0x448;
    t.Menu_PortraitSlot = 0x2350;
    t.Menu_Char2 = 0x3134;
    t.Menu_Level2 = 0x327C;
    t.Menu_Back2 = 0x33C4;
    t.Menu_Change2 = 0x350C;
    t.Menu_Char3 = 0x36DC;
    t.Menu_Level3 = 0x3824;
    t.Menu_Back3 = 0x396C;
    t.Menu_Change3 = 0x3AB4;
    t.MenuCaller_Creature = -0x38;
    t.Creature_Stats = 0x1198;
    t.Stats_ForceA = 0x130;
    t.Stats_ForceB = 0x132;
    t.Creature_VtMaxHp = 0x98;
    t.Creature_VtCurHp = 0x9C;
    t.Creature_IconCacheData = 0xFFC;
    t.Creature_IconCacheCount = 0x1000;
    t.IconEnt_F14 = 0x14;
    t.IconEnt_F1c = 0x1C;
    t.Border_Width = 0xC;
    t.Border_Height = 0x10;
    t.Border_Rot = 0x20;
    t.Tip_Slot = 0x1D0;
    t.Tip_Text = 0x1D4;
    // GuiGateGlobal: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GuiControlCtor: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GuiSceneCtor: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneAddModel: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneRemoveModel: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneGetModel: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneViewVtable: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneCameraVtable: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GobVtable: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GobSetPosition: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneViewSize: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // View_Scene: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // View_SceneObj: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // View_Camera: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // View_ModelCount: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneVt_Init: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GobVt_SetPosition: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GobVtReadable: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GobReadable: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // Gob_Scale: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    return t;
}
} } // namespace k2addr::detail
inline constexpr K2AddrTable kGog = k2addr::detail::makeGog();
```

## 5. Riskiest claims to spot-check

1. **TextColor -> 0x00915510** (not the name-matching `SetColor` 0x009153E0): the Steam function is 50 B and byte-identical to GOG `CSWGuiTextParams::SetDefaultColor`; aligned call in TextLoad confirms. Spot check: TextLoad 0x00415850 calls 0x00915510 right after SetAlignment.
2. **PtrListAdd -> 0x00534170** is the CSWGuiControl* instance only (Steam folded all instances into 0x0083EA60; the one xref-count mismatch in 83 pairs).
3. **Vtable addresses** are not shifted consistently (Label 0x0098998C but Border 0x009A994C / Text 0x009A9A28); each is quoted from the ctor `MOV [EAX],imm32` and confirmed by slot-by-slot PTRS dumps.
4. Menu control names: `Menu_Char2/3`, `Level`, `Back`, `Change` come from the string-tag pushes before `InitControl` in the CSWGuiInGameMenu constructor (strings from `strings.tsv`); the offsets are byte-identical in GOG.

## 6. Reproduce

`cd gog_map_21b; python3 run_ghidra.py steam|gog <dir with requests.txt>` re-dumps (about 3-4 min per run, rounds r1..r5 in `dumps/`); `python3 align.py pairs_r1.tsv dumps/steam_r1.txt dumps/gog_r1.txt` prints the alignment; `python3 build_map.py` regenerates this document (from `gog_map_21b/`, writes `../21b_gog_address_map.md`).
