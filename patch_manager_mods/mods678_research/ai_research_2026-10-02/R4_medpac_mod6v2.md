# R4 medpac rule + Mod 6 v2 (2026-10-02)

Static research only. Nothing was compiled, installed or launched. Conventions: "verified" = quoted source line (`file:line`) or asm (`addr  instruction`) from the Steam dump `decomp/<2hex>/00<addr>.asm`; "plausible" = inferred. Every asm claim is tagged **[Opus spot-check]**.
Abbreviations: `W` = `~/Games/KOTOR2_Mod_Workspace/mod8_kite_script`; `nwscript` = `W/vanilla/nwscript.nss` (CRLF, line numbers from `grep -a`); `gen` = `W/src_party_ai/k_inc_generic.nss` (the current generated party-AI source).

## Summary table

| # | Question | Answer | Status | [Opus spot-check]? |
|---|---|---|---|---|
| 1 | Who owns the shared inventory in script terms? | No special object. Any creature with `creature+0x11AC != 0` (every party member, PC included) resolves `GetItemInInventory` / `GetItemPossessedBy` to the one party repository (`CSWSCreature::GetItemRepository 0x58A2C0(creature; 1)`). Vanilla scripts pass `GetFirstPC()` (most) or `GetPartyLeader()`. | verified (asm + vanilla scripts) | yes |
| 2 | Cheapest way to count medpacs | `CheckInventoryNumbers("g_I_medeqpmnt01","G_I_MEDEQPMNT02","g_I_medeqpmnt03")`, vanilla `k_inc_item_gen.nss:250`: 3x (`GetItemPossessedBy` + `GetIsObjectValid` + `GetNumStackedItems`). About 9 engine commands, ~40 NCS instructions. The engine does the O(N) scan natively. | verified (source), cost plausible | no |
| 3 | Loop alternative | `GetFirstItemInInventory`/`GetNextItemInInventory` + `GetTag`: 3 commands and ~12-20 NCS instr per stack, i.e. ~1k instr at 67 stacks (0.8 % of the 0x20000 cap). Works, 20x dearer, no benefit. | cost plausible | no |
| 4 | Does `GetCreatureHasTalent(tSpe11_05)` already imply "has a medpac"? | Yes: for a spell talent the engine scans the 20 equip slots and the shared repository for an item with an ACTIVATE_ITEM property of that spell (`0x6C0330`). So the H1-style guard is also a stock >= 1 test. | verified (asm) | yes |
| 5 | DLL helper worth it? | Not for counting. Only for an INI threshold: a read-only "virtual global" via a hook at `0x67DC4A` (`GetGlobalNumber` handler, value is a signed byte). Sketch only. | plausible | yes |
| 6 | Threshold source | Recommend a script constant (`PARTY_MEDPAC_MIN_STOCK`, 0 = off) for v1. INI-backed virtual global as an optional v2. Reject module/creature locals and real globals (saved into the save game). | design | no |
| 7 | Where it plugs in | New party-only `else if` in `GN_TalentMasterRoutine`, GEN_TALENT_HEALING branch, in front of the NPC `else if` at `gen:2562`. | verified (source) | no |
| 8 | Static guards | `medpac_guard_check` in `build_party_ai_scripts.py:338-370` asserts exactly ONE `tUse = tSpe11_05`; it must be taught a second, party-gated assignment. | verified (source) | no |
| 9 | `GetName 0x7FDFA0` convention | `thiscall(ECX=client item; CExoString* out (hidden return slot), int identified)`, `RET 8`, returns EAX=out; caller must run the dtor. | verified (asm) | yes |
| 10 | Mod 6 sort works on... | Server items (`0x606100(i)` -> id -> server object array), not client items, not rows. | verified (source + asm) | yes |
| 11 | Server -> client item | `clientItem = 0x73F530(ECX=*(*(0xA1B4A4)+4); id)` with `id = *(int*)(serverItem+0x14)`. Client -> server: `0x7FEE70(ECX=clientItem)`. | verified (asm) | yes |
| 12 | Best name source for the sort | The server item's own `CExoLocString` at `item+0x2C0` via `CExoLocString::GetString 0x7356B0` (the exact recipe `SetItem 0x8B1250` uses for the row text). It needs no client lookup and equals what the user sees. `GetName` is a fallback. | verified (SetItem asm); equivalence of `+0x18C` text plausible | yes |
| 13 | Level tint reachable with Mod 6's call? | No. Mod 6 calls `0x5BD6A0(creature,item,&slot,0,0,0)`; the level block is skipped unless `[ebp+0x10]==1` (param 4). It also needs `[ebp+0x14]==0`, the PC flag, `GetLevel < GetMinEquipLevel` and a module-level flag (`[[srv+4]+0x10004]+0xD0 != 0`) of unknown value. | verified (asm) | yes |
| 14 | Enable flag set | `p4=1, p5=0, p6=0` (silent query). Probably a no-op for companions (PC-only) and possibly for the PC too (flag unknown). | verified (asm); flag value unknown | yes |

---

## Medpac: stock counting options

### Prototypes (nwscript, quoted)

```
nwscript:2208  object GetItemPossessedBy(object oCreature, string sItemTag);
nwscript:2204  object GetItemPossessor(object oItem);
nwscript:2767  int GetItemStackSize( object oItem );
nwscript:4312  int GetNumStackedItems(object oItem);
nwscript:3729  object GetFirstItemInInventory(object oTarget=OBJECT_SELF);
nwscript:3735  object GetNextItemInInventory(object oTarget=OBJECT_SELF);
nwscript:4660  object GetFirstPC();
nwscript:4805  object GetPartyMemberByIndex(int nIndex);     // "member 0 is always the current party leader"
nwscript:6080  object GetPartyLeader();
nwscript:4798  int IsObjectPartyMember(object oCreature);
```
`nwscript.nss` has nothing party-inventory specific: no `GetPartyInventory`, no count command. The comments of `GetFirst/NextItemInInventory` say "Returns OBJECT_INVALID if the caller is not a creature, item, placeable or store" (it is the *argument* type that matters).

