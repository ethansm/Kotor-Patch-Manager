# S1 bridge recon

Anchors SRC-relative. RGB=bridge/ghidra/ReaGhidraBridge.java EXT=bridge/ghidra/ReaGhidraExtensions.java BTR=dist/application/BinaryTargetResolver.js GSC=dist/ghidra/GhidraSocketConnection.js GT=dist/ghidra/GhidraTransport.js

## (a) 28 commands (RGB)
ping:341 shutdown:315 inspect_native_load_image:352 read_bytes:353 address_to_file_offset:354 address_name:355 list_documents:356 list_names:357 list_procedures:358 list_segments:359 list_strings:360 procedure_address:361 procedure_assembly:362 procedure_callees:363 procedure_callers:364 procedure_info:365 procedure_pseudo_code:366 read_function_instructions:367 procedure_references:368 resolve_containing_procedure:369 search_procedures:370 search_strings:371 inspect_native_data_type:372 inspect_native_instruction:373 resolve_native_call_targets:374 xrefs:375 analyze_function:376 annotate_native_function:377

## (b) Program-DB writers
- Only annotate_native_function: setComment RGB:1042, setName RGB:1046, one transaction 1033-1078, rolls back on error.
- Unix-socket only (RGB:378). Startup extensions may mutate (RGB:202-204): extension code unread.
- No retype, apply-type, set-data, or save calls.

## (c) Scripts and files
- None run scripts; no command takes a file path.
- Disk writes only endpoint, socket, and descriptor cleanup (RGB:173,231-275; no Files.copy).
- Startup-only reads: descriptor RGB:166,3209-3212; EXT:39-46 loads a digest-checked jar and instantiates its class.

## (d) open_binary (BTR)
- Any readable file. BTR:23 cwd-resolve; :26 access; :28 realpath; :196-205 dirs need .app; :37 symlinks reject; :189 lstat recheck.
- No allowlist, no root confinement.

## (e) Sockets
- Unix-socket: RGB:230 path from descriptor, :233 bind, :234 chmod after bind; GT:9 bridge.sock.
- TCP: RGB:248 127.0.0.1 port 0; non-unix transports are read-only (RGB:414).
- Token checked on every request, constant-time: RGB:3232,3341. Single accept: RGB:286. Client GSC:49-50.

## OPEN
- Does the open_binary path (or canonical sourcePath) reach Ghidra's importer unchanged? Outside listed targets.
