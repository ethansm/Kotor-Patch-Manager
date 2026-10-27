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
//   Enabled=1, ProximityEnabled=1, OpenM=3.0, CloseM=5.0 (>= OpenM+1), TakeKey=70 (VK, 'F'), ScanMs=250.
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
#include "QuickLootLogic.h"
#include "SafeRead.h"
#include "GameAddr.h"
#include "ActQueue.h"          // actq::playerPaused
#include "CombatTraceRead.h"   // ctrace::readHudCombat
#include <stdlib.h>

KP_LOG_PATCH("quick-loot");

namespace {

constexpr const char* Version = "0.3.0-dev";
constexpr unsigned Site = 0x7100;   // saferead failure-ring site id
// Steam addresses (static RE, doc 52 / .claude/research/quick_loot/README.md); AppGlobal and ClientToServerObjectId come from GameAddr.h.
constexpr uintptr_t GetPlaceableByGameObjectID = 0x0051C160;   // thiscall(serverApp, serverId) RET 4 -> CSWSPlaceable* (0 = none)
constexpr uintptr_t ItemListGetItem = 0x00606100;              // thiscall(repo, index) RET 4 -> CSWSItem* (0 = none)
constexpr uintptr_t EventClassGlobal = 0x0099244C;             // DAT_0099244c: the EventHandler's first argument is compared with it
constexpr int Plc_Open = 0x390, Plc_Repo = 0x3C4;              // open flag (byte), item repository pointer
constexpr int Repo_Array = 0x0C, Repo_Count = 0x10;            // object-id array, count
constexpr int Item_Stack = 0x2CC;                              // CSWSItem stack size (int)
constexpr int Item_Name = 0x2C0;                               // QL1b: CExoLocString (inventory-sort reads it on client items; UNVERIFIED on server CSWSItem)
// QL1b: target menu (menu = ctx->menu = hud + 0xCC): flags bit0 = shown, origin px, name label rect (panel-local px). Same offsets as enemy-effects-strip.
constexpr int Menu_Flags = 0x1BAC, Menu_OriginX = 0x1648, Menu_OriginY = 0x164C;
constexpr int Menu_NameX = 0x166C, Menu_NameY = 0x1670, Menu_NameW = 0x1674, Menu_NameH = 0x1678;
constexpr int NameCap = 48;                                    // captured name buffer (sanitizeName cuts at ToastNameMax = 40)
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
qlogic::Take g_take;
qlogic::Peek g_peek;
qlogic::Cooldown g_cool;
qlogic::SupTracker g_sup;
bool g_proxOn = true;
float g_openM = qlogic::DefaultOpenM, g_closeM = qlogic::DefaultCloseM;
int g_takeKey = qlogic::DefaultTakeKey;
uint32_t g_scanMs = qlogic::DefaultScanMs;
uint32_t g_lastScan = 0;
bool g_keyPrev = false;
uintptr_t g_lastArea = 0;
uint32_t g_nearId = 0;   // SCAN change detector: nearest in-range placeable id (0 = none) and its ok flag
int g_nearOk = -1;
k2hud::Client g_hud;
bool g_enabled = true;
uint32_t g_frame = 0;   // last HUD frame counter seen by onHudFrame
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

struct Captured {
    unsigned count = 0;   // names captured (<= ToastMaxItems)
    unsigned total = 0;   // real repository count
    uint32_t ids[qlogic::ToastMaxItems];
    int stacks[qlogic::ToastMaxItems];
    char names[qlogic::ToastMaxItems][NameCap];
};

// Reads the container's item names / stacks. SafeRead-guarded; an unreadable part just leaves fewer (or "item") entries.
void captureItems(uintptr_t plc, Captured& c) {
    c.count = 0; c.total = 0;
    uintptr_t repo = 0;
    const int n = itemCount(plc, &repo);
    unsigned lines = qlogic::clampItems(n);
    uint32_t arr = 0;
    if (!lines || !repo || !rd<uint32_t>(repo, Repo_Array, &arr) || !arr) return;
    c.total = static_cast<unsigned>(n);
    if (lines > qlogic::ToastMaxItems) lines = qlogic::ToastMaxItems;
    const int lang = clientLanguage();
    for (unsigned i = 0; i < lines; ++i) {
        uint32_t id = 0; int stack = -1;
        if (!rd<uint32_t>(arr, static_cast<int>(i * 4), &id)) break;
        const uintptr_t item = reinterpret_cast<LookupFn>(ItemListGetItem)(repo, i);
        c.names[i][0] = 0;
        if (item) { rd<int>(item, Item_Stack, &stack); readItemName(item, lang, c.names[i], NameCap); }
        c.ids[i] = id; c.stacks[i] = stack;
        ++c.count;
    }
}

struct Toast {
    bool active = false;
    uint32_t shownAt = 0;
    char text[96] = {0};
};
Toast g_toast;
char g_shownText[128] = {0};   // text currently set on the label (setText only when it differs)
uintptr_t g_lbl = 0;          // the toast label (0 = none: creation failed or the HUD changed)
int32_t g_panel = 0;          // HUD object the label belongs to (K2HudContext.panel at BUILD == ctx->hud later)
int g_vis = -1;               // change-only caches: -1 unknown, 0 hidden, 1 shown
bool g_styled = false;        // font / align / colour applied
bool g_rectSet = false;
qlogic::Rect g_lastRect = {0, 0, 0, 0};
bool g_haveRect = false;      // a target-menu name rect has been seen
qlogic::Rect g_nameRect = {0, 0, 0, 0};
bool g_buildFailLogged = false;

void lblHide() {
    if (!g_lbl || g_vis == 0) return;
    guikit::setVisible(g_lbl, false);
    guikit::setClickable(g_lbl, false);
    g_vis = 0;
}

// The HUD was rebuilt: the old label may be freed. Drop every pointer without writing to it.
void lblDrop() {
    g_lbl = 0; g_panel = 0; g_vis = -1; g_styled = false; g_rectSet = false; g_haveRect = false;
    g_shownText[0] = 0;
    g_toast.active = false;
}

// K2HUD_BUILD: create the toast label (hidden, click-through) in the HUD panel; draw order = creation order.
void KPATCH_CALL onHudBuild(const K2HudContext* ctx, void*) {
    lblDrop();
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, panel) + sizeof(ctx->panel) || !ctx->panel) return;
    enter();
    const uintptr_t panel = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
    int why = guikit::CF_None;
    uint32_t bv = 0;
    const uintptr_t c = guikit::createLabelFromTemplate(panel, "LBL_QUEUE1", &why, &bv);
    if (!c) {
        if (!g_buildFailLogged) { g_buildFailLogged = true; KP_LOGV(chMain, KPLOG_WARN, "BUILD: toast label create failed (why=%d vtable=%08x): no toast", why, (unsigned)bv); }
        return;
    }
    guikit::setVisible(c, false);
    guikit::setClickable(c, false);
    if (!guikit::appendToPanel(panel, c)) {
        if (!g_buildFailLogged) { g_buildFailLogged = true; KP_LOGV(chMain, KPLOG_WARN, "BUILD: toast label append failed (control %08x left hidden): no toast", (unsigned)c); }
        return;
    }
    g_lbl = c; g_panel = ctx->panel; g_vis = 0;
}

