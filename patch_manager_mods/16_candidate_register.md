# Engine-bug candidate register (doc 16): Phase 0/1 of the doc-15 research

Status: **Phase 1 complete (2026-10-01); Phase 2 builds INSTALLED live 2026-10-02 01:33 (doc 17), awaiting T1/T2.** Plan: `15_engine_bug_candidates_research_plan.md`; context: `15a_research_session_context_2026-10-01.md`.
Static analysis only. No game launch, no installs, nothing written to the game dir.

## Hand-off summary (top section; updated at the end of Phase 2)
Phase 0 + Phase 1 (static analysis) result, 2026-10-01. Nothing was launched, installed or written to the game dir (`swkotor2.exe` md5 `c9a1e3c5...` unchanged).

**Outcome: 2 confirmed candidates, 1 latent, 8 killed, 4 parked.** Both candidates are tiny data patches (no engine hook logic), but both are unverified for visible impact until the user runs the Phase 3 test cards below.
- **L1 dialogue letterbox (confirmed, best candidate).** K2 has the K1 formula: bar = trunc((H - trunc(W / 2.3333330154)) / 2). At the user's 3840x1600 it is **-22 px** (bars vanish), 3440x1440 -> -17, 5120x1440 -> -377. Fix = write c = 1.5*W/H over the double at `.rdata` 0x009A7D30 (bar = H/6); 8-byte data write, no live-hook overlap. Residual: at 5120x1440 the 300 px reply panel exceeds the 240 px bar.
- **L4 glyph advance (confirmed, cosmetic).** Line-breaker adds 0.25 and truncates; Draw uses exact widths. Fix = re-point the 4-byte operand at 0x0047B234 from the 0.25 double (0x98C1D8) to the existing 0.5 double (0x987F08). Pure byte patch.
- **L15 last-line drop (latent).** Not hit at 3840x1600 with vanilla labels; park.
- **Killed:** L2 listbox inflation (cycle broken at every call site, Mod 6 unaffected), L3 wrap hang (100-line cap; silent text loss only), L5 minimap, L6 popup width cap, L7 QPC, L8 Linux symbols, L9 on this install.
- **New finding (L10):** `heads.2da` has 249 live rows against 254 usable byte-path ids; modders adding heads will hit the cap soon.
- **Method notes for the next phase:** the K1->K2 bridge (`engine_bug_research/tools/k1_to_k2_bridge.py`, `k1_k2_bridge.tsv`) resolved 77 of 104 KMRP K1 sites to Steam K2 addresses by name (validated on ClearItems, OrganizeControls, DrawMap, SetText, WrapStrings); all analysts were Sonnet, every top claim was re-checked by Opus against the asm and the vanilla exe (notes at the end of each V file). The community 2DA table is K1-only.
- **Next:** user runs test cards T1/T2 (and optionally T3-T5); then doc 16 gets a ranked list + a build sketch per pick and a user decision gate (Phase 4/5 of doc 15). Not built: anything.

**Phase 2 (2026-10-02, built + staged, NOT installed; details in `17_dialogue_letterbox_and_wrap_fixes.md`).** L1 became `k2-dialogue-letterbox-fix` 1.0.0 (DLL: rewrites the double at 0x9A7D30 to c = 1.5*W/H*(1-1e-9) from a hook in `CSWGuiManager::SetScreenSize` 0x411AC1, bar = H/6; INI `enabled/mode/log`) and L4 became `k2-wrap-measure-fix` 1.0.0 (one `simple` byte patch at 0x47B232). Both are in stage dir `~/Games/KOTOR2_Mod_Workspace/stage_engfix_1002_0121` (28 kpatches = the 26 live ids + the 2 new; PROBLEMS 0; game dir untouched). **Next:** the user installs from the stage dir (steps in doc 17) and runs T1/T2 once without (current live) and once with the staged patches; re-stage first if a sibling session changed the live set. Note: the plan's "3840x1600 -> 266" was an arithmetic slip; the engine formula gives **267**.

