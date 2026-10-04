# R1: companion-as-actor route for item self-use (static, 2026-10-04)

Steam addresses (Aspyr swkotor2.exe). Decomp/disasm in `dump/` (one `.c` + `.asm` per function). Nothing run in game.
Written by Opus from the Sonnet R1 report (its Write was refused); anchors re-checked by Opus against `dump/*.asm` and
`query_ghidra.py address` (0x57D8B0 / 0x57B4C0 / 0x51C8B0 all `verified (batch B12a/B9)`).

## Signatures
| Function | Conv / cleanup | Args (stack order) | Anchors |
|---|---|---|---|
| `0x57B4C0` CSWSCreature::UseItem (GOG 0x693FF0) | thiscall ECX=creature, `RET 0x1C` (7) | `(itemServerId, propIdx byte, subIdx byte=0, targetId, float* pos, areaId, flag)` | `0057b935 c21c00`; callers `0065c5ff` (msg handler, flag 1 at `0065c5d4`), `0057d98f`/`0057d9da` (0x57D8B0, flag 0), 0x57DA90 |
| `0x57D8B0` CSWSCreature::AddUseTalentOnObjectActions (GOG 0x6963E0) | thiscall ECX=creature, `RET 0x20` (8) | `(talentType 0=spell/item 1=feat 2=skill, talentId, targetId, byte, itemId (0x7F000000=none), propIdx (0xFFFFFFFF=none), byte, byte)` | `0057da89 c22000`; `0057d8e3 MOV ECX,[EAX+0xEC]` commandable gate; `0057d964..0057d98f` item branch pushes `[ebp+0x18]` item, `byte [ebp+0x1c]` propIdx, 0, `[ebp+0x10]` target, &pos, area, 0 -> `CALL 0x57B4C0`; UseItem result ignored (silent refusal) |
| `0x66BDC0` script cmd 0x135 ActionUseTalentOnObject + 0x136 (AtLocation) | VM command | ECX = OBJECT_SELF's server creature | `0066beaa CALL 0x57D8B0` (8 pushes from `0066be84`) |
| `0x51C8B0` CServerExoApp::ClientToServerObjectId | | `id & ~DAT_0099449C` | verified B9 |
| `0x51C0E0` | | server id -> CSWSItem* | used by hotbar `serverItemFor` |
| `0x5D2F80` CanUseItem | thiscall on server creature | `(CSWSItem* item, 0, strict)` — takes a POINTER, not an id (old notes said `(itemId,0,1)`) | doc 07 decision table |

## Queue behaviour (UseItem does not apply the item)
1. `0x57B4C0` -> `0x5648D0(..., last=0)`: adds an item entry to the combat round (`0x591890`), calls interrupt wrapper `0x563C90(4)` -> ClearAllActions(1) when `+0x520 == 0` and `0x590110()`, then queues combat action `0x3F`.
2. AIActionCombat `0x6D9500` (action 0x3F) calls `0x5648D0(..., last!=0)` at `006d9ef6`: builds the `0x2E` ItemCastSpell action.
3. RunActions case 0x2E `005d5fc4 CALL 0x5B8000`: executor casts via `0x5482A0`; consumes via `0x6070A0` DecrementUses only on success.
So a direct call gives the companion its own combat-round use with its own animation, exactly like a player use — just on the companion's creature.

## Item possession
None of `0x57B4C0`, `0x5D2F80`, `0x5B8000` or the head of `0x6070A0` checks that the actor holds the item. Party inventory items (held by the PC) should be usable by a companion via the server item id. **Unverified:** the tail of `0x6070A0` (which inventory the stack is decremented from) — live probe CS3 must check the stack drops by 1 and nothing else moves.

## Blockers / cancels
| Cause | Effect | Confidence |
|---|---|---|
| `creature+0xEC == 0` (not commandable: cutscene, stunned, scripted) | 0x57D8B0 returns before UseItem (`0057d8e3`) | high |
| Any ClearAllActions(1) `0x541080` | also clears the round entry via `0x5908F0`; callers: conversations, stance/attack actions, `0x51CB10`, `0x5D75B0`, companion-ai | high |
| UseItem refusal (CanUseItem, perception veto) | silent with route A; route B returns a code | high |
| Target in another area | rejected in 0x5648D0 (self target: n/a) | high |
| Combat-AI re-plan of the companion, party leader swap | may wipe the queued 0x3F before it runs | hypothesis, live check |

## Route table
| Route | Entry | Args | Anchor | Conf | Risks |
|---|---|---|---|---|---|
| **A (primary)** | `0x57D8B0` on companion server creature (`0x77D800(member)`) | `(0, 0, selfServerId, 0, itemServerId, propIdx(0), 0, 0)` | `0057d98f`, `0066beaa` | med-high | silent no-op on refusal (pre-check CanUseItem strict ourselves); must run on game thread (frame callback is) |
| B | `0x57B4C0` direct | 7 args; caller supplies pos + area | `0065c5ff` | med | skips +0xEC gate (do it ourselves); pos/area must be correct |
| C | `0x53F7F0` AddAction(0x2E, ...) | raw action | — | low-med | bypasses CanUseItem + combat round; no |
| **D (fallback)** | script `ActionUseTalentOnObject` via `AssignCommand` (cmd 0x135 -> 0x66BDC0) | NCS + VM entry | `0066beaa` | med | same engine path as A plus a VM/NCS dependency; robustness 1.3.0 has an open AssignCommand leak note |
| E (rejected) | client `PerformUseItem 0x775600` | — | `0065c5ff` | high | server always acts as the sender's controlled creature |

## Lessons for ghidra_knowledge
- Trace the executor chain (RunActions `0x5D56B0` case table) before trusting a function called "enqueue": UseItem only adds a round entry + 0x3F; the 0x2E action is built later by AIActionCombat.
- Ghidra thiscall decomp shows ECX as param 1; arg count = `RET n / 4`; settle arg order from callsite pushes. Read the asm ECX at every thiscall callsite (decomp mislabeled it at `0066beaa`).
- CanUseItem `0x5D2F80` takes a CSWSItem pointer, not an id.
- `0x563C90` is an interrupt wrapper ending in ClearAllActions(1); the DB name "ActionManager" hides that.
- `0x5D32C0` is mis-named; it is a class-restriction check.
- DB row for `0x66BDC0` should note it serves both cmd 0x135 and 0x136.
