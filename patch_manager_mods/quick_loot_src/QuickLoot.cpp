// Quick Loot 0.7.2-dev = 0.7.1 + hook H4 on the target-arrow draw 0x0074FB20: skipped while the list is up (QL14; NEW HOOK = KPM reinstall, not a hot-swap).
// Quick Loot 0.7.1-dev = 0.7.0 + page registered at init (it missed the Options build), four page rows (Item tooltip, Quest items first, Compare with equipped, Panel background), FOOT / EQUIP-MISS logs.
// Quick Loot 0.7.0-dev (hot-swap, no hook changes) = 0.6.1 + the QL9..QL13 review set: QL9 tooltip redesign (icon, title + category line, rules from the
//   baseitems row + the engine's own property text 0x00614E10, flavor, foot; layout and rules are pure: QuickLootLogic.h TipDraw / QuickLootRules.h), QL10 stack
//   column (right-aligned "x2" per row), QL11 sorted rows (plot first, credits second "Credits (N)", then container order; clicks / tooltips address items by
//   object id, never by UI row), QL12 GUI sounds (hover 1, take 9, scrollbar 0; quick_loot.ini Sounds), QL13 tooltip foot ("Equipped: ..." / "In inventory: N"),
//   panel underlays (second uibit_fill_2bt under the list and the tooltip: 0.95 opacity). quick_loot.ini: Rules=0 skips the property lines (base rules only),
//   Sounds=0 mutes. New logs: SORT, PROPTXT, RULES-MISS, EQUIP, SOUND, TLK-EMPTY.
// Quick Loot 0.6.0-dev = 0.5.1-dev + four user-picked features (doc 52, 10-28):
//   QL2A anchor: the block position is no longer latched when it first shows (it drifted when the camera turned). K2HUD_TARGET_MENU_POST runs after the engine
//   positioned the target name tag (0x743850) and before drawing, so onMenuPost re-places the block EVERY frame from the current name rect. quick_loot.ini
//   Anchor: 0 = under the name tag (default; block top = name bottom + the gap that clears the health bar, centred on the name, clamped to the panel, flipped
//   above the name at the bottom), 1 = below the container (the container's server position projected with the scene camera's MapToScreen, camera vtable slot
//   0x28 = 0x4809D0 RET 0x14, normalized = 1, x / y times the GUI size shorts at [[[G+4]+0x274]+0x6C / +0x6E]: the same math as GetMenuPoint 0x77E970 without
//   its +1.75 lift; behind-camera, NaN and off-panel results fall back to mode 0), 2 = beside the name tag (left = name right + gap, top = name top, flips to
//   the left of the tag at the panel's right edge). With no name rect this frame the last one is kept NameHoldMs (250 ms), then the block hides (modes 0 / 2;
//   the Took list keeps its last rect until it expires). Logs: ANCHOR (change-only: wanted mode, used mode, reason), PROJ (first 40 different points + every
//   verdict change: name rect, projected px, ret, why) so a live session can calibrate mode 1. LABEL-LATCH is gone.
//   QL4 per-item take: a left click on an item row of the PEEK list (rows are click-eating labels only while the peek list is up) takes that one item with the
//   server CloseInventory drain sequence for the clicked index (engineTakeOne): ItemListGetItem, RemoveFromRepo 0x605E80, the first-pickup branch for base class
//   0x18 (a silent local-boolean 0x5E set on "lockertsfinv"; the pop-up case is refused, F runs the real drain), GetItemRepository 0x58A2C0(1), InsertOrMerge
//   0x605090, SetOwner 0x600380. HUD frame only, only with the open flag +0x390 == 1 and the leader within CloseM. The header flashes "Acquired: <name>" for 1.5 s;
//   an emptied container is closed the normal way (Close flag 0, once). Logs: TAKE1 / TAKE1-REFUSED. F still takes all.
//   QL8 tooltip: 300 ms on one row shows a vanilla-framed box beside the list (right, flipped left at the panel edge, top = the row, clamped): item name (16x16 when
//   it fits), "Quantity: N", the description (item+0x2B0 DescIdentified through GetLocString like the name; word-wrapped, 10 lines max with "..."). The engine has no
//   function for the stats / properties lines (u03.md B), so none are shown. Log: TIP (once per hover target, 20 max: lines + the body label's extent).
//   QL7 Mod Configuration page (QuickLootPage.h, selfheal pattern): List position, Auto-open, List rows, Open distance; all live. TakeKey stays ini-only (the
//   declarative layer has no key-binding row). New imports (ini writer): REINSTALL.
// Structure (PV1 step 3): the peek / take decision machine lives in QuickLootMachine.h (host-simulated by quick_loot_sim_test.cpp); this file is the
//   engine adapter (EngineEnv: SafeRead reads, OpenInventory / Close calls, label, toast) plus the three hooks.
// Quick Loot 0.5.0-dev = 0.4.0-dev + QL2V (doc 52): the list looks and behaves like the vanilla INVENTORY list / EQUIPMENT picker. Each item row is a
//   bordered protoitem button (a label cloned from the HUD template LBL_MOULDING3: uibit_fill_2bt + uibit_brdr_16bet edges + uibit_brdr_16bct corners,
//   teal tint; the hovered row = the vanilla HILIGHT, the same images in the near-white-green tint (0.929, 1, 0.929), name text cream) of RowH 64 px
//   (was 32) with RowGapPx between rows, an item icon square at the left (engine GetIcon 0x00615CC0 on the server item, resref copied next to the
//   name in captureItems, texture through guikit::imageForRetry; a missing texture = an empty icon box) and a scrollbar at the right, only while the
//   list has more rows than ListRows: a teal-outlined dark track, a thumb sized visible/total and placed by the offset, an up arrow (uibit_uparrowg) and
//   a down arrow (lbl_mapdown, tinted teal: there is no vanilla down-arrow texture, the engine rotates the up arrow of its SCROLLBAR control, which a
//   label cannot do). No HUD template carries the 16g scrollbar border (uibit_brdr_16gc / 16ge), so the scrollbar is fill-only labels (a teal
//   uibit_fill_16g outline label under a dark uibit_fill_2bt inset). Clicks: an arrow = one row, the track above / below the thumb = one page. The
//   scrollbar column is the only click-eating (non-click-through) label; the click itself is detected the way action-hotbar does (guikit::clickEdge =
//   LMB edge with the game window foreground + the cursor in engine px against the drawn rects), so the world never gets a click on the bar. The wheel
//   still works. New log lines: ICON (first 32 icon lookups per session), SBCLICK.
// Quick Loot 0.5.1-dev = 0.5.0-dev + menu gate fix (HUD-on-top panel check, GATE log) + vanilla wording "[F] Get Items (N)" / "Acquired Items (N)".
// Quick Loot 0.4.0-dev = 0.3.1-dev + QL2L, the multi-row item list (doc 52): peek and Took draw ONE block = header ("[F] Take all (25)" / "Took (25)",
//   cream) + up to ListRows item rows (teal, one per item, names cut at 40 chars, " xN" stacks, the hovered row cream) on a vanilla frame panel
//   (backing label created from the HUD template LBL_MOULDING3 / LBL_MENUBG: uibit_fill_2bt + uibit_brdr_16bet edges + uibit_brdr_16bct corners,
//   teal tint, alpha 1.0 over the fill's own 0.78; LBL_QUEUE1 + a flat black fill only when the layout has neither), font dialogfont10x10. More rows than fit: the mouse
//   wheel over the block scrolls it one row per 120 (gui-input k2.guiinput@1 wheel subscription; consumed only while the block is up, scrollable,
//   drawn within 250 ms and the cursor is inside it, else the camera zoom still gets it) and the header row shows "1-8 of 25" in dim teal.
//   Without gui-input the first rows show and nothing scrolls (WHEEL-SUB unavailable logged once). Items past the 64 captured count as one last
//   "+K more" row. The Took list stays 3 s + 0.5 s per list row past 3 (cap 6 s) and its timer is held while the cursor is over the block. Labels:
//   frame, header, indicator and 12 row labels made once per K2HUD_BUILD (no destroy API); if any row label fails the old one-line text is drawn in
//   the header label (fallback). Config: quick_loot.ini [QuickLoot] ListRows=8 (1..12). New log lines: LIST (once per new list), SCROLL (each change),
//   WHEEL-SUB (token or failure); LABEL-EXTENT now covers the header and the first row; LABEL-LATCH shows the block rect. Layout and text rules are
//   pure (QuickLootLogic.h, host-tested); the scroll / hover / Took-hold state lives in the machine (QuickLootMachine.h, sim-tested).
// Quick Loot 0.3.1-dev = 0.3.0-dev + the PV1 UX review (.claude/reviews/quick_loot_pv1_ux.md): peek and Took share one 1200 px / 56-char one-line
//   label ("[F] Take all (4): Medpac x2, Blaster Pistol, +2 more" / "Took (4): ..."), item names cut at 20 chars, a black alpha-0.55 backing
//   panel behind the text (second label made first in K2HUD_BUILD, same pill image + tint + alpha path as enemy-effects-strip), the label
//   position (A under the name tag, x clamped to the panel / B fixed spot) latched until the label is gone, a 400 ms dwell before the
//   auto-open, Took shown 3 s (3.5 s for a long line, cap 4 s), the label's real getExtent logged once (LABEL-EXTENT), and a give-up fallback
//   (GIVEUP-SERVERCLOSE) so a container is never left open when the client message object never comes.
// Quick Loot 0.3.0-dev (doc 52 QL2p on QL1b on QL1a). QL2p proximity peek: every ScanMs (K2HUD_FRAME) the leader's area is scanned for a
//   placeable within OpenM (HasInventory, not locked, no armed trap, useable, closed, not in the cooldown set); the nearest is opened with the
//   engine's OpenInventory (lid animation on), its ShowContainerGui is consumed as a PEEK, and a label lists the contents ("3 items  [F] take
//   all: Medpac x2, ..."). TakeKey (rising edge) sends the take-all Close; leaving (> CloseM, combat / pause / menu / dialog / load, area
//   change, container gone) sends Close(flag 0) once (items stay). A peeked container sits in a cooldown set until the leader is > CloseM
//   away. OpenInventory / Close are only ever called from the FRAME tick, never from the EventHandler / CloseInventory hooks.
//   QL1a fix: ShowContainerGui's flag means "had items at Show", not loot mode -- flag 0 is consumed too; the Close flag is chosen at send
//   time (items > 0 ? 1 : 0; an empty container gets Close 0, TAKE-EMPTY, no toast).
// QL1b: clicking a loot container takes everything at once, no container menu, then a one-line
//   "Took: Medpac x2, Blaster Pistol, +3 more" toast shows under the target name tag for 3000 ms (label made in K2HUD_BUILD, placed in
//   K2HUD_TARGET_MENU_POST, hidden from POST / FRAME when expired). Item names are captured right before the Close is sent (TOOK log lines).
//   QlShowContainer consumes ShowContainerGui (return 1) when Enabled, hud-host is connected and Shift is up; the container is then
//   "pending". QlPlaceableEvent notes its OnOpen (event 0x16). The hud-host FRAME tick sends the vanilla OK-button message (0x0087A550, Close
//   flag 1 = take all) 2 frames after OnOpen, or 1000 ms after Show when no OnOpen came. Close is never called from inside the EventHandler hook.
//   Anything else returns 0 (vanilla menu). Shift held = vanilla menu. Config: quick_loot.ini [QuickLoot] (read once at init):
//   Enabled=1, ProximityEnabled=1, OpenM=3.0, CloseM=5.0 (>= OpenM+1), TakeKey=70 (VK, 'F'), ScanMs=250, ListRows=8 (1..12), Anchor=0 (0 under the name tag / 1 below the container / 2 beside the name tag; 0.6.0-dev).
// Below: the QL0 probe description (all its log lines are kept). Questions it answered (patch_manager_mods/52_quick_loot.md):
//   1. does loot that OnOpen generates (queued event 0x16) reach the container before the player's Close?   (SHOW n vs CLOSE n, EVT 0x16 dt)
//   2. timing: ms between Show, event 0x16 and Close.                                                        (dt= on EVT / CLOSE lines)
//   3. does take-all (Close flag 1) fire OnInvDisturbed (event 0x1B)?                                         (EVT id=1b after CLOSE flag=1)
// Hooks read ECX / [ESP+4..] (KPM passes the game's stack: [esp+0] = return address). Log lines: QuickLootLogic.h fmt*.
#define INIW_FILE_API 1    // ModConfigDecl.h ini-bound rows write through IniWrite.h's file API (doc 46); before the first include of it
#define MCDECL_FILE_API 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "KpLog.h"
#include "HudHost.h"
#include "GuiKit.h"
#include "GuiInput.h"        // k2input::Client: wheel subscription + cursor in engine px (provider gui-input)
#include "QuickLootLogic.h"
#include "QuickLootMachine.h"
#include "QuickLootRules.h"
#include "SafeRead.h"
#include "GameAddr.h"
#include "ActQueue.h"          // actq::playerPaused
#include "CombatTraceRead.h"   // ctrace::readHudCombat
#include "ProviderClient.h"    // k2pc::RegisterOnce (Mod Configuration page)
#include "ModConfigDecl.h"
#include "QuickLootPage.h"
#include "QuickLootBezel.h"
#include "EmbedTex.h"
#include "QuickLootBezelTex.inc"
#include "PreviewCanvas.h"
#include "IniWrite.h"
#include <math.h>
#include <stdlib.h>

KP_LOG_PATCH("quick-loot");

