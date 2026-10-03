# R10 findings: PC level-up filter, static map (C1), 2026-10-03

Scope: Steam Aspyr swkotor2.exe. Static only (Ghidra via `tools/ghidra_run.sh`, one run at a time; capstone over the exe for xrefs/disasm). Nothing was installed, committed, launched or written outside `phase6_evidence/R10_*`.
Confidence: C = confirmed in decompile/disasm (line cited), I = inferred, U = unverified (needs the log-only probe). File names are in this directory; `dec1..dec7` = `R10_raw_decN.txt` (Ghidra DecompileArgs output), `callmap` = `R10_out_callmap.txt`.
Decompile offsets in raw files are decimal/scaled in places (RE guide 3.3); every offset below was cross-read against the asm (`R10_out_disasm_*`).

## 0. Class map (all C; anchors = string + vtable + ctor)
| panel | ctor | vftable | anchor string (xref) | notes |
|---|---|---|---|---|
| CSWGuiLevelUpCharGen (outer) | 0x8F4AF0 | `CSWGuiLevelUpCharGen::vftable` (Ghidra symbol, dec1:517) | "MAINCG_p" | +0x68 temp baseline creature (new 0x7645F0, filled by 0x8F6FE0), +0x70 = main step panel, +0x2CB4 = creature object id |
| CSWGuiLevelUpPanel (step panel) | 0x902F60 | 0x9AA1AC | "LEVELUPPNL_p" 0x9030D9 | `+0x68 = 0x73F550(creatureId)` = the LIVE CLIENT creature (dec5:456, `param_1[0x1a] = FUN_0073f550(param_3)`; dec5:436 `CSWGuiLevelUpPanel::vftable`) |
| Feats panel | 0x909E00 | 0x9AA434 | "FTCHRGEN_p" 0x90A0D3, "LB_FEATS" 0x90A269 (R10_out_strings.txt) | size 0x1C08, grid at +0x1BF4 |
| Powers panel | 0x9074E0 | 0x9AA34C | "pwrlvlup_p" 0x9077EA, "LB_POWERS" 0x907981 | grid at +0x1BF8 |
| Skills panel | 0x90C6A0 | 0x9AA714 | "SKCHRGEN_p" 0x90CACA | 8 FIXED skill rows (+/- buttons), no list |
Vtables: R10_out_vtables.txt. Slot 15 (+0x3C) is the input/event switch: feats 0x90B200, powers 0x908920, skills 0x90E860 (dec2:437-486 shows the case table: 0x27/0x2D accept, 0x28/0x2E back, 0x29 click-select, 0x2A recommended, 0x2F-0x32/0x3D-0x40 arrow nav). Slot 18 (+0x48) 0x90AD90 / 0x908460 / 0x90E810 = open-confirm helper (I). Only the names are inferred; the code reads are C.
Existing DB rows confirmed: 0x8F8070 BeginPlayGame (dec1:35-72), 0x90B430 feats accept (dec1:74-139), 0x908B50 powers accept (dec1:140-172), 0x8480B0 CanChooseFeat (dec1:173+), 0x847E50 CalcNumberFeats (dec1:374), 0x847F90 ComputeNumberKnownSpellsLeft (dec1:404). The DB name "CSWGuiFeatsCharGen::OnAcceptButton" is consistent (0x90B430 is event case 0x27 target, dec2:446).

