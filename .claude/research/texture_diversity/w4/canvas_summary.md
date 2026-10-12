# Canvas summary (W2, 35 commoner outfit atlases, 4x4 grid cells A1..D4, 256px at 1024)

## from w2/G1.md
## LAYOUT (female; shared by ref_F and all n_commf*)
- A1+B1+C1: tunic front wrap, V-neck at B1 (A1 top-left skin/strap sliver: avoid)
- C1+D1, C2+D2: tunic back and shoulder/back-body panels (widest plain cloth)
- A2+A3: dark leather gloves (centre) and ammo pouch strip (left edge): avoid
- B2+B3: corset/belt with lacing and studs, navy strap under it at B3: avoid
- A4+B4: trousers/skirt front (large plain field); C3+D3: sleeve/arm with belt pouch strip at y 330-375: avoid
- C4+D4: leg panels; boot across B4-C4 and pouch in D4 corner: avoid
- Unused UV: no cell is mostly black. D3/D4 hold the only real gaps (41-46% black, commf01 only); commf02/03 are fully padded (0% black)


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
### n_commf04
- base: dark teal jacket (A1-D1 with ribbed bands), olive-grey back panel (C2-D2), dark brown gloves (A2-A3), mauve-grey trousers (A4-B4), brown leather panel (B2-B3), dark-green lower jacket (C3-D4), brown boots (B4-C4)
- paint_zones: D2 olive-grey back panel (plain, widest); C2 grey-green side panel; A4 mauve trouser front (plain, large); D3 dark-green lower panel; keep off the ribbed bands
- avoid: B1-C1 clasp and strap rows (x 110-295, y 110-160), C1-D1 ribbed bands, A2-A3 gloves and pouch strip, B2-B3 leather panel and navy strap, C3 buckle strip (x 330-440, y 325-375), boots B4-C4, D4 pouch
- skin_arms: no (no bare skin island; skin 0.084 is dark cloth)
- fit_roles: merch
### n_commf05
- base: rust-brown wrap top (A1-D1, C2-D2 back), ochre-gold corset (B2-B3), dark brown gloves (A2-A3), grey-blue vest/trousers (A4-B4), rust-brown lower panels (C3-D3), brown pouch (D4 corner)
- paint_zones: A1 chest panel (rust wrap, plain left half); C1-D1 upper band (plain rust); D2 back panel (rust, flat); A4 lower-left grey panel (plain); D3 rust lower-right panel
- avoid: B2-B3 ochre corset and studs, A2-A3 gloves and pouch strip, B1 ochre V-neck lining, C3 ochre patch (x 335-440, y 330-375), D4 bag corner (x 445-512, y 440-500), A1 top-left skin/strap sliver
- skin_arms: no (skin 0.106 is rust cloth; no bare skin)
- fit_roles: farmer, refugee, merchant
- palette_clash: r
## from w2/G2.md
## LAYOUT (female, shared by all n_commf; view = 512 px, 1 cell = 128 view px = 256 atlas px)
- A1-C1 row 1 + upper row 2 (view y0-185) = torso top, V-neck wrap; D1 = upper-right back/sleeve panel
- A2-A3 (+B2 edge) = leather gloves (view x35-145 y205-355); column-A edge strip (view x0-45 y185-355) = bracer or wrist strap
- B2-B3 (+C2 edge) = waist: laced corset/belt (F07-F10) or red skirt (F06)
- C2-D2 (view x290-480 y130-330) = tunic/skirt side panel; D2 edge = sleeve strap
- A3-A4 + B4 = trouser legs (bottom-left, large, plain)
- B4-C4 = boot/shoe (view x215-400 y340-512); C3 = belt pouch flap (view x335-440 y330-375)
- D4 = pouch (view x460-505 y440-500) plus leftover cloth; no whole cell is black
- Unused: B3-C4 seam gap (view x215-270 y370-470, dark in F07/F10): never paint. Dark mark at A1 (view x80-110 y20-75) is fixed in all five


