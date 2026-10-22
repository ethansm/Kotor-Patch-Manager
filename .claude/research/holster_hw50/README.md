# HW50 heavy weapon feel: verified recon and staged path (2026-10-22)

Read-only. Source: `patch_manager_mods/holster_src/*` (paths below are relative to it unless marked). Eight Haiku reports are copied here unedited as `H1_codepath.md` .. `H8_ini_variants.md`; corrections live only in this file. Doc: `patch_manager_mods/42c_smooth_holstering_master_plan.md` row HW50 (line 86), section 4.16 (line 187). Prior recon: `.claude/research/holster_hw44/cat_heavy.md`.

## Verdict

The Haiku reports are mostly sound; four need corrections (see "Corrections"). Two findings matter more than the rest. (1) `w_HvyBlstr`, the item named "Heavy Blaster", is a PISTOL in code (HolsterLogic.h:66), so "heavy blaster" may not be the CatHeavy path at all (Stage 0). (2) For a CatHeavy repeater the via (`reach_via_heavy=1`) is the leading cause of a "swung prop" read, and the Haiku numbers understate it: they rest on a 45 deg GUESSED back-mount quat, but the repo documents the live heavy mount as barrel down within 3 deg (holster.ini:68-70, cat_heavy.md:15). Re-running the same harness at that axis gives a via path of about 355 deg instead of 90 deg and peak turn speed 2518 deg/s instead of 593 (model, GUESS head/hand proxies, roll unspecified). No real-play log has a heavy slide, so nothing here is play-confirmed.

## Verified facts

