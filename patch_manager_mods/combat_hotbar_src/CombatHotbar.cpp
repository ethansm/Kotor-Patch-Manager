// Combat Action Hotbar (Mod 31, 0.4.0-dev: probe + CH2 invoke keys + CH3b arrow hide + CH4b icon button grid; consumer of hud-host)
// Diagnostic probe for CSWGuiTargetActionMenu (hud + 0xCC).
// Safely reads and logs target action groups when an enemy is targeted.
// 0.2.0: no own hook any more. hud-host owns the PopulateMenus hook (0x00743540) and calls onTargetMenu through k2.hud@1
// (K2HUD_TARGET_MENU_PRE). Connects once in KPatchInit, so hud-host must be listed before combat-hotbar in the config:
// there is no hook of ours to retry g_hud.poll() from.
// 0.3.0-dev: CH2 invoke keys (INI [Invoke]: Ctrl/Shift-masked F9..F11 -> select the last usable entry of group g and call the engine invoke
// wrapper 0x00751710 (thiscall hud, g, 0)) inside TARGET_MENU_PRE; CH3b (INI [Arrows] Hide=1) clears bit1 of the carousel arrow controls
// (menu+0x43C / +0x60C, stride 0x750, 3 each) in TARGET_MENU_POST (priority 100).
// 0.4.0-dev: CH4b clickable icon buttons (INI [Buttons] Enabled/Cols/MaxRows/Gap). K2HUD_BUILD (hud-host 0.3.0+) creates 3*Cols*MaxRows hidden
// labels from LBL_QUEUE1 while the HUD layout is open; TARGET_MENU_POST lays them out under the three vanilla group boxes (ch::layoutGroupGrid,
// 2 columns per group, row-major), fills icon/alpha/tooltip from the group entries and invokes the clicked entry (same path as the CH2 keys).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <vector>
#include <string>

#include "KpLog.h"
#include "SafeRead.h"
#include "HudHost.h"
#include "GameAddr.h"
#include "GuiKit.h"
#include "GuiTip.h"
#include "CombatLogic.h"

KP_LOG_PATCH("combat-hotbar");

namespace {

constexpr unsigned SiteBase = 0x6F00;
constexpr unsigned S_Menu   = SiteBase + 0x10;
constexpr unsigned S_Group  = SiteBase + 0x20;
constexpr unsigned S_Entry  = SiteBase + 0x30;
constexpr unsigned S_Menu2  = SiteBase + 0x40;
constexpr unsigned S_InvRd  = SiteBase + 0x50;   // CH2 reads (target type, group header, entries, selection slot)
constexpr unsigned S_InvWr  = SiteBase + 0x60;   // CH2 selection slot write probe
constexpr unsigned S_ArrWr  = SiteBase + 0x70;   // CH3b arrow flag write probe
constexpr unsigned S_BtnRd  = SiteBase + 0x80;   // CH4b reads (menu origin, 1BAC flag, screen size, entry name)
constexpr unsigned S_ProjRd = SiteBase + 0x90;   // CH4b BUILD projection flag read / write / restore
constexpr unsigned S_ProjWr = SiteBase + 0x91;
constexpr unsigned S_ProjRs = SiteBase + 0x92;

constexpr uintptr_t HudTargetInvoke = 0x00751710;   // thiscall (hud, group, 0), RET 8; reads the selected key (fallback entry[0])

constexpr int EntryStride = 0x3C;
constexpr int MaxEntriesToLog = 8;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chTarget, "target");

struct TargetState {
    uintptr_t targetObj = 0;
    int8_t targetType = -1;
    int counts[3] = { -1, -1, -1 };
    int32_t selKeys[3] = { -1, -1, -1 };
    DWORD lastLogTick = 0;
};

TargetState g_state;
k2hud::Client g_hud;

