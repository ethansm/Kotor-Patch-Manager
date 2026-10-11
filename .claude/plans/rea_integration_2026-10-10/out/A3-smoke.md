# A3 smoke test: rea-recon

Contract: recon. Calls run serially, once each, in brief order.
Binary: /tmp/claude-1000/rea_probe/swkotor2.exe, provider_id "ghidra", no snapshot_path.

| # | Tool | Status | Seconds | Key result fields |
|---|------|--------|---------|-------------------|
| 1 | ToolSearch select (open_binary, procedure_info, procedure_callees, close_binary) | OK: all 4 schemas loaded | unknown | none |
| 2 | mcp__rea__open_binary | OK | unknown | format=pe, kind=executable, architecture=x86, sha256=6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef |
| 3 | mcp__rea__procedure_info (0x00775700) | provider_timeout (retryable) | unknown | details: operation=procedure_info, timeout_ms=330000. Procedure name: unknown (not returned). |
| 4 | mcp__rea__procedure_callees (0x00775700) | provider_timeout (retryable) | unknown | details: operation=procedure_callees, timeout_ms=330000. Callee count: unknown. First 5 callees: unknown. |
| 5 | mcp__rea__close_binary (no arguments) | OK | unknown | result: null |

Notes:
- Wall-clock seconds are unknown. This agent has no clock or shell tool. timeout_ms=330000 is the provider limit from the error, not a measured elapsed time.
- REA_GHIDRA_STARTUP_TIMEOUT_MS=900000: this agent cannot set environment variables. Both procedure calls reported timeout_ms=330000, which does not match 900000. The override was probably not in effect for these calls. This is an inference and is unconfirmed.
- No retries were made. Steps 3 and 4 each ran once.
