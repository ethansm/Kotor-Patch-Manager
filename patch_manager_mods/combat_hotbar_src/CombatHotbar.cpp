// Combat Action Hotbar (Mod 31, 0.8.0-dev: probe + CH2 invoke keys + CH3b arrow hide + CH4c own buttons replacing the vanilla target boxes + CH4d per-group pill containers + CH4e content-sized pills + CH5 profile pin/hide/sort; consumer of hud-host)
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
// 0.5.0-dev: CH4c replaces the three vanilla target-menu group boxes (BTN/LBL/UP/DOWN, bit1 cleared every POST) with our own vanilla-styled buttons
// (lbl_miscroll_f frame, lbl_miscroll_hi on hover, entry icon inset on top = 2 labels per slot). TARGET_MENU_POST now runs at the PopulateMenus tail
// (hud-host 0.4.0), after the engine wrote the menu origin and before anything is drawn, so origin reads are zero-lag. ch::layoutBannerGrid puts three
// columns under the banner (LBL_NAME + PB_HEALTH); while the cursor is over the grid the origin menu+0x1648/+0x164C is frozen (written back) so the banner
// stops following the target. INI [Buttons] Replace/Cols/MaxRows/BtnPx/Pad/GroupGap/Inset/ClickSound (Gap is gone); hover plays GUI sound 1, a click ClickSound.
// 0.6.0-dev: CH4d draws one vanilla pill container per target-menu group behind the buttons (3 labels from LBL_QUEUE1 created in BUILD before the slot labels, so they
// draw underneath; lbl_miscroll_f art). ch::layoutBannerGroups sizes the pills (INI [Buttons] CapPad = top/bottom padding, SidePad = left/right padding) and puts the
// buttons inside; empty groups keep their pill. Tint and alpha of each pill (and of that group's button frames) are copied live every POST from the hidden vanilla
// BTN_TARGETn (getBorderTint/getAlpha), so the pills keep the vanilla look; the hover highlight keeps the copied tint. The pills are clickable but carry no action,
// so a click in a gap between buttons is swallowed instead of reaching the world. The hover-freeze area is the union of last frame's three pills.
// 0.7.0-dev: CH4e hides the pill of an empty group and draws no pills at all when every group is empty (matches the vanilla row collapse in 00743850). [Buttons] ColsTotal
// (default 6) is split across the visible pills (3x2, 2x3, 1x6); every pill gets the same content-sized width and height and the set is centred under the banner
// (ch::layoutBannerPills). Reflow click guard: a click, or a hover sound, on the frame the group counts change is ignored (the buttons moved under the cursor).
// [Buttons] HideEmpty=0 restores the CH4d three-equal-pill layout (ch::layoutBannerGroups).
// 0.8.0-dev: CH5 [Profile] Pin0..2/Hide0..2 (comma lists; a token matches an entry's name exactly, or as a " "-suffix when no entry has that exact name, or a 0x engine key)
// and Sort0..2=count|engine (default count for grenades g2, by stack size +0x34); tiers are native (doc 31 sec 2H). combat_hotbar.ini hot-reloads within 1 s (FileWatch); [General] Verbose=1 (default 0) enables the per-frame/probe log lines
// except Buttons Enabled/Cols/MaxRows, which take effect after a game reload. The reflow guard also compares an order signature.
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
#include "FileWatch.h"

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
constexpr unsigned S_OrgWr  = SiteBase + 0xA0;   // CH4c menu origin (+0x1648/+0x164C) freeze write probe
constexpr unsigned S_VanWr  = SiteBase + 0xA1;   // CH4c vanilla group control flag write probe (hide / restore)

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
    bool replace = true;         // CH4c: hide the vanilla group controls
    int btnPx = 0;               // 0 = auto (vanilla BTN width / 2, capped by fit), else 8..256
    int pad = ch::kDefaultPad;                 // was 4: 0..32 px between buttons / rows
    int groupGap = ch::kDefaultGroupGap;       // was 8: 0..64 px between the three group columns
    int inset = 3;                             // 0..16 px icon inset inside the frame
    int clickSound = 0;                        // GUI sound idx on click, -1 off
    int capPad = ch::kDefaultCapPad;           // was 10: 0..64 px above/below the buttons inside a group pill
    int sidePad = ch::kDefaultSidePad;         // was 6: 0..64 px left/right of the buttons inside a group pill
    int hideEmpty = 1;                         // CH4e 0/1: hide the pill of an empty group, content-sized centred pills (0 = CH4d three equal pills)
    int colsTotal = 6;                         // CH4e 1..12: button columns split across the visible pills (6 = 3x2, 2x3, 1x6)
    bool verbose = false;                      // CH6 [General] Verbose: per-frame / probe logging (default off)
} g_cfg;

