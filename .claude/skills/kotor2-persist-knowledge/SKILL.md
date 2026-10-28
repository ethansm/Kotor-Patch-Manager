---
name: kotor2-persist-knowledge
description: >-
  Save what ad-hoc research found into the KOTOR II saved knowledge before the session ends or context is cleared:
  REA ledger rows, the mod's plan doc, RE_AGENT_GUIDE traps, research notes, play-test rows and memory. Use after any
  one-off investigation during patch/mod development (a decompile read, an address or struct offset worked out, a
  log diagnosis, a dead end, a live-game finding), when the user says "persist this", "save what we learned",
  "record the findings", or before /plan-handoff when research happened outside a plan phase.
---

# Persist ad-hoc research

Rule: nothing learned may live only in chat, in a subagent transcript, or in a scratch dir outside the repo.
Research spans many sessions and context clears; an unrecorded fact gets paid for twice.

## 1. Pick the sink for each finding (one finding can go to several)

| Finding | Sink | How |
|---|---|---|
| Name/role of an exe address (function, global, vtable, string, field) | REA ledger `patch_manager_mods/ghidra_knowledge/rea_findings/<YYYYMMDD>_<topic>.tsv` | Append rows (cols `addr kind name confidence evidence topic date`; `confidence` = confirmed\|probable\|refuted; evidence starts `0x<addr>: <quoted insn or line>`). Check first: `python3 scripts/query_ghidra.py address <hex>`. Then `python3 patch_manager_mods/ghidra_knowledge/tools/rea_ledger.py lint` must not add issues |
| Struct field offset, calling convention, hook site | The mod's plan doc (`patch_manager_mods/<NN>_*.md`) ledger/findings section + a code comment at the use site | Quote the decomp/asm line and its address |
| A trap that misled you or a model (decoy twin, wrong offset base, misleading label) | `patch_manager_mods/ghidra_knowledge/RE_AGENT_GUIDE.md` §3 Traps | One short subsection: symptom, how to detect, example address |
| Longer write-up (several findings, method, evidence) | `.claude/research/<topic>/README.md` (or `REPORT.md`) | Link it from the plan doc |
| Dead end / refuted hypothesis | Plan doc "Dead ends" or the plan's handoff `.claude/handoffs/<plan_stem>.handoff.md` | State what was tried and why it failed, so nobody retries it |
| Something only the live game can confirm | `patch_manager_mods/PLAY_TEST_QUEUE.md` | `pending` row with steps + pass signal; never block on it |
| Cross-session status, user preference, decision | Memory (`~/.claude/projects/.../memory/`) | Update the existing project file; add an index line only for a new file |

Never write `name_map` / the knowledge DB directly from ad-hoc research: names reach it only via a ledger row that
passed a Haiku check with decoys and the user's OK: `ingest_names.py cards|score` then `promote_names.py`
(all in `patch_manager_mods/ghidra_knowledge/tools/`, see `ghidra_knowledge/INGEST_PLAN.md` I2/I4).

## 2. Steps (about 5-10 min)
1. List the findings of this session (max 10 lines; drop anything already recorded: `rg -n '<addr or name>'` first).
2. Write each to its sink (table above). Quote evidence; mark guesses `probable`, not `confirmed`.
3. If you added ledger rows: `python3 patch_manager_mods/ghidra_knowledge/tools/rea_ledger.py lint` (issue count must not grow).
4. If you added a new script or command: list it in the agent docs in the same step (CLAUDE.md "Fast Local Tools",
   RE_AGENT_GUIDE, SUBAGENT_ROLES or the owning skill).
5. Commit only your paths: `git commit -m "<topic>: persist findings" -- <paths>` (the worktree is shared).

## 3. Done check
- Every finding has a file:line in the repo (or a memory file) you can name.
- `rea_ledger.py lint` count unchanged or lower; no raw hex went into `raw_addresses.toml` without `query_ghidra.py address`.
- Report the paths in `### Actions Completed`.
