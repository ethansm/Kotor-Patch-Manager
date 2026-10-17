# 39 — Enemy Effects Strip (target effect icons, Awareness-gated)

## Status
- **Mod id (proposed):** `enemy-effects-strip` · source `enemy_effects_strip_src/` · Steam Aspyr `6A522E71…` (live exe = vanilla+4GB)
- **State: P0 in progress (2026-10-12).** U1 resolved statically. Phase 0 is a probe that logs and draws nothing.
- Origin: user request after Buff HUD 0.2.2 (doc 02). The Buff HUD only shows party effects; this shows a *target's* effects.

## Design (user decisions, 2026-10-12)
1. **Placement:** a single row of small icons **above the target's vitality bar and name**.
2. **Duration:** the same draining alpha wipe overlay the party portraits/Buff HUD use (`Gui2D.h` `drawVFill`, ratio from `Effects.h` `effectRatio`). No numbers.
3. **Simple, uncluttered:** one row, max `MaxIcons` (default 5), icon only, no tooltip, no text. Sort: debuffs first, then buffs.
4. **Awareness gating (lore-friendly, deliberately low information).** Rank = current party leader's Awareness (skills.2da row 3). Thresholds are INI values (defaults are placeholders, tune in play):

| Leader Awareness | Shown |
|---|---|
| `< TierObvious` (default 5) | nothing (vanilla) |
| `>= TierObvious` | only *obvious* conditions: stun, paralysis, slow, knockdown, hold; icon, no drain overlay |
| `>= TierOwn` (default 12) | + debuffs the party applied (effect creator is a party member) |
| `>= TierFull` (default 20) | + enemy buffs (shields, Valor, etc.) and the drain overlay |

   Optional later: a Force Sight bonus tier (spells.2da 176 / 284). Out of scope for v1.

## Existing pieces to reuse (verified in tree)
- `_shared_src/Effects.h`: `readEffectList(creature, &arr, &count)` (creature+0x148/+0x14c), effect fields `Effect_Type 0x08`, `Effect_Duration 0x0c`, `Effect_Creator 0x18`, `Effect_SpellId 0x1c`, `effectRatio(eff, day, ms, haveTime)`.
- `_shared_src/DebuffIconTable.inc` / `EffectIconTable.inc` / `EffectIconRowTable.inc`: spell id -> icon (0.2.2 live), `iconForSpell(id, debuff, &item)`.
- `_shared_src/Gui2D.h`: `fillRect`, `drawImage`, `drawVFill`, `ClipScope`. `HudKit.h`: slot frame/disc tiles.
- Target pointer is available at the target-menu hook `0x00743850` (doc 31a; `[esp+8]` = target, may be null), but that is the *action menu*, NOT the name/health bar (see Phase 0).

## Unknowns (all resolved by Phase 0, in this order)
- U1: **RESOLVED 2026-10-12 (static RE, Steam 6A522E71, nothing run).** The target NAME and VITALITY bar are NOT a floating label: they are controls inside `CSWGuiTargetActionMenu` (= hud+0xCC), a 2D GUI drawn in a viewport the engine moves to the target's projected screen position every frame. **No new engine hook needed**: hud-host already hooks the tail `0x00743847` (`HH_TargetMenuTail`) and dispatches `K2HUD_TARGET_MENU_POST` with `ctx.menu`, `ctx.target` (0 = none), `ctx.leader`; combat-hotbar consumes it (`CombatHotbar.cpp:877`). Offsets (from menu; sources decomp 0074a780.c, 00742750.c, 00743850.c, 00743280.c):
  - panel origin x +0x1648, y +0x164C; size w +0x1650, h +0x1654 (h = bar bottom, or bottom of action groups); clamp bounds x +0x1658, y +0x165C, w +0x1660, h +0x1664
  - name label (CSWGuiLabel LBL_NAME) at +0x1668: x +0x166C y +0x1670 w +0x1674 h +0x1678 (control rects are LOCAL to the panel)
  - health bar (CSWGuiProgressBar PB_HEALTH) at +0x1A40: x +0x1A44 y +0x1A48 w +0x1A4C h +0x1A50; max +0x1AA0, cur +0x1AA4
  - target ptr menu+0x1644; flags +0x1BAC bit0 = target exists/visible, bit1 = mouse over; timer float +0x1BA0
  - screen pos: panel origin written by 0x743850 via CSWCCreature::GetMenuPoint (FUN_0077e970); units UNPROVEN (probably virtual UI px, y down). P0 probe logs them.
- U2: Can the mod read the leader's Awareness rank? Expected on the creature stats struct; find the skill-rank offset (use `query_ghidra.py struct`).
- U3: Are enemy effect durations/creators populated client-side the same way as for party members? (Probe logs raw fields.)
- U4: Cost with many enemies: draw only for the *selected* target.

## Phases (small, probe first)
- **P0 Probe (no drawing)** — 0.1.0-probe. Hook the target change; log target id, effect list (type, spellId, duration, creator, expiry), leader Awareness rank, and the screen rect of the name/vitality bar if found. Throttle: log on target change only. Verify with `python3 scripts/kotor2_logs.py check enemystrip_log.txt --new`.
- **P1 Logic + tests** (offline, no game): `EnemyStripLogic.h` — tier gating, obvious-effect list, sort/cap, icon choice; `enemy_strip_logic_test.cpp`; `common_contract_test.cpp` pinning the Common APIs used (per repo convention).
- **P2 Draw** — create N icon labels (+ overlay labels for the drain) at `K2HUD_BUILD` like combat-hotbar queue icons (`guikit::createLabelFromTemplate` + `appendToPanel`, `CombatHotbar.cpp:393-414`), reposition each `TARGET_MENU_POST` to a strip above `panelY + nameLabelY`. No custom draw hook. One row above the bar, static icons (tier 1-2), then drain overlay (tier 3). Release-quiet logging (`Verbose=0`).
- **P3 Polish/tune** — thresholds, MaxIcons, spacing, widescreen/ultrawide placement check (doc: ultrawide GUI), interaction with the action menu.
- **P4 Install** — gate, `kpm_hotswap` or full install if hooks are new (no new engine hook is needed, but it is a NEW mod id, so it is not hot-swappable: **full KPM install, hud-host listed BEFORE it, needs user GO with the game closed**).

