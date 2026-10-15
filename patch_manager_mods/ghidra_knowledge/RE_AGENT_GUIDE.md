# Reverse-engineering guide for small-model agents (Haiku / Sonnet), KOTOR II Steam exe

Status: 2026-10-03. Sources:
- the Haiku pilot (`steam_db_pr3/haiku_eval_plan.md`, "Pilot result");
- Gemini's write-up of its address-DB batch process (Batches 1-8, `steam_db_pr3/20_address_db_session_context_2026-10-02.md`, branch `addressdb/k2-steam-acc`);
- the engine-AI atlas run (`engine_ai_atlas/`, 8 Sonnet batches, 2 Opus gates, lead spot-checks).

Every rule below was checked on disk; corrections to the source claims are marked **[checked]**.

**Read this before writing any brief for a Haiku or Sonnet analyst, or any "judge this function" prompt.** The analyst
contracts (`engine_research_findings/CONTRACT.md`, `engine_ai_atlas/CONTRACT_AI.md`) point here.

## 1. The core rule: scripts compute, models judge narrow questions

The Haiku pilot (434 real rows + 40 decoys, one pass, per-function "evidence cards") gave:
- **decoys:** a 33% false accept (13 of 40 said `match`);
- **real rows:** 82 false mismatches, with 0 actual DB errors.

Haiku acted as a lenient pattern matcher: "`ret N` fits, size looks similar, so match". The failures were not knowledge
gaps a bigger prompt fixes. They were math and convention traps that a script removes completely.

So: **never ask a small model to convert hex/decimal, compute deltas, normalise stack sizes, or decide a class from a
name.** Pre-compute these and put the result in the prompt as a stated fact.

### Tiered funnel (use this shape for any batch over ~20 functions)
| tier | who | job |
|---|---|---|
| 0 | scripts | Anchors, delta sweep, invariants, ABI normalisation, decimal->hex, callee topology. Auto-accept clean contiguous delta runs; auto-reject hard contradictions. |
| 1 | Haiku | **Sieve, never a final gate.** Extract content anchors (strings, callees, field offsets); flag `mismatch` / `unsure`. A Haiku `match` alone never promotes a row. |
| 2 | Sonnet | Rows Haiku marks unsure/mismatch, decoy conflicts, anything needing semantics (what a field means, which path is live). This is where the atlas analyst batches sit. |
| 3 | Opus + lead | Gate: re-read the 3-5 highest-impact claims per batch for MEANING (not just that the quoted line exists), resolve contradictions, assign confidences, commit. |

Precision beats coverage: a wrong address row calls into unrelated code; a missing one only disables a feature.

## 2. Tier 0: deterministic pipeline (what scripts do before any model sees a row)
1. **Scope:** pick classes by mod demand and GOG parity (whole classes, not scattered functions).
2. **Anchors:** high-entropy ground truth.
   - String literals (`strings.tsv`: 2DA names, script names, GFF field names, error strings such as "aborted walking, Maximum number of bumps happened" in 0x5CB430).
   - Dispatch tables:
     - effect apply/remove fill in 0x593A30;
     - command table `InitCommands 0x665F50`, 877 slots / 774 filled, index = nwscript.nss order **[checked by GATE_wave2]**;
     - `RunActions` jump table 0x5D7038, 0x47 entries, index = type-1.
   - RTTI and vtables.
3. **Delta sweep:** MSVC keeps source order inside a translation unit, so a class's GOG->Steam delta is constant over long
   runs. Our tool is `ghidra_knowledge/gog_cluster_delta_map.py` (vote = strong, drift = hypothesis).
   - A function off the run's delta is an Aspyr change: inspect it by hand.
   - Gemini reports these per-class deltas; only CExoResMan is cross-checked against our DB:
     - CSWSEffectListHandler Δ=0;
     - CSWSItemPropertyHandler Δ=-0x217410;
     - CSWSCreatureStats Δ=+0x3C480;
     - CExoResMan Δ=+0xF66A0 (matches DB source `gog_delta+0xF66A0`).
