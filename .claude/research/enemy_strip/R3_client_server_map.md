# R3 client/server map (Steam exe 6a522e71, static only, no live run)

## 1. Client object id
- `CSWCObject::GetServerObject` = `0x007F2540` (name_map steam-db-B0, high). Reads the client id at `[this+0x04]` (asm `007f258d MOV EAX,[EDX+4]`), then calls `0x0051C8B0`.
- `0x0051C8B0` = ClientToServerObjectId: a thunk to `0x00539D30` (asm `0051c8c1`). `0x539D30`: if id == `0x7F000000` return it, else `id & ~[0x0099449C]`.
- `[0x0099449C]` is a .rdata constant = `0x80000000` (read from the exe). So serverId = `clientId & 0x7FFFFFFF`.
- The objTable argument is read (`0051c8be MOV ECX,[ECX+4]`) but never used. It must be a readable pointer.

## 2. Where objTable comes from
- `A = [0x00A1B4A4]` (.data, AppGlobal).
- `S = [A+8]` = CServerExoApp* (asm `007f2577 MOV ECX,[EAX+8]`; `patch_manager_mods/30_queue_hud_evidence/RE.md:184`; `_shared_src/ItemRead.h:74`).
- `[A+4]` is the client app, not the server table.
- Inside `0x51C0A0`: `I=[S+4]`, `H=[I+0x1005C]`, and H is the hash struct (asm `0051c0a9..0051c0c5`).

## 3. Server id -> server object
- GetGameObject = `0x0051C0A0`: __thiscall, ECX = S, one stack arg = serverId, RET 4. Returns the object or 0 (asm `0051c0d0 CALL 0x53DFB0`).
- `0x53DFB0` (decomp `53/0053dfb0.c`): bucket = `(id&0xFFF) + (id sign bit ? 0x1000 : 0)`. Node = {+0 key = id&0x7FFFFFFF, +4 obj, +8 next}.
- Existing users of `0x51C0A0`: `companion_ai_src/CompanionAI.cpp:72`, `npc_diversity_src/DiversityAnimEngine.h:19`.
- Existing users of the item variant: `_shared_src/ItemRead.h:72-81`, `action_hotbar_src/ActionHotbar.cpp:566-573`.
- Creature variant: `_shared_src/GameAddr.h:36` MemberServerCreature = `0x0077D800`; `_shared_src/Party.h:86-90`.
- `portrait_slots_src/common_contract_test.cpp:221`: roster(i) = `party::serverCreature(member)`. The buff HUD gets its creature through this (`buff_duration_hud_src/BuffDurationHUD.cpp:202-209`).
- `shield_bar_src`, `buff_duration_hud_src`, `queue_hud_src`: no direct reference to `0x77D800` or `0x51C0A0` (rg).

## 4. Server creature getter and cache
- `0x0077D800` (asm `0077d80c`, `0077d818`, `0077d828`): calls `GetServerObject(ECX=client)`, then `vtbl[0x30]()` on the result with no stack args, plain RET.
- `GetServerObject` (asm `007f254c..25df`):
  - if `[this+0xF8] != 0`, return it (cache hit).
  - if `[this+0xE4] != 0`, return 0.
  - `obj = GetGameObject(S, ClientToServer([this+4]))`.
  - `[this+0xF8] = obj->vtbl[0x10]()`. This is the only write, and no clear was found in this function.
- `vtbl[0x30]` = AsCreature is assumed by name (`30_queue_hud_evidence/RE.md:214`), used the same way as in `0x5B8000`.

## 5. Icon cache at creature+0xFFC (CORRECTS doc 02 "client-side")
- Owner = SERVER creature. Reader `0x0074EE90` (CSWGuiInGameGameplay::UpdatePartyPortraits, name_map) sets `local_3c = 0x77D800()` (decomp `74/0074ee90.c:65-70`). Asm `0074f516 ADD EAX,0xffc`.
- Filler `0x005A43C0` = CSWSEffectListHandler::OnApplyEffectIcon (name_map steam-db-B0, verified). Creature = `param_1->vtbl[0x30]()`.
- List = CExoArrayList at +0xFFC: `+0x00` entry** data, `+0x04` (= +0x1000) count.
- Entry = 0x24 bytes (`FUN_00919723(0x24)`). Offsets verified from the decomp (`short*` indexing scaled to bytes):
  - `+0x00` short spell id = effect GetInteger(0), i.e. `*(effect+0x34)[0]` (`0x5E4E60`). Dedupe key.
  - `+0x02..+0x0F` zero (ctor `0x7107F0` = memset 16).
  - `+0x10..+0x13` never written.
  - `+0x14` dword buff flag = spells.2da "Good" column of the row != 0 (string `"Good"` at .rdata `0x9963C8`).
  - `+0x18` ushort Priority. List sorted ascending by it (`0x414280` insert).
  - `+0x1C` dword: bool result of the "Good" read, normally 1. The count loop skips entries where it is 0.
  - `+0x20` dword NameStrRef.
