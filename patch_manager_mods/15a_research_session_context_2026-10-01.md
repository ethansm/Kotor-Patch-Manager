# Session context: Gemini round 2 triage + Odyssey-community survey + K1-bug audit (2026-09-30 / 10-01)

Companion to `15_engine_bug_candidates_research_plan.md` (the plan). This file records everything learned in the session
so a fresh context can start Phase 0 without re-deriving it. No code was written, nothing installed, no game launched.

## 1. How the session went (chronological)
1. User pasted a Gemini web-app "mod ideas" list (5 items) and asked to check repo docs for ideas already considered/rejected.
2. Triaged each against repo docs + the Ghidra dump (`~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/`). Result in section 2.
3. User asked for a web search of other Odyssey-engine games / communities doing RE modding. Result in section 3.
4. User asked which of the KMRP (K1) bugs were already investigated here. Result in section 4: **none**.
5. Plan written (`15_...plan.md`), this context file written, then plan mode for the next phase.

## 2. Gemini round 2 triage (verified against the repo and the dump)

| # | Gemini idea | Verdict | Evidence |
|---|---|---|---|
| 1 | Auto-pin engine threads to one core (crackle/freeze) | **Reject** | `09_load_hang_investigation.md` s4/s5/s7: `taskset -c 0,6` = two SMT siblings of ONE physical core and is the **#1 hang suspect**; mitigation there is to relax pinning. `imports.tsv` has no `SetProcessAffinityMask`/`SetThreadAffinityMask`/`GetProcessAffinityMask` (only `GetSystemInfo` x1, `SetPriorityClass` x3). Launch options already include `PULSE_LATENCY_MSEC=60`. No doc supports a thread-desync claim. |
| 2 | Save corruption "Level 1" | **Unsupported** | `engine_research_findings/R4.md`: save path audited, balanced; `WriteResource_2` 0x721BB0 "never freed" claim REFUTED (holder dtor 0x722670 frees at 0x721E30); leaking branch dead (all 3 callers pass flag=1); GFF written by fwrite, no per-resource heap copy. K1 `SaveGameMemoryLeak` is K1-only (doc 06 Track B) and a K1-style free hook would double free. `CWorldSaveGame` is not a name in this exe/map. Needs a repro save. |
| 3 | 2DA fixed static buffers | **Premise mostly false** | `R5.md`: C2DA parser 0x71C3D0 allocs by actual row/col counts (`cols*8+4`, `rows*8+4`); getters 0x71AFA0/0x71B3F0/0x71B8B0/0x71BCF0 are bounds-checked. Hard caps (`upgrade.2da`, `placeables.2da`) live in consumers (`CRASH_INVESTIGATION.md` ~l.208). `appearance.2da` has no hard cap (live file 783 rows vs vanilla 671 works). Real parser hazard = candidate **TD**: token >255 chars overflows a 256-byte stack buffer (`local_130[256]`, 0x71C180 NextToken has no dest bound). Consumers never audited. "LoadOub" is not a name in the map. |
| 4 | Soft Shadows GL fix | **Test first / likely drop** | `SoftShadowsFix` (Patch Manager) is K1-only, non-NVIDIA (doc 06 Track B, `04_recommended_installs.md`). K2 exe strings: `Soft Shadows` 0x9A01C4 (users FUN_007B88B0, FUN_007B5F50), `AllowSoftShadows` 0x9A0418, `CB_SOFTSHADOWS` 0x9A1B1C (FUN_007D6170), `GL_EXT_stencil_two_side`/`glActiveStencilFaceEXT` in FUN_004317D0. Pass itself never reverse-engineered. User has RTX 5070. `CRASH_INVESTIGATION.md` l.93: Agy's soft-shadow advice was rendering-only and unrelated to the crash. A one-minute in-game toggle answers the 144->15 fps claim. |
| 5 | Stuck animations after dialogue | **Not documented; unverifiable** | Nothing in the repo. `ClearAllActions` is mapped (Steam `CServerExoApp::ClearAllActions` 0x51CB10 per the R9 TSV rows, `CSWSObject::ClearAllActions`, `CSWCObject::ClearAllActions`) and doc 13 notes the party AI re-issues `ClearAllActions(); ActionAttack` every round. Flushing all queues on conversation exit could wipe scripted-scene actions. Needs a repro save. |

