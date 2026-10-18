# U4 Question D: how the preview camera becomes the GL view (static recon)

Sources: Steam static dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/` (paths below are relative to `decomp/`), exe `swkotor2_vanilla_6a522e71.exe` (constants and vtables read from .rdata/.data), REA ledger rows `patch_manager_mods/ghidra_knowledge/rea_findings/20261016_holster-c4-weapon-attach.tsv`. No REA used.

## Verdict
- Engine basis (confirmed): camera pose = attached node's world pose. View = inverse(T(pos) R(q)). Eye forward = R(q)(0,0,-1), i.e. local -Z (glu convention). Near 0.1, far 100. fovy is vertical degrees.
- `holster_editor_src/HseView.h:172-186` (`worldToScreen`) already implements exactly this basis. The basis is not the bug.
- For hook q = Rz(+90), that basis makes the camera look straight down world -Z. The hand mounts then project at NDC x about +/-10 (portrait ext), which matches the "thousands of px outside" symptom.
- Hard limit: at depth 0.74 m, hands at +/-0.44 m lateral need tan(half-fov) x aspect >= 0.6. At fovy 10.33 that means aspect >= 6.6. So with the logged hook as the camera, the hands cannot be in frame. The miss is in the render-time camera pose, fov, aspect or marker frame, not the axis convention.

## 1. Scene camera world transform from the attached node
- Camera object C = view+0x78, vtable 0x98C45C (`Camera::vftable`, ctor 0047ecd0). Its embedded child Gob is at C+4. Pose: position C+0xA8 (x,y,z), quaternion C+0xB4 (w,x,y,z). Evidence: 0047f320.c reads param_1+0xa8 / +0xb4; 0047ecd0.c sets vtable at 0x47f0a0/0x47ecd0.
- Attach: camera vt+0x74 = 0x4819E0 (wrapper, `004819e0` asm: child = this+4, CALL 0x461670 at 0x4819F9). 0x461670 (type 1 case) copies node vt+0xC into child+0xA4/+0xB0 once (ledger 0x4616f1), i.e. C+0xA8 and C+0xB4. Node vt+0xC = Part world pose 0x457510 (parent chain plus owner Gob pose).
- Type 1 builds CAurBehaviorAttachTranslate (ctor 0x504F60, vtbl 0x991914). Behaviour pointer stored at child+0x1B4 (= C+0x1B8) via 0x461600.
- Per frame: CAurBehaviorAttach refresh 0x504D40 re-copies node pose into the child (CALL at 0x504EF3). Translate 0x505020 (`50/00505020.c:25-59`) then rewrites child position from behaviour offset (+0x3C,+0x40,+0x44), applied only when flag +0x48 == 0. Its composition uses child vt+0x64 and vt+0x68 (not fully decompiled; zero offset gives identity).
- Ctor zeroes the offset (0x504F60). Preview3dProvider.cpp:505 is the only camera attach and passes no offset.
- Result: default camera position = hook world pose. No boom distance by default. Camera orientation = hook world quaternion.

## 2. Which axis is forward
- Camera vt+0x08 = 0x47F320 (render/apply). Perspective: `gluPerspective(fovy=+0x204, aspect=vp w/h, near=+0x210, far=+0x214)` at `47/0047f320.c:188-189`.
- Frustum plane corners use z = `_DAT_00987474` = -1.0 (`0047f320.c:204-205`). Forward = eye -Z.
- Modelview: `FUN_00435f00(pos=param+0xa8, quat=param+0xb4)` at `0047f320.c:279-280`. Branch at 283 is used only when DAT_00a32a54 != 0; that global is initialised to identity by 0x983860, so default is off.
- `43/00435f00.c`: glLoadIdentity (line 22). DAT_009f497c = 1 (static), so the quaternion branch runs: line 31 negates w (`_DAT_00986d98` = -1.0), line 39 `glMultMatrixf(FUN_00518390)`, line 40 `glTranslatef(-pos)`. The Euler branch (line 27) gives the same inverse rotation.
- `51/00518390.c:45-55`: column-major standard quaternion-to-matrix (param_1[1] = 2(xy+wz), param_1[4] = 2(xy-wz)).
- Therefore view = R(q)^T * T(-pos) = inverse(T(pos) R(q)). Eye +X = R(q)(1,0,0). Eye +Y = R(q)(0,1,0). Eye -Z (forward) = R(q)(0,0,-1). Forward is local -Z, not +X or +Y.

## 3. Near, far, fov
- Ctor 0x47ECD0 (`47/0047ecd0.c:35-38`): fov +0x204 = 45.0 (`_DAT_0098C274`), near +0x210 = 0.1 (`_DAT_009876FC`), far +0x214 = 100.0 (`_DAT_0098C270`), +0x20C = 1.0.
- Near/far setter: vt+0x40 = 0x47F160. Fov setter: vt+0x44 = 0x47F190 (stores +0x204 raw, no clamp).
- Preview3dProvider.cpp:381 sets fov via fovForZoom(22.726, zoom). Never sets near/far, so 0.1 / 100 hold.
- Check: 22.726 / 2.2 = 10.33 and 22.726 / 1.05 = 21.6, matching the log. fovy is vertical.

## Reconciliation with the observed data
- hook q(w,x,y,z) = (0.7071,0,0,0.7071) = +90 deg about Z. R(q)(0,0,-1) = (0,0,-1). Camera at (0.024,0.001,1.593) looks straight down.
- Screen right = world +Y, screen up = world -X. Arms spread along Y therefore run horizontally across the screen, as expected.
- Projection with HseView model (aspect 576/896 = 0.643, tan(fovy/2) = 0.0904):
  - right hand (0.03,0.44,0.85): eye (0.439,-0.006,-0.743), NDC (10.2,-0.09)
  - left hand (0.03,-0.45,0.87): eye (-0.451,-0.006,-0.723), NDC (-10.7,-0.09)
  - centre (0,0,0.9): eye (-0.001,0.024,-0.693), NDC (-0.02,0.38), in frame
- Script: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/8836d620-712d-477e-8a54-f7e5bfdc9d45/scratchpad/proj_check.py` (pure math, no game files).

