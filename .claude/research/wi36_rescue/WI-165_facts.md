# WI-165 facts: feats/powers panel vftables, per-frame slot, prologue bytes (Steam exe)
Tags: [DUMP] decomp .asm line, D=~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp · [SF] static_facts.py addr · [QG] query_ghidra.py symbol · [PE] exe bytes via scratch script (scratch dir /tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/33bbfba9-58dd-4711-b431-42e5d2bdedec/scratchpad/WI-165-u01/, outputs vtable_slots.txt, vtable_compare.txt, prologue.txt) · [guess] inference
Exe check: sha256 of backups/vanilla_clean/swkotor2.exe starts 6a522e71 = dump id [PE]. Image base 0x00400000.

## 1. Vtables (confirmed)
- CSWGuiFeatsCharGen: $D/90/00909e00.asm:21 `MOV dword ptr [ECX],0x9aa434` -> vftable 0x009AA434 [DUMP].
- CSWGuiPowersLevelUp: $D/90/009074e0.asm:21 `MOV dword ptr [ECX],0x9aa34c` -> vftable 0x009AA34C [DUMP].
- CSWGuiPanel base ctor 0x0040E5A0: $D/40/0040e5a0.asm:30 `MOV dword ptr [EAX],0x98739c` -> base vftable 0x0098739C [DUMP][PE].
- Each derived vftable has 28 code slots (0x00..0x6C); bytes from 0x70 are text ("TRE_", "BTN_"), not pointers [PE].
- Slots 0x98 and 0xA4 are not in either vftable [PE]. Ctor calls at $D/90/00909e00.asm:539 (`CALL [EAX+0x98]`, EAX=this+0x968) and :562 (slot 0xA4, this+0x15C8) run on sub-objects [DUMP].
- REA ledger row "CSWGuiPanel::vftable 27 slots 0..26" undercounts: exe shows 28 (0..27) [PE]. Ledger not edited.

## 2. Slot table (off = byte offset from vftable; full dump in vtable_slots.txt)
Off 0x04..0x6C identical in base, feats, powers except the rows below.
| off  | CSWGuiPanel base | CSWGuiFeatsCharGen | CSWGuiPowersLevelUp |
| 0x00 | 0x0087FDF0       | 0x0090A970 (dtor)  | 0x00908040          |
| 0x34 | 0x0040EC90 Draw  | 0x0090BBB0         | 0x009091E0          |
| 0x3C | 0x0040EAA0       | 0x0090B200         | 0x00908920          |
| 0x48 | 0x0040F3C0       | 0x0090AD90         | 0x00908460          |
Identical rows [PE]: 04 0040E980 · 08 0040EC00 · 0C 0040E770 · 10 00681880 · 14-24 00623320 · 28 00458940 · 2C/30 004558A0 · 38 0040E770 · 40 0040EAE0 · 44 0040E7B0 · 4C 0040F4C0 · 50 0040E7D0 · 54 0040E7E0 · 58 008878D0 · 5C 0040E800 · 60 0040E820 · 64 0040E840 · 68 0040FC50 · 6C 0040FFB0.

## 3. Per-frame slot
- Slot 0x34 = Draw: base vftable +0x34 = CSWGuiPanel::Draw 0x0040EC90 [PE][QG steam_val verified B0]. Both overrides CALL 0x40EC90 [SF]. CONFIRMED as Draw slot.
- Slot 0x3C = input: derived 0x0090B200 / 0x00908920, size 281, RET 8 [SF]. Base 0x0040EAA0 size 51, RET 8, 58 callers, contains indirect slot-0x3C call at 0x0040EACB [SF].
- Per-frame caller: OPEN. REA slot-0x34 callsites 0x007683A4, 0x007683BF, 0x00768409 sit in $D/76/007682b0.asm:72,84,107. 0x7683A4 and 0x768409 are 0-arg calls; 0x7683BF passes 4 args. Draw candidates are RET 4 (1 arg). So these belong to other classes.
- CSWGuiManager::Draw has no symbol [QG]. CSWGuiManager::AddPanel 0x410530: size 317, RET 12, 151 callers [SF].

## 4. Prologue bytes (exe bytes = dump bytes [PE])
| VA         | role              | first 12 bytes                    | cum boundaries | 5-byte cut       |
| 0x0090BBB0 | feats Draw        | 55 8B EC 83 EC 08 89 4D FC 8B 45 FC | 1,3,6,9,12 | splits SUB ESP,8 |
| 0x009091E0 | powers Draw       | 55 8B EC 83 EC 08 89 4D FC 8B 45 FC | 1,3,6,9,12 | splits SUB ESP,8 |
| 0x0090B200 | feats OnInputEvent| 55 8B EC 83 EC 14 89 4D F0 83 7D 0C | 1,3,6,9,13 | splits SUB ESP,14 |
| 0x00908920 | powers OnInputEvent| 55 8B EC 83 EC 14 89 4D F0 83 7D 0C | 1,3,6,9,13 | splits SUB ESP,14 |
- Verdict: 5-byte cut is NOT an instruction boundary for all four. First boundary >= 5 is 6 bytes (PUSH EBP; MOV EBP,ESP; SUB ESP,imm8). A JMP5 hook must relocate 6 bytes and NOP-pad 1.
- Pair prefixes are byte-identical (Draw pair, input pair). Match by VA, not by byte pattern.

## 5. Roles (no strings reported by static_facts for any)
- 0x0041FC30: size 345, RET 16 (4 args), 38 callers (e.g. 0x758E30, 0x7592F0), callees 0x41D290, 0x4142E0, CExoArrayList<uint32>::Add 0x83EA60, 0x421720, 0x41EA50 [SF]. Role [guess]: generic 4-arg helper, not panel-specific.
- 0x0089C290: size 181, RET 0 (this only), callers 0x8A47C0, 0x8A4830, 0x909360, 0x90BD50 (last two = powers/feats RefreshGridStates), callee 0x89A0E0 [SF]. Role [guess]: returns grid index cached at this+0x1BD8 (feats) / +0x1BB0 (powers).
- 0x00914E70: size 203, RET 4 (1 arg = flag), callees operator new 0x919723, CSWGuiFeatsCharGen ctor 0x909E00, CSWGuiManager::AddPanel 0x410530 [SF]. Direct-CALL callers: none (grep `CALL 0x00914e70` over decomp = 0 hits). Role [guess]: feats-panel factory, flag-0 twin of the 0x904510 opener.

## OPEN
1. Per-frame caller: which driver calls [reg+0x34] with one stack arg each frame.
2. Name of base slot 0x3C 0x0040EAA0 and target of its indirect call at 0x40EACB.
3. Real callers of 0x914E70 (table or indirect reference).
4. Class behind this+0x968 (slot 0x98) and this+0x15C8 (slot 0xA4) in the feats ctor.
5. Targets of slot 0x48 overrides 0x0090AD90 and 0x00908460 (unnamed).
6. Roles of 0x41FC30 and 0x89C290 stay [guess]; need pseudo-C.
