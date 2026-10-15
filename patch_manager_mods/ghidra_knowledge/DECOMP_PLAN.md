# DECOMP plan: REA + static Ghidra dumps -> scripted decompile and documentation of swkotor2.exe (Steam)

Resume: `python3 scripts/plan_manager.py resume patch_manager_mods/ghidra_knowledge/DECOMP_PLAN.md`.
Handoff: `.claude/handoffs/DECOMP_PLAN.handoff.md`. Recon: `.claude/research/rea_decomp/SUMMARY.md` (9630412).

## Phases

| # | Phase | Status | Record |
|---|---|---|---|
| D0 | Plan doc + hygiene + ledger lint (`rea_ledger.py`, query_ghidra superseded flag, rea_questions sha gate/manifest/RUNNING, DESIGN 4.9) | DONE | spec §D0 |
| D1 | `rea_session.py` persistent REA MCP client + fake server + live probe (`.claude/research/rea_decomp/D1_probe.md`) | DONE | spec §D1 |
| D2 | `static_facts.py` ($0 facts, twins) + `batch.py cards --facts` + REVIEWER_BRIEF/DESIGN 4.10 | DONE | spec §D2 |
| D3 | `rea_facts.py` live-only facts (indirect targets, body ranges, data xrefs) + `rea_questions --residual`; G3 on 46 R1 addrs | PARTIAL | spec §D3 |
| D4 | `named_decomp.py` named decompile; G4 CSWCCreature | TODO (gated on D1) | spec §D4 |
| D5 | `fn_doc.py` + `persist_purposes.py`; CSWCCreature page + KDB purposes | TODO | spec §D5 |
| D6 | R2 rebuilt (`--arm facts|facts_rea`), scored; MASTER_PLAN R2/R3 | TODO | spec §D6 |
| D7 | rea-recon agent v2 + `test_agent_copy.py`; live check on 0x85d260/2d0/320/3d0 | TODO | spec §D7 |
| D8 | Docs sweep: every new script listed (free/fast first, REA last) in all agent-facing docs; `check_skills.py` + a grep that each tool name appears in each doc | TODO | Decisions: docs rule |

## Cost log

| phase | start (ISO) | agents | est. cost | note |
|---|---|---|---|---|
| D0 | 2026-10-15T18:44 | 3 Haiku, 2 Sonnet | $0.60 | T0.1 Haiku peaked 116k ctx; lint baseline 53 (L2 26, L6 3, L8 24); suites 89/314/13 OK |
| D1+D2 | 2026-10-15T19:00 | 4 Haiku, 5 Sonnet | ~$1.6 | 2 live REA probes (~11 min, $0 tokens); suites 196 (ghidra tools) / 320 (steam tools) OK |
| D3 | 2026-10-15T19:40 | 1 Sonnet (V3) | $0.39 | G3 live 46 addrs -> 98 rows (52 callsite, 4 function, 42 xref), lint 53 (+0), exe sha unchanged; suites 221/328 OK |

## Decisions (Opus)

- **Docs rule (user, 10-15):** every phase ends with a docs step before mark-done. Each new script gets a one-line
  entry (command, what it answers, cost $0, speed) in: `CLAUDE.md` "Fast Local Tools", `ghidra_knowledge/RE_AGENT_GUIDE.md`
  section 2 (tool order: static scripts first, live REA only for indirect targets / body ranges / data xrefs / named
  decompile), `ghidra_knowledge/README.md` "Querying", `SUBAGENT_ROLES.md` (REA routing), and from D7 the rea-recon
  agent. D8 sweeps all of them plus skills that mention `query_ghidra.py`/`cg.py`/rea-recon
  (`.claude/skills/openkotor-ghidra-server`, `kotor2-log-analysis`, `kotor2-crash-triage`, `kotor2-address-db`).
- **D1 gates (probe 10-15, `.claude/research/rea_decomp/D1_probe.md`):** cold first analysis call ~315 s per session,
  then median 0.01 s per call -> D3/D4 GO. `ReaSession.open()` runs a warm call under `open_timeout_s`.
  Results are envelopes: read `normalized_result`.
