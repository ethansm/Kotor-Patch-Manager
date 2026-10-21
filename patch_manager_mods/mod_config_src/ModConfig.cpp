// ModConfig.cpp -- mod-config 0.3.4-dev (Steam build): the "Mod Configuration" provider on the in-game Options screen (doc 42,
// plan .claude/plans/holster_editor_options.md, card c18). Provides two interfaces declared in ModConfigApi.h:
//   k2.modconfig@1  K2ModConfigApi  Register / Unregister: a mod adds one entry (name, description, build/enter/leave/tick/destroy).
//   k2.menuui@1     K2MenuUiApi     NewPage / AddButton / AddLabel / AddCustom / Clicked / SetText / SetRect / SetDesc / PaneRect / Control /
//                                   SetDescSplit (an entry page shrinks LB_DESC to its bottom 15..40 % and gets MUI_PANE_PREVIEW above it).
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
// Esc on a non-MAIN page is swallowed at the Options panel's key-handler slot and steps back one page (entry -> list -> main); on MAIN the game handles it. Fallback: the Esc key edge resets a non-MAIN page to MAIN. Reopening the Options screen starts at MAIN.
//
// Option rows (AddRow): toggle, stepper and slider (kind 2, MCF5: caption + wedge + frame + mark in one row slot, press / drag on the track sets the
// value through the page's change handler; a kind this build does not know makes AddRow return < 0 so the caller can fall back to a stepper).
//
// Failure values (documented here because the header leaves them open): NewPage returns 0 (page 0 is MAIN, never an entry page);
// AddButton / AddLabel / AddCustom return -1. One entry owns one page: a second NewPage in the same build() returns the same page id
// (the title is updated). Items can only be added to the page of the entry whose build() is running.
// Probe lines (log only): CONFIG, CTOR, BTNCLONE, LABEL, APPEND, TEMPLATE, REFLOW, RELAYOUT, HOVER, DESC, INPUTCLASS, PAGE, BUILD,
// REGISTER, MUI, FREE, DTOR, HIDDENDRAW, PROVIDE, DESCSPLIT, ROW, SLIDER, GEAR, MODAL, EMBED.
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
#include "GuiInput.h"   // before GuiKit.h: the mouse wheel comes from the gui-input provider (no window subclass of our own)
#include "GuiKit.h"
#include "GuiButton.h"
#include "MenuLayout.h"
#include "ModConfigApi.h"
#include "MenuUiLogic.h"
#include "SliderRowLogic.h"   // MCF5: the look-alike slider row maths (kind 2)
#include "SliderArt.h"        // MCF5: frame / wedge / mark art (EmbedTex textures)
#include "EditKit.h"
#include "GearButton.h"   // the shared gear: art names + EmbedTex registration (AddGear)
#include "CodePatch.h"    // Esc: the Options panel's key-handler vtable slot is pointed at escKeyHook (data patch, no code hook)

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
constexpr int kListMax = mui::kRowsVisible;   // list-page buttons (the 8 vanilla column slots); more entries scroll through them
constexpr int kBarCtl = 3;                    // scroll bar controls per scrolling page: up, down, thumb
constexpr int kBarUp = 0, kBarDown = 1, kBarThumb = 2;
constexpr int kBarPctOfCol = 7;               // bar width = this % of the column height (min kBarMinPx)
constexpr int kBarMinPx = 12;
constexpr int kWheelNotch = 120;
constexpr int kMaxOwn = 1 + kListMax + 2 + mui::kMaxSlots * mui::kMaxItems + (mui::kMaxSlots + 1) * kBarCtl;   // root + list + back + title + entry and modal items + scroll bars
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
    bool arrowIcons = false;   // row arrows: false = "<" ">" glyph text on the button clone; true = chevron textures (NOT built yet: logged only)
    int selftest = 0;          // 1 = register the built-in "Mod-config self test" entry (6 rows); never on in a shipped config
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

// ---------------------------------------------------------------------------------------------------- option rows (doc 46)
// A row is one toggle button, a stepper = caption label + "<" + ">" buttons, or a slider = caption label + frame + wedge + mark labels (MCF5). Rows are added by AddRow; the provider lays them out in the
// COLUMN pane (mui::rowSlotRect), draws the caption (mui::rowFormat), steps the value on a click / arrow press and calls the page's change handler.
constexpr int kMaxRows = 32;
struct RowX {
    bool used = false;
    mui::RowCfg cfg = {0, 0, 1, 1, 0, 0};
    int item[4] = {-1, -1, -1, -1};   // item ids on the page: toggle = [0]; stepper = caption label [0], "<" [1], ">" [2]; slider = caption [0], frame [1], wedge [2], mark [3]
    mui::Rect track = {0, 0, 0, 0};   // slider: the frame rect as laid out (the hit area and the value <-> pixel maths); only meaningful while slot >= 0
    bool artDone = false;             // slider: the art fill was applied (or given up); artTries counts the fill attempts while the textures were not resolvable yet
    int artTries = 0;
    int slot = -1;                // visible slot (0..kRowsVisible-1), -1 = not shown
    bool hasFmt = false, hasZero = false;
    char label[64] = {}, desc[192] = {}, fmt[48] = {}, zero[24] = {}, cap[96] = {};
    const char* const* choices = nullptr;
    int nChoices = 0;
};
struct RowPage {
    int n = 0;
    RowX rows[kMaxRows];
    K2MuiChangeFn fn = nullptr;
    void* user = nullptr;
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
    bool gear = false;         // AddGear: provider-drawn gear button (placed in the preview pane's top-right corner, art filled once the textures exist)
    bool gearFilled = false;   // both gear images were set on the button (or the attempt was given up)
    int gearTries = 0;         // fill attempts while the textures were not resolvable yet
    bool userHidden = false;   // SetItemShown(false): stays hidden, takes no hit
    Target tg;
    char text[96] = {};
    char desc[192] = {};
};

