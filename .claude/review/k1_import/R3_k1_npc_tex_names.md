# R3 K1 NPC texture names (recon, 2026-10-11)

Q1 K1 uses N_Comm* names, not K2-only: UNVERIFIED. No page found lists K1 N_Comm names. ModdingWiki Appearance.2da blocked (Anubis bot page). Hint only (fetch excerpt, not verbatim): https://deadlystream.com/files/file/2618-commoners-texture-pack-v10/ says TSL had commoner textures "exclusive to this game".

Q2 K1 vanilla p_carthh, p_bastilah, n_darthmalak exist: UNVERIFIED. No page found names them; "p_carthh" search returned only Kotor (Montenegro) pages. Head thread https://deadlystream.com/topic/7135-k1-heads-question/ names only P_MissionH01 and P_MissionBB.

Q3 HD or upscaled K1 pack keeps vanilla names: UNVERIFIED on names. Pack exists: https://www.dsogaming.com/?p=127399 (K1, Red11BY, every NPC plus armor, upscaled to 2048x2048, "drop the .tga files into the Override folder"). Page does not say names are kept. Nexus mods/1639 returned HTTP 403.

Q4 TPC from TGA keeps mip structure: VERIFIED, with caveat. TPC is a container: "A proprietary BioWare wrapper around native DXT-compressed OpenGL texture data" (https://synchro.codeberg.page/rakata/formats/textures/index.html). tga2tpc: "Bicubic downsampling for mipmaps (precomputed lower detail versions of the texture, part of the TPC format)" and "can be uncompressed or compressed with either DXT1 or DXT5" (https://deadlystream.com/files/file/1152-tga2tpc/). Mips are regenerated from the source, not preserved, and DXT is lossy.

Overall: vanilla K1 name list not found; K1 HD drop-in exists but name-preservation unstated.
