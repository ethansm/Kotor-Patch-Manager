# Analysis-batch contract (KOTOR II Steam swkotor2.exe, sha256 6a522e71..., 32-bit x86 MSVC, image base 0x400000)

You are a read-only static analyst. Root: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` (= ROOT).

## Inputs (text only; NO Ghidra, NO DB writes, NO repo edits, NO game/installed-file changes)
- `decomp/<bb>/<addr8>.c` Ghidra decompile of each non-thunk function; `<bb>` = chars 3-4 of the 8-hex address
  (e.g. 0x00712f30 -> `decomp/71/00712f30.c`). First line: `// FN <addr> <name> size= ret=N callers=... callees=...`
  (`ret=N` = stack bytes popped: thiscall/stdcall arg bytes; 0 = cdecl or no args).
- `decomp/<bb>/<addr8>.asm` disassembly: `address  bytes  instruction`. Use it for hook sites and `original_bytes`.
- `callgraph.tsv` (caller_fn, call_site, callee_fn|import|-, CALL/TAILJMP/IMPORT/INDIRECT, insn), `callers_count.tsv` (fan-in).
- `strings.tsv` (addr, string, ref_count, fn:name;...), `imports.tsv` (import, lib, calling_fn, name, site, reftype, insn).
- `alloc_sites.tsv` every call to operator new 0x919723 / _malloc 0x9215E6 / _realloc 0x934311 / calloc 0x931AB0 with
  constant size (or `var:`), `checked` (result NULL-tested nearby; heuristic, confirm in the .asm), next insns.
- `globals.tsv` readers/writers of seeded engine globals (may be absent for early batches).
- `inputs/census_by_a2.tsv` live heap census (menu -> in-game, 67610 frames, 257 MB live). **Key on a2** (the return
  address one frame up; a1 can be a stale stack value, lesson 155). a2 is a *return address*: the call instruction is the
  one ending right before it; find the function containing it via the .asm files. `inputs/perflab_alloc*.txt` raw.
- `inputs/name_map.tsv` (Steam names already transferred), `inputs/db_functions.tsv` (verified purposes),
  `inputs/candidates.tsv` (optimisation candidates), `inputs/lessons_recent.tsv`, `inputs/gog_functions.tsv`
  (GOG build names with class, function, callconv, param_size_bytes = Steam `ret N`; addresses are GOG, NOT Steam),
  `inputs/gog_globals.tsv` (GOG global pointer addresses), `inputs/steam_functions.tsv`.
- Tools: grep/rg, Read, python3 for text crunching over these files. Do not modify anything under ROOT except your output file.
- **GOG -> Steam name mapper (read-only, no Ghidra):**
  `python3 /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ghidra_knowledge/gog_cluster_delta_map.py 'CLASS_REGEX' [...] --out ROOT/findings/<BATCH>_map_<class>.tsv`
  maps every GOG function of the matching classes to a Steam address by per-run delta voting (ret N == GOG param size)
  plus drift. `vote` rows are strong, `drift+0xNN` rows are hypotheses (ret N 0 drift rows weakest); confirm a mapped
  row by reading its `decomp/` file (ret N, callees, strings) before relying on it. Fails on classes with odd GOG param
  sizes and very small classes. Run it BEFORE hunting a GOG-named function by hand; existing results are already in
  `inputs/name_map.tsv` (source `gog_cluster_delta_map.py`). Always pass `--out` (never let it write elsewhere).

## Known Steam engine globals (GOG global_pointers + 0x98A0, verified for resman at 0x5363A0)
`DAT_00a1b490` = CExoResMan* (g_pExoResMan), `DAT_00a1b49c` = GUI manager, `DAT_00a1b4a0` = Aurora, `DAT_00a1b4a4` =
CAppManager* (->+4 client, ->+8 server), `DAT_00a1b4a8` = script VM, `DAT_00a1b4b0` = TLK table, `DAT_00a1b4d0` = CSWRules,
`DAT_00a1b4e0` = window-active flag, `DAT_00a1b710` frame-limit float, `DAT_00a806c8` = _crtheap, `DAT_00a1bb84` = max
texture id. Operator delete is expected at 0x9196FD (verify).

