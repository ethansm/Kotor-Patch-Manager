# J0-o (3C-FD) vs KPM `k2_aspyr_shader_fixes`: gap analysis

Static analysis only (no game launch, no game-dir writes, no edits in the kotor-patch-manager repo).
Date 2026-10-02. Vanilla = `backups/vanilla_clean/swkotor2.exe` (6A522E...), J0-o = 3C-FD exe (4AB72F...).
Scripts that produced every number below are in `tools/` (see end).

## Summary table

| # | J0-o change (VA) | What it is | KPM `k2_aspyr_shader_fixes` 1.3.3 | Class | Where the equivalent lives / action |
|---|---|---|---|---|---|
| 0 | PE byte 0x11E (LAA) | 4 GB flag | not in ShaderFixes | COVERED (by `4gb-patch`, live) | none |
| 1 | `0x989DB0` gamma-pass fragment program: `MUL r.rgb,.90/.85/.90` before `POW` | colour grade ("remove Aspyr yellow tint", ~10% darker), runs in the full-screen gamma pass | no hook at this VA | **MISSING** | standalone KPM patch `colorshift_k2` (K2AspyrColorCorrection) is byte-identical to J0-o and NOT installed. Add 1 simple hook (266 B) or install the standalone patch |
| 2 | `0x990EC8` lightmap program: fog via `fragment.color.secondary` + `LRP` | fog on lightmapped world geometry | replaced by `OPTION ARB_fog_linear` program with `ADD_SAT`, keeps alpha | CONFLICTING, KPM better | keep KPM; J0-o drops vertex-alpha multiply |
| 3 | `0x990918` diffuse+cube-env program: same fog edit | fog on reflective surfaces | `ARB_fog_linear` version | CONFLICTING, KPM equal/better | keep KPM |
| 4 | `0x990020` lightmap+env program: `texture[2]` 2D->CUBE + fog | fixes malformed reflections + fog | CUBE + `ARB_fog_linear` + `MAD_SAT`/`ADD_SAT` | CONFLICTING, KPM better | keep KPM |
| 5 | vertex programs `0x9F7B80`, `0x9F7FC8`, `0x9F8580`: add `result.color.secondary.x` = linear fog factor | feeds the fog factor to J0-o's fragment programs | not touched (ARB_fog_linear reads `result.fogcoord`, already written by all three) | COVERED by design (different mechanism) | none; do NOT port (would be dead code) |
| 6 | `0x70E130` 3-byte NOP of `fmul dword [ebp-0x28]` | **audio, not fog**: removes the music/ambient "ducking" multiplier (x0.2 while a conversation runs) | not touched | **MISSING** (but exists as standalone) | KPM patch `K2_music_fix` (K2AspyrMusicFix), byte-identical, NOT installed. Install it; do not port into ShaderFixes (unrelated to shaders, same-address overlap if both ship) |

