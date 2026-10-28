# WI-166 facts: per-frame driver of slot-0x34 (Draw) calls (Steam exe 6a522e71, static only)
Dump: ~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/<hh>/<8hex>.asm. Scan: /tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/7f740322-a7a6-485a-8450-ccf247d33e11/scratchpad/WI-166/scan34.py (cands.tsv, scan34.out.txt alongside).

## 1. Candidate scan (Step 1)
- `python3 -u scan34.py`: 82 slot-0x34 indirect call sites (load `+0x34]` + CALL reg within 2 lines, vtable load within 6 lines). 985 raw `+ 0x34]` lines in 509 files, mostly data fields.
- Literal rule (exactly 1 PUSH between vtable load and CALL): 3 sites, 2 fns. Almost never matches: MSVC pushes args before the vtable load.
- Thiscall rule (exactly 1 PUSH in the 6 instrs before vtable load): 16 sites, 12 fns. The 6-instr window missed 0x004116d0 in 0x4113a0 (see 2).
- Top fns (both rules, hits): 004113a0 2 (41/004113a0.asm:140,181) · 00499610 2 (49/00499610.asm:171,183) · 00730750 2 (73/00730750.asm:105,308) · 0086c460 2 (86/0086c460.asm:38,79) · 0086c5b0 2 (86/0086c5b0.asm:43,91) · 00731070 1 (73/00731070.asm:56) · 0083bec0, 0083c0c0, 0085fe10, 0085fec0, 0086b210, 0086b2d0: 1 each.

## 2. Top 5 ranked (Step 2, static_facts addr; twins = none for all 5)
| fn | size | RET | callers | backward JMP loop | verdict |
|---|---|---|---|---|---|
| 004113a0 | 1227 | 4 | 00409ed0, 0052f610 SaveGame, 00534600, 0053af20 DoPCAutosave, 00781be0 MainLoop | yes: 0x41158e->0x411515 (covers 0x41156f); 0x41160d->0x4115a5 (covers 0x4115ee) | BEST |
| 00499610 | 2200 | 0 | 0046c2b0 Scene::Render | not checked | size out of 800-1300 |
| 00730750 | 1847 | 8 | 0072a130 | not checked | size out; DirectInput8Create (input) |
| 0086c460 | 323 | 12 | 00868230, 0086b210 | not checked | size out |
| 0086c5b0 | 389 | 16 | 008681b0, 0086b2d0 | not checked | size out |
- Linux CSWGuiManager__Draw = 0x085f82ce, 1026b, trusted (`query_ghidra.py symbol`). No Steam twin is linked; 0x4113a0 is 1227 B, so the twin is NOT confirmed.

## 3. Verdict (Step 3): MEDIUM confidence
- Driver fn: 0x004113a0 (unnamed, thiscall, RET 4). Slot-0x34 callsites inside it: 0x0041156f `FF D2` (41/004113a0.asm:140, loop over this+0x88), 0x004115ee `FF D0` (:181, loop over this+0x94), 0x004116d3 `FF D0` (:256, single object this+0x3c, not in a loop).
- Per-frame caller: CClientExoAppInternal::MainLoop 0x00781be0, callsite 0x007837ef `E8 AC DB C8 FF` (78/00781be0.asm:1593). ECX = [EDX+0x274] set at 0x007837e9. Caveat: 0x4113a0 also runs from the save paths (SaveGame, DoPCAutosave, StallEventSaveGame).
- The 1 stack arg is a float, not a context pointer. Driver side: `PUSH ECX` placeholder, then `FLD float [EBP+8]` / `FSTP float [ESP]` (0x41155d-0x411561, same at 0x4115dc-0x4115e0 and 0x4116bb-0x4116bf). MainLoop side: `FLD float [0x009f62b0]` / `FSTP [ESP]` at 0x007837da-0x007837e0.
- Not shown: the slot-0x34 receiver is a feats/powers panel. Static facts do not tie this+0x88, +0x94 or +0x3c to CSWGuiPanel or the feats/powers vftables.

