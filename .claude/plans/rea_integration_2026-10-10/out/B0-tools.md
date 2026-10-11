# B0 tool gate (A2): main session, global REA still registered

Target: `/tmp/claude-1000/rea_probe/swkotor2.exe`, provider `ghidra`, no `snapshot_path`. Probed address: `0x00775700`.
Served = the adapter accepted the call and dispatched it. A typed capability-unavailable error would mean not served. No returned values are recorded here.

| tool | status | error |
|---|---|---|
| open_binary | served; opened; returned quickly | none |
| procedure_info | served; call failed | provider_timeout, timeout_ms 330000 |
| resolve_containing_procedure | served (declared by adapter); call failed | provider_timeout, timeout_ms 330000 |
| batch_decompile | served (REA workflow layer); item failed | provider_timeout on procedure_address and procedure_pseudo_code, timeout_ms 330000 |
| read_bytes | served; call failed | provider_timeout, timeout_ms 330000 |
| close_binary | served; completed | none; `/tmp/rea-ghidra-*` absent after close |

## Decisions
1. `resolve_containing_procedure` and `batch_decompile` added to `tools:` in `~/.claude/agents/rea-recon.md` and `~/.claude/agents/rea-row-verify.md`.
2. `REA_GHIDRA_STARTUP_TIMEOUT_MS: "900000"` added to both inline `env` blocks. Reason: the README names this key for large binaries, and the package uses it as the call timeout (`dist/ghidra/GhidraProviderClient.js`). All four failures report 330000 ms, the default.
3. Not yet proven: any procedure call succeeding in this environment. The A3 smoke test (after the restart) is the first real check.
