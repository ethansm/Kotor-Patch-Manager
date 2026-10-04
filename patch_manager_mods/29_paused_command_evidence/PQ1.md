# PQ1: pause + command path (KOTOR II Steam swkotor2.exe 6a522e71), static only

SUMMARY (most mod-relevant first)
1. CONFIRMED: server pause is bit 2 of `byte [CServerExoAppInternal+0x10078]` (bit 1 = timestop). It does NOT skip UpdateState, AIUpdate or RunActions; it freezes the world timer (`+0x24 == 1`, so dt = 0 and event times stand still) and explicitly blocks only: the combat-round timer (0x571030), AIActionCombat 0x6D9500 (returns result 1 = "in progress", node goes back to the queue head), one branch of AIActionPhysicalAttacks and one of AIActionStanceTotalDefense. Use-item and cast-spell are both queued as a combat-round node plus an `AIActionCombat (0x3F)` action, so a paused command is accepted at once but cannot start until unpause.
2. CONFIRMED: client messages are NOT buffered by pause. Senders (use item 0x879AF0 = major 6/minor 9; cast 0x87A5C0 = minor 0x12) go to the net layer; the server tick runs `ProcessReceivedFrames` (0x703F30, `005338f8`) every tick with no pause test, handler 0x65BE70 runs at once. Queuing (AddSWItemSpellAction / AddSWSpellAction + `AddAction(0x3F)` at the TAIL, optional `ClearAllActions(1)` via ActionManager) happens during the pause; the real actions (move, 0x11/0x13 or 0x12/0x31, 0x2E/0x0F) are built later by AIActionCombat with `AddActionToFront`.
3. CONFIRMED (delay candidates after unpause): AIActionCombat still waits when `[round+0xAA8] != 0` (previous action's animation window) or `[round+0xAB8] >= 1` (one action per 3000 ms round, `0058e8da MOV [EAX+0xa9c],0xbb8`); both count only unpaused game time. A command issued right after another action can wait up to a round, with no on-screen sign. Exact user-visible delay is SPECULATIVE until probes P1-P4 run.
4. CONFIRMED (no indicator): the client queue panel (Steam 0x74DC80, called from HUD Update 0x74B3F0 at `0074c181`) is gated by bit 1 of `[CSWGuiMainInterface+0xA904]`, set every frame from `CClientExoApp::GetCombatMode()` (`[CClientExoAppInternal+0x320]`, 0x740860). Combat mode turns on only while a party member has a live creature in `[creature+0x550/+0x564/+0x568]` (0x79D840), so a medpac/buff out of combat on self or an ally leaves the panel hidden. The panel reads the SERVER creature queue (`+0x100`) of party slot 0 directly (same process), 4 slots, types 0x3F/0x0C/0x0F/0x2E only.
5. Polarity difference (CONFIRMED asm, effect SPECULATIVE): AddItemCastSpellActions calls ActionManager(4) (= `ClearAllActions(1)` for a player-controlled actor) when a round action IS pending (`00564983 JZ` skips), AddCastSpellActions when NOT pending (`005653cc JNZ` skips). ActionManager is a no-op unless the actor is the player's client object (0x51C600), so a non-controlled party member never has its queue cleared and the new 0x3F lands behind whatever is queued.

## (a) Function table (Steam addrs; "C" = confirmed against asm/decompile in this batch, "M" = from name_map / mapper TSV)
| addr | proposed name | conf | purpose | evidence |
|---|---|---|---|---|
| 0x533830 | CServerExoAppInternal::MainLoop (fastcall, ECX=this) | high M+C | server tick | `0053396e CALL 0x0051d790` |
| 0x51C6B0 / 0x538C20 | CServerExoApp(+Internal)::TogglePauseState(mask) | high C | flip bit | `00538c34 NEG EAX` / `00538c44 CALL 0x00538ed0` |
| 0x51C720 / 0x538CD0 | ...::GetPauseState(mask) | high C | (byte[+0x10078] & mask) > 0 | `00538cda MOVZX ECX,byte ptr [EAX + 0x10078]` / `00538ce5 AND ECX,EDX` |
| 0x51C740 / 0x538DC0 | ...::GetActivePauseState | high C | 2 if bit2, else 1 if bit1, else 0 | `00538dc7 PUSH 0x2` ... `00538dd5 MOV AL,0x2` |
| 0x51C760 / 0x538ED0 | ...::SetPauseState(mask,value) (ret 8) | high C | set/clear bit, pause world timer, notify client | `00538f27 MOV byte ptr [EAX + 0x10078],DL`, `00538f3f CALL 0x0051b050` |
| 0x51B050 / 0x51B0A0 | CWorldTimer::PauseWorldTimer / UnpauseWorldTimer | high M+C | flag `+0x24`, saved day/time `+0x28/+0x2C` | `0051b087 MOV dword ptr [ECX + 0x24],0x1` |
| 0x51AD20 | CWorldTimer::GetWorldTime(&day,&time) | high M+C | frozen value when paused | `0051ad3a CMP dword ptr [EAX + 0x24],0x1` / `0051ad46 MOV EAX,dword ptr [EDX + 0x28]` |
| 0x51C350 / 0x538E00 | CServerExoApp(+Internal)::GetActiveTimer(objId) | high M+C | per-object timer choice | `00538e09 CMP dword ptr [EBP + 0x8],0x7f000000`, `00538e59 MOV EAX,[EDX+0x10050]`, `00538e6c MOV EDX,[ECX+0x10048]` |
| 0x51C790 / 0x5393B0, 0x5395A0, 0x51C7F0 | AddToExclusionList / IsOnActiveExclusionList / GetActiveExclusionList | high M | per-mask lists (+0x10070 bit2, +0x10074 bit1) | sole caller of 0x51C790 is OnApplyTimestop `005a4db1` (mask 1, `005a4daa PUSH 1`) |
| 0x637350 | server->client pause sync | med C | calls 0x73F6A0 | `00637445 CALL 0x0073f6a0`; callers `00538f7a`, `006606cf` |
| 0x73F6A0 / 0x784740 | CClientExoApp(+Internal)::SetPauseState (ret 12) | high M+C | client mirror, byte `[+0x1CC]` | `0073f6ba CALL 0x00784740`; c: `*(byte*)(this+0x1cc) \|= param_2` |
| 0x73F680 / 0x784420, 0x73FE00 / 0x784440 | client GetPauseState / GetActivePauseState | high M+C | | `0078442a MOVZX EAX,byte ptr [EAX + 0x1cc]` |
| 0x740350 / 0x79BF40, 0x79E420, 0x79BF00 | client SetPausedByCombat / SetAutoPaused / helper | med C | UI pause REQUEST: stores `[+0x380]`, `[+0x37C]\|=4`; 0x79BF00 = server GetPauseState(2) | `0079bf2b CALL 0x0051c720`; MainLoop `00783c56 CALL 0x0079bf00`, `00783c67 CMP ECX,[EAX+0x380]`, `00783c87 CALL 0x0087a730` |
| 0x87A730 | client TogglePauseRequest (GOG 0x506010) | med C | direct server call, not a message | `0087a756 PUSH 0x2` / `0087a75b CALL 0x0051c6b0` |
| 0x40BC40 | AppDeactivated | high M+C | SetPauseState(2,1) on focus loss | `0040bcc4 CALL 0x0051c760` |
| 0x569320 | CSWSCreature::AIUpdate | high | per-creature tick | `005695b1 CALL 0x005d56b0` |
| 0x571030 | CSWSCreature::UpdateCombatRoundTimer | high M+C | round timers, pause-blocked | `00571064 PUSH 0x2` / `00571069 CALL 0x0051c720` / `00571070 JNZ 0x005713e6` |
| 0x5D56B0 | CSWObject::RunActions | high (AI4) | no pause awareness | not among callers of 0x51C720/0x51C740/0x538DC0 (cg) |
| 0x6D9500 | CSWSCreature::AIActionCombat (type 0x3F, ret 4) | high M+C | converts the round node into real actions | `006d9584 CALL 0x0051c720` / `006d958b JNZ 0x006da49a` / `006da49a MOV EAX,[0x009944b4]` |
| 0x6D7390, 0x5D29F0 | AIActionPhysicalAttacks, AIActionStanceTotalDefense | high M | one pause-gated branch each | `006d8c35`, `005d2b49 CALL 0x0051c720` |
| 0x65F8A0 / 0x5314E0 / 0x51C420 | HandlePlayerToServerMessage / Internal::HandleMessage / ServerExoApp::HandleMessage | med | router | `00531580 CALL 0x0065f8a0`; 0x51C420 has no direct caller (net layer vtable call `00704036 MOV EAX,[EDX+4]` / `00704039 CALL EAX`) |
| 0x65BE70 | CSWSMessage::HandlePlayerToServerInputMessage (GOG 0x801C10), ret 8 | high C | switch on minor opcode | `0065bf3c CMP [EBP+0xfffffc20],0x26` / `0065bf4f JMP dword ptr [ECX*0x4 + 0x65e59c]` |
| 0x703F30 | CNetLayer::ProcessReceivedFrames | med M+C | drains frames in the tick | `005338f8 CALL 0x00703f30` (also `0053400c`) |
| 0x87A350 | CSWCMessage::SendPlayerToServerMessage(major,minor,buf,len) | med C | header bytes 0x70,major,minor, then 0x704060 | c: `*local_10 = 0x70; local_10[1] = param_2; local_10[2] = param_3` |
| 0x879AF0 | SendPlayerToServerInput_UseItem (ret 20) | high C | major 6, minor 9 | `00879c38 CALL 0x0087a350` with `PUSH 9`/`PUSH 6` before it |
| 0x87A5C0 | SendPlayerToServerInput_CastSpell (ret 28) | high C | major 6, minor 0x12 | `0087a71c PUSH 0x12` / `0087a71e PUSH 0x6` |
| 0x775600 / 0x775700 | CSWCCreature::PerformUseItem / PerformCastSpell (ret 0x10 / 0x1C) | high C | thin wrappers, no pause test | `00775658 CALL 0x00879af0`, `00775747 CALL 0x0087a5c0` |
| 0x7791A0 / 0x7794E0 | CSWCCreature::ForceMenuAction / CSWGuiInterfaceAction_UseForcePower | med | force-power entry (radial/hotbar) | `00779368`/`00779488 CALL 0x00775700`, `007794c8 CALL 0x00879af0` |
| 0x77C780 / 0x77CC80 | PartyMenuUseItemSelect / PartyMenuUseItem | med | party-menu item use | `0077c9a2`, `0077ccc3 CALL 0x00775600` |
| 0x57B4C0 | CSWSCreature::UseItem (ret 28) | high M+C | server item use | `0065c5ff CALL 0x0057b4c0` |
| 0x5648D0 | CSWSCreature::AddItemCastSpellActions (ret 44) | high M+C | item cast queuing | callees: `00564944 CALL 0x00591890`, `005649ed CALL 0x0053f7f0` |
| 0x565310 | CSWSCreature::AddCastSpellActions (ret 72) | high M+C | spell queuing | `0065d5ce CALL 0x00565310` |
| 0x57D8B0 | CSWSCreature::AddUseTalentOnObjectActions (ret 32) | high M+C | opcode 0x23 + script ActionUseTalent | `0065d8d9 CALL 0x0057d8b0`, `0057da25 CALL 0x00565310` |
| 0x591670 / 0x591890 | CSWSCombatRound::AddSWSpellAction / AddSWItemSpellAction | high M+C | round node types 9 / 0xA (0x88 B) | c: `*(local_30+0x10) = 9` / `= 10` |
| 0x590110 / 0x590220 / 0x590270 | CSWSCombatRound::GetActionPending / GetNextDueAction / InsertAction | med C | head node time `[node]` <= round timer `[+0xA94]` | `0059015b MOV EAX,[ECX+0xa94]` / `00590161 CMP EAX,[EDX]` |
| 0x563C90 | CSWSCreature::ActionManager(mode) | high M+C | mode 1/2: ClearAllActions(1) if category `< 10`; mode 4/8: always; only if `0x51C600(this+4) != 0` | c: `FUN_00541080(1)` |
| 0x5718A0 / 0x5719A0 / 0x571980 | recompute / set / test `[+0x1116]` action category | med C | 0x571980 returns `param < 10` | c |
| 0x51C600 / 0x52F330 | CServerExoApp(+Internal)::GetClientObjectByObjectId | high M+C | walks `[this+0x10064]`, matches `[obj+4]` | `0052f350 MOV ECX,[EAX+0x10064]`, `0052f3a2 CMP EDX,[EBP+8]` |
| 0x58E730 / 0x58E990 | StartCombatRound / EndCombatRound | high M+C | `+0xA9C=3000`, `+0xA84=1` / reset `+0xAB8=0` | `0058e8da`, `0058ea53` |
| 0x588ED0 | CSWSCreature::UpdateActionQueue (GOG 0x6A19B0) | high M+C | builds 10 x 0x1C entries at `+0x3F0` from queue groups | callers `00569f0a`, `0059830f` |
| 0x636A90 / 0x636B80 | CSWSMessage::UpdateLastUpdateActionQueue / ComputeLastUpdate_ActionQueue | med M | net update of the +0x3F0 array (0x1C stride) | `00636ac4 IMUL EDX,EDX,0x1c`; callers 0x6415E0 / 0x63EDC0 |
| 0x74B3F0 | CSWGuiMainInterface::Update(dt) (vtable, 3874 B; GOG 0x51DAA0) | med C | HUD per frame; hooked by action_hotbar (`0x74B3F0`) | `0074c181 CALL 0x0074dc80` |
| 0x74DC80 | CSWGuiMainInterface::UpdateActionQueue_2 (GOG 0x51F400, ret 0) | high C | the client queue display | `0074dccc CALL 0x007e5d80`, `0074dce3 CALL 0x0077d800`; loop `local_34 < 4` |
| 0x74E910 | CSWGuiMainInterface::ShowActionQueue? (GOG 0x523510, ret 4) | med C | sets bit1 of `[+0xA904]` and of 4 slot widgets `[+0x5F2C+i*0x148+0x48]` | `0074e950 MOV [EDX+0xa904],ECX` |
| 0x740860 / 0x740880 / 0x79D6E0 / 0x79D840 / 0x77FD20 | CClientExoApp::GetCombatMode / SetCombatMode / Internal::SetCombatMode / UpdateCombatMode / CSWCCreature::SetCombatMode | high M+C | `[+0x320]` | `0074086f MOV EDX,[ECX+0x320]`, `0079d710 MOV [EAX+0x320],ECX` |
| 0x77D800 | party entry -> server CSWSCreature | med | `0x7F2540` result, `vtbl+0x30` | `0077d828 MOV EAX,[EDX+0x30]` |

Mapper contradictions (both quoted): TSVs `B11b_map_CSWGuiMainInterface` / `U4_map_CSWGuiMainInterface` map GOG UpdateActionQueue 0x51E730 -> Steam 0x74CFE0 (`drift+0x30`) and ShowActionQueue 0x523510 -> 0x751D90 (`vote`). REFUTED by body: 0x74CFE0 is named UpdateTargetMenu in name_map (mods678-batch3-dump, call site `0074b545` in the HUD Update), and 0x751D90 (167 B) only calls ShowTutorialWindow (`00751e17 CALL 0x00740a20`). Use 0x74DC80 (queue loop) and 0x74E910 (show flag).

## (b) Findings

### Q1 PAUSE GATE
**CONFIRMED: the flag.** `00538cda MOVZX ECX,byte ptr [EAX + 0x10078]` (GetPauseState 0x538CD0); set at `00538f1f OR EDX,EAX` / `00538f27 MOV byte ptr [EAX + 0x10078],DL`. Bit 1 = timestop (OnApplyTimestop `005a4dc2 PUSH 0x1` / `005a4dc7 CALL 0x0051c6b0`); bit 2 = player pause (client request `0087a756 PUSH 2 / 0087a75b CALL 0x0051c6b0`; AppDeactivated `0040bcc4`; message opcode 0x19 `0065dff4 PUSH 2` / `0065dffc CALL 0x0051c760`). Client mirror: `byte [CClientExoAppInternal+0x1CC]`, written by 0x784740, set by the server via 0x637350. SERVER: 0x51C6B0/720/740/760 (+Internal 0x538C20/CD0/DC0/ED0). CLIENT: 0x73F680/6A0/FE00 (+Internal 0x784420/740/440). SetPausedByCombat 0x740350/0x79BF40 and SetAutoPaused 0x79E420 are client UI request flags, not the server pause: MainLoop compares requested vs `GetPauseState(2)` and calls 0x87A730 (single-player direct toggle).

**CONFIRMED: UpdateState / AIUpdate / RunActions are not skipped.** Guards on the call: `00533947 CMP [EBP+0xffffff34],0x0` / `0053394e JNZ 0x0053399b` (`**([0xa1b4a4]+0x14) == 0`: a pending app request record is absent; the action_hotbar hook comment says `*(*(0xA1B4A4)+0x14)+0x18` is the save folder, so this is a save/load request record, SPECULATIVE) and `0053395d CMP EDX,0x2` / `00533960 JNZ 0x0053399b` (`word [this+0x10008] == 2`, module running state, SPECULATIVE); then `00533968 MOV ECX,[EAX+0x10044]` / `0053396e CALL 0x0051d790`. UpdateState, AIUpdate and RunActions contain no pause test; cg callers of GetPauseState/GetActivePauseState (0x51C720/0x51C740/0x538DC0) are only: AppDeactivated, 0x571030, 0x5D29F0, 0x660600, 0x6D7390, 0x6D9500, 0x79BF00, 0x892870, AddEventDeltaTime 0x51E170 (result discarded), module AIUpdate 0x557260 (timestop test `005573c0 CMP ECX,1`), OnApplyTimestop.

**CONFIRMED: world timer frozen, dt = 0.** `0051ad3a CMP dword ptr [EAX + 0x24],0x1` / `0051ad3e JNZ 0x0051ad58` / `0051ad46 MOV EAX,dword ptr [EDX + 0x28]`; flag set by `0051b087 MOV dword ptr [ECX + 0x24],0x1` (PauseWorldTimer on `[this+0x10048]`, `00538f3f`). GetActiveTimer 0x538E00: object INVALID or active state 0 -> `[+0x10048]`; state 2 and object on the mask-2 exclusion list -> `[+0x10050]` (pause timer, never paused); state 2 otherwise -> `[+0x10048]` (frozen); state 1 likewise with `[+0x1004C]`. The engine never fills the mask-2 list (only AddToExclusionList caller passes mask 1), so all creatures are frozen under player pause. AIUpdate: `00569525 CALL 0x0051c350` (GetActiveTimer(this+4)), `0056952c CALL 0x0051ad20`, `0056955e CALL 0x0051af60` (SubtractWorldTimes), `0056956c MOV [ECX+0xD0],EDX` (dt = 0 while paused), `005695b1 CALL RunActions` with the frozen (day,time). AI events use the same frozen timer (`0051e191 MOV EDX,[ECX+0x10048]` in AddEventDeltaTime; `0051ed40` in EventPending), so delta events due "now+X" do not fire while paused. The tick refreshes timers by state: `00533898 CALL 0x00538dc0` / `005338a2 JNZ 0x005338b5` / `005338b0 CALL 0x0051a860` (world timer only when state 0), `005338bb..005338d4` (timestop timer unless state 2), `005338df..005338e5` (pause timer always).

**CONFIRMED: explicit bit-2 blocks.**
- UpdateCombatRoundTimer: `00571069 CALL 0x0051c720` (arg 2) / `00571070 JNZ 0x005713e6` returns before touching the round timers.
- AIActionCombat: `006d957c PUSH 0x2`, `006d9584 CALL 0x0051c720`, `006d958b JNZ 0x006da49a`, `006da49a MOV EAX,[0x009944b4]` (value 1 = in progress, read from .rdata). RunActions requeues result 1 at the head and ends the loop (AI4 key behaviour 10).
- AIActionPhysicalAttacks `006d8c35`/`006d8c3c JNZ 0x006d8c6b`, AIActionStanceTotalDefense `005d2b49`/`005d2b50 JNZ 0x005d2b73`: pause-gated sub-branches (bodies not read, SPECULATIVE what they skip).
- Not pause-gated: AIActionCastSpell 0x5B96F0, AIActionItemCastSpell 0x5B8000 and move actions; they just see frozen time or sit behind the head 0x3F.

### Q2 COMMAND PATH WHILE PAUSED
**CONFIRMED: no message buffering.** `0x879AF0` / `0x87A5C0` -> `0x87A350` (`*local_10 = 0x70`, then `0x704060` ring-buffer writer). Server tick `005338f8 CALL 0x00703f30` runs before UpdateState and has no pause test; it dispatches each frame through `[netlayer+0x20008]` vtable slot 1 (`00704036/00704039`, exact target 0x51C420 SPECULATIVE) -> 0x5314E0 -> `00531580 CALL 0x0065f8a0` -> 0x65BE70. Latency is at most one frame (client vs server MainLoop order not determined, SPECULATIVE).

**Opcode map (CONFIRMED, jump table at 0x65E59C read from the exe, index = minor-1):** 9 -> 0x65C45F UseItem; 0x12 -> 0x65D37F CastSpell; 0x23 -> 0x65D74E UseTalent on object; 0x18 -> 0x65DEF7 TogglePauseState(2); 0x19 -> 0x65DF7F SetPauseState(2, bool from message). No Steam client sender for minor 0x23/0x18/0x19 is among the 28 callers of 0x87A350 (0x23 is reached by script via 0x66BDC0; pause uses the direct call 0x87A730), SPECULATIVE whether 0x18/0x19 are dead in single player.

**(a) USE ITEM, CONFIRMED chain.** Party menu `PartyMenuUseItem 0x77CC80` / hotbar -> `PerformUseItem 0x775600` -> message (6, 9) -> handler `0x65C45F` reads item id, prop index, subprop, target obj, optional float x3 position (`0065c51d CALL 0x005e3b60`), then:
1. `0065c5c1 CMP [..+0x520]` / `0065c5c8 JNZ 0x0065c5d4` / `0065c5ca PUSH 1` / `0065c5cf CALL 0x00563c90`: actor NOT in combat -> ActionManager(1) = `ClearAllActions(1)` if category `[+0x1116] < 10` AND actor is the player's client object.
2. `0065c5ff CALL 0x0057b4c0` UseItem(item, prop, sub, target, &pos, area, 1). Item property type 10 (cast spell) -> `FUN_005648d0(item,prop,sub,x,y,z,target,area,target==INVALID,front=local_a0,real=0)`. UseItem returning 0 -> `0065c60f CALL 0x00584da0` with `PUSH 0x17` (failure feedback).
3. AddItemCastSpellActions round path (p12 = real flag = 0): `00564944 CALL 0x00591890` (round node type 0xA, time 0), `0056497c CALL 0x00590110` / `00564983 JZ 0x00564992` / `00564985 PUSH 0x4` / `0056498d CALL 0x00563c90` (ActionManager(4) when a round action IS pending and `[+0x520]==0`), `00564992 PUSH 0x3f` / `0056499a CALL 0x0053f560` (is a 0x3F queued?) / `005649ed CALL 0x0053f7f0` = AddAction TAIL of type 0x3F. Returns 1. Nothing else is queued at this point (no move, no 0x2E).
4. Later, unpaused only: AIActionCombat pops the round node (type 0xA) and calls `005648d0(item,prop,sub,pos,target,[node+0x84],[node+0x48], front=1, real=1)`: AddActionToFront (0x540CA0) in the order 0x2E ItemCastSpell, then (object target) 0x13 ChangeFacingObject, 0x11 CheckMoveToObjectRadius or (location) 0x31, 0x12, then if distance^2 > range^2 `AddMoveToPointActionToFront 0x5644B0` (type 1). Final head->tail: `1, 0x11|0x12, 0x13|0x31, 0x2E, <existing>, 0x3F` (0x3F is re-added at the tail because the handler returns 4 = `[0x9944c0]`). Type 1 is a stop-loop type (AI4), so the following action runs one AIUpdate later.

**(b) CAST FORCE POWER, CONFIRMED chain.** Radial/hotbar -> `0x7794E0` -> `ForceMenuAction 0x7791A0` -> `PerformCastSpell 0x775700` -> message (6, 0x12) -> handler `0x65D37F`:
1. `0065d533 CALL 0x006b8f40` (spell uses/FP) / `0065d53d JLE 0x0065d749` aborts silently when none left.
2. `0065d552 CMP [EBP+0xfffffd08],0x1` (`[creature+0x520]`) / `0065d559 JZ` / `0065d55b PUSH 1` / `0065d560 CALL 0x00563c90`: ActionManager(1) when not in combat.
3. `0065d565 PUSH 0x0` ... `0065d5ce CALL 0x00565310` (p19 = 0, round path). `0056538d CALL 0x00591670` (round node type 9, `[node+8]=500`), `005653c5 CALL 0x00590110` / `005653cc JNZ 0x005653db` / `005653ce PUSH 0x4` / `005653d6 CALL 0x00563c90` (ActionManager(4) only when NO round action is pending: opposite polarity to the item path), `005653db PUSH 0x3f` / `005653e3 CALL 0x0053f560`, `00565436 CALL 0x0053f7f0` = AddAction(0x3F) at the TAIL. Returns 1.
4. Unpaused execution: AIActionCombat case 9 -> `00565310(..., front p13 = 1, ..., p19 = 1)` with the same AddActionToFront order and 0x0F CastSpell instead of 0x2E (c lines 321-365 of 0x565310): head->tail `1, 0x11|0x12, 0x13|0x31, 0x0F`, then 0x3F again. The tail variant (p13 == 0, p19 == 1; callers 0x66E7E0, 0x670D10, 0x57DA90) appends `1, 0x12|0x11, 0x31|0x13, 0x0F` with AddAction (script ActionCastSpell).
`AddUseTalentOnObjectActions 0x57D8B0` (opcode 0x23 / script) calls `0057b4c0` for item-bound powers (`0057d98f`, `0057d9da`) or `00565310(real=0)` (`0057da25`): the same round path.

**CONFIRMED: when the old queue is wiped.** `0x563C90`: if `0x51C600(this+4) != 0` { `0x5718A0()`; mode 1/2 and category < 10 -> `ClearAllActions(1)`; mode 4/8 -> `ClearAllActions(1)` }. `0x51C600` -> `0x52F330` walks `[this+0x10064]` comparing `[obj+4]` with the id (`0052f3a2`): non-null only for the player's controlled creature (exact list meaning SPECULATIVE). A non-controlled party member keeps its queue and the new 0x3F goes behind it. A queue holding pickup/drop/equip/unequip/cast-type actions has category >= 10 (0x5719A0 maps types 7, 9, 8, 0xB, 0xF/0x2E, 0x14, 0x15, 0x19.. to categories 0xA..), so ActionManager(1) leaves it alone and the new command waits in line.

**CONFIRMED: AIActionCombat result codes and gates** (.rdata: `[0x9944b4]=1, [0x9944b8]=2, [0x9944bc]=3, [0x9944c0]=4`). Paused, or `[round+0xAA8] != 0` (`006d9554`/`006d9567 JNZ 0x006da49a`) -> 1 (requeue at head, loop ends). Node not due, or `[round+0xAB8] >= 1` (`006d96c0 MOV ECX,[EAX+0xab8]` / `006d96cc CMP [EBP+0xffffff04],0x1` / `006d96d3 JGE 0x006da425`) -> `006da45a..006da49f`: returns 1 if `[round+0xA84] != 0` (round started) else 2 (node dropped). `[round+0xAB8]` is incremented by AIActionCastSpell (`005bb06d ADD ECX,0x1` / `005bb076 MOV [EDX+0xab8],ECX`), AIActionItemCastSpell (`005b9447`), AIActionPhysicalAttacks (`006d90b0`) and reset by EndCombatRound (`0058ea53`); StartCombatRound sets `[+0xA9C]=3000`, `[+0xA94]=0`, `[+0xA84]=1`. So one cast/item/attack per 3000 ms round. SPECULATIVE: the user-visible post-unpause delay equals the remaining round time or the previous action's `SetRoundPaused/SetPauseTimer` window (`005ba032`, `005ba056`); needs P1-P4.

**Order of what is queued DURING the pause (item or power):** `ClearAllActions(1)` (conditional, controlled actor only), then exactly one `AddAction(0x3F)` (tail, only if no 0x3F is queued) plus one combat-round node (type 9 or 0xA, time 0). Moves (type 1, 0x03/0x0A/0x11 variants) and 0x0F/0x2E appear only after unpause.

### Q3 CLIENT ACTION-QUEUE DISPLAY
**CONFIRMED: Steam addresses.** Display `UpdateActionQueue_2` = 0x74DC80 (GOG 0x51F400); show/hide flag setter 0x74E910 (GOG 0x523510 ShowActionQueue, med); GOG client UpdateActionQueue 0x51E730 has no confirmed Steam twin (mapper row refuted above); server `CSWSCreature::UpdateActionQueue` 0x588ED0 (GOG 0x6A19B0); `UpdateLastUpdateActionQueue` 0x636A90 and `ComputeLastUpdate_ActionQueue` 0x636B80 (med, 0x1C stride).

**CONFIRMED: display gate.** HUD Update 0x74B3F0: `0074b424 MOV ECX,[EAX+0x48]` / `0074b427 SHR ECX,6` / `0074b42a AND ECX,1` / `0074b42d JZ 0x0074c303` (HUD visible bit); leader id != INVALID (`0074b48a CMP [EBP-0x18],0x7f000000`); then `0074c11b CALL 0x00740860` (GetCombatMode) / `0074c120 PUSH EAX` / `0074c127 CALL 0x0074e910` (sets bit 1 of `[this+0xA904]`), and `0074c16e MOV EDX,[ECX+0xa904]` / `0074c174 SHR EDX,1` / `0074c179 JZ 0x0074c186` / `0074c181 CALL 0x0074dc80`. No pause test exists among the HUD Update callees, so the panel keeps updating while paused but stays hidden unless combat mode is on.
**CONFIRMED: what 0x74DC80 draws.** Party slot 0 only (`0074dccc CALL 0x007e5d80` = idx-0 entry, `0074dce3 CALL 0x0077d800` -> server creature), reading the SERVER object directly: queue `local_84 + 0x100`, round `[creature+0x10DC]` (`0074dcee`), round started `[+0xA84]` (`0074dd01`), `0x590880` GetCurrentAction; at most 4 slots (`local_34 < 4`) at `[this+0x5F2C+i*0x148]`; draws types 0x0C, 0x0F, 0x2E (and 0x3F only when `[+0xA84] != 0` and a current action exists), and for a queued 0x3F with a non-null round list (`[round+0xB00]`) one icon per pending round node (`FUN_0074e0b0`/`FUN_0074e7f0`). A paused command (queue = `0x3F` + round node) WOULD be drawn if the panel were visible.
**CONFIRMED mechanism, SPECULATIVE trigger set: what makes it visible.** `[CClientExoAppInternal+0x320]` combat mode is written only by 0x79D6E0 (`0079d710`), the ctor `00780850` and UnloadModule `007855ff`. 0x79D840 UpdateCombatMode (per frame from client MainLoop, `007834d2`) calls `0x77FD20(1)` for a party entry with `[entry+0x458]&1 == 0` when `[creature+0x550]` or `[+0x568]` names a valid object whose `vtbl+0x94()==0` (`0079d87f` label), and `0x77FD20(0)` at `0079dc56` otherwise (third target `+0x564`). Meaning of `vtbl+0x94` (likely "is dead") and of the `+0x458` bit is SPECULATIVE. Consequence: a medpac or buff on self (or an ally while no hostile is targeted) does not enable combat mode, so no indicator.
**CONFIRMED: server display array.** `0x588ED0` fills 10 entries (stride 0x1C: `+0x3F0+i*0x1C` spell id, `+0x3F4..+0x3FC` target position, `+0x400` target object) from queue action groups. Periodic caller is AIUpdate: `00569e97 MOV ECX,[EBP+0xfffffe6c]` / `00569e9d MOV EDX,[ECX+0x10e8]` / `00569ea9 CMP [EBP+0xfffffe98],0x1` / `00569eb0 JNZ 0x00569f2d` (only the creature with the "IsPC" byte, which stays on the Exile per the memory note), `00569ebd CALL 0x0051c3b0` (GetPauseTimer, never paused), `00569efb CMP [EBP-0x78],0x12c`, `00569f0a CALL 0x00588ed0`: every 300 ms real time, also while paused. The array feeds the network update (0x636A90 / 0x63C2F0), not the in-process HUD.

## (c) Runtime probes
| id | counter / log | hook | expected signal |
|---|---|---|---|
| P1 | per creature: AIActionCombat calls blocked by pause; tick of the first unblocked pass after unpause | H4 `0x6D957C` (node `[EBP+8]`, creature `[EBP+0xFFFFFE54]`) | blocks >0 during a queued paused command; first unblocked pass within 1 tick of unpause |
| P2 | at unpause: `[round+0xAA8]`, `[round+0xAB8]`, `[round+0xA94]`, `[round+0xA9C]`, round node count, creature `+0x520` | H5 `0x538ED0` (mask 2, value 0), deferred read; or H4 | tells round gating (AB8 >= 1 / AA8 != 0) from conversion lag |
| P3 | item/spell id, target, `[this+0x520]`, queue length, head type at message time | H1 `0x57B4C0`, H2 `0x565310` (p19 at `[esp+0x48]`), H3 `0x5648D0` | round path (last arg 0) during pause, real path (1) after unpause; ActionManager(4) polarity |
| P4 | timestamp unpause -> first `AddCastSpellActions(p19=1)` / `AddItemCastSpellActions(p12=1)` -> first type 0x0F/0x2E reaching 0x5B96F0/0x5B8000 | H2/H3 + AI4-H08 `0x5D5C22` | measures the visible delay in ms |
| P5 | `GetCombatMode` and `[this+0xA904]` per frame while a command is queued out of combat | H8 `0x74C168` | 0 for self/ally targets (panel hidden) |
| P6 | RunActions result histogram for type 0x3F while paused | AI4-H09 `0x5D6A60` | all 1 during pause |
| P7 | is the actor the player's client object (ActionManager reach) | `0x563C90` entry, log `0x51C600` result | 0 for non-leader actors |

## (d) Open questions
1. Client vs server MainLoop order inside one frame (0 or 1 frame message latency).
2. Exact meaning of `**([0xa1b4a4]+0x14)` (save/load request record?) and `word [server+0x10008]==2`.
3. `vtbl+0x94` and `[entry+0x458]&1` in UpdateCombatMode; which party-menu cases turn combat mode on.
4. Actor of `PartyMenuUseItem` (leader or the menu's member); whether `GetClientObjectByObjectId` means "controlled creature" in every case.
5. Bodies of the pause-gated branches in AIActionPhysicalAttacks `006d8c35` and StanceTotalDefense `005d2b49`; `0x660600` and `0x892870` (other 0x51C720 callers).
6. Senders of minor opcodes 0x18/0x19/0x23 (dead in Steam single player?).
7. `0x590880` exact condition behind `local_4c` in 0x74DC80; icon builders `0x74E0B0`/`0x74E7F0` unread.
8. UseItem side effect for party members (`*local_a4 = 0xb; FUN_005e1bd0(0,0xfa1)` + AddEventDeltaTime), purpose unknown.
9. Interaction with the shipped action_hotbar mod, which already hooks HUD Update `0x74B3F0` (live window `0x74B3F0`, 5 bytes).

## (e) Struct field table
| class | offset | size | type | proposed name | readers (addr) | writers (addr) | confidence | evidence |
|---|---|---|---|---|---|---|---|---|
| CServerExoAppInternal | 0x10078 | 1 | byte mask | pauseMask (bit1 timestop, bit2 player pause) | 0x538CD0 | 0x538ED0 (`00538f27`, clear `00538f9f`) | high | `00538cda MOVZX ECX,byte ptr [EAX + 0x10078]` |
| CServerExoAppInternal | 0x10048 / 0x1004C / 0x10050 | 4 | CWorldTimer* | worldTimer / timestopTimer / pauseTimer | 0x51C370/390/3B0, 0x538E00 | ctor | high | `0051c37f`, `0051c39f`, `0051c3bf` |
| CServerExoAppInternal | 0x10070 / 0x10074 | 4 | list | exclusion list mask 2 / mask 1 | 0x538D00, 0x539600 | 0x5393B0 | med | `00539600` |
| CServerExoAppInternal | 0x10044 | 4 | ptr | CServerAIMaster | 0x533830 | | high | `00533968` |
| CServerExoAppInternal | 0x10008 | 2 | short | state (2 = run) | 0x533830 | | low | `00533956` |
| CServerExoAppInternal | 0x10064 | 4 | list | player / client object list | 0x52F330 | | med | `0052f350` |
| CWorldTimer | 0x24 / 0x28 / 0x2C | 4 | int | paused flag / saved day / saved time | 0x51AD20, 0x51B0A0 | 0x51B050 | high | `0051b087`, `0051ad46` |
| CClientExoAppInternal | 0x1CC | 1 | byte mask | client pauseMask | 0x784420 | 0x784740 | high | `0078442a` |
| CClientExoAppInternal | 0x320 | 4 | int | combatMode | 0x740860 | 0x79D6E0, ctor `00780850`, unload `007855ff` | high | `0074086f`, `0079d710` |
| CClientExoAppInternal | 0x37C / 0x380 | 4 | flags / int | UI pause request pending (bit 2) / requested state | MainLoop `00783c67`, `00783cba` | 0x79BF40 (`0079bfdf`) | med | |
| CSWGuiMainInterface | 0xA904 | 4 | flags | bit1 = action queue / combat HUD visible | 0x74B3F0 `0074c16e` | 0x74E910 `0074e950`, 0x747210 `007489e5` | high | |
| CSWGuiMainInterface | 0x5F2C + i*0x148 | 0x148 | widget[4] | action queue slot widgets (flags at +0x48) | 0x74DC80 | 0x74E910, 0x74E7F0 | med | tail loop of 0x74DC80 |
| CSWSCreature | 0x10DC | 4 | ptr | combat round | many | | high | `0074dcee MOV EDX,[ECX+0x10dc]` |
| CSWSCreature | 0x520 | 4 | int | inCombat | `0065c5b5`, `0065d543` | | high (AI4) | |
| CSWSCreature | 0x550 / 0x564 / 0x568 | 4 | objid | combat target / third target / spell target | 0x79D840 | 0x565310 (`[+0x568]=INVALID` at start), 0x591670 / 0x591890 (`[round+0xB04]->+0x568` when INVALID) | med | `0079d87f` block |
| CSWSCreature | 0x1116 / 0x1118 | 1 / 4 | byte / int | action category (>= 10 protects the queue) / paired value | 0x563C90 | 0x5718A0, 0x5719A0 | med | |
| CSWSCreature | 0x10E8 | 4 | int | IsPC byte (Exile), gates UpdateActionQueue | `00569e9d` | | med | memory note + `00569ea9 CMP ..,1` |
| CSWSCreature | 0x3E0 / 0x3E4 | 4 | int | last UpdateActionQueue day/time | `00569edd` | `00569f18`, `00569f24` | med | |
| CSWSCreature | 0x3F0 + i*0x1C | 0x1C | struct[10] | display queue entries (spell, pos x3, obj) | 0x636A90 | 0x588ED0 | med | |
| CSWSObject | 0x100 | | list | action queue | 0x74DC80 (`local_84+0x100`) | AddAction/Front | high (AI4) | |
| CSWSCombatRound | 0xA84 | 4 | int | round started | 0x6D9500 `006da47e`, 0x74DC80 | 0x58E730 `0058e873` | high | |
| CSWSCombatRound | 0xA94 / 0xA9C | 4 | int | round timer / length (3000) | 0x590110, 0x58F7A0 | 0x58E730 `0058e8da` | high | |
| CSWSCombatRound | 0xAA8 / 0xAAC | 4 | int | round busy flag / value | 0x6D9500 `006d9554`, 0x571030 | 0x58FC60 | med | |
| CSWSCombatRound | 0xAB0 / 0xAB4 | 4 | int | pause timer / flag | 0x58FB40 | 0x58FCD0 | med | |
| CSWSCombatRound | 0xAB8 | 4 | int | actions done this round | 0x6D9500 `006d96c0` | 0x5B96F0 `005bb076`, 0x5B8000 `005b9447`, 0x6D7390 `006d90b0`, reset `0058ea53` | high | |
| CSWSCombatRound | 0xB00 / 0xB04 / 0xB18 | 4 | ptr | pending round list / master creature / current round node | 0x590110, 0x74DC80 | 0x590270 | med | |
| round action node | 0x88 B: +0 time, +4 word, +8 int (500 for spells), +0x10 byte type (1/0xB attack, 3, 6, 7, 9 spell, 0xA item, 0xC, 0xD stance, 0xE), +0x24.. spell args (type 9), +0x38.. pos (type 0xA), +0x64 item, +0x84 extra | | | | 0x6D9500 | 0x591670, 0x591890, ctor 0x58C810 | med | |

## (f) Hook-site catalogue
Each window was checked by script: whole-instruction boundary, len >= 5, no relative CALL/Jcc/JMP inside, no branch from outside to an address strictly inside, no overlap within 12 bytes of `live_hooks_2026-09-30g.tsv` or any `kotor2-steam-aspyr.hooks.toml` (only hit: H-HUD entry `0x74B3F0`, used by action_hotbar). The lead re-checks with the atlas window checker. Stolen bytes run after the handler. Sibling self-patch sites (0x40D69E, 0x8ABA8C..0x8AE793) are > 0x100000 away from every window.
| id | address | length | stolen bytes | convention / live regs | consumed exit? | EAX/stack notes | hot/cold | nearest sibling site |
|---|---|---|---|---|---|---|---|---|
| H1 UseItem entry | 0x57B4C0 | 5 | `55 8B EC 6A FF` | thiscall, ECX=creature; [esp+4]=item id, +8=prop idx, +0xC=subprop, +0x10=target obj (0x7F000000 none), +0x14=float* pos, +0x18=area, +0x1C=flag; ret 28 | no | EAX dead | warm | far |
| H2 AddCastSpellActions entry | 0x565310 | 9 | `55 8B EC 81 EC 20 01 00 00` | thiscall, ECX=creature; [esp+4]=spell id, +0x24=target obj (p10), +0x2C=location flag (p12), +0x30=front flag (p13), +0x48=real flag (p19: 0 = round/queue path, 1 = AIActionCombat conversion); ret 72 | no | EAX dead | warm | far |
| H3 AddItemCastSpellActions entry | 0x5648D0 | 9 | `55 8B EC 81 EC 10 01 00 00` | thiscall, ECX=creature; [esp+4]=item id, +8 prop, +0xC sub, +0x10/0x14/0x18 pos xyz, +0x1C=target obj (p8), +0x28=front (p11), +0x2C=real flag (p12); ret 44 | no | EAX dead | warm | far |
| H3b AddUseTalentOnObjectActions entry | 0x57D8B0 | 6 | `55 8B EC 83 EC 34` | thiscall, ECX=creature; ret 32 | no | EAX dead | cold | far |
| H4 AIActionCombat pause check | 0x6D957C | 8 | `6A 02 8B 8D 20 FF FF FF` | in-function: creature=`[EBP+0xFFFFFE54]`, node=`[EBP+8]`; stolen MOV loads ECX for the following `CALL 0x0051C720` (not stolen); reached only when `[round+0xAA8]==0` | no | EAX free | hot only while a 0x3F sits at the queue head | far |
| H4b AIActionCombat entry | 0x6D9500 | 5 | `55 8B EC 6A FF` | thiscall, ECX=creature, [esp+4]=action node; ret 4 | no | EAX dead | as H4 | far |
| H4c UpdateCombatRoundTimer pause check | 0x571064 | 5 | `6A 02 8B 4D E0` | creature=`[EBP+0xFFFFFF70]`; next `CALL 0x0051C720` then `JNZ 0x005713E6` (not stolen) | no | EAX free | HOT (per creature with a round, per tick) | far |
| H5 server SetPauseState | 0x538ED0 | 9 | `55 8B EC 81 EC C4 00 00 00` | thiscall, ECX=CServerExoAppInternal; [esp+4]=mask (byte), [esp+8]=value (1 set, 0 clear) | no | EAX dead | cold (pause toggles) | far |
| H5b GetPauseState (flag read) | 0x538CD0 | 7 | `55 8B EC 51 89 4D FC` | thiscall, ECX=server internal, [esp+4]=mask; returns EAX bool | no | EAX dead at entry | HOT | far |
| H6 GetWorldTime frozen test | 0x51AD37 | 7 | `8B 45 EC 83 78 24 01` | timer at `[EBP-0x14]`; next `JNZ 0x0051AD58` (not stolen) uses the stolen CMP | no | EAX = day (already saved to `[EBP-8]`), overwritten by the stolen MOV: safe | HOT | far |
| H7 server tick, running branch start | 0x533892 | 6 | `8B 8D 60 FE FF FF` | this(server internal)=`[EBP+0xFFFFFE60]`; once per tick, just before the timer refresh `CALL 0x538DC0` | no | EAX free | once per frame | far |
| H7b before UpdateState | 0x533968 | 6 | `8B 88 44 00 01 00` | EAX = CServerExoAppInternal (loaded at `00533962`); next `CALL 0x0051D790` not stolen | no | stolen insn READS EAX (fine, not consumed) | once per frame | far |
| H8 HUD queue visibility gate | 0x74C168 | 12 | `8B 8D 94 FE FF FF 8B 91 04 A9 00 00` | inside 0x74B3F0; start is a loop-exit branch target (OK); after the stolen pair EDX=`[this+0xA904]`, `SHR EDX,1 / AND EDX,1 / JZ 0x74C186` follow (not stolen) | no | a mod can write `[this+0xA904]` in the handler; the stolen MOV then reloads it | per frame | far |
| H9 UpdateActionQueue_2 entry | 0x74DC80 | 5 | `55 8B EC 6A FF` | ECX=CSWGuiMainInterface | no | EAX dead | per frame only when visible | far |
| H9b show/hide flag setter | 0x74E910 | 6 | `55 8B EC 83 EC 30` | thiscall, ECX=interface; [esp+4]=bool (combat mode) | no | EAX dead | per frame (every HUD Update) | far |
| H10 client SetCombatMode | 0x79D6E0 | 6 | `55 8B EC 83 EC 08` | thiscall, ECX=CClientExoAppInternal; [esp+4]=new mode | no | EAX dead | cold | far |
| H10b UpdateCombatMode | 0x79D840 | 9 | `55 8B EC 81 EC A0 00 00 00` | fastcall, ECX=CClientExoAppInternal | no | EAX dead | per frame | far |
| H10c GetCombatMode | 0x740860 | 6 | `55 8B EC 83 EC 08` | thiscall, ECX=CClientExoApp; returns `[[ECX+4]+0x320]` | no | EAX dead at entry | per frame, several callers | far |
| H11 server queue display gate | 0x569E9D | 6 | `8B 91 E8 10 00 00` | in AIUpdate: ECX=creature (loaded at `00569e97`); stolen reads `[ECX+0x10E8]` | no | EDX overwritten | HOT (every creature AIUpdate) | far |
| H11b UpdateActionQueue entry | 0x588ED0 | 9 | `55 8B EC 81 EC 00 01 00 00` | fastcall, ECX=creature | no | EAX dead | cold (300 ms, Exile only) | far |
| H12 ActionManager entry | 0x563C90 | 6 | `55 8B EC 83 EC 10` | thiscall, ECX=creature, [esp+4]=mode (1,2,4,8) | no | EAX dead | warm | far |
| H13 message router entry | 0x65BE70 | 5 | `55 8B EC 6A FF` | thiscall, ECX=CSWSMessage; [esp+4]=player id, [esp+8]=minor opcode (byte in a dword slot) | no | EAX dead | warm (per input message) | far |
| H14 UseItem message site | 0x65C5D4 | 8 | `6A 01 8B 85 74 FF FF FF` | in 0x65BE70 opcode 9: creature=`[EBP-0x10]`, item=`[EBP+0xFFFFFF7C]`, prop=`[EBP-0x7D]`, sub=`[EBP+0xFFFFFF73]`, target=`[EBP-0x7C]`; start is the `JNZ 0x65C5D4` target (OK) | no | EAX free | warm | far |
Not hookable (relative CALL in range): the AddAction(0x3F) sites `0x565436`, `0x5649ED` and the pause CALLs `0x6D9584`, `0x571069`; use the preceding PUSH/MOV pairs (H4, H4c). Existing atlas sites that also apply: AI4-H08 `0x5D5C22` (RunActions dispatch), AI4-H09 `0x5D6A60` (result histogram).

## (g) Mod relevance
- medpac: the whole Q2 chain (UseItem -> AddItemCastSpellActions round path, ActionManager(4) polarity, AIActionCombat gating, one action per 3000 ms round).
- party-flag (`+0x11AC`): AddItemCastSpellActions / AddCastSpellActions skip the visibility test for party members (`[this+0x11AC] == 0` guard), UseItem posts event 0xFA1 only for party members.
- none from the closed list for the pause semantics; pending item for this bug is "paused-command indicator + post-unpause timing" (action hotbar mod): either a client overlay keyed on `GetPauseState(2)` + a 0x3F node at the head of party slot 0's queue, or probes P1-P4 first.
- perf: H5b, H6, H4c, H11 are hot; prefer H5 (cold) and H4 (conditional) for probes.
