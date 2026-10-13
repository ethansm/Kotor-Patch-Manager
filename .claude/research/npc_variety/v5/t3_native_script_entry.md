# T3: native script entry 0x006FD8D0 (recon, read-only)

Tags: VERIFIED = line read in a repo file, or static callgraph cited by a repo file (not re-run here). UNVERIFIED = repo claim with no byte check in this session, or inference. OPEN = not proven.
Sources read: patch_manager_mods/38_k1_to_k2_asset_import.md:149-195; patch_manager_mods/engine_research_findings/W7.md:80-100; patch_manager_mods/companion_ai_src/CompanionAI.cpp:35,1225-1262. No REA, no binary read.

## 1. Signature of 0x006FD8D0

- Chain: ExecuteScript (VM slot 8) 0x679050 -> 0x6FD8D0 -> 0x7022A0 (doc38:192) [UNVERIFIED, REA label].
- Static callgraph: `006fd8d0 006fd925 007022a0 CALL`; `007022a0 007023e1 00702850 CALL`; `007028e2 006ff9f0 CALL` (W7.md:91) [VERIFIED as cited].
- Queued path: ActionDoCommand, DelayCommand, AssignCommand -> RunScriptSituation 0x702590 -> 0x702850 -> 0x6FF9F0 (W7.md:90) [VERIFIED as cited].
- arg1 = pointer to a CExoString-like struct, dword 0 = char* (doc38:192); may be 0 (CompanionAI.cpp:1237-1238) [UNVERIFIED layout].
- arg2 = objid (OBJECT_SELF); stored by 0x7022A0 at this+0x18c[depth] (doc38:192) [UNVERIFIED].
- arg3 = "flag" = the VM optional int (doc38:192); T19 passes KOTOR_DEFAULT_EVENT_ON_HEARTBEAT there (doc38:150) [UNVERIFIED].
- Convention: CompanionAI.cpp:1237 says thiscall, but doc38:192 says ECX, retn size and prologue are UNKNOWN. Stack vs ECX is OPEN.
- The `__cdecl` on CompanionAI.cpp:1239 is our export's convention, not the target's (doc38:159).

## 2. Can our DLL CALL it?

- Not safely yet. Missing: convention, ECX, retn size (doc38:192; doc38:194 open list). Arg1 is not a 16-byte CResRef (doc38:192), so a chitin resref must be wrapped in a CExoString-like struct first.
- Existing hook: CompanionAI RS probe is a detour at entry, `CompanionAiProbeRunScript(ret, script, self)` (CompanionAI.cpp:35, 1237-1239). A KPM detour runs before the original bytes and cannot change args or return (doc38:193). Hook param map (ecx / [esp+4] / ebp) is in kotor2-steam-aspyr.hooks.toml, not read (doc38:159) [OPEN].
- Our own call re-enters this detour; it returns at CompanionAI.cpp:1241-1242 unless `ret` is a probed site [VERIFIED from code; rsSite body not read].
- Safe call site: none proven. The T19 spawn is an engine heartbeat ExecuteScript, play-verified (doc38:150, 164), not a DLL call. CompanionAI.cpp:1244 `tick(now)` runs per RunScript call, not per frame. Spawning inside LoadCreatures is unsafe (doc38:193). 0x522bf0 has 10 callers (doc38:189), so it is not a clean site.
- Resolution by name at runtime (doc38:152) [UNVERIFIED for a DLL-originated call].

## 3. T19 override-script pattern

- Override `k_def_heartbt01.ncs` wraps the original heartbeat: gate on tag `204_Ith_Rec` and `GetObjectByTag("k1p_probe_npc") == OBJECT_INVALID`, then `ExecuteScript("k_ai_master", OBJECT_SELF, KOTOR_DEFAULT_EVENT_ON_HEARTBEAT)` (doc38:150, 153) [VERIFIED in doc text; .ncs bytes not read].
- Ships as 7 files: override/k_def_heartbt01.ncs, probe_spawn/override/k1p_probe_npc.utc, models, textures, appearance.2da (doc38:171) [VERIFIED in doc].
- Installed by the doc-38 session under user GO, manifests k1p_probe_20261012_0146 and k1p_probe_20261012_1414 (doc38:171, 173) [VERIFIED in doc].
- Play result: spawn works with scripts only, no DLL (doc38:164-167, user screenshot) [VERIFIED by user report].

## 4. OPEN items

1. Convention, ECX source and retn size of 0x6FD8D0 (is arg1 stack-passed; is the flag arg3?).
2. CExoString layout (char*, length field), buffer ownership, null handling.
3. Whether 0x7022A0 is synchronous; what the flag does beyond the VM optional int.
4. Safe game-thread call context, and the VM depth state that this+0x18c[depth] needs.
5. objid behaviour for OBJECT_INVALID or a non-party objid.