namespace {

constexpr const char* Version = "0.8.0-dev";
constexpr unsigned Site = 0x7100;   // saferead failure-ring site id
// Steam addresses (static RE, doc 52 / .claude/research/quick_loot/README.md); AppGlobal and ClientToServerObjectId come from GameAddr.h.
constexpr uintptr_t GetPlaceableByGameObjectID = 0x0051C160;   // thiscall(serverApp, serverId) RET 4 -> CSWSPlaceable* (0 = none)
constexpr uintptr_t ItemListGetItem = 0x00606100;              // thiscall(repo, index) RET 4 -> CSWSItem* (0 = none)
constexpr uintptr_t EventClassGlobal = 0x0099244C;             // DAT_0099244c: the EventHandler's first argument is compared with it
constexpr int Plc_Open = 0x390, Plc_Repo = 0x3C4;              // open flag (byte), item repository pointer
constexpr int Repo_Array = 0x0C, Repo_Count = 0x10;            // object-id array, count
constexpr int Item_Stack = 0x2CC;                              // CSWSItem stack size (int)
// 0.5.0-dev (QL2V, .claude/waves/w4-52-QL2V/u02.md): GetIcon = thiscall(item, char out[16]) RET 4 (writes a NON-terminated 16-byte CResRef "i<itemclass>_<nnn>");
// it calls GetBaseItem (item -> base via the index at [item+0xC]) with no NULL check on item, so the item is probed first. Name_map: CSWSItem::GetIcon /
// CSWItem::GetBaseItem (Steam DB batch B0, verified); the repository items are the same pointers the server CloseInventory hands to GetBaseItem.
constexpr uintptr_t ItemGetIcon = 0x00615CC0;
constexpr uintptr_t ItemGetBaseItem = gameaddr::GetBaseItem;       // thiscall(item) RET 0 -> base item* (0 = none)
constexpr int Item_BaseIdx = 0xC;                                // base item index (dword)
constexpr size_t Item_IconProbe = 0x2B0;                         // item bytes the icon path reads (variation bytes at +0x2A4 / +0x2A6)
constexpr size_t BaseItem_Probe = 0x80;                          // base item bytes read (resref prefix at +0x3C, flags +0x18 / +0x74)
constexpr unsigned SiteIcon = 0x7130;                            // saferead failure-ring site id
constexpr int Item_Name = 0x2C0;                               // QL1b: CExoLocString (inventory-sort reads it on client items; UNVERIFIED on server CSWSItem)
// QL1b: target menu (menu = ctx->menu = hud + 0xCC): flags bit0 = shown, origin px, name label rect (panel-local px). Same offsets as enemy-effects-strip.
constexpr int Menu_Flags = 0x1BAC, Menu_OriginX = 0x1648, Menu_OriginY = 0x164C;
constexpr int Menu_NameX = 0x166C, Menu_NameY = 0x1670, Menu_NameW = 0x1674, Menu_NameH = 0x1678;
constexpr int NameCap = qlogic::NameCap;                       // captured name buffer (rowName cuts rows at 40; the one-line fallback at ToastNameMax = 20)
const char* const TexBacking = "lbl_miscroll_f";               // backing panel image: the enemy-effects-strip pill image (tinted black here)
// QL1a: vanilla OK button path (0x8B2850): msgObj = *(*(*(*AppGlobal + 4) + 4) + 0x150); thiscall(msgObj, clientObjId, closeFlag) RET 8.
constexpr uintptr_t SendContainerClose = 0x0087A550;
constexpr int App_Client = 4, Client_Inner = 4, Inner_MsgObj = 0x150;
constexpr uintptr_t SetMouseCursor = gameaddr::SetMouseCursor; // CClientExoAppInternal::SetMouseCursor thiscall(app, id, a, b) RET 0xC (doc 12 M7-R1)
constexpr int Inner_HoverObj = 0x4B0;            // CClientExoAppInternal + 0x4B0: hovered object id (doc 12 / doc 18)
constexpr int Plc_LocName = 0x268;               // CSWSPlaceable::GetFirstName -> CExoLocString at plc+0x268
// ---- QL2p. Evidence files: .claude/waves/w3-52-R2/u01.md (placeable fields), u02.md (leader / area list), u03.md (suppress helpers).
constexpr uintptr_t GetGameObject = 0x0051C0A0;     // thiscall(serverApp, objId) RET 4 -> object* or 0 (u02 recipe A step 6; companion_ai CompanionAI.cpp:81)
constexpr uintptr_t OpenInventory = 0x005F5590;     // CSWSPlaceable::OpenInventory thiscall(plc, openerServerObjId, playAnim) RET 8 (Opus 5f/005f5590.c; u01 "OpenInventory checks")
constexpr uintptr_t CloseInventory = 0x005F5770;   // CSWSPlaceable::CloseInventory thiscall(plc, playerServerId, flag) RET 8 (hook H3 target; QL0 verified prologue)
constexpr uintptr_t DialogByte = 0x00A309BC;        // global byte: dialog active (SetGlobalDialogState 0x7CEB80 writer; u03 (b), V11.md:22)
constexpr int Plc_Locked = 0x2A0;                   // byte; u01 field table (save 5f/005f18c0.c:98 param_1[0xa8] & 0xff)
constexpr int Plc_HasInv = 0x37C, Plc_Useable = 0x380;   // dwords; u01 (save :99 param_1[0xdf], :83 param_1[0xe0])
constexpr int Plc_Trap = 0x2C8;                     // dword TrapFlag; u01 (save :79 param_1[0xb2])
constexpr int Obj_Id = 0x04, Obj_Type = 0x08, Obj_PosX = 0x94, Obj_PosY = 0x98;   // u02 recipe A/B (type 9 = placeable; position inferred, SCAN log confirms)
constexpr int Area_Ids = 0x194, Area_Count = 0x198; // area object-id array and count; u02 recipe B (GetFirstObjectInArea 0x523250 reads them)
constexpr int ObjTypePlaceable = 9;
constexpr int Inner_PartyTable = 0x270;             // [client+4]+0x270: probed before the party-table getter (CompanionAI.cpp:361)
constexpr unsigned SiteWorld = 0x7110, SitePause = 0x7120;   // saferead sites (pause uses +0..+3)
// ---- 0.6.0-dev (QL4 per-item take): the server CloseInventory drain (asm 0x5F5770..0x5F5A5A, .claude/waves/w5-52-QL2A/u03.md A1), call by call.
constexpr uintptr_t GetCreatureObj = 0x0051C600;     // thiscall(serverApp, objId) RET 4 -> object (asm 0x5F57CF: ECX = [[AppGlobal]+8], arg = the opener's server id)
constexpr uintptr_t CreatureHolder = 0x00632A20;     // fastcall(object) RET 0 -> the object found through [object+0x28] (0 = none) (asm 0x5F57E4)
constexpr int Holder_VtCtx = 0x30;                   // vtable slot of that holder: thiscall(holder), no stack args -> ctx (asm 0x5F57FD..0x5F5800)
constexpr uintptr_t GetItemRepository = 0x0058A2C0;  // thiscall(ctx, 1) RET 4 -> the destination item repository (asm 0x5F5A33..0x5F5A38)
constexpr uintptr_t RepoRemove = 0x00605E80;         // thiscall(repo, item) RET 4 (asm 0x5F5852..0x5F585E)
constexpr uintptr_t RepoInsert = 0x00605090;         // thiscall(dstRepo, CSWSItem** itemInOut, 1, 1, 1) RET 0x10 (asm 0x5F5A29..0x5F5A3F)
constexpr uintptr_t ItemSetOwner = 0x00600380;       // thiscall(item, ownerServerId, 1, 1, 0x7F000000) RET 0x10 (asm 0x5F5A4A..0x5F5A5A)
constexpr uintptr_t VarGetBool = 0x005E9210;         // thiscall(item + 0x124, bit) RET 4 -> 0 / 1 (asm 0x5F58C9..0x5F58D1)
constexpr uintptr_t VarSetBool = 0x005E9260;         // thiscall(item + 0x124, bit, value) RET 8 (asm 0x5F58AC..0x5F58B9)
constexpr int Item_Vars = 0x124, Item_Desc = 0x2B0;  // local variable table; DescIdentified (CExoLocString; +0x2B8 Description, +0x2C0 LocalizedName)
constexpr int Base_NoticeClass = 0xAC;               // base item byte compared with 0x18 by the drain (asm 0x5F5871)
constexpr int Plc_Tag = 0x18;                        // the placeable's CExoString tag (asm 0x5F5899: ADD ECX,0x18), compared with "lockertsfinv"
constexpr unsigned SiteTake = 0x7140, SiteProj = 0x7150;
// ---- 0.6.0-dev (QL2A mode 1): the scene camera's MapToScreen (.claude/waves/w5-52-QL2A/u01.md, holster-editor U4 K contract). G = [[AppGlobal]+4]; A = [G+4];
// camera = [[A+0x18]+0x40]; GUI size shorts at [[A+0x274]+0x6C / +0x6E]. Camera vtable slot 0x28 must still be 0x4809D0 (RET 0x14) before it is called.
constexpr uintptr_t CamMapToScreen = 0x004809D0;
constexpr int Cam_Pos = 0xA8, Cam_Quat = 0xB4, Cam_Fovy = 0x204, Cam_Vp = 0x218, CamVt_MapToScreen = 0x28;
constexpr int Gui_W = 0x6C, Gui_H = 0x6E, Inner_Gui = 0x274, Inner_CamHolder = 0x18, CamHolder_Cam = 0x40;
constexpr int Obj_PosZ = 0x9C;                       // server object z (inferred next to +0x94 / +0x98; the PROJ line logs the world point)
// ---- 0.7.0-dev (QL9/QL11/QL13). Evidence: .claude/waves/w7-52-QL9/r01.md (Q5 property builder), u01 (baseitems / itempropdef), Steam DB names.
constexpr int Item_Plot = 0x10C;                     // CSWSItem Plot byte (probable: the item GFF writer stores the "Plot" field from *(item+0x10C) & 0xff)
constexpr int BaseId_Credits = 57;                   // baseitems row 57 = credits (the stack is the amount)
constexpr int Item_PropCount = 0x288, Item_PropArray = 0x290, Prop_Stride = 0x1C;   // item property array: u16 name +0, u16 subtype +2, u8 costTable +4, u16 costValue +6, u8 param1 +8, u8 param1Value +9
constexpr unsigned PropsMax = 16;
constexpr size_t BaseItem_Rules = 0xB0;              // base item bytes read for the rules: +0 name strref, +0x0C damage flags, +0x1C..0x1F dice / die / crit, +0x8A BaseAC, +0xAD DexBonus
constexpr int Creature_Inventory = 0x1150;           // CSWSCreature -> CSWInventory*
constexpr uintptr_t ItemPropBuilder = 0x00614E10;    // CSWSItem::GetPropertyStrings (name_map B0): __thiscall-like, ECX unused, 9 stack args, RET 0x24
constexpr uintptr_t C2daGetString = 0x0071B1D0;      // C2DA::GetCExoStringEntry_3: thiscall(2DA, int row, CExoString* col, CExoString* out) RET 12, nonzero = found
constexpr uintptr_t TlkGetString = 0x007408F0;       // CClientExoApp::GetGUIString: thiscall(client, CExoString* out, int strref) RET 8
constexpr uintptr_t InvGetItemInSlot = 0x006D0620;   // CSWInventory::GetItemInSlot: thiscall(inv, u32 slotMask) RET 4 -> item or 0
constexpr uintptr_t RulesGlobal = 0x00A1B4D0;        // [[[G]+0x130]+0x3C] = the itempropdef 2DA
constexpr int Rules_ItemPropDef1 = 0x130, Rules_ItemPropDef2 = 0x3C;
constexpr uint32_t SlotMainHand = 0x10, SlotBody = 0x2;   // inventory slot masks (0x2 body armor: probable)
const char* const IniPath = ".\\quick_loot.ini";   // ".\\" prefix: GetPrivateProfile*A with a bare name looks in the Windows dir

KP_CHANNEL(chMain, "main");

qlogic::Tracker g_t;
qlogic::Machine g_m;   // the peek / take decision machine (QuickLootMachine.h); config copied into g_m.cfg at init
qlogic::Take& g_take = g_m.take;
qlogic::Peek& g_peek = g_m.peek;
bool g_proxOn = true;
float g_openM = qlogic::DefaultOpenM, g_closeM = qlogic::DefaultCloseM;
int g_takeKey = qlogic::DefaultTakeKey;
uint32_t g_scanMs = qlogic::DefaultScanMs;
k2hud::Client g_hud;
bool g_enabled = true;
bool g_started = false;
LARGE_INTEGER g_freq;
bool g_timeInit = false;

// 0.8.0-dev (QL14): Hardware Bezels & Unified Header globals
bool g_hardwareBezels = true;
bool g_peekOnSelect = false;      // QL2: peek on select (quick_loot.ini peek_on_select)
uint32_t g_clickTimeoutMs = 0;    // QL2: auto-close timeout in ms (quick_loot.ini click_peek_timeout, 0 = off)
bool g_hoverPeek = true;          // QL3: hover peek (quick_loot.ini HoverPeek)
bool g_emptyCursor = true;        // QL3: empty container cursor indicator (quick_loot.ini EmptyCursor)
uint32_t g_hoverDwellMs = 150;    // QL3: hover dwell before open (ms)
qlbezel::CategoryStyle g_catStyles[qlbezel::CatCount];
int g_selectedCat = 0;
uint32_t g_modalPage = 0;
int g_gearItem = -1;
void* g_pagePanel = nullptr;
pvc::Canvas g_cv;
bool g_pvOn = false;
char g_targetName[80] = {0};
bool g_actionBtnClicked = false;
char g_giveTags[512] = {0};   // QL6: comma-/semicolon-separated tags to route to vanilla menu

inline void ensureBezelTextures() {
    static bool s_done = false;
    if (!s_done) {
        s_done = embtex::ensure(kBezelTex, kBezelTexCount);
    }
}

template <typename T> bool rd(uintptr_t base, int off, T* out) { return saferead::readAt(Site, base, off, out); }

uint32_t nowMs() {
    if (!g_timeInit) { QueryPerformanceFrequency(&g_freq); g_timeInit = true; }
    LARGE_INTEGER t; QueryPerformanceCounter(&t);
    return static_cast<uint32_t>(static_cast<unsigned long long>(t.QuadPart) * 1000ull / static_cast<unsigned long long>(g_freq.QuadPart));
}

void enter() {
    saferead::beginScope();
    if (!g_started) { g_started = true; if (KpLog_HasCore()) KP_LOG(chMain, "START quick-loot %s", Version); }
}

using LookupFn = uint32_t(__thiscall*)(uintptr_t self, uint32_t arg);

// *(*AppGlobal + 8) = the server app / object table (same hop as ItemRead.h serverItemFor).
uintptr_t serverApp() {
    uint32_t app = 0, srv = 0;
    if (!rd<uint32_t>(gameaddr::AppGlobal, 0, &app) || !app || !rd<uint32_t>(app, 8, &srv)) return 0;
    return srv;
}

uintptr_t placeableFor(uint32_t clientId) {
    uintptr_t srv = serverApp();
    if (!srv) return 0;
    uint32_t serverId = reinterpret_cast<LookupFn>(gameaddr::ClientToServerObjectId)(srv, clientId);
    if (!serverId) return 0;
    return reinterpret_cast<LookupFn>(GetPlaceableByGameObjectID)(srv, serverId);
}

// Item count of the placeable's repository, -1 when unreadable. *repoOut receives the repository pointer.
int itemCount(uintptr_t plc, uintptr_t* repoOut = nullptr) {
    uint32_t repo = 0; int n = 0;
    if (!plc || !rd<uint32_t>(plc, Plc_Repo, &repo) || !repo || !rd<int>(repo, Repo_Count, &n)) return -1;
    if (repoOut) *repoOut = repo;
    return n;
}

int openFlag(uintptr_t plc) { uint8_t o = 0; return rd<uint8_t>(plc, Plc_Open, &o) ? static_cast<int>(o) : -1; }

void logLine(const char* s) { KP_LOG(chMain, "%s", s); }
bool g_verbose = false;   // quick_loot.ini Verbose=1: QL0 probe lines (ITEM/EVTRAW/EVT/SCAN); default 0 keeps the log short (doc 52 QLR)
void probeLine(const char* s) { if (g_verbose) logLine(s); }

using SendCloseFn = void(__thiscall*)(uintptr_t msgObj, uint32_t objId, int closeFlag);

// The client message object, SafeRead at each hop. 0 = a hop failed.
uintptr_t messageObject() {
    uint32_t app = 0, client = 0, inner = 0, msg = 0;
    if (!rd<uint32_t>(gameaddr::AppGlobal, 0, &app) || !app) return 0;
    if (!rd<uint32_t>(app, App_Client, &client) || !client) return 0;
    if (!rd<uint32_t>(client, Client_Inner, &inner) || !inner) return 0;
    if (!rd<uint32_t>(inner, Inner_MsgObj, &msg) || !msg) return 0;
    return msg;
}

// ---- QL1b: item names + the toast label
using GetLocStringFn = bool(__thiscall*)(void*, int, void*, unsigned char);   // CExoLocString::GetString(this, language, CExoString* out, gender) RET 0xC
using GetClientLanguageFn = int(__fastcall*)(int);                            // ECX = *(*AppGlobal + 4)
using ExoDtorFn = void(__thiscall*)(void*);

// Client language for GetString (same chain as inventory-sort clientLanguage). -1 when the app chain is unreadable.
int clientLanguage() {
    uint32_t app = 0, client = 0, inner = 0;
    if (!rd<uint32_t>(gameaddr::AppGlobal, 0, &app) || !app || !rd<uint32_t>(app, App_Client, &client) || !client) return -1;
    if (!rd<uint32_t>(client, Client_Inner, &inner) || !inner) return -1;   // GetClientLanguage reads *(*(client+4)+0x148)
    return reinterpret_cast<GetClientLanguageFn>(gameaddr::GetClientLanguage)(static_cast<int>(client));
}

// Localised item text: the CExoLocString at item+`off` (0x2C0 name, 0x2B0 DescIdentified) through GetString, copy of inventory-sort readItemName. Leaves out[0] = 0
// (unknown) when the handle / language is unusable, GetString fails or the text is unreadable. The engine out string is always freed with the CExoString dtor.
void readItemText(uintptr_t item, int off, int lang, char* out, int cap) {
    out[0] = 0;
    const uintptr_t handle = item + off;
    uint32_t first = 0, probe = 0;
    // GetString dereferences [handle+0] with no null check: require a readable pointer there (server items are unverified for these offsets).
    if (!item || lang < 0 || !rd<uint32_t>(handle, 0, &first) || !first || !rd<uint32_t>(first, 0, &probe)) return;
    int exo[2] = {0, 0};   // CExoString {char* p; uint32 len}
    const bool ok = reinterpret_cast<GetLocStringFn>(gameaddr::GetLocString)(reinterpret_cast<void*>(handle), lang, exo, 0);
    if (ok && exo[0]) {
        static char tmp[qlogic::TipDescCap + 16];
        if (saferead::readExo(Site, reinterpret_cast<uintptr_t>(exo), tmp, static_cast<int>(sizeof tmp), static_cast<int>(sizeof tmp) - 1, false)) {
            int k = 0;
            while (tmp[k] && k < cap - 1) { out[k] = tmp[k]; ++k; }
            out[k] = 0;
        }
    }
    if (exo[0]) reinterpret_cast<ExoDtorFn>(gameaddr::ExoDtor)(exo);
}
void readItemName(uintptr_t item, int lang, char* out, int cap) { readItemText(item, Item_Name, lang, out, cap); }
void readPlaceableName(uintptr_t plc, int lang, char* out, int cap) { readItemText(plc, Plc_LocName, lang, out, cap); }

using Captured = qlogic::Captured;

using IconFn = void*(__thiscall*)(uintptr_t item, char* outResRef16);
using BaseItemFn = uintptr_t(__thiscall*)(uintptr_t item);

// Item icon resref (sanitized, NUL-terminated into out[ResrefCap]); out[0] = 0 = none. Never calls the engine on an unreadable item, an implausible
// base index or an unreadable base item; the engine's own empty result (null base) stays "".
void readItemIcon(uintptr_t item, char* out) {
    out[0] = 0;
    int idx = -1;
    if (!item || !saferead::probeRead(SiteIcon, reinterpret_cast<const void*>(item), Item_IconProbe) || !rd<int>(item, Item_BaseIdx, &idx) ||
        !qlogic::baseItemIdxOk(idx))
        return;
    const uintptr_t base = reinterpret_cast<BaseItemFn>(ItemGetBaseItem)(item);
    if (!base || !saferead::probeRead(SiteIcon, reinterpret_cast<const void*>(base), BaseItem_Probe)) return;
    char raw[16];
    memset(raw, 0, sizeof raw);
    reinterpret_cast<IconFn>(ItemGetIcon)(item, raw);
    qlogic::sanitizeResref(out, raw);
}

// Reads the container's item names / stacks / icon resrefs. SafeRead-guarded; an unreadable part just leaves fewer (or "item") entries.
// 0.7.1-dev page rows (quick_loot.ini TipDetail / SortFirst / EquippedLine / Panel; QuickLootPage.h), set at init and by the page.
int g_tipDetail = qlpage::TipFull;
bool g_sortFirst = true, g_equipLine = true, g_panelDark = true;

void captureItems(uintptr_t plc, Captured& c) {
    c.count = 0; c.total = 0;
    uintptr_t repo = 0;
    const int n = itemCount(plc, &repo);
    unsigned lines = qlogic::clampItems(n);
    uint32_t arr = 0;
    if (!lines || !repo || !rd<uint32_t>(repo, Repo_Array, &arr) || !arr) return;
    c.total = static_cast<unsigned>(n);
    if (lines > qlogic::ListMaxItems) lines = qlogic::ListMaxItems;
    const int lang = clientLanguage();
    uint8_t plot[qlogic::ListMaxItems];
    bool credits[qlogic::ListMaxItems];
    for (unsigned i = 0; i < lines; ++i) {
        uint32_t id = 0; int stack = -1;
        if (!rd<uint32_t>(arr, static_cast<int>(i * 4), &id)) break;
        const uintptr_t item = reinterpret_cast<LookupFn>(ItemListGetItem)(repo, i);
        c.names[i][0] = 0;
        c.icons[i][0] = 0;
        plot[i] = 0; credits[i] = false;
        if (item) {
            int baseId = -1;
            rd<int>(item, Item_Stack, &stack); readItemName(item, lang, c.names[i], NameCap); readItemIcon(item, c.icons[i]);
            rd<uint8_t>(item, Item_Plot, &plot[i]);
            if (rd<int>(item, Item_BaseIdx, &baseId) && baseId == BaseId_Credits) {   // QL11: "Credits (150)", no stack column
                credits[i] = true;
                if (stack > 0) snprintf(c.names[i], NameCap, "Credits (%d)", stack); else snprintf(c.names[i], NameCap, "Credits");
                stack = 1;
            }
        }
        c.ids[i] = id; c.stacks[i] = stack;
        ++c.count;
    }
    // QL11: plot items first, credits second, then the container's own order. The ids stay the object ids, so clicks and tooltips find the item by id.
    if (c.count > 1 && g_sortFirst) {
        static Captured src;
        src = c;
        uint8_t order[qlogic::ListMaxItems];
        qlogic::sortOrder(plot, credits, c.count, order);
        for (unsigned r = 0; r < c.count; ++r) {
            const unsigned k = order[r];
            c.ids[r] = src.ids[k]; c.stacks[r] = src.stacks[k];
            memcpy(c.names[r], src.names[k], sizeof c.names[r]);
            memcpy(c.icons[r], src.icons[k], sizeof c.icons[r]);
        }
    }
    unsigned nPlot = 0, nCred = 0;
    for (unsigned i = 0; i < c.count; ++i) { nPlot += plot[i] ? 1u : 0u; nCred += credits[i] ? 1u : 0u; }
    static unsigned lastN = ~0u, lastPlot = ~0u, lastCred = ~0u;   // change-only: the peek re-captures every scan
    if (c.count != lastN || nPlot != lastPlot || nCred != lastCred) {
        lastN = c.count; lastPlot = nPlot; lastCred = nCred;
        KP_LOG(chMain, "SORT n=%u plot=%u credits=%u", c.count, nPlot, nCred);
    }
}

// Checks that the repository slot `index` still holds object id `expectId` and returns the item (0 = no). *repoOut = the repository.
// 0.7.0-dev (QL11): `index` is the UI row of the SORTED list, not a container slot: the item is found by scanning the repository's id array for expectId
// (Stale only when the id is gone), then fetched by its slot.
uintptr_t itemAt(uintptr_t plc, unsigned index, uint32_t expectId, uintptr_t* repoOut, qlogic::TakeWhy* why) {
    uintptr_t repo = 0;
    const int n = itemCount(plc, &repo);
    uint32_t arr = 0;
    if (n <= 0 || !repo) { *why = qlogic::TakeWhy::NoRepo; return 0; }
    if (index >= static_cast<unsigned>(n) || index >= qlogic::ListMaxItems) { *why = qlogic::TakeWhy::BadIndex; return 0; }
    if (!rd<uint32_t>(repo, Repo_Array, &arr) || !arr) { *why = qlogic::TakeWhy::Stale; return 0; }
    unsigned slot = ~0u;
    const unsigned scan = n > 1024 ? 1024u : static_cast<unsigned>(n);
    for (unsigned k = 0; k < scan; ++k) {
        uint32_t id = 0;
        if (!rd<uint32_t>(arr, static_cast<int>(k * 4), &id)) break;
        if (id == expectId) { slot = k; break; }
    }
    if (slot == ~0u) { *why = qlogic::TakeWhy::Stale; return 0; }
    const uintptr_t item = reinterpret_cast<LookupFn>(ItemListGetItem)(repo, slot);
    if (!item || !saferead::probeRead(SiteTake, reinterpret_cast<const void*>(item), Item_IconProbe)) { *why = qlogic::TakeWhy::NoItem; return 0; }
    *repoOut = repo;
    return item;
}

bool tagIsLocker(uintptr_t plc) {   // the drain compares the placeable's tag with "lockertsfinv" (CExoString compare, asm 0x5F5899)
    char tag[40];
    if (saferead::readExo(SiteTake, plc + Plc_Tag, tag, sizeof tag, 39, false) <= 0) return false;
    const char* want = "lockertsfinv";
    for (int i = 0;; ++i) {   // case-insensitive compare without a CRT import
        const char a = tag[i] >= 'A' && tag[i] <= 'Z' ? static_cast<char>(tag[i] + 32) : tag[i];
        if (a != want[i]) return false;
        if (!want[i]) return true;
    }
}

// QL4: ONE item from the container to the leader = the server CloseInventory drain body for the clicked index (u03.md A1). Everything that can fail is checked
// BEFORE the item leaves the container: the slot still holds the expected id, the destination repository resolves, and the first-pickup pop-up case is refused.
bool engineTakeOne(uintptr_t plc, uint32_t opener, unsigned index, uint32_t expectId, qlogic::TakeOneOut* out) {
    using namespace qlogic;
    saferead::beginScope();
    out->why = TakeWhy::Failed; out->notice = 0; out->rc = 0; out->id = 0;
    out->nBefore = itemCount(plc);
    uintptr_t repo = 0;
    const uintptr_t item = itemAt(plc, index, expectId, &repo, &out->why);
    if (!item) return false;
    const uintptr_t srv = serverApp();
    if (!srv || !opener) { out->why = TakeWhy::NoCtx; return false; }
    const uintptr_t cre = reinterpret_cast<LookupFn>(GetCreatureObj)(srv, opener);
    if (!cre || !saferead::probeRead(SiteTake, reinterpret_cast<const void*>(cre), 0x30)) { out->why = TakeWhy::NoCtx; return false; }
    const uintptr_t holder = reinterpret_cast<uintptr_t(__fastcall*)(uintptr_t)>(CreatureHolder)(cre);
    uint32_t vt = 0, slot = 0;
    if (!holder || !rd<uint32_t>(holder, 0, &vt) || !vt || !rd<uint32_t>(vt, Holder_VtCtx, &slot) || !slot || !saferead::probeRead(SiteTake, reinterpret_cast<const void*>(static_cast<uintptr_t>(slot)), 1)) { out->why = TakeWhy::NoCtx; return false; }
    const uintptr_t ctx = reinterpret_cast<uintptr_t(__thiscall*)(uintptr_t)>(static_cast<uintptr_t>(slot))(holder);
    if (!ctx) { out->why = TakeWhy::NoCtx; return false; }
    const uintptr_t dst = reinterpret_cast<uintptr_t(__thiscall*)(uintptr_t, int)>(GetItemRepository)(ctx, 1);
    if (!dst || !saferead::probeRead(SiteTake, reinterpret_cast<const void*>(dst), 0x20)) { out->why = TakeWhy::NoDst; return false; }
    // The drain's first-pickup branch: base item class byte 0x18 only.
    int idx = -1;
    if (!rd<int>(item, Item_BaseIdx, &idx) || !baseItemIdxOk(idx)) { out->why = TakeWhy::NoItem; return false; }
    const uintptr_t base = reinterpret_cast<BaseItemFn>(ItemGetBaseItem)(item);
    uint8_t cls = 0;
    if (!base || !rd<uint8_t>(base, Base_NoticeClass, &cls)) { out->why = TakeWhy::NoItem; return false; }
    NoticePlan plan = NoticePlan::None;
    if (cls == BaseItemNoticeClass) {
        const bool bit = reinterpret_cast<int(__thiscall*)(uintptr_t, int)>(VarGetBool)(item + Item_Vars, ItemLocalBoolBit) != 0;
        plan = noticePlan(cls, tagIsLocker(plc), bit);
        if (plan != NoticePlan::None) out->notice = 1;
        if (plan == NoticePlan::Popup) { out->why = TakeWhy::Popup; return false; }   // F (take all) runs the real drain and shows the pop-up
    }
    reinterpret_cast<int(__thiscall*)(uintptr_t, uintptr_t)>(RepoRemove)(repo, item);
    if (plan == NoticePlan::SetBit) reinterpret_cast<void(__thiscall*)(uintptr_t, int, int)>(VarSetBool)(item + Item_Vars, ItemLocalBoolBit, 1);
    uintptr_t held = item;   // InsertOrMerge may replace it with the merged stack or null it (special base types)
    out->rc = reinterpret_cast<int(__thiscall*)(uintptr_t, uintptr_t*, int, int, int)>(RepoInsert)(dst, &held, 1, 1, 1);
    if (held) reinterpret_cast<int(__thiscall*)(uintptr_t, uint32_t, int, int, int)>(ItemSetOwner)(held, opener, 1, 1, 0x7F000000);
    out->why = TakeWhy::Ok; out->id = expectId;
    out->nAfter = itemCount(plc);
    return true;
}

// ---- 0.7.0-dev (QL9 / QL13): engine text helpers. An engine CExoString is {char* p; int len} (8 bytes): built with ExoCtor, read with readExo, freed with ExoDtor.
struct ExoStr { uint32_t p; int len; };
using ExoCtorFn = void(__thiscall*)(void*, const char*);
void exoInit(ExoStr& e, const char* text) { e.p = 0; e.len = 0; reinterpret_cast<ExoCtorFn>(gameaddr::ExoCtor)(&e, text); }
void exoFree(ExoStr& e) { reinterpret_cast<ExoDtorFn>(gameaddr::ExoDtor)(&e); }
int exoRead(ExoStr& e, char* out, int cap) { return saferead::readExo(Site, reinterpret_cast<uintptr_t>(&e), out, cap, cap - 1, false); }

bool g_rules = true;     // quick_loot.ini Rules: 0 = base rules only (no engine property text)
bool g_sounds = true;    // quick_loot.ini Sounds
bool g_hideArrow = true; // quick_loot.ini HideTargetArrow (0.7.2-dev QL14, QlTargetArrow)
unsigned g_soundLogs = 0;

// QL12: GUI sound idx through the GUI sound manager (0 = scrollbar click, 1 = hover, 9 = gui_invadd). Skips silently without the manager.
void playGuiSound(char idx) {
    if (!g_sounds) return;
    uint32_t mgr = 0;
    if (!rd<uint32_t>(gameaddr::GuiSoundMgr, 0, &mgr) || !mgr) return;
    if (g_soundLogs < 10) { ++g_soundLogs; KP_LOG(chMain, "SOUND idx=%d", static_cast<int>(idx)); }
    reinterpret_cast<void(__thiscall*)(uintptr_t, char)>(gameaddr::PlayGuiSound)(mgr, idx);
}

// TLK text of strref (CClientExoApp::GetGUIString). If the temp string stays empty the return value is tried as a CExoString* (logged once). false = no text.
bool tlkText(int strref, char* out, int cap) {
    out[0] = 0;
    uint32_t app = 0, client = 0;
    if (!rd<uint32_t>(gameaddr::AppGlobal, 0, &app) || !app || !rd<uint32_t>(app, App_Client, &client) || !client) return false;
    ExoStr t;
    exoInit(t, "");
    const uintptr_t ret = reinterpret_cast<uintptr_t(__thiscall*)(uintptr_t, ExoStr*, int)>(TlkGetString)(client, &t, strref);
    if (exoRead(t, out, cap) <= 0) {
        static bool logged = false;
        if (!logged) { logged = true; KP_LOG(chMain, "TLK-EMPTY strref=%d ret=%08x p=%08x len=%d", strref, (unsigned)ret, (unsigned)t.p, t.len); }
        if (ret && ret != reinterpret_cast<uintptr_t>(&t)) saferead::readExo(Site, ret, out, cap, cap - 1, false);
    }
    exoFree(t);
    return out[0] != 0;
}

// itempropdef.2da 'label' column of row `row` ("Damage", "DecreaseAC", "CastSpell", ...). false = unresolved.
bool itemPropLabel(int row, char* out, int cap) {
    out[0] = 0;
    uint32_t g = 0, x1 = 0, tda = 0;
    if (!rd<uint32_t>(RulesGlobal, 0, &g) || !g || !rd<uint32_t>(g, Rules_ItemPropDef1, &x1) || !x1 || !rd<uint32_t>(x1, Rules_ItemPropDef2, &tda) || !tda) return false;
    ExoStr col, res;
    exoInit(col, "label");
    exoInit(res, "");
    const int ok = reinterpret_cast<int(__thiscall*)(uintptr_t, int, ExoStr*, ExoStr*)>(C2daGetString)(tda, row, &col, &res);
    if (ok) exoRead(res, out, cap);
    exoFree(res);
    exoFree(col);
    return out[0] != 0;
}

// The engine's own text for one item property (label / subtype / cost value) through 0x00614E10. Fills p.name / p.sub / p.val (tags stripped, cut to the field).
void itemPropTexts(uint16_t prop, uint16_t sub, uint16_t costValue, uint8_t p1Value, qlogic::RulesProp* p) {
    using PropFn = void(__thiscall*)(uintptr_t unusedEcx, uint16_t, uint16_t, uint16_t, uint8_t, ExoStr*, ExoStr*, ExoStr*, ExoStr*, int);
    ExoStr l, s2, c, p1;
    exoInit(l, ""); exoInit(s2, ""); exoInit(c, ""); exoInit(p1, "");
    reinterpret_cast<PropFn>(ItemPropBuilder)(0, prop, sub, costValue, p1Value, &l, &s2, &c, &p1, 0);
    char tmp[160];
    if (exoRead(l, tmp, sizeof tmp) > 0) qlogic::stripTags(p->name, sizeof p->name, tmp);
    if (exoRead(s2, tmp, sizeof tmp) > 0) qlogic::stripTags(p->sub, sizeof p->sub, tmp);
    if (exoRead(c, tmp, sizeof tmp) > 0) qlogic::stripTags(p->val, sizeof p->val, tmp);
    exoFree(p1); exoFree(c); exoFree(s2); exoFree(l);
}

// Damage words (rules_detail dmgWords): [0] TLK 38552 (bits 0-2), [b] TLK 38553 + b - 3 for b = 3..12, [13] TLK 0xA3AF. Resolved at the first describe (a few tries: the TLK may not be up at BUILD).
char g_dmgBuf[14][48];
const char* g_dmgWords[14];
bool g_dmgReady = false;
unsigned g_dmgTries = 0;
void resolveDmgWords() {
    if (g_dmgReady || g_dmgTries >= 3) return;
    ++g_dmgTries;
    unsigned got = 0;
    for (int b = 0; b < 14; ++b) {
        g_dmgWords[b] = nullptr;
        if (b == 1 || b == 2) continue;
        const int ref = b == 0 ? 38552 : (b == 13 ? 0xA3AF : 38553 + b - 3);
        if (tlkText(ref, g_dmgBuf[b], sizeof g_dmgBuf[b])) { g_dmgWords[b] = g_dmgBuf[b]; ++got; }
    }
    g_dmgReady = got > 0;
    KP_LOG(chMain, "DMGWORDS try=%u got=%u", g_dmgTries, got);
}

struct BaseStats { uint32_t nameRef = 0, dmgFlags = 0; int numDice = 0, die = 0, critThreat = 0, critMult = 0, baseAC = 0, dexBonus = 0; };
// baseitems row of `item` (GetBaseItem) -> the columns the rules need. false = unreadable.
bool readBaseStats(uintptr_t item, BaseStats* b) {
    int idx = -1;
    if (!item || !rd<int>(item, Item_BaseIdx, &idx) || !qlogic::baseItemIdxOk(idx)) return false;
    const uintptr_t base = reinterpret_cast<BaseItemFn>(ItemGetBaseItem)(item);
    if (!base || !saferead::probeRead(SiteIcon, reinterpret_cast<const void*>(base), BaseItem_Rules)) return false;
    uint8_t nd = 0, die = 0, ct = 0, cm = 0, ac = 0, dex = 0;
    rd<uint32_t>(base, 0x00, &b->nameRef); rd<uint32_t>(base, 0x0C, &b->dmgFlags);
    rd<uint8_t>(base, 0x1C, &nd); rd<uint8_t>(base, 0x1D, &die); rd<uint8_t>(base, 0x1E, &ct); rd<uint8_t>(base, 0x1F, &cm);
    rd<uint8_t>(base, 0x8A, &ac); rd<uint8_t>(base, 0xAD, &dex);
    b->numDice = nd; b->die = die; b->critThreat = ct; b->critMult = cm; b->baseAC = ac; b->dexBonus = dex;
    return true;
}

// The leader creature (server object of `opener`) or 0.
uintptr_t leaderCreature(uint32_t opener) {
    const uintptr_t srv = serverApp();
    if (!srv || !opener) return 0;
    const uintptr_t cre = reinterpret_cast<LookupFn>(GetCreatureObj)(srv, opener);
    return (cre && saferead::probeRead(SiteTake, reinterpret_cast<const void*>(cre), 0x30)) ? cre : 0;
}
// The leader's item repository (the drain's destination, GetItemRepository(ctx, 1)) or 0.
uintptr_t leaderRepo(uintptr_t cre) {
    const uintptr_t holder = reinterpret_cast<uintptr_t(__fastcall*)(uintptr_t)>(CreatureHolder)(cre);
    uint32_t vt = 0, slot = 0;
    if (!holder || !rd<uint32_t>(holder, 0, &vt) || !vt || !rd<uint32_t>(vt, Holder_VtCtx, &slot) || !slot || !saferead::probeRead(SiteTake, reinterpret_cast<const void*>(static_cast<uintptr_t>(slot)), 1)) return 0;
    const uintptr_t ctx = reinterpret_cast<uintptr_t(__thiscall*)(uintptr_t)>(static_cast<uintptr_t>(slot))(holder);
    if (!ctx) return 0;
    const uintptr_t dst = reinterpret_cast<uintptr_t(__thiscall*)(uintptr_t, int)>(GetItemRepository)(ctx, 1);
    return (dst && saferead::probeRead(SiteTake, reinterpret_cast<const void*>(dst), 0x20)) ? dst : 0;
}
// The item in the creature's inventory slot `mask` (CSWInventory::GetItemInSlot) or 0.
uintptr_t equippedItem(uintptr_t cre, uint32_t mask) {
    uint32_t inv = 0;
    static unsigned logs = 0;
    if (!rd<uint32_t>(cre, Creature_Inventory, &inv) || !inv || !saferead::probeRead(SiteTake, reinterpret_cast<const void*>(static_cast<uintptr_t>(inv)), 0x20)) {
        if (logs < 10) { ++logs; KP_LOG(chMain, "EQUIP-MISS mask=%x cre=%08x inv=%08x (creature+0x1150 unreadable)", (unsigned)mask, (unsigned)cre, (unsigned)inv); }
        return 0;
    }
    const uintptr_t it = reinterpret_cast<uintptr_t(__thiscall*)(uintptr_t, uint32_t)>(InvGetItemInSlot)(inv, mask);
    if (logs < 10) { ++logs; KP_LOG(chMain, "EQUIP mask=%x item=%08x", (unsigned)mask, (unsigned)it); }
    return (it && saferead::probeRead(SiteTake, reinterpret_cast<const void*>(it), Item_IconProbe)) ? it : 0;
}

// QL13 foot. Weapon: "Equipped: <name> (<min>-<max>)" from the main-hand item, armor: "Equipped: <name> (Defense +N)" from the body slot, nothing: "Equipped: none".
// Anything else stackable: "In inventory: N" (stacks of the same base item and name in the leader's repository); 0 = no foot.
void buildFoot(uintptr_t item, const BaseStats& bs, int baseId, int stack, const char* ownName, int lang, char* foot, size_t cap) {
    foot[0] = 0;
    if (baseId == BaseId_Credits) return;
    const bool weapon = bs.numDice > 0, armor = !weapon && bs.baseAC > 0;
    const uintptr_t cre = leaderCreature(g_m.peek.opener);
    if (!cre) return;
    if (weapon || armor) {
        const uintptr_t eq = equippedItem(cre, weapon ? SlotMainHand : SlotBody);
        if (!eq) { snprintf(foot, cap, "Equipped: none"); return; }
        static char en[qlogic::NameCap + 16];
        readItemText(eq, Item_Name, lang, en, sizeof en);
        BaseStats eb;
        char suffix[32] = "";
        if (readBaseStats(eq, &eb)) {
            if (weapon && eb.numDice > 0) snprintf(suffix, sizeof suffix, " (%d-%d)", eb.numDice, eb.numDice * eb.die);
            else if (armor && eb.baseAC > 0) snprintf(suffix, sizeof suffix, " (Defense +%d)", eb.baseAC);
        }
        qlogic::fmtEquippedFoot(foot, cap, en, suffix);
        return;
    }
    if (stack < 1) return;
    const uintptr_t dst = leaderRepo(cre);
    int n = 0;
    if (!dst || !rd<int>(dst, Repo_Count, &n) || n <= 0) return;
    if (n > 512) n = 512;
    char mine[qlogic::NameCap + 16];
    qlogic::stripTags(mine, sizeof mine, ownName);
    long total = 0;
    for (int i = 0; i < n; ++i) {
        const uintptr_t it = reinterpret_cast<LookupFn>(ItemListGetItem)(dst, static_cast<unsigned>(i));
        int idx = -1, st = 0;
        if (!it || !rd<int>(it, Item_BaseIdx, &idx) || idx != baseId) continue;
        static char nm[qlogic::NameCap + 16], stripped[qlogic::NameCap + 16];
        readItemText(it, Item_Name, lang, nm, sizeof nm);
        qlogic::stripTags(stripped, sizeof stripped, nm);
        if (strcmp(stripped, mine) != 0) continue;
        total += (rd<int>(it, Item_Stack, &st) && st > 0) ? st : 1;
    }
    (void)item;
    if (total > 0) snprintf(foot, cap, "In inventory: %ld", total);
}

// QL8 / QL9: the tooltip model of the container item with object id expectId (the UI row `index` is not the slot: sorted list). The item stays where it is.
bool engineDescribe(uintptr_t plc, unsigned index, uint32_t expectId, qlogic::TipData* out) {
    if (g_tipDetail == qlpage::TipOff) return false;   // "Item tooltip: Off" (the machine just asks again after the next hover delay; nothing is read)
    saferead::beginScope();
    uintptr_t repo = 0;
    qlogic::TakeWhy why = qlogic::TakeWhy::Ok;
    const uintptr_t item = itemAt(plc, index, expectId, &repo, &why);
    if (!item) return false;
    const int lang = clientLanguage();
    int stack = -1, baseId = -1;
    rd<int>(item, Item_Stack, &stack);
    rd<int>(item, Item_BaseIdx, &baseId);
    static char name[qlogic::NameCap + 16], desc[qlogic::TipDescCap], baseName[64];
    readItemText(item, Item_Name, lang, name, sizeof name);
    if (baseId == BaseId_Credits) {   // QL11: "Credits (N)", no " xN" in the title
        if (stack > 0) snprintf(name, sizeof name, "Credits (%d)", stack); else snprintf(name, sizeof name, "Credits");
        stack = 1;
    }
    readItemText(item, Item_Desc, lang, desc, sizeof desc);
    resolveDmgWords();
    static qlogic::RulesIn in;
    memset(&in, 0, sizeof in);
    BaseStats bs;
    baseName[0] = 0;
    if (readBaseStats(item, &bs)) {
        in.baseRow = baseId; in.numDice = bs.numDice; in.die = bs.die; in.critThreat = bs.critThreat; in.critMult = bs.critMult; in.baseAC = bs.baseAC; in.dexBonus = bs.dexBonus;
        in.dmgFlags = bs.dmgFlags;
        if (bs.nameRef != 0 && bs.nameRef != 0xFFFFFFFFu) tlkText(static_cast<int>(bs.nameRef), baseName, sizeof baseName);
    } else {
        in.baseRow = baseId;
    }
    in.desc = desc; in.baseName = baseName;
    for (int b = 0; b < 14; ++b) in.dmgWords[b] = g_dmgWords[b];
    if (g_rules) {   // the engine's own property text; an unresolved property is skipped
        uint32_t cnt = 0, arr = 0;
        if (rd<uint32_t>(item, Item_PropCount, &cnt) && cnt > 0 && cnt < 256 && rd<uint32_t>(item, Item_PropArray, &arr) && arr) {
            static unsigned propLogs = 0, missLogs = 0;
            for (uint32_t i = 0; i < cnt && in.nProps < PropsMax; ++i) {
                const int o = static_cast<int>(i) * Prop_Stride;
                uint16_t pn = 0, sub = 0, cv = 0;
                uint8_t ctab = 0, p1 = 0, p1v = 0;
                if (!rd<uint16_t>(arr, o, &pn) || !rd<uint16_t>(arr, o + 2, &sub) || !rd<uint8_t>(arr, o + 4, &ctab) || !rd<uint16_t>(arr, o + 6, &cv) ||
                    !rd<uint8_t>(arr, o + 8, &p1) || !rd<uint8_t>(arr, o + 9, &p1v))
                    break;
                qlogic::RulesProp& p = in.props[in.nProps];
                memset(&p, 0, sizeof p);
                if (!itemPropLabel(pn, p.label, sizeof p.label)) {
                    if (missLogs < 20) { ++missLogs; KP_LOG(chMain, "RULES-MISS prop=%u", (unsigned)pn); }
                    continue;
                }
                itemPropTexts(pn, sub, cv, p1 == 0xFF ? static_cast<uint8_t>(0xFF) : p1v, &p);
                if (propLogs < 20) { ++propLogs; KP_LOG(chMain, "PROPTXT prop=%u sub=%u cv=%u label=%s name=%s sub=%s val=%s", (unsigned)pn, (unsigned)sub, (unsigned)cv, p.label, p.name, p.sub, p.val); }
                ++in.nProps;
            }
        }
    }
    qlogic::buildTipModel(in, name, stack, out);
    if (g_tipDetail == qlpage::TipStats && out->nRules > 0) out->flavor[0] = 0;   // "Stats only"; an item without rules keeps its description (it IS the rules, e.g. the medpac)
    if (g_equipLine) buildFoot(item, bs, baseId, stack, name, lang, out->foot, sizeof out->foot);
    else out->foot[0] = 0;
    static unsigned footLogs = 0;   // 0.7.1-dev: 0.7.0 play showed no EQUIP line at all; say which branch the foot took
    if (footLogs < 10) { ++footLogs; KP_LOG(chMain, "FOOT base=%d dice=%d ac=%d stack=%d opener=%08x cre=%08x foot='%s'", baseId, bs.numDice, bs.baseAC, stack, (unsigned)g_m.peek.opener, (unsigned)leaderCreature(g_m.peek.opener), out->foot); }
    return true;
}

// QL2A mode 1: the container's world point on screen. Reads the camera chain by SafeRead and calls ONLY the camera's MapToScreen, after checking that its
// vtable slot 0x28 is still 0x4809D0. Fills `pr` (state 1 = a value, 2 = unavailable with a note) and the world point for the PROJ line.
struct World3 { bool ok = false; float x = 0, y = 0, z = 0; };
World3 g_projWorld;   // the container point last read (kept while the Took list is up)
bool g_projNoted = false;
void projectWorld(const World3& w, qlogic::Proj* pr) {
    using namespace qlogic;
    *pr = Proj();
    pr->wx = w.x; pr->wy = w.y; pr->wz = w.z;
    static char note[48];
    auto fail = [&](const char* why) { snprintf(note, sizeof note, "%s", why); pr->state = 2; pr->note = note; };
    if (!w.ok) { fail("noworld"); return; }
    uint32_t app = 0, G = 0, A = 0, B = 0, cam = 0, gui = 0, vt = 0, slot = 0;
    if (!rd<uint32_t>(gameaddr::AppGlobal, 0, &app) || !app || !rd<uint32_t>(app, 4, &G) || !G || !rd<uint32_t>(G, 4, &A) || !A) { fail("chain-G"); return; }
    if (!rd<uint32_t>(A, Inner_CamHolder, &B) || !B || !rd<uint32_t>(B, CamHolder_Cam, &cam) || !cam) { fail("chain-cam"); return; }
    if (!rd<uint32_t>(A, Inner_Gui, &gui) || !gui) { fail("chain-gui"); return; }
    short gw = 0, gh = 0;
    if (!rd<short>(gui, Gui_W, &gw) || !rd<short>(gui, Gui_H, &gh) || gw <= 0 || gh <= 0) { fail("gui-size"); return; }
    if (!rd<uint32_t>(cam, 0, &vt) || !vt || !rd<uint32_t>(vt, CamVt_MapToScreen, &slot)) { fail("vtable"); return; }
    if (slot != CamMapToScreen) { snprintf(note, sizeof note, "slot=%08x", (unsigned)slot); pr->state = 2; pr->note = note; return; }
    float fovy = 0, pos[3], q[4];
    int vp[4];
    if (!rd<float>(cam, Cam_Fovy, &fovy) || !(fovy > 0.0f) || !rd<int>(cam, Cam_Vp + 8, &vp[2]) || !rd<int>(cam, Cam_Vp + 12, &vp[3]) || vp[2] <= 0 || vp[3] <= 0) { fail("viewport"); return; }
    for (int i = 0; i < 3; ++i) if (!rd<float>(cam, Cam_Pos + 4 * i, &pos[i])) { fail("pose"); return; }
    for (int i = 0; i < 4; ++i) if (!rd<float>(cam, Cam_Quat + 4 * i, &q[i])) { fail("pose"); return; }
    float out[3] = {0, 0, 0};
    reinterpret_cast<float*(__thiscall*)(uintptr_t, float*, float, float, float, int)>(static_cast<uintptr_t>(slot))(cam, out, w.x, w.y, w.z, 1);
    pr->x = out[0] * static_cast<float>(gw);
    pr->y = out[1] * static_cast<float>(gh);
    // front / behind: Pc.z = (R(q) e_z) . (P - cam) with q = (w, x, y, z); in front when Pc.z < 0 (K contract; the call itself mirrors silently).
    const float qw = q[0], qx = q[1], qy = q[2], qz = q[3];
    const float rz[3] = { 2.0f * (qx * qz + qw * qy), 2.0f * (qy * qz - qw * qx), 1.0f - 2.0f * (qx * qx + qy * qy) };
    const float pcz = rz[0] * (w.x - pos[0]) + rz[1] * (w.y - pos[1]) + rz[2] * (w.z - pos[2]);
    pr->ret = std::isfinite(pcz) ? (pcz < 0.0f ? 1 : 2) : 0;
    pr->state = 1;
    if (!g_projNoted) { g_projNoted = true; KP_LOG(chMain, "PROJ-CALL world=%.2f,%.2f,%.2f cam=%08x gui=%d,%d fovy=%.1f vp=%d,%d out=%.4f,%.4f pc.z=%.2f", (double)w.x, (double)w.y, (double)w.z, (unsigned)cam, (int)gw, (int)gh, (double)fovy, vp[2], vp[3], (double)out[0], (double)out[1], (double)pcz); }
}

// ---- labels. Draw order = creation order (BUILD makes them in this order): frame panel (back), header, scroll indicator, then per row (12 max) the
// bordered row frame, the icon, the text, then the scrollbar parts (track outline, track inset, thumb, up, down). Every draw is change-only per label.
struct Lbl {
    uintptr_t c = 0;                  // the control (0 = none: creation failed or the HUD changed)
    int vis = -1;                     // -1 unknown, 0 hidden, 1 shown
    bool rectSet = false, colSet = false;
    const char* font = nullptr;       // font resref last applied (static strings only)
    unsigned align = 0;
    qlogic::Rect rect = {0, 0, 0, 0};
    float col[3] = {0, 0, 0};
    uint32_t textHash = 0;            // FNV-1a of the text last set (0 = none yet): change-only without a per-label copy (tooltip texts are long)
    // 0.5.0-dev fill / border state (frames, icons, scrollbar parts)
    bool fillSet = false, tintSet = false, alphaSet = false;
    const char* fillName = nullptr;   // fill image resref last applied (static strings only; icons use `icon`)
    float tint[3] = {0, 0, 0};
    float alpha = -1.0f;
    char icon[qlogic::ResrefCap] = {0};   // icon resref last applied to this label's fill
    unsigned iconTries = 0;               // imageForRetry attempts for `icon` (a texture the engine cannot load stops being asked after IconTriesMax)
    bool clickable = false;               // click-eating while shown (the scrollbar column only)
};
Lbl g_back, g_head, g_ind, g_row[qlogic::ListRowsMax], g_rowFrame[qlogic::ListRowsMax], g_rowIcon[qlogic::ListRowsMax];
Lbl g_backFill, g_rowStack[qlogic::ListRowsMax];   // 0.7.0-dev: the list's opacity underlay (made BEFORE the frame) and the per-row stack column (made after the row text)
Lbl g_sbOuter, g_sbInner, g_sbThumb, g_sbUp, g_sbDown;
Lbl g_bezel;                                       // 0.8.0-dev (QL14): Hardware bezel label
Lbl g_title, g_headDiv, g_headFrame, g_headLed;    // 0.8.0-dev (QL14): Unified header controls
// 0.6.0-dev (QL8) hover tooltip, created last (drawn above the list); 0.7.0-dev (QL9) layout: underlay fill, frame, icon, title, category line, three dividers,
// label / value columns (one multi-line label each), flavor (g_tipBody), foot.
Lbl g_tipFill, g_tipFrame, g_tipIcon, g_tipTitle, g_tipSub, g_tipDiv[3], g_tipLabels, g_tipValues, g_tipBody, g_tipFoot;
bool g_tipOk = false;         // every tooltip label exists
bool g_font16Ok = true;       // dialogfont16x16 could be set on the tooltip title (else the 10x10 font)
bool g_rowsOk = false;        // header + indicator + every row / scrollbar label exists; false = one-line text in the header label (0.3.1 behaviour)
bool g_rowFlat = false;       // the row frames are flat fills (no frame template in the HUD layout)
bool g_backFrame = false;     // the backing is a vanilla frame template (fill + edges + corners from the .gui); false = flat tinted fill
bool g_backStyled = false;    // backing tint / alpha (/ fill) applied
bool g_backOff = false;       // backing could not be styled: text only
bool g_backRectSet = false;
qlogic::Rect g_backLast = {0, 0, 0, 0};
int32_t g_panel = 0;          // HUD object the labels belong to (K2HudContext.panel at BUILD == ctx->hud later)
bool g_buildFailLogged = false;
bool g_extentHead = false, g_extentRow = false;   // LABEL-EXTENT: header and first row, once per session
unsigned g_iconLogs = 0;      // ICON log lines written this session (first IconLogMax)
constexpr unsigned IconLogMax = 32, IconTriesMax = 6;

// Vanilla palette (character_p.gui BORDER / TEXT COLOR; scripts/character_page_mockup.py:45-47).
const float TealFrame[3] = { 0.05098f, 0.34902f, 0.27059f };   // frame tint
const float TealText[3] = { 0.10196f, 0.69804f, 0.54902f };    // item rows
const float Cream[3] = { 0.8f, 0.8f, 0.69804f };               // header and the hovered row
const float TealDim[3] = { 0.0663f, 0.4537f, 0.3569f };        // scroll indicator (TealText x 0.65)
const float RowHilight[3] = { 0.92941f, 1.0f, 0.92941f };      // vanilla inventory_p protoitem HILIGHT COLOR (same images as the row border, near-white-green)
const float White[3] = { 1.0f, 1.0f, 1.0f };
const float SbThumbTint[3] = { 0.98039f, 1.0f, 0.98039f };     // vanilla inventory SCROLLBAR BORDER COLOR; the 16g textures are already teal
const float SbOutlineTint[3] = { 0.6f, 0.6f, 0.6f };           // track outline: uibit_fill_16g (teal) darkened
const float SbInsetTint[3] = { 1.0f, 1.0f, 1.0f };             // track inset: uibit_fill_2bt is already near black at alpha 0.78
const float ArrowTint[3] = { 0.10196f, 0.69804f, 0.54902f };   // lbl_mapdown is white: tinted to the row teal
const float AlphaDim = 0.4f;                                    // an arrow that cannot move (list at its top / bottom)
const char* const TexFill16g = "uibit_fill_16g";
const char* const TexFill2bt = "uibit_fill_2bt";
const char* const TexUpArrow = "uibit_uparrowg";
const char* const TexDownArrow = "lbl_mapdown";                // 32x32 white down arrow (no uibit_*arrow* down texture exists)
const char* const FontName = "dialogfont10x10";                  // vanilla list font
const char* const FontTitle = "dialogfont16x16";                 // vanilla standard font (tooltip title)
// Vanilla frame templates in the HUD layout (mipc28x6_p.gui): FILL uibit_fill_2bt + EDGE uibit_brdr_16bet + CORNER uibit_brdr_16bct, DIMENSION 16,
// COLOR = TealFrame. LBL_MOULDING3 is the vanilla tooltip frame, LBL_MENUBG the target-menu frame. The backing is created from the first one that
// exists, so the edges and corners come from the .gui (GuiKit has no edge / corner setter); LBL_QUEUE1 + a flat black fill is the last resort.
const char* const FrameTemplates[] = { "LBL_MOULDING3", "LBL_MENUBG" };

void lblHideOne(Lbl& l) {
    if (!l.c || l.vis == 0) return;
    guikit::setVisible(l.c, false);
    guikit::setClickable(l.c, false);
    l.vis = 0;
}

// Wheel / hover state shared with the gui-input callback (it runs inside the game window's WM_MOUSEWHEEL, so it only touches these).
volatile LONG g_wheelAccum = 0;
volatile LONG g_listRect[4] = {0, 0, 0, 0};   // x, y, w, h of the block drawn last (w 0 = no list)
volatile LONG g_listScroll = 0;               // the drawn list has more rows than fit
volatile DWORD g_listTick = 0;                // GetTickCount() of that draw
bool g_hover = false;                         // the cursor is over the block drawn last
int g_hoverRow = -1;
// The scrollbar of the block drawn last (absolute engine px), for the click test. Game thread only (FRAME and POST both run there).
struct SbPub { bool shown = false; qlogic::Sb sb; qlogic::Rect blk = {0, 0, 0, 0}; DWORD tick = 0; };
SbPub g_sbPub;
// 0.6.0-dev: the block drawn last, for the row click test and the hover row (game thread only).
struct BlockPub { bool shown = false, took = false, sb = false; qlogic::Rect blk = {0, 0, 0, 0}; unsigned vis = 0; DWORD tick = 0; };
BlockPub g_blockPub;
int g_rowClickPending = -1;                   // visible row of a left click on the peek list, delivered by Env::rowClick()
guikit::ClickEdge g_clickEdge;                // LMB edge state (guikit::clickEdge), polled every frame by sbClickPoll

void listPublishHide() { g_listRect[2] = 0; g_listScroll = 0; g_hover = false; g_hoverRow = -1; g_sbPub.shown = false; g_blockPub.shown = false; }

void tipHide() {
    Lbl* all[] = { &g_tipFill, &g_tipFrame, &g_tipIcon, &g_tipTitle, &g_tipSub, &g_tipDiv[0], &g_tipDiv[1], &g_tipDiv[2], &g_tipLabels, &g_tipValues, &g_tipBody, &g_tipFoot };
    for (Lbl* l : all) lblHideOne(*l);
}

void lblHideAll() {
    lblHideOne(g_bezel);
    lblHideOne(g_backFill);
    lblHideOne(g_back);
    lblHideOne(g_title);
    lblHideOne(g_headDiv);
    lblHideOne(g_headFrame);
    lblHideOne(g_headLed);
    lblHideOne(g_head);
    lblHideOne(g_ind);
    for (unsigned i = 0; i < qlogic::ListRowsMax; ++i) { lblHideOne(g_rowFrame[i]); lblHideOne(g_rowIcon[i]); lblHideOne(g_row[i]); lblHideOne(g_rowStack[i]); }
    lblHideOne(g_sbOuter); lblHideOne(g_sbInner); lblHideOne(g_sbThumb); lblHideOne(g_sbUp); lblHideOne(g_sbDown);
    tipHide();
    g_backRectSet = false;
    listPublishHide();
}

// The HUD was rebuilt: the old labels may be freed. Drop every pointer without writing to them.
void lblDrop() {
    g_bezel = Lbl(); g_back = Lbl(); g_backFill = Lbl(); g_title = Lbl(); g_headDiv = Lbl(); g_headFrame = Lbl(); g_headLed = Lbl(); g_head = Lbl(); g_ind = Lbl();
    for (Lbl& r : g_row) r = Lbl();
    for (Lbl& r : g_rowStack) r = Lbl();
    for (Lbl& r : g_rowFrame) r = Lbl();
    for (Lbl& r : g_rowIcon) r = Lbl();
    g_sbOuter = Lbl(); g_sbInner = Lbl(); g_sbThumb = Lbl(); g_sbUp = Lbl(); g_sbDown = Lbl();
    g_tipFill = Lbl(); g_tipFrame = Lbl(); g_tipIcon = Lbl(); g_tipTitle = Lbl(); g_tipSub = Lbl(); g_tipLabels = Lbl(); g_tipValues = Lbl(); g_tipBody = Lbl(); g_tipFoot = Lbl();
    for (Lbl& r : g_tipDiv) r = Lbl();
    g_tipOk = false;
    g_rowFlat = false;
    g_rowsOk = false; g_backFrame = false; g_backStyled = false; g_backOff = false; g_backRectSet = false;
    g_panel = 0;
    listPublishHide();
}

// Creates one hidden, click-through label in the HUD panel from the layout template `tag` (draw order = creation order). 0 on failure.
uintptr_t makeLabel(uintptr_t panel, const char* tag, const char* what, bool logFail = true) {
    int why = guikit::CF_None;
    uint32_t bv = 0;
    const uintptr_t c = guikit::createLabelFromTemplate(panel, tag, &why, &bv);
    if (!c) {
        if (logFail && !g_buildFailLogged) { g_buildFailLogged = true; KP_LOGV(chMain, KPLOG_WARN, "BUILD: %s label (%s) create failed (why=%d vtable=%08x): no %s", what, tag, why, (unsigned)bv, what); }
        return 0;
    }
    guikit::setVisible(c, false);
    guikit::setClickable(c, false);
    if (!guikit::appendToPanel(panel, c)) {
        if (logFail && !g_buildFailLogged) { g_buildFailLogged = true; KP_LOGV(chMain, KPLOG_WARN, "BUILD: %s label append failed (control %08x left hidden): no %s", what, (unsigned)c, what); }
        return 0;
    }
    return c;
}

// K2HUD_BUILD: the frame panel first, then header, indicator, the row pool (bordered frame, icon, text per row) and the scrollbar parts (hidden,
// click-through) in the HUD panel; draw order = creation order. There is no destroy API, so the pool is made once per BUILD at its maximum
// (ListRowsMax). Any label failing -> one-line fallback in the header label.
void KPATCH_CALL onHudBuild(const K2HudContext* ctx, void*) {
    lblDrop();
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, panel) + sizeof(ctx->panel) || !ctx->panel) return;
    enter();
    ensureBezelTextures();
    const uintptr_t panel = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
    const char* frameTag = "LBL_QUEUE1";
    g_bezel.c = makeLabel(panel, "LBL_QUEUE1", "bezel", false);             // 0.8.0-dev (QL14): Hardware bezel underneath
    g_backFill.c = makeLabel(panel, "LBL_QUEUE1", "backing fill", false);   // 0.7.0-dev: underlay first (draw order = creation order); losing it only costs opacity
    for (const char* tag : FrameTemplates) {
        g_back.c = makeLabel(panel, tag, "frame", false);
        if (g_back.c) { g_backFrame = true; frameTag = tag; break; }
    }
    if (!g_back.c) g_back.c = makeLabel(panel, "LBL_QUEUE1", "backing");   // flat fill; losing the backing only costs the dark panel
    g_panel = ctx->panel;
    g_title.c = makeLabel(panel, "LBL_QUEUE1", "title");
    g_ind.c = makeLabel(panel, "LBL_QUEUE1", "indicator");
    g_headDiv.c = makeLabel(panel, "LBL_QUEUE1", "header divider");
    g_headFrame.c = makeLabel(panel, frameTag, "action button frame");
    g_headLed.c = makeLabel(panel, "LBL_QUEUE1", "action button led");
    g_head.c = makeLabel(panel, "LBL_QUEUE1", "header");
    if (!g_head.c) return;
    bool rowsOk = g_ind.c != 0 && g_title.c != 0 && g_headDiv.c != 0 && g_headFrame.c != 0;
    g_rowFlat = !g_backFrame;   // the row frames come from the same template as the backing; flat fills when the layout has none
    for (unsigned i = 0; i < qlogic::ListRowsMax && rowsOk; ++i) {
        g_rowFrame[i].c = makeLabel(panel, frameTag, "row frame");
        g_rowIcon[i].c = g_rowFrame[i].c ? makeLabel(panel, "LBL_QUEUE1", "row icon") : 0;
        g_row[i].c = g_rowIcon[i].c ? makeLabel(panel, "LBL_QUEUE1", "row") : 0;
        g_rowStack[i].c = g_row[i].c ? makeLabel(panel, "LBL_QUEUE1", "row stack") : 0;
        if (!g_rowStack[i].c) rowsOk = false;
    }
    Lbl* sbParts[5] = { &g_sbOuter, &g_sbInner, &g_sbThumb, &g_sbUp, &g_sbDown };
    for (Lbl* l : sbParts) {
        if (!rowsOk) break;
        l->c = makeLabel(panel, "LBL_QUEUE1", "scrollbar");
        if (!l->c) rowsOk = false;
    }
    g_rowsOk = rowsOk;
    if (rowsOk) {   // the tooltip, last = on top (0.7.0-dev QL9 set). A failure only costs the tooltip.
        bool tipOk = true;
        auto mk = [&](Lbl& l, const char* tag, const char* what) { if (tipOk) { l.c = makeLabel(panel, tag, what, false); if (!l.c) tipOk = false; } };
        mk(g_tipFill, "LBL_QUEUE1", "tip fill");   // underlay before the frame
        mk(g_tipFrame, frameTag, "tip frame");
        mk(g_tipIcon, "LBL_QUEUE1", "tip icon");
        mk(g_tipTitle, "LBL_QUEUE1", "tip title");
        mk(g_tipSub, "LBL_QUEUE1", "tip category");
        for (Lbl& dv : g_tipDiv) mk(dv, "LBL_QUEUE1", "tip divider");
        mk(g_tipLabels, "LBL_QUEUE1", "tip labels");
        mk(g_tipValues, "LBL_QUEUE1", "tip values");
        mk(g_tipBody, "LBL_QUEUE1", "tip flavor");
        mk(g_tipFoot, "LBL_QUEUE1", "tip foot");
        g_tipOk = tipOk;
        Lbl* tl[] = { &g_tipFill, &g_tipFrame, &g_tipIcon, &g_tipTitle, &g_tipSub, &g_tipDiv[0], &g_tipDiv[1], &g_tipDiv[2], &g_tipLabels, &g_tipValues, &g_tipBody, &g_tipFoot };
        for (Lbl* l : tl) l->vis = l->c ? 0 : -1;
    }
    g_bezel.vis = g_bezel.c ? 0 : -1;
    g_backFill.vis = g_backFill.c ? 0 : -1;
    g_back.vis = g_back.c ? 0 : -1; g_head.vis = 0; g_ind.vis = g_ind.c ? 0 : -1;
    g_title.vis = g_title.c ? 0 : -1; g_headDiv.vis = g_headDiv.c ? 0 : -1;
    g_headFrame.vis = g_headFrame.c ? 0 : -1; g_headLed.vis = g_headLed.c ? 0 : -1;
    for (unsigned i = 0; i < qlogic::ListRowsMax; ++i) {
        g_row[i].vis = g_row[i].c ? 0 : -1; g_rowStack[i].vis = g_rowStack[i].c ? 0 : -1; g_rowFrame[i].vis = g_rowFrame[i].c ? 0 : -1; g_rowIcon[i].vis = g_rowIcon[i].c ? 0 : -1;
    }
    for (Lbl* l : sbParts) l->vis = l->c ? 0 : -1;
    if (KpLog_HasCore()) KP_LOG(chMain, "BUILD frame=%s%s rows=%s tip=%d", frameTag, g_backFrame ? "" : " (flat)", g_rowsOk ? "pool" : "fallback-oneline", g_tipOk ? 1 : 0);
}

