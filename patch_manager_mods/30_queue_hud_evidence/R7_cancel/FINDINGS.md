# R7 — vanilla "Cancel Combat" (BTN_CLEARALL) and the Cancel route for queue-hud

Static RE, Steam Aspyr swkotor2.exe, 2026-10-05 (Sonnet agent; written up by Opus because the agent's Write was refused).
Excerpts in `dump/`. Prologs: every cited function is 16-byte aligned with `55 8B EC`; RET n read from asm.
Opus spot-check: `query_ghidra.py address` — 0x51CB10 = CServerExoApp::ClearAllActions (verified B9),
0x7F24C0 = CSWCObject::ClearAllActions (B0), 0x541080 = CSWSObject::ClearAllActions (B0). 0x7523E0 has no DB row (HUD handler).

## 1. The control
- BTN_CLEARALL, GUI id 83, button, TEXT strref 109864 "Cancel Combat", red border (0.741, 0.110, 0). Source `ultrawide_gui/originals/mipc28x6_p.gui`.
- HUD ctor 0x747210: `FUN_0040f620(param_1+0x1b0f, ...)` → hud+0x6C3C (`00747d78 ADD EDX,0x6c3c`), i.e. LBL_QUEUE0 (hud+0x5F2C) + 0xD10.
- Siblings: BTN_CLEARONE hud+0x689C, BTN_CLEARONE2 hud+0x6A6C (transparent hit boxes over the queue icons).
- Visibility: 0x74E910 (thiscall, RET 4), called per frame from 0x74B3F0 with combat mode `[internal+0x320]` (0x740860); toggles `+0x48 bit1` on the queue labels and buttons (BTN_CLEARALL+0x48 = hud+0x6C84), only on a change of `[hud+0xA904] bit1`. Whole block hidden out of combat. 0x74DC80 does not touch these controls.

## 2. Click chain
- Ctor registers event 0x27 handlers via 0x418AF0 (CSWGuiControl::AddEvent, thiscall, RET 0xC), asm 0x748cf3..0x748d4b: CLEARONE/CLEARONE2 → 0x752330, CLEARALL → 0x7523B0.
- 0x7523B0 (thiscall ECX=hud, RET 4): tests `event+0x50`, calls 0x7523E0.
- 0x7523E0 (thiscall ECX=hud, no args): leader (party slot 0) via 0x73FB90 / 0x7E5D80; `0x77FD20(leader,0)` (clears `[cr+0x458]` bit0; if controlled, combat mode `[internal+0x320]=0` via 0x740880/0x79D6E0); `0x7F24C0(leader)`; `0x4122A0([hud+0x1c],0)` (UNVERIFIED); resets floats `[hud+0x595C]`, `[hud+0x70]`.
- 0x7F24C0 → 0x51C8B0(clientId) (strips client bits `~[0x99449C]`) → **0x51CB10(CServerExoApp, serverId)** → `0x541080(cr,1)` (`0051cb33`) + `0x590430(round)` (`0051cb47`); also sets `[round+0xB20]`, `[cr+0x548]`, `[cr+0x568]`, `[cr+0x56c]` = 0x7f000000.
- CLEARONE: 0x752330 → 0x752360 → 0x7511B0, pops one round node (0x5904C0), else falls back to `0x541080(cr,1)`.
- Keyboard/pad: HUD vtable slot 15 = 0x7469F0 (RET 8), case 0x28, calls 0x541080 directly.

## 3. Candidates
| addr | ABI | drops cr+0x100 | drops round +0xB00 | target |
|---|---|---|---|---|
| 0x7F24C0 | thiscall ECX=client cr, RET | yes | yes | client id → server id |
| 0x51CB10 | thiscall ECX=[[0xA1B4A4]+8], 1 arg server id, RET 4 | yes | yes | server object id |
| 0x541080 | thiscall ECX=server cr, 1 arg, RET 4 | yes | no | server creature |
| 0x7F2500 | thiscall ECX=client cr, RET (→0x51CBA0) | no | yes | not a Cancel |

- Doc 29 D6 holds for 0x541080 alone, not for 0x51CB10.
- 0x541080 no-ops unless vtable+0x10 gate passes and `[cr+0xEC]==1` (`005410e9`). Each node → CSWSCreature::ClearAction 0x56E620 (vtbl+0x74, RET 8); deleted only if it returns 1 (returns 0 when `node+0x70==0`, meaning UNVERIFIED).
- Pause/combat safety: no asserts, no pause check, no server message; vanilla CLEARONE/Esc call 0x541080 while paused (code reading only).
- **Companion hazard (UNVERIFIED):** ClearAction also deletes a follower's FollowLeader (0x3D) node. Only adds found: 0x669FC0 (script ActionFollowLeader), 0x7E5730 (CSWParty::AddCharacter). Live-check that a companion keeps following after Cancel.

## 4. Extent (800x600 design)
BTN_CLEARALL [317,570,166,24]; QUEUE0 [432,521,40,40]; QUEUE1 [378,531,20,20]; QUEUE2 [352,531,20,20]; QUEUE3 [326,531,20,20]; CLEARONE [320,526,100,30]; CLEARONE2 [422,515,60,52].
Style: dialogfont16x16, text (0.8,0.8,0.698), align 18, hover border (0.698,0.698,0.6).

## Recommended call per target
- **Leader and companion (uniform):** `CServerExoApp::ClearAllActions` 0x51CB10, thiscall, ECX = `*(void**)(*(char**)AppGlobal + 8)` (server app), stack arg = server object id `*(uint32_t*)(crServer+4)`, RET 4. This is the worker vanilla Cancel Combat reaches; it empties both cr+0x100 and the round list.
- Not 0x7523E0 for companions (hardcodes slot 0); its extra leader work (0x77FD20 combat-mode reset, HUD floats) is combat-only and skipped out of combat.
- Companion: live-verify FollowLeader survives (see hazard).
