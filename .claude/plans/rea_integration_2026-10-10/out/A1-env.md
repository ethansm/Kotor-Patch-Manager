# A1-env: REA doctor environment recon

## Host checks
- `node -v`: `v22.23.2`
- `java -version 2>&1`:
```
openjdk version "21.0.12.1" 2026-08-18
OpenJDK Runtime Environment (build 21.0.12.1+1-1-24.04.4-Ubuntu)
OpenJDK 64-Bit Server VM (build 21.0.12.1+1-1-24.04.4-Ubuntu, mixed mode, sharing)
```
- Headless check: `headless-present`

## Doctor run
- Command: `cd /tmp/claude-1000/rea_probe && GHIDRA_INSTALL_DIR=/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC npx -y rea-agents@6.3.0 doctor --json`
- Exit code: `exit=1` (doctor ran and produced JSON; no run failure, no retry)
- `doctor.err`: empty (0 bytes)
- `doctor.json`: 3200 lines (over 150), so only ok:false entries and summary fields are copied.
- Summary field: no top-level `summary` key exists in doctor.json. Closest top-level fields, verbatim: `"healthy": false`, `"environment_healthy": false`.

## Ghidra checks (all ok:true, for reference)
- ghidra: healthy, `/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC`
- ghidra-version: healthy, `12.1.4`
- ghidra-headless: healthy, `.../support/analyzeHeadless`
- ghidra-native_decompiler: healthy, `linux/x64`
- ghidra-java: healthy, `21.0.12.1; 64-bit; JDK; /usr/lib/jvm/java-21-openjdk-amd64`

## Failing entries, verbatim from doctor.json `checks[]` (ok:false)
The `scope_checks[]` array repeats these same entries (doctor.json lines 11-128).
```json
{
  "name": "host",
  "ok": false,
  "classification": "unsupported_host",
  "detail": "pop 24.04",
  "remediation": "REA supports macOS 12+, Ubuntu 24.04+, Fedora 41+, Nobara 44+, 64-bit Arch Linux, CachyOS, and the experimental Windows x64 Ghidra P0 boundary."
}
{
  "name": "hopper",
  "ok": false,
  "classification": "missing_analysis_engine",
  "remediation": "Run rea setup to install Hopper, or set HOPPER_LAUNCHER_PATH."
}
{
  "name": "ida-registration",
  "ok": false,
  "classification": "missing_analysis_engine",
  "detail": "IDA MCP registration is not configured.",
  "remediation": "Set REA_IDA_MCP_CONFIG to your upstream IDA MCP registration JSON; see docs/ida-provider.md."
}
{
  "name": "skill:identity",
  "ok": false,
  "classification": "config_drift",
  "detail": "Installed REA skill identity is missing.",
  "remediation": "Run rea setup to update the installed REA skill."
}
{
  "name": "registration:antigravity",
  "ok": false,
  "classification": "config_drift",
  "detail": "/home/ethansm/.gemini/config/mcp_config.json",
  "remediation": "Run rea setup to refresh this registration, then restart the client."
}
{
  "name": "registration:claude_code",
  "ok": false,
  "classification": "config_drift",
  "detail": "/home/ethansm/.claude.json",
  "remediation": "Run rea setup to refresh this registration, then restart the client."
}
{
  "name": "registration:codex",
  "ok": false,
  "classification": "config_drift",
  "detail": "/home/ethansm/.config/orca/codex-runtime-home/home/config.toml",
  "remediation": "Run rea setup to refresh this registration, then restart the client."
}
{
  "name": "registration:gemini_cli",
  "ok": false,
  "classification": "config_drift",
  "detail": "/home/ethansm/.gemini/settings.json",
  "remediation": "Run rea setup to refresh this registration, then restart the client."
}
{
  "name": "registration:opencode",
  "ok": false,
  "classification": "config_drift",
  "detail": "/home/ethansm/.config/opencode/opencode.jsonc",
  "remediation": "Run rea setup to refresh this registration, then restart the client."
}
```

## Failing provider check, verbatim from doctor.json `providerInspections[id=ida].checks[]`
```json
{
  "name": "registration",
  "ok": false,
  "code": "not_configured",
  "detail": "IDA MCP registration is not configured.",
  "remediation": "Set REA_IDA_MCP_CONFIG to your upstream IDA MCP registration JSON; see docs/ida-provider.md.",
  "classification": "missing_analysis_engine"
}
```

## Non-ok state fields, verbatim from doctor.json `identity` (not in checks[])
- `identity.skill`: `{"installed_version": null, "installed_tool_count": null, "state": "missing", "remediation": "Run rea setup to update the installed REA skill."}`
- `identity.live_server`: `{"state": "unknown", "remediation": "Call binary_session in the active client to inspect the running server identity."}`
- `identity.registrations[]`: all five entries (antigravity, claude_code, codex, gemini_cli, opencode) have `"state": "missing"`, same config paths as the failing `registration:*` checks above.
