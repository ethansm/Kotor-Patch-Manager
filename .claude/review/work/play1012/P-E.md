# P-E recon: play 2026-10-12 (5 mod logs)

Scope: kplog session 2026-10-12 00:16:23 for the four kplog logs. buffhud_log.txt is non-kplog: newest launch is #18 of 18 (lines 3729-4131, 403 lines), read via `log_triage.py --all-launches --json` and `kotor2_logs.py check`. Digit-masked signatures only; raw values read only where kplog_parse.py gives them.

## E1 WARN/ERR, append failed, cap reached

| log | event lines | WARN/ERR | append failed | cap reached | top-3 signatures (count) |
|---|---|---|---|---|---|
| combat-hotbar_log.txt | 220 (all INFO) | 0/0 | 0 | 1 (`VIS cap reached`, L80) | `pills: visible=# cols=# usedCols=# rows=# pill=[#,#] btn=#` (65); `VIS group# shown=# why=no-target rect=[#,#,#,#]` (26); `VIS group# shown=# why=empty-group rect=[#,#,#,#]` (21) |
| character-effects-panel_log.txt | 667 (all INFO) | 0/0 | 0 | 0 | `H# beat: client=#c# server=#bf#d# entries=# first=# over=# stage=#` (11); `H# LBL_EFX_D#: ctl=#c#b# id=# (expect #) ...` (7); `H# LBL_EFX_I#: ctl=#c# id=# ...` (7) |
| equipment-3d-character_log.txt | 257 (all INFO) | 0/0 | 0 | 0 | `H# cover LBL_E#D_M#: ctl=# id=# ...` (12); same with `ctl=#ae#` (7); same with `ctl=#c#` (6) |
| inventory-3d-viewport_log.txt | 112 (all INFO) | 0/0 | 0 | 0 | `H# tick ## panel=#b# slot=usable` (5); `  dump view +#: ...` (4); `  dump view +#: # # #a ...` (4) |
| buffhud_log.txt (launch #18) | 403 | alerts 0 (0 in all 18 launches) | 0 | 0 | `COLUMN## buff slot=# ...` (80); `ARROWS slot=# creature=# ...` (59); `icon[#] ip_knightspeed rect=...` (45) |

## E2 buffhud launch #18

VIS lines (shown value masked in check output; source sets shown=1 iff why=drawn, BuffDurationHUD.cpp:123 and :496):
- `VIS slot# shown=# why=drawn count=# rect=[...]`: 5 (L3735). Inferred shown=1.
- `VIS menu# shown=# why=drawn rect=[...]`: 2 (L3941). Inferred shown=1.
- `VIS slot# shown=# why=no-slot rect=[...]`: 1 (L3745). shown=0.

DBGMISS: 36 effect lines plus 3 creature headers. Five distinct signatures are below. Only four are distinct: the creature header is one of them.
1. L3946 x20: `DBGMISS i=# spell=# type=# sub=# dur=# creator=# debuffIcon=- buffIcon=-`. No debuff icon and no buff icon resolved.
2. L3948 x15: `... debuffIcon=- buffIcon=ip_resistenergy#`. Resist Energy buff icon resolved, no debuff icon.
3. L3955 x1: `... debuffIcon=- buffIcon=ip_battlemed#`. Battle-meditation icon resolved, no debuff icon.
4. L3945 x3: `DBGMISS creature=# raw=# listOk=# count=#`. Per-creature header.
- Effects are named only by spell id, which is digit-masked here. Raw spell ids were not read (check-only rule).
- Format: #18 DBGMISS lines have no `row=`, `n=`, `rowIcon=` fields. HEAD emits them (BuffDurationHUD.cpp:371, added in 4261c1b, 0.2.1). So #18 ran the pre-0.2.1 build and the EffectIcon/Slow path was not exercised.
- Last DEBUFF column line (tail): `COLUMN#80911 DEBUFF ... tracked=0 ... draw=0`.

## E3 combat-hotbar (channel combat-hotbar/main for VIS, combat-hotbar/target for pills, clicks, invoke)

- VIS group0/1/2 and buttons, shown values: only `shown=0`. No `shown=1` logged.
  - group0: no-target 9, empty-group 7
  - group1: no-target 9, empty-group 7
  - group2: no-target 8, empty-group 7
  - buttons: no-target 8, empty-groups 7
- VIS budget: 64 state-change lines per process (CombatHotbar.cpp:298-302). `VIS cap reached` at L80. Every VIS line after that is dropped, including the target-present period (pills visible=101 first at L118).
- Target pills worked: `pills: visible=101` 10 lines; `visible=000` 55 lines.
- Literal `target click`: 0. Click lines (`click g=# idx=# src=# key=#`) in combat-hotbar/target: 26.
- `invoke ... returned ... restored=`: returned 26; restored=1 26; restored=0 0.

## E4 hang / exception / fault / invalid (word match)

- 0 hits in all five logs. Substring `hang` hits seen earlier were `change`/`changed` false positives.
- buffhud: alerts 0 for all 18 launches; check summary VERDICT CLEAN.

## Verdict

REFUTED on the missed-icon criterion: buffhud #18 has 36 DBGMISS effect lines. The #18 build predates 0.2.1, so the 0.2.1 fix is unverified in this play.
combat-hotbar group visibility is unknown, not disproven: the VIS log cap was spent before target pills appeared.

## OPEN

Was buffhud 0.2.1 installed before the 00:16 play? #18 DBGMISS has no `row=`/`rowIcon=` fields.

## Contract note

Reply format passed `agent_contract.py check --type recon`. Its 8 OUT_OF_SCOPE FAILs are other sessions' uncommitted files in the shared worktree (no pre-launch snapshot), not writes by this recon. Only this file was written.
