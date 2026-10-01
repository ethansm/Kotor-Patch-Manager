# Engine bug / improvement candidates: research plan (2026-10-01)

Goal: turn a pile of unverified leads into a ranked, evidence-backed list of Patch Manager mod candidates.
Nothing here is built or installed. Every lead below is **unverified for K2** until a phase says otherwise.

## 1. Lead inventory (where each came from)

| ID | Lead | Source | Prior repo work |
|---|---|---|---|
| L1 | Dialogue letterbox sized from screen width; breaks past 21:9 (user runs 3840x1600 = 2.4:1) | KMRP (K1) | `semi-transparent-letterbox` installed (visual only); `0x4113A0` checked for GL state only (`shield_bar_src/blackout_investigation.md`) |
| L2 | `CSWGuiListBox` row-height inflation on repopulate | KMRP (K1) | ~40 `CSWGuiListBox` functions named in the knowledge DB; never checked for this bug. Mod 6 repopulates this list. |
| L3 | Line-breaker infinite loop on unbreakable text (crashes if allocation fails) | KMRP (K1) | none |
| L4 | Glyph advance under-measured ~3% (text under scrollbars) | KMRP (K1) | doc 14 only assumes uniform glyph scale |
| L5 | Minimap pan not clamped | KMRP (K1) | `k2-aspyr-map-aspect-fix` covers aspect, not pan |
| L6 | Message popup auto-fit capped at a 640x480-authored width | KMRP (K1) | none |
| L7 | `QueryPerformanceCounter` frame-pacing lead (Jade Empire fix) | Peixoto's Patch | 5 QPC import sites in the K2 exe; never examined |
| L8 | Symbols/names from the Aspyr Linux (or macOS) build | KPM macOS port, NWNX:EE | doc 06 dismissed the Linux build as not applicable; symbol presence never checked |
| L9 | 2DA token over 255 chars smashes a 256-byte stack buffer (`0x71C3D0`/`0x71C180`) | R5 / candidate TD | found, not built; no offline scan of Override + BIF 2DAs done |
| L10 | Consumer-side fixed caps in 2DA-backed tables (`heads`, `appearance` lookups, etc.) | Gemini #3 (premise corrected by R5) | `upgrade.2da`/`placeables.2da` caps noted in `CRASH_INVESTIGATION.md`; consumers never audited |
| L11 | Stuck idle/combat animation after conversation exit | Gemini #5 | nothing; needs a repro |
| L12 | Save reset to Level 1 | Gemini #2 | R4 found no leak and no stat overwrite on the save path; needs a repro |
| L13 | Soft Shadows slowdown / crash | Gemini #4 | strings only (`0x9A01C4`, `0x9A0418`, `CB_SOFTSHADOWS`); K1-only `SoftShadowsFix` exists; user has an NVIDIA card |
| L14 | Existing doc-10 perf candidates (F1 PlayMovies spin, C2 Demand Sleep(1), TX texture pinning, etc.) | doc 10 | researched, gated on perf-lab run A |

Already rejected, do not reopen without new evidence: CPU-affinity pinning mod (doc 09 ranks `taskset -c 0,6` as the top hang suspect), K1-style `SaveGameMemoryLeak` free hook (R4: would double free).

## 2. Evidence standard (what promotes a lead to a candidate)

A lead becomes a **candidate** only when all four hold:
1. **Located:** the K2 function/address is named, with decompile and asm excerpts saved under `engine_research_findings/` (same format as R*/U*), and cross-checked against the name map.
2. **Mechanism confirmed statically:** the buggy behaviour is read from the K2 code, not inferred from the K1 write-up.
3. **User-visible impact:** either reproduced in-game on this install, or the code path is provably reachable with our content (resolution, strings, mods).
4. **Patchable:** a hook window exists with no relative branches in the stolen bytes and no overlap (+-8 bytes) with the live hook set (the Q8 scan in doc 08).

A lead is **killed** if the K2 code differs, the path is unreachable on this install, or the fix would need more than about 3 hooks for a cosmetic gain.

## 3. Phases (run in order; verify and commit between phases, no background job overlapping a coding phase)

### Phase 0: Register and baseline (about 1 hour)
- Create the candidate register (one table: ID, hypothesis, source, status, evidence link, impact, risk, hooks needed, kill reason).
- Pull KMRP's `reverse-engineering/` docs (`binary-inventory.md`, `listbox-geometry.md`, plus the line-breaker, popup and map notes) into the scratchpad so K1 addresses and behaviour descriptions are on disk, not re-fetched each time.

