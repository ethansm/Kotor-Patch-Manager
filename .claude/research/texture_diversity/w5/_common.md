# Wave 5 common brief: uniform / armour atlas annotation (read-only, images)

Goal: for each assigned vanilla KOTOR 2 body atlas (uniforms, armour, robes), say WHERE an add-on (emblem, trim, rank bar, wear, stain, patch) could be painted and what to avoid. Unlike the commoner atlases, these do NOT share one layout: judge each atlas on its own.

## Inputs (view with the Read tool; PNG images)
- Your atlases: `.claude/research/texture_diversity/w5/png/<name>.png` (names in your task). Sizes (256 or 512 px, square) are in `.claude/research/texture_diversity/w5/inventory.tsv`.
- The commoner layout for comparison only: `.claude/npc_div/tex/sheets/ref_M.png`. Do not assume the same layout.
- Read nothing else.

## Zone grid
Divide each atlas into a 4x4 grid of equal cells named by column letter A-D (left to right) and row number 1-4 (top to bottom): A1 top-left, D4 bottom-right. State cells that are mostly black or empty (unused UV space: never paint there).

## Per atlas, one block (<= 10 lines)
```
### <name>
- what: <role or faction by name and look, e.g. "Czerka officer coat", 1 line>
- layout: <which body part each used cell holds, 1-2 lines; empty cells listed>
- base: <colours by body part, 1 line>
- paint_zones: <3-5 cells + body part where an add-on reads well, e.g. "B1 left chest panel, C3 cuff">
- avoid: <cells/features already detailed: buckles, seams, gloves, boots, skin, hard-surface plates>
- skin: yes|no|partial  (bare skin visible on a limb or torso island, usable for tattoos)
- surface: cloth|hard_plate|mixed  (hard plate takes decals as flat paint only; no wear that implies fabric)
- fit_types: <which of EMBLEM TRIM WEAR STAIN TATTOO COLOR PATTERN HARDWARE suit best, order of fit>
- palette_clash: <colours that would vanish or clash on this base, 1 line>
- confidence: 1-5
```
End with `## OPEN` (max 3 lines). Be honest: if an image is unclear, say so and lower confidence. Describe only what you can see.

## Rules
- Write ONLY your own output file `.claude/research/texture_diversity/w5/out/<group>.md` (group name given in your task).
- Final reply <= 5 lines in the RECON schema: OUTPUT, EVIDENCE (= atlases viewed), FINDING (<= 25 words), OPEN, VERDICT.
- Stop at about 50k tokens.