struct Config {
    bool invokeEnabled = true;
    int mod = 3;                 // bit0 Ctrl, bit1 Shift (must match exactly)
    int keys[3] = { 0x78, 0x79, 0x7A };   // VK_F9..VK_F11
    bool keepSelection = false;
    bool hideArrows = false;
    bool btnEnabled = true;      // CH4b [Buttons]
    int cols = 2;                // 1..4
    int maxRows = 4;             // 1..8
    int gap = 2;                 // 0..16 px
} g_cfg;
ch::KeyEdge g_edge[3];
int g_arrowState = -1;   // -1 never applied, 0 failed, 1 applied (log on change only)

const char* const IniPath = ".\\combat_hotbar.ini";   // ".\\" prefix: GetPrivateProfile*A with a bare name looks in the Windows dir

int clampInt(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

void loadIni() {
    g_cfg.invokeEnabled = GetPrivateProfileIntA("Invoke", "Enabled", 1, IniPath) != 0;
    g_cfg.mod = GetPrivateProfileIntA("Invoke", "Mod", 3, IniPath) & 3;
    const int defKeys[3] = { 0x78, 0x79, 0x7A };
    const char* const names[3] = { "Key0", "Key1", "Key2" };
    for (int i = 0; i < 3; ++i) {
        int k = GetPrivateProfileIntA("Invoke", names[i], defKeys[i], IniPath);
        g_cfg.keys[i] = (k > 0 && k <= 0xFF) ? k : defKeys[i];
    }
    g_cfg.keepSelection = GetPrivateProfileIntA("Invoke", "KeepSelection", 0, IniPath) != 0;
    g_cfg.hideArrows = GetPrivateProfileIntA("Arrows", "Hide", 0, IniPath) != 0;
    g_cfg.btnEnabled = GetPrivateProfileIntA("Buttons", "Enabled", 1, IniPath) != 0;
    g_cfg.cols = clampInt(GetPrivateProfileIntA("Buttons", "Cols", 2, IniPath), 1, 4);
    g_cfg.maxRows = clampInt(GetPrivateProfileIntA("Buttons", "MaxRows", 4, IniPath), 1, 8);
    g_cfg.gap = clampInt(GetPrivateProfileIntA("Buttons", "Gap", 2, IniPath), 0, 16);
    KP_LOGV(chMain, KPLOG_INFO, "config: invoke=%d mod=%d keys=[%02x,%02x,%02x] keepSelection=%d arrowsHide=%d buttons=%d cols=%d maxRows=%d gap=%d",
            (int)g_cfg.invokeEnabled, g_cfg.mod, g_cfg.keys[0], g_cfg.keys[1], g_cfg.keys[2], (int)g_cfg.keepSelection, (int)g_cfg.hideArrows,
            (int)g_cfg.btnEnabled, g_cfg.cols, g_cfg.maxRows, g_cfg.gap);
}

bool modifiersMatch() {
    const bool ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    const bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    return ctrl == ((g_cfg.mod & 1) != 0) && shift == ((g_cfg.mod & 2) != 0);
}

// Snapshot of group g (count clamped to 16, entries read until the first unreadable one). *baseOut = the entry array base (0 = none).
void readGroup(uintptr_t menu, int g, ch::GroupView* gv, uintptr_t* baseOut) {
    memset(gv, 0, sizeof(*gv));
    int count = 0; uintptr_t base = 0;
    saferead::readAt(S_InvRd, menu, g * 12 + 4, &count);
    saferead::readAt(S_InvRd, menu, g * 12, &base);
    if (baseOut) *baseOut = base;
    gv->count = base ? ch::clampCount(count) : 0;
    for (int i = 0; i < gv->count; ++i) {
        uintptr_t e = base + (uintptr_t)i * EntryStride;
        uint32_t flags = 0;
        if (!saferead::readAt(S_InvRd, e, 0x08, &gv->e[i].key) || !saferead::readAt(S_InvRd, e, 0x1C, &gv->e[i].owner) || !saferead::readAt(S_InvRd, e, 0x30, &flags)) { gv->count = i; break; }
        gv->e[i].usable = (flags & 1) != 0;
        saferead::readResRef16(S_InvRd, e + 0x20, gv->e[i].icon, 0);
    }
}

void readGroups(uintptr_t menu, ch::GroupView gv[3], uintptr_t base[3]) {
    for (int g = 0; g < 3; ++g) readGroup(menu, g, &gv[g], &base[g]);
}

// Select entry ev of group g for target type `type` (selection slot menu+0x24+type*12+g*4 := key), call the engine invoke wrapper, restore the slot.
void invokeEntry(uintptr_t hud, uintptr_t menu, int g, int type, int idx, const ch::EntryView& ev) {
    int32_t* slot = reinterpret_cast<int32_t*>(menu + 0x24 + (uintptr_t)type * 12 + (uintptr_t)g * 4);
    int32_t oldSel = 0;
    if (!saferead::readAt(S_InvRd, reinterpret_cast<uintptr_t>(slot), 0, &oldSel)) { KP_LOGV(chTarget, KPLOG_WARN, "invoke g%d: skip, selection slot unreadable", g); return; }
    if (!saferead::probeWrite(S_InvWr, slot, 4)) { KP_LOGV(chTarget, KPLOG_WARN, "invoke g%d: skip, not writable (slot=%p)", g, (void*)slot); return; }
    KP_LOGV(chTarget, KPLOG_INFO, "invoke g%d: pre idx=%d key=%08x oldSel=%08x owner=%08x usable=%d type=%d", g, idx, (unsigned)ev.key, (unsigned)oldSel, (unsigned)ev.owner, (int)ev.usable, type);
    *slot = static_cast<int32_t>(ev.key);
    using Fn = void(__thiscall*)(void*, int, int);
    reinterpret_cast<Fn>(HudTargetInvoke)(reinterpret_cast<void*>(hud), g, 0);
    int32_t selAfter = 0;
    saferead::readAt(S_InvRd, reinterpret_cast<uintptr_t>(slot), 0, &selAfter);
    bool restored = false;
    if (!g_cfg.keepSelection && selAfter == static_cast<int32_t>(ev.key) && saferead::probeWrite(S_InvWr, slot, 4)) { *slot = oldSel; restored = true; }
    KP_LOGV(chTarget, KPLOG_INFO, "invoke g%d: returned selAfter=%08x restored=%d", g, (unsigned)selAfter, (int)restored);
}

// CH2 key path: the last usable owned entry of group g.
void invokeGroup(uintptr_t hud, uintptr_t menu, int g) {
    int8_t type = -1;
    if (!saferead::readAt(S_InvRd, menu, 0x1BAA, &type) || type < 0 || type > 3) { KP_LOGV(chTarget, KPLOG_INFO, "invoke g%d: skip, no target (type=%d)", g, (int)type); return; }
    ch::GroupView gv; uintptr_t base = 0;
    readGroup(menu, g, &gv, &base);
    const int idx = ch::pickInvokeEntry(gv, type);
    if (idx < 0) { KP_LOGV(chTarget, KPLOG_INFO, "invoke g%d: skip, no entry (type=%d count=%d)", g, (int)type, gv.count); return; }
    invokeEntry(hud, menu, g, type, idx, gv.e[idx]);
}

void pollInvokeKeys(const K2HudContext* ctx) {
    if (!g_cfg.invokeEnabled) return;
    const bool active = guikit::detail::winForeground() && modifiersMatch();
    for (int g = 0; g < 3; ++g) {
        const bool down = active && (GetAsyncKeyState(g_cfg.keys[g]) & 0x8000) != 0;
        if (g_edge[g].update(down) && ctx->hud) invokeGroup(static_cast<uintptr_t>(static_cast<uint32_t>(ctx->hud)), static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu)), g);
    }
}

