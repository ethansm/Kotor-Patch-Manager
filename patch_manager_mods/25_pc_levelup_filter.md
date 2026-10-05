# 22 - PC level-up filter (R10): design note (Phase 6b C2, 2026-10-03)

Status: DESIGN ONLY. Nothing built. Decision gate C3 (build or park) is the user's.
Static evidence: `research/companion_levelup/phase6_evidence/R10_findings.md` (C1, commit 66a6020). DB rows for the R10 identities are
being verified per `ghidra_knowledge/RE_AGENT_GUIDE.md` (C1b); until then, treat the addresses below as "R10_findings confidence".

## 1. Goal
The level-up override (doc 08) leaves the player character (the Exile) vanilla on purpose. R10 is a separate, opt-in feature: while the PC
levels up manually, the feat / power lists show only entries that fit a chosen build, so a "Jedi Sentinel dual-wield light-side" PC does not
have to scroll through ~150 feats and ~80 powers. The player still picks every entry (R1: manual picks are never overridden).

## 2. Filter source (what counts as "fits")
| option | what | pro | con |
|---|---|---|---|
| A. PC preset paths | a `paths/presets/pc__*.toml` like the companion presets; filter = the set of feats/powers the preset ever takes | reuses the table, check_paths.py, wizard schema and the options page | needs ~6-10 PC presets written + reviewed (B3-style); the set is rigid (one build) |
| B. Theme filter | tags per feat/power (light/dark/neutral, melee/ranged/force/tech/defence), the PC picks 1-3 themes | flexible, small data (one 2da-row tag table) | new data set to curate (~230 rows); themes are judgement calls |
| C. A + B | preset-derived set, plus a "show everything in theme X" toggle | best UX | most work |

Recommendation: **A first** (the data pipeline exists; companion presets give a model), B later if users want it.

## 3. Hide vs dim
- **Dim (grey)** = remove the id from the CHOOSABLE vector only: F2 0x90C2BE:5 (feats, in 0x90C230) and P2 0x9099A2:5 (powers, in 0x9098D0).
  The engine already greys non-choosable grid entries (state 3, 0x89A160) and clicking one shows the vanilla refusal popup. Smallest change, uses
  vanilla visuals, no navigation risk.
- **Hide** = also skip the grid row: F1 0x89B6E4:6 (in 0x89B600) and P1 0x89AB29:9 (in 0x89AAA0), TOGETHER with F2/P2.
  Shorter lists, but more risk: the empty-grid cursor case (0x89C290 reads `rows[0]`), and the arrow-key navigation over fewer rows (0x89C350
  skips -1 slots, so expected safe).
- Accept/selection is by id, never by index (0x90B430, 0x908B50, 0x89C170) -> removing rows or ids does not corrupt picks.

Recommendation: **dim first** (F2+P2 only). Hide is a later option once the probe shows the cursor guard is solid.

## 4. The accept trap (must handle)
- Feats accept refuses while picks remain (`+0x1BDE != 0`) AND choosable count `+0x1BC4 >= 1`; powers likewise (`+0x1BA8`, `+0x1BD8`).
- A hidden-but-still-choosable entry therefore soft-locks the panel (the player cannot see what to pick but cannot accept).
  -> Hide must act on BOTH the grid row and the choosable vector (F1+F2, P1+P2), never F1/P1 alone.
- If the filter empties the choosable vector while picks remain, vanilla accepts with picks unspent (a silent loss of a feat/power).
  -> **Fallback rule:** when the filtered choosable set would be empty while picks remain, do not filter that panel (show vanilla) and log
  `R10 FALLBACK`. Unspent picks would be worse than an unfiltered list.
- The **Recommended** button (0x90B6B0 -> AddDefaultPackageFeats 0x8489F0 / Spells 0x848AE0) calls CanChooseFeat / 0x848660 directly and
  ignores the vectors. Options: (a) leave it vanilla (documented: "Recommended ignores the filter"); (b) hook its callers 0x848AA8 / 0x848BF8 -
  `call` with stack args, RET n, not safely skippable (S1_gui_guards lesson). Recommendation: (a).

## 5. PC detection
- Server creature from the panel's client creature: `0x77D800` (ecx = `*(panel+0x68)`), the same call the engine makes at 0x90777B.
- Test: **empty Tag CExoString (server +0x18)** (live-confirmed at LevelUp time, doc 08; `LevelUpOverride.cpp` `readCreatureTag`), cross-checked
  by IsPC +0x10E8 in the probe. Do not use PlayerCreated +0x1200.
