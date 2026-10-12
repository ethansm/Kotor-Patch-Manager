# R3 engine route: can a KPM DLL add a creature to the current area and give it behaviour?

Recon, read-only. Labels: VERIFIED (query_ghidra steam_val/name_map, batch id given) / INFERRED / UNSURE / MISSING.
Address space: Steam exe. GOG column is for cross-check only.

## 1. Symbol table (Steam address, status, convention)

| Name | Steam | Status | Convention / args | Note |
|---|---|---|---|---|
| Global::ExecuteCommandCreateObject (NWScript CreateObject handler) | 0x0066F990 | VERIFIED B15 (GOG 0x00784F70) | VM command handler, called as handler(id, argc) from ExecuteCommand 0x668FD0 (AI7 line 3, 15) | args popped from VM stack (INFERRED); not callable from a DLL without a VM stack |
| CVirtualMachine::RunScript (public wrapper) | 0x006FD8D0 | VERIFIED B0 (GOG 0x00640AF0) | thiscall, ECX = VM, stack: CExoString*, objId, flag; ret 12 (U1 table, AI7-RS hook catalogue) | 63 callers; returns on null string |
| CVirtualMachineInternal::RunScript | 0x007022A0 | VERIFIED B0 (GOG 0x00645540) | thiscall, ECX = internal (VM+0x1C); loads NCS by resref via ReadScriptFile 0x701FD0 | U1 B-table; resref load = INFERRED to read from override/ |
| CSWSCreature::LoadCreature | 0x00574180 | VERIFIED B12a (GOG 0x0068CCB0) | GOG proto: thiscall (CSWSCreature*, CResGFF*, int, int) | needs a parsed UTC GFF |
| CSWSCreature::AddToArea | 0x00568BD0 | VERIFIED B12a (GOG 0x00681800) | args not in DB proto: UNSURE | cited by engine_ai_atlas/findings/AI4.md:127 |
| CSWSArea::AddObjectToArea | 0x00522BF0 | VERIFIED B0 (GOG 0x00717DA0) | args UNSURE | area object list insert |
| CSWSArea::LoadCreatures | not resolved | GOG 0x0071B140 (Linux 0x08487928) | GOG proto: (CSWSArea*, CResGFF*, ...) | area-load path, not needed for (a) |
| CServerExoApp::GetObjectArray | 0x0051C080 | VERIFIED B9 (GOG 0x0064BAA0) | getter | object table access |
| CServerExoApp::AddObject / object-id allocator | MISSING | no labeled Steam symbol | n/a | blocks route (b) |
| CreateGameObject / NextObjectID | MISSING | no symbol in DB | n/a | |
| CSWSObject ctor | not resolved | GOG 0x006A55E0 | n/a | |
| VIRTUAL_MACHINE_PTR (global) | not resolved | name used as GP('VIRTUAL_MACHINE_PTR') in upstream_prs/acc_parity/simulate_init.py:201 | n/a | OPEN, see section 5 |
| ExecuteScript command | 0x00679050 | doc only (AI7 line 68) | command id 8 | not DB-verified |
| AssignCommand / DelayCommand / ActionDoCommand | 0x0066D9F0 / 0x00670370 / 0x006699D0 | doc only (AI7 line 68, U1) | | |

Trap: Steam 0x005F2A80 is CSWSPlaceable::AddToArea (VERIFIED B0, GOG 0x0076CE50), not a creature function. steam_db_pr3/README.md:218 lists it in the CSWSPlaceable bullet, so do not reuse it for creatures.

