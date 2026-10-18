# NB2 anim driver recon (Steam swkotor2.exe, sha256 6a522e71)

Date: 2026-10-18. Read-only. Static dumps first (`ghidra_dumps/steam_6a522e71/decomp`, `asm`, `callgraph.tsv`). REA used once: `open_binary` (provider ghidra, sha256 prefix 6a522e71 confirmed), `read_bytes 0x0099ee14 len 288`, `close_binary`. No snapshot.
Ledger: `patch_manager_mods/ghidra_knowledge/rea_findings/20261018_nb2-anim-driver.tsv`. Reused: `20261016_nb2-recon.tsv` (CSWCCreature vtable head, ctor default 10000), `20261016_holster-c6-unarmed-anim.tsv` (0x863650 data slots).

## Summary

| Q | Verdict | Answer |
|---|---|---|
| Q1 | PARTIAL | +0x48 writers: 0x007ed9a0 (L517) and 0x00777e30 (L59). Both then call body holder vt+0x54. Name lookup inside holder vt+0x54: UNKNOWN. |
| Q2 | PARTIAL | Engine rotates idle variants on a timer in 0x0077d390 (pause1/pause2 + six flourishes). That function never passes the chosen name to a call. |
| Q3 | UNKNOWN | Creature driver 0x00777e30 can overwrite a direct Gob play with idle (timer expiry calls vt+0x9c). Whether it fires on the next tick depends on FUN_007ef5d0, undecoded. |
| Q4 | CONFIRMED (setter) | CSWCCreature vt+0x9C = 0x007ed9a0. thiscall (creature, short id, dword 0, int flag), retn 12. Can no-op (gates below). |

## Q1. Who turns +0x48 into a play

- **0x007ed9a0** (vt+0x9C, thiscall, retn 12, event-driven: runs only when called). Decompile L517 `*(short*)(param_1+0x12) = param_2;` writes +0x48. L520/L523 call `holder vt+0x54(id, +0x4C, mode 0|2, speed)`. Early return 0 at L106-112 if holder `vt+0x50(id)` is 0.
- **0x00777e30** (`__fastcall`, polled each tick). L59 `*(short*)(param_1+0x12) = local_c[0];` from queue pop `FUN_007efac0`. L101/L104 call holder `vt+0x54(id, dur, mode 1|3, 0)`. Called from 0x007664b0 (CSWCCreature vt+0x74, probable per-frame update) and 0x007675b0.
- **Holder** = `[creature+0x68]` (decomp `param_1[0x1a]`). Class and vt+0x54 body: UNKNOWN. The ctor 0x007645f0 asm has no write to +0x68, so the holder is set elsewhere.
- **FUN_00863650** has no direct CALL in the dumps (no callgraph rows). It is reached only through 7 data slots (ledger row). Link to the play path: UNKNOWN.

## Q2. Who writes +0x48, and the engine idle chooser

- Writers: 0x007ed9a0 (L517 and special-id branches), 0x00777e30 (L59), ctor 0x007645f0 default 10000 (ledger). Default fallback 10000 at 0x007ed9a0 L216 and L498.
- **0x0077d390** (`__thiscall`, ret 8, callers 0x008fb130, 0x008f92b0, 0x00907360). Gate: holder `+0xC6 == 1` (L22). Timer: `+0x3A8 += FUN_0091c860()` vs threshold `+0x3AC` (L31-32). Fires when elapsed >= threshold. Picks `rand()&1` -> pause1 or pause2 (50/50). Picks `rand()%6` -> greeting, hturnl, hturnr, pausebrd, pausesh, pause3. Resets threshold to `(rand%0xC9)*100+10000` (L85), i.e. 10000-30000 units.
- **Caveat:** asm 0x0077d43a / 0x0077d449 / 0x0077d5b3 `LEA ECX,[EBP-0x20]; CALL 0x007338d0` writes the name into a stack CExoString. Nothing in the function consumes it. The chooser is confirmed, the play is not visible here.

## Q3. Does the driver re-issue idle after our play?

- 0x00777e30 L31-40: `if (FUN_007ef5d0(+0x48)==1) +0x64 -= +0x44; if +0x64 < 0` then `FUN_007ed980(0,0)` and `vt+0x9C(result)`. That re-issues the default idle.
- L41-59: queue pop `FUN_007efac0` replays a queued id and writes +0x48.
- A direct `Gob::PlayAnimation` touches neither +0x48 nor +0x64 nor the queue. So idle returns by the driver only if the timer is armed and FUN_007ef5d0(+0x48) is 1. If +0x64 <= dt, that is the next tick. FUN_007ef5d0 is UNKNOWN.
- 0x00769780 (per-tick blend, replays current id via holder vt+0x54) is not in the CSWCCreature vtable. Its caller is UNKNOWN, so per-tick re-issue is UNKNOWN.

## Q4. Cheapest forced idle re-issue

- Call `vt+0x9C` on the creature with `(10000, 0, 0)` (thiscall, ECX=creature, 3 stack args, retn 12). Writes +0x48 = 10000, sets +0xBA = 10000, skips FUN_00774b40 (only for id != 10000), and calls holder vt+0x54 for idle.
- Can no-op: (a) holder `vt+0x50(id)` == 0 (L106-112); (b) `creature+0x54 != 0` and app `+0xD8 != 0` (L105, else L529-531 return 0). Who clears `creature+0x54` is UNKNOWN; (c) gate L67-102 when the global manager is set (depends on FUN_0077d800 and vt+0x94, undecoded).
- Direct Gob call: `Gob::PlayAnimation` 0x004e3270, thiscall(name char*, float speed, uint flags, float start), retn 16 (decomp L5). Name "NULL" or empty clears the list (L32-50). Flag 0x80 = stop (fires "done%s" via vt+0x1b4). Flag 0x400 only sets entry flags |= 0x401. The only direct caller is 0x0045a190 (Gob "Dying" teardown, callgraph).

## Project note (not REA)

`inputs/lessons_recent.tsv` row 152 (live note, mod5-equip3d, no date): `PlayAnimation(name,-1.0,0,0)` once, then re-issue idle after len+40 ms works live. Consistent with the Q3 risk: the creature driver does not stop the flourish by itself in that test.

## Unknowns

1. Holder class and its vt+0x54 (name lookup, FUN_00863650 link).
2. Caller of 0x00769780 and whether it runs per tick.
3. FUN_007ef5d0 (gates the timer decrement), FUN_007ed980 (default id source), FUN_0091c860 (time unit).
4. Who clears creature `+0x54`.
5. Whether a direct Gob flourish's entry is removed when it ends, so the older idle entry resumes (Gob removal path not decoded).
6. Initial values of `+0x3A8`/`+0x3AC`.
