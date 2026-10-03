// LevelUpUi.h - M4 character-screen UI of Level-Up Override 0.3.1: Configure button (R6) with the R9 nudge, the single options page (OptionsLogic.h; WizardLogic.h supplies the Item type),
// the R11 status line. NOT a standalone header: LevelUpOverride.cpp includes it INSIDE its anonymous namespace after the gates (it uses the .cpp's
// tagConfig / evalStats / ptApply / ptRemove / g_presets / bankedLevels / logLine), and includes GuiKit.h + WizardLogic.h at file scope.
//
// Lifecycle per character-screen panel (CSWGuiInGameCharacter):
//   ctor hook 0x84D82C -> uiPanelCtor(panel): the layout GFF is still open, so the labels are created here from templates (addToList=0) and
//     stored in a record keyed by the panel pointer (4 records, round robin). Nothing is appended yet (Mod 14 binds its ids 69..150 later).
//   SetStats hook 0x84F221 (every frame while the screen is the updated panel) -> uiTick(panel, stats, realCanLevelUp, heldResult): the first
//     tick appends every control in z-order, then each tick places (scale from BTN_LEVELUP = control index 66), shows/hides, sets text, polls clicks.
// Templates: plain LBL_EXPERIENCE_STAT (green text, unit rect 60,449,108,18), framed LBL_XP_BACK (framed fill, 54,431,223,39).
// "Character screen is the active panel" = this tick runs: SetStats is called from CSWGuiInGameCharacter::Update 0x84FBB0 (the function Mod 14
// ticks on, CharEffectsPanel.cpp CharEffectsTick ~1162 + its hooks.toml), so a click is only evaluated while the panel is being updated, and
// guikit::clickEdge adds the foreground-window check (the same two conditions Mod 14's tickInput 969-973 uses: its tick + GetForegroundWindow).
// Layout units are the 800x600 GUI units of character_p.gui; Mod 14's panel is x 536-752, ours stays within x 54..456 (options page 54..300).

#include "OptionsLogic.h"   // also included at file scope by LevelUpOverride.cpp (pragma once -> no-op here)

// ---- pure text builders (unit-tested) -----------------------------------------------------------------------------------
// R11 status line. false = hidden (PC / not configurable, or unconfigured). held: "Holding N level(s) for <Jedi class> - <short>"
// (N = banked levels from XP; 0 = nothing banked yet); configured, not held: "Auto path: <short>" + " (instant)".
bool buildStatus(bool configurable, bool configured, bool held, int banked, int holdClass, const char* shortName, bool instant, char* out, size_t cap) {
    if (out && cap) out[0] = 0;
    if (!out || !cap || !configurable || !configured) return false;
    char cb[16];
    if (held && banked > 0) snprintf(out, cap, "Holding %d level%s for %s - %s", banked, banked == 1 ? "" : "s", className(holdClass, cb, sizeof cb), shortName ? shortName : "");
    else if (held) snprintf(out, cap, "Holding the next level for %s - %s", className(holdClass, cb, sizeof cb), shortName ? shortName : "");
    else snprintf(out, cap, "Auto path: %s%s", shortName ? shortName : "", instant ? " (instant)" : "");
    return true;
}
// Configure button text; *highlight = the R9 nudge (unconfigured while the real CanLevelUp says the character can level).
const char* cfgButtonText(bool configured, bool canLevelUp, bool* highlight) {
    *highlight = !configured && canLevelUp;
    return configured ? "Change path" : (*highlight ? "Plan level-ups" : "Configure");
}


