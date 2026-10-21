# HW25 item 5: reachArm refusal hold (recon r4, read-only)

Anchors: `patch_manager_mods/holster_src/Holster.cpp` (reachArm 1322-1477, ReachEnt 1269-1274), `HolsterReach.h` (hs::Base 357-370), `holster_reach_test.cpp`.

## 1. Refusal paths in reachArm

| Line | Path | Reason | Class | Logged | Hold? |
|---|---|---|---|---|---|
| 1325 | parent (beh+0x14) null | none | owner gone | silent | no |
| 1344 | find slot 0x10C differs gob vs parent | findslot | permanent per type | refuse=findslot, 5 s cooldown | no (never solved) |
| 1349, 1351 | bic/fore/hand missing or owner/pose slot mismatch | nobone | permanent until parts exist | refuse=nobone | no (never solved) |
| 1364 | entry in 5 s cooldown | none | cooldown | silent | no |
| 1366 | reachStale: owner or vtable of bic/fore/hand changed | none | permanent; entry dropped, leftover write not undone | silent | no |
| 1369 | reachReadQ: unreadable, non-finite, or norm^2 outside 0.9-1.1 | none | transient | silent | YES |
| 1386 | pose: parB/parF null or poseSane fails | refuse=pose | transient | refuse=pose | YES (after persist restore 1380) |
| 1429 | solveTwoBone false: non-finite input or bone length < 1e-4 | refuse=solve | transient | refuse=solve | YES |
| 1373-1377 | w<=0 undo, return false | none | intended slide end | silent | never |
| 1434/1443/1460 | probeWrite fails: partial write, no return | none | structural | silent | no |
| 1465-1468 | carry invalid: (wrote&3)!=3 or attach vt mismatch | none | not an arm refusal | silent | no |

Key fact: `return carriedOk` (1469/1477) is false on every successful non-carry solve. A false return means "not carrying", not "refused". The hold must be keyed on refusal kind.

## 2. State to re-apply last solved locals

Already stored (no new field for the quats):
- `bb/fb/wb.wrote` (hs::Base): last solved local quat, set by reachNoteWrite at 1434/1443/1460. Bits: bicep=1, forearm=2, wrist=4 (wrist only in carry with wrote==3 and wristPct>0).
- The brief says 4 bones. The mod writes 3 (bicep, forearm, wrist-if-found). The hand is read only.
- `.have` is set by reachBase at 1379 even on frames later refused at 1386/1429. It does not mean the last frame solved.

New fields needed in ReachEnt:
- `uint8_t solved`: write-bit mask of the last successful solve (set after 1468 on success).
- `uint8_t holdN`: consecutive held frames (reset on success).

Clearing:
- Slide end (1373-1377): add solved=0, holdN=0. `have` is already cleared there.
- Entry drop: parent=0 at 1363/1366 and ring overwrite at 1333-1335. Owner change is caught by reachStale (1316).
- Do not store world rid (spec wording "store last rid"). The owner moves every frame, so a world pose goes stale. Store locals and recompute carried from attachNode with reachNodePose (HW18 section 2: the pose walks the parent chain each call).

## 3. Proposed minimal design

- Hold only at transient sites 1369, 1386, 1429. Never at 1325, 1344, 1349, 1351, 1364, 1366, or 1373.
- Max hold: kReachHoldFrames = 3 (matches HW25 item 3 grace, about 50 ms at 60 fps). Strict reading of "for that frame" is 1.
- Helper `reachHold(e, attachNode, carry, carried)`: require solved != 0 and holdN < max; re-write the solved bits from bb/fb/wb.wrote via reachWriteQ (probeWrite + finite); if carry and (solved&3)==3, recompute carried using the same vt guard as 1466-1467; ++holdN; return carriedOk.
- Stale guard: reachStale (1366) runs before every hold site. A dropped entry fails the lookup at 1328. w<=0 never reaches the hold.
- Call site 1857 needs no edit: a true return makes the existing rid block run. previewPlace 1595 (editor only) inherits the hold. 1818 undo is unchanged.

## 4. Risks and test strategy

Risks:
- Caller refusals return before reachArm (PrReadIn 1824, PrInsaneIn 1825, PrNoMount 1838/1843, PrInsaneOut 1849, PrNoWrite 1850). The hold does not cover them.
- If the engine does not resample the arm (persist=1, HW18 OPEN-2 for NPC gobs), bones already hold and only the weapon fallback at 1857 snaps. Check persist1 counts in `scripts/holster_log_check.py` (line 467) first.
- Engine blend base (HW18 section 1, 0x4DEA20 reads target+0x14): a 3-frame hold is inside tolerance; longer may drift.
- The hand is not held, so during a hold the forearm-to-hand segment is engine-driven.

Tests:
- Pure: put the decision in HolsterReach.h (holdAllowed(kind, solved, holdN, w, max)) and test in holster_reach_test.cpp. Run `python3 scripts/run_tests.py holster_reach`. Cases: transient+solved+holdN<3 true; holdN==3 false; permanent false; w<=0 false; solved==0 false; carry needs solved&3==3.
- Live: add a pending row to `patch_manager_mods/PLAY_TEST_QUEUE.md` (reach_test=1, reach_carry=1, draw and holster in combat). Pass: `hold=` appears only mid-slide, with no weapon jump at a refuse line.
- Logging first (HW24 pattern): add hold= to the 1470 line and count silent reasons (read, stale, cooldown) before enabling the hold.