uint32_t textHash(const char* t) {   // FNV-1a, never 0 (0 = "no text set yet")
    uint32_t h = 2166136261u;
    for (; *t; ++t) { h ^= static_cast<unsigned char>(*t); h *= 16777619u; }
    return h ? h : 1u;
}

// Style (font once, align / colour on change), then text / rect / visibility change-only. false = a write failed (caller hides everything).
bool lblDrawText(Lbl& l, const char* text, const qlogic::Rect& r, unsigned align, const float* col, const char* font = FontName) {
    if (!l.c) return false;
    if (l.font != font) {   // font once per label (the tooltip title switches between 16x16 and 10x10)
        if (!guikit::setFont(l.c, font)) return false;
        l.font = font;
    }
    if (l.align != align) { if (!guikit::setAlign(l.c, align)) return false; l.align = align; }
    if (!l.colSet || memcmp(l.col, col, sizeof l.col) != 0) {
        if (!guikit::setTextColor(l.c, col[0], col[1], col[2])) return false;
        memcpy(l.col, col, sizeof l.col);
        l.colSet = true;
    }
    const uint32_t th = textHash(text);
    if (l.textHash != th) {
        if (!guikit::setText(l.c, text)) return false;
        l.textHash = th;
    }
    if (!l.rectSet || r != l.rect) { guikit::setRect(l.c, r.x, r.y, r.w, r.h); l.rect = r; l.rectSet = true; }
    if (l.vis != 1) { guikit::setVisible(l.c, true); guikit::setClickable(l.c, false); l.vis = 1; }   // shown labels stay click-through
    return true;
}

