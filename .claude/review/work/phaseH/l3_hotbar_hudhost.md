# L3 recon: hud-host BUILD48 and action-hotbar (previous probe build)

## Log inventory
- hud-host_log.txt: 32 lines, 1 kplog session header (2026-10-11 02:37:21), 1 launch. Parse: 23 body lines, VERDICT CLEAN.
- action-hotbar_log.txt: 421 lines, 1 kplog session header (2026-10-11 02:37:21), 1 launch. Check: VERDICT ALERTS(1).

## 1. hud-host BUILD48 (+0x48)
- Block 1 entry: `+0x40 00000000 00000000 ff8fb44f ff90b500`. Owners action-hotbar, companion-selfheal, combat-hotbar, queue-hud all read `ff8fb44f -> ff8fb44f`.
- frame0 (block 1): `+0x48 ff8fb44d`.
- Block 2 entry: `+0x40 00000000 00000000 ff8fb447 ff90b500`. All four owners read `ff8fb447 -> ff8fb447`. This is the 0x47 case.
- Not constant: the value changes between the two BUILD48 blocks in the same launch. Not a per-session difference, since there is only one session.
- Panel: `first BUILD dispatch: panel=08a02008`. action-hotbar gate and heartbeat also report `panel=08a02008` throughout. No second panel pointer is printed for block 2.
- Frame0 differs from entry in block 1 (0x4d vs 0x4f).
- Block 2 has no frame0 line.
- Last block-2 timestamp: T=169628.340.

## 2. action-hotbar
- Gate ACTIVE: 0 lines. Gate lines are all `inactive`: `other panel on top` x8, `hud not in list` x6, `modal open` x3.
- Heartbeats: `gate=0` at frame 1800 and 3600. `vis=0000000000` in every `heartbeat: bar` line. The hotbar was never visible.
- `gateflaps` climbs to 14 over the run.
- H3 build lines: `window 000700b2 client=3840x1600 ref=800x600 -> x correction 0.5556` (x2). `projection flag 00a1b9f0 = 1 read=ok` (x2). `picker background rect [11,960,1120,298]` (x2).
- Panels at last gate line: hud=08a02008, mgr=06a90260, panels=4.
- Errors/warnings: one alert, `character: creature 0c3bf3b0 has invalid object id ffffffff (party members=3 slot=0 tag='')`. No ERR/fault lines (wfaults/pfaults/efaults/ifaults all 0).

## 3. Other
- The only alert in either log is the invalid object id line above.
- No crash, no repeated error lines.
- The two slot-state lines `hidden (hud not active)` are normal for a never-active gate.
- Open: gateflaps is high while the gate never goes ACTIVE, so the flap source is unexplained.
