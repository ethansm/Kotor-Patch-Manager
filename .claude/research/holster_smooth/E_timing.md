# E_timing: draw/stow timing, reach grab, and snap mechanisms (holster 0.11.0-dev)

Recon only (read-only). Anchors are repo-relative line numbers in the working tree. `H:` = patch_manager_mods/holster_src/.

## Verdict table

| Claim | Verdict | Anchor | Note |
|---|---|---|---|
| E1 draw 0..570, stow 0..1000, default 350/350 | PARTIAL | H:HolsterSlide.h:103-113, H:Holster.cpp:72-73 | Clamps and defaults match. 570 = latest `draw_weapon` anim-event time over 12 clips (0.03-0.57 s), not a verified attach time (42_holstered_weapons.md:52; event has no known consumer, :59 G4). Clamp is silent (H:Holster.cpp:329-335). |
| E2 reach_grab_pct global, 0..60, default 30, no override | TRUE | H:Holster.cpp:88, 394-397, 1485, 1753 | One static float; only the parse writes it, only two reach calls read it. Per-category keys exist for bulge and pole only (H:Holster.cpp:359, 375). |
| E3 no dwell; hand reverses at grab at full speed | PARTIAL | H:HolsterSlide.h:316-342 | No plateau: TRUE. Draw arrives at speed 0 and leaves at 1/(1-g). Stow arrives at full speed 1/(1-g) and releases from 0. "Full speed arrival" holds for stow only. |
| E4 snap/interrupt mechanisms | PARTIAL | section 2 | Seven paths checked. Engine re-attach happens on equip, never on draw. Two code paths cut or restart a slide. |
| E5 wall-clock, no per-frame rate dependence | TRUE (shape) / PARTIAL (visibility) | H:Holster.cpp:1527-1540, H:HolsterSlide.h:56-62, 84-88 | `now` = QueryPerformanceCounter ms. Slide math is wall-clock. Pose is written only when the engine attach refresh runs, so refresh cadence sets what is visible. |

## 1. Claim detail

**E1.** The 570 comes from the `draw_weapon` anim event (0.03-0.57 s, 12 clips; 42_holstered_weapons.md:52), labelled "vanilla draw window" at 42b_holster_remaining_plan.md:24 and R2_review.md:30. No engine consumer is verified. Raising the clamp cannot delay the first attack: the slide writes only the Gob pose (H:Holster.cpp:1747-1750), and hit/damage/impact timing ignore the Gob (R2_review.md:16, claim 4). The residual risk is cosmetic (weapon arrival vs swing start, R2_review.md:22). A desync from the attach cannot be measured yet because the rhand attach is persistent, not timed.

**E3.** Let g = reach_grab_pct/100 (default 0.30), S(u) = 3u^2 - 2u^3, p = elapsed/len.
- Draw, reach (p < g): pathF = 1 - S(p/g). Speed 0 at start and at grab; peak 1.5/g at p = g/2 (H:HolsterSlide.h:325-326).
- Draw, carry (p >= g): pathF = (p-g)/(1-g). Speed jumps 0 to 1/(1-g), about 1.43 (H:HolsterSlide.h:327-328).
- Stow, carry (p < c = 1-g): pathF = from*(1-p/c). Linear, speed about -1.43 right from the start (H:HolsterSlide.h:334-337).
- Stow, release (p >= c): pathF = S((p-c)/g). Speed 0 at start, peak 1.5/g (H:HolsterSlide.h:340).
- Grab = C0, not C1: velocity steps at the grab in both directions. Carry is not eased by easeSmooth(frac); the weapon follows via carryW = easeSmooth(q/0.1) (H:HolsterSlide.h:328, 337), the arm via reachWeightRamp ramp 0.04 (H:HolsterReach.h:180-189).

**E5.** Math: `frac(now) = (now - t0)/len` (H:HolsterSlide.h:56-62). `now` = `slideNowMs()` (H:Holster.cpp:1527-1540), QPC converted to integer ms. `GetTickCount` drives only hold and throttle timers (H:Holster.cpp:598, 913-918), not the slide. `touch()` freezes gaps over 100 ms and lets gaps of 100 ms or less advance in real time (H:HolsterSlide.h:84-88).

## 2. Snap / interrupt mechanisms

Ordered by how directly they cut or restart a slide.

**S1. Refused tick leaves the engine hand pose.** Trigger: PrReadIn, PrInsaneIn/Out, PrNoWrite, PrNoMount (H:Holster.cpp:1723-1749). Timing: any tick in the slide. Effect: the engine's hand pose (per-refresh copy at 0x504EF3, c4 (b)3) stays in gob+0xA4, so the weapon jumps to the hand for that tick. The slide clock keeps running. Not a restart.

