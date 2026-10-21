# REVIEW S3: timing / snap / interrupt slice (holster 0.11.0-dev)
H: = patch_manager_mods/holster_src/. Anchors verified on disk (working tree). reachPlan is in HolsterReach.h:316-342 (not HolsterSlide.h).

## 1) Verdict table
| id | source | claim | verdict | anchor | build impact |
|---|---|---|---|---|---|
| T1 | Gemini 5 | draw cap 570, grab 30% => 171 ms reach | TRUE at the cap; PARTIAL at defaults (350 ms => 105 ms, ~6 frames) | H:HolsterSlide.h:104, Holster.cpp:88,395 | reach really is short; tune grab pct and ms per category |
| T2 | Gemini 5 | raise draw cap to 750-850 for back mounts | PARTIAL: feasible (one constant) but not needed first | HolsterSlide.h:102-106 | defer; see R1 below |
| T3 | Claude 12 | "find out whether the 570 clamp comes from the engine" | FALSE (not engine): 570 = latest draw_weapon anim-event time (0.03-0.57 s, 12 clips); event has no known consumer | 42_holstered_weapons.md:52,59; 42b_holster_remaining_plan.md:24 | cap is a design choice, safe to change |
| T4 | Claude 12 | per-profile ms (pistol 300, saber 350, hip sword 400, back 520, staff/heavy at cap) | PARTIAL: all fit under 570; no per-category slide or grab key exists | Holster.cpp:88 (one static g_reachGrab), g_drawSlideMs/g_stowSlideMs globals; per-cat only bulge (359) and pole (375) | add keys (build 4) |
| T5 | Claude 12 | "stow can run longer" | TRUE (cap 1000); no engine reason recorded, only D2 line | HolsterSlide.h:108-112, 42b:24 | long stow = more reversal exposure (R2) |
| T6 | Gemini 5 | hand slams into the weapon at full speed and bounces back | PARTIAL: draw arrives eased (speed 0), leaves at 1/(1-g)=1.43 from rest; STOW arrives at full speed 1.43 then release eases from 0 | HolsterReach.h:325-327, 336-339 | ease the stow carry end and the draw carry start |
| T7 | Gemini 5 | ~30 ms dwell at grab | PARTIAL: no plateau today; carryW=easeSmooth(q/0.1) already eases the weapon onto the hand over the first 10% of carry | HolsterReach.h:326 | optional; low value vs T6 ease |
| T8 | Claude 13 | reversal continues from current hand position | FALSE today: slide fraction continues, hand path does not (H6 table re-derived: reach f=0.02 jump 0.97, f=0.10 0.64, f=0.15 0.35; carry p=0.5 0.21) | HolsterReach.h:326,337; HolsterSlide.h:65-80 | top fix (build 1) |
| T9 | Claude 13 | swap/death/cutscene snaps to safe end state | TRUE (silent reset to target end); unreadable => HAND end | Holster.cpp:1676-1686; HolsterDrawGate.h:44-48 | no new work; make resets rarer (build 2) |
| T10 | Claude 11 | attach-switch residual at grab/landing | PREMISE FALSE in drawgate: attach is native rhand, never switches (applyBoneTest returns early). A residual still exists: carried hand pose vs hipPose at the carryW 0->1 step | Holster.cpp:2117, 1755-1760 | log the gap (build 5); no decay offset needed |
| T11 | Claude 16 | saber blade shows mid-slide | UNKNOWN (H5 no evidence; mod never touches ignition; engine 0x7a7620) | n/a | live check L3 |
| T12 | Claude 17 | party start jitter | PARTIAL: each Gob has its own Slide; engine combat flags flip per creature; lockstep only through the 250 ms party cache (party_combat_hold) | Holster.cpp:914,1671 | optional, low (build 7) |
| T13 | E_timing E5 | slide math is wall-clock | TRUE (QPC ms; touch() freezes gaps >100 ms) | Holster.cpp:1527-1540; HolsterSlide.h:84-88 | none |