- Companions keep the vanilla lists (their level-up is handled by doc 08 presets; a manual companion level-up stays vanilla).

## 6. Skills
- 8 fixed rows, no list. Options: S1 veto rank-up at 0x90EDE0 (entry hook, fastcall, needs a consumed `ret`) or grey the row via 0x90F610.
- Recommendation: **out of scope for v1** (8 rows are already short; a veto that silently ignores a click is worse UX than no filter).

## 7. Config UX
- When the PC leads, the doc 08 options page (character screen -> Configure) gets one PC row: `Filter: Off | <preset> | ...`, stored per playthrough
  in `levelup_playthroughs.ini` as `pc=<preset>|filter=dim` (Playthrough.h line format; `pc` is not a companion tag, so the rewrite path never
  sees it).
- Default Off (vanilla). Off = no hook does anything.

## 8. Log-only probe (first build, PHASE7 step 1)
Hooks F1, F2, P1, P2 as LOG-ONLY (return 0, behaviour unchanged). Per panel open (feats 0x90AEA0, powers ctor 0x9074E0, skills ctor 0x90C6A0)
log: client creature id, server creature (0x77D800), its tag, +0x10E8, +0x1200, stats+0x6C, party slot, level (0x844860), isBonus
(`panel+0x1BE0` bit1); per F1/P1 call the id; per F2/P2 the id and whether CanChoose passed; row count after the build. Answers R10 open
questions 1, 2, 6 and 7. Question 3 (what state 3 looks like) needs one screenshot. Then a dim build behind `r10=1` with the fallback rule.

## 9. Effort / risk
| piece | effort | risk |
|---|---|---|
| probe (4 log hooks + panel-open logs) | S | low (log only; sites live == vanilla, +-16 clean) |
| dim build (F2/P2 + PC test + fallback) | M | low-medium (vanilla grey path; trap handled) |
| PC presets (A) + options row | M | low (reuses doc 08 pipeline) |
| hide (F1/P1) | S on top of dim | medium (cursor on empty grid, double feats panel) |
| skills | - | out of scope |
Testing: the Phase 6b toolkit applies directly: an offline T2 test drives the filter with fake vectors (the vector layouts are in R10 (a));
a forged save with a PC that can level (scenario S10 in `tests/luo_scenarios/saves.toml`) gets the game into the state in one load.

## 10. Questions for C3 (user)
1. Build R10 at all, or park it?
2. If build: filter source A (PC presets) / B (themes) / C?
3. Dim only, or dim then hide?

## 11. User proposal 2026-10-04: in-screen filter toggle (R10 UI)
Proposal (user-pasted): a button ON the feats panel (`CSWGuiFeatSelection`, ctor 0x909E00, init 0x90AEA0, grid +0x1BF4) and the powers panel
(`CSWGuiPowersLevelUp`, ctor 0x9074E0, grid +0x1BF8) cycling `Filter: [All] -> [Build (Dim)] -> [Build (Hide)] -> [Combat] -> [Force] -> [All]`,
refreshed in place, last choice remembered per playthrough (`feat_filter=` / `power_filter=`). Phases: probe (section 8) -> dim toggle -> hide toggle.
Same safety rules as sections 4-5 (fallback to All + `R10 FALLBACK` when picks remain and the filtered set is empty; PC gate = empty tag + IsPC).

Review notes (Opus, 2026-10-04; not yet planned):
- Fits this doc: it replaces section 7 (options-page row) with an in-panel control and implies source C (A presets for "Build", B themes for
  Combat/Force). Theme tags are still a new ~230-row data set.