// Shows a label (change-only). clickable = the control eats clicks while shown (flags bit 5 clear; hidden always restores click-through).
void lblShow(Lbl& l, bool clickable) {
    if (l.vis == 1 && l.clickable == clickable) return;
    guikit::setVisible(l.c, true);   // shown = bit 5 cleared (clickable)
    guikit::setClickable(l.c, clickable);
    l.vis = 1; l.clickable = clickable;
}
void lblPlace(Lbl& l, const qlogic::Rect& r) {
    if (!l.rectSet || r != l.rect) { guikit::setRect(l.c, r.x, r.y, r.w, r.h); l.rect = r; l.rectSet = true; }
}
bool lblTint(Lbl& l, const float* tint) {
    if (l.tintSet && memcmp(l.tint, tint, sizeof l.tint) == 0) return true;
    if (!guikit::setBorderTint(l.c, tint)) return false;
    memcpy(l.tint, tint, sizeof l.tint);
    l.tintSet = true;
    return true;
}
bool lblAlpha(Lbl& l, float a) {
    if (l.alphaSet && l.alpha == a) return true;
    if (!guikit::setAlpha(l.c, a)) return false;
    l.alpha = a; l.alphaSet = true;
    return true;
}

// A bordered row frame (template clone: the edges, corners and fill come from the .gui): tint = TealFrame, or RowHilight while hovered (the vanilla HILIGHT
// is the same textures in another colour). Flat fallback (no frame template): the 2bt fill tinted the same. false = a write failed.
bool lblDrawFrame(Lbl& l, const qlogic::Rect& r, const float* tint, bool clickable = false) {
    if (!l.c) return false;
    if (g_rowFlat && !l.fillSet) {
        void* img = guikit::imageFor(TexFill2bt);
        if (!img || !guikit::setFill(l.c, img) || !guikit::setFillStyle(l.c, 2)) return false;
        l.fillSet = true;
    }
    if (!lblTint(l, tint) || !lblAlpha(l, qlogic::FrameAlpha)) return false;
    lblPlace(l, r);
    lblShow(l, clickable);   // 0.6.0-dev: the peek list's row frames eat the click (take that item) instead of walking the character
    return true;
}