## 4. Receiver chain (Step 3b, retry)
- Field: `query_ghidra.py struct CSWGuiManager` -> `0x0088 CSWGuiManager::panels`. Driver reads `MOV EDX,[ECX+0x88]` and item `MOV ECX,[EDX+EAX*4]` into [EBP-0xc] (41/004113a0.asm:115-119). ECX of 0x4113a0 = CSWGuiManager is inferred from that field offset, not runtime-proven.
- Slot-0x34 receiver = [EBP-0xc] = the list item (:136-140 and :178-181), i.e. the panel, not the manager.
- (a) Scan `recv_scan.py` (scratch): `ADD E?X,0x88|0x94` followed within 3 lines by `CALL 0x0083ea60` (CExoArrayList<uint32>::Add) gives 19 fns. Two are GUI: AddPanel 0x00410530 (`ADD ECX,0x88` 41/00410530.asm:92, CALL :93) and 0x00410900 (`ADD ECX,0x94` 41/00410900.asm:60, CALL :61). The other 17 sit in 0x6d-0x6e; the two checked (0x6df2a0, 0x6db4a0) are CSWSCreature attack resolvers.
- Size: 0x410900 = 180 B, RET 4, equal to GOG PushModalPanel 180 B (Linux 206 B). Steam AddPanel 0x410530 = 317 B (Linux 244 B; build-dependent, weak).
- Ctors: 0x909e00 = CSWGuiFeatsCharGen::Constructor (steam_val C1b; 2914 B = GOG 2914 B). 0x9074e0 = CSWGuiPowersLevelUp::Constructor (2903 B = GOG 2903 B). Each writes its vftable: 0x9aa434 (90/00909e00.asm:21), 0x9aa34c (90/009074e0.asm:21).
- Callers: 0x00904510 calls the feats ctor at 90/00904510.asm:52 (PUSH [this+0x1c] at :49-50) and AddPanel at :70 with ECX=[this+0x1c] (:69), pushes panel, 0x3, 0x1. 0x00904420 calls the powers ctor at 90/00904420.asm:37 (PUSH [this+0x1c] at :34-35) and AddPanel at :55 with ECX=[this+0x1c] (:54), flags 3. 0x0090b430 (CSWGuiFeatsCharGen::OnAcceptButton) also calls AddPanel at :55 and :172.
- Linux proto `CSWGuiFeatsCharGen(this, CSWGuiManager* param_2, ...)` (query_ghidra symbol): so param_2 = [this+0x1c] = manager (type from Linux, Steam not checked).
- Flags 3: AddPanel appends P to +0x88 (410530.asm:92-93). Flag bit 0 then calls 0x410900 (:100), which appends P to +0x94.
- Slot 0x34 (vftable read from `backups/vanilla_clean/swkotor2.exe`, sha256 prefix 6a522e71 = dump build, read-only): feats 0x9aa434+0x34 -> 0x0090bbb0 (244 B, RET 4); powers 0x9aa34c+0x34 -> 0x009091e0 (240 B, RET 4). Both call CSWGuiPanel::Draw 0x0040ec90 (90/0090bbb0.asm:63, 90/009091e0.asm:63).
- Chain: P (0x909e00 / 0x9074e0) -> AddPanel 0x410530 via 0x904510:70 / 0x904420:55 -> manager M=[X+0x1c] list +0x88 (410530.asm:92) and +0x94 (410900.asm:60) -> 0x4113a0 calls [P+0x34] at 41/004113a0.asm:140 (+0x88 loop) and :181 (+0x94 loop) -> Draw override -> CSWGuiPanel::Draw.
- Missing link of section 4 (M == [X+0x1c]) is resolved in section 5 G2.

