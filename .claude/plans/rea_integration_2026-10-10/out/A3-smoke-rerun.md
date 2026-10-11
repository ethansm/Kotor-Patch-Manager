# A3 smoke rerun (REA recon, serial)

Binary: /tmp/claude-1000/rea_probe/swkotor2.exe, provider_id "ghidra", no snapshot_path passed on open_binary or close_binary. Calls run one at a time, none retried.

| Step | Tool | Status | timeout_ms | Key result |
|---|---|---|---|---|
| 1 | ToolSearch (open_binary, procedure_info, procedure_callees, close_binary) | OK, all 4 schemas loaded | n/a | n/a |
| 2 | mcp__rea__open_binary | OK | not present in response | format pe, kind executable, arch x86, sha256 6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef (prefix 6a522e71), provider Ghidra 12.1.4 |
| 3 | mcp__rea__procedure_info (0x00775700) | OK | not present in response | name FUN_00775700 (Ghidra default name, not a source symbol), entry 0x775700, length 82 bytes, 3 basic blocks, body 0x775700-0x775751. Evidence ev_9150de38afbc4630c68d5851548889b517298e041f897a28dae205377bbb7004 |
| 4 | mcp__rea__procedure_callees (0x00775700) | OK | not present in response | callee count 2 (all callees returned, fewer than 5). First callees: 0x73f810, 0x87a5c0. Evidence ev_f9595b126e456758efdbf7d1ca3cde977708dde6b9c31b600cc1f495935f346a |
| 5 | mcp__rea__close_binary | OK (result null, no error returned) | not present in response | no snapshot written |

## Timeout check

- No provider_timeout or other error was returned by any call.
- No response contained a timeout_ms field.
- Timeout value seen: unknown. Neither 900000 nor 330000 was observed.
- Env override reached server: unknown. All calls succeeded, which fits the 900000 override, but a 330000 limit would also succeed on these calls. Confirming needs a response that exposes timeout_ms, or a call that runs long enough to hit the limit.

## Not run

- scripts/agent_contract.py snapshot and check: this agent has no Bash tool.
