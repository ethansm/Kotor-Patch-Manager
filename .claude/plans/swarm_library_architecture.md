# Haiku swarm library: solution architecture (v2, proposal only)

Status: PROPOSAL. No code, config or pipeline has been changed. Evidence: 4 Haiku 5.5 verifiers (25 claims) and 6 Haiku 5.5 critics (about 70 findings), run 2026-10-10. Every number below was either checked on disk by a verifier or computed from a stated formula, and the lead spot-checked three high-impact claims (sections 2.1 and 3). Raw outputs and the claim ledger are persisted in `.claude/plans/swarm_review_2026-10-10/`.

## 1. Answer first
1. **Urgent, before any port:** the fixer sandbox leaves `~/.aws`, `~/.gitconfig` and `~/.netrc` readable, and the map runner has no OS sandbox. A live fix run was active when I checked (pid 3834907). Decide D0 below before starting another run.
2. Build ONE shared library, `~/.claude/swarm/`, whose single runner is the only code allowed to call `claude -p`. This is a target. Today about 17 scripts make that call directly.
3. Four recipes sit on top: deep-haiku-review (audit, existing), haiku-fix (fix, existing), mod-ideate (new), bug-investigate (new). swarm-review (verify-and-critique, the pattern used for this document) is a fifth, generic recipe.
4. Safety invariants move out of profiles into the library's built-in deny list. Today the git and install guards exist in code but are switched on only when a profile lists them, and the default list is empty.
5. First build is the thinnest slice: a $0 argv parity probe between the two engines (section 11, step M1). Build the full runner only if the probe shows the drift matters.

## 2. Urgent safety findings (current pipelines, not the proposal)

### 2.1 Verified on disk
- **Fixer sandbox leaves home secrets readable.** `.claude/review/fix/fix_settings.py:105-112` mounts `--ro-bind / /` and then hides only `~/orca`, `~/.config`, `~/.ssh`, `~/Games`, `~/.local/share/Steam` and RHOME. `~/.aws`, `~/.gitconfig` and `~/.netrc` stay readable to the fixer and gate code. (Lead spot-check: confirmed.)
- **Fixer can rewrite Claude's own permission config.** The claude profile binds `~/.claude` and `~/.claude.json` read-write (`fix_settings.py:119-121`).
- **Map runner has no OS sandbox.** `bundle/engine/run_map.sh:167-170` relies on `--tools Read,Grep,Glob`, `--permission-mode dontAsk`, `--strict-mcp-config` and a deny file. The worker sees the whole sanitised tree, not only its packet (`prep.py:205-240`).
- **Canary cannot see a home-read regression.** `canary_test.sh:73-74` probes `../` and absolute scratch paths only.
- **No value scrubber for packets.** `verify/vcommon.py:173-187` redacts excluded-path tokens in claim text only. The repo has 85 tracked non-.txt files containing `/home/ethansm` and 3 containing Steam-ID-shaped numbers, and these are copied into packets.
- **Prompt-injection path to writes.** For non-auto-fixable classes, worker-cited evidence files join `allowed_files` and receive Edit/Write rules (`fix_intake.py:226-232`, `fix_settings.py:78-84`). Injected JSON can widen a fixer's write scope.
- **Integrator can run the gate with an editable test entrypoint.** `fixgate.sh:108-110` runs `scripts/run_tests.py`, which a fixer can edit (`fix_settings.py:98-99`).
- **Log routing is convention only.** Workers have no Bash, so `kotor2_logs.py` cannot be enforced, and Read/Grep can open raw `*_log.txt` files if they sit in the tree (`dr_profile.py:17-19` has no deny for them).

### 2.2 Decision D0 (for you)
Recommend: no new live fix or map run until the fixer and map sandboxes hide `$HOME` outside the task copy and the canary has a HOME probe. The live run (pid 3834907, `run_fix.sh --only T2a…`) I did not touch. Let it finish, or stop it, before any change under `.claude/review/fix/` or the bundle.

## 3. Corrections to my earlier report (verified)