// ---- controls -------------------------------------------------------------------------------------------------------------
constexpr unsigned S_Ui = 120;
constexpr int NumRows = opt::NumRows + 1, MaxUiPanels = 4, BtnLevelUpIndex = 66, UiText = 160;   // option rows: Convert, Role, Style, Alignment, Skills, Variant
constexpr uint32_t UiMagic = 0x4C554932;   // 'LUI2'
enum UiCtl { C_CFG, C_STATUS, C_BG0, C_BG1, C_BG2, C_TITLE, C_ROW0, C_BUILD = C_ROW0 + NumRows, C_INSTANT, C_BANK, C_REC, C_APPLY, C_VANILLA, C_CANCEL, C_N };
constexpr int NumBg = 3;
struct UiSpec { bool framed; int x, y, w, h; };   // layout units (800x600)
// Options page over the stats column only (live 2026-10-02: BTN_AUTO/BTN_LEVELUP, SLD_ALIGN x 310 and LBL_LIGHT/DARK x 304 draw ON TOP of anything
// placed in the 3D-character column, whatever the list order) -> x 54..300, y 134..470. Three stacked framed labels with the black fill = opaque.
constexpr int kRowY0 = 162, kRowStep = 30;   // option rows: y = kRowY0 + kRowStep * (index among the SHOWN rows)
const UiSpec kUiSpec[C_N] = {
    {true, 362, 372, 94, 26},                                                                            // cfgBtn, under BTN_LEVELUP [362,325,94,40]
    {false, 54, 474, 350, 20},                                                                           // status, below LBL_XP_BACK (bottom 470), left of LBL_BAR5 (x 407)
    {true, 54, 134, 246, 336}, {true, 54, 134, 246, 336}, {true, 54, 134, 246, 336},                     // backgrounds
    {false, 62, 138, 230, 20},                                                                           // title
    {true, 62, kRowY0, 230, 26}, {true, 62, kRowY0, 230, 26}, {true, 62, kRowY0, 230, 26},               // option rows (y recomputed per slot)
    {true, 62, kRowY0, 230, 26}, {true, 62, kRowY0, 230, 26}, {true, 62, kRowY0, 230, 26},
    {false, 62, 346, 230, 20},                                                                           // build line
    {true, 62, 370, 113, 26}, {true, 179, 370, 113, 26},                                                 // Instant, Bank for Jedi
    {true, 62, 400, 230, 24},                                                                            // Use recommended
    {true, 62, 428, 72, 32}, {true, 138, 428, 72, 32}, {true, 214, 428, 78, 32}};                        // Apply, Vanilla, Cancel
const char* const kUiName[C_N] = {"cfg", "status", "bg0", "bg1", "bg2", "title", "row0", "row1", "row2", "row3", "row4", "row5", "build", "instant", "bank", "recommended", "apply", "vanilla", "cancel"};
const char* const kTplPlain = "LBL_EXPERIENCE_STAT";
const char* const kTplFramed = "LBL_XP_BACK";
const char* const kPageBackFill = "uibit_fill_2bt";   // opaque black (Mod 14 LBL_EFX_BACK); LBL_XP_BACK's own fill uibit_fill_2wt is see-through
constexpr int kPlainRect[4] = {60, 449, 108, 18}, kFramedRect[4] = {54, 431, 223, 39};   // the templates' own unit rects (fallback scale)

struct UiPanel {
    uint32_t magic = 0;
    uintptr_t panel = 0;
    uintptr_t c[C_N] = {};
    int nCreated = 0;
    bool appended = false, createLogged = false;
    guikit::Scale fb = {1.f, 1.f, 0.f, 0.f, 60, 449, false};   // fallback scale from a control's ctor-time extent
    char text[C_N][UiText] = {};
    uint32_t textFail = 0, shownMask = 0;   // shownMask: bit i = control i currently shown by us (clicks only count on shown controls)
    bool colorTried = false, colorOk = false, nudgeOn = false;
    float base[3] = {};
    guikit::ClickEdge edge;
    int hover = -1;
    char lastTag[32] = {};
    // options page
    bool open = false;
    char pageTag[32] = {};
    uint64_t pageTable = 0;
    wiz::Item items[wiz::MaxItems] = {};
    int itemPreset[wiz::MaxItems] = {};
    int nItems = 0, recItem = -1;
    opt::State os = {};
    bool instant = false, bank = true, failed = false;
    int pause = 0;               // 0.3.1: bank on + pause N = last character level taken before banking; 0 = preset default (convertAt-1)
    int rowSlot[NumRows] = {};   // compact slot of each shown option row, -1 = hidden
};
UiPanel g_ui[MaxUiPanels];
int g_uiNext = 0;
bool g_uiNoRecLogged = false, g_uiNoScaleLogged = false;

