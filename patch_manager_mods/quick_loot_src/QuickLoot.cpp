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
//   Enabled=1, ProximityEnabled=1, OpenM=3.0, CloseM=5.0 (>= OpenM+1), TakeKey=70 (VK, 'F'), ScanMs=250, ListRows=8 (1..12).
// Below: the QL0 probe description (all its log lines are kept). Questions it answered (patch_manager_mods/52_quick_loot.md):
//   1. does loot that OnOpen generates (queued event 0x16) reach the container before the player's Close?   (SHOW n vs CLOSE n, EVT 0x16 dt)
//   2. timing: ms between Show, event 0x16 and Close.                                                        (dt= on EVT / CLOSE lines)
//   3. does take-all (Close flag 1) fire OnInvDisturbed (event 0x1B)?                                         (EVT id=1b after CLOSE flag=1)
// Hooks read ECX / [ESP+4..] (KPM passes the game's stack: [esp+0] = return address). Log lines: QuickLootLogic.h fmt*.
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
#include "SafeRead.h"
#include "GameAddr.h"
#include "ActQueue.h"          // actq::playerPaused
#include "CombatTraceRead.h"   // ctrace::readHudCombat
#include <stdlib.h>

KP_LOG_PATCH("quick-loot");

namespace {

constexpr const char* Version = "0.5.0-dev";
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

// Localised item name: CExoLocString at item+0x2C0 through GetString, copy of inventory-sort readItemName. Leaves out[0] = 0 (unknown) when the
// handle / language is unusable, GetString fails or the text is unreadable. The engine out string is always freed with the CExoString dtor.
void readItemName(uintptr_t item, int lang, char* out, int cap) {
    out[0] = 0;
    const uintptr_t handle = item + Item_Name;
    uint32_t first = 0, probe = 0;
    // GetString dereferences [handle+0] with no null check: require a readable pointer there (server items are unverified for this offset).
    if (!item || lang < 0 || !rd<uint32_t>(handle, 0, &first) || !first || !rd<uint32_t>(first, 0, &probe)) return;
    int exo[2] = {0, 0};   // CExoString {char* p; uint32 len}
    const bool ok = reinterpret_cast<GetLocStringFn>(gameaddr::GetLocString)(reinterpret_cast<void*>(handle), lang, exo, 0);
    if (ok && exo[0]) {
        char tmp[256];
        if (saferead::readExo(Site, reinterpret_cast<uintptr_t>(exo), tmp, sizeof tmp, 255, false)) {
            int k = 0;
            while (tmp[k] && k < cap - 1) { out[k] = tmp[k]; ++k; }
            out[k] = 0;
        }
    }
    if (exo[0]) reinterpret_cast<ExoDtorFn>(gameaddr::ExoDtor)(exo);
}

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
    for (unsigned i = 0; i < lines; ++i) {
        uint32_t id = 0; int stack = -1;
        if (!rd<uint32_t>(arr, static_cast<int>(i * 4), &id)) break;
        const uintptr_t item = reinterpret_cast<LookupFn>(ItemListGetItem)(repo, i);
        c.names[i][0] = 0;
        c.icons[i][0] = 0;
        if (item) { rd<int>(item, Item_Stack, &stack); readItemName(item, lang, c.names[i], NameCap); readItemIcon(item, c.icons[i]); }
        c.ids[i] = id; c.stacks[i] = stack;
        ++c.count;
    }
}

