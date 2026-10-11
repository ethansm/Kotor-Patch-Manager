# REA integration proposal

Pin: `rea-agents@6.3.0` only. Never `@latest`. Anchors: `dist/...` (rea-agents package), `src/...` (upstream clone).

## 1. Go/no-go for the human setup step

**Verdict: GO for `rea setup --client claude_code --dry-run`.** The dry-run writes nothing (`dist/application/Setup.js:117-128`; `dist/application/SetupPlan.js:160-188`). The install step waits on review of its output.

- The `supported` host flag is read at four sites: doctor host line (`src/application/DoctorDiagnostics.ts:107`), `src/application/SetupHost.ts:81`, `src/application/SetupPlan.ts:52` (Hopper install proposal), `src/application/LinuxHopper.ts:192` (install refusal). No read exists in `src/ghidra` or `src/server` (S3-upstream §b.1-2).
- Doctor exit=1 comes from the host line (`unsupported_host`, pop 24.04; A1-env.md:15, 31-35). The Ghidra checks pass (A1-env.md:21-25).
- Remaining check: the dry-run output must show only the writes listed in §4. Its stdout format is not traced (S1-setup OPEN), so review it by hand.

## 2. Tools and bridge

- Nine tool contracts (`dist/contracts/officialToolContracts.js:58` and others; S1-tools):
  - Seven are Ghidra provider operations: `procedure_pseudo_code`, `xrefs`, `procedure_references`, `analyze_function`, `procedure_info`, `search_procedures`, `search_strings`. `dist/ghidra/GhidraProviderCapabilities.js:156` sets `mutatesArtifact` false for all nine.
  - `open_binary` and `close_binary` are session tools, not Ghidra operations (`dist/contracts/sessionToolContracts.js:40-41`).
- `open_binary` accepts any readable file: no allowlist, no root confinement. Symlinks are rejected (`dist/application/BinaryTargetResolver.js:23-37`). Pilot risk: any caller can point it at any readable file.
- The bridge has 28 commands (`bridge/ghidra/ReaGhidraBridge.java:341-377`). Only `annotate_native_function` writes the program database (`setComment`, `setName`: `ReaGhidraBridge.java:1042,1046`). Writer access is unix-socket only (`:378`). Startup extensions may also mutate (`:202-204`; extension code unread).
- `annotate_native_function` is not one of the nine contracts. Whether MCP exposes it is open (§6 item 5).

## 3. What stays

- Server export: `scripts/ghidra_server_api.py` and `scripts/export_server_knowledge.sh` (A1-ours.md:2-3).
- Steam address DB `kotor2_steam_aspyr.db` (A1-exe.md:7).
- Knowledge DB and persist writers `persist_hotbar_edge_lessons.py`, `persist_re_guide_lessons.py`, `persist_selfheal_lessons.py` (A1-ours.md:4-6). The DB is a research cache, not read by the runtime KotorPatcher (A1-ours.md:8).
- `scripts/query_ghidra.py`: read-only CLI over `kotor2_ghidra_knowledge.db`. It does not call Ghidra (A1-ours.md:1).
- Tests: `scripts/test_query_ghidra.py` (A1-tests.md:20); `scripts/test_ghidra_server_api.py`, which mocks the server (A1-tests.md:23).
- KpLog: no REA interaction stated.

## 4. Setup writes, session dirs, network

**Setup writes (`--client claude_code`):**
- `~/.claude.json` (or `$CLAUDE_CONFIG_DIR`; `dist/application/SupportedClients.js:104`) gets one key: `mcpServers.rea = {command, args, env?}` (`dist/application/ClientConfigurationDocument.js:1105-1109`). Backup `~/.claude.json.rea.backup` only if the file exists and the entry is not current; never overwritten (`dist/application/SetupClientConfiguration.js:38-43,149-156`). `env` may hold runtime provider keys (`dist/application/SetupHost.js:65-70`). Check it in the dry-run.
- Skill: `$CLAUDE_CONFIG_DIR` or `~/.claude/skills/reverse-engineer-anything/`, with `SKILL.md` and five references (`dist/application/SetupSkill.js:5-12,14,30-36`). `~/.agents/skills` is skipped for a claude_code-only selection (`dist/application/SetupSkill.js:16-19,26`). Each changed existing file gets `<file>.rea.backup` (`SetupSkill.js:86-89`).
- `~/.claude/settings.json`: not written (only Gemini matches, `dist/application/SupportedClients.js:132`).
- Uninstall reverses `mcpServers.rea` and both skill dirs (`dist/application/Uninstall.js:141-170,228-229`). `~/.rea` is removed only with `--purge-data` (`Uninstall.js:83-84`).

**Session footprint (POSIX):**
- Session dir `tmpdir()/rea-ghidra-XXXXXX`, mode 0700 (`dist/ghidra/GhidraClient.js:241-245`; `dist/process/PrivateRuntimeRoot.js:43-45`).
- `session.json` and `ownership.json` at 0600 (`dist/ghidra/GhidraLauncher.js:45,113,298`). `target-<sha12><ext>` at 0600 is a copy of the exe, 6,588,416 bytes (`dist/ghidra/GhidraTargetSnapshot.js:14,31-32`; A1-exe.md:4-5).
- Per-call snapshot and evidence writes stage `.rea-write-*` beside the caller's path, then rename or link (`dist/application/JsonFiles.js:78-102`).
- Cleanup runs only in `close_binary` (`dist/ghidra/GhidraClient.js:461-475`; `dist/process/PrivateRuntimeRoot.js:57-59`). Crash or SIGKILL leaves `rea-ghidra-*` dirs and the exe copy in /tmp. No startup sweep exists. Killed `.rea-write-*` staging dirs orphan too (S1-session §d).
- Normal exit has no removal path found (§6 item 1).

