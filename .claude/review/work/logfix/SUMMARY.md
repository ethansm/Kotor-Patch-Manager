# Log-hygiene batch (2026-10-13) - session summary

Request: "selfheal logfix should be next, while touching logs, look for other logging todos that might be related and could be fixed together". Then: "once all the research is complete create a detailed implementation plan in plan mode ... persist any session context or scratchpad resources before switching to plan mode".

## 1. Done and committed (0a3385e, NOT installed, NOT pushed)
- `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp`: new macro `REFUSEI` (INFO, `return false`); `REFUSE debounce member=` and the dedupe `REFUSE already queued member=` now use it (text byte-identical; `scripts/selfheal_log_check.py:61` matches `^REFUSE` on text, not level). `gateLog`: past `GateLogCap` (64) it keeps a 30 s heartbeat `GATE late open=%d why=%s hud=%08x f48=%08x top=%08x topf48=%08x` via `KP_LOG_EVERY(chHud, 30000, ...)`; dedupe compare moved into `const bool same`.
- `patch_manager_mods/action_hotbar_src/ActionHotbar.cpp`: same `gateLog` change, channel `chGate`.
- Worker (Haiku) tests: `run_tests.py companion_selfheal` 1/1, `run_tests.py selfheal` 4/4, `run_tests.py hotbar_log_test hotbar_logic_test` 2/2, `build_check.py --mod companion_selfheal_src --mod action_hotbar_src` PASS=2 FAIL=0. ActionHotbar.cpp has no unit suite (only the build gate covers it). Opus reviewed the diff: matches the spec exactly. No checker parses `GATE` lines (rg over scripts/*check*.py, log_triage.py: none), so `GATE late` is safe.
- Versions unchanged (selfheal 0.8.2-dev, action-hotbar 0.1.3-dev); bump decision is open (see plan).

## 2. Pre-install gate: `scripts/pre_install_gate.sh --mod companion_selfheal_src --mod action_hotbar_src` = GATE FAIL (262 passed, 5 failed). Attribution (Verify-After-Fix invariant):
| Failing suite | Cause | Mine? |
|---|---|---|
| suite:check-raw-addresses | `patch_manager_mods/voice_probe_src/VoiceProbe.cpp:5 0x0068EA70` not in GameAddr.h/raw_addresses.toml (13 FAIL total, peer voice work; `tests/static/raw_addresses.toml` is peer-dirty) | no |
| suite:check-hook-spans | `area-hook-probe` detours overlap `k1-spawn-probe` at 0x00533830 and 0x0055A460 (peer probes) | no |
| py:patch_manager_mods/kvoice/test_kvoice_lip.py | `kpve3c.lip: first key time 0.25 != 0` (1 of 679 vanilla lips; kvoice, peer) | no |
| py:scripts/test_agent_contract.py | `TestDocsDrift.test_files_identical`: `AGENTS.md` and `CLAUDE.md` "Subagent Contracts" sections differ (CLAUDE.md has extra "Backend policy" + "Wave recipe" lines; both files git-clean, so drift is committed) | no |
| py:scripts/test_compat_breakage.py | test_1_break_saferead_clamp: "Expected BREAKING message" absent, 15/16 FAIL. CONTROL RUN: same failure with my two files reverted to 0a3385e^ -> not caused by this batch. Root cause not found yet | no (cause open) |
Kept raw: the gate must be GATE PASS before `kpm_hotswap.py --apply`; resolution paths are in the plan.

## 3. Recon reports in this dir
R1 = selfheal WARN/ERR site audit (33 WARN-capable sites: 13 expected, 20 genuine-class; auto refusals capped at 1 per 6 s per member via AutoCooldownMs=6000; no per-frame WARN). R3 = GATE cap mechanics. R4 = queue-hud audit. R2 (below) = todo list from handoffs (agent could not write; list saved from its reply).

## 4. Deferred logging todos (not in 0a3385e), with exact code sites
Size S = few-line edit, M = few lines + test, L = needs hook/RE.
1. **queue-hud VIS cap shared by all labels** (S). `QueueHud.cpp:388-391` `visLog`: `static int n = 0; if (!g_logOn || n > 64) return; if (n == 64) { ++n; KP_LOG(chMain, "VIS cap reached"); return; } ++n;` - VIS hit the cap at T=702949 in play, silencing all later VIS (cancel0/1/2 + others). Fix: per-label counters or higher cap.
2. **queue-hud ROUND/orphan-round origin** (M). `B` flag is derived by `scripts/queue_hud_log_check.py:47,279` from QJ lines (`QJ ... ours=` only within 2000 ms of our last action, `QueueHud.cpp:1531,1589`), not emitted by roundWatchTick. ROUND line = `qh::fmtRound(...)` `QueueHudLogic.h:1192` (`ROUND m= n= subs= cur= ne= types=`), emitted `QueueHud.cpp:1183-1187` under `g_cfg.traceQueue`. Proposed: add `p0=<param0 of tail entry>` and `lc=<ms since our last cancel>` via a new `g_lastCancelTick[3]` set where `QueueHud.cpp:818` sets `g_lastOurs[k].tag = "cancel"` (tick already stored in `g_lastOurs[k].tick`); checker tolerates extra keys (`queue_hud_log_check.py:188-199`); check FIELD_RE needs. True origin of the re-pushed 0x3F needs a hook (candidate UseItem 0x5648D0, `patch_manager_mods/27_companion_selfheal.md:96`) = L, out of scope.
3. **queue-hud `blank icon` WARN** (S). `QueueHud.cpp:200`: `if (firstTime(seenBlank, nBlank, 16, key)) QLOGV(chIcon, KPLOG_WARN, "blank icon: type=0x%02X param0=%08x node=%08x key=%08x", ...)`. R4: not by-design in source (buildSlots `QueueHudLogic.h:52-53,102-103` only pushes 0x3F for valid subtypes); engine-expected question open (needs RE of 0x74E0B0 or log harvest). Do not downgrade until answered.
4. **combat-hotbar VIS budget** (S+M). `CombatHotbar.cpp:296-303`: `int g_visCount = 0; bool g_visCapped = false; bool visBudget() { if (g_visCount < 64) { ++g_visCount; return true; } if (!g_visCapped) { g_visCapped = true; KP_LOGV(chMain, KPLOG_INFO, "VIS cap reached"); } return false; }` shared by group and buttons; `visReset()` does not reset the count. Probe gap: pill visibility has no VIS line (only change-gated `pills: visible=` at :597), and the `shown=1` group line (:671) is lost to the cap. 26/26 pill invokes restored=1 in play, so function is fine.
5. **hud-host HUD48 cap** (S). `hud_host_src/HudHostProvider.cpp:110-126` `constexpr int Cap = 64; if (lines > Cap) return; ... if (lines == Cap) {++lines; KP_LOG(chCore,"HUD48 cap reached"); return;}` shared between `HUD48 hud=` changes and `HUD48 frame=` dumps.
6. **buffhud caps** (S). `buff_duration_hud_src/BuffDurationHUD.cpp:82-89` `VisMaxLines = 64` per process (`logf("VIS cap reached")`), `DBGMISS` cap 40 (:354); `DBGMISS` is marked "Remove after triage" (:336-338,357,371): KEEP until the 0.2.1 Slow replay confirms (0.2.1 is live c65dc679; the 2026-10-12 session ran 0.2.0). `logf` writes raw lines with no T=/channel stamp and only when `buffhud_debug.txt` exists (:56-62,516) (M, optional).
7. **action-hotbar `invalid object id ffffffff`** (S). INFO line at `ActionHotbar.cpp:405` matches the log_triage ALERT regex `\binvalid\b` (`scripts/log_triage.py:71`); transient (T=550576 / T=604581 at area load), not stuck. Either reword the message or add the case to the triage allowlist.
8. **queue-hud strip `k=` edge lines uncapped** (S, verify first). `QueueHud.cpp:621-627`: 215 flips in 318 s during the Down flicker (pre-0.4.1). Re-check after a play session with the 0.4.1 Down gate before editing.
9. **docs**: `.claude/handoffs/27_companion_selfheal.handoff.md:41` says DISC cap 48 but code is 96 (`CompanionSelfHeal.cpp:595,609`; PILL is 48). That handoff is peer-dirty: edit only after the peer commits.
10. selfheal other WARNs (R1): `REFUSE canuse=0` (line ~384, genuine-class: pick filter did not exclude an item the engine refuses; persistent case is the realistic auto spam path, max ~10/min/member) and `REFUSE not commandable (+0xEC=0)` (~369, probably expected: auto skips that state before useNow) - leave at WARN for now.

## 5. Constraints and conflicts
- queue-hud: a peer session (doc 33 FIFO work) has UNCOMMITTED edits in `queue_hud_src/QueueHud.cpp` and `QueueHudLogic.h` (queueCountOf `haveCur`, `shouldDispatchNext`, `decideAction(..., bool haveCur)`). Do NOT edit or commit those two files until it commits; then re-run `git status` and rebase the log changes on top.
- Other mods' MIRROR rows in `kpm_status` (cmdq-probe, gui-input, inventory-3d-viewport, inventory-sort-filter, mouse-probe, npc-diversity) belong to peers.
- Install needs the user's GO with the game closed; never two installers at once.
- Memory rules: `feedback-commit-explicit-paths` (git commit -- <paths>), `feedback-patch-versioning` (0.x.y), `feedback-haiku-subagents-first`.

## 6. Agents used this batch
R1..R4 recon (general-purpose haiku; R1 71k..R4 89k tokens, R2 141k, R3 71k; R2 could not write its file), 1 Haiku worker (78k tokens, 26 tool uses, STATUS PASS). Opus: spec, diff review, gate triage.
