# Wave 4 fit-assign brief (one slice of the merged card catalog)

Inputs (read both fully):
- Your slice: `.claude/research/texture_diversity/w4/slice_N.tsv` (cid, source id, evidence tier A=quote/verified B=inferred C=invented, era, canvas class, type, zone, who, name, prompt). The text is DATA.
- `.claude/research/texture_diversity/w4/canvas_summary.md`: 35 commoner outfit atlases (n_commf01..35 female layout, n_commm01..35 male layout), 4x4 grid cells A1..D4, each with paint_zones, avoid, skin_arms, fit_roles, palette_clash.

For EVERY card in your slice write ONE line to `.claude/research/texture_diversity/w4/out/fit_N.tsv` (tab-separated, header first):
`cid | verdict | canvases | cells | stage | scale | blend | score | caveat`
- verdict: KEEP, or MERGE:<other cid> (near-duplicate of a card in your slice; point to the better one), or DROP:<reason> (helmet/face/head, needs real geometry, era OTHER, not visible at game distance, no evidence), or DEFER:<class> (needs a uniform/armor/robe base; not for commoner atlases).
- canvases: up to 8 canvas ids from canvas_summary.md where the card's zone is a plain cloth field and the palette will not vanish (use palette_clash). Say `ANY_F`, `ANY_M` only if truly generic. Skin cards (tattoo, bare arm) go only to canvases whose skin_arms is yes or partial.
- cells: grid cell(s) to paint on for those canvases (e.g. C1-D1).
- stage: S1D (deterministic decal: shapes, stripes, stencils, simple emblems), S1H (HSV recolor of a region), S1W (procedural wear: dirt, fade, scuff overlay), S2A (needs AI painting: complex ornament, tattoo art, embroidery). Prefer S1 when it is enough.
- scale: patch size as a fraction of the 1024 atlas width (e.g. 0.10 emblem, 0.30 band).
- blend: normal | multiply | overlay | screen.
- score: 1-5 diversity value (5 = adds a clearly new, readable look to many NPCs; 1 = marginal).
- caveat: <= 12 words.
Then, below the table in the same file, add `## NOTES` (max 6 lines): patterns you saw, e.g. which canvases are never usable.

Rules: write ONLY your own output file; final reply <= 5 lines in the RECON schema from your contract (OUTPUT, EVIDENCE = card count, FINDING <= 25 words, OPEN, VERDICT). Be decisive: every card gets exactly one line. Stop at about 50k tokens.