UiPanel* findUi(uintptr_t panel) {
    for (UiPanel& r : g_ui) if (r.magic == UiMagic && r.panel == panel) return &r;
    return nullptr;
}

// ---- creation (ctor hook, layout open) ------------------------------------------------------------------------------------
void uiPanelCtor(uintptr_t panel) {
    saferead::beginScope();
    if (!panel) return;
    UiPanel* r = findUi(panel);
    if (!r) r = &g_ui[g_uiNext++ % MaxUiPanels];
    *r = UiPanel();
    r->magic = UiMagic;
    r->panel = panel;
    for (int i = 0; i < C_N; ++i) {
        const bool framed = kUiSpec[i].framed;
        bool usedFramed = framed;
        uintptr_t c = guikit::createLabelFromTemplate(panel, framed ? kTplFramed : kTplPlain);
        if (!c) { usedFramed = !framed; c = guikit::createLabelFromTemplate(panel, framed ? kTplPlain : kTplFramed); }   // missing tag: use the other template
        r->c[i] = c;
        if (!c) continue;
        ++r->nCreated;
        int x, y, w, h;
        if (!r->fb.ok && guikit::getExtent(c, &x, &y, &w, &h) && w > 0 && h > 0) {   // ctor-time extent = the template's rect in engine px
            const int* u = usedFramed ? kFramedRect : kPlainRect;
            r->fb = {static_cast<float>(w) / u[2], static_cast<float>(h) / u[3], static_cast<float>(x), static_cast<float>(y), u[0], u[1], true};
        }
    }
    logLine("UI ctor panel=%08x created=%d/%d fallbackScale=%d", (unsigned)panel, r->nCreated, (int)C_N, r->fb.ok);
}


// First tick: append every control (draw order = append order: the three backgrounds first), set alignments, log once.
void uiAppend(UiPanel* r) {
    int ok = 0;
    for (int i = 0; i < C_N; ++i) {
        if (!r->c[i]) continue;
        if (guikit::appendToPanel(r->panel, r->c[i])) ++ok; else r->c[i] = 0;   // a failed control stays hidden and unused
    }
    r->appended = true;
    for (int b = 0; b < NumBg; ++b) {   // opaque background: swap the framed template's translucent fill for black, full alpha (3 stacked = opaque)
        if (!r->c[C_BG0 + b]) continue;
        const bool f = guikit::setFill(r->c[C_BG0 + b], guikit::imageFor(kPageBackFill));
        const bool a = guikit::setAlpha(r->c[C_BG0 + b], 1.0f);
        logLine("UI page back%d fill=%s ok=%d alpha ok=%d", b, kPageBackFill, f, a);
    }
    for (int i = 0; i < C_N; ++i) {
        if (!r->c[i] || (i >= C_BG0 && i <= C_BG2) || i == C_STATUS) continue;
        guikit::setAlign(r->c[i], i == C_TITLE ? 17 : 18);   // 17 top-centre, 18 centre; the status line keeps the template's left alignment
    }
    unsigned mask = 0;
    for (int i = 0; i < C_N; ++i) if (r->c[i] && guikit::detail::textObjOk(r->c[i])) mask |= 1u << i;
    logLine("UI create panel=%08x n=%d appended=%d textobj=%05x", (unsigned)r->panel, r->nCreated, ok, mask);
    r->createLogged = true;
}

