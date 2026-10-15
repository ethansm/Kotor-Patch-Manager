# 43 — Asset import and NPC programme: overview and map (2026-10-14)

Status: OVERVIEW. This doc adds no new research. It ties together the efforts that bring assets from other compatible games (mainly KotOR 1) into KotOR II, plus the patches that create, insert and customise NPCs and characters. Each effort keeps its own doc; read those for evidence. Facts below are copied from them and marked with the doc number. Nothing here is verified in game unless a row says so.

## 1. Goal
Make KotOR II worlds feel populated and varied without replacing vanilla files:
1. Bring in characters, models and textures from K1 (and from our own generated textures) in a form K2 loads.
2. Put new NPCs into areas at runtime with a patch, not a baked-in `.utc` or `.mod` edit.
3. Vary how NPCs look, move and sound.
4. Give characters, including imported ones, visible gear behaviour (holstered weapons).

## 2. The five tracks
| Track | What | Doc | Code | State (per its doc) |
|---|---|---|---|---|
| A. Extract and convert | K1 models, textures, heads and 2DA rows to K2 format, with a licence ledger | 38 | `k1_port/` | Converter works on `n_commm`, Lena, Juhani, Carth (staged); a fixed-pipeline model is not yet played |
| B. Native spawn and world | Runtime creature spawn, area/tick hooks, shared `npc-world` library | 38 §12-17, 40 §12-13, 35 | `npc_world_src`, `npc_world_probe_src`, `area_hook_probe_src`, `npc_add_src` | `k1-spawn-probe` 0.2.0 live; `npc-world` 0.1.0-dev and the probe client are host builds, live run V2b pending |
| C. Appearance and texture variety | Appearance dispatcher, texture overlays, AI-assisted recolour | 34, `.claude/research/texture_diversity/PLAN.md`, `.claude/research/texture_ai_pipeline/report.md` | `npc_diversity_src` | `npc-diversity` 0.1.6-probe; overlay tooling built; looks not yet installed |
| D. Behaviour and voice | Extra walkers, animation variety, scripted encounters, custom barks | 40, 41 | `kvoice/`, `voice_probe_src/` | Research and host builds; nothing installed |
| E. Gear on characters | Holstered weapons on hip and back | 42, 42 master plan | `holster_src` | `holster` 0.1.0-dev installed but inert (probe only); play tests HW5 pending |

## 3. Track A — extract and convert (doc 38)
Pipeline today, all under `patch_manager_mods/k1_port/` (usage in its `README.md`):
1. **Read K1 assets** from the local K1 install and from community packs (sources are listed in `sources.json`).
2. **Convert the model** with `k1_port_model.py` using a patched pykotor. Stock pykotor 2.3.12 cannot write K2 MDL (doc 38 §2). The patch fixes the K2 header size, skin pointers, `unknown0`, tangent flags, padding slots (`pad_skin_bones`) and face plane distances (`fix_face_planes`). `--supermodel-map OLD=NEW` redirects K1-only supermodels.
3. **Convert textures** with `k1_port_textures.py` (TPC/TGA, DXT codec in `k1_port_tex_dxt.py` and `k1_port_tex_tpc.py`).
4. **Ingest with the licence rule** (`k1_port_ingest.py`, `k1_port_prune.py`, `sources.json`, ledger outside the repo). Packs that need the author's approval are staged flagged; `gate PATH...` fails on flagged files before anything is copied into the repo or a release (doc 38 §8).
5. **Build an in-game character**: `k1_port_char_build.py` writes appearance and heads rows plus a UTC for an imported character (Juhani = appearance 818 / heads 249, Carth = 819 / 250; stage only).
6. **Probe installer**: `k1_port_probe.py` plans and installs the staged probe files. It blocks on the licence gate, a running game, and `heads.2da` (it only replaces `appearance.2da`).

