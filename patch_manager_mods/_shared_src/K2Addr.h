// K2Addr.h -- per-game-version engine address table for the shared KOTOR2 Patch Manager headers (header-only, no STL).
// One named field per engine function / global / vtable / version-sensitive struct offset that Effects.h, Gui2D.h, GuiKit.h,
// SceneView.h and GameAddr.h use. kSteam = the Steam Aspyr exe (the only build the mods were written against); kGog = the GOG Aspyr exe (filled from
// doc 21b, static-verified only, untested live). A ZERO FIELD MEANS UNKNOWN: a helper that needs it does nothing / returns its neutral
// value and never calls address 0.
//
// Selection: k2addr::get() reads KOTOR_VERSION_SHA once (KotorPatcher sets it before any patch DLL loads, from patch_config.toml
// target_version_sha). That value is the ORIGINAL exe hash KPM detected (GameDetector resolves it from kpm_install_state.json
// OriginalHash), not the hash of the live patched file: the live Steam exe is vanilla 6A522E... + 4gb-patch (7751F7D8...), and
// the env still carries 6A522E.... Compared case-insensitively against the full SHA-256 hex.
//   unset or empty -> kSteam (host tests, hot-swapped DLLs, current behaviour)
//   known sha      -> that table
//   unknown sha    -> nullptr (every helper is inert; never guess an address for a build we have not mapped)
// Host tests force a table with k2addr::setForTest(&table) / clearForTest().
//
// Steam values below are the pre-existing constants (GameAddr.h + the Addr* / offset constants that Effects.h, Gui2D.h and GuiKit.h
// carried); those headers still export them under their old names as "the Steam column" so existing mods keep compiling.
#pragma once
#include "GameAddr.h"
#include <stdint.h>
#include <stdlib.h>

struct K2AddrTable {
    const char* sha = "";   // full SHA-256 hex of the (original, unpatched) exe this table is for; "" = none

    // ---- engine globals / functions shared across mods (GameAddr.h) ----
    uintptr_t AppGlobal = 0;             // global app pointer; *(app)+4 -> client/server ptr A; used by 8 mods under 6 names
    uintptr_t GuiSoundMgr = 0;           // global GUI sound manager pointer
    uintptr_t PlayGuiSound = 0;          // thiscall(mgr, char idx), idx 1 = hover
    uintptr_t ScreenW = 0;               // engine framebuffer width (int)
    uintptr_t ScreenH = 0;               // engine framebuffer height (int)
    uintptr_t GuiRefW = 0;               // int (800): GUI layout reference space width
    uintptr_t GuiRefH = 0;               // int (600): GUI layout reference space height
    uintptr_t ProjectionFlag = 0;        // GUI projection mode flag
    uintptr_t New = 0;                   // cdecl(size) engine operator new
    uintptr_t LabelCtor = 0;             // CSWGuiLabel ctor, thiscall ECX only, no stack args, object size LabelSize
    uintptr_t InitControl = 0;           // thiscall(panel, ctl, CExoString* tag, addToList, scale) RET 16
    uintptr_t StopLoadFromLayout = 0;    // StopLoadFromLayout (createLabelFromTemplate must run BEFORE this)
    uintptr_t ExoCtor = 0;               // CExoString ctor, thiscall(this, const char*)
    uintptr_t ExoDtor = 0;               // CExoString dtor, thiscall(this)
    uintptr_t TextSet = 0;               // thiscall(label+0xF0, CExoString*)
    uintptr_t SetFont = 0;               // thiscall(label+0xF0, char resref[16]) RET 4
    uintptr_t SetAlign = 0;              // thiscall(label+0xF0, unsigned) RET 4
    uintptr_t TextColor = 0;             // thiscall(label+0xF0, float rgb[3]) RET 4
    uintptr_t PtrListAdd = 0;            // CExoArrayList<void*>::Add thiscall(list, v) RET 4
    uintptr_t GuiLoadImage = 0;          // cdecl(resref) -> new 8-byte image wrapper each call
    uintptr_t GetControlledCreature = 0; // thiscall(partyTable) -> controlled creature
    uintptr_t PartyGetAt = 0;            // party member accessor
    uintptr_t MemberServerCreature = 0;  // thiscall(client party member) -> server creature (CSWSCreature*)
    uintptr_t ClientPartyTable = 0;      // FUN_0073fb90(A) = *(*(A+4)+0x270), the party table getter
    uintptr_t LabelVtable = 0;           // CSWGuiLabel RTTI vtable
    uintptr_t BorderVtable = 0;          // CSWGuiBorder RTTI vtable
    uintptr_t TextVtable = 0;            // CSWGuiText RTTI vtable
    uintptr_t KeyboardDeviceGlobal = 0;  // rdata constant 0 (device-array index)
    uintptr_t TexPendingList = 0;        // engine pending-texture list

