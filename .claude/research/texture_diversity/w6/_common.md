# Wave 6 common brief: gap atlases (Dark Jedi, smugglers, gangers, player bodies, species bodies) (read-only, images)

Goal: for each assigned vanilla KOTOR 2 body atlas (clothing, armour, robes, species skin), say WHERE an add-on (emblem, trim, rank bar, wear, stain, patch) could be painted and what to avoid. Unlike the commoner atlases, these do NOT share one layout: judge each atlas on its own.

## Inputs (view with the Read tool; PNG images)
- Your atlases: `.claude/research/texture_diversity/w6/png/<name>.png` (names in your task). Sizes are in `.claude/research/texture_diversity/w6/inventory.tsv`; the 2048 px originals were downsized to 512 for viewing. Some are species bodies (skin, scales or fur): mark `surface: skin` for those and note which parts are clothing.
- The commoner layout for comparison only: `.claude/npc_div/tex/sheets/ref_M.png`. Do not assume the same layout.
- Read nothing else.

## Zone grid
Divide each atlas into a 4x4 grid of equal cells named by column letter A-D (left to right) and row number 1-4 (top to bottom): A1 top-left, D4 bottom-right. State cells that are mostly black or empty (unused UV space: never paint there).

## Per atlas, one block (<= 10 lines)
```
### <name>
- what: <role, faction or species by name and look, e.g. "Czerka officer coat", 1 line>
- layout: <which body part each used cell holds, 1-2 lines; empty cells listed>
- base: <colours by body part, 1 line>
- paint_zones: <3-5 cells + body part where an add-on reads well, e.g. "B1 left chest panel, C3 cuff">
- avoid: <cells/features already detailed: buckles, seams, gloves, boots, skin, hard-surface plates>
- skin: yes|no|partial  (bare skin/scales/fur visible on limb or torso island, usable for tattoos, scars, face-free body paint); say the species if it is a species body
- surface: cloth|hard_plate|mixed|skin  (hard plate takes decals as flat paint only; no wear that implies fabric)
- fit_types: <which of EMBLEM TRIM WEAR STAIN TATTOO COLOR PATTERN HARDWARE suit best, order of fit>
- palette_clash: <colours that would vanish or clash on this base, 1 line>
- confidence: 1-5
```
End with `## OPEN` (max 3 lines). Be honest: if an image is unclear, say so and lower confidence. Describe only what you can see.

## Rules
- Write ONLY your own output file `.claude/research/texture_diversity/w6/out/<group>.md` (group name given in your task).
- Final reply <= 5 lines in the RECON schema: OUTPUT, EVIDENCE (= atlases viewed), FINDING (<= 25 words), OPEN, VERDICT.
- Stop at about 50k tokens.
