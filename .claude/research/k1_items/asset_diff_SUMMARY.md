# K1 vs K2 resource diff (MDL, MDX, TPC, TGA, WOK)

K1 dir: `/home/ethansm/.local/share/Steam/steamapps/common/swkotor` (read-only). K2 dir: `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II` (read-only). Output: `/home/ethansm/orca/workspaces/local-setup/KOTOR-II/.claude/research/k1_items`.

## Counts per type x status

| type | only_k1 | only_k2 | both_same_hash | both_different | total |
| --- | ---: | ---: | ---: | ---: | ---: |
| MDL | 546 | 2478 | 0 | 2286 | 5310 |
| MDX | 546 | 2479 | 319 | 1967 | 5311 |
| TPC | 1244 | 7497 | 1937 | 1683 | 12361 |
| TGA | 1607 | 3998 | 3994 | 0 | 9599 |
| WOK | 332 | 1419 | 137 | 733 | 2621 |
| total | 4275 | 17871 | 6387 | 6669 | 35202 |

## Focus families x status (all types)

| family | only_k1 | only_k2 | both_same_hash | both_different | total |
| --- | ---: | ---: | ---: | ---: | ---: |
| i_ | 135 | 81 | 2 | 422 | 640 |
| iw | 0 | 284 | 39 | 108 | 431 |
| w | 0 | 180 | 61 | 319 | 560 |
| a | 0 | 1 | 0 | 6 | 7 |
| p | 45 | 246 | 4 | 32 | 327 |
| icon | 4 | 1613 | 66 | 305 | 1988 |
| total | 184 | 2405 | 172 | 1192 | 3953 |

## Primary copy group x status (which container supplies each side's primary)

| side / group | only_k1 | only_k2 | both_same_hash | both_different | total |
| --- | ---: | ---: | ---: | ---: | ---: |
| K1 OVERRIDE | 0 | 0 | 0 | 0 | 0 |
| K1 CHITIN | 3031 | 0 | 4450 | 4986 | 12467 |
| K1 MODULE | 0 | 0 | 0 | 0 | 0 |
| K1 RIMS | 0 | 0 | 0 | 0 | 0 |
| K1 TEXPACK | 1244 | 0 | 1937 | 1683 | 4864 |
| K2 OVERRIDE | 0 | 8019 | 5733 | 4402 | 18154 |
| K2 CHITIN | 0 | 8376 | 179 | 1559 | 10114 |
| K2 MODULE | 0 | 119 | 78 | 82 | 279 |
| K2 RIMS | 0 | 0 | 0 | 0 | 0 |
| K2 TEXPACK | 0 | 1357 | 397 | 626 | 2380 |

## Output files

- asset_diff_all.tsv: 35202 rows, 6264672 bytes, written (under 20971520 bytes).
- asset_diff_focus.tsv: 3781 rows (focus families, status other than both_same_hash)
- asset_diff_only_k1.tsv: 4275 rows (all families)
- asset_diff_mdl_different.tsv: 2286 rows (MDL both_different)

## Containers per game (copies by group x type)

| game | group | MDL | MDX | TPC | TGA | WOK |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| K1 | OVERRIDE | 0 | 0 | 0 | 0 | 0 |
| K1 | CHITIN | 2832 | 2832 | 0 | 5601 | 1202 |
| K1 | MODULE | 0 | 0 | 0 | 0 | 0 |
| K1 | RIMS | 239 | 252 | 0 | 0 | 0 |
| K1 | TEXPACK | 0 | 0 | 11452 | 0 | 0 |
| K2 | OVERRIDE | 2118 | 2117 | 4950 | 7898 | 1071 |
| K2 | CHITIN | 3237 | 3237 | 3801 | 10 | 1236 |
| K2 | MODULE | 201 | 201 | 73 | 99 | 362 |
| K2 | RIMS | 0 | 0 | 0 | 0 | 0 |
| K2 | TEXPACK | 0 | 0 | 11770 | 0 | 0 |

- K1: module files 234 (symlink case-variants collapsed 0, failed to open 0); rims/ files 12 (failed 0); TexturePacks 4 (failed 0).
- K2: module files 462 (symlink case-variants collapsed 822, failed to open 0); rims/ files 0 (failed 0); TexturePacks 4 (failed 0).

## Parse failures and unreadable resources (counts)

- K1 MDL parse: yes=2788, no-mdx=0, parse-fail=44
- K1 copies whose bytes could not be read: 0; resources with no readable copy (excluded from the diff): 0
- K1 container open failures: modules 0, rims 0, texture packs 0
- K2 MDL parse: yes=4739, no-mdx=0, parse-fail=25
- K2 copies whose bytes could not be read: 0; resources with no readable copy (excluded from the diff): 0
- K2 container open failures: modules 0, rims 0, texture packs 0

## Content conflicts (resources whose readable copies differ; primary chosen by precedence)

| game | MDL | MDX | TPC | TGA | WOK |
| --- | ---: | ---: | ---: | ---: | ---: |
| K1 | 0 | 0 | 2724 | 0 | 0 |
| K2 | 608 | 636 | 3416 | 2 | 214 |

## MDL both_different (texture lists)

- MDL both_different: 2286; texture lists compared (both parses yes): 2257; not compared (a parse failed or no MDX): 29
- with names only in K1: 196; with names only in K2: 169

## Rules and assumptions

- Containers and primary copy: see the module docstring. Copies that cannot be read are skipped and counted; the primary is the first readable copy in precedence order.
- Keys are (type, resref lower-cased). A TPC and a TGA with the same name are separate resources.
- Status from the primary-copy sha1 of each game: only_k1 / only_k2 (absent in the other game's scan), both_same_hash, both_different. Hashes cover one resource's bytes, not its MDX/MDL pair.
- MDL: read_mdl(primary MDL, source_ext=primary MDX). `nodes` = every node in the tree. `mdl_textures` = sorted unique lower-cased texture_1 of mesh nodes (null and empty skipped). mdl_parse: yes, no-mdx (no MDX copy), parse-fail (read_mdl raised).
- mdl_tex_only_k1 / mdl_tex_only_k2: texture names only in one game, for MDL both_different rows where both parses are yes. Blank otherwise.
- Families, first match wins: i_ -> i_; iw_ -> iw; w_ -> w; a_ -> a; p_ -> p; ia_/ii_/ip_/if_ -> icon; else regex `^i[a-z]+_\d+` -> icon; else not focus. Matching is on the lower-cased resref.
- Literal icon regex `^i[a-z]+_\d+` matches distinct resrefs: K1 0, K2 6; the ia_/ii_/ip_/if_ prefixes are what the item icons actually use (see k1_item_inventory.py icon_rule).
- TEXPACK order: packs are taken in sorted name order. This is not checked against the engine's pack lookup order. K1 has 2724 TPC names whose copies in different packs differ in bytes, so the chosen primary may not be the copy the game loads.
- K2 has no rims/ folder (module rims live in modules/). K2 TGA copies: OVERRIDE 7898, CHITIN 10. K2 Override is not vanilla-only, so OVERRIDE primaries on the K2 side may be installed or imported files. Not verified per file.
- Output is sorted by type (MDL, MDX, TPC, TGA, WOK) then resref. No timestamps, so reruns on the same installs are byte-identical.