// ---- CH4b: icon button grid under the three vanilla group boxes ----
constexpr int MaxBtn = 3 * 4 * 8;   // 3 groups * Cols max * MaxRows max

struct Btn {
    uintptr_t c = 0;
    int vis = -1;                 // -1 unknown, 0 hidden, 1 shown
    ch::Rect rect = { 0, 0, 0, 0 };
    bool rectSet = false;
    char icon[17] = { 0 };
    bool iconSet = false;
    float alpha = -1.f;
    uint32_t tipKey = 0;
    bool tipSet = false;
};
Btn g_btn[MaxBtn];
int g_nBtn = 0;                    // controls created in the last BUILD
int g_per = 0;                     // Cols*MaxRows used by that BUILD
guikit::ClickEdge g_click;
ch::Rect g_lastRect[MaxBtn];       // last frame's laid-out rects (zeroed when unused)
int g_lastN[3] = { 0, 0, 0 };
bool g_haveLast = false;
bool g_frozen = false;
uintptr_t g_logObj = 0; int g_logType = -2;
int g_flagLast = -1;

// RAII: ProjectionFlag cleared for the label binds, restored on EVERY path (same recipe as companion-selfheal / hotbar BUILD).
struct ProjGuard {
    int* flag; int saved = 0; bool cleared = false;
    ProjGuard() : flag(reinterpret_cast<int*>(gameaddr::ProjectionFlag)) {
        bool readOk = saferead::probeRead(S_ProjRd, flag, 4);
        saved = readOk ? *flag : 0;
        if (saved) { if (saferead::probeWrite(S_ProjWr, flag, 4)) { *flag = 0; cleared = true; } else KP_LOGV(chMain, KPLOG_WARN, "BUILD: projection flag not writable, labels bind with the aspect projection"); }
    }
    ~ProjGuard() { if (cleared) { if (saferead::probeWrite(S_ProjRs, flag, 4)) *flag = saved; else KP_LOGV(chMain, KPLOG_WARN, "BUILD: projection flag restore failed (saved=%d)", saved); } }
};

