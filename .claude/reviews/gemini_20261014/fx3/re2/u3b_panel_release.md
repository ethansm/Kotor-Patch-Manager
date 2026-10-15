# U3b panel release (WI-87) recon
Binary: swkotor2.exe, sha256 7751f7d8b06c63a21eacbef6e5ded6cb4f497fa960cca6a6cf645552701dc04f, REA provider ghidra. Read-only. Addresses as REA prints them.

| address | what | evidence |
|---|---|---|
| 0x40f620 | InitControl: ECX=panel, stack ctl,tag,addToList,scale; RET 0x10 | asm 0x40f629, 0x40f632-0x40f651, 0x40f9e7 |
| 0x40f620 | ctl+0x38 = panel always (ECX=ctl, first stack arg=panel, stored by 0x418df0) | asm 0x40f64a-0x40f651; pseudo 0x418df0 `in_ECX[0xe]=param_1` |
| 0x40f620 | addToList=0: list block skipped, nothing added to panel+0x24 | asm 0x40f963-0x40f967 (JZ 0x40f9e4) |
| 0x40f620 | addToList!=0: idx=ctl+0x54; idx<panel+0x28 gives panel+0x24[idx]=ctl (overwrites, no release); else NULL-pad then push via 0x83ea60 (ECX=panel+0x24) | asm 0x40f969-0x40f9df |
| 0x40f620 | other writes: ctl+0x4..0x10 rect via slot1; sub-object flag bits (LBL_BAR path); no focus, hover or global writes seen | pseudo 0x40f620; pseudo 0x4199a0 |
| 0x418df0 | ctl method: walks panel+0x34 names; tag match (0x7339d0, unresolved) calls ctl slot18; no list write | pseudo 0x418df0 |
| 0x83ea60 | push_back on ECX (data [0], count [1], cap [2]); ECX=panel+0x24 | pseudo 0x83ea60; asm 0x40f9c8 |
| 0x9878bc | CSWGuiLabel vftable: s0 0x7510b0, s1 0x4199a0, s18 0x4198d0, s19 0x4558a0, s20 0x458940 | read_bytes 0x9878bc len 128 |
| 0x7510b0 | label slot0: vftable reset; dtor chain 0x417290, 0x414c60, 0x418640; if flags&1 then 0x9196fd(this) (_free) | pseudo 0x7510b0 |
| 0x417290, 0x414c60 | release this+0x14 and this+0x6c/+0x70/+0x74 via their slot1(1), not slot0 | pseudo 0x417290, 0x414c60 |
| 0x419740 | label ctor, no allocation; clears +0x48 bit2 | pseudo 0x419740 |
| 0x40e860 | CSWGuiPanel dtor: slot0(1) on +0x60 and +0x30 objects; calls 0x40f5a0; NO loop over +0x24 | pseudo 0x40e860 |
| 0x40e5a0 | panel ctor: heap 0x78 object (0x919723) stored at +0x60 | pseudo 0x40e5a0 |
| 0x40f5a0 | StopLoadFromLayout: if +0x48 bit1, slot0(1) on +0x30, clear, call 0x40fb70; no child free | pseudo 0x40f5a0 |
| 0x40fb70 | iterates +0x24 (count +0x28); calls child slot 0x4c (no free, no slot0); nonzero result R used as ECX for 0x418720/0x4186f0 (R+0x60+4i) | asm 0x40fbcd-0x40fc26 |
| 0x98739c | CSWGuiPanel::vftable xrefs: 0x40e60b (ctor), 0x40e88b (dtor) | find_xrefs_to_name |
| 0x754ed0 | InGameMenu ctor: InitControl(addToList=1) on embedded members at byte offsets 0x68, 0x228, ... | pseudo 0x754ed0 |
| 0x741940 | registers its own object into param_1 with addToList=(param_3==0); children embedded at +0x1d0, +0x3a0, +0x570 | asm 0x741a0e-0x741ad6 |
| 0x7423d0 | LBL_NAME, LBL_NAMEBG, LBL_HEALTHBG, PB_HEALTH embedded at +0x1668, +0x17b0, +0x18f8, +0x1a40; addToList=0 | pseudo 0x7423d0 |

## Answers
1. addToList=1 stores ctl at panel+0x24 index ctl+0x54. addToList=0 writes no list, focus or hover; only ctl+0x38 = panel.
2. No routine found that calls child slot0(1) over panel+0x24. Panel dtor and StopLoadFromLayout free only their own +0x60/+0x30 objects. Children on the 0x754ed0, 0x741940 and 0x7423d0 paths are interior (embedded) pointers.
3. slot0(flags=1) on a heap-allocated label built with addToList=0 frees it and its sub-objects; the list is not involved. Unsafe for embedded labels (_free on an interior pointer); use slot0(flags=0) there.

## OPEN
- Panel dtor Ghidra label 'Free(...)' unresolved (may free the +0x24 buffer; unverified).
- 0x40fb70 writes R+0x60..; if R is a panel, that clashes with the 0x78 object at +0x60. R's class and label slot19 stub 0x4558a0 unresolved.
- No offset-based xref search for other +0x24 iterators; CSWGuiInGameMenu dtor not found (vftable xref only 0x754f1a).
- REA session dropped after about 20 calls; one recovery open_binary with the same args was used.