### Which object owns the shared inventory (verified)

All asm below is **[Opus spot-check]**.

`ExecuteCommandGetItemInInventory 0x680830` (commands 339/340; `param_1 == 0x153` = First). For a creature argument (`vtable+0x30` = AsCreature non-null) it takes the party repository:
```
0x680830 (c)   iVar2 = (**(code **)(*piVar4 + 0x30))();    // AsCreature
               ... else { local_18 = FUN_0058a2c0(1); local_10 = local_18; ...
               cursor kept ON THE CREATURE: +0x1158 (index), +0x115A, +0x115C (last item id)
```
`CSWSCreature::GetItemRepository 0x58A2C0` (`thiscall(creature; int partyShared)`, `RET 4`):
```
0058a2e9  83baac11000000    CMP dword ptr [EDX + 0x11ac],0x0      ; creature+0x11AC = is party member (doc 13)
0058a2f0  7406              JZ 0x0058a2f8
0058a2f2  837d0800          CMP dword ptr [EBP + 0x8],0x0         ; partyShared arg
0058a2f6  7506              JNZ 0x0058a2fe                        ; -> party repository
0058a2f8  837dfcff          CMP dword ptr [EBP + -0x4],-0x1       ; GuiInGame top-menu-override character
0058a2fc  741e              JZ 0x0058a31c                         ; -> creature's OWN repository [creature+0x1154]
0058a2fe  ... CALL 0x0051c8d0 ; ECX=[0xa1b4a4]+8 -> srvInternal+0x1f0b4
0058a313  e818d70600        CALL 0x005f7a30                       ; lazily creates/returns the single party repository (+0x1ac)
```
So **any party member** (PC, companions, the Remote if flagged) sees the same repository; there is no separate "party inventory object". `GetItemPossessedBy 0x681050` does the same (`FUN_0058a2c0(1)`), then `0x605CB0(repo; tag)` and finally the 20 equip slots. Vanilla scripts use `GetFirstPC()` (`c_hasitem.nss:10`, `k_inc_item_gen.nss:256-280`, `k_inc_utility.nss:1636,1665,1706`) or `GetPartyLeader()` (`c_have_item.nss:18`, `a_take_item.nss`).

`GetItemPossessedBy` internals (**[Opus spot-check]**):
```
0x681050  FUN_00734690(local_18)  = CExoString::ToLower on the tag argument (DB name_map)
0x605CB0  loop: item = FUN_0051c0e0(id[i]); if (FUN_007339d0(...)) return item+0x14; recurse into item+0x2ac (containers)
          FUN_007339d0 = CExoString::operator== -> _strcmp (case-SENSITIVE; the arg was lowered, so use lowercase tags)
```
Return = the FIRST stack with that tag, so a script counts `GetNumStackedItems` of that one object. Vanilla treats it as the whole count (`c_have_item.nss`). Whether a tag can span two stack objects (stack cap) is unverified, plausible no for medpacs. `GetItemStackSize 0x697170` and `GetNumStackedItems 0x685480` both read `item+0x2CC`; they differ only on an invalid item (0 vs -1).

### Vanilla scripts that count items (quoted)

`k_inc_utility.nss:1540-1565` (loop; comment says "in the party inventory"):
```
//:: UT_GetNumItems ... Counts the number of items with the given tag in the party inventory.
int UT_GetNumItems(object oTarget, string sItemTag)
{
    int nCount = 0;
    object oItem = GetFirstItemInInventory(oTarget);
    while (GetIsObjectValid(oItem) == TRUE)
    {   if (GetTag(oItem) == sItemTag) { nCount = nCount + 1; }
        oItem = GetNextItemInInventory(oTarget); }
    return nCount;
}
```
Note it counts stacks, not `GetItemStackSize`.

`k_inc_item_gen.nss:249-262` (stack sums, the model to reuse; same file also has the exact medpac call):
```
//Counts and totals up to four different items within the Player's inventory.
int CheckInventoryNumbers(string sTag1, string sTag2 = "", string sTag3 = "", string sTag4 = "")
{   int nNumber = 0;  object oGrenade;
    if(sTag1 != "")
    {   oGrenade = GetItemPossessedBy(GetFirstPC(), sTag1);
        if(GetIsObjectValid(oGrenade)) { nNumber += GetNumStackedItems(oGrenade); } }
    ... (same for sTag2..sTag4)
k_inc_item_gen.nss:61   nNumber = CheckInventoryNumbers("g_I_medeqpmnt01","G_I_MEDEQPMNT02","g_I_medeqpmnt03");
k_inc_item_gen.nss:63   if((nNumber <= 10 && nGlobal < nJediFound) || nGlobal == 0)   // Disciple "needs equipment" at <= 10 medpacs
```
`c_have_item.nss:18-23` and `k_inc_utility.nss:1700-1710` (`UT_ReturnSpikePartAmount`) use `GetItemPossessedBy(...)` + `GetItemStackSize` / `GetNumStackedItems`. `a_clear_inv.nss` loops `GetFirstItemInInventory(oTarget)`.

Medpac tags: `g_i_medeqpmnt01/02/03` (`k_inc_item_gen.nss:61`; templates in `k_inc_treas_k2.nss:564-571`, tag == template is plausible). Droid repair kits: `g_i_drdrepeqp001/002/003` (`k_inc_treas_k2.nss:575-580`, tags assumed equal).

Note: `k_inc_item_gen.nss` is not included by `k_inc_generic` (`gen:10-12` include gensupport, walkways, drop only). Do not `#include` it (it pulls `k_inc_debug` and a large body); write a small private helper instead.

