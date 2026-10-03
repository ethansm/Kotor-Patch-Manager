#!/usr/bin/env python3
"""Persist the doc-15 Phase 3 static triage (V8, V8b, V9, V10; 2026-10-03; Opus-verified) into ghidra_knowledge/kotor2_ghidra_knowledge.db.

Idempotent, same approach as persist_phase2_db.py:
  * lessons are keyed by topic: inserted when absent, updated only when the stored note/scope differs, otherwise left alone;
  * name_map rows are added only where the address has no name yet (existing names are kept and printed).
A second run therefore adds 0 and changes 0.

Usage: persist_phase3_db.py [--db PATH]   (default: the real knowledge DB next to this tool tree)
"""
import argparse, os, sqlite3

DEFAULT_DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
DATE = "2026-10-03"
SRC = "doc15-phase3-2026-10-03"

# (scope, topic, note). Topic is the idempotency key. Source files: engine_research_findings/V8.md, V8b.md, V9.md, V10.md
# (each ends with an "Opus verification (2026-10-03)" section that overrides the body where they differ).
LESSONS = [
    ("movies", "L16 Bink movie scale is width-only (K2 0x40CC70): latent BUG, hook window 0x40D26A",
     "CExoMoviePlayerInternal::OpenClip 0x40CC70 computes ONE float s = clientW / movieW (0x40D262 FDIVP, 0x40D264 FSTP [EBP-0xB4]); the drawn size is ftol(movieW*s) x ftol(movieH*s) "
     "(0x40D28B and 0x40D2BE both FMUL [EBP-0xB4]); the movie height never limits the scale, the client height only enters the centring offset. Same mechanism as KMRP movies.md. "
     "User install: all 85 Movies/*.bik are 3440x1440 (header bytes 20/24), so at 3840x1600 s = 1.1163, drawn 3840x1607, offset (0,-3), 7 rows cropped = invisible. A 640x480 movie would show 55.6 % of its "
     "rows (44 % cropped), a 16:9 movie 74.1 % (26 % cropped), no bars. Hook window for a fix (nothing built): 0x40D26A, 6 B `8B 85 10 FF FF FF` (MOV EAX,[EBP-0xF0]), instruction boundary "
     "(previous FSTP ends there), FPU empty, no branch target inside, nearest live hook 0x4083C5 / 0x411AC1; fix sketch = overwrite the float at [EBP-0xB4] with min(cw/mw, ch/mh) using integer math or "
     "a half-ulp bias (naive float32 fit loses a row: 480*3.33333325 -> 1599). The function runs on the MOVIE THREAD (0x40D430), so a detour must only read memory. Status confirmed (latent); "
     "only matters if a non-3440x1440 movie set is installed. Source V9 Q1a + Opus verification."),
    ("movies", "L17 Bink equal-width fast path uses unsigned SHR for the offsets (latent, unreachable on this install)",
     "0x40CC70: when clientW == movieW the JZ at 0x40D229 jumps to 0x40D32F, which computes the offsets with logical shifts (`SHR EAX,1` at 0x40D340, `SHR EDX,1` at 0x40D360 after SUB). "
     "A 3840-wide movie taller than 1600 (e.g. 3840x2160 on 3840x1600) gets y offset 0x7FFFFEE8. None of the 85 installed movies is 3840 wide, so unreachable here. Window (not built): 0x40D218, 6 B "
     "`8B 55 E0 2B 55 D8` (MOV EDX,[EBP-0x20]; SUB EDX,[EBP-0x28]); fix sketch = replace with XOR EDX,EDX / INC EDX / NOPs so CMP EDX,[ECX] never matches and the scaled branch (s = 1.0) is used. Source V9 Q1b."),
    ("movies", "No 640x480 display-mode switch around movies in K2 (0x40BE50 re-applies the CURRENT W/H); the only 640x480 request is credits-gated",
     "Movie entry 0x40BE50 (name_map: Movie::PrepareWindowForMovie) saves [0x9F42A4]/[0x9F42A8] into [0x9F42C0]/[0x9F42C4] and re-applies the CURRENT mode via 0x409750 (PUSH 0x20 at 0x40BEF6, "
     "[0x9F42A8] at 0x40BEF8, [0x9F42A4] at 0x40BEFE); exit 0x40BF40 restores it. The only `PUSH 0x1E0` / `PUSH 0x280` pair is 0x407F87/0x407F8C in the main loop 0x407920, gated on [client+0x4C4] "
     "(single writer 0x7A1400, caller 0x8925F0 next to credits_p); the credits GUI also forces the W/H globals at 0x892207/0x892211. NOTE: name_map calls 0x7A1400 `SetEvilEndingHackInProgress` (GOG mapper); V9 inferred "
     "'credits flag' from its caller only (SPECULATIVE), so treat the flag name as unresolved. K2 also PatBlts the MAIN window black before showing the movie popup (0x40D3E5) but the movie window class brush is NULL "
     "(0x40CD15), so what a letterboxed movie shows beside the picture is SPECULATIVE. Source V9 Q1c/M4."),
    ("gui-text", "L18 K3 leading newline: K2 builds descriptions as props + description with every newline APPENDED (no bug)",
     "CSWSItem::GetPropertyDescription 0x607790 ends with `out = props + description` (ECX = props accumulator at 0x6079DB, CALL 0x733DF0 CExoString::operator+ at 0x6079DE). Every property builder "
     "0x609110..0x610FC0 and 0x615FF0 appends '\\n' (PUSH 0x986B2C) as the SECOND operand of acc + '\\n' (ECX = [EBP+8] before CALL 0x733DF0; 20 sites scanned); block header = strref + ':\\n' (PUSH 0x997608). "
     "Result: 'Header:\\nline\\n...\\n' + '\\n' + description = a blank line BETWEEN properties and description (by design), never a leading newline. Journal 0x7FC390 = '<planet>:\\n' + body. "
     "CSWGuiTextParams::SetText 0x416E30 (103 bytes) does not trim and does not need to. Only 2 of 136,540 dialog.tlk strings start with '\\n' (strref 124480 and 124904, Droid Ion Blast descriptions); one empty line = 38 px "
     "(fnt_d16x16b, fontheight 0.19 x fs 2.0 x 100) or 32 px (dialogfont16x16b) at 3840x1600. Status killed. Optional log-only probe P1 0x416E43 (10 B `8B 4D FC C7 41 08 FF FF FF FF`). Source V8 Q1."),
    ("gui-listbox", "L19 listbox client rect always reserves the scrollbar width (same as K1, by design); SetPadding validates the OLD value",
     "CSWGuiListBox::SetExtent 0x41D710: client width = rect width - [this+0x118] (scrollbar width) ALWAYS (0x41D77E SUB EAX,[EBP-0x14]), left += it under LEFTSCROLLBAR (bit 4 of [+0x2CC], 0x41D775). "
     "At 3840x1600 (800-unit GUI x 4.8) the 16-unit scrollbar = 76.8 px = 6.5 % of the 246-unit inventory description box (6.9 % journal, 7.3 % upgrade); the ultrawide generator narrows boxes but keeps the 16-unit bar, "
     "so the share is ~1.3 points higher than vanilla. SetPadding 0x41E9D0 compares the OLD stored byte (0x41E9EA) against client size / 2 and stores the NEW value unchecked (0x41EA24..0x41EA27): "
     "effectively inert, harmless (all 58 shipped/override listboxes have PADDING 0). Status killed (by design). Source V8 Q2."),
    ("gui-text", "L20 font metrics: spacingR read by breaker AND Draw (font+0x10); name resolver 0x413CE0 force-appends 'b'",
     "TXI parser 0x424F60 stores spacingR at CAurFontInfo+0x10 (PUSH 0x988418 'spacingR' at 0x425069, ADD EAX,0x10 at 0x42507D). Readers: WrapStrings 0x47B223 (+ 0x47AFB1, 0x47B2F4 ...) adds spR*scale*100 px per glyph; "
     "Draw 0x47C5AA adds ~spR*texW*scale^2*1e4/W (6.7 % of the breaker value at 3840). All 15 stock font TXIs have spacingR 0 except fnt_console (0.02): no effect here. Font-name resolver 0x413CE0 (sole caller "
     "0x416D60) appends 'b' to every name except fnt_console because the 'Use Small Fonts' byte read at 0x413D2C is overwritten by `MOV byte [EBP-2],2` at 0x413D35 (the a/resolution branches are dead). So authored "
     "dialogfont10x10 renders as dialogfont10x10b, whose TXI fontheight is 0.16 (the 16x16 height): 4 mipc28x6_p HUD controls are 60 % taller than authored; the INI key Use Small Fonts is dead. The default FONT "
     "'dialogfont16x16' (0x9876A0) only fills a MISSING font (0x4174D2, 0x416A61). Opus: the game Override has NO font files, so the stock TXIs are the fonts in use. Cosmetic, informational (no fix). Source V8 Q3."),
    ("gui-text", "L4 (k2-wrap-measure-fix, 0.25 -> 0.5) is a no-op at 3840x1600 and fs 1.0/3.0; it only changes fs 1.5",
     "Breaker advance x = ((u1-u0)*texW + spacingR) * scale * 100 + 0.25, then _ftol2 (0x47B203..0x47B238). With the real stock TXIs (swpc_tex_gui.erf; texturewidth 2.56 for dialogfont16x16b, 5.12 for fnt_d16x16b) "
     "every glyph advance at fs 1/2/3 is within 0.0006 px of an integer (TXI coordinates are 6-decimal roundings of integer cells), so +0.25 already rounds correctly: glyphs where trunc(x+0.25) != trunc(x+0.5) = 0 of 95 "
     "at fs 1.0, 2.0 and 3.0; 29 (dialogfont16x16b) / 14 (fnt_d16x16b) at fs 1.5 (screen widths 2046..3069). V3's 'mean 0.25 px/char under-measure' was modelled with uniform fractional parts and is REFUTED for stock fonts. "
     "Opus independently recomputed the table with pykotor. Plain truncation (no bias) would be wrong for 46/95 and 58/95 glyphs, so the bias itself matters, only the 0.25 vs 0.5 choice does not. Remaining truncations "
     "(first glyph of each line 0x47B020, break widths 0x47B2C8.., Draw fallback 0x47BCB1) are bounded by 1 px each. Keep the patch (harmless) or drop it at the next install (user choice). Source V8 F3c + Opus."),
    ("area-map", "L21 full area map (K2): canvas/MapHider same size, markers fs-scaled, hit test same frame, 0x894CD0 640x480 centring skipped (no bug)",
     "CSWGuiInGameMap ctor 0x893950 and OnPanelAdded 0x894320 size the canvas image (this+0x13B8, CSWGuiImage) and the fog/marker overlay CSWGuiMapHider (this+0x1160) with the same expression "
     "{0,0,ftol(xs*588.0),ftol(ys*294.0)} (FMUL [0x9931F0]=588.0 / [0x9931E0]=294.0 at 0x894449/0x8944A3 and 0x89445A/0x8944B7): no 512-vs-440 split as in K1. With live k2-aspyr-map-aspect-fix (0x894435, 0x895E6D) both are "
     "1568x784 at 3840x1600, LBL_Map (1136,448,1568,784), centre x 1920. Markers: hard-coded .rdata doubles x fs (selected note 20.0 @0x994700, other notes 14.0 @0x9A6950, party 16.0 @0x9946D0, arrow 32.0 @0x99D798; "
     "40/28/32/64 px at fs 2.0 = 75 % of the vanilla proportion), inner image rects are overwritten by CSWGuiImage::SetExtent each frame (stretch mode), one constant per draw path. Hit test: 0x894DC0 passes "
     "(x-[this+0x6C], y-[this+0x70]) to MapHider::HitCheckMouse 0x8972C0 which tests the same control rects Draw uses. 0x894CD0 Draw: `AND EDX,1 / JNZ 0x894D44` at 0x894D08..0x894D0E skips the (W-640)/2,(H-480)/2 shift "
     "because LoadPanel 0x40EE40 sets bit 0 of [this+0x48] (0x40F364/0x40F367, called on this at 0x893A9C): answers test card T4 statically (runtime bit not measured; optional probe 0x894D08 6 B `8B 51 48 83 E2 01`). Status killed. Source V9 Q2/Q3."),
    ("gui-button", "L22 button focus: HILIGHT border replaces the normal one; fill inset = DIMENSION only with a CORNER image (authored design, no bug)",
     "CSWGuiButton::Draw 0x41A0A0 draws border block +0xE8 when [this+0x48]&1 (focus, set by SetActive 0x41A290) else +0x70 (JZ at 0x41A0B0), then the text at +0x160. CSWGuiBorder::Draw 0x415300 insets the fill by "
     "DIMENSION (raw GFF int at border+0x14, never scaled by fs) only inside `CMP [ECX+0x6C],0 / JZ 0x415936` at 0x4153A0 (CORNER image present); INNEROFFSET only widens the text rect (0x414D80, computed once in 0x419ED0). "
     "Census of 1110 buttons (Override first): only 17 galaxy-map planet buttons (normal inset 6, focus 0: fill 74.4x73.3 -> 86.4x85.3 px, +16 % at 3840x1600) and 6 inventory BTN_FILTER_* (2 px gradient inset on focus) change inset with focus. "
     "The user's generator never edits DIMENSION/INNEROFFSET/HILIGHT. Status killed. Source V10 Q1."),
    ("textures", "L23 texture residency: budget loop exists but the pixel-changing downsample (0x4258B0) is bypassed by JMP at 0x4258B9; do NOT 'fix' it",
     "Budget = texpacks.2da row [Texture Quality] (user 2: Mem 134217728, DynMemRatio 0.75, pack swpc_tex_tpa) via SetTexturePack 0x78D6A0 -> 0x4218C0 (ratio) and AurTextureSetMem 0x427DE0. K2 splits it into dynamic list A "
     "(+0xD7 != 0; hard limit 100,663,296, target x0.8 = 80,530,640) and static list B (33,554,432), selected by the creation flag [0x9F4768] (0x428F50 = 0 static, 0x428F60 = 1). Enforcer 0x427B60 (only caller 0x427DE0, "
     "which runs at init, SetTexturePack and GL-context re-creation, never per frame) bumps the downsample level (+0xC0) of the largest candidate until the accounted size fits: pure bookkeeping, because ApplyDownSampleLevel 0x4258B0 starts "
     "with `004258B9 E9 E4 04 00 00 JMP 0x425DA2` (the body 0x4258BE..0x425D9B is still in the exe, dead; +0xC2 never leaves 0) so no texture is ever uploaded smaller than its file. Resident TPC 205,080,341 B vs Mem 134,217,728 "
     "(1.53x; 2.55x list A target) but nothing acts on it. The accounting is a one-shot snapshot: only 0x427DE0 assigns the totals [0xA1BBA0..], the dtor 0x423480 subtracts, nothing adds at creation, so perf-lab gl_bytes_est is "
     "negative and falling (-49 MB ... -680 MB). WARNING: re-enabling 0x4258B9 would downsample the user's HD set. SetTexturePack/context re-creation also reset and force-construct every texture (reload hitch). "
     "White-first-draw (KMRP lead) UNCLEAR, parked: CSWGuiBorder::Draw calls the GUI batch flush 0x47EA60 (-> 0x427F30 -> 0x427810, drains all pending texture builds) before and after the fill (0x415940), which argues against it. "
     "Status killed. Source V10 Q2 + Opus."),
    ("dialogue-letterbox", "V8b letterbox draw path: Draw 0x8BB260 slides the bar, SetExtent 0x8BAC30 mode 2 forces h = H - y, base Draw 0x40EC90 skips h <= 0, fill 0x479460 = black alpha quad",
     "Letterbox vtable 0x9A7CBC: +4 SetExtent 0x8BAC30, +0x34 Draw 0x8BB260 (manager loop 0x4113A0 calls it for panels with flag bit 2). Mode [this+0x88]: 1 SetTop (0x8BAD7C), 2 SetBottom (0x8BAF01), 3 SetFull. Draw slides per frame by "
     "trunc((target - origin) / 0.5 * dt) (FDIV [0x987F08] at 0x8BB56E; ~0.5 s, 34 frames at 60 fps; step 0 at >= 535 fps), sets done flag [+0x8C] at 0x8BB446/0x8BB4D9 (reset at every conversation end by 0x7C6A20), then ALWAYS calls base "
     "panel Draw 0x40EC90 (0x8BB541). SetExtent: mode 1 forces y = 0, mode 2 forces h = H - y (0x8BAC82..0x8BAC89), then CSWGuiPanel::SetExtent 0x40E980 stores +4..+0x10. Base Draw skips when !bit6, x<0, y<0, h<=0 (0x40ECC8 CMP [ECX+0x10],0 / JLE 0x40EE3A), "
     "w<=0 or the rect leaves W x H; otherwise the fill 0x479460 draws ONE untextured quad (glViewport + unit quad), colour [+0x54..] = (0,0,0), alpha [+0x50] = 1.0 vanilla (ctor 0x40E700, ResetFade 0x8BAFD0) or 0.5 with live "
     "semi-transparent-letterbox (alpha byte 128). Consequence (corrects V1 B5): vanilla at 3840x1600 draws NO top bar (h = -22 skipped) and a 200 px bottom BAND y 1400..1600 (bottom y clamps to H-100*fs = 1400, h = H - y); with L1 "
     "bars are 267 px top (262 after SetReply, 0x8BB8D0 -> 0x8BAB20) and 267 px bottom (y 1333), alpha 0.5 live. 'Letterbox only for cinematic conversations' is REFUTED: every ConversationType != 1 (0x6C5762 CMP [EBP-0x34],1; type 1 = Computer panel [+0x44]) "
     "uses panel [+0x40] and SetTop/SetBottom via 0x7BFB10. Visual still unconfirmed (screenshots missing). Optional log-only windows (all clean, nearest live hook >= 149 B): 0x8BB461 6 B, 0x8BB52B 6 B, 0x8BB538 9 B, 0x40ED9F 5 B, 0x7CEB95 7 B. Source V8b + Opus."),
    ("dialogue-letterbox", "Letterbox post-process side data: [0xA309B4]/[0xA309B8]/[0xA309BC] (bar edges + active flag) clip five composite passes; misleading mapper names in the dialog area",
     "0x8BB467 stores the top bar's current h to [0xA309B4] and 0x8BB531 the bottom bar's y to [0xA309B8]; [0xA309BC] = (arg == 1) set by 0x7CEB80 (dialog start 0x545B9D PUSH 1, close 0x7C6CB3 PUSH 0). V8b read five post-process composite passes "
     "(0x4426F0, 0x4421F0, 0x442C90, 0x443190, 0x445310, called from 0x437BD0) that limit their full-screen quad to y in [b4, b8] while the flag is 1 (SPECULATIVE; consumers not traced by Opus). Name caution from the same batch: Ghidra FID label "
     "CMFCVisualManagerBitmapCache::IsCached on 0x410800 is wrong (it is GuiManager::Contains); mapper names IsFadingInDialog (0x7CB750) / IsCameraDialog (0x7CB6A0) were judged misleading by V8b (content: 'current panel == cinematic panel' and "
     "'SetFade forwarder for the FULL panel only'); re-check 0x7CB750's name_map row before relying on it. Source V8b (d)2/(f)."),
    ("method", "Phase 3 triage method: Opus verification section overrides the analyst body; contradictions seen",
     "Rule: in each V8/V8b/V9/V10 file the trailing 'Opus verification (2026-10-03)' section overrides the body. Overrides found: V8b bar facts (vanilla draws a 200 px bottom band, NOT a 22 px strip, and no top bar; A/B expectation for T1 is "
     "enabled=0 -> no top band + 200 px bottom band, enabled=1 -> 267 px top + 267 px bottom); V8 'user Override has dialogfont16x16b.tga...' is wrong (Override has no font files); V10 body said R3/U5 called 0x4258B0 'empty' (the body exists but is "
     "jumped over). Wording trap: V10 verdict 4 says 'HD textures are being silently downsampled: NO BUG' - read as 'no, they are not downsampled'. Constants are read from the vanilla exe, not the installed one; each BUG window was re-read byte-for-byte "
     "(0x40D26A, 0x40D218) and grepped for branch targets. Outcome: 0 user-visible bugs, 1 latent with a hook window (L16 movies), L4 found to be a no-op at the user's resolution."),
]

