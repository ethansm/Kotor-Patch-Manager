# HN-T09
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HN-6n-05 | PARTIAL | patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:171-175 | yes | Hex branch never uses n; anchor line 171 matches. Latent: callers pass 64B. |
- K1SpawnProbe.cpp:152 - header comment says out needs 64 bytes.
- K1SpawnProbe.cpp:153 - signature takes size_t n; n is used only by snprintf at :156 and :167.
- K1SpawnProbe.cpp:171 - anchor `char* o = out;` matches exactly (anchor check OK).
- K1SpawnProbe.cpp:172-173 - memcpy of fixed 4 bytes "hex:" then pointer advance; no n check.
- K1SpawnProbe.cpp:174-175 - fixed 16-iteration loop; first write per iteration is *o++ with no bound check.
- K1SpawnProbe.cpp:161 - text-branch length scan capped at 16 bytes (bounded; not the hex path).
- K1SpawnProbe.cpp:295 - callers' buffers declared as char ar1[64], ar2[64].
- K1SpawnProbe.cpp:306 - caller passes ar1 with sizeof ar1 = 64 (not under 52).
- K1SpawnProbe.cpp:307 - caller passes ar2 with sizeof ar2 = 64 (not under 52).
- OPEN: exact 52-byte hex count lies at line 176+ (outside range); other fmtRef16 callers outside 295-312 unseen.
