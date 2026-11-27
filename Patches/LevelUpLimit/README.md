# Max Level Increase

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Sets the maximum level in kotor 1

## Details

- Patch id: `level-up-limit`, version 2.0.1
- Author: Arniel_Stanislaus aka LanguageWriter123 (with revisions from Lane)
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `hooks.toml`: 62 hook(s) (6 detour, 17 replace, 39 simple)

## Files

- `Reallocations.cpp`: source
- `additional/acbonus.2da`
- `additional/classes.2da`
- `additional/classpowergain.2da`
- `additional/cls_spgn_jedi.2da`
- `additional/cls_st_cm_drd.2da`
- `additional/cls_st_ex_drd.2da`
- `additional/cls_st_jedi_c.2da`
- `additional/cls_st_jedi_g.2da`
- `additional/cls_st_jedi_s.2da`
- `additional/cls_st_minion.2da`
- `additional/cls_st_scndrl.2da`
- `additional/cls_st_scout.2da`
- `additional/cls_st_soldier.2da`
- `additional/exptable.2da`
- `additional/featgain.2da`
- `additional/xptable.2da`
- `exports.def`: DLL export list
- `hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
