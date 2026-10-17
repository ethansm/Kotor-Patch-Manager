// ModConfig.cpp -- mod-config 0.1.0-dev (Steam build): the "Mod Configuration" provider on the in-game Options screen (doc 42,
// plan .claude/plans/holster_editor_options.md, card c18). Provides two interfaces declared in ModConfigApi.h:
//   k2.modconfig@1  K2ModConfigApi  Register / Unregister: a mod adds one entry (name, description, build/enter/leave/tick/destroy).
//   k2.menuui@1     K2MenuUiApi     NewPage / AddButton / AddLabel / AddCustom / Clicked / SetText / SetRect / SetDesc / PaneRect / Control.
//
// The engine code is lifted from the PRB1-proven holster-editor (HolsterEditor.cpp, button clones, column reflow, release-edge clicks,
// the description path, the r7 free recipe) and generalised from "one entry + Back" to a page model driven by MenuUiLogic.h (namespace mui):
//   MAIN   vanilla Options + one "Mod Configuration" button under Gameplay (reflow: the later rows move down one pitch)
//   LIST   vanilla controls hidden; one button per registered entry on the column rects, hover shows the entry description
//   ENTRY  the entry's own page (items created by its build() through k2.menuui)
// Page ids: 0 MAIN, 1 LIST, 2 + k ENTRY(k), k = registry token - 1. Page changes fire on the LMB RELEASE edge (mui::ClickTracker).
//
// Hooks (kotor2-steam-aspyr.hooks.toml; handlers are plain extern "C" __cdecl functions, run BEFORE the stolen bytes):
//   McBind     8A16FE, inside the Options ctor with the layout still open (panel = [ebp-0xAC]): clones the controls, then runs every
//              entry's build().
//   McTick     CSWGuiPanel::Draw 40EC90, every panel every frame: returns at once unless [ECX] is the Options vtable.
//   McDestroy  8A1930, the Options scalar deleting dtor, on entry: leave callback, clear engine references, truncate the list, entry
//              destroy() callbacks, free our controls (never a MUI_NOFREE one).
// Esc is never intercepted (it closes the whole in-game GUI). Reopening the Options screen starts at MAIN.
//
// Failure values (documented here because the header leaves them open): NewPage returns 0 (page 0 is MAIN, never an entry page);
// AddButton / AddLabel / AddCustom return -1. One entry owns one page: a second NewPage in the same build() returns the same page id
// (the title is updated). Items can only be added to the page of the entry whose build() is running.
// Probe lines (log only): CONFIG, CTOR, BTNCLONE, LABEL, APPEND, TEMPLATE, REFLOW, RELAYOUT, HOVER, DESC, INPUTCLASS, PAGE, BUILD,
// REGISTER, MUI, FREE, DTOR, HIDDENDRAW, PROVIDE.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <new>
#include "SafeRead.h"
#include "KpLog.h"
#include "K2Addr.h"
#include "GameAddr.h"
#include "GuiKit.h"
#include "GuiButton.h"
#include "MenuLayout.h"
#include "ModConfigApi.h"
#include "MenuUiLogic.h"

KP_LOG_PATCH("mod-config");

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chInput, "input");