Settled facts worth keeping:
- Stock pykotor writes K2 MDL 8 bytes wrong per mesh header and misreads real K2 models (doc 38 §2).
- **The explosion cause was junk in the skin `bones[16]` padding slots.** K1 leaves values far out of range; a hand-edited model with in-range values rendered normally (user play, 10-14, doc 38 §16). Zero face plane distances are harmless in game.
- K1-only character models are 19 party-member models plus `c_terantanak`. Candidate order: Zaalbar, Juhani, T3-M3 as the pipeline control (doc 38 §16).
- Most K1 creatures need no port: every pet candidate uses a K2 appearance and a vanilla K2 template (doc 38 §15).

Open in Track A:
- No model from the fixed pipeline has been played yet (the played `probe_skinfix` was hand-edited).
- Install path for Juhani and Carth is an owner decision (extend the probe installer, or a full KPM reinstall).
- Whether TSL serialises `Appearance_Type` into saves, and TLK sound versus DLG `VO_ResRef` precedence (doc 38 §4).
- Tooling research not finished: MDLOps `-k2` under Proton is blocked (no binary); MDLEdit round trip is manual only (doc 38 §3).
- Redistribution: ingested packs are local-only until authors approve. Do not put any K1 or community-pack asset into a release without checking its ledger row.

## 4. Track B — put NPCs into the world
Why not edit templates or the `.mod` GIT: `.utc` swaps bake into saves and conflict with TSLRCM; a GIT edit is fixed per install (docs 34, 40 §1). The adopted route is a patch hooking area load.
- **Hooks** (Steam addresses): `0x568BD0` AddToArea, `0x55A460` LoadModuleInProgress, `0x533830` main loop. The native spawn sequence (alloc 0x1220, ctor `0x561f30`, `0x572a70` load from template, orientation, AddToArea) is in doc 38 §14-15.
- **`k1-spawn-probe`**: 0.2.0 is live (spawns a mouse droid and `k1p_native_npc` in `204tel`); 0.3.0-probe is a DLL-only client of `npc-world` (doc 38 §17).
- **`npc-world`** (`npc_world_src`, 0.1.0-dev): owns the shared hooks and native spawn and provides `k2.npcworld@1`. `npc-world-probe` (0.1.0-probe) tests OID stability, save persistence, capacity and frame cost. Live run V2b needs the user's go and a reinstall (doc 40 §6).
- **Doc 35 (`npc_add_src`)**: the `.mod` GIT route is retired; the builders stay as a fallback toolbox.

Open in Track B: whether hook-spawned NPCs persist or duplicate across save/reload, spawn cap and frame cost, whether a vanilla `MouseDroid` already exists in `204tel` (doc 38 §17 O1), the install route (surgical edit versus full reinstall).

## 5. Track C — appearance and texture variety
- **`npc-diversity`** (doc 34): appends appearance rows 802-817 and remaps generic commoners at runtime through a texture and body dispatcher. The 10-11 review found blockers (stack-slot parameter, texture seed, rows without models); probe 0.1.x answers them in game. A `visitSalt` is added so a section varies per visit (doc 40 §1).
- **Texture overlays** (`.claude/research/texture_diversity/PLAN.md`): deterministic recolour, trim and emblem decals first, AI second (Qwen-Image-Edit, Apache-2.0), always mask-composited and gated against the parent texture. Catalog: 123 usable cards, 184 atlases. Licence tags carry over to generated copies (user decision, doc 38 §12). Built: output gate, DXT codec, UV island masks, derived-ledger rows, helmets and head-item allow-list, the Character Revisited base. Next: user reviews sheets and gives an install go.
- Both depend on Track A's ledger and gate.

## 6. Track D — behaviour and voice
- **Doc 40**: extra walkers (aliens, TSF agents, Czerka), animation variety through the native `PlayAnimation` injector, scripted encounters (data-first route, override `.ncs` as fallback). Build order NB1 visit salt, NB2 animation, NB3 walkers, NB4 encounters. Everything needs `npc-world` live first.
- **Doc 41**: custom and reused voice lines. `kvoice` packs audio to the engine's container, loudness and lip format with a verifier; V-C live tests E1-E8 need a forged save and the user's go. Reused K1 clips are personal-use only.

