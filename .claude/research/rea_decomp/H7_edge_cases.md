# H7: operational edge cases for the REA pipeline over swkotor2.exe

Read-only recon, 2026-10-15. REA and Ghidra were not started. Hashes were computed only on the vanilla backup (`~/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe`, 6588416 bytes). The game folder was not touched.

Sources: `steam_db_batches/B13a/rea_pilot/PILOT.md`, `steam_db_batches/DESIGN.md` s.4.9 and s.5, `steam_db_batches/tools/rea_questions.py`, `ghidra_knowledge/rea_findings/README.md`, `~/.claude/agents/rea-recon.md`, memory `rea-integration.md` and `kotor2-vanilla-exe-migration.md`, `ghidra_knowledge/RE_AGENT_GUIDE.md` s.3-4, `scripts/query_ghidra.py` load_ledger.

Ledger scan: 11 files matching `2026*.tsv`, 180 rows, 127 distinct addresses. Kinds: function 64, xref 46, callsite 33, global 14, vtable 11, string 5, field 4, data 2, table 1. Confidence: confirmed 153, probable 24, refuted 3.

Status key: CODED (handled in code or doc), DOC-ONLY (written down, not enforced), MISMATCH (sources disagree or are wrong), GAP (nothing covers it), PARTIAL, PASS (checked clean), OPEN (needs a decision).

## Identity and exe hashes

| ID | Category | Symptom | Cause | Mitigation (proposed) | Pinning test (proposed) | Status and evidence |
|---|---|---|---|---|---|---|
| E1 | Exe identity | DESIGN 4.9 calls 6a522e71 the "Steam md5". Backup md5 is 2f2d31e2, sha1 2a9f5b44, crc32 97cd8657. The sha256 prefix is 6a522e71. | Doc mislabels the algorithm. Memory notes use the same 8-hex id. | Relabel as sha256 prefix. Gate on the full sha256. | sha256(BINARY) equals the pinned full digest | MISMATCH: DESIGN.md:322 |
| E2 | Exe identity | Question builder opens whatever BINARY says. Nothing checks it. | BINARY is a hard-coded path with no digest check (rea_questions.py:30). | Preflight in the builder: sha256 must match, else exit 2. Agent echoes the digest into evidence. | Wrong-digest fixture gives exit 2 | GAP |
| E3 | Exe identity | An agent can open any absolute path. The live exe is never checked. | rea-recon.md only requires an absolute path and provider ghidra. | Brief pins the path. Check the open_binary argument in the run transcript. | Grep open_binary args in each transcript | GAP: rea-recon.md rules |
| E4 | Exe identity | Live exe = vanilla + 4 GB patch. Twin addresses on live are unchecked. | Patch may touch code bytes. | Keep REA on vanilla only. Before any live use, compare twin bytes with read_bytes. | read_bytes diff of 0x775930/50/70/90 on both exes | OPEN: kotor2-vanilla-exe-migration memory |

## Concurrency, JVM, memory

