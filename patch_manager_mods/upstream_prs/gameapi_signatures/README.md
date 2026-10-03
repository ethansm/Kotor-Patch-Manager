# GameAPI wrapper signature research (task R1)

Status: 2026-10-03, read-only research. Nothing was committed, no DB or game file was touched.
Per-row evidence (addresses, asm anchors, harm, confidence) is in `wrapper_fixes.tsv`; this file has the method, the recommended PR scope and the exact diffs.

## Method

1. Rows: the 22 `wrapper_issue` rows of `steam_db_pr3/verified.tsv`. Wrapper source, callers and DBs come from the clean upstream/master export.
2. K2 Steam: `ghidra_dumps/steam_6a522e71/decomp/*.asm|.c` plus `objdump -M intel` on `swkotor2_steam_vanilla.exe` (sha 6a522e71). The final `ret N` of all 22 functions was re-read from the exe bytes (first `ret` after the prologue; every one agrees with the dump).
3. K2 GOG and K1 1.03: the shared Ghidra server through `scripts/ghidra_server_api.py`:
   - `export-script ... ExportFingerprints.java` on `/K1/k1_win_gog_swkotor.exe` (the server's K1 GOG 1.03 program; every one of the 22 K1 DB addresses lands on a function with the matching name) to get `retn`, callees and displacements. The GOG K2 `fp.tsv` already existed.
   - For real disassembly I imported `ghidra_server_api.py` as a module, pointed its `SCRIPT_PATH` at a scratch dir and ran my own read-only `ExportDisasmList.java` (disassembly and decompile of a list of addresses, `-readOnly -noanalysis`). The "no write permission to repository" line in the log is the headless save attempt being refused; nothing is written. Outputs: `evidence/gog_k2_disasm.txt`, `evidence/k1_gog103_disasm.txt`. The credential vault was never opened by me.
4. Callers: `grep -rn` over `Patches/**`, `tests/`, `src/`, `docs/` of the export. Result: no in-tree code calls any of the 22 wrapper methods except `StartLoadFromLayout` and `InitControl` (K1-only GUI patches) and the Destructor wrappers (reached through `~CRes/~CResGFF/~CSWGuiBorder`, used only by K1 patches). Manifests of those patches list only K1 hashes (ModOptions, Snake, K1GUITests, PartyRosterLimit32K1, PlanetsLimit).
5. Which games the wrappers matter for: `Patches/create-patch.py:389` states "Only the Windows module links Common/GameAPI and sqlite". So only DBs with `platform = Windows` matter: `kotor1_0_3.db` (gog/steam/cd 1.03), `kotor2_gog_aspyr.db`, and the Steam K2 DB (the upstream 48-row one carries none of the 22 names; the 1,683-row branch DB carries all 22). The macOS/Linux DBs are irrelevant: `kotor1_steam_aspyr_macos.db` has most of these names as `sysv`, and `kotor2_steam_aspyr_{linux,macos}.db` have none; their patch modules do not link GameAPI.
6. Traps applied: 4-byte slot promotion (short/byte args), thunks (a K1 thunk is `MOV ECX,[ECX+4]; JMP`), hex addresses only, labels never trusted (e.g. Ghidra's `CMFCVisualManagerBitmapCache::IsCached` inside 0x781590 is an FID mislabel).

## Findings in one table

| scope | rows | games agree? | action |
| --- | --- | --- | --- |
| A, fix unconditionally (harmful, identical in K1/GOG/Steam) | `CClientExoApp::DisplayMainMenu`, `ExitProgram`, `GetPausedByCombat`; `CSWCObject::GetPortrait`; `CSWCPlaceable::ActionMenuSecurity`; `Gob::EnableRenderBumpedOut`, `Gob::GetPartLocalPosition` | yes (all 3) | ship in the PR |
| C, harmful on K2 only, K1 signature is right | `CSWGuiPanel::StartLoadFromLayout`, `InitControl`; `CGuiInGame::ShowPartySelection`, `ShowUpgradeScreen`, `UpdateMessageGUI`; `CRes`/`CResGFF`/`CSWGuiBorder` `Destructor` | NO (K1 differs from K2; K2 GOG == K2 Steam) | only with `GameVersion::GetTitle()` gating, K1 path left byte-identical; optional second commit |
| D, row is wrong, not the signature | `CGuiInGame::ShutDownGlobal` (K2 rows point at a deleting destructor of a two-CExoString object) | K1 ok, K2 bogus | do not "fix" the typedef; gate to K1 or drop K2 DB rows |
| E, benign, no change | `CreatureAcquireItem`, `GetCameraAnimationName`, `Scene::GetObjectScale/IsGeomFadeEnabled/SetGeomFadeProperties`, `CResGFF::Destructor_2` (dead pointer) | yes | leave; maybe fix a comment |

Special cases you asked about:

- `CGuiInGame::ShowUpgradeScreen`: Steam and GOG both `RET 0x18` = 24 bytes (Steam 0x7cda9a, GOG 0x4de19a). The GOG DB value 20 is wrong. Args: `itemId, characterId, disableItemCreation, disableUpgrade, CExoString-by-value (8 bytes)`. The by-value string is proven: caller 0x692e50 does `SUB ESP,8; MOV ECX,ESP; CALL 0x7335f0` (CExoString copy ctor into the arg slot, 0x692fa0-0x692fac) and the callee destroys it (`LEA ECX,[EBP+0x18]; CALL 0x733780` at 0x7cda84). K1 pops 4 (one arg), so games disagree.
- `CSWCPlaceable::ActionMenuSecurity`: all three games `RET 8` (K1 0x682df5, GOG 0x536012, Steam 0x842792) and none of them reads either stack arg. Both DB rows that say 0 (GOG) or 4 (K1) are wrong. Universal fix.
- `CSWGuiPanel::StartLoadFromLayout` / `InitControl`: K1 4 and 12, K2 12 and 16. The only in-tree callers are K1 patches, so an unconditional fix would break them. Vanilla K2 callers push `(resref,1,1)` and `(ctl,label,0|1,1)`.
- Destructors: in K2 the DB name `Destructor` is the scalar-deleting variant (flag at `[EBP+8]`, `AND 1` then operator delete, `ret 4`) while K1's `Destructor` is the plain one (`ret 0`); K1 `Destructor_2` is the deleting one. The wrapper calls `destructor(objectPtr)` and then `free(objectPtr)`: on K2 that is a 4-byte stack skew plus a garbage flag, and if bit 0 happens to be set the engine deletes the object and the wrapper frees it again. `CResGFF::Destructor_2` is the opposite sign (wrapper pushes a flag, K2 callee takes none) but `destructor2` is never called, so it is dead.
- `ExitProgram` / `GetPausedByCombat`: all three games have a thunk doing `MOV ECX,[ECX+4]` (K1 0x5ed450 / 0x5edc10, GOG 0x40c75a / 0x40d7fa, Steam 0x73f29a / 0x74033a), so the stdcall wrapper dereferences whatever ECX held. The callee behind the thunk ignores ECX; harm is a latent access violation, not a wrong result.

## Recommended PR scope

"GameAPI: Match wrapper signatures to the engine", rule applied: fix only what is harmful AND proven on every game whose DB has the name.

Commit 1, unconditional (7 rows, all three games proven, no in-tree callers, so the public-signature changes cannot break anything):
`DisplayMainMenu`, `ExitProgram`, `GetPausedByCombat`, `GetPortrait`, `ActionMenuSecurity`, `EnableRenderBumpedOut`, `GetPartLocalPosition`.

Commit 2, optional, title-gated (K1 branch untouched, K2 branch added; K2 GOG and K2 Steam agree on every one):
`StartLoadFromLayout`, `InitControl`, `ShowPartySelection`, `ShowUpgradeScreen`, and the `Destructor` wrappers of `CRes`, `CResGFF`, `CSWGuiBorder`. I recommend including these only if you accept adding a `GameVersion::GetTitle()` branch to the wrappers (the header comment of `GameVersion.h` and Roadmap2026 both anticipate version gating). They cannot be runtime-tested on K1 from this machine; the K1 branch would not change, so the risk is limited to the K2 branch, which also has no in-tree callers.

Leave out, as notes in the PR description (or an issue): `ShutDownGlobal` (DB row problem, Section D), `UpdateMessageGUI` (K2 needs a message-GUI pointer whose owner is unknown), the benign rows (E), and the extra GUI destructors listed below.

PR3 (DB) side effects worth stating there: GOG DB `ShowUpgradeScreen` param_size 20 should be 24; GOG DB `ActionMenuSecurity` 0 and K1 DB 4 should be 8; K1 DB `DisplayMainMenu` 0 should be 4; both K2 `ShutDownGlobal` rows are a deleting destructor.

## Exact diffs

All edits are in `Patches/Common/GameAPI/`. `Fn` typedefs are in the `.h`, call sites in the `.cpp`.

### Commit 1

`CClientExoApp.h`
```diff
-    typedef void(__thiscall* DisplayMainMenuFn)(void* thisPtr);
+    typedef void(__thiscall* DisplayMainMenuFn)(void* thisPtr, int unused);   // engine pops 4: arg ignored by every target
-    typedef void(__stdcall* ExitProgramFn)();
+    typedef void(__thiscall* ExitProgramFn)(void* thisPtr);                  // thunk reads [this+4]
-    typedef int(__stdcall* GetPausedByCombatFn)();
+    typedef int(__thiscall* GetPausedByCombatFn)(void* thisPtr);
```
`CClientExoApp.cpp`
```diff
-    displayMainMenu(objectPtr);
+    displayMainMenu(objectPtr, 0);
 ...
-    // __stdcall global helper: does not take a 'this' pointer.
-    exitProgram();
+    exitProgram(objectPtr);
 ...
-    // __stdcall global helper: does not take a 'this' pointer.
-    return getPausedByCombat();
+    return getPausedByCombat(objectPtr);
```
(The `if (!objectPtr || ...)` guards already exist and stay.)

`CSWCObject.h/.cpp`
```diff
-    typedef void* (__thiscall* GetPortraitFn)(void* thisPtr, void* outResRef);
+    typedef void* (__thiscall* GetPortraitFn)(void* thisPtr, void* outResRef, BYTE size);
-    CResRef* GetPortrait(CResRef* outResRef);
+    CResRef* GetPortrait(CResRef* outResRef, BYTE size = 0);   // 0 = base resref, 1..5 = T,S,M,L,H suffix
-    void* resultPtr = getPortrait(objectPtr, outResRef ? outResRef->GetPtr() : nullptr);
+    void* resultPtr = getPortrait(objectPtr, outResRef ? outResRef->GetPtr() : nullptr, size);
```
(`CSWSObject::GetPortrait` is a different class and is not in scope.)

`CSWCPlaceable.h/.cpp`
```diff
-    typedef void(__thiscall* ActionMenuSecurityFn)(void* thisPtr, void* creature);
+    typedef void(__thiscall* ActionMenuSecurityFn)(void* thisPtr, void* creature, int unused);   // engine pops 8, reads neither
-    actionMenuSecurity(objectPtr, creature ? creature->GetPtr() : nullptr);
+    actionMenuSecurity(objectPtr, creature ? creature->GetPtr() : nullptr, 0);
```

`Gob.h/.cpp`
```diff
-    typedef void (__thiscall* EnableRenderBumpedOutFn)(void* thisPtr);
+    typedef void (__thiscall* EnableRenderBumpedOutFn)(void* thisPtr, const char* textureName, float strength);
-    void EnableRenderBumpedOut();
+    void EnableRenderBumpedOut(const char* textureName, float strength);
-    enableRenderBumpedOut(objectPtr);
+    enableRenderBumpedOut(objectPtr, textureName, strength);

-    typedef int (__thiscall* GetPartLocalPositionFn)(void* thisPtr, Vector* outPosition, Quaternion* outOrientation);
+    typedef int (__thiscall* GetPartLocalPositionFn)(void* thisPtr, const char* partName, Vector* outPosition, Quaternion* outOrientation);
-    int GetPartLocalPosition(Vector* outPosition, Quaternion* outOrientation);
+    int GetPartLocalPosition(const char* partName, Vector* outPosition, Quaternion* outOrientation);
-    return getPartLocalPosition(objectPtr, outPosition, outOrientation);
+    return getPartLocalPosition(objectPtr, partName, outPosition, outOrientation);
```
Both Gob methods change their public signature; nothing in the tree calls them. `float` args are pushed as one 4-byte slot under `__thiscall`, so the 8-byte pop matches.

### Commit 2 (title-gated; K1 typedefs and call paths stay exactly as they are today)

Pattern: add a second typedef and static pointer per function, resolved from the same DB name, and branch on `GameVersion::GetTitle() == GameTitle::KOTOR2` in the method. Both pointers can be assigned from the one `GetFunctionAddress` result.

```cpp
// CSWGuiPanel.h
typedef void (__thiscall* StartLoadFromLayoutK2Fn)(void* thisPtr, void* guiResref, int scaleA, int scaleB);
typedef void (__thiscall* InitControlK2Fn)(void* thisPtr, void* controlToInit, void* label, int activate, int scaleToViewport);
// StartLoadFromLayout(): K2 -> startLoadFromLayoutK2(objectPtr, guiResref ? guiResref->GetPtr() : nullptr, 1, 1);   // vanilla pushes (resref,1,1)
// InitControl():         K2 -> initControlK2(objectPtr, ctl, label, activate, 1);                                  // vanilla 4th arg is 1

// CGuiInGame.h
typedef void(__thiscall* ShowPartySelectionK2Fn)(void* thisPtr, void* exitScript, int requireAccept, int forcedNPC1, int forcedNPC2, int arg5);
// K2 -> showPartySelectionK2(objectPtr, exitScript, requireAccept, forcedNPC1, forcedNPC2, 0);   // arg5 = script's 4th param, default 0

struct ExoStringByValue { char* buf; DWORD len; };       // CExoString layout: {char* @0, uint @4}
typedef void(__thiscall* ShowUpgradeScreenK2Fn)(void* thisPtr, DWORD itemId, DWORD characterId,
                                                int disableItemCreation, int disableUpgrade, ExoStringByValue override2DA);
// K2 -> showUpgradeScreenK2(objectPtr, itemId, 0x7f000000 /*OBJECT_INVALID*/, 0, 0, ExoStringByValue{nullptr, 0});
//   the callee destroys the by-value string (MSVC), so pass {nullptr,0} or an engine-allocated buffer, never a stack char*.

// CRes.h / CResGFF.h / CSWGuiBorder.h
typedef void(__thiscall* DestructorK2Fn)(void* thisPtr, DWORD deleteFlag);   // CSWGuiBorder: returns void*
// ~CRes/~CResGFF/~CSWGuiBorder: K2 -> destructorK2(objectPtr, 0);   then the existing free(objectPtr). Flag 0 runs the dtor body without operator delete.
```
`CResGFF::Destructor_2` (dead): either delete the unused `destructor2` pointer/typedef, or give it a no-flag K2 typedef. I would delete it.

Not changed: `ShutDownGlobal`, `UpdateMessageGUI` (see Open questions). If you want a minimal safety net: in both wrappers add `if (GameVersion::GetTitle() == GameTitle::KOTOR2) return;`.

## Open questions and things the lead should re-verify

1. `ShutDownGlobal` on K2: no real function located. K1 0x62a970 frees four panels at this+0x98/9c/a0/a4 and zeroes +0x38; the K2 equivalent was not searched for.
2. `UpdateMessageGUI` on K2: arg is a pointer to the message GUI (callees 0x758e30/0x7592f0/0x759820, GOG 0x52a530 "AddMessages"). Vanilla passes 0 once and an object once. Not enough to pick a default.
3. Names of the new args are my labels from asm/script structure: `characterId/disableItemCreation/disableUpgrade/override2DA` (matches the TSL script ShowUpgradeScreen signature and the caller's defaults at 0x692e78-0x692e86; the TSL signature itself is from memory), `arg5` of ShowPartySelection, `scaleToViewport`, `strength`. Counts and widths are proven, names are not.
4. K1 `CreatureAcquireItem` is a tail-jump stub; its `ret 8` is inferred from the stub rewriting `[ESP+4]` and jumping, plus the DB. Benign either way.
5. `GetPortrait` default `size = 0`: sel 0 leaves the base resref unchanged in 0x85e8c0 (no `case 0`), and vanilla 0x8490e0 passes 0. Confirm that is the behaviour you want from a size-less call.
6. Other GUI wrappers have the same K2 Destructor problem but are outside the 22: GOG `fp.tsv` shows retn 4, size 44, callee operator delete (0x921321) for `CSWGuiImage` 0x914770, `CSWGuiListBox` 0x91b170, `CSWGuiScrollBar` 0x91a0a0, `CSWGuiSlider` (wrapper uses `Destructor_2`) 0x597840, `CSWGuiText` 0x915630. Not verified on Steam, not in verified.tsv. Candidate follow-up.
7. Nothing here was run in a game. No K1 binary or runtime exists on this machine; K1 evidence is the server's K1 GOG 1.03 program (names at all 22 DB addresses match, so the addresses line up with the 1.03 DBs).
8. The K1/K2 DB sig columns that disagree with the asm (K1 `DisplayMainMenu`, `ActionMenuSecurity`, `GetPartLocalPosition` arg label; GOG `ShowUpgradeScreen`, `ActionMenuSecurity`) belong to the DB PRs, not this one.

## Files

- `wrapper_fixes.tsv`: 22 rows, one per input row.
- `evidence/gog_k2_disasm.txt`, `evidence/k1_gog103_disasm.txt`: disassembly and decompile of every row address (and the thunk targets) from the server.
- `evidence/ExportDisasmList.java`, `evidence/run_disasm.py`: the read-only export used (run_disasm.py has the scratch path hard-coded).
- `evidence/va.py`: PE VA to string reader used for the portrait suffix strings; `evidence/db_names_query.py`: DB lookup.