struct McState {
    uintptr_t panel = 0;
    int vanillaCount = 0;
    ItemX root, back, title;
    ItemX list[kListMax];
    int listTok[kListMax] = {};       // registry token shown by each list button (rebound when the list scrolls)
    int nList = 0;                    // list buttons made (min(entries, kListMax))
    int nEntries = 0;                 // registered entries at bind time (> kListMax = the list scrolls)
    int entTok[mui::kMaxEntries] = {};   // registry tokens at bind time, in list order (the list scrolls over this snapshot)
    bool built[mui::kMaxEntries] = {};
    bool pageMade[mui::kMaxSlots] = {};          // entry slots, then modal slots (mui::ModalTable)
    char pageTitle[mui::kMaxSlots][64] = {};
    mui::PageTable pt;
    ItemX items[mui::kMaxSlots][mui::kMaxItems];
    bool gearMade = false;                       // some page has an AddGear button: embtex::heal() runs every tick
    mui::ModalTable md;                          // modal pages (NewModal): owner entry per modal, the open one
    Rect modalBox[mui::kMaxModals] = {};         // NewModal box (w > 0: the modal's rows lay out in it instead of the column pane)
    mui::PageStack ps;
    mui::HoverTracker hov;
    mui::ClickTracker clk, clkBack, clkGear;   // clkGear: the page's gear while its modal is open (a click there closes the modal)
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
    bool prevEsc = false;      // Esc key state last tick (edge = one page back)
    DWORD leaveAt = 0;         // Back released on LIST: go to MAIN at this tick count (0 = none)
    Rect col[kColumnN] = {};   // vanilla column rects, measured BEFORE the reflow (PaneRect COLUMN)
    bool colOk = false;
    Rect descOrig = {0, 0, 0, 0};   // LB_DESC rect as the layout made it (measured at McBind, before any describe or split touched it)
    bool descOrigOk = false;
    int split[mui::kMaxEntries] = {};   // SetDescSplit permille per entry page (clamped; 0 = none)
    bool descSplitOn = false;           // the shrunk rect is currently asserted on LB_DESC (so the original must be put back)
    int descSplitPage = -1;
    int descReasserts = 0;              // times a describe or tick had to write the shrunk rect again (log cap)
    RowPage rp[mui::kMaxSlots];
    unsigned char itemRow[mui::kMaxSlots][mui::kMaxItems] = {};    // row index + 1 of an item that belongs to a row, 0 = none
    unsigned char itemPart[mui::kMaxSlots][mui::kMaxItems] = {};   // mui::RowPart: 0 toggle / caption, 1 "<", 2 ">", 3 slider frame, 4 wedge, 5 mark
    editkit::Repeat rep = {0, 0, 0};   // hold-repeat of the arrow under the press
    int repItem = -1;                  // item id (on the shown entry page) the press went down on, -1 = none
    // ---- scrolling (MCF2b): one window per option-row entry page (sw[k]) and one for the LIST page (lsw)
    mui::ScrollWindow sw[mui::kMaxSlots];
    mui::ScrollWindow lsw;
    ItemX sb[mui::kMaxSlots][kBarCtl];       // entry / modal page scroll bar (made at build when a page has more rows than fit)
    bool sbMade[mui::kMaxSlots] = {};
    ItemX lsb[kBarCtl];                      // LIST-page scroll bar
    bool lsbMade = false;
    editkit::Repeat barRep = {0, 0, 0};      // hold-repeat of the bar's up / down buttons
    int barHeld = -1;                        // kBarUp / kBarDown while that button is pressed, -1 = none
    bool dragging = false;                   // the thumb is being dragged (tickResetDue + wheel guard)
    int dragGrab = 0;                        // cursor y minus thumb top at the press
    int wheelRem = 0;                        // wheel units not yet a whole notch
    int scrollLogs = 0;                      // SCROLL lines logged (cap 100)
    int rowLogs = 0;                   // ROW + SLIDER lines logged (shared cap 200)
    mui::SliderDrag sd;                // the slider being held (one at a time); active only while the LMB is down on a shown entry / modal page
    int sdSlot = -1;                   // page slot of the dragged row
    int sdAsked = 0;                   // last value the handler was asked for in this drag (mui::sliderAskHandler)
    bool sliderMade = false;           // some page has a slider row: embtex::heal() runs every tick
    int sliderArt = 0;                 // 0 not tried, 1 textures ensured (fills pending / done), 2 art unavailable (fallback look for every slider)
    int gapLogs = 0;                   // TICKGAP lines logged (cap 32)
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
        else if (!_stricmp(k, "arrow_icons")) g_cfg.arrowIcons = on;
        else if (!_stricmp(k, "selftest")) g_cfg.selftest = atoi(v);
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

// ---------------------------------------------------------------------------------------------------- description split
// Entry pages can opt in (SetDescSplit) to a shrunk LB_DESC: the bottom part of the ORIGINAL rect, with MUI_PANE_PREVIEW above it. The
// vanilla describe routine rewrites LB_DESC's rect on every call, so the shrunk rect is written again after every describe and once per
// tick while the page is shown, and the original rect is put back when the page is left. Math: mui::splitDesc (MenuUiLogic.h).
mui::Rect toMui(const Rect& r) { return mui::Rect{r.x, r.y, r.w, r.h}; }

// LB_DESC rect as the layout made it: the McBind cache; else a live measure now (cached, so it is only as good as the first one).
bool descOriginal(Rect* out) {
    McState& e = g_mc;
    if (!e.descOrigOk && e.panel) {
        Rect r = {0, 0, 0, 0};
        if (ctlRect(listCtl(e.panel, kIdxDesc), &r)) { e.descOrig = r; e.descOrigOk = true; }
    }
    if (!e.descOrigOk) return false;
    *out = e.descOrig;
    return true;
}

// The split of the shown entry page (0 = none, or no entry page shown). Needs the first tick to have run (items appended).
int shownSplit() {
    const McState& e = g_mc;
    if (!e.appended || e.ps.state != mui::ENTRY || e.ps.entry < 0 || e.ps.entry >= mui::kMaxEntries) return 0;
    return e.split[e.ps.entry];
}

void restoreDescRect(const char* why) {
    McState& e = g_mc;
    if (!e.descSplitOn) return;
    e.descSplitOn = false;
    Rect o = {0, 0, 0, 0};
    const uintptr_t c = e.panel ? listCtl(e.panel, kIdxDesc) : 0;
    if (c && descOriginal(&o)) guikit::setRect(c, o.x, o.y, o.w, o.h);
    KP_LOG(chMain, "DESCSPLIT restore page=%d (%s) orig=(%d,%d,%d,%d)", e.descSplitPage, why, o.x, o.y, o.w, o.h);
    e.descSplitPage = -1;
}

// Makes LB_DESC match the shown page: the shrunk rect on a page with a split (written only when it differs), the original rect once the
// page that had one is left. Cheap enough for every tick; logs only on a state change (one line per page shown / restored).
void syncDescSplit(const char* why) {
    McState& e = g_mc;
    if (!e.panel) return;
    const int want = shownSplit();
    if (want == 0) { restoreDescRect(why); return; }
    Rect o = {0, 0, 0, 0};
    const uintptr_t c = listCtl(e.panel, kIdxDesc);
    if (!c || !descOriginal(&o)) return;
    const mui::DescSplit ds = mui::splitDesc(toMui(o), want);
    int x = 0, y = 0, w = 0, h = 0;
    const bool have = guikit::getExtent(c, &x, &y, &w, &h);
    const bool differs = !have || x != ds.desc.x || y != ds.desc.y || w != ds.desc.w || h != ds.desc.h;
    if (differs) guikit::setRect(c, ds.desc.x, ds.desc.y, ds.desc.w, ds.desc.h);
    const int page = 2 + e.ps.entry;
    if (!e.descSplitOn || e.descSplitPage != page) {
        e.descSplitOn = true;
        e.descSplitPage = page;
        KP_LOG(chMain, "DESCSPLIT apply page=%d permille=%d (%s) orig=(%d,%d,%d,%d) desc=(%d,%d,%d,%d) preview=(%d,%d,%d,%d) was=(%d,%d,%d,%d)", page, want, why,
               o.x, o.y, o.w, o.h, ds.desc.x, ds.desc.y, ds.desc.w, ds.desc.h, ds.preview.x, ds.preview.y, ds.preview.w, ds.preview.h, x, y, w, h);
    } else if (differs && e.descReasserts < 8) {
        ++e.descReasserts;
        KP_LOG(chMain, "DESCSPLIT reassert n=%d (%s) page=%d was=(%d,%d,%d,%d)", e.descReasserts, why, page, x, y, w, h);
    }
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
    syncDescSplit("describe");   // 8A2320 rewrote LB_DESC's rect: put the shrunk rect of a split page back
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
        case MUI_PANE_PREVIEW: {
            // The split of the entry being built, else of the shown entry page; no split = the whole original description rect.
            Rect orig = {0, 0, 0, 0};
            if (!descOriginal(&orig)) return false;
            const int k = (g_inBuild && g_buildSlot >= 0) ? g_buildSlot : (g_mc.ps.state == mui::ENTRY ? g_mc.ps.entry : -1);
            const int pm = (k >= 0 && k < mui::kMaxEntries) ? g_mc.split[k] : 0;
            const mui::DescSplit ds = mui::splitDesc(toMui(orig), pm);
            *out = Rect{ds.preview.x, ds.preview.y, ds.preview.w, ds.preview.h};
            return out->w > 0 && out->h > 0;
        }
        case MUI_PANE_TITLE: return ctlRect(listCtl(P, kIdxTitle), out);
        case MUI_PANE_BACK: return ctlRect(listCtl(P, kIdxExit), out);
        case MUI_PANE_VIEW:
        case MUI_PANE_STATUS: {
            Rect desc = {0, 0, 0, 0};
            // While a split shrinks LB_DESC, the view / status panes still follow the ORIGINAL rect.
            if (!(g_mc.descSplitOn && descOriginal(&desc)) && !ctlRect(listCtl(P, kIdxDesc), &desc)) return false;
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

// Scroll bar width for a column pane (see "scroll bars" below).
int barWidth(const Rect& col) {
    const int w = col.h * kBarPctOfCol / 100;
    return w < kBarMinPx ? kBarMinPx : w;
}

// ---------------------------------------------------------------------------------------------------- item helpers
template <typename F>
void eachOwn(F f) {
    McState& e = g_mc;
    f(e.root);
    for (int i = 0; i < e.nList; ++i) f(e.list[i]);
    f(e.back);
    f(e.title);
    for (int k = 0; k < mui::kMaxSlots; ++k)
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) f(e.items[k][i]);
    if (e.lsbMade)
        for (int i = 0; i < kBarCtl; ++i) f(e.lsb[i]);
    for (int k = 0; k < mui::kMaxSlots; ++k)
        if (e.sbMade[k])
            for (int i = 0; i < kBarCtl; ++i) f(e.sb[k][i]);
}

// The item the API names, or null (page not an entry or modal page, item out of range, or no panel bound).
ItemX* itemAt(uint32_t page, int32_t item) {
    if (page < 2 || page >= 2 + static_cast<uint32_t>(mui::kMaxSlots) || item < 0) return nullptr;
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
    return 2 + g_mc.md.shownSlot(g_mc.ps);   // the open modal's page while one is open
}

// The slot (entry slot, or modal slot while a modal is open) whose controls are on screen and take input; -1 outside an entry page.
int shownSlot() { return g_mc.md.shownSlot(g_mc.ps); }

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
    for (int k = 0; k < mui::kMaxSlots; ++k)
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) appendItem(e.items[k][i], "item");
    if (e.lsbMade)
        for (int i = 0; i < kBarCtl; ++i) appendItem(e.lsb[i], "listbar");
    for (int k = 0; k < mui::kMaxSlots; ++k)
        if (e.sbMade[k])
            for (int i = 0; i < kBarCtl; ++i) appendItem(e.sb[k][i], "bar");

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
    if (colOk) {
        Rect pane = {0, 0, 0, 0};
        const int bw = (e.lsbMade && e.nEntries > kListMax && paneRect(P, MUI_PANE_COLUMN, &pane)) ? barWidth(pane) : 0;
        for (int i = 0; i < e.nList; ++i) {
            Rect lr = col[i];
            if (bw > 0) lr.w -= bw + mui::kRowGap;   // room for the scroll bar on the right
            addTarget(e, e.list[i].ctl, lr);
        }
    }
    if (xOk) addTarget(e, e.back.ctl, exitR);
    if (tOk) addTarget(e, e.title.ctl, titleR);
    applyTargets(e, true);

    // Entry page items: the rects the entries set during build().
    for (int k = 0; k < mui::kMaxSlots; ++k)
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
    for (int k = 0; k < mui::kMaxSlots; ++k)
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) applyItemText(e.items[k][i]);

    if (e.lsbMade)
        for (int i = 0; i < kBarCtl; ++i) { applyItemText(e.lsb[i]); setShown(e.lsb[i], false); }
    for (int k = 0; k < mui::kMaxSlots; ++k)
        if (e.sbMade[k])
            for (int i = 0; i < kBarCtl; ++i) { applyItemText(e.sb[k][i]); setShown(e.sb[k][i], false); }
    // Clones start hidden + click-through. MAIN shows only the root.
    setShown(e.root, true);
    for (int i = 0; i < e.nList; ++i) setShown(e.list[i], false);
    setShown(e.back, false);
    setShown(e.title, false);
    for (int k = 0; k < mui::kMaxSlots; ++k)
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) setShown(e.items[k][i], false);
    logRect("root", e.root.ctl);
    logRect("back", e.back.ctl);
    logRect("title", e.title.ctl);
    if (e.nList > 0) logRect("list0", e.list[0].ctl);
}

// ---------------------------------------------------------------------------------------------------- option rows
RowX* rowAt(uint32_t page, int32_t row) {
    if (page < 2 || page >= 2 + static_cast<uint32_t>(mui::kMaxSlots) || row < 0) return nullptr;
    RowPage& rp = g_mc.rp[page - 2];
    return row < rp.n ? &rp.rows[row] : nullptr;
}

// Slot k (an entry page, or a modal page) has its controls on screen now: with a modal open that is the modal's slot, not its parent's.
bool entryShown(int k) { return g_mc.appended && k >= 0 && shownSlot() == k; }

void sliderPlaceParts(int k, RowX& r);   // slider rows (below): wedge + mark rects for the current value

// Rewrites the caption control of a row (override text, else mui::rowFormat). A disabled toggle gets " (locked)" (it has no arrows to hide).
// A slider row also moves its wedge and mark to the new value (every value path ends here: SetValue, a handler result, SetRowRange, a drag).
void rowCaption(int k, RowX& r) {
    if (r.item[0] < 0) return;
    char buf[sizeof(ItemX::text)];
    if (r.cap[0]) copyStr(buf, sizeof buf, r.cap);
    else (void)mui::rowFormat(buf, sizeof buf, r.label, r.cfg, r.hasFmt ? r.fmt : nullptr, r.choices, r.nChoices, r.hasZero ? r.zero : nullptr);
    if (r.cfg.kind == mui::RK_TOGGLE && (r.cfg.flags & mui::RF_DISABLED)) {
        const size_t n = strlen(buf);
        if (n + 10 < sizeof buf) strcat(buf, " (locked)");
    }
    ItemX& it = g_mc.items[k][r.item[0]];
    copyStr(it.text, sizeof it.text, buf);
    it.hasText = true;
    if (g_mc.appended) applyItemText(it);
    if (r.cfg.kind == mui::RK_SLIDER) sliderPlaceParts(k, r);
}

// Puts a control we own on a rect (resets its drift counters).
void placeItem(ItemX& it, const mui::Rect& r) {
    McState& e = g_mc;
    it.tg = Target();
    it.tg.ctl = it.ctl;
    it.tg.r = Rect{r.x, r.y, r.w, r.h};
    it.hasRect = true;
    if (e.appended && it.ctl && !it.unlinked) applyOne(e, it.tg, true);
}

void setPartRect(int k, int itemId, const mui::Rect& r) {
    if (itemId < 0) return;
    placeItem(g_mc.items[k][itemId], r);
}

// ---- slider rows (MCF5)
// A slider row = caption label + wedge + frame + mark labels in ONE row slot (mui::sliderGeom). Item ids are added in draw order (later items draw over
// earlier ones): caption, wedge, frame (the outline and the hit area), mark. RowX.item[] indexes: [0] caption, [1] frame, [2] wedge, [3] mark.
constexpr int kSliderArtMaxTries = 600;   // art fill attempts (one engine image lookup per image per try) before the plain look stays for good
static_assert(static_cast<int>(hse::sliderart_detail::kSliderTexCount) + static_cast<int>(gearbtn::detail::kEditTexCount) <= embtex::MaxTex,
              "mod-config's embedded textures (slider art + gear art) must fit embtex::MaxTex");

void embedLog(const char* fmt, ...) {   // embtex::g_log sink
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    KP_LOG(chMain, "EMBED %s", buf);
}

int sliderWedgeW(const RowX& r) { return mui::sliderWedgeFor(r.cfg, r.track, r.cfg.value).w; }

// Puts the wedge and the mark of a laid-out slider row (slot >= 0, track set) on their rects for the current value. The wedge is 0 px wide at the
// minimum: it is parked 1 px wide and hidden there (a label cannot be 0 wide).
void sliderPlaceParts(int k, RowX& r) {
    if (r.cfg.kind != mui::RK_SLIDER || r.slot < 0 || r.item[2] < 0 || r.item[3] < 0) return;
    // wedge and thumb stay inside the frame stroke; the wedge ends at the thumb centre (the wedge texture keeps clear of the stroke vertically)
    mui::Rect w = mui::sliderWedgeFor(r.cfg, r.track, r.cfg.value);
    const bool wedgeOn = w.w > 0;
    if (w.w < 1) w.w = 1;
    setPartRect(k, r.item[2], w);
    setPartRect(k, r.item[3], mui::sliderThumbFor(r.cfg, r.track, r.cfg.value));
    if (entryShown(k)) setShown(g_mc.items[k][r.item[2]], wedgeOn);
}