| ID | Category | Symptom | Cause | Mitigation (proposed) | Pinning test (proposed) | Status and evidence |
|---|---|---|---|---|---|---|
| C1 | Concurrency | 3 parallel runs died with "Connection closed" on first procedure call (round 2). | Parallel REA JVMs plus another session's REA. | Max 2-3 REA runs. One reconnect (already in rea_questions.py rule 2). | Runner counts live REA pids and refuses at 3 or more | DOC-ONLY: PILOT "Ops", rea_questions.py:93 |
| C2 | Concurrency | DESIGN allows "<= 12 parallel" rea-recon runs. PILOT caps at 2-3. | Stale design text. | One cap constant in the runner. Grep test that DESIGN and PILOT agree. | Constant equals the documented cap | MISMATCH: DESIGN.md:320 vs PILOT "Ops" |
| C3 | Concurrency | During this audit, another session's REA ran `rea-agents function ... swkotor2.exe` (pids 1970132, 1970133, 1970372). | No per-exe lock. | Lock file per binary. Refuse to start while a GhidraClassLoader java pid exists. | Fixture process list with one match gives exit 2 | GAP: live `pgrep` |
| C4 | JVM guard | DESIGN's guard `pgrep -af '^java.*analyzeHeadless'` does not match REA's JVM (pid 1970372 cmdline has GhidraClassLoader and no analyzeHeadless). | Guard was written for the headless path only. | Guard matches GhidraClassLoader or rea-agents. | Fixture cmdline must trigger the guard | MISMATCH: DESIGN.md s.5 process rules |
| C5 | Memory | Heap ceiling and peak RSS are unknown. | No -Xmx or maxMemory anywhere in the sources. | Record max RSS per run (`/usr/bin/time -v`, already used by another session). | Run log has a max-RSS line under a cap | GAP / OPEN |
| C6 | Budget | Runs use 88k-227k tokens and 30-85 tool calls for 8 addresses. 5 of 6 good runs are over 100k. | Brief says "at most 4 REA calls per address" (rea_questions.py:98) with no counter. | Hard cap in the brief. Count calls from the transcript (agent_usage.py). | Calls <= 4 x addresses + 3 per transcript | PARTIAL: PILOT "Run cost" |

## Cold start and timeouts

| ID | Category | Symptom | Cause | Mitigation (proposed) | Pinning test (proposed) | Status and evidence |
|---|---|---|---|---|---|---|
| T1 | Cold start | First procedure call takes about 356 s. Default timeout is 330000 ms. | Ghidra cold start. | `REA_GHIDRA_STARTUP_TIMEOUT_MS=900000` in the agent env (rea-recon.md). | `/proc/<mcp pid>/environ` contains the var before the first call | CODED: rea-integration.md, rea-recon.md env |
| T2 | Config shadowing | Subagent env is ignored when a same-named global `rea` server exists, so the 900 s override is lost. | Name collision. Global rea was removed 10-10. | Grep config for a global rea entry before each batch. | Config grep returns no global rea | CODED: rea-integration.md |
| T3 | Partial output | A run takes 8.5-14 min and writes its ledger only once, at the end. A crash loses all rows. | rea-recon.md writes "AFTER the analysis". | Append rows per address, or flush after each. | Kill after 2 addresses, then expect 2 rows | GAP: rea-recon.md ledger step |

## Snapshot, staleness, locking

| ID | Category | Symptom | Cause | Mitigation (proposed) | Pinning test (proposed) | Status and evidence |
|---|---|---|---|---|---|---|
| S1 | Snapshot | open or close can write a snapshot that a later run reuses. | snapshot_path option. | Forbidden in rea-recon.md. Never pass it. | Transcript grep for snapshot_path gives 0 | CODED: rea-recon.md rule 2 |
| S2 | Temp copy | A failed run may skip close_binary, leaving a temp copy. | Error path. | Runner checks open/close pairing. Count temp copies before and after a failed run. | Paired open/close per transcript | PARTIAL: rea-recon.md rule 3 |
| S3 | Stale rows | Any confirmed `<B>-q*` row permanently skips that address. No exe digest on rows, so a changed binary is not detected. | rea_questions.py:67-78 has no provenance. | Stamp each file with the exe sha256. Skip only rows whose stamp matches. | Change BINARY digest, then confirmed rows must not be skipped | GAP |
| S4 | Builder race | Re-running the builder deletes every q_*.md, including one an agent is reading. | rea_questions.py:162-163 unlinks at start. | Refuse when a run marker exists. Write to a fresh dir. | Marker present gives exit 2 and keeps files | GAP |

## Address edge cases

