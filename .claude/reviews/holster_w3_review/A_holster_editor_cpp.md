# Review A: holster_editor_src/HolsterEditor.cpp (HEAD, 1043 lines; commits 96f96d6, 03fc2ae, 776b3b1, 7c94d35)

Decision: ACCEPT WITH FIXES. 0 BLOCKING, 1 MAJOR, 12 MINOR. Static review only (no build, no install, no live log read).

## Verified OK (checked against code, no finding)
- KPatchInit (l.1008-1040) does no file I/O; holster_editor.ini is read in cbBuild via readConfig. The g_fallbackSet static initializer is the repo-wide pattern (already in 31e6e2d).
- No raw 0x00[4-B]xxxxx literal anywhere (incl. comments). 0xE70100u and friends are saferead site ids; 0x84/0x08 in readWeaponModel match holster's gobModelName.
- Item count is 22 (view, status + 20) <= 24; every Add* result is guarded with `>= 0` before use; viewItem only added if viewCtl.
- SetRect on every enter: cbEnter calls placeItem + applyLayout each time.
- Gob lifetime: every Rebuild/SetWeapons/Destroy fires cbBeforeFree, which calls EndEdit then UnregisterPreview for both hands and clears regGob; cbDestroy removes the registration after Destroy. Pick, Leave and Destroy paths call exitEdit/EndEdit before Destroy.
- BeginEdit/EndEdit pairing: Back/Esc (cbLeave), cbDestroy, member switch, hand switch (EndEdit then BeginEdit), Done, and BeforeFree all end the edit.
- Log spam: the per-frame paths (SetView, nudges) log nothing; CAM is change-gated (but see F5).
- Buffers: all snprintf targets are bounded; readWeaponModel caps at 16 chars; the longest EDIT status line (~83 chars typical) is under the 95-char mod-config SetText cap.
- NudgeWorld axes: preview-3d rotates the creature (CrSetFacing), the camera is fixed, so camera-relative dp in world space is right at any yaw.
- Plan deviation, harmless: no WAIT state (plan 2.6). Holster fills parent/node itself in previewPlace at hook time, so same-tick RegisterPreview is fine. Only the CAMHOOK probe is affected (F10).

## Findings

### F1 MAJOR - HolsterEditor.cpp:697-698 (scope cycle), 482 (enterEdit hand switch), 561/577/604 (exitEdit via Done/Back/pick)
SetScope is documented "implicit Revert; the caller confirms Discard first" (HolsterApi.h, Holster.cpp apiSetScope -> edReseed). The editor cycles the scope with no confirm and no dirty check, and EndEdit (hand switch, Done, Back, member pick) also drops the overlay with no warning.
Failure: user spends a minute nudging a sabre (status shows `*` = dirty), clicks "Scope: Weapon" to see Category, or clicks the other-hand button, or Back; all unsaved work is gone with one click and no prompt.
Fix: before SetScope/EndEdit-driven transitions, if `editInfo.dirty` (GetEdit) either ignore the click and show "Unsaved: Save, Revert or click again" (second click within ~2 s discards), or auto-Save. The plan puts the Discard confirm in D3d, but since D3c shipped Save/Scope this is now user-visible data loss.

### F2 MINOR - HolsterEditor.cpp:482 (BeginEdit), 698 (SetScope), 709 (Save), 687 (CycleBone)
Return codes are ignored. apiBeginEdit returns 0 when the Gob is not in holster's registry or loadTune fails; the editor still sets mode=MODE_WEAPON and shows the edit layout. NudgeWorld/Save then silently no-op (g_edit.active false), the status line shows zeros. Save rc is only logged, so a failed write gives no user feedback.
Failure: holster.ini unreadable -> Edit button opens an edit page that does nothing.
Fix: in enterEdit `if (!BeginEdit(...)) { log; set status "edit unavailable"; return; }` before changing mode/layout; on Save rc==0 set the status text to "Save failed"; use GetEdit's return in updateStatusText (0 = edit closed, drop to NAV).

### F3 MINOR - HolsterEditor.cpp:220-236 (cbBeforeFree)
The callback ignores `token`. The BeforeFree registration is global (SetBeforeFree has no token argument), so a Destroy/Rebuild/SetWeapons of any other viewer (preview-3d supports several tokens, evicts the oldest on CreateView when full) runs it with the other token: it EndEdits, sets mode=MODE_NAV and nulls regGob while the editor's own Gobs are alive and still registered with holster. The UI is then stuck on the edit layout with edit buttons dead (mode NAV, controls parked/shown wrongly) and `regGob` null, so Edit never works until the next rebuild.
Fix: `if (token != e->token) return;` at the top.

### F4 MINOR - HolsterEditor.cpp:818-819 (Ctrl snap in LMB nudge)
Snap is applied to the per-frame delta: `dp = round(dp/0.01)*0.01` with scale 0.0025/zoom (about 0.001 m per px at zoom 2.2) rounds to 0 for any drag under about 4 px/frame, and lastMouseX/Y are advanced anyway (l.822-823), so the movement is discarded, not accumulated. Holding Ctrl makes slow drags do nothing and fast ones jump.
Fix: accumulate raw dx/dy into a member, apply and subtract in 0.01 m steps (as the RMB path effectively does by not advancing lastMouse when deg is 0).

