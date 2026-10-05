# R5: why CanUseItem 0x5D2F80 returns 0 in companion-selfheal (static, 2026-10-05)

Steam addresses. Dumps in `dump/` (`.c` line numbers below are `cat -n` of `dump/005d2f80.c`; asm addresses from `dump/005d2f80.asm`).
Live log (`companion-selfheal_log.txt`, 21 lines): `REFUSE canuse=0 member=1 item=000002d0` x6, `member=2 item=000002d1` x1. No cand dump in that log, so the exact failing branch is NOT observable statically (see "Branch ranking").

## Q1 Parameters
`bool __thiscall CanUseItem(CSWSCreature* this, CSWSItem* item, int unused, int strict)`, `RET 0xC` (`005d32b2 c20c00`).
- this: `[EBP-0x58]` = ECX saved at `005d2f87`; used as the creature (`+0x1198` stats ptr, `+0x10E8`, `+0x53C`, vtable `+0x98/+0x9C`).
- item: `[EBP+8]`, a CSWSItem pointer (null -> return 0, `005d2f91 CMP [EBP+8],0` / `005d2f97 XOR EAX,EAX`). Base item = `0x6D5200(item+0xC)` (`005d31a6`, `006d6e30` at `005d3008`).
- param 3 (`[EBP+0xC]`): UNUSED. Only read at `005d3168 CMP [EBP+0xC],0`, a block Ghidra marks unreachable (`005d316e MOV EAX,1; TEST EAX,EAX; JNZ` always taken). Name: `unused`.
- param 4 `strict` (`[EBP+0x10]`): forwarded to `0x5D3650(item, strict)` (`005d2fe6..2ff1`) and tested at `005d3023 CMP [EBP+0x10],0`.
The mod's args `(m.server, itemPtr, 0, 1)` are CORRECT and identical to vanilla (Q3).

## Q2 Branches that return 0 (order of evaluation, `.c` line refs)
Accumulator `bVar9`; any failing gate -> 0.
1. `.c:19` item==0.
2. `.c:23-27` `0x5D3480` alignment gate (property 0x2B, `5d3480.c:10,27`) and `0x5D32C0` class-restriction gate (property 0x2C, `5d32c0.c:11,32,43`).
3. `.c:29-33` `0x5D3650(item,strict)` (`005d2ff1`): (a) race gate: `baseitems+0xB4` in {1,2} vs creature race `short *(stats+0xE0)` against `DAT_0099A8BC` (1) / `DAT_0099A8B8` (2) (`5d3650.c:25-47`); on mismatch returns `strict ? 0 : 1` (`5d3650.c:30-33,41-44`) = STRICT-ONLY. Waived when item has property 10 subtype 0x81 (`5d3650.c:17-24`). (b) class mask `baseitems+0xB8 & (1<<class byte stats+0xEC)` != 0 -> 0 (`5d3650.c:48-51,92-94`), strict-independent.
4. `.c:35-42` STRICT-ONLY full-HP gate: if `strict` and `baseitems+0xAC` (type byte) is 0x2D or 0x1A (`005d3029 CMP ..,0x2d`, `005d302f CMP ..,0x1a`) then `this->vtbl[0x9C](0) == this->vtbl[0x98](1)` -> 0 (`005d3045 CALL EAX`, `005d305f CMP ESI,ECX`, `005d3061 JNZ`). Party.h/doc 08 L513: `+0x98(1)` = max HP (`0x57E8C0`), `+0x9C(0)` = current HP (`0x5413C0`). A hurt member passes.
5. `.c:45-59` `0x5D3A20` feat (prop 0x39), `0x5D3B60` gender-like via vtbl+0x88 (prop 0x3C), `0x5D3C90` tag "ExpectedTag" (prop 0x3E), `0x5D3EE0` min ability (prop 0x45), `0x5D4190` module "baodur" base-item blocklist (`5d4190.c:7-12`). All strict-independent; all pass when the item lacks the property (`5d3a20.c:29`, `5d3b60.c:36`, `5d3ee0.c:73`).
6. `.c:60-66` item-level restriction: only if `creature+0x10E8 == 1` (`005d310a..3113`): total level (`0x6A4E00`, ECX=stats `005d311c`) < `0x606C30` item-value level AND `0x51BD40()+0xD0 != 0` (`005d3154`) -> 0. Strict-independent, likely off.
7. `.c:67-73` base-item level gate: `baseitems+8 != 8 && +9 != 0 && (baseitems+0x1B - creature+0x53C) > 1` -> 0 (`005d31c1..3224`). `.c:74-83` feat-mask loop over `baseitems+4` bits 0..0x13 -> `0x5BDA40` (`005d3299`). Strict-independent.
NOT checked: possession/owner (no inventory test anywhere in 0x5D2F80 or callees; same conclusion as R1 "Item possession"), droid-vs-organic by name (it is the race gate in 3a), combat state, item charges.