const bool g_fallbackSet = (KpLog_SetFallbackFile("mod-config_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

namespace {

using menulayout::Rect;

constexpr uint32_t kOptionsVtable = static_cast<uint32_t>(gameaddr::OptionsInGameVtable);
constexpr uint32_t kLabelVtable = static_cast<uint32_t>(gameaddr::LabelVtable);
constexpr uint32_t kButtonVtable = static_cast<uint32_t>(gameaddr::ButtonVtable);
// Vanilla optionsingame_p list indices (ID == index, d2c section 1).
constexpr int kIdxQuit = 2, kIdxTitle = 3, kIdxExit = 13, kIdxDesc = 15;
constexpr int kIdxBarFirst = 8, kIdxBarLast = 12;   // LBL_BAR1..5: the horizontal frame bars; they stay visible on LIST / ENTRY
constexpr int kBarFirst = 8, kBarLast = 12;                                     // the five slider bars (view/status floor only)
constexpr int kColumnN = 8, kInsertAfter = 2;                                   // column positions top -> bottom, insert after GAMEPLAY
constexpr int kColumnIdx[kColumnN] = {0, 14, 1, 4, 5, 6, 7, 2};                 // SAVEGAME LOADGAME GAMEPLAY FEEDBACK AUTOPAUSE GRAPHICS SOUND QUIT
constexpr int kMovedN = kColumnN - kInsertAfter - 1;                            // rows that move down: positions 3..7
constexpr int kMaxVanilla = 64;
constexpr int kMaxTargets = 32;                                                 // 1 new row + 5 moved + 8 list + Back + title = 16, with headroom
constexpr int kMaxReapply = 8;                                                  // per target, then give up (a SetRect the engine keeps rewriting)
constexpr DWORD kTickGapMs = 500;    // longer than this between two Options draws = the panel was not on screen: back to MAIN
constexpr DWORD kLeaveDelayMs = 100; // Back (LIST -> MAIN): restore the vanilla controls this long after the LMB is up
constexpr const char* kLabelTemplate = "LBL_TITLE";
constexpr const char* kRootText = "Mod Configuration";
constexpr const char* kRootDesc = "Settings for installed mods. Each mod that offers options is listed here.";
constexpr const char* kListBlurb = "Choose a mod to change its settings.";
constexpr int kMaxOwn = 1 + mui::kMaxEntries + 2 + mui::kMaxEntries * mui::kMaxItems;   // root + list + back + title + entry items
// Options panel offsets (static dump reads, doc r4/r7/r9): layout slot, manager, focus, description string object, text holder pointer, label.
constexpr int kPanelLayoutSlot = 0x30, kPanelMgr = 0x1C, kPanelFocus = 0x20, kDescStr = 0x1120, kDescHolderPtr = 0x1170, kDescLabel = 0x1030;
constexpr int kMgrHover = 0x08, kMgrCaptureFlag = 0x0C, kMgrCapture = 0x10, kMgrLastHit = 0x14;
constexpr int kVtSetActive = 0x08, kVtHover = 0x0C;
constexpr int kHolderObj = 0x14, kObjSetText = 0x18;

struct Config {
    bool buttonClone = true;   // clone CSWGuiButton templates (else label clones). A failed clone falls back to a label either way
    bool reflow = true;        // insert the root button after GAMEPLAY and move the later rows (else place it below QUIT)
    bool desc = true;          // write the hover description through the vanilla path
    bool freeLabels = true;    // free our labels and buttons in McDestroy (r7 recipe)
};
Config g_cfg;
bool g_enabled = true;
bool g_debug = false;

// ---------------------------------------------------------------------------------------------------- registry
mui::Registry g_reg;
K2ModConfigEntry g_ent[mui::kMaxEntries];   // indexed by slot = token - 1; live when g_reg.slots[slot].live

bool entLive(int slot) { return slot >= 0 && slot < mui::kMaxEntries && g_reg.slots[slot].live; }

// ---------------------------------------------------------------------------------------------------- state
struct Target {
    uintptr_t ctl = 0;
    Rect r = {0, 0, 0, 0};
    int applied = 0;
    bool gaveUp = false;
};

// One control we own or track: the root / list / Back / title clones and every entry page item.
struct ItemX {
    uintptr_t ctl = 0;
    bool isButton = false;     // a real CSWGuiButton (else a label)
    bool nofree = false;       // MUI_NOFREE: listed and shown/hidden with its page, never freed by us
    bool listed = false;       // appended to (or already in) the panel list
    bool unlinked = false;     // McDestroy removed it from the list: Control() answers nullptr
    bool latched = false;      // a release-edge click not yet read through Clicked()
    bool hasRect = false;
    bool hasText = false;
    Target tg;
    char text[96] = {};
    char desc[192] = {};
};

struct McState {
    uintptr_t panel = 0;
    int vanillaCount = 0;
    ItemX root, back, title;
    ItemX list[mui::kMaxEntries];
    int listTok[mui::kMaxEntries] = {};
    int nList = 0;
    bool built[mui::kMaxEntries] = {};
    bool pageMade[mui::kMaxEntries] = {};
    char pageTitle[mui::kMaxEntries][64] = {};
    mui::PageTable pt;
    ItemX items[mui::kMaxEntries][mui::kMaxItems];
    mui::PageStack ps;
    mui::HoverTracker hov;
    mui::ClickTracker clk, clkBack;
    bool appended = false;
    int savedFlags[kMaxVanilla] = {};
    bool haveSaved = false;
    bool restorePending = false;   // ENTER(MAIN) arrived while the LMB was down: restore the vanilla controls once it is up
    DWORD lastTick = 0;
    int lastFlags48 = -1;
    Target tg[kMaxTargets];
    int nTg = 0;
    int relayouts = 0;
    bool prevDown = false;
    DWORD leaveAt = 0;         // Back released on LIST: go to MAIN at this tick count (0 = none)
    Rect col[kColumnN] = {};   // vanilla column rects, measured BEFORE the reflow (PaneRect COLUMN)
    bool colOk = false;
};
McState g_mc;
bool g_inBuild = false;
int g_buildSlot = -1;

void resetState() { new (&g_mc) McState(); }

// ---------------------------------------------------------------------------------------------------- engine helpers (lifted)
template <typename T>
bool rd(uintptr_t base, int off, T* out) { return saferead::readAt<T>(0xE70100u, base, off, out); }

int ctlFlagsOff() { return k2addr::off(&K2AddrTable::Ctl_Flags); }

// Engine call shapes (thiscall = ECX is the object). Every call site is gated on a vtable / pointer read first.
using Call1Fn = void(__thiscall*)(void*, int);
using Call2Fn = void(__thiscall*)(void*, int, int);
using TextPushFn = void(__thiscall*)(void*, const char*);
using AssignFn = void*(__thiscall*)(void*, void*);
using FixFn = void(__fastcall*)(void*);

// Vanilla control i of the panel list (Panel_CtlArray {ptr, count, cap}), 0 when out of range or unreadable.
uintptr_t listCtl(uintptr_t panel, int i) {
    const int oArr = k2addr::off(&K2AddrTable::Panel_CtlArray), oCount = k2addr::off(&K2AddrTable::Panel_CtlCount);
    int count = 0;
    uint32_t arr = 0, c = 0;
    if (!oArr || !oCount || !rd<int>(panel, oCount, &count) || i < 0 || i >= count || !rd<uint32_t>(panel, oArr, &arr) || !arr) return 0;
    return rd<uint32_t>(arr, i * 4, &c) ? c : 0;
}

int ctlId(uintptr_t ctl) {
    int id = -999;
    const int oId = k2addr::off(&K2AddrTable::Ctl_Id);
    if (ctl && oId) (void)rd<int>(ctl, oId, &id);
    return id;
}

uintptr_t cloneLabel(uintptr_t panel, const char* what) {
    int why = 0;
    uint32_t badVt = 0;
    const uintptr_t c = guikit::createLabelFromTemplate(panel, kLabelTemplate, &why, &badVt);
    KP_LOG(chMain, "LABEL %s from %s -> %x why=%d badvt=%x", what, kLabelTemplate, static_cast<unsigned>(c), why, static_cast<unsigned>(badVt));
    return c;
}

// A real CSWGuiButton clone of `tag`; on any failure (or ini button_clone=0) the label clone instead. *isButton = which one you got.
uintptr_t cloneEntry(uintptr_t panel, const char* tag, const char* what, bool* isButton) {
    *isButton = false;
    if (g_cfg.buttonClone) {
        int why = 0;
        uint32_t badVt = 0;
        bool canary = false;
        const uintptr_t b = guibutton::createButtonFromTemplate(panel, tag, &why, &badVt, &canary);
        KP_LOG(chMain, "BTNCLONE %s from %s ctl=%x vt=%x why=%d canary=%d", what, tag, static_cast<unsigned>(b), static_cast<unsigned>(badVt), why,
               canary ? 1 : 0);
        if (b && canary) {
            KP_LOG(chMain, "BTNCLONE %s ok canary=1", what);
            *isButton = true;
            return b;
        }
        if (b) {   // constructed fine but the canary slack was overwritten: do not use it
            KP_LOG(chMain, "BTNCLONE %s canary overrun: releasing the button", what);
            guibutton::releaseUnlinkedButton(b);
        }
        KP_LOG(chMain, "BTNCLONE %s fallback label", what);
    } else {
        KP_LOG(chMain, "BTNCLONE %s skipped (button_clone=0): label", what);
    }
    return cloneLabel(panel, what);
}

bool setCtlText(uintptr_t ctl, bool isButton, const char* text) {
    if (!ctl) return false;
    const bool ok = isButton ? guibutton::setButtonText(ctl, text) : guikit::setText(ctl, text);
    if (!ok) KP_LOG(chMain, "TEXT set failed ctl=%x button=%d", static_cast<unsigned>(ctl), isButton ? 1 : 0);
    return ok;
}

bool iniBool(const char* v) { return !_stricmp(v, "1") || !_stricmp(v, "on") || !_stricmp(v, "true") || !_stricmp(v, "yes"); }

void readConfig() {
    FILE* f = fopen("mod_config.ini", "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof line, f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') ++p;
        if (*p == ';' || *p == '#' || *p == '[' || !*p) continue;
        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = 0;
        char* v = eq + 1;
        size_t n = strlen(v);
        while (n && (v[n - 1] == '\r' || v[n - 1] == '\n' || v[n - 1] == ' ' || v[n - 1] == '\t')) v[--n] = 0;
        char* k = p;
        n = strlen(k);
        while (n && (k[n - 1] == ' ' || k[n - 1] == '\t')) k[--n] = 0;
        const bool on = iniBool(v);
        if (!_stricmp(k, "enabled")) g_enabled = on;
        else if (!_stricmp(k, "debug")) g_debug = on;
        else if (!_stricmp(k, "button_clone")) g_cfg.buttonClone = on;
        else if (!_stricmp(k, "reflow")) g_cfg.reflow = on;
        else if (!_stricmp(k, "desc")) g_cfg.desc = on;
        else if (!_stricmp(k, "free_labels")) g_cfg.freeLabels = on;
    }
    fclose(f);
}

void logRect(const char* what, uintptr_t ctl) {
    int x = 0, y = 0, w = 0, h = 0;
    const bool ok = ctl && guikit::getExtent(ctl, &x, &y, &w, &h);
    KP_LOG(chMain, "RECT %s ctl=%x id=%d ok=%d (%d,%d,%d,%d) visible=%d", what, static_cast<unsigned>(ctl), ctlId(ctl), ok ? 1 : 0, x, y, w, h,
           ctl && guikit::isVisible(ctl) ? 1 : 0);
}

bool ctlRect(uintptr_t ctl, Rect* out) {
    int x = 0, y = 0, w = 0, h = 0;
    if (!ctl || !guikit::getExtent(ctl, &x, &y, &w, &h) || w <= 0 || h <= 0) return false;
    *out = Rect{x, y, w, h};
    return true;
}

// One drift check / write for a target. initial = write when different; else count re-applies and give up after kMaxReapply.
void applyOne(McState& e, Target& t, bool initial) {
    if (!t.ctl || t.gaveUp) return;
    int x = 0, y = 0, w = 0, h = 0;
    if (!guikit::getExtent(t.ctl, &x, &y, &w, &h)) return;
    if (x == t.r.x && y == t.r.y && w == t.r.w && h == t.r.h) return;
    if (!initial) {
        if (t.applied >= kMaxReapply) {
            t.gaveUp = true;
            KP_LOG(chMain, "RELAYOUT gave up on ctl=%x id=%d after %d re-applies (now %d,%d,%d,%d want %d,%d,%d,%d)", static_cast<unsigned>(t.ctl),
                   ctlId(t.ctl), t.applied, x, y, w, h, t.r.x, t.r.y, t.r.w, t.r.h);
            return;
        }
        ++t.applied;
        ++e.relayouts;
        if (e.relayouts <= 32)
            KP_LOG(chMain, "RELAYOUT n=%d ctl=%x id=%d was (%d,%d,%d,%d) -> (%d,%d,%d,%d)", e.relayouts, static_cast<unsigned>(t.ctl), ctlId(t.ctl), x, y, w,
                   h, t.r.x, t.r.y, t.r.w, t.r.h);
    }
    guikit::setRect(t.ctl, t.r.x, t.r.y, t.r.w, t.r.h);
}

// Fixed targets (moved vanilla rows, root, list, Back, title). A second target for the same control replaces the first.
void addTarget(McState& e, uintptr_t ctl, const Rect& r) {
    if (!ctl) return;
    for (int i = 0; i < e.nTg; ++i) {
        if (e.tg[i].ctl == ctl) {
            e.tg[i] = Target();
            e.tg[i].ctl = ctl;
            e.tg[i].r = r;
            return;
        }
    }
    if (e.nTg >= kMaxTargets) return;
    e.tg[e.nTg] = Target();
    e.tg[e.nTg].ctl = ctl;
    e.tg[e.nTg].r = r;
    ++e.nTg;
}

void applyTargets(McState& e, bool initial) {
    for (int i = 0; i < e.nTg; ++i) applyOne(e, e.tg[i], initial);
}

// SetActiveControl(0, 0) (40EC00 via vtable slot 2): drops the keyboard focus so Enter cannot open a hidden sub-panel.
bool clearFocus(uintptr_t P) {
    uint32_t vt = 0, fn = 0;
    if (!rd<uint32_t>(P, 0, &vt) || vt != kOptionsVtable || !rd<uint32_t>(vt, kVtSetActive, &fn) || !fn) return false;
    reinterpret_cast<Call2Fn>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(P), 0, 0);
    return true;
}

// Description path. The vanilla function 8A2320 takes a TLK STRREF (416EA0 stores it at P+0x1120+8 and resolves it through the TLK table),
// not a string, so a custom text cannot be passed to it. Instead: store our text into the CExoString at P+0x1120 (7337C0), do what the
// strref branch of 416EA0 does afterwards (push the C string to the text object [[P+0x1170]+0x14] vt[0x18], then 417800), then call
// 8A2320(P, -1): strref -1 skips the TLK branch, every later step (temp copy, list clear, rect, relayout, scroll flag) runs unchanged.
bool describe(uintptr_t P, const char* text, const char** why) {
    *why = "";
    uint32_t vt = 0, lvt = 0;
    if (!guibutton::steamBuild() || !guikit::g_gk.exoCtor || !guikit::g_gk.exoDtor) { *why = "no-engine"; return false; }
    if (!rd<uint32_t>(P, 0, &vt) || vt != kOptionsVtable) { *why = "panel-vt"; return false; }
    if (!rd<uint32_t>(P, kDescLabel, &lvt) || lvt != kLabelVtable) { *why = "label-vt"; return false; }
    char exo[16] = {0};
    guikit::g_gk.exoCtor(exo, text);
    reinterpret_cast<AssignFn>(gameaddr::ExoStringAssign)(reinterpret_cast<void*>(P + kDescStr), exo);
    guikit::g_gk.exoDtor(exo);
    uint32_t cstr = 0;
    if (!rd<uint32_t>(P, kDescStr, &cstr) || !cstr) { *why = "assign"; return false; }
    uint32_t holder = 0, obj = 0, ovt = 0, fn = 0;
    int pushed = 0;
    if (rd<uint32_t>(P, kDescHolderPtr, &holder) && holder && rd<uint32_t>(holder, kHolderObj, &obj) && obj && rd<uint32_t>(obj, 0, &ovt) && ovt &&
        rd<uint32_t>(ovt, kObjSetText, &fn) && fn) {
        reinterpret_cast<TextPushFn>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(obj), reinterpret_cast<const char*>(static_cast<uintptr_t>(cstr)));
        reinterpret_cast<FixFn>(gameaddr::GuiTextObjRefresh)(reinterpret_cast<void*>(holder));
        pushed = 1;
    }
    reinterpret_cast<Call1Fn>(gameaddr::OptionsSetDescription)(reinterpret_cast<void*>(P), -1);
    static volatile LONG s_d = 0;
    if (InterlockedIncrement(&s_d) <= 16) KP_LOG(chMain, "DESC ok via=8A2320 strref=-1 holder=%x pushed=%d", static_cast<unsigned>(holder), pushed);
    return true;
}

// Writes the right-hand description pane (ini desc=0 turns it off).
bool writeDesc(const char* text) {
    if (!g_mc.panel || !text) return false;
    if (!g_cfg.desc) {
        static volatile LONG s_off = 0;
        if (InterlockedIncrement(&s_off) <= 1) KP_LOG(chMain, "DESC skipped (desc=0)");
        return false;
    }
    const char* why = "";
    if (!describe(g_mc.panel, text, &why)) {
        KP_LOG(chMain, "DESC no-refresh why=%s", why);
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------- pane geometry (PaneRect)
// The 3D view sits LEFT of the description pane (LB_DESC), so the title and status labels do not share its rect. Layout units are the
// vanilla 800x600 GUI space: LB_DESC is 232 x 336 and the view is 216 x 336, 12 units left of it. Everything scales by s = desc.h / 336
// (integer math, rounded to nearest): w = 216 s, gap = 12 s, x = desc.x - gap - w, y = desc.y, h = desc.h. A desc rect with no area
// gives the zero rect. (Same formula as holster-editor's viewRect.)
constexpr int kViewRefW = 216, kViewRefH = 336, kViewRefGap = 12;
Rect paneViewRect(const Rect& desc) {
    Rect r = {0, 0, 0, 0};
    if (desc.w <= 0 || desc.h <= 0) return r;
    const int w = (kViewRefW * desc.h + kViewRefH / 2) / kViewRefH;
    const int gap = (kViewRefGap * desc.h + kViewRefH / 2) / kViewRefH;
    r.x = desc.x - gap - w;
    r.y = desc.y;
    r.w = w;
    r.h = desc.h;
    return r;
}

// Floor for menulayout::statusStrip: the top y of the nearest slider bar below the button column (strictly below quitBottom) that leaves
// at least 16 px under the view; else the nearest bar at least 16 px under the view; else viewBottom + 34 (a full 28 px strip).
int paneStatusFloor(const int* barY, int n, int quitBottom, int viewBottom) {
    const int kRoom = 16;
    int best = 0;
    bool have = false;
    for (int i = 0; barY && i < n; ++i)
        if (barY[i] > quitBottom && barY[i] - viewBottom >= kRoom && (!have || barY[i] < best)) { best = barY[i]; have = true; }
    if (have) return best;
    for (int i = 0; barY && i < n; ++i)
        if (barY[i] - viewBottom >= kRoom && (!have || barY[i] < best)) { best = barY[i]; have = true; }
    return have ? best : viewBottom + 34;
}

// Measures the vanilla button column (top -> bottom). false when any row is unreadable.
bool measureColumn(uintptr_t P, Rect* col) {
    bool ok = true;
    for (int i = 0; i < kColumnN; ++i)
        if (!ctlRect(listCtl(P, kColumnIdx[i]), &col[i])) ok = false;
    return ok;
}

// Column rects as of before the reflow: the cache filled at McBind / first tick, else a live measure.
bool columnNow(uintptr_t P, Rect* col) {
    if (g_mc.colOk) {
        for (int i = 0; i < kColumnN; ++i) col[i] = g_mc.col[i];
        return true;
    }
    return measureColumn(P, col);
}

bool paneRect(uintptr_t P, int which, Rect* out) {
    Rect col[kColumnN];
    switch (which) {
        case MUI_PANE_COLUMN: {
            if (!columnNow(P, col)) return false;
            int x0 = col[0].x, y0 = col[0].y, x1 = col[0].x + col[0].w, y1 = col[0].y + col[0].h;
            for (int i = 1; i < kColumnN; ++i) {
                if (col[i].x < x0) x0 = col[i].x;
                if (col[i].y < y0) y0 = col[i].y;
                if (col[i].x + col[i].w > x1) x1 = col[i].x + col[i].w;
                if (col[i].y + col[i].h > y1) y1 = col[i].y + col[i].h;
            }
            *out = Rect{x0, y0, x1 - x0, y1 - y0};
            return true;
        }
        case MUI_PANE_DESC: return ctlRect(listCtl(P, kIdxDesc), out);
        case MUI_PANE_TITLE: return ctlRect(listCtl(P, kIdxTitle), out);
        case MUI_PANE_BACK: return ctlRect(listCtl(P, kIdxExit), out);
        case MUI_PANE_VIEW:
        case MUI_PANE_STATUS: {
            Rect desc = {0, 0, 0, 0};
            if (!ctlRect(listCtl(P, kIdxDesc), &desc)) return false;
            const Rect v = paneViewRect(desc);
            if (v.w <= 0 || v.h <= 0) return false;
            if (which == MUI_PANE_VIEW) { *out = v; return true; }
            int barY[kBarLast - kBarFirst + 1];
            int nBars = 0;
            for (int i = kBarFirst; i <= kBarLast; ++i) {
                Rect b = {0, 0, 0, 0};
                if (ctlRect(listCtl(P, i), &b)) barY[nBars++] = b.y;
            }
            int quitBottom = 0;
            if (columnNow(P, col)) quitBottom = col[kColumnN - 1].y + col[kColumnN - 1].h;
            const int floorY = paneStatusFloor(barY, nBars, quitBottom, v.y + v.h);
            *out = menulayout::statusStrip(v, floorY);
            return out->h > 0;
        }
        default: return false;
    }
}

// ---------------------------------------------------------------------------------------------------- item helpers
template <typename F>
void eachOwn(F f) {
    McState& e = g_mc;
    f(e.root);
    for (int i = 0; i < e.nList; ++i) f(e.list[i]);
    f(e.back);
    f(e.title);
    for (int k = 0; k < mui::kMaxEntries; ++k)
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) f(e.items[k][i]);
}

// The item the API names, or null (page not an entry page, item out of range, or no panel bound).
ItemX* itemAt(uint32_t page, int32_t item) {
    if (page < 2 || page >= 2 + static_cast<uint32_t>(mui::kMaxEntries) || item < 0) return nullptr;
    McState& e = g_mc;
    if (item >= e.pt.itemCount(static_cast<int>(page))) return nullptr;
    return &e.items[page - 2][item];
}

void setShown(ItemX& it, bool v) {
    if (it.ctl && !it.unlinked) guikit::setVisible(it.ctl, v);
}

bool overCtl(const ItemX& it, int x, int y) { return it.ctl && !it.unlinked && guikit::hit(it.ctl, x, y); }

void applyItemText(ItemX& it) {
    if (it.ctl && it.hasText && !it.unlinked) setCtlText(it.ctl, it.isButton, it.text);
}

void copyStr(char* dst, size_t cap, const char* src) {
    if (!cap) return;
    if (!src) src = "";
    strncpy(dst, src, cap - 1);
    dst[cap - 1] = 0;
}

int curPageId() {
    if (g_mc.ps.state == mui::MAIN) return 0;
    if (g_mc.ps.state == mui::LIST) return 1;
    return 2 + g_mc.ps.entry;
}

// ---------------------------------------------------------------------------------------------------- first draw
void appendItem(ItemX& it, const char* what) {
    if (!it.ctl) return;
    const uintptr_t P = g_mc.panel;
    if (it.nofree) {   // a consumer control may already be listed (its id is its list index)
        const int id = ctlId(it.ctl);
        if (id >= 0 && listCtl(P, id) == it.ctl) {
            it.listed = true;
            KP_LOG(chMain, "APPEND %s ctl=%x already listed id=%d", what, static_cast<unsigned>(it.ctl), id);
            return;
        }
    }
    const bool ok = guikit::appendToPanel(P, it.ctl);
    it.listed = ok;
    KP_LOG(chMain, "APPEND %s ctl=%x ok=%d id=%d", what, static_cast<unsigned>(it.ctl), ok ? 1 : 0, ctlId(it.ctl));
    if (!ok && !it.nofree) it.ctl = 0;   // unlisted and unreachable: leak it rather than risk a free
}

// First Options draw: list our controls (id = list index), measure the vanilla rects (ultrawide-aware), place everything, show the root.
void firstTick() {
    McState& e = g_mc;
    const uintptr_t P = e.panel;
    e.appended = true;
    appendItem(e.root, "root");
    for (int i = 0; i < e.nList; ++i) appendItem(e.list[i], "list");
    appendItem(e.back, "back");
    appendItem(e.title, "title");
    for (int k = 0; k < mui::kMaxEntries; ++k)
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) appendItem(e.items[k][i], "item");

    // Button column, top -> bottom (measured before any row moves).
    Rect col[kColumnN];
    const bool colOk = measureColumn(P, col);
    if (colOk) {
        for (int i = 0; i < kColumnN; ++i) e.col[i] = col[i];
        e.colOk = true;
        if (g_debug)
            for (int i = 0; i < kColumnN; ++i) KP_LOG(chMain, "COLUMN pos=%d idx=%d (%d,%d,%d,%d)", i, kColumnIdx[i], col[i].x, col[i].y, col[i].w, col[i].h);
    }
    Rect exitR = {0, 0, 0, 0}, titleR = {0, 0, 0, 0};
    const bool xOk = ctlRect(listCtl(P, kIdxExit), &exitR);
    const bool tOk = ctlRect(listCtl(P, kIdxTitle), &titleR);
    KP_LOG(chMain, "TEMPLATE column=%d exit=%d (%d,%d,%d,%d) title=%d (%d,%d,%d,%d) vanilla=%d", colOk ? 1 : 0, xOk ? 1 : 0, exitR.x, exitR.y, exitR.w, exitR.h,
           tOk ? 1 : 0, titleR.x, titleR.y, titleR.w, titleR.h, e.vanillaCount);

    // Root button: after GAMEPLAY with the later rows moved down one pitch, else below QUIT.
    Rect nw = {0, 0, 0, 0}, moved[kMovedN];
    if (e.root.ctl) {
        if (colOk && g_cfg.reflow && menulayout::columnReflow(col, kColumnN, kInsertAfter, &nw, moved)) {
            addTarget(e, e.root.ctl, nw);
            for (int j = 0; j < kMovedN; ++j) addTarget(e, listCtl(P, kColumnIdx[kInsertAfter + 1 + j]), moved[j]);
            KP_LOG(chMain, "REFLOW on new=(%d,%d,%d,%d) moved=%d", nw.x, nw.y, nw.w, nw.h, kMovedN);
        } else if (colOk) {
            int pitch = col[kColumnN - 1].y - col[kColumnN - 2].y;
            if (pitch <= 0) pitch = col[kColumnN - 1].h * 36 / 32;
            nw = col[kColumnN - 1];
            nw.y = menulayout::slotBelow(col[kColumnN - 1], pitch, 1);
            addTarget(e, e.root.ctl, nw);
            KP_LOG(chMain, "REFLOW off (cfg=%d) new=(%d,%d,%d,%d) pitch=%d", g_cfg.reflow ? 1 : 0, nw.x, nw.y, nw.w, nw.h, pitch);
        } else {
            Rect q;   // column unreadable: below QUIT, 36/32 pitch
            const bool qOk = ctlRect(listCtl(P, kIdxQuit), &q);
            if (qOk) {
                nw = q;
                nw.y = menulayout::slotBelow(q, q.h * 36 / 32, 1);
                addTarget(e, e.root.ctl, nw);
            }
            KP_LOG(chMain, "REFLOW off (column unreadable) placed=%d new=(%d,%d,%d,%d)", qOk ? 1 : 0, nw.x, nw.y, nw.w, nw.h);
        }
    }
    // List page: one button per entry on the column rects from the top slot down; Back on the BTN_EXIT rect; the title on LBL_TITLE.
    if (colOk)
        for (int i = 0; i < e.nList; ++i) addTarget(e, e.list[i].ctl, col[i]);
    if (xOk) addTarget(e, e.back.ctl, exitR);
    if (tOk) addTarget(e, e.title.ctl, titleR);
    applyTargets(e, true);

    // Entry page items: the rects the entries set during build().
    for (int k = 0; k < mui::kMaxEntries; ++k)
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) {
            ItemX& it = e.items[k][i];
            if (it.ctl && it.hasRect && !it.unlinked) {
                it.tg.ctl = it.ctl;
                applyOne(e, it.tg, true);
            }
        }

    setCtlText(e.root.ctl, e.root.isButton, kRootText);
    for (int i = 0; i < e.nList; ++i) applyItemText(e.list[i]);
    setCtlText(e.back.ctl, e.back.isButton, "Back");
    if (e.title.ctl) guikit::setText(e.title.ctl, kRootText);
    for (int k = 0; k < mui::kMaxEntries; ++k)
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) applyItemText(e.items[k][i]);

    // Clones start hidden + click-through. MAIN shows only the root.
    setShown(e.root, true);
    for (int i = 0; i < e.nList; ++i) setShown(e.list[i], false);
    setShown(e.back, false);
    setShown(e.title, false);
    for (int k = 0; k < mui::kMaxEntries; ++k)
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) setShown(e.items[k][i], false);
    logRect("root", e.root.ctl);
    logRect("back", e.back.ctl);
    logRect("title", e.title.ctl);
    if (e.nList > 0) logRect("list0", e.list[0].ctl);
}

