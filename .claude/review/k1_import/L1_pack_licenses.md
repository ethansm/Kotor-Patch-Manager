# L1: K1/NPC pack licenses (2026-10-11)
Method: listings via `unrar lb`, py7zr `getnames()`, zipfile. Text files only, extracted to `~/Games/KOTOR2_Mod_Workspace/k1_port_extract/readmes/<id>/`. No executables run. ~/Downloads read-only. Quotes checked against the extracted files.
Flags: `ok_credit` = use with credit. `approval_needed` = hold until author says yes. `unlicensed_remove_candidate` = no grant found. `ok_gpl_tool` = tool, not an asset.

## Summary
| id | license_class | approval | tool flag | credit to give |
|---|---|---|---|---|
| upscale_plus_v052 | permission_required | yes (redistribution) | approval_needed | RedRob41 + title/version statement |
| twilek_f_npc_diversity | credit_required | no (SpaceAlex K1EP terms unverified) | ok_credit | Leilukin; SpaceAlex; DarthParametric |
| carth_onasi | unknown | unknown | unlicensed_remove_candidate | unknown |
| npc_clothing_m | unknown | unknown | unlicensed_remove_candidate | unknown |
| effixians_k1_to_k2_heads | permission_required | yes ("ask me") | approval_needed | Effix(ian) |
| k1_heads_port_101 | permission_required | yes | approval_needed | Ferc Kast |
| kotor2_to_kotor1_game_bridge | unknown | unknown | unlicensed_remove_candidate | none stated |
| uco_redux_tpc_1282 | unknown | unknown (assume yes) | unlicensed_remove_candidate | Red11BY (per DSOGaming) |
| characters_revisited_creatures_addon | unknown | unknown | unlicensed_remove_candidate | Laast (snippet, unverified) |
| characters_revisited_updates | unknown | unknown | unlicensed_remove_candidate | Laast (snippet, unverified) |
| mdlops_100 | open | no | ok_gpl_tool | Chuck Chargin Jr. (courtesy) |

## Per pack
### 1. upscale_plus_v052 (`Upscale+ Character Fixes - KotOR V0.52 (4x tpc).7z`)
- Readme: `readmes/upscale_plus_v052/Read Me - 4x Upscale+ Fixes V0_52 for KotOR.txt`, "Permission:" (lines 113-121). Author: RedRob41.
- Quotes: "This mod may NOT be uploaded or redistributed, in whole or in part, to any mod hosting site without my explicit permission." / "as long as their creation remains free to use (not for profit); and credit is given to me as the source of any modded files."
- Contains: N_CommM, N_CommF, N_CommKidM, N_CzerkaOff (.mdl/.mdx + .tpc); 190 models total; no llk_*.

### 2. twilek_f_npc_diversity (`K1 Twi'lek Female NPC Diversity.7z`)
- Readme: `readmes/twilek_f_npc_diversity/KotOR 1 Twi'lek Female NPC Diversity/READ ME.txt`, section 6 (line 74). Author: Leilukin. Also credited: SpaceAlex (K1EP textures), DarthParametric (optional boots).
- Quote: "You may use assets from this mod as part of your mod, however, both SpaceAlex and I should be credited accordingly".
- Contains: llk_CasandraB/H, llk_LenaB/H, llk_MalareB/H, llk_ZeenaB/H, llk_ZoriisB/H (10 models, .tpc, .utc); no N_* resrefs.

