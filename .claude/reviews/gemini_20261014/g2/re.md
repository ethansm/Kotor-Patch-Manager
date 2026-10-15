# G2 RE answers: swkotor2.exe (sha256 7751f7d8...dc04f, Ghidra 12.1.4 via REA, read-only)

## CO-T05 (CO-2c-06)
- **Answer**: The four pops write, in order: [EBP-0x20] CExoString (ptr at -0x20, len at -0x1C) from a tag-5 string pop; [EBP-0x10] int x from a tag-3 pop; [EBP-0x14] int y from a tag-3 pop; [EBP-0x18] 4-byte float life from a tag-4 pop. Hook site 0x6F62F2 is reached only when all four pops succeed. Pop order string, int, int, float matches the stated AurPostString(string,int,int,float) order.
- **Evidence**:
  - 0x6F622B `CALL 0x733540` on ECX=&[EBP-0x20]: ctor zeroes [obj] and [obj+4] (asm 0x733540).
  - 0x6F6241 `CALL 0x6FDA70` pushes &[EBP-0x20]: string wrapper -> 0x702DD0, checks tag == 5, then 0x7337C0 assigns into the pushed object (asm 0x702E2C-0x702E2F).
  - 0x6F6272 `CALL 0x6FD9A0` pushes &[EBP-0x10]: int wrapper -> 0x702B60, checks tag == 3, writes 4 bytes.
  - 0x6F62A0 `CALL 0x6FD9A0` pushes &[EBP-0x14]: same int path, tag 3.
  - 0x6F62CE `CALL 0x6FD9E0` pushes &[EBP-0x18]: -> 0x702C30, checks tag == 4, writes 4 bytes (float).
  - 0x6F62D5 `JNZ 0x6F62F2` (success edge). 0x6F62F2 bytes C7 45 CC 00 00 00 00 = MOV [EBP-0x34],0 (return slot, read at 0x6F6308).
  - Failure paths store 0xFFFFF82F into [EBP-0x24/-0x28/-0x2C/-0x30] and return it.
- **Verdict**: REFUTED. The mod's frame slots (string -0x20, x -0x10, y -0x14, life -0x18) match the pops.
- **Confidence**: high for slots and wrapper tags. Medium for type names: tags 3/4/5 mapped to int/float/string by NWScript convention, and nwscript.nss is not in the exe.

## X-T04 (SH-3a-07)
- **Answer**: The brief's addresses are not function entries in this database. 0x009179E0 is an interior, non-instruction-boundary address inside FUN_00916EF0 (body 0x916EF0-0x9179F4). 0x0090DAA0 is a non-boundary address inside FUN_0090C6A0. No CSWGuiLabel or InitControl symbol exists. Allocation and list-insertion questions are unanswered.
- **Evidence**:
  - 0x9179E0: resolve_containing_procedure -> 0x916EF0; inspect_native_instruction status "not-instruction-boundary".
  - 0x90DAA0: resolve_containing_procedure -> 0x90C6A0 (non-contiguous body to 0x90E40D); status "not-instruction-boundary".
  - FUN_00916EF0 pseudocode references CSWGuiPortraitCharGen::vftable; FUN_0090C6A0 references CSWGuiSkillsCharGen::vftable. Neither is the label ctor.
  - CSWGuiLabel::vftable at 0x9878BC: more than 100 xrefs (e.g. 0x419778, 0x42049E).
  - CSWGuiPanel::vftable at 0x98739C: xrefs 0x40E60B, 0x40E88B.
- **Verdict**: UNSURE. The claim cannot be checked at the stated addresses.
- **Confidence**: low.

## X-T05 (SH-3a-08)
- **Answer**: The only window-creation import is USER32 CreateWindowExA (EXTERNAL:0x133). No CreateWindowExW import exists. DestroyWindow is EXTERNAL:0x137. Destroy-and-create code exists in the binary, but I did not identify the main-window path or count how often it runs per process.
- **Evidence**:
  - CreateWindowExA callers: 0x4087A0, 0x408AF0, 0x409550, 0x40CC70, 0x441DD0.
  - DestroyWindow callers: 0x409550, 0x409750, 0x40CB20, 0x441DD0, 0x478300.
  - 0x409550 (511 bytes) and 0x441DD0 (642 bytes) appear in both lists.
  - Caller names are FUN_ only; none is labelled as main window.