// ---------------------------------------------------------------------------------------------------- page changes
void hideVanilla() {
    McState& e = g_mc;
    const int oFlags = ctlFlagsOff();
    if (!oFlags || e.haveSaved) return;
    const int n = e.vanillaCount < kMaxVanilla ? e.vanillaCount : kMaxVanilla;
    for (int i = 0; i < n; ++i) {
        const uintptr_t c = listCtl(e.panel, i);
        int fl = 0;
        if (i == kIdxDesc || (i >= kIdxBarFirst && i <= kIdxBarLast)) { e.savedFlags[i] = -1; continue; }   // LB_DESC (hover descriptions) and the LBL_BAR frame bars stay visible on LIST / ENTRY; -1 = restore skips it
        e.savedFlags[i] = (c && rd<int>(c, oFlags, &fl)) ? fl : -1;
        if (g_debug) KP_LOG(chMain, "ENTER save idx=%d ctl=%x flags=%x", i, static_cast<unsigned>(c), static_cast<unsigned>(e.savedFlags[i]));
        if (c && e.savedFlags[i] != -1) guikit::setVisible(c, false);   // everything vanilla, BTN_EXIT and LBL_TITLE included
    }
    e.haveSaved = true;
}

bool lmbDown() { return guikit::g_gk.gameForeground && guikit::g_gk.lbuttonDown && guikit::g_gk.gameForeground() && guikit::g_gk.lbuttonDown(); }