| ID | Category | Symptom | Cause | Mitigation (proposed) | Pinning test (proposed) | Status and evidence |
|---|---|---|---|---|---|---|
| A1 | Thunk | A one-line JMP or MOV ECX;JMP thunk looks like a plain function. 12 false mismatches in the pilot. | ECX passes invisibly. | Resolve the thunk to its target before judging (guide 3.2). | JMP-thunk fixture resolves to target | CODED: RE_AGENT_GUIDE s.3.2 |
| A2 | ICF folding | Two Steam addresses share bytes and callees, so their REA evidence is identical and looks like a match. | MSVC COMDAT or ICF folding. | Compare entry-to-RET bytes of twins with read_bytes. Mark folded and count once. | Byte-hash uniqueness across same-size rows | GAP: no ICF text in guide, DESIGN or PILOT |
| A3 | Mid-function | A row is keyed to an interior address and labels the wrong function. | Caller passes a body address. | Q1 plus resolve_containing_procedure. Require row addr == procedure entry. | Function row addr == procedure_info entry | PARTIAL: rea_questions.py:114, tool allowlisted but not enforced |
| A4 | Jump table and data | JMP dword ptr [EAX*4+0x76ecfc] lives in 0x76eb20. Rows 0x76ecfc and 0x76ed20 are kind=data. | Table is not code. | Data and table rows need read_bytes evidence and no function verdict. | Data rows get no function verdict | PARTIAL: 2 data and 1 table rows in ledger |
| A5 | Twin family | 0x775930, 0x775950, 0x775970, 0x775990 all do MOV ECX,[EAX+0x224] then CALL. Size 25, RET none. They differ only in callee 0x85d260, 0x85d2d0, 0x85d320, 0x85d3d0. | Same signature, different callee. | Card shows all neighbours. Callee needs a ledger name before it can anchor. | score_pilot fixture for B13a-1-53 must not accept | CODED-IN-DOC: PILOT "Accepted decoy", guide 3.5 |
| A6 | Callee identity | Zero ledger rows exist for 0x85d260, 0x85d2d0, 0x85d320, 0x85d3d0. | The callees were never queued. | Queue the four callees as questions. | Every twin callee has a ledger row | GAP: ledger scan |
| A7 | Same-RET neighbour | A neighbour with the same RET N gets accepted. | Only ret and size are checked. | Invert the burden of proof: a content anchor is required (guide 4.1). | Neighbour pair fixture gives unsure | CODED: RE_AGENT_GUIDE s.4.1, 3.5 |
| A8 | ABI and offsets | 30 of 82 false mismatches came from narrow-arg slot rounding. int* offsets scale by 4. | Arg size and pointer scaling. | Pre-convert with re_card_prep.py annotate. Slot = ceil(bytes/4) x 4. | GOG 1 byte gives slot 4. `+100` on int* gives 0x190 | CODED: guide 3.1, 3.3 |
| A9 | Labels and dead code | FID labels hit game functions (7 false). An XOR/JZ dead block was called live. | Labels are hypotheses. | Ignore labels. Check the flag source before calling a block live. | Rename-invariance fixture | CODED: guide 3.4, 3.6 |

## Ledger hygiene