// ---- placement ------------------------------------------------------------------------------------------------------------
// ctor-time fallback. false = no usable scale.
bool uiScale(const UiPanel* r, guikit::Scale* out) {
    uint32_t arr = 0, ctl = 0;
    int cnt = 0;
    if (readAt(S_Ui, r->panel, guikit::Panel_CtlCount, &cnt) && cnt > BtnLevelUpIndex && readAt(S_Ui, r->panel, guikit::Panel_CtlArray, &arr) && arr &&
        readAt(S_Ui, arr, BtnLevelUpIndex * 4, &ctl) && ctl) {
        int x, y, w, h;
        if (guikit::getExtent(ctl, &x, &y, &w, &h) && w > 0 && h > 0) {
            const float a = static_cast<float>(w) / static_cast<float>(h);
            if (a >= 1.95f && a <= 2.75f) {
                const guikit::Scale s = guikit::scaleFromRef(ctl, 362, 325, 94, 40);
                if (s.ok) { *out = s; return true; }
            }
        }
    }
    if (r->fb.ok) { *out = r->fb; return true; }
    return false;
}

// Compact slots of the shown option rows (only while the page is open).
void uiSlots(UiPanel* r) {
    int k = 0;
    for (int row = 0; row < NumRows; ++row) r->rowSlot[row] = (r->open && opt::shown(r->os, row)) ? k++ : -1;
}
void uiPlace(UiPanel* r, const guikit::Scale& s) {
    for (int i = 0; i < C_N; ++i) {
        if (!r->c[i]) continue;
        int y = kUiSpec[i].y;
        if (i >= C_ROW0 && i < C_ROW0 + NumRows) y = kRowY0 + kRowStep * (r->rowSlot[i - C_ROW0] >= 0 ? r->rowSlot[i - C_ROW0] : 0);
        int x, py, w, h, ex, ey, ew, eh;
        guikit::unitRect(s, kUiSpec[i].x, y, kUiSpec[i].w, kUiSpec[i].h, &x, &py, &w, &h);
        if (!guikit::getExtent(r->c[i], &ex, &ey, &ew, &eh) || ex != x || ey != py || ew != w || eh != h) guikit::setRect(r->c[i], x, py, w, h);
    }
}

// ---- small setters ------------------------------------------------------------------------------------------------------
void uiShow(UiPanel* r, int i, bool vis) {
    if (vis) r->shownMask |= 1u << i; else r->shownMask &= ~(1u << i);
    if (r->c[i]) guikit::setVisible(r->c[i], vis);
}
void uiSetText(UiPanel* r, int i, const char* t) {
    if (!r->c[i] || !strcmp(r->text[i], t)) return;
    if (guikit::setText(r->c[i], t)) { snprintf(r->text[i], UiText, "%s", t); return; }
    if (!(r->textFail & (1u << i))) { r->textFail |= 1u << i; logLine("UI setText failed ctl=%s panel=%08x (text object not ready?)", kUiName[i], (unsigned)r->panel); }
}
// R9 highlight on the Configure button. The base colour is read back once; colour writes happen only when that readback looks sane (Mod 14 1140-1150).
void uiNudge(UiPanel* r, bool on) {
    if (!r->c[C_CFG]) return;
    if (!r->colorTried) {
        r->colorTried = true;
        float v[3];
        r->colorOk = guikit::getTextColor(r->c[C_CFG], v) && v[0] >= 0.f && v[0] <= 2.f && v[1] >= 0.f && v[1] <= 2.f && v[2] >= 0.f && v[2] <= 2.f;
        if (r->colorOk) memcpy(r->base, v, sizeof v);
        logLine("UI colour readback ok=%d base=(%.2f,%.2f,%.2f)", r->colorOk, r->colorOk ? v[0] : 0.0, r->colorOk ? v[1] : 0.0, r->colorOk ? v[2] : 0.0);
    }
    if (!r->colorOk || on == r->nudgeOn) return;
    if (on) guikit::setTextColor(r->c[C_CFG], 1.0f, 0.85f, 0.3f);
    else guikit::setTextColor(r->c[C_CFG], r->base[0], r->base[1], r->base[2]);
    r->nudgeOn = on;
}


