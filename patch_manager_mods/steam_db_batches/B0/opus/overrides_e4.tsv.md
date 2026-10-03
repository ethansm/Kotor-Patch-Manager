# B0 Opus gate, part e4 (99 reviewer `match` rows, 100% Opus read)

Gate: Opus, 2026-10-03, following `OPUS_GATE_BRIEF.md`. Verdict rows are in `overrides_e4.tsv`.

## Method
For every row I did the following:
- Compared the GOG and Steam `fp.tsv` rows (size, retn, n_insn, callees, strings, disp, imm).
- Checked that each GOG callee's fp equals the fp of the Steam callee in the same role.
- Compared the multisets of caller sizes. Every GOG caller of the GOG address was compared with every Steam caller of the Steam address, Unwind funclets excluded.
- Read the Mac x86 symbol of the same name for shape. Mac vtable slots are +4 and Mac uses its own layout.
- Read the Steam asm or decompile for meaning.
- Listed same-fingerprint twins within 0x800 on both sides and settled each one by content.

The main risk for this part was a GOG label that is wrong even though the body counterpart is right. For each row I checked the name against three things: the Mac symbol, the GOG neighbours, and the meaning of the Steam body.

Every asm line quoted in the TSV (110 address-tagged lines) was re-checked on disk in `steam_6a522e71/decomp` with a script. All of them are present, with the quoted mnemonic.

## Result
- 97 of 99 rows have identical GOG and Steam fingerprints. The other two differ only in these ways:
  - `CServerExoAppInternal::SaveGame`: GOG has one extra string ref, the empty string 0x9858f5. The other 5 strings are the same text: 'SAVES:', 'SAVEGAME', 'SAVES:%06d - %s', 'SAVES', '%s%06d - %s%sScreen'. All 22 callees are fp-equal.
  - `CVirtualMachineInternal::ReadScriptFile`: Ghidra split the GOG function into a 64 B head at 0x645270 (r-1) and a 626 B body at 0x6452b0. The GOG span is 720 B against Steam's 716 B. The Steam disp/imm set is the union of the two chunks. GOG SetUpScriptSituation calls 0x645270 and Steam's calls 0x701fd0 (`00702a15`).
- Every pipeline Steam address is the body counterpart. Caller-size multisets agree on every row that has callers, for example 260/261, 393/396, 70/70 and 117/117 for the CServerExoApp getters. The small gaps come from GOG function splits such as ExecuteCode.
- There are no corrections, no `rejected` rows and no `unresolved` rows.
- I found **no GOG label error in this part.** Every name fits its body: the Mac shape matches (with vt+4 where it applies) and the Steam body meaning matches.

| verdict | count |
|---|---|
| verified | 69 |
| likely | 30 |
| label_suspect | 0 |
| rejected / unresolved | 0 |

`likely` follows the part-C convention: a thin getter, setter, flag toggle or vcall forwarder. Its only anchors are the field offset (Mac disp equal), fp equality and the run position, and nothing contradicts it.

## Rows that needed more than fingerprint equality

**CSWVirtualMachineCommands dtor (0x77b430 -> 0x665eb0)**
- The brief warns about the ItemPropertyHandler trap, where a body that installs only the base vptr is really a base dtor. This row is not that case.
- 0x665eb0 stores 0x9940d0 at `00665edb`, frees +0xc, and restores 0x994100 at `00665f1a`.
- The inlined construction in CServerExoAppInternal init 0x5363a0 stores base 0x994100 at `005369a8`, then 0x9940d0 at `005369b4`. So 0x9940d0 is the derived (CSWVirtualMachineCommands) vptr, and the body is the derived dtor.
- The deleting dtor 0x536be0 is placed far away, next to its only allocating user, and calls 0x665eb0.

**ConsoleFunc::Constructor_3 (0x867880 -> 0x475910)**
- Three 160 B ctors have identical fps, and the linker order of their 43 B funcholder helpers differs:
  - GOG: 0x8684e0, 0x868510, 0x868540, then the 20 B function at 0x868570.
  - Steam: 0x476570, then the 20 B function at 0x4765a0, then 0x4765c0 and 0x4765f0.
- Content settles it. The only callers of GOG 0x867880 register 'addlightside'/'adddarkside' (0x982ac0/0x982af0), and the only callers of Steam 0x475910 register the same two strings (0x984260/0x984290, `00984272 CALL 0x00475910`).

**CServerExoApp getters (twins: 14-21 same-fp thunks/getters within the block)**
- Thunks (GetCreatureByGameObjectID, GetActiveTimer, ClientToServerObjectId, GetPlayerCreatureId): the Steam callee is fp-equal to the same-named GOG Internal callee, and the callee's meaning fits:
  - a creature-type check;
  - a timer selector;
  - the 0x7f000000 / client-bit mask;
  - an id-or-invalid return.