## 5. Gates (Step 3c, escalation). D = decomp dir
- Name evidence: name_map `query_ghidra.py address 0x4113a0` -> CSWGuiManager::Draw (R9-Phase2, conf medium, GOG delta/size, Opus spot-check); 0x410830 -> CSWGuiManager::FindModalPanelIndex (medium); 0x410530 -> CSWGuiManager::AddPanel (steam_val B0 verified). This supersedes "twin none" in section 2 (static_facts twins only reads links).
- G1a (writers of bit 2 of [panel+0x48]): SET by the base ctor CSWGuiPanel 0x0040e5a0, `OR ECX,0x4` at 40/0040e5a0.asm:80-82 (0x40e6b2). Same ctor stores manager arg at [this+0x1c] (:75, 0x40e69f) and clears bit 1 (:68). Both feats ctor (90/00909e00.asm:18, CALL 0x40e5a0) and powers ctor (90/009074e0.asm:18) call it before writing their vftable, so feats/powers panels start with bit 2 SET.
- G1b (clears): the 8 `AND EAX,0xfffffffb` hits in the ctors (90/00909e00.asm:571,590,609,628; 90/009074e0.asm:565,584,603,622) target sub-controls at this+0xbf8/0xf98/0xdc8/0x1168 (feats, `ADD EAX,..` at :564,583,602,621) and this+0x11d8/0x13a8/0x1578/0x1748 (powers, 009074e0.asm:558,577,596,615), not `this`. AddPanel 0x410530 only clears bit 7 and bits 8-9 (410530.asm:42-50) and toggles bit 3 (:74-78); it never clears bit 2. Verdict: feats/powers panels have bit 2 SET at add time. Later clears (OnAccept etc., virtual slots) not scanned.
- G1c (+0x94 loop gate): SAME gate. 41/004113a0.asm:171-173 (`SHR EDX,2; AND EDX,1; JZ`, 0x4115d4-0x4115da) before the slot-0x34 call at :181.
- Extra gate in the +0x88 loop: after the bit-2 test, 0x410830 (FindModalPanelIndex, searches [mgr+0x94], count [mgr+0x98]; D/41/00410830.asm:14-24) is called (4113a0.asm:130); the Draw call at :140 only happens if it returns -1 (:131-132, JNZ skips). AddPanel flags: callers push 1, 3, panel (90/00904510.asm:64-67 -> flags=3 at :65, same pattern 90/00904420.asm:49-52). Flag bit 0 also adds to +0x94 (CALL 0x410900 at 0x41064d, and 0x4105d0 in the other branch), so feats/powers panels sit in BOTH lists: the +0x88 loop skips them (found in modal list) and the +0x94 loop (:181) is the call that draws them. Hook site is therefore 0x004115ee (callsite in +0x94 loop), not 0x0041156f.
- G2 (manager identity): MainLoop ECX = [EDX+0x274], EDX = this of CClientExoAppInternal::MainLoop (78/00781be0.asm:1592; this saved at :14 `[EBP+0xfffffd38]`). The only writer of [CClientExoAppInternal+0x274] that is a manager is StartServices 0x00788d90: `new(0xa8)` then CALL 0x004100c0 then `MOV [EDX+0x274],EAX` (78/00788d90.asm:1089-1107). 0x004100c0 (698 B, single caller StartServices per static_facts) inits CExoArrayLists at +0x88 and +0x94 (41/004100c0.asm:24,29) = the CSWGuiManager ctor. Other `+0x274` stores (62/, 55/) are other classes (not scanned).
- G2 link to AddPanel: AddPanel receiver is [X+0x1c] (90/00904510.asm:68-70 i.e. 0x9045e6-0x9045e9; 90/00904420.asm:54), X a CSWGuiPanel subclass whose +0x1c was set by the CSWGuiPanel ctor from its manager arg (40/0040e5a0.asm:75). Static proof that every panel gets the StartServices manager is only the single-instance structure (one CSWGuiManager ctor caller); the field chain of X's own creation (panel -> its creator's manager arg) is not traced here.

## OPEN
1. Object at this+0x3c (3rd slot-0x34 call, 0x4116d3, 41/004113a0.asm:256): pseudo-C and vftable not read.
2. G2 residual: trace of X's [this+0x1c] back to the StartServices manager (creator chain of the char-gen panel) not done; only the single-instance argument. Confirm live: log M=[CClientExoAppInternal+0x274] and [X+0x1c] once.
3. Value of global float 0x009f62b0 (per-frame arg) not read.
4. G1 residual: panels' bit 2 could be cleared after add by non-ctor code (OnAccept/Show/Hide virtuals, vftable slot 0x48 called at 0x41065d in AddPanel); not scanned. Live check: read [panel+0x48] in the hook.
5. Fallback candidates if 0x4113a0 fails: 0x00499610 (2200 B, Scene::Render), 0x0086c460 (323 B), 0x00730750 (DirectInput).
6. Scan window (6 instrs) misses PUSH-before-vtable-load further back; thiscall counts are lower bounds.
7. 0x0090bbb0 and 0x009091e0 have no steam_val names (evidence = vtable read + CSWGuiPanel::Draw call).