## Branch ranking for the live REFUSE (hurt organic, party has medpacs)
Static evidence cannot name the branch. Facts: (i) 0x57B544 (the engine's own gate inside UseItem) calls it LENIENT, `(item,0,0)` on the acting creature (`0057b536 PUSH 0; 0057b538 PUSH 0; 0057b544 CALL 0x5D2F80`); (ii) our call is strict. Strict adds only gates 3a and 4. Gate 4 is excluded for a member with cur<max (same vtbl reads Party.h uses for hp). So either:
- H1 (most likely): `analyzeMember` (CompanionSelfHeal.cpp:127-165) never calls CanUseItem; it picks by `heal::pickHealItem` over every spell-row item in HUD group 1 (`usable = isSpell && p.usable`, :148-152), so a Repair Kit / droid-only item can be picked for an organic (or a Medpac for a droid) and the post-pick CanUseItem (:276) correctly refuses via gate 3a. Both refused items (2d0 for member 1, 2d1 for member 2) are single picks repeated each click; the log has no cand lines to confirm the item kind. R3 FINDINGS.md:23,38 assumed candidates were CanUseItem-gated; the code does not do that.
- H2: gate 3a race mismatch for a real medpac (race value of that companion is not the one `DAT_0099A8BC` expects; table at 0x99A8B8 = 5,6,7,8 per EDGE_CASES.md:52; names unverified).
- H3: a strict-independent gate (class/feat/level) also refuses; then the engine's own lenient call at 0x57B544 refuses too and route A is a silent no-op anyway.

## Q3 What vanilla / the hotbar call
- Vanilla party pick and solo path 0x77C780: `FUN_005d2f80(serverItem, 0, 1)` with ECX = TARGET member's server creature (`dump/decomp_77c780.txt:84-85` after `0x77D800`, `0x51C0E0(itemId)`; solo `:107-108`); asm `0077c80d PUSH 1; 0077c80f PUSH 1; 0077c818 CALL 0x5D2F80` is the other variant `(item,1,1)` (decomp :119). So vanilla = strict=1, param3 ignored. Identical to our call.
- ActionHotbar.cpp: `useOnMember` :522 and `targetedUse` eligibility :562 and :900 call `CanUseItem(v[i].server, itemPtr, 0, 1)`, then `PerformUseItem`. Same signature; there it is applied PER CANDIDATE MEMBER before choosing (c[i].eligible, :561-562), which is why the hotbar does not hit this refusal loop.
- Server side 0x57B4C0 (reached by route A 0x57D8B0): lenient `(item,0,0)` on the acting creature (`0057b544`).

## Q4 Recommendation: (a)
Keep the args `(member server, itemPtr, 0, 1)`; they are already correct and match vanilla 0x77C780. The defect is the call site: move the check INTO candidate building. In `analyzeMember` (:127-165) set `usable = isSpell && p.usable != 0 && p.itemPtr && m.server && CanUseItem(m.server, itemPtr, 0, 1) != 0` so `pickHealItem` only ever sees items the engine accepts for that member (what R3 FINDINGS.md:23 intended and the hotbar already does). Keep the post-pick check at :276 as a guard. Do not choose (c): route A (0x57B4C0) is lenient, so without strict filtering it would accept a Repair Kit on an organic or a Medpac on a droid (`EDGE_CASES.md:16`, 1a) and burn the item.
Side effect to know: strict also refuses a Medpac at full HP (gate 4); auto/click paths already require `missing > 0`.
Confirm H1 vs H2 with one probe line on refusal: log `baseitems+0xAC` (`*(0x6D5200(item+0xC)+0xAC)`), `+0xB4`, race `*(short*)(stats+0xE0)` and item name. Cost: 1 log line, no game-state change.
