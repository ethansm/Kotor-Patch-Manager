# QL2 recon RB: clear-then-add hand-off (rea-recon haiku, run 2, swkotor2.exe sha 7751f7d8)

Verdict PARTIAL. Run 1 was BLOCKED (no binary path in the brief); this file replaces that stub. The agent could not write it (no Read tool), so the main agent transcribed its reply.

- Q1 `0x51CB10` ClearAllActions: calls `0x590430` with ECX=[cr+0x10DC] at `0x51cb47` (read_function_instructions 0x51cb10). `0x590430` drains the list at `this+0xB00` (round list), unconditionally (procedure_pseudo_code 0x590430). The `cr+0x100` queue is cleared through `0x541080`: `0x5410e9 CMP [cr+0xEC],1`, then `0x5410f9 ADD ECX,0x100; CALL 0x7a1720`, plus a virtual filter. `cr+0xEC` is the commandable flag (doc 27 :96, doc 28 :82: 0 = cutscene/stun), normally 1 in combat. So both lists are emptied in a normal combat.
- Q4: `0x51CB10` has NO combat refusal. "Cancel refused in combat" (QueueHud.cpp ~:654) is our own UI gate. Doc 27 :96 also records the engine itself calling ClearAllActions(1) `0x541080` in combat (companion-ai re-plan).
- Q3: a new round node is pushed to the FRONT (`0x739DA0`). The target list identity and the dequeue end (`0x73A200`) are unresolved.
- Q2: attack enqueuer NOT identified; how soon the attack is re-added after a clear is UNKNOWN.
- Open: `0x775600` -> `0x5648D0` path; vtbl +0x10/+0x74 meaning.
- Decision: no evidence of the attack being re-added ahead of the item. Live test S1 settles it; `handoff_clear_first=0` is the rollback.
