# GameAPI K2 follow-up research (task R-D)

Status 2026-10-04, read-only. Nothing committed, no DB or game file touched. Inputs: Steam vanilla exe sha 6a522e71 (objdump), GOG K2 raw blocks, `ghidra_dumps/upstream_server/*/fp.tsv`, K1 GOG 1.03 via read-only `ExportDisasmList.java` on the server. Per-row detail is in `followup.tsv`; the older rows are in `wrapper_fixes.tsv`. Worktree read: `kpm-pr-gameapi2` (upstream master).

## Findings

| # | question | answer | confidence |
| --- | --- | --- | --- |
| 1 | Do the 5 GUI wrapper destructors hit a K2 deleting dtor? | Yes on all 5, on Steam and GOG. K2 DB role "deleting" = `push ebp; call plain; and eax,1; operator delete; ret 4`. K1 role = plain, `ret 0`. | high |
| 1b | Names | K2 Image/ListBox/ScrollBar/Text: `Destructor` = deleting. K2 **Slider is reversed**: `Destructor` = plain, `Destructor_2` = deleting, so the wrapper's `Destructor_2` lands on the deleting dtor on K2 and is the plain one only on K1. | high |
| 1c | K2 signature | `void* __thiscall(void* this, DWORD deleteFlag)`, `ret 4`, pass **0** (wrapper allocates with `malloc`, frees with `free`; flag bit 0 would call the engine's `operator delete` on it). K1 stays `(void* this)`, `ret 0`. | high |
| 2 | Real K2 `CGuiInGame::ShutDownGlobal` | Steam **0x7c7b40**, GOG **0x4d8230** (plain, `ret 0`, same typedef as K1). The DB rows (Steam 0x7bdb10, GOG 0x4cdda0) point at a deleting dtor of a 2-`CExoString` object: wrong on K2. | high |
| 3 | Who owns `UpdateMessageGUI`'s pointer | The **caller**; the callee only borrows it as `this` for `CSWGuiInGameMessages::AddMessages/AddDialogMessages/AddCombatMessages` (null = no-op). Never stored, copied or freed. K2 keeps the panel at `CGuiInGame+0x78` (K1: +0x1c). | high |
| 4 | Reachability on K2 today (new) | `CGuiInGame::InitializeFunctions` uses the throwing `GetFunctionAddress`; on the K2 GOG DB (and the Steam branch DB) it throws at name #6 `GetCurrentEquipScreenNPC`. Only AddFloatyText, AppendToMsgBuffer, GetMessageBufferSize, **UpdateMessageGUI**, ChangeCharacter, DoQuickLoad resolve. So `ShowPartySelection`, `ShowUpgradeScreen`, `ShutDownGlobal` are dormant no-ops today, `UpdateMessageGUI` is the one live hazard. | high |
| 5 | Destructor reachability (new) | GOG DB: Image, Slider, Text, Border, CRes, CResGFF, ListBox, ScrollBar destructors all resolve before any throw, so they are live. Steam: upstream DB has 48 rows (none of these); branch DB has CRes/CResGFF/Border/Panel rows only, so Image/ListBox/ScrollBar/Slider/Text never resolve on Steam until those DB rows are added. | high |
| 6 | Stale comment | `CGuiInGame.h` lines 15-17 say CGuiInGame is only in the K1 DB; GOG upstream has 100+ CGuiInGame rows. Update the comment in the same commit. | high |

## 1. Destructors (CSWGuiImage, ListBox, ScrollBar, Slider, Text)

DB rows (K2 GOG upstream DB, K1 1.03 DB; Steam has no rows, VAs found by structure, see below):

| class | wrapper DB name | K1 VA (role) | GOG VA (role) | Steam VA (role) |
| --- | --- | --- | --- | --- |
| CSWGuiImage | Destructor | 0x415ad0 plain, `ret 0` | 0x914770 deleting, `ret 4` (plain 0x9147e0) | 0x4163a0 deleting (plain 0x416410) |
| CSWGuiListBox | Destructor | 0x41b060 plain | 0x91b170 deleting (plain 0x91b1a0) | 0x41d160 deleting (plain 0x41d190) |
| CSWGuiScrollBar | Destructor | 0x419e40 plain | 0x91a0a0 deleting (plain 0x91a0d0) | 0x41c090 deleting (plain 0x41c0c0) |
| CSWGuiSlider | **Destructor_2** | 0x41bb40 plain (0x41bb20 = deleting) | 0x597840 deleting (plain **Destructor** 0x56ab00) | 0x84df40 deleting (plain 0x41ac10) |
| CSWGuiText | Destructor | 0x415ff0 plain | 0x915630 deleting (plain 0x915660) | 0x417260 deleting (plain 0x417290) |

Anchors (GOG, e.g. Image 0x914770): `55 8B EC 51 89 4D FC 8B 4D FC E8 <plain> 8B 45 08 83 E0 01 74 0C 8B 4D FC 51 E8 <operator delete 0x921321> 83 C4 04 8B 45 FC 8B E5 5D C2 04 00` (44 bytes). Steam 0x4163a0 and 0x84df40 disassembled byte-for-byte the same shape with `call 0x9196fd`; 0x417260, 0x41c090, 0x41d160 show the same call/`and eax,1`/`call 0x9196fd`/`ret 4` skeleton.

Identity anchors for the Steam VAs (no DB rows exist, so matched by structure and RTTI):
- Each plain dtor stores a vtable pointer; vtable slot 0 equals the deleting dtor above; RTTI at vtable-4 reads `.?AVCSWGuiImage@@`, `ListBox`, `ScrollBar`, `Slider`, `Text` on both Steam (0x98764c, 0x987d3c, 0x987c74, 0x987aec, 0x9876b4) and GOG (0x9a99d0, 0x9a9bf4, 0x9a9b34, 0x99124c, 0x9a9a28).
- fp.tsv fingerprints agree between titles: ListBox plain size 250 with disp 0x2dc x3; ScrollBar plain size 131 with disp 0x13c; Slider plain size 143 with disp 0x170; Image plain size 154 with disp 0x24 x3; Text plain size 175 with disp 0x14.

K1 verification (server, read-only): K1 `Destructor` of Image/ListBox/ScrollBar/Text is plain `ret`; `Destructor_2` is `call plain; test byte [esp+8],1; operator delete 0x6fa390; ret 4`. K1 Slider is the reverse (0x41bb20 deleting, 0x41bb40 plain), which is why the wrapper uses `Destructor_2`.

Harm today (GOG): the wrapper calls `destructor(objectPtr)` with no flag, then `free(objectPtr)`. On K2 that leaves one stale dword for the callee's `ret 4` to pop (stack skew) and a garbage flag in `[ebp+8]`; if bit 0 happens to be set the engine `operator delete`s the object and the wrapper frees it a second time. Who frees after the fix: the wrapper (`malloc` in the ctor path, `free` in `~T`), the engine never.

Checked and fine: `CSWGuiPanel::Destructor` is plain `ret 0` on K2 (GOG 0x90cce0 `~CSWGuiPanel`, Steam DB 0x40e860). No change.

## 2. K2 `CGuiInGame::ShutDownGlobal`

| | K1 1.03 | K2 GOG | K2 Steam |
| --- | --- | --- | --- |
| real function | 0x62a970 | **0x4d8230** | **0x7c7b40** |
| DB row today | 0x62a970 (right) | 0x4cdda0 (wrong) | 0x7bdb10 (wrong) |
| frees via `vtable[0](1)` | this+0x98, 0x9c, 0xa0, 0xa4 | this+0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4 | same as GOG |
| final store | `[this+0x38]=0` | `[this+0x38]=0` @0x4d83fe | `[this+0x38]=0` @0x7c7d0e |
| ret | 0 | `c3` @0x4d8408 | `c3` @0x7c7d18 |

Anchors:
- Same shape as K1: for each field `cmp [this+X],0; je; push 1; mov edx,[obj]; mov eax,[edx]; call eax; mov [this+X],0`; then `mov [edx+0x38],0`. K1 +0x38 is `manager` (K1 DB offsets), so the template matches.
- fp.tsv fingerprint is identical on both titles: size 473, retn 0, disp `0x38,0xa0x3,0xa4x3,0xa8x3,0xacx3,0xb0x3,0xb4x3`; prolog `55 8B EC 83 EC 4C 89 4D` at both 0x7c7b40 and 0x4d8230 (16-byte aligned).
- Only caller is the `CGuiInGame` destructor: Steam 0x7bdbc0 (`call 0x7c7030` ShutDown @0x7bdbf2, then `call 0x7c7b40` @0x7bdbfa); GOG 0x4cde50 (`call 0x4d7720` @0x4cde82, then `call 0x4d8230` @0x4cde8a). Order ShutDown, ShutDownGlobal matches the K1 pattern.
- Field types, from the vtables of the constructors called in `CGuiInGame::Initialize` (Steam 0x7be4c0, GOG 0x4ceb50, both 5472 bytes): +0xa0 `CSWGuiMessageBox` (ctor 0x75ae40), +0xa4 `CSWGuiMessageBox` (0x75b370), +0xac `CSWGuiSkillInfoBox` (0x89c5f0), +0xb0 `CSWGuiTutorialBox` (0x898ca0), +0xb4 `CSWGuiMessageBox` (0x75ae40). K1 had message_box, skill_info_box, tutorial_box, controller_loss_box; K2 has six panel slots (the +0xa8 store was not matched by my scan pattern, so its class is unnamed, but the free loop in the function covers it).
- No global is read or written; the function only touches `this`.
- The wrong rows: Steam 0x7bdb10 / GOG 0x4cdda0 are the scalar/vector deleting dtor of a 16-byte object with two `CExoString`s (`and eax,2` vector path, `and ecx,1` operator delete, `ret 4`), called from the CGuiInGame dtor at Steam 0x7bdc28.

Wrapper consequence: no new typedef. The K1 typedef is correct for the real function. Needs a DB repoint (GOG row, Steam branch row, `param_size` 0). Until then the wrapper is dormant (see finding 4), so the safe plan is no wrapper change and a note for the DB PRs. If the DB is repointed before the next wrapper change, nothing else is needed.

## 3. `UpdateMessageGUI` argument ownership

K2 (Steam 0x7cd040 / GOG 0x4dd740, both `ret 4`): `cmp [ebp+8],0; je epilogue`, then three calls with `ecx=[ebp+8]` and (buffer,count) pairs from this+0x110/0x11c, 0x114/0x120, 0x118/0x124: Steam 0x758e30 / 0x7592f0 / 0x759820 = GOG 0x52a530 `CSWGuiInGameMessages::AddMessages` / 0x52a9f0 `AddDialogMessages` / 0x52af20. `[ebp+8]` is never stored, copied or freed: the **caller owns it**, the callee borrows it.

Callers (GetInGameGui 0x73f750 / 0x40cc10 is `ret 0`, so the push before it belongs to UpdateMessageGUI):
- Steam 0x759e20 (770 bytes, `ret 8`), GOG 0x52b520 `CSWGuiInGameMessages::HandleInputEvent`: `push 0` @0x75a0e6 (GOG @0x52b7e6), `call 0x7cd040` @0x75a0f2. Null makes it a no-op.
- Steam 0x75a1f0 (188 bytes), GOG 0x52b8f0 `CSWGuiInGameMessages::OnPanelAdded`: pushes its own `this` (`[ebp-0x1c]`) @0x75a20f (GOG @0x52b90f). So the pointer is the CSWGuiInGameMessages panel, and the K2 engine moved K1's implicit `this->in_game_messages` (+0x1c) into an explicit argument.

K2 field to read: `CGuiInGame+0x78` holds the CSWGuiInGameMessages (Initialize stores the result of ctor 0x757c40 / 0x529340, RTTI `.?AVCSWGuiInGameMessages@@`, at Steam 0x7bf18a and GOG 0x4cf81a; `new` size 0x2508). Offset +0x1c on K2 is `CSWGuiPartySelection` (ctor 0x89cf30), not messages. The K2 DBs carry no `CGuiInGame` offsets, so hard-code 0x78 under the K2 title gate (or add `in_game_messages` rows to both K2 DBs and use `GameVersion::GetOffset`).

Harm today: it is resolved on K2 and the wrapper pushes no arg, so the callee reads a stack slot (stale or the caller's data) and `ret 4` unbalances the stack.

## 4. Commit 2 typedefs (K1 untouched, K2 branched on `GameVersion::GetTitle() == GameTitle::KOTOR2`)

K1 path = today's typedef and call, unchanged. Unknown title falls to the K1 path. All files are in `Patches/Common/GameAPI/` (worktree lines are at upstream master; Commit 1 touches CClientExoApp, CSWCObject, CSWCPlaceable, Gob only, so no overlap).

| fn | K1 typedef (kept) | K2 typedef (new) | Steam / GOG VA | file:lines |
| --- | --- | --- | --- | --- |
| StartLoadFromLayout | `void(__thiscall*)(void*, void* resref)` ret 4 | `(void*, void* resref, int scaleA, int scaleB)` ret 0xC, call with `(resref,1,1)` | 0x40ee40 / 0x90d2c0 | CSWGuiPanel.h:142, CSWGuiPanel.cpp:465-468 |
| InitControl | `(void*, void* ctl, void* label, int activate)` ret 0xC | `(..., int activate, int scaleToViewport)` ret 0x10, call with 1 | 0x40f620 / 0x90daa0 | CSWGuiPanel.h:137, CSWGuiPanel.cpp:435-440 |
| ShowPartySelection | `(void*, void* exitScript, int requireAccept, int npc1, int npc2)` ret 0x10 | `(..., int npc2, int arg5)` ret 0x14, call with 0 | 0x7cb920 / 0x4dc020 | CGuiInGame.h:149, CGuiInGame.cpp:481-486 |
| ShowUpgradeScreen | `(void*, DWORD itemId)` ret 4 | `(void*, DWORD itemId, DWORD characterId, int disableItemCreation, int disableUpgrade, ExoStringByValue override2DA)` ret 0x18 | 0x7cd7e0 / 0x4ddee0 | CGuiInGame.h:165, CGuiInGame.cpp:588-593 |
| UpdateMessageGUI | `(void*)` ret 0 | `(void*, void* inGameMessages)` ret 4 | 0x7cd040 / 0x4dd740 | CGuiInGame.h:114, CGuiInGame.cpp:261-266 |
| ShutDownGlobal | `(void*)` ret 0 | no change (DB repoint only) | 0x7c7b40 / 0x4d8230 | CGuiInGame.h:176 (none) |
| CRes, CResGFF, CSWGuiBorder, Image, ListBox, ScrollBar, Slider, Text destructors | `T(__thiscall*)(void*)` ret 0 | `void*(__thiscall*)(void*, DWORD)` ret 4, flag 0 | see section 1 and `followup.tsv` | see snippet C |

Argument names in the K2 column that are labels rather than proven: `scaleA/scaleB`, `scaleToViewport`, `arg5`, `characterId`, `disableItemCreation`, `disableUpgrade` (counts, widths and defaults are proven, see `followup.tsv`).

### Snippet A, panel and CGuiInGame wrappers (paste into the listed files)

```cpp
// CSWGuiPanel.h, next to StartLoadFromLayoutFn / InitControlFn
typedef void (__thiscall* StartLoadFromLayoutK2Fn)(void* thisPtr, void* guiResref, int scaleA, int scaleB);
typedef void (__thiscall* InitControlK2Fn)(void* thisPtr, void* controlToInit, void* label, int activate, int scaleToViewport);

// CSWGuiPanel.cpp
void CSWGuiPanel::StartLoadFromLayout(CResRef* guiResref) {
    if (!objectPtr || !startLoadFromLayout) return;
    void* res = guiResref ? guiResref->GetPtr() : nullptr;
    if (GameVersion::GetTitle() == GameTitle::KOTOR2) {
        reinterpret_cast<StartLoadFromLayoutK2Fn>(startLoadFromLayout)(objectPtr, res, 1, 1);  // vanilla K2 pushes (resref,1,1)
        return;
    }
    startLoadFromLayout(objectPtr, res);                                                        // K1: unchanged
}
void CSWGuiPanel::InitControl(CSWGuiControl* controlToInit, CExoString* label, int activate) {
    if (!objectPtr || !initControl) return;
    void* ctl = controlToInit ? controlToInit->GetPtr() : nullptr;
    void* lbl = label ? label->GetPtr() : nullptr;
    if (GameVersion::GetTitle() == GameTitle::KOTOR2) {
        reinterpret_cast<InitControlK2Fn>(initControl)(objectPtr, ctl, lbl, activate, 1);       // vanilla 4th arg is 1
        return;
    }
    initControl(objectPtr, ctl, lbl, activate);                                                 // K1: unchanged
}

// CGuiInGame.h
struct ExoStringByValue { char* buf; DWORD len; };   // CExoString layout {char* @0, uint @4}; callee destroys it
typedef void(__thiscall* UpdateMessageGUIK2Fn)(void* thisPtr, void* inGameMessages);
typedef void(__thiscall* ShowPartySelectionK2Fn)(void* thisPtr, void* exitScript, int requireAccept, int forcedNPC1, int forcedNPC2, int arg5);
typedef void(__thiscall* ShowUpgradeScreenK2Fn)(void* thisPtr, DWORD itemId, DWORD characterId,
                                                int disableItemCreation, int disableUpgrade, ExoStringByValue override2DA);

// CGuiInGame.cpp (K1 lines stay as they are)
static constexpr int K2_OFFSET_IN_GAME_MESSAGES = 0x78;   // Steam 0x7bf18a, GOG 0x4cf81a; K1 uses +0x1c internally
void CGuiInGame::UpdateMessageGUI() {
    if (!objectPtr || !updateMessageGUI) return;
    if (GameVersion::GetTitle() == GameTitle::KOTOR2) {
        void* msgs = getObjectProperty<void*>(objectPtr, K2_OFFSET_IN_GAME_MESSAGES);   // caller-owned, borrowed by the callee
        if (!msgs) return;
        reinterpret_cast<UpdateMessageGUIK2Fn>(updateMessageGUI)(objectPtr, msgs);
        return;
    }
    updateMessageGUI(objectPtr);
}
void CGuiInGame::ShowPartySelection(CExoString* exitScript, int requireAccept, int forcedNPC1, int forcedNPC2) {
    if (!objectPtr || !showPartySelection) return;
    void* s = exitScript ? exitScript->GetPtr() : nullptr;
    if (GameVersion::GetTitle() == GameTitle::KOTOR2) {
        reinterpret_cast<ShowPartySelectionK2Fn>(showPartySelection)(objectPtr, s, requireAccept, forcedNPC1, forcedNPC2, 0);
        return;
    }
    showPartySelection(objectPtr, s, requireAccept, forcedNPC1, forcedNPC2);
}
void CGuiInGame::ShowUpgradeScreen(DWORD itemId) {
    if (!objectPtr || !showUpgradeScreen) return;
    if (GameVersion::GetTitle() == GameTitle::KOTOR2) {
        // {nullptr,0}: the callee runs ~CExoString on it (frees buf if non-null), never pass a stack buffer
        reinterpret_cast<ShowUpgradeScreenK2Fn>(showUpgradeScreen)(objectPtr, itemId, 0x7f000000 /*OBJECT_INVALID*/, 0, 0, ExoStringByValue{nullptr, 0});
        return;
    }
    showUpgradeScreen(objectPtr, itemId);
}
```

Caveat for the two dormant rows (ShowPartySelection, ShowUpgradeScreen) and ShutDownGlobal: they stay unreachable while the throwing resolve loop aborts at #6. Either keep that as is (gating is then future-proofing) or change that loop to `GameVersion::ResolveFunction`, which is a separate behaviour change and not part of this commit. UpdateMessageGUI is the only row that is exercised on K2 today.

### Snippet B, one shared helper for the 8 destructor sites (new file `GameAPI/GameDestructor.h`)

```cpp
#pragma once
#include "../Common.h"
#include "GameVersion.h"

// K1: the DB role the wrapper resolves is the plain dtor (thiscall, ret 0).
// K2: the same role is the scalar deleting dtor (thiscall, ret 4, flag at [esp+4]).
// The wrapper owns objectPtr (malloc) and frees it itself, so the flag is 0 (never run operator delete).
inline void CallGameDestructor(void* fn, void* object) {
    if (!fn || !object) return;
    if (GameVersion::GetTitle() == GameTitle::KOTOR2) {
        reinterpret_cast<void*(__thiscall*)(void*, DWORD)>(fn)(object, 0);
    } else {
        reinterpret_cast<void*(__thiscall*)(void*)>(fn)(object);   // identical to today's call
    }
}
```

### Snippet C, call sites (replace `destructor(objectPtr);`, keep the `free(objectPtr)` that follows)

```cpp
CallGameDestructor(reinterpret_cast<void*>(destructor), objectPtr);
```

Files and lines at upstream master: CSWGuiImage.cpp:101, CSWGuiListBox.cpp:171, CSWGuiScrollBar.cpp:118, CSWGuiSlider.cpp:128, CSWGuiText.cpp:115, CSWGuiBorder.cpp:113, CRes.cpp:110 (no null check today, the helper adds one), CResGFF.cpp:421. Add `#include "GameDestructor.h"` to each. `CSWGuiSlider.cpp:51` comment ("Destructor_2 is the plain one") should say "on K1; on K2 Destructor_2 is the deleting dtor and Destructor is plain". `CResGFF` `destructor2` (CResGFF.h:109, `Destructor2Fn` with a `byte` flag) is never called; on K2 `Destructor_2` is the plain dtor (Steam 0x7171b0, GOG 0x623310), so the K1 flag typedef is wrong there too: delete the unused pointer.

Alternative if the maintainer dislikes a new header: per class add `typedef void*(__thiscall* DestructorK2Fn)(void*, DWORD)` and branch in `~T`; same behaviour.

Also update `CGuiInGame.h:15-17` (stale "K1 DB only" comment).

## 5. Things the implementer or lead should know

1. DB follow-ups (belong to the DB PRs, not this one): repoint `CGuiInGame::ShutDownGlobal` (GOG 0x4d8230, Steam 0x7c7b40, param 0); GOG `ShowUpgradeScreen` param_size 20 should be 24; Steam rows are missing for CSWGuiImage/ListBox/ScrollBar/Slider/Text and need adding, in the GOG naming order (ascending address: Image/ListBox/ScrollBar/Text `Destructor` = deleting then `Destructor_2` = plain; Slider `Destructor` = plain 0x41ac10 then `Destructor_2` = deleting 0x84df40). Add K2 `CGuiInGame in_game_messages = 0x78` offset rows if you prefer `GetOffset` over the constant.
2. Nothing here was run in a game. K1 evidence is the server's K1 GOG 1.03 program; K2 is static disassembly of both exes.
3. Steam VAs for the five GUI classes come from structural matching (RTTI, vtable slot 0, fp fingerprints), not from a DB row or a name; every Steam address above was disassembled from the exe (sha 6a522e71).
4. Scratch artifacts (not in repo): K1 export `k1out/disasm.txt` in the session scratch dir.
