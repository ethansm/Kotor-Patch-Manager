# Commoner looks P1-P4

Source lines for recipe writing. Female atlases n_commf*, male n_commm*. 4x4 grid cells A1 (top-left) .. D4; avoid cells are hard exclusions.

Island labels for these atlases (use in layer 'islands'): torso (cloth panels, main body), legs, torso_arm (sleeves), arms (includes hands: never paint), so for cloth use ["torso","legs"] or ["torso"]; the skin guard protects recolours only.

## Look P1: Telosian civilian
Cards:
- K033 (A2-02) Restoration sprout emblem | type EMBLEM | zone shoulder_L | palette olive green, dark brown | rule cells A1,A1-B1 | blend normal | scale 0.14 | note: Shoulder_L mapped to A1; verify placement in-game
- K027 (A1-07) Telos dust on shins and hem | type WEAR | zone shin, hem | palette grey-tan, pale tan | rule cells A3-B4,C2-D3,C2-D2 | blend screen | scale 0.20 | note: Dust film on trouser legs; keep off boots and pouches

### n_commf03
- base: mid-grey ribbed tunic (A1-D2, C3-D3), brown corset with brass studs (B2-B3), dark brown gloves (A2-A3), khaki-olive trousers (A4-B4), grey leg panel (C4-D4), brown boot with orange piping (B4-C4)
- paint_zones: A1 chest panel (grey ribbed, clean); D1 upper-right band (plain grey); C2-D2 back-body panel (grey, flat); D3 lower-right grey panel (x 384-512, y 256-384); A4 trouser front (plain khaki)
- avoid: B2-B3 studded corset, A2-A3 gloves and buckled pouch strip, B4-C4 boot and orange piping (x 230-260), C3 ochre pouch (x 335-440, y 330-375), D4 pouch corner, skin sliver at A1 top-left
- skin_arms: no (skin 0.052 is the A1 pink sliver only)
- fit_roles: mechanic, guard-

### n_commm11
- base: worn beige wrap tunic (A1-D1, V-wrap centre fold), grey-tan trousers C2-D2 with frayed hems, tan leather corset B2-C2 with eyelets, plain grey-beige vest A3-B4, dark brown belt pouch C3 and boot C4
- paint_zones: D1 back band (flat beige, largest field), A4-B4 grey-beige vest (most uniform plain block, emblem zone), D2 trouser back (worn grey-tan), A1-B1 chest left of wrap fold
- avoid: V-wrap fold B1-C1, tan corset B2-C2 with eyelets, brown gloves A2-B3, bracer strip A2-A3, frayed hems C2-D2 bottom, C3 pouch, C4 boot, D4 small pouch
- skin_arms: no
- fit_roles: refugee, farmer, merchant
- palette_clash: beige, tan, khaki and cream vanish on the tunic and vest; grey pai

## Look P2: Nar Shaddaa refugee
Cards:
- K037 (A2-06) Refugee knee patches | type WEAR | zone knee | palette tan, grey, dark thread | rule cells C2,C2,C3 | blend normal | scale 0.14 | note: Knee patches; male cell is trouser top
- K063 (A5-01) Refugee numbered sleeve tag | type EMBLEM | zone shoulder_L, upper_arm | palette off-white, black, rust-brown | rule cells C1,C1-D1,C1,D1,D1,D1,D1 | blend normal | scale 0.08 | note: Upper-arm cell unverified; digits illegible at distance.
- K020 (A12-10) Sun-faded shoulders | type WEAR | zone shoulder_L, shoulder_R, upper_back | palette pale sand, dusty grey | rule cells D1,D1,D1 | blend screen | scale 0.30 | note: Bleached shoulders; light sand reads on dark bases only