| # | Claim | Status | Anchor / how checked |
|---|---|---|---|
| a1 | `w_HvyBlstr` is CatPistol; only `w_RptnBlstr`, `w_HvRptBltr` are CatHeavy | VERIFIED | HolsterLogic.h:66, :70; baseitems row 13 "Heavy_Blaster" ww4 pistol tier, row 23/24 ww6 `research/holstered_weapons/wave2_3.md:699,709-710,732`. Lengths: hvyblstr 0.47 m, rptn 0.72, hvrpt 0.75-0.85 (ItemModelTable.inc:352-363, HolsterShape.inc:60-66) |
| a2 | Pistol path = draw 300 / stow 350 ms, grab 30, via off by default; heavy = 570 / 700, grab 45 | VERIFIED | HolsterTune.h:715-722; holster.ini:186-188 (via melee/rifle/heavy only) |
| b1 | Via adds orientation path and doubles peak turn speed (45 deg guess: 90 -> 143.3 deg, 593 -> 1217 deg/s) | VERIFIED (reproduced) | Rebuilt H4's harness in scratchpad `V50/` (copy of `holster_sweep_test.cpp` + driver): same 143.3 / 1217 |
| b2 | Sensitivity to the mount axis | NEW, model only | back deg -> via-on path / peak deg/s (off stays 90 / 593): 0: 90 / 1107; 45: 143 / 1217; 90: 210 / 1515; 135: 284 / 1962; 165: 334 / 2345; 177: 355 / 2518. The extra path grows with how far the mount axis is from vertical-up. Peak roughly doubles even at 0 deg (two eased 120 ms legs, HolsterReach.h:653-660). Caveats: qFromTo fixes the axis only (roll free), live tilt is toward -y not +x, hand frame is the sweep constant |
| b3 | Via mechanics: residual window 240 ms, two legs, bump 16 s^2(1-s)^2 peaking at the via | VERIFIED | HolsterReach.h:643, :653-660, :665-672; Holster.cpp:1564-1572 (`reachViaFor`). Cap: `msEff = min(slide time left, resMs)` (Holster.cpp:1950-1953), so a fresh heavy draw can use at most the 313 ms carry (570 x 0.55), not 600 |
| c1 | Heavy has no ease of its own: `reachPlan` takes no category; same `reachCarryF` / `easeSmooth` for all | VERIFIED | HolsterReach.h:384-392, :408-450; callers Holster.cpp:2301, :2665 |
| c2 | Draw ends at full hand speed, stow starts at full speed; header comment "zero speed at both ends" is wrong at the hand end | VERIFIED | `reachCarryF` slope v = 1.081 for q > 0.15 (HolsterReach.h:386-391); mid-draw speed ratio easeSmooth / carry = 1.5 / 1.081 = 1.39 (arithmetic) |
| c3 | Global `kReachResidMs` 240 ms (ini `reach_resid_ms` 0..600); the weapon trails the hand through 77% of the heavy carry | VERIFIED | HolsterReach.h:643; Holster.cpp:135, :596-600 (240 / 313.5 = 77%) |
| c4 | Replica numbers (hand 10/50/90% at 310/426/542 ms; weapon catches hand at 377 ms; stow turn from 143 ms) | UNVERIFIABLE | H3 used a Python replica; not re-run |
| d1 | Heavy `outT` = 0.10 (all categories); 0.45 is `outPole` (elbow flare); doc 42c "(c) outT 0.45" is wrong | VERIFIED | Holster.cpp:1522-1523; HolsterReach.h:259-260; holster.ini:206-208 |
| d2 | Hinge runs only for topo HingeBack (dz >= -0.25 arm) and only if `reach_hinge`=1; bulge/pole keys are inert then | VERIFIED | Holster.cpp:1737-1740, :95; HolsterReach.h:278-298 |
| d3 | Live heavy topo (hinge vs over vs thigh) | UNVERIFIABLE | not in repo; REACH `topo=` answers it. Note the sweep GUESS mount (z 1.028, dz -0.44 m = -0.70 arm) would be Thigh by the gate, yet the sweep calls `reachHinge` for every mount (H5) |
| e1 | `holster_log_check.py --row SM-L15` does not measure head clip (only `vo=` and `pb=`) | VERIFIED | scripts/holster_log_check.py:876-916. Head clip appears only as play text in PLAY_TEST_QUEUE.md:116 |
| e2 | Sweep head proxy is a GUESS (center z 1.50, r 0.105); no head bone is read by the mod | VERIFIED | holster_sweep_test.cpp:31, :51 ("head_g unmeasured"); `rg` for head bone names in Holster.cpp: no hit |
| f1 | Boot-only: draw/stow_slide_ms*, reach_grab_pct*, reach_via*, reach_via_out_m*, reach_hinge_out_*, reach_bulge_*, reach_pole_*, reach_resid_ms, reach_hinge, reach_extract_pct_*, reach_exit_* | VERIFIED | `readConfig` is called once (Holster.cpp:3144); these parse inside it (:433, :523-546, :596-625) |
| f2 | Live (500 ms poll) only when `probe=1`: place*, left_extra_rot*, slow_holster_x | VERIFIED | `maybeReload` runs only under `g_probe` (Holster.cpp:856, :904, :1253); isTuneKey HolsterTune.h:215-220. slow_holster_x is global (all categories), 1..8 |
| f3 | Draw ms hard cap 570, stow 1000 at set time; slow factor applied after | VERIFIED | HolsterSlide.h:220-230 |
| g1 | Left arm is never driven for a right-hand heavy; `isTwoHandedStem`, `isLong`, `preferredZone`, `chooseMount` have no runtime caller | VERIFIED | `rg` excluding `*_test.cpp`: definitions only (HolsterLogic.h:95,107,139,166); reachArm called with `ent->left` from the attach node (Holster.cpp:2672) |
| g2 | Engine may already pose the left hand: weaponwield 6 -> style 9 -> ready idle `g7r1` | PARTIAL | doc 10_equipment_3d_character.md:47 states it; SM-L8 still `pending` (PLAY_TEST_QUEUE.md:119) |
| h1 | No real-play heavy SLIDE or REACH line | VERIFIED for current logs | checker on `holster_log.txt` (28 lines) and `.1` (5,514): `SLIDE start=0`, `REACH found=0`, SM-L15/L17 NO-DATA. Scratch be2e copy: heavy gob d74dbc0 has 157 PLACE samples, but SLIDE start has gob= only (Holster.cpp:2602) and the 60 slides were not joined to it by me (H7 says none). SM-L18 step 2 (heavy) is the first chance |
| h2 | SLIDE start has no cat/mname; SLIDE end has no gob | VERIFIED | Holster.cpp:2602, :2614. REACH already has `cat=` (numeric, heavy = 6), `topo=`, `vo=`, `f=`, `dzA=` (HolsterReachLog.h:6-9) |

