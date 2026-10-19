# h7: K1 Mission Vao and Zaalbar models, supermodel resolution (recon, read-only)

## Verdict
PARTIAL. The four K1 models parse cleanly and their structure is confirmed. Two premises need correcting:
1. `P_MissionBB` is not K1-only. The K2 install's `Override/` folder holds `p_missionbb.mdl` (plus `p_missionba`, `p_missionh`, `p_zaalbar`). These are non-vanilla copies dated 2026-04-17. No vanilla K2 BIF or chitin copy exists.
2. No automatic K1 to K2 supermodel rename exists in the importer. The name is kept verbatim. Only a recipe's `--supermodel-map` rewrites it. Zaalbar's `N_WookieM` has no recipe mapping and resolves by name to vanilla K2 `n_wookiem`.

## Inputs and method
- K1 bytes: read with pykotor (`k1port` venv) from `swkotor/data/party.bif` (all four). Copies in `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/885aae19-5eea-47ec-b2fb-cd9b10afd222/scratchpad/h7/k1mdl/`.
- Parse: stdlib `struct` only, offsets from `h6c_tree_quirks_offsets.md`. Script: `.../scratchpad/h7/parse_mdl.py`. Output: `.../scratchpad/h7/k1_parse.json`.
  - Node header 80 B. Name index at +4, supernode at +2, children at +44 and +48, all raw and relative to file byte 12.
  - Vertex count = node abs + 80 + 304 (pykotor `_TrimeshHeader`, field `vertex_count`).
  - Dangly test: `type & 0x100`. Skin test: `type & 0x40`.
- Cross-check: pykotor `read_mdl` on the same bytes (`xcheck_pk.py`). Node count, names, vertex counts and supermodel match for all four.
- Chain check (h6c rule R4): `geometry total (file 56) = own nodes + supermodel total + 1`. Holds for K1 and K2 chitin copies.
- K2 vanilla: pykotor `CHITIN` + `MODULES` lookup, bypassing `Override`. Copies in `.../scratchpad/h7/k2chitin/` and `.../scratchpad/h7/k2fam/`.

## 1. K1 models (p_missionba, p_missionbb, p_missionh, p_zaalbar)

| K1 file | Model name / root node | Own nodes (walked) | Chain total (file 56) | Node types (flag: count) | Dangly (0x0100) | Skin (0x0040) | Anims | Supermodel (file 148) |
|---|---|---|---|---|---|---|---|---|
| p_missionba.mdl | P_MissionBA / P_MissionBA | 63 | 571 | 0x0001: 17, 0x0021: 43, 0x0061: 3 | 0 | 3 | 0 | `P_MissionBB` |
| p_missionbb.mdl | P_MissionBB / P_MissionBB | 90 | 507 | 0x0001: 21, 0x0021: 66, 0x0061: 3 | 0 | 3 | 0 | `S_Female03` |
| p_missionh.mdl | P_MissionH / P_MissionH | 36 | 544 | 0x0001: 9, 0x0021: 24, 0x0061: 3 | 0 | 3 | 2 (`pause1`, `talk`) | `P_MissionBB` |
| p_zaalbar.mdl | P_Zaalbar / P_Zaalbar | 81 | 497 | 0x0001: 20, 0x0021: 56, 0x0061: 5 | 0 | 5 | 1 (`taunt`) | `N_WookieM` |

No aabb (0x0221) or saber (0x0821) nodes. Node counts match between raw parse and pykotor.

### Skin nodes (name, vertex count)

| Model | Skin nodes |
|---|---|
| p_missionba | Larm 235, Rarm 235, torso 617 |
| p_missionbb | LArm 284, torso 936, RArm 269 |
| p_missionh | tongue 22, hair 266, head 446 |
| p_zaalbar | Arms 520, torso 1674, head 473, Feet 240, tongue 30 |