### n_commf01
- base: tan wrap tunic (A1-C1), tan back panels (C1-D2), tan trousers (A4-B4), tan leg panels (C3-D4), brown gloves (A2-A3), brown corset (B2-B3), brown boots (B4-C4)
- paint_zones: A1 left chest panel (plain tan wrap); C1-D1 upper back band (flat, widest); C2 back body panel; A4 trouser front (large plain tan); keep off D3-D4 gaps
- avoid: B2-B3 corset and buckles, A2-A3 gloves and pouch strip, B4-C4 boot, D4 pouch, B1 pink V-neck lining, black gaps at D3-D4 and A1 corners
- skin_arms: no (tan cloth is skin-toned; only a small pink sliver at B1 neckline)
- fit_roles: refugee, farmer, merchant
- palette_clash: tan, khaki, ochre and light brown vanish; pink or rose trim clashes

### n_commf05
- base: rust-brown wrap top (A1-D1, C2-D2 back), ochre-gold corset (B2-B3), dark brown gloves (A2-A3), grey-blue vest/trousers (A4-B4), rust-brown lower panels (C3-D3), brown pouch (D4 corner)
- paint_zones: A1 chest panel (rust wrap, plain left half); C1-D1 upper band (plain rust); D2 back panel (rust, flat); A4 lower-left grey panel (plain); D3 rust lower-right panel
- avoid: B2-B3 ochre corset and studs, A2-A3 gloves and pouch strip, B1 ochre V-neck lining, C3 ochre patch (x 335-440, y 330-375), D4 bag corner (x 445-512, y 440-500), A1 top-left skin/strap sliver
- skin_arms: no (skin 0.106 is rust cloth; no bare skin)
- fit_roles: farmer, refugee, merchant
- palette_clash: r

### n_commf07
- base: taupe-grey wrap top (A1-C1) and tunic panel (C2-D2); dark brown laced corset (B2-B3); dark leather gloves (A2-A3) and bracer; taupe trouser legs (A3-B4); dark brown boot (C3-C4); near-black belt pouch (C3-D4)
- paint_zones: A1 plain taupe chest left (view x10-80 y20-170); C1 upper-right back (view x300-500 y10-110); C2 tunic side (view x300-460 y140-230, seam at x355); A4 trouser front left (view x10-110 y380-500)
- avoid: corset B2-B3 (view x140-290 y185-340); gloves A2-A3; bracer edge A2; pouch C3 (view x335-440 y330-375); boots C3-C4; tear at A1 (view x90-115 y25-75); pouch D4; gap B3-C4
- skin_arms: no
- fit_roles: refugee, mechanic, thug
- palette_clash: near-black

## Look P3: Exchange thug
Cards:
- K067 (A5-05) Smugglers District crew patch | type EMBLEM | zone chest_left | palette black, white, dull grey | rule cells A1,A1,A1,A1,B1,A1 | blend normal | scale 0.12 | note: Black ring reads on light chest panels only.
- K068 (A5-06) Swoop racing stripes | type TRIM | zone shoulder_L, shoulder_R, upper_arm | palette racing orange, white, black | rule cells D1,D1,D1,D1,D1 | blend normal | scale 0.20 | note: Sleeve cells approximate; check stripe runs shoulder to cuff.
- K091 (A7-07) Battle scorch and repaired plate | type WEAR | zone chest_left, shoulder_L | palette scorch black, patch grey | rule cells A1 | blend multiply | scale 0.15 | note: Scorch only; drop the riveted plate; keep off the A1 tear.

### n_commm05
- base: red robe front (A1-B2, heavy folds), red back panel (C1-D1), dark green-grey ribbed band (B2-B3), red trouser legs (C2-D3), beige-cream lower vest (A4-B4), ochre-olive cloth (C4)
- paint_zones: A4+B4 beige lower vest (flat, wrinkled; best with dark ink); C1+D1 red back panel (plain, mild folds); C2 red trouser leg (flat inner area); A1 lower-left red robe front (between hand blob and V)
- avoid: B2-B3 ribbed band; A2-A3 brown bracer and black glove/pouch; C3/D3 seam gold oval pouch; C4 ochre cloth; D4 cream shin shape and red pouch; D2-D3 ochre edge strip
- skin_arms: no
- fit_roles: scholar, merchant, entertainer
- palette_clash: crimson and dark red vanish on the robe;