## 7. Track E — holstered weapons (doc 42)
Goal: characters visibly stow weapons out of combat. Lightsabers on the hip, short one-handed blades on the hip, long two-handed weapons on the back, two weapon sets on four mounts, draw never delays the first attack.
- **Approach (user decision 10-13):** a self-contained DLL, no shipped models or animations. It redirects the weapon attach node to existing bones (`pelvis_g`, `lthigh_g`/`rthigh_g`, `torsoUpr_g`), because no holster dummy nodes exist on any of the 133 humanoid bodies.
- **Built:** `holster_src` 0.1.0-dev with six log-only detours, pure logic and tests, and a flag-file `bone_test` that repoints the `rhand`/`lhand` literal in `FUN_007637E0` (`holster.ini`, default off). The offline preview (`k1_port_tex_preview.py weapons`) renders weapons on bones in rest pose.
- **State:** installed 10-13 22:55 but inert (probe only). Play runs HW5 (probe, then bone test) are the user's; HW6 answers gates G2-G7; HW7 is the P1 MVP.
- **Link to this programme:** imported K1 characters (Track A) arrive with K1 bodies and supermodels. The holster mod attaches by bone name, so each imported body must carry `pelvis_g`, `torso_g`, `torsoUpr_g` and the thigh bones. Check this when a converted character is staged; the preview tool already lists those nodes. NPC holstering is out of scope until after P1 (PC and party).

## 8. How the tracks depend on each other
```
A convert --> ledger/gate --> C overlays ----\
A characters --> B npc-world spawn --> D walkers/encounters/voice
                    |                      \
                    +--> C dispatcher        +--> (needs B live)
E holster: independent hooks (0x5DF926, 0x7637E0); shares the KPM install and Common headers
```
Shared constraints:
- **Hook collisions:** two hooks on one address fail `check_hook_spans`. `holster` sits at `0x5DF926` because cmdq-probe owns `0x5DF920`; `k1-spawn-probe` and `area-hook-probe` clashed on `0x533830`/`0x55A460` (probe removed).
- **Install rules:** a new mod id with hooks needs a full REINSTALL, not a hot swap; every live install needs explicit user GO and a closed game (kotor2-kpm-install skill).
- **Versioning:** manifest versions stay 0.X.Y until the public KPM PR.
- **Licence:** K1 and community assets are local-only; run `k1_port_prune.py gate` before copying into the repo or a release.

## 9. Decisions waiting on the owner
1. Install path for staged characters (Juhani, Carth): probe installer extension or full reinstall.
2. Go for `npc-world` live run V2b (needs reinstall and a pre-visit save).
3. Go for holster play runs HW5 (user plays; the DLL is already installed).
4. Go for texture look install after sheet review.
5. Go for voice live tests E1-E8 (needs the D0 pause lifted).

## 10. Where things live
| Need | Path |
|---|---|
| Converter, ledger, README | `patch_manager_mods/k1_port/` |
| Native spawn and hooks | `npc_world_src/`, `npc_world_probe_src/`, `k1_spawn_probe_src` (live 0.2.0 archive: `.claude/research/k1_spawn_probe_0.2.0_src/`) |
| Appearance dispatcher | `npc_diversity_src/` |
| Texture plan and catalog | `.claude/research/texture_diversity/`, `.claude/research/texture_ai_pipeline/` |
| Character candidates, creature catalog | `.claude/research/k1_char_candidates/`, `.claude/research/creature_catalog/` |
| Voice tooling | `patch_manager_mods/kvoice/`, `voice_probe_src/` |
| Holster mod and research | `holster_src/`, `research/holstered_weapons/`, docs 42 and 42 master plan |
| Review bundles for outside review | `python3 scripts/review_bundle.py list` (`patch_manager_mods/review_bundles.toml`) |
