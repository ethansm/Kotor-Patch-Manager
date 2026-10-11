# B1 free-form: native function 0x00775700 (swkotor2.exe)

Provider: ghidra 12.1.4, sha256 6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef.

FUN_00775700 is an 82-byte x86 function (0x775700-0x775751, 3 basic blocks). It has no tool-reported signature parameters. Its assembly shows 7 dword stack arguments at [EBP+0x8] through [EBP+0x20] and a callee-cleanup return (RET 0x1c). ECX is saved on entry and never read.

Logic: it calls 0x73f810 and, if the result is non-zero, calls 0x87a5c0 with ECX set to that result plus the 7 stack arguments.

Callers: 0x7791a0 (FUN_007791a0) and 0x899270 (FUN_00899270), both entries of their containing functions.

Unknown: the formal calling-convention label, and the identity of the global at 0x00a1b4a4 (no name reported).

Full fields: findings JSON in the same out/ directory.