## 2. Existing findings (cited, not repeated)
- .claude/npc_div/aliens/04_engine_feasibility.md section 3: walk/run and drive speeds differ per F row (Rodian 1.7/5.4 vs commoner 1.813/3.66). Matters if a spawned creature uses an alien row.
- 04 section 5 risk 6: animations live in the MDL; check with scripts/list_creature_anims.py before shipping alien spawns.
- 04 section 8: area ResRef offset not decoded, so area-scoped placement is blocked. Placement is GIT data, not runtime creation.
- .claude/npc_div/aliens/03_placement_map.md: no spawn or creation finding (grep for spawn, CreateObject, GIT, UTC, placement returned only the title).
- patch_manager_mods/34_npc_diversity_dispatcher.md:85: LoadCreature GOG 0x0068CCB0 / Steam 0x00574180. Matches the DB (B12a).
- patch_manager_mods/10_engine_optimization_research.md:336 and engine_research_findings/U1.md:70: RunScript copies the full NCS per run; nesting cap 8; instruction cap 0x20000. One spawn script per run is cheap.
- U1.md:86 (B2.2): heartbeats run one script per scripted object per 6 s. A spawned UTC with scripts adds to that load.
- U1.md:99 (B3.4): nested RunScriptSituation clears the shared VM stack (SPECULATIVE). Avoid triggering a situation from inside the spawn script.

## 3. Precedents in patch_manager_mods (DLL side)
- rg for CreateObject|RunScript|ExecuteScript|AddObject over *_src .cpp/.h: no DLL creates objects, calls AddToArea, LoadCreature or AddObject. Zero creature-spawn precedent.
- Only DLL call into RunScript: upstream_prs/acc_parity/README.md:110 (the `runscript <resref>` console command). It is a silent no-op because the CExoString size lookup fails (empty `classes` table in every K2 DB). See upstream_prs/runscript_classes/FALLBACK.md:15-16: the string is passed null, and the fallback is an 8-byte CExoString {char* CStr @0, uint Length @4}, with Length stored as strlen+1.
- Observers only (detour RunScript entry, no call): companion_ai_src/CompanionAI.cpp:1239, perf_lab_src/perflab/PerfLab.cpp:1631.
- Script-first precedent: ranged_ai_src/script/kite_routine.nss and party_ai_patches.py:457-473 ship NCS behaviour that calls ExecuteScript("k_ai_master", ...). This is the build pipeline a spawn script would reuse.
- Best precedent for (a): acc_parity runscript command for the DLL call shape, plus ranged_ai_src for the shipped-NCS pipeline.

## 4. Verdict and route
Verdict: (a) is easier.
Route (a): (1) write a small .nss that does CreateObject(OBJECT_TYPE_CREATURE, "<utc resref>", location) then AssignCommand(obj, ActionRandomWalk()); (2) compile to .ncs (nwnnsscomp, per kotor2-script-compiler memory) and ship it in override/ with the UTC; (3) DLL builds the 8-byte CExoString, then calls RunScript 0x6FD8D0 with ECX = VM, push flag, push objId, push &str (ret 12).
Rationale:
1. (a) is one engine call. CreateObject handler 0x0066F990 already does object-id allocation, the UTC load and area insert, so none of those are reimplemented.
2. (b) needs a parsed CResGFF for LoadCreature 0x00574180, AddToArea 0x00568BD0 (args UNSURE), and an object-id allocator that is MISSING.
3. (a) blockers are small and probe-able: VM global address (OPEN) and the CExoString size bug (use the hand-built 8-byte string per FALLBACK.md).

Risks for (a), INFERRED: the spawned creature is part of the area object list, so it persists in saves unless destroyed on area exit. Alien rows change gait (04 section 3).

## 5. OPEN
- VIRTUAL_MACHINE_PTR and APP_MANAGER_PTR Steam addresses: not resolved by query_ghidra in this pass. Resolve from the address DB global pointer table (steam batch 5 README).
- CServerExoApp::AddObject and the object-id allocator: MISSING. Needed only for route (b).
- LoadCreatures and CSWSObject ctor Steam mapping: not resolved (not needed for (a)).
- SetAppearance hook 0x0076D200: name_map says conf medium (gog_cluster_delta_map.py). npc_diversity_src/NpcDiversityDispatcher.cpp:161 labels it Steam. Verify before shipping.
- Not tested: whether CreateObject works from a script run by a DLL with no PC-driven event; a log-only probe on a one-line NCS would settle it.
