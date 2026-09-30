# Shield blackout investigation (2026-09-30, read-only; game was RUNNING the whole time, so no Ghidra)

The game was relaunched at about 02:09 while I worked, so the Proton log now holds only the new run. Evidence from the old run was
copied to `evidence/`. Live `patches/buff-duration-hud.dll` md5 is `d65312df` (the OLD FILL build) in the new run as well.

## Verified facts
- Renderer: native OpenGL through Wine opengl32/winex11 on an NVIDIA RTX 5070. Not DXVK, not wined3d.
  - `opengl32.dll` in the game dir is **ReShade 6.8.0** (see ReShade.log).
  - Active techniques: Deband (`enable_depthbuffer=1`), Curves, qUINT_sharp, SMAA, Tonemap, Vibrance, **MXAO** (depth-based).
  - Keys: End = effects on/off, Home = overlay, PrintScreen = post-effect screenshot.
  - Pixel format 65: 24-bit depth, 8-bit stencil, 4x MSAA. ini: Frame Buffer=1, Soft Shadows=1, Shadows=1, AA=4, V-Sync=1. Monitor runs at 74.48 Hz.
  - ReShade.ini was modified at 01:31, mid-session 4.
- GUI clip primitives, from static objdump of swkotor2.exe (IAT resolved):
  - `0x4792c0` Begin2D: saves GL_VIEWPORT to entry[0] and disables LIGHTING and DEPTH_TEST.
  - `0x4793f0` End2D: pops PROJECTION and re-enables LIGHTING and DEPTH_TEST. It does NOT restore the viewport.
  - `0x479460` Push:
    - No depth overflow check.
    - Refuses when `[0xa32a30]!=0` or the rect is off-screen.
    - Calls `glViewport`.
    - With a real colour it draws an immediate-mode quad with TEXTURE_2D and LIGHTING disabled, then re-enables both unconditionally.
    - It does not flush the GUI batch first.
    - `flag!=0` means glClear + DEPTH_TEST on.
  - `0x4799c0` Pop: restores the parent viewport, and DEPTH_TEST follows the parent entry's flag.
  - `0x47ea60` Flush: disables LIGHTING, draws, then enables LIGHTING, so toggling LIGHTING during the GUI pass is normal.
  - Engine panels use colour fills themselves (`0x40ed9a`, `0x74c215`, `0x411658`, `0x8859ed`).
  - Conclusion: our push/pop/fill usage is balanced and does not alter any state that survives End2D. The only per-frame GL
    state our draws leave is `glViewport`, which is restored by the pop.
- The GUI never touches FBOs, scissor or clear. Only `0x4113a0` (letterbox) uses glScissor + glClear, and it balances them.
- The scene/framebuffer-effect cluster uses glCopyTexSubImage2D/glViewport/glClear in `0x436b90..0x448910`:
  - `0x46c5c0` -> `0x4373a0` (full-screen FB post effect), gated by `[0xa732bc]`, `[0x9f4e74]`, `[0xa1dd29]`, `[0x9f48fb]`.
  - `0x46cf20` -> `0x440cb0`/`0x441610` (render-target copies), gated by `[0x9f5da8]`.
- Main loop rates:
  - Normal: 54-58 fps.
  - Menus: about 340/s (HUD tick stops).
  - Focus lost: about 900/s with the HUD ticking. The hotbar log at 2622625328 shows `foreground window ... ours=0`.
  - The 4g "fast" window is therefore a focus loss, not proof of a render skip.
- playerId FFFFFFFD is the PC (Meetra). It is not an error.
- In 4g the shields were applied at t=109.5 (Meetra) and 118.2 (Atton). The leader was switched rapidly between 120 and 124. Focus was lost at 125.3.

## Hypotheses (ranked)
1. **Engine framebuffer-effect / texture-shell render path under Wine+NVIDIA+MSAA+ReShade (speculative, strongest fit).**
   - For: the scene is black while the HUD draws. That matches a full-screen scene pass (copy to texture, then redraw) producing black, with the GUI drawn afterwards.
   - For: it tracks the shield VFX lifetime, and it is not specific to one texture.
   - For: the variability (area, character, number of shells) fits a state-dependent path.
   - Against: not yet shown that the shell sets the FB flags.
2. **ReShade depth effects (MXAO/Deband) acting on a changed depth buffer.**
   - For: ReShade is present, depth-based, and its ini was edited mid-session.
   - Against: it should darken HUD pixels too (the GUI does not write depth).
   - It is still cheap to exclude: press End.
3. **Max Lights 3->7.**
   - Light block moved to env[96..140]; per-program text overflow is estimated at 480 B per light.
   - A shield shell pass that uses its own ARB vertex program could fail to compile. Every draw with that program would then be GL_INVALID_OPERATION.
   - If failures cascade to the bound program state, the whole scene would go black.
   - It is present in every blackout session.
4. **Our DLL (not ruled out, now less likely).**
   - GL side: verified balanced (see above).
   - Remaining risk: `swapHealthBar` writes our pooled image into engine-owned border fill `+0x74`. The border dtor or fill setter frees what it holds (hotbar INVARIANT, ForcePowerHotbar.cpp:453), which could leave dangling pointers. That would crash, not black out.
   - Also: probe-1 fx_tex GUI draws (history).