- **D3 redesign:** `resolve_native_call_targets` returns `unresolved` for vtable calls -> step 1 uses the static slot
  (`static_facts.indirect_slots`) + `class_methods` slot candidates ($0, `callsite` rows `probable`; `confirmed` only
  with exactly one candidate whose class matches the caller's class). Step 2 body ranges via `analyze_function`
  (`normalized_result.procedure.body.ranges`). Step 3 `xrefs {address: <fn>}` for DATA refs to the function
  (vtable/table slots), written as `xref`/`table` rows.
- **D4 naming:** `set_addresses_names`/`set_address_name` are rejected by the Ghidra provider; use per-address
  `annotate_native_function {procedure, name}` (0.13 s each; visible in pseudo-C and batch_decompile). Post-pass 3b
  only for addresses whose annotate call fails.
- **T0.3 manifest:** `q_manifest.tsv` holds `topic n_addr sha_of_list` only; no kind counts (blindness).
- **D1 read-only (from rea-agents 6.3.0 source):** `open_binary` is a strict schema `{path, format?, provider_id?, snapshot_path?}`;
  there is no `read_only` input. REA always imports with `-readOnly -deleteProject` (ephemeral project), so in-session
  names/annotations never touch disk. `ReaSession(read_only=True)` is therefore a CLIENT guard: it refuses `set_*`,
  `annotate_*` and other write tools. `set_addresses_names` input is `{names: {"0x..": name}}`, output per-address bool.
  MCP response budget floor is 10 MiB (`REA_MCP_MAX_RESPONSE_BYTES`).
- **T0.3 DESIGN:** live exe label stays md5 `62411a3a` (verified with md5sum 10-15; Haiku had relabelled it sha256).

## Spec (approved plan, verbatim)

## Context
- **User request (10-15):**
  - Combine REA with our own Ghidra tooling so we can decompile swkotor2.exe and keep documenting it.
  - Haiku 5.5 swarms do the first pass of each task; Sonnet validates and fixes their output; Opus (lead) has the final word.
  - Full test coverage for everything built.
- **What the recon found** (commit 9630412, `.claude/research/rea_decomp/SUMMARY.md`; Opus-verified):
  1. **Our static dumps are complete.** `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` covers 100% of real
     code: 12,355 `.asm` + `.c` pairs, plus `callgraph.tsv`, `callers_count.tsv`, `strings.tsv`, `globals.tsv` and
     `imports.tsv`.
     - H4's "45.8% hole" is wrong: the gap is 14,589 `Unwind@` SEH funclets, 11 bytes each.
     - These dumps match the REA ledger on 955 of 958 checks: size, RET, callers, callees, strings and function starts.
     - So the 6 fixed REA questions only repeat the dumps. Haiku+REA cost $0.047/card for facts we already had at $0.
  2. **Only live Ghidra adds** four things:
     - the targets of the 12,283 INDIRECT call sites (vtable dispatch; `callgraph.tsv` has callee `-`);
     - non-contiguous function bodies (0x7680a0 has 3 ranges, with its RET past the contiguous size);
     - live data xrefs;
     - named pseudo-C, through in-session annotation.
  3. **The REA CLI is too slow per address.** Each new address or subcommand re-imports the exe: about 5 min and
     3.8 GB RSS. `--snapshot` only caches identical requests.
     - The REA MCP server keeps one JVM per process: `open_binary` pays the cold start once, then requests run one at
       a time.
     - So a stdlib Python MCP stdio client that holds one session gives 0-token, deterministic live facts.
     - The `mcp` Python package is not installed, so the client is stdlib only. Node 22.23 is present.
  4. **`annotate` changes vanish when the session closes.** Annotate (and `set_addresses_names`) works only inside one
     session. Read-only sessions drop annotate.
  5. **Hygiene gaps (H7):**
     - No sha256 check on the exe. DESIGN 4.9 says "md5 6a522e71", but that is a sha256 prefix.
     - DESIGN allows 12 parallel REA runs; the measured safe maximum is 2-3.
     - The process guard misses REA's `GhidraClassLoader` JVM.
     - Ledger: 49 rows use kinds not in the README list, 31 rows have no asm quote, and confirmed and refuted rows
       conflict with nothing marking which one wins.
     - `load_ledger` silently truncates rows with more than 7 cells.
     - rea-recon writes its ledger only at the end of a run.
     - rea-recon greps the ledger before analysing, which leaks earlier names into blind runs.
  6. **Coverage today:** the Steam DB names 3,420 of the 12,355 real functions (about 28%). KDB
     `functions.purpose` is filled for 680 Steam rows. There is no Steam signature store and no function-to-refs table.
- **Outcome:**
  - Static facts move to $0 scripts.
  - REA gets a scripted persistent session, used only for live-only facts and named decompile.
  - A per-class documentation pipeline writes pages to the repo, and validated purpose text goes into the KDB.
  - The R2 re-pilot is rebuilt on the new evidence.
  - Every tool gets unit tests and fixture tests. A fake MCP server means REA is never needed in tests.

## 0. Preserved context and rules (post-clear)
- **Plan doc (new, created in D0):** `patch_manager_mods/ghidra_knowledge/DECOMP_PLAN.md`.
  - Phase table `| # | Phase | Status | Record |`, rows D0-D7.
  - Resume with `python3 scripts/plan_manager.py resume patch_manager_mods/ghidra_knowledge/DECOMP_PLAN.md`.
  - Mark rows with `plan_manager.py mark-done D<n> --commit <sha>`. Handoff file: `.claude/handoffs/DECOMP_PLAN.handoff.md`.
  - The Steam DB `MASTER_PLAN.md` row R2 is rewritten in D6 to point at this plan.
- **Exe (read-only):** `~/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe`.
  - Full sha256: `6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef`.
  - Never use the game folder: the live exe is vanilla plus the 4 GB patch.
- **REA environment:**
  - `GHIDRA_INSTALL_DIR=/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC`
  - `JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64`
  - `REA_GHIDRA_STARTUP_TIMEOUT_MS=900000`
  - command `npx -y rea-agents@6.3.0 mcp`
- **Dump root:** `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/`.
  - Each `.c` file starts with a header like `// FN 00776db0 FUN_00776db0 size=84 ret=4 callers=a,b callees=`.
  - TSV headers:
    - `callgraph.tsv`: `caller_fn call_site callee_fn type insn` (type is CALL, TAILJMP, IMPORT or INDIRECT; addresses
      are unprefixed `%08x`).
    - `strings.tsv`: `addr string ref_count functions` (`fn:name` entries, comma-separated).
    - `globals.tsv`: `global target_addr site reftype fn fn_name insn`.
    - `imports.tsv`: `import library calling_fn fn_name site reftype insn`.
- **Code to reuse, not rewrite:**
  - `patch_manager_mods/engine_ai_atlas/tools/cg.py`: callers, callees, path, indirect `--slot`.
  - `patch_manager_mods/engine_ai_atlas/tools/atlas_common.py`: `resolve_indirect` (slot recovery from the asm).
  - `scripts/query_ghidra.py` `load_ledger` (L26-43).
  - `patch_manager_mods/ghidra_knowledge/tools/re_card_prep.py`: `annotate_text`, `anchors`, `prologue`.
  - `steam_db_batches/tools/batch.py`: `make_card` L1032, `load_card_facts` L997 (the blind per-address channel
    pattern), `load_rea_rows` L1010, `cmd_cards` L1164.
- **KDB schema** (`kotor2_ghidra_knowledge.db`):
  - `functions(binary_hash, address, symbol, size, purpose, notes, discovered_date, PK(binary_hash, address))`, with
    the address unprefixed.
  - `name_map(steam_addr '0x%08x' PK, name, source, confidence, evidence, date)`.
- **Hard rules:**
  - Only the lead writes DBs (KDB or Steam DB). Agents write files only inside their Write Scope.
  - Never edit `*_src`, the GOG DB or the game folder.
  - Commit with `git commit -- <paths>`. Never `-a` or `add -A`. Never stage the KDB binary unless a phase says so;
    D5 does, and the lead does it.
  - Run no more than 2 live REA sessions at once, counting other sessions' REA. Check with `rea_session.py guard`
    before each one.
  - No `rea-row-verify` on DB rows. REA evidence stays report-only: no trust route, no change to the AUTO rule.
  - Blindness: real and decoy card addresses get identical treatment.
  - Run long REA jobs with `run_in_background` plus `python -u | tee`, and wait for the notification. Never poll.

## 1. Agent model for every phase (waves)
```
W  Haiku workers (parallel, disjoint Write Scopes, <=3 files each, target <60k ctx, brief = spec section verbatim
   + `agent_contract.py print worker`)  ->  worker 5-line reply
V  Sonnet validator per worker (or per related pair): reads the spec section + diff, runs the scoped tests, fixes
   defects directly (any size), adds missing test cases from the matrix; reply = worker schema with NOTES listing
   each fix (file:line, why)
G  Opus lead gate: full suites, 2 targeted spot-checks per tool, live REA runs, DB writes, commit, mark-done
```
- **Before W:** run `python3 scripts/agent_contract.py snapshot`.
- **After W and V:** run `agent_contract.py check --scope <globs> -` for each reply.
- **Escalation:** a Haiku FAIL gets one Haiku retry with the failure excerpt. A second FAIL goes to the Sonnet
  validator, which owns the task. If Sonnet cannot get green, Opus takes it. Opus decides any spec dispute and
  records the decision in the plan doc.
- **Recon lesson:** all 7 Haiku recon agents went over 100k context. Briefs therefore inline the exact signatures,
  headers and fixture rows from this plan, and forbid repo-wide reads ("read only the files named in your brief").
- **Cost check per phase:** `python3 scripts/agent_usage.py --since <phase start>`, logged in the plan doc row.

## 2. Phases

### D0: plan doc + hygiene + ledger lint ($0 live; 3 Haiku, 2 Sonnet)
**T0.1 `rea_ledger.py` (new)**
- **Contract:** worker.
- **Scope:**
  - `patch_manager_mods/ghidra_knowledge/tools/rea_ledger.py`
  - `.../tools/tests/test_rea_ledger.py`
  - `.../rea_findings/README.md`
- **API:**
  - `COLS = ("addr","kind","name","confidence","evidence","topic","date")`
  - `KINDS = {"function","global","vtable","string","field","callsite","xref","data","table"}`
  - `CONF = {"confirmed","probable","refuted"}`
  - `read_rows(dir, strict=False) -> list[dict]`: in strict mode, any row with a cell count other than 7 raises
    `LedgerError(file, line)`.
  - `lint(dir) -> list[Issue]`, with these rules:
    - L1: kind is in KINDS.
    - L2: function, global, vtable, string and field rows have evidence matching `0x[0-9a-f]{6,8}: \S`.
    - L4: exactly 7 cells.
    - L5: addr is hex; date is ISO.
    - L6: the same addr and kind has both a confirmed row and a refuted row with no `supersedes:` marker in the
      evidence.
    - L8: every `function` row in a `<B>-q*` topic has a sibling `xref` row.
  - `append_rows(dir, file, rows)`: atomic append (temp file plus rename of the whole file). It refuses a row that
    fails lint and never rewrites existing lines.
  - `dedupe(rows)`: one row per key `(addr, kind, name)`. A later refuted row hides an earlier confirmed one and is
    flagged `superseded`.
  - CLI: `rea_ledger.py lint [--dir D] [--strict]`. Exit 0 if clean; exit 1 if there are issues, printed as
    `file:line rule msg`.
- **README:** add the kinds `xref`, `data` and `table` with their evidence formats, the `supersedes: <file>:<line>`
  convention, and "scripted topics `<B>-s<k>`, written only by `rea_session.py` tools".
- **Tests:**
  - every rule has a pass fixture and a fail fixture;
  - an extra tab raises in strict mode;
  - `append_rows` is atomic (a simulated crash mid-write leaves the original file intact) and rejects a bad row;
  - `dedupe` handles 0x6f5b40-style conflicts;
  - the real ledger dir lints to a known baseline count, recorded and not required to be zero, plus no new L4/L5
    issues.

**T0.2 query_ghidra integration**
- **Scope:** `scripts/query_ghidra.py`, `scripts/test_query_ghidra.py`.
- **Changes:**
  - `load_ledger` keeps its signature. It gains `strict=False` and a `superseded` flag, using `rea_ledger.dedupe` via a
    path import with a fallback to the current logic if the import fails.
  - Text output adds ` [superseded]`.
  - `stats` adds `rea_findings_issues` (the lint count).
- **Tests:**
  - the 9 existing tests stay green;
  - new tests: a superseded row is shown flagged; a malformed row with more than 7 cells is still skipped in
    non-strict mode and counted.

**T0.3 rea_questions.py and DESIGN fixes**
- **Scope:**
  - `steam_db_batches/tools/rea_questions.py`
  - `.../tests/test_rea_questions.py`
  - `steam_db_batches/DESIGN.md` (section 4.9 and section 5 lines 320/322/332 only)
- **rea_questions.py:**
  - Add `EXE_SHA256` (the full digest) and `preflight_exe(path)`. If the sha256 does not match, or the path contains
    `steamapps`, exit 2.
  - Add a `--skip-exe-check` option for tests only, documented as such.
  - Write a composition line in each q file (`addresses: N`, no kinds). Pin the kinds counts in a sibling
    `q_manifest.tsv` with `topic n_addr sha_of_list`; the counts are not printed in the q file.
  - Refuse (exit 2) when `B<n>/rea/RUNNING` exists. That marker is written by the lead before launching agents and
    removed after.
- **DESIGN 4.9:**
  - "md5" becomes "sha256 prefix".
  - "<= 12 parallel" becomes "<= 2 live REA sessions machine-wide".
  - The guard line becomes `rea_session.py guard` (it matches `analyzeHeadless|GhidraClassLoader|rea-agents`).
- **Tests:**
  - a wrong digest gives exit 2;
  - a `steamapps` path gives exit 2;
  - the RUNNING marker gives exit 2 and the files are kept;
  - the manifest is deterministic;
  - the existing 11 tests stay green, using `--skip-exe-check` with temp fixtures.

**T0.4 (lead)**
- Create `DECOMP_PLAN.md`: the phase table D0-D7, the rules from section 0, and the cost log.
- Then run G: `cd patch_manager_mods/ghidra_knowledge/tools && python3 -m unittest discover -s tests`, the
  steam_db_batches suite (303 tests plus new ones), and `python3 -m unittest scripts.test_query_ghidra`.

### D1: `rea_session.py`, a persistent REA MCP client (2 Haiku, 1 Sonnet, 1 live probe by the lead)
**T1.1 fake server (test fixture first)**
- **Scope:** `patch_manager_mods/ghidra_knowledge/tools/tests/fake_rea_mcp.py`, `.../tests/fixtures/rea/*.json`.
- **Behaviour:** stdlib, newline-delimited JSON-RPC 2.0 on stdio.
  - `initialize` returns `{protocolVersion, capabilities:{tools:{}}, serverInfo}`.
  - `tools/list` returns the 16 rea-recon tools plus `resolve_native_call_targets` and `set_addresses_names`.
  - `tools/call` returns `{content:[{type:"text", text:<json>}]}` from `fixtures/rea/<tool>__<addr>.json`, or
    `isError` for an unknown address.
  - Modes from `FAKE_REA_MODE`: `crash_after:N` (exit after N calls), `slow:S`, `no_init_reply`,
    `require_open` (tools before `open_binary` return the "No app is open" error), `garbage_line` (emits one non-JSON
    line first).
- **Fixtures:** cut from H1's real outputs in the scratchpad (`cold.json` for 0x776db0, `warm.json` for 0x7680a0),
  trimmed to the keys used, stored in the repo as small JSON files under 20 KB each.

**T1.2 client**
- **Scope:** `patch_manager_mods/ghidra_knowledge/tools/rea_session.py`, `.../tests/test_rea_session.py`.
- **API (stdlib only):**
  ```python
  EXE = Path.home()/"Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe"
  EXE_SHA256 = "6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef"
  REA_CMD = ["npx", "-y", "rea-agents@6.3.0", "mcp"]
  class ReaError(Exception); class ReaGuardError(ReaError)
  def preflight(exe=EXE, sha=EXE_SHA256) -> None          # digest, no 'steamapps', is_file
  def live_rea_procs(ps_lines=None) -> list[str]          # matches analyzeHeadless|GhidraClassLoader|rea-agents
  def guard(max_live=2, ps_lines=None) -> None            # raises ReaGuardError when >= max_live
  class ReaSession:                                       # context manager
      def __init__(self, exe=EXE, *, cmd=REA_CMD, env=None, call_timeout_s=120, open_timeout_s=900,
                   read_only=True, cache_dir=None, lock_dir=Path.home()/".cache/kotor2_rea")
      def start(self)            # flock lock_dir/<sha12>.lock (non-blocking -> ReaGuardError), spawn, initialize,
                                 # notifications/initialized, tools/list (records available tool names)
      def open(self)             # open_binary {path, provider_id:"ghidra", read_only}; times it
      def call(self, tool, args, *, timeout_s=None, use_cache=True) -> dict
                                 # JSON result; cache key sha1(tool+canonical args) -> cache_dir/<tool>/<key>.json
                                 # on EOF/BrokenPipe: one reconnect (start+open) then retry once, else ReaError
      def close(self)            # close_binary (never passes snapshot_path), terminate, release lock
  def telemetry_row(...)         # appends cache_dir/runs.tsv: ts tool addr wall_s ok rss_kb(/proc/<pid>/status VmHWM)
  ```
- **CLI:**
  - `rea_session.py guard`
  - `rea_session.py probe [--addr 0x776db0 ...]`: open, then for each address `procedure_info` + `procedure_assembly`
    + `procedure_callers`, printing timings.
  - `rea_session.py call <tool> --args '<json>'`
- **Cache dir:** `<dump root>/rea_cache/` by default, outside git.
- **Tests (all against the fake server):**
  - the handshake order is exact;
  - responses are matched by id when a stale line arrives;
  - `garbage_line` is skipped with a warning;
  - the call timeout raises;
  - `crash_after:1` triggers one reconnect and the call succeeds; `crash_after:0` twice raises ReaError;
  - `require_open` gives a clear error if `call` happens before `open`;
  - a cache hit makes no server call;
  - the lock is exclusive (a second session raises ReaGuardError) and is released after `close` even on an
    exception;
  - `guard` with fixture ps lines (2 REA JVMs, or 1 analyzeHeadless plus 1 rea-agents) raises; 1 does not;
  - `preflight` rejects a wrong digest, a `steamapps` path and a missing file;
  - `close` never sends `snapshot_path`;
  - the telemetry row is well-formed;
  - the env includes the 3 REA variables.

**T1.3 live probe (lead, background, about 10 min, read-only)**
- Run `rea_session.py guard && rea_session.py probe --addr 0x776db0 0x7680a0 0x775930 0x775950`.
- Then in the same session:
  1. `batch_decompile` with 20 addresses;
  2. `resolve_native_call_targets` on 3 INDIRECT sites from `callgraph.tsv`;
  3. a second session with `read_only=false` that runs `set_addresses_names` on 0x85d320 = "ProbeName" and then
     `procedure_pseudo_code` on 0x775970.
- Afterwards, check that the exe sha256 is unchanged and that no files were written next to the exe.
- **Record in `.claude/research/rea_decomp/D1_probe.md`:**
  - open cold time;
  - median and p95 per-call time;
  - VmHWM;
  - the batch response size and limit;
  - resolver output quality, compared with `atlas_common.resolve_indirect` slot guesses;
  - whether the annotation shows up in the pseudo-C.
- **Decision gates (Opus):**
  - per-call median 15 s or less: D3/D4 go ahead on REA;
  - otherwise D4 switches to the fallback in D4-alt;
  - annotate not visible: D4 drops annotation and puts names in a post-pass (D4 step 3b).

### D2: static facts ($0) and `cards --facts` (3 Haiku, 2 Sonnet)
**T2.1 `static_facts.py`**
- **Scope:** `patch_manager_mods/ghidra_knowledge/tools/static_facts.py`, `.../tests/test_static_facts.py`,
  `.../tests/fixtures/dump_mini/`.
- **API:**
  ```python
  class Dump:                                   # lazy-loads TSVs once; root default = dump root
      def __init__(self, root=DUMP_ROOT, names: dict[int,str]|None=None)
      def callers(self, a) -> list[tuple[int,int]]       # (call_site, caller_fn), CALL+TAILJMP
      def callees(self, a) -> list[tuple[int,int|None,str]]  # (site, target|None, type); INDIRECT -> slot via
                                                          # atlas_common.resolve_indirect (import by path)
      def strings(self, a) -> list[tuple[int,str]]; def globals_(self, a); def imports(self, a)
      def size_ret(self, a) -> tuple[int,int]            # steam_functions.tsv
      def asm(self, a) -> list[str]                      # decomp/<bb>/<addr>.asm
      def shape_hash(self, a) -> str                     # sha1 of asm mnemonics + operands with imm/addr masked
      def twins(self, a, window=0x400) -> list[int]      # same size, RET and shape_hash within window, excl. a
      def distinguishing(self, a, twins) -> list[str]    # per twin: first differing callee/imm/string, asm line quoted
  def load_names(steam_db=STEAM_DB, kdb=KDB, exclude_sources=()) -> dict[int,str]
                                                          # Steam DB functions (ro) + name_map confidence high,
                                                          # skipping rows whose source is in exclude_sources
  def facts(dump, a, *, exclude_addrs=()) -> dict        # size, ret, callers(n, first5 named), callees named,
                                                          # indirect slots, strings, globals, imports, twins
  def card_lines(f: dict, max_lines=10) -> list[str]     # "- callers 3: 0x00865250 Foo::Bar, ..." etc.
  ```
- **CLI:** `static_facts.py addr <hex> [--json]`, `static_facts.py twins <hex>`.
- **Blindness rules (tested):**
  - the card's own address never shows a name;
  - `exclude_sources` drops the current batch's own `name_map` staging (`source` starting with `B<n>`);
  - real and decoy cards use the same code path.
- **Tests (fixture `dump_mini/`, about 10 functions):**
  - callers and callees are correct, and INDIRECT edges get a slot;
  - a twin family of 4 forwarders in the 0x775930 style is found, and `distinguishing` quotes the differing CALL
    line for each;
  - a non-twin of the same size with a different shape is excluded;
  - the excluded name does not appear;
  - a missing dump gives `facts['asm']=None` and does not raise;
  - `card_lines` is deterministic and respects the cap;
  - a gold test on the real dump (skipped if the dump root is absent): 0x776db0 has callers
    {00865250, 007b3050, 008652c0}, size 84, RET 4.

**T2.2 `batch.py cards --facts`**
- **Scope:** `steam_db_batches/tools/batch.py` (`make_card`, `cmd_cards`, the parser), `.../tests/test_facts_cards.py`.
- **Changes:**
  - A `--facts` flag appends `### Static facts (script output, report-only)` from `static_facts.card_lines` at `ce.eff`,
    plus a `twins:` line for every card (real and decoy).
  - The `name_map` source of the current batch is excluded.
  - Without the flag the output is byte-identical (test with a golden hash of the fixture batch).
- **Tests:**
  - the same section shape for real and decoy;
  - the flag off gives byte-identical output;
  - `--facts --rea` together keep the order Static, then REA;
  - the full suite stays green.

**T2.3 `REVIEWER_BRIEF.md` + DESIGN 4.10 (docs)**
- **Scope:** the two files, one section each.
- **REVIEWER_BRIEF:**
  - static facts count as anchors when they quote an asm line;
  - a `twins:` line means: for a match, the reviewer must cite the distinguishing anchor for the candidate, otherwise
    answer `unsure`.
- **DESIGN 4.10 "Static facts":** source files, blindness, no trust route.

### D3: live-only facts via the session (1 Haiku, 1 Sonnet, live run by the lead)
**T3.1 `rea_facts.py`**
- **Scope:** `patch_manager_mods/ghidra_knowledge/tools/rea_facts.py`, `.../tests/test_rea_facts.py`.
- **Flow:** `rea_facts.py run --addrs FILE|--keys B<n>/cards/key*.tsv --topic <B>-s<k> [--max-sites 8]`. For each
  address, sorted and deduped (blind, with no kind column read):
  1. **INDIRECT sites** (from `Dump.callees`, type INDIRECT; at most `--max-sites`): call
     `resolve_native_call_targets {address: site}`.
     - Write a `callsite` row with evidence `resolve_native_call_targets 0xSITE: <insn> -> 0xT1[,0xT2]` and
       confidence `probable`.
     - Use `confirmed` only when there is exactly 1 target and the slot matches `class_methods`.
  2. **Body check:** when the REA `procedure_info` body span is larger than the static size, write a `function` row
     with `body ranges a-b;c-d` and the last-RET asm line.
  3. **Data xrefs:** call `xrefs` for each global the function writes (from `globals.tsv`, at most 4) and write
     `xref` rows.
- Output goes through `rea_ledger.append_rows` and is flushed after every address, so a crash keeps the finished
  ones. A resume run skips addresses that already have rows for that topic.
- **Tests (fake server plus `dump_mini`):**
  - rows are lint-clean;
  - a crash after 2 addresses leaves 2 addresses' rows, and a re-run completes without duplicates;
  - the per-site cap is respected;
  - there is no kind or GOG leakage, because the input reader ignores those columns;
  - topic collisions are refused;
  - output is deterministic.

**T3.2 rea_questions.py residual mode**
- **Scope:** `rea_questions.py` plus its test.
- `--residual` drops the 6 fixed questions and asks only "semantic purpose in at most 15 words, with a quoted asm
  line" plus "anything the scripted facts could not resolve". These go to rea-recon runs only for addresses the Opus
  gate selects.
- **Tests:** the question text, and no fixed questions in residual mode.

**G3 (lead):** run `rea_facts.py` on the 46 R1 pilot addresses (background). Spot-check 3 resolved indirect targets
against the asm and `class_methods`. Commit the ledger file.

### D4: named decompile (1 Haiku, 1 Sonnet, live run by the lead)
**T4.1 `named_decomp.py`**
- **Scope:** `patch_manager_mods/ghidra_knowledge/tools/named_decomp.py`, `.../tests/test_named_decomp.py`.
- **Flow:** `named_decomp.py run --addrs FILE|--class <Class> [--batch 20] [--out <dump root>/named_decomp]`.
  1. Load names with `static_facts.load_names()`, which is the Steam DB plus high-confidence name_map entries. These
     are documentation outputs, not review cards, so no blindness rule applies here.
  2. Open the session with `read_only=False`, which only touches REA's ephemeral copy. Call
     `set_addresses_names` in chunks of 200, or per-address `annotate_native_function` if the batch tool is missing.
  3. Call `batch_decompile` in chunks of `--batch`. Write `named_decomp/<bb>/<addr>.c` with the header
     `// NAMED <addr> <name> evidence=<evidence_id or session id> names=<n applied> exe=<sha12>`.
     - 3b, the fallback if D1 showed annotate does not show in the output: textual post-pass replacing
       `FUN_xxxxxxxx` and `DAT_` with known names, marked `names=postpass`.
  4. Write a manifest `named_decomp/manifest.tsv` with columns `addr name status bytes wall_s`. A resume skips `ok`
     rows. Check the exe sha256 before and after the run.
- **Tests (fake server):**
  - names are applied before decompile, in call order;
  - chunking is right;
  - per-item errors are recorded and do not abort;
  - the post-pass replacement does not touch substrings, using word boundaries (`FUN_0040a0` vs `FUN_0040a0b0`);
  - resume works;
  - the manifest is deterministic;
  - the sha mismatch check aborts.
- **D4-alt** (only if D1 gate fails): an Opus decision on a headless `ApplyNamesAndDump.java` against a copy of the
  local project `KOTOR2_SteamAspyr.rep`. That gets its own plan row and is not built speculatively.

**G4 (lead):** do the named decompile of one pilot class, CSWCCreature: about 177 Steam DB rows, from the B13a
verified rows. Record the cost per 100 functions. Output stays in the dump dir, outside git.

### D5: documentation pages + purposes (Haiku drafts, Sonnet validates, lead persists)
**T5.1 `fn_doc.py`**
- **Scope:** `patch_manager_mods/ghidra_knowledge/tools/fn_doc.py`, `.../tests/test_fn_doc.py`.
- **`fn_doc.py page <Class> [--out patch_manager_mods/ghidra_knowledge/steam_docs/]`** writes `<Class>.md`
  deterministically. For each Steam DB function of the class, sorted by address, it prints:
  - address, name, calling convention, param bytes;
  - size and RET;
  - callers (first 5, named) and callees (named);
  - INDIRECT slots and resolved targets from the ledger `callsite` rows;
  - strings and globals;
  - `struct_fields` entries touched (KDB, matched by instruction address range);
  - `purpose` (KDB);
  - the first 30 lines of named pseudo-C, if present.
  - Pages over 400 KB split into `<Class>.<n>.md`.
- **`fn_doc.py draft-input <Class> --n 25`** writes Haiku worker inputs `steam_docs/_work/<Class>_<k>.md`: per
  function, the facts plus the named pseudo-C. Output TSV: `addr purpose anchor`.
- **`fn_doc.py check-drafts <tsv>`** is the deterministic check:
  - `anchor` must be an exact `0xADDR: TEXT` line found in that function's asm;
  - `purpose` is at most 20 words and has no address-only text;
  - output is `pass` or `fail` with a reason per row.
- **Tests:**
  - page determinism (golden file from `dump_mini` plus a temp KDB);
  - the split threshold;
  - anchor check pass, fail and normalisation (case, `dword ptr`);
  - the over-length purpose is rejected;
  - a missing named decompile is handled.

**T5.2 `persist_purposes.py`**
- **Scope:** `patch_manager_mods/ghidra_knowledge/tools/persist_purposes.py`, `.../tests/test_persist_purposes.py`.
- **Behaviour:**
  - Dry-run by default, `--apply` to write.
  - Upserts KDB `functions(binary_hash='6a522e71', address='%08x')`.
  - Sets `purpose` only where it is NULL or where the row came from an earlier `persist_purposes` run (marked in
    `notes`).
  - Never overwrites a human or atlas purpose.
  - A second `--apply` changes 0 rows.
  - Runs a `PRAGMA integrity_check`.
- **Tests:** a temp-copy KDB; idempotence; a protected row is not overwritten; dry-run writes nothing.

**Wave D5-run, for the pilot class:**
1. Haiku drafts, one worker per `_work` file of 25 functions, under 60k context.
2. `check-drafts`.
3. A Sonnet validator per class reviews every row and every `fail`. For each row it decides keep, fix or drop
   (semantics against the pseudo-C) and writes `<Class>_validated.tsv`.
4. The Opus gate spot-checks 5 rows. Then `persist_purposes.py --apply`, `fn_doc.py page`, and a commit of the page,
   the validated TSV and the KDB.
   - The KDB binary is staged explicitly in this phase only, after an integrity check and a backup copy to the
     scratchpad.

### D6: R2 rebuilt (Sonnet reviewers; scored)
- **T6.1 (Haiku):** `B13a/rea_pilot/build_pilot_cards.py` gets `--arm facts|facts_rea`. It builds the cards with
  `cards B13a --facts [--rea] --out cards_full_<arm>` and copies the section over the committed card text, in the
  same order and with the same seed. Tests: the arms differ only by their sections; ids and order are identical.
- **Run (lead):** `score_pilot.py` already globs `review/<arm>_*.tsv`; add the arms `sonnet+facts`,
  `sonnet+facts+rea` and `haiku+facts`.
  - 2 Sonnet reviewers per Sonnet arm and 2 Haiku reviewers for the cheap arm, all following REVIEWER_BRIEF.
  - **GO** (replaces the R2 bar): label_suspect flagged over 10/14, or at least 1 of the 4 Opus-only rows; 0 decoys
    accepted (B13a-1-53 included); under $0.08 per card.
  - If haiku+facts reaches 10/14 with 0 decoys, it becomes the cheap default.
- **Lead:** write `PILOT.md` section "R2". Rewrite the MASTER_PLAN R2 row as "DONE via DECOMP D6" and set R3 GO or
  NO-GO.

### D7: rea-recon agent v2 (1 Haiku, 1 Sonnet)
- **Scope:**
  - `patch_manager_mods/ghidra_knowledge/agents/rea-recon.md`, then `~/.claude/agents/rea-recon.md` (copied by the
    lead, which also keeps the two copies identical)
  - `patch_manager_mods/ghidra_knowledge/tools/tests/test_agent_copy.py`
- **Changes:**
  - Rule: "first run `static_facts.py addr` (via Read of a pre-generated facts file in the brief), never re-ask static
    facts".
  - Grep the ledger by address only and do not read prior `name` cells (B1).
  - Append ledger rows after each address (T3).
  - Allowlist: add `resolve_native_call_targets`, `find_xrefs_to_name`, `inspect_native_instruction` (L3 provenance).
  - Hard call budget of 3 per address plus 3.
  - The return line `LEDGER: <path> (<N> rows)`.
- **Tests:**
  - the tracked copy equals the home copy;
  - the frontmatter tool list contains no write tools (`annotate`, `set_*`, `goto_address`);
  - the env block has the 3 variables.
- **Live check (lead):** one rea-recon run on 4 residual addresses, the twin callees 0x85d260, 0x85d2d0, 0x85d320
  and 0x85d3d0 (gap A6). Target: under 60k tokens and lint-clean rows.

## 3. Test matrix summary (all stdlib unittest, fixtures in temp dirs; no REA, no network)
| tool | suite | key edge cases |
|---|---|---|
| rea_ledger.py | ghidra_knowledge/tools/tests/test_rea_ledger.py | L1-L8, strict cells, atomic append, supersede dedupe |
| query_ghidra.py | scripts/test_query_ghidra.py | superseded flag, malformed count, 9 legacy tests |
| rea_questions.py | steam_db_batches/tools/tests/test_rea_questions.py | sha gate, steamapps path, RUNNING marker, manifest, residual mode, 11 legacy tests |
| fake_rea_mcp.py | used by all live-tool tests | crash, slow, garbage line, require_open, no_init_reply |
| rea_session.py | test_rea_session.py | handshake, id matching, timeout, reconnect once, lock, guard, cache, no snapshot_path |
| static_facts.py | test_static_facts.py | INDIRECT slot, twins and distinguishing quote, blindness exclusions, missing dump, real-dump gold (skip if absent) |
| batch.py --facts | test_facts_cards.py | byte-identical off, real = decoy shape, order with --rea |
| rea_facts.py | test_rea_facts.py | per-address flush and resume, site cap, lint-clean, no kind leakage |
| named_decomp.py | test_named_decomp.py | names before decompile, word-boundary post-pass, per-item errors, sha before and after |
| fn_doc.py | test_fn_doc.py | golden page, split, anchor check normalisation, purpose length |
| persist_purposes.py | test_persist_purposes.py | dry-run, idempotent, protected purpose |
| agent copy | test_agent_copy.py | identical copies, no write tools, env |

## 4. Verification (lead, end of each phase and at the end)
- [ ] `cd patch_manager_mods/ghidra_knowledge/tools && python3 -m unittest discover -s tests` is green.
- [ ] `cd patch_manager_mods/steam_db_batches/tools && python3 -m unittest discover -s tests` is green (303 or more).
- [ ] `python3 -m unittest scripts.test_query_ghidra` is green.
- [ ] `python3 patch_manager_mods/ghidra_knowledge/tools/rea_ledger.py lint` reports no new issues over the D0
      baseline.
- [ ] `python3 tools/batch.py cards B13a` without the new flags is byte-identical to the pre-change output, via
      `BD_ROOT` on a scratch copy.
- [ ] D1 probe md has the timings and both gate decisions. D3 and D4 runs have their exe sha unchanged before and
      after, and the guard passed.
- [ ] D5: `steam_docs/CSWCCreature.md` is committed; KDB integrity is ok; the second persist `--apply` gives 0.
- [ ] D6: `PILOT.md` R2 table; MASTER_PLAN R2/R3 updated.
- [ ] Each phase: `plan_manager.py mark-done D<n> --commit <sha>`, the cost line, and the 4-section report.

## 5. Budget estimate
- Haiku workers: about 14 tasks at about $0.06, roughly $1.
- Sonnet validators: about 9 at about $0.40, roughly $3.6.
- D5 drafts: about 177 functions, so 8 Haiku at about $0.05, plus 1 Sonnet at about $0.6.
- D6 reviewers: 4 Sonnet at about $0.6 plus 2 Haiku, roughly $2.6.
- Live REA scripts cost $0 in tokens, about 60 min of wall time in total.
- **Total: about $9.**