void restoreVanilla(const char* why) {
    McState& e = g_mc;
    e.restorePending = false;
    const int oFlags = ctlFlagsOff();
    const int n = e.vanillaCount < kMaxVanilla ? e.vanillaCount : kMaxVanilla;
    int restored = 0;
    if (e.haveSaved && oFlags) {
        for (int i = 0; i < n; ++i) {
            const uintptr_t c = listCtl(e.panel, i);
            // Review F5: put back only the two bits setVisible changed (visible bit1, click-through 0x20); any other state bit the
            // vanilla code changed while the page was open (hover, selected, disabled) is kept as it is now.
            int cur = 0;
            const int mask = guikit::FlagVisible | guikit::FlagClickThrough;
            if (c && e.savedFlags[i] != -1 && rd<int>(c, oFlags, &cur) && guikit::detail::wr<int>(c, oFlags, (cur & ~mask) | (e.savedFlags[i] & mask))) ++restored;
        }
        KP_LOG(chMain, "VANILLA restored=%d of %d (%s)", restored, n, why);
    }
    e.haveSaved = false;
}

void callEntryLeave(int k) {
    if (!entLive(k) || !g_ent[k].leave) return;
    g_ent[k].leave(static_cast<uint32_t>(2 + k), g_ent[k].user);
}

