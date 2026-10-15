# Session Handoff & Plan State: patch_manager_mods/ghidra_knowledge/DECOMP_PLAN.md

## 0. Next-Phase Direction
Execute D4: `named_decomp.py` named decompile; G4 CSWCCreature

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/ghidra_knowledge/DECOMP_PLAN.md`
- **Branch:** `KOTOR-II` (85ad108 DECOMP D3: cost line)
- **Generated:** 2026-10-15 19:50:28 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
85ad108 DECOMP D3: cost line
 patch_manager_mods/ghidra_knowledge/DECOMP_PLAN.md | 1 +
 1 file changed, 1 insertion(+)
```
- **Dirty Files (150):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/42_holstered_weapons_master_plan.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M patch_manager_mods/34_npc_diversity_dispatcher.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ... +140 more (run `git status --short`)

## 2. Immediate Objective (D4)
**`named_decomp.py` named decompile; G4 CSWCCreature**

Status: TODO (GATED ON D1)
Task: `named_decomp.py` named decompile; G4 CSWCCreature

## 2b. Upcoming Batches in Pipeline
- **D5** : `fn_doc.py` + `persist_purposes.py`; CSWCCreature page + KDB purposes
- **D6** : R2 rebuilt (`--arm facts
- **D7** : rea-recon agent v2 + `test_agent_copy.py`; live check on 0x85d260/2d0/320/3d0

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/ghidra_knowledge/DECOMP_PLAN.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate D4` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
- **Commits this session:** 6650190 (D0), 5f4175a (D2), 5682e3b (D1), f9c8f2c (plan/cost), 707cf82 (D3 WIP, unvalidated).
- **REA facts (D1 probe, `.claude/research/rea_decomp/D1_probe.md`):** first analysis call per session ~315 s, then ~0.01 s/call;
  results are envelopes (read `normalized_result`); `resolve_native_call_targets` returns `unresolved` for vtable calls
  (use static slot + class_methods); `set_addresses_names`/`set_address_name` rejected (capability_unavailable);
  `annotate_native_function {procedure,name}` works and shows in pseudo-C; `analyze_function` gives body ranges.
  JVM RSS ~0.98 GB. Another session's `npm exec rea-agents` (pid 1511977) has been live all day: guard counts it (1 of 2).
- **Dead ends:** `agent_contract.py check --scope a b c reply` swallows the reply (nargs+): put the reply path FIRST;
  it also reported NOT_DIRTY for clearly modified files (not investigated; verified with `git diff` instead).
  Byte-identity checks of `batch.py cards` from a `git archive` copy differ unless BOTH old and new run from scratch
  copies with `BD_KDB` set (relative KDB path) — verified IDENTICAL that way.
  Haiku relabelled the live exe md5 `62411a3a` as sha256; it is md5 (fixed).
- **T3.1 deviations to validate (Sonnet V3):** a lost REA session aborts with exit 1 and writes no row for the victim
  address; step-4 marker row when steps 2-3 add nothing; added `--lock-dir`; callsite evidence uses the dump's `how`
  text (dump.asm lines carry no addresses) — check the L2/anchor format and whether `0x<site>: <insn>` can be rebuilt
  from `atlas_common.read_asm`; `--no-live` resume cannot skip addresses without callsites.
- **Verify:** `cd patch_manager_mods/ghidra_knowledge/tools && timeout 600 python3 -m unittest discover -s tests` (214 OK);
  `cd patch_manager_mods/steam_db_batches/tools && python3 -m unittest discover -s tests` (326 OK);
  `python3 -m unittest scripts.test_query_ghidra` (13 OK); `python3 patch_manager_mods/ghidra_knowledge/tools/rea_ledger.py lint` (baseline 53).
- **D3 closed (65d4524, cost 85ad108):** V3 fixed callsite evidence (exact `0x<site>: <insn>` from `atlas_common.read_asm`,
  falls back to `how`), `--no-live` writes marker `no live run (static only)` (never counts as done for a live run);
  `--residual` already met spec (tests added). G3: `rea_findings/20261015_B13a-s1.tsv` 98 rows, exit 0, lint 53, sha OK.
  Slot candidates are often unrelated CSWGui* classes (receiver class unknown) -> keep `probable`; never treat as names.
- **D4 next (Batch B):** pilot class is **CSWSCreatureStats** (124 shipped rows), NOT CSWCCreature (0 rows); record in
  DECOMP_PLAN Decisions at the start of Batch B (phase-table text still says CSWCCreature).