    // ---- Effects.h ----
    uintptr_t WorldTimerGetCurrentTime = 0;   // FUN_0051ad20 thiscall(timer, int* day, uint* ms) RET 8
    int AppGlobal_ToWorld = 0;           // *(*(AppGlobal)+8)   -> world object
    int World_ToSub = 0;                 // *(world+4)          -> sub object holding the timer
    int Sub_ToTimer = 0;                 // *(sub+0x10048)      -> world timer (0 = no world: main menu, load)
    int Creature_EffectArray = 0;        // creature (CSWSObject) CGameEffect** array
    int Creature_EffectCount = 0;        // creature int count (sane: 0..512)
    int Effect_Type = 0;                 // CGameEffect ushort type
    int Effect_Subtype = 0;              // CGameEffect ushort subtype
    int Effect_Duration = 0;             // float, total seconds
    int Effect_ExpiryDay = 0;            // int expiry day
    int Effect_ExpiryTime = 0;           // int expiry ms-of-day
    int Effect_Creator = 0;              // creator object id
    int Effect_SpellId = 0;              // spells.2da row
    int Effect_IntCount = 0;             // int count of the ints array
    int Effect_Ints = 0;                 // int* ints

    // ---- Gui2D.h ----
    uintptr_t BeginGui2D = 0;            // cdecl, no args: open the 2D batch (ortho matrix, viewport)
    uintptr_t EndGui2D = 0;              // cdecl, no args: close it
    uintptr_t PushClipRect = 0;          // cdecl(x, y, w, h, const float* color|ClipNoFillColor, flag, float alpha) -> nonzero if pushed
    uintptr_t PopClipRect = 0;           // cdecl, no args
    uintptr_t FlushGuiBatch = 0;         // cdecl(float = *(float*)FlushArg): draws the batched quads
    uintptr_t FlushArg = 0;              // float the engine passes to FlushGuiBatch
    uintptr_t HudScale = 0;              // cdecl -> float in st0
    uintptr_t ClipDepth = 0;             // clip stack index (low 16 bits): 0 in the HUD frame, 0xFFFF in menu frames
    uintptr_t ClipNoFillColor = 0;       // colour the engine's own icon row passes: clip only, no fill
    uintptr_t PanelRect = 0;             // thiscall(panel, int r[4])
    int ImageVt_Draw = 0;                // image wrapper vtable slot index of Draw

    // ---- GuiKit.h ----
    int LabelSize = 0;                   // sizeof(CSWGuiLabel)
    int Ctl_Flags = 0;                   // +flags: bit1 visible, bit5 click-through
    int Ctl_Id = 0;                      // +id (= index in panel list)
    int Ctl_X = 0, Ctl_Y = 0, Ctl_W = 0, Ctl_H = 0;   // extent = engine pixels x, y, w, h
    int Ctl_VtSetRect = 0;               // byte offset in the control vtable of slot 1 (SetRect)
    int Label_Border = 0;                // label +border sub-object (vtable BorderVtable)
    int Label_Text = 0;                  // label +text sub-object (vtable TextVtable)
    int Text_Renderer = 0;               // text +renderer
    int Text_StringSub = 0;              // text +string sub-object (= label+0xF0)
    int Text_Color = 0;                  // string sub-object +colour rgb
    int Border_Alpha = 0;                // border +alpha (float)
    int Border_Tint = 0;                 // border +tint rgb (3 floats)
    int Border_Fill = 0;                 // border +fill image*
    int Border_FillStyle = 0;            // border +FILLSTYLE in bits 0-1 (0 tile, 1 centre, 2 stretch)
    int Panel_CtlArray = 0;              // panel control list {ptr, count, cap}
    int Panel_CtlCount = 0;              // count inside that list

