# B1 checklist: native function 0x00775700

Binary: /tmp/claude-1000/rea_probe/swkotor2.exe (PE x86, sha256 6a522e71...). Ghidra name FUN_00775700, 82 bytes, 3 basic blocks.

- Prologue: push ebp; mov ebp,esp; sub esp,0xc. ECX is spilled to [ebp-0xc] at 0x775706 and never read.
- Calls FUN_0073f810 at 0x775717. If the result is nonzero, it calls FUN_0087a5c0 at 0x775747 with ECX set to the first result and 7 stack arguments.
- Epilogue returns with ret 0x1c, so the callee pops 28 bytes.
- Callers: FUN_007791a0 and FUN_00899270, both reported as entry addresses. Call-site addresses were not returned by any tool.
- Calling convention: unconfirmed.
