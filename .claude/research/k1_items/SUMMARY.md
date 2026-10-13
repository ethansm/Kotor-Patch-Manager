# K1 item inventory vs K2 (static)

K1 dir: `/home/ethansm/.local/share/Steam/steamapps/common/swkotor` (read-only). K2 dir: `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II` (read-only). Output: `/home/ethansm/orca/workspaces/local-setup/KOTOR-II/.claude/research/k1_items`.

## Counts

- K1 UTI occurrences (all containers): 1055; distinct K1 UTI resrefs: 810
- K2 UTI occurrences (all containers): 1957; distinct K2 UTI resrefs: 1527
- K1-only UTI resrefs (no UTI with that resref in K2): 586
- K1-only whose base item label also exists in K2: 585
- K1-only with a tag match in K2 (non-empty tag): 12; name match in K2 (non-empty name): 78
- K1-only with a model that loads (MDL+MDX parsed): 382 of 586; status breakdown: no-mdl=107, no-model=97, yes=382
- K1-only model_derived: rule=382, guess=107, none=97
- K1-only with textures and ALL textures missing in K2 (TPC/TGA): 0; K1-only with no mesh texture_1 at all: 205
- K1-only texture entries missing from K2 chitin/override/modules but present in K2 TexturePacks (not counted as missing above): 8
- K1-only items whose chosen icon is absent in K2 (override/chitin/modules/TexturePacks): 22; icon_rule counts: itemclass_NNN=563, defaulticon=22, none=1

## K1-only per baseitem label (top 20)

- Plot_Useable_Items: 65
- Mask: 30
- Data_Pad: 28
- Lightsaber_Crystals: 28
- Credits: 23
- Forearm_Bands: 23
- Droid_Utility_Device: 15
- Implant_3: 14
- Blaster_Pistol: 13
- Lightsaber: 13
- Armor_Class_4: 12
- Belt: 12
- Blaster_Rifle: 12
- Gauntlets: 12
- Armor_Class_5: 11
- Vibro_Sword: 11
- Armor_Class_6: 10
- Armor_Class_9: 10
- Blaster_Carbine: 10
- Heavy_Blaster: 10

## Base item diff (case-insensitive label match)

- DIFF_DEFAULTMODEL: 1
- DIFF_ITEMCLASS: 1
- K2_ONLY: 17
- SAME: 89

## Parse failures and resolution misses (counts only)

- K1 UTI copies that failed to parse: 0; distinct resrefs with no parseable copy: 0
- K1 UTI names that failed to resolve via talk table: 0; baseitems names that failed: 0
- K1 rims files that failed to open: 0 of 12
- K1 blank baseitems labels (excluded from diff): 1; duplicate baseitems labels (first row kept): 0
- K2 UTI copies that failed to parse: 0; distinct resrefs with no parseable copy: 0
- K2 UTI names that failed to resolve via talk table: 0; baseitems names that failed: 0
- K2 rims files that failed to open: 0 of 0
- K2 blank baseitems labels (excluded from diff): 1; duplicate baseitems labels (first row kept): 0
- K1-only MDL/MDX read failures (parse-fail): 0; no-mdx: 0; no-mdl: 107

## Rules and assumptions

- Containers: override, chitin bifs, every module file in modules/ (.rim, _s.rim, .mod; K1 234 files, K2 462 files; symlink case-variants collapsed: K1 0, K2 822), and every .rim/.mod/.erf in rims/ (K1 12, K2 0).
- One row per distinct resref (case-insensitive). Primary copy: OVERRIDE > CHITIN > MODULE > RIMS, then container name. Every copy is listed in `containers`; `copies` counts them.
- Matching is case-insensitive for resref, tag, name and baseitems label. Empty tags and empty names never match.
- Names: UTI LocalizedName and baseitems `name` strref, resolved through each install's own talk table (dialog.tlk, default/masculine). Unresolved = empty string.
- Baseitem stats: no `baseitemstats` 2DA exists in either install; `stat columns` means the baseitems.2da stat columns (baseac, dexbonus, accheck, armorcheckpen, numdice, dietoroll, critthreat, crithitmult, basecost, weapontype, weaponsize, weaponwield, rangedweapon, maxattackrange, stacking, maxprops, minprops, propcolumn, armortype, tenthlbs, weaponmattype, ammunitiontype, itemtype, itemmultiplier), each written only when present.
- Icon rule (icon_rule): candidate = `i` + lowercase itemclass + `_` + 3-digit N, N = TextureVar for armor (modeltype 1), ModelVariation otherwise. Used when that TPC/TGA exists (override, chitin, modules, TexturePacks). Otherwise the baseitems.2da `defaulticon` is used (icon_rule=defaulticon). Checked on the installed files: the candidate exists for most rows in both games (see counts in the K1/K2 files); this is an empirical rule, not decompiled engine code. textures.2da is absent in both games.
- Model texture checks (texture_in_k2) use TPC/TGA from override/chitin/modules only, as specified. TexturePacks are counted separately above.
- Model rule (model_derived): candidate A = `itemclass_NNN` when ModelVariation NNN > 0 (3-digit zero padded); candidate B = baseitems `defaultmodel` unless it is I_Null. The first candidate that exists as an MDL in that game (override/chitin/modules) gives model_derived=rule. If none exists: model_derived=guess and model_resref = defaultmodel (or A when defaultmodel is I_Null); if no name can be formed: model_derived=none. Example confirmations in K1 chitin: w_Qtrstaff_001 (weapon), i_belt_001 (belt, itemclass I_Belt), a_jedirobe_001 (armor, itemclass a_JediRobe). Armor body-part models (modeltype 1) are not fully derivable from these fields.
- Textures: MDL mesh `texture_1` of every mesh node (lowercased, `null` skipped) is the texture list; `texture_2` (lightmap) is a separate column and is not checked against K2.
- K2 presence for model and texture checks uses override + chitin + modules only (rims excluded). TPC and TGA both count as a texture.
- K1 MDL reads reuse the install_get / read_mdl(source_ext=MDX) call from patch_manager_mods/k1_port/k1_port_tex_preview.py dump_model.
- `model_loads`: yes = MDL+MDX parsed; no-model = no model name; no-mdl / no-mdx = missing; parse-fail = read_mdl raised.
- Output is sorted by resref (lowercase) and label, so reruns on the same installs are identical.