KPM-only items J0-o never had (so nothing to port back): +15 brightness offset removal (4 hooks), fog-end fix for fog-less areas (`0x46AADD`), fog on `0x98FE50`/`0x98F068`, skinned-mesh fade alpha (`0x98F950`, #278), alpha fix `0x98F068`, hilt texcoord (`0x4313A2`), normal-map path (`0x4A715E`).

Headline: **J0-o = the older trio ColorCorrection + FogReflectionFix + MusicFix (+LAA).** Account for all 5,124 bytes: PE 1 byte, `K2AspyrFogReflectionFix` covers 4,876, `K2AspyrColorCorrection` 244, `K2AspyrMusicFix` 3; nothing is uncovered (`coverage_accounting.py`). `K2AspyrShaderFixes` supersedes FogReflectionFix; the other two are separate KPM patches that are not in the live 28-id set (live state checked in `kpm_install_state.json`: no `colorshift_k2`, no `K2_music_fix`, no `fog_reflection_fix_k2`).

Correction to `README.md` in this dir: the 0x70E130 NOP is **not** a fog clamp bypass; it is the music-ducking disable.

## 1. Shader programs J0-o changed (vanilla vs J0-o)

PE sections: .text 0x401000, .rdata 0x986000, .data 0x9F4000. 18 diff clusters collapse to 8 programs + 1 code site. J0-o edited in place and used leading NUL padding after each string, so no neighbouring string is touched (checked: every diff byte lies inside the program text or its NUL padding).

### 1.1 `0x989DB0` (.rdata, file 0x5889B0), gamma pass fragment program
Used by `FUN_00436b90`, the full-screen gamma pass: copies the framebuffer to a `GL_TEXTURE_RECTANGLE` (0x84F5), draws a quad with this program, `env[1].x` = gamma exponent set by `FUN_00478870` (`SetGamma(float)`). The pass only runs when the exponent != 1.0 (`_DAT_009f5ef4 != 1.0`).
```
vanilla                                   J0-o (266 bytes, 10 more than vanilla: uses NUL padding)
TEX r0,T0,texture[0],RECT;                TEX r0,T0,texture[0],RECT;
POW r0.r,r0.r,exponent.r;                 MUL r0.r,r0.r,.90; MUL r0.g,r0.g,.85; MUL r0.b,r0.b,.90;
POW r0.g,...; POW r0.b,...;               POW r0.r,r0.r,ex.r; POW r0.g,..; POW r0.b,..;
```
Plain terms: scales the final image before the gamma curve: red/blue x0.90, green x0.85, i.e. about 10% darker with slightly less green. Not a fog/lighting bug fix; a colour-grading choice (K2AspyrColorCorrection: "Adjusts color to remove yellow tint from the Aspyr Update").

### 1.2 `0x990EC8` (.rdata) lightmapped diffuse: `tex0 * (vertexLight + lightmap)`
```
vanilla: ... ADD r2.rgb,v0,r1; MUL r0.a,v0,r0; MUL r0.rgb,r0,r2; MOV result.color,r0; END
J0-o   : ATTRIB f=fragment.color.secondary; ... ADD k.rgb,v,l; MUL d.rgb,d,k;
         LRP d.rgb,f.x,state.fog.color,d; MOV result.color,d; END
```
J0-o adds manual fog (blend toward `state.fog.color` by the secondary-colour fog factor from the vertex program) but **drops `MUL r0.a,v0,r0`** (vertex alpha x texture alpha), so fading/transparent lightmapped surfaces lose their fade.

### 1.3 `0x990918` (.rdata) diffuse + cube environment map (no lightmap)
```
vanilla: ... MUL r0.rgb,r0,v0; SUB rTemp0,1.0,r0; MAD r0.rgb,rTemp0.a,r1,r0; MOV r0.a,c[0]; MOV result.color,r0; END
J0-o   : same body (renamed regs) + ATTRIB f=fragment.color.secondary; LRP d.rgb,f.x,state.fog.color,d before the MOV
```
Pure fog addition.

### 1.4 `0x990020` (.rdata) lightmap + environment (third texture)
```
vanilla: TEX r2,T2,texture[2],2D;  ... ADD r0.rgb,r3,r4; MOV r0.a,c[0].a; MOV result.color,r0
J0-o   : TEX m,U,texture[2],CUBE;  ... + ATTRIB f=fragment.color.secondary; LRP d.rgb,f.x,state.fog.color,d
```
Two edits: the 2D->CUBE fix (vanilla samples the env texture with the wrong target = "malformed reflections") and fog.

### 1.5 Vertex programs `0x9F7B80` (1094 B), `0x9F7FC8` (1462 B), `0x9F8580` (1462 B) (.data)
Statement-level diff (names normalised; `tools/vp_stmt_diff.py`), identical for all three apart from register-list reshuffling:
```
+ PARAM f=state.fog.params
+ ADD vReg1.x,-f.y,vReg0.x          ; dist - fogStart       (vReg0.x = eye distance, already computed)
+ SLT t.x,0.0,state.fog.color       ; t.x = (0 < fog.color.x)  -> "fog on" gate
+ MUL t.y,vReg1.x,f.w               ; * 1/(end-start)       (f.w)
+ MUL result.color.secondary.x,t.y,t.x   ; fog factor goes out as secondary colour R (clamped 0..1 by rasteriser)
```
So J0-o's fog = per-vertex linear fog factor in secondary colour, consumed by the three edited fragment programs. Fragility: the gate is "fog colour red > 0", not "area has fog"; fog-less areas keep stale `glFog*` state (`FUN_0046a980` only calls `glDisable(GL_FOG)` when `area+0x60 == 0`), so a stale non-black fog colour could still fog in fog-less areas. KPM handles that explicitly (see 3.1).

Vertex programs not touched by J0-o: 26 other `!!ARBvp1.0` programs; all except four (0x9F50F0, 0x9F53B8, 0x9F5770, 0x9FDD78) already write `result.fogcoord`, so KPM's `ARB_fog_linear` works on every path J0-o patched and more.

## 2. KPM `k2_aspyr_shader_fixes` hooks (Steam, 13 simple hooks) mapped to vanilla programs

Decoded by `tools/kpm_hooks.py`; every hook's `original_bytes` equals vanilla (the live install proves it too). Engine setup of all fragment programs: `FUN_004b4690` (xrefs at 0x4B46C5 etc.).

| Hook VA | Vanilla program it replaces (engine ref) | KPM replacement | J0-o touched it? |
|---|---|---|---|
| `0x990EC8` (272 B) | lightmapped diffuse (`0x4B46C5`) | `OPTION ARB_fog_linear`, `ADD_SAT` on light+lightmap, alpha kept | yes (1.2) |
| `0x990020` (459 B) | lightmap + env, tex2 as 2D (`0x4B495E`) | `ARB_fog_linear`, tex2 -> `CUBE`, `MAD_SAT`/`ADD_SAT` | yes (1.4) |
| `0x98FE50` (461 B) | lightmap + env, tex2 already `CUBE` (`0x4B49B4`) | same as above (fog + saturate) | **no** |
| `0x990918` (344 B) | diffuse + cube env (`0x4B4836`) | `ARB_fog_linear` | yes (1.3) |
| `0x98F068` (349 B) | lightmapped, `DP3 ... c7` luminance/tint variant (`0x4B4C6A`) | `ARB_fog_linear`; fixes alpha (`MUL r0.a,r0.a,v0.a`, vanilla wrote `r2.a`, a dead register) | **no** |
| `0x98F950` (274 B) | skinned-mesh fog blend, already has `LRP r0.rgb,v1.r,c[1]` fog (`0x4B4ADC`) | adds `MOV r0.a,v0.a` so a fading character keeps its fade under fog (#278) | **no** |
| `0x46AADD` (21 B) | in `FUN_0046a980` (area fog setup): `glDisable(GL_FOG)` when area has no fog | `glFogf(GL_FOG_END, 1.0e6)` (+NOPs) (#280: 1e6, not FLT_MAX) | no |
| `0x7D5997`, `0x7D59A2`, `0x7BBFA0`, `0x7BBFAF` | `add reg,0xF` = Aspyr's +15 brightness offset (brightness-slider mapping in `FUN_007D5970`-area and `FUN_007BBF90`) | `add reg,0` | no |
| `0x4313A2` (18 B) | `glDisableClientState(texcoord array)` after particles | NOPs (hilt texcoord array stays on) | no |
| `0x4A715E` (182 B) | render-path selector | restores combined env+bump+tangent normal-map path | no |

`K2GrassFogFix` (`0x4665D8`, live) and the other Aspyr patches are independent and untouched by J0-o.

## 3. Per-change verdicts

### 3.1 Fog (items 2-5): KPM covers it, differently and better
- Mechanism: J0-o = hand-rolled per-vertex fog in secondary colour + `LRP` in 3 fragment programs + 3 vertex programs. KPM = `OPTION ARB_fog_linear` (driver applies fog per fragment from `result.fogcoord`), hooks `0x46AADD` so fog-less areas get `GL_FOG_END = 1e6` instead of `glDisable` (ARB_fog options ignore `GL_FOG` enable, so a finite huge end is needed; #280 fixed the FLT_MAX denormal black-surface bug on AMD).
- KPM adds fog to two more programs (`0x98FE50`, `0x98F068`) that J0-o left unfogged, saturates overbright sums (`ADD_SAT`/`MAD_SAT`), keeps vertex alpha in `0x990EC8`, fixes skinned fade (#278), and does not depend on a black-fog heuristic.
- Visual difference to expect after the migration (not a regression): reflective/lit surfaces no longer blow out above 1.0; previously unfogged variants now fog. Vertex-programs: KPM needs no change.
- Verdict: COVERED / CONFLICTING-KPM-better. **Do not port J0-o's fog.** Porting would also create overlapping hooks at 0x990EC8/0x990918/0x990020/0x9F7xxx (the KPM FogReflectionFix patch conflicts the same way and is intentionally not installed).

### 3.2 Gamma-pass colour grade (item 1): MISSING
Not provided by ShaderFixes. Interaction: ShaderFixes removes Aspyr's +15 brightness offset (the 4 `add ..,0xF` hooks in the brightness-slider mapping code at 0x7D5970/0x7BBF90; the exact path into `SetGamma` was not traced), which already shifts the exponent. J0-o (3C-FD) kept the +15 and applied the x.90/.85/.90 grade, so the two designs are different takes on the same "Aspyr looks washed/yellow" complaint. Installing both makes the picture darker than J0-o's and darker than ShaderFixes alone; this is a taste call and needs one visual A/B in game (cannot be judged statically). Because the pass is skipped when exponent == 1.0, the grade has no effect if the slider ever lands exactly on 1.0.

### 3.3 `0x70E130` NOP (item 6): audio ducking, not fog
Function: `FUN_0070dfe0` = `CExoSoundSourceInternal::SetVolume(byte vol, int flag/useCurrent, float)` (ret 0xC; this+0x24 = `CExoSoundInternal*`; `this+0x0C` = source type byte; `this+0x154` = per-source gain byte; ends with `AIL_set_sample_volume_levels` via `[0x986538]`). Decompile already in `ghidra_dumps/steam_6a522e71/decomp/70/0070dfe0.c`; no new Ghidra run needed (knowledge DB had only the neighbours; static dump + capstone sufficed).
```
slider = GetSliderType(this)           ; 0x70A320: types 8,9,0x10,0x11,0x1A -> 1 (dialog); types 1,2 -> 3 (music); else 2 (effects)
v = byte[this+0x154]/127 * GetSliderVolume(slider, flag)
if (slider == 3 || type == 4 || type == 5)     ; music streams and type 4/5 (ambient)
    v *= *(float*)(CExoSoundInternal+0)        ; <-- 0x70E130: fld [ebp-0x10]; fmul [ebp-0x28]; fstp [ebp-0x10]
level = ftol(vol * v / 127); AIL_set_sample_volume_levels(handle, level, level)
```
`*(float*)CExoSoundInternal` is the ducking factor: `StartDuckingMusicAndAmbientSounds` (`FUN_00704D30`) writes `_DAT_00995174` = **0.2**, `StopDucking...` (`FUN_00704D50`) writes 1.0, each followed by `ApplyNewVolume` (`FUN_00709BE0`). The only caller chain: `CSWSDialog` ctor (`FUN_006C42C0` -> `FUN_0070B4A0`) starts ducking, dtor (`FUN_006C47A0` -> `FUN_0070B4C0`) stops it. So in vanilla music and ambient loops drop to 20% for the whole conversation.
J0-o: `D8 4D D8` -> `90 90 90` (fld/fstp pair stays balanced, value unchanged): music/ambient stay at full slider volume during dialogue. Harmless to stack/FPU.

KPM coverage: **no patch in `k2_aspyr_shader_fixes`**; the exact same edit exists as `Patches/K2AspyrMusicFix` (id `K2_music_fix`, "Disables audio ducking...", 8-byte pattern `D8 4D D8 D9 5D F0 DB 45` at 0x70E130, targets both 6A522E and 4AB72F). It is not in the live 28-id install. No other KPM hook lies in 0x70DFE0-0x70E17C (searched all `Patches/*/kotor2-steam-aspyr*.toml`).
Recommendation: **skip porting; install `K2_music_fix`** if the user wants J0-o's no-ducking behaviour (it changes dialogue mix: music no longer dips under voice-over, which some find too loud; reversible).

## 4. Proposed additions to `k2_aspyr_shader_fixes` (only if consolidating)

Generated and verified by `tools/coverage_accounting.py` -> `tools/proposed_hooks_steam.toml` (identical to J0-o bytes; neither hook overlaps any of the 13 existing ShaderFixes hook ranges).

1. **Gamma colour grade** (optional): new simple hook, address `0x989DB0`, `original_bytes` = 266 vanilla bytes (256-char program + NUL + 9 NUL padding; the next non-zero data at +270 is untouched), `replacement_bytes` = the J0-o 266 bytes. Length is fixed at 266 because the program's NUL padding ends at +269. No overlap with any existing hook. Equivalent to copying the single hook of `K2AspyrColorCorrection`.
2. **Music ducking off** (not recommended inside ShaderFixes): simple hook at `0x70E130`, 8 original bytes -> 3 NOPs. Unrelated to shaders; keep as `K2_music_fix`.
If either is folded in, add the standalone id to ShaderFixes' `conflicts` (`colorshift_k2`, `K2_music_fix`) so two patches never claim the same bytes, and bump the version. Keep the Steam LAA hash alias (`4AB72F`) support in mind: the standalone patches already list it.

No vertex-program edits are needed.

## 5. Recommendations (priority)
1. Do not port any of J0-o's fog/vertex-program edits; KPM's are a superset.
2. For parity with 3C-FD without code work: install `K2_music_fix` (zero-risk, ducking off) and optionally `colorshift_k2` (visual A/B first, because ShaderFixes' +15 removal already changes brightness).
3. Fix the description in `vanilla_migration/README.md` ("fog clamp bypass" -> "music ducking disable").
4. If a single patch is desired, copy the two blocks from `tools/proposed_hooks_steam.toml` (hook 1 at least), add `conflicts`, test on the Steam exe only (GOG/Linux/macOS addresses differ and were not verified here).

## Tools (`patch_manager_mods/vanilla_migration/tools/`)
- `j0o_diff.py` PE section map + byte diff grouping. `j0o_dump.py` full vanilla/J0-o text per changed program.
- `kpm_hooks.py [toml]` decode hooks to text, check vs vanilla and J0-o. `match_patches.py` which KPM patches equal J0-o bytes.
- `vp_stmt_diff.py` vertex-program statement diff. `coverage_accounting.py` every diff byte accounted + proposed hook emitter. `disasm.py` (needs repo `.venv` capstone), `xref.py` call/pointer xrefs.
