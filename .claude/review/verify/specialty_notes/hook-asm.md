Domain checks for hook, detour and ASM-cave claims. Apply the ones the claim touches.
- Calling convention at the hooked site: which registers carry arguments (`this` in ECX for thiscall), who pops the stack (`RET n`, caller cleanup), and whether the handler's declared convention matches.
- Stack offsets: `[ESP+n]` / `[EBP+n]` argument offsets before and after any `PUSH`/`PUSHAD`/`PUSHFD`; is the offset corrected for what the cave pushed?
- Register and flag use: which registers does the handler clobber, and does the hook restore them or list them in an exclusion field? Are flags saved if a `CMP`/`TEST` result is consumed after the cave?
- Triple consistency: the hook name and address in the `*.hooks.toml` entry, the symbol listed in `exports.def`, and the function definition in the source must agree on the name, calling convention and parameter list. Grep all three.
- Byte patches: `original_bytes` length equals the whole-instruction window being stolen; the return/exit address lands on an instruction boundary after that window; stolen bytes hold no relative `CALL`/`JMP` that breaks once relocated; patch size fits the gap.
- Steam vs GOG: does a sibling hooks file for the other executable build carry a matching entry (or a stated reason it does not)?
- Failure path: what runs if the hook fails to install, and is state left half-initialised?
