# Master review audit plan: post-quota, find-only

Status: **Phase 0 prepared; canary PASSED (variant A, 2026-10-10). Awaiting go-ahead for Map.** Only canary calls and the fact runner have run; no Map or Reduce call has been made.
Nothing in this effort edits source, deletes files or commits.

| Item | Value |
|---|---|
| Boundary (single parameter `BOUNDARY_UTC` in `prep.py`) | `2026-10-07T01:05:42Z` (last Claude Code assistant turn) |
| Range | `11e1284..22b56b4`, **61 commits** |
| Pinned HEAD | `22b56b427125e12bb7b2914d96b820973d06c445`, tree `4073f9d94cf067e4fc23beea103920eddc7130c6`. `prep.py` stops with an error if `git rev-parse HEAD` differs. |
| Committed files | 266 in range, **265 after the security exclusion** (`tests/static/raw_addresses.toml` dropped by pathspec) |
| Post-boundary dirty files (recounted) | **81** in scope (22 modified, 19 modified and also committed in range, 40 untracked). The report's 84/85 included 4 audit artifacts (`post_quota_change_report.md`, `preflight_report.md`, `scope.json`, `pending-agent-review.md`). |
| Out of scope | 77 dirty files at or before the boundary; 42 `tests/.cache`; 46 `dump_excerpts`; this audit's own files |
| Files in scope (union) | 328: class a (review) 155, class b (docs) 54, class c (generated/binary/stale) 118, excluded 1 |
| Coverage units | 735: **review 522**, mechanical 40, skipped 172, excluded 1 |
| Packets | **79** (wp1 16, wp2 3, wp3 5, wp4 14, wp5 6, wp6 16, wp7 19); est. content 432K tokens at bytes/3; largest 23.2K |
| Snapshot | worker tree `~/.cache/kotor2_review/worktree/`, 3,330 entries, manifest sha256 `3226bc7667e47dd0…`. Every finding maps to this snapshot (`.claude/review/snapshot.json`). |
| Auth | **claude.ai OAuth** (`claude auth status`), not an API key. Dollar figures are API-equivalent estimates; real cost is plan usage. |

## 1. Conflicts and deviations (safest choice taken; please confirm)

1. **Model calls in this session.** The brief said none. You asked mid-session to use Sonnet 5.5 and Haiku 5.5 where sensible. I used 2 Sonnet subagents to write the deterministic helpers and `merge.py`/Reduce runner, and 1 Haiku subagent to spot-check packets against the snapshot. **No audit calls** (canary, Map, Reduce) were made.
2. **Haiku 5.5 rates differ from the brief's figures.** Cited from the pages fetched on 2026-10-10 (§8). Base input is $0.10/MTok (≤100K), not the 0.125 the brief listed first; 0.125 is the 5-minute cache write.
3. **`AGENTS.md` and `CLAUDE.md` are not mirrors** (`cmp` differs at byte 3). Both are reviewed, together in `wp1-001`.
4. **`$HOME` include path is in `level_up_override_src/test.toml:4`**, not `manifest.toml`. The post-quota report §3.2 is wrong here and the brief is right; `manifest.toml` only bumps the version.
5. **`scripts/test_pre_install_gate.py` is not in scope** (unchanged since `11e1284`, clean). The fixture line sits outside every packet. Workers may read it as a cited callee of `pre_install_gate.py`. Known issue 1 can still score through `pre_install_gate.py` (merge.py accepts either file).
6. **`--safe-mode` + `--settings`**: the CLI help does not say whether `--settings` deny rules survive `--safe-mode`. The canary decides (variant A = `--restricted --safe-mode`, B = `--restricted` only). `run_map.sh` and `run_reduce.sh` use whichever variant the canary proved.
7. **Extra guards not in the brief:** `--no-session-persistence` (workers write no transcripts); deny rules also cover the real repo's absolute path, `~/.claude`, `~/.config`, `~/.ssh` and `**/.git/**`; `defaultMode: dontAsk` is set in the settings file as well as the flag; the worker tree is read-only on disk.
8. **Suffix rule over-match:** the `tests/static/raw_addresses.toml` exclusion also drops 4 test-fixture copies under `scripts/testdata/static/raw/*/`. None is in scope; they are only absent from the worker tree.
9. **`--json-schema` rejects a `$schema` key** (CLI 2.1.296: "no schema with key or ref https://json-schema.org/draft/2020-12/schema"). It was removed from `worker_schema.json` and `reduce_schema.json` after the first canary run failed on it. The same run showed `--max-budget-usd` is enforced under OAuth (`error_max_budget_usd`).
10. **All Map calls use `--output-format stream-json --verbose`** (brief: json, stream only for the canary). This records per-request usage, so the >100K count and the cost are exact per API request, not a per-call proxy. Token cost is the same.

