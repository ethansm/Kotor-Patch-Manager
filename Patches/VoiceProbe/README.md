# Voice Probe

> **STATUS** Track: DIAGNOSTIC. Version 0.2.0 (matches manifest.toml).
> **What it does / explores:** Logs one line per `Global::ExecuteCommandPlaySound` call (voice/PlaySound test ladder). Since 0.2.0, an opt-in hotkey runs one named script with the PC as OBJECT_SELF.
> **Requires:** npc-world provider (`k2.npcworld@1`, `Patches/K2NpcWorld`); KOTOR II Steam (Aspyr) LAA build, sha256 `6A522E71...`; Kotor Patch Manager.
> **Known limits:** Steam/Aspyr only. Not standalone. Engine glue in `VoiceProbe.cpp` (tick handler) has no host test. VM global `0x00A1B4A8` has no DB row yet. Both the probe and the trigger default off.
> **Testing:** not yet played in game.
> **Install:** do not install unless testing voice scripts. Not for normal play.

## Features
- **Probe (log only):** one line per `ExecuteCommandPlaySound` call (call number, tick, `this`) to `voice_probe_log.txt`. The sound resref is not read here; the script side logs it via `SendMessageToPC`.
- **Trigger (0.2.0):** on a fresh key press (game in foreground), runs one NCS resref named in `voice_probe.ini` with the PC as OBJECT_SELF. Polled from the npc-world tick.

## Configuration (`voice_probe.ini`)
- `[probe] probe=0`: 0 = off (default), 1 = log every PlaySound entry.
- `[trigger] key=0`: Win32 virtual-key code, decimal. 0 = off (default). Example Ctrl+F8 is `key=119 ctrl=1`.
- `[trigger] ctrl=1 shift=0`: required modifiers.
- `[trigger] cooldown_ms=1000`: minimum gap between fires.
- `[trigger] script=`: NCS resref (1-16 chars `[A-Za-z0-9_]`, no extension). Empty = off. The script must be loadable in the current module.

## Logging
- Needs a `[voice-probe]` section in `kplog.ini` on a current core (`VoiceProbe.cpp:13-14`). Without it, lines fall back to the old core format.
- Output: `voice_probe_log.txt` (capped). Each trigger fire writes one `TRIGGER #n ... ret=` line.

## Hooks
- `0x0068EA70` `Global::ExecuteCommandPlaySound`: hooked by this mod (Steam Aspyr entry in `kotor2-steam-aspyr.hooks.toml`). DB-verified.
- `0x006FD8D0` `CVirtualMachine::RunScript`: called only, not hooked. DB-verified.
- `0x00A1B4A8` VM global (`CVirtualMachine*`): used as a constant. No DB row yet; the same value is used in `Patches/CompanionAI`.

## Build
From this folder run `python3 ../create-patch.py`.

Host unit tests for the trigger logic and shared headers live in the author's development workspace and are not shipped here.

## Scripts
- `nss/`: developer probe scripts for the voice test ladder. Compiled outputs in `nss/build/`. Not part of the install. See `nss/README.md`.
- `nss/hb/k_hen_heartbt01.nss`: retired TEST override of the henchman heartbeat. Do not stage it.

## What was learned
- See `nss/README.md` for the voice test ladder scripts.

## Changelog
- None yet. Version history is in `manifest.toml` `description`.
