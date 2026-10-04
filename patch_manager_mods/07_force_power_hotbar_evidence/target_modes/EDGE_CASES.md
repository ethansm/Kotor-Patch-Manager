# Item-use edge cases for @lowhp / target modes (static, 2026-10-04)

Static only (Ghidra headless read-only + exe bytes). Existing evidence in this folder (`FINDINGS.md`, `TARGETABLE.md`, `decomp_*`, `disasm_*`) is cited by file; new decompiles this session were run with `research/companion_levelup/tools/ghidra_run.sh DecompileArgs.java / DisasmRanges.java` (Steam VAs). Address-DB names (`query_ghidra.py`): CanUseItem 0x5D2F80 (verified B12b), UseItem 0x57B4C0 (high), AddItemCastSpellActions 0x5648D0 (high), AIActionItemCastSpell 0x5B8000 (verified B12b), AIActionCombat 0x6D9500 (high), GetVisibleListElement 0x5D1750 (high), GetDead 0x5477F0, GetRacialType 0x687930 (verified B15). Decompile offsets below are asm-checked unless marked decimal. Naming of fields is by use (guide 3.8), not by label.

Confidence words: VERIFIED = asm/decomp line quoted with address. HYPOTHESIS = no anchor, or anchor proves the code but not the data/name meaning.

## 0. Corrections to the brief (read first)
- **0x775600 takes 4 stack args, not 5.** Signature `thiscall PerformUseItem(ECX unused; itemId, packedByte, targetObjId, float* pos) RET 0x10` (`00775606 MOV [EBP-0x14],ECX` is stored and never read; `00775660 RET 0x10`). It forwards to 0x879AF0 (msg 6/9). The "user" is NOT a parameter: server handler 0x65BE70 case 8 reads the item/target from the packet and calls `UseItem 0x57B4C0` on the **sending player's controlled creature** (decomp_65be70.txt case 8 -> `FUN_0057b4c0(local_88,local_81,local_91)`). VERIFIED. Consequence: the acting creature is always the controlled character, whichever bar the hotbar entry sits on.
- `packedByte` low byte = **index into the item's property array** (`0x57B4C0` does `0x604260(item, idx)`, `0057b55c CALL 0x00604260`). The vanilla HUD always passes 0 (0x77C780 `FUN_00775600(param_2,0,...)`). VERIFIED.
- The CanUseItem the vanilla *party pick* uses is called with the **target member's** server creature as ECX (0x77C780 party-pick path, FINDINGS Q5 [O]). The server's own call at `0057b544` uses the **acting** creature and strict=0 (see Q1).

## 1. Summary table

