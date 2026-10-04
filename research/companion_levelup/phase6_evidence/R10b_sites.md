# R10b hook sites: static evidence (2026-10-04, read-only)
Exe: Steam Aspyr swkotor2.exe (live = vanilla + 4gb-patch). Disasm: objdump -d -M intel.
Vector layout (both builders, helper 0x83ea60 / 0x593450): {ptr +0, count +4, capacity +8}; grow = cap*2 (first 0x10) via realloc.

## Site F: feats 0x90C36A len 7 (BuildChoosableFeatVector 0x90C230..0x90C387)
1. Bytes: PASS. live and swkotor2.exe.unpatched_vanilla (in $G) both:
   `90c36a: c7 45 fc ff ff ff ff  mov DWORD PTR [ebp-0x4],0xffffffff`
2. No branch into site+1..+6: PASS. Only branches to 0x90C36A itself: `90c2a0: jge 0x90c36a`. Other targets in fn: 0x90c288 (loop head), 0x90c27f (inc), 0x90c365 (jmp back), none in 0x90c36b-0x90c370. No jmp/jcc after the site (straight to epilogue 90c371..90c387 ret).
3. Exit paths: PASS. Single ret (0x90C387). Only loop exit is `90c2a0: jge 0x90c36a` (index >= [g_tbl+0x11e]); no early return, no other path to epilogue. Every continue/skip goes via 0x90c365 -> jmp 0x90c27f (loop), never past site.
4. Overlap: PASS. Existing hooks near: level-up-override LuoR10_90C2BE (0x90C2BE, 5 bytes, consumed_exit 0x90C365) = 0xAC below. Nearest other: none within 0x90C35A..0x90C37A. check_hook_spans.py --quiet: FAIL=0 (4 WARN unrelated, plus a level-up-override source/live drift, unrelated).
5. Panel ptr: PASS. [ebp-0x28] written once: `90c255: mov [ebp-0x28],ecx`; no other write to [ebp-0x28] in fn (all other uses are reads: mov eax/ecx,[ebp-0x28]). Calls pass it as ecx only (callees cannot modify the local).
6. Vector panel+0x1BC0: append callee 0x593450 (call at 90c360 after `add ecx,0x1bc0`):
   `593499: mov ecx,[eax]` ; `59349b: mov ax,WORD PTR [ebp+0x8]` ; `59349f: mov WORD PTR [ecx+edx*2],ax` ; count `[eax+4]` += 1; capacity `[eax+8]`.
   Element = ushort (2 bytes). ptr @panel+0x1BC0, count @+0x1BC4, cap @+0x1BC8. (Find-callee 0x90c620 used for membership checks of 0x1ba8/0x1bb4/0x1bcc/0x1bc0.)
7. Callers of 0x90C230 (call sites; enclosing fn start; refresh order):
   - 0x90B1E1 in fn 0x90AEA0: builder then `90b1e9: call 0x90bd50` (RefreshGridStates) then ret. AFTER: OK (direct).
   - 0x90C0F6 in fn 0x90BF90 (wrapper, ret 4, no refresh inside). Wrapper callers: 90b7d7, 90b8de; refresh 0x90bd50 follows at 90b859 and 90b8e6 (next calls in the callers). AFTER: OK via caller.
   - 0x90C224 in fn 0x90C120 (wrapper, ret 4, no refresh inside). Wrapper callers: 90b790, 90b8fe, 90c1fb; refresh at 90b859 / 90b906 follows. AFTER: OK via caller (90c1fb is inside the 0x90C120..0x90C22C span, i.e. an inner recursive/inline call; not traced).
   Caveat: order via wrapper callers inferred from adjacent call addresses, not full CFG.

## Site P: powers 0x909B56 len 7 (BuildChoosablePowerVector 0x9098D0..0x909B73)
1. Bytes: PASS. live and unpatched_vanilla both:
   `909b56: c7 45 fc ff ff ff ff  mov DWORD PTR [ebp-0x4],0xffffffff`
2. No branch into site+1..+6: PASS. Branch targets in fn: 0x90991e, 0x90992a, 0x909a74, 0x909b1f, 0x909b11, 0x909b1a, 0x909b3e, 0x909b51, 0x909b56 (site start only). None in 0x909b57-0x909b5c. Epilogue 909b5d..909b73 has no branches.
3. Exit paths: PASS. Single ret (0x909B73). Only loop exit `909948: jge 0x909b56`. All skip/continue paths (je/jne at 909964/909980/90999c/9099b8/9099e2/909a1c/909a2f and jmp 909b3c) go to 0x909b51 -> `jmp 0x90991e` (loop inc), none bypass the site. Early return before vector built: none.
4. Overlap: PASS. Nearest hook: level-up-override LuoR10_9099A2 (0x9099A2, consumed_exit 0x909B51) = 0x1B4 below site. Nothing within +-16 bytes of 0x909B56 (check_hook_spans FAIL=0).
5. Panel ptr: PASS. [ebp-0x68] written once: `9098f5: mov [ebp-0x68],ecx`; every other occurrence is a read (mov eax/ecx/edx,[ebp-0x68]). (Scratch locals at ebp-0x6c/-0x70/-0x74/-0x78 are distinct slots.)
6. Vector panel+0x1BD4:
   - append `90 9b4c: call 0x83ea60` (after `add ecx,0x1bd4`): `83eaae: mov [ecx+edx*4],eax` (dword store), count [this+4]++, cap [this+8], init cap 0x10, grow shl 1.
   - insert-at-index `909b37: call 0x414280` (ret 8): shifts `mov edx,[edx+esi*4-4]; mov [ecx+eax*4],edx`, then `4142d4: mov [edx+eax*4],ecx`.
   - find `4142e0` (membership): `41430e: mov eax,[ecx+edx*4]`; cmp `[this+4]` count.
   Element = DWORD (4 bytes). ptr @panel+0x1BD4, count @+0x1BD8 (read at 909a81 `[edx+0x1bd8]`), capacity @+0x1BDC.
   Build logic keeps elements = spell id (push of ushort idx zero-extended); sub-power insertion keeps list sorted-by-chain.
7. Callers of 0x9098D0:
   - 0x9088DF in fn 0x908540: builder then `9088ea: call 0x909360` (RefreshGridStates). AFTER: OK (direct).
   - 0x90977B in fn 0x9095F0 (wrapper ret 4, no refresh inside). Wrapper callers 908d6f, 908ea3; refresh 0x909360 at 908e04 (after 908d6f) and 908eab (after 908ea3). AFTER: OK via caller.
   - 0x9098BA in fn 0x9097A0 (wrapper ret 4, no refresh inside). Wrapper callers 908d3b, 908ec2, 90988e; refresh at 908e04 (after 908d3b) and 908eca (after 908ec2). AFTER: OK via caller (90988e is inside the 0x9097A0 span, not traced).
   Caveat: wrapper-caller order inferred from adjacent call addresses, not full CFG.

## VERDICT
- Site F 0x90C36A len 7: SAFE. No inbound branch to +1..+6, single loop exit, sole ret, panel [ebp-0x28] stable, no hook within +-16, refresh runs after builder. Element size = 2 bytes (ushort); vec ptr/count/cap at panel+0x1BC0/+0x1BC4/+0x1BC8.
- Site P 0x909B56 len 7: SAFE. Same reasons; panel [ebp-0x68] stable. Element size = 4 bytes (dword); vec ptr/count/cap at panel+0x1BD4/+0x1BD8/+0x1BDC. Compaction must use 4-byte moves and decrement count at +0x1BD8.
