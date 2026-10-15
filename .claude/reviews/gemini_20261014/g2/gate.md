# G2.3 Opus gate (2026-10-14)

Scope per plan: every real=yes with impact crash/corruption (none were returned), 20% spot check of the rest, every real=no that
overturns a G1 CONFIRMED (none: the 6 real=no rows were all G1 PARTIAL/UNSURE), plus every G1-CONFIRMED P1 that a skeptic downgraded to latent.
Each line: fid | decision | lines read by Opus | reason.

## P1 G1-CONFIRMED -> latent (all agreed)
- CO-2c-02, CO-2d-07, HB-1c-01, SH-3a-02 (X-T01 wr) | AGREE latent | _shared_src/GuiKit.h:123-127, 24 wr<> call sites (GuiKit.h:168-334, GuiTip.h:48) | every target is a live engine heap GUI control or creature field; a readable-but-read-only target is not reachable, and probeWrite would not catch the realistic stale-freed-heap case either. Hardening only.
- SH-3a-04 (SH-T03 writeChecked) | AGREE latent | _shared_src/CodePatch.h:87-96; callers ActivationFix.cpp:79, HolsterBonePatch.h:21, PerfLab.cpp:803/833/841 | every caller passes a constant swkotor2.exe .text site with n<=12; memcpy before memcmp only matters for an unmapped site. Hardening only.
- HB-1a-02 (HB-T01 TwoDA wrap) | AGREE latent | action_hotbar_src/TwoDA.h:74-90 | the DLL is 32-bit so size_t can wrap, but it needs nrows*cols > 2^31 from an override 2DA (~43k columns); stock data cannot trigger it. Hardening only.
- HB-1a-04 (X-T02 readAt wrap) | AGREE latent | as G1 gate | read-only; worst case is a wrong value.

## Spot checks of real=yes (med runtime rows, all 5 read)
- QH-2a-02 | AGREE yes/med | queue_hud_src/QueueHud.cpp:778-783, _shared_src/ActQueue.h:27 | engineNe counts the follower's permanent 0x3D FollowLeader entry, so companions never look idle to decideAction.
- SH-3a-05 | AGREE yes/med | _shared_src/GuiKit.h:292-302 | a null loadImage result is pooled and returned forever; lazy retries can never succeed.
- CO-2d-03 | AGREE yes/med | companion_selfheal_src/CompanionSelfHeal.cpp:707-708 | keyFor(0) is the slot name "slot0", so leader settings follow the slot, not the creature, across a leader swap.
- K1-5a-01 | AGREE yes/med | k1_port/k1_port_tex_dxt.py:222-233 | the DXT1 mode-match mask silently drops edited blocks whose parent is 3-colour; deliberate alpha protection but no report.
- CO-2d-09/QH-2a-11/SH-3a-06 | AGREE yes/low (log-only, one root cause) | per skeptic evidence EmbedTex.h:149-156 | heals counts attempts; give-up log fires on a successful 20th fill.

## Not overturned
No skeptic row was changed by the gate. real=no rows (6) go to refuted.md with the skeptic's disproving line; unsure rows (4: CO-2d-02, SH-3a-08, HN-6n-03, QH-2a-03) need runtime data and go to work items as verify-only.
