# A: CSWCCreature::UpdateWeaponClass (0x0076D590), Steam swkotor2.exe

Method: static dumps (decomp/, asm/, callgraph), cg.py, field_census.py, static_facts.py. No REA, no game files. Table matches research/equip3d/f17_dualwield_style.md (asm re-read agrees).

## Signature and reads
- VERIFIED: __thiscall, ECX = client CSWCCreature, no stack args, ends `0x76D875 RET` (ret 0), void.
- VERIFIED: slot read 0x76E610(slot) (thiscall, ret 4). 0x10 -> creature+0x238, 0x20 -> creature+0x23C. Empty = 0x7F000000 (cmp 0x76D5D0 / 0x76D5D9). Right/left naming is GUESS (no RIGHTWEAPON string).
- VERIFIED: item = 0x73F530 (name map: GetItemByGameObjectID). Baseitem row = 0x6D5200(item+0xC): table CSWRules[0xA1B4D0]+0x34, row = base + id*200, 0 if out of range (asm 0x6D5203-0x6D5236).
- VERIFIED: row+0x08 = weaponwield (REA ledger 0x6D5240 "MOV [ECX+8],DL"). row+0x09 -> creature+0xC7.
- VERIFIED decomp 0x615C80: returns 1 if server item +0xC is 8, 9 or 10. Name map says IsLightsaber, F17 says "ranged". Label GUESS.
- VERIFIED: anim base = 0x7ED830 = `MOV EAX,[ECX+0x68]; RET`. No NULL check before the writes.

## Q1: +0xC5 value (VERIFIED asm 0x76D5B2..0x76D7D2)
| right 0x10 | left 0x20 | +0xC5 | +0xC7 written |
|---|---|---|---|
| empty | empty | 8 (unarmed) | 2 (0x76D5F4) |
| empty | item | 0 | no |
| item, lookup fails | any | 0 | no |
| item | empty | R.ww 1..6 -> 1,2,3,5,7,9; else 0 | R.row+9 (0x76D6FE) |
| item | item | L.ww 2 -> 4, 4 -> 6, else 0 (R ignored) | L.row+9 (0x76D7BB) |
- Left hand matters: left-only gives 0, not 8.

## Q3 inputs: every write (VERIFIED census + asm)
- +0xC5: 0x76D7D2, always, last store. Only other writer: CSWCAnimBase ctor 0x85F8C3.
- +0xC7: 0x76D5F4, 0x76D6FE, 0x76D7BB (rules above). Correction: wave2_3.md:644 and R7f omit +0xC7.
- +0xD0: 0x76D7E9, always. 1 only if right is lightsaber-class and left is not a non-lightsaber server item.
- +0xD4 (dword): 0x76D84E = 1 / 0x76D868 = 0, only when 0x77D800(creature) and creature+0x1198 are non-null. Value = HasFeat(0xCF) via 0x6B83F0 -> 0x6C0AA0 (linear scan).
- Side effect: 0x7F2540 lazily binds a client object to its server object, caches it at object+0xF8, calls 0x54A8E0 (uncharacterized). Used by 0x7FEE70 (item) and 0x77D800 (creature; name map says "party entry", GUESS).
- Other callees read-only: 0x76E610, 0x7ED830, 0x73F530 (-> 0x78BD80 lookup), 0x6D5200, 0x615C80, 0x6B83F0. No anim play, no network send.

## Q2: callers (cg.py: 5 sites in 4 functions)
- 0x76E130 CSWCCreature::EquipToInventorySlot, site 0x76E485, only for slot 0x10/0x20, then visual update (0x76D890, 0x768060). Its callers: 0x80FE80 (clears a weapon slot holding the removed item, GUESS unequip/destroy), 0x815D00 (equip from inventory, GUESS), 0x80EFB0 (equip to preferred slot, GUESS), 0x804F10.
- 0x804F10 (6709 B), sites 0x80632F (slot 0x20) and 0x8064DD (slot 0x10): only in branch local_38 == 0, same branch calls 0x7637E0 "Loading". Load/reconcile path (GUESS), not a live equip. Chain: 0x8130D0 <- 0x80DEB0 <- 0x810CF0 <- CClientExoAppInternal::HandleMessage 0x781840 (message-driven).
- 0x85A1F0, site 0x85A684: appearance build. Sole caller CSWCCreature::SetAppearance 0x76D200 (site 0x76D355).
- Global::CreateSimple 0x85B940, site 0x85BFE6 (last call): creation. Sole caller 0x85A1F0.
- No caller is a combat-start function. No absolute pointer to 0x76D590 in the exe image (no vtable slot).

## Q3: verdict on the plan
- (a) Writing C5 = 8 is right only when both hands are empty. It is not durable: any call above while weapons stay in 0x238/0x23C rewrites C5 to the armed value.
- (a) also needs C6 == 1 (resolver gate, 0x863650 decomp line 13). D4 picks the wield-8 row: 0xF9 if D4 == 0, else 0x217. Writing C5 alone leaves D4 as last computed.
- (b) 0x76D590(creature) recomputes from slots. Weapons still equipped returns the armed value (undoes (a)). Weapons unequipped returns 8, so the weapon must go back into the slot first (0x76E130 path).
- Heavier than expected: 3 extra fields (C7, D0, D4), 2-3 baseitem lookups, 2-3 GetServerItem, up to 4 client->server lookups with first-bind cache writes, 1 HasFeat scan. Cheap per call, not measured.
- Resolver 0x863650 reads C5 (0x863670), C6 gate, D4, E8, CC (decomp lines 11-13). Wield 0 has no case, so the id passes through.

## Corrections
- The object 0x863650 reads is the anim base, not the creature: it reads the same fields the ctor 0x85F7E0 zeroes. REA ledger row labels it "creature".
- Unarmed = 8 and none = 0 match KotOR.js (research/holstered_weapons/raw/holster_engine_re.md:21).

## Open
- Do combat-start or weapon-swap paths reach 0x76E130 with slot 0x10/0x20? Not traced.
- Walk: wield 8 on ids 0x2712/0x2713 falls to default row 0x154 (decomp 0x863650 lines 62-75). Walk needs its own check.