### Cost per call

| Option | Engine commands | NCS instr (estimate) | Native work | Verdict |
|---|---|---|---|---|
| A: 3 tags, `GetItemPossessedBy` + `IsObjectValid` + `GetItemStackSize` | 9 | ~35-45 | 3 linear repository scans (`0x605CB0`), N stacks each, a hash lookup + `strcmp` per stack; microseconds | recommended |
| B: `GetFirst/NextItemInInventory` loop on `GetFirstPC()` | 3 per stack (`GetNext`, `IsObjectValid`, `GetTag`) + 1 per match | ~12-20 per stack | `GetNext` is O(1) with the per-creature cursor; `GetTag` allocs a string | no gain, cursor is per creature (nested loops on the same creature clobber each other) |

Inventory scale: the Mod 6 Stage 0 log shows **67 backpack rows** (doc 11, "Stage 0 in-game result"); late-game saves plausibly 100-250 stacks. B at 250 stacks ~3-5k instr, still far below the cap `0x20000` (design review `review_ai_scripts_decision.md:37`, secondary), but 100x option A. The rule fires only on the rare path (self injured < 50 %, no heal power, medpac talent valid), so either is affordable; A is simplest. Instruction counts are estimates (not compiled).

## Medpac: DLL helper sketch (only a sketch, all **[Opus spot-check]**)

The count itself gains nothing from a DLL (script does it in 9 commands). A DLL helps only for an INI threshold. Existing precedent: companion-ai's `T1` already hooks a VM command to overwrite the value the command is about to push (`companion_ai_src/kotor2-steam-aspyr.hooks.toml:12-20`, `AiTraceShipBuild` writes `[ebp-4]`).

Candidate: the shared handler `0x67DB70` (named GetGlobalNumber in the DB `name_map`, but it is the shared Boolean/Number handler; `param_1` is the opcode `0x242` = 578 GetGlobalBoolean, `0x244` = 580 GetGlobalNumber). The result is a signed byte in `[ebp-0xd]`:
```
0067db95  894dcc   MOV [EBP-0x34],ECX
0067dbab  8d45e8   LEA EAX,[EBP-0x18]      ; CExoString name {char* @-0x18, len @-0x14} filled by 0x6fda70
0067dc27  ...      (opcode 0x244 branch) CALL 0x0051c890 ; CALL 0x00654860  -> writes byte [EBP-0xd]
0067dc4a  0fbe55f3 MOVSX EDX,byte ptr [EBP + -0xd]    ; <- hook here (4 bytes)
0067dc4e  52       PUSH EDX                           ; 4+1 = 5 bytes = one detour window
0067dc4f  8b0da8b4a100 MOV ECX,[0x00a1b4a8] ; CALL 0x006fd9c0 (push int result to the VM)
```
Handler (pseudo): `if ([ebp+8] != 0x244) return; name = *(char**)(ebp-0x18); if (!name || name[0] != 'P') return; if (!strcmp(name,"PAI_MEDPAC_MIN")) *(int8_t*)(ebp-0xd) = cfg.medpacMin;` (0..127; `MOVSX` sign-extends, so 128+ would read negative). Read-only: nothing is stored in the save's global table. Costs: every `GetGlobalBoolean/Number` call in every script pays one function call + a first-char test (hot-ish: dialogues, area scripts); needs `check_windows.py` validation; both executables have the same bytes.

Variant "DLL counts too" (`PAI_MEDPAC_STOCK`): needs OBJECT_SELF -> creature (`VM=[0xA1B4A8]`, `[VMI+0x18C+depth*4]`, DB lesson), `0x58A2C0(creature;1)` -> repo, `repo+0xC` id array / `repo+0x10` count, per id `0x51C0E0` -> item, tag `CExoString` at `item+0x28` (`0x605CB0 @00605d19`: `ADD ECX,0x28`), stack `item+0x2CC`. Same data the script reaches with 3 commands, so not worth the offsets risk.

Variant "engine veto": hook `0x6C0330` (item-spell HasTalent) so party members' item talents return 0 below the threshold and no script change is needed. Rejected: `HasTalent` is also used by other callers (`0x6B8840` callers `0x6BC0E0`, `0x6BA970`), and it hides the medpac from the player-facing logic too.

## Medpac: rule design

### Where it plugs in
`GN_TalentMasterRoutine`, `GEN_TALENT_HEALING` (`gen:2508-2600`). Today (quoted):
```
gen:2540  if(GetCreatureHasTalent(tSpe11_01) || GetCreatureHasTalent(tSpe11_02) || GetCreatureHasTalent(tSpe11_03) )   // Heal / Cure / Master Heal (affordable: HasTalent includes the FP test, see H1)
gen:2562  else if(!IsObjectPartyMember(OBJECT_SELF) &&
              (GetCreatureHasTalent(tSpe11_05) || ...06 || ...07))      // MEDPAC-a: party skips the whole item/Drain/Death Field branch
gen:2585-2596  inner else: party -> bValid=FALSE (MEDPAC-b, dead code for the party); NPC -> tUse = tSpe11_05
gen:2823-2829  if(bValid == TRUE && bHostile == FALSE) { ... "Clear 2300" ... ClearAllActions(); ActionUseTalentOnObject(tUse, OBJECT_SELF); return TRUE; }
```
Facts that shape the rule:
- The medpac is cast on `OBJECT_SELF` (`gen:2825-2830`), so it can only heal the user. The vanilla inner branch is already `if(oTarget == OBJECT_SELF)`.
- `GN_CheckIfInjured` (`gen:2357+`) returns the FIRST of leader / member1 / member2 below 50 %. For a companion `oTarget == OBJECT_SELF` only when no lower-index member is below 50 %. The deferred "H2 extension, self first" (design section 9 item 3) is what would make this reachable more often. Open question 1.
- "No usable heal power" is already encoded: the new block sits in the `else` side of the `if` at `gen:2540`.
- Callers of the healing routine: `gen:645, 957, 1071, 1306` (in-combat) and `gen:4451` (E1 post-combat exit): the rule would also fire after a fight. Open question 2.