## Test aids (no enemy spawn needed for logic)
- Offline: P1 tests with fake effect lists.
- Live: console `runscript` (EnableCheats=1 is set in swkotor2.ini; unproven) with a script that applies `EffectSlow` / casts spells 141-143, 201, 269 at a hostile target, or use any combat. Awareness rank can be raised with a forged save (see `kotor2-forced-state-testing`).

## Research wave 10-17 (6 Haiku recons, details in `.claude/research/enemy_strip/R1..R6`; all static, nothing proven live)
- **Probe bug explained (R3, CONFIRMED statically):** `ctx.target`/`ctx.leader` are CLIENT creatures; the effect list (+0x148/+0x14C), icon cache (+0xFFC, count +0x1000, 0x24-byte entries, +0x00 short spell id, +0x14 buff flag) and stats (+0x1198) are on the SERVER creature (doc 02 "client-side" is wrong). Recipe: `cid = *(T+4)`; `sid = cid & 0x7FFFFFFF` (skip 0x7F000000); `S = *(*(0xA1B4A4)+8)`; `obj = thiscall(S) 0x0051C0A0(sid)` (RET 4); `creature = obj->vtbl[0x30]()`. Shortcut `0x0077D800(T)` (thiscall, ECX = client creature; used by Party.h) may return a cached +0xF8 pointer.
- **Awareness (R4):** `stats+0x174` is a POINTER to a signed int8 base array (the probe treated it as inline). `GetSkillRank` 0x006B7BB0: last arg 0 = TOTAL (adds bonuses), 1 = base. Awareness row 3 and skill count 8 still unverified (skills.2da not extracted).
- **Placement (R6):** the target menu draw clips and walks no child list, so labels must be children of the HUD panel at ABSOLUTE display px (3840x1600 on the dev box, int32). Row y = menuY + nameLocalY(2) - 2 - iconH; clamp min y is 106, so there is room. Create in `K2HUD_BUILD`, reposition in `TARGET_MENU_POST` (precedent QueueHudLogic.h:275, CombatHotbar.cpp:568). Panel rect observed: w 533, name h 32, bar y 35 h 16.
- **Drain (R2):** the Buff HUD draws a dim (0.35) full icon plus a clipped bright bottom slice (`brightPx`), via a draw hook; not `drawVFill`, and Gui2D at POST is unproven. Plan: LABELS ONLY, icon label + black overlay label (alpha 0.65, height `s - brightPx`). Missing asset: no solid-fill texture in the repo (options: tint `buffhud_circle` black, or ship a 4x4 solid TGA in override/).
- **Icons (R1):** reuse `k2fx::iconForSpell(spell, true)` first (harmful), else `(spell,false)`; 0x43 EffectIcon effects use `kEffectIconRows[ints[0]]`. Icons are `ip_*` (live override 128x128 TGAs), tint white, no frame; the char pane marks harmful by section order, the Buff HUD by right-edge column. Fear/Horror/Insanity = rows 16/30/31 = `ip_fear/ip_horror/ip_insanity`.
- **Classes (R5):** Fear/Horror/Insanity apply `SetState` (type 0x08) state 8 "Horrified", not "Frightened"; Stasis st5, Stun st4, droid stun st3, sleep st6, Entangle 0x12, Knockdown 0x14, slow = 0x1d. The adhesive grenade is Entangle (not slow); sonic grenade is a DEX decrease (not a state). Never icon: 0x1e, 0x43 as a tile, shield pool (0x02 sub&8), bookkeeping 0x09/0x17/0x44/0x3c/0x67/0x13/0x1f/0x39/0x01. Gate OBVIOUS on type/state, not spell id alone. Spell id is -1 for item-property/innate/trap sources.
- **Still unknown (P0b answers empirically):** live spellId of Fear/Stun/grenade effects, vtbl[0x30] identity, whether the server list is populated for enemies, whether `+0xF8` is cleared on death.

## Phases (revised 10-17)
- **P0b probe fix** (hot-swap OK, mod is live): resolve the server creature per R3; relog on effect-list signature change (count + spell ids), still never per frame; log the +0xFFC cache entries; read Awareness through `stats+0x174` as a pointer; keep Verbose.
- **P1** logic + tests (`EnemyStripLogic.h`: tiers, obvious-state set, sort/cap, icon choice, row layout math). **P2** labels draw (BUILD pool, POST layout, drain overlay, optional pill backing `lbl_miscroll_f` like the combat hotbar). **P3** tune. **P4** install (hot-swap).

## Risks
- Panel-origin units/clamping are UNPROVEN (P0 logs them); wide/ultrawide may need scaling.
- New mod id => full reinstall, not hot-swap.
- Information level: Awareness gating keeps it from being a free "enemy status readout"; thresholds are a play-tuning item, not code.

## Acceptance
- Below `TierObvious`: zero visual change (byte-identical draw path, nothing drawn).
- Slowing an enemy at the right rank shows one icon above its bar that disappears when Slow expires; at `TierFull` the icon drains.
- No log spam (<50 lines on startup), no crash on null/dead/destroyed target.

- [x] P2 (completed)