## Output: exactly one file `ROOT/findings/<BATCH>.md` with these sections
(a) **Function table**: `| addr | proposed name | confidence (high/med/low) | one-line purpose | evidence (quoted line + address) |`
(b) **Findings**: each a bug / leak / cost / hook point with exact addresses and the quoted code lines (decompile AND the
    relevant .asm lines for anything that would become a hook or patch: give the instruction bytes).
(c) **Runtime measurements** that would confirm each finding: counter to add, hook address (instruction boundary, >= 5
    bytes, no relative branch/call inside the stolen bytes unless noted), `original_bytes`, expected signal.
(d) **Open questions**.

## Rules
- Every claim cites an address and a quoted decompile or asm line. Anything not directly shown is labelled `speculative`.
- Ghidra decompiles of this MSVC build use `FUN_xxxxxxxx`/`DAT_xxxxxxxx`; `this` is `param_1` or `in_ECX` for thiscall.
- Prefer quoting a few precise lines over pasting whole functions. Keep the file under ~600 lines.
- Budget your reading: use grep over `decomp/` to find callers/users first, then read only the functions you need.
- Finish with a 5-line summary at the top of the file (most important findings first).

## Verified facts from wave 1 (Opus-checked; build on these, do not re-derive)
- 194.8 MB live at census a2 0x712FB7 = CExoResMan::Malloc `size+10` branch (CRes+0x24!=0), set only by CResTPC (0x945CE0),
  CResTGA (0x9452C0), CResPLT: i.e. **texture data**; the TPC load path keeps a Demand reference (not evictable).
- Refcount CRes+4: ++ only in Demand 0x711C20, -- only in Release 0x7130B0. FreeChunk evicts refcount-0 only.
- Async resource I/O is live: thread proc 0x73D060 loops { 0x73D1A0 (fread, then sets [0xA7EA8C+0xC]=[0xA7EA98]=1) ;
  SuspendThread(self) }. Demand's `while(!FUN_00739a10()) Sleep(1)` at 0x711D18-0x711D2F waits on that flag.
- CERFFile::WriteResource_2 0x721BB0 does NOT leak (holder at [EBP-0x1C] frees [EBP-0x14] via 0x722670). SL refuted.
- 0x5122E0 = binary MDL/MDX loader (model blobs 6.6-12.9 MB, owner model cache DAT_00a1ffa8).
- Grass double free (K1 GrassMemorySafety) absent in K2: CAurTriangleBin dtor 0x50C7A0 / DestroyGrassPolys 0x50DA00;
  aliasing [+0x3C]=[+0x38] is always zeroed before return. Mesh buckets = std::map<texid, vector*> (insert 0x4946B0 via
  map find-or-insert 0x472630, vector push 0x473F40 doubling from 16): no fixed-size array.
- Render-pass bucket maps 0x465F90/0x4662E0/0x466700 allocate ~7.6 map nodes per frame (32 B) -> per-frame churn.
- Allocators: operator new 0x919723, new[] thunk 0x91D288, delete 0x9196FD (-> _free), delete[] thunk 0x91C6AA.
  `__security_check_cookie` = 0x9196EE. alloc_sites.tsv now includes new[] (operator_new_arr rows).
- Some census a2 values are stale/mid-instruction (0x58444D, 0x45FFAB, 0x561445, 0x739981): cross-check a1.

## Phase 3 questions (2026-09-30; batches T1-T4)
- Output file `ROOT/findings/T<n>.md` only. Read `findings/R1..R9.md` for context first (R3 = textures, R1 = resman, R4 = saves, R7 = frame loop/movie) and do not re-derive what they verified.
- Every verdict must be CONFIRMED / REFUTED / SPECULATIVE with the quoted evidence; an absent writer in a grep is NOT proof of no writer (lesson 168: check indirect writes via `this+off` and asm `[reg + off]`).
- Indirect (vtable) callers: grep the .c files for `(**(code **)(*<var> + 0x<slot>))(` AND the .asm files for `CALL dword ptr [<reg> + 0x<slot>]`; decide the object's class from where the pointer comes from (quote it).
- T1 must end with a one-line TX verdict: SAFE / SAFE WITH GUARD <guard> / UNSAFE <path>.

