# KOTOR 2 texture add-on catalog (research, 10-12)

Status: RESEARCH ONLY. No game file, no repo code, no live install touched. Follows `.claude/research/texture_ai_pipeline/report.md` (pipeline: deterministic stage 1, AI stage 2). This doc answers: which lore-grounded add-ons go on which characters and which base textures, and how to turn few add-ons into many distinct textures.

## 1. Bottom line
1. **81 usable add-on cards** for the 35 commoner atlases (21 tier A, 56 tier B, 4 tier C; 60 are deterministic S1D decals, 15 procedural wear, 6 HSV recolours). Only 1 tattoo survives: bare skin exists only on a few hand islands.
2. **The commoner pool supports civilians, refugees, thugs, patrons, workers, not uniformed roles.** 35 cards need uniform, armour or robe bases (Republic officer, TSF, Czerka staff, Onderon soldier, Sith, Mandalorian) and 32 need species bodies. Next unlock = annotate the uniform/armour atlases (n_czerkaoff01, pmb*/pfb* sets), same method as wave 2.
3. **Gemini concepts: 0 of 14 confirmed outright, 8 partly right, 6 unsourced, none provably wrong except the Old Sith hex.** Resolved forms in `w4/out/facts.md` (23 claims, quotes grep-checked).
4. **Source access:** Fandom HTML is Cloudflare-blocked (HTTP 402/403 for bots), but `api.php` works (`action=parse`, categories). TextExtracts is not installed. Our harvester (`tools/harvest.py`, no model tokens) pulled 320 pages; Haiku read local text only.
5. **Lore is thin for KOTOR 2 civilian dress.** Real sourced looks: Onderon uniforms, Republic cog, Mandalorian marks, Sith trooper/commando colours. Everything else is INFERRED or INVENTED and tagged. Named factions use tier A/B only; tier C is allowed for generic wear/palette.

## 2. Evidence tiers
- **A**: verbatim quote from a page we hold locally (or a verified source). **B**: reasonable extrapolation from a sourced page. **C**: our design, no source.
- Era: only KOTOR2 or TIMELESS cards are kept. Clone Wars/Imperial/Sith Eternal/New Republic items are OTHER and dropped.
- Legends vs canon is recorded in the evidence quote. Most KOTOR-era pages are Legends-tagged on Wookieepedia; that is accepted for this setting.

## 3. Resolved lore (full table: `w4/out/facts.md`)
- Republic cog = canon eight-spoked sigil ("Bendu" is the origin). Senate seal is a different emblem (cog plus triple prong). Khoonda is described "nine-spoked": paint the eight-spoke cog, unlabelled.
- Old Sith six-pointed hex: WRONG. Sith hex charm is dagger-shaped (Sith Eternal era). Use an unlabelled round red/black medallion or the dagger charm (K098).
- Czerka logo shape, "CZ-SEC", Exchange colours, Drexl symbol, TSF uniform look, Gand findsman marks: UNSOURCED. Use neutral forms (plain two-colour badge, generic unit-number stencil, dark-grey trim). Czerka motto "Anytime, anywhere... we're there." is sourced (K137).
- Blazing Claw: pirate-only, Legends. Jaig eyes: honour mark from clan chiefs, not veteran-only. Mythosaur skull: a right to wear; Legends calls it Mand'alor's symbol. Vizsla: shriek-hawk.
- Onderon: enlisted brown, officers very dark blue-grey, gold piping, white shoulder wrap; Vaklu light blue. Sith troopers silver/white, commandos scarlet. Republic soldier red and yellow armour.

## 4. Pipeline implications
- Stage 1 covers 81% of kept cards (decal, HSV, procedural wear). Stage 2 AI is needed for fewer than 10 cards (tattoo art, complex ornament); do not start the GPU work for them first.
- Zone to cell mapping is a 4x4 estimate. The mask tool (T2) must replace it before any bake: sleeve, cuff and hip cells are unverified.
- Skin fractions in `canvas_inventory.tsv` are unreliable (they count cloth); do not use them.
- Canvas inventory says 1024 px, previews are 512: gate must compare to the parent file, not to a constant (already in report section 10).
- `canvas_rule` in `catalog_final.json` is a cell-level UPPER bound (median 30 of 35 canvases per card); `canvases` is the conservative per-card agent pick. Bake from the intersection, then run gate G1-G7.

## 5. Variant system (summary; full draft `w4/out/system.md`)
8 slots (palette, emblem, trim, wear, stain, hardware, pattern, tattoo), max 3 per look, no two faction emblems together. Target about 400 baked textures (35 canvases x 11 looks + spares). Quality gate: G1 avoid cells, G2 area 3-25%, G3 palette dE>=15, G4 pair dE>=8 or IoU<=0.5, G5 legibility at 256 px, G6 blind shape read, G7 lore tiers. **The draft's bake set and archetypes cite 7 cards the fit pass deferred or dropped (K024 K043 K058 K061 K078 K132 K150) and the officer/TSF/Czerka/Onderon-soldier archetypes rely on uniform bases; treat those archetypes as blocked until uniform atlases are annotated.**

