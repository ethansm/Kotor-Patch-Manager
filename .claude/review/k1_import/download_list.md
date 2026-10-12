# K1 → K2 import: download and install list

**Next action:** download [HD Carth Onasi (DeadlyStream #1133)](https://deadlystream.com/files/file/1133-hd-carth-onasi/) into an empty folder and send me the path.

Context: [doc 38](../../../patch_manager_mods/38_k1_to_k2_asset_import.md) · [K1c vanilla names](K1c_vanilla_names.md) · [R4 Neo Cities](R4_neo_cities.md)

## 1. Already installed (no action)

| Item | Location | Notes |
|---|---|---|
| KOTOR 1 base game | [swkotor](file:///home/ethansm/.local/share/Steam/steamapps/common/swkotor) | Steam: fully downloaded |
| KOTOR 2 base game | [Knights of the Old Republic II](file:///home/ethansm/.local/share/Steam/steamapps/common/Knights%20of%20the%20Old%20Republic%20II) | Target game |
| Proton 9.0 (Beta) | [folder](file:///home/ethansm/.local/share/Steam/steamapps/common/Proton%209.0%20%28Beta%29) | Headless run unverified |
| Proton - Experimental | [folder](file:///home/ethansm/.local/share/Steam/steamapps/common/Proton%20-%20Experimental) | Headless run unverified |
| Project venv (pykotor 2.3.12) | [.venv](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/.venv) | Stock build: K2 write bug |

## 2. Tools to download or set up

| Tool | Why | Status | Link |
|---|---|---|---|
| MDLOps (`mdlops.exe`) | Headless K1→K2 route (Track A) | 403 for us; download in a browser | [DeadlyStream #779](https://deadlystream.com/files/file/779-mdlops/) |
| tga2tpc | Upscaled TGA → TPC (skip if pykotor writes TPC) | Not downloaded | [DeadlyStream #1152](https://deadlystream.com/files/file/1152-tga2tpc/) |
| Patched pykotor | K2 writer fix (8-byte) and reader fix | Diff ready; `pykotor_patches/` is missing | [fix diff](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/.claude/review/k1_import/B2a_pk_fix.diff) |
| xoreos-tools | GFF and dialog work (later) | Optional | [GitHub](https://github.com/xoreos/xoreos-tools) |
| Perl 5.12+ | Only for the uncompiled MDLOps | Not needed for `mdlops.exe` | [perl.org](https://www.perl.org/get.html) |

## 3. K1 texture packs (get author permission before shipping)

| Pack | Why | Status | Link |
|---|---|---|---|
| HD Carth Onasi | Direct match for the `p_carthh` proof of concept | Priority 1; not downloaded | [DeadlyStream #1133](https://deadlystream.com/files/file/1133-hd-carth-onasi/) |
| Redrob41 Character Textures | Broadest NPC coverage, including aliens; K1 coverage partial | Not downloaded | [DeadlyStream #2659](https://deadlystream.com/files/file/2659-4x-upscale-character-textures-model-fixes/) |
| Red11BY Ultimate Character Overhaul | 2048 AI upscale of every character; needs Nexus login | 403 for us | [Nexus #1282](https://www.nexusmods.com/kotor/mods/1282) |
| Male NPC Clothing | Commoner clothing with vanilla numbered names | Not downloaded | [DeadlyStream #2516](https://deadlystream.com/files/file/2516-npc-clothing-m/) |
| Laast's Characters Revisited | Full retexture of player and NPC models | Optional; 403 for us | [Nexus #1639](https://www.nexusmods.com/kotor/mods/1639) |

## 4. Optional reference sources

| Item | Why | Link |
|---|---|---|
| Leilukin K1 Twi'lek pack | K1 Twi'lek models; credit required; ID conflict (#2179 vs #2242) | [DeadlyStream #2179](https://deadlystream.com/files/file/2179-kotor-1-twilek-female-npc-diversity/) |
| Effixian K1→K2 heads | Ships TSLPatcher installers; reference only | [DeadlyStream #1392](https://deadlystream.com/files/file/1392-effixians-k1-to-k2-heads) |
| Ferc Kast K1 Heads Port | Head-hook reference | [DeadlyStream #1407](https://deadlystream.com/files/file/1407-k1-heads-port/) |
| Malkior Texture-to-Game Bridge | K1 texture port reference; license unknown | [DeadlyStream #1330](https://deadlystream.com/files/file/1330-kotor-1-texture-to-kotor-2-game-bridge/) |
| Neo Cities K1 build list | Browse only; about 190 mods | [kotor.neocities.org](https://kotor.neocities.org/modding/mod_builds/k1/spoiler-free) |

## 5. Excluded

| Item | Reason | Link |
|---|---|---|
| Selphadur Kotor Texture Redux | Personal use only; cannot ship in the patch | [Nexus #1302](https://www.nexusmods.com/kotor/mods/1302) |

## 6. Known gaps

- Nothing listed in sections 2 to 4 has been downloaded or opened, so every pack's file names are unverified.
- Nexus and ModDB return 403 to our fetcher. Those need a browser download.
- No K1 pack states a license. Shipping any of them needs the author's permission.
- The Leilukin pack ID is recorded two ways (#2179 in doc 38, #2242 in doc 34). Doc 34 also has a stale note about `llk_*.mdx`.
- Links came from research evidence. They have not all been re-fetched.
