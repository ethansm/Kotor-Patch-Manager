# N1 NPC diversity vs candidate K1/TSL sources (recon)

Q1. Shipped assets: patch_manager_mods/npc_diversity_src/override = 106 files (over 40, names not listed). manifest.toml:18-20 ships no assets (sources, exports, hooks only).
- .tpc 76 (N_CommM/N_CommF 35 each, N_CzerkaOff01, N_CzerkaOffC01, llk_* x5)
- .mdl 13, .mdx 13 (N_FatComM/F, N_CommM_Fat/F_Fat, N_TwiDance, N_TwiDance_Boot, N_TwiUnd, N_CzerkaOff, llk_ahrnell/dahnish/harra/kehvehn/luposhar)
- .2da 2 (appearance.2da, heads.2da), .tga 1 (N_CommM10.tga), .json 1 (diversity_2da_mapping.json)

Q2. Candidate IDs, author names, K1/Telos/Lite in npc_diversity_src (incl. override text): None.
- Only hit is a false positive: DiversityExclusion.h:76 ("literal" contains "lite").
- No hit for 2179, 1392, 1414, 2246, 3053, 1407, 1330, Leilukin, Effix, SpaceAlex, JasonRyder, N-DReW, Malkior, Ferc, Lily.

Q3. Inputs from outside the repo:
- scripts/normalize_diversity_textures.py:28-35 reads ~/Games/KOTOR2_Mod_Workspace/01_extracted/{nexus_mod144_uco_redux, mod149_character_textures_2x, extra_fashion_line_b, mod41_twilek_diversity, extra_hd_female_twileks, mod37_head_fixes}. Writes to override (:38).
- scripts/generate_diversity_2da.py:49-51 reads the live Steam override (~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/override). :12 prefers <override>/_diversity_premerge_backup/*.2da.bak.
- README.md:38 confirms the 2DA generator appends to the live appearance.2da and heads.2da.
- No dir in repo maps these folder names to DeadlyStream IDs.

Q4. patch_manager_mods/34_npc_diversity_dispatcher.md:110 says only: "Background diversity pool for refugees and generic planetary citizens." This is a role description, not a usage claim. No SpaceAlex dir in normalize:28-35 and no SpaceAlex-named file.

Q5. Overlap with candidate list:
- Probable match, name-level only: N_TwiDance, N_TwiDance_Boot, N_TwiUnd (+ female Twi'lek textures). Matches #3053 title. Source dir extra_hd_female_twileks (normalize:33). Doc 34:112 credits Lily and links #2242, so the ID is unresolved.
- Match on author: llk_ahrnell, llk_harra, llk_kehvehn, llk_luposhar (normalize:11). Leilukin ID conflict: doc 34:111 says #2242, doc 38:27 says #2179.
- Not shipped: #1414, #1392, #1407, #1330, #2246. No file or reference found.
- Stale note: doc 34:268 says llk_*.mdx is not in source, but 5 llk_*.mdx files are present.
- Unknown: mod41_twilek_diversity (normalize:32) has no recorded ID.
