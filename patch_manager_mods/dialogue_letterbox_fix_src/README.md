# k2-dialogue-letterbox-fix 1.0.2 (STAGED, not installed)

Changelog: 1.0.2 = logging only: all lines now go through the shared `KpLog.h` (channels `main`, `set`; problem lines at WARN/ERR),
the own file logger, the `[GetTickCount]` prefix and the 5000-line cap are gone, and the load line is emitted by `KPatchInit` (or the
first hook call on an old core) instead of `DllMain`. Hooks, the constant write, the FOREIGN guard, the modes and the `log=0` first-event
gating are unchanged. The probe variant (`probe/`, version `1.0.2-probe`) is built from the same source: see "Probe variant" below.

Changelog: 1.0.1 = logging only. `log` now defaults to 0 (was 1): the DLL writes the load line, the first APPLY pair (header + outcome),
the first SETTOP and the first SETBOTTOM per process, and problem lines; `log=1` restores full logging. Hooks and the write logic are
unchanged from 1.0.0 (hooks file byte-identical).

Doc 15 Phase 2 S3, finding L1 (build doc: `../17_dialogue_letterbox_and_wrap_fixes.md`). Targets KOTOR II Steam Aspyr (sha256 6A522E71...).

## What / why

The dialogue letterbox bar height is `bar = trunc((H - trunc(W / c)) / 2)`, where `c` is a double in `.rdata` at VA
`0x009A7D30` (vanilla 2.3333330154418945, bytes `00 00 00 80 AA AA 02 40`), read by exactly 7 `FDIV` instructions and
nothing else. At 3840x1600, `W/c = 1645 > H`, so the bar is -22 and the letterbox vanishes. The fix writes
`c = 1.5*W/H` (times `1 - 1e-9`, see V7 Q5) whenever the screen size changes, so the bar is `H/6` at every resolution
(3840x1600 -> 267, 1920x1080 -> 180, 3440x1440 -> 240, 800x600 -> 100, 640x480 -> 80). Details: `engine_research_findings/V1.md`, `V7.md`.

## Hooks (all `detour`, cdecl, `exclude_from_restore = []`)

| address | window | function | param | role |
|---|---|---|---|---|
| 0x00411AC1 | 7 B `8B 55 F4 83 7A 7C 00` | `dlbOnScreenSize` | `ebp` (pointer) | inside `CSWGuiManager::SetScreenSize` (0x411A80) after both W/H shorts (+0x6C/+0x6E) are stored; manager = `[ebp-0xC]`. Reached only when W or H changed (startup, options Apply, credits 640x480 force + restore). Writes the constant. |
| 0x008BAD7C | 10 B `C7 81 88 00 00 00 01 00 00 00` | `dlbLogSetTop` | `ecx` (pointer) | last store of `CSWGuiDialogLetterbox::SetTop`; log-only (+ one-shot LATE fallback if the SetScreenSize hook was never seen). Mode `+0x88` shows the value before this store (the handler runs before the stolen bytes: KPM `wrapper_x86.cpp` executes them after restoring state). |
| 0x008BAF01 | 10 B `C7 81 88 00 00 00 02 00 00 00` | `dlbLogSetBottom` | `ecx` (pointer) | same for `SetBottom`. |

Letterbox layout used by the log: start rect +0x04 (x,y,w,h), target rect +0x68 (x,y,w,h), mode +0x88, GUI manager +0x1C.

## Config: `dialogue_letterbox_fix.ini` (game dir, read once at load; missing = defaults)

```
enabled=1        ; 0 = never write anything (log "DISABLED, no write")
mode=h6          ; h6 (default): c = 1.5*W/H*(1-1e-9), bar = H/6
                 ; clamp: only change c when the vanilla bar would be negative (c = W/H*(1-1e-9), bar 0), else vanilla
                 ; vanilla: keep/restore vanilla c
log=0            ; 0 (default) = the load line, the first APPLY pair (header + wrote/unchanged/DISABLED), the first SETTOP and
                 ; the first SETBOTTOM per process, and problem lines (INVALID/FOREIGN/protect/verify/null manager);
                 ; 1 = full logging (every APPLY, every SETTOP/SETBOTTOM up to 400 lines)
```
Spaces, CRLF and lines starting with `;` or `#` are tolerated; an unknown mode is logged and treated as `h6`.

Safety guard: the DLL writes 0x9A7D30 only if its current 8 bytes are the vanilla bytes or the last value this DLL wrote.
Anything else is logged as `FOREIGN` (another mod owns the constant) and left alone.

## Log (KpLog.h, 1.0.2)

On a core with the `kpatch.log` service, lines go to `k2-dialogue-letterbox-fix_log.txt` with the core prefix `T=<ms> f=<frame> k2-dialogue-letterbox-fix/<channel> `
once `kplog.ini` (game dir) enables them:

