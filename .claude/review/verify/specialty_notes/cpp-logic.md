Domain checks for C++ and header logic claims. Apply the ones the claim touches.
- Boundaries and sentinels: inclusive vs exclusive range ends, signed vs unsigned comparison, `0`/`-1`/high-value sentinel ids versus the range of real ids, predicates widened with `||`. Work through the claimed edge value by hand against the quoted condition.
- Null and size checks: is the pointer or index validated on the path that reaches the cited line? Look for the check in the caller as well as the callee. Check `snprintf`/`memcpy` sizes against the buffer.
- Lifetimes: who allocates, who destroys, and on which events (character change, screen close, area transition, save load)? Is an old pointer, id, or cached value used after the owner is gone?
- Offsets and casts: struct offsets and pointer arithmetic on a 32-bit target; does the offset come from a named constant or a lookup (grep `GameAddr.h` and the mod's own headers) or is it invented?
- Definitions: read the actual definition of any macro, inline helper or constant the claim leans on; a macro may change the meaning of the quoted line.
- Shared headers: if a shared (Common / `_shared_src`) header changed, check whether the dependents named in the claim still match the new signature or semantics.
- Intent: comments, log strings and tests next to the code often state what the author meant; quote them when they settle the question.
