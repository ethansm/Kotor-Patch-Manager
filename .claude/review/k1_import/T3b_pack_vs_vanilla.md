# T3b: Upscale+ pack vs vanilla K1, four models

Pack: `Upscale+ Character Fixes - KotOR V0.52 (4x tpc).7z` (untrusted). Only 10 entries extracted to `~/Games/KOTOR2_Mod_Workspace/k1_port_extract/upscale_plus/Copy contents to KotOR's Override folder/`.
Vanilla: K1 chitin only (pykotor 2.3.12, `SearchLocation.CHITIN`), read-only. Override folder has no copies of these names.

## Per model (pack vs vanilla)

| model | MDL bytes | MDX bytes | node tree | verts / faces | texture_1 / texture_2 | UV1 changed (max abs delta) |
|---|---|---|---|---|---|---|
| N_CommM | identical | differs | same (59 nodes) | same / same | same (N_CommM04), tex2 empty | yes: ArmL 0.021608 (23/228 verts), ArmR 0.021863 (23/228) |
| N_CommF | differs (24 bytes, 8 runs) | differs | same (64 nodes) | same / same | same (N_CommF01) | yes: torso 0.039167 (29/581), ArmL 0.033576 (29/217), ArmR 0.033774 (29/217) |
| N_CommKidM | identical | differs | same (77 nodes) | same / same | same (N_child_M, N_CommKidM01, PMBF) | yes: torso2 0.445310 (22/600; 8 verts >0.05), RArm 0.021339, ArmL2 0.021658 |
| N_CzerkaOff | identical | differs | same (60 nodes) | same / same | same (N_CzerkaOff01) | yes: Torso 0.082913 (40/632), rarm 0.094841 (58/304), larm 0.094157 (52/262) |

- Face indices, materials, adjacency: identical on all four.
- Vertex normals: identical on all four. Vertex positions: identical on N_CommM, N_CommKidM, N_CzerkaOff.
- N_CommF only: ArmL and ArmR positions move (max 0.013018); 22 faces (11 per arm) differ only in pykotor's face `normal` field (max component delta 0.127067); all other face fields identical.
- Bounding box, radius, average: identical on all four.

## Textures

| file | pack | vanilla (K1 `TexturePacks/swpc_tex_tpa.erf`) |
|---|---|---|
| N_CommM04.tpc | 2048x2048, DXT5, 5,592,560 B | 512x512, DXT1, 174,904 B |
| N_CommF01.tpc | 2048x2048, DXT5, 5,592,560 B | 512x512, DXT1, 174,904 B |

- Both vanilla textures exist in the install. The pack is 4x linear, about 32x bytes, and DXT1 becomes DXT5.
- Pack N_CommM04 and N_CommF01 have equal size but different bytes.

## Base choice

N_CommM: use the pack. Its geometry and topology match vanilla, and its only model change is the UV1 edit on ArmL and ArmR. Vanilla lacks that edit.

## Caveats

- Checked with pykotor parsing, not in-game. Verdict covers data only.
- Scratch scripts and outputs: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/4b302fe0-554c-4823-af59-025f7d920a16/scratchpad/T3/` (`compare_clean.txt`, `deep_out.txt`, `hash_and_tpc.py`).