// CH6: high-volume and probe log lines only when [General] Verbose=1. Lines the log analyzers parse (profile:, config: reloaded, pills: visible=, click g=) and warnings stay on KP_LOGV.
#define CH_VLOG(ch, lvl, ...) do { if (g_cfg.verbose) KP_LOGV(ch, lvl, __VA_ARGS__); } while (0)
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
    g_cfg.replace = GetPrivateProfileIntA("Buttons", "Replace", 1, IniPath) != 0;
    const int bp = GetPrivateProfileIntA("Buttons", "BtnPx", 0, IniPath);
    g_cfg.btnPx = bp <= 0 ? 0 : clampInt(bp, 8, 256);
    g_cfg.pad = clampInt(GetPrivateProfileIntA("Buttons", "Pad", ch::kDefaultPad, IniPath), 0, 32);
    g_cfg.groupGap = clampInt(GetPrivateProfileIntA("Buttons", "GroupGap", ch::kDefaultGroupGap, IniPath), 0, 64);
    g_cfg.inset = clampInt(GetPrivateProfileIntA("Buttons", "Inset", 3, IniPath), 0, 16);
    g_cfg.clickSound = clampInt(GetPrivateProfileIntA("Buttons", "ClickSound", 0, IniPath), -1, 18);
    g_cfg.capPad = clampInt(GetPrivateProfileIntA("Buttons", "CapPad", ch::kDefaultCapPad, IniPath), 0, 64);
    g_cfg.sidePad = clampInt(GetPrivateProfileIntA("Buttons", "SidePad", ch::kDefaultSidePad, IniPath), 0, 64);
    g_cfg.hideEmpty = GetPrivateProfileIntA("Buttons", "HideEmpty", 1, IniPath) != 0 ? 1 : 0;
    g_cfg.colsTotal = clampInt(GetPrivateProfileIntA("Buttons", "ColsTotal", 6, IniPath), 1, 12);
    g_cfg.verbose = GetPrivateProfileIntA("General", "Verbose", 0, IniPath) != 0;
    KP_LOGV(chMain, KPLOG_INFO, "config: invoke=%d mod=%d keys=[%02x,%02x,%02x] keepSelection=%d arrowsHide=%d buttons=%d replace=%d cols=%d maxRows=%d btnPx=%d pad=%d groupGap=%d inset=%d clickSound=%d capPad=%d sidePad=%d hideEmpty=%d colsTotal=%d",
            (int)g_cfg.invokeEnabled, g_cfg.mod, g_cfg.keys[0], g_cfg.keys[1], g_cfg.keys[2], (int)g_cfg.keepSelection, (int)g_cfg.hideArrows,
            (int)g_cfg.btnEnabled, (int)g_cfg.replace, g_cfg.cols, g_cfg.maxRows, g_cfg.btnPx, g_cfg.pad, g_cfg.groupGap, g_cfg.inset, g_cfg.clickSound, g_cfg.capPad, g_cfg.sidePad, g_cfg.hideEmpty, g_cfg.colsTotal);
}

ch::Profile g_prof;
filewatch::ReloadPoll g_iniPoll;
static char g_names[3][16][64];      // CH5 entry names per group (engine order), re-read when the group's identity signature changes
static uint32_t g_nameSig[3];
static bool g_nameValid[3];

void loadProfile() {
    ch::defaultProfile(g_prof);
    static const char* const keys[9] = { "Pin0", "Pin1", "Pin2", "Hide0", "Hide1", "Hide2", "Sort0", "Sort1", "Sort2" };
    for (int i = 0; i < 9; ++i) {
        char buf[512] = { 0 };
        GetPrivateProfileStringA("Profile", keys[i], "", buf, sizeof(buf), IniPath);
        if (!buf[0]) continue;   // empty: default (Sort keeps the default mode)
        if (!ch::parseProfileLine(g_prof, keys[i], buf)) KP_LOGV(chMain, KPLOG_WARN, "profile: %s=%s not understood", keys[i], buf);
    }
    char raw[1024] = { 0 }, line[1400] = { 0 };
    const int n = ch::formatProfile(g_prof, raw, sizeof(raw));
    int o = 0;
    for (int i = 0; n > 0 && i < n && o < (int)sizeof(line) - 3; ++i) {
        if (raw[i] == '\n') { line[o++] = ';'; line[o++] = ' '; } else line[o++] = raw[i];
    }
    line[o] = 0;
    KP_LOGV(chMain, KPLOG_INFO, "profile: %s", line);
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
        if (!saferead::readAt(S_InvRd, e, 0x34, &gv->e[i].count)) gv->e[i].count = 0;
    }
}

void readGroups(uintptr_t menu, ch::GroupView gv[3], uintptr_t base[3]) {
    for (int g = 0; g < 3; ++g) readGroup(menu, g, &gv[g], &base[g]);
}

// Returns true when the engine invoke ran. Select entry ev of group g for target type `type` (selection slot menu+0x24+type*12+g*4 := key), call the engine invoke wrapper, restore the slot.
bool invokeEntry(uintptr_t hud, uintptr_t menu, int g, int type, int idx, const ch::EntryView& ev) {
    int32_t* slot = reinterpret_cast<int32_t*>(menu + 0x24 + (uintptr_t)type * 12 + (uintptr_t)g * 4);
    int32_t oldSel = 0;
    if (!saferead::readAt(S_InvRd, reinterpret_cast<uintptr_t>(slot), 0, &oldSel)) { KP_LOGV(chTarget, KPLOG_WARN, "invoke g%d: skip, selection slot unreadable", g); return false; }
    if (!saferead::probeWrite(S_InvWr, slot, 4)) { KP_LOGV(chTarget, KPLOG_WARN, "invoke g%d: skip, not writable (slot=%p)", g, (void*)slot); return false; }
    KP_LOGV(chTarget, KPLOG_INFO, "invoke g%d: pre idx=%d key=%08x oldSel=%08x owner=%08x usable=%d type=%d", g, idx, (unsigned)ev.key, (unsigned)oldSel, (unsigned)ev.owner, (int)ev.usable, type);
    *slot = static_cast<int32_t>(ev.key);
    using Fn = void(__thiscall*)(void*, int, int);
    reinterpret_cast<Fn>(HudTargetInvoke)(reinterpret_cast<void*>(hud), g, 0);
    int32_t selAfter = 0;
    saferead::readAt(S_InvRd, reinterpret_cast<uintptr_t>(slot), 0, &selAfter);
    bool restored = false;
    if (!g_cfg.keepSelection && selAfter == static_cast<int32_t>(ev.key) && saferead::probeWrite(S_InvWr, slot, 4)) { *slot = oldSel; restored = true; }
    KP_LOGV(chTarget, KPLOG_INFO, "invoke g%d: returned selAfter=%08x restored=%d", g, (unsigned)selAfter, (int)restored);
    return true;
}

