# H3: REA MCP internals (rea-agents@6.3.0, static read)

Package root: `/home/ethansm/.npm/_npx/ec0ab2283efe73f6/node_modules/rea-agents/` (abbrev `PKG/`).
Method: static reading only. REA, Ghidra and MCP servers were NOT started.
Paths below are relative to `PKG/dist/` unless shown otherwise.

## 0. How REA is wired for Claude Code

- The only REA MCP config is in subagent frontmatter: `~/.claude/agents/rea-recon.md:6-13` and `~/.claude/agents/rea-row-verify.md` (same block).
- Command: `npx -y rea-agents@6.3.0 mcp` (stdio). Env keys only: `GHIDRA_INSTALL_DIR`, `JAVA_HOME`, `REA_GHIDRA_STARTUP_TIMEOUT_MS=900000`.
- No `rea` key in `~/.claude.json`, `~/.claude/settings.json`, `.mcp.json`, or `.claude/settings*.json` (rg, no matches). Global `mcpServers` are `{}`.
- Each subagent spawn starts its own `npx` process, so each run gets a new JVM and a fresh analysis.
- Engines: Node `^22.19.0 || ^24.11.0 || >=26` (`PKG/package.json`).

## 1. MCP tool names (native binaries)

Contract modules scanned: `contracts/officialToolContracts.js` (38 tools), `contracts/sessionToolContracts.js` (21), `contracts/enhancedToolContracts.js` (12), `contracts/functionWorkflowToolContracts.js` (2), `contracts/native/nativeToolContracts.js` (8). Other modules (web, android, firmware, evm, managed, electron) are not native-PE tools and were not enumerated.

Allow key: **Y** = in rea-recon.md allowlist; **N** = read-only but NOT allowed; **W** = write-capable, forbidden by rea-recon.md.

Session and lifecycle (sessionToolContracts.js:40-41):
- Y `open_binary`: open target, bind one provider (`provider_id: "ghidra"`). Same target+profile reopen keeps the active DB.
- Y `close_binary`: drain requests, optional snapshot write (only if `snapshot_path` passed), close.
- N `binary_session`: report target/provider status, no analysis.
- N `get_navigation_context`, `inspect_address_context`, `get_evidence_bundle`, `list_unknowns`.
- N `export_evidence_bundle`, `import_evidence_bundle`: file write / merge.
- N `record_unknown`, `update_unknown`, `verify_unknown_resolution`: write to evidence store.
- N `compare_functions`, `compare_artifacts`, `compare_bundles`, `find_changed_behavior`, `build_call_path`, `correlate_static_and_runtime`, `verify_reconstruction`, `capture_process_scenario`, `compare_process_captures` (names only, not read).

Procedure and read tools (officialToolContracts.js:25-69):
- Y `procedure_pseudo_code`: decompiled C for one procedure.
- Y `procedure_assembly`, `read_function_instructions`: disassembly of procedure or function.
- Y `procedure_info`: procedure metadata.
- Y `procedure_callers`, `procedure_callees`: direct call edges.
- Y `procedure_references`: references to a procedure.
- Y `xrefs`: cross-references for an address or name.
- Y `search_procedures`, `search_strings`: search by name or string.
- Y `resolve_containing_procedure`: procedure containing an address.
- Y `analyze_function` (functionWorkflowToolContracts): function dossier, description not read.
- Y `batch_decompile` (enhancedToolContracts.js:22): decompile many addresses in one call. See section 4.
- Y `read_bytes`: listed in rea-recon.md and `toolEffects.js` (evidence), but NO contract definition found in the five scanned modules. See OPEN.
- N `procedure_address`: resolve name to address.
- N `list_procedures`, `list_strings`, `list_names`, `list_segments`, `list_bookmarks`, `list_documents`.
- N `address_name`, `comment`, `inline_comment`: reads at an address.
- N `current_address`, `current_procedure`, `current_document`, `next_address`, `prev_address`: GUI-cursor state.
- N `inspect_native_instruction`, `inspect_native_data_type`, `inspect_native_load_image`, `resolve_native_call_targets`.
- N `find_xrefs_to_name`: xref variant that resolves an exact name.
- N `get_call_graph`, `trace_call_path`, `trace_feature`, `trace_native_values`, `trace_native_ui_action`: multi-hop traversals and tracers.
- N `binary_overview`: segment and count metadata.
- N `inspect_native_api`: API inspection (functionWorkflow).
- N `inspect_native_dispatch_metadata`, `get_objc_classes`, `get_objc_protocols`, `analyze_swift_types`: ObjC and Swift only, not applicable to PE.
- N Mach-O and Apple: `inspect_macho`, `inspect_signature`, `inspect_plist`, `list_architectures`, `demangle_swift`, `observe_native_ui`, `capture_native_ui_scenario`, `observe_native_calls`.

