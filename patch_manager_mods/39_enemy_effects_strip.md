# 39 — Enemy Effects Strip (target effect icons, Awareness-gated)

## Status
- **Mod id (proposed):** `enemy-effects-strip` · source `enemy_effects_strip_src/` · Steam Aspyr `6A522E71…` (live exe = vanilla+4GB)
- **State: PLAN (2026-10-12).** Nothing built. Phase 0 is a probe that logs and draws nothing.
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
- U1: **Which engine code draws the target's name + vitality bar** (screen position, rect source, per-frame hook point). Candidates: `SetMainInterfaceTarget 0x004dee10`, `UpdateTargetMenu 0x0074CFE0`, `CSWGuiMainInterface` target label/bar controls. Needs RE (static first via `query_ghidra.py`, `rea-recon` if the decomp is missing).
- U2: Can the mod read the leader's Awareness rank? Expected on the creature stats struct; find the skill-rank offset (use `query_ghidra.py struct`).
- U3: Are enemy effect durations/creators populated client-side the same way as for party members? (Probe logs raw fields.)
- U4: Cost with many enemies: draw only for the *selected* target.

## Phases (small, probe first)
- **P0 Probe (no drawing)** — 0.1.0-probe. Hook the target change; log target id, effect list (type, spellId, duration, creator, expiry), leader Awareness rank, and the screen rect of the name/vitality bar if found. Throttle: log on target change only. Verify with `python3 scripts/kotor2_logs.py check enemystrip_log.txt --new`.
- **P1 Logic + tests** (offline, no game): `EnemyStripLogic.h` — tier gating, obvious-effect list, sort/cap, icon choice; `enemy_strip_logic_test.cpp`; `common_contract_test.cpp` pinning the Common APIs used (per repo convention).
- **P2 Draw** — one row above the bar, static icons (tier 1-2), then drain overlay (tier 3). Release-quiet logging (`Verbose=0`).
- **P3 Polish/tune** — thresholds, MaxIcons, spacing, widescreen/ultrawide placement check (doc: ultrawide GUI), interaction with the action menu.
- **P4 Install** — gate, `kpm_hotswap` or full install if hooks are new (a new hook means a **full KPM reinstall, needs user GO**).

## Test aids (no enemy spawn needed for logic)
- Offline: P1 tests with fake effect lists.
- Live: console `runscript` (EnableCheats=1 is set in swkotor2.ini; unproven) with a script that applies `EffectSlow` / casts spells 141-143, 201, 269 at a hostile target, or use any combat. Awareness rank can be raised with a forged save (see `kotor2-forced-state-testing`).

## Risks
- The name/vitality bar may be a 3D-projected label, not a GUI control; then placement needs the projection math (more work, still feasible).
- A new hook => full reinstall, not hot-swap.
- Information level: Awareness gating keeps it from being a free "enemy status readout"; thresholds are a play-tuning item, not code.

## Acceptance
- Below `TierObvious`: zero visual change (byte-identical draw path, nothing drawn).
- Slowing an enemy at the right rank shows one icon above its bar that disappears when Slow expires; at `TierFull` the icon drains.
- No log spam (<50 lines on startup), no crash on null/dead/destroyed target.