| # | Question | Answer | Anchor | Status |
|---|---|---|---|---|
| 1a | CanUseItem rejects medpac on droid / repair kit on organic? | YES, but only when called with strict=1 on the TARGET's server creature (vanilla party pick does this). The server's own call (0x57B544) is strict=0 and acts on the user, so it lets the mismatch through. Waived for item CastSpell subtype 0x81 (Recovery Stim). | 0x5D3650 `005d36f0..005d3771`; call sites `0077c818` (1,1), `0057b544` (0,0) | VERIFIED code; which baseitems rows carry 1/2 is HYPOTHESIS (2DA not read) |
| 1b | Where is the race / use-limitation check? | CanUseItem 0x5D2F80 -> 0x5D3650 (baseitems+0xB4 vs race), 0x5D3480 alignment, 0x5D32C0 class, 0x5D3A20 feat, 0x5D3B60 gender-like (vtbl+0x88), 0x5D3C90 tag, 0x5D3EE0 attribute min, 0x5D4190 module "baodur" base-item blocklist | decomp in `mods678_research/dump_excerpts/005d2f80.c` + this session | VERIFIED (structure); field meanings HYPOTHESIS |
| 1c | Does CanUseItem also refuse a healing item on a full-HP target? | YES when strict=1 and baseitems+0xAC is 0x2D or 0x1A: false if vtbl+0x9C(0) == vtbl+0x98(1) | `005d3023 CMP [EBP+0x10],0` `005d3029 CMP ..,0x2d` `005d302f CMP ..,0x1a` then vtbl+0x9C | VERIFIED compare; "HP equal" naming HYPOTHESIS |
| 2 | Racial type field | `server_creature +0x1198 -> stats; stats +0xE0 = uint16 racial type`. Droid = 5, Human = 6 (exe table 0x99A8B8 = 5,6,7,8) | `005d36f0/36fc` and `GetRacialType 0x687930` (`*(ushort*)(*(int*)(creature+0x1198)+0xe0)`) | VERIFIED field + values; "5 = droid" naming HYPOTHESIS (consistent with spec guess, no racialtypes.2da read) |
| 3 | Item -> spells.2da row | server item: count `+0x288`, array ptr `+0x290`, stride `0x1C`; entry `+0` int16 type (10 = CastSpell), `+2` int16 subtype = spells row. Row = subtype, passed through 0x576430 (identity except 0x30-class items remap 0x57..0x5F,0xBA,0xBB -> 0xF6..0x100). The HUD entry (stride 0x3C) carries NO spell id | `00604260..00604299`, `0x603B50`, `0x576430`, 0x5648D0 (`FUN_00604260(param_3)`, `+2`) | VERIFIED |
| 4a | Out of range: walk-to queued? | YES. UseItem -> 0x5648D0 (p12=0) only enqueues a combat-round special action + a Combat(0x3F) action; the Combat handler 0x6D9500 re-enters 0x5648D0 (caller `006d9ef6`) whose second mode queues MoveToPoint/MoveToObject(0x11)+0x13+ItemCastSpell(0x2E) when dist^2 > range^2 | `00564944 CALL 0x591890`, `005649ed CALL 0x53F7F0 (type 0x3F)`, decomp 5648d0 (0x11 / 0x13 / 0x2E queue) | VERIFIED (queue); second-mode arg value from 0x6D9500 HYPOTHESIS (not decoded) |
| 4b | Other area | Refused in second mode: area(target) != area(caster) -> return 0 | `00564a61 CALL 0x5453c0`, `00564a6f CALL 0x5453c0`, `00564a74 CMP`, `00564a79 XOR EAX,EAX` | VERIFIED |
| 4c | Dead / knocked-out target | 0x2E action cancels (returns failure DAT_009944BC): target obj missing, vtbl+0x94 != 0, or (+0x11AC creature && vtbl+0x9C(0) < 1). Same test on the caster aborts with `(vtbl+0x7C)(10000)` | decomp 5b8000 (`if (iVar4 == 0) { ... bVar1 = sVar2 < 1`) | VERIFIED branch; vtbl+0x94 = GetDead is HYPOTHESIS (name) |
| 4d | Stealthed target | 0x57B4C0 returns early (DAT_009944B8, no queue, no 0x17 message) when the user's perception entry for the target has bits2-3 == 1 (atlas: "hidden"). Not expected for party mates; party stealth interaction HYPOTHESIS | decomp 57b4c0 (`FUN_005d1750(param_5)`, `>>2&3 == 1`); atlas AI3 line 58 | VERIFIED code; friendly-stealth effect HYPOTHESIS |
| 4e | Solo mode | No solo-mode test found in 0x57B4C0 / 0x5648D0 / 0x5B8000. Absence is not proof (guide 3.10). Client side: member lookup 0x7E5DA0 returns 0 when idx out of range or object missing | decomp_helpers.txt 0x7E5DA0 | HYPOTHESIS |
| 4f | Can combat cancel it? | Queue-level yes: ClearHostileActionsVersus removes queued types 0xC/0xF/0x2E; the 0x5648D0 front-of-queue flag (`+0x520 != 0 && HasAction(0xC)`) inserts it AHEAD of an attack in second mode; 0x2E aborts on caster death. Net: the use is a queued action, not instant. | atlas AI6 line 69; decomp 5648d0 (`FUN_00540ca0` = AddActionToFront vs `FUN_0053f7f0`); 0x57B4C0 `FUN_0053f560(0xC)` | VERIFIED pieces; whether an enemy hit interrupts the 0x2E animation phase is HYPOTHESIS |
| 5a | Modded spells.2da rows | Engine uses subtype as the row directly (0x576430 identity). A column-driven filter works; a hard-coded row list does not | 0x576430 | VERIFIED engine side |
| 5b | Items with charges / per-day uses | CanUseItem does NOT test charges. 0x5B8000 tests `0x603B50(idx)` at animation start and cancels with message 0x17 when 0 (except use-types 0x0D and 0x07). Use-types (entry +6): 1 stack-consumed, 2..6 item charges (item+0x298), 8..12 per-day (entry +0x10), 0x0E..0x12 once-per-day | decomp 603b50, 6070a0, 5b8000 | VERIFIED code; type labels HYPOTHESIS |
| 5c | Last item of a stack | Consumed at animation start by 0x6070A0: stack at item+0x2CC; `<2` clears entry flag (+0xC) and the item object is destroyed (0x51C600/0x64B760 branch). Item id is dead afterwards | decomp 6070a0 case 1; DB `struct_fields` CSWSItem 0x2CC "stack size" (atlas AI7) | VERIFIED |
| 5d | Plot items | No plot test in CanUseItem / UseItem / 0x5B8000 / 0x6070A0 | the five decompiles | HYPOTHESIS (absence) |
| 5e | First property not CastSpell | UseItem refuses: `0x604260(idx)` must have type 10 (or 0x25 / 0x2E) -> else returns 0 and the handler sends feedback 0x17 | `0057b55c`, decomp 57b4c0, 65be70 case 8 (`FUN_00584da0(0x17)`) | VERIFIED |
| 6 | What does the vanilla HUD invoke target? | Never "the current friendly target". Callback ECX = bar owner; target = `[ECX+4]`. >=2 party members: first click only arms party-pick mode (gui+0x15230 = 1, item in +0x15238); the player then clicks a portrait (slot gui+0x15234) -> CanUseItem(member server, item, 0, strict=1) -> PerformUseItem(item,0,member id,member+0x24). Solo (party count == 1) or baseitems+0xAC == 0x14 or 0x2F: immediate use on the owner | 0x751750 call at `007519a5`; 0x77C780 decomp (`*piVar4 != 1`, `0x15230/0x15234/0x15238`, `0xac == 0x14`) | VERIFIED flow; DAT_00A7EAD4 alternate branch (CanUseItem(item,1,1) + 0x77CC80) meaning HYPOTHESIS |
| 7 | Is Self valid for every class? | Engine accepts Self for any item whose property idx is CastSpell and that passes the user's CanUseItem. No class-specific Self refusal exists in the engine. Refuse in UI: hostile/grenade/launcher rows, plot/quest items you do not own the effect of, first-prop != CastSpell. Shields (itemtargeting=1) are Self-ONLY by design (Self OK, other members NOT) | 0x57B4C0 / 0x5B8000 (no self test), TARGETABLE.md | engine side VERIFIED; effect of a hostile row on Self HYPOTHESIS |

