# k2-dialogue-letterbox-fix 1.0.1-probe (log-only probe build; STAGED, not installed)

**SUPERSEDED by 1.0.2-probe** (`../dialogue_letterbox_fix_src/probe/`, built by `build_probe.sh`, on KpLog.h and based on 1.0.1). This dir and stage B (`stage_lbprobe_1003_1529`) stay as the T1 fallback until 1.0.2-probe has run once live.

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

## What this probe build adds (1.0.1-probe)

Same patch id `k2-dialogue-letterbox-fix`, so installing it REPLACES 1.0.0. The three 1.0.0 hooks and all 1.0.0 behaviour are unchanged
(the `loaded:` line gains ` probe=N` after `log=N`). Five more detour hooks, all log-only (`exclude_from_restore = []`, the wrapper
FXSAVE/FXRSTORs x87/SSE; the handlers do no float work, floats are logged as raw bits). Design: `../engine_research_findings/V8b.md` (c).

| window | address / stolen bytes | handler | param | meaning |
|---|---|---|---|---|
| W1 | 0x008BB461, 6 B `8B 45 CC 8B 48 10` | `dlbProbeW1` | `ebp` | letterbox Draw (0x8BB260), top bar, after SetExtent |
| W2 | 0x008BB52B, 6 B `8B 4D CC 8B 51 08` | `dlbProbeW2` | `ebp` | same, bottom bar |
| W3 | 0x008BB538, 9 B `D9 45 08 D9 1C 24 8B 4D CC` | `dlbProbeW3` | `ebp` | just before `CALL 0x40EC90` (base panel Draw); steals an x87 FLD/FSTP pair |
| W4 | 0x0040ED9F, 5 B `83 C4 1C 85 C0` | `dlbProbeW4` | `ebp`, `eax` (int) | base `CSWGuiPanel::Draw` right after the fill call; HOT (every panel every frame): the handler returns at once if probe=0, then if `*(void**)[ebp-0x38] != 0x009A7CBC` |
| W5 | 0x007CEB95, 7 B `83 7D 08 01 0F 94 C2` | `dlbProbeW5` | `ebp` | `CGuiInGame::SetGlobalDialogState` (0x7CEB80) |

All game memory is read through `IsBadReadPtr`-guarded copies (a failed read prints `[UNREADABLE]`/`[SOME-UNREADABLE]` instead of crashing).

### INI key and how to turn it off

`dialogue_letterbox_fix.ini`: `probe=1|0`, DEFAULT `0` (also off if the key is missing). With `probe=0` the five handlers return on their first
instruction and write nothing: the log is identical to 1.0.0 (apart from the `1.0.1-probe` version text and `probe=0` in the `loaded:` line).
`log=0` also silences the PROBE lines (probe lines need `probe=1` AND `log=1`). To drop the probe entirely: set `probe=0`, or reinstall
1.0.0 (`Patches/K2DialogueLetterboxFix`).

### Log lines (`dialogue_letterbox_fix_log.txt`, prefix `PROBE Wn n=<record no. since the last SETTOP> skip=<records dropped since the previous line of this window>`)

- `PROBE W1/W2 ... this= mode(+0x88)= done(+0x8C)= alpha(+0x50)=0x<float bits> passedRect(x,y,w,h)= ctrlRect(x,y,w,h)= targetH(+0x74)=`
  passedRect = rect just handed to SetExtent (`[ebp-0x18..-0xC]` W1, `[ebp-0x28..-0x1C]` W2), ctrlRect = `[this+4..+0x10]` after SetExtent.
- `PROBE W3 ... vtbl= flags(+0x48)= bit2= bit6= rect= alpha= color(+0x54..)= mgr= W= H= predicate=WOULD-DRAW|FAIL:<first failing term> dt=<bits> DAT_a309b4= DAT_a309b8= DAT_a309bc=`
  predicate = base-Draw gate in V8b order: bit6, x>=0, y>=0, w>0, h>0, x+w<=W, y+h<=H (W/H = shorts at `[[this+0x1C]+0x6C/0x6E]`).
- `PROBE W4 ... this= mode= eax=<fill result> fillRect(x,y,w,h)= alpha=` (only for the letterbox panel).
- `PROBE W5 ... arg=<[ebp+8]> this=<[ebp-0x24]>`.

Rate limit: counters reset at every SETTOP; per window the first 120 records are written every frame, then at most one line per second; global
cap 3000 PROBE lines per process (one `PROBE cap (3000 lines) reached` notice). Total log cap stays 5000 lines.

### Expected signals at 3840x1600, h6 mode, live semi-transparent-letterbox (copied from V8b (c))