## Corrections to the Haiku reports and the doc

1. Doc 42c (c): outT is 0.10; the heavy-only value is outPole 0.45 and it moves the elbow 0.03 m, not the hand (H5 sim, UNVERIFIED re-run).
2. H3 item 3: `reach_exit_len_heavy=0.25` alone does nothing. Heavy exitSign is 0 (HolsterShape.inc:63-65), and `exSign != 0` gates the lift (Holster.cpp:1801); `reach_exit_sign_heavy=1|-1` must be set too (:1794-1799).
3. H5 knob table lists `reach_via_heavy` default 0; the shipped template sets it to 1 (holster.ini:188) and the live config reportedly 1 (cat_heavy.md:5).
4. H4/H2 use a 45 deg back mount; the repo documents the live heavy mount as barrel down (holster.ini:68-70, cat_heavy.md:15). Treat H2/H4 absolute numbers as a lower bound for the via effect. H2 peak 1192 vs H4 1217 deg/s: sampling-metric difference, not a conflict.
5. Sweep head/torso clearance is negative for the baseline in many rows (e.g. stow, 45 deg: -0.047 / -0.082), so it cannot clear a change; at 177 deg via off vs on differ by sign on stow head (-0.075 vs +0.010) but proxies are GUESSes.

## Hypotheses re-ranked (doc 42c a-d plus new)

| rank | hypothesis | evidence | confidence |
|---|---|---|---|
| 0 | N1 misclassification: the user's "heavy blaster" is `w_HvyBlstr` (pistol: 300/350 ms, hip, via off, pistol tuning) | HolsterLogic.h:66; item named "Heavy Blaster" is the pistol row. But "no longer clips" and the approved back mount point at a repeater | unknown: Stage 0 decides; if true, none of the heavy-only knobs apply |
| 1 | (a) via: the weapon rolls through "long axis up" | b1-b3: path 90 -> 143 (guess) .. 355 (barrel down) deg, peak x2 .. x4.2, plus 0.12 m bump; rifle is approved with a 45 deg mount, heavy is the barrel-down outlier | code + model, not play |
| 2 | N2 timing and ease: no weight cue (same curves as every category, hand stops dead, weapon trails 240 ms) | c1-c3. Real but it is a shape tweak; pistol and rifle share it and read fine | medium-low |
| 3 | (d) support hand | g1 true in code; g2 says the engine may already pose it; SM-L8 never played; cost highest | low until SM-L8 |
| 4 | (c) hinge arc | d1 refuted as worded; outPole changes elbow 0.11 -> 0.14 m only; applies only if topo=hinge | low |

## Open questions (what answers each)

| question | answered by |
|---|---|
| Exact item: Heavy Blaster / Repeating Blaster / Heavy Repeating Blaster; hip or back | user; or REACH `cat=` (6 = heavy) and PLACENODE `mname=` in one play (checker, not raw grep) |
| Which part reads silly: roll/flip, constant pace, abrupt stop, draw or stow | user, one sentence |
| Live topo for heavy | REACH `topo=` + `dzA=` via `holster_log_check.py <log>`; needs `reach_test=1`, `reach_carry=1`, `log_verbose=1` |
| Live heavy mount quat (is the axis really barrel down) | `log_mount=1`, then `holster_log_check.py <log> --mounts --json` |
| Does the engine already hold the heavy with the left hand | SM-L8 (`ch_anim=1`) |
| Real head clearance | no current source (see Stage 3 check) |

## Staged path

Stage 0 (user, 1 minute): "Which exact item is it (name in the inventory), on back or hip, and which step looks wrong: the weapon flipping over, the pace, or the stop?"