5. **Siblings.**
   - perf-lab: IAT HeapAlloc hooks, added only at 01:15/01:57, so later than the first blackouts. Unlikely.
   - equip3d: added 01:00, later than the first blackouts. Unlikely.
   - inv3d: menu-only. Unlikely.
   - hotbar: HUD labels only. Unlikely.

## Experiments (live toggles during a blackout, no restart)
1. Press **End** (ReShade effects off). If the scene returns, the cause is ReShade. Also take a PrintScreen (ReShade) and an F9 shot (DLL, pre-present).
2. Options > Graphics > Advanced during the blackout: **Frame Buffer Effects off**, then Soft Shadows, then Shadows. Test one at a time and note which restores the scene.
3. Our DLL kill switch (new build): `buffhud_off.txt`, polled every 60 frames, skips all draws and swaps. Create it during the blackout. If it restores the scene, the cause is ours.
4. Restart variants, if 1-3 fail:
   - (a) move opengl32.dll aside (ReShade off entirely);
   - (b) stock 16 without max-lights;
   - (c) stock 16 without buff-duration-hud;
   - (d) stock 16.
   Repeat 2 human shields + T3 in the same area for each.

## Logging plan (buff-duration-hud, flag `shieldprobe.txt`)
- GL entry points:
  - `hGL = GetModuleHandleA("opengl32.dll")` (the ReShade proxy re-exports everything).
  - glGetError, glGetIntegerv, glIsEnabled, glReadPixels, glGetString via GetProcAddress.
  - GL_PROGRAM_ERROR_POSITION_ARB 0x864B and GL_PROGRAM_ERROR_STRING_ARB 0x8874 via wglGetProcAddress when needed.
- Blackout detector, once per 30 HUD frames in the leader call of ShieldProbeSlot (the 3D scene is already rendered, GUI half drawn):
  - glReadPixels a 5x5 grid over the scene area (avoid the HUD: x 20..70%, y 15..60%) from GL_BACK.
  - `dark = max channel < 8 for all 25`. Log `SCENE dark=1 meanRGB=.. fg=ours|other menu=..` on each change.
  - This is the pre-ReShade image. If the user sees black while dark=0, the cause is ReShade.
- GL state on each dark/light change and on F9:
  - GL_VIEWPORT, GL_SCISSOR_BOX, IsEnabled(SCISSOR, STENCIL, DEPTH, LIGHTING, BLEND, FOG, TEXTURE_2D, VERTEX_PROGRAM_ARB 0x8620, FRAGMENT_PROGRAM_ARB 0x8804).
  - GL_COLOR_WRITEMASK, GL_DEPTH_WRITEMASK, GL_CURRENT_COLOR, GL_DRAW_BUFFER, GL_READ_BUFFER, GL_FRAMEBUFFER_BINDING 0x8CA6, GL_STENCIL_FUNC/REF/VALUE_MASK.
  - Drain glGetError (up to 8).
- Engine state on the same trigger:
  - Clip depth `[0xa309c0]` (short) and entry[0] `[0xa309c8..ce]`, `[0xa32a30]`.
  - FB flags `[0xa732bc]`, `[0x9f5da8]`, `[0x9f4e74]`, byte `[0xa1dd29]`, byte `[0x9f48fb]`.
  - Screen w/h `[0x9f42a4]`/`[0x9f42a8]`, max lights `[0x00A0F03C]`.
- Balance: log a line whenever the clip depth before != after our draws (or is not 0/expected).
- Heartbeat every 2 s: frames, `GetForegroundWindow()==ours`, number of pools per slot, and the dark flag.
- F9 (`GetAsyncKeyState` edge, only when the window is ours):
  - `MARK n` + full dump above.
  - glReadPixels of the full back buffer, written to `blackout_NNN.bmp` (3840x1600x3, about 18 MB).
  - Ask the user to press PrintScreen right after, so the ReShade shot can be compared.
- Correlation: every log line already carries `t=GetTickCount`. The ReShade screenshot name has the local time, and SYSTEMTIME is written in MARK.
- APPLY/REMOVE: keep them, and add `vfx=` and the leader id on type 0x6b/0x1e events.

## Launch options for the next run
`PROTON_LOG=1 WINEDEBUG=+timestamp,+pid,+tid,+seh,+loaddll,+debugstr,warn+wgl,warn+opengl,fixme-dinput,fixme-nls %command%`
- DXVK_* and WINE_D3D_CONFIG are useless here, because this is not D3D.
- Do not use `+opengl` trace: it logs every GL call.
- For a full GL capture: apitrace (32-bit Windows `opengl32.dll` wrapper). ReShade's `ProxyLibrary` could chain it; otherwise swap it for ReShade for one run. Trim the trace to the blackout frames.

## Ghidra TODO (game closed)
- 0x878f10 tail, i.e. what the shell sets on the client creature.
- Who sets `[0xa732bc]`/`[0x9f5da8]`/`[0x9f4e74]`.
- 0x46c5c0 (scene render) branches for shell/stealth.
- Max Lights `SetProgramLightCount` sizing for shell programs.
