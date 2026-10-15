# Session Handoff & Plan State: patch_manager_mods/ghidra_knowledge/DECOMP_PLAN.md

## 0. Next-Phase Direction
Execute D3: `rea_facts.py` live-only facts (indirect targets, body ranges, data xrefs) + `rea_questions --residual`; G3 on 46 R1 addrs

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/ghidra_knowledge/DECOMP_PLAN.md`
- **Branch:** `KOTOR-II` (5682e3b DECOMP D1: rea_session.py persistent REA MCP client, fake server, live probe)
- **Generated:** 2026-10-15 19:28:30 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
5682e3b DECOMP D1: rea_session.py persistent REA MCP client, fake server, live probe
 .claude/research/rea_decomp/D1_probe.md            |  49 ++
 patch_manager_mods/SUBAGENT_ROLES.md               |   3 +-
 .../ghidra_knowledge/RE_AGENT_GUIDE.md             |   6 +
 .../ghidra_knowledge/tools/rea_session.py          | 607 +++++++++++++++++++++
 .../ghidra_knowledge/tools/tests/fake_rea_mcp.py   | 426 +++++++++++++++
 .../fixtures/rea/analyze_function__0x007680a0.json | 342 ++++++++++++
 .../rea/annotate_native_function__0x0085d320.json  |  23 +
 .../tools/tests/fixtures/rea/close_binary.json     |   4 +
 .../tools/tests/fixtures/rea/open_binary.json      |  13 +
 .../rea/procedure_assembly__0x007680a0.json        |  14 +
 .../rea/procedure_assembly__0x00775930.json        |  14 +
 .../rea/procedure_assembly__0x00775950.json        |  14 +
 .../rea/procedure_assembly__0x00776db0.json        |  14 +
 .../rea/procedure_callees__0x007680a0.json         |  19 +
 .../rea/procedure_callees__0x00776db0.json         |  15 +
 .../rea/procedure_callers__0x007680a0.json         |  24 +
 .../rea/procedure_callers__0x00775930.json         |  14 +
 .../rea/procedure_callers__0x00775950.json         |  14 +
 .../rea/procedure_callers__0x00776db0.json         |  18 +
 .../fixtures/rea/procedure_info__0x007680a0.json   | 114 ++++
 .../fixtures/rea/procedure_info__0x00775930.json   |  46 ++
 .../fixtures/rea/procedure_info__0x00775950.json   |  46 ++
 .../fixtures/rea/procedure_info__0x00776db0.json   |  50 ++
 .../rea/procedure_pseudo_code__0x007680a0.json     |  15 +
 .../rea/procedure_pseudo_code__0x00775970.json     |  14 +
 .../rea/procedure_pseudo_code__0x00776db0.json     |  15 +
 .../resolve_native_call_targets__0x007680e1.json   |  24 +
 .../tests/fixtures/rea/set_addresses_names.json    |   5 +
 .../tests/fixtures/rea/xrefs__0x00a1b4a4.json      |  27 +
 .../tools/tests/test_fake_rea_mcp.py               | 434 +++++++++++++++
 .../tools/tests/test_rea_session.py                | 527 ++++++++++++++++++
 31 files changed, 2949 insertions(+), 1 deletion(-)
```
- **Dirty Files (154):**
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
  - ... +144 more (run `git status --short`)

## 2. Immediate Objective (D3)
**`rea_facts.py` live-only facts (indirect targets, body ranges, data xrefs) + `rea_questions --residual`; G3 on 46 R1 addrs**

Status: TODO
Task: `rea_facts.py` live-only facts (indirect targets, body ranges, data xrefs) + `rea_questions --residual`; G3 on 46 R1 addrs

## 2b. Upcoming Batches in Pipeline
- **D4** : `named_decomp.py` named decompile; G4 CSWCCreature
- **D5** : `fn_doc.py` + `persist_purposes.py`; CSWCCreature page + KDB purposes
- **D6** : R2 rebuilt (`--arm facts

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/ghidra_knowledge/DECOMP_PLAN.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate D3` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