// CH2 key path: the last usable owned entry of group g.
void invokeGroup(uintptr_t hud, uintptr_t menu, int g) {
    int8_t type = -1;
    if (!saferead::readAt(S_InvRd, menu, 0x1BAA, &type) || type < 0 || type > 3) { CH_VLOG(chTarget, KPLOG_INFO, "invoke g%d: skip, no target (type=%d)", g, (int)type); return; }
    ch::GroupView gv; uintptr_t base = 0;
    readGroup(menu, g, &gv, &base);
    const int idx = ch::pickInvokeEntry(gv, type);
    if (idx < 0) { CH_VLOG(chTarget, KPLOG_INFO, "invoke g%d: skip, no entry (type=%d count=%d)", g, (int)type, gv.count); return; }
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

// ---- CH4c: own vanilla-styled buttons replacing the three vanilla target-menu group boxes ----
constexpr int MaxBtn = 3 * 4 * 8;   // 3 groups * Cols max * MaxRows max
constexpr const char* FrameResref = "lbl_miscroll_f";    // doc 31b Q: 64x128 pill, stretched into a square (accepted)
constexpr const char* HiResref = "lbl_miscroll_hi";

// One engine label with change-only setters (cache = last value written).
struct Wid {
    uintptr_t c = 0;
    int vis = -1;                 // -1 unknown, 0 hidden, 1 shown
    ch::Rect rect = { 0, 0, 0, 0 };
    bool rectSet = false;
    void* fill = nullptr;
    bool fillSet = false;
    float alpha = -1.f;
    float tint[3] = { 0.f, 0.f, 0.f };
    bool tintSet = false;
};

struct Btn {
    Wid bg, icon;
    char iconName[17] = { 0 };
    bool iconSet = false;
    uint32_t tipKey = 0;
    bool tipSet = false;
};
Btn g_btn[MaxBtn];
Wid g_cont[3];                     // CH4d one pill container per group (created before the slot labels so they draw behind the buttons)
int g_nBtn = 0;                    // slots created in the last BUILD
int g_per = 0;                     // Cols*MaxRows used by that BUILD
guikit::ClickEdge g_click;
ch::Rect g_lastRect[MaxBtn];       // last frame's displayed button rects (zeroed when unused)
ch::Rect g_lastCont[3];            // last frame's pill rects; their union is the hover-freeze area
int g_lastN[3] = { 0, 0, 0 };
uint32_t g_lastSig[3] = { 0, 0, 0 };   // CH5 order signature of last frame (reflow guard)
bool g_haveLast = false;
int g_pillMask = -1;               // CH4e last logged visible-pill bitmask (bit g = group g has a pill), -1 = nothing logged yet
ch::FreezeState g_freeze = { false, 0, 0 };
bool g_frozenLog = false;
int g_hover = -1;                  // flat slot under the cursor, -1 none
void* g_imgFrame = nullptr;        // guikit::imageFor results, cached once non-null (pooled, never released)
void* g_imgHi = nullptr;
bool g_imgWarned = false;
bool g_replaced = false;           // vanilla bit1 cleared by us (Replace=1), so Replace=0 must restore it
int g_replState = -1;              // -1 never applied, 0 failed, 1 applied (log on change only)
uintptr_t g_logObj = 0; int g_logType = -2;
int g_flagLast = -1;
int g_contState[3] = { -1, -1, -1 };          // -1 never logged, 0 tint read failed, 1 tint read ok (log on change only)
float g_contLog[3][4] = { { 0 }, { 0 }, { 0 } };   // last logged tint rgb + alpha per group

// GUI sound by index (mirrors guikit::playHoverSound, which is idx 1): idx < 0 = off. False when the manager is null.
bool playGuiSound(int idx) {
    if (idx < 0) return false;
    uint32_t mgr = 0;
    if (!guikit::g_gk.playSound || !guikit::g_gk.guiSoundMgrAddr || !guikit::detail::rd32(guikit::g_gk.guiSoundMgrAddr, 0, &mgr) || !mgr) return false;
    guikit::g_gk.playSound(reinterpret_cast<void*>(static_cast<uintptr_t>(mgr)), static_cast<char>(idx));
    return true;
}

void widHide(Wid& w) {
    if (!w.c || w.vis == 0) return;
    guikit::setVisible(w.c, false); guikit::setClickable(w.c, false);
    w.vis = 0;
}
void widShow(Wid& w, bool clickable) {
    if (!w.c || w.vis == 1) return;
    guikit::setVisible(w.c, true); guikit::setClickable(w.c, clickable);
    w.vis = 1;
}
void widRect(Wid& w, const ch::Rect& r) {
    if (!w.c) return;
    if (!w.rectSet || w.rect.x != r.x || w.rect.y != r.y || w.rect.w != r.w || w.rect.h != r.h) { guikit::setRect(w.c, r.x, r.y, r.w, r.h); w.rect = r; w.rectSet = true; }
}
void widFill(Wid& w, void* img) {
    if (!w.c) return;
    if (!w.fillSet || w.fill != img) { guikit::setFill(w.c, img); w.fill = img; w.fillSet = true; }
}
void widAlpha(Wid& w, float a) {
    if (!w.c) return;
    if (w.alpha != a) { guikit::setAlpha(w.c, a); w.alpha = a; }
}

void widTint(Wid& w, const float* rgb) {
    if (!w.c) return;
    if (!w.tintSet || w.tint[0] != rgb[0] || w.tint[1] != rgb[1] || w.tint[2] != rgb[2]) {
        guikit::setBorderTint(w.c, rgb);
        w.tint[0] = rgb[0]; w.tint[1] = rgb[1]; w.tint[2] = rgb[2]; w.tintSet = true;
    }
}

ch::Rect unionRect(const ch::Rect& a, const ch::Rect& b) {
    const int x0 = a.x < b.x ? a.x : b.x, y0 = a.y < b.y ? a.y : b.y;
    const int x1 = (a.x + a.w) > (b.x + b.w) ? (a.x + a.w) : (b.x + b.w), y1 = (a.y + a.h) > (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    ch::Rect r = { x0, y0, x1 - x0, y1 - y0 };
    return r;
}

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
// Per slot: bg first, then icon (later in the panel list draws on top).
void KPATCH_CALL onHudBuild(const K2HudContext* ctx, void*) {
    for (Btn& b : g_btn) b = Btn();
    for (Wid& w : g_cont) w = Wid();
    for (int g = 0; g < 3; ++g) { g_contState[g] = -1; for (int k = 0; k < 4; ++k) g_contLog[g][k] = 0.f; }
    memset(g_lastCont, 0, sizeof(g_lastCont));
    memset(g_lastSig, 0, sizeof(g_lastSig));
    g_nBtn = 0; g_per = 0; g_click = guikit::ClickEdge(); g_haveLast = false; g_pillMask = -1; g_freeze = { false, 0, 0 }; g_frozenLog = false; g_hover = -1; g_logObj = 0; g_logType = -2;
    if (!g_cfg.btnEnabled) { KP_LOGV(chMain, KPLOG_INFO, "BUILD: buttons disabled ([Buttons] Enabled=0)"); return; }
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, panel) + sizeof(ctx->panel) || !ctx->panel) { KP_LOGV(chMain, KPLOG_WARN, "BUILD: no panel in the context"); return; }
    saferead::beginScope();
    const uintptr_t panel = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
    const int per = g_cfg.cols * g_cfg.maxRows, total = 3 * per;
    ProjGuard pg;
    int n = 0, failAlloc = 0, failVt = 0, failTpl = 0, failOther = 0, failAppend = 0;
    uint32_t badVt = 0;
    int nCont = 0;
    for (int g = 0; g < 3; ++g) {   // pill containers first: the panel list is the draw order, so they sit behind every button label
        int why = guikit::CF_None; uint32_t bv = 0;
        uintptr_t c = guikit::createLabelFromTemplate(panel, "LBL_QUEUE1", &why, &bv);
        if (!c) { KP_LOGV(chMain, KPLOG_WARN, "BUILD: container g%d create failed (why=%d vtable=%08x)", g, why, (unsigned)bv); continue; }
        guikit::setVisible(c, false);
        guikit::setClickable(c, false);
        if (!guikit::appendToPanel(panel, c)) { KP_LOGV(chMain, KPLOG_WARN, "BUILD: container g%d append failed, control %08x left hidden + click-through", g, (unsigned)c); continue; }
        g_cont[g].c = c; g_cont[g].vis = 0;
        ++nCont;
    }
    KP_LOGV(chMain, nCont == 3 ? KPLOG_INFO : KPLOG_WARN, "BUILD: containers=%d/3", nCont);
    for (int i = 0; i < total && i < MaxBtn; ++i) {
        uintptr_t cs[2] = { 0, 0 };
        bool okSlot = true;
        for (int k = 0; k < 2 && okSlot; ++k) {   // 0 = bg, 1 = icon
            int why = guikit::CF_None; uint32_t bv = 0;
            uintptr_t c = guikit::createLabelFromTemplate(panel, "LBL_QUEUE1", &why, &bv);
            if (!c) {
                switch (why) { case guikit::CF_Alloc: ++failAlloc; break; case guikit::CF_Vtable: ++failVt; badVt = bv; break; case guikit::CF_Template: ++failTpl; break; default: ++failOther; break; }
                okSlot = false; break;
            }
            guikit::setVisible(c, false);
            guikit::setClickable(c, false);
            if (!guikit::appendToPanel(panel, c)) { ++failAppend; KP_LOGV(chMain, KPLOG_WARN, "BUILD: append failed, control %08x left hidden + click-through", (unsigned)c); okSlot = false; break; }
            cs[k] = c;
        }
        if (!okSlot) continue;   // slot stays unused (a half-built slot's appended bg stays hidden + click-through)
        g_btn[i].bg.c = cs[0]; g_btn[i].bg.vis = 0;
        g_btn[i].icon.c = cs[1]; g_btn[i].icon.vis = 0;
        ++n;
    }
    g_nBtn = total < MaxBtn ? total : MaxBtn; g_per = per;
    if (n == total) KP_LOGV(chMain, KPLOG_INFO, "BUILD buttons created=%d/%d slots x2 labels (panel=%08x cols=%d rows=%d)", n, total, (unsigned)panel, g_cfg.cols, g_cfg.maxRows);
    else KP_LOGV(chMain, KPLOG_WARN, "BUILD buttons created=%d/%d alloc=%d vtable=%d(%08x) template=%d append=%d other=%d", n, total, failAlloc, failVt, (unsigned)badVt, failTpl, failAppend, failOther);
}