4. **Invariants** (any failure means reject or escalate):
   - **Entry alignment 16 bytes:** a strong signal, not a hard rule **[checked]**. 1,785 of 1,796 high-confidence `name_map` rows are aligned; all 11 exceptions are CRT/library code (`operator delete` 0x9196FD, `operator new[]` 0x91D288, `__security_check_cookie` 0x9196EE, `_malloc` 0x9215E6). Over all Ghidra functions only 56% are aligned, because many are compiler fragments and thunks.
   - **Standard prologue:**
     - `55 8B EC` (push ebp);
     - `56 8B F1` (push esi; mov esi,ecx);
     - `53`, `83 EC` / `81 EC`;
     - SEH `6A FF 68`;
     - or an `E9` thunk.
   - **`ret N` vs GOG param size after slot promotion** (section 3.1).
   - **No overlap** with an already-established function body.
5. **Callee topology:** a candidate's callees and callers must agree with already-confirmed rows (`cg.py callers|callees`). Resolve single-line thunks to their target first.
6. **Persist:**
   - Write via an idempotent persist script (add-only `name_map`, demote with a note, `PRAGMA integrity_check`).
   - The runtime address DB gets only rows that also passed a live test.
   - **[checked]** The 1,704-function DB is on the `kpm-pr-steam-db` branch (`tools/kpm-pr-steam-db/AddressDatabases/kotor2_steam_aspyr.db`). The live game `addresses.db` still has the original 48 functions / 14 globals / 21 offsets (2026-10-03). Do not assume rows are live.

Tools that do tier-0 work here:
- `ghidra_knowledge/gog_cluster_delta_map.py` (delta sweep);
- `engine_ai_atlas/tools/cg.py` (call graph, paths, vtable-slot sites);
- `engine_ai_atlas/tools/field_census.py` (every `[reg+off]` read/write);
- `ghidra_knowledge/tools/re_card_prep.py`:
  - `annotate` rewrites decimal offsets to hex, adds the pointer-scaled byte offset, and flags auto labels and thunks;
  - `abi` gives the promoted `ret N` verdict;
  - `anchors` lists strings, callees and offsets;
  - `prologue` checks alignment and prologue.

### 2.1 Free static facts first, live REA last (DECOMP plan, `ghidra_knowledge/DECOMP_PLAN.md`)
The static dumps in `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` cover every real function
(12,355 `.asm`/`.c` pairs + `callgraph.tsv`, `strings.tsv`, `globals.tsv`, `imports.tsv`, `inputs/steam_functions.tsv`).
They matched live REA on 955 of 958 checks. So size, RET, function start, callers, direct callees and strings cost
$0 and take milliseconds; never ask REA or a model for them.

| question | tool (all $0, < 1 s) |
|---|---|
| known name / ledger finding at an address | `python3 scripts/query_ghidra.py address <hex>` (ledger rows tagged `[rea]`, overridden ones `[superseded]`) |
| callers, callees, call path, vtable-slot sites | `engine_ai_atlas/tools/cg.py callers|callees|path|indirect --slot` |
| field reads/writes | `engine_ai_atlas/tools/field_census.py` |
| all static facts for one address (size, RET, callers/callees named, INDIRECT vtable slots, strings, globals, imports, look-alike twins + the distinguishing asm line) | `python3 ghidra_knowledge/tools/static_facts.py addr <hex> [--json]` / `twins <hex>` (~0.4 s) |
| the same facts on every review card (blind, real = decoy) | `steam_db_batches/tools/batch.py cards B<n> --facts` |
| ledger hygiene before using or adding rows | `python3 ghidra_knowledge/tools/rea_ledger.py lint` (rules L1 L2 L4 L5 L6 L8; baseline 53 issues on 10-15) |

Live REA, scripted ($0 tokens): `python3 ghidra_knowledge/tools/rea_session.py guard` (counts live REA sessions;
max 2) then `rea_session.py call <tool> --args '<json>'` or the `ReaSession` class: one persistent MCP session,
~5 min cold start once, then ~0.01 s per call; results are envelopes (read `normalized_result`); cached under the
dump dir `rea_cache/`. REA cannot resolve vtable calls (use the static slot instead) and rejects
`set_addresses_names` (use `annotate_native_function`). Details: `.claude/research/rea_decomp/D1_probe.md`.