// BUILD phase (hud-host 0.3.0+): the HUD layout is open, so labels can be created from the LBL_QUEUE1 template. A new HUD = new controls.
void KPATCH_CALL onHudBuild(const K2HudContext* ctx, void*) {
    for (Btn& b : g_btn) b = Btn();
    g_nBtn = 0; g_per = 0; g_click = guikit::ClickEdge(); g_haveLast = false; g_frozen = false; g_logObj = 0; g_logType = -2;
    if (!g_cfg.btnEnabled) { KP_LOGV(chMain, KPLOG_INFO, "BUILD: buttons disabled ([Buttons] Enabled=0)"); return; }
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, panel) + sizeof(ctx->panel) || !ctx->panel) { KP_LOGV(chMain, KPLOG_WARN, "BUILD: no panel in the context"); return; }
    saferead::beginScope();
    const uintptr_t panel = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
    const int per = g_cfg.cols * g_cfg.maxRows, total = 3 * per;
    ProjGuard pg;
    int n = 0, failAlloc = 0, failVt = 0, failTpl = 0, failOther = 0, failAppend = 0;
    uint32_t badVt = 0;
    for (int i = 0; i < total && i < MaxBtn; ++i) {
        int why = guikit::CF_None; uint32_t bv = 0;
        uintptr_t c = guikit::createLabelFromTemplate(panel, "LBL_QUEUE1", &why, &bv);
        if (!c) {
            switch (why) { case guikit::CF_Alloc: ++failAlloc; break; case guikit::CF_Vtable: ++failVt; badVt = bv; break; case guikit::CF_Template: ++failTpl; break; default: ++failOther; break; }
            continue;
        }
        guikit::setVisible(c, false);
        guikit::setClickable(c, false);
        if (!guikit::appendToPanel(panel, c)) { ++failAppend; KP_LOGV(chMain, KPLOG_WARN, "BUILD: append failed, control %08x left hidden + click-through", (unsigned)c); continue; }
        g_btn[i].c = c; g_btn[i].vis = 0; ++n;
    }
    g_nBtn = total < MaxBtn ? total : MaxBtn; g_per = per;
    if (n == total) KP_LOGV(chMain, KPLOG_INFO, "BUILD buttons created=%d/%d (panel=%08x cols=%d rows=%d)", n, total, (unsigned)panel, g_cfg.cols, g_cfg.maxRows);
    else KP_LOGV(chMain, KPLOG_WARN, "BUILD buttons created=%d/%d alloc=%d vtable=%d(%08x) template=%d append=%d other=%d", n, total, failAlloc, failVt, (unsigned)badVt, failTpl, failAppend, failOther);
}

