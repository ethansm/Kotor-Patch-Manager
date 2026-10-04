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