## (a) Where the lists are built
### Feats (all C)
- Panel ctor 0x909E00 ends with 0x90AEA0 (callmap line for 0x909E00: `... 90AEA0 418AF0...`; dec2:~279 `FUN_0090aea0()`).
- **0x90AEA0** (init, dec2:339-436): reads `creature+0x310` = CSWCLevelUpStats `S` (dec2:`local_14 = *(*(param_1+0x68)+0x310)`); pushes known feats `S+0xB8/+0xBC` (skipping 0xCF, 0xEE) and class-granted feats (0x8470A0) into panel vectors, CalcNumberFeats -> `+0x1BDA` regular / `+0x1BDB` bonus, `+0x1BDC/+0x1BDD` = count+1 caps, `+0x1BDE` = picks left; then **0x89B600(S, isBonus)** builds the GRID, **0x90C230** builds the CHOOSABLE vector, **0x90BD50** paints states.
- **Grid = 3-column icon grid, not a CSWGuiListBox.** Row object = 0x324 bytes created by **0x89BED0(grid, baseFeat, tier2Feat, tier3Feat)** (dec4:145-216; `alloc 0x324`, 3 slots at `row+0x108 + c*0xB4`: id at +0x108, state byte +0x10C, flags +0x110; grid vector {rows ptr, count} at grid+0/+4; cursor column +0xC, row +0xD). Only call sites: 0x89BE91 (in 0x89B600) and 0x89B5CD (in 0x89B320, another screen, caller 0x8A47FF). Panel stores grid at panel+0x1BF4.
- **0x89B600** (dec3:2039-2272, asm `R10_out_disasm_89B660_body.txt`): iterates the SORTED feat id list (ptr `[ebp-0x24]`, idx `[ebp-0x28]`, count `[ebp-0x20]`), per entry: skip 0xCF/0xEE or missing row (0x89B724/0x89B72D); stop at first non-base feat (+0x38 != -1, `local_34=0`); eligibility `local_14`: known/class-list tests 0x84A1D0 (S already has), 0x6F2830 (class feat table +0x1E4 entry), 0x6F2710 (granted at level), 0x6F2610 (bonus-list flag bit1 when isBonus) / 0x6F2690 (regular-list bit0), plus per-tag special cases (strings "handmaiden","baodur","kreia","hanharr","atton","g0t0"... compared with the creature tag via 0x734F60); if eligible it scans forward for the tier-2/3 feats whose +0x38 == base id and calls 0x89BED0 (call site **0x89BE91**, args pushed at 0x89BE78-0x89BE8B).
- **Sort order (0x89B100, dec4:219-283, C):** feat ids 0..N-1 (`[rules+0x11C]` count), skip 0xCF/0xEE and rows with +8 == 0 (no name); tier = 0 if +0x38 == -1, 1 if +0x3A == -1, else 2; insert before the first element with a higher tier (0x5934C0), else append (0x593450). So order = tier, then feat id ascending. Result is cached in GUI state `0x73F750()+0x103C` (so it is built once per game session, filtered per row by 0x89B600 each time the panel opens).
- **Choosable vector:** panel+0x1BC0 {ptr, count at +0x1BC4}. Built by **0x90C230** (dec2:939-1000, asm `R10_out_disasm_90C230.txt`): loops `[rules+0x118]` ushort feat ids (count `[rules+0x11E]`), skips ids already in vectors +0x1BA8 (known), +0x1BB4 (granted), +0x1BCC (selected), +0x1BC0; then **CanChooseFeat(S, feat, +0x1BDC, +0x1BDD, &selected(+0x1BCC))** at **0x90C349** (`e8 62 bd f3 ff`, ecx=S=[ebp-0x10], 4 stack args, RET 0x10); true -> `0x593450` push into +0x1BC0 (0x90C360). Re-run after every select/deselect (0x90BF90 and 0x90C120 call it).
### Powers (all C unless noted)
- Panel ctor 0x9074E0: `0x907DFE call 0x89AAA0` (grid builder, ecx = panel+0x1BF8, args = server stats `[serverCreature+0x1198]`, flag 1) then `0x907E09 call 0x908540` (data build). R10_out_callmap.txt (0x9074E0 block).
- **0x89AAA0** (dec4:35-143): spells sorted by **0x89A760** (dec7:35-116: spells.2da row type +0x14C in {1,6}, name strref +0xC != 0, multi-digit sort key from column getter 0x6C11C0: key%10 asc, then key/1000, then (key%1000)/10) and cached in GUI state +0x1048. Per entry rows via **0x89AF30(spell, tier2, tier3, flag)** at **0x89ADD4** (normal) and **0x89AEB3** (PlayerCreated extra: owner+0x1200 != 0, ids 0xB0-0xB4, 0xB6, 0x10E, `push 1; push -1; push -1`). I: second path is for the Exile when `PlayerCreated`; not needed for the filter.
- **0x908540** (dec3:35-139): builds known-power vector (+0x1BC8 ptr, +0x1BCC count) alphabetical by name (0x733C40 compare, insert 0x414280 / append 0x83EA60), granted powers via 0x847A70 -> vector +0x1BEC/+0x1BF0, picks `+0x1BA8 = 0x847F90()` (ComputeNumberKnownSpellsLeft), `+0x1BA9 = +1`; then **0x9098D0** builds the CHOOSABLE vector (+0x1BD4 ptr, +0x1BD8 count; dec3:140-214): loop spell ids `0..**[rules+0x104]`, skip ids in 4 vectors (0x4142E0 find), **0x848660(S, spell, +0x1BA9, &selected +0x1BE0)** at **0x9099DB** (`e8 80 ec f3 ff`), spell row +0x180 != -2 and +0xC != 0, alphabetical insert by name.
- **Power per-entry test = 0x848660** (dec3:690-745): row +0x14C in {1,5}; class has powers (0x6C10A0 != -1); picks left param_3 != 0; not already known (0x84A170); not in selected; prereq 0x846DD0; count < picks. The feat equivalent is 0x8480B0.
### Skills (C)
- Fixed 8 rows (computer use, demolitions, stealth, persuade, repair, security, awareness, treat injury), controls built in ctor 0x90C6A0 (strings `COMPUTER_USE_LBL ... TREAT_INJURY_POINTS_BTN` with `*_PLUS_BTN/*_MINUS_BTN`, R10_out_callmap/strings). No list, no sort. Skill table = `rules+0x12C`, stride 0x20, 8 shown; class table `rules+0x124` stride 0x238 (+0x1DC skill table, +0x1E4 feat table, 0xC-byte records).