### S5 gate: Linux symbols (L8)
Unnamed-function counts: function-table rows needing a guessed or `FUN_` name: V1 8/19, V2 4/19, V3 10/18, V4 8/23, V5 5/24, V6 10/26 (about 35% overall), but every analyst's (f) table says a name would NOT have changed a verdict (V5 notes one case where it would help judge reachability of three UI timers, a non-candidate). The gate needs both conditions, so: **not needed; Linux depot download dropped.**


## Register
Status values: `open` (not yet analysed), `confirmed` (mechanism read in K2 code and reachable here), `killed` (K2 code differs / unreachable / not worth it), `parked` (needs a repro or user test).

| ID | Lead | Source | Status | Evidence | User-visible impact | Hooks needed | Risk | Kill / park reason |
|---|---|---|---|---|---|---|---|---|
| L1 | Dialogue letterbox sized from screen width; negative past 21:9 (user: 3840x1600 = 2.4:1) | KMRP K7 | **built + installed live 10-02 (doc 17), awaiting T1/T2** (was: confirmed (geometry); visible severity needs Phase 3 screenshot) | V1 | bar = trunc((H - trunc(W/2.3333330154)) / 2): 3840x1600 -> -22, 3440x1440 -> -17, 5120x1440 -> -377; bars vanish (SPECULATIVE whether a stray strip draws). K2 clamps reply panel/bottom edge, so only the bars are lost | 1 data write (8 bytes at .rdata 0x9A7D30: c = 1.5*W/H -> bar = H/6) or 7 operand re-points | low (no live overlap; semi-transparent-letterbox only alpha) | |
| L2 | `CSWGuiListBox` row-height inflation on repopulate | KMRP K2 | **killed** | V2 | none (cycle broken at every call site; Mod 6 path unaffected) | 0 | | K2 restores saved rects in ClearItems (flag bit 2) / row SetExtent discards height; Mod 6 uses restore=1 |
| L3 | Line-breaker infinite loop on unbreakable text | KMRP K1 | **killed** (no hang) | V3 | none on vanilla; mod-made labels narrower than 2 glyphs lose text silently | 0 | | K2 has the same bad progress compare but a 100-line cap (0x47B0E8) bounds it; no OOM |
| L4 | Glyph advance under-measured ~3% (text under scrollbars) | KMRP K6 | **built + installed live 10-02 (doc 17), awaiting T1/T2** (was: confirmed (cosmetic); needs Phase 3 check) | V3 | wrapped lines can be ~0.25 px/char wider than box | 1 data patch (re-point operand at 0x47B234 from 0.25 @0x98C1D8 to 0.5 @0x987F08) | low | |
| L5 | Minimap (KMRP K8 is a zoom mismatch, not a pan clamp; L5 as written is a misreading) | KMRP K8 | **killed** | V4 | none: radar/map sizes come from the GUI rects, live HUD keeps vanilla 120x120 / 512x256, live `k2-aspyr-map-aspect-fix` (0x7508AA) corrects the 4:3 stretch; no pan clamp in K2 either (as vanilla) | 0 | | not reproducible on this install |
| L6 | Message popup auto-fit capped at a 640x480-authored width | KMRP message-popup.md | **killed** for the width cap; **parked** for two UNCLEAR items | V4 | caps are `440*fs` x `280*fs` (fs = width tier: 2.0 at 3840), start width 1392 px > cap so the widening branch is dead; UNCLEAR: icon/datapad popup uses `fs*{400,300}` box (800x600 px at 3840x1600) and full map `CSWGuiInGameMap::Draw` 0x894CD0 hard-codes 640x480 centring | 0 | | park items need an in-game look (test cards T3, T4) |
| L7 | `QueryPerformanceCounter` frame-pacing lead | Peixoto's Patch | **killed** | V5 | none: one wrapper `Timer::Now` 0x73B810 returns int64 microseconds, all pacing subtracts int64 first | 0 | | Peixoto's relative-QPC fix would change nothing in K2. Side findings (profiling clock overflow, 49.7-day GetTickCount UI timers, 0x51A860 unsigned divide) are not user-visible |
| L8 | Symbols from the Aspyr Linux/macOS build | KPM | **not needed** | S5 gate below | | | | no analyst reported that a missing name changed a verdict; reopen only if a later phase is name-limited |
| L9 | 2DA token > 255 chars smashes 256-byte stack buffer (`0x71C3D0`/`0x71C180`) | R5 | **killed for this install** (defensive only) | V6, `engine_bug_research/scan_2da_limits_output.txt` | none today: all 424 BIF + 37 override tables are binary `2DA V2.b`; longest column name 23 chars; the binary path only copies column names to the stack; the hazard needs a hand-written text 2DA | 1 guard in 0x71C180 if ever wanted | low | would only protect hand-made text 2DAs |
| L10 | Consumer-side fixed caps in 2DA-backed tables | Gemini #3 | **partly confirmed**, informational | V6 | community table is K1 (not K2). K2 caps: appearance u16, portraits <0xFFFE, soundset/classes count truncation (65536 / 256 rows load as 0), heads byte path 1..254. **Live `heads.2da` = 249 rows, ~5 spare ids**; placeables/upgrade/baseitems have no K2 row cap | none (documentation) | | modder-facing note, build only if heads headroom runs out |
| L11 | Stuck idle/combat animation after conversation exit | Gemini #5 | parked | none | | | | needs a repro save |
| L12 | Save reset to Level 1 | Gemini #2 | parked | R4 (no leak, no stat overwrite) | | | | needs a repro save |
| L13 | Soft Shadows slowdown/crash | Gemini #4 | parked | strings only | | | | needs a user toggle test |
| L15 | Last text line dropped when taller than its box (KMRP K5); latent: vanilla labels safe to 3840x1600, 18-unit labels in character_p.gui vanish at 5120x1440 (SPECULATIVE metrics) | V3 | **confirmed (latent)** | V3 | none at 3840x1600 | 2 caves (9-byte @0x47B7DB, 11-byte @0x47B6D4, each re-emits a rel Jcc) | medium | not worth building unless resolution grows |
| L14 | Existing doc-10 perf candidates (F1, C2, TX, ...) | doc 10 | open (continues under doc 10) | doc 10 | | | | gated on perf-lab run A |

