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
