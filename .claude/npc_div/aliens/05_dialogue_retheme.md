# 05 Alien commoner dialogue retheme (static recon)

Scope: read-only. Game dir `G` = `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II`.
Evidence method: pykotor `Installation` (Override > Modules > data/*.bif order), `read_dlg`, `read_tlk`, `read_utc`, `read_2da`; `find` for voice files.

## 1. Mechanism: node -> text + voice

- A creature's UTC `Conversation` names a DLG resref. Verified: `c_ithorian001` -> `200ithr`, `c_ithorian002` -> `204habat`, `c_ithorian003` -> `204ithr`.
- Each DLG node has `Text` (LocalizedString: `stringref` into `dialog.tlk`, or -1 plus embedded substrings), `VO_ResRef` (audio resref), `Sound` (blank in every node checked).
- DLG link fields `active1`/`active2` (+`_not`, `_param1..6`) gate which starter plays.
- The TLK entries behind the shared strrefs have an empty voiceover (78576, 108239, 108238, 110938-110940). In these rows the DLG `VO_ResRef` is the only voice source. The general TLK-vs-VO precedence is still open (doc 38, open question 2).
- Lookup order that pykotor uses: Override, then Modules `*.mod`, then `data/Dialogs.bif`.

## 2. Shared conversations (measured)

| conv | DLG found in | starters / nodes | VO files on disk | creatures (registry) |
|---|---|---|---|---|
| 200comm | `data/Dialogs.bif` | 3 one-liners, no replies | `StreamVoice/202/200comm/202200COMM001.wav`, `StreamVoice/GBL/200comm/GBL200COMM002,003.wav` (all present) | 14 Telosian (human-type) |
| 200comf | `data/Dialogs.bif` | 3 one-liners | `202/200comf/202200COMF001`, `GBL/200comf/GBL200COMF002,003` (present) | 16 |
| 201travm | `Modules/201TEL.mod` | 2 entries, 2 replies | `201/201travm/201201travm001,002` (present) | 2 Traveler |
| 200merc | `Modules/209tel.mod` | 3 entries | `203/200MERC/203200MERC001-003` (present) | 3 Mercenary |
| 200ithr | `Override/200ithr.dlg` | 3 one-liners | `StreamVoice/GBL/200ithr/` is EMPTY; `gbl200ithr001-003` absent from install | 6 Ithorian |

- Only 200ithr is silent. Its VO refs point at nothing, so the vanilla Ithorian line is subtitle-only (expected from disk; not played in-game).
- `Override/200ithr.dlg` already exists (md5 `b680d031`). Vanilla `Dialogs.bif` copy is md5 `86c344e1`. Same strrefs and text, different bytes. Owner unknown. Any mod that ships a 200ithr override collides with it.
- pykotor `Installation.resource(name, WAV)` returned MISSING for all 12 VO names checked, even those present on disk. Verify VO with path checks, not `Installation`.

## 3. Alien voice source: `alienvo.2da`

- `Override/alienvo.2da` (binary 2DA): 42 species rows (Aqualish ... Sith_Holocron). Columns are emotion/comment categories. Cells are AVO base names only. There is no text column.
- Example: Ithorian `comment_generic_medium` = `AVO_IthoCmntM`. Files: `StreamVoice/AVO/_IthoCmn/AVO_IthoCmntM1..3.wav` (also `_IthoAn`, `_IthoGre`, `_IthoQue`, `_IthoSca`).
- `StreamVoice/AVO/` holds 391 entries (per-species, per-category folders). Clips are L/M/S size with variants 1-3.
- Consequence: alien clips carry no subtitle text. Subtitles must come from a DLG node or an SSF/TLK entry.

## 4. Repo and KPM precedent

- TLK append precedent: `scripts/append_ranged_kite_tlk.py` (dry run by default; `--apply` backs up, verifies a pykotor round trip, atomic replace; game must be closed).
- `scripts/02_merge_2da_and_tlk.py` appended Echoes strings 136375-136398 and staged UTC files into the live override.
- DLG reader precedent: `scripts/npc_div_registry.py` `locstring_text` (TLK strref, else first embedded substring). Reader only.
- Embedded text is unproven in-engine. The 201travm reply nodes use strref -1 with empty substrings.
- KPM: no `.tlk`, `.dlg` or override installer in `src/**/*.cs`. A `.kpatch` carries DLL, hooks and manifest. Static files go to the game dir through repo staging (`patch_manager_mods/npc_diversity_src/README.md`, "Packaging").
- Doc 38 claim 9: `.kpatch` cannot add 2DA rows. Same limit applies to DLG/TLK.

## 5. Constraints

- Keeping 200comm on an alien is not viable for alien voice. VO lives per node in a shared DLG, so the alien would play Telosian VO. A per-creature conversation is needed.
- Shared template scope: `c_ithorian001` covers 6 creatures (all Ithorian). Overriding `c_ithorian001.utc` retargets all 6. A single creature needs a unique template resref plus a GIT edit in the module capsule (`scripts/04_merge_modules.py` pattern). GIT cannot be overridden.
- Never edit a shared strref in place. Other DLG or SSF entries may use it (usage count not measured). Append new TLK rows instead.
- Subtitle duration and lip sync for AVO clips are unknown. Not measured.

## 6. Recommended pipeline

1. Pick target creatures: template resref and current UTC `Conversation` (e.g. `c_ithorian001` -> `200ithr`).
2. Pick species VO: `alienvo.2da` row, category `comment_generic_medium` or `greeting_medium`. Confirm `StreamVoice/AVO/_<Species><Cat>/AVO_<Base><L|M|S><n>.wav` exists by path.
3. Clone the source DLG with pykotor. New resref of 16 chars or fewer, no collision with `Override/`, `data/Dialogs.bif` or modules. Keep the node graph and link conditions. Set each entry `VO_ResRef` to a chosen AVO base name.
4. Dry run, then append new TLK rows at the end of `dialog.tlk` (strrefs N..N+k, empty voiceover). Point the cloned DLG nodes at those strrefs. Do not touch existing rows.
5. Write a UTC copy with `Conversation` set to the new resref. If the template is alien-only, ship it as `override/<template>.utc`. Otherwise plan a new template plus GIT edit.
6. Stage the `.dlg`/`.utc` under `patch_manager_mods/<mod>_src/override/`. Install through repo scripts (not `.kpatch`).
7. Pre-install gate: check every new resref against `Override/`, `Dialogs.bif` and modules; confirm 200ithr override handling; path-check every VO file.
8. Colocated tests: DLG round trip, strref range, VO path exists, non-empty text, no shared strref modified.
9. User play check: confirm subtitle text and clip audio for each starter. Log the conv and VO names.
10. Record in the mod doc and commit explicit paths only.

## 7. Build list

- `alien_conv_retheme.py`: clone, retheme and validate a DLG (pykotor read/write). Colocated `test_alien_conv_retheme.py`.
- TLK append: reuse the `append_ranged_kite_tlk.py` pattern (dry run, backup, round-trip check) for new rows.
- UTC retarget: override copy (shared template) or new template plus GIT edit (single creature).
- VO path checker against the `StreamVoice/AVO` tree (not `Installation`).
- Optional in-game probe: one log line per conversation start (conv, VO resref, strref).

## 8. Open

- Engine VO lookup for a DLG `VO_ResRef`: flat search or folder-specific? Test with one alien clip.
- Whether AVO grunts ever show subtitle text (alienvo has no text column).
- Embedded locstring rendering for NPC nodes.
- Who installed `Override/200ithr.dlg`, and whether it differs from vanilla beyond bytes.
- Number of DLG/SSF users per strref (before any in-place TLK edit).