### n_commm06
- base: charcoal jacket (A1-C2 front, C1-D1 back), charcoal trouser legs (C2-D3), pink-salmon ruched sash (B2), navy striped band (B3), tan-khaki vest (A4-B4), navy trouser legs (C3-C4)
- paint_zones: A4+B4 tan vest (flat, wrinkled; best with dark or navy ink); D1 charcoal back panel upper right (flat dark, light ink reads); D2 charcoal trouser panel (flat, light trim or patch); A1 charcoal front below the V trim (small)
- avoid: B2 pink ruched sash; B3 navy striped band; A2-A3 black pouch and bracer strap; V-neck trim (red-maroon and navy) at B1-C1; C3/D3 light-blue pouch; C3-D4 red-brown patch and red-brown pouch; right-edge blue-grey bracer strip
- skin_arms: no
- fit_roles: m

### n_commm11
- base: worn beige wrap tunic (A1-D1, V-wrap centre fold), grey-tan trousers C2-D2 with frayed hems, tan leather corset B2-C2 with eyelets, plain grey-beige vest A3-B4, dark brown belt pouch C3 and boot C4
- paint_zones: D1 back band (flat beige, largest field), A4-B4 grey-beige vest (most uniform plain block, emblem zone), D2 trouser back (worn grey-tan), A1-B1 chest left of wrap fold
- avoid: V-wrap fold B1-C1, tan corset B2-C2 with eyelets, brown gloves A2-B3, bracer strip A2-A3, frayed hems C2-D2 bottom, C3 pouch, C4 boot, D4 small pouch
- skin_arms: no
- fit_roles: refugee, farmer, merchant
- palette_clash: beige, tan, khaki and cream vanish on the tunic and vest; grey pai

## Look P4: Serroco thug
Cards:
- K054 (A4-02) Serroco veteran chevrons | type EMBLEM | zone shoulder_L | palette grey, black | rule cells C1-D1,C1,C1-D1,D1,D1,D1,D1,D1 | blend normal | scale 0.10 | note: Dull chevrons read weakly; shoulder mapped to back band.
- K053 (A4-01) Serroco cuff band | type TRIM | zone cuff | palette dark grey, charcoal | rule cells D1,D1,D1,D1,D1,D1,D1,D1 | blend normal | scale 0.06 | note: Dark cuff band; sleeve cell D1 assumed, unverified.
- K066 (A5-04) Cargo-crate stencil code | type PATTERN | zone upper_back | palette stencil black, grey-tan, rust | rule cells C1-D1,C1-D1,C2-D2,C1-D1,D1,D1,D1,D1 | blend normal | scale 0.25 | note: Stencil blurs at distance; light and mid bases only.

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

### n_commf05
- base: rust-brown wrap top (A1-D1, C2-D2 back), ochre-gold corset (B2-B3), dark brown gloves (A2-A3), grey-blue vest/trousers (A4-B4), rust-brown lower panels (C3-D3), brown pouch (D4 corner)
- paint_zones: A1 chest panel (rust wrap, plain left half); C1-D1 upper band (plain rust); D2 back panel (rust, flat); A4 lower-left grey panel (plain); D3 rust lower-right panel
- avoid: B2-B3 ochre corset and studs, A2-A3 gloves and pouch strip, B1 ochre V-neck lining, C3 ochre patch (x 335-440, y 330-375), D4 bag corner (x 445-512, y 440-500), A1 top-left skin/strap sliver
- skin_arms: no (skin 0.106 is rust cloth; no bare skin)
- fit_roles: farmer, refugee, merchant
- palette_clash: r