Other facts checked on disk this session: `imports.tsv` lists `QueryPerformanceCounter` x5 + `QueryPerformanceFrequency` x2, `timeGetTime` x3, `GetTickCount` x20, `Sleep` x17, `VirtualAlloc` x2, `GlobalMemoryStatus` x2.

## 3. Web survey: other Odyssey / BioWare-engine RE communities

Search tool returned many generic results; fetched pages sometimes lacked technical detail. What is solid vs not is flagged.

| Project | What it is | Relevance / caveat |
|---|---|---|
| **KMRP** (KOTOR Modern Restoration Patch, RayesDiyab) https://github.com/RayesDiyab/KMRP-KOTOR-Modern-Restoration-Patch | K1 (`swkotor.exe`) UI/engine restoration, 48 resolutions. Engine fixes via new PE sections with hand-written x86 stubs + imm32 rewrites; **since 2026-09-29 ships as a delta (`kmrp-kpm.dat`) applied by the KPM proxy**. Docs: `reverse-engineering/binary-inventory.md` (hash + address inventory), `listbox-geometry.md`, `inventory-item-rows.md`, `movies.md`, `map.md`, `map-markers.md`, `area-map-surface.md`, `docs/font-scaling.md`, `docs/universal-resolution-math.md`. Scaling rule `max(1.0, height/720)`. **K1 only; K2 not addressed.** | Closest peer: same engine family, same framework. |
| **Kotor-Patch-Manager** PRs #286 (FTD516 widescreen + stray fixes), #271, #202, #267 https://github.com/LaneDibello/Kotor-Patch-Manager | Community patches incl. macOS Aspyr port of KMRP fixes; Linux windowing fixes. #286 lists 9 engine bugs (word-wrap hang, list growth, leading newline, video fullscreen size, tall-line disappearing, text measurement margin, letterbox sizing past 21:9, minimap zoom, HiDPI) + 5 widescreen code fixes. | Source of the K1 bug list. macOS port reportedly "every address read from the Mac code and names its Windows equivalent" = cross-build mapping works (matches our GOG->Steam delta method). |
| **Peixoto's Patch** https://buymeacoffee.com/peixoto/peixotos-patch-v-0-10-7-released | Generic wrapper; Jade Empire fix: `QueryPerformanceCounter` made relative to process start (fixes low frame rate). | Lead only (L7). Whether K2/Wine shows the symptom is unchecked. Jade Empire has no engine-hook community found. |
| **NWNX:EE / NWNCX / NWN Community Patch 1.71** | Aurora (NWN) server/client extenders and an optional plugin for hardcoded engine fixes. NWNX:EE's approach: Beamdog released an API exposing engine functions/structures, "very little to reverse"; symbols not distributed. | Process model only. |
| **reone / xoreos / KotOR.js** | Clean-room reimplementations. Already used in `13a_ai_community_research.md` (not proof of vanilla behaviour). | Low. |

Not verified: whether the Aspyr Linux/macOS builds retain symbols (no source found). That is lead **L8** and the first Phase 1 task. Not found: any public Ghidra project/DB for K2 with names.

## 4. K1-bug (KMRP) audit against this repo: nothing investigated

Searched all `*.md` (patch_manager_mods, CRASH_INVESTIGATION, compatch docs), `engine_research_findings`, the knowledge DB TSVs, and the ultrawide memory note for: listbox, row height, inflation, line-break, word-wrap, unbreakable, glyph, advance, auto-fit, popup, minimap, pan, stack count, letterbox, font scale.