// ---- options page -----------------------------------------------------------------------------------------------------------
int uiCountSelectable(const char* tag) {
    int n = 0;
    for (int i = 0; i < g_nPresets; ++i) {
        const Preset& p = g_presets[i];
        if (!p.indexed || _stricmp(p.tag, tag)) continue;
        const char* const ans[wiz::NumQuestions] = {p.role, p.style, p.alignment, p.convert, p.skills};
        if (wiz::selectable(ans)) ++n;
    }
    return n;
}
void uiClosePage(UiPanel* r) {
    r->open = false;
    r->nItems = 0;
    r->recItem = -1;
    r->os = opt::State();
    r->failed = false;
    for (int row = 0; row < NumRows; ++row) r->rowSlot[row] = -1;
}
// First class of a preset's records (the companion's base class); -1 when none.
int presetBaseClass(const Preset& p) {
    for (int cl = 0; cl <= MaxLevel; ++cl) if (p.lv[cl].present) return p.lv[cl].cls;
    return -1;
}
// 0.3.1 Pause at level N. Only presets with a hold class and a conversion level can pause. Default pause = convertAt-1 (the preset's own rule);
// valid explicit range minPause .. convertAt-2, minPause = max(1, char level of the first present record - 1) (the level the companion already is).
bool uiCanPause(const Preset& p) { return p.holdClass != 0 && p.convertAt > 0; }
int uiMinPause(const Preset& p) {
    for (int cl = 0; cl <= MaxLevel; ++cl) if (p.lv[cl].present) return cl - 1 > 1 ? cl - 1 : 1;
    return 1;
}
// Pause value as stored: out of range (incl. the default convertAt-1 and above) or not pausable -> 0.
int uiClampPause(const Preset& p, int pause) {
    if (!uiCanPause(p) || pause < uiMinPause(p) || pause > p.convertAt - 2) return 0;
    return pause;
}
// Next state of the Bank control: default -> convertAt-2 -> ... -> minPause -> Off -> default. Presets without a pause range just toggle.
void uiBankCycle(const Preset& p, bool* bank, int* pause) {
    if (!uiCanPause(p)) { *bank = !*bank; *pause = 0; return; }
    if (!*bank) { *bank = true; *pause = 0; return; }
    const int next = (*pause > 0 ? *pause : p.convertAt - 1) - 1;
    if (next < uiMinPause(p)) { *bank = false; *pause = 0; } else *pause = next;
}
// Text of the Bank control (<= 18 chars: the control is 113 wide).
const char* uiBankText(const Preset& p, bool bank, int pause, char* buf, size_t cap) {
    if (!uiCanPause(p)) snprintf(buf, cap, "%s", bank ? "Bank for Jedi: On" : "Bank for Jedi: Off");
    else if (!bank) snprintf(buf, cap, "Bank for Jedi: Off");
    else snprintf(buf, cap, "Pause at level %d", pause > 0 ? pause : p.convertAt - 1);
    return buf;
}
// Re-clamps the page's pause into the current preset's range (after a row cycle / Use recommended changed the preset).
void uiClampPagePause(UiPanel* r) { if (r->os.n > 0) r->pause = uiClampPause(g_presets[r->itemPreset[r->os.cur]], r->pause); }