- GetObjectArray: `0051c08f [ECX+0x1005c]` is the field that GetGameObject 0x51c0a0 (verified elsewhere) passes to CGameObjectArray::GetGameObject at `0051c0d0`.
- GetGlobalVariableTable: the immediate 0x100fc is equal. Its callers include the ExecuteCommandGet/SetGlobal* family; GOG GetGlobalNumber 0x793150 is 307 B and so is Steam 0x67db70.
- Get/SetLoadFromSaveGame: field +0x100c4.
  - The getter's 65 B caller is ExecuteCommandGetLoadFromSaveGame (GOG 0x798390 -> Steam 0x682dc0).
  - The setter's callers are DoQuickLoad (0x7d0f50) and CSWGuiSaveLoad::LoadGame (0x853a00), as on GOG.
- GetWorldTimer stays `likely`:
  - +0x10048 is the default return of GetActiveTimer, and the next getters read +0x1004c and +0x10050.
  - The GOG order is World, Timestop, Pause.
  - The callers agree (70/70), but nothing names the field directly.

**CWorldTimer GetWorldTimeHour/Minute (four 49 B twins)**
- The pushes before `CALL 0x51ab40` map the slots as follows: param_3=[EBP-0x10], param_4=[EBP-0x8], param_5=[EBP-0xc], param_6=[EBP-0x4].
- In 0x51ab40:
  - `*param_3 = (t/1000/60)/[+0x38]` (hours);
  - `*param_4 = (t/1000/60)%[+0x38]` (minutes).
- Hour returns [EBP-0x10] (`0051b12a`) and Minute returns [EBP-0x8] (`0051b16a`). Both are correct, and consistent with part C's Second/Millisecond rows.

**CVirtualMachine wrappers vs CVirtualMachineInternal bodies (twins of 28/32 B)**
- Each wrapper's single callee is a distinct VMI body in this same part, and each body is identified by its stack tag: int 3, float 4, string 5, object 6.
- The Mac Internal functions carry the same tag immediates.
- LoadScriptSituation's callee 0x703530 differs from GOG 0x6467d0 only by the empty-string ref. Ghidra's FID label `SchedulerResourceManagement::Statistics` on 0x6fdb80 is noise.

**CVirtualMachineStack::Destructor (0x641680 -> 0x6fe440)**
- The GOG fp-twin 0x641520 calls ~CExoString, so it is a different function.
- The 7 Steam callers of 0x6fe440 are the counterparts of the 7 GOG callers: CopyFromStack, the VMI ctor, ~CVirtualMachineScript, ~VMI, DeleteScript, RunScript and RunScriptSituation.

**Gob flag pairs (Enable/Disable Distortion, ForceAnimations, AlwaysRender; TurnOn/OffShadows)**
- The GOG fps of each pair are identical, so they cannot be told apart on GOG alone.
- The Steam bodies are at a constant run delta (0x3f25e0) from GOG, and the lower address writes 1 (Enable/TurnOn), the higher writes 0. The GOG labels agree with that.
- The Mac order matches as well, for example EnableForceAnimations 0xa98f6 < Disable 0xa9902.
- These rows are `likely` (thin), not suspect.

**Gob::GetPartLocalPosition (0x855260 -> 0x462cd0)**
- Calls vt+0x10c to fetch the part, then copies +8..+0x10 (position) and +0x14..+0x20 (quaternion) directly. That is local, not world.
- Mac: Local is 95 B with vt 0x110, and Position is 155 B. On GOG, GetPartPosition is the larger 187 B sibling at 0x855120. They are not swapped.

## Side finding (outside this part's rows, for the lead)
- **The real `Gob::SetScene` is GOG 0x854030 -> Steam 0x461aa0.** Part b lists GOG 0x84b0b0 (also labelled `Gob::SetScene`) as label_suspect (= FindGobIdx) with "Real Gob::SetScene: to locate"; this row locates it. The GOG DB carries the SetScene label twice, and 0x854030 is the correct one:
  - It is 134 B with the string 'UpdateScenes' and disp 0x94 x5, 0xb0, 0xb4, 0x1b4.
  - Mac `Gob::SetScene` 0xa9f7e has disp 0x94, 0xb4, 0xb8, 0x1b8 (vt+4) and 1 string.
  - It is the callee of Gob::AttachToScene on all three builds (Steam `0046d456 CALL 0x00461aa0`).
  - If the lead wants a Steam row for `Gob::SetScene = 0x461aa0` (thiscall, param 4, RET 4), it can ship as verified.

## GOG label errors found in this part
None.

## AUTO meaning checks
None in this part.

sample_errors: 0 (this part has no rows from the "Sample of the other reviewer matches" section; all 99 are escalation rows)
