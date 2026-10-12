# Q-C recon: how a creature's HP and Down state are read

Scope: read-only. Steam build (swkotor2.exe 6a522e71) addresses unless noted.

## C1. companion_selfheal HP read and Down decision

No separate Down function. Down is computed inside the shared helper `party::vitals`, and selfheal only consumes `m.down`.

Quote (Party.h and CompanionSelfHeal.cpp):
```
Party.h:99    constexpr int SlotMaxHp = 0x98;   // arg 1
Party.h:100   constexpr int SlotCurHp = 0x9C;   // arg 0
Party.h:115   v.max = g_callVital(fMax, server, 1);
Party.h:116   v.cur = g_callVital(fCur, server, 0);
Party.h:117   v.ok = v.max > 0;
Party.h:118   v.down = v.ok && v.cur <= 0;
CompanionSelfHeal.cpp:810  int cnt = party::members(v, 3);
CompanionSelfHeal.cpp:815  c.m = v[i];
```

Call chain: `refreshCache` (CompanionSelfHeal.cpp:806) -> `party::members` (Party.h:125) -> `party::member(table,i)` via PartyGetAt -> `party::serverCreature(client)` via MemberServerCreature (Party.h:89) -> `party::vitals(server,&v)` (Party.h:108).

Engine addresses (Steam, `patch_manager_mods/_shared_src/GameAddr.h`):
- PartyGetAt 0x007E5DA0 (GameAddr.h:34)
- MemberServerCreature 0x0077D800 (GameAddr.h:35), thiscall(client member) -> CSWSCreature*
- GetControlledCreature 0x007E5D80, ClientPartyTable 0x0073FB90 (not used for HP)

HP is read through the server creature vtable, not a direct function:
- max HP = `[vt+0x98]`, thiscall(creature, 1), RET 4 (K2Addr.h:306, Steam table makeSteam at K2Addr.h:195)
- cur HP = `[vt+0x9C]`, thiscall(creature, 0), RET 4 (K2Addr.h:307)
- Struct offsets: `CSWSCreature` vtable pointer at `+0x0` of the server object (`saferead::readAt(SiteBase, server, 0, &vt)`).

Caveat: `ok` is false when max <= 0 or any read fails. `down` is false in that case, so "not ok" means unknown, not alive.

## C2. Shared helper

Yes. `party::vitals(uintptr_t server, Vitals* out)` in `patch_manager_mods/_shared_src/Party.h:108-121` already returns `down`. `party::members(Vitals* out, int cap)` at Party.h:125 fills all party slots at once. Queue-hud already includes Party.h (QueueHud.cpp:31), so it can call these with no copy.

Other consumers: `ShieldLogic.h:25` checks `Creature_VtMaxHp` and `Creature_VtCurHp` from the K2 table.

## C3. queue-hud existing stats and creature pointers

- No stats access. `rg -n 'stats|Stats|CSWSCreatureStats'` in `patch_manager_mods/queue_hud_src` returns nothing.
- Hex hits are not creature stats: QueueHud.cpp:62 `AddrRulesGlobal = 0x00A1B4D0` (spell table), QueueHud.cpp:65 `PauseInternal = 0x10078` (player pause byte).
- Creature pointers exist per member already:
```
QueueHud.cpp:249  uintptr_t m = party::member(tbl, k);
QueueHud.cpp:639  if (tbl) for (int k = 1; k <= 2; ++k) { cli[k] = party::member(tbl, k); srv[k] = cli[k] ? party::serverCreature(cli[k]) : 0; }
```
So `srv[k]` is the server creature for companion k (1-2). `party::vitals(srv[k], &v)` gives `v.down` with no new code.

## C4. Engine dead / unconscious flag

Symbol and address lookups with `scripts/query_ghidra.py`:

| Name | Steam addr | Notes |
|---|---|---|
| CSWSObject::GetDead | 0x005477F0 | name_map steam-db-B0, conf high. GOG body 0x006AE4D0, 202 bytes. Body not decoded in this pass. |
| Global::ExecuteCommandGetIsDead (script cmd 140) | 0x0068B4B0 | engine_ai_atlas/AI7_script_interface.md:29, :60. Rule: `IsDead (vt+0x94)` OR (`+0x11AC != 0` AND HP <= 0). |
| CClientExoAppInternal::GetControlledCharacterDead | 0x0079C060 | Controlled PC only. Not useful for companions. |
| GetCurrentHitPoints | none (GOG 0x006A80A0 only) | No Steam mapping found. |

Not found: IsDead, GetIsDead, GetIsDying, IsDying, Unconscious (no symbol hits).

Key point: the engine's own script-level party "dead" test for companions is HP <= 0 (AI7 atlas, :128). Party.h:95 says the same: "KOTOR party members at 0 HP are unconscious/dead; the engine's dead check is never called." So `party::Vitals.down` (cur <= 0) matches the engine rule.

## Answers

- Accessor: `party::vitals(server,&v)` (Party.h:108), `down = ok && cur <= 0`.
- Shared helper: yes, Party.h, already included by queue-hud.
- Engine flag: no unconscious flag found. CSWSObject::GetDead 0x005477F0 exists but its body is undecoded.
- OPEN: Does queue-hud need only HP <= 0 (`party::Vitals.down`, same rule the engine uses for companions), or the flag behind CSWSObject::GetDead?
- VERDICT: CONFIRMED. The reusable HP read exists with known offsets (+0x98 max, +0x9C cur).