    // ---- portrait-slots / buff-duration-hud / shield-bar (party portrait slot, in-game menu, creature, tooltip) ----
    uintptr_t CExoStringFormat = 0;      // cdecl(CExoString* dst, const char* fmt, ...): sprintf into a CExoString (FUN_00734270)
    uintptr_t GetMaxForcePoints = 0;     // fastcall(creature) -> max Force points (FUN_0057eca0)
    int Slot_CardRect = 0;               // portrait slot +card rect (4 ints x, y, w, h)
    int Slot_Flags = 0;                  // slot +flags word: bit 1 = icon row visible
    int Slot_VitCtl = 0;                 // slot +embedded health-bar control (vtable ptr at +0)
    int Slot_VitRect = 0;                // slot +health-bar rect (4 ints)
    int Slot_ForceCtl = 0;               // slot +embedded force-bar control (vtable ptr at +0)
    int Slot_ForceRect = 0;              // slot +force-bar rect (4 ints)
    int Slot_ArrowBuff = 0;              // slot +buff arrow counter (unsigned char)
    int Slot_ArrowDebuff = 0;            // slot +debuff arrow counter (unsigned char)
    int Slot_Overlay0 = 0;               // slot +overlay flag word 0 (bit 1 = shown; FUN_00744f00 mask&2)
    int Slot_Overlay1 = 0;               // slot +overlay flag word 1 (mask&1)
    int Slot_Overlay2 = 0;               // slot +overlay flag word 2 (mask&4)
    int Menu_PortraitSlot = 0;           // CSWGuiInGameMenu +top-bar portrait slot (panel = slot - this)
    int Menu_Char2 = 0;                  // menu +LBL_CHAR2 control
    int Menu_Level2 = 0;                 // menu +LEVELUP2 control
    int Menu_Back2 = 0;                  // menu +BACK2 control
    int Menu_Change2 = 0;                // menu +BTN_CHANGE2 control
    int Menu_Char3 = 0;                  // menu +LBL_CHAR3 control
    int Menu_Level3 = 0;                 // menu +LEVELUP3 control
    int Menu_Back3 = 0;                  // menu +BACK3 control
    int Menu_Change3 = 0;                // menu +BTN_CHANGE3 control
    int MenuCaller_Creature = 0;         // frame offset from the FUN_00745770 caller's ebp: local holding the selected creature (negative)
    int Creature_Stats = 0;              // creature +stats object
    int Stats_ForceA = 0;                // stats +short, current Force part A (current FP = A + B)
    int Stats_ForceB = 0;                // stats +short, current Force part B
    int Creature_VtMaxHp = 0;            // creature vtable byte offset: max HP getter, thiscall(creature, 1) RET 4
    int Creature_VtCurHp = 0;            // creature vtable byte offset: current HP getter, thiscall(creature, 0) RET 4
    int Creature_IconCacheData = 0;      // creature +client effect-icon cache data ptr (diagnostic)
    int Creature_IconCacheCount = 0;     // creature +icon cache count (diagnostic)
    int IconEnt_F14 = 0;                 // icon cache entry +0x14 field (diagnostic; entry id short is at +0)
    int IconEnt_F1c = 0;                 // icon cache entry +0x1c field (diagnostic)
    int Border_Width = 0;                // CSWGuiBorder +width (0 = Draw returns at once)
    int Border_Height = 0;               // CSWGuiBorder +height
    int Border_Rot = 0;                  // CSWGuiBorder +rotation bits
    int Tip_Slot = 0;                    // portrait char button +portrait slot pointer
    int Tip_Text = 0;                    // portrait char button +tooltip CExoString

