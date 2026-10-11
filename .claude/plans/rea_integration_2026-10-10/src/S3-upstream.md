# S3 upstream recon: rea-agents 6.3.0

HEAD verified: 74cf9e1401fcd89482fe39c27cc898f5858e9b72

## (a) Derivative exclusion
- Explicit ID allowlist: arch, cachyos, ubuntu>=24, fedora>=41, nobara>=44. ID_LIKE only sets packageFamily. CLONE src/application/LinuxHopper.ts:149-163, 356-366.
- DIST dist/application/LinuxHopper.js:62-74 matches the CLONE logic.
- No Pop, Mint, or derivative comment in src, tests, or docs. "derivative" appears only as test input: src/application/LinuxHopper.test.ts:92.
- Intent signal: test title LinuxHopper.test.ts:96; vendor list src/application/SetupHost.ts:82. No rationale is stated. Intent OPEN.

## (b) DIST claims
1. CONFIRMED. Readers: src/application/DoctorDiagnostics.ts:107 (host line); SetupHost.ts:81; SetupPlan.ts:52 (Hopper install proposal); LinuxHopper.ts:192 (install refusal).
2. CONFIRMED. No reads in src/ghidra or src/server. No binary files in src/ghidra. GhidraDoctor.test.ts:69 stubs a different flag (supportedLinuxHopper).
3. CONFIRMED with caveat. src/ghidra/GhidraClient.ts:358-359 gives parent SESSION_ROOT and prefix rea-ghidra-. src/process/PrivateRuntimeRoot.ts:77 is mkdtemp and :81 is chmod 0o700, POSIX only (:72-75 Windows). DIST GhidraClient.js:242-243 passes only the prefix; mkdtemp is in dist/process/PrivateRuntimeRoot.js:43-45.

## (c) Ghidra tool-contract tests
1. tests/boundary/mcp/ghidraNativeApiMcp.test.ts:5, inspect_native_api {procedure: "fixture_main"} (:13-15).
2. tests/boundary/mcp/ghidraInventoryMcp.test.ts:8, read_bytes {address, length: 4} (:22-23).
3. tests/boundary/mcp/ghidraRoutingMcp.test.ts:6, batch_decompile {addresses: [...]} (:11-13); set_comment call (:27-28).

## Verdict
PARTIAL. (b) and (c) confirmed. Intent of the derivative exclusion is undocumented (OPEN).
