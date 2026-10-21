# C_geometry: weapon geometry and reach target (Gemini claims C1-C6)

Scope: static repo evidence only (no in-game run). Holster state at session start, commit 6c70f306.

## Verdicts

| Claim | Verdict | Anchor | Note |
|---|---|---|---|
| C1 dblsbr origin at lower end, grip centre +0.21 m | TRUE | `.claude/research/holstered_weapons/back_dblsaber.md:4`; `holster_src/HolsterLogic.h:153-161` | Hilt spans -0.065..+0.479 m, origin 0.065 m above lower tip, centre +0.207. "Grip centre = hilt centre" is an assumption. |
| C2 rifle barrel +y, grip -z, receiver above hand on back | PARTIAL | `inventory_3d_viewport_src/research/R17_fit_survey_2026-09-29.txt:258-260`; `42_holster_mounts_and_weapon_types.md:99` | Barrel +y measured (long=y). Grip -z is a doc assertion; bbox cannot show a grip. Receiver-above-hand is unmeasured (render estimate, carbine model). |
| C3 heavy repeaters held low at hip/abdomen when drawn | UNKNOWN-not-measured | `R17...txt:129-136` (size only); `42_holster_mounts_and_weapon_types.md:78` | Drawn pose never measured. Repo mounts heavy on upper back, not hip. |
| C4 quarterstaff is a symmetric rod; measured? | PARTIAL | `R17...txt:2-6` | Measured (yes). Cross-section 0.04-0.07 m square; long z 0.52-1.03 m; bbox centre z +0.14..+0.15, so NOT centred on origin. Doc 42 says "unmeasured" (`:98`, `:103`), stale. |
| C5 reach target = weapon origin, no grip offset, weapon jumps at grab | PARTIAL | `holster_src/Holster.cpp:1297`, `:1331`, `:1745`, `:1756` | Target = placed origin: TRUE. No per-weapon grip offset in code or ini: TRUE. "Jump along axis" not produced by the target choice (see chain below). |
| C6 vanilla: model origin is the grip for normal weapons | PARTIAL | `42_holstered_weapons.md:47`, `:53`, `:59`, `:65`, `:67`; `.claude/research/holster_c1_postcopy.md:8,29` | Attach copies the node pose (no offset seen, callee not decompiled). Origin = grip is asserted for blasters and single saber; double is the exception; G7 still open. |

## C5 evidence chain (code)

- `Holster.cpp:1133-1168` `mountPose`: returns the MOUNT BONE node pose (`pelvis_g` / `torsoUpr_g` ...), not a grip.
- `Holster.cpp:1745` `hipPose = applyPlacement(base, pl)`: bone pose + `place_off` (rotated). This is where the weapon origin sits.
- `Holster.cpp:1750` writes `hipPose`-blend into Gob `+0xA4`, i.e. the weapon model origin. So `mountPos` = weapon origin.
- Callers: `Holster.cpp:1498` (preview, `mountPos = out.pos`) and `:1756` (world, `mountPos = hipPose.pos`).
- `Holster.cpp:1297` `T = tgt.pos`; `:1318` and `:1331` carry path runs from `mountPos` to the hand. Hand aims at the ORIGIN.
- No grip key: `HolsterTune.h:6-21` lists place / place_rot / place_off / per-category / body / per-model `place_off_w_<stem>`. No `grip` key. No `w_dblsbr` key in `holster.ini`.
- `HolsterLogic.h:155-161` `hiltLengthMeters` and `gripOffsetFromOrigin` are defined but have NO production caller (rg excluding tests). The "centre on mount" in the comment at `:153` is not wired.
- Jump: the reach is aimed at the same origin the weapon is pinned to, so the target choice adds no axial step. Carry weight ramps in over the first 10% of carry (`HolsterReach.h:316-335`, easeSmooth). A jump appears only if the solve clamps (`Holster.cpp:1334`, reach<1): then the weapon is displaced by the shortfall, eased in. That is an arm-reach effect. Size not measured.

## Doc 42 staleness and conflicts

- `42_holster_mounts_and_weapon_types.md:98,100,103` call the quarterstaff and repeaters "unmeasured"; R17 has bbox data for both.
- `:99` says rifles have grip -z; `42_holstered_weapons.md:67` and `back_rifle.md:8` say blasters/carbine have grip at origin. Conflict.
- Heavy back-mount research (`back_heavy.md`) used `w_hvyblstr_001`, which is a PISTOL (`42_holster_mounts_and_weapon_types.md:62`). No repeater mount evidence exists.

## Unmeasured shapes

- Double-saber grip centre in the hand (G7, `42_holstered_weapons.md:59`).
- Heavy repeater drawn hold height (C3).
- Rifle receiver height when slung (only a render estimate, `back_rifle.md:3-4`, carbine model).
- Vibro / long / two-hand melee, pike, bowcaster, ion/sonic/disruptor: no shape check here (`:96-97`, `:103`).
- Quarterstaff grip location (only the rod bbox exists).

## Gaps to build

1. Live probe of the rhand attach to answer whether a vanilla double saber is held at its origin (G7, bone_test).
2. Wire a per-model grip offset (e.g. `place_grip_w_<stem>`) into placement; `gripOffsetFromOrigin` is defined but unused.
3. Log the reach ratio at the carry start to size any grab shortfall.
4. Measure grip direction with a hand-pose render; the R17 bbox cannot show the grip.
5. Update doc 42 section 4 with R17 extents for the quarterstaff and repeaters.
