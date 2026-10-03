# A2 robustness review - level_up_override_src 1.0.0 (read-only, static)

Scope: `patch_manager_mods/level_up_override_src/{LevelUpOverride.cpp, LevelUpUi.h, OptionsLogic.h, WizardLogic.h, Playthrough.h}` and `_shared_src/{GuiKit.h, SafeRead.h}`.
Method: full read of all files, doc 08 (U5, S1 guards, hold rule), the committed `tests/luo_scenarios/saves.toml` and `scenario_test.cpp` action vocabulary
(`load save newserver tick auto guard ui click`), sizes/stack measured with `i686-w64-mingw32-g++ -O2 -fstack-usage` on a scratch copy. Nothing in the repo or game dir was modified.
LO = `LevelUpOverride.cpp`, UI = `LevelUpUi.h`, PT = `Playthrough.h`, GK = `GuiKit.h`, SR = `SafeRead.h`.

Severity key: BUG = behaviour that contradicts the stated design or loses data; RISK = plausible failure / unproven; OK = proven safe by code structure (reason given).

## Summary table

| ID | Item | Severity | One line |
|----|------|----------|----------|
| F1 | 10 | BUG | `onLevelUp` returns before any rewrite when the log cannot be opened or the 20000-line cap is hit (LO:1205) |
| F2 | 8 | BUG | Pause path `HOLD`s a creature that already has a DIFFERENT Jedi class; the non-pause rule returns VANILLA (LO:568-572) |
| F3 | 3/10 | BUG (low) | Rewriting `levelup_playthroughs.ini` silently drops trailing text after `]`, inline comments, CRLF; contradicts "never destroys hand-written content" (PT:146-149,196) |
| F4 | 5 | RISK | `leaderInstant` never checks 0x740FC0 although doc 08 U5 lists it as a condition (LO:1573-1595) |
| F5 | 5/8 | RISK (known) | Multi-level award across the pause/convert boundary: levels above the boundary are engine-picked (HOLD_MIDLOOP) |
| F6 | 3 | RISK | Any "lossy" parse (line > 255 chars, > 16 extra lines in a section, > 64 sections, file > 541 KB) disables ALL writes incl. Vanilla button; UI shows a generic error |
| F7 | 3 | RISK | 65th playthrough refused; sections >= 65 in a hand-made file are ignored silently (= vanilla) |
| F8 | 3 | RISK (low) | 17th tag line in a section is kept verbatim but never applied, no log |
| F9 | 3/10 | RISK (low) | UTF-8 BOM breaks the first header / first ini key |
| F10 | 4 | RISK | A truncated / malformed / over-cap table disables the whole table: every HOLD lifts until the next mtime change |
| F11 | 1 | RISK | Controls stored per panel are written every tick with no re-validation that the engine still owns them (layout rebuild without ctor) |
| F12 | 2 | RISK (low) | Ctor on a live panel orphans appended controls (still visible, never updated); round-robin eviction of a live record hides the UI |
| F13 | 1 | RISK (low) | `maybeBanked` calls an engine function through an un-validated GuiInGame pointer (LO:1487-1521) |
| F14 | 10 | RISK | `noteHold` HOLD/UNHOLD log-on-change can flap per frame; per-line fopen; log cap silences later errors |
| F15 | 10 | RISK (low) | Log rotation only once per process; byte size unbounded within a session (cap is lines, 2 KB each) |
| F16 | 8 | RISK (low) | Stored pause is honoured raw at runtime but clamped to 0 when the page is reopened (UI/runtime mismatch, silent change on next Apply) |
| F17 | 10 | RISK (info) | Unknown `mode=` value (incl. `mode=on`, `1`, `true`, inline comment) = Log (inert) while a missing key/file = Rewrite |
| F18 | 6 | RISK (info) | Single-thread assumption is structural, no runtime assertion; committed evidence shows one tid only |
| F19 | 7 | RISK (info) | Static memory is ~2.75 MB, not ~1.3 MB; whole-table failure cliffs at 128 presets / 256 KB |
| F20 | 10 | RISK (info) | Upgrade from 0.3.x ini: dev keys without `dev=1` are now ignored |
| OK1-OK10 | various | OK | see the end |

Counts: BUG 3 (F1-F3), RISK 17 (F4-F20), OK 12 (OK1-OK12).

---

## 1. UI tick during load screens / module transitions

**Where the tick runs.** Only `LuoHold_84F221` (LO:1664-1671) -> `uiTick` (UI:401). That site is inside SetStats 0x84E6F0, called from `CSWGuiInGameCharacter::Update`,
i.e. only while the character panel is the updated panel. During load screens / module transitions the panel is not updated, so no tick. The other hold sites (0x5FD7CE XP
award, 0x5FEE32 AutoLevelUpNPCs, 0x66C95C LevelUpHenchman, ...) can run during gameplay transitions and only READ creature memory through `readAt` (probe + copy), and write nothing.

