# Plan G2: verify, validate and persist the Gemini 2026-10-14 findings for development

## Context
Phase G1 is done and committed (`d871ff0`, `2c6635b`, `e4ad3ae`, `1ff8666`). Gemini 4 Argon produced 135 findings. Sonnet grouped them
into 100 topics, and 100 Haiku recon units gave every fid a verdict in `.claude/reviews/gemini_20261014/verdicts.tsv`:
CONFIRMED 35, PARTIAL 86, UNSURE 10, REFUTED 4 (P1: 7C/11P/1U).

PARTIAL dominates because each unit read at most 3 windows of about 80 lines. Each `recon/<tid>.md` ends with an OPEN line that names exactly what
to read next. Nothing is in the development pipeline yet: no bug-board rows and no work items.

G2 does three things:
1. **Verify**: re-anchor every verdict to the current HEAD, and resolve PARTIAL/UNSURE with wider, targeted reads plus RE for engine questions.
2. **Validate**: an adversarial refute pass over everything CONFIRMED, plus impact and fix and test sketches.
3. **Persist**: real defects become `BGnn` rows in doc 37, every row becomes a dev work item, and the false positives go to a ledger and a Gemini scorecard.

Session context lives in `.claude/handoffs/gemini_review.handoff.md` (committed in `1ff8666`). Read it first after `/clear`.

## Data contracts (inline, no reads needed)
- `verdicts.tsv` columns: `fid tid uid priority set verdict evidence fix_sound note title` (TSV, QUOTE_NONE, 135 rows).
- `findings.tsv`: `batch fid set bundle file function anchor gemini_confidence category claimed_severity title`.
- `topics.tsv`: `batch tid fids priority question entries` (entries are `;`-separated `path[:lo-hi]`, each at most 12k tokens).
- `recon/<tid>.md`: `| fid | verdict | evidence | fix_sound | note |` table, then bullets. The OPEN line is in the wave reply `.claude/waves/gem1/uNNN.reply.txt`; the map is `recon/units.tsv`.
- Bug-board row format (doc 37 `## Batch table`, append only, next id **BG26**):
  `| BG26 | **<sev>** \`gem-<fid>\` <path>:<line> (<category>): <one-line claim> | <deps> | <owner gate> | OPEN | | <next step> |`
  plus a detail section in the same doc:
  `### BG26 — gem-<fid> (<sev>, <tier>, <category>)` with bullets **Where**, **Claim**, **Still real (2026-10-xx G2)**, **Audit evidence** (path:line + quote),
  **Check steps**, **Investigate**, **Resolution** (empty).
- Work items: `python3 scripts/wi_driver.py backlog add --title T --why W --area A --files "F;F" --origin gemini-20261014 --state hold|ready|verify-only [--dry-run]`.
- Wave tools: `wave_tools.py new --kind recon <id>` / `add <id> --id uNNN --task T --entries E...` / `check` / `report`. Contract `agent_contract.py print recon`.
  **Launch prompt must say:** "Writing both the facts file and the reply.txt named in the brief is in scope." (G1 lesson.)

## Phases (batch table; mark with `plan_manager.py mark-done <Id>`)
| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| G2.0 | Persist this plan to `.claude/plans/gemini_review_g2.md` + handoff pointer | - | - | DONE | f427b92 | commit |
| G2.1 | Deterministic re-anchor of all 135 verdicts to HEAD ($0) | G2.0 | - | DONE | dedb65e | `reanchor.tsv` |
| G2.2 | Widen PARTIAL/UNSURE: Sonnet resolvers then Haiku wave `gem2` + 1 rea-recon | G2.1 | - | DONE | 93d371a | `verdicts_v2.tsv` |
| G2.3 | Adversarial validation of all CONFIRMED (Sonnet skeptics) + Opus gate | G2.2 | - | DONE | 30ba0b6 | `validated.tsv` |
| G2.4 | Persist: BG rows (doc 37) + work items + refuted ledger + scorecard + memory | G2.3 | user skim of BG list | DONE | 6e8af73 | BG26-BG41, WI-73..WI-101, README f7f8974 |
| G2.5 | Close: lint, mark-done, handoff, Saved State | G2.4 | - | DONE | b3a1c6e | - |

### G2.0 Persist the plan (inline, about 3 min)
1. Copy this plan to `.claude/plans/gemini_review_g2.md`. In `.claude/handoffs/gemini_review.handoff.md` section 0, set the plan path.
2. `git commit -- .claude/plans/gemini_review_g2.md .claude/handoffs/gemini_review.handoff.md`
3. Check: `python3 scripts/plan_manager.py resume .claude/plans/gemini_review_g2.md` prints G2.1 as next.