Dangly: none in any of the four. p_missionh has no node named for lekku. The lekku, if any, sit inside the skin meshes `head` (446 v) or `hair` (266 v). I did not check this from the geometry.

## 2. Supermodel resolution

Rule set used:
- The importer keeps the header supermodel name unless told otherwise. `load_supermodel` in `k1_port_model.py:604` looks the name up in the pack dir, then the K1 install, and never renames it.
- `--supermodel-map OLD=NEW` (`k1_port_model.py:683-749`) is the only rewrite. Recipes supply it. In `k1_port_import.py:1348` and `:1437`, `sm = [body_b.k1=body_b.k2]` is applied to every model except body_b. So body_a and the head get the rewrite. body_b keeps its own supermodel.
- The MDLOps path (`k1_port_mdlops.py:255-288`) resolves K2 supermodels with `k2_model_bytes`. It checks `Override/` first, then pykotor for the whole install.

### Presence of each supermodel name

| Supermodel name | K1 install | K2 vanilla (chitin/modules) | K2 `Override/` | Own supermodel | Notes |
|---|---|---|---|---|---|
| `P_MissionBB` | yes (party.bif, `p_missionbb`) | **no** | yes: `p_missionbb.mdl` 234486 B, dated 2026-04-17 | `S_Female03` | K1 data re-saved in K2 format (+552 B = 69 mesh nodes x 8). Chain total 616 (K2 Override). |
| `S_Female03` | yes (models.bif) | yes (models.bif, 1666902 B, 68 anims, total 525) | yes: 1711966 B, 70 anims, **shadows vanilla** | `S_Female02` | Override copy differs from vanilla (md5 and anim count). |
| `N_WookieM` | yes (models.bif, total 415) | yes (models.bif, 519804 B, total 506) | yes: 519804 B, same size, **md5 differs** from vanilla | `S_Female02` | Override shadows vanilla. |

### Result per model (no recipe, and with the recipe-style map)

| Model | Name in header | Under current code (no map) | Under recipe-style map (body_b = p_missionbb -> k1p_missionbb) |
|---|---|---|---|
| p_missionba (body_a) | `P_MissionBB` | Resolves by name to K2 Override `p_missionbb` (non-vanilla). No vanilla K2 model with that name. | `k1p_missionbb` (converted body_b, does not exist yet). |
| p_missionbb (body_b) | `S_Female03` | Resolves to vanilla K2 `s_female03` (chitin), shadowed by Override copy. | Unchanged (body_b is not remapped). |
| p_missionh (head) | `P_MissionBB` | Same as body_a. | `k1p_missionbb`. |
| p_zaalbar (single model) | `N_WookieM` | Resolves to vanilla K2 `n_wookiem` (chitin, models.bif). No rewrite needed. | No recipe exists. Same as no map. |

**Answer:** no automatic mapping exists. Mission's body resolves to `P_MissionBB` by name, which in K2 is a non-vanilla Override file. The only way to point it at a converted copy is a recipe `--supermodel-map`. Zaalbar's `N_WookieM` resolves to vanilla K2 `n_wookiem`.

### Supernode numbering (not a name issue, but it matters for both)
- K1 p_zaalbar: chain total 497 = 81 + 415 + 1, using the K1 `N_WookieM` total of 415.
- K2 `N_WookieM` (vanilla): total 506. Zaalbar would need 81 + 506 + 1 = 588, which is what K2 Override `p_zaalbar.mdl` already stores.
- K1 p_missionbb: 507 = 90 + 416 + 1, using the K1 `S_Female03` total of 416. K2 has 616 = 90 + 525 + 1 (K2 Override `p_missionbb`).
- So a K1-numbered Zaalbar or Mission model would hit the same chain-number mismatch that Juhani hit (doc 38 s22). Renumbering against the K2 chain is required, via MDLOps pass2 or `k1_port_mdl_supernode.py`.