| Earlier claim | Verified | Source |
|---|---|---|
| Deep-review engine about 1,850 lines | **12,826 lines in 33 .py/.sh files** (1,850 was a wc total that counted .md files) | V1, C02 refuted |
| About 20 files differ between the two trees | **33 entries** differ or are one-sided (22 verify, 11 tools) | V1, C04 partial |
| About two dozen scripts call `claude -p` | **about 17** real implementers (23 raw grep hits) | V1, C05 partial |
| sha256 helper in three places | **at least 10** path-hash helpers; "21 fork / 10 bundle" are mentions, not definitions, so count before citing | V1, K5 |
| Fix pilot $0.59 for 2 patches | $0.59 for a **5-task pilot** that produced 2 patches | V2, C12 partial |
| Ladder costs 28% of Sonnet-first | **64.6% including Sonnet rescue** ($0.028 vs $0.098 per parent before rescue) | K3, K5, memory |
| Ladder rule passed; "rounds 2-3 added nothing" | Strict adoption rule **failed** (64.6% vs 60%, n=12). Rounds 2-3 cost **$0.04** and gained **0 accepts** | K3 |
| D3 "measured" | The experiment compared a fix-pipeline ladder against Sonnet-first. It did **not** compare 2 vs 3 Haiku layers | K3 |
| `sum_cost` reads total_cost_usd | It reads `cost_usd_total`, then `attempts[].cost_usd`, then `cost_usd`; total_cost_usd is a 4th fallback (`dr.py:440-455`) | V3, C16 partial |
| Dry-run plans 6 files | 6 files plus the plan doc = 7 entries; doc 37 free; auto-pick has no collision check (`scaffold_mod.py:45-51`) | V3, C18 partial |
| Guards are enforced | Only when a profile lists them; default `forbidden_commands` is empty (`dr_profile.py:37`); `engine/prep.py:1228,1237` run git unguarded | V4, C22 partial |
| Credential rule is global | It is in the **project** CLAUDE.md (:18-20) | V4, C20 partial |
| `--bare` is used | It is **not** used. It requires API-key or apiKeyHelper auth (OAuth excluded), so do not adopt it on subscription auth | K2, K4 |

Confirmed as stated: fix-* stages refuse with exit 2 (C01); vcommon 273 vs 554 and canary 177 vs 238 (C03); zero-byte ghidra DB copies (C07); effort settings haiku xhigh / sonnet high (C08); $9.2 and 2.5 h audit (C11); no temperature or seed flag (C13); `--bare` skips CLAUDE.md auto-discovery (C14); CLAUDE_CODE_EFFORT_LEVEL overrides --effort (C15); 21 live logs and a 45 MB perflab_metrics.txt (C17); doc 37 free (C25).

## 4. Problems to solve (ranked, verified)
1. **Safety is opt-in and partly unenforced** (section 2).
2. **Two engines drift.** The fork passes **no `--effort`**, while the bundle passes `--effort` per call (`run_map.sh:116-123`). A port that pins effort will change the fork's results. Byte-comparison fails on intentional deltas, so the deltas need a written contract first.
3. **Runner correctness gaps** (K1, all on disk):
   - Per-call result files are written non-atomically (`record_run.py:182`). A truncated file makes prep see zero pending, and `run_map.sh:255` swallows the error with `|| true`, so the packet is silently skipped.
   - Packet ids are reused after a rebuild (`prep.py:743`), and completion is keyed on the id alone (`prep.py:896-901`), so a stale finding can be served as current.
   - Budget is read once per wave (`run_map.sh:292`). A wave of 8 calls at $0.30 starting from $14.50 ends near $16.90 against a $15 cap. Each attempt can also use the full cap again on retry (`run_map.sh:160`).
   - A timeout kills claude after it has billed, and the missing cost is stored as 0 (`record_run.py:169`, `prep.py:1154`). The budget gate then passes.
   - The map lock trusts a bare pid (`run_map.sh:64-72`). dr.py's `StageLock` with flock and a start-time check (`dr.py:301-313`, `275-282`) is the correct pattern and is not used by the runner.
   - The schema validator implements only some keywords and ignores the rest (`record_run.py:31-62`). A schema with `maxItems` or `const` is silently not enforced.
   - `DR_CLAUDE_BIN` left exported in a shell makes production runs call the stub, which reports cost 0 and status ok (`dr.py:199`, `run_map.sh:47`).
4. **Cache key is incomplete.** It must not be built as drafted. The existing skip is per packet id (`prep.py:896-901`), and no input-hash cache exists. The key must cover the resolved model id, CLI version, effort, tools, permission mode, system prompt, settings file, schema, budget, packet and input shas. The alias `haiku` (`~/.claude/settings.json:2`, `dr.py:568`) can resolve to a new model later.
5. **Determinism and calibration statistics are too small to decide anything** (section 9).
6. **Effort and model are inherited, not pinned,** in the fork. `CLAUDE_CODE_EFFORT_LEVEL` silently overrides effort (C15), and records do not store effort, model or CLI version (K1-08).

## 5. Target architecture

### 5.1 Principle
One runner, invariants enforced in code and not in profiles, and measured parity before any migration.

### 5.2 Layers
- **L0 library**, `~/.claude/swarm/` (decision D1, global by default):
  - `bin/swarm_run.py`: the only code that calls `claude -p`. Reads a role file, builds argv, pins `--effort` and unsets `CLAUDE_CODE_EFFORT_LEVEL` for the call, writes results atomically (the `dr.py:113-118` `jwrite` pattern), takes the `StageLock` from `dr.py:301-313`, validates against the schema and fails closed on unknown keywords, treats a missing cost as null (never 0), reserves budget per attempt under a lock, and records resolved model, CLI version, effort and settings sha per call.
  - `bin/swarm_packet.py`: deterministic packets, built on the existing `subagent_prompt.py` rules where they apply.
  - `bin/swarm_merge.py`: the existing `merge.py` logic, with the quote check from `bundle/engine/verify/quote_check.py` (one canonical copy; the fork copy is deleted after M3).
  - `bin/swarm_ledger.py`: spend from CLI-reported cost only. List-price estimates reuse `agent_usage.py` pricing and are always labelled as estimates.
  - `roles/*.toml`, `prompts/`, `schemas/`, `tests/`: described in 5.4 and 5.5.