**S2. Unreadable owner/combat read resets the slide to hand (restart).** Trigger: `rdU32Ok` fails on par, owner, or +0x138 (H:Holster.cpp:1664-1668). Unreadable means "drawn" (H:HolsterDrawGate.h:45-48). Timing: any tick. Effect: `ent->slide.reset(!hip)` = reset(hand), len 0 (H:Holster.cpp:1684-1685). A draw completes at once. A stow snaps to the hand and restarts with the full stow_slide_ms (H:HolsterSlide.h:65-80). If owner read as 0, the next readable tick sees an owner change and silently resets to hip (H:Holster.cpp:1676).

**S3. Ent rebuilt, silent placement (cut short).** Trigger: cache miss on (node, gob, handle), model change (GOBREUSE, H:Holster.cpp:1577-1581), or 128-slot round-robin overwrite (H:Holster.cpp:1566-1567). New weapon Gobs come from equip/swap (H:Holster.cpp:1618 gateInit). Timing: at equip or swap. An in-combat swap is action 0x0E, 1000 ms (42_holstered_weapons.md:51), so it can start inside a slide. Effect: `first` gives `reset(!hip)` with len 0 (H:Holster.cpp:1676, 1685), an instant jump with no slide. R2_review.md:42 (D8) accepts "new Gob = new Ent" as a known gap.

**S4. Engine equip re-attach (equip only, not draw).** Trigger: equip path 0x76D890 -> 0x7637E0, or `AppearanceForceUpdate`. Re-attach runs only if creature+0x278/+0x27C changed (research/holstered_weapons/ux_v3/G3_reattach.txt:7). Vanilla combat start does not re-attach (G3_reattach.txt:15). Effect: the attach-time copy of N into the child (.claude/research/holster_c4_weapon_attach.md (c) P2, site 0x4616F3) may show one hand-pose frame before the next refresh rewrites hip. GUESS: depends on render order.

**S5. Sparse refresh: freeze, then catch-up jump (most likely for a long slide).** Trigger: no call of attach vtbl+0x08 in a frame (caller UNKNOWN, c4 (b)5), or a frame stall. Timing: any tick; longer slides cover more gaps. Effect: gob+0xA4 keeps the last mod-written mid pose (R2_review.md:76), so the weapon freezes. The next write jumps to the current fraction. touch() freezes gaps over 100 ms (H:HolsterSlide.h:84-88), but a gap of 100 ms or less jumps up to 100 ms of slide. The SLIDE end log prints frames and maxdt (H:Holster.cpp:1709-1710) to test this.

**S6. Gate flicker, repeated reversals.** Trigger: holdActive, flourishOpenFor, or party_combat_hold flipping `hip` (H:Holster.cpp:1669-1671; party refresh every 250 ms, :914). Timing: per flip. Effect: `Slide.target` reverses from the current fraction (no pop, H:HolsterSlide.h:68) but restarts the duration as ms x remaining distance. This looks like an interrupt. Count "SLIDE start" lines (H:Holster.cpp:1698) to test.

**S7. Checked, not snap sources.** DELAY cut and toggle stow write only server +0x524 (H:Holster.cpp:855, 897; throttled 100 ms at :915). The Gob changes only once the combat flag clears, which is a retarget with no pop. The swap hook H3 logs only (H:Holster.cpp:538-547). A gate flip re-resolves the same placement (`seq = -1`, H:Holster.cpp:1677-1679); resolveFull's inputs do not include hip (H:Holster.cpp:1726).

**Most likely cause of "longer slide looks interrupted by a snap":** S5 (freeze then jump, scales with slide length), conditional on the refresh cadence (open). S2 and S3 are the only paths that actually end or restart the slide.

## 3. Gaps to build

- Log refresh gaps and slide interruptions: a line when touch() takes the >100 ms branch or reset() runs during an active slide (H:Holster.cpp:1684-1689).
- On a transient unreadable read, keep ent->slide and skip reset(hand) (H:Holster.cpp:1684-1685).
- Key the slide to the weapon item instead of the Gob pointer, so a re-equip keeps its fraction (R2_review.md:42 D8).
- Ease the carry ends: ease-in on stow carry start, ease-out on draw carry end (H:HolsterSlide.h:327-340).
- cfgWarn on draw/stow clamp (H:Holster.cpp:333-334), so 900 -> 570 shows in logs.

## Corrections to existing notes

- G3_reattach.txt:9 says bone_test repoints every engine attach to hip. That holds only outside drawgate: applyBoneTest returns early in drawgate (H:Holster.cpp:2117). Live is drawgate, so the attach stays on native rhand.
- reachPlan is in H:HolsterSlide.h:316, not HolsterReach.h (which has reachWeightRamp at :180).

OPEN: who calls the attach behavior's vtbl+0x08 each frame, and does the renderer read gob+0xA4 after the write (c4 (b)5; G3 OPEN)?