// A fill-only label (the scrollbar parts, template LBL_QUEUE1): fill image `resref` (applied once), tint, alpha. A texture that will not load hides the
// label (soft: true); false = a write failed.
bool lblDrawFilled(Lbl& l, const char* resref, const float* tint, float alpha, const qlogic::Rect& r, bool clickable) {
    if (!l.c) return false;
    if (!l.fillSet || l.fillName != resref) {
        void* img = guikit::imageFor(resref);
        if (!img) { lblHideOne(l); return true; }
        if (!guikit::setFill(l.c, img, tint) || !guikit::setFillStyle(l.c, 2)) return false;
        memcpy(l.tint, tint, sizeof l.tint); l.tintSet = true;
        l.fillName = resref; l.fillSet = true;
    }
    if (!lblTint(l, tint) || !lblAlpha(l, alpha)) return false;
    lblPlace(l, r);
    lblShow(l, clickable);
    return true;
}

// The item icon: texture `res` (the engine resref from captureItems; "" = none) as the label's fill, looked up with imageForRetry (a texture that is not
// loadable yet is asked again, IconTriesMax times per resref). No texture = the label stays hidden: an empty icon box, never a crash. false = a write failed.
bool lblDrawIcon(Lbl& l, const char* res, const qlogic::Rect& r) {
    if (!l.c) return false;
    if (strcmp(l.icon, res) != 0) {   // a different item in this row: ask for its texture
        snprintf(l.icon, sizeof l.icon, "%s", res);
        l.fillSet = false; l.iconTries = 0;
        if (!res[0]) guikit::setFill(l.c, nullptr);   // clear the old image (never leave the previous item's icon on a texture-less row)
    }
    if (!res[0]) { lblHideOne(l); return true; }
    if (!l.fillSet && l.iconTries < IconTriesMax) {
        ++l.iconTries;
        void* img = guikit::imageForRetry(res);
        if (g_iconLogs < IconLogMax && (img || l.iconTries == IconTriesMax)) {
            ++g_iconLogs;
            KP_LOG(chMain, "ICON i=%u res=%s img=%d", g_iconLogs - 1, res, img ? 1 : 0);
        }
        if (img) {
            if (!guikit::setFill(l.c, img, White) || !guikit::setFillStyle(l.c, 2)) return false;
            memcpy(l.tint, White, sizeof l.tint); l.tintSet = true;
            l.fillSet = true;
        }
    }
    if (!l.fillSet) { lblHideOne(l); return true; }
    if (!lblAlpha(l, 1.0f)) return false;
    lblPlace(l, r);
    lblShow(l, false);
    return true;
}

// The backing panel under the text. Frame template: keeps the .gui's fill / edge / corner images, sets the tint (TealFrame or custom) and FrameAlpha.
// Flat fallback (no frame template in the HUD layout): the pill image tinted black at BackAlpha, the 0.3.1 look.
void backDraw(const qlogic::Rect& r, const float* frameTint = nullptr) {
    if (!g_back.c || g_backOff) return;
    const float* tint = frameTint ? frameTint : TealFrame;
    if (!g_backStyled || (frameTint && memcmp(g_back.tint, tint, sizeof g_back.tint) != 0)) {
        bool ok;
        if (g_backFrame) {
            ok = guikit::setBorderTint(g_back.c, tint) && guikit::setAlpha(g_back.c, qlogic::FrameAlpha);
            if (ok) { memcpy(g_back.tint, tint, sizeof g_back.tint); g_back.tintSet = true; }
        } else {
            static const float black[3] = { 0.0f, 0.0f, 0.0f };
            void* img = guikit::imageFor(TexBacking);
            ok = img && guikit::setFill(g_back.c, img) && guikit::setBorderTint(g_back.c, black) && guikit::setAlpha(g_back.c, qlogic::BackAlpha);
        }
        if (!ok) {
            g_backOff = true;
            KP_LOGV(chMain, KPLOG_WARN, "BACKING: tint / alpha%s unavailable (frame=%d): text without backing", g_backFrame ? "" : " / fill", g_backFrame ? 1 : 0);
            lblHideOne(g_back);
            return;
        }
        g_backStyled = true;
    }
    if (!g_backRectSet || r != g_backLast) { guikit::setRect(g_back.c, r.x, r.y, r.w, r.h); g_backLast = r; g_backRectSet = true; }
    if (g_back.vis != 1) { guikit::setVisible(g_back.c, true); guikit::setClickable(g_back.c, false); g_back.vis = 1; }
}

// 0.7.0-dev: the opacity underlay under the frame (block-relative rect `fill`): a second uibit_fill_2bt, only with the vanilla frame template (the flat backing has its own alpha).
void backFillDraw(const qlogic::Rect& fill, const qlogic::Rect& blk) {
    if (!g_backFill.c || !g_backFrame || g_backOff || !g_panelDark) { lblHideOne(g_backFill); return; }
    if (!lblDrawFilled(g_backFill, TexFill2bt, SbInsetTint, 1.0f, { blk.x + fill.x, blk.y + fill.y, fill.w, fill.h }, false)) lblHideOne(g_backFill);
}

void logExtent(Lbl& l, bool& done, const char* text) {
    if (done || !l.c) return;
    int x = 0, y = 0, w = 0, h = 0;
    if (guikit::getExtent(l.c, &x, &y, &w, &h)) {   // the real extent after the first SetText (a wrapped label would show a taller extent than RowH)
        done = true;
        char b[128];
        qlogic::fmtLabelExtent(b, sizeof b, static_cast<unsigned>(strlen(text)), x, y, w, h);
        logLine(b);
    }
}

k2input::Client g_input;   // gui-input provider (wheel + cursor); not connected = no scrolling, the first rows are shown

qlogic::Rect atBlock(const qlogic::Rect& r, const qlogic::Rect& blk) { return { blk.x + r.x, blk.y + r.y, r.w, r.h }; }
qlogic::Rect inset(const qlogic::Rect& r, int d) { return { r.x + d, r.y + d, r.w - 2 * d, r.h - 2 * d }; }

// The scrollbar of the block (absolute rects): a teal outline label (the click-eating column) under a dark inset, the thumb, the two arrows (dimmed
// while the list cannot move that way). Hidden when the list fits. Published for the click test (sbClickPoll).
bool sbDraw(const qlogic::ListDraw& d, const qlogic::Rect& blk, const float* frameTint = nullptr) {
    if (!d.sb) {
        lblHideOne(g_sbOuter); lblHideOne(g_sbInner); lblHideOne(g_sbThumb); lblHideOne(g_sbUp); lblHideOne(g_sbDown);
        g_sbPub.shown = false;
        return true;
    }
    const qlogic::Sb sb = qlogic::sbLayout(blk.w, d.vis, d.rowsTotal, d.off, d.rowH, true);
    const unsigned maxOff = d.rowsTotal > d.vis ? d.rowsTotal - d.vis : 0u;
    const qlogic::Rect col = atBlock(sb.col, blk);
    const qlogic::Rect up = inset(atBlock(sb.up, blk), 4), dn = inset(atBlock(sb.down, blk), 4);
    const float* outlineTint = frameTint ? frameTint : SbOutlineTint;
    if (!lblDrawFilled(g_sbOuter, TexFill16g, outlineTint, 1.0f, col, true) ||            // the one click-eating label: arrows, track and thumb all sit on it
        !lblDrawFilled(g_sbInner, TexFill2bt, SbInsetTint, 1.0f, inset(col, qlogic::SbInset), false) ||
        !lblDrawFilled(g_sbThumb, TexFill16g, SbThumbTint, 1.0f, atBlock(sb.thumb, blk), false) ||
        !lblDrawFilled(g_sbUp, TexUpArrow, White, d.off == 0 ? AlphaDim : 1.0f, up, false) ||
        !lblDrawFilled(g_sbDown, TexDownArrow, ArrowTint, d.off >= maxOff ? AlphaDim : 1.0f, dn, false))
        return false;
    g_sbPub.shown = true; g_sbPub.sb = sb; g_sbPub.blk = blk; g_sbPub.tick = GetTickCount();
    return true;
}

// QL8: the tooltip for the hovered peek row (Machine::tipShown), beside the block. false = a label write failed (the caller hides everything).
unsigned g_tipLogged = 0, g_tipLoggedSerial = 0;
constexpr unsigned TipLogMax = 20;
bool tipDraw(const qlogic::ListDraw& d, const qlogic::Rect& blk, const qlogic::Rect& panel, uint32_t now, int extraPadX = 0) {
    const qlogic::TipData* td = nullptr;
    int absRow = -1;
    if (!g_tipOk || d.took || !g_m.tipShown(now, &td, &absRow) || absRow < static_cast<int>(d.off) || absRow >= static_cast<int>(d.off + d.vis)) { tipHide(); return true; }
    const qlogic::Rect rr = atBlock(qlogic::rowRect(static_cast<unsigned>(absRow) - d.off, blk.w, d.sb, d.rowH, true), blk);
    qlogic::TipDraw t;
    if (!qlogic::buildTip(*td, blk, rr, panel, &t, extraPadX)) { tipHide(); return true; }
    const char* titleFont = (t.bigTitle && g_font16Ok) ? FontTitle : FontName;
    const int iconRow = static_cast<int>(absRow) - static_cast<int>(d.off);
    // draw order = creation order: underlay, frame, icon, title, category, dividers, columns, flavor, foot
    if (!g_panelDark) lblHideOne(g_tipFill);   // "Vanilla" panel: the template fill only
    else if (!lblDrawFilled(g_tipFill, TexFill2bt, SbInsetTint, 1.0f, t.fill, false)) return false;
    if (!lblDrawFrame(g_tipFrame, t.frame, TealFrame)) return false;
    if (!lblDrawIcon(g_tipIcon, (iconRow >= 0 && iconRow < static_cast<int>(d.vis)) ? d.icons[iconRow] : "", t.icon)) return false;
    if (!lblDrawText(g_tipTitle, t.titleText, t.title, 9, Cream, titleFont)) {
        if (titleFont != FontTitle) return false;
        g_font16Ok = false;   // no 16x16 font on this label: the 10x10 one from now on
        logLine("TIP dialogfont16x16 unavailable: title in 10x10");
        if (!lblDrawText(g_tipTitle, t.titleText, t.title, 9, Cream, FontName)) return false;
    }
    if (t.subText[0]) { if (!lblDrawText(g_tipSub, t.subText, t.sub, 9, TealDim)) return false; } else lblHideOne(g_tipSub);
    for (unsigned k = 0; k < 3; ++k) {
        if (k < t.nDividers) { if (!lblDrawFilled(g_tipDiv[k], TexFill16g, TealFrame, 1.0f, t.dividers[k], false)) return false; } else lblHideOne(g_tipDiv[k]);
    }
    if (t.haveRules) {
        if (!lblDrawText(g_tipLabels, t.labelsText, t.labels, 9, TealDim) || !lblDrawText(g_tipValues, t.valuesText, t.values, 9, Cream)) return false;
    } else { lblHideOne(g_tipLabels); lblHideOne(g_tipValues); }
    if (t.haveFlavor) { if (!lblDrawText(g_tipBody, t.flavorText, t.flavor, 9, TealText)) return false; } else lblHideOne(g_tipBody);
    if (t.haveFoot) { if (!lblDrawText(g_tipFoot, t.footText, t.foot, 9, TealDim)) return false; } else lblHideOne(g_tipFoot);
    if (g_m.tip.serial != g_tipLoggedSerial && g_tipLogged < TipLogMax) {   // once per new hover target: the flavor label's real extent shows whether the engine re-wrapped
        g_tipLoggedSerial = g_m.tip.serial;
        ++g_tipLogged;
        int ex = 0, ey = 0, ew = 0, eh = 0;
        if (t.haveFlavor) guikit::getExtent(g_tipBody.c, &ex, &ey, &ew, &eh);
        char b[120];
        qlogic::fmtTip(b, sizeof b, g_m.tip.serial, t.lines, ex, ey, ew, eh, t.flipped);
        logLine(b);
    }
    return true;
}

// The multi-row block at blk: hardware bezel, frame, container title, divider, action button,
// d.vis bordered rows (frame, icon, text; the row under the cursor gets the HILIGHT tint and cream text),
// and the scrollbar when the list overflows.
void blockDraw(const qlogic::ListDraw& d, const qlogic::Rect& blk, const qlogic::Rect& panel, uint32_t now, const qlbezel::CategoryStyle& style, qlbezel::Category cat) {
    float cx = 0, cy = 0;
    const bool haveCur = g_input.connected && g_input.cursor_engine(&cx, &cy);
    g_hover = haveCur && qlogic::pointInRect(blk, cx, cy);
    g_hoverRow = g_hover ? qlogic::rowAtPoint(blk, d.vis, cx, cy, d.sb, d.rowH, true) : -1;

    // Hardware bezel (outermost layer)
    const bool showBezel = g_hardwareBezels && (style.frame != qlbezel::FrameNone);
    if (showBezel) {
        const qlogic::Rect bzl = qlogic::bezelOuterRect(blk, true);
        const char* bzlRes = qlbezel::frameResref(static_cast<qlbezel::BezelFrame>(style.frame));
        lblDrawFilled(g_bezel, bzlRes, nullptr, 1.0f, bzl, false);
    } else {
        lblHideOne(g_bezel);
    }

    backFillDraw(d.fill, blk);
    backDraw(blk, style.frameRgb);

    // Container title & scroll indicator
    const qlogic::Rect tr = atBlock(qlogic::titleRect(blk.w), blk);
    const char* title = g_targetName[0] ? g_targetName : qlbezel::categoryName(cat);
    lblDrawText(g_title, title, tr, 9, Cream, (g_font16Ok ? FontTitle : FontName));

    if (d.indicator[0]) {
        const qlogic::Rect ir = { tr.x + tr.w - d.indicatorPx, tr.y + 4, d.indicatorPx + qlogic::LabelSlackPx, tr.h };
        if (!lblDrawText(g_ind, d.indicator, ir, 9, TealDim)) { lblHideAll(); return; }
    } else {
        lblHideOne(g_ind);
    }

    // 2px horizontal divider
    const qlogic::Rect dr = atBlock(qlogic::titleDivRect(blk.w), blk);
    lblDrawFilled(g_headDiv, TexFill16g, style.frameRgb, 1.0f, dr, false);

    // Proposal B framed Action Button
    const qlogic::Rect ar = atBlock(qlogic::actionBtnRect(blk.w), blk);
    const bool hovBtn = haveCur && qlogic::pointInRect(ar, cx, cy);
    lblDrawFrame(g_headFrame, ar, hovBtn ? RowHilight : style.btnRgb, true);

    if (style.led) {
        const qlogic::Rect lr = qlogic::actionLedRect(ar);
        lblDrawFilled(g_headLed, "uibit_fill_circ", style.ledRgb, 1.0f, lr, false);
    } else {
        lblHideOne(g_headLed);
    }

    const qlogic::Rect atr = qlogic::actionTextRect(ar, style.led);
    if (!lblDrawText(g_head, d.header, atr, 9, hovBtn ? Cream : style.textRgb)) { lblHideAll(); return; }

    for (unsigned i = 0; i < qlogic::ListRowsMax; ++i) {
        if (i >= d.vis) { lblHideOne(g_rowFrame[i]); lblHideOne(g_rowIcon[i]); lblHideOne(g_row[i]); lblHideOne(g_rowStack[i]); continue; }
        const bool hov = static_cast<int>(i) == g_hoverRow;
        const qlogic::Rect rr = atBlock(qlogic::rowRect(i, blk.w, d.sb, d.rowH, true), blk);
        if (!lblDrawFrame(g_rowFrame[i], rr, hov ? RowHilight : style.frameRgb, qlogic::rowsEatClicks(d.took, true)) || !lblDrawIcon(g_rowIcon[i], d.icons[i], qlogic::rowIconRect(rr, d.rowH)) ||
            !lblDrawText(g_row[i], d.rows[i], qlogic::rowTextRect(rr, d.rowH), 9, hov ? Cream : style.textRgb)) { lblHideAll(); return; }
        if (d.stacks[i][0]) {   // QL10: the stack column, right-aligned by placement
            const qlogic::Rect sr = qlogic::rowStackRect(rr, d.rowH);
            const int tw = qlogic::textPx10(d.stacks[i]);
            if (!lblDrawText(g_rowStack[i], d.stacks[i], { sr.x + sr.w - tw, sr.y, tw + qlogic::LabelSlackPx, sr.h }, 9, hov ? Cream : style.textRgb)) { lblHideAll(); return; }
        } else {
            lblHideOne(g_rowStack[i]);
        }
    }
    if (!sbDraw(d, blk, style.frameRgb)) { lblHideAll(); return; }
    g_blockPub.shown = true; g_blockPub.took = d.took; g_blockPub.sb = d.sb; g_blockPub.blk = blk; g_blockPub.vis = d.vis; g_blockPub.tick = GetTickCount();
    const int extraPadX = showBezel ? qlogic::BezelInsetPx : 0;
    if (!tipDraw(d, blk, panel, now, extraPadX)) { lblHideAll(); return; }
    g_listRect[0] = blk.x; g_listRect[1] = blk.y; g_listRect[2] = blk.w; g_listRect[3] = blk.h;
    g_listScroll = d.scrollable ? 1 : 0;
    g_listTick = GetTickCount();
    logExtent(g_head, g_extentHead, d.header);
    logExtent(g_row[0], g_extentRow, d.rows[0]);
}