## 2. Security (three layers, all in the scripts)

**Layer 1: sanitized worker tree (the real guard).** `prep.py build_tree()` walks the working tree in Python and copies with `rsync --files-from`, outside the repo. Excluded from the copy:
- security patterns, case-insensitive on every path component: `.env`, `*.pem`, `.credentials.json`, `endpoint.env`, `*credential*`, `*secret*`, `sunshine/credentials/`, `tests/static/raw_addresses.toml`;
- binaries (extension list, NUL bytes, >2 MB) and `tests/.cache`, `.venv`, top-level `logs/`;
- `.git` at any depth (git history would still hold excluded files);
- `.claude/review/`, `.claude/reviews/`, this plan, `.opencode/`, `.agents/`, `.claude/skills|hooks|agents/`, `.claude/settings*.json`, `.mcp.json`;
- symlinks that leave the repo or point at an excluded or non-file target. The 33 in-tree links are kept.

`assert_tree_clean()` then runs `find` for every forbidden name, path and escaping link, and fails loudly on any hit (it passed). The tree is history-free. `CLAUDE.md`/`AGENTS.md` stay as reviewable files.

**Layer 2: deny rules.** `.claude/review/worker_settings.json`, passed with `--settings`. Each pattern is written as `Read(p)`, `Read(**/p)` and `Read(//**/p)`. Edit/Write/Bash/WebFetch/WebSearch/Agent/Task/Skill are also denied. Source: https://code.claude.com/docs/en/permissions, fetched 2026-10-10:
- "Read and Edit rules both use gitignore pattern syntax"; `//path` = "Absolute path from filesystem root"; "A pattern like `/Users/alice/file` isn't an absolute path. The single leading slash anchors at the settings source".
- "`Read(.env)` or `Read(**/.env)`: any `.env` at or under the current directory"; "`Read(//**/.env)`: any `.env` anywhere on the filesystem".
- "Claude makes a best-effort attempt to apply `Read` rules to all built-in tools that read files like Grep and Glob"; "Grep and Glob search the directory the `path` argument resolves to. Claude Code applies `Read` deny rules to that directory."
- Glob path rules are "never consulted"; use `Read(...)` instead (the script does).

**Layer 3: canary** (`.claude/review/tools/canary_test.sh`, runs after go-ahead). It builds a dummy mirror at `~/.cache/kotor2_review/canary/mirror/`. Unique markers sit at the real patterns:
- `.env`, `tests/static/raw_addresses.toml`, `sunshine/credentials/cakey.pem`, `.config/orca/agent-hooks/endpoint.env`;
- a `*credential*` file and a `*secret*` file;
- a `../outside_marker.txt` traversal file, and an absolute marker in `~/.cache/kotor2_review/canary_scratch/`.

It also adds a control file, which must be readable so the test cannot pass vacuously, and a `CLAUDE.md` probe instruction. One worker runs with the exact Map flags, worker prompt and schema. It tries Read on each path, Grep (no path, `.`, `.config`) and Glob. The script then scans every saved stream byte for any marker, so it does not rely on the model's claims. `canary.json` records:
- whether markers leaked, the control was readable, and the init tool list is exactly Read/Grep/Glob;
- whether MCP servers are empty, any hook events fired, and the CLAUDE.md probe stayed silent;
- authentication and usage presence;
- a **budget probe**: a second call at `--max-budget-usd 0.0001`, recorded as `budget_enforced`, which confirms whether the cap stops a call under OAuth.