void sliderFallbackLog(const char* why, int k, int row) {
    static volatile LONG s_fb = 0;
    if (InterlockedIncrement(&s_fb) <= 1) KP_LOG(chMain, "SLIDER art fallback (%s) page=%d row=%d: caption + plain track, drag still works", why, 2 + k, row);
}

// The first slider row of a panel registers the art textures (EmbedTex, idempotent, Steam only). A failure leaves every slider on the plain look.
void sliderArtInit() {
    McState& e = g_mc;
    e.sliderMade = true;
    if (e.sliderArt) return;
    const bool ok = hse::sliderArtEnsure();
    e.sliderArt = ok ? 1 : 2;
    KP_LOG(chMain, "SLIDER art ensure=%d textures=%d gear=%d max=%d", ok ? 1 : 0, hse::sliderArtTextureCount(), gearbtn::textureCount(), embtex::MaxTex);
    if (!ok) sliderFallbackLog("ensure failed", -1, -1);
}

// Plain look: the mark label shows a "|" so the position is still visible; the wedge and the frame stay empty.
void sliderGiveUp(int k, int row, RowX& r, const char* why) {
    r.artDone = true;
    sliderFallbackLog(why, k, row);
    if (r.item[3] < 0) return;
    ItemX& mk = g_mc.items[k][r.item[3]];
    copyStr(mk.text, sizeof mk.text, "|");
    mk.hasText = true;
    applyItemText(mk);
}

// Called every tick for the shown page: fills the art of each slider row once the textures resolve (they appear a few ticks after ensure).
void sliderArtTick(int k) {
    McState& e = g_mc;
    if (!e.sliderMade || k < 0 || k >= mui::kMaxSlots) return;
    RowPage& rp = e.rp[k];
    for (int i = 0; i < rp.n; ++i) {
        RowX& r = rp.rows[i];
        if (r.cfg.kind != mui::RK_SLIDER || r.artDone) continue;
        ItemX& fr = e.items[k][r.item[1]];
        ItemX& wd = e.items[k][r.item[2]];
        ItemX& mk = e.items[k][r.item[3]];
        if (e.sliderArt != 1) { sliderGiveUp(k, i, r, "art unavailable"); continue; }
        if (!fr.ctl || !wd.ctl || !mk.ctl || fr.unlinked || wd.unlinked || mk.unlinked) { sliderGiveUp(k, i, r, "control missing"); continue; }
        if (++r.artTries > kSliderArtMaxTries) { sliderGiveUp(k, i, r, "textures never resolved"); continue; }
        if (!hse::sliderArtImage(hse::SA_Frame) || !hse::sliderArtImage(hse::SA_Wedge) || !hse::sliderArtImage(hse::SA_Mark)) continue;   // retry next tick
        r.artDone = true;
        int n = 0;
        if (hse::sliderArtFill(fr.ctl, hse::SA_Frame)) { ++n; (void)guikit::setFillStyle(fr.ctl, 2); }
        if (hse::sliderArtFill(wd.ctl, hse::SA_Wedge)) { ++n; (void)guikit::setFillStyle(wd.ctl, 2); }
        const bool markOk = hse::sliderArtFill(mk.ctl, hse::SA_Mark);
        if (markOk) { ++n; (void)guikit::setFillStyle(mk.ctl, 2); }
        static volatile LONG s_ok = 0;
        if (InterlockedIncrement(&s_ok) <= 4) KP_LOG(chMain, "SLIDER art applied page=%d row=%d parts=%d/3 after %d tries", 2 + k, i, n, r.artTries);
        if (!markOk) sliderGiveUp(k, i, r, "mark fill refused");
        else if (n < 3) sliderFallbackLog("fill refused", k, i);
    }
}

void releaseRowCtl(ItemX& it) {
    if (!it.ctl) return;
    if (it.isButton) guibutton::releaseUnlinkedButton(it.ctl);
    else guikit::releaseUnlinkedLabel(it.ctl, kLabelVtable);
    it.ctl = 0;
}

// ---------------------------------------------------------------------------------------------------- scroll bars (MCF2b)
// A scrolling page (option rows past the 8 that fit, or more mod entries than list buttons) gets a bar on the right edge of the COLUMN pane:
// an up button on top, a down button at the bottom, and a thumb button (no text) riding between them. The rows / list buttons shrink by the
// bar width + one gap. Input: the two buttons (press + hold-repeat), the thumb (drag), a click on the empty track (page jump), the mouse wheel.
struct BarGeom {
    mui::Rect up, down, track;
};

BarGeom barGeom(const Rect& col) {
    const int bw = barWidth(col);
    const int bx = col.x + col.w - bw;
    const int g = mui::kRowGap / 3;
    BarGeom b;
    b.up = mui::Rect{bx, col.y, bw, bw};
    b.down = mui::Rect{bx, col.y + col.h - bw, bw, bw};
    b.track = mui::Rect{bx, col.y + bw + g, bw, col.h - 2 * bw - 2 * g};
    return b;
}

bool makeBar(ItemX* bar, const char* tag) {
    static const char* const kTxt[kBarCtl] = {"^", "v", ""};
    static const char* const kName[kBarCtl] = {"up", "down", "thumb"};
    for (int i = 0; i < kBarCtl; ++i) {
        char what[40];
        snprintf(what, sizeof what, "bar_%s_%s", tag, kName[i]);
        bar[i] = ItemX();
        bar[i].ctl = cloneEntry(g_mc.panel, "BTN_GAMEPLAY", what, &bar[i].isButton);
        if (!bar[i].ctl) {
            for (int q = 0; q < i; ++q) releaseRowCtl(bar[q]);
            return false;
        }
        copyStr(bar[i].text, sizeof bar[i].text, kTxt[i]);
        bar[i].hasText = true;
    }
    return true;
}

// Moves the bar's three controls: arrows at the ends, the thumb where sw says. Does not change visibility.
void layoutBar(ItemX* bar, const mui::ScrollWindow& sw, const Rect& col) {
    const BarGeom g = barGeom(col);
    placeItem(bar[kBarUp], g.up);
    placeItem(bar[kBarDown], g.down);
    placeItem(bar[kBarThumb], sw.thumb(g.track));
}

void showBar(ItemX* bar, bool on) {
    for (int i = 0; i < kBarCtl; ++i) setShown(bar[i], on);
}

// Rows area of page k's column: the pane minus the bar when the page scrolls.
Rect rowsArea(int k, const Rect& col) {
    Rect r = col;
    if (g_mc.sbMade[k] && g_mc.sw[k].needBar()) r.w -= barWidth(col) + mui::kRowGap;
    return r;
}

// The pane page k's rows (and scroll bar) lay out in: the COLUMN pane, or a modal's own NewModal box.
bool slotPane(int k, Rect* col) {
    McState& e = g_mc;
    if (mui::ModalTable::isModalSlot(k) && e.md.valid(mui::ModalTable::indexOf(k))) {
        const Rect& b = e.modalBox[mui::ModalTable::indexOf(k)];
        if (b.w > 0 && b.h > 0) { *col = b; return true; }
    }
    return paneRect(e.panel, MUI_PANE_COLUMN, col);
}

// Puts every visible row of page k on its slot of the COLUMN pane (hidden rows take no slot). Rows past the scroll window take no slot either:
// slot = visible index - offset, so showRows hides them (hidden, never only moved: hit testing needs the visibility bit).
void layoutRows(int k) {
    McState& e = g_mc;
    RowPage& rp = e.rp[k];
    if (rp.n == 0 || !e.panel) return;
    Rect col = {0, 0, 0, 0};
    if (!slotPane(k, &col)) {
        static volatile LONG s_nc = 0;
        if (InterlockedIncrement(&s_nc) <= 4) KP_LOG(chMain, "ROWS layout: column pane unavailable (page %d)", 2 + k);
        return;
    }
    mui::ScrollWindow& sw = e.sw[k];
    int total = 0;
    for (int i = 0; i < rp.n; ++i)
        if (!(rp.rows[i].cfg.flags & mui::RF_HIDDEN)) ++total;
    sw.total = total;
    sw.visible = mui::kRowsVisible;
    sw.clamp();
    const mui::Rect c = toMui(rowsArea(k, col));
    int vi = 0;
    for (int i = 0; i < rp.n; ++i) {
        RowX& r = rp.rows[i];
        if (r.cfg.flags & mui::RF_HIDDEN) { r.slot = -1; continue; }
        r.slot = sw.slotOf(vi++);
        if (r.slot < 0) continue;
        const mui::Rect slotR = mui::rowSlotRect(c, r.slot);
        if (r.cfg.kind == mui::RK_SLIDER) {
            const mui::SliderGeom sg = mui::sliderGeom(slotR);
            r.track = sg.track;
            setPartRect(k, r.item[0], sg.caption);
            setPartRect(k, r.item[1], sg.track);
            sliderPlaceParts(k, r);   // wedge + mark for the current value
            continue;
        }
        const mui::RowGeom g = mui::rowGeom(slotR, r.cfg.kind);
        if (r.cfg.kind == mui::RK_TOGGLE) {
            setPartRect(k, r.item[0], g.whole);
        } else {
            setPartRect(k, r.item[0], g.label);
            setPartRect(k, r.item[1], g.left);
            setPartRect(k, r.item[2], g.right);
        }
    }
    if (e.sbMade[k]) layoutBar(e.sb[k], sw, col);
}

// Shows / hides the controls of page k's rows: hidden or unslotted rows are hidden, a disabled stepper keeps its caption but loses its arrows
// (a disabled slider keeps all four parts; it just takes no grab).
void showRows(int k) {
    McState& e = g_mc;
    RowPage& rp = e.rp[k];
    for (int i = 0; i < rp.n; ++i) {
        RowX& r = rp.rows[i];
        const bool vis = r.slot >= 0;
        const bool slider = r.cfg.kind == mui::RK_SLIDER;
        for (int p = 0; p < 4; ++p) {
            if (r.item[p] < 0) continue;
            const bool arrow = !slider && p > 0;
            bool on = vis && !(arrow && (r.cfg.flags & mui::RF_DISABLED));
            if (slider && p == 2) on = on && sliderWedgeW(r) > 0;   // the wedge is hidden at the minimum
            setShown(e.items[k][r.item[p]], on);
        }
    }
}

void rowsRelayout(int k) {
    layoutRows(k);
    if (entryShown(k)) {
        showRows(k);
        if (g_mc.sbMade[k]) showBar(g_mc.sb[k], g_mc.sw[k].needBar());
    }
}

// Asks the page's change handler about `req` (old = the stored value), stores its FINAL value (clamped; no handler = req), redraws the caption and
// logs through the shared capped ROW / SLIDER log. Returns the stored value.
int rowCommit(int k, int row, int old, int req, int dir) {
    McState& e = g_mc;
    RowPage& rp = e.rp[k];
    const K2MuiChangeFn fn = rp.fn;
    int32_t fin = 0;
    if (fn) fin = fn(static_cast<uint32_t>(2 + k), row, req, dir, rp.user);
    RowX& r2 = rp.rows[row];   // the handler may have changed this row (SetValue / SetRowState): decide on its current state
    const int nv = mui::rowApplyHandlerResult(r2.cfg, req, fin, fn != nullptr);
    r2.cfg.value = nv;
    rowCaption(k, r2);
    if (++e.rowLogs <= 200) {
        if (r2.cfg.kind == mui::RK_SLIDER) KP_LOG(chMain, "SLIDER page=%d row=%d req=%d final=%d", 2 + k, row, req, nv);
        else KP_LOG(chMain, "ROW page=%d row=%d old=%d req=%d final=%d handler=%d", 2 + k, row, old, req, nv, fn ? 1 : 0);
    }
    return nv;
}

