# 27 — Companion self-heal portrait buttons (design)

## Status
- 2026-10-04 CS0 research DONE (static only, nothing installed). Evidence: `27_companion_selfheal_evidence/` R1 actor route, R2 portrait UI, R3 item choice.
- Home (user, 10-04): **new mod `companion-selfheal`**, a `k2.portraitslots@1` consumer. No new hook address.
- Next: CS1 (pure logic + tests). Build rows live in doc 24 (CS0-CS6, owner gate "selfheal").

## User decisions (10-04)
- One click on a button by a companion's portrait: the mod picks the **smallest healing item that covers the companion's missing HP** (else the largest). Droids get repair kits.
- The **companion itself is the actor**: it plays the animation and uses its own action (and its own skill scales the heal).
- Leader card gets no button (the hotbar / normal item use covers the controlled character).

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