### 3. carth_onasi (`Carth Onasi.rar`)
- No text in archive; author not in archive. Page not collected (download_list.md line 31, DS #1133). License text: no license text found.
- Contains: P_CarthH01 head, P_CarthBA01, P_CarthBB01, PO_pcarth portrait. Textures only.

### 4. npc_clothing_m (`NPC clothing M.rar`)
- No text in archive; author not in archive. Page not collected (download_list.md line 34, DS #2516). License text: no license text found.
- Contains: N_CommM01-08, N_CommMD01, N_CommM0201/0301/0401/0501/0601/0801. Textures only.

### 5. effixians_k1_to_k2_heads (`Effixians_K1_to_K2_Heads.zip`)
- Readmes: `readmes/effixians_k1_to_k2_heads/*__readme.txt`, "DISTRIBUTION NOTES" (lines 50-53). Author: Effix(ian).
- Quotes: "You may not: Use the material from this mod in your own mod without giving me credit, implying it was your creation." / "You might: Use the material from this mod in your own mod if you simply ask me."
- Contains: PFHC01 as pfhx01.mdl/.mdx + PFHX01*.tga; PMHA05 as pmhx05.mdl/.mdx + PMHX05*.tga. Converted models ship. No N_* or llk_*.

### 6. k1_heads_port_101 (`k1-heads-port-1.0.1.zip`)
- Readme: `readmes/k1_heads_port_101/README.txt`, PERMISSIONS (line 419). Author: Ferc Kast. Credits also: Fred Tetra (KOTOR Tool), Stoffe (TSLPatcher).
- Quote: "You may not redistribute this mod on other sites without my permission. You may use this mod in your own if you ask me."
- Contains: 30 head models k1_pfha/pfhb/pfhc/pmha/pmhb/pmhc 01-05 (+ .tpc, d-variants). No N_* or llk_*.

### 7. kotor2_to_kotor1_game_bridge (`Kotor 2 to Kotor 1 Game Bridge.rar`)
- Readme: `readmes/kotor2_to_kotor1_game_bridge/Readme.txt`, "Permissions:". Author not named. download_list.md line 44 links DS #1330 "Malkior ... license unknown" (match not verified).
- Quote: "These obviously aren't my files, they are straight from my Kotor game, and as such are property of whoever currently owns them."
- Contains: 40 C_* creature textures (C_Drd*, C_Hutt01-04, C_Khounda*, C_Kinrath01-04, C_Brith01). No models.

### 8. uco_redux_tpc_1282 (`Ultimate Character Overhaul -REDUX- ( FULL ) - TPC Version-1282-4-1-1628547676.rar`, 1.2 GB)
- Listed only (771 .tpc, no text entries; nothing extracted). Author: Red11BY (R2_k1_tex_other_hosts.md line 12, VERIFIED via DSOGaming).
- License text: none in archive. R2 line 14: "VERIFIED none in DSOGaming article. Nexus terms UNVERIFIED." Nexus #1282 returns 403.
- Contains: N_CommM01-08, N_CommF01-09, N_CommKidM01-02, N_CommKidF01-02, N_CzerkaOff01, N_CzerkaOffC01 (textures only), C_* textures. No models.

### 9. characters_revisited_creatures_addon (`Characters Revisited_Creatures_Addon-1639-1-0-1716349899.rar`)
- 45 entries, all tga/txi under `creatures/Override/`; no text. Author: Laast (R2 line 27, snippet, UNVERIFIED). License text: none found (R2 line 32, "None seen").
- Contains: 40 C_* creature textures (C_bantha01 to C_Wraid04). No models, no N_* resrefs.

### 10. characters_revisited_updates (`2_Characters Revisited_Updates-1639-Up-07-1723138875.rar`)
- 26 entries under `update/Override/`; no text. Author: Laast (as pack 9). License text: none found (R2 line 32).
- Contains: PFBI01-04, PFBJ02, PMBJ02, PMBE07, PMHC01 (+d1-d3), pmbam01, pmbama01, pmbamB01, P_CandH01, czerka_com_h.tga (textures); c_drdwar.mdl/.mdx/.utc. No N_* resrefs.

### 11. mdlops_100 (`mdlops-1.0.0.7z`, tool)
- Extracted: readme_MDLOps_1-0.txt, GPL.txt, changelog.txt, tutorials. Not extracted: mdlops.exe/.pl/.pm. Author: Chuck Chargin Jr.
- Quotes: "This script is released under the GPL, see the included GPL.txt." / "This script may only be hosted from sites that do not claim ownership of files they host."
- GPL governs redistributing mdlops itself, not its output. The "send me an e-mail" request is a courtesy, not an approval.

## Open
- Does pushing converted Upscale+ textures to this public repo count as "redistribution to a mod hosting site" (needs RedRob41's approval)?