// Shared label draw (toast + peek): style once; text / rect / visibility change-only.
void labelDraw(const char* text, const qlogic::Rect& r) {
    if (!g_styled) {
        if (!(guikit::setFont(g_lbl, "dialogfont10x10") && guikit::setAlign(g_lbl, 18) && guikit::setTextColor(g_lbl, 0.9f, 0.85f, 0.5f))) { lblHide(); return; }
        g_styled = true;
    }
    if (strcmp(g_shownText, text) != 0) {
        if (!guikit::setText(g_lbl, text)) { lblHide(); return; }
        strncpy(g_shownText, text, sizeof g_shownText - 1);
        g_shownText[sizeof g_shownText - 1] = 0;
    }
    if (!g_rectSet || r != g_lastRect) { guikit::setRect(g_lbl, r.x, r.y, r.w, r.h); g_lastRect = r; g_rectSet = true; }
    if (g_vis != 1) { guikit::setVisible(g_lbl, true); guikit::setClickable(g_lbl, false); g_vis = 1; }   // shown labels stay click-through
}

// K2HUD_TARGET_MENU_POST: runs before every drawn HUD frame (also with no target). Caches the name rect while the target menu is shown.
// The peek label (while a peek is active) wins over the toast: name-anchored while the target menu is shown, else a fixed spot on the HUD panel.
void KPATCH_CALL onMenuPost(const K2HudContext* ctx, void*) {
    if (!g_lbl || !ctx) return;
    if (ctx->hud != g_panel) { lblDrop(); return; }   // stale HUD: no writes to the old label
    saferead::beginScope();
    const uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    uint8_t flags = 0;
    bool menuShown = false;
    if (menu && rd<uint8_t>(menu, Menu_Flags, &flags) && (flags & 1)) {
        int32_t mx = 0, my = 0, nx = 0, ny = 0, nw = 0, nh = 0;
        if (rd<int32_t>(menu, Menu_OriginX, &mx) && rd<int32_t>(menu, Menu_OriginY, &my) && rd<int32_t>(menu, Menu_NameX, &nx) &&
            rd<int32_t>(menu, Menu_NameY, &ny) && rd<int32_t>(menu, Menu_NameW, &nw) && rd<int32_t>(menu, Menu_NameH, &nh)) {
            g_nameRect = { mx + nx, my + ny, nw, nh };   // screen rect of the name label
            g_haveRect = true;
            menuShown = true;
        }
    }
    if (g_peek.active && g_peek.text[0]) {   // QL2p peek label
        qlogic::Rect r = { 0, 0, 0, 0 };
        int px = 0, py = 0, pw = 0, ph = 0;
        if (menuShown) r = qlogic::peekRectName(g_nameRect.x, g_nameRect.y, g_nameRect.w, g_nameRect.h);
        else if (guikit::getExtent(static_cast<uintptr_t>(static_cast<uint32_t>(g_panel)), &px, &py, &pw, &ph) && pw > 0 && ph > 0) r = qlogic::peekRectFixed(px, py, pw, ph);
        else if (g_haveRect) r = qlogic::peekRectName(g_nameRect.x, g_nameRect.y, g_nameRect.w, g_nameRect.h);   // last name rect
        else { lblHide(); return; }
        labelDraw(g_peek.text, r);
        return;
    }
    if (!g_toast.active) { lblHide(); return; }
    if (!qlogic::toastVisible(GetTickCount(), g_toast.shownAt, qlogic::ToastMs) || !g_haveRect) { g_toast.active = false; lblHide(); return; }
    labelDraw(g_toast.text, qlogic::toastRect(g_nameRect.x, g_nameRect.y, 0, 0, g_nameRect.w, g_nameRect.h));
}

