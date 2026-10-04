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