void hideAllButtons() {
    for (Wid& w : g_cont) widHide(w);
    for (int i = 0; i < g_nBtn; ++i) {
        Btn& b = g_btn[i];
        if (!b.bg.c) continue;
        widHide(b.bg); widHide(b.icon);
        b.tipSet = false;
    }
    g_haveLast = false; g_pillMask = -1; g_freeze = { false, 0, 0 }; g_frozenLog = false; g_hover = -1; g_logObj = 0; g_logType = -2;
    memset(g_lastRect, 0, sizeof(g_lastRect)); memset(g_lastCont, 0, sizeof(g_lastCont)); memset(g_lastN, 0, sizeof(g_lastN)); memset(g_lastSig, 0, sizeof(g_lastSig));
}

bool anyButtonVisible() { for (int i = 0; i < g_nBtn; ++i) if (g_btn[i].bg.c && g_btn[i].bg.vis == 1) return true; return false; }

// Vanilla group controls: block g at menu+0x54+g*0x750, sub-controls BTN +0, LBL +0x1D0, UP +0x3A0, DOWN +0x570; flags dword at control +0x48 (bit1 = visible + hit-testable).
constexpr uintptr_t GroupBase = 0x54, GroupStride = 0x750, CtlFlags = 0x48;
constexpr uintptr_t VanillaOffs[4] = { 0, 0x1D0, 0x3A0, 0x570 };