## (b) Per-entry availability tests
- **Feats:** CanChooseFeat 0x8480B0 (dec1:173-373): false if feat is 0xCF/0xEE, row missing, both counts 0, baodur/kreia special rows, already known (0x8450E0 loop over `S+0xB8`), already selected, prereq 0x846B20; then regular/bonus slot accounting via 0x8487B0 (class list flags). Two callers only: 0x848AA8 (AddDefaultPackageFeats 0x8489F0, the Recommended button path 0x90B6B0 -> 0x8489F0) and 0x90C349 (R10_out_callers.txt).
- **Powers:** 0x848660 above; callers 0x848BF8 (AddDefaultPackageSpells 0x848AE0) and 0x9099DB.
- **Skills:** rank-up button handler **0x90EDE0** (dec4:835+): cap = (character level 0x844860 & 0xFF) + 3, halved when not a class skill (0x9103A0 -> 0x844FF0/0x6F2440); cost = 0x846350(skill, 1) = 1 for class skill, 2 cross-class (dec4:1385-1425, `2 - classSkill`); needs points `[S+0xB6]` >= cost; rank set by 0x846690 (call 0x90F160). Rank-down 0x8468C0 (dec3:789-845) refunds 1 or 2. Mode flags `panel+0x4CF8` bit1/bit2 and the strrefs 0xA5E5/0xA5E7 gate skills 4 and 7 (G0-T0 special case, tag string 0x995154 "g0t0"). Row dimming = 0x90F610 (dec3:1047-1137): `disabled = !(skillRow+0x1C bit4)` etc.; colour constants DAT_00A7FD4C (disabled) vs DAT_00A7FDAC (normal).

## (c) Greyed vs hidden; index safety
- **State machine (C):** grid slot state byte set by 0x89A160 (dec4:284-393): 0 = normal choosable, 1/2 = known/granted (blue-ish constants DAT_00A7FE24), 3 = **dimmed** (DAT_00A7FDD0 colour, border flag cleared), 4 = selected (DAT_00A7FDAC), 5/6 = misc. 0x89C220(3) (dec3:2273) first sets EVERY slot to 3. **0x90BD50** (dec2:734-780) then repaints: known +0x1BA8 -> 1, selected +0x1BCC -> 4, choosable +0x1BC0 -> 0, granted +0x1BB4 -> 2. Powers 0x909360 (dec3:370-423) same, plus spells 6 and 0xE forced to 3 when `panel+0x1BAC` bit0 == 0.
  => **An entry that is in the grid but not in the choosable vector is shown greyed (state 3), not hidden.** Clicking it gives a vanilla refusal popup (0x90B890 case 4 -> strref 0xA4C8; case 2/3 -> 0xA4C7/0xA4C6; dec2:574-620). Vanilla already uses this: CanChooseFeat=false feats stay visible and grey.
  => **Hidden** = the entry has no grid row at all, i.e. the row-add (0x89BED0 / 0x89AF30) is skipped.