| ID | Category | Symptom | Cause | Mitigation (proposed) | Pinning test (proposed) | Status and evidence |
|---|---|---|---|---|---|---|
| L1 | Kind enum | 49 of 180 rows use kinds outside the README list: xref 46, data 2, table 1. | Template asks for xref. README was not updated. | Add xref, data and table to the README. Lint the enum. | Lint all TSVs, zero off-enum kinds | MISMATCH: README kind table, rea_questions.py:98 |
| L2 | Evidence contract | 31 rows have no asm quote (24 xref, 7 other). 10 rows cite no tool. | Rule 10 is not linted. | Lint `0x[0-9a-f]{6,8}: ` for function, global, vtable, string and field rows. | Lint fails on a fixture like holster row 31 | MISMATCH: template rule 10 |
| L3 | Allowlist | holster-C3 row 31 cites inspect_native_instruction, which is not in rea-recon's tool list. | Another server or agent, provenance unclear. | Lint tool names against the allowlist. Ask the lead which run wrote it. | Tool names are a subset of the allowlist | MISMATCH: holster-C3 TSV line 31, rea-recon.md tools |
| L4 | Silent skip | load_ledger drops rows with fewer than 7 cells and skips a whole file on a header mismatch. It accepts more than 7 cells and truncates. | Lenient loader. | Fail-loud lint. Reject rows with more than 7 cells. | Fixture with an extra tab must error | GAP: query_ghidra.py:26-42 |
| L5 | Basic checks | 0 bad column counts, 0 non-hex addrs, 0 empty evidence, 0 evidence over 200 chars (max 166), all dates ISO. | n/a | Keep the lint as a CI gate. | Lint returns zero | PASS: this audit |
| L6 | Conflicts | 0x006f5b40 and 0x0077a9e0 (steamdb probe) and 0x0098b3ec (vtable) each have a confirmed row and a refuted row. The loader returns both with no supersedes flag. | No supersession column. | Dedupe on (addr, kind, name) and show a refuted flag. | Query for 0x6f5b40 shows the refuted flag | GAP |
| L7 | Multi-row | 0x00456d00 has 3 field rows. That is legal, since field name = Class+offset. `_2` addenda reuse the parent topic. | No run-id column. | Dedupe key includes name. | 0x456d00 dedupes to 3 rows | PARTIAL: README field rule |
| L8 | Missing xref | 16 function addresses have no xref row in any file. Callers exist only in replies (PILOT). | Template rule 7 is not enforced. | Lint: each function row needs a sibling xref row. | Lint fails on a function row with no xref | GAP |
| L9 | Topics | Topics B13a-q1..q6, holster-C3 and steamdb-r1-probe are all distinct. next_topic_index keeps k unique. | n/a | Keep. | Unique topic per run | PASS: rea_questions.py:125-129 |

## Blindness and model reliability

| ID | Category | Symptom | Cause | Mitigation (proposed) | Pinning test (proposed) | Status and evidence |
|---|---|---|---|---|---|---|
| B1 | Blindness leak | rea-recon greps the ledger for each address before open. The 23 addresses with only probable or refuted rows are re-queued, so old names return (e.g. refuted AddFeat for 0x6f5b40). | Grep-first rule conflicts with rule 8 "do not copy names". | Grep addresses only, with names stripped. Or drop probable and refuted names from the prompt. | rg of prior names in a built question file gives zero hits | GAP: rea-recon.md step 1, rea_questions.py:67-78 |
| B2 | Real vs decoy | Question files carry no names, kinds or card ids, so the file itself is blind. The per-run real and decoy mix is not logged and drifts as confirmed addresses drop out. | No composition log. | Log composition per file. Re-check after each wave. | File has no names. Composition line present | PARTIAL: rea_questions.py:12, 81-122 |
| B3 | Twin decoy accepted | Haiku+REA accepted 1 of 20 decoys (B13a-1-53 = 0x775970). Sonnet accepted 0 of 20. | Model cannot separate twins. | Decoy-cannot-accept gate. No REA trust route (DESIGN s.4.9). | score_pilot fixture for B13a-1-53 | CODED-IN-DOC: PILOT score table |
| B4 | Recall | Haiku+REA got 7 of 14 LS rows, Sonnet 10 of 14. 4 rows were missed by every arm. | Haiku recall limit. | Sonnet escalation for twins and unsure rows. | Gate LS >= 70%, 0 decoys | CODED: PILOT score table |
| B5 | Arithmetic | Verifier reported 55 bytes. procedure_info said 57. | LLM counting. | Compute size from procedure_info or the asm range in code. | size == end - start from asm | CODED: rea-integration.md 10-11 correction |

## Open decision

- Should the exe gate pin the full sha256 of the vanilla backup in rea_questions.py (E1, E2)?
