# Wave 4 variant system: slots, archetypes, counts, gate, POC

Basis: catalog_compact.tsv (174 cards), w4/canvas_summary.md (35 canvases), placement_census.tsv, A12 TECHNIQUES, w3b sym_01/sym_03 grammar.
Tier rule: A/B for named factions; C only for generic WEAR and PAL. Stages: S1 palette, S2 overlays (EMB, TRM, HW, PAT, TAT), S3 weathering (WEAR, STN).
Census groups: 1xx Peragus, 2xx Telos, 3xx Nar Shaddaa, 4xx Dxun, 5xx Onderon (by NPC names; see RISKS).

## 1. SLOTS (bake set = 40 cids; reserve = other eligible cids)

| Slot | Mode | Bake-set cids (count) | Notes |
|---|---|---|---|
| PAL palette recolour | 1 max, S1, exclusive | K044 K072 K074 K075 K078 K115 (6) | drop per canvas if G3 fails |
| EMB emblem (chest, shoulder, back) | 1 max, S2, exclusive | K033 K042 K054 K063 K067 K148 K149 K150 (8) | one faction identity per NPC |
| TRM trim band (cuff, collar, hem) | 1 max, S2 | K024 K035 K043 K053 K076 K082 K166 (7) | with EMB only on cuff, collar, hem |
| WEAR (families: fade K012 K020; dust K027; soot K028 K091; patch K037) | 2-3 families, S3 | K012 K020 K027 K028 K037 K091 (6) | counts as 1 slot |
| STN stain | 1 max, S3, exclusive | K015 K030 K049 K061 (4) | never on a zone that has soot |
| HW belt or pouch hardware | 1 max, S2 | K050 K058 K163 K168 (4) | exclusive with belt TRM and hip PAT |
| PAT stencil or pattern | 1 max, S2 | K018 K066 K069 (3) | not on chest_left with EMB |
| TAT tattoo (bare-skin islands) | 1 max, S2 | K099 K132 (2) | only the 7 canvases with skin_arms partial |

Reserve (eligible, not in bake set): EMB K019 K025 K026 K062 K087 K088 K098 K103 K107 K137; TRM K016 K045 K065 K068 K077 K083 K086 K089 K094 K106 K144 K172; WEAR K011 K013 K029(C) K036 K048 K052 K060 K064 K080 K110 K111 K112; PAL K085 K145 K167; HW K090 K102 K109 K162 K164; PAT K014 K032 K040 K041 K081 K165.

Rules:
1. Max 3 slots per look; WEAR and TAT count once.
2. Two faction-tagged emblems never combine on one NPC.
3. PAT on upper_back is fine with EMB; on chest_left or chest_center it is not.
4. STN and soot on the same zone are exclusive.
5. Species-locked cards never go on human canvases: K001-K010, K116-K136, K169. Also excluded: K017 (tagged INFERRED), K093, K096, K105 (Legends-sourced).
6. sym_01 and sym_03 emblems are not add-ons: they are not in the catalog, era OTHER, not KOTOR 2 factions. Use their grammar (ring_spokes, chevron, wings, tally, cog) only to brief new tier-B cards.

## 2. ARCHETYPES (11)

Weights = percent of that archetype's NPCs that get at least one add-on in the slot (PAL/EMB/TRM/WEAR/STN/HW/PAT/TAT). (R) = reserve cid.

**A1 Telosian civilian.** Census 2xx 27 (202:4, 203:9, 204:6, 207:8). Canvases (10): n_commf01 f05 f33 f35 f04 n_commm01 m08 m09 m11 commm34.
- Weights: PAL0 EMB30 TRM15 WEAR45 STN10 HW10 PAT10 TAT0.
- Examples: EMB K033 K148 K063 | TRM K024 K045(R) K016(R) | WEAR K027 K037 K012 | STN K030 K015 | HW K050 K163 | PAT K018 K066.

**A2 Republic officer.** Census 2xx 9 (202:1, 205:3, 222:5); 3 more at m26 (unmapped). Canvases (7): n_commf03 f11 n_commm02 commm04 commm13 commm15 n_commm09.
- Weights: PAL30 EMB45 TRM30 WEAR20 STN0 HW10 PAT0 TAT0.
- Examples: PAL K075 K145(R) | EMB K149 K148 K026(R) | TRM K024 K016(R) | WEAR K028 K091 K012 | HW K163.

**A3 TSF officer.** Census 2xx 19 (202:3, 203:1, 205:2, 220:6, 221:6, 222:1). Canvases: same 7 as A2.
- Weights: PAL0 EMB0 TRM40 WEAR40 STN10 HW15 PAT0 TAT0.
- Examples: TRM K043 K024 K016(R) | WEAR K028 K027 K091 | STN K030 | HW K050.
- No tier A/B TSF emblem exists; K022, K023, K031 are C and excluded.