- **Index safety (C):** selection is by object id, never by grid or list index into a parallel array. Feat accept 0x90B430 (dec1:95-97) walks the selected-id vector `+0x1BCC` (ushort ids) and calls 0x6C0A70(id); powers accept 0x908B50 (dec1:150-153) walks `+0x1BE0` (spell ids) and calls 0x8453E0; 0x89C170/0x89C070 locate slots by scanning rows for the id (dec3:2290-2380) and silently do nothing when absent. Arrow-key nav (0x89C350) walks rows and skips empty (-1) slots. So removing rows or ids is safe for accept and navigation. The grid is rebuilt only when the panel opens (feats 0x90AEA0 -> 0x89B600; powers ctor).
- **Trap (C):** feats accept refuses when `+0x1BDE` (picks left) != 0 AND `+0x1BC4` (choosable count) >= 1 (popup 0xBE5A), and powers accept when `+0x1BA8 != 0 && +0x1BD8 >= 1` (popup 0x1EBA5) (dec1:95; dec1:150). A hidden entry that is still counted choosable would leave the player stuck. So a filter must act on BOTH the grid row AND the choosable vector, or only on the vector (grey). If the filter empties the choosable vector while picks remain, accept is allowed with picks unspent (a silent skip).
- Recommended button (0x90B6B0 feats, BTN_RECOMMENDED) calls AddDefaultPackageFeats/Spells -> CanChooseFeat/0x848660 directly: it ignores the grid/vector filter unless the same test is filtered.