## 2. Detail

### Q1. Droid / organic and the use-limitation check
- CanUseItem is `thiscall(ECX = creature, item(server CSWSItem*), p2 unused, strict)`. Order inside 0x5D2F80: alignment 0x5D3480, class 0x5D32C0, then `0x5D3650(item, strict)` (`005d2ff1 CALL 0x005d3650`), then 0x5D3A20, 0x5D3B60, 0x5D3C90, 0x5D3EE0, 0x5D4190, level/proficiency tail. The loop `for i < item+0x28C: entry = 0x6042A0(i)` is gated by `entry+0x11 == 0xFF || (1<<entry+0x11) & item+0x2D4` (upgrade-slot mask).
- **0x5D3650 race gate** (VERIFIED, asm): `baseitem = 0x6D6E30(item)` (row `item+0xC`, stride 200, `0x6D5200`), `b = baseitem+0xB4`. If `b != 0` and the item has no CastSpell(10) property with subtype 0x81 (`005d3668 PUSH 0xa`, `005d367d PUSH 0x81`, `006042e0`), then:
  - `b == 1`: `[stats+0xE0] (word) == [0x99A8BC] (=6)` else FAIL;
  - `b == 2`: `[stats+0xE0] == [0x99A8B8] (=5)` else FAIL;
  - FAIL means `return 0` only if the strict arg `[EBP+0xC]` is nonzero (`005d3716 CMP [EBP+0xC],0 / JZ`), else it **returns 1** (allowed) and skips the rest of 0x5D3650.
  - Recovery Stim (spells row 129 = subtype 0x81) therefore works on droids and organics.
