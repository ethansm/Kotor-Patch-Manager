# DRAFT: Haiku swarm library architecture (for critique; not yet approved)

## 1. Goals
G1. One shared library for every Haiku swarm skill: role definitions, prompt templates, JSON schemas, one runner, one merge/verify toolset, one cost ledger.
G2. Four recipes on top of it: deep-haiku-review (audit, existing), haiku-fix (fix pipeline, existing), mod-ideate (new), bug-investigate (new). A fifth, swarm-review (verify a plan or design), is the generic pattern used in this session.
G3. Reduce nondeterminism: content-addressed inputs, pinned model and effort per role, schema-constrained output, measured agreement instead of assumed agreement.
G4. Cut token spend: deterministic packets, narrow roles, cache by input hash, CLI-reported cost only.
Non-goals: no change to KOTOR game code; no live install or hot-swap from any swarm; no fix applied to the real tree without a per-batch human approval.

## 2. Current state (claims to be verified; see claims.md)
- Global deep-review engine lives in ~/.claude/skills/deep-haiku-review/bundle (dr.py state machine, engine/ runners, verify/ ladder, templates/). Fix stages S9-S14 are not built.
- A second copy of the engine lives in .claude/review/ (verify/, tools/) and has drifted from the bundle.
- The `claude -p` call plus stream-json parsing is re-implemented in many scripts across both trees.
- sha256 helpers are re-implemented in several scripts.
- Fix pipeline (.claude/review/fix/) has its own runner, judge, ladder and bwrap sandbox.
- Phase C ladder result: ladder 8/12 accepted, Sonnet-first 8/12, ladder cost about 28% of Sonnet-first; rounds 2-3 added nothing.
- Haiku decoy pilot (RE): 33% decoy acceptance on a single pass.
- CLI: no temperature or seed flag; --effort, --json-schema, --max-budget-usd, --tools, --permission-mode, --settings, --strict-mcp-config, --disable-slash-commands, --no-session-persistence exist.
- Map runner sets: --tools "Read,Grep,Glob" --permission-mode dontAsk --max-budget-usd --json-schema --strict-mcp-config --disable-slash-commands --no-session-persistence --output-format stream-json --verbose.
- Effort defaults: ~/.claude/settings.json modelSettings haiku=xhigh, sonnet=high. CLAUDE_CODE_EFFORT_LEVEL overrides --effort (runner warns).

## 3. Architecture