// Opens the page: items = the tag's selectable presets; start = the configured preset, else the recommended one, else the first.
void uiOpenPage(UiPanel* r, const char* tag) {
    uiClosePage(r);
    for (int i = 0; i < g_nPresets && r->nItems < wiz::MaxItems; ++i) {
        const Preset& p = g_presets[i];
        if (!p.indexed || _stricmp(p.tag, tag)) continue;
        const char* const ans[wiz::NumQuestions] = {p.role, p.style, p.alignment, p.convert, p.skills};
        if (!wiz::selectable(ans)) continue;
        wiz::Item& it = r->items[r->nItems];
        it.id = p.id;
        for (int q = 0; q < wiz::NumQuestions; ++q) it.ans[q] = ans[q];
        it.shortName = p.shortName;
        if (p.recommended && r->recItem < 0) r->recItem = r->nItems;
        r->itemPreset[r->nItems++] = i;
    }
    const TagCfg* cfg = tagConfig(tag);
    const bool configured = cfg && cfg->preset >= 0 && g_tableOk;
    int start = -1;
    const char* how = "first";
    if (configured) for (int i = 0; i < r->nItems; ++i) if (r->itemPreset[i] == cfg->preset) { start = i; how = "configured"; break; }
    if (start < 0 && r->recItem >= 0) { start = r->recItem; how = "recommended"; }
    if (start < 0) start = 0;
    opt::init(r->os, r->items, r->nItems, start);
    r->instant = configured ? cfg->instant : false;
    r->bank = configured ? cfg->hold : true;
    r->pause = configured ? cfg->pause : 0;
    uiClampPagePause(r);
    snprintf(r->pageTag, sizeof r->pageTag, "%s", tag);
    r->pageTable = g_tableStamp;
    r->open = r->nItems > 0;
    if (r->open) logLine("UI page open tag=%s start=%s (%s)", tag, r->items[start].id, how);
}
// Text of one option row for the current preset (rows Convert, Role, Style, Alignment, Skills, Variant).
const char* uiRowText(const Preset& p, int row, char* buf, size_t cap) {
    char c1[16];
    switch (row) {
    case 0: {   // Convert
        const char* lab = wiz::answerLabel(wiz::Convert, p.convert);
        if (p.holdClass && p.convertAt > 0) snprintf(buf, cap, "Jedi: %s - %s at %d", lab, className(p.holdClass, c1, sizeof c1), p.convertAt);
        else if (wiz::ieq(p.convert, "never") && presetBaseClass(p) >= 0) snprintf(buf, cap, "Jedi: Never (stays %s)", className(presetBaseClass(p), c1, sizeof c1));
        else snprintf(buf, cap, "Jedi: %s", lab);
        break;
    }
    case 1: snprintf(buf, cap, "Role: %s", wiz::answerLabel(wiz::Role, p.role)); break;
    case 2: snprintf(buf, cap, "Weapons: %s", wiz::answerLabel(wiz::Style, p.style)); break;
    case 3: snprintf(buf, cap, "Powers: %s", wiz::answerLabel(wiz::Alignment, p.alignment)); break;
    case 4: snprintf(buf, cap, "Skills: %s", wiz::answerLabel(wiz::Skills, p.skills)); break;
    default: snprintf(buf, cap, "Variant: %s", p.shortName); break;
    }
    return buf;
}

