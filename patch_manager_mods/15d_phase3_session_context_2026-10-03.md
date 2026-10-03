# Session context: doc-15 Phase 3 (L1/L4 validation + KMRP static triage) (2026-10-03)

**SUPERSEDED for next actions by `15e_phase4_session_context_2026-10-03.md` (Phase 4 done: L4 dropped, V11, L1 1.0.1 built, stage A/B).**

Companion to `15c_phase2_session_context_2026-10-02.md` (Phase 2: owned files, build/smoke facts), plan
`engine_bug_research/PHASE3_PLAN.md` (verbatim user-approved plan), `17_dialogue_letterbox_and_wrap_fixes.md` (build doc, STATUS at top),
`16_candidate_register.md` (register L1-L23, test cards T1-T6). Branch `KOTOR-II`.
Commits this session: `bfd543e` Step 0, `719f034` B0/B1 briefs, `53ccd55` A1 analyzer, `a9bece5` V8b+V9, `31381e5` V10, `2001983` V8,
`fbcfe26` B4 (register/community doc/DB), `88b7ea1` doc 17 T1 correction, `d22b715` 1.0.1-probe.

## 1. Where the plan stands
| Step | State |
|---|---|
| Step 0 (plan copy, T1 finding recorded) | DONE |
| A1 analyzer `engine_bug_research/tools/analyze_engfix_logs.py` + `test_analyze_engfix_logs.py` (22 tests) | DONE; fixtures `engine_bug_research/fixtures/letterbox_log_1002_0238.txt` (no dialogue yet), `letterbox_log_1003_0112.txt` (63 lines, 13 launches, 12 dialogues, PASS) |
| A2 user visual check | **OPEN.** 10-03 session 01:30-02:00 had 0 dialogues, 0 screenshots, no INI (copy `~/Games/KOTOR2_Mod_Workspace/playlogs_engfix_1003_0200/`). Need ONE F12 screenshot during a conversation |
| A2 V8b + 1.0.1-probe | DONE, **staged only, NOT installed** |
| A3 decision gate | waiting on the screenshot |
| A4 1.0.1 trim (log=0 default keeps first dialogue only, version bump, smoke, checker, re-stage, user-gated install) | NOT started (only after T1 visual pass) |
| B0 fetch, B1 briefs, B2 V8/V9/V10, B3 Opus gate, B4 persist | ALL DONE |
| B4.5 build sketches | none needed (0 user-visible bugs); L16 latent only |

## 2. Live state (re-read 2026-10-03)
- Live set: **31 ids / 198 hooks**, `kpm_install_state.json` UpdatedAt 2026-10-03 00:24 (sibling reinstall). Both `k2-dialogue-letterbox-fix` 1.0.0 and
  `k2-wrap-measure-fix` 1.0.0 still installed. No `dialogue_letterbox_fix.ini` in the game dir (defaults enabled=1 mode=h6 log=1).
- **Phase 4 re-read (10-03 ~11:45):** still 31 ids, but UpdatedAt **2026-10-03 11:29** (sibling hotbar build 28 reinstall), `swkotor2.exe` md5 `62411a3a3282ecf92bdf54aeb1bac671`. `stage_lbprobe_1003_0200` is STALE.
- **User decision 10-03: DROP L4 (`k2-wrap-measure-fix`) at the next install** (no-op at fs 1/2/3); install id list = live ids minus it (doc 17 Install).
- Game was closed at ~02:00. Never launch it; installs only on the user's explicit go, re-stage first (re-read UpdatedAt + id list).

## 3. Verified facts (Opus-checked against asm/bytes this session)
- **Letterbox draw path (V8b):** Draw 0x8BB260 slides per frame `trunc(2*(target-origin)*dt)` (~0.5 s); vtable +4 SetExtent 0x8BAC30: mode 1 forces y=0,
  mode 2 sets h = H - y; base panel Draw 0x40EC90 skips if bit6 of [+0x48] clear, x<0, y<0, h<=0 (0x40ECC8), w<=0, or off-screen; fill 0x479460 = one
  untextured quad, alpha [+0x50] (0.5 with semi-transparent-letterbox). Drawn for every ConversationType except 1 (computer).
- **Vanilla at 3840x1600 draws NO top bar and a 200 px bottom band** (bottom y = min(H+22, H-100fs) = 1400; h = H-y). L1 gives 267 top + 267 bottom.
  The A/B tell is the TOP band. (Corrects V1 B5 and old T1 text; doc 17 + register T1 updated.)
- 0x8BB467 writes top-bar h to `[0xA309B4]`, 0x8BB531 bottom-bar y to `[0xA309B8]`; 0x7CEB80 sets `[0xA309BC]` = (arg==1). Consumers not traced.
- **L4 is a no-op at fs 1.0/2.0/3.0** (V8 + independent pykotor recomputation over stock `TexturePacks/swpc_tex_gui.erf`): glyph advances are within
  0.0005 px of integers, +0.25 and +0.5 agree on all 95 ASCII glyphs; only fs 1.5 (2046-3069 px wide) differs (14/29 glyphs). Override has no font files.
  Caveat: breaker scale `[ECX+0x54]` assumed = fs. Decision 10-03: drop L4 at the next install.