R1 detail. Extending a draw past 570 cannot delay the first attack (slide writes only gob+0xA4 and arm bones; attack timing is ms countdowns, 42_holstered_weapons.md:51); no attach desync (attach persistent). Real costs: (a) weapon still in transit during the first swing's contact frames; (b) reachArm overrides arm bones at weight ~1 for the whole slide, so a longer slide fights the engine attack animation longer. Gemini 750-850 is feasible but unproven; Claude's "stay inside the cap" plus per-category scaling is the safer first step. Stow side: no engine limit found; 1000 is a doc choice.

R2 detail (re-derived). Draw reach reversal: pathF 1-S(p/g) before, from=f after (HolsterReach.h:337 uses slide fraction as path position). Extra finding Haiku missed: after any retarget t0=now, so reachWeightRamp (HolsterReach.h:180-189) is 0 on the first frame (active needs d>0) => arm weight collapses to the engine pose for that frame, then ramps 0.04 of the new (short) length. So a reversal = arm snap to engine rest, then to the new pathF, plus carryW step (stow release->draw 0 to 1; stow carry->draw 0.84 to 1.0).
R3 detail. Hard resets fire when `owner==0` (par=[beh+0x14] or [par+0x1D8] zero/unreadable), combatOff==0, or rdU32Ok fails (Holster.cpp:1664-1668), or on `first` (new entry/owner). Steady fights: rare. Likely at area load, party swap, death/revive, appearance rebuild, 128-slot ring overwrite. Outcome depends on which read failed: owner 0 => gate.owner stored 0 (HolsterDrawGate.h:88), so next frame `first` => hand pop then silent hip pop (E2 right); combat-only read failure => hand pop then a full-length stow from the hand (E_timing right). Refusal frames (1723,1724,1738,1742,1748,1749): engine hand pose for that tick; PrNoMount needs a null node owner/find-slot mismatch (1135-1155), so rare in steady state.
R14 detail. Draw grab: speed 0 -> 1.43/len instantly (C0 with velocity step from rest, not a bounce). Stow grab: 1.43 -> 0 instantly. A 30 ms dwell costs 30 of the g*len reach (105 ms at defaults, leaves 75). Better value: ease-in on draw carry start (+ keep carryW) and ease-out on stow carry end; ease must be done inside reachPlan so total length is unchanged.

## 2) Gemini vs Claude conflicts and resolution
- Duration: Gemini wants 700-800 for back mounts (raise cap); Claude wants 520-570 inside the cap and says to check the clamp source first. Resolution: clamp is not engine-bound, but keep 570 now; raise grab_pct (45%) and per-category slide_ms (<=570). Raise cap only if L2 shows the swing overlap is fine.
- Dwell/ease: Gemini prescribes a 30 ms plateau + ease; Claude prescribes a residual offset decay (T10, premise false in drawgate). Resolution: ease curves in reachPlan, log the residual; no plateau first.
- Reversal: only Claude addresses it; it is real (T8) and is the highest-value timing fix.
- Blade: only Claude; unknown, needs play check.

## 3) Ranked snap sources (today's code) with fix
1. Mid-slide reversal discontinuity (T8 + arm-weight collapse). Fires on every combat/flourish/party/hold flip inside a slide (delay features make flips common: re-engage during a delayed stow, 250 ms party edges). Fix: carry path position through target(); restart arm ramp from current weight.
2. Saber blade/hilt split (hypothesis, saber only): engine ignites/douses at the combat-flag flip while the hilt still travels 350+ ms, so the blade appears at the hip or vanishes first. UNKNOWN until L3. Fix if confirmed: delay ignition via the engine call 0x7a7620 or hide blade node until the slide ends.
3. Hard resets S2/S3 (silent pop/hand pop, no log): rare but the largest jump; any transient owner/combat read failure or Gob rebuild mid-slide. Fix: on transient unreadable keep the slide (grace of ~3 frames) and log "SLIDE cut".
4. Stow carry end full-speed stop (T6): a deterministic small hitch every stow. Fix: ease-out.
5. Sparse refresh then catch-up (E_timing S5), refusal frames (S1): cadence-dependent, scale with slide length; confirm via SLIDE end frames/maxdt.

