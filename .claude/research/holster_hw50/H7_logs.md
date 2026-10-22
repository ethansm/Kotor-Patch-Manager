# H7: real-play heavy draw/stow logs (HW50)

Read-only recon. No game-dir file was written. Copies made for inspection live in
/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/H7/

## Sources found
1. Live holster_log.txt (game dir, 28 lines, session 10-22 01:02, build 0.21.1-dev): no heavy, no SLIDE, no REACH. SM-L15 and SM-L17 both NO-DATA.
2. Rotated holster_log.txt.1 (game dir, 5,514 lines, session 10-22 00:48-00:58, build 0.21.1-dev): 0 SLIDE, 0 REACH, 0 SNAP, 0 heavy. Only cat=pistol, melee, and rifle (w_BlstrRfl_001). Both rows NO-DATA.
3. Scratch prb1_logs/holster_log.txt (other session, build 0.1.0-dev, 10-16): no SLIDE, REACH or SNAP, no heavy.
4. Scratch extracts be2e1c91.../scratchpad/all.txt and place.txt (build 0.11.0-dev, two sessions, play date unknown). The only heavy evidence: w_hvrptbltr_001, cat=heavy, gob d74dbc0.
5. Frozen archive ~/Games/KOTOR2_Mod_Workspace/logs_archive/ holds only 10-03 and 10-04 folders. No holster logs.
- PLAY_TEST_QUEUE.md: SM-L15 and SM-L17 are `pending | 10-21`. No user result recorded. SM-L18 step 2 (heavy) is pending 10-22.
- doc 42c HW50 row: "user report 2026-10-22". No measurements attached.

## Numbers
- Heavy, 0.11 extract: gob d74dbc0 has 157 PLACE lines and 0 SLIDE, 0 REACH, 0 SNAP, 0 DRAWGATE, 0 MOUNT lines. Its owner cba3218 (P_AttonBB, preview) ran 6 draw/stow cycles on its two pistols (12 SLIDE starts) and none on the heavy.
- Heavy PLACE, n=7313..7357 (12 logged steps): pos0 z 0.90 to 1.56 m, y -48.71 to -48.43, x -44.71 to -44.63. Sample index only: no ms, no frame dt.
- All 60 SLIDE starts (melee 36, pistol 24): draw 350 ms = 17-21 frames, maxdt 21-30 ms; draw 570 ms = 25-34 frames, maxdt 20-32 ms; stow 350 ms = 15-21 frames, maxdt 19-34 ms; stow 900 ms = 43-53 frames, maxdt 21-25 ms.
- REACH, 0.11 extract: 92 lines, 9 owners. Fields: w, reach, S/E/H/T, side, cat (numeric, no legend), u, lt, err, persist, wrote. No topo, hg, tw, sw, vo, ex, of: those start at builds 0.15-0.20 (holster_log_check.py lines 159-165). None of them has a heavy slide.
- 0.21.1 rotated session: CONFIG slowx=2.00, snaplog_m=0.050 and place=1 (TUNE boot), yet 0 SLIDE lines. Inference: no slide started in that window (cap max_lines=30000 not reached at 5,514 lines).

## Gaps (logging needed to judge "weight")
1. SLIDE start/end carry no cat= or mname=. Heavy identity needs a gob join to PLACENODE. Add both.
2. SLIDE end has no gob=, so start/end pairing uses owner order only. Add gob=.
3. No per-slide peaks: peak weapon speed (m/frame), peak rotation per frame (deg), frame of each peak. SNAP fires only above snaplog_m 0.05 m per frame.
4. REACH cat=N is numeric with no legend in the line. Print the category name.
5. No slide-end summary of topo, hg, vo (via), tw/sw (wrist) and of (outward). SM-L15 and SM-L17 cannot be read per heavy draw.
6. PLACE lines have no slide id or state, and their sample step changes (4 early, about 800 later). Timing cannot be recovered from them.

## Next action
Add cat= and mname= to SLIDE start, and gob= to SLIDE end (gaps 1-2). Then one heavy draw and stow in play (SM-L18 step 2) gives heavy numbers.