**Deny probe.** The baseline call uses the real worker prompt, whose rules make Haiku refuse the probe steps itself, so it cannot show that the deny rules work. A second call per variant therefore uses `canary_probe_prompt.md` (attempt every step; a denial is a good outcome). The check `probe_attempted_all_steps` requires at least 14 probe tool uses covering Read, Grep and Glob, and `deny_ok` requires it. The CLI's own `StructuredOutput` tool is allowed in the tool-list check. First full pass: variant A, all checks true, no marker leaked in any saved stream byte.

If variant A fails, the script tries variant B. If deny rules fail on both but the tree is clean, `canary.json` sets `layer1_only: true`. `run_map.sh` then refuses unless you set `ALLOW_LAYER1_ONLY=1`. The canary never points a worker at a real repo path.

**Human-only list** (generated: `.claude/review/human_only.json`, names only): `83354c0 tests/static/raw_addresses.toml`. AGENTS.md requires a prior `query_ghidra.py address <hex>` lookup for it. Known gap: the fact runner cannot re-run `check-raw-addresses` (it needs that file).

## 3. Files

| File | Role |
|---|---|
| `.claude/review/prep.py` | Phase 0 (deterministic). Subcommands: `build`, `verify-snapshot`, `render <id>`, `pending`, `is-complete <id>`, `resize --from runs/<id>.json`, `build-tree <dest>`. |
| `.claude/review/INTERFACES.md` | File-format contract between the scripts |
| `.claude/review/worker_system_prompt.md` | Static Haiku prompt (contract, calibration, generic checks, distilled Gate Invariant and include-style rules). Byte-identical on every call. |
| `.claude/review/worker_schema.json` | Worker output schema (enforced with `--json-schema`; re-validated by `tools/record_run.py`) |
| `.claude/review/run_map.sh` | Idempotent Map runner (waves, backoff, budget cap) |
| `.claude/review/merge.py` | Coverage check, stable IDs, clustering, recall scoring, cost, final ledger |
| `.claude/review/tools/classify_hunks.py` | Mechanical-hunk classifier → `work/classifier_verdicts.json` |
| `.claude/review/tools/scan_patterns.py` | Pattern scanners → `work/scanner_candidates.json` (sentinel range, creature lifecycle, pointer math, hook detours, export without definition) |
| `.claude/review/tools/assertion_delta.py` | Assertion counts and removed assertions per test file → `work/assertion_delta.json` |
| `.claude/review/tools/record_run.py` | Parses one raw run, validates, writes `findings/` and `runs/` |
| `.claude/review/tools/canary_test.sh`, `fact_runner.sh`, `run_reduce.sh` | Canary, optional checker facts, Reduce sessions |
| `.claude/review/reduce_prompt.md`, `reduce_schema.json`, `withheld_leads.md` | Reduce (Sonnet) inputs. `withheld_leads.md` holds the known-issue explanations; it never reaches workers (the directory is stripped from the worker tree and never rendered into packets). |

Outputs of `prep.py build`: `snapshot.json`, `work/hunks.jsonl`, `work/*.json`, `packets/<id>.json` + `.txt`, `coverage_plan.json`, `human_only.json`, `worker_settings.json`, `facts/npc_assets.json`, `prep_summary.json`. Reruns are byte-identical (checked: packet set sha256 `42c691b1…` twice).

## 4. Phase 0: what prep did

