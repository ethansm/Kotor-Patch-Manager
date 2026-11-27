# Queue HUD (`queue-hud`)

> **STATUS** Track: SHIPPABLE. Version 0.4.13-dev (matches manifest.toml).
> **What it does / explores:** Shows the vanilla combat action-queue panel out of combat, so a use-item or cast-spell command given while paused stays visible while it waits. Buffers those commands, draws per-member queue strips and a Cancel control.
> **Requires:** Steam/Aspyr KOTOR II only (target hash in manifest.toml `[patch.supported_versions]`); KPM with the `portrait-slots` (`k2.portraitslots@1`) and `hud-host` (`k2.hud@1`) providers.
> **Known limits:** Steam/Aspyr only, no GOG address map. Cancel controls and the leader panel are out of combat only; left-click on a companion strip icon also works in combat. Items (medpacs) queued for a companion still wait. Per-node cancel address (RQ12) unverified. Hook DB backing unconfirmed (see Hooks). **Testing:** see Testing below.
> **Install:** normal play via KPM install, together with `portrait-slots` and `hud-host`. `queue_hud.ini` is optional.

Shared kit art (`qh_frame`, `kit_frame`) comes from `Common/HudKit.h`. Shared headers used: `KpLog.h`, `SafeRead.h`, `ActQueue.h`, `K2Addr.h`, `Party.h`, `HudKit.h`.

## Features
- Leader queue panel out of combat: up to 4 icons, the head icon dims while it waits (paused, round busy or delayed).
- Buffers use-item and cast-spell commands given while paused (hooks on `PerformUseItem` and `PerformCastSpell`).
- Companion strips for party members 1-2, above, below or left of the portrait card.
- Cancel control: a wide "CANCEL QUEUE" bar under the leader queue, and a control left of each companion strip. A click clears that member's queue (`CServerExoApp::ClearAllActions`).
- Left-click on one companion strip icon removes only that action (0.4.11, also in combat).
- Out of combat, a stray no-target attack the engine parks in front of a queued power/item is removed, so the power/item runs first (0.4.12, `strip_stale_attack`).
- In combat, the engine's own queued auto-attack moves behind a power/item the leader clicks, so the click runs next; attacks you click yourself keep their place (0.4.13, `auto_attack_last`).
- Combat hand-off: the buffered queue goes to the engine when combat starts. Commands the engine wipes while paused are re-injected after unpause.

## Requirements and install
1. Install `portrait-slots` and `hud-host` through KPM.
2. Install `queue-hud` through KPM.
3. Optional: copy `queue_hud.ini` next to `swkotor2.exe`. Every key is optional; without the file the built-in defaults apply.

## Configuration (`queue_hud.ini`)
Key summary.

| Key | Built-in default | Purpose |
|---|---|---|
| `enabled` | 1 | Master switch for the out-of-combat panel |
| `kit` | 1 | 1 = embedded `qh_frame` art; 0 = dark teal skin |
| `cancel` | 1 | Cancel controls and companion strip click-to-remove |
| `companions` | 1 | Companion strips (party members 1-2) |
| `strip_side` | left | `left`, `above` or `below` |
| `strip_max` | 4 | Icons per strip, 1..4 |
| `companion_server_cast` | 1 | QL20: companion Force powers run while another member leads |
| `pause_reinject` | 1 | Re-inject commands the engine wiped while paused |
| `strip_stale_attack` | 1 | QL26: out of combat, remove the engine's no-target attack in front of a queued power/item |
| `auto_attack_last` | 1 | QL27: in combat, the leader's engine-queued auto-attack moves behind a clicked power/item (the attack already swinging still finishes; companions untouched) |
| `handoff_on_combat` | 1 | Hand the buffered queue to the engine at combat start |
| `hide_banner_soft` / `hide_redbar_soft` | 1 | Hide the top banner / red bar in soft (ally-only) combat |
| `log` | 1 | Edge-triggered log lines (`kplog.ini [queue-hud]`) |
| `trace_queue` / `trace_clicks` | 1 / 1 | Per-command trace and one CLICK line per press (needs `log`) |
| `mode_cap` | 2000 | Max MODE lines per session |

Colours are `r,g,b` floats 0..1. A bad value or unknown key is skipped and counted in one startup WARN.

## Logging
- Log file: `queue-hud_log.txt` in the game folder (KpLog fallback file, INFO level).
- `kplog.ini` section `[queue-hud]` has channels `main`, `cap`, `icon`, `draw`, `cancel`, `trace`.

## Hooks
Steam/Aspyr only, from `kotor2-steam-aspyr.hooks.toml`:
- H9 `PerformUseItem` at `0x00775603` (detour, 6 bytes).
- H10 `PerformCastSpell` at `0x00775703` (detour, 6 bytes).

Both addresses are used only by this mod. Their DB backing is not yet confirmed: the address lookup returned no result in the static RE pass. No GOG map exists.

## Build
From this folder run `python3 ../create-patch.py`. Host unit tests (Common contract and queue logic) live in the author's development workspace and are not shipped here.

## Testing
- Live-tested on Steam: queued Force powers replay correctly.
- Still needs an in-game check: the mode banner, per-icon strip cancel, companion strip cancel, out-of-combat leader panel and the combat handoff.

Treat these as untested in any upstream PR text.

## Known issues
- Medpacs and other items queued for a companion are held (QL20 covers Force powers only).
- Cancel on a companion's strip: whether the engine drops the late 3D follow action is open.
- Hook addresses need DB backing before an upstream PR.

## Changelog
- No CHANGELOG.md.
