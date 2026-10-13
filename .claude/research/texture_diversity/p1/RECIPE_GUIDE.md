# Recipe-writing guide (commoner looks)

Deliverable: ONE JSON file = a list of recipe dicts, one per (look, atlas) pair, written to the path in your brief. Validate each with
`.venv/bin/python -c "import sys,json; sys.path.insert(0,'patch_manager_mods/k1_port'); import k1_port_tex_overlay as o; r=json.load(open(sys.argv[1])); [print(x['id'],o.validate_recipe(x)) for x in r]" <file>` ; every list must print `[]`. Fix and re-run until clean.

Recipe: {"id":"P3-m05","family":"commoner","atlas":"n_commm05","model":"n_commm","note":"what + which cells dropped and why","seed":1,
 "layers":[{"kind":"recolor|trim|emblem|wear","op":"recolor_hsv|band|decal|wear|tint","cells":["A1","B1"],"islands":["torso"],"avoid_cells":["A2"],"card":"K067","params":{...}}]}
- model is "n_commf" for n_commf* atlases and "n_commm" for n_commm* atlases. id = "<look>-<f|m><NN>" e.g. "P2-f07".
- At most 3 layers, at most 1 emblem layer (and at most 1 decal op). Keep each look to 2 or 3 layers: one broad layer (recolor_hsv or tint or wear), plus one trim/emblem.
- Cells come ONLY from that atlas's `paint_zones` lines in the source file, minus anything listed in its `avoid`. NEVER put an avoid cell in `cells`; ALSO copy that atlas's avoid cells into the layer's `avoid_cells`. If a card's zone has no paintable cell on this atlas, drop that card and say so in `note`. Cell coordinates: A1 top-left .. D4 bottom-right (column letter, row digit).
- islands: use ["torso","legs"] for cloth (all commoner cloth is in those islands); never "arms" (hands).
- Params (all colours are "#rrggbb"):
  - recolor_hsv: hue_shift_deg (-180..180), sat_scale (0..2), val_scale (0..2), keep_skin true (default). For a palette change use sat_scale 0.8-1.2, val_scale 0.8-1.1, hue shift 20-90 deg; avoid extremes.
  - band: orientation "horizontal"|"vertical"|"diagonal", period_px (8-48), width_px (<= period_px, 3-10), colour, opacity 0.5-0.9, angle_deg 0. A stripe across a cell is period_px ~ cell size (64 at 256 atlas, 128 at 512), width 6-10.
  - decal: name (one of bar_stack, chevrons3, cog8, crate_stencil, hawk, jaig_eyes, medallion, patch_round, star5, sprout, sun_disc, swoop_stripes), cx, cy (centre in atlas pixels; atlas is 512x512 for these commoners; cell (col,row) centre = (col*128+64, row*128+64) with A=0..D=3 and rows 1..4 -> 0..3), size_px 24-56 (never larger than the cell, 128), rotation_deg, colour, opacity 0.8-1.0, blend "normal"|"multiply".
  - wear: kind dust|fade|stain|scuff, strength 0.15-0.5, direction down|up|none (seed optional).
  - tint: colour, strength 0.1-0.4.
- Colour guidance: pick muted, cloth-plausible colours that contrast with the annotation's palette_clash line (e.g. do not use tan on a tan field).
- Emblem mapping: K033 sprout->decal sprout; K067 crew patch->patch_round or hawk; K054 chevrons->chevrons3; K066 crate stencil->crate_stencil; K042 Czerka emblem->cog8 or medallion; K063 sleeve tag->bar_stack small; K050 badge->patch_round small. Trim cards (K068, K053, K045, K076, K077, K065) -> band. Wear cards (K027, K020, K037, K110, K049, K091) -> wear. Colour cards (K072, K074, K115) -> recolor_hsv or tint; K069 pattern -> band diagonal or decal medallion.
- Output ONLY the JSON file; no other file.
