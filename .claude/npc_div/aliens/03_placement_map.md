# Alien placement map (recon 03)

Source: `.claude/npc_div/registry.jsonl` (3519 rows). Pool = `fam` non-empty AND `role=generic`: 149 rows in 37 modules.
Category split of the pool: V = civilian 68, C = combat/gang 52, F = Dark Jedi / Sith Ghost / Proselyte 29.
Planet tags are inferred from stem suffix plus creature names. "unid" = no defensible name. V/C/F below is the category, not `fam`.
Brief hints corrected: Dantooine = 6xx `dan`, Korriban = 7xx `kor`, Malachor = 9xx `mal`, Onderon = 5xx `ond`, Dxun = 4xx `dxn`.

## (a) Modules with generic-family rows

| module | n | V/C/F | area (inferred unless noted) | main names |
|---|---|---|---|---|
| 512ond | 19 | 2/17/0 | Onderon, hostile zone | Thug{Hostile} x6, Beast-rider Thug x4 |
| 231tel | 11 | 0/11/0 | Telos surface, Czerka mercs | Mercenary{Grunt 01} x4 |
| 302nar | 11 | 11/0/0 | Nar Shaddaa, refugee quarter | Refugee{Man/Old Man} |
| 207tel | 10 | 10/0/0 | Telos Citadel, Telosian civilians | Telosian{M01s/M03s/F01s} |
| 203tel | 9 | 9/0/0 | Telos Citadel | Telosian{F02} x4 |
| 301nar | 9 | 8/1/0 | Nar Shaddaa, commoner district | {CA}Commoner{Nar Shaddaa Male} |
| 503ond | 8 | 4/4/0 | Onderon, cantina | Patron{Sitter} x4, Thug x2 |
| 204tel | 6 | 6/0/0 | Telos Citadel | Telosian{F01/F02/F03} |
| 960cor | 5 | 0/5/0 | cor (unid.), gang district | Gang Lieutenant x5 |
| tatm17aa | 5 | 5/0/0 | Tatooine, Anchorhead (name-confirmed) | Anchorhead Tradesman x5 |
| 202tel | 4 | 4/0/0 | Telos Citadel | Telosian{F01/M01s} |
| 605dan | 4 | 4/0/0 | Dantooine, salvage site | Salvager x3 |
| 903mal | 4 | 0/0/4 | Malachor | Dark Jedi |
| 906mal | 3 | 0/0/3 | Malachor | Dark Jedi |
| sley99ai | 3 | 0/3/0 | sley (unid.) | Czerka Security x3 |
| yavbase1 | 3 | 0/0/3 | Yavin 4, base | Dark Jedi x3 |
| yavshrine1 | 3 | 0/0/3 | Yavin 4, shrine | Sith Ghost x3 |
| 208tel | 2 | 0/2/0 | Telos Citadel | Benok / Exchange Thug |
| 306nar | 2 | 2/0/0 | Nar Shaddaa, cantina | Dancer, Twi'lek Dancer |
| 307nar | 2 | 2/0/0 | Nar Shaddaa, cantina | Dancer, Twi'lek Dancer |
| 610dan | 2 | 0/2/0 | Dantooine | Dead Mercenary x2 |
| 745kor | 2 | 0/0/2 | Korriban | Dark Jedi{Female 2}, Proselyte |
| 772kor | 2 | 0/0/2 | Korriban | Sith Ghost x2 |
| 905mal | 2 | 0/0/2 | Malachor | Dark Jedi |
| 965cor | 2 | 0/2/0 | cor (unid.) | Gang Lieutenant x2 |
| 969cor | 2 | 0/2/0 | cor (unid.) | Gang Lieutenant, Gang Underboss |
| lev3 | 2 | 0/0/2 | lev (unid.) | Dark Jedi x2 |
| mansithb1 | 2 | 0/0/2 | mans (unid.), Sith site | Dark Jedi x2 |
| yavtomb1 | 2 | 0/0/2 | Yavin 4, tomb | Dark Jedi x2 |
| 402dxn | 1 | 1/0/0 | Dxun | Scout{dead} |
| 740kor | 1 | 0/0/1 | Korriban | Dark Jedi{Male 2} |
| 959cor | 1 | 0/1/0 | cor (unid.) | Gang Lieutenant |
| gra802 | 1 | 0/0/1 | gra (unid.) | Dark Jedi |
| korr_m38ab | 1 | 0/0/1 | Korriban | Sith Ghost |
| lev2 | 1 | 0/0/1 | lev (unid.) | Dark Jedi |
| sley99aj | 1 | 0/1/0 | sley (unid.) | Twi'lek Thug |
| tatruins2 | 1 | 0/1/0 | Tatooine, ruins | Gang Lieutenant |