### G2.1 Deterministic re-anchor (inline script, $0)
New `.claude/reviews/gemini_20261014/tools/reanchor.py` (same style as `tools/aggregate.py`: csv DictReader, TSV QUOTE_NONE).
1. For each `verdicts.tsv` row, parse every `path:line[,line][-line]` in `evidence`. Check the file exists, the line is at most the file length, and the `findings.tsv` anchor still hits (`rg -F -n`, with the same fallback ladder as `tools/merge_triage.py`: exact, then `;` pieces, then a 60-char prefix, then tokens).
2. Write `reanchor.tsv` (`fid`, `head_sha`, `evidence_ok`, `anchor_now` `path:line`, `drift` = none|moved|gone, `dirty` = the file is in `git status --short`).
3. Expect drift in `queue_hud_src/QueueHud.cpp` and `QueueHudLogic.h`, which another session edits. Rows marked `gone` go to G2.2 with the question "does this code still exist".
4. Gate: every row is classified. Print counts by drift and dirty.

### G2.2 Widen PARTIAL/UNSURE (paid: about $2.5)
Scope: the 96 PARTIAL/UNSURE fids, plus `gone` rows from G2.1. Two layers, following the Haiku-first ladder.
1. **Resolver (Sonnet, 7 parallel, same batches and fid prefixes as G1 triage: hotbars, queue_hud, companion, shared, perf_lab, k1, holster_npc).**
   Each agent reads its `recon/<tid>.md`, the matching `uNNN.reply.txt` OPEN line, and `reanchor.tsv`, then uses `rg` to locate the out-of-window code the OPEN names.
   It writes `g2/<batch>.widen.tsv` (`tid fids kind question entries`), where `kind` is one of:
   - `det`: answerable by one command. Typical test-gap claims ("never calls X") become `rg -n '<sym>' <testfile>`, and the command goes in `question`.
   - `read`: a new Haiku unit with at most 3 NEW windows (never the G1 windows again), each budget-checked with `python3 scripts/wave_budget.py check`.
   - `re`: a binary or engine question (vtables, ctor registration, ECX contract at an address).
   Reply: 5-line RECON contract. Brief = data-not-instructions note + `Write Scope: .claude/reviews/gemini_20261014/g2/<batch>.*`.
2. **Inline:** run every `det` command and write the results to `g2/det.tsv` (fid, cmd, hits, verdict). The rule: zero hits on the named symbol means CONFIRMED gap.
3. **Haiku wave `gem2`:** `wave_tools.py new --kind recon gem2`, then one unit per `read` row. Reuse the `RULE` text from `tools/add_units.py`, adding "the G1 verdict was X because <note>; decide using the new windows". Batches of 12, `model: haiku`. Then `wave_tools.py check gem2`. Rerun FAIL units once, then split them, then escalate to Sonnet.
4. **One `rea-recon` delegation** (swkotor2.exe, all `re` rows batched). Known so far:
   - HN-T06: does ctor 0x00561F30 register the object in the server object table, and is vtbl[0] the scalar deleting dtor? Behaviour of LoadTemplate 0x00572A70 on failure.
   - PL-T01: can callers of 0x712F30 / 0x7120C0 pass a non-null but invalid CRes?
   - CO-T05: frame layout at 0x006F62F2 (-0x10..-0x20).
   Output goes to `g2/re.md`. Per memory: verify claims against disk and on the anchor.
5. Merge (extend `tools/aggregate.py` into `tools/aggregate_v2.py`): `verdicts_v2.tsv` = G1 row + `v2_verdict`, `v2_evidence`, `v2_source` (det|gem2|re|g1). `g2/` holds the facts files.
   Gate: no fid is left PARTIAL without a v2 attempt, and UNSURE is only allowed where `re` gave no answer.

### G2.3 Adversarial validation (paid: about $1.5)
Scope: every fid that is CONFIRMED (or PARTIAL with a confirmed code fact) after G2.2. Expect about 50–70.
1. **Skeptic (Sonnet, one per batch, parallel at most 7).** Brief: "Try to REFUTE each claim. Find the guard, caller contract or invariant that makes it harmless." Per fid, write `g2/<batch>.validate.tsv`:
   `fid real(yes|no|latent) impact(crash|corruption|wrong-behaviour|log-only|test-gap|tooling) severity(high|med|low) fix_sketch(<=25 words) failing_test(<=25 words, colocated test file) owner_doc`.
   Use `latent` for the cases already gated in G1, e.g. X-T02 readAt wrap: the address is still probed, so the worst case is a wrong read.