// Replace=1: clear bit1 on all four sub-controls of the three vanilla groups (the engine rewrites the arrow bits every populate, so every POST).
void hideVanilla(uintptr_t menu) {
    bool ok = true;
    for (int g = 0; g < 3; ++g)
        for (int k = 0; k < 4; ++k) {
            uint32_t* p = reinterpret_cast<uint32_t*>(menu + GroupBase + (uintptr_t)g * GroupStride + VanillaOffs[k] + CtlFlags);
            if (!saferead::probeWrite(S_VanWr, p, 4)) { ok = false; continue; }
            *p &= ~2u;
        }
    g_replaced = true;
    const int st = ok ? 1 : 0;
    if (st != g_replState) { g_replState = st; KP_LOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "buttons: vanilla group controls hide %s (menu=%08x)", ok ? "applied" : "FAILED (not writable)", (unsigned)menu); }
}

// Replace turned off after it was applied: give the vanilla box (BTN) and label back once; the engine owns the arrows again.
void restoreVanilla(uintptr_t menu) {
    for (int g = 0; g < 3; ++g)
        for (int k = 0; k < 2; ++k) {
            uint32_t* p = reinterpret_cast<uint32_t*>(menu + GroupBase + (uintptr_t)g * GroupStride + VanillaOffs[k] + CtlFlags);
            if (saferead::probeWrite(S_VanWr, p, 4)) *p |= 2u;
        }
    g_replaced = false; g_replState = -1;
    KP_LOGV(chMain, KPLOG_INFO, "buttons: vanilla group boxes restored (menu=%08x)", (unsigned)menu);
}

