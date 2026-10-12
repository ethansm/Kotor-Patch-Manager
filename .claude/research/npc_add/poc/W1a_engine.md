# W1a engine recon (swkotor2.exe, Ghidra 12.1.4, sha256 7751f7d8...)

Contract: recon. Read-only REA session (open_binary provider ghidra, close_binary). Binary path: /home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe

## OUTPUT
- Q1 archive precedence: NOT CONFIRMED. No server-side loader found.
- Q2 CreateObject spawn: spawn-firing call NOT located. Body of 0x0066F990 does not call a script dispatcher.
- Q3 blank modela: NO race fallback in the appearance-model routine. Blank cell yields an empty model name. Downstream effect unverified.
- Q4 GIT creature load: CONFIRMED. Loader is FUN_00525F90 (entry 0x00525F90). Field names listed below.

## EVIDENCE
- procedure_pseudo_code 0x0066F990 (CreateObject). Case 1 = creature: FUN_00919723(0x1220) alloc; FUN_00561F30(0x7F000000,0) construct; FUN_00572A70(resref,0) template load; FUN_0054BDC0 position snap; FUN_00568BD0(area,x,y,z,1,1) AddToArea; returns object id from +4.
- procedure_pseudo_code 0x0076D200 (SetAppearance). Calls vtable+0xE0, FUN_0085A1F0, FUN_0085A6C0/A7C0/A8D0, FUN_0085D430. No direct "modela" or "race" string.
- procedure_pseudo_code 0x0085E601 (contained in FUN_0085E320). Builds column name "ModelA" (builtin strncpy). Row = uint16 at param+0x18 via FUN_0071B1D0. Variant byte at param+0x1 (0 -> 1). Format "%s%02d". Fallbacks: FUN_00711ED0(...,3,0) and (...,0xBBF,0), then variant "01". Failed read sets empty string (&DAT_00986925). No "race" or "racialtypes" read in this routine.
- procedure_pseudo_code 0x00574180 (LoadCreature). Reads AreaId, DetectMode, StealthMode, CreatureSize, IsDestroyable, IsRaiseable, DeadSelectable, BonusForcePoints, AssignedPup, PlayerCreated, Animation, CreatnScrptFird, PM_IsDisguised/PM_Appearance, Listening, ForceAlwaysUpdate. CreatnScrptFird absent and param_3==1 sets this+0xE3 (= save offset 0x38C) to 1. FUN_00573950(...,param_3==0,0). Does not read ObjectId, TemplateResRef, or positions itself.
- procedure_pseudo_code 0x00525F90 (GIT creature list). Group "Creature List" (string 0x99365C). Per element with type 4:
  - param_4==0 (instance path): ObjectId via FUN_00718C00 default 0x7F000000; FUN_00561F30(id,0); LoadCreature FUN_00574180(area,elem,param_3).
  - param_4!=0 (template path): no ObjectId read; FUN_00561F30(0x7F000000,0); TemplateResRef via FUN_00718E40; FUN_00572A70(resref,1).
  - Both: XPosition, YPosition, ZPosition via FUN_00718D00 (default 0); FUN_0054BDC0; FUN_00568BD0(area,x,y,z,param_3,0); XOrientation, YOrientation, ZOrientation via FUN_00718D00; FUN_00543F10 (facing).
- procedure_pseudo_code 0x00574500 (save side, contains 0x0057498E). Writes ScriptHeartbeat (this+0x270), ScriptSpawn (this+0x2B0), CreatnScrptFird (this+0x38C), ObjectId in item and perception lists.
- search_strings: "_s.rim" 0x9A5AB8; "Creature List" 0x99365C; "MODULES:" 0x993E50; "ScriptSpawn" 0x9952AC; "TemplateResRef" 0x99364C; "CModelA" 0x99519F; "modela" lowercase not found as standalone string.
- xrefs: 0x9A5AB8 -> 0x88304D, 0x883116, 0x8C1255, 0x8C131E. 0x99365C -> 0x525FC1, 0x52A640, 0x55DE25, 0x55DF1B. 0x9952AC -> 0x57357F, 0x57498E, 0x76CA10. 0x993E50 -> 27 sites incl. 0x882308-0x882E26, 0x8C0FFB, 0x8C103D.
- procedure_pseudo_code 0x008C1255 (contained in FUN_008C0CB0). CSWGuiLoadModuleDebugMenu (vtable), panel "debug_p". Enumerates "%s\\modules" per search dir (FUN_00407800), "live%d:modules", two lists (type codes 0x7DB, 0xBBA), "MODULES:" as UI header. Tests "<name>_s.rim" companions. Debug UI, not the server loader.
- procedure_pseudo_code 0x00882C00 (contains 0x88304D, 0x883116). Same "%s\\modules" enumeration with "_s.rim" tests. Caller not traced.

## FINDING
- Q1: No loader located that orders <name>.mod, <name>.rim, <name>_s.rim, or Override. The only _s.rim consumers found (0x8C0CB0, 0x882C00) are debug or UI enumerators. So: .mod preferred over .rim+_s.rim = UNKNOWN. GIT in Override consulted for an area = UNKNOWN (Override never searched).
- Q2: 0x0066F990 case 1 has no call to RunScript (0x006FD8D0 / 0x007022A0) or any spawn dispatcher. The creature goes through FUN_00572A70 (template load, not decompiled) and AddToArea (0x00568BD0, not decompiled). The spawn-firing call is not confirmed. Heartbeat parity with GIT creatures = UNKNOWN. Note: the CreateObject path passes flags (1,1) to AddToArea; the GIT path passes (param_3,0).
- Q3: Blank modela -> empty model string via the local_51 branch in FUN_0085E320. No race fallback in this routine. Rows 72, 73, 464, 63 were not checked (no 2DA read). Invisible vs default model downstream = UNKNOWN.
- Q4: TemplateResRef is read only on the template path (param_4!=0) in 0x00525F90. XPosition, YPosition, ZPosition, XOrientation, YOrientation are read on both paths. ObjectId is read only on the instance path (param_4==0), default 0x7F000000 (same invalid literal as 0x0066F990). Engine ID assignment when absent = not seen in this function (UNKNOWN).

## OPEN
1. Q1: locate CExoResMan / module add-resource function (search "modules" strings, "Override", ".mod", ".rim" xrefs from CServerExoApp/CSWSModule). Also check 0x0052FEA0-0x533001 and 0x5602DC-0x560AF4 (MODULES: xrefs).
2. Q2: decompile 0x0057357F and 0x0076CA10 (ScriptSpawn xrefs); FUN_00572A70; FUN_00568BD0 (AddToArea args 5-6 meaning).
3. Q2: confirm whether CreatnScrptFird (this+0x38C) gates OnSpawn for fresh vs saved creatures.
4. Q3: check FUN_0071B1D0 (2DA cell read) return on blank cell; check rows 72, 73, 464, 63 in appearance.2da and racialtypes.2da (not accessible to REA).
5. Q4: FUN_00572FC0 (loads UTC template on instance path?) and FUN_00561F30 ID assignment.

## VERDICT
Partial. Q4 confirmed (0x00525F90). Q1, Q2, Q3 open; no answer to the precedence or spawn-firing questions from this session.
