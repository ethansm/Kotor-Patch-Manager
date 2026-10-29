# B20 Opus gate part a (lead, inline)
B20 = I9 graphpair rows (fp-equal or call-graph neighbours mapped through trusted pairs) + I9c B18 re-checks.
- Graph check on every verified row: all 26 agree with the graphpair pick; sample 5/5 + AUTO meaning 3/3 correct.
- Overrides: 10 unresolved -> verified (strong graph, fp identical or reviewer match), 12 -> likely.
- Left unresolved: CExoLinkedListInternal::Remove_2 (rival 33 vs 36), PowersLevelUp dtor / Union / gluPerspective (1 hit; Union body reads as intersection).
- Corrections to re-gate later (gate top != graph pick): RenderDrawableSquares -> 0x004356e0, AurCubeMapAvailable -> 0x00484970,
  AurEnableShadows -> 0x00478ca0, GetTextureTID -> 0x004daf90.
- Reviewer format slip: 3 rows had verdict blank and `match` in corrected_addr (column shift).
sample_errors: 0