- Gates before insert:
  - `(effect subtype & 7) == 0` -> skip (word at .rdata `0x9967AC` = 0; asm `005a4428`).
  - spells.2da IconResRef empty -> skip (`0x71B1D0`). The icon name itself is not stored.
  - "Good" column read fails -> skip.
  - spell id already present -> skip.
- Count loop `0x0074EE90` (decomp `150-165`): entries with `+0x1C != 0`; `+0x14 == 0` -> debuff, else buff. Counts run only when not dead/disabled (decomp `if (iVar9 == 0)`).
- `0x00744F00` = CSWGuiInGameGameplay::SetPortraitOverlay (name_map, high). It is the GUI slot writer (`slot+0xDDC/+0xDDD`), not the cache.

## 6. Where ctx.target / ctx.leader come from
- ctx.target: `0x74CFE0` (decomp lines 14-35) calls `0x73F4D0` (CClientExoApp::GetGameObject, client hash, REA-confirmed), then vtbl[0x0C] on the result. That value goes to `0x743540` (CSWGuiTargetActionMenu::Update) as target. So ctx.target is CLIENT side.
- ctx.leader: `0x7E5D80()` = `0x7E5DA0(0)` = `0x73F550` (CClientExoApp::GetCreatureByGameObjectID). `patch_manager_mods/30_queue_hud_evidence/RE.md:143` and decomp `7e/007e5da0.c`. So ctx.leader is a CLIENT creature.
- Server leader = `0x77D800(ctx.leader)` (decomp `74/0074cfe0.c` lines 9-12).
- EnemyStrip reads CLIENT objects: `enemy_effects_strip_src/EnemyStrip.cpp:32,139-153` (target +0x148/+0x14C), `:195-198` (leader +0x1198).

## 7. Safest recipe (given ctx.target = T, client creature)
1. If T is 0 or -1, stop. `cid = *(uint32*)(T+4)`. If `cid == 0x7F000000`, stop. `sid = cid & 0x7FFFFFFF`.
2. `A = *(uint32*)0xA1B4A4` and `S = *(uint32*)(A+8)`, both via saferead. If either is 0, stop.
3. `obj = thiscall(ECX=S) 0x0051C0A0(sid)`. If 0, stop (dead or removed).
4. `creature = obj->vtbl[0x30]()` (no args, guard the vtable read). If 0, stop.
5. Effects: `arr=*(creature+0x148)`, `cnt=*(creature+0x14C)`. Require `arr != 0` and `0 < cnt <= 512` (`Effects.h`).
   Icon/buff cache (sparse, not the full list): `*(creature+0xFFC)` with count `*(creature+0x1000)`, 0x24-byte entries as in section 5.
- Shortcut: one call `0x0077D800(T)` also works, but it may return the cached `+0xF8` pointer. Prefer steps 1-4, which re-resolve by id.

## OPEN / UNPROVEN
- vtbl[0x30] identity (AsCreature) is by use only.
- Identity of `vtbl[0x10]` (the value cached at +0xF8) is UNPROVEN.
- Whether `+0xF8` is cleared when a server object dies is UNPROVEN (no clear found in `0x7F2540`).
- What a client CSWCCreature holds at +0x148/+0x14C is not checked. The probe reads the wrong object; the content is UNPROVEN.
- Client ids carrying bit 31 is inferred from the mask use, not from a live id.
- Creature type byte at obj+8: item = 0x06 (`0x537B00` vs `[0x00994475]`); creature value UNPROVEN.
- Removal path of icon entries (who erases from +0xFFC) not traced.
- spells.2da column "Good" existence is inferred from the gate (icons show live), not read from the 2DA.
- `+0x1C` upper bytes (bool return) UNPROVEN.
- Entry `+0x10` use UNPROVEN.