- **L1 recipes**: each is `SKILL.md` plus `recipe.toml` referencing roles by name. No recipe calls `claude` directly.
- **L2 project profile**, `<repo>/.claude/swarm/profile.toml`: packet roots, protected files, log routes, Ghidra DB path, caps and gates. It holds no safety invariants.

### 5.3 Built-in invariants (L0, not configurable)
- **Deny, always:** `git add`, `commit`, `push`, `reset`; `kpm_hotswap`; any install with `--apply`; remote posts; `--dangerously-skip-permissions` for worker calls.
- **Credential and secret paths** refused in packets and unreadable to workers.
- **Sandbox for every worker call, map and fix alike:** bwrap with `--tmpfs $HOME`, ro-binds for the needed toolchain, rw only for the task copy, `~/.claude` read-only. Today only fix workers have bwrap, and it is incomplete (section 2).
- **Gate test entrypoints pinned** to base-commit hashes; the gate refuses if `scripts/` changed.
- **Write scope** enforced by `agent_contract.py snapshot --out` and `check --scope` (`scripts/agent_contract.py:279-286`), which already exist. Only the repo-level recipes use this, since the bundle never references it.
- **Protected files** hashed before and after each stage; any change aborts (existing behaviour in dr.py, moved to L0).
- **CLI lockdown flags:** `--restricted` (removes Bash, PowerShell, REPL and WebFetch unless `--tools` names them, and ignores user, project and local settings files). Because `--restricted` ignores settings files, deny rules must move into flags. Add `--exclude-dynamic-system-prompt-sections` for cache reuse (benefit UNSURE; verify with `cache_read_input_tokens` from the existing run JSON, `record_run.py:65`).

### 5.4 Role catalog (initial, five roles, not eight)
| Role | Model | Effort | Tools | Output |
|---|---|---|---|---|
| packet-mapper | haiku | medium | Read, Grep, Glob | findings for one packet |
| claim-checker | haiku | low | Read, Grep, Glob | verdict plus quote |
| style-judge | haiku | high | Read, Grep, Glob | adjudication: skeptic, advocate, evidence |
| comparator | haiku | medium | none | agree / split / escalate |
| escalator | sonnet | high | Read, Grep, Glob | verdict with quotes, residue only |

Add fixer, reviewer and lens-critic only when their recipe is built. Effort is pinned explicitly per role (decision D4), and the parity probe in M1 shows the effect first.

### 5.5 Prompt and schema assets
- **Reuse, do not re-author:** `bundle/templates/adjudicate_style_{skeptic,advocate,evidence}.md`, `compare_prompt.md`, `compare_schema.json`, `escalate_prompt.md`, `lens_{contract,logic,security,state}.md`.
- **New:** `style_adversarial` (no template exists), the mod-ideate lenses (feasibility, prior art), and the bug-investigate hypothesis roles.
- **Rule for every prompt:** the verdict field is a prompt-level hint, not a structural guarantee. The validator checks no key order (`record_run.py:31-62`). Enforce order in the schema or the parser instead.
- **Stub double:** extend `bundle/stub/claude` to the new argv (`--restricted`, `--append-system-prompt`). Do not build a second one.

### 5.6 Topology and layer policy
- Layer 1: k=3 styles on the same unit (skeptic, advocate, evidence), Haiku.
- Layer 2: one comparator on the three judgments, Haiku. Unanimous, non-escalated items resolve.
- Layer 3: OFF by default. The measurement that would justify it does not exist yet (section 9). Bundle config `compare_judges=2` (`engine/dr_profile.py:44`) applies only when the ladder is enabled.
- Residue: Sonnet escalator on small batches, read-only.
- Log per-unit style votes so overlap between styles can be measured (K3-07).
- Human gates: first packet (audit), findings skim, fix go, recipe pick, and any install (never inside a swarm).

## 6. Cache and cost

### 6.1 Cache key (content-addressed, all fields required for a hit)
Role id, role file sha256, resolved model id, effort, CLI version, settings file sha256, tool list, permission mode, system prompt sha256, schema sha256, budget, packet sha256, and input file shas. Open: whether the stream-json result exposes the resolved model id (not verifiable on disk). If it does not, the cache must refuse to key on the alias.

### 6.2 Sampled re-runs bypass the cache
A determinism re-run that hits the cache reports 100% agreement by construction (K1-07). Sampled runs must set an explicit no-cache flag and record both outputs.