void hideAllButtons() {
    for (int i = 0; i < g_nBtn; ++i) {
        Btn& b = g_btn[i];
        if (!b.c || b.vis == 0) continue;
        guikit::setVisible(b.c, false); guikit::setClickable(b.c, false);
        b.vis = 0; b.tipSet = false;
    }
    g_haveLast = false; g_frozen = false; g_logObj = 0; g_logType = -2;
    memset(g_lastRect, 0, sizeof(g_lastRect)); memset(g_lastN, 0, sizeof(g_lastN));
}

bool anyButtonVisible() { for (int i = 0; i < g_nBtn; ++i) if (g_btn[i].c && g_btn[i].vis == 1) return true; return false; }

void updateButtons(const K2HudContext* ctx) {
    if (!g_cfg.btnEnabled || g_nBtn == 0 || g_per == 0) return;
    const uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    const uintptr_t hud = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->hud));
    const int8_t type = static_cast<int8_t>(ctx->targetType);
    // Click edge is sampled every frame so a press that started while the buttons were hidden never fires when they appear.
    const bool click = guikit::clickEdge(g_click);
    if (!ctx->target || type < 0 || type > 3) { hideAllButtons(); return; }

    uint8_t hoverFlag = 0;
    if (saferead::readAt(S_BtnRd, menu, 0x1BAC, &hoverFlag) && (int)hoverFlag != g_flagLast) { KP_LOGV(chTarget, KPLOG_DEBUG, "menu+0x1BAC = %02x (bit1=%d)", (unsigned)hoverFlag, (hoverFlag >> 1) & 1); g_flagLast = hoverFlag; }

    int sw = 0, sh = 0;
    if (guikit::g_gk.screenWAddr) saferead::readAt(S_BtnRd, guikit::g_gk.screenWAddr, 0, &sw);
    if (guikit::g_gk.screenHAddr) saferead::readAt(S_BtnRd, guikit::g_gk.screenHAddr, 0, &sh);
    int ox = 0, oy = 0, ow = 0, oh = 0;
    bool ok = sw >= 320 && sh >= 200 && saferead::readAt(S_BtnRd, menu, 0x1648, &ox) && saferead::readAt(S_BtnRd, menu, 0x164C, &oy) &&
              saferead::readAt(S_BtnRd, menu, 0x1650, &ow) && saferead::readAt(S_BtnRd, menu, 0x1654, &oh);
    ch::Rect box[3];
    for (int g = 0; ok && g < 3; ++g) {
        int ex = 0, ey = 0, ew = 0, eh = 0;
        if (!guikit::getExtent(menu + 0x54 + (uintptr_t)g * 0x750, &ex, &ey, &ew, &eh)) { ok = false; break; }
        box[g].x = ox + ex; box[g].y = oy + ey; box[g].w = ew; box[g].h = eh;
    }
    if (!ok) { hideAllButtons(); return; }

    ch::GroupView gv[3]; uintptr_t base[3] = { 0, 0, 0 };
    readGroups(menu, gv, base);
    int counts[3] = { gv[0].count, gv[1].count, gv[2].count };
    const int per = g_per;
    ch::Rect rects[MaxBtn]; int outN[3] = { 0, 0, 0 };
    ch::layoutGroupGrid(box, counts, g_cfg.cols, g_cfg.maxRows, g_cfg.gap, sw, sh, rects, outN);

    // Hover freeze: while the cursor is over last frame's buttons, keep their positions (the banner reflows with the carousel), unless the entry counts changed.
    // Union = bounding box of every used slot, so crossing a gap between buttons does not unfreeze for a frame.
    bool freeze = false; int cx = 0, cy = 0;
    if (g_haveLast && outN[0] == g_lastN[0] && outN[1] == g_lastN[1] && outN[2] == g_lastN[2] && guikit::cursorEngine(&cx, &cy)) {
        int x0 = INT_MAX, y0 = INT_MAX, x1 = INT_MIN, y1 = INT_MIN;
        for (int s = 0; s < 3 * per; ++s) {
            const ch::Rect& r = g_lastRect[s];
            if (r.w <= 0 || r.h <= 0) continue;
            if (r.x < x0) x0 = r.x; if (r.y < y0) y0 = r.y;
            if (r.x + r.w > x1) x1 = r.x + r.w; if (r.y + r.h > y1) y1 = r.y + r.h;
        }
        freeze = x0 < x1 && cx >= x0 && cx < x1 && cy >= y0 && cy < y1;
    }
    if (freeze != g_frozen) { g_frozen = freeze; KP_LOGV(chTarget, KPLOG_DEBUG, "buttons: hover freeze %s (cursor %d,%d)", freeze ? "on" : "off", cx, cy); }
    if (freeze) memcpy(rects, g_lastRect, sizeof(ch::Rect) * 3 * per);
    memcpy(g_lastRect, rects, sizeof(ch::Rect) * 3 * per);
    memcpy(g_lastN, outN, sizeof(outN)); g_haveLast = true;

    const uintptr_t obj = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->target));
    if (obj != g_logObj || type != g_logType) {
        g_logObj = obj; g_logType = type;
        KP_LOGV(chTarget, KPLOG_INFO, "buttons: first draw obj=%p type=%d origin=[%d,%d,%d,%d] boxes=[%d,%d,%d,%d][%d,%d,%d,%d][%d,%d,%d,%d] outN=[%d,%d,%d] first=[%d,%d,%d,%d][%d,%d,%d,%d][%d,%d,%d,%d] screen=%dx%d",
                (void*)obj, (int)type, ox, oy, ow, oh,
                box[0].x, box[0].y, box[0].w, box[0].h, box[1].x, box[1].y, box[1].w, box[1].h, box[2].x, box[2].y, box[2].w, box[2].h,
                outN[0], outN[1], outN[2],
                rects[0].x, rects[0].y, rects[0].w, rects[0].h, rects[per].x, rects[per].y, rects[per].w, rects[per].h, rects[2 * per].x, rects[2 * per].y, rects[2 * per].w, rects[2 * per].h,
                sw, sh);
    }

    for (int g = 0; g < 3; ++g) {
        for (int k = 0; k < per; ++k) {
            const int s = g * per + k;
            if (s >= g_nBtn) break;
            Btn& b = g_btn[s];
            if (!b.c) continue;
            if (k >= outN[g]) {
                if (b.vis != 0) { guikit::setVisible(b.c, false); guikit::setClickable(b.c, false); b.vis = 0; b.tipSet = false; }
                continue;
            }
            const ch::EntryView& e = gv[g].e[k];
            const ch::Rect& r = rects[s];
            if (!b.rectSet || b.rect.x != r.x || b.rect.y != r.y || b.rect.w != r.w || b.rect.h != r.h) { guikit::setRect(b.c, r.x, r.y, r.w, r.h); b.rect = r; b.rectSet = true; }
            if (!b.iconSet || strncmp(b.icon, e.icon, 16) != 0) {
                guikit::setFill(b.c, e.icon[0] ? guikit::imageFor(e.icon) : nullptr);
                memcpy(b.icon, e.icon, 16); b.icon[16] = 0; b.iconSet = true;
            }
            const float a = e.usable ? 1.0f : 0.35f;
            if (b.alpha != a) { guikit::setAlpha(b.c, a); b.alpha = a; }
            if (b.vis != 1) { guikit::setVisible(b.c, true); guikit::setClickable(b.c, true); b.vis = 1; }
            if (!b.tipSet || b.tipKey != e.key) {
                char name[64] = { 0 };
                saferead::readExo(S_BtnRd, base[g] + (uintptr_t)k * EntryStride, name, sizeof(name));
                if (name[0]) guitip::setNative(b.c, name);
                b.tipKey = e.key; b.tipSet = true;
            }
        }
    }

    if (click && anyButtonVisible() && guikit::cursorEngine(&cx, &cy)) {
        const int hit = ch::hitSlot(rects, 3 * per, cx, cy);
        int g = -1;
        const int idx = ch::slotEntry(hit, g_cfg.cols, g_cfg.maxRows, &g);
        if (idx >= 0 && g >= 0 && idx < outN[g] && g_btn[hit].c && g_btn[hit].vis == 1 && hud) {
            KP_LOGV(chTarget, KPLOG_INFO, "click g=%d idx=%d key=%08x", g, idx, (unsigned)gv[g].e[idx].key);
            invokeEntry(hud, menu, g, type, idx, gv[g].e[idx]);
        }
    }
}

