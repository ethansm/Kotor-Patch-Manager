# B1 evidence card: FUN_00775700 (entry 0x00775700)

Target: /tmp/claude-1000/rea_probe/swkotor2.exe (PE, x86, sha256 6a522e71...dbbffef, Ghidra 12.1.4).

- Body: 0x00775700-0x00775751, 82 bytes, 3 basic blocks (procedure_info).
- Bytes at entry: 55 8b ec 83 ec 0c 89 4d f4 a1 a4 b4 (read_bytes).
- Prologue spills ECX to [EBP-0xc] (0x00775706) and never reads it again. Calling convention is unconfirmed.
- Epilogue is RET 0x1c, so the callee pops 7 dword stack args ([EBP+0x8] to [EBP+0x20]).
- Callees: 0x0073f810 (0x00775717), then conditionally 0x0087a5c0 (0x00775747).
- Callers: 0x007791a0 and 0x00899270. Both resolve to themselves as containing entries. Call-site addresses are unknown.

Open: call-site addresses inside the callers.