# (steam_addr as stored in name_map, name, confidence, evidence). Added only where the address has NO name yet.
NAMES = [
    ("0x008bb260", "CSWGuiDialogLetterbox::Draw", "high", "V8b; vtable 0x9A7CBC slot +0x34; slide step then base panel Draw 0x40EC90"),
    ("0x008bac30", "CSWGuiDialogLetterbox::SetExtent", "high", "V8b; vtable slot +4; mode 1 forces y=0, mode 2 forces h=H-y, then 0x40E980"),
    ("0x008ba980", "CSWGuiDialogLetterbox::Constructor", "high", "V8b; vtable 0x9A7CBC, [+0x88]=0, [+0x8C]=0, colour from 0xA7FD34"),
    ("0x008bafd0", "CSWGuiDialogLetterbox::ResetFade", "high", "V8b; alpha [+0x50] = 0 for mode 3, 1.0 for modes 0/1/2"),
    ("0x008bab20", "CSWGuiDialogLetterbox::ResizeToText (SPECULATIVE name)", "low", "V8b; target grow or direct rect set, used by SetReply 0x8BB8D0 / 0x8BC100"),
    ("0x008bb550", "CSWGuiDialogLetterbox::SlideStepTop", "medium", "V8b; ftol((target h - [+0x84]) / 0.5 * dt)"),
    ("0x008bb590", "CSWGuiDialogLetterbox::SlideStepBottom", "medium", "V8b; ftol((target y - [+0x7C]) / 0.5 * dt)"),
    ("0x0040e980", "CSWGuiPanel::SetExtent", "high", "V8b; stores rect at +4/+8/+0xC/+0x10, forwards (0,0,w,h) to child [+0x60] vtable +4"),
    ("0x00479460", "GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected)", "high", "V8b; rejects w/h < 1, x/y >= display size, colour (-1,-1,-1), alpha byte 0; (-2,-2,-2) = black alpha 0x96"),
    ("0x007bfb10", "CGuiInGame::ShowDialogPanels (SPECULATIVE name)", "low", "V8b; SetFull/SetTop/SetBottom + manager add (0x410530) for the current dialog panel"),
    ("0x006c53f0", "dialog-open handler (reads ConversationType)", "medium", "V8b; PUSH 0x99A16C 'ConversationType' at 0x6C55D2, selects panel [+0x44] (type 1) or [+0x40]"),
    ("0x00413ce0", "CSWGuiManager::ResolveFontName (appends 'b')", "medium", "V8 Q3; name SPECULATIVE, body high; fnt_console exempt; sole caller 0x416D60"),
    ("0x00416d60", "CSWGuiTextParams::SetFontRename", "medium", "V8; resolves the base font name through 0x413CE0"),
    ("0x00416de0", "CSWGuiTextParams::SetBaseFont", "medium", "V8; SetFont when the resolved name differs"),
    ("0x00424f60", "CAurFontInfo::ParseTXIKey", "medium", "V8; name SPECULATIVE; strings 'spacingR' (0x988418) -> font+0x10"),
    ("0x0041d710", "CSWGuiListBox::SetExtent", "high", "V8; client width = width - [this+0x118]; ret 4"),
    ("0x0041e9d0", "CSWGuiListBox::SetPadding", "high", "V8; validator compares the OLD stored byte"),
    ("0x0041ffd0", "CSWGuiListBox::Load", "high", "V8; reads BORDER, SCROLLBAR, LEFTSCROLLBAR, PADDING, COLOR, LOOPING, PROTOITEM"),
    ("0x0041d8e0", "CSWGuiListBox::SetLeftScrollbar", "medium", "V8; toggles bit 4 of [+0x2CC]"),
    ("0x0041d810", "CSWGuiListBox::LayoutScrollBar (SPECULATIVE name)", "low", "V8; sets the embedded scrollbar control rect (this+0x10C)"),
    ("0x007fc390", "journal entry detail updater (SPECULATIVE name)", "low", "V8; '<planet>:\\n' + body -> SetText"),
    ("0x00414980","CSWGuiBorder::GetDimension", "medium", "V10; returns [border+0x14] if non-zero else the corner image width"),
    ("0x0041a0a0", "CSWGuiButton::Draw", "high", "V10; focus border +0xE8 when [+0x48]&1 else +0x70, then text +0x160"),
    ("0x00419f80", "CSWGuiButton::Load", "high", "V10; tags BORDER -> +0x70, HILIGHT -> +0xE8, text -> +0x160"),
    ("0x0041a290", "CSWGuiButton::SetActive", "high", "V10; sets bit 0 of [+0x48] (focus)"),
    ("0x00419ed0", "CSWGuiButton::SetExtent", "high", "V10; forwards to both borders, sizes the text from their inner rects"),
    ("0x00416290", "CSWGuiImage::Constructor", "high", "V9; vftable CSWGuiImage::vftable at 0x98764C"),
    ("0x00416530", "CSWGuiImage::SetExtent", "medium", "V9; vtable +4; copies the control rect into the draw rect when not in a fixed-rect mode"),
    ("0x004168b0", "CSWGuiImage::Draw", "medium", "V9; vtable +0xC"),
    ("0x00893950", "CSWGuiInGameMap::Constructor", "high", "V9; loads map_p, sizes canvas (+0x13B8) and CSWGuiMapHider (+0x1160)"),
    ("0x00894320", "CSWGuiInGameMap::OnPanelAdded", "high", "V9; re-sizes canvas + MapHider on every open; texture lbl_map%s"),
    ("0x00894dc0", "CSWGuiInGameMap::HitCheckMouse", "high", "V9; passes (x-[this+0x6C], y-[this+0x70]) to MapHider::HitCheckMouse"),
    ("0x00895620", "CSWGuiMapHider::Constructor", "high", "V9; textures blackdot, mm_barrow, lbl_mapcircle, whitetarget"),
    ("0x00895cc0", "CSWGuiMapHider::Draw", "high", "V9; fog quads, note controls, party markers, player arrow"),
    ("0x008972c0", "CSWGuiMapHider::HitCheckMouse", "high", "V9; own rect, then selected control and pool controls (vtable +0x44)"),
    ("0x0078d6a0", "CClientExoApp::SetTexturePack (texpacks.2da row: Texture, Gui, DynMemRatio, Mem)", "medium", "V10; ret 8 here; calls 0x4218C0 (ratio) and 0x427DE0 (Mem)"),
    ("0x004218c0", "Global::AurTextureSetDynamicMemoryRatio", "medium", "V10; stores [0x9F4758] (K1 name by analogy)"),
    ("0x00425db0", "CAurTextureBasic::GetMemoryUsage", "medium", "V10; shift = +0xC0 - +0xC2; S3TC chain size via 0x483360 (K1 name by analogy)"),
    ("0x00483360", "S3TC full-chain byte size (sums every mip)", "high", "V10; ((w+3)>>2)*((h+3)>>2)*(8 or 16) per level until 0x0"),
    ("0x004268f0", "CAurTextureBasic::IsDownSampleCandidate (SPECULATIVE name)", "medium", "V10; +0xC0 < +0xBC and (w>>shift) > 2 && (h>>shift) > 2"),
    ("0x0047ea60", "GUI quad-batch flush (drains pending texture builds via 0x427F30)", "medium", "V10; called before and after the fill in CSWGuiBorder::Draw"),
]

