# R4 grass state: RenderGrassPolys 0x0050DAC0 (static only)
DUMP = ~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71. `0xADDR` = asm in DUMP/decomp/<bb>/. Enum values read from exe .rdata/.data. UNVERIFIED = not proven.

## 0. Brief corrections
- 0x46661B is the OUTER bucket loop init ([EBP-4], bound [0xA7D69C], predicate FUN_0050C3E0 at 0x46663C). The inner BIN loop starts at 0x46664B ([EBP-0xC], bound [0xA73360] at 0x466660).
- Callee ECX (this = the bin) = [0xA7335C][j], loaded at 0x46668C. Pushed args: bucket (2nd), DAT_00A32A48+0xA8 (3rd, never read by callee), *(caller+0x90) (4th).
- 004664e0.c:38 drops the ECX arg, so it misleads: the bin is not DAT_00A32A48+0xA8.
- Callee early-outs: bucket match this+0x10 == param_2 (0x50DB31-0x50DB39); fade==0 exits (0x50DBBC FCOMP [0x986B38], double 0.0).

## 1. Q1: GL calls per bin (args constant unless marked)
Prologue, every matching bin with fade != 0 (0050dac0.c:69-88):
- 0x50DC19 glMaterialfv(GL_FRONT 0x404, GL_AMBIENT 0x1200, {0,0,0,1}): invariant.
- 0x50DC3E glMaterialfv(GL_FRONT, GL_DIFFUSE 0x1201, {0,0,0,fade}): alpha bin-dependent.
- 0x50DCA1 glMaterialfv(GL_FRONT, GL_EMISSION 0x1600, {bucket+0x40..0x48, fade}): rgb bucket-only, alpha bin-dependent.
- 0x50DD4E / 0x50DD63 glEnableClientState(0x8074 VERTEX / 0x8075 NORMAL): invariant; never disabled in this function.
- 0x50DD75 glEnableClientState(0x8078 TEXCOORD): invariant args, but B tails disable it (0x50E5B3, 0x50E1FF).
Path split: gate 0x50DD8A/0x50DD97 (byte[+2]&8 and DAT_00A0F07C). Fail = path A at 0x50E60D (no direct GL; DAT_00A1BCB4 ops 0x38 at 0x50E636, 0x4A at 0x50E6B2; draw 0x50E695).
Path B VP block, only if FUN_00484550()==0 (0x50DDAA-0x50DFB3; 0050dac0.c:102-135):
- 0x50DDC9 glEnable(0x8620 VERTEX_PROGRAM_ARB): invariant. Tail glDisable at 0x50E602: invariant.
- 0x50DE89 FUN_00506640(0,1) binds VP id DAT_00A7D6CC (compiled once, guard DAT_00A7D6D0 bit0 at 0x50DDCF): id invariant, callee has its own cache.
- 0x50DF1B glProgramEnvParameter4fARB(0x8620, 0x5E, bucket+0x28/2C/30, fade): alpha bin-dependent. Only if DAT_009F62DC!=0.
- 0x50DFAD same, index 0x5F, (bucket+0x34/38/3C, 1.0): bucket-only.
B1 (FUN_00484480()==0, JZ at 0x50DFBD to 0x50E238; 0050dac0.c:137-211):
- Pre-loop, invariant: 0x50E2CB glDisable(0x0B50 LIGHTING); 0x50E2E9 glColor4f(1,1,1,1); 0x50E324 glEnable(0x0DE1 TEX_2D); 0x50E36A glTexEnvi(0x2300,0x2200,0x1E01 REPLACE).
- Pre-loop, bin-dependent: 0x50E359 vcall +0xB8 on this+0x54.
- Sub-bin loop 0x50E382-0x50E4A7 (count this+0x2E): 0x50E3C7 glEnable(0x0DE1) and 0x50E421 glTexEnvi(MODULATE 0x2100) are invariant but re-issued each sub-bin. 0x50E410 texture bind is sub-bin-dependent. 0x50E487 draw FUN_004369a0 is dependent.
- Post-loop, invariant: 0x50E4E1 glTexEnvi(MODULATE); 0x50E51A glDisable(0x0DE1); 0x50E55C glColor4f(0,0,0,1); 0x50E571 glEnable(0x0B50); 0x50E5B3 glDisableClientState(0x8078).
B2 (FUN_00484480()!=0; 0050dac0.c:212-255): no direct glEnable/glTexEnvi/glColor. Direct GL: 0x50E000 glEnableClientState(0x8078), 0x50E1FF glDisableClientState(0x8078). Per sub-bin: DAT_00A1BCB4 op 0x39 or 0x1A (0x50E0AF / 0x50E11C) then draw 0x50E16B.
ARB pointers: [0xA32C38] glClientActiveTextureARB, 8 reads (0x50DFE4-0x50E5D7). [0xA32DA8] glActiveTextureARB, 6 reads (0x50E30F-0x50E53E). [0xA32DEC] glProgramEnvParameter4fARB, reads at 0x50DF1B and 0x50DFAD. Set in FUN_004317D0 at 0x431D9E / 0x431DAE / 0x43215E (004317d0.c:156,157,216). Guards: FUN_00484710 for a32c38/a32da8, DAT_009F62DC for a32dec. Unguarded a32da8 use exists in FUN_004B7F40 case 0x38.