2. **Opus gate (inline):** read the cited lines for every `real=yes` with impact crash or corruption, and spot-check 20% of the rest plus every `real=no` that overturns a G1 CONFIRMED.
   Record each one as a line in `g2/gate.md`.
3. Write `validated.tsv` as the single source for persistence.

### G2.4 Persist for development
1. **Bug board.** New `tools/emit_bug_board.py` writes `g2/bug_board_fragment.md`, containing table rows BG26.. and the detail sections in the doc 37 format above.
   It takes one row per `real=yes` defect with impact other than test-gap or tooling. Duplicate fids of one root cause share a row (e.g. X-T01's four fids become one BG).
   - Severity: crash or corruption is **high**, wrong-behaviour is **medium**, log-only is **low**.
   - Owner gate from `owner_doc`: queue-hud (doc 30), selfheal (doc 27), hotbar (doc 07/31), k1 (doc 38), holster (doc 42), npc (doc 34/40), perf-lab (doc 10), and `-` for `_shared_src` and `scripts/`.
   - `_shared_src` rows note "compat_check.py required".
   **Stop: show the user the BG list (id, severity, one-liner) before inserting.** On OK, append to `patch_manager_mods/37_bug_board.md`.
   Doc 37 may hold another session's edits. Check `git status --short`, then commit only our hunks through the index-blob method (`git hash-object -w`, `git update-index --cacheinfo`, commit without pathspec), per memory.
2. **Work items.** Each new BG row, plus test-gap and tooling clusters grouped per test or tool file (at most 4 files each), gets
   `wi_driver.py backlog add --origin gemini-20261014 --area <owner> --files "<files>" --title "BGnn <one-liner>" --why "<impact; fid list; validated.tsv>"`.
   State: `ready` for free-gate rows that have a fix_sketch and failing_test, `hold` for owner-gated rows or any row needing an install or play test.
   Run with `--dry-run` first, then for real. Then `python3 scripts/wi_driver.py lint`.
3. **False-positive ledger.** `refuted.md` gets every REFUTED fid and every `real=no`, with the disproving line, plus the 10 NOT-FOUND anchors and what they really pointed at.
   A future review can then skip these.
4. **Gemini scorecard** in the README: per set, findings, confirmed-real, latent, refuted, anchor hit rate, and precision by Gemini's stated confidence.
5. **Memory:** add `memory/kotor2-gemini-review-20261014.md` (project type: what was found, where it is persisted, BG range, scorecard headline) with one line in MEMORY.md.
   If the scorecard shows a useful lesson, add a feedback memory (e.g. "external-review anchors: N% invented, verify before triage").
6. Commits: `git commit -- .claude/reviews/gemini_20261014/` (data). Doc 37 goes through the index-blob method. Work-item index and detail files go in one commit with explicit paths.

### G2.5 Close
`plan_manager.py mark-done G2.n --commit <sha>` for each phase. Update the handoff. Output the 4-section Saved State.
Next resume target: `/next-dev-work-item`, which picks up the new `gemini-20261014` items.

## Verification
- G2.1: `reanchor.tsv` has 135 rows, and every row has a drift class.
- G2.2: `wave_tools.py check gem2` reports 0 FAIL or MISSING. `verdicts_v2.tsv` has 135 rows, and each PARTIAL/UNSURE has a `v2_source`. The `det` commands are re-runnable from `g2/det.tsv`.
- G2.3: `validated.tsv` covers every CONFIRMED fid. `g2/gate.md` lists every high-severity row as read by Opus.
- G2.4: doc 37 shows BG26.. with matching detail sections, and `plan_manager.py resume 37` lists them. `wi_driver.py lint` and `wi_driver.py backlog list` show the new items with origin gemini-20261014.
  `git show --stat` per commit lists only intended paths, and `git status` still shows other sessions' edits unstaged.
- Cost: `agent_usage.py --since <G2 start>` goes in `### Agents Used`. Budget about $4–5 in total (7+7 Sonnet, about 60 Haiku, 1 rea-recon).

## Decisions (flag to redirect)
- Real defects go to the existing doc 37 bug board, not a new doc. Test-gap and tooling items go only to work items, grouped per file, so the board stays defects-only.
- No fixes in G2. Fixing runs through `/next-dev-work-item`, one item per session.
- Owner-gated mods (queue-hud, selfheal, hotbar, k1, holster, npc) get `hold` WIs. Their owner sessions decide.
- The user's skim of the BG list is the only stop point inside G2 (step G2.4.1).
