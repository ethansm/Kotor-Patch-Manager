# Handoff: texture add-on diversity research (10-12)

State: research waves 1-5 DONE (46 + 13 Haiku agents). Wave 5 (10-12): 51 uniform/armour/robe atlases annotated (`w5/out/H1-9.md`), 30 of the 35 deferred cards now KEEP-UNI (CATALOG section 12, `w5/fit_merged.tsv`, `tools/merge_w5.py`); 3 merged, 2 dropped. `w5/png` (13 MB of extracted game textures) is NOT committed; rebuild with `scripts/npc_div_texture_sheets.py` load_game. Deliverable `.claude/research/texture_diversity/CATALOG.md` (+ `catalog_final.json`). Nothing installed, no repo code changed except `tools/` harvesters under that research dir.

Key facts: 81 KEEP cards (21 A, 56 B, 4 C); commoner atlases cover civilians/thugs/patrons/workers only; 35 cards DEFER to uniform/armour bases (now resolved by wave 5); 32 species-locked; variant-system draft cites 7 non-KEEP cards (K024 K043 K058 K061 K078 K132 K150) so officer/TSF/Czerka/Onderon-soldier archetypes are blocked; corrected POC set in CATALOG.md section 5.

Lore: Fandom HTML is bot-blocked, `api.php` works (`python3 .claude/research/texture_diversity/tools/harvest.py`; `harvest_sections.py`; `canvas_rule.py` rebuilds `catalog_final.json` canvas_rule). Only `AurekFonts/Aurebesh_Rodian` declares MIT. Commons files are CC BY-SA (share-alike).

Next (pick one): (1) map atlases to NPC templates (registry/UTC, deterministic) to ground-truth the faction-to-atlas guesses (Vaklu, Covenant, Sith trooper/commando); (2) T2 island-mask tool replacing the 4x4 cell estimate; (3) in-game/REA look at Czerka, TSF, Exchange textures for logo and colour ground truth. T1/T2/T6 from the pipeline report still wait on user go. Not committed: harvest/ page text, w3/chunks, images.