void applyEvent(const mui::PageEvent& ev, const char* why) {
    McState& e = g_mc;
    const bool enter = ev.kind == mui::ENTER;
    switch (ev.state) {
        case mui::MAIN:
            if (!enter) {
                e.restorePending = false;
                hideVanilla();
                setShown(e.root, false);
            } else {
                // Never uncover BTN_EXIT under a held LMB (its release could land on it): wait for LMB up (McTick finishes it).
                if (lmbDown()) { e.restorePending = true; KP_LOG(chMain, "VANILLA restore deferred: LMB down (%s)", why); }
                else restoreVanilla(why);
                setShown(e.root, true);
                setShown(e.back, false);
                setShown(e.title, false);
            }
            break;
        case mui::LIST:
            if (!enter) {
                for (int i = 0; i < e.nList; ++i) setShown(e.list[i], false);
                setShown(e.back, false);
                setShown(e.title, false);
            } else {
                const bool cleared = clearFocus(e.panel);
                if (e.title.ctl) guikit::setText(e.title.ctl, kRootText);
                for (int i = 0; i < e.nList; ++i) setShown(e.list[i], true);
                setShown(e.back, true);
                setShown(e.title, true);
                KP_LOG(chMain, "LIST enter entries=%d focusCleared=%d", e.nList, cleared ? 1 : 0);
                writeDesc(kListBlurb);
            }
            break;
        case mui::ENTRY: {
            const int k = ev.entry;
            if (k < 0 || k >= mui::kMaxEntries) break;
            if (!enter) {
                callEntryLeave(k);
                for (int i = 0; i < e.pt.itemCount(2 + k); ++i) setShown(e.items[k][i], false);
                setShown(e.back, false);
                setShown(e.title, false);
            } else {
                const bool cleared = clearFocus(e.panel);
                if (e.title.ctl) guikit::setText(e.title.ctl, e.pageTitle[k]);
                for (int i = 0; i < e.pt.itemCount(2 + k); ++i) {
                    ItemX& it = e.items[k][i];
                    if (it.ctl && it.hasRect && !it.unlinked) {
                        it.tg.ctl = it.ctl;
                        it.tg.applied = 0;
                        it.tg.gaveUp = false;
                        applyOne(e, it.tg, true);
                    }
                    setShown(it, true);
                }
                setShown(e.back, true);
                setShown(e.title, true);
                KP_LOG(chMain, "ENTRY enter id=%s page=%d items=%d focusCleared=%d", entLive(k) ? g_ent[k].id : "?", 2 + k, e.pt.itemCount(2 + k), cleared ? 1 : 0);
                if (entLive(k) && g_ent[k].enter) g_ent[k].enter(static_cast<uint32_t>(2 + k), g_ent[k].user);
            }
            break;
        }
    }
}

void pageChange(int from, const mui::PageEvents& ev, const char* why) {
    McState& e = g_mc;
    for (int i = 0; i < ev.n; ++i) applyEvent(ev.ev[i], why);
    e.leaveAt = 0;
    e.hov = mui::HoverTracker();
    e.clk = mui::ClickTracker();
    e.clkBack = mui::ClickTracker();
    const int to = curPageId();
    if (from != to) KP_LOG(chMain, "PAGE %d->%d (%s)", from, to, why);
}

void resetPages(const char* why) {
    const int from = curPageId();
    pageChange(from, g_mc.ps.reset(), why);
}

// ---------------------------------------------------------------------------------------------------- input
// The item index (0 .. kMaxItems-1) of the current page's clickable control under (x, y), else -1. Back is handled apart.
int hitItem(int x, int y) {
    McState& e = g_mc;
    if (e.ps.state == mui::MAIN) return overCtl(e.root, x, y) ? 0 : -1;
    if (e.ps.state == mui::LIST) {
        for (int i = 0; i < e.nList; ++i)
            if (overCtl(e.list[i], x, y)) return i;
        return -1;
    }
    const int k = e.ps.entry;
    if (k < 0 || k >= mui::kMaxEntries) return -1;
    for (int i = 0; i < e.pt.itemCount(2 + k); ++i) {
        const ItemX& it = e.items[k][i];
        if (e.pt.pages[2 + k].items[i].kind == mui::LABEL) continue;
        if (overCtl(it, x, y)) return i;
    }
    return -1;
}

ItemX* shownItem(int item) {
    McState& e = g_mc;
    if (item < 0) return nullptr;
    if (e.ps.state == mui::MAIN) return item == 0 ? &e.root : nullptr;
    if (e.ps.state == mui::LIST) return item < e.nList ? &e.list[item] : nullptr;
    const int k = e.ps.entry;
    if (k < 0 || k >= mui::kMaxEntries || item >= e.pt.itemCount(2 + k)) return nullptr;
    return &e.items[k][item];
}

const char* descFor(int item) {
    McState& e = g_mc;
    if (e.ps.state == mui::MAIN) return item == 0 ? kRootDesc : nullptr;
    if (e.ps.state == mui::LIST) {
        if (item < 0 || item >= e.nList) return nullptr;
        const int slot = e.listTok[item] - 1;
        return entLive(slot) ? g_ent[slot].desc : nullptr;
    }
    ItemX* it = shownItem(item);
    return it && it->desc[0] ? it->desc : nullptr;
}

void hoverStep(int hit) {
    McState& e = g_mc;
    const mui::HoverEvents he = e.hov.update(hit);
    for (int i = 0; i < he.n; ++i) {
        const bool enter = he.ev[i].kind == mui::ENTER;
        ItemX* it = shownItem(he.ev[i].item);
        uint32_t M = 0, m08 = 0;
        if (rd<uint32_t>(e.panel, kPanelMgr, &M) && M) (void)rd<uint32_t>(M, kMgrHover, &m08);
        const int hl = (it && it->ctl && it->isButton) ? (guibutton::buttonHighlighted(it->ctl) ? 1 : 0) : -1;
        static volatile LONG s_hv = 0;
        if (InterlockedIncrement(&s_hv) <= 64)
            KP_LOG(chInput, "HOVER page=%d item=%d %s hl=%d m08=%x", curPageId(), he.ev[i].item, enter ? "enter" : "leave", hl, static_cast<unsigned>(m08));
        if (enter) {
            const char* d = descFor(he.ev[i].item);
            if (d && d[0]) writeDesc(d);
        }
    }
    if (he.n > 0 && e.hov.cur < 0 && e.ps.state == mui::LIST) writeDesc(kListBlurb);
}

void entryClicked(int listIndex) {
    McState& e = g_mc;
    if (listIndex < 0 || listIndex >= e.nList) return;
    const int slot = e.listTok[listIndex] - 1;
    const char* id = entLive(slot) ? g_ent[slot].id : "?";
    if (!entLive(slot) || !e.pageMade[slot] || e.pt.itemCount(2 + slot) == 0) {
        KP_LOG(chMain, "ENTRY %s empty", id);
        return;
    }
    const int from = curPageId();
    pageChange(from, e.ps.push(mui::ENTRY, slot), "click");
}