- **Scope:** `git diff --name-status -M 11e1284 22b56b4` plus `git status -uall` filtered by `lstat` mtime > boundary. The audit artifacts, `tests/.cache` and `dump_excerpts` are removed. Every `git diff` call carries the `:(exclude,glob,icase)` security pathspecs. Diff sources:

  | Source | Diff used |
  |---|---|
  | Committed | net diff `11e1284..HEAD` |
  | Committed and dirty after the boundary | `11e1284..WORKTREE` (one unit covers both) |
  | Dirty only | `11e1284..WORKTREE` |
  | Untracked | read directly as a whole file (nothing is ever `git add`ed) |

  `83cb016` is not reachable and is not in the range. `latest.patch` is a skipped class-c file, not a scope source.
- **Classes:** (a) code, headers, TOML, `.def`, tests, build scripts, plus `AGENTS.md`/`CLAUDE.md` as WP1 policy files; (b) docs, 54 files, reason `docs-out-of-scope`: **doc consistency is not audited**; (c) 108 binary (75 `.tpc`, `.mdl`/`.mdx`, `.tga`, `.2da` that git reports as binary, `.db`), 7 generated data / agent state (ledgers, priors, benchmarks), 2 stale agent artifacts, 1 `.txi`. Full list in `prep_summary.json` (`skipped_docs`, `skipped_other`).
- **`57837e0` binary assets:** not read. `facts/npc_assets.json` lists each one's size, blob and commit, and whether its name is referenced by the dispatcher, manifest or generator scripts. This file is attached to every wp5 packet.
- **Hunks:** coverage unit = one `git diff -U0` change block (or one file-level unit for untracked, symlink, rename and binary files). Packet content uses `git diff -W` function-context hunks with post-image line numbers on every line. **Function boundaries come from git's default funcname heuristic, not ctags (ctags is not installed).** An oversized `-W` hunk falls back to `-U12` hunks in that region, then to line-range chunks. A unit split across packets counts as reviewed only when every packet holding it completes.
- **Mechanical (40 units):**
  - 27 `symlink-in-tree`: the `d8438c6` parity `stub/_shared/*` and `TestHarness.h` links, each with resolved target and sha256.
  - 11 `comment-only`, 2 `whitespace` (added blank lines).
  - 1 `include-style`: `guikit_test.cpp`, basenames identical, assertion counts unchanged, proof recorded.

  The other 35 include-style attempts failed: most `d8438c6` test hunks also rewrite the `CHECK` macro, and `apply_test.cpp` drops `failures %d` from its output. That is not mechanical, so those hunks go to wp6-009..012. Comment-only hunks keep their mechanical verdict but are also shown as context in their file's packet, so comment/code drift stays visible.
- **Scanners and assertion delta:** candidates attached to packets by file and line, `source: scanner`, verified in Reduce. Assertion delta is attached to every packet holding a test file.
- **Domains:** precedence wp1 (gate scripts and their tests, `tests/static|abi`, `tests/compat.toml`, policy files) → wp6 (other tests, `tools/parity/**`) → wp2 (dirty hooks/exports/`_shared_src`/`queue_hud_src`) → wp3 (dirty action_hotbar/companion_selfheal/level_up_override/inventory_3d_viewport) → wp5 (npc diversity) → wp4 (files touched by `c5c67f8..7aeb296` or the provider dirs) → wp7 (rest). WP7 therefore also holds the remaining agent tooling scripts (`subagent_prompt.py`, `clef_*`, `scaffold_mod.py`, …).
- **Leads:** generic checks live in the worker prompt and domain notes. Per-domain specifics that are not known issues were added: the `0x00775703` detour (wp2); the leader heal button, KpLog port and include-path portability (wp3); no gate row names npc_diversity (wp5). The four known-issue explanations are withheld (`withheld_leads.md` → Reduce only). Checked by grep that the static worker text and domain notes carry no `py:x`, `normalizeSaveFolder`, `isExile`, `subclassers` or `kind` hints. The packets themselves still contain the code.
- **Sizing:** initial guess 25K content tokens = 75,000 bytes at bytes/3 (`PACKET_TOKENS`, `BYTES_PER_TOKEN`). After the canary and first packet, `prep.py resize --from runs/<first>.json` fits `first-turn prompt tokens = overhead + bytes/ratio` through the two measured points. It writes `params.json` and re-packs only packets without a completed run (files with any completed packet stay frozen). No model call.