// One accepted click / arrow step on a row: stepper moves by dir, a toggle flips (dir 0). The handler's return is the FINAL value.
void rowChange(int k, int row, int dir) {
    McState& e = g_mc;
    RowPage& rp = e.rp[k];
    if (row < 0 || row >= rp.n) return;
    RowX& r = rp.rows[row];
    if (r.cfg.flags & (mui::RF_DISABLED | mui::RF_HIDDEN)) return;
    if (r.cfg.kind == mui::RK_SLIDER) return;   // sliders change through sliderCommit (a drag), never by an arrow or a click
    const int old = r.cfg.value;
    const int req = mui::rowStep(r.cfg, old, r.cfg.kind == mui::RK_TOGGLE ? 0 : dir);
    if (req == old && r.cfg.kind == mui::RK_STEPPER) return;   // already at an end: nothing to ask the handler
    (void)rowCommit(k, row, old, req, r.cfg.kind == mui::RK_TOGGLE ? 0 : dir);
}

// Arrow press with hold-repeat (first step on the press, repeats after editkit::kRepeatDelayMs while the cursor stays on that arrow).
void rowArrowInput(int k, bool down, int hit, DWORD now) {
    McState& e = g_mc;
    const bool press = down && !e.prevDown;
    int row = -1, part = 0;
    if (hit >= 0 && hit < mui::kMaxItems && e.itemRow[k][hit]) {
        row = e.itemRow[k][hit] - 1;
        part = e.itemPart[k][hit];
    }
    if (press) e.repItem = (row >= 0 && mui::rowPartIsArrow(part)) ? hit : -1;
    if (!down || (e.repItem >= 0 && hit != e.repItem)) e.repItem = -1;   // released, or the cursor left the arrow: stop for this press
    const bool held = e.repItem >= 0;
    int n = editkit::repeatTick(e.rep, held, now);
    if (!held) return;
    const int rr = e.itemRow[k][e.repItem] - 1, pp = e.itemPart[k][e.repItem];
    for (; n > 0; --n) rowChange(k, rr, pp == 1 ? -1 : 1);
}

// ---- slider input (MCF5)
void sliderEndDrag(const char* why) {
    McState& e = g_mc;
    if (!e.sd.active) return;
    (void)why;
    mui::sliderEnd(e.sd);
    e.sdSlot = -1;
    e.dragging = false;   // the thumb drag cannot be active at the same time (both begin on a press edge)
}

// One slider value change (a grab or a drag move): the handler decides, the final value is stored. Equal to the stored value: nothing to ask.
void sliderCommit(int k, int row, int req) {
    McState& e = g_mc;
    if (k < 0 || k >= mui::kMaxSlots || row < 0 || row >= e.rp[k].n) return;
    RowX& r = e.rp[k].rows[row];
    if (!mui::sliderGrabbable(r.cfg.kind, r.cfg.flags)) return;
    const int old = r.cfg.value;
    req = mui::rowClamp(r.cfg, req);
    if (req == old) return;
    (void)rowCommit(k, row, old, req, mui::sliderChangeDir(old, req));
}

// Per tick, after the scroll bar. A press EDGE on a live slider's track grabs it (returns true: the press is consumed and never becomes a click);
// while the LMB stays down the value follows the pointer (the handler is asked only when the snapped value changes); the LMB going up, the page
// or panel hiding, the row going away or being disabled ends the drag. McState::dragging is true while a drag is active.
bool sliderTick(bool down, bool haveCur, int x, int y) {
    McState& e = g_mc;
    const int k = (e.appended && e.ps.state == mui::ENTRY) ? shownSlot() : -1;
    if (e.sd.active) {
        RowX* r = (k >= 0 && k == e.sdSlot && e.sd.row >= 0 && e.sd.row < e.rp[k].n) ? &e.rp[k].rows[e.sd.row] : nullptr;
        if (!down || !r || r->slot < 0 || !mui::sliderGrabbable(r->cfg.kind, r->cfg.flags)) { sliderEndDrag(down ? "row gone" : "release"); return false; }
        if (haveCur) {
            bool moved = false;
            const int row = e.sd.row;
            const int v = mui::sliderMove(e.sd, r->cfg, mui::sliderTravelOf(r->track), x, &moved);
            if (mui::sliderAskHandler(moved, v, e.sdAsked)) {
                e.sdAsked = v;
                sliderCommit(k, row, v);
            }
            if (e.sd.active && e.sdSlot == k) mui::sliderSetStored(e.sd, e.rp[k].rows[row].cfg.value);   // a snapped or rejected value is what later moves compare with
        }
        if (e.sd.active) e.dragging = true;
        return false;
    }
    if (!down || e.prevDown || !haveCur || k < 0) return false;
    RowPage& rp = e.rp[k];
    for (int i = 0; i < rp.n; ++i) {
        RowX& r = rp.rows[i];
        if (r.slot < 0 || !mui::sliderGrabbable(r.cfg.kind, r.cfg.flags) || !mui::rectHas(r.track, x, y)) continue;
        const int v = mui::sliderBegin(e.sd, i, r.cfg, mui::sliderTravelOf(r.track), x);
        e.sdSlot = k;
        e.sdAsked = v;
        e.dragging = true;
        sliderCommit(k, i, v);
        if (e.sd.active) mui::sliderSetStored(e.sd, rp.rows[i].cfg.value);
        return true;
    }
    return false;
}

// LIST page: the 8 vanilla column buttons show entries [lsw.offset, lsw.offset + 8) of the bind-time snapshot.
void rebindList() {
    McState& e = g_mc;
    for (int i = 0; i < e.nList; ++i) {
        const int idx = e.lsw.offset + i;
        const int tok = idx < e.nEntries ? e.entTok[idx] : 0;
        e.listTok[i] = tok;
        copyStr(e.list[i].text, sizeof e.list[i].text, entLive(tok - 1) ? g_ent[tok - 1].name : "");
        e.list[i].hasText = true;
        applyItemText(e.list[i]);
    }
}

void listBarSync() {
    McState& e = g_mc;
    e.lsw.total = e.nEntries;
    e.lsw.visible = kListMax;
    e.lsw.clamp();
    Rect col = {0, 0, 0, 0};
    if (e.lsbMade && paneRect(e.panel, MUI_PANE_COLUMN, &col)) layoutBar(e.lsb, e.lsw, col);
}

// The mouse wheel arrives from the gui-input provider (optional; without it the bar buttons and the thumb still work).
void wheelSubscribeOnce() {
    static bool ok = false;
    if (ok) return;
    guikit::g_gkInput.poll();
    if (guikit::g_gkInput.connected && guikit::g_gkInput.wheel_subscribe(guikit::detail::guikitWheelCb, nullptr) > 0) {
        ok = true;
        KP_LOG(chMain, "WHEEL subscribed through gui-input");
    }
}

bool wheelDragGuard() { return g_mc.dragging; }

