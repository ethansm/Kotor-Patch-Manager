# H1: REA CLI live probe (rea-agents@6.3.0, Ghidra provider)

Target: `/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe` (read only).
sha256 6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef.
Raw outputs and the snapshot: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/42a6a25e-5315-402d-a861-d94a14444190/scratchpad/h1/`
(`cold.json`, `warm.json`, `warm2.json`, `repeat.json`, `swkotor2.snap`, `*.err`, `*_help.txt`).

## Bottom line
- Scriptable: yes. An identical request re-run takes 2.2 s at 280 MB RSS.
- Any new address or any other subcommand takes about 5 min at 3.8 GB RSS. Each run re-imports into Ghidra.
- The snapshot is an exact-request result cache, not a per-binary analysis cache. It does not amortize across addresses.

## Setup and prerequisites
- `doctor` (rc=1, healthy=false): `ghidra` and `ida` are `missing_analysis_engine`. Ghidra needs `GHIDRA_INSTALL_DIR`.
  Set to `/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC` (12.1.4; REA asks for 12.1.x).
  Running without it was not tested.
- `doctor` also flags host `pop 24.04` as unsupported (REA lists Ubuntu 24.04+, Pop!_OS is not listed) and
  config_drift on five client registrations. Neither blocked the Ghidra run.
- `REA_GHIDRA_STARTUP_TIMEOUT_MS=900000` was set for every run. The default was not tested.
- Provider ID: `ghidra` (from `doctor` providerInspections).
- ANOMALY: `providers` printed output byte-identical to `capabilities` (145,926 bytes, capability list, no provider IDs).
  The provider list was not captured.

## Command surface (from --help and --schema)
- `function <path> [address] --provider <id> --snapshot <file> --format <toon|json|yaml|md|jsonl>`.
  Global options also include `--filter-output`, `--token-limit`, `--token-offset`, `--full-output`.
- `--provider` accepts `auto` or `^[a-z][a-z0-9-]*$`. `snapshot` is "load and update a local analysis snapshot".
- `decompile` and `instructions` take the same options. `instructions` returns procedure, instructions, limitations only.
- Not tested: `--filter-output`, `--token-limit`, formats other than json, concurrent runs.

## Runs (all rc=0, all `--format json`, all same snapshot file except cold/warm which share it too)
| run | command | wall | max RSS (KB) | stdout bytes |
|---|---|---|---|---|
| cold | function 0x00776db0 | 5:19.00 | 3,829,184 | 119,611 |
| warm | function 0x007680a0 | 5:03.92 | 3,829,016 | 678,875 |
| warm2 | instructions 0x00776db0 | 5:07.88 | 3,828,904 | 6,505 |
| repeat | identical to cold | 0:02.20 | 279,724 | 119,611 |

Snapshot `swkotor2.snap` (mode 0600): 138,252 B after cold; 873,794 B after warm.
Top-level keys: binding, entries, evidence_bundle, target, workflow_entries.
entries = 2 (both `operation: analyze_function`) after cold, warm, and warm2; workflow_entries = 0.
So `instructions` wrote no snapshot entry, and the repeat added none.

## Snapshot semantics (observed)
- Identical repeat: same `evidence_id` (ev_63ba7e6b...). Not byte-identical: `analysis_profile.parameters` key set differs
  (repeat adds `analyzer_preset`; cold-only keys include `target_kind`, `target_format`, `import_mode`).
  Compare by `evidence_id`, not by file hash.
- New address (warm): full re-analysis, no speed-up.
- Different subcommand on an already-analysed address (warm2): full re-analysis, no new entry.

## JSON field map (`function`; `normalized_result` repeats the same keys as `raw_result`)
- Envelope: `evidence_id`, `subject{name, digest.sha256, format, architecture, local_path}`, `provider{id,name,version}`,
  `analysis_profile`, `predicate_type`, `operation`, `parameters`, `confidence` ("observed"), `authority` ("shipped-artifact"),
  `environment`, `limitations[]`, `locations` (empty in probe), `evidence_links`.
- `raw_result.procedure{address, name, signature, body{available, ranges[{start,end}], total_bytes, span_bytes, non_contiguous, contains_entry}}`.
  `ranges.end` is inclusive. Signature for 0x776db0 is `undefined FUN_00776db0()`.
- `raw_result.pseudocode`: string, Ghidra C-style decompile.
- `raw_result.assembly[]`: strings of the form `"0xADDR: MNEMONIC OPS"`.
- `raw_result.callers[]` and `callees[]`: `{address, name, classification, body}`.
  Callers: 3 for 0x776db0, 9 for 0x7680a0. Callees: 0 and 3.
- `raw_result.incoming_references[]`: `{source_address, target_address, source_procedure, target_procedure, kind{type, call, flow, data, ...}}`.
  One entry per reference site: 5 for 0x776db0, 9 for 0x7680a0.
- `raw_result.outgoing_references[]`: same shape. `target_address` can be `stack:0x4`. `kind.type` READ or WRITE.
  Counts: 15 and 89.
- `raw_result.referenced_strings[]`: empty in both samples (not proof of absence).
- `raw_result.referenced_names[]`: `{address, value, source_address}`. Values are `LAB_*` jump labels, not strings. Counts: 6 and 35.
- `raw_result.native_api{signature_source, calling_convention, return_type, parameters[{name,size_bytes}], pseudocode{...}}`.
  CAUTION: `calling_convention` is `__stdcall` with `signature_source: default`, while the pseudocode reads `in_ECX`.
  The field is a default, not inferred.
- `raw_result.native_value_flow`: operations and parameters.
- `instructions` run: `raw_result` keys are `procedure`, `instructions[]` (24 `"addr: text"` strings), `limitations[]`.

## Coverage matrix
| need | function | instructions |
|---|---|---|
| size | yes: `procedure.body.total_bytes` and `ranges` | procedure object present, not field-checked |
| RET | no field; parse last `assembly` line (`RET n`) | parse `instructions[]` |
| callers | yes: `callers[]` | no |
| callees | yes: `callees[]` | no |
| strings | `referenced_strings` (empty in samples) | no |
| xrefs | `incoming_references`, `outgoing_references` | no |
| pseudo-code | `pseudocode` | no |
| asm | `assembly[]` | `instructions[]` |

## Fact checks
| fact | result | where |
|---|---|---|
| 0x776db0: `MOV dword ptr [ECX + 0x3c0], 0x0` at 0x776dd6 | PASS | cold `assembly[]` |
| 0x776db0: `RET 0x4` at 0x776e01 | PASS (last line) | cold `assembly[]` |
| 0x776db0 body 0x776db0..0x776e03, inclusive | 84 bytes, consistent with a 3-byte RET 0x4 | cold `procedure.body` |
| 0x7680a0: `PUSH 0x272e` at 0x7680d1 | PASS | warm `assembly[]` |
| 0x7680a0: `RET 0x8` at 0x7682a5 | PASS | warm `assembly[]` |
| 0x7680a0 body | 3 ranges, 516 bytes, gaps 0x768297-0x768298 and 0x7682a0-0x7682a1 | warm `procedure.body` |

## Failures and anomalies
- No step failed; no retries were needed.
- `providers` returned capability output (see Setup).
- Repeat output not byte-identical (see Snapshot semantics).
- `doctor` rc=1 (see Setup); non-blocking for Ghidra.
- Not tested: the error text without `GHIDRA_INSTALL_DIR`, `--filter-output`, `--token-limit`, `decompile`, concurrent runs.

## Files written outside scratch
- Exe directory `ls -la` before and after: identical (`exe_dir_before.txt` and `exe_dir_after.txt` in scratch).
- Files newer than cold start under `~/.cache`, `~/.config`, `~/.local/share`, `~/.rea`, `~/.npm/_npx`: no REA-specific files
  (only unrelated Steam, orca, and opencode activity). `/tmp` outside scratch was not checked.
- The Ghidra import is ephemeral (`import_mode: ephemeral-source-immutable`; limitations state the temp project is deleted on close).

## Recommendation for a script driver
1. Set `GHIDRA_INSTALL_DIR` and `REA_GHIDRA_STARTUP_TIMEOUT_MS=900000` in the driver environment.
   Keep one snapshot per binary, outside the game directory.
2. Call `function` per address, not `instructions`. One call returns size, callers, callees, xrefs, pseudo-code, and asm.
   Derive RET from the last assembly line.
3. Cache results per (address, command) in the driver. Compare by `evidence_id`, not bytes.
4. Budget about 5 min and 3.8 GB per new address. Run serially (concurrency untested).
5. Use `--filter-output` to keep the 120 KB JSON out of agent context (untested).

OPEN: does `mcp__rea__batch_decompile` (or any multi-address path) reuse one Ghidra import across addresses? Not tested.