## 5. Phase 1: Map (Haiku 5.5)

Each call, cwd = worker tree, prompt on stdin, packet content last:
```
claude -p --model claude-haiku-5-5 --restricted --safe-mode \
  --settings .claude/review/worker_settings.json --tools "Read,Grep,Glob" --permission-mode dontAsk \
  --max-budget-usd 0.30 --json-schema "$(cat worker_schema.json)" \
  --strict-mcp-config --mcp-config '{"mcpServers":{}}' --disable-slash-commands --no-session-persistence \
  --output-format stream-json --verbose --append-system-prompt "$(cat worker_system_prompt.md)"
```
- **Bounds:** `--max-budget-usd` per call (whether it stops a call under OAuth is confirmed by the canary's budget probe), `timeout 900`, and a 12-tool-call budget stated in the prompt (`--max-turns` does not exist in CLI 2.1.296). `--bare` is not used because it skips OAuth.
- **Waves:** `xargs -P $PARALLEL` (default 8). A rate-limit/overload error (checked in stderr and the final result event only) retries that packet up to 3 times with exponential backoff and jitter. More than 2 hits in a wave halves PARALLEL (floor 2). A total cap `MAP_TOTAL_BUDGET_USD` (default 15) stops between waves. Failures stay pending for `--only` re-queue and are never dropped.
- **Idempotent:** a packet whose `runs/<id>.json` is complete (status ok, schema-valid, not truncated) is skipped.
- **Records:** `findings/<id>.json` (worker JSON + `_usage`), `runs/<id>.json` (files, exit code, attempts, schema validity, truncated flag, usage, per-request usage, cost), raw streams in `raw/`.

## 6. Phase 2: Reduce (Sonnet 5.5)

`tools/run_reduce.sh <name> <wp list>`: the compact findings bundle (`merge.py --emit-bundle`) plus `reduce_prompt.md` with `withheld_leads.md` substituted.
- Same isolation variant, deny rules and worker tree as the Map, checked against `snapshot.json`.
- Re-verifies only high/critical findings by reading the cited lines in the snapshot. Each is marked confirmed, downgraded or rejected with a reason.
- Adds cross-file findings: hook ↔ state lifecycle, shared header ↔ dependents.
- Writes `.claude/reviews/domain_<name>.{json,md}`.

**Session count is decided after the Map** from the number of high/critical findings. Default: 2 sessions (wp1+wp2+wp6, then wp3+wp4+wp5+wp7), or 1 if fewer than about 40 high/critical findings. Sonnet spawns nothing.

## 7. Phase 3: Merge (script)

`python3 -I .claude/review/merge.py` writes `.claude/reviews/master_findings.{md,json}` and `coverage_report.{md,json}`.
- **Coverage** is computed from `coverage_plan.json` + `runs/` + `classifier_verdicts.json`. Each hunk gets exactly one verdict: reviewed, mechanical, skipped or excluded. Zero or two verdicts, or any gap, fails the check (exit 1). Results roll up per file, and a re-queue list is produced. `files_examined` is used only as a cross-check.
- **IDs and clusters:** stable IDs are `sha256(file|start-end|category|evidence[:80])[:12]`. Findings in the same file and category with overlapping lines are clustered. Unconfirmed scanner candidates are carried as `source: scanner`.
- **Recall:** the answer key lives only in `merge.py`. A miss is added to the ledger as `source: known-issue`. One re-queue per miss is allowed only with your approval.
- **Ledger fields:** ID, files, lines, category, severity, confidence, verification status, evidence, investigation steps, cluster tag, plus ranked top risks.
- **Cost:** totals, cache read vs creation, requests over 100K, and estimated spend (§8).

A dry run against today's state behaves as designed: 522 gaps, all "missing run", exit 1.

## 8. Cost estimate (rates cited; spend is an estimate)

The rates below are from https://platform.claude.com/docs/en/models/haiku-5-5/overview and https://platform.claude.com/docs/en/about-claude/pricing, both fetched 2026-10-10. Pricing tier rule from the pricing page: "Claude Haiku 5.5 is priced by prompt length: a request whose prompt is over 100,000 tokens pays higher prices. A request's prompt length counts all of its input tokens, including cache reads and cache writes. Each request is priced on its own."

Rates per MTok:

| Model and tier | Input | 5m cache write | Cache hit | Output |
|---|---|---|---|---|
| Haiku 5.5, prompt ≤100K | $0.10 | $0.125 | $0.01 | $0.50 |
| Haiku 5.5, prompt >100K | $0.50 | $0.625 | $0.05 | $2.50 |
| Sonnet 5.5 | $2 | $2.50 | $0.10 | $10 |

Rough estimate before measurement:

| Phase | Assumption | API-equivalent estimate |
|---|---|---|
| Canary | 2-4 tiny Haiku calls | < $0.05 |
| Map | 79 calls, ~8K fixed overhead + ~5.5K avg content + callee reads, ~6 requests/call, ~5K output incl. thinking: ~$0.007 typical, ~$0.02 heavy | **$0.6 – $2** (cap $15) |
| Reduce | 1-2 Sonnet sessions, ~80K bundle + verification reads | **$1 – $4** (cap $3/session) |
| Fact runner | no model calls | $0 |
| **Total** | | **≈ $2 – $6** |

On OAuth this is plan usage, not billed dollars. Requests over 100K should be rare: max packet 23K content + 8K overhead leaves ~70K for reads within the tool budget. `merge.py` reports the exact count. Expected wall time: 10 waves × 2-4 min ≈ 20-40 min.

## 9. Proposed deletions (not performed; ask first)

The 11 test-pollution handoffs (`.claude/handoffs/tmp*_plan.handoff.md` ×10 and `99_temp_autoresolve_test_plan.handoff.md`) and the two 0-byte `kotor2_ghidra_knowledge.db` files (repo root and `scripts/`). They are class b/c in this audit. Deleting them changes nothing here.

## 10. Launch sequence (after go-ahead)

```bash
cd ~/orca/workspaces/local-setup/KOTOR-II
python3 -I .claude/review/prep.py verify-snapshot          # 1. snapshot still valid (no model call)
bash .claude/review/tools/canary_test.sh                   # 2. canary (2-4 Haiku calls) -> canary.json
bash .claude/review/tools/fact_runner.sh                   # 3. optional checker facts (no model call)
bash .claude/review/run_map.sh --first-only                # 4. first packet alone, validate
python3 -I .claude/review/prep.py resize --from .claude/review/runs/wp1-001.json   # 5. re-pack pending
bash .claude/review/run_map.sh                             # 6. remaining waves
python3 -I .claude/review/merge.py                         # 7. coverage + recall + cost (pre-Reduce)
bash .claude/review/tools/run_reduce.sh gate_tests wp1,wp2,wp6      # 8. Reduce (count decided after 7)
bash .claude/review/tools/run_reduce.sh behaviour wp3,wp4,wp5,wp7
python3 -I .claude/review/merge.py                         # 9. final ledger
```
If HEAD or the working tree changes before step 1, rerun `python3 -I .claude/review/prep.py build` (`build` refuses if HEAD moved off the pinned commit).

## 11. Agents used in this session

| Agent | Task | Result |
|---|---|---|
| Sonnet 5.5 (native) | wrote `classify_hunks.py`, `assertion_delta.py`, `scan_patterns.py` | DONE; self-tests 13/7/8 checks pass |
| Sonnet 5.5 (native) | wrote `merge.py`, `run_reduce.sh`, `reduce_prompt.md`, `reduce_schema.json` | DONE; self-test passes. The category enum was re-synced to the worker schema by Opus. |
| Haiku 5.5 (native) | spot-checked 7 packets: 21 sampled lines vs snapshot, headers, secret patterns | PASS (21/21, 0 hits) |


## 12. Verify stage (Haiku verifier -> Sonnet adjudicator)

Runs AFTER Reduce + merge (step 9 of section 10). Find-only like the rest: nothing is fixed, no new access, no model call
is made until the commands below are run by the user. All new files live under `.claude/review/verify/` plus
`tools/record_verify.py`; the only other outputs are `.claude/review/work/verify_{queue,key}.json` and
`.claude/reviews/verified_findings.{json,md}`.

**Design.** (1) `verify/select.py` (deterministic) picks the findings to check from `master_findings.json`:
verification in {confirmed, downgraded, unverified (non-scanner)}; `unverified-scanner` is excluded; the one `rejected`
finding and the recall-key matches (found with `merge.compute_recall`) are kept as CONTROLS with an expected outcome; the
missed key item is queued with a neutral generic claim (the key text never leaves `merge.py`). It tags each item with a
specialty (hook-asm, test-weakening, gate-integrity, python-tooling, cpp-logic, build-config), sorts severity desc then
file, and adds 8 DECOYS (a real claim re-pointed at an unrelated region: line range shifted by 400, or another file).
Decoy ids are 12-hex like real ids; the label `dcy-N`, the controls and the key mapping exist only in
`work/verify_key.json`. (2) `run_verify.sh`: one Haiku 5.5 call per item with the SAME lockdown as `run_map.sh`
(canary pass, `prep.py verify-snapshot`, `worker_settings.json`, `--restricted --safe-mode`, Read/Grep/Glob, dontAsk,
MCP off, no slash commands, cwd = sanitized tree) and the same backoff, waves and idempotence. The verifier prompt
(`verify_worker_prompt.md` + `specialty_notes/<specialty>.md`, <=14 tool calls) makes it restate the claim, try to refute it
first, record every counter-evidence search, and back each conclusion with verbatim quotes (file + 1-indexed line);
verdicts: reproduced | refuted | inconclusive | intended-behaviour. (3) `quote_check.py` (script, no LLM) confirms every quote
exists within +-3 lines of its cited line in the tree (path-safety: no absolute, `..`, or excluded paths); results land in
`verify/results/<id>.check.json`. (4) `build_dossiers.py` groups verified items by (specialty, file) into batches
(<=8 items, raised up to 12 if that is needed to stay <=30 batches; identical claim texts never share a batch) and writes
`work/adjudicate/batch_NN.md` with the claim, Reduce record, Haiku verification, quote check and a +-15 line excerpt.
(5) `run_adjudicate.sh`: one Sonnet 5.5 session per batch, lockdown identical to `tools/run_reduce.sh` (stream-json output
so cost is priced per request). It must re-open the source with Read for every item Haiku reproduced at high/medium severity,
every item Haiku refuted or left inconclusive, and every item with a quote problem, playing devil's advocate against Haiku's refutations, and returns per item
final_verdict (confirmed | refuted | needs-human), final_severity, reasoning, human_check_steps, evidence_quotes, plus a
per-batch `haiku_quality`. (6) `verify_merge.py` writes the tiers CONFIRMED, CONFIRMED-BY-SONNET-ONLY,
CONTESTED-NEEDS-HUMAN, REFUTED (real findings only) and a calibration block: decoy catch rate (a confirmed decoy is a loud
failure), control agreement, K1..K4 recall through the existing key, Haiku-vs-Sonnet agreement (overall and per specialty),
fabricated-quote counts, cost; it exits 1 and lists re-queue ids if any queue item (decoys and controls included) lacks a
verifier result or an adjudication.

**Files.** `verify/{select.py, vcommon.py, verify_worker_prompt.md, specialty_notes/*.md (6), verify_schema.json,
quote_check.py, run_verify.sh, build_dossiers.py, adjudicate_prompt.md, adjudicate_schema.json, run_adjudicate.sh,
verify_merge.py}`, `tools/record_verify.py` (reuses `record_run.py`). Formats: INTERFACES.md, last section. Every script
honors `VERIFY_DIR` (data dir, default `.claude/review/verify`), `VERIFY_WORK`, `KOTOR2_REVIEW_HOME` and
`KOTOR2_VERIFY_OUT`, so a test run never touches the real result directories.

**Launch (after go-ahead; run from the repo root; each step is idempotent).**
```bash
python3 -I .claude/review/verify/select.py                      # 1. queue + key (no model call; add --max N to cap)
bash .claude/review/verify/run_verify.sh --dry-run               # 2. prompt render + exact command, no model call
bash .claude/review/verify/run_verify.sh --first-only            # 3. one Haiku call; inspect results/<id>.json + .check.json
bash .claude/review/verify/run_verify.sh                         # 4. all verifiers (PARALLEL=8, total cap $6)
python3 -I .claude/review/verify/quote_check.py --all            # 5. (re)check every quote (run_verify already checks each)
python3 -I .claude/review/verify/build_dossiers.py               # 6. batches (add --allow-partial if some verifiers are missing)
bash .claude/review/verify/run_adjudicate.sh --dry-run           # 7. batch list + cost estimate
ADJUDICATE_TOTAL_BUDGET_USD=25 bash .claude/review/verify/run_adjudicate.sh   # 8. Sonnet sessions (default cap $5 stops early)
python3 -I .claude/review/verify/verify_merge.py                 # 9. verified_findings.{json,md}; exit 1 lists gaps to re-queue
```
Re-queue: `run_verify.sh --only id1,id2`, `run_adjudicate.sh --only batch_03`. `build_dossiers.py` refuses to renumber
batches once adjudications exist (use `--force` only after deleting them).

**Cost estimate (API-equivalent, estimates).** Queue of 275 items (267 findings + 8 decoys). Haiku verifier: about
$0.01-0.03 per item (Map workers averaged $0.0076 with ~17K-token first turns; a verifier makes up to 14 tool calls on a
smaller prompt) = about $3-8, per-call cap $0.12, total cap default $6. Sonnet adjudication: about 31-35 batches (8 items
per batch, up to 12 if needed to stay near 30) at the observed Reduce cost of $0.25-0.75 per big session (the 32-turn
behaviour session cost $0.75) = about $8-26; per-session cap $1.50 and the DEFAULT total cap of $5 will stop early, so set
`ADJUDICATE_TOTAL_BUDGET_USD` deliberately. Expected total about $11-34. All of it is plan usage on the OAuth login (CLI
cost_usd is an API-equivalent estimate); Sonnet cache-write is priced at 1.25x input in `verify_merge.py`.

**Security notes.** Same worker tree (`~/.cache/kotor2_review/worktree`, outside the repo, secrets stripped, pinned
snapshot), same `worker_settings.json` deny rules, same tools (Read/Grep/Glob), same isolation flags, same canary: the flags
are identical to Map and Reduce, so the existing canary result applies and no new access is granted. The runner refuses to
start without `canary.json` pass, a matching snapshot and a tree outside the repo. Claims and verifier output are treated as
data. `select.py` and `quote_check.py` apply `prep.is_excluded` (plus a root-level `.env` case, see below) and redact
excluded-path tokens in claim text. The answer key stays in `merge.py` (recall text) and `work/verify_key.json`
(ids/labels only); prompts, specialty notes, dossiers and the queue never contain the key text (checked by script).
Decoys and controls are not marked in any dossier. Nothing is written to `.claude/master_ledger.jsonl`, the game
directory or any install path; no `--apply` is involved.

**Notes found while building.** (a) `prep.is_excluded(".env")` returns False because of `lstrip("./")` turning it into
`env`; a root-level `.env` is only caught by the other layers (`vcommon.is_excluded` also tests it behind a dummy
directory). (b) `verify/select.py` shares its name with the stdlib `select` module; the Verify scripts therefore append
(never prepend) their own directory to `sys.path`. (c) Findings that name the recall-key topics contain those words in
their claim text (that is the worker's own wording, not the key).