Stage 1 (play only, no code): one heavy draw and stow x3 inside SM-L18 step 2 with `log_verbose=1`, `log_mount=1`, `reach_test=1`, `reach_carry=1`. Read with `holster_log_check.py <log>` and `--mounts --json`; look for REACH `cat=6`, `topo=`, `vo=`, `f=`. Optional code (about 12 lines, Holster.cpp:2602/2614, host-test free): add `cat=<name>` and `mname=` to SLIDE start, `gob=` to SLIDE end, and a per-slide peak shown-weapon turn in deg/s on SLIDE end. Only needed if REACH `cat=` is not enough.

Stage 2 (ini only; back up the live ini first; every key below is boot-only, so restart the game each time; run BASE first):
- V1 `reach_via_heavy=0` (holster.ini:188). Model prediction (conditional on the barrel-down axis): path 355 -> 90 deg, peak 2518 -> 593 deg/s. Watch head and torso on stow; sweep proxies cannot clear it.
- V2 `reach_via_heavy=1` + `reach_via_out_m_heavy=0.04`: bump only, rotation unchanged, so not a swing fix (H4).
- V3 `reach_exit_sign_heavy=-1` + `reach_exit_len_heavy=0.25` (try `1` if it lifts the wrong way; sign UNVERIFIED), optionally `reach_extract_pct_back=0.30` (global back, 0..0.5): the weapon rises straight out before it turns.
- V4 `stow_slide_ms_heavy=900` (draw cannot pass 570; live alternative `slow_holster_x=1.5` slows every category, probe=1 only, for a look test).
- V5 `reach_resid_ms=300` (global; heavy gains at most 313 ms of window). Low value.
Order: V1, then V1+V3, then V4. Skip V5 unless V1 fails.

Stage 3 (code, ranked by value / lines / risk; each only after Stage 2 says ini cannot do it):
1. Heavy-only soft ease: replace `reachCarryF(q)` by `easeSmooth(q)` for CatHeavy at HolsterReach.h:429, :433, :440 and the X in `reachOriF` (:401). About 12 lines plus a per-slide flag (Slide or a `reachPlan` parameter) set where the slide is armed. Risk low: same geometric path, only time-warped. Host test: new case in `holster_slide_test.cpp` (zero slope both ends for soft; every other category bit-identical), `holster_sweep` TSV diff shows unchanged min clearances.
2. Partial via: a per-category tilt or amount (`reach_via_tilt_deg_<cat>`, replace world `up` by an up tilted toward forward in `reachViaFor`, Holster.cpp:1564-1572). About 30 lines incl. ini parse. Risk medium (the via keeps the head-clear role). Host test: `holster_resvia_test.cpp` path length for axis 0/90/177, tilt 0 reproduces today bit for bit.
3. Per-category residual window (`reach_resid_ms_<cat>`): about 25 lines, but capped at the 313 ms carry, so +30% at most. Low value.
4. Support hand (H6): about 80-90 lines, gated by SM-L8; HW33 stays closed until then.
Not recommended: torso lean, shoulder drop (UNVERIFIED parent chain), inertia low-pass (UNVERIFIED).

Head-clearance check (SM-L15 does not do this): (i) offline, after Stage 1: `holster_log_check.py <log> --mounts --json > m.json`, then `run_tests.py holster_sweep` binary with `--mounts-json m.json --dir both`, compare `head_tip/head_butt/head_shaft` columns baseline vs variant; (ii) replace the GUESS head center with a measured one: one-shot log of the head bone (new, about 20 lines, no head bone is read today) and a `--head x,y,z` sweep option (about 10 lines); (iii) optional in-game field `hd=` (min weapon tip/butt to head bone per solve) with a new checker clause (about 40 + 25 lines). Until (ii) exists, every clip verdict is the user's eye on f 0.2-0.4 of the draw and f 0.3-0.6 of the stow.

Scratch (not in repo): `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/V50/` (b2 and clearance rows).