- Breaker advance (0x47B203..0x47B238): `((lr.u[+0x24]-ul.u[+0x18])*texW[+0xC] + spacingR[+0x10]) * scale[+0x54] * 100.0 + 0.25`, `_ftol2`.
- **Bink (V9, L16 latent BUG):** 0x40CC70 one scale `clientW/movieW` (FDIVP 0x40D262, FSTP [ebp-0xB4]) used for width (0x40D28B) and height (0x40D2BE).
  All 85 installed .bik are 3440x1440 -> 3840x1607 (7 rows cropped, invisible). Window 0x40D26A 6 B `8B 85 10 FF FF FF`. L17 equal-width unsigned offset latent.
- **Texture downsample compiled out (V10):** 0x4258B9 `E9 E4 04 00 00 JMP 0x425DA2` right after the prolog (bytes confirmed in the vanilla exe). Budget
  134,217,728 x 0.75 = 100,663,296; ~205 MB resident, nothing downsampled. Do not "fix".
- Button focus (V10): Draw 0x41A0A0 uses focus border +0xE8 instead of +0x70 when [+0x48]&1; inset only with a CORNER (0x4153A0). Authored design.
- K3 leading newline (V8): K2 builders append "\n" -> no bug. Listbox client rect subtracts scrollbar width (0x41D710) like K1, by design.
  0x413CE0 force-appends `b` to font names (cosmetic; explains K1 "10x10 renders as 16x16").

## 4. 1.0.1-probe (staged, NOT installed)
- Mirror `dialogue_letterbox_fix_probe_src/`, kpatch in `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/letterbox_probe_stage/` (moved out of `Patches/` 10-03, doc 20d F1; rebuild steps in the probe README)
  (same id `k2-dialogue-letterbox-fix`, version `1.0.1-probe`). kpatch md5 `4d68098658809e25307117a9bb33f460`, DLL `e46cfcc8d5684de395d2c472f764deb0`
  (KERNEL32 + msvcrt). Base 1.0.0 build dir + `dialogue_letterbox_fix_src/` untouched.
- Windows: W1 0x8BB461 (6), W2 0x8BB52B (6), W3 0x8BB538 (9), W4 0x40ED9F (5, hot base Draw, vtable 0x9A7CBC filter), W5 0x7CEB95 (7). INI `probe=1` (default 0);
  rate limit 120 records/window/dialogue then 1/s, cap 3000. Wine smoke 12 modes pass. `check_engfix_windows.py` (9 windows) ALL OK + PROXIMITY OK
  (output `engine_bug_research/tools/check_engfix_windows_output_2026-10-03.txt`); `verify_windows_both_exes.py` ALL MATCH.
- Stage `~/Games/KOTOR2_Mod_Workspace/stage_lbprobe_1003_0200` (31 kpatches, PROBLEMS 1 = known `4gb-patch` hooks=DIFF artifact). Re-stage if the live set changed.
- Opus review: all new lines read; predicate checks w before h (engine checks h first) = only the reported term differs when both fail.

## 5. Next actions
**Phase 4 (2026-10-03) plan: `engine_bug_research/PHASE4_PLAN.md`** (everything except the user's T1 screenshot: L4 drop, V11, 1.0.1 pre-build, analyzer probe branch, stage A/B).
1. User: one F12 screenshot during a conversation (bright area). Optional A/B: `enabled=0` in `$G/dialogue_letterbox_fix.ini` (game closed), repeat, delete.
   Steam screenshots: `~/.local/share/Steam/userdata/62703561/760/remote/208580/screenshots/` (dir did not exist on 10-03; if F12 does nothing, find another capture).
2. After the session: copy log + screenshots to `~/Games/KOTOR2_Mod_Workspace/playlogs_engfix_<MMDD_HHMM>/`, run `analyze_engfix_logs.py` on the copy, Read the images.
3. A3: top band visible -> A4 1.0.1 trim + archive probe; no bands -> ask the user to install 1.0.1-probe (re-stage first), one conversation, analyse PROBE lines per V8b (c).
4. ~~Ask the user: keep or drop L4 at the next install.~~ DECIDED 10-03: drop (doc 17 DECISION, register L4).
- Out of scope (unchanged): upstream KPM PR (draft `engine_bug_research/upstream_kpm_k2_status_note.patch`, user-gated), L15, GOG port.

## 6. Gotchas
- A "wait for game exit" watcher using `pgrep -f 'swkotor2\.exe'` matches its own bash command line and never fires; use `pgrep -x` on the process name or exclude `$$`.
- Never stage `research/companion_levelup/logs/*`, `ranged_ai_src/script/kite_routine.nss`, `steam_db_pr3/*`, `force_power_hotbar_src/*` (sibling-owned edits).
- KPM clone stays uncommitted; repo `*_src/` mirrors are the committed copies.
- Knowledge DB: `persist_phase3_db.py` is idempotent (second run adds 0); pre-run copy in the session scratchpad only.
