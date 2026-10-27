# R1: placeable open / close / inventory script flow (Steam swkotor2, static)

Method: static decompile only (dumps under ghidra_dumps/steam_6a522e71/decomp, cg.py callers/callees, asm). No REA.
Anchors: 0xADDR + decompile line (file.c:line). Event id = (short)*param_4 in CSWSPlaceable::EventHandler 0x005f2fe0.

## Slot table (CSWSPlaceable script CResRef; byte offset = dword index x4)
Save routine 0x005f18c0 pairs each slot with its GFF label (005f18c0.c:125-157). Load routine 0x005ef570 uses the same labels (005ef570.c:317-417).
| Byte off | Label | Event that runs it (005f2fe0.c) | Event-side write |
|---|---|---|---|
| 0x2E4 | OnClosed | 0x17 (:329) | +0x3AC closer |
| 0x2EC | OnDamaged | 0x04 | +0x184 |
| 0x2F4 | OnDeath | 0x0A | +0x178 |
| 0x2FC | OnDisarm | 0x18 (:337, gated by +0x2C8 trap flag) | +0x3B8 |
| 0x304 | OnHeartbeat | 0x00 (:244) | +0x3A0/+0x3A4 |
| 0x30C | OnInvDisturbed | 0x1B (:465-471) | +0x188 caller (:466), +0x198, +0x19C |
| 0x314 | OnLock | 0x1C (:473) | +0x3BC |
| 0x31C | OnMeleeAttacked | not via switch (:210) | none seen |
| 0x324 | OnOpen | 0x16 (:325-327) | +0x3A8 opener (:326) |
| 0x334 | OnTrapTriggered | 0x1A (:349-364) | +0x3B4 |
| 0x33C | OnUnlock | 0x1D (:477) | +0x3C0 |
| 0x344 | OnUsed | 0x19 (:343-348), gated by +0x380 != 0 | +0x3B0 |
| 0x34C | OnUserDefined | 0x0B | +0x174 |
| 0x354 | OnDialog | 0x07 (:313) | none |
| 0x364 | OnFailToOpen | 0x22 (:481-483) | +0x3B4 |
Placeable fields (dword-index names inferred from use): +0x390 open flag, +0x380 usable, +0x37C has-inventory, +0x38C DieWhenEmpty, +0x3C4 ItemList inventory, +0x2C8 trap-armed, +0x4A8 open-pending.
Confidence: HIGH on labels (two independent routines agree); HIGH on event-to-slot pairs.

## Q1. Which scripts fire when a placeable is opened
- OpenInventory 0x005f5590 calls no script runner. Its callees (cg.py) contain no 0x006fd8d0 (CVirtualMachine::RunScript). HIGH.
- It queues event 0x16 with zero delay: 0x005f5728 CALL 0x0051e170 (CServerAIMaster::AddEventDeltaTime). 5f5590.c:48.
- Event 0x16 runs OnOpen (+0x324) inside EventHandler, later: 005f2fe0.c:325-327 `param_1[0xea] = param_3; FUN_006fd8d0(param_1 + 0xc9, ...)`. HIGH on mapping, MEDIUM on deferral timing (AddEventDeltaTime queue 0x51e220 not read).
- OnUsed (0x19) and OnInvDisturbed (0x1B) are NOT fired by the open path. HIGH for OpenInventory.
- Second open-like routine 0x006062c0 (AIActionUseObject caller) also sends 0x64b620 and queues 0x16 to itself, sets flag |=2. Target type unresolved. MEDIUM.

## Q2. Ordering: script vs GUI items
- Client open message goes out first, synchronously: 0x005f568a CALL 0x0064b620 (msg 0x19 to client, 5f5590.c:32). Its has-items flag is `local_1c = (ItemList count != 0)` (5f5590.c:~31), computed BEFORE OnOpen runs.
- Then OnOpen runs later from the queued 0x16 (see Q1). Loot created inside OnOpen is therefore NOT in the has-items flag sent at open.
- Whether the client later receives the new items (inventory sync) was not traced. MEDIUM on ordering, OPEN on client result.

## Q3. Remove / acquire items and OnInvDisturbed
- RemoveItem 0x005f4e60: calls 0x00600380(item, owner=0x7f000000, notify=1, 0x7f000000) at 0x005f4eb7. The 0x1B send in 0x600380 needs a valid NEW owner (600380.c:221-262, local_60 must be valid). With none, no OnInvDisturbed fires. MEDIUM-HIGH.
- AcquireItem 0x005f4ed0: calls 0x00600380 at 0x005f5145 with the acquirer id as new owner. If the source is a placeable, 0x600380 queues 0x1B to that placeable with caller = acquirer (600380.c:253-262). MEDIUM (arg mapping read from asm, not confirmed by a test).
- OnInvDisturbed sets the last-disturber field +0x188 only when event 0x1B is processed (005f2fe0.c:465-466). OpenInventory and CloseInventory do not write +0x188. HIGH.

## Q4. Open state and last opener
- OpenInventory sets the open flag directly: `param_1[0xe4] = 1` (5f5590.c:52). HIGH.
- Last opener (+0x3A8) is written only when event 0x16 is processed (005f2fe0.c:326), not in OpenInventory. HIGH.
- No animation write in OpenInventory. The "Opened" sound lookup (SoundAppType) and an open delay sit in AIActionUseObject (5dfbe0.c ~300-352). MEDIUM.
- CloseInventory clears the open flag: `local_a4[0xe4] = 0` (5f5770.c:194). HIGH.

## Q5. AIActionUseObject gating before OpenInventory (0x005dfbe0)
- NPC user (no client): 0x0051c600 GetClientObjectByObjectId returns 0; no GUI, feedback 0x275b only (5dfbe0.c:306-311). HIGH.
- Usable flag: `if (local_54[0xe0] == 0) return` (5dfbe0.c:257). HIGH.
- Trap: if armed and FUN_005e13c0(actor,0) < 0x5A, queues 0x1A OnTrapTriggered instead of opening (5dfbe0.c:262, 276). HIGH.
- Already open: `local_54[0xe4] != 0` returns (5dfbe0.c:316). HIGH.
- Two-phase timer: pass 1 sends 0x275b and sets pending +0x4A8=1 (5dfbe0.c:343-344); pass 2 calls `FUN_005f5590(param_1[1], 0)` (5dfbe0.c:351, and 448 for the second site). HIGH.
- Distance: no range compare on the open path. The only squared-distance test near here is the trap-proximity check in EventHandler (`_DAT_00986b78`). Range is probably enforced upstream by the AI move. MEDIUM.
- OpenInventory callers: AIActionUseObject 0x005e055b, 0x005e09b9; EventHandler 0x005f4b68 (005f2fe0.c:554-556, feedback 0); OnApplyDamage 0x0059761d (feedback 1). HIGH.

## Corpse / body-bag path
- Body-bag placeables carry IsBodyBag, IsBodyBagVisible, DieWhenEmpty (005f18c0.c save fields). They use the same OpenInventory path. HIGH on fields.
- DieWhenEmpty: CloseInventory with ItemList count 0 sets +0x380 = 0 and queues DAT_00992420 (5f5770.c:194-210). EventHandler handles DAT_00992420 at 005f2fe0.c:491 (ClosePlaceableForAllPlayers + self-delete). HIGH on code path, MEDIUM on exact effect.
- Live creature corpses: not resolved. 0x006062c0 is the only other open-like routine seen. OPEN.

## Not resolved
- Which function queues event 0x19 (OnUsed) for placeables. The 0x19 writers found so far (0x5dec20 ~330, 0x5db8f0, 0x59c550) are creature-side or other. OPEN.
