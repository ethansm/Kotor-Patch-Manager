# 27 — Companion self-heal portrait buttons (design)

## Status
- **10-12 17:44 0.8.2-dev INSTALLED (hot-swap a98b5866 -> a4544278, backup `companion-selfheal_swap_20261012_174417`, repo 1d65bd2, KPM 563870a):** polish round 3: Auto-heal pill 2:1 (64x32 textures, pillW = 2*pillH), cursor inside the open popover no longer hovers buttons beneath (no sound/halo; log `POPOCCLUDE m= btn=`), checker summarises `ARROW` + `POPOCCLUDE`. Awaiting play test: pill narrower, no hover through popover, leader left-click heal (`CLICK member=0 ok` + `USE member=0`), report whether native tooltips still show under the popover.
- **10-12 11:05 0.8.1-dev INSTALLED (hot-swap bb2a0b3b -> a98b5866, backup `companion-selfheal_swap_20261012_110526`, repo b6581b9, KPM fbbfeac):** vanilla look polish (r1.md in `.claude/waves/w3-27-polish/`): teal/cream palette, stepper arrows = vanilla `uibit_arrowneg_p/pos_p` textures (dim at bound, text fallback), pill recoloured, checkbox sound (idx 3) on toggle. Awaiting play test; check log line `ARROW neg=.. pos=..` (non-zero = vanilla textures resolved).
- **10-12 0.8.0-dev INSTALLED (hot-swap ad5af355 -> bb2a0b3b, backup `$G/backups/companion-selfheal_swap_20261012_005943`, repo 33221b9+d14d856, KPM a9577ff+50ff536):** leader LMB = medpack on self (disabled at full HP / no item), leader RMB = popover; popover sized from text (`POPSIZE`); Auto-heal pill toggle with hover texture + hover/click sounds; stepper rows beige on hover. Awaiting play test (checklist in the 10-12 plan; then `python3 scripts/selfheal_log_check.py` -> P1-P7).
- **10-05 21:34 0.7.0-dev INSTALLED (doc 24 CS10-CS12, hot-swap 968041f1 -> 469ee363):** registers hud-host strip slot order 0 (`shui::stripReserve`), button draws the shared `hudkit::frame` (HudKit.h) behind `csh_cross`, disc = `kit_disc`; 16 widgets. Play check: button right-most, queue strip left of it, no overlap, art unchanged.
- **10-05 CS6 DONE `f50bd15`:** `companion_selfheal_src/README.md` (settings GLOBAL, not per save) + Wine smoke `selfheal` (wire/inert/badchain/kick-off, wiring only; kick-rotate unreachable in Wine). Install already live 0.6.0-dev. Play test (doc 27 CS12 checklist) still pending.
- **10-05 01:41 CS9b + CS11 INSTALLED (user GO):** stage `$K/cs11_stage_1005` (head 70e5144), 36 ids / 221 hooks, selfheal dll `fe9025eb` (kpatch `6f73a4f7`), queue-hud 0.3.0-dev `8bfa99d9`, config `d9ed58a2`, backup `$G/backups/cs11_install_20261005_0141` (keep_shield-bar/perf-lab/mouse-probe restored; verify = those 3 mismatches only). CS12 0.6.0-dev installed 01:55, see CS12 RESUME HERE below.
- 2026-10-04 CS0 research DONE (static only, nothing installed). Evidence: `27_companion_selfheal_evidence/` R1 actor route, R2 portrait UI, R3 item choice.
- Home (user, 10-04): **new mod `companion-selfheal`**, a `k2.portraitslots@1` consumer. No new hook address.
- 10-04 later: user added in-game config (auto-consume, HP%/reserve thresholds, auto-on indicator, no click at full HP) -> section 3b. R4 done: own hook 0x0040F5A0 gives text labels without touching the hotbar.
- 10-04 CS1 DONE `6f67e8b` (KPM `84c2586`): `_shared_src/HealItems.h` (formulaFor/healEstimate/pickHealItem/shouldAutoUse/isHealingRow copy), GuiKit `rclickEdge`; cov 100%, 23/23 mutants killed.
- 10-04 CS2 DONE `4c8f7e7` (KPM `fa18db3`): `companion_selfheal_src` 0.1.0-probe, built NOT installed; pre_install_gate PASS, dll md5 `af3a6fde587c1df32c419fe6e71c5689`. New `_shared_src/ItemRead.h` (hotbar adopts in CS7). Deviation: the dump calls the hotbar's two read-only lookups ClientToServerObjectId 0x51C8B0 + GetItemByObjectId 0x51C0E0 (needed for item props); no CanUseItem, no 0x57D8B0, no engine writes. Candidate cost is fixed at 0 in the probe.
- 10-04 16:57 CS3 probe INSTALLED (user GO): dll `c968a0cd81b23196e1492c8f9979ff30` (rebuilt in KPM `ad1ef30`; STAGE_INFO head=ad1ef302), stage `$K/selfheal_stage_1004`, 33 ids / 211 hooks, config `522e9917`, backup `$G/backups/selfheal_cs3_install_20261004_1657`. Config diff = appended block only; no other dll changed. Rollback: restore that backup dir. Log file: `$G/companion-selfheal_log.txt` (KpLog).
- 10-04 evening (offline, user away): **log checker** `python3 scripts/selfheal_log_check.py [--from N]` (56b6ae7 + fda9723; 10 tests) checks the list below and prints a per-member table + `SELFHEAL P1= P3= DUMP= VERDICT=PASS|FAIL|INCOMPLETE`. Non-heal HUD items (row=-1, heal=-1) are skipped. Not routed in `kotor2_logs.py` yet (sibling-dirty file).
- 10-04 evening: **CS4 prep** `_shared_src/SelfHealUi.h` (d55ac6e; KPM Common + forwarder 60a8699): `shui::buttonRect` (§2 formula), `resolveState` (Hidden > FullHp > NoItem > Pending > AutoOn > Normal), `clickAllowed`/`clickMinCount`, `stepHpPct`/`stepKeep`, `CompCfg` INI `parseLine`/`serialize`. Coverage 100% (66/66), mutation 168/181 killed, 13 survivors all equivalent.
- 10-04 evening: **CS3b 0.2.0-probe BUILT, NOT installed** (repo d4f6796 + review bf28234; KPM 9a85a6b + 26b0d2c; kpatch dll md5 `b772718cc240916566f0ba4e99e04299`, gate PASS build-only; builds are nondeterministic, so the gate's own dll md5 differs). GameAddr `CanUseItem 0x5D2F80` (thiscall, 12 param bytes = RET 0xC). The default INI behaves like 0.1.0 (one extra init line). `companion_selfheal.ini` opt-ins for the test only:
  - `[probe] call=1`: Ctrl+Shift+J -> first non-leader with a pick; REFUSE (debounce 2 s / post window / +0xEC=0 / ctx.inMenu / ids / `canuse=0`) else `CALL routeA member= item= pre hp= stack=`, one call, `POST t= hp= stack= cmd=` every 500 ms for 10 s, `RESULT hp+ stack+ (gone=)`.
  - `[probe] label=1`: one LBL_QUEUE1 "TEST" label per HUD ctor (ProjectionFlag cleared/restored), `P2 label= why= extent=`; P4/P5 not implemented (logged once).
  - Install on user GO only, after the 0.1.0 log PASSes.
- Next: user play session, then read the `companion-selfheal_log.txt` lines with `kotor2_logs.py check` (the CS3 one-click live call stays TODO until this log PASSes):
  - `P1 hud built ... layoutOpen=1` once per HUD build (hook fires with layout still open; ctlCount sane).
  - `P3 slot[i]=... k=0|1|2` for every portrait slot (never `k=none`) -> confirms slot->HUD formula.
  - Ctrl+Shift+H: per companion `cand` rows (row 64-66 organic / 84,127,128 droid, heal vs in-game skills, count = stack), the pick, route A args (server ids = client & ~mask), auto decision.
  - Pass = 2 companions x (organic, droid) match in-game numbers (section 6). Then the one-click live call.
- 10-04 night: **CS4 + CS5 0.3.0-dev BUILT, NOT installed** (user: "keep implementing the feature, we can test everything built together later"). Repo 093816c (SelfHealUi popover/INI/pending, cov 100%) + d234649 + review de98925; KPM d27a188 + 10283dc + b56483b; checker f12f0f4; kpatch dll md5 `bd80fb2903ffe4867c693a34e0e7869f` (1 hook 0x0040F5A0, gate PASS build-only; dll 1.2 MB from static libstdc++).
  - Built: "H" button per companion card (states: full HP dim, no item red, pending pulse, auto green, native tooltip); LMB = `useNow` (gate + debounce 2 s + +0xEC + CanUseItem + route A); RMB popover (Auto / Use below % / Keep) -> `.\companion_selfheal.ini` `[companions]` via atomicWrite + FileWatch reload; auto every 250 ms with minCount keep+1, only for shown cards; `DONE` on HP up / stack down / 6 s timeout. `[probe] label` removed; `[probe] call` + Ctrl+Shift+J/H kept.
  - Review fix: INI path now `.\` (a bare name sends GetPrivateProfileIntA to the Windows dir; 0.1.0/0.2.0 read their `[General]`/`[probe]` keys from there).
  - Live risks: click-through to world, route A behaviour, companion-ai ClearAllActions wiping the queued use.
  - **INSTALLED 10-04 20:44 (user GO "install")**: stage `$K/selfheal030_stage_1004` (STAGE_INFO head=b56483b; DIRTY = sibling CharEffectsPanel/LUO .bak only; PROBLEMS 1 = known 4gb `n` artifact), 33 ids applied, verify mismatches 0, live dll `bd80fb29`, config `56e83f2e` (same lines as 281effa4; 3 hook-less ids reordered). Backup `$G/backups/selfheal030_install_20261004_2044` (rollback: restore it). No INI yet = defaults. Next = user plays the checklist below, then "check selfheal log".
- 10-04 late: **0.4.0-dev BUILT, NOT installed** (cross icon + gold auto ring). Art 65b614c (scripts/gen_selfheal_textures.py -> SelfHealTextures.inc, csh_cross + csh_ring0..7); look d223930 (shui::lookFor/ringRect, cov 100%; KPM 272e073); wiring 9f5a422 (KPM ea0aeb1; 12 widgets, embedded-texture copy of hotbar recipe, PanelWhitelist -> gameaddr vtable names, checker BUILD n/N). kpatch dll md5 `d6f02fc9dc0ffd42a15791c43458be4f`, 1 hook 0x0040F5A0, gate PASS build-only, raw_addresses FAIL=0. Review gate (Opus) PASS, no findings. Install on user GO (replaces 0.3.0).
  - **INSTALLED 10-04 23:11 (user GO "install")**: stage `$K/selfheal040_stage_1004` (STAGE_INFO head=09c14f63; DIRTY = sibling CharEffectsPanel/LUO .bak + our 0.3.0 .bak only; PROBLEMS 1 = known 4gb artifact), 34 ids (incl. sibling queue-hud), verify mismatches 0, live dll `d6f02fc9`, config `b073054f` (was bdadbf1e). Backup `$G/backups/selfheal040_install_20261004_2311` (rollback: restore it). Next = play the batch checklist, then "check selfheal log".
  - **0.4.1-dev DLL SWAP 10-05 00:13 (user "install when ready")**: fixes the inverted leader rule (doc 30 finding: `vit.w < 70` treated the live 47x96 companion vits as leader, so 0.3.0/0.4.0 never drew a button; leader vit is 103x208). New `shui::isLeaderVit` (w >= 70), tests use the live rects. Repo 8e3fc20, KPM d39eb4e. Swapped `patches/companion-selfheal.dll` only (d6f02fc9 -> `395cc7b9`), hook set unchanged (1 hook 0x0040F5A0), config untouched `629754e3`: a full KPM reinstall would have reverted the sibling queue-hud hand swap (00:09). Backup `$G/backups/selfheal041_swap_20261005_0013` (rollback: copy the dll back). Next KPM reinstall must take CompanionSelfHeal.kpatch from `$K/Patches/CompanionSelfHeal` (dll 395cc7b9).
  - **0.4.1 play test 10-05 00:20 (user screenshots) — RESUME HERE (CS8 batch, plan in doc 24 CS8-CS12):**
    1. Buttons DRAW now (gold cross + ring on member 1 auto, grey cross on member 2). Ring art reads OK.
    2. "Button isn't clickable": log shows clicks DO arrive, the engine refuses: `REFUSE canuse=0 member=1 item=000002d0` x7 (CanUseItem 0x?, thiscall(server, itemPtr, 0, 1) at CompanionSelfHeal.cpp:273; log prints the server id, the ptr is passed) + `REFUSE debounce` x6 (debounce stamps on refused attempts). Route A was never live-verified (0.2.0 probe never installed). Root cause open.
    3. RMB moves the camera: rclickEdge only polls GetAsyncKeyState, nothing swallows RMB (LMB is swallowed via clickable flag 0x20 hit-test).
    4. RMB popover: LBL_MOULDING3 bg draws (dark-green rounded box) but the item labels are invisible; Auto still toggled (hit-test works). Suspects: LBL_QUEUE1 template text colour/alpha never set; wText cache never re-applies.
    5. `WARN member=2 has an empty tag, INI key slot2` (tag read empty for member 2).
    User direction: cross styled like the hotbar wrench (texture + shading) + a dark disc/outline the gold ring spins on; companion queue = separate mod on the shared HUD dll (hud-host, doc 31 s5), selfheal takes the right-most slot; dark-green hotbar-style panels (border (0,44,26)/(10,96,80), fill (0,0,0,175)) + Cancel button like vanilla "Cancel Combat"; slim per-companion version.
  - Batch test checklist (on user GO): 1) install 0.3.0-dev; 2) play ~10 min: each companion's button heals, full HP dimmed, right-click popover toggles persist across restart, auto fires below threshold and stops at keep, clicks on buttons do not move the party; 3) `python3 scripts/selfheal_log_check.py` (USE/DONE/TIMEOUT summary). 0.4.0-dev additions: green cross when hurt; grey + faint at full HP / no item; gold cross + circling ring when auto is on (ring also at full HP); ring gone in menus/dialogue; log has no `ICON fallback` / `TEX ... giving up`.
- 10-05 (sibling, combat-hotbar session): **0.4.2-dev** a9ee44d (KPM 0e8942d) moves the 12-label build onto hud-host `K2HUD_BUILD` (hud-host 0.3.0-dev). The 0x40F5A0 hook is gone, so hud-host must be installed and listed before companion-selfheal. **0.4.2-dev: build moved to hud-host K2HUD_BUILD, LIVE 10-05 00:54** (combat-hotbar session install): selfheal dll `3e763e45`, hud-host 0.3.0-dev `c6121ae2` (owns 0x40F5A0), combat-hotbar 0.4.0-dev `ee1112e0`, config 629754e3 -> `1f7b0947`, backup `$G/backups/combathud3_install_20261005_005409`.
- 10-05 **CS8 + CS9: 0.5.0-dev BUILT, NOT installed** (repo 174984a, art 03710fd, design f0fa15a):
  - KPM 5a9e9c9 (on top of 0e8942d). kpatch dll md5 `693cc97b`. Gates: pre_install_gate PASS; hook_spans vs live 1f7b0947 FAIL=0 (WARN = source-only 0x7B3817, expected); shared headers 0 drifted.
  - Evidence:
    - R5 `R5_canuse/FINDINGS.md`: the args were right. analyzeMember never ran CanUseItem per candidate, so the pick could be an item the engine refuses for that member.
    - R6 `R6_rmb/FINDINGS.md`: the RMB-hold flag 0xA7FCBC is set in 0x7B3810 before the GUI sees the click. Fix = own detour 0x7B3817 (skip_original_bytes; bytes checked against the exe).
  - Changes:
    - CanUseItem runs strict per candidate (skipped at full HP).
    - Debounce stamps only real uses. REFUSE logs ptr/name/race.
    - RMB swallow over the shown rects (Mouse Look=1 TODO).
    - Popover colour + alpha are set on show; one-shot `DIAG pop`.
    - `TAG member raw=` probe.
    - 14 widgets: csh_disc under each button.
  - **INSTALLED 10-05 00:57 (user GO "install")**: stage `$K/selfheal050_stage_1005` (STAGE_INFO head=5a9e9c95), 36 ids / 221 hooks, live dll `693cc97b`, config 1f7b0947 -> `db096abc` (diff = the new 0x7B3817 hook block only). Backup `$G/backups/selfheal050_install_20261005_005737` (rollback: restore patch_config.toml, kpm_install_state.json, patches/).
    - Stage had dll=NO-MATCH for perf-lab / mouse-probe / queue-hud (hand-swapped, hooks SAME). User chose install + restore: their pre-install live dlls (`f085f754` / `71e8f8e5` / `2950ce26`, copies `keep_*.dll` in the backup) were copied back after the install, so verify reports these 3 as expected mismatches.
    - Next = play the checklist below, then `python3 scripts/selfheal_log_check.py`.
  - Install needs: hud-host 0.3.0 live (HL3); the hook set changes (+0x7B3817), so a full staged KPM install (not a dll swap); `dll=NO-MATCH` 0 first.
  - Checklist:
    1. Click heals (log `USE` + `DONE`).
    2. RMB opens a readable popover and the camera stays still.
    3. Textured cross on a dark disc.
    4. Gold ring spins on the disc edge.
    5. `TAG member=2 raw=` line for the empty-tag investigation.
- **0.5.0 first look 10-05 01:00 (screenshot) — RESUME HERE:** buttons + discs draw on both companions. User direction:
  - Cross v3 (CS9b): a custom cross like the hotbar wrench/gear (pre-coloured mint->teal gradient, outline, rim light, shade, as in `gen_hotbar_textures.py` `make_edit_textures`), NOT a white body tinted bright green. Make it a bit smaller inside the circle. Use the UX scheme.
  - CS11 not visible: no dark-green out-of-combat action bar with a Cancel button at the bottom, and none on the companion queues. queue-hud 0.2.0 is loaded (portrait-slots register line 16) but has no log file in `$G`. It draws only when the queue is non-empty, and it has no Cancel.
  - Log so far (8 lines): only `WARN member=1/2 has an empty tag`, no USE/REFUSE yet; TAG probe line not visible at the current log level.
- **CS9b + CS11 built 10-05 ~01:10, NOT installed (KOTOR-II f645863 + ca81e72, KPM 70e5144, stage `$K/cs11_stage_1005`):** cross v3 (mint/teal shaded, ~22 px, untinted; gold ring via `shui::kRingR/G/B`), selfheal on shared `EmbedTex.h`; queue-hud 0.3.0-dev kit frames + Cancel (doc 30, R7). Install awaits GO; restore perf-lab, mouse-probe AND shield-bar (01:22 hot-swap) dlls after.
- **CS12 fix 0.6.0-dev INSTALLED 10-05 01:55 (user "build and install") — RESUME HERE:** R8 (`27_companion_selfheal_evidence/R8_follower_starvation/FINDINGS.md`, 9ae86b5) = route B rotate: after route A, when the follower queue is `[0x3D..,0x3F]` and gates hold (cr+0x20C==0, cr+0x11AC!=0, round AA8==0, AB8<1, RoundHasPending 0x590110, unpaused), pop head 0x7F2A90 + AddTail 0x54AA30 k times. Dedupe: useNow refuses while the round list holds an item node (`actq::useQueued`); pending uses `shui::pendingDone2` (timeout only over a not-queued stretch). Shared reader `_shared_src/ActQueue.h` (queue-hud uses it too). INI `[queue] kick=1` (0 = no rotate). KOTOR-II e900c1d, KPM 568b221, stage `$K/cs12_stage_1005`, selfheal dll `968041f1` (kpatch 77a75ddb), queue-hud `7d7650a8` (kpatch 51d38ee7), config `f5ef36cf`, backup `$G/backups/cs12_install_20261005_0155` (keep dlls restored, verify = shield-bar/perf-lab/mouse-probe only).
  Live checklist: (1) out of combat, click a companion cross -> item used within ~1 s, companion keeps following; (2) 2nd click while queued -> `REFUSE already queued`, strip never shows 2 medpacs; (3) auto never stacks two; (4) paused -> item on the strip, runs right after unpause; (5) `python3 scripts/selfheal_log_check.py` shows `KICK ... route=rotate`, no `TIMEOUT unqueued` while queued. UNVERIFIED gate `cr+0x11AC`: if the log shows only `KICK skipped (cr11ac=0)`, drop that gate.
- **CS12 user report 10-05 01:09:** companion heal items queued via route A stay queued / run seconds late / run at once when the companion becomes leader; can double-queue (screenshot: 2 medpacs on one strip). H1 = queued behind FollowLeader/AI actions of a non-controlled creature; H2 = 6 s pending timeout re-arms the button while the item is still queued. Plan: R8 RE + dedupe + front-insert/kick. See `.claude/handoffs/27_companion_selfheal.handoff.md` addendum.
- Build rows live in doc 24 (CS0-CS7, owner gate "selfheal").

## User decisions (10-04)
- One click on a button by a companion's portrait: the mod picks the **smallest healing item that covers the companion's missing HP** (else the largest). Droids get repair kits.
- The **companion itself is the actor**: it plays the animation and uses its own action (and its own skill scales the heal).
- Leader card gets no button (the hotbar / normal item use covers the controlled character).
- **(10-04, later)** In-game configuration through GUI elements like the Action Hotbar's, not only an INI:
  - click-to-use stays;
  - **auto-consume** with an **HP threshold** and an **inventory reserve threshold**;
  - the button **shows when auto is on** and is still clickable to force a use;
  - **unclickable at 100% vitality** (the user suggested it, so it's adopted).

## 1. Actor route (R1)
Client `PerformUseItem 0x775600` is out: server handler `0x65BE70` runs UseItem on the sender's controlled creature (`0065c5ff`).

**Primary (route A):** call `CSWSCreature::AddUseTalentOnObjectActions 0x57D8B0` (thiscall, `RET 0x20`, verified B12a) on the companion's server creature:
```
ecx = server creature = 0x77D800(member)         // Party.h members()
0x57D8B0(0 /*spell/item*/, 0, selfServerId, 0, itemServerId, propIdx /*0*/, 0, 0)
itemServerId = 0x51C8B0(clientItemId)              // ClientToServerObjectId, verified B9
```
- It checks commandable `creature+0xEC` (`0057d8e3`) and then calls UseItem `0x57B4C0` (`0057d98f`). Script `ActionUseTalentOnObject` uses the same path (`0066beaa`).
- UseItem → `0x5648D0` adds a combat-round item entry + action 0x3F. AIActionCombat `0x6D9500` builds action 0x2E (`006d9ef6`). Executor `0x5B8000` (`005d5fc4`) casts the spell, then consumes the item (`0x6070A0`) on success. Result: the companion's own animation, in the combat round, like a player use.
- The UseItem result is ignored, so a refusal is silent. **Pre-check `CanUseItem 0x5D2F80(itemPtr, 0, 1)` strict ourselves** (it takes a CSWSItem*, via `0x51C0E0`). It covers droid/organic and full HP.
- Possession: no checked function requires the actor to hold the item. Party inventory should work. The tail of `0x6070A0` is unverified, so CS3 must confirm the stack drops by 1.
- Cancels: `+0xEC == 0` (cutscene/stun) → no-op. Any ClearAllActions(1) `0x541080` wipes the round entry: conversations, attack/stance actions, **companion-ai** calls it. A combat re-plan and a leader swap are hypotheses (CS3 checks them).
- Fallback (route D): AssignCommand(companion, ActionUseTalentOnObject) through the script VM. It uses the same engine path plus an NCS/VM dependency (robustness 1.3.0 AssignCommand leak note).
- Route B (`0x57B4C0` direct) is a second fallback: it returns a refusal code, but it skips the +0xEC gate and needs pos/area.

## 2. UI (R2)
- **Rect:** anchored to the live `Slot_VitRect` (K2Addr `Slot_VitRect` +0x694, screen px, same space as draw and hit-test). The button sits in the free strip left of the companion health bar:
  `pxu = vit.h/36; s = clamp(round(vit.h*0.25),16,24); gap = max(2, round(2*pxu)); x = vit.x-gap-s; y = vit.y+(vit.h-s)/2`
  At 3840x1600 that gives 24 px at x 3642; at 1080p, 16 px.
  - **No hotbar aspectFix:** portrait cards are right-anchored.
  - Skip the leader (capture `ctx->isLarge` in `K2PS_SLOT`, or `vit.w < 70`), `creature == 0`, and absurd rects.
- **Draw:** `K2PS_HUD_DRAW` (leader call, clip stack empty), priority 300 (after shield 100 and buff 200). Use flat `gui2d::fillRect` under `CapGuard` (textured fx flickered, doc 08). A plain TGA icon via `drawImage` is OK later. Hover brightens the button and plays `guikit::playHoverSound`.
- **Input:** `K2PS_FRAME_BEGIN` polls `guikit::clickEdge` (VK_LBUTTON edge + foreground) with the cursor from `GetCursorPos`/`ScreenToClient`. Hit-test against the rects cached in the same frame. Gate: `ctx->inMenu == 0` + a HUD-active check (the hotbar's `g_hudGateOk` is private; copy its predicate).
- **Main v1 risk: click-through.** A polled button has no engine control under it, so the same press may also reach the world (the leader walks/attacks at that point). The portrait select (`BTN_CHAR`) is not under the strip. Accepted for v1; CS4 measures it live.
  - Fix later: hotbar-owned click-absorbing labels via the doc 22 `hud-host` service (`k2.hud@1`, L4 deferred).
- **Tooltip:** not possible with a pure draw (Gui2D has no text). v1 ships without one: a recognisable icon + hover.

## 3. Item choice (R3)
- Heal comes from the vanilla script `k_sup_healing` (spells.2da impactscript), scaled by the **user's** skill. Live override rows 64-67/84/127-129 are vanilla.

| Row | Item | Heal |
|---|---|---|
| 64 | Medpac | TI + 10 + WIS |
| 65 | Advanced Medpac | 20 + 2·TI + WIS |
| 66 | Life Support Pack | 30 + 3·TI + WIS |
| 84 | Repair Kit | Repair + INT + 15 |
| 127 | Advanced Repair Kit | 2·Repair + INT + 25 |
| 128 | Construction Kit | 3·Repair + INT + 35 |
| 67 / 129 | Antidote / Squad Recovery Stim | excluded by row |

- **Candidates:**
  - Read HUD item group 1 (`readGroup(hud,1,…)`); stack count from the `Name (N)` suffix via `hb::StackCount`.
  - `serverItemFor` → `classifyItem` gives the spells row and `isHealingRow`.
  - Gate each with strict CanUseItem on the companion's server creature, then key the formula by row.
- **Skill inputs:** stats `creature+0x1198`, skill ranks `+0x174` (Repair 5, TI 7), abilities `+0xED`. These are unverified and base only, so the estimate is a lower bound. CS2 calibrates it against live HP deltas.
- **Rule:** `int pickHealItem(int missingHp, const HealCand* c, int n)`.
  1. Missing ≤ 0 → -1.
  2. Drop unusable, count 0, heal ≤ 0, and rows 67/129.
  3. Pick the smallest heal ≥ missing, else the largest.
  4. Ties: lower cost, then larger stack, then lower key.

  11 worked examples are in `R3_item_choice/FINDINGS.md`.
- **Missing HP:** `party::Vitals{cur,max}` (Party.h) on the companion.

## 3b. Auto-consume + config (requirement 10-04, design draft; GUI mechanism pending R4)
**Settings.** Each companion has its own settings. Defaults come from the INI, and per-save storage follows the hotbar's per-save model.

| Setting | Meaning | Default |
|---|---|---|
| `auto` | Auto-consume on/off for this companion | off |
| `hpPct` | Auto fires when `cur*100 < hpPct*max` | 35 |
| `keep` | Inventory reserve: auto only uses an item while the party stack of the picked item is > `keep` (manual clicks ignore `keep`) | 2 |

**Auto rule (pure, CS1).** `shouldAutoUse(cfg, vitals, now, lastUse, pending)` fires only when all of these hold:
- `auto` is on;
- the companion is alive, commandable (`+0xEC`) and in the HUD frame, with no dialog/menu/pause;
- HP is below the threshold;
- no use of ours is pending;
- at least 6 s (one combat round) has passed since our last use;
- `pickHealItem` finds a candidate whose count is > `keep`. The picker gets a `minCount` parameter (`keep+1` for auto, 1 for manual).

Auto runs the same route A as a click. It's evaluated in FRAME_BEGIN at most every 250 ms.

**Button states** (flat draw; order = priority):

| State | Look | Click |
|---|---|---|
| hidden | the slot has no creature, or it's the leader | none |
| full HP (100%) | dimmed | ignored (user) |
| no usable item | dimmed + red tint | logs "no item" |
| use pending | pulse | ignored |
| auto on | green ring/border | forces a use (ignores `keep`) |
| normal | plain | uses the pick |

**Config GUI (R4, `27_companion_selfheal_evidence/R4_config_gui/FINDINGS.md`).** The mod gets its own hook at **StopLoadFromLayout entry `0x0040F5A0`**:
- Original bytes `55 8B EC 83 EC 10`, ECX = panel. The HUD constructor calls it at `0x00748508`, after the hotbar's `0x748502` hook.
- The hook filters on HUD vtable `0x0099DCC4`, and checks the layout is still open (`[panel+0x30] != 0`, `[panel+0x48] & 2`).
- No mod hooks it. KPM only rejects an identical address (`HookValidator.cs:116-132`), and `check_hook_spans.py` passes.
- Opus verified the bytes, the callsite and that no other hook uses this address.

**What the hook builds.** About 16 runtime labels from the `LBL_QUEUE1` / `LBL_MOULDING3` templates (hotbar recipe: `guikit::createLabelFromTemplate` + `appendToPanel`, projection flag `0x00A1B9F0` cleared around the binds). This gives:
- real text: `setText`, `setFont("dialogfont10x10")`;
- **native click absorption**, which removes R2's click-through risk;
- tooltips.

Per-frame work stays in `K2PS_FRAME_BEGIN`, and HUD_DRAW becomes optional.

**Widgets:**
- **Button:** `H` per companion card, positioned each frame with the R2 formula.
  - Left click: use now.
  - Right click: open the popover. This needs a new `VK_RBUTTON` edge in GuiKit; the fallback is Shift+LMB.
- **Popover:** `[Auto: ON/OFF]`, `[Use below − 35% +]` (step 5, 5–95), `[Keep − 2 +]` (step 1, 0–20). "Apply to all" is optional.

**Persistence.** v1 uses a global `companion_selfheal.ini`: FileWatch reload + atomicWrite, keyed per companion. Per-save storage needs the save hooks (hotbar / perf-lab), so it waits for `k2.saveevents@1` (L3) + a shared `SaveDir.h`.

**Fallbacks:**
1. b1, a hook at `0x007484F7` inside the HUD constructor (check_hook_spans WARN).
2. INI + a flat R2 button (no text).

**Upgrade.** `k2.hud@1` (L4) later replaces our hook.

**Probes:**
- P1: log-only hook (panel, vtable, layout-open flags).
- P2: one "TEST" label with text; a click on it must not move the leader.
- P3: the slot→HUD formula `slot-0x1F34-i*0xDE4` equals the hook's panel.
- P4: native GuiTip.
- P5: right-click edge.

**companion-ai interplay.** The AI may already medpac (backlog AI-MEDPAC-FORCECLASS, R4 `GN_PartyMedpacStock` design), and its ClearAllActions can wipe our queued use. CS3 measures both. If they clash, our auto skips a companion that companion-ai has just healed (no shared flag in v1).

## 4. Home decision
| Option | For | Against |
|---|---|---|
| **New `companion-selfheal` mod (chosen)** | Small and isolated. Uses portrait-slots phases only (no new hook address). Ships and rolls back independently. | Own input poll; v1 click-through; must share item code with the hotbar |
| Extend Action Hotbar | Owns 0x748502 + item code; labels absorb clicks; tooltips | Hotbar grows; collides with the HP2-HP7 polish queue |
| companion-ai | Already drives companions | Wrong scope (autonomous AI vs. player command); calls ClearAllActions, which would cancel our use |

**Shared logic extraction:** CS1 moves `isHealingRow` (HotbarLogic.h:430) and the pure part of `classifyItem` (ActionHotbar.cpp:506) into a `_shared_src/HealItems.h` with the new `pickHealItem` + the row→formula table. The hotbar adopts it later (owner gate hotbar). Until then, selfheal includes the header and the hotbar is untouched.

## 5. Risks
1. Click-through to the world (section 2).
2. Silent refusal of route A. Mitigation: pre-check CanUseItem + `+0xEC`, then log "queued" and watch HP/stack in the next frames ("applied" or "timeout 3 s").
3. companion-ai ClearAllActions wipes the queued use. Mitigation: CS3 tests with companion-ai live; if needed, add a short "self-heal pending" flag that companion-ai honours (owner companion-ai).
4. Heal estimate understated (gear bonuses). This wastes at most one tier, and a second click fixes it.
5. Party inventory decrement path (`0x6070A0` tail) is unverified.

## 6. Probe gate (live, user GO each)
- **CS2 log-only DLL:**
  - A debug hotkey (no button) logs, per companion: member idx, server creature, +0xEC, cur/max, candidates (row, heal, count, CanUseItem), the pick, and the route A args.
  - It makes no call.
  - Pass: 2 companions × (organic, droid) logged correctly against the in-game numbers.
- **CS3 one-click live:**
  - Hotkey calls route A on one companion holding a medpac, out of combat.
  - Pass:
    - the companion animates;
    - HP rises by about the estimate;
    - the party medpac stack drops by 1;
    - the leader doesn't act.
  - Then repeat in combat and with companion-ai live.
- Rollback: remove the patch id from the live config (KPM), the backup dir is named in the install row.

## 7. Shared companion strip + queue UX kit (CS10-CS12 design, 10-05)
Design only. The code belongs to the queue-hud / hud-host owner (docs 30/31). Proposal for that owner: `30_queue_hud_evidence/PROPOSAL_selfheal_strip.md`.

### 7.1 One strip per companion card
- Each companion card gets one strip that runs right to left, ending next to the vit bar.
- Selfheal takes the **right-most slot**, so it sits next to the vit. Queue-hud already keeps that space free: `selfHealReserve(vit)` (QueueHudLogic.h:178) = gap + button + ring overhang + `kStripGap`, which is 38 px on the live 47x96 vit.
- The queue icons fill the strip leftward from there.
- The leader card (vit 103x208, `shui::isLeaderVit`) has no selfheal slot.
- The hard-coded reserve becomes a hud-host reservation, so neither mod knows the other's size:
  `ReserveSlot(owner, card, order, width) -> Rect`. Order 0 is right-most; selfheal asks for order 0 and queue-hud for 1..n. hud-host stacks the slots right to left from `vit.x - gap`.
- Until that API exists, both sides keep the current constants. Our `shui::buttonRect` and their `selfHealReserve` must stay equal; a contract test on the live vit pins this.

### 7.2 Queue UX kit (CS11)
Same look as the action hotbar, from `scripts/gen_hotbar_textures.py`:

| part | spec |
|---|---|
| panel | rounded rect, border (0,44,26) (hover (208,204,170)), fill (0,0,0,175) inset 6/128, radius 22/128 |
| frame accent | (10,96,80), the hotbar wrench `fph_edit` frame |
| icon | textured, shaded glyph (wrench recipe: gradient body, 1 px bevel, dark outline), tinted by state |
| Cancel | modelled on vanilla "Cancel Combat": a dark-green panel with a red-ish X glyph; LMB cancels the companion queue (route = queue-hud owner's RE, doc 30) |
| slim variant | per companion: icon size = selfheal button (`shui::buttonRect` side), no title, Cancel as the left-most slot |

- The selfheal disc (CS9 `csh_disc`: fill (0,0,0,175) + 2 px (10,96,80) rim) already uses the kit colours, so the button fits next to the queue tiles without restyling.

### 7.3 Queue as its own mod (CS12)
- queue-hud stays a separate mod. Both mods are hud-host consumers, so there is one shared dll for HUD phases and no private copies of the kit.
- **RESOLVED 10-05 by a9ee44d (option b): selfheal 0.4.2+ registers `K2HUD_BUILD` and has no 0x40F5A0 hook.** Original note:
- **Hook collision to settle first:** hud-host 0.3.0-dev (9e18444) hooks 0x0040F5A0 for `K2HUD_BUILD`, and selfheal has hooked the same address since 0.3.0. Two KPM hooks on one address must not both ship.
  - Plan: selfheal moves to `Register(K2HUD_BUILD, ...)` and drops its own hook once hud-host 0.3.0 is live.
  - Until then, installing hud-host 0.3.0 needs a check that both hooks don't overlap (`check_hook_spans.py`).
- Order of work: hud-host `ReserveSlot` + BUILD live → selfheal on hud-host (no own hook) → queue-hud kit + Cancel → slim variant.

---

## 8. 2026-10-08 Polish Additions (doc 33 workstreams SH1, SH2, SH3)

1. **SH1: 58px Companion Health Buttons & Action Tray UX:**
   - Enlarge companion health button from 16..24 px to 58x58 px (`w=58, h=58`), matching companion queue icon dimensions.
   - Combine with queue-hud companion strips into a unified action tray using custom `lbl_miqueue_teal` style framing (sea-green `#139373` border, translucent black fill).
2. **SH2: Leader Health Button (Top-Left of Leader Portrait):**
   - Drop `isLeaderVit` restriction.
   - Anchor a 58x58 px health button at top-left of player portrait (`Slot_CardRect 0x54C` / `Slot_VitRect 0x694`).
   - Wire member 0 into click-to-heal and auto-heal loop.
3. **SH3: Options Popover UX Blank Text Fix:**
   - Popover labels cloned from `LBL_QUEUE1` lack an initialized `CSWGuiTextParams` renderer (`+0xD8+0x14 == 0`), causing silent failure of `setText`, `setFont`, and `setTextColor`.
   - Clone from `LBL_HOTBARPTITLE` or render via custom 2D quads; with 58px button size, popover rows expand from 16px to 40..58px, eliminating font clipping.