### 6.3 Cost rules
- Spend comes only from CLI-reported cost. A missing cost is an error, shown as a count, never 0 (`merge.py:788-792` already counts `calls_with_cli_cost`).
- Budget reservation per attempt, under a lock, with the approved ceiling read at runtime (`dr.py:733` sets `DR_SPEND_CEILING_USD`, but no engine script reads it).
- ABORT flag checked between and during waves (`run_fix.sh:291`).
- Pre-wave worst-case gate (`run_fix.sh:276-277`).
- Open: whether claude -p already reuses a cached shared prefix. Check it from existing run JSON at $0 before claiming any saving.

## 7. Playtest bundle (bug-investigate input)
- Inputs: symptom text, time window, save and mod names.
- Outputs: bounded JSON evidence with stable ids (log name, line range, excerpt sha).
- Route only through `scripts/kotor2_logs.py`, which already routes about 18 log types, and `crash_triage.py` for crashes. The rule in CLAUDE.md:7 forbids cat, tail and grep on mod logs; it does not name sed or rg, so the library must refuse them for workers as well.
- Context pack: KPM install hash, mod versions, recent commits touching the suspect mod. Passed only AFTER a suspect-blind pass (K6-F7).
- Scrubbed before render: absolute home paths, Steam IDs, tokens (new scrubber shared with packets, with a planted-ID self-test).

## 8. Recipes

### 8.1 deep-haiku-review (existing)
Map, reduce, verify, ladder (k styles, comparator, escalate), refute audit, human skim. Migrated onto swarm_run only in M3, behind a flag, with `dr.py` kept as the state machine.

### 8.2 haiku-fix (existing)
Task, sandbox, checks, review, judge ladder, integrate, human gate. Migrated in M3 after section 2 is fixed. Patches stay in isolated copies; applying to the real tree is a separate per-batch human approval.

### 8.3 bug-investigate (new)
1. Symptom intake: what, when, which save, which mod.
2. Playtest bundle (section 7).
3. Crash fast path: `crash_triage.py`, when the symptom is a crash or hang.
4. **Blind pass:** hypotheses from the symptom and bundle only, no context pack and no recent commits.
5. **Null hypothesis:** a mandatory role judges "not a bug, config, or engine as designed" on the same evidence.
6. Hypothesis swarm: timeline, per-suspect source (ranges via `rg`, never whole files), mod interaction (`compat_graph.py`, `check_hook_spans.py`), regression (`git log -S`), engine asm via the Ghidra wrapper only.
7. Verifier per hypothesis: quotes checked by the quote tool, plus a refutation attempt, with one non-Haiku verifier (Sonnet) on each surviving hypothesis.
8. Comparator ranking. Visual and UX items get a `needs human repro` verdict; screenshots are an intake channel.
9. Reproduce plan: forced-state test (`kotor2-forced-state-testing`) or a probe.
10. Human gate, then fix through haiku-fix in an isolated copy.

Feasibility from the 43 feedback items (K6, counted from headings and bullets): crash 7, logic 18, visual 11, UX 7. About 58% (crash plus logic) can be checked from code or existing logs in principle. The remaining 42% (visual and UX) need a screenshot or a live repro.

### 8.4 mod-ideate (new)
1. Intake: idea, goal, constraints.
2. Context pack from `patch_manager_mods/16_candidate_register.md` (its "Hooks needed" column routes ideas to hook or data branches), `engine_research_findings/` (CONTRACT.md, briefs), `query_ghidra.py` (the real DB is 9,838,592 bytes), and `search_docs.py` after a re-index gate (`.doc_search_cache.db` predates the Oct 9 edits).
3. Lens swarm: feasibility, prior art, hook and byte conflicts (`check_hook_spans.py`, `compat_graph.py` as deterministic input, Haiku only explains), scope, testability, risk.
4. Comparator, then a design brief.
5. Human pick.
6. `scaffold_mod.py` with a new `kind` selector (`hook`, `data`, `ui`). Today it has no kind parameter (`scaffold_mod.py:37`) and always emits a DLL detour scaffold (`:91`, `:103-108`), so a pure data or 2DA idea gets a wrong-shaped plan. Also add the collision check.

### 8.5 swarm-review (generic)
Claim checks on disk, then critic lenses over a plan, then a comparator, then a lead spot-check of the top claims. This is the pattern used for this document.

## 9. Measurement and calibration (computed)
Formulas: Wilson score interval for proportions; sample size n = (z·sqrt(p(1-p))/E)^2 with z=1.96; paired discordance size n = (z_a sqrt(φ) + z_b sqrt(φ-d²))² / d² with α=0.05 two-sided, power 0.8, d=0.10.

