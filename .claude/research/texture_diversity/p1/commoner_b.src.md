# Commoner looks P5-P8

Source lines for recipe writing. Female atlases n_commf*, male n_commm*. 4x4 grid cells A1 (top-left) .. D4; avoid cells are hard exclusions.

Island labels for these atlases (use in layer 'islands'): torso (cloth panels, main body), legs, torso_arm (sleeves), arms (includes hands: never paint), so for cloth use ["torso","legs"] or ["torso"]; the skin guard protects recolours only.

## Look P5: Bar patron
Cards:
- K072 (A5-10) Cantina dancer purple panel | type COLOR | zone chest_center | palette deep purple, lilac | rule cells A1,A1,A1,A1,A1,A1,A1 | blend overlay | scale 0.30 | note: Purple left-chest panel; B1 V-neck kept; sheer edge optional.
- K069 (A5-07) Sabacc card-suit pattern | type PATTERN | zone hip_pocket, thigh | palette gold, black, cream | rule cells A4,A3,C2,C2,C2,C2,A4 | blend normal | scale 0.35 | note: Dark card faces read only on light trousers.
- K065 (A5-03) Jekk'Jekk Tarr infrared-room trim | type TRIM | zone collar, cuff | palette infrared red, black, dark grey | rule cells A1-B1,A1,A1,A1,B1 | blend screen | scale 0.06 | note: Screen blend for glow; thin collar line; keep off straps.

### n_commf02
- base: cream tunic (A1-D1, dark diagonal seams at A1/B1), olive-khaki tan panel (C2-D2), khaki-olive trousers (A4-B4), brown gloves (A2-A3), rust-brown corset (B2-B3), brown boot (B3-C4)
- paint_zones: C1-D1 cream band (plain, above the seam near y 105); A1 lower-left between the seams; C2-D2 olive-tan panel (flat); A4 trouser front (plain khaki); D3 cream lower-right panel (x 430-500, y 330-450)
- avoid: dark diagonal seams A1-B1, B2-B3 corset and navy strap, A2-A3 gloves and pouch strip, C3 ochre pouch patch (x 335-440, y 330-375), D2 gold strip at right edge, boot B3-C4, D4 pouch
- skin_arms: no (cream reads skin-toned; inventory skin=0.177 is cloth; no bare skin island)
-

### n_commf07
- base: taupe-grey wrap top (A1-C1) and tunic panel (C2-D2); dark brown laced corset (B2-B3); dark leather gloves (A2-A3) and bracer; taupe trouser legs (A3-B4); dark brown boot (C3-C4); near-black belt pouch (C3-D4)
- paint_zones: A1 plain taupe chest left (view x10-80 y20-170); C1 upper-right back (view x300-500 y10-110); C2 tunic side (view x300-460 y140-230, seam at x355); A4 trouser front left (view x10-110 y380-500)
- avoid: corset B2-B3 (view x140-290 y185-340); gloves A2-A3; bracer edge A2; pouch C3 (view x335-440 y330-375); boots C3-C4; tear at A1 (view x90-115 y25-75); pouch D4; gap B3-C4
- skin_arms: no
- fit_roles: refugee, mechanic, thug
- palette_clash: near-black

### n_commm07
- base: grey-brown wrap shirt (A1-C2 front, C1-D1 back panel), grey trousers (C2-D3), slate-grey lower vest (A4-B4), dark grey-brown belt (B2-B3), dark trouser shadows at the C3/D3 seam
- paint_zones: A4+B4 slate-grey lower vest (flat, smooth, largest clean area); D1 shirt back panel (flat grey-brown); B1 wrap-front V (TRIM along the V-neck edge); D2 grey trouser top right (STAIN or wear)
- avoid: A1 dark hand blob; A2-A3 glove and bracer; B2-B3 buckled corset belt; C4 boot; D4 pouch; C3/D3 dark shadow strip
- skin_arms: no (inventory skin=0.0)
- fit_roles: refugee, farmer, mechanic
- palette_clash: grey, silver and charcoal add-ons vanish on the grey-brown shirt and grey trouser

