# H2: REA 6.3.0 CLI catalog for swkotor2.exe (x86 PE)

Sources: `npx -y rea-agents@6.3.0 <cmd> --help`, `--schema`, `--llms`, `capabilities`, `providers`, plus package source at `~/.npm/_npx/ec0ab2283efe73f6/node_modules/rea-agents/dist`. Raw help saved in `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/42a6a25e-5315-402d-a861-d94a14444190/scratchpad/h2/`. No binary opened, no Ghidra launched, no commands run against swkotor2.exe.

## Machine state (2026-10-15)
- `providers`: ghidra unavailable (`GHIDRA_INSTALL_DIR` unset); hopper unavailable (`/opt/hopper/bin/Hopper` missing); ida unavailable (`REA_IDA_MCP_CONFIG` unset). Java 21 is present.
- Result: `function`, `decompile`, `annotate-native-function` and the other provider-backed commands cannot run until the Ghidra provider is configured.

## Global options
`--format toon|json|yaml|md|jsonl`, `--filter-output <keys>`, `--token-limit <n>`, `--token-offset <n>`, `--token-count`, `--full-output`, `--schema`, `--llms[-full]`. Analysis commands also take `--snapshot <file>` ("load and update a local analysis snapshot") and `--provider <id|auto>`.

## Catalog

| Command | Purpose | Required args | Key options | Writes? | Notes |
|---|---|---|---|---|---|
| function | Full function dossier with evidence | path, address or `--procedure` | --snapshot, --provider | snapshot file only | Largest output; use --filter-output |
| decompile | Decompiled code for one procedure | path, address | --snapshot, --provider | snapshot file only | Main decompile call |
| instructions | Raw instructions, no decompile | path, address | --snapshot, --provider | snapshot file only | Fallback when decompile is unclear |
| search | Strings or procedure names | path, pattern | --kind strings\|procedures, --mode literal\|regex, --case-sensitive, --snapshot | snapshot file only | Anchor functions on known strings |
| trace | Literal feature through references | path, query | --case-sensitive, --snapshot | snapshot file only | Feature to xref path |
| xrefs | Bounded refs to one address or symbol | path, address | --snapshot, --provider | snapshot file only | One address per call |
| read-bytes | Exact bytes at a VA | path, address | --length (default 256) | no | Prologues, data, jump tables |
| resolve-native-call-targets | Resolve one call site | path, address | --provider | no | Indirect call targets |
| inspect-native-instruction | One instruction, decoded operands | path, address | --provider | no | |
| inspect-native-api | One function API boundary with evidence | path, address | --snapshot, --provider | snapshot file only | Signature-level evidence |
| inspect-native-data-type | Recovered type or typed data object | path, --type or --address | --provider | no | Needs provider DB |
| annotate-native-function | Set name and/or comments | path, procedure (exact address or unique name) | --name, --comment, --inline-comment (empty clears) | ephemeral session DB only; no --snapshot option | Not persisted, see (b) |
| analyze | App overview | path | --snapshot, --integrity-policy fail\|record-and-continue | snapshot file only | |
| inspect | App overview with evidence | path | --snapshot, --provider | snapshot file only | |
| inspect-binary-layout | Layout of ELF or relocatable object | path | none | no | ELF only, rejects PE |
| address-to-file-offset | VA to file byte offset | path, address | --provider | no | PE VA-to-offset cross-check |
| build-reconstruction-obligation-ledger | Obligation ledger as JSON | inputJson (inline or file) | none | no (stdout) | Parity-oriented, see (c) |
| evaluate-reconstruction-coverage | Fail-closed coverage check | inputJson `{coverage, boundary_id}` | none | no | See (c) |
| evidence-export | Validate and write Evidence JSON | source, output | --overwrite | WRITES output file | Atomic write |
| evidence-import | Validate an Evidence bundle | path | none | no | |
| compare (alias compare-bundles) | Diff two Evidence bundles | left, right | none | no | |
| capabilities | Provider capability and side-effect list | none | none | no | 18 ops, host-gated, large JSON |
| providers | Provider availability | none | none | no | Shows the three blockers above |
| doctor | Readiness check | none | --target, --client, --provider, --skill | no | Not run here |
| setup | Install agent integrations | none | --dry-run, --yes, --client, --all-detected | WRITES agent config | Run --dry-run first |
| mcp add | Register REA as MCP server | none | --agent, --command, --no-global | WRITES agent config | |
| `--mcp` (global flag) | Start MCP stdio server | none | none | no | Blocks; exposes batch_decompile and annotate |