// One-line fallback (row labels unavailable): the old single text line in the header label on the same frame, in the rect blk the anchor gave
// (width = text + pads, one header row high).
void lineDraw(const char* text, const qlogic::Rect& blk) {
    backFillDraw(qlogic::fillRect({ 0, 0, blk.w, blk.h }), blk);
    backDraw(blk);
    if (!lblDrawText(g_head, text, { blk.x + qlogic::BlockPadX, blk.y + qlogic::BlockPadY, blk.w - 2 * qlogic::BlockPadX + qlogic::LabelSlackPx, qlogic::HeaderH }, 9, TealText)) { lblHideAll(); return; }
    logExtent(g_head, g_extentHead, text);
}

// Left click on the block drawn last: the SbPart (as int) of the scrollbar the cursor is over at the press edge, else 0; a press on an item row of the PEEK
// list (not the Took list) is parked in g_rowClickPending for Env::rowClick(). Polled every FRAME tick so the edge state tracks the button even while no list
// is up. Same method as action-hotbar's slot clicks: guikit::clickEdge (LMB down AND the game window is this process's foreground window) + the cursor in engine
// px (gui-input) against the drawn rects; the scrollbar column and (peek list) the row frames are the labels that eat the click, so it never reaches the world.
// A press that started elsewhere never fires (edge only).
int sbClickPoll() {
    g_rowClickPending = -1;
    const bool edge = guikit::clickEdge(g_clickEdge);
    if (!edge || !g_blockPub.shown || GetTickCount() - g_blockPub.tick > qlogic::WheelFreshMs) return 0;
    float cx = 0, cy = 0;
    if (!g_input.connected || !g_input.cursor_engine(&cx, &cy)) return 0;
    if (g_sbPub.shown) {
        const int part = static_cast<int>(qlogic::sbHitAt(g_sbPub.sb, cx - static_cast<float>(g_sbPub.blk.x), cy - static_cast<float>(g_sbPub.blk.y)));
        if (part != 0) return part;
    }
    if (qlogic::actionBtnHitAt(g_blockPub.blk, cx, cy, true)) {
        g_actionBtnClicked = true;
        return 0;
    }
    g_rowClickPending = qlogic::rowClickAt(g_blockPub.blk, g_blockPub.vis, g_blockPub.sb, g_blockPub.took, cx, cy, qlogic::RowH, true);
    return 0;
}

// gui-input wheel callback: consumes the wheel only while the list is up, scrollable, drawn within WheelFreshMs and the cursor (engine px, the
// same space as GuiKit extents) is inside the block; otherwise 0 so the camera zoom still gets it. The HUD tick applies the sum (listTick).
int32_t KPATCH_CALL listWheelCb(const K2WheelEvent* ev, void*) {
    if (!ev || ev->struct_size < sizeof(K2WheelEvent) || !g_listScroll || g_listRect[2] <= 0) return 0;
    if (GetTickCount() - g_listTick > qlogic::WheelFreshMs) return 0;
    const float x = ev->engine_x, y = ev->engine_y;
    const float rx = static_cast<float>(g_listRect[0]), ry = static_cast<float>(g_listRect[1]);
    const bool inside = x >= rx && x < rx + static_cast<float>(g_listRect[2]) && y >= ry && y < ry + static_cast<float>(g_listRect[3]);
    static int wheelLogs = 0;   // probe: proves the hit test in engine px against the drawn block (first 20 wheel events while scrollable)
    if (wheelLogs < 20) {
        ++wheelLogs;
        KP_LOGV(chMain, KPLOG_INFO, "WHEEL delta=%d engine=(%.0f,%.0f) client=(%d,%d) rect=[%ld,%ld,%ld,%ld] inside=%d", ev->delta, x, y,
                ev->client_x, ev->client_y, (long)g_listRect[0], (long)g_listRect[1], (long)g_listRect[2], (long)g_listRect[3], (int)inside);
    }
    if (!inside) return 0;
    InterlockedExchangeAdd(&g_wheelAccum, ev->delta);
    return 1;
}

bool g_wheelSub = false;
long g_wheelNextTry = 0;
unsigned g_wheelFails = 0, g_noProvFrames = 0;
bool g_noProvLogged = false;

// Per HUD frame: connect (poll), subscribe the wheel (retry every 60 frames, the provider WARNs per attempt), keep the provider's window hook alive.
void ensureWheel(long frame) {
    g_input.poll();
    if (g_input.connected) {
        if (!g_wheelSub) {
            if (frame >= g_wheelNextTry) {
                const int32_t tok = g_input.wheel_subscribe(listWheelCb, nullptr);
                if (tok > 0) {
                    g_wheelSub = true;
                    char b[64];
                    qlogic::fmtWheelSub(b, sizeof b, tok, g_wheelFails);
                    logLine(b);
                } else {
                    ++g_wheelFails;
                    g_wheelNextTry = frame + 60;
                    if (g_wheelFails == 1) KP_LOGV(chMain, KPLOG_WARN, "WHEEL-SUB failed (%d): retrying every 60 frames, no list scrolling until it succeeds", static_cast<int>(tok));
                }
            }
        } else {
            (void)g_input.game_hwnd();   // subscribe is one-shot: this lets the provider re-subclass a recreated game window
        }
        return;
    }
    if (!g_noProvLogged && ++g_noProvFrames >= 600) {   // poll() retries the connect every 256 calls; log once if it never came
        g_noProvLogged = true;
        logLine("WHEEL-SUB unavailable: gui-input provider not connected, the list does not scroll (first rows only)");
    }
}

using Fn0 = uint32_t(__thiscall*)(uintptr_t self);
using OpenInvFn = void(__thiscall*)(uintptr_t plc, uint32_t opener, int playAnim);

// u02 recipe A (same chain as CompanionAI.cpp controlledId + leader object + area). ok = leader object and its area resolved.
bool readWorld(qlogic::World& w) {
    w = qlogic::World();
    const uintptr_t srv = serverApp();
    w.haveSrv = srv != 0;
    uint32_t app = 0, client = 0, inner = 0;
    if (!srv || !rd<uint32_t>(gameaddr::AppGlobal, 0, &app) || !app || !rd<uint32_t>(app, App_Client, &client) || !client ||
        !rd<uint32_t>(client, Client_Inner, &inner) || !inner)
        return false;
    if (!saferead::probeRead(SiteWorld, reinterpret_cast<const void*>(inner + Inner_PartyTable), 4)) return false;
    const uintptr_t list = reinterpret_cast<Fn0>(gameaddr::ClientPartyTable)(client);
    if (!list || !saferead::probeRead(SiteWorld, reinterpret_cast<const void*>(list), 0x28)) return false;   // [list] count, [list+0x24] entry 0
    const uintptr_t p = reinterpret_cast<LookupFn>(gameaddr::PartyGetAt)(list, 0);
    if (!p || !saferead::probeRead(SiteWorld, reinterpret_cast<const void*>(p), 0x100)) return false;
    const uintptr_t q = reinterpret_cast<Fn0>(gameaddr::MemberServerCreature)(p);
    if (!q || !rd<uint32_t>(q, Obj_Id, &w.ctrl) || !w.ctrl) return false;
    const uintptr_t leader = reinterpret_cast<LookupFn>(GetGameObject)(srv, w.ctrl);
    if (!leader || !rd<float>(leader, Obj_PosX, &w.x) || !rd<float>(leader, Obj_PosY, &w.y)) return false;
    w.area = reinterpret_cast<Fn0>(gameaddr::ServerObjectGetArea)(leader);
    w.ok = w.area != 0;
    return w.ok;
}

// Full-screen menu check = action_hotbar's live-verified gate (ActionHotbar.cpp gateEval): the HUD must be the top non-ignored panel of the GUI
// manager and no non-ignored modal may be open; messages / fade / pause / auto-pause panels do not count. 0.5.0 read the clip stack index here
// instead: it is only meaningful inside a draw batch (0xFFFF after any batch closed), so in K2HUD_FRAME it always said "menu" and the peek never
// ran (live log 10-28: SUPPRESS none>menu, then nothing). Returns the reason text ("" = HUD on top) for the change-only GATE log.
constexpr int Hud_Manager = 0x1C, Panel_Flags = 0x48, PanelIgnoredByInput = 0x600;
constexpr int Mgr_Panels = 0x88, Mgr_PanelCount = 0x8C, Mgr_Modals = 0x94, Mgr_ModalCount = 0x98;
const char* hudBlocked(uintptr_t hud, uint32_t* topVt) {
    *topVt = 0;
    if (!hud) return "no-hud";
    uint32_t mgr = 0;
    if (!rd<uint32_t>(hud, Hud_Manager, &mgr) || !mgr) return "no-manager";
    auto ignored = [](uintptr_t p) { uint32_t f = 0; return rd<uint32_t>(p, Panel_Flags, &f) && (f & PanelIgnoredByInput) != 0; };
    auto allowed = [](uintptr_t p) {
        uint32_t vt = 0;
        if (!rd<uint32_t>(p, 0, &vt)) return false;
        return vt == gameaddr::MessagesPanelVtable || vt == gameaddr::FadePanelVtable || vt == gameaddr::PausePanelVtable || vt == gameaddr::AutoPausePanelVtable;
    };
    uint32_t modals = 0, panels = 0;
    int modalCount = 0, panelCount = 0;
    if (!rd<uint32_t>(mgr, Mgr_Modals, &modals) || !rd<int>(mgr, Mgr_ModalCount, &modalCount) || modalCount < 0 || modalCount > 64) return "bad-modals";
    for (int i = modalCount - 1; i >= 0; --i) {
        uint32_t p = 0;
        if (!rd<uint32_t>(modals, i * 4, &p) || !p) return "bad-modal-ptr";
        if (!ignored(p) && !allowed(p)) { rd<uint32_t>(p, 0, topVt); return "modal-open"; }
    }
    if (!rd<uint32_t>(mgr, Mgr_Panels, &panels) || !rd<int>(mgr, Mgr_PanelCount, &panelCount) || panelCount < 0 || panelCount > 256) return "bad-panels";
    for (int i = panelCount - 1; i >= 0; --i) {
        uint32_t p = 0;
        if (!rd<uint32_t>(panels, i * 4, &p) || !p) return "bad-panel-ptr";
        if (p == hud) return "";
        if (ignored(p) || allowed(p)) continue;
        rd<uint32_t>(p, 0, topVt);
        return "other-panel-on-top";
    }
    return "hud-not-in-list";
}

qlogic::Sup currentSuppress(const K2HudContext* ctx, bool worldOk) {
    const uintptr_t hud = ctx ? static_cast<uintptr_t>(static_cast<uint32_t>(ctx->hud)) : 0;
    uint32_t topVt = 0;
    const char* why = hudBlocked(hud, &topVt);
    static const char* lastWhy = nullptr;
    static uint32_t lastVt = 0;
    if (why != lastWhy || topVt != lastVt) {   // change-only: names the panel that blocks the peek (vtable) so menus can be told apart in the log
        lastWhy = why; lastVt = topVt;
        KP_LOGV(chMain, KPLOG_INFO, "GATE %s top_vt=%08x hud=%08x", *why ? why : "hud-on-top", (unsigned)topVt, (unsigned)hud);
    }
    uint8_t dlg = 0;
    rd<uint8_t>(DialogByte, 0, &dlg);
    const bool paused = actq::playerPaused(SitePause);
    const int combat = ctrace::readHudCombat(SiteWorld + 1, ctx ? static_cast<uintptr_t>(static_cast<uint32_t>(ctx->hud)) : 0);
    return qlogic::suppressReason(worldOk, *why != 0, dlg != 0, paused, combat);
}

bool takeKeyDown() {
    if (!(GetAsyncKeyState(g_takeKey) & 0x8000)) return false;
    HWND fg = GetForegroundWindow();
    DWORD pid = 0;
    if (!fg) return false;
    GetWindowThreadProcessId(fg, &pid);
    return pid == GetCurrentProcessId();   // a press in another window never counts
}

bool tagInList(const char* tag, const char* list) {
    if (!tag || !tag[0] || !list || !list[0]) return false;
    const char* p = list;
    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == ',' || *p == ';') ++p;
        if (!*p) break;
        const char* start = p;
        while (*p && *p != ' ' && *p != '\t' && *p != ',' && *p != ';') ++p;
        const size_t len = static_cast<size_t>(p - start);
        const size_t tlen = strlen(tag);
        if (len == tlen) {
            bool match = true;
            for (size_t i = 0; i < len; ++i) {
                const char a = (start[i] >= 'A' && start[i] <= 'Z') ? static_cast<char>(start[i] + 32) : start[i];
                const char b = (tag[i] >= 'A' && tag[i] <= 'Z') ? static_cast<char>(tag[i] + 32) : tag[i];
                if (a != b) { match = false; break; }
            }
            if (match) return true;
        }
    }
    return false;
}

// QL6 Give / quest hand-in container detection:
// (a) if gui != 0, read container_p at gui + 0x54; if read succeeds and (flags & 1) == 0, return true (Give mode active).
// (b) if plc != 0, read CExoString at plc + Plc_OnInvDisturbed (0x30C); if non-empty string, return true (quest hand-in container).
// (c) if plc != 0, check placeable tag at plc + Plc_Tag against g_giveTags list read from quick_loot.ini.
bool isGiveContainer(uintptr_t gui, uintptr_t plc) {
    if (gui != 0) {
        uint32_t cont = 0;
        if (rd<uint32_t>(gui, qlogic::Gui_ContainerOffset, &cont) && cont != 0) {
            uint32_t flags = 0;
            if (rd<uint32_t>(cont, qlogic::Container_ModeFlags, &flags)) {
                if ((flags & 1) == 0) return true;
            }
        }
    }
    if (plc != 0) {
        char onDist[64];
        if (saferead::readExo(Site, plc + qlogic::Plc_OnInvDisturbed, onDist, sizeof onDist, 63, false) > 0 && onDist[0] != '\0') {
            return true;
        }
        if (g_giveTags[0] != '\0') {
            char tag[64];
            if (saferead::readExo(Site, plc + Plc_Tag, tag, sizeof tag, 63, false) > 0 && tag[0] != '\0') {
                if (tagInList(tag, g_giveTags)) return true;
            }
        }
    }
    return false;
}

bool readCandFlags(uintptr_t plc, qlogic::CandFlags& f) {
    uint8_t locked = 0, open = 0;
    uint32_t inv = 0, trap = 0, use = 0;
    if (!rd<uint32_t>(plc, Plc_HasInv, &inv) || !rd<uint8_t>(plc, Plc_Locked, &locked) || !rd<uint32_t>(plc, Plc_Trap, &trap) ||
        !rd<uint32_t>(plc, Plc_Useable, &use) || !rd<uint8_t>(plc, Plc_Open, &open))
        return false;
    f.hasInv = inv != 0; f.locked = locked != 0; f.trap = trap != 0; f.useable = use != 0; f.open = open != 0;
    f.isGive = isGiveContainer(0, plc);
    return true;
}
inline bool readCandFlags(qlogic::CandFlags* f, uintptr_t plc) {
    return f ? readCandFlags(plc, *f) : false;
}

uint32_t readHoverId() {
    uint32_t app = 0, client = 0, inner = 0, hoverId = 0x7F000000u;
    if (!rd<uint32_t>(gameaddr::AppGlobal, 0, &app) || !app) return 0x7F000000u;
    if (!rd<uint32_t>(app, App_Client, &client) || !client) return 0x7F000000u;
    if (!rd<uint32_t>(client, Client_Inner, &inner) || !inner) return 0x7F000000u;
    if (!rd<uint32_t>(inner, Inner_HoverObj, &hoverId)) return 0x7F000000u;
    return hoverId;
}