### Corrected POC set (KEEP-only cards, preferred canvases = cell rule and agent pick)
| # | Archetype | cids | preferred canvases |
|---|---|---|---|
| P1 | Telosian civilian | K033 K027 | n_commf03, n_commm11 |
| P2 | Nar Shaddaa refugee | K037 K063 K020 | n_commf01, f05, f07 |
| P3 | Exchange thug | K067 K068 K091 | n_commm05, m06, m11 |
| P4 | Serroco thug | K054 K053 K066 | n_commf02, f03, f05 |
| P5 | Bar patron | K072 K069 K065 | n_commf02, f12, n_commm07 |
| P6 | Onderon citizen/soldier (palette only) | K074 K076 K077 | n_commf03, f09, f11 (soldier needs uniform base) |
| P7 | Czerka staff | K042 K045 K050 | n_commm08 (thin: rule gives 5 canvases) |
| P8 | Peragus miner | K115 K049 K110 | n_commf01, f02, f03 |

## 6. Open items and risks
1. Czerka logo, TSF look, Exchange colours: no text source. Needs in-game screenshots (Telos Czerka offices, TSF officers, Exchange thugs) or a REA/texture look at `n_czerkaoff01` and exchange-thug textures before painting named-faction emblems.
2. Uniform/armour atlas annotation (wave 2 method) is the biggest unlock: 35 deferred cards.
3. Human tattoo coverage is 1 card; commission tier-B body-art cards or drop the slot.
4. K105, K096, K093 (Legends-only) and K017 (inferred) are held. K016 (cuff) dropped: gloves cover cuffs on most canvases.
5. Emblem art: reference images were viewed locally only (Fandom images are franchise art). Never composite them. For decals use our own simple shapes or the Commons manifest (`harvest/commons_decals.tsv`; 62 CC BY-SA 4.0, 11 CC BY-SA 3.0 share-alike, 18 PD, 10 CC0). Aurebesh fonts: only `AurekFonts/Aurebesh_Rodian` declares MIT; `ThetaApps/font-aurebesh` and `AndreLZGava/star-wars-font` declare no licence.
6. Licence tags carry to every derived texture (user decision 10-12); local-only until the prune gate passes.

## 7. Agent run notes (for tuning)
- 12 (wave 1 web) + 7 (wave 2 canvases) + 14 (wave 3 text chunks of 24 prepared; 10 skipped as low yield: strategy_items_2, symbols_2..6, telos, traditions x2, workers) + 7 (wave 3b emblem sheets) + 6 (wave 4) = 46 Haiku agents. Peaks 100-190k tokens on chunk and synthesis agents (ceiling 100k): split chunk files to <= 40 KB and the adjudicator into two next time.
- Text-chunk extraction yield was low (most pages have no visual description); the image viewers and the cell-rule pass added more than extra text agents would.
- Two agents ignored injected instruction text found on fetched pages.

## 8. Files
`catalog_final.json` (174 cards, status, fit, rule canvases), `catalog_compact.tsv`, `canvas_inventory.tsv`, `placement_census.tsv`, `w1/` (12 research files), `w2/` (7 canvas annotation files), `w3/out/`, `w3b/out/` (emblem grammar), `w4/out/` (fit, facts, system), `tools/` (harvesters, `canvas_rule.py`). Not committed: `harvest/`, `w3/chunks/`, `w3b/sheets/` (bulk page text and franchise images).


## 9. KEEP cards (81)
Columns: canvases = agent pick / rule upper bound. Tier = evidence tier + diversity score 1-5. Painting prompts are in section 10.