### n_commf06
- base: light grey torso top (A1-C1, D1); blue-slate tunic (C2-D2); red skirt (B2-B3); blue-slate trouser legs (A3-B4); pale grey lower side (C3-D3); brown leather gloves; brown boots
- paint_zones: A1 plain grey chest left (view x0-125 y30-170); C1 grey upper-right back (view x300-512 y0-120); C2 blue tunic upper (view x300-460 y140-260); B2 red skirt centre (view x150-280 y200-320); A4 trouser front left (view x0-70 y400-500)
- avoid: gloves A2-A3; wrist strap (view x0-45 y185-355); black belt pouch C3 (view x335-440 y330-370); blue button tab (view x330-345 y280-325); boots B4-C4; pouch D4; pocket A4 (view x75-130 y420-500)
- skin_arms: no
- fit_roles: merchant, scholar, ent
### n_commf07
- base: taupe-grey wrap top (A1-C1) and tunic panel (C2-D2); dark brown laced corset (B2-B3); dark leather gloves (A2-A3) and bracer; taupe trouser legs (A3-B4); dark brown boot (C3-C4); near-black belt pouch (C3-D4)
- paint_zones: A1 plain taupe chest left (view x10-80 y20-170); C1 upper-right back (view x300-500 y10-110); C2 tunic side (view x300-460 y140-230, seam at x355); A4 trouser front left (view x10-110 y380-500)
- avoid: corset B2-B3 (view x140-290 y185-340); gloves A2-A3; bracer edge A2; pouch C3 (view x335-440 y330-375); boots C3-C4; tear at A1 (view x90-115 y25-75); pouch D4; gap B3-C4
- skin_arms: no
- fit_roles: refugee, mechanic, thug
- palette_clash: near-black
### n_commf08
- base: olive corduroy tunic (A1-D2, C2-D2 side); rust-red leather corset with gold accents (B2-B3); dark brown gloves; pale grey-khaki trouser legs (A3-A4); olive trouser leg (C3-D3); mauve-grey pouch (C3); rust boot (B4-C4)
- paint_zones: A1 olive chest left (view x0-120 y20-170, avoid mark); D1 olive top-right back (view x300-500 y10-110); C2 olive tunic side (view x300-460 y140-230); A4 pale grey trouser front (view x10-120 y390-500); D3 olive trouser leg (view x400-500 y380-460)
- avoid: corset B2-B3 (view x140-295 y190-340, rivets and straps); gloves A2-A3; wrist strap A-edge (view x0-45 y185-355); mauve pouch C3 (view x335-440 y330-375); rust boot B4-C4 (view x215-400 y4
### n_commf09
- base: blue-violet tunic (A1-C1 with light piping; C2-D2 side); dark brown leather corset (B2-B3) and gloves (A2-A3); blue trouser legs (A4, D3); dark brown boots (B4-C4); dark brown belt flap (C3)
- paint_zones: A1 blue chest left (view x10-150 y60-150, below collar); C1 upper-right blue panel (view x310-500 y30-110); C2 blue tunic side (view x300-460 y140-230); A4 blue trouser front (view x10-100 y380-500, seam at x100)
- avoid: corset B2-B3; gloves A2-A3; boots C3-C4; belt flap C3 (view x330-440 y330-375); collar piping (view x150-230 y0-60) and hem band (view y150-190); leather strap D2 (view x470-512 y140-300); pouch D4; tear at A1 (view x80-110 y20-75)
- skin_arms: no
- 
### n_commf10
- base: dark brown tunic (A1-D2); orange V-straps (B1-C2 diagonal) and orange-framed tag (view x230-255 y40-70); brown leather corset with orange trim (B2-B3); dark brown gloves; khaki-olive trouser legs (A3-A4) with orange strap (view x100-125 y355-512); brown boot (B4-C4); ochre pouch (C3)
- paint_zones: C1 plain dark brown upper-right (view x330-500 y20-110); D2 tunic side (view x380-460 y140-240, avoid pale strip x470-512); A3 khaki trouser front (view x10-95 y370-500); A1 lower dark cloth (view x0-120 y130-170, below V-strap)
- avoid: orange V-strap and tag (view x140-285 y0-190); orange trouser strap (view x100-125 y355-512); corset B2-B3; gloves A2-A3; boots B4-C4; ochre
## from w2/G3.md
## LAYOUT (female, shared by ref_F REF-F01..F11; 4x4 grid; px coords below are on the 512 files, cell = 128 px)
- A1+B1 = wrap-top front (V-neck body); C1+D1 = top back and sleeve strip
- A2+A3 = fingerless gloves and bracer strip (left edge x0-45); B2+B3 = corset or skirt, lace/eyelets on corset
- C2+D2 = back vest or jacket back (largest plain panel); D2 right edge = thin strap
- A4+B4 = trouser or skirt leg (lower-left, large plain panel); B3/B4 edge = boot shaft
- C3+C4 = arm or boot top, C4 lower = shoe sole; D3+D4 = boot piece and small pouch (x465-505 y440-500)
- Mostly black/empty: D4 (pouch only), D3 and C4 lower half on commf11 only (41-54% near-black); commf12-15 have no cell above 10% black

## Per canvas


### n_commf11
- base: A1-D1 slate-grey wrap top (cell means 0x29-0x36, dark); B1 brown V-neck undershirt; A2-B3 dark brown leather gloves, bracer and corset; A4-B4 khaki-brown trouser; C2-D2 grey back vest; C3 grey-blue arm
- paint_zones: C2+D2 grey vest (largest plain panel); D1 top-back strip; A4+B4 khaki trouser leg; A1 upper-left wrap panel away from the tear (x85-115 y25-75)
- avoid: gloves and bracer A2-A3; corset lace B2-B3; belt buckle B3 (x225-250 y335-370); boot shaft B3-B4; black gaps C3-D4; tear in A1; pouch D4
- skin_arms: no (skin=0.0)
- fit_roles: mechanic, guard-civilian, refugee
- palette_clash: black, charcoal, navy and dark brown add-ons vanish on the dark base; olive or k
### n_commf12
- base: A1-D1 dark green ribbed jacket (means 0x22-0x2c); B1-B2 brown belt straps with brass studs; A2-A3 orange-tan hand island with bracer at left edge; B2-B3 rust-brown skirt with navy hem band (inventory says olive, measured rust); C2-D2 sage-grey back panel; A4-B4 lilac-grey top or trouser; C3-D3 dark green trouser with brown belt; C4 brown shoe
- paint_zones: C2+D2 sage-grey back panel (largest plain area); A4+B4 lilac-grey panel; B2 upper rust skirt (x135-295 y190-260); D1 dark green top-back; D3 green trouser side
- avoid: brass-stud straps B1-B2 (y110-160); hand A2-A3 unless tattoo; bracer x0-45; navy hem band B3 (y320-345); belt C3 (x330-440 y325-360); boot shaft and 
### n_commf13
- base: A1-B1 pale grey V-neck top with white piping and olive/red stain (x55-115 y0-80); C1-D1 pale grey back top; A2-A3 bare-skin-looking hand island with bracer; B2-B3 bright red skirt; C2-D2 slate-blue shirt with white stitching; A4-B4 blue-grey denim with pockets; C3 light grey arm
- paint_zones: C1+D1 pale grey back top (light, plain); C2+D2 slate shirt mid-panel (avoid stitching); A4+B4 denim leg (large plain panel); B1 plain grey below the V-neck
- avoid: white piping along the V-neck; stain in A1; hand A2-A3; red skirt B2-B3 (saturated, clashes); black hip pouch C3 (x335-440 y330-375); two buttons C3 (x335-345 y285-325); boot shaft and shoe B3-C4
- skin_arms: partial (
### n_commf14
- base: A1-D1 olive corduroy tunic (means 0x56-0x5c) with dark red stain (x85-110 y20-75); B2-B3 brown leather corset with eyelet column (x195-225 y195-330); A2-A3 brown hand island; C2-D2 pale olive-grey vest; A4-B4 pale khaki-grey trouser; C4 brown boot, D4 dark olive trouser
- paint_zones: D1 olive top-back panel (plain); C2+D2 pale olive-grey vest (large plain panel); A4+B4 pale khaki trouser (light, plain); D3 olive edge panel
- avoid: red stain in A1; corset eyelets B2-B3; hand A2-A3; pale panel at x395-445 y375-475 (C4/D3 edge); boot shaft and shoe B3-C4
- skin_arms: partial (A2-A3 hand island, skin=0.122; which pixels are skin is unverified)
- fit_roles: farmer, guard-c
### n_commf15
- base: A1-D1 dark green ribbed jacket (means 0x22-0x2a); B1-B2 brown belt straps with brass studs; A2-A3 dark brown glossy leather glove; B2-B3 rust-brown skirt or corset; C2-D2 sage-grey back panel; A4-B4 lilac-grey trouser; C3-D3 dark green trouser with brown belt; C4 brown boot, D4 dark green with pouch
- paint_zones: C2+D2 sage-grey back panel (largest plain area); A4+B4 lilac-grey panel; D1 dark green top-back; B2 upper rust skirt (x135-295 y190-260); D3 green trouser side
- avoid: brass-stud straps B1-B2 (y110-160); glove A2-A3; bracer x0-45; belt C3 (x330-440 y325-360); boot shaft and shoe B3-C4; pouch D4
- skin_arms: no (dark leather glove reads as glove; skin=0.103 un
## from w2/G4.md


### n_commf16
- base: near-black grey-brown tunic (A1-D2), olive-khaki trousers (A3-B4), dark brown leather corset (B2-C3) and gauntlets (A2-B3)
- paint_zones: D1 dark tunic right upper (plain, large); A4 olive trouser left (plain); B4 olive trouser centre (below corset line); D2 grey-green side panel (narrow)
- avoid: orange V-straps (B1-C2), corset B2-C3, gauntlets A2-B3, orange stripe (A3-A4, x 100-125), olive belt C3, pouch D4
- skin_arms: partial (tiny pink sliver at A1 top edge, not usable)
- fit_roles: mechanic, thug, refugee
- palette_clash: black, dark brown and dark grey vanish on the tunic; orange and rust merge with the orange straps; olive and khaki vanish on the olive trousers

### n_commf33
- base: cream belted dress (A2-A3 and D2-D3 panels), belt across row 2, maroon-brown leather panel (B2-C3), cream fur strip (B3-C4), teal collar accents (A1, D1)
- paint_zones: A3 skirt front (plain cream, lower-left); D3 skirt right (plain cream); A2 bodice above belt (left); D2 bodice above belt (right)
- avoid: belt row (y 185-215 across row 2), maroon leather B2-C3, fur strip B3-C4, teal collars A1/D1, brown pouch A4, dark stain blobs B1-C1
- skin_arms: no (cream dress reads as cloth; pipeline skin=0.615 is likely the cream)
- fit_roles: merchant, scholar, farmer
- palette_clash: cream, ivory and pale yellow emblems vanish on cream panels; teal and blue accents clash with t
### n_commf35
- base: dirty cream-tan wrap top (A1-C1), cream trousers (C2-D3), cream tabard (A3-B4), brown lace corset (B2-C3), brown strap edge (A2-A3)
- paint_zones: D1 tunic back hem band (plain cream, upper-right); C2 trouser leg (plain, centre-right); A4 tabard left half (plain cream); A1 left wrap panel (left of the top V seam)
- avoid: bare hand A2-B3 (tattoo only), corset B2-C3, brown strap A2-A3 and right edge D2-D3, belt C3, boot C4, top V seam
- skin_arms: partial (bare hand A2-B3 is a usable island; small pink sliver at A1 top edge)
- fit_roles: farmer, refugee, merchant
- palette_clash: cream, bone and ivory emblems vanish; khaki and tan stains blend into the cream; pink tattoo
### n_commm01
- base: tan wrap shirt (A1-C1), tan trousers (C1-D3), tan tabard (A3-B4), brown leather lace belt (B2-C3), dark gauntlets (A2-B3), brown boot (C4)
- paint_zones: D1 tunic back hem band (plain tan, upper-right); C2 trouser leg front (plain, creased); A4 tabard left half (plain tan); D3 trouser lower-right (plain cloth)
- avoid: dark grey-green patch A1 (x 50-110, y 0-90), gauntlets A2-B3, lace belt B2-C3, side strap A2-A3, boot strip C4 and B3-C4, leather edge D2-D3, black D4, V-neck fold (B1-B2)
- skin_arms: no (no bare skin visible; pipeline skin=0.61 counts tan cloth)
- fit_roles: farmer, merchant, refugee
- palette_clash: tan, khaki and ochre emblems or trims vanish on the t
### n_commm02
- base: pale grey-white wrap shirt (A1-C1), grey-white back band (D1), grey trousers (C2-D3), blue-grey side panel (D2-D3 edge), pale grey vest (A3-B4), dark teal gauntlets (A2-B3)
- paint_zones: A4 pale vest left half (plain); D1 back band above copper seam (y 0-100); C2 grey trouser between seams; A1 left wrap panel (grey, avoid the top-left teal patch)
- avoid: copper V piping (B1-B2), copper seam C1-D1 (y ~100-125), orange band C3-D3 (y 300-335), blue-grey panel D2-D3 edge, teal gauntlets A2-B3, lace belt B2-C3, blue-piped boot C4, pouch D4
- skin_arms: no (no bare skin visible; pipeline skin=0.324 counts pale grey)
- fit_roles: mechanic, guard-civilian, merchant
- palette_
## from w2/G5.md
## LAYOUT (male; ref_M.png tiles M01-M11 and the 5 canvases agree)
- Row 1: A1 tunic front + dark hand blob | B1 tunic V-neck | C1 front edge + shirt back | D1 shirt back/sleeve panel (flat)
- Row 2: A2 glove + bracer (avoid) | B2 waist band (belt, sash or ribbed band, varies) | C2 trouser leg top | D2 trouser leg top
- Row 3: A3 glove/bracer lower (avoid) | B3 waist band lower | C3 trouser leg (pouch at C3/D3 seam) | D3 trouser leg, side panel
- Row 4: A4 lower vest/tunic (flat, large) | B4 lower vest (flat, large) | C4 boot/shin (avoid) | D4 pouch and gear (avoid)
- Mostly black/empty cells: none. Black gaps only at the D4 corner and between the boots; never paint gaps.


### commm03
- base: rust-red tunic and trousers (A1-D3), purple-grey vest (A4-B4), red trouser legs (C3-C4), dark teal glove (A2-A3)
- paint_zones: A4+B4 purple-grey vest (flat, largest clean area); C2 red shirt-side panel (plain cloth); D1 red back panel between the top hardware and the belt stripe (small); B1 red tunic left of the lace
- avoid: B1/C1 lace eyelets; A1 hand blob; A2-A3 glove and bracer; B2-B3 brown corset belt; D2-D3 blue tech panel; C3/D3 dark pouch; D4 grey boot and gear; C3-C4 rust-orange trim
- skin_arms: no (inventory skin=0.466 is a false positive on rust-red cloth)
- fit_roles: mechanic, thug, guard-civilian
- palette_clash: red, maroon and brick vanish on the rust cl
### commm04
- base: olive-green coat (A1-D1 upper), blue-grey panel (B2-B3), olive trouser legs (C2-D3 edge), khaki-tan lower vest (A4-B4), black boots (C3-C4)
- paint_zones: A4+B4 khaki-tan vest (flat, light; best with dark ink); C1+D1 olive back panel above the brown trim (flat corduroy); B2-B3 slate blue-grey panel (flat, skip the tag); A1 lower-left olive front
- avoid: A2-A3 black pouch cluster and bracer stack; B2 small black tag; C3-C4 black boot shaft and foot; C3/D3 dark brown shoe sole; D4 pouch; brown trim line across C1-D1
- skin_arms: no
- fit_roles: guard-civilian, mechanic, merchant
- palette_clash: moss and olive vanish on the olive coat; navy and slate blue vanish on B2-B3; 
### commm05
- base: red robe front (A1-B2, heavy folds), red back panel (C1-D1), dark green-grey ribbed band (B2-B3), red trouser legs (C2-D3), beige-cream lower vest (A4-B4), ochre-olive cloth (C4)
- paint_zones: A4+B4 beige lower vest (flat, wrinkled; best with dark ink); C1+D1 red back panel (plain, mild folds); C2 red trouser leg (flat inner area); A1 lower-left red robe front (between hand blob and V)
- avoid: B2-B3 ribbed band; A2-A3 brown bracer and black glove/pouch; C3/D3 seam gold oval pouch; C4 ochre cloth; D4 cream shin shape and red pouch; D2-D3 ochre edge strip
- skin_arms: no
- fit_roles: scholar, merchant, entertainer
- palette_clash: crimson and dark red vanish on the robe; 
### commm06
- base: charcoal jacket (A1-C2 front, C1-D1 back), charcoal trouser legs (C2-D3), pink-salmon ruched sash (B2), navy striped band (B3), tan-khaki vest (A4-B4), navy trouser legs (C3-C4)
- paint_zones: A4+B4 tan vest (flat, wrinkled; best with dark or navy ink); D1 charcoal back panel upper right (flat dark, light ink reads); D2 charcoal trouser panel (flat, light trim or patch); A1 charcoal front below the V trim (small)
- avoid: B2 pink ruched sash; B3 navy striped band; A2-A3 black pouch and bracer strap; V-neck trim (red-maroon and navy) at B1-C1; C3/D3 light-blue pouch; C3-D4 red-brown patch and red-brown pouch; right-edge blue-grey bracer strip
- skin_arms: no
- fit_roles: m
### commm07
- base: grey-brown wrap shirt (A1-C2 front, C1-D1 back panel), grey trousers (C2-D3), slate-grey lower vest (A4-B4), dark grey-brown belt (B2-B3), dark trouser shadows at the C3/D3 seam
- paint_zones: A4+B4 slate-grey lower vest (flat, smooth, largest clean area); D1 shirt back panel (flat grey-brown); B1 wrap-front V (TRIM along the V-neck edge); D2 grey trouser top right (STAIN or wear)
- avoid: A1 dark hand blob; A2-A3 glove and bracer; B2-B3 buckled corset belt; C4 boot; D4 pouch; C3/D3 dark shadow strip
- skin_arms: no (inventory skin=0.0)
- fit_roles: refugee, farmer, mechanic
- palette_clash: grey, silver and charcoal add-ons vanish on the grey-brown shirt and grey trouser
## from w2/G6.md


### n_commm08
- base: dark green robe (A1-D1 front and back, C2-D2 trousers), orange/grey striped sash B2-C2, brown stitched leather vest A3-B4, brown-olive C3-C4
- paint_zones: C1 back wrap (largest flat green panel), D2 trouser leg (flat green), A1-B1 left chest fold (green, avoid V-collar), A4 brown vest (stitched leather, trim lines only)
- avoid: B2-C2 orange sash, A2-B3 grey gauntlets, A2-A3 bracer strip, C3-D3 brown belt band and boot shaft, D4 rust pouch
- skin_arms: no
- fit_roles: scholar, merchant, farmer
- palette_clash: dark green, black and navy vanish; moss-olive and brown-olive blend into the robe; orange and rust trims clash with the orange sash
- confidence: 4
### n_commm09
- base: light-blue tunic (A1-D1 front and back wrap, white piping), light-blue trousers C2-D2 and leg wraps C3-D3, black leather corset B2-C2, dark gloves A2-B3, beige-tan vest A3-B4
- paint_zones: B1 tunic chest (plain blue between collar and piping), D1 back wrap band (large flat light-blue flap), D2 trouser back (plain light blue), A4 beige vest (plain tan, left of stripe)
- avoid: V-collar and white piping B1-C1, black corset B2-C2, gloves A2-B3, brown belt band C3 and boot C4, dark-teal vest stripe A3-A4, strap D2-D3 edge
- skin_arms: no
- fit_roles: merchant, farmer, guard-civilian
- palette_clash: pale blue, white and cyan vanish on the tunic; teal collar panel clashes; 
### n_commm10
- base: dark-brown tunic (A1-D1, orange V-stripe B1-C1), dark-brown trouser legs C2-D2, brown leather corset B2-C2 with rivets, olive-yellow vest A3-B4 with orange/black centre stripe, olive leg wraps and belt band C3-D3
- paint_zones: D1 back wrap band (flat dark brown, top right), D2 trouser back (plain dark brown), A4 olive vest left of stripe (plain olive-yellow), A1-B1 chest left of V-stripe (avoid top-left collar blob)
- avoid: orange V-stripe B1-C1, existing chest badge at B1, corset B2-C2, gloves A2-B3, bracer strip A2-A3, D4 satchel
- skin_arms: no
- fit_roles: mechanic, farmer, refugee
- palette_clash: dark brown and black vanish on the tunic; orange and rust clash wi
### n_commm11
- base: worn beige wrap tunic (A1-D1, V-wrap centre fold), grey-tan trousers C2-D2 with frayed hems, tan leather corset B2-C2 with eyelets, plain grey-beige vest A3-B4, dark brown belt pouch C3 and boot C4
- paint_zones: D1 back band (flat beige, largest field), A4-B4 grey-beige vest (most uniform plain block, emblem zone), D2 trouser back (worn grey-tan), A1-B1 chest left of wrap fold
- avoid: V-wrap fold B1-C1, tan corset B2-C2 with eyelets, brown gloves A2-B3, bracer strip A2-A3, frayed hems C2-D2 bottom, C3 pouch, C4 boot, D4 small pouch
- skin_arms: no
- fit_roles: refugee, farmer, merchant
- palette_clash: beige, tan, khaki and cream vanish on the tunic and vest; grey pai
### n_commm12
- base: rust-red wrap coat (A1-C1 front, C1-D1 back), rust-red trousers C2-D2 with indigo trim at bottom, rust-red leg wraps C3-D3, grey-brown leather corset B2-C2, purple-grey vest A3-B4, blue-grey patterned strip D2-D3 edge
- paint_zones: D1 back band (flat rust, below clasps), A4-B4 purple-grey vest (plain, large), A2-A3 bare hand and A1 forearm (tattoo), C3 leg wrap (flat rust)
- avoid: hook-and-eye line B1-C1, two clasps at C1-D1 top edge, grey-white piping B1-D1, corset B2-C2, D2 indigo trim, D2-D3 patterned strip, D4 pouch, C4 boot
- skin_arms: partial (bare hand A2-A3 and forearm A1 only)
- fit_roles: mechanic, thug, entertainer
- palette_clash: rust and red-orange vani
## from w2/G7.md
## LAYOUT (male, from ref_M; 512 px canvases use the same 4x4 grid at 128 px cells)
- Row 1: A1+B1 = chest front (B1 holds the V-collar); C1+D1 = shoulder and back, seam at y~100. A1 top-left corner = small hand, glove or skin patch.
- Row 2: A2+A3 = hands or gloves (one mitten blob); B2 = waist band (belt, sash, corset or trouser top); C2+D2 = coat side and sleeve panel, D2 right-edge strip.
- Row 3: A2-A3 far-left edge = pouch and strap strip; B3 = trouser thigh or lower panel; C3+D3 = coat skirt with shoe or belt.
- Row 4: A4+B4 = plain khaki, tan or cream lower panel (largest plain area); C4 = boot; D4 = brown pouch or holster corner.
- Mostly black or empty cells: none in these 5 canvases (all fully textured).

## Canvases


### commm13
- base: olive coat (A1-D3 chest, sleeves, skirt), blue-grey trousers (B2-B3, C3-D3 band), khaki lower A4-B4, black boots C4, bare orange hands A2-A3.
- paint_zones: A1 left chest panel (plain olive, below corner patch); C2 coat side (plain olive); D1 upper back above seam (y<100); D3 coat lower panel; A4 khaki lower panel (large plain tan).
- avoid: B1 V-collar; A2-A3 hands and far-left pouches; B2 belt tab; C3-D3 shoe; C4 boots; D4 brown pouch; C1-D1 seam (y~100-125).
- skin_arms: partial (bare hands A2-A3 only; arms covered)
- fit_roles: mechanic, farmer, guard-civilian
- palette_clash: olive, khaki, dark blue-grey and black vanish; bright orange or red reads strongly; orange p
### commm14
- base: dark green robe (A1-D3, plain folds, V-collar at B1), orange-and-grey striped sash B2-C2, brown ribbed lower panel A3-B4, bare orange hands A2-A3, brown holster and bag D3-D4.
- paint_zones: A1 left robe chest (plain green, below corner patch); C1 robe shoulder (smooth); D2 robe side (x 300-460, y 130-300); D3 upper robe skirt (x 390-460, y 256-330).
- avoid: B2-C2 striped sash; A2-A3 hands and far-left straps; A3-B4 brown ribbed panel; C3-D4 holster and bag; D2 right-edge purple-grey strip (x>465).
- skin_arms: partial (bare hands A2-A3 only)
- fit_roles: scholar, merchant, entertainer
- palette_clash: dark green, black and brown vanish; orange, rust and tan clash with t
### commm15
- base: olive coat (same cut as commm13), blue-grey trousers B2-B3 with C3 band, khaki lower A4-B4, black dress shoe C3-D3, black boots C4, black gauntlets A2-A3 and A1 corner.
- paint_zones: A1 left chest panel (plain olive, below glove corner); C2 coat side (plain olive); D1 upper back above seam (y<100); D3 coat lower panel; A4 khaki lower panel.
- avoid: B1 V-collar; A2-A3 gauntlets; A1 corner glove; B2 belt tab; C3-D3 shoe; C4 boots; D4 brown pouch; C1-D1 seam.
- skin_arms: no (gloves cover the hands; no bare skin)
- fit_roles: mechanic, thug, guard-civilian
- palette_clash: olive, khaki and blue-grey vanish; black, charcoal and dark grey vanish on the gauntlets; orange, red
### commm16
- base: torn rust-brown and tan rags across A1-D2, tattered pale tan band B2, bloodied rust cloth C2-D2, dirty beige lower A4-B4, black gloves A2-A3, leather strap and metal pouch D3-D4.
- paint_zones: A1 chest left (torn rags; WEAR or STAIN only); C1 tattered sleeve (pale tan, x 300-512, y 0-120); D2 right-edge tan strip (x>465, y 130-300); A4 beige lower panel (x 0-230, y 370-512).
- avoid: B1 dark collar stains; A2-A3 gloves; B2 band; C2-D2 blood patches (new marks read as blood); D3-D4 strap and pouch.
- skin_arms: no (rust and blood tones only; the skin metric 0.276 is a colour false positive)
- fit_roles: refugee, thug, mechanic
- palette_clash: rust, brown, blood-red and g
### commm34
- base: cream-olive wrap tunic (A1-B1 chest, V-fold at B1), pale grey wrinkled cloth C2-D2, cream lower panel A4-B4, brown laced corset B2 (+C2 edge), brown belt pouch C3-D3, dark boot C4, bare pink hands A1 corner and A2-A3.
- paint_zones: A1 left chest panel (cream wrap, below fingers); C2 pale grey side (plain); D1 right sleeve top (cream, x 300-512, y 0-120); A4 cream lower panel (plain, stitched).
- avoid: B1 V-fold; B2 corset and laces; A2-A3 pink palm and A1 fingers; far-left pouches; C1 rust streak (x 380-420, y 0-30); C3-D3 belt pouch; C4 boot.
- skin_arms: partial (bare hands only: A1 corner fingers and A2-A3 palm)
- fit_roles: farmer, refugee, merchant
- palette_clash:
