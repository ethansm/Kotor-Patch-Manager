# Mod Idea 3: Force Power Hotbar (1-0 keybound, customizable)

**Status:** Idea captured 2026-09-29, not yet researched. No Ghidra work
done for this mod — this doc exists so the idea and its open questions
survive a context clear, same purpose as `01_inventory_3d_viewport.md` and
`02_buff_duration_hud.md` served before their own Phase 2 work started.
Phase 1 (GUI/keybind research), Phase 2 (Ghidra), Phase 3 (hook/C++), and
Phase 4 (install/verification) are all explicitly out of scope until
someone picks this up.

## Goal

A row of hotbar buttons across the bottom of the screen, one per available
Force power, bound to keys **1-0** (10 slots) and **customizable** (the
player assigns which power sits in which slot). This replaces the
friction of the game's existing single quickbar item + scroll-wheel power
selection with direct one-key access — no scrolling to cycle to the power
you want mid-fight.

## Why this is a Patch Manager task

The existing quickbar-plus-scroll-wheel mechanic and the 2DA-driven power
list are native engine/NWScript territory, but a *new persistent row of
clickable/keybound buttons*, each independently customizable and each
needing its own icon/state render, is a new GUI/input surface the engine
doesn't have a slot for today — same category of "impossible without a
raw engine hook" as the other two mods in this doc set. Two engine-level
pieces are needed that nothing in NWScript/2DA/GFF can provide:
1. **Input**: binding physical keys 1-0 to "invoke the power assigned to
   this slot," independent of (or in addition to) however the existing
   quickbar's own input handling works.
2. **Render + persistence**: drawing 10 buttons with correct icons, and
   remembering per-player slot assignments across sessions (an in-memory-
   only solution would reset every relaunch, which isn't "customizable"
   in any durable sense).

## Relationship to the Buff Duration HUD mod ([[02_buff_duration_hud.md]])

The user's own framing, captured verbatim for whoever resumes this:
there's a natural temptation to make an active hotbar button *change
color* to show its power is still active/on cooldown (reusing exactly the
active-effect data the Buff Duration HUD mod's Target 1 research already
resolved — `CGameEffect::GetExpiryTime`, Steam `0x005e4fa0`,
`CGameEffect+0x10`/`+0x14`, filtered by spell ID per the
`02_buff_duration_hud.md` effect-ID table). **Decision (user, this
session): keep the two mods' UX separate for v1.** Don't merge the
cooldown-wipe visual into the hotbar buttons themselves. Instead:
- The hotbar mod's own hook, when a slot is pressed, invokes the power
  through whatever call the existing quickbar/action-bar system already
  uses (see "Open questions" below — this needs to be found, not
  reinvented).
- The Buff Duration HUD mod's **existing, separately-hooked** render pass
  near the party portraits (Target 2, `FUN_007457e0`, per
  `02_buff_duration_hud.md`) picks up the resulting active effect
  automatically, because it already reads the same live active-effect
  list — no direct hotbar-to-buff-HUD hook wiring is needed, they're
  decoupled by construction as long as both read from the same
  `CGameEffect` active-effect array. If a later session wants the
  combined "color the hotbar button too" UX, it would layer on top of
  this decoupled baseline rather than requiring it.

## Implementation pipeline (rough, pre-Ghidra)

### 1. Data layer — the power list
Likely reuses `02_buff_duration_hud.md`'s already-solved approach: read
the **installed** `spells.2da` via `pykotor` (`.venv` in this worktree),
not a hardcoded vanilla table — this build's mods (TSLRCM, Thematic Sith
Lords/Jedi Masters, mods #8/#102-109/#117-119/#139-142) add/change force
power rows, so "available Force powers" must be read from the live
install, and further filtered to whatever powers the *player's own
character* actually knows (a class/level-gated subset, not every row in
the 2DA) — that per-character "known powers" list is a new research target
this mod needs that Buff Duration HUD didn't (Buff Duration HUD only
needed *active effect* filtering, not *known/learnable* power lists).

### 2. Input layer — 1-0 keybinding
- Needs a hook into the game's keyboard input dispatch to claim 1-0 (or
  detect when they're free vs. already bound to something else — KOTOR2's
  default control scheme should be checked first, this build may already
  use some of 1-0 for something).
- The already-evaluated-and-skipped `ExpandedKeyboardControl` GOG-port
  candidate (see `06_gog_to_steam_conversions.md` — skipped because
  "nobody plans to use [the HUD] without a mouse") is a plausible
  **prior-art reference** for how this framework's patches generally
  claim new keybinds, even though that specific patch itself was never
  ported — worth reading its hooks.toml/source once this mod starts real
  research, rather than inventing an input-hook pattern from scratch.

### 3. Render layer — 10 buttons
- The existing **action bar** (`CSWGuiMainInterfaceAction`, GOG-named,
  7-entry array at `CSWGuiMainInterface+0x733c`, stride `0x750` —
  confirmed real on Steam via `02_buff_duration_hud.md`'s Phase 2 Target 2
  research, methods `Draw`/`Update`/`SetIcon`/`HitCheckMouse`/`Show`/
  `SetQueuedFlash`/`Initialize`) is the closest existing native precedent
  for "a row of icon buttons with click/hotkey handling and per-slot
  state" — worth decompiling as a reference/reuse target before building
  a new button widget from scratch, same "don't build a new draw
  primitive if the engine already has one" principle the Buff Duration
  HUD mod's own R7 open item follows. Note the action bar is 7 slots, not
  10 — this mod needs 10, so either extend that structure or build a
  sibling one modeled on it.
- Must respect the ultrawide HUD coordinate conventions already
  established ([[kotor2-ultrawide-gui]], `HUD_DESC_COEFF`) — a new
  full-width bottom bar is exactly the kind of element likely to need
  ultrawide-specific placement math, not a raw 800x600 assumption.

### 4. Customization + persistence
**Open question, not yet researched at all**: where do per-slot power
assignments live so they survive a relaunch? Options to evaluate once
Ghidra research starts:
- Piggyback on an existing per-character save-compatible field (needs
  finding an unused/repurposable slot — risky, could collide with a
  different mod or a future patch).
- A separate flat config file the DLL reads/writes itself, outside the
  game's own save system entirely (simpler, no save-compat risk, but
  "customization" then lives per-install rather than per-save-game — decide
  if that's acceptable before assuming it away).
- No persistence (session-only) — simplest, but doesn't meet "customizable"
  in the sense the user described; listed only as a fallback if the other
  two prove infeasible.

## Open questions for whoever picks this up

1. What function(s) does the existing single-quickbar-item + scroll-wheel
   mechanic call to actually invoke a power? (Likely somewhere in the
   `HandleButtonAction`/`QueueActiveAction`/`DoPersonalAction` family
   already named in the GOG address DB for `CSWGuiMainInterface` — see
   `02_buff_duration_hud.md`'s Phase 2 findings for the query technique.)
   This is the fastest way in: reuse the invocation call rather than
   reversing the power-cast pathway from scratch.
2. Does this build already bind 1-0 to anything (item quickslots, party
   selection, etc.)? Check before assuming the keys are free.
3. What determines a character's *known* powers (class + level-gated
   list), as opposed to Buff Duration HUD's simpler "is this specific
   effect currently active" question? New research, not reusable from the
   other mod as-is.
4. Persistence mechanism (see above) — genuinely open, no leaning yet.

## Effort estimate (rough, pre-Ghidra)

Likely **higher** than Buff Duration HUD: needs new input-hook research
(neither existing custom mod has claimed a new keybind), a new "known
powers per character" data path (not just "active effects," which Buff
Duration HUD already solved), and a real customization/persistence
mechanism (Buff Duration HUD has no analogous problem — it only reads,
never needs to remember player choices). The render side has a stronger
head start than Buff Duration HUD's Target 2 did, given the action bar's
7-slot icon-button widget is already a confirmed-real, closely-analogous
existing native precedent.