Planet subtotals (pool n): Telos 42 (202-208tel + 231tel) | Onderon 27 | Nar Shaddaa 24 | cor (unid.) 10 | Malachor 9 | Yavin 4 8 | Dantooine 6 | Korriban 6 | Tatooine 6 | sley (unid.) 4 | lev (unid.) 3 | mansith (unid.) 2 | Dxun 1 | gra (unid.) 1.

## (b) Top 8 areas by pool size: lore shortlist

Species app IDs are name-matched from the registry, not checked against appearance.2da.
Trandoshan 464/465 (54 rows) | Gamorrean 68/240-242/337 (87) | Weequay 599 (39) | Rodian 39/257/258/262/264 (50) | Sullustan 601 (8) | Duros 373/374/24 (46 incl. 631) | Ithorian 72/243/244 (24) | Bith 12 (12).

- Trandoshan: big-game and slave hunters. Fits beast-rider and bounty roles.
- Gamorrean: hired muscle for Hutts, Mandalorians, Sith. Already 21 rows in 302nar.
- Weequay: outer-rim pirates and freelance mercenaries.
- Rodian: traders, smugglers, bounty hunters. Fits Nar Shaddaa and any underworld module.
- Sullustan: pilots, engineers, clerks. Only 8 rows in the game, so rare by default.
- Duros: spacers and merchants. Fits Nar Shaddaa and Telos.
- Ithorian: medics and botanists. Fits Telos Citadel.
- Bith: scientists and musicians. Fits cantina bands and Telos labs.

## (c) Rarity targets per area

| area (module) | pool n | target | rows | shortlist | cap / notes |
|---|---|---|---|---|---|
| Onderon hostile (512ond) | 19 | 5-10% | 1-2 | Trandoshan, Gamorrean, Weequay | 17 combat rows, only 2 civilians |
| Telos surface mercs (231tel) | 11 | 5-10% | 1 | Weequay, Trandoshan | all combat rows |
| Nar refugees (302nar) | 11 | 0-3% | 0-1 | Rodian, Sullustan, Duros | Gamorrean already 21: exclude |
| Telos Citadel (207tel) | 10 | 3-5% | 0-1 | Ithorian, Sullustan, Duros | Bith 3 and Gamorrean 2 present |
| Telos Citadel (203tel) | 9 | 3-5% | 0-1 | Duros, Sullustan | Ithorian 2 present |
| Nar commoners (301nar) | 9 | ~10% | 1 | Rodian, Bith, Sullustan | Trandoshan 6, Weequay 4, Duros 2: cap |
| Onderon cantina (503ond) | 8 | 5-10% | 1 | Weequay, Gamorrean, Trandoshan | Twi'lek 5, Bith 3 present |
| Telos Citadel (204tel) | 6 | 0-3% | 0 | Duros (only if one row wanted) | Ithorian 8 already: cap reached |

Global: 120 non-Force pool rows x 3-10% = 4-12 alien rows.
Per planet: Telos (42) 1-4 | Onderon (27) 1-3 | Nar Shaddaa (24) 1-2 | Dantooine (6) 0-1, Rodian traders (no Rodian rows there yet).

Telos already has (registry): Ithorian 16 rows in 2xx-tel (204tel 8, 201tel 3, 203tel 2, 231tel 2, 208tel 1); Duros 6 rows (201tel 4, 202tel 2).

## Notes

- 29 F rows (Dark Jedi, Sith Ghost, Proselyte) are not commoners. Recommend excluding them from swaps.
- TM/TF families are mixed (Gang Lieutenant, Czerka Security, Twi'lek Dancer). Not a clean Twi'lek flag. Verify in appearance.2da.
- Brief's Duros list (24/374) is name-weak (Gang Member rows) and omits 373 (12 rows, Duros Miner / Bar Patron) and 631 (Duros Bounty Hunter, 401dxn).
- Ithorian 243/244 are name-weak (Moza, Holdo, Pato Ado appear under them).
- Percent targets round to 0 in modules under 10 rows. Use the planet-level pools.
- Stems cor, sley, lev, gra, mansith have no planet tag.