```
[global]
enabled=1
[k2-dialogue-letterbox-fix]
enabled=1
level=info
channels=main,set        ; probe variant: main,set,probe,probe_w4
```
Channels: `main` = load line, APPLY header + result lines, LATE notice; `set` = SETTOP/SETBOTTOM and the 400-line cap note.
Problem lines (INVALID size, FOREIGN, unknown mode, null manager = WARN; not readable, VirtualProtect FAILED, verify MISMATCH = ERR) need no `kplog.ini`.
`log=` in the INI is still the content gate described above (log=0: only the load line, first APPLY pair, first SETTOP/SETBOTTOM and problems).
On an old core (no service, `KPatchInit` never called) the header's fallback sink writes `dialogue_letterbox_fix_log.txt` at INFO, each line prefixed
`<ms> k2-dialogue-letterbox-fix/<channel> ` instead of 1.0.1's `[GetTickCount] `. Message text below is unchanged from 1.0.1.

- `k2-dialogue-letterbox-fix 1.0.2 loaded: enabled=1 mode=h6 log=0 const@0x009a7d30=<8 bytes> (<double>) pid=N`
- `APPLY src=<SetScreenSize|LATE(SetTop)|LATE(SetBottom)> W= H= mode= old= new= vanillaBar= newBar= X= fs= panelH= replyY=`
  followed by `wrote (protect RW ok, old prot=...; protect restore ok -> ...); verify OK, now <bytes> = <double>`,
  or `unchanged`, `DISABLED, no write`, `INVALID size, no write`, `FOREIGN value ...: another mod owns ..., not writing`.
- `SETTOP` / `SETBOTTOM lb= mode(+0x88...)= start(x,y,w,h)= target(x,y,w,h)= mgr= W= H= c=` (max 400 lines per session; with log=0 only the first of each).

## Build

```
cd ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/K2DialogueLetterboxFix && python3 ../create-patch.py
```
(mingw i686, links only KERNEL32 + msvcrt; `KpLog.h` comes from `Patches/Common`). The sources in this directory are byte-identical mirrors of that folder.

## Probe variant (`probe/`, 1.0.2-probe)

Same `K2DialogueLetterboxFix.cpp`; `probe/dlb_variant.h` (`#define DLB_PROBE 1`) is found through `__has_include`, so the probe build must have
`probe/` on its include path (the shipping dir has no `dlb_variant.h`). The probe build uses `probe/manifest.toml`, `probe/exports.def` (KPatchInit + 3 + 5 handlers) and
`probe/kotor2-steam-aspyr.hooks.toml` (3 shared hooks + W1-W5). It adds INI `probe=1|0` (default 0), the channels `probe` (PROBE W1/W2/W3/W3b/W5) and `probe_w4`
(PROBE W4), and splits `PROBE W3` into `PROBE W3` + `PROBE W3b` (same `n=`) to stay under the core's 232-byte line cut. Spec and expected signals:
`../dialogue_letterbox_fix_probe_src/README.md` (the 1.0.0-based predecessor, left untouched). Same patch id as the shipping build: never install both.

## Smoke test (Wine, no game)

```
mkdir -p /tmp/claude-1000/l1_101_smoke && cd /tmp/claude-1000/l1_101_smoke
unzip -o -j <Patches>/K2DialogueLetterboxFix/K2DialogueLetterboxFix.kpatch binaries/windows_x86.dll
i686-w64-mingw32-g++ -O1 -static-libgcc -static-libstdc++ -o smoke.exe <this dir>/smoke_dialogueletterbox.cpp
export WINEPREFIX=/tmp/claude-1000/l1_101_smoke/prefix WINEDEBUG=-all
for m in h6 clamp vanilla disabled foreign unchanged late ini log0 default; do
  "$HOME/.local/share/Steam/steamapps/common/Proton - Experimental/files/bin/wine" smoke.exe $m
  cp dialogue_letterbox_fix_log.txt log_$m.txt; done
```
The harness maps 0x9A7D30 as ordinary memory of a 32-bit exe at base 0x400000 (7 MB .bss pad), calls the three handlers
with a fake frame / fake manager / fake letterbox, and checks bars, page protection restore, and log lines. Exit 0 = pass.
NOTE: the smoke harness below still targets the 1.0.1 log shape (`[ms] text`); it has not been ported to the fallback prefix yet. Since 1.0.1 the modes that check full logging write `log=1` into the INI; `log0` (INI `log=0`) and `default` (no INI) check the new
log=0 output (first APPLY pair, first SETTOP/SETBOTTOM, INVALID problem line; a second apply and a second dialogue are not logged).
Window check: `python3 engine_bug_research/tools/check_engfix_windows.py`.

## Rollback

Set `enabled=0` in `dialogue_letterbox_fix.ini` (the DLL then never writes; the bytes in `.rdata` are vanilla again after
the next game start), or reinstall without the id `k2-dialogue-letterbox-fix`. STAGED only: not installed in the game.
