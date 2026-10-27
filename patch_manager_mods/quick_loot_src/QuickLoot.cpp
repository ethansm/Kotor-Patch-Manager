// Quick Loot 0.2.0-dev (doc 52 QL1b on QL1a): clicking a loot container takes everything at once, no container menu, then a one-line
//   "Took: Medpac x2, Blaster Pistol, +3 more" toast shows under the target name tag for 3000 ms (label made in K2HUD_BUILD, placed in
//   K2HUD_TARGET_MENU_POST, hidden from POST / FRAME when expired). Item names are captured right before the Close is sent (TOOK log lines).
//   QlShowContainer consumes ShowContainerGui (return 1) when Enabled, hud-host is connected, flag == 1 and Shift is up; the container is then
//   "pending". QlPlaceableEvent notes its OnOpen (event 0x16). The hud-host FRAME tick sends the vanilla OK-button message (0x0087A550, Close
//   flag 1 = take all) 2 frames after OnOpen, or 1000 ms after Show when no OnOpen came. Close is never called from inside the EventHandler hook.
//   Anything else returns 0 (vanilla menu). Shift held = vanilla menu. Config: quick_loot.ini [QuickLoot] Enabled=1 (read once at init).
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

KP_LOG_PATCH("quick-loot");

namespace {

constexpr const char* Version = "0.2.0-dev";
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
const char* const IniPath = ".\\quick_loot.ini";   // ".\\" prefix: GetPrivateProfile*A with a bare name looks in the Windows dir

KP_CHANNEL(chMain, "main");

qlogic::Tracker g_t;
qlogic::Take g_take;
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
    bool textDirty = false;
    char text[96] = {0};
};
Toast g_toast;
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

// Draws the live toast (text, rect, visibility), every setter change-only.
void toastDraw() {
    if (!g_styled) {
        if (!(guikit::setFont(g_lbl, "dialogfont10x10") && guikit::setAlign(g_lbl, 18) && guikit::setTextColor(g_lbl, 0.9f, 0.85f, 0.5f))) { lblHide(); return; }
        g_styled = true;
    }
    if (g_toast.textDirty) {
        if (!guikit::setText(g_lbl, g_toast.text)) { lblHide(); return; }
        g_toast.textDirty = false;
    }
    const qlogic::Rect r = qlogic::toastRect(g_nameRect.x, g_nameRect.y, 0, 0, g_nameRect.w, g_nameRect.h);
    if (!g_rectSet || r != g_lastRect) { guikit::setRect(g_lbl, r.x, r.y, r.w, r.h); g_lastRect = r; g_rectSet = true; }
    if (g_vis != 1) { guikit::setVisible(g_lbl, true); guikit::setClickable(g_lbl, false); g_vis = 1; }   // shown labels stay click-through
}

// K2HUD_TARGET_MENU_POST: runs before every drawn HUD frame (also with no target). Caches the name rect while the target menu is shown.
void KPATCH_CALL onMenuPost(const K2HudContext* ctx, void*) {
    if (!g_lbl || !ctx) return;
    if (ctx->hud != g_panel) { lblDrop(); return; }   // stale HUD: no writes to the old label
    saferead::beginScope();
    const uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    uint8_t flags = 0;
    if (menu && rd<uint8_t>(menu, Menu_Flags, &flags) && (flags & 1)) {
        int32_t mx = 0, my = 0, nx = 0, ny = 0, nw = 0, nh = 0;
        if (rd<int32_t>(menu, Menu_OriginX, &mx) && rd<int32_t>(menu, Menu_OriginY, &my) && rd<int32_t>(menu, Menu_NameX, &nx) &&
            rd<int32_t>(menu, Menu_NameY, &ny) && rd<int32_t>(menu, Menu_NameW, &nw) && rd<int32_t>(menu, Menu_NameH, &nh)) {
            g_nameRect = { mx + nx, my + ny, nw, nh };   // screen rect of the name label
            g_haveRect = true;
        }
    }
    if (!g_toast.active) { lblHide(); return; }
    if (!qlogic::toastVisible(GetTickCount(), g_toast.shownAt, qlogic::ToastMs) || !g_haveRect) { g_toast.active = false; lblHide(); return; }
    toastDraw();
}

// Called from the FRAME tick: POST may not run when there is no target, so expire the toast here too.
void toastExpire(const K2HudContext* ctx) {
    if (qlogic::toastVisible(GetTickCount(), g_toast.shownAt, qlogic::ToastMs)) return;
    g_toast.active = false;
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
    g_toast.textDirty = true;
    g_toast.active = true;
}

// K2HUD_FRAME: send the pending take-all Close. Runs on the game thread, outside any EventHandler.
void KPATCH_CALL onHudFrame(const K2HudContext* ctx, void*) {
    if (ctx) g_frame = ctx->frame;
    if (g_toast.active) toastExpire(ctx);
    if (!g_take.active) return;
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
    uint32_t obj = g_take.obj;
    int openSeen = g_take.openSeen ? 1 : 0;
    uint32_t dt = qlogic::msSince(now, g_take.showMs);
    static Captured cap;   // QL1b: names / stacks BEFORE the Close (the repository then includes the OnOpen loot)
    captureItems(g_take.plc, cap);
    g_take.clear();   // before the call: a re-entrant Show must not see a stale pending take
    reinterpret_cast<SendCloseFn>(SendContainerClose)(msg, obj, 1);
    qlogic::fmtTakeSend(b, sizeof b, obj, dt, openSeen, n);
    logLine(b);
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

}   // namespace

extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_enabled = GetPrivateProfileIntA("QuickLoot", "Enabled", 1, IniPath) != 0;
    g_hud.init(api, "quick-loot");
    g_hud.add(K2HUD_FRAME, 100, onHudFrame);
    g_hud.add(K2HUD_BUILD, 300, onHudBuild);               // QL1b toast label
    g_hud.add(K2HUD_TARGET_MENU_POST, 300, onMenuPost);
    g_hud.connect();
    if (KpLog_HasCore()) KP_LOG(chMain, "INIT quick-loot %s enabled=%d hud=%d", Version, g_enabled ? 1 : 0, g_hud.connected ? 1 : 0);
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
    g_hud.poll();
    bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    qlogic::Reason why = qlogic::shouldConsume(g_enabled, g_hud.connected, flag, shift, plc != 0, g_take.active);
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
    if (g_take.active && plc == g_take.plc) {   // QL1a: note the container's OnOpen (the Close is sent from the HUD frame, never here)
        enter();
        uint16_t tid = 0;
        if (rd<uint16_t>(static_cast<uint32_t>(a3), 0, &tid)) g_take.onEvent(plc, static_cast<short>(tid), g_frame);
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
