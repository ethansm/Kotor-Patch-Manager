# Which KOTOR 1 engine bugs exist in KOTOR II? (status table for modders)

The KOTOR 1 community has documented and fixed a set of engine bugs: KMRP (KOTOR Modern Restoration Patch,
`RayesDiyab/KMRP-KOTOR-Modern-Restoration-Patch`) and KPM PR #286 (`K1WidescreenPatch/KMRP-ENGINE-FIXES.md`,
`K1StrayBugFixes`). The two games share most of the Odyssey/Aurora GUI code, so an obvious question is whether each bug
also exists in KOTOR II. This page records what we found, so nobody has to repeat the work.

**Scope and method.** The target is KOTOR II Steam (Aspyr) `swkotor2.exe`, sha256 `6A522E71631DCEE9...FFEF`. Its LAA variant
`4AB72FC1...5A5B` has the same code at every address listed here. GOG Aspyr addresses differ but the code is very
likely the same; we did not check GOG. The analysis was static: Ghidra decompile and disassembly, with constants read from
the vanilla exe. We located each K2 function independently. A matching name was never treated as proof. Every verdict below
was re-checked against the disassembly. Done 2026-10-01/02.
Full evidence (quoted asm, arithmetic, hook windows) is in `engine_research_findings/V1.md` .. `V7.md`. The working
register is `16_candidate_register.md`.

## KMRP K1-K9

| KMRP | K1 bug | In K2? | Why (K2 Steam addresses) |
|---|---|---|---|
| K1 | Word-wrap infinite loop / out-of-memory on text with no break point (progress is compared against the string start, not the line start) | **No hang** | `CAurGUIStringInternal::WrapStrings` 0x47AF30 has the same bad compare (0x47B321), but a hard 100-line cap (0x47B0E8 / 0x47B109) ends the loop. Worst case: a label narrower than two glyphs silently loses its text. V3. |
| K2 | Listbox rows grow taller on every repopulate (Powers/Feats 42 -> 56 -> 126 px) | **No** | The feedback loop is broken in K2. `AddControls` 0x41FC30 saves each row's rect, and `ClearItems` 0x41D290 restores it through SetExtent (vtable +4) when list flag bit 2 is set. The inflated height only reaches locals. V2. |
| K3 | Leading newline in item/feat descriptions | **Not examined** | K2's setter is `CSWGuiText::SetText` 0x416E30. This is cosmetic and an open lead. |
| K4 | Video mode follows the widescreen target | n/a | macOS-only K1 issue. |
| K5 | A single line taller than its text box vanishes (Draw drops lines from the top, including the last) | **Yes, latent** | Same drop loops in `Draw` 0x47B500 (0x47B6CB centre, 0x47B7CB bottom). Vanilla labels are safe up to 3840x1600. Estimated: some 18-unit labels in `character_p.gui` vanish at 5120x1440, because the K2 font scale grows with screen *width* (0x479AB0). Not fixed. A fix needs 2 code caves. V3. |
| K6 | Glyph advance under-measured (+0.25 then truncate), so wrapped text runs ~0.25 px/char past its box | **Yes** | `FADD double [0x98C1D8]` (= 0.25) at 0x47B232, then `_ftol` truncation, while Draw uses exact floats. **Fixed** by `k2-wrap-measure-fix` (re-points the operand to the 0.5 double at 0x987F08). Doc 17. |
| K7 | Dialogue letterbox bars computed as `(H - W/(7/3))/2`, so they go negative past 21:9 | **Yes** | Same formula, with the double 2.3333330154 at 0x9A7D30 read by 7 FDIVs (0x8BACE9, 0x8BADE9, 0x8BAF6E, 0x8BB6A9, 0x8BB9A6, 0x8BBC25, 0x8BC467). Bar heights: 3840x1600 -22, 3440x1440 -17, 5120x1440 -377. K2 differs from K1 in one useful way: the reply panel is clamped to `H - max(bar, 100*fs)` and scaled by a width tier, so replies stay on screen and only the bars vanish. **Fixed** by `k2-dialogue-letterbox-fix` (bar = H/6, hook in `CSWGuiManager::SetScreenSize` 0x411AC1). V1, V7, doc 17. |
| K8 | Minimap zoom mismatch when only the radar is enlarged (KMRP: a zoom bug, not a pan clamp) | **No (as shipped)** | Radar and map sizes come from the GUI rects. The vanilla HUD keeps 120x120 / 512x256. It only appears if a mod enlarges the radar without the map. There is no pan clamp in K2 either. The existing `k2-aspyr-map-aspect-fix` (0x7508AA) handles the separate 4:3 stretch. V4. |
| K9 | Retina display modes | n/a | macOS-only K1 issue. |

## Other leads checked at the same time

| Lead | In K2? | Why |
|---|---|---|
| KMRP `message-popup.md`: popup auto-fit widening capped at a 640x480-authored width | **No** (two items unclear) | The caps are `440*fs` x `280*fs` (fs = 2.0 at 3840 wide), and the start width already exceeds the cap, so the widening branch is dead. Unclear, needs an in-game look: the icon/datapad popup forces an `fs*{400,300}` box, and full-map `CSWGuiInGameMap::Draw` 0x894CD0 subtracts 640x480 under a flag. V4. |
| Peixoto's Jade Empire fix: QueryPerformanceCounter magnitude breaking frame pacing | **No** | A single wrapper, `Timer::Now` 0x73B810, returns int64 microseconds, and every pacing user subtracts int64 values first. A relative-QPC fix would change nothing. The `GetTickCount` 49.7-day UI timers and the profiling-clock overflow are not user-visible. V5. |
| 2DA token > 255 characters overflowing the 256-byte stack buffer | **Only for hand-written text 2DAs** | One parser, 0x71C3D0, handles both text `2DA V2.0` and binary `2DA V2.b`. The binary path copies only column names into the buffer. All 424 BIF tables and normal override tables are binary. The buffer ends exactly at the stack cookie. Scanner: `engine_bug_research/tools/scan_2da_limits.py`. V6. |
| Community "2DA row limits" table (`kotor-patch-manager/docs/2da_row_limits.md`) | **That table is K1** | K2's own caps: appearance u16 (65536 rows); portraits < 0xFFFE; soundset row count is u16, so 65536 rows load as 0; classes row count is a byte, so 256 rows load as 0; heads uses a byte path, ids 1..254. Vanilla `heads.2da` + mods leave only a few spare ids. Placeables, upgrade and baseitems have no K2 row cap found. V6. |
| Aspyr Linux/macOS build symbols, needed to name K2 functions? | **Not needed** | No verdict above depended on a missing function name. |

## Not yet looked at (possible future leads)
K3 (leading newline). KMRP topics with no K2 check yet: `movies.md` (Bink fullscreen aspect fit), `map-markers.md`,
`area-map-surface.md`, `text-padding.md`, `font-atlases.md`, `button-focus-badge-geometry.md`, `texture-residency.md`.
Community reports with no repro yet: stuck idle/combat animation after leaving a conversation; party member reset
to level 1 after load; Soft Shadows slowdown.

## Caveats
- "No" means the K1 mechanism is absent or neutralised in this K2 build's code. It does not mean K2 has no bug of
  that kind.
- L1 and L4 are confirmed in code and arithmetic. Their on-screen effect is being checked in-game as of 2026-10-02.
- If you port a K1 fix anyway, re-derive the K2 addresses. K1 addresses never transfer. A K1 -> K2 name bridge is in
  `engine_bug_research/k1_k2_bridge.tsv` and is a starting point only.
