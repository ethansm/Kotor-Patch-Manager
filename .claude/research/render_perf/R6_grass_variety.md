# R6: grass variety (2026-10-25, doc 50 side quest)

Static only (dumps steam_6a522e71/decomp, exe sha 6a522e71). Two recon agents: Sonnet static RE, Haiku web search. Addresses UNVERIFIED live.

## 1. How a blade is generated (FUN_0050EBC0, sole caller BuildGrassPolys 0x50D960 <- 0x4987E0)
- Bins built lazily when visible, destroyed out of view (DestroyGrassPolys 0x50DA00), rebuilt identically on re-entry.
- Count per face = round(0.5 * area * density) (0x50D32F-0x50D36A; density slot+0x20 at [EBP-0x224], no remainder carry).
- RNG = MSVC CRT rand. srand once per FACE at 0x50EF90: seed = round(v0.x + v1.y + v2.z) (sum 0x50EF53-0x50EF87 -> [EBP-0x9C]; verts in globals 0xA7D6D4..0xA7D6F4). Faces with equal rounded sum get identical blade sequences -> the repeating pattern.
- Position: 3 rands r%10000/10000 at 0x50F111/0x50F133/0x50F155 ([EBP-0xC/-8/-4]) normalised by their sum (0x44B210 call at 0x50F19B): centre-biased, not uniform over the triangle.
- Yaw: already uniform random (rand 0x50FD0D, [EBP-0x4EC], quaternion about Z via 0x5195A0).
- Shape: one flat upright quad per blade, side f = QuadSize (slot+0x24); not billboard, not crossed.
- Scale jitter: s = (r-0.5)*0.4+1 (0.8..1.2) at [EBP-0x104] (0x50FEF1), top two verts only.
- Atlas: 4 quadrants picked by cumulative Grass_Prob_LL/LR/UL/UR (slot+0x4C+4i; loop 0x50FDC3-0x50FE3E); u0 [EBP-0x100], v0 [EBP-0xF8]; per-vertex UV writes 0x51021C/0x5105E9/0x510A36/0x510E6D. No flip.
- Colour: one per blade, only on bin flag 0x10 path (face vertex colour, 0x5102A4-0x5102C9); flag 0x08 shader path uses lightmap UV instead. Which path is live: UNVERIFIED (the 0.1.5 path probe answers this).
- Texture: one Grass_TexName per area (area+0xF0, default "grass"), copied to every grass surfacemat slot (0x7A2FB0 / 0x50C430). Bins/buckets key by slot (bin+0x10), so per-slot textures are structurally possible.

## 2. Variety tweaks (ranked)
| # | Tweak | Hook site (orig bytes) | Effort | Risk |
|---|---|---|---|---|
| 1 | Per-face seed from float bit hash (kills repeats) | 0x50EF89 `8B 8D 64 FF FF FF`, rewrite [EBP-0x9C] | S | low; deterministic; needs ebp-relative pointer param + allowlist |
| 2 | Uniform barycentric (no centre bunching) | 0x50F177 `D9 45 F4 D8 45 F8`, rewrite [EBP-0xC/-8/-4] | S | low; preserve x87 |
| 3 | Stochastic count rounding / density noise | 0x50D35C `D9 85 DC FD FF FF`, [EBP-0x224] | S | low |
| 4 | Per-blade tint jitter (0x10 path only) | 0x50FB15, bytes [EBP-0x75/-0xB9/-0xD5] | S-M | no effect on 0x08 path |
| 5 | Wider scale jitter | 0x50FEF7 (7 B, absolute operand), [EBP-0x104] | S | changes taper too |
| 6 | Horizontal UV flip (4 -> 8 looks) | loop tail 0x50F0F3 or the 4 UV writes | M | stride depends on path |
Not worth it: extra yaw (already random), per-bin reseed (seed is per face; use #1).

## 3. Existing mods (web, Haiku)
No K2 mod ships several grass variants. Sources for variants: Restored Dantooine Textures Pack [K1] (N-DReW25, deadlystream 2043; Grass 1-5 sets, no reuse without permission), New Grass (MadDerp, Nexus K1 1207), K1 Dantooine Textures Restoration for TSL (N-DReW25, deadlystream 1046; ask before reuse), Grass and Foliage Reimplanted (Malkior, deadlystream 1486, Dxun WIP), Vanilla Planets HD (deadlystream 2976). K2 grass names seen: DXn_grassfx.tga, DXN_jungfx.tga. Local-install only; never in repo/release.
Asset route with no code: one 4-quadrant atlas mixing grass types + per-area Grass_Prob_* weights.