// CH3b: clear bit1 (visible) of the 6 carousel arrow controls after the stock population; log only when the outcome changes. Then CH4b buttons.
void KPATCH_CALL onTargetMenuPost(const K2HudContext* ctx, void*) {
    if (!ctx || !ctx->menu) return;
    saferead::beginScope();
    const uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    if (g_cfg.hideArrows) {
        bool ok = true;
        for (int i = 0; i < 3; ++i) {
            const uintptr_t addrs[2] = { menu + 0x43C + (uintptr_t)i * 0x750, menu + 0x60C + (uintptr_t)i * 0x750 };
            for (uintptr_t a : addrs) {
                uint32_t* p = reinterpret_cast<uint32_t*>(a);
                if (!saferead::probeWrite(S_ArrWr, p, 4)) { ok = false; continue; }
                *p &= ~2u;
            }
        }
        const int st = ok ? 1 : 0;
        if (st != g_arrowState) { g_arrowState = st; KP_LOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "arrows: hide %s (menu=%08x)", ok ? "applied" : "FAILED (not writable)", (unsigned)menu); }
    }
    updateButtons(ctx);
}

void onConnect(bool ok, const char* why) {
    KP_LOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "k2.hud connect: %s (%s)", ok ? "ok" : "FAILED", why ? why : "?");
}

