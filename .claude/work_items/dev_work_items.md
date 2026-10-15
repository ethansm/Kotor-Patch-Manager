# Dev work-item list (priority order)

Built 2026-10-12 from the 415 open code ToDos in `ToDo_master_list.md` by a Haiku swarm. 100 work items. Each WI is one Haiku-worker task (at most 4 write files, one patch). Detail lives in `.claude/work_items/WI-NN.md`; do not open those yourself.

## How a Sonnet orchestrator uses this

1. Read only this file. Pick the first unchecked row whose state is `ready` or `verify-only` and whose `after` WI is checked.
2. `python3 scripts/agent_contract.py snapshot --out .claude/waves/<id>/contract_baseline.tsv`, then spawn ONE Haiku worker (`model: haiku`). Brief = `Contract: worker` + `Write Scope:` = the row's files + `python3 scripts/agent_contract.py print worker` + "Read .claude/work_items/WI-NN.md and execute its Steps". Do not read the WI file yourself.
3. Check wave: one Haiku recon agent runs the WI's Verify commands (scoped `run_tests.py`, `build_check.py`) and replies in the 5-line recon contract. Then `agent_contract.py check --scope <files> <reply>`.
4. Docs wave: one Haiku worker updates the plan rows in the WI's Source rows (`plan_manager.py mark-done`, never sed the master plan) and the CHANGELOG named in its Docs section.
5. Tick the row here (`[ ]` to `[x]`) with Edit. Rows in state `hold` need a user decision, GO, install or play test first: list them to the user, do not start them. Never install or play-test; those are the user's.

Orchestrator cost per WI: about 1.5k tokens (one spawn prompt and three 5-line replies). Priority: P0 = rank 1-12, P1 = 13-35, P2 = 36-65, P3 = rest. Score = 0.45 impact/risk + 0.35 unblocking + 0.20 quick-win, hold -3, verify-only +0.5.

