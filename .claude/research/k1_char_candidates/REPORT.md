# K1 character import candidates (doc 38 follow-up, 2026-10-13)

Static, read-only. Nothing touched the game. Script: `profile_models.py` (same dir; run with `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python`). Goal: one isolated K1 character to import into K2 with model, texture, animation and audio.

## Method
1. Diffed K1 vs K2 `chitin.key` by resref: 1,706 K1-only MDLs, but 1,686 are area/minigame/stunt models (`m*`, `mgf_*`, `stunt*`). The only K1-only non-area character models are 19 party-member models and one creature.
2. Profiled each with pykotor (supermodel, node and animation counts, textures, K2 presence), then followed the dialog (`k_h*_dialog`) VO references into `streamwaves/`.

## K1-only character models (everything else in K2 under the same resref)
`p_bastilaba p_bastilabb02 p_candba p_candbb p_candh p_carthba p_carthbbh p_joleeba p_joleebb p_joleeh p_juhani p_juhaniba p_juhanibb p_juhanih p_missionba p_missionbb p_missionh p_t3m3 p_zaalbar` and creature `c_terantanak`.

## Profile of the shortlist
| Character | Model files | Verts | Own anims | Supermodel | Textures | Audio found |
|---|---|---|---|---|---|---|
| **Zaalbar** | `p_zaalbar` (1 MDL+MDX, head and body fused, 294/286 KB) | 6263 | 1 (`taunt`) | `N_WookieM` (exists in K2 and K1) | 1 (`p_zaalbar02`) | soundset `p_zaalbar_*` 29 wavs (chitin); 44 VO wavs under `streamwaves/<module>/zaal*/`; companion dialog `k_hzaa_dialog` 50 entries / 47 replies / 67 VO refs |
| Juhani | `p_juhani` body (76 nodes) + `p_juhanih` head (supermodel `P_JuhaniBB`, K1-only) + `p_juhaniba/bb` | 4040 + 1625 | 13 body, 2 head | body NULL, head `P_JuhaniBB` (K1 only) | `p_juhanibb01`, `p_juhanih01` | soundset 29 wavs; `k_hjuh_dialog` 1,272 entries+replies, 850 VO refs, 533 wavs on disk (20 MB) |
| Mission / Jolee / Canderous | head + 2 bodies each, all on K1-only `P_*BB` supermodels | n/a | 0-2 | K1-only | 1 per file | 134 / 348 / 213 VO wavs |
| T3-M3 (`p_t3m3`) | 1 MDL | 2206 | 67 | NULL | `p_t3m3_01` | 27 soundset wavs, identical in K2 |
| Terentatek (`c_terantanak`) | 1 MDL (532 KB) | n/a | 29 | NULL | `c_terantanak01` | 5 wavs (`c_terantank_*`) |

## Findings that change the ranking
- **T3-M3 is NOT a new asset.** K2 ships `p_t3m4` with the same 31 nodes and 2206 vertices (K2 has 71 anims vs K1's 67) and the same 27 soundset wavs. Use it as the pipeline control: any failure on T3-M3 is the pipeline, not the content.
- **Terentatek is already in this K2 install:** appearance row 353 and a `c_terentatek` UTC exist in K2 (Override), so it is K1-only only in vanilla. Not a clean test.
- The 2DA/UTC side is cheap for any of these; the doc 38 spawn tooling (`k1-spawn-probe`) already spawns arbitrary appearance rows natively.
- **Skin risk is shared:** Zaalbar, Juhani and every humanoid body are skinned. The pykotor writer still has the open `bones[16]` junk-slot defect (doc 38 section 11/12; `probe_skinfix` result pending). The first import must use the MDLOps K2 output route or the skinfix writer.

## Ranking
1. **Zaalbar** (primary). One model file, one texture, supermodel already in K2 so walking, running and combat animations come from K2's own `n_wookiem` skeleton (the generic NPC path every humanoid uses), a ready soundset, a compact companion dialog, and Wookiee growls so no English VO licensing question for the first audio test. Open checks: (a) bone names of K1 `N_WookieM` vs K2 `n_wookiem` (node count and names), (b) the K1 appearance row and `heads.2da` need (none: head is fused), (c) soundset 2DA row for K2 (`soundset.2da` index, ssf).
2. **Juhani** (phase 2, audio-heavy). Largest voice set, 13 own body animations, but four models and a K1-only head supermodel to port. Best test for the `kvoice` K1 VO path (K1 `streamwaves` MP3-in-WAV to K2 `StreamVoice` DLG subfolder, claims 11/12 in doc 38).
3. **T3-M3** as the control run before either.

## Audio facts for the import
- K1 VO lives in `streamwaves/<module>/<dlg-or-speaker>/NAME_.wav` (13,799 wavs, 82 top-level dirs). K1 soundset clips are in `chitin.key` BIFs (no loose file), so a K2 import needs them as loose Override or StreamSounds wavs. Whether K2 plays a loose Override wav through `PlaySound` is exactly the pending E1 result.
- The dialog VO_ResRef values carry a trailing underscore (for example `nglobezaal17066_`). Resolve by file stem, case-insensitive, never by a recursive follow-symlinks scan.

## Other compatible games (future track, UNVERIFIED)
No research done yet beyond the K1 install. Candidates to evaluate: Neverwinter Nights 1 (Aurora; the K1 MDL family descends from its model format; textures TGA/DDS/PLT, audio WAV/BMU); Jade Empire (Odyssey derivative per xoreos coverage, formats modified); NWN2 and Dragon Age use a different model format and are incompatible. Check which of these the user owns before any recon, and run a Haiku format-compatibility recon first.

## Next step (needs user decision)
Plan `38:T-char-import`: (1) pipeline control run with `p_t3m3` mapped to the K2 side, (2) Zaalbar import with the MDLOps K2 output route, (3) E1/E2 results decide the audio delivery route for the soundset and VO.
