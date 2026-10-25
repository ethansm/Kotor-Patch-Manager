# Doc 50: K2 Render Performance (grass, shadows, fog) — master plan

**Mods:** `k2-grass-draw-distance` (doc 48), `k2-fog-distance` (doc 49), new `k2-shadow-distance`
**Status:** planned 2026-10-25; Wave 1 next
**Goal:** every optimisation the engine allows for the grass, shadow and fog passes, each one switchable and measured,
so the gains add up and each change's cost or gain is visible in the logs.

---

## 1. Facts (Steam 6A522E71, all checked in the static dumps `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/`)

### 1.1 Grass pass
- `Scene::CollectAndTraverse` 0x00499610: octree query radius `FLD float ptr [0x00A0F074]` @0x00499ADC
  (`D9 05 74 F0 A0 00`) builds/collects in-range grass bins. The same global is the fade end in
  `CAurTriangleBin::RenderGrassPolys` 0x0050DAC0 @0x0050DB4C; fade start `[0x00A0F070]` @0x0050DB43; wind gate
  @0x0050DCC8 (already rewritten to the DLL's `g_windDistSq` by grass 0.1.1).
- With the alpha test, opaque texels clip at `sqrt(start² + (1-cutoff)(end²-start²))` = 138.8 m at 120..150 m / 0.40.
  Bins between the clip distance and the fade end are collected, built and drawn but cannot show a pixel
  (inferred from the fade formula, to be confirmed by the PF1a log + screenshot).
- `Scene::DoBinBucket` 0x004664E0 (grass draw): enables state, `glAlphaFunc(GL_GREATER, [0x009F5D38])` @0x00466615,
  then the bin loop: @0x0046661B `MOV dword ptr [EBP-0x4],0` (7 bytes `C7 45 FC 00 00 00 00`) starts it, inner loop
  over `[0x00A7335C]` list / `[0x00A73360]` count calls RenderGrassPolys @0x0046668F; the loop exits to
  @0x00466698 `FLD float ptr [0x00A20040]` (6 bytes `D9 05 40 00 A2 00`), which restores the alpha ref.
  K2 Grass Fog Fix owns 0x004665D8..0x004665F5 (30 bytes); KPM Shader Fixes owns 0x0046AADD. Neither touches 0x0046661B or 0x00466698.
- RenderGrassPolys issues per bin: glTexEnvi, glColor4f, glMaterialfv, glEnable/DisableClientState, client active
  texture `[0x00A32C38]`/`[0x00A32DA8]`, program env `[0x00A32DEC]`, and backend calls `[0x00A1BCB4]`; CPU wind
  `UpdateGrassForWind` 0x00510F70 inside the wind radius. One draw per bin (~1,000 bins at 150 m in dense areas).
- PERF history (doc 48): fps correlates only -0.36 with bins; the same bin counts ran 58-67 fps early and 37-45 fps later
  in one session, so grass is not the only cost; measure each change.

### 1.2 Shadows (`.claude/research/render_distance/R3_shadows.md`, gates re-checked in asm)
- Object cut 35.0 m: `FLD float ptr [0x009F5D50]` (.data, writable float 35.0) @0x00468984 in `Scene::DoGobShadows`
  0x004687E0 and @0x00468D76 in `DoGobShadowsForThisLight` 0x00468B50, both behind `CMP [0x009F5D54],0` (.data 1).
- Second cut dist² <= 1100 (~33.2 m): `FLD float ptr [0x0098BA98]` (.rdata) @0x00468CDB (`D9 05 98 BA 98 00`),
  around the shadow-volume build `CALL 0x00465C90` @0x00468DC1. Needs an operand rewrite like the grass wind FLD.
- Light candidate cap `[0x00A0F040]` (.data, static 1) in 0x0050A630 @0x0050A74B/0x0050A881/0x0050A950/0x0050A95B;
  live value and meaning are unverified, so read and log it first and do not write it.
- 0x00468B50 entry: `55 8B EC 81 EC 8C 00 00 00` (9 bytes; PUSH EBP / MOV EBP,ESP / SUB ESP,0x8C), ECX = Scene, no relative branch.

### 1.3 Fog
- GL fixed-function fog per fragment: no cost to cut. Doc 49 `k2-fog-distance` only moves it. Culling at the fog end
  (camera far plane = fog end) is **rejected**: skyboxes are geometry and would be clipped.

### 1.4 Not levers
- Camera far plane 10,000 m in world scenes (R2). Particle emitters: frustum only, no distance cut (R1).
- Max Lights 3 -> 7 is already installed (KPM `Patches/Max Lights`).

---

## 2. Logging standard (every phase)

1. **Patch log at init**, one line per site: `patch <name> @0x<addr> expect=<bytes> seen=<bytes> rc=<1|0|-1>`
   (`codepatch::writeChecked`). A mismatch writes nothing, logs WARN, and the feature stays off.
2. **Settings line** on load and on every change: all effective values, including which optimisations are on.
3. **PERF line every 5 s** (existing grass window, `kPerfGapMs` drop): fps plus per-feature counters, e.g.
   `grass_ms_avg/max` (CPU time of the DoBinBucket bin loop, QPC), `bins` (drawn), `collected`, `trimmed`, `a2c`,
   shadow `casters/s` and `dist`. Each optimisation has an ini/menu toggle so A/B runs show its delta.
4. **Once-only capability lines:** GL facts the feature depends on (e.g. `GL_SAMPLE_BUFFERS`, `GL_SAMPLES` for A2C).
5. Logs stay under ~50 startup lines; per-frame detail only with `verbose = 1`. `kotor2_logs.py check` must read CLEAN.

---

## 3. Phases

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| **PF0** | Recon + plan (this doc); fog mod built (doc 49 F1) | - | - | DONE | c5a6e142 | doc 50 master plan; recon R1-R3; fog built (doc 49 F1) |
| **PF1a** | grass 0.1.4-dev: collect radius = clip + 2 m (operand rewrite 0x00499ADC -> `g_collectSq`, toggle `trim_hidden`), bin-loop CPU timing (detours 0x0046661B / 0x00466698, QPC), alpha-to-coverage toggle `smooth_edges` (glEnable/glDisable `GL_SAMPLE_ALPHA_TO_COVERAGE` 0x809E in the same detours, gated on GL_SAMPLE_BUFFERS), PERF + 2 menu rows | PF0 | - | BUILT | 3467b4ec | grass 0.1.4-dev; KPM dae7f2b; GATE PASS; play GR5 |
| **PF1b** | new `k2-shadow-distance` 0.1.0-dev: Shadow Distance slider (vanilla 35 m; writes 0x009F5D50 and rewrites 0x00468CDB operand to `g_shadowRefSq` = dist² x 1100/1225), entry hook 0x00468B50 (ini poll, casters counter), logs live `[0x00A0F040]`/`[0x009F5D54]`, menu page | PF0 | - | BUILT | 3467b4ec | k2-shadow-distance 0.1.0-dev; KPM dae7f2b; GATE PASS; play SH1 |
| **PF1c** | Recon for Wave 2 (read-only): RenderGrassPolys per-bin state and draw path, `[0x00A1BCB4]` backend, wind update 0x00510F70 data flow, quad/index layout for LOD | PF0 | - | TODO | | 2 Haiku recon, out `.claude/research/render_perf/` |
| **PF1d** | Install wave: fog F2 + grass 0.1.4 + shadow 0.1.0 in one `kpm_reinstall.py` (new hooks/ids) | PF1a, PF1b | user GO | TODO | | play rows GR5, SH1, FG1 |
| **PF2a** | Redundant per-bin state hoist (set texenv/material/client state once per bucket) | PF1c, PF1d | - | TODO | | from PF1c findings |
| **PF2b** | Wind throttle: update wind for bins past N m every k-th frame | PF1c, PF1d | - | TODO | | |
| **PF2c** | Far-grass LOD: draw a fraction of a far bin's quads (index count), density compensation by alpha | PF1c, PF1d | - | TODO | | |
| **PF2d** | GPU timing of the grass pass (`GL_ARB_timer_query` if present) into PERF | PF1d | - | TODO | | |
| **PF3** | Defaults from A/B logs; log tightening (`perf_log` default off, verbose gating) | PF2* | - | TODO | | |

Wave 1 (PF1a, PF1b, PF1c) runs in parallel: disjoint write scopes, no shared header edits (`_shared_src` read-only).