- W1: per frame while the top bar slides: mode 1, rect h 8, 16, ... (60 fps) up to 267, control rect `[this+4..+0x10]` equal to it, then one record with
  done = 1 (`[this+0x8C]` = 1 is written at 0x8BB446 BEFORE this hook on the last frame) and h 267, alpha `0x3F000000`. Expected 34 records at 60 fps.
  No W1 record at all = Draw not called on the top bar (panel not in the manager list). Records with h stuck at 0 = step 0 (fps >= 535) or stale done flag.
- W2: y 1600 -> 1333 in -8 steps, control h = 1600 - y after SetExtent (`[this+0x10]`).
- W3: `vtbl=0x009a7cbc(letterbox)`, WOULD-DRAW with h 262..267 (after SetReply) and alpha 0x3f000000 (0.5), colour (0,0,0). A `FAIL:` term names the base-Draw
  gate that would skip the bar. DAT_a309b4 = top-bar h, DAT_a309b8 = bottom-bar y, DAT_a309bc = last SetGlobalDialogState arg == 1.
- W4: `eax` = 1 for the bars once W3 says WOULD-DRAW; 0 would point at the fill's `DAT_00a32a30` switch or display-size reject. This answers "is a fill issued" directly.
- W5: arg 1 once per conversation start after the SETTOP lines, arg 0 at the end.
- Vanilla A/B (`mode=vanilla` or `enabled=0`): no top bar (no W1/W3 WOULD-DRAW for the top), a 200 px bottom band.

## Build

The built probe lives in the KPM root stage dir `letterbox_probe_stage/`, NOT in `Patches/` (moved 2026-10-03, doc 20d F1). It reuses
the id `k2-dialogue-letterbox-fix` on purpose (installing it replaces 1.0.0), and KPM's `tools/validate-patches.py` FAILs on two
catalog dirs with the same id, so a `Patches/K2DialogueLetterboxFixProbe/` dir must not stay in the catalog. To rebuild:
```
K=~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager
mkdir $K/Patches/K2DialogueLetterboxFixProbe
cp exports.def K2DialogueLetterboxFix.cpp kotor2-steam-aspyr.hooks.toml manifest.toml $K/Patches/K2DialogueLetterboxFixProbe/
cd $K/Patches/K2DialogueLetterboxFixProbe && python3 ../create-patch.py
mv K2DialogueLetterboxFixProbe.kpatch $K/letterbox_probe_stage/ && cd .. && rm -r K2DialogueLetterboxFixProbe
```
(mingw i686, links only KERNEL32 + msvcrt). This directory is the source of record (the smoke harness and this README live only here).
The staged kpatch is sha256 `0d490cd4cf32fd65934b9dfec061b8cd0682d761710e268930c1db9aa9fd5954` (built 10-03 01:58).

## Smoke test (Wine, no game)

```
mkdir -p /tmp/claude-1000/l1_smoke && cd /tmp/claude-1000/l1_smoke
unzip -o -j <KPM>/letterbox_probe_stage/K2DialogueLetterboxFixProbe.kpatch binaries/windows_x86.dll
i686-w64-mingw32-g++ -O1 -static-libgcc -static-libstdc++ -o smoke.exe <this dir>/smoke_dialogueletterbox.cpp
export WINEPREFIX=/tmp/claude-1000/l1_smoke/prefix WINEDEBUG=-all
for m in h6 clamp vanilla disabled foreign unchanged late ini log0 probe probe0 probelog0; do
  "$HOME/.local/share/Steam/steamapps/common/Proton - Experimental/files/bin/wine" smoke.exe $m; done
```
The harness maps 0x9A7D30 as ordinary memory of a 32-bit exe at base 0x400000 (7 MB .bss pad), calls the three handlers
with a fake frame / fake manager / fake letterbox, and checks bars, page protection restore, and log lines. Exit 0 = pass.
Window check: `python3 engine_bug_research/tools/check_engfix_windows.py` (output saved as `engine_bug_research/tools/check_engfix_windows_output_2026-10-03.txt`).
The `probe` mode puts a fake letterbox (vtable value 0x9A7CBC), manager (W/H shorts 3840/1600) and two ebp frames at game VAs 0xA20000+ in the pad, plus DAT_00a309b4/b8/bc, and checks every field of the five PROBE lines, the W3 predicate terms, the W4 vtable filter and guarded reads, the 120/1-per-second rate limit, the SETTOP reset and the 3000-line global cap. `probe0` checks that nothing PROBE is ever written and the 1.0.0 lines still are; `probelog0` that probe=1 with log=0 is silent.

## Rollback

Set `enabled=0` in `dialogue_letterbox_fix.ini` (the DLL then never writes; the bytes in `.rdata` are vanilla again after
the next game start), or reinstall without the id `k2-dialogue-letterbox-fix`. STAGED only: not installed in the game.