Proposed text (in the MEDPAC-a `new` text of `party_ai_patches.py`, inserted before the NPC `else if`; helper body appended to `party_ai_fixes.nss`, prototype added to `PARTY_PROTOS`):
```nss
//Party AI rule MEDPAC v2: a party member uses a medpac / repair kit only on itself, only when it has no usable heal power
//(this branch is the else of the Heal/Cure/Master Heal test), the engine reports the item talent, and the shared stock
//is at or above PARTY_MEDPAC_MIN_STOCK (0 = never).
else if(IsObjectPartyMember(OBJECT_SELF))
{
    if(oTarget == OBJECT_SELF && GetCreatureHasTalent(tSpe11_05))          // H1 pattern: never an invalid talent
    {
        int nStock = GN_PartyMedpacStock();
        if(PARTY_MEDPAC_MIN_STOCK > 0 && nStock >= PARTY_MEDPAC_MIN_STOCK)
        {
            GN_MyPrintString("GENERIC DEBUG *************** Party MEDPAC: use (stock " + IntToString(nStock) + " >= " + IntToString(PARTY_MEDPAC_MIN_STOCK) + ")");
            bValid = TRUE;
            tUse = tSpe11_05;
        }
        else
        {
            GN_MyPrintString("GENERIC DEBUG *************** Party MEDPAC: held (stock " + IntToString(nStock) + " < " + IntToString(PARTY_MEDPAC_MIN_STOCK) + ")");
        }
    }
}
else if(!IsObjectPartyMember(OBJECT_SELF) && ...   // unchanged NPC branch
```
Helper (party_ai_fixes.nss):
```nss
int PARTY_MEDPAC_MIN_STOCK = 10;   //vanilla's own "well stocked" mark is 10 (k_inc_item_gen.nss:63); user decision
int GN_PartyStackOf(string sTag)
{   object oItem = GetItemPossessedBy(GetFirstPC(), sTag);          // lowercase tags: the engine lowers the arg, tags are compared with strcmp
    if (GetIsObjectValid(oItem)) { return GetItemStackSize(oItem); }
    return 0; }
int GN_PartyMedpacStock()
{   if (GetRacialType(OBJECT_SELF) == RACIAL_TYPE_DROID)           // droids take TalentSpell(128/127/84) = repair kits
    { return GN_PartyStackOf("g_i_drdrepeqp001") + GN_PartyStackOf("g_i_drdrepeqp002") + GN_PartyStackOf("g_i_drdrepeqp003"); }
    return GN_PartyStackOf("g_i_medeqpmnt01") + GN_PartyStackOf("g_i_medeqpmnt02") + GN_PartyStackOf("g_i_medeqpmnt03"); }
```
Notes: the global initialiser needs the const/global pattern the build already uses (check that a file-scope `int` with an initialiser compiles in this nwnnsscomp flow; `NPC_AISTYLE_PARTY_KITE = 16` in `MOD8_PROTOS` is precedent). All stack counts use the first stack per tag (vanilla assumption).

### Tiers
The engine chooses the item talent: `tSpe11_05 = GetCreatureTalentBest(0x1408, 20)` (`gen:2520`) = best item talent "closest to CRMax without going over", i.e. the highest tier it finds; droids try `TalentSpell(128)`, `(127)`, `(84)` in that order (`gen:2524-2530`). So with stock = sum of the three tiers the AI burns the best medpac first. Preferring the lowest tier needs the spell ids of the three medpac items (spells.2da, not available here) and `TalentSpell(id)` per tier. Open question 3.

### Threshold source: weighing
| Option | Pros | Cons |
|---|---|---|
| Script constant (recommended v1) | no DLL, no new hook, no save impact, rollback = reinstall scripts, tied to the same compile gates | change = rebuild + reinstall the k_ai_master.ncs (not hot) |
| INI read by the DLL, exposed as a virtual `GetGlobalNumber("PAI_MEDPAC_MIN")` | live tuning, no save pollution | a new hook on a hot shared handler (0x67DB70), signed-byte range 0..127, extra validation |
| Module / creature local (`SetLocalNumber`) | script-only | locals are bytes 0..255, saved into the save game, indices 20/21 already taken (rereview), vanilla uses 0-6, 8-16, 29 |
| Real global (`SetGlobalNumber`) | script-only | saved into the save game, needs a one-time setter script |
Recommendation: constant with 0 = off as the kill switch; add the virtual global only if the user wants live tuning.

### `GetCreatureHasTalent(tSpe11_05)` guard (H1 pattern) and a verified side benefit
H1 (`gen` revitalize): `if (IsObjectPartyMember(OBJECT_SELF) && (nID == -1 || !GetCreatureHasTalent(tUse))) { bValid = FALSE; tUse = TalentSpell(-1); }`. Same idea here: a party member without a valid medpac talent leaves `bValid` FALSE and `tUse` an invalid talent, so the generic fallback at `gen:2844-2855` (`GetIsTalentValid(tUse)`, "Clear 2500") stays closed (the existing static guard relies on this). Without the guard a Drain Life companion (Visas) would reach the medpac line with an invalid talent: the H1 idle loop (design section 4).

Verified side benefit **[Opus spot-check]**: `HasTalent 0x6B8840` for type 0 (spell) ends in `0x6C0330(spellId)`:
```
0x6c0330 (c)  for slot 0..0x13: item = 0x6d0620(1<<slot); ... property type 10 (ACTIVATE_ITEM, nwscript:419) with spell id == param_2 -> return 1
              iVar2 = FUN_0058a2c0(1);                      // shared repository
              for i < repo+0x10: item = FUN_00606100(i); if baseitem+4 == 0 (not equippable) && has ACTIVATE_ITEM with spell id == param_2
                   && (property+0xC != 0 || item+0x298 > 0)  -> return 1
```
So `GetCreatureHasTalent(tSpe11_05)` is TRUE only when the shared inventory holds a usable item granting that spell: it is already a "stock >= 1" test; the stock helper only adds the threshold.