## Phase 2 risk screen + builds (2026-10-02)
Details: `17_dialogue_letterbox_and_wrap_fixes.md`; hook point analysis `engine_research_findings/V7.md`.
- **Hook overlap:** `engine_bug_research/tools/check_engfix_windows.py` (output `engine_bug_research/tools/check_engfix_windows_output_2026-10-02.txt`): the 4 windows (L1 0x411AC1/7 B, 0x8BAD7C/10 B, 0x8BAF01/10 B; L4 0x47B232/6 B) match the bytes in the exe, no overlap and nothing within +-8 bytes among 269 live hooks (09-30g snapshot 120 + live `patch_config.toml` 26 ids / 149 hooks). All 4 windows identical in the LAA and the base exe.
- **Cross-dependencies:** L1 vs `semi-transparent-letterbox` are orthogonal (alpha-only hooks at 0x8BAA5E/0x8BB043/0x8BB108/0x8BB3C3; L1 changes only the bar height). No GL state touched by either fix. 0x9A7D30 has exactly 7 readers (the 7 FDIVs), no other writer.
- **V7 hook point:** `CSWGuiManager::SetScreenSize` 0x411A80 is the only writer of the manager W/H (+0x6C/+0x6E), 4 callers (startup, options Apply, credits force/restore); hook 0x411AC1 (7 B) is after both stores and reached only on a W/H change. Rounding nudge (1-1e-9) needed (V7 Q5).
- **Builds:** L4 `k2-wrap-measure-fix` 1.0.0 (kpatch md5 11bfc231..., no DLL), L1 `k2-dialogue-letterbox-fix` 1.0.0 (kpatch md5 6033b700..., DLL md5 9bbbfcdd..., Wine smoke 9 modes pass; bars 3840x1600 267, 1920x1080 180, 3440x1440 240, 5120x1440 240, 800x600 100, 640x480 80). Commits `7d7e310` S1, `0fa8046` S2, `40e2161` S3.
- **Stage:** `~/Games/KOTOR2_Mod_Workspace/stage_engfix_1002_0121` (28 kpatches, PROBLEMS 0). NOT installed; awaiting the user + T1/T2.
- **Residuals:** 5120x1440 reply panel (300 px) sticks out 60 px above the 240 px bar (640x480: 100 vs 80); L15 stays latent; KMRP K1-style reply-panel stretch not done.