void updateButtons(const K2HudContext* ctx) {
    const uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    const bool replace = g_cfg.btnEnabled && g_cfg.replace;
    if (!replace && g_replaced) restoreVanilla(menu);
    if (!g_cfg.btnEnabled || g_nBtn == 0 || g_per == 0) return;
    const uintptr_t hud = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->hud));
    const int8_t type = static_cast<int8_t>(ctx->targetType);
    // Click edge is sampled every frame so a press that started while the buttons were hidden never fires when they appear.
    const bool click = guikit::clickEdge(g_click);
    if (!ctx->target || type < 0 || type > 3) { hideAllButtons(); return; }

    if (replace) hideVanilla(menu);

    uint8_t hoverFlag = 0;
    if (saferead::readAt(S_BtnRd, menu, 0x1BAC, &hoverFlag) && (int)hoverFlag != g_flagLast) { CH_VLOG(chTarget, KPLOG_DEBUG, "menu+0x1BAC = %02x (bit1=%d)", (unsigned)hoverFlag, (hoverFlag >> 1) & 1); g_flagLast = hoverFlag; }

    int sw = 0, sh = 0;
    if (guikit::g_gk.screenWAddr) saferead::readAt(S_BtnRd, guikit::g_gk.screenWAddr, 0, &sw);
    if (guikit::g_gk.screenHAddr) saferead::readAt(S_BtnRd, guikit::g_gk.screenHAddr, 0, &sh);
    int ox = 0, oy = 0, ow = 0, oh = 0;
    bool ok = sw >= 320 && sh >= 200 && saferead::readAt(S_BtnRd, menu, 0x1648, &ox) && saferead::readAt(S_BtnRd, menu, 0x164C, &oy) &&
              saferead::readAt(S_BtnRd, menu, 0x1650, &ow) && saferead::readAt(S_BtnRd, menu, 0x1654, &oh);

    ch::GroupView gv[3]; uintptr_t base[3] = { 0, 0, 0 };
    readGroups(menu, gv, base);
    const int per = g_per;
    // CH5 names: re-read per group only when its identity signature (count + keys) changes.
    for (int g = 0; g < 3; ++g) {
        int idn[16]; const int c = gv[g].count < 0 ? 0 : (gv[g].count > 16 ? 16 : gv[g].count);
        for (int i = 0; i < c; ++i) idn[i] = i;
        const uint32_t ns = ch::orderSig(gv[g], idn, c);
        if (g_nameValid[g] && ns == g_nameSig[g]) continue;
        for (int i = 0; i < 16; ++i) {
            g_names[g][i][0] = 0;
            if (i < c && saferead::readExo(S_BtnRd, base[g] + (uintptr_t)i * EntryStride, g_names[g][i], sizeof(g_names[g][i])) <= 0) g_names[g][i][0] = 0;
        }
        g_nameSig[g] = ns; g_nameValid[g] = true;
        char nb[512]; int no = 0;
        nb[0] = 0;
        for (int i = 0; i < c && no < (int)sizeof(nb) - 80; ++i) no += snprintf(nb + no, sizeof(nb) - no, "%s%s", i ? " | " : "", g_names[g][i]);
        CH_VLOG(chTarget, KPLOG_DEBUG, "names g%d: %s", g, nb);
    }
    int ord[3][16]; int nShow[3];
    uint32_t sig[3];
    int pre[3];   // displayed counts = what layoutBannerPills (HideEmpty=1) or layoutBannerGroups (HideEmpty=0) will report in outN
    for (int g = 0; g < 3; ++g) {
        nShow[g] = ch::orderGroup(gv[g], g_names[g], g_prof, g, ord[g], per > 16 ? 16 : per);
        if (nShow[g] < 0) nShow[g] = 0;
        pre[g] = nShow[g];
        sig[g] = ch::orderSig(gv[g], ord[g], nShow[g]);
    }

    // Hover freeze: while the cursor is over last frame's pill containers (and the counts did not change) the menu origin is held, so the whole banner and
    // the buttons under the cursor stop following the target. Written back before the engine draws (POST runs at the PopulateMenus tail).
    const uintptr_t obj = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->target));
    const bool retarget = obj != g_logObj || type != g_logType;
    int cx = 0, cy = 0;
    const bool haveCursor = guikit::cursorEngine(&cx, &cy);
    bool inGrid = false;
    if (ok && !retarget && g_haveLast && haveCursor && pre[0] == g_lastN[0] && pre[1] == g_lastN[1] && pre[2] == g_lastN[2] &&
        sig[0] == g_lastSig[0] && sig[1] == g_lastSig[1] && sig[2] == g_lastSig[2]) {
        // Union only the visible pills: a hidden pill's zero rect would stretch the union to the screen origin.
        ch::Rect gb = { 0, 0, 0, 0 };
        bool anyPill = false;
        for (int g = 0; g < 3; ++g) {
            if (g_lastCont[g].w <= 0 || g_lastCont[g].h <= 0) continue;
            gb = anyPill ? unionRect(gb, g_lastCont[g]) : g_lastCont[g];
            anyPill = true;
        }
        inGrid = anyPill && gb.w > 0 && gb.h > 0 && cx >= gb.x && cx < gb.x + gb.w && cy >= gb.y && cy < gb.y + gb.h;
    }
    int fx = ox, fy = oy;
    bool frozen = false;
    if (ok) {
        frozen = ch::freezeOrigin(g_freeze, inGrid, ox, oy, &fx, &fy);
        if (frozen && (fx != ox || fy != oy)) {
            int* px = reinterpret_cast<int*>(menu + 0x1648); int* py = reinterpret_cast<int*>(menu + 0x164C);
            if (saferead::probeWrite(S_OrgWr, px, 4) && saferead::probeWrite(S_OrgWr, py, 4)) { *px = fx; *py = fy; }
            else { fx = ox; fy = oy; KP_LOGV(chTarget, KPLOG_WARN, "buttons: freeze origin write failed (menu=%08x), following the live origin", (unsigned)menu); }
        }
        if (frozen != g_frozenLog) { g_frozenLog = frozen; CH_VLOG(chTarget, KPLOG_DEBUG, "buttons: hover freeze %s (cursor %d,%d origin %d,%d live %d,%d)", frozen ? "on" : "off", cx, cy, fx, fy, ox, oy); }
    }

    // Screen rects: banner = LBL_NAME + PB_HEALTH, boxesUnion = the three vanilla BTN extents (menu-local extent + origin).
    ch::Rect banner = { 0, 0, 0, 0 }, boxes[3], boxesUnion = { 0, 0, 0, 0 };
    for (int g = 0; ok && g < 3; ++g) {
        int ex = 0, ey = 0, ew = 0, eh = 0;
        if (!guikit::getExtent(menu + GroupBase + (uintptr_t)g * GroupStride, &ex, &ey, &ew, &eh)) { ok = false; break; }
        boxes[g].x = fx + ex; boxes[g].y = fy + ey; boxes[g].w = ew; boxes[g].h = eh;
        boxesUnion = g == 0 ? boxes[0] : unionRect(boxesUnion, boxes[g]);
    }
    if (ok) {
        int nx = 0, ny = 0, nw = 0, nh = 0, hx = 0, hy = 0, hw = 0, hh = 0;
        if (!guikit::getExtent(menu + 0x1668, &nx, &ny, &nw, &nh) || !guikit::getExtent(menu + 0x1A40, &hx, &hy, &hw, &hh)) ok = false;
        else { const ch::Rect a = { fx + nx, fy + ny, nw, nh }, b = { fx + hx, fy + hy, hw, hh }; banner = unionRect(a, b); }
    }
    if (!ok) { hideAllButtons(); return; }

    ch::Rect rects[MaxBtn]; int outN[3] = { 0, 0, 0 }; ch::Rect cont[3];
    const int btnPx = g_cfg.btnPx > 0 ? g_cfg.btnPx : boxes[0].w / 2;
    if (g_cfg.hideEmpty) ch::layoutBannerPills(banner, boxesUnion, pre, per, g_cfg.colsTotal, btnPx, g_cfg.pad, g_cfg.groupGap, g_cfg.capPad, g_cfg.sidePad, sw, sh, rects, outN, cont);
    else ch::layoutBannerGroups(banner, boxesUnion, pre, g_cfg.cols, g_cfg.maxRows, btnPx, g_cfg.pad, g_cfg.groupGap, g_cfg.capPad, g_cfg.sidePad, sw, sh, rects, outN, cont);
    // Reflow: the counts changed since last frame, so the buttons moved under a possibly stationary cursor; hover sound and click are ignored this frame.
    const bool reflow = g_haveLast && (pre[0] != g_lastN[0] || pre[1] != g_lastN[1] || pre[2] != g_lastN[2] || sig[0] != g_lastSig[0] || sig[1] != g_lastSig[1] || sig[2] != g_lastSig[2]);
    memcpy(g_lastRect, rects, sizeof(ch::Rect) * 3 * per);
    memcpy(g_lastCont, cont, sizeof(cont));
    memcpy(g_lastN, outN, sizeof(outN)); memcpy(g_lastSig, sig, sizeof(sig)); g_haveLast = true;

    int mask = 0, firstVis = -1;
    for (int g = 0; g < 3; ++g) if (cont[g].w > 0 && cont[g].h > 0) { mask |= 1 << g; if (firstVis < 0) firstVis = g; }
    if (mask != g_pillMask) {
        int nVis = 0, maxN = 0, rowsU = 0, colsU = 0, usedCols = 0, pw = 0, ph = 0, bw = 0;
        if (firstVis >= 0) {
            for (int g = 0; g < 3; ++g) if (mask & (1 << g)) { ++nVis; if (outN[g] > maxN) maxN = outN[g]; }
            colsU = g_cfg.hideEmpty ? clampInt(g_cfg.colsTotal / nVis, 1, per) : g_cfg.cols;
            usedCols = colsU < maxN ? colsU : maxN;
            for (int g = 0; g < 3; ++g) if (mask & (1 << g)) { const int r = (outN[g] + colsU - 1) / colsU; if (r > rowsU) rowsU = r; }
            pw = cont[firstVis].w; ph = cont[firstVis].h; bw = rects[firstVis * per].w;
        }
        KP_LOGV(chTarget, KPLOG_INFO, "pills: visible=%d%d%d cols=%d usedCols=%d rows=%d pill=[%d,%d] btn=%d", mask & 1, (mask >> 1) & 1, (mask >> 2) & 1, colsU, usedCols, rowsU, pw, ph, bw);
        g_pillMask = mask;
    }

    if (retarget) {
        g_logObj = obj; g_logType = type;
        int firstW = 0;
        for (int g = 0; g < 3 && !firstW; ++g) if (outN[g] > 0) firstW = rects[g * per].w;
        CH_VLOG(chTarget, KPLOG_INFO, "buttons: first draw obj=%p type=%d origin=[%d,%d,%d,%d]%s banner=[%d,%d,%d,%d] boxesUnion=[%d,%d,%d,%d] outN=[%d,%d,%d] first=[%d,%d,%d,%d][%d,%d,%d,%d][%d,%d,%d,%d] cont=[%d,%d,%d,%d][%d,%d,%d,%d][%d,%d,%d,%d] btn=%d screen=%dx%d",
                (void*)obj, (int)type, fx, fy, ow, oh, frozen ? " frozen" : "",
                banner.x, banner.y, banner.w, banner.h, boxesUnion.x, boxesUnion.y, boxesUnion.w, boxesUnion.h,
                outN[0], outN[1], outN[2],
                rects[0].x, rects[0].y, rects[0].w, rects[0].h, rects[per].x, rects[per].y, rects[per].w, rects[per].h, rects[2 * per].x, rects[2 * per].y, rects[2 * per].w, rects[2 * per].h,
                cont[0].x, cont[0].y, cont[0].w, cont[0].h, cont[1].x, cont[1].y, cont[1].w, cont[1].h, cont[2].x, cont[2].y, cont[2].w, cont[2].h,
                firstW, sw, sh);
    }

    // Art (cached once resolved).
    if (!g_imgFrame) g_imgFrame = guikit::imageFor(FrameResref);
    if (!g_imgHi) g_imgHi = guikit::imageFor(HiResref);
    if ((!g_imgFrame || !g_imgHi) && !g_imgWarned) { g_imgWarned = true; KP_LOGV(chMain, KPLOG_WARN, "buttons: art not resolved yet (frame=%p hi=%p)", g_imgFrame, g_imgHi); }
    else if (g_imgFrame && g_imgHi && g_imgWarned) { g_imgWarned = false; KP_LOGV(chMain, KPLOG_INFO, "buttons: art resolved (frame=%p hi=%p)", g_imgFrame, g_imgHi); }

    // Hover: the visible button under the cursor (unused slots have zeroed rects, which never hit).
    int over = -1;
    if (haveCursor) {
        const int hit = ch::hitSlot(rects, 3 * per, cx, cy);
        int hg = -1;
        const int hidx = ch::slotEntry(hit, g_cfg.cols, g_cfg.maxRows, &hg);
        if (hidx >= 0 && hg >= 0 && hidx < outN[hg] && hit < g_nBtn && g_btn[hit].bg.c) over = hit;
    }
    if (over != g_hover) {
        const int prev = g_hover;
        g_hover = over;
        CH_VLOG(chTarget, KPLOG_DEBUG, "buttons: hover slot %d -> %d cursor=(%d,%d)", prev, over, cx, cy);
        if (over >= 0) {
            int hg = -1;
            const int hidx = ch::slotEntry(over, g_cfg.cols, g_cfg.maxRows, &hg);
            if (hidx >= 0 && hg >= 0 && hidx < nShow[hg] && gv[hg].e[ord[hg][hidx]].usable && !reflow) playGuiSound(1);
        }
    }

    // Per group: copy the vanilla BTN_TARGETn tint + alpha (a failed read keeps the defaults), then show the pill behind the buttons.
    // The pill is clickable (swallows gap clicks) but has no action: the click handler below only acts on button hits.
    bool tOk[3], aOk[3]; float tint[3][3], alp[3];
    for (int g = 0; g < 3; ++g) {
        tint[g][0] = 0.85f; tint[g][1] = 0.15f; tint[g][2] = 0.15f;
        alp[g] = 0.95f;
        const uintptr_t btnCtl = menu + GroupBase + (uintptr_t)g * GroupStride;
        // Sanity gates (the hidden-BTN tint is UNVERIFIED): pure black or a near-zero alpha would make the pills vanish, so keep the defaults instead.
        tOk[g] = guikit::getBorderTint(btnCtl, tint[g]) && (tint[g][0] > 0.01f || tint[g][1] > 0.01f || tint[g][2] > 0.01f);
        aOk[g] = guikit::getAlpha(btnCtl, &alp[g]) && alp[g] >= 0.05f && alp[g] <= 1.0f;
        // Force combat red theme for container & button border tint (matching native combat UI)
        tint[g][0] = 0.85f; tint[g][1] = 0.15f; tint[g][2] = 0.15f;
        tOk[g] = true;
        if (!aOk[g]) { alp[g] = 0.95f; aOk[g] = true; }
        if (!tOk[g]) { if (g_contState[g] != 0) { g_contState[g] = 0; CH_VLOG(chTarget, KPLOG_INFO, "container g%d tint read failed or rejected (black)", g); } }
        else if (g_contState[g] != 1 || g_contLog[g][0] != tint[g][0] || g_contLog[g][1] != tint[g][1] || g_contLog[g][2] != tint[g][2] || (aOk[g] && g_contLog[g][3] != alp[g])) {
            g_contState[g] = 1;
            for (int k = 0; k < 3; ++k) g_contLog[g][k] = tint[g][k];
            if (aOk[g]) g_contLog[g][3] = alp[g];
            CH_VLOG(chTarget, KPLOG_INFO, "container g%d tint=(%.2f,%.2f,%.2f) alpha=%.2f", g, tint[g][0], tint[g][1], tint[g][2], aOk[g] ? alp[g] : -1.f);
        }
        Wid& c = g_cont[g];
        if (!c.c) continue;
        if (!replace) { widHide(c); continue; }   // Replace=0: the vanilla pills are still drawn, never stack ours on them
        if (cont[g].w == 0 || cont[g].h == 0) { widHide(c); continue; }   // CH4e: empty group, no pill
        widRect(c, cont[g]);
        // Use g_imgHi (cream border in lbl_miscroll_hi) so red tint produces vibrant red (lbl_miscroll_f has teal baked in)
        void* contImg = g_imgHi ? g_imgHi : g_imgFrame;
        if (contImg) widFill(c, contImg);
        if (tOk[g]) widTint(c, tint[g]);
        if (aOk[g]) widAlpha(c, alp[g]);
        widShow(c, true);
    }

    for (int g = 0; g < 3; ++g) {
        for (int k = 0; k < per; ++k) {
            const int s = g * per + k;
            if (s >= g_nBtn) break;
            Btn& b = g_btn[s];
            if (!b.bg.c) continue;
            if (k >= outN[g]) {
                if (b.bg.vis != 0 || b.icon.vis != 0) { widHide(b.bg); widHide(b.icon); b.tipSet = false; }
                continue;
            }
            const ch::EntryView& e = gv[g].e[ord[g][k]];
            const ch::Rect& r = rects[s];
            int in = g_cfg.inset;
            if (r.w - 2 * in < 1 || r.h - 2 * in < 1) in = 0;
            const ch::Rect ir = { r.x + in, r.y + in, r.w - 2 * in < 1 ? 1 : r.w - 2 * in, r.h - 2 * in < 1 ? 1 : r.h - 2 * in };
            widRect(b.bg, r); widRect(b.icon, ir);
            void* want = g_imgHi ? g_imgHi : g_imgFrame;
            if (want) widFill(b.bg, want);
            if (!b.iconSet || strncmp(b.iconName, e.icon, 16) != 0) {
                widFill(b.icon, e.icon[0] ? guikit::imageFor(e.icon) : nullptr);
                memcpy(b.iconName, e.icon, 16); b.iconName[16] = 0; b.iconSet = true;
            }
            float bTint[3] = { tint[g][0], tint[g][1], tint[g][2] };
            if (s == g_hover) { bTint[0] = 1.0f; bTint[1] = 0.35f; bTint[2] = 0.35f; }
            widTint(b.bg, bTint);
            widAlpha(b.bg, (s == g_hover) ? 1.0f : (aOk[g] ? alp[g] : 0.85f));
            widAlpha(b.icon, e.usable ? 1.0f : 0.35f);
            widShow(b.bg, true); widShow(b.icon, false);

            if (!b.tipSet || b.tipKey != e.key) {
                const char* name = g_names[g][ord[g][k]];
                if (name[0]) guitip::setNative(b.bg.c, name);
                b.tipKey = e.key; b.tipSet = true;
            }
        }
    }

    if (click && reflow) CH_VLOG(chTarget, KPLOG_DEBUG, "click ignored: layout reflow");
    else if (click && anyButtonVisible() && haveCursor && over >= 0) {
        int g = -1;
        const int idx = ch::slotEntry(over, g_cfg.cols, g_cfg.maxRows, &g);
        if (idx >= 0 && g >= 0 && idx < outN[g] && idx < nShow[g] && g_btn[over].bg.vis == 1 && hud) {
            const int src = ord[g][idx];
            KP_LOGV(chTarget, KPLOG_INFO, "click g=%d idx=%d src=%d key=%08x", g, idx, src, (unsigned)gv[g].e[src].key);
            if (invokeEntry(hud, menu, g, type, src, gv[g].e[src])) playGuiSound(g_cfg.clickSound);
        }
    }
}

