# Q-B recon: queue-hud poll sites and companion strip draw gate

Source: patch_manager_mods/queue_hud_src/ (QueueHud.cpp, QueueHudLogic.h, queue_hud_logic_test.cpp). Shared: patch_manager_mods/_shared_src/Party.h.

## B1 Poll sites (who emits QJ / ROUND)

- QJ: `journalTick` at QueueHud.cpp:1496-1545. Called every frame from `onFrameBegin` (QueueHud.cpp:1601).
  - Throttle: 100 ms sample period (`kJournalSampleMs`, :1478), 4 QJ lines/s per member (:1479-1480), 5000 lines per session (:1481).
  - Per member k=0..2: `party::member(tbl,k)` (:1512), `party::serverCreature(m)` (:1513), `actq::read(srv, SiteQueue, SiteRound)` (:1515), `makeQueueSig`, `sigEqual` against `g_jm[k].sig` (:1525-1527), rate window (:1529-1531), `fmtQj` (QueueHudLogic.h:960) then QTRACE only on change.
  - Also logs a creature readable/lost edge (`crOk`, :1517-1519).
- ROUND: `roundWatchTick` at QueueHud.cpp:1129-1162. Runs only from the combat branch of `tickItemQueues` (QueueHud.cpp:1208), and only for members whose `g_roundWatchUntil[k]` is armed (arm at :1116 for 15 s; combat-edge arm `kEdgeWatchMs` = 8000 at :1162).
  - Per armed member per frame, with no time throttle: `member`, `serverCreature`, `gatherFor(srv)` (actq::read plus buffered append), diff against `g_lastRound[k]` (struct at :934), `fmtRound` (QueueHudLogic.h:1152) only on change.
- Out-of-combat per-member loop: `tickItemQueues` (QueueHud.cpp:1202), per frame via :1602. Identity pre-pass, hand-off, drop logic. It does not emit QJ or ROUND.
- Frequency summary: QJ is checked every frame and reads at most every 100 ms. ROUND is checked every frame, only inside armed combat windows.
- Per-member work: creature lookup, one queue and round snapshot via saferead, signature build and diff, then a log line on change only. No HP read.

## B2 Draw gate (companion strips)

- `drawAll` at QueueHud.cpp:720-736, called from `onHudDraw` (:738-741), HUD_DRAW each frame.
  - Gate: `qh::shouldShow(enabled, combat, inMenu, age, 1, showInCombat)` at :728. Inputs per QueueHudLogic.h:125-127: `enabled && (showInCombat || !combat) && !inMenu && ageMs < 250 && nSlots > 0`.
  - On a failed gate: `stripsHidden()` (:730, defined :623) logs "strip k=.. hidden".
  - `drawStrips` runs only if `g_cfg.companions` (:734).
- `drawStrips` at QueueHud.cpp:633-717.
  - Party match: slot creature compared to `cli[k]` / `srv[k]` for k=1..2 (:638-639 and the loop after).
  - Slot geometry: 8 `rd` reads with range checks (vit and card rects).
  - `gatherFor(srv[k])` at :661; `buildSlots` at :663 gives `n`.
  - Hold path: `panelHold` at :666 (QueueHudLogic.h:132-135, 400 ms, held only when not in menu and not combat-hidden without showInCombat).
  - `resolveIcon` per entry; `stripRects` at :700 (0 means skip, :701).
  - `stripNote` shown or hidden at :708-709 (defined :617-622, QLOG "strip k=%d shown|hidden").
  - Draw loop at :716.
- Inputs that gate visibility today: cfg enabled, combat unless show_in_combat, menu, capture age < 250 ms, companions cfg, slot geometry readable, `buildSlots` n > 0 (engine queue or active buffered action), icon resolved, `stripRects` > 0.
- Existing HP / dead / unconscious check in queue_hud_src: NONE. The grep `-i 'hp|dead|unconsc|down|CurrentHP|isDead'` returned only false positives (`hPrefix`, `g_hudPhase`).
- Existing check elsewhere (unused by queue-hud):
  - `Vitals { cur, max, ok, down }` at _shared_src/Party.h:96.
  - `party::vitals(server, &v)` at Party.h:108-121; `v.down = v.ok && v.cur <= 0` at :118.
  - `party::members()` at Party.h:125.
  - companion_selfheal uses these (CompanionSelfHeal.cpp:218, 344, 362). The comment at Party.h says the engine's dead check is never called.
  - QueueHud.cpp includes Party.h (:31) but never calls `vitals` or `members`.

## B3 Per-member data at the poll sites

- `journalTick`: `k` (party index 0..2), `tbl`, `m` (client member pointer), `srv` (server creature pointer), `g_jm[k]` state. Runs at most every 100 ms.
- `drawStrips`: `k` (1..2, matched from the slot's creature to `cli[k]` or `srv[k]`), slot index `i` from `g_ps.slotAt`, `srv[k]`, `g_heldStrip[k].creature`.
- A new `isDown` check can use `party::vitals(srv, &v)` then `v.down` (Party.h:118). It reads the vtable HP slot at 0x9C (Party.h:100) with one thiscall per call. Both sites already hold a non-zero `srv`.
- Caveat: the panel-hold path (:666) would keep repainting a held strip for 400 ms after a companion goes Down unless the Down test runs before it.

## B4 Unit-test sections (queue_hud_logic_test.cpp; all called from `main` at :2196)

- testWaitingAndShow, 276-308: `shouldShow` and `isWaiting` gate. Registered at :2205. This is the natural place to pin a new gate.
- testPanelHoldAndSlotRules, 743-770: `panelHold` boundaries.
- testStripConfig 450; testStripGeometry 473; testStripAvoidsSelfHeal 545; testStripLeftLive 578; testStripHudHostSlots 629.
- testStripCancel 1153; testCancelVisible 1201; testCancelStyle 2148.
- testQueueSig 1489 (QJ signature and `sigEqual`); testLogFormatGolden 1564 (QJ goldens 1651-1672).
- testHandoffClearAndRoundFormat 772 (ROUND goldens 785-812).
- Suggested pin: a pure `bool` helper in QueueHudLogic.h beside `shouldShow` (:125), tested beside testWaitingAndShow. The `Vitals` struct lives in `_shared_src`, so keep the logic to a plain `bool down` parameter.

## OPEN

- Should a Down companion's strip be hidden only at `drawStrips`, or should `journalTick` and `roundWatchTick` also skip Down members?
