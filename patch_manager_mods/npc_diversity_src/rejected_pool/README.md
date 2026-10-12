# Rejected pool textures (32 here; 35 rejected in total)

Labelled out of the plain commoner pool (`texture_labels.json` `meta.rejected_files`): robes, armor, uniforms, heads.
They are textures for OTHER meshes (Onderon / Sith commoner bodies), so on the plain N_CommM/F body they look wrong.
Kept for the alt-body outfit tier (doc 34 follow-on F1): copy a file into the game `override/` only when a row
whose body mesh uses it is wired in. Not installed by default; `check_diversity_assets.py` ignores this dir.
Live-game backup (pre-move copy): `<game>/backups/npc_div_rejected_pool/`.
Three rejected slots stay in `override/` because the appended 2DA rows name them as default textures: N_CommM17, N_CommM35, N_CommF34 (this dir holds the other 32).
`scripts/normalize_diversity_textures.py` skips the 32 (see `rejected_skip_set`).