## Look P6: Onderon citizen (palette only)
Cards:
- K074 (A6-01) Enlisted brown field uniform | type COLOR | zone whole_garment | palette brown, dark brown | rule cells C2-D2,C2-D2,C2-D2,C1-D1,C2,D1,D2,D2 | blend overlay | scale 0.40 | note: Colour-only recolour, not a uniform cut; skip tan bases.
- K076 (A6-03) Gold piping stripe | type TRIM | zone upper_arm, hem | palette gold, brown | rule cells D1+A4,D1+D3,D1+D3,D1+A4,C1+D2,C1+D2 | blend normal | scale 0.05 | note: Gold piping on dark bases only; cells approximate.
- K077 (A6-04) White wrap-style shoulder cloth | type TRIM | zone shoulder_L, shoulder_R | palette white, off-white | rule cells D1,C1-D1,C1-D1,D1,C1-D1,C1-D1 | blend normal | scale 0.20 | note: Wide white band; fold shading optional; shoulder cells approximate.

### n_commf03
- base: mid-grey ribbed tunic (A1-D2, C3-D3), brown corset with brass studs (B2-B3), dark brown gloves (A2-A3), khaki-olive trousers (A4-B4), grey leg panel (C4-D4), brown boot with orange piping (B4-C4)
- paint_zones: A1 chest panel (grey ribbed, clean); D1 upper-right band (plain grey); C2-D2 back-body panel (grey, flat); D3 lower-right grey panel (x 384-512, y 256-384); A4 trouser front (plain khaki)
- avoid: B2-B3 studded corset, A2-A3 gloves and buckled pouch strip, B4-C4 boot and orange piping (x 230-260), C3 ochre pouch (x 335-440, y 330-375), D4 pouch corner, skin sliver at A1 top-left
- skin_arms: no (skin 0.052 is the A1 pink sliver only)
- fit_roles: mechanic, guard-

### n_commf09
- base: blue-violet tunic (A1-C1 with light piping; C2-D2 side); dark brown leather corset (B2-B3) and gloves (A2-A3); blue trouser legs (A4, D3); dark brown boots (B4-C4); dark brown belt flap (C3)
- paint_zones: A1 blue chest left (view x10-150 y60-150, below collar); C1 upper-right blue panel (view x310-500 y30-110); C2 blue tunic side (view x300-460 y140-230); A4 blue trouser front (view x10-100 y380-500, seam at x100)
- avoid: corset B2-B3; gloves A2-A3; boots C3-C4; belt flap C3 (view x330-440 y330-375); collar piping (view x150-230 y0-60) and hem band (view y150-190); leather strap D2 (view x470-512 y140-300); pouch D4; tear at A1 (view x80-110 y20-75)
- skin_arms: no
-

### n_commf11
- base: A1-D1 slate-grey wrap top (cell means 0x29-0x36, dark); B1 brown V-neck undershirt; A2-B3 dark brown leather gloves, bracer and corset; A4-B4 khaki-brown trouser; C2-D2 grey back vest; C3 grey-blue arm
- paint_zones: C2+D2 grey vest (largest plain panel); D1 top-back strip; A4+B4 khaki trouser leg; A1 upper-left wrap panel away from the tear (x85-115 y25-75)
- avoid: gloves and bracer A2-A3; corset lace B2-B3; belt buckle B3 (x225-250 y335-370); boot shaft B3-B4; black gaps C3-D4; tear in A1; pouch D4
- skin_arms: no (skin=0.0)
- fit_roles: mechanic, guard-civilian, refugee
- palette_clash: black, charcoal, navy and dark brown add-ons vanish on the dark base; olive or k

## Look P7: Czerka staff
Cards:
- K042 (A3-01) Czerka chest emblem | type EMBLEM | zone chest_left | palette olive green, off-white, charcoal | rule cells A1,A1-B1,A1 | blend normal | scale 0.12 | note: Corporate badge; dark olive square, off-white mark
- K045 (A3-04) Office staff collar and cuff piping | type TRIM | zone collar, cuff | palette light grey, slate | rule cells A1-B1+D1,A1-B1+D1,A1-B1+D1,A1-B1+D1,A1-B1+D1 | blend normal | scale 0.06 | note: Thin light line; weak at distance; cuff cell unverified.
- K050 (A3-09) Staff name badge on hip | type HARDWARE | zone hip_pocket | palette white, blue, dark grey | rule cells D3,D3,D3,D2,D3 | blend normal | scale 0.10 | note: Small hip badge; hip cell unverified on atlas.

