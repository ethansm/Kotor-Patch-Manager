# Handoff: texture add-on diversity research (10-12)

State: research waves 1-4 DONE (46 Haiku agents). Deliverable `.claude/research/texture_diversity/CATALOG.md` (+ `catalog_final.json`). Nothing installed, no repo code changed except `tools/` harvesters under that research dir.

Key facts: 81 KEEP cards (21 A, 56 B, 4 C); commoner atlases cover civilians/thugs/patrons/workers only; 35 cards DEFER to uniform/armour bases; 32 species-locked; variant-system draft cites 7 non-KEEP cards (K024 K043 K058 K061 K078 K132 K150) so officer/TSF/Czerka/Onderon-soldier archetypes are blocked; corrected POC set in CATALOG.md section 5.

Lore: Fandom HTML is bot-blocked, `api.php` works (`python3 .claude/research/texture_diversity/tools/harvest.py`; `harvest_sections.py`; `canvas_rule.py` rebuilds `catalog_final.json` canvas_rule). Only `AurekFonts/Aurebesh_Rodian` declares MIT. Commons files are CC BY-SA (share-alike).

Next (pick one): (1) wave-2-style annotation of uniform/armour atlases (n_czerkaoff01, pmb*/pfb* decoys, n_mandalorian01-03) to unblock 35 cards; (2) T2 island-mask tool replacing the 4x4 cell estimate; (3) in-game/REA look at Czerka, TSF, Exchange textures for logo and colour ground truth. T1/T2/T6 from the pipeline report still wait on user go. Not committed: harvest/ page text, w3/chunks, images.