void logTargetGroup(uintptr_t menu, int g, int count) {
    if (count <= 0) return;
    uintptr_t base = 0;
    if (!saferead::readAt(S_Group, menu, g * 12, &base) || !base) return;

    int logCount = (count > MaxEntriesToLog) ? MaxEntriesToLog : count;
    for (int i = 0; i < logCount; ++i) {
        uintptr_t e = base + (uintptr_t)i * EntryStride;
        if (!saferead::probeRead(S_Entry, reinterpret_cast<const void*>(e), EntryStride)) break;

        uint32_t key = 0, flags = 0, strPtr = 0;
        int32_t strLen = 0;
        saferead::readAt(S_Entry, e, 0x08, &key);
        saferead::readAt(S_Entry, e, 0x30, &flags);
        saferead::readAt(S_Entry, e, 0x00, &strPtr);
        saferead::readAt(S_Entry, e, 0x04, &strLen);
        uint32_t fn = 0, owner = 0;
        saferead::readAt(S_Entry, e, 0x0C, &fn);
        saferead::readAt(S_Entry, e, 0x1C, &owner);

        char name[64] = {0};
        saferead::readExoLoose(S_Entry, strPtr, strLen, name, sizeof(name));

        char icon[17] = {0};
        saferead::readResRef16(S_Entry, e + 0x20, icon, 0);

        KP_LOGV(chTarget, KPLOG_DEBUG, "  g%d[%d]: key=%08x usable=%d fn=%08x owner=%08x icon=%.16s name=%s",
                g, i, (unsigned)key, (int)(flags & 1), (unsigned)fn, (unsigned)owner, icon, name);
    }
}