**OK1 (OK): panel pointer is never trusted without a ctor record.** `findUi(panel)` (UI:94-97) requires `magic == UiMagic && panel == key`; a tick for a panel whose ctor
hook did not run is logged once and ignored (UI:404-408). Creature state is re-read from `stats` every tick (`readTag`, UI:413); the only creature-derived state kept across ticks is
`lastTag`/`pageTag` strings (UI:77,80), no pointers. Party swap / leader change flips `lastTag` and closes the page (UI:415).

**F11 (RISK): stale control pointers when the engine rebuilds the panel list without running the ctor hook.** `UiPanel.c[]` (UI:67) holds raw control pointers; after `appended == true`
every tick calls `setRect/setVisible/setText/setAlpha` on them (UI:176-209, 187-195) through `wr<>` which only checks that the target is READABLE (GK:93-96), not that it is still a live control
of this panel. If a video-mode change / GUI reload re-runs the layout loader on the same panel (list cleared, controls freed) without hitting 0x84D82C, writes go to freed (but mapped) heap.
Whether the engine can do that is not shown anywhere in doc 08 (Mod 14 has the same exposure).
Proposed check (cheap): in `uiTick`, before placement verify `ctl.id == index && panel.array[id] == ctl` for `c[C_CFG]` (GK `appendToPanel` already establishes this invariant); on mismatch `appended=false`-style reset + log `UI panel list changed`.
Offline proof: `ui_test.cpp` / scenario action `shrinklist` (new): after `ACT ui atton`, set the fake panel's `Panel_CtlCount` to 60 and zero the array slots, then `ACT ui atton`;
expect `ABSENT` no write to the old control addresses (the fake arena can poison them: memset 0xDD, then assert unchanged) and `EVENT ^UI panel list changed`. Today the second tick writes into the poisoned memory.

**F13 (RISK, low): banked message calls the engine with an unvalidated GuiInGame.** `guiInGame()` (LO:1487-1492) probes every link, but a freed-yet-mapped GuiInGame still passes
`probeRead`; `BankedAdd_t(g_eng.bankedAdd)(gui, ...)` (LO:1518) then runs engine code on it. Reachable only from the hold gates during gameplay where the client app is live, so the practical window
is a frame around a client-app teardown. Mitigation: only fire when the same gate call chain came from the UI tick or when `*(gui+0)` equals the known GuiInGame vtable (read once at the first successful message).
Offline proof (partial): scenario `X3_banked_gui_null`: action `guinull` zeroes `[ci+0x40]` -> `tick atton` -> `ABSENT ^BANKED tag=`, `EVENT ^HOLD tag=atton `, no crash; and `guifree` pointing at a poisoned region
(expect the vtable check to refuse; today the fake BankedAdd is called with it).

**SR cache note (OK).** `probeRead` caches whole VirtualQuery regions per thread until the next `beginScope()` (SR:35-60). Every hook entry calls `beginScope` and the tick calls it again
(UI:403, LO:1668); the engine calls inside a tick (`appendToPanel` list growth, `setText`) re-read the array pointer from the panel afterwards (GK:150-153). No stale-cache read found.

## 2. Panel re-created at the same address

**OK2 (OK): no stale per-panel state.** `uiPanelCtor` does `*r = UiPanel()` on the found record (UI:103-106) - magic, `appended`, controls, text cache, `colorTried`, edge, `open`, `pageTag`,
`pageTable`, `items` are all reset, and the labels are created fresh from the template (UI:108-121). The only per-process flags that survive are one-shot log guards (`g_uiNoRecLogged`, `g_uiNoScaleLogged`, UI:92).
`appended` is per record, so a re-created panel appends exactly once; `appendToPanel` also verifies count+1 and slot (GK:143-154), so a double append of the same control within a record is impossible.

**F12 (RISK, low), two sub-cases.**
a) The ctor hook fires for a panel that is still alive and already has our appended controls (any non-destroy re-entry of 0x84C3A0's layout part): the record is reset, 19 NEW controls are created,
but the previously appended controls stay in the panel list with whatever visibility they had (a stuck status line / opaque background is possible) and are never updated again (UI:106 resets without `uiHideAll`).
b) `g_ui` has 4 records, round-robin (UI:104). The 5th distinct panel address evicts a live record (not necessarily the oldest alive); the evicted live panel then logs `UI tick: no record` once and has no UI.
Not reachable with the single character panel, but it is silent.
Offline proof: `ui_test.cpp` action `ctor2` (call `LuoUiPanelCtor` twice for the same fake panel with an `appended` first tick in between): expect old controls have `FlagVisible` cleared after the second ctor
(proposal: `uiPanelCtor` calls `uiHideAll(r)` on a record that was `appended` before resetting). Second: action `ctor5` (5 distinct panels) expecting a `UI record evicted` log line (new).