### F5 MINOR - HolsterEditor.cpp:955-963 (CAM log)
`s_camLogs` is a function static capped at 40 for the process lifetime and fires every frame the yaw changes. One spin drag uses the whole budget; afterwards no CAM line is ever written, including the final yaw/zoom/panZ that play row D3B step 8 tells the user to read from the log.
Fix: log on drag end / button / wheel (those already log BTN/DRAG end/WHEEL), or log CAM only when the view has been stable for ~0.5 s, and reset the counter in cbBuild.

### F6 MINOR - HolsterEditor.cpp:547-563 (cbLeave), 906-917 (NEEDBUILD)
stanceIdx, the stance label, the Play/Pause text and emoteIdx survive Back + re-enter, but the rebuilt creature plays the default idle (PlayAnim is never re-sent). Pick resets them (l.587-593) but leave does not.
Failure: choose "Run", press Back, re-enter the page: label says "Stance: Run", button says "Pause", character stands idle; Next goes Run -> Walk (Rifle).
Fix: in cbLeave (or on NEEDBUILD) reset stanceIdx=0 and set both label texts, as the pick path does; factor the three copies of the reset into one helper.

### F7 MINOR - HolsterEditor.cpp:187-216 (readConfig)
Globals are never reset to defaults. If the user removes a key or deletes holster_editor.ini between Options opens (the stated tuning workflow, plan 2.6), the previous value stays (e.g. `enabled=0`, `free_view=1`, old view_yaw), and a missing file returns before g_cfgView is recomputed.
Fix: set g_enabled=true, g_debug=false, g_freeView=false, g_cfgYaw=180, g_cfgZoom=2.2, g_cfgPanZ=0 at the top of readConfig.

### F8 MINOR - HolsterEditor.cpp:911 (SetWeapons(token,1) after Rebuild)
Redundant: SetWeapons(1) was recorded in cbBuild (l.~290) and buildPreview already calls refreshWeapons when weaponsOn (Preview3dProvider.cpp:442). The second call fires BeforeFree and detaches and re-attaches both weapon models on every enter and every member pick (extra model load, an extra BeforeFree burst).
Fix: delete the call (plan 2.1 says Rebuild re-attaches). If kept for safety, move it before RegisterPreview only when WeaponGob is null for both hands.

### F9 MINOR - HolsterEditor.cpp:578-580, 905-909 (IDLE dead end)
After `REBUILD fail` the state goes IDLE, and the picker, stance and edit buttons are only polled in LIVE, so the user cannot try another member; only Back + re-enter recovers. Plan 5 says the page must stay usable.
Fix: poll prev/next in IDLE too (or set state back to NEEDBUILD on the next pick click).

### F10 MINOR - HolsterEditor.cpp:942-947 (CAMHOOK)
The plan logs CAMHOOK in WAIT (one tick after Rebuild); the code calls GetNodeWorld(RigGob, "camerahook") in the same tick as Rebuild, before the rig has been drawn/updated. The D3B pass signal `CAMHOOK ... ok=1` may log ok=0 or a zero pos for a healthy build.
Fix: log it on the first LIVE tick after the rebuild (flag `camHookPending`), not in NEEDBUILD.

### F11 MINOR - HolsterEditor.cpp:786, 806, 835 (edit-mode drag start)
Unlike NAV (which has the `pressed` guard), the WEAPON-mode MMB/LMB/RMB drags start whenever the button is down and the cursor is inside the view, even if the press began outside it (on a strip button, or the Done click that is still held).
Failure: press LMB on Save, drag into the view before releasing -> sudden nudge of the weapon.
Fix: track "press seen outside" (reuse `pressed`) and only begin when the button went down while insideView.

### F12 MINOR - HolsterEditor.cpp:793
MMB spin subtracts dx (`yaw - dx*0.5`), while the NAV drag (p3d::yawDrag via dragMove) adds it. Dragging right spins the character opposite ways in the two modes.
Fix: use `+` (or confirm the intent and comment it).

### F13 MINOR - dead code and stale text
- HolsterEditor.cpp:107 `qrotate` is never used.
- :91 / :1013 `g_api` is written but never read.
- :84 / :206 `g_debug` is parsed and logged but gates nothing, although holster_editor.ini says "1 = extra debug lines".
- :1 header says 0.2.2-dev, manifest/KPatchInit/statusText say 0.2.4-dev; holster_editor.ini:1 says 0.2.2-dev; the header block (l.3-13) still describes only the D3b page and its probe list lacks EDIT, STANCE, EMOTE, MMB, LAYOUT-edit lines.
- :111-120 `qaxis` is used only by the RMB path (keep); `Editor::strip`, `wpnName` fine.
Fix: delete qrotate/g_api, either drop `debug=` from the ini or gate the WHEEL/BTN logs on it, update the three version strings and header comment.