- Letterbox: `semi-transparent-letterbox` installed (visual only). `shield_bar_src/blackout_investigation.md` l.28: `0x4113a0` is the letterbox draw (glScissor + glClear, balanced). Dialogue-letterbox *sizing* never examined. User runs **3840x1600 = 2.4:1 > 21:9 (2.333)** = past the K1 failure threshold.
- Listbox: knowledge DB names ~40 `CSWGuiListBox` functions (`Constructor` 0x41CE90, `ClearItems` 0x41D290, `SetExtent` 0x41D710, `OrganizeControls` 0x41EA50, `OrganizeUnequal` 0x41F170, `OrganizeOversized` 0x41F7F0, `Draw` 0x41F980, `AddControls` 0x41FC30/0x41FD90, `Load` 0x41FFD0, `LoadProtoItem` 0x420180, `ReSetFont` 0x420E90 ...). Mod 6 (`11_inventory_sort_filter.md`) repopulates via `PopulateItemListBox` 0x8A7BA0; doc 14 uses `ClearItems`.
- Text: doc 07 found text setter `FUN_00416E30` (ECX = label+0xF0, resets layout `+8=-1`); `CAurGUIStringInternal` names exist (`GetFontTexture` 0x47A0C0, `GetFontPixelHeight` 0x47A510, `LoadAlternateFonts` 0x479B60). Doc 14 l.32 assumes uniform 8/3 px per texel. No line-break or glyph-advance analysis.
- Map: aspect/projection fixed by `k2-aspyr-map-aspect-fix` (doc 06: `preserveMiniMapAspect` Steam `0x007508AA`, `CSWGuiMainInterface::FUN_00750790`); pan clamp not examined.
- Popups: `Global::ExecuteCommandDisplayMessageBox` 0x69BEA0 / `PopUpGUIPanel` 0x68ECE0 named; auto-fit never examined.

## 5. Key repo facts a fresh context needs
- Live install: 19 patch ids, ~63 hooks (doc 10 header). Hook-overlap check = scan all `kotor2-steam-aspyr*.hooks.toml` (+-8 bytes), see doc 08 Q8 and `mods678_research/check_windows.py`.
- Ghidra: full-program dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` (`decomp/`, `strings.tsv`, `imports.tsv`, `callgraph.tsv`, `globals.tsv`, `alloc_sites.tsv`). Names DB: `patch_manager_mods/ghidra_knowledge/` (`steam_functions.tsv`, `gog_map_mods678.tsv`, `kotor2_ghidra_knowledge.db`, README). Query it before any fresh Ghidra run.
- Findings format to copy: `engine_research_findings/CONTRACT.md` + `R*.md`/`U4.md` (summary, function table, findings, runtime measurement, open questions).
- Process rules (memory): verify third-party/agent claims on disk; Sonnet subagents do routine sweeps, Opus designs/reviews/gates/commits; never overlap a background job with a coding phase; never launch the game, the user runs tests; add exhaustive logging first when a hook misbehaves; sibling sessions overwrite installs, so re-md5 before relying on a stage.
- Git: branch `KOTOR-II`; this session's files are uncommitted.

## 6. Files created this session
- `patch_manager_mods/15_engine_bug_candidates_research_plan.md` (high-level plan, lead register L1-L14, evidence standard, phases)
- `patch_manager_mods/15a_research_session_context_2026-10-01.md` (this file)
- README index entry added.

## 7. Addenda from plan mode (2026-10-01, before Phase 0/1 execution)

### 7.1 Why the Linux binary was on the list, and the decision
The native Linux build (`KOTOR2` ELF, sha256 `ED043D21...`, Steam depot 208580, **not installed**; we run Windows `swkotor2.exe`, depot 208581, under Proton)
would only ever be a *name reference*, never a hook target. KPM's docs say the ELF keeps symbols, but Aspyr diverged the builds and we already have 10,425 named
GOG functions mapped to Steam. **Decision: drop the Linux download from Phase 1.** Revisit only if Phase 1 shows named-function coverage is the bottleneck
(gate S5: count unnamed `FUN_xxxxxxxx` functions the V1-V6 analyses needed). Free substitute used now: `AddressDatabases/` in the KPM clone.

### 7.2 New facts
- KPM ships a community 2DA row-limit table: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/docs/2da_row_limits.md` ("DP's 2da markdown", ~209 rows, cites engine functions).
  It answers **L10 / Gemini #3**: limits are integer-width / bit limits, not static arrays. Key caps: `appearance` 65536 (short), `heads` 256, `placeables` 256, `upgrade` 32 (bitfield),
  `portraits` 65536, `spells` 65536, `feat` 65536, `classes` 256, `soundset` 65536, `aiscripts` unlimited (but CSWGuiScriptSelect leaks). Not yet checked: which game(s) it describes,
  or whether our live tables approach a cap (Phase 1 / V6).
