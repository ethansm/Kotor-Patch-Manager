# Target modes: Gate R (2026-10-03)

Raw evidence in this folder (decomp_/disasm_/callers_ files, Ghidra headless on KOTOR2_SteamAspyr, Sonnet analyst; Opus re-read the anchors marked [O]).
Targetable rule: TARGETABLE.md (with the Opus correction at the bottom).

## Answers
- **Q1 HudInvokeGroup 0x751750** (thiscall hud, g; RET 8): calls entry+0xC as thiscall(ECX = owner + entry[0x10], key = entry+8, creature = 0x7E5D80()) at 0x7519a5. No selected-target field is passed.
- **Q2 power cb 0x7791A0** (RET 8): target pushed = `[this+4]` (bar owner id) at 0x779368 / 0x779488 -> 0x775700. p2 only read at +0x310. Force-class casts first try targeting mode 0x740a20 (mode 4 if forcehostile set, else 3).
- **Q3 0x775700** (thiscall, ECX unused, RET 0x1C): (spell, targetObjId, float* pos, byte class, byte, byte, int). Builder 0x87A5C0, msg 6/0x12, target at +0x20.
  Vanilla "power on clicked object" = 0x899270: 0x775700(spell [this+0x9d0], [obj+4], &zero, class [this+0x9cc], 0, 0, 0) at 0x8992e9 [O].
- **Q4 items**: cb 0x77CC80 pushes `[this+4]` (0x77ccc3/0x77cce8). 0x775600 (thiscall, ECX unused, RET 0x10): (itemId, packed byte, targetObjId, float* pos) -> builder 0x879AF0 msg 6/9.
- **Q5 vanilla use-on-companion** [O]: 0x77C780, after a party-portrait pick (gui+0x15230 awaiting / +0x15234 slot / +0x15238 item):
  member = 0x7E5DA0(table, idx) (0x77ca99); server = 0x77D800(member) (0x77caa4); gate 0x5D2F80 thiscall(ECX = server; 0x51C0E0(itemId), 0, 1) (0x77cad8, JZ = skip);
  0x775600(itemId, 0, *(member+4), member+0x24) at 0x77cb26.
- Server: handler 0x65be70 case 8 (use item, 0x57B4C0) / case 0x11 (cast, 0x565310) act as the sending player's creature; object ids read `& 0x7fffffff` (0x632ef0).

## Route verdict (Opus)
- **Items: Route A' = replicate the vanilla 0x77C780 sequence** for the chosen member: CanUseItem gate on its server creature, then 0x775600 with its client id and position.
  Confidence high (vanilla code path). The gate also handles repair kits vs droids and other per-creature restrictions.
- **Powers: no route needed** in 1.0.0: no vanilla K2 power is single-target friendly (TARGETABLE.md correction), so the mode button is disabled for all powers.
- Route C (server AddUseTalentOnObjectActions 0x57D8B0) is not needed.
- Self index -> keep today's invokeEntry (HudInvokeGroup) unchanged.

## Needs the live probe (TargetProbe=1)
1. 0x775600 with a companion target while the Exile is controlled: companion healed, medpac consumed once, caster animation on the controlled character.
2. Distant / out-of-sight member: refused or walk-to? Dead/unconscious member (server 0x57B4C0 checks undecoded).
3. Controlled character != party slot 0 cases (after a character switch, member 0 = controlled).