### Anomaly: K2 Override p_missionh
- K2 Override `p_missionh.mdl` stores file 56 = 36 (own nodes only, no chain total). Its supermodel `P_MissionBB` has a K2 total of 616, so the expected total is 653.
- The other three K2 Override copies (BA 680, BB 616, Zaalbar 588) are consistent with their chains. Only the head is not.

## 3. K2 vanilla Wookiee and Twi'lek models (chitin/modules copies)

Own nodes, skin count, dangly count, anim count, supermodel:

| Name | Supermodel | Own nodes | Skin | Dangly | Anims |
|---|---|---|---|---|---|
| n_wookiem (N_WookieM) | S_Female02 | 82 | 5 | 3 | 19 |
| n_wookief (N_WookieF) | S_Female03 | 86 | 5 | 1 | 9 |
| l_wookie (L_Wookie) | N_WookieM | 64 | 1 | 0 | 0 |
| l_wookief (L_WookieF) | N_WookieF | 76 | 1 | 1 | 0 |
| p_hanharr (P_Hanharr) | S_Female02 | 82 | 5 | 3 | 22 |
| plc_wookcrps (PLC_WookCrps) | NULL | 2 | 0 | 0 | 0 |
| n_twilekf (N_TwilekF) | S_Female03 | 87 | 4 | 0 | 2 |
| n_twilekm (N_TwilekM) | S_Female02 | 83 | 3 | 0 | 0 |
| l_twilekf (L_TwilekF) | N_TwilekF | 73 | 1 | 0 | 0 |
| l_twilekm (L_TwilekM) | N_TwilekM | 72 | 1 | 0 | 0 |
| twilek_f / f02 / f03 / f04 | N_TwilekF | 33 each | 3 | 0 | 0 |
| twilek_m / m02 / m03 | N_TwilekM | 33 each | 3 | 0 | 0 |
| n_twiassh01 / h02 | S_Female03 | 33 each | 3 | 0 | 0 |
| n_twiassasin | S_Female03 | 62 | 4 | 0 | 0 |
| n_contestwin | S_Female02 | 34 | 2 | 0 | 0 |
| n_jedimalek | S_Male02 | 87 | 5 | 2 | 0 |

Override-only names (no chitin copy): `p_missionba`, `p_missionbb`, `p_missionh`, `p_zaalbar`, `p_twih1..7` (several), `p_twihpc*`, `p_twihass*`, `sm_twifem`, `twidanceblue`, `n_twidance*`, `n_twiund*`, `pfbltwi`. No K2 vanilla model contains `zaal`.

## 4. Other findings
- Override shadowing: K2 `Override/` shadows several vanilla names. `s_female03` (70 anims vs 68), `s_female02`, `s_female01`, `n_wookiem`, `p_hanharr`, `twilek_f` all differ in md5 from the chitin copies. `n_twilekf` and `l_wookie` are identical. Override folder mtimes are 2026-09-24. Cause not checked.
- The Mission and Zaalbar Override copies come with `p_zaalbar.utc`, `p_zaalbar.ssf`, and Mission textures (`p_missionba01.tpc`, `p_missionbb01.tpc`, all dated 2026-04-17).
- Importer collision guard: the new names (`k1p_*`) are not affected. Only raw K1 resrefs are.

## Open
Should the 2026-04-17 K2 `Override/` copies of `p_missionba`, `p_missionbb`, `p_missionh`, `p_zaalbar` (with `.utc`, `.ssf`, `.tpc`) be removed? They shadow nothing vanilla, but they make `P_MissionBB` resolve in K2 to non-vanilla data.

## Reproduce
- `.../scratchpad/h7/parse_mdl.py` (stdlib), `.../scratchpad/h7/xcheck_pk.py` (venv), `.../scratchpad/h7/inventory.py`, `.../scratchpad/h7/extract_k2chitin.py`, `.../scratchpad/h7/extract_k2fam.py`.
