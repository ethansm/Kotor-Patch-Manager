# Phase H3 play-log review (2026-10-11 session, kplog header 18:06:39)

Read-only. Source: action-hotbar, companion-selfheal, queue-hud, hud-host, combat-hotbar, buffhud logs via `kplog_parse.py` / `kotor2_logs.py check`. One launch per log (no second session). Layer 1 = 4 Haiku recon (H-A..H-D); Layer 2 = 2 independent Haiku re-verifies (selfheal, combat-hotbar VIS); Opus read the disputed windows itself.

## 1. Verdict table

| # | Criterion | Result | Evidence (T= ms in log) |
|---|-----------|--------|-------------------------|
| C1 | Implausible f48 -> gate still opens | PASS (fix exercised twice) | action-hotbar `implausible f48=ff8fb445` T=33203.594 then `open=1 why=open` T=33203.595 (hud=08ab2588); again `f48=aaa83c45` T=604639.081 -> open=1 T=604639.085 (hud=386ab650). 15 implausible lines total, 0 `hud-ignored`. |
| C2 | Clean-flag path | N/A (garbage path taken) | |
| C3 | Stuck `hud-ignored` with plausible f48 | PASS | 0 `hud-ignored` lines in action-hotbar and selfheal. `open=0` reasons only `not-found`, `modal-open`, `other-panel-on-top`. |
| C4 | No `GATE cap reached`; transitions sane | PASS | cap 0; action-hotbar open=1 21 lines / open=0 28 lines. |
| C5 | Selfheal gate same as C1-C4 | PASS | selfheal 14 implausible lines (10x ff8fb445 hud 08ab2588, 4x aaa83c45 hud 386ab650), each followed by `open=1`; first open=1 T=33216.652; 0 hud-ignored. |
| C6 | MAP/LEADERBTN geometry | PASS | `MAP member=0 vit=[3511,1386,103,208] card=[3591,1394,191,192]`; members 1/2 `vit=[3671,..,47,96]` (h=96); one LEADERBTN `rect=[3445,1403,39,39] ring=[3436,1394,58,58] disc=58 reserve=157`; 3436+58+17=3511 and ring.y 1394 == card.y 1394. Only one LEADERBTN line, so the rect never moved. |
| C7 | Down state + wake | NOT-EXERCISED | 0 `Down` / `hp=0` lines. 18 STATE lines: AutoOn->Normal 4, Normal->AutoOn 3, FullHp->Normal 3, Normal->FullHp 3, Hidden->FullHp 2, Hidden->AutoOn 1, Normal->Pending 1, Pending->Normal 1. |
| C8 | No click/USE while Down | NOT-EXERCISED | vacuous (no Down). |
| C9 | No append failed / WARN / ERR | PASS | selfheal: 0 append failed, 0 WARN/ERR, 0 REFUSE; coarse `SELFHEAL P1=PASS P3=PASS VERDICT=PASS`. |
| C10 | First-build `+0x48` | garbage present (so C1 applies) | hud-host `BUILD48 entry +0x40 .. ff8fb44f` T=26022.473; second HUD (hud=386ab650) `+0x48=aaa83c45` frame=39363 T=602614.855. Other builds clean (47/45/05). |
| C11 | Queue strip vs selfheal disc | PASS (plan formula corrected) | Strip icons 58x58: k=1 rect (3456,1304,58,58), k=2 (3456,1200,58,58), two-icon row starts x=3396; cancel1 (3384,1304,58,58), cancel2 (3384,1200,58,58). Right edge 3514 = member1 vit.x 3671 - 157 (the logged selfheal reserve). The plan's "vit.x - 66" was a stale constant; 66 is the old fallback reserve. Selfheal ring [3436..3494,1394..1452] does not intersect any strip/cancel rect (strips end y<=1362). `VIS cancel0/1/2 shown=1` seen. |
| C12 | append failed / cap / WARN | PASS | queue-hud and hud-host: append failed 0, cap 0. queue-hud 1 WARN `blank icon: type=0x3F param0=00000001 node=0caa96b8` T=640199.581 (cosmetic, icon load). |
| C13 | Cross-mod WARN sweep | PASS | combat-hotbar 0 WARN/ERR; buffhud and shield-bar non-kplog, `kotor2_logs.py check` CLEAN. No NEW signatures vs H2 baseline. |
| C14 | Other HUD elements still shown | PARTIAL / not a regression | buffhud `VIS slot# why=drawn` x3, `VIS menu# why=drawn` x2. combat-hotbar logs only `VIS group<g> shown=0 why=no-target / empty-group` (0 `shown=1` lines) but the pill path works: `target pills visible=101 btn=47` T=615319.115 and 4 `target click g=0` + `invoke ... restored=1` T=623859..636083. combat-hotbar DLL is untouched by H3. The VIS probe seems to track only the vanilla group controls, not pills. |

## 2. Extra observations
- Max HP shifts per member (m2 81->72->21) trail the panel rebuild at T=249256 and T=444398 and coincide with action-hotbar `character:` changes (atton <-> kreia), i.e. party composition changed. No defect.
- action-hotbar coarse ALERT(1): `creature 0bf59d30 has invalid object id ffffffff (party members=3 slot=0 tag='')` T=604581.310, fires right at the area load before the second HUD rebuild. The `objfffffffd` character is the PC (empty tag) and appears throughout; the alert is a load-time transient, not an H3 path. Not proven pre-existing; one-line watch item.
- Queue strips: `follower 0x3d wiped` x2 (L362/L363) and `eaten_paused` vanishes are queue-hud engine behavior, outside H3.

## 3. Agent contract tally
6 Haiku replies (4 L1 + 2 L2): all 5-line format OK; 5 of 6 `OUTPUT:` were not a path because Explore agents are read-only (plan flaw: use a writer agent type if files are required). `agent_contract.py check` FAILs only on peer-session dirty files (`k1_import/`, `master_ledger.jsonl`); no agent wrote anything. Verdicts: H-A CONFIRMED, H-B PARTIAL (Down), H-C REFUTED (wrong plan constant, resolved by Opus), H-D REFUTED (resolved: pills path works), L2-selfheal PARTIAL (confirms C5/C7), L2-combat PARTIAL.

## Open
- Down state and wake (C7/C8) need a targeted retest.
- Optional: combat-hotbar VIS probe does not cover the pill path (logging gap, not a bug).
- Fresh-launch first-load is now PROVEN for the garbage path (ff8fb445 on the first HUD of the process); the clean-flag path (C2) was not seen.
