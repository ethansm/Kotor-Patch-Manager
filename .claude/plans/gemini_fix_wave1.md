# Plan FX1: fix the top Gemini bugs (WI-76, WI-85..88, WI-97..99) with a Haiku variant wave and Sonnet consolidators

## Context
G2 validated the Gemini 2026-10-14 review (`.claude/reviews/gemini_20261014/validated.tsv`, doc 37 BG26-BG41, WI-73..WI-101).
8 work items are `ready` and free-gate (`_shared_src` only, no owner session):
| WI | BG / fids | Files (write) | Test id |
|---|---|---|---|
| WI-76 | BG29 SH-3a-05 (+ SH-3c-01 moved here from WI-98, same test): `imageFor` pools null loads, so lazy retries never reload | GuiKit.h, HudKit.h (CompanionSelfHeal.cpp read-only), hudkit_test.cpp | cpp:hudkit_test |
| WI-85 | BG38 CO-2d-09/QH-2a-11/SH-3a-06: `heal()` logs "giving up" on the 20th refill even if it succeeded | EmbedTex.h, embtex_test.cpp | cpp:embtex_test |
| WI-86 | BG39 SH-3a-01: wheel callback passes engine size as client rect to `clientToEngineF` (INI path) | GuiKit.h, guikit_test.cpp (GuiInputProvider.cpp read-only unless ABI change) | cpp:guikit_test, suite:guikit |
| WI-87 | BG40 SH-3a-07/HB-1c-02: label leaked on `createLabelFromTemplate` and `bindByTag` CF_Vtable return (:551) | GuiKit.h, guikit_test.cpp | cpp:guikit_test |
| WI-88 | BG41 SH-3b-05: `pendingDone2` now==0 sentinel stores 1, then 0-1 wraps and times out at once | SelfHealUi.h, selfheal_ui_test.cpp | cpp:selfheal_ui_test |
| WI-97 | test gaps SH-3c-04, SH-3b-04, SH-3c-03, SH-3c-05 | actqueue/effects/embtex/filewatch _test.cpp | 4 cpp ids |
| WI-98 | test gaps SH-3b-01, SH-3c-06, SH-3c-07 (SH-3c-01 -> WI-76) | guikit/guitip/item_read _test.cpp | 3 cpp ids |
| WI-99 | test gaps SH-3b-02, SH-3c-08, SH-3b-06, SH-3c-09 | k2addr/party/sceneview/textfit _test.cpp | 4 cpp ids |
All paths are under `patch_manager_mods/_shared_src/`. Every brief embeds the FULL validated.tsv row and the doc 37 detail section.

User's requested shape: Wave 1 = several parallel Haiku 5.5 agents per WI, each with a different variant prompt, each proposing a
solution. Wave 2 = one Sonnet 5.5 per WI (own worktree) evaluates, verifies and consolidates. Opus only generates briefs, runs gates,
applies final diffs and commits.

## Phases (batch table; mark with `plan_manager.py mark-done <Id> --plan .claude/plans/gemini_fix_wave1.md --no-handoff`)
| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| FX1.0 | Persist plan + handoff, pre-flight (claims, dirt, test ids) | - | - | OPEN | | commit |
| FX1.1 | Context packs + Haiku briefs (script, $0) | FX1.0 | - | OPEN | | `fix1/` packs |
| FX1.2 | Haiku wave `fix1`: 37 proposal units, parallel | FX1.1 | - | OPEN | | proposals |
| FX1.3 | Sonnet consolidators: 8 parallel, one per WI, each in its own worktree | FX1.2 | - | OPEN | | `final.diff` x8 |
| FX1.4 | Apply in conflict order, gate, `close --manual` per WI, doc 37 resolution | FX1.3 | - | OPEN | | 8 commits |
| FX1.5 | Close: mark-done, handoff, memory, Saved State | FX1.4 | - | OPEN | | - |

Pre-flight result (FX1.0): no live claims; `_shared_src` clean; all 15 test ids exist; baseline 14/14 PASS at BASE `1963cca`.

### FX1.1 Context packs and briefs
`.claude/reviews/gemini_20261014/tools/fix1_briefs.py` writes `.claude/waves/fix1/<WI>/pack.md` (validated rows, doc 37 section,
write scope, test id, rg anchors), the Haiku unit briefs and `consolidate.md` per WI.
Variants: bug WIs A minimal / B contract / C test-first adversary (15 units); test-gap fids T1 behaviour pin / T2 mutation killer (22 units).
Proposal format: `## Design` (<=8 lines), `## Edits` (FILE + fenced OLD/NEW), `## Test`, `## Risks`.

### FX1.2 Haiku wave
Snapshot, launch 37 `model: haiku` units in batches of 12, `wave_tools.py check fix1` + `agent_contract.py check --scope '.claude/waves/fix1/**' -`.
Retry FAIL/MISSING once. Gate: >=2 proposals per bug WI, >=1 per test-gap fid.

### FX1.3 Sonnet consolidators
8 `model: sonnet` Agents, `isolation: worktree`, one message. Re-verify defect, evaluate every proposal (apply/build/test/red-on-BASE),
consolidate (correctness > minimal diff > no ABI change), verify (tests, compat_check, suite:guikit for 86/87), write absolute-path
`final.diff` + `verdict.md` + 5-line worker reply. WI-86 ABI growth = keep open, user GO.

### FX1.4 Apply, gate, commit
Order: G = WI-87 -> WI-86 -> WI-76 -> WI-98; E = WI-85 -> WI-97; free = WI-88, WI-99.
Per WI: `git apply --check` + apply (conflict -> one rebase request, then park); re-run test ids + compat_check; `wi_driver.py close
WI-NN --manual --paths ... --reason ...`; bug WIs mark BGnn DONE in doc 37. No installs.

### FX1.5 Close
mark-done phases, `/plan-handoff` last, `wi_driver.py lint` + `audit --apply`, memory updates (gemini review + variant-wave yield),
`agent_usage.py --since <FX1.2 start>`, 4-section Saved State.

## Verification
- FX1.2: 0 FAIL/MISSING after one retry; no repo edits by Haiku.
- FX1.3: 8 `final.diff` + `verdict.md`; red-on-BASE and green-after per new test.
- FX1.4: scoped tests + compat_check PASS per WI; `git show --stat` only that WI's paths; BG29, BG38-BG41 DONE.
- End: all touched test ids PASS together at final HEAD.

## Cost
About $5-6. No installs, pushes or GitHub posts.