Write-capable (forbidden for rea-recon):
- W `annotate_native_function`: sets name and comments, writes filesystem (`toolEffects.js` annotate entry).
- W `set_address_name`, `set_addresses_names` (batch rename), `set_comment`, `set_inline_comment`, `set_bookmark`, `unset_bookmark`.
- W `goto_address`: changes UI state.

Note: `batch_decompile` is the only batch-style read tool. `set_addresses_names` is a batch write.

## 2. Ghidra provider: lifecycle and caching

Per `open_binary` (GhidraClient.js `#startWithin`, starts ~line 241):
1. Runtime root: `PrivateRuntimeRoot.create({parent: SESSION_ROOT, prefix: "rea-ghidra-"})` (`ghidra/GhidraClient.js:242`). `SESSION_ROOT = ghidraSessionRoot()` (`GhidraClient.js:27`), which is `os.tmpdir()` unless the path has a dot element (`ghidra/GhidraSessionRoot.js:29`). Falls back to `/tmp`.
2. Target copy: `createGhidraTargetSnapshot` copies the exe into the runtime root as `target-<sha12>.exe`, mode 0600, digest-verified (`ghidra/GhidraTargetSnapshot.js:14,31`, `GhidraClient.js:264`). Nothing is written next to the exe.
3. Launch: `support/analyzeHeadless` from `GHIDRA_INSTALL_DIR` (`ghidra/GhidraInstallation.js:144`), run as Java directly: `ghidra.Ghidra ghidra.app.util.headless.AnalyzeHeadless` (`ghidra/GhidraJavaLaunch.js:61-62`). Heap is `GHIDRA_HEADLESS_MAXMEM` or `GHIDRA_MAXMEM`, default `-Xmx2G` (`GhidraJavaLaunch.js:53`).
4. Headless args (`ghidra/GhidraLauncher.js:211-257`): `<runtime>/project rea-project -import <snapshot> -readOnly -deleteProject -log ... -postScript ReaGhidraBridge.java <descriptor>`.
   - Fresh project each open, deleted at the end, read-only. No analysis DB survives across opens.
   - Full auto-analysis runs on every open, bounded by the startup deadline.
5. Long-lived JVM: the postScript `ReaGhidraBridge` serves requests on `bridge.sock` (unix) or `127.0.0.1` loopback TCP (`ghidra/GhidraTransport.js:9,77-83`). The JVM lives as long as the MCP process keeps its binding.
6. Concurrency: `GhidraRequestQueue` admits one Program request at a time (`ghidra/GhidraRequestQueue.js:2-3`). Lifecycle ops take one in-process lock (`application/binary/BinarySession.js:210`).

Isolation and env:
- `HOME`, `XDG_CACHE_HOME`, `XDG_CONFIG_HOME`, `XDG_DATA_HOME`, `-Duser.home`, `-Djava.io.tmpdir` all point inside the runtime root (`ghidra/GhidraLauncher.js:304,318-320,325`). `~/.cache` and `~/.ghidra` are not touched by this provider.
- REA_GHIDRA_* keys actually read: `REA_GHIDRA_STARTUP_TIMEOUT_MS` (`config/parseConfig.js:49`, default 330000 ms in `config/ghidraStartupTimeout.js:3`) and `REA_GHIDRA_NATIVEAOT_JAR` (`parseConfig.js:50-52`). No cache-dir variable exists.

Cleanup: runtime and socket roots are removed on close (`GhidraClient.js:458-467`). Persistence exists only through the explicit analysis snapshot: `close_binary`/`open_binary` `snapshot_path` (`application/binary/BinarySession.js:210-213`, `sessionToolContracts.js:40-41`). rea-recon.md forbids passing it.

Answer to Q2: no cross-open analysis cache. One JVM per MCP process; new JVM and full re-analysis per `npx` process.

## 3. Python as an MCP stdio client