    // ---- SceneView.h (CSWGui3DSceneView block + its scene models; inventory-3d-viewport, equip preview) ----
    uintptr_t GuiGateGlobal = 0;         // int, non-zero once the GUI/3D layer is up
    uintptr_t GuiControlCtor = 0;        // CSWGuiControl ctor, thiscall ECX only
    uintptr_t GuiSceneCtor = 0;          // CSWGuiScene ctor, thiscall ECX = view+View_Scene
    uintptr_t SceneAddModel = 0;         // thiscall(scene, CExoString* resref, int idx) -> model
    uintptr_t SceneRemoveModel = 0;      // thiscall(scene, int idx, int deleteIt) RET 8
    uintptr_t SceneGetModel = 0;         // thiscall(scene, int idx) -> Gob*
    uintptr_t SceneViewVtable = 0;       // CSWGui3DSceneView RTTI vtable
    uintptr_t SceneCameraVtable = 0;     // camera object vtable (view+View_Camera)
    uintptr_t GobVtable = 0;             // scene model (Gob) vtable
    uintptr_t GobSetPosition = 0, GobSetOrientation = 0, GobGetPosition = 0, GobGetOrientation = 0, GobGetParent = 0;   // expected Gob vtable slot values
    int SceneViewSize = 0;               // bytes the mods allocate for a view block (>= the engine object)
    int View_Scene = 0;                  // view +CSWGuiScene sub-object
    int View_SceneObj = 0;               // view +Scene* (= scene sub-object +0x14)
    int View_Camera = 0;                 // view +camera object*
    int View_ModelCount = 0;             // view +model count (= scene sub-object +0x20)
    int SceneVt_Init = 0;                // Scene* vtable byte offset: thiscall(scene, const char* room, float pos[3], float quat[4])
    int GobVt_SetPosition = 0, GobVt_SetOrientation = 0, GobVt_GetPosition = 0, GobVt_GetOrientation = 0, GobVt_GetParent = 0;   // Gob vtable byte offsets
    int GobVtReadable = 0;               // bytes of the Gob vtable a guard must be able to read (covers every slot above)
    int GobReadable = 0;                 // bytes of the Gob a guard must be able to read
    int Gob_Scale = 0;                   // Gob +uniform scale (float, 1.0 = none)
};

