# Ranged AI (Kiting)

> **STATUS** Track: SHIPPABLE. Version 0.4.2 (matches manifest.toml).
> **What it does / explores:** Party ranged companions hold position instead of walking into melee when the line of fire is blocked, and retarget like the player does. Adds party stance 16 "Ranged (Kiting)" to the stance menu.
> **Requires:** Steam/Aspyr KOTOR II (manifest `supported_versions`), Kotor Patch Manager with the KpLog core. Stance 16 also needs the TLK strings and party AI scripts below. The DLL alone gives hold and retarget only.
> **Known limits:** Stance 16 menu name and message need the TLK append. The party AI scripts and the stance-16 2da are built with author tools that are not shipped in this fork. Hook addresses are author RE, not yet DB-backed. Distance thresholds are not configurable (only the kite_approach flag).
> **Testing:** the default-attack and ranged-hold behaviours still need an in-game check.
> **Install:** Install through KPM (provider `k2.rangedai@1`). Then the stance-16 data steps below. Safe for normal play once installed.

## Features
- Line-of-fire hold: a ranged companion whose shot is blocked ends its attack action instead of walking to melee range (`RangedAiHold`, `0x006D83EA`).
- Retarget: on the second blocked check at the same position, the companion picks another target, as the player does.
- Stance 16 "Ranged (Kiting)": approaches like Aggressive until in weapon range, then shoots. Only with a ranged weapon; melee-weapon companions approach normally.

## Configuration (`ranged_ai.ini`, copy next to `swkotor2.exe`)
Re-read at most once a second; no restart needed.

| Key | Default | Meaning |
|---|---|---|
| `hold` | 1 | Hold position when the shot is blocked |
| `retarget` | 1 | Retarget on the second blocked check |
| `hold_stances` | 16 | AI stances the hold applies to |
| `retarget_stances` | 16 | AI stances the retarget applies to |
| `ranged_rules` | 1 | Stance 16 gets vanilla Ranged's "do not walk to target" rule |
| `kite_approach` | 1 | 0 = 0.2.0 rule; 1 = approach like Aggressive when out of range |
| `kite_strref_msg` | 123710 | Message strref on selecting stance 16 (vanilla fallback) |
| `kite_icon` | ib_ranged | Stance menu / HUD icon (`Override/<name>.tga`) |
| `log` | 1 | Diagnostics to `ranged-ai_log.txt` (KpLog) |
| `log_all` | 0 | Also log non-party creatures (noisy) |
| `debug_log` | 1 | Decision lines (D) with reasons |

## Installation (stance 16 extras)
1. Install the mod through KPM and copy `ranged_ai.ini` next to `swkotor2.exe`.
2. Close the game. Append the stance-16 name and message strings to `dialog.tlk` (back it up first) with your own TLK tool, and note the new strrefs in a JSON file. The author's append tool is not shipped in this fork.
3. Build the stance-16 `aiscripts.2da` row using those strrefs and place the result at `Override/aiscripts.2da`.
4. Compile the 17 party AI scripts and install the `.ncs` outputs to the game `Override` folder.

## Logging
KpLog core: `ranged-ai_log.txt` in the game folder, one session per file. Older core: `ranged_ai_log.txt`.

## Hooks
`kotor2-steam-aspyr.hooks.toml` lists 7 hooks (including `0x77ACBC`, `0x77D8D4`, `0x74DBDD`, `0x6D850F`). The hook addresses are not yet in the address DB.

## Build
From this folder run `python3 ../create-patch.py`.

Host unit tests (logic header, contract and party-script tests) live in the author's development workspace and are not shipped here.

## Changelog pointer
Upstream PR text: `PULL_REQUEST.DRAFT.md`.