## 4) Build steps (in order)
1. HolsterSlide.h Slide::target + HolsterReach.h reachPlan: store `pathFrom` (path position at retarget, computed from the old plan) and use it instead of `from` in the stow-carry and draw-carry branches; reach reversal enters release from pathFrom, carryW 0. Tests holster_reach_test.cpp: |pathF(new,t_rev+1)-pathF(old,t_rev)|<0.01 and carryW step <0.05 for reversals in reach, carry, release; fix testReachPlanPartialDraw (line 618: expect ~0.28 before the reversal, not 0.4-0.5).
2. HolsterReach.h reachWeightRamp + Holster.cpp:1754: keep arm weight continuous across a retarget (carry old weight as `w0` ramping to the new curve). Test in holster_reach_test.cpp near line 538.
3. Holster.cpp:1684-1686 (reset branch): split `first` from `!readable`; on `!readable` with a running slide, hold the slide (skip reset) up to N=3 consecutive frames; add "SLIDE cut reason=first|unreadable frac=" log when reset hits an active slide. Test: HolsterSlide test for grace counter (pure helper in HolsterDrawGate.h).
4. Per-category `draw_slide_ms_<cat>`, `stow_slide_ms_<cat>`, `reach_grab_pct_<cat>` keys (parse next to Holster.cpp:359/394), resolved with the global as fallback; each clamped by clampDrawSlideMs/clampStowSlideMs. cfgWarn when a clamp bites (Holster.cpp:329-335). Tests in config parse suite.
5. reachPlan ease: smoothstep ease-in on the draw carry start (first ~15% of carry) and ease-out on the stow carry end, length preserved; add residual log |carried.pos - hipPose.pos| at the carryW step (Holster.cpp:1755-1760). Tests: monotone pathF, endpoints exact, speed at grab < 0.3 of linear.
6. Cap raise: only if L2 passes; add new `draw_slide_cap` ini default 570.
7. (Optional) party jitter: add 0-60 ms to t0 for non-leader party Gobs after target().
Gate each step: scripts/run_tests.py holster_reach / holster_slide scoped, then pre_install_gate for holster_src.

## 5) Live checks (add rows to PLAY_TEST_QUEUE.md; never block)
- L1 reversal: draw then immediately stow (and the reverse) with log_verbose=1. Pass: no visible weapon jump; log "SLIDE start dir= from=" pairs show continuous, no "SLIDE cut".
- L2 long back draw: back-mounted weapon, draw_slide_ms=570 vs a temporary 800 (after build 6). Pass: first swing arm not fought; SLIDE end frames>=ms/20 and maxdt<=50.
- L3 saber blade: draw and stow a saber in slow motion/recording. Pass signal: blade visible state vs hilt position per frame; record whether blade appears at the hip. Needs a play check (H5 UNKNOWN).
- L4 cut frequency: count `DRAWGATE owner/combat unreadable` (log_verbose=1), `SLIDE start` without `SLIDE end`, and PLACE refuse counters n= for PrReadIn/PrNoMount/PrInsaneIn over a full fight. Pass: zero unreadable lines in steady combat.
- L5 delayed holster: with combat_exit_ms>0 and move_hold, walk, stop, re-engage during the stow. Pass: no jump at the re-engage.

## 6) Haiku errors corrected
- E_timing E3/gaps and "Corrections": cites reachPlan at HolsterSlide.h:316-342 and says it is "not in HolsterReach.h". Wrong; it is HolsterReach.h:316-342 (H6 was right).
- E_timing S2 and E2 scenario 1 each describe one half: result depends on which read failed (owner 0 => hand pop + silent hip pop; combat-only failure => hand pop + full stow from the hand).
- E_timing S6 / E2 treat the retarget as pop-free; true for the slide fraction only. Missed the arm-weight collapse (ramp restarts at t0) and the path-position jump (H6 only partly).
- E_timing E1 "TRUE-ish" on desync: no desync possible since attach is persistent; but missed the arm IK override overlap with the first swing.
- H6 numbers verified; its "swap/death/cutscene already implemented" is right but the endpoint on unreadable is the hand, not a "safe" hip.