| done | # | WI | P | state | size | after | title | rows | files | why |
|---|---|---|---|---|---|---|---|---|---|---|
| [x] | 1 | WI-48 | P0 | verify-only | S | - | Bug board: verify BG19 and BG13; defer BG24 and BG20 | 5 | 2 | Bug board: verify BG19 and BG13, defer BG24; 5 open. |
| [x] | 2 | WI-33 | P0 | verify-only | S | - | Hotbar save-events migration verify (row 4.1a) | 1 | 4 | Hotbar save-events migration verify; blocked on owner GO. |
| [x] | 3 | WI-10 | P0 | ready | M | - | Force Power Hotbar polish and config round-trip | 7 | 4 | Force Power Hotbar polish and config round-trip; 6 open, ready. |
| [x] | 4 | WI-01 | P0 | verify-only | S | - | Level-up override: KpLog verify and BG20 close | 1 | 2 | Level-up verify plus bug-board BG20 close; verify-only, 1 open. |
| [x] | 5 | WI-10b | P0 | ready | S | WI-10 | Force Power Hotbar EKC hook move and arrow art (split from WI-10) | 3 | 2 | Hotbar EKC hook move and arrow art; 3 open, ready. |
| [x] | 6 | WI-03 | P0 | hold | S | - | Inventory 3D viewport hook, manifest and bodyvar rows | 3 | 3 | Inventory 3D viewport hook and manifest; feature rows, 2 open. |
| [ ] | 7 | WI-41 | P0 | ready | M | - | Queue HUD open ToDos: dispatch gate, FIFO, flicker | 9 | 3 | Queue HUD dispatch, FIFO and flicker bugs; active plan, 6 open. |
| [x] | 8 | WI-18 | P0 | ready | S | - | Inventory sort filter logging on KpLog.h (C180) | 1 | 1 | Inventory sort filter logging on KpLog.h; logging only. |
| [x] | 9 | WI-06 | P0 | ready | L | - | Buff duration HUD open ToDos | 10 | 2 | Buff duration HUD: 8 open ToDos, 1 blocked; large and ready. |
| [x] | 10 | WI-42 | P0 | ready | S | - | Companion self-heal: IniPath check and BG18 stale HP | 6 | 1 | Companion self-heal IniPath check and stale BG18 row; bug board. |
| [x] | 11 | WI-15 | P0 | verify-only | S | - | Level-up F10 over-cap hold verify | 1 | 0 | Level-up F10 over-cap hold verify; single open row. |
| [x] | 12 | WI-11 | P0 | ready | L | WI-10 | Force Power Hotbar config and statics | 9 | 3 | Force Power Hotbar config and statics; large, 8 open, ready. |
| [ ] | 13 | WI-05 | P1 | ready | S | WI-03 | Inventory 3D viewport no-preview fallback and build-hook ver | 4 | 2 | No-preview fallback and build rows; 1 open, 2 blocked. |
| [ ] | 14 | WI-35 | P1 | ready | S | - | Equip menu-3d stale-claim cleanup (C206, C208) | 2 | 3 | Equip menu-3d stale-claim cleanup; ready, 2 doc rows. |
| [ ] | 15 | WI-07 | P1 | ready | M | WI-06 | Buff HUD shared helpers and accessor | 5 | 4 | Buff HUD shared helpers and accessor; ready, after WI-06. |
| [ ] | 16 | WI-04 | P1 | verify-only | M | - | Equipment 3D character shared-code migration | 9 | 4 | Equipment 3D shared-code migration; refactor, owner GO needed. |
| [ ] | 17 | WI-20b | P1 | verify-only | S | - | Robustness TD and GUIID fixes, verify only (C115, C116) | 2 | 1 | Robustness TD and GUIID fixes, verify only; target file present. |
| [ ] | 18 | WI-38 | P1 | ready | L | - | Companion self-heal open rows: verify and build | 10 | 3 | Companion self-heal open rows: verify and build; 7 open, ready. |
| [ ] | 19 | WI-02 | P1 | ready | M | - | KPM tooling H1-H6 build batch | 5 | 4 | KPM tooling batch; tooling, not game behavior. |
| [ ] | 20 | WI-12 | P1 | ready | M | WI-10 | Force Power Hotbar arrow art and setFont check | 4 | 4 | Hotbar arrow art and setFont check; arrow spec unknown. |
| [ ] | 21 | WI-09 | P1 | ready | S | - | Ultrawide GUI and FullPartySelect follow-ups | 3 | 1 | Ultrawide GUI and FullPartySelect follow-ups; 1 open, ready. |
| [ ] | 22 | WI-13 | P1 | verify-only | M | - | Level-up override verify and Configure nudge check | 10 | 0 | Level-up override verify and Configure nudge check; 9 open. |
| [ ] | 23 | WI-50 | P1 | ready | S | - | deep-haiku-review H1/H2 backoff, verify scripts and ladder c | 4 | 0 | deep-haiku-review H1/H2 backoff and verify scripts; tooling. |
| [ ] | 24 | WI-44 | P1 | verify-only | S | - | NPC diversity selector bias and float guard | 1 | 0 | NPC diversity selector bias and float guard; 1 verify open. |
| [ ] | 25 | WI-18c | P1 | ready | M | WI-18 | KpLog remainder: probe, companion AI, parser, design rows (C | 6 | 3 | KpLog remainder for probe, companion AI and parser rows; mostly parked |
| [ ] | 26 | WI-14 | P1 | verify-only | L | - | Level-up override F-row verify and deferred items | 10 | 0 | Level-up F-row verify and deferred items; large, 8 open. |
| [ ] | 27 | WI-30 | P1 | ready | L | - | Letterbox fix KpLog verify and analyzer cleanup | 9 | 4 | Letterbox fix KpLog verify and analyzer cleanup; 9 open, ready. |
| [ ] | 28 | WI-46 | P1 | ready | M | - | Agent tooling hardening: kpm_hotswap, kpm_status, gate, raw  | 7 | 4 | Agent tooling hardening, incl. kpm_hotswap; tooling, 2 blocked. |
| [ ] | 29 | WI-02c | P1 | ready | S | - | Smoke harness include normalization (H8) | 0 | 1 | Smoke harness include normalization; test tooling, no blockers. |
| [ ] | 30 | WI-23 | P1 | ready | M | - | Ranged AI a1_retarget strict range gate | 1 | 2 | Ranged AI a1_retarget strict range gate; 1 open, line numbers unknown. |
| [ ] | 31 | WI-60 | P1 | verify-only | S | - | Phase3d scope fix and sandbox HOME verify | 4 | 0 | Phase3d scope fix and sandbox HOME verify; tooling, 3 blocked. |
| [ ] | 32 | WI-47 | P1 | ready | S | WI-46 | KPM production filter, ledger lock, GPG flag | 3 | 2 | KPM production filter, ledger lock and GPG flag; ready, tooling. |
| [ ] | 33 | WI-08 | P1 | ready | M | - | Shield bar tooltip, probe, and logging ToDos | 8 | 2 | Shield bar tooltip, probe and logging ToDos; 8 open, ready. |
| [ ] | 34 | WI-43 | P1 | ready | S | - | NPC diversity dispatcher open source fixes | 10 | 1 | NPC diversity dispatcher open source fixes; 10 open, ready. |
| [ ] | 35 | WI-36 | P1 | ready | L | - | PC level-up in-panel Filter button | 1 | 2 | PC level-up in-panel Filter button; new feature, ready. |
| [ ] | 36 | WI-02b | P2 | ready | M | WI-02,WI-02c | KPM tooling H7, H10, H9 new scripts | 3 | 3 | New KPM tooling scripts; tooling only, after WI-02. |
| [ ] | 37 | WI-29 | P2 | verify-only | M | - | Portrait-slots verify and companion-AI KpLog scope | 2 | 0 | Portrait-slots verify and companion-AI KpLog scope; verify only. |
| [ ] | 38 | WI-21 | P2 | ready | M | WI-18 | Inventory sort name tie-break (C132); rest deferred | 6 | 2 | Inventory sort name tie-break; 1 open, 5 parked. |
| [ ] | 39 | WI-31 | P2 | ready | M | - | Portrait-slots split v1 decision comment | 4 | 2 | Portrait-slots split v1 decision comment; 2 open, 2 parked. |
| [ ] | 40 | WI-50b | P2 | verify-only | S | WI-50 | deep-haiku-review H3/H4 verify, H6 and H9 decisions | 4 | 0 | deep-haiku-review H3/H4 verify and H6/H9 decision; tooling. |
| [ ] | 41 | WI-51 | P2 | verify-only | M | - | deep-haiku-review ladder verify (L2-L4, P2b-a/b, style runne | 9 | 0 | deep-haiku-review ladder verify; tooling, spend GO excluded. |
| [ ] | 42 | WI-64 | P2 | ready | S | - | master review root .env exclusion check | 1 | 0 | Master review .env exclusion check; ready, 1 open. |
| [ ] | 43 | WI-34 | P2 | verify-only | L | - | Verify k2.hud@1 provider and consumers | 1 | 2 | k2.hud provider verify; consumer wiring still open. |
| [ ] | 44 | WI-68 | P2 | verify-only | S | - | Argv probe per-case tree check | 2 | 0 | Argv probe per-case tree check; verify, out-of-repo engine half. |
| [ ] | 45 | WI-19 | P2 | ready | M | - | Perf-lab counters verify and M1 TGA/cube build | 10 | 1 | Perf-lab counters verify and M1 TGA/cube build; research tooling. |
| [ ] | 46 | WI-46b | P2 | ready | M | WI-46 | Agent tooling hardening: ship pipeline modules | 3 | 3 | Agent tooling ship pipeline modules; tooling, 1 blocked. |
| [ ] | 47 | WI-59 | P2 | verify-only | M | - | Deep-haiku-review fix ports: verify and close | 4 | 0 | deep-haiku-review fix ports verify and close; 4 blocked. |
| [ ] | 48 | WI-40b | P2 | ready | S | - | Cmdq LAT and hold-reason parser | 1 | 1 | Cmdq LAT and hold-reason parser; probe-log tooling, ready. |
| [ ] | 49 | WI-24 | P2 | ready | L | - | Active effects panel: logging and shared-helper migration | 8 | 1 | Active effects panel; 7 of 8 rows blocked on unbuilt helpers. |
| [ ] | 50 | WI-49 | P2 | ready | S | - | K1 asset import: pykotor node-order fix (T12c) | 5 | 1 | K1 asset import pykotor node-order fix; 4 open, 1 parked. |
| [ ] | 51 | WI-25 | P2 | verify-only | M | - | 2DA limit verify and wrap width probe | 2 | 0 | 2DA limit verify and wrap width probe; engine bug candidates. |
| [ ] | 52 | WI-66b | P2 | ready | S | - | Bug-investigate per-stage reliability and playtest bundle ch | 2 | 1 | Bug-investigate per-stage reliability and playtest; tooling. |
| [ ] | 53 | WI-51b | P2 | ready | M | WI-51 | deep-haiku-review ladder wiring: refute-audit and fix (C296) | 1 | 0 | deep-haiku-review ladder wiring; tooling, after WI-51. |
| [ ] | 54 | WI-37 | P2 | ready | M | - | Forced-state expected decisions into saves.toml | 2 | 2 | Forced-state expected decisions into saves.toml; test tooling. |
| [ ] | 55 | WI-56 | P2 | verify-only | M | WI-55 | Verify deep-haiku-review effort plumbing E1-E3 (gaps reporte | 3 | 0 | deep-haiku-review effort plumbing verify; tooling, gaps reported. |
| [ ] | 56 | WI-54 | P2 | ready | M | WI-50 | Deep-haiku-review H2 verify-script backoff, lookup order and | 6 | 0 | deep-haiku-review H2 verify-script backoff and lookup; tooling. |
| [ ] | 57 | WI-27 | P2 | ready | M | - | Mouse camera probe logging and hooks | 10 | 2 | Mouse camera probe logging and hooks; 7 open, 1 awaiting user. |
| [ ] | 58 | WI-63 | P2 | ready | S | - | fix_merge safe_rel for the integrated patch path | 4 | 1 | fix_merge safe_rel for integrated patch path; path safety, 3 blocked. |
| [ ] | 59 | WI-45 | P2 | verify-only | S | - | NPC add POC build verify and Wave4 gate | 9 | 0 | NPC add POC build verify and Wave4 gate; 4 blocked. |
| [ ] | 60 | WI-60b | P2 | ready | S | WI-58,WI-60 | Phase3d effort pins in p2 profile | 2 | 1 | Phase3d effort pins in p2 profile; tooling, 2 blocked. |
| [ ] | 61 | WI-66 | P2 | ready | M | - | Bug-investigate null verify gate and style logging | 5 | 2 | Bug-investigate null gate and style logging; tooling, 1 blocked. |
| [ ] | 62 | WI-59b | P2 | ready | S | WI-59 | Deep-haiku-review refute stub and checks (R2) | 3 | 0 | deep-haiku-review refute stub and checks; tooling, 2 blocked. |
| [ ] | 63 | WI-68b | P2 | ready | S | - | Mod scaffold kind selector | 1 | 1 | Mod scaffold kind selector; ready, tooling nice-to-have. |
| [ ] | 64 | WI-61 | P2 | verify-only | M | - | deep-haiku-review skill tooling verify (G2-G9) | 10 | 0 | deep-haiku-review skill tooling verify G2-G9; 9 open. |
| [ ] | 65 | WI-68c | P2 | hold | S | - | Docs search cache refresh | 1 | 1 | Docs search cache refresh; ready, tooling nice-to-have. |
| [ ] | 66 | WI-17 | P3 | hold | M | - | A-id registry and enginestate provider (load-hang probe) | 3 | 3 | A-id registry and enginestate provider; hold, owner GO needed. |
| [ ] | 67 | WI-65 | P3 | ready | M | - | Swarm sandbox, scrub and runner safety fixes | 10 | 0 | Swarm sandbox, scrub and runner safety fixes; out-of-repo, 9 open. |
| [ ] | 68 | WI-05b | P3 | hold | S | - | Shared providers: GuiInput, GuiKit, IniKit (A1, B2, B3) | 4 | 3 | Shared GuiInput, GuiKit, IniKit providers; all rows owner-gated. |
| [ ] | 69 | WI-62 | P3 | ready | M | - | deep-haiku-review engine verify, SKILL.md router, R5 check | 6 | 0 | deep-haiku-review engine verify and SKILL.md router; tooling. |
| [ ] | 70 | WI-53 | P3 | verify-only | S | - | Verify deep-haiku-review fix stages F-a to F-e (S9-S13 prese | 6 | 0 | deep-haiku-review fix stages F-a to F-e verify; all 6 blocked. |
| [ ] | 71 | WI-59c | P3 | verify-only | S | - | Deep-haiku-review replay and lens profiles: verify | 3 | 4 | deep-haiku-review replay and lens profiles verify; all blocked. |
| [ ] | 72 | WI-03b | P3 | hold | M | WI-03 | Inventory 3D viewport GUI, logging and build rows | 7 | 4 | Inventory 3D viewport GUI, logging and build rows; on hold. |
| [ ] | 73 | WI-67 | P3 | ready | M | - | Swarm result writes, budget and cache keys | 10 | 0 | Swarm result writes, budget and cache keys; out-of-repo, 5 blocked. |
| [ ] | 74 | WI-17b | P3 | hold | M | WI-17 | Common code-patch and INI helpers (C102, C106, C123) | 3 | 3 | Common code-patch and INI helpers; hold, owner GO needed. |
| [ ] | 75 | WI-40c | P3 | hold | S | WI-41 | Queue panel shows when paused with 0x3F (held for GO) | 1 | 2 | Queue panel shows when paused with 0x3F; held for user GO. |
| [ ] | 76 | WI-35b | P3 | hold | M | WI-35 | Equip menu-3d SafeRead, SceneView and GuiKit label migration | 3 | 3 | Equip menu-3d SafeRead, SceneView and GuiKit label; hold, 2 blocked. |
| [ ] | 77 | WI-55 | P3 | ready | L | - | Deep-haiku-review ladder stages and wiring | 4 | 0 | deep-haiku-review ladder stages and wiring; large, 3 blocked. |
| [ ] | 78 | WI-57 | P3 | verify-only | L | - | Verify deep-haiku-review fix stages in bundle | 10 | 0 | deep-haiku-review fix stages in bundle; all 10 rows blocked. |
| [ ] | 79 | WI-33b | P3 | hold | M | WI-33 | Hotbar portrait target and guiinput wheel fallback | 3 | 3 | Hotbar portrait target and wheel fallback; hold, user answer needed. |
| [ ] | 80 | WI-52 | P3 | ready | S | - | deep-haiku-review SKILL.md and scaffold verify | 4 | 0 | deep-haiku-review SKILL.md and scaffold verify; all 4 blocked. |
| [ ] | 81 | WI-16 | P3 | hold | S | - | Load-hang probe gated rows | 4 | 1 | Load-hang probe rows; gated, 2 blocked, nothing open. |
| [ ] | 82 | WI-35c | P3 | hold | S | WI-35b | Equip 21 px tooltip onto TextFit and GOG SceneView fields | 2 | 3 | Equip 21 px tooltip onto TextFit; hold, 2 blocked. |
| [ ] | 83 | WI-05c | P3 | hold | S | - | Shared providers: SceneView, PanelSlots, Scene3D (B4, B8) | 2 | 3 | Shared SceneView, PanelSlots, Scene3D providers; owner-gated, new file |
| [ ] | 84 | WI-16b | P3 | hold | S | - | Letterbox IsBadReadPtr to probeRead (C098 letterbox half) | 1 | 1 | Letterbox IsBadReadPtr swap to probeRead; parked, on hold. |
| [ ] | 85 | WI-01b | P3 | hold | S | - | Bug-board triage (C010) | 1 | 0 | Bug-board triage; blocked on a user decision for C010. |
| [ ] | 86 | WI-16c | P3 | hold | S | - | BuffDurationHUD slot age guard (C098 buff half) | 1 | 1 | BuffDurationHUD slot age guard; parked, needs design. |
| [ ] | 87 | WI-18b | P3 | hold | M | WI-18,WI-19 | Perf-lab logging on KpLog.h channels (C122) | 1 | 1 | Perf-lab logging on KpLog.h channels; logging, on hold. |
| [ ] | 88 | WI-26b | P3 | hold | S | WI-30 | Dialogue reply-panel stretch (C157) | 1 | 1 | Dialogue reply-panel stretch; parked, doc marks it out of scope. |
| [ ] | 89 | WI-33c | P3 | hold | S | WI-35c | Hotbar item-target move and FPH item table handoff | 2 | 3 | Hotbar item-target move and FPH item handoff; 2 parked. |
| [ ] | 90 | WI-26 | P3 | hold | S | - | Wrap last-line drop at 5120+ width (C156) | 1 | 1 | Wrap drops last line at 5120+ width; parked, user GO needed. |
| [ ] | 91 | WI-53b | P3 | hold | M | - | Build deep-haiku-review fix merge gate and gate wiring (F-f, | 2 | 0 | deep-haiku-review fix merge gate; hold until user GO. |
| [ ] | 92 | WI-20 | P3 | hold | S | - | TX cave pair (texture memory release) gates | 2 | 1 | Texture memory release pair; both rows blocked on S3 saves gate. |
| [ ] | 93 | WI-58 | P3 | hold | S | - | Run p2 profile init and free prep | 1 | 1 | Run p2 profile init and free prep; gated on user go. |
| [ ] | 94 | WI-28 | P3 | hold | S | - | Hotbar SafeRead swap verify and close | 1 | 0 | Hotbar SafeRead swap verify; parked, owner has uncommitted work. |
| [ ] | 95 | WI-53c | P3 | hold | L | WI-53b | Port fix half S9-S14 and canary bwrap settings fix (P5, bug  | 2 | 0 | deep-haiku-review fix half S9-S14 port; large, hold. |
| [ ] | 96 | WI-40 | P3 | hold | M | - | Paused command queue batch-2 probe (held for GO) | 2 | 2 | Paused command queue batch-2 probe; held for user GO. |
| [ ] | 97 | WI-39 | P3 | hold | L | - | Point-and-click movement mod, held for GO | 4 | 3 | Point-and-click movement mod; all rows held for user GO. |
| [ ] | 98 | WI-32 | P3 | hold | M | - | Compat-check --sync contract-test gate | 1 | 1 | Compat-check --sync contract-test gate; hold, suite names unknown. |
| [ ] | 99 | WI-20c | P3 | hold | S | - | RB keep-nodes patch at 0x4957AA (target unknown, C112) | 1 | 0 | RB keep-nodes patch at 0x4957AA; target file unknown, research. |
| [ ] | 100 | WI-22 | P3 | hold | L | - | Empty-container indicators (Mod 7, cancelled) | 7 | 1 | Empty-container indicators; cancelled, all 7 rows parked. |
| [ ] | 101 | WI-69 | P3 | hold | S | - | Add plan ids to Source rows that have none [audit:plan-row-hygiene] | 0 | 0 | 5 closed items left plan rows unresolved |
| [ ] | 102 | WI-70 | P3 | hold | S | - | Action Hotbar A11: extract picker tab filter/Same dedupe and panel box math | 0 | 3 | WI-10 leftover step 8 (optional M item) skipped; move ActionHotbar.cpp |
| [ ] | 103 | WI-71 | P3 | hold | S | - | Inventory 3D viewport v2: PC-own-body armor/robe preview via 008cd060 (Branch B) | 0 | 0 | v1 uses the Workbench fixed row 137 and cat==4 (bodyvar folded in by g |
| [ ] | 104 | WI-72 | P3 | hold | S | - | analyze_play_logs.py: strip KpLog prefix for inventory_sort_filter_log.txt | 0 | 0 | KpLog fallback prefixes each line '<ms32> <patch>/<ch> ', breaking ^-a |
| [ ] | 105 | WI-73 | P3 | hold | S | - | BG26 DXT1 splice skips RGB edits on parents with c0<=c1 since encode_rgb is always 4-colou | 0 | 2 | wrong-behaviour/med: Guard is deliberate alpha protection but silently |
| [ ] | 106 | WI-74 | P3 | ready | S | - | BG27 engineNe counts resting 0x3D/bare 0x3F, forcing buffering and permanent engine_busy | 0 | 3 | wrong-behaviour/med: engineNe counts every entry; ActQueue.h documents |
| [ ] | 107 | WI-75 | P3 | ready | S | - | BG28 keyFor(0) returns slot0 ignoring tag, so settings follow party position not creature  | 0 | 1 | wrong-behaviour/med: keyFor(0) returns slot0 even though c0.tag is rea |
| [x] | 108 | WI-76 | P3 | ready | S | - | BG29 imageFor pools failed (null) lookups so hudkit::image retry never reloads | 0 | 4 | wrong-behaviour/med: imageFor pools null (line 299) so lazy retries in |
| [ ] | 109 | WI-77 | P3 | ready | S | - | BG30 CrTrack caches raw creature pointer; stale spinning state and dangling cr after death | 0 | 1 | log-only/low: e1Evaluate only runs from AiTraceAttackEntry, so spinnin |
| [ ] | 110 | WI-78 | P3 | ready | S | - | BG31 g_lastCtrlArea overwritten with 0 during transition so PARTY why=area misses | 0 | 1 | log-only/low: Line 589 overwrites the baseline with 0 on a transient F |
| [ ] | 111 | WI-79 | P3 | ready | S | - | BG32 Unreadable creature logs false old=0 edge=1 combat transition | 0 | 1 | log-only/low: rdU32 returns 0 on saferead failure, so an unreadable cr |
| [ ] | 112 | WI-80 | P3 | ready | S | - | BG33 Stale tooltip kept when new slot name read is empty | 0 | 3 | wrong-behaviour/low: tipKey and tipSet update even when setNative is s |
| [ ] | 113 | WI-81 | P3 | ready | S | - | BG34 Vanilla tint read immediately overwritten; failure branches are dead code | 0 | 1 | log-only/low: tOk and aOk are forced true right after the reads, so th |
| [ ] | 114 | WI-82 | P3 | ready | S | - | BG35 Transient g_combatUnreadable overwrites g_prevCombat, firing false combat rising edge | 0 | 1 | wrong-behaviour/low: Unreadable frame stores prev=false so next readab |
| [ ] | 115 | WI-83 | P3 | ready | S | - | BG36 Pending, auto and cooldown state indexed by party slot not creature; leader swap misa | 0 | 1 | wrong-behaviour/low: Pending/auto/cooldown state is slot-indexed with  |
| [ ] | 116 | WI-84 | P3 | ready | S | - | BG37 LMB on companion button at FullHp/Pending skips popover close and noneed log | 0 | 1 | log-only/low: Companion LMB at FullHp/Pending falls through silently a |
| [x] | 117 | WI-85 | P3 | ready | S | - | BG38 heal() logs giving up on the 20th refill even if that fill succeeded | 0 | 2 | log-only/low: heals counts attempts and ignores fill() result; line 15 |
| [x] | 118 | WI-86 | P3 | ready | S | - | BG39 Wheel callback passes engine screen size as client rect size to clientToEngineF | 0 | 3 | wrong-behaviour/low: cw/ch are the provider's engine size (client rect |
| [!] | 119 | WI-87 | P3 | ready | S | - | BG40 Allocated label leaked on createLabelFromTemplate and bindByTag failure paths | 0 | 2 | wrong-behaviour/low: Leak real in createLabelFromTemplate and bindByTa |
| [x] | 120 | WI-88 | P3 | ready | S | - | BG41 pendingDone2 now==0 sentinel branch untested | 0 | 2 | wrong-behaviour/low: Untested sentinel branch is also buggy: storing 1 |
| [ ] | 121 | WI-89 | P3 | hold | S | - | Gemini test/tooling gaps: hotbar (07_force_power_hotbar.md, keybif_test.cpp, twoda_test.cp | 0 | 3 | 5 validated test-gap/tooling fids: HB-1b-01: -; HB-1d-03: keybif_test. |
| [ ] | 122 | WI-90 | P3 | hold | S | - | Gemini test/tooling gaps: k1 (k1_port_ingest.py, k1_port_probe.py, k1_port_validate.py, te | 0 | 4 | 4 validated test-gap/tooling fids: K1-5b-02: test_k1_port_ingest.py: p |
| [ ] | 123 | WI-91 | P3 | hold | S | - | Gemini test/tooling gaps: k1 (test_k1_port_char_build.py, test_k1_port_cr_extract.py, test | 0 | 4 | 4 validated test-gap/tooling fids: K1-5d-01: test_k1_port_char_build.p |
| [ ] | 124 | WI-92 | P3 | hold | S | - | Gemini test/tooling gaps: k1 (test_k1_port_tex_audit.py, test_k1_port_tex_dxt.py, test_k1_ | 0 | 4 | 4 validated test-gap/tooling fids: K1-5c-06: test_k1_port_tex_audit.py |
| [ ] | 125 | WI-93 | P3 | hold | S | - | Gemini test/tooling gaps: npc (nw_registry_test.cpp) | 0 | 1 | 1 validated test-gap/tooling fids: HN-6n-07: nw_registry_test.cpp: cal |
| [ ] | 126 | WI-94 | P3 | hold | S | - | Gemini test/tooling gaps: perf-lab (analyze_runs.py, ExportCallGraph.java, NewNullCheck.ja | 0 | 4 | 5 validated test-gap/tooling fids: PL-4d-03: test_analyze_runs.py: leg |
| [ ] | 127 | WI-95 | P3 | hold | S | - | Gemini test/tooling gaps: perf-lab (smoke_perflab3.cpp, hookbytes.py, persist_findings.py) | 0 | 3 | 3 validated test-gap/tooling fids: PL-4b-01: smoke_perflab3.cpp M5 che |
| [ ] | 128 | WI-96 | P3 | hold | S | - | Gemini test/tooling gaps: queue-hud (QueueHud.cpp, queue_hud_logic_test.cpp) | 0 | 2 | 4 validated test-gap/tooling fids: QH-2a-09: -; QH-2b-01: queue_hud_lo |
| [x] | 129 | WI-97 | P3 | ready | S | - | Gemini test/tooling gaps: shared (actqueue_test.cpp, effects_test.cpp, embtex_test.cpp, fi | 0 | 4 | 4 validated test-gap/tooling fids: SH-3c-04: actqueue_test.cpp: feed 2 |
| [x] | 130 | WI-98 | P3 | ready | S | - | Gemini test/tooling gaps: shared (guikit_test.cpp, guitip_test.cpp, hudkit_test.cpp, item_ | 0 | 4 | 4 validated test-gap/tooling fids: SH-3b-01: guikit_test.cpp: setAlpha |
| [x] | 131 | WI-99 | P3 | ready | S | - | Gemini test/tooling gaps: shared (k2addr_test.cpp, party_test.cpp, sceneview_test.cpp, tex | 0 | 4 | 4 validated test-gap/tooling fids: SH-3b-02: k2addr_test.cpp: STEAM(Bo |
| [ ] | 132 | WI-100 | P3 | hold | S | - | Gemini latent hardening: GuiKit wr write-probe, CodePatch writeChecked order, TwoDA cells  | 0 | 4 | Real in code, unreachable today (g2/gate.md): X-T01 wr<T> uses probeRe |
| [ ] | 133 | WI-101 | P3 | verify-only | S | - | Gemini runtime-unsure checks: CO-2d-02, HN-6n-03, QH-2a-03, SH-3a-08 | 0 | 0 | CO-2d-02: Hover/click gated by inPop (1134, 1143) but the native toolt |