## 3. Playthrough caps (64 sections / 16 tags / lossy)

Caps: `MaxSections 64`, `MaxTags 16`, `MaxExtra 16` (-> `MaxLines 32`), `MaxLine 256`, file buffer `MaxText = 541,320` bytes (PT:18-19, LO:1046, 1092-1099).

**OK3 (OK): the cap policy is "refuse, never lose".** `ptWrite` returns false on `lossy` (LO:1105); `setTag` fails on bad/too-long args or a full section (PT:210-222); a failed `setTag`/`ptWrite` is
followed by `reloadPlay(true)` which re-parses the file and restores the in-memory doc (LO:1141-1143); `ptApply` returns false (UI shows "Could not save - see log", UI:326). User config in the file is never truncated by a cap.

**F6 (RISK): a lossy parse locks out every write.** Triggers: any physical line >= 256 chars (PT:138), a section with > 32 lines (PT:121; i.e. 16 tags + more than 16 comment/unparseable lines), > 64 sections (PT:149), a section header name >= 64 chars,
or a file larger than 541 KB (LO:1093). A long hand-written comment is enough. Result: `Apply` AND `Vanilla` both fail for ALL playthroughs (LO:1105 is global), the only signal is a log line (`PLAYTHROUGH write refused`) and the generic title string. Reads still work for the parsed part.
Proposed: (a) the UI title for `failed` should say "playthrough file too large/odd - see log" when `g_ptDoc.lossy`; (b) `lossy` only for what would actually be dropped, not for an over-long COMMENT line (truncate+flag per line instead of global).
Offline proof: saves.toml-style extension (needs `ptfile = "<raw text>"` key, see "Runner extensions" below) - scenario `X4_pt_lossy_longline`: file `[g5eed0004]\natton=atton__melee_dual_light_early|instant=0|hold=1\n; <300 x 'a'>\n`, actions `load`, `ui atton`, `click cfg`, `click apply`;
`EVENT ^PLAYTHROUGH write refused`, `ABSENT ^PLAYTHROUGH write g`, `DECISION ptfile.unchanged 1` (file bytes identical), `DECISION ui.failed 1`.

**F7 (RISK): 65th playthrough.** `ptApply` with `g_guid == 0` and `g_ptDoc.n >= 65` refuses (LO:1127) - no eviction, no pruning of dead playthroughs, so after 64 configured playthroughs (every New Game that is configured; forged test saves count if their GUID goes through the UI) the feature stops for NEW games with only a log line.
Sections 65+ in a hand-edited file are parsed as lossy and ignored: a save whose GUID lives in section 65+ reads as unconfigured (vanilla) silently (PT:149 `cur = nullptr`).
Offline proof: scenario `X5_pt_64_sections`: generator writes 64 `[gXXXXXXXX]` sections, actions `load` (GUID none) `ui atton` `click cfg` `click apply`; expect `EVENT ^GUID new: levelup_playthroughs.ini already holds 64`, `DECISION save.globals 0`, `DECISION ui.failed 1`. Companion `X5b_pt_65_sections_read`: 65 sections, GUID = the 65th, `tick atton` -> expect `DECISION atton.canLevelUp NONE/PASS` (unconfigured) and `EVENT ^PLAYTHROUGH loaded: 64 section\(s\) LOSSY`.
Suggested fix: when full, drop the oldest section that is NOT the current GUID only if the user confirms, or raise to 256 (Doc is 720 KB at 64; 4x is 2.9 MB BSS, still fine) .

**F8 (RISK, low): tag line 17+.** `parse` only creates tag lines while `tagCount < 16` (PT:159); the 17th `tag=preset|...` line falls through to a verbatim non-tag line (PT:175-176): preserved on rewrite, never applied, not logged. The game has ~12 companions, so practically unreachable; the hand-edit case is silent.
Proof: unit in `playthrough_test.cpp` (17 tag lines -> `tagCount==16`, line 17 round-trips verbatim); add a log line `PLAYTHROUGH: extra tag line ignored`.

**F3 (BUG, low): hand-written content is modified on rewrite.** The header `[g5eed0001] ; note` is re-emitted as `[g5eed0001]` (PT:146-149: everything after `]` dropped); a tag line `atton=preset|instant=1 ; why` is re-emitted canonical without the comment (PT:196-197, `atoi("1 ; why")` still parses); CRLF files become LF (PT:136, 199). Contradicts the header promise (PT:5-7) and the doc.
Proof: `playthrough_test.cpp` round-trip: input `"[g5eed0001] ; keep me\r\natton=p|instant=1 ; why\r\n"`, `setTag` another tag, serialise; assert `strstr(out, "; keep me")` and `"; why"` (fails today).