## 2. Q2: [0xA1BCB4]
- Function pointer: per-op state dispatcher, (op, p2, p3, p4, p5).
- Writers, both in FUN_004317D0, branch at 0x432AC4 on (DAT_00A32DF8 & DAT_009F60BC)==DAT_009F60BC:
  - 0x432AD0 `MOV [0xA1BCB4],0x4B7F40` (ARB path) (004317d0.c:374).
  - 0x432BB1 `MOV [0xA1BCB4],0x42ABC0` (non-ARB path) (004317d0.c:390).
- FUN_004317D0 callers: 0x478C90, 0x477990.
- FUN_004B7F40 ops used here: 0x38 = a32da8(0x84C0)+glEnable(0x0DE1). 0x39 = unit0 and unit1 glEnable(0x0DE1) + vcall +0xB8 on arg2/arg3. 0x4A and 0x1A = break (004b7f40.c:716-738, 1335-1338). Callees: 0x484710, 0x4B50C0, 0x427330, 0x432CE0, 0x484CA0.
- FUN_0042ABC0 ops: 0x38 and 0x1A = break; 0x39 same as ARB; 0x4A = glEnable(0x0B50) + return (42abc0.c:1392-1393). Callees: 0x484710, 0x437280, 0x430C50, 0x4844C0, 0x432CE0.
- Selector DAT_00A7BAC4 (writers 0x478F13, 0x478F2C in FUN_00478F10; 0x478F43 in FUN_00478F40; meaning UNVERIFIED) picks 0x39 vs 0x1A (B2) and 0x38 vs 0x4A (path A).

## 3. Q3: hoist proposals (bytes from DUMP asm)
Caller side, insert at 0x46664B (after JZ 0x466649 `74 4b`), once per matching bucket:
- H1 move glMaterialfv(GL_FRONT, GL_AMBIENT, {0,0,0,1}) block 0x50DBFA-0x50DC1F (37 B): `8d45c4 50 b901000000 c1e102 8b91b8859800 52 33c0 c1e002 8b88ac859800 51 ff159c639800`, plus the vector stores 0x50DBD1-0x50DBE4 (FLDZ/FSTP x3, FLD1/FSTP) that build {0,0,0,1} at [EBP-0x3C]. Safe: glMaterialfv absent from 004b7f40, 0042abc0, 004369a0. Not at 0x46661B, because that would change AMBIENT even when no bin draws.
- H2 move glEnableClientState(VERTEX, NORMAL) 0x50DD4E / 0x50DD63 (`ff1514639800`). Safe only if no glDisableClientState(0x8074/0x8075) is reachable (none in the dumps).
- H3 move the 0x5F env block 0x50DF79-0x50DFAD. Only inside FUN_00484550()==0 and DAT_009F62DC!=0. Hazard: 0x4C3430 also writes env params (0x4c3430.c:552,563,892).
- Teardown: not needed for H1/H2/H3 (original code also leaves AMBIENT, VERTEX, NORMAL set).
Callee side, same behaviour:
- H4 in B1, hoist glEnable(0x0DE1) block `b904000000 c1e102 8b9128499f00 52 ff1568629800` (0x50E3B8-0x50E3CC, 21 B) and glTexEnvi(MODULATE) block `68 00 21 00 00 68 00 22 00 00 68 00 23 00 00 ff1544629800` (0x50E412-0x50E426, 21 B) above loop head 0x50E382, after one unit1 select. Keep per iteration: unit1 select, bind 0x50E410, unit0 select, draw 0x50E487. Needs UNVERIFIED: the bind vcall and FUN_004369a0 never touch TEX_2D or texenv.
- Not hoistable: any call with fade (0x50DC3E, 0x50DCA1, 0x50DF1B); bin/sub-bin binds (0x50E359, 0x50E410, DAT_00A1BCB4 args); TEXCOORD enable/disable pairs (0x50DD75, 0x50E281/0x50E5B3, 0x50E000/0x50E1FF), since path A relies on the top enable.

## 4. Open (UNVERIFIED)
- Which of path A / B1 / B2 runs live: depends on FUN_00484480, FUN_00484550 and DAT_00A32DF8 (runtime mask, no static value).
- Writers of the bin array pointer [0xA7335C] and count [0xA73360]: only zeroing at 0x495804 (FUN_00495760) found.
- FUN_00484CA0 guard inside FUN_004B7F40: not analysed.