// CH3b: clear bit1 (visible) of the 6 carousel arrow controls after the stock population; log only when the outcome changes. Then CH4c buttons.
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
    if (filewatch::changed(g_iniPoll, IniPath, GetTickCount())) {
        const bool oe = g_cfg.btnEnabled; const int oc = g_cfg.cols, om = g_cfg.maxRows;
        loadIni(); loadProfile();
        const bool differs = oe != g_cfg.btnEnabled || oc != g_cfg.cols || om != g_cfg.maxRows;
        g_cfg.btnEnabled = oe; g_cfg.cols = oc; g_cfg.maxRows = om;
        KP_LOGV(chMain, KPLOG_INFO, "config: reloaded");
        if (differs) KP_LOGV(chMain, KPLOG_INFO, "config: cols/maxRows/enabled change takes effect after reload");
        g_pillMask = -1;
        for (int g = 0; g < 3; ++g) g_nameValid[g] = false;
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

        CH_VLOG(chTarget, KPLOG_DEBUG, "  g%d[%d]: key=%08x usable=%d fn=%08x owner=%08x icon=%.16s name=%s",
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
            CH_VLOG(chTarget, KPLOG_INFO, "target_menu: obj=%p leader=%p forceUnusable=%d state1644=%d type=%s counts=[%d,%d,%d] sel=[%08x,%08x,%08x]",
                   (void*)targetObj, (void*)leader, forceUnusable, menuState, typeStr,
                   counts[0], counts[1], counts[2],
                   (unsigned)selKeys[0], (unsigned)selKeys[1], (unsigned)selKeys[2]);

            if (g_cfg.verbose) {
                for (int g = 0; g < 3; ++g) {
                    logTargetGroup(menu, g, counts[g]);
                }
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
    loadProfile();
    filewatch::accept(g_iniPoll, IniPath);
    g_hud.add(K2HUD_TARGET_MENU_PRE, 100, onTargetMenu);
    g_hud.add(K2HUD_TARGET_MENU_POST, 100, onTargetMenuPost);
    g_hud.add(K2HUD_BUILD, 300, onHudBuild);
    g_hud.connect();
}

} // extern "C"