- Dim refresh in place = re-run BuildChoosableFeatVector 0x90C230 then RefreshGridStates 0x90BD50 (both DB-verified C1b, CSWGuiFeatsCharGen).
  0x90BD50 is FEATS only - powers need their own pair (candidate 0x909360 `CSWGuiPowersLevelUp::RefreshGridStates`, check its C1b verdict;
  the proposal's powers "data build 0x908540" conflicts with a C1b claim naming it a feats function - verify before use).
  These would be NEW direct calls from a button handler (not hooks): call them deferred, outside the GUI click stack, like the level-up
  apply-now fix (doc 08, 2026-10-04).
- Hide toggle = rebuild the grid (0x89B600 / 0x89AAA0) on click: cursor/empty-grid risk from section 3 applies; keep it last.
- Persistence: Playthrough.h is a line format (`pc=<preset>|filter=dim`, section 7), not INI sections - use `pc=...|feat_filter=..|power_filter=..`
  rather than a `[PC]` section, or the playthrough parser/lossy checks need a change.
- The "<0.1 ms" and effort numbers are estimates, not measured. Section 10 questions (build or park; source; dim vs hide) still gate this.

## 12. Batch 1 (built, NOT installed) - 2026-10-04
Decisions: R10 lives inside level-up-override (shares preset pipeline, playthrough binding, log, GuiKit; a second DLL would need its own save
binding). Dim first, hide last; source = tags. Persistence uses Playthrough.h line fields, not a `[PC]` section.
| part | commit (KOTOR-II) | what |
|---|---|---|
| contract test | d89b511 | SafeRead pins (PHASE7 C2 / doc 24 T5d, 43 checks) |
| probe hooks | eb079ba | F1 0x89B6E4, F2 0x90C2BE, P1 0x89AB29, P2 0x9099A2: consumed-exit detours, LOG ONLY (always return 0), `r10probe=1` gate. Pure state machine `R10Probe.h` (r10_probe_test 37). Lines: `R10 panel kind= panel= bonus= client= server= tag="" ispc= stats6c= lvl= picks=` and `R10 done kind= panel= rows= choosable_seen=`; `R10 id ...` only with verbose=1 (cap 400 lines). P2 panel = `[ebp-0x68]` (disasm 0x9098F5); server = client+0xF8 cache (UNVERIFIED field). Flush on the 0x84F221 char-screen tick (may land early if that tick runs under the panel: then a duplicate session appears, harmless). |
| tag table | f68f2b1, 2be4856 | `runtime/r10_tags.ini` from `r10/gen_r10_tags.py` (82 feats, 73 powers, 0 preset conflicts, 16 overrides, 2 preset waivers); power alignment = spells.2da `goodevil`; Haiku spot-check 18/20 + decoys 3/3. Known limit: powers carry all 5 roles (only alignment narrows them), so Build mode is permissive. |
| filter logic | f782862 | `R10Filter.h`: modes All -> build_dim -> build_hide -> combat -> force, `fits()` = Python rule, `filterChoosable()` -1 = fallback (empty filtered set with picks left would let Accept skip the picks). Playthrough fields `|pcfilter=role/style/align|ffilter=<mode>|pfilter=<mode>` written only when set. r10_filter_test 204, playthrough 298, mutate 162/162. |
Build: KPM 06c67a4, LevelUpOverride.kpatch md5 2604a541 (dll 1c6208ea), previous kpatch kept as `LevelUpOverride.kpatch.100_c7a2bb6.bak`.
Gates 2026-10-04: run_tests 97/97, check_hook_bytes FAIL=0 (30 hooks), check_exports FAIL=0, parity/scenarios/mutate green, mingw 0 warnings.

**Install (user present, game closed):** hooks + exports changed -> full KPatchLauncher run with all live ids (a DLL swap is NOT enough); then
`r10probe=1` in levelup_override.ini; one feats + one powers level-up on the PC and on one companion; read the `R10 panel` / `R10 done` lines
(`kotor2_logs.py check <log> --new`). Decide the PC test from tag="" vs ispc vs stats6c per creature.

### Batch 2 outline (after the probe log)
1. Dim: at F2/P2 return 1 (consumed exit = skip the choosable push) for ids that `fits()` rejects; run `filterChoosable` semantics per panel
   (pre-count the choosable set, fall back to vanilla when the filtered set would be empty with picks left). PC only; `r10=1` gate.
2. Profile: PC wizard answers -> `pcfilter=` (Configure page); companions untouched.
3. In-panel `Filter:` button (GuiKit, feats + powers panels) cycling the mode, persisted in `ffilter=`/`pfilter=`; rebuild choosable on click.
4. Hide last: F1/P1 skip grid rows together with F2/P2 (never F1/P1 alone: accept soft-lock), empty-grid guard (R10_findings open question 2).
5. Recommended button ignores the filter (calls CanChooseFeat directly): document, or filter later.

## 13. Batch 2a: dim wiring (built, NOT installed, gated off) - 2026-10-04
Changed vs the section 12 outline: dim is NOT done at F2/P2 (consumed exit per id). Instead, two PLAIN detours after the build loops compact the
finished choosable vector in place, so one decision covers the whole set (fallback included) and the probe hooks stay log-only.
| part | what |
|---|---|
| sites | feats **0x90C36A** (in 0x90C230, panel `[ebp-0x28]`), powers **0x909B56** (in 0x9098D0, panel `[ebp-0x68]`): 7 bytes `c7 45 fc ff ff ff ff`, single loop exit, no inbound branch, no hook within +-16, live == vanilla. Evidence `research/companion_levelup/phase6_evidence/R10b_sites.md`. Vectors {ptr, count, cap}: feats `panel+0x1BC0` **ushort**, powers `panel+0x1BD4` **dword**. Callers clear the vector (0x90C670) or the builder de-duplicates (0x90C620 / 0x4142E0), then RefreshGridStates (0x90BD50 / 0x909360) greys ids not in it, so compacting each build is idempotent. |
| logic | `R10Filter.h` `dimInPlace<T>()` -> verdict off / nochange / fallback / noprofile / badcount / applied; never grows, writes only on applied; `build_hide` = `build_dim` this batch. r10_filter_test 265. |
| wiring | `LevelUpOverride.cpp` `r10Dim` / `LuoR10Dim_90C36A` / `LuoR10Dim_909B56`. Gate: `r10=1` and mode != off -> PC test (0x77D800 on `panel+0x68` client creature, server tag EMPTY; any read failure = skip) -> tags `r10_tags.ini` in the directory of `table=` (re-read on stamp change; error -> `R10 tags error line=N <reason>`, dim off). |
| config | `r10=0|1`, `r10.profile=<role>/<style>/<align>`, `r10.feats=` / `r10.powers=` `all|build_dim|build_hide|combat|force`. |
| log | `R10 dim kind= panel= mode= before= after= picks= fallback= verdict=`; `R10 dim skip kind= panel= reason=not_pc|unreadable|picks_unreadable`; `R10 config ...`; `R10 tags loaded <path> feats= powers=`. |
| tests | gate_test.cpp R10 block (fake panel, 10 feat ids, powers dword, r10=0 / PC / companion / null creature / fallback picks 1 vs 0 / all / bad profile / mode=off / idempotent rebuild), 209 checks incl. -m32. mutate 173/173 (RF21-25, RD01-06 new). |
Build: version 1.2.0-dev, LevelUpOverride.kpatch md5 7c687dec (dll 66392e8d), previous kept as `LevelUpOverride.kpatch.110probe_2604a541.bak`.
Gates: run_tests 97/97, check_hook_bytes FAIL=0 (32 hooks), check_hook_spans FAIL=0, check_exports FAIL=0, parity/scenarios(-m32)/options/wizard/guikit/
simulate/check_paths/gen_r10_tags --check green, mingw 0 warnings.

**Live test (user present, game closed, consent):** one full KPatchLauncher run carries probe + dim (hooks changed). Run 1: `r10probe=1`, `r10=0` ->
confirm tag="" only for the PC in the `R10 panel` lines. Run 2: `r10=1`, `r10.profile=melee/single/light` (or the PC's build), `r10.feats=build_dim`,
`r10.powers=build_dim`; PC level-up: non-fitting feats greyed, `R10 dim ... verdict=applied`; companion: `reason=not_pc`. Recommended button still
ignores the filter (section 12 item 5). Next offline: batch 2b = in-panel `Filter:` button + save-bound `pcfilter=` (needs panel GUI RE).

## 14. Batch 3a: character-screen UX for the PC filter + settings icon (built, NOT installed) - 2026-10-04
User direction: a level-up settings UX for the PC's filter, sharing the companion options page; the big `Configure/Change path` button only while
the engine shows BTN_LEVELUP/BTN_AUTO, otherwise a small icon with a tooltip in a corner of the 3D view.
| part | what |
|---|---|
| icon | `C_ICON` (framed label, inserted before the tip controls) at unit rect [492,172,28,28], top-right of LBL_3DCHAR; texture `lbl_icn_opt2` (vanilla hammer "Options" icon, swpc_tex_gui.erf), ini `ui.icon=<resref>`, text "LU" if the image fails. Recon `research/companion_levelup/r10/ui_icon/README.md` (overlap PASS; BTN_3DCHAR covers the view, our own hit test runs first). Tip "Level-up settings", box kept in units 350..525 x 134..255. |
| rule | `lvl = heldResult != 0` (SetStats' CanLevelUp/held result = the engine's BTN_LEVELUP visibility). lvl -> C_CFG shown, icon hidden; else icon shown, C_CFG hidden; page open -> both hidden. Log `UI lvlbtn shown=%d isVisible=%d` on change (cross-check of control 66). The R9 nudge only colours a shown button. |
| PC | `uiReadTag`: readable creature + empty tag = PC (r10IsPc's rule). PC button text "Level-up filter"; C_CFG/C_ICON open the R10 page (`pcPage`) in C_BG0..2, C_TITLE "Level-up filter", C_ROW0..5 = `R10Page.h` rows (Filter on/off, Role, Style, Alignment, Feats, Powers; modes all/build_dim/combat/force), Apply, Cancel. Start = playthrough `pc` line, else ini r10* when r10=1, else default (off, melee/none/none, build_dim x2). Status "Level-up filter: on - feats X, powers Y" (hidden when off). |
| save | `Playthrough.h`: reserved key `pc` + preset `r10`, new field `|r10=1` (written only when on: old files byte-identical), `findPc` / `setPc` (create-or-update, refuses lossy). `ptSetPc()` in LevelUpOverride.cpp (GUID via the new `ptEnsureGuid()`, shared with ptApply). `rebuildPtCfg` skips the `pc` line. |
| dim | `r10Effective()`: the GUID's `pc` line wins (on/profile/modes); no line -> ini r10* keys (dev override). Log `R10 config source=playthrough|ini on= profile= feats= powers=` on change. |
| tips | Native tip strings for C_CFG/rows/Apply swap per creature kind (`uiTipKind`, log `UI tips kind=pc|companion native=N`) on a kind change in uiTick, never from uiClick (cursor is on the party selector then). Deliberate deviation from "native tips only in uiAppend"; verify live with TIPPROBE. |
| tests | r10_page_test 1222 (run_tests + parity.py), playthrough_test 315, ui_test 687 (CFG/ICON switch, PC page open/cancel/apply/reopen/off, tip swap), gate_test 213 (pc line on/off/all vs ini), scenarios 22 (cfg click -> icon when no level-up), gui_ids count 25 (ids 151..175). mutate 184/184 (RB01-11 new, RD02/RD04 retargeted). |
Build: 1.3.0-dev, LevelUpOverride.kpatch md5 8dfce2de (dll 545e330b), previous kept as `LevelUpOverride.kpatch.120dev_7c687dec.bak`; KOTOR-II 4d0fb1e/9a2e705/7caa75b, KPM de587d3.
Gates: run_tests 104/104, check_hook_bytes FAIL=0 (32 hooks, unchanged), check_hook_spans / check_exports FAIL=0, parity/scenarios(-m32)/options/wizard/guikit/simulate/check_paths/gen_r10_tags --check green, mingw clean.

**Live test (user present, consent; one reinstall carries 2a + 3a):** (1) character screen, no level-up: icon top-right of the 3D view + tooltip, no
Change path button; check the icon draws over the 3D model (not live-verified) and that clicking it does not rotate the model. (2) Level-up available:
Change path under Level Up, icon hidden. (3) PC: icon -> filter page -> On + profile -> Apply -> `pc=r10|...|r10=1|...` in levelup_playthroughs.ini ->
PC feats level-up greyed entries + `R10 config source=playthrough` + `R10 dim` lines. Next offline: batch 3b (in-panel `Filter:` button, needs panel GUI RE).

**Live result 2026-10-04 ~16:15 (1.3.0-dev 8dfce2de, installed by the C1/C2 reinstall, backup `$G/backups/c1c2_reinstall_20261004_1611`; log from line 629):**
icon fill `lbl_icn_opt2` ok=1, 25 controls appended, `UI tips kind=pc native=8`; PC icon -> page -> Cancel / rows / Apply -> `pc=r10|...|pcfilter=melee/double/light|ffilter=build_dim|pfilter=build_dim`
written (r10=0: On was toggled twice) and reloaded (`start=playthrough`). Native tooltips CONFIRMED live (`TIPPROBE ... owner_is_ours=1 shown=1 ctl40=1`).
Bugs: banner still says `Version = "1.2.0-dev"` (LevelUpOverride.cpp:93); first `UI lvlbtn shown=0 isVisible=1` (likely a stale ctor-time flag, recheck).
User feedback (batch 3c): gear icon instead of the hammer; the icon is stretched (not square: sx != sy on this screen); the config page needs a proper
bordered window that fully covers the menu underneath. No screenshots were taken.

## 15. Batch 3c: gear icon (embedded), square icon, framed config window (built, NOT installed) - 2026-10-04
User feedback after the 3a live test (s14 "Live result"): gear instead of the hammer, square icon, a bordered window that fully covers the menu.
Gear art matched to the hotbar wrench family (same gradient / outline / glow / rim recipe) at the user's request.
| part | what |
|---|---|
| gear | `scripts/gen_luo_textures.py` -> `level_up_override_src/LuoTextures.inc` (`luo_gear` teal, `luo_gear_hi` gold, 64x64 32-bit TGA; previews + `family_compare.png` in `research/companion_levelup/r10/ui_icon/`; `--check` = deterministic). Registered at the first UI tick by the new shared `EmbedTex.h` (`embtex::ensure`; `heal()` every uiTick; Steam-only). Default `ui.icon=luo_gear`; fallback `lbl_icn_opt2`, then text "LU". FILLSTYLE 2 (stretch) via new `guikit::setFillStyle` (border+0x34 bits 0-1). Hover swaps the fill to `luo_gear_hi` on a change only. Logs `tex luo_gear: created ...`, `UI icon fill=%s ok=%d` (unchanged) + new `UI icon emb=%d hi=%d style=%d`. |
| square | `uiIconRect(Scale)`: side = round(28 * sy), right edge pinned at unitX(520), top unitY(172); used by `uiPlace` and the tip anchor (`uiRectUnits(r, i, &sc)`); the hit test reads the placed extent. |
| window | `C_BG0` tries Mod 14's `LBL_EFX_BACK` first (opaque frame `uibit_brdr_16wc/16we`, fill `uibit_fill_2bt`): `r->win`, own frame/fill kept, alpha 1, BG1/BG2 stay hidden (`uiShowBack`). Without Mod 14: the 3-stack as before. Same rect [54,134,246,336] (x <= 300). Logs `UI page window=efx|stack`, `UI page window back alpha ok=%d`. The failed template try costs one extra label alloc per panel ctor. |
| bugs | Banner `Version` 1.2.0-dev -> 1.4.0-dev. `UI lvlbtn shown=%d isVisible=%d` is now logged on the tick AFTER the change (the engine writes bit1 after our hook); format unchanged. |
| tests | embtex_test (create / upload / reset refill / missing rebuild / give-up 20 / bad TGA / non-Steam), guikit_test (setFillStyle), ui_test 713 (section 7: wide-screen square rect + anchor, gear + hover once, idempotent second panel, two fallbacks, window vs stack, heal refill + give-up, lvlbtn timing), parity/scenarios(-m32)/options/wizard/guikit green, mutate 189/189 (IC01-IC05 new). gui_ids unchanged (no new control). |
Build: 1.4.0-dev, LevelUpOverride.kpatch md5 978b39fd (dll d14fa7cd), previous kept as `LevelUpOverride.kpatch.130dev_8dfce2de.bak`; KOTOR-II 0044115/a111b83/0972045/dbabc2b/889e5ba, KPM 6015fe4.
Gates: check_hook_bytes FAIL=0 (32 hooks unchanged -> DLL hot-swap install), check_exports FAIL=0, check_shared_headers 0 drift, mingw -Wall -Wextra clean.

**Live test (user present, game closed, GO; tell the other sessions first):** hot-swap `$G/patches/level-up-override.dll` (back up to
`$G/backups/luo_3c_<HHMM>`, md5 check). (1) Gear square in the 3D-view corner, gold on hover. (2) Options/filter page = one framed opaque window,
no stats show through. (3) Log: `tex luo_gear: created ... queued`, `UI icon emb=1 hi=1 style=1`, `UI page window=efx`, banner `1.4.0-dev`.
- **INSTALLED 2026-10-05 00:28:** 1.4.0-dev dll d14fa7cd hot-swapped into `$G/patches/level-up-override.dll` (md5 verified); previous 545e330b backed up to `$G/backups/luo_3c_0028`. Awaiting the user live check; read `levelup_override_log.txt` from line 794.
- **Live result 2026-10-05 ~01:00 (1.4.0-dev d14fa7cd, log 794-825):** all new log lines as designed (window=efx, gear textures created, emb=1 hi=1 style=1,
  lvlbtn consistent). User: gear looks good. Batch 3d feedback: gear needs the button fill behind it; highlight the border on hover; keep the gear while
  the page is open and make it act as Cancel; page = solid background, rounded border, vanilla colours (the EFX window and row fills are see-through live).
  Handoff `.claude/handoffs/25_pc_levelup_filter.handoff.md`.

## 16. Batch 3d: gear on a button, vanilla hover highlight, gear closes the page, opaque vanilla-framed window (built, NOT installed) - 2026-10-05
User feedback on 1.4.0-dev (s15 "Live result"); screenshots `~/Documents/Screenshot_2026-10-05_01-01-13.png` / `_01-01-26.png`.
| part | what |
|---|---|
| gear | New `C_GEAR` (plain `LBL_EXPERIENCE_STAT` clone, right after `C_ICON` = drawn on top) carries the gear fill (stretch, alpha 1, click-through re-set after each show), placed by `uiGearRect` = `uiIconRect` inset round(side * 0.18). `C_ICON` keeps its framed template fill (looks like the rows / Apply). Hover swap (`luo_gear_hi`) moved to `C_GEAR`. Embedded failure or `ui.icon=<other>`: the image goes on `C_ICON` as before, `C_GEAR` hidden (`r->gearOk`). |
| hover | `guikit::setBorderTint` (border+0x28; Draw 0x415300 tints corners, edges and fill). `uiHoverTint`: hovered clickable control -> `kHiTint` 0.8/0.8/0.69804 (vanilla BTN_LEVELUP HILIGHT COLOR), previous one -> its base COLOR read once in `uiAppend`; restored when the control is hidden (`uiShow`). New log `UI hover tint base_ok=%d n=%d/%d`. |
| close | `uiClickable(C_ICON)` = shown (page open or not); icon + gear shown on every open page (PC and companion, any `lvl`); click while open -> `uiClosePage` like Cancel, new log `UI gear close tag=%s`. The icon's tip box stays in the 3D-view region while the page is open. |
| window | `luo_fill` (8x8 opaque RGB 2,8,6, 3rd `kLuoTex`) stretched over BG0 (efx window or the LBL_XP_BACK clone), alpha 1; frame + COLOR kept. `r->oneBack = win \|\| fillOk` hides BG1/BG2. Fill failure: 3-stack as before. New log `UI page fill=luo_fill ok=%d style=%d one=%d`. Rect unchanged [54,134,246,336]. |
| tests | guikit_test (setBorderTint), ui_test 739 (section 8: gear rect/fill, hover tint enter/once/leave/follow/hidden, icon closes companion + PC page and writes nothing, one back on the 3-stack, fill failure -> 3-stack + fallback icon), parity/scenarios(-m32)/options/wizard green, mutate 194/194 (HL01-HL05 new, RB07 regex updated). gui_ids count 26. |
Build: 1.5.0-dev, LevelUpOverride.kpatch md5 cb0d73bf (dll 91bbda85), previous kept as `LevelUpOverride.kpatch.140dev_978b39fd.bak`; KOTOR-II d2856d9, KPM 540b91f.
Gates: check_hook_bytes FAIL=0 (hooks unchanged -> DLL hot-swap install), check_exports FAIL=0, check_shared_headers 0 drift, mingw -Wall -Wextra clean, gen_luo_textures --check 0.

**Live test (user present, game closed, GO; tell the other sessions first):** back up to `$G/backups/luo_3d_<HHMM>`, copy `binaries/windows_x86.dll`
from the kpatch to `$G/patches/level-up-override.dll`, md5 91bbda85. Read `levelup_override_log.txt` from line 886.
(1) Gear on a teal button fill; cream border on hover (rows, Apply, Cancel too). (2) Page open: gear still there, clicking it closes the page.
(3) Page solid (no stats / column lines through it), rounded teal frame. (4) Log: `UI page fill=luo_fill ok=1 style=1 one=1`, `UI hover tint base_ok=1`, banner `1.5.0-dev`.