// ---------------------------------------------------------------------------------------------------- McDestroy support
// Clears every engine reference to L (focus, hover, capture, last hit). Hover leave runs L->vt[0x0C](0) first.
void dropRefs(uintptr_t P, uint32_t M, uintptr_t L) {
    uint32_t v = 0;
    if (rd<uint32_t>(P, kPanelFocus, &v) && v == static_cast<uint32_t>(L)) { clearFocus(P); KP_LOG(chMain, "FREE refs: focus cleared for %x", static_cast<unsigned>(L)); }
    if (!M) return;
    if (rd<uint32_t>(M, kMgrHover, &v) && v == static_cast<uint32_t>(L)) {
        uint32_t vt = 0, fn = 0;
        if (rd<uint32_t>(L, 0, &vt) && vt && rd<uint32_t>(vt, kVtHover, &fn) && fn) reinterpret_cast<Call1Fn>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(L), 0);
        guikit::detail::wr<uint32_t>(M, kMgrHover, 0);
        KP_LOG(chMain, "FREE refs: hover left + cleared for %x", static_cast<unsigned>(L));
    }
    if (rd<uint32_t>(M, kMgrCapture, &v) && v == static_cast<uint32_t>(L)) {
        guikit::detail::wr<uint32_t>(M, kMgrCapture, 0);
        guikit::detail::wr<uint32_t>(M, kMgrCaptureFlag, 0);
        KP_LOG(chMain, "FREE refs: capture cleared for %x", static_cast<unsigned>(L));
    }
    if (rd<uint32_t>(M, kMgrLastHit, &v) && v == static_cast<uint32_t>(L)) guikit::detail::wr<uint32_t>(M, kMgrLastHit, 0);
}

// vt[0](1) for a control whose vtable is the label or button vtable (R9 Q1: scalar deleting dtor 885AB0 for buttons).
void freeCtl(uintptr_t L, const char* what) {
    uint32_t vt = 0, fn = 0;
    const bool got = rd<uint32_t>(L, 0, &vt) && (vt == kLabelVtable || vt == kButtonVtable) && rd<uint32_t>(vt, 0, &fn) && fn;
    KP_LOG(chMain, "FREE %s ctl=%x vt=%x call=%d", what, static_cast<unsigned>(L), static_cast<unsigned>(vt), got ? 1 : 0);
    if (got && guikit::g_gkCallLabelDtor) guikit::g_gkCallLabelDtor(fn, L, 1u);
}

bool isOwnCtl(uintptr_t v) {
    if (!v) return false;
    bool own = false;
    eachOwn([&](ItemX& it) {
        if (it.ctl == v) own = true;
    });
    return own;
}

bool isOursFn(uintptr_t v, void*) { return isOwnCtl(v); }

// Clears references to all of our controls (incl. MUI_NOFREE) and truncates them off the panel list (the vanilla dtor frees only the
// array buffer, never a listed control). Gated on mui::canTruncate. Returns true when the list is clean of our controls.
bool unlinkOwn(uintptr_t P) {
    McState& e = g_mc;
    uint32_t M = 0, A = 0;
    int C = 0;
    (void)rd<uint32_t>(P, kPanelMgr, &M);
    const int oArr = k2addr::off(&K2AddrTable::Panel_CtlArray), oCount = k2addr::off(&K2AddrTable::Panel_CtlCount);
    int nOwn = 0;
    eachOwn([&](ItemX& it) {
        if (!it.ctl) return;
        dropRefs(P, M, it.ctl);
        if (it.listed) ++nOwn;
    });
    bool clean = false;
    if (!e.appended) {
        clean = true;   // never listed: nothing references them but this struct
    } else if (!oArr || !oCount || !rd<uint32_t>(P, oArr, &A) || !A || !rd<int>(P, oCount, &C)) {
        KP_LOG(chMain, "FREE skip: list unreadable");
    } else {
        const int V = e.vanillaCount;
        static uintptr_t listed[kMaxVanilla + kMaxOwn + 8];
        const int cap = static_cast<int>(sizeof listed / sizeof listed[0]);
        if (C < V || C > V + nOwn || C > cap) {
            KP_LOG(chMain, "FREE skip: list count %d outside [%d, %d]", C, V, V + nOwn);
        } else {
            for (int i = 0; i < C; ++i) {
                uint32_t entry = 0;
                listed[i] = rd<uint32_t>(A, i * 4, &entry) ? entry : 0;
            }
            if (!mui::canTruncate(listed, C, V, isOursFn, nullptr)) {
                KP_LOG(chMain, "FREE skip: a list entry in [%d, %d) is not ours (no truncate, no frees)", V, C);
            } else if (!guikit::detail::wr<int>(P, oCount, V)) {
                KP_LOG(chMain, "FREE skip: count write failed");
            } else {
                for (int i = V; i < C; ++i) guikit::detail::wr<uint32_t>(A, i * 4, 0);
                KP_LOG(chMain, "FREE truncated list %d -> %d", C, V);
                clean = true;
            }
        }
    }
    if (clean) eachOwn([](ItemX& it) {
        if (it.ctl) it.unlinked = true;
    });
    return clean;
}

// ---------------------------------------------------------------------------------------------------- k2.modconfig@1
int32_t KPATCH_CALL mcRegister(const K2ModConfigEntry* e) {
    if (!e || e->struct_size < sizeof(K2ModConfigEntry) || !e->id || !e->id[0]) return 0;
    const int token = g_reg.registerEntry(e->id);
    if (token <= 0) {
        KP_LOG(chMain, "REGISTER %s refused (all %d slots used)", e->id, mui::kMaxEntries);
        return 0;
    }
    K2ModConfigEntry& d = g_ent[token - 1];
    d = *e;
    if (!d.name || !d.name[0]) d.name = d.id;
    if (!d.desc) d.desc = "";
    KP_LOG(chMain, "REGISTER %s token=%d", d.id, token);
    return token;
}

void KPATCH_CALL mcUnregister(int32_t token) {
    if (token < 1 || token > mui::kMaxEntries || !g_reg.slots[token - 1].live) return;
    KP_LOG(chMain, "UNREGISTER %s token=%d", g_ent[token - 1].id, static_cast<int>(token));
    g_reg.unregister(token);
    g_ent[token - 1] = K2ModConfigEntry();
}

const K2ModConfigApi g_mcApi = {sizeof(K2ModConfigApi), &mcRegister, &mcUnregister};

// ---------------------------------------------------------------------------------------------------- k2.menuui@1
// NewPage / Add* are valid only inside an entry's build() and only for that entry's own page.
bool buildGate(uint32_t page, const char* what) {
    if (!g_inBuild || g_buildSlot < 0 || !g_mc.panel) {
        static volatile LONG s_ref = 0;
        if (InterlockedIncrement(&s_ref) <= 8) KP_LOG(chMain, "MUI refused (not in build): %s", what);
        return false;
    }
    if (page != static_cast<uint32_t>(2 + g_buildSlot)) {
        KP_LOG(chMain, "MUI refused: %s page %u is not the page of the entry being built (%d)", what, static_cast<unsigned>(page), 2 + g_buildSlot);
        return false;
    }
    return true;
}

uint32_t KPATCH_CALL muiNewPage(void* panel, const char* title) {
    if (!g_inBuild || g_buildSlot < 0 || !g_mc.panel) {
        static volatile LONG s_ref = 0;
        if (InterlockedIncrement(&s_ref) <= 8) KP_LOG(chMain, "MUI refused (not in build): NewPage");
        return 0;
    }
    if (reinterpret_cast<uintptr_t>(panel) != g_mc.panel) {
        KP_LOG(chMain, "MUI refused: NewPage panel %p is not the bound Options panel", panel);
        return 0;
    }
    McState& e = g_mc;
    const int k = g_buildSlot;
    if (e.pageMade[k]) KP_LOG(chMain, "MUI NewPage again for %s: same page %d (title updated)", g_ent[k].id, 2 + k);
    e.pageMade[k] = true;
    copyStr(e.pageTitle[k], sizeof e.pageTitle[k], title && title[0] ? title : g_ent[k].name);
    return static_cast<uint32_t>(2 + k);
}

int addItemCommon(uint32_t page, mui::ItemKind kind, ItemX& proto, int flags) {
    McState& e = g_mc;
    const int k = static_cast<int>(page) - 2;
    mui::Rect r0 = {0, 0, 0, 0};
    const int id = e.pt.addItem(static_cast<int>(page), kind, reinterpret_cast<void*>(proto.ctl), flags, r0);
    if (id < 0) return -1;
    e.items[k][id] = proto;
    return id;
}

