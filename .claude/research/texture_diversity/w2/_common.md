# Wave 2 common brief: canvas zone annotation (read-only, images)

Goal: for each assigned commoner outfit texture (1024x1024 RGB atlas, `n_commXNN`), say WHERE an add-on (emblem, trim, wear, tattoo, patch) could be painted and what to avoid. A later compositor paints add-ons into zones on the atlas.

## Inputs (view with the Read tool; they are PNG images)
- Layout reference sheets: `.claude/npc_div/tex/sheets/ref_M.png` (male, 10 outfits) and `.claude/npc_div/tex/sheets/ref_F.png` (female). All n_commm/n_commf atlases share ONE layout per sex. Study the layout once.
- Your canvases: `.claude/npc_div/tex/png/n_<name>.png` (name given in your task).
- One-line labels per canvas: `.claude/research/texture_diversity/canvas_inventory.tsv`.

## Zone grid (use this, do not invent pixel boxes)
Divide the 1024x1024 atlas into a 4x4 grid of 256 px cells named by column letter A-D (left to right) and row number 1-4 (top to bottom): A1 is top-left, D4 bottom-right. First, from the reference sheet, state ONCE (in a LAYOUT header, max 8 lines) which body part each cell mostly holds for this sex (e.g. "A1+B1 = shirt front and back wrap; D1-D3 = trouser legs; A2 = belt and gloves"), and mark cells that are mostly black/empty (unused UV space: never paint there).

## Per canvas (one block each, <= 9 lines)
```
### <name>
- base: <garment colours by body part, 1 line>
- paint_zones: <cells + body part where an add-on would read well, 3-5 items, e.g. "A1 left chest panel (plain cloth), B1 sleeve">
- avoid: <cells/features already detailed: belts, buckles, seams, gloves, boots, skin>
- skin_arms: yes|no|partial  (bare skin on arm/torso island, usable for tattoos)
- fit_types: <which of EMBLEM TRIM WEAR STAIN TATTOO COLOR PATTERN HARDWARE suit this canvas best, order of fit>
- fit_roles: <2-4 character archetypes this outfit plausibly suits: refugee, merchant, mechanic, noble, thug, farmer, entertainer, scholar, guard-civilian>
- palette_clash: <colours that would vanish or clash on this base, 1 line>
- confidence: 1-5
```
End with `## OPEN` (max 3 lines). Be honest: if the image is unclear, say so and lower confidence.

## Rules
- Write ONLY your own output file. Final reply <= 5 lines in the RECON schema from your contract (OUTPUT, EVIDENCE = canvases viewed, FINDING <=25 words, OPEN, VERDICT).
- Read nothing else. Stop at about 50k tokens.
