# V-C runbook: live voice probe E1 / E2 / E8 / E7 / E4 (doc 41 section 6)

State: **host prep done, NOTHING installed or run.** Every step below needs the preconditions in section 0. Each step is one command, one expected result, one pass rule. Ladder source: `.claude/research/voice_audio/w3/b8.md`. Time: about 2.5 h live for E1-E8; this runbook covers E1, E2, E8, E7, E4 (about 115 min).

Assets (all built by this phase):
- Clips: `patch_manager_mods/kvoice/kvoice_ematrix.py` (13 files, `kpv*` prefix, synthetic 1 kHz tone, -17 LUFS).
- Scripts: `patch_manager_mods/voice_probe_src/nss/*.nss`, compiled `nss/build/*.ncs` (nwnnsscomp via `tools/compile.sh`, `check_ncs_arg_order.py` 0 flags). Recompile after any `.nss` edit.
- Native logger: `patch_manager_mods/voice_probe_src/` (`VoiceProbe`, one detour on `ExecuteCommandPlaySound` 0x0068EA70, prologue `55 8B EC 6A FF` checked by `check_hook_bytes.py`, switch `voice_probe.ini` `probe=0` by default, log `voice_probe_log.txt`).

## 0. Preconditions (all must hold; ask the user for each)
1. Explicit user GO for a live run, and the D0 live-run pause lifted (memory `feedback-pause-live-fix-runs`). Without both, stop here.
2. Game closed (`pgrep -f swkotor2` prints nothing).
3. **Trigger route (OPEN, blocks every step).** The scripts need something to run them. `python3 scripts/kpm_status.py` on 2026-10-13 lists no `AdditionalConsoleCommands` patch (doc 06:485 had `runscript` live on 2026-09-29, not targeted-tested), so the console route is not installed. Decide with the user before GO:
   - (a) reinstall/port the console patch and confirm `runscript kpv_e1` works with a no-op script first;
   - (b) add a native key trigger to `voice_probe_src` that runs the script (new code, new review);
   - (c) point the user at an already-live trigger if one exists.
4. E4 only: `p_bastila_rprty` (row 36878) is not at the top level of `StreamVoice/` or `StreamSounds/`. Confirm where the resref lives (BIF or subdir) before E4, else pick another row and edit `kpv_e4.nss`.
5. Probe DLL reaches the game through the normal route: `kotor2-kpm-install` skill (stage, `scripts/pre_install_gate.sh --mod voice_probe_src` must print `GATE PASS`, then `kpm_hotswap.py`). Set `probe=1` in the game-dir `voice_probe.ini` for the run, back to 0 after.

## 1. Build the clip stage (host only)
Command (vanilla source for E2d is any StreamSounds `.wav`, read-only; use a direct path, never a directory scan):
```
SS="$HOME/.local/share/Steam/steamapps/common/Knights of the Old Republic II/StreamSounds"
python3 patch_manager_mods/kvoice/kvoice_ematrix.py --out "$SP/ematrix" --sfx-prefix-from "$SS/A_ALIENGEN_ATK.wav"
```
Expected: `wrote 13 clips under .../Override (skipped: none)`. Pass: 13 files and `manifest.json`; `kpve2c.mp3` lists `R1`, `kpve3c.lip` lists `R7`, `kpve2d.wav` lists `R1`. Never commit the stage dir (E2d holds 470 vanilla bytes).

## 2. Copy to the game (cleanup is step 9)
```
OVR="$(readlink -f "$HOME/.local/share/Steam/steamapps/common/Knights of the Old Republic II/Override")"
cp "$SP/ematrix/Override/"* "$OVR/"
cp patch_manager_mods/voice_probe_src/nss/build/kpv_*.ncs "$OVR/"
```
Expected: 13 clips + 9 `.ncs` = 22 new files, all prefixed `kpv`/`Kpv`. Pass: `ls "$OVR" | rg -ic '^kpv'` prints 22.

## 3. Forge a save
`kotor2-forced-state-testing` skill: NEW removable slot only, ini switch per the skill. Pass: slot loads to a controllable PC with at least one creature nearby (E4, E7 need one).

## 4. E1 (15 min): loose WAV in Override plays?
Run `kpv_e1`, then `python3 scripts/kotor2_logs.py check <log> --new` (never raw `cat`).
Pass: tone audible AND `kpv:` message AND `PLAYSOUND call #1` in `voice_probe_log.txt`. Fail splits: log line but silent = engine called, stream voice does not search Override (route A dead, go to StreamVoice staging). No log line = trigger or hook broken, fix that first.

## 5. E2 (30 min): container matrix
Run `kpv_e2a`, `kpv_e2b`, `kpv_e2c`, `kpv_e2d` one at a time (each audible or silent).
Pass rule per variant: audible = that container plays from Override (a PCM, b 58-byte+MP3, c bare MP3, d 470-byte prefix+PCM). Record all four, then persist; a silent variant is a result, not a failure of the run. Optional follow-up if b is silent in Override: repeat in `StreamVoice/` (staged via the canonical dir only, not a symlink walk).

## 6. E8 (15 min): case and name length
Run `kpv_e8` (plays `KpvE8Mixed` at 0 s, 16-char at 3 s, 17-char at 6 s).
Pass: record which of the three are audible. Expected on case-sensitive Proton: mixed-case file may not match the lowercased lookup; the 17-char name is over the 16-char resref limit and should be silent.

## 7. E7 (30 min): concurrency
Run `kpv_e7a` (3 at once), then `kpv_e7b` (8 at once). Positional part (1 m vs about 900 units) needs the PC moved between runs: repeat `kpv_e7a` at both distances.
Pass: record how many voices are audible and the `PLAYSOUND` line count (3 and 8 expected; fewer audible than logged = a concurrency cap).

## 8. E4 (25 min): BarkString route
Stand near a creature, run `kpv_e4`.
Pass: vanilla line audible AND overhead text "Let's move together again." drawn. Record whether text drew with audio absent (and vice versa). E3 (lip) and E6 (precedence) wait for the E1 result; no scripts exist for them yet (question in `nss/README.md`).

## 9. Cleanup (every run, even a failed one)
```
rm -f "$OVR"/kpv* "$OVR"/Kpv*
ls "$OVR" | rg -ic '^kpv'          # expect 0
```
Then set `voice_probe.ini` `probe=0`, delete the forged slot per the forced-state skill, and uninstall the probe DLL if the user wants the live set back to baseline. Pass: zero `kpv*` files, `kpm_status.py` shows no `voice-probe`.

## 10. Result table (fill in, then persist to doc 41 section 6 and memory)
| ID | Variant | Audible | `PLAYSOUND` line | Verdict / unlocks |
|---|---|---|---|---|
| E1 | kpve1a.wav PCM | | | |
| E2a | PCM | | | |
| E2b | 58-byte + MP3 | | | |
| E2c | bare MP3 | | | |
| E2d | 470 prefix + PCM | | | |
| E8 | KpvE8Mixed | | | |
| E8 | kpve8_sixteenchr | | | |
| E8 | kpve8_seventeench | | | |
| E7 | 3 at once (near / far) | | | |
| E7 | 8 at once | | | |
| E4 | strref 36878 | | | |

Suggest `/plan-handoff 41:V-D` only after the E1 result is recorded.