**F9 (RISK, low): BOM.** `trimStr` strips only space/tab/CR/LF (PT:68-73), so a BOM-prefixed first line is `\xEF\xBB\xBF[g...]`: not a header (PT:146) -> the first playthrough section is silently treated as preamble text (its tag lines are verbatim, unconfigured), and `ptApply` for that GUID appends a SECOND section with the same name. In `levelup_override.ini` the first key is ignored (LO:245-247; `mode=off` on line 1 with a BOM would be ignored = Rewrite).
Proof: `playthrough_test.cpp` BOM input -> expect `findGuid` finds the section (fails today). Fix: skip a leading `EF BB BF` in both parsers.

**GUID live-write ordering (OK4, benign).** `ptApply` writes the 32 globals and sets `g_guid` (LO:1136-1139) BEFORE `setTag/ptWrite` (LO:1141). If the file write then fails (cap, MoveFileEx, disk), the next save carries a GUID with no section = the S6 "orphan" case (vanilla), and the next Apply retries without creating a new GUID (g_guid != 0). No data loss.
`MoveFileExA` failure: `remove(PtTmp)`, return false, doc restored (LO:1112-1114,1143). OK. Proof exists in spirit (S6); add `X6_pt_write_fails`: make `levelup_playthroughs.ini` a directory (`mkdir`) so MoveFileEx fails: `EVENT ^PLAYTHROUGH write g[0-9a-f]{8} tag=atton ok=0`, `DECISION ui.failed 1`, no `.tmp` left.

## 4. Table hot-reload with the options page open

**OK5 (OK): the page cannot hold dangling preset indices.** The page stores `itemPreset[]` (indices into `g_presets`) and `Item.id/shortName` pointers into `g_presets` (UI:273-278). `uiTick` calls `gateActive()` (which may `reloadTable`) at UI:411 and then closes the page if `pageTable != g_tableStamp` (UI:416) before any click or render; if the new table has no selectable presets for the tag, `uiHideAll` closes it first (UI:414). `uiClick` and `uiRenderPage` run after that check in the same tick, `reloadTable` cannot run in between (single thread, no engine call between UI:411 and UI:430 that reaches a gate). `onLevelUp` does `reloadTable(true)` (LO:1203) but never touches the page. Gate/`Eval` pointers (`e.p`, `e.cfg`) are re-derived inside each call after the reload.
Proof (offline): extend `ACT` with `tablecopy <file>` (replace the table file between ticks, bump mtime) and run: `ui atton`, `click cfg` (page open), `tablecopy table_without_atton_variants.ini`, `ui atton`: expect `ui.open 0` and no read of freed presets (ASAN build of `scenario_test` catches it if the page kept pointers; table arrays are static so ASAN will not - use a poison pattern: memset `g_presets` to 0xDD inside the action before reload).

**F10 (RISK): a bad table lifts every HOLD.** `parseTable` failure sets `g_tableOk=false` and `g_nPresets=0` (LO:503-506, 514); `evalStats` then returns `!ok` (LO:1406) -> `holdGate` passes the real CanLevelUp through (LO:1530) and `onLevelUp` skips decisions (LO:1304). Window: from the moment the table file's mtime/size stamp changes to a parse failure (half-written copy: `cp` is not atomic; an editor that saves truncated; a typo) until the next mtime change. In that window a companion with banked levels is levelable (vanilla) and auto-level NPCs level with engine picks, defeating the bank. Cliffs that cause total failure instead of per-preset skipping: > 128 presets (LO:404, today 85 sections incl. 3 headers), file >= 256 KB (LO:425, today 72 KB), any malformed line (LO:503).
Proposed: keep the previous good table when a reload fails (parse into a scratch struct; swap on success) - costs +1.2 MB or a two-phase parse; at minimum log `TABLE ERROR ... keeping N held` and keep HOLD for tags that were held in `g_holdState`.
Offline proof: `X7_table_truncated`: action `tablecopy testdata/levelup_paths_truncated.ini` (first 40 KB of the real table cut mid-line) then `tick atton` with Atton Scoundrel 7 / XP 28000 and a held playthrough config: `DECISION atton.canLevelUp HOLD` expected (today `PASS` = bug), `EVENT ^TABLE ERROR`.

## 5. Leader instant (0x74F423) vs 0x740FC0, party swap mid level-up