// One tick of scroll input for the shown page (list or option rows). Returns true when the position changed (and re-lays the page out).
bool scrollTick(bool down, bool haveCur, int x, int y, DWORD now) {
    McState& e = g_mc;
    mui::ScrollWindow* sw = nullptr;
    ItemX* bar = nullptr;
    int k = -1;
    if (e.ps.state == mui::LIST && e.lsbMade && e.lsw.needBar()) { sw = &e.lsw; bar = e.lsb; }
    else if (e.ps.state == mui::ENTRY && shownSlot() >= 0 && e.sbMade[shownSlot()] && e.sw[shownSlot()].needBar()) {
        k = shownSlot();
        sw = &e.sw[k];
        bar = e.sb[k];
    }
    Rect col = {0, 0, 0, 0};
    if (!sw || !(k >= 0 ? slotPane(k, &col) : paneRect(e.panel, MUI_PANE_COLUMN, &col))) {
        guikit::g_wheel.clearRect(0);   // nothing scrolls here: the wheel goes to the game
        e.barHeld = -1;
        e.dragging = false;
        e.wheelRem = 0;                 // a part notch left over from another page does not carry into the next one
        (void)editkit::repeatTick(e.barRep, false, now);
        return false;
    }
    wheelSubscribeOnce();
    guikit::g_wheel.scoped = true;
    guikit::g_wheel.dragGuard = wheelDragGuard;
    guikit::g_wheel.touch(now);
    guikit::g_wheel.setRect(0, col.x, col.y, col.w, col.h);
    e.wheelRem += guikit::g_wheel.take();
    const bool slideLock = e.sd.active;   // a held slider owns the pointer: no wheel scroll, no bar press, no thumb drag until it is let go
    const int notches = slideLock ? 0 : e.wheelRem / kWheelNotch;
    if (slideLock) e.wheelRem = 0;
    e.wheelRem -= notches * kWheelNotch;
    bool changed = false;
    const char* src = "";
    if (notches != 0 && sw->wheel(notches)) { changed = true; src = "wheel"; }
    const BarGeom g = barGeom(col);
    const bool press = down && !e.prevDown && haveCur && !slideLock;
    if (press) {
        const mui::Rect th = sw->thumb(g.track);
        if (overCtl(bar[kBarUp], x, y)) e.barHeld = kBarUp;
        else if (overCtl(bar[kBarDown], x, y)) e.barHeld = kBarDown;
        else if (overCtl(bar[kBarThumb], x, y)) { e.dragging = true; e.dragGrab = y - th.y; }
        else if (mui::rectHas(g.track, x, y) && sw->setOffset(sw->offset + (y < th.y ? -sw->visible : sw->visible))) { changed = true; src = "track"; }
    }
    if (!down) {
        e.barHeld = -1;
        e.dragging = false;
    } else if (e.barHeld >= 0 && !(haveCur && overCtl(bar[e.barHeld], x, y))) {
        e.barHeld = -1;   // the cursor left the button: stop for this press
    }
    int n = editkit::repeatTick(e.barRep, e.barHeld >= 0, now);
    if (e.barHeld >= 0 && !slideLock)
        for (; n > 0; --n)
            if (sw->wheel(e.barHeld == kBarUp ? 1 : -1)) { changed = true; src = "button"; }
    if (!slideLock && e.dragging && haveCur && sw->setOffset(sw->offsetFromThumbTop(y, e.dragGrab, g.track))) { changed = true; src = "drag"; }
    for (int i = 0; i < kBarCtl; ++i)
        if (bar[i].ctl && bar[i].hasRect && !bar[i].unlinked) applyOne(e, bar[i].tg, false);
    if (changed) {
        sliderEndDrag("scroll");   // a scroll that moves the page ends a slider drag
        if (k >= 0) rowsRelayout(k);
        else { rebindList(); layoutBar(e.lsb, e.lsw, col); }
        e.hov = mui::HoverTracker();   // the hover description follows what is under the cursor now
        e.clk.pressItem = -1;          // (down is kept, so the release clicks nothing) a press that began before the scroll (wheel under a held LMB) must not click whatever slid under it
        if (++e.scrollLogs <= 100) KP_LOG(chMain, "SCROLL page=%d off=%d total=%d src=%s", curPageId(), sw->offset, sw->total, src);
    }
    return changed;
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

// ---- shared gear (AddGear) and modal pages (NewModal)
constexpr int kGearMaxTries = 600;   // fill attempts (one engine image lookup per image per try) before the text fallback stays for good

// Top-right corner of the preview pane (the description pane without a split), sized from the row slot height.
void placeGear(ItemX& it) {
    McState& e = g_mc;
    Rect pane = {0, 0, 0, 0}, col = {0, 0, 0, 0};
    if (!e.panel || (!paneRect(e.panel, MUI_PANE_PREVIEW, &pane) && !paneRect(e.panel, MUI_PANE_DESC, &pane)) || !paneRect(e.panel, MUI_PANE_COLUMN, &col)) return;
    const int slotH = (col.h * mui::kRowSlotRef) / mui::kRowRefH;
    const mui::Rect g = mui::gearButtonRect(toMui(pane), slotH);
    if (g.w > 0) placeItem(it, g);
}

// Gives the gear button its two images once the embedded textures resolve; until then (or on a label clone) the "*" text stays.
void gearFill(ItemX& it) {
    if (!it.gear || it.gearFilled || !it.ctl || it.unlinked) return;
    if (!it.isButton || ++it.gearTries > kGearMaxTries) {
        it.gearFilled = true;
        if (!it.isButton || it.gearTries > kGearMaxTries) KP_LOG(chMain, "GEAR art not applied (button=%d tries=%d): the text fallback stays", it.isButton ? 1 : 0, it.gearTries);
        return;
    }
    void* normal = guikit::imageForRetry(gearbtn::artName(false));
    void* hover = guikit::imageForRetry(gearbtn::artName(true));
    if (!normal || !hover) return;
    it.gearFilled = true;
    if (!guikit::setButtonFill(it.ctl, normal, hover)) { KP_LOG(chMain, "GEAR setButtonFill refused: the text fallback stays"); return; }
    (void)guikit::setButtonFillStyle(it.ctl, 2);   // stretch the 64x64 glyph over the square
    it.text[0] = 0;
    it.hasText = true;
    applyItemText(it);
    KP_LOG(chMain, "GEAR art applied after %d tries", it.gearTries);
}

// Shows slot k's controls the way ENTER(ENTRY) does: rows on their column slots, rects re-applied, everything shown, then unslotted rows hidden again.
// underModal: k is the parent of an open modal: only its AddCustom controls (a preview must keep drawing) and its gear (the toggle that closes the modal) stay;
// rows, buttons and labels hide.
void showSlot(int k, bool underModal) {
    McState& e = g_mc;
    layoutRows(k);
    for (int i = 0; i < e.pt.itemCount(2 + k); ++i) {
        ItemX& it = e.items[k][i];
        if (it.gear) {
            placeGear(it);
        } else if (it.ctl && it.hasRect && !it.unlinked) {
            it.tg.ctl = it.ctl;
            it.tg.applied = 0;
            it.tg.gaveUp = false;
            applyOne(e, it.tg, true);
        }
        const bool keep = !underModal || it.gear || e.pt.pages[2 + k].items[i].kind == mui::CUSTOM;
        setShown(it, keep && !it.userHidden);
    }
    if (!underModal) showRows(k);   // after the show-all loop: hidden / unslotted rows and the arrows of disabled rows go back to hidden
    if (e.sbMade[k]) showBar(e.sb[k], !underModal && e.sw[k].needBar());
}

void hideSlot(int k) {
    McState& e = g_mc;
    for (int i = 0; i < e.pt.itemCount(2 + k); ++i) setShown(e.items[k][i], false);
    if (e.sbMade[k]) showBar(e.sb[k], false);
}

void resetPageInput() {
    McState& e = g_mc;
    sliderEndDrag("page input reset");
    e.repItem = -1;
    e.hov = mui::HoverTracker();
    e.clk = mui::ClickTracker();
    e.clkBack = mui::ClickTracker();
    e.clkGear = mui::ClickTracker();
}

// The on-screen half of opening / closing modal m (md.open is already updated): swap the parent's controls for the modal's and the title text.
void modalApply(int m, bool open) {
    McState& e = g_mc;
    const int s = mui::ModalTable::slotOf(m), k = e.md.parent[m];
    if (!e.appended || k < 0) return;
    if (open) {
        showSlot(k, true);
        e.sw[s].offset = 0;
        showSlot(s, false);
        if (e.title.ctl) guikit::setText(e.title.ctl, e.pageTitle[s][0] ? e.pageTitle[s] : e.pageTitle[k]);
    } else {
        hideSlot(s);
        showSlot(k, false);
        if (e.title.ctl) guikit::setText(e.title.ctl, e.pageTitle[k]);
    }
    resetPageInput();
    KP_LOG(chMain, "MODAL %s page=%d parent=%d rows=%d items=%d", open ? "open" : "close", 2 + s, 2 + k, e.rp[s].n, e.pt.itemCount(2 + s));
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
                if (e.lsbMade) showBar(e.lsb, false);
                setShown(e.back, false);
                setShown(e.title, false);
            } else {
                const bool cleared = clearFocus(e.panel);
                if (e.title.ctl) guikit::setText(e.title.ctl, kRootText);
                listBarSync();
                rebindList();
                for (int i = 0; i < e.nList; ++i) setShown(e.list[i], true);
                if (e.lsbMade) showBar(e.lsb, e.lsw.needBar());
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
                syncDescSplit("leave");   // ps already moved off the entry page: the original LB_DESC rect goes back
                if (e.md.open >= 0 && e.md.parent[e.md.open] == k) {   // leaving with a modal open (Esc, tick gap, panel hidden): it closes with the page
                    hideSlot(mui::ModalTable::slotOf(e.md.open));
                    e.md.clear();
                }
                hideSlot(k);
                setShown(e.back, false);
                setShown(e.title, false);
            } else {
                const bool cleared = clearFocus(e.panel);
                if (e.title.ctl) guikit::setText(e.title.ctl, e.pageTitle[k]);
                e.md.clear();         // a page always opens without its modal
                e.sw[k].offset = 0;   // a page always opens at its top
                showSlot(k, false);   // option rows take their column slots (the party / values may have changed since the last visit)
                setShown(e.back, true);
                setShown(e.title, true);
                KP_LOG(chMain, "ENTRY enter id=%s page=%d items=%d focusCleared=%d", entLive(k) ? g_ent[k].id : "?", 2 + k, e.pt.itemCount(2 + k), cleared ? 1 : 0);
                syncDescSplit("enter");   // before the entry's enter(), so its first SetDesc already lands in the shrunk pane
                if (entLive(k) && g_ent[k].enter) g_ent[k].enter(static_cast<uint32_t>(2 + k), g_ent[k].user);
            }
            break;
        }
    }
}

void pageChange(int from, const mui::PageEvents& ev, const char* why) {
    McState& e = g_mc;
    sliderEndDrag("page change");
    for (int i = 0; i < ev.n; ++i) applyEvent(ev.ev[i], why);
    e.leaveAt = 0;
    e.repItem = -1;
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
    const int k = shownSlot();
    if (k < 0) return -1;
    // Two passes: real controls first, AddCustom controls (a preview's label pool covers the whole pane, gear included) only when nothing else is hit.
    for (int pass = 0; pass < 2; ++pass) {
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) {
            const ItemX& it = e.items[k][i];
            const int kind = e.pt.pages[2 + k].items[i].kind;
            if (kind == mui::LABEL || (kind == mui::CUSTOM) != (pass == 1)) continue;
            if (it.userHidden || !mui::rowPartHittable(e.itemPart[k][i])) continue;   // the wedge and mark sit on the frame: the frame is the hit
            if (overCtl(it, x, y)) return i;
        }
    }
    return -1;
}

// While a modal is open: the owning page's gear (it stays on screen and a click on it closes the modal, like Back), else null.
ItemX* modalGear() {
    McState& e = g_mc;
    if (e.md.open < 0 || e.ps.state != mui::ENTRY) return nullptr;
    const int k = e.md.parent[e.md.open];
    if (k < 0) return nullptr;
    for (int i = 0; i < e.pt.itemCount(2 + k); ++i)
        if (e.items[k][i].gear && !e.items[k][i].userHidden) return &e.items[k][i];
    return nullptr;
}

