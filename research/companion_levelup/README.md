# Companion level-up research (KOTOR II, all convertibles -> Jedi)

Scraped 2026-09-29. Goal: per-companion feat / Force power / skill / class-switch plans for the r/kotor Full build
(TSLRCM + PartySwap + Extended Enclave + Thematic Companions, etc.). Nothing here is committed to git yet.

## Layout
- `raw/elkripper/`   elkripper.com per-companion build pages (vanilla-stat baseline, level-by-level tables). HTML.
- `raw/strategywiki/` StrategyWiki TSL pages: companions (Atton, Bao-Dur, Disciple, Handmaiden, Mira, Visas) and
  mechanics (Classes, Feats, Force_powers, Skills, Attributes, Implants, Experience, Party). HTML.
- `raw/deadlystream/` DeadlyStream pages for installed mods that touch progression (see below).
- `text/`            plain-text conversions of everything above (`html_to_text.py`). Grep these first.
- `installed_mods_progression_relevant.json`  entries from ../../neocities_mods_detailed.json for the mods that matter.
- `installed_mod_thematic_companions/`  details.txt/.rtf = official change list, utc/ = mod's shipped templates.
- `live_utc/`  p_*.utc copied from the LIVE game Override (ground truth for this install).
- `live_companions.json`  live_utc decoded by `dump_companion_utc.py` (attrs, class/level, feat ids, powers, skills).

Fetch note: StrategyWiki and elkripper return 403/blocked without a browser User-Agent; curl with a Firefox UA works.

## Findings that shape the plan
1. Vanilla data is STALE for this install. Thematic KOTOR 2 Companions (#8) changed: Atton CON12/CHA15; Disciple
   STR12 DEX14 INT15 WIS15 CHA8 + skills; Handmaiden CHA13 and skills 27->9 pts; Mira STR14 INT12 CON13 WIS12;
   Bao-Dur loses Flurry/Critical Strike. Use `live_companions.json`, not elkripper's starting tables.
2. Live Disciple still has Close Combat (139) and no Finesse: Melee (194) although the change list says otherwise
   (feat 157 "Superior Weapon Focus: Lightsaber II" also present) - verify in game before planning his feats.
3. Classic Class Attack Bonus (#10): Guardian 1.0/lvl, Sentinel 0.75, Consular 0.5 -> Consular Disciple is a weak
   melee class here; Guardian (Handmaiden, Bao-Dur) is best.
4. Alignment Affects Force Powers + Treat Injury Affects Force Healing (#142): LS powers up to +25% at full LS,
   DS powers down to -50% (CHA mod reduces the penalty 5%/pt); Heal potency scales with Treat Injury. Avoid DS powers
   (e.g. Drain Life/Death Field in elkripper's Handmaiden build). Conflicts with a "Treat Injury 1-2 ranks" plan.
5. Handmaiden Jedi switch is gated by 3 duels: 2nd needs Exile able to reach level 14 (91,000 XP), 3rd level 18
   (153,000 XP); first duel has no level gate. Do not level/equip her before the duels. Battle Precognition comes
   from a conversation after the first duel (StrategyWiki), not after all three.
6. Companion attributes auto-increase (fixed per companion); feats, powers, skill points are player-chosen.
7. Auto level-up: packages.2da is per CLASS (8 rows), pointing at class-wide ordered feat/skill preference lists
   (PackFtJedis1 etc.). No per-companion path and no power preference table -> per-character override needs an
   engine/script hook (e.g. the Patch Manager framework), not just 2DA edits (Exile/Atton/Mira share JediSentinel).
8. Improved LS Enlightenment (#139): Jedi Master/Sith Lord +50% influence gain AND loss. Bao-Dur Heavy Armor (#108)
   adds Heavy armor proficiency to Tech Specialist level-ups. JC's Crystal Attunement (#109) makes the Quest Crystal
   class-based (Exile only).

## Still unresolved
- Bao-Dur early-switch plan (elkripper only models switching at 30); Atton's higher-level feat picks; Visas sub-builds.
- Effect of TSLRCM 1.8 / Extended Enclave / PartySwap on conversion gates and levels (TSLRCM 1.8 reworked Atton,
  Mira, Visas, Bao-Dur post-Jedi training content - see moddb news).
- Exact conversion-level requirements are not hard level gates for Atton/Bao-Dur/Mira/Disciple (influence + dialogue).
- Confirm in game whether Thematic Companions feat changes match `live_companions.json`.

## Not yet fetched (leads)
- Steam guide "Guide to building all the meatbag party members" (steamcommunity sharedfiles/discussions 1732089092451047169)
- DeadlyStream "KotOR2 Companion Fix" (file 2404) and Snigaroo's compendium (r/kotor)
- kotor.neocities.org/modding/mod_builds/k2/full (source of the build list)