**A4 Telos mercenary.** Census 2xx 25 (202:2, 231:9+2, 233:9+3). Canvases (6): n_commf07 n_commf16 commm03 n_commm12 commm15 commm16.
- Weights: PAL5 EMB15 TRM35 WEAR45 STN15 HW35 PAT10 TAT0.
- Examples: PAL K167(R) | EMB K067 K062(R) K087(R) | TRM K166 K043 K086(R) | WEAR K091 K028 K048(R) | STN K061 K030 | HW K163 K168 K058.

**A5 Exchange thug.** Census 2xx 2 (208); 3xx 32 (Exchange 24, Thug 3, Thug Boss 1, Twik's Thug 4); 5xx 15 (Thug at 503, 512). Canvases: same 6 as A4.
- Weights: PAL5 EMB25 TRM35 WEAR35 STN20 HW20 PAT15 TAT5.
- Examples: EMB K067 K054 K062(R) | TRM K053 K068(R) K166 | WEAR K091 K012 K048(R) | STN K061 K049 K015 | HW K058 K050 K168 | PAT K066 K069 K018 | TAT K099.

**A6 Serroco thug.** Census 3xx 20 (302: Serroco Thug 16, Serroco Veteran 4). Canvases: same 6 as A4.
- Weights: PAL0 EMB35 TRM40 WEAR35 STN15 HW15 PAT10 TAT0.
- Examples: EMB K054 (signature) K067 K062(R) | TRM K053 (signature) K068(R) K166 | WEAR K091 K020 K012 | STN K061 | HW K058 | PAT K066.
- Only two Serroco-tagged B cards exist (K053, K054). Merge with A5 if the signature cannot carry a look on its own.

**A7 Nar Shaddaa refugee.** Census 3xx 19 (302 Refugee 11, 301 Commoner 8). Canvases (12): n_commf01 f05 f07 f11 f16 f35 n_commm01 m10 m11 commm07 commm16 commm34.
- Weights: PAL0 EMB15 TRM0 WEAR75 STN5 HW0 PAT5 TAT0.
- Examples: WEAR K037 K012 K020 | EMB K063 K019(R) K025(R) | STN K015 | PAT K014(R).

**A8 Bar patron.** Census 3xx 28 (304 Bar Patron 12, 306 Patron 7, 307 Patron 7, Dancer 1+1); 2xx 1 (207); 5xx 4 (503). Canvases (6): n_commf06 f33 commm05 commm14 n_commm08 n_commm12.
- Weights: PAL30 EMB0 TRM25 WEAR5 STN10 HW0 PAT35 TAT0.
- Examples: PAL K072 (only B card) | TRM K065(R) K035 K083(R) | PAT K069 K041(R) K014(R) | STN K061.
- No HW: K071 is C and excluded.

**A9 Onderon citizen.** Census 5xx 16 (Iziz Citizen 11, Crowd 5). Canvases (5): n_commf33 f35 n_commm01 n_commm09 n_commm11.
- Weights: PAL0 EMB0 TRM45 WEAR40 STN10 HW0 PAT15 TAT0.
- Examples: TRM K082 K083(R) | WEAR K080(R) K012 K020 | PAT K081(R) K066 | STN K030.
- K084 is C and excluded, leaving two TRM cards.

**A10 Onderon soldier.** Census 5xx 22 (Onderon Soldier 9, Royalist 7, Vaklu 6). Canvases (7): n_commf03 f11 f14 n_commm02 commm04 commm13 commm15.
- Weights: PAL50 EMB15 TRM45 WEAR30 STN15 HW10 PAT0 TAT0.
- Examples: PAL K074 K075 K078 | EMB K150 (borrowed Khoonda card) | TRM K076 K077(R) K144(R) | WEAR K028 K091 K012 | STN K030 | HW K163.

**A11 Czerka staff.** Census: 'sle' 12 (Czerka Security; code unmapped). No clerk row exists, so "Czerka Security" stands in. Canvases (6): n_commf03 f04 n_commm02 commm04 commm13 commm15.
- Weights: PAL15 EMB30 TRM30 WEAR15 STN0 HW10 PAT5 TAT0.
- Examples: PAL K044 | EMB K042 K107(R) K137(R) | TRM K043 K045(R) K106(R) | WEAR K028 K052(R) | HW K050 K163 | PAT K018.

Not archetyped: Citadel merchant (0 rows; served by A1 canvases tagged merchant), Peragus worker (0 humanoid rows at 1xx, 107 droid rows), Khoonda settler (0 rows), Dxun 4xx (0 covered; Cannok, Maalraas and Boma are species or armour). Telos species rows (18: Ithorian 8, Bith 2, Twi'lek 2, Nahata 2, Matu 2, Aqualish 1, Benok 1) need species bodies.

Census coverage (mapped rows): 2xx 83 (A1 27, A2 9, A3 19, A4 25, A5 2, A8 1). 3xx 99 (A5 32, A6 20, A7 19, A8 28). 5xx 57 (A5 15, A8 4, A9 16, A10 22). 1xx and 4xx 0. About 45 rows fall in unmapped codes (sle, 970/971, 955, m26, tat).

## 3. COUNTS

- Canvases: 35 (18 female, 17 male). 28 have fit_roles; 7 female are untagged (n_commf02, 08, 09, 10, 12, 13, 15).
- Bake-set add-ons: 40 (PAL 6 + EMB 8 + TRM 7 + WEAR 6 + STN 4 + HW 4 + PAT 3 + TAT 2).
- Single add-on variants: 35 x 40 = 1,400.
- Minus TAT on 28 non-skin canvases (2 x 28 = 56): 1,344.
- Minus palette clash (estimate 2 per canvas, 70; exact count after G3): about 1,274.
- Two-add-on looks: about 20k before slot rules (estimate: 35 canvases x C(36,2)).
- Bake plan: 35 canvases x 11 looks = 385 (1 base plus 10 add-on looks each), plus 15 gate spares = 400 textures.
- Each look averages 1.6 add-ons, so each add-on appears about 14 times. Per-archetype reach: 5 to 12 canvases x 11 = 55 to 132 textures.

## 4. QUALITY GATE (per bake; all must pass)

- G1 Avoid: 0 add-on pixels inside any `avoid` cell or box in canvas_summary.
- G2 Area: add-on pixels / cloth pixels in [3%, 25%]. PAL is exempt.
- G3 Palette: deltaE76 (mean Lab over paint zones, variant vs canvas base) >= 15, else drop that PAL option on that canvas.
- G4 Distinct pairs: within a canvas, every pair of its 11 looks has deltaE76 >= 8 (mean Lab over paint zones), OR add-on mask IoU <= 0.5 with at least 2 differing slot cids. Failing looks are dropped and respared.
- G5 Legibility at 256 px (atlas / 4): EMB, PAT, TAT need deltaL* >= 20 against local cloth and strokes >= 2 px. TRM needs deltaL* >= 15 and a band >= 2 px. WEAR and STN need deltaL* >= 6.
- G6 Shape read: 4 blind raters name the emblem grammar at 256 px; at least 3 of 4 must match the card's grammar.
- G7 Lore: 0 tier-C cards in named-faction slots; 0 Legends or INFERRED cards; faction tag matches the archetype.
(Tooling: sRGB to CIELAB D65; G2-G4 at 1024 px, G5 at 256 px.)

## 5. POC: 8 recipes for the Telos play test

Bake order is S1, then S2, then S3. Base colours differ across recipes: tan, cream, pale grey, olive, taupe, rust, blue-slate, light blue.

| # | Canvas (base) | cids | Stage | Serves |
|---|---|---|---|---|
| P1 | n_commm01 (tan) | K033, K027 | S2,S3 | A1 Telosian civilian |
| P2 | n_commf33 (cream dress) | K018, K028 | S2,S3 | A1 |
| P3 | n_commm02 (pale grey) | K075, K024, K149 | S1,S2 | A2 Republic officer |
| P4 | commm15 (olive coat) | K043, K091 | S2,S3 | A3 TSF officer, A4 pool |
| P5 | n_commf07 (taupe wrap) | K166, K061 | S2,S3 | A4 Telos mercenary |
| P6 | commm12 (rust wrap coat) | K067, K058 | S2 | A5 Exchange thug (Telos 208) |
| P7 | n_commf06 (blue-slate tunic) | K072, K069 | S1,S2 | A8 bar patron (Telos 207) |
| P8 | n_commm09 (light blue tunic) | K042, K050, K015 | S2,S3 | A11 Czerka staff |

P1 and P2 are the closest pair (tan vs cream-tan) and should be checked first under G4.

## RISKS
- TAT has only 2 human-valid cids (K099 B, K132 A); commission 2-3 tier-B human tattoo cards or drop the slot.
- TSF and Serroco have no tier A/B signature emblem; A3 borrows K043 and A10 borrows K150, which needs a lore check.
- K105, K096, K093 are Legends-sourced and K017 is INFERRED; K021 duplicates K148 (Republic cog); K075 is Onderon-tagged but serves as Republic PAL.
- Census: 1xx and Khoonda humanoid rows absent; 4xx covers 0 NPCs while Onderon names sit in 5xx; 7 female canvases are untagged.
- Species bodies: K001-K010, K116-K136 and 18 Telos alien rows cannot use human canvases; the 35 canvases are human-shaped only.