// Called from the FRAME tick: POST may not run when there is no target, so expire the toast here too.
void toastExpire(const K2HudContext* ctx) {
    if (qlogic::toastVisible(GetTickCount(), g_toast.shownAt, qlogic::ToastMs)) return;
    g_toast.active = false;
    if (g_peek.active) return;   // the label belongs to the peek
    if (!g_lbl || !ctx || !ctx->hud) return;   // hud unknown: POST hides it
    if (ctx->hud != g_panel) { lblDrop(); return; }
    saferead::beginScope();
    lblHide();
}

void toastStart(const char* text) {
    if (!g_lbl) return;
    strncpy(g_toast.text, text, sizeof g_toast.text - 1);
    g_toast.text[sizeof g_toast.text - 1] = 0;
    g_toast.shownAt = GetTickCount();
    g_toast.active = true;
}

// Logs the TOOK lines and starts the "Took:" toast for a captured item list (shared by the click take and the peek take).
void reportTook(const Captured& cap) {
    char b[160];
    const char* np[qlogic::ToastMaxItems];
    for (unsigned i = 0; i < cap.count; ++i) {
        qlogic::fmtTookItem(b, sizeof b, i, cap.ids[i], cap.names[i], cap.stacks[i]);
        logLine(b);
        np[i] = cap.names[i];
    }
    char text[96];
    if (qlogic::fmtTook(text, sizeof text, np, cap.stacks, cap.count, cap.total > cap.count ? cap.total - cap.count : 0)) {
        qlogic::fmtToast(b, sizeof b, text);
        logLine(b);
        toastStart(text);
    }
}