// Draws the page for the current state.
void uiRenderPage(UiPanel* r, const char* tag) {
    if (r->os.n <= 0) { uiClosePage(r); return; }
    const Preset& P = g_presets[r->itemPreset[r->os.cur]];
    const TagCfg* cfg = tagConfig(tag);
    const bool configured = cfg && cfg->preset >= 0 && g_tableOk;
    char buf[UiText];
    uiSlots(r);
    for (int b = 0; b < NumBg; ++b) uiShow(r, C_BG0 + b, true);
    if (r->failed) uiSetText(r, C_TITLE, "Could not save - see log");
    else { snprintf(buf, sizeof buf, "%s - %s", P.name[0] ? P.name : tag, P.recommended ? "recommended build" : "level-up path"); uiSetText(r, C_TITLE, buf); }
    uiShow(r, C_TITLE, true);
    for (int row = 0; row < NumRows; ++row) {
        const bool sh = r->rowSlot[row] >= 0;
        if (sh) uiSetText(r, C_ROW0 + row, uiRowText(P, row, buf, sizeof buf));
        uiShow(r, C_ROW0 + row, sh);
    }
    uiSetText(r, C_BUILD, P.shortName);
    uiShow(r, C_BUILD, true);
    uiSetText(r, C_INSTANT, r->instant ? "Instant: On" : "Instant: Off");
    uiShow(r, C_INSTANT, true);
    { char bb[24]; uiSetText(r, C_BANK, uiBankText(P, r->bank, r->pause, bb, sizeof bb)); }
    uiShow(r, C_BANK, P.holdClass != 0);
    uiSetText(r, C_REC, "Use recommended");
    uiShow(r, C_REC, r->recItem >= 0 && r->os.cur != r->recItem);
    uiSetText(r, C_APPLY, "Apply");
    uiShow(r, C_APPLY, true);
    uiSetText(r, C_VANILLA, "Vanilla");
    uiShow(r, C_VANILLA, configured);
    uiSetText(r, C_CANCEL, "Cancel");
    uiShow(r, C_CANCEL, true);
}

void uiHideAll(UiPanel* r) {
    for (int i = 0; i < C_N; ++i) uiShow(r, i, false);
    if (r->open) uiClosePage(r);
    r->hover = -1;
}

// ---- input ----------------------------------------------------------------------------------------------------------------
bool uiClickable(const UiPanel* r, int i) {
    if (!(r->shownMask & (1u << i))) return false;
    if (i == C_CFG) return !r->open;
    if (!r->open) return false;
    return (i >= C_ROW0 && i < C_ROW0 + NumRows) || i == C_INSTANT || i == C_BANK || i == C_REC || i == C_APPLY || i == C_VANILLA || i == C_CANCEL;
}
// Clickable shown control under (x,y); -1 = none.
int uiHit(const UiPanel* r, int x, int y) {
    for (int i = 0; i < C_N; ++i) if (uiClickable(r, i) && r->c[i] && guikit::hit(r->c[i], x, y)) return i;
    return -1;
}

void uiClick(UiPanel* r, const char* tag, int ctl) {
    const int row = (ctl >= C_ROW0 && ctl < C_ROW0 + NumRows) ? ctl - C_ROW0 : -1;
    logLine("UI click %s tag=%s row=%s", kUiName[ctl], tag, row >= 0 ? opt::rowName(row) : "-");
    if (ctl == C_CFG) { uiOpenPage(r, tag); return; }
    if (!r->open) return;
    if (ctl == C_CANCEL) { uiClosePage(r); return; }
    r->failed = false;
    if (row >= 0) { opt::cycle(r->os, row, +1); uiClampPagePause(r); return; }
    switch (ctl) {
    case C_INSTANT: r->instant = !r->instant; break;
    case C_BANK: uiBankCycle(g_presets[r->itemPreset[r->os.cur]], &r->bank, &r->pause); break;
    case C_REC: if (r->recItem >= 0) { r->os.cur = r->recItem; uiClampPagePause(r); } break;   // re-init on the recommended item; toggles are kept (pause clamped)
    case C_APPLY: {
        const Preset& P = g_presets[r->itemPreset[r->os.cur]];
        const bool hold = P.holdClass ? r->bank : true;
        const int pause = (P.holdClass && r->bank) ? uiClampPause(P, r->pause) : 0;
        const bool ok = ptApply(tag, P.id, r->instant, hold, pause);
        logLine("UI apply tag=%s preset=%s instant=%d hold=%d pause=%d ok=%d", tag, P.id, r->instant, hold, pause, ok);
        if (ok) uiClosePage(r); else r->failed = true;
        break;
    }
    case C_VANILLA: {
        const bool ok = ptRemove(tag);
        logLine("UI vanilla tag=%s ok=%d", tag, ok);
        if (ok) uiClosePage(r); else r->failed = true;
        break;
    }
    default: break;
    }
}

