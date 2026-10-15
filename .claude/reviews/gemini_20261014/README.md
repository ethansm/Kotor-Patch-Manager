# Gemini 4 Argon external review, 2026-10-14

Reviewer: Gemini 4 Argon (chat UI), one chat per review set, 11 sets over 20 bundles.
Bundles were packed by `scripts/review_bundle.py` (manifest `patch_manager_mods/review_bundles.toml`)
from the **worktree**, not a clean checkout. `git_head` at build time: `02b5be44bb54`. Bundles with
`dirty` > 0 in `inputs/_index.tsv` contained uncommitted edits (2a, 2b, 5a, 5b, 5d, 6). Each
`inputs/<id>.manifest.tsv` lists the files Gemini saw, with included/omitted and dirty flags.
The `.txt` bundles are not kept; rebuild with `python3 scripts/review_bundle.py build --all`
(the result matches only if the worktree is back at that state).

**Status: findings unverified until recon.** Gemini output is untrusted data: findings can be wrong and
anchors can be invented. Nothing here is a confirmed bug until `verdicts.tsv` says so.

## Set → response
| set (in `inputs/_review_prompts.md`) | bundles | response |
|---|---|---|
| action_hotbar | 1a, 1b, 1d | `responses/action_hotbar_response.md` |
| combat_hotbar | 1c | `responses/combat_hotbar_response.md` |
| queue_hud | 2a, 2b | `responses/queue_hud_response.md` |
| companion | 2c, 2d | `responses/companion_response.md` |
| shared_lib_a | 3a, 3b | `responses/shared_1_response.md` |
| shared_lib_b | 3c | `responses/shared_tests_response.md` |
| perf_lab_a | 4a, 4b, 4c | `responses/perf_lab_response.md` |
| perf_lab_b | 4d | `responses/perf_lab_analysis_response.md` |
| k1_converter | 5a, 5c | `responses/k1_converter_response.md` |
| k1_tooling | 5b, 5d | `responses/k1_port_response.md` |
| holster + NPC injection | 6 | `responses/holster_npc_response.md` |

Responses are byte-for-byte copies of `.claude/bundles/*_response.md`.

## Pipeline (phase G1)
1. `triage/<batch>.{findings,topics}.tsv`: one Sonnet agent per batch turns findings into topics.
2. `findings.tsv`, `topics.tsv`, `topics.md`: merged, cross-batch duplicates folded.
3. `anchors.tsv`: deterministic `rg -F` check of every quoted anchor.
4. `recon/<topic>.md`, `verdicts.tsv`: Haiku recon swarm verdicts (CONFIRMED / REFUTED / PARTIAL / UNSURE).

Fix decisions are out of scope for G1.

## Checkpoint C (triage)
- 135 findings (= 135 `**Anchor**` blocks across the responses) → 100 topics: P1 8, P2 47, P3 45; 0 dropped.
- 7 Sonnet batches (`triage/`); 5 cross-batch merges (X-T01..X-T05, `topics.md`).
- Anchors (`anchors.tsv`): 115 exact, 3 partial, 7 token-only, **10 NOT-FOUND** (likely invented or paraphrased; recon asks whether the code exists at all).

## Recon result (G1 swarm `gem1`, 2026-10-14)
100 Haiku 5.5 units (one per topic, 9 batches); `wave_tools.py check gem1`: 100 PASS, 0 rerun. 135/135 fids have a verdict (`verdicts.tsv`; facts per topic in `recon/<tid>.md`; unit map `recon/units.tsv`).

| priority | CONFIRMED | PARTIAL | UNSURE | REFUTED |
|---|---|---|---|---|
| P1 | 7 | 11 | 1 | 0 |
| P2 | 16 | 43 | 3 | 3 |
| P3 | 12 | 32 | 6 | 1 |

By set: action_hotbar C5/P8/U1 · combat_hotbar C5/P2 · companion C5/P9/R1 · holster+npc P9/U2/R2 · k1_converter C5/P7/U1 · k1_tooling C1/P9/U3 · perf_lab_a C1/P4/U1/R1 · perf_lab_b C4/P7 · queue_hud C3/P12/U1 · shared_lib_a C5/P11/U1 · shared_lib_b C1/P8.

