# S1 tool contract table (rea-agents 6.3.0)

Anchors are SRC-relative (`dist/...`). All 9 names found.

| tool | required inputs | optional inputs | key output fields | ghidra_supported | effects.mutatesArtifact | anchor |
|---|---|---|---|---|---|---|
| procedure_pseudo_code | procedure | document | normalized_result: string or null | yes | absent | officialToolContracts.js:58; toolOutputSchemaGroups.js:122; GhidraFunctionValues.js:24 |
| xrefs | none (schema) | address, document | normalized_result: address string array | yes | absent | officialToolContracts.js:68; toolOutputSchemaGroups.js:132; GhidraFunctionValues.js:27 |
| procedure_references | procedure | direction (incoming or outgoing, default outgoing), document | procedure, direction, reference_kinds_available, unresolved_calls, references | yes | absent | officialToolContracts.js:53; toolOutputSchemaGroups.js:115; GhidraFunctionValues.js:26 |
| analyze_function | procedure | none (strict, no document) | procedure, pseudocode, assembly, comments, callers, callees, in/outgoing_references, referenced_strings, referenced_names, basic_blocks, native_api, limitations | yes | absent | functionWorkflowToolContracts.js:27; enhancedInputs.js:38-40; hopperValues.js:253; toolOutputSchemaGroups.js:263 |
| procedure_info | procedure | document | name, entrypoint, basicblock_count, length, signature, locals, classification, body | yes | absent | officialToolContracts.js:50; toolOutputSchemaPrimitives.js:401; GhidraFunctionValues.js:23 |
| search_procedures | pattern | mode (literal default or regex), case_sensitive (default false), document | array of {address, value} | yes | absent | officialToolContracts.js:60; analysisSearchContract.js:3-10; toolOutputSchemaGroups.js:124; GhidraInventoryValues.js:21 |
| search_strings | pattern | mode, case_sensitive, document | array of address, value, provider_value, decoding, string{encoding, termination} | yes | absent | officialToolContracts.js:61; toolOutputSchemaGroups.js:125; hopperValues.js:43; GhidraInventoryValues.js:22 |
| open_binary | path (absolute) | format (only "dos-com"), provider_id, snapshot_path | result: path, format, kind, sha256, architecture | no (session tool, not a GHIDRA_OPERATIONS entry) | absent | sessionToolContracts.js:40; sessionLifecycleInputs.js:19-24; toolOutputSchemaGroups.js:303 |
| close_binary | none | snapshot_path, overwrite (default false) | result: null or path, bytes, primitive_entries, workflow_entries, evidence_records | no (session tool) | absent | sessionToolContracts.js:41; sessionLifecycleInputs.js:28-31; toolOutputSchemaGroups.js:310 |

Notes:
- The `effects` helper has no `mutatesArtifact` key (toolEffects.js:1-11). Read tools use `evidence` = mutatesSession true (toolEffects.js:12, 126-156).
- open_binary: mutatesSession, launchesProcess (toolEffects.js:265). close_binary: mutatesSession, writesFilesystem, mayDiscardData, idempotent false (toolEffects.js:266-270).
- Ghidra capability sets mutatesArtifact false for all 9 (GhidraProviderCapabilities.js:156). Ops list: GhidraFunctionValues.js:14-28; GhidraInventoryValues.js:9-24. available: true at GhidraProviderCapabilities.js:147.

OPEN:
- targetFormatSchema and targetKindSchema full enum values (toolOutputSchemaPrimitives.js:22, :42) not expanded.
- evidenceBaseSchema envelope fields (dist/domain/evidence.js:110) not expanded.