Live REA (`rea-recon`) is only for what the dumps cannot answer: targets of INDIRECT call sites, non-contiguous
function bodies, live data xrefs, and named pseudo-C. At most 2 live REA sessions machine-wide.

## 3. Traps (put the relevant ones in every small-model prompt as facts)

### 3.1 x86 stack slots: narrow args still pop 4 bytes
GOG metadata records argument sizes by type: bool 1, short 2, odd totals such as 5, 13, 14. On 32-bit x86 every stack
argument occupies a 4-byte slot, so the expected callee pop is `ceil(gog_bytes/4)*4`.
- cdecl pops 0.
- In the pilot, 30 of the 82 false mismatches were exactly this.
- State it pre-computed: `Stack Slot Match: TRUE (GOG 1 byte -> 1 slot; ASM RET 4)`.

### 3.2 Thiscall thunks print without `__thiscall`
A one-line forwarding thunk (`JMP target`, or `MOV ECX,..; JMP`) is often decompiled as a plain function; ECX passes
through invisibly. That produced 12 false mismatches in the pilot. Resolve thunks to their target and judge the target.

### 3.3 Decompile offsets: bare numbers are DECIMAL, and pointer arithmetic is SCALED
**[checked]** Ghidra prints most offsets in hex but small ones in decimal: 458 decimal vs 12,189 hex `+ N)` forms in
0x5xxxxx. On top of that, `p + N` on a typed pointer scales by the element size, as in C.

Example: in 0x5F2FE0, `param_1` is `int *` and line 175 is `*(short *)(param_1 + 100) = ...`. That is byte offset
100 x 4 = **0x190**, not 0x64 or 0x100. The asm agrees: `005f3385 MOV word ptr [EDX + 0x190],AX`.

The rules:
- An `int` / `char *` / `undefined *` base adds bytes.
- An `int *` / `undefined4 *` base multiplies by 4; `short *` by 2.
- Asm operands are always hex byte offsets and settle any doubt: quote the asm.

Pre-convert in the prompt, e.g. `(param_1 + 0x64 [dec 100] {int*: byte +0x190})` (`re_card_prep.py annotate`). The pilot
had one offset false mismatch from reading `+ 100` as hex.

### 3.4 Ghidra auto labels are not ground truth
FID/library labels (`FID_conflict:*`, `vector_deleting_destructor`, `_atexit`, ...) land on small game functions: 7
false mismatches in the pilot. Our mapper and DB names are hypotheses too: confirm them by body (ret N, callees,
strings), never by the label.

### 3.5 Neighbours with the same `ret N` (the decoy trap)
Adjacent functions often share a signature. A model that checks only `ret N` + size accepts the neighbour.

Real cases from this repo (use them as negative few-shots):
- **0x5644B0 vs 0x564770:** both were named `AddMoveToPointAction` (one at "high").
  - 0x5644B0 calls AddActionToFront (`005645e4 CALL 0x00540ca0`), so it is `AddMoveToPointActionToFront`.
  - 0x564770 calls AddAction (`005648a4 CALL 0x0053f7f0`).
  - The callee is the anchor that separates them.
- **0x552A90 vs 0x552C90:** the GOG param sizes for Initialize/Put are swapped, so the mapper voted wrong.
  - 0x552A90 has `ret=0` and `00552aa5 PUSH 0x2000` + `CALL 0x00919723` (allocates the table), so it is Initialize.
  - 0x552C90 has `RET 0x10` and calls the hash Get, so it is Put.
  - Structure beats the vote.
- **The CSWSCreature outlier cluster:** the mapper re-maps a 19-row false cluster as `vote`, including 7 known-false
  addresses (`AI0_mapper_summary.md`). "vote" is not proof.

### 3.6 Dead code behind `XOR r,r; Jcc`
`005bc4b8 XOR EDX,EDX` / `005bc4ba JZ 0x5bc5de` is always taken, so the LOS block after it in 0x5BC1D0 never runs. A
Sonnet batch (AI1) described that block as live behaviour; the Opus gate refuted it. Before calling a block live, check
the flag source of the branch that guards it, and look for inbound jumps into it.

