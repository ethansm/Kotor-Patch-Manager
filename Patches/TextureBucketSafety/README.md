# Texture Bucket Memory Safety

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Bounds-checks the three 5000-entry texture bucket arrays (meshBuckets, fadeBuckets, capBuckets), which are indexed by driver-assigned GL texture names. ClearBuckets used maxTexID+1 as an iteration count without clamping (crash 0x0046BF22), and AddPartToMeshBuckets indexed and wrote through the arrays with no range check (crash 0x0046BEAE). Out-of-range parts skip the bucket insert but still reach meshShadowBucket, and are unreachable by the renderer anyway since DoMeshBuckets iterates the clamped AurTextureGetOrdering list.

## Details

- Patch id: `texture-bucket-safety`, version 1.0.0
- Author: VexFlint
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)
- KotOR 1 GOG 1.03 (`kotor1_gog_103`)

## Hooks

- `kotor1_103.hooks.toml`: 2 hook(s) (2 replace), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03

## Files

- `kotor1_103.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