## (a) Batch support
- CLI: no command takes multiple addresses. Every decompile-side command takes one `address` string (confirmed in `--schema`). `search` and `trace` take one pattern or query.
- MCP only: `batch_decompile` takes `addresses: string[]` (default `[]`) and decompiles them concurrently (`dist/contracts/enhancedInputs.js:15-18`, `dist/contracts/enhancedToolContracts.js:22`). It is listed under MCP tools without a dedicated CLI command (`dist/cliCommandNames.js:147`).
- No batch size cap was found in the schema. Open.
- CLI workaround: loop over addresses and pass the same `--snapshot <file>` to every call. Speed not measured.

## (b) Does annotate persist names? NO
- The CLI `annotate-native-function` has no `--snapshot` option (help text).
- Official contract (`dist/contracts/officialToolContracts.js`): edits go to the "ephemeral session analysis database"; "original executable bytes remain unchanged"; "CLI returns the updated dossier before discarding its session."
- `dist/ghidra/GhidraProviderCapabilities.js:34-40` and `:151-156`: metadata edits "invalidate immutable analysis snapshots and are discarded on close."
- Read-only sessions drop annotate (`dist/ghidra/GhidraSessionValues.js:46`). `rea-recon` is read-only, so it cannot annotate.
- What works in REA: one MCP session, issue annotate calls for each known name, then decompile or batch_decompile in that same session. Each CLI call is a fresh session, so a CLI annotate cannot change a later CLI decompile. Not runtime-tested.
- Durable names need a store outside REA, such as the repo's `ghidra_knowledge/` plus `scripts/query_ghidra.py`.

## (c) Ledger and coverage commands
- Ledger input schema: `dist/domain/reconstructionObligationLedgerSchemas.js`. Each obligation has a status (unowned, characterized, implemented, verified, contradicted, blocked, out-of-scope, unknown), a layer (cli, protocol, electron, persistence, process, runtime, packaging, native-abi, application, other), a case kind (positive, negative, malformed, cancellation, teardown) and a proof authority (unit, integration, protocol, renderer, packaged-process, native-abi, live-observation, external). Output is JSON to stdout. No file writes.
- Coverage input: `{coverage, boundary_id}` (`dist/domain/reconstructionCoverageInput.js:4-7`). Output is a fail-closed report. No file writes.
- Fit for exe documentation coverage: weak. The vocabulary targets app-behaviour parity (CLI, Electron, persistence), and inputs need hand-written owner, verifier and fixture bindings. The status ladder (unowned to verified) could be borrowed for a doc-coverage tracker. This is an assessment, not tested.

## (d) Serve mode and env vars
- `rea --mcp` starts the MCP stdio server. `rea mcp add` registers it with an agent (`--agent`, `--command`, `--no-global`). `rea mcp doctor` checks startup and tool listing.
- No env vars appear in any help text. The list below comes from `dist` source.
- Relevant to a PE target: `GHIDRA_INSTALL_DIR` (required, Ghidra 12.1.x), `GHIDRA_JAVA_MAJOR`, `GHIDRA_STARTUP_TIMEOUT_MS` / `REA_GHIDRA_STARTUP_TIMEOUT_MS`, `REA_ANALYSIS_PROVIDER` (default `auto`), `REA_MCP_MAX_RESPONSE_BYTES` (floor is the stdio buffer size), `REA_LOG_LEVEL`.
- Other providers: `REA_IDA_MCP_CONFIG`, `HOPPER_LAUNCHER_PATH`.
- Not PE-relevant, present in source: `REA_JADX_*`, `REA_ILSPY_CMD_PATH`, `REA_BROWSER_EXECUTABLE`, `REA_PWNDBG_*`, `REA_*_PRLIMIT_COMMAND`, `REA_BINWALK/UNBLOB/WAKARU/MITMDUMP_COMMAND`, `REA_PWNTOOLS_PYTHON`.

## Caveats
- Host gating: `capabilities` marks macOS-only ops unavailable on this Linux host (observe-native-calls, inspect-native-dispatch-metadata, demangle-swift, inspect-macho and others).
- Windows P0 limits (`dist/ghidra/GhidraProviderCapabilities.js:21-23`): approved native x86 and x86-64 PE only; DLL, managed, hostile and mutable-path targets are unsupported. swkotor2.exe is x86 PE, so it should qualify, but the approval step was not checked. The P0 gate appears tied to a Windows host, and this host is Linux. Open.
- Not run: function, decompile, doctor, setup. No live test of any command.
