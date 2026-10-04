# 07b. Force Power Hotbar: logging migration map (logf/VLOG -> KpLog.h), 2026-10-03

Research only. Nothing in the mod, the fork clone or the live game was changed. This is the map the migration session follows; the guide it implements is `19f_kplog_migration_guide.md`, the reference migration is `mouse_probe_src/MouseProbe.cpp` lines 16-70.

Source studied: `force_power_hotbar_src/ForcePowerHotbar.cpp` (1411 lines, v1.0.0), `HotbarConfig.h`, `HotbarLogic.h`, `_shared_src/SafeRead.h`. Logger: `Patches/Common/KpLog.h` plus the core docs `docs/PatchRegistry.md` and `src/KotorPatcher/README.md` (fork clone).

## 0. Answers first

- **Row count: 146 call sites** (117 `logf`, 29 `VLOG`). How counted: `grep -cE '\blogf\(|VLOG\(' ForcePowerHotbar.cpp` prints 148 matching lines and `grep -oE` finds 149 occurrences (119 `logf(` + 30 `VLOG(`). Three occurrences are definitions, not sites: line 107 `void logf(...)`, and line 158 which holds both `#define VLOG(...)` and the `logf(__VA_ARGS__)` inside that macro. 149 - 3 = 146 occurrences, on 146 distinct lines (no line holds two sites). The table in section 1 has exactly 146 rows, checked by script against the grep line list.
- **Lines that can exceed 232 bytes** (message text only; the core's cut applies to the text after its `T= f= force-power-hotbar/<chan> ` prefix, verified in core tests): 278, 614, 819, 965, 1254, 1407. Near the limit (200-232): 409, 412, 1242. Heartbeat 1368 and H3 summary 1354 stay under 232 once the int fields are bounded (160 and 190 bytes; 283 and 245 only if every `%d` is counted at 11 digits), but both are split anyway (section 3). Addresses 1408 is 127 bytes, not a problem.
- **Repeaters found beyond the ones you listed:** 278 (AutoFill VLOG every refresh), 528 (game-window lookup every 120 frames forever), 764 (wheel subclass retried and logged every frame on failure), 731/483/500/509/706/719/725/738 (widget failures retried every refresh or frame), 140/144.
- **KpLog.h / core discrepancies:** listed in section 8 (8 items). The two that change the plan: `KpLog_HasCore()` is always false in `DllMain`, so the private log rotation cannot live there; and the 2000 lines/s cap covers INFO, DEBUG and TRACE but exempts WARN and ERR.

## Conventions used below

- Tag text stays first in every message (`grep "H3 build"`, `grep "slot Bottom"` keep working). Continuation lines that today start with spaces (`    Bottom1 = ...`, `    avail ...`) get a tag, because a multi-line or indented fragment would otherwise lose its identity once the core prefixes every line.
- Channel variables (all `KP_CHANNEL`, file scope, names lowercase and at most 12 chars):

```cpp
KP_LOG_PATCH("force-power-hotbar");                       // == manifest id
extern "C" void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); saferead::enableRing(true); }   // + exports.def line
KP_CHANNEL(chBuild,  "build");    // H3, tex
KP_CHANNEL(chCfg,    "cfg");      // cfg, persave
KP_CHANNEL(chInput,  "input");    // cap, click, hover, wheel
KP_CHANNEL(chEdit,   "edit");     // edit, picker
KP_CHANNEL(chSlots,  "slots");    // btn, readGroup, avail, slot, invoke, eff, buttons
KP_CHANNEL(chTarget, "target");
KP_CHANNEL(chTip,    "tip");      // tip, sound, img
KP_CHANNEL(chChar,   "char");     // character, creature
KP_CHANNEL(chGate,   "gate");     // gate, panels, modal, tick
KP_CHANNEL(chMain,   "main");     // banner, addresses, heartbeat; same name as the built-in channel KP_WARN/KP_ERR use (Channel() is idempotent per (patch, name); MouseProbe does the same)
```

- `exports.def` today lists 8 exports and no `KPatchInit`: add it and bump `manifest.toml` version (1.0.0 -> 1.0.1 or 1.1.0).
- Levels: WARN = failure a user must see in a crash report (no kplog.ini needed); INFO = today's normal `logf`; DEBUG = today's `VLOG`; TRACE = per-control rects (1034, 771) and per-member target lines (375).
- Rate-policy codes in the table: `EV` event-driven (bounded by user input or state change, a few lines per second at most); `ONCE` once per process; `BLD` once per HUD build; `CHG` change-only cache that already exists; `CHG+` change-only cache to ADD (key defined in section 2, tags R1..R6); `CAPn` an existing counter cap; `FRM` existing 120-frame throttle; `EVERY n` a `KP_LOG_EVERY` / time gate.
- Action codes: `keep` = only the macro changes; `reword` = text changes (indent removed, continuation tag added, field order); `split` = becomes several lines (section 3); `gate` = add a cache, a rate gate or a `KP_ENABLED` guard.
- Max length column: bytes of the expanded message text, worst case with these bounds: `%08x`/`%p` 8; `%d`/`%ld` 6 digits (2 for slot/group/flag-like values, 1 for booleans, 10 for the frame counter); `%lu` 10; `%lld` 20; item or power name 80 (`saferead::readExo` maxLen); `hb::Format(ref)` 100 (`item:` + 80 + `@lowhp:100:self`); character key 11 (`obj` + 8 hex); `IniPath` 22; slot key 7; cwd 260; tip text 290 (title 80 + type line 20 + force points 20 + range 16 + 3 wrapped description lines of 38 + `Not usable right now`). Rows flagged **>232** need a split; `(near)` is 200-232.

## 1. Call-site table (146 rows)

`line | old tag/text | channel | level | rate policy | max bytes | action`. "Level" with a parenthesis means the level depends on the outcome (`KP_LOGV(ch, ok ? KPLOG_INFO : KPLOG_WARN, ...)`).

| line | old tag / text (abbreviated) | channel | level | rate policy | max bytes | action |
|---|---|---|---|---|---|---|
| 140 | logf `readGroup g=%d: implausible count=%d base=%08x` | slots | WARN | CHG+ (g,count,base) [R1] | 54 | gate |
| 144 | logf `readGroup g=%d: entry %d at %08x unreadable, stopping` | slots | WARN | CHG+ (g,i,addr) [R1] | 57 | gate |
| 205 | VLOG `  panels: hud=%08x hudvt=%08x mgr=%08x modals=%d panels=%d` | gate | DEBUG | EV (gate edge) | 78 | gate (KP_ENABLED around logPanels body) |
| 206 | VLOG `    modal[%d]=%08x vt=%08x flags=%x` | gate | DEBUG | EV, <=16/edge | 49 | reword (drop indent) |
| 207 | VLOG `    panel[%d]=%08x vt=%08x flags=%x` | gate | DEBUG | EV, <=32/edge | 49 | reword (drop indent) |
| 216 | logf `creature: unavailable (%s)` | char | INFO | CHG (lastFail) | 40 | keep |
| 229 | logf `character: creature %p has invalid object id %08x (party m..` | char | INFO | CAP30 + CHG | 82 | keep |
| 240 | logf `cfg: wrote template %s` | cfg | INFO | ONCE | 42 | keep |
| 253 | logf `cfg: profile for character '%s' -> %d/%d slots bound` | cfg | INFO | EV (cfg load / char change / assign) | 69 | keep |
| 256 | logf `    %s = %s` | cfg | DEBUG | EV, <=10 per 253 | 114 | reword ("cfg:   BottomN = ..."), gate KP_ENABLED(DEBUG) replaces g_cfg.general.verbose |
| 278 | VLOG `eff: autofill group %d pick key=%08x '%s' ref=%s%s` | slots | DEBUG | CHG+ per group (key,dup) [R3]; today every 8 frames x3 groups when AutoFill=1 | 244 **>232** | gate + reword (drop name for items) |
| 288 | logf `eff: character '%s' autofill=%d explicit=%d auto=%d` | slots | INFO | CHG (effHash) | 68 | keep |
| 289 | logf `    Bottom%c = %s%s` | slots | INFO | CHG, <=10 per 288 | 121 | reword (tag "eff:   BottomN") |
| 309 | logf `cfg: cannot read %s (mtime=%lld, keeping previous)` | cfg | WARN | EV (mtime change) | 86 | keep |
| 311 | logf `cfg: loaded %s: invoke=%d verbose=%d kbdev=%d profiles=%d ..` | cfg | INFO | EV (mtime change) | 98 | keep |
| 314 | logf `cfg: TargetProbe=%d` | cfg | INFO | EV | 23 | keep |
| 315 | logf `cfg: WARNING %s` | cfg | WARN | EV, 1 per warning | 172 | reword (itemised "cfg: WARNING [i/n] ...") |
| 334 | logf `  invoke: hud selection not writable` | slots | WARN | EV (per invoke) | 36 | reword (drop indent, add group/sel/active) |
| 335 | VLOG `  invoke: pre group=%d key=%08x sel=%08x active=%d` | slots | DEBUG | EV (per invoke) | 62 | reword (drop indent) |
| 340 | logf `  invoke: returned group=%d key=%08x (sel before=%08x afte..` | slots | INFO | EV (per invoke) | 111 | reword (drop indent) |
| 341 | logf `  invoke: selection restored to %08x` | slots | INFO | EV (per invoke) | 40 | reword (drop indent) |
| 342 | logf `  invoke: selection changed by the game to %08x, NOT resto..` | slots | WARN | EV (per invoke) | 66 | reword (drop indent) |
| 353 | logf `target: object table unreadable (app=%08x), using it norma..` | target | WARN | EV (per use) | 65 | keep |
| 355 | logf `target: no server id for client item %08x, using it normally` | target | WARN | EV (per use) | 64 | keep |
| 357 | logf `target: item %08x (server %08x) not found, using it normally` | target | WARN | EV (per use) | 68 | keep |
| 360 | logf `target: base item unreadable (item=%08x base=%08x), using ..` | target | WARN | EV (per use) | 77 | keep |
| 363 | logf `target: slot item '%s' client=%08x server=%08x item=%08x t..` | target | INFO | EV (per use) | 172 | keep |
| 364 | logf `target: item type %d is self-only/direct in vanilla, using..` | target | INFO | EV (per use) | 74 | keep |
| 367 | logf `target: party members unreadable, using it normally` | target | WARN | EV (per use) | 51 | keep |
| 375 | logf `target:   [%d] client=%08x id=%08x server=%08x hp=%d/%d ok..` | target | TRACE | EV, <=3 per use | 101 | keep (TRACE) |
| 380 | logf `target: lowest-HP below %d%% (else %s) -> nobody` | target | INFO | EV (per use) | 56 | keep |
| 381 | logf `target: lowest-HP below %d%% (else %s) -> %d` | target | INFO | EV (per use) | 52 | keep |
| 382 | logf `target: TargetProbe=%d, not targeting (normal use on self)` | target | INFO | EV (per use) | 58 | keep |
| 383 | logf `target: nobody to use it on, skipped` | target | INFO | EV (per use) | 36 | keep |
| 385 | logf `target: (Invoke=0: would use on member %d)` | target | INFO | EV (per use) | 42 | keep |
| 388 | logf `target: member %d id/client unreadable (self=%08x member=%..` | target | WARN | EV (per use) | 89 | keep |
| 391 | logf `target: member %d position unreadable, using it normally` | target | WARN | EV (per use) | 56 | keep |
| 392 | logf `target: PerformUseItem(item %08x -> member %d id %08x) cal..` | target | INFO | EV (per use) | 69 | keep |
| 394 | logf `target: PerformUseItem(item %08x -> member %d id %08x) ret..` | target | INFO | EV (per use) | 71 | keep |
| 403 | logf `slot %s: unbound now (config changed?)` | slots | INFO | EV (per key) | 47 | keep |
| 406 | logf `slot %s: %s -> entry not available for '%s'; available now:` | slots | INFO | EV (per key) | 175 | reword (fields first; "available now:" dropped, 409 follows) |
| 409 | logf `    avail g%d.%d key=%08x %s "%s"` | slots | INFO | CAP20 per process | 213 (near) | reword (drop indent; omit name for items) |
| 412 | logf `slot %s: %s -> group %d index %d key=%08x flags=%08x usabl..` | slots | INFO | EV (per key) | 212 (near) | split (fmtSlotResolve, ref last) |
| 472 | logf `img: loaded '%s' -> %p (pool %d)` | tip | INFO | EV, once per resref (<=256) | 56 | keep |
| 473 | logf `img: pool full (%d), cannot load '%s'` | tip | WARN | CHG+ first + EVERY 10 s [R4] | 55 | gate |
| 483 | logf `btn[%d].%s: flags unreadable, cannot set visible` | slots | WARN | CHG+ (widget,kind) [R2] | 49 | gate |
| 489 | VLOG `btn[%d].%s key %c: %s -> %s (%s) flags %08x->%08x` | slots | DEBUG | EV (per widget change) | 95 | keep |
| 500 | logf `btn[%d].%s: border vtable %08x != %08x, not setting fill '..` | slots | WARN | CHG+ (widget,kind) [R2] | 84 | gate |
| 503 | VLOG `btn[%d].%s key %c: fill '%s' -> '%s' image=%p (old fill %0..` | slots | DEBUG | EV (per widget change) | 124 | keep |
| 509 | logf `btn[%d].%s: border vtable bad, not setting alpha` | slots | WARN | CHG+ (widget,kind) [R2] | 49 | gate |
| 510 | VLOG `btn[%d].%s key %c: alpha %.2f -> %.2f` | slots | DEBUG | EV (per widget change) | 39 | keep |
| 528 | logf `hover: game window %p client=%ldx%ld` | input | INFO | CHG+ (hwnd,w,h) + 3-try WARN [R5]; today every 120 frames while missing | 48 | gate |
| 543 | logf `tip: %s -> %s (cols=%d rows=%d%s%s)` | tip | INFO | ONCE (WARN if FAILED) | 127 | keep (level by ok) |
| 548 | logf `tip: %s -> %s (strings=%u%s%s)` | tip | INFO | ONCE (WARN if FAILED) | 111 | keep (level by ok) |
| 571 | VLOG `tip: spell row %d fp='%s' range='%s' desc strref='%s'` | tip | DEBUG | EV (per tip show) | 87 | keep |
| 582 | logf `tip: %s vtable unreadable` | tip | WARN | CHG+ (widget,kind) [R2] | 32 | gate |
| 594 | VLOG `tip: hide slot %d (%s)` | tip | DEBUG | EV | 56 | keep |
| 600 | logf `tip: show slot %d skipped: tooltip labels not bound (bg=%p..` | tip | WARN | CHG+ once per HUD build | 79 | gate |
| 603 | logf `tip: slot %d: text object bad (vt=%08x renderer=%08x), too..` | tip | WARN | CHG+ once per HUD build | 78 | gate |
| 614 | VLOG `tip: show slot %d lines=%d maxLen=%d rect=[%d,%d,%d,%d] cl..` | tip | DEBUG | EV (per tip show) | 424 **>232** | split (header + one line per tip text line; never embed \n) |
| 622 | logf `sound: manager null (%s), skipped` | tip | INFO | CAP20 (shared counter) | 63 | keep |
| 623 | VLOG `sound: hover (%s) mgr=%p` | tip | DEBUG | CAP40 (shared counter) | 60 | keep |
| 637 | VLOG `hover: slot %d -> %d cursor=(%ld,%ld)` | input | DEBUG | EV (hover edge) | 47 | keep |
| 638 | logf `hover: cursor=(%ld,%ld) over=%d` | input | DEBUG | FRM 120 frames + cursor moved | 37 | keep |
| 649 | logf `click: slot %d in %s mode -> handled by the editor, not in..` | input | INFO | EV (per click) | 67 | keep |
| 652 | logf `click: slot %d %s` | input | INFO | EV (per click) | 34 | keep |
| 653 | logf `click: ring full, dropped` | input | WARN | EV (per click) | 25 | keep |
| 667 | logf `buttons: no usable bar for hud %08x (bar panel=%08x built=..` | slots | WARN | CHG (lastMissingHud) | 136 | keep |
| 689 | VLOG `tip: slot %d entry changed while hovered, refreshing` | tip | DEBUG | EV | 52 | keep |
| 706 | logf `edit: %s flags unreadable` | edit | WARN | CHG+ (widget,kind) [R2] | 32 | gate + reword (use "tab0..2"/"cell3" names) |
| 713 | VLOG `edit: %s %s -> %s flags %08x->%08x` | edit | DEBUG | EV (per widget change) | 57 | keep |
| 719 | logf `edit: %s border vtable bad, not setting fill '%s'` | edit | WARN | CHG+ (widget,kind) [R2] | 70 | gate |
| 720 | VLOG `edit: %s fill '%s' -> '%s' image=%p` | edit | DEBUG | EV (per widget change) | 76 | keep |
| 725 | logf `edit: %s border vtable bad, not setting alpha` | edit | WARN | CHG+ (widget,kind) [R2] | 52 | gate |
| 726 | VLOG `edit: %s alpha %.2f -> %.2f` | edit | DEBUG | EV (per widget change) | 36 | keep |
| 731 | logf `edit: %s text object bad, cannot set "%s"` | edit | WARN | CHG+ (widget,kind) [R2]; today retried every frame in Pick mode | 110 | gate |
| 732 | VLOG `edit: %s text "%s" -> "%s"` | edit | DEBUG | EV (per widget change) | 157 | keep |
| 738 | logf `edit: %s text object bad, cannot set colour` | edit | WARN | CHG+ (widget,kind) [R2] | 50 | gate |
| 739 | VLOG `edit: %s text colour (%.2f,%.2f,%.2f) -> (%.2f,%.2f,%.2f)` | edit | DEBUG | EV (per widget change) | 70 | keep |
| 756 | VLOG `wheel: delta=%d client=(%ld,%ld) rect=[%ld,%ld,%ld,%ld] in..` | input | DEBUG | CAP40 | 87 | keep |
| 764 | logf `wheel: subclassed window %p, old proc %p (err=%lu)` | input | INFO (WARN if old proc null) | ONCE on success; CHG+ first 3 + EVERY 60 s on failure [R6]; today every frame on failure | 69 | gate + reword (level by g_oldProc) |
| 771 | VLOG `edit: %s moved to [%d,%d,%d,%d]` | edit | TRACE | EV (per placePanel) | 54 | keep (TRACE) |
| 786 | logf `edit: mode %s -> %s slot %d -> %d (%s) char='%s'` | edit | INFO | EV (mode change) | 95 | keep |
| 804 | logf `edit: assign REFUSED: config has %d warning(s); saving wou..` | edit | WARN | EV (per click) | 92 | split (header + one line per warning, sec 6) |
| 807 | logf `edit: assign REFUSED: character key unknown (load in progr..` | edit | WARN | EV (per click) | 63 | keep |
| 812 | logf `edit: assign REFUSED: entry '%s' has no stable ref` | edit | WARN | EV (per click) | 128 | keep |
| 813 | logf `edit: '%s' moved from slot %c to %c` | edit | INFO | EV | 111 | keep |
| 816 | logf `edit: backup %s -> force_power_hotbar.ini.bak: %s` | edit | INFO (WARN if FAILED) | ONCE | 73 | keep (level by ok) |
| 819 | logf `edit: assign slot %d (%s) := %s; froze %d autofilled slot(..` | edit | INFO (WARN if Save FAILED) | EV (per assign) | 251 **>232** | split (fmtAssign) |
| 830 | VLOG `edit: scroll %d -> %d (%s)` | edit | DEBUG | EV (per scroll step) | 64 | keep |
| 842 | logf `edit: Esc flag set in Normal mode (ignored)` | edit | INFO | EV | 43 | keep |
| 847 | logf `edit: foreground window %p pid=%lu ours=%d` | edit | INFO | CHG (lastFgOk) | 59 | keep |
| 872 | VLOG `edit: control hover %d -> %d` | edit | DEBUG | EV (hover edge) | 36 | keep |
| 892 | logf `edit: picker list tab=%s off=%d entries=%d (was %d) slot=%d` | edit | INFO | CHG (list signature) | 74 | keep |
| 896 | VLOG `edit: scroll offset %d clamped to %d (entries=%d)` | edit | DEBUG | EV | 61 | keep |
| 902 | VLOG `edit: wheel acc=%ld steps=%d offset %d -> %d (max %d)` | edit | DEBUG | EV (per wheel batch) | 72 | keep |
| 931 | VLOG `edit: cell hover %d -> %d` | edit | DEBUG | EV (hover edge) | 29 | keep |
| 937 | VLOG `edit: toggle hover %d -> %d` | edit | DEBUG | EV (hover edge) | 31 | keep |
| 958 | logf `edit: tab click %d (%s)` | edit | INFO | EV (per click) | 26 | keep |
| 959 | logf `edit: prev click ignored (at start)` | edit | INFO | EV (per click) | 35 | keep |
| 960 | logf `edit: next click ignored (at end)` | edit | INFO | EV (per click) | 33 | keep |
| 961 | logf `edit: Clear click slot %d` | edit | INFO | EV (per click) | 25 | keep |
| 963 | logf `edit: click on empty cell %d ignored` | edit | INFO | EV (per click) | 36 | keep |
| 965 | logf `edit: cell %d click -> slot %d := group %d key=%08x %s "%s"` | edit | INFO | EV (per click) | 239 **>232** | split (name/ref on 2nd line) |
| 969 | logf `edit: click at (%ld,%ld) in Pick mode hit nothing` | edit | INFO | EV (per click) | 55 | keep |
| 985 | logf `H3: window %p client=%dx%d ref=%dx%d -> x correction %.4f` | build | INFO | BLD | 82 | keep |
| 1002 | logf `H3 %s: allocation failed` | build | WARN | BLD, once per control | 40 | keep |
| 1003 | logf `H3 %s: ctor left vtable %08x (expected %08x), not created` | build | WARN | BLD, once per control | 81 | keep |
| 1004 | logf `H3 %s: template %s not in the HUD layout, control not crea..` | build | WARN | BLD, once per control | 88 | keep |
| 1005 | logf `H3 %s: panel control count unreadable, control not created` | build | WARN | BLD, once per control | 74 | keep |
| 1011 | logf `H3 %s: panel control count unreadable, control not created` | build | WARN | BLD, once per control | 74 | keep |
| 1019 | logf `H3 %s: text object bad, font/text/align not applied` | build | WARN | BLD, once per control | 67 | keep |
| 1022 | logf `H3 %s: template already has a fill, keeping it` | build | INFO | BLD, once per control | 62 | keep |
| 1030 | logf `H3 %s: append to the panel list failed (count %d -> %d), c..` | build | WARN | BLD, once per control | 97 | keep |
| 1034 | VLOG `H3 %s: created at index %d from %s rect=[%d,%d,%d,%d] fill..` | build | TRACE | BLD, 75 per build | 144 | keep (TRACE) |
| 1160 | logf `tex %s: embedded TGA not uncompressed 32-bit, skipped` | build | WARN | ONCE per texture | 67 | keep |
| 1161 | logf `tex %s: embedded TGA truncated (%dx%d, %u bytes)` | build | WARN | ONCE per texture | 78 | keep |
| 1162 | logf `tex %s: embedded TGA too large (%dx%d, limit %d), skipped` | build | WARN | ONCE per texture | 83 | keep |
| 1165 | logf `tex %s: engine malloc(%u) failed` | build | WARN | ONCE per texture | 54 | keep |
| 1174 | logf `tex %s: %s, %dx%d RGBA queued for upload (basic %p, pendin..` | build | INFO | EV (initial + heals <=20/tex) | 125 | keep |
| 1182 | logf `tex %s: allocation failed` | build | WARN | ONCE per texture | 39 | keep |
| 1191 | logf `tex %s (%s): gl=%u constructed=%d missing=%d pixels=%p %dx%d` | build | INFO | ONCE (frame 120) | 111 | keep |
| 1206 | logf `tex %s: rebuilt without our pixels (missing=%d w=%d), rese..` | build | WARN | CAP20 per texture (t.heals) | 85 | keep |
| 1236 | logf `persave: %s: save folder not recognised ('%s'), no snapshot` | cfg | WARN | EV (per save) | 167 | reword (fields first) |
| 1238 | logf `persave: %s: folder '%s' not found, no snapshot` | cfg | WARN | EV (per save) | 187 | reword (fields first) |
| 1242 | logf `persave: %s: hotbar -> '%s': %s (profiles=%d)` | cfg | INFO (WARN if FAILED) | EV (per save) | 217 (near) | split (fmtPersaveSnapshot) |
| 1254 | logf `persave: %s: %s -> live hotbar %s (%d profile(s), snapshot..` | cfg | INFO (WARN if not replaced) | EV (per load/new game) | 405 **>232** | split (fmtPersaveReplace + itemised warnings) |
| 1260 | logf `persave: %s: save folder not recognised ('%s'), live hotba..` | cfg | WARN | EV (per load) | 172 | reword (fields first) |
| 1286 | logf `cap: stuck modifier cleared (ctrl %d->%d alt %d->%d shift ..` | input | INFO | EV | 77 | keep |
| 1300 | VLOG `cap: dev=%d dik=%02x %s mods=%c%c%c -> %s` | input | DEBUG | EV (per digit/mod/Esc key event) | 52 | keep |
| 1329 | logf `H3 build: panel=%08x vtable=%08x controls=%d gff=%08x (pre..` | build | INFO | BLD | 112 | keep |
| 1330 | logf `H3: panel null/unreadable, abort` | build | WARN | BLD | 32 | keep |
| 1331 | logf `H3: replacing bar of old HUD %08x (its labels stay leaked,..` | build | INFO | BLD | 75 | keep |
| 1337 | logf `H3: projection flag %08x = %d (cleared for the binds, rest..` | build | INFO | BLD | 77 | keep (WARN variant for probe failure, sec 4) |
| 1354 | logf `H3: %d controls created at runtime (indices %d..%d), %d/%d..` | build | INFO | BLD | 190 | split (fmtH3Summary + fmtH3Rect) |
| 1357 | logf `H3: WARNING %d of %d controls NOT created (see the lines a..` | build | WARN | BLD | 71 | keep (append names of missing controls) |
| 1366 | logf `tick: gap of %lu ms since previous HUD tick (load/menu/HUD..` | gate | INFO | EV (per gap) | 77 | keep |
| 1368 | logf `heartbeat: frame=%ld bar(panel=%08x built=%d vis=%d%d%d%d%..` | main | INFO | EVERY 1800 frames (~30 s) | 160 | split (fmtHeartbeat + fmtHeartbeatBar) |
| 1369 | logf `tick: first HUD tick hud=%08x kbdev=%d` | gate | INFO | ONCE | 46 | keep |
| 1370 | logf `tick: hud pointer now %08x` | gate | INFO | CHG (g_hud) | 30 | keep |
| 1371 | logf `tick: hud %08x unreadable -> gate off` | gate | WARN | CHG (g_hudGateOk) | 41 | keep |
| 1380 | logf `gate: %s (%s)` | gate | INFO | CHG (gate,why) | 36 | keep |
| 1389 | logf `character: '%s' -> '%s'` | char | INFO | CHG (charKey) | 41 | keep |
| 1396 | logf `tick: gate closed, discarding %d queued request(s)` | gate | INFO | EV (once per discard) | 50 | keep |
| 1407 | logf `force-power-hotbar 1.0.0 loaded; cwd=%s pid=%lu ini=%s` | main | INFO | ONCE | 339 **>232** | split (fmtBanner + fmtChunks for cwd) |
| 1408 | logf `addresses: invoke=%08x kbdevGlobal=%08x app=%08x getCreatu..` | main | INFO | ONCE | 127 | keep (127 bytes) |

Totals by channel: edit 34, build 24, slots 21, target 17, tip 13, cfg 12, input 10, gate 9, char 3, main 3 (= 146). By level: INFO 65, WARN 49, DEBUG 29, TRACE 3. Of these, 49 sites are WARN (failures that must show without `Verbose=1`; levels counted by first word, so "INFO (WARN if FAILED)" counts as INFO), 3 are TRACE (1034, 771, 375), and 256 (the `Verbose`-gated `logf` profile dump) joins DEBUG.

## 2. Rate policy: sites that can repeat per refresh or per frame

How often each caller runs today: `updateButtons` re-resolves every `ButtonRefreshFrames` = 8 frames (and every frame while settling), each slot scans up to 6 groups through `resolve` -> `readGroup`; `updateEditor` runs `readGroup` for all 6 groups EVERY frame while the picker is open; `updateHover`/`updateEditor` run every HUD frame. A 60 fps HUD means up to about 450 `readGroup` calls per second in normal play and 360 in Pick mode.

| Tag | Sites | What repeats | Prescription |
|---|---|---|---|
| R1 | 140, 144 (+ new 147, 124) | `readGroup` implausible count / unreadable entry, on every call while the engine list is bad | Change-only cache per group: `struct GroupFault { int count = -2, base = 0; uintptr_t entry = 0; int idx = -1; int logs = 0; }; static GroupFault s_gf[7];`. 140 key = `(g, count, base)`; 144 key = `(g, i, e)`. Log (WARN) when the key differs from the cached one; clear the cache when the group reads cleanly so a later recurrence logs again; at most 5 logs per group per session (count the rest into the heartbeat `pfaults=`). No `KP_LOG_EVERY` here: its gate is per call site, so seven groups would share one gate. |
| R2 | 483, 500, 509, 582, 706, 719, 725, 731, 738 | Widget operation failures. Today the state is not updated on failure (`b.icon`, `w.text`, `w.alpha` stay old), so the same failing op is retried and re-logged every refresh (slot buttons) or every frame (editor title text 731 in Pick mode). 40 slot widgets x every 8 frames is the worst case. | Per-widget fault mask: add `unsigned failLogged = 0;` to `Widget` and a helper `bool firstFault(Widget& w, unsigned kind)` that returns true once per bit. Kinds: 1 flags, 2 fill, 4 alpha, 8 text, 16 colour, 32 rect, 64 vtable. Key = (widget instance, kind); `Bar()` is rebuilt in `HotbarBuildButtons`, so the mask resets with every HUD build. Log WARN once, include the widget name and `ctl` pointer; when the same op later succeeds clear the bit and log one DEBUG `recovered`. Count every suppressed fault in `g_bar.faults` (heartbeat `wfaults=`). Give picker tabs/cells distinct names (`tab0..2`, `cell0..7`, `cell3.qty`): today `nm` is shared (`"tab"`, `"cell.bg"`) so a log line cannot say which one failed. |
| R3 | 278 | `VLOG("eff: autofill ...")` runs for groups 1-3 inside every `refreshEffective`, which runs every 8 frames while `AutoFill=1`: about 22 DEBUG lines per second of identical text. | Cache `static unsigned lastPick[4]` = `key * 31 + dup` per group and log only on change. (The surrounding `logf` at 288 is already change-only through `g_effHash`.) |
| R4 | 473 | `img: pool full` for every new resref once the 256-entry pool is full (setFill/wFill call `imageFor` whenever the wanted icon differs). | First occurrence WARN, later ones coalesced: `static int hits; if (++hits == 1) KP_WARN("img: pool full (%d), cannot load '%s'", pool, resref); else KP_LOG_EVERY(chTip, 10000, "img: pool full (%d), %d loads refused so far, last '%s'", pool, hits, resref);`. |
| R5 | 528 | `cursorInGame` retries the window lookup every 120 frames forever when no window matches and logs `hover: game window 00000000 client=0x0` each time (about one line per 2 s). | Cache `(hwnd, w, h)` and log only when it differs from the last logged triple. Count consecutive misses; on the 3rd miss log one WARN `hover: game window not found after 3 tries (EnumWindows)`; then stay silent until found, and log an INFO `hover: game window found ...` on recovery. |
| R6 | 764 (and the guard at 762) | `ensureWheelHook` returns early only when `g_oldProc` is set. If `SetWindowLongPtrA` returns 0, it is called and logged again on every `updateEditor` frame. | Count attempts: attempts 1-3 log (WARN when `g_oldProc == 0`, with `GetLastError`), later attempts `KP_LOG_EVERY(chInput, 60000, "wheel: subclass still failing (attempts=%d err=%lu)", ...)`. Separate behaviour change (needs your OK, not part of logging): stop retrying after about 5 attempts. |
| R7 | 1380 | Already change-only on `(gate, gateWhy)`. A flapping gate (HUD on top <-> other panel on top) would log every frame. | Keep change-only; add a flap counter `gateFlaps` shown in the heartbeat. If more than 20 edges occur in 10 s, switch that site to `KP_LOG_EVERY(chGate, 1000, "gate: flapping (%d edges so far) now %s (%s)", ...)` until it settles. |

Sites that look repeating but are already bounded, so no change: 216 (`lastFail` pointer), 229 (`badLogs < 30` + change), 409 (`availLogs < 20` per process), 622/623 (`logs < 20/40`), 638 (120-frame throttle + cursor moved), 667 (`lastMissingHud`), 756 (`g_wheelLogs < 40`), 847 (`lastFgOk`), 892 (list signature), 1206 (`t.heals` cap 20), 1366, 1370, 1371. `checkConfig` (309/311/314/315) fires only on a changed mtime because `filewatch::changed` stamps the file on every `true` return, so an unreadable INI does not log every second.

## 3. Long lines (message text over 232 bytes) and the split

Prefix not counted: the core adds `T=<ms> f=<n> force-power-hotbar/<chan> ` and cuts the TEXT at 232 bytes ending in `[...]` (core tests `any_log_service.cpp:545-549`). `KpLog.h` itself cuts at 511.

| line | worst bytes | cause | proposed split (parser-relevant fields first) |
|---|---|---|---|
| 278 | 244 | item `ref` (100) + item name (80) are the same string for items | `eff: autofill group %d pick key=%08x ref=%s%s` and, for non-items only, append ` name='%s'`. Max about 180. |
| 614 | 424 | tip text (up to 290 with 6 embedded `\n`). Embedded `\n` is the real bug: `KpLog` splits on `\n`, so every text line would arrive as a separate, untagged line | line 1: `tip: show slot %d lines=%d maxLen=%d rect=[%d,%d,%d,%d] client=%ldx%ld textHeight=%d` (about 100). Then one line per text line from `fmtTipLines(text)`: `tip: text[%d] "%s"` (each at most 80 + 18). |
| 819 | 251 | `Format` value (100) + slot key + char key + trailer | line 1 `edit: assign slot=%d key=%s save=%s froze=%d mtime=%lld char='%s'` (about 95), line 2 `edit: assign value=%s` (at most 125; `none` on clear). |
| 965 | 239 | `Format` (100) + item name (80) + two keys | line 1 `edit: cell %d click -> slot %d := group %d key=%08x` (about 55), line 2 `edit: cell %d ref=%s` plus ` name="%s"` for non-items only. |
| 1254 | 405 | persave `source` string is `'<dir>/force_power_hotbar.ini' found` (dir up to 112, `cloudsaves/<17-digit id>/` + folder up to 80) | line 1 `persave: <why>: live hotbar replaced|NOT replaced profiles=%d snapshot_warnings=%d backup=ok|none` (about 110 with a 24-char why), line 2 `persave: <why>: source=%s` (at most about 210; drop the surrounding quotes), then one WARN line per snapshot warning (section 6). |
| 1407 | 339 | `cwd` is `MAX_PATH` (260) | line 1 `force-power-hotbar 1.0.0 loaded; pid=%lu ini=%s core=%d` (about 70; keep the first words, tools grep them), then `fmtChunks("cwd", cwd, 200)` = `cwd[0]=<200 bytes>`, `cwd[1]=<rest>`. |
| 409 (near, 213) | 213 | item `ref` + name | `avail g%d.%d key=%08x %s`; add ` "%s"` only for spells/modes. Max about 150. |
| 412 (near, 212) | 212 | `Format` (100) in the middle of the line | `fmtSlotResolve`: `slot Bottom1: group=%d index=%d key=%08x flags=%08x usable=%d INVOKING ref=%s` (`(Invoke=0)` instead of `INVOKING` when off). Ref last; max about 185. |
| 1242 (near, 217) | 217 | `dst` path up to 136 | line 1 `persave: <why>: save ok|FAILED profiles=%d`, line 2 `persave: <why>: dst=%s` (at most about 180). |
| 1354 (190, split anyway) | 190 bounded (245 if every int is 11 digits) | 11 ints plus the editor state | `fmtH3Summary`: `H3: %d/%d controls created (indices %d..%d) buttons=%d/%d editor=%d/%d -> editor ENABLED|DISABLED frame=%ld` and `fmtH3Rect`: `H3: picker background rect [%d,%d,%d,%d]`. Reason to split: new fields (disabled parts, missing control names, section 4) must fit, and `frame=` is one of the two places the hotbar's own frame counter stays in text. |
| 1368 (160, split anyway) | 160 bounded (283 if every int is 11 digits) | 10 `vis` ints printed as `%d` (a `-1` takes 2 bytes) | `fmtHeartbeat`: `heartbeat: frame=%ld gate=%d char='%s' ring=%d invoke=%d cfgMtime=%lld`; `fmtHeartbeatBar`: `heartbeat: bar panel=%08x built=%d vis=%s wfaults=%u pfaults=%u efaults=%u` with `vis` from `fmtVisMask` (one char per slot: `?` / `0` / `1`). |

Other long-ish lines checked and fine: 363 (172), 406 (175), 1236 (167), 1238 (187), 1260 (172), 315 (172, bounded by the 160-byte config message), 1408 addresses (127, not a suspect).

**Pure formatters for `HotbarLogic.h`** (only `<string>`, `<vector>`, `<cstdio>`, no Win32 types, so `hotbar_logic_test.cpp` can feed worst-case inputs and assert `size() <= 232`):

```cpp
namespace hb {
constexpr size_t LogMax = 232;
std::string fmtClip(const std::string& s, size_t max = LogMax);                 // same rule as the core: cut, end with "[...]"
std::vector<std::string> fmtChunks(const char* tag, const std::string& s, size_t chunk = 200);   // "tag[i]=<chunk>"
std::string fmtVisMask(const int* vis, int n);                                  // -1 '?', 0 '0', 1 '1'

struct HeartbeatInfo { long frame; unsigned panel; int built; int vis[10]; int gate; std::string charKey; int ring;
                       long long cfgMtime; int invoke; unsigned widgetFaults, probeFaults, exoFaults; };
std::string fmtHeartbeat(const HeartbeatInfo&);       // line 1 (frame first)
std::string fmtHeartbeatBar(const HeartbeatInfo&);    // line 2

struct H3Summary { int created, total, before, after, buttons, buttonCount, editorBound, editorCount; bool editorEnabled; long frame; int rect[4]; };
std::string fmtH3Summary(const H3Summary&);
std::string fmtH3Rect(const H3Summary&);

std::string fmtSlotResolve(const std::string& slotKey, int group, int index, unsigned key, unsigned flags, bool invoke, const std::string& refText);
std::string fmtSlotMiss(const std::string& slotKey, const std::string& refText, const std::string& charKey);     // "slot Bottom1: entry not available ..."

std::string fmtPersaveSnapshot(const char* why, bool ok, int profiles);          // "persave: <why>: save ok profiles=N"
std::string fmtPersaveDst(const char* why, const std::string& dst);
std::string fmtPersaveReplace(const char* why, bool replaced, int profiles, int snapWarnings, bool backupOk);
std::string fmtPersaveSource(const char* why, const std::string& source);

std::string fmtAssign(int slot, const std::string& slotKey, bool saveOk, int frozen, long long mtime, const std::string& charKey);
std::vector<std::string> fmtTipLines(const std::string& text);                   // "tip: text[i] \"...\"", one per '\n' piece
}
```

`common_contract_test.cpp` pins the Common APIs the mod uses; add the `KpLog.h` entry points used (`KP_LOGV`, `KP_LOG_EVERY`, `KpLog_HasCore`, `KpLog_SetFallbackFile/Level`) to the "force-power-hotbar relies on:" list per `feedback-common-contract-tests`.

## 4. New log lines for silent failure paths

Line numbers verified against the current file by content (drift noted where it exists). None of these may run per frame: every one is change-only, once per build, or once per session.

| # | Path (verified line) | Exact proposed message | Chan | Level | Policy |
|---|---|---|---|---|---|
| 1 | Projection flag read fails, line 1336 (`!probeRead ? 0 : *projFlag`). Today the failure and a real 0 both print `= 0` at 1337. | `H3: projection flag %08x unreadable (probeRead failed), treating it as 0` and change the 1337 line to `H3: projection flag %08x = %d read=%s (cleared for the binds, restored after)` with `read=ok` or `read=FAILED` | build | WARN (first line) / INFO (1337) | once per HUD build |
| 2 | Clear fails, 1338 (`savedFlag && !probeWrite`) | `H3: projection flag %08x not writable (probeWrite failed): controls bind with the engine's aspect projection, slots may be too wide` | build | WARN | once per HUD build |
| 3 | Restore fails, 1351 | `H3: projection flag %08x restore failed (probeWrite), saved=%d; the HUD ctor clears it at 0x0074850D anyway` | build | WARN | once per HUD build |
| 4 | `CaptureHotbarKey` wrong device, 1274 (`dev != g_kbDevice`; 1273 is the `!rec` guard). This returns for every mouse and other-device record, so it must stay silent per call. | Track a 32-bit mask of devices seen and two counters (`g_capMatched`, `g_capForeign`). First event from each new device (max 16 devices): `cap: first event from device %d (kbdev=%d), ignored`. From the tick (not the hook): when `g_frame >= 600 && g_capMatched == 0 && g_capForeign > 0`, once: `cap: no input event matched kbdev=%d after %u foreign events (devices seen mask=%08x); keys will not work, set KeyboardDevice in [General]` | input | INFO / WARN | per device once; WARN once per session |
| 5 | `updateEditor` not built, 837 (`panel != hud || built == 0 || !editBound`) | `edit: editor disabled panel=%08x hud=%08x built=%d editBound=%d (%s)` where `%s` is `bar panel is not this hud`, `no buttons built`, or `editor controls missing (%d/%d)` | edit | WARN when `panel == hud && built > 0 && !editBound`, else INFO (667 already WARNs for the first two cases) | change-only on `(panel == hud, built, editBound)` via a `static` key |
| 6 | `logPanels` unreadable manager, 202 | `gate: logPanels: manager unreadable hud=%08x` | gate | DEBUG | none needed (runs only on a gate-off edge) |
| 7 | Cursor failures, 531 (`!GetCursorPos || !ScreenToClient`) and 524 (window lost) | `hover: cursor unavailable (GetCursorPos=%d ScreenToClient=%d err=%lu)`; on recovery `hover: cursor available again`; when `g_gameWnd` exists but `!IsWindow`: `hover: game window %p gone, searching again` | input | WARN (first), INFO (recovery, window gone) | change-only on a `static bool lastCursorOk` and the window handle |
| 8 | `ensureWheelHook` no window, 762 (`!g_gameWnd || !IsWindow`) | `wheel: waiting for the game window (g_gameWnd=%p IsWindow=%d), picker wheel scrolling unavailable` | input | DEBUG | once per session; failure after window exists is R6 |
| 9 | Click edge without cursor, 953 (`!click || !haveCur`) | `edit: click ignored, no cursor position (gate=%d mode=%s)` (log only when `click && !haveCur`) | edit | INFO | `KP_LOG_EVERY(chEdit, 2000, ...)`: the click edge is rare, this only guards a stuck-true loop |
| 10 | `healTextures` give-up, 1203 (`t.heals >= 20 -> continue` is silent) | after each `++t.heals` (1204/1206): `if (t.heals == 20) tex %s: giving up after 20 failed refills (built=%d missing=%d pixels=%p); art missing, labels still work` | build | WARN | exactly once per texture (`heals == 20` is crossed once; reset only when healthy) |
| 11 | Icon probe fails, 147 (`probeRead(e+0x20, 16)` false, `en.icon` stays empty) | `readGroup: icon unreadable g=%d i=%d entry=%08x (icon left empty)` | slots | DEBUG | first 5 per process, then only counted (`iconFail` in heartbeat) |
| 12 | `readExoStr` fails, 124 (returns `""` for unreadable AND for rejected-but-readable names; `readExo` cannot say which) | `exo: CExoString rejected site=%04x addr=%08x` | slots (site `S_Entry`) / cfg (site `S_Save`) | DEBUG | first 5 per site (array indexed by `site - 0x4801`), then counted (`efaults`). Limitation: only the probe failures reach the saferead ring (section 5); plausibility rejects appear only here. |
| 13 | Tip widget flags unreadable, 586 (`setWidgetVisible` returns silently, also for `!w.ctl`) | `tip: widget flags unreadable ctl=%p, visibility not applied` (only when `w.ctl` is set; add a `name` parameter so it can say `tipbg` / `tip`) | tip | WARN | R2 mask, once per widget |
| 14 | `wTextColor` disabled, 736 (`!w.textRgbSet`) is by design for non-qty widgets | At build, in `createLabel` after 1038 for names containing `QTY`: count failures, then one line after the control loop: `H3: text colour readback failed for %d qty control(s), first=%s; colour writes disabled (out-of-stock 0 will not turn grey)` | build | WARN | once per HUD build |
| 15 | `!ctl` early returns, 482, 494, 508, 705, 717, 724, 730, 736 | Nothing per call (a disabled part would log forever). After the build loop (after 1356): `H3: disabled parts: slots=%d/%d editor=%s tooltip=%s` (`ok` or `off`) and, when anything is missing, `fmtChunks("H3: missing controls", names, 200)`, names comma-joined | build | WARN only if something is missing, else nothing | once per HUD build |
| 16 | `computeCharacterKey` null, 223 (`!c`); `controlledCreature` already logs a fail reason through 216 | only when `why == F_None` and `c == 0`: `character: controlled creature null with party count %d` | char | WARN | change-only (`static bool lastNull`) |
| 17 | `targetedUse` idx 0, 384 (`idx == 0` returns false silently) | `target: member 0 (the controlled character) chosen, normal use on self` | target | DEBUG | per use (event) |
| 18 | Resolve failure in the tick, 684 (`resolve(...)` false; the comment at 690 is not the code). Only the key-press path (406) logs today. | `slot %s: resolve miss ref=%s char='%s'`; on recovery `slot %s: resolve ok group=%d index=%d` | slots | INFO | change-only per slot: add `bool lastHave` to `Button`; skip while `settling` (the hide-settling branch at 686 is expected); reset on HUD rebuild |

## 5. SafeRead ring plan

`SafeRead.h` has a 64-entry ring (`g_ring`, `g_ringCount`, both `static` in `namespace saferead`, so visible inside this translation unit), `enableRing(bool)` (default off, so today `recordFail` returns immediately and nothing is ever kept) and `dump(buf, cap)` which formats the ring oldest-first, one entry per line: `  t=%lu site=%u addr=%08x len=%u caller=%08x`. Nothing in the mod calls either today (checked with grep for `enableRing` and `saferead::dump`).

1. **Enable:** `saferead::enableRing(true)` in `KPatchInit` (new core) AND in the `startOnce()` of section 7 (old core never runs `KPatchInit`). It is a plain bool store, idempotent, no I/O. Cost per failure is one `InterlockedIncrement` plus `GetTickCount`.
2. **Dump helper**, one WARN line per entry. Do not hand the whole dump to one `KP_WARN`: `KpLog::Write` formats into 512 bytes first (the full ring is about 4.8 KB) and only then splits on `\n`, so everything past the first few entries would be lost.

```cpp
void dumpRing(const char* why) {
    static int dumps = 0; static LONG lastCount = 0;
    LONG c = saferead::g_ringCount;
    if (c == lastCount || dumps >= 8) return;          // nothing new, or session cap (WARN is exempt from the core's rate cap)
    ++dumps; lastCount = c;
    static char buf[8192]; saferead::dump(buf, sizeof buf);   // 64 entries x about 75 B + header, fits
    KP_WARN("saferead: dump #%d reason=%s", dumps, why);
    for (char* p = buf; *p; ) {
        char* e = strchr(p, '\n'); size_t n = e ? (size_t)(e - p) : strlen(p);
        while (n && *p == ' ') { ++p; --n; }
        if (n) KP_WARN("saferead: %.*s", (int)n, p);   // each at most about 80 bytes; the header line already starts with "saferead:"
        p = e ? e + 1 : p + n;
    }
}
```
(The header line `saferead: N failed probes (showing last M)` would then read `saferead: saferead: ...`; strip it with a `strncmp(p, "saferead:", 9)` check in the real code.)

3. **When to call** (all on the game thread, never per frame): (a) end of `HotbarBuildButtons` when `created != ControlCount` or a projection probeWrite failed, reason `H3 failure`; (b) in the tick when `g_frame % 600 == 0` (about 10 s) and `g_ringCount - lastCount >= 25`, reason `25 new failures`; (c) on a gate-open to gate-closed edge (a quiet moment) when failures are new, reason `gate closed`; (d) from the `target:` WARN returns 353-394, reason `target`. Cap 8 dumps per session, so at most about 520 WARN lines.
4. **Shutdown: not recommended.** `DLL_PROCESS_DETACH` runs under the loader lock, under Wine in an unspecified order relative to the patcher DLL, and the core may already be torn down. Dump (c) and the 10-second rule (b) cover the last failures before an exit. If a final dump is wanted, do it from the fallback path only (`!KpLog_HasCore()`), writing with `fputs` to the fallback file.
5. Site ids in the dump are the numeric `Site` enum (`0x4801` `S_Hud` ... `0x4811` `S_WrProjRestore`); add the legend to doc 07 so `site=18433` is readable (`0x4801` = 18433).

## 6. Config warnings, itemised

- **`cfg: WARNING %s` (315)** already writes one line per warning; change it to `cfg: WARNING [%d/%d] %s` at WARN level, capped at 10 lines plus `cfg: WARNING (+%d more)`. The count line 311 (`warnings=%d`) stays, so a grep for `warnings=` still works.
- **Refused assign (804)**: header `edit: assign REFUSED: config has %d warning(s); saving would drop the lines they concern` (WARN, as today) followed by one WARN line per warning, `edit: assign REFUSED: cfg warning [%d/%d] %s`, cap 5 plus `(+%d more)`. The count alone does not tell the player which INI line to fix.
- **Persave snapshot warnings (1254, `snap->warnings`)**: today only `snapshot warnings=%d` is logged, AFTER the snapshot replaced the live INI (`n.Save`). Data is lost silently: `Parse` leaves the offending slot empty and `Serialize` never writes warnings back. Log the items BEFORE the `Save` at line 1251, from `replaceLiveProfiles` while `snap` is in scope: `persave: <why>: snapshot warning [%d/%d] %s` at WARN, cap 10.
- **`msg[160]` truncation risk (`HotbarConfig.h` line 296, the `char msg[160]` in `Parse`)**: every `snprintf(msg, sizeof msg, ...)` truncates silently at 159 bytes, with no `[...]` marker. The user-controlled parts (`key`, `val`, `section`) are unbounded. Worst cases:
  - `line %d: bad value '%s' for %s (slot left empty)` has 48 bytes of fixed text with a 6-digit line number (53 with 11 digits). It truncates when `len(val) + len(key) > 111`. A normal item binding does it: `item:<80-char name>@lowhp:100:self` is 100 bytes, plus the key 7 = 107, plus 48 = 155 (just fits); with a 6-digit line number and a longer name or target text it is cut. When it is cut, the tail goes first, and the slot key (`for %s`) sits after the value, so the player loses WHICH slot was bad.
  - `line %d: unknown key '%s' in [%s]`, `line %d: unknown section [%s]`, `line %d: %s '%s' must be an integer (kept default)`, `HealThreshold '%s' ...`, `HealFallback '%s' ...`: all unbounded for the same reason.
  - Safe for the logger: after the `cfg: WARNING [i/n] ` prefix a 159-byte message stays under 232, so truncation is the only risk, not the 232 cut.
  - Proposed fix (a `HotbarConfig.h` change, so a separate edit with `config_roundtrip_test.cpp` re-run): clip `key`, `val` and `section` to 60 bytes with a trailing `...` before formatting, move the key first (`line %d: %s: bad value '%s' (slot left empty)`), and widen to `char msg[200]`. Worst case then is about 175 bytes, always under 232 even with the logger prefix text `cfg: WARNING [10/10] `.

## 7. Fallback and no-core behaviour

- **Fallback file name must stay `force_power_hotbar_log.txt`** (underscores): `load_hang_probe_src/LoadHangProbe.cpp:505` tails exactly that name. Set it from a namespace-scope initialiser, like MouseProbe line 37: `const bool g_fallbackSet = (KpLog_SetFallbackFile("force_power_hotbar_log.txt"), true);` (stores the path, no I/O, safe in a static initialiser). On a new core the per-patch file is `force-power-hotbar_log.txt` (hyphens, from the id), so LoadHangProbe will not find it: follow-up for that mod, tail both names.
- **`KpLog_HasCore()` exists** (`KpLog.h` line 411: `gApi != nullptr`).
- **Private rotation (line 1405, `DeleteFileA(LogPrevPath); MoveFileA(LogPath, LogPrevPath);`) only when `!KpLog_HasCore()`: it cannot stay in `DllMain`.** `KpLog_Init` runs from `KPatchInit`, which the registry core calls before the first hook, so inside `DllMain` `KpLog_HasCore()` is false even on a new core; the check there would always rotate. Also the banner (1407/1408) logs from `DllMain` today; on an old core that would open the fallback file (kept open by the header, `fopen "a"`) before the rotation, and a `MoveFileA` on an open file fails. So: remove the banner and the rotation from `DllMain`, keep only the wheel-proc restore, and run both from `startOnce()` at the top of every exported hook (8 of them):

```cpp
void startOnce() {
    static bool done = false; if (done) return; done = true;
    saferead::enableRing(true);
    if (!KpLog_HasCore()) {                                   // old core: behave exactly like 1.0.0
        KpLog_SetFallbackLevel(KPLOG_INFO);                   // header default is WARN: INFO lines would be dropped
        DeleteFileA(LogPrevPath); MoveFileA(LogPath, LogPrevPath);   // before the first fallback write
    }
    // banner lines (1407/1408 split, section 3) go here, via KP_LOG(chMain, ...)
}
```
  On a new core the core itself moves the previous file to `.1` at the session's first write, so no private rotation is needed.
- **Fallback level**: INFO normally, DEBUG when the INI has `Verbose=1`: in `checkConfig` after a successful load, `if (!KpLog_HasCore()) KpLog_SetFallbackLevel(g_cfg.general.verbose ? KPLOG_DEBUG : KPLOG_INFO);`. TRACE stays off in the fallback (rect and per-member lines). WARN always passes (fallback level only drops lines above it, and it is never set below WARN).
- **`Verbose` on a new core**: kplog.ini decides (`[force-power-hotbar] enabled=1 level=debug`). Keep parsing and saving the INI key (round-trip tests) but stop using it as a log gate: the two direct uses, 254 (profile dump) and 638 (cursor sample), become `KP_ENABLED(chCfg/chInput, KPLOG_DEBUG)`. Log one INFO at startup when `Verbose=1` and a core is present: `cfg: Verbose=1 only applies without the kpatch.log core; use kplog.ini [force-power-hotbar] level=debug`.
- **Behaviour change to announce**: with no `kplog.ini`, a new core records only WARN and ERR, so the normal INFO trail (banner, heartbeat, slot lines) disappears for players who never made a `kplog.ini`; today it is always written. Ship a sample `kplog.ini` section for the mod in doc 07.
- **LOG CAP**: the in-mod `g_logLines` / `MaxLogLines` (60000) / `LOG CAP REACHED` text go away. The header's fallback sink has the same 60000-line default and writes `--- log cap reached ---` once (so the old text is no longer there to grep); the new core rotates by size (`max_bytes`) instead.
- **Frame counter**: `g_frameForLog` and the per-line `f<N>` stamp go away; the core's `f=` is the shared counter. Do NOT call `KP_FRAME_TICK()`: `HotbarHudTick` runs only while the gameplay HUD draws, not once per frame globally (guide point 7). Keep `g_frame` itself (the refresh logic uses it) and keep `frame=%ld` in text only in the H3 summary (`fmtH3Summary`) and the heartbeat (`fmtHeartbeat`). Optional, if you want frame context on the old core: add `frame=` to the `gate:` and `tick: gap` lines (cheap, 2 lines).
- Line shape on the old core: `<GetTickCount ms> force-power-hotbar/<chan> [WARN ]text`, no `T=`/`f=`; any future parser must accept both shapes. No script in the repo parses the hotbar log today (checked with grep for `force_power_hotbar_log`: only LoadHangProbe and docs 07/09 mention the name).

## 8. KpLog.h and core discrepancies (against the task brief)

1. **Rate cap scope**: the 2000 lines/s per patch limit applies to INFO, DEBUG and TRACE; WARN and ERR are exempt (`PatchRegistry.md` line 381). "INFO+" in the brief reads as "INFO and above" and would include WARN. Consequence: WARN is safe from the cap but is also NOT throttled, so every WARN site needs its own change-only cache (sections 2 and 4). Nothing in `KpLog.h` itself implements the cap; it is core-side.
2. **232-byte cut is the core's, not the header's**: `KpLog.h` formats into 512 bytes and cuts at 511 with `[...]`; the core then cuts the text at 232 (confirmed by `any_log_service.cpp:545-549` and `any_kplog_header.cpp:743-763`). The fallback sink therefore keeps lines up to 511 bytes, so the same long line is intact on the old core and cut on the new one.
3. **`<ms>` is not `T=`**: the core's `T=` is milliseconds since the service started (docs line 266 shows `T=2000.001`), the fallback `<ms>` is `GetTickCount` truncated to 32 bits. Different clocks, do not diff them.
4. **Channel names**: the header never checks them; the core accepts 1 to 31 characters from `[A-Za-z0-9_.-]`. "Lowercase and at most 12" is a project recommendation only (all ten channels above comply).
5. **`KP_LOG_EVERY` is INFO-only and per call site**: the `static std::atomic` gate lives in the macro expansion, so one use inside a shared helper (for example `wVisible`) throttles every widget together; there is no DEBUG/TRACE variant. Per-key rate control needs a hand-made cache (section 2).
6. **`KpLog_HasCore()` is false until `KpLog_Init` succeeds**, hence false in `DllMain` and static initialisers even on a new core (section 7). `KP_WARN`/`KP_ERR` before that go to the fallback sink at the default fallback level WARN.
7. **Embedded `\n` splits a message** into separate lines, each without the tag, and the 512-byte buffer is cut before the split (section 3 row 614, section 5).
8. **File name differs between modes**: new core `force-power-hotbar_log.txt`, fallback `force_power_hotbar_log.txt` (section 7). Also `WARN`/`ERR` are "always on" on a new core only because the core clamps the level to at least WARN; on the old core they follow `KpLog_SetFallbackLevel`.

Verified as stated in the brief: `KP_CHANNEL`, `KP_LOG` = INFO, `KP_LOGV`, `KP_WARN`/`KP_ERR` on `main`, `KP_LOG_EVERY(ch, ms, ...)`, the core prefix `T=<ms> f=<n> <patch>/<chan> ` (WARN lines read `<chan> WARN text`), 232 cut ending `[...]`, 60000-line fallback cap, `KpLog_SetFallbackFile/Level`.

## 9. Migration order (for the implementing session)

1. `KP_LOG_PATCH`, `KPatchInit`, `exports.def`, manifest bump, channel declarations, `startOnce()`; delete `logf`/`VLOG`/`g_logLines`/DllMain logging. Build with the macros temporarily mapped so the 146 sites compile (`logf` -> `KP_LOG(chX, ...)` per the table).
2. Mechanical conversion by table row (channel and level), keep texts. Check with `rg -c 'KP_(LOG|LOGV|WARN|ERR|LOG_EVERY)\('` against the 146 rows plus the section 4 additions.
3. Add the R1-R6 caches, then the section 4 lines.
4. Add the `fmt*` functions with worst-case tests (`hotbar_logic_test.cpp`), then switch the 6 over-limit sites and the near-limit ones.
5. Section 5 ring, section 6 itemised warnings, then the `HotbarConfig.h` message fix as its own commit.
6. Verify per guide point 8: no `kplog.ini` shows only WARN/ERR; with a `kplog.ini` (`level=debug`) every channel appears; grep `DROPPED`; run the old-core path with the fallback file and check `force_power_hotbar_log.txt` still appears and LoadHangProbe finds it.