// ---- tick (from the SetStats hook) ----------------------------------------------------------------------------------------
void uiTick(uintptr_t panel, uintptr_t stats, int realCan, int heldResult) {
    (void)heldResult;   // the held state is re-evaluated (evalStats) so that "holding with no XP yet" is shown too
    saferead::beginScope();
    UiPanel* r = findUi(panel);
    if (!r) {
        if (!g_uiNoRecLogged) { g_uiNoRecLogged = true; logLine("UI tick: no record for panel=%08x (ctor hook not seen)", (unsigned)panel); }
        return;
    }
    const bool click = guikit::clickEdge(r->edge);   // sampled every tick so a press that began elsewhere never fires
    if (!r->appended) uiAppend(r);
    if (!gateActive()) { uiHideAll(r); return; }
    char tag[32];
    const bool haveTag = stats && readTag(stats, tag, sizeof tag);
    if (!haveTag || !uiCountSelectable(tag)) { uiHideAll(r); r->lastTag[0] = 0; return; }   // PC (empty tag) or a tag without selectable presets
    if (strcmp(r->lastTag, tag)) { if (r->open) uiClosePage(r); snprintf(r->lastTag, sizeof r->lastTag, "%s", tag); }
    if (r->open && (_stricmp(r->pageTag, tag) || r->pageTable != g_tableStamp)) uiClosePage(r);
    guikit::Scale sc;
    if (!uiScale(r, &sc)) {
        if (!g_uiNoScaleLogged) { g_uiNoScaleLogged = true; logLine("UI no usable scale (BTN_LEVELUP missing, no fallback) - controls hidden"); }
        uiHideAll(r);
        return;
    }
    // input first (acts on what the previous tick drew)
    int cx = 0, cy = 0;
    const int hit = guikit::cursorEngine(&cx, &cy) ? uiHit(r, cx, cy) : -1;
    const bool clickable = hit >= 0 && uiClickable(r, hit);
    const bool hovered = clickable && hit == r->hover;   // the cursor was already over this control on the previous tick (guards clicks meant for a GUI on top)
    if (clickable && hit != r->hover) guikit::playHoverSound();
    r->hover = clickable ? hit : -1;
    if (click && hovered) uiClick(r, tag, hit);

    uiSlots(r);
    uiPlace(r, sc);

    // draw
    const TagCfg* cfg = tagConfig(tag);
    const Preset* P = cfg && cfg->preset >= 0 && g_tableOk ? &g_presets[cfg->preset] : nullptr;
    const bool configured = P != nullptr;
    if (r->open) {
        uiShow(r, C_CFG, false);
        uiShow(r, C_STATUS, false);
        uiRenderPage(r, tag);
        return;
    }
    for (int i = C_BG0; i < C_N; ++i) uiShow(r, i, false);
    bool hl = false;
    uiSetText(r, C_CFG, cfgButtonText(configured, realCan != 0, &hl));
    uiNudge(r, hl);
    uiShow(r, C_CFG, true);
    char st[UiText];
    bool held = false;
    int banked = 0;
    if (configured) {
        const Eval e = evalStats(stats, 0);
        held = e.ok && e.d == Decision::Hold && g_cfg.mode == Mode::Rewrite;
        if (held) banked = bankedLevels(stats);
    }
    bool sv = configured && buildStatus(true, true, held, banked, P->holdClass, P->shortName, cfg->instant, st, sizeof st);
    if (sv && g_cfg.mode != Mode::Rewrite) { const size_t l = strlen(st); snprintf(st + l, sizeof st - l, " (inactive: mode=log)"); }
    if (sv) uiSetText(r, C_STATUS, st);
    uiShow(r, C_STATUS, sv);
}
