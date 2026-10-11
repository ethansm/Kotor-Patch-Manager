# L1 recon: hud-host BUILD48 and action-hotbar gate (phase H)

Source: kplog_parse.py --bare and kotor2_logs.py check only. No raw log reads.

## 1. hud-host_log.txt (VERDICT: CLEAN, 1 launch, 27 lines)

Register lines (8 total, consumer order = registration order):
- register phase=1 prio=100 owner=combat-hotbar -> 1
- register phase=2 prio=100 owner=combat-hotbar -> 2
- register phase=3 prio=300 owner=combat-hotbar -> 3
- register phase=3 prio=200 owner=companion-selfheal -> 4
- register phase=0 prio=300 owner=queue-hud -> 5
- register phase=3 prio=300 owner=queue-hud -> 6
- register phase=0 prio=100 owner=action-hotbar -> 7
- register phase=3 prio=100 owner=action-hotbar -> 8

First BUILD dispatch: `first BUILD dispatch: panel=08a024f8 consumers build=4`

BUILD48 lines (one dispatch only):
- `BUILD48 entry +0x40 00000000 00000000 ff8fb44f ff90b500`  (+0x48 = ff8fb44f)
- `BUILD48 owner=action-hotbar +0x48 ff8fb44f -> ff8fb44f`
- `BUILD48 owner=companion-selfheal +0x48 ff8fb44f -> ff8fb44f`
- `BUILD48 owner=combat-hotbar +0x48 ff8fb44f -> ff8fb44f`
- `BUILD48 owner=queue-hud +0x48 ff8fb44f -> ff8fb44f`
- `BUILD48 frame0 +0x48 ff8fb44d`

Findings:
- The upper bits (0xFF8F....) were already set at `entry`. No owner changed the value; all four owner lines are no-op (ff8fb44f -> ff8fb44f).
- Only the frame0 line changed it, and only the low byte (4f -> 4d). Upper bits unchanged.
- Panel pointer: 08a024f8. Owner order: action-hotbar, companion-selfheal, combat-hotbar, queue-hud (matches build=4).
- BUILD dispatches: 1 logged ("first BUILD dispatch"). Log does not record later dispatches, so the total count is unknown.
- Missing lines: none for the single dispatch (entry, 4 owners, frame0 all present).
- Also logged: `first dispatch: menu=08a025c4 hud=08a024f8 leader=0a98d060 target=00000000 type=-1 consumers frame=2 pre=1` and `first POST dispatch: menu=08a025c4 frame=1 consumers post=1`.

## 2. action-hotbar_log.txt (VERDICT: CLEAN, 1 launch, 260 lines)

Gate lines (no ACTIVE state logged):
- `gate: inactive (modal open)`
- `gate: inactive (hud not in list)`
- `gate: inactive (other panel on top)`
- `gate: inactive (modal open)`
- `gate: inactive (other panel on top)`

H3 build (one):
- `H3 build: panel=08a024f8 vtable=0099dcc4 controls=111 gff=084715e8 (previous bar panel=00000000 built=0)`

Heartbeat: `heartbeat: frame=1800 gate=0 char='objfffffffd' ring=0 invoke=1 cfgMtime=1791518926855`
Heartbeat: `heartbeat: bar panel=08a024f8 built=10 vis=0000000000 wfaults=0 pfaults=0 efaults=0 ifaults=0 gateflaps=2`

+0x48 values logged by action-hotbar: none. Panel pointer 08a024f8 matches hud-host.

## 3. Not logged (needed to find the writer of panel+0x48 upper bits)

1. +0x48 at HUD ctor 0x00748502 (before any BUILD): no log point. Needed: value at ctor return and after StopLoadFromLayout returns.
2. +0x48 per frame: hud-host logs BUILD48 once. Needed: a per-frame or per-dispatch line `+0x48 <value>` to find the frame of the change.
3. Writer identity: no log of return address or caller for writes to panel+0x48. Needed: a write watch on panel+0x48 logging the caller.
4. Owner-side: action-hotbar logs no +0x48 value at all (gate and heartbeat lines show only panel pointer, flags, vt).
5. Source of the 0xFF8F upper bits: value at the source field (the one written at ctor) is unknown.

## Answer summary
- First owner showing a change: none. Bad value present at BUILD48 entry, before action-hotbar.
- Writers of upper bits: not in these logs. Look at pre-BUILD (ctor/StopLoadFromLayout) points.