### Emblems and insignia (24)
| cid | name | type | who | zone | stage scale | canv | tier | source |
|---|---|---|---|---|---|---|---|---|
| K057 | Red Eclipse crescent | EMBLE | Red Eclipse Slaver | upper_back | S1D 0.30 | 6/32 | B4 | w1:A4-05 |
| K087 | Jaig eyes chest honour mark | EMBLE | Veteran Mandalorian Warrior, Battle Circle cha | chest_center, upper_ba | S1D 0.15 | 5/35 | B4 | w1:A7-03 |
| K088 | Mythosaur skull chest emblem | EMBLE | Mandalorian warrior of any clan, Mandalore the | chest_center, shoulder | S1D 0.20 | 6/32 | B4 | w1:A7-04 |
| K019 | Decorative iron-on patch on upper arm | EMBLE | Refugee, Dock Worker | upper_arm | S1D 0.14 | 3/9 | B3 | w1:A12-09 |
| K021 | Republic cog chest emblem | EMBLE | Republic Officer, Republic soldier, Republic N | chest_left | S1D 0.10 | 3/17 | B3 | w1:A1-01 |
| K025 | Scarlet medical sigil, upper chest | EMBLE | Republic Medic, Refugee relief staff, Telosian | chest_center | S1D 0.16 | 3/17 | B3 | w1:A1-05 |
| K026 | Relief armband with sigil | EMBLE | Refugee relief staff, Republic relief staff | upper_arm | S1D 0.14 | 3/9 | B3 | w1:A1-06 |
| K033 | Restoration sprout emblem | EMBLE | Restoration Technician, Telos Restoration Proj | shoulder_L | S1D 0.14 | 2/28 | B3 | w1:A2-02 |
| K042 | Czerka chest emblem | EMBLE | Czerka Security, Czerka Office Staff | chest_left | S1D 0.12 | 3/17 | B3 | w1:A3-01 |
| K054 | Serroco veteran chevrons | EMBLE | Serroco Veteran | shoulder_L | S1D 0.10 | 8/28 | B3 | w1:A4-02 |
| K059 | Zhug family crest | EMBLE | Zhug Brother | chest_center | S1D 0.15 | 6/17 | B3 | w1:A4-07 |
| K067 | Smugglers District crew patch | EMBLE | Exchange Thug; Serroco Thug; smuggler (no cens | chest_left | S1D 0.12 | 6/17 | B3 | w1:A5-05 |
| K103 | Jedi medallion bird | EMBLE | Jedi Order remnant, Telos Jedi enclave student | chest_center | S1D 0.12 | 5/17 | A3 | w1:A8-09 |
| K107 | Czerka mark on upper back | EMBLE | Czerka employee, Peragus miner | upper_back | S1D 0.08 | 5/32 | B3 | w1:A9-02 |
| K148 | Republic cog (spokes on disc) | EMBLE | Citadel Station technicians, mechanics and lab | chest_center | S1D 0.10 | 3/17 | A3 | w3:REPUB-1 |
| K171 | Blazing Claw | EMBLE | pirate or raider NPCs (chunk: "the most identi | shoulder_L | S1D 0.15 | 3/28 | A3 | w3:SYM1-2 |
| K173 | Crest of Alderaan (upside-down triangle) | EMBLE | royal-household, guard or noble style NPCs (no | chest_center | S1D 0.12 | 3/17 | A3 | w3:SYM1-4 |
| K022 | TSF shoulder shield patch | EMBLE | TSF Officer, Telos Security Force enforcer | shoulder_L | S1D 0.12 | 2/28 | C2 | w1:A1-02 |
| K034 | Dobo Brothers emporium patch | EMBLE | Shopkeeper, Dobo Brothers' Emporium staff | hip_pocket | S1D 0.10 | 2/30 | C2 | w1:A2-03 |
| K062 | Hutt cartel seal | EMBLE | Vogga Thug | hip_pocket | S1D 0.08 | 6/30 | B2 | w1:A4-10 |
| K063 | Refugee numbered sleeve tag | EMBLE | Refugee (planet 302, census Refugee:11) | shoulder_L, upper_arm | S1D 0.08 | 7/31 | B2 | w1:A5-01 |
| K098 | Dagger hex charm (Sith assassin) | EMBLE | Sith Assassin, Sith cultist (self-decoration) | chest_center | S1D 0.12 | 5/17 | B2 | w1:A8-04 |
| K137 | Czerka motto stencil | EMBLE | Czerka mercenaries and staff at Citadel Statio | upper_back | S1D 0.30 | 8/32 | A2 | w3:CZERKA-1 |
| K149 | Galactic Roundel (dashed ring) | EMBLE | same civilian Citadel Station staff as REPUB-1 | upper_back | S1D 0.15 | 5/32 | A2 | w3:REPUB-2 |

### Trim, piping, bands (15)
| cid | name | type | who | zone | stage scale | canv | tier | source |
|---|---|---|---|---|---|---|---|---|
| K065 | Jekk'Jekk Tarr infrared-room trim | TRIM | Bar Patron (Devaronian and Trandoshan red-room | collar, cuff | S1D 0.06 | 5/12 | A4 | w1:A5-03 |
| K068 | Swoop racing stripes | TRIM | Exchange Thug; Serroco Thug (swoop-rider varia | shoulder_L, shoulder_R | S1D 0.20 | 5/31 | B4 | w1:A5-06 |
| K035 | Swoop-track racing stripe | TRIM | Swoop Racer, Entertainment Module Worker | thigh | S1D 0.04 | 3/30 | B3 | w1:A2-04 |
| K038 | Black mourning armband | TRIM | Telosian, Refugee (mourning) | upper_arm | S1D 0.12 | 3/9 | C3 | w1:A2-07 |
| K076 | Gold piping stripe | TRIM | Onderonian soldier, all ranks | upper_arm, hem | S1D 0.05 | 6/31 | A3 | w1:A6-03 |
| K077 | White wrap-style shoulder cloth | TRIM | Onderonian soldier, all ranks | shoulder_L, shoulder_R | S1D 0.20 | 6/28 | A3 | w1:A6-04 |
| K082 | Iziz arch hem trim | TRIM | Iziz citizen, palace staff | hem, belt | S1D 0.08 | 5/31 | B3 | w1:A6-09 |
| K086 | Guard Captain red shoulder trim | TRIM | Mandalorian Guard Captain, Mandalorian Sergean | shoulder_L, shoulder_R | S1D 0.15 | 5/31 | B3 | w1:A7-02 |
| K106 | Czerka rank bars | TRIM | Czerka employee, Peragus miner (male models; w | chest_left | S1D 0.08 | 2/17 | B3 | w1:A9-01 |
| K045 | Office staff collar and cuff piping | TRIM | Czerka Office Staff (human, Twi'lek, Rodian, I | collar, cuff | S1D 0.06 | 5/12 | B2 | w1:A3-04 |
| K053 | Serroco cuff band | TRIM | Serroco Thug, Serroco Veteran | cuff | S1D 0.06 | 8/9 | B2 | w1:A4-01 |
| K083 | Purple silk waist sash | TRIM | Onderon noble; wealthier Iziz merchant | belt | S1D 0.15 | 2/2 | B2 | w1:A6-10 |
| K094 | Vengeance sand-gold cuff band | TRIM | Mandalorian on a revenge task | cuff, forearm | S1D 0.10 | 6/9 | A2 | w1:A7-10 |
| K166 | Light combat suit collar and shoulder tr | TRIM | Hired guns and mercenaries; drop sources Samha | collar | S1D 0.25 | 7/3 | A2 | w3:STRAT1-4 |
| K172 | Corellian bloodstripe trouser piping | TRIM | Corellian-born civilian or ex-military NPCs in | thigh | S1D 0.03 | 6/30 | A1 | w3:SYM1-3 |

### Wear and weathering (16)
| cid | name | type | who | zone | stage scale | canv | tier | source |
|---|---|---|---|---|---|---|---|---|
| K028 | Battle soot on right shoulder and chest | WEAR | TSF Officer, Republic Officer, Citadel Station | shoulder_R, chest_left | S1W 0.25 | 3/32 | B4 | w1:A1-08 |
| K011 | Hige whisker fade at hip | WEAR | Dock Worker, Refugee, Mercenary | hip_pocket, belt | S1W 0.25 | 3/31 | B3 | w1:A12-01 |
| K013 | Boro patch on forearm | WEAR | Refugee, Dock Worker, Rural Worker | forearm | S1D 0.14 | 2/9 | B3 | w1:A12-03 |
| K020 | Sun-faded shoulders | WEAR | Refugee, Rural Trader | shoulder_L, shoulder_R | S1W 0.30 | 3/32 | B3 | w1:A12-10 |
| K027 | Telos dust on shins and hem | WEAR | Refugee, Telosian Restoration worker, Republic | shin, hem | S1W 0.20 | 3/30 | B3 | w1:A1-07 |
| K036 | Bombardment soot and torn hem | WEAR | Telosian Survivor, Refugee | hem | S1W 0.25 | 3/30 | B3 | w1:A2-05 |
| K037 | Refugee knee patches | WEAR | Refugee | knee | S1D 0.14 | 3/30 | B3 | w1:A2-06 |
| K048 | Restoration Zone soot and scorch | WEAR | Czerka Guard, Czerka Mercenary (Telos Restorat | thigh, hip_pocket | S1W 0.25 | 8/30 | B3 | w1:A3-07 |
| K064 | Refugee stitched hem repair | WEAR | Refugee; Refugee Sector residents | hem, knee | S1D 0.30 | 6/30 | B3 | w1:A5-02 |
| K080 | Beast Rider wilderness wear | WEAR | Beast Rider (Onderonian exile) | whole_garment | S1W 0.60 | 6/34 | B3 | w1:A6-07 |
| K091 | Battle scorch and repaired plate | WEAR | Battle-scarred Mandalorian, Mercenary, Veteran | chest_left, shoulder_L | S1W 0.15 | 8/32 | B3 | w1:A7-07 |
| K110 | Asteroid dust on lower legs | WEAR | Peragus miner, Mining facility crew | shin, hem | S1W 0.35 | 5/30 | B3 | w1:A9-05 |
| K112 | Blast scorch | WEAR | Peragus miner (survivor), Mining facility crew | whole_garment | S1W 0.30 | 6/34 | B3 | w1:A9-07 |
| K012 | Honeycomb knee fade | WEAR | Dock Worker, Miner, Refugee | knee | S1W 0.15 | 2/30 | B2 | w1:A12-02 |
| K060 | Ubese scraped identity patch | WEAR | Ubese Bounty Hunter | chest_left | S1W 0.20 | 7/17 | B2 | w1:A4-08 |
| K111 | Homespun knee patch | WEAR | Khoonda settler, Dantooine farmer | knee | S1D 0.12 | 5/30 | B2 | w1:A9-06 |

### Stains (3)
| cid | name | type | who | zone | stage scale | canv | tier | source |
|---|---|---|---|---|---|---|---|---|
| K015 | Grease streak down the chest | STAIN | Dock Foreman, Mechanic | chest_center | S1W 0.10 | 3/17 | B3 | w1:A12-05 |
| K049 | Peragus fuel stain on hem | STAIN | Peragus Miner, Mining Facility Worker | hem, knee | S1W 0.30 | 8/30 | B3 | w1:A3-08 |
| K030 | Dark red knee stain | STAIN | Republic soldier, Citadel Station defender | knee | S1W 0.12 | 3/30 | B2 | w1:A1-10 |

### Patterns and stencils (8)
| cid | name | type | who | zone | stage scale | canv | tier | source |
|---|---|---|---|---|---|---|---|---|
| K014 | Sashiko grid on upper back | PATTE | Refugee, Rural Worker | upper_back | S1D 0.30 | 3/32 | B4 | w1:A12-04 |
| K081 | Drexl scale patch | PATTE | Beast Rider; Beast hunter (Dxun/Onderon) | chest_center | S1D 0.20 | 8/17 | B4 | w1:A6-08 |
| K018 | Stencil name code on chest | PATTE | Dock Worker, Hangar Mechanic | chest_left | S1D 0.14 | 3/17 | B3 | w1:A12-08 |
| K066 | Cargo-crate stencil code | PATTE | Exchange Thug; Serroco Thug (cargo-run crews,  | upper_back | S1D 0.25 | 8/32 | B3 | w1:A5-04 |
| K069 | Sabacc card-suit pattern | PATTE | Bar Patron (card-table patron) | hip_pocket, thigh | S1D 0.35 | 7/30 | B3 | w1:A5-07 |
| K040 | Restoration survey grid | PATTE | Restoration Technician | forearm | S1D 0.20 | 3/9 | B2 | w1:A2-09 |
| K041 | Herd tally marks for trade | PATTE | Ithorian-adjacent Merchant, Exchange Trader | hip_pocket | S1D 0.15 | 3/30 | B2 | w1:A2-10 |
| K165 | Mandalorian mesh weave | PATTE | Mandalorian fighters on Dxun (Mandalorian Ruin | whole_garment | S1D 0.50 | 7/34 | A2 | w3:STRAT1-3 |

### Flat hardware (belts, pouches, badges) (8)
| cid | name | type | who | zone | stage scale | canv | tier | source |
|---|---|---|---|---|---|---|---|---|
| K031 | TSF utility hip pouch | HARDW | TSF Officer, TSF enforcer | hip_pocket | S1D 0.12 | 2/30 | C2 | w1:A1-11 |
| K050 | Staff name badge on hip | HARDW | Czerka staff at Peragus II or Telos (miners, o | hip_pocket | S1D 0.10 | 5/30 | B2 | w1:A3-09 |
| K102 | Leather waist tie (Handmaiden) | HARDW | Handmaiden Sister (Echani), white outfit | belt | S1D 0.30 | 1/2 | B2 | w1:A8-08 |
| K109 | Charge pouches on belt | HARDW | Peragus miner, Mining facility worker | belt, hip_pocket | S1D 0.15 | 1/31 | A2 | w1:A9-04 |
| K162 | Prisoner electro-collar band | HARDW | enslaved prisoners on mining platforms (generi | collar | S1D 0.25 | 5/3 | A2 | w3:SITHJ2-3 |
| K163 | Czerka utility belt tools | HARDW | Czerka maintenance staff and Republic workers; | belt | S1D 0.30 | 0/2 | A2 | w3:STRAT1-1 |
| K164 | Telos mining safety harness | HARDW | Peragus II Mining Tunnels miners and Telos wor | chest_center | S1D 0.30 | 6/17 | A2 | w3:STRAT1-2 |
| K168 | Waist shield unit | HARDW | Telos and Nar Shaddaa workers or guards; drop  | belt | S1D 0.12 | 0/2 | A1 | w3:STRAT1-6 |

### Palette shifts (recolour) (6)
| cid | name | type | who | zone | stage scale | canv | tier | source |
|---|---|---|---|---|---|---|---|---|
| K044 | Commander green recolour | COLOR | Czerka Commander, Czerka Supervisor (non-param | whole_garment | S1H 1.00 | 4/35 | B4 | w1:A3-03 |
| K072 | Cantina dancer purple panel | COLOR | Dancer; Twi'lek Dancer | chest_center | S1H 0.30 | 7/35 | B3 | w1:A5-10 |
| K074 | Enlisted brown field uniform | COLOR | Onderonian enlisted and non-commissioned soldi | whole_garment | S1H 0.40 | 8/35 | A3 | w1:A6-01 |
| K075 | Officer deep blue-gray uniform | COLOR | Onderonian commissioned officer (source does n | whole_garment | S1H 0.40 | 8/35 | A3 | w1:A6-02 |
| K101 | Light-grey Jedi robe recolour (Handmaide | COLOR | Handmaiden (Brianna) wearing her mother's Jedi | whole_garment | S1H 0.75 | 7/35 | A3 | w1:A8-07 |
| K115 | Peragus green recolor | COLOR | Peragus miner (Czerka-style uniform) | whole_garment | S1H 0.75 | 8/35 | B3 | w1:A9-10 |

### Tattoos (bare skin) (1)
| cid | name | type | who | zone | stage scale | canv | tier | source |
|---|---|---|---|---|---|---|---|---|
| K099 | Dark side forearm tattoo | TATTO | Sith Acolyte, dark side cultist with bare fore | upper_arm, forearm | S1D 0.20 | 5/1 | B2 | w1:A8-05 |

## 10. Painting prompts (KEEP cards)
| cid | prompt | palette |
|---|---|---|
| K011 | Pale grey-blue horizontal creases across the hip and belt, soft edges, as if from years of bending and sitting. | pale grey-blue, dark indigo |
| K012 | Small pale cell-shaped lattice on the knee, lighter cell centres with dark edges, like crushed creases. Soft, irregular. | pale grey, dark indigo |
| K013 | Square patch of darker cloth on the forearm, edges sewn with white running stitches in a grid, slightly uneven. | indigo, off-white thread |
| K014 | Indigo cloth with a white running-stitch geometric grid across the upper back, diamonds and scale-like rows. Bold, evenly spaced. | indigo, off-white |
| K015 | Dark brown-black oil streak running down the front of the chest from collar to belt, uneven, with a few drips. | black-brown, dull grey |
| K018 | Short block-letter stencil code on the left chest, three or four capital letters, black paint, slightly broken edges. | black, faded white |
| K019 | Diamond-shaped patch on the upper arm in bright contrasting cloth, edges stitched in dark thread. Decorative and bold. | rust red, cream |
| K020 | Pale bleached patches across both shoulders and the top of the back, soft-edged, like sun-faded cloth. Sand and dusty grey tones. | pale sand, dusty grey |
| K021 | Small white eight-spoked cog emblem on a dark navy disc, spokes converging on a central ring. Flat, crisp edges, no gradient, no text. | white, navy |
| K022 | Dark blue shield-shaped shoulder patch with one wide gold horizontal bar across its middle. Flat, sharp outline, no text, no shading. | dark blue, gold |
| K025 | Bold scarlet medical sigil centered on the upper chest, on a plain white circle. Flat, high contrast, no shading, no text. | scarlet, white |
| K026 | Wide white armband on the left upper arm with a small scarlet sigil at its centre. Flat band, crisp edges, no fringe. | white, scarlet |
| K027 | Pale grey-tan dust film across the lower legs and hem, heavier near the boots, brushed off unevenly. Soft edges. | grey-tan, pale tan |
| K028 | Dark soot scorch marks across the right shoulder and upper chest, ragged feathered edges, a few charred streaks. Smoky black and charcoal. | black, charcoal |
| K030 | Dried dark-red stain across the left knee, small splatter drops below it, irregular edges. Matte, no gloss. | dark red, rust |
| K031 | Flat dark olive utility pouch on the hip with a brass snap flap and a painted belt loop. Matte finish, crisp outline. | olive, brass |
| K033 | Small olive-green sprout with two leaves on a dark brown patch, flat simple shapes, bold outline, about fist-sized. | olive green, dark brown |
| K034 | Rectangular rust-orange patch on a hip pocket with one bold white capital D, flat stitched look, no other detail. | rust orange, white |
| K035 | Two thin bright orange stripes running straight down the outer thigh, parallel, with a thin black line between them, like racing livery. | bright orange, black |
| K036 | Uneven charcoal-black soot smudges and scorch streaks across the lower legs, with a frayed, torn hem edge that looks burnt. | charcoal black, scorch brown |
| K037 | Two square patches of contrasting tan and grey cloth on the knee, stitched with coarse dark thread, visible mending, slightly uneven edges. | tan, grey, dark thread |
| K038 | Narrow black cloth armband around the upper arm with a single thin pale grey stripe in the middle, plain and flat, slightly worn at the edges. | black, pale grey |
| K040 | Fine thin grey grid lines in a regular square pattern across the forearm, like a survey map, with a few tiny cross marks. | pale grey, dark grey |
| K041 | Row of short dark brown tally strokes in groups of five, painted in thin ink across a pale tan hip pocket, hand-marked look. | dark brown, pale tan |
| K042 | Small simple corporate badge, a plain abstract mark in off-white outlined on a dark olive square. Flat print, no text, crisp edges. | olive green, off-white, charco |
| K044 | Recolor the whole garment a deep forest green with slightly darker shading along the seams. Keep fabric folds. No logos or text. | deep forest green, peach orang |
| K045 | Thin single-colour piping along the collar and both sleeve cuffs, in light grey. Clean office-wear finish, no pattern, no text. | light grey, slate |
| K048 | Soot-black scorch marks and smudged grime across the thigh and hip, heavier on one side. Dark smears only, no sharp edges, no text. | soot black, ash grey |
| K049 | Dark fuel stains and oily streaks across the lower hem and knee, pooling at the bottom edge. Matte black-brown, soaked look, no text. | black-brown, dull amber |
| K050 | Flat rectangular name badge clipped to the hip pocket, white plastic look with a blue top bar and a short dark text line. Printed, no clip hanging. | white, blue, dark grey |
| K053 | Narrow solid dark-grey band stitched around the cuff edge, matte fabric, plain, no lettering. | dark grey, charcoal |
| K054 | Two stacked dull grey chevrons on the left shoulder, worn stencil edges, flat print, no lettering. | grey, black |
| K057 | Large red crescent with a black disc overlapping its lower edge, bold flat shapes, centred on the back, no lettering. | red, black |
| K059 | Round family crest on the chest, bronze ring around a black diamond, clean flat shapes, no lettering. | bronze, black |
| K060 | Faded rectangular patch on the chest, edges scraped and rubbed down, a pale ghost shape where lettering was removed. | faded grey, pale tan |
| K062 | Small round dull-gold seal on the hip pocket, plain black shape at its centre, flat print, no lettering. | dull gold, black |
| K063 | A small rectangular cloth tag stitched on the upper arm, with a black serial number printed on off-white. Plain stitching, no logo. | off-white, black, rust-brown |
| K064 | Coarse dark thread stitching a torn hem closed in visible patches. Mismatched cloth patches, several stitches, frayed edge. Thread colour: dark brown. | dark brown thread, grey-brown  |
| K065 | A narrow band of glowing red trim along the collar edge, like light from a red-lit room. Solid saturated red, faint glow edge. | infrared red, black, dark grey |
| K066 | Black stencilled block letters and numbers in a column across the upper back, like cargo-crate codes. Plain rectangular stencil font, one line of characters. | stencil black, grey-tan, rust |
| K067 | A round crew patch on the chest, plain black ring with a white bar across the middle. No animals, no text. Flat, hard-edged. | black, white, dull grey |
| K068 | Two parallel racing stripes running down each sleeve, from shoulder seam to cuff. Solid bright orange stripes with a thin white edge. | racing orange, white, black |
| K069 | Repeating small playing-card face motifs in a grid across the thigh, one suit symbol per card, plain geometric shapes. Gold outline on dark card faces. | gold, black, cream |
| K072 | Recolour the chest panel to deep purple, with a sheer look at the edges. Solid purple, slightly lighter at the seam. Keep the rest of the garment unchanged. | deep purple, lilac |
| K074 | Recolor the garment flat plain brown, matte, like a field uniform. No patterns, no added markings, even colour across the whole piece. | brown, dark brown |
| K075 | Plain deep blue-gray garment, smooth fabric, even colour across the whole piece, no pattern or markings. | deep blue-gray |
| K076 | Thin gold piping stripe running along the sleeve seam and the lower hem. Single clean line, no other decoration, flat yellow-gold. | gold, brown |
| K077 | Wide white wrap-style cloth draped across each shoulder, painted as a soft band running over the shoulder seam. Plain white, slight fold shading only. | white, off-white |
| K080 | Scatter dust, dried mud, and scuffs over the garment, heaviest at the hem and knees. Faded patches where sun bleached the fabric. Keep colour changes gentle. | dusty tan, mud brown, faded gr |
| K081 | Overlapping purple scale pattern on a rectangular leather patch across the chest. Each scale a simple rounded shape, plain dark outline, flat painted look. | purple, dark purple, dark brow |
| K082 | Row of evenly spaced rounded arches along the lower hem, outlined in pale yellow, arch openings filled with blue. Flat, bold, no shading. | pale yellow, blue |
| K083 | Wide purple silk sash wrapped around the waist, smooth sheen, gentle fold shading, tied at the side. Plain silk, no embroidery, no pattern. | purple, dark violet |
| K086 | Solid crimson band across both shoulder plates and along sleeve cuffs, two fingers wide, crisp edges, flat matte paint. | crimson, black |
| K087 | Two narrow pointed eye shapes in flat bone-white paint, centred on a dark chest plate. Matte, no glow, no outline. | bone white, charcoal |
| K088 | Bold bone-white horned skull silhouette, centred on a dark chest plate, slightly chipped edges, flat paint, no shading. | bone white, dark grey |
| K091 | Black scorch blotch on the left chest plate, with a rough square grey patch riveted over a dent. Flat paint, no smoke. | scorch black, patch grey |
| K094 | Sand-gold band around both forearm cuffs, two fingers wide, with a thin black line at the inner edge. Flat matte paint. | sand gold, black |
| K098 | Small flat dagger-shaped emblem painted in black with a red inlay, centred on the upper chest. | black, red, dark grey |
| K099 | Black and dark-red angular tribal bands wrapping the forearm and upper arm on bare skin, sharp spiral lines. | black, dark red |
| K101 | Recolour the whole garment soft light grey with slight cloth wear, and a faint cream border along the hem. | light grey, cream |
| K102 | Wide brown leather tie cinched across the waist, flat painted buckle, stitched edges, on white cloth. | brown, tan, white |
| K103 | Round bronze medallion centred on the chest, bearing a stylized bird with wings spread upward, cream background. | bronze, cream, dark brown |
| K106 | Three short orange horizontal bars stacked neatly on the left chest of the green garment, flat printed, thick simple blocks. | orange, dark green |
| K107 | Small round green badge with a bold orange center bar, printed centered on upper back, no text, simple shape. | dark green, orange |
| K109 | Two small square black charge pouches with grey clasps on the belt, flat painted, slight raised shading, no text. | black, grey |
| K110 | Pale grey-tan dust and grime smudged across the lower legs and hem, heavier toward the bottom, soft gradient. | dust tan, grey |
| K111 | One square undyed cream cloth patch stitched onto the knee with brown thread, rough stitched outline, slightly darker edge. | cream, brown thread |
| K112 | Dark sooty scorch marks across the upper torso and one arm, black-brown with burnt edges, irregular and smudged. | soot black, brown |
| K115 | Darker forest-green garment colour with muted peach-orange piping at collar and cuffs, flat even tone, no pattern. | forest green, peach orange |
| K137 | Single straight line of small black block-capital stencil lettering reading ANYTIME, ANYWHERE... WE'RE THERE, slightly worn, across the upper back of a dark grey jacket. | black, dark grey (placeholder, |
| K148 | Flat painted emblem of eight thick spokes joined to one round disc, silver on dark navy, crisp edges, no background, no lettering. | silver, dark navy (design pick |
| K149 | Small flat emblem: eight-rayed star centred in a circle of dashed silver lines, navy and silver, sharp points, transparent background, no lettering. | navy, silver (design pick; chu |
| K162 | Narrow prisoner collar band, matte charcoal metal ring, slight wear at the clasp, plain, no text. | charcoal, steel grey |
| K163 | Flat painted utility belt: dark brown leather band, row of small tool pouches and loops, grey metal buckle, no shading. | dark brown, steel grey, black |
| K164 | Two flat webbing straps crossing the chest and shoulders, dark grey webbing, black buckles, one small yellow clip at centre. | dark grey, black, yellow |
| K165 | Fine diamond mesh weave over the whole garment, dark grey grid lines on charcoal, faint lighter highlights, no shading. | charcoal, dark grey |
| K166 | Contrasting dark grey trim panels on the collar and shoulder seams, reinforced black stitch lines, flat matte finish. | dark grey, black |
| K168 | Matte grey waist unit: rounded plate on a dark belt, two thin vent slots, one small blue indicator strip, flat shading. | grey, black, blue |
| K171 | Bold flat emblem of one or two claws ringed by orange-red flames, black claw silhouette, painted on a dark sleeve | black, orange, red (design cho |
| K172 | Single thin piping stripe running down the outer seam of dark trousers, flat painted, crisp edges, no other markings | yellow, red (chunk colours; re |
| K173 | Upside-down triangle of interlacing curves, gold line work on dark fabric, flat painted, symmetric, small | gold, silver (Legends page col |

## 11. Not for commoner atlases
- DEFER (35): K017, K023, K024, K043, K078, K085, K089, K090, K093, K095, K096, K097, K138, K139, K140, K141, K142, K143, K144, K145, K146, K147, K150, K151, K152, K153, K154, K155, K156, K157, K159, K160, K167, K170, K174
- SPECIES-LOCKED (32): K001, K002, K003, K004, K005, K006, K007, K008, K009, K010, K116, K117, K118, K119, K120, K121, K122, K123, K124, K125, K126, K127, K128, K129, K130, K131, K132, K133, K134, K135, K136, K169
- HOLD (1): K105
- DROP/MERGE: see `catalog_final.json` status_note.
