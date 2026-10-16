# PRB1 play-test sweep (10-16, about 19:10-19:25)

Scope: 7 logs via `kotor2_logs.py check`, game-dir crash scan, plus `holster-editor_log.txt` for item 4.

## 1. Crash evidence: NO
- Game dir, mtime under 60 min: no dump, minidump or report file. Newest non-log files are `swkotor2.exe` and its backup at 19:02, the install step before play.
- load-hang-probe: 2 load screens, each UP paired with DOWN (f2855 to f2944, 9327 ms; f9517 to f9594, 7553 ms). No UP without DOWN.
- Tails of all 7 logs end on normal events. None stops mid-session.

## 2. Per-log verdict and WARN/ERR
| log | check verdict | WARN/ERR | note |
|---|---|---|---|
| preview-3d_log.txt | CLEAN (43 lines) | 0 | section 3 |
| hud-host_log.txt | CLEAN (46 lines) | 0 | owners registered: combat-hotbar, queue-hud, action-hotbar, companion-selfheal |
| mouse-probe_log.txt | CLEAN (1027 lines) | 0 | section 4 |
| load-hang-probe_log.txt | ALERTS(36) | 0 real | false positive, see below |
| equipment-3d-character_log.txt | CLEAN (124 lines) | 0 | slot 0 ready, EquipDestroy at 199451 |
| k2-robustness-fixes_log.txt | CLEAN (16 lines) | 0 | all COUNTERS 0, no FIRED lines |
| k2-texture-memory-release_log.txt | CLEAN (18 lines) | 0 | tpc_freed 277 to 536, kept_controller 7 to 8 |

- load-hang ALERTS(36) is a false positive. `log_triage.py` alerts on `\bhang\b`, and the channel tag `load-hang-probe` contains "hang". With the tag removed, 0 alert words remain across 232 lines. The `fails+N` counters are absent-override probes, documented as normal in `patch_manager_mods/10_engine_optimization_research.md:222`.
- Tool routing bug: `kotor2_logs.py check` sends `k2-robustness-fixes` and `k2-texture-memory-release` to `analyze_runs`, which prints an old perf-lab summary (launch 58, 02:18, pid 312) and rejects `--new`. Verdicts above come from `log_triage.py --no-advance` on each file.
- `holster-editor_log.txt --new` returned EMPTY (cursor already past), so it was read with `--no-advance`.

## 3. preview-3d, holster-editor token (tag HSE_VIEW)
| token | CreateView to ready (s) | Destroy to done (s) | holster DTOR (s) | panel |
|---|---|---|---|---|
| 1 | 22217.715 to .978 | 151971.695 to .697 | 151971.629 | 090c4e38 |
| 2 | 152802.173 to .375 | 199505.418 to .420 | 199504.822 | 0b99ef20 |

- 2 creates, 2 destroys, gate=1 on both. Each destroy follows its holster-editor DTOR (0.07 s and 0.6 s earlier).
- No refusal lines. No leak lines.

## 4. Regression scan (button clone, column reflow)
- Editor loaded at 412.99 with button_clone=1, reflow=1, desc=1, free_labels=1, free_view=0.
- Bar rects idx 8 to 12 all ok=1.
- 4 press/release pairs: each press hitBtn=1 or hitBack=1 matches release overBtn=1 or overBack=1.
- mouse-probe and holster-editor log the same clicks within 1 ms. Y is mirrored (holster y + mouse y = 1599).
- mouse-probe: 76 CLICK rows, guard3EC=0 on all. movept_bg is non-zero on 2 clicks: 170210.9 (value 2) and 195363.4 (value 4, the holster-button click). No baseline to call this wrong.
- DEFACT x3 and WORLDCLICK branch=B x5 are world-actor clicks (0x800003b0, 0x80000022), not Options-screen clicks.
- Presses at 198095.6 and 198967.8 hit no control with the editor closed. No release line follows.

## 5. Other
- robustness "12/13 groups": movspin is default OFF (`patch_manager_mods/robustness_fixes_src/RobustnessFixes.cpp:70`), so expected.

## Open
- mouse-probe has no screen tag, so Options-specific mouse oddities cannot be isolated from logs. Need the play-test note or a screenshot for the 198095 and 198967 presses.
