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
constexpr uintptr_t BorderVtable = 0x009875BC;          // CSWGuiBorder RTTI vtable
constexpr uintptr_t TextVtable = 0x009876B4;            // CSWGuiText RTTI vtable
constexpr uintptr_t KeyboardDeviceGlobal = 0x0099C884;  // rdata constant 0 (device-array index)
constexpr uintptr_t TexPendingList = 0x00A1BC2C;        // engine pending-texture list
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
constexpr uintptr_t HudVtable = 0x0099DCC4;              // CSWGuiInGame (HUD panel) RTTI vtable
constexpr uintptr_t ServerIdMask = 0x0099449C;           // global: server id = client id & ~[mask]
constexpr uintptr_t UseItemRouteA = 0x0057D8B0;          // thiscall(server creature; 0,0,selfServerId,0,itemServerId,0,0,0): logged by the probe, never called

} // namespace gameaddr