**Network and spawns:**
- No network on `rea mcp` startup (`dist/main.js:40-62`) or Ghidra tool calls. Optional provider constructors loaded at startup (`dist/main.js:59-60`) were not read (S1-net OPEN). Only `rea update` uses npm (`dist/application/UpdateRuntime.js:90-100`; `dist/cli/setupCommands.js:99`).
- Ghidra TCP is loopback only (`dist/ghidra/GhidraSocketConnection.js:49-50`; `bridge/ghidra/ReaGhidraBridge.java:248`).
- Spawns on the Ghidra path: `java` (`dist/ghidra/GhidraLauncher.js:85-99`), `java -envvars` and `-vmargs` (`dist/ghidra/GhidraJavaLaunch.js:23,38`), `java -version` (`dist/ghidra/GhidraInstallation.js:396`), `ps` on POSIX (`dist/process/ProcessOwnershipObservation.js:149`).
- Windows only: `cmd.exe` (`dist/ghidra/GhidraLauncher.js:193`), `taskkill` (`dist/process/ProcessOwnership.js:31`).

## 5. Part B pilot definition

Status: proposed, not run. Preconditions: dry-run reviewed (§1); `rea setup` run after backups; `skill:identity` and `registration:claude_code` cleared (A1-env.md:51-70).

1. **Workers:** three Haiku workers, each with a different prompt style: (A) step checklist, (B) question-first, (C) minimal. Tools: REA MCP provider `ghidra` only. The provider ID comes from the brief; the docs do not list provider ID values (A1-docs.md:45).
2. **Target:** `CSWCCreature::PerformCastSpell` at 0x00775700, on the vanilla exe keyed to 6A522E71 (A1-exe.md:3-8). Do not switch to 7751F7D8 until open item 6 is resolved.
3. **Output:** each worker returns the same fixed field list. The list is set before launch from the pointer sources below.
4. **Comparator:** a scratch script checks each worker's output against ground truth the workers never see. It records field-level mismatches. A comparator Haiku then merges the three results into one table.
5. **Ground truth (pointer only):** `patch_manager_mods/29_paused_command_evidence/PQ1.md` (lines 39, 88); `patch_manager_mods/30_queue_hud.md:77`; `python3 scripts/query_ghidra.py address 0x00775700`. Values are not copied here.
6. **Results:** appended to this file under a "Pilot results" heading. Workers run in the background, per process rules.

## 6. Open items

1. Does any path other than `close_binary` reach `GhidraClient.close` on normal exit? (S1-session §c OPEN; none found in `dist/server/createServer.js:124-135` or `dist/composition/binary.js`.)
2. Is excluding Pop!_OS and Linux Mint intended? No code or doc says so (S3-upstream §a). It affects only the host line and Hopper install. Observed: `rea doctor` gives `unsupported_host` for pop 24.04; the Ghidra CLI run still worked.
3. Provider keys in `mcpServers.rea.env`? **Answered (pilot):** the server needs `GHIDRA_INSTALL_DIR` and `JAVA_HOME`. Set `REA_GHIDRA_STARTUP_TIMEOUT_MS` to `900000` for the first procedure call.
4. Does `open_binary`'s canonical path reach Ghidra's importer unchanged? (S1-bridge OPEN.)
5. Is `annotate_native_function` exposed as an MCP tool? **Partly answered (B0-tools.md):** B0 probed six tools and did not include it. Exposure stays open. (§2; S1-bridge §b.) Update: `doctor.json` lists it for Ghidra with `mutatesTarget`, `mutatesSession`, `writesFilesystem` all true. Keep it off every allowlist.
6. Live exe conflict: knowledge DB row `4ab72fc1` says live = pristine + LAA flag flip. `patch_manager_mods/04_recommended_installs.md:49` says live = `7751F7D8` (vanilla + 4gb patch). Unresolved (A1-exe.md:14).

## 7. Pilot results

**Scores (F1-F5):** evidence-card 5/5 (one credit is a scorer artifact; strict 4/5). Checklist 4/5. Free-form 3/5.
**Gate: not GO.** The gate needs two workers at 5/5; only one reached it.

**Misses:**
- F2 (calling-convention field) was missed by checklist and free-form. No REA tool returns that field, and all three workers report it unconfirmed.
- The F3 miss in free-form is a parsing artifact. The value is correct but has trailing text.

**Agreement:** the three workers agree on every other field. Differences are label wording and zero-padding of address lists.

**Calls:** 10 per worker, all ok. Timings not recorded (workers have no clock).

**Cost:** $0.0342 for four Haiku runs (cap $0.60). Contamination: none. Temp dirs after each run: none.

**Repo scripts replaced:** none shown by this pilot.

**Re-score (scorer v2, same B1 outputs, no new REA calls):** v1 gave 5/5, 4/5, 3/5 (NOT GO, above). v2 gives 4/5 for each worker. F1, F3, F4, and F5 pass in all three, so the D1 gate is **GO (3/3)**. F2 is a known REA gap: REA returns no label.