// ---- QL2p: world state (leader, area), suppress states, key
struct World {
    bool ok = false;
    uint32_t ctrl = 0;           // leader server object id (the OpenInventory opener)
    uintptr_t srv = 0, leader = 0, area = 0;
    float x = 0, y = 0;
};
using Fn0 = uint32_t(__thiscall*)(uintptr_t self);
using OpenInvFn = void(__thiscall*)(uintptr_t plc, uint32_t opener, int playAnim);

// u02 recipe A (same chain as CompanionAI.cpp controlledId + leader object + area). ok = leader object and its area resolved.
bool readWorld(World& w) {
    w = World();
    w.srv = serverApp();
    uint32_t app = 0, client = 0, inner = 0;
    if (!w.srv || !rd<uint32_t>(gameaddr::AppGlobal, 0, &app) || !app || !rd<uint32_t>(app, App_Client, &client) || !client ||
        !rd<uint32_t>(client, Client_Inner, &inner) || !inner)
        return false;
    if (!saferead::probeRead(SiteWorld, reinterpret_cast<const void*>(inner + Inner_PartyTable), 4)) return false;
    const uintptr_t list = reinterpret_cast<Fn0>(gameaddr::ClientPartyTable)(client);
    if (!list || !saferead::probeRead(SiteWorld, reinterpret_cast<const void*>(list), 0x28)) return false;   // [list] count, [list+0x24] entry 0
    const uintptr_t p = reinterpret_cast<LookupFn>(gameaddr::PartyGetAt)(list, 0);
    if (!p || !saferead::probeRead(SiteWorld, reinterpret_cast<const void*>(p), 0x100)) return false;
    const uintptr_t q = reinterpret_cast<Fn0>(gameaddr::MemberServerCreature)(p);
    if (!q || !rd<uint32_t>(q, Obj_Id, &w.ctrl) || !w.ctrl) return false;
    w.leader = reinterpret_cast<LookupFn>(GetGameObject)(w.srv, w.ctrl);
    if (!w.leader || !rd<float>(w.leader, Obj_PosX, &w.x) || !rd<float>(w.leader, Obj_PosY, &w.y)) return false;
    w.area = reinterpret_cast<Fn0>(gameaddr::ServerObjectGetArea)(w.leader);
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

// Ends the peek (any outcome): the container goes into the cooldown set.
void peekEnd() {
    char b[160];
    const uint32_t sid = g_peek.serverId;
    g_cool.add(sid);
    qlogic::fmtCooldown(b, sizeof b, "add", sid, -1.0f, g_cool.n);
    logLine(b);
    g_peek.clear();
}

// Sends the peek's Close(flag) once. Returns false when the message object is not available yet (retry on the next tick, until GiveUpMs).
// plcResolves false: the container is gone, nothing to close (sent stays 0).
bool peekSendClose(int flag, bool plcResolves, uint32_t now, int* sent) {
    *sent = 0;
    if (!plcResolves) return true;
    if (!g_peek.closeStartMs) g_peek.closeStartMs = now ? now : 1;
    const uintptr_t msg = messageObject();
    if (!msg) {
        char b[160];
        if (!g_peek.closeTried) { g_peek.closeTried = true; qlogic::fmtErr(b, sizeof b, "peek-msgobj", qlogic::msSince(now, g_peek.sinceMs)); logLine(b); }
        return qlogic::giveUp(now, g_peek.closeStartMs);   // true = give up, the peek ends without a Close
    }
    reinterpret_cast<SendCloseFn>(SendContainerClose)(msg, g_peek.obj, flag);
    *sent = 1;
    return true;
}

// Re-reads the peeked container and rebuilds the label text (change-only, logged).
void peekRefresh(int n) {
    static Captured cap;
    captureItems(g_peek.plc, cap);
    char key[8];
    qlogic::keyLabel(g_takeKey, key, sizeof key);
    const char* np[qlogic::ToastMaxItems];
    for (unsigned i = 0; i < cap.count; ++i) np[i] = cap.names[i];
    char text[sizeof g_peek.text];
    qlogic::fmtPeek(text, sizeof text, key, n, np, cap.stacks, cap.count);
    if (strcmp(text, g_peek.text) != 0) {
        strncpy(g_peek.text, text, sizeof g_peek.text - 1);
        g_peek.text[sizeof g_peek.text - 1] = 0;
        char b[200];
        qlogic::fmtPeekLabel(b, sizeof b, text);
        logLine(b);
    }
}

// Rising edge of TakeKey while peeking: take everything (Close flag 1), toast, cooldown.
void peekTake(uint32_t now) {
    const int n = itemCount(g_peek.plc);
    if (n <= 0) return;   // nothing listed: the key does nothing
    const uintptr_t msg = messageObject();
    char b[160];
    if (!msg) { qlogic::fmtErr(b, sizeof b, "peek-take-msgobj", qlogic::msSince(now, g_peek.sinceMs)); logLine(b); return; }
    static Captured cap;   // names BEFORE the Close
    captureItems(g_peek.plc, cap);
    const uint32_t obj = g_peek.obj;
    const uint32_t dt = qlogic::msSince(now, g_peek.sinceMs);
    reinterpret_cast<SendCloseFn>(SendContainerClose)(msg, obj, qlogic::closeFlagFor(n));
    qlogic::fmtPeekTake(b, sizeof b, obj, dt, n);
    logLine(b);
    reportTook(cap);
    peekEnd();
}

// Active peek, every ScanMs: leave checks, empty check, label refresh.
void peekPoll(const World& w, qlogic::Sup sup, uint32_t now) {
    char b[200];
    const uint32_t sid = g_peek.serverId;
    const uintptr_t cur = w.srv ? reinterpret_cast<LookupFn>(GetPlaceableByGameObjectID)(w.srv, sid) : 0;
    const bool resolves = cur != 0 && cur == g_peek.plc;
    float dist = 1.0e9f;
    float px = 0, py = 0;
    if (resolves && w.ok && rd<float>(cur, Obj_PosX, &px) && rd<float>(cur, Obj_PosY, &py)) dist = qlogic::dist2d(w.x, w.y, px, py);
    const qlogic::Leave lv = qlogic::leaveReason(resolves, sup != qlogic::Sup::None, qlogic::areaChanged(g_peek.area, w.area), dist, g_closeM);
    if (lv != qlogic::Leave::None) {
        const int n = resolves ? itemCount(cur) : -1;
        int sent = 0;
        if (!peekSendClose(0, resolves, now, &sent)) return;
        qlogic::fmtPeekClose(b, sizeof b, g_peek.obj, lv == qlogic::Leave::Suppress ? (qlogic::supName(sup)) : qlogic::leaveName(lv), sent, dist, n);
        logLine(b);
        peekEnd();
        return;
    }
    const int n = itemCount(cur);
    if (g_peek.emptyNow(n, g_frame, now)) {   // OnOpen settled and still nothing in it: close quietly
        int sent = 0;
        if (!peekSendClose(0, true, now, &sent)) return;
        qlogic::fmtPeekEmpty(b, sizeof b, g_peek.obj, qlogic::msSince(now, g_peek.sinceMs), openFlag(cur));
        logLine(b);
        peekEnd();
        return;
    }
    peekRefresh(n);
}

// Expecting phase: OpenInventory was called, its ShowContainerGui has not come. Gives up after ExpectTimeoutMs.
void peekExpectCheck(uint32_t now) {
    if (!g_peek.expired(now)) return;
    char b[160];
    const int open = openFlag(g_peek.plc);
    qlogic::fmtPeekNoShow(b, sizeof b, g_peek.serverId, qlogic::msSince(now, g_peek.expectMs), open);
    logLine(b);
    // Opened but no client Show came: no client object id to send the Close message with. Close it server-side (flag 0, items stay),
    // else the open flag (+0x390) stays set and OpenInventory refuses this container for the rest of the session.
    if (open == 1 && g_peek.opener) {
        using CloseInvFn = void(__thiscall*)(uintptr_t plc, uint32_t player, int flag);
        reinterpret_cast<CloseInvFn>(CloseInventory)(g_peek.plc, g_peek.opener, 0);
        logLine("PEEK-NOSHOW server close sent");
    }
    peekEnd();
}

// Opens the chosen container with the engine's OpenInventory (lid animation on). The ShowContainerGui it triggers is consumed in QlShowContainer.
void peekOpen(const World& w, uint32_t sid, uintptr_t plc, float dist, uint32_t now) {
    char b[160];
    g_peek.begin(plc, sid, w.area, now);   // before the call: a synchronous Show must already find the expectation
    qlogic::fmtPeekOpen(b, sizeof b, sid, plc, dist, itemCount(plc));
    logLine(b);
    g_peek.opener = w.ctrl;
    reinterpret_cast<OpenInvFn>(OpenInventory)(plc, w.ctrl, 1);
    const int open = openFlag(plc);
    if (open == 0 && !g_peek.active) {   // refused (OpenInventory gates failed): do not retry until the leader has left
        qlogic::fmtPeekRefused(b, sizeof b, sid, open);
        logLine(b);
        peekEnd();
    }
}

// Scan the leader's area for the nearest openable placeable; open it. Cooldown ids are released here once the leader is > CloseM away.
void proxScan(const World& w, uint32_t now) {
    char b[200];
    uint32_t arr = 0; int cnt = 0;
    if (!rd<uint32_t>(w.area, Area_Ids, &arr) || !arr || !rd<int>(w.area, Area_Count, &cnt)) return;
    if (cnt < 0) cnt = 0;
    if (cnt > static_cast<int>(qlogic::MaxScanObjects)) cnt = static_cast<int>(qlogic::MaxScanObjects);
    uint32_t bestId = 0, nearId = 0;
    uintptr_t bestPlc = 0;
    float bestD = 0, nearD = 0;
    qlogic::CandFlags nearF;
    const char* nearWhy = "";
    bool nearOk = false;
    for (int i = 0; i < cnt; ++i) {
        uint32_t id = 0;
        if (!rd<uint32_t>(arr, i * 4, &id) || !id) continue;
        const uintptr_t o = reinterpret_cast<LookupFn>(GetGameObject)(w.srv, id);
        uint8_t type = 0;
        if (!o || !rd<uint8_t>(o, Obj_Type, &type) || type != ObjTypePlaceable) continue;
        float px = 0, py = 0;
        if (!rd<float>(o, Obj_PosX, &px) || !rd<float>(o, Obj_PosY, &py)) continue;
        const float d = qlogic::dist2d(w.x, w.y, px, py);
        bool cd = g_cool.contains(id);
        if (cd && g_cool.release(id, d, g_closeM)) {
            qlogic::fmtCooldown(b, sizeof b, "release", id, d, g_cool.n);
            logLine(b);
            cd = false;
        }
        if (d > g_closeM) continue;
        const uintptr_t plc = reinterpret_cast<LookupFn>(GetPlaceableByGameObjectID)(w.srv, id);
        qlogic::CandFlags f;
        if (!plc || !readCandFlags(plc, f)) continue;
        f.cooldown = cd;
        const char* why = qlogic::candidateWhy(f, d, g_openM);
        const bool ok = strcmp(why, "ok") == 0;
        if (!nearId || d < nearD) { nearId = id; nearD = d; nearF = f; nearWhy = why; nearOk = ok; }
        if (ok && (!bestId || d < bestD)) { bestId = id; bestD = d; bestPlc = plc; }
    }
    // SCAN line only when the nearest in-range placeable (or its verdict) changes -- never per scan
    const int okI = nearOk ? 1 : 0;
    if (nearId != g_nearId || (nearId && okI != g_nearOk)) {
        if (nearId) qlogic::fmtScan(b, sizeof b, nearId, nearD, nearF, nearWhy);
        else qlogic::fmtScanNone(b, sizeof b);
        logLine(b);
    }
    g_nearId = nearId; g_nearOk = nearId ? okI : -1;
    if (bestId) peekOpen(w, bestId, bestPlc, bestD, now);
}

// Proximity peek tick (K2HUD_FRAME). Key edge every frame; the scan / poll every ScanMs.
void proxTick(const K2HudContext* ctx) {
    if (!g_proxOn || !g_enabled) return;
    const bool edge = qlogic::risingEdge(g_keyPrev, takeKeyDown());   // updated every frame: a key held while the peek opens never fires
    const uint32_t now = nowMs();
    if (g_peek.active && edge) { enter(); peekTake(now); return; }
    if (qlogic::msSince(now, g_lastScan) < g_scanMs) return;
    g_lastScan = now;
    enter();
    World w;
    const bool wok = readWorld(w);
    if (wok && qlogic::areaChanged(g_lastArea, w.area)) {
        char b[160];
        g_cool.clear();   // a new area: the old ids mean nothing
        g_nearId = 0; g_nearOk = -1;
        int cnt = -1;
        rd<int>(w.area, Area_Count, &cnt);
        qlogic::fmtArea(b, sizeof b, w.area, cnt);
        logLine(b);
    }
    if (wok) g_lastArea = w.area;
    const qlogic::Sup sup = currentSuppress(ctx, wok);
    const qlogic::Sup prevSup = g_sup.last;
    if (g_sup.update(sup)) {   // change-only
        char b[96];
        qlogic::fmtSuppress(b, sizeof b, qlogic::supName(prevSup), qlogic::supName(sup));
        logLine(b);
    }
    if (g_peek.active) { peekPoll(w, sup, now); return; }
    if (g_peek.expect) { peekExpectCheck(now); return; }
    if (g_take.active || sup != qlogic::Sup::None || !wok) return;
    proxScan(w, now);
}

// K2HUD_FRAME: send the pending take-all Close (click take), then the proximity peek. Runs on the game thread, outside any EventHandler.
void takeTick() {
    uint32_t now = nowMs();
    if (!qlogic::readyToSend(g_take.openSeen, g_frame, g_take.openFrame, now, g_take.showMs)) return;
    enter();
    char b[160];
    uintptr_t msg = messageObject();
    if (!msg) {
        uint32_t dt = qlogic::msSince(now, g_take.showMs);
        if (!g_take.errLogged) { g_take.errLogged = true; qlogic::fmtErr(b, sizeof b, "msgobj", dt); logLine(b); }
        if (qlogic::giveUp(now, g_take.showMs)) { qlogic::fmtErr(b, sizeof b, "giveup", dt); logLine(b); g_take.clear(); }
        return;
    }
    int n = itemCount(g_take.plc);
    const int closeFlag = qlogic::closeFlagFor(n);   // items at send time, not the Show flag (OnOpen loot arrives after Show)
    uint32_t obj = g_take.obj;
    int openSeen = g_take.openSeen ? 1 : 0;
    uint32_t dt = qlogic::msSince(now, g_take.showMs);
    static Captured cap;   // QL1b: names / stacks BEFORE the Close (the repository then includes the OnOpen loot)
    cap.count = 0; cap.total = 0;
    if (closeFlag) captureItems(g_take.plc, cap);
    g_take.clear();   // before the call: a re-entrant Show must not see a stale pending take
    reinterpret_cast<SendCloseFn>(SendContainerClose)(msg, obj, closeFlag);
    qlogic::fmtTakeSend(b, sizeof b, obj, dt, openSeen, n);
    logLine(b);
    if (!closeFlag) {   // empty: closed with flag 0, no toast
        qlogic::fmtTakeEmpty(b, sizeof b, obj, dt, openSeen);
        logLine(b);
        return;
    }
    reportTook(cap);
}

void KPATCH_CALL onHudFrame(const K2HudContext* ctx, void*) {
    if (ctx) g_frame = ctx->frame;
    if (g_toast.active) toastExpire(ctx);
    if (g_take.active) takeTick();
    proxTick(ctx);
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
    g_hud.init(api, "quick-loot");
    g_hud.add(K2HUD_FRAME, 100, onHudFrame);
    g_hud.add(K2HUD_BUILD, 300, onHudBuild);               // QL1b toast label
    g_hud.add(K2HUD_TARGET_MENU_POST, 300, onMenuPost);
    g_hud.connect();
    if (KpLog_HasCore()) KP_LOG(chMain, "INIT quick-loot %s enabled=%d hud=%d prox=%d openM=%.2f closeM=%.2f key=%d scanMs=%u", Version, g_enabled ? 1 : 0,
                                  g_hud.connected ? 1 : 0, g_proxOn ? 1 : 0, static_cast<double>(g_openM), static_cast<double>(g_closeM), g_takeKey, g_scanMs);
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
    if (plc && g_peek.onShow(objId, plc, now, g_frame)) {   // QL2p: the ShowContainerGui of our OpenInventory = a peek, not a take (any flag)
        qlogic::fmtPeekShow(b, sizeof b, objId, plc, flag, n);
        logLine(b);
        return 1;
    }
    g_hud.poll();
    bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    qlogic::Reason why = qlogic::shouldConsume(g_enabled, g_hud.connected, shift, plc != 0, g_take.active);
    if (why != qlogic::Reason::Consume) {
        qlogic::fmtVanilla(b, sizeof b, why);
        logLine(b);
        return 0;
    }
    g_take.arm(objId, plc, now, g_frame);
    qlogic::fmtTakeArm(b, sizeof b, objId, plc, n);
    logLine(b);
    return 1;
}

// H2. ECX = placeable, [esp+4] = a1, [esp+8] = a2, [esp+0xC] = a3 (event record pointer). Runs on every placeable heartbeat: filter first.
extern "C" void __cdecl QlPlaceableEvent(uintptr_t plc, int a1, int a2, int a3) {
    uint32_t now = nowMs();
    if ((g_take.active && plc == g_take.plc) || (plc && plc == g_peek.plc && g_peek.busy())) {   // QL1a / QL2p: note the container's OnOpen (the Close is sent from the HUD frame, never here)
        enter();
        uint16_t tid = 0;
        if (rd<uint16_t>(static_cast<uint32_t>(a3), 0, &tid)) {
            g_take.onEvent(plc, static_cast<short>(tid), g_frame);
            g_peek.onEvent(plc, static_cast<short>(tid), g_frame);
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