- **Verdict**: UNSURE. A stale HWND is possible if the main window is recreated, but that is not confirmed.
- **Confidence**: low.

## HN-T06 (HN-6n-01)
- **Answer**: Ctor 0x561F30 (args p at [ebp+8], id at [ebp+0xC]) calls base ctor 0x53E830 at 0x561F74, then stores vtable 0x00994F5C at 0x561F86. Its body also constructs two 14-element CExoString arrays at this+0x270 and this+0x2E0 (ctor 0x733540, dtor 0x733780). The scalar deleting dtor at vtbl[0] is 0x00563490. It calls body dtor 0x5634C0 at 0x56349A, then operator delete at 0x5634AB if arg&1. No global-list registration appears in the decoded bytes. Base ctor 0x53E830 is only partly decoded, and its callee 0x53D660 and base dtor 0x5634C0 are not decoded, so registration is unknown. LoadTemplate 0x572A70 calls operator new(0xA0) at 0x572AAA. On NULL, [ebp-0xAC]=0 at 0x572AE0 and no object is constructed. Otherwise it calls the ctor at 0x572AD3 and stores the pointer. A second check at 0x572B10 tests object+0x94 and is undecoded.
- **Evidence**:
  - 0x994F5C: vtbl[0] = 0x00563490, vtbl[1] = 0x00563C40.
  - 0x561F74: `CALL 0x53E830`. 0x561F86: C7 00 5C 4F 99 00 = MOV [EAX],0x994F5C.
  - 0x563490-0x5634BB: body dtor call, `AND arg,1`, delete call, RET 4.
  - 0x572AAA: new(0xA0). 0x572AC0: JZ to 0x572AE0 (NULL branch).
- **Verdict**: UNSURE. A dangling registration is neither confirmed nor refuted. On new() failure the ctor is never reached, so the claimed ctor-then-vtbl[0] sequence does not occur on that branch.
- **Confidence**: low.

## K1-T15 (K1-5b-03)
- **Answer**: Not determined. The 2DA parser and its row-label handling were not located. The exe holds only class names and strings: C2DA (0xA100D4), CRes2DA (0xA1020C), " 2DA file" (0x99A8FC), "2DAName" (0x999C8C).
- **Evidence**:
  - search_strings "K1P_CommM_Probe": no match, so the row label is game data, not in the exe.
  - search_strings "2DA V2": no match.
  - Load2DArrays 0x4805B0 and Load2DArrays_Appearance 0x480C10 were not decoded.
- **Verdict**: UNSURE.
- **Confidence**: low.

## PL-T01 (PL-4a-01, PL-4a-02)
- **Answer**: CExoResMan::Malloc (0x712F30) reads [CRes+0x18] at entry with no NULL test (0x712F3F: MOV EDX,[EAX+0x18]), so a NULL CRes faults in game code. FreeResourceData (0x7120C0) tests the arg first (0x7120C9: CMP [EBP+8],0), then reads [CRes+0x10] at 0x7120D4, [CRes+0x8] at 0x7120DB, and [CRes+0x20]. Caller guarantees on liveness were not decoded.
- **Evidence**:
  - Malloc callers: 0x7133A0, 0x713BF0, 0x713FB0, 0x714B30.
  - FreeResourceData callers: 0x711FE0, 0x712160, 0x722A30.
  - Byte decode of 0x712F30 and 0x7120C0 (64 bytes each).
- **Verdict**: UNSURE. For NULL, a hook that reads CRes+0x10 before the 0x7120C9 test would fault where the game does not, in FreeResourceData. Stale or freed non-null pointers cannot be decided statically from these callers.
- **Confidence**: medium for offsets; low for caller liveness.

## PL-T06 (PL-4d-01)
- **Answer**: Not enumerated. REA exposes no IAT-thunk inventory, and I did not scan .idata slots. Spot checks found only USER32 CreateWindowExA (EXTERNAL:0x133) and DestroyWindow (EXTERNAL:0x137).
- **Verdict**: UNSURE.
- **Confidence**: low.