// ---- labels. Draw order = creation order (BUILD makes them in this order): frame panel (back), header, scroll indicator, then per row (12 max) the
// bordered row frame, the icon, the text, then the scrollbar parts (track outline, track inset, thumb, up, down). Every draw is change-only per label.
struct Lbl {
    uintptr_t c = 0;                  // the control (0 = none: creation failed or the HUD changed)
    int vis = -1;                     // -1 unknown, 0 hidden, 1 shown
    bool styled = false, rectSet = false, colSet = false;
    unsigned align = 0;
    qlogic::Rect rect = {0, 0, 0, 0};
    float col[3] = {0, 0, 0};
    char text[qlogic::RowTextCap + 8] = {0};
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
Lbl g_sbOuter, g_sbInner, g_sbThumb, g_sbUp, g_sbDown;
bool g_rowsOk = false;        // header + indicator + every row / scrollbar label exists; false = one-line text in the header label (0.3.1 behaviour)
bool g_rowFlat = false;       // the row frames are flat fills (no frame template in the HUD layout)
bool g_backFrame = false;     // the backing is a vanilla frame template (fill + edges + corners from the .gui); false = flat tinted fill
bool g_backStyled = false;    // backing tint / alpha (/ fill) applied
bool g_backOff = false;       // backing could not be styled: text only
bool g_backRectSet = false;
qlogic::Rect g_backLast = {0, 0, 0, 0};
int32_t g_panel = 0;          // HUD object the labels belong to (K2HudContext.panel at BUILD == ctx->hud later)
bool g_haveRect = false;      // a target-menu name rect has been seen
qlogic::Rect g_nameRect = {0, 0, 0, 0};
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
guikit::ClickEdge g_clickEdge;                // LMB edge state (guikit::clickEdge), polled every frame by sbClickPoll

void listPublishHide() { g_listRect[2] = 0; g_listScroll = 0; g_hover = false; g_hoverRow = -1; g_sbPub.shown = false; }

void lblHideAll() {
    lblHideOne(g_back);
    lblHideOne(g_head);
    lblHideOne(g_ind);
    for (unsigned i = 0; i < qlogic::ListRowsMax; ++i) { lblHideOne(g_rowFrame[i]); lblHideOne(g_rowIcon[i]); lblHideOne(g_row[i]); }
    lblHideOne(g_sbOuter); lblHideOne(g_sbInner); lblHideOne(g_sbThumb); lblHideOne(g_sbUp); lblHideOne(g_sbDown);
    g_backRectSet = false;
    listPublishHide();
}

// The HUD was rebuilt: the old labels may be freed. Drop every pointer without writing to them.
void lblDrop() {
    g_back = Lbl(); g_head = Lbl(); g_ind = Lbl();
    for (Lbl& r : g_row) r = Lbl();
    for (Lbl& r : g_rowFrame) r = Lbl();
    for (Lbl& r : g_rowIcon) r = Lbl();
    g_sbOuter = Lbl(); g_sbInner = Lbl(); g_sbThumb = Lbl(); g_sbUp = Lbl(); g_sbDown = Lbl();
    g_rowFlat = false;
    g_rowsOk = false; g_backFrame = false; g_backStyled = false; g_backOff = false; g_backRectSet = false;
    g_panel = 0; g_haveRect = false;
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
    const uintptr_t panel = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
    const char* frameTag = "LBL_QUEUE1";
    for (const char* tag : FrameTemplates) {
        g_back.c = makeLabel(panel, tag, "frame", false);
        if (g_back.c) { g_backFrame = true; frameTag = tag; break; }
    }
    if (!g_back.c) g_back.c = makeLabel(panel, "LBL_QUEUE1", "backing");   // flat fill; losing the backing only costs the dark panel
    g_head.c = makeLabel(panel, "LBL_QUEUE1", "header");
    if (!g_head.c) return;
    g_panel = ctx->panel;
    g_ind.c = makeLabel(panel, "LBL_QUEUE1", "indicator");
    bool rowsOk = g_ind.c != 0;
    g_rowFlat = !g_backFrame;   // the row frames come from the same template as the backing; flat fills when the layout has none
    for (unsigned i = 0; i < qlogic::ListRowsMax && rowsOk; ++i) {
        g_rowFrame[i].c = makeLabel(panel, frameTag, "row frame");
        g_rowIcon[i].c = g_rowFrame[i].c ? makeLabel(panel, "LBL_QUEUE1", "row icon") : 0;
        g_row[i].c = g_rowIcon[i].c ? makeLabel(panel, "LBL_QUEUE1", "row") : 0;
        if (!g_row[i].c) rowsOk = false;
    }
    Lbl* sbParts[5] = { &g_sbOuter, &g_sbInner, &g_sbThumb, &g_sbUp, &g_sbDown };
    for (Lbl* l : sbParts) {
        if (!rowsOk) break;
        l->c = makeLabel(panel, "LBL_QUEUE1", "scrollbar");
        if (!l->c) rowsOk = false;
    }
    g_rowsOk = rowsOk;
    g_back.vis = g_back.c ? 0 : -1; g_head.vis = 0; g_ind.vis = g_ind.c ? 0 : -1;
    for (unsigned i = 0; i < qlogic::ListRowsMax; ++i) {
        g_row[i].vis = g_row[i].c ? 0 : -1; g_rowFrame[i].vis = g_rowFrame[i].c ? 0 : -1; g_rowIcon[i].vis = g_rowIcon[i].c ? 0 : -1;
    }
    for (Lbl* l : sbParts) l->vis = l->c ? 0 : -1;
    if (KpLog_HasCore()) KP_LOG(chMain, "BUILD frame=%s%s rows=%s", frameTag, g_backFrame ? "" : " (flat)", g_rowsOk ? "pool" : "fallback-oneline");
}

// Style (font once, align / colour on change), then text / rect / visibility change-only. false = a write failed (caller hides everything).
bool lblDrawText(Lbl& l, const char* text, const qlogic::Rect& r, unsigned align, const float* col) {
    if (!l.c) return false;
    if (!l.styled) {
        if (!guikit::setFont(l.c, FontName)) return false;
        l.styled = true;
    }
    if (l.align != align) { if (!guikit::setAlign(l.c, align)) return false; l.align = align; }
    if (!l.colSet || memcmp(l.col, col, sizeof l.col) != 0) {
        if (!guikit::setTextColor(l.c, col[0], col[1], col[2])) return false;
        memcpy(l.col, col, sizeof l.col);
        l.colSet = true;
    }
    if (strcmp(l.text, text) != 0) {
        if (!guikit::setText(l.c, text)) return false;
        strncpy(l.text, text, sizeof l.text - 1);
        l.text[sizeof l.text - 1] = 0;
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
bool lblDrawFrame(Lbl& l, const qlogic::Rect& r, const float* tint) {
    if (!l.c) return false;
    if (g_rowFlat && !l.fillSet) {
        void* img = guikit::imageFor(TexFill2bt);
        if (!img || !guikit::setFill(l.c, img) || !guikit::setFillStyle(l.c, 2)) return false;
        l.fillSet = true;
    }
    if (!lblTint(l, tint) || !lblAlpha(l, qlogic::FrameAlpha)) return false;
    lblPlace(l, r);
    lblShow(l, false);
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

// The backing panel under the text. Frame template: keeps the .gui's fill / edge / corner images, sets the tint (TealFrame) and FrameAlpha.
// Flat fallback (no frame template in the HUD layout): the pill image tinted black at BackAlpha, the 0.3.1 look.
void backDraw(const qlogic::Rect& r) {
    if (!g_back.c || g_backOff) return;
    if (!g_backStyled) {
        bool ok;
        if (g_backFrame) {
            ok = guikit::setBorderTint(g_back.c, TealFrame) && guikit::setAlpha(g_back.c, qlogic::FrameAlpha);
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
bool sbDraw(const qlogic::ListDraw& d, const qlogic::Rect& blk) {
    if (!d.sb) {
        lblHideOne(g_sbOuter); lblHideOne(g_sbInner); lblHideOne(g_sbThumb); lblHideOne(g_sbUp); lblHideOne(g_sbDown);
        g_sbPub.shown = false;
        return true;
    }
    const qlogic::Sb sb = qlogic::sbLayout(blk.w, d.vis, d.rowsTotal, d.off, d.rowH);
    const unsigned maxOff = d.rowsTotal > d.vis ? d.rowsTotal - d.vis : 0u;
    const qlogic::Rect col = atBlock(sb.col, blk);
    const qlogic::Rect up = inset(atBlock(sb.up, blk), 4), dn = inset(atBlock(sb.down, blk), 4);
    if (!lblDrawFilled(g_sbOuter, TexFill16g, SbOutlineTint, 1.0f, col, true) ||            // the one click-eating label: arrows, track and thumb all sit on it
        !lblDrawFilled(g_sbInner, TexFill2bt, SbInsetTint, 1.0f, inset(col, qlogic::SbInset), false) ||
        !lblDrawFilled(g_sbThumb, TexFill16g, SbThumbTint, 1.0f, atBlock(sb.thumb, blk), false) ||
        !lblDrawFilled(g_sbUp, TexUpArrow, White, d.off == 0 ? AlphaDim : 1.0f, up, false) ||
        !lblDrawFilled(g_sbDown, TexDownArrow, ArrowTint, d.off >= maxOff ? AlphaDim : 1.0f, dn, false))
        return false;
    g_sbPub.shown = true; g_sbPub.sb = sb; g_sbPub.blk = blk; g_sbPub.tick = GetTickCount();
    return true;
}

// The multi-row block at blk: frame, header, indicator (dim teal, right end of the header row), d.vis bordered rows (frame, icon, text; the row under
// the cursor gets the HILIGHT tint and cream text), the scrollbar when the list overflows.
void blockDraw(const qlogic::ListDraw& d, const qlogic::Rect& blk) {
    float cx = 0, cy = 0;
    const bool haveCur = g_input.connected && g_input.cursor_engine(&cx, &cy);
    g_hover = haveCur && qlogic::pointInRect(blk, cx, cy);
    g_hoverRow = g_hover ? qlogic::rowAtPoint(blk, d.vis, cx, cy, d.sb, d.rowH) : -1;
    backDraw(blk);
    const qlogic::Rect hr = qlogic::headerRect(blk.w);
    if (!lblDrawText(g_head, d.header, { blk.x + hr.x, blk.y + hr.y, hr.w, hr.h }, 9, Cream)) { lblHideAll(); return; }
    if (d.indicator[0]) {
        const qlogic::Rect ir = qlogic::indicatorRect(blk.w, d.indicatorPx);
        if (!lblDrawText(g_ind, d.indicator, { blk.x + ir.x, blk.y + ir.y, ir.w, ir.h }, 9, TealDim)) { lblHideAll(); return; }
    } else {
        lblHideOne(g_ind);
    }
    for (unsigned i = 0; i < qlogic::ListRowsMax; ++i) {
        if (i >= d.vis) { lblHideOne(g_rowFrame[i]); lblHideOne(g_rowIcon[i]); lblHideOne(g_row[i]); continue; }
        const bool hov = static_cast<int>(i) == g_hoverRow;
        const qlogic::Rect rr = atBlock(qlogic::rowRect(i, blk.w, d.sb, d.rowH), blk);
        if (!lblDrawFrame(g_rowFrame[i], rr, hov ? RowHilight : TealFrame) || !lblDrawIcon(g_rowIcon[i], d.icons[i], qlogic::rowIconRect(rr, d.rowH)) ||
            !lblDrawText(g_row[i], d.rows[i], qlogic::rowTextRect(rr, d.rowH), 9, hov ? Cream : TealText)) { lblHideAll(); return; }
    }
    if (!sbDraw(d, blk)) { lblHideAll(); return; }
    g_listRect[0] = blk.x; g_listRect[1] = blk.y; g_listRect[2] = blk.w; g_listRect[3] = blk.h;
    g_listScroll = d.scrollable ? 1 : 0;
    g_listTick = GetTickCount();
    logExtent(g_head, g_extentHead, d.header);
    logExtent(g_row[0], g_extentRow, d.rows[0]);
}

// One-line fallback (row labels unavailable): the old single text line in the header label on the same frame, centred on the latched label rect.
void lineDraw(const char* text, const qlogic::Rect& r) {
    const int w = qlogic::textPx10(text) + 2 * qlogic::BlockPadX;
    const qlogic::Rect blk = { r.x + r.w / 2 - w / 2, r.y - qlogic::BlockPadY, w, qlogic::HeaderH + 2 * qlogic::BlockPadY };
    backDraw(blk);
    if (!lblDrawText(g_head, text, { blk.x + qlogic::BlockPadX, blk.y + qlogic::BlockPadY, w - 2 * qlogic::BlockPadX + qlogic::LabelSlackPx, qlogic::HeaderH }, 9, TealText)) { lblHideAll(); return; }
    logExtent(g_head, g_extentHead, text);
}

// Left click on the scrollbar of the block drawn last: the SbPart (as int) the cursor is over at the press edge, else 0. Polled every FRAME tick so the
// edge state tracks the button even while no list is up. Same method as action-hotbar's slot clicks: guikit::clickEdge (LMB down AND the game window
// is this process's foreground window) + the cursor in engine px (gui-input) against the drawn rects; the scrollbar column label is the only one that
// eats the click, so it never reaches the world. A press that started elsewhere never fires (edge only).
int sbClickPoll() {
    const bool edge = guikit::clickEdge(g_clickEdge);
    if (!edge || !g_sbPub.shown || GetTickCount() - g_sbPub.tick > qlogic::WheelFreshMs) return 0;
    float cx = 0, cy = 0;
    if (!g_input.connected || !g_input.cursor_engine(&cx, &cy)) return 0;
    return static_cast<int>(qlogic::sbHitAt(g_sbPub.sb, cx - static_cast<float>(g_sbPub.blk.x), cy - static_cast<float>(g_sbPub.blk.y)));
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

qlogic::Sup currentSuppress(const K2HudContext* ctx, bool worldOk) {
    int clip = 0;
    const uintptr_t clipAddr = k2addr::addr(&K2AddrTable::ClipDepth);   // clip stack index: 0 in the HUD frame, 0xFFFF in menu frames (u03 (c))
    if (clipAddr) rd<int>(clipAddr, 0, &clip);
    uint8_t dlg = 0;
    rd<uint8_t>(DialogByte, 0, &dlg);
    const bool paused = actq::playerPaused(SitePause);
    const int combat = ctrace::readHudCombat(SiteWorld + 1, ctx ? static_cast<uintptr_t>(static_cast<uint32_t>(ctx->hud)) : 0);
    return qlogic::suppressReason(worldOk, (clip & 0xffff) != 0, dlg != 0, paused, combat);
}

bool takeKeyDown() {
    if (!(GetAsyncKeyState(g_takeKey) & 0x8000)) return false;
    HWND fg = GetForegroundWindow();
    DWORD pid = 0;
    if (!fg) return false;
    GetWindowThreadProcessId(fg, &pid);
    return pid == GetCurrentProcessId();   // a press in another window never counts
}

bool readCandFlags(uintptr_t plc, qlogic::CandFlags& f) {
    uint8_t locked = 0, open = 0;
    uint32_t inv = 0, trap = 0, use = 0;
    if (!rd<uint32_t>(plc, Plc_HasInv, &inv) || !rd<uint8_t>(plc, Plc_Locked, &locked) || !rd<uint32_t>(plc, Plc_Trap, &trap) ||
        !rd<uint32_t>(plc, Plc_Useable, &use) || !rd<uint8_t>(plc, Plc_Open, &open))
        return false;
    f.hasInv = inv != 0; f.locked = locked != 0; f.trap = trap != 0; f.useable = use != 0; f.open = open != 0;
    return true;
}

// The engine side of qlogic::Env (QuickLootMachine.h): every read goes through SafeRead, every action is the same engine call as before.
struct EngineEnv : qlogic::Env {
    const K2HudContext* ctx = nullptr;
    uintptr_t srv = 0;   // server app, cached by areaList for the scan that follows
    void enter() override { ::enter(); }
    bool readWorld(qlogic::World& w) override { return ::readWorld(w); }
    qlogic::Sup suppress(bool worldOk) override { return currentSuppress(ctx, worldOk); }
    bool keyDown() override { return takeKeyDown(); }
    bool hudOk() override { g_hud.poll(); return g_hud.connected; }
    bool shiftDown() override { return (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }
    uintptr_t resolvePlc(uint32_t sid) override {
        const uintptr_t s = serverApp();
        return s ? reinterpret_cast<LookupFn>(GetPlaceableByGameObjectID)(s, sid) : 0;
    }
    uintptr_t resolveClientPlc(uint32_t clientObj) override { return placeableFor(clientObj); }
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
    void log(const char* line) override { logLine(line); }
    int wheelDelta() override { return static_cast<int>(InterlockedExchange(&g_wheelAccum, 0)); }
    bool overList() override { return g_hover; }
    int sbClick() override { return sbClickPoll(); }
};
EngineEnv g_env;


// K2HUD_TARGET_MENU_POST: runs before every drawn HUD frame (also with no target). Caches the name rect while the target menu is shown, then draws
// what the machine says: the Took line (while up) or the peek line. Position A (under the name, clamped to the panel) or B (fixed spot) is
// chosen when the label first shows and latched until it is gone (QuickLootMachine.h placeLabel).
void KPATCH_CALL onMenuPost(const K2HudContext* ctx, void*) {
    if (!g_head.c || !ctx) return;
    if (ctx->hud != g_panel) { lblDrop(); return; }   // stale HUD: no writes to the old labels
    saferead::beginScope();
    const uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    uint8_t flags = 0;
    qlogic::Geometry g;
    if (menu && rd<uint8_t>(menu, Menu_Flags, &flags) && (flags & 1)) {
        int32_t mx = 0, my = 0, nx = 0, ny = 0, nw = 0, nh = 0;
        if (rd<int32_t>(menu, Menu_OriginX, &mx) && rd<int32_t>(menu, Menu_OriginY, &my) && rd<int32_t>(menu, Menu_NameX, &nx) &&
            rd<int32_t>(menu, Menu_NameY, &ny) && rd<int32_t>(menu, Menu_NameW, &nw) && rd<int32_t>(menu, Menu_NameH, &nh)) {
            g_nameRect = { mx + nx, my + ny, nw, nh };   // screen rect of the name label
            g_haveRect = true;
            g.menuShown = true;
        }
    }
    const uint32_t now = nowMs();
    const char* text = g_m.labelText(now);
    if (!text) { lblHideAll(); return; }
    g.haveName = g_haveRect;
    g.name = g_nameRect;
    int px = 0, py = 0, pw = 0, ph = 0;
    if (guikit::getExtent(static_cast<uintptr_t>(static_cast<uint32_t>(g_panel)), &px, &py, &pw, &ph) && pw > 0 && ph > 0) {
        g.havePanel = true;
        g.panel = { px, py, pw, ph };
    }
    qlogic::ListDraw d;
    if (g_rowsOk && g_m.listDraw(now, &d)) {   // the multi-row block, positioned from the latched A / B anchor
        qlogic::Rect blk = { 0, 0, 0, 0 };
        if (!g_m.placeBlock(g_env, g, d.w, d.h, &blk)) { lblHideAll(); return; }
        blockDraw(d, blk);
        return;
    }
    qlogic::Rect r = { 0, 0, 0, 0 };   // fallback: the one-line text (row labels missing)
    if (!g_m.placeLabel(g_env, g, &r)) { lblHideAll(); return; }
    lineDraw(text, r);
}

// K2HUD_FRAME: the pending click-take Close, then the proximity peek. Runs on the game thread, outside any EventHandler.
void KPATCH_CALL onHudFrame(const K2HudContext* ctx, void*) {
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
    g_proxOn = GetPrivateProfileIntA("QuickLoot", "ProximityEnabled", 1, IniPath) != 0;
    g_takeKey = qlogic::clampTakeKey(GetPrivateProfileIntA("QuickLoot", "TakeKey", qlogic::DefaultTakeKey, IniPath));
    g_scanMs = qlogic::clampScanMs(GetPrivateProfileIntA("QuickLoot", "ScanMs", static_cast<int>(qlogic::DefaultScanMs), IniPath));
    char fb[32];
    GetPrivateProfileStringA("QuickLoot", "OpenM", "3.0", fb, sizeof fb, IniPath);
    g_openM = qlogic::clampOpenM(static_cast<float>(atof(fb)));
    GetPrivateProfileStringA("QuickLoot", "CloseM", "5.0", fb, sizeof fb, IniPath);
    g_closeM = qlogic::clampCloseM(g_openM, static_cast<float>(atof(fb)));
    g_m.cfg.enabled = g_enabled; g_m.cfg.proxOn = g_proxOn; g_m.cfg.openM = g_openM; g_m.cfg.closeM = g_closeM; g_m.cfg.takeKey = g_takeKey; g_m.cfg.scanMs = g_scanMs;
    g_m.cfg.listRows = qlogic::clampListRows(GetPrivateProfileIntA("QuickLoot", "ListRows", static_cast<int>(qlogic::ListRowsDefault), IniPath));
    g_input.init(api, "quick-loot");
    g_input.connect();   // gui-input may load later: ensureWheel polls
    g_hud.init(api, "quick-loot");
    g_hud.add(K2HUD_FRAME, 100, onHudFrame);
    g_hud.add(K2HUD_BUILD, 300, onHudBuild);               // QL1b toast label
    g_hud.add(K2HUD_TARGET_MENU_POST, 300, onMenuPost);
    g_hud.connect();
    if (KpLog_HasCore()) KP_LOG(chMain, "INIT quick-loot %s enabled=%d hud=%d prox=%d openM=%.2f closeM=%.2f key=%d scanMs=%u rows=%u input=%d", Version, g_enabled ? 1 : 0,
                                  g_hud.connected ? 1 : 0, g_proxOn ? 1 : 0, static_cast<double>(g_openM), static_cast<double>(g_closeM), g_takeKey, g_scanMs, g_m.cfg.listRows, g_input.connected ? 1 : 0);
}

// H1. ECX = gui, [esp+4] = container object id (client), [esp+8] = flag. Returns 1 to consume (take-all armed), 0 = the original menu runs.
extern "C" int __cdecl QlShowContainer(uintptr_t gui, uint32_t objId, int flag) {
    (void)gui;
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
        logLine(b);
    }
    return g_m.onShow(g_env, objId, plc, flag, n, now);   // QL2p peek (any flag) or QL1a take-all armed (1), else the vanilla menu (0)
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
        logLine(b);
    }
    if (haveId && qlogic::isWatchedEvent(static_cast<short>(id16))) {
        qlogic::fmtEvt(b, sizeof b, static_cast<short>(id16), qlogic::msSince(now, g_t.showMs), itemCount(plc));
        logLine(b);
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
