# U2 allocator recon (WI-87) - backup copy

Target file NOT written: Write to .claude/waves/fix2/re2/u2_allocator.md was refused ("File has not been read yet"); the file exists and no Read tool is available to this agent.

Binary: swkotor2.exe (Steam), sha256 7751f7d8b06c63a21eacbef6e5ded6cb4f497fa960cca6a6cf645552701dc04f, REA provider ghidra 12.1.4.
Address form: REA normalized (no leading zeros, e.g. 0x921347).

| Address | What | Evidence |
|---|---|---|
| 0x921347 | REA name `__vsnprintf`, body 0x92133d-0x921359 (29 B). Brief's "operator new" NOT supported | procedure_info(0x921347); ev_aac93d5d |
| 0x92128a | `__vsnprintf_l`, sole callee of 0x921347. No malloc/Heap* call | procedure_callees(0x921347); pseudo(0x92128a) |
| 0x9196fd | `FUN_009196fd(void*)`: body only `_free(param_1)` | pseudo(0x9196fd); ev_d5034836 |
| 0x91c6b5 | `_free`. If DAT_00a80dd8==3 tries ___sbh_find_block/___sbh_free_block; then `HeapFree(DAT_00a806c8,0,p)` | pseudo(0x91c6b5); ev_aa227dba |
| 0x9215e6 | `_malloc`. DAT_00a80dd8==1: `HeapAlloc(DAT_00a806c8,0,n)`. ==3: `_V6_HeapAlloc` first, else `HeapAlloc(DAT_00a806c8,0,rounded)` | pseudo(0x9215e6); ev_00252cd3 |
| 0x919723 | `FUN_00919723(size_t)`: loop `_malloc(n)`, else `__callnewh(n)`, else throw bad_alloc. Operator-new shape. Name NOT confirmed (search "operator new": 0 hits) | pseudo(0x919723); ev_8c74cbe4 |
| 0x9215e6 callers | 0x919723, 0x91c3be, 0x91c42e, 0x926978, +10 more | procedure_callers(0x9215e6) |
| 0x91c6b5 callers | 0x9196fd, +40 more | procedure_callers(0x91c6b5) |
| 0x7510b0 | CSWGuiLabel scalar deleting dtor: `*this = CSWGuiLabel::vftable`; if (flag&1) `FUN_009196fd(this)`. Callees 0x414c60, 0x417290, 0x418640, 0x9196fd | pseudo(0x7510b0); callees(0x7510b0) |
| 0x9179e0 | NOT a CSWGuiLabel ctor. Inside `FUN_00916ef0`, body 0x916ef0-0x9179f4, which writes CSWGuiPortraitCharGen::vftable | procedure_info(0x9179e0) |
| 0x916ef0 callers | 0x914bf0, 0x915f60 (callers of containing procedure) | procedure_callers(0x9179e0) |
| CSWGuiLabel::vftable | 0x9878bc. Xref sites 0x916a10, 0x917a30, 0x74a780 write it into parent member slots, not a standalone ctor | find_xrefs_to_name; pseudo(0x916a10); pseudo(0x917a30) |

Heap pair: free 0x9196fd -> 0x91c6b5 -> HeapFree(DAT_00a806c8). Alloc (operator-new shape) 0x919723 -> 0x9215e6 -> HeapAlloc(DAT_00a806c8) or _V6_HeapAlloc (sbh, gated by DAT_00a80dd8==3, which _free also checks). Same global, same CRT heap. 0x921347 is not on this path.

Ctor callers (brief item 3): unresolved. Real CSWGuiLabel ctor not located.
Not checked: writer of DAT_00a806c8/DAT_00a80dd8; whether 0x919723 is what K2AddrTable::New calls.

Verdict: brief premise (0x921347 = operator new) REFUTED. Same heap for 0x919723 vs 0x9196fd CONFIRMED on decompile, symbol unconfirmed.
