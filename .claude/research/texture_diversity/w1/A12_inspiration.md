# A12 Inspiration and technique sources (wave 1)
Non-lore inspiration. `who` uses generic role labels (in-game names not verified). `UNFETCHED` = search snippet only (page returned 402/403 or was not read).
## A12-01: Hige whisker fade at hip
- who: Dock Worker, Refugee, Mercenary
- type: WEAR
- zone: hip_pocket, belt
- prompt: Pale grey-blue horizontal creases across the hip and belt, soft edges, as if from years of bending and sitting.
- palette: pale grey-blue, dark indigo
- lore: INFERRED | https://www.heddels.com/?p=674 (read: whiskers are "long, stretched creases often found around the waist and crotch area")
- note: Large light streaks on a dark base read at distance; flat fade, no geometry.
## A12-02: Honeycomb knee fade
- who: Dock Worker, Miner, Refugee
- type: WEAR
- zone: knee
- prompt: Small pale cell-shaped lattice on the knee, lighter cell centres with dark edges, like crushed creases. Soft, irregular.
- palette: pale grey, dark indigo
- lore: INFERRED | https://www.heddels.com/?p=674 (read: honeycombs "behind the knees")
- note: Cells must be about palm-sized or they vanish at game distance (my estimate); no geometry.
## A12-03: Boro patch on forearm
- who: Refugee, Dock Worker, Rural Worker
- type: WEAR
- zone: forearm
- prompt: Square patch of darker cloth on the forearm, edges sewn with white running stitches in a grid, slightly uneven.
- palette: indigo, off-white thread
- lore: INFERRED | https://en.wikipedia.org/wiki/Visible_mending (read: boro was a seam-reinforcing repair by poor rural households in Edo Japan)
- note: Stitch grid reads as texture at distance only if the patch edge has strong contrast; flat paint.
## A12-04: Sashiko grid on upper back
- who: Refugee, Rural Worker
- type: PATTERN
- zone: upper_back
- prompt: Indigo cloth with a white running-stitch geometric grid across the upper back, diamonds and scale-like rows. Bold, evenly spaced.
- palette: indigo, off-white
- lore: INFERRED | https://en.wikipedia.org/wiki/Visible_mending (read: decorative sashiko uses geometric motifs such as fish scales, diamonds, mountains, bamboo, arrows)
- note: Decorative, not repair, so it works as a craft or faction marker; big motifs only, or they blur.
## A12-05: Grease streak down the chest
- who: Dock Foreman, Mechanic
- type: STAIN
- zone: chest_center
- prompt: Dark brown-black oil streak running down the front of the chest from collar to belt, uneven, with a few drips.
- palette: black-brown, dull grey
- lore: INFERRED | UNFETCHED: https://starwars.com/news/dressing-the-galaxy-designer-michael-wilkinson-defines-the-look-of-andor-s-costumes (snippet: Andor's crumpled uniform "with stains down the front")
- note: Vertical dark streak reads at distance and signals a dirty job; painted onto the existing front, no geometry.
## A12-06: Stiff cuff band
- who: Administrator, Imperial-affiliated clerk
- type: TRIM
- zone: cuff
- prompt: Narrow stiff band around the cuff, glossy and smooth, contrasting with the matte cloth. Single colour, crisp straight edges.
- palette: glossy grey, dark slate
- lore: INFERRED | https://nofilmschool.com/andor-costume-design (read: Wilkinson, "Something soft, something stiff, something that is close to the body, something that is loose")
- note: Gloss-versus-matte contrast is what reads at distance; one band, no geometry.
## A12-07: Rank pips and shoulder metal tabs
- who: Imperial-affiliated officer (INFERRED)
- type: HARDWARE
- zone: collar, shoulder_L, shoulder_R
- prompt: Small flat painted metal tabs on the collar and both shoulders, dull silver with a dark edge. Plain, no bright highlights.
- palette: dull silver, dark grey
- lore: INFERRED | https://nofilmschool.com/andor-costume-design (read: "collar pips" and "a little bit of metal at his collar and on his shoulders")
- note: Two small silver marks at the collar read as rank at distance; flat paint, no geometry.
## A12-08: Stencil name code on chest
- who: Dock Worker, Hangar Mechanic
- type: PATTERN
- zone: chest_left
- prompt: Short block-letter stencil code on the left chest, three or four capital letters, black paint, slightly broken edges.
- palette: black, faded white
- lore: INFERRED | UNFETCHED: https://intrepid.emuseum.com/objects/23684/us-navy-uniform-stencil (snippet: 1944-46 Navy name stencil cut from card stock, painted black)
- note: Only 3-4 letters stay legible at game distance; longer names blur into a grey bar.
## A12-09: Decorative iron-on patch on upper arm
- who: Refugee, Dock Worker
- type: EMBLEM
- zone: upper_arm
- prompt: Diamond-shaped patch on the upper arm in bright contrasting cloth, edges stitched in dark thread. Decorative and bold.
- palette: rust red, cream
- lore: INFERRED | https://en.wikipedia.org/wiki/Visible_mending (read: 1943 British pamphlet "Make Do and Mend" urged readers to "make your mend as decorative as possible")
- note: High-contrast diamond is a readable shape at distance; flat patch, no geometry.
## A12-10: Sun-faded shoulders
- who: Refugee, Rural Trader
- type: WEAR
- zone: shoulder_L, shoulder_R, upper_back
- prompt: Pale bleached patches across both shoulders and the top of the back, soft-edged, like sun-faded cloth. Sand and dusty grey tones.
- palette: pale sand, dusty grey
- lore: INFERRED | https://denver7.com/news/front-range/denver/15-things-we-learned-from-the-star-wars-the-power-of-costume-exhibition-at-the-denver-art-museum (read: Lucas wanted "simple but timeless" costumes with "worn out garments" in earth tones; the sun-fade detail is my inference)
- note: Shoulder fade reads as worn outdoors and fits the earth-tone look; no geometry.