### Interaction with the existing static guards
`build_party_ai_scripts.py` `medpac_guard_check` (`build_party_ai_scripts.py:338-370`) asserts on the generated `k_inc_generic.nss`:
- `assigns` (lines matching `tUse = tSpe11_05`) has **exactly 1** entry -> the new rule adds a second: change to 2.
- `inner` regex (party `bValid = FALSE` / NPC `tUse = tSpe11_05` else) and `outer` regex (`else if(!IsObjectPartyMember(OBJECT_SELF) &&\s*(GetCreatureHasTalent(tSpe11_05) ||`) still match if the NPC branch and MEDPAC-b stay as they are. Keep them untouched.
- `odd` mentions allowed: `GetCreatureHasTalent\(tSpe11_05\)`, declaration, `tSpe11_05 = `, comments. The new `if(oTarget == OBJECT_SELF && GetCreatureHasTalent(tSpe11_05))` line is allowed.
- Add a new assertion for the party assignment: within the preceding ~12 lines there must be `IsObjectPartyMember(OBJECT_SELF)`, `oTarget == OBJECT_SELF`, `GetCreatureHasTalent(tSpe11_05)`, `GN_PartyMedpacStock()` and `PARTY_MEDPAC_MIN_STOCK`, and the helper must call only `GetItemPossessedBy` / `GetItemStackSize` / `GetIsObjectValid` / `GetFirstPC` / `GetRacialType` (no `ActionUse...`).
- `step_static` requires the literal `Party MEDPAC` in `k_ai_master.ncs` (`need` list): the new markers keep it.
- The docstring of `party_ai_patches.py` (lines 12-14), the MEDPAC-a/-b comments (`:238-263`) and doc text "uses no medpacs" (checklist CAI-2) must be updated: the rule is no longer "never".
- ANTIDOTE guard (poison branch, `TalentSpell(67)`) is a separate check and stays closed.
- The vanilla arg-swap risk (`check_ncs_arg_order.py`, pykotor): re-run the arg-order gate; the new calls `GetItemPossessedBy(GetFirstPC(), "tag")` mirror vanilla.