**F4 (RISK / doc-code gap): `leaderInstant` does not check 0x740FC0.** Doc 08 (lines 693-695) states the substitution applies for "a configured instant, not-held creature (and 0x740FC0 == 0, no character change in progress)". The implementation (LO:1573-1595) checks mode, engine bytes, `[ebp-0x38]` creature, `evalStats`, `instant`, not Hold - and returns `g_noCtl`; `g_eng.charChange` is not called.
During a leader change (the case 0x740FC0 reports; 0x84F9FE/0x84FD2F treat nonzero as "block"), the engine is mid-swap; instead of the vanilla indicator branch the portrait refresh calls `AutoLevelup(1)` on the creature that is being swapped to/from.
Whether that is harmful is unknown (static); the doc wanted the safe side. Fix: `if (charChange(ecx_of_gui)) return p;` (needs the right ECX: same object `guiGuard` receives, call-site ECX of 0x84FD2F differs from this call site's ECX - not available here, so use `leaderCreature()`-style chain `*(*[0xA1B4A4]+4)` + the GetGuiInGame accessor used by 0x84FD2F; confirm with the Ghidra DB before coding).
Offline proof: `X8_leader_instant_changing` (needs actions `leaderinstant <tag>` and `ccret <n>`; the fake already has `g_ccRet`): atton instant=1, leader = atton, `ccret 1`, `leaderinstant atton` -> `ABSENT ^INSTANT-LEADER`, `DECISION atton.leaderCtl REAL`; with `ccret 0` -> `EVENT ^INSTANT-LEADER tag=atton`, `DECISION atton.leaderCtl DUMMY`. Today both give DUMMY.

**OK6 (OK): leader instant for held / PC / unconfigured.** Held (`e.d == Hold`) -> real object (LO:1585); the PC has an empty tag so `readCreatureTag` fails -> `e.ok=false` -> real object (S10 asserts `INSTANT(-LEADER)? tag=\?` absent); mode != Rewrite and engine byte mismatch pass through (LO:1576-1581). `g_noCtl` is 16 bytes, engine reads only `+4` (doc U5).

**guiGuard (OK7).** Returns the real 0x740FC0 result when nonzero (LO:1617), so a swap in progress is already blocked by vanilla; the evaluation uses party slot 0 = the creature the character screen shows (doc S1). A leader swap while the options page is open changes `lastTag` -> page closes (UI:415). A hold flip while the vanilla level-up GUI (CSWGuiLevelUpCharGen) is already open does not affect the open GUI; its Accept reaches the server LevelUp hook as a manual flow and is `SKIP manual` (LO:1326) - the player's picks are kept. 
Proof: existing `guard` action covers HOLD/PASS; add `X9_swap_while_page_open`: `ui atton`, `click cfg`, leader := kreia (`leader kreia` new action), `ui kreia` -> `ui.open 0`.

**F5 (RISK, documented limitation, now visible with Pause): the LevelUp hook can never veto, the hold gates guard only the START.** A multi-level award while `key <= pause` passes the gate; the engine's AutoLevelup loop then calls LevelUp for key > pause and `levelNow` returns HOLD -> `HOLD_MIDLOOP` -> engine picks (LO:1310-1313, test at scenario_test.cpp:455).
Example: Atton Scoundrel 4, pause=5 (preset convertAt 8), one XP grant worth 3 levels: level 5 is PATH-rewritten, levels 6-7 are vanilla auto picks and are NOT banked; the user asked to bank above 5.
Proof: `X10_pause_multilevel`: creature atton `Scoundrel 4`, xp 105000, config pause=5, action `autoloop atton` (new: repeat `tick`+LevelUpOverrideEntry with the engine chain until the real CanLevelUp is 0, like `simulate_auto_levelup`); expect `DECISION atton.levelsTaken 1` (desired) - today 3 with `EVENT decision=HOLD_MIDLOOP`.
Possible fix (needs RE): gate AutoLevelup's internal loop condition, or clamp the XP the award path gives (not simple); at minimum document it in README (A5).

## 6. Single-thread assumption

**OK8 (OK by structure, F18 unproven live).** All globals (`g_cfg`, `g_ptDoc`, `g_guid`, `g_ui`, caches) are plain; only `g_calls` uses `InterlockedIncrement` (LO:872,1199). Hooked functions: CSWSCreatureStats::LevelUp (server, main loop), CanLevelUp/accessor/guard sites (UI/server main loop), the globals saver/loader and server-app init (main thread). `saferead` state is `thread_local` (SR:27-28), the ring uses an interlocked counter (SR:64) - those are safe if some other thread ever enters.
Evidence: committed live log has a single `tid=316` for all LevelUp calls (`research/companion_levelup/logs/live/levelup_override_log_run3_20261002.txt`); no other hook logs the thread id.
**F18 (info):** nothing asserts it. Cheap guard: remember the first `GetCurrentThreadId()` in `ensureHeader` (LO:927) and `logLine("THREAD other tid=%lu")` once in `gateActive/guidSave/guidLoad` if different; then `luo_log_check.py` fails the run on that line. Offline proof is impossible (fake engine is single-threaded); a `scenario_test` thread spawn calling `LuoHold_74F3C7` concurrently would only show data races (TSAN) that the design already accepts - not recommended.

## 7. Static memory, caps, stack

Measured (i686-w64-mingw32, -O2, sizeof of the real types): `g_presets` 1,211,392 B (128 x 9,464; `Preset` holds 51 `Rec` x 180 B), `g_ptDoc` 720,468 B (65 sections x 11,084), `g_ptText` 541,320 B, `g_fileBuf` 262,144 B, `g_ui` 15,360 B (4 x 3,840), `g_cfg`+`g_ptCfg` 2 x 1,744 B, `imageFor` pool ~6 KB.
**F19:** total ~2.75 MB of `.bss` (zero pages are committed lazily; `parse`/`clear` touch only used sections), roughly 2x the "~1.3 MB" in the plan (that figure is `g_presets` alone). Not a problem for a 32-bit process with the 4 GB patch, but update the plan/doc number. Caps today vs headroom: presets 85/128 sections (max 8 presets per tag, `MaxItems` 16, UI:268 truncates silently above 16 per tag), table 72 KB/256 KB, playthrough sections 64.
Doc 21 should state the cliffs in F10 (whole-table failure) and F7.
**Stack (OK9):** `-fstack-usage`: `onLevelUp` 9,728 B, `logPend` 8,304 B (called from it only when verbose) -> ~18 KB peak; `reloadConfig` 4,432; `fileCrc32` 4,144 (once); `uiPanelCtor` 3,920 (the `UiPanel()` temporary); `uiTick` 416; `holdGate` 384. All far below a 1 MB main-thread stack, and `onLevelUp` runs inside AutoLevelup (5 KB function) so total depth stays < 40 KB.
Proof is the compiler output; rerun in the A3 gate: add `-fstack-usage` + a threshold script (max < 32 KB) to the mingw `-fsyntax-only` check.

## 8. Pause edge cases

Rule (LO:563-590): `hold && pauseAt > 0 && p->holdClass && key > pauseAt && creature lacks holdClass -> HOLD`, evaluated BEFORE the record match. The UI clamps to `minPause .. convertAt-2` (UI:244-247) but the file / dev key value is used raw (`atoi`, `>0`, LO:275, PT:115).

* **Pause from a dev key > convertAt (OK10a):** harmless. With `pause=20` and convertAt 8 the pause branch fires only at key > 20; below that the normal rule already HOLDs at the end of the base-class records, so the effective behaviour equals the default. `hold=0` or `holdClass=0` disables pause (LO:568). Proof: `S12_pause_gt_convert`: atton Scoundrel 7, xp 28000, `dev={"pause.atton"="20"}` + dev=1 -> `EVENT ^HOLD tag=atton `.
* **Pause < minPause (OK10b):** `pause=1` for a Scoundrel 5: key 6 > 1 -> HOLD at once. Equivalent to "hold from now"; no crash, no negative index. The page, when reopened, displays the clamped value (see F16).
* **Companion joins above the pause level (OK10c):** `pause=3`, companion arrives at Scoundrel 6 without the hold class: HOLD from the first level-up (key 7 > 3). Matches the intended "bank everything until Jedi". If it already has the hold class the branch is skipped and the record match applies (REBASE).
* **F2 (BUG): a creature that already has a DIFFERENT Jedi class is HELD forever by the pause branch.** `lacks` only tests for `p->holdClass` (LO:569-571). Without pause the normal path returns VANILLA for such a creature (newest class = Jedi Guardian has no preset record, `top = -1`, LO:573-589). With pause set, `key > pause` + holdClass (Sentinel) absent -> HOLD although the creature can never take the preset's hold class on its own; the player's only exit is the Vanilla/Bank-off button, and the status line says "waiting for Jedi Sentinel".
  Real trigger: a save where Atton was already converted manually (Guardian/Consular) before the player turned Pause on, or a preset whose hold class differs from the player's choice.
  Proof: `S13_pause_other_jedi`: atton `[["Scoundrel", 5], ["JediGuardian", 2]]`, xp for level 8, `companions = { atton = "atton__melee_dual_light_early|instant=0|hold=1|pause=3" }` (preset holdClass = Sentinel) -> expect `DECISION atton.canLevelUp PASS` and `ABSENT ^HOLD tag=atton `; today HOLD. Fix: in the pause branch also require "newest class has a preset record" or "creature has no Jedi (hold-group) class".
* **Preset changed in the table under a stored pause (F16, RISK low):** runtime keeps using the stored raw value; when the options page is reopened `uiOpenPage` -> `uiClampPagePause` (UI:263,291) shows `0 = default` for an out-of-range value, and an Apply without touching Bank writes `pause=0` back (silent change of behaviour). The raw value in the file is otherwise harmless (see a/b). Proof: ui_test: table with convertAt 8 -> store pause 5; replace table with a preset whose convertAt is 5; `ui`, `click cfg`: assert text `Bank for Jedi`/`Pause at level 4` (default) and log `UI page open ... pause clamped 5->0` (new); today silent.
* **Pause and bank math:** `uiBankCycle` terminates (`next < minPause` -> Off) and `convertAt <= 2` gives an empty range (every pause clamps to 0): verified by reading, covered by `options_parity.py`/`ui_test` (no new test).

## 9. Default mode=rewrite with no ini

**OK11 (OK): an absent `levelup_override.ini` leaves unconfigured companions and the PC vanilla.** `reloadConfig` with the file missing keeps `Config{mode=Rewrite, apply=true, dev=false}` and `c.nTags = 0` (LO:176-185, 280); `tagConfig` (LO:300-305) returns a TagCfg only from the playthrough section of the CURRENT GUID (`g_ptActive`, set only when `g_guid != 0` and a section exists, LO:1055-1056); the PC's empty tag fails `readCreatureTag` (LO:660) so `evalStats` is `!ok`; `onLevelUp` has `cfg == nullptr` -> no DECIDE, no rewrite. The only state-changing path without a prior user action is none: Configure/Apply (UI:381-389) is the sole writer. With `levelup_paths.ini` also missing, `g_tableOk=false` -> no presets -> button hidden (`uiCountSelectable == 0`, UI:414).
Existing coverage: S6 (orphan GUID), S7 (no LUO_G), S10 (PC) run with an ini; they do not exercise a MISSING ini file.
Proof to add: `X11_no_ini` (needs runner support `INI-MISSING` = delete the file instead of writing it; scenarios.py emits it for a new `config.ini = "missing"` key): creatures atton (Scoundrel 7, xp 28000) + pc (xp 105000, leader), `PTGUID none`, actions `load tick atton auto atton tick pc auto pc ui atton`; expect `EVENT ^CONFIG missing \(defaults\) mode=rewrite`, `DECISION atton.canLevelUp NONE|PASS` (real, ungated), `DECISION atton.autoLevel NONE`, `ABSENT ^DECIDE`, `ABSENT ^APPLIED`, `ABSENT ^HOLD`, `ABSENT ^INSTANT`; and that after `ui atton` the only button shown is `cfg` (`ui.status 0`).
**F20 (info, upgrade path):** a 0.3.x ini with `preset.atton=`/`tags=` but no `dev=1` is now ignored (LO:280) - the log line `CONFIG loaded ... tags=` is then empty. Intended (doc header LO:57-59) but worth one README line (A5).
**F17 (info):** `mode=` anything other than `off`/`rewrite` is `Mode::Log` (LO:249): `mode=on`, `mode=true`, `mode=Rewrite ;x` become inert dry-run, whereas missing = Rewrite. Safe direction, surprising. Proof: `X12_mode_garbage`: `INI mode=on` -> `EVENT ^CONFIG loaded mode=log`, `ABSENT ^APPLIED`. Suggest logging `CONFIG unknown mode 'on' -> log`.

## 10. Anything else

**F1 (BUG): rewrite silently depends on the log.** `onLevelUp`: `if (!logOpen()) return;` (LO:1205) comes after config/table reload and BEFORE the decision and `applyRewrite`. `logOpen()` is false when `g_logLines >= 20000` (LO:122,129) or `fopen(levelup_override_log.txt, "a")` fails (read-only dir, sharing violation from an editor/AV on the file, disk full). Then every auto level-up goes through vanilla while the hold/instant gates (which do not need the log) keep working - a half-active mod with nothing logged. 20000 lines is far at verbose=0 (~4 lines per call) but reachable when F14 flaps, and fopen failure is independent of volume.
Fix: do the decision/apply without a log (`logf` is already a no-op when `g_f == nullptr`, LO:135); only skip the dump.
Proof: scenario_test action `logcap` (sets `g_logLines = MaxLogLines`, reachable since the .cpp is `#include`d) then `auto atton`: expect `DECISION atton.autoLevel APPLIED` (today NONE: the hook returned early). Second action `logro` (chmod 0444 on the log) same expectation. `EVENT` cannot be used (nothing is logged) so use the `DECISION` plus an `ACT pendcheck` that compares the fake pending struct to the preset record.

**F14 (RISK): log-on-change can flap, per-line fopen, cap silences errors.** `noteHold` (LO:1420-1432) is keyed by TAG, but the hold sites evaluate both client and server creature objects (and several sites per frame) for the same tag; if their class/level state differs for even a few frames (client lags the server after a level-up) the state alternates HOLD/UNHOLD and each flip logs a line (open/append/close per `logLine`, LO:142-150). At 60 fps that is 120+ lines/s -> the 20000-line cap in ~3 minutes; afterwards `logLine`/`logf` are silent for the rest of the session, including GUID/PLAYTHROUGH failures that explain a broken Apply.
Mitigation: key `HoldState` by (tag, creature pointer), or rate-limit to one change line per tag per second, and reserve the last 100 lines of the cap for `ERROR/refused/ABORTED` lines.
Proof: `X13_hold_flap`: two creatures with the same tag `atton` (`Scoundrel 7` vs `Scoundrel 8`, a `CREATURE2` record in the .dat) and an action `flap 1000` alternating `tick` on both; `DECISION log.lines.max 20` (fails today with ~2000).

**F15 (RISK, low): rotation once per process.** `rotateIfLarge` runs only in `ensureHeader` (LO:927-930, 875-881): the 4 MB limit is checked at game start, never during a session; a session may add up to 20000 lines x up to ~2 KB (the `Buf` size) = ~40 MB in the worst case. `DeleteFileA/MoveFileA` failures are ignored (logging then appends to the oversized file). Practical size is far smaller (verbose=0). Proof: unit - preload a 5 MB log, call `ensureHeader` -> `levelup_override_log.old.txt` exists; and one with the old file locked (mkdir of the same name) -> log still works.

**F10b (note):** the table, config and playthrough reloads are mtime^size stamps (`fileStamp`, LO:194-198). A rewrite that keeps both mtime (1 s granularity on some filesystems under Wine) and size (a same-length edit, e.g. `hold=1` -> `hold=0`) is not detected; `ptApply/ptRemove` force-reload after their own write, but an external edit within the same second to a same-size file is missed until the size/mtime changes. Low value; mention in README ("edit, then change the file size or wait 2 s").

**MaxItems (F19b, info):** `uiOpenPage` stops adding presets at 16 per tag (UI:268). Today max 8 per tag. If a tag gains > 16 presets the configured preset may not be listed (start falls back to recommended, UI:284-286) and `uiCountSelectable` (UI:213-221) counts all. Proof: `options_test` with 17 items asserting the configured one stays reachable (fails if it is #17).

**rand() (info):** the new GUID uses `QueryPerformanceCounter ^ GetTickCount ^ rand()` (LO:1134) with an un-seeded CRT `rand()`; the collision loop only checks the local ini (LO:1135). 32 bits, per-install; no action needed beyond noting that two installs sharing saves could (1 in 4e9) collide.

**Engine state on error paths (OK12, verified).** `applyRewrite` checks every precondition before the first write and logs `APPLY ABORTED` with the struct untouched (LO:826-850); additions go through the engine's own helpers; the post-apply `readPend` verifies (LO:1340-1344). The hold gates only change the return value; guard/Auto-OK paths return the engine's value on any unreadable link. The globals writer reads back every bit and reports `ok=0` (LO:1003-1022); a failed `guidSave` leaves vanilla bits. No path was found that leaves engine state half-modified apart from F1 (skips a rewrite entirely).

## Runner extensions needed for the proposed scenarios

(`scenario_test.cpp` + `scenarios.py`; the saves.toml contract stays the same.)
- New actions: `leader <tag>`, `leaderinstant <tag>`, `ccret <n>`, `tablecopy <file>`, `autoloop <tag>`, `logcap`, `logro`, `flap <n>`, `shrinklist`, `ctor2`, `ctor5`, `guinull`, `guifree`.
- New config keys: `ini = "missing"` (delete instead of write), `ptfile = "<raw text>"` / `ptfile_generate = "sections:64"` (raw levelup_playthroughs.ini content).
- New `DECISION` keys: `atton.leaderCtl` (REAL|DUMMY), `atton.levelsTaken`, `ui.failed`, `ptfile.unchanged`, `log.lines.max`.
- In saves.toml format the ready ones need no runner change: `S12_pause_gt_convert` and `S13_pause_other_jedi` (creature + companions line + `canLevelUp` decision) - both can be added today.

## OK list (proven safe by structure, with the reason)

1. OK1 UI tick requires a ctor-created record; stats re-read each tick (UI:94-97,404-415).
2. OK2 Same-address panel: full reset on ctor (UI:103-106); single append guarded by `appended` + GK verification (GK:143-154).
3. OK3 Caps refuse instead of losing data; doc restored after a failed write (LO:1105,1141-1143; PT:210-222).
4. OK4 GUID live-write before file write is benign (orphan GUID = vanilla).
5. OK5 Page vs table reload: stamp check precedes all uses (UI:411-416).
6. OK6 Leader instant leaves held / PC / unconfigured creatures alone (LO:1583-1586).
7. OK7 `guiGuard` returns the vanilla nonzero result during a character change (LO:1617).
8. OK8 Thread-local SafeRead cache, interlocked ring, only main-thread hooks (SR:27-35,64).
9. OK9 Stack < 20 KB in every hook (measured).
10. OK10-12 Pause branches a/b/c; unconfigured/PC vanilla with no ini (OK11); apply preconditions all-or-nothing (OK12).
