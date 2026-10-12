# K1a: KOTOR 1 NPC model extraction (pykotor 2.3.12, stock venv)

Install: /home/ethansm/.local/share/Steam/steamapps/common/swkotor (read-only). `Installation(...).game()` returns K1 (printed as `GAME 1`).

## Resource count
- Unique MDL names (chitin + patch.erf + override): 2832
- Prefix n_ / c_ / p_: 164 (n_ 86, c_ 54, p_ 24)
- First 20 (sorted, all c_): c_bantha, c_bmspecdiff, c_brith, c_decal, c_dewback, c_drdassassin, c_drdastro, c_drdmkfour, c_drdmkone, c_drdmktwo, c_drdprobe, c_drdprot, c_drdsentry, c_drdspyder, c_drdwar, c_embmcube, c_embmsphere, c_emcube, c_emsphere, c_female

## Extracted (MDL bytes / MDX bytes), saved to K1a/
- p_bastilah: 144016 / 74784. Root P_BastilaH. Supermodel P_BastilaBB.
- p_carthh: 80038 / 53120. Root P_CarthH. Supermodel S_Female02.
- n_darthmalak: 2255820 / 300768. Root N_DarthMalak. Supermodel NULL (none).

## Supermodel chain (extracted to K1a/)
- p_bastilabb: 234646 / 189792. Supermodel S_Female03.
- s_female02: 906680 / 44152. read_mdl raises OSError "This operation would exceed the streams boundaries".
- s_female03: 1087950 / 44152. Same OSError.
- The S_Female headers are self-consistent (MDL length = file size - 12, MDX length matches), so the failure is in the pykotor parser, not truncated bytes.

## read_mdl game argument
- `read_mdl(source, offset=0, size=None, source_ext=None, offset_ext=0, size_ext=0, file_format=None) -> MDL` (mdl_auto.py:251).
- No game or version parameter exists. `file_format` defaults to None and is auto-detected from the bytes (binary MDL vs MDL_ASCII).

## Call pattern used
`inst = Installation(K1_PATH)`; `mdl = inst.resource(name, ResourceType.MDL).data`; `mdx = inst.resource(name, ResourceType.MDX).data`; `read_mdl(mdl, 0, None, mdx, 0, 0)`.

## Traps
- Installation writes about 43 MB of DEBUG to stdout (K1a/run.log). Filter with `rg -av DEBUG`.
- The MDL `supermodel` attribute (mdl_data.py:1790) gives the parent name. The chain can be two levels deep (p_bastilah -> P_BastilaBB -> S_Female03).