ItemX* shownItem(int item) {
    McState& e = g_mc;
    if (item < 0) return nullptr;
    if (e.ps.state == mui::MAIN) return item == 0 ? &e.root : nullptr;
    if (e.ps.state == mui::LIST) return item < e.nList ? &e.list[item] : nullptr;
    const int k = shownSlot();
    if (k < 0 || item >= e.pt.itemCount(2 + k)) return nullptr;
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
    const bool own = page == static_cast<uint32_t>(2 + g_buildSlot);
    const bool ownModal = page >= 2 + static_cast<uint32_t>(mui::kMaxEntries) && page < 2 + static_cast<uint32_t>(mui::kMaxSlots) &&
                          g_mc.md.parentOfSlot(static_cast<int>(page) - 2) == g_buildSlot;   // a modal NewModal made for the entry being built
    if (!own && !ownModal) {
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

int32_t KPATCH_CALL muiSetDescSplit(uint32_t page, int32_t previewPermille) {
    McState& e = g_mc;
    if (!e.panel || previewPermille < 0 || page < 2 || page >= 2 + static_cast<uint32_t>(mui::kMaxEntries)) return 0;
    const int k = static_cast<int>(page) - 2;
    if (!entLive(k) || !e.pageMade[k]) return 0;
    saferead::beginScope();
    e.split[k] = mui::clampSplit(previewPermille);
    if (e.appended && e.ps.state == mui::ENTRY && e.ps.entry == k) syncDescSplit("set");   // the page is shown now: act at once
    return 1;
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

// ---- option rows (K2MenuUiApi slots 12..18)
bool rowPageGate(uint32_t page, const char* what) {
    if (!buildGate(page, what)) return false;
    if (!g_mc.pageMade[static_cast<int>(page) - 2]) { KP_LOG(chMain, "MUI refused: %s before NewPage", what); return false; }
    return true;
}

// The ABI values are copied straight into mui::RowCfg (kind, flags): pin the two enum sets to each other.
static_assert(MUI_ROW_TOGGLE == mui::RK_TOGGLE && MUI_ROW_STEPPER == mui::RK_STEPPER && MUI_ROW_SLIDER == mui::RK_SLIDER, "MUI_ROW_* == mui::RK_*");
static_assert(MUI_RF_WRAP == mui::RF_WRAP && MUI_RF_RESTART == mui::RF_RESTART && MUI_RF_DISABLED == mui::RF_DISABLED && MUI_RF_HIDDEN == mui::RF_HIDDEN,
              "MUI_RF_* == mui::RF_*");
// The v1 spec ends at zeroText. A later, longer spec must keep accepting v1 callers: compare struct_size with this floor (not with a
// grown sizeof) and read any newer field only when struct_size covers it.
constexpr size_t kRowSpecV1 = offsetof(K2MuiRowSpec, zeroText) + sizeof(static_cast<const K2MuiRowSpec*>(nullptr)->zeroText);
static_assert(kRowSpecV1 == sizeof(K2MuiRowSpec), "K2MuiRowSpec grew: gate the new fields on struct_size");
static_assert(sizeof(void*) != 4 || (sizeof(K2MuiRowSpec) == 52 && offsetof(K2MuiRowSpec, fmt) == 36 && offsetof(K2MuiRowSpec, zeroText) == 48),
              "K2MuiRowSpec 32-bit layout (13 x 4 bytes, no padding under pack(8))");

int32_t KPATCH_CALL muiAddRow(uint32_t page, const K2MuiRowSpec* sp) {
    if (!sp || sp->struct_size < kRowSpecV1 || !mui::rowKindValid(sp->kind)) return -1;   // an unknown kind (3+) is refused: callers fall back to a stepper
    if (!rowPageGate(page, "AddRow")) return -1;
    McState& e = g_mc;
    const int k = static_cast<int>(page) - 2;
    RowPage& rp = e.rp[k];
    const bool toggle = sp->kind == MUI_ROW_TOGGLE;
    const bool slider = sp->kind == MUI_ROW_SLIDER;
    const int parts = mui::rowPartCount(sp->kind);
    if (rp.n >= kMaxRows || e.pt.itemCount(static_cast<int>(page)) + parts > mui::kMaxItems) {
        KP_LOG(chMain, "MUI refused: AddRow page %u is full (rows=%d items=%d)", static_cast<unsigned>(page), rp.n, e.pt.itemCount(static_cast<int>(page)));
        return -1;
    }
    // The 9th row of any visibility gets the page its scroll bar: a row added hidden can be shown later (SetRowState), and a page whose
    // 9th visible row only appears then would have no bar to reach it. The bar stays hidden while <= 8 rows are visible (needBar).
    const bool needsBar = rp.n >= mui::kRowsVisible;
    RowX r;
    r.used = true;
    r.cfg.kind = sp->kind;
    r.cfg.minV = toggle ? 0 : sp->min;
    r.cfg.maxV = toggle ? 1 : sp->max;
    r.cfg.step = sp->step < 1 ? 1 : sp->step;
    r.cfg.flags = sp->flags & (MUI_RF_WRAP | MUI_RF_RESTART | MUI_RF_DISABLED | MUI_RF_HIDDEN);
    if (slider) r.cfg.flags &= ~MUI_RF_WRAP;   // a slider has no ends to wrap
    r.choices = sp->choices;
    r.nChoices = sp->choices && sp->nChoices > 0 ? sp->nChoices : 0;
    if (!toggle && r.nChoices > 0) { r.cfg.minV = 0; r.cfg.maxV = r.nChoices - 1; }
    r.cfg.value = mui::rowClamp(r.cfg, sp->value);
    copyStr(r.label, sizeof r.label, sp->label);
    copyStr(r.desc, sizeof r.desc, sp->desc);
    if (sp->fmt && sp->fmt[0]) { copyStr(r.fmt, sizeof r.fmt, sp->fmt); r.hasFmt = true; }
    if (sp->zeroText && sp->zeroText[0]) { copyStr(r.zero, sizeof r.zero, sp->zeroText); r.hasZero = true; }
    // Create every control first so a failed clone leaves no half row on the page.
    // Slider controls are all label clones, created in draw order (later items draw over earlier ones): caption, wedge, frame, mark.
    static const unsigned char kSlItem[4] = {0, 2, 1, 3};   // creation order -> RowX.item[] index (item[1] frame, item[2] wedge)
    static const unsigned char kSlPart[4] = {mui::RP_CAPTION, mui::RP_WEDGE, mui::RP_FRAME, mui::RP_MARK};
    static const mui::ItemKind kSlKind[4] = {mui::BUTTON, mui::LABEL, mui::BUTTON, mui::LABEL};   // only the caption and the frame are hit-testable
    if (slider) sliderArtInit();
    ItemX ctl[4];
    char what[40];
    for (int p = 0; p < parts; ++p) {
        snprintf(what, sizeof what, "row_%u_%d_%d", static_cast<unsigned>(page), rp.n, p);
        if (slider || (!toggle && p == 0)) { ctl[p].ctl = cloneLabel(e.panel, what); ctl[p].isButton = false; }
        else ctl[p].ctl = cloneEntry(e.panel, "BTN_GAMEPLAY", what, &ctl[p].isButton);
        if (!ctl[p].ctl) {
            for (int q = 0; q < p; ++q) releaseRowCtl(ctl[q]);
            KP_LOG(chMain, "MUI refused: AddRow control %d could not be cloned", p);
            return -1;
        }
        ctl[p].hasText = true;
        copyStr(ctl[p].desc, sizeof ctl[p].desc, sp->desc);
    }
    if (needsBar && !e.sbMade[k]) {
        char tag[16];
        snprintf(tag, sizeof tag, "p%u", static_cast<unsigned>(page));
        if (!makeBar(e.sb[k], tag)) {
            for (int q = 0; q < parts; ++q) releaseRowCtl(ctl[q]);
            KP_LOG(chMain, "MUI refused: AddRow page %u needs a scroll bar and its controls could not be cloned", static_cast<unsigned>(page));
            return -1;
        }
        e.sbMade[k] = true;
    }
    copyStr(ctl[0].text, sizeof ctl[0].text, "");
    if (!toggle && !slider) { copyStr(ctl[1].text, sizeof ctl[1].text, "<"); copyStr(ctl[2].text, sizeof ctl[2].text, ">"); }
    for (int p = 0; p < parts; ++p) {
        const int id = addItemCommon(page, slider ? kSlKind[p] : mui::BUTTON, ctl[p], 0);   // the caption label is hit-testable too (hover shows the row description)
        if (id < 0) {   // cannot happen after the capacity check; keep the page consistent anyway
            for (int q = p; q < parts; ++q) releaseRowCtl(ctl[q]);
            return -1;
        }
        r.item[slider ? kSlItem[p] : p] = id;
        e.itemRow[k][id] = static_cast<unsigned char>(rp.n + 1);
        e.itemPart[k][id] = static_cast<unsigned char>(slider ? kSlPart[p] : p);
    }
    // Only the "<" ">" text path exists in 0.2.0-dev: arrow_icons is read and logged but not built yet (MCR1 decides whether it is needed).
    if (rp.n == 0) KP_LOG(chMain, "ARROWPATH text page=%u arrow_icons=%d (icons not built: text path used)", static_cast<unsigned>(page), g_cfg.arrowIcons ? 1 : 0);
    const int row = rp.n;
    rp.rows[rp.n++] = r;
    rowCaption(k, rp.rows[row]);
    return row;
}

void KPATCH_CALL muiSetChangeHandler(uint32_t page, K2MuiChangeFn fn, void* user) {
    if (page < 2 || page >= 2 + static_cast<uint32_t>(mui::kMaxSlots)) return;
    RowPage& rp = g_mc.rp[page - 2];
    rp.fn = fn;
    rp.user = user;
}

int32_t KPATCH_CALL muiGetValue(uint32_t page, int32_t row) {
    const RowX* r = rowAt(page, row);
    return r ? r->cfg.value : 0;
}

void KPATCH_CALL muiSetValue(uint32_t page, int32_t row, int32_t v) {
    RowX* r = rowAt(page, row);
    if (!r) return;
    saferead::beginScope();
    r->cfg.value = mui::rowClamp(r->cfg, v);
    rowCaption(static_cast<int>(page) - 2, *r);
}

void KPATCH_CALL muiSetRowText(uint32_t page, int32_t row, const char* text) {
    RowX* r = rowAt(page, row);
    if (!r) return;
    saferead::beginScope();
    copyStr(r->cap, sizeof r->cap, text);
    rowCaption(static_cast<int>(page) - 2, *r);
}

void KPATCH_CALL muiSetRowState(uint32_t page, int32_t row, int32_t flags) {
    RowX* r = rowAt(page, row);
    if (!r) return;
    saferead::beginScope();
    const int k = static_cast<int>(page) - 2;
    const int keep = r->cfg.flags & (mui::RF_WRAP | mui::RF_RESTART);
    r->cfg.flags = keep | (flags & (MUI_RF_DISABLED | MUI_RF_HIDDEN));
    rowCaption(k, *r);
    rowsRelayout(k);
}

void KPATCH_CALL muiSetRowRange(uint32_t page, int32_t row, int32_t minV, int32_t maxV) {
    RowX* r = rowAt(page, row);
    if (!r || r->cfg.kind == mui::RK_TOGGLE) return;   // steppers and sliders
    saferead::beginScope();
    if (r->nChoices > 0) {   // a choice stepper stores an index: keep the range inside [0, nChoices - 1] (a subset is fine)
        const int top = r->nChoices - 1;
        minV = minV < 0 ? 0 : (minV > top ? top : minV);
        maxV = maxV < 0 ? 0 : (maxV > top ? top : maxV);
    }
    r->cfg.minV = minV;
    r->cfg.maxV = maxV;
    r->cfg.value = mui::rowClamp(r->cfg, r->cfg.value);
    rowCaption(static_cast<int>(page) - 2, *r);
}

// ---- item visibility, modal pages and the shared gear (K2MenuUiApi slots 19..22)
void KPATCH_CALL muiSetItemShown(uint32_t page, int32_t item, int32_t shown) {
    McState& e = g_mc;
    ItemX* it = itemAt(page, item);
    if (!it) return;
    const int k = static_cast<int>(page) - 2;
    if (e.itemRow[k][item]) return;   // a row's controls follow the row (SetRowState)
    saferead::beginScope();
    it->userHidden = shown == 0;
    (void)e.pt.setVisible(static_cast<int>(page), item, shown != 0);
    if (!e.appended || !it->ctl || it->unlinked) return;
    const int sh = shownSlot();
    const bool underModal = e.md.open >= 0 && sh == mui::ModalTable::slotOf(e.md.open) && e.md.parent[e.md.open] == k && e.pt.pages[page].items[item].kind == mui::CUSTOM;
    if (sh == k || underModal) setShown(*it, shown != 0);
}

uint32_t KPATCH_CALL muiNewModal(uint32_t page, const K2MuiRect* box, const char* title) {
    if (!buildGate(page, "NewModal")) return 0;
    McState& e = g_mc;
    const int k = static_cast<int>(page) - 2;
    if (k != g_buildSlot) { KP_LOG(chMain, "MUI refused: NewModal needs the entry's own page, not page %u", static_cast<unsigned>(page)); return 0; }
    if (!e.pageMade[k]) { KP_LOG(chMain, "MUI refused: NewModal before NewPage"); return 0; }
    const int m = e.md.add(k);
    if (m < 0) { KP_LOG(chMain, "MUI refused: NewModal (all %d modals used)", mui::kMaxModals); return 0; }
    const int s = mui::ModalTable::slotOf(m);
    e.pageMade[s] = true;
    copyStr(e.pageTitle[s], sizeof e.pageTitle[s], title);
    e.modalBox[m] = (box && box->w > 0 && box->h > 0) ? Rect{box->x, box->y, box->w, box->h} : Rect{0, 0, 0, 0};
    KP_LOG(chMain, "MODAL made page=%d parent=%u box=%d", 2 + s, static_cast<unsigned>(page), e.modalBox[m].w > 0 ? 1 : 0);
    return static_cast<uint32_t>(2 + s);
}

void KPATCH_CALL muiSetModalOpen(uint32_t modalPage, int32_t open) {
    McState& e = g_mc;
    if (modalPage < 2 + static_cast<uint32_t>(mui::kMaxEntries) || modalPage >= 2 + static_cast<uint32_t>(mui::kMaxSlots)) return;
    const int m = mui::ModalTable::indexOf(static_cast<int>(modalPage) - 2);
    if (!e.md.valid(m)) return;
    saferead::beginScope();
    if (open != 0) {
        if (e.md.open >= 0 && e.md.open != m) {   // one modal at a time: the other one closes first
            const int prev = e.md.open;
            if (e.md.setOpen(prev, false, e.ps)) modalApply(prev, false);
        }
        if (e.md.setOpen(m, true, e.ps)) modalApply(m, true);
    } else if (e.md.setOpen(m, false, e.ps)) {
        modalApply(m, false);
    }
}

int32_t KPATCH_CALL muiAddGear(uint32_t page, const char* desc) {
    if (!buildGate(page, "AddGear")) return -1;
    McState& e = g_mc;
    const int k = static_cast<int>(page) - 2;
    if (k != g_buildSlot) { KP_LOG(chMain, "MUI refused: AddGear needs the entry's own page, not page %u", static_cast<unsigned>(page)); return -1; }
    if (!e.pageMade[k]) { KP_LOG(chMain, "MUI refused: AddGear before NewPage"); return -1; }
    if (e.pt.itemCount(static_cast<int>(page)) >= mui::kMaxItems) { KP_LOG(chMain, "MUI refused: page %u is full", static_cast<unsigned>(page)); return -1; }
    for (int i = 0; i < e.pt.itemCount(static_cast<int>(page)); ++i)
        if (e.items[k][i].gear) { KP_LOG(chMain, "MUI refused: page %u already has a gear", static_cast<unsigned>(page)); return -1; }
    (void)gearbtn::registerArt();   // idempotent, Steam only; the textures resolve a few ticks later (gearFill retries)
    char what[40];
    snprintf(what, sizeof what, "gear_%u", static_cast<unsigned>(page));
    ItemX it;
    it.ctl = cloneEntry(e.panel, "BTN_GAMEPLAY", what, &it.isButton);
    if (!it.ctl) return -1;
    copyStr(it.text, sizeof it.text, "*");   // shown until the gear art is applied (and for good on a label clone)
    it.hasText = true;
    copyStr(it.desc, sizeof it.desc, desc);
    it.gear = true;
    const int id = addItemCommon(page, mui::BUTTON, it, 0);
    if (id < 0) {
        if (it.isButton) guibutton::releaseUnlinkedButton(it.ctl);
        else guikit::releaseUnlinkedLabel(it.ctl, kLabelVtable);
        return -1;
    }
    e.gearMade = true;
    return id;
}


// ---------------------------------------------------------------------------------------------------- built-in self test (mod_config.ini selftest=1)
// Eight rows that cover every row feature (toggle, steppers, two sliders) plus a change handler that snaps and rejects; the MCR1 vehicle.
const char* const kStChoices[] = {"Off", "Slow", "Fast"};

int32_t KPATCH_CALL stChange(uint32_t page, int32_t row, int32_t requested, int32_t dir, void*) {
    int32_t fin = requested;
    if (row == 2) {   // 5..95 step 5: snap to the nearest multiple of 5, and reject 60 (stay where it was)
        fin = ((requested + 2) / 5) * 5;
        if (fin == 60) fin = requested - 5 * dir;
    } else if (row == 7) {   // the 5..95 step 5 slider: the same snap, and 60 is rejected (the slider keeps its stored value; dir is the sign of the move)
        fin = ((requested + 2) / 5) * 5;
        if (fin == 60) fin = muiGetValue(page, row);
    }
    KP_LOG(chMain, "SELFTEST change page=%u row=%d requested=%d dir=%d final=%d", static_cast<unsigned>(page), row, requested, dir, fin);
    return fin;
}

void KPATCH_CALL stBuild(void* panel, void*) {
    const uint32_t pg = muiNewPage(panel, "Mod-config self test");
    if (!pg) return;
    K2MuiRowSpec sp = {};
    sp.struct_size = sizeof sp;
    sp.kind = MUI_ROW_TOGGLE; sp.label = "Toggle"; sp.desc = "A plain on/off row. Click it to flip."; sp.value = 1;
    muiAddRow(pg, &sp);
    sp = K2MuiRowSpec(); sp.struct_size = sizeof sp;
    sp.kind = MUI_ROW_STEPPER; sp.label = "Speed"; sp.desc = "A wrapping list stepper: Off / Slow / Fast, then back to Off."; sp.min = 0; sp.max = 2; sp.step = 1; sp.value = 1;
    sp.flags = MUI_RF_WRAP; sp.choices = kStChoices; sp.nChoices = 3;
    muiAddRow(pg, &sp);
    sp = K2MuiRowSpec(); sp.struct_size = sizeof sp;
    sp.kind = MUI_ROW_STEPPER; sp.label = "Heal"; sp.desc = "5 to 95 in steps of 5. The handler snaps to a multiple of 5 (start 37 -> 40) and rejects 60."; sp.min = 5; sp.max = 95; sp.step = 5; sp.value = 37;
    sp.fmt = "Heal below %d%%";
    muiAddRow(pg, &sp);
    sp = K2MuiRowSpec(); sp.struct_size = sizeof sp;
    sp.kind = MUI_ROW_STEPPER; sp.label = "Keep"; sp.desc = "0 to 10, clamped at both ends. Zero reads Auto."; sp.min = 0; sp.max = 10; sp.step = 1; sp.value = 0;
    sp.zeroText = "Auto";
    muiAddRow(pg, &sp);
    sp = K2MuiRowSpec(); sp.struct_size = sizeof sp;
    sp.kind = MUI_ROW_TOGGLE; sp.label = "Disabled"; sp.desc = "A disabled row ignores clicks."; sp.value = 0; sp.flags = MUI_RF_DISABLED;
    muiAddRow(pg, &sp);
    sp = K2MuiRowSpec(); sp.struct_size = sizeof sp;
    sp.kind = MUI_ROW_TOGGLE; sp.label = "Needs restart"; sp.desc = "The caption gets (restart)."; sp.value = 0; sp.flags = MUI_RF_RESTART;
    muiAddRow(pg, &sp);
    sp = K2MuiRowSpec(); sp.struct_size = sizeof sp;
    sp.kind = MUI_ROW_SLIDER; sp.label = "Volume"; sp.desc = "A slider, 0 to 100 in steps of 1. Press the track to jump, drag to follow."; sp.min = 0; sp.max = 100; sp.step = 1; sp.value = 40;
    sp.fmt = "Volume %d%%";
    muiAddRow(pg, &sp);
    sp = K2MuiRowSpec(); sp.struct_size = sizeof sp;
    sp.kind = MUI_ROW_SLIDER; sp.label = "Snap"; sp.desc = "A slider, 5 to 95 in steps of 5. The handler snaps to a multiple of 5 and rejects 60 (the thumb stays put)."; sp.min = 5; sp.max = 95; sp.step = 5; sp.value = 35;
    sp.fmt = "Snap %d";
    muiAddRow(pg, &sp);
    if (g_cfg.selftest >= 2) {   // 26 rows in all (the 8 above plus 18 filler rows alternating steppers and toggles, one slider among them), so the page scrolls
        static const char* const kNames[18] = {"Row 07", "Row 08", "Row 09", "Row 10", "Row 11", "Row 12", "Row 13", "Row 14", "Row 15",
                                               "Row 16", "Row 17", "Row 18", "Row 19", "Row 20", "Row 21", "Row 22", "Row 23", "Row 24"};
        for (int i = 0; i < 18; ++i) {
            sp = K2MuiRowSpec(); sp.struct_size = sizeof sp;
            sp.label = kNames[i];
            sp.desc = "A scroll-test row. Its description should change when the page scrolls under the cursor.";
            if (i == 5) { sp.kind = MUI_ROW_SLIDER; sp.min = 0; sp.max = 100; sp.step = 5; sp.value = 50; }
            else if (i % 2 == 0) { sp.kind = MUI_ROW_TOGGLE; sp.value = 0; }
            else { sp.kind = MUI_ROW_STEPPER; sp.min = 0; sp.max = 9; sp.step = 1; sp.value = i / 2; }
            muiAddRow(pg, &sp);
        }
    }
    muiSetChangeHandler(pg, &stChange, nullptr);
}

void KPATCH_CALL stDummyBuild(void* panel, void*) {
    const uint32_t pg = muiNewPage(panel, "Dummy entry");
    if (!pg) return;
    K2MuiRowSpec sp = {};
    sp.struct_size = sizeof sp;
    sp.kind = MUI_ROW_TOGGLE; sp.label = "Dummy toggle"; sp.desc = "A filler entry that makes the mod list longer than 8."; sp.value = 1;
    muiAddRow(pg, &sp);
}

const K2MenuUiApi g_muiApi = {sizeof(K2MenuUiApi), &muiNewPage, &muiAddButton, &muiAddLabel, &muiAddCustom, &muiClicked,
                              &muiSetText,         &muiSetRect, &muiSetDesc,   &muiPaneRect,  &muiControl, &muiSetDescSplit,
                              &muiAddRow,          &muiSetChangeHandler, &muiGetValue, &muiSetValue, &muiSetRowText, &muiSetRowState, &muiSetRowRange,
                              &muiSetItemShown,    &muiNewModal,         &muiSetModalOpen,  &muiAddGear};

}  // namespace

namespace {
// ---- Esc steps back one page (0.4.0-dev; slot corrected 0.4.1-dev). The Options panel's vtable slot 15 (gameaddr::OptionsInGameKeyHandler, thiscall(this, eventId, state), RET 8) is its input-event
// handler: case 0x2a calls FUN_004122a0(1) (closes the whole in-game GUI), 0x28/0x2d/0x2e/0xdf are navigation/click. The slot is pointed at escKeyHook
// (a data patch of the shared vtable, so mod-config stays hot-swappable). On a non-MAIN page, Esc going down with event 0x2a performs the Back action
// and is swallowed, so the GUI stays open; on MAIN (and for every other event) the original runs, i.e. vanilla Esc behaviour. The first calls are
// logged (KEYEV) so the event id of Esc is confirmed from the log; the McTick Esc edge stays as the fallback (reset to MAIN) when the hook did not act.
constexpr int kVtKeySlot = 15;   // 0x8A2070 is stored at 0x9A7058 = 0x9A701C + 15*4 (slot 14 is 0x40E770, the per-frame update)
constexpr uint32_t kKeyHandlerOrig = static_cast<uint32_t>(gameaddr::OptionsInGameKeyHandler);
constexpr int kEscEventId = 0x2a;       // the close case in the handler switch
constexpr int kEscKeyDownId = 0x28;     // what Esc actually delivers (KEYEV log 10-21: id=28 state=1 with Esc held)
using KeyFn = void(__fastcall*)(void* self, void* edx, int id, int state);
KeyFn g_keyOrig = nullptr;
bool g_escHooked = false;
DWORD g_escConsumedAt = 0;

bool escHeld() { return guikit::g_gk.gameForeground && guikit::g_gk.gameForeground() && (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0; }

// Vanilla Esc plays GUI sound 5 (FUN_007CA060 close branch -> PlayGuiSound(5), REA 20261021_options-esc-sound); the hook swallows that path, so play it here.
constexpr int kEscSoundIdx = 5;
void playEscSound() {
    uint32_t mgr = 0;
    const bool ok = guikit::g_gk.playSound && guikit::g_gk.guiSoundMgrAddr && guikit::detail::rd32(guikit::g_gk.guiSoundMgrAddr, 0, &mgr) && mgr;
    static volatile LONG s_n = 0;
    if (InterlockedIncrement(&s_n) <= 8) KP_LOG(chInput, "ESCSOUND idx=%d ok=%d", kEscSoundIdx, ok ? 1 : 0);
    if (ok) guikit::g_gk.playSound(reinterpret_cast<void*>(static_cast<uintptr_t>(mgr)), kEscSoundIdx);
}

// Esc on a mod-config page: close an open modal, else go back one page. False on MAIN (the game handles Esc).
bool escStepBack() {
    McState& e = g_mc;
    if (!e.appended || e.ps.state == mui::MAIN) return false;
    playEscSound();
    const int from = curPageId();
    const int om = e.md.open;
    if (e.ps.state == mui::ENTRY && e.md.closeFirst()) modalApply(om, false);   // an open modal closes first, the page stays
    else pageChange(from, e.ps.back(), "esc key");
    return true;
}

void __fastcall escKeyHook(void* self, void* edx, int id, int state) {
    static volatile LONG s_n = 0;
    const bool held = escHeld();
    if (InterlockedIncrement(&s_n) <= 120)
        KP_LOG(chInput, "KEYEV id=%02x state=%d page=%d esc=%d", id, state, g_mc.appended ? curPageId() : -1, held ? 1 : 0);
    if ((id == kEscKeyDownId || id == kEscEventId) && state != 0 && held && escStepBack()) {
        g_escConsumedAt = GetTickCount();
        return;   // swallowed: the GUI stays open
    }
    g_keyOrig(self, edx, id, state);
}

void installEscHook() {
    if (g_escHooked) return;
    const uintptr_t at = kOptionsVtable + kVtKeySlot * 4;
    const uint32_t hook = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&escKeyHook));
    uint8_t expect[4], repl[4], seen[4];
    memcpy(expect, &kKeyHandlerOrig, 4);
    memcpy(repl, &hook, 4);
    const int r = codepatch::writeChecked(at, expect, repl, 4, seen);
    if (r != 1) { KP_LOG(chMain, "ESCHOOK vtable slot %d not patched r=%d seen=%02x%02x%02x%02x", kVtKeySlot, r, seen[3], seen[2], seen[1], seen[0]); return; }
    g_keyOrig = reinterpret_cast<KeyFn>(kKeyHandlerOrig);
    g_escHooked = true;
    KP_LOG(chMain, "ESCHOOK vtable slot %d -> %08x (orig %08x)", kVtKeySlot, hook, kKeyHandlerOrig);
}
}  // namespace

// H-O1 8A16FE: Options ctor, layout open, all vanilla InitControl calls done; panel = [ebp-0xAC].
extern "C" void __cdecl McBind(int panelArg) {
    installEscHook();
    static bool s_cfgRead = false;
    if (!s_cfgRead) {   // lazily, on the first Options panel (KPatchInit must not do file I/O)
        s_cfgRead = true;
        readConfig();
        KP_LOG(chMain, "CONFIG enabled=%d debug=%d button_clone=%d reflow=%d desc=%d free_labels=%d arrow_icons=%d selftest=%d table=%s", g_enabled ? 1 : 0,
               g_debug ? 1 : 0, g_cfg.buttonClone ? 1 : 0, g_cfg.reflow ? 1 : 0, g_cfg.desc ? 1 : 0, g_cfg.freeLabels ? 1 : 0, g_cfg.arrowIcons ? 1 : 0,
               g_cfg.selftest, k2addr::get() ? "ok" : "none");
        if (g_enabled && g_cfg.selftest > 0) {   // the built-in test entry (rows of every kind); registered like any other mod's entry
            static const K2ModConfigEntry st = {sizeof(K2ModConfigEntry), "mc-selftest", "Mod-config self test", "Option-row self test: toggle, steppers, sliders, snap / reject, disabled, restart. For play checks only.",
                                                nullptr, &stBuild, nullptr, nullptr, nullptr, nullptr};
            mcRegister(&st);
            if (g_cfg.selftest >= 2) {   // six filler entries so the list page has more than 8 buttons
                static const char* const kDId[6] = {"mc-dummy1", "mc-dummy2", "mc-dummy3", "mc-dummy4", "mc-dummy5", "mc-dummy6"};
                static const char* const kDName[6] = {"Dummy 1", "Dummy 2", "Dummy 3", "Dummy 4", "Dummy 5", "Dummy 6"};
                static K2ModConfigEntry de[6];
                for (int i = 0; i < 6; ++i) {
                    de[i] = K2ModConfigEntry();
                    de[i].struct_size = sizeof(K2ModConfigEntry);
                    de[i].id = kDId[i];
                    de[i].name = kDName[i];
                    de[i].desc = "Self-test filler entry (selftest=2).";
                    de[i].build = &stDummyBuild;
                    mcRegister(&de[i]);
                }
            }
        }
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
    { Rect d0 = {0, 0, 0, 0}; (void)descOriginal(&d0); }   // cache the layout's LB_DESC rect now, before any describe rewrites it
    // Buttons and labels from the vanilla layout (open until StopLoadFromLayout right after this hook), in this order:
    // root button, list buttons, Back, title.
    e.root.ctl = cloneEntry(P, "BTN_GAMEPLAY", "root", &e.root.isButton);
    e.nEntries = g_reg.count() < mui::kMaxEntries ? g_reg.count() : mui::kMaxEntries;
    for (int i = 0; i < e.nEntries; ++i) e.entTok[i] = g_reg.at(i);
    e.nList = e.nEntries < kListMax ? e.nEntries : kListMax;
    e.lsw.total = e.nEntries;
    e.lsw.visible = kListMax;
    for (int i = 0; i < e.nList; ++i) {
        const int tok = e.entTok[i];
        const int s = tok - 1;
        e.listTok[i] = tok;
        char what[32];
        snprintf(what, sizeof what, "list_%d", i);
        e.list[i].ctl = cloneEntry(P, "BTN_GAMEPLAY", what, &e.list[i].isButton);
        copyStr(e.list[i].text, sizeof e.list[i].text, entLive(s) ? g_ent[s].name : "");
        e.list[i].hasText = true;
    }
    if (e.nEntries > kListMax) {
        e.lsbMade = makeBar(e.lsb, "list");
        if (!e.lsbMade) KP_LOG(chMain, "CTOR list scroll bar clone failed: entries past %d are unreachable", kListMax);
    }
    e.back.ctl = cloneEntry(P, "BTN_EXIT", "back", &e.back.isButton);
    e.title.ctl = cloneLabel(P, "title");
    if (!e.root.ctl) {
        KP_LOG(chMain, "CTOR root button clone failed: entries are not built for this panel");
        return;
    }
    for (int i = 0; i < e.nEntries; ++i) {   // every registered entry builds its page, scrolled-out list entries included
        const int s = e.entTok[i] - 1;
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
    for (int k = 0; k < mui::kMaxSlots; ++k)
        for (int i = 0; i < e.pt.itemCount(2 + k); ++i) e.items[k][i].latched = false;
    const DWORD now = GetTickCount();
    const DWORD gap = e.lastTick ? now - e.lastTick : 0;
    e.lastTick = now;
    int flags48 = 0;
    (void)rd<int>(P, ctlFlagsOff(), &flags48);
    const bool down = lmbDown();
    static volatile LONG s_hd = 0;
    if ((flags48 != e.lastFlags48 || gap > kTickGapMs) && InterlockedIncrement(&s_hd) <= 32)
        KP_LOG(chMain, "HIDDENDRAW panel=%x flags48=%x (was %x) gap=%lums page=%d", static_cast<unsigned>(P), static_cast<unsigned>(flags48),
               static_cast<unsigned>(e.lastFlags48), static_cast<unsigned long>(gap), curPageId());
    e.lastFlags48 = flags48;
    // Esc closes the whole in-game GUI, but the hidden panel is not drawn, so a quick Esc, Esc (< kTickGapMs) reopened on the old mod page
    // (live 10-18). Act on the Esc key edge itself: any non-MAIN page goes back to MAIN the moment Esc goes down. Esc is still not swallowed.
    const bool escDown = escHeld();
    // With the key hook installed the hook owns Esc: the tick can run BEFORE the key event reaches it (live 10-21: page 4 reset to MAIN first, then
    // the hook saw MAIN and vanilla closed the menu), so this edge reset is the fallback for a failed hook only.
    if (escDown && !e.prevEsc && e.appended && e.ps.state != mui::MAIN && !g_escHooked)
        resetPages("esc key");
    e.prevEsc = escDown;
    {   // A long gap used to always mean "the panel was closed". While the panel is still marked shown (flags48 bit 0x40) a game hitch under a held
        // button looks the same, so only a very long gap with no button held counts then (mui::tickResetDue). Every decision past 500 ms is logged.
        const bool due = mui::tickResetDue(static_cast<unsigned>(gap), flags48, down, e.dragging);
        if (gap > kTickGapMs && ++e.gapLogs <= 32)
            KP_LOG(chMain, "TICKGAP gap=%lu flags48=%x lmb=%d reset=%d", static_cast<unsigned long>(gap), static_cast<unsigned>(flags48), down ? 1 : 0, due ? 1 : 0);
        if (due) resetPages("tick gap");
    }
    // Live logs 10-17: a shown Options panel draws with flags48 0x4d, an Esc-hidden one with 0x0d (still drawn, ~22 ms gaps). Bit 0x2 is
    // clear in BOTH states (it is only the CTOR-time "layout open" bit), so 0x40 is the shown bit. Hidden -> back to MAIN, nothing else.
    if (e.appended && !(flags48 & 0x40)) {   // never before the first build: a wrong bit can then cost the Esc reset, never the button
        if (e.ps.state != mui::MAIN) resetPages("panel hidden");
        return;
    }
    if (!e.appended) firstTick();
    applyTargets(e, false);
    syncDescSplit("tick");   // no-op unless a split page is shown (or was just left)
    if (e.gearMade || e.sliderMade) embtex::heal();   // refill the gear textures after a GL reset (idempotent, cheap)
    if (e.ps.state == mui::ENTRY && e.ps.entry >= 0 && e.ps.entry < mui::kMaxEntries) {
        const int sh = shownSlot();   // the open modal's slot while one is open; its parent's controls (a preview) keep their rects too
        if (sh >= 0) sliderArtTick(sh);   // slider art fills once the embedded textures resolve
        for (int pass = 0; pass < 2; ++pass) {
            const int k = pass == 0 ? sh : e.ps.entry;
            if (k < 0 || (pass == 1 && k == sh)) continue;
            for (int i = 0; i < e.pt.itemCount(2 + k); ++i) {
                ItemX& it = e.items[k][i];
                if (pass == 0) gearFill(it);
                if (it.ctl && it.hasRect && !it.unlinked) applyOne(e, it.tg, false);
            }
        }
    }

    int x = 0, y = 0;
    const bool cur = guikit::cursorEngine(&x, &y);
    // Clicks act on the RELEASE: LMB up over the control the press went down on. Not-foreground counts as up (GuiKit clickEdge rule).
    if (e.restorePending && !down) restoreVanilla("lmb up");
    const ItemX* gearUnder = cur ? modalGear() : nullptr;
    const bool overGear = gearUnder && overCtl(*gearUnder, x, y);   // the gear is not a control of the shown (modal) page: its click is handled apart below
    const int hit = (cur && !overGear) ? hitItem(x, y) : -1;
    const bool overBack = cur && e.ps.state != mui::MAIN && overCtl(e.back, x, y);
    hoverStep(overBack ? -1 : hit);
    if (shownSlot() >= 0 && e.rp[shownSlot()].n > 0) rowArrowInput(shownSlot(), down, overBack ? -1 : hit, now);
    (void)scrollTick(down, cur, x, y, now);
    const bool sliderGrab = sliderTick(down, cur, x, y);   // a press edge on a slider track: consumed here, never a click on whatever is under it
    if (down && !e.prevDown) {
        static volatile LONG s_in = 0;
        if (InterlockedIncrement(&s_in) <= 40)
            KP_LOG(chInput, "INPUTCLASS press cur=%d x=%d y=%d page=%d item=%d overBack=%d", cur ? 1 : 0, x, y, curPageId(), hit, overBack ? 1 : 0);
    }
    e.prevDown = down;
    const int clicked = e.clk.tick(down, (overBack || sliderGrab) ? -1 : hit);
    const int backClicked = e.clkBack.tick(down, overBack ? 0 : -1);
    if (e.clkGear.tick(down, overGear ? 0 : -1) >= 0 && e.md.open >= 0) {   // the gear of the modal's page: a second click closes the modal
        const int om = e.md.open;
        if (e.md.closeFirst()) {
            KP_LOG(chMain, "MODAL gear click closes page=%d", 2 + mui::ModalTable::slotOf(om));
            modalApply(om, false);
        }
    }
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
            const int k = shownSlot();
            if (k >= 0 && clicked < mui::kMaxItems && e.itemRow[k][clicked]) {
                // A row part: a toggle flips on its release; the arrows already stepped on their press (rowArrowInput); neither latches Clicked.
                const int row = e.itemRow[k][clicked] - 1;
                if (e.itemPart[k][clicked] == 0 && e.rp[k].rows[row].cfg.kind == mui::RK_TOGGLE) rowChange(k, row, 0);
            } else {
                ItemX* it = shownItem(clicked);
                if (it) it->latched = true;
            }
        }
    } else if (backClicked >= 0) {
        const int from = curPageId();
        if (e.ps.state == mui::ENTRY) {
            const int om = e.md.open;
            if (e.md.closeFirst()) modalApply(om, false);   // Back closes an open modal first, the page stays
            else pageChange(from, e.ps.back(), "back");
        } else if (e.ps.state == mui::LIST && e.leaveAt == 0) e.leaveAt = now + kLeaveDelayMs;
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
    guikit::g_wheel.clearRect(0);   // the wheel goes back to the game
    KP_LOG(chMain, "DTOR panel=%x page=%d appended=%d free_labels=%d", static_cast<unsigned>(P), curPageId(), e.appended ? 1 : 0, g_cfg.freeLabels ? 1 : 0);
    // (1) leave the shown page: only the entry's leave callback (the panel is going away, nothing to show or hide).
    if (e.ps.state == mui::ENTRY && e.ps.entry >= 0) callEntryLeave(e.ps.entry);
    restoreDescRect("destroy");
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
    embtex::g_log = embedLog;   // gear + slider art textures (EmbedTex) log through the main channel
    guikit::initGuiInput(api, "mod-config");   // optional: the wheel provider (k2.guiinput); a missing provider only means no wheel
    KP_LOG(chMain, "mod-config 0.4.4-dev loaded (Mod Configuration button under Gameplay; k2.modconfig@1 + k2.menuui@1; config mod_config.ini is read at the first Options panel: enabled debug button_clone reflow desc free_labels)");
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