// The engine side of qlogic::Env (QuickLootMachine.h): every read goes through SafeRead, every action is the same engine call as before.
struct EngineEnv : qlogic::Env {
    const K2HudContext* ctx = nullptr;
    uintptr_t srv = 0;   // server app, cached by areaList for the scan that follows
    void enter() override { ::enter(); }
    bool readWorld(qlogic::World& w) override { return ::readWorld(w); }
    qlogic::Sup suppress(bool worldOk) override { return currentSuppress(ctx, worldOk); }
    bool keyDown() override { const bool k = takeKeyDown() || g_actionBtnClicked; g_actionBtnClicked = false; return k; }
    bool hudOk() override { g_hud.poll(); return g_hud.connected; }
    bool shiftDown() override { return (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }
    uintptr_t resolvePlc(uint32_t sid) override {
        const uintptr_t s = serverApp();
        return s ? reinterpret_cast<LookupFn>(GetPlaceableByGameObjectID)(s, sid) : 0;
    }
    uintptr_t resolveClientPlc(uint32_t clientObj) override { return placeableFor(clientObj); }
    uint32_t serverIdForObj(uint32_t clientObj) override {
        const uintptr_t s = serverApp();
        return s ? reinterpret_cast<LookupFn>(gameaddr::ClientToServerObjectId)(s, clientObj) : 0;
    }
    uint32_t hoverObjectId() override { return readHoverId(); }
    void setEmptyCursor(bool empty) override {
        if (!empty) return;
        uint32_t app = 0, client = 0, inner = 0;
        if (!rd<uint32_t>(gameaddr::AppGlobal, 0, &app) || !app) return;
        if (!rd<uint32_t>(app, App_Client, &client) || !client) return;
        if (!rd<uint32_t>(client, Client_Inner, &inner) || !inner) return;
        using SetMouseCursorFn = void(__thiscall*)(uintptr_t app, int id, int a, int b);
        reinterpret_cast<SetMouseCursorFn>(SetMouseCursor)(inner, 1, 0, 0);
    }
    bool plcPos(uintptr_t plc, float* x, float* y) override { return rd<float>(plc, Obj_PosX, x) && rd<float>(plc, Obj_PosY, y); }
    int itemCount(uintptr_t plc) override { return ::itemCount(plc); }
    int openFlag(uintptr_t plc) override { return ::openFlag(plc); }
    uintptr_t msgObject() override { return messageObject(); }
    void capture(uintptr_t plc, qlogic::Captured& c) override { captureItems(plc, c); }
    bool areaList(const qlogic::World& w, int* count) override {
        uint32_t arr = 0;
        srv = serverApp();
        return rd<uint32_t>(w.area, Area_Ids, &arr) && arr && rd<int>(w.area, Area_Count, count);
    }
    bool areaRawCount(const qlogic::World& w, int* count) override { return rd<int>(w.area, Area_Count, count); }
    bool areaObject(const qlogic::World& w, int i, uint32_t* id, float* x, float* y) override {
        uint32_t arr = 0;
        *id = 0;
        if (!rd<uint32_t>(w.area, Area_Ids, &arr) || !arr || !rd<uint32_t>(arr, i * 4, id) || !*id) return false;
        const uintptr_t o = reinterpret_cast<LookupFn>(GetGameObject)(srv, *id);
        uint8_t type = 0;
        if (!o || !rd<uint8_t>(o, Obj_Type, &type) || type != ObjTypePlaceable) return false;
        return rd<float>(o, Obj_PosX, x) && rd<float>(o, Obj_PosY, y);
    }
    bool candFlags(uint32_t id, uintptr_t* plc, qlogic::CandFlags* f) override {
        *plc = reinterpret_cast<LookupFn>(GetPlaceableByGameObjectID)(srv, id);
        return *plc && readCandFlags(*plc, *f);
    }
    void openInventory(uintptr_t plc, uint32_t opener) override { reinterpret_cast<OpenInvFn>(OpenInventory)(plc, opener, 1); }
    void sendClose(uintptr_t msg, uint32_t obj, int flag) override { reinterpret_cast<SendCloseFn>(SendContainerClose)(msg, obj, flag); }
    void serverClose(uintptr_t plc, uint32_t opener, int flag) override {
        using CloseInvFn = void(__thiscall*)(uintptr_t plc, uint32_t player, int flag);
        reinterpret_cast<CloseInvFn>(CloseInventory)(plc, opener, flag);
    }
    void labelShow(const char*) override {}   // the label is drawn from g_m.labelText() in onMenuPost
    void labelHide() override {}
    void toast(const char*) override {}       // the Took line lives in g_m.toastS (text + duration); drawn in onMenuPost
    void log(const char* line) override { if (!strncmp(line, "SCAN", 4)) probeLine(line); else logLine(line); }
    int wheelDelta() override { return static_cast<int>(InterlockedExchange(&g_wheelAccum, 0)); }
    bool overList() override { return g_hover; }
    int sbClick() override { return sbClickPoll(); }
    int rowClick() override { const int r = g_rowClickPending; g_rowClickPending = -1; return r; }
    int hoverRow() override { return g_blockPub.shown && !g_blockPub.took ? g_hoverRow : -1; }
    bool takeOne(uintptr_t plc, uint32_t opener, unsigned index, uint32_t expectId, qlogic::TakeOneOut* out) override { return engineTakeOne(plc, opener, index, expectId, out); }
    bool describe(uintptr_t plc, unsigned index, uint32_t expectId, qlogic::TipData* out) override { return engineDescribe(plc, index, expectId, out); }
    void sound(int idx) override { playGuiSound(static_cast<char>(idx)); }
};
EngineEnv g_env;


// Mode 1: the world point of the container the list belongs to: the peek's (or the pending click-take's) server placeable position, kept while the Took
// list of that container is up. Cleared when no label is up.
void updateProjWorld() {
    const uintptr_t plc = (g_m.peek.active || g_m.peek.expect) ? g_m.peek.plc : (g_m.take.active ? g_m.take.plc : 0);
    if (!plc) return;
    World3 w;
    w.ok = rd<float>(plc, Obj_PosX, &w.x) && rd<float>(plc, Obj_PosY, &w.y) && rd<float>(plc, Obj_PosZ, &w.z) && std::isfinite(w.x) && std::isfinite(w.y) && std::isfinite(w.z) &&
           fabsf(w.z) < 100000.0f;
    if (w.ok) g_projWorld = w;
}

// K2HUD_TARGET_MENU_POST: runs after the engine positioned the target name tag (0x743850) and before every drawn HUD frame (also with no target). 0.6.0-dev (QL2A):
// the name rect read here is THIS frame's, and the block is re-placed from it every frame (QuickLootMachine.h placeBlock; no latch). What is drawn: the Took
// line (while up) or the peek list.
void KPATCH_CALL onMenuPost(const K2HudContext* ctx, void*) {
    if (!g_head.c || !ctx) return;
    if (ctx->hud != g_panel) { lblDrop(); return; }   // stale HUD: no writes to the old labels
    saferead::beginScope();
    const uint32_t now = nowMs();
    const char* text = g_m.labelText(now);
    if (!text) { g_projWorld.ok = false; lblHideAll(); return; }
    const uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    uint8_t flags = 0;
    qlogic::Geometry g;
    if (menu) {
        saferead::readExo(Site, menu + 0x1758, g_targetName, sizeof g_targetName, 79, false);
        if (saferead::probeWrite(Site, reinterpret_cast<void*>(menu + 0x1BAC), sizeof(uint32_t))) {
            *(uint32_t*)(menu + 0x1BAC) &= ~1u;   // 0.8.0-dev (QL14): suppress vanilla 533px name and vitality bar
        }
        if (rd<uint8_t>(menu, Menu_Flags, &flags) && (flags & 1)) {
            int32_t mx = 0, my = 0, nx = 0, ny = 0, nw = 0, nh = 0;
            if (rd<int32_t>(menu, Menu_OriginX, &mx) && rd<int32_t>(menu, Menu_OriginY, &my) && rd<int32_t>(menu, Menu_NameX, &nx) &&
                rd<int32_t>(menu, Menu_NameY, &ny) && rd<int32_t>(menu, Menu_NameW, &nw) && rd<int32_t>(menu, Menu_NameH, &nh)) {
                g.name = { mx + nx, my + ny, nw, nh };   // screen rect of the name label, as the engine just positioned it
                g.menuShown = true;
            }
        }
    }
    if (g_targetName[0] == '\0' && g_m.peek.active && g_m.peek.plc) {
        readPlaceableName(g_m.peek.plc, clientLanguage(), g_targetName, sizeof g_targetName);
    }
    int px = 0, py = 0, pw = 0, ph = 0;
    if (guikit::getExtent(static_cast<uintptr_t>(static_cast<uint32_t>(g_panel)), &px, &py, &pw, &ph) && pw > 0 && ph > 0) {
        g.havePanel = true;
        g.panel = { px, py, pw, ph };
    }
    if (g_m.cfg.anchor == 1 || !g.menuShown) {   // mode 1, or no name tag this frame: the projection is the first fallback (anchorBlock)
        updateProjWorld();
        projectWorld(g_projWorld, &g.proj);
    }
    const qlogic::Rect panel = g.havePanel ? g.panel : qlogic::Rect{ 0, 0, 0, 0 };
    qlogic::ListDraw d;
    qlogic::Rect blk = { 0, 0, 0, 0 };
    if (g_rowsOk && g_m.listDraw(now, &d, true)) {   // the multi-row block with unified header
        if (!g_m.placeBlock(g_env, g, d.w, d.h, now, &blk)) { lblHideAll(); return; }
        uint32_t isBodyBag = 0, isCorpse = 0;
        uint16_t app = 0;
        char tag[64] = {0}, resref[17] = {0};
        const uintptr_t plc = g_peek.plc;
        if (plc) {
            rd<uint32_t>(plc, 0x498, &isBodyBag);
            rd<uint32_t>(plc, 0x4A4, &isCorpse);
            rd<uint16_t>(plc, 0x270, &app);
            saferead::readExo(Site, plc + 0x18, tag, sizeof tag, 63, false);
            saferead::readResRef16(Site, plc + 0x3D0, resref, 0);
        }
        const qlbezel::Category cat = qlbezel::classifyPlaceable(isBodyBag, isCorpse, app, g_targetName, tag, resref);
        qlbezel::CategoryStyle style = g_catStyles[cat];
        if (!g_hardwareBezels) style.frame = qlbezel::FrameNone;
        blockDraw(d, blk, panel, now, style, cat);
        return;
    }
    // fallback: the one-line text (row labels missing)
    if (!g_m.placeBlock(g_env, g, qlogic::textPx10(text) + 2 * qlogic::BlockPadX, qlogic::HeaderH + 2 * qlogic::BlockPadY, now, &blk)) { lblHideAll(); return; }
    lineDraw(text, blk);
}

// ---- 0.6.0-dev (QL7): the Mod Configuration page (QuickLootPage.h, selfheal pattern: ModConfigDecl + TomlMini + IniWrite, lazy-connected from the HUD frame).
// Every row applies live: pageOnChange updates g_live and copies it into the machine's config. The ini-bound rows (anchor, prox, rows) are written by the
// declarative layer itself; the unbound "dist" row writes OpenM here (CloseM follows as OpenM + 2 unless the ini has its own CloseM). Page ini edits made by
// hand are read once at start (KPatchInit); the page is the live editor.
k2mc::ConfigClient g_mcCfg;
k2mc::MenuClient g_mcMenu;
k2pc::RegisterOnce g_mcReg;
mcdecl::Page g_page;
K2ModConfigEntry g_mcEntry;
bool g_pageLoaded = false;
qlpage::Live g_live;
int g_pageLogs = 0;

void mcLog(int level, const char* msg) { KP_LOGV(chMain, level, "%s", msg); }

void applyLive() {
    g_proxOn = g_live.prox;
    g_openM = g_live.openM; g_closeM = g_live.closeM;
    g_m.cfg.anchor = g_live.anchor; g_m.cfg.proxOn = g_live.prox; g_m.cfg.listRows = g_live.rows;
    g_m.cfg.openM = g_live.openM; g_m.cfg.closeM = g_live.closeM;
    g_tipDetail = g_live.tip; g_sortFirst = g_live.sort; g_equipLine = g_live.equip; g_panelDark = g_live.panel != 0;
    g_hardwareBezels = g_live.bezels;
}

const char* const kCatChoices[qlbezel::CatCount] = {
    "Droids & Wreckage",
    "Weapon Racks & Armory",
    "Medical & Lab",
    "Strongboxes & Vaults",
    "Remains / Corpses",
    "Footlockers & Lockers",
    "Standard Containers"
};

const char* const kFrameChoices[qlbezel::FrameCount] = {
    "None (Plain Border)",
    "Datapad",
    "Computer Console",
    "Upgrade Workbench",
    "Lab Station",
    "Merchant Store",
    "Gold Pause Bezel"
};

const char* const kPalChoices[qlbezel::PalCount] = {
    "Datapad Teal",
    "Terminal Blue",
    "Workbench Cobalt",
    "Lab Emerald",
    "Store Bronze",
    "Classic Gold",
    "Custom RGB"
};

int32_t KPATCH_CALL pvModalChange(uint32_t page, int32_t item, int32_t requested, int32_t oldVal, void* user) {
    (void)page; (void)oldVal; (void)user;
    if (item == 0) { // cat_sel
        g_selectedCat = (requested >= 0 && requested < qlbezel::CatCount) ? requested : 0;
        const qlbezel::CategoryStyle& s = g_catStyles[g_selectedCat];
        if (g_mcMenu.ok() && g_modalPage) {
            g_mcMenu.p->SetValue(g_modalPage, 1, s.frame);
            g_mcMenu.p->SetValue(g_modalPage, 2, s.palette);
            g_mcMenu.p->SetValue(g_modalPage, 3, s.led ? 1 : 0);
            g_mcMenu.p->SetValue(g_modalPage, 4, static_cast<int>(lroundf(s.frameRgb[0] * 100.0f)));
            g_mcMenu.p->SetValue(g_modalPage, 5, static_cast<int>(lroundf(s.frameRgb[1] * 100.0f)));
            g_mcMenu.p->SetValue(g_modalPage, 6, static_cast<int>(lroundf(s.frameRgb[2] * 100.0f)));
        }
        return g_selectedCat;
    }
    qlbezel::CategoryStyle& s = g_catStyles[g_selectedCat];
    if (item == 1) { // cat_frame
        s.frame = (requested >= 0 && requested < qlbezel::FrameCount) ? requested : 0;
        char k[32], v[16]; snprintf(k, sizeof k, "Cat_%d_Frame", g_selectedCat); snprintf(v, sizeof v, "%d", s.frame);
        iniw::updateFile(qlpage::kIniFile, qlpage::kSection, k, v);
        return s.frame;
    }
    if (item == 2) { // cat_pal
        s.palette = (requested >= 0 && requested < qlbezel::PalCount) ? requested : 0;
        if (s.palette < qlbezel::PalCustom) {
            qlbezel::getPaletteColors(static_cast<qlbezel::Palette>(s.palette), s.frameRgb, s.btnRgb, s.textRgb, s.ledRgb);
            if (g_mcMenu.ok() && g_modalPage) {
                g_mcMenu.p->SetValue(g_modalPage, 4, static_cast<int>(lroundf(s.frameRgb[0] * 100.0f)));
                g_mcMenu.p->SetValue(g_modalPage, 5, static_cast<int>(lroundf(s.frameRgb[1] * 100.0f)));
                g_mcMenu.p->SetValue(g_modalPage, 6, static_cast<int>(lroundf(s.frameRgb[2] * 100.0f)));
            }
        }
        char k[32], v[16]; snprintf(k, sizeof k, "Cat_%d_Palette", g_selectedCat); snprintf(v, sizeof v, "%d", s.palette);
        iniw::updateFile(qlpage::kIniFile, qlpage::kSection, k, v);
        return s.palette;
    }
    if (item == 3) { // cat_led
        s.led = requested != 0;
        char k[32], v[16]; snprintf(k, sizeof k, "Cat_%d_Led", g_selectedCat); snprintf(v, sizeof v, "%d", s.led ? 1 : 0);
        iniw::updateFile(qlpage::kIniFile, qlpage::kSection, k, v);
        return s.led ? 1 : 0;
    }
    if (item >= 4 && item <= 6) { // RGB steppers
        s.palette = qlbezel::PalCustom;
        if (g_mcMenu.ok() && g_modalPage) g_mcMenu.p->SetValue(g_modalPage, 2, qlbezel::PalCustom);
        const float val = (requested < 0 ? 0 : (requested > 100 ? 100 : requested)) / 100.0f;
        s.frameRgb[item - 4] = s.btnRgb[item - 4] = val;
        char k[32], v[32];
        snprintf(k, sizeof k, "Cat_%d_RGB", g_selectedCat);
        snprintf(v, sizeof v, "%.2f,%.2f,%.2f", (double)s.frameRgb[0], (double)s.frameRgb[1], (double)s.frameRgb[2]);
        iniw::updateFile(qlpage::kIniFile, qlpage::kSection, k, v);
        return requested;
    }
    return requested;
}

void pvBuildModal(const K2MenuUiApi* ui, uint32_t page) {
    K2MuiRect col = {0, 0, 0, 0}, box = {0, 0, 0, 0};
    const bool haveCol = ui->PaneRect(g_pagePanel, MUI_PANE_COLUMN, &col) && col.w > 0 && col.h > 0;
    if (haveCol) box = col;
    g_modalPage = ui->NewModal(page, haveCol ? &box : nullptr, "CATEGORY BEZEL & COLOR MAPPING");
    if (!g_modalPage) return;

    const qlbezel::CategoryStyle& s = g_catStyles[g_selectedCat];

    K2MuiRowSpec r0 = {}; r0.struct_size = sizeof r0; r0.kind = MUI_ROW_STEPPER;
    r0.flags = MUI_RF_WRAP;
    r0.label = "Category"; r0.desc = "Select loot container category to configure.";
    r0.choices = kCatChoices; r0.nChoices = qlbezel::CatCount; r0.value = g_selectedCat;
    r0.min = 0; r0.max = qlbezel::CatCount - 1; r0.step = 1;
    ui->AddRow(g_modalPage, &r0);

    K2MuiRowSpec r1 = {}; r1.struct_size = sizeof r1; r1.kind = MUI_ROW_STEPPER;
    r1.flags = MUI_RF_WRAP;
    r1.label = "Bezel frame"; r1.desc = "Hardware bezel frame for this category.";
    r1.choices = kFrameChoices; r1.nChoices = qlbezel::FrameCount; r1.value = s.frame;
    r1.min = 0; r1.max = qlbezel::FrameCount - 1; r1.step = 1;
    ui->AddRow(g_modalPage, &r1);

    K2MuiRowSpec r2 = {}; r2.struct_size = sizeof r2; r2.kind = MUI_ROW_STEPPER;
    r2.flags = MUI_RF_WRAP;
    r2.label = "Color palette"; r2.desc = "Color palette preset or custom RGB.";
    r2.choices = kPalChoices; r2.nChoices = qlbezel::PalCount; r2.value = s.palette;
    r2.min = 0; r2.max = qlbezel::PalCount - 1; r2.step = 1;
    ui->AddRow(g_modalPage, &r2);

    K2MuiRowSpec r3 = {}; r3.struct_size = sizeof r3; r3.kind = MUI_ROW_TOGGLE;
    r3.label = "Action button LED"; r3.desc = "Show circular LED status light on action button.";
    r3.value = s.led ? 1 : 0;
    ui->AddRow(g_modalPage, &r3);

    K2MuiRowSpec r4 = {}; r4.struct_size = sizeof r4; r4.kind = MUI_ROW_STEPPER;
    r4.label = "Custom Red %"; r4.desc = "Red component (0..100%). Sets palette to Custom RGB.";
    r4.min = 0; r4.max = 100; r4.step = 5; r4.fmt = "Red: %d%%";
    r4.value = static_cast<int>(lroundf(s.frameRgb[0] * 100.0f));
    ui->AddRow(g_modalPage, &r4);

    K2MuiRowSpec r5 = {}; r5.struct_size = sizeof r5; r5.kind = MUI_ROW_STEPPER;
    r5.label = "Custom Green %"; r5.desc = "Green component (0..100%). Sets palette to Custom RGB.";
    r5.min = 0; r5.max = 100; r5.step = 5; r5.fmt = "Green: %d%%";
    r5.value = static_cast<int>(lroundf(s.frameRgb[1] * 100.0f));
    ui->AddRow(g_modalPage, &r5);

    K2MuiRowSpec r6 = {}; r6.struct_size = sizeof r6; r6.kind = MUI_ROW_STEPPER;
    r6.label = "Custom Blue %"; r6.desc = "Blue component (0..100%). Sets palette to Custom RGB.";
    r6.min = 0; r6.max = 100; r6.step = 5; r6.fmt = "Blue: %d%%";
    r6.value = static_cast<int>(lroundf(s.frameRgb[2] * 100.0f));
    ui->AddRow(g_modalPage, &r6);

    ui->SetChangeHandler(g_modalPage, pvModalChange, nullptr);
    g_gearItem = ui->AddGear(page, "Customize category bezels and colors");
}

void pvRefreshPreview() {
    if (!g_pvOn || !g_mcMenu.ok() || !g_pagePanel) return;
    const K2MenuUiApi* ui = g_mcMenu.p;
    if (!g_cv.begin(ui, g_pagePanel, 850, 480)) {
        g_cv.hideAll();
        return;
    }
    const qlbezel::Category cat = static_cast<qlbezel::Category>(g_selectedCat);
    const qlbezel::CategoryStyle& style = g_catStyles[cat];
    const bool showBezel = g_hardwareBezels && (style.frame != qlbezel::FrameNone);
    const char* bzlResref = qlbezel::frameResref(static_cast<qlbezel::BezelFrame>(style.frame));
    void* bzlImg = (showBezel && bzlResref[0]) ? guikit::imageFor(bzlResref) : nullptr;
    void* fillImg = guikit::imageFor(TexFill2bt);
    void* solidImg = guikit::imageFor(TexFill16g);
    void* ledImg = guikit::imageFor("uibit_fill_circ");

    pvc::Rect blk = { 40, 40, 400, 360 };
    if (showBezel && bzlImg) {
        pvc::Rect bzlRect = { blk.x - 40, blk.y - 36, 480, 432 };
        g_cv.put(0, bzlRect, bzlImg, nullptr, 1.0f);
    }
    float darkFill[3] = { 0.02f, 0.05f, 0.06f };
    g_cv.put(1, blk, fillImg ? fillImg : solidImg, darkFill, 0.95f);
    g_cv.put(2, blk, solidImg, style.frameRgb, 0.8f);

    pvc::Rect tr = { blk.x + 20, blk.y + 16, blk.w - 120, 24 };
    g_cv.put(3, tr, nullptr, nullptr, 1.0f);
    g_cv.text(3, qlbezel::categoryName(cat));

    pvc::Rect cntRect = { blk.x + blk.w - 90, blk.y + 16, 70, 24 };
    g_cv.put(8, cntRect, nullptr, nullptr, 1.0f);
    g_cv.text(8, "3 ITEMS");

    pvc::Rect div = { blk.x + 20, blk.y + 44, blk.w - 40, 2 };
    g_cv.put(4, div, solidImg, style.frameRgb, 1.0f);

    pvc::Rect btn = { blk.x + 20, blk.y + 54, blk.w - 40, 34 };
    g_cv.put(5, btn, solidImg, style.btnRgb, 0.8f);

    if (style.led) {
        pvc::Rect led = { btn.x + 10, btn.y + 9, 16, 16 };
        g_cv.put(6, led, ledImg ? ledImg : solidImg, style.ledRgb, 1.0f);
    }

    pvc::Rect btnTxt = { btn.x + (style.led ? 34 : 14), btn.y + 7, btn.w - 40, 22 };
    g_cv.put(7, btnTxt, nullptr, nullptr, 1.0f);
    g_cv.text(7, "[F] GET ITEMS (3)");

    const char* sampleItems[3] = { "Disruptor Carbine", "Parts x5", "Combat Suit" };
    const char* sampleIcons[3] = { "iw_dsrptrfl_001", "ii_progspike_002", "ia_class4_001" };
    for (int r = 0; r < 3; ++r) {
        pvc::Rect rr = { blk.x + 20, blk.y + 100 + r * 44, blk.w - 40, 38 };
        g_cv.put(9 + r * 3, rr, solidImg, style.frameRgb, 0.5f);
        pvc::Rect ir = { rr.x + 6, rr.y + 4, 30, 30 };
        void* iconImg = guikit::imageFor(sampleIcons[r]);
        g_cv.put(10 + r * 3, ir, iconImg, nullptr, 1.0f);
        pvc::Rect rt = { rr.x + 42, rr.y + 7, rr.w - 50, 24 };
        g_cv.put(11 + r * 3, rt, nullptr, nullptr, 1.0f);
        g_cv.text(11 + r * 3, sampleItems[r]);
    }

    pvc::Rect tip = { blk.x + blk.w + (showBezel ? 40 : 0) + 20, 40, 320, 360 };
    g_cv.put(18, tip, fillImg ? fillImg : solidImg, darkFill, 0.95f);
    g_cv.put(19, tip, solidImg, style.frameRgb, 0.7f);
    pvc::Rect tipTitle = { tip.x + 16, tip.y + 16, tip.w - 32, 24 };
    g_cv.put(20, tipTitle, nullptr, nullptr, 1.0f);
    g_cv.text(20, "Disruptor Carbine");
    pvc::Rect tipSub = { tip.x + 16, tip.y + 44, tip.w - 32, 20 };
    g_cv.put(21, tipSub, nullptr, nullptr, 1.0f);
    g_cv.text(21, "Ranged Weapon (Blaster)");

    pvc::Rect tipBody = { tip.x + 16, tip.y + 74, tip.w - 32, 100 };
    g_cv.put(22, tipBody, nullptr, nullptr, 1.0f);
    g_cv.text(22, "Damage: Energy 2d8\nCritical: 19-20, x2\nRange: 28m");

    g_cv.end();
}

void pvOnBuilt(void*, uint32_t page, void* panel) {
    ensureBezelTextures();
    g_pvOn = false; g_modalPage = 0; g_gearItem = -1; g_pagePanel = panel;
    if (!g_mcMenu.hasModal() || !g_mcMenu.hasDescSplit()) return;
    const K2MenuUiApi* ui = g_mcMenu.p;
    if (!g_cv.build(ui, panel, page, 28)) return;
    guikit::setAlign(g_cv.w[3].c, 9);
    guikit::setAlign(g_cv.w[7].c, 9);
    guikit::setAlign(g_cv.w[8].c, 9);
    for (int i = 0; i < 3; ++i) guikit::setAlign(g_cv.w[11 + i * 3].c, 9);
    guikit::setAlign(g_cv.w[20].c, 9);
    guikit::setAlign(g_cv.w[21].c, 9);
    guikit::setAlign(g_cv.w[22].c, 9);

    if (!ui->SetDescSplit(page, 700)) return;
    pvBuildModal(ui, page);
    g_pvOn = true;
    if (KpLog_HasCore()) KP_LOGV(chMain, KPLOG_INFO, "PREVIEW BUILD ok: 28 labels, split 700 permille");
}

void pvOnTick(void*) {
    if (g_pvOn && g_gearItem >= 0 && g_modalPage && g_mcMenu.ok() && g_mcMenu.p->Clicked(g_page.page, g_gearItem)) {
        g_mcMenu.p->SetModalOpen(g_modalPage, 1);
    }
    embtex::heal();
    pvRefreshPreview();
}

void KPATCH_CALL pvDestroy(void* panel, void* user) {
    if (g_mcMenu.ok()) {
        g_cv.destroy(g_mcMenu.p, g_page.page);
    } else {
        g_cv.reset();
    }
    g_pvOn = false; g_modalPage = 0; g_gearItem = -1; g_pagePanel = nullptr;
    mcdecl::Page::cbDestroy(panel, user);
}

int32_t pageOnChange(void*, int, const char* id, int32_t requested, int) {
    bool persistOpen = false;
    const int32_t fin = qlpage::applyRow(g_live, id, requested, &persistOpen);
    applyLive();
    bool wrote = true;
    if (persistOpen) {
        char v[16];
        snprintf(v, sizeof v, "%.1f", static_cast<double>(g_live.openM));
        wrote = iniw::updateFile(qlpage::kIniFile, qlpage::kSection, "OpenM", v);
    }
    if (g_pageLogs < 60) {
        ++g_pageLogs;
        KP_LOG(chMain, "PAGE row=%s req=%d final=%d anchor=%d prox=%d rows=%u openM=%.1f closeM=%.1f tip=%d sort=%d equip=%d panel=%d ini=%d", id, static_cast<int>(requested), static_cast<int>(fin), g_live.anchor,
               g_live.prox ? 1 : 0, g_live.rows, static_cast<double>(g_live.openM), static_cast<double>(g_live.closeM), g_live.tip, g_live.sort ? 1 : 0, g_live.equip ? 1 : 0, g_live.panel, wrote ? 1 : 0);
    }
    return fin;
}

void pageOnEnter(void*) { g_page.setValue(qlpage::kRowDist, qlpage::distIndexFor(g_live.openM)); }   // the unbound row starts at the nearest preset of the current OpenM

void pageInit(const KPatchApi* api) {
    g_mcCfg.init(api);
    g_mcMenu.init(api);
    g_mcCfg.onConnect = [](bool ok, const char* why) { KP_LOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "PAGE k2.modconfig %s: %s", ok ? "ok" : "unavailable", why); };
    g_mcMenu.onConnect = [](bool ok, const char* why) { KP_LOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "PAGE k2.menuui %s: %s", ok ? "ok" : "unavailable", why); };
    char err[96] = {};
    g_pageLoaded = g_page.load(qlpage::kPageSchema, strlen(qlpage::kPageSchema), err, sizeof err);
    if (!g_pageLoaded) { KP_LOGV(chMain, KPLOG_WARN, "PAGE schema rejected: %s", err); return; }
    g_page.menu = &g_mcMenu;
    g_page.io = mcdecl::fileIo();
    g_page.hooks.onChange = pageOnChange;
    g_page.hooks.onEnter = pageOnEnter;
    g_page.hooks.onBuilt = pvOnBuilt;
    g_page.hooks.onTick = pvOnTick;
    g_mcEntry = g_page.entry(qlpage::kEntryId);
    g_mcEntry.destroy = pvDestroy;
}