### n_commm08
- base: dark green robe (A1-D1 front and back, C2-D2 trousers), orange/grey striped sash B2-C2, brown stitched leather vest A3-B4, brown-olive C3-C4
- paint_zones: C1 back wrap (largest flat green panel), D2 trouser leg (flat green), A1-B1 left chest fold (green, avoid V-collar), A4 brown vest (stitched leather, trim lines only)
- avoid: B2-C2 orange sash, A2-B3 grey gauntlets, A2-A3 bracer strip, C3-D3 brown belt band and boot shaft, D4 rust pouch
- skin_arms: no
- fit_roles: scholar, merchant, farmer
- palette_clash: dark green, black and navy vanish; moss-olive and brown-olive blend into the robe; orange and rust trims clash with the orange sash
- confidence: 4

## Look P8: Peragus miner
Cards:
- K115 (A9-10) Peragus green recolor | type COLOR | zone whole_garment | palette forest green, peach orange | rule cells A1-D2 tunic only | blend overlay | scale 0.75 | note: Forest-green recolour; collar and cuff piping need a separate S1D.
- K049 (A3-08) Peragus fuel stain on hem | type STAIN | zone hem, knee | palette black-brown, dull amber | rule cells A4,A4,A3,C2,C2,C2,C2-D2,A4 | blend multiply | scale 0.30 | note: Hem stain pools at bottom edge; keep off boot B4-C4.
- K110 (A9-05) Asteroid dust on lower legs | type WEAR | zone shin, hem | palette dust tan, grey | rule cells A4-B4 | blend normal | scale 0.35 | note: Dust on dark lower fields only; pale dust vanishes on tan.

### n_commf01
- base: tan wrap tunic (A1-C1), tan back panels (C1-D2), tan trousers (A4-B4), tan leg panels (C3-D4), brown gloves (A2-A3), brown corset (B2-B3), brown boots (B4-C4)
- paint_zones: A1 left chest panel (plain tan wrap); C1-D1 upper back band (flat, widest); C2 back body panel; A4 trouser front (large plain tan); keep off D3-D4 gaps
- avoid: B2-B3 corset and buckles, A2-A3 gloves and pouch strip, B4-C4 boot, D4 pouch, B1 pink V-neck lining, black gaps at D3-D4 and A1 corners
- skin_arms: no (tan cloth is skin-toned; only a small pink sliver at B1 neckline)
- fit_roles: refugee, farmer, merchant
- palette_clash: tan, khaki, ochre and light brown vanish; pink or rose trim clashes

### n_commf02
- base: cream tunic (A1-D1, dark diagonal seams at A1/B1), olive-khaki tan panel (C2-D2), khaki-olive trousers (A4-B4), brown gloves (A2-A3), rust-brown corset (B2-B3), brown boot (B3-C4)
- paint_zones: C1-D1 cream band (plain, above the seam near y 105); A1 lower-left between the seams; C2-D2 olive-tan panel (flat); A4 trouser front (plain khaki); D3 cream lower-right panel (x 430-500, y 330-450)
- avoid: dark diagonal seams A1-B1, B2-B3 corset and navy strap, A2-A3 gloves and pouch strip, C3 ochre pouch patch (x 335-440, y 330-375), D2 gold strip at right edge, boot B3-C4, D4 pouch
- skin_arms: no (cream reads skin-toned; inventory skin=0.177 is cloth; no bare skin island)
-

### n_commf03
- base: mid-grey ribbed tunic (A1-D2, C3-D3), brown corset with brass studs (B2-B3), dark brown gloves (A2-A3), khaki-olive trousers (A4-B4), grey leg panel (C4-D4), brown boot with orange piping (B4-C4)
- paint_zones: A1 chest panel (grey ribbed, clean); D1 upper-right band (plain grey); C2-D2 back-body panel (grey, flat); D3 lower-right grey panel (x 384-512, y 256-384); A4 trouser front (plain khaki)
- avoid: B2-B3 studded corset, A2-A3 gloves and buckled pouch strip, B4-C4 boot and orange piping (x 230-260), C3 ochre pouch (x 335-440, y 330-375), D4 pouch corner, skin sliver at A1 top-left
- skin_arms: no (skin 0.052 is the A1 pink sliver only)
- fit_roles: mechanic, guard-