### Trace marker
`Party MEDPAC: use (stock N >= M)` and `Party MEDPAC: held (stock N < M)`. `GN_MyPrintString` is `if(!ShipBuild()) PrintString(s)`; the companion-ai trace flips `ShipBuild` for `k_ai_master`, so the lines appear in `ai_trace.txt` (the trace's 1 s dedupe suppresses repeats). The string concatenation runs even with ShipBuild TRUE (a few VM instr on this rare path). The old line `Party MEDPAC: never uses medpacs or repair kits` (MEDPAC-b) stays for the unreachable path.

### Test-matrix row
| Id | Setup | Expected | Log evidence | Fail means |
|---|---|---|---|---|
| **CAI-9** (new) | Party with a companion that has NO heal power (e.g. Visas) or no FP; add 12 medpacs (stock >= 10); injure that companion below 50 % and keep the leader and the other member above 50 %; fight on | The companion uses ONE medpac on itself, then stops once above 50 %; stack drops by exactly 1 per use | `Party MEDPAC: use (stock 12 >= 10)` then a `Clear 2300` line; no repeat storm | idle loop (guard missing), or uses with stock < min |
| CAI-9b | same, stock 3 | no medpac used, companion keeps fighting | `Party MEDPAC: held (stock 3 < 10)` | consumption below threshold |
| CAI-9c | a companion WITH Heal and Force points, stock 12 | casts Heal, never a medpac | no `Party MEDPAC:` line | rule runs before the power |
| CAI-9d | droid companion (HK-47/T3-M4) with repair kits (`g_i_drdrepeqp00x`) >= 10 | repair kit used on itself | `Party MEDPAC: use` | wrong tag set |
| CAI-9e | enemy NPC with a medpac (e.g. a Sith soldier) | unchanged vanilla behaviour | `I have a Med Pack` | NPC regression |
Also confirm the other injured member is NOT healed by the medpac (it is cast on self only).

---

## Mod 6 R14: GetName convention (asm)

**[Opus spot-check]** for every line in this section.

`CSWCItem::GetName 0x7FDFA0` (`FUN_007fdfa0`, size 618):
```
007fdfc8  894d8c       MOV  [EBP-0x74],ECX            ; this = client item (ECX)
007fdffc  837d0c00     CMP  dword ptr [EBP+0xc],0x0   ; param 2 = identified
007fe002..007fe00e     (identified == 0) ADD EAX,0x18c ; PUSH EAX ; MOV ECX,[EBP+0x8] ; CALL 0x007335f0
                                                      ; = CExoString copy-ctor(out=[EBP+8], src=this+0x18c)
007fe03c  ...          (identified != 0) if strlen(*(this+0x18c))<1 || *(this+0x114)==0
                       -> name rebuilt from the baseitem row: CALL 0x006d5200(this+0xc) ; CALL 0x0071df90 (TLK) ; (+ suffix when this+0x114==0)
                       else same copy of this+0x18c
007fe1f1  8b4df4 ...  007fe209  c20800  RET 0x8      ; 2 stack args; returns EAX = out ([EBP+8])
```
So: `CExoString* (__thiscall*)(void* clientItem, CExoString* out /*uninitialised return slot*/, int identified)`. The caller owns `out` and must run `~CExoString 0x733780`. The name is the 8-byte `CExoString {char* buf @0, int len @4 (strlen+1)}` at `this+0x18C`.

Call sites confirm the push order (identified pushed first, out second): `0x78EBA0 @0078ef8c..0078efa4`: `PUSH 0x1 ; LEA EDX,[EBP-0x90] ; PUSH EDX ; ... CALL EAX(vtable+0x34) ; MOV ECX,EAX ; CALL 0x007fdfa0`; `0x8301E0 @0083063b..00830647`: `PUSH 0x0 ; LEA ECX,[...] ; PUSH ECX ; MOV ECX,[EBP-0x54] ; CALL 0x007fdfa0`. This matches the DB row (`thiscall(clientItem; CExoString* out, bool identified) RET 8`).

## Mod 6 R14: server -> client mapping (asm) and what Mod 6 sorts

**What the sort works on (verified):** server items. `PopulateItemListBox 0x8A7BA0` backpack loop: `local_1c = FUN_00606100(local_18)` (repository index -> `FUN_0051c0e0(id[i])`); `[ebp-0x18]` is that pointer. `InventorySortFilter.cpp` reads `item = rd<int>(ebp, F_Item)` (`F_Item = -0x18`), stores it in `isf::Entry.item` ("server item pointer") and reads `ItemIdOff 0x14`, `ItemFlagsOff 0x2C8`, `ItemBaseOff 0x0C` from it. `0x51C0E0` -> `0x537B00` looks the id up in the SERVER object array (`ECX=[0xA1B4A4]+8`) and calls `vtable+0x38`:
```
0x537b00  cVar1 = FUN_0053dfb0(param_2,&local_8);  if (ok && (char)local_8[2]==DAT_00994475) uVar2 = (**(code **)(*local_8 + 0x38))(...)
```
The same server item has the fields `SetItem` uses for the row: `+0x2C0` loc-string name, `+0x2CC` stack size, `+0x2C8` flags.

**Server -> client (verified, `8A7BA0` equipped loop does the reverse):**
```
008a7cc2  a1a4b4a100   MOV EAX,[0x00a1b4a4]
008a7cc7  8b4804       MOV ECX,[EAX+0x4]            ; holder
008a7cd1  ...          PUSH id (from 0x76e610(slot) for the equipped loop; for a backpack item id = *(int*)(serverItem+0x14))
008a7cdd  e84e78e9ff   CALL 0x0073f530              ; thiscall(holder; id) RET 4 -> client item
008a7cf2  e87971f5ff   CALL 0x007fee70              ; ECX=client item -> SERVER item (client+0x10 sub-object, +0xF8 cached server obj, vtable+0x38)
```
`0x73F530` -> `MOV ECX,[ECX+4]` -> `0x78BD80`: hash lookup `0x53DFB0` (bucket `id & 0xFFF`, key `id & 0x7FFFFFFF`, chain via `+8`), type byte `[obj+8] == [0x994475]`, then `vtable+0x34` (the client item). The backpack loop has no client lookup, so no precedent that every backpack item has a client object (plausible yes, unverified).

**Where the row text comes from today (verified):** `SetItem 0x8B1250`:
```
008b1318  CALL 0x0051c8b0 ; 008b1321  CALL 0x0051c0e0                      ; id -> SERVER item  ([EBP-0x2c])
008b1342  6a00 PUSH 0  ; 008b1347 PUSH &out ; 008b134b CALL 0x0073f350     ; ECX=[GuiGlobal+4] -> *(*(ECX+4)+0x148) = language id
008b1350  50 PUSH EAX  ; 008b1354 ADD ECX,0x2c0 ; 008b135a CALL 0x007356b0 ; CExoLocString::GetString(ECX=item+0x2C0; lang, &out, gender=0)
008b1366  CALL 0x00734190 (find '<' 0x3c / '{' 0x7b) -> 0x0071ebd0 token pass ; 008b16ba CALL 0x00416e30 (label set)
```
`CExoLocString::GetString 0x7356B0`: `thiscall(ECX=locstr {impl@0, strref@4}; int lang, CExoString* out, byte gender)`, `RET 0xC` (`007357e4 c20c00`); it first looks `key = lang*2 + gender` in the embedded list (`0x735860` -> `0x73B470`), else TLK strref `[locstr+4]` via `0x71DF90`; returns AL.

## Mod 6 R14: design

**Source of the key (recommendation):** the server item's loc string, exactly as `SetItem`. Pseudo (new, `InventorySortFilter.cpp`):
```cpp
constexpr int FnLocGetString = 0x007356B0;   // thiscall(locstr; int lang, CExoString* out, byte gender) RET 0xC -> AL
constexpr int FnGetLang      = 0x0073F350;   // thiscall(holder=*(*GuiGlobal+4)) -> int language
constexpr int ItemLocNameOff = 0x2C0;
using LocGetStringFn = unsigned char(__thiscall*)(int, int, void*, unsigned char);
using GetLangFn      = int(__thiscall*)(int);
void itemName(int item, char* out, int cap) {           // out is always NUL-terminated; "" on any failure
    out[0] = 0;
    int holder = rd<int>(rd<int>(GuiGlobal, 0), 4);
    if (!item || !holder) return;
    char exo[16] = {0};
    reinterpret_cast<CExoCtor>(FnExoCtor)(exo, "");     // = empty CExoString {0,0}; same recipe as setLabelText
    reinterpret_cast<LocGetStringFn>(FnLocGetString)(item + ItemLocNameOff,
        reinterpret_cast<GetLangFn>(FnGetLang)(holder), exo, 0);
    int p = rd<int>(reinterpret_cast<int>(exo), 0);      // char*
    for (int i = 0; p && i < cap - 1; ++i) { char c = rd<char>(p, i); if (!c) break; out[i] = c; out[i+1] = 0; }
    reinterpret_cast<CExoDtor>(FnExoDtor)(exo);
}
```
Fallback/alternative (`GetName`): `client = ((fn)0x73F530)(holder, id)`, then `GetName(client, &exo, 0)` (identified = 0 returns the raw `+0x18C` string, no TLK call). Needs an object-array lookup and trusts that the client object exists and its `+0x18C` text equals what is shown (plausible, not verified). Use only if the server path misbehaves.

**Where to call it:** in `SortFilterFlush` just before `sortEntries` (after the Usable/Upgrade drops, so only surviving rows pay), only when `g_cfg.sort && g_cfg.sortName`. Not in H6b (hot per-row path). No new hook; `Entry.item` is still valid at flush (already used by `createRow`).

**Tie-break (SortLogic.h):**
```cpp
struct Entry { ...existing 7 fields...; char name[48]; };   // appended last: aggregate init in sort_logic_test.cpp ({1,1,0,false,true,50,false}) still compiles, name = zeros
inline int cmpFold(const char* a, const char* b) {           // ASCII fold, bytes >= 0x80 compared as unsigned, no locale
    for (; *a && *b; ++a, ++b) { unsigned char x = fold(*a), y = fold(*b); if (x != y) return x < y ? -1 : 1; }
    return (unsigned char)*a < (unsigned char)*b ? -1 : (*a != *b);
}
inline bool before(const Entry& a, const Entry& b) {         // isNew, usable, baseItem as today, then:
    if (a.baseItem != b.baseItem) return a.baseItem < b.baseItem;
    if (g_sortName) { int c = cmpFold(a.name, b.name); if (c) return c < 0; }   // pass nameOn as a parameter in the real code
    return a.orig < b.orig;
}
```
Truncation to 47 bytes keeps a strict weak order (equal truncated keys fall through to `orig`). INI: `sort_name=0|1` under `[Inventory]` next to `sort`; default 0 until the in-game check (with `sort=0` or `sort_name=0` the order is bit-identical to 0.2.0). Optional `sort_name=2` = alphabetical across base items inside the usable group (not requested; skip).

**Host tests to add to `sort_logic_test.cpp`:** (1) same base item, names `"b","A","a"` -> `A`/`a` tie resolved by `orig`, then `b`; (2) base item still dominates the name; (3) `isNew` and `usable` still dominate; (4) names with `0xE9` (Latin-1 e-acute) sort after `z` and do not break the order (unsigned compare); (5) empty name (failed read) sorts first within its group and is stable; (6) 47-byte truncation tie -> `orig`; (7) `sort_name` off reproduces the 0.2.0 order on the existing 7-entry and 100-entry vectors; (8) strict weak ordering on 500 random names (`!before(b,a)` whenever `before(a,b)`; equivalence transitivity).

**Cost per populate (plausible, not measured):** per surviving row: 1 ctor (no alloc for ""), 1 `GetString` (embedded-list walk, usually a TLK lookup `0x71DF90` + string copy = a heap alloc), a copy of <= 47 bytes, 1 dtor (free). Order of 1-3 microseconds each -> 67 rows (S0 log) ~0.1-0.3 ms per populate, plus `std::stable_sort` with ~67*6 = 400 compares of <= 48 bytes (~10 microseconds). `SetItem` already performs the same `GetString` per row, so this roughly doubles one small part of a row build; row creation itself (layout, `SetItem` 2.5 KB, label set) dominates. Add a `QueryPerformanceCounter` span around the key build and log `name_us=` and `rows=` per flush (guard `log=1`) so the first in-game run confirms it. If > 2 ms, add an `id -> name` cache (1024 slots, cleared when `g_panel` changes).

## Mod 6 level tint: 0x5BD6A0 branch analysis

**[Opus spot-check]** for everything here. `0x5BD6A0` (`CSWSCreature::CanEquipItem`, DB `name_map`): `thiscall(ECX=creature; [ebp+8]=item, [ebp+0xC]=u32* slot, [ebp+0x10]=p4, [ebp+0x14]=p5, [ebp+0x18]=p6)`, `RET 0x14`, AL. In the brief's numbering (creature = param_1): item = param_2, slot = param_3, **param_4 = `[ebp+0x10]`**, param_5 = `[ebp+0x14]`, **param_6 = `[ebp+0x18]`**.

How Mod 6 calls it: `InventorySortFilter.cpp` `CanEquipFn(creature, item, &slot, 0, 0, 0)` with `using CanEquipFn = unsigned char(__thiscall*)(int, int, unsigned*, int, int, int)`, i.e. **param_4 = 0, param_5 = 0, param_6 = 0** (not param_4 = 1). The equipment list `0x8AC330` also pushes `0,0,0` (`008ac77a..008ac78b`).

The level block:
```
005bd6aa  837d1001   CMP  dword ptr [EBP+0x10],0x1      ; param_4 == 1 ?
005bd6ae  0f85a7000000 JNZ 0x005bd75b                   ; no  -> SKIP the whole block (Mod 6 lands here)
005bd6b4  8b45d4 / 005bd6b7 8b8898110000  MOV ECX,[EAX+0x1198]   ; creature->stats
005bd6c3  8b426c     MOV  EAX,[EDX+0x6c]                ; stats+0x6C = IsPC (doc 13: cached to creature+0x10E8)
005bd6c9  837de801   CMP  [EBP-0x18],0x1 ; 005bd6cd JNZ 0x005bd75b   ; not the PC -> skip (companions: no level check)
005bd6f7  837d1400   CMP  dword ptr [EBP+0x14],0x0 ; 005bd6fb JNZ 0x005bd75b   ; param_5 must be 0
005bd70e  e8ed760e00 CALL 0x006a4e00 (CSWSCreatureStats::GetLevel(0)) -> ESI
005bd719  e812950400 CALL 0x00606c30 (CSWSItem::GetMinEquipLevel, ECX=item) -> ECX
005bd721  3bf1 CMP ESI,ECX ; 005bd723 JGE 0x005bd75b       ; level >= requirement -> OK, fall to the other checks
005bd734  CALL 0x0051bd40 ; 005bd739 CMP dword ptr [EAX+0xd0],0x0 ; 005bd740 JZ 0x005bd75b
                                                           ; EAX = *(*([GuiGlobal+8]+4)+0x10004): a config object; field +0xD0 == 0 -> no restriction
005bd742  CMP [EBP+0x18],0x1 ; JNZ 0x005bd754 ; PUSH 0x0 ; PUSH 0x62 ; CALL 0x00584da0   ; feedback strref 0x62 only if param_6 == 1
005bd754  32c0 XOR AL,AL ; 005bd756 JMP 0x005bda39         ; return FALSE ("cannot equip")
```
`0x606C30` = `GetMinEquipLevel`: loops a 2da (`DAT_00a1b4d0+0x130 -> +0x118`) comparing the item value (`0x606A80`) with column `MAXSINGLEITEMVALUE` and returns row index + 1 (decompile of `0x606c30`; ECX handling plausible).

Answers:
1. **Reachable with Mod 6's params? No.** `005bd6aa/005bd6ae` jump over the block for `param_4 != 1`; Mod 6 passes 0.
2. **What enables it:** `param_4 = 1` and `param_5 = 0`, with `param_6 = 0` to stay silent (all feedback calls need `param_6 == 1`; checked in `0x5BD6A0` and in the sub-check `0x5BD610 @005bd62f..005bd635`: `CMP [EBP+0x10],1 / CMP [EBP+0x14],1` where `[EBP+0x14]` of the callee is the caller's param_6). Call: `CanEquipFn(creature, item, &slot, 1, 0, 0)`. Real equip (`CSWSCreature::RunEquip 0x566650 @005668d6..005668ea`) calls it with `p4=1, p5=0, p6=1`, so the engine does enforce this when the player actually equips. `ReadItemsFromGff 0x573950` passes `1,1,1` (param_5 = 1 skips the level check on load). `CanUseItem 0x5D2F80` has its own unconditional version (`param_1[0x43A] == 1`, i.e. creature+0x10E8 IsPC, `GetLevel(0) < GetMinEquipLevel` and the same `+0xD0` flag).
3. **Would it ever tint?** Three gates must all be true: the shown character is the PC (stats+0x6C == 1; companions never), `GetLevel < GetMinEquipLevel`, and the `+0xD0` flag. The flag is plausibly NWN's `ItemLevelRestrictions` (a server-info object constructed at `0x5363A0` / `0x788D90`; `0x7EB5C0` does not set it) but its default and writers in TSL were not found. If the flag is 0 in TSL (likely, since TSL lets you equip high-level gear), `param_4 = 1` changes nothing: no tint, no regression.
4. **Side effects of `param_4 = 1`, `param_6 = 0`:** none found. Sub-checks only use `+0x10/+0x14` to gate feedback (`0x5BD610`), and `0x5BD1D0` / `0x5BD3E0` do not read them (grep of `[EBP+0x10]`/`[EBP+0x14]`).
5. **Recommendation:** do not ship a level tint. First add a read-only diagnostic to Mod 6 (log `flag`, `GetLevel`, `GetMinEquipLevel` per row when `log_items=1`) in the next build; ship `param_4 = 1` only if the flag is non-zero in a real save.

Side observation (plausible): `CreateItemEntry 0x8A75F0` has its own level compare for baseitem `+0xAC` in `{0x1A, 0x2D, 0x2F}` (`CMP byte ptr ...; sVar2 = *(short*)(*(param_2+0x310)+0x4C) ... FUN_00848cf0()`), choosing the row click handler `0x8A8760` / `0x8A87F0` / `0x8A86D0`; not related to `0x5BD6A0`.

---

## Open questions
1. **Self-first.** Medpacs only fire when `oTarget == OBJECT_SELF`. `GN_CheckIfInjured` returns the first of leader/member1/member2 under 50 %, so a hurt companion behind a hurt leader never self-heals. Add the deferred "H2 extension" (self first when self < 50 % and no heal power)? It changes who gets healed.
2. **After-fight use.** The E1 exit (`gen:4451` path) also calls the healing routine, so the rule would spend medpacs after combat. Allow, or restrict to in-combat?
3. **Tiers.** Which medpac spell ids (spells.2da) map to `g_i_medeqpmnt01/02/03`? Needed to prefer the lowest tier; today the engine picks the best (`GetCreatureTalentBest(0x1408,20)`).
4. **Default threshold.** 10 (vanilla's own "well stocked" mark) vs lower; user decision. Per-tier or total (this design: total of three tags).
5. **Stack multiplicity.** Can one tag span two stack objects (stack cap)? `GetItemPossessedBy` returns only the first (vanilla ignores it).
6. **Droid kits.** Are the repair-kit tags equal to the templates `g_i_drdrepeqp001-003` (taken from `k_inc_treas_k2.nss`)? Are `TalentSpell(84/127/128)` exactly those items?
7. **Item use on self by a party member** through `ActionUseTalentOnObject(item talent, OBJECT_SELF)`: vanilla only exercises it for NPCs. Test row CAI-9 covers it; plausible, not verified.
8. **Module flag `+0xD0`** (`[[srv+4]+0x10004]+0xD0`): meaning and TSL value. A read-only log in Mod 6 settles it.
9. **Name equivalence.** Is the client `+0x18C` string identical to the server `+0x2C0` loc string the row shows (needed only for the `GetName` fallback)? Do all backpack items have client objects?
10. **Name text details.** `SetItem` runs a token pass (`<...>` / `{...}`) after `GetString`; the sort key ignores it. Fine unless an item name contains tokens.
11. **Instruction counts** quoted for the scripts are estimates; the cap `0x20000` is quoted from the earlier review, not re-derived.
