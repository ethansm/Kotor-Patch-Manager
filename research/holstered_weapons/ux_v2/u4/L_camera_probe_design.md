# U4 Q-L: log-only camera probe spec (design, no code)
Inputs: D, E, H read. J and K reports are not in u4/ (only A-H exist). Assumed: J chain = view+0x78; K contract = H's MapToScreen signature.
## 1. Engine facts (static, verified 10-18)
- Camera::vftable 0x98C45C (= GameAddr SceneCameraVtable). Slots: +0x08 0x47F320 (render), +0x28 0x4809D0 (MapToScreen), +0x34 0x47F2B0 (SetViewPort), +0x44 0x47F190 (fov), +0x74 0x4819E0 (attach).
- MapToScreen Steam 0x4809D0: size 1042, RET 20, thiscall ECX=cam, args (float* out, x, y, z, char normalized). GOG symbol MapToScreen = 0x008C1B40 (same size).
- Fields: vp ints +0x218 x, +0x21C y, +0x220 w, +0x224 h (GL px); fovy +0x204 (vertical deg); near +0x210; far +0x214; ortho +0x20C; pos +0xA8..+0xB0; quat +0xB4 w, +0xB8 x, +0xBC y, +0xC0 z.
- Math (004809d0.c, 00514450.c): eye = conj(q)(P - pos). X = vx + W/2 + (H/2)/t * (-ex/ez); Yg = vy + H/2 + (H/2)/t * (-ey/ez); t = tan(fovy*pi/360). No aspect term. Normalized: nx = X/W, ny = 1 - Yg/H.
- No behind-camera clip: ez >= 0 gives a mirrored value. fovy <= 0 returns (0,0,0). W==0 or H==0 makes the engine call glGetIntegerv (state-dependent).
- Fast-path quirk: DAT_00A0F190 (file value 1, no direct store in dump) skips rotation when w == 0 or w == 1 exactly. Only w == 0 is wrong. Same gate in render frustum (047f320.c:288).
- Render 0x47F320: glViewport(vp); gluPerspective(fovy, aspect = vp W/H). Modelview from +0xA8/+0xB4 via 0x435F00 (no flag gate). So MapToScreen's pose equals the render pose.
- Attach offset: behaviour ptr cam+0x1B8; floats +0x3C, +0x40, +0x44; flag +0x48 (per D).
- Correction to H: its "set camera viewport first" mutates the camera. This design skips instead (log-only).
## 2. Pointer chain
- view = e.viewCtl (ViewControl(token)). cam = rd(view, 0x78) [K2 View_Camera]. vt = rd(cam, 0). vt_ok = (vt == SceneCameraVtable).
- Scene cross-check: camera at scene+0x18 with scene inline at view+0x60, so scene_cam must equal cam.
- On vt mismatch: walk view+0x74, +0x78, +0x7C, read each vt, log each (CAMPTR WARN).
## 3. Gating and rate limit
- New ini key probe_cam (0/1; code default 1). Global g_probeCam beside g_probeCalib (:103). Parse in ini loop (:322). CONFIG (:367) adds probe_cam=%d. Not probe_calib: CALIB needs clicks.
- Editor fields after :234: camEngCaps, camEngKey {yaw, zoom, panZ, cam ptr}, camEngCfgLogged, camEngDumped.
- kMaxCamEngCaptures = 10 per Editor. New WarnKind W_CAMENG before W_KINDS (:141); kMaxWarnLogs = 20.
- Capture only when the key changed (first 'live' capture always). Separate counter from projLogs, so PROJ is not starved.
- CAMRAW once per Editor. saferead::beginScope() at entry. Main thread only.
## 4. Placement
- Add readCamEng() and logCamEng(Editor&, const char* why) after logEditProj (ends :704), before callItemName.
- Call sites: logCamEng(e, "live") after :2051; logCamEng(e, "settled") after :2081. Not inside logEditProj.
- Pure helpers in HseView.h next to worldToScreen (:161): hse::eyeOf(camPos, camQ, P, eye[3]) and hse::rawPredict(eye, vp, fovy, out[2]). Unit tests in hse_view_test.cpp.
- rd() uses site 0xE70100u (:277). Probe uses a new block 0xE70400u+.
## 5. Log lines (capture = CAMENG + CAMATT + 3 CAMPROJ + CAMCHK; CAMPTR/CAMRAW once)
- CAMPTR why=%s view=%08x cam=%08x vt=%08x vt_ok=%d s08=%08x s28=%08x s44=%08x s74=%08x scene_cam=%08x
- CAMRAW base=%03x d=%08x x8 (bases 000, 0A0, 1B0, 200, 220; raw dwords)
- CAMENG why=%s pos=(%.4f,%.4f,%.4f) q=(%.5f,%.5f,%.5f,%.5f) qlen=%.5f fovy=%.3f near=%.3f far=%.3f vp=(%d,%d,%d,%d) ortho=%.4f hook_dpos=(%.4f,%.4f,%.4f) hook_dot=%.6f ctl=(%d,%d,%d,%d) flag=%d rot_fast=%d yaw=%.2f zoom=%.3f panZ=%.3f
- CAMATT why=%s beh=%08x off=(%.4f,%.4f,%.4f) flag48=%d
- CAMPROJ why=%s hand=%c p=(%.3f,%.3f,%.3f) eye=(%.4f,%.4f,%.4f) valid=%d eng_raw=(%.2f,%.2f) eng_n=(%.5f,%.5f) pred_raw=(%.2f,%.2f) ours=(%.1f,%.1f) inside=%d
- CAMCHK why=%s n=%d max_eng_pred_px=%.2f max_eng_ours_frac=%.5f
- hand = R, L, or M (mount, weapon edit only). hook_dot = |q_cam . q_hook| (1.0 = same rotation).
- valid = (eye.z < 0). valid=0 is a state, not a failure; those points are left out of CAMCHK.
- Ours: worldToScreen with ext = vp rect, so ours is top-down from vy. Fractions: fx = (X-vx)/W; fy_eng = 1-(Yg-vy)/H; fy_ours = (sy-vy)/H.
## 6. Engine call (K contract = H signature)
- Gate: vt_ok; slot 0x28 == gameaddr SceneCamMapToScreen; fovy in (0,179); W>0, H>0; pose finite; qlen > 1e-6.
- float out[3] = {0}; reinterpret_cast<float*(__thiscall*)(void*, float*, float, float, float, char)>(fn)(cam, out, x, y, z, norm), once with norm=0 and once with norm=1. Expect out[2] == 0. Pure math, no GL call.
## 7. Failure paths (each logs KP_LOGV(chView, KPLOG_WARN, "CAMENG WARN ...") via warnOk(e, W_CAMENG))
1. viewCtl == 0: "view control null".
2. Provider or token missing: reuse readCam c.why.
3. vt != SceneCameraVtable: "camera vtable %08x != %08x" plus candidate walk.
4. Offset or address 0 (View_Camera, Cam_*, SceneCamVt_MapToScreen, SceneCamMapToScreen, GOG table): "offsets unknown on this build"; skip.
5. Slot 0x28 unreadable or != expected: "MapToScreen slot %08x != %08x; not called".
6. W or H <= 0: "viewport unset; engine would call glGetIntegerv; not called".
7. fovy out of range or pose non-finite: reason; not called.
8. Any rd() failure: "read failed off=%03x".
9. GetNodeWorld(rhand/lhand) or GetMountWorld false: "hand=%c unavailable".
10. rot_fast == 1 and w != 1: "fast path active w=%.9g flag=%d".
11. eng returns zero with valid=1: "MapToScreen returned zero".
12. beh == 0: "attach behaviour null (offset unknown)".
- INFO only, not a failure: cap reached, logged once as "CAMENG cap reached".
## 8. Registration (no code written)
- query_ghidra address 0x4809d0, 0xa0f190, 0x98c45c: no rows. Symbol MapToScreen is GOG-only. Steam evidence: camera vtable slot +0x28 and static_facts (size 1042, RET 20). A Steam knowledge-DB row is still needed; add later via kotor2-address-db. Not a probe prerequisite.
- _shared_src/GameAddr.h near :56: SceneCamMapToScreen = 0x004809D0; SceneCamRotFlag = 0x00A0F190. The raw-address checker excludes GameAddr.h, so no tests/static/raw_addresses.toml entry. Grep the names first.
- _shared_src/K2Addr.h: add uintptr_t SceneCamMapToScreen; int SceneCamVt_MapToScreen, Cam_Pos, Cam_Quat, Cam_Fovy, Cam_Near, Cam_Far, Cam_Vp, Cam_Ortho, Cam_Behaviour. Steam factory (:345-351) sets 0x28, 0xA8, 0xB4, 0x204, 0x210, 0x214, 0x218, 0x20C, 0x1B8. GOG stays 0.
- Then run python3 scripts/compat_check.py on touched files. Ship via kotor2-kpm-install. Queue the live check as a PLAY_TEST_QUEUE pending row: "CAM1: open holster editor once; read CAMENG lines".
## 9. Optional mode 2 (needs user OK)
- probe_cam=2 sweeps yaw 180, 90, 270, 0 for 600 ms each via g_p3.p->SetView, one capture per step, then restores e.view. Changes the on-screen view for about 3 s.
## 10. What the lines decide
- hook_dpos ~ 0 and hands inside=0: render pose is the hook, and the hook placement is wrong.
- hook_dpos != 0 and inside=1: a camera offset exists; read CAMATT.
- CAMCHK max_eng_pred_px < 1: eye basis and quaternion order are right.
- eng_raw != pred_raw with valid=1: quaternion order, sign, or pose source is wrong.
- Not logged: render early-return when vp is outside screen bounds (047f320.c:121-124).
