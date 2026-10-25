# R5: grass wind + far-bin LOD (Steam exe, static dumps only)

D = /home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp
`c:N` = D/<dir>/<file>.c line N. `a:0xADDR` = asm dump instruction. Steam VAs.

## Q1 UpdateGrassForWind 0x00510F70 (sole caller 0x0050DAC0; c:1)
1. Gates inside the function:
   - bin byte [+0x02] bit 0x04 must be set: a:0x510F7F-0x510F8F; c:33
   - global [0x00A0F080] != 0: a:0x510F96-0x510F9D
   - vtbl[+0xA8] of object [0x00A20020] returns true (meaning UNVERIFIED): a:0x510FA5-0x510FB8
2. Per-bin caller gate (RenderGrassPolys): [0x00A0F078] != 0 AND float [bin+0x04] < float [0x00A0F070]. a:0x50DCB6, 0x50DCC5-0x50DCD5; c:77-79.
   Copy bin+0x38 to bin+0x3C, call, zero bin+0x3C: a:0x50DCE6, 0x50DCF3, 0x50DCFE.
3. Per-call setup: amplitude clamp a:0x510FC1-0x510FEE; slot strength float [DAT_00A7D698 + [bin+0x10]*0x60 + 0x24] a:0x511116; distance attenuation K/|v| with K=[0x00991370], |v| from FUN_005142f0(this+0x18) a:0x5110F4, 0x511120-0x511136; c:72-78.
4. Per quad (loop bound [bin+0x0C] at a:0x511163; c:80):
   - FUN_00507250 sampler, scale 1.0: a:0x511224. FUN_00507680 -> FUN_005076c0 (scale = amp): a:0x51129A.
   - Trig: none in loop body. Any trig/noise is inside callees, UNVERIFIED.
   - x87 math about 40 ops. Stores: v3.x a:0x51144B, v3.y a:0x511463, v2.x a:0x511482, v2.y a:0x5114A0. v2 = v3 - v0 + v1 (c:121-124).
   - Write skipped when displacement is exactly zero: a:0x511412.
   - Integrator (no dt): v3 += g*(wind + noise + (v0 - v3)), g = amp*slot*[0x00A0F084] (c:79, c:106-124).
5. Cost: per quad 2 out-of-line calls, about 40 FP ops, 4 float stores (2 of 4 verts written). Per call 1 extra call (FUN_005142f0).
6. GL: no GL calls in 0x510F70. No glBufferData/glBufferSubData in D/50/0050dac0.c or D/43/004369a0.c. Draw path binds GL_ARRAY_BUFFER=0 (glBindBuffer 0x8892,0) at a:0x4369B3, a:0x436AA2. So vertices are client arrays; the driver re-reads them each draw. No VBO re-upload in the wind code.

## Q2 Bin layout and draw count (RenderGrassPolys 0x0050DAC0; helper 0x004369A0)
- Bin fields (byte offsets, from c:33-94 of 0050dac0.c):
  - +0x00 u16 S0. Normal/UV stride = S0-12 (c:94 arg; a:0x50E157).
  - +0x02 flags: 0x04 visible gate, 0x08 shader select.
  - +0x04 float: dist squared (writer UNVERIFIED; hooks.toml:8 says thresholds are squared).
  - +0x0C dword: quad count.
  - +0x10 int: slot index into 0x60-stride table DAT_00A7D698.
  - +0x38 vertex ptr (fixed draw base). +0x3C wind scratch. +0x40 inline base (shader path). +0x54 material. +0x58 per-group quad counts. +0x5C group count N. +0x64 group objects.
- Vertex layout: positions vec3 float, 12 B, 4 verts per quad (0x30 B/quad), contiguous from base (a:0x4369CF, stride 0xC).
  Normal/UV block at base + count*0x30 (a:0x4369DB), stride S0-12: normal +0x00, uv0 +0x0C, uv1 +0x14 (a:0x4369FD, 0x436ACB-0x436B36).
- No index buffer. glDrawArrays(GL_QUADS=7, first, count<<2): a:0x436A8B-0x436A93 and a:0x436B7C-0x436B86. first = accumulated quads*4 (c:178, c:236).
- Count reads (each feeds FUN_004369A0 arg6):
  - Site C, single draw, first=0: read a:0x50E65D (`[bin+0x0C]`); call a:0x50E695; c:94.
  - Site A, group loop, base [bin+0x38]: read a:0x50E125-0x50E131 (`[bin+0x58][i]`); call a:0x50E16B; c:234.
  - Site B, group loop, base inline bin+0x40: read a:0x50E453-0x50E459; call a:0x50E487; c:176.
- Thinning constraint: arg6 is used twice in the helper, as the draw count (count<<2) and as the normal/UV block offset (count*0x30). Lowering arg6 corrupts normals and UVs. Thin only the draw count by changing the `count<<2` at a:0x436A8B and a:0x436B7C. Keep the *0x30 offsets at a:0x4369DB, a:0x4369F7, a:0x436ACB, a:0x436AE7, a:0x436B33 on the full count. Prefix thinning (first K quads) is then valid. Needs a per-call global, since the helper does not see the bin (design UNVERIFIED).

## Q3 Wind update location
- Wind runs only inside RenderGrassPolys, at a:0x50DCF3. No separate pass. Caller header: D/51/00510f70.c:1.
- Wind gate operand a:0x50DCC8 vanilla = `D9 05 70 F0 A0 00` (FLD [0x00A0F070]). Vanilla value 225.0 = 15^2 (code_patches.toml:8, :20-22). Our mod rewrites the imm32 to &g_windDistSq (code_patches.toml:8-12), so the mod's wind radius is decoupled from fade start.
- Fade start/end are [0x00A0F070]/[0x00A0F074]. Alternates [0x0098BA68]/[0x0098AEBC] are chosen by [0x00A7D690] (a:0x50DB43-0x50DB67; c:45-59). The wind gate ignores that alternate.
- Fade value sets the 4th component of glMaterialfv params (c:65-75). Draw skipped when fade == 0 (c:60).
- Consequence: vanilla bins beyond 15 m keep frozen vertex state. Visual effect UNVERIFIED.

## OPEN
- Shader path draws from inline bin+0x40 but wind writes [bin+0x38]. Are they the same memory? UNVERIFIED.
- Sites A/B pass the group count G_i as the normal-block offset, not the bin count. Valid only if the group path is unused or N=1. UNVERIFIED.
- Who writes bin+0x04? Does DoBinBucket (0x004664E0) run once per frame? Wind has no dt, so its rate depends on the call count.