- `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/AddressDatabases/`: `kotor1_0_3.db` (9,849 functions, 977 classes, 5,991 offsets), `kotor2_gog_aspyr.db` (10,425 functions, 778 offsets),
  `kotor1_steam_aspyr_macos.db` (1,927), `kotor2_steam_aspyr_linux.db` (small). Schema in `tools/SqliteTools/Migrations/*.sql`. A KMRP K1 address can be bridged to K2 by name:
  K1 addr -> `kotor1_0_3.db` name -> K2 Steam via `ghidra_knowledge/steam_functions.tsv` / `gog_map_mods678.tsv` (Phase 1 S1, `engine_bug_research/tools/k1_to_k2_bridge.py`).
- KPM Linux hook files cite names such as `CAurInternalGL::PresentFrame` @ 0x084144D4; `docs/NATIVE_LINUX.md`: the ELF is `ET_EXEC`, fixed base, symbols preserved.
- Local tooling: `nm`, `readelf`, `objdump`, `python3`, `dotnet`; **no `steamcmd`, no `sqlite3` CLI** (use `python3 -c "import sqlite3..."`).
- Install dir has `swkotor2.exe` (+ `.backup.*`, `.unpatched_vanilla`); no ELF. md5 of live `swkotor2.exe` at Phase 1 start: `c9a1e3c57fd5491b4256dac1c0ab97cb` (must be unchanged at the end).

### 7.3 KMRP/KPM PR #286 detail read in Phase 0 (source of the K1 bug descriptions; K1 macOS addresses, NOT K2)
Sources: `Patches/K1WidescreenPatch/KMRP-ENGINE-FIXES.md` and `Patches/K1StrayBugFixes/README.md` in https://github.com/LaneDibello/Kotor-Patch-Manager/pull/286 (merged), and KMRP
`reverse-engineering/{listbox-geometry,message-popup,map,map-scaling,text-padding,movies,font}.md`, `docs/{font-scaling,universal-resolution-math}.md`. Local copies are in the session scratchpad only.

| KMRP id | Bug | Mechanism as KMRP/KPM describe it (K1; unverified for K2) |
|---|---|---|
| K1 | word-wrap hang | `WrapStrings` backs the cursor up one char when a line overflows, then checks progress against the start of the whole *string* (`[this+0x18]`) instead of the current *line*, so the check always passes and an empty line is appended per pass (memory exhaustion). Two guards (narrow-label blanking) exist only to dodge it. |
| K2 | listbox rows grow on repopulate | `CSWGuiListBox::OrganizeControls` spreads leftover height over visible rows, writes the enlarged height into the reused row controls and into a local (`[rbp-0x34]`); next pass takes max row height as item height (`+0x368`) and inflates again (Powers/Feats 42 -> 56 -> 126 px on Windows). Fix = NOP two stores. |
| K3 | leading newline in descriptions | builder prefixes `"\n"` per property line; trimmed in `CSWGuiTextParams::SetText`. |
| K5 | single line taller than box vanishes | `CAurGUIStringInternal::Draw` drops lines from the top until the rest fits, including the last one. |
| K6 | wrap under-measure | line-breaker truncates each glyph advance to int after `+0.25f` (avg 0.25 px/char under); fix uses `+0.5f`. |
| K7 | dialogue letterbox | bar = `(H - W/(7/3))/2`, 7/3 a float constant read by 6 sites; **negative past 21:9** (3440x1440 -> -17 px; our 3840x1600 -> about -22 px). Reply panel is a hard-coded 100 px tall. Fix: bar = H/6. |
| K8 | minimap zoom | HUD templates author the radar 120x120 and the map 512x512; enlarging only the radar shows more area. NOT a pan-clamp bug: **L5 as written in doc 15 is a misreading**, K8 only matters if the radar is enlarged (our ultrawide GUI work may or may not touch it; V4 must check). |
| (popup) | `message-popup.md` | see V4. |

### 7.4 Constraints for the executing session
Static only; never launch the game; no installs; nothing under the game dir or live `override/`; commits docs/tools under `patch_manager_mods/` only.
