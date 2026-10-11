# S1 Ghidra session footprint (rea-agents 6.3.0, POSIX)

Anchors are SRC-relative (dist/...).

## (a) Paths
- Root: `tmpdir()`; `/tmp` fallback if dot-prefixed (ghidra/GhidraSessionRoot.js:29, 31-37; ghidra/GhidraClient.js:27)
- `rea-ghidra-XXXXXX/` via mkdtemp, chmod 0700 (ghidra/GhidraClient.js:241-245; process/PrivateRuntimeRoot.js:43-45)
- `project/ home/ tmp/ cache/ config/ data/`, mode 0700 (ghidra/GhidraLauncher.js:291)
- Socket `/tmp/rea-ghidra-socket-XXXXXX/s`, 0700, only if endpoint path too long (ghidra/GhidraTransport.js:11, 15-18)

## (b) Writes
- Session: `session.json`, `ownership.json` 0600 (ghidra/GhidraLauncher.js:45, 113, 298); `ghidra.log`, `script.log` (:267-268); `target-<sha12><ext>` 0600 (ghidra/GhidraTargetSnapshot.js:14, 31-32); `extensions/*.jar` (ghidra/extensions/GhidraExtensions.js:143-150)
- Per call: snapshot via `writeAnalysisSnapshot` (application/binary/AnalysisSnapshotFiles.js:18-26); evidence via `writeEvidenceBundle` (application/EvidenceBundleFiles.js:39). Both use `publishFile` (application/JsonFiles.js:78-102): stage `.rea-write-*/content` 0600 beside the caller path, then rename or link, then rm.

## (c) Cleanup
- close_binary: ghidra/GhidraClient.js:161 -> #requestClose:199 -> #close:376 -> #cleanup:390 -> #removeRuntimeRoots:461-475 -> `PrivateRuntimeRoot.close` rm -rf (process/PrivateRuntimeRoot.js:57-59)
- Normal exit: no removal path found (OPEN).
- Crash/SIGKILL: no removal code.

## (d) Orphans: YES
No exit/signal handler or startup sweep for `rea-ghidra-*` in dist/ghidra or dist/process. Staging `.rea-write-*` dirs also orphan if killed mid-write.
