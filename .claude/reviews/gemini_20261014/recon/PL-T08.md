# PL-T08
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4d-06 | CONFIRMED | patch_manager_mods/perf_lab_src/ghidra_scripts/NewNullCheck.java:63,66,67,69-73 | yes | Both gaps real: RET not a stop (67 continue); only MOV updates held. |

- NewNullCheck.java:62 - q is seeded with ci.getNext(), the instruction after the allocator CALL.
- NewNullCheck.java:60 - held is seeded as {"EAX"} only; matching is by operand string, so writes to AL/AX never remove EAX.
- NewNullCheck.java:63 - forward loop is capped at k<8 over q.getNext(); no isTerminal/isReturn test exists in the loop.
- NewNullCheck.java:66 - the only break besides jumps is on isCall(); a RET does not stop the walk.
- NewNullCheck.java:67 - for <2-operand insns, breaks only if isJump(), else continue. RET (0-1 operands) and 1-operand POP both take continue, so POP never updates held.
- NewNullCheck.java:69-70 - only MOV updates held (add o0 if o1 is held, else remove o0).
- NewNullCheck.java:71-73 - TEST/OR/CMP only set checked=yes; no branch removes o0 from held, so XOR EAX,EAX (o0=o1=EAX) and LEA EAX,[..] leave EAX held.
- NewNullCheck.java:65 - after-column records the first 5 insns, which can include post-RET code (manual judgement only, does not affect checked).
- NewNullCheck.java:51 - the backward push walk does stop at call/jump; the forward walk has no equivalent RET stop.
- Fix check: a stop-at-RET and a remove-on-non-MOV-write both only lower checked=yes, so they are safe; the list must also cover the 1-operand POP path at :67 and subregister writes.
