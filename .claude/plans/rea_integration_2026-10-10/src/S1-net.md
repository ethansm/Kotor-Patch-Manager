# S1 network and spawn audit (rea-agents@6.3.0)

SRC-relative anchors.

## (a) Network

| Anchor | Use | Reachable from |
|---|---|---|
| dist/application/UpdateRuntime.js:90-100,139 | `npm view --global ... dist-tags.latest` (registry fetch) | `rea update` only: dist/cli/setupCommands.js:99 |
| dist/application/Update.js:46-108 | update orchestrator | `rea update` only |
| dist/application/LinuxHopper.js:132, MacHopper.js:182 | fetch to hopperapp.com | Hopper path, not Ghidra |
| dist/browser/CdpEndpoint.js:189 | loopback CDP discovery (node:http) | browser tool calls, not Ghidra |
| dist/ghidra/GhidraSocketConnection.js:49-50 | TCP only to 127.0.0.1 (GhidraTransport.js:76-78) or unix socket | Ghidra tool calls, loopback |

- `rea mcp` startup (dist/main.js:40-62): no update check, no network call.
- Static import closure of dist/main.js (852 files) excludes cli/setupCommands.js and application/Update*.js.

## (b) Spawn targets on Ghidra path

1. java, POSIX direct launch: dist/ghidra/GhidraLauncher.js:85-99 → dist/process/ProviderProcess.js:36; command from dist/ghidra/GhidraJavaLaunch.js:8. POSIX without javaHome throws (GhidraLauncher.js:83-84).
2. java LaunchSupport `-envvars` / `-vmargs`: dist/ghidra/GhidraJavaLaunch.js:23,38.
3. java `-version` probe: dist/ghidra/GhidraInstallation.js:396 (caller :173).
4. cmd.exe `/c analyzeHeadless.bat`, win32 only: dist/ghidra/GhidraLauncher.js:193.
5. ps, POSIX cleanup: GhidraLauncher.js:210 → dist/process/ProcessOwnership.js:46 → dist/process/ProcessOwnershipObservation.js:149,268.
6. taskkill.exe, win32 only: dist/process/ProcessOwnership.js:31.

analyzeHeadless is never executed on POSIX (only its directory is used, GhidraJavaLaunch.js:6).

## (c) Disable switches

None found: no process.env UPDATE/OFFLINE/NETWORK/CHECK/DISABLE hits in dist or scripts. The update check runs only on explicit `rea update`, so no switch is needed for mcp.