- Protocol: standard MCP JSON-RPC 2.0 over stdio. SDK is `@modelcontextprotocol/server` and `core`, under `PKG/../node_modules/@modelcontextprotocol/` (node_modules sibling of `rea-agents`).
- Versions: LATEST `2025-11-25`, supported `2025-06-18`, `2025-03-26` (`@modelcontextprotocol/core/dist/auth-BNDyLTqp.mjs:4-9`).
- Handshake schemas: `initialize` (`auth-BNDyLTqp.mjs:321`) and `notifications/initialized` (`auth-BNDyLTqp.mjs:353`). Send `initialize`, read result, then send `notifications/initialized`, then `tools/list` and `tools/call`.
- Caveat: the SDK has a "modern envelope" path (`@modelcontextprotocol/server/dist/stdio.mjs` imports `carriesValidModernEnvelopeClaim`, `modernOnlyStrictRejection`). Whether `initialize` can be skipped was NOT verified.
- Framing: server appends stdin chunks to a `ReadBuffer` and drains messages (`stdio.mjs:47-48`, `processReadBuffer` at `stdio.mjs:79-88`). The buffer's delimiter was not opened; standard MCP stdio is newline-delimited JSON.
- Session lifetime: process runs until stdin `end`/`close` or SIGINT/SIGTERM (`PKG/dist/main.js:24-27`). A Python client can hold stdin open and make many `tools/call`s over one session.
- open_binary semantics: "binding remains stable until close or an explicit switch, with no failure fallback" (`sessionToolContracts.js:40`). One target per process.
- Concurrency: calls are queued, not parallel (`GhidraRequestQueue.js:3`). Separate processes do not share a lock (grep for lock/flock/pidfile in `process/`, `application/binary/`, `main/`, `server/` found none). Each process spawns its own JVM with `-Xmx2G` default.

## 4. batch_decompile

- Input: `{ addresses: string[] }`, default `[]` (`dist/contracts/enhancedInputs.js:15-19`). No max-count in the schema.
- Handler: `application/EnhancedTools.js:180-218`. `Promise.all` over all addresses. Each address is resolved, then `procedure_pseudo_code` is called. Concurrency is capped by the Ghidra queue, so effectively serial.
- Output: `{items:[{address, procedure, status:"ok", pseudocode} | {address, procedure, status:"error", error}], total, succeeded, failed}` (`EnhancedTools.js` return near line 213-217).
- Per-item errors do not fail the batch.
- Response size: `createToolResultDelivery(parseMcpResponseBudget(REA_MCP_MAX_RESPONSE_BYTES))` (`server/createServer.js:156-159`). Default budget lives in `config/mcpResponseBudget.js` (not opened).

## 5. "Connection closed" and "No app is open"

- "No app is open. ... call open_binary with its local path." is the message of the class at `domain/configurationErrors.js:15`. Thrown when no target is bound. Startup with no `HOPPER_TARGET_PATH` returns ok without a target (`main/startup.js:3-4`).
- Server exit paths (client sees "Connection closed"):
  - stdin `end`/`close` (`main.js:26-27`).
  - SIGINT/SIGTERM (`main.js:24-25`).
  - Startup failure: `runEntrypoint` catch writes a generic message, exit 1 (`main.js:84`).
  - Initial target open failure: exit 1 (`main/startup.js:20`).
- Timeouts (tool errors, not exits):
  - Default startup 330 s (`config/ghidraStartupTimeout.js:3`). rea-recon sets 900 s.
  - "Ghidra startup deadline elapsed" (`ghidra/GhidraSocketConnection.js:42,81`, `GhidraClientStartup.js:25`).
  - "per-file deadline" analysis timeout (`GhidraClientStartup.js:17`), mapped to `AnalysisTimeoutError` (`ghidra/GhidraProviderClient.js:209-210`).
- Bridge loss: "Ghidra bridge socket closed" fails all pending requests (`ghidra/GhidraClient.js:70-74`).
- Single-instance lock: none found in the scanned directories. Only in-process lifecycle serialization (`BinarySession.js:210`).
- The string "Connection closed" is not in the package. It is the MCP client's message when the server process exits.

## Open items (not verified)

1. `read_bytes`: rea-recon.md allowlists it, but no contract definition was found in the five modules scanned. Run `rg -n 'read_bytes' PKG/dist` in a follow-up to confirm the tool is registered.
2. Does the server accept `tools/call` without `initialize` on the "modern" envelope path? (`stdio.mjs` imports)
3. `ReadBuffer` delimiter: confirm in `@modelcontextprotocol/core` source.
4. Default `mcpMaxResponseBytes` (`config/mcpResponseBudget.js`) and `main/shutdown.js` behaviour were not opened.
5. Whether `analyze_function` and `compare_functions` pass through `batch`-style concurrency: not read.

## Summary for rea-recon allowlist

rea-recon.md allows 16 REA tools (`~/.claude/agents/rea-recon.md:4`): open_binary, close_binary, procedure_pseudo_code, procedure_info, procedure_assembly, procedure_callers, procedure_callees, procedure_references, xrefs, analyze_function, search_procedures, search_strings, read_bytes, read_function_instructions, resolve_containing_procedure, batch_decompile. Excluded read-only tools include `find_xrefs_to_name`, `get_call_graph`, `list_*`, `address_name`, `comment`, and `procedure_address`. The allowlist is correct per the schema. The one unverified entry is `read_bytes`.