## TECHNIQUES
1. Stack wear as separate overlays (grease, fade, dust) over the base colour so each NPC gets 2-3 of 5 layers.
2. Put the most varied detail on chest and shoulders (camera-facing); keep back and hem simpler. Forum recap says Unity crowd figures near camera get the full generator, far ones a 29-variant pool (UNFETCHED: https://discussions.unity.com/t/how-to-can-i-create-a-crowd-system-in-by-game-similer-to-ac-u/647400).
3. Fade lines at stress points: hip creases, knee cells, raised seam edges (denim "atari", snippet only).
4. Stitch at 2-3 px on the 1024 atlas in a contrasting thread; geometric grids read, random stitching does not (estimate).
5. Chipped-paint look: base colour, then lighter dabs at edges; chips reveal the lighter layer (fan replica builds, UNFETCHED: https://www.thedentedhelmet.com/forums/threads/staggz-din-djarin-season-1-build.36617).
6. Pair one glossy or stiff band with matte cloth per outfit (see A12-06).
7. Keep stencil text to 3-4 letters (see A12-08).
8. Limit duplicates by pool size: a mod reports duplicates falling from ~15% to ~1% after adding appearances (UNFETCHED: https://www.nexusmods.com/cyberpunk2077/mods/25762).

## SOURCES
- https://en.wikipedia.org/wiki/Visible_mending
- https://www.heddels.com/?p=674
- https://gdcvault.com/play/1022141/Massive-Crowd-on-Assassin-s
- https://wiki.redmodding.org/cyberpunk-2077-modding/modding-guides/npcs/appearances-change-the-looks
- https://nofilmschool.com/andor-costume-design
- https://denver7.com/news/front-range/denver/15-things-we-learned-from-the-star-wars-the-power-of-costume-exhibition-at-the-denver-art-museum

## GAPS
- starwars.com, Variety (402), Intrepid (403), Nexus 25762 (403) and DTIC were not readable; their claims are snippet-only.
- No official Mandalorian or Rogue One weathering source found. Porro (brief says "Porter") is credited via snippet only.
- Skyrim/EBD and Fallout 4 NPC-variation claims are from modder pages (snippet, UNFETCHED); not used on cards.