| Question | Current evidence | Result |
|---|---|---|
| Is a 5% determinism sample meaningful? | 5% of 40 units = 2 units. 2/2 agreement gives exact interval [0.158, 1.0] | No. Need **≥139 units** for ±5 points at 90% agreement, or drop the gate |
| RE decoy pilot | 13/40 accepted: Wilson [0.201, 0.480] | Too wide to set a threshold. Need **~340 decoys** for ±5 points at 33% |
| "Near zero" false accepts | 0/40 accepted: upper bound 0.088 | Need **≥74 decoys with 0 accepts** to bound false accept at 5% |
| Is 2 layers better than 3? | Only a fix-pipeline ladder vs Sonnet-first, n=12, 8/12 both | Unmeasured. Need **~155 paired labelled units** for d=0.10 (φ=0.20) |
| Agreement reporting | Point estimates only | Report Wilson intervals per stage, never a point estimate alone |

Also: the ladder oracle and reviewer were both Haiku, so "accepted" labels share model blind spots. Gold-label the 12 parents with Sonnet or Opus before any voting claim (K3-06). The stub is deterministic by construction, so $0 selftests cannot measure run-to-run variance; that needs one small live batch under a human go.

Reliability metric already in the repo: 4 invalid quotes in about 45 (`ladder_experiment.md:34`). Track it per stage.

Flag precision: the RE pilot had 82 flags on real rows with 0 database errors. Track flag precision per stage, and treat flags as unconfirmed.

## 10. Test strategy
- **$0:** stub-double selftests (mechanics), golden diffs, schema tests, cache-hit tests, budget-refusal tests, lock and concurrency tests, invariant-deny tests (git add, install and credential path must each be refused by the library with an empty profile), sandbox HOME-probe canary, planted-ID scrub test. Register new suites in `run_tests.py` with filters (`run_tests.py` takes over 60 s when run in full).
- **Parity:** bundle and fork run_map.sh against `fixture/mock_engine` on the same packet, diffing argv previews and parsed results (M1).
- **Calibration (paid, human go):** decoys in every stage plus planted bugs with known answers. Gold-labelled set before any ladder claim.