### 3.7 Field offsets do not identify the class
`field_census --offset 0x3A0 --all` returns writers in creature code AND in placeable code (0x5EED40 ctor and others).
The same offset means different fields in different classes. Decide the class from the base register's provenance
(`this` of a known method, `[ebp-N]` copy of ECX) and quote it.

### 3.8 Field names are hypotheses too
`+0x10E8` is called "IsPC", but it stays on the Exile while the player controls a companion. Its writers are the ctor,
load (0x5768FE) and SwitchPlayerCharacter only; the UI leader switch never writes it. Our DLLs had assumed "IsPC =
controlled". Name fields by what writes them, not by what they look like.

### 3.9 A quoted line can exist and still be misread
Lead spot-checks confirm a quoted line exists. The gate must confirm what it means.
- Example: the spot-check distance term `00579007 NEG EAX` / `0057900f SUB ECX,[EBP-0x10]`. The lines were right, but
  it took the gate to decode the constants (3.0, 2.0) and the margin's sign convention before calling it a bug.
- Ask analysts for the semantic chain: constant values, sign conventions, and who reads the result.

### 3.10 Absence is not proof
A grep or census that finds no writer only means none was found by that method (lesson 168). Check vtable/indirect
writes, `[reg+off]` with other bases, and table-filled function pointers. Absence claims are never `high`.

## 4. Prompt rules for small models

### 4.1 Invert the burden of proof
> Assume this function is a neighbouring function or thunk with an identical `RET` signature. You are FORBIDDEN to
> answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite at least one content anchor from the
> evidence: a string literal (address + text), a confirmed callee (address + name), or a member offset with its access
> (asm line). If you cannot, answer `unsure`.

### 4.2 Evidence before verdict (output schema order matters)
The model decides as it writes, so the verdict field goes LAST:
```json
{
  "observed_prologue": "55 8b ec",
  "is_thunk": false,
  "thunk_target": null,
  "stack_slot_match": "TRUE (given)",
  "content_anchor": "PUSH 0x9965f8 'aborted walking, Maximum number of bumps happened' at 005cbcb5",
  "neighbour_decoy_considered": "<neighbour addr> also RET N; string not referenced there",
  "auto_label_ignored": true,
  "confidence_1_to_5": 4,
  "verdict": "match | mismatch | unsure",
  "reason": "<= 20 words"
}
```

### 4.3 Few-shot negatives in every prompt
Include 2-3 traps with their answers:
- the neighbour pair (3.5);
- a bool argument with `RET 4` (3.1);
- the decimal + scaled offset (3.3: `int *p; p + 100` = byte 0x190);
- optionally the dead `XOR/JZ` block (3.6).

Keep decoys in every wave: gold negatives paired with real rows (neighbour, same-`ret N` function in the same run, random).

### 4.4 Calibrate before trusting
- Run each new prompt on a gold set (decompile-confirmed rows + decoys) before using it.
- Haiku may promote rows only when its decoy false-accept is near 0. Until then it only flags.
- Re-run a small gold slice in every wave.

## 5. Process rules learned in the atlas run (Sonnet analysts)
- **Private scratch dirs:**
  - Parallel agents must each use `mktemp -d`.
  - One atlas analyst overwrote shared scratch helpers (`pe.py`, `cmds.pkl`) while another was running. The gate then had
    to re-derive every exe-table-byte claim the other one made.
- **Tool order:** `cg.py`, then `field_census.py`, then grep, then read `.c`/`.asm`. Cite the tool output.
- **Verdict words:** every claim needs CONFIRMED / REFUTED / SPECULATIVE plus a quoted line with its address.
- **Gate checks:**
  - The gate re-reads the top claims for meaning, and decodes constants from the exe via the PE section table (`.text`: VA - 0x400000 - 0xC00).
  - The lead spot-checks 2-3 lines per batch, plus the gate's top 3.
- **Persistence:** persist scripts are idempotent; a second run must add 0 rows.
- **Contradictions are results:** when a batch contradicts a lesson or an earlier doc, quote both. The gate settles it and
  updates the lesson with a dated note.