// Every frame (cheap): connect lazily, register once, only when the provider has option rows.
void pagePoll() {
    if (!g_pageLoaded || g_mcReg.registered) return;
    g_mcCfg.poll();
    g_mcMenu.poll();
    static unsigned s_polls = 0;
    if (++s_polls == 600 && !(g_mcCfg.ok() && g_mcMenu.ok() && g_mcMenu.hasRows()))   // once, ~10 s of frames in
        KP_LOGV(chMain, KPLOG_WARN, "PAGE not registered after %u polls: modconfig=%d menuui=%d hasRows=%d", s_polls, (int)g_mcCfg.ok(), (int)g_mcMenu.ok(), (int)g_mcMenu.hasRows());
    if (!g_mcCfg.ok() || !g_mcMenu.ok() || !g_mcMenu.hasRows()) return;
    k2pc::registerOnce(g_mcReg, g_mcCfg, g_mcEntry, mcLog);
}

// K2HUD_FRAME: the pending click-take Close, then the proximity peek. Runs on the game thread, outside any EventHandler.
void KPATCH_CALL onHudFrame(const K2HudContext* ctx, void*) {
    pagePoll();
    ensureBezelTextures();
    embtex::heal();
    g_env.ctx = ctx;
    const uint32_t now = nowMs();
    ensureWheel(static_cast<long>(ctx ? ctx->frame : g_m.frame));
    g_m.onFrame(g_env, ctx ? ctx->frame : g_m.frame, now);
    if (!g_m.labelText(now) && g_head.c && ctx && ctx->hud) {   // POST may not run when there is no target: hide an expired list here too
        if (ctx->hud != g_panel) { lblDrop(); return; }
        saferead::beginScope();
        lblHideAll();
    }
}

}   // namespace

extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_enabled = GetPrivateProfileIntA("QuickLoot", "Enabled", 1, IniPath) != 0;
    g_verbose = GetPrivateProfileIntA("QuickLoot", "Verbose", 0, IniPath) != 0;
    g_proxOn = GetPrivateProfileIntA("QuickLoot", "ProximityEnabled", 1, IniPath) != 0;
    g_takeKey = qlogic::clampTakeKey(GetPrivateProfileIntA("QuickLoot", "TakeKey", qlogic::DefaultTakeKey, IniPath));
    g_scanMs = qlogic::clampScanMs(GetPrivateProfileIntA("QuickLoot", "ScanMs", static_cast<int>(qlogic::DefaultScanMs), IniPath));
    char fb[32];
    GetPrivateProfileStringA("QuickLoot", "OpenM", "3.0", fb, sizeof fb, IniPath);
    g_openM = qlogic::clampOpenM(static_cast<float>(atof(fb)));
    GetPrivateProfileStringA("QuickLoot", "CloseM", "", fb, sizeof fb, IniPath);   // 0.6.0-dev: absent = OpenM + 2 (the page's distance presets move both); present = an explicit override
    const bool closeExplicit = fb[0] != 0;
    const float closeExplicitM = closeExplicit ? static_cast<float>(atof(fb)) : qlogic::DefaultCloseM;
    g_closeM = qlpage::closeFor(g_openM, closeExplicit, closeExplicitM);
    g_m.cfg.enabled = g_enabled; g_m.cfg.proxOn = g_proxOn; g_m.cfg.openM = g_openM; g_m.cfg.closeM = g_closeM; g_m.cfg.takeKey = g_takeKey; g_m.cfg.scanMs = g_scanMs;
    g_peekOnSelect = GetPrivateProfileIntA("QuickLoot", "peek_on_select", 0, IniPath) != 0;
    if (!g_peekOnSelect) g_peekOnSelect = GetPrivateProfileIntA("QuickLoot", "PeekOnSelect", 0, IniPath) != 0;
    int timeout = GetPrivateProfileIntA("QuickLoot", "click_peek_timeout", 0, IniPath);
    if (!timeout) timeout = GetPrivateProfileIntA("QuickLoot", "ClickPeekTimeout", 0, IniPath);
    if (!timeout) timeout = GetPrivateProfileIntA("QuickLoot", "click_peek_timeout_ms", 0, IniPath);
    if (timeout > 0 && timeout <= 60) timeout *= 1000;
    g_clickTimeoutMs = timeout > 0 ? static_cast<uint32_t>(timeout) : 0;
    g_m.cfg.peekOnSelect = g_peekOnSelect;
    g_m.cfg.clickTimeoutMs = g_clickTimeoutMs;
    g_hoverPeek = GetPrivateProfileIntA("QuickLoot", "hover_peek", 1, IniPath) != 0;
    if (!g_hoverPeek) g_hoverPeek = GetPrivateProfileIntA("QuickLoot", "HoverPeek", 1, IniPath) != 0;
    g_m.cfg.hoverPeek = g_hoverPeek;
    g_emptyCursor = GetPrivateProfileIntA("QuickLoot", "empty_cursor", 1, IniPath) != 0;
    if (!g_emptyCursor) g_emptyCursor = GetPrivateProfileIntA("QuickLoot", "EmptyCursor", 1, IniPath) != 0;
    g_m.cfg.emptyCursor = g_emptyCursor;
    int dwell = GetPrivateProfileIntA("QuickLoot", "hover_dwell_ms", 150, IniPath);
    if (dwell <= 0) dwell = GetPrivateProfileIntA("QuickLoot", "HoverDwellMs", 150, IniPath);
    if (dwell > 0) g_m.cfg.hoverDwellMs = static_cast<uint32_t>(dwell);
    g_m.cfg.listRows = qlogic::clampListRows(GetPrivateProfileIntA("QuickLoot", "ListRows", static_cast<int>(qlogic::ListRowsDefault), IniPath));
    g_m.cfg.anchor = qlogic::clampAnchorMode(GetPrivateProfileIntA("QuickLoot", "Anchor", 0, IniPath));
    g_m.cfg.showTook = GetPrivateProfileIntA("QuickLoot", "ShowAcquired", 0, IniPath) != 0;   // 0.6.1-dev: off by default
    g_rules = GetPrivateProfileIntA("QuickLoot", "Rules", 1, IniPath) != 0;     // 0.7.0-dev: 0 = tooltip base rules only (kill switch for the engine property text)
    g_sounds = GetPrivateProfileIntA("QuickLoot", "Sounds", 1, IniPath) != 0;   // 0.7.0-dev QL12
    g_hideArrow = GetPrivateProfileIntA("QuickLoot", "HideTargetArrow", 1, IniPath) != 0;   // 0.7.2-dev QL14
    g_hardwareBezels = GetPrivateProfileIntA("QuickLoot", "HardwareBezels", 1, IniPath) != 0; // 0.8.0-dev QL14
    GetPrivateProfileStringA("QuickLoot", "GiveTags", "", g_giveTags, sizeof g_giveTags, IniPath); // QL6
    g_live.bezels = g_hardwareBezels;
    for (int c = 0; c < qlbezel::CatCount; ++c) {
        g_catStyles[c] = qlbezel::defaultStyle(static_cast<qlbezel::Category>(c));
        char key[32];
        snprintf(key, sizeof key, "Cat_%d_Frame", c);
        g_catStyles[c].frame = static_cast<uint8_t>(GetPrivateProfileIntA("QuickLoot", key, g_catStyles[c].frame, IniPath));
        snprintf(key, sizeof key, "Cat_%d_Palette", c);
        g_catStyles[c].palette = static_cast<uint8_t>(GetPrivateProfileIntA("QuickLoot", key, g_catStyles[c].palette, IniPath));
        snprintf(key, sizeof key, "Cat_%d_Led", c);
        g_catStyles[c].led = GetPrivateProfileIntA("QuickLoot", key, g_catStyles[c].led ? 1 : 0, IniPath) != 0;
        snprintf(key, sizeof key, "Cat_%d_RGB", c);
        char rgbBuf[32] = "";
        GetPrivateProfileStringA("QuickLoot", key, "", rgbBuf, sizeof rgbBuf, IniPath);
        if (rgbBuf[0]) {
            float r = 0, g = 0, b = 0;
            if (sscanf(rgbBuf, "%f,%f,%f", &r, &g, &b) == 3) {
                g_catStyles[c].frameRgb[0] = g_catStyles[c].btnRgb[0] = r;
                g_catStyles[c].frameRgb[1] = g_catStyles[c].btnRgb[1] = g;
                g_catStyles[c].frameRgb[2] = g_catStyles[c].btnRgb[2] = b;
            }
        }
        g_live.catStyles[c] = g_catStyles[c];
    }
    g_live.anchor = g_m.cfg.anchor; g_live.prox = g_proxOn; g_live.rows = g_m.cfg.listRows; g_live.openM = g_openM; g_live.closeM = g_closeM;
    g_live.closeExplicit = closeExplicit; g_live.closeExplicitM = closeExplicitM;
    g_live.tip = GetPrivateProfileIntA("QuickLoot", "TipDetail", qlpage::TipFull, IniPath);   // 0.7.1-dev page rows; applyLive below clamps nothing, so clamp here
    if (g_live.tip < qlpage::TipFull || g_live.tip > qlpage::TipOff) g_live.tip = qlpage::TipFull;
    g_live.sort = GetPrivateProfileIntA("QuickLoot", "SortFirst", 1, IniPath) != 0;
    g_live.equip = GetPrivateProfileIntA("QuickLoot", "EquippedLine", 1, IniPath) != 0;
    g_live.panel = GetPrivateProfileIntA("QuickLoot", "Panel", 1, IniPath) != 0 ? 1 : 0;
    g_tipDetail = g_live.tip; g_sortFirst = g_live.sort; g_equipLine = g_live.equip; g_panelDark = g_live.panel != 0;
    pageInit(api);
    pagePoll();   // 0.7.1-dev: register now: mod-config binds its entry list once, when the Options panel is first built (27.9 s in the 10-29 log); the HUD-frame poll registered at 31.6 s and the page never showed
    g_input.init(api, "quick-loot");
    g_input.connect();   // gui-input may load later: ensureWheel polls
    g_hud.init(api, "quick-loot");
    g_hud.add(K2HUD_FRAME, 100, onHudFrame);
    g_hud.add(K2HUD_BUILD, 300, onHudBuild);               // QL1b toast label
    g_hud.add(K2HUD_TARGET_MENU_POST, 300, onMenuPost);
    g_hud.connect();
    if (KpLog_HasCore()) KP_LOG(chMain, "INIT quick-loot %s enabled=%d hud=%d prox=%d openM=%.2f closeM=%.2f key=%d scanMs=%u rows=%u anchor=%d input=%d rules=%d sounds=%d", Version, g_enabled ? 1 : 0,
                                  g_hud.connected ? 1 : 0, g_proxOn ? 1 : 0, static_cast<double>(g_openM), static_cast<double>(g_closeM), g_takeKey, g_scanMs, g_m.cfg.listRows, g_m.cfg.anchor, g_input.connected ? 1 : 0, g_rules ? 1 : 0, g_sounds ? 1 : 0);
}

// H1. ECX = gui, [esp+4] = container object id (client), [esp+8] = flag. Returns 1 to consume (take-all armed), 0 = the original menu runs.
// 0.7.2-dev (QL14): the engine's target arrow (hostilearrow / friendlyarrow sprite, 0x0074FB20) drew over the list. While the list is up the draw is
// skipped (return 1 = consumed: epilogue 0x00750764). quick_loot.ini HideTargetArrow=0 (g_hideArrow) turns this off. Hot path (every frame): one bool and one compare.
extern "C" int __cdecl QlTargetArrow(uintptr_t owner) {
    (void)owner;
    if (!g_enabled || !g_hideArrow) return 0;
    const bool hide = g_m.labelText(nowMs()) != nullptr;
    static bool last = false;
    static unsigned logs = 0;
    if (hide != last && logs < 20) { ++logs; KP_LOG(chMain, "ARROW %s", hide ? "hidden (list up)" : "shown"); }
    last = hide;
    return hide ? 1 : 0;
}

extern "C" int __cdecl QlShowContainer(uintptr_t gui, uint32_t objId, int flag) {
    enter();
    uintptr_t plc = placeableFor(objId);
    uintptr_t repo = 0;
    int n = itemCount(plc, &repo);
    uint32_t now = nowMs();
    g_t.onShow(plc, now);
    char b[160];
    qlogic::fmtShow(b, sizeof b, objId, flag, plc, plc ? openFlag(plc) : -1, n);
    logLine(b);
    unsigned lines = qlogic::clampItems(n);
    uint32_t arr = 0;   // object-id array
    if (!repo || !rd<uint32_t>(repo, Repo_Array, &arr) || !arr) lines = 0;
    for (unsigned i = 0; i < lines; ++i) {
        uint32_t id = 0; int stack = -1;
        if (!rd<uint32_t>(arr, static_cast<int>(i * 4), &id)) break;
        uintptr_t item = reinterpret_cast<LookupFn>(ItemListGetItem)(repo, i);
        if (item) rd<int>(item, Item_Stack, &stack);
        qlogic::fmtItem(b, sizeof b, i, id, stack);
        probeLine(b);
    }
    return g_m.onShow(g_env, objId, plc, flag, n, now, isGiveContainer(gui, plc));   // QL2p peek (any flag) or QL1a take-all armed (1), else the vanilla menu (0)
}

// H2. ECX = placeable, [esp+4] = a1, [esp+8] = a2, [esp+0xC] = a3 (event record pointer). Runs on every placeable heartbeat: filter first.
extern "C" void __cdecl QlPlaceableEvent(uintptr_t plc, int a1, int a2, int a3) {
    uint32_t now = nowMs();
    if (g_m.wantsEvent(plc)) {   // QL1a / QL2p: note the container's OnOpen (the Close is sent from the HUD frame, never here)
        enter();
        uint16_t tid = 0;
        if (rd<uint16_t>(static_cast<uint32_t>(a3), 0, &tid)) {
            g_m.onEvent(plc, static_cast<short>(tid));
        }
    }
    if (!g_t.matches(plc, now)) return;
    enter();
    int ref = 0;
    rd<int>(EventClassGlobal, 0, &ref);
    uint16_t id16 = 0;
    bool haveId = rd<uint16_t>(static_cast<uint32_t>(a3), 0, &id16);
    char b[160];
    if (g_t.wantRaw()) {
        qlogic::fmtEvtRaw(b, sizeof b, static_cast<unsigned>(plc), a1, a2, static_cast<unsigned>(a3), haveId ? id16 : -1, ref);
        probeLine(b);
    }
    if (haveId && qlogic::isWatchedEvent(static_cast<short>(id16))) {
        qlogic::fmtEvt(b, sizeof b, static_cast<short>(id16), qlogic::msSince(now, g_t.showMs), itemCount(plc));
        probeLine(b);
    }
}

// H3. ECX = placeable, [esp+4] = player object id, [esp+8] = flag (1 = take all).
extern "C" void __cdecl QlCloseInventory(uintptr_t plc, uint32_t player, int flag) {
    (void)player;
    uint32_t now = nowMs();
    if (!g_t.matches(plc, now)) return;
    enter();
    g_t.onClose(plc, now);
    char b[160];
    qlogic::fmtClose(b, sizeof b, flag, qlogic::msSince(now, g_t.showMs), itemCount(plc), openFlag(plc));
    logLine(b);
}
