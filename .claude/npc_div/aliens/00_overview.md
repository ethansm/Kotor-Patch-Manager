# Rare alien commoners: feasibility overview (2026-10-11)

Synthesis of five static Haiku recon files (01 rows, 02 audio, 03 placement, 04 engine, 05 dialogue). All verdicts PARTIAL: static evidence only, nothing tested in game. Not part of the Wave 6 plan; a follow-on if the user wants it.

## Verdict
Feasible in two tiers.
1. **Tier A (cheap, same hook):** swap a small, deterministic share of generic commoners to an alien appearance row inside `RemapNpcAppearance` (same `Mix(hash, salt)` roll, new salt 6, per-area table). Only rows whose models exist qualify.
2. **Tier B (new work):** alien-language voice with lore-themed subtitles (per-creature DLG + appended TLK rows), plus area-specific tables.

## Which species work (01)
- Head-swap only (human N_CommM/F body, alien head): Devaronian 570, Twi'lek male rows 416/449/450/606/618 (and 805/806 partial). Lowest risk.
- Species-body rows (model exists): Bith 12, Duros 24, Rodian 39, Selkath 40 (type L unverified), Sullustan 601, Gand 569. Risk: animation/walk speed/head gating unproven.
- Not viable: Ithorian and Jawa (blank modela), female Twi'lek F rows (P_TwiH heads missing).

## Where and how rare (03)
Pool is small: 149 generic-row creatures in 37 modules, about 68 real civilians; 3-10% gives 4-12 alien swaps game-wide. Use planet-level pools (Telos 42, Onderon 27, Nar Shaddaa 24). Telos already has Ithorians (16) and Duros (6): cap them. Suggested: Telos Citadel 3-5% Sullustan/Duros; Nar Shaddaa ~10% Rodian/Bith/Sullustan; Onderon 5-10% Weequay/Trandoshan/Gamorrean. Exclude the 29 non-commoner F rows (Dark Jedi, Sith Ghost, Proselyte).

## Engine (04)
Body model reads the row at SetAppearance entry, so the swap works for F-type alien rows. Unproven: voice (UTC soundset, not the row), head gating for F rows (0x85D430), drive speed/footsteps, hitbox, gender byte. Needs a probe build that logs these, then one in-game look. Story-tag exclusion is still inactive (tag unreadable), so keep-default roles from the registry must land first.

## Audio and text (02, 05)
- Voice: UTC SoundSetFile -> soundset.2da -> .ssf -> TLK strref -> WAV. Alien emotes live in `alienvo.2da` (43 species rows) pointing to `StreamVoice/AVO/<folder>/AVO_*.wav`; Rodian, Twi'lek, Duros, Bith, Ithorian, Hutt, Gand, Gran, Sullustan, Devaronian are fully covered.
- Text: AVO clips have no text; alien speech in the game is English text or silent vocals. A TLK text swap keeps the audio (`TalkTable.replace` with empty `sound_resref`).
- Cleanest pipeline: new per-creature DLG nodes keep `vo_resref=AVO_*`, text strrefs appended to `dialog.tlk` (precedent: `scripts/append_ranged_kite_tlk.py`).
- Soundset follows the UTC, not the appearance row, so a swapped human commoner would still use a human voice for combat grunts/barks: needs a UTC soundset change or accept the mismatch.
- KPM has no TLK or 2DA-row patch support (confirmed in memory `kotor2-k1-asset-import`), so appended TLK/DLG ships via the existing full-install path.

## Open (needs a probe or REA)
1. DLG `vo_resref` vs TLK voiceover precedence. 2. `alienvo` loader and cell-to-file rule. 3. Whether bark text is drawn. 4. Walk speed, footsteps, head gating, soundset source for a swapped row.

## Suggested next steps (after Wave 6 passes)
A. Probe build logging soundset/speed/head for rows 570, 416, 12, 24, 39. B. Add `kAlienTable` (area -> rows, percent) behind the keep-default gate. C. One alien-language bark DLG as a pilot (e.g. Rodian in Nar Shaddaa).