- Strict callers: vanilla HUD paths `FUN_005d2f80(item,0,1)` (0x77C780, party pick `0077cad8` and solo path) and `(item,1,1)` at `0077c818`. Lenient: server `0057b544` = `(item,0,0)` (pushes `0057b536/538` both 0) on the ACTING creature.
- So for the hotbar: call CanUseItem **strict=1 on the target member's server creature** (Route A' already says this). Do not rely on the server to reject: with `PerformUseItem` the server would accept a medpac on a droid (the spell script `k_sup_healing` is shared by rows 64-67, 84, 127-129, `twoda_spells_targeting.tsv`; whether it no-ops on the wrong kind is HYPOTHESIS, 2DA/NCS not read).
- Which baseitems rows have `+0xB4` = 1/2 is data (column name probably `droidOrHuman`, HYPOTHESIS; Steam game dir not read this session). Do not hard-code: rely on CanUseItem(strict=1).
- Healing-class full-HP refusal (1c) also lives here: strict + baseitems+0xAC in {0x2D, 0x1A} + `vtbl+0x9C(0) == vtbl+0x98(1)` returns false. In `@lowhp` this is moot for members under threshold but matters when the mod falls back to "default" (full-HP leader) -> the vanilla-style refusal, no item consumed.

### Q2. Racial type read chain
- From a client creature: `server = 0x77D800(member)` (FINDINGS Q5; or `0x51C0A0(objTable=*(app+8), id)` then vtbl+0x30 = AsCreature, as in 0x5B8000 `FUN_0051c0a0(...)`, `(**(code**)(*obj+0x30))()`).
- `stats = *(void**)(server + 0x1198)`; `race = *(uint16_t*)(stats + 0xE0)`. Quoted: `005d36f0 MOV EAX,[EDX+0x1198]`, `005d36fc MOV DX,word ptr [ECX+0xE0]`, and GetRacialType `0x687930`: object type byte `local_10[2] == 5` (creature), `iVar1 = vtbl+0x30()`, `*(ushort*)(*(int*)(iVar1+0x1198)+0xe0)`.
- Values: table at VA 0x99A8B8 = `05 00 00 00 06 00 00 00 07 00 00 00 08 00 00 00` (file 0x5994B8). 5 and 6 are what CanUseItem compares. "droid = 5" matches the spec guess; the exe does not name it (HYPOTHESIS for the name).
- Better fallback than reading race: ask CanUseItem(strict=1) (Q1), which already encodes the per-item rule. Keep the race read only as a cheap pre-filter to avoid calling CanUseItem 3x per frame.
- Always `SafeRead` the chain: `server` null, `+0x1198` null are possible (member not in area).

### Q3. Item -> spells row
- Server item `CSWSItem`: property count `+0x288`, array `+0x290`, entry stride `0x1C` (`00604286 IMUL EAX,EAX,0x1c`, `0060428c ADD EAX,[ECX+0x290]`). A SECOND list lives at `+0x28C/+0x294` (0x6042A0) used for use-limitation properties; do not mix them.
- Entry layout seen in code (stride 0x1C): `+0 int16 type`, `+2 int16 subtype`, `+6 int16 use-type`, `+0xC byte usable`, `+0x10 byte per-day remaining`, `+0x11 byte upgrade-slot (0xFF = none)`. Offsets +0..+2 and +6/+0xC/+0x10/+0x11 VERIFIED from 0x603B50 / 0x6070A0 / 0x603F60; names HYPOTHESIS.
- CastSpell: `type == 10`; spells row = `subtype` (`0x5648D0`: `local_ba = *(ushort*)(entry+2); FUN_00576430(...)`; 0x576430 only remaps for items whose baseitem class (+0xAC) is 0x30). `@lowhp` should read the row of the property at index 0 (what PerformUseItem will use). If property 0 is not CastSpell the use will fail (Q5e), so skip that item.
- The HUD entry (stride 0x3C, hud+0x78+g*0xC) has `+8` key (= itemObjId | 0x40000000), `+0xC` callback, `+0x1C` object lookup (0x73F4D0), `+0x30` flags; no spell id. Client-side: resolve key `& 0xBFFFFFFF` -> server item with `0x51C8B0` then `0x51C0E0` (0x77C780 head) and read the list above.
- Read chain: `itemId = key & 0xBFFFFFFF; item = 0x51C0E0(0x51C8B0(itemId))` (server CSWSItem*, null-check); `n = *(int*)(item+0x288); e = *(void**)(item+0x290)`; `if n>0 && *(int16*)(e+0)==10: row = *(int16*)(e+2)`. Then index `twoda_spells_targeting.tsv` by `row`.