## 11. Migration plan (each step $0 unless marked; no live fix run during any port step)
- **M0, now:** D0 decision. Record the live fix run (pid 3834907) and let it finish or stop it. Freeze both trees as they are.
- **M1, smallest slice ($0):** one new file, `swarm_argv_probe.py`. Build argv for one role; diff it against the argv previews from the bundle and fork `run_map.sh` under the stub. Result: a parity table. If the only difference is `--effort`, the drift is cosmetic and the fork should be deleted instead of building a library.
- **M2, safety ($0 tests, human go to ship):** section 2 fixes. Tmpfs `$HOME` in map and fix sandboxes; canary HOME probe under a fake HOME; `~/.claude` read-only; gate entrypoints pinned; `allowed_files` limited to primary plus human-approved paths; built-in deny list; scrubber with planted-ID test.
- **M3, runner ($0 selftests):** `swarm_run.py` with the section 4.3 fixes. Port map first behind a flag. Normalised equality on the fixture, with the intentional deltas written down first (the fork's `vcommon.py` has 471 differing lines; `run_map.sh` has 299).
- **M4, measurement (paid, human go):** gold-label the 12 ladder parents; build the decoy set (~340 for ±5 points); run the paired 2-vs-3 layer test (~155 units).
- **M5, recipes:** bug-investigate first, then mod-ideate. Pilot each with a $1 cap on a known past case. Ground truth: `00_ui_queue_hotbar_feedback_2026-10-06.md` and `33_ui_queue_hotbar_fixes.md` record the action-hotbar "character not identified" bug. **But** the two docs give different root causes (the 00 snippet no longer matches `HotbarLogic.h:74-75,89`). Reconcile to one canonical, commit-anchored record first, and exclude both docs from the blind pass's packet.
- **Retire the forks** once the diffs are empty.

Time: M1 is a single short session. M2 and M3 are one or two sessions each. I have not estimated M4 or M5 beyond that, because they depend on the human go and on the M4 results.

## 12. Risk register (from the critics)
| # | Risk | Sev | Mitigation |
|---|---|---|---|
| R1 | Fixer reads `~/.aws` and similar secrets during a run | H | M2 sandbox, D0 |
| R2 | Injected JSON widens fixer write scope | H | allowed_files limit (M2) |
| R3 | Gate runs repo code an injected finding edited | H | pinned entrypoints (M2) |
| R4 | Silent skip after a truncated result file (exit 0) | H | atomic writes, fail loud (M3) |
| R5 | Budget overshoot within a wave | H | per-attempt reservation (M3) |
| R6 | Stale findings served after packet rebuild | H | key completion on packet and config sha (M3) |
| R7 | Port changes results because the fork passes no `--effort` | H | parity probe before any effort pin (M1) |
| R8 | A port orphans the live fixer or corrupts its task dir | H | no cutover while `ps` shows a fix task |
| R9 | Port overwrites `active.json` or `p1/state.json` (paused run) | H | read-only schema, golden resume test |
| R10 | Decoy thresholds set from n=40 | H | section 9 sample sizes |
| R11 | Determinism gate passes vacuously | H | no-cache sampled runs (6.2) |
| R12 | Library is as large as the plumbing it replaces; net gain near zero until forks retire | M | M1 decision before M3 |
| R13 | Mod-ideate gives every idea a hooked DLL scaffold | M | `kind` selector (8.4) |
| R14 | Bug-investigate confirms its first hypothesis | M | blind pass, null hypothesis (8.3) |
| R15 | All roles Haiku on one packet, so shared errors go undetected | M | vary packet slices, one Sonnet verifier per hypothesis |
| R16 | Prior-art index stale | M | re-index gate (8.4) |
| R17 | Double counting: Mod 1 and Mod 2 items in both audit and playtest docs | M | dedupe by file:line before counting |
| R18 | Canary passes while home reads are unrestricted (p1 canary: pass) | M | HOME probe (M2) |

## 13. Edge cases the critics raised that the design must still handle
- Concurrent runs of one stage; stale lock after a crash or `/clear` (pid reuse; use `StageLock` start-time check).
- Stream-json result missing on timeout: cost unknown, not zero.
- `DR_CLAUDE_BIN` exported in a shell: production silently uses the stub.
- Schema keyword not implemented by the validator: must fail closed.
- A cache hit after a silent change to CLI version, effort or model alias.
- Sampled determinism re-runs served from cache.
- Shared P_TREE for all calls (`prep.py:1127-1128`): concurrent or stale runs share a tree. Per-call copies are needed for isolation claims.
- Worker-cited evidence files widening scope (R2).
- `fix_merge.py:206` joins an absolute path without `safe_rel` (escape read, low).

## 14. Opportunities (reuse, verified to exist)
- **Stub double:** `bundle/stub/claude`, extend rather than rebuild.
- **Prompt templates:** `bundle/templates/*` (section 5.5).
- **Quote check:** one canonical copy after the fork is retired.
- **Ledger pricing:** `scripts/agent_usage.py` plus `agent_usage_rates.json`, for labelled estimates only.
- **Write-scope and snapshot:** `scripts/agent_contract.py`.
- **Selftests:** `scripts/run_tests.py` filters.
- **Conflict input:** `scripts/check_hook_spans.py`, `scripts/compat_graph.py`. Makes the mod-ideate conflict lens deterministic.
- **Crash and log routing:** `scripts/crash_triage.py`, `scripts/kotor2_logs.py`.
- **Lock pattern:** `dr.py:301-313` (StageLock) and `275-282` (start-time check). Also the atomic `jwrite`, `dr.py:113-118`.
- **CLI features:** `--agents` (JSON agent definitions; field names UNSURE), `--worktree` (could replace the hand-built copy in haiku-fix; print-mode behaviour UNSURE), `--fork-session`. Do NOT use `--fallback-model`: it silently changes the model and breaks pinned-model calibration.
- **Pilot p1 resume data:** `~/.cache/deep_haiku_review/kotor2-pilot/p1/state.json`. Paused after verify-select, first-packet approved, no findings-skim yet. Treat as a frozen schema.

Top three to adopt first (K4): extend the stub double and prompt templates (removes most of M1 at $0); wire `agent_contract` snapshot and check into the runner; add `--restricted` and `--exclude-dynamic-system-prompt-sections` after a $0 stub test.

## 15. Decisions for you
- **D0 (urgent):** pause new live fix and map runs until the sandboxes hide `$HOME`. Recommend yes.
- **D1 Library home:** `~/.claude/swarm/` (recommended) vs `.claude/swarm/` in the repo. Decide after M1, since the library may not be built.
- **D2 Port order:** safety (M2) first, then deep-review map (M3). Fix pipeline last, because it has a live run.
- **D3 Ladder depth:** 2 Haiku layers plus Sonnet residue by default. The 3rd layer stays off until the ~155-unit paired test (M4).
- **D4 Effort per role:** explicit per role (recommended) vs inherit settings. Decide after the M1 parity table, since pinning effort changes the fork's results.
- **D5 New-skill order and shape:** bug-investigate first (the evidence exists: 43 feedback items, and its tools exist), then mod-ideate (needs the `kind` selector first). Separate skills rather than one with modes.

## 16. Verification ledger (summary)
25 claims checked: 12 CONFIRMED, 12 PARTIAL, 1 REFUTED (C02, engine size). Verifier and critic outputs are Haiku 5.5 work; numbers that drive a decision were re-checked by the lead (section 2.1, budget arithmetic, and the sample-size formulas in section 9).

Still open (could not be checked read-only): whether the stream-json result exposes the resolved model id; whether claude -p reuses cached prefixes; the `--agents` field names; `--worktree` in print mode; and which profile the production deep-review run loads.

## 17. M1 result (2026-10-10, $0)
Probe: `scripts/swarm_argv_probe.py` (untracked). It runs the real `effort_args` and `run_one` functions copied from each `run_map.sh`, with a recording `claude` shim in front of `bundle/stub/claude`. Prep and record calls are faked. Nothing reads or writes a live run dir.

| Case | Result |
|------|--------|
| bundle, default profile vs fork | argv IDENTICAL (18 tokens: flags, values, order, schema content hash) |
| bundle, `EFFORT_MAP=xhigh` | adds `--effort xhigh`; this is the only argv difference (positive control PASS) |
| worker_settings.json, worker_system_prompt.md | NOT COMPARED. The bundle writes both at build time from the profile (`prep.py:828-829`, `:980`). The fork file is its own output. |

Corrections found while building the probe:
- The fork's `--dry-run` `cmd:` line (`.claude/review/run_map.sh:116`) is a hard-coded string. It cannot show drift. The bundle's preview (`bundle/engine/run_map.sh:264`) is also a literal flag list, with only the effort and paths computed. It is not a measurement either. Earlier "argv previews" comparisons should not be used.
- `--restricted` ignores user settings files (`claude --help`). So the `effortLevel` in `~/.claude/settings.json` does not reach map calls in either engine. Neither worker settings file sets an effort. Both engines therefore run at the CLI default effort unless a profile passes `--effort`. That default value is UNSURE, and it cannot be read at $0.
- The fork has its own stub (`.claude/review/tools/stub_claude/claude`). It differs from the bundle stub, so the fork's self-tests do not exercise the bundle's stub. Not part of the argv probe.

Verdict, scoped to the map call only: argv parity holds at the default profile. The fork's run_map argv is equivalent. "Fork deleted" is NOT yet supported. It still needs the parsed-result and prompt-render comparison, the settings and system-prompt content check (materialise one bundle run dir and compare), and the rest of the 33-entry drift (verify/ and tools/). D4 is now a real choice: pinning effort changes map results in both engines, because today both use the unknown CLI default.

## 18. M4 review follow-ups: fix packages R1..R6 (2026-10-13, PLANNED, nothing implemented)
Source: `.claude/reviews/bug_investigate_m4_session_review.md` (8 Haiku recon lenses; raw `.claude/reviews/bug_investigate_m4_review/u1.md`..`u8.md`). Implementation plan: `.claude/plans/swarm_m4_remediation.md` (copied from the approved plan file at its step 0). Handoff: `.claude/handoffs/swarm_m2_safety.handoff.md` section "M4 session review". Track status in this table only (edit the Status cell; never delete a row).
Constraints: bundle `5a8be20` and LIB `651047e` stay untouched unless the row says "LIB, needs go"; roles, prompts and schemas stay frozen (`$SP/frozen.sha256` style check) until R5.2 is done; paid calls only at gate G-R1 with an explicit in-chat yes and ceiling.

| Id | Package and item | Evidence (file:line) | Paid | Status |
|---|---|---|---|---|
| R1.1 | verify preflight: refuse (real engine) when units x cap_call > cap_total, print the cap needed | `bug_investigate.py:693-699`, `swarm_run.py:296-297` | $0 | DONE 0bcbfad |
| R1.2 | pending verify units make decision `incomplete`, nonzero exit, listed in summary as "budget deferred" (not "no record") | `bug_investigate.py:499-515`, `:725`, `swarm_run.py:786-787` | $0 | DONE 0bcbfad |
| R1.3 | verify order: ranked/round-robin by role so no role starves (skeptic got 0 of 25) | `bug_investigate.py:693` | $0 | DROPPED: new unit ids would orphan 10 recorded verify runs; preflight (R1.1) makes order irrelevant, per-style coverage reported (R4.4) |
| R1.4 | same preflight inside the runner (`check_spend_flags`) | `swarm_run.py:282-297` | $0 | TODO, LIB, needs go |
| R2.1 | run `probe` before `write_units`; write into a fresh/archived `out/` | `bug_investigate.py:626-627`, `:331` | $0 | DROPPED: runner status recomputes unit_sha256 from the current unit file (swarm_run.py:814-850), stale findings already incomplete |
| R2.2 | stamp `bundle_sha256` + sha256(claim, mechanism, falsifier) into each verify unit; ignore mismatched verdicts | `bug_investigate.py:458`, `:467-471` | $0 | DONE 862714e |
| R2.3 | `bundle` refuses when stage `out/` exists; refuses empty evidence or a missing launch | `bug_investigate.py:590-615`, `:611` | $0 | DONE 862714e |
| R2.4 | per-run lock (`fcntl` on `RUN/.lock`) for mutating commands; `new` uses `mkdir` without `exist_ok` plus random suffix | `bug_investigate.py:562-564`, `:533` | $0 | DONE 862714e |
| R2.5 | resume sends only incomplete units (check what `swarm_run map` already skips) | `bug_investigate.py:648-650`, `swarm_run.py:524-526` | $0 | DROPPED: resume already sends only incomplete units (swarm_run.py:524-526) |
| R3.1 | null unit needs non-empty verdicts (`all([])` is True today) | `bug_investigate.py:495`, `:503` | $0 | DONE a9d2d3c (pin test) |
| R3.2 | malformed complete units (hypotheses not a list, missing id) become incomplete with a reason | `bug_investigate.py:428-439`, `:490-492` | $0 | DONE a9d2d3c |
| R3.3 | forbidden-term check: NFKC, strip zero-width, collapse whitespace and `_`/`-`, match on joined excerpt text, whole-word for alnum edges; apply to verify units and to the symptom at `new` | `bug_investigate.py:655-660`, `:682-699` | $0 | DONE a9d2d3c (blind/null + symptom at new; separators deleted not collapsed so partycount matches party_count) |
| R3.4 | rank on distinct `text_sha256`; dedupe `valid_evidence_ids` | `bug_investigate.py:478`, `evidence_quote_check.py:156-157` | $0 | DONE b7ecd02 + a9d2d3c |
| R3.5 | validate `L<n>\|` markers against the excerpt line range; `MIN_CHARS` on letters/digits after stripping tag/timestamp; hint `ev:<sha8>` form on truncated ids; catch lone surrogate in CLI | `evidence_quote_check.py:57`, `:140`, `:123`, `:246` | $0 | DONE b7ecd02 |
| R4.1 | scorer: negation window (not/never/no/n't/without within ~6 words), hedge flag | `bug_investigate_score.py:117-119` | $0 | DONE a589b90 |
| R4.2 | scorer: strip the symptom sentence and quoted evidence before matching (rubric `companion`/`follower` echo) | `queue_reinject_bg09.json:44-56` | $0 | DONE a589b90 |
| R4.3 | rank-1: None top counts as miss; assert top equals report `ranked[0]`; use producer rank key after R3.4; report rank-1 under reversed unit order | `bug_investigate_score.py:154-157`, `:327-330` | $0 | DONE d03aab2 |
| R4.4 | per-run proportions with run-cluster CI beside pooled Wilson; all-label rate (incl. quote-invalid); per-style breakdown | `bug_investigate_score.py:319-337` | $0 | DONE d03aab2 |
| R4.5 | one-case assertion in `aggregate`, reject empty regex, `cost_complete` needs expected unit count, stamp rubric sha256 in `score.json` | `bug_investigate_score.py:104-105`, `:164-194`, `:408-411` | $0 | DONE a589b90 + d03aab2 |
| R5.1 | correct the M4 note: 2 of 28 verified, "rubric label" wording, null saturation, in-sample agreement | `bug_investigate_m4_measurement.md:27,38,39,43` | $0 | DONE (see commit docs(bug-investigate) after 6808f6a) |
| R5.2 | freeze both rubrics; hand-label >= 20 fresh hypotheses by another agent or the user; report kappa; no rubric edit on that set | `hotbar_charkey.json:28-36`, `queue_reinject_bg09.json:42-63` | $0 | TODO, needs user input |
| R5.3 | design 1 to 2 not-a-bug control bundles for the null unit (design only; building needs a go) | measurement note :43 | $0 | DONE design only (measurement note) |
| R5.4 | SKILL.md: cap sizing table, wait via Monitor never sleep/self-matching pgrep, Edit not heredoc, required verify flags, stub needs `--stub-script` | `SKILL.md:62-65` | $0 | DONE |
| R5.5 | gated verify resume with a cap covering all pending units (about $1.1 for 51 units: two stages or a sample); re-score config B | `swarm_run.py:47` ceiling 1.00 | up to $1.00, gate G-R1 | TODO, needs explicit yes |
| R6.1 | stub default outputs per role; `rehearse` must end all-ok before a paid gate | session events [122]-[134] | $0 | DONE 6808f6a (refuse stub without --stub-script; stub default outputs not built) |
| R6.2 | `measure --runs N --ceiling USD` driver (frozen check, new/bundle/blind/null/score, ceiling line) replacing hand-written `b1.sh` | review U1 | $0 | DEFERRED: measure driver out of scope for this plan |
| R6.3 | `score --sample N --seed S` and `agree --labels` (held-out sampling), run id stored from `report.json`, prefix lookup | review U1, U3 | $0 | DONE 1ea4d87 (subcommands sample and agree) |
| R6.4 | worker-brief snippet in SKILL.md: red = assertion failure, `export TMPDIR=<scratch>` first, no edits after green, `sed -n` windows | review U2 | $0 | DONE |

Order: R3 and R4 (pure code, $0) can run in parallel with R1; R2 after R3.1/R3.3; R5.1/R5.4 any time; R5.2 needs a human or second agent; R5.5 last (after R1, R2, R4). Done = tests for every row red-then-green, mutation per package, `run_tests.py bug_investigate evidence_quote playtest swarm-lib swarm-bundle swarm-fix` PASS, `check_skills.py` PASS, and this table updated with the commit sha per row.
