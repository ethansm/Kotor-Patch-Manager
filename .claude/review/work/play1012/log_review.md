# Play session 2026-10-12 00:16:23 log review (about 36 min; read-only)

Method: coarse `kotor2_logs.py check` over 17 logs, then 6 narrow Haiku recon agents (P-A..P-F, reports in this dir), then Opus verification of every disputed claim. All logs are one launch per kplog file (buffhud holds 18 launches, newest = this session).

## 1. Results

| Area | Result | Evidence |
|------|--------|----------|
| Gate fix (+0x48 garbage) | PASS | 7/7 `implausible f48` lines followed by `open=1` (first T=43844.711 -> 43844.714). Only HUD 08af2448 had garbage; later HUDs clean (`00000045`). 0 `hud-ignored`. |
| Down state (the open item from H3) | PASS, 3 episodes | m2 `AutoOn->Down hp=-11/72` T=706355; m1 `Down->Pending hp=3/21` T=710450 right after auto `USE Medpac` (stack 9->8) then `Pending->AutoOn` T=711241; m1 `Hidden->Down hp=-11/72` T=712488. No `USE`/`CLICK` while Down. m2 wakes `Down->NoItem hp=1/72` (no medpac left). |
| Self-heal log hygiene | PASS | 0 ERR, 0 append failed; 5 WARN = `REFUSE already queued member=N (auto)` (benign de-dupe; should log at INFO). Coarse `SELFHEAL P1,P3,P4,P5=PASS`. |
| Gate "stalls" | benign | Two ~75 s `open=0 why=other-panel-on-top` windows: T=294.7-368.9 s top panel 08c50068 = the dialogue letterbox bottom bar (letterbox log: 4 dialogues); T=557.8-633.2 s inventory + equipment + effects panels were open. |
| action-hotbar alert `invalid object id ffffffff` | transient | T=550576 (P-B: not stuck, character resolved afterwards); `objfffffffd` is the PC (empty tag). Not an H3 path; cause unconfirmed. |
| queue-hud | OK | 0 ERR, 1 WARN (blank icon 0x3F), no strip/ring overlap, VANISH eaten_paused 0 (prior session 22). 3 B-flags `orphan-round` after our cancel (T=672959 m2, 679790 m1, 680716 m1): a 0x3F entry reappears 320 ms after a cancel; origin not logged. `VIS cap reached` T=702949 (VIS logging stops there). |
| combat-hotbar | OK | 26/26 pill invokes `restored=1`; VIS probe only logs group `shown=0` and hit its cap (probe gap, not a bug). |
| Buff HUD | session ran 0.2.0, not 0.2.1 | 0.2.1 committed 00:52 (4261c1b), session began 00:16; now LIVE c65dc679. Newest launch has 36 `DBGMISS` (engine flags a debuff, mod draws none), old format without `row=`/`rowIcon=`; e.g. `debuffIcon=- buffIcon=ip_resistenergy*`. This is the Slow case 0.2.1 fixes. Needs a replay. |
| npc-diversity | FAILs are checker artifacts | `RESULT FAIL (2 FAIL, 1 WARN)`: check 16 compares this launch (0.1.4) against 0.1.2 launches (22 of 23 keys); check 11 trips on invalid oid 7f000000 (tv 1 vs 102, `why=skip:vary`, no write). WARN 4 (tag empty) is documented. Live 0.1.4; rule-11 fix sits in unbuilt 0.1.5. No POP-STABLE marker (probe 0.2.0 not live). |
| Probes / engine fixes | OK | mouse-probe SYNTHDBL x1 benign (T=136644, followed by `DBLACT isDbl=1`); letterbox PASS; equip-icon PASS; perf-lab PASS (hangs=0); ranged-ai INFO = ai_trace absent; cmdq-probe 64 of 167 CLR->start gaps >500 ms (informational). |

## 2. Open
1. **Long Down (about 318 s)**: m1/m2 stayed Down from T=706/712 s to T=1030.395 s, then both woke within 5 ms. At that moment queue-hud flips `combat=0` (T=1030498), selfheal logs a P3 party-slot remap (T=1030395.579) and the leader changed at T=1017 (`atton -> objfffffffd`). Companions had only 1-3 queue events/min meanwhile while the player fought. Consistent with a real long knock-out, but a stale Down cannot be ruled out from the logs. Ask the user whether the companions were actually knocked out for that fight.
2. Selfheal and action-hotbar gate logs hit their GATE cap (T=954 s / T=557 s), so gate state after that is unlogged.
3. HP max flips per slot (21 / 72 / 81 / 124, `hp=23/21` at T=1030395) follow party-order changes (P3 remaps), so a freshly remapped slot can show a stale max for one tick.
4. Buff HUD 0.2.1 (Slow icon) and queue-hud orphan-round origin: need a replay / one more log field.