// Steam Aspyr exe (vanilla 6A522E..., which is also what the live 4gb-patched file reports). Built by name so a new field
// can never be filled positionally into the wrong slot.
namespace k2addr { namespace detail {
constexpr K2AddrTable makeSteam() {
    K2AddrTable t{"6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF"};
    t.AppGlobal = gameaddr::AppGlobal;
    t.GuiSoundMgr = gameaddr::GuiSoundMgr;
    t.PlayGuiSound = gameaddr::PlayGuiSound;
    t.ScreenW = gameaddr::ScreenW;
    t.ScreenH = gameaddr::ScreenH;
    t.GuiRefW = gameaddr::GuiRefW;
    t.GuiRefH = gameaddr::GuiRefH;
    t.ProjectionFlag = gameaddr::ProjectionFlag;
    t.New = gameaddr::New;
    t.LabelCtor = gameaddr::LabelCtor;
    t.InitControl = gameaddr::InitControl;
    t.StopLoadFromLayout = gameaddr::StopLoadFromLayout;
    t.ExoCtor = gameaddr::ExoCtor;
    t.ExoDtor = gameaddr::ExoDtor;
    t.TextSet = gameaddr::TextSet;
    t.SetFont = gameaddr::SetFont;
    t.SetAlign = gameaddr::SetAlign;
    t.TextColor = gameaddr::TextColor;
    t.PtrListAdd = gameaddr::PtrListAdd;
    t.GuiLoadImage = gameaddr::GuiLoadImage;
    t.GetControlledCreature = gameaddr::GetControlledCreature;
    t.PartyGetAt = gameaddr::PartyGetAt;
    t.MemberServerCreature = gameaddr::MemberServerCreature;
    t.ClientPartyTable = gameaddr::ClientPartyTable;
    t.LabelVtable = gameaddr::LabelVtable;
    t.BorderVtable = gameaddr::BorderVtable;
    t.TextVtable = gameaddr::TextVtable;
    t.KeyboardDeviceGlobal = gameaddr::KeyboardDeviceGlobal;
    t.TexPendingList = gameaddr::TexPendingList;

    t.WorldTimerGetCurrentTime = 0x0051AD20;
    t.AppGlobal_ToWorld = 8;
    t.World_ToSub = 4;
    t.Sub_ToTimer = 0x10048;
    t.Creature_EffectArray = 0x148;
    t.Creature_EffectCount = 0x14c;
    t.Effect_Type = 0x08;
    t.Effect_Subtype = 0x0a;
    t.Effect_Duration = 0x0c;
    t.Effect_ExpiryDay = 0x10;
    t.Effect_ExpiryTime = 0x14;
    t.Effect_Creator = 0x18;
    t.Effect_SpellId = 0x1c;
    t.Effect_IntCount = 0x30;
    t.Effect_Ints = 0x34;

    t.BeginGui2D = 0x004792C0;
    t.EndGui2D = 0x004793F0;
    t.PushClipRect = 0x00479460;
    t.PopClipRect = 0x004799C0;
    t.FlushGuiBatch = 0x0047EA60;
    t.FlushArg = 0x009F5F60;
    t.HudScale = 0x00479AB0;
    t.ClipDepth = 0x00A309C0;
    t.ClipNoFillColor = 0x00A1BA3C;
    t.PanelRect = 0x00410020;
    t.ImageVt_Draw = 6;

    t.LabelSize = 0x148;
    t.Ctl_Flags = 0x48;
    t.Ctl_Id = 0x54;
    t.Ctl_X = 4;
    t.Ctl_Y = 8;
    t.Ctl_W = 0xC;
    t.Ctl_H = 0x10;
    t.Ctl_VtSetRect = 4;
    t.Label_Border = 0x60;
    t.Label_Text = 0xD8;
    t.Text_Renderer = 0x14;
    t.Text_StringSub = 0x18;
    t.Text_Color = 0x2C;
    t.Border_Alpha = 0x24;
    t.Border_Tint = 0x28;
    t.Border_Fill = 0x74;
    t.Border_FillStyle = 0x34;
    t.Panel_CtlArray = 0x24;
    t.Panel_CtlCount = 0x28;

    t.CExoStringFormat = 0x00734270;
    t.GetMaxForcePoints = 0x0057ECA0;
    t.Slot_CardRect = 0x54c;
    t.Slot_Flags = 0x590;
    t.Slot_VitCtl = 0x690;
    t.Slot_VitRect = 0x694;
    t.Slot_ForceCtl = 0x7fc;
    t.Slot_ForceRect = 0x800;
    t.Slot_ArrowBuff = 0xddc;
    t.Slot_ArrowDebuff = 0xddd;
    t.Slot_Overlay0 = 0x1b8;
    t.Slot_Overlay1 = 0x300;
    t.Slot_Overlay2 = 0x448;
    t.Menu_PortraitSlot = 0x2350;
    t.Menu_Char2 = 0x3134;
    t.Menu_Level2 = 0x327C;
    t.Menu_Back2 = 0x33C4;
    t.Menu_Change2 = 0x350C;
    t.Menu_Char3 = 0x36DC;
    t.Menu_Level3 = 0x3824;
    t.Menu_Back3 = 0x396C;
    t.Menu_Change3 = 0x3AB4;
    t.MenuCaller_Creature = -0x38;
    t.Creature_Stats = 0x1198;
    t.Stats_ForceA = 0x130;
    t.Stats_ForceB = 0x132;
    t.Creature_VtMaxHp = 0x98;
    t.Creature_VtCurHp = 0x9c;
    t.Creature_IconCacheData = 0xFFC;
    t.Creature_IconCacheCount = 0x1000;
    t.IconEnt_F14 = 0x14;
    t.IconEnt_F1c = 0x1c;
    t.Border_Width = 0xC;
    t.Border_Height = 0x10;
    t.Border_Rot = 0x20;
    t.Tip_Slot = 0x1d0;
    t.Tip_Text = 0x1d4;

    t.GuiGateGlobal = gameaddr::GuiGateGlobal;
    t.GuiControlCtor = gameaddr::GuiControlCtor;
    t.GuiSceneCtor = gameaddr::GuiSceneCtor;
    t.SceneAddModel = gameaddr::SceneAddModel;
    t.SceneRemoveModel = gameaddr::SceneRemoveModel;
    t.SceneGetModel = gameaddr::SceneGetModel;
    t.SceneViewVtable = gameaddr::SceneViewVtable;
    t.SceneCameraVtable = gameaddr::SceneCameraVtable;
    t.GobVtable = gameaddr::GobVtable;
    t.GobSetPosition = gameaddr::GobSetPosition;
    t.GobSetOrientation = gameaddr::GobSetOrientation;
    t.GobGetPosition = gameaddr::GobGetPosition;
    t.GobGetOrientation = gameaddr::GobGetOrientation;
    t.GobGetParent = gameaddr::GobGetParent;
    t.SceneViewSize = 0x400;
    t.View_Scene = 0x60;
    t.View_SceneObj = 0x74;
    t.View_Camera = 0x78;
    t.View_ModelCount = 0x80;
    t.SceneVt_Init = 0x70;
    t.GobVt_SetPosition = 0x5C;
    t.GobVt_SetOrientation = 0x60;
    t.GobVt_GetPosition = 0x64;
    t.GobVt_GetOrientation = 0x68;
    t.GobVt_GetParent = 0x108;
    t.GobVtReadable = 0x10c;
    t.GobReadable = 0x200;
    t.Gob_Scale = 0x1e0;
    return t;
}

} } // namespace k2addr::detail