void KPATCH_CALL onTargetMenu(const K2HudContext* ctx, void*) {
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, targetType) + sizeof(ctx->targetType)) return;
    if (!ctx->menu) return;
    saferead::beginScope();   // cheap; hud-host also scopes each PopulateMenus call
    pollInvokeKeys(ctx);
    uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    uintptr_t leader = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->leader));
    uintptr_t targetObj = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->target));
    const int forceUnusable = ctx->forceUnusable;

    // ctx->targetType (read from menu+0x1BAA by hud-host): target type 0..3, -1 = no target (engine 0xFF).
    const int8_t targetType = static_cast<int8_t>(ctx->targetType);
    int menuState = 0;
    saferead::readAt(S_Menu2, menu, 0x1644, &menuState);

    int counts[3] = {0, 0, 0};
    saferead::readAt(S_Menu, menu, 0x04, &counts[0]);
    saferead::readAt(S_Menu, menu, 0x10, &counts[1]);
    saferead::readAt(S_Menu, menu, 0x1C, &counts[2]);

    int32_t selKeys[3] = { -1, -1, -1 };
    if (targetType >= 0 && targetType <= 3) {
        uintptr_t selBase = menu + 0x24 + (uintptr_t)targetType * 12;
        saferead::readAt(S_Menu, selBase, 0, &selKeys[0]);
        saferead::readAt(S_Menu, selBase, 4, &selKeys[1]);
        saferead::readAt(S_Menu, selBase, 8, &selKeys[2]);
    }

    DWORD now = GetTickCount();
    bool changed = (targetObj != g_state.targetObj) ||
                   (targetType != g_state.targetType) ||
                   (counts[0] != g_state.counts[0]) ||
                   (counts[1] != g_state.counts[1]) ||
                   (counts[2] != g_state.counts[2]) ||
                   (selKeys[0] != g_state.selKeys[0]) ||
                   (selKeys[1] != g_state.selKeys[1]) ||
                   (selKeys[2] != g_state.selKeys[2]);

    if (changed || (targetObj != 0 && (now - g_state.lastLogTick >= 5000))) {
        g_state.targetObj = targetObj;
        g_state.targetType = targetType;
        g_state.counts[0] = counts[0];
        g_state.counts[1] = counts[1];
        g_state.counts[2] = counts[2];
        g_state.selKeys[0] = selKeys[0];
        g_state.selKeys[1] = selKeys[1];
        g_state.selKeys[2] = selKeys[2];
        g_state.lastLogTick = now;

        if (targetObj != 0 || counts[0] > 0 || counts[1] > 0 || counts[2] > 0) {
            char typeStr[16];
            if (targetType >= 0 && targetType <= 3) snprintf(typeStr, sizeof(typeStr), "%d", (int)targetType);
            else snprintf(typeStr, sizeof(typeStr), "none");
            KP_LOG(chTarget, "target_menu: obj=%p leader=%p forceUnusable=%d state1644=%d type=%s counts=[%d,%d,%d] sel=[%08x,%08x,%08x]",
                   (void*)targetObj, (void*)leader, forceUnusable, menuState, typeStr,
                   counts[0], counts[1], counts[2],
                   (unsigned)selKeys[0], (unsigned)selKeys[1], (unsigned)selKeys[2]);

            for (int g = 0; g < 3; ++g) {
                logTargetGroup(menu, g, counts[g]);
            }
        }
    }
}

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_hud.init(api, "combat-hotbar");
    g_hud.onConnect = onConnect;
    loadIni();
    g_hud.add(K2HUD_TARGET_MENU_PRE, 100, onTargetMenu);
    g_hud.add(K2HUD_TARGET_MENU_POST, 100, onTargetMenuPost);
    g_hud.add(K2HUD_BUILD, 300, onHudBuild);
    g_hud.connect();
}

} // extern "C"