## Candidate camera model
- Position P = C+0xA8 (default = hook world position, zero offset).
- Forward f = R(q)(0,0,-1), right r = R(q)(1,0,0), up u = R(q)(0,1,0), with q = C+0xB4 (w,x,y,z).
- fovy = C+0x204 (vertical deg). Aspect = GL viewport w/h (used when C+0x220/0x224 are zero), not necessarily ext w/h.
- near 0.1, far 100.
- Confidence: HIGH (about 85%) for the engine basis. Matches HseView.h:172-186 exactly.
- Confidence: UNSURE for the render-time camera position. The log read the hook node, not C+0xA8 or the Translate offset at C+0x1B8+0x3C.

## Calibration that would close it (one live run, $0 tokens)
1. Log C+0xA8, C+0xB4 (camera, not hook), C+0x204, C+0x220/0x224, and the behaviour offset: *(int*)(C+0x1B8), then floats at +0x3C, +0x40, +0x44 and flag at +0x48.
2. After FUN_0047F320 returns, glGetFloatv(0x0BA6 GL_MODELVIEW) and glGetFloatv(0x0BA7 GL_PROJECTION), plus glGetIntegerv(0x0BA2 GL_VIEWPORT). Run worldToScreen on a known rendered point and compare to where it draws.
3. Confirm UI-units vs GL-pixels for ext: if the UI is virtual and stretched, aspect differs from ext.w/ext.h.

## Notes
- Preview calls cameraAttach on the rig (`rig`, Preview3dProvider.cpp:505), not on the creature. The hook follows panZ, so the creature is a parent of the rig node chain, or both move together. Not verified here.
- Static facts only, no REA, no live game logs read.