## Phase 3 test cards (user-run; nothing is launched by the session)
Evidence for each: V1-V6 under `engine_research_findings/`. The user runs these; the session never launches the game. If a card needs a probe it is log-only and listed.

**T1 (L1 dialogue letterbox, highest value).** At 3840x1600: start any conversation with a voiced NPC (e.g. the first Atton or Kreia conversation, or any dialogue on Peragus). Take one screenshot during an NPC line and one while the reply list is shown, once with `semi-transparent-letterbox` installed (as now) and once with it disabled in the Patch Manager. Look for: black bars at top/bottom (expected by the code: none), the NPC text position, the reply list position/scrollbar. Optional log-only probe (not built): dump `[letterbox+0x74]`, `[+0x6C]`, the reply panel rect at the end of `CSWGuiDialogLetterbox::SetBottom` (0x8BADA0) and `CSWGuiDialogCinematic::Reset` (0x8BC3E0); V1 section (c) lists the windows (SPECULATIVE until re-verified against asm).
**T2 (L4 text under scrollbars).** Open the character sheet / item description / Journal quest entry with a long description at 3840x1600; screenshot a wrapped paragraph next to a scrollbar. Look for characters touching or overlapping the scrollbar track on the right edge of wrapped lines. If nothing visible, L4 stays cosmetic/not worth a mod.
**T3 (L6 icon/datapad popup, UNCLEAR).** Trigger a popup with an icon (pick up a datapad / item-acquired / level-up message) and a plain confirm box (quit game dialog); screenshot both. Compare size with the layout we authored (datapad_p.gui is overridden 2.2x). V4 says `FixMessageLabel` forces an `fs*{400,300}` box (800x600 px at 3840x1600). Probe if needed: popup final-rect dump at 0x75C80F (V4 section (c)).
**T4 (full map 640x480 offset, UNCLEAR).** Open the full area map (M); screenshot. Check the map is centred (0x894D1A/0x894D34 subtract 640x480 under a `[this+0x48]&1` gate; the live `preserveAreaMapAspect` hook at 0x894435 may already compensate).
**T5 (L13 soft shadows, parked).** One-minute toggle: Options > Graphics > Soft Shadows on/off in a busy outdoor module (e.g. Telos surface), note FPS with the in-game counter or perf-lab. No code is involved; this decides whether L13 is worth research.
**T6 (L11/L12 repros, parked).** If either symptom occurs (stuck animation after leaving a conversation; party member reset to level 1 after load), keep that save, note module + party, and tell the session. No hooking work before a repro exists.
**Perf-lab (L14).** Continues under doc 10; run A S2/S3 data is still pending.


## Ranking
| Rank | ID | Why | Build size | Needs before build |
|---|---|---|---|---|
| 1 | L1 dialogue letterbox | confirmed arithmetic at the user's resolution (-22 px); K1 community fixed the same bug (bar = H/6) | 1 data write (8 B at 0x9A7D30; value depends on W/H so a DLL init or a patch that computes it) | T1 screenshots to confirm the symptom is visible and see what a negative bar draws; decide 5120x1440 reply-panel handling |
| 2 | L4 glyph advance | confirmed in code, tiniest possible patch (4-byte operand), shared with community fix | 1 byte patch | T2 (is it visible at all?) |
| 3 | L10 heads cap headroom | real, close (5 spare ids), but modder-only | none (doc) | only if more heads are planned |
| 4 | L15 last-line drop | latent at 5120x1440 only | 2 caves | resolution change |
| - | L5, L6, L7, L8, L2, L3, L9 | killed | | |
Parked evidence: L11 conversation-exit stuck animation (a repro save + module/party list; then log `ClearAllActions` callers on conversation end), L12 save reset to level 1 (a repro save; R4 found nothing in the save path), L13 soft shadows (T5 toggle; then a GL-state read of the soft-shadow pass 0x7B88B0/0x7B5F50/0x7D6170), L6 UNCLEAR items (T3/T4).

