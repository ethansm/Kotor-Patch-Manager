# k2-dialogue-letterbox-fix 0.1.0 (STAGED, not installed)

> **STATUS** Track: SHIPPABLE. Version 0.1.0 (matches manifest.toml). The DIAGNOSTIC probe build is separate and not shipped here (version 0.1.0-probe).
> **What it does / explores:** Rewrites the letterbox aspect constant at 0x009A7D30 on every GUI size change, so the dialogue bars are H/6 tall on ultrawide (vanilla bars vanish at 3840x1600).
> **Requires:** Steam/Aspyr KOTOR II exe (sha256 6A522E71...) and a Patch Manager build with the kpatch core (`KpLog.h` from `Patches/Common`). `requires = []`.
> **Known limits:** no host test suite (Wine smoke only); hook sites 0x00411AC1, 0x008BAD7C, 0x008BAF01 not yet verified against a Ghidra address lookup. **Testing:** never played live.
> **Install:** normal KPM patch install (id `k2-dialogue-letterbox-fix`). Not installed in the live game yet. Never install together with the probe build (same id).

Changelog: 0.1.0 (current) = logging only: all lines now go through the shared `KpLog.h` (channels `main`, `set`; problem lines at WARN/ERR),
the own file logger, the `[GetTickCount]` prefix and the 5000-line cap are gone, and the load line is emitted by `KPatchInit` (or the
first hook call on an old core) instead of `DllMain`. Hooks, the constant write, the FOREIGN guard, the modes and the `log=0` first-event
gating are unchanged. The probe variant (version `0.1.0-probe`) is built from the same source; see "Probe variant and smoke test" below.

Earlier drafts: the 1.0.0 / 1.0.1 / 1.0.2 labels were README-only draft numbers; the manifest and code say 0.1.0.
Draft 1.0.1 = logging only. `log` defaulted to 0 (was 1): the DLL writes the load line, the first APPLY pair (header + outcome),
the first SETTOP and the first SETBOTTOM per process, and problem lines; `log=1` restores full logging. Hooks and the write logic are
unchanged from draft 1.0.0 (hooks file byte-identical).

Targets KOTOR II Steam Aspyr (sha256 6A522E71...).

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

## Log (KpLog.h, 0.1.0)

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
`<ms> k2-dialogue-letterbox-fix/<channel> ` instead of the draft 1.0.1 `[GetTickCount] `. Message text below is unchanged from the draft 1.0.1.

- `k2-dialogue-letterbox-fix 0.1.0 loaded: enabled=1 mode=h6 log=0 const@0x009a7d30=<8 bytes> (<double>) pid=N`
- `APPLY src=<SetScreenSize|LATE(SetTop)|LATE(SetBottom)> W= H= mode= old= new= vanillaBar= newBar= X= fs= panelH= replyY=`
  followed by `wrote (protect RW ok, old prot=...; protect restore ok -> ...); verify OK, now <bytes> = <double>`,
  or `unchanged`, `DISABLED, no write`, `INVALID size, no write`, `FOREIGN value ...: another mod owns ..., not writing`.
- `SETTOP` / `SETBOTTOM lb= mode(+0x88...)= start(x,y,w,h)= target(x,y,w,h)= mgr= W= H= c=` (max 400 lines per session; with log=0 only the first of each).

## Build

Set once (no machine paths in this file):
- `KPM_DIR` = your kotor-patch-manager checkout (contains `Patches/Common`)
- `STAGE` = output dir for the shipping build, e.g. `$KPM_DIR/letterbox_stage/0.1.0`

Shipping (writes `$STAGE/K2DialogueLetterboxFix.kpatch`; mingw i686, links only KERNEL32 + msvcrt; `KpLog.h` comes from `Patches/Common`):
```
cd "$KPM_DIR/Patches/K2DialogueLetterboxFix" && python3 ../create-patch.py --targets windows_x86 -o "$STAGE"
```

## Probe variant and smoke test

A log-only diagnostic probe build (0.1.0-probe, windows W1-W5) and a Wine smoke harness exist in the author's development workspace. They are not shipped here. Never install the probe alongside the shipping build.

## Rollback

Set `enabled=0` in `dialogue_letterbox_fix.ini` (the DLL then never writes; the bytes in `.rdata` are vanilla again after
the next game start), or reinstall without the id `k2-dialogue-letterbox-fix`. STAGED only: not installed in the game.
