# k2-dialogue-letterbox-fix 1.0.0 (STAGED, not installed)

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
log=1            ; 0 = only the load line and problem lines (INVALID/FOREIGN/protect/verify) are written
```
Spaces, CRLF and lines starting with `;` or `#` are tolerated; an unknown mode is logged and treated as `h6`.

Safety guard: the DLL writes 0x9A7D30 only if its current 8 bytes are the vanilla bytes or the last value this DLL wrote.
Anything else is logged as `FOREIGN` (another mod owns the constant) and left alone.

## Log: `dialogue_letterbox_fix_log.txt` (game dir, appended, `[GetTickCount] text`, max 5000 lines per process)

- `... loaded: enabled=1 mode=h6 log=1 const@0x009a7d30=<8 bytes> (<double>) pid=N`
- `APPLY src=<SetScreenSize|LATE(SetTop)|LATE(SetBottom)> W= H= mode= old= new= vanillaBar= newBar= X= fs= panelH= replyY=`
  followed by `wrote (protect RW ok, old prot=...; protect restore ok -> ...); verify OK, now <bytes> = <double>`,
  or `unchanged`, `DISABLED, no write`, `INVALID size, no write`, `FOREIGN value ...: another mod owns ..., not writing`.
- `SETTOP` / `SETBOTTOM lb= mode(+0x88...)= start(x,y,w,h)= target(x,y,w,h)= mgr= W= H= c=` (max 400 lines per session).

## Build

```
cd ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/K2DialogueLetterboxFix && python3 ../create-patch.py
```
(mingw i686, links only KERNEL32 + msvcrt). The sources in this directory are byte-identical mirrors of that folder.

## Smoke test (Wine, no game)

```
mkdir -p /tmp/claude-1000/l1_smoke && cd /tmp/claude-1000/l1_smoke
unzip -o -j <Patches>/K2DialogueLetterboxFix/K2DialogueLetterboxFix.kpatch binaries/windows_x86.dll
i686-w64-mingw32-g++ -O1 -static-libgcc -static-libstdc++ -o smoke.exe <this dir>/smoke_dialogueletterbox.cpp
export WINEPREFIX=/tmp/claude-1000/l1_smoke/prefix WINEDEBUG=-all
for m in h6 clamp vanilla disabled foreign unchanged late ini log0; do
  "$HOME/.local/share/Steam/steamapps/common/Proton - Experimental/files/bin/wine" smoke.exe $m; done
```
The harness maps 0x9A7D30 as ordinary memory of a 32-bit exe at base 0x400000 (7 MB .bss pad), calls the three handlers
with a fake frame / fake manager / fake letterbox, and checks bars, page protection restore, and log lines. Exit 0 = pass.
Window check: `python3 engine_bug_research/tools/check_engfix_windows.py`.

## Rollback

Set `enabled=0` in `dialogue_letterbox_fix.ini` (the DLL then never writes; the bytes in `.rdata` are vanilla again after
the next game start), or reinstall without the id `k2-dialogue-letterbox-fix`. STAGED only: not installed in the game.