int32_t KPATCH_CALL muiAddButton(uint32_t page, const char* text, const char* desc) {
    if (!buildGate(page, "AddButton")) return -1;
    McState& e = g_mc;
    const int k = static_cast<int>(page) - 2;
    if (!e.pageMade[k]) { KP_LOG(chMain, "MUI refused: AddButton before NewPage"); return -1; }
    if (e.pt.itemCount(static_cast<int>(page)) >= mui::kMaxItems) { KP_LOG(chMain, "MUI refused: page %u is full", static_cast<unsigned>(page)); return -1; }
    char what[40];
    snprintf(what, sizeof what, "item_%u_%d", static_cast<unsigned>(page), e.pt.itemCount(static_cast<int>(page)));
    ItemX it;
    it.ctl = cloneEntry(e.panel, "BTN_GAMEPLAY", what, &it.isButton);
    if (!it.ctl) return -1;
    copyStr(it.text, sizeof it.text, text);
    it.hasText = true;
    copyStr(it.desc, sizeof it.desc, desc);
    const int id = addItemCommon(page, mui::BUTTON, it, 0);
    if (id < 0) {
        if (it.isButton) guibutton::releaseUnlinkedButton(it.ctl);
        else guikit::releaseUnlinkedLabel(it.ctl, kLabelVtable);
        return -1;
    }
    return id;
}

int32_t KPATCH_CALL muiAddLabel(uint32_t page, const char* text) {
    if (!buildGate(page, "AddLabel")) return -1;
    McState& e = g_mc;
    const int k = static_cast<int>(page) - 2;
    if (!e.pageMade[k]) { KP_LOG(chMain, "MUI refused: AddLabel before NewPage"); return -1; }
    if (e.pt.itemCount(static_cast<int>(page)) >= mui::kMaxItems) { KP_LOG(chMain, "MUI refused: page %u is full", static_cast<unsigned>(page)); return -1; }
    char what[40];
    snprintf(what, sizeof what, "label_%u_%d", static_cast<unsigned>(page), e.pt.itemCount(static_cast<int>(page)));
    ItemX it;
    it.ctl = cloneLabel(e.panel, what);
    if (!it.ctl) return -1;
    copyStr(it.text, sizeof it.text, text);
    it.hasText = true;
    const int id = addItemCommon(page, mui::LABEL, it, 0);
    if (id < 0) {
        guikit::releaseUnlinkedLabel(it.ctl, kLabelVtable);
        return -1;
    }
    return id;
}

int32_t KPATCH_CALL muiAddCustom(uint32_t page, void* ctl, int32_t flags) {
    if (!buildGate(page, "AddCustom")) return -1;
    McState& e = g_mc;
    const int k = static_cast<int>(page) - 2;
    if (!ctl) return -1;
    if (!e.pageMade[k]) { KP_LOG(chMain, "MUI refused: AddCustom before NewPage"); return -1; }
    ItemX it;
    it.ctl = reinterpret_cast<uintptr_t>(ctl);
    it.nofree = (flags & MUI_NOFREE) != 0;
    return addItemCommon(page, mui::CUSTOM, it, flags);
}

int32_t KPATCH_CALL muiClicked(uint32_t page, int32_t item) {
    ItemX* it = itemAt(page, item);
    if (!it || !it->latched) return 0;
    it->latched = false;
    return 1;
}

void KPATCH_CALL muiSetText(uint32_t page, int32_t item, const char* text) {
    ItemX* it = itemAt(page, item);
    if (!it || !text) return;
    saferead::beginScope();
    copyStr(it->text, sizeof it->text, text);
    it->hasText = true;
    if (g_mc.appended && g_mc.pt.pages[page].items[item].kind != mui::CUSTOM) applyItemText(*it);   // before the first draw, firstTick applies it
}

void KPATCH_CALL muiSetRect(uint32_t page, int32_t item, const K2MuiRect* r) {
    ItemX* it = itemAt(page, item);
    if (!it || !r) return;
    saferead::beginScope();
    it->tg = Target();
    it->tg.ctl = it->ctl;
    it->tg.r = Rect{r->x, r->y, r->w, r->h};
    it->hasRect = true;
    if (g_mc.appended && it->ctl && !it->unlinked) applyOne(g_mc, it->tg, true);   // before the first draw, firstTick applies it
}

void KPATCH_CALL muiSetDesc(uint32_t /*page*/, const char* text) {
    saferead::beginScope();
    writeDesc(text);
}

int32_t KPATCH_CALL muiPaneRect(void* panel, int32_t which, K2MuiRect* out) {
    if (!out || !g_mc.panel || reinterpret_cast<uintptr_t>(panel) != g_mc.panel) return 0;
    saferead::beginScope();
    Rect r = {0, 0, 0, 0};
    if (!paneRect(g_mc.panel, which, &r)) return 0;
    out->x = r.x;
    out->y = r.y;
    out->w = r.w;
    out->h = r.h;
    return 1;
}

// nullptr once McDestroy has unlinked the item from the panel: a MUI_NOFREE control is then safe for its owner to free. While the unlink
// was refused (or has not run) the pointer stays, so consumers do not free a control that is still listed.
void* KPATCH_CALL muiControl(uint32_t page, int32_t item) {
    ItemX* it = itemAt(page, item);
    if (!it || it->unlinked || !it->ctl) return nullptr;
    return reinterpret_cast<void*>(it->ctl);
}

const K2MenuUiApi g_muiApi = {sizeof(K2MenuUiApi), &muiNewPage, &muiAddButton, &muiAddLabel, &muiAddCustom, &muiClicked,
                              &muiSetText,         &muiSetRect, &muiSetDesc,   &muiPaneRect,  &muiControl};

}  // namespace

// H-O1 8A16FE: Options ctor, layout open, all vanilla InitControl calls done; panel = [ebp-0xAC].
extern "C" void __cdecl McBind(int panelArg) {
    static bool s_cfgRead = false;
    if (!s_cfgRead) {   // lazily, on the first Options panel (KPatchInit must not do file I/O)
        s_cfgRead = true;
        readConfig();
        KP_LOG(chMain, "CONFIG enabled=%d debug=%d button_clone=%d reflow=%d desc=%d free_labels=%d table=%s", g_enabled ? 1 : 0, g_debug ? 1 : 0,
               g_cfg.buttonClone ? 1 : 0, g_cfg.reflow ? 1 : 0, g_cfg.desc ? 1 : 0, g_cfg.freeLabels ? 1 : 0, k2addr::get() ? "ok" : "none");
    }
    if (!g_enabled || !panelArg) return;
    saferead::beginScope();
    const uintptr_t P = static_cast<uintptr_t>(static_cast<unsigned>(panelArg));
    uint32_t vt = 0, slot = 0;
    int flags48 = 0, count = 0;
    (void)rd<uint32_t>(P, 0, &vt);
    (void)rd<uint32_t>(P, kPanelLayoutSlot, &slot);
    (void)rd<int>(P, ctlFlagsOff(), &flags48);
    (void)rd<int>(P, k2addr::off(&K2AddrTable::Panel_CtlCount), &count);
    KP_LOG(chMain, "CTOR panel=%x vt=%x (expect %x) flags48=%x layout=%x controls=%d", static_cast<unsigned>(P), static_cast<unsigned>(vt),
           static_cast<unsigned>(kOptionsVtable), static_cast<unsigned>(flags48), static_cast<unsigned>(slot), count);
    if (g_mc.panel && g_mc.panel != P) KP_LOG(chMain, "CTOR previous panel %x was not destroyed through the hook: state reset", static_cast<unsigned>(g_mc.panel));
    g_inBuild = false;
    g_buildSlot = -1;
    resetState();
    if (vt != kOptionsVtable || !(flags48 & 2) || !slot) { KP_LOG(chMain, "CTOR refused: not the open Options layout"); return; }
    McState& e = g_mc;
    e.panel = P;
    e.vanillaCount = count;
    for (int i = 0; i < mui::kMaxPages; ++i) (void)e.pt.addPage();   // page ids 0 MAIN, 1 LIST, 2 + k ENTRY(k)
    e.colOk = measureColumn(P, e.col);

    // Buttons and labels from the vanilla layout (open until StopLoadFromLayout right after this hook), in this order:
    // root button, list buttons, Back, title.
    e.root.ctl = cloneEntry(P, "BTN_GAMEPLAY", "root", &e.root.isButton);
    e.nList = g_reg.count() < mui::kMaxEntries ? g_reg.count() : mui::kMaxEntries;
    for (int i = 0; i < e.nList; ++i) {
        const int tok = g_reg.at(i);
        const int s = tok - 1;
        e.listTok[i] = tok;
        char what[32];
        snprintf(what, sizeof what, "list_%d", i);
        e.list[i].ctl = cloneEntry(P, "BTN_GAMEPLAY", what, &e.list[i].isButton);
        copyStr(e.list[i].text, sizeof e.list[i].text, entLive(s) ? g_ent[s].name : "");
        e.list[i].hasText = true;
    }
    e.back.ctl = cloneEntry(P, "BTN_EXIT", "back", &e.back.isButton);
    e.title.ctl = cloneLabel(P, "title");
    if (!e.root.ctl) {
        KP_LOG(chMain, "CTOR root button clone failed: entries are not built for this panel");
        return;
    }
    for (int i = 0; i < e.nList; ++i) {
        const int s = e.listTok[i] - 1;
        if (!entLive(s) || !g_ent[s].build) continue;
        g_inBuild = true;
        g_buildSlot = s;
        g_ent[s].build(reinterpret_cast<void*>(P), g_ent[s].user);
        g_inBuild = false;
        g_buildSlot = -1;
        e.built[s] = true;
        KP_LOG(chMain, "BUILD %s items=%d", g_ent[s].id, e.pt.itemCount(2 + s));
    }
}