### Q4. PerformUseItem when the target is not trivially reachable
- Phase 1 (immediate, server `0x57B4C0`): checks CanUseItem(acting, strict 0) (`0057b544`), property idx, property use flag `entry+0xC != 0`, optional perception veto (Q4d), then `0x5648D0(..., p12 = 0)`. With p12 == 0 that function only (a) stores a combat-round special action via 0x591890 (node type 10 at +0x10, item +0x64, idx +0x68, target +0x44, pos +0x38..0x40; `00564944 CALL 0x00591890`), (b) if the combat round is not mid-action (`+0x520 == 0 && 0x590110()`) calls `0x563c90(4)` (cancels the current round), (c) enqueues Combat action 0x3F if none (`005649a3..005649ed`, `CALL 0x0053f7f0`), returns 1. The packet handler never learns whether the cast later succeeds; its `return 0` -> 0x17 feedback only covers the phase-1 refusals.
- Phase 2 (from `AIActionCombat 0x6D9500`, caller `006d9ef6`): `0x5648D0` with p12 != 0 (value not decoded, HYPOTHESIS). Here `target = 0x51C0A0(id)`; null -> return 0; **area(target) != area(caster) -> return 0** (`00564a74`). Then the NPC-only perception gate (`caster+0x11AC == 0` -> entry flags), `range = 0x5642D0`, `dist^2 = |caster+0x94 - pos|^2`; if `dist^2 > range^2` it queues `0x564770/0x5644B0` (MoveToPoint family), `0x11` (MoveToObject with radius), `0x13`, then `0x2E` ItemCastSpell, with the front-insert variant when `local_a0` (an attack action is queued and `+0x520 != 0`: `0057b6a1 CALL 0x0053f560` with 0xC). **So an out-of-range target is walked to, by the controlled character.**
- 0x2E executor `0x5B8000`: aborts (fail DAT_009944BC, anim 10000) if the actor is dead/HP<1; if the target object vanished; if the target creature is dead (vtbl+0x94) or HP<1 for +0x11AC creatures (this is the "unconscious party member" case); if the item (`0x51C0E0`) or property entry is gone; and at animation start if `0x603B50(idx) == 0` with message `0x584DA0(0x17)`. It then calls `0x5482A0` (the spell cast) and `0x6070A0` (consume) only on success.
- Stealthed: only the user's perception entry veto in 0x57B4C0. Party members perceive each other, so expect no effect; the veto does not block a use that is already queued.
- Solo mode / other-area member: the client member pointer from `0x7E5DA0` may be null and the server creature from `0x77D800` may be null; neither is rescued by the engine. Skip such members in `chooseTarget` (not "eligible").

### Q5. Misclassification risks of a "healing-only" rule (`isHealing` = usertype 4, range T, category in {0x1408,0x1404,0x1048})
1. **Modded rows**: engine takes `subtype` as the row (Q3), so a mod that adds a T-range 0x1408 row works; a hard-coded `{64..67,84,127..129}` list silently drops it. Keep the filter column-driven. Rows with blank hostilesetting are treated as not-healing (TARGETABLE.md).
2. **Plot items**: nothing in the engine refuses them (Q5d). A plot item with a medpac CastSpell property would be consumed. Low risk; HYPOTHESIS.
3. **Charges**: an item with use-type 2..6 / 8..12 can be at 0 uses while still in the inventory (entry flag `+0xC` cleared). CanUseItem allows it; the cast then cancels with 0x17 and wastes a turn. The rule must test `entry+0xC != 0` (and for per-day `entry+0x10 != 0`) before choosing.
4. **Last of a stack**: consumed once at animation start; the object disappears. Two PerformUseItem calls before the first executes can both pass phase 1 and the second fails at 0x5B8000. Debounce per item per member until the stack count (`item+0x2CC`) or the member's HP changes; never reuse a cached item id across the call.
5. **First property not CastSpell** (e.g. a stim whose property 0 is something else): refused at phase 1 (Q5e). Skip.
6. **Class 0x2D / 0x1A items at full HP** are refused by CanUseItem(strict=1) (Q1c): not a bug, but `@lowhp` fallback to the leader on a full-HP leader will burn a click without effect. Prefer "no valid target -> do nothing" (logged) over fallback.

