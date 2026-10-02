# Play checklist, session of 2026-10-02 (Phase A)

This is the checklist the user was given on 2026-10-02 (the 2026-10-01 list plus E3). `analyze_play_logs.py` uses these item ids in its pass/fail table.

## Mod 6 (inventory)
- **M6-1** The backpack shows new items first, then items the shown character can use, then the rest grouped by kind.
- **M6-2** Gear the shown character can't equip has a red tint, and the tint changes when you switch party members.
- **M6-3** Close the inventory, then open an Equipment slot list, hover and click. Equipping works and no rows are wrongly red (H6d).
- **M6-4** Clicking "All" again cycles New -> Usable -> Upgradeable -> All, and the caption fits.

## Mod 8 (Ranged Kiting)
- **M8-1** Pick "Ranged (Kiting)" for a blaster companion. The message shows and the HUD icon appears.
- **M8-2** It keeps its distance from melee enemies, and moves for a clear shot when an ally or pillar is in the way.
- **M8-3** It kites at most twice in a row, then shoots.
- **M8-4** With the target far away or a melee weapon equipped, it walks in instead of freezing (0.3.0 `kite_approach`).
- **M8-5** It already kites on its very first combat round.

## Companion AI (any stance)
- **CAI-1** Freeze fix (A1): a vanilla Ranged companion whose target is out of range no longer stands frozen. It switches to an enemy in range, or re-thinks within about 3 s.
- **CAI-2** Healing (H2): a companion with a heal power heals any party member below 50%, not only the leader. It uses no medpacs or antidotes.
- **CAI-3** Poison: a companion with Heal or Master Heal cures a poisoned party member within about 15 m. A companion with only Cure doesn't try.
- **CAI-4** Area powers: a Jedi Support companion never centres Storm, Wave or similar powers on an ally.
- **CAI-5** Revitalize (H1): when someone is knocked out, a Jedi Support companion without Revitalize keeps fighting.
- **CAI-6** After a fight (E1): Ranged and Kiting companions follow as soon as no enemy is left. They don't leave while an unseen enemy is still shooting.
- **CAI-7** Walking into a companion (FM1): it steps aside and keeps following or shooting.
- **CAI-8** Save/load (FM4): companions still follow normally after a save and a load.

## E3 (logs only)
- **E3** A companion on the vanilla "Ranged" stance (11) in an open area, with an ally on the line of fire. The user notes the clock time.

## Logs (copy them out right after the game closes)
- `$G/ai_trace.txt`
- `$G/ranged_ai_log.txt`
- `$G/inventory_sort_filter_log.txt`
- `~/steam-208580.log`

Copy them to `~/Games/KOTOR2_Mod_Workspace/playlogs_cai_<MMDD_HHMM>/`.