### Phase 1: Cheap, high-leverage static checks (no game launch)
1. **L8 symbol check first.** Download the K2 Linux depot with `steamcmd` (platform override) into a scratch dir and run `readelf`/`nm`. If symbols exist, extract names and add them to the name map by constant delta or call-graph matching. This could speed every later phase, so it goes before them. Ask before touching the Steam install or login.
2. **L1 letterbox.** Decompile `0x4113A0` and its callers. Find where the bar height/width derive from screen size. Answer: does anything go negative or too small at aspect > 21:9? Compute for 3840x1600.
3. **L2 listbox.** Read `OrganizeControls` `0x41EA50`, `AddControls` `0x41FC30`, `ClearItems` `0x41D290`. Look for row heights accumulating across repopulations. Then check what Mod 6's buffer-and-flush does to the same fields.
4. **L3/L4 text layout.** Locate the line-breaker and glyph-advance code via `CAurGUIStringInternal` functions and the `fnt_*` measurement path.
5. **L5/L6.** Locate minimap pan (`FUN_00750790` region, already mapped for the aspect fix) and the popup width loop.
6. **L7.** List the five QPC call sites, what each feeds (frame timer, movie, audio), and whether a start-of-process offset matters under Wine.
7. **L9 offline scan.** Script that scans Override + game BIF 2DAs for tokens over 255 chars. Read-only, minutes to run. Settles whether TD matters on this install.
8. **L10.** Sweep the consumers of the 2DAs our mods extend, looking for fixed-size arrays sized by constants. Start with the tables in `CRASH_INVESTIGATION.md`.

Delegation: the sweeps (L3-L6, L10) are routine and can go to Sonnet subagents with the R* brief/CONTRACT format. The orchestrating model designs the checks and verifies every claimed address against the asm before it enters the register.

### Phase 2: Interaction and risk screen
- Run the Q8 hook-overlap scan for every surviving lead against the live 19-id hook set.
- Note cross-dependencies: L2 and Mod 6 share list code; L1 and L13 touch the render/GUI GL state that caused the shield-bar blackout, so any fix there needs the CapGuard-style state restore.
- Note which candidates are pure bug fixes (small, low risk) vs behaviour changes (need design).

### Phase 3: Dynamic confirmation (user-run; log-only probes first)
Only for leads that survive Phase 1 and are plausibly visible:
- L1: open dialogue at 3840x1600, with the letterbox on and off; screenshot.
- L2: open the inventory, apply and cycle filters repeatedly, close and reopen; watch row spacing.
- L3/L4/L6: a log-only probe that records the measured vs drawn width for long strings and popup sizes.
- L7: frame histogram from perf-lab (already built) to see whether pacing is irregular.
- L11/L12: ask the user to keep a save whenever either symptom occurs, with the module name and what was in the party. No hooking work until a repro exists.
- Never launch the game from the session; the user runs every test. Log thoroughly from the first build (exhaustive logging, trim later).

### Phase 4: Scoring and the candidate list
Score each surviving lead on user-visible impact (does it affect this install), confidence (evidence standard met), hooks needed, regression risk, and overlap with live mods. Output: `patch_manager_mods/16_candidate_list.md` with ranked candidates, each with a one-paragraph build sketch and a kill/keep decision, plus a "parked" section with the evidence needed to revive each parked lead.

### Phase 5: Decision gate
Present the ranked list to the user. They pick what to build; a build plan is written per pick in the style of docs 11 and 13. No code before this gate.

## 4. Suggested order and effort

| Order | Work | Effort | Why this order |
|---|---|---|---|
| 1 | L8 symbol check | small | could change the cost of everything else |
| 2 | L1 letterbox, L2 listbox | medium | most likely to affect this install; L2 touches Mod 6 |
| 3 | L9 offline 2DA scan | small | cheap, read-only, settles an existing candidate |
| 4 | L3-L6, L10 sweeps | medium, delegable | broad but routine |
| 5 | L7 QPC | small | only after perf-lab data exists |
| 6 | L11, L12, L13 | waiting | blocked on repro saves or a user test |
| 7 | L14 | existing | continue under doc 10 |

## 5. Risks to the research itself
- K1 write-ups may not transfer; the K2 code must be read independently (the calibrate-detectors rule: verify on disk, not from the summary).
- Name-map transfer errors: spot-check every borrowed name against `ret N` and the asm before relying on it.
- Gemini and web summaries are unverified; treat them as leads only.