## Phase 4 questions (2026-09-30 evening; batches U1-U6)
- Output file `ROOT/findings/U<n>.md` only (plus mapper TSVs `ROOT/findings/U<n>_map_<class>.tsv` via `--out`). Read `findings/R*.md` and `findings/T*.md` that touch your subsystem first; do not re-derive verified facts.
- `inputs/name_map.tsv`, `inputs/db_functions.tsv`, `inputs/candidates.tsv`, `inputs/lessons_recent.tsv` were re-exported from the DB on 2026-09-30 19:50 (include T1-T4 rows).
- **Runtime data exists now** (`inputs/runA_perflab_metrics.txt`, perf-lab 0.2.0, 2 launches, 3 loads incl. one module change, ~35 min in-game; line formats in the file header lines `#METRICS`/`RT`/...): load 4.7 s; TPC resident 193.5 MB; C2 Demand wait = 0 (no Demand ever hit an in-flight async CRes); glFinish 1.3% of load; gluBuild2DMipmaps ~12% of load (574 ms / 52 calls); 0 evictions, 1 re-demand per load; GetImage after upload by non-upload callers = 0; render bucket 0x472630 ~225 calls/frame (max 972); action bar 0x74CFE0 ~1/frame; module change drops the live heap to 7 MB then reloads (no gross cross-module leak). Use it to rank costs; say when a static claim contradicts it.
- **Every batch answers the same four axes**, each as its own subsection in (b): (1) leaks / unbounded growth (quote the add site AND the missing remove, or the remove that balances it); (2) per-frame / per-tick allocation and CPU churn (show the call chain from the frame loop 0x407920 -> MainLoop 0x781BE0 or the server tick; say per-frame vs per-event); (3) crash / robustness paths (NULL, unchecked sizes/indexes, data-triggered, resource exhaustion); (4) hook points for runtime counters (instruction boundary, >= 5 stolen bytes, no relative CALL/JMP/Jcc inside and no branch target inside the stolen range, `original_bytes` copied from the .asm, parameter sources as `ecx` / `[esp+N]` at the hook / `[ebp-N]`/`[ebp+N]`).
- Verdict words: CONFIRMED / REFUTED / SPECULATIVE, as in Phase 3. Rank findings by expected impact (memory MB, ms per frame/load, crash likelihood).
- Mapped GOG names are hypotheses until the decompile (ret N, callees, strings) matches: say which you confirmed.

## Phase 6 questions (2026-10-02; batches W1-W7; the V1-V7 names belong to another session's doc 15 work)
- Output file `ROOT/findings/W<n>.md` only. Read the earlier findings your brief names first; do not re-derive verified facts.
- Goal = **patch-ready fix designs** for robustness items and two perf items. For EVERY proposed patch site give:
  (1) the exact .asm lines (address, bytes, instruction) from 16 bytes before to 16 bytes after the site;
  (2) a whole-dump scan for branch/call targets inside the bytes you would overwrite: `grep -rE "(J[A-Z]+|CALL|JMP) +0x0*<addr>" decomp/*/*.asm` for every
      address inside the stolen range (quote the command and the result, even if empty), plus jump tables if the function has a switch;
  (3) the new bytes (hand-encoded; show the encoding: opcode, ModRM, rel32 base = address of the NEXT instruction);
  (4) which registers/flags are live after the site (who reads them next) so the fix does not clobber anything;
  (5) reachability: the call chain from a real trigger (frame loop 0x407920 / MainLoop 0x781BE0 / server tick / options menu / load) with quoted call sites,
      and a verdict CONFIRMED reachable / REFUTED / SPECULATIVE;
  (6) the counter you would add (what the guarded path firing means) and a harness idea (fake objects, which fields).
- End the file with a per-item verdict line: `FIX <site(s)> <bytes>` / `DOCUMENT <reason>`.
