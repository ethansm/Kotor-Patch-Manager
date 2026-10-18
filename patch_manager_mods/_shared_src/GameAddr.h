// GameAddr.h -- one canonical name per engine address shared across KOTOR2 mods (header-only, no dependencies).
// Every constant below is the STEAM Aspyr column (swkotor2.exe 6A522E..., also what the 4gb-patched live file reports).
// They stay compile-time constants so existing mods keep building, but code that must also run on GOG Aspyr reads the same
// names from the per-version table instead: K2Addr.h defines struct K2AddrTable (one field per name here, same spelling, plus the
// Effects.h / Gui2D.h / GuiKit.h addresses and struct offsets), kSteam (built from these constants), kGog (unfilled, all zero)
// and k2addr::get() (selected at runtime from KOTOR_VERSION_SHA; zero field = unknown = do nothing).
// See patch_manager_mods/19b_cross_patch_sharing_survey.md section D.
#pragma once
#include <stdint.h>

namespace gameaddr {

constexpr uintptr_t AppGlobal = 0x00A1B4A4;             // global app pointer; *(app)+4 -> client/server ptr A; used by 8 mods under 6 names
constexpr uintptr_t GuiSoundMgr = 0x00A1B49C;           // global GUI sound manager pointer                Mod3 78
constexpr uintptr_t PlayGuiSound = 0x004122A0;          // thiscall(mgr, char idx), idx 1 = hover          Mod3 78
constexpr uintptr_t ScreenW = 0x009F42A4;               // engine framebuffer width (int)                  Mod14 syncScreen 218
constexpr uintptr_t ScreenH = 0x009F42A8;               // engine framebuffer height (int)                 Mod14 syncScreen 218
constexpr uintptr_t GuiRefW = 0x009F4390;               // int (800): GUI layout reference space width
constexpr uintptr_t GuiRefH = 0x009F4394;               // int (600): GUI layout reference space height
constexpr uintptr_t ProjectionFlag = 0x00A1B9F0;        // GUI projection mode flag
constexpr uintptr_t New = 0x00919723;                   // cdecl(size) engine operator new                 Mod3 FnNew 52
constexpr uintptr_t LabelCtor = 0x00419740;             // CSWGuiLabel ctor, thiscall ECX only, no stack args, object size 0x148   Mod3 FnLabelCtor 49
constexpr uintptr_t ButtonCtor = 0x0041A510;            // CSWGuiButton ctor, __fastcall(this) = ECX only, no stack args (dump decomp/41/0041a510.c:3)
constexpr uintptr_t InitControl = 0x0040F620;           // thiscall(panel, ctl, CExoString* tag, addToList, scale) RET 16   Mod3 50
constexpr uintptr_t StopLoadFromLayout = 0x0040F5A0;    // StopLoadFromLayout (createLabelFromTemplate must run BEFORE this)
constexpr uintptr_t ExoCtor = 0x00733570;               // CExoString ctor, thiscall(this, const char*)    Mod3 51
constexpr uintptr_t ExoDtor = 0x00733780;               // CExoString dtor, thiscall(this)                 Mod3 51
constexpr uintptr_t TextSet = 0x00416E30;               // thiscall(label+0xF0, CExoString*)               Mod3 FnTextSet 77
constexpr uintptr_t SetFont = 0x00416DE0;               // thiscall(label+0xF0, char resref[16]) RET 4     Mod3 970
constexpr uintptr_t SetAlign = 0x00416FA0;              // thiscall(label+0xF0, unsigned) RET 4            Mod3 971
constexpr uintptr_t TextColor = 0x00417140;             // thiscall(label+0xF0, float rgb[3]) RET 4        Mod14 FnTextColor 47
constexpr uintptr_t PtrListAdd = 0x0083EA60;            // CExoArrayList<void*>::Add thiscall(list, v) RET 4   Mod3 969
constexpr uintptr_t GuiLoadImage = 0x0047EB60;          // cdecl(resref) -> new 8-byte image wrapper each call (not "LoadImage": a windows.h macro)  Mod3 53 / Mod14 46
constexpr uintptr_t GetControlledCreature = 0x007E5D80; // thiscall(partyTable) -> controlled creature
constexpr uintptr_t PartyGetAt = 0x007E5DA0;            // party member accessor
constexpr uintptr_t MemberServerCreature = 0x0077D800;  // thiscall(client party member) -> server creature (CSWSCreature*); 0x77caa4 in 0x77C780
constexpr uintptr_t ClientPartyTable = 0x0073FB90;      // FUN_0073fb90(A) = *(*(A+4)+0x270), the party table getter
constexpr uintptr_t LabelVtable = 0x009878BC;           // CSWGuiLabel RTTI vtable
constexpr uintptr_t ButtonVtable = 0x00987A1C;          // CSWGuiButton vtable, size 0x1D0 (dump decomp/41/0041a510.c writes it)
constexpr uintptr_t BorderVtable = 0x009875BC;          // CSWGuiBorder RTTI vtable
constexpr uintptr_t TextVtable = 0x009876B4;            // CSWGuiText RTTI vtable
constexpr uintptr_t KeyboardDeviceGlobal = 0x0099C884;  // rdata constant 0 (device-array index)
constexpr uintptr_t TexPendingList = 0x00A1BC2C;        // engine pending-texture list
constexpr uintptr_t TexBasicCtor = 0x00423000;           // CAurTextureBasic ctor, thiscall(this 0xF4 B, name, name) RET 8; registers the name globally
constexpr uintptr_t EngineNewArr = 0x0091D288;          // engine operator new[] (FreeImage releases +0x44 with delete[] when +0xE7 == 0)
constexpr uintptr_t TexListAdd = 0x004491C0;            // thiscall(pendingList, basic): pending-build list add, consumed by the texture tick
constexpr uintptr_t TexReset = 0x00423990;              // CAurTextureBasic::Reset, fastcall(basic)
// ---- CSWGui3DSceneView (SceneView.h; Mod 1 Inventory3DViewport.cpp 31-60 / Mod 5 EquipCharPreview.cpp 28-62) ----
constexpr uintptr_t GuiGateGlobal = 0x009F42B4;         // int, non-zero once the GUI/3D layer is up (inv3d/equip refuse a scene view while 0)
constexpr uintptr_t GuiControlCtor = 0x00418990;        // CSWGuiControl ctor, thiscall ECX only
constexpr uintptr_t GuiSceneCtor = 0x00417F40;          // CSWGuiScene ctor, thiscall ECX = view+0x60
constexpr uintptr_t SceneAddModel = 0x00418360;         // thiscall(view+0x60, CExoString* resref, int idx) -> model
constexpr uintptr_t SceneRemoveModel = 0x00418430;      // thiscall(view+0x60, int idx, int deleteIt) RET 8
constexpr uintptr_t SceneGetModel = 0x004184B0;         // thiscall(view+0x60, int idx) -> Gob*
constexpr uintptr_t SceneViewVtable = 0x009A3D34;       // CSWGui3DSceneView RTTI vtable
constexpr uintptr_t SceneCameraVtable = 0x0098C45C;     // scene camera object vtable (view+0x78)
constexpr uintptr_t GobVtable = 0x0098B5CC;             // scene model (Gob) vtable
constexpr uintptr_t GobSetPosition = 0x00461490;        // Gob vtable slot 0x5C
constexpr uintptr_t GobSetOrientation = 0x004614E0;     // Gob vtable slot 0x60
constexpr uintptr_t GobGetPosition = 0x00459710;        // Gob vtable slot 0x64
constexpr uintptr_t GobGetOrientation = 0x00459740;     // Gob vtable slot 0x68
constexpr uintptr_t GobGetParent = 0x00461530;          // Gob vtable slot 0x108

// ---- companion-selfheal probe / ItemRead.h (doc 27) ----
constexpr uintptr_t ClientToServerObjectId = 0x0051C8B0; // thiscall(objTable, clientId) RET 4 -> server object id
constexpr uintptr_t GetItemByObjectId = 0x0051C0E0;      // thiscall(objTable, serverId) RET 4 -> item ptr (0 = none)
// ---- Action Hotbar (hotbar-only engine entry points / globals) ----
constexpr uintptr_t HudSyncStance = 0x0074DA40;          // fastcall(ECX=hud), no stack args, plain RET: resyncs hud+0x1C8C (stance) / +0x1C90 (form) from the creature
constexpr uintptr_t HudInvokeGroup = 0x00751750;         // thiscall(hud, group 0..6, 0) RET 8
constexpr uintptr_t PerformUseItem = 0x00775600;         // thiscall(controlled client creature; clientItemId, 0, targetClientId, float* pos) RET 0x10, void
constexpr uintptr_t PerformCastSpell = 0x00775700;       // thiscall(controlled client creature; spellId, arg2, targetClientId, multispell, metamagic, domain, float* pos) RET 0x1C, void
constexpr uintptr_t CloudSavesFlag = 0x00A7EA88;         // Steam Cloud byte (set -> SAVES: alias = cloudsaves dir, else saves/)
constexpr uintptr_t CloudSavesDir = 0x00A7EABC;          // CExoString "cloudsaves/<steam id>/"
// ---- holster (doc 42): static RE, Steam only ----
constexpr uintptr_t ClientCombatStateSet = 0x007682B0;   // thiscall(client creature, newState) -> old; stores this+0x138
constexpr uintptr_t WeaponAttach = 0x007637E0;           // cdecl(u32 obj, char* tag, int* creature): in-world rhand/lhand attach, RET 0
constexpr uintptr_t SwapWeaponsExec = 0x005DF920;        // thiscall(action executor): swaps 0x10<->0x40000, 0x20<->0x80000
constexpr uintptr_t AnimResolve = 0x00863650;            // thiscall(client creature; u16 animId): ready-idle / wield-transition row resolver
constexpr uintptr_t AnimSend = 0x005644B0;               // thiscall(client creature; animId, pos[3], speed): anim message sender
constexpr uintptr_t GobUpdateDependent = 0x005047B0;     // thiscall(Gob): UpdateDependentPosition copy of the attach-node transform
constexpr uintptr_t GobAttachByName = 0x00461670;        // thiscall(parent Gob; node name, kind): named-node attach, miss = silent no-attach
constexpr uintptr_t HudVtable = 0x0099DCC4;             // CSWGuiInGame (HUD panel) RTTI vtable
// Panels that do not block gameplay hotkeys (RTTI vtables; doc 07 gate whitelist; shared by the hotbar/selfheal HUD gates)
constexpr uintptr_t MessagesPanelVtable = 0x0099E5EC;    // CSWGuiMessages
constexpr uintptr_t FadePanelVtable = 0x009A044C;        // fade panel
constexpr uintptr_t PausePanelVtable = 0x009A7BAC;       // pause panel
constexpr uintptr_t AutoPausePanelVtable = 0x009A8FCC;   // auto-pause panel
constexpr uintptr_t ServerIdMask = 0x0099449C;           // global: server id = client id & ~[mask]
constexpr uintptr_t UseItemRouteA = 0x0057D8B0;          // thiscall(server creature; 0,0,selfServerId,0,itemServerId,0,0,0) RET 0x20: called only by companion-selfheal 0.2.0-probe with [probe] call=1
constexpr uintptr_t CanUseItem = 0x005D2F80;             // thiscall(server creature; itemPtr,0,1) RET 0xC -> nonzero = usable; verified B12b
constexpr uintptr_t ActListRemoveHead = 0x007F2A90;      // thiscall(ECX = cr+0x100 action list) RET 0 -> data ptr of the head, node freed (R8 rotate pop; string-anchored thunk)
constexpr uintptr_t ActListAddTail = 0x0054AA30;         // thiscall(ECX = cr+0x100 action list; push data) RET 4: append at the tail (R8 rotate push; body-anchored thunk)
constexpr uintptr_t RoundHasPending = 0x00590110;        // thiscall(ECX = [cr+0x10DC] round object) RET 0 -> nonzero if pending round nodes (GetActionPending, R8 Q4 rotate gate)
constexpr uintptr_t ServerClearAllActions = 0x0051CB10;    // CServerExoApp::ClearAllActions thiscall(ECX = *(*AppGlobal + 8), serverObjectId) RET 4: empties cr+0x100 and the round list (what vanilla Cancel Combat reaches); verified B9, doc 30 R7
constexpr uintptr_t RmbHoldFlag = 0x00A7FCBC;          // DAT_00A7FCBC: RMB-hold flag; the camera director (0x78C350) follows mouse deltas while it is nonzero
constexpr uintptr_t RButtonDownHoldSite = 0x007B3817;    // PerformRButtonDownAction 0x7B3810: MOV [RmbHoldFlag],1 (10 bytes); companion-selfheal detours it (R6 swallow)

// ---- addresses shared by several mods under different local names (F20; canonical names, mods still hold their own copies) ----
constexpr uintptr_t GetLocString = 0x007356B0;           // CExoLocString::GetString (REA 20261015): bool thiscall(ECX = handle, int language, CExoString* out, uint8 gender = 0) RET 0xC; free out with ExoDtor; inventory-sort item name (item+0x2C0)
constexpr uintptr_t GetClientLanguage = 0x0073F350;      // __fastcall(ECX = *(*(AppGlobal 0xA1B4A4)+4)) -> int language for GetLocString (REA Q2, caller 0x8BF52B-0x8BF54F)
constexpr uintptr_t GetBaseItem = 0x006D6E30;            // CSWItem::GetBaseItem (verified B0) thiscall(itemPtr), no stack args -> baseitems row ptr (slot mask +4, gender req byte +0xB4); action-hotbar GetBaseItem, inventory-sort FnGetBaseItem
constexpr uintptr_t ServerObjectGetArea = 0x005453C0;    // CSWSObject::GetArea (name_map, medium conf) thiscall(server object) -> area ptr (0 = none); companion-ai / ranged-ai FnGetArea
constexpr uintptr_t AppActiveFlag = 0x00A1B4E0;          // int: engine app-active flag; AppActivated 0x40BCF0 sets it 1, AppDeactivated 0x40BC40 clears it; MainLoop's load-screen dismiss tests it; activation-fix / load-hang-probe FlagA1B4E0
constexpr uintptr_t MovieRestoreSkipFlag = 0x00A1B76C;   // int: set to 1 by the movie thread (0x40D637); RestoreAfterMovie 0x40BF40 then skips its re-activation (compared with 0); activation-fix / load-hang-probe Gate76C
constexpr uintptr_t ClientCreatureVtable = 0x0099EE14;   // CSWCCreature vtable: the client creature ctor 0x7645F0 stores it at [this] (mov [ecx],0x99EE14); CharEffectsPanel ClientCreatureVftable == EquipCharPreview CreatureVftable
constexpr uintptr_t WorldTimerGetCurrentTime = 0x0051AD20; // CWorldTimer::GetWorldTime (verified B0) thiscall(timer, int* day, uint* ms) RET 8; mirrors K2Addr Effects.h field

// ---- in-memory GFF resources (EmbedGui.h; holster_edit D2F, static only) ----
constexpr uintptr_t ResManGlobal = 0x00A1B490;           // DWORD: CExoResMan* (every resman caller loads [0xA1B490]; perf-lab G_ResMan)
constexpr uintptr_t ResGffCtor = 0x00716FD0;             // CResGFF::Constructor_2 (verified B0) thiscall(obj; type, ext, resref) RET 12, object 0xA0
constexpr uintptr_t ResManMalloc = 0x00712F30;           // CExoResMan::Malloc (verified B0) thiscall(resman; CRes*): allocates the data buffer at obj+0x10
constexpr uintptr_t ResGffOnServiced = 0x00717300;       // CResGFF::OnResourceServiced (verified B0) fastcall(obj): parses obj+0x10 in place, 1 ok / 0 fail
constexpr uintptr_t ResManFreeData = 0x007120C0;         // CExoResMan::FreeResourceData (verified B0) thiscall(resman; CRes*)
constexpr uintptr_t ResRelease = 0x007112B0;             // CRes::Release (R1-static, high) fastcall(obj)

constexpr uintptr_t OptionsInGameVtable = 0x009A701C;    // in-game Options panel vtable: ctor 0x8A1170 writes it at 0x8A11BA (static asm); holster-editor filters CSWGuiPanel::Draw on it

// ---- 3D character preview (equip3d EquipCharPreview.cpp, live since 10-05; preview-3d provider copies the recipe) ----
constexpr uintptr_t ClientGetItemById = 0x0073F530;      // CClientExoApp::GetItemByGameObjectID (verified B11a) thiscall(mgr = [[AppGlobal]+4], id) -> client item*
constexpr uintptr_t ClientItemGetName = 0x007FDFA0;      // CSWCItem::GetName (static facts: 618 B, RET 8, caller CClientExoAppInternal::GetObjectName 0x78EBA0; doc 11 R14) thiscall(clientItem; CExoString* out, int identified); probe only (holster-editor U0). identified=0 copies item+0x18C (side-effect free, may be empty); identified=1 can take a lookup branch via 0x71DF90 (strref->achievement switch, unchecked 0x6D5200(item+0xC) deref: NULL on a bad base-item row), so call it with 0 unless probing
constexpr uintptr_t CreateModelGob = 0x00462320;         // model-based Gob creator (REA holster C5) cdecl(name, inst, 0, 0) -> Gob*
constexpr uintptr_t PreviewModelInstName = 0x009A4AB0;   // instance-name string passed to CreateModelGob (equip3d, live)
constexpr uintptr_t ClientCreatureCtor = 0x007645F0;     // CSWCCreature ctor (REA holster C6) fastcall ECX = block (0x470 bytes)
constexpr uintptr_t ClientCreatureSetAppearance = 0x0076D200;   // CSWCCreature::SetAppearance (name_map medium) thiscall(preview, copy60, flags, 1, 0) RET 0x10
constexpr uintptr_t PreviewSetAlign = 0x0085D430;        // thiscall(preview[0x89], int align) RET 4 (equip3d, live)
constexpr uintptr_t ClientCreatureSetup3DScene = 0x0077EC20;    // CSWCCreature::Setup3DScene (name_map medium) thiscall(preview, view, real, 0, 1) RET 0x10

// ---- in-game Options panel description path (holster-editor 0.1.1-probe, card c13; static dump reads, not yet live-verified) ----
constexpr uintptr_t OptionsSetDescription = 0x008A2320;  // Options panel description fn: thiscall ECX=panel, ONE stack arg = TLK strref (int, -1 = none), RET 4 (dump decomp/8a/008a2320.asm:36-39,134)
constexpr uintptr_t ExoStringAssign = 0x007337C0;        // CExoString::operator= thiscall(dst, CExoString* src) RET 4; CExoString = {char* ptr, uint cap}, dump decomp/73/007337c0.c
constexpr uintptr_t GuiTextObjRefresh = 0x00417800;      // fastcall(ECX = text holder): when holder+0x14/+0x10/+0xC are all set, calls [holder+0x14]->vt[0x4C](-1); tail of 416EA0 (dump decomp/41/00416ea0.asm 416f68, 41/00417800.c)

} // namespace gameaddr
