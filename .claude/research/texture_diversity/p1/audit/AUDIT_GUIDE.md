# Atlas audit guide (recon; read-only except your findings file)

Goal: find GAPS (clothing/armour pieces our looks leave untouched that should be recoloured) and MISIDENTIFIED zones (annotation or recipe says a cell is X but the image shows Y; or a look paints something it must not: hands, bare skin, visor/lens glass, gloves, belts, boots, pouches, hardware, emblem).

Audit image = 2x2 panels of one atlas, rows in stored order, yellow grid, cell names A1 (top-left) .. D4 in each cell's top-left corner:
- top-left: the atlas as is.  - top-right: UV islands in colours; legend (label names) top right.
- bottom-left: GREEN = pixels some look of this atlas paints.  - bottom-right: gap map of content NOT painted: RED = non-skin, YELLOW = skin-like, CYAN = also outside every UV island (usually padding, bleed or unused UV, normally not worth painting).

Method: (1) Read the audit image. (2) Read the stats json (cells: content/painted/skin_like fractions; islands). (3) Read the source md (annotation + recipe layers). (4) Compare panels: name each large red region (cell range + what it is: cloth panel, plate, leather, glove, boot, pouch, hardware, lens, trim, padding) and decide if it should be painted by a palette recolour (cloth and plates: yes; gloves, boots, belts, pouches, hardware, lens, skin: no). (5) For each region you want painted say which island label covers it (top-right panel) so a fix can name islands instead of cells. (6) Check green regions for anything that should not be painted.

Write ONLY your findings file (path in the brief), max 40 lines, one finding per line, exactly:
`GAP|MISID|OVERPAINT | cells | what it is | island label(s) covering it | action: add island X / widen cells / keep excluded / fix annotation | confidence 1-5`
First line: `SUMMARY: <atlas> painted <n>% of content; <one sentence>`. No prose beyond that. Judge only what you can see in the image; if unsure say confidence 1-2.

Rules for judging: the annotation's avoid lists were eyeballed and are the SUSPECT here. Decide from the image, not from the annotation. Do not report "paints an avoid cell" unless the image shows the painted pixels are really a glove, boot, belt, pouch, hardware, lens, skin or emblem. You MUST open the audit PNG; a findings file written without looking at the image is rejected.