## (d) Identifying the PC on the client during level-up
- The panel creature `*(panel+0x68)` is the **client** CSWCCreature (id at +4, stats at +0x310); `*(outer+0x2CB4)` = its object id. The panels edit this LIVE creature (dec5 0x902F60, dec6 0x9074E0:130-131). F1 correction in doc 08 Item 6 holds.
- **Server creature from the client creature (C):** `0x77D800` with ecx = client creature (dec5:45-66; asm `0x90777B call 0x77D800` with `ecx=[panel+0x68]`, pw_ctor in dec6) -> `0x7F2540` resolves `[0xA1B4A4]+8` (server app) by object id +4 and caches at +0xF8. The server creature has Tag CExoString at +0x18, IsPC cache +0x10E8, PlayerCreated +0x1200, stats ptr +0x1198 (DB rows + doc 08). Engine itself does this at the powers ctor: `0x90777B..0x90779E`: `server = 0x77D800(client); stats = server+0x1198; panel+0x1BAC.bit0 = stats+0x6C & 1` (dec6:132-133) and uses that bit to grey spells 6 and 0xE (dec3:~400).
- Candidate PC tests, ranked:
  1. **Empty server-creature Tag (+0x18)**: live-confirmed for the PC at LevelUp 0x6B9870 time ("PC tag empty -> tag=?", doc 08 line 270, LevelUpOverride.cpp:634-645 `readCreatureTag`). Same object (stats+0x24 owner == 0x77D800 result: I). Confidence high for "Exile has empty tag", medium that a mod-added companion never has one.
  2. **IsPC +0x10E8** on that server creature: stays on the Exile even while a companion is controlled (lesson 3.8, REVIEW_synthesis E1); the level-up GUI only opens for the controlled leader (party slot 0, S1_gui_guards.txt), so +0x10E8 == 1 means "leader is the Exile". Not independently verified on the GUI path: U.
  3. `stats+0x6C & 1` (engine's own flag in this GUI): meaning "has client / player" per doc 08; U whether it is 1 for companions too.
  4. **PlayerCreated +0x1200 is NOT a PC test**: loader 0x574180 reads GFF "PlayerCreated" (custom/chargen creatures, doc 08 line 340); whether the Exile has it set is unknown (the powers builder reads it, dec4:0x89AAA0 second branch). Do not use.
  5. Object id equality with 0x73F450()+4 only says "controlled", not "Exile" (U5_findings.txt).
- Recommended: tag-empty AND IsPC both logged by the probe; use whichever agrees on PC vs each companion.

## (e) Hook candidates (live == vanilla bytes; +-16 overlap vs all repo/live hooks: NONE for every site, `R10_site_check_1.txt`; boundary/inbound-jump/EAX check `R10_window_check.txt`)
Windows chosen EAX-free and flag-free so a consumed-exit detour (framework: handler runs, stolen bytes run on continue, nonzero return jumps to the consumed exit) is safe (lesson "consumed_exit stolen bytes must not use EAX"). Param registers listed are valid at the window start.
| id | site:len | bytes | what | skip target (consumed exit) | handler inputs |
|---|---|---|---|---|---|
| **F1** feats: hide grid row | **0x89B6E4:6** | `89 8d 3c ff ff ff` | loop body of 0x89B600 right after `lea ecx,[eax+edx*2]`: **ecx = &featId (ushort)** of the current candidate; EAX/flags untouched by the window | **0x89BE96** (loop increment, `8b 55 d8`); safe: nothing pushed yet | ecx, ebp (`[ebp-0x24]` id array, `[ebp-0x28]` index, `[ebp+8]` S) |
| **F2** feats: remove from choosable (grey) | **0x90C2BE:5** | `0f b7 55 e4 52` | loop of 0x90C230 before the 4 vector probes; id = word `[ebp-0x1C]`, panel `[ebp-0x28]`, S `[ebp-0x10]` | **0x90C365** (`e9 15 ff ff ff` -> loop head); nothing pushed yet | ebp |
| **P1** powers: hide grid row | **0x89AB29:9** | `89 4d b8 8b 15 d0 b4 a1 00` | loop of 0x89AAA0 after `mov ecx,[eax]`: **ecx = spellId** (uint); window EAX-free | **0x89AEB8** (loop increment) | ecx, ebp (`[ebp-0x28]` array, `[ebp-0x10]` index) |
| **P2** powers: remove from choosable (grey) | **0x9099A2:5** | `0f b7 4d e4 51` | in 0x9098D0 after the 4 vector probes, before 0x848660 push; id = word `[ebp-0x1C]` | **0x909B51** (`e9 c8 fd ff ff`) | ebp |
| **S1** skills: veto rank-up | 0x90EDE0:6 (function entry, fastcall ecx=panel; skill idx `[ecx+0x4CEC]`) | `55 8b ec 6a ff 68` | rank-up handler; also 0x90F610 (row styling, dim) | entry hook returning early needs a consumed ret (RET 0 fastcall, no stack args): I | ecx |
Not recommended: the CanChooseFeat call 0x90C349 and the 0x848660 call 0x9099DB (`e8` with 3-4 pushed args, callee-cleaned RET 0x10/0x0C: a skip leaves the stack unbalanced, S1_gui_guards.txt lesson); the grid add calls 0x89BE91/0x89ADD4/0x89AEB3 (same reason). Second-best fallback for dimming only: F2/P2 are enough. For hiding: F1+F2 and P1+P2 together (see trap in (c)).
Per-site byte checks: `R10_site_check_1.txt` (U_site_check.py), `R10_pebytes_live.txt` / `R10_pebytes_vanilla.txt` (pebytes.py, F1/P1/CanChooseFeat/0x9099DB bytes identical live and vanilla).

## Open questions / for the probe
1. PC discrimination in the GUI: log per panel open (feats 0x90AEA0, powers ctor, skills ctor): client creature id, server creature (0x77D800), its tag, +0x10E8, +0x1200, stats+0x6C, party slot of that creature, level (0x844860). Expect PC: empty tag.
2. Does skipping a row in 0x89B600 at F1 break the first-cursor logic (0x89C290 selects `rows[0].slot0`; empty grid case when everything is hidden -> `(rows)[0]` null)? Log row count after build; need a guard when 0 rows.
3. What the 0x89A160 state-3 colour looks like in game (DAT_00A7FDD0 is `.data`, runtime-initialised; not read statically): confirm "grey" visually.
4. Skills S1 entry: decide between vetoing rank-up (0x90EDE0) vs graying the row (0x90F610); confirm that the PLUS button state is refreshed by 0x90F610 only.
5. Is the feat sort cache (`0x73F750()+0x103C`) ever invalidated? (F1 filters per open, so it does not matter.)
6. Feats panel runs twice when a class grants both regular and bonus feats (0x90B430 creates a second panel with isBonus); F1/F2 fire in both: the probe should log isBonus (`panel+0x1BE0` bit1).
7. The probe must not change behaviour: log-only hooks at F1/F2/P1/P2 returning 0 with ecx/ebp-derived ids and the tag/IsPC fields above.

## Unverified function identities for the knowledge DB (NOT inserted: no Haiku verification + decoy pass done in this step)
Each has an anchor listed; DB insert should follow the RE guide section 1/4 funnel.
| addr | proposed name | anchor | conf |
|---|---|---|---|
| 0x909E00 | CSWGuiFeatsCharGen ctor | "FTCHRGEN_p"/"LB_FEATS" push 0x90A0D3/0x90A269; vftable 0x9AA434 written at 0x909E4A | high |
| 0x9074E0 | CSWGuiPowersLevelUp ctor | "pwrlvlup_p" 0x9077EA; vftable 0x9AA34C at 0x90752A | high |
| 0x90C6A0 | CSWGuiSkillsCharGen ctor | "SKCHRGEN_p" 0x90CACA; vftable 0x9AA714 at 0x90C6EA | high |
| 0x902F60 | CSWGuiLevelUpPanel ctor | "LEVELUPPNL_p" 0x9030D9; RTTI-style vftable symbol in decompile dec5 | high |
| 0x90AEA0 | feats panel init (build lists + grid + choosable) | calls 0x89B600, 0x90C230, 0x90BD50, CalcNumberFeats 0x847E50 | high |
| 0x90C230 | feats: BuildChoosableFeatVector | CanChooseFeat call 0x90C349; push 0x593450 into +0x1BC0 | high |
| 0x90BD50 | feats: RefreshGridStates | 0x89C220(3) then 0x89C170 x4 vectors | high |
| 0x908540 / 0x9098D0 / 0x909360 | powers: build known list / build choosable (0x848660 at 0x9099DB) / refresh states | dec3 | high |
| 0x89B600 | feat grid builder (rows via 0x89BED0) | sorted id walk + 0x89BED0 at 0x89BE91 | high |
| 0x89B100 | feat sort (tier, id) + cache +0x103C | dec4:219-283 | high |
| 0x89AAA0 / 0x89A760 | power grid builder / power sort | dec4:35, dec7:35 | high |
| 0x89BED0 / 0x89AF30 | feat/power grid row add (alloc 0x324) | dec4:145; dec7:135 | high (0x89AF30 body only skimmed) |
| 0x89A160 | grid slot SetState(col, state) | switch 0-6 over colour constants | high |
| 0x89C170 / 0x89C070 / 0x89C220 / 0x89C350 | SetStateById / SelectById / SetAllStates / HandleNavKey | dec3:2273-2449 | high |
| 0x848660 | CSWCLevelUpStats::CanChooseSpell (power availability) | dec3:690; callers 0x9099DB, 0x848BF8 | high |
| 0x8468C0 / 0x846350 / 0x846690 | skill rank-down / skill cost (1 or 2) / skill rank-up | dec3:789, dec4:1385 | high / high / medium |
| 0x90EDE0 | skills panel rank-up button handler | cap level+3, cost 0x846350, 0x846690 at 0x90F160 | high |
| 0x77D800 | client creature -> server creature (ecx = client creature) | dec5:45; engine use 0x90777B | high (name inferred) |
