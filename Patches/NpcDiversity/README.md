# Master NPC Diversity Dispatcher (`npc-diversity`)

> **STATUS** Track: EXPERIMENTAL. Version 0.2.7-probe (matches manifest.toml).
> **What it does / explores:** Deterministic per-creature variety for commoner clothing textures, fat-body models and alien heads, plus an optional idle-flourish experiment (`anim_*` keys). The remap hook (hook 2) makes the choice. The texture hook (hook 1) only logs in this build.
> **Requires:** Steam Aspyr (manifest.toml SHA `6A522E71...`) or GOG Aspyr (`777BEE23...`). No required providers (`requires = []`). Optional: the npc-world provider (`k2.npcworld`) for `visit_variety` and the `anim_*` keys. KPM version: not recorded in this dir.
> **Known limits:** Hook 1 is log-only. The `anim_*` keys are probe code and default off. Commoner textures are large (Prerequisites 4). Named, merchant and uniformed NPCs keep their default look. **Testing:** live-tested on Steam; several NPC groups (named, merchant and animation variants) still need an in-game check.
> **Install:** Do not install for normal play. This is a probe build that logs layout facts and runs the ambient-animation experiment. Install only to run the pending play checks.

> **Assets not included:** the `override/` textures, models and 2DAs and the `rejected_pool/` named in this README are game-derived or third-party and are not in this repository. Without them the DLL only logs (Hook 1 is log-only); generate them locally with the author's scripts before using the remap.

## What it does

1. **Hash.** Each remap call hashes the creature object id and the final appearance row (see Hashes). The result is the same look every visit unless `visit_variety=1`.
2. **Texture variant (hook 2).** `RemapNpcAppearance` writes the variant row. About 35% of generic commoners get a texture variant (`kVaryPct`). Slots span `N_CommM01..35` and `N_CommF01..35`.
3. **Body and head.** 15% of male and female commoners get the fat body model (`kFatPct`). 5% of female commoners get the Czerka officer (`kCzerkaPct`). Alien head variants come from the same remap.
4. **Protection.** Companions, story NPCs and named, merchant or uniformed NPCs keep their default look. The creature tag is not yet readable, so keep-default is position-only (`DiversityRoles.h`, `kKeepPositions`).
5. **Hook 1 (log-only).** `OverrideNpcTexture` counts and logs the texture ResRef the engine requests. It never writes.

## Engine Hooks & Signatures

| Target | Hook Function | Function / Purpose | Address |
|---|---|---|---|
| Steam Aspyr | `OverrideNpcTexture` | `CSWCCreatureAppearance::GetBTypeModelAndTextureResRef` (log-only) | `0x0085E601` |
| Steam Aspyr | `RemapNpcAppearance` | `CSWCCreature::SetAppearance` (writes the variant) | `0x0076D200` |
| GOG Aspyr | `OverrideNpcTexture` | `CSWCCreatureAppearance::GetBTypeModelAndTextureResRef` (log-only) | `0x0048E691` |
| GOG Aspyr | `RemapNpcAppearance` | `CSWCCreature::SetAppearance` (writes the variant) | `0x00419070` |

## Hashes

- **Remap hook (hook 2):** `HashCreatureId` (`DiversityLogic.h:90`). FNV-1a over the creature oid (4 bytes, least significant first), then the final appearance row (2 bytes). The visit salt is mixed in when `visit_variety=1` and npc-world supplies it.
- **Hook 1 seed:** `ResolveTextureSeed` (`HookDecision.h:40`). A stash wins when present. Otherwise it is FNV-1a over `(pDescPtr, retAddr)`. The dispatcher does not call it in this build, because hook 1 is log-only.

## Configuration (`npc-diversity.ini`, section `[NpcDiversity]`)

A missing file or key reads as 0 unless noted.

| Key | Default | Effect |
|---|---|---|
| `visit_variety` | 0 | 1 = new look per visit (per-visit salt from npc-world; looks stay fixed without it) |
| `anim_variety` | 0 | 1 = play an idle flourish on idle eligible creatures (npc-world TICK drives it) |
| `anim_probe` | 0 | 1 = log the engine animation chain once per eligible creature, no play |
| `anim_probe_all` | 0 | 1 = also log creatures of unchecked rows (never played) |
| `anim_flags` | 9 | PlayAnimation flags: `0x1` once, `0x2` no cross-fade, `0x8` overlay (idle stays current). `0/1/2/4` replace the idle |
| `anim_restore` | 0 | 1 = after a play (plus the post-log delay), re-issue the default idle via client `SetAnimation(10000,0,0)` |
| `anim_probe_max` | 24 | Chain log lines per process |

## Logging

- With the logging core: `kplog.ini` section `[npc-diversity]`, `level=debug` for one REMAP line per row change.
- Probe builds without the logging core: create `npc-diversity_debug.txt` in the game dir to raise the sink to DEBUG. Delete it to drop back to INFO. The flag is polled every 30th remap call.

## Build

From this folder run `python3 ../create-patch.py`. Host unit tests live in the author's development workspace and are not shipped here.

## Prerequisites

1. **Echoes/TSLRCM-edited base 2DAs are not shipped as base.** The author's generator appends rows to the live `appearance.2da` and `heads.2da`. The shipped copies in `override/` are built on the author's base, so installing them over Echoes or TSLRCM table edits replaces those edits.
2. **HD Twi'lek assets come from another mod.** `P_TwiHNGreen` and `P_TwiHTRed` (models) and `P_TwiNGreen`, `P_TwiNBlue`, `P_TwiToneRed` (textures) back heads 198, 200 and 204 (rows 809-811). Dancer textures `N_TwiDanceA01` and `N_TwiDanceB01` (rows 809-810) are also live-only. Without them those heads and textures are missing (expected at runtime: missing head or texture, not tested).
3. **`N_CommM10` is a TGA.** It is a valid uncompressed 32-bit 512x512 TGA with a TGA 2.0 footer, so the engine can load it as shipped.
4. **Commoner textures are large.** 15 of the `N_Comm*` textures are 2048x2048 DXT5, about 4x the VRAM of the 1024x1024 set.
5. **GOG note.** GOG hook 1 at `0x0048E691` collides with `k2-class-menu-clothes` (see the `conflicts` entry in `manifest.toml`).

## Asset provenance

Provenance and license status are not recorded in this dir. `override/` (74 files) and `rejected_pool/` (33 files) include game-derived and third-party assets, such as the `llk_*` models. Before a public push, the owner must confirm the license of these files, or move them to install-time generation.

## Packaging (.kpatch)

KPM package `NpcDiversity.kpatch` is built via `Patches/create-patch.py` under the KPM tree (`$K/Patches/NpcDiversity`):
- `manifest.toml`: Patch metadata, supported versions, build definitions.
- `kotor2-steam-aspyr.hooks.toml`: Steam Aspyr hook definitions.
- `kotor2-gog-aspyr.hooks.toml`: GOG Aspyr hook definitions.
- `binaries/windows_x86.dll`: MinGW-compiled 32-bit PE DLL exporting `NpcDiversity_Init`, `OverrideNpcTexture`, `RemapNpcAppearance` and `KPatchInit` (`exports.def`). `KPatchInit` receives the KPatchApi for the npc-world client. Adding it changes the export list, so the build needs a full reinstall, not a hot-swap (`NpcDiversityDispatcher.cpp:45-47`).

Static asset overrides (textures `N_CommM01..35`/`N_CommF01..35`, models, `appearance.2da`, `heads.2da`) reside in `override/` for game directory installation.
