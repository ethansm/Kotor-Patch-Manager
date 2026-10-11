# Phase 3F apply procedure (from handoff Phase E/F sections)

1. Base choice: patches were made against `~/.cache/kotor2_review/fix/base` commit `a8ea0fb` (sanitized snapshot + overlays), not `22b56b4`. Real tree == base for all 40 files.
2. Clone: `~/.cache/kotor2_review/apply`, branch `fix/audit-findings` from `a8ea0fb`.
3. Batches applied in order 01..17, then rc fixture (T32db00261f), then extra Tc307b197f4. Overlap: batch_10 Tec4de88307 dropped in favor of batch_13 T7ccac6b666 (run_tests.py).
4. Per-patch command: not stated in Phase E. Apply sequentially; `git apply --check a b c` is a dead end (false FAIL on batch_13).
5. Held: Td18cc50320 (batch_06) held, deletes uncommitted H10 WIP. Phase F later dropped it for good.
6. Gates per batch = scoped gates vs baseline (logs in `apply/apply_logs/`). build_check FAIL=1 (queue_hud_src, H10 held).
7. Series: `git format-patch a8ea0fb..fix/audit-findings` into `~/.cache/kotor2_review/apply_series/0001..0019-*.patch`.
8. Land (Phase F): commit Agy WIP first (12 wip commits), then `git am --3way` the 19-patch series (0 conflicts), then Haiku/Sonnet wrap-ups.
9. Result: 34 commits on KOTOR-II on 78807d4, local only, not pushed.
