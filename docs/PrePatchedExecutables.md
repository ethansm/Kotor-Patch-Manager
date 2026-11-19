# Pre-Patched Executables

KPM identifies the game by the SHA-256 of the executable and only installs patches whose manifests list that hash.
Executables that other tools have already modified therefore show up as an unknown version. This page lists the
pre-patched hashes we have seen, what they contain, and how to move to a supported setup.

## KOTOR II Steam (Aspyr): 3C-FD Patcher / LAA

| | SHA-256 |
|---|---|
| Vanilla Steam Aspyr `swkotor2.exe` (supported, `kotor2_steam_aspyr`) | `6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF` |
| After 3C-FD Patcher (not supported) | `4AB72FC1AB082F427E008CDA32FC5602D27B4E12FEF48C4A1A2C6F7B2F36FB5A` |
| Vanilla + only the Large Address Aware flag (e.g. KPM `4gb-patch`) | `7751F7D8B06C63A21EACBEF6E5DED6CB4F497FA960CCA6A6CF645552701DC04F` |

Many community mod builds run the 3C-FD Patcher (or a 4GB patcher) as their first step, so a K2 Steam install that KPM
reports as unknown is often one of these. The `4AB72FC1…` hash was observed on one install; other 3C-FD versions or
option sets may produce different hashes.

### What 3C-FD changed (5,124 bytes vs vanilla)

Every changed byte is covered by an existing KPM patch:

| 3C-FD change | Where | KPM equivalent |
|---|---|---|
| Large Address Aware flag, `0x0103` -> `0x0123` | PE header, file offset `0x11E` | `4gb-patch` (byte-identical) |
| Colour grade in the gamma pass (R/B x0.90, G x0.85; removes the yellow tint) | VA `0x989DB0` | `colorshift_k2` (byte-identical) |
| 3 NOPs over `fmul dword [ebp-0x28]`: no dialogue ducking of music/ambient volume | VA `0x70E130`, `CExoSoundSourceInternal::SetVolume` | `K2_music_fix` (byte-identical) |
| Fog and reflection shader program edits | VA `0x990020`, `0x990918`, `0x990EC8`, `0x9F7B80`, `0x9F7FC8`, `0x9F8580` | older `fog_reflection_fix_k2`; superseded by `k2_aspyr_shader_fixes` |

The code layout is otherwise identical to vanilla, so KPM hooks would apply, but installing KPM patches on top of
3C-FD risks double-patching (the same bytes changed twice, or `original_bytes` checks failing on the edited shaders).

### Recommended setup

1. Restore the vanilla executable (Steam "Verify integrity of game files", or a clean backup), and confirm the hash is
   `6A522E71…`. Verifying also restores the stock `binkw32.dll`, which removes KPM's loader; reinstall your patches
   afterwards.
2. Install the KPM equivalents: `4gb-patch`, `k2_aspyr_shader_fixes`, `colorshift_k2`, `K2_music_fix`.
   `k2_aspyr_shader_fixes` conflicts with `k2-aspyr-saber-hilt-texture-fix` and `k2-aspyr-normal-map-fix` (it includes
   them) and replaces the same programs as `fog_reflection_fix_k2`, so leave those three off.
3. `k2_aspyr_shader_fixes` also removes a +15 brightness offset, so `colorshift_k2` on top of it can look slightly
   darker than 3C-FD did; drop `colorshift_k2` if it does.

After step 2 the executable hash is `7751F7D8…` (vanilla + LAA). That is expected: KPM records the original
`6A522E71…` in `kpm_install_state.json` and keeps identifying the game as Steam Aspyr.