def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--db", default=DEFAULT_DB, help="knowledge DB path (default: the real DB)")
    args = ap.parse_args()
    db = os.path.abspath(args.db)
    c = sqlite3.connect(db); cur = c.cursor()
    have = {r[0]: (r[1], r[2]) for r in cur.execute("select topic,scope,note from lessons")}
    nxt = (cur.execute("select max(id) from lessons").fetchone()[0] or 0) + 1
    ins = upd = same = 0
    for scope, topic, note in LESSONS:
        if topic in have:
            if have[topic] == (scope, note):
                same += 1; continue
            cur.execute("update lessons set scope=?, note=?, date=? where topic=?", (scope, note, DATE, topic)); upd += 1; continue
        cur.execute("insert into lessons (id,scope,topic,note,date) values (?,?,?,?,?)", (nxt, scope, topic, note, DATE)); nxt += 1; ins += 1
    added = kept = 0
    for addr, name, conf, ev in NAMES:
        row = cur.execute("select name,source from name_map where lower(steam_addr)=?", (addr.lower(),)).fetchone()
        if row:
            print("name_map keeps existing", addr, row[0], "(%s)" % row[1]); kept += 1; continue
        cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)", (addr, name, SRC, conf, ev, DATE)); added += 1
    c.commit()
    print("db", db)
    print("lessons added", ins, "updated", upd, "unchanged", same, "| names added", added, "kept-existing", kept,
          cur.execute("pragma integrity_check").fetchone(), "lessons:", cur.execute("select count(*) from lessons").fetchone()[0],
          "name_map:", cur.execute("select count(*) from name_map").fetchone()[0])

if __name__ == "__main__":
    main()