// H-O2 40EC90: CSWGuiPanel::Draw (ECX = panel). HOT: every panel, every frame.
extern "C" void __cdecl McTick(int panelArg) {
    if (!panelArg || *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(static_cast<unsigned>(panelArg))) != kOptionsVtable) return;   // ECX is Draw's this
    if (!g_enabled) return;
    const uintptr_t P = static_cast<uintptr_t>(static_cast<unsigned>(panelArg));
    saferead::beginScope();
    McState& e = g_mc;
    if (P != e.panel) {
        static volatile LONG s_other = 0;
        if (InterlockedIncrement(&s_other) <= 4) KP_LOG(chMain, "HIDDENDRAW options draw for an unbound panel %x (bound %x)", static_cast<unsigned>(P), static_cast<unsigned>(e.panel));
        return;
    }
    // A click latched last tick and never read is dropped now (Clicked answers 1 once, for one tick).
    for (int k = 0; k < mui::kMaxEntries; ++k)
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) e.items[k][i].latched = false;
    const DWORD now = GetTickCount();
    const DWORD gap = e.lastTick ? now - e.lastTick : 0;
    e.lastTick = now;
    int flags48 = 0;
    (void)rd<int>(P, ctlFlagsOff(), &flags48);
    static volatile LONG s_hd = 0;
    if ((flags48 != e.lastFlags48 || gap > kTickGapMs) && InterlockedIncrement(&s_hd) <= 32)
        KP_LOG(chMain, "HIDDENDRAW panel=%x flags48=%x (was %x) gap=%lums page=%d", static_cast<unsigned>(P), static_cast<unsigned>(flags48),
               static_cast<unsigned>(e.lastFlags48), static_cast<unsigned long>(gap), curPageId());
    e.lastFlags48 = flags48;
    if (gap > kTickGapMs) resetPages("tick gap");
    if (!e.appended) firstTick();
    applyTargets(e, false);
    if (e.ps.state == mui::ENTRY && e.ps.entry >= 0 && e.ps.entry < mui::kMaxEntries) {
        const int k = e.ps.entry;
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i)
            if (e.items[k][i].ctl && e.items[k][i].hasRect && !e.items[k][i].unlinked) applyOne(e, e.items[k][i].tg, false);
    }

    int x = 0, y = 0;
    const bool cur = guikit::cursorEngine(&x, &y);
    // Clicks act on the RELEASE: LMB up over the control the press went down on. Not-foreground counts as up (GuiKit clickEdge rule).
    const bool down = lmbDown();
    if (e.restorePending && !down) restoreVanilla("lmb up");
    const int hit = cur ? hitItem(x, y) : -1;
    const bool overBack = cur && e.ps.state != mui::MAIN && overCtl(e.back, x, y);
    hoverStep(overBack ? -1 : hit);
    if (down && !e.prevDown) {
        static volatile LONG s_in = 0;
        if (InterlockedIncrement(&s_in) <= 40)
            KP_LOG(chInput, "INPUTCLASS press cur=%d x=%d y=%d page=%d item=%d overBack=%d", cur ? 1 : 0, x, y, curPageId(), hit, overBack ? 1 : 0);
    }
    e.prevDown = down;
    const int clicked = e.clk.tick(down, overBack ? -1 : hit);
    const int backClicked = e.clkBack.tick(down, overBack ? 0 : -1);
    if (clicked >= 0 || backClicked >= 0) {
        static volatile LONG s_up = 0;
        if (InterlockedIncrement(&s_up) <= 40)
            KP_LOG(chInput, "INPUTCLASS release page=%d item=%d back=%d", curPageId(), clicked, backClicked >= 0 ? 1 : 0);
    }
    if (clicked >= 0) {
        const int from = curPageId();
        if (e.ps.state == mui::MAIN) {
            if (clicked == 0) pageChange(from, e.ps.push(mui::LIST, -1), "click");
        } else if (e.ps.state == mui::LIST) {
            entryClicked(clicked);
        } else {
            ItemX* it = shownItem(clicked);
            if (it) it->latched = true;
        }
    } else if (backClicked >= 0) {
        const int from = curPageId();
        if (e.ps.state == mui::ENTRY) pageChange(from, e.ps.back(), "back");
        else if (e.ps.state == mui::LIST && e.leaveAt == 0) e.leaveAt = now + kLeaveDelayMs;
    }
    // Back on LIST: restore the vanilla controls only once the LMB is up and the delay has passed (the release must not land on BTN_EXIT).
    if (e.ps.state == mui::LIST && e.leaveAt && !down && static_cast<LONG>(now - e.leaveAt) >= 0) {
        const int from = curPageId();
        pageChange(from, e.ps.back(), "back");
    }
    if (e.ps.state == mui::ENTRY && e.ps.entry >= 0 && e.ps.entry < mui::kMaxEntries) {
        const int k = e.ps.entry;
        if (entLive(k) && g_ent[k].tick) g_ent[k].tick(static_cast<uint32_t>(2 + k), g_ent[k].user);
    }
}

// H-O4 8A1930: Options scalar deleting dtor (ECX = this), on entry, before the children are freed.
extern "C" void __cdecl McDestroy(int panelArg) {
    if (!g_enabled || !panelArg) return;
    const uintptr_t P = static_cast<uintptr_t>(static_cast<unsigned>(panelArg));
    if (P != g_mc.panel) return;
    saferead::beginScope();
    McState& e = g_mc;
    KP_LOG(chMain, "DTOR panel=%x page=%d appended=%d free_labels=%d", static_cast<unsigned>(P), curPageId(), e.appended ? 1 : 0, g_cfg.freeLabels ? 1 : 0);
    // (1) leave the shown page: only the entry's leave callback (the panel is going away, nothing to show or hide).
    if (e.ps.state == mui::ENTRY && e.ps.entry >= 0) callEntryLeave(e.ps.entry);
    // (2) clear references to every control of ours and truncate them off the list (MUI_NOFREE ones too).
    const bool clean = unlinkOwn(P);
    // (3) each built entry's destroy(): its MUI_NOFREE controls answer nullptr from Control() once unlinked.
    for (int k = 0; k < mui::kMaxEntries; ++k)
        if (e.built[k] && entLive(k) && g_ent[k].destroy) g_ent[k].destroy(reinterpret_cast<void*>(P), g_ent[k].user);
    // (4) free our own labels and buttons (never a MUI_NOFREE one), only when the list is clean of them.
    int freed = 0, nofree = 0;
    eachOwn([&](ItemX& it) {
        if (!it.ctl) return;
        if (it.nofree) { ++nofree; return; }
        if (clean && g_cfg.freeLabels && it.unlinked) {
            freeCtl(it.ctl, "ctl");
            ++freed;
            it.ctl = 0;
        }
    });
    // (5)
    KP_LOG(chMain, "DTOR freed=%d nofree=%d clean=%d", freed, nofree, clean ? 1 : 0);
    resetState();
}

// Written out literally (create-patch's export scan reads the source text) and listed in exports.def. No file I/O here (loader lock):
// the config is read at the first McBind.
extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    KP_LOG(chMain, "mod-config 0.1.0-dev loaded (Mod Configuration button under Gameplay; k2.modconfig@1 + k2.menuui@1; config mod_config.ini is read at the first Options panel: enabled debug button_clone reflow desc free_labels)");
    int rcC = -1, rcM = -1;
    if (api && api->struct_size >= offsetof(KPatchApi, require) + sizeof(api->require) && api->provide) {
        rcC = api->provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &g_mcApi);
        rcM = api->provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &g_muiApi);
    } else {
        KP_LOG(chMain, "no KPatchApi with a registry (patcher predates it?): nothing provided");
    }
    KP_LOG(chMain, "PROVIDE modconfig=%d menuui=%d", rcC, rcM);
}

// No I/O here (loader lock).
BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
