# 27 — Companion self-heal portrait buttons (design)

## Status
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
  - Batch test checklist (on user GO): 1) install 0.3.0-dev; 2) play ~10 min: each companion's button heals, full HP dimmed, right-click popover toggles persist across restart, auto fires below threshold and stops at keep, clicks on buttons do not move the party; 3) `python3 scripts/selfheal_log_check.py` (USE/DONE/TIMEOUT summary).
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
