# SH-T03
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3a-04 | CONFIRMED | patch_manager_mods/_shared_src/CodePatch.h:91 (read), :92 (compare), :94 (protect), :70-74 (committed, unused here) | yes | Read precedes committed()/protect; no n<=0 guard in write or writeChecked. Gap real; caller reach unverified. |

- CodePatch.h:89-91: writeChecked declares `seen`, then `memcpy(seen, p, n)` reads the target with no prior check. Anchor `memcpy(seen, p, n);` is exact at :91.
- CodePatch.h:92-93: memcmp compare runs after the read; mismatch returns -1 before any protect call.
- CodePatch.h:94: protect(PAGE_EXECUTE_READWRITE) happens only after the read and the compare.
- CodePatch.h:70-74: committed() exists (VirtualQuery, MEM_COMMIT, region bound) but write (:77-85) and writeChecked (:89-99) never call it.
- CodePatch.h:80: write() protects before its memcpy, so a protect failure on a free or reserved target returns -2 with no read or write. writeChecked has no such gate, so an unreadable target faults at :91 instead of returning a code.
- CodePatch.h:87-88: doc comment says `seen` is filled before compare so every return code leaves evidence. Read-first is intentional for evidence; the gap is the missing committed check.
- CodePatch.h:81, :91, :95: memcpy size is int n converted to size_t, so n<0 becomes a huge length. n==0 is also unguarded. No `n <= 0` check exists in either function.
- Fix shape: `if (n <= 0 || !committed(at, n)) return <new code>;` before :91 is safe. 1/-1/-2 are taken, so a new code is needed; `seen` stays unfilled on that path.
- Do not move protect above the read: it would change mismatch-path side effects (protect then restore), though no bytes are written.
