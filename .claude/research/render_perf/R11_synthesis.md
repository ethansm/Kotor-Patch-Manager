# R11: render/engine performance levers beyond grass/fog/shadow (synthesis 2026-10-25)

Sources: R7_fbo_flush, R8_models_anim, R9_rooms_vis, R10_emitters_pacing (Haiku static recon, anchors re-checked
by Opus in the Steam 6A522E71 asm), engine_research_findings/R7.md (per-frame costs), live game dir state 10-25.

## Live facts (10-25)
- `perflab_debug.txt` active since 10-05: `alloc=1 heapfilter=1 scan=8 metrics=1 vq=1` -> every malloc/free goes
  through IAT hooks, 17 metric detours, VirtualQuery scans; `perflab_metrics.txt` = 97 MB / 1.9M lines in one session.
  Same metrics header confirms `a1b710=0 a1b704=0` (engine limiter and Sleep(0) dormant).
- Stale debug flags still on: `npc-diversity_debug.txt` (2.9 MB log/session), `buffhud_`, `chareffects_`,
  `equip3d_`, `inv3d_`, `hangprobe_debug.txt`.
- swkotor2.ini: V-Sync=1, RefreshRate=60, Frame Buffer=1; monitor 3840x1600 @ 74.48 Hz; RTX 5070, driver 595.84;
  no Steam launch options. Grass PERF (17:18 session): 37-50 fps at ~1,000 bins, so frames miss V-Sync.

## Ranked levers
| # | Lever | Kind | Value | Risk | Anchor |
|---|---|---|---|---|---|
| 1 | Turn perf-lab research mode and stale `*_debug.txt` flags off (rename to `.off`) | config | high (CPU, disk) | none, reversible | game dir |
| 2 | NVIDIA env A/B: `__GL_THREADED_OPTIMIZATIONS=1` (immediate-mode GL, ~1,000+ draws/frame), `__GL_YIELD=USLEEP` (no spin in the V-Sync wait) | launch option | med-high, unmeasured | low (A/B) | Steam launch options |
| 3 | Idle when minimised/unfocused: loop spins with no swap and no Sleep when client width = 0 | new mod (frame pacing) | high for idle power | low | 0x00408427 JNZ / 0x0040842B CALL 0x436B90; Sleep gate 0x00408475 on [0xA1B704] |
| 4 | Optional fps cap with Sleep+short spin (e.g. 60 on a 74 Hz monitor) | same mod | med (GPU/CPU use) | low | limiter 0x0040847F ([0xA1B710], QPC spin) |
| 5 | Skip/throttle Gob animation for objects not in view (Gob+0x11 written by scene callback, never read by Gob::Animate) and far away | new hook | med-high in crowded areas (npc-world adds NPCs) | med-high (anim events, sounds, attachments) | Scene::Animate 0x0046D230 CALL EDX @0x0046D367; Gob::Animate 0x004E1D40 gate [0x00A04A20] @0x004E1D7C |
| 6 | Texture-id scan 0x00427970 runs 3x/frame (full grid walk + vector copy): run once per frame, reuse | hook | med | low-med | calls @0x00466033, 0x0046630E, 0x0046673F; entry `55 8B EC 83 EC 10` |
| 7 | GlobalMemoryStatus 2x/frame (Wine reads /proc/meminfo): cache, refresh 1x/s | IAT hook | low-med, unmeasured | low | 0x00478A20 |
| 8 | Per-bucket blend/depth/alpha state cache (0x004DB1F0), per-mesh map lookups (0x004660F0/0x00466110), fog save/restore in pass C (0x00466700) | hook | low-med | low-med | R9 levers 2,4,5 |
| 9 | Emitter camera-distance cut (visual change, slider) | hook | med in effect-heavy fights | low | 0x0049AB43 |

## Not levers
- glFlush: default path issues none per frame (sites dead/fallback). Nothing flushes before SwapBuffers (R7_fbo_flush).
- Room VIS already applied per frame (0x00499300); no occlusion, but a distance cut on rooms would pop geometry.
- Frame Buffer effects: 1-3 full-screen glCopyTexSubImage2D/frame; only lever is the ini toggle (loses effects).
- Off-screen emitters already skip simulation (frustum list); light selection 0x00508CA0 caller unknown.
- Skinning: CPU palette build per drawn mesh only (0x004C9640); already gated by visibility.

## Status
- Lever 1: perf-lab turned OFF 2026-10-25 (`perflab_debug.txt` -> `perflab_debug.txt.disabled`; note in
  `patch_manager_mods/perf_lab_src/README.md`). Also renamed to `.disabled` the same day: `buffhud_debug.txt`,
  `inv3d_debug.txt`, `chareffects_debug.txt` (rename back to re-enable). Other `*_debug.txt` flags kept: `hangprobe_debug.txt` (LHP-PT1),
  `npc-diversity_debug.txt` (WI-119), `equip3d_debug.txt` (EQ10-P1) are play-row signals; drop each after its row passes.

## Order
1-2 first (no code, measure with grass PERF + GPU timing), then a frame-pacing mod (3-4), then a
measurement probe for 5-7 (per-frame CPU timing of Scene::Animate, 0x427970, GlobalMemoryStatus) before
building any of them.