### Q6. Vanilla HUD item invoke (0x751750 -> 0x77C780)
- `0x751750 HudInvokeGroup(this=hud, g)` picks the selected entry (`hud+0x1C7C+g*4`, stride 0x3C) or entry 0, and calls `(*(entry+0xC))(entry[8], creature = 0x7E5D80())` with ECX = owner + entry[0x10] (FINDINGS Q1, `007519a5`). The target is the callback's owner `[ECX+4]`, not a selected target.
- `0x77C780` (decomp_77c780.txt), `DAT_00A7EAD4 == 0`:
  - `party = 0x73FB90(); if (*party != 1)` (>= 2 members): if `gui+0x15230 == 0` (not armed): baseitems+0xAC == 0x14 -> `0x775600(item,0,[ECX+4],ECX+0x24)` immediately on the owner; else arm: `gui+0x15238 = item; gui+0x15230 = 1` and return (the player now picks a portrait). If armed: `slot = gui+0x15234` (-1 -> return), `member = 0x7E5DA0(party, slot)`, `server = 0x77D800(member)`, `CanUseItem(server; item,0,1)`; success -> `0x775600(item,0,member+4,member+0x24)`; the pick state is reset (`gui+0x15230 = 0`, `gui+0x15234 = -1`).
  - party of 1 (solo) or baseitems+0xAC == '/' (0x2F): `member = 0x7E5DA0(party, slot==-1 ? 0 : slot)`, CanUseItem strict, `0x775600(item,0,[ECX+4],ECX+0x24)`: the owner.
- `DAT_00A7EAD4 != 0` branch uses `CanUseItem(item,1,1)` then `0x77CC80(item, param_3)`, which passes `[this+4]`, with a zero position when `*(param_3+4) == [this+4]` (self) else `this+0x24` (decomp_77cc80.txt). What sets DAT_00A7EAD4 was not traced (HYPOTHESIS: an alternate "use on object" mode).
- Hotbar consequence: "default" for an item = the owner, same as vanilla; "lowhp" is a mod-only feature that mirrors the armed-pick endpoint.

### Q7. "Use on self" validity by class
`PerformUseItem(*, item, 0, selfId, self+0x24)` passes phase 1 for every item that (a) passes CanUseItem of the ACTING creature (lenient), (b) has CastSpell/0x25/0x2E at the chosen index. There is no self-vs-other filter in 0x57B4C0, 0x5648D0 (`local_ac != local_b0` only skips the NPC perception gate), or 0x5B8000 (it even picks the animation by `local_88 == param_1` for item classes 0x19/0x1A/0x2D, i.e. self-use is a first-class path).

| class | Self | Other member | Handling |
|---|---|---|---|
| Medpac / antidote / repair kit / recovery stim (0x1408/0x1048/0x1404) | OK (CanUseItem strict also checks droid/organic and full HP) | OK, strict CanUseItem on target | use vanilla pick semantics |
| Stims / adrenals / combat shots (0x1808, T) | OK | OK (spell script decides) | allow, not in `@lowhp` |
| Energy shields, droid deflector/hazard shields, disguises, space suits, Cold Serum, Heat Shield, Cloaking Device (itemtargeting=1 or row 108) | OK (intended) | **refuse** | deny "other" in UI |
| Grenades, rockets, darts, launcher ammo, Shield Disruptor, Oil Slick (hostilesetting 1 / forcehostile set / itemtargeting 2,3) | **refuse** (area/hostile effect centred on the caster; HYPOTHESIS for the actual damage) | refuse | deny in UI: Items filter already excludes them from target modes; also exclude from any Self re-routing |
| Plot / quest items | no engine test; unknown effect | same | accept engine behaviour, log |
| Item whose idx-0 property is not CastSpell | refused at phase 1 (0x17) | refused | skip entry in chooseTarget |
| Item at 0 charges / empty use flag | passes phase 1, cancels at 0x5B8000 (0x17) | same | filter in chooseTarget (entry+0xC) |

