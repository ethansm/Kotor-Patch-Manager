# R1 pilot: REA live evidence on B13a review cards (2026-10-15)

**Verdict: NO-GO** for "REA-enriched Haiku reviewers" as the reviewer tier. The REA plumbing (DESIGN 4.9) stays in
place as report-only evidence. REA facts were accurate. The Haiku reviewers are the weak link, not the evidence.

## Sample
- `key_pilot.tsv`: 30 real cards + all 20 B13a decoys = 50 cards, 46 distinct Steam addresses.
- Real cards: every on-card Opus `label_suspect` (14), every Opus address correction (11, incl. 1 unresolved) and the
  other unresolved row (1), plus 4 Opus-verified rows (seed `B13a-pilot`).
- Cards = the COMMITTED B13a card text (what the Sonnet reviewers saw), with or without the REA section
  (`build_pilot_cards.py`). A fresh `cards B13a` differs from the committed cards in 25 lines: corpus drift (newer trusted
  rows add A-CALLEE credit). HEAD's batch.py gives the same output as the new code without `--rea`, so the drift is not
  from this change.

## REA wave
- `tools/rea_questions.py B13a --keys B13a/rea_pilot/key_pilot.tsv` -> 6 question files (<= 8 addrs, blind).
- Ledger: `ghidra_knowledge/rea_findings/20261015_B13a-q{1,2,3}.tsv` + `_2` caller addenda, `q{4,5,6}_2.tsv`.
- Every one of the 46 addresses got a `function` row with a quoted asm line.
- Ops:
  - 3 parallel runs worked in round 1.
  - In round 2, 3 runs (with another session's REA live) all died on the first procedure call ("Connection closed").
    Retried 2 + 1 with one reconnect allowed: all OK.
  - Rule now in the template: reconnect once; run at most 2-3 REA runs in parallel.
- Round-1 ledgers had no caller rows (callers were only in the replies), so the agents added `_2` addenda. The template
  now asks for a `kind=xref` row (callers, callees, size, RET).
- Run cost: 88k-227k tokens and 8.5-14 min per run, with 30-85 tool calls. Over the 100k target in 5 of 6 good runs.

### Opus spot-check (ledger vs static `decomp/*.asm`): 4/4 exact
| ledger fact | asm dump |
|---|---|
| 0x7680a0 `0x7680d1: PUSH 0x272e`, `0x7682a5: RET 0x8` | `007680d1 PUSH 0x272e`, `007682a5 RET 0x8` |
| 0x776db0 `0x776dd6: MOV [ECX+0x3c0],0x0`, RET 0x4 | `00776dd6 MOV dword ptr [ECX + 0x3c0],0x0`, `00776e01 RET 0x4` |
| 0x774260 strings Fx_Shadow_S/_M/_L at 0x99f1a4/98/8c | `00774328 PUSH 0x99f1a4`, `00774337 PUSH 0x99f198`, `00774346 PUSH 0x99f18c` |
| 0x778900 size 55, RET 16 | `00778934 RET 0x10` |

Sizes and RETs also match `steam_functions.tsv` for 10/10 checked addresses.

## Score (`score_pilot.py`; acceptance = `bdlib.reviewer_accepts`, finalize's rule; 1 reviewer per card per arm)
| arm | rows | LS flagged | ADDR rejected | OK accepted | decoys accepted | unsure |
|---|---|---|---|---|---|---|
| Sonnet (original B13a reviews) | 50 | 10/14 | 11/11 | 4/4 | 0/20 | 0 |
| Haiku + REA | 50 | 7/14 | 10/11 | 4/4 | **1/20** | 6 |
| Haiku, no REA (control) | 50 | 5/14 | 11/11 | 4/4 | 0/20 | 8 |

- The GO bar was LS >= 70%, 0 decoys and < $0.05/card. Haiku+REA got 50% LS and accepted 1 decoy: **fail**.
- REA lift on Haiku: +2 LS (1-10, 1-51) and 2 fewer unsure. But the 7 REA flags are a subset of Sonnet's 10, so REA
  found nothing Sonnet missed.
- The 4 LS rows that every arm missed (1-01, 1-15, 1-34, 2-18) were Opus-only finds (Mac name, caller context).
- Accepted decoy: B13a-1-53 = 0x775970, one of the 0x775930/50/70/90 `[this+0x224]` forwarder twins. The REA callee
  fact (`CALL 0x85d320`) is correct but does not separate the twins, and the reviewer took the twin's callee as an
  anchor. This is the rotation trap: REA rows need twin separation (`decoy` / neighbour rows on the card) before they
  can anchor.

## Cost (agent_usage.py, est.)
| item | $ | $/card (50) |
|---|---|---|
| REA runs (6 good + 3 failed) | 1.13 | 0.023 |
| Haiku review with REA (2) | 1.20 | 0.024 |
| **REA-enriched total** | 2.33 | **0.047** |
| Haiku review without REA (2) | 1.13 | 0.023 |
| build workers (3) | 0.11 | - |

The cost bar passes, just barely: REA about doubles the Haiku review cost.

## Decision and next rows (MASTER_PLAN)
- REA stays report-only (DESIGN 4.9). No trust route, no AUTO change.
- R2 (replaces the GO-path R2): Sonnet reviewers + REA on the same 50 pilot cards. Does REA lift Sonnet past 10/14,
  and does it catch any of the 4 Opus-only rows? Cost bar: under $0.08/card. If not, REA is used only for the u06 fill.
- R3 (only if R2 is GO): REA in B17 cards.
- R4 (optional): Opus calibration to let REA callers count as A-CALLEE.
- Twin rule before any REA-anchored accept: the card must show REA rows for the same-shape neighbours too.

# R2 pilot: static facts (+ REA) on the same 50 cards (2026-10-16, DECOMP D6)

**Verdict: NO-GO.** No arm beats Sonnet-orig 10/14 and no arm catches any of the 4 Opus-only rows. Static facts are
$0 and stay on cards as report-only evidence. REA stays report-only (u06 fill only). R3 does not run.

## Arms
- `build_pilot_cards.py --arm facts|facts_rea`: same ids, order and committed body as R1, plus the
  `### Static facts (script output, report-only)` section (and the REA section) from `cards B13a --facts [--rea]`
  (`cards_full_facts*/`). R1 `cards_rea/` + `cards_norea/` regenerate byte-identical (`test_build_pilot_cards`, 6 OK).
- Facts sections are small (~190 B/card); pilot parts 105-126 KB.
- Reviewers: 2 Sonnet per Sonnet arm (25 cards each); haiku+facts split into 4 Haiku parts of 12-13 cards
  (48-58 KB), to try to stay near the 60k ctx cap.

## Score (`score_pilot.py`; $/card from `review/cost.tsv`; sonnet+facts+rea includes R1's $1.13 REA runs)
| arm | rows | LS flagged | ADDR rejected | OK accepted | decoys accepted | unsure | $/card |
|---|---|---|---|---|---|---|---|
| sonnet (orig) | 50 | 10/14 | 11/11 | 4/4 | 0/20 | 0 | - |
| haiku+REA (R1) | 50 | 7/14 | 10/11 | 4/4 | 1/20 | 6 | 0.047 |
| haiku no-REA (R1) | 50 | 5/14 | 11/11 | 4/4 | 0/20 | 8 | 0.023 |
| sonnet+facts | 50 | 9/14 | 11/11 | 4/4 | 0/20 | 1 | 0.045 |
| sonnet+facts+rea | 50 | 8/14 | 11/11 | 4/4 | 0/20 | 2 | 0.067 |
| haiku+facts | 50 | 4/14 | 10/11 | 4/4 | **1/20** | 5 | 0.017 |

- GO rule (LS > 10/14 or >= 1 of 1-01/1-15/1-34/2-18; 0 decoys; < $0.08/card): every arm fails the LS leg.
- All Sonnet arms answer `match` on 1-01, 1-15, 1-34, 2-18 (the Opus-only rows need Mac names / caller context that
  neither facts nor REA carry). Sonnet+facts loses 1-56 (match); +rea also loses 2-13 (unsure). Facts add no LS lift.
- haiku+facts: 4/14 (below R1 no-REA 5/14). It accepted decoy B13a-1-31 = 0x775930, another twin of the
  0x775930/50/70/90 `[this+0x224]` forwarder family. It cited the callee `0x00775940 CALL 0x0085D260` as the
  "distinguishing anchor", but that callee is not mapped to the GOG side, so the twins rule was obeyed in form only.
  It did answer `unsure` on R1's trap 1-53. Haiku is not a cheap default.
- Haiku ctx: all 4 Haiku reviewers peaked at 140-161k despite 48-58 KB inputs (dump reads dominate); the 60k cap
  needs a per-call read budget enforced by tooling, not by the brief.
- Hygiene (self-reported, no answer files opened): one Sonnet printed 150 B of R1 `norea_2.tsv` (card 1-41), one saw
  `[rea]` ledger lines via `query_ghidra.py` that matched its card, one `rg` listed `opus/` paths, one Sonnet used
  `upstream_server/.../fp.tsv` (GOG fingerprints, outside the brief's tool list).

## Cost (agent_usage.py, est.)
| item | $ |
|---|---|
| T6.1 builder (Haiku) | 0.03 |
| Sonnet facts (2) | 2.23 |
| Sonnet facts+rea (2) | 2.22 (+1.13 R1 REA runs = 3.35 arm) |
| Haiku facts (4) | 0.84 |
| **R2 total new spend** | **5.32** |

## Decision (MASTER_PLAN)
- R2 DONE (NO-GO). R3 NO-GO: no REA in B17 cards. Facts sections stay as $0 report-only card evidence.
- The remaining LS gap is name/context evidence (Mac/Linux names, caller context), not body facts: a future pilot
  would put upstream names on the card, not more REA.
