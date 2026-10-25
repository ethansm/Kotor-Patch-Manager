# Doc 49: K2 Render Distance (fog, shadows; follow-on to doc 48 grass)

**Mod Directory:** `patch_manager_mods/k2_fog_distance_src/`
**Mod ID:** `k2-fog-distance` (`0.1.0-dev`)
**Status:** Built, gate PASS; awaiting install GO (new id: full reinstall)
**Owning Mod Scope:** `patch_manager_mods/k2_fog_distance_src/` (do not touch other mods)

---

## 1. What limits view distance (static recon 2026-10-25, Steam 6A522E71)

The in-game graphics options are already at their maximum (`swkotor2.ini` [Graphics Options]: Anisotropy 16, AA 4,
Texture Quality 2, Frame Buffer, Grass, Shadows, Soft Shadows all on). Remaining levers:

| Lever | Finding | Mod path | Source |
|---|---|---|---|
| Area fog | Scene +0x60 on / +0x64 GL_FOG_START / +0x68 GL_FOG_END, filled from .are SunFog*/MoonFog* (loader 0x00523870); read by Scene::DoFog 0x0046A980 (vtable slot 0x128 @0x0098BC6C), room pass 0x00466700, 0x004B3D80 | **this doc, k2-fog-distance** | asm 0x0046AA6C..0x0046AAB0 |
| Camera far clip | Camera +0x214 -> gluPerspective; world scenes set 10000 m at init (0x00987700) | none needed (not a limit) | `.claude/research/render_distance/R2_far_clip.md` |
| Particle emitters | no camera-distance cut; frustum test only (0x00494200) | none | `.claude/research/render_distance/R1_emitters.md` |
| Object shadows | cut at 35.0 m: `FLD [0x009F5D50]` (.data, writable) @0x00468984 in Scene::DoGobShadows 0x004687E0 and @0x00468D76; second cut dist^2 <= 1100 (~33 m, .rdata 0x0098BA98) @0x00468CDB in DoGobShadowsForThisLight 0x00468B50 (needs operand rewrite, like the grass wind FLD) | next phase **S1** | `.claude/research/render_distance/R3_shadows.md` |
| Shadow lights | candidate light cap `[0x00A0F040]` = 1 (.data) in 0x0050A630; live value unverified | S1 probe reads it | R3 F4 |
| Dynamic lights | Max Lights (3 -> 7) already installed | none | KPM `Patches/Max Lights` |

## 2. k2-fog-distance design

- Hook 1 detour at 0x0046A989 (`CMP dword ptr [0x00A20064],0`, 7 bytes, handler first, ECX = Scene). No overlap with
  KPM Shader Fixes (0x0046AADD) or K2 Grass Fog Fix (0x004665D8); `check_hook_spans.py` PASS.
- Per scene, `k2fog::FogTracker` remembers the area's own start/end; a value differing from our last write is new
  area fog (area load, day/night, script) and becomes the origin. Scaling never compounds.
- Fog-off areas (+0x60 == 0) are left alone. Fog Distance 0% = No Fog: start/end 0.99e6/1e6 (the Shader Fixes value).
- Start is kept at or below 95% of the applied end. Off restores the stored vanilla values next frame.
- Mod Configuration page "K2 Fog Distance": Extended Fog Distance toggle, Fog Distance 0..400% (0 = No Fog),
  Fog Start 0..400%, step 25, defaults 200%/200%. INI `k2_fog_distance.ini` [Fog], hot-reloaded.
- Log channel `area`: one line per distinct applied range (`area fog scene=.. vanilla=a..bm applied=c..dm`), capped 200.

## 3. Phases

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| **F0** | Static recon: fog fields, far clip, emitters, shadows (3 Haiku recon agents + main) | - | - | DONE | | R1-R3 in `.claude/research/render_distance/` |
| **F1** | k2-fog-distance 0.1.0-dev: DoFog hook, tracker, 3-row Mod Configuration page, tests | F0 | - | DONE | | `run_tests.py fog_distance` 2/2 PASS; GATE PASS |
| **F2** | Install + play FG1 | F1 | user GO | TODO | | `python3 scripts/kpm_reinstall.py --mod k2-fog-distance` (game closed) |
| **S1** | Shadow distance: write 0x009F5D50 + rewrite the 0x00468CDB FLD operand; probe the live 0x00A0F040 value; add rows to a K2 Shadow Distance page | F2 | - | TODO | | RE check of the 0x00468B50 reference point (Scene+0x118 = camera?) |

## 4. Verification

```bash
python3 scripts/run_tests.py fog_distance
./scripts/pre_install_gate.sh --mod k2_fog_distance_src --skip-default
python3 scripts/kotor2_logs.py check k2-fog-distance_log.txt
```