## 3. Recommended handling (per case)

| case | handling | where |
|---|---|---|
| droid vs organic (1a) | **filter in chooseTarget**: call CanUseItem strict=1 on each candidate's server creature (vanilla sequence) and skip failures; optional cheap pre-filter on `race` 5/6 | chooseTarget |
| full-HP healing-class (1c) | filter in chooseTarget (HP below threshold already implies not full) ; no fallback to leader | chooseTarget |
| dead / knocked-out target (4c) | **filter in chooseTarget**: skip `HP < 1` and null client/server pointers | chooseTarget |
| other area / solo-mode absent member (4b/4e) | filter in chooseTarget: null `0x7E5DA0` / `0x77D800`, or area mismatch (`0x5453C0`) | chooseTarget |
| out-of-range (4a) | **accept engine**: the controlled character walks over (queued action). Log distance. Decide at design time whether to also require range < N m to avoid the party sprinting into combat | log + optional filter |
| stealth veto (4d) | accept engine refusal + log | log |
| combat interrupt (4f) | accept engine behaviour + log | log |
| charges / use flag (5b) | filter in chooseTarget (entry `+0xC`, `+0x10`), plus UI badge already shows stack count | chooseTarget |
| last of stack (5c) | debounce per (item, member) until stack count or HP changes; re-resolve item id each call | UI / use path |
| modded rows (5a) | keep column-driven rule; never hard-code row ids | chooseTarget |
| first property not CastSpell (5e) | filter in item eligibility (read property 0) | chooseTarget |
| itemtargeting=1 (7) | deny "other" modes in UI (already: mode button disabled / not isTargetable); allow Self | deny in UI |
| hostile/grenade (7) | deny in UI for all modes except vanilla default path | deny in UI |
| plot items (5d) | accept engine behaviour + log | log |

## 4. Verification (Haiku sieve, decoys)
Method per RE_AGENT_GUIDE 4.1-4.3: evidence file of asm excerpts (`scratchpad/haiku/evidence.txt`), 3 real claims + 3 decoys, TRUE only with a quoted address, verdict field last. Result (`scratchpad/haiku/verdicts.json`):

| claim | kind | Haiku | judged |
|---|---|---|---|
| 0x57B544 CanUseItem last arg = 0 (strict off) | real | TRUE (`0057b536 PUSH 0`) | correct |
| 0x77C818 CanUseItem last arg = 1 (strict on) | real | TRUE (`0077c80d PUSH 1`) | correct |
| 0x5D3650 race = word at [[this+0x1198]+0xE0] vs table 0x99A8BC | real | FALSE | **my wording error**: the claim said "dword table entry"; asm is `MOVZX ECX,word ptr [0x0099a8bc]` (a word read of a dword cell, value 6). Offsets 0x1198/0xE0 were not contested. Wording fixed here (race compare is a 16-bit read). |
| 0x57B544 last arg = 1 | decoy | FALSE | correct reject |
| race read from [[this+0x1190]+0xE2] | decoy | FALSE | correct reject |
| 0x6042A0 uses the same list as 0x604260 | decoy | FALSE (0x28C/0x294 vs 0x288/0x290) | correct reject |

Score: 2/3 real claims confirmed, 1 mis-worded (offsets confirmed), 3/3 decoys rejected (0 false accepts). Haiku is a sieve only; the quoted asm above is the anchor.

## 5. Open items (not statically provable)
1. Value of p12 passed by 0x6D9500 to 0x5648D0 (`006d9ef6`), and whether an incoming hit interrupts the 0x2E animation phase.
2. baseitems.2da `+0xB4` / `+0xAC` per-row values and racialtypes row names (needs the user's game data; deliberately not read this session).
3. What `DAT_00A7EAD4` is and when 0x77C780 takes the alternate branch.
4. k_sup_healing.ncs behaviour on the wrong kind (medpac on a droid) when CanUseItem is not strict.
5. Live probe (TargetProbe) to confirm walk-to + consume-once on a distant member, as already listed in FINDINGS.md.