### 3.1 Layers
L0 Library  ~/.claude/swarm/   (global, shared by all repos)
   bin/swarm_run.py       the ONLY code that calls `claude -p`. Builds argv from a role file, unsets or refuses CLAUDE_CODE_EFFORT_LEVEL, runs in a per-call scratch dir, parses stream-json (result line, total_cost_usd, usage), validates the output against the role schema, writes atomically (temp file, then rename), backs off on 429/529, caches by input hash, enforces a per-call and per-stage budget, returns an exit code.
   bin/swarm_packet.py    deterministic packetizer: sorted inputs, stable ids, token-size caps, secret-path refusal, redaction of absolute home paths, no timestamps in prompts.
   bin/swarm_merge.py     dedupe and cluster findings, coverage gate (every unit has an output), quote check (every cited quote must appear verbatim at the cited path:line), comparator input shuffling.
   bin/swarm_ledger.py    per-run spend (CLI-reported only), call counts, cache hits, wall time, and a determinism check (re-run a sample; report agreement).
   roles/*.toml           one file per role: name, model id, effort, allowed tools, permission mode, budget per call, timeout, schema path, system prompt template, user prompt template, cache flag.
   prompts/               shared templates: style_skeptic, style_advocate, style_evidence, style_adversarial, lens_*, compare, escalate, plus a PROMPT_RULES.md (verdict last, negative few-shots, decoys required).
   schemas/               finding, verdict, adjudication, comparison, claim-check, idea-brief, hypothesis.
   tests/                 stub claude double + golden fixtures; $0 selftests.

L1 Recipes  (each skill is SKILL.md + recipe.toml that references roles by name)
   deep-haiku-review   map -> reduce -> verify -> ladder (k styles -> comparator -> escalate) -> refute audit -> human skim.
   haiku-fix           fix task -> sandbox (bwrap) -> checks -> review -> judge ladder -> integrate -> human gate.
   mod-ideate          intake -> context pack (register rows, mod list, hook windows, Ghidra lookups) -> lens swarm (feasibility, prior art, conflicts, scope, testability, risk) -> comparator -> design brief -> human pick -> scaffold_mod.py.
   bug-investigate     symptom intake -> playtest bundle -> crash fast path (crash_triage) -> hypothesis swarm (timeline, per-suspect source, mod interaction, regression, engine asm) -> verifier per hypothesis (quotes plus a refutation attempt) -> comparator ranking -> reproduce plan (forced-state or probe) -> human gate -> fix via haiku-fix in an isolated copy.
   swarm-review        claim verification (fact checks on disk) + critique lenses over a plan -> comparator -> spot-check by lead.

L2 Project profile  <repo>/.claude/swarm/profile.toml
   packet roots, protected files, forbidden commands, playtest log routes (kotor2_logs.py), Ghidra DB path, mod dirs, budget caps, stage gates.

### 3.2 Role catalog (initial)
R1 packet-mapper      Haiku, effort medium, tools Read,Grep,Glob; output: findings for one packet.
R2 claim-checker      Haiku, effort low-medium; one claim per call or a small batch; output: verdict + quote.
R3 style-judge        Haiku, effort high; styles skeptic / advocate / evidence / adversarial; output: adjudication.
R4 lens-critic        Haiku, effort high; one lens per call; output: findings list (capped).
R5 comparator         Haiku, effort medium; sees anonymised, shuffled judgments; output: agree / split / needs-escalation.
R6 escalator          Sonnet, effort high; only the residue; output: verdict with quotes.
R7 fixer              Haiku in sandbox; Sonnet for design-level fixes only; output: patch plus checks.
R8 reviewer           Haiku; read-only on the patch; output: review verdict.

### 3.3 Topology and layer policy
- Layer 1: k=3 styles (skeptic, advocate, evidence) over the same unit, Haiku.
- Layer 2: comparator on the three judgments, Haiku. Unanimous and non-needs-human items resolve.
- Layer 3: optional, a second comparator with different wording; OFF by default (see decision D3).
- Residue: Sonnet escalator, small batches, read-only.
- Human gates only at: first packet (audit), findings skim, fix go, recipe pick (mod-ideate), install (never inside a swarm).

### 3.4 Determinism
- The CLI has no seed or temperature flag, so nondeterminism is handled structurally:
  (a) model id and effort pinned in the role file; (b) prompts built from sorted, stable-id inputs with no timestamps or absolute paths; (c) schema-constrained output with the verdict field last; (d) content-addressed cache keyed by role, model, effort, prompt sha256 and schema sha256; (e) voting (k styles plus comparator) where a single pass is unreliable; (f) determinism sample: re-run a random 5% of units each stage and report agreement; the stage fails below a calibrated threshold.
- Calibration: every recipe ships a calibration set with known answers (decoys plus planted bugs). The stub claude double makes the mechanics testable at $0; the real calibration costs money and needs a human go.

### 3.5 Cost controls
- Cost is taken only from the CLI result (total_cost_usd). List-price estimates are never reported as spend.
- Per-call cap: --max-budget-usd from the role file. Per-stage cap: a human approval (approve spend-<stage>) above a threshold.
- Cache: identical call inputs are never re-billed.
- Prompt caching of the shared system prompt and schema: an opportunity to verify (UNSURE: whether claude -p already caches shared prefixes).
- Packet sizing: units capped at a token budget; narrow roles (one deliverable, at most 3 files or 1,500 lines, short reports).

### 3.6 Isolation and security
- Lockdown per call: --tools limited to the role's list, --permission-mode dontAsk, --strict-mcp-config with an empty MCP set, --disable-slash-commands, --no-session-persistence, a --settings file with deny rules, cwd set to a per-call copy or scratch dir.
- Untrusted input: repo text, logs and feedback are data. Workers output schema only. The orchestrator never executes commands suggested by a worker.
- Secrets: packet builder hard-refuses credential and secret paths (global CLAUDE.md policy) and redacts known patterns.
- Writes: a worker writes only its own output file; agent_contract check enforces the write scope after each agent.
- Invariants enforced in code, not prose: no git add or commit, no install or hot-swap, no remote posts, protected files hashed before and after every stage.

### 3.7 Playtest bundle (bug-investigate input)
- Input: symptom text, time window, save or mod names.
- Output: bounded JSON evidence items with stable ids (log name, line range, sha of the excerpt).
- Route only through scripts/kotor2_logs.py (project CLAUDE.md forbids raw cat, tail or grep on mod logs). perflab_metrics-class files are summarised, never read whole.
- Context pack: active KPM install hash (kpm_status), mod versions, recent commits touching the suspect mod.

### 3.8 Migration plan (each step is gated; all $0 until a human go)
M0  Inventory: freeze the global bundle as the source of truth, record the fork diff report. No deletions.
M1  Build the library: swarm_run.py, roles, schemas, stub double, golden fixtures. Selftests at $0.
M2  Port deep-review map and verify onto swarm_run behind a flag; byte-compare normalised results on the fixture; then real quick run.
M3  Port the fix pipeline; then retire the .claude/review/ forks once diffs are empty.
M4  Build mod-ideate and bug-investigate. Pilot each with a $1 cap on a known past case (playtest feedback 2026-10-06 in patch_manager_mods/00_ui_queue_hotbar_feedback_2026-10-06.md, with the root cause already recorded in doc 33).
M5  swarm-review skill: generalise the claim-check plus critic pattern used in this session.

### 3.9 Test strategy
- $0: stub double selftests (mechanics), golden fixture diffs, schema-validation tests, cache-hit tests, budget-refusal tests, lock and concurrency tests.
- Calibration: decoys in every stage; the stage fails if decoy acceptance exceeds the threshold.
- Determinism: repeat-run agreement, reported per stage.

## 4. Decisions for the user (draft)
D1 Library home: ~/.claude/swarm (global, default) vs .claude/swarm (repo).
D2 Port order: deep-review first (existing pilot p1 is paused at a human gate) vs fix pipeline first.
D3 Ladder depth: 2 Haiku layers plus Sonnet residue by default (measured) vs 3 Haiku layers (earlier memory guidance).
D4 Effort per role: explicit per role (proposed) vs inherit settings (xhigh for Haiku).
D5 Whether mod-ideate and bug-investigate ship as separate skills or as modes of one "swarm-investigate" skill.

## 5. Known risks (draft; swarm critique should extend this)
- Haiku over-refutes real findings (K2, K4, K3a in the audit). Refuted items are not safe to drop.
- Decoy acceptance was 33% in the RE pilot; calibration is required per recipe.
- Two engines drift until forks are removed; a port can silently change results.
- In-flight runs: pilot p1 (paused) and fix runs must not be disturbed by a port.
- Over-spawning: many small agents cost overhead; the cap per wave must be explicit.