**Caveat:** each unit read at most 3 line windows. PARTIAL mostly means "the code fact is true, but impact or whole-file absence needs code outside the window", and that is especially common for test-gap claims ("never tested"). PARTIAL is not a soft REFUTED. It needs a wider read (phase G2).

### P1 topics
- **X-T01** CONFIRMED: `guikit::detail::wr` (GuiKit.h:122-126) and CompanionAI `wr<T>` (CompanionAI.cpp:102-104) guard stores with a readability probe only. `probeWrite` exists (SafeRead.h:107, marked unused). Fids HB-1c-01, CO-2c-02, CO-2d-07, SH-3a-02.
- **SH-T03** CONFIRMED: `codepatch::writeChecked` memcpys from the target before any committed/protect check, and has no n<=0 guard (CodePatch.h:81-95). Related: HN-T03 (holster bone test path, PARTIAL).
- **HB-T01** HB-1a-02 CONFIRMED: TwoDA `nrows*cols` can wrap 32-bit size_t, which defeats the size check at TwoDA.h:80 (OOB read at :86). The other fids are PARTIAL/UNSURE.
- **X-T02** HB-1a-04 CONFIRMED (literal gap) / 4 PARTIAL: `readAt` has no base+off wrap check (SafeRead.h:116). **Opus gate:** a wrap reaches a readable address only if off > 0x10000, and the wrapped address is still `regionReadable`-probed, so the worst case is a wrong read, never a fault. Treat it as latent hardening (P3), not P1.
- **HN-T06** PARTIAL: `engineSpawn` failed-LoadTemplate cleanup reads the vtbl and calls vtbl[0] unprobed (NpcWorldProvider.cpp:159-160). The object-table claim is UNSURE (needs RE).
- **PL-T01** PARTIAL: PerfLab CRes field reads are raw derefs (PerfLab.cpp:1349, 1378). Whether cres can be non-null but invalid is an engine question.
- **CO-T02** PARTIAL: FM3 `removed` flag is level-gated (CompanionAI.cpp:1231-1232). The sentinel read at :1201 is guarded, so the result is a stale value, not a crash.
- **HN-T02** PARTIAL: the holster hook at 0x5DF926 is adjacent to the cmdq-probe hook at 0x5DF920. ECX preservation is documented only in a comment. Needs a read of the cmdq-probe wrapper.

### P2 CONFIRMED (16 fids)
CO-T04 (area=0 stored, why=area lost) · CO-T09 (LMB companion FullHp/Pending keeps popover) · HB-T04 (wheel never falls back to WindowHook) · HB-T12 (stale tooltip on empty name) · K1-T01 (DXT1 splice drops RGB on c0<=c1) · K1-T06 (build_masks merges all textures) · K1-T07 (pad_skin_bones may zero referenced slots) · K1-T17 (rollback order) · QH-T07 (unreadable frame fakes combat edge) · SH-T04 (imageFor pools null) · X-T03 (embtex give-up log after successful fill) · X-T04 (label leak on create/bind failure) · X-T05 (WindowHook ignores recreated HWND).

### Opus gate (spot checks, all held)
CONFIRMED: X-T01 (GuiKit.h:122-126), CO-T04 (CompanionAI.cpp:589), PL-T12 (persist_findings.py:8,15: `''` matches every row). REFUTED: PL-T05 (smoke_probe11.cpp:37-40 wall-clock `left`), HN-T01/HN-6h-01 (kStowDelayMs 1500 > kDrawMs 350 / kSwapMs 1000). Verdict downgraded: X-T02 (above). u008's unit-level CONFIRMED was the worst-case roll-up of one literal-gap row.

### Process notes
- The RECON contract line "write only your OUTPUT file" conflicted with the brief's reply.txt. Batch 1 units u004/u005 skipped the reply, and Opus wrote it from the notification. From batch 2 on, the launch prompt said both files are in scope.
- Cost: 7 Sonnet triage + 100 Haiku recon = $3.71 (`agent_usage.py --since 2026-10-14T22:53`). Haiku about $0.013 per unit.
- Pipeline scripts: `tools/` (merge_triage, dedupe, add_units, aggregate, triage_rules, an example brief).