inline constexpr K2AddrTable kSteam = k2addr::detail::makeSteam();
// GOG Aspyr exe (777BEE23..., target_versions of every kotor2-gog-aspyr.hooks.toml, GameDetector, address DB game_version).
// Source: patch_manager_mods/21b_gog_address_map.md section 4 (static-verified only: every function pair instruction-aligned to
// the Steam body, every struct offset byte-identical; nothing here has run on a live GOG process). A ZERO FIELD MEANS UNKNOWN:
// KeyboardDeviceGlobal (hotbar only, no GOG pair found) and the inventory-3D Scene/Gob/View/GuiGate fields (out of scope for
// portrait-slots / buff-duration-hud / shield-bar) stay 0, so those helpers are inert on GOG. ClipNoFillColor and TexPendingList
// are "high" in 21b: each quotes an aligned absolute operand (PUSH 0xa1ba3c -> 0xa768a4 in RowDraw; MOV ECX,0xa1bc2c -> 0xa14cd4 in
// three functions) but has no second semantic anchor, so they need the live GOG check.
namespace k2addr { namespace detail {
constexpr K2AddrTable makeGog() {
    K2AddrTable t{"777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914"};
    t.AppGlobal = 0x00A11C04;
    t.GuiSoundMgr = 0x00A11BFC;
    t.PlayGuiSound = 0x00910720;
    t.ScreenW = 0x009F22A4;
    t.ScreenH = 0x009F22A8;
    t.GuiRefW = 0x00A0FFE8;
    t.GuiRefH = 0x00A0FFEC;
    t.ProjectionFlag = 0x00A76858;
    t.New = 0x00921347;
    t.LabelCtor = 0x009179E0;
    t.InitControl = 0x0090DAA0;
    t.StopLoadFromLayout = 0x0090DA20;
    t.ExoCtor = 0x00605680;
    t.ExoDtor = 0x00605890;
    t.TextSet = 0x00915200;
    t.SetFont = 0x009151B0;
    t.SetAlign = 0x00915370;
    t.TextColor = 0x00915510;
    t.PtrListAdd = 0x00534170;
    t.GuiLoadImage = 0x008D5470;
    t.GetControlledCreature = 0x004789B0;
    t.PartyGetAt = 0x004789D0;
    t.MemberServerCreature = 0x00429780;
    t.ClientPartyTable = 0x0040D070;
    t.LabelVtable = 0x0098998C;
    t.BorderVtable = 0x009A994C;
    t.TextVtable = 0x009A9A28;
    t.KeyboardDeviceGlobal = 0;   // UNKNOWN (zero = helper inert)
    t.TexPendingList = 0x00A14CD4;   // high
    t.WorldTimerGetCurrentTime = 0x0080B200;
    t.AppGlobal_ToWorld = 0x8;
    t.World_ToSub = 0x4;
    t.Sub_ToTimer = 0x10048;
    t.Creature_EffectArray = 0x148;
    t.Creature_EffectCount = 0x14C;
    t.Effect_Type = 0x8;
    t.Effect_Subtype = 0xA;
    t.Effect_Duration = 0xC;
    t.Effect_ExpiryDay = 0x10;
    t.Effect_ExpiryTime = 0x14;
    t.Effect_Creator = 0x18;
    t.Effect_SpellId = 0x1C;
    t.Effect_IntCount = 0x30;
    t.Effect_Ints = 0x34;
    t.BeginGui2D = 0x008CFBD0;
    t.EndGui2D = 0x008CFD00;
    t.PushClipRect = 0x008CFD70;
    t.PopClipRect = 0x008D02D0;
    t.FlushGuiBatch = 0x008D5370;
    t.FlushArg = 0x009F7144;
    t.HudScale = 0x008D03C0;
    t.ClipDepth = 0x00A32B90;
    t.ClipNoFillColor = 0x00A768A4;   // high
    t.PanelRect = 0x0090E4A0;
    t.ImageVt_Draw = 0x6;
    t.LabelSize = 0x148;
    t.Ctl_Flags = 0x48;
    t.Ctl_Id = 0x54;
    t.Ctl_X = 0x4;
    t.Ctl_Y = 0x8;
    t.Ctl_W = 0xC;
    t.Ctl_H = 0x10;
    t.Ctl_VtSetRect = 0x4;
    t.Label_Border = 0x60;
    t.Label_Text = 0xD8;
    t.Text_Renderer = 0x14;
    t.Text_StringSub = 0x18;
    t.Text_Color = 0x2C;
    t.Border_Alpha = 0x24;
    t.Border_Tint = 0x28;
    t.Border_Fill = 0x74;
    t.Border_FillStyle = 0x34;
    t.Panel_CtlArray = 0x24;
    t.Panel_CtlCount = 0x28;
    t.CExoStringFormat = 0x00606380;
    t.GetMaxForcePoints = 0x006977D0;
    t.Slot_CardRect = 0x54C;
    t.Slot_Flags = 0x590;
    t.Slot_VitCtl = 0x690;
    t.Slot_VitRect = 0x694;
    t.Slot_ForceCtl = 0x7FC;
    t.Slot_ForceRect = 0x800;
    t.Slot_ArrowBuff = 0xDDC;
    t.Slot_ArrowDebuff = 0xDDD;
    t.Slot_Overlay0 = 0x1B8;
    t.Slot_Overlay1 = 0x300;
    t.Slot_Overlay2 = 0x448;
    t.Menu_PortraitSlot = 0x2350;
    t.Menu_Char2 = 0x3134;
    t.Menu_Level2 = 0x327C;
    t.Menu_Back2 = 0x33C4;
    t.Menu_Change2 = 0x350C;
    t.Menu_Char3 = 0x36DC;
    t.Menu_Level3 = 0x3824;
    t.Menu_Back3 = 0x396C;
    t.Menu_Change3 = 0x3AB4;
    t.MenuCaller_Creature = -0x38;
    t.Creature_Stats = 0x1198;
    t.Stats_ForceA = 0x130;
    t.Stats_ForceB = 0x132;
    t.Creature_VtMaxHp = 0x98;
    t.Creature_VtCurHp = 0x9C;
    t.Creature_IconCacheData = 0xFFC;
    t.Creature_IconCacheCount = 0x1000;
    t.IconEnt_F14 = 0x14;
    t.IconEnt_F1c = 0x1C;
    t.Border_Width = 0xC;
    t.Border_Height = 0x10;
    t.Border_Rot = 0x20;
    t.Tip_Slot = 0x1D0;
    t.Tip_Text = 0x1D4;
    // GuiGateGlobal: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GuiControlCtor: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GuiSceneCtor: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneAddModel: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneRemoveModel: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneGetModel: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneViewVtable: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneCameraVtable: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GobVtable: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GobSetPosition: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneViewSize: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // View_Scene: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // View_SceneObj: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // View_Camera: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // View_ModelCount: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // SceneVt_Init: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GobVt_SetPosition: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GobVtReadable: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // GobReadable: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    // Gob_Scale: out of scope (not used by portrait-slots / buff-duration-hud / shield-bar; added to K2Addr.h after this work started)
    return t;
}
} } // namespace k2addr::detail
inline constexpr K2AddrTable kGog = k2addr::detail::makeGog();
namespace k2addr {
namespace detail {
inline bool ieq(const char* a, const char* b) {
    for (;; ++a, ++b) {
        char x = *a, y = *b;
        if (x >= 'a' && x <= 'z') x = static_cast<char>(x - 'a' + 'A');
        if (y >= 'a' && y <= 'z') y = static_cast<char>(y - 'a' + 'A');
        if (x != y) return false;
        if (!x) return true;
    }
}
inline const K2AddrTable*& forced() { static const K2AddrTable* p = nullptr; return p; }
inline bool& isForced() { static bool f = false; return f; }
}

// Pure selection: null / empty -> Steam, a known full SHA-256 (any case) -> its table, anything else -> nullptr.
inline const K2AddrTable* selectBySha(const char* sha) {
    if (!sha || !*sha) return &kSteam;
    if (detail::ieq(sha, kSteam.sha)) return &kSteam;
    if (detail::ieq(sha, kGog.sha)) return &kGog;
    return nullptr;
}
// Selection from the environment, uncached (get() caches this once).
inline const K2AddrTable* selectFromEnv() { return selectBySha(getenv("KOTOR_VERSION_SHA")); }

// The table for this process, chosen once on first use. nullptr = unknown exe: do nothing.
inline const K2AddrTable* get() {
    if (detail::isForced()) return detail::forced();
    static const K2AddrTable* t = selectFromEnv();
    return t;
}

// Test seam: force the table (nullptr forces "unknown exe"); clearForTest() goes back to the environment selection.
inline void setForTest(const K2AddrTable* t) { detail::forced() = t; detail::isForced() = true; }
inline void clearForTest() { detail::forced() = nullptr; detail::isForced() = false; }

// Engine-struct address fields of the shared headers (g_fx.appGlobal, g_g2d.flushArgAddr, ...) default to this: "use the table's
// address". A test (or mod) that sets the field to an explicit value, 0 included, overrides the table.
constexpr uintptr_t UseTable = ~static_cast<uintptr_t>(0);

// ---- field access for the helpers (0 = unknown) ----
inline uintptr_t addr(uintptr_t K2AddrTable::* m) { const K2AddrTable* t = get(); return t ? t->*m : 0; }
inline int off(int K2AddrTable::* m) { const K2AddrTable* t = get(); return t ? t->*m : 0; }
// Engine function pointer of type F, or nullptr when the address is unknown.
template <typename F> inline F fnptr(uintptr_t K2AddrTable::* m) {
    uintptr_t a = addr(m);
    return a ? reinterpret_cast<F>(a) : nullptr;
}

// A feature runs only when the exe's address table has every field it needs (null table / 0 = unknown). Returns ok; the first
// !ok call per `*noted` flag logs ONE line (a mod keeps one `static bool noted` per session) and sets it. `log` is the mod's
// printf-style logger (call from the hook, never from KPatchInit); a null `log` is safe, a null `noted` logs on every !ok call.
// Text is shield_bar's probe line: "K2ADDR <feature> disabled (no address table for this exe, or a needed field is 0)".
inline bool featureOk(bool ok, const char* feature, void (*log)(const char* fmt, ...), bool* noted) {
    if (ok) return true;
    if (noted && *noted) return false;
    if (noted) *noted = true;
    if (log) log("K2ADDR %s disabled (no address table for this exe, or a needed field is 0)", feature ? feature : "?");
    return false;
}

} // namespace k2addr
